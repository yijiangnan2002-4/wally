/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

// For Register AT command handler
// System head file

#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include "at_command.h"

#include "syslog.h"

#define LOGMSGIDE(fmt,arg...)   LOG_MSGID_E(atcmd ,"ATCMD: "fmt,##arg)
#define LOGMSGIDW(fmt,arg...)   LOG_MSGID_W(atcmd ,"ATCMD: "fmt,##arg)
#define LOGMSGIDI(fmt,arg...)   LOG_MSGID_I(atcmd ,"ATCMD: "fmt,##arg)

#if !defined(MTK_AES_AT_COMMAND_DISABLE) && defined(__GNUC__) && defined(HAL_AES_MODULE_ENABLED)

#include "hal_aes.h"

#define STRNCPY_CRYPTO(dest, source) strncpy(dest, source, strlen(source)+1)

/* #define AES_DEBUG_DUMP */

/*
 * sample code
*/

static const uint8_t hardware_id_128[16] = {
    0x4d, 0x54, 0x4b, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x32, 0x30, 0x31, 0x34, 0x30, 0x38, 0x31, 0x35
};

static const uint8_t hardware_id_192[24] = {
    0x4d, 0x54, 0x4b, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x32, 0x30, 0x31, 0x34, 0x30, 0x38, 0x31, 0x35,
    0x4d, 0xaa, 0x4b, 0xef, 0x35, 0x40, 0x45, 0x50,
};

static const uint8_t hardware_id_256[32] = {
    0x4d, 0x54, 0x4b, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x32, 0x30, 0x31, 0x34, 0x30, 0x38, 0x31, 0x35,
    0x4d, 0xaa, 0x4b, 0xef, 0x35, 0x40, 0x45, 0x50,
    0xaa, 0x50, 0x31, 0xba, 0xcd, 0x38, 0x31, 0x35
};

static const uint8_t aes_cbc_iv[HAL_AES_CBC_IV_LENGTH] = {
    0x61, 0x33, 0x46, 0x68, 0x55, 0x38, 0x31, 0x43,
    0x77, 0x68, 0x36, 0x33, 0x50, 0x76, 0x33, 0x46
};

static ATTR_RWDATA_IN_NONCACHED_RAM_4BYTE_ALIGN uint8_t plain[16] = {
    0x00, 0x0b, 0x16, 0x21, 0x2c, 0x37, 0x42, 0x4d, 0x58, 0x63, 0x00, 0x0b, 0x16, 0x21, 0x11, 0x22
};

static ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN uint8_t encrypted_buffer[16];
static ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN uint8_t decrypted_buffer[16];

#ifdef AES_DEBUG_DUMP
void result_dump(uint8_t *result, uint8_t length)
{
    uint8_t i;

    for (i = 0; i < length; i++) {
        if (i % 16 == 0) {
            LOGMSGIDI("\r\n", 0);
        }

        LOGMSGIDI(" %02x ", 1, result[i]);
    }
    LOGMSGIDI("\r\n", 0);
}
#endif

/*
    input=16B (total=16B)
    padding=0B
    output=16B
*/
bool aes_cbc_ecb_test(int bits)
{
    hal_aes_buffer_t plain_text = {
        .buffer = plain,
        .length = sizeof(plain)
    };
    hal_aes_buffer_t key = {
        .buffer = (uint8_t *)hardware_id_128,
        .length = sizeof(hardware_id_128)
    };

    hal_aes_buffer_t encrypted_text = {
        .buffer = encrypted_buffer,
        .length = sizeof(encrypted_buffer)
    };

    hal_aes_buffer_t decrypted_text = {
        .buffer = decrypted_buffer,
        .length = sizeof(decrypted_buffer)
    };

    if (bits == 128) {
        key.buffer = (uint8_t *)hardware_id_128;
        key.length = sizeof(hardware_id_128);
    } else if (bits == 192) {
        key.buffer = (uint8_t *)hardware_id_192;
        key.length = sizeof(hardware_id_192);
    } else if (bits == 256) {
        key.buffer = (uint8_t *)hardware_id_256;
        key.length = sizeof(hardware_id_256);
    } else {
        return 0;
    }

    if (-1 == hal_aes_cbc_encrypt(&encrypted_text, &plain_text, &key, (uint8_t *)aes_cbc_iv)) {
        return 0;
    } else {
        if (-1 == hal_aes_cbc_decrypt(&decrypted_text, &encrypted_text, &key, (uint8_t *)aes_cbc_iv)) {
            return 0;
        } else {
            if (0 == memcmp(plain_text.buffer, decrypted_text.buffer, plain_text.length)) {
#ifdef AES_DEBUG_DUMP
                result_dump(plain_text.buffer, plain_text.length);
                result_dump(encrypted_text.buffer, encrypted_text.length);
                result_dump(decrypted_text.buffer, decrypted_text.length);
#endif
                /* continue next test case - aes_ecb */
            } else {
                return 0;
            }
        }
    }

    if (-1 == hal_aes_ecb_encrypt(&encrypted_text, &plain_text, &key)) {
        return 0;
    } else {
        if (-1 == hal_aes_ecb_decrypt(&decrypted_text, &encrypted_text, &key)) {
            return 0;
        } else {
            if (0 == memcmp(plain_text.buffer, decrypted_text.buffer, plain_text.length)) {
#ifdef AES_DEBUG_DUMP
                result_dump(plain_text.buffer, plain_text.length);
                result_dump(encrypted_text.buffer, encrypted_text.length);
                result_dump(decrypted_text.buffer, decrypted_text.length);
#endif
                /* return 1; */
            } else {
                return 0;
            }
        }
    }

    return 1;
}

/*--- Function ---*/
atci_status_t atci_cmd_hdlr_crypto(atci_parse_cmd_param_t *parse_cmd);

/*
AT+ECRYPTO=<op>             |   "OK"
AT+ECRYPTO=AES,128,CBC?     |   "+ECRYPTO=(AES, 128, CBC)","OK"
AT+ECRYPTO=AES,128,ECB?     |   "+ECRYPTO=(AES, 128, ECB)","OK"
AT+ECRYPTO=AES,192,CBC?     |   "+ECRYPTO=(AES, 192, CBC)","OK"
AT+ECRYPTO=AES,192,ECB?     |   "+ECRYPTO=(AES, 192, ECB)","OK"
AT+ECRYPTO=AES,256,CBC?     |   "+ECRYPTO=(AES, 256, CBC)","OK"
AT+ECRYPTO=AES,256,ECB?     |   "+ECRYPTO=(AES, 256, ECB)","OK"
*/

/* AT command handler  */
atci_status_t atci_cmd_hdlr_crypto(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = atci_mem_alloc(sizeof(atci_response_t));
    if(NULL == response) {
        return ATCI_STATUS_ERROR;
    }
    memset(response, 0, sizeof(atci_response_t));

    char *param = NULL;
    char *type = NULL;
    char *key_length = NULL;
    char *mode = NULL;
    /* hal_aes_status_t ret; */

    LOGMSGIDI("atci_cmd_hdlr_crypto \r\n", 0);
#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    response->cmd_id = parse_cmd->cmd_id;
#endif

    response->response_flag = 0; /*    Command Execute Finish.  */

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:    /* rec: AT+ECRYPTO=?   */
            STRNCPY_CRYPTO((char *)response->response_buf, "+ECRYPTO=(\"<AES, Key Length, Mode>\")\r\nOK\r\n");
            response->response_len = strlen((char *)response->response_buf);
            atci_send_response(response);
            break;

        case ATCI_CMD_MODE_EXECUTION: /* rec: AT+ECRYPTO=<op>  the handler need to parse the parameters  */
            param = strtok(parse_cmd->string_ptr, "=");
            type = strtok(NULL, ",");  /* get type: AES */
            key_length = strtok(NULL, ",");  /* get key length: 128/192/256 */
            mode = strtok(NULL, ",");  /* get mode: CBC/ECB */

            LOGMSGIDI("\n\r param=%s\n\r \n\r TYPE=%s\n\r \n\r KEY_LEN=%s\n\r \n\r MODE=%s\n\r", 4, param, type, key_length, mode);
            if (type == NULL) {
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
                STRNCPY_CRYPTO((char *)response->response_buf, "ERROR: TYPE should be AES\r\n");
                response->response_len = strlen((char *)response->response_buf);
                atci_send_response(response);
                break;
            }

            if (key_length == NULL) {
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
                STRNCPY_CRYPTO((char *)response->response_buf, "ERROR: KEY LENGTH should be 128/192/256\r\n");
                response->response_len = strlen((char *)response->response_buf);
                atci_send_response(response);
                break;
            }

            if (mode == NULL) {
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
                STRNCPY_CRYPTO((char *)response->response_buf, "ERROR: MODE should be CBC/ECB/ALL\r\n");
                response->response_len = strlen((char *)response->response_buf);
                atci_send_response(response);
                break;
            }

            /* do test */
            if (0 == strcmp("AES", type)) {
                /*
                    Since CBC/ECB testing codes are integrated into single function, so no need to
                    check "mode" variable and do proper actions.
                */
                if (0 == strcmp("128", key_length)) {
                    if (0 == aes_cbc_ecb_test(128)) {  /* fail */
                        response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
                        STRNCPY_CRYPTO((char *)response->response_buf, "ERROR: AES CBC/ECB TEst 128b fail\r\n");
                        response->response_len = strlen((char *)response->response_buf);
                        atci_send_response(response);
                        break;
                    } else {
                        STRNCPY_CRYPTO((char *)response->response_buf, "AES CBC/ECB TEst 128b ok\r\n");
                        response->response_len = strlen((char *)response->response_buf);
                        atci_send_response(response);
                        break;
                    }
                }

                if (0 == strcmp("192", key_length)) {
                    if (0 == aes_cbc_ecb_test(192)) {  /* fail */
                        response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
                        STRNCPY_CRYPTO((char *)response->response_buf, "ERROR: AES CBC/ECB TEst 192b fail\r\n");
                        response->response_len = strlen((char *)response->response_buf);
                        atci_send_response(response);
                        break;
                    } else {
                        STRNCPY_CRYPTO((char *)response->response_buf, "AES CBC/ECB TEst 192b ok\r\n");
                        response->response_len = strlen((char *)response->response_buf);
                        atci_send_response(response);
                        break;
                    }
                }

                if (0 == strcmp("256", key_length)) {
                    if (0 == aes_cbc_ecb_test(256)) {  /* fail */
                        response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
                        STRNCPY_CRYPTO((char *)response->response_buf, "ERROR: AES CBC/ECB TEst 256b fail\r\n");
                        response->response_len = strlen((char *)response->response_buf);
                        atci_send_response(response);
                        break;
                    } else {
                        STRNCPY_CRYPTO((char *)response->response_buf, "AES CBC/ECB TEst 256b ok\r\n");
                        response->response_len = strlen((char *)response->response_buf);
                        atci_send_response(response);
                        break;
                    }
                }
            }

            response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            STRNCPY_CRYPTO((char *)response->response_buf, "ERROR: ATCI\r\n");
            response->response_len = strlen((char *)response->response_buf);
            atci_send_response(response);

            break;

        default :
            /* others are invalid command format */
            STRNCPY_CRYPTO((char *)response->response_buf, "ERROR\r\n");
            response->response_len = strlen((char *)response->response_buf);
            atci_send_response(response);
            break;
    }

    atci_mem_free(response);
    return ATCI_STATUS_OK;
}

#endif

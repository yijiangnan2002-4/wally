/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.
 * Without the prior written permission of Airoha and/or its licensors,
 * any reproduction, modification, use or disclosure of Airoha Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) Airoha Software
 * if you have agreed to and been bound by the applicable license agreement with
 * Airoha ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of Airoha Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
 */

#ifdef AIR_USB_ENABLE
#ifdef ATCI_ENABLE

/* C library */
#include <stdio.h>
#include <string.h>

/* USB Middleware includes */
#include "usb_custom.h"
#include "usb_nvkey_struct.h"
#include "usb_resource.h"
#include "usb_main.h"

/* Other Middleware includes */
#include "atci.h"

#ifdef AIR_NVDM_ENABLE
#include "nvkey.h"
#include "nvkey_id_list.h"
#endif /* AIR_NVDM_ENABLE */

/* Kernel includes */
#if FREERTOS_ENABLE
#include "FreeRTOS.h"
#endif /* FREERTOS_ENABLE */

/* Syslog create module for usb_detect_host.c */
#include "usb_dbg.h"
log_create_module_variant(USB_ATCI, DEBUG_LOG_ON, PRINT_LEVEL_INFO);

static atci_status_t usb_at_string_handler(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t usb_at_string_reset_handler(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t usb_at_string_save_handler(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t usb_at_basic_info_handler(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t usb_at_init_handler(atci_parse_cmd_param_t *parse_cmd);
static atci_status_t usb_at_deinit_handler(atci_parse_cmd_param_t *parse_cmd);


/* tricky reference function */
void USB_String_Reset(void);

static atci_cmd_hdlr_item_t g_usb_atci_cmd[] = {
    {
        .command_head = "AT+USBSTR",
        .command_hdlr = usb_at_string_handler,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+USBSTR_RESET",
        .command_hdlr = usb_at_string_reset_handler,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+USBSTR_SAVE",
        .command_hdlr = usb_at_string_save_handler,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+USB_BASICINFO",
        .command_hdlr = usb_at_basic_info_handler,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+USB_INIT",
        .command_hdlr = usb_at_init_handler,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
    {
        .command_head = "AT+USB_DEINIT",
        .command_hdlr = usb_at_deinit_handler,
        .hash_value1 = 0,
        .hash_value2 = 0,
    },
};

void usb_atci_init(void)
{
    atci_register_handler(g_usb_atci_cmd, sizeof(g_usb_atci_cmd) / sizeof(atci_cmd_hdlr_item_t));
}

static atci_status_t usb_at_string_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = (atci_response_t *)pvPortMalloc(sizeof(atci_response_t));

    usb_string_usage_t usage;
    uint8_t id;
    char str[31];
    int status;

    char *s = NULL;

    LOG_MSGID_I(USB_ATCI, "usb_at_string_handler mode = %d", 1, parse_cmd->mode);
    if (response == NULL) {
        LOG_MSGID_E(USB_ATCI, "usb_at_string_handler get memory fail", 0);
        return ATCI_STATUS_ERROR;
    }
    memset(response, 0x0, sizeof(atci_response_t));
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            s = parse_cmd->string_ptr + parse_cmd->name_len + 1;

            status = sscanf(s, "%x,%x,%[^\n\r]", (unsigned int *)&usage, (unsigned int *)&id, str);
            if (status > 0) {
                USB_String_Create(usage, id, str);

                snprintf((char *)response->response_buf,
                         ATCI_UART_TX_FIFO_BUFFER_SIZE,
                         "usage:x%02x,id:x%02x,len:x%02x,%31s\r\n",
                         usage, id, strlen(str), str
                        );
                response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else {
                response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            }
            break;
        }
        default:
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            break;
    }
    response->response_len = strlen((char *)response->response_buf);
    atci_send_response(response);
    vPortFree(response);
    return ATCI_STATUS_OK;
}

static atci_status_t usb_at_string_reset_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = (atci_response_t *)pvPortMalloc(sizeof(atci_response_t));

    LOG_MSGID_I(USB_ATCI, "usb_at_string_reset_handler mode = %d", 1, parse_cmd->mode);
    if (response == NULL) {
        LOG_MSGID_E(USB_ATCI, "usb_at_string_reset_handler get memory fail", 0);
        return ATCI_STATUS_ERROR;
    }
    memset(response, 0x0, sizeof(atci_response_t));
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_ACTIVE: {
            USB_String_Reset();
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            break;
        }
        default:
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            break;
    }
    atci_send_response(response);
    vPortFree(response);
    return ATCI_STATUS_OK;
}

static atci_status_t usb_at_string_save_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = (atci_response_t *)pvPortMalloc(sizeof(atci_response_t));

    LOG_MSGID_I(USB_ATCI, "usb_at_string_save_handler mode = %d", 1, parse_cmd->mode);
    if (response == NULL) {
        LOG_MSGID_E(USB_ATCI, "usb_at_string_save_handler get memory fail", 0);
        return ATCI_STATUS_ERROR;
    }
    memset(response, 0x0, sizeof(atci_response_t));
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_ACTIVE: {
            USB_String_Nvkey_Write();
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            break;
        }
        default:
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            break;
    }
    atci_send_response(response);
    vPortFree(response);
    return ATCI_STATUS_OK;
}

static atci_status_t usb_at_basic_info_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = (atci_response_t *)pvPortMalloc(sizeof(atci_response_t));

    char *s = NULL;
    const char *format_s = "%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x";
    uint32_t u32data[13];

    nvkey_status_t nvkey_status;
    uint32_t nvkey_size;
    usb_nvkey_basicinfo_t basicinfo;
    int status;

    LOG_MSGID_I(USB_ATCI, "usb_at_basic_info_handler mode = %d", 1, parse_cmd->mode);
    if (response == NULL) {
        LOG_MSGID_E(USB_ATCI, "usb_at_basic_info_handler get memory fail", 0);
        return ATCI_STATUS_ERROR;
    }
    memset(response, 0x0, sizeof(atci_response_t));
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: {
            s = parse_cmd->string_ptr + parse_cmd->name_len + 1;

            status = sscanf(s, format_s,
                            &u32data[0],  &u32data[1],  &u32data[2],  &u32data[3], &u32data[4],
                            &u32data[5],  &u32data[6],  &u32data[7],  &u32data[8], &u32data[9],
                            &u32data[10], &u32data[11], &u32data[12], &u32data[13]
                           );

            basicinfo.use_class_info   = u32data[0];
            basicinfo.use_product_info = u32data[1];
            basicinfo.use_power_info   = u32data[2];
            basicinfo.bcdUSB           = u32data[3];
            basicinfo.bDeviceClass     = u32data[4];
            basicinfo.bDeviceSubClass  = u32data[5];
            basicinfo.bDeviceProtocol  = u32data[6];
            basicinfo.idVendor         = u32data[7];
            basicinfo.idProduct        = u32data[8];
            basicinfo.bcdDevice        = u32data[9];
            basicinfo.self_power       = u32data[10];
            basicinfo.remote_wakeup    = u32data[11];
            basicinfo.max_power        = u32data[12];

            /* Debug use log */
            /*
            for (int i = 0; i < USB_BASICINFO_NVKEY_SIZE; i++) {
                LOG_MSGID_I(USB_ATCI, "usb_at_basic_info_handler basic_info[%d]=%02x", 2, i, basic_info[i]);
            }
            */

            LOG_MSGID_I(USB_ATCI, "usb_at_basic_info_handler status = %d", 1, status);
            if (status > 0) {
                nvkey_size = sizeof(basicinfo);
                nvkey_status = nvkey_write_data(NVID_USB_BASICINFO, (uint8_t *)&basicinfo, nvkey_size);

                if (nvkey_status) {
                    snprintf((char *)response->response_buf,
                             ATCI_UART_TX_FIFO_BUFFER_SIZE,
                             "write nvkey failed, status:%d\r\n",
                             status
                            );
                    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
                    break;
                }

                snprintf((char *)response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "use_class:x%02x,use_product:x%02x,use_power:x%02x\r\n",
                         basicinfo.use_class_info, basicinfo.use_product_info, basicinfo.use_power_info);
                response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            } else {
                response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            }
            break;
        }
        /*
        case ATCI_CMD_MODE_READ: {
            u32data[0]  = *use_class;
            u32data[1]  = *use_product;
            u32data[2]  = *use_power;
            u32data[3]  = class_info->bcdUSB;
            u32data[4]  = class_info->class;
            u32data[5]  = class_info->sub_class;
            u32data[6]  = class_info->protocol;
            u32data[7]  = product_info->vender_id;
            u32data[8]  = product_info->product_id;
            u32data[9]  = product_info->bcd_version;
            u32data[10] = power_info->self_power;
            u32data[11] = power_info->remote_wakeup;
            u32data[12] = power_info->maxpower;
            snprintf((char *)response->response_buf,
                     1024,
                     format_s,
                     u32data[0],  u32data[1],  u32data[2],  u32data[3], u32data[4],
                     u32data[5],  u32data[6],  u32data[7],  u32data[8], u32data[9],
                     u32data[10], u32data[11], u32data[12], u32data[13]
            );
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            break;
        }
        */
        default:
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            break;
    }
    atci_send_response(response);
    vPortFree(response);
    return ATCI_STATUS_OK;
}

static atci_status_t usb_at_init_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = (atci_response_t *)pvPortMalloc(sizeof(atci_response_t));

    LOG_MSGID_I(USB_ATCI, "usb_at_init_handler mode = %d", 1, parse_cmd->mode);
    if (response == NULL) {
        LOG_MSGID_E(USB_ATCI, "usb_at_init_handler get memory fail", 0);
        return ATCI_STATUS_ERROR;
    }
    memset(response, 0x0, sizeof(atci_response_t));
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_ACTIVE: {
            ap_usb_init(USB_AUDIO);
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            break;
        }
        default:
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            break;
    }
    atci_send_response(response);
    vPortFree(response);
    return ATCI_STATUS_OK;
}


static atci_status_t usb_at_deinit_handler(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = (atci_response_t *)pvPortMalloc(sizeof(atci_response_t));

    LOG_MSGID_I(USB_ATCI, "usb_at_deinit_handler mode = %d", 1, parse_cmd->mode);
    if (response == NULL) {
        LOG_MSGID_E(USB_ATCI, "usb_at_deinit_handler get memory fail", 0);
        return ATCI_STATUS_ERROR;
    }
    memset(response, 0x0, sizeof(atci_response_t));
    response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_ACTIVE: {
            ap_usb_deinit();
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
            break;
        }
        default:
            response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
            break;
    }
    atci_send_response(response);
    vPortFree(response);
    return ATCI_STATUS_OK;
}

#endif /* ATCI_ENABLE */
#endif /* AIR_USB_ENABLE */

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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "at_command.h"
#include "hal_gpt.h"
#include "hal_log.h"
#include "hal_irrx_internal.h"
#include "nvkey_id_list.h"
#include "hal_clock_internal.h"


#if defined(MTK_NVDM_ENABLE)
#include "nvkey.h"
#endif /* MTK_NVDM_ENABLE */

#ifdef HAL_DCXO_MODULE_ENABLED
extern ATTR_TEXT_IN_TCM void set_dcxo_mode_4testonly(dcxo_mode_t mode);
#endif

atci_status_t atci_cmd_hdlr_capid(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *p_response = atci_mem_alloc(sizeof(atci_response_t));
    if (NULL == p_response) {
        return ATCI_STATUS_ERROR;
    }
    memset(p_response, 0, sizeof(atci_response_t));

#ifdef HAL_DCXO_MODULE_ENABLED
    char *param1 = NULL;
    char *param2 = NULL;
    uint8_t things = 0;
    int32_t result = 0;
    uint32_t debug_capid = 0;
    //uint32_t read_efuse_capid=0;
#endif

    //log_hal_msgid_info("atci_cmd_hdlr_capid \r\n", 0);

    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION:
          #ifdef HAL_DCXO_MODULE_ENABLED
            //atci_send_response(&resonse);
            if (strncmp(parse_cmd->string_ptr, "AT+CAPID=", strlen("AT+CAPID=")) == 0) {
                param1 = (char *)strtok(parse_cmd->string_ptr, "=");
                param1 = (char *)strtok(NULL, ",");
                things = atoi(param1);
                //log_hal_msgid_info("things=%d\r\n", 1, things);

                if (1 == things) { //AT+CAPID=1,420   modify capid=420 dynamic.
                    /* set CAPID register */
                    param2 = (char *)strtok(NULL, ",");
                    debug_capid = atoi(param2);

                    set_capid_rg(debug_capid);   /* set CAPID register */

                    hal_gpt_delay_ms(1); /* waiting for signal stable */
                    //log_hal_msgid_info("Set %u to CAPID register.\r\n", 1, (int)debug_capid);

                    p_response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                } else if (2 == things) { //AT+CAPID=2,404,2  save capid to efuse CAPID2
                    /* write capid to nvdm */
                    /* also write to register */
                    param2 = (char *)strtok(NULL, ",");
                    debug_capid = atoi(param2);

                    result = set_capid_nvdm(debug_capid); /* Write CAPID to NVDM */
                    if (result == NVDM_STATUS_OK) {
                        set_capid_rg(debug_capid);   /* set CAPID register */
                    }

                    p_response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;

                } else if (3 == things) { //AT+CAPID=3  read efuse capid
                    /* Read CAPID NVKey, but don't set to register */
                    /* result : the read capid value from NVKey */
                    result = get_capid_nvdm();

                    p_response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                } else if (4 == things) { //AT+CAPID=4  read current capid
                    /* Read CAPID value, and save in result variable */
                    result = get_capid_rg();
                    //log_hal_msgid_info("CAPID register = %d\r\n", 1, (int)result);

                    p_response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                } else if (5 == things) { /* AT+CAPID=5,1 or 0 --> Set DCXO FPM or LPM */
                    param2 = (char *)strtok(NULL, ",");
                    set_dcxo_mode_4testonly(atoi(param2)==0? DCXO_LP_MODE:DCXO_NORMAL_MODE);
                    result = dcxo_current_mode();

                    p_response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                }
            }

            snprintf((char *)p_response->response_buf, sizeof(p_response->response_buf), "+CAPID:%d,%ld\r\n", things, result);
          #else  /* HAL_DCXO_MODULE_ENABLED */
            snprintf((char *)p_response->response_buf, sizeof(p_response->response_buf), "DCXO MODULE not ENABLED\r\n");
            p_response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;
          #endif /* !HAL_DCXO_MODULE_ENABLED */

            p_response->response_len = strlen((char *)p_response->response_buf);
            atci_send_response(p_response);

            break;
        default :
            /* others are invalid command format */
            p_response->response_len = strlen((char *)p_response->response_buf);
            p_response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR; // ATCI will help append "ERROR" at the end of response buffer
            atci_send_response(p_response);
            break;
    }

    atci_mem_free(p_response);
    return ATCI_STATUS_OK;
}

/*---ATA Command handler Function End---*/

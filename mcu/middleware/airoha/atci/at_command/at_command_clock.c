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

#include "hal_clock.h"

#if !defined(MTK_CLOCK_AT_COMMAND_DISABLE) && defined(HAL_CLOCK_MODULE_ENABLED)

#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "at_command.h"
#include <string.h>
#include "hal_clock_internal.h"

#include "syslog.h"
#define LOGE(fmt,arg...)   LOG_E(atcmd, "[clock]"fmt,##arg)
#define LOGW(fmt,arg...)   LOG_W(atcmd, "[clock]"fmt,##arg)
#define LOGI(fmt,arg...)   LOG_I(atcmd ,"[clock]"fmt,##arg)

typedef enum {
    AT_CMD_METER = 0,
    AT_CMD_MUX,
    AT_CMD_CG,
    AT_CMD_TEST,
    AT_CMD_DCXO
} at_cmd_clock;

#define LOGMSGIDE(fmt,cnt,arg...)   LOG_MSGID_E(atcmd, "[clock]"fmt,cnt,##arg)
#define LOGMSGIDW(fmt,cnt,arg...)   LOG_MSGID_W(atcmd, "[clock]"fmt,cnt,##arg)
#define LOGMSGIDI(fmt,cnt,arg...)   LOG_MSGID_I(atcmd ,"[clock]"fmt,cnt,##arg)

/*--- Function ---*/
atci_status_t atci_cmd_hdlr_clock(atci_parse_cmd_param_t *parse_cmd);

// AT command handler
atci_status_t atci_cmd_hdlr_clock(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *p_response = atci_mem_alloc(sizeof(atci_response_t));
    if (NULL == p_response) {
        return ATCI_STATUS_ERROR;
    }
    memset(p_response, 0, sizeof(atci_response_t));

    uint32_t param0_val, param1_val;

    //LOGMSGIDI("atci_cmd_hdlr_clock\r\n", 0);
#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    p_response->cmd_id = parse_cmd->cmd_id;
#endif
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:
            /* AT+CLOCK=? */
            snprintf((char *)p_response->response_buf, sizeof(p_response->response_buf), "AT+CLOCK=?\r\nTest %s\r\n", "OK");
            break;

        case ATCI_CMD_MODE_READ:
            /* AT+CLOCK? */
#if (PRODUCT_VERSION != 3335)
            clock_dump_info();
#endif
            snprintf((char *)p_response->response_buf, sizeof(p_response->response_buf), "AT+CLOCK=?\r\nTest %s\r\n", "OK");
            break;

        default :
            /* others are invalid command format */
            p_response->response_flag = ATCI_RESPONSE_FLAG_APPEND_ERROR;

            char *param = NULL;
            char *param_0 = NULL;
            char *param_1 = NULL;

            char *pch = strtok(parse_cmd->string_ptr, "=");
            if (!strcmp(pch, "AT+CLOCK")) {
                while ((pch = (strtok(NULL, ",")))) {
                    if (!param) {
                        param = pch;
                    } else if (!param_0) {
                        param_0 = pch;
                    } else if (!param_1) {
                        param_1 = pch;
                    } else {
                        break;
                    }
                }
                uint8_t func = strtol(param, NULL, 10);
                if (param_0) {
                    param0_val = strtol(param_0, NULL, 10);
                }
                if (param_1) {
                    param1_val = strtol(param_1, NULL, 10);
                }
                switch (func) {
                    case AT_CMD_METER: // AT+CLOCK=0,1000,15
#ifdef HAL_CLOCK_METER_ENABLE
                        if (param_0 && param_1) {
                            uint32_t winset = param0_val;
                            uint32_t clk_src = param1_val;
                            int freq = hal_clock_get_freq_meter(clk_src, winset);
                            snprintf((char *)p_response->response_buf, sizeof(p_response->response_buf), "freq meter [%d]\r\n", freq);
                            p_response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                        }
#endif
                        break;

                    case AT_CMD_MUX:
                        if (param_0 && param_1) {
                            uint32_t mux_id = param0_val;
                            uint32_t mux_sel = param1_val;
                            if (clock_mux_sel(mux_id, mux_sel) == HAL_CLOCK_STATUS_OK) {
                                p_response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                            }
                        }
                        break;
                    case AT_CMD_CG:
                        if (param_0 && param_1) {
                            hal_clock_cg_id clk_cg_id = param0_val;
                            int result = 0;
                            uint32_t on_off = param1_val;

                            if (on_off) {
                                result = hal_clock_enable(clk_cg_id);
                            } else {
                                result = hal_clock_disable(clk_cg_id);
                            }

                            if (result != HAL_CLOCK_STATUS_OK) {
                                p_response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR; // ATCI will help append "ERROR" at the end of response buffer
                            } else {
                                p_response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                            }
                        }
                        break;

                    case AT_CMD_TEST:

                        p_response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                        break;

#if defined(AIR_BTA_IC_STEREO_HIGH_G3) || defined(AIR_BTA_IC_PREMIUM_G3)
    #ifdef HAL_DCXO_MODULE_ENABLED
                    /* DCXO Mode Test: AT+CLOCK=4,param_0,param_1 */
                    case AT_CMD_DCXO:  /* Before test, let BT standby: AT+BTCMIT=BT_STANDBY */
                        hal_dcxo_at_cmd(param_0, param_1, (char *)p_response->response_buf);
                        p_response->response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
                        break;
    #endif
#endif
                }
            }
    }
    p_response->response_len = strlen((char *)p_response->response_buf);
    atci_send_response(p_response);

    atci_mem_free(p_response);
    return ATCI_STATUS_OK;
}

#endif /* !MTK_CLOCK_AT_COMMAND_DISABLE && HAL_CLOCK_MODULE_ENABLED */


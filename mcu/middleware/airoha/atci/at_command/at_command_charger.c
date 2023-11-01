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

#include "hal_feature_config.h"
#ifdef HAL_PMU_MODULE_ENABLED
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "at_command.h"
#include "syslog.h"
#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
#include "battery_management.h"
#include "battery_management_core.h"
#endif
#if (IC_CONFIG == ab155x)
#include "hal_charger.h"
#endif
log_create_module(atci_charger, PRINT_LEVEL_INFO);
#define MAX_CHAR_NUM 6
#include "hal_pmu.h"
int task_timer = 1000;
int count = 0;
static TaskHandle_t charger_task_handle = NULL;
int pmu_debug_index = 0;
#if defined(AIR_BTA_PMIC_G2_HP)
#include "hal_pmu_auxadc_2568.h"
#endif
#ifdef MTK_BATTERY_MANAGEMENT_ENABLE

#if defined(AIR_BTA_PMIC_HP) || defined(AIR_BTA_PMIC_G2_HP)

const char *charger_type[10] = { "", "SDP", "CDP", "DCP", "SS", "IPAD2_IPAD4", "IPHONE_5V_1A", "NON-STD", "DP/DM_Floating", "UNABLE_TO_IDENTIFY"};
const char *charger_state[8] = { "CHARGER OFF", "PRECC", "CC", "EXTENSION", "EOC", "THERMAL", "VBAT_OVP", "SAFETY TIMER"};
#endif
#if defined(AIR_BTA_PMIC_LP) || defined(AIR_BTA_PMIC_G2_LP)
const char *charger_type[10] = { "", "SDP", "CDP", "DCP", "SS", "IPAD2_IPAD4", "IPHONE_5V_1A", "NON-STD", "DP/DM_Floating", "UNABLE_TO_IDENTIFY"};
const char *charger_state[8] = { "CHARGER OFF", "TRICKLE", "CC1", "CC2", "CV_INT", "CV", "EOC", "RECHG"};
#endif
#endif
#ifdef MTK_FUEL_GAUGE
#include "battery_meter.h"
#endif
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
#define BATTERY_VOLTAGE 4200
#include "hal_pmu_charger_lp.h"
extern TaskHandle_t battery_lineat_task_t;
#endif
#if defined(AIR_BTA_IC_PREMIUM_G3_TYPE_P)
#include "hal_pmu_charger_lp.h"
#endif

void pmu_dump_auxadc(void)
{
#if defined(AIR_BTA_PMIC_HP) || defined(AIR_BTA_PMIC_G2_HP)
    LOG_MSGID_I(common, "[PMU_AUXADC][CH0]PN_ZCV==%d,"
                        "[CH0]WK_ZCV==%d,[CH0]BATSNS==%d"
                        "[CH0]BAT_RECHARGER==%d,[CH1]VBUS_UART==%d"
                        "[CH2]VBUS==%d,[CH3]CHR_THM==%d"
                        "[CH3]HW_JEITA==%d,[CH4]PMIC_AP==%d", 9
                        ,pmu_auxadc_get_channel_value(PMU_AUX_PN_ZCV),pmu_auxadc_get_channel_value(PMU_AUX_WK_ZCV)
                        ,pmu_auxadc_get_channel_value(PMU_AUX_BATSNS),pmu_auxadc_get_channel_value(PMU_AUX_BAT_RECHARGER)
                        ,pmu_auxadc_get_channel_value(PMU_AUX_VBUS_UART),pmu_auxadc_get_channel_value(PMU_AUX_VBUS)
                        ,pmu_auxadc_get_channel_value(PMU_AUX_CHR_THM),pmu_auxadc_get_channel_value(PMU_AUX_HW_JEITA)
                        ,pmu_auxadc_get_channel_value(PMU_AUX_PMIC_AP));
#endif
}

void pmu_dump_battery(void)
{
#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
    int32_t charger_status;
    charger_status = battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_EXIST);
    LOG_MSGID_I(common, "[BM]Bat cap perc = %d, Chg stat = %d, "
                            "Bat temp JEITA CH3 = %d (Celsius), Bat temp PMIC CH4 = %d (Celsius)"
                            "Bat volt = %d (mV)", 5, (int)battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY)
                            ,(int)charger_status, (int)battery_management_get_battery_property(BATTERY_PROPERTY_TEMPERATURE)
                            ,(int)battery_management_get_battery_property(BATTERY_PROPERTY_PMIC_TEMPERATURE)
                            ,(int) battery_management_get_battery_property(BATTERY_PROPERTY_VOLTAGE));
    if (charger_status) {
        LOG_I(common, "[BM]Charger type = %s, Charger State = %s\n", charger_type[battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_TYPE)]
                                                                   , charger_state[battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_STATE)]);
    }
#endif
}

void pmu_charger_test(void)
{
#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
    if (battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_STATE) == CHARGER_STATE_CHR_OFF) {
        //LOG_MSGID_I(common, "Charger_disable", 0);
        pmu_enable_charger(0);
    } else {
        //LOG_MSGID_I(common, "Charger_enable", 0);
        pmu_enable_charger(1);
    }
#endif
}

int ctoi(char s[])
{
    int i = 0;
    int n = 0;
    for (; (s[i] >= '0' && s[i] <= '9'); ++i) {
        n = 10 * n + (s[i] - '0');
    }
    return n;
}

// change the data with hex type in string to data with dec type
int htoi(char s[])
{
    int i;
    int n = 0;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        i = 2;
    } else {
        i = 0;
    }
    for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z'); ++i) {
        if (tolower((int)s[i]) > '9') {
            n = 16 * n + (10 + tolower((int)s[i]) - 'a');
        } else {
            n = 16 * n + (tolower((int)s[i]) - '0');
        }
    }
    return n;
}

static void charger_task(void *pvParameters)
{
#ifdef MTK_BATTERY_MANAGEMENT_ENABLE

    for (;;) {
        const TickType_t xDelay = task_timer / portTICK_PERIOD_MS;
        vTaskDelay(xDelay);
        switch (pmu_debug_index) {
            case 0:
                pmu_dump_battery();
                break;
            case 1:
                pmu_dump_auxadc();
                break;
            case 2:
                pmu_charger_test();
                break;
        }
    }
#endif
}

atci_status_t atci_cmd_hdlr_charger(atci_parse_cmd_param_t *parse_cmd)
{
    char *str  = NULL;
    atci_response_t *response = atci_mem_alloc(sizeof(atci_response_t));
    if(NULL == response) {
        return ATCI_STATUS_ERROR;
    }
    memset(response, 0, sizeof(atci_response_t));

    atci_response_heavy_data_t dumpall_response;
    uint8_t str_size = strlen(parse_cmd->string_ptr);
    dumpall_response.response_buf = pvPortMalloc(str_size + 1);
    if (dumpall_response.response_buf == NULL) {
        //LOG_MSGID_I(common, "Command is null", 0);
        atci_mem_free(response);
        return ATCI_STATUS_OK;
    }
    str = (char *)dumpall_response.response_buf;
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_EXECUTION: /* AT+ECHAR=SQC... */
            if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=SQC,", 12) == 0) {
                char *mid_pos = NULL;
                int input_domain = 0;
                int input_value = 0;
                char *end_pos = NULL;
                mid_pos = strchr(parse_cmd->string_ptr, ',');
                mid_pos++;
                end_pos = strchr(mid_pos, ',');
                if ((strlen(mid_pos) - strlen(end_pos)) < str_size) {
                    memcpy(str, mid_pos, strlen(mid_pos) - strlen(end_pos));
                    input_domain = (int)strtol(mid_pos, NULL, 0);
                    end_pos++;
                    input_value = (int)strtol(end_pos, NULL, 0);
                    task_timer = ctoi(end_pos);
                    mid_pos = NULL;
                    end_pos = NULL;
                    //LOG_MSGID_I(common, "input_value1:%d, input_value2:%d\r\n", 2, input_domain, input_value);
                    switch (input_domain) {
                        case 0:
                            vTaskDelete(charger_task_handle);
                            charger_task_handle = NULL;
                            count = 0;
                            break;

                        case 1:
                            pmu_debug_index = 0;
                            if (charger_task_handle == NULL) {
                                xTaskCreate(charger_task, "charger_task", 512, (void *) &input_value, tskIDLE_PRIORITY, &charger_task_handle);
                            }
                            break;

                        case 2:
                            vTaskSuspend(charger_task_handle);
                            break;

                        case 3:
                            vTaskResume(charger_task_handle);
                            break;
                        case 4:
                            pmu_debug_index = 1;
                            if (charger_task_handle == NULL) {
                                xTaskCreate(charger_task, "charger_task", 512, (void *) &input_value, tskIDLE_PRIORITY, &charger_task_handle);
                            }
                            break;
                        case 5:
                            pmu_debug_index = 2;
                            if (charger_task_handle == NULL) {
                                xTaskCreate(charger_task, "charger_task", 512, (void *) &input_value, tskIDLE_PRIORITY, &charger_task_handle);
                            }
                            break;
                    }
                } else {
                    //LOG_MSGID_I(common, "command lengh error", 0);
                }
            } else if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=ICL,", 12) == 0) {
#ifdef AIR_BTA_PMIC_HP
                int input_addr = 0;
                char *end_pos = NULL;
                end_pos = strchr(parse_cmd->string_ptr, ',');
                end_pos++;
                input_addr = (int)strtol(end_pos, NULL, 0);
                end_pos = NULL;
                //LOG_MSGID_I(common, "Input leve : %d\r\n", 1, input_addr);
                pmu_set_charger_current_limit(input_addr);
#endif
            }
#ifdef MTK_FUEL_GAUGE
            else if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=FG_DEPUTY_TEMP,", 23) == 0) {
                            char *mid_pos = NULL;
                            int input_domain = 0;
                            int input_value = 0;
                            char *end_pos = NULL;
                            mid_pos = strchr(parse_cmd->string_ptr, ',');
                            mid_pos++;
                            end_pos = strchr(mid_pos, ',');
                            if ((strlen(mid_pos) - strlen(end_pos)) < str_size) {
                                memcpy(str, mid_pos, strlen(mid_pos) - strlen(end_pos));
                                input_domain = ctoi(mid_pos);
                                end_pos++;
                                input_value = ctoi(end_pos);
                                mid_pos = NULL;
                                end_pos = NULL;
                                //LOG_MSGID_I(common, "input_value1:%d, input_value2:%d\r\n", 2, input_domain, input_value);
#ifdef FUEL_GAUGE_DEPUTY_TABLE_ENABLE
                                battery_set_deputy_temp(input_domain,input_value);
#endif
                            }
            }
            else if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=FG_DUMP,", 12) == 0) {
                            int input_addr = 0;
                            char *end_pos = NULL;
                            end_pos = strchr(parse_cmd->string_ptr, ',');
                            end_pos++;
                            input_addr = (int)strtol(end_pos, NULL, 0);
                            end_pos = NULL;
                            //LOG_MSGID_I(common, "AT+ECHAR=FG_DUMP\r\n", 1, input_addr);
                            switch(input_addr){
                            case 0:
                                battery_dump_zcv_table();
                                break;
                            }
            }
#endif
            else if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=DEBUG,", 12) == 0) {
                int input_addr = 0;
                char *end_pos = NULL;
                end_pos = strchr(parse_cmd->string_ptr, ',');
                end_pos++;
                input_addr = (int)strtol(end_pos, NULL, 0);
                end_pos = NULL;
                LOG_MSGID_I(common, "Input leve : %d\r\n", 1, input_addr);
            }
#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
            else if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=CHARGER,", 12) == 0) {
                char *end_pos = NULL;
                int input_value = 0;
                end_pos = strchr(parse_cmd->string_ptr, ',');
                end_pos++;
                input_value = (int)strtol(end_pos, NULL, 0);
                end_pos = NULL;
                //log_hal_msgid_info("Setting : %d\r\n", 1, (int) input_value);

                battery_set_enable_charger(input_value);
                if (input_value) {
                    snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "Charger Enable [%d][%s]\r\n", input_value,
                             charger_state[battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_STATE)]);
                } else {
                    snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "Charger Disable [%d][%s]\r\n", input_value,
                             charger_state[battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_STATE)]);
                }
            } else if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=STA", 12) == 0) {
                snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "STA:%s\r\n", charger_state[battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_STATE)]);
            } else if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=SOC", 12) == 0) {
                snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "SOC:%d\r\n", (int) battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY));
            } else if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=VBAT", 12) == 0) {
                snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "VBAT:%d\r\n", (int) battery_management_get_battery_property(BATTERY_PROPERTY_VOLTAGE));
            } else if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=CHG_OPTION,", 12) == 0) {
#ifdef AIR_BTA_PMIC_HP
                int input_addr = 0;
                char *end_pos = NULL;
                end_pos = strchr(parse_cmd->string_ptr, ',');
                end_pos++;
                input_addr = (int)strtol(end_pos, NULL, 0);
                end_pos = NULL;
                //LOG_MSGID_I(common, "Input leve : %d\r\n", 1, input_addr);
                battery_switch_charger_option(input_addr);
#endif
            }
#endif
#if defined(AIR_BTA_IC_PREMIUM_G3_TYPE_P)
            else if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=SET,", strlen("AT+ECHAR=SET,")) == 0) {
                int input_value = 0;
                char *mid_pos = NULL;
                char *end_pos = NULL;
                mid_pos = strchr(parse_cmd->string_ptr, ',');
                mid_pos++;

                if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=SET,CC,", strlen("AT+ECHAR=SET,CC,")) == 0) {
                    end_pos = strchr(mid_pos, ',');
                    end_pos++;
                    input_value = ctoi(end_pos);
                    mid_pos = NULL;
                    end_pos = NULL;
                    pmu_chg_set_cc_current(input_value);
                } else if(strncmp(parse_cmd->string_ptr, "AT+ECHAR=SET,CC2,", strlen("AT+ECHAR=SET,CC2,")) == 0){
                    end_pos = strchr(mid_pos, ',');
                    end_pos++;
                    input_value = ctoi(end_pos);
                    mid_pos = NULL;
                    end_pos = NULL;
                    pmu_chg_set_cc2_ctrl_en(1);
                    pmu_chg_set_cc2_current(input_value);
                }else {
                    snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "error charger cmd\r\n");
                    break;
                }
            }
#endif
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
            else if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=GET,PRECC", strlen("AT+ECHAR=GET,PRECC")) == 0) {
                snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "PRECC_CURR:%d (mA)\r\n", (int) pmu_get_chg_setting(PMU_PRECC_SETTING));
            }
            else if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=GET,CC", strlen("AT+ECHAR=GET,CC")) == 0) {
                snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "CC_CURR:%d (mA)\r\n", (int) pmu_get_chg_setting(PMU_CC_SETTING));
            }
            else if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=GET,ITERM", strlen("AT+ECHAR=GET,ITERM")) == 0) {
                snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "ITERM_RATIO:%d (%%)\r\n", (int) pmu_get_chg_setting(PMU_ITERM_SETTING));
            }
            else if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=GET,CV", strlen("AT+ECHAR=GET,CV")) == 0) {
                snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "CV_VOLTAGE:%d (mV)\r\n", (int) pmu_get_chg_setting(PMU_CV_SETTING));
            }
            else if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=GET,PRE_ITERM", strlen("AT+ECHAR=GET,PRE_ITERM")) == 0) {
                snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "PRE_ITERM_RATIO:%d (%%)\r\n", (int) pmu_get_chg_setting(PMU_PRE_ITERM_SETTING));
            }
            else if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=SET,", strlen("AT+ECHAR=SET,")) == 0) {
                int input_value = 0;
                char *mid_pos = NULL;
                char *end_pos = NULL;
                mid_pos = strchr(parse_cmd->string_ptr, ',');
                mid_pos++;
                //dac condition is determined by customer battery
                //pmu_chg_set_bat_dac(BATTERY_VOLTAGE);
                //stop get pure vbat task
                vTaskSuspend(battery_lineat_task_t);

                    //pmu_chg_force_state(CHG_FORCE_TRICKLE);
                if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=SET,STATE,", strlen("AT+ECHAR=SET,STATE,")) == 0) {
                    end_pos = strchr(mid_pos, ',');
                    end_pos++;
                    input_value = ctoi(end_pos);
                    mid_pos = NULL;
                    end_pos = NULL;
                    pmu_chg_force_state(input_value);
                } else if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=SET,PRECC,", strlen("AT+ECHAR=SET,PRECC,")) == 0) {
                    end_pos = strchr(mid_pos, ',');
                    end_pos++;
                    input_value = ctoi(end_pos);
                    mid_pos = NULL;
                    end_pos = NULL;
                    pmu_chg_set_precc_current(input_value);
                } else if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=SET,CC_THD,", strlen("AT+ECHAR=SET,CC_THD,")) == 0) {
                    end_pos = strchr(mid_pos, ',');
                    end_pos++;
                    input_value = ctoi(end_pos);
                    mid_pos = NULL;
                    end_pos = NULL;
                    pmu_chg_set_cc1_threshold(input_value);
                } else if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=SET,CC,", strlen("AT+ECHAR=SET,CC,")) == 0) {
                    end_pos = strchr(mid_pos, ',');
                    end_pos++;
                    input_value = ctoi(end_pos);
                    mid_pos = NULL;
                    end_pos = NULL;
                    pmu_chg_set_cc1_current(input_value);
                } else if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=SET,CV,", strlen("AT+ECHAR=SET,CV,")) == 0) {
                    end_pos = strchr(mid_pos, ',');
                    end_pos++;
                    input_value = ctoi(end_pos);
                    mid_pos = NULL;
                    end_pos = NULL;
                    pmu_chg_set_cv_threshold(input_value);
                } else if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=SET,BAT,", strlen("AT+ECHAR=SET,BAT,")) == 0) {
                    end_pos = strchr(mid_pos, ',');
                    end_pos++;
                    input_value = ctoi(end_pos);
                    mid_pos = NULL;
                    end_pos = NULL;
                    pmu_chg_set_bat_dac(input_value);
                } else if (strncmp(parse_cmd->string_ptr, "AT+ECHAR=SET,ITERM,", strlen("AT+ECHAR=SET,ITERM,")) == 0) {
                    end_pos = strchr(mid_pos, ',');
                    end_pos++;
                    input_value = ctoi(end_pos);
                    mid_pos = NULL;
                    end_pos = NULL;
                    pmu_chg_set_iterm_current(input_value);
                } else {
                    vTaskResume(battery_lineat_task_t);
                    snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "error charger cmd\r\n");
                    break;
                }
            }
#endif
            else {
                response->response_flag = 0 | ATCI_RESPONSE_FLAG_APPEND_ERROR;
                atci_send_response(response);
                break;
            }
            response->response_flag = 0 | ATCI_RESPONSE_FLAG_APPEND_OK;
            response->response_len = strlen((const char *) response->response_buf);
            atci_send_response(response);
            break;
    }
    vPortFree(dumpall_response.response_buf);
    atci_mem_free(response);
    return ATCI_STATUS_OK;
}


#endif /* HAL_CHARGER_MODULE_ENABLED */


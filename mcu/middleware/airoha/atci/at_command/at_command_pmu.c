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
#include "hal_feature_config.h"
#ifdef HAL_PMU_MODULE_ENABLED
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include "at_command.h"
#include "hal.h"
#include "hal_pmu.h"
#include "FreeRTOS.h"
#define STRNCPY_PMU(dest, source) strncpy(dest, source, strlen(source)+1);
#if defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
#ifdef AIR_BTA_PMIC_HP
#include "hal_pmu_hp_platform.h"
#include "hal_pmu_charger_hp.h"
#include "hal_pmu_internal_hp.h"
#include "hal_pmu_auxadc_hp.h"
#elif defined(AIR_BTA_PMIC_LP)
#include "hal_pmu_lp_platform.h"
#include "hal_pmu_charger_lp.h"
#include "hal_pmu_internal_lp.h"
#include "hal_pmu_auxadc_lp.h"
#endif /* HP or LP */
#endif /* AIR_BTA_IC_PREMIUM_G3 */
void pmu_pk_callback(void)
{
    log_hal_msgid_info("[pmu_pk_callback]\r\n", 0);
}

/*--- Function ---*/
atci_status_t atci_cmd_hdlr_pmu(atci_parse_cmd_param_t *parse_cmd);
#if defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
static int ctoi(char s[])
{
    int i = 0;
    int n = 0;
    for (; (s[i] >= '0' && s[i] <= '9'); ++i) {
        n = 10 * n + (s[i] - '0');
    }
    return n;
}
#endif
static int htoi(char s[]);
/*
 AT+EPMUREG=<op>                |   "OK"
 AT+EPMUREG=?                |   "+EPMUREG=(0,1)","OK"
 */
// AT command handler
atci_status_t atci_cmd_hdlr_pmu(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = atci_mem_alloc(sizeof(atci_response_t));
    if(NULL == response) {
        return ATCI_STATUS_ERROR;
    }
    memset(response, 0, sizeof(atci_response_t));

    uint16_t read_value = 0;
    int input_addr = 0;
    int input_value = 0;
    char *end_pos = NULL;
    response->response_flag = 0; // Command Execute Finish.
#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    response->cmd_id = parse_cmd->cmd_id;
#endif
    atci_response_heavy_data_t dumpall_response;
    uint8_t str_size = strlen(parse_cmd->string_ptr); /*0d/0a*/
    dumpall_response.response_buf = pvPortMalloc(str_size + 1);
    if (dumpall_response.response_buf == NULL) {
        //LOG_MSGID_I(common, "Command is null", 0);
        atci_mem_free(response);
        return ATCI_STATUS_OK;
    }
    char *str  = NULL;
    str = (char *)dumpall_response.response_buf;
    switch (parse_cmd->mode) {
        case ATCI_CMD_MODE_TESTING:    // rec: AT+EPMUREG=?
            STRNCPY_PMU((char *) response->response_buf, "+EPMUREG=(0,1)\r\nOK\r\n");

            response->response_len = strlen((const char *) response->response_buf);
            atci_send_response(response);
            break;
        case ATCI_CMD_MODE_EXECUTION: // rec: AT+EPMUREG=<op>  the handler need to parse the parameters

            if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=0,", strlen("AT+EPMUREG=0,")) == 0) {
                /*command: AT+EPMUREG=0,1234*/
                end_pos = strchr(parse_cmd->string_ptr, ',');
                end_pos++;
                input_addr = htoi(end_pos);
                end_pos = NULL;
                /* read data of input register address */
                read_value = pmu_get_register_value(input_addr, 0xFFFF, 0);
                snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "+EPMUREG:0x%x,0x%x\r\n", input_addr, read_value);
                /* ATCI will help append "OK" at the end of response buffer  */
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                response->response_len = strlen((const char *) response->response_buf);
            } else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=1,", strlen("AT+EPMUREG=1,")) == 0) {
                /*command: AT+EPMUREG=1,1234,456*/
                char *mid_pos = NULL;
                mid_pos = strchr(parse_cmd->string_ptr, ',');
                mid_pos++;
                end_pos = strchr(mid_pos, ',');
                if ((strlen(mid_pos) - strlen(end_pos)) < str_size) {
                    memcpy(str, mid_pos, strlen(mid_pos) - strlen(end_pos));
                    input_addr = htoi(mid_pos);
                    end_pos++;
                    input_value = htoi(end_pos);
                    mid_pos = NULL;
                    end_pos = NULL;
                    /* write input data to input register address*/
                    pmu_set_register_value(input_addr, 0xFFFF, 0, input_value);
                } else {
                    //LOG_MSGID_I(common, "command lengh error", 0);
                }
                /* ATCI will help append "OK" at the end of response buffer */
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                response->response_len = strlen((const char *) response->response_buf);
            } else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=AUX,", strlen("AT+EPMUREG=AUX,")) == 0) {
#if (defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)) && !defined(AIR_BTA_IC_PREMIUM_G3_TYPE_D)
                int input_addr = 0;
                char *end_pos = NULL;
                end_pos = strchr(parse_cmd->string_ptr, ',');
                end_pos++;
                input_addr = htoi(end_pos);
                end_pos = NULL;
                //log_hal_msgid_info("Input channel : %d\r\n", 1, (int)input_addr);
#ifdef AIR_BTA_PMIC_HP
                uint32_t temp = pmu_auxadc_get_channel_value(input_addr);
                //log_hal_msgid_info("Index :%d : %lx", 2, (int)input_addr, temp);
                snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "[Channel-%d = %d][adc:%x]\r\n", input_addr, (unsigned int)temp, (unsigned int)temp);
#elif defined AIR_BTA_PMIC_LP
                uint32_t temp = pmu_auxadc_get_channel_value(input_addr);
    #if defined(AIR_BTA_IC_STEREO_HIGH_G3)
                if ( (int)input_addr == 1){
                    snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "[Channel-%d = %d][volt:%d]\r\n", input_addr, (unsigned int)temp, (unsigned int)temp);
                } else if ((int)input_addr == 2){
                    snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "[Channel-%d = %d][volt:%d]\r\n", input_addr, (unsigned int)temp, (unsigned int)temp);
        #if defined(PMIC_STEREO_HIGH_G3_LP_TYPE_C)
                } else if ((int)input_addr == 5){
#ifndef AIR_PMU_DISABLE_CHARGER
                    snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "[Channel-%d = %d][temperature:%ld]\r\n", input_addr, (unsigned int)temp, pmu_jeita_adc_to_temp((unsigned int)temp));
#endif
        #endif
                }else {
                    snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "[Channel-%d = %d][adc:%x]\r\n", input_addr, (unsigned int)temp, (unsigned int)temp);
                }
    #else
                if ( (int)input_addr == 3 || (int)input_addr == 2){
                    snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "[Channel-%d = %d][volt:%x]\r\n", input_addr, (unsigned int)temp, (unsigned int)temp);
                } else {
                    snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "[Channel-%d = %d][adc:%x]\r\n", input_addr, (unsigned int)temp, (unsigned int)temp);
                }
    #endif
#endif
#endif /* AIR_BTA_IC_PREMIUM_G3 || AIR_BTA_IC_STEREO_HIGH_G3*/
                /* ATCI will help append "OK" at the end of response buffer  */
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                response->response_len = strlen((const char *) response->response_buf);
            } else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=PWR,", strlen("AT+EPMUREG=PWR,")) == 0) {
                end_pos = strchr(parse_cmd->string_ptr, ',');
                end_pos++;
                input_addr = htoi(end_pos);
                end_pos = NULL;
                if (input_addr == 0) {
                    pmu_power_off_sequence(PMU_PWROFF);
                } else {
                    pmu_power_off_sequence(PMU_RTC);
                }
                /* ATCI will help append "OK" at the end of response buffer  */
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                response->response_len = strlen((const char *) response->response_buf);
            } else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=DUMMY,DISABLE", strlen("AT+EPMUREG=DUMMY,DISABLE")) == 0) {
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
                pmu_disable_dummy_load();
#endif
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                response->response_len = strlen((const char *) response->response_buf);
            } else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=DUMMY,", strlen("AT+EPMUREG=DUMMY,")) == 0) {
#if defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
                char *mid_pos = NULL;
                int input_domain = 0;
                int input_value = 0;
                mid_pos = strchr(parse_cmd->string_ptr, ',');
                mid_pos++;
                end_pos = strchr(mid_pos, ',');
                if ((strlen(mid_pos) - strlen(end_pos)) < str_size) {
                    memcpy(str, mid_pos, strlen(mid_pos) - strlen(end_pos));
                    input_domain = htoi(mid_pos);
                    end_pos++;
                    input_value = ctoi(end_pos);
                    mid_pos = NULL;
                    end_pos = NULL;
                    //log_hal_msgid_info("input_domain :%d input_value :%d", 2, (int)input_domain, (int)input_value);
                    pmu_set_dummy_load(input_domain, input_value);
                } else {
                    //LOG_MSGID_I(common, "command lengh error", 0);
                }
#endif
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                response->response_len = strlen((const char *) response->response_buf);
            } else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=RIPPLE,", strlen("AT+EPMUREG=RIPPLE,")) == 0) {
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
                char *mid_pos = NULL;
                int input_domain = 0;
                int input_value = 0;
                mid_pos = strchr(parse_cmd->string_ptr, ',');
                mid_pos++;
                end_pos = strchr(mid_pos, ',');
                if ((strlen(mid_pos) - strlen(end_pos)) < str_size) {
                    memcpy(str, mid_pos, strlen(mid_pos) - strlen(end_pos));
                    input_domain = htoi(mid_pos);
                    end_pos++;
                    input_value = ctoi(end_pos);
                    mid_pos = NULL;
                    end_pos = NULL;
                    //log_hal_msgid_info("input_domain :%d input_value :%d", 2, (int)input_domain, (int)input_value);
                    pmu_set_ripple_control(input_domain, input_value);
                } else {
                    //LOG_MSGID_I(common, "command lengh error", 0);
                }
#endif
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                response->response_len = strlen((const char *) response->response_buf);
            } else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=VAUDOUT,", strlen("AT+EPMUREG=VAUDOUT,")) == 0) {
#if defined(AIR_BTA_IC_PREMIUM_G3)
                char *mid_pos = NULL;
                int input_domain = 0;
                int input_value = 0;
                mid_pos = strchr(parse_cmd->string_ptr, ',');
                mid_pos++;
                end_pos = strchr(mid_pos, ',');
                if ((strlen(mid_pos) - strlen(end_pos)) < str_size) {
                    memcpy(str, mid_pos, strlen(mid_pos) - strlen(end_pos));
                    input_domain = htoi(mid_pos);
                    end_pos++;
                    input_value = ctoi(end_pos);
                    mid_pos = NULL;
                    end_pos = NULL;
                    //log_hal_msgid_info("input_domain :%d input_value :%d", 2, (int)input_domain, (int)input_value);
                    switch(input_domain){
                       case 0:
                           pmu_set_micbias_vout(PMIC_MICBIAS0,input_value);
                       break;
                       case 1:
                           pmu_set_micbias_vout(PMIC_MICBIAS1,input_value);
                       break;
                       case 2:
                           pmu_set_micbias_vout(PMIC_MICBIAS2,input_value);
                       break;
                       case 3:
                           pmu_set_micbias_vout(PMIC_MICBIAS0,input_value);
                           pmu_set_micbias_vout(PMIC_MICBIAS1,input_value);
                           pmu_set_micbias_vout(PMIC_MICBIAS2,input_value);
                       break;
                    }
                } else {
                    //LOG_MSGID_I(common, "command lengh error", 0);
                }
#endif
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                response->response_len = strlen((const char *) response->response_buf);
            }else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=AUDMODE,", strlen("AT+EPMUREG=AUDMODE,")) == 0) {
                end_pos = strchr(parse_cmd->string_ptr, ',');
                end_pos++;
                input_addr = htoi(end_pos);
                end_pos = NULL;
                //log_hal_msgid_info("Audio mode input : %d\r\n", 1, (int)input_addr);
#if defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
                pmu_set_audio_mode(input_addr, PMU_ON);
#endif
                /* ATCI will help append "OK" at the end of response buffer  */
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                response->response_len = strlen((const char *) response->response_buf);
            } else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=MICBIASON,", strlen("AT+EPMUREG=MICBIASON,")) == 0 ||
                       strncmp(parse_cmd->string_ptr, "AT+EPMUREG=MICBIASOFF,", strlen("AT+EPMUREG=MICBIASOFF,")) == 0) {
#if defined(AIR_BTA_IC_PREMIUM_G3)
                char *mid_pos = NULL;
                int input_domain = 0;
                int input_value = 0;
                mid_pos = strchr(parse_cmd->string_ptr, ',');
                mid_pos++;
                end_pos = strchr(mid_pos, ',');
                if ((strlen(mid_pos) - strlen(end_pos)) < str_size) {
                    memcpy(str, mid_pos, strlen(mid_pos) - strlen(end_pos));
                    input_domain = htoi(mid_pos);
                    end_pos++;
                    input_value = htoi(end_pos);
                    mid_pos = NULL;
                    end_pos = NULL;
                    //log_hal_msgid_info("input_domain :%d input_value :%d", 2, (int)input_domain, (int)input_value);
                    if(strncmp(parse_cmd->string_ptr, "AT+EPMUREG=MICBIASON,", strlen("AT+EPMUREG=MICBIASON,")) == 0){
                        pmu_enable_micbias(input_domain, input_value, PMIC_MICBIAS_NM, PMU_ON);
                    }else{
                    pmu_enable_micbias(input_domain, input_value, PMIC_MICBIAS_NM, PMU_OFF);
                    }
                } else {
                    //LOG_MSGID_I(common, "command lengh error", 0);
                }
#endif
                /* ATCI will help append "OK" at the end of response buffer  */
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                response->response_len = strlen((const char *) response->response_buf);
            } else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=LOCK,", strlen("AT+EPMUREG=LOCK,")) == 0) {
                char *mid_pos = NULL;
                int input_domain = 0;
                int input_value = 0;
                mid_pos = strchr(parse_cmd->string_ptr, ',');
                mid_pos++;
                end_pos = strchr(mid_pos, ',');
                if ((strlen(mid_pos) - strlen(end_pos)) < str_size) {
                    memcpy(str, mid_pos, strlen(mid_pos) - strlen(end_pos));
                    input_domain = htoi(mid_pos);
                    end_pos++;
                    input_value = htoi(end_pos);
                    mid_pos = NULL;
                    end_pos = NULL;
                    //log_hal_msgid_info("input_domain :%d input_value :%d", 2, (int)input_domain, (int)input_value);
                    switch (input_domain) {
                        case 0:
                            pmu_lock_vcore(PMU_DVS, input_value, PMU_UNLOCK);
                            break;
                        case 1:
                            pmu_lock_vcore(PMU_DVS, input_value, PMU_LOCK);
                            break;
                        case 2:
#ifndef AIR_BTA_IC_PREMIUM_G3_TYPE_D
                            pmu_dynamic_debug.val = input_value;
                            snprintf((char *)response->response_buf, sizeof(response->response_buf), "pmu_dynamic_debug 0x%02X, vcore %d, other %d\r\n", pmu_dynamic_debug.val, pmu_dynamic_debug.b.vcore, pmu_dynamic_debug.b.other);
#endif
                            break;
                    }
                } else {
                    //LOG_MSGID_I(common, "command lengh error", 0);
                }
                /* ATCI will help append "OK" at the end of response buffer  */
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                response->response_len = strlen((const char *) response->response_buf);
            } else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=DEBUG,", strlen("AT+EPMUREG=DEBUG,")) == 0) {
                end_pos = strchr(parse_cmd->string_ptr, ',');
                end_pos++;
                input_addr = htoi(end_pos);
                end_pos = NULL;
                switch (input_addr) {
#if defined(AIR_BTA_IC_PREMIUM_G3)
                    case 0:
                        pmu_dump_rg();
                        break;
                    case 1:
#ifdef AIR_BTA_PMIC_HP
                        pmu_register_callback(RG_INT_PWRKEY, pmu_pk_callback, NULL);
#endif
                        break;
                    case 2:
                        pmu_dump_otp();
                        break;
                    case 3:
                        pmu_dump_nvkey();
                        break;
#endif /* AIR_BTA_IC_PREMIUM_G3 */
                }
                /* ATCI will help append "OK" at the end of response buffer  */
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                response->response_len = strlen((const char *) response->response_buf);
            } else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=RG,", strlen("AT+EPMUREG=RG,")) == 0) {
#if defined(AIR_BTA_IC_PREMIUM_G3)
                uint16_t start = 0, end = 0, timer = 0;
                end_pos = strchr(parse_cmd->string_ptr, ',');
                end_pos++;
                start = htoi(end_pos); // starting range of RG
                end_pos = strchr(end_pos, ',');
                end_pos++;
                end = htoi(end_pos); // end range of RG
                end_pos = strchr(end_pos, ',');
                end_pos++;
                timer = ctoi(end_pos); // RG check timer, 0 = check once
                end_pos = NULL;
                pmu_check_rg_timer(start, end, timer);
#endif
                /* ATCI will help append "OK" at the end of response buffer  */
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                response->response_len = strlen((const char *) response->response_buf);
            } else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=VBUS_DEBOUNCE,", strlen("AT+EPMUREG=VBUS_DEBOUNCE,")) == 0) {
                end_pos = strchr(parse_cmd->string_ptr, ',');
                end_pos++;
                input_addr = htoi(end_pos);
                end_pos = NULL;
                //LOG_MSGID_I(common, "input_addr:", 1, input_addr);
#ifdef AIR_BTA_PMIC_HP
                pmu_set_vbus_debounce_time_hp(input_addr);
#endif
                /* ATCI will help append "OK" at the end of response buffer  */
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                response->response_len = strlen((const char *) response->response_buf);
            } else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=WOHOLD,", strlen("AT+EPMUREG=WOHOLD,")) == 0) {
                end_pos = strchr(parse_cmd->string_ptr, ',');
                end_pos++;
                input_addr = htoi(end_pos);
                end_pos = NULL;
                //LOG_MSGID_I(common, "input_addr:", 1, input_addr);
#ifdef AIR_BTA_PMIC_HP
                pmu_enable_powerhold_hp(input_addr);
#endif
                /* ATCI will help append "OK" at the end of response buffer  */
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                response->response_len = strlen((const char *) response->response_buf);
            } else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=LPSD,", strlen("AT+EPMUREG=LPSD,")) == 0) {
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
                end_pos = strchr(parse_cmd->string_ptr, ',');
                end_pos++;
                input_addr = htoi(end_pos);
                end_pos = NULL;
                //LPSD w/wo autoreset
                if(input_addr == 0){
                    pmu_regen_lpsd_type_sel_lp(PMU_REGEN_LPSD_AUTO_REBOOT);
                }else if(input_addr == 1){
                    pmu_regen_lpsd_type_sel_lp(PMU_REGEN_LPSD_NO_AUTO_REBOOT);
                }
#endif
                /* ATCI will help append "OK" at the end of response buffer  */
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                response->response_len = strlen((const char *) response->response_buf);
            } else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=PDM,", strlen("AT+EPMUREG=PDM,")) == 0) {
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
                char *mid_pos = NULL;
                int input_domain = 0;
                int input_value = 0;
                mid_pos = strchr(parse_cmd->string_ptr, ',');
                mid_pos++;
                input_domain = htoi(mid_pos);
                end_pos = strchr(mid_pos, ',');
                end_pos++;
                input_value = htoi(end_pos);
                end_pos = NULL;
                //pmu enable power
                pmu_enable_power(input_domain, input_value);
#endif
                /* ATCI will help append "OK" at the end of response buffer  */
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                response->response_len = strlen((const char *) response->response_buf);
            }  else if (strncmp(parse_cmd->string_ptr, "AT+EPMUREG=SWITCH_VAUD,", strlen("AT+EPMUREG=SWITCH_VAUD,")) == 0) {
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
                end_pos = strchr(parse_cmd->string_ptr, ',');
                end_pos++;
                input_addr = htoi(end_pos);
                end_pos = NULL;
                pmu_set_audio_mode(PMU_CLASSG2, PMU_ON);
                //ENTER VAUD18 SW MODE CONTROL -> 0:0.9V  1:1.8V
                pmu_force_vaud18_vosel(input_addr);
#endif
                /* ATCI will help append "OK" at the end of response buffer  */
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                response->response_len = strlen((const char *) response->response_buf);
            } else if(strncmp(parse_cmd->string_ptr, "AT+EPMUREG=PON_REASON,", strlen("AT+EPMUREG=PON_REASON")) == 0){
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
                extern uint16_t power_on_reason;
                if(power_on_reason & BOOT_MODE){
                    if(power_on_reason & PMU_KEY){
                        snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "Power on reason: REGEN_PON\r\n");
                    }
                    if(power_on_reason & PMU_CHRIN){
                        snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "Power on reason: CHG_PON\r\n");
                    }
                }else{
                    if(power_on_reason & PMU_KEY){
                        snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "Power on reason: REGEN_ALARM\r\n");
                    }
                    if(power_on_reason & PMU_CHRIN){
                        snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "Power on reason: CHG_ALARM\r\n");
                    }
                    if(power_on_reason & PMU_RTCA){
                        snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "Power on reason: RTC_ALARM\r\n");
                    }
                    if(power_on_reason == 0){
                        snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "Power on reason: None. WDT_RST or SYS_RST\r\n");
                    }
                }
#endif
                /* ATCI will help append "OK" at the end of response buffer  */
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                response->response_len = strlen((const char *) response->response_buf);
            } else if(strncmp(parse_cmd->string_ptr, "AT+EPMUREG=POFF_REASON,", strlen("AT+EPMUREG=POFF_REASON")) == 0){
#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
                uint16_t reason = pmu_get_power_off_reason();
                char *poff_reason;
                if(reason == 1){
                    poff_reason = "RTC_MODE";
                }else if(reason == 14){
                    poff_reason = "CAP_LPSD";
                }else if(reason == 10){
                    poff_reason = "REGEN_LPSD";
                }else if(reason == 13){
                    poff_reason = "SYS_RST";
                }else if(reason == 8){
                    poff_reason = "WD_RST";
                }else if(reason == 15){
                    poff_reason = "VBUS_PAT_RST";
                }else if(reason == 0){
                    poff_reason = "OFF_MODE";
                }else{
                    poff_reason = "null";
                }
                snprintf((char *) response->response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "Power off reason: %s\r\n", poff_reason);
#endif
                /* ATCI will help append "OK" at the end of response buffer  */
                response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
                response->response_len = strlen((const char *) response->response_buf);
            }else {
                /*invalide parameter, return "ERROR"*/
                STRNCPY_PMU((char *) response->response_buf, "ERROR\r\n");
                response->response_len = strlen((const char *) response->response_buf);
            }
            atci_send_response(response);

            break;
        default:
            /* others are invalid command format */
            STRNCPY_PMU((char *) response->response_buf, "ERROR\r\n");
            response->response_len = strlen((const char *) response->response_buf);
            atci_send_response(response);
            break;
    }
    vPortFree(dumpall_response.response_buf);
    atci_mem_free(response);
    return ATCI_STATUS_OK;
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
#endif


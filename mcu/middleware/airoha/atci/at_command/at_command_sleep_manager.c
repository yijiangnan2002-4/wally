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

#if (PRODUCT_VERSION == 2523 || PRODUCT_VERSION == 2533 )
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include "at_command.h"
#include "syslog.h"
#include "hal_cm4_topsm.h"
#include "hal_sys_topsm.h"
#include "hal_bsi.h"
#include "hal_sleep_manager.h"
#include "hal_pmu_internal.h"
#include "hal_dsp_topsm.h"
#include "hal_gpt.h"
#include "hal_clock.h"
#include "hal_clock_internal.h"
#include "hal_sleep_driver.h"
#include "hal_nvic_internal.h"
#include "hal_pmu.h"
#include "memory_attribute.h"
#include "hal_feature_config.h"
#include <ctype.h>
#include "hal_nvic.h"
#include "hal_dvfs.h"
#include "timers.h"

#ifdef MTK_SMART_BATTERY_ENABLE
#include "battery_management.h"
#endif
#ifdef HAL_SLEEP_MANAGER_ENABLED
extern atci_status_t atci_cmd_hdlr_sleep_manager(atci_parse_cmd_param_t *parse_cmd);
#endif
ATTR_RWDATA_IN_TCM extern bool FAST_WAKEUP_VERIFICATION;
extern uint8_t sleep_manager_handle;

#ifdef MTK_DEMO_PROJ_LOWPOWER_AT_CMD_ENABLE
#include "mt25x3_hdk_backlight.h"
#include "mt25x3_hdk_lcd.h"
#include "task_def.h"
#endif

char *end_pos = NULL;
int input_value = 0;
int input_1 = 0;
int input_2 = 0;
char *mid_pos = NULL;
char str[20] = { 0 };

extern void mtcmos_bt(void);
extern void mtcmos_mm(void);
extern void mtcmos_md2g(void);

ATTR_RWDATA_IN_TCM extern bool FAST_WAKEUP_VERIFICATION;

extern uint8_t sleep_manager_handle;
#define LOGE(fmt,arg...)   LOG_E(atcmd, "ATCMD: "fmt,##arg)
#define LOGW(fmt,arg...)   LOG_W(atcmd, "ATCMD: "fmt,##arg)
#define LOGI(fmt,arg...)   LOG_I(atcmd ,"ATCMD: "fmt,##arg)

#define LOGMSGIDE(fmt,cnt,arg...)   LOG_MSGID_E(atcmd ,"ATCMD: "fmt,cnt,##arg)
#define LOGMSGIDW(fmt,cnt,arg...)   LOG_MSGID_W(atcmd ,"ATCMD: "fmt,cnt,##arg)
#define LOGMSGIDI(fmt,cnt,arg...)   LOG_MSGID_I(atcmd ,"ATCMD: "fmt,cnt,##arg)

static TimerHandle_t xTimerofTest = NULL;

void at_power_off_task(TimerHandle_t pxTimer)
{
    hal_sleep_manager_enter_power_off_mode();
}

void at_force_to_sleep(TimerHandle_t pxTimer)
{
    extern sleep_driver_struct sleepdrv;
    sleepdrv.sleep_lock_index = 0;
}

/*--- Function ---*/
atci_status_t atci_cmd_hdlr_sleep_manager(atci_parse_cmd_param_t *parse_cmd);

int sm_htoi(char s[])
{
    int i;
    int n = 0;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        i = 2;
    } else {
        i = 0;
    }
    for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z'); ++i) {
        if (tolower((unsigned char)s[i]) > '9') {
            n = 16 * n + (10 + tolower((unsigned char)s[i]) - 'a');
        } else {
            n = 16 * n + (tolower((unsigned char)s[i]) - '0');
        }
    }
    return n;
}

atci_status_t atci_cmd_hdlr_sleep_manager(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}};
    response.response_flag = 0;
#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    response.cmd_id = parse_cmd->cmd_id;
#endif

    if (strstr((char *) parse_cmd->string_ptr, "AT+SM=0") != NULL) {
        strcpy((char *)response.response_buf, "Test Command Pass!!\r\n");
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=MTCMOS") != NULL) {
        mid_pos = strchr(parse_cmd->string_ptr, ',');
        mid_pos++;
        end_pos = strchr(mid_pos, ',');
        input_1 = sm_htoi(mid_pos);
        end_pos++;
        input_2 = sm_htoi(end_pos);
        mid_pos = NULL;
        end_pos = NULL;
        LOGMSGIDW("[MTCMOS : %d %d ]", 2, input_1, input_2);
        if (input_1 > 2 || input_2 > 1) {
            LOGMSGIDW("[Error Conmand]\n", 0);
            return ATCI_STATUS_OK;
        }
        sys_topsm_mtcmos_control((hal_sys_topsm_mtcmos_enum_t) input_1, input_2);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=SLEEP") != NULL) {
        mid_pos = strchr(parse_cmd->string_ptr, ',');
        mid_pos++;
        end_pos = strchr(mid_pos, ',');
        input_1 = sm_htoi(mid_pos);
        end_pos++;
        input_2 = sm_htoi(end_pos);
        mid_pos = NULL;
        end_pos = NULL;
        switch (input_1) {
            case 1:
                if (input_2 == 1) {
                    sys_topsm_debug_log(HAL_SYS_TOPSM_MTCMOS_DEBUG);
                    cm4_topsm_debug_option(true);
                } else if (input_2 == 0) {
                    cm4_topsm_debug_option(false);
                } else {
                    response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
                    atci_send_response(&response);
                    return ATCI_STATUS_ERROR;
                }
                break;
            case 3:
                sleepdrv_get_lock_module();
                break;
            case 7:
                if (input_2 == 0) {
                    LOGMSGIDW("[BT MTCMOS lock in sleep]\n", 0);
                    cm4_topsm_lock_BT_MTCMOS();
                } else if (input_2 == 1) {
                    LOGMSGIDW("[MM MTCMOS lock in sleep]\n", 0);
                    cm4_topsm_lock_MM_MTCMOS();
                } else if (input_2 == 2) {
                    LOGMSGIDW("[MD2G MTCMOS lock in sleep]\n", 0);
                    cm4_topsm_lock_MD2G_MTCMOS();
                } else if (input_2 == 3) {
                    LOGMSGIDW("[BT MTCMOS unlock in sleep]\n", 0);
                    cm4_topsm_unlock_BT_MTCMOS();
                } else if (input_2 == 4) {
                    LOGMSGIDW("[MM MTCMOS unlock in sleep]\n", 0);
                    cm4_topsm_unlock_MM_MTCMOS();
                } else if (input_2 == 5) {
                    LOGMSGIDW("[MD2G MTCMOS unlock in sleep]\n", 0);
                    cm4_topsm_unlock_MD2G_MTCMOS();
                }
                break;
        }
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=1") != NULL) {
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=1,0") != NULL) {
            LOGMSGIDW("[Eint Setting] \n", 0);
            sleep_driver_fast_wakeup_eint_setting(HAL_GPIO_4, HAL_EINT_NUMBER_3);
        } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=1,1") != NULL) {
            LOGMSGIDW("[EVB Eint Setting] \n", 0);
            sleep_driver_fast_wakeup_eint_setting(HAL_GPIO_3, HAL_EINT_NUMBER_14);
        } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=1,3") != NULL) {
            LOGMSGIDW("[Fast_wakeup]\n", 0);
            FAST_WAKEUP_VERIFICATION = true;
            hal_dvfs_target_cpu_frequency(26000, HAL_DVFS_FREQ_RELATION_L);
            hal_dvfs_target_cpu_frequency(26000, HAL_DVFS_FREQ_RELATION_H);
            *((volatile uint32_t *)(0xA2020C18)) = 0xF0000000;
            *((volatile uint32_t *)(0xA2020004)) = 0x00008000;
            *((volatile uint32_t *)(0xA2020304)) = 0x00008000;  //high
            hal_sleep_manager_unlock_sleep(sleep_manager_handle);
        } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=1,4") != NULL) {
            LOGMSGIDW("[Normail_wakeup] \n", 0);
            FAST_WAKEUP_VERIFICATION = true;
            *((volatile uint32_t *)(0xA2020C18)) = 0xF0000000;
            *((volatile uint32_t *)(0xA2020004)) = 0x00008000;
            *((volatile uint32_t *)(0xA2020304)) = 0x00008000;  //high
            hal_sleep_manager_unlock_sleep(sleep_manager_handle);
        } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=1,5") != NULL) {
            FAST_WAKEUP_VERIFICATION = true;
        } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=1,6") != NULL) {
            xTimerofTest = xTimerCreate("TimerofTest", /* Just a text name, not used by the kernel. */
                                        (5 * 1000 / portTICK_PERIOD_MS), /* The timer period in ticks. */
                                        pdTRUE, /* The timers will auto-reload themselves when they expire. */
                                        NULL, /* Assign each timer a unique id equal to its array index. */
                                        at_force_to_sleep /* Each timer calls the same callback when it expires. */
                                       );
            xTimerStart(xTimerofTest, 0);

        } else {
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&response);
            return ATCI_STATUS_ERROR;
        }
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=3") != NULL) {
        sys_topsm_debug_log(HAL_SYS_TOPSM_MTCMOS_DEBUG);
        response.response_flag = ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=4") != NULL) {
        /*Sleep 15s*/
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=4,0") != NULL) {
            SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
            __asm volatile("cpsid i");
            hal_sleep_manager_set_sleep_time(500000);
            hal_sleep_manager_enter_sleep_mode(HAL_SLEEP_MODE_DEEP_SLEEP);
            __asm volatile("cpsie i");
        } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=4,1") != NULL) {
            SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
            __asm volatile("cpsid i");
            hal_sleep_manager_set_sleep_time(5000);
            hal_sleep_manager_enter_sleep_mode(HAL_SLEEP_MODE_SLEEP);
            __asm volatile("cpsie i");
        } else {
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&response);
            return ATCI_STATUS_ERROR;
        }
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    }  else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=5") != NULL) {
        /*Power off*/
        xTimerofTest = xTimerCreate("TimerofTest",       /* Just a text name, not used by the kernel. */
                                    (3 * 1000 / portTICK_PERIOD_MS),   /* The timer period in ticks. */
                                    pdTRUE,        /* The timers will auto-reload themselves when they expire. */
                                    NULL,   /* Assign each timer a unique id equal to its array index. */
                                    at_power_off_task /* Each timer calls the same callback when it expires. */
                                   );
        xTimerStart(xTimerofTest, 0);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=6") != NULL) {
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=6,0") != NULL) {
            hal_sleep_manager_lock_sleep(sleep_manager_handle);
        } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=6,1") != NULL) {
            hal_sleep_manager_unlock_sleep(sleep_manager_handle);
        } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=6,2") != NULL) {
#ifdef MTK_DEMO_PROJ_LOWPOWER_AT_CMD_ENABLE
#ifdef HAL_DISPLAY_LCD_MODULE_ENABLED
            bsp_lcd_display_off();
            bsp_backlight_deinit();
#endif
            task_def_delete_wo_curr_task();
#endif
        } else {
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&response);
            return ATCI_STATUS_ERROR;
        }
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=8") != NULL) {
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=8,0") != NULL) {
            mtcmos_bt();
        } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=8,1") != NULL) {
            mtcmos_mm();
        } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=8,2") != NULL) {
            mtcmos_md2g();
        } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=8,3") != NULL) {
#ifdef MTK_SMART_BATTERY_ENABLE
            battery_management_deinit();
#endif
        } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=8,4") != NULL) {
            PMIC_VR_CONTROL(PMIC_VA28, PMIC_VR_CTL_DISABLE);
        } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=8,5") != NULL) {
            uint8_t A_sleep_manager_handle;
            A_sleep_manager_handle = hal_sleep_manager_set_sleep_handle("A_sleep_manager_handle_array");
            hal_sleep_manager_lock_sleep(A_sleep_manager_handle);
        } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=8,6") != NULL) {
        } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=8,7") != NULL) {
            sleepdrv_get_lock_module();
        } else {
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&response);
            return ATCI_STATUS_ERROR;
        }
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else {
        strcpy((char *) response.response_buf, "Not Support\n");
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
        atci_send_response(&response);
        return ATCI_STATUS_ERROR;
    }
    return ATCI_STATUS_OK;
}
#elif (PRODUCT_VERSION == 2625)
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "at_command.h"
#include "syslog.h"

#include "hal_sleep_manager.h"
#include "hal_sleep_driver.h"
#include "n1_md_sleep_api.h"
#include "hal_spm.h"
#include "hal_pmu.h"
#include "hal_pmu_internal.h"
#include "nvdm.h"
#include "hal_rtc.h"
#include "hal_rtc_external.h"
#include "hal_clock_internal.h"
#include <stdlib.h>
#ifdef MTK_SWLA_ENABLE
#include "swla.h"
#endif

#ifdef HAL_SLEEP_MANAGER_ENABLED
extern atci_status_t atci_cmd_hdlr_sleep_manager(atci_parse_cmd_param_t *parse_cmd);
#endif

static uint8_t host_ds_lock = 0xff;
static uint8_t host_lock = 0xff;

extern void os_gpt0_pause(void);
extern void os_gpt0_resume(bool update, uint32_t new_compare);
extern void tickless_system_status(void);

extern uint8_t sys_lock_handle;

#define RTC_WAKEUP_DEEP_SLEEP   0

#if RTC_WAKEUP_DEEP_SLEEP
static uint8_t param[7] = {15, /*year*/
                           11, /*month*/
                           06, /*day*/
                           5,  /*day of week*/
                           15, /*hour*/
                           26, /*minute*/
                           15  /*second*/
                          };

static uint8_t alarm_param[7] = {15, /*year*/
                                 11, /*month*/
                                 06, /*day*/
                                 5,  /*day of week*/
                                 15, /*hour*/
                                 26, /*minute*/
                                 20  /*second*/
                                };

/**
*@brief  Clear RTC time structure.
*@param[in] rtc_time: pointer to a hal_rtc_time_t structure that contains the data and time setting that will be set to 0.
*@return none.
*/
static void rtc_clear_time(hal_rtc_time_t *rtc_time)
{
    rtc_time->rtc_year = 0;
    rtc_time->rtc_mon = 0;
    rtc_time->rtc_day = 0;
    rtc_time->rtc_week = 0;
    rtc_time->rtc_hour = 0;
    rtc_time->rtc_min = 0;
    rtc_time->rtc_sec = 0;
}

/**
*@brief  In this function, we fill in RTC time structure by passing in parameters.
*@param[in] rtc_time: pointer to a hal_rtc_time_t structure that contains the data and time setting that will be set.
*@param[in] param[]: the data that will pass to rtc_time setting.
*@return None.
*/

static void rtc_fill_time(hal_rtc_time_t *rtc_time, uint8_t param[])
{
    rtc_time->rtc_year = param[0];
    rtc_time->rtc_mon = param[1];
    rtc_time->rtc_day = param[2];
    rtc_time->rtc_week = param[3];
    rtc_time->rtc_hour = param[4];
    rtc_time->rtc_min = param[5];
    rtc_time->rtc_sec = param[6];
}
#endif

void sm_set_register_value(uint32_t address, uint32_t mask, uint32_t shift, uint32_t value)
{
    uint32_t mask_buffer, target_value;
    mask_buffer = (~(mask << shift));
    target_value = *((volatile uint32_t *)(address));
    target_value &= mask_buffer;
    target_value |= (value << shift);
    *((volatile uint32_t *)(address)) = target_value;
}

atci_status_t atci_cmd_hdlr_sleep_manager(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}};
    response.response_flag = 0;
#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    response.cmd_id = parse_cmd->cmd_id;
#endif
    uint32_t result = 0;
    uint32_t sys_default_lock = false;
    char cmd[256] = {0};
    uint8_t  i = 0;

    strncpy(cmd, (char *)parse_cmd->string_ptr, sizeof(cmd));
    for (i = 0; i < strlen((char *)parse_cmd->string_ptr); i++) {
        cmd[i] = (char)toupper((unsigned char)cmd[i]);
    }

    if (strstr(cmd, "AT+SM=0") != NULL) {
        strcpy((char *)response.response_buf, "Test Command Pass!!\r\n");
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=SHIP") != NULL) {
        __asm volatile("cpsid i");
        spm_control_mtcmos(SPM_MTCMOS_SDIO_SLV, SPM_MTCMOS_PWR_DISABLE);
        spm_control_mtcmos(SPM_MTCMOS_MDSYS, SPM_MTCMOS_PWR_DISABLE);
        pmu_enter_ship_mode();
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=LS") != NULL) {
#ifdef MTK_NBIOT_TARGET_BUILD
        N1MdSleepPowerMdsysPDOff();
#endif
        if (strstr(cmd, "AT+SM=LS,1") != NULL) {
        } else if (strstr(cmd, "AT+SM=LS,2") != NULL) {
            sm_set_register_value(0xA2110250, 0x1, 0, 1);
        }

        __asm volatile("cpsid i");
        os_gpt0_pause();
        spm_control_mtcmos(SPM_MTCMOS_MDSYS, SPM_MTCMOS_PWR_DISABLE);
        spm_control_mtcmos(SPM_MTCMOS_SDIO_SLV, SPM_MTCMOS_PWR_DISABLE);
        pmu_ctrl_power(PMU_VPA, PMU_CTL_DISABLE);
        hal_sleep_manager_enter_sleep_mode(HAL_SLEEP_MODE_LIGHT_SLEEP);

        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=DS") != NULL) {
#if RTC_WAKEUP_DEEP_SLEEP
        hal_rtc_time_t rtc_time;
        hal_rtc_time_t alarm_time;

        rtc_fill_time(&rtc_time, param);
        rtc_fill_time(&alarm_time, alarm_param);
        hal_rtc_set_time(&rtc_time);
        hal_rtc_set_alarm(&alarm_time);
        hal_rtc_enable_alarm();
        //wait milli_sec is 0
        while (1) {
            hal_rtc_get_time(&rtc_time);
            if (rtc_time.rtc_sec == 16) {
                break;
            }
        }
#endif

        if (strstr(cmd, "AT+SM=DS,1") != NULL) {
            hal_rtc_retention_sram_config(0xf, HAL_RTC_SRAM_PD_MODE);
            // need to modify hal_rtc.c if you want to see LDO on current.
            // mask this line: rtc_set_register_value(0xa207080c, 0x1, 0, 0);
        } else if (strstr(cmd, "AT+SM=DS,2") != NULL) {
            hal_rtc_retention_sram_config(0xc, HAL_RTC_SRAM_PD_MODE);
            hal_rtc_retention_sram_config(0x3, HAL_RTC_SRAM_SLEEP_MODE);
        } else if (strstr(cmd, "AT+SM=DS,3") != NULL) {
            hal_rtc_retention_sram_config(0xf, HAL_RTC_SRAM_SLEEP_MODE);
        }

        __asm volatile("cpsid i");
        hal_rtc_enter_retention_mode();
//        hal_sleep_manager_enter_sleep_mode(HAL_SLEEP_MODE_DEEP_SLEEP);
        __asm volatile("cpsie i");
        strcpy((char *)response.response_buf, "Test Command Pass!!\r\n");
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=DP") != NULL) {
        __asm volatile("cpsid i");
        hal_sleep_manager_enter_sleep_mode(HAL_SLEEP_MODE_DEEPER_SLEEP);
        __asm volatile("cpsie i");
        strcpy((char *)response.response_buf, "Test Command Pass!!\r\n");
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=HQA") != NULL) {
#ifdef MTK_NBIOT_TARGET_BUILD
        N1MdSleepPowerMdsysPDOff();
#endif
        __asm volatile("cpsid i");
        os_gpt0_pause();
        //hal_sleep_manager_set_sleep_time(30000000);
        if (strstr(cmd, "AT+SM=HQA,1") != NULL) {
            spm_control_mtcmos(SPM_MTCMOS_SDIO_SLV, SPM_MTCMOS_PWR_DISABLE);
            pmu_ctrl_power(PMU_VPA, PMU_CTL_DISABLE);
            hal_sleep_manager_enter_sleep_mode(HAL_SLEEP_MODE_LIGHT_SLEEP);
        } else if (strstr(cmd, "AT+SM=HQA,2") != NULL) {
            hal_sleep_manager_enter_sleep_mode(HAL_SLEEP_MODE_IDLE);
        } else if (strstr(cmd, "AT+SM=HQA,3") != NULL) {
            hal_clock_set_switch_to_0P9V();
            hal_sleep_manager_enter_sleep_mode(HAL_SLEEP_MODE_IDLE);
        }
        os_gpt0_resume(false, 0);
        __asm volatile("cpsie i");
        strcpy((char *)response.response_buf, "Test Command Pass!!\r\n");
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=BOOT") != NULL) {
        result = rtc_power_on_result_external();
        if (result == 0) {
            strcpy((char *)response.response_buf, "BOOT 0!!\r\n");
        } else if (result == 1) {
            strcpy((char *)response.response_buf, "BOOT 1!!\r\n");
        } else if (result == 1) {
            strcpy((char *)response.response_buf, "BOOT 2!!\r\n");
        }
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=H_DS_L") != NULL) {
        if (host_ds_lock == 0xFF) {
            host_ds_lock = hal_sleep_manager_set_sleep_handle("host_ds_lock");
        }
        hal_sleep_manager_acquire_sleeplock(host_ds_lock, HAL_SLEEP_LOCK_DEEP);

        response.response_len = 0;
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=H_DS_U") != NULL) {
        if (host_ds_lock == 0xFF) {
            host_ds_lock = hal_sleep_manager_set_sleep_handle("host_ds_lock");
        }
        hal_sleep_manager_release_sleeplock(host_ds_lock, HAL_SLEEP_LOCK_DEEP);

        response.response_len = 0;
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=H_L") != NULL) {
        if (host_lock == 0xFF) {
            host_lock = hal_sleep_manager_set_sleep_handle("host_lock");
        }
        hal_sleep_manager_acquire_sleeplock(host_lock, HAL_SLEEP_LOCK_ALL);

        response.response_len = 0;
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=H_U") != NULL) {
        if (host_lock == 0xFF) {
            host_lock = hal_sleep_manager_set_sleep_handle("host_lock");
        }
        hal_sleep_manager_release_sleeplock(host_lock, HAL_SLEEP_LOCK_ALL);

        response.response_len = 0;
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=D_STA") != NULL) {
        uint32_t lock_low;
        uint32_t lock_high;
        sleep_management_get_lock_sleep_info(HAL_SLEEP_LOCK_DEEP, &lock_high, &lock_low);
        snprintf((char *)response.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "Lock status 0x%x-%x\r\n", (unsigned int)lock_high, (unsigned int)lock_low);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=L_STA") != NULL) {
        uint32_t lock_low;
        uint32_t lock_high;
        sleep_management_get_lock_sleep_info(HAL_SLEEP_LOCK_ALL, &lock_high, &lock_low);
        snprintf((char *)response.response_buf, ATCI_UART_TX_FIFO_BUFFER_SIZE, "Lock status 0x%x-%x\r\n", (unsigned int)lock_high, (unsigned int)lock_low);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=CG_V") != NULL) {
        snprintf((char *)response.response_buf, 1024, "\r\n [PDN_COND0]:%lx\r\n [XO_PDN_COND0]:%lx\r\n [0xC0002008]:%lx\r\n", *((volatile uint32_t *)(0xA21D0300)), *((volatile uint32_t *)(0xA2030B00)), *((volatile uint32_t *)(0xC0002008)));
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=ENABLE_VSIM") != NULL) {
        pmu_ctrl_power(PMU_VSIM, PMU_CTL_ENABLE);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=DISABLE_VSIM") != NULL) {
        pmu_ctrl_power(PMU_VSIM, PMU_CTL_DISABLE);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=ENABLE_VFEM") != NULL) {
        pmu_ctrl_power(PMU_VFEM, PMU_CTL_DISABLE);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=DISABLE_VFEM") != NULL) {
        pmu_ctrl_power(PMU_VFEM, PMU_CTL_DISABLE);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
#ifdef MTK_SWLA_ENABLE
    } else if (strstr(cmd, "AT+SM=SWLAS") != NULL) {
        SLA_CustomLogging("SMG", SA_START);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=SWLAE") != NULL) {
        SLA_CustomLogging("SMG", SA_STOP);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
#endif
    } else if (strstr(cmd, "AT+SM=CLOSE_UART0") != NULL) {
        hal_uart_deinit(0);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=CLOSE_UART1") != NULL) {
        hal_uart_deinit(1);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=CLOSE_UART2") != NULL) {
        hal_uart_deinit(2);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=CLOSE_UART3") != NULL) {
        hal_uart_deinit(3);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=CLOSE_UART") != NULL) {
        hal_uart_deinit(0);
        hal_uart_deinit(1);
        hal_uart_deinit(2);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=LOCK") != NULL) {
        sys_default_lock = true;
        uint32_t lock_low;
        uint32_t lock_high;
        //nvdm_write_data_item("sleep_manager", "sys_default_lock", NVDM_DATA_ITEM_TYPE_RAW_DATA, (uint8_t *)&sys_default_lock, sizeof(sys_default_lock));
        hal_sleep_manager_get_sleep_lock_status(HAL_SLEEP_LOCK_ALL, &lock_high, &lock_low);
        if (sys_lock_handle < 32) {
            if (!(lock_low & (1 << sys_lock_handle))) {
                hal_sleep_manager_acquire_sleeplock(sys_lock_handle, HAL_SLEEP_LOCK_ALL);
            }
        } else {
            if (!(lock_high & (1 << (sys_lock_handle - 32)))) {
                hal_sleep_manager_acquire_sleeplock(sys_lock_handle, HAL_SLEEP_LOCK_ALL);
            }
        }
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr(cmd, "AT+SM=UNLOCK") != NULL) {
        sys_default_lock = false;
        //nvdm_write_data_item("sleep_manager", "sys_default_lock", NVDM_DATA_ITEM_TYPE_RAW_DATA, (uint8_t *)&sys_default_lock, sizeof(sys_default_lock));
        hal_sleep_manager_release_sleeplock(sys_lock_handle, HAL_SLEEP_LOCK_ALL);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
#if configUSE_TICKLESS_IDLE == 2
    } else if (strstr(cmd, "AT+SM=STATUS") != NULL) {
        tickless_system_status();
        LOGMSGIDW("CLOCK 0x%x\r\n", *((volatile unsigned int *)0xA21D0300));
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
#endif
    } else {
        strcpy((char *) response.response_buf, "Not Support\n");
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
        atci_send_response(&response);
        return ATCI_STATUS_ERROR;
    }

    return ATCI_STATUS_OK;
}
#elif ((PRODUCT_VERSION == 1552) || defined(AM255X) || (PRODUCT_VERSION == 2811))
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include "at_command.h"
#include "syslog.h"
#ifdef HAL_AUDIO_MODULE_ENABLED
#include "bt_sink_srv_ami.h"
#endif
#ifdef HAL_SLEEP_MANAGER_ENABLED

#include "hal_sleep_manager.h"
#include "hal_sleep_manager_internal.h"
#include "hal_spm.h"
#include "hal_pmu.h"
#include "nvdm.h"
#include "hal_rtc.h"
#include "hal_clock_internal.h"
#include <stdlib.h>
#ifdef MTK_SWLA_ENABLE
#include "swla.h"
#endif

log_create_module(atci_slp, PRINT_LEVEL_INFO);
#define LOGI(fmt,arg...)   LOG_I(atci_slp ,"ATCMD: "fmt,##arg)
#define LOGMSGIDI(fmt,cnt,arg...)   LOG_MSGID_I(atci_slp ,"ATCMD: "fmt,cnt,##arg)

extern uint8_t sleep_manager_handle;
extern void bsp_ept_gpio_setting_init(void);
#ifdef HAL_PMU_MODULE_ENABLED
extern void pmu_sw_enter_sleep(pmu_power_domain_t domain);
#endif
extern uint32_t clock_get_pll_state(clock_pll_id pll_id);
extern void rtc_32k_off(void);

#ifdef HAL_SLEEP_MANAGER_ENABLED
extern atci_status_t atci_cmd_hdlr_sleep_manager(atci_parse_cmd_param_t *parse_cmd);
#endif

#ifdef HAL_RTC_MODULE_ENABLED
static void rtc_alarm_setting()
{
    hal_rtc_time_t time, rtc_time;

    hal_rtc_get_time(&time);
    rtc_time.rtc_year = time.rtc_year;
    rtc_time.rtc_mon = time.rtc_mon;
    rtc_time.rtc_day = time.rtc_day;
    rtc_time.rtc_hour = time.rtc_hour;
    rtc_time.rtc_min = time.rtc_min;
    rtc_time.rtc_sec = time.rtc_sec;
    rtc_time.rtc_week = time.rtc_week;
    rtc_time.rtc_milli_sec = time.rtc_milli_sec;
    if (time.rtc_sec + 25 > 59) {
        rtc_time.rtc_sec = time.rtc_sec + 25 - 60;
        rtc_time.rtc_min = time.rtc_min + 1;
    } else {
        rtc_time.rtc_sec = time.rtc_sec + 25;
    }
    hal_rtc_set_alarm(&rtc_time);
    LOGMSGIDI("alarm : %d/%d/%d %d:%d:%d(%d)\r\n", 7, rtc_time.rtc_year, rtc_time.rtc_mon, rtc_time.rtc_day,
              rtc_time.rtc_hour, rtc_time.rtc_min, rtc_time.rtc_sec, rtc_time.rtc_milli_sec);
}

void enter_rtc_off_mode()
{
    LOGMSGIDI("Enter RTC Off Mode\r\n", 0);
    hal_gpt_delay_ms(100);
    rtc_32k_off();
    hal_gpt_delay_ms(2000);
    hal_rtc_enter_rtc_mode();
}

extern bool back_from_rtc_mode;
void enter_rtc_mode()
{
#ifdef MTK_HAL_EXT_32K_ENABLE
    LOGMSGIDI("enter_rtc_mode : RTC_32K_XTAL\r\n", 0);
#else
    LOGMSGIDI("enter_rtc_mode : RTC_32KLess\r\n", 0);
#endif

    hal_gpt_delay_ms(200);
    if (back_from_rtc_mode == false) {
        rtc_alarm_setting();
        hal_rtc_enable_alarm();
        hal_rtc_enter_rtc_mode();
    } else {
        LOGMSGIDI("rtc mode wakeup by alarm test pass\r\n", 0);
        rtc_alarm_setting();
        hal_rtc_disable_alarm();
        hal_rtc_enter_rtc_mode();
    }
}

extern bool back_from_rtc_mode;
void rtc_alarm_irq_test(void)
{
    if (back_from_rtc_mode == false) {
        rtc_alarm_setting();
        hal_rtc_enable_alarm();
        hal_rtc_enter_rtc_mode();
    } else {
        LOGMSGIDI("rtc mode wakeup by alarm test pass\r\n", 0);
        rtc_alarm_setting();
        hal_rtc_disable_alarm();
        hal_rtc_enter_rtc_mode();
    }
}
#endif
#include "hal_i2c_master.h"
#include "hal_platform.h"
#include "hal_nvic_internal.h"
#include "hal_pmu_mt6388_platform.h"
#include "hal_eint.h"

typedef volatile unsigned short *APBADDR;
#define ABB_BASE          (0xA2070000)

#define AUDENC_ANA_CON0      ((APBADDR)(ABB_BASE + 0x0100))
#define AUDENC_ANA_CON1      ((APBADDR)(ABB_BASE + 0x0104))
#define AUDENC_ANA_CON2      ((APBADDR)(ABB_BASE + 0x0108))
#define AUDENC_ANA_CON3      ((APBADDR)(ABB_BASE + 0x010C))
#define AUDENC_ANA_CON4      ((APBADDR)(ABB_BASE + 0x0110))
#define AUDENC_ANA_CON5      ((APBADDR)(ABB_BASE + 0x0114))
#define AUDENC_ANA_CON6      ((APBADDR)(ABB_BASE + 0x0118))
#define AUDENC_ANA_CON7      ((APBADDR)(ABB_BASE + 0x011C))
#define AUDENC_ANA_CON8      ((APBADDR)(ABB_BASE + 0x0120))
#define AUDENC_ANA_CON9      ((APBADDR)(ABB_BASE + 0x0124))
#define AUDENC_ANA_CON10     ((APBADDR)(ABB_BASE + 0x0128))
#define AUDENC_ANA_CON11     ((APBADDR)(ABB_BASE + 0x012C))
#define AUDENC_ANA_CON12     ((APBADDR)(ABB_BASE + 0x0130))
#define AUDENC_ANA_CON13     ((APBADDR)(ABB_BASE + 0x0134))
#define AUDENC_ANA_CON14     ((APBADDR)(ABB_BASE + 0x0138))

#define AUDDEC_ANA_CON0      ((APBADDR)(ABB_BASE + 0x0200))
#define AUDDEC_ANA_CON1      ((APBADDR)(ABB_BASE + 0x0204))
#define AUDDEC_ANA_CON2      ((APBADDR)(ABB_BASE + 0x0208))
#define AUDDEC_ANA_CON3      ((APBADDR)(ABB_BASE + 0x020C))
#define AUDDEC_ANA_CON4      ((APBADDR)(ABB_BASE + 0x0210))
#define AUDDEC_ANA_CON5      ((APBADDR)(ABB_BASE + 0x0214))
#define AUDDEC_ANA_CON6      ((APBADDR)(ABB_BASE + 0x0218))
#define AUDDEC_ANA_CON7      ((APBADDR)(ABB_BASE + 0x021C))
#define AUDDEC_ANA_CON8      ((APBADDR)(ABB_BASE + 0x0220))
#define AUDDEC_ANA_CON9      ((APBADDR)(ABB_BASE + 0x0224))
#define AUDDEC_ANA_CON10     ((APBADDR)(ABB_BASE + 0x0228))
#define AUDDEC_ANA_CON11     ((APBADDR)(ABB_BASE + 0x022C))
#define AUDDEC_ANA_CON12     ((APBADDR)(ABB_BASE + 0x0230))
#define AUDDEC_ANA_CON13     ((APBADDR)(ABB_BASE + 0x0234))
#define AUDDEC_ANA_CON14     ((APBADDR)(ABB_BASE + 0x0238))
#define AUDIO_TOP_CON0       ((APBADDR)(0x70000000))

#define PMU2_ANA_CON0        ((APBADDR)(ABB_BASE + 0x0500))
#define PMU2_ANA_RO          ((APBADDR)(ABB_BASE + 0x0510))

void dump_clock_cg(void)
{
    unsigned int XO_PDN_COND0_status, PDN_COND1_status, PDN_COND0_status, i;

    PDN_COND0_status = *((volatile uint32_t *)(0xA2270300));
    LOGMSGIDI("PDN_COND0:\r\n", 0);
    for (i = 1; i < 28; i++) {
        if (((PDN_COND0_status >> i) & 0x01) == 0) {
            //LOGMSGIDI("PDN_COND0[%d]\r\n",i );
            switch (i) {
                case 1 :
                    LOGMSGIDI("RG_SW_SPISLV_CG Clock on:%d\r\n", 1, i);
                    break;
                case 2 :
                    LOGMSGIDI("RG_SW_SDIOMST_CG Clock on:%d\r\n", 1, i);
                    break;
                case 3 :
                    LOGMSGIDI("RG_SW_SDIOMST_BUS_CG Clock on:%d\r\n", 1, i);
                    break;
                case 4 :
                    LOGMSGIDI("RG_SW_I2S0_CG Clock on:%d\r\n", 1, i);
                    break;
                case 5 :
                    LOGMSGIDI("RG_SW_I2S1_CG Clock on:%d\r\n", 1, i);
                    break;
                case 6 :
                    LOGMSGIDI("RG_SW_I2S2_CG Clock on:%d\r\n", 1, i);
                    break;
                case 7 :
                    LOGMSGIDI("RG_SW_I2S3_CG Clock on:%d\r\n", 1, i);
                    break;
                case 8 :
                    LOGMSGIDI("RG_SW_SPIMST0_CG Clock on:%d\r\n", 1, i);
                    break;
                case 9 :
                    LOGMSGIDI("RG_SW_SPIMST1_CG Clock on:%d\r\n", 1, i);
                    break;
                case 10 :
                    LOGMSGIDI("RG_SW_SPIMST2_CG Clock on:%d\r\n", 1, i);
                    break;
                case 11 :
                    LOGMSGIDI("RG_SW_UART1_CG Clock on:%d\r\n", 1, i);
                    break;
                case 12 :
                    LOGMSGIDI("RG_SW_UART2_CG Clock on:%d\r\n", 1, i);
                    break;
                case 13 :
                    LOGMSGIDI("RG_SW_BSI_CG Clock on:%d\r\n", 1, i);
                    break;
                case 14 :
                    LOGMSGIDI("RG_SW_IRRX_26M_CG Clock on:%d\r\n", 1, i);
                    break;

                case 16 :
                    LOGMSGIDI("RG_SW_CM_SYSROM_CG Clock on:%d\r\n", 1, i);
                    break;
                case 17 :
                    LOGMSGIDI("RG_SFC_SW_CG Clock on:%d\r\n", 1, i);
                    break;
                case 18 :
                    LOGMSGIDI("RG_SW_TRNG_CG Clock on:%d\r\n", 1, i);
                    break;
                case 19 :
                    LOGMSGIDI("RG_EMI_SW_CG Clock on:%d\r\n", 1, i);
                    break;
                case 20 :
                    LOGMSGIDI("RG_SW_UART0_CG Clock on:%d\r\n", 1, i);
                    break;
                case 21 :
                    LOGMSGIDI("RG_SW_CRYPTO_CG Clock on:%d\r\n", 1, i);
                    break;
                case 23 :
                    LOGMSGIDI("RG_SW_GPTIMER_CG Clock on:%d\r\n", 1, i);
                    break;
                case 24 :
                    LOGMSGIDI("RG_SW_OSTIMER_CG Clock on:%d\r\n", 1, i);
                    break;
                case 25 :
                    LOGMSGIDI("RG_SW_USB_CG Clock on:%d\r\n", 1, i);
                    break;
                case 26 :
                    LOGMSGIDI("RG_SW_USB_BUS_CG Clock on:%d\r\n", 1, i);
                    break;
                case 27 :
                    LOGMSGIDI("RG_SW_USB_DMA_CG Clock on:%d\r\n", 1, i);
                    break;
            }
        }
    }

    PDN_COND1_status = *((volatile uint32_t *)(0xA2270330));
    LOGMSGIDI("PDN_COND1\r\n", 0);
    for (i = 0; i < 13; i++) {
        if (((PDN_COND1_status >> i) & 0x01) == 0) {
            //  LOGMSGIDI("PDN_COND1[%d]\r\n",1,i );
            switch (i) {
                case 0 :
                    LOGMSGIDI("RG_SW_PWM0_CG Clock on:%d\r\n", 1, i);
                    break;
                case 1 :
                    LOGMSGIDI("RG_SW_PWM1_CG Clock on:%d\r\n", 1, i);
                    break;
                case 2 :
                    LOGMSGIDI("RG_SW_PWM2_CG Clock on:%d\r\n", 1, i);
                    break;
                case 3 :
                    LOGMSGIDI("RG_SW_PWM3_CG Clock on:%d\r\n", 1, i);
                    break;
                case 4 :
                    LOGMSGIDI("RG_SW_PWM4_CG Clock on:%d\r\n", 1, i);
                    break;
                case 5 :
                    LOGMSGIDI("RG_SW_PWM5_CG Clock on:%d\r\n", 1, i);
                    break;
                case 6 :
                    LOGMSGIDI("RG_SW_PWM6_CG Clock on:%d\r\n", 1, i);
                    break;
                case 7 :
                    LOGMSGIDI("RG_SW_PWM7_CG Clock on:%d\r\n", 1, i);
                    break;
                case 8 :
                    LOGMSGIDI("RG_SW_PWM8_CG Clock on:%d\r\n", 1, i);
                    break;
                case 9 :
                    LOGMSGIDI("RG_SW_PWM9_CG Clock on:%d\r\n", 1, i);
                    break;
                case 10 :
                    LOGMSGIDI("RG_SW_CM4_DMA_CG Clock on:%d\r\n", 1, i);
                    break;
                case 11 :
                    LOGMSGIDI("RG_SW_DSP_DMA_CG Clock on:%d\r\n", 1, i);
                    break;
                case 12 :
                    LOGMSGIDI("RG_SW_I2S_DMA_CG Clock on:%d\r\n", 1, i);
                    break;
            }
        }
    }

    XO_PDN_COND0_status = *((volatile uint32_t *)(0xA2030B00));
    LOGMSGIDI("XO_PDN_COND0\r\n", 0);
    for (i = 0; i < 27; i++) {
        if (((XO_PDN_COND0_status >> i) & 0x01) == 0) {
            // LOGMSGIDI("XO_PDN_COND0[%d]\r\n",1 ,i );
            switch (i) {
                case 0 :
                    LOGMSGIDI("RG_SW_I2C_DMA_CG Clock on:%d\r\n", 1, i);
                    break;
                case 1 :
                    LOGMSGIDI("RG_SW_CAP_TOUCH_CG Clock on:%d\r\n", 1, i);
                    break;
                case 2 :
                    LOGMSGIDI("RG_SW_AUD_ENGINE_CG Clock on:%d\r\n", 1, i);
                    break;
                case 3 :
                    LOGMSGIDI("RG_SW_AUD_UL_HIRES_CG Clock on:%d\r\n", 1, i);
                    break;
                case 4 :
                    LOGMSGIDI("RG_SW_AUD_DL_HIRES_CG Clock on:%d\r\n", 1, i);
                    break;
                case 5 :
                    LOGMSGIDI("RG_SW_AUD_INTERFACE0_CG Clock on:%d\r\n", 1, i);
                    break;
                case 6 :
                    LOGMSGIDI("RG_SW_AUD_INTERFACE1_CG Clock on:%d\r\n", 1, i);
                    break;
                case 7 :
                    LOGMSGIDI("RG_SW_AUD_GPSRC_CG Clock on:%d\r\n", 1, i);
                    break;
                case 8 :
                    LOGMSGIDI("RG_SW_AUD_BUS_CG Clock on:%d\r\n", 1, i);
                    break;
                case 9 :
                    LOGMSGIDI("RG_SW_BT_26M_CG Clock on:%d\r\n", 1, i);
                    break;
                case 10 :
                    LOGMSGIDI("RG_SW_IRRX_BCLK_CG Clock on:%d\r\n", 1, i);
                    break;
                case 11 :
                    LOGMSGIDI("RG_SW_I2C0_CG Clock on:%d\r\n", 1, i);
                    break;
                case 12 :
                    LOGMSGIDI("RG_SW_I2C1_CG Clock on:%d\r\n", 1, i);
                    break;
                case 13 :
                    LOGMSGIDI("RG_SW_I2C2_CG Clock on:%d\r\n", 1, i);
                    break;
                case 14 :
                    LOGMSGIDI("RG_SW_ANC_CG Clock on:%d\r\n", 1, i);
                    break;
                case 15 :
                    LOGMSGIDI("RG_SW_BT_104M_CG Clock on:%d\r\n", 1, i);
                    break;
                case 16 :
                    LOGMSGIDI("RG_SW_SPM_CG Clock on:%d\r\n", 1, i);
                    break;
                case 17 :
                    LOGMSGIDI("RG_SW_MIXEDSYS_D_CG Clock on:%d\r\n", 1, i);
                    break;
                case 18 :
                    LOGMSGIDI("RG_SW_EFUSE_CG Clock on:%d\r\n", 1, i);
                    break;
                case 19 :
                    LOGMSGIDI("RG_SW_SEJ_CG Clock on:%d\r\n", 1, i);
                    break;
                case 20 :
                    LOGMSGIDI("RG_SW_I2C_AO_CG Clock on:%d\r\n", 1, i);
                    break;
                case 21 :
                    LOGMSGIDI("RG_SW_AUXADC_CG Clock on:%d\r\n", 1, i);
                    break;
                case 22 :
                    LOGMSGIDI("RG_SW_ABB_26M_CG Clock on:%d\r\n", 1, i);
                    break;
                case 23 :
                    LOGMSGIDI("RG_SW_OSC_1P1_CG Clock on:%d\r\n", 1, i);
                    break;
                case 24 :
                    LOGMSGIDI("RG_SW_OSC_0P9_CG Clock on:%d\r\n", 1, i);
                    break;
                case 25 :
                    LOGMSGIDI("RG_SW_GPLL_26M_CG Clock on:%d\r\n", 1, i);
                    break;
                case 26 :
                    LOGMSGIDI("RG_SW_APLL_26M_CG Clock on:%d\r\n", 1, i);
                    break;
            }
        }
    }

    LOGMSGIDI("0xA2030B00:0x%x\r\n", 1, *((volatile unsigned int *)(0xA2030B00)));
    LOGMSGIDI("0xA2270300:0x%x\r\n", 1, *((volatile unsigned int *)(0xA2270300)));
    LOGMSGIDI("0xA2270330:0x%x\r\n", 1, *((volatile unsigned int *)(0xA2270330)));
}

void lowpower_setting_check(void)
{
    int pll_result = 0, clock_pll_state[3], i;

    for (i = 0; i < 3; i++) {
        clock_pll_state[i] = clock_get_pll_state(i);
        if (clock_pll_state[i] != 0) {
            pll_result++;
        }
    }
    LOGMSGIDI("clock_pll_state[0]:%d\r\n", 1, (int)clock_get_pll_state(0));
    LOGMSGIDI("clock_pll_state[1]:%d\r\n", 1, (int)clock_get_pll_state(1));
    LOGMSGIDI("clock_pll_state[2]:%d\r\n", 1, (int)clock_get_pll_state(2));
    if (pll_result != 0) {
        LOGMSGIDI("===========PLL Low Power Check : Fail===========\r\n", 0);
    }

    LOGMSGIDI("AUDENC_ANA_CON0 =0x%x\r\n", 1,  *AUDENC_ANA_CON0);
    LOGMSGIDI("AUDENC_ANA_CON1 =0x%x\r\n", 1,  *AUDENC_ANA_CON1);
    LOGMSGIDI("AUDENC_ANA_CON2 =0x%x\r\n", 1,  *AUDENC_ANA_CON2);
    LOGMSGIDI("AUDENC_ANA_CON3 =0x%x\r\n", 1,  *AUDENC_ANA_CON3);
    LOGMSGIDI("AUDENC_ANA_CON4 =0x%x\r\n", 1,  *AUDENC_ANA_CON4);
    LOGMSGIDI("AUDENC_ANA_CON5 =0x%x\r\n", 1,  *AUDENC_ANA_CON5);
    LOGMSGIDI("AUDENC_ANA_CON6 =0x%x\r\n", 1,  *AUDENC_ANA_CON6);
    LOGMSGIDI("AUDENC_ANA_CON7 =0x%x\r\n", 1,  *AUDENC_ANA_CON7);
    LOGMSGIDI("AUDENC_ANA_CON8 =0x%x\r\n", 1,  *AUDENC_ANA_CON8);
    LOGMSGIDI("AUDENC_ANA_CON9 =0x%x\r\n", 1,  *AUDENC_ANA_CON9);
    LOGMSGIDI("AUDENC_ANA_CON10 =0x%x\r\n", 1, *AUDENC_ANA_CON10);
    LOGMSGIDI("AUDENC_ANA_CON11 =0x%x\r\n", 1, *AUDENC_ANA_CON11);
    LOGMSGIDI("AUDENC_ANA_CON12 =0x%x\r\n", 1, *AUDENC_ANA_CON12);
    LOGMSGIDI("AUDENC_ANA_CON13 =0x%x\r\n", 1, *AUDENC_ANA_CON13);
    LOGMSGIDI("AUDENC_ANA_CON14 =0x%x\r\n", 1, *AUDENC_ANA_CON14);

    LOGMSGIDI("AUDDEC_ANA_CON0 =0x%x\r\n", 1,  *AUDDEC_ANA_CON0);
    LOGMSGIDI("AUDDEC_ANA_CON1 =0x%x\r\n", 1,  *AUDDEC_ANA_CON1);
    LOGMSGIDI("AUDDEC_ANA_CON2 =0x%x\r\n", 1,  *AUDDEC_ANA_CON2);
    LOGMSGIDI("AUDDEC_ANA_CON3 =0x%x\r\n", 1,  *AUDDEC_ANA_CON3);
    LOGMSGIDI("AUDDEC_ANA_CON4 =0x%x\r\n", 1,  *AUDDEC_ANA_CON4);
    LOGMSGIDI("AUDDEC_ANA_CON5 =0x%x\r\n", 1,  *AUDDEC_ANA_CON5);
    LOGMSGIDI("AUDDEC_ANA_CON6 =0x%x\r\n", 1,  *AUDDEC_ANA_CON6);
    LOGMSGIDI("AUDDEC_ANA_CON7 =0x%x\r\n", 1,  *AUDDEC_ANA_CON7);
    LOGMSGIDI("AUDDEC_ANA_CON8 =0x%x\r\n", 1,  *AUDDEC_ANA_CON8);
    LOGMSGIDI("AUDDEC_ANA_CON9 =0x%x\r\n", 1,  *AUDDEC_ANA_CON9);
    LOGMSGIDI("AUDDEC_ANA_CON10 =0x%x\r\n", 1, *AUDDEC_ANA_CON10);
    LOGMSGIDI("AUDDEC_ANA_CON11 =0x%x\r\n", 1, *AUDDEC_ANA_CON11);
    LOGMSGIDI("AUDDEC_ANA_CON12 =0x%x\r\n", 1, *AUDDEC_ANA_CON12);
    LOGMSGIDI("AUDDEC_ANA_CON13 =0x%x\r\n", 1, *AUDDEC_ANA_CON13);
    LOGMSGIDI("AUDDEC_ANA_CON14 =0x%x\r\n", 1, *AUDDEC_ANA_CON14);

    LOGMSGIDI("AUDIO_TOP_CON0 =0x%x\r\n", 1,   *AUDIO_TOP_CON0);

    LOGMSGIDI("PMU2_ANA_CON0 =0x%x\r\n", 1,    *PMU2_ANA_CON0);
    LOGMSGIDI("PMU2_ANA_RO =0x%x\r\n", 1,      *PMU2_ANA_RO);

    LOGMSGIDI("dump clock cg setting\r\n", 0);
    dump_clock_cg();

    if (pll_result == 0) {
        LOGMSGIDI("====================================================\r\n", 0);
        LOGMSGIDI("===========Low Power Setting Check : Pass===========\r\n", 0);
        LOGMSGIDI("====================================================\r\n", 0);
    } else {
        LOGMSGIDI("====================================================\r\n", 0);
        LOGMSGIDI("===========Low Power Setting Check : Fail===========\r\n", 0);
        LOGMSGIDI("====================================================\r\n", 0);
    }
}
#ifdef HAL_AUDIO_SUPPORT_MULTIPLE_STREAM_OUT
extern HAL_DSP_PARA_AU_AFE_CTRL_t audio_nvdm_HW_config;
#endif
atci_status_t atci_cmd_hdlr_sleep_manager(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}};
    response.response_flag = 0;

    if (strstr((char *)parse_cmd->string_ptr, "AT+SM=0") != NULL) {
        strncpy((char *)response.response_buf, "Low Power Test Command Pass!!\r\n", strlen("Low Power Test Command Pass!!\r\n"));
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=HQA") != NULL) {
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=HQA,1") != NULL) {
#ifdef HAL_PMU_MODULE_ENABLED
            pmu_vcore_voltage_sel_6388(PMU_SLEEP, PMIC_VCORE_0P7_V);
            pmu_sw_enter_sleep(PMU_LDO_VA18);
            pmu_sw_enter_sleep(PMU_LDO_VLDO33);
            //pmu_sw_enter_sleep(PMU_BUCK_VRF);
            pmu_sw_enter_sleep(PMU_BUCK_VAUD18);
#endif
            __asm volatile("cpsid i");
            *SPM_CM4_WAKEUP_SOURCE_MASK = 0x3FFF;
            *SPM_DSP0_WAKEUP_SOURCE_MASK = 0x3FFF;
            *SPM_DSP1_WAKEUP_SOURCE_MASK = 0x3FFF;

            //bsp_ept_gpio_setting_init();
            while (1) {
                __asm volatile("cpsid i");
                sleep_management_enter_deep_sleep(0);
                __asm volatile("cpsie i");
            }
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=HQA,2") != NULL) {
#ifdef HAL_PMU_MODULE_ENABLED
            pmu_vcore_voltage_sel_6388(PMU_SLEEP, PMIC_VCORE_0P7_V);
            pmu_sw_enter_sleep(PMU_LDO_VA18);
            pmu_sw_enter_sleep(PMU_LDO_VLDO33);
            pmu_sw_enter_sleep(PMU_BUCK_VRF);
            pmu_sw_enter_sleep(PMU_BUCK_VAUD18);
#endif
            __asm volatile("cpsid i");
            *SPM_CM4_WAKEUP_SOURCE_MASK = 0x3FFF;
            *SPM_DSP0_WAKEUP_SOURCE_MASK = 0x3FFF;
            *SPM_DSP1_WAKEUP_SOURCE_MASK = 0x3FFF;

            bsp_ept_gpio_setting_init();
            while (1) {
                __asm volatile("cpsid i");
                sleep_management_enter_deep_sleep(0);
                __asm volatile("cpsie i");
            }
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=HQA,3") != NULL) {
            LOGMSGIDI("===============================================\r\n", 0);
            LOGMSGIDI("===============Start A2DP======================\r\n", 0);
            LOGMSGIDI("===============================================\r\n", 0);
            lowpower_setting_check();

            *((volatile uint32_t *)(0xA2060010)) |= 0x1000000;
            *((volatile uint32_t *)(SPM_BASE + 0x0110)) = 0x1D;         /* rg_vcore_xo_settle_time */

            *((volatile uint32_t *)(SPM_BASE + 0x0150)) = 0x10100;      /* IGNORE_DSP0_ACTIVE */
            *((volatile uint32_t *)(SPM_BASE + 0x01D4)) = 0x00101;      /* SKIP_SFC_CM4 */
#ifdef HAL_PMU_MODULE_ENABLED
            pmu_vcore_voltage_sel_6388(PMU_SLEEP, PMIC_VCORE_0P9_V);
#endif
            /*
                        LOGMSGIDI("===============================================\r\n" ,0);
                        LOGMSGIDI("============adj normal vcore to 0.85V===========\r\n" ,0);
                        LOGMSGIDI("===============================================\r\n" ,0);
                        //adj normal vcore to 0.85V
                        pmu_set_register_value_mt6388(PMU_RG_BUCK_VCORE_VOSEL_ADDR, PMU_RG_BUCK_VCORE_VOSEL_MASK, PMU_RG_BUCK_VCORE_VOSEL_SHIFT, 0x18);
            */
            response.response_len = strlen((char *) response.response_buf);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&response);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=HQA,4") != NULL) {
#ifdef HAL_PMU_MODULE_ENABLED
            pmu_vcore_voltage_sel_6388(PMU_SLEEP, PMIC_VCORE_0P7_V);
#endif
            //disable Global Bias(~120uA)
            *((volatile uint32_t *)(0xA2070224)) = 0x3030;
#ifdef HAL_PMU_MODULE_ENABLED
            pmu_sw_enter_sleep(PMU_LDO_VA18);
            //pmu_sw_enter_sleep(PMU_LDO_VLDO33);
            pmu_sw_enter_sleep(PMU_BUCK_VRF);
            pmu_sw_enter_sleep(PMU_BUCK_VAUD18);
#endif
            __asm volatile("cpsid i");
            *SPM_CM4_WAKEUP_SOURCE_MASK = 0x3FFF;
            *SPM_DSP0_WAKEUP_SOURCE_MASK = 0x3FFF;
            *SPM_DSP1_WAKEUP_SOURCE_MASK = 0x3FFF;

            bsp_ept_gpio_setting_init();
            while (1) {
                __asm volatile("cpsid i");
                sleep_management_enter_deep_sleep(0);
                __asm volatile("cpsie i");
            }
        }
    }  else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=BT") != NULL) {
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=BT,1") != NULL) {
#ifdef HAL_PMU_MODULE_ENABLED
            pmu_vcore_voltage_sel_6388(PMU_SLEEP, PMIC_VCORE_0P7_V);
            pmu_sw_enter_sleep(PMU_LDO_VA18);
            //pmu_sw_enter_sleep(PMU_LDO_VLDO33);
            //pmu_sw_enter_sleep(PMU_BUCK_VRF);
            pmu_sw_enter_sleep(PMU_BUCK_VAUD18);
#endif
            __asm volatile("cpsid i");
            *SPM_CM4_WAKEUP_SOURCE_MASK = 0x3FFF;
            *SPM_DSP0_WAKEUP_SOURCE_MASK = 0x3FFF;
            *SPM_DSP1_WAKEUP_SOURCE_MASK = 0x3FFF;

            bsp_ept_gpio_setting_init();
            while (1) {
                __asm volatile("cpsid i");
                sleep_management_enter_deep_sleep(0);
                __asm volatile("cpsie i");
            }
        }
        response.response_len = strlen((char *) response.response_buf);
        atci_send_response(&response);
    }  else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=BASE") != NULL) {
#ifdef HAL_PMU_MODULE_ENABLED
        pmu_vcore_voltage_sel_6388(PMU_SLEEP, PMIC_VCORE_0P7_V);
        pmu_sw_enter_sleep(PMU_LDO_VA18);
        pmu_sw_enter_sleep(PMU_LDO_VLDO33);
        pmu_sw_enter_sleep(PMU_BUCK_VRF);
        pmu_sw_enter_sleep(PMU_BUCK_VAUD18);
#endif
        *((volatile uint32_t *)(SPM_BASE + 0x0250)) = 0xF;                  //sysram1 enter power down in sleep
#ifdef HAL_PMU_MODULE_ENABLED
        pmu_lp_mode_6388(PMU_BUCK_VCORE, PMU_HW_MODE);
#endif
        __asm volatile("cpsid i");
        *SPM_CM4_WAKEUP_SOURCE_MASK = 0x3FFF;
        *SPM_DSP0_WAKEUP_SOURCE_MASK = 0x3FFF;
        *SPM_DSP1_WAKEUP_SOURCE_MASK = 0x3FFF;

        //bsp_ept_gpio_setting_init();
        while (1) {
            __asm volatile("cpsid i");
            sleep_management_enter_deep_sleep(0);
            __asm volatile("cpsie i");
        }
        response.response_len = strlen((char *) response.response_buf);
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=LOCK") != NULL) {
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=LOCK,0,SLP") != NULL) {
            //hal_clock_disable(HAL_CLOCK_CG_EMI);
            hal_sleep_manager_unlock_sleep(sleep_manager_handle);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            strncpy((char *)response.response_buf, "unlock sleep manager handle\r\n", strlen("unlock sleep manager handle\r\n"));
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=LOCK,1,SLP") != NULL) {
            hal_sleep_manager_lock_sleep(sleep_manager_handle);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            strncpy((char *)response.response_buf, "lock sleep manager handle\r\n", strlen("lock sleep manager handle\r\n"));
        }
        response.response_len = strlen((char *) response.response_buf);
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=LP") != NULL) {
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=LP,1") != NULL) {
            *((volatile uint32_t *)(0xA207021C)) = 0x103;
            *((volatile uint32_t *)(0x70000004)) = 0x9B0007;
            *((volatile uint32_t *)(0x70000000)) = 0x290CC300;
            hal_gpt_delay_ms(10);

            *((volatile uint32_t *)(0xA2060010)) |= 0x1000000;
            *((volatile uint32_t *)(SPM_BASE + 0x0110)) = 0x1D;         /* rg_vcore_xo_settle_time */
            hal_gpt_delay_ms(10);
            *((volatile uint32_t *)(SPM_BASE + 0x0150)) = 0x10100;      /* IGNORE_DSP0_ACTIVE */
            *((volatile uint32_t *)(SPM_BASE + 0x01D4)) = 0x00101;      /* SKIP_SFC_CM4 */
            hal_gpt_delay_ms(10);

            /* ADJ Vcore to SW mode and 0.87V */
            pmu_set_register_value_mt6388(0x308, 0xFFFF, 0, 0x0);
            pmu_set_register_value_mt6388(0x30A, 0xFFFF, 0, 0x1);
            pmu_set_register_value_mt6388(0x316, 0xFFFF, 0, 0x1B);

            hal_gpt_delay_ms(10);
            /* VRF 1.4V*/
            pmu_set_register_value_mt6388(0x332, 0xFFFF, 0, 0x1C);
            pmu_set_register_value_mt6388(0x346, 0xFFFF, 0, 0x1C);

            hal_gpt_delay_ms(10);
            /* VAUD18 0.85 */
            pmu_set_register_value_mt6388(0xB2, 0xFFFF, 0, 0x340D);
            hal_gpt_delay_ms(10);
            //unlock sleep
            hal_sleep_manager_unlock_sleep(sleep_manager_handle);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            strncpy((char *)response.response_buf, "unlock sleep manager handle\r\n", strlen("unlock sleep manager handle\r\n"));
        }

        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=LP,2") != NULL) {
            *((volatile uint32_t *)(0xA207021C)) = 0x103;
            *((volatile uint32_t *)(0x70000004)) = 0x9B0007;
            *((volatile uint32_t *)(0x70000000)) = 0x290CC300;
            hal_gpt_delay_ms(10);
            //For I2SM lowpower
            *((volatile uint32_t *)(0x70000EE8)) = 0;
            *((volatile uint32_t *)(0x70000EEC)) = 0;
            *((volatile uint32_t *)(0x70000EE0)) = 0;
            *((volatile uint32_t *)(0x70000EDC)) = 0;
            hal_gpt_delay_ms(10);
            *((volatile uint32_t *)(0x70000004)) = 0x9B000E;
            *((volatile uint32_t *)(0x70000000)) = 0x2F00C000;
            hal_gpt_delay_ms(10);

            *((volatile uint32_t *)(0xA2060010)) |= 0x1000000;
            *((volatile uint32_t *)(SPM_BASE + 0x0110)) = 0x1D;         /* rg_vcore_xo_settle_time */
            hal_gpt_delay_ms(10);
            *((volatile uint32_t *)(SPM_BASE + 0x0150)) = 0x10100;      /* IGNORE_DSP0_ACTIVE */
            *((volatile uint32_t *)(SPM_BASE + 0x01D4)) = 0x00101;      /* SKIP_SFC_CM4 */
            hal_gpt_delay_ms(10);

            /* ADJ Vcore to SW mode and 0.87V */
            pmu_set_register_value_mt6388(0x308, 0xFFFF, 0, 0x0);
            pmu_set_register_value_mt6388(0x30A, 0xFFFF, 0, 0x1);
            pmu_set_register_value_mt6388(0x316, 0xFFFF, 0, 0x1B);

            hal_gpt_delay_ms(10);
            /* VRF 1.4V*/
            pmu_set_register_value_mt6388(0x332, 0xFFFF, 0, 0x1C);
            pmu_set_register_value_mt6388(0x346, 0xFFFF, 0, 0x1C);

            hal_gpt_delay_ms(10);
            /* VAUD18 0.85 */
            pmu_set_register_value_mt6388(0xB2, 0xFFFF, 0, 0x340D);
            hal_gpt_delay_ms(10);
            //unlock sleep
            hal_sleep_manager_unlock_sleep(sleep_manager_handle);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            strncpy((char *)response.response_buf, "unlock sleep manager handle\r\n", strlen("unlock sleep manager handle\r\n"));
        }
        response.response_len = strlen((char *) response.response_buf);
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR") != NULL) {
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,DSP0,0") != NULL) {
            strncpy((char *)response.response_buf, "DSP0 Power Off\r\n", strlen("DSP0 Power Off\r\n"));
            spm_control_mtcmos(SPM_MTCMOS_DSP0, SPM_MTCMOS_PWR_DISABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,DSP0,1") != NULL) {
            strncpy((char *)response.response_buf, "DSP0 Power On\r\n", strlen("DSP0 Power On\r\n"));
            spm_control_mtcmos(SPM_MTCMOS_DSP0, SPM_MTCMOS_PWR_ENABLE);
            //hal_dsp_core_reset(HAL_CORE_DSP0, DSP0_BASE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,DSP1,0") != NULL) {
            strncpy((char *)response.response_buf, "DSP1 Power Off\r\n", strlen("DSP1 Power Off\r\n"));
            spm_control_mtcmos(SPM_MTCMOS_DSP1, SPM_MTCMOS_PWR_DISABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,DSP1,1") != NULL) {
            strncpy((char *)response.response_buf, "DSP1 Power On\r\n", strlen("DSP1 Power On\r\n"));
            spm_control_mtcmos(SPM_MTCMOS_DSP1, SPM_MTCMOS_PWR_ENABLE);
            //hal_dsp_core_reset(HAL_CORE_DSP1, DSP1_BASE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,AUDIO,0") != NULL) {
            strncpy((char *)response.response_buf, "AUDIO Power Off\r\n", strlen("AUDIO Power Off\r\n"));
            spm_control_mtcmos(SPM_MTCMOS_AUDIO, SPM_MTCMOS_PWR_DISABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,AUDIO,1") != NULL) {
            strncpy((char *)response.response_buf, "AUDIO Power On\r\n", strlen("AUDIO Power On\r\n"));
            spm_control_mtcmos(SPM_MTCMOS_AUDIO, SPM_MTCMOS_PWR_ENABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,CONN,0") != NULL) {
            strncpy((char *)response.response_buf, "CONN Power Off\r\n", strlen("CONN Power Off\r\n"));
            spm_control_mtcmos(SPM_MTCMOS_CONN, SPM_MTCMOS_PWR_DISABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,CONN,1") != NULL) {
            strncpy((char *)response.response_buf, "CONN Power On\r\n", strlen("CONN Power On\r\n"));
            spm_control_mtcmos(SPM_MTCMOS_CONN, SPM_MTCMOS_PWR_ENABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,ALL,0") != NULL) {
            strncpy((char *)response.response_buf, "DSP0/DSP1/CONN/AUDIO MTCMOS Off\r\n", strlen("DSP0/DSP1/CONN/AUDIO MTCMOS Off\r\n"));
            spm_control_mtcmos(SPM_MTCMOS_DSP0, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_DSP1, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_CONN, SPM_MTCMOS_PWR_DISABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,ALL,1") != NULL) {
            strncpy((char *)response.response_buf, "DSP0/DSP1/CONN/AUDIO MTCMOS On\r\n", strlen("DSP0/DSP1/CONN/AUDIO MTCMOS On\r\n"));
            spm_control_mtcmos(SPM_MTCMOS_DSP0, SPM_MTCMOS_PWR_ENABLE);
            spm_control_mtcmos(SPM_MTCMOS_DSP1, SPM_MTCMOS_PWR_ENABLE);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO, SPM_MTCMOS_PWR_ENABLE);
            spm_control_mtcmos(SPM_MTCMOS_CONN, SPM_MTCMOS_PWR_ENABLE);
        }
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=RTC") != NULL) {
#ifdef HAL_RTC_MODULE_ENABLED
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=RTC,0") != NULL) {
            // enter_rtc_off_mode();
            //////////
            //RTC OFF Mode
            rtc_32k_off();
            LOGMSGIDI("LPHQA_enter_rtc_mode : RTC OFF Mode\r\n", 0);
            hal_gpt_delay_ms(2000);
            hal_rtc_enter_rtc_mode();
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=RTC,1") != NULL) {
            //enter_rtc_mode();
            /*
            RTC_32K_XTAL : MTK_HAL_EXT_32K_ENABLE = y
            RTC_32KLess  : MTK_HAL_EXT_32K_ENABLE = n
            */
#ifdef MTK_HAL_EXT_32K_ENABLE
            LOGMSGIDI("LPHQA_enter_rtc_mode : RTC_32K_XTAL\r\n", 0);
#else
            LOGMSGIDI("LPHQA_enter_rtc_mode : RTC_32KLess\r\n", 0);
#endif
            hal_gpt_delay_ms(200);
            rtc_alarm_irq_test();
        }
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
#else
        strncpy((char *)response.response_buf, "Not Support\n", strlen("Not Support\n"));
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
        atci_send_response(&response);
#endif
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=DEBUGIO") != NULL) {
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=DEBUGIO,1") != NULL) {
            strncpy((char *)response.response_buf, "1_SPM_debug_bus_0x43424440\r\n", strlen("1_SPM_debug_bus_0x43424440\r\n"));
            spm_debug_io(0x43424440);//1_SPM_debug_bus_0x43424440
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=DEBUGIO,2") != NULL) {
            strncpy((char *)response.response_buf, "2_SPM_debug_bus_0x45424440\r\n", strlen("2_SPM_debug_bus_0x45424440\r\n"));
            spm_debug_io(0x45424440);//2_SPM_debug_bus_0x45424440
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=DEBUGIO,3") != NULL) {
            strncpy((char *)response.response_buf, "3_SPM_debug_bus_0x43424540\r\n", strlen("3_SPM_debug_bus_0x43424540\r\n"));
            spm_debug_io(0x43424540);//3_SPM_debug_bus_0x43424540
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=DEBUGIO,4") != NULL) {
            strncpy((char *)response.response_buf, "4_SPM_debug_bus_0x43424445\r\n", strlen("4_SPM_debug_bus_0x43424445\r\n"));
            spm_debug_io(0x43424445);//4_SPM_debug_bus_0x43424445
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=DEBUGIO,5") != NULL) {
            strncpy((char *)response.response_buf, "5_SPM_debug_bus_0x45444140\r\n", strlen("5_SPM_debug_bus_0x45444140\r\n"));
            spm_debug_io(0x45444140);//5_SPM_debug_bus_0x45444140
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=DEBUGIO,6") != NULL) {
            strncpy((char *)response.response_buf, "6_SPM_debug_bus_0x43424441\r\n", strlen("6_SPM_debug_bus_0x43424441\r\n"));
            spm_debug_io(0x43424441);//6_SPM_debug_bus_0x43424441
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=DEBUGIO,7") != NULL) {
            strncpy((char *)response.response_buf, "7_SPM_debug_bus_0x43424448\r\n", strlen("7_SPM_debug_bus_0x43424448\r\n"));
            spm_debug_io(0x43424448);//7_SPM_debug_bus_0x43424448
        }
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=STATUS") != NULL) {
        hal_gpt_delay_ms(200);
        if (hal_sleep_manager_is_sleep_locked()) {
            sleep_management_get_lock_sleep_handle_list();
        } else {
            LOGMSGIDI("\r\nAny Sleep Handle not locked\r\n", 0);
        }
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=MONO") != NULL) {
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=MONO,R") != NULL) {
#if 0
            *((volatile uint32_t *)(0xA2120B04)) &= 0xFFFFFFFC;
            *((volatile uint32_t *)(0xA2120B04)) |= 0x01;
#else
            if (AUD_EXECUTION_SUCCESS != ami_set_audio_device(STREAM_OUT, AU_DSP_VOICE, HAL_AUDIO_DEVICE_DAC_R, HAL_AUDIO_INTERFACE_1, NOT_REWRITE)) {
                LOGMSGIDI("Set MONO_R error\r\n", 0);
            }
            if (AUD_EXECUTION_SUCCESS != ami_set_audio_device(STREAM_OUT, AU_DSP_AUDIO, HAL_AUDIO_DEVICE_DAC_R, HAL_AUDIO_INTERFACE_1, NOT_REWRITE)) {
                LOGMSGIDI("Set MONO_R error\r\n", 0);
            }
#endif
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=MONO,L") != NULL) {
#if 0
            *((volatile uint32_t *)(0xA2120B04)) &= 0xFFFFFFFC;
            *((volatile uint32_t *)(0xA2120B04)) |= 0x02;
#else
            if (AUD_EXECUTION_SUCCESS != ami_set_audio_device(STREAM_OUT, AU_DSP_VOICE, HAL_AUDIO_DEVICE_DAC_L, HAL_AUDIO_INTERFACE_1, NOT_REWRITE)) {
                LOGMSGIDI("Set MONO_L error\r\n", 0);
            }
            if (AUD_EXECUTION_SUCCESS != ami_set_audio_device(STREAM_OUT, AU_DSP_AUDIO, HAL_AUDIO_DEVICE_DAC_L, HAL_AUDIO_INTERFACE_1, NOT_REWRITE)) {
                LOGMSGIDI("Set MONO_L error\r\n", 0);
            }
#endif
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=MONO,LR") != NULL) {
#if 0
            *((volatile uint32_t *)(0xA2120B04)) &= 0xFFFFFFFC;
            *((volatile uint32_t *)(0xA2120B04)) |= 0x03;
#else
            if (AUD_EXECUTION_SUCCESS != ami_set_audio_device(STREAM_OUT, AU_DSP_VOICE, HAL_AUDIO_DEVICE_DAC_DUAL, HAL_AUDIO_INTERFACE_1, NOT_REWRITE)) {
                LOGMSGIDI("Set MONO_LR error\r\n", 0);
            }
            if (AUD_EXECUTION_SUCCESS != ami_set_audio_device(STREAM_OUT, AU_DSP_AUDIO, HAL_AUDIO_DEVICE_DAC_DUAL, HAL_AUDIO_INTERFACE_1, NOT_REWRITE)) {
                LOGMSGIDI("Set MONO_LR error\r\n", 0);
            }
#endif
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        }
        response.response_len = strlen((char *) response.response_buf);
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=I2S") != NULL) {
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=I2S,0") != NULL) {
            *((volatile uint32_t *)(0xA2120B04)) &= 0xFFFFFFFB;
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=I2S,1") != NULL) {
#if 0
            *((volatile uint32_t *)(0xA2120B04)) &= 0xFFFFFFFB;
            *((volatile uint32_t *)(0xA2120B04)) |= 0x04;
#else
            if (AUD_EXECUTION_SUCCESS != ami_set_audio_device(STREAM_OUT, AU_DSP_VOICE, HAL_AUDIO_DEVICE_I2S_MASTER, HAL_AUDIO_INTERFACE_1, NOT_REWRITE)) {
                LOGMSGIDI("Set I2S_MASTER error\r\n", 0);
            }
            if (AUD_EXECUTION_SUCCESS != ami_set_audio_device(STREAM_OUT, AU_DSP_AUDIO, HAL_AUDIO_DEVICE_I2S_MASTER, HAL_AUDIO_INTERFACE_1, NOT_REWRITE)) {
                LOGMSGIDI("Set I2S_MASTER error\r\n", 0);
            }
#endif
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        }
        response.response_len = strlen((char *) response.response_buf);
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=AMIC") != NULL) {
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=AMIC,0") != NULL) {
            //DMIC DUAL
#if 0
            *((volatile uint32_t *)(0xA2120B04)) &= 0xFFFFFF87;
#else
            if (AUD_EXECUTION_SUCCESS != ami_set_audio_device(STREAM_IN, AU_DSP_VOICE, HAL_AUDIO_DEVICE_DIGITAL_MIC_DUAL, HAL_AUDIO_INTERFACE_1, NOT_REWRITE)) {
                LOGMSGIDI("Set DIGITAL_MIC_DUAL error\r\n", 0);
            }
#endif
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=AMIC,1") != NULL) {
            //AMIC DUAL or LINEINPLAYBACK DUAL
#if 0
            *((volatile uint32_t *)(0xA2120B04)) &= 0xFFFFFF87;
            *((volatile uint32_t *)(0xA2120B04)) |= 0x08;
#else
            if (AUD_EXECUTION_SUCCESS != ami_set_audio_device(STREAM_IN, AU_DSP_VOICE, HAL_AUDIO_DEVICE_MAIN_MIC_DUAL, HAL_AUDIO_INTERFACE_1, NOT_REWRITE)) {
                LOGMSGIDI("Set MAIN_MIC_DUAL error\r\n", 0);
            }
#endif
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=AMIC,2") != NULL) {
#ifdef HAL_AUDIO_SUPPORT_MULTIPLE_STREAM_OUT
            //AMIC ACC mode
#ifndef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
            audio_nvdm_HW_config.UL_AFE_PARA &= 0xFD;
            audio_nvdm_HW_config.UL_AFE_PARA |= 0x02;
#else

#endif
#endif
            *((volatile uint32_t *)(0xA2120B04)) &= 0xFFFFFF87;
            *((volatile uint32_t *)(0xA2120B04)) |= 0x10;
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=AMIC,3") != NULL) {
            // AMIC mono L
#if 0
            *((volatile uint32_t *)(0xA2120B04)) &= 0xFFFFFF87;
            *((volatile uint32_t *)(0xA2120B04)) |= 0x20;
#else
            if (AUD_EXECUTION_SUCCESS != ami_set_audio_device(STREAM_IN, AU_DSP_VOICE, HAL_AUDIO_DEVICE_MAIN_MIC_L, HAL_AUDIO_INTERFACE_1, NOT_REWRITE)) {
                LOGMSGIDI("Set MAIN_MIC_L error\r\n", 0);
            }
#endif
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=AMIC,4") != NULL) {
            // AMIC mono R
#if 0
            *((volatile uint32_t *)(0xA2120B04)) &= 0xFFFFFF87;
            *((volatile uint32_t *)(0xA2120B04)) |= 0x40;
#else
            if (AUD_EXECUTION_SUCCESS != ami_set_audio_device(STREAM_IN, AU_DSP_VOICE, HAL_AUDIO_DEVICE_MAIN_MIC_R, HAL_AUDIO_INTERFACE_1, NOT_REWRITE)) {
                LOGMSGIDI("Set MAIN_MIC_R error\r\n", 0);
            }
#endif
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=AMIC,5") != NULL) {
#ifdef HAL_AUDIO_SUPPORT_MULTIPLE_STREAM_OUT
            // AMIC DCC mode
#ifndef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
            audio_nvdm_HW_config.UL_AFE_PARA &= 0xFD;
            //audio_nvdm_HW_config.UL_AFE_PARA |= 0x00;
#else
#endif
#endif
            *((volatile uint32_t *)(0xA2120B04)) &= 0xFFFFFF87;
            *((volatile uint32_t *)(0xA2120B04)) |= 0x00;
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        }
        response.response_len = strlen((char *) response.response_buf);
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=HANDLE_LOCK") != NULL) {
        int handle_index;
        char *data = NULL;
        data = strchr(parse_cmd->string_ptr, ',');
        data++;
        handle_index = atoi(data);
        LOGMSGIDI("force lock sleep handle : %d\r\n", (int)handle_index, 0);
        hal_sleep_manager_lock_sleep(handle_index);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=HANDLE_UNLOCK") != NULL) {
        int handle_index;
        char *data = NULL;
        data = strchr(parse_cmd->string_ptr, ',');
        data++;
        handle_index = atoi(data);
        LOGMSGIDI("force lock sleep handle : %d\r\n", (int)handle_index, 0);
        hal_sleep_manager_unlock_sleep(handle_index);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=DEBUG") != NULL) {
        char *data = NULL;
        data = strchr(parse_cmd->string_ptr, ',');
        data++;
#ifdef  SLEEP_MANAGEMENT_DEBUG_ENABLE
        int mode;
        mode = atoi(data);
        if (mode == 0) {
            sleep_management_debug_dump_lock_sleep_time();
        } else if (mode == 1) {
            sleep_management_debug_dump_backup_restore_time();
        } else {
            LOGMSGIDI("unknown command\r\n", 0);
        }
#else
        LOGMSGIDI("need enable 'SLEEP_MANAGEMENT_DEBUG_ENABLE' feature option\r\n", 0);
#endif
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else {
        strcpy((char *) response.response_buf, "Not Support\n");
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
        atci_send_response(&response);
        return ATCI_STATUS_ERROR;
    }
    return ATCI_STATUS_OK;
}

#endif
#elif (PRODUCT_VERSION == 3335)

#include "hal_platform.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "at_command.h"
#include "hal.h"
#include "hal_sleep_manager.h"
#include "hal_sleep_manager_internal.h"
#include "hal_spm.h"
#include "hal.h"
#include "hal_clock.h"
#include "hal_clock_internal.h"
#include "at_command.h"
#include "syslog.h"

extern ATTR_ZIDATA_IN_TCM volatile uint32_t sram_size_d;
static TaskHandle_t sleep_task_handle = NULL;

#define LOGW(fmt,arg...)   LOG_W(atcmd, "SPM: "fmt,##arg)
extern uint8_t sleep_manager_handle;
extern int sm_wfi;
extern int sm_lp;

#ifdef HAL_SLEEP_MANAGER_ENABLED
extern atci_status_t atci_cmd_hdlr_sleep_manager(atci_parse_cmd_param_t *parse_cmd);
#endif
// change the data with hex type in string to data with dec type
int sm_htoi(char s[])
{
    int i;
    int n = 0;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        i = 2;
    } else {
        i = 0;
    }
    for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z'); ++i) {
        if (tolower((unsigned char)s[i]) > '9') {
            n = 16 * n + (10 + tolower((unsigned char)s[i]) - 'a');
        } else {
            n = 16 * n + (tolower((unsigned char)s[i]) - '0');
        }
    }
    return n;
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


//extern int32_t gnss_io_send_data_to_port(char *port_name, char *buffer, int32_t length);
extern int32_t lp_test_uart_write(uint8_t port, int8_t *buffer, int32_t length);

void vTestTask_at_sleep(void *pvParameters)
{
    portTickType xLastExecutionTime;
    xLastExecutionTime = xTaskGetTickCount();
    while (1) {
        xLastExecutionTime = xTaskGetTickCount();
        vTaskDelayUntil(&xLastExecutionTime, ((portTickType) 1000 / portTICK_RATE_MS));
        log_hal_msgid_info("Enter Sleep\r\n", 0);
        __asm volatile("cpsid i");
        hal_sleep_manager_set_sleep_time(15000);
        sleep_management_enter_deep_sleep(0);
        __asm volatile("cpsie i");
        log_hal_msgid_info("Wakeup from Sleep\r\n", 0);
    }

    vTaskDelete(NULL);
}
int task_timer = 1000;
extern int debug_option ;
static void sleep_task(void *pvParameters)
{
    for (;;) {
        const TickType_t xDelay = task_timer / portTICK_PERIOD_MS;
        vTaskDelay(xDelay);
        log_hal_msgid_info("=========================", 0);
        log_hal_msgid_info("sm_wfi[%d]sm_lp[%d]", 2, sm_wfi, sm_lp);
        if (hal_sleep_manager_is_sleep_locked() > 0) {
            hal_sleep_manager_sleep_driver_dump_handle_name();
        }
#ifdef  SLEEP_MANAGEMENT_DEBUG_ENABLE
        sleep_management_debug_dump_backup_restore_time();
        sleep_management_debug_dump_lock_sleep_time();
#endif
        log_hal_msgid_info("=========================", 0);
    }
}


atci_status_t atci_cmd_hdlr_sleep_manager(atci_parse_cmd_param_t *parse_cmd)
{

    int read_value = 0;
    int input_addr = 0;
    char *end_pos = NULL;
    atci_response_t response = {{0}};
    response.response_flag = 0;
#ifdef ATCI_APB_PROXY_ADAPTER_ENABLE
    response.cmd_id = parse_cmd->cmd_id;
#endif

    if (strstr((char *) parse_cmd->string_ptr, "AT+SM=TEST") != NULL) {
        log_hal_msgid_info("SLEEP MANAGER AT_COMMAND SUPPORT ", 0);
        strcpy((char *)response.response_buf, "Test Command Pass!!\r\n");
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=SLEEP") != NULL) {
        xTaskCreate(vTestTask_at_sleep, "vTestTask_at_sleep", 2048, NULL, 2, NULL);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=HQASLEEP") != NULL) {
        end_pos = strchr(parse_cmd->string_ptr, ',');
        end_pos++;
        input_addr = sm_htoi(end_pos);
        end_pos = NULL;
        switch (input_addr) {
            case 0:
                sram_size_d = 1;
                *SPM_INFRA_SYSRAM0_CONTROL_0 = 0x0;
                *SPM_INFRA_SYSRAM1_CONTROL_0 = 0x0;
                *SPM_INFRA_SYSRAM2_CONTROL_0 = 0x0;
                break;
            case 1:
                sram_size_d = 2;
                *SPM_INFRA_SYSRAM0_CONTROL_0 = 0x0;
                *SPM_INFRA_SYSRAM1_CONTROL_0 = 0xfc;
                *SPM_INFRA_SYSRAM2_CONTROL_0 = 0x1f;
                break;
            case 2:
                sram_size_d = 3;
                *SPM_INFRA_SYSRAM0_CONTROL_0 = 0xff;
                *SPM_INFRA_SYSRAM1_CONTROL_0 = 0xff;
                *SPM_INFRA_SYSRAM2_CONTROL_0 = 0x1f;
                break;
        }
        spm_mask_wakeup_source(HAL_SLEEP_MANAGER_WAKEUP_SOURCE_ALL);
        xTaskCreate(vTestTask_at_sleep, "vTestTask_at_sleep", 2048, NULL, 2, NULL);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=RTC") != NULL) {
        log_hal_msgid_info("====[RTC]====", 0);
        hal_rtc_enter_rtc_mode();
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=IDLE") != NULL) {
        end_pos = strchr(parse_cmd->string_ptr, ',');
        end_pos++;
        input_addr = sm_htoi(end_pos);
        end_pos = NULL;
        spm_mask_wakeup_source(HAL_SLEEP_MANAGER_WAKEUP_SOURCE_ALL);
        hal_gpio_pull_down(27);
#ifdef HAL_DVFS_MODULE_ENABLEDHAL_DVFS_MODULE_ENABLED
        switch (input_addr) {
            case 0:
                log_hal_msgid_info("=======[IDLE_192Mhz_0p6v]======\r\n", 0);
                hal_clock_set_osc_freq(384000);
                hal_dvfs_target_cpu_frequency(192000, HAL_DVFS_FREQ_RELATION_L);
                break;
            case 1:
                log_hal_msgid_info("=======[IDLE_192Mhz_0p7v]======\r\n", 0);
                hal_clock_set_osc_freq(384000);
                hal_dvfs_target_cpu_frequency(384000, HAL_DVFS_FREQ_RELATION_L);
                clock_mux_sel(CLK_SYS_SEL, 3);

                break;
            case 2:
                log_hal_msgid_info("=======[IDLE_384Mhz_0p7v]======\r\n", 0);
                hal_clock_set_osc_freq(384000);
                hal_dvfs_target_cpu_frequency(384000, HAL_DVFS_FREQ_RELATION_L);
                break;

        }
        log_hal_msgid_info("======[Freq:%d]======\r\n", 1, (int)hal_clock_get_freq_meter(30, 1000));
#endif
        while (1) {
            hal_sleep_manager_enter_sleep_mode(HAL_SLEEP_MODE_IDLE);
        }
        //*XO_PDN_PD_SET0 |= 0x300000;
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=MTCMOS,") != NULL) {
        char *mid_pos = NULL;
        int input_domain = 0;
        int input_value = 0;
        mid_pos = strchr(parse_cmd->string_ptr, ',');
        mid_pos++;
        end_pos = strchr(mid_pos, ',');
        input_domain = sm_htoi(mid_pos);
        end_pos++;
        input_value = sm_htoi(end_pos);
        mid_pos = NULL;
        end_pos = NULL;
        switch (input_domain) {
            case 0:
                log_hal_msgid_info("SPM_MTCMOS_AON  %d\r\n", 1, input_value);
                spm_control_mtcmos(SPM_MTCMOS_AON, input_value);
                break;
            case 1:
                log_hal_msgid_info("SPM_MTCMOS_L1  %d\r\n", 1, input_value);
                spm_control_mtcmos(SPM_MTCMOS_L1, input_value);
                break;
            case 2:
                log_hal_msgid_info("SPM_MTCMOS_L5  %d\r\n", 1, input_value);
                spm_control_mtcmos(SPM_MTCMOS_L5, input_value);
                break;
            case 3:
                spm_control_mtcmos(SPM_MTCMOS_AON, SPM_MTCMOS_PWR_ENABLE);
                spm_control_mtcmos(SPM_MTCMOS_L1, SPM_MTCMOS_PWR_ENABLE);
                spm_control_mtcmos(SPM_MTCMOS_L5, SPM_MTCMOS_PWR_ENABLE);
                break;
            case 4:
                spm_control_mtcmos(SPM_MTCMOS_L5, SPM_MTCMOS_PWR_DISABLE);
                spm_control_mtcmos(SPM_MTCMOS_L1, SPM_MTCMOS_PWR_DISABLE);
                spm_control_mtcmos(SPM_MTCMOS_AON, SPM_MTCMOS_PWR_DISABLE);
                break;
        }
        log_hal_msgid_info("[%d][%d][%d]\r\n", 3, spm_get_mtcmos_status(SPM_MTCMOS_AON), spm_get_mtcmos_status(SPM_MTCMOS_L1), spm_get_mtcmos_status(SPM_MTCMOS_L5));
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=MASK_SOURCE,") != NULL) {
        end_pos = strchr(parse_cmd->string_ptr, ',');
        end_pos++;
        input_addr = sm_htoi(end_pos);
        end_pos = NULL;
        if (input_addr == 100) {
            log_hal_msgid_info("0:GPT 1:EINT 3:SPI 4:WDT 13:GPS_L1 15:GPS_L5 ", 0);
        }

        spm_mask_wakeup_source(input_addr);
        log_hal_msgid_info("SPM_CM4_WAKEUP_SOURCE_MASK [%x][%x]\r\n", 2, (unsigned int)SPM_CM4_WAKEUP_SOURCE_MASK, (unsigned int)*SPM_CM4_WAKEUP_SOURCE_MASK);
        log_hal_msgid_info("SPM_CM4_WAKEUP_SOURCE_MASK [%x][%x]\r\n", 2, (unsigned int)SPM_GPS_WAKEUP_SOURCE_MASK, (unsigned int)*SPM_GPS_WAKEUP_SOURCE_MASK);

        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=SPMDEBUG,") != NULL) {
        end_pos = strchr(parse_cmd->string_ptr, ',');
        end_pos++;
        input_addr = sm_htoi(end_pos);
        end_pos = NULL;
        log_hal_msgid_info("Debug setting[%x]", 1, input_addr);
        spm_debug_io(input_addr);

        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=LOCK,") != NULL) {
        end_pos = strchr(parse_cmd->string_ptr, ',');
        end_pos++;
        input_addr = sm_htoi(end_pos);
        end_pos = NULL;
        if (input_addr == 0) {
            hal_sleep_manager_unlock_sleep(sleep_manager_handle);

        } else {
            hal_sleep_manager_lock_sleep(sleep_manager_handle);
        }
        hal_sleep_manager_sleep_driver_dump_handle_name();
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=STA") != NULL) {
        hal_sleep_manager_sleep_driver_dump_handle_name();
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=LOW_STAGE,") != NULL) {
        char *mid_pos = NULL;
        int input_domain = 0;
        int input_value = 0;
        mid_pos = strchr(parse_cmd->string_ptr, ',');
        mid_pos++;
        end_pos = strchr(mid_pos, ',');
        input_domain = sm_htoi(mid_pos);
        end_pos++;
        input_value = sm_htoi(end_pos);
        mid_pos = NULL;
        end_pos = NULL;
        switch (input_domain) {
            case 0:
                log_hal_msgid_info("[low power flag %d]", 1, sleep_management_get_low_power_flag());
                break;

            case 1:
                sleep_management_set_low_power_flag(input_value);
                break;
        }

        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=WADBG") != NULL) {
        end_pos = strchr(parse_cmd->string_ptr, ',');
        end_pos++;
        input_addr = sm_htoi(end_pos);
        end_pos = NULL;
        log_hal_msgid_info("Debug setting[%x]", 1, input_addr);
        debug_option = input_addr;

        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=TICKCHECK,") != NULL) {
        end_pos = strchr(parse_cmd->string_ptr, ',');
        end_pos++;
        input_addr = sm_htoi(end_pos);
        end_pos = NULL;
        char *mid_pos = NULL;
        int input_domain = 0;
        int input_value = 0;
        char *end_pos = NULL;
        mid_pos = strchr(parse_cmd->string_ptr, ',');
        mid_pos++;
        end_pos = strchr(mid_pos, ',');
        input_domain = sm_htoi(mid_pos);
        end_pos++;
        input_value = sm_htoi(end_pos);
        task_timer = ctoi(end_pos);
        mid_pos = NULL;
        end_pos = NULL;
        LOG_MSGID_I(common, "input_value1:%d, input_value2:%d\r\n", 2, input_domain, input_value);
        switch (input_domain) {
            case 0:
                vTaskDelete(sleep_task_handle);
                sleep_task_handle = NULL;
                break;

            case 1 :
                if (sleep_task_handle == NULL) {
                    xTaskCreate(sleep_task, "sleep_task", 512, (void *)&input_value, tskIDLE_PRIORITY, &sleep_task_handle);
                }
                break;

            case 2:
                vTaskSuspend(sleep_task_handle);
                break;

            case 3:
                vTaskResume(sleep_task_handle);
                break;
        }
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=D_DEBUG") != NULL) {
        log_hal_msgid_info("PDN_PD_COND0 [%x][%x]\r\n", 2, (unsigned int)PDN_PD_COND0, (unsigned int)*PDN_PD_COND0);
        log_hal_msgid_info("XO_PDN_AO_COND0 [%x][%x]\r\n", 2, (unsigned int)XO_PDN_AO_COND0, (unsigned int)*XO_PDN_AO_COND0);
        log_hal_msgid_info("XO_PDN_TOP_COND0 [%x][%x]\r\n", 2, (unsigned int)XO_PDN_TOP_COND0, (unsigned int)*XO_PDN_TOP_COND0);
        log_hal_msgid_info("XO_PDN_PD_COND0 [%x][%x]\r\n", 2, (unsigned int)XO_PDN_PD_COND0, (unsigned int)*XO_PDN_PD_COND0);
        log_hal_msgid_info("2[%d][%d][%d]\r\n", 3, spm_get_mtcmos_status(SPM_MTCMOS_AON), spm_get_mtcmos_status(SPM_MTCMOS_L1), spm_get_mtcmos_status(SPM_MTCMOS_L5));
        log_hal_msgid_info("[%x][%x]\r\n", 2, (unsigned int)((volatile uint32_t *)(0xe000e200)), (unsigned int) * ((volatile uint32_t *)(0xe000e200)));
        log_hal_msgid_info("[%x][%x]\r\n", 2, (unsigned int)((volatile uint32_t *)(0xe000e204)), (unsigned int) * ((volatile uint32_t *)(0xe000e204)));
        log_hal_msgid_info("[%x][%x]\r\n", 2, (unsigned int)((volatile uint32_t *)(0xe000e100)), (unsigned int) * ((volatile uint32_t *)(0xe000e100)));
        log_hal_msgid_info("[%x][%x]\r\n", 2, (unsigned int)((volatile uint32_t *)(0xe000e104)), (unsigned int) * ((volatile uint32_t *)(0xe000e104)));
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=DUMP_SPM") != NULL) {
        int i = 0;
        uint32_t base_addr = 0xA2100000;
        for (i = 0; i < 252; i++) {
            log_hal_msgid_info("[%x][%x]\r\n", 2, (unsigned int)((volatile uint32_t *)base_addr), (unsigned int) * ((volatile uint32_t *)base_addr));
            base_addr += 0x4;
        }
        log_hal_msgid_info("PDN_PD_COND0 [%x][%x]\r\n", 2, (unsigned int)PDN_PD_COND0, (unsigned int)*PDN_PD_COND0);
        log_hal_msgid_info("XO_PDN_AO_COND0 [%x][%x]\r\n", 2, (unsigned int)XO_PDN_AO_COND0, (unsigned int)*XO_PDN_AO_COND0);
        log_hal_msgid_info("XO_PDN_TOP_COND0 [%x][%x]\r\n", 2, (unsigned int)XO_PDN_TOP_COND0, (unsigned int)*XO_PDN_TOP_COND0);
        log_hal_msgid_info("XO_PDN_PD_COND0 [%x][%x]\r\n", 2, (unsigned int)XO_PDN_PD_COND0, (unsigned int)*XO_PDN_PD_COND0);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=RREG,") != NULL) {
        /*command: AT+EPMUREG=0,1234*/
        end_pos = strchr(parse_cmd->string_ptr, ',');
        end_pos++;
        input_addr = sm_htoi(end_pos);
        end_pos = NULL;
        read_value = *((volatile uint32_t *)(input_addr));
        log_hal_msgid_info("[0x%x  :0x%x]\r\n", 2, (unsigned int) input_addr, (unsigned int)read_value);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=WREG,") != NULL) {

        /*command: AT+EPMUREG=1,1234,456*/
        char *mid_pos = NULL;
        int input_domain = 0;
        int input_value = 0;
        mid_pos = strchr(parse_cmd->string_ptr, ',');
        mid_pos++;
        end_pos = strchr(mid_pos, ',');
        input_domain = sm_htoi(mid_pos);
        end_pos++;
        input_value = sm_htoi(end_pos);
        mid_pos = NULL;
        end_pos = NULL;
        log_hal_msgid_info("input_domain:0x%x, input_value:0x%x\r\n", 2, (unsigned int)input_domain, (unsigned int) input_value);
        /* write input data to input register address*/
        *((volatile uint32_t *)(input_domain)) = input_value;
        /* ATCI will help append "OK" at the end of resonse buffer    */
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);

    } else {
        strcpy((char *) response.response_buf, "Not Support\n");
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
        atci_send_response(&response);
        return ATCI_STATUS_ERROR;
    }
    return ATCI_STATUS_OK;
}

#elif ( defined(AIR_BTA_IC_PREMIUM_G2) )
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "at_command.h"
#include "syslog.h"
#ifdef HAL_AUDIO_MODULE_ENABLED
#include "bt_sink_srv_ami.h"
#endif
#ifdef HAL_SLEEP_MANAGER_ENABLED

#include "hal_sleep_manager.h"
#include "hal_sleep_manager_internal.h"
#include "hal_spm.h"
#include "hal_pmu.h"
#include "nvdm.h"
#include "hal_rtc.h"
#include "hal_clock_internal.h"
#include <stdlib.h>
#ifdef MTK_SWLA_ENABLE
#include "swla.h"
#endif

log_create_module(atci_slp, PRINT_LEVEL_INFO);
#define LOGI(fmt,arg...)   LOG_I(atci_slp ,"ATCMD: "fmt,##arg)
#define LOGMSGIDI(fmt,cnt,arg...)   LOG_MSGID_I(atci_slp ,"ATCMD: "fmt,cnt,##arg)

extern uint8_t sleep_manager_handle;
extern void bsp_ept_gpio_setting_init(void);
#ifdef HAL_PMU_MODULE_ENABLED
extern void pmu_sw_enter_sleep(pmu_power_domain_t domain);
#endif
extern uint32_t clock_get_pll_state(clock_pll_id pll_id);
extern void rtc_32k_off(void);

#ifdef HAL_SLEEP_MANAGER_ENABLED
extern atci_status_t atci_cmd_hdlr_sleep_manager(atci_parse_cmd_param_t *parse_cmd);
#endif

int sm_htoi(char s[])
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

#ifdef HAL_RTC_MODULE_ENABLED
static void rtc_alarm_setting()
{
    hal_rtc_time_t time, rtc_time;

    hal_rtc_get_time(&time);
    rtc_time.rtc_year = time.rtc_year;
    rtc_time.rtc_mon = time.rtc_mon;
    rtc_time.rtc_day = time.rtc_day;
    rtc_time.rtc_hour = time.rtc_hour;
    rtc_time.rtc_min = time.rtc_min;
    rtc_time.rtc_sec = time.rtc_sec;
    rtc_time.rtc_week = time.rtc_week;
    rtc_time.rtc_milli_sec = time.rtc_milli_sec;
    if (time.rtc_sec + 25 > 59) {
        rtc_time.rtc_sec = time.rtc_sec + 25 - 60;
        rtc_time.rtc_min = time.rtc_min + 1;
    } else {
        rtc_time.rtc_sec = time.rtc_sec + 25;
    }
    hal_rtc_set_alarm(&rtc_time);
    LOGMSGIDI("alarm : %d/%d/%d %d:%d:%d(%d)\r\n", 7, rtc_time.rtc_year, rtc_time.rtc_mon, rtc_time.rtc_day,
              rtc_time.rtc_hour, rtc_time.rtc_min, rtc_time.rtc_sec, rtc_time.rtc_milli_sec);
}

void enter_rtc_off_mode()
{
    LOGMSGIDI("Enter RTC Off Mode\r\n", 0);
    hal_gpt_delay_ms(100);
    rtc_32k_off();
    hal_gpt_delay_ms(2000);
    hal_rtc_enter_rtc_mode();
}

extern bool back_from_rtc_mode;
void enter_rtc_mode()
{
#ifdef MTK_HAL_EXT_32K_ENABLE
    LOGMSGIDI("enter_rtc_mode : RTC_32K_XTAL\r\n", 0);
#else
    LOGMSGIDI("enter_rtc_mode : RTC_32KLess\r\n", 0);
#endif

    hal_gpt_delay_ms(200);
    if (back_from_rtc_mode == false) {
        rtc_alarm_setting();
        hal_rtc_enable_alarm();
        hal_rtc_enter_rtc_mode();
    } else {
        LOGMSGIDI("rtc mode wakeup by alarm test pass\r\n", 0);
        rtc_alarm_setting();
        hal_rtc_disable_alarm();
        hal_rtc_enter_rtc_mode();
    }
}

extern bool    hal_rtc_is_back_from_rtcmode();
void rtc_alarm_irq_test(void)
{
    if (hal_rtc_is_back_from_rtcmode() == false) {
        rtc_alarm_setting();
        hal_rtc_enable_alarm();
        hal_rtc_enter_rtc_mode();
    } else {
        LOGMSGIDI("rtc mode wakeup by alarm test pass\r\n", 0);
        rtc_alarm_setting();
        hal_rtc_disable_alarm();
        hal_rtc_enter_rtc_mode();
    }
}
#endif
#ifdef HAL_AUDIO_SUPPORT_MULTIPLE_STREAM_OUT
extern HAL_DSP_PARA_AU_AFE_CTRL_t audio_nvdm_HW_config;
#endif
atci_status_t atci_cmd_hdlr_sleep_manager(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}};
    response.response_flag = 0;

    if (strstr((char *)parse_cmd->string_ptr, "AT+SM=0") != NULL) {
        strncpy((char *)response.response_buf, "Low Power Test Command Pass!!\r\n", strlen("Low Power Test Command Pass!!\r\n") + 1);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    }  else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=BASE") != NULL) {
        __asm volatile("cpsid i");
        uint32_t mask;

        //spm_control_mtcmos(SPM_MTCMOS_PERISYS, SPM_MTCMOS_PWR_DISABLE);

        int i;
        for (i = 0; i < 32; i++) {

            if (hal_nvic_get_pending_irq(i) == 1) {
                hal_nvic_disable_irq(i);
                log_hal_msgid_info("pending_irq:%d\r\n", 1, i);
                hal_nvic_clear_pending_irq(i);
                hal_nvic_disable_irq(i);
            }
        }
        for (i = 0; i < 32; i++) {

            if (hal_nvic_get_pending_irq(i) == 1) {
                log_hal_msgid_info("pending_irq:%d\r\n", 1, i);
                hal_nvic_clear_pending_irq(i);
                hal_nvic_disable_irq(i);
            }
        }
        hal_nvic_disable_irq(CRYPTO_IRQn);
        hal_nvic_disable_irq(SPM_IRQn);

        *SPM_IGNORE_CPU_ACTIVE |= 0x100;    //IGNORE DSP ACTIVE
        *SPM_CM4_WAKEUP_SOURCE_MASK = 0xFFFFFFFF;
        *SPM_DSP_WAKEUP_SOURCE_MASK = 0xFFFFFFFF;

        while (1) {
            hal_nvic_clear_pending_irq(20);
            *SPM_CONN_AUDIO_ABB_XO_SIDEBAND = 0;        //for sleep test, need BT or Audio Control.
            hal_nvic_save_and_set_interrupt_mask(&mask);
#ifdef MTK_LOCK_S1
            *SPM_SPM_STATE_CONTROL_0 = 0;
#endif
            hal_sleep_manager_enter_sleep_mode(HAL_SLEEP_MODE_SLEEP);
            hal_nvic_restore_interrupt_mask(mask);
            *SPM_CONN_AUDIO_ABB_XO_SIDEBAND = 0x101;    //for sleep test, need BT or Audio Control.
        }

        response.response_len = strlen((char *) response.response_buf);
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=DEBUGIO") != NULL) {

        char *end_pos = NULL;
        int input_value = 0;
        end_pos = strchr(parse_cmd->string_ptr, ',');
        end_pos ++;

        input_value = sm_htoi(end_pos);
        end_pos = NULL;

        LOGMSGIDI("set DEBUGIO:0x%x\r\n", 1, input_value);

        strncpy((char *)response.response_buf, "Set_SPM_debug_bus\r\n", strlen("Set_SPM_debug_bus\r\n") + 1);
        spm_debug_io(input_value);

        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=HANDLE_LOCK") != NULL) {
        int handle_index;
        char *data = NULL;
        data = strchr(parse_cmd->string_ptr, ',');
        data++;
        handle_index = atoi(data);
        LOGMSGIDI("force lock sleep handle : %d\r\n", (int)handle_index, 0);
        hal_sleep_manager_lock_sleep(handle_index);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=HANDLE_UNLOCK") != NULL) {
        int handle_index;
        char *data = NULL;
        data = strchr(parse_cmd->string_ptr, ',');
        data++;
        handle_index = atoi(data);
        LOGMSGIDI("force lock sleep handle : %d\r\n", (int)handle_index, 0);
        hal_sleep_manager_unlock_sleep(handle_index);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=LOCK") != NULL) {
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=LOCK,0,SLP") != NULL) {
            hal_sleep_manager_unlock_sleep(sleep_manager_handle);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            strncpy((char *)response.response_buf, "unlock sleep manager handle\r\n", strlen("unlock sleep manager handle\r\n") + 1);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=LOCK,1,SLP") != NULL) {
            hal_sleep_manager_lock_sleep(sleep_manager_handle);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            strncpy((char *)response.response_buf, "lock sleep manager handle\r\n", strlen("lock sleep manager handle\r\n") + 1);
        }
        response.response_len = strlen((char *) response.response_buf);
        atci_send_response(&response);
    } else if (strstr((char *)parse_cmd->string_ptr, "AT+SM=SET_GPIO") != NULL) {
        LOGMSGIDI("AT+SM=SET_GPIO", 0);
        hal_gpio_pin_t pin;
        uint8_t function_index;
        char *config_s = NULL;
        int ret1, ret2;
        config_s = strchr((char *)parse_cmd->string_ptr, ',');
        config_s++;

        ret1 = sscanf(config_s, "%d", (int *)&pin);
        config_s = strchr(config_s, ',');
        config_s++;
        ret2 = sscanf(config_s, "%d", (int *)&function_index);
        hal_gpio_init(pin);

        if ((ret1 != 1) || (ret2 != 1)) {
            LOGMSGIDI("AT+SM=SET_GPIO set fail\r\n", 0);
        }

        hal_pinmux_set_function(pin, function_index);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR") != NULL) {
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,DSP,0") != NULL) {
            *SPM_IGNORE_CPU_ACTIVE |= 0x100;    //IGNORE DSP ACTIVE
            strncpy((char *)response.response_buf, "DSP Power Off\r\n", strlen("DSP Power Off\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_DSP, SPM_MTCMOS_PWR_DISABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,DSP,1") != NULL) {
            strncpy((char *)response.response_buf, "DSP Power On\r\n", strlen("DSP Power On\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_DSP, SPM_MTCMOS_PWR_ENABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,CONN_TOP_ON,0") != NULL) {
            strncpy((char *)response.response_buf, "CONN_TOP_ON Power Off\r\n", strlen("CONN_TOP_ON Power Off\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_CONN_TOP_ON, SPM_MTCMOS_PWR_DISABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,CONN_TOP_ON,1") != NULL) {
            strncpy((char *)response.response_buf, "CONN_TOP_ON Power On\r\n", strlen("CONN_TOP_ON Power On\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_CONN_TOP_ON, SPM_MTCMOS_PWR_ENABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,CONN_TOP_OFF,0") != NULL) {
            strncpy((char *)response.response_buf, "CONN_TOP_OFF Power Off\r\n", strlen("CONN_TOP_OFF Power Off\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_CONN_TOP_OFF, SPM_MTCMOS_PWR_DISABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,CONN_TOP_OFF,1") != NULL) {
            strncpy((char *)response.response_buf, "CONN_TOP_OFF Power On\r\n", strlen("CONN_TOP_OFF Power On\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_CONN_TOP_OFF, SPM_MTCMOS_PWR_ENABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,AUDIO,0") != NULL) {
            strncpy((char *)response.response_buf, "AUDIO Power Off\r\n", strlen("AUDIO Power Off\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO, SPM_MTCMOS_PWR_DISABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,AUDIO,1") != NULL) {
            strncpy((char *)response.response_buf, "AUDIO Power On\r\n", strlen("AUDIO Power On\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO, SPM_MTCMOS_PWR_ENABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,PERISYS,0") != NULL) {
            strncpy((char *)response.response_buf, "PERISYS Power Off\r\n", strlen("PERISYS Power Off\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_PERISYS, SPM_MTCMOS_PWR_DISABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,PERISYS,1") != NULL) {
            strncpy((char *)response.response_buf, "PERISYS Power On\r\n", strlen("PERISYS Power On\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_PERISYS, SPM_MTCMOS_PWR_ENABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,VOW,0") != NULL) {
            strncpy((char *)response.response_buf, "VOW Power Off\r\n", strlen("VOW Power Off\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_VOW, SPM_MTCMOS_PWR_DISABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,VOW,1") != NULL) {
            strncpy((char *)response.response_buf, "VOW Power On\r\n", strlen("VOW Power On\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_VOW, SPM_MTCMOS_PWR_ENABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,ALL,0") != NULL) {
            strncpy((char *)response.response_buf, "DSP/AUDIO/CONN TOP ON/CONN TOP OFF/VOW MTCMOS Off\r\n", strlen("DSP/AUDIO/CONN TOP ON/CONN TOP OFF/VOW MTCMOS Off\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_CONN_TOP_ON, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_CONN_TOP_OFF, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_DSP, SPM_MTCMOS_PWR_DISABLE);
            //spm_control_mtcmos(SPM_MTCMOS_PERISYS, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_VOW, SPM_MTCMOS_PWR_DISABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,ALL,1") != NULL) {
            strncpy((char *)response.response_buf, "DSP/AUDIO/CONN TOP ON/CONN TOP OFF/VOW MTCMOS On\r\n", strlen("DSP/AUDIO/CONN TOP ON/CONN TOP OFF/VOW MTCMOS On\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_CONN_TOP_ON, SPM_MTCMOS_PWR_ENABLE);
            spm_control_mtcmos(SPM_MTCMOS_CONN_TOP_OFF, SPM_MTCMOS_PWR_ENABLE);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO, SPM_MTCMOS_PWR_ENABLE);
            spm_control_mtcmos(SPM_MTCMOS_DSP, SPM_MTCMOS_PWR_ENABLE);
            //spm_control_mtcmos(SPM_MTCMOS_PERISYS, SPM_MTCMOS_PWR_ENABLE);
            spm_control_mtcmos(SPM_MTCMOS_VOW, SPM_MTCMOS_PWR_ENABLE);
        }
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=RTC") != NULL) {
#ifdef HAL_RTC_MODULE_ENABLED
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=RTC,0") != NULL) {
            hal_rtc_enter_rtc_mode();
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=RTC,1") != NULL) {
            //enter_rtc_mode();
            /*
            RTC_32K_XTAL : MTK_HAL_EXT_32K_ENABLE = y
            RTC_32KLess  : MTK_HAL_EXT_32K_ENABLE = n

#ifdef MTK_HAL_EXT_32K_ENABLE
            LOGMSGIDI("LPHQA_enter_rtc_mode : RTC_32K_XTAL\r\n" ,0);
#else
            LOGMSGIDI("LPHQA_enter_rtc_mode : RTC_32KLess\r\n" ,0);
#endif*/
            //hal_gpt_delay_ms(200);
            rtc_alarm_irq_test();
            }
            response.response_len = strlen((char *) response.response_buf);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&response);
#else
        strncpy((char *)response.response_buf, "Not Support\n", strlen("Not Support\n"));
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
        atci_send_response(&response);
#endif
            } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=STATUS") != NULL) {
            hal_gpt_delay_ms(200);
            if (hal_sleep_manager_is_sleep_locked()) {
            sleep_management_get_lock_sleep_handle_list();
            } else {
            LOGMSGIDI("\r\nAny Sleep Handle not locked\r\n", 0);
            }
            response.response_len = strlen((char *) response.response_buf);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&response);
            } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=DEBUG") != NULL) {
            char *data = NULL;
            data = strchr(parse_cmd->string_ptr, ',');
            data++;
#ifdef  SLEEP_MANAGEMENT_DEBUG_ENABLE
            int mode;
            mode = atoi(data);
            if (mode == 0) {
            sleep_management_debug_dump_lock_sleep_time();
            } else if (mode == 1) {
            sleep_management_debug_dump_backup_restore_time();
            } else {
            LOGMSGIDI("unknown command\r\n", 0);
            }
#else
            LOGMSGIDI("need enable 'SLEEP_MANAGEMENT_DEBUG_ENABLE' feature option\r\n", 0);
#endif
            response.response_len = strlen((char *) response.response_buf);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&response);
            } else {
            strncpy((char *)response.response_buf, "Not Support\n", strlen("Not Support\n") + 1);
            response.response_len = strlen((char *) response.response_buf);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&response);
            return ATCI_STATUS_ERROR;
            }
            return ATCI_STATUS_OK;
            }
#endif

#elif defined (AIR_BTA_IC_PREMIUM_G3)

#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "at_command.h"
#include "syslog.h"
#ifdef HAL_AUDIO_MODULE_ENABLED
#include "bt_sink_srv_ami.h"
#endif
#ifdef HAL_SLEEP_MANAGER_ENABLED

#include "hal_sleep_manager.h"
#include "hal_sleep_manager_internal.h"
#include "hal_spm.h"
#include "hal_pmu.h"
#include "nvdm.h"
#include "hal_rtc.h"
#include "hal_clock_internal.h"
#include <stdlib.h>
#ifdef MTK_SWLA_ENABLE
#include "swla.h"
#endif

log_create_module(atci_slp, PRINT_LEVEL_INFO);
#define LOGI(fmt,arg...)   LOG_I(atci_slp ,"ATCMD: "fmt,##arg)
#define LOGMSGIDI(fmt,cnt,arg...)   LOG_MSGID_I(atci_slp ,"ATCMD: "fmt,cnt,##arg)

extern uint8_t sleep_manager_handle;
extern void bsp_ept_gpio_setting_init(void);
#ifdef HAL_PMU_MODULE_ENABLED
extern void pmu_sw_enter_sleep(pmu_power_domain_t domain);
#endif
extern uint32_t clock_get_pll_state(clock_pll_id pll_id);
extern void rtc_32k_off(void);

#ifdef HAL_SLEEP_MANAGER_ENABLED
extern atci_status_t atci_cmd_hdlr_sleep_manager(atci_parse_cmd_param_t *parse_cmd);
#endif

int sm_htoi(char s[])
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

#ifdef HAL_RTC_MODULE_ENABLED
static void rtc_alarm_setting()
{
    hal_rtc_time_t time, rtc_time;

    hal_rtc_get_time(&time);
    rtc_time.rtc_year = time.rtc_year;
    rtc_time.rtc_mon = time.rtc_mon;
    rtc_time.rtc_day = time.rtc_day;
    rtc_time.rtc_hour = time.rtc_hour;
    rtc_time.rtc_min = time.rtc_min;
    rtc_time.rtc_sec = time.rtc_sec;
    rtc_time.rtc_week = time.rtc_week;
    rtc_time.rtc_milli_sec = time.rtc_milli_sec;
    if (time.rtc_sec + 25 > 59) {
        rtc_time.rtc_sec = time.rtc_sec + 25 - 60;
        rtc_time.rtc_min = time.rtc_min + 1;
    } else {
        rtc_time.rtc_sec = time.rtc_sec + 25;
    }
    hal_rtc_set_alarm(&rtc_time);
    LOGMSGIDI("alarm : %d/%d/%d %d:%d:%d(%d)\r\n", 7, rtc_time.rtc_year, rtc_time.rtc_mon, rtc_time.rtc_day,
              rtc_time.rtc_hour, rtc_time.rtc_min, rtc_time.rtc_sec, rtc_time.rtc_milli_sec);
}

void enter_rtc_off_mode()
{
    LOGMSGIDI("Enter RTC Off Mode\r\n", 0);
    hal_gpt_delay_ms(100);
    rtc_32k_off();
    hal_gpt_delay_ms(2000);
    hal_rtc_enter_rtc_mode();
}

extern bool back_from_rtc_mode;
void enter_rtc_mode()
{
#ifdef MTK_HAL_EXT_32K_ENABLE
    LOGMSGIDI("enter_rtc_mode : RTC_32K_XTAL\r\n", 0);
#else
    LOGMSGIDI("enter_rtc_mode : RTC_32KLess\r\n", 0);
#endif

    hal_gpt_delay_ms(200);
    if (back_from_rtc_mode == false) {
        rtc_alarm_setting();
        hal_rtc_enable_alarm();
        hal_rtc_enter_rtc_mode();
    } else {
        LOGMSGIDI("rtc mode wakeup by alarm test pass\r\n", 0);
        rtc_alarm_setting();
        hal_rtc_disable_alarm();
        hal_rtc_enter_rtc_mode();
    }
}

extern bool    hal_rtc_is_back_from_rtcmode();
void rtc_alarm_irq_test(void)
{
    if (hal_rtc_is_back_from_rtcmode() == false) {
        rtc_alarm_setting();
        hal_rtc_enable_alarm();
        hal_rtc_enter_rtc_mode();
    } else {
        LOGMSGIDI("rtc mode wakeup by alarm test pass\r\n", 0);
        rtc_alarm_setting();
        hal_rtc_disable_alarm();
        hal_rtc_enter_rtc_mode();
    }
}
#endif
#ifdef HAL_AUDIO_SUPPORT_MULTIPLE_STREAM_OUT
extern HAL_DSP_PARA_AU_AFE_CTRL_t audio_nvdm_HW_config;
#endif
atci_status_t atci_cmd_hdlr_sleep_manager(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t response = {{0}};
    response.response_flag = 0;

    if (strstr((char *)parse_cmd->string_ptr, "AT+SM=0") != NULL) {
        strncpy((char *)response.response_buf, "Low Power Test Command Pass!!\r\n", strlen("Low Power Test Command Pass!!\r\n") + 1);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    }  else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=BASE") != NULL) {
        __asm volatile("cpsid i");
        uint32_t mask;

        int i;
        for (i = 0; i < 75; i++) {

            if (hal_nvic_get_pending_irq(i) == 1) {
                hal_nvic_disable_irq(i);
                log_hal_msgid_info("pending_irq:%d\r\n", 1, i);
                hal_nvic_clear_pending_irq(i);
                hal_nvic_disable_irq(i);
            }
        }
        for (i = 0; i < 75; i++) {

            if (hal_nvic_get_pending_irq(i) == 1) {
                log_hal_msgid_info("pending_irq:%d\r\n", 1, i);
                hal_nvic_clear_pending_irq(i);
                hal_nvic_disable_irq(i);
            }
        }
        hal_nvic_disable_irq(CRYPTO_IRQn);
        hal_nvic_disable_irq(SPM_IRQn);

        *SPM_IGNORE_CPU_ACTIVE |= 0x100;    //IGNORE DSP ACTIVE
        *SPM_CMSYS_WAKEUP_SOURCE_MASK = 0xFFFFFFFF;
        *SPM_DSP_WAKEUP_SOURCE_MASK = 0xFFFFFFFF;

#if defined(AIR_BTA_IC_PREMIUM_G3_TYPE_A) || defined(AIR_BTA_IC_PREMIUM_G3_TYPE_S)
        pmu_set_audio_mode(PMU_CLASSD, PMU_OFF);
        pmu_enable_micbias(PMIC_MICBIAS_LDO0, PMIC_MICBIAS0, PMIC_MICBIAS_NM, PMU_OFF);
        pmu_enable_micbias(PMIC_MICBIAS_LDO1, PMIC_MICBIAS1, PMIC_MICBIAS_NM, PMU_OFF);
        pmu_enable_micbias(PMIC_MICBIAS_LDO2, PMIC_MICBIAS2, PMIC_MICBIAS_NM, PMU_OFF);
#endif

        while (1) {
            hal_nvic_clear_pending_irq(GPT_IRQn);
            *SPM_CONN_AUDIO_ABB_XO_SIDEBAND = 0;        //for sleep test, need BT or Audio Control.
            hal_nvic_save_and_set_interrupt_mask(&mask);
            __asm volatile("cpsid i");
#ifdef MTK_LOCK_S1
            *SPM_SPM_STATE_CONTROL_0 = 0;
#endif
            hal_sleep_manager_enter_sleep_mode(HAL_SLEEP_MODE_SLEEP);
            __asm volatile("cpsie i");
            hal_nvic_restore_interrupt_mask(mask);
            *SPM_CONN_AUDIO_ABB_XO_SIDEBAND = 0x101;    //for sleep test, need BT or Audio Control.
        }

        response.response_len = strlen((char *) response.response_buf);
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=DEBUGIO") != NULL) {

        char *end_pos = NULL;
        int input_value = 0;
        end_pos = strchr(parse_cmd->string_ptr, ',');
        end_pos ++;

        input_value = sm_htoi(end_pos);
        end_pos = NULL;

        LOGMSGIDI("set DEBUGIO:0x%x\r\n", 1, input_value);

        strncpy((char *)response.response_buf, "Set_SPM_debug_bus\r\n", strlen("Set_SPM_debug_bus\r\n") + 1);
        spm_debug_io(input_value);

        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=HANDLE_LOCK") != NULL) {
        int handle_index;
        char *data = NULL;
        data = strchr(parse_cmd->string_ptr, ',');
        data++;
        handle_index = atoi(data);
        LOGMSGIDI("force lock sleep handle : %d\r\n", 1, (int)handle_index);
        hal_sleep_manager_lock_sleep(handle_index);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=HANDLE_UNLOCK") != NULL) {
        int handle_index;
        char *data = NULL;
        data = strchr(parse_cmd->string_ptr, ',');
        data++;
        handle_index = atoi(data);
        LOGMSGIDI("force unlock sleep handle : %d\r\n", 1, (int)handle_index);
        hal_sleep_manager_unlock_sleep(handle_index);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=LOCK") != NULL) {
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=LOCK,0,SLP") != NULL) {
            hal_sleep_manager_unlock_sleep(sleep_manager_handle);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            strncpy((char *)response.response_buf, "unlock sleep manager handle\r\n", strlen("unlock sleep manager handle\r\n") + 1);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=LOCK,1,SLP") != NULL) {
            hal_sleep_manager_lock_sleep(sleep_manager_handle);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            strncpy((char *)response.response_buf, "lock sleep manager handle\r\n", strlen("lock sleep manager handle\r\n") + 1);
        }
        response.response_len = strlen((char *) response.response_buf);
        atci_send_response(&response);
    } else if (strstr((char *)parse_cmd->string_ptr, "AT+SM=SET_GPIO") != NULL) {
        LOGMSGIDI("AT+SM=SET_GPIO", 0);
        hal_gpio_pin_t pin;
        uint8_t function_index;
        char *config_s = NULL;
        int ret1, ret2;
        config_s = strchr((char *)parse_cmd->string_ptr, ',');
        config_s++;

        ret1 = sscanf(config_s, "%d", (int *)&pin);
        config_s = strchr(config_s, ',');
        config_s++;
        ret2 = sscanf(config_s, "%d", (int *)&function_index);
        hal_gpio_init(pin);

        if ((ret1 != 1) || (ret2 != 1)) {
            LOGMSGIDI("AT+SM=SET_GPIO set fail\r\n", 0);
        }

        hal_pinmux_set_function(pin, function_index);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR") != NULL) {
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,DSP,0") != NULL) {
            *SPM_IGNORE_CPU_ACTIVE |= 0x100;    //IGNORE DSP ACTIVE
            strncpy((char *)response.response_buf, "DSP Power Off\r\n", strlen("DSP Power Off\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_DSP, SPM_MTCMOS_PWR_DISABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,DSP,1") != NULL) {
            strncpy((char *)response.response_buf, "DSP Power On\r\n", strlen("DSP Power On\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_DSP, SPM_MTCMOS_PWR_ENABLE);
            *SPM_IGNORE_CPU_ACTIVE &= (~(1 << 8));    //IGNORE DSP ACTIVE
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,CONN_TOP,0") != NULL) {
            strncpy((char *)response.response_buf, "CONN_TOP_ON Power Off\r\n", strlen("CONN_TOP Power Off\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_CONN_TOP_OFF, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_CONN_TOP_ON, SPM_MTCMOS_PWR_DISABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,CONN_TOP,1") != NULL) {
            strncpy((char *)response.response_buf, "CONN_TOP_ON Power On\r\n", strlen("CONN_TOP Power On\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_CONN_TOP_ON, SPM_MTCMOS_PWR_ENABLE);
            spm_control_mtcmos(SPM_MTCMOS_CONN_TOP_OFF, SPM_MTCMOS_PWR_ENABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,AUDIO_SYS,0") != NULL) {
            strncpy((char *)response.response_buf, "AUDIO_SYS Power Off\r\n", strlen("AUDIO_SYS Power Off\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO_SYS, SPM_MTCMOS_PWR_DISABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,AUDIO_SYS,1") != NULL) {
            strncpy((char *)response.response_buf, "AUDIO_SYS Power On\r\n", strlen("AUDIO_SYS Power On\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO_SYS, SPM_MTCMOS_PWR_ENABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,AUDIO_HS,0") != NULL) {
            strncpy((char *)response.response_buf, "AUDIO_HS Power Off\r\n", strlen("AUDIO_HS Power Off\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO_HS, SPM_MTCMOS_PWR_DISABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,AUDIO_HS,1") != NULL) {
            strncpy((char *)response.response_buf, "AUDIO_HS Power On\r\n", strlen("AUDIO_HS Power On\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO_HS, SPM_MTCMOS_PWR_ENABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,AUDIO_ANC,0") != NULL) {
            strncpy((char *)response.response_buf, "AUDIO_ANC Power Off\r\n", strlen("AUDIO_ANC Power Off\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO_ANC, SPM_MTCMOS_PWR_DISABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,AUDIO_ANC,1") != NULL) {
            strncpy((char *)response.response_buf, "AUDIO_ANC Power On\r\n", strlen("AUDIO_ANC Power On\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO_ANC, SPM_MTCMOS_PWR_ENABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,PERISYS,0") != NULL) {
            strncpy((char *)response.response_buf, "PERISYS Power Off\r\n", strlen("PERISYS Power Off\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_PERISYS, SPM_MTCMOS_PWR_DISABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,PERISYS,1") != NULL) {
            strncpy((char *)response.response_buf, "PERISYS Power On\r\n", strlen("PERISYS Power On\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_PERISYS, SPM_MTCMOS_PWR_ENABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,VOW,0") != NULL) {
            strncpy((char *)response.response_buf, "VOW Power Off\r\n", strlen("VOW Power Off\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_VOW, SPM_MTCMOS_PWR_DISABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,VOW,1") != NULL) {
            strncpy((char *)response.response_buf, "VOW Power On\r\n", strlen("VOW Power On\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_VOW, SPM_MTCMOS_PWR_ENABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,ALL,0") != NULL) {
            strncpy((char *)response.response_buf, "DSP/AUDIO/CONN TOP ON/CONN TOP OFF/VOW MTCMOS Off\r\n", strlen("DSP/AUDIO/CONN TOP ON/CONN TOP OFF/VOW MTCMOS Off\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_CONN_TOP_OFF, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_CONN_TOP_ON, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO_HS, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO_ANC, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO_SYS, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_DSP, SPM_MTCMOS_PWR_DISABLE);
            //spm_control_mtcmos(SPM_MTCMOS_PERISYS, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_VOW, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_COLIAS_TOP, SPM_MTCMOS_PWR_DISABLE);
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,ALL,1") != NULL) {
            strncpy((char *)response.response_buf, "DSP/AUDIO/CONN TOP ON/CONN TOP OFF/VOW MTCMOS On\r\n", strlen("DSP/AUDIO/CONN TOP ON/CONN TOP OFF/VOW MTCMOS On\r\n") + 1);
            spm_control_mtcmos(SPM_MTCMOS_CONN_TOP_ON, SPM_MTCMOS_PWR_ENABLE);
            spm_control_mtcmos(SPM_MTCMOS_CONN_TOP_OFF, SPM_MTCMOS_PWR_ENABLE);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO_SYS, SPM_MTCMOS_PWR_ENABLE);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO_ANC, SPM_MTCMOS_PWR_ENABLE);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO_HS, SPM_MTCMOS_PWR_ENABLE);
            spm_control_mtcmos(SPM_MTCMOS_DSP, SPM_MTCMOS_PWR_ENABLE);
            spm_control_mtcmos(SPM_MTCMOS_PERISYS, SPM_MTCMOS_PWR_ENABLE);
            spm_control_mtcmos(SPM_MTCMOS_VOW, SPM_MTCMOS_PWR_ENABLE);
            spm_control_mtcmos(SPM_MTCMOS_COLIAS_TOP, SPM_MTCMOS_PWR_ENABLE);
        }
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(&response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=RTC") != NULL) {
#ifdef HAL_RTC_MODULE_ENABLED
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=RTC,0") != NULL) {
            hal_rtc_enter_rtc_mode();
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=RTC,1") != NULL) {
            //enter_rtc_mode();
            /*
            RTC_32K_XTAL : MTK_HAL_EXT_32K_ENABLE = y
            RTC_32KLess  : MTK_HAL_EXT_32K_ENABLE = n

#ifdef MTK_HAL_EXT_32K_ENABLE
            LOGMSGIDI("LPHQA_enter_rtc_mode : RTC_32K_XTAL\r\n" ,0);
#else
            LOGMSGIDI("LPHQA_enter_rtc_mode : RTC_32KLess\r\n" ,0);
#endif*/
            //hal_gpt_delay_ms(200);
            rtc_alarm_irq_test();
            }
            response.response_len = strlen((char *) response.response_buf);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&response);
#else
        strncpy((char *)response.response_buf, "Not Support\n", strlen("Not Support\n") + 1);
        response.response_len = strlen((char *) response.response_buf);
        response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
        atci_send_response(&response);
#endif
            } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=STATUS") != NULL) {
            hal_gpt_delay_ms(200);
            if (hal_sleep_manager_is_sleep_locked()) {
            sleep_management_get_lock_sleep_handle_list();
            } else {
            LOGMSGIDI("\r\nAny Sleep Handle not locked\r\n", 0);
            }
            response.response_len = strlen((char *) response.response_buf);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&response);
            } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=DEBUG") != NULL) {
            char *data = NULL;
            data = strchr(parse_cmd->string_ptr, ',');
            data++;
#ifdef  SLEEP_MANAGEMENT_DEBUG_ENABLE
            int mode;
            mode = atoi(data);
            if (mode == 0) {
            sleep_management_debug_dump_lock_sleep_time();
            } else if (mode == 1) {
            sleep_management_debug_dump_backup_restore_time();
            } else {
            LOGMSGIDI("unknown command\r\n", 0);
            }
#else
            LOGMSGIDI("need enable 'SLEEP_MANAGEMENT_DEBUG_ENABLE' feature option\r\n", 0);
#endif
            response.response_len = strlen((char *) response.response_buf);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            atci_send_response(&response);
            } else {
            strncpy((char *)response.response_buf, "Not Support\n", strlen("Not Support\n") + 1);
            response.response_len = strlen((char *) response.response_buf);
            response.response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
            atci_send_response(&response);
            return ATCI_STATUS_ERROR;
            }
            return ATCI_STATUS_OK;
            }
#endif
#elif defined (AIR_BTA_IC_STEREO_HIGH_G3)

#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "at_command.h"
#include "syslog.h"
#ifdef HAL_AUDIO_MODULE_ENABLED
#include "bt_sink_srv_ami.h"
#endif
#ifdef HAL_SLEEP_MANAGER_ENABLED

#include "hal_sleep_manager.h"
#include "hal_sleep_manager_internal.h"
#include "hal_spm.h"
#include "hal_pmu.h"
#include "nvdm.h"
#include "hal_rtc.h"
#include "hal_clock_internal.h"
#include <stdlib.h>
#ifdef MTK_SWLA_ENABLE
#include "swla.h"
#endif

log_create_module(atci_slp, PRINT_LEVEL_INFO);
#define LOGI(fmt,arg...)   LOG_I(atci_slp ,"ATCMD: "fmt,##arg)
#define LOGMSGIDI(fmt,cnt,arg...)   LOG_MSGID_I(atci_slp ,"ATCMD: "fmt,cnt,##arg)

extern uint8_t sleep_manager_handle;
extern void bsp_ept_gpio_setting_init(void);
#ifdef HAL_PMU_MODULE_ENABLED
extern void pmu_sw_enter_sleep(pmu_power_domain_t domain);
#endif
extern uint32_t clock_get_pll_state(clock_id pll_id);
extern void rtc_32k_off(void);

#ifdef HAL_SLEEP_MANAGER_ENABLED
extern atci_status_t atci_cmd_hdlr_sleep_manager(atci_parse_cmd_param_t *parse_cmd);
#endif

int sm_htoi(char s[])
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

#ifdef HAL_RTC_MODULE_ENABLED
static void rtc_alarm_setting()
{
    hal_rtc_time_t time, rtc_time;

    hal_rtc_get_time(&time);
    rtc_time.rtc_year = time.rtc_year;
    rtc_time.rtc_mon = time.rtc_mon;
    rtc_time.rtc_day = time.rtc_day;
    rtc_time.rtc_hour = time.rtc_hour;
    rtc_time.rtc_min = time.rtc_min;
    rtc_time.rtc_sec = time.rtc_sec;
    rtc_time.rtc_week = time.rtc_week;
    rtc_time.rtc_milli_sec = time.rtc_milli_sec;
    if (time.rtc_sec + 25 > 59) {
        rtc_time.rtc_sec = time.rtc_sec + 25 - 60;
        rtc_time.rtc_min = time.rtc_min + 1;
    } else {
        rtc_time.rtc_sec = time.rtc_sec + 25;
    }
    hal_rtc_set_alarm(&rtc_time);
    LOGMSGIDI("alarm : %d/%d/%d %d:%d:%d(%d)\r\n", 7, rtc_time.rtc_year, rtc_time.rtc_mon, rtc_time.rtc_day,
              rtc_time.rtc_hour, rtc_time.rtc_min, rtc_time.rtc_sec, rtc_time.rtc_milli_sec);
}

void enter_rtc_off_mode()
{
    LOGMSGIDI("Enter RTC Off Mode\r\n", 0);
    hal_gpt_delay_ms(100);
    rtc_32k_off();
    hal_gpt_delay_ms(2000);
    hal_rtc_enter_rtc_mode();
}

extern bool back_from_rtc_mode;
void enter_rtc_mode()
{
#ifdef MTK_HAL_EXT_32K_ENABLE
    LOGMSGIDI("enter_rtc_mode : RTC_32K_XTAL\r\n", 0);
#else
    LOGMSGIDI("enter_rtc_mode : RTC_32KLess\r\n", 0);
#endif

    hal_gpt_delay_ms(200);
    if (back_from_rtc_mode == false) {
        rtc_alarm_setting();
        hal_rtc_enable_alarm();
        hal_rtc_enter_rtc_mode();
    } else {
        LOGMSGIDI("rtc mode wakeup by alarm test pass\r\n", 0);
        rtc_alarm_setting();
        hal_rtc_disable_alarm();
        hal_rtc_enter_rtc_mode();
    }
}

extern bool    hal_rtc_is_back_from_rtcmode();
void rtc_alarm_irq_test(void)
{
    if (hal_rtc_is_back_from_rtcmode() == false) {
        rtc_alarm_setting();
        hal_rtc_enable_alarm();
        hal_rtc_enter_rtc_mode();
    } else {
        LOGMSGIDI("rtc mode wakeup by alarm test pass\r\n", 0);
        rtc_alarm_setting();
        hal_rtc_disable_alarm();
        hal_rtc_enter_rtc_mode();
    }
}
#endif
#ifdef HAL_AUDIO_SUPPORT_MULTIPLE_STREAM_OUT
extern HAL_DSP_PARA_AU_AFE_CTRL_t audio_nvdm_HW_config;
#endif
atci_status_t atci_cmd_hdlr_sleep_manager(atci_parse_cmd_param_t *parse_cmd)
{
    atci_response_t *response = atci_mem_alloc(sizeof(atci_response_t));
    if (NULL == response) {
        return ATCI_STATUS_ERROR;
    }
    response->response_flag = 0;

    if (strstr((char *)parse_cmd->string_ptr, "AT+SM=0") != NULL) {
        snprintf((char *)response->response_buf, sizeof(response->response_buf), "Low Power Test Command Pass!!\r\n");
        response->response_len = strlen((char *) response->response_buf);
        response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(response);
    }  else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=BASE") != NULL) {
        __asm volatile("cpsid i");
        uint32_t mask;

        int i;
        for (i = 0; i < 75; i++) {

            if (hal_nvic_get_pending_irq(i) == 1) {
                hal_nvic_disable_irq(i);
                log_hal_msgid_info("pending_irq:%d\r\n", 1, i);
                hal_nvic_clear_pending_irq(i);
                hal_nvic_disable_irq(i);
            }
        }
        for (i = 0; i < 75; i++) {

            if (hal_nvic_get_pending_irq(i) == 1) {
                log_hal_msgid_info("pending_irq:%d\r\n", 1, i);
                hal_nvic_clear_pending_irq(i);
                hal_nvic_disable_irq(i);
            }
        }
        hal_nvic_disable_irq(CRYPTO_IRQn);
        hal_nvic_disable_irq(SPM_IRQn);

        *SPM_IGNORE_CPU_ACTIVE |= 0x100;    //IGNORE DSP ACTIVE
        *SPM_CM4_WAKEUP_SOURCE_MASK = 0xFFFFFFFF;
        *SPM_DSP_WAKEUP_SOURCE_MASK = 0xFFFFFFFF;

#if defined(PMIC_STEREO_HIGH_G3_LP_TYPE_A)
        pmu_set_audio_mode(PMU_CLASSD, PMU_OFF);
        /*pmu_enable_micbias(PMIC_MICBIAS_LDO0, PMIC_MICBIAS0, PMIC_MICBIAS_NM, PMU_OFF);
        pmu_enable_micbias(PMIC_MICBIAS_LDO1, PMIC_MICBIAS1, PMIC_MICBIAS_NM, PMU_OFF);
        pmu_enable_micbias(PMIC_MICBIAS_LDO2, PMIC_MICBIAS2, PMIC_MICBIAS_NM, PMU_OFF);*/
#endif

        while (1) {
            hal_nvic_clear_pending_irq(GPT_IRQn);
            *SPM_CONN_AUDIO_ABB_XO_SIDEBAND = 0;        //for sleep test, need BT or Audio Control.
            hal_nvic_save_and_set_interrupt_mask(&mask);
            __asm volatile("cpsid i");
#ifdef MTK_LOCK_S1
            *SPM_SPM_STATE_CONTROL_0 = 0;
#endif
            hal_sleep_manager_enter_sleep_mode(HAL_SLEEP_MODE_SLEEP);
            __asm volatile("cpsie i");
            hal_nvic_restore_interrupt_mask(mask);
            *SPM_CONN_AUDIO_ABB_XO_SIDEBAND = 0x101;    //for sleep test, need BT or Audio Control.
        }

        response->response_len = strlen((char *) response->response_buf);
        atci_send_response(response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=DEBUGIO") != NULL) {

        char *end_pos = NULL;
        int input_value = 0;
        end_pos = strchr(parse_cmd->string_ptr, ',');
        end_pos ++;

        input_value = sm_htoi(end_pos);
        end_pos = NULL;

        LOGMSGIDI("set DEBUGIO:0x%x\r\n", 1, input_value);

        snprintf((char *)response->response_buf, sizeof(response->response_buf), "Set_SPM_debug_bus\r\n");
        spm_debug_io(input_value);

        response->response_len = strlen((char *) response->response_buf);
        response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=HANDLE_LOCK") != NULL) {
        int handle_index;
        char *data = NULL;
        data = strchr(parse_cmd->string_ptr, ',');
        data++;
        handle_index = atoi(data);
        LOGMSGIDI("force lock sleep handle : %d\r\n", 1, (int)handle_index);
        hal_sleep_manager_lock_sleep(handle_index);
        snprintf((char *)response->response_buf, sizeof(response->response_buf), "force lock sleep handle : %d\r\n", handle_index);
        response->response_len = strlen((char *) response->response_buf);
        response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=HANDLE_UNLOCK") != NULL) {
        int handle_index;
        char *data = NULL;
        data = strchr(parse_cmd->string_ptr, ',');
        data++;
        handle_index = atoi(data);
        LOGMSGIDI("force unlock sleep handle : %d\r\n", 1, (int)handle_index);
        hal_sleep_manager_unlock_sleep(handle_index);
        snprintf((char *)response->response_buf, sizeof(response->response_buf), "force unlock sleep handle : %d\r\n", handle_index);
        response->response_len = strlen((char *) response->response_buf);
        response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=LOCK") != NULL) {
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=LOCK,0,SLP") != NULL) {
            hal_sleep_manager_unlock_sleep(sleep_manager_handle);
            response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            snprintf((char *)response->response_buf, sizeof(response->response_buf), "unlock sleep manager handle\r\n");
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=LOCK,1,SLP") != NULL) {
            hal_sleep_manager_lock_sleep(sleep_manager_handle);
            response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
            snprintf((char *)response->response_buf, sizeof(response->response_buf), "lock sleep manager handle\r\n");
        }
        response->response_len = strlen((char *) response->response_buf);
        atci_send_response(response);
    } else if (strstr((char *)parse_cmd->string_ptr, "AT+SM=SET_GPIO") != NULL) {
        LOGMSGIDI("AT+SM=SET_GPIO", 0);
        hal_gpio_pin_t pin;
        uint8_t function_index;
        char *config_s = NULL;
        int ret1, ret2;
        config_s = strchr((char *)parse_cmd->string_ptr, ',');
        config_s++;

        ret1 = sscanf(config_s, "%d", (int *)&pin);
        config_s = strchr(config_s, ',');
        config_s++;
        ret2 = sscanf(config_s, "%d", (int *)&function_index);
        hal_gpio_init(pin);

        if ((ret1 != 1) || (ret2 != 1)) {
            LOGMSGIDI("AT+SM=SET_GPIO set fail\r\n", 0);
            snprintf((char *)response->response_buf, sizeof(response->response_buf), "AT+SM=SET_GPIO set fail\r\n");
        } else {
            snprintf((char *)response->response_buf, sizeof(response->response_buf), "AT+SM=SET_GPIO set success\r\n");
        }

        spm_pinmux_set_function(pin, function_index);
        response->response_len = strlen((char *) response->response_buf);
        response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(response);
    } else if (strstr((char *)parse_cmd->string_ptr, "AT+SM=DSP_PERFMON") != NULL) {
        /* For DSP performance counter */
        char *end_pos = NULL, *mid_pos = NULL, *start_pos = NULL;
        int input_1 = 0, input_2 = 0, input_3 = 0;

        #include "hal_resource_assignment.h"

        start_pos = strchr((char *)parse_cmd->string_ptr, ',');
        start_pos++;
        mid_pos = strchr(start_pos, ',');
        mid_pos++;
        end_pos = strchr(mid_pos, ',');
        end_pos++;

        sscanf(start_pos, "%d", (int *)&input_1);
        input_1 = (int)strtoul(start_pos, NULL, 0);
        input_2 = (int)strtoul(mid_pos, NULL, 0);
        input_3 = (int)strtoul(end_pos, NULL, 0);

        if (input_1 == 2) {
            *((volatile uint32_t *)HW_SYSRAM_PRIVATE_MEMORY_DSP_PERFMON2_SELECT_START) = input_2;
            *((volatile uint32_t *)HW_SYSRAM_PRIVATE_MEMORY_DSP_PERFMON2_MASK_START) = input_3;
            snprintf((char *)response->response_buf, sizeof(response->response_buf), "+DSP_PERFMON2:%d,0x%x,0x%x\r\n",
                     input_1,
                     *((volatile unsigned int *)HW_SYSRAM_PRIVATE_MEMORY_DSP_PERFMON2_SELECT_START),
                     *((volatile unsigned int *)HW_SYSRAM_PRIVATE_MEMORY_DSP_PERFMON2_MASK_START));
        } else {
            *((volatile uint32_t *)HW_SYSRAM_PRIVATE_MEMORY_DSP_PERFMON1_SELECT_START) = input_2;
            *((volatile uint32_t *)HW_SYSRAM_PRIVATE_MEMORY_DSP_PERFMON1_MASK_START) = input_3;
            snprintf((char *)response->response_buf, sizeof(response->response_buf), "+DSP_PERFMON1:%d,0x%x,0x%x\r\n",
                     input_1,
                     *((volatile unsigned int *)HW_SYSRAM_PRIVATE_MEMORY_DSP_PERFMON1_SELECT_START),
                     *((volatile unsigned int *)HW_SYSRAM_PRIVATE_MEMORY_DSP_PERFMON1_MASK_START));
        }

        response->response_len = strlen((char *) response->response_buf);
        response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR") != NULL) {
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,DSP,0") != NULL) {
            *SPM_IGNORE_CPU_ACTIVE |= 0x100;    //IGNORE DSP ACTIVE
            snprintf((char *)response->response_buf, sizeof(response->response_buf), "DSP Power Off\r\n");
            spm_control_mtcmos(SPM_MTCMOS_DSP, SPM_MTCMOS_PWR_DISABLE);
        } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,DSP,1") != NULL) {
            snprintf((char *)response->response_buf, sizeof(response->response_buf), "DSP Power On\r\n");
            spm_control_mtcmos(SPM_MTCMOS_DSP, SPM_MTCMOS_PWR_ENABLE);
            *SPM_IGNORE_CPU_ACTIVE &= (~(1 << 8));    //IGNORE DSP ACTIVE
        } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,AUDIO,0") != NULL) {
            snprintf((char *)response->response_buf, sizeof(response->response_buf), "AUDIO Power Off\r\n");
            spm_control_mtcmos(SPM_MTCMOS_AUDIO, SPM_MTCMOS_PWR_DISABLE);
        } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,AUDIO,1") != NULL) {
            snprintf((char *)response->response_buf, sizeof(response->response_buf), "AUDIO Power On\r\n");
            spm_control_mtcmos(SPM_MTCMOS_AUDIO, SPM_MTCMOS_PWR_ENABLE);
        } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,ALL,0") != NULL) {
            snprintf((char *)response->response_buf, sizeof(response->response_buf), "AUDIO/DSP MTCMOS Off\r\n");
            spm_control_mtcmos(SPM_MTCMOS_AUDIO, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_DSP, SPM_MTCMOS_PWR_DISABLE);
        } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=PWR,ALL,1") != NULL) {
            snprintf((char *)response->response_buf, sizeof(response->response_buf), "AUDIO/DSP MTCMOS On\r\n");
            spm_control_mtcmos(SPM_MTCMOS_AUDIO, SPM_MTCMOS_PWR_ENABLE);
            spm_control_mtcmos(SPM_MTCMOS_DSP, SPM_MTCMOS_PWR_ENABLE);
        }
        response->response_len = strlen((char *) response->response_buf);
        response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=RTC") != NULL) {
#ifdef HAL_RTC_MODULE_ENABLED
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=RTC,0") != NULL) {
            hal_rtc_enter_rtc_mode();
        }
        if (strstr((char *) parse_cmd->string_ptr, "AT+SM=RTC,1") != NULL) {
            //enter_rtc_mode();
            /*
            RTC_32K_XTAL : MTK_HAL_EXT_32K_ENABLE = y
            RTC_32KLess  : MTK_HAL_EXT_32K_ENABLE = n

#ifdef MTK_HAL_EXT_32K_ENABLE
            LOGMSGIDI("LPHQA_enter_rtc_mode : RTC_32K_XTAL\r\n" ,0);
#else
            LOGMSGIDI("LPHQA_enter_rtc_mode : RTC_32KLess\r\n" ,0);
#endif*/
            //hal_gpt_delay_ms(200);
            rtc_alarm_irq_test();
        }
        response->response_len = strlen((char *) response->response_buf);
        response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(response);
#else
        snprintf((char *)response->response_buf, sizeof(response->response_buf), "Not Support\n");
        response->response_len = strlen((char *) response->response_buf);
        response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
        atci_send_response(response);
#endif
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=STATUS") != NULL) {
        hal_gpt_delay_ms(200);
        if (hal_sleep_manager_is_sleep_locked()) {
            sleep_management_get_lock_sleep_handle_list();
            snprintf((char *)response->response_buf, sizeof(response->response_buf), "Any Sleep Handle locked\r\n");
        } else {
            // LOGMSGIDI("\r\nAny Sleep Handle not locked\r\n", 0);
            snprintf((char *)response->response_buf, sizeof(response->response_buf), "Any Sleep Handle not locked\r\n");
        }
        response->response_len = strlen((char *) response->response_buf);
        response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(response);
    } else if (strstr((char *) parse_cmd->string_ptr, "AT+SM=DEBUG") != NULL) {
        char *data = NULL;
        data = strchr(parse_cmd->string_ptr, ',');
        data++;
#ifdef  SLEEP_MANAGEMENT_DEBUG_ENABLE
        int mode;
        mode = atoi(data);
        if (mode == 0) {
            sleep_management_debug_dump_lock_sleep_time();
        } else if (mode == 1) {
            sleep_management_debug_dump_backup_restore_time();
        } else {
            LOGMSGIDI("unknown command\r\n", 0);
        }
#else
        LOGMSGIDI("need enable 'SLEEP_MANAGEMENT_DEBUG_ENABLE' feature option\r\n", 0);
#endif
        response->response_len = strlen((char *) response->response_buf);
        response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_OK;
        atci_send_response(response);
    } else {
        snprintf((char *)response->response_buf, sizeof(response->response_buf), "Not Support\n");
        response->response_len = strlen((char *) response->response_buf);
        response->response_flag |= ATCI_RESPONSE_FLAG_APPEND_ERROR;
        atci_send_response(response);
        atci_mem_free(response);
        return ATCI_STATUS_ERROR;
    }
    atci_mem_free(response);
    return ATCI_STATUS_OK;
}
#endif
#else
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "at_command.h"
#include "syslog.h"
#ifdef HAL_AUDIO_MODULE_ENABLED
#include "bt_sink_srv_ami.h"
#endif
#ifdef HAL_SLEEP_MANAGER_ENABLED

#include "hal_sleep_manager.h"
#include "hal_sleep_manager_internal.h"
#include "hal_spm.h"
#include "hal_pmu.h"
#include "nvdm.h"
#include "hal_rtc.h"
#include "hal_clock_internal.h"
#include <stdlib.h>
#ifdef MTK_SWLA_ENABLE
#include "swla.h"
#endif

atci_status_t atci_cmd_hdlr_sleep_manager(atci_parse_cmd_param_t *parse_cmd)
{

    /* to be implemented */

    return ATCI_STATUS_OK;

}

#endif
#endif

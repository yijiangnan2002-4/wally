/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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
#include "hal_platform.h"
#ifdef HAL_RTC_MODULE_ENABLED
#include "hal_rtc_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hal.h"
#include "hal_eint.h"
#include "hal_gpt.h"
#include "hal_nvic.h"
#include "hal_clock.h"
#include "syslog.h"
#include "hal_spm.h"
#include "hal_nvic_internal.h"
#include "hal_gpio.h"
#include "hal_clock_internal.h"
/***********************************************************************************************
 * Pre Processor Definitions
***********************************************************************************************/
/*DE will ECO rtc hw?*/
//#define     RTC_WILL_ECO_HW
#define     RTC_ECO_DONE_FLG    false

/*version rg*/
#define   RTC_CHIP_VERSION      *((uint32_t *)0xA2000008)
#define   RTC_ECO_VERSION       *((uint32_t *)0xA2000000)
#define   ECO_VER_E1            0xCA00
#define   ECO_VER_E2            0xCA01
#define   ECO_VER_E3            0xCA02

/*register number*/
#define   RTC_SPARE_RG_NUM      6
/*avaiable byte per register*/
#define   RTC_SPARE_RG_BYTE     2

/*For RTC Write RG time check*/
//#define RTC_TIME_CHECK_ENABLE

#ifdef RTC_TIME_CHECK_ENABLE
#define     RTC_DEBUG_GET_1M_TICK( tick )   hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &tick)
#else
#define     RTC_DEBUG_GET_1M_TICK( tick )
#endif

#define _abs(value) ((value < 0)?(0-value):value)

/***********************************************************************************************
 * Private type Declarations
***********************************************************************************************/
static  volatile RTC_REGISTER_T             *rtc_register          = (RTC_REGISTER_T *)RTC_BASE;
static  volatile RTC_ABIST_FQMTR_REGISTER_T *rtc_fqmtr_register    = (RTC_ABIST_FQMTR_REGISTER_T *)RTC_ABIST_FQMTR_BASE;
static  const    rtc_period_type_mask_map_t  g_rtc_type_mask_map[] = {
    { .type = HAL_RTC_TIME_NOTIFICATION_NONE,             .mask = 0},
    { .type = HAL_RTC_TIME_NOTIFICATION_EVERY_SECOND,     .mask = 3},
    { .type = HAL_RTC_TIME_NOTIFICATION_EVERY_MINUTE,     .mask = 4},
    { .type = HAL_RTC_TIME_NOTIFICATION_EVERY_HOUR,       .mask = 5},
    { .type = HAL_RTC_TIME_NOTIFICATION_EVERY_DAY,        .mask = 6},
    { .type = HAL_RTC_TIME_NOTIFICATION_EVERY_MONTH,      .mask = 8},
    { .type = HAL_RTC_TIME_NOTIFICATION_EVERY_YEAR,       .mask = 9},
    { .type = HAL_RTC_TIME_NOTIFICATION_EVERY_SECOND_1_2, .mask = 2},
    { .type = HAL_RTC_TIME_NOTIFICATION_EVERY_SECOND_1_4, .mask = 1},
    { .type = HAL_RTC_TIME_NOTIFICATION_EVERY_SECOND_1_8, .mask = 0},
};
static  volatile    bool                    g_cg_enabled   = false;
static  volatile    uint32_t                g_cg_ref_cnt   = 0;
/***********************************************************************************************
 * Function Declare
***********************************************************************************************/
inline void                 rtc_internal_reload(void);
uint32_t                    rtc_internal_measure_counter(uint32_t fixed_clock, uint32_t tested_clock, uint32_t window_setting);

/***********************************************************************************************
 * Private Function
***********************************************************************************************/
static void rtc_local_wait_done(void)
{
#define     RTC_TIMEOUT_WAIT_BUSY_US    5000

    uint32_t  count = 0;
    uint32_t  pre_tick = 0, cur_tick = 0, tick_gap = 0;

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &pre_tick);
    while (1) {
        if ((RTC_WRTGR_CBUSY_MASK & rtc_register->RTC_WRTGR) == 0) {
            break;
        }
        /* for check wait busy timeout */
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &cur_tick);
        hal_gpt_get_duration_count(pre_tick, cur_tick, &tick_gap);
        /* if timeout, then print log and update pre-tick */
        if (tick_gap > RTC_TIMEOUT_WAIT_BUSY_US) {
            if (count++ == 5) {
                log_rtc_error("[hal][rtc] local_wait_done: failed ,sw will return\r\n", 0);
                break;
            } else {
                pre_tick = cur_tick;
                log_rtc_info("[hal][rtc] local_wait_done(%d): timeout=%dus, RTC_WRTGR = %x!\r\n", 3, count, tick_gap, rtc_register->RTC_WRTGR);
            }
        }
    }
}


static  void    rtc_local_trigger_wait()
{
    rtc_register->RTC_WRTGR |= RTC_WRTGR_WRTGR_MASK;
    rtc_local_wait_done();
}

static  void    rtc_local_config_wakeup_source(rtc_wakeup_source_t source, bool enable)
{
    uint32_t    key  = 0;
    uint32_t    mask = 0;
    switch (source) {
        case RTC_WAKEUP_SOURCE_ALARM:
            key = KEY_BBPU_MASK_ALARM;
            break;
        case RTC_WAKEUP_SOURCE_TICK:
            key = KEY_BBPU_MASK_TICK;
            break;
        case RTC_WAKEUP_SOURCE_EINT0:
            key = KEY_BBPU_MASK_EINT0;
            break;
        case RTC_WAKEUP_SOURCE_EINT1:
            key = KEY_BBPU_MASK_EINT1;
            break;
        case RTC_WAKEUP_SOURCE_EINT2:
            key = KEY_BBPU_MASK_EINT2;
            break;
        case RTC_WAKEUP_SOURCE_EINT3:
            key = KEY_BBPU_MASK_EINT3;
            break;
        case RTC_WAKEUP_SOURCE_CAP:
            key = KEY_BBPU_MASK_CAP;
            break;
        //case RTC_WAKEUP_SOURCE_GALARM: key = KEY_BBPU_MASK_GALARM;break;
        default:
            break;
    }
    mask = (1 << source);
    if (enable) {
        rtc_register->RTC_BBPU = ((key << 8) | mask);
    } else {
        rtc_register->RTC_BBPU = (key << 8);
    }
    rtc_local_trigger_wait();
}

static  void    rtc_local_lowpower_detect()
{
    //rtc_internal_reload();
    /*enable lpd*/
    rtc_register->RTC_LPD_CON |= RTC_LPD_CON_EOSC32_LPDEN_MASK;
    rtc_local_trigger_wait();
    /*reset lpd*/
    rtc_register->RTC_LPD_CON |= (RTC_LPD_CON_EOSC32_LPDEN_MASK | RTC_LPD_CON_LPRST_MASK);
    rtc_local_trigger_wait();
    /* designer suggests delay at least 1 ms */
    hal_gpt_delay_us(1000);
    /*disable rst*/
    rtc_register->RTC_LPD_CON &= ~RTC_LPD_CON_LPRST_MASK;
    rtc_local_trigger_wait();
    rtc_register->RTC_LPD_CON |= RTC_LPD_CON_EOSC32_LPDEN_MASK;
    rtc_local_trigger_wait();

    if ((rtc_register->RTC_LPD_CON & RTC_LPD_CON_LPSTA_RAW_MASK) != 0) {
        log_rtc_error("rtc_lpd_init fail : RTC_LPD_CON = %x!\r\n", 1, (unsigned int) rtc_register->RTC_LPD_CON);
    }
}


static  uint32_t  _rtc_osc32con_control(uint32_t reg_id, rtc_op_type_t op, uint32_t val)
{
    volatile    uint32_t *reg_addr = NULL;
    uint32_t    magic_key[] = { RTC_OSC32CON0_MAGIC_KEY, \
                                RTC_OSC32CON1_MAGIC_KEY, \
                                RTC_OSC32CON2_MAGIC_KEY,  \
                                RTC_OSC32CON3_MAGIC_KEY
                              };
    switch (reg_id) {
        case 0:
            reg_addr = &rtc_register->RTC_OSC32CON0;
            break;
        case 1:
            reg_addr = &rtc_register->RTC_OSC32CON1;
            break;
        case 2:
            reg_addr = &rtc_register->RTC_OSC32CON2;
            break;
        case 3:
            reg_addr = &rtc_register->RTC_OSC32CON3;
            break;
        default:    {
            return 0;
        };
    }
    if (op == RTC_OP_SET) {
        *reg_addr = (magic_key[reg_id] << 16) | (0xFFFF & val);
        rtc_local_trigger_wait();
    } else {
        rtc_internal_reload();
    }
    return *reg_addr;
}



/***********************************************************************************************
 * Public Function
***********************************************************************************************/
void    rtc_internal_output_32k_to_gpio()
{
    uint32_t    temp = 0;

    *((volatile uint32_t *)(0x42010008)) = 0x4;

    /*output EOSC-32k to GPIO*/
    temp |= (0x1 << RTC_DEBUG_CON0_DBG_EN_OFFSET);
    temp |= (0x1 << RTC_DEBUG_CON0_GPIO_SEL_OFFSET); //RTC GPIO1 output mon_bit_2
    temp |= (0xd << RTC_DEBUG_CON0_DBG_SEL_OFFSET); //select debug signal
    rtc_register->RTC_DEBUG_CON_0 = temp;

    hal_gpio_init(HAL_GPIO_2);
    hal_pinmux_set_function(HAL_GPIO_2, 10);
}

void    hal_rtc_output_32k_to_gpio(uint8_t pin, uint8_t pin_func, uint32_t ctrl)
{
    uint32_t    temp = 0;

    *((volatile uint32_t *)(0x42010008)) = 0x4;

    temp = (0x1 << 8) | (ctrl & 0x7F);
    rtc_register->RTC_DEBUG_CON_0 = temp;

    hal_gpio_init(pin);
    hal_gpio_disable_pull(pin);
    hal_pinmux_set_function(pin, pin_func);
}

void    rtc_internal_reload()
{
    rtc_register->RTC_BBPU = KEY_BBPU_MASK_VAL_RELOAD << RTC_BBPU_KEY_BBPU_OFFSET;
    rtc_local_trigger_wait();
}

int32_t     rtc_internal_ioctrl(uint32_t cmd, uint32_t option, uint32_t args)
{
    int32_t reslt = 0;

    switch (cmd) {
        case RTC_IOCTL_CLEAR_WAKEUP_STATUS: {
            rtc_register->RTC_BBPU = (KEY_BBPU_MASK_CLR_WAKEUP << 8) | 0x2;
            rtc_local_trigger_wait();
        }
        break;
        case RTC_IOCTL_GET_IRQ_STATUS: {
            reslt = rtc_register->RTC_IRQ_STA;
        }
        break;
        case RTC_IOCTL_SET_32K_OFF: {
            rtc_register->RTC_RESET_CON = 0x0;
        }
        break;
        case RTC_IOCTL_GET_PWRKEY_STATUS: {
            uint32_t mask = (RTC_WRTGR_PWR_STA_MASK | RTC_WRTGR_CLK_STA_MASK);
            reslt = (rtc_register->RTC_WRTGR & mask) == mask ? true : false;
        }
        break;
        case RTC_IOCTL_GET_PWRBYRTC_ST: {
            reslt = ((rtc_register->RTC_BBPU & RTC_BBPU_RTC_WAKEUP_MASK) != 0) ? true : false;
        }
        break;
        case RTC_IOCTL_CLEAR_PWR_STATE: {
            rtc_register->RTC_RESET_CON = 0x301;
            rtc_local_trigger_wait();
        }
        break;
        case RTC_IOCTL_SET_ONETIME_CALI: {
            rtc_register->RTC_DIFF = option;
            rtc_local_trigger_wait();
        }
        break;
        case RTC_IOCTL_GET_ONETIME_CALI: {
            reslt = (rtc_register->RTC_CALI & RTC_CALI_VALUE_MASK);
        }
        break;
        case RTC_IOCTL_SET_SPAR_REG: {
            rtc_register->RTC_SPAR_REG = option;
            rtc_local_trigger_wait();
        }
        break;
        case RTC_IOCTL_GET_SPAR_REG: {
            reslt = rtc_register->RTC_SPAR_REG;
        }
        break;
        case RTC_IOCTL_SET_TIME_CALI: {
            if (option == RTC_TIME_CALI_K_EOSC) {
                rtc_register->RTC_CALI = (args & RTC_CALI_VALUE_MASK) | (1 << RTC_CALI_RW_SEL_OFFSET);
            } else {
                rtc_register->RTC_CALI = (args & RTC_CALI_VALUE_MASK);
            }
            rtc_local_trigger_wait();
        }
        break;
        case RTC_IOCTL_GET_TIME_CALI: {
            reslt = rtc_register->RTC_CALI;
        }
        break;
        case RTC_IOCTL_UNLOCK_PROTECT: {
            rtc_register->RTC_PROT = RTC_PROTECT1;
            rtc_local_trigger_wait();

            rtc_register->RTC_PROT = RTC_PROTECT2;
            rtc_local_trigger_wait();
        }
        break;
        case RTC_IOCTL_GET_PWRON_REASON: {
#ifdef RTC_CAPTOUCH_SUPPORTED
            if (option == 0) {
                reslt = RTC_POWERED_BY_CAPTOUCH;
                log_rtc_warn("rtc power by captouch!\r\n", 0);
            }
#endif
            if ((option & RTC_IRQ_STA_ALARM_MASK)) {
                reslt = RTC_POWERED_BY_ALARM;
            } else if ((option & RTC_IRQ_STA_TICK_MASK)) {
                reslt = RTC_POWERED_BY_TICK;
            } else if ((option & RTC_IRQ_STA_EINT0_MASK)) {
                reslt = RTC_POWERED_BY_EINT0;
            } else if ((option & RTC_IRQ_STA_EINT1_MASK)) {
                reslt = RTC_POWERED_BY_EINT1;
            } else if ((option & RTC_IRQ_STA_EINT2_MASK)) {
                reslt = RTC_POWERED_BY_EINT2;
            } else if ((option & RTC_IRQ_STA_EINT3_MASK)) {
                reslt = RTC_POWERED_BY_EINT3;
            }
            log_rtc_warn("rtc power by %d (1:1ST 2:ALARM 3:TICK 4~7:EINT 9:CAP)!\r\n", 1, reslt);
        }
        break;
        case RTC_IOCTL_SET_EOSC_FREQ_CALI: {
            uint32_t value;
            value = _rtc_osc32con_control(1, RTC_OP_GET, 0);
            value &= ~RTC_OSC32CON1_XOSC_CALI_MASK;
            value |= ((option << RTC_OSC32CON1_XOSC_CALI_OFFSET) & RTC_OSC32CON1_XOSC_CALI_MASK);
            //rtc_local_set_osc32con(1,value);
            _rtc_osc32con_control(1, RTC_OP_SET, value);
        }
        break;
        case RTC_IOCTL_CALC_TIME_CALI: {
            uint32_t    actual_freq = 0;
            int         cali        = 0;

            actual_freq = hal_rtc_measure_32k_with_windows(option, RTC_EOSC_FREQ_CALI_WINDOW);
            /* Set RTC_CALI */
            cali = (32768 - actual_freq);
            log_rtc_warn("[hal][rtc] time cali value %d(0x%x) = (32768 - %d)", 3, cali, cali, (unsigned int)actual_freq);

            if (cali > 4095) {
                cali = 4095;
                log_rtc_error("[hal][rtc] cali is too big\r\n", 0);
            } else if (cali < (-4096)) {
                cali = -4096;
                log_rtc_error("[hal][rtc] cali is too small\r\n", 0);
            }
            reslt = cali;
        }
        break;
        case RTC_IOCTL_GET_32K_SEL: {
            uint32_t temp, sel_32k;

            rtc_register->RTC_DEBUG_CON_0 = 0x107;
            temp = rtc_register->RTC_DEBUG_CON_1;
            sel_32k = (temp >> 8) & 0x3;
            switch (sel_32k) {
                case 0x0:
                    reslt = HAL_RTC_OSC32K_DCXO_MODE;
                    break;
                case 0x1:
                    reslt = HAL_RTC_OSC32K_EOSC_MODE;
                    break;
                default: {
                    reslt = -1;
                    log_rtc_error("[hal][rtc] invalid 32k sel:%d\r\n", 1, sel_32k);
                }
                break;
            }
        }
        break;
        default: {
        } break;
    }
    return reslt;
}


void    rtc_internal_enable_setting(bool force_ctrl, bool enable)
{
    static bool flg_setting_cg = false;

    if (flg_setting_cg == true) {
        return;
    }
    rtc_register->RTC_OSC32CON2 = (RTC_OSC32CON2_MAGIC_KEY << 16) | 0x7;
    rtc_local_wait_done();
    flg_setting_cg = true;
}

bool    rtc_internal_enable_tick_notify(hal_rtc_time_notification_period_t type, bool enable)
{
    uint32_t     i = 0, temp = 0;
    uint32_t     mask = 0;

    if (enable == false || type == HAL_RTC_TIME_NOTIFICATION_NONE) {
        mask   = 0;
        enable = false;
    } else {
        temp = sizeof(g_rtc_type_mask_map) / sizeof(rtc_period_type_mask_map_t);
        for (i = 0; i < temp; i++) {
            if (g_rtc_type_mask_map[i].type == type) {
                mask = g_rtc_type_mask_map[i].mask;
                break;
            }
        }
        mask = (mask | (1 << RTC_CII_EN_TC_EN_OFFSET));
    }
    rtc_register->RTC_CII_EN = mask;
    rtc_local_trigger_wait();
    rtc_local_config_wakeup_source(RTC_WAKEUP_SOURCE_TICK, enable);
    return true;
}

void    rtc_internal_alarm_control(rtc_op_type_t op, hal_rtc_time_t *gtime, const hal_rtc_time_t *stime, bool *enable, uint32_t *mask)
{
    uint16_t value_al3, value_al2, value_al1, value_al0;
    uint32_t lmask = 0;

    if (op == RTC_OP_GET) {
        hal_rtc_time_t  *time;
        time = gtime;
        value_al3 = rtc_register->RTC_AL3;
        value_al2 = rtc_register->RTC_AL2;
        value_al1 = rtc_register->RTC_AL1;
        value_al0 = rtc_register->RTC_AL0;
        if (time != NULL) {
            time->rtc_year = (value_al3 & RTC_AL3_AL_YEAR_MASK) >> RTC_AL3_AL_YEAR_OFFSET;
            time->rtc_milli_sec = 0; // have no meaning, just init
            time->rtc_week = (value_al2 & RTC_AL2_AL_DOW_MASK) >> RTC_AL2_AL_DOW_OFFSET;
            time->rtc_mon  = (value_al2 & RTC_AL2_AL_MONTH_MASK) >> RTC_AL2_AL_MONTH_OFFSET;
            time->rtc_hour = (value_al1 & RTC_AL1_AL_HOUR_MASK) >> RTC_AL1_AL_HOUR_OFFSET;
            time->rtc_day  = (value_al1 & RTC_AL1_AL_DOM_MASK) >> RTC_AL1_AL_DOM_OFFSET;
            time->rtc_min  = (value_al0 & RTC_AL0_AL_MINUTE_MASK) >> RTC_AL0_AL_MINUTE_OFFSET;
            time->rtc_sec  = (value_al0 & RTC_AL0_AL_SECOND_MASK) >> RTC_AL0_AL_SECOND_OFFSET;
        }
        if (enable != NULL) {
            *enable = (rtc_register->RTC_AL_MASK & ((1 << RTC_AL_MASK_AL_EN_OFFSET))) ? true : false;
        }
        if (mask != NULL) {
            *mask = rtc_register->RTC_AL_MASK & RTC_AL_MASK_AL_MASK_MASK;
        }
    } else {
        const hal_rtc_time_t    *time;

        time = stime;
        if (time != NULL) {
            rtc_register->RTC_AL0 = (time->rtc_sec << RTC_AL0_AL_SECOND_OFFSET) | (time->rtc_min << RTC_AL0_AL_MINUTE_OFFSET);
            rtc_register->RTC_AL1 = (time->rtc_hour << RTC_AL1_AL_HOUR_OFFSET) | (time->rtc_day << RTC_AL1_AL_DOM_OFFSET);
            rtc_register->RTC_AL2 = (time->rtc_mon << RTC_AL2_AL_MONTH_OFFSET) | (time->rtc_week << RTC_AL2_AL_DOW_OFFSET);
            rtc_register->RTC_AL3 = (time->rtc_year << RTC_AL3_AL_YEAR_OFFSET);
        }
        /*enable alarm irq, & unmask tick condition*/
        if (enable != NULL) {
            if (mask != NULL) {
                lmask = *mask;
            }
            if (*enable) {
                rtc_register->RTC_AL_MASK = (1 << RTC_AL_MASK_AL_EN_OFFSET) | lmask;
            } else {
                rtc_register->RTC_AL_MASK &= ~(1 << RTC_AL_MASK_AL_EN_OFFSET);
            }
            rtc_local_config_wakeup_source(RTC_WAKEUP_SOURCE_ALARM, *enable);
        }
        rtc_local_trigger_wait();
    }
}

void    rtc_internal_datetime_control(rtc_op_type_t op, hal_rtc_time_t *gtime, const hal_rtc_time_t *stime)
{
    if (op == RTC_OP_GET) {
        uint16_t    value_tc2  = 0;
        uint16_t    value_tc1  = 0;
        uint16_t    value_tc0  = 0;
        uint16_t    int_cnt    = 0;
        uint16_t    int_cnt_pre = 0;
        hal_rtc_time_t  *time;

        time = gtime;
        /* re-read time information if internal millisecond counter has carried */
        do {
            int_cnt_pre     = rtc_register->RTC_INT_CNT;
            time->rtc_year  = rtc_register->RTC_TC3;
            value_tc2       = rtc_register->RTC_TC2;
            value_tc1       = rtc_register->RTC_TC1;
            value_tc0       = rtc_register->RTC_TC0;
            int_cnt         = rtc_register->RTC_INT_CNT;
        } while (int_cnt < int_cnt_pre);
        time->rtc_week  = ((value_tc2 & RTC_TC2_DOW_MASK) - 1) >> RTC_TC2_DOW_OFFSET;
        time->rtc_mon   = (value_tc2  & RTC_TC2_MONTH_MASK) >> RTC_TC2_MONTH_OFFSET;
        time->rtc_hour  = (value_tc1  & RTC_TC1_HOUR_MASK) >> RTC_TC1_HOUR_OFFSET;
        time->rtc_day   = (value_tc1  & RTC_TC1_DOM_MASK) >> RTC_TC1_DOM_OFFSET;
        time->rtc_min   = (value_tc0  & RTC_TC0_MINUTE_MASK) >> RTC_TC0_MINUTE_OFFSET;
        time->rtc_sec   = (value_tc0  & RTC_TC0_SECOND_MASK) >> RTC_TC0_SECOND_OFFSET;
        time->rtc_milli_sec = int_cnt;
    } else {

        const hal_rtc_time_t    *time;

        time = stime;
        rtc_register->RTC_TC0 = (time->rtc_min << RTC_TC0_MINUTE_OFFSET) | (time->rtc_sec << RTC_TC0_SECOND_OFFSET);
        rtc_register->RTC_TC1 = (time->rtc_day << RTC_TC1_DOM_OFFSET)    | (time->rtc_hour << RTC_TC1_HOUR_OFFSET);
        // day-of-week range is 1~7, header file is 0~6
        rtc_register->RTC_TC2 = (time->rtc_mon << RTC_TC2_MONTH_OFFSET)  | ((time->rtc_week + 1) << RTC_TC2_DOW_OFFSET);
        rtc_register->RTC_TC3 = time->rtc_year << RTC_TC3_YEAR_OFFSET;
        rtc_register->RTC_INT_CNT = time->rtc_milli_sec;
        rtc_local_trigger_wait();
    }
}

void    rtc_internal_set_power_key()
{
    uint32_t   value;
    uint32_t   mask;

    mask = (RTC_WRTGR_PWR_STA_MASK | RTC_WRTGR_CLK_STA_MASK);
    /*disable lpd,->set pwrkey -> enable lpd */
    /* setting default OSCCON RG*/
    _rtc_osc32con_control(0, RTC_OP_SET, 0x101);
    _rtc_osc32con_control(2, RTC_OP_SET, 0x7);

    /*disable lpd*/
    rtc_register->RTC_LPD_CON = 0x100;
    rtc_local_trigger_wait();
    rtc_register->RTC_LPD_CON = 0x101;
    rtc_local_trigger_wait();

    /*set eosc step pwd and lpd*/
    //value  = rtc_local_get_osc32con(1);
    value = _rtc_osc32con_control(1, RTC_OP_GET, 0);
    value |= RTC_OSC32CON1_EOSC_STP_PWD_MASK;
    //rtc_local_set_osc32con(1,value);
    _rtc_osc32con_control(1, RTC_OP_SET, value);
    rtc_local_lowpower_detect();

    /*reset clock status*/
    rtc_register->RTC_RESET_CON = 0x101;
    rtc_local_trigger_wait();
    rtc_register->RTC_RESET_CON = 0x201;
    rtc_local_trigger_wait();

    hal_gpt_delay_us(200);
    mask = (RTC_WRTGR_PWR_STA_MASK | RTC_WRTGR_CLK_STA_MASK);
    if ((rtc_register->RTC_WRTGR & mask) != mask) {
        log_rtc_error("[hal][rtc] rtc_set_power_key fail : rtc_wrtgr = %x!\r\n", 1, (unsigned int) rtc_register->RTC_WRTGR);
        //rtc_internal_dump(NULL);
    }
}



float  hal_rtc_measure_32k_with_windows(hal_rtc_osc32k_mode_t mode, uint32_t windowset)
{
    float               frequency   = 0;
    uint32_t            fqmtr_data  = 0;
    rtc_osc32k_mode_t   d_mode = RTC_OSC32K_EOSC_MODE;
    uint32_t            save_mask;

    switch (mode) {
        case HAL_RTC_OSC32K_EOSC_MODE:
            d_mode = RTC_OSC32K_EOSC_MODE;
            break;
        case HAL_RTC_OSC32K_XOSC_MODE:
            d_mode = RTC_OSC32K_XOSC_MODE;
            break;
        case HAL_RTC_OSC32K_DCXO_MODE:
            d_mode = RTC_OSC32K_DCXO_MODE;
            break;
        case HAL_RTC_OSC32K_FCLK_MODE:
            d_mode = RTC_OSC32K_FCLK_MODE;
            break;
    }

    //disable irq
    save_mask = __get_BASEPRI();
    __set_BASEPRI(((0x01 << (8 - __NVIC_PRIO_BITS)) & 0xFF));
    __DMB();
    __ISB();

    fqmtr_data = rtc_internal_measure_counter(d_mode, RTC_FQMTR_TCKSEL_XO_CK, windowset);

    //enable irq
    __set_BASEPRI(save_mask);
    __DMB();
    __ISB();

    if (fqmtr_data != 0) {
        frequency  = (float)(((double)AIR_RTC_FCXO_CLOCK  / (double) fqmtr_data) * (windowset + 1));
    }
    return frequency;
}

uint32_t    rtc_internal_get_eosc32_freq_calibration(uint32_t target_32k_freq)
{
    uint32_t    value          = 0;
    uint8_t     eosc_cali      = 0;
    int         low_xosccali   = 0x00;
    int         high_xosccali  = 0x1f;
    uint32_t    medium_xosccali = 0;
    uint32_t    low_frequency  = 0;
    uint32_t    high_frequency = 0;
    uint32_t    medium_frequency;
    int         temp  = 0;
    int         itemp = 0;
    uint32_t    ftune_val = 0;

    _rtc_osc32con_control(3, RTC_OP_SET, 0);
    value = _rtc_osc32con_control(1, RTC_OP_GET, 0);
    value &= ~(RTC_OSC32CON1_XOSC_CALI_MASK);
    value |= ((low_xosccali << RTC_OSC32CON1_XOSC_CALI_OFFSET) | RTC_OSC32CON1_EOSC_STP_PWD_MASK);
    //rtc_local_set_osc32con(1, value);
    _rtc_osc32con_control(1, RTC_OP_SET, value);

    high_frequency = hal_rtc_measure_32k_with_windows(HAL_RTC_OSC32K_EOSC_MODE, RTC_EOSC_FREQ_CALI_WINDOW);
    if (high_frequency <= target_32k_freq) {
        log_rtc_error("[hal][rtc]high_freq(%u) <= %d, xosccali = %d \r\n", 2, (unsigned int)high_frequency, RTC_CALI_TARGET_FREQUENCY, (unsigned int)low_xosccali);
        goto error;
    }

    value &= ~RTC_OSC32CON1_XOSC_CALI_MASK;
    value |= (high_xosccali << RTC_OSC32CON1_XOSC_CALI_OFFSET);
    //rtc_local_set_osc32con(1, value);
    _rtc_osc32con_control(1, RTC_OP_SET, value);
    low_frequency = hal_rtc_measure_32k_with_windows(HAL_RTC_OSC32K_EOSC_MODE, RTC_EOSC_FREQ_CALI_WINDOW);;
    if (low_frequency >= target_32k_freq) {
        log_rtc_error("[hal][rtc] low_freq(%u) >= %d, xosccali = %d \r\n", 2, (unsigned int)low_frequency, RTC_CALI_TARGET_FREQUENCY, (unsigned int)high_xosccali);
        goto error;
    }

    while (_abs(high_xosccali - low_xosccali) > 1) {
        medium_xosccali = (low_xosccali + high_xosccali) / 2;
        value &= ~RTC_OSC32CON1_XOSC_CALI_MASK;
        value |= (medium_xosccali << RTC_OSC32CON1_XOSC_CALI_OFFSET);
        //rtc_local_set_osc32con(1,value);
        _rtc_osc32con_control(1, RTC_OP_SET, value);
        medium_frequency = hal_rtc_measure_32k_with_windows(HAL_RTC_OSC32K_EOSC_MODE, RTC_EOSC_FREQ_CALI_WINDOW);;
        log_rtc_info("[hal][rtc] eosc cali(%d) -> frequency(%d) \r\n", 2, (int) medium_xosccali, (int)medium_frequency);
        if (medium_frequency > target_32k_freq) {
            low_xosccali = medium_xosccali;
            high_frequency = medium_frequency;
        } else if (medium_frequency < target_32k_freq) {
            high_xosccali = medium_xosccali;
            low_frequency = medium_frequency;
        } else {
            log_rtc_info("[hal][rtc] eosc cali value = %d \r\n", 1, (int) medium_xosccali);
            eosc_cali = medium_xosccali;
            goto error;
        }
    }

    /*get low frequency value for later ftune */
    medium_xosccali = high_xosccali;
    value &= ~RTC_OSC32CON1_XOSC_CALI_MASK;
    value |= (medium_xosccali << RTC_OSC32CON1_XOSC_CALI_OFFSET);
    _rtc_osc32con_control(1, RTC_OP_SET, value);
    eosc_cali = (medium_xosccali);
    medium_frequency = hal_rtc_measure_32k_with_windows(HAL_RTC_OSC32K_EOSC_MODE, RTC_EOSC_FREQ_CALI_WINDOW);
    log_rtc_warn("[hal][rtc]rtc eosc cali value = %d, frequency %d \r\n", 2, (unsigned int) medium_xosccali, (unsigned int) medium_frequency);

    /* ftune for eosc frequency */
    itemp  = target_32k_freq - medium_frequency;
    ftune_val = (uint32_t)(itemp * 1.1f);
    if (ftune_val > 0xFFF) {
        ftune_val = 0xFFF;
    }
    _rtc_osc32con_control(3, RTC_OP_SET, ftune_val);
    hal_gpt_delay_ms(1);
    temp = hal_rtc_measure_32k_with_windows(HAL_RTC_OSC32K_EOSC_MODE, RTC_EOSC_FREQ_CALI_WINDOW);
    log_rtc_warn("[hal][rtc] coarse ftune %d -> %d \r\n", 2, ftune_val, temp);
    itemp = target_32k_freq - temp;
    ftune_val += (itemp * 1.1f);
    _rtc_osc32con_control(3, RTC_OP_SET, ftune_val);
    temp = hal_rtc_measure_32k_with_windows(HAL_RTC_OSC32K_EOSC_MODE, RTC_EOSC_FREQ_CALI_WINDOW);
    log_rtc_warn("[hal][rtc] fine   ftune %d -> %d \r\n", 2, ftune_val, temp);
error:
    return eosc_cali;
}




void    rtc_internal_select_32k(hal_rtc_osc32k_mode_t mode)
{
    uint32_t    temp = 0;

    switch (mode) {
        case HAL_RTC_OSC32K_DCXO_MODE: {
            /*swtich clock: dcxo-> 0x00*/
            temp = _rtc_osc32con_control(0, RTC_OP_GET, 0);
            temp &= (~RTC_OSC32CON0_32K_SEL_MASK);
            _rtc_osc32con_control(0, RTC_OP_SET, temp);
            hal_gpt_delay_us(70);
        }
        break;

        case HAL_RTC_OSC32K_EOSC_MODE: {
            /*swtich clock:eosc-> 0x01*/
            temp = _rtc_osc32con_control(0, RTC_OP_GET, 0);
            temp &= ~RTC_OSC32CON0_32K_SEL_MASK;
            temp |= 0x1;
            _rtc_osc32con_control(0, RTC_OP_SET, temp);
            hal_gpt_delay_us(70);
        }
        break;
        case HAL_RTC_OSC32K_XOSC_MODE: {
#ifdef RTC_XOSC_SUPPORTED
            /*swtich clock:eosc-> 0x03**/
            temp = _rtc_osc32con_control(0, RTC_OP_GET, 0);
            temp &= ~RTC_OSC32CON0_32K_SEL_MASK;
            temp |= 0x3;
            _rtc_osc32con_control(0, RTC_OP_SET, temp);
            hal_gpt_delay_us(70);
#endif
        }
        break;
        default:
            break;
    }
}





void    rtc_internal_set_osc32_mode(hal_rtc_osc32k_mode_t mode)
{
    uint32_t    temp = 0;

    switch (mode) {
        case HAL_RTC_OSC32K_DCXO_MODE: {
            /*setup eosc*/
            temp = _rtc_osc32con_control(1, RTC_OP_GET, 0);
            temp &= ~(RTC_OSC32CON1_EOSC_BIAS_SEL_MASK);
            temp |= (RTC_OSC32CON1_EOSC_STP_PWD_MASK | RTC_OSC32CON1_EOSC_CHOP_EN_MASK | RTC_OSC32CON1_EOSC_VCT_EN_MASK);
            temp |= (0x2 << RTC_OSC32CON1_EOSC_BIAS_SEL_OFFSET);
            _rtc_osc32con_control(1, RTC_OP_SET, temp);

            /*swtich clock: dcxo-> 0x00*/
            temp = _rtc_osc32con_control(0, RTC_OP_GET, 0);
            temp &= (~RTC_OSC32CON0_32K_SEL_MASK);
            _rtc_osc32con_control(0, RTC_OP_SET, temp);
            hal_gpt_delay_us(70);

            /*power down XOSC*/
#ifdef RTC_XOSC_SUPPORTED
            temp = _rtc_osc32con_control(2, RTC_OP_GET, 0);
            temp &= ~(RTC_OSC32CON2_EOSC_PWDB_MASK | RTC_OSC32CON2_XOSC_PWDB_MASK);
            temp |= RTC_OSC32CON2_EOSC_PWDB_MASK | RTC_OSC32CON2_SETTING_CG_MASK;
            _rtc_osc32con_control(2, RTC_OP_SET, temp);
#endif
        }
        break;

        case HAL_RTC_OSC32K_EOSC_MODE: {
            /*setup eosc*/
            temp = _rtc_osc32con_control(1, RTC_OP_GET, 0);
            temp &= ~(RTC_OSC32CON1_EOSC_BIAS_SEL_MASK);
            temp |= (RTC_OSC32CON1_EOSC_STP_PWD_MASK | RTC_OSC32CON1_EOSC_CHOP_EN_MASK | RTC_OSC32CON1_EOSC_VCT_EN_MASK);
            temp |= (0x2 << RTC_OSC32CON1_EOSC_BIAS_SEL_OFFSET);
            _rtc_osc32con_control(1, RTC_OP_SET, temp);

            /*swtich clock:eosc-> 0x01*/
            temp = _rtc_osc32con_control(0, RTC_OP_GET, 0);
            temp &= ~RTC_OSC32CON0_32K_SEL_MASK;
            temp |= 0x1;
            _rtc_osc32con_control(0, RTC_OP_SET, temp);
            hal_gpt_delay_us(70);
            /*power down XOSC*/
#ifdef RTC_XOSC_SUPPORTED
            temp = _rtc_osc32con_control(2, RTC_OP_GET, 0);
            temp &= ~(RTC_OSC32CON2_EOSC_PWDB_MASK | RTC_OSC32CON2_XOSC_PWDB_MASK);
            temp |= RTC_OSC32CON2_EOSC_PWDB_MASK | RTC_OSC32CON2_SETTING_CG_MASK;
            _rtc_osc32con_control(2, RTC_OP_SET, temp);
#endif
        }
        break;
        case HAL_RTC_OSC32K_XOSC_MODE: {
#ifdef RTC_XOSC_SUPPORTED
            /*setup xosc*/
            temp = _rtc_osc32con_control(1, RTC_OP_GET, 0);
            temp |= (RTC_OSC32CON1_EOSC_STP_PWD_MASK);
            _rtc_osc32con_control(1, RTC_OP_SET, temp);

            /*swtich clock:eosc-> 0x03**/
            temp = _rtc_osc32con_control(0, RTC_OP_GET, 0);
            temp &= ~RTC_OSC32CON0_32K_SEL_MASK;
            temp |= 0x3;
            _rtc_osc32con_control(0, RTC_OP_SET, temp);
            hal_gpt_delay_us(70);
            /*power down eosc*/
            temp = _rtc_osc32con_control(2, RTC_OP_GET, 0);
            temp &= ~(RTC_OSC32CON2_EOSC_PWDB_MASK | RTC_OSC32CON2_XOSC_PWDB_MASK);
            temp |= (RTC_OSC32CON2_XOSC_PWDB_MASK) | RTC_OSC32CON2_SETTING_CG_MASK;
            _rtc_osc32con_control(2, RTC_OP_SET, temp);
#endif
        }
        break;
        default:
            break;
    }
}


hal_rtc_status_t rtc_internal_eint_setting(hal_rtc_eint_config_t *eint_config)
{
#define     MAX_EINT_DELAY_TM   200//us

    uint32_t    eint_val = 0;
    uint32_t    offs = 0;
    uint32_t    eint_rg_val = 0;
    uint32_t    wk_src = 0;
    uint32_t    wk_en = false;
    volatile uint32_t *eint_reg = NULL;

    if (eint_config == NULL) {
        return HAL_RTC_STATUS_INVALID_PARAM;
    }

    if (eint_config->is_enable_debounce) {
        eint_val |= (RTC_EINT_DEB_EN_MASK | RTC_EINT_SYN_EN_MASK);
    }
    if (eint_config->is_falling_edge_active == 1) {
        eint_val |= RTC_EINT_INV_EN_MASK;
    } else if (eint_config->is_falling_edge_active == 2) {
        eint_val |= RTC_EINT_DUAL_EDGE_MASK;
    }
    if (eint_config->is_enable_rtc_eint) {
        eint_val |= (RTC_EINT_EINT_EN_MASK | RTC_EINT_IRQ_EN_MASK);
    }
    if (eint_config->is_enable_wakeup) {
        wk_en = true;
    }

    switch (eint_config->rtc_gpio) {
        case HAL_RTC_GPIO_0: {
            eint_reg = &rtc_register->RTC_EINT_CON0;
            offs     = 0;
            wk_src   = RTC_WAKEUP_SOURCE_EINT0;
        }
        break;
        case HAL_RTC_GPIO_1: {
            eint_reg = &rtc_register->RTC_EINT_CON0;
            offs     = 8;
            wk_src   = RTC_WAKEUP_SOURCE_EINT1;
        }
        break;
        case HAL_RTC_GPIO_2: {
            eint_reg = &rtc_register->RTC_EINT_CON1;
            offs     = 0;
            wk_src   = RTC_WAKEUP_SOURCE_EINT2;
        }
        break;
        case HAL_RTC_GPIO_3: {
            eint_reg = &rtc_register->RTC_EINT_CON1;
            offs     = 8;
            wk_src   = RTC_WAKEUP_SOURCE_EINT3;
        }
        break;
    }
    if (eint_reg == NULL) {
        return HAL_RTC_STATUS_INVALID_PARAM;
    }
    eint_rg_val = *eint_reg;

    /*disable eint*/
    eint_rg_val &= ~((RTC_EINT_EINT_EN_MASK | RTC_EINT_IRQ_EN_MASK) << offs);
    *eint_reg = eint_rg_val;
    rtc_local_trigger_wait();

    /*clear eint value*/
    eint_rg_val &= ~((RTC_EINT_EINT_CLR_MASK) << offs);
    *eint_reg = eint_rg_val;
    rtc_local_trigger_wait();
    hal_gpt_delay_us(MAX_EINT_DELAY_TM);

    /* write value */
    eint_rg_val &= ~(0xFF << offs);
    eint_rg_val |= (eint_val << offs);
    *eint_reg    = eint_rg_val;
    rtc_local_trigger_wait();
    rtc_local_config_wakeup_source(wk_src, wk_en);
    return HAL_RTC_STATUS_OK;
}


hal_rtc_status_t    rtc_internal_set_retention_reg(uint32_t offset, uint8_t *buff, uint32_t size)
{
    uint8_t              i      = 0;
    uint8_t              sps_sz = 0;
    uint8_t              offs   = 0;
    uint32_t             temp   = 0;
    uint32_t             idx    = 0;
    volatile uint32_t   *base_reg;
    hal_rtc_status_t     status ;
    uint16_t             reg_mem[12];
    uint8_t              sta_reg_id = 0;
    uint8_t              sto_reg_id = 0;
#ifdef RTC_TIME_CHECK_ENABLE
    uint16_t             tick0, tick1;
    RTC_DEBUG_GET_1M_TICK(tick0);
#endif

    sta_reg_id = (offset / RTC_SPARE_RG_BYTE);
    sto_reg_id = (offset + size) / RTC_SPARE_RG_BYTE;
    base_reg   = &(rtc_register->RTC_SPAR0);

    for (i = sta_reg_id; i < (sto_reg_id + 1); i++) {
        reg_mem[i] = (uint16_t) base_reg[i];
    }

    sps_sz   = RTC_SPARE_RG_NUM * RTC_SPARE_RG_BYTE;
    for (i = 0; i < size; i++) {
        idx = offset + i;
        if (idx < sps_sz) {
            offs     = idx / RTC_SPARE_RG_BYTE;
            idx      = idx % RTC_SPARE_RG_BYTE;
            idx      = (idx * 8);
            temp     = reg_mem[offs];
            temp    &= ~(0xFF << idx);
            temp    |= (buff[i] << idx);
            reg_mem[offs] = temp;
            status = HAL_RTC_STATUS_OK;
        } else {
            status = HAL_RTC_STATUS_ERROR;
            break;
        }
    }
    for (i = sta_reg_id; i < (sto_reg_id + 1); i++) {
        base_reg[i] = (uint32_t)reg_mem[i];
    }
    rtc_local_trigger_wait();
#ifdef RTC_TIME_CHECK_ENABLE
    RTC_DEBUG_GET_1M_TICK(tick1);
    log_rtc_info("[hal][rtc] write retention rg % byte spend %dus)", 2, size, (int)(tick1 - tick0));
#endif
    return status;
}


hal_rtc_status_t    rtc_internal_get_retention_reg(uint32_t offset, uint8_t *buff, uint32_t size)
{
    uint8_t              i      = 0;
    uint8_t              idx    = 0;
    uint8_t              sps_sz = 0;
    uint8_t              offs   = 0;
    volatile uint32_t   *base_reg;
    hal_rtc_status_t     status = HAL_RTC_STATUS_ERROR;
#ifdef RTC_TIME_CHECK_ENABLE
    uint32_t        tick0, tick1;
    RTC_DEBUG_GET_1M_TICK(tick0);
#endif
    sps_sz   = RTC_SPARE_RG_NUM * RTC_SPARE_RG_BYTE;
    for (i = 0; i < size; i++) {
        idx = offset + i;
        if (idx < sps_sz) {
            base_reg = &(rtc_register->RTC_SPAR0);
            offs     = idx / RTC_SPARE_RG_BYTE;
            idx      = idx % RTC_SPARE_RG_BYTE;
            idx      = idx * 8;
            buff[i]  = base_reg[offs] >> (idx);
            status = HAL_RTC_STATUS_OK;
        } else {
            status = HAL_RTC_STATUS_ERROR;
            break;
        }
    }
#ifdef RTC_TIME_CHECK_ENABLE
    RTC_DEBUG_GET_1M_TICK(tick1);
    log_rtc_info("[hal][rtc] read retention rg % byte spend %dus)", 2, size, (int)(tick1 - tick0));
#endif
    return status;
}



#ifdef RTC_SRAM_SUPPORTED
static  void   rtc_local_write_sram_con(uint8_t rg_id, uint32_t value)
{
    switch (rg_id) {
        case 0: {
            rtc_register->RTC_SRAM_CON0 = RTC_SRAM_CON0_MAGIC_KEY_1;
            rtc_local_wait_done();
            rtc_register->RTC_SRAM_CON0 = RTC_SRAM_CON0_MAGIC_KEY_2;
            rtc_local_wait_done();
            rtc_register->RTC_SRAM_CON0 = value;
            rtc_local_wait_done();
        }
        break;
        case 1: {
            rtc_register->RTC_SRAM_CON1 = RTC_SRAM_CON1_MAGIC_KEY_1;
            rtc_local_wait_done();
            rtc_register->RTC_SRAM_CON1 = RTC_SRAM_CON1_MAGIC_KEY_2;
            rtc_local_wait_done();
            rtc_register->RTC_SRAM_CON1 = value;
            rtc_local_wait_done();
        }
        break;

        default:
            break;
    }
}

void    rtc_internal_sram_setting_cg(bool   disable_ck, bool disable_do)
{
    uint32_t  temp = 0;

    rtc_internal_reload();
    temp = rtc_register->RTC_SRAM_CON0;
    if (disable_ck) {
        temp |= RTC_SRAM_CON0_IOS_EN_CKCS_MASK;
    } else {
        temp &= ~RTC_SRAM_CON0_IOS_EN_CKCS_MASK;
    }
    if (disable_do) {
        temp |= RTC_SRAM_CON0_IOS_EN_DO_MASK;
    } else {
        temp &= ~RTC_SRAM_CON0_IOS_EN_DO_MASK;
    }
    rtc_local_write_sram_con(0, temp);
}

void    rtc_internal_sram_setting(hal_rtc_sram_cell_t cell, hal_rtc_sram_mode_t mode)
{
    uint8_t   select_cell = (1 << cell);
    uint32_t  val_con0 = 0, val_con1 = 0;

    rtc_internal_reload();
    val_con1 = rtc_register->RTC_SRAM_CON1;
    val_con0 = rtc_register->RTC_SRAM_CON0;
    switch (mode) {
        case HAL_RTC_SRAM_NORMAL_MODE: {
            val_con1 &= ~(select_cell << RTC_SRAM_CON1_PD_OFFSET);
            val_con1 |= (select_cell << RTC_SRAM_CON1_SLEEP_OFFSET);
            rtc_local_write_sram_con(1, val_con1);
            hal_gpt_delay_us(1);
            val_con0 |= (select_cell << RTC_SRAM_CON0_ISO_INTB_OFFSET);
            rtc_local_write_sram_con(0, val_con0);
        }
        break;

        case HAL_RTC_SRAM_SLEEP_MODE: {
            hal_gpt_delay_us(1);
            val_con0 &= ~(select_cell << RTC_SRAM_CON0_ISO_INTB_OFFSET);
            rtc_local_write_sram_con(0, val_con0);
            hal_gpt_delay_us(2);
            val_con1 &= ~(select_cell << RTC_SRAM_CON1_SLEEP_OFFSET);
            val_con1 &= ~(select_cell << RTC_SRAM_CON1_PD_OFFSET);
            rtc_local_write_sram_con(1, val_con1);
        }
        break;

        case HAL_RTC_SRAM_PD_MODE: {
            val_con0 |= (select_cell << RTC_SRAM_CON0_ISO_INTB_OFFSET);
            val_con1 |= (select_cell << RTC_SRAM_CON1_SLEEP_OFFSET);
            val_con1 |= (select_cell << RTC_SRAM_CON1_PD_OFFSET);
            rtc_local_write_sram_con(0, val_con0);
            rtc_local_write_sram_con(1, val_con1);
        }
        break;
    }
}
#endif


void    rtc_internal_init_register()
{
    _rtc_osc32con_control(0, RTC_OP_SET, 0x101);
    _rtc_osc32con_control(1, RTC_OP_SET, 0xF4F);
    _rtc_osc32con_control(2, RTC_OP_SET, 0x007);
    _rtc_osc32con_control(3, RTC_OP_SET, 0x000);
    /* Clear BBPU */
    rtc_register->RTC_BBPU = KEY_BBPU_MASK_ALL << 8;
    rtc_register->RTC_IRQ_EN = 0x0;
    rtc_register->RTC_CII_EN = 0x0;
    rtc_register->RTC_AL_MASK = 0x0;
    rtc_register->RTC_TC0 = 0x0;
    rtc_register->RTC_TC1 = 0x100;
    rtc_register->RTC_TC2 = 0x101;
    rtc_register->RTC_TC3 = 0x0;
    rtc_register->RTC_AL0 = 0x000;
    rtc_register->RTC_AL1 = 0x100;
    rtc_register->RTC_AL2 = 0x101;
    rtc_register->RTC_AL3 = 0x0;
    rtc_register->RTC_LPD_CON   = 0x100;
    rtc_local_trigger_wait();

    rtc_register->RTC_SPAR0 = 0x0;
    rtc_register->RTC_SPAR1 = 0x0;
    rtc_register->RTC_SPAR2 = 0x0;
    rtc_register->RTC_SPAR3 = 0x0;
    rtc_register->RTC_SPAR4 = 0x0;
    rtc_register->RTC_SPAR5 = 0x0;
    rtc_register->RTC_SPAR_REG = 0x0;
    rtc_register->RTC_DIFF  = 0x0;
    rtc_register->RTC_CALI  = 0x0;
#ifdef RTC_CAPTOUCH_SUPPORTED
    rtc_register->RTC_CAP_CON   = 0x0;
#endif
    rtc_register->RTC_RESET_CON = 1;
    rtc_register->RTC_EINT_CON0 = 0;
    rtc_register->RTC_EINT_CON1 = 0;

    rtc_local_trigger_wait();
}


uint32_t rtc_internal_measure_counter(uint32_t fixed_clock, uint32_t tested_clock, uint32_t window_setting)
{

    uint32_t fqmtr_data = 0;
    uint32_t tick_pre, tick_cur, tick_dur;

    /* 1) *PLL_ABIST_FQMTR_CON1__F_FQMTR_EN = 0x0*/
    rtc_fqmtr_register->RTC_ABIST_FQMTR_CON1 &= (~(1 << RTC_ABIST_ENABLE_OFFSET)); //disable en
    /* 2) *PLL_ABIST_FQMTR_CON1__F_FQMTR_RST = 0x1 */
    rtc_fqmtr_register->RTC_ABIST_FQMTR_CON1 |= (1 << RTC_ABIST_RESET_OFFSET);   //enable reset
    /* 3) *PLL_ABIST_FQMTR_CON1__F_FQMTR_RST = 0x0 */
    rtc_fqmtr_register->RTC_ABIST_FQMTR_CON1 &= ~(1 << RTC_ABIST_RESET_OFFSET);  //disable reset
    /* 4) *CKSYS_TST_SEL_1__F_TST_SEL_1 = REF_CLK << 8 + SRC_CLK */
    *RTC_CKSYS_TST_SEL_1_BASE = (fixed_clock << 8) | (tested_clock);
    /* 5) *PLL_ABIST_FQMTR_CON0__F_FQMTR_WINSET = 0x63 */
    rtc_fqmtr_register->RTC_ABIST_FQMTR_CON0  = (window_setting);
    /* 6) *PLL_ABIST_FQMTR_CON1__F_FQMTR_EN = 0x1 */
    rtc_fqmtr_register->RTC_ABIST_FQMTR_CON1 |= (1 << RTC_ABIST_ENABLE_OFFSET); //enable en
    //log_rtc_info("FQMTR = %x",1,rtc_fqmtr_register->RTC_ABIST_FQMTR_CON1);
    /* 7) Wait FQMTR start*/
    hal_gpt_delay_us(500);
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &tick_pre);
    while (rtc_fqmtr_register->RTC_ABIST_FQMTR_CON1 & 0x80) {
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &tick_cur);
        hal_gpt_get_duration_count(tick_pre, tick_cur, &tick_dur);
        if (tick_dur > 32768) {
            log_rtc_info("[hal][rtc] ABIST_FQMTR-CON0:%x, CON1:%x, CON2:%x, DATA:%x", 4,
                         rtc_fqmtr_register->RTC_ABIST_FQMTR_CON0,
                         rtc_fqmtr_register->RTC_ABIST_FQMTR_CON1,
                         rtc_fqmtr_register->RTC_ABIST_FQMTR_CON2,
                         rtc_fqmtr_register->RTC_ABIST_FQMTR_DATA
                        );
            while (1) {
                log_rtc_info("[hal][rtc] fatal error, mearsure eosc32 failed(clk meter st is busy)......", 0);
                hal_gpt_delay_ms(200);
            }
        }
    }
    /* 8) fqmtr_data =*PLL_ABIST_FQMTR_DATA__F_DATA;*/
    //log_rtc_info("FQMTR STA= %x,FQMTR DATA= %d", 2, rtc_fqmtr_register->RTC_ABIST_FQMTR_CON1, rtc_fqmtr_register->RTC_ABIST_FQMTR_DATA);
    fqmtr_data = rtc_fqmtr_register->RTC_ABIST_FQMTR_DATA;
    /* 8) Disable FQMTR*/
    rtc_fqmtr_register->RTC_ABIST_FQMTR_CON1 &= (~(1 << RTC_ABIST_ENABLE_OFFSET)); //disable en
    return fqmtr_data;
}


#ifdef RTC_CAPTOUCH_SUPPORTED

hal_rtc_status_t  rtc_internal_set_capcon(uint32_t val)
{
    rtc_register->RTC_CAP_CON = 0x05AD;
    rtc_local_trigger_wait();
    rtc_register->RTC_CAP_CON = 0x92E1;
    rtc_local_trigger_wait();
    rtc_register->RTC_CAP_CON = val;
    rtc_local_trigger_wait();
    return HAL_RTC_STATUS_OK;
}


hal_rtc_status_t rtc_internal_captouch_init(void)
{
    rtc_local_config_wakeup_source(RTC_WAKEUP_SOURCE_CAP, true);
    return HAL_RTC_STATUS_OK;
}

hal_rtc_status_t rtc_internal_captouch_deinit(void)
{
    return HAL_RTC_STATUS_OK;
}

hal_rtc_status_t rtc_internal_captouch_lowpower(rtc_captouch_power_type lowpower_type)
{
    return HAL_RTC_STATUS_OK;
}

bool rtc_internal_captouch_get_capcon_state(void)
{
    return true;
}
#endif



void    rtc_internal_dump(char *comment)
{
    (void)comment;

    rtc_internal_reload();
    log_rtc_info("------------ Start Dump ------------", 0);
    log_rtc_info("BBPU(%x), LPD_CON(%x),  WRTGR(%x),  RESETCON(%x)", 4,
                 (unsigned int)rtc_register->RTC_BBPU,
                 (unsigned int)rtc_register->RTC_LPD_CON,
                 (unsigned int)rtc_register->RTC_WRTGR,
                 (unsigned int)rtc_register->RTC_RESET_CON
                );
    log_rtc_info("OSC32CON0(%x),  CON32CON1(%x),  CON32CON2(%x),  CON32CON3(%x)", 4,
                 (unsigned int)rtc_register->RTC_OSC32CON0,
                 (unsigned int)rtc_register->RTC_OSC32CON1,
                 (unsigned int)rtc_register->RTC_OSC32CON2,
                 (unsigned int)rtc_register->RTC_OSC32CON3
                );
    log_rtc_info("EINT0(%x),  EINT1(%x),  CALI(%x),  DIFF(%x), CAP_CON(%x)", 5,
                 (unsigned int)rtc_register->RTC_EINT_CON0,
                 (unsigned int)rtc_register->RTC_EINT_CON1,
                 (unsigned int)rtc_register->RTC_CALI,
                 (unsigned int)rtc_register->RTC_DIFF,
                 (unsigned int)rtc_register->RTC_CAP_CON
                );
    log_rtc_info("TC0(%x), TC1(%x), TC2(%x),  TC3(%x), CII_EN(%x)", 5,
                 (unsigned int)rtc_register->RTC_TC0,
                 (unsigned int)rtc_register->RTC_TC1,
                 (unsigned int)rtc_register->RTC_TC2,
                 (unsigned int)rtc_register->RTC_TC3,
                 (unsigned int)rtc_register->RTC_CII_EN
                );

    log_rtc_info("AL_MASK(%x), AL0(%x), AL1(%x),  AL2(%x), AL3(%x)", 5,
                 (unsigned int)rtc_register->RTC_AL_MASK,
                 (unsigned int)rtc_register->RTC_AL0,
                 (unsigned int)rtc_register->RTC_AL1,
                 (unsigned int)rtc_register->RTC_AL2,
                 (unsigned int)rtc_register->RTC_AL3
                );
    log_rtc_info("SPARE0(%x), SPARE1(%x), SPARE2(%x), SPARE3(%x),SPARE4(%x), SPARE5(%x)", 6,
                 (unsigned int)rtc_register->RTC_SPAR0,
                 (unsigned int)rtc_register->RTC_SPAR1,
                 (unsigned int)rtc_register->RTC_SPAR2,
                 (unsigned int)rtc_register->RTC_SPAR3,
                 (unsigned int)rtc_register->RTC_SPAR4,
                 (unsigned int)rtc_register->RTC_SPAR5
                );
    log_rtc_info("DEBUG_CON0(%x), DEBUG_CON1(%x)", 2,
                 (unsigned int)rtc_register->RTC_DEBUG_CON_0,
                 (unsigned int)rtc_register->RTC_DEBUG_CON_1
                );
    log_rtc_info("------------ End  Dump ------------", 0);
}
/***********************************************************************************************
 * Private using api
***********************************************************************************************/
/*private macro, donnot modify*/
#define     HAL_RTC_CHK_MONTH_OFFSET    (22)
#define     HAL_RTC_CHK_DAY_OFFSET      (17)
#define     HAL_RTC_CHK_HOUR_OFFSET     (12)
#define     HAL_RTC_CHK_MINUTE_OFFSET   (6)
#define     HAL_RTC_CHK_SECOND_OFFSET   (0)
#define     HAL_RTC_CHK_MONTH_MASK      (0x0F)
#define     HAL_RTC_CHK_DAY_MASK        (0x1F)
#define     HAL_RTC_CHK_HOUR_MASK       (0x1F)
#define     HAL_RTC_CHK_MINUTE_MASK     (0x3F)
#define     HAL_RTC_CHK_SECOND_MASK     (0x3F)
#define     RTC_DAY_OF_FOUR_YEAR        (366+365+365+365)
#define     RTC_SECOND_OF_DAY           (24*3600)
#define     RTC_SECOND_OF_FOUR_YEAR     (RTC_DAY_OF_FOUR_YEAR*24*3600)
#define     RTC_SECOND_OF_LYEAR         (366*24*3600)
#define     RTC_SECOND_OF_YEAR          (365*24*3600)

static  uint32_t                g_rtc_year_days[]       = {366, 365, 365, 365};
static  uint32_t                g_rtc_year_month_day[]  = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static  uint32_t                g_rtc_lyear_month_day[] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};


uint32_t rtc_internal_time_to_tick(uint8_t  year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t second)
{
    uint32_t    year_x4   = 0, year_lf = 0;
    uint32_t    total_sec = 0, temp    = 0;
    uint32_t    i     = 0;
    uint32_t    *pdom = NULL;

    year_x4 = year >> 2;
    year_lf = year % 4;
    temp = RTC_DAY_OF_FOUR_YEAR * year_x4;
    /*calc year second*/
    for (i = 0; i < year_lf; i++) {
        temp += g_rtc_year_days[i];
    }
    /*calc month second*/
    pdom = (year_lf == 0) ? g_rtc_lyear_month_day : g_rtc_year_month_day;
    for (i = 1; i < month; i++) {
        temp += pdom[i];
    }
    if (day > 0) {
        temp += (day - 1);
    }
    total_sec = temp * RTC_SECOND_OF_DAY + hour * 3600 + min * 60 + second;
    return total_sec;
}

bool rtc_internal_tick_to_time(uint32_t tick, hal_rtc_time_t *time)
{
    uint32_t    one_year_second  = 0;
    uint32_t    year_x4 = 0;
    uint32_t    year_lf = 0;
    uint32_t    i       = 0;
    uint32_t   *pdom    = NULL;
    uint32_t    uitmp   = 0;

    if (time == NULL) {
        return false;
    }
    year_x4 = tick / RTC_SECOND_OF_FOUR_YEAR;
    tick   -= (year_x4 * RTC_SECOND_OF_FOUR_YEAR);
    one_year_second = RTC_SECOND_OF_LYEAR;
    /*calc year*/
    uitmp    = year_x4 * 4;
    for (i = 0; i < 4; i++) {

        if (tick > one_year_second) {
            tick -= one_year_second;
            one_year_second = RTC_SECOND_OF_YEAR;
            uitmp++;
            year_lf++;
        } else {
            break;
        }
    }
    time->rtc_year = uitmp;
    /*calc month*/
    pdom = (year_lf == 0) ? (g_rtc_lyear_month_day) : (g_rtc_year_month_day);
    for (i = 1; i < 13; i++) {
        uitmp = pdom[i] * RTC_SECOND_OF_DAY;
        if (tick > uitmp) {
            tick -= uitmp;
        } else {
            break;
        }
    }
    time->rtc_mon = i;
    /*calc day*/
    uitmp = tick / RTC_SECOND_OF_DAY;
    tick -= (uitmp * RTC_SECOND_OF_DAY);
    uitmp++;
    time->rtc_day = uitmp;
    /*calc hour*/
    uitmp  = tick / 3600;
    tick -= uitmp * 3600;
    time->rtc_hour = uitmp;
    /*calc minute*/
    uitmp = tick / 60;
    tick -= (uitmp * 60);
    time->rtc_min = uitmp;
    /*calc second*/
    time->rtc_sec = tick;
    return true;
}


/***********************************************************************************************
 * Special API For ESCO HW Issue
 ***********************************************************************************************/

/*****************************************************************************
 * Description:
 *   Below functions is for resolve captouch-32k is disturb by audio 812.5kHz
 *   and EOSC Drift > 1500ppm
 *   For resolve this, using DCXO-32k when audio/a2dp/hfp is enabled;other case using
     EOSC-32k for save power.
 *       Because of bt using 32k clock, so before switch 32k clock source need
 *   BT switch clock from 32k to 26M first, that is avoid BT disconnect.
 *       The 32k clock may stop 10us when 32k clock switch proecess.
 *******************************************************************************/
#if (defined(__EXT_BOOTLOADER__) || defined(__EXT_DA__))
#else

#ifdef RTC_USING_EOSC_FOR_NORMAL_MODE

#include "hal_clock_internal.h"

#define GET_1M_TICK(tick)       hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, tick)

extern      void            bt_switch_clock_to_26M(void);
extern      void            bt_switch_clock_to_32K(void);
extern      void            bt_wait_clock_ready(void);
extern      bool            bt_check_32k_clock(void);
extern hal_nvic_status_t    hal_nvic_save_and_set_interrupt_mask_special(uint32_t *mask);
extern hal_nvic_status_t    hal_nvic_restore_interrupt_mask_special(uint32_t mask);

static uint16_t             rtc_debug_info_bkup[3][4] = {0};
static uint8_t              rtc_debug_idx = 0;
static uint32_t             rtick1, rtick2, rtick3, rtick4, rtick5;

static void rtc_debug_code()
{
    uint32_t temp = 0;

    if (rtc_debug_idx >= 3) {
        rtc_debug_idx = 0;
    }
    /*output EOSC-32k to GPIO*/
    temp = 0;
    rtc_register->RTC_DEBUG_CON_0 = RTC_DEBUG_CON0_DBG_EN_MASK | (temp << RTC_DEBUG_CON0_DBG_SEL_OFFSET);
    hal_gpt_delay_us(2);
    rtc_debug_info_bkup[rtc_debug_idx][0] = rtc_register->RTC_DEBUG_CON_1;

    temp = 1;
    rtc_register->RTC_DEBUG_CON_0 = RTC_DEBUG_CON0_DBG_EN_MASK | (temp << RTC_DEBUG_CON0_DBG_SEL_OFFSET);
    hal_gpt_delay_us(2);
    rtc_debug_info_bkup[rtc_debug_idx][1] = rtc_register->RTC_DEBUG_CON_1;

    temp = 7;
    rtc_register->RTC_DEBUG_CON_0 = RTC_DEBUG_CON0_DBG_EN_MASK | (temp << RTC_DEBUG_CON0_DBG_SEL_OFFSET);
    hal_gpt_delay_us(2);
    rtc_debug_info_bkup[rtc_debug_idx][2] = rtc_register->RTC_DEBUG_CON_1;

    temp = 8;
    rtc_register->RTC_DEBUG_CON_0 = RTC_DEBUG_CON0_DBG_EN_MASK | (temp << RTC_DEBUG_CON0_DBG_SEL_OFFSET);
    hal_gpt_delay_us(2);
    rtc_debug_info_bkup[rtc_debug_idx][3] = rtc_register->RTC_DEBUG_CON_1;
    rtc_debug_idx++;
}

static void hal_rtc_switch_clock_with_check(hal_rtc_osc32k_mode_t mode)
{
    uint32_t timeout = 10;

    while (1) {
        if (hal_rtc_switch_32k_source(mode) == HAL_RTC_STATUS_OK) {
            break;
        }
        if (timeout == 0) {
            assert(0);
        } else {
            rtc_debug_code();
        }
        timeout--;
    }
}

static void rtc_safe_switch_32k_source(hal_rtc_osc32k_mode_t sel_32k_src)
{
    uint32_t temp_flag = 0;

    GET_1M_TICK(&rtick1);
    if (bt_check_32k_clock()) {
        bt_switch_clock_to_26M(); /* Only write registers. */
        temp_flag = 1;
    }
    GET_1M_TICK(&rtick2);
    hal_rtc_switch_clock_with_check(sel_32k_src); /* switch to EOSC-32k */
    GET_1M_TICK(&rtick3);
    hal_gpt_delay_us(500); /* dummy wait for BT */
    GET_1M_TICK(&rtick4);
    bt_wait_clock_ready(); /*about0~ 500us: clear reg and polling reg */
    if (temp_flag == 1) {
        bt_switch_clock_to_32K();
        temp_flag = 0;
    }
    GET_1M_TICK(&rtick5);
}
#endif /* RTC_USING_EOSC_FOR_NORMAL_MODE */

#define     HAL_RTC_MAGIC_PROTECT_ID   0xA7000000

void    hal_rtc_switch_to_dcxo(hal_rtc_clock_switch_user_t user_id, bool    enable_dcxo)
{
#ifdef RTC_USING_EOSC_FOR_NORMAL_MODE
    static uint32_t s_user_enable_state = HAL_RTC_MAGIC_PROTECT_ID;
    uint32_t user_mask = 0, irq_mask = 0;
    uint32_t tick0, tick1;

    if (user_id >= HAL_RTC_CLOCK_USER_NONE) {
        log_rtc_error("[hal][rtc] hal_rtc_switch_to_dcxo() fail: err user(%d)", 1, user_id);
        return;
    }
    /*New chip design have fixed this hw limitation.(157x/158x/2831/2833 and later chip)*/
    if (HAL_RTC_CLOCK_USER_AUDIO == user_id) {
        (void)user_id;
        (void)enable_dcxo;
        return;
    }

    user_mask = (1 << user_id);
    log_rtc_warn("[hal][rtc] hal_rtc_switch_to_dcxo(cur_sta 0x%x): user%d en/disable(%d) dcxo", 3, s_user_enable_state, user_id, enable_dcxo);

    hal_nvic_save_and_set_interrupt_mask_special(&irq_mask);
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &tick0);

    /* if status is corrupt, then assert it */
    if((HAL_RTC_MAGIC_PROTECT_ID & s_user_enable_state) != HAL_RTC_MAGIC_PROTECT_ID) {
        assert(0);
    }
    /* if one user enable two times, then assert it */
    if ((enable_dcxo == 1) && ((s_user_enable_state & user_mask) == user_mask)) {
        assert(0);
    }
    /* if one user disable two times, then assert it */
    if ((enable_dcxo == 0) && ((s_user_enable_state & user_mask) == 0)) {
        assert(0);
    }
    /* switch 32k clock source */
    if (enable_dcxo == true) {
        if (s_user_enable_state == 0) {
#ifdef HAL_SLEEP_MANAGER_ENABLED
            spm_force_sleep_state(SPM_RTC, SPM_STATE4, SPM_ENABLE); /* SPM lock dcxo*/
#endif
            //switch_system_32k_to_dcxo();
            rtc_safe_switch_32k_source(HAL_RTC_OSC32K_DCXO_MODE);
        }
        s_user_enable_state |= user_mask;
    } else {
        s_user_enable_state &= ~user_mask;
        if (s_user_enable_state == 0) {
#ifdef HAL_SLEEP_MANAGER_ENABLED
            spm_force_sleep_state(SPM_RTC, SPM_STATE4, SPM_DISABLE); /* SPM unlock dcxo*/
#endif
            //switch_system_32k_to_eosc();
            rtc_safe_switch_32k_source(HAL_RTC_OSC32K_EOSC_MODE);
        }
    }
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &tick1);
    hal_nvic_restore_interrupt_mask_special(irq_mask);

    if (s_user_enable_state == 0) {
        /*for spm rd request*/
        log_rtc_info("[hal][spm] lock sleep user=%d, state=%d, enable=%d", 3, SPM_RTC, SPM_STATE4, enable_dcxo);
    }
    hal_gpt_delay_ms(1);
    log_rtc_info("[hal][rtc] hal_rtc_switch_to_dcxo(cur_sta 0x%x): end", 1, s_user_enable_state);
    //log_rtc_warn("[hal][rtc] hal_rtc_switch_to_dcxo:state %x, time %d:%d-%d-%d-%dus)", 6, s_user_enable_state, tick1 - tick0, rtick2 - rtick1, rtick3 - rtick2, rtick4 - rtick3, rtick5 - rtick4);
#else
    (void)user_id;
    (void)enable_dcxo;
    log_rtc_warn("[hal][rtc] hal_rtc_switch_to_dcxo: no need switch, always dcxo", 0);
#endif /* RTC_USING_EOSC_FOR_NORMAL_MODE */
}
#endif /*__EXT_BOOTLOADER__ */
#endif /*HAL_RTC_MODULE_ENABLED*/



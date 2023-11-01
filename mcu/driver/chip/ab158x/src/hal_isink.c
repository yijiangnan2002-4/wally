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

#include "hal.h"
#include "hal_platform.h"
#ifdef HAL_ISINK_MODULE_ENABLED
#include "hal_isink.h"
#include "hal_isink_internal.h"
#include "hal_pmu.h"
#include "hal_spm.h"

/*Option for enable isink when system in dleep*/
#define HAL_ISINK_SLEEP_ALIVE_ENABLED

#ifdef ISINK_PLAIN_LOG_ENABLE
#define log_isink_info(_message, cnt, ...)    printf(_message, ##__VA_ARGS__)
#define log_isink_warn(_message, cnt, ...)    printf(_message, ##__VA_ARGS__)
#define log_isink_error(_message, cnt, ...)   printf(_message, ##__VA_ARGS__)
#else
#define log_isink_info(_message, cnt, ...)    log_hal_msgid_info(_message, cnt, ##__VA_ARGS__)
#define log_isink_warn(_message, cnt, ...)    log_hal_msgid_warning(_message, cnt, ##__VA_ARGS__)
#define log_isink_error(_message, cnt, ...)   log_hal_msgid_error(_message, cnt, ##__VA_ARGS__)
#endif

#define     PMU_CKCFG2_DRV_F32K_CK_PDN_OFFS         (10)
#define     PMU_CKCFG2_DRV_F32K_CK_PDN_MASK         (0x1)

#define     PMU_LED_EN_CTRL_CLK_LP_EN_OFFS          (4)
#define     PMU_LED_EN_CTRL_CLK_LP_EN_MASK          (0x1)

#define     PMU_ISINK_CH0_STEP_SHIFT                (0)
#define     PMU_ISINK_CH0_DOUBLE_SHIFT              (3)
#define     PMU_ISINK_CH1_STEP_SHIFT                (4)
#define     PMU_ISINK_CH1_DOUBLE_SHIFT              (7)

#define     PMU_ISINK_CH0_STEP_MASK                 (0x7)
#define     PMU_ISINK_CH0_DOUBLE_MASK               (0x1)
#define     PMU_ISINK_CH1_STEP_MASK                 (0x7)
#define     PMU_ISINK_CH1_DOUBLE_MASK               (0x1)

#define     PMU_LED_EN_CTRL_CH0_EN_OFFS             (0)
#define     PMU_LED_EN_CTRL_CH1_EN_OFFS             (1)
#define     PMU_LED_EN_CTRL_CH0_CHIOP_EN_OFFS       (2)
#define     PMU_LED_EN_CTRL_CH1_CHIOP_EN_OFFS       (3)


#define     ISINK_EN_CTRL_RETRY_TIME  3

#ifdef HAL_ISINK_SLEEP_ALIVE_ENABLED
static  uint32_t    g_isink_channel_state = 0;
#endif

#ifdef AIR_BTA_IC_PREMIUM_G3_TYPE_A
#define LED_EN_CTRL     PMU_LED_EN_CTRL

static  uint16_t    pmu_led_reg[2][PMU_LED_NONE] = {
    { PMU_LED0_CON0, PMU_LED0_UNIT_CTRL, PMU_LED0_PERIOD0, PMU_LED0_PERIOD1, PMU_LED0_PWM0, PMU_LED0_PWM1, PMU_LED0_PWM2},
    { PMU_LED1_CON0, PMU_LED1_UNIT_CTRL, PMU_LED1_PERIOD0, PMU_LED1_PERIOD1, PMU_LED1_PWM0, PMU_LED1_PWM1, PMU_LED1_PWM2},
};
#else
#define LED_EN_CTRL     0x0500

static  uint16_t    pmu_led_reg[2][PMU_LED_NONE] = {
    /*ledx_con     led_unit         led_period0   led_period1     led_pwm0     led_pwm1      led_pwm2*/
    { 0x0502,   0x0504,     0x0506,     0x0508,     0x050A,     0x050C,     0x050E},
    { 0x0510,   0x0512,     0x0514,     0x0516,     0x0518,     0x051A,     0x051C},
};
#endif

static bool    led_write_register(uint16_t address, uint16_t value)
{
    pmu_operate_status_t status;
    status = pmu_set_register_value(address, 0xFFFFFFFF, 0, value);
    if (status != PMU_OK) {
        return false;
    }
    return true;
}


static  uint32_t    led_read_register(uint16_t address)
{
    uint32_t  temp;
    temp = pmu_get_register_value(address, 0xFFFFFFFF, 0);
    return temp;
}


static int led_set_timeunit(uint8_t led, int tm_unit_ms)
{
    int xn = 0, unit = 0, i = 0;
    int temp;

    for (xn = 1; xn < 9; xn = xn * 2) {
        unit = (tm_unit_ms * 32) / xn;
        if (unit <= 255) {
            break;
        }
        i++;
    }
    if (xn <= 8) {
        temp = (i << 8) | unit;

        led_write_register(pmu_led_reg[led][PMU_LED_UNIT], temp);
        temp = ((xn * unit) / 32);
        //log_hal_msgid_info("[hal][isink]led time unit:%d,  rg_xn:%d, rg_unit:%d", 3, temp, xn, unit);
        return temp;
    } else {
        return 0;
    }
}

static bool    led_set_timing(uint8_t led, uint32_t t0, uint32_t t1, uint32_t t2, uint32_t t3, uint32_t time_unit)
{
    uint8_t     tmp0, tmp1;

    tmp0 = t0 / time_unit;
    tmp1 = t1 / time_unit;

    if (t0 != 0) {
        tmp0 = (tmp0 == 0) ? 1 : tmp0;
    }
    if (t1 != 0) {
        tmp1 = (tmp1 == 0) ? 1 : tmp1;
    }

    led_write_register(pmu_led_reg[led][PMU_LED_PERIOD0], (tmp0 << 8 | tmp1)); /* set T0 & T1*/
    tmp0 = t2 / time_unit;
    tmp1 = t3 / time_unit;
    led_write_register(pmu_led_reg[led][PMU_LED_PERIOD1], (tmp0 << 8 | tmp1)); /* set T2 & T3*/
    return true;
}


static bool    led_set_mode(uint8_t led, uint32_t t1, uint32_t t2, bool is_blink)
{
#define     ISINK_LED_DURATION      100
    uint32_t    min;
    uint32_t    pwm_cyc_tm;
    uint32_t    pwm_period;
    uint32_t    pwm_duration;
    uint32_t    pwm_step;

    min = (t1 > t2) ? t2 : t1;
    if (min <= 500) {
        pwm_cyc_tm   = 1;//1ms
        pwm_period   = 31;
    } else if (min <= 2000) {
        pwm_cyc_tm   = 5;//5ms
        pwm_period   = 159;
    } else {
        pwm_cyc_tm   = 8;//8ms
        pwm_period   = 255;
    }

    min = (min >> 1);
    if (min != 0) {
        pwm_step = (pwm_period + 1) / (min / pwm_cyc_tm);
        pwm_step = (pwm_step == 0) ? 1 : pwm_step;
    } else {
        pwm_step = 0;
    }
    if (is_blink) {
        pwm_period   = 31;
        pwm_step     = 0;
    }
    pwm_duration = (pwm_period + 1) / 2;
    led_write_register(pmu_led_reg[led][PMU_LED_PWM1], (pwm_period << 8) | pwm_duration);
    led_write_register(pmu_led_reg[led][PMU_LED_PWM2], (pwm_step << 8)   | pwm_step);
    //led_write_register(pmu_led_reg[led][PMU_LED_PWM0], 1);
    //log_hal_msgid_info("[hal][isink] set mode, period %d, duaration:%d, pwm_step:%d", 3, pwm_period, pwm_period, pwm_step);
    return true;
}



void    led_set_repeat(uint8_t led, uint32_t blink_num)
{
    uint32_t mask;

    mask = led_read_register(pmu_led_reg[led][PMU_LED_CON0]);
    mask &= ~(0x1F);
    blink_num = (blink_num > 16) ? 0 : blink_num;
    if (blink_num == 0) {
        mask |= (1 << 4);
    } else {
        mask |= ((blink_num - 1) & 0xF);
    }
    led_write_register(pmu_led_reg[led][PMU_LED_CON0], mask);
}


#ifdef HAL_ISINK_FEATURE_HW_PMIC2562
static  void    led_set_style(uint8_t led, hal_isink_config_ext_t *config)
{
#define     ISINK_LED_DURATION      100
    uint32_t     pwm_cyc_tm;
    uint32_t     pwm_period;
    uint32_t     pwm_duration;
    uint32_t     pwm_step_on = 0, pwm_step_off = 0;
    uint32_t     tr = 0, tf = 0, max = 0;

    /* get max time of led setting and select appropriate led cycle setting */
    tr = config->timing.t1.t_rising;
    tf = config->timing.t2.t_falling;

    max = (tr > tf) ? tr : tf;
    if (tr == 0) {
        max = tr;
    } else if (tf == 0) {
        max = tr;
    }
    if (max <= 255) {
        pwm_cyc_tm   = 1;//1ms
        pwm_period   = 31;
    } else if (max <= 1280) {
        pwm_cyc_tm   = 5;//5ms
        pwm_period   = 159;
    } else {
        pwm_cyc_tm   = 8;//8ms
        pwm_period   = 255;
    }

    pwm_duration = ((pwm_period) * (config->brightness)) / 255;
    pwm_duration = (pwm_duration == 0) ? (pwm_period >> 1) : pwm_duration;

    /* calc the step_on and setp_off setting value */
    if (config->timing.t1.t_rising != 0) {
        pwm_step_on = (config->timing.t1.t_rising  / pwm_cyc_tm / pwm_duration) + 1;
    } else {
        pwm_step_on = 0;
    }
    if (config->timing.t2.t_falling != 0) {
        pwm_step_off = (config->timing.t2.t_falling / pwm_cyc_tm / pwm_duration) + 1;
    } else {
        pwm_step_off = 0;
    }

    led_write_register(pmu_led_reg[led][PMU_LED_PWM1], (pwm_period << 8)   | pwm_duration);
    led_write_register(pmu_led_reg[led][PMU_LED_PWM2], (pwm_step_off << 8) | pwm_step_on);
    //led_write_register(pmu_led_reg[led][PMU_LED_PWM0], 1);
    //log_hal_msgid_info("[hal][isink] config, tr(%dms), tf(%d), cyc_tm(%d),duration(0x%x)", 4, config->timing.t1.t_rising,config->timing.t2.t_falling, pwm_cyc_tm, pwm_duration);
    //log_hal_msgid_info("[hal][isink] config, step on/off:%x/%x, period %x,duration %x", 4, pwm_step_on, pwm_step_off, pwm_period, pwm_duration);
}
#endif






hal_isink_status_t  hal_isink_init(hal_isink_channel_t  channel)
{

#ifdef AIR_BTA_IC_PREMIUM_G3_TYPE_A
    /* enable f32k for isink*/
    pmu_set_register_value(PMU_CKCFG2,      PMU_CKCFG2_DRV_F32K_CK_PDN_MASK,    PMU_CKCFG2_DRV_F32K_CK_PDN_OFFS,    0);
    pmu_set_register_value(PMU_LED_EN_CTRL, PMU_LED_EN_CTRL_CLK_LP_EN_MASK,     PMU_LED_EN_CTRL_CLK_LP_EN_OFFS,     0);
#endif /* AIR_BTA_IC_PREMIUM_G3_TYPE_A */

    /*protect isink not turn off when system into sleep*/
#ifdef HAL_ISINK_SLEEP_ALIVE_ENABLED
    uint32_t save_mask;
#if defined(HAL_SLEEP_MANAGER_ENABLED) && defined(AIR_BTA_IC_PREMIUM_G3_TYPE_A)
    spm_force_on_pmic(SPM_ISINK_REQUEST, SPM_ENABLE);
#endif
    hal_nvic_save_and_set_interrupt_mask(&save_mask);
    g_isink_channel_state |= 1 << channel;
    hal_nvic_restore_interrupt_mask(save_mask);
    log_hal_msgid_info("[hal][isink] turn on pmic pwr protect\r\n", 0);
#endif
    return HAL_ISINK_STATUS_OK;
}

hal_isink_status_t  hal_isink_deinit(hal_isink_channel_t channel)
{
    bool    status = true;

    /*unprotect isink*/
#ifdef HAL_ISINK_SLEEP_ALIVE_ENABLED
    uint32_t save_mask;

    hal_nvic_save_and_set_interrupt_mask(&save_mask);
    g_isink_channel_state &= ~(1 << channel);
    hal_nvic_restore_interrupt_mask(save_mask);
    if (g_isink_channel_state == 0) {
#ifdef AIR_BTA_IC_PREMIUM_G3_TYPE_A
        /* disable isink 32k clock*/
        pmu_set_register_value(PMU_LED_EN_CTRL, 0xFFFFFFFF, 0, 0);
        pmu_set_register_value(PMU_CKCFG2,      PMU_CKCFG2_DRV_F32K_CK_PDN_MASK, PMU_CKCFG2_DRV_F32K_CK_PDN_OFFS,   1);
#ifdef HAL_SLEEP_MANAGER_ENABLED
        spm_force_on_pmic(SPM_ISINK_REQUEST, SPM_DISABLE);
#endif
#endif
        log_hal_msgid_info("[hal][isink] turn off pmic pwr protect\r\n", 0);
    }
#endif
    if (status == false) {
        return HAL_ISINK_STATUS_ERROR;
    } else {
        return HAL_ISINK_STATUS_OK;
    }
}

hal_isink_status_t  hal_isink_set_clock_source(hal_isink_channel_t channel, hal_isink_clock_source_t source_clock)
{
    return HAL_ISINK_STATUS_OK;
}

hal_isink_status_t  hal_isink_set_mode(hal_isink_channel_t channel, hal_isink_mode_t mode)
{
    return HAL_ISINK_STATUS_OK;
}

hal_isink_status_t hal_isink_set_step_current(hal_isink_channel_t channel, hal_isink_current_t current)
{
    if(channel >= HAL_ISINK_MAX_CHANNEL){
        return HAL_ISINK_STATUS_ERROR_INVALID_PARAMETER;
    }
#ifdef AIR_BTA_IC_PREMIUM_G3_TYPE_A
    bool status = true;

    switch (channel) {
        case HAL_ISINK_CHANNEL_0:
            status &= pmu_set_register_value(PMU_ISINK_ANA_CON1, PMU_ISINK_CH0_STEP_MASK, PMU_ISINK_CH0_STEP_SHIFT, (uint32_t)current);
            break;
        case HAL_ISINK_CHANNEL_1:
            status &= pmu_set_register_value(PMU_ISINK_ANA_CON1, PMU_ISINK_CH1_STEP_MASK, PMU_ISINK_CH1_STEP_SHIFT, (uint32_t)current);
            break;
        default:
            return HAL_ISINK_STATUS_ERROR_CHANNEL;
    }

    if (true == status) {
        return HAL_ISINK_STATUS_OK;
    } else {
        return HAL_ISINK_STATUS_ERROR;
    }
#else
    uint32_t    mask;
    mask = led_read_register(pmu_led_reg[channel][PMU_LED_CON0]);
    mask &= ~(0x3<<8);
    mask |= ((current & 0x3) << 8);
    led_write_register(pmu_led_reg[channel][PMU_LED_CON0], mask);
    return HAL_ISINK_STATUS_ERROR;
#endif
}

hal_isink_status_t hal_isink_set_double_current(hal_isink_channel_t channel, hal_isink_current_t current)
{
#ifdef AIR_BTA_IC_PREMIUM_G3_TYPE_A
    bool status = true;
    switch (channel) {
        case HAL_ISINK_CHANNEL_0:
            status &= pmu_set_register_value(PMU_ISINK_ANA_CON1,   PMU_ISINK_CH0_STEP_MASK,   PMU_ISINK_CH0_STEP_SHIFT, (uint32_t)current);
            status &= pmu_set_register_value(PMU_ISINK_ANA_CON1,   PMU_ISINK_CH0_DOUBLE_MASK, PMU_ISINK_CH0_DOUBLE_SHIFT, 1);

            break;
        case HAL_ISINK_CHANNEL_1:
            status &= pmu_set_register_value(PMU_ISINK_ANA_CON1,   PMU_ISINK_CH1_STEP_MASK,   PMU_ISINK_CH1_STEP_SHIFT, (uint32_t)current);
            status &= pmu_set_register_value(PMU_ISINK_ANA_CON1,   PMU_ISINK_CH1_DOUBLE_MASK, PMU_ISINK_CH1_DOUBLE_SHIFT, 1);
            break;
        default:
            return HAL_ISINK_STATUS_ERROR_CHANNEL;
    }
    if (true == status) {
        return HAL_ISINK_STATUS_OK;
    } else {
        return HAL_ISINK_STATUS_ERROR;
    }
#else
    return HAL_ISINK_STATUS_OK;
#endif
}


hal_isink_status_t hal_isink_enable_breath_mode(hal_isink_channel_t channel, hal_isink_breath_mode_t breath_mode)
{
    return HAL_ISINK_STATUS_OK;
}

hal_isink_status_t hal_isink_enable_pwm_mode(hal_isink_channel_t channel, hal_isink_pwm_mode_t pwm_mode)
{
    return HAL_ISINK_STATUS_OK;
}

hal_isink_status_t hal_isink_enable_register_mode(hal_isink_channel_t channel, hal_isink_current_t current, bool double_en)
{
    return HAL_ISINK_STATUS_OK;
}


hal_isink_status_t hal_isink_configure(hal_isink_channel_t channel, hal_isink_config_t *cfg)
{
    uint32_t t_hi, t_lo, t_min, t_unit;

    if (channel >= HAL_ISINK_MAX_CHANNEL || cfg == NULL) {
        log_hal_msgid_info("[hal][isink] config fail, invalid param", 0);
        return HAL_ISINK_STATUS_ERROR_INVALID_PARAMETER;
    }
    t_hi = cfg->config.pwm_mode.hi_level_time;
    t_lo = cfg->config.pwm_mode.lo_level_time;
    t_min = (t_hi > t_lo) ? t_lo : t_hi;
    if (t_min < 50) {
        t_unit = 1;//1ms
    } else {
        t_unit = 50;//50ms
    }
    led_set_timeunit(channel, t_unit);
    switch (cfg->mode) {
        case HAL_ISINK_MODE_BREATH: {
            log_hal_msgid_info("[hal][isink] config for breath mode, t1(%d),t2(%d),blink_num(%d)", 3,
                               cfg->config.pwm_mode.hi_level_time,
                               cfg->config.pwm_mode.lo_level_time,
                               cfg->config.pwm_mode.blink_nums
                              );
            led_set_repeat(channel, cfg->config.pwm_mode.blink_nums);
            led_set_timing(channel, 0, cfg->config.pwm_mode.hi_level_time, cfg->config.pwm_mode.lo_level_time, 0, t_unit);
            led_set_mode(channel, cfg->config.pwm_mode.hi_level_time, cfg->config.pwm_mode.lo_level_time, false);
        }
        break;
        case HAL_ISINK_MODE_PWM: {
            log_hal_msgid_info("[hal][isink] config for blink mode, t1(%d),t2(%d),blink_num(%d)", 3,
                               cfg->config.pwm_mode.hi_level_time,
                               cfg->config.pwm_mode.lo_level_time,
                               cfg->config.pwm_mode.blink_nums
                              );
            led_set_repeat(channel, cfg->config.pwm_mode.blink_nums);
            led_set_timing(channel, 0, cfg->config.pwm_mode.hi_level_time, cfg->config.pwm_mode.lo_level_time, 0, t_unit);
            led_set_mode(channel, cfg->config.pwm_mode.hi_level_time, cfg->config.pwm_mode.lo_level_time, true);
        }
        break;
        case HAL_ISINK_MODE_REGISTER: {

        } break;

        default: {
            return HAL_ISINK_STATUS_ERROR_INVALID_PARAMETER;
        }
    }
    return HAL_ISINK_STATUS_OK;
}


#ifdef HAL_ISINK_FEATURE_HW_PMIC2562
hal_isink_status_t  hal_isink_configure_ext(hal_isink_channel_t channel, hal_isink_config_ext_t *cfg)
{
    uint32_t t_hi, t_lo, t_max, t_unit;

    if (channel >= HAL_ISINK_MAX_CHANNEL || cfg == NULL) {
        log_hal_msgid_info("[hal][isink] config fail, invalid param", 0);
        return HAL_ISINK_STATUS_ERROR_INVALID_PARAMETER;
    }
    t_hi = cfg->timing.t1.t_rising  + cfg->timing.t1.t_lightest;
    t_lo = cfg->timing.t2.t_falling + cfg->timing.t2.t_darkest;
    t_max = (t_hi > t_lo) ? t_hi : t_lo;
    if (t_hi == 0) {
        t_max = t_lo;
    } else if (t_lo == 0) {
        t_max = t_hi;
    }
    if (t_max < 255) {
        t_unit = 1;//1ms
    } else {
        t_unit = 50;//50ms
    }
    led_set_timeunit(channel, t_unit);
    led_set_repeat(channel, cfg->blink_nums);
    led_set_timing(channel, cfg->timing.t0, t_hi, t_lo, cfg->timing.t3, t_unit);
    led_set_style(channel, cfg);

    return HAL_ISINK_STATUS_OK;
}
#endif



hal_isink_status_t hal_isink_start(uint32_t channel)
{
    uint32_t    mask = 0;
    uint32_t i = 0,bit_l,bit_h;

    if (channel >= HAL_ISINK_MAX_CHANNEL) {
        log_hal_msgid_info("[hal][isink] start fail, invalid param", 0);
        return HAL_ISINK_STATUS_ERROR_INVALID_PARAMETER;
    }
    /* load pwm setting*/
    led_write_register(pmu_led_reg[channel][PMU_LED_PWM0], 1);
    /* wait led_en_sync enable */
    for(i = 0;i < ISINK_EN_CTRL_RETRY_TIME; i++) {
        mask  = led_read_register(LED_EN_CTRL);
        bit_l = (mask >> channel) & 0x1;
        bit_h = (mask >> (channel + 8)) & 0x1;
        if (bit_l == bit_h) {
            break;
        } else {
            hal_gpt_delay_us(32);
        }
    }
#ifdef AIR_BTA_IC_PREMIUM_G3_TYPE_A
    mask |= ((1 << channel) | (1 << (channel + 2))); /* enable led & enable chop */
#else
    mask |= (1 << channel); /* enable led*/
#endif
    led_write_register(LED_EN_CTRL, mask);
    return HAL_ISINK_STATUS_OK;
}

hal_isink_status_t hal_isink_stop(uint32_t channel)
{
    uint32_t    mask = 0;
    uint32_t i = 0,bit_l,bit_h;

    if (channel >= HAL_ISINK_MAX_CHANNEL) {
        log_hal_msgid_info("[hal][isink] stop fail, invalid param", 0);
        return HAL_ISINK_STATUS_ERROR_INVALID_PARAMETER;
    }

    /* wait led_en_sync enable */
    for(i = 0;i < ISINK_EN_CTRL_RETRY_TIME; i++) {
        mask  = led_read_register(LED_EN_CTRL);
        bit_l = (mask >> channel) & 0x1;
        bit_h = (mask >> (channel + 8)) & 0x1;
        if (bit_l == bit_h) {
            break;
        } else {
            hal_gpt_delay_us(32);
        }
    }

#ifdef AIR_BTA_IC_PREMIUM_G3_TYPE_A
    mask &= ~((1 << channel) | (1 << (channel + 2)));  /* disable led & disable chop */
#else
    mask &= ~(1 << channel); /* disable led*/
#endif
    led_write_register(LED_EN_CTRL, mask);
    return HAL_ISINK_STATUS_OK;
}


hal_isink_status_t hal_isink_get_running_status(hal_isink_channel_t channel, hal_isink_running_status_t *running_status)
{
    return HAL_ISINK_STATUS_OK;
}


void hal_isink_dump_register()
{
    log_hal_msgid_info("=================Isink Dump(En_Ctrl %x, Debug %x) ==================", 2,
                       (unsigned int) led_read_register(LED_EN_CTRL), (unsigned int) led_read_register(0x0602)
                      );
    for (int i = 0; i < 2; i++) {
        log_hal_msgid_info("LED %d ==> LED_CON0:%x, LED0_UNIT:%x, PERIOD0:%x", 4,                   \
                           i,                                                                             \
                           (unsigned int)led_read_register(pmu_led_reg[i][PMU_LED_CON0]),                  \
                           (unsigned int)led_read_register(pmu_led_reg[i][PMU_LED_UNIT]),                  \
                           (unsigned int)led_read_register(pmu_led_reg[i][PMU_LED_PERIOD0])                \
                          );
        log_hal_msgid_info("           PERIOD1:%x, PWM0:%d, PWM1:%x ,PWM2:%x", 4,                    \
                           (unsigned int)led_read_register(pmu_led_reg[i][PMU_LED_PERIOD1]),     \
                           (unsigned int)led_read_register(pmu_led_reg[i][PMU_LED_PWM0]),        \
                           (unsigned int)led_read_register(pmu_led_reg[i][PMU_LED_PWM1]),        \
                           (unsigned int)led_read_register(pmu_led_reg[i][PMU_LED_PWM2])         \
                          );
    }
}

#endif



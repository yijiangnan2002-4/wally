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
#ifdef HAL_RTC_MODULE_ENABLED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hal_captouch.h"
#include "hal_resource_assignment.h"
#include "hal_pmu_ddie.h"
#include "hal_pmu.h"
#include "hal_clock_internal.h"
#include "hal_rtc_internal.h"

#if !(defined(__EXT_BOOTLOADER__) || defined(__EXT_DA__))
#include "ept_gpio_drv.h"
#include "ept_eint_drv.h"
#else
/* 0:RTC_GPIO2 1:Analog */
#define RTC_GPIO_PORT0_MODE         1
#define RTC_GPIO_PORT1_MODE         0
#define RTC_GPIO_PORT2_MODE         0
#define RTC_GPIO_PORT3_MODE         0

/* 0:INPUT 1:OUTPUT */
#define RTC_GPIO_PORT0_DIR          0
#define RTC_GPIO_PORT1_DIR          0
#define RTC_GPIO_PORT2_DIR          0
#define RTC_GPIO_PORT3_DIR          0

/* 0:low 1:high */
#define RTC_GPIO_PORT0_OUTPUT_LEVEL 0
#define RTC_GPIO_PORT1_OUTPUT_LEVEL 0
#define RTC_GPIO_PORT2_OUTPUT_LEVEL 0
#define RTC_GPIO_PORT3_OUTPUT_LEVEL 0

/* 0:disable 1:enable */
#define RTC_GPIO_PORT0_PU           0
#define RTC_GPIO_PORT1_PU           0
#define RTC_GPIO_PORT2_PU           0
#define RTC_GPIO_PORT3_PU           0

/* 0:disable 1:enable */
#define RTC_GPIO_PORT0_PD           1
#define RTC_GPIO_PORT1_PD           1
#define RTC_GPIO_PORT2_PD           1
#define RTC_GPIO_PORT3_PD           1
#endif
#ifndef RTC_GPIO_PORT3_MODE
#define RTC_GPIO_PORT3_MODE         0
#endif
#ifndef RTC_GPIO_PORT3_DIR
#define RTC_GPIO_PORT3_DIR          0
#endif
#ifndef RTC_GPIO_PORT3_OUTPUT_LEVEL
#define RTC_GPIO_PORT3_OUTPUT_LEVEL 0
#endif
#ifndef  RTC_GPIO_PORT3_PU
#define RTC_GPIO_PORT3_PU           0
#endif
#ifndef RTC_GPIO_PORT3_PD
#define RTC_GPIO_PORT3_PD           1
#endif

#define RTC_GPIO_DIGTAL             0
#define RTC_GPIO_INPUT              0

/*For reserve spare RG for save time offset when GPS enabled*/
#ifdef RTC_GPS_SUPPORTED
#define     RTC_DATA_RESV_FOR_GPS_SZ         (4)
#endif
/*Enable rtc init in bootload stage*/
//#define     RTC_BOOTLOADER_INIT_ENABLED

/******************************************************************************
 * Pre Processor Definitions
*******************************************************************************/

/*Use EPT config rtc gpio*/
#define     RTC_EPT_SUPPORTED

/*Enable lowpoer test for rtc: power down rtc-sram*/
//#define     RTC_LOWPOWER_TEST_ENABLED
#if defined(RTC_LOWPOWER_TEST_ENABLED) && defined(RTC_SRAM_SUPPORTED)
#define     RTC_SRAM_OFF_ENABLED
#endif

#define     __HAL_GET_RETURN_ADDRESS(temp) {     \
    __asm volatile("MOV %0, lr" : "=r"(temp));   \
}
#define     HAL_RTC_PRIVATE_DATA_ADDR   (&g_rtc_private_data)

#define     HAL_RTC_SET_F32K_FREQ_FOR_SHARE(temp)     *((volatile uint32_t *)HW_SYSRAM_PRIVATE_MEMORY_RTC_FREQ_START) = temp
/**********************************END*****************************************/

/***********************************************************************************************
 * Private type Declarations
***********************************************************************************************/
static  rtc_private_parameter_t     g_rtc_private_data = {0, 0, 0};

/***********************************************************************************************
 * Reserve Function for compatible
***********************************************************************************************/
//void             rtc_32k_off(void){}
//void             rtc_query_32kless_cali(){}
hal_rtc_status_t hal_rtc_gpio_init(hal_rtc_gpio_config_t *gpio_config);
hal_rtc_status_t hal_rtc_gpio_set_output(hal_rtc_gpio_t rtc_gpio, bool level);
#ifdef RTC_USING_EOSC_FOR_NORMAL_MODE
void             hal_rtc_start_period_meter_32k();
#endif
/***********************************************************************************************
 * Private Function Declarations
***********************************************************************************************/

static void     rtc_check_chip_config(rtc_private_parameter_t *config)
{
    if (config == NULL) {
        log_rtc_error("[hal][rtc] rtc_check_chip_config(): args is null\r\n", 0);
        return;
    }
    config->used_gpio = false;
    config->used_xosc = false;
#ifdef RTC_XOSC_SUPPORTED
    config->used_xosc = true;
#endif
}

static bool     rtc_check_time_is_valid(const hal_rtc_time_t *time)
{
    bool  ret = false;

    if (time != NULL) {
        if ((time->rtc_sec  <= 59) && (time->rtc_min <= 59)  &&  \
            (time->rtc_hour <= 23) && (time->rtc_year <= 127) &&  \
            (time->rtc_day  >= 1)  && (time->rtc_day <= 31)  &&  \
            (time->rtc_mon  >= 1)  && (time->rtc_mon <= 12)      \
           ) {
            ret =  true;
        }
    }
    return ret;
}

static void     rtc_init_eint_setting()
{
#ifdef RTC_EINT_SUPPORTED
    hal_rtc_eint_config_t rtc_eint_config;

    rtc_eint_config.is_falling_edge_active  = false;
    rtc_eint_config.is_enable_debounce      = true;
    rtc_eint_config.is_enable_wakeup        = true;
    rtc_eint_config.rtc_gpio                = HAL_RTC_GPIO_0;
#ifdef RTC_CAPTOUCH_SUPPORTED
    rtc_eint_config.is_enable_rtc_eint      = false;
    rtc_internal_eint_setting(&rtc_eint_config);
    log_rtc_info("[hal][rtc] rtc captouch enabled\r\n", 0);
#else
    rtc_eint_config.is_enable_rtc_eint      = false;
    rtc_internal_eint_setting(&rtc_eint_config);
#endif
    rtc_eint_config.is_enable_rtc_eint      = false;
    rtc_eint_config.rtc_gpio                = HAL_RTC_GPIO_1;
    rtc_internal_eint_setting(&rtc_eint_config);
    rtc_eint_config.rtc_gpio                = HAL_RTC_GPIO_2;
    rtc_internal_eint_setting(&rtc_eint_config);
    rtc_eint_config.rtc_gpio                = HAL_RTC_GPIO_3;
    rtc_internal_eint_setting(&rtc_eint_config);
#endif
}

static void     rtc_eint_irq_callback(void *user_data)
{
    uint32_t value;
    rtc_private_parameter_t *config = HAL_RTC_PRIVATE_DATA_ADDR;

    (void)user_data;

    hal_eint_mask(HAL_EINT_RTC);
    /* read clear interrupt status */
    value = rtc_internal_ioctrl(RTC_IOCTL_GET_IRQ_STATUS, 0, 0);

    /* clear rtc wakeup status*/
    rtc_internal_ioctrl(RTC_IOCTL_CLEAR_PWR_STATE, 0, 0);

    /*process alarm event*/
    if ((value & RTC_IRQ_STA_ALARM_MASK) != 0) {
        hal_rtc_disable_alarm();
        if (config->alarm_callback != NULL) {
            config->alarm_callback(config->alarm_userdata);
        }
    }
    /*process tick event*/
    if ((value & RTC_IRQ_STA_TICK_MASK) != 0) {
        if (config->tick_callback != NULL) {
            config->tick_callback(config->tick_userdata);
        }
    }
    /*process eint event*/
    if ((value & RTC_IRQ_STA_EINT0_MASK) != 0) {
        if (config->eint_callback[0] != NULL) {
            config->eint_callback[0](config->eint_userdata[0]);
        }
    }
    if ((value & RTC_IRQ_STA_EINT1_MASK) != 0) {
        if (config->eint_callback[1] != NULL) {
            config->eint_callback[1](config->eint_userdata[1]);
        }
    }
    if ((value & RTC_IRQ_STA_EINT2_MASK) != 0) {
        if (config->eint_callback[2] != NULL) {
            config->eint_callback[2](config->eint_userdata[2]);
        }
    }
    if ((value & RTC_IRQ_STA_EINT3_MASK) != 0) {
        if (config->eint_callback[3] != NULL) {
            config->eint_callback[3](config->eint_userdata[3]);
        }
    }
    log_rtc_info("rtc irq triggered->status 0x%x(bit 0:alm 1:tick 2:lp 3~6:eint)!!", 1, value);
    hal_eint_unmask(HAL_EINT_RTC);
}

#ifdef RTC_NVIC_SUPPORTED
static void     rtc_nvic_irq_callback(hal_nvic_irq_t irq_num)
{
    hal_nvic_clear_pending_irq(RTC_IRQn);
    rtc_eint_irq_callback(NULL);
}
#endif

static void     rtc_init_irq_setting(void)
{
#ifdef RTC_EINT_SUPPORTED
    hal_eint_config_t       eint_config;
    hal_eint_status_t       result;

    eint_config.trigger_mode  = HAL_EINT_LEVEL_LOW;
    eint_config.debounce_time = 0;
    result = hal_eint_init(HAL_EINT_RTC, &eint_config);
    if (result != HAL_EINT_STATUS_OK) {
        log_rtc_error("[hal][rtc] hal_eint_init fail: %d\r\n", 1, result);
        return;
    }
    hal_eint_register_callback(HAL_EINT_RTC, rtc_eint_irq_callback, NULL);
    result = hal_eint_unmask(HAL_EINT_RTC);
    if (result != HAL_EINT_STATUS_OK) {
        log_rtc_error("[hal][rtc] hal_eint_unmask fail: %d\r\n", 1, result);
    }
    hal_nvic_enable_irq(EINT_IRQn);
    log_rtc_info("[hal][rtc] rtc init irq as EINT type\r\n", 1, result);
#else
#ifdef RTC_NVIC_SUPPORTED
    hal_nvic_disable_irq(RTC_IRQn);
    hal_nvic_register_isr_handler(RTC_IRQn, rtc_nvic_irq_callback);
    hal_nvic_enable_irq(RTC_IRQn);
    log_rtc_info("[hal][rtc] rtc init irq as NVIC type\r\n", 1, result);
#endif
#endif
}

static void     rtc_init_gpio_setting()
{
#ifdef RTC_GPIO_SUPPORTED
#ifdef RTC_EPT_SUPPORTED
    uint8_t   ept_mode[] = {RTC_GPIO_PORT0_MODE, RTC_GPIO_PORT1_MODE, RTC_GPIO_PORT2_MODE, RTC_GPIO_PORT3_MODE};
    uint8_t   ept_dir[]  = {RTC_GPIO_PORT0_DIR,  RTC_GPIO_PORT1_DIR,  RTC_GPIO_PORT2_DIR,  RTC_GPIO_PORT3_DIR};
    uint8_t   ept_pd[]   = {RTC_GPIO_PORT0_PD,   RTC_GPIO_PORT1_PD,   RTC_GPIO_PORT2_PD,   RTC_GPIO_PORT3_PD};
    uint8_t   ept_pu[]   = {RTC_GPIO_PORT0_PU,   RTC_GPIO_PORT1_PU,   RTC_GPIO_PORT2_PU,   RTC_GPIO_PORT3_PU};
    uint8_t   ept_level[] = {RTC_GPIO_PORT0_OUTPUT_LEVEL,  RTC_GPIO_PORT1_OUTPUT_LEVEL, \
                             RTC_GPIO_PORT2_OUTPUT_LEVEL,  RTC_GPIO_PORT3_OUTPUT_LEVEL
                            };
    uint8_t   i    = 0;
    hal_rtc_gpio_config_t   io_cfg;

    log_rtc_info("[hal][rtc] rtc init gpio as ept config", 0);
    for (i = 0; i < HAL_RTC_GPIO_MAX; i++) {
        io_cfg.rtc_gpio     = i;
        io_cfg.is_analog = (ept_mode[i] == 2) ? true : false;
        io_cfg.is_input  = (ept_dir[i]  == RTC_GPIO_INPUT) ? 1 : 0;
        io_cfg.is_pull_down = (ept_pd[i] == 0) ? 0 : 1;
        io_cfg.is_pull_up   = (ept_pu[i] == 0) ? 0 : 1;

        hal_rtc_gpio_init(&io_cfg);
        //if rtc gpio is digtal & output, then set rtc gpio output level
        if (io_cfg.is_analog == false && io_cfg.is_input == false) {
            bool level = ept_level[i];
            hal_rtc_gpio_set_output(i, level);
        }
    }
#else
    hal_rtc_gpio_config_t   io_cfg;
    uint32_t  mask = 0;

    io_cfg.is_analog    = false;
    io_cfg.is_input     = true;
    io_cfg.is_pull_down = true;
    io_cfg.is_pull_up   = false;
    log_rtc_info("[hal][rtc] rtc init gpio as default", 0);

    io_cfg.rtc_gpio     = HAL_RTC_GPIO_1;
    hal_rtc_gpio_init(&io_cfg);
    io_cfg.rtc_gpio     = HAL_RTC_GPIO_2;
    hal_rtc_gpio_init(&io_cfg);
    io_cfg.rtc_gpio     = HAL_RTC_GPIO_3;

    /* if captouch enabled, then set rtc gpio0 for captpuch*/
    io_cfg.rtc_gpio     = HAL_RTC_GPIO_0;
#ifdef RTC_CAPTOUCH_SUPPORTED
    io_cfg.is_analog    = true;
    io_cfg.is_input     = true;
    io_cfg.is_pull_down = true;
    hal_rtc_gpio_init(&io_cfg);
#else
    hal_rtc_gpio_init(&io_cfg);
#endif /* RTC_CAPTOUCH_SUPPORTED */

#endif /* RTC_EPT_SUPPORTED */
#endif /* RTC_GPIO_SUPPORTED */
}

static void     rtc_setup_lowpower_mode()
{
    rtc_private_parameter_t *config = HAL_RTC_PRIVATE_DATA_ADDR;

    /*switch clock source from DCXO to EOSC or XOSC*/
    if (config->used_xosc) {
        rtc_internal_set_osc32_mode(HAL_RTC_OSC32K_XOSC_MODE); /*switch to xosc*/
        rtc_internal_ioctrl(RTC_IOCTL_SET_TIME_CALI, RTC_TIME_CALI_NORMAL, config->cali_time);
    } else {
        /*set cali for eosc*/
        rtc_internal_ioctrl(RTC_IOCTL_SET_EOSC_FREQ_CALI, config->cali_eosc, 0);
        rtc_internal_ioctrl(RTC_IOCTL_SET_TIME_CALI, RTC_TIME_CALI_K_EOSC, config->cali_time);
        rtc_internal_set_osc32_mode(HAL_RTC_OSC32K_EOSC_MODE); /*switch to eosc*/
    }
    rtc_internal_ioctrl(RTC_IOCTL_SET_SPAR_REG, RTC_SPAR_REG_BROM_SKIP_MASK, 0);
    rtc_internal_ioctrl(RTC_IOCTL_CLEAR_WAKEUP_STATUS, 0, 0);

#ifdef HAL_CAPTOUCH_MODULE_ENABLED
    hal_captouch_lowpower_control(HAL_CAPTOUCH_MODE_LOWPOWER);
#endif
    //rtc_internal_set_ldo_mode(true);
}

/********************************************************************
Function:   bl_rtc_func_init
Description:
    This function be called in bootloader, And is used to setup RTC SRAM
    setting in early stage. The purpose is make RTC SRAM avaiable for app
    initial. In app region init, startup.s
    will load .data area(include Zi/RW) from flash to RTC SRAM.

Input : None
Output: None
History: 2019-02-20 first create
*********************************************************************/
void    bl_rtc_func_init()
{
#ifdef RTC_BOOTLOADER_INIT_ENABLED
    uint16_t    temp    = 0;
    uint16_t    irq_sta = 0;

    rtc_internal_enable_setting(false, true);
    rtc_internal_ioctrl(RTC_IOCTL_CLEAR_PWR_STATE, 0, 0);
    rtc_internal_ioctrl(RTC_IOCTL_UNLOCK_PROTECT,  0, 0);
    rtc_internal_reload();

    temp    = rtc_internal_ioctrl(RTC_IOCTL_GET_SPAR_REG, 0, 0);
    irq_sta = rtc_internal_ioctrl(RTC_IOCTL_GET_IRQ_STATUS, 0, 0);
    if (rtc_internal_ioctrl(RTC_IOCTL_GET_PWRKEY_STATUS, 0, 0) == false) {
        rtc_internal_set_power_key();
        rtc_internal_init_register();
        irq_sta  |= RTC_SPAR_REG_PWR_FLG_MASK;
    }
    temp    &= ~(0xFF << RTC_SPAR_REG_RESRV_OFFSET);
    temp    |= irq_sta << RTC_SPAR_REG_RESRV_OFFSET;
    rtc_internal_ioctrl(RTC_IOCTL_SET_SPAR_REG, temp, 0);
    hal_gpt_delay_ms(1);
#ifdef RTC_SRAM_SUPPORTED
    hal_rtc_retention_sram_config(0xF, HAL_RTC_SRAM_NORMAL_MODE);
#endif
    rtc_internal_enable_setting(false, false);
#endif
}

/***********************************************************************************************
 * Public Function Declarations
***********************************************************************************************/
/********************************************************************
Function:   hal_rtc_init
Description:
    This function should be call everytime when power on.
    Important Notice:if RTC powerkey is correct, the function
    will not do initial flow, only switch clock to assigned
    mode(eosc or xosc or dcxo)
Input:
        None
Output:
        Always 0.
History:
        2019-02-20 first create
*********************************************************************/

hal_rtc_status_t hal_rtc_init(void)
{
    rtc_private_parameter_t *config = HAL_RTC_PRIVATE_DATA_ADDR;
    uint32_t                 temp = 0;

    log_rtc_info("#################  rtc initialize started ################# V1.1", 0);
    memset(config, 0, sizeof(rtc_private_parameter_t));
    rtc_check_chip_config(config);

    /*normal initial flow*/
    rtc_internal_enable_setting(false, true);
    rtc_internal_ioctrl(RTC_IOCTL_UNLOCK_PROTECT,  0, 0);
    rtc_internal_reload();

    /*check rtc power lost state and set state*/
    if (rtc_internal_ioctrl(RTC_IOCTL_GET_SPAR_REG, 0, 0) & RTC_SPAR_REG_BROM_SKIP_MASK) {
        config->op_state |= RTC_STATE_RTC_MODE_FLG; /* system back from sw rtc mode */
    }
#ifdef RTC_BOOTLOADER_INIT_ENABLED
    temp = rtc_internal_ioctrl(RTC_IOCTL_GET_SPAR_REG, 0, 0);
    config->irq_status = (temp >> RTC_SPAR_REG_RESRV_OFFSET);

    if (config->irq_status & RTC_SPAR_REG_PWR_FLG_MASK) {
        config->op_state |= RTC_STATE_INIT_BY_BL_FLG;
        config->op_state |= RTC_STATE_POWER_LOST_FLG;
    }
#else
    config->irq_status = rtc_internal_ioctrl(RTC_IOCTL_GET_IRQ_STATUS, 0, 0);
#endif
    if (rtc_internal_ioctrl(RTC_IOCTL_GET_PWRKEY_STATUS, 0, 0) == false) {
        rtc_internal_set_power_key();
        config->op_state    = RTC_STATE_POWER_LOST_FLG;
        config->irq_status |= RTC_SPAR_REG_PWR_FLG_MASK;
    }
    rtc_internal_ioctrl(RTC_IOCTL_CLEAR_PWR_STATE, 0, 0);
    temp = (config->irq_status << RTC_SPAR_REG_RESRV_OFFSET);
#ifdef RTC_SRAM_SUPPORTED
    /*default set rtc-sram to normal mode*/
    hal_rtc_retention_sram_config(0xF, HAL_RTC_SRAM_NORMAL_MODE);
    config->sram_mode   = HAL_RTC_SRAM_NORMAL_MODE;
#ifdef RTC_SRAM_OFF_ENABLED
    hal_rtc_retention_sram_config(0xF, HAL_RTC_SRAM_PD_MODE);
    config->sram_mode = HAL_RTC_SRAM_PD_MODE;
#endif
#endif
    /*print system power on reason*/
    if (config->op_state & RTC_STATE_POWER_LOST_FLG) {
        bool boot_flg = 0;
        if (config->op_state & RTC_STATE_INIT_BY_BL_FLG) {
        } else {
            rtc_internal_init_register();
            boot_flg = 1;
        }
        log_rtc_warn("rtc 1st power on, set default by %d(0:bl, 1:main)", 1, boot_flg);
        (void) boot_flg;
        rtc_internal_dump(NULL);
    } else {
        bool mode_flg = 0;
        if (rtc_internal_ioctrl(RTC_IOCTL_GET_PWRBYRTC_ST, 0, 0) == true) {
            config->op_state |= RTC_STATE_RTC_WAKE_FLG;
            rtc_internal_ioctrl(RTC_IOCTL_GET_PWRON_REASON, config->irq_status, 0);
            mode_flg = 1;
        }
        log_rtc_warn("rtc back from %d mode(0:unknow, 1:rtc)", 1, mode_flg);
        (void) mode_flg;
    }
    rtc_internal_ioctrl(RTC_IOCTL_SET_SPAR_REG, temp, 0);
    /*calc & set eosc cali*/
    config->cali_eosc = rtc_internal_get_eosc32_freq_calibration(RTC_CALI_TARGET_FREQUENCY);
    config->cali_time = rtc_internal_ioctrl(RTC_IOCTL_CALC_TIME_CALI, HAL_RTC_OSC32K_EOSC_MODE, 0);
    rtc_internal_ioctrl(RTC_IOCTL_SET_EOSC_FREQ_CALI, config->cali_eosc, 0);
    rtc_internal_ioctrl(RTC_IOCTL_SET_TIME_CALI, RTC_TIME_CALI_K_EOSC, config->cali_time);
#ifdef RTC_USING_EOSC_FOR_NORMAL_MODE
    log_rtc_warn("rtc use iner eosc-32k", 0);
    config->sel_clk = HAL_RTC_OSC32K_EOSC_MODE;
    rtc_internal_set_osc32_mode(HAL_RTC_OSC32K_EOSC_MODE);                /*switch to EOSC*/
#else
    if (config->used_xosc) {
        log_rtc_warn("rtc use ext xosc-32k", 0);
        config->sel_clk = HAL_RTC_OSC32K_XOSC_MODE;
        rtc_internal_ioctrl(RTC_IOCTL_SET_EOSC_FREQ_CALI, 0x8, 0);
        rtc_internal_set_osc32_mode(HAL_RTC_OSC32K_XOSC_MODE);          /*switch to XOSC*/
        rtc_internal_ioctrl(RTC_IOCTL_SET_TIME_CALI, RTC_TIME_CALI_NORMAL, 0);/*set time cali*/
    } else {
#ifdef HAL_DCXO_MODULE_ENABLED
        dcxo_32k_ctrl(true);
#else
#if (defined(__EXT_BOOTLOADER__) || defined(__EXT_DA__))
#else
        #error "RTC module need DCXO enable fisrt!!!!!"
#endif
#endif
        log_rtc_warn("rtc use iner dcxo-32k", 0);
        config->sel_clk = HAL_RTC_OSC32K_DCXO_MODE;
        rtc_internal_set_osc32_mode(HAL_RTC_OSC32K_DCXO_MODE);               /*switch to DCXO*/
        rtc_internal_ioctrl(RTC_IOCTL_SET_TIME_CALI, RTC_TIME_CALI_NORMAL, 0);/*set time cali*/
    }
#endif
    /*setup gpio & eint & irq & clear wakeup state*/
    rtc_init_gpio_setting();/*init rtc gpio*/
    rtc_init_eint_setting();/*init rtc eint*/
    rtc_internal_ioctrl(RTC_IOCTL_CLEAR_WAKEUP_STATUS, 0, 0);
    rtc_init_irq_setting(); /*init rtc interrupt*/
    rtc_internal_ioctrl(RTC_IOCTL_GET_IRQ_STATUS, 0, 0);

    rtc_internal_dump(NULL);
    rtc_internal_enable_setting(false, false);

    /* output initial done log */
    config->freq_eosc = hal_rtc_measure_32k_with_windows(HAL_RTC_OSC32K_EOSC_MODE, 99);
    config->freq_xosc = hal_rtc_measure_32k_with_windows(HAL_RTC_OSC32K_XOSC_MODE, 99);
    config->freq_f32k = hal_rtc_measure_32k_with_windows(HAL_RTC_OSC32K_FCLK_MODE, 99);
    HAL_RTC_SET_F32K_FREQ_FOR_SHARE(config->freq_f32k);
    log_rtc_info("RTC_CLK(%d),EOSC(%d),XOSC(%d), DCXO(%d)",  4,
                 (int) config->freq_f32k,
                 (int) config->freq_eosc,
                 (int) config->freq_xosc,
                 (int) hal_rtc_measure_32k_with_windows(HAL_RTC_OSC32K_DCXO_MODE, 99));
    hal_gpt_delay_ms(1);
    config->initialized = true;
#ifdef RTC_USING_EOSC_FOR_NORMAL_MODE
    hal_rtc_start_period_meter_32k();
#endif
    log_rtc_info("################# rtc initialized done #################", 0);
    return HAL_RTC_STATUS_OK;
}

/********************************************************************
Function: hal_rtc_deinit



Description:
Input: none
Output: none
History:
        2019-02-20 first create
*********************************************************************/
hal_rtc_status_t hal_rtc_deinit(void)
{
    return HAL_RTC_STATUS_OK;
}


/********************************************************************
Function:
        hal_rtc_enter_rtc_mode
Description:
        Make system enter into RTC Mode (another module will be power off,
        only rtc alive)
Input:
        None
Output:
        Always 0.
History:
        2019-02-20 first create
*********************************************************************/
hal_rtc_status_t hal_rtc_enter_rtc_mode(void)
{
    uint32_t    temp = 0;

    __HAL_GET_RETURN_ADDRESS(temp);
#ifdef RTC_SRAM_SUPPORTED
    rtc_private_parameter_t *config = HAL_RTC_PRIVATE_DATA_ADDR;

    /*config rtc-sram mode*/
#ifdef RTC_SRAM_OFF_ENABLED
    if (config->sram_mode == HAL_RTC_SRAM_SLEEP_MODE) {
        hal_rtc_retention_sram_config(0xF, HAL_RTC_SRAM_NORMAL_MODE);
    }
    hal_rtc_retention_sram_config(0xF, HAL_RTC_SRAM_PD_MODE);
    log_rtc_warn("rtc ---> in lp test", 0);
#else
    if (config->sram_mode == HAL_RTC_SRAM_NORMAL_MODE) {
        hal_rtc_retention_sram_config(0xF, HAL_RTC_SRAM_SLEEP_MODE);
    }
#endif
#endif
    log_rtc_warn("system will enter RTC mode, caller is 0x%x", 1, temp);
    rtc_internal_enable_setting(false, true);
    rtc_setup_lowpower_mode();
    rtc_internal_dump(NULL);
    rtc_internal_enable_setting(true, false);

    hal_gpt_delay_ms(10); /*wait log send done */
#ifdef HAL_PMU_MODULE_ENABLED
#if 0	// original
    pmu_power_off_sequence(1); /* call pmu api to turn off system*/
#else	// richard for customer UI spec
	if(pmu_get_chr_detect_value())
		pmu_power_off_sequence(1); 	/* call pmu api to turn off system*/
	else
		pmu_power_off_sequence(0); 	/* call pmu api to turn off system*/
#endif
    hal_gpt_delay_ms(2000);
    log_rtc_error("enter rtc mode failed, pmu power off system fail!!", 0);
#else
    log_rtc_error("enter rtc mode failed, pmu need enabled first!!", 0);
#endif
    rtc_internal_enable_setting(false, true);
    rtc_internal_dump(NULL);
    rtc_internal_enable_setting(false, false);
    return HAL_RTC_STATUS_OK;
}

/********************************************************************
Function:
        hal_rtc_back_from_rtcmode
Description:
        Make system enter into RTC Mode (another module will be power off,
        only rtc alive)
Input:
        None
Output: back from rtc mode(1:yes, 0: no)

History:
        2019-02-20 first create
*********************************************************************/
bool    hal_rtc_is_back_from_rtcmode()
{
    uint32_t  temp = 0;

    rtc_internal_enable_setting(false, true);
    temp = rtc_internal_ioctrl(RTC_IOCTL_GET_SPAR_REG, 0, 0);
    temp = temp >> 8;
    rtc_internal_enable_setting(false, false);
    return (temp & RTC_SPAR_REG_PWR_FLG_MASK) ? false : true;
}



int    hal_rtc_get_power_on_reason()
{
    int     temp = 0;
    hal_rtc_power_reason_t   reason;
    rtc_private_parameter_t *config = HAL_RTC_PRIVATE_DATA_ADDR;

    temp = rtc_internal_ioctrl(RTC_IOCTL_GET_SPAR_REG, 0, 0);
    temp = temp >> RTC_SPAR_REG_RESRV_OFFSET;

    if (config->op_state & RTC_STATE_POWER_LOST_FLG) {
        reason = RTC_POWERED_BY_1ST;
    } else if (config->op_state & RTC_STATE_RTC_WAKE_FLG) {
        reason = rtc_internal_ioctrl(RTC_IOCTL_GET_PWRON_REASON, temp, 0);
    } else {
        reason = RTC_POWERED_REASON_UNKNOW;
    }
    return reason;
}


/********************************************************************
Function:
        hal_rtc_retention_sram_config
Description:
        config sram mode.
Input:
Output:
History:
*********************************************************************/
#ifdef RTC_SRAM_SUPPORTED
hal_rtc_status_t hal_rtc_retention_sram_config(uint32_t mask, hal_rtc_sram_mode_t mode)
{
    rtc_private_parameter_t *config = HAL_RTC_PRIVATE_DATA_ADDR;
    uint32_t  i = 0;

    rtc_internal_enable_setting(false, true);
    /*config retention sram ro normal mode*/
    rtc_internal_sram_setting_cg(true, true);
    hal_gpt_delay_us(1);
    for (i = 0; i < HAL_RTC_SRAM_CELL_MAX; i++) {
        if (mask & (1 << i)) {
            rtc_internal_sram_setting(i, mode);
        }
    }
    rtc_internal_sram_setting_cg(false, false);
    rtc_internal_enable_setting(false, false);
    config->sram_mask = mask;
    config->sram_mode = mode;
    return HAL_RTC_STATUS_OK;
}

void hal_rtc_exit_retention_mode(void)
{
    uint32_t   i = 0;
    //uint32_t   mask = 0;
    //rtc_private_parameter_t *config = HAL_RTC_PRIVATE_DATA_ADDR;

    rtc_internal_enable_setting(false, true);
    rtc_internal_sram_setting_cg(false, true);
    hal_gpt_delay_us(1);
    for (i = 0; i < HAL_RTC_SRAM_CELL_MAX; i++) {
        rtc_internal_sram_setting(i, HAL_RTC_SRAM_NORMAL_MODE);//(hal_rtc_sram_mode_t)config->sram_mode);
    }
    rtc_internal_sram_setting_cg(false, false);
    rtc_internal_enable_setting(false, false);
}

hal_rtc_status_t hal_rtc_enter_retention_mode(void)
{
    uint32_t   mode = 0;
    //uint32_t   i=0, mask = 0;
    //rtc_private_parameter_t *config = HAL_RTC_PRIVATE_DATA_ADDR;

    //mask = config->sram_mask;
    mode = HAL_RTC_SRAM_NORMAL_MODE;

    rtc_internal_enable_setting(false, true);
    rtc_setup_lowpower_mode();
    if (mode == HAL_RTC_SRAM_NORMAL_MODE) {
        hal_rtc_retention_sram_config(0xF, HAL_RTC_SRAM_SLEEP_MODE);
    }
    rtc_internal_enable_setting(false, false);
#ifdef HAL_PMU_MODULE_ENABLED
    pmu_power_off_sequence(1);
#else
    log_rtc_error("enter retention mode failed,pmu need enabled first!!!\r\n", 0);
#endif
    hal_gpt_delay_ms(2000);
    hal_rtc_exit_retention_mode();
    return HAL_RTC_STATUS_OK;
}
#endif

/********************************************************************
Function:
        hal_rtc_set_time
Description:
        Set RTC time,and rtc will work base on this value.
Input:
        time: take care of this para, the year is type of uint8_t, so user should using
                this year value add leap year to get current years.
Output:
History:
        2019-02-20 first create
*********************************************************************/

hal_rtc_status_t hal_rtc_set_time(const hal_rtc_time_t *time)
{
    hal_rtc_status_t ret = HAL_RTC_STATUS_ERROR;

#ifdef RTC_GPS_SUPPORTED
    hal_rtc_time_t  cur_tm;
    int             tick_offset = 0;
    int             rtc_tick, usr_tick;
    uint32_t        i     = 0;
    uint8_t         temp  = 0;

    if (rtc_check_time_is_valid(time) == false) {
        return HAL_RTC_STATUS_INVALID_PARAM;
    }
    rtc_internal_enable_setting(false, true);
    rtc_internal_datetime_control(RTC_OP_GET, (hal_rtc_time_t *)&cur_tm, NULL);
    rtc_tick = rtc_datetime_to_tick(cur_tm.rtc_year, cur_tm.rtc_mon, cur_tm.rtc_day,
                                    cur_tm.rtc_hour, cur_tm.rtc_min, cur_tm.rtc_sec);
    usr_tick = rtc_datetime_to_tick(time->rtc_year, time->rtc_mon, time->rtc_day,
                                    time->rtc_hour, time->rtc_min, time->rtc_sec);
    tick_offset = usr_tick - rtc_tick;

    for (i = 0; i < 4; i++) {
        temp = ((tick_offset >> (8 * i)) & 0xFF);
        rtc_internal_set_retention_reg(i, temp);
    }
    rtc_internal_enable_setting(false, false);
    return HAL_RTC_STATUS_OK;
#else
    if (rtc_check_time_is_valid(time) == true) {
        rtc_internal_enable_setting(false, true);
        rtc_internal_datetime_control(RTC_OP_SET, NULL, (const hal_rtc_time_t *)time);
        rtc_internal_enable_setting(false, false);
        ret = HAL_RTC_STATUS_OK;
    } else {
        log_rtc_error("[hal][rtc] invalid argument\r\n", 0);
    }
#endif
    return ret;
}

/********************************************************************
Function:
        hal_rtc_get_time
Description:
        Get rtc time.
Input:
        time: take care of this para, the year is type of uint8_t, so user should using
                this year value add leap year to get current years.
Output:
History:
        2019-02-20 first create
*********************************************************************/

hal_rtc_status_t hal_rtc_get_time(hal_rtc_time_t *time)
{
#ifdef RTC_GPS_SUPPORTED
    hal_rtc_time_t  cur_tm;
    int         tick_offset = 0;
    int         rtc_tick    = 0, usr_tick = 0;
    uint32_t    i      = 0;
    uint8_t     *ucPtr = NULL;

    if (time == NULL) {
        return HAL_RTC_STATUS_INVALID_PARAM;
    }

    ucPtr = (uint8_t *)&tick_offset;
    for (i = 0; i < 4; i++) {
        rtc_internal_get_retention_reg(i, (uint8_t *)(&ucPtr[i]));
    }
    /*get current tm*/
    rtc_internal_datetime_control(RTC_OP_GET, &cur_tm, NULL);
    rtc_tick = rtc_internal_time_to_tick(cur_tm.rtc_year, cur_tm.rtc_mon, cur_tm.rtc_day,
                                         cur_tm.rtc_hour, cur_tm.rtc_min, cur_tm.rtc_sec);
    usr_tick = rtc_tick + tick_offset;
    rtc_internal_tick_to_time(usr_tick, time);

    return HAL_RTC_STATUS_OK;
#else
    if (time != NULL) {
        rtc_internal_datetime_control(RTC_OP_GET, time, NULL);
        return HAL_RTC_STATUS_OK;
    }
    return HAL_RTC_STATUS_INVALID_PARAM;
#endif

}

/********************************************************************
Function:
        hal_rtc_set_alarm
Description:
        Get rtc time.
Input:
Output:
History:
        2019-02-20 first create
*********************************************************************/
hal_rtc_status_t hal_rtc_set_alarm(const hal_rtc_time_t *time)
{
#ifdef RTC_GPS_SUPPORTED
    int             ofs_tick = 0;
    int             alm_tick = 0, cur_tick = 0;
    hal_rtc_time_t  cur_tm;

    if (rtc_check_time_is_valid(time) == false) {
        return HAL_RTC_STATUS_INVALID_PARAM;
    }
    /*get user timebase tick*/
    hal_rtc_get_time(&cur_tm);
    cur_tick = rtc_internal_time_to_tick(cur_tm.rtc_year, cur_tm.rtc_mon, cur_tm.rtc_day,
                                         cur_tm.rtc_hour, cur_tm.rtc_min, cur_tm.rtc_sec);
    alm_tick = rtc_internal_time_to_tick(time->rtc_year, time->rtc_mon, time->rtc_day,
                                         time->rtc_hour, time->rtc_min, time->rtc_sec);

    /*calc tick offset*/
    ofs_tick = alm_tick - cur_tick;
    if (ofs_tick > 0) {
        hal_rtc_set_alarm_by_second(ofs_tick);
        return HAL_RTC_STATUS_OK;
    } else {
        return HAL_RTC_STATUS_INVALID_PARAM;
    }

#else
    bool     enable = false;
    uint32_t mask = (1 << RTC_AL_MASK_DOW_OFFSET);

    if (rtc_check_time_is_valid(time) == false) {
        return HAL_RTC_STATUS_INVALID_PARAM;
    }
    rtc_internal_enable_setting(false, true);
    rtc_internal_alarm_control(RTC_OP_SET, NULL, time, &enable, &mask);
    rtc_internal_enable_setting(false, false);
    return HAL_RTC_STATUS_OK;
#endif
}

/********************************************************************
Function:
        hal_rtc_set_alarm_by_second
Description:
        set rtc alarm time by second
Input:
Output:
History:
        2019-02-20 first create
*********************************************************************/
hal_rtc_status_t hal_rtc_set_alarm_by_second(uint32_t second)
{
    int             alm_tick = 0;
    hal_rtc_time_t  cur_tm;
    hal_rtc_time_t *time;
    bool        enable = false;
    uint32_t    mask   = (1 << RTC_AL_MASK_DOW_OFFSET);

    rtc_internal_enable_setting(false, true);

    memset(&cur_tm, 0, sizeof(cur_tm));
    rtc_internal_datetime_control(RTC_OP_GET, &cur_tm, NULL);
    time = &cur_tm;

    //printf("get cur time:%d-%d-%d %d:%d:%d", time->rtc_year, time->rtc_mon, time->rtc_day, time->rtc_hour, time->rtc_min, time->rtc_sec);
    alm_tick = rtc_internal_time_to_tick(time->rtc_year, time->rtc_mon, time->rtc_day,
                                         time->rtc_hour, time->rtc_min, time->rtc_sec);

    //printf("cur time tick:%d, alarm tick:%d", alm_tick, (alm_tick + second));
    alm_tick += second;
    rtc_internal_tick_to_time(alm_tick, &cur_tm);
    rtc_internal_alarm_control(RTC_OP_SET, NULL,  time, &enable, &mask);

    //printf("set alm time:%d-%d-%d %d:%d:%d", time->rtc_year, time->rtc_mon, time->rtc_day, time->rtc_hour, time->rtc_min, time->rtc_sec);
    rtc_internal_enable_setting(false, false);
    return HAL_RTC_STATUS_OK;
}

/********************************************************************
Function:
        hal_rtc_get_alarm
Description:
        Get rtc time.
Input:
Output:
History:
        2019-02-20 first create
*********************************************************************/
hal_rtc_status_t hal_rtc_get_alarm(hal_rtc_time_t *time)
{
#ifdef RTC_GPS_SUPPORTED
    hal_rtc_time_t  alm_tm;
    hal_rtc_time_t  cur_tm;
    int         ofs_tick = 0;
    int         alm_tick = 0, cur_tick;

    if (time == NULL) {
        return HAL_RTC_STATUS_ERROR;
    }
    rtc_internal_enable_setting(false, true);
    rtc_internal_alarm_control(RTC_OP_GET, time, NULL, NULL, NULL);
    rtc_internal_datetime_control(RTC_OP_GET, &cur_tm, NULL);

    cur_tick = rtc_internal_time_to_tick(cur_tm.rtc_year, cur_tm.rtc_mon, cur_tm.rtc_day, \
                                         cur_tm.rtc_hour, cur_tm.rtc_min, cur_tm.rtc_sec);
    alm_tick = rtc_internal_time_to_tick(alm_tm.rtc_year, alm_tm.rtc_mon, alm_tm.rtc_day, \
                                         alm_tm.rtc_hour, alm_tm.rtc_min, alm_tm.rtc_sec);
    ofs_tick = alm_tick - cur_tick;
    rtc_internal_enable_setting(false, false);

    hal_rtc_get_time(&cur_tm);
    cur_tick = rtc_internal_time_to_tick(cur_tm.rtc_year, cur_tm.rtc_mon, cur_tm.rtc_day, \
                                         cur_tm.rtc_hour, cur_tm.rtc_min, cur_tm.rtc_sec);
    alm_tick = ofs_tick + cur_tick;
    rtc_internal_tick_to_time(alm_tick, time);
    return HAL_RTC_STATUS_OK;
#else
    if (time != NULL) {
        rtc_internal_enable_setting(false, true);
        rtc_internal_alarm_control(RTC_OP_GET, time, NULL, NULL, NULL);
        rtc_internal_enable_setting(false, false);
        return HAL_RTC_STATUS_OK;
    } else {
        return HAL_RTC_STATUS_INVALID_PARAM;
    }
#endif
}

/********************************************************************
Function:
        hal_rtc_enable_alarm
Description:
        Get rtc time.
Input:
Output:
History:
        2019-02-20 first create
*********************************************************************/
hal_rtc_status_t hal_rtc_enable_alarm(void)
{
    bool        enable = true;
    uint32_t    mask   = (1 << RTC_AL_MASK_DOW_OFFSET);

    rtc_internal_enable_setting(false, true);
    rtc_internal_alarm_control(RTC_OP_SET, NULL, NULL, &enable, &mask);
    rtc_internal_enable_setting(false, false);
    return HAL_RTC_STATUS_OK;
}

/********************************************************************
Function:
        hal_rtc_disable_alarm
Description:
        Get rtc time.
Input:
Output:
History:
        2019-02-20 first create
*********************************************************************/
hal_rtc_status_t hal_rtc_disable_alarm(void)
{
    bool        enable = false;
    uint32_t    mask   = (1 << RTC_AL_MASK_DOW_OFFSET);

    rtc_internal_enable_setting(false, true);
    rtc_internal_alarm_control(RTC_OP_SET, NULL, NULL, &enable, &mask);
    rtc_internal_enable_setting(false, false);
    return HAL_RTC_STATUS_OK;
}
/********************************************************************
Function:
        hal_rtc_set_time_notification_period
Description:
        Get rtc time.
Input:
Output:
History:
        2019-02-20 first create
*********************************************************************/

hal_rtc_status_t hal_rtc_set_time_notification_period(hal_rtc_time_notification_period_t period)
{
    bool    rslt   = false;
    bool    enable = true;

    if (period >= HAL_RTC_TIME_NOTIFICATION_MAX) {
        return HAL_RTC_STATUS_ERROR;
    }
    rtc_internal_enable_setting(false, true);
    rslt = rtc_internal_enable_tick_notify(period, enable);
    rtc_internal_enable_setting(false, false);
    return ((rslt == true) ? HAL_RTC_STATUS_OK : HAL_RTC_STATUS_ERROR);
}

/********************************************************************
Function:
        hal_rtc_set_time_callback
Description:
        Get rtc time.
Input:
Output:
History:
        2019-02-20 first create
*********************************************************************/
hal_rtc_status_t hal_rtc_set_time_callback(hal_rtc_time_callback_t callback_function, void *user_data)
{
    rtc_private_parameter_t *config = HAL_RTC_PRIVATE_DATA_ADDR;

    if (callback_function == NULL) {
        return HAL_RTC_STATUS_ERROR;
    }
    config->tick_callback = callback_function;
    config->tick_userdata = user_data;
    return HAL_RTC_STATUS_OK;
}

/********************************************************************
Function:
        hal_rtc_set_alarm_callback
Description:
        set alarm callback.
Input:
Output:
History:
        2019-02-20 first create
*********************************************************************/
hal_rtc_status_t hal_rtc_set_alarm_callback(const hal_rtc_alarm_callback_t callback_function, void *user_data)
{
    rtc_private_parameter_t *config = HAL_RTC_PRIVATE_DATA_ADDR;

    if (callback_function == NULL) {
        return HAL_RTC_STATUS_ERROR;
    }
    config->alarm_callback = callback_function;
    config->alarm_userdata = user_data;
    return HAL_RTC_STATUS_OK;
}

/********************************************************************
Function:
        hal_rtc_set_eint_callback
Description:
        Get rtc time.
Input:
Output:
History:
        2019-02-20 first create
*********************************************************************/

hal_rtc_status_t hal_rtc_eint_register_callback(hal_rtc_gpio_t eint_num, hal_rtc_eint_callback_t callback_function, void *user_data)
{
    rtc_private_parameter_t *config = HAL_RTC_PRIVATE_DATA_ADDR;

    if (callback_function == NULL || eint_num >= HAL_RTC_GPIO_MAX) {
        return HAL_RTC_STATUS_ERROR;
    }
    config->eint_callback[eint_num] = callback_function;
    config->eint_userdata[eint_num] = user_data;
    return HAL_RTC_STATUS_OK;
}

hal_rtc_status_t hal_rtc_set_eint_callback(hal_rtc_gpio_t eint_num, hal_rtc_eint_callback_t callback_function, void *user_data)
{
    return hal_rtc_eint_register_callback(eint_num, callback_function, user_data);
}


/********************************************************************
Function:
        hal_rtc_set_one_shot_calibration
Description:
        set oneshot calibration
Input:
Output:
History:
        2019-02-20 first create
*********************************************************************/
hal_rtc_status_t hal_rtc_set_one_shot_calibration(int16_t ticks)
{
    rtc_internal_enable_setting(false, true);
    rtc_internal_ioctrl(RTC_IOCTL_SET_ONETIME_CALI, ticks, 0);
    rtc_internal_enable_setting(false, false);
    return HAL_RTC_STATUS_OK;
}

/********************************************************************
Function:
        hal_rtc_get_one_shot_calibration
Description:
        get oneshot calibration
Input:
Output:
History:
        2019-02-20 first create
*********************************************************************/
hal_rtc_status_t hal_rtc_get_one_shot_calibration(int16_t *ticks)
{
    if (ticks == NULL) {
        return HAL_RTC_STATUS_ERROR;
    }
    rtc_internal_enable_setting(false, true);
    *ticks = rtc_internal_ioctrl(RTC_IOCTL_GET_ONETIME_CALI, 0, 0);
    rtc_internal_enable_setting(false, false);
    return HAL_RTC_STATUS_OK;
}

/********************************************************************
Function:
        hal_rtc_set_repeat_calibration
Description:
        set repeat caliration(for xosc)
Input:
Output:
History:
        2019-02-20 first create
*********************************************************************/
hal_rtc_status_t hal_rtc_set_repeat_calibration(int16_t ticks_per_8_seconds)
{
    rtc_internal_enable_setting(false, true);
    rtc_internal_ioctrl(RTC_IOCTL_SET_TIME_CALI, RTC_TIME_CALI_K_EOSC, ticks_per_8_seconds);
    rtc_internal_enable_setting(false, false);
    return HAL_RTC_STATUS_OK;
}

/********************************************************************
Function:
        hal_rtc_get_repeat_calibration
Description:
        get repeat caliration(for xosc)
Input:
Output:
History:
        2019-02-20 first create
*********************************************************************/
hal_rtc_status_t hal_rtc_get_repeat_calibration(int16_t *ticks_per_8_seconds)
{
    if (ticks_per_8_seconds == NULL) {
        return HAL_RTC_STATUS_ERROR;
    }
    rtc_internal_enable_setting(false, true);
    *ticks_per_8_seconds = (rtc_internal_ioctrl(RTC_IOCTL_GET_TIME_CALI, 0, 0) & RTC_CALI_VALUE_MASK);
    rtc_internal_enable_setting(false, false);
    return HAL_RTC_STATUS_OK;
}

/********************************************************************
Function:
        hal_rtc_set_data
Description:
        save data to rtc spare register
Input:
Output:
History:
        2019-02-20 first create
*********************************************************************/
hal_rtc_status_t hal_rtc_set_data(uint16_t offset, const char *buf, uint16_t len)
{
    //uint16_t            i = 0;
    hal_rtc_status_t    status = HAL_RTC_STATUS_OK;

    if (buf == NULL || len == 0 || (offset + len) > HAL_RTC_BACKUP_BYTE_NUM_MAX) {
        return HAL_RTC_STATUS_ERROR;
    }
#ifdef RTC_GPS_SUPPORTED
    offset = RTC_DATA_RESV_FOR_GPS_SZ + offset;
#endif
    rtc_internal_enable_setting(false, true);
    status = rtc_internal_set_retention_reg(offset, (uint8_t *)buf, len);
    rtc_internal_enable_setting(false, false);
    return status;
}

/********************************************************************
Function:
        hal_rtc_get_data
Description:
        get data from rtc spare register
Input:
Output:
History:
        2019-02-20 first create
*********************************************************************/
hal_rtc_status_t hal_rtc_get_data(uint16_t offset, char *buf, uint16_t len)
{
    //uint16_t            i = 0;
    hal_rtc_status_t    status = HAL_RTC_STATUS_OK;

    if (buf == NULL || len == 0 || (offset + len) > HAL_RTC_BACKUP_BYTE_NUM_MAX) {
        return HAL_RTC_STATUS_ERROR;
    }
#ifdef RTC_GPS_SUPPORTED
    offset = RTC_DATA_RESV_FOR_GPS_SZ + offset;
#endif
    rtc_internal_enable_setting(false, true);
    status = rtc_internal_get_retention_reg(offset, (uint8_t *)buf, len);
    rtc_internal_enable_setting(false, false);
    return status;
}

/********************************************************************
Function:
        hal_rtc_clear_data
Description:
        clear rtc spare register
Input:
Output:
History:
        2019-02-20 first create
*********************************************************************/
hal_rtc_status_t hal_rtc_clear_data(uint16_t offset, uint16_t len)
{
    //uint16_t            i = 0;
    hal_rtc_status_t    status = HAL_RTC_STATUS_OK;
    uint8_t             buf[HAL_RTC_BACKUP_BYTE_NUM_MAX];

#ifdef RTC_GPS_SUPPORTED
    offset = RTC_DATA_RESV_FOR_GPS_SZ + offset;
#endif
    if (len == 0 || (offset + len) > HAL_RTC_BACKUP_BYTE_NUM_MAX) {
        return HAL_RTC_STATUS_ERROR;
    }
    memset(buf, 0, sizeof(buf));
    rtc_internal_enable_setting(false, true);
    status = rtc_internal_set_retention_reg(offset, (uint8_t *)buf, len);
    rtc_internal_enable_setting(false, false);
    return status;
}

/********************************************************************
Function:
        hal_rtc_get_f32k_frequency
Description:
        get rtc-32k clock frequency
Input:
Output:
History:
        2019-02-20 first create
*********************************************************************/
hal_rtc_status_t hal_rtc_get_f32k_frequency(uint32_t *frequency)
{
    rtc_private_parameter_t *config = HAL_RTC_PRIVATE_DATA_ADDR;

    if(frequency != NULL){
        if(abs(config->freq_f32k- 32768) > 3000){
            config->freq_f32k = 32768; /* this only for bootloader */
        }
        *frequency = config->freq_f32k;
        return HAL_RTC_STATUS_OK;
    } else {
        return HAL_RTC_STATUS_ERROR;
    }
}


/********************************************************************
Function:
        hal_rtc_eint_init
Description:
        config rtc eint
Input:
Output:
History:
        2019-02-20 first create
*********************************************************************/
hal_rtc_status_t hal_rtc_eint_init(hal_rtc_eint_config_t *eint_config)
{
    if (eint_config == NULL) {
        return HAL_RTC_STATUS_ERROR;
    }
#ifdef RTC_EINT_SUPPORTED
    rtc_internal_enable_setting(false, true);
    rtc_internal_eint_setting(eint_config);
    rtc_internal_enable_setting(false, false);
#endif
    return HAL_RTC_STATUS_OK;
}

/********************************************************************
Function:
        hal_rtc_gpio_init
Description:
        config rtc gpio
Input:
Output:
History:
        2019-12-26  first create
*********************************************************************/

#ifdef RTC_GPIO_SUPPORTED
hal_rtc_status_t hal_rtc_gpio_init(hal_rtc_gpio_config_t *gpio_config)
{
    rtc_private_parameter_t *pinfo = &g_rtc_private_data;
    uint32_t                 tmp = 0;

    if (gpio_config == NULL || gpio_config->rtc_gpio >= HAL_RTC_GPIO_MAX) {
        return HAL_RTC_STATUS_INVALID_PARAM;
    }

    if (gpio_config->is_analog == false) {
        tmp |= RTC_GPIO_CFG_AD_MASK; //digtal
        if (gpio_config->is_input == false) {
            tmp |= RTC_GPIO_CFG_IO_MASK; //output
        }
    }
    if (gpio_config->is_pull_up == true) {
        tmp |= RTC_GPIO_CFG_PU_MASK;
    }
    if (gpio_config->is_pull_down == true) {
        tmp |= RTC_GPIO_CFG_PD_MASK;
    }
    pinfo->rtc_gpio_config[gpio_config->rtc_gpio] = tmp;
    pmu_set_debug_mode(0, 0, 0, 0);
    pmu_rtc_gpio_config(gpio_config->rtc_gpio + PMU_STRUP_RTC_GPIO0, tmp);
    //log_rtc_warn("[hal][rtc] hal_rtc_gpio_init: cfg 0x%x\r\n", 1, tmp);
    return HAL_RTC_STATUS_OK;
}

hal_rtc_status_t hal_rtc_gpio_set_output(hal_rtc_gpio_t rtc_gpio, bool level)
{
    rtc_private_parameter_t *pinfo = &g_rtc_private_data;
    hal_rtc_status_t         status = HAL_RTC_STATUS_OK;
    uint8_t                  tmp = 0;

    if (rtc_gpio >= HAL_RTC_GPIO_MAX) {
        return HAL_RTC_STATUS_INVALID_PARAM;
    }
    tmp = pinfo->rtc_gpio_config[rtc_gpio];
    if (level) {
        tmp |= RTC_GPIO_CFG_O_MASK;
    } else {
        tmp &= ~RTC_GPIO_CFG_O_MASK;
    }
    pmu_set_debug_mode(0, 0, 0, 0);
    pmu_rtc_gpio_config(rtc_gpio + PMU_STRUP_RTC_GPIO0, tmp);
    //log_rtc_warn("[hal][rtc] hal_rtc_gpio_set_output: cfg 0x%x\r\n", 1, tmp);
    return status;
}

hal_rtc_status_t hal_rtc_gpio_get_input(hal_rtc_gpio_t rtc_gpio, bool *level)
{
    if (rtc_gpio >= HAL_RTC_GPIO_MAX || level == NULL) {
        return HAL_RTC_STATUS_INVALID_PARAM;
    }
    if (pmu_get_rtc_gpio_status(rtc_gpio + PMU_STRUP_RTC_GPIO0)) {
        *level = 1;
    } else {
        *level = 0;
    }
    //log_rtc_warn("[hal][rtc] hal_rtc_gpio_get_input: get io %d\r\n", 1, *level);
    return HAL_RTC_STATUS_OK;
}
#endif




#ifdef RTC_CAPTOUCH_SUPPORTED

/********************************************************************
Function:
        hal_rtc_captouch_init
Description:
        initial captouch wake function.
Input:
Output:
History:
        2019-02-20 first create
*********************************************************************/
hal_rtc_status_t hal_rtc_captouch_init(void)
{
    rtc_internal_captouch_init();
    return HAL_RTC_STATUS_OK;
}

/********************************************************************
Function:
        hal_rtc_captouch_deinit
Description:
        de-initial captouch wake function.
Input:
Output:
History:
        2019-02-20 first create by gopal.yang
*********************************************************************/

hal_rtc_status_t hal_rtc_captouch_deinit(void)
{
    rtc_internal_captouch_deinit();
    return HAL_RTC_STATUS_OK;
}

hal_rtc_status_t hal_rtc_captouch_control(hal_rtc_captouch_cmd_t cmd, uint32_t option)
{
    switch (cmd) {
        case HAL_RTC_CAPTOUCH_SET_CAPCON: {
            rtc_internal_set_capcon(option);
        }
        break;
        case HAL_RTC_CAPTOUCH_CLR_WAKEUP_ST: {
            rtc_internal_ioctrl(RTC_IOCTL_CLEAR_WAKEUP_STATUS, 0, 0);
        }
        break;
        default:
            break;
    }
    return HAL_RTC_STATUS_OK;
}

#endif /* RTC_CAPTOUCH_SUPPORTED */


hal_rtc_status_t hal_rtc_switch_32k_source(hal_rtc_osc32k_mode_t mode)
{
#if (defined(__EXT_BOOTLOADER__) || defined(__EXT_DA__))
    return HAL_RTC_STATUS_ERROR;
#else
    rtc_private_parameter_t *config = HAL_RTC_PRIVATE_DATA_ADDR;

    switch (mode) {
        case HAL_RTC_OSC32K_EOSC_MODE: {
            rtc_internal_select_32k(mode);
            //rtc_internal_ioctrl(RTC_IOCTL_SET_EOSC_FREQ_CALI, config->cali_eosc, 0);
            rtc_internal_ioctrl(RTC_IOCTL_SET_TIME_CALI, RTC_TIME_CALI_K_EOSC, config->cali_time);/*set time cali*/
            config->freq_f32k = config->freq_eosc;
            config->sel_clk   = HAL_RTC_OSC32K_EOSC_MODE;
#ifdef HAL_DCXO_MODULE_ENABLED
            dcxo_32k_ctrl(false); /* dsiable dcxo-32k */
#endif
        }
        break;
        case HAL_RTC_OSC32K_DCXO_MODE: {
#ifdef HAL_DCXO_MODULE_ENABLED
            dcxo_32k_ctrl(true); /* enable dcxo-32k */
#endif
            rtc_internal_select_32k(mode);
            rtc_internal_ioctrl(RTC_IOCTL_SET_TIME_CALI, RTC_TIME_CALI_NORMAL, 0);/*set time cali*/
            config->freq_f32k = 32768;
            config->sel_clk   = HAL_RTC_OSC32K_DCXO_MODE;
        }
        break;
        case HAL_RTC_OSC32K_XOSC_MODE: {
            rtc_internal_select_32k(mode);                /*switch to XOSC*/
            rtc_internal_ioctrl(RTC_IOCTL_SET_EOSC_FREQ_CALI, 0x8, 0);
            rtc_internal_ioctrl(RTC_IOCTL_SET_TIME_CALI, RTC_TIME_CALI_NORMAL, 0);/*set time cali*/
            config->freq_f32k = config->freq_xosc;
            config->sel_clk   = HAL_RTC_OSC32K_XOSC_MODE;
#ifdef HAL_CLOCK_MODULE_ENABLED
            DCXO_PCON_REG.DCXO32K_EN = 0x0; /* disable dcxo-32k */
#endif
        }
        break;
    }
#if 0
    log_rtc_info("[hal][rtc]RTC_CLK(%d),EOSC(%d),XOSC(%d), DCXO(%d)",  4,
                 (int) hal_rtc_measure_32k_with_windows(HAL_RTC_OSC32K_FCLK_MODE, 99),
                 (int) hal_rtc_measure_32k_with_windows(HAL_RTC_OSC32K_EOSC_MODE, 99),
                 (int) hal_rtc_measure_32k_with_windows(HAL_RTC_OSC32K_XOSC_MODE, 99),
                 (int) hal_rtc_measure_32k_with_windows(HAL_RTC_OSC32K_DCXO_MODE, 99));
#endif
    config->ref_cnt_sw_clk++;

    /* set 32k freq for dsp using */
    HAL_RTC_SET_F32K_FREQ_FOR_SHARE(config->freq_f32k);
    if (rtc_internal_ioctrl(RTC_IOCTL_GET_32K_SEL, 0, 0) == mode) {
        return HAL_RTC_STATUS_OK;
    } else {
        return HAL_RTC_STATUS_ERROR;
    }
#endif /*__EXT_BOOTLOADER__*/
}


/***********************************************************************************************
 * Special API For 32k Drift Issue
***********************************************************************************************/

/*****************************************************************************
* Description:
* Becuase of EOSC-32k will drift when temperature is change, and this will lead
* led not sync of left and right earbuds.
* before we used the clock meter to meter the eosc32k, but it have risk. Some
* other user may using this clock meter hw within dvfs api, this may lead a
* race condition of multi-user access the same hw.
* For resolve this issue, the rtc will using sw function to meter the 32k clock.
*******************************************************************************/
#ifdef RTC_USING_EOSC_FOR_NORMAL_MODE

/* HAL_RTC_METER_PERIOD_TIME_MS should bigger than HAL_RTC_METER_WINDOWN_MS */
#define HAL_RTC_METER_PERIOD_TIME_MS    60000
#define HAL_RTC_METER_WINDOWN_MS        1000


enum {
    HAL_RTC_METER_START = 0,
    HAL_RTC_METER_BEGIN,
    HAL_RTC_METER_COMPLETE,
};

void    hal_rtc_gpt_meter_handle(void *user_data)
{
    hal_rtc_32k_merter_info_t *config = (hal_rtc_32k_merter_info_t *) user_data;
    uint32_t mask;
    hal_gpt_status_t gpt_stat = 0;
    rtc_private_parameter_t   *priv_info = HAL_RTC_PRIVATE_DATA_ADDR;
    static uint8_t  pre_ref_cnt = 0;

    switch (config->meter_state) {
        case HAL_RTC_METER_START: {
                gpt_stat = hal_gpt_sw_start_timer_ms(config->gpt_handle, HAL_RTC_METER_PERIOD_TIME_MS - HAL_RTC_METER_WINDOWN_MS, hal_rtc_gpt_meter_handle, user_data);
                if(gpt_stat != 0){
                    log_rtc_info("[hal][rtc]hal_rtc_gpt_meter_handle:start sw gpt fail:%d", 1, gpt_stat);
                }
                config->meter_state = HAL_RTC_METER_BEGIN;
            }break;
        case HAL_RTC_METER_BEGIN: {
                hal_nvic_save_and_set_interrupt_mask(&mask);
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M,  &config->clk_1m_cnt[0]);
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &config->clk_32k_cnt[0]);
                pre_ref_cnt = priv_info->ref_cnt_sw_clk;
                hal_nvic_restore_interrupt_mask(mask);

                gpt_stat = hal_gpt_sw_start_timer_us(config->gpt_handle, HAL_RTC_METER_WINDOWN_MS * 1000, hal_rtc_gpt_meter_handle, user_data);
                if(gpt_stat != 0){
                    log_rtc_info("[hal][rtc]hal_rtc_gpt_meter_handle:start sw gpt fail:%d", 1, gpt_stat);
                }
                config->meter_state = HAL_RTC_METER_COMPLETE;
            }break;
        case HAL_RTC_METER_COMPLETE: {
                hal_nvic_save_and_set_interrupt_mask(&mask);
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M,  &config->clk_1m_cnt[1]);
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &config->clk_32k_cnt[1]);
                hal_nvic_restore_interrupt_mask(mask);
                if (priv_info->sel_clk == HAL_RTC_OSC32K_EOSC_MODE) {
                    if (pre_ref_cnt == priv_info->ref_cnt_sw_clk) {
                        uint32_t cnt_1m,cnt_32k;

                        hal_gpt_get_duration_count(config->clk_1m_cnt[0],  config->clk_1m_cnt[1],  &cnt_1m);
                        hal_gpt_get_duration_count(config->clk_32k_cnt[0], config->clk_32k_cnt[1], &cnt_32k);
                        priv_info->freq_eosc = (1000000.0f * cnt_32k) / cnt_1m;
                    }
                    priv_info->freq_f32k = priv_info->freq_eosc;
                } else {
                    priv_info->freq_xosc = 32768;
                    priv_info->freq_f32k = 32768;
                }
                log_rtc_info("[hal][rtc]hal_rtc_gpt_meter_handle: clk src %d, clk freq %d,", 2, priv_info->sel_clk, priv_info->freq_f32k);
                config->meter_state = HAL_RTC_METER_START;
                hal_rtc_gpt_meter_handle(user_data);
            }break;
        default: {
            config->meter_state = HAL_RTC_METER_START;
        } break;
    }
}


void hal_rtc_start_period_meter_32k()
{
    rtc_private_parameter_t   *priv_info = HAL_RTC_PRIVATE_DATA_ADDR;
    hal_gpt_status_t gpt_stat = 0;

    gpt_stat = hal_gpt_sw_get_timer(&priv_info->meter_32k_handle.gpt_handle);
    if(gpt_stat != 0){
        log_rtc_error("[hal][rtc]hal_rtc_start_period_meter_32k:get sw timer fail:%d", 1, gpt_stat);
        return;
    }
    priv_info->meter_32k_handle.meter_state = 0;
    hal_rtc_gpt_meter_handle(&priv_info->meter_32k_handle);
}
#endif /* RTC_USING_EOSC_FOR_NORMAL_MODE */

#endif /*HAL_RTC_MODULE_ENABLED*/



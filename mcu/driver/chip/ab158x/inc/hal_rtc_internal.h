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

#ifndef _HAL_RTC_INTERNAL_H_
#define _HAL_RTC_INTERNAL_H_
#include "hal.h"
#include "hal_platform.h"
#ifdef  HAL_RTC_MODULE_ENABLED
#ifdef __cplusplus
extern "C" {
#endif
#include "hal_rtc.h"
#include "assert.h"

#define     RTC_CALI_TARGET_FREQUENCY   32768
#define     HAL_RTC_FEATURE_32K_TO_GPIO

/*for user define debug log level*/
//#define RTC_PLAIN_LOG_ENABLE

/* 32k source select */
#ifdef AIR_SYS32K_CLOCK_SOURCE_DCXO
    #define     RTC_USING_DCXO_FOR_NORMAL_MODE
#else
    #define     RTC_USING_EOSC_FOR_NORMAL_MODE
#endif


#if defined(HAL_RTC_FEATURE_EINT) && defined(HAL_EINT_MODULE_ENABLED)/*chip support eint?*/
    #define     RTC_EINT_SUPPORTED
#endif
#ifdef HAL_NVIC_MODULE_ENABLED          /*chip nvid enable?*/
    //#define     RTC_NVIC_SUPPORTED
#endif
#ifdef HAL_RTC_FEATURE_GPS_TIME         /*chip supportt gps time?*/
    #define     RTC_GPS_SUPPORTED
#endif
#if defined(HAL_RTC_FEATURE_GPIO) && defined(HAL_PMU_MODULE_ENABLED)             /*chip supportt GPIO?*/
    #define     RTC_GPIO_SUPPORTED
#endif
#ifdef HAL_RTC_FEATURE_RETENTION_SRAM   /*chip supportt retention SRAM?*/
    //#define     RTC_SRAM_SUPPORTED
#endif

/*chip is support captouch?*/
#if defined(HAL_RTC_FEATURE_CAPTOUCH) && defined(HAL_CAPTOUCH_MODULE_ENABLED)
    #define     RTC_CAPTOUCH_SUPPORTED
#endif
#ifdef MTK_HAL_EXT_32K_ENABLE           /*chip is support ext-32k?*/
    //#define     RTC_XOSC_SUPPORTED
#endif

#ifdef RTC_PLAIN_LOG_ENABLE
    #define log_rtc_info(_message, cnt, ...)    printf(_message, ##__VA_ARGS__)
    #define log_rtc_warn(_message, cnt, ...)    printf(_message, ##__VA_ARGS__)
    #define log_rtc_error(_message, cnt, ...)   printf(_message, ##__VA_ARGS__)
#else
    #define log_rtc_info(_message, cnt, ...)    log_hal_msgid_info(_message,cnt,    ##__VA_ARGS__)
    #define log_rtc_warn(_message, cnt, ...)    log_hal_msgid_warning(_message,cnt, ##__VA_ARGS__)
    #define log_rtc_error(_message,cnt, ...)    log_hal_msgid_error(_message,cnt,   ##__VA_ARGS__)
#endif

#define     RTC_EOSC_FREQ_CALI_WINDOW           99

/*strap pin info RG*/
#define     RG_SYSTEM_INFO                      (0xA2010000)
#define     SYSTEM_INFO_RTC_CLOCK_TYPE_OFS      (4)
#define     SYSTEM_INFO_RTC_CLOCK_TYPE_MASK     (1<<SYSTEM_INFO_RTC_CLOCK_TYPE_OFS)

/*measure 32k-clock*/
#define     AIR_RTC_FCXO_CLOCK                  (26000000.0)

#ifdef AIR_CPU_IN_SECURITY_MODE
#define     RTC_ABIST_FQMTR_BASE                (0x42030400)
#define     RTC_CKSYS_TST_SEL_1_BASE            ((volatile uint32_t *) 0x42030224)
#else
#define     RTC_ABIST_FQMTR_BASE                (0x52030400)
#define     RTC_CKSYS_TST_SEL_1_BASE            ((volatile uint32_t *) 0x52030224)
#endif
#define     RTC_ABIST_RESET_OFFSET              (24)
#define     RTC_ABIST_ENABLE_OFFSET             (16)
#define     RTC_ABIST_STATUS_OFFSET             (7)
/*Referrence clock for measure 32k*/
#define     RTC_FQMTR_TCKSEL_XO_CK              (xo_ck)

#define     RTC_SPAR_RG_RESRV_OFFSET            (8)
#define     RTC_SPAR_RG_RESRV_MASK              (0xFF<<RTC_SPAR_RG_RESRV_OFFSET)
#define     RTC_SPAR_PWR_FLG_OFFSET             (7)
#define     RTC_SPAR_PWR_FLG_MASK               (1<<RTC_SPAR_PWR_FLG_OFFSET)
#define     RTC_SPAR_BROM_SKIP_OFFSET           (0)
#define     RTC_SPAR_BROM_SKIP_MASK             (0x1<<RTC_SPAR_BROM_SKIP_OFFSET)

#define     RTC_ST_NO_INIT_REG_MASK             (0x1)
#define     RTC_ST_PWR_BY_RTC_MASK              (0x2)

/* frequency meter start */
typedef struct {
    __IO uint32_t RTC_ABIST_FQMTR_CON0;     /* Frequency Meter Control Register 0 */
    __IO uint32_t RTC_ABIST_FQMTR_CON1;     /* Frequency Meter Control Register 1 */
    __IO uint32_t RTC_ABIST_FQMTR_CON2;     /* Frequency Meter Control Register 2 */
    __IO uint32_t RTC_ABIST_FQMTR_DATA;     /* Frequency Meter Data */
} RTC_ABIST_FQMTR_REGISTER_T;

typedef enum {
    RTC_POWERED_REASON_UNKNOW = 0,
    RTC_POWERED_BY_1ST,
    RTC_POWERED_BY_ALARM,
    RTC_POWERED_BY_TICK,
    RTC_POWERED_BY_EINT0,
    RTC_POWERED_BY_EINT1,
    RTC_POWERED_BY_EINT2,
    RTC_POWERED_BY_EINT3,
    RTC_POWERED_BY_CAPTOUCH,
    RTC_POWERED_BY_PWRKEY,
}hal_rtc_power_reason_t;

typedef enum {
    RTC_TIME_CALI_NORMAL = 0,
    RTC_TIME_CALI_K_EOSC = 1,
}rtc_time_cali_mode_t;

typedef enum {
    KEY_BBPU_MASK_ALL       = 0x43,
    KEY_BBPU_MASK_ALARM     = 0xA2, //bit0
    KEY_BBPU_MASK_CLR_WAKEUP= 0x39, //bit1
    KEY_BBPU_MASK_TICK      = 0xDA, //bit2
    KEY_BBPU_MASK_EINT0     = 0xC8, //bit3
    KEY_BBPU_MASK_EINT1     = 0x64, //bit4
    KEY_BBPU_MASK_EINT2     = 0xEE, //bit5
    KEY_BBPU_MASK_EINT3     = 0x5F, //bit6
    KEY_BBPU_MASK_CAP       = 0x73, //bit7
    KEY_BBPU_MASK_VAL_RELOAD= 0x58,
}rtc_bbpu_bitmask_key_t;

typedef enum {
    RTC_WAKEUP_SOURCE_ALARM  =  0,
    RTC_WAKEUP_SOURCE_TICK   =  2,
    RTC_WAKEUP_SOURCE_EINT0  =  3,
    RTC_WAKEUP_SOURCE_EINT1  =  4,
    RTC_WAKEUP_SOURCE_EINT2  =  5,
    RTC_WAKEUP_SOURCE_EINT3  =  6,
    RTC_WAKEUP_SOURCE_CAP    =  7,
//RTC_WAKEUP_SOURCE_GALARM  = 4,
}rtc_wakeup_source_t;

typedef enum {
    RTC_IRQ_ENABLE_LOWPWR   = 0,
    RTC_IRQ_ENABLE_TICK     = 1,
    RTC_IRQ_ENABLE_ONESHOT  = 8,
}rtc_irq_enable_t;

typedef enum {
    RTC_TC_EN_1_8_SECOND    = 0,
    RTC_TC_EN_1_4_SECOND    = 1,
    RTC_TC_EN_1_2_SECOND    = 2,
    RTC_TC_EN_SECOND        = 3,
    RTC_TC_EN_MINUTE        = 4,
    RTC_TC_EN_HOUR          = 5,
    RTC_TC_EN_DAY           = 6,
    RTC_TC_EN_WEEK          = 7,
    RTC_TC_EN_MONTH         = 8,
    RTC_TC_EN_YEAR          = 9,
}rtc_tick_enable_t;

typedef enum {
    RTC_AL_MASK_SECOND      = 0,
    RTC_AL_MASK_MINUTE      = 1,
    RTC_AL_MASK_HOUR        = 2,
    RTC_AL_MASK_DAY         = 3,
    RTC_AL_MASK_WEEK        = 4,
    RTC_AL_MASK_MONTH       = 5,
    RTC_AL_MASK_YEAR        = 6,
    RTC_AL_MASK_MS          = 7,
}rtc_alarm_mask_t;

typedef enum {
    RTC_CAPTOUCH_NORMAL_MODE = 0,
    RTC_CAPTOUCH_LOWPOWER_MODE
}rtc_captouch_power_type;

typedef enum {
    RTC_OSC32K_FCLK_MODE = 1,
    RTC_OSC32K_EOSC_MODE = 4,
    RTC_OSC32K_DCXO_MODE = 5,
    RTC_OSC32K_XOSC_MODE = 6,
}rtc_osc32k_mode_t;

typedef enum {
    HAL_RTC_OSC32K_EOSC_MODE = 0,
    HAL_RTC_OSC32K_DCXO_MODE = 1,
    HAL_RTC_OSC32K_XOSC_MODE = 2,
    HAL_RTC_OSC32K_FCLK_MODE,
}hal_rtc_osc32k_mode_t;

typedef enum {
    HAL_RTC_CLOCK_USER_CONSYS = 0,
    HAL_RTC_CLOCK_USER_AUDIO,
    HAL_RTC_CLOCK_USER_NONE,
}hal_rtc_clock_switch_user_t;

typedef enum {
    RTC_OP_SET,
    RTC_OP_GET
}rtc_op_type_t;

enum {
    RTC_IOCTL_CLEAR_WAKEUP_STATUS = 0,
    RTC_IOCTL_GET_IRQ_STATUS,
    RTC_IOCTL_SET_32K_OFF,
    RTC_IOCTL_GET_PWRKEY_STATUS,
    RTC_IOCTL_GET_PWRBYRTC_ST,
    RTC_IOCTL_CLEAR_PWR_STATE,
    RTC_IOCTL_SET_ONETIME_CALI,
    RTC_IOCTL_GET_ONETIME_CALI,
    RTC_IOCTL_SET_TIME_CALI,
    RTC_IOCTL_GET_TIME_CALI,
    RTC_IOCTL_SET_SPAR_REG,
    RTC_IOCTL_GET_SPAR_REG,
    RTC_IOCTL_SET_EOSC_FREQ_CALI,
    RTC_IOCTL_UNLOCK_PROTECT,
    RTC_IOCTL_GET_PWRON_REASON,
    RTC_IOCTL_PRINT_PWRON_REASON,
    RTC_IOCTL_CALC_TIME_CALI,
    RTC_IOCTL_GET_32K_SEL,
};

typedef enum {
    HAL_RTC_CAPTOUCH_SET_CAPCON = 0,
    HAL_RTC_CAPTOUCH_CLR_WAKEUP_ST,
}hal_rtc_captouch_cmd_t;

typedef struct {
    hal_rtc_time_notification_period_t type;
    uint8_t                            mask;
}rtc_period_type_mask_map_t;


typedef struct {
    uint32_t  gpt_handle;
    uint32_t  clk_1m_cnt[2];
    uint32_t  clk_32k_cnt[2];
    uint8_t   meter_state;
}hal_rtc_32k_merter_info_t;



#define     RTC_STATE_INIT_BY_BL_FLG        0x1    /* flag for indicate whether rtc init by bl */
#define     RTC_STATE_POWER_LOST_FLG        0x2  /* flag for indicate whether rtc power lost */
#define     RTC_STATE_RTC_MODE_FLG          0x4  /* flag for indicate whether back from  rtc mode */
#define     RTC_STATE_RTC_WAKE_FLG          0x8  /* flag for indicate system wakeup by rtc event */

/* pd | pu << 1 | smt << 2 | e4 << 3 | e << 4 | g << 5 | i << 6 */
#define     RTC_GPIO_CFG_PD_MASK            (0x1 << 0)  /* <Pulldown> 1:enable;0:disable */
#define     RTC_GPIO_CFG_PU_MASK            (0x1 << 1)  /* <Pullup>   1:enable;0:disable */
#define     RTC_GPIO_CFG_SMT_MASK           (0x1 << 2)  /* <SMT> 1:enable; 0:disable, for decrease the glitch affect when input */
#define     RTC_GPIO_CFG_E4_MASK            (0x1 << 3)  /* <E4>  1:enable; 0:disable, for increase the io driving when output */
#define     RTC_GPIO_CFG_IO_MASK            (0x1 << 4)  /* <E>   1:output; 0:input, for io direction */
#define     RTC_GPIO_CFG_AD_MASK            (0x1 << 5)  /* <G>   1:digtal; 0:analog, for io typw */
#define     RTC_GPIO_CFG_O_MASK             (0x1 << 6)  /* <i>   1:high;   0:low, for io level control when direction is output */

typedef struct {
    bool       initialized;
    bool       used_xosc;
    bool       used_gpio;
    uint8_t    op_state;
    uint8_t    sel_clk;
    uint8_t    sram_mode;
    uint16_t   irq_status;
    int        cali_eosc; /*only for eosc adjust*/
    int        cali_time;  /*for all osc32 adjust*/
    uint32_t   sram_mask;
    uint32_t   freq_f32k;
    uint32_t   freq_eosc;
    uint32_t   freq_xosc;
    hal_rtc_time_callback_t   tick_callback;
    void                     *tick_userdata;
    hal_rtc_alarm_callback_t  alarm_callback;
    void                     *alarm_userdata;
    //hal_rtc_alarm_callback_t  galarm_callback;
    //void                     *galarm_userdata;
    hal_rtc_eint_callback_t   eint_callback[HAL_RTC_GPIO_MAX];
    void                     *eint_userdata[HAL_RTC_GPIO_MAX];
    uint8_t                   rtc_gpio_config[HAL_RTC_GPIO_MAX];
    uint8_t                   ref_cnt_sw_clk;
    hal_rtc_32k_merter_info_t meter_32k_handle;
}rtc_private_parameter_t;





void                rtc_internal_reload(void);
void                rtc_internal_enable_setting(bool force_ctrl, bool enable);
bool                rtc_internal_enable_tick_notify(hal_rtc_time_notification_period_t type, bool enable);
void                rtc_internal_set_power_key(void);
void                rtc_internal_set_osc32_mode(hal_rtc_osc32k_mode_t mode);
void                rtc_internal_init_register(void);
hal_rtc_status_t    rtc_internal_eint_setting(hal_rtc_eint_config_t *eint_config);
hal_rtc_status_t    rtc_internal_set_retention_reg(uint32_t offset, uint8_t *buff, uint32_t size);
hal_rtc_status_t    rtc_internal_get_retention_reg(uint32_t offset, uint8_t *buff, uint32_t size);
hal_rtc_status_t    rtc_internal_set_ldo_mode(bool  is_lp);
void                rtc_internal_set_ldocon(uint32_t reg_id, uint32_t val);
uint32_t            rtc_internal_get_ldocon(uint32_t reg_id);
uint32_t            rtc_internal_get_eosc32_freq_calibration(uint32_t target_32k_freq);

int32_t             rtc_internal_ioctrl(uint32_t cmd, uint32_t option, uint32_t args);
void                rtc_internal_alarm_control(rtc_op_type_t op, hal_rtc_time_t *gtime, const hal_rtc_time_t *stime, bool *enable, uint32_t *mask);
void                rtc_internal_datetime_control(rtc_op_type_t op, hal_rtc_time_t *gtime, const hal_rtc_time_t *stime);

hal_rtc_status_t    rtc_internal_set_capcon(uint32_t val);

void                rtc_internal_dump(char*);
void                rtc_internal_output_32k_to_gpio();

#ifdef RTC_GPIO_SUPPORTED
void                rtc_internal_gpio_init(hal_rtc_gpio_t  rtc_gpio, uint32_t value);
hal_rtc_status_t    rtc_internal_gpio_control(hal_rtc_gpio_t  rtc_gpio, bool is_output, bool *level);
#endif

#ifdef RTC_CAPTOUCH_SUPPORTED
hal_rtc_status_t    rtc_internal_captouch_init(void);
hal_rtc_status_t    rtc_internal_captouch_deinit(void);
hal_rtc_status_t    rtc_internal_captouch_ldo08_boost(bool is_boost);
hal_rtc_status_t    rtc_internal_captouch_lowpower(rtc_captouch_power_type lowpower_type);
bool                rtc_internal_captouch_get_capcon_state(void);
#endif

#ifdef RTC_SRAM_SUPPORTED
void                rtc_internal_sram_setting_cg(bool   disable_ck, bool disable_do);
void                rtc_internal_sram_setting(hal_rtc_sram_cell_t cell, hal_rtc_sram_mode_t mode);
void                rtc_internal_sram_setting(hal_rtc_sram_cell_t cell, hal_rtc_sram_mode_t mode);
#endif

void                rtc_internal_select_32k(hal_rtc_osc32k_mode_t mode);
int                 hal_rtc_get_power_on_reason();
/*internal using*/
uint32_t            rtc_internal_time_to_tick(uint8_t  year,uint8_t month, uint8_t day,uint8_t hour, uint8_t min, uint8_t second);
bool                rtc_internal_tick_to_time(uint32_t tick, hal_rtc_time_t *time);



void                rtc_internal_lp_switch_32k(hal_rtc_osc32k_mode_t mode);
hal_rtc_status_t    hal_rtc_switch_32k_source(hal_rtc_osc32k_mode_t mode);
float               hal_rtc_measure_32k_with_windows(hal_rtc_osc32k_mode_t mode, uint32_t windowset);
void                hal_rtc_switch_to_dcxo(hal_rtc_clock_switch_user_t handle, bool switch_to_dcxo);
hal_rtc_status_t    hal_rtc_captouch_control(hal_rtc_captouch_cmd_t cmd, uint32_t option);
#ifdef HAL_RTC_FEATURE_32K_TO_GPIO
void                hal_rtc_output_32k_to_gpio(uint8_t pin, uint8_t pin_func, uint32_t ctrl);
#endif

#ifdef __cplusplus
}
#endif

#endif /*End HAL_RTC_MODULE_ENABLED*/
#endif /*End _HAL_RTC_INTERNAL_H_*/



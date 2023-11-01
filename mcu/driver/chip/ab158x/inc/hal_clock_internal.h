/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights RESERVED.
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

#ifndef __HAL_CLOCK_INTERNAL_H__
#define __HAL_CLOCK_INTERNAL_H__


#include "hal_clock.h"
#include "memory_attribute.h"
#ifdef HAL_CLOCK_MODULE_ENABLED
#include "hal_clock_platform.h"

#define HAL_CLOCK_CG_PSUEDO_NUM (HAL_CLOCK_CG_END - HAL_CLOCK_CG_PSUEDO_CLK_26M)
#define CLK_PHYS_MUX_NUM (CLK_OSC_26M_SEL - CLK_SYS_SEL + 1)
#define  CLK_PSUEDO_MUX_NUM (TOTAL_MUX_CNT - CLK_MCLK_SEL)



#define CM4_DCM_CON_REG                  ((cm4_dcm_con*)CMSYS_DCM_CON_0)->field
#define SFC_DCM_CON_REG                  ((sfc_dcm_con*)SFC_DCM_CON_0) ->field
#define ESC_DCM_CON_REG                  ((esc_dcm_con*)ESC_DCM_CON_0) ->field
#define DSP_DCM_CON_REG                  ((dsp_dcm_con*)DSP_DCM_CON_0) ->field
#define AUD_DCM_CON_REG                  ((aud_dcm_con*)AUD_DCM_CON_0)->field

#define GPIO_CLKO_CTRL_A_REG             ((gpio_clko_ctrl_a*)GPIO_CLKO_CTRL_A) ->field
#define PLL_ABIST_FQMTR_COM_REG          ((pll_abist_fqmtr_con*)PLL_ABIST_FQMTR_CON0) ->field
#define CKSYS_MISC_REG                   ((cksys_misc*)CKSYS_MISC_0) ->field
#define AUD_CKDIV_REG                    ((aud_ckdiv_cfg*)RG_AUD_CKDIV_CFG0)->field
#define CKSYS_REF_CLK_REG                ((cksys_ref_clk_sel*)CKSYS_REF_CLK_SEL)->field
#define XPLL_DBG_PROB_REG                ((xpll_dbg_prob*)XPLL_DBG_PROB)->field

#define CKSYS_CLK_CFG_REG                ((cksys_clk_cfg*)CKSYS_CLK_CFG_0)->field
#define CKSYS_CLK_UPDATE_REG             ((cksys_clk_update*)CKSYS_CLK_UPDATE_0)->field
#define CKSYS_CLK_UPDATE_STATUS_REG      ((cksys_clk_update_status*)CKSYS_CLK_UPDATE_STATUS_0) ->field
#define CKSYS_CLK_FORCE_ON_REG           ((cksys_clk_force_on*)CKSYS_CLK_FORCE_ON_0) ->field
#define XO_DCM_CON_REG                   ((xo_dcm_con*)XO_DCM_CON_0)->field

#define CKSYS_CLK_DIV_REG                ((cksys_clk_div*)CKSYS_CLK_DIV_0) ->field
#define CKSYS_SRC_CLK_CG_REG             ((cksys_src_clk_cg*)CKSYS_SRC_CLK_CG0) ->field
#define SYS_ABIST_MON_CON_REG            ((sys_abist_mon_con*)SYS_ABIST_MON_CON0) ->field
#define CLKSQ_CON_REG                    ((clksq_con*)CLKSQ_CON0)->field
#define LPOSC1_CON_REG                   ((lposc_con*)LPOSC1_CON0)->field
#define LPOSC2_CON_REG                   ((lposc_con*)LPOSC2_CON0)->field
#define APLL1_CON_REG                    ((apll_con*)APLL1_CTL0) ->field
#define APLL2_CON_REG                    ((apll_con*)APLL2_CTL0) ->field
#define UPLL_CON_REG                     ((upll_con*)UPLL_CON0)  ->field
#define RSV_CON0_REG                     ((rsv_con0*)RSV_CON0) ->field

#define SSC1_CON                         ((ssc_con*)SSC1_CON0) ->field
#define SSC2_CON                         ((ssc_con*)SSC2_CON0) ->field
#define DCXO_PCON_REG                    ((dcxo_pcon*)DCXO_PCON0) ->field
#define DCXO_DEBUG_REG                   ((dcxo_debug*)DCXO_DEBUG0) ->field
#define CLOCK_SEL_REG                    ((clock_sel_con*)CLOCK_SEL0) ->field


#define BUS_DCM_CON_REG                  ((bus_dcm_con*)BUS_DCM_CON_0)->field


#define EFUSE                            ((EFUSE_Type*)             EFUSE_BASE)
#define DCXO_CFG                         ((DCXO_CFG_Type*)          DCXO_CFG_BASE)

#define MUX_SEL_NUM                      8
#define TUNE_TBL_SIZE                    2
#define OSC_NUM                          2
#define NUM_DIV_SRC                      3

#define CG_PWM_MULTI_CTRL_BITMASK     (0x3FFF)
#define PWM_MULTI_CTRL_REQUEST(clock_id)  ((clock_id & ~(CG_PWM_MULTI_CTRL_BITMASK)) == (HAL_CLOCK_CG_PWM_MULTI_CTRL))

extern unsigned int dcxo_capid;
void hal_dcxo_init();

typedef enum{
    LPOSC1=0,
    LPOSC2,
}osc_id;

typedef enum {
    CLK_APLL1 = 0,
    CLK_APLL2 = 1,
    NR_APLL = 2,
    CLK_UPLL = 2,
    NR_PLLS = 3,
    CLK_OSC1 = 4,
    CLK_OSC2 = 5,
    CLK_XO = 6
} clock_pll_id;

typedef struct {
    clock_pll_id src;
    uint8_t div;
} pin_mux_t;

typedef void (*post_handle)();

typedef enum{
    FINE_FT,
    COARSE_CALI,
    COARSE_FT,
    FINE_CALI,
}osc_cali_t;

/*
 * Max value for each scale type
 * Check corresponding register field definition
 */
typedef enum {
    lposc_cali_scale = 0x3F,
    lposc_ft_scale = 0xF
}lposc_cali_t;

typedef struct {
    lposc_cali_t scale;
    hal_src_clock src;
    uint32_t cali_target;
}cali_info_t ;

typedef enum{
    CLK_DISABLE= 0xff,
    CLK_ENABLE= 0xfe,
}clock_linkage_type;

typedef enum {
    clk_dsp_dcm,
    clk_aud_bus_dcm,
    clk_bus_dcm,
    clk_sfc_dcm,
    clk_cm4_dcm,
    clk_esc_dcm
}hal_clk_dcm_id;


typedef enum{
    OSC1_DIV_D2_TYPE = 0,
    OSC1_DIV_D3_TYPE,
    OSC1_DIV_D5_TYPE,
    OSC1_DIV_D12_TYPE,
    OSC1_DIV_D8_TYPE,
    OSC2_DIV_D2_TYPE = 0x10,
    OSC2_DIV_D3_TYPE,
    OSC2_DIV_D5_TYPE,
    OSC2_DIV_D12_TYPE,
    NONE_DIV_TYPE,
}div_type_e;


typedef enum{
    XO_CLK_26M   = 26000,
    OSC_CLK_104M = 104000,
    OSC_CLK_173M = 173000,
    OSC_CLK_312M = 312000,
    OSC_CLK_520M = 520000
}osc_clk_target_freq;


typedef enum {
    VOLT_0P75_LV,
    VOLT_0P8_LV,
    VOLT_0P9_LV,
    VOLT_LV_NUM,
    VOLT_NONE_LV
}hal_clock_volt_lv;

typedef enum {
    XO_CK,                 //26m/24m
    RTC_CK,                //32k
    PAD_SOC_REF_CK,            //NA
    CHOP_CK,               //1.3.20
    EOSC_F32K,             //NA
    DCXO_F32K,             //NA
    XOSC_F32K,             //NA
}ref_clock;

typedef enum{
    decr,
    incr
}mod_type;

typedef enum{
    nonexist,
    exist,
}clk_usr_status;

typedef enum{
    CLK_APLL_45M,
    CLK_APLL_49M
}clk_apll_freq;

typedef enum{
    DCXO_LP_MODE,
    DCXO_NORMAL_MODE,
}dcxo_mode_t ;

typedef enum {
    DCXO_FPM_LOCK_ATC,
    DCXO_FPM_LOCK_BT,
    DCXO_FPM_LOCK_USB,
    DCXO_FPM_LOCK_TEST,
    DCXO_FPM_LOCK_MAX
} dcxo_fpm_lock_src_t;

typedef enum {
    DCXO_FPM_UNLOCK,
    DCXO_FPM_LOCK
} dcxo_fpm_lock_op_t;

hal_clock_status_t clk_set_apll(clock_pll_id,clk_apll_freq);
typedef clk_usr_status (*usr_status)();

typedef struct {
   usr_status  usr_sta;
   post_handle post_hdr;
}clk_src_handler ;

typedef enum{
    UPLL_DIV_IDX,
    OSC1_DIV_IDX,
    OSC2_DIV_IDX,
    NONE_DIV
}clock_div_ctrl_idx;
/* This struct define matches the divider enable register field offset */
typedef union {
    uint32_t src_cnt[2];
    struct {
        uint8_t div_d2;
        uint8_t div_d3;
        uint8_t div_d5;
        uint8_t div_d12;
        uint8_t div_d8;
        uint8_t reserved0;
        uint16_t reserved1;
    }__attribute__((packed)) field;
}clk_div_info;

typedef union {
    uint32_t src_cnt;
    struct {
        uint8_t apll1;
        uint8_t apll2;
        uint8_t dcxo;
        uint8_t upll;
    }__attribute__((packed)) field;
}clk_dcxo_non_div_info;

typedef union{
    uint32_t src_cnt[3];
    struct {
        clk_div_info upll;
        clk_dcxo_non_div_info free;
    } field;
}clk_dcxo_domain;

typedef union{
    uint32_t src_cnt[3];
    struct {
        uint32_t free;
        clk_div_info osc;
    } field;
}clk_lposc_domain;

/* definition matches F_CLK_DSL_SEL value */
typedef enum{
    DVFS_DSP_CLOCK_156M = 3,
    DVFS_DSP_CLOCK_312M = 2,
    DVFS_DSP_CLOCK_520M = 5,
}dvfs_clock_sys_sel;

typedef enum {
    SYS_26M_XO = 0,
    SYS_26M_OSC = 1,
    SYS_156M_OSC1_D2 = 2,
    SYS_104M_OSC1_D3 = 3,
    SYS_78M_OSC1_D2_D2 = 4,
    SYS_260M_OSC2_D2 = 5,
    SYS_130M_OSC2_D2_D2 = 6,
    SYS_156M_UPLL_D2 = 7
} f_clk_sys_sel;

typedef enum {
    DSP_26M_XO = 0,
    DSP_26M_OSC = 1,
    DSP_312M_OSC1 = 2,
    DSP_156M_OSC1_D2 = 3,
    DSP_104M_OSC1_D3 = 4,
    DSP_520M_OSC2 = 5,
    DSP_104M_OSC2_D5 = 6,
    DSP_156M_UPLL_D2 = 7
} f_clk_dsp_sel;

typedef enum {
    SFC_26M_XO = 0,
    SFC_26M_OSC = 1,
    SFC_104M_OSC1_D3 = 2,
    SFC_52M_OSC1_D3_D2 = 3,
    SFC_39M_OSC1_D8 = 4,
    SFC_173M_OSC2_D3 = 5,
    SFC_104M_OSC2_D5 = 6,
    SFC_104M_UPLL_D3 = 7
} f_clk_sfc_sel;

typedef enum {
    ESC_26M_XO = 0,
    ESC_104M_OSC1_D3 = 1,
    ESC_78M_OSC1_D2_D2 = 2,
    ESC_52M_OSC1_D3_D2 = 3,
    ESC_39M_OSC1_D8 = 4,
    ESC_104M_OSC2_D5 = 5,
    ESC_86M_OSC2_D3_D2 = 6,
    ESC_104M_UPLL_D3 = 7
} f_clk_esc_sel;

typedef enum {
    FPWM_26M_XO = 0,
    FPWM_32K_RTC = 1,
    FPWM_OSC1_D8 = 2,
    FPWM_OSC2_D3_D2 = 3
} f_fpwm_sel;


typedef union{
    uint32_t value;
    struct {
        uint8_t sys;
        uint8_t dsp;
        uint8_t sfc;
        uint8_t esc;
    }__attribute__((packed)) field;
}clk_volt_lv_info ;
/* Definition for clk_cfg3 register
 * used to pre-define DVFS opp freqs for
 * audio intbus, gpsrc, uplink, dwlink
 */
typedef union {
    uint32_t value;
    struct {
        uint8_t aud_intbus;
        uint8_t aud_gpsrc;
        uint8_t aud_uplink;
        uint8_t aud_dwlink;
    }__attribute__((packed)) field;
} clk_cfg3_config;
typedef enum{
    CLK_OSC_FREE,
    CLK_OSC_DIV
}osc_domain_indx;

hal_clock_status_t clock_div_ctrl(pin_mux_t mux,bool on_off);

/* used 3 bits */
typedef enum {
    CLK_DIV_D2 = 0,
    CLK_DIV_D3 = 1,
    CLK_DIV_D5 = 2,
    CLK_DIV_D8 = 3,
    CLK_DIV_D12 = 4,
    CLK_DIV_D15 = 5,
    CLK_DIV_26M = 6,
    CLK_DIV_NONE = 7
} CLK_DIV_TYPE;

typedef enum {
    DIV_TBL_UPLL = 0,
    DIV_TBL_OSC1,
    DIV_TBL_OSC2
} div_tbl_idx;


#if 1
/* structs for checking if clock resource (divider, clock source) have any users */
/* struct for clk src user check */
typedef struct {
    uint8_t lposc1;
    uint8_t lposc2;
    uint8_t upll;
    uint8_t apll1;
    uint8_t apll2;
} clk_src_chk_t;

/* struct for clk src divider check (lposc1, lposc2, upll) */
typedef struct {
    uint8_t d2;
    uint8_t d3;
    uint8_t d5;
    uint8_t d12;
    uint8_t d8;
} div_chk_t;

/* root struct for clock resource check */
typedef struct {
    clk_src_chk_t src_chk;
    div_chk_t lposc1_div_chk;
    div_chk_t lposc2_div_chk;
    div_chk_t upll_div_chk;
} clock_resource_chk_t;

typedef enum {
    MUX_NOT_ALLOW = 0,
    MUX_ALLOW = 1
} hal_mux_allow_t;

typedef enum {
    PHYSICAL_CG,
    PSUEDO_CG
} hal_cg_type_t;

typedef enum {
    PHYSICAL_N_TO_1_MUX,
    PHYSICAL_NORM_MUX,
    PSUEDO_MUX
} hal_mux_type_t;

typedef struct {
    hal_clock_cg_id cg_id;
    hal_cg_type_t cg_type;
} hal_cg_t;

typedef struct {
    clock_pll_id src : 4; /* clock_pll_id */
    uint8_t div : 3; /* CLK_DIV_TYPE */
    hal_mux_allow_t mux_allow : 1; /* hal_mux_allow_t */
} hal_mux_t;

/* mux_sel_id == idx in tbl */
typedef struct {
    hal_mux_t sels[MUX_SEL_NUM];
    hal_mux_type_t mux_type;
    /* curr_sel_ptr:
     * for physical mux: points to 8 bit register of current mux sel option
     * for psuedo mux: points to entry in psuedo_mux_tbl of current mux select option
     */
    volatile uint8_t *curr_sel_ptr;
    hal_cg_t cg_info;
} mux_tbl_t;

typedef struct {
    mux_tbl_t mux_tbl[TOTAL_MUX_CNT];
} clock_tbl_t;

typedef enum {
    CG_REQUEST_STATUS,
    CG_REQUEST_CLEAR,
    CG_REQUEST_SET,
} cg_request_t;
/* below are for cg_init code readability */
#define CLK_REQ_ENABLE   CG_REQUEST_CLEAR
#define CLK_REQ_DISABLE  CG_REQUEST_SET
#endif


typedef union {
    uint32_t value[8];
    struct {
        __IO uint8_t GSM_DCXO_CTL_EN;
        __IO uint8_t DIS_DCXO_ISO_EN;
        __IO uint8_t FRC_BB26MCK_OFF;
        __IO uint8_t FRC_DCXO_PWR_ON;//0
        __IO uint8_t FRC_DCXO_F26M_RDY;
        __IO uint8_t FRC_BB26MCK_EN;
        __IO uint8_t FRC_COCLK_EN;
        __IO uint8_t EXT_DCXO_CTL_EN;//1
        __IO uint8_t FRC_EXT_SGLSTGE;
        __IO uint8_t HW_COCLK_EN;
        __IO uint16_t FRC_GSM_SGLSTGE;//2
        __IO uint8_t DCXO_CON_RSV;
        __IO uint8_t DCXO_ISO_EN_TD;
        __IO uint8_t DCXO_BUF_EN_TD;
        __IO uint8_t DCXO_EN_TD;//3
        __IO uint16_t DCXO_PWR_EN_TD;
        __IO uint8_t DCXO_SLEEP_TD;
        __IO uint8_t DCXO_26M_RDY_EN;//4
        __IO uint8_t EN_26M_FPM;
        __IO uint8_t BT_26M_EN;
        __IO uint16_t DCXO32K_EN;//5
        __IO uint16_t DCXO_CAPID_EFUSE_SEL;
        __IO uint16_t DCXO_CAPID_EFUSE;//6
        __IO uint8_t DCXO_CK_RDY_COMP_VREF_SEL;
        __IO uint8_t DCXO_CK_RDY_COMP_VREF_DBB;
        __IO uint8_t RG_DCXO_FPM_LDO_OFF_PULLDOWN;
        __IO uint8_t HI_CDAC_IBOOST_EN;//7
    }field;
}dcxo_pcon;


typedef union {
    uint32_t value[6];
    struct {
        __IO uint8_t CLK_PLL1_DIV_EN;
        __IO uint8_t CLK_OSC1_DIV_EN;
        __IO uint8_t CLK_OSC2_DIV_EN;
        uint8_t RESERVED_0;
        __IO uint8_t CLK_PLL1_D2_EN;
        __IO uint8_t CLK_PLL1_D3_EN;
        __IO uint8_t CLK_PLL1_D5_EN;
        uint8_t RESERVED_1;
        __IO uint8_t CLK_OSC1_D2_EN;
        __IO uint8_t CLK_OSC1_D3_EN;
        __IO uint8_t CLK_OSC1_D5_EN;
        __IO uint8_t CLK_OSC1_D12_EN;
        __IO uint32_t CLK_OSC1_D8_EN;
       __IO uint8_t CLK_OSC2_D2_EN;
        __IO uint8_t CLK_OSC2_D3_EN;
        __IO uint8_t CLK_OSC2_D5_EN;
        __IO uint8_t CLK_OSC2_D12_EN;
        __IO uint8_t CHOP_DIV_EN;
        __IO uint8_t CHOP_DIV_CHG;
        __IO uint16_t CHOP_DIV_SEL;
    }field;
}cksys_clk_div;

typedef union {
    uint32_t value;
    struct {
        uint8_t RG_TM_REF_CLK_SEL;
        uint8_t RG_APLL_REF_CLK_SEL;
        uint8_t CR_PLL_TEST;
        uint8_t RESERVED_0;
    }__attribute__((packed)) field;
}cksys_ref_clk_sel;


typedef union {
    uint32_t value;
    struct {
        uint8_t CLK_MODE0;
        uint8_t CLK_MODE1;
        uint8_t CLK_MODE2;
        uint8_t CLK_MODE3;
    }__attribute__((packed)) field;
}gpio_clko_ctrl_a;

typedef union{
    uint64_t value;
    struct {
        __IO uint16_t DCXO_DEBUG_EN;
        __IO uint16_t DCXO_DEBUG_SEL;
        __O uint16_t DCXO_DEBUG_O;
        uint16_t reserved;
    } field;
}dcxo_debug;

typedef union {
    uint64_t value;
    struct {
        __IO uint8_t RG_AUD_SFSEL; // 1bit
        __IO uint8_t RG_AUD_DCM_DBC_NUM; // 1bit
        __IO uint8_t RG_AUD_DCM_DBC_EN; // 1bit
        __IO uint8_t RG_AUD_PLLCK_SEL; // 1bit
        __IO uint8_t RG_AUD_CLKOFF_EN;
        __IO uint8_t RG_AUD_CLKSLOW_EN; // 1bit
        __IO uint8_t RG_AUD_FORCE_CLKOFF; // 1bit
        __IO uint8_t RG_AUD_FORCE_CLKSLOW; // 1bit
    } field;
}aud_dcm_con;

typedef union {
    uint64_t value;
    struct {
        __IO uint8_t RG_CM_SFSEL; // 1bit
        __IO uint8_t RG_CM_DCM_DBC_NUM; // 1bit
        __IO uint8_t RG_CM_DCM_DBC_EN; // 1bit
        __IO uint8_t RG_CM_PLLCK_SEL; // 1bit
        uint8_t RESERVED_2;
        __IO uint8_t RG_CM_CLKSLOW_EN; // 1bit
        __IO uint8_t RG_CM_FORCE_CLKOFF; // 1bit
        __IO uint8_t RG_CM_FORCE_CLKSLOW; // 1bit
    } field;
}cm4_dcm_con;
typedef union {
    uint64_t value;
    struct {
        __IO uint8_t RG_XO_SFSEL; // 1bit
        __IO uint8_t RG_XO_DCM_DBC_NUM; // 1bit
        __IO uint8_t RG_XO_DCM_DBC_EN; // 1bit
        __IO uint8_t RG_XO_PLLCK_SEL;
        __IO uint8_t RG_XO_CLKOFF_EN; // 1bit
        __IO uint8_t RG_XO_CLKSLOW_EN; // 1bit
        __IO uint8_t RG_XO_FORCE_CLKOFF; // 1bit
        __IO uint8_t RG_XO_FORCE_CLKSLOW; // 1bit
    } field;
}xo_dcm_con;


typedef union {
    uint64_t value;
    struct {
        __IO uint8_t RG_DSP_SFSEL; // 1bit
        __IO uint8_t RG_DSP_DCM_DBC_NUM; // 1bit
        __IO uint8_t RG_DSP_DCM_DBC_EN; // 1bit
        __IO uint8_t RG_DSP_PLLCK_SEL;
        uint8_t RESERVED_0;
        __IO uint8_t RG_DSP_CLKSLOW_EN; // 1bit
        __IO uint8_t RG_DSP_FORCE_CLKOFF; // 1bit
        __IO uint8_t RG_DSP_FORCE_CLKSLOW; // 1bit
    } field;
}dsp_dcm_con;

typedef union {
    uint64_t value;
    struct {
        uint8_t RESERVED_0;
        __IO uint8_t RG_SFC_DCM_DBC_NUM;
        __IO uint8_t RG_SFC_DCM_DBC_EN;
        __IO uint8_t RG_SFC_DCM_APB_SEL;
        __IO uint8_t RG_SFC_CLK_OFF_EN;
        uint8_t RESERVED_1;
        __IO uint8_t RG_SFC_FORCE_CLK_OFF;
        __IO uint8_t RG_SFC_DCM_APB_TOG;
    } field;
}sfc_dcm_con;

typedef union {
    uint32_t value[4];
    struct {
        __IO uint16_t ABIST_MON_CFG;
        __IO uint16_t ABIST_MON_CHG;
        __IO uint16_t ABIST_LMON_SEL;
        __IO uint16_t ABIST_HMON_SEL;
        __IO uint16_t ABIST_LMON_OUT;
        __IO uint16_t ABIST_HMON_OUT;
        __IO uint16_t ABIST_LMON_DATA;
        __IO uint16_t ABIST_HMON_DATA;
    } field; //SYS_ABIST_MON_CON
}sys_abist_mon_con;

typedef union {
    uint64_t value;
    struct {
        uint8_t RESERVED_0;
        __IO uint8_t RG_ESC_DCM_DBC_NUM;
        __IO uint8_t RG_ESC_DCM_DBC_EN;
        __IO uint8_t RG_ESC_DCM_APB_SEL;
        __IO uint8_t RG_ESC_CLK_OFF_EN;
        uint8_t RESERVED_1;
        __IO uint8_t RG_ESC_FORCE_CLK_OFF;
        __IO uint8_t RG_ESC_DCM_APB_TOG;
    } field;
}esc_dcm_con;


typedef union {
    uint64_t value;
    struct {
        __IO uint32_t XPLL_DBG_SEL;
        __O uint32_t XPLL_DBG_PROB_MON_REG; /* defined as XPLL_DBG_PROB in CODA, named to MON_REG here */
    } field;
}xpll_dbg_prob;

typedef union {
    uint64_t value;
    struct {
        __IO uint8_t OSC_CKSYS_EN;
        __IO uint8_t OSC_CKDIV_EN;
        uint16_t RESERVED;
        __IO uint8_t PLL1_CKSYS_EN;
        __IO uint8_t PLL1_CKDIV_EN;
        uint16_t RESERVED_1;
    } field;
}cksys_src_clk_cg;

typedef union {
    uint32_t value[11];
    struct {
        __IO uint8_t DA_UPLL_EN; //word 0
        __IO uint8_t RG_UPLL_PREDIV;
        __IO uint16_t RG_UPLL_POSTDIV;
        __IO uint8_t RG_UPLL_FBDIV;//word 1
        __IO uint8_t RG_UPLL_FBDIV2;
        __IO uint8_t RG_UPLL_48M_EN;
        __IO uint8_t RG_UPLL_624M_EN;
        __IO uint8_t RG_UPLL_MONCK_EN;//word 2
        __IO uint8_t RG_UPLL_MONREF_EN;
        __IO uint8_t RG_UPLL_MONVC_EN;
        __IO uint8_t RG_UPLL_48M_MON_EN;
        __IO uint8_t RG_UPLL_CLKIN_SEL;//word 3
        __IO uint8_t RG_UPLL_CLKMUX_SEL;
        __IO uint16_t RG_UPLL_CLKOUT_SEL;
        __IO uint8_t RG_UPLL_BP;//word 4
        __IO uint8_t RG_UPLL_BP_2X;
        __IO uint8_t RG_UPLL_BR;
        __IO uint8_t RG_UPLL_DIV;
        __IO uint8_t RG_UPLL_LDOOUT1_SEL;//word 5
        __IO uint8_t RG_UPLL_LDOOUT2_SEL;
        __IO uint8_t RG_UPLL_LDOOUT3_SEL;
        __IO uint8_t RG_UPLL_VREF_FC;
        __IO uint8_t RG_UPLL_LOCKDET_EN;//word 6
        __IO uint8_t RG_UPLL_LOCKDET_WINDOW_SEL;
        __IO uint8_t RG_UPLL_VTMON_EN;
        __IO uint8_t RG_UPLL_VTMON_WINDOW_SEL;
        __IO uint8_t RG_UPLL_IBANK_EN ;//word 7
        __IO uint8_t RG_UPLL_IBANK_FC;
        __IO uint16_t RG_UPLL_IBANK_FINETUNE;
        __IO uint8_t RG_UPLL_GLOBAL_LDO_BIAS_EN;//word 8
        __IO uint8_t RG_UPLL_GLOBAL_LDO_SEL;
        __IO uint8_t RG_UPLL_LDO_VOUT_TEST;
        __IO uint8_t RG_UPLL_RESERVE;
        __IO uint8_t RG_UPLL_RESET_DLY;//word 9
        __IO uint8_t RG_UPLL_CP_EN;
        __IO uint8_t RG_UPLL_CLKSQ_MON_EN;
        __IO uint8_t RG_UPLL_TESTMODE;
        __IO uint8_t RG_UPLL_KVCO_SEL;//word 10
        __IO uint8_t RG_UPLL_CP_ISEL;
        __IO uint16_t RG_UPLL_BIAS_CALI;
    } field;
}upll_con;

typedef union {
    uint32_t value[7];
    struct {
        __IO uint32_t BP_PLL_DLY;
        __IO uint8_t UPLL_EN_SEL;
        __IO uint8_t UPLL_CK_CG;
        __IO uint8_t UPLL_FORCE_ON;
        __IO uint8_t RG_UPLL_RDY;
        __IO uint8_t LPOSC1_EN_SEL;
        __IO uint8_t LPOSC1_CK_CG;
        __IO uint8_t LPOSC1_FORCE_ON;
        __IO uint8_t RG_LPOSC1_RDY;
        __IO uint8_t LPOSC2_EN_SEL;
        __IO uint8_t LPOSC2_CK_CG;
        __IO uint8_t LPOSC2_FORCE_ON;
        __IO uint8_t RG_LPOSC2_RDY;
        __IO uint16_t UPLL_START_TIME;
        __IO uint16_t UPLL_SETTLE_TIME;
        __IO uint16_t LPOSC1_START_TIME;
        __IO uint16_t LPOSC1_SETTLE_TIME;
        __IO uint16_t LPOSC2_START_TIME;
        __IO uint16_t LPOSC2_SETTLE_TIME;
    } field;
}clksq_con;

typedef union {
    uint32_t value[17];
    struct {
        __IO uint8_t RG_APLL1_DDS_PWR_ON;        /*!< (@ 0x00000000) APLL1_CTL0                                             */
        __IO uint8_t RG_APLL1_DDS_ISO_EN;
        __IO uint8_t AD_RGS_APLL1_DDS_PWR_ACK;
        __IO uint8_t RG_APLL1_V2I_EN;
        __IO uint8_t RG_APLL1_EN;                /*!< (@ 0x00000004) APLL1_CTL1                                             */
        __IO uint8_t RG_APLL1_DIV16_EN;
        __IO uint8_t RG_APLL1_POSDIV;
        __IO uint8_t RG_APLL1_PREDIV;
        __IO uint8_t RG_APLL1_FBKDIV;            /*!< (@ 0x00000008) APLL1_CTL2                                             */
        __IO uint8_t RG_APLL1_BR;
        __IO uint8_t RG_APLL1_BP;
        __IO uint8_t RG_APLL1_FBDIV_PREDIV2_EN;
        __IO uint8_t RG_APLL1_MONREF_EN;         /*!< (@ 0x0000000C) APLL1_CTL3                                             */
        __IO uint8_t RG_APLL1_MONVC_EN;
        __IO uint8_t RG_APLL1_DIV;
        __IO uint8_t RG_APLL1_MONCK_EN;
        __IO uint8_t RG_APLL1_DDSFBK_EN;         /*!< (@ 0x00000010) APLL1_CTL4                                             */
        __IO uint8_t RG_APLL1_BP2X;
        __IO uint8_t RG_APLL1_LVROD_EN;
        __IO uint8_t RG_APLL1_RESET_DLY;
        __IO uint8_t RG_APLL1_LDOOUT2_SEL;       /*!< (@ 0x00000014) APLL1_CTL5                                             */
        __IO uint8_t RG_APLL1_LDOOUT1_SEL;
        __IO uint8_t RG_APLL1_VREF_FC;
        __IO uint8_t RG_APLL1_CP_EN;
        __IO uint8_t RG_APLL1_KVCO_SEL;          /*!< (@ 0x00000018) APLL1_CTL6                                             */
        __IO uint8_t RG_APLL1_IBANK_FC;
        __IO uint8_t RG_APLL1_IBANK_FINETUNE;
        __IO uint8_t RG_APLL1_LDOOUT3_SEL;
        __IO uint8_t RG_APLL1_VTMON_WINDOW_SEL;  /*!< (@ 0x0000001C) APLL1_CTL7                                             */
        __IO uint8_t RG_APLL1_VTMON_EN;
        __IO uint8_t RG_APLL1_LOCKDET_WINDOW_SEL;
        __IO uint8_t RG_APLL1_LOCKDET_EN;
        __IO uint8_t RG_APLL1_TESTBUF_EN;        /*!< (@ 0x00000020) APLL1_CTL8                                             */
        __IO uint8_t RG_APLL1_CLKMUX_SEL;
        __IO uint8_t RG_APLL1_CP_ISEL;
        __IO uint8_t RG_APLL1_CLK_BYPASS_EN;
        __IO uint8_t RG_APLL1_RESERVE;           /*!< (@ 0x00000024) APLL1_CTL9                                             */
        __IO uint8_t RG_APLL1_TESTDIV2_EN;
        __IO uint8_t RG_APLL1_LCDDS_LVROD_EN;
        __IO uint8_t RG_APLL1_TESTBUF_DRIVING_SEL;
        __IO uint32_t RG_APLL1_LCDDS_PCW_NCPO;   /*!< (@ 0x00000028) APLL1_CTL10                                             */
        __IO uint8_t RG_APLL1_LCDDS_EN;          /*!< (@ 0x0000002C) APLL1_CTL11                                             */
        __IO uint8_t RG_APLL1_LCDDS_MON_EN;
        __IO uint8_t RG_APLL1_LCDDS_C;
        __IO uint8_t RG_APLL1_LCDDS_PREDIV2;
        __IO uint8_t RG_APLL1_LCDDS_PCW_NCPO_CHG;/*!< (@ 0x00000030) APLL1_CTL12                                             */
        __IO uint8_t RG_APLL1_LCDDS_SSC_EN;
        __IO uint8_t RG_APLL1_DIVSEL;
        uint8_t RESERVED_2;
        __IO uint32_t RG_APLL1_LCDDS_TUNER_EN;   /*!< (@ 0x00000034) APLL1_CTL13                                             */
        __IO uint32_t RG_APLL1_LCDDS_TUNER_PCW_NCPO; /*!< (@ 0x00000038) APLL1_CTL14                                             */
        __IO uint32_t DA_APLL1_LCDDS_PCW_NCPO;   /*!< (@ 0x0000003C) APLL1_CTL15                                             */
        __IO uint8_t RG_APLL1_LDO_VOUT_TEST_EN;  /*!< (@ 0x00000040) APLL1_CTL16                                             */
        __IO uint8_t RG_APLL1_LDO_VOUT_TEST;
        __IO uint16_t RG_APLL1_IBANK_EN;
    } field;
}apll_con;

typedef union {
    uint64_t value[4];
    struct {
        __IO uint8_t APLL12_DIV0_PDN;
        __IO uint8_t APLL12_DIV1_PDN;
        __IO uint8_t APLL12_DIV2_PDN;
        __IO uint8_t APLL12_DIV3_PDN;
        __IO uint8_t APLL_I2S0_MCK_SEL;
        __IO uint8_t APLL_I2S1_MCK_SEL;
        __IO uint8_t APLL_I2S2_MCK_SEL;
        __IO uint8_t APLL_I2S3_MCK_SEL;
        __IO uint8_t APLL12_CK_DIV0;
        __IO uint8_t APLL12_CK_DIV0_CHG;
        __IO uint8_t APLL12_CK_DIV1;
        __IO uint8_t APLL12_CK_DIV1_CHG;
        __IO uint8_t APLL12_CK_DIV2;
        __IO uint8_t APLL12_CK_DIV2_CHG;
        __IO uint8_t APLL12_CK_DIV3;
        __IO uint8_t APLL12_CK_DIV3_CHG;
    } field;
}aud_ckdiv_cfg;

typedef union {
    struct {
        __IO uint8_t DA_LPOSC_EN;
        __IO uint8_t RG_LPOSC_CK_EN;
        __IO uint8_t RG_LPOSC_DIV3_CK_EN;
        __IO uint8_t RG_LPOSC_DIV48_CK_EN;
        __IO uint8_t DA_LPOSC_RBANK_FT;
        __IO uint8_t DA_LPOSC_RBANK_CALI;
        __IO uint8_t RG_LPOSC_BIAS_SEL;
        __IO uint8_t RG_LPOSC_KVCO_SEL;
        __IO uint8_t RG_LPOSC_CBANK_SEL;
        __IO uint8_t RG_LPOSC_AMP_CP_EN;
        __IO uint8_t RG_LPOSC_AMP_CF_SEL;
        __IO uint8_t RG_LPOSC_LDO_EN;
        __IO uint8_t RG_LPOSC_RSV;
        __IO uint8_t RG_LPOSC_MONCK_EN;
        __IO uint8_t RG_LPOSC_MONCK_DIV_SEL;
        uint8_t RESERVED;
    } field;
}lposc_con;

typedef union {
    uint64_t value;
    struct {
        __IO uint8_t RG_BUS_SFSEL;
        __IO uint8_t RG_BUS_DCM_DBC_NUM;
        __IO uint8_t RG_BUS_DCM_DBC_EN;
        __IO uint8_t RG_BUS_PLLCK_SEL;
        __IO uint8_t RG_BUS_CLKOFF_EN;
        __IO uint8_t RG_BUS_CLKSLOW_EN;
        __IO uint8_t RG_BUS_FORCE_CLK_OFF;
        __IO uint8_t RG_BUS_FORCE_CLKSLOW;
    } field;
}bus_dcm_con;

typedef union { 
    uint64_t value;
    struct {
        __IO uint8_t RG_UART0_BCLK_SEL;
        __IO uint8_t RG_UART1_BCLK_SEL;
        uint16_t RESERVED;
        __IO uint8_t RG_GPT_BCLK_SEL;
        __IO uint8_t RG_OSGPT_BCLK_SEL;
        uint16_t RESERVED_1;
    } field;
}clock_sel_con;

typedef union {
    uint32_t value[3];
    struct {
        __IO uint32_t FQMTR_WINSET;
        __IO uint16_t PLL_ABIST_FQMTR_CON1_; //FQMTR_BUSY Frequency-Meter busy +status
        __IO uint8_t FQMTR_EN;
        __IO uint8_t FQMTR_RST;
        __IO uint8_t FQMTR_CLKDIV;
        __IO uint8_t FQMTR_CLKDIV_EN;
        uint16_t RESERVED;
    } field;
}pll_abist_fqmtr_con;

typedef union {
    uint32_t value[6];
    struct {
        __IO uint8_t CLK_SYS_SEL;
        __IO uint8_t CLK_DSP_SEL;
        __IO uint8_t CLK_SFC_SEL;
        __IO uint8_t CLK_ESC_SEL;
        __IO uint8_t CLK_SPIMST0_SEL;
        __IO uint8_t CLK_SPIMST1_SEL;
        __IO uint8_t CLK_SPIMST2_SEL;
        __IO uint8_t CLK_SPISLV_SEL;
        __IO uint8_t CLK_SDIOMST0_SEL;
        __IO uint8_t CLK_USB_SEL;
        __IO uint8_t CLK_I3C_SEL;
        __IO uint8_t CLK_BT_HOP_SEL;
        __IO uint8_t CLK_AUD_BUS_SEL;
        __IO uint8_t CLK_AUD_GPSRC_SEL;
        __IO uint8_t CLK_AUD_ULCK_SEL;
        __IO uint8_t CLK_AUD_DLCK_SEL;
        __IO uint8_t CLK_AUD_SPDIF_SEL;
        __IO uint8_t CLK_AUD_INTF0_SEL;
        __IO uint8_t CLK_AUD_INTF1_SEL;
        uint8_t RESERVED_2;
        __IO uint8_t CLK_26M_SEL;
        __IO uint8_t CLK_AUD_ENGINE_SEL;
        __IO uint8_t CLK_VOW_SEL;
        __IO uint8_t CLK_OSC_26M_SEL;
    } field;
}cksys_clk_cfg;


typedef union {
    uint32_t value[4];
    struct {
        __IO uint8_t CHG_SYS;
        __IO uint8_t CHG_DSP;
        __IO uint8_t CHG_SFC;
        __IO uint8_t CHG_ESC;
        __IO uint8_t CHG_SPIMST0;
        __IO uint8_t CHG_SPIMST1;
        __IO uint8_t CHG_SPIMST2;
        __IO uint8_t CHG_SPISLV;
        __IO uint8_t CHG_SDIOMST0;
        __IO uint8_t CHG_USB;
        __IO uint8_t CHG_I3C;
        __IO uint8_t CHG_BT_HOP;
        __IO uint8_t CHG_AUD_BUS;
        __IO uint8_t CHG_AUD_GPSRC;
        __IO uint8_t CHG_AUD_ULCK;
        __IO uint8_t CHG_AUD_DLCK;
    } field;
}cksys_clk_update;

typedef union {
    uint32_t value[4];
    struct {
        __IO uint8_t CLK_SYS_OK;
        __IO uint8_t CLK_DSP_OK;
        __IO uint8_t CLK_SFC_OK;
        __IO uint8_t CLK_ESC_OK;
        __IO uint8_t CLK_SPIMST0_OK;
        __IO uint8_t CLK_SPIMST1_OK;
        __IO uint8_t CLK_SPIMST2_OK;
        __IO uint8_t CLK_SPISLV_OK;
        __IO uint8_t CLK_SDIOMST0_OK;
        __IO uint8_t CLK_USB_OK;
        __IO uint8_t CLK_I3C_OK;
        __IO uint8_t CLK_BT_HOP_OK;
        __IO uint8_t CLK_AUD_BUS_OK;
        __IO uint8_t CLK_AUD_GPSRC_OK;
        __IO uint8_t CLK_AUD_ULCK_OK;
        __IO uint8_t CLK_AUD_DLCK_OK;
    } field;
}cksys_clk_update_status;

typedef union {
    uint32_t value[4];
    struct {
        __IO uint8_t CLK_SYS_FORCE_ON;
        __IO uint8_t CLK_DSP_FORCE_ON;
        __IO uint8_t CLK_SFC_FORCE_ON;
        __IO uint8_t CLK_ESC_FORCE_ON;
        __IO uint8_t CLK_SPIMST0_FORCE_ON;
        __IO uint8_t CLK_SPIMST1_FORCE_ON;
        __IO uint8_t CLK_SPIMST2_FORCE_ON;
        __IO uint8_t CLK_SPISLV_FORCE_ON;
        __IO uint8_t CLK_SDIOMST0_FORCE_ON;
        __IO uint8_t CLK_USB_FORCE_ON;
        __IO uint8_t CLK_I3C_FORCE_ON;
        __IO uint8_t CLK_BT_HOP_FORCE_ON;
        __IO uint8_t CLK_AUD_BUS_FORCE_ON;
        __IO uint8_t CLK_AUD_GPSRC_FORCE_ON;
        __IO uint8_t CLK_AUD_ULCK_FORCE_ON;
        __IO uint8_t CLK_AUD_DLCK_FORCE_ON;
    } field;
}cksys_clk_force_on;


typedef union {
    uint32_t value[5];
    struct {
        __IO uint8_t HFOSC_FHCTL_EN;
        __IO uint8_t HFOSC_SFSTR_EN;
        __IO uint8_t HFOSC_FRSSC_EN;
        __IO uint8_t HFOSC_FHCTL_RST;
        __IO uint8_t HFOSC_SFSTR_DTS;
        __IO uint8_t HFOSC_SFSTR_DYS;
        __IO uint8_t HFOSC_FRSSC_DTS;
        __IO uint8_t HFOSC_FRSSC_DYS;
        __I uint8_t HFOSC_FHCTL_STATE;
        __I uint8_t FRSSC_HFOSC_PRD;
        __I uint8_t SFSTR_HFOSC_PRD;
        uint8_t RESERVED ;
        __IO uint16_t HFOSC_FRSSC_UPLMT;
        __IO uint16_t HFOSC_FRSSC_DNLMT;
        __IO uint16_t HFOSC_FREQ_STR;
        __IO uint16_t HFOSC_FHSET;
    } field;
}ssc_con;

typedef union {
    uint8_t value;
    struct {
        __IO uint8_t SCAN_RSV0;
    } field;
} rsv_con0;



/**
  * @brief EFUSE (EFUSE)
  */

typedef struct {                                /*!< (@ 0x420C0000) EFUSE Structure                                            */
  __IM  uint32_t  RESERVED[195];

  union {
    __IOM uint32_t M_ANA_CFG_FT_BTRF1;          /*!< (@ 0x0000030C) M_ANA_CFG_FT_BTRF1                                         */

    struct {
            uint32_t            : 12;
      __IM  uint32_t VBG_CAL    : 7;            /*!< [18..12] VBG_CAL                                                          */
            uint32_t            : 13;
    } M_ANA_CFG_FT_BTRF1_b;
  } ;
  __IM  uint32_t  RESERVED1[62];

  union {
    __IOM uint32_t M_ANA_CFG_UPLL_IBANK;        /*!< (@ 0x00000408) M_ANA_CFG_UPLL_IBANK                                       */

    struct {
      __IM  uint32_t UPLL_IBANK_EN : 1;         /*!< [0..0] UPLL_IBANK_EN                                                      */
      __IM  uint32_t UPLL_IBANK_FINETUNE : 4;   /*!< [4..1] UPLL_IBANK_FINETUNE                                                */
      __IM  uint32_t UPLL_KVCO_SEL : 3;         /*!< [7..5] UPLL_KVCO_SEL                                                      */
            uint32_t            : 24;
    } M_ANA_CFG_UPLL_IBANK_b;
  } ;
} EFUSE_Type;                                   /*!< Size = 1036 (0x40c)                                                       */




/* =========================================================================================================================== */
/* ================                                         DCXO_CFG                                          ================ */
/* =========================================================================================================================== */



/**
  * @brief DCXO_CFG (DCXO_CFG)
  */

typedef struct {                                /*!< (@ 0x430A0000) DCXO_CFG Structure                                         */
  __IM  uint32_t  RESERVED[2];
  
  union {
    __IOM uint32_t DCXO_CTUNE_FPM;              /*!< (@ 0x00000008) DCXO_CTUNE_FPM                                             */
    
    struct {
      __IOM uint32_t DCXO_CDAC_FPM : 9;         /*!< [8..0] DCXO_CDAC_FPM                                                      */
            uint32_t            : 23;
    } DCXO_CTUNE_FPM_b;
  } ;
  
  union {
    __IOM uint32_t DCXO_CTUNE_LPM;              /*!< (@ 0x0000000C) DCXO_CTUNE_LPM                                             */
    
    struct {
      __IOM uint32_t DCXO_CDAC_LPM : 9;         /*!< [8..0] DCXO_CDAC_LPM                                                      */
            uint32_t            : 23;
    } DCXO_CTUNE_LPM_b;
  } ;
  __IM  uint32_t  RESERVED1[2];
  
  union {
    __IOM uint32_t DCXO_CDAC_ST2;               /*!< (@ 0x00000018) DCXO_CDAC_ST2                                              */
    
    struct {
      __IOM uint32_t DCXO_CDAC_G2_ST : 9;       /*!< [8..0] DCXO_CDAC_G2_ST                                                    */
            uint32_t            : 23;
    } DCXO_CDAC_ST2_b;
  } ;
  
  union {
    __IOM uint32_t DCXO_CDAC_ST3;               /*!< (@ 0x0000001C) DCXO_CDAC_ST3                                              */
    
    struct {
      __IOM uint32_t DCXO_CDAC_G3_ST : 9;       /*!< [8..0] DCXO_CDAC_G3_ST                                                    */
            uint32_t            : 23;
    } DCXO_CDAC_ST3_b;
  } ;
  
  union {
    __IOM uint32_t DCXO_CDAC_ST4;               /*!< (@ 0x00000020) DCXO_CDAC_ST4                                              */
    
    struct {
      __IOM uint32_t DCXO_CDAC_G4_ST : 9;       /*!< [8..0] DCXO_CDAC_G4_ST                                                    */
            uint32_t            : 23;
    } DCXO_CDAC_ST4_b;
  } ;
  
  union {
    __IOM uint32_t DCXO_CDAC_ST5;               /*!< (@ 0x00000024) DCXO_CDAC_ST5                                              */
    
    struct {
      __IOM uint32_t DCXO_CDAC_G5_ST : 9;       /*!< [8..0] DCXO_CDAC_G5_ST                                                    */
            uint32_t            : 23;
    } DCXO_CDAC_ST5_b;
  } ;
  
  union {
    __IOM uint32_t DCXO_CORE_ISEL_1;            /*!< (@ 0x00000028) DCXO_CORE_ISEL_1                                           */
    
    struct {
      __IOM uint32_t DCXO_CORE_ISEL_FPM_G4 : 4; /*!< [3..0] DCXO_CORE_ISEL_FPM_G4                                              */
      __IOM uint32_t DCXO_CORE_ISEL_FPM_G3 : 4; /*!< [7..4] DCXO_CORE_ISEL_FPM_G3                                              */
      __IOM uint32_t DCXO_CORE_ISEL_FPM_G2 : 4; /*!< [11..8] DCXO_CORE_ISEL_FPM_G2                                             */
      __IOM uint32_t DCXO_CORE_ISEL_STARTUP : 4;/*!< [15..12] DCXO_CORE_ISEL_STARTUP                                           */
            uint32_t            : 16;
    } DCXO_CORE_ISEL_1_b;
  } ;
  
  union {
    __IOM uint32_t DCXO_CORE_ISEL_2;            /*!< (@ 0x0000002C) DCXO_CORE_ISEL_2                                           */
    
    struct {
            uint32_t            : 4;
      __IOM uint32_t DCXO_CORE_ISEL_LPM : 4;    /*!< [7..4] DCXO_CORE_ISEL_LPM                                                 */
      __IOM uint32_t DCXO_CORE_ISEL_FPM_G6 : 4; /*!< [11..8] DCXO_CORE_ISEL_FPM_G6                                             */
      __IOM uint32_t DCXO_CORE_ISEL_FPM_G5 : 4; /*!< [15..12] DCXO_CORE_ISEL_FPM_G5                                            */
            uint32_t            : 16;
    } DCXO_CORE_ISEL_2_b;
  } ;
  
  union {
    __IOM uint32_t BGCORE_CTRL0;                /*!< (@ 0x00000030) BGCORE_CTRL0                                               */
    
    struct {
            uint32_t            : 1;
      __IOM uint32_t BG_FC_OFF  : 1;            /*!< [1..1] BG_FC_OFF                                                          */
      __IOM uint32_t DCXO_LDO_BIAS_EN : 1;      /*!< [2..2] DCXO_LDO_BIAS_EN                                                   */
      __IOM uint32_t BG_EN      : 1;            /*!< [3..3] BG_EN                                                              */
            uint32_t            : 4;
      __IOM uint32_t VREF_V2I_SEL : 6;          /*!< [13..8] VREF_V2I_SEL                                                      */
            uint32_t            : 18;
    } BGCORE_CTRL0_b;
  } ;
  
  union {
    __IOM uint32_t BGCORE_CTRL1;                /*!< (@ 0x00000034) BGCORE_CTRL1                                               */
    
    struct {
            uint32_t            : 8;
      __IOM uint32_t VREF_DCXO_SEL : 6;         /*!< [13..8] VREF_DCXO_SEL                                                     */
            uint32_t            : 18;
    } BGCORE_CTRL1_b;
  } ;
  __IM  uint32_t  RESERVED2[2];
  
  union {
    __IOM uint32_t DCXO_FPM_LDO2;               /*!< (@ 0x00000040) DCXO_FPM_LDO2                                              */
    
    struct {
      __IOM uint32_t FULL_REG   : 16;           /*!< [15..0] FULL_REG                                                          */
            uint32_t            : 16;
    } DCXO_FPM_LDO2_b;
  } ;
  
  union {
    __IOM uint32_t DCXO_FPM_LDO3;               /*!< (@ 0x00000044) DCXO_FPM_LDO3                                              */
    
    struct {
      __IOM uint32_t FULL_REG   : 16;           /*!< [15..0] FULL_REG                                                          */
            uint32_t            : 16;
    } DCXO_FPM_LDO3_b;
  } ;
  
  union {
    __IOM uint32_t DCXO_FPM_LDO4;               /*!< (@ 0x00000048) DCXO_FPM_LDO4                                              */
    
    struct {
      __IOM uint32_t FULL_REG   : 16;           /*!< [15..0] FULL_REG                                                          */
            uint32_t            : 16;
    } DCXO_FPM_LDO4_b;
  } ;
  
  union {
    __IOM uint32_t BBPLL_XOPS;                  /*!< (@ 0x0000004C) BBPLL_XOPS                                                 */
    
    struct {
            uint32_t            : 7;
      __IOM uint32_t BBPLL_XOPS_EN_PO_MAN : 1;  /*!< [7..7] BBPLL_XOPS_EN_PO_MAN                                               */
            uint32_t            : 24;
    } BBPLL_XOPS_b;
  } ;
  
  union {
    __IOM uint32_t DCXO_XOPS;                   /*!< (@ 0x00000050) DCXO_XOPS                                                  */
    
    struct {
            uint32_t            : 4;
      __IOM uint32_t RG_DCXO_XOPS_START : 1;    /*!< [4..4] RG_DCXO_XOPS_START                                                 */
            uint32_t            : 27;
    } DCXO_XOPS_b;
  } ;
  
  union {
    __IOM uint32_t DCXO_LPM_LDO;                /*!< (@ 0x00000054) DCXO_LPM_LDO                                               */
    
    struct {
      __IOM uint32_t FULL_REG   : 16;           /*!< [15..0] FULL_REG                                                          */
            uint32_t            : 16;
    } DCXO_LPM_LDO_b;
  } ;
  __IM  uint32_t  RESERVED3;
  
  union {
    __IOM uint32_t DCXO_32KDIV_LPM;             /*!< (@ 0x0000005C) DCXO_32KDIV_LPM                                            */
    
    struct {
      __IOM uint32_t FULL_REG   : 16;           /*!< [15..0] FULL_REG                                                          */
            uint32_t            : 16;
    } DCXO_32KDIV_LPM_b;
  } ;
  __IM  uint32_t  RESERVED4[15];
  
  union {
    __IOM uint32_t DCXO_MAN2;                   /*!< (@ 0x0000009C) DCXO_MAN2                                                  */
    
    struct {
      __IOM uint32_t FULL_REG   : 16;           /*!< [15..0] FULL_REG                                                          */
            uint32_t            : 16;
    } DCXO_MAN2_b;
  } ;
} DCXO_CFG_Type;                                /*!< Size = 160 (0xa0)                                                         */





void dvfs_pre_proc(uint8_t next_opp_idx);
void dvfs_post_proc(void);
void dvfs_switch_clock_freq(uint8_t next_opp_idx);
uint8_t clock_mux_cur_sel(clock_mux_sel_id mux_id);
void clock_dump_info(void);
void hal_clock_dcm_ctrl(void); /* DCM enable/disable (via internal compile option) */
uint32_t get_curr_cpu_freq_hz(void); /* Designed for SystemCoreClockUpdate() usage (returns current CPU freq in Hz) */
uint32_t hal_clock_get_freq_meter(hal_src_clock SRC_CLK,uint32_t winset);


hal_clock_status_t clock_mux_sel(clock_mux_sel_id mux_id, uint32_t mux_sel);
hal_clock_status_t hal_clock_chop_ck(uint32_t enable, uint32_t div_sel);

#ifdef HAL_DCXO_MODULE_ENABLED
dcxo_mode_t dcxo_current_mode(void);
void dcxo_32k_ctrl(uint8_t dcxo32k_en);   /* API for enable/disable of DCXO_32K */
void dcxo_lp_mode(dcxo_mode_t mode);
void dcxo_fpm_lock_ctrl(dcxo_fpm_lock_src_t lock_src, dcxo_fpm_lock_op_t lock_op);
uint8_t dcxo_fpm_lock_get_src(void);

void set_capid_rg(uint32_t capid_val);
uint32_t get_capid_rg(void);
int set_capid_nvdm(uint32_t capid_val);
uint32_t get_capid_nvdm(void);
void hal_dcxo_at_cmd(char *param_0, char *param_1, char *rsp_buf);
#endif
/*************************************************************************
 * Define customized function prototype
 *************************************************************************/

#endif /* HAL_CLOCK_MODULE_ENABLED */
#endif /* __HAL_CLOCK_INTERNAL_H__ */

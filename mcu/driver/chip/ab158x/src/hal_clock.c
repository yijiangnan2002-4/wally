/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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


#include "hal_clock_internal.h"
#ifdef HAL_CLOCK_MODULE_ENABLED

#include "hal_nvic_internal.h"
#include "hal_nvic.h"
#include <assert.h>
#include <stdio.h>
#include "hal_gpt.h"
#include "hal_flash_sf.h"
#include "hal.h"

#if defined(__EXT_BOOTLOADER__)
#include "bl_common.h"
#define clk_dbg_log(dbg_msg, param_cnt, ...)
#define clk_dbg_print(dbg_msg, param_cnt, ...) bl_print(LOG_DEBUG, dbg_msg, ##__VA_ARGS__)
#define clk_dbg_prt_warn(dbg_msg, param_cnt, ...)
#define clk_dbg_prt_err(dbg_msg, param_cnt, ...)
#elif defined(__EXT_DA__)
/* need to add da env corresponding debug log function call */
#define clk_dbg_log(dbg_msg, param_cnt, ...)
#define clk_dbg_print(dbg_msg, param_cnt, ...)
#define clk_dbg_prt_warn(dbg_msg, param_cnt, ...)
#define clk_dbg_prt_err(dbg_msg, param_cnt, ...)
#else
#include "syslog.h"
/* dbg_log, added in critical section, only enable when we need to debug stuff */
#define clk_dbg_log(dbg_msg, param_cnt, ...)
#define clk_dbg_print(dbg_msg, param_cnt, ...)     log_hal_msgid_info(dbg_msg, param_cnt, ##__VA_ARGS__)
#define clk_dbg_prt_warn(dbg_msg, param_cnt, ...)  log_hal_msgid_warning(dbg_msg, param_cnt, ##__VA_ARGS__)
#define clk_dbg_prt_err(dbg_msg, param_cnt, ...)   log_hal_msgid_error(dbg_msg, param_cnt, ##__VA_ARGS__)
#endif

#ifdef FREERTOS_ENABLE
#include "task.h"
#endif

#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
#include "battery_management.h"
#endif

/* Definitions to control LPOSC target calibration freq
 * LPOSC1: 312 MHz    (AIR_RFI_SUPPRESS_DISABLE defined)
 *         299.25 MHz (AIR_RFI_SUPPRESS_DISABLE NOT defined)
 * LPOSC2: 520 MHz
 */
#if defined(AIR_RFI_SUPPRESS_DISABLE)
#define LPOSC1_TARGET_KHZ   (uint32_t)(104 * 1000 * 3)    /* 312 MHz */
#else
#define LPOSC1_TARGET_KHZ   (uint32_t)(99.75 * 1000 * 3)  /* 299.25 MHz */
#endif /* AIR_RFI_SUPPRESS_DISABLE */
#define LPOSC2_TARGET_KHZ   (520000)            /* 520 MHz */

#define LPOSC1_HZ           (LPOSC1_TARGET_KHZ * 1000)
#define LPOSC2_HZ           (LPOSC2_TARGET_KHZ * 1000)
#define DCXO_HZ              26000000           /* 26 MHz */
#define APLL1_HZ             45158400           /* 45.1584 MHz */
#define APLL2_HZ             49152000           /* 49.152 MHz */
#define UPLL_HZ              312000000          /* 312 MHz */

/* Dynamic clock gating enable/disable definition (comment out to disable) */
#define DCM_ENABLE

#if defined(AIR_CLK_SW_TRACKING) && defined(SYSTEM_DAEMON_TASK_ENABLE)
/* AIR_CLK_SW_TRACKING currently relies on SYSTEM_DAEMON_TASK to be enabled
 * only enable CLK SW tracking when both are defined/exist
 */
#define CLK_SW_TRACKING_TASK_ENABLE
#endif

#if defined(CLK_SW_TRACKING_TASK_ENABLE)
#include "system_daemon.h"
/* definition to control the interval for checking if LPOSC needs to be recalibrated */
#define LPOSC_RECALI_INTERVAL_MS 3000
#define LPOSC_RG_FT_MAX  0xF
#define LPOSC_MONITOR_PRINT_TIMES_MAX    20

/* debug logs for code tracing */
#define clk_recali_dbg(dbg_msg, param_cnt, ...)
/* necessary logs for runtime info */
#define clk_recali_print(dbg_msg, param_cnt, ...)     log_hal_msgid_info("[CLK_TRACKING] "dbg_msg, param_cnt, ##__VA_ARGS__)

static uint32_t lposc_1st_time_k_fqmtr_data[2];
static uint8_t lposc_recali_monitor_print_times[2];
static uint8_t lposc_recali_cali_cur[2], lposc_recali_ft_cur[2];

void lposc_recali(void);
void lposc_recali_gpt_callback(void *arg);
#else
void lposc_recali(void) { }  /* for build pass if CLK_SW_TRACKING_TASK_ENABLE not defined */
#endif /* CLK_SW_TRACKING_TASK_ENABLE */

ATTR_RWDATA_IN_TCM static cali_info_t osc_tune_tbl[OSC_NUM][TUNE_TBL_SIZE] = {{{.scale = lposc_cali_scale, .src = AD_OSC1_D3_CK, .cali_target = (LPOSC1_TARGET_KHZ / 3)}, // OSC1
        {.scale = lposc_ft_scale,   .src = AD_OSC1_CK,   .cali_target = LPOSC1_TARGET_KHZ}
    },
    {   {.scale = lposc_cali_scale, .src = AD_OSC2_D3_CK, .cali_target = (LPOSC2_TARGET_KHZ / 3)}, //OSC2
        {.scale = lposc_ft_scale,   .src = AD_OSC2_CK,   .cali_target = LPOSC2_TARGET_KHZ}
    }
};

#define BYTE_REG(instance)      ((volatile uint8_t *)&instance)
void lposc1_enable(void);
//static void lposc1_disable(void);   /* Comment out lposc1_disable() code (currently not used) */
void lposc2_enable(void);
static void lposc2_disable(void);
static int32_t hal_phys_cg_ctrl(hal_clock_cg_id clock_id, cg_request_t request);

static uint8_t from_clock_enable_to_clock_mux_sel;

// ---------------------------------------------------------

#if 1
/* clock CG status debug function */
void dbg_clock_cg(void)
{
    /* XO_PDN_PD_COND0(0x42040B00) */
    clk_dbg_print("[CLK] XO_PDN_PD_COND0: 0x%x\r\n", 1, *XO_PDN_PD_COND0);
    /* XO_PDN_AO_COND0(0x42040B30) */
    clk_dbg_print("[CLK] XO_PDN_AO_COND0: 0x%x\r\n", 1, *XO_PDN_AO_COND0);
    /* XO_PDN_TOP_COND0(0x42040B60) */
    clk_dbg_print("[CLK] XO_PDN_TOP_COND0: 0x%x\r\n", 1, *XO_PDN_TOP_COND0);
    /* XO_PDN_TOP_COND1(0x42040B90) */
    clk_dbg_print("[CLK] XO_PDN_TOP_COND1: 0x%x\r\n", 1, *XO_PDN_TOP_COND1);
    /* PDN_PD_COND0(0x422B0300) */
    clk_dbg_print("[CLK] PDN_PD_COND0: 0x%x\r\n", 1, *PDN_PD_COND0);
    /* PDN_AO_COND0(0x422B0330) */
    clk_dbg_print("[CLK] PDN_AO_COND0: 0x%x\r\n", 1, *PDN_AO_COND0);
    /* PDN_TOP_COND0(0x422B0360) */
    clk_dbg_print("[CLK] PDN_TOP_COND0: 0x%x\r\n", 1, *PDN_TOP_COND0);
}
#endif
const static clock_tbl_t clock_tbl;
/* Psuedo_CG, set to all default Clock gating default enabled for now
 * should be set to enabled after 1st MUX_SEL call
 * Used for indicating if clock source is in-use
 * (must match psuedo cg list sequence defined in hal_clock_cg_id)
 */
ATTR_RWDATA_IN_TCM static uint8_t psuedo_cg_arr[HAL_CLOCK_CG_END - HAL_CLOCK_CG_PSUEDO_CLK_26M] = {
    1, // HAL_CLOCK_CG_PSUEDO_CLK_26M
    1, // HAL_CLOCK_CG_PSUEDO_CLK_OSC_26M
    1, // HAL_CLOCK_CG_PSUEDO_MCLK
};

/* array for keeping psuedo mux select option
 * MCLK: intialized to 2 to represent default clock source DCXO 26MHz
 * (must match clock_mux_sel_id list sequence defined in )
 */

ATTR_RWDATA_IN_TCM static uint8_t psuedo_mux_arr[CLK_PSUEDO_MUX_NUM] = {2, // CLK_MCLK_SEL
                                                                        0, // CLK_SPDIF_SEL
                                                                        0, // CLK_PWM0_SEL
                                                                        0, // CLK_PWM1_SEL
                                                                        0, // CLK_PWM2_SEL
                                                                        0, // CLK_PWM3_SEL
                                                                        0, // CLK_PWM4_SEL
                                                                        0, // CLK_PWM5_SEL
                                                                        0, // CLK_PWM6_SEL
                                                                        0, // CLK_PWM7_SEL
                                                                        0, // CLK_PWM8_SEL
                                                                        0, // CLK_PWM9_SEL
                                                                        0, // CLK_PWM10_SEL
                                                                        0, // CLK_PWM11_SEL
                                                                        0, // CLK_PWM12_SEL
                                                                        0  // CLK_PWM13_SEL
                                                                       };


static uint8_t clk_pll_on(clock_pll_id pll_id);
static uint8_t clk_pll_off(clock_pll_id pll_id);
ATTR_RWDATA_IN_TCM static clk_apll_freq apll_freq[NR_APLL] = {CLK_APLL_45M, CLK_APLL_49M};


uint8_t clock_set_pll_on(clock_pll_id pll_id)
{
    return 1;
}

uint8_t clock_set_pll_off(clock_pll_id pll_id)
{
    return 1;
}

clk_usr_status clock_get_apll1_status()
{
    //return APLL1_DOMAIN_USR_EXIST;
    return exist;
}

clk_usr_status clock_get_apll2_status()
{
    //return APLL2_DOMAIN_USR_EXIST;
    return exist;
}

clk_usr_status clock_get_upll_status()
{
    //return UPLL_DOMAIN_USR_EXIST;
    return exist;
}

clk_usr_status clock_get_lposc1_status()
{
    // return LPOSC1_DOMAIN_USR_EXIST;
    return exist;
}

clk_usr_status clock_get_lposc2_status()
{
    // return LPOSC2_DOMAIN_USR_EXIST;
    return exist;
}

ATTR_TEXT_IN_TCM static void apll1_enable(void)
{
    clk_pll_on(CLK_APLL1);
}
ATTR_TEXT_IN_TCM static void apll1_disable(void)
{
    clk_pll_off(CLK_APLL1);
}
ATTR_TEXT_IN_TCM static void apll2_enable(void)
{
    clk_pll_on(CLK_APLL2);
}
ATTR_TEXT_IN_TCM static void apll2_disable(void)
{
    clk_pll_off(CLK_APLL2);
}
ATTR_TEXT_IN_TCM static void upll_disable(void)
{
    clk_pll_off(CLK_UPLL);
}
ATTR_TEXT_IN_TCM static void upll_enable(void)
{
    clk_pll_on(CLK_UPLL);
}


/* Performs mux switch for all physical and psuedo mux */
ATTR_TEXT_IN_TCM static void clock_top_mux_ctrl(clock_mux_sel_id mux_id, uint8_t target_sel)
{
    volatile uint8_t *sel = NULL;
    volatile uint8_t *chg = NULL;
    volatile uint8_t *chg_ok = NULL;
    volatile uint8_t *force_on = NULL;
    switch (mux_id) {
        case CLK_SYS_SEL:
            sel =      BYTE_REG(CKSYS_CLK_CFG_REG.CLK_SYS_SEL);
            force_on = BYTE_REG(CKSYS_CLK_FORCE_ON_REG.CLK_SYS_FORCE_ON);
            chg =      BYTE_REG(CKSYS_CLK_UPDATE_REG.CHG_SYS);
            chg_ok =   BYTE_REG(CKSYS_CLK_UPDATE_STATUS_REG.CLK_SYS_OK);
            break;
        case CLK_DSP_SEL:
            sel =      BYTE_REG(CKSYS_CLK_CFG_REG.CLK_DSP_SEL);
            force_on = BYTE_REG(CKSYS_CLK_FORCE_ON_REG.CLK_DSP_FORCE_ON);
            chg =      BYTE_REG(CKSYS_CLK_UPDATE_REG.CHG_DSP);
            chg_ok =   BYTE_REG(CKSYS_CLK_UPDATE_STATUS_REG.CLK_DSP_OK);
            break;
        case CLK_SFC_SEL:
            sel =      BYTE_REG(CKSYS_CLK_CFG_REG.CLK_SFC_SEL);
            force_on = BYTE_REG(CKSYS_CLK_FORCE_ON_REG.CLK_SFC_FORCE_ON);
            chg =      BYTE_REG(CKSYS_CLK_UPDATE_REG.CHG_SFC);
            chg_ok =   BYTE_REG(CKSYS_CLK_UPDATE_STATUS_REG.CLK_SFC_OK);
            break;
        case CLK_ESC_SEL:
            sel =      BYTE_REG(CKSYS_CLK_CFG_REG.CLK_ESC_SEL);
            force_on = BYTE_REG(CKSYS_CLK_FORCE_ON_REG.CLK_ESC_FORCE_ON);
            chg =      BYTE_REG(CKSYS_CLK_UPDATE_REG.CHG_ESC);
            chg_ok =   BYTE_REG(CKSYS_CLK_UPDATE_STATUS_REG.CLK_ESC_OK);
            break;
        case CLK_SPIMST0_SEL:
            sel =      BYTE_REG(CKSYS_CLK_CFG_REG.CLK_SPIMST0_SEL);
            force_on = BYTE_REG(CKSYS_CLK_FORCE_ON_REG.CLK_SPIMST0_FORCE_ON);
            chg =      BYTE_REG(CKSYS_CLK_UPDATE_REG.CHG_SPIMST0);
            chg_ok =   BYTE_REG(CKSYS_CLK_UPDATE_STATUS_REG.CLK_SPIMST0_OK);
            break;
        case CLK_SPIMST1_SEL:
            sel =      BYTE_REG(CKSYS_CLK_CFG_REG.CLK_SPIMST1_SEL);
            force_on = BYTE_REG(CKSYS_CLK_FORCE_ON_REG.CLK_SPIMST1_FORCE_ON);
            chg =      BYTE_REG(CKSYS_CLK_UPDATE_REG.CHG_SPIMST1);
            chg_ok =   BYTE_REG(CKSYS_CLK_UPDATE_STATUS_REG.CLK_SPIMST1_OK);
            break;
        case CLK_SPIMST2_SEL:
            sel =      BYTE_REG(CKSYS_CLK_CFG_REG.CLK_SPIMST2_SEL);
            force_on = BYTE_REG(CKSYS_CLK_FORCE_ON_REG.CLK_SPIMST2_FORCE_ON);
            chg =      BYTE_REG(CKSYS_CLK_UPDATE_REG.CHG_SPIMST2);
            chg_ok =   BYTE_REG(CKSYS_CLK_UPDATE_STATUS_REG.CLK_SPIMST2_OK);
            break;
        case CLK_SPISLV_SEL:
            sel =      BYTE_REG(CKSYS_CLK_CFG_REG.CLK_SPISLV_SEL);
            force_on = BYTE_REG(CKSYS_CLK_FORCE_ON_REG.CLK_SPISLV_FORCE_ON);
            chg =      BYTE_REG(CKSYS_CLK_UPDATE_REG.CHG_SPISLV);
            chg_ok =   BYTE_REG(CKSYS_CLK_UPDATE_STATUS_REG.CLK_SPISLV_OK);
            break;
        case CLK_SDIOMST0_SEL:
            sel =      BYTE_REG(CKSYS_CLK_CFG_REG.CLK_SDIOMST0_SEL);
            force_on = BYTE_REG(CKSYS_CLK_FORCE_ON_REG.CLK_SDIOMST0_FORCE_ON);
            chg =      BYTE_REG(CKSYS_CLK_UPDATE_REG.CHG_SDIOMST0);
            chg_ok =   BYTE_REG(CKSYS_CLK_UPDATE_STATUS_REG.CLK_SDIOMST0_OK);
            break;
        case CLK_USB_SEL:
            sel =      BYTE_REG(CKSYS_CLK_CFG_REG.CLK_USB_SEL);
            force_on = BYTE_REG(CKSYS_CLK_FORCE_ON_REG.CLK_USB_FORCE_ON);
            chg =      BYTE_REG(CKSYS_CLK_UPDATE_REG.CHG_USB);
            chg_ok =   BYTE_REG(CKSYS_CLK_UPDATE_STATUS_REG.CLK_USB_OK);
            break;
        case CLK_I3C_SEL:
            sel =      BYTE_REG(CKSYS_CLK_CFG_REG.CLK_I3C_SEL);
            force_on = BYTE_REG(CKSYS_CLK_FORCE_ON_REG.CLK_I3C_FORCE_ON);
            chg =      BYTE_REG(CKSYS_CLK_UPDATE_REG.CHG_I3C);
            chg_ok =   BYTE_REG(CKSYS_CLK_UPDATE_STATUS_REG.CLK_I3C_OK);
            break;
        case CLK_BT_HOP_SEL:
            sel =      BYTE_REG(CKSYS_CLK_CFG_REG.CLK_BT_HOP_SEL);
            force_on = BYTE_REG(CKSYS_CLK_FORCE_ON_REG.CLK_BT_HOP_FORCE_ON);
            chg =      BYTE_REG(CKSYS_CLK_UPDATE_REG.CHG_BT_HOP);
            chg_ok =   BYTE_REG(CKSYS_CLK_UPDATE_STATUS_REG.CLK_BT_HOP_OK);
            break;
        case CLK_AUD_BUS_SEL:
            sel =      BYTE_REG(CKSYS_CLK_CFG_REG.CLK_AUD_BUS_SEL);
            force_on = BYTE_REG(CKSYS_CLK_FORCE_ON_REG.CLK_AUD_BUS_FORCE_ON);
            chg =      BYTE_REG(CKSYS_CLK_UPDATE_REG.CHG_AUD_BUS);
            chg_ok =   BYTE_REG(CKSYS_CLK_UPDATE_STATUS_REG.CLK_AUD_BUS_OK);
            break;
        case CLK_AUD_GPSRC_SEL:
            sel =      BYTE_REG(CKSYS_CLK_CFG_REG.CLK_AUD_GPSRC_SEL);
            force_on = BYTE_REG(CKSYS_CLK_FORCE_ON_REG.CLK_AUD_GPSRC_FORCE_ON);
            chg =      BYTE_REG(CKSYS_CLK_UPDATE_REG.CHG_AUD_GPSRC);
            chg_ok =   BYTE_REG(CKSYS_CLK_UPDATE_STATUS_REG.CLK_AUD_GPSRC_OK);
            break;
        case CLK_AUD_ULCK_SEL:
            sel =      BYTE_REG(CKSYS_CLK_CFG_REG.CLK_AUD_ULCK_SEL);
            force_on = BYTE_REG(CKSYS_CLK_FORCE_ON_REG.CLK_AUD_ULCK_FORCE_ON);
            chg =      BYTE_REG(CKSYS_CLK_UPDATE_REG.CHG_AUD_ULCK);
            chg_ok =   BYTE_REG(CKSYS_CLK_UPDATE_STATUS_REG.CLK_AUD_ULCK_OK);
            break;
        case CLK_AUD_DLCK_SEL:
            sel =      BYTE_REG(CKSYS_CLK_CFG_REG.CLK_AUD_DLCK_SEL);
            force_on = BYTE_REG(CKSYS_CLK_FORCE_ON_REG.CLK_AUD_DLCK_FORCE_ON);
            chg =      BYTE_REG(CKSYS_CLK_UPDATE_REG.CHG_AUD_DLCK);
            chg_ok =   BYTE_REG(CKSYS_CLK_UPDATE_STATUS_REG.CLK_AUD_DLCK_OK);
            break;

        case CLK_AUD_INTERFACE0_SEL:
            sel =      BYTE_REG(CKSYS_CLK_CFG_REG.CLK_AUD_INTF0_SEL);
            break;
        case CLK_AUD_INTERFACE1_SEL:
            sel =      BYTE_REG(CKSYS_CLK_CFG_REG.CLK_AUD_INTF1_SEL);
            break;
        case CLK_26M_SEL:
            sel =      BYTE_REG(CKSYS_CLK_CFG_REG.CLK_26M_SEL);
            break;
        case CLK_VOW_SEL:
            sel =      BYTE_REG(CKSYS_CLK_CFG_REG.CLK_VOW_SEL);
            break;
        case CLK_AUD_ENGINE_SEL:
            sel =      BYTE_REG(CKSYS_CLK_CFG_REG.CLK_AUD_ENGINE_SEL);
            break;
        case CLK_OSC_26M_SEL:
            sel =      BYTE_REG(CKSYS_CLK_CFG_REG.CLK_OSC_26M_SEL);
            break;

        /* Below are psuedo muxes */
        case CLK_MCLK_SEL:
        case CLK_SPDIF_SEL:
        case CLK_PWM0_SEL:
        case CLK_PWM1_SEL:
        case CLK_PWM2_SEL:
        case CLK_PWM3_SEL:
        case CLK_PWM4_SEL:
        case CLK_PWM5_SEL:
        case CLK_PWM6_SEL:
        case CLK_PWM7_SEL:
        case CLK_PWM8_SEL:
        case CLK_PWM9_SEL:
        case CLK_PWM10_SEL:
        case CLK_PWM11_SEL:
        case CLK_PWM12_SEL:
        case CLK_PWM13_SEL:
            sel =      &psuedo_mux_arr[mux_id - CLK_MCLK_SEL];
            break;
        default:
            return;
    }
    // n to 1
    if (sel) {
        *sel = target_sel;
    }
    if (force_on) {
        *force_on = 1;
    }
#ifndef FPGA_ENV
    if (chg) {
        uint32_t counter = 0; /* counter for debugging when mux switch hangs */
        *chg = 1;
        while (*chg) {
            if (counter > 1000) {
                clk_dbg_print("while chg, counter %d!\r\n", 1, counter);
            }
            counter++;
        }
    }
    if (chg_ok) {
        while (*chg_ok) {
            uint32_t counter = 0; /* counter for debugging when mux switch hangs */
            if (counter > 1000) {
                clk_dbg_print("while chg_ok, counter %d!\r\n", 1, counter);
            }
            counter++;
        }
    }
#endif
    if (force_on) {
        *force_on = 0;
    }

#if !defined(__EXT_BOOTLOADER__) && !defined(__EXT_DA__)
    if (mux_id == CLK_SYS_SEL) {
        /* Notify FreeRTOS CPU's clk freq setting change */
        SystemCoreClockUpdate();
    }
#endif
}

/*************************************************************************
 * Clock mux select API definition part
 * 1. Enable individual clock divider
 * 2. Force clock on th prevent clock can't switch to target clock
 * 3. Set CSW to target clock freq. and set change bit
 * 4. After clock change to target freq. Change bit will be cleared to0 and release clock gating
 * 5. Disable forced on clock
 *************************************************************************/

/* Call corresponding clock source enable functions
 * If clock sources are already enabled, it should return directly
 */
ATTR_TEXT_IN_TCM static hal_clock_status_t clock_src_enable(hal_mux_t sels)
{
    switch (sels.src) {
        case CLK_XO:
            /* DCXO is currently enabled by default */
            break;
        case CLK_OSC1:
            lposc1_enable();
            break;
        case CLK_OSC2:
            lposc2_enable();
            break;
        case CLK_APLL1:
            apll1_enable();
            break;
        case CLK_APLL2:
            apll2_enable();
            break;
        case CLK_UPLL:
            upll_enable();
            break;
        default:
            break;
    }
    return HAL_CLOCK_STATUS_OK;
}


ATTR_TEXT_IN_TCM static hal_clock_status_t clock_div_enable(hal_mux_t sels)
{
    clk_dbg_log("clock_div_enable src %d, div %d\r\n", 2, sels.src, sels.div);
    /* Only LPOSC1, LPOSC2, and UPLL dividers are currently dynamically enable/disabled */
    if (sels.src == CLK_OSC1) {
        CKSYS_CLK_DIV_REG.CLK_OSC1_DIV_EN = 0x1;
        if (sels.div == CLK_DIV_D2) {
            CKSYS_CLK_DIV_REG.CLK_OSC1_D2_EN = 0x1;
        } else if (sels.div == CLK_DIV_D3) {
            CKSYS_CLK_DIV_REG.CLK_OSC1_D3_EN = 0x1;
        } else if (sels.div == CLK_DIV_D5) {
            CKSYS_CLK_DIV_REG.CLK_OSC1_D5_EN = 0x1;
        } else if (sels.div == CLK_DIV_D12) {
            CKSYS_CLK_DIV_REG.CLK_OSC1_D12_EN = 0x1;
        } else if (sels.div == CLK_DIV_D8) {
            CKSYS_CLK_DIV_REG.CLK_OSC1_D8_EN = 0x1;
        }
        /* CLK_DIV_NONE 0xFF means divide by 1 */
    } else if (sels.src == CLK_OSC2) {
        CKSYS_CLK_DIV_REG.CLK_OSC2_DIV_EN = 0x1;
        if (sels.div == CLK_DIV_D2) {
            CKSYS_CLK_DIV_REG.CLK_OSC2_D2_EN = 0x1;
        } else if (sels.div == CLK_DIV_D3) {
            CKSYS_CLK_DIV_REG.CLK_OSC2_D3_EN = 0x1;
        } else if (sels.div == CLK_DIV_D5) {
            CKSYS_CLK_DIV_REG.CLK_OSC2_D5_EN = 0x1;
        } else if (sels.div == CLK_DIV_D12) {
            CKSYS_CLK_DIV_REG.CLK_OSC2_D12_EN = 0x1;
        }
        /* CLK_DIV_NONE 0xFF means divide by 1 */
    } else if (sels.src == CLK_UPLL) {
        CKSYS_CLK_DIV_REG.CLK_PLL1_DIV_EN = 0x1;
        if (sels.div == CLK_DIV_D2) {
            CKSYS_CLK_DIV_REG.CLK_PLL1_D2_EN = 0x1;
        } else if (sels.div == CLK_DIV_D3) {
            CKSYS_CLK_DIV_REG.CLK_PLL1_D3_EN = 0x1;
        } else if (sels.div == CLK_DIV_D5) {
            CKSYS_CLK_DIV_REG.CLK_PLL1_D5_EN = 0x1;
        }
        /* CLK_DIV_NONE 0xFF means UPLL 624M for now */
    }
    return HAL_CLOCK_STATUS_OK;
}


ATTR_TEXT_IN_TCM static hal_clock_status_t clock_div_disable(hal_mux_t sels)
{
    clk_dbg_log("clock_div_disable src %d, div %d\r\n", 2, sels.src, sels.div);
    /* Only LPOSC1, LPOSC2, and UPLL dividers are currently dynamically enable/disabled
     * master divider enable/disable is not disabled for now
     */
    if (sels.src == CLK_OSC1) {
        //CKSYS_CLK_DIV_REG.CLK_OSC1_DIV_EN = 0x0;
        if (sels.div == CLK_DIV_D2) {
            CKSYS_CLK_DIV_REG.CLK_OSC1_D2_EN = 0x0;
        } else if (sels.div == CLK_DIV_D3) {
            CKSYS_CLK_DIV_REG.CLK_OSC1_D3_EN = 0x0;
        } else if (sels.div == CLK_DIV_D5) {
            CKSYS_CLK_DIV_REG.CLK_OSC1_D5_EN = 0x0;
        } else if (sels.div == CLK_DIV_D12) {
            CKSYS_CLK_DIV_REG.CLK_OSC1_D12_EN = 0x0;
        } else if (sels.div == CLK_DIV_D8) {
            CKSYS_CLK_DIV_REG.CLK_OSC1_D8_EN = 0x0;
        }
        /* CLK_DIV_NONE 0xFF means divide by 1 */
    } else if (sels.src == CLK_OSC2) {
        //CKSYS_CLK_DIV_REG.CLK_OSC2_DIV_EN = 0x0;
        if (sels.div == CLK_DIV_D2) {
            CKSYS_CLK_DIV_REG.CLK_OSC2_D2_EN = 0x0;
        } else if (sels.div == CLK_DIV_D3) {
            CKSYS_CLK_DIV_REG.CLK_OSC2_D3_EN = 0x0;
        } else if (sels.div == CLK_DIV_D5) {
            CKSYS_CLK_DIV_REG.CLK_OSC2_D5_EN = 0x0;
        } else if (sels.div == CLK_DIV_D12) {
            CKSYS_CLK_DIV_REG.CLK_OSC2_D12_EN = 0x0;
        }
        /* CLK_DIV_NONE 0xFF means divide by 1 */
    } else if (sels.src == CLK_UPLL) {
        //CKSYS_CLK_DIV_REG.CLK_PLL1_DIV_EN = 0x0;
        if (sels.div == CLK_DIV_D2) {
            CKSYS_CLK_DIV_REG.CLK_PLL1_D2_EN = 0x0;
        } else if (sels.div == CLK_DIV_D3) {
            CKSYS_CLK_DIV_REG.CLK_PLL1_D3_EN = 0x0;
        } else if (sels.div == CLK_DIV_D5) {
            CKSYS_CLK_DIV_REG.CLK_PLL1_D5_EN = 0x0;
        }
        /* CLK_DIV_NONE 0xFF means UPLL 624M for now */
    }
    return HAL_CLOCK_STATUS_OK;
}

/* Make sure current, and next clock source/divder have been enabled */
ATTR_TEXT_IN_TCM static hal_clock_status_t mux_switch_preproc(clock_mux_sel_id mux_id, uint32_t next_mux_sel)
{
    //clk_dbg_log("mux_switch_preproc\r\n", 0);
    /* N-to-1 mux requires both original, and next clock source/dividers both be enabled */
    if (clock_tbl.mux_tbl[mux_id].mux_type == PHYSICAL_N_TO_1_MUX) {
        /* enable current clock source, divider */
        uint8_t curr_sel = *(volatile uint8_t *)(clock_tbl.mux_tbl[mux_id].curr_sel_ptr);
        hal_mux_t curr_mux_info = clock_tbl.mux_tbl[mux_id].sels[curr_sel];
        clock_src_enable(curr_mux_info);
        clock_div_enable(curr_mux_info);
    }

    /* enable next clock source, divider */
    hal_mux_t next_mux_info = clock_tbl.mux_tbl[mux_id].sels[next_mux_sel];
    clock_src_enable(next_mux_info);
    clock_div_enable(next_mux_info);

    /* process psuedo CG */
    if (clock_tbl.mux_tbl[mux_id].cg_info.cg_type == PSUEDO_CG) {
        /* nobody actually calls enable/disable psuedo_cg
         * just used as an indication whether or not clock resource need to be enable/disabled during post process
         * (PSUEDO CG is set to on(clock disabled by default), and will maintain on status for now)
         */
        if (clock_tbl.mux_tbl[mux_id].cg_info.cg_id > HAL_CLOCK_CG_PHYS_NUM && clock_tbl.mux_tbl[mux_id].cg_info.cg_id < HAL_CLOCK_CG_END) {
            uint32_t psuedo_cg_arr_idx = clock_tbl.mux_tbl[mux_id].cg_info.cg_id - HAL_CLOCK_CG_PSUEDO_CLK_26M;
            psuedo_cg_arr[psuedo_cg_arr_idx] = 0; /* disable psuedo cg */
        } else {
            /* check if psuedo cg_id is in expected range (report error if unexpected occurs) */
            clk_dbg_print("unexpected psuedo cg_id %d", 1, clock_tbl.mux_tbl[mux_id].cg_info.cg_id);
        }
    }

    return HAL_CLOCK_STATUS_OK;
}

/* Disable unused resources clock source/divider */
ATTR_TEXT_IN_TCM static hal_clock_status_t mux_switch_postproc(void)
{
    clk_dbg_log("mux_switch_postproc\r\n", 0);
    clock_resource_chk_t curr_chk = { .src_chk = { .lposc1 = 0, .lposc2 = 0, .upll = 0, .apll1 = 0, .apll2 = 0},
                                      .lposc1_div_chk = { .d2 = 0, .d3 = 0, .d5 = 0, .d12 = 0, .d8 = 0},
                                      .lposc2_div_chk = { .d2 = 0, .d3 = 0, .d5 = 0, .d12 = 0, .d8 = 0},
                                      .upll_div_chk = { .d2 = 0, .d3 = 0, .d5 = 0, .d12 = 0, .d8 = 0},
                                    };
    /* loop through whole table, and check if each clk source, divider have any users
     * We only need to know if each divider, clock source have any users (we don't need the actual count)
     * Note: CG status is also checked, if CG is enabled, we wont increment count during reference count
     */
    /* Check if the clock_id has a corresponding mux that need to be switched
     * If one exists -> call mux_sel to trigger clock source/divider enable etc..
     * TODO: find a way to shorten the below code
     */
    for (uint32_t mux_tbl_idx = 0; mux_tbl_idx < TOTAL_MUX_CNT; mux_tbl_idx++) {
        uint8_t curr_sel_num = *((volatile uint8_t *)clock_tbl.mux_tbl[mux_tbl_idx].curr_sel_ptr); /* ptr that points to current selected num */
        hal_cg_t cg_info = clock_tbl.mux_tbl[mux_tbl_idx].cg_info;
        if (hal_clock_is_enabled(cg_info.cg_id)) {
            /* clock is enabled, start to check clock source, divider */
            hal_mux_t sels = clock_tbl.mux_tbl[mux_tbl_idx].sels[curr_sel_num];
            if (sels.src == CLK_OSC1) {
                //curr_chk.src_chk.lposc1 = 1;
                //CKSYS_CLK_DIV_REG.CLK_OSC1_DIV_EN = 0x0;
                if (sels.div == CLK_DIV_D2) {
                    curr_chk.lposc1_div_chk.d2 = 1;
                } else if (sels.div == CLK_DIV_D3) {
                    curr_chk.lposc1_div_chk.d3 = 1;
                } else if (sels.div == CLK_DIV_D5) {
                    curr_chk.lposc1_div_chk.d5 = 1;
                } else if (sels.div == CLK_DIV_D12) {
                    curr_chk.lposc1_div_chk.d12 = 1;
                } else if (sels.div == CLK_DIV_D8) {
                    curr_chk.lposc1_div_chk.d8 = 1;
                }
                /* CLK_DIV_NONE 0xFF means divide by 1 */
            } else if (sels.src == CLK_OSC2) {
                curr_chk.src_chk.lposc2 = 1;
                //CKSYS_CLK_DIV_REG.CLK_OSC2_DIV_EN = 0x0;
                if (sels.div == CLK_DIV_D2) {
                    curr_chk.lposc2_div_chk.d2 = 1;
                } else if (sels.div == CLK_DIV_D3) {
                    curr_chk.lposc2_div_chk.d3 = 1;
                } else if (sels.div == CLK_DIV_D5) {
                    curr_chk.lposc2_div_chk.d5 = 1;
                } else if (sels.div == CLK_DIV_D12) {
                    curr_chk.lposc2_div_chk.d12 = 1;
                }
                /* CLK_DIV_NONE 0xFF means divide by 1 */
            } else if (sels.src == CLK_UPLL) {
                curr_chk.src_chk.upll = 1;
                if (sels.div == CLK_DIV_D2) {
                    curr_chk.upll_div_chk.d2 = 1;
                } else if (sels.div == CLK_DIV_D3) {
                    curr_chk.upll_div_chk.d3 = 1;
                } else if (sels.div == CLK_DIV_D5) {
                    curr_chk.upll_div_chk.d5 = 1;
                }
                /* CLK_DIV_NONE 0xFF means UPLL 624M for now */
            } else if (sels.src == CLK_APLL1) {
                curr_chk.src_chk.apll1 = 1;
            } else if (sels.src == CLK_APLL2) {
                curr_chk.src_chk.apll2 = 1;
            }
        }
    }

    /* Use the above resource check results, and disable unused clock resource */
    /* LPOSC1 chk */
    //if (curr_chk.src_chk.lposc1 == 0) {
        /* Do not disable LPOSC1 for this generation of chips (for SPM clock control) */
    //}
    if (curr_chk.lposc1_div_chk.d2 == 0) {
        clock_div_disable((hal_mux_t) {
            .src = CLK_OSC1, .div = CLK_DIV_D2
        });
    }
    if (curr_chk.lposc1_div_chk.d3 == 0) {
        clock_div_disable((hal_mux_t) {
            .src = CLK_OSC1, .div = CLK_DIV_D3
        });
    }
    if (curr_chk.lposc1_div_chk.d5 == 0) {
        clock_div_disable((hal_mux_t) {
            .src = CLK_OSC1, .div = CLK_DIV_D5
        });
    }
    if (curr_chk.lposc1_div_chk.d12 == 0) {
        clock_div_disable((hal_mux_t) {
            .src = CLK_OSC1, .div = CLK_DIV_D12
        });
    }
    if (curr_chk.lposc1_div_chk.d8 == 0) {
        clock_div_disable((hal_mux_t) {
            .src = CLK_OSC1, .div = CLK_DIV_D8
        });
    }

    /* LPOSC2 chk */
    if (curr_chk.src_chk.lposc2 == 0) {
        lposc2_disable();
    }
    if (curr_chk.lposc2_div_chk.d2 == 0) {
        clock_div_disable((hal_mux_t) {
            .src = CLK_OSC2, .div = CLK_DIV_D2
        });
    }
    if (curr_chk.lposc2_div_chk.d3 == 0) {
        clock_div_disable((hal_mux_t) {
            .src = CLK_OSC2, .div = CLK_DIV_D3
        });
    }
    if (curr_chk.lposc2_div_chk.d5 == 0) {
        clock_div_disable((hal_mux_t) {
            .src = CLK_OSC2, .div = CLK_DIV_D5
        });
    }
    if (curr_chk.lposc2_div_chk.d12 == 0) {
        clock_div_disable((hal_mux_t) {
            .src = CLK_OSC2, .div = CLK_DIV_D12
        });
    }

    /* UPLL chk */
    if (curr_chk.src_chk.upll == 0) {
        upll_disable();
    }
    if (curr_chk.upll_div_chk.d2 == 0) {
        clock_div_disable((hal_mux_t) {
            .src = CLK_UPLL, .div = CLK_DIV_D2
        });
    }
    if (curr_chk.upll_div_chk.d3 == 0) {
        clock_div_disable((hal_mux_t) {
            .src = CLK_UPLL, .div = CLK_DIV_D3
        });
    }
    if (curr_chk.upll_div_chk.d5 == 0) {
        clock_div_disable((hal_mux_t) {
            .src = CLK_UPLL, .div = CLK_DIV_D5
        });
    }
    /* APLL1 chk */
    if (curr_chk.src_chk.apll1 == 0) {
        apll1_disable();
    }
    /* APLL2 chk */
    if (curr_chk.src_chk.apll2 == 0) {
        apll2_disable();
    }
    return HAL_CLOCK_STATUS_OK;
}

ATTR_TEXT_IN_TCM hal_clock_status_t clock_mux_sel(clock_mux_sel_id mux_id, uint32_t mux_sel)
{
    hal_clock_status_t result = HAL_CLOCK_STATUS_OK;

#ifdef FREERTOS_ENABLE
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        if (from_clock_enable_to_clock_mux_sel) {  /* from hal_clock_enable, don't print log */
            from_clock_enable_to_clock_mux_sel = 0;
        }
        else {
            clk_dbg_print("clock_mux_sel(%d,%d), Caller 0x%08X", 3, mux_id, mux_sel, (uint32_t)__builtin_return_address(0));
        }
    }
#else
    clk_dbg_log("clock_mux_sel(%d,%d)\r\n", 2, mux_id, mux_sel);
#endif

    /* Need to check if next mux is allowed for switching */
    if (clock_tbl.mux_tbl[mux_id].sels[mux_sel].mux_allow == MUX_NOT_ALLOW) {
        clk_dbg_prt_err("clock_mux_sel(%d,%d) not allowed, Caller 0x%08X", 3, mux_id, mux_sel, (uint32_t)__builtin_return_address(0));
        return result;
    }
#if !defined(__EXT_BOOTLOADER__) && !defined(__EXT_DA__)
    uint32_t irq_mask = 0;
    hal_nvic_save_and_set_interrupt_mask_special(&irq_mask);
    /* ================ Critical Section Start ======================== */
#endif
    /* Make sure current, and next clock source/divder have been enabled */
    mux_switch_preproc(mux_id, mux_sel);

    /* perform mux switch */
    clock_top_mux_ctrl(mux_id, mux_sel);

    /* Disable unused resources clock source/divider */
    mux_switch_postproc();

#if !defined(__EXT_BOOTLOADER__) && !defined(__EXT_DA__)
    /* ================ Critical Section End ======================== */
    hal_nvic_restore_interrupt_mask_special(irq_mask);
#endif
    return result;
}


/* returns current selected mux num */
ATTR_TEXT_IN_TCM uint8_t clock_mux_cur_sel(clock_mux_sel_id mux_id)
{
    if (mux_id < TOTAL_MUX_CNT) {
        uint32_t mux_tbl_idx = mux_id; /* tbl_idx actually equals mux_id */
        uint8_t curr_sel = *(volatile uint8_t *)(clock_tbl.mux_tbl[mux_tbl_idx].curr_sel_ptr);
        return curr_sel;
    } else {
        clk_dbg_print("clock_mux_cur_sel() incorrect parameter, mux_id %d\r\n", 1, mux_id);
        return 0;
    }
}

ATTR_TEXT_IN_TCM uint32_t hal_clock_freq_meter_data(ref_clock REF_CLK, hal_src_clock SRC_CLK, uint32_t winset)
{
#if !defined(__EXT_BOOTLOADER__) && !defined(__EXT_DA__)
    uint32_t irq_mask;
    hal_nvic_save_and_set_interrupt_mask(&irq_mask);
#endif
    /* ================ Critical Section Start ======================== */
    PLL_ABIST_FQMTR_COM_REG.FQMTR_EN = 0x0;
    PLL_ABIST_FQMTR_COM_REG.FQMTR_RST = 0x1;
    PLL_ABIST_FQMTR_COM_REG.FQMTR_RST = 0x0;
    *CKSYS_TST_SEL_1 = (REF_CLK << 8) + SRC_CLK;
    PLL_ABIST_FQMTR_COM_REG.FQMTR_WINSET = winset;
    PLL_ABIST_FQMTR_COM_REG.FQMTR_EN = 0x1;

    /* ================ Critical Section End ======================== */
    #if !defined(__EXT_BOOTLOADER__) && !defined(__EXT_DA__)
        hal_nvic_restore_interrupt_mask(irq_mask);
    #endif

    uint32_t delay_time = 5;

    /* wait FQMTR start (at least 3T REF_CLK) */
    if (REF_CLK == EOSC_F32K) { /* Due to high variation of eosc 32KHz, the delay for 32KHz is set to 500us for now */
        delay_time = 500;
    }
    else if (REF_CLK == RTC_CK || REF_CLK == DCXO_F32K || REF_CLK == XOSC_F32K) {
        delay_time = 100;
    }
    hal_gpt_delay_us(delay_time);

#ifndef FPGA_ENV
    while (PLL_ABIST_FQMTR_COM_REG.PLL_ABIST_FQMTR_CON1_ & 0x80);
#endif

    return(*PLL_ABIST_FQMTR_DATA);
}

ATTR_TEXT_IN_TCM uint32_t clock_freq_meter(ref_clock REF_CLK, hal_src_clock SRC_CLK, uint32_t winset, uint32_t REF_CLK_frequency)
{
    uint32_t freq_meter_data;

    freq_meter_data = hal_clock_freq_meter_data(REF_CLK, SRC_CLK, winset);

    uint32_t freq_meter  = REF_CLK_frequency * (freq_meter_data) / (winset + 1);

    clk_dbg_log("freq_meter_data SRC_CLK %d, 0x%x(%d)\r\n", 3, freq_meter_data, SRC_CLK, freq_meter_data);
    return freq_meter;
}

#if 0
/* Estimate winset setting (currently returns accuracy of 0.01%) target winset value
 * Function is currently designed for 26MHz as reference clk
 */
ATTR_TEXT_IN_TCM static uint32_t fqmtr_winset_estimate(uint32_t target_freq_hz)
{
    uint32_t winset;
    /* data 40000: targets fqmtr to give an frequency measurement accuracy of 0.01% */
    const uint32_t data = 40000, src_freq_hz = 26 * 1000 * 1000;
    /* typecasted to 64 bit uint (to avoid multiplication overflow) */
    winset = (((uint64_t)src_freq_hz) * data) / target_freq_hz;
    winset--;

    return winset;
}
#else
/* Estimate winset setting (currently returns accuracy of 0.04%) target winset value
 * Function is currently designed for 26MHz as reference clk
 * (winset + 1)          data
 * ------------ = ------------------
 *   Ref Clk        Src Clk (target)
 */
ATTR_TEXT_IN_TCM static uint32_t fqmtr_winset_estimate(uint32_t target_freq_hz)
{
    uint32_t winset;
    /* data 10000: targets fqmtr to give an frequency measurement accuracy of 4/10000 = 0.04% */
    uint32_t data = 10000, ref_freq_hz = DCXO_HZ;
    /* typecasted to 64 bit uint (to avoid multiplication overflow) */
    winset = (((uint64_t)ref_freq_hz) * data) / target_freq_hz;
    winset--;

    /* check winset range (21 bits) for safety */
    if(winset > 2097151) {
        /* data 400: targets fqmtr to give an frequency measurement accuracy of 4/400 = 1% */
        data = 400;
        winset = (((uint64_t)ref_freq_hz) * data) / target_freq_hz;
        winset--;
    }
    return winset;
}
#endif

ATTR_TEXT_IN_TCM uint32_t hal_clock_get_freq_meter(hal_src_clock SRC_CLK, uint32_t winset)
{
    uint32_t ret_val = 0;
    static const uint32_t src_khz = 26000; /* DCXO 26MHz */

    if (SRC_CLK < FQMTR_SYS_DBG) {
        /* normal freq_meter API request */
        ret_val = clock_freq_meter(XO_CK, SRC_CLK, winset, src_khz);
    } else if (SRC_CLK == FQMTR_SYS_DBG) {
        /* debug use, prints CPU, DSP, SFC freqs */
        clk_dbg_print("CM33 [%d]KHz, DSP [%d] KHz, SFC [%d]KHz\r\n", 3,
                      (int)clock_freq_meter(XO_CK, hf_fsys_ck, winset, src_khz),
                      (int)clock_freq_meter(XO_CK, hf_fdsp_ck, winset, src_khz),
                      (int)clock_freq_meter(XO_CK, hf_fsfc_ck, winset, src_khz));
    }

    return ret_val;
}

#define LPOSC_CON_REG(rg)  (rg->field)


/* LPOSC calibration code, designed according to programming guide
 * Calibration actual freq will be smaller than target freq
 */
#define RBANK_CALI_MSB   (0x20)
#define RBANK_CALI_FT_MSB (0x8)
#define LPOSC1_RBANK_CALI_DEFAULT 0x28
#define LPOSC2_RBANK_CALI_DEFAULT 0x28
#define LPOSC1_RBANK_FT 0x0
#define LPOSC2_RBANK_FT 0x0
/**
 * This function should only be used for 1st time boot LPOSC calibration (when system is running off DCXO)
 * System will hang (due to LPOSC gating during calibration) if LPOSC is in use
 * Note: code flow assumes coarse cali, and FT cali are both called in order (will reset FT value in coarse cali flow)
 */
ATTR_TEXT_IN_TCM static void lposc_cali(osc_id osc, uint8_t task)
{
    uint8_t i, lposc_cali_ft_val;
    uint32_t fqmtr_data, target_khz, winset;
    volatile lposc_con *p_lposc_reg = NULL;
    volatile uint8_t *lposc_rdy = NULL, *lposc_cali_ft_reg;
    cali_info_t *cali_ctx = NULL;

    /* register address/field selection */
    if (osc == LPOSC1) {
        p_lposc_reg = ((lposc_con *)LPOSC1_CON0);
        lposc_rdy = (volatile uint8_t *)&CLKSQ_CON_REG.RG_LPOSC1_RDY;
    } else {
        p_lposc_reg = ((lposc_con *)LPOSC2_CON0);
        lposc_rdy = (volatile uint8_t *)&CLKSQ_CON_REG.RG_LPOSC2_RDY;
    }

    /* specify task related value settings */
    if (task == COARSE_CALI) {
        i = RBANK_CALI_MSB;
        lposc_cali_ft_val = RBANK_CALI_MSB;
        lposc_cali_ft_reg = &LPOSC_CON_REG(p_lposc_reg).DA_LPOSC_RBANK_CALI;
        LPOSC_CON_REG(p_lposc_reg).DA_LPOSC_RBANK_FT = 0; /* reset FT register field before coarse calibration */
        cali_ctx = &osc_tune_tbl[osc][0]; /* retrieve coarse calibration configs */
    } else { /* FINE_CALI */
        i = RBANK_CALI_FT_MSB;
        lposc_cali_ft_val = RBANK_CALI_FT_MSB;
        lposc_cali_ft_reg = &LPOSC_CON_REG(p_lposc_reg).DA_LPOSC_RBANK_FT;
        cali_ctx = &osc_tune_tbl[osc][1];  /* retrieve fine tune calibration configs */
    }

    target_khz = cali_ctx->cali_target;
    winset = fqmtr_winset_estimate(target_khz * 1000);
    clk_dbg_log("winset val %d\r\n", 1, winset);

    /*
     * actual calibration (selects a bit for each loop, starting from MSB to LSB)
     * algorithm originated from programming guide
     */
    while (i > 0) {
        *lposc_rdy = 0x0; /* enable LPOSC clock gating */

        *lposc_cali_ft_reg = lposc_cali_ft_val; /* assign value to cali or ft register field */

        hal_gpt_delay_us(20); /* Wait 20us for LPOSC stable */

        *lposc_rdy = 0x1; /* disable LPOSC clock gating */

        /* Use fqmtr to obtain OSC_D3_CK or OSC_CK */
        fqmtr_data = hal_clock_get_freq_meter(cali_ctx->src, winset);
        clk_dbg_log("[CLK] lposc%d cali target = %d kHz, fqmtr_data = %d kHz, cal_val(hex) %x\r\n", 4, (osc + 1), target_khz, fqmtr_data, lposc_cali_ft_val);
        if (fqmtr_data >= target_khz) {
            lposc_cali_ft_val = lposc_cali_ft_val - i + (i >> 1);
        } else {
            lposc_cali_ft_val = lposc_cali_ft_val + (i >> 1);
        }

        i = i >> 1; /* select next bit position */
    }
    /* assign value to cali or ft register field */
    *lposc_cali_ft_reg = lposc_cali_ft_val; /* assign value to cali or ft register field */

    /* print LPOSC calibration result */
    /* Use fqmtr to obtain OSC_CK */
    fqmtr_data = hal_clock_get_freq_meter(cali_ctx->src, winset);
    clk_dbg_log("[CLK] coarse 0x%x fine tune 0x%x\r\n", 2, LPOSC_CON_REG(p_lposc_reg).DA_LPOSC_RBANK_CALI, LPOSC_CON_REG(p_lposc_reg).DA_LPOSC_RBANK_FT);
    clk_dbg_log("[CLK] Final lposc%d target = %d kHz, fqmtr_data = %d kHz\r\n", 3, (osc + 1), target_khz, fqmtr_data);
}

ATTR_TEXT_IN_TCM static void lposc1_init(void)
{
    lposc_cali(LPOSC1, COARSE_CALI);
    lposc_cali(LPOSC1, COARSE_FT);
}

ATTR_TEXT_IN_TCM static void lposc2_init(void)
{
    lposc_cali(LPOSC2, COARSE_CALI);
    lposc_cali(LPOSC2, COARSE_FT);
}

/**
 * Setting for enable LPOSC1
 */
ATTR_TEXT_IN_TCM void lposc1_enable(void)
{
    static uint8_t lposc1_calibrated = 0;

    if (LPOSC1_CON_REG.DA_LPOSC_EN == 1) {
        lposc1_calibrated = 1;  /* Since LPOSC1 is in-use, it is expected to already be calibrated */
        return;
    }

#if 0
    /* disable resetting LPOSC calibration values */
    LPOSC1_CON_REG.DA_LPOSC_RBANK_CALI = 0x28;
    LPOSC1_CON_REG.DA_LPOSC_RBANK_FT  = 0x0;
#endif
    LPOSC1_CON_REG.RG_LPOSC_CBANK_SEL = 0x1;
    LPOSC1_CON_REG.RG_LPOSC_KVCO_SEL = 0x2;
    LPOSC1_CON_REG.DA_LPOSC_EN  = 0x1;         /* enable LPOSC1 */
    LPOSC1_CON_REG.RG_LPOSC_CK_EN  = 0x1;
    LPOSC1_CON_REG.RG_LPOSC_DIV3_CK_EN  = 0x1; /* used in calibration flow */
    LPOSC1_CON_REG.RG_LPOSC_DIV48_CK_EN  = 0x1; /* for hopping, SPM clock control */
    hal_gpt_delay_us(20);                      /* wait 20us for LPOSC1 stable */
    CLKSQ_CON_REG.RG_LPOSC1_RDY = 0x1;         /* release LPOSC1 clock gating */

    if (lposc1_calibrated == 0) {
        lposc1_init();
        lposc1_calibrated = 1;
    }
}

/**
 * Setting for enable LPOSC2
 */
ATTR_TEXT_IN_TCM void lposc2_enable(void)
{
    static uint8_t lposc2_calibrated = 0;

    if (LPOSC2_CON_REG.DA_LPOSC_EN == 1) {
        lposc2_calibrated = 1;  /* Since LPOSC2 is in-use, it is expected to already be calibrated */
        return;
    }

#if 0
    /* disable resetting LPOSC calibration values */
    LPOSC2_CON_REG.DA_LPOSC_RBANK_CALI = 0x28;
    LPOSC2_CON_REG.DA_LPOSC_RBANK_FT  = 0x0;
#endif
    LPOSC2_CON_REG.RG_LPOSC_CBANK_SEL = 0x2;
    LPOSC2_CON_REG.DA_LPOSC_EN  = 0x1;          /* enable LPOSC2 */
    LPOSC2_CON_REG.RG_LPOSC_CK_EN  = 0x1;
    LPOSC2_CON_REG.RG_LPOSC_DIV3_CK_EN  = 0x1; /* used in calibration flow */
    LPOSC2_CON_REG.RG_LPOSC_DIV48_CK_EN  = 0x1; /* for hopping, SPM clock control */
    hal_gpt_delay_us(20);                      /* wait 20us for LPOSC2 stable */
    CLKSQ_CON_REG.RG_LPOSC2_RDY = 0x1;         /* release LPOSC2 clock gating */

    if (lposc2_calibrated == 0) {
        lposc2_init();
        lposc2_calibrated = 1;
    }
}

#if 0
/* Comment out lposc1_disable() code (currently not used) */
ATTR_TEXT_IN_TCM static void lposc1_disable(void)
{
#if 0
    /* LPOSC1 should not be disabled
     * (for SPM clock control function to work)
     */
    CLKSQ_CON_REG.RG_LPOSC1_RDY = 0x0;
    LPOSC1_CON_REG.RG_LPOSC_DIV3_CK_EN  = 0x0;
    LPOSC1_CON_REG.RG_LPOSC_DIV48_CK_EN = 0x0;
    LPOSC1_CON_REG.RG_LPOSC_CK_EN  = 0x0;
    LPOSC1_CON_REG.DA_LPOSC_EN  = 0x0;
#endif
}
#endif

ATTR_TEXT_IN_TCM static void lposc2_disable(void)
{
    if (LPOSC2_CON_REG.DA_LPOSC_EN) {
        CLKSQ_CON_REG.RG_LPOSC2_RDY = 0x0;
        LPOSC2_CON_REG.RG_LPOSC_DIV3_CK_EN  = 0x0;
        LPOSC2_CON_REG.RG_LPOSC_DIV48_CK_EN = 0x0;
        LPOSC2_CON_REG.RG_LPOSC_CK_EN  = 0x0;
        LPOSC2_CON_REG.DA_LPOSC_EN  = 0x0;
    }
}

#if defined(__EXT_BOOTLOADER__) || defined(__EXT_DA__)
/* Settings to allow SPM to control clock on/off
 * currently called/enabled in bootloader
 */
ATTR_TEXT_IN_TCM static void spm_clock_ctrl_enable(void)
{
    /* Required settings_check */
    if (LPOSC1_CON_REG.DA_LPOSC_EN == 0) {
        clk_dbg_log("[CLK] spm_clock_ctrl warning, LPOSC1 was not enabled!\r\n", 0);
        /* LPOSC1 must be enabled */
        lposc1_enable();
    }
    /* TODO: check if DIV48_CK_EN is enabled for the current in-use LPOSC */

    CLKSQ_CON_REG.BP_PLL_DLY = 0x0;
    CLKSQ_CON_REG.UPLL_EN_SEL = 0x0;
    CLKSQ_CON_REG.LPOSC1_EN_SEL = 0x0;
    CLKSQ_CON_REG.LPOSC2_EN_SEL = 0x0;
    /* TODO: move MIXEDSYS CG control to a common function */
    hal_clock_enable(HAL_CLOCK_CG_MIXEDSYS);
}
#endif

/* CKSYS_CLK_CFG_0 register value for targeted DVFS OPP */
ATTR_TEXT_IN_TCM const clk_volt_lv_info dvfs_freq_tbl[VOLT_LV_NUM] = {
    {.field = { .sys = 3, .dsp = 3, .sfc = 2, .esc = ESC_104M_OSC1_D3}},
    {.field = { .sys = 2, .dsp = 2, .sfc = 2, .esc = ESC_104M_OSC1_D3}},
    {.field = { .sys = 5, .dsp = 5, .sfc = 2, .esc = ESC_104M_OSC1_D3}}
};
/* CKSYS_CLK_CFG_3 register value for targeted DVFS OPP */
ATTR_TEXT_IN_TCM const clk_cfg3_config dvfs_cfg3_tbl[VOLT_LV_NUM] = {
    {.field = {
              #if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE) || defined(AIR_HEARTHROUGH_VIVID_PT_ENABLE)
                .aud_intbus = 0,  // F_FXO_CK 26MHz
              #else
                //.aud_intbus = 2,  // OSC1_D3 104/99.75MHz (AIR_RFI_SUPPRESS_DISABLE defined/not_defined)
                .aud_intbus = 3,  // OSC1_D2_D2 78/74.8125MHz (AIR_RFI_SUPPRESS_DISABLE defined/not_defined)
              #endif
                                 .aud_gpsrc = 4, .aud_uplink = 0, .aud_dwlink = 1}},
    {.field = { .aud_intbus = 1, .aud_gpsrc = 1, .aud_uplink = 1, .aud_dwlink = 1}},
    {.field = { .aud_intbus = 1, .aud_gpsrc = 1, .aud_uplink = 1, .aud_dwlink = 1}}
};


/* API that returns current CPU clk freq in Hz
 * This was designed for usage in SystemCoreClockUpdate()
 */
ATTR_TEXT_IN_TCM uint32_t get_curr_cpu_freq_hz(void)
{
    //CLK_SYS_SEL corresponding frequency meaning definitions (in Hz)
    const uint32_t cpu_hz_arr[MUX_SEL_NUM] = {
        26 * 1000 * 1000,       // 0 : F_FXO_CK     26MHz
        26 * 1000 * 1000,       // 1 : OSC_26       26MHz
#if defined(AIR_RFI_SUPPRESS_DISABLE)
        156 * 1000 * 1000,      // 2 : OSC1_D2      156MHz
        104 * 1000 * 1000,      // 3 : OSC1_D3      104MHz
        78 * 1000 * 1000,       // 4 : OSC1_D2_D2   78MHz
#else
        149.625 * 1000 * 1000,  // 2 : OSC1_D2      149.625MHz
        99.75 * 1000 * 1000,    // 3 : OSC1_D3      99.75MHz
        74.8125 * 1000 * 1000,  // 4 : OSC1_D2_D2   74.8125MHz
#endif
        260 * 1000 * 1000,      // 5 : OSC2_D2      260MHz
        130 * 1000 * 1000,      // 6 : OSC2_D2_D2   130MHz
        156 * 1000 * 1000,      // 7 : UPLL_D2      156MHz
    };
    /* CLK_SYS_SEL represents the current mux select option, and the above defined array represents the corresponding freq definition */
    uint8_t curr_sel_num = *((uint8_t *)BYTE_REG(CKSYS_CLK_CFG_REG.CLK_SYS_SEL));

    return cpu_hz_arr[curr_sel_num];
}

/* Enable the dvfs requested mux switch required resources (before actual mux switch in dvfs_switch_clock_freq() */
ATTR_TEXT_IN_TCM void dvfs_pre_proc(uint8_t next_opp_idx)
{
    mux_switch_preproc(CLK_SYS_SEL,  dvfs_freq_tbl[next_opp_idx].field.sys);
    mux_switch_preproc(CLK_DSP_SEL,  dvfs_freq_tbl[next_opp_idx].field.dsp);
    mux_switch_preproc(CLK_SFC_SEL,  dvfs_freq_tbl[next_opp_idx].field.sfc);
    mux_switch_preproc(CLK_ESC_SEL,  dvfs_freq_tbl[next_opp_idx].field.esc);
    mux_switch_preproc(CLK_AUD_BUS_SEL,  dvfs_cfg3_tbl[next_opp_idx].field.aud_intbus);
    mux_switch_preproc(CLK_AUD_GPSRC_SEL,  dvfs_cfg3_tbl[next_opp_idx].field.aud_gpsrc);
    mux_switch_preproc(CLK_AUD_ULCK_SEL,  dvfs_cfg3_tbl[next_opp_idx].field.aud_uplink);
    mux_switch_preproc(CLK_AUD_DLCK_SEL,  dvfs_cfg3_tbl[next_opp_idx].field.aud_dwlink);
}

/* Check and disable unused divider/clock source accordingly */
ATTR_TEXT_IN_TCM void dvfs_post_proc(void)
{
    mux_switch_postproc();
}

/* all dvfs freq/mux switch is performed in this function */
ATTR_TEXT_IN_TCM void dvfs_switch_clock_freq(uint8_t next_opp_idx)
{
    clock_top_mux_ctrl(CLK_SYS_SEL, dvfs_freq_tbl[next_opp_idx].field.sys);
    clock_top_mux_ctrl(CLK_DSP_SEL, dvfs_freq_tbl[next_opp_idx].field.dsp);
    clock_top_mux_ctrl(CLK_SFC_SEL, dvfs_freq_tbl[next_opp_idx].field.sfc);
    clock_top_mux_ctrl(CLK_ESC_SEL, dvfs_freq_tbl[next_opp_idx].field.esc);

    /*
       === Workaound flow when switch AUD_INTBUS and AUD_GPSRC clock mux ===
        1. AUD_GPSRC clock off if it's on
        2. Switch AUD_INTBUS clock mux
        3. Switch AUD_GPSRC clock mux
        4. AUD_GPSRC clock on if it's turned off at step1
    */
    if(hal_clock_is_enabled(HAL_CLOCK_CG_AUD_GPSRC))
    {
        hal_phys_cg_ctrl(HAL_CLOCK_CG_AUD_GPSRC, CG_REQUEST_SET);   // Turn off AUD_GPSRC
        clock_top_mux_ctrl(CLK_AUD_BUS_SEL, dvfs_cfg3_tbl[next_opp_idx].field.aud_intbus);
        clock_top_mux_ctrl(CLK_AUD_GPSRC_SEL, dvfs_cfg3_tbl[next_opp_idx].field.aud_gpsrc);
        hal_phys_cg_ctrl(HAL_CLOCK_CG_AUD_GPSRC, CG_REQUEST_CLEAR); // Turn on AUD_GPSRC
    }
    else
    {
        clock_top_mux_ctrl(CLK_AUD_BUS_SEL, dvfs_cfg3_tbl[next_opp_idx].field.aud_intbus);
        clock_top_mux_ctrl(CLK_AUD_GPSRC_SEL, dvfs_cfg3_tbl[next_opp_idx].field.aud_gpsrc);
    }

    clock_top_mux_ctrl(CLK_AUD_ULCK_SEL, dvfs_cfg3_tbl[next_opp_idx].field.aud_uplink);
    clock_top_mux_ctrl(CLK_AUD_DLCK_SEL, dvfs_cfg3_tbl[next_opp_idx].field.aud_dwlink);

}

#if !defined(__EXT_BOOTLOADER__) && !defined(__EXT_DA__)
static hal_clock_status_t clk_mux_init_rtos(void)
{
    hal_clock_status_t result = HAL_CLOCK_STATUS_OK;
    clk_dbg_log("clk_mux_init_rtos\r\n", 0);

    /* Trigger LPOSC1, LPOSC2 calibration again during sys_init()
     * mainly to set the the lposc1_calibrated, lposc2_calibrated flags
     * (To avoid re-performing LPOSC full calibration during DVFS)
     */
    lposc1_enable();
    lposc2_enable();

#if defined(CLK_SW_TRACKING_TASK_ENABLE)
    lposc_recali();
#endif

    return result;
}
#else
ATTR_TEXT_IN_TCM static hal_clock_status_t clk_mux_init_baremetal(void)
{
    hal_clock_status_t result = HAL_CLOCK_STATUS_OK;
    lposc1_enable(); /* lposc1 must be enabled for SPM clock control */
    lposc2_enable();

    dvfs_pre_proc(HAL_DVFS_OPP_HIGH);
    dvfs_switch_clock_freq(HAL_DVFS_OPP_HIGH);

    /* set MCU/DSP side used module clock sources */
    clock_mux_sel(CLK_I3C_SEL, 1);
    clock_mux_sel(CLK_SPIMST0_SEL, 1);

    spm_clock_ctrl_enable();

    hal_clock_dcm_ctrl(); /* Enable/Disable DCM feature (controlled by compile option) */

    return result;
}
#endif



hal_clock_status_t clk_mux_init(void)
{
    hal_clock_status_t result = HAL_CLOCK_STATUS_OK;
#if !defined(__EXT_BOOTLOADER__) && !defined(__EXT_DA__)
    clk_mux_init_rtos();
#else
    clk_mux_init_baremetal();
#endif /*__EXT_BOOTLOADER__*/
    return result;
}

#if defined(DCM_ENABLE)
/* Settings for DCM enable */
static void hal_clock_dcm_enable(hal_clk_dcm_id dcm_id)
{
    switch (dcm_id) {
        case clk_aud_bus_dcm:
            /* for audio bus DCM to take affect , there are audio pdn register fields
             * that all need to be set to 1, they are listed as below
             *     pdn_asrc1, pdn_asrc2, pdn_asrc3, pdn_i2s_dma
             */
            AUD_DCM_CON_REG.RG_AUD_SFSEL = 0; // divide by 64
            AUD_DCM_CON_REG.RG_AUD_CLKSLOW_EN = 0x1;
            AUD_DCM_CON_REG.RG_AUD_CLKOFF_EN = 0x1;
            AUD_DCM_CON_REG.RG_AUD_PLLCK_SEL = 0x1;
            break;
        case clk_dsp_dcm :
            DSP_DCM_CON_REG.RG_DSP_SFSEL = 0; // divide by 64
            DSP_DCM_CON_REG.RG_DSP_CLKSLOW_EN = 0x1;
            DSP_DCM_CON_REG.RG_DSP_PLLCK_SEL = 0x1;
            break;
        case clk_bus_dcm : //BUS DCM
            //slow bus dcm
            XO_DCM_CON_REG.RG_XO_SFSEL = 0x2; // divide by 16
            XO_DCM_CON_REG.RG_XO_DCM_DBC_NUM = 0x2;
            XO_DCM_CON_REG.RG_XO_DCM_DBC_EN = 0x1;
            XO_DCM_CON_REG.RG_XO_CLKSLOW_EN = 0x1;
            XO_DCM_CON_REG.RG_XO_CLKOFF_EN = 0x1;
            XO_DCM_CON_REG.RG_XO_PLLCK_SEL = 0x1;

            // fast bus dcm
            BUS_DCM_CON_REG.RG_BUS_SFSEL = 0; // divide by 64
            BUS_DCM_CON_REG.RG_BUS_DCM_DBC_NUM = 0x2;
            BUS_DCM_CON_REG.RG_BUS_DCM_DBC_EN = 0x1;
            BUS_DCM_CON_REG.RG_BUS_CLKSLOW_EN = 0x1;
            BUS_DCM_CON_REG.RG_BUS_CLKOFF_EN = 0x1;
            BUS_DCM_CON_REG.RG_BUS_PLLCK_SEL = 0x1;
            hal_gpt_delay_us(1);
            break;
        case clk_sfc_dcm :
            SFC_DCM_CON_REG.RG_SFC_DCM_DBC_NUM = 0xFF;
            SFC_DCM_CON_REG.RG_SFC_DCM_DBC_EN = 0x1;
            SFC_DCM_CON_REG.RG_SFC_CLK_OFF_EN = 0x1;
            SFC_DCM_CON_REG.RG_SFC_DCM_APB_SEL = 0x6;
            SFC_DCM_CON_REG.RG_SFC_DCM_APB_TOG = ~SFC_DCM_CON_REG.RG_SFC_DCM_APB_TOG;
            break;

        case clk_esc_dcm :
            ESC_DCM_CON_REG.RG_ESC_DCM_DBC_NUM = 0xFF;
            ESC_DCM_CON_REG.RG_ESC_DCM_DBC_EN = 0x1;
            ESC_DCM_CON_REG.RG_ESC_CLK_OFF_EN = 0x1;
            ESC_DCM_CON_REG.RG_ESC_DCM_APB_SEL = 0x6;
            ESC_DCM_CON_REG.RG_ESC_DCM_APB_TOG = ~ESC_DCM_CON_REG.RG_ESC_DCM_APB_TOG;
            break;
        case clk_cm4_dcm :
            CM4_DCM_CON_REG.RG_CM_SFSEL = 0; // divide by 64
            CM4_DCM_CON_REG.RG_CM_CLKSLOW_EN = 0x1;
            CM4_DCM_CON_REG.RG_CM_PLLCK_SEL  = 0x1;
            hal_gpt_delay_us(1);
            break;
        default :
            return;
    }

    hal_gpt_delay_us(1);
}

#else
/* Settings for DCM disable */
static void hal_clock_dcm_disable(hal_clk_dcm_id dcm_id)
{
    switch (dcm_id) {
        case clk_aud_bus_dcm:
            AUD_DCM_CON_REG.RG_AUD_PLLCK_SEL = 0x0;
            AUD_DCM_CON_REG.RG_AUD_CLKSLOW_EN = 0x0;
            AUD_DCM_CON_REG.RG_AUD_CLKOFF_EN = 0x0;
            break;
        case clk_dsp_dcm :
            DSP_DCM_CON_REG.RG_DSP_PLLCK_SEL = 0x0;
            DSP_DCM_CON_REG.RG_DSP_CLKSLOW_EN = 0x0;
            break;
        case clk_bus_dcm : //BUS DCM
            //slow bus dcm
            XO_DCM_CON_REG.RG_XO_PLLCK_SEL = 0x0;
            XO_DCM_CON_REG.RG_XO_CLKSLOW_EN = 0x0;
            XO_DCM_CON_REG.RG_XO_CLKOFF_EN = 0x0;

            // fast bus dcm
            BUS_DCM_CON_REG.RG_BUS_PLLCK_SEL = 0x0;
            BUS_DCM_CON_REG.RG_BUS_CLKSLOW_EN = 0x0;
            BUS_DCM_CON_REG.RG_BUS_CLKOFF_EN = 0x0;
            hal_gpt_delay_us(1);
            break;
        case clk_sfc_dcm :
            SFC_DCM_CON_REG.RG_SFC_DCM_DBC_EN = 0x0;
            SFC_DCM_CON_REG.RG_SFC_CLK_OFF_EN = 0x0;
            SFC_DCM_CON_REG.RG_SFC_DCM_APB_SEL = 0x6;
            SFC_DCM_CON_REG.RG_SFC_DCM_APB_TOG = ~SFC_DCM_CON_REG.RG_SFC_DCM_APB_TOG;
            break;

        case clk_esc_dcm :
            ESC_DCM_CON_REG.RG_ESC_DCM_DBC_EN = 0x0;
            ESC_DCM_CON_REG.RG_ESC_CLK_OFF_EN = 0x0;
            ESC_DCM_CON_REG.RG_ESC_DCM_APB_SEL = 0x6;
            ESC_DCM_CON_REG.RG_ESC_DCM_APB_TOG = ~ESC_DCM_CON_REG.RG_ESC_DCM_APB_TOG;
            break;
        case clk_cm4_dcm :
            CM4_DCM_CON_REG.RG_CM_PLLCK_SEL  = 0x0;
            CM4_DCM_CON_REG.RG_CM_CLKSLOW_EN = 0x0;
            hal_gpt_delay_us(1);
            break;
        default :
            return;
    }

    hal_gpt_delay_us(1);
}
#endif

/* DCM ctrl API (controlled by internal compiled option) */
void hal_clock_dcm_ctrl(void)
{
#if defined(DCM_ENABLE)
    clk_dbg_log("[CLK] Dynamic Clock Management: Enable\r\n", 0);
    /* Enable */
    hal_clock_dcm_enable(clk_sfc_dcm);
    hal_clock_dcm_enable(clk_aud_bus_dcm);
    hal_clock_dcm_enable(clk_dsp_dcm);
    hal_clock_dcm_enable(clk_bus_dcm);
    hal_clock_dcm_enable(clk_esc_dcm);
    hal_clock_dcm_enable(clk_cm4_dcm);
#else
    /* Disable */
    clk_dbg_log("[CLK] Dynamic Clock Management: Disable\r\n", 0);
    hal_clock_dcm_disable(clk_sfc_dcm);
    hal_clock_dcm_disable(clk_aud_bus_dcm);
    hal_clock_dcm_disable(clk_dsp_dcm);
    hal_clock_dcm_disable(clk_bus_dcm);
    hal_clock_dcm_disable(clk_esc_dcm);
    hal_clock_dcm_disable(clk_cm4_dcm);
#endif
}


ATTR_TEXT_IN_TCM static void apll2_cali(void)
{
    uint32_t rg_val;

    XPLL_DBG_PROB_REG.XPLL_DBG_SEL = 0x8;
    APLL2_CON_REG.RG_APLL1_VTMON_EN = 0x1;
    hal_gpt_delay_us(10);
#ifndef FPGA_ENV
    while (1) {
        rg_val = *XPLL_DBG_PROB_MON;
        if (rg_val & 0x8) {
            // VTUNE in-range, calibration done
            break;
        } else if (rg_val & 0x2) {
            // VTUNE needs to be higher
            if (APLL2_CON_REG.RG_APLL1_IBANK_FINETUNE == 0xF) {
                clk_dbg_print("apll2 cali FT max err!(cali fail)\r\n", 0);
                assert(0);
                break;
            }

            APLL2_CON_REG.RG_APLL1_IBANK_FINETUNE += 1;
            hal_gpt_delay_us(50);
        } else if (rg_val & 0x4) {
            // VTUNE needs to be lower
            if (APLL2_CON_REG.RG_APLL1_IBANK_FINETUNE == 0) {
                clk_dbg_print("apll2 cali FT min err!(cali fail)\r\n", 0);
                assert(0);
                break;
            }

            APLL2_CON_REG.RG_APLL1_IBANK_FINETUNE -= 1;
            hal_gpt_delay_us(50);
        } else {
            clk_dbg_print("apll2 cali fail all 0\r\n", 0);
            assert(0);
            break;
        }
    }

#endif
    clk_dbg_log("APLL2 cali result FINETUNE = 0x%x\r\n", 1, APLL2_CON_REG.RG_APLL1_IBANK_FINETUNE);
    XPLL_DBG_PROB_REG.XPLL_DBG_SEL = 0x0;
    APLL2_CON_REG.RG_APLL1_VTMON_EN = 0x0;
}

ATTR_TEXT_IN_TCM static void apll1_cali(void)
{
    uint32_t rg_val;

    XPLL_DBG_PROB_REG.XPLL_DBG_SEL = 0x1;
    APLL1_CON_REG.RG_APLL1_VTMON_EN = 0x1;
    hal_gpt_delay_us(10);
#ifndef FPGA_ENV
    while (1) {
        rg_val = *XPLL_DBG_PROB_MON;
        if (rg_val & 0x8) {
            // VTUNE in-range, calibration done
            break;
        } else if (rg_val & 0x2) {
            // VTUNE needs to be higher
            if (APLL1_CON_REG.RG_APLL1_IBANK_FINETUNE == 0xF) {
                clk_dbg_print("apll1 cali FT max err!(cali fail)\r\n", 0);
                assert(0);
                break;
            }

            APLL1_CON_REG.RG_APLL1_IBANK_FINETUNE += 1;
            hal_gpt_delay_us(50);
        } else if (rg_val & 0x4) {
            // VTUNE needs to be lower
            if (APLL1_CON_REG.RG_APLL1_IBANK_FINETUNE == 0) {
                clk_dbg_print("apll1 cali FT min err!(cali fail)\r\n", 0);
                assert(0);
                break;
            }

            APLL1_CON_REG.RG_APLL1_IBANK_FINETUNE -= 1;
            hal_gpt_delay_us(50);
        } else {
            // INRANGE, VTMON_H, VTMON_L all 0, unexpected error
            clk_dbg_print("apll1 cali fail all 0\r\n", 0);
            assert(0);
            break;
        }
    }
#endif
    clk_dbg_log("APLL1 cali result FINETUNE = 0x%x\r\n", 1, APLL1_CON_REG.RG_APLL1_IBANK_FINETUNE);
    XPLL_DBG_PROB_REG.XPLL_DBG_SEL = 0x0;
    APLL1_CON_REG.RG_APLL1_VTMON_EN = 0x0;
}

/**
 *  SW UPLL calibration flow
 *  - Should only be executed as a fallback solution
 */
static void upll_cali(void)
{
    uint32_t rg_val;

    SYS_ABIST_MON_CON_REG.ABIST_LMON_SEL = 0x16;
    UPLL_CON_REG.RG_UPLL_VTMON_EN = 0x1;
    hal_gpt_delay_us(10);
#ifndef FPGA_ENV
    while (1) {
        rg_val = *ABIST_MON_DATA0;
        if (rg_val & 0x4) {
            // VTUNE in-range, calibration done
            break;
        } else if (rg_val & 0x2) {
            // VTUNE needs to be higher
            if (UPLL_CON_REG.RG_UPLL_IBANK_FINETUNE == 0xF) {
                clk_dbg_print("upll cali FT max err!(cali fail)\r\n", 0);
                assert(0);
                break;
            }
            UPLL_CON_REG.RG_UPLL_IBANK_FINETUNE += 1;
            hal_gpt_delay_us(50);
        } else if (rg_val & 0x1) {
            // VTUNE needs to be lower
            if (UPLL_CON_REG.RG_UPLL_IBANK_FINETUNE == 0) {
                clk_dbg_print("upll cali FT min err!(cali fail)\r\n", 0);
                assert(0);
                break;
            }
            UPLL_CON_REG.RG_UPLL_IBANK_FINETUNE -= 1;
            hal_gpt_delay_us(50);
        } else {
            // INRANGE, VTMON_H, VTMON_L all 0, unexpected error
            clk_dbg_print("upll cali fail all 0\r\n", 0);
            assert(0);
            break;
        }
    }
#endif
    clk_dbg_log("UPLL cali result FINETUNE = 0x%x\r\n", 1, UPLL_CON_REG.RG_UPLL_IBANK_FINETUNE);
    SYS_ABIST_MON_CON_REG.ABIST_LMON_SEL = 0x0;
    UPLL_CON_REG.RG_UPLL_VTMON_EN = 0x0;
}

/* clk_pll_on: enables APLL1, APLL2, UPLL
 * (returns if it is already enabled)
 */
#define APLL_LOW_PWR_MODE /* define this to enable APLL_LOW_PWR_MODE */
ATTR_TEXT_IN_TCM static uint8_t clk_pll_on(clock_pll_id pll_id)
{
    // Check in advance for saving time
    uint8_t need2run = 0;
    switch (pll_id) {
        case CLK_APLL1:
            if (APLL1_CON_REG.RG_APLL1_EN == 0) {
                need2run = 1;
            }
            break;
        case CLK_APLL2:
            if (APLL2_CON_REG.RG_APLL1_EN == 0) {
                need2run = 1;
            }
            break;
        case CLK_UPLL:
            if (UPLL_CON_REG.DA_UPLL_EN == 0) {
                need2run = 1;
            }
            break;
        default:
            break;
    }
    if(!need2run) {
        return 0;
    }

    hal_clock_enable(HAL_CLOCK_CG_MIXEDSYS);
    switch (pll_id) {
        case CLK_APLL1:
            APLL1_CON_REG.RG_APLL1_V2I_EN = 0x1;
            APLL1_CON_REG.RG_APLL1_DDS_PWR_ON = 0x1;
            hal_gpt_delay_us(5);
            APLL1_CON_REG.RG_APLL1_DDS_ISO_EN = 0x0;
            APLL1_CON_REG.RG_APLL1_LCDDS_PCW_NCPO        = (apll_freq[CLK_APLL1] == CLK_APLL_45M)  ? 0x0DE517AA + 1 : 0x0F1FAA4D + 1; //? 45M+1 : 49M+1  /*? 0xCEC4EC5 : 0xE276276;    //APLL1_CON_REG[10] 42M : 46M*/
            APLL1_CON_REG.RG_APLL1_LCDDS_TUNER_PCW_NCPO  = (apll_freq[CLK_APLL1] == CLK_APLL_45M)  ? 0x0DE517AA - 1 : 0x0F1FAA4D - 1; //? 45M-1 : 49M-1  /*? 0xEC4EC4F : 0x104EC4EC;   //APLL1_CON_REG[14] 48M : 53M*/
            APLL1_CON_REG.RG_APLL1_DIV16_EN = 0x1;
            APLL1_CON_REG.RG_APLL1_IBANK_EN = 0X1;
#if defined(APLL_LOW_PWR_MODE)
            /* low power mode */
            APLL1_CON_REG.RG_APLL1_LDOOUT2_SEL = 0x2;
            APLL1_CON_REG.RG_APLL1_LDOOUT1_SEL = 0x0;
            APLL1_CON_REG.RG_APLL1_CP_EN = 0x1;
            APLL1_CON_REG.RG_APLL1_RESERVE = 0xC4;
            RSV_CON0_REG.SCAN_RSV0 = 0x20;
#else
            APLL1_CON_REG.RG_APLL1_LDOOUT2_SEL = 0x2;
            APLL1_CON_REG.RG_APLL1_LDOOUT1_SEL = 0x2;
            APLL1_CON_REG.RG_APLL1_CP_EN = 0x1;
            APLL1_CON_REG.RG_APLL1_RESERVE = 0x04;
            RSV_CON0_REG.SCAN_RSV0 = 0x00;
#endif
            APLL1_CON_REG.RG_APLL1_LCDDS_EN = 0x1;
            APLL1_CON_REG.RG_APLL1_EN = 0x1;
            hal_gpt_delay_us(50);
            apll1_cali();

            APLL1_CON_REG.RG_APLL1_LCDDS_TUNER_EN = 0x1;
            break;

        case CLK_APLL2:
            APLL2_CON_REG.RG_APLL1_V2I_EN = 0x1;
            APLL2_CON_REG.RG_APLL1_DDS_PWR_ON = 0x1;
            hal_gpt_delay_us(5);
            APLL2_CON_REG.RG_APLL1_DDS_ISO_EN = 0x0;
            APLL2_CON_REG.RG_APLL1_LCDDS_PCW_NCPO        = (apll_freq[CLK_APLL2] == CLK_APLL_45M)  ? 0x0DE517AA + 1 : 0x0F1FAA4D + 1; //? 45M+1 : 49M+1  /*? 0xCEC4EC5 : 0xE276276;    //APLL1_CON_REG[10] 42M : 46M*/
            APLL2_CON_REG.RG_APLL1_LCDDS_TUNER_PCW_NCPO  = (apll_freq[CLK_APLL2] == CLK_APLL_45M)  ? 0x0DE517AA - 1 : 0x0F1FAA4D - 1; //? 45M-1 : 49M-1  /*? 0xEC4EC4F : 0x104EC4EC;   //APLL1_CON_REG[14] 48M : 53M*/
            APLL2_CON_REG.RG_APLL1_DIV16_EN = 0x1;
            APLL2_CON_REG.RG_APLL1_IBANK_EN = 0X1;
#if defined(APLL_LOW_PWR_MODE)
            /* low power mode */
            APLL2_CON_REG.RG_APLL1_LDOOUT2_SEL = 0x2;
            APLL2_CON_REG.RG_APLL1_LDOOUT1_SEL = 0x0;
            APLL2_CON_REG.RG_APLL1_CP_EN = 0x1;
            APLL2_CON_REG.RG_APLL1_RESERVE = 0xC4;
            RSV_CON0_REG.SCAN_RSV0 = 0x20;
#else
            APLL2_CON_REG.RG_APLL1_LDOOUT2_SEL = 0x2;
            APLL2_CON_REG.RG_APLL1_LDOOUT1_SEL = 0x2;
            APLL2_CON_REG.RG_APLL1_CP_EN = 0x1;
            APLL2_CON_REG.RG_APLL1_RESERVE = 0x04;
            RSV_CON0_REG.SCAN_RSV0 = 0x00;
#endif
            APLL2_CON_REG.RG_APLL1_LCDDS_EN = 0x1;
            APLL2_CON_REG.RG_APLL1_EN = 0x1;
            hal_gpt_delay_us(50);
            apll2_cali();
            APLL2_CON_REG.RG_APLL1_LCDDS_TUNER_EN = 0x1;
            break;

        case CLK_UPLL:
            UPLL_CON_REG.RG_UPLL_GLOBAL_LDO_BIAS_EN = 0x1;
            hal_gpt_delay_us(5);
            UPLL_CON_REG.RG_UPLL_IBANK_EN = 0x1;
            UPLL_CON_REG.RG_UPLL_CP_EN = 0x1;
            UPLL_CON_REG.RG_UPLL_PREDIV = 0x0;
            UPLL_CON_REG.RG_UPLL_FBDIV = 24;
            UPLL_CON_REG.RG_UPLL_POSTDIV = 0x1;
            UPLL_CON_REG.DA_UPLL_EN = 0x1;
            UPLL_CON_REG.RG_UPLL_48M_EN = 0x1;
            UPLL_CON_REG.RG_UPLL_624M_EN  = 0x1;
            hal_gpt_delay_us(20);
            CLKSQ_CON_REG.RG_UPLL_RDY = 0x1;
            if (EFUSE->M_ANA_CFG_UPLL_IBANK_b.UPLL_IBANK_EN) {
                /* Apply calibrated settings from efuse, or else don't modify HW default values */
                UPLL_CON_REG.RG_UPLL_IBANK_FINETUNE = EFUSE->M_ANA_CFG_UPLL_IBANK_b.UPLL_IBANK_FINETUNE;
                UPLL_CON_REG.RG_UPLL_KVCO_SEL = EFUSE->M_ANA_CFG_UPLL_IBANK_b.UPLL_KVCO_SEL;
                clk_dbg_log("UPLL IBANK_FINETUNE 0x%x, KVCO_SEL 0x%x\r\n", 2, EFUSE->M_ANA_CFG_UPLL_IBANK_b.UPLL_IBANK_FINETUNE, EFUSE->M_ANA_CFG_UPLL_IBANK_b.UPLL_KVCO_SEL);
            } else {
                clk_dbg_print("Warning! UPLL efuse FT not done! Performing upll_cali\r\n", 0);
                upll_cali();
            }
            break;
    }
    /* Must wait 20us before disabling MIXEDSYS clock
     * (for UPLL internal DLY_CNT to process)
     */
    hal_gpt_delay_us(20);
    hal_clock_disable(HAL_CLOCK_CG_MIXEDSYS);
    return 1;
}

ATTR_TEXT_IN_TCM static uint8_t clk_pll_off(clock_pll_id pll_id)
{
    // Check in advance for saving time
    uint8_t need2run = 0;
    switch (pll_id) {
        case CLK_APLL1:
            if (APLL1_CON_REG.RG_APLL1_EN) {
                need2run = 1;
            }
            break;
        case CLK_APLL2:
            if (APLL2_CON_REG.RG_APLL1_EN) {
                need2run = 1;
            }
            break;
        case CLK_UPLL:
            if (UPLL_CON_REG.DA_UPLL_EN) {
                need2run = 1;
            }
            break;
        default:
            break;
    }
    if(!need2run) {
        return 0;
    }

    hal_clock_enable(HAL_CLOCK_CG_MIXEDSYS);
    switch (pll_id) {
        case CLK_APLL1:
            APLL1_CON_REG.RG_APLL1_EN = 0x0;
            APLL1_CON_REG.RG_APLL1_LCDDS_EN = 0x0;
            APLL1_CON_REG.RG_APLL1_DDS_ISO_EN = 0x1;
            APLL1_CON_REG.RG_APLL1_DDS_PWR_ON = 0x0;
            hal_gpt_delay_us(5);

            APLL1_CON_REG.RG_APLL1_V2I_EN = 0x0;
            APLL1_CON_REG.RG_APLL1_LCDDS_PCW_NCPO_CHG = 0x0;
            // cksys tuner off
            APLL1_CON_REG.RG_APLL1_LCDDS_PCW_NCPO  = APLL1_CON_REG.DA_APLL1_LCDDS_PCW_NCPO;
            APLL1_CON_REG.RG_APLL1_LCDDS_TUNER_EN = 0x0;
            break;
        case CLK_APLL2:
            APLL2_CON_REG.RG_APLL1_EN = 0x0;
            APLL2_CON_REG.RG_APLL1_LCDDS_EN = 0x0;
            APLL2_CON_REG.RG_APLL1_DDS_ISO_EN = 0x1;
            APLL2_CON_REG.RG_APLL1_DDS_PWR_ON = 0x0;
            hal_gpt_delay_us(5);

            APLL2_CON_REG.RG_APLL1_V2I_EN = 0x0;
            APLL2_CON_REG.RG_APLL1_LCDDS_PCW_NCPO_CHG = 0x0;
            // cksys tuner off
            APLL2_CON_REG.RG_APLL1_LCDDS_PCW_NCPO  = APLL2_CON_REG.DA_APLL1_LCDDS_PCW_NCPO;
            APLL2_CON_REG.RG_APLL1_LCDDS_TUNER_EN = 0x0;
            break;
        case CLK_UPLL:
            CLKSQ_CON_REG.RG_UPLL_RDY = 0x0;
            UPLL_CON_REG.RG_UPLL_48M_EN = 0x0;
            UPLL_CON_REG.DA_UPLL_EN = 0x0;
            UPLL_CON_REG.RG_UPLL_IBANK_EN = 0x0;
            UPLL_CON_REG.RG_UPLL_CP_EN = 0x0;
            UPLL_CON_REG.RG_UPLL_GLOBAL_LDO_BIAS_EN = 0x0;
            break;
    }
    hal_clock_disable(HAL_CLOCK_CG_MIXEDSYS);
    return 1;
}

ATTR_TEXT_IN_TCM hal_clock_status_t clk_set_apll(clock_pll_id pll, clk_apll_freq apll)
{

    switch (apll) {
        case CLK_APLL_45M:
            apll_freq[pll] = CLK_APLL_45M;
            goto pll_rst ;

        case CLK_APLL_49M:
            apll_freq[pll] = CLK_APLL_49M;
            goto pll_rst ;

        default :
            return HAL_CLOCK_STATUS_ERROR;
    }
pll_rst :

    if (pll == CLK_APLL1) {
        clk_pll_off(CLK_APLL1);
        clk_pll_on(CLK_APLL1);
    } else if (pll == CLK_APLL2) {
        clk_pll_off(CLK_APLL2);
        clk_pll_on(CLK_APLL2);
    }

    return HAL_CLOCK_STATUS_OK;

}

/*************************************************************************
 * CG Clock API definition
 *************************************************************************/

/* Check if specified cg_id has a corresponding mux that need processing
 * return -1: no corresponding mux
 * return any positive num: has corresponding mux (returns its corresponding mux_id)
 */
ATTR_TEXT_IN_TCM static int32_t clock_mux_exist(hal_clock_cg_id cg_id)
{
    /* Check if the clock_id has a corresponding mux that need to be switched
     * If one exists -> call mux_sel to trigger clock source/divider enable etc..
     */
    for (uint32_t mux_tbl_idx = 0; mux_tbl_idx < TOTAL_MUX_CNT; mux_tbl_idx++) {
        if (clock_tbl.mux_tbl[mux_tbl_idx].cg_info.cg_id == cg_id) {
            return mux_tbl_idx;
        }
    }

    return HAL_CLOCK_STATUS_INVALID_PARAMETER;
}

/* request
 * 0: status (return status, 1: cg enabled, 0: cg disabled)
 * 1: cg disable (return 0)
 * 2: cg enable (return 0)
 * others: return -1
 */
ATTR_TEXT_IN_TCM static int32_t hal_phys_cg_ctrl(hal_clock_cg_id clock_id, cg_request_t request)
{
    //clk_dbg_log("hal_phys_cg_ctrl\r\n", 0);
    volatile uint32_t *clr_addr = NULL, *status_addr = NULL, *set_addr = NULL;
    uint32_t bit_idx = clock_id & (0x1f);

    /* Set pointer to CG reg ptr */
    if (clock_id >= HAL_CLOCK_CG_UART1 && clock_id <= HAL_CLOCK_CG_AUXADC) {
        clr_addr = XO_PDN_PD_CLRD0;
        status_addr = XO_PDN_PD_COND0;
        set_addr = XO_PDN_PD_SETD0;
    } else if (clock_id >= HAL_CLOCK_CG_PWM0 && clock_id <= HAL_CLOCK_CG_GPTIMER1) {
        clr_addr = XO_PDN_AO_CLRD0;
        status_addr = XO_PDN_AO_COND0;
        set_addr = XO_PDN_AO_SETD0;
    } else if (clock_id >= HAL_CLOCK_CG_SPIMST0 && clock_id <= HAL_CLOCK_CG_EFUSE) {
        clr_addr = XO_PDN_TOP_CLRD0;
        status_addr = XO_PDN_TOP_COND0;
        set_addr = XO_PDN_TOP_SETD0;
    } else if (clock_id >= HAL_CLOCK_CG_OSC1_D12_D2 && clock_id <= HAL_CLOCK_CG_AUD_VOW_BUS) {
        clr_addr = XO_PDN_TOP_CLRD1;
        status_addr = XO_PDN_TOP_COND1;
        set_addr = XO_PDN_TOP_SETD1;
    } else if (clock_id >= HAL_CLOCK_CG_FAST_DMA0 && clock_id <= HAL_CLOCK_CG_SECURITY_PERI) {
        clr_addr = PDN_PD_CLRD0;
        status_addr = PDN_PD_COND0;
        set_addr = PDN_PD_SETD0;
    } else if (clock_id >= HAL_CLOCK_CG_BUS_ERR && clock_id <= HAL_CLOCK_CG_SEC_MON) {
        clr_addr = PDN_AO_CLRD0;
        status_addr = PDN_AO_COND0;
        set_addr = PDN_AO_SETD0;
    } else if (clock_id == HAL_CLOCK_CG_CMSYS_ROM) {
        clr_addr = PDN_TOP_CLRD0;
        status_addr = PDN_TOP_COND0;
        set_addr = PDN_TOP_SETD0;
    } else {
        clk_dbg_log("hal_phys_cg_ctrl(), invalid parameter clock_id %d", 1, clock_id);
        return -1;
    }

    if (request == CG_REQUEST_STATUS) {
        /* status (return cg bit directly*/
        return ((*(status_addr) >> bit_idx) & 0x1);
    } else if (request == CG_REQUEST_CLEAR) {
        /* cg disable (clock on) */
        *(clr_addr) |= (0x1 << bit_idx);
    } else if (request == CG_REQUEST_SET) {
        /* special case handling (do not allow the below CG to be disabled)
         * HAL_CLOCK_CG_ESC: for BT HW design, avoid bus hang when mmap address is accessed when clock is off
         * HAL_CLOCK_CG_AUD_INTBUS, HAL_CLOCK_CG_AUD_ENGINE_BUS: avoid bus hang when clock off
         * HAL_CLOCK_CG_I3C: CG_I3C is a master CG for all I2C/I3C HW modules, keep enabled for now
         * others: CG will be used on both and MCU/DSP side
         */
        hal_clock_cg_id clk_always_on_tbl[] = {HAL_CLOCK_CG_UART_DMA_0, HAL_CLOCK_CG_I3C0, HAL_CLOCK_CG_I2C1
                                                , HAL_CLOCK_CG_SPIMST0, HAL_CLOCK_CG_I3C, HAL_CLOCK_CG_ESC, HAL_CLOCK_CG_AUD_INTBUS, HAL_CLOCK_CG_AUD_ENGINE_BUS
                                                , HAL_CLOCK_CG_FAST_DMA1, HAL_CLOCK_CG_SPIMST0_BUS, HAL_CLOCK_CG_I3C_DMA0, HAL_CLOCK_CG_I2C_DMA1, HAL_CLOCK_CG_TRNG
                                                };
        for (int idx = 0; idx < (sizeof(clk_always_on_tbl)/sizeof(clk_always_on_tbl[0])); idx++) {
            if (clock_id == clk_always_on_tbl[idx]) {
                /* do not allow disabling of the CG in the table */
                return HAL_CLOCK_STATUS_OK;
            }
        }

        /* cg enable (clock off) */
        *(set_addr) |= (0x1 << bit_idx);
    }
    return 0;
}

/* CG Enable */
ATTR_TEXT_IN_TCM hal_clock_status_t hal_clock_enable(hal_clock_cg_id clock_id)
{
    hal_clock_status_t ret = HAL_CLOCK_STATUS_OK;

    //clk_dbg_log ("hal_clock_enable\r\n", 0);
    /* Below disables clock gating to specified CG register or psuedo CG */
    if (clock_id < HAL_CLOCK_CG_PHYS_NUM) {
        hal_phys_cg_ctrl(clock_id, CG_REQUEST_CLEAR);
    } else if (clock_id > HAL_CLOCK_CG_PHYS_NUM && clock_id < HAL_CLOCK_CG_END) {
        /* Psuedo CG */
        uint32_t psuedo_cg_arr_idx = clock_id - HAL_CLOCK_CG_PSUEDO_CLK_26M;
        psuedo_cg_arr[psuedo_cg_arr_idx] = 0;
    } else if (PWM_MULTI_CTRL_REQUEST(clock_id)) {
        /* Process PWM multi-channel CG control request */
        volatile uint32_t *clr_addr = XO_PDN_AO_CLRD0;
        uint32_t set_bits = clock_id & CG_PWM_MULTI_CTRL_BITMASK;

        //clk_dbg_print("bitmask debug 0x%x, 0x%x", (clock_id & ~(CG_PWM_MULTI_CTRL_BITMASK)), ~(CG_PWM_MULTI_CTRL_BITMASK));
        *(clr_addr) = set_bits;
    } else {
        clk_dbg_log("hal_clock_enable() err! invalid clock_id %d", 1, clock_id);
        ret = HAL_CLOCK_STATUS_INVALID_PARAMETER;
    }

    int32_t clock_mux_idx = clock_mux_exist(clock_id);
    if (clock_mux_idx != (HAL_CLOCK_STATUS_INVALID_PARAMETER)) {
        /* CG's corresponding mux exists, perform mux select (to trigger clock enable etc..) */
        clock_mux_sel_id curr_mux_sel_id = clock_mux_idx;
        from_clock_enable_to_clock_mux_sel = 1;
        clock_mux_sel(curr_mux_sel_id, (int32_t)(*(clock_tbl.mux_tbl[clock_mux_idx].curr_sel_ptr)));
    }

    return ret;
}

/* CG Disable */
ATTR_TEXT_IN_TCM hal_clock_status_t hal_clock_disable(hal_clock_cg_id clock_id)
{
    hal_clock_status_t ret = HAL_CLOCK_STATUS_OK;

    //clk_dbg_log("hal_clock_disable\r\n", 0);
    /* Below disables clock gating to specified CG register or psuedo CG */
    if (clock_id < HAL_CLOCK_CG_PHYS_NUM) {
        hal_phys_cg_ctrl(clock_id, CG_REQUEST_SET);
    } else if (clock_id > HAL_CLOCK_CG_PHYS_NUM && clock_id < HAL_CLOCK_CG_END) {
        /* Psuedo CG */
        //clk_dbg_print("psuedo_cg\r\n", 0);
        uint32_t psuedo_cg_arr_idx = clock_id - HAL_CLOCK_CG_PSUEDO_CLK_26M;
        psuedo_cg_arr[psuedo_cg_arr_idx] = 1;
    } else if (PWM_MULTI_CTRL_REQUEST(clock_id)) {
        /* Process PWM multi-channel CG control request */
        volatile uint32_t *set_addr = XO_PDN_AO_SETD0;
        uint32_t set_bits = clock_id & CG_PWM_MULTI_CTRL_BITMASK;

        //clk_dbg_print("bitmask debug 0x%x, 0x%x", 2, (clock_id & ~(CG_PWM_MULTI_CTRL_BITMASK)), ~(CG_PWM_MULTI_CTRL_BITMASK));
        *(set_addr) = set_bits;
    } else {
        clk_dbg_print("hal_clock_disable() err! invalid clock_id %d", 1, clock_id);
        ret = HAL_CLOCK_STATUS_INVALID_PARAMETER;
    }

    int32_t clock_mux_idx = clock_mux_exist(clock_id);
    if (clock_mux_idx != (HAL_CLOCK_STATUS_INVALID_PARAMETER)) {
      #if !defined(__EXT_BOOTLOADER__) && !defined(__EXT_DA__)
        uint32_t irq_mask = 0;
        hal_nvic_save_and_set_interrupt_mask_special(&irq_mask);
        /* ================ Critical Section Start ======================== */
      #endif

        /* clock gating enabled may cause some clock resource to become able to disable */
        mux_switch_postproc();

      #if !defined(__EXT_BOOTLOADER__) && !defined(__EXT_DA__)
        /* ================ Critical Section End ======================== */
        hal_nvic_restore_interrupt_mask_special(irq_mask);
      #endif
    }

    return ret;
}


ATTR_TEXT_IN_TCM bool hal_clock_is_enabled(hal_clock_cg_id clock_id)
{
    int32_t cg_status = true;

    //clk_dbg_log("hal_clock_is_enabled\r\n", 0);
    /* Below retrieves clock gating status of specified CG register or psuedo CG */
    if (clock_id < HAL_CLOCK_CG_PHYS_NUM) {
        cg_status = hal_phys_cg_ctrl(clock_id, CG_REQUEST_STATUS);
    } else if (clock_id > HAL_CLOCK_CG_PHYS_NUM && clock_id < HAL_CLOCK_CG_END) {
        /* Psuedo CG */
        uint32_t psuedo_cg_arr_idx = clock_id - HAL_CLOCK_CG_PSUEDO_CLK_26M;
        cg_status = psuedo_cg_arr[psuedo_cg_arr_idx];
    } else {
        clk_dbg_log("hal_clock_disable() err! invalid clock_id %d", 1, clock_id);
        return HAL_CLOCK_STATUS_INVALID_PARAMETER;
    }

    if (cg_status == 0x1) {
        //clk_dbg_log("cg_id %d is disabled\r\n", 1, clock_id);
        return false;
    } else {
        //clk_dbg_log("cg_id %d is enabled\r\n", 1, clock_id);
        return true;
    }

}


/* Clock gating initial registRer settings */
static void _hal_clock_cg_init(void)
{
    struct cg_init_stat {
        uint8_t cg_id; /* hal_clock_cg_id (physical CG bit used)*/
        uint8_t stat;
    };

    /* MCU/DSP side both used CG: default clock enable
     * HAL_CLOCK_CG_UART_DMA_0, HAL_CLOCK_CG_I3C0, HAL_CLOCK_CG_I2C1
     * HAL_CLOCK_CG_SPIMST0, HAL_CLOCK_CG_I3C,
     * HAL_CLOCK_CG_FAST_DMA1, HAL_CLOCK_CG_SPIMST0_BUS, HAL_CLOCK_CG_I3C_DMA0, HAL_CLOCK_CG_I2C_DMA1, HAL_CLOCK_CG_TRNG
     */

    const struct cg_init_stat cg_init_tbl[] = {
        /* XO_PDN_PD_COND0(0x42040B00) */
        {.cg_id = HAL_CLOCK_CG_UART1, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_UART2, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_UART_DMA_0, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_UART_DMA_1, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_UART_DMA_2, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_IRRX, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_IRRX_BUS, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_I3C0, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_I3C1, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_I2C0, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_I2C1, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_I2C2, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_ROSC, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_UART0, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_AUXADC, .stat = CLK_REQ_DISABLE},
        /* XO_PDN_AO_COND0(0x42040B30) */
        {.cg_id = HAL_CLOCK_CG_PWM0, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_PWM1, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_PWM2, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_PWM3, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_PWM4, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_PWM5, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_PWM6, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_PWM7, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_PWM8, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_PWM9, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_PWM10, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_PWM11, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_PWM12, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_PWM13, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_SPM_PCLK, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_BCLK_CM33, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_BCLK_DSP, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_SPM_DIV, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_I2C_AO, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_OSTIMER, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_GPTIMER0, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_GPTIMER1, .stat = CLK_REQ_ENABLE},
        /* XO_PDN_TOP_COND0(0x42040B60) */
        {.cg_id = HAL_CLOCK_CG_SPIMST0, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_SPIMST1, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_SPIMST2, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_SDIOMST, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_I3C, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_BT_HOP, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_AUD_GPSRC, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_AUD_UPLINK, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_AUD_DWLINK, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_SPDIF, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_AUD_INTF0, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_AUD_INTF1, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_AUD_TEST, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_AUD_ANC, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_AUD_CLD, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_SFC, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_ESC, .stat = CLK_REQ_ENABLE}, /* ESC clk always on (avoid bus hang, for BT HW) */
        {.cg_id = HAL_CLOCK_CG_SPISLV, .stat = CLK_REQ_DISABLE},
#ifndef __EXT_DA__
        {.cg_id = HAL_CLOCK_CG_USB, .stat = CLK_REQ_DISABLE},
#endif
        {.cg_id = HAL_CLOCK_CG_AUD_INTBUS, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_DSP, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_SEJ, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_MIXEDSYS, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_EFUSE, .stat = CLK_REQ_ENABLE},
        /* XO_PDN_TOP_COND1(0x42040B90) */
        {.cg_id = HAL_CLOCK_CG_OSC1_D12_D2, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_OSC2_D10_D2, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_CMSYS_BUS, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_RSV_BUS, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_CONN_BUS, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_RSV_RTC_BUS, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_RSV_MCLK_BUS, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_AUD_ENGINE_BUS, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_AUD_VOW_BUS, .stat = CLK_REQ_DISABLE},
        /* PDN_PD_COND0(0x422B0300) */
        {.cg_id = HAL_CLOCK_CG_FAST_DMA0, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_FAST_DMA1, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_SPIMST0_BUS, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_SPIMST1_BUS, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_SPIMST2_BUS, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_I3C_DMA0, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_I3C_DMA1, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_I2C_DMA0, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_I2C_DMA1, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_I2C_DMA2, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_SECURITY_PD, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_AESOTF, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_AESOTF_ESC, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_CRYPTO, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_TRNG, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_SPISLV_BUS, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_SDIOMST0, .stat = CLK_REQ_DISABLE},
#ifndef __EXT_DA__
        {.cg_id = HAL_CLOCK_CG_USB_BUS, .stat = CLK_REQ_DISABLE},
        {.cg_id = HAL_CLOCK_CG_USB_DMA, .stat = CLK_REQ_DISABLE},
#endif
        {.cg_id = HAL_CLOCK_CG_SECURITY_PERI, .stat = CLK_REQ_ENABLE},
        /* PDN_AO_COND0(0x422B0330) */
        {.cg_id = HAL_CLOCK_CG_BUS_ERR, .stat = CLK_REQ_ENABLE},
        {.cg_id = HAL_CLOCK_CG_SEC_MON, .stat = CLK_REQ_ENABLE},
        /* PDN_TOP_COND0(0x422B0360) */
        {.cg_id = HAL_CLOCK_CG_CMSYS_ROM, .stat = CLK_REQ_ENABLE}
    };

    for (uint8_t idx = 0; idx < sizeof(cg_init_tbl) / sizeof(cg_init_tbl[0]); idx++) {
        if (cg_init_tbl[idx].stat == CLK_REQ_DISABLE) {
            hal_clock_disable(cg_init_tbl[idx].cg_id);
        } else {
            hal_clock_enable(cg_init_tbl[idx].cg_id);
        }
    }

}


hal_clock_status_t hal_clock_init(void)
{
    clk_dbg_log("hal_clock_init\r\n", 0);
#if 0
    /* Clock free run */
    *XO_PDN_PD_CLRD0 = 0xffffffff;
    *XO_PDN_AO_CLRD0 = 0xffffffff;
    *XO_PDN_TOP_CLRD0 = 0xffffffff;
    *XO_PDN_TOP_CLRD1 = 0xffffffff;
    *PDN_PD_CLRD0 = 0xffffffff;
    *PDN_TOP_CLRD0 = 0xffffffff;
    *PDN_AO_CLRD0 = 0xffffffff;
#endif
    /* Actual CG init settings */
    _hal_clock_cg_init();

    return clk_mux_init();
}

#if !defined(__EXT_BOOTLOADER__) && !defined(__EXT_DA__)
/**
 * hal_clock_chop_ck(): This function controls chop_ck functionality
 * parameter enable: enable/disable the function
 * div_sel: set divider setting (actual divder = div_sel * 2 + 2)
 *          div_sel valid value range: 0 ~ 2047
 */
hal_clock_status_t hal_clock_chop_ck(uint32_t enable, uint32_t div_sel)
{
    if (enable) {
        //uint32_t divider = div_sel * 2 + 2;
        CKSYS_CLK_DIV_REG.CHOP_DIV_SEL = div_sel;
        CKSYS_CLK_DIV_REG.CHOP_DIV_CHG = ~CKSYS_CLK_DIV_REG.CHOP_DIV_CHG;
        hal_gpt_delay_us(471);
    }

    CKSYS_CLK_DIV_REG.CHOP_DIV_EN = enable;

    clk_dbg_log("hal_clock_chop_ck() enable: %d, div_sel:%d (actual divider: %d)\r\n", 3,
                 (int)enable
                 , CKSYS_CLK_DIV_REG.CHOP_DIV_SEL
                 , CKSYS_CLK_DIV_REG.CHOP_DIV_SEL * 2 + 2);

    return HAL_CLOCK_STATUS_OK;
}
#endif

#if defined(CLK_SW_TRACKING_TASK_ENABLE)

/* function for LPOSC calibration fine tune (+-1) */
static uint32_t lposc_recali_fine_tune(volatile uint32_t *lposc_con1, uint8_t lposcN, uint8_t ft_add_one)
{
    uint32_t ft = *lposc_con1 & 0xFF;
    uint32_t cali = (*lposc_con1 >> 8) & 0xFF;
    uint32_t combined_rg;

    if (ft_add_one) {  /* FT +1 */
        if (ft == LPOSC_RG_FT_MAX) {
            ft = 0;
            cali++;
        } else {
            ft++;
        }
    } else {  /* FT -1 */
        if (ft == 0) {
            ft = LPOSC_RG_FT_MAX; /* LPOSC_RG_FT_MAX == 0xF */
            cali--;
        } else {
            ft--;
        }
    }

    lposc_recali_cali_cur[lposcN-1] = cali;
    lposc_recali_ft_cur[lposcN-1] = ft;

    /* LPOSC register CALI, FT needs to be written to RG at the same time
     * to avoid abrupt frequency change when both cali, ft fields are changed
     */
    combined_rg = ((*lposc_con1) & 0xFFFF0000) | cali << 8 | ft;

    return combined_rg;
}

static void lposc_recali_add_more_debug_log(uint8_t lposcN)
{
    /*
        === 42050204 LPOSC1_CON1 ===
        25:24 RG_LPOSC1_KVCO_SEL    *
        17:16 RG_LPOSC1_BIAS_SEL
        13:8  DA_LPOSC1_RBANK_CALI  *
         3:0  DA_LPOSC1_RBANK_FT    *

        === 42050208 LPOSC1_CON2 ===
        24    RG_LPOSC1_LDO_EN
        19:16 RG_LPOSC1_AMP_CF_SEL
        8     RG_LPOSC1_AMP_CP_EN
         1:0  RG_LPOSC1_CBANK_SEL   *

        === 42050254 LPOSC2_CON1 ===
        25:24 RG_LPOSC2_KVCO_SEL    *
        17:16 RG_LPOSC2_BIAS_SEL
        13:8  DA_LPOSC2_RBANK_CALI  *
         3:0  DA_LPOSC2_RBANK_FT    *

        === 42050258 LPOSC2_CON2 ===
        24    RG_LPOSC2_LDO_EN
        19:16 RG_LPOSC2_AMP_CF_SEL
        8     RG_LPOSC2_AMP_CP_EN
         1:0  RG_LPOSC2_CBANK_SEL   *

        === DA_DCXO_CDAC[8:0] / DA_DCXO_CORE_ISEL[3:0] ===
        write 0x430A00A8[12:9] = 1
        read 0x430A00AC & 0x430A00B0
        DA_DCXO_CDAC[8:0] = {0x430A00B0[2:0], 0x430A00AC[15:10]}
        DA_DCXO_CORE_ISEL[3:0] = 0x430A00AC[9:6]
        
        === DA_DCXO_STABLE_32K ===
        write 0x430A00A8[12:9] = 6
        read 0x430A00AC
        DA_DCXO_STABLE_32K = 0x430A00AC[12]
        
        === DA_DCXO_LPM_LDO_EN / DA_DCXO_FPM_LDO_EN ===
        write 0x430A00A8[12:9] = 0
        read 0x430A00AC
        {DA_DCXO_LPM_LDO_EN, DA_DCXO_FPM_LDO_EN} = 0x430A00AC[15:14]
    */
    volatile uint32_t *lposc_con1 = LPOSC1_CON1;
    volatile uint32_t *lposc_con2 = LPOSC1_CON2;

    if(lposcN == 2) {
        lposc_con1 = LPOSC2_CON1;
        lposc_con2 = LPOSC2_CON2;
    } 

  #ifdef MTK_BATTERY_MANAGEMENT_ENABLE
    clk_recali_print("LPOSC%d Temp %d", 2, lposcN, battery_management_get_battery_property(BATTERY_PROPERTY_TEMPERATURE));
  #endif
    clk_recali_print("LPOSC%d CON1 0x%08X, CON2 0x%08X", 3, lposcN, *lposc_con1, *lposc_con2);
    
    // DCXO related RG
    uint32_t vDCXO_AUX_SEL = *(volatile uint32_t *)(DCXO_CFG_BASE + 0xA8);
    
    vDCXO_AUX_SEL &= 0xFFFFE1FF;  // clear [12:9]
    *(volatile uint32_t *)(DCXO_CFG_BASE + 0xA8) = vDCXO_AUX_SEL + (1 << 9);  // [12:9] = 1
    clk_recali_print("LPOSC%d DCXO_AUX_SEL=1, AC=0x%04X, B0=0x%04X", 3, lposcN, *(volatile uint32_t *)(DCXO_CFG_BASE + 0xAC), *(volatile uint32_t *)(DCXO_CFG_BASE + 0xB0));
    
    vDCXO_AUX_SEL &= 0xFFFFE1FF;  // clear [12:9]
    *(volatile uint32_t *)(DCXO_CFG_BASE + 0xA8) = vDCXO_AUX_SEL + (6 << 9);  // [12:9] = 6
    clk_recali_print("LPOSC%d DCXO_AUX_SEL=6, AC=0x%04X", 2, lposcN, *(volatile uint32_t *)(DCXO_CFG_BASE + 0xAC));
    
    vDCXO_AUX_SEL &= 0xFFFFE1FF;  // clear [12:9]
    *(volatile uint32_t *)(DCXO_CFG_BASE + 0xA8) = vDCXO_AUX_SEL;  // [12:9] = 0
    clk_recali_print("LPOSC%d DCXO_AUX_SEL=0, AC=0x%04X", 2, lposcN, *(volatile uint32_t *)(DCXO_CFG_BASE + 0xAC));
}

/* actual lposc frequency detect/recalibrate SW flow
 * recalibration flow will end/terminate if LPOSC is disabled
 */
static int lposc_recali_flow(const uint32_t lposc_winset, const uint32_t target_lposc_data, hal_src_clock src_clk, volatile uint32_t *lposc_con1, volatile uint8_t *lposc_en, const uint32_t data_upper_bound, const uint32_t data_lower_bound)
{
    uint32_t fqmtr_data;
    uint32_t recali_cnt = 0;
    uint8_t lposcN = 1;  // LPOSC1

    if(src_clk == AD_OSC2_CK) {
        lposcN = 2;  // LPOSC2
        hal_dvfs_lock_control(HAL_DVFS_OPP_HIGH, HAL_DVFS_LOCK);
    }
    while (1) {
        /* LPOSC is not expected to require 10 units of difference/change at once */
        if (recali_cnt > 10) {
            clk_dbg_prt_err("LPOSC%d recalibration error (recalibrated %d times!!)", 2, lposcN, recali_cnt);
            assert(0);
        }

        /* backup lposc_en_register value (for lposc status check outside of critical section) */
        uint8_t lposc_en_stat = *lposc_en;

        /* only perform lposc freq meter check if it is enabled (or else freq. meter value would not be accurate) */
        if (lposc_en_stat) {
            /* retrieve frequency meter data of LPOSC */
            fqmtr_data = hal_clock_freq_meter_data(XO_CK, src_clk, lposc_winset);
            clk_recali_dbg("fqmtr_data %d , target_lposc_data %d", 2, fqmtr_data, target_lposc_data);
        }

        if (lposc_en_stat == 0) {
            /* LPOSC was disabled during recalibration flow, exit */
            clk_dbg_prt_err("LPOSC%d disabled on recali, should not happen", 1, lposcN);
            break;
        } else {
            /* LPOSC is enabled during freq. meter measurement (valid fqmtr_data) */
            if (fqmtr_data < data_lower_bound) {
                // LPOSC too slow
                lposc_recali_add_more_debug_log(lposcN);
                clk_recali_print("LPOSC%d before increment CON1[0x%x], winset %d, data %d(target %d)", 5, lposcN, *lposc_con1, lposc_winset, fqmtr_data, target_lposc_data);
                *lposc_con1 = lposc_recali_fine_tune(lposc_con1, lposcN, 1);
                clk_recali_print("LPOSC%d after increment CON1[0x%x]", 2, lposcN, *lposc_con1);
                hal_gpt_delay_us(20); /* Wait 20us for LPOSC stable */
            } else if (fqmtr_data > data_upper_bound) {
                // LPOSC too fast
                lposc_recali_add_more_debug_log(lposcN);
                clk_recali_print("LPOSC%d before decrement CON1[0x%x], winset %d, data %d(target %d)", 5, lposcN, *lposc_con1, lposc_winset, fqmtr_data, target_lposc_data);
                *lposc_con1 = lposc_recali_fine_tune(lposc_con1, lposcN, 0);
                clk_recali_print("LPOSC%d after decrement CON1[0x%x]", 2, lposcN, *lposc_con1);
                hal_gpt_delay_us(20); /* Wait 20us for LPOSC stable */
            } else {
                // LPOSC calibration done (or not required)
                if(recali_cnt) {
                    lposc_recali_monitor_print_times[lposcN-1] = 0;
                    clk_recali_print("LPOSC%d ReKd fqmtr_data %d", 2, lposcN, fqmtr_data);
                }
                else {
                    lposc_recali_monitor_print_times[lposcN-1] ++;
                    if(lposc_recali_monitor_print_times[lposcN-1] == LPOSC_MONITOR_PRINT_TIMES_MAX) {
                        lposc_recali_monitor_print_times[lposcN-1]= 0;
                        clk_recali_print("LPOSC%d ReKd fqmtr_data %d (target %d), CALI 0x%02X, FT 0x%02X", 5, lposcN, fqmtr_data, target_lposc_data, (*lposc_con1 >> 8) & 0x3F, *lposc_con1 & 0x0F);
                        lposc_recali_add_more_debug_log(lposcN);
                    }
                }
                break;
            }
            recali_cnt++;
        }
    }
    if(lposcN == 2) {  // LPOSC2
        hal_dvfs_lock_control(HAL_DVFS_OPP_HIGH, HAL_DVFS_UNLOCK);
    }

    return recali_cnt;
}

/* lposc recalibration init, gpt sw timer on-shot callback register */
void lposc_recali(void)
{
    const uint32_t expect_fqmtr_data = 6000;
    const uint32_t dcxo_freq_khz = 26000;

    static uint8_t recali_init_done = 0;
    static uint8_t lposc_dump_info = 0;
    static uint32_t target_lposc1_data, target_lposc2_data;
    static uint32_t lposc1_winset, lposc2_winset;
    static uint32_t lposc_recali_handle;
    static uint32_t lposc1_data_upper_bound, lposc1_data_lower_bound;
    static uint32_t lposc2_data_upper_bound, lposc2_data_lower_bound;

#if 0
    uint32_t lposc1_recali_count = 0;
    uint32_t lposc1_recali_max_cycle = 0;
    uint32_t lposc2_recali_count = 0;
    uint32_t lposc2_recali_max_cycle = 0;
#endif
    clk_recali_dbg("lposc_recali", 0);

    if (recali_init_done) {
        int recal_count;

        if(!lposc_dump_info)
        {
            lposc_dump_info = 1;
            clk_recali_print("LPOSC1 Info: CALI 0x%02X, FT 0x%02X, winset %d, fqmtr_data %d (LB %d, center %d, UB %d)", 7,
                lposc_recali_cali_cur[0], lposc_recali_ft_cur[0], lposc1_winset, lposc_1st_time_k_fqmtr_data[0],
                lposc1_data_lower_bound, target_lposc1_data, lposc1_data_upper_bound);
            clk_recali_print("LPOSC2 Info: CALI 0x%02X, FT 0x%02X, winset %d, fqmtr_data %d (LB %d, center %d, UB %d)", 7,
                lposc_recali_cali_cur[1], lposc_recali_ft_cur[1], lposc2_winset, lposc_1st_time_k_fqmtr_data[1],
                lposc2_data_lower_bound, target_lposc2_data, lposc2_data_upper_bound);
        }

        if((lposc_recali_cali_cur[0]!=LPOSC1_CON_REG.DA_LPOSC_RBANK_CALI) || (lposc_recali_ft_cur[0]!=LPOSC1_CON_REG.DA_LPOSC_RBANK_FT))
        {
            clk_dbg_prt_err("LPOSC1 ReKchg: CALI 0x%02X->0x%02X, FT 0x%02X->0x%02X", 4, lposc_recali_cali_cur[0],
                            LPOSC1_CON_REG.DA_LPOSC_RBANK_CALI, lposc_recali_ft_cur[0], LPOSC1_CON_REG.DA_LPOSC_RBANK_FT);
        }

        /* LPOSC1 recalibration */
        recal_count = lposc_recali_flow(lposc1_winset, target_lposc1_data, AD_OSC1_CK, LPOSC1_CON1, BYTE_REG(LPOSC1_CON_REG.DA_LPOSC_EN), lposc1_data_upper_bound, lposc1_data_lower_bound);

        if (recal_count) {
            clk_recali_print("LPOSC1 recalibrated %d cycles, CALI 0x%02X, FT 0x%02X", 3, recal_count, (*LPOSC1_CON1 >> 8) & 0xFF, *LPOSC1_CON1 & 0xFF);
#if 0
            lposc1_recali_count++;
            if (recal_count > lposc1_recali_max_cycle) {
                lposc1_recali_max_cycle = recal_count;
            }
            clk_recali_print("LPOSC1 recali count %d, max cycle count %d\r\n", 2, lposc1_recali_count, lposc1_recali_max_cycle);
#endif
        }

        if(*BYTE_REG(LPOSC2_CON_REG.DA_LPOSC_EN)) {  // LPOSC2 enabled
            if((lposc_recali_cali_cur[1]!=LPOSC2_CON_REG.DA_LPOSC_RBANK_CALI) || (lposc_recali_ft_cur[1]!=LPOSC2_CON_REG.DA_LPOSC_RBANK_FT))
            {
                clk_dbg_prt_err("LPOSC2 ReKchg: CALI 0x%02X->0x%02X, FT 0x%02X->0x%02X", 4, lposc_recali_cali_cur[1],
                                LPOSC2_CON_REG.DA_LPOSC_RBANK_CALI, lposc_recali_ft_cur[1], LPOSC2_CON_REG.DA_LPOSC_RBANK_FT);
            }

            /* LPOSC2 recalibration */
            recal_count = lposc_recali_flow(lposc2_winset, target_lposc2_data, AD_OSC2_CK, LPOSC2_CON1, BYTE_REG(LPOSC2_CON_REG.DA_LPOSC_EN), lposc2_data_upper_bound, lposc2_data_lower_bound);

            if (recal_count) {
                clk_recali_print("LPOSC2 recalibrated %d cycles, CALI 0x%02X, FT 0x%02X", 3, recal_count, (*LPOSC2_CON1 >> 8) & 0xFF, *LPOSC2_CON1 & 0xFF);
#if 0
                lposc2_recali_count++;
                if (recal_count > lposc2_recali_max_cycle) {
                    lposc2_recali_max_cycle = recal_count;
                }
                clk_recali_dbg("LPOSC2 recali count %d, max cycle count %d\r\n", 2, lposc2_recali_count, lposc2_recali_max_cycle);
#endif
            }
        }
    } else {
        uint32_t lposc_variation_data;
        lposc1_winset = (uint32_t)(((double)expect_fqmtr_data * dcxo_freq_khz) / LPOSC1_TARGET_KHZ - 1 + 0.5);
        target_lposc1_data = ((double)(lposc1_winset + 1) * LPOSC1_TARGET_KHZ) / dcxo_freq_khz;

        lposc_variation_data = ((double)target_lposc1_data + 0.5) * 0.0025;
        lposc1_data_upper_bound = target_lposc1_data + lposc_variation_data + 2;
        lposc1_data_lower_bound = target_lposc1_data - lposc_variation_data - 2;


        lposc2_winset = (uint32_t)(((double)expect_fqmtr_data * dcxo_freq_khz) / LPOSC2_TARGET_KHZ - 1 + 0.5);
        target_lposc2_data = ((double)(lposc2_winset + 1) * LPOSC2_TARGET_KHZ) / dcxo_freq_khz;

        lposc_variation_data = ((double)target_lposc2_data + 0.5) * 0.0025;
        lposc2_data_upper_bound = target_lposc2_data + lposc_variation_data + 2;
        lposc2_data_lower_bound = target_lposc2_data - lposc_variation_data - 2;

        clk_recali_dbg("lposc1_winset %d, lposc2_winset %d", 2, lposc1_winset, lposc2_winset);
        clk_recali_dbg("target_lposc1_data %d, target_lposc2_data %d", 2, target_lposc1_data, target_lposc2_data);

        if (HAL_GPT_STATUS_OK != hal_gpt_sw_get_timer(&lposc_recali_handle)) {
            clk_recali_print("lposc_recali_handle error!", 0);
        }

        recali_init_done = 1;

        lposc_recali_cali_cur[0] = LPOSC1_CON_REG.DA_LPOSC_RBANK_CALI;
        lposc_recali_ft_cur[0] = LPOSC1_CON_REG.DA_LPOSC_RBANK_FT;
        lposc_recali_cali_cur[1] = LPOSC2_CON_REG.DA_LPOSC_RBANK_CALI;
        lposc_recali_ft_cur[1] = LPOSC2_CON_REG.DA_LPOSC_RBANK_FT;
        lposc_1st_time_k_fqmtr_data[0] = hal_clock_freq_meter_data(XO_CK, AD_OSC1_CK, lposc1_winset);
        lposc_1st_time_k_fqmtr_data[1] = hal_clock_freq_meter_data(XO_CK, AD_OSC2_CK, lposc2_winset);
    }

    //printf("lposc1_winset %d, lposc2_winset %d\r\n", lposc1_winset, lposc2_winset);
    //printf("target_lposc1_data %d, target_lposc2_data %d\r\n", target_lposc1_data, target_lposc2_data);
    //printf("target_lposc1_data upper bound %d, lower bound %d\r\n", lposc1_data_upper_bound, lposc1_data_lower_bound);
    //printf("target_lposc2_data upper bound %d, lower bound %d\r\n", lposc2_data_upper_bound, lposc2_data_lower_bound);

    hal_gpt_sw_start_timer_ms(lposc_recali_handle, LPOSC_RECALI_INTERVAL_MS, lposc_recali_gpt_callback, NULL);
}

/* LPOSC recalibration gpt sw timer callback, registers actual LPOSC SW tracking task using system_daemon task */
void lposc_recali_gpt_callback(void *arg)
{
    clk_recali_dbg("before invoke", 0);
    system_daemon_clk_invoke();
    clk_recali_dbg("after invoke", 0);
}

#endif /* defined(CLK_SW_TRACKING_TASK_ENABLE) */

void clock_dump_info(void)
{
#if !defined(FPGA_ENV) && !defined(__EXT_BOOTLOADER__) && !defined(__EXT_DA__)
    uint32_t winset, winset1;

    winset = fqmtr_winset_estimate(DCXO_HZ);
    clk_dbg_print("[CLK_DUMP] AD_DCXO_CLK26M: %d kHz", 1, hal_clock_get_freq_meter(AD_DCXO_CLK26M, winset));

    winset = fqmtr_winset_estimate(LPOSC1_HZ);
    clk_dbg_print("[CLK_DUMP] AD_OSC1_CK %d kHz: %d kHz", 2, LPOSC1_TARGET_KHZ, hal_clock_get_freq_meter(AD_OSC1_CK, winset));

    winset = fqmtr_winset_estimate(LPOSC2_HZ);
    clk_dbg_print("[CLK_DUMP] AD_OSC2_CK %d kHz: %d kHz", 2, LPOSC2_TARGET_KHZ, hal_clock_get_freq_meter(AD_OSC2_CK, winset));

    winset = fqmtr_winset_estimate(APLL1_HZ);
    winset1 = fqmtr_winset_estimate(APLL2_HZ);
    clk_dbg_print("[CLK_DUMP] AD_APLL_CK: %d %d kHz", 2, hal_clock_get_freq_meter(AD_APLL1_CK, winset), hal_clock_get_freq_meter(AD_APLL2_CK, winset1));

    winset = fqmtr_winset_estimate(UPLL_HZ);
    winset1 = fqmtr_winset_estimate(24000000);
    clk_dbg_print("[CLK_DUMP] AD_UPLL, 48M_24M_CK: %d %d kHz", 2, hal_clock_get_freq_meter(AD_UPLL_CK, winset), hal_clock_get_freq_meter(AD_UPLL_48M_24M_CK, winset1));

    // ------------------------------------------------------------------------
    winset = 1000;  // fixed, because doesn't know the target frequency

    clk_dbg_print("[CLK_DUMP] SYS mux %d: %d kHz", 2, clock_mux_cur_sel(CLK_SYS_SEL), hal_clock_get_freq_meter(hf_fsys_ck, winset));
    clk_dbg_print("[CLK_DUMP] DSP mux %d: %d kHz", 2, clock_mux_cur_sel(CLK_DSP_SEL), hal_clock_get_freq_meter(hf_fdsp_ck, winset));
    clk_dbg_print("[CLK_DUMP] SFC mux %d: %d kHz", 2, clock_mux_cur_sel(CLK_SFC_SEL), hal_clock_get_freq_meter(hf_fsfc_ck, winset));
    clk_dbg_print("[CLK_DUMP] ESC mux %d: %d kHz", 2, clock_mux_cur_sel(CLK_ESC_SEL), hal_clock_get_freq_meter(hf_fesc_ck, winset));
    clk_dbg_print("[CLK_DUMP] SPIM mux %d %d %d: %d %d %d kHz", 6, clock_mux_cur_sel(CLK_SPIMST0_SEL), clock_mux_cur_sel(CLK_SPIMST1_SEL),
                  clock_mux_cur_sel(CLK_SPIMST2_SEL), hal_clock_get_freq_meter(hf_fspimst0_ck, winset),
                  hal_clock_get_freq_meter(hf_fspimst1_ck, winset), hal_clock_get_freq_meter(hf_fspimst2_ck, winset));
    clk_dbg_print("[CLK_DUMP] USB mux %d: %d kHz", 2, clock_mux_cur_sel(CLK_USB_SEL), hal_clock_get_freq_meter(hf_fusb_ck, winset));
    clk_dbg_print("[CLK_DUMP] I3C mux %d: %d kHz", 2, clock_mux_cur_sel(CLK_I3C_SEL), hal_clock_get_freq_meter(hf_fi3c_ck, winset));
    clk_dbg_print("[CLK_DUMP] BT_HOP mux %d: %d kHz", 2, clock_mux_cur_sel(CLK_BT_HOP_SEL), hal_clock_get_freq_meter(hf_fbt_hop_ck, winset));
    clk_dbg_print("[CLK_DUMP] AUD_INTBUS mux %d: %d kHz", 2, clock_mux_cur_sel(CLK_AUD_BUS_SEL), hal_clock_get_freq_meter(hf_faud_intbus_ck, winset));
    clk_dbg_print("[CLK_DUMP] AUD_GPSRC mux %d: %d kHz", 2, clock_mux_cur_sel(CLK_AUD_GPSRC_SEL), hal_clock_get_freq_meter(hf_faud_gpsrc_ck, winset));
    clk_dbg_print("[CLK_DUMP] AUD_UPLINK mux %d: %d kHz", 2, clock_mux_cur_sel(CLK_AUD_ULCK_SEL), hal_clock_get_freq_meter(hf_faud_uplink_ck, winset));
    clk_dbg_print("[CLK_DUMP] AUD_DWLINK mux %d: %d kHz", 2, clock_mux_cur_sel(CLK_AUD_DLCK_SEL), hal_clock_get_freq_meter(hf_faud_dwlink_ck, winset));
    clk_dbg_print("[CLK_DUMP] AUD_INTF mux %d %d: %d %d kHz", 4, clock_mux_cur_sel(CLK_AUD_INTERFACE0_SEL), clock_mux_cur_sel(CLK_AUD_INTERFACE1_SEL),
                  hal_clock_get_freq_meter(hf_faud_int0_ck, winset), hal_clock_get_freq_meter(hf_faud_int1_ck, winset));
    clk_dbg_print("[CLK_DUMP] 26M mux %d: %d kHz", 2, clock_mux_cur_sel(CLK_26M_SEL), hal_clock_get_freq_meter(f_f26m_ck, winset));
    clk_dbg_print("[CLK_DUMP] AUD_ENGINE mux %d: %d kHz", 2, clock_mux_cur_sel(CLK_AUD_ENGINE_SEL), hal_clock_get_freq_meter(hf_faud_engine_ck, winset));
    clk_dbg_print("[CLK_DUMP] AUD_VOW mux %d: %d kHz", 2, clock_mux_cur_sel(CLK_VOW_SEL), hal_clock_get_freq_meter(hf_faud_vow_ck, winset));

    // ------------------------------------------------------------------------
    clk_dbg_print("[CLK_DUMP] UART ck: %d %d %d", 3, hal_clock_is_enabled(HAL_CLOCK_CG_UART0),
                  hal_clock_is_enabled(HAL_CLOCK_CG_UART1), hal_clock_is_enabled(HAL_CLOCK_CG_UART2));
    clk_dbg_print("[CLK_DUMP] UART_DMA ck: %d %d %d", 3, hal_clock_is_enabled(HAL_CLOCK_CG_UART_DMA_0),
                  hal_clock_is_enabled(HAL_CLOCK_CG_UART_DMA_1), hal_clock_is_enabled(HAL_CLOCK_CG_UART_DMA_2));
    clk_dbg_print("[CLK_DUMP] IRRX&BUS ck: %d %d", 2, hal_clock_is_enabled(HAL_CLOCK_CG_IRRX), hal_clock_is_enabled(HAL_CLOCK_CG_IRRX_BUS));
    clk_dbg_print("[CLK_DUMP] I3C ck: %d %d", 2, hal_clock_is_enabled(HAL_CLOCK_CG_I3C0), hal_clock_is_enabled(HAL_CLOCK_CG_I3C1));
    clk_dbg_print("[CLK_DUMP] I2C ck: %d %d %d", 3, hal_clock_is_enabled(HAL_CLOCK_CG_I2C0), hal_clock_is_enabled(HAL_CLOCK_CG_I2C1), hal_clock_is_enabled(HAL_CLOCK_CG_I2C2));
    clk_dbg_print("[CLK_DUMP] ROSC ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_ROSC));
    clk_dbg_print("[CLK_DUMP] AUXADC ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_AUXADC));
    clk_dbg_print("[CLK_DUMP] PWM ck: %d %d %d %d %d %d %d %d %d %d %d %d %d %d", 14, hal_clock_is_enabled(HAL_CLOCK_CG_PWM0), hal_clock_is_enabled(HAL_CLOCK_CG_PWM1),
                  hal_clock_is_enabled(HAL_CLOCK_CG_PWM2), hal_clock_is_enabled(HAL_CLOCK_CG_PWM3), hal_clock_is_enabled(HAL_CLOCK_CG_PWM4), hal_clock_is_enabled(HAL_CLOCK_CG_PWM5),
                  hal_clock_is_enabled(HAL_CLOCK_CG_PWM6), hal_clock_is_enabled(HAL_CLOCK_CG_PWM7), hal_clock_is_enabled(HAL_CLOCK_CG_PWM8), hal_clock_is_enabled(HAL_CLOCK_CG_PWM9),
                  hal_clock_is_enabled(HAL_CLOCK_CG_PWM10), hal_clock_is_enabled(HAL_CLOCK_CG_PWM11), hal_clock_is_enabled(HAL_CLOCK_CG_PWM12), hal_clock_is_enabled(HAL_CLOCK_CG_PWM13));
    clk_dbg_print("[CLK_DUMP] SPM_PCLK ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_SPM_PCLK));
    clk_dbg_print("[CLK_DUMP] BCLK_CM33 ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_BCLK_CM33));
    clk_dbg_print("[CLK_DUMP] BCLK_DSP ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_BCLK_DSP));
    clk_dbg_print("[CLK_DUMP] SPM_DIV ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_SPM_DIV));
    clk_dbg_print("[CLK_DUMP] I2C_AO ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_I2C_AO));
    clk_dbg_print("[CLK_DUMP] OSTIMER ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_OSTIMER));
    clk_dbg_print("[CLK_DUMP] GPTIMER ck: %d %d", 2, hal_clock_is_enabled(HAL_CLOCK_CG_GPTIMER0), hal_clock_is_enabled(HAL_CLOCK_CG_GPTIMER1));
    clk_dbg_print("[CLK_DUMP] SPIMST ck: %d %d %d", 3, hal_clock_is_enabled(HAL_CLOCK_CG_SPIMST0), hal_clock_is_enabled(HAL_CLOCK_CG_SPIMST1), hal_clock_is_enabled(HAL_CLOCK_CG_SPIMST2));
    clk_dbg_print("[CLK_DUMP] SDIOMST ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_SDIOMST));
    clk_dbg_print("[CLK_DUMP] I3C ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_I3C));
    clk_dbg_print("[CLK_DUMP] BT_HOP ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_BT_HOP));
    clk_dbg_print("[CLK_DUMP] AUD_GPSRC ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_AUD_GPSRC));
    clk_dbg_print("[CLK_DUMP] AUD_UPLINK ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_AUD_UPLINK));
    clk_dbg_print("[CLK_DUMP] AUD_DWLINK ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_AUD_DWLINK));
    clk_dbg_print("[CLK_DUMP] SPDIF ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_SPDIF));
    clk_dbg_print("[CLK_DUMP] AUD_INTF ck: %d %d", 2, hal_clock_is_enabled(HAL_CLOCK_CG_AUD_INTF0), hal_clock_is_enabled(HAL_CLOCK_CG_AUD_INTF1));
    clk_dbg_print("[CLK_DUMP] AUD_TEST ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_AUD_TEST));
    clk_dbg_print("[CLK_DUMP] AUD_ANC ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_AUD_ANC));
    clk_dbg_print("[CLK_DUMP] AUD_CLD ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_AUD_CLD));
    clk_dbg_print("[CLK_DUMP] SFC ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_SFC));
    clk_dbg_print("[CLK_DUMP] ESC ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_ESC));
    clk_dbg_print("[CLK_DUMP] SPISLV ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_SPISLV));
    clk_dbg_print("[CLK_DUMP] USB ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_USB));
    clk_dbg_print("[CLK_DUMP] AUD_INTBUS ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_AUD_INTBUS));
    clk_dbg_print("[CLK_DUMP] DSP ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_DSP));
    clk_dbg_print("[CLK_DUMP] SEJ ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_SEJ));
    clk_dbg_print("[CLK_DUMP] MIXEDSYS ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_MIXEDSYS));
    clk_dbg_print("[CLK_DUMP] EFUSE ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_EFUSE));
    clk_dbg_print("[CLK_DUMP] OSC1_D12_D2 ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_OSC1_D12_D2));
    clk_dbg_print("[CLK_DUMP] OSC2_D10_D2 ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_OSC2_D10_D2));
    clk_dbg_print("[CLK_DUMP] CMSYS_BUS ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_CMSYS_BUS));
    clk_dbg_print("[CLK_DUMP] RSV_BUS ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_RSV_BUS));
    clk_dbg_print("[CLK_DUMP] CONN_BUS ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_CONN_BUS));
    clk_dbg_print("[CLK_DUMP] RSV_RTC_BUS ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_RSV_RTC_BUS));
    clk_dbg_print("[CLK_DUMP] RSV_MCLK_BUS ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_RSV_MCLK_BUS));
    clk_dbg_print("[CLK_DUMP] AUD_ENGINE_BUS ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_AUD_ENGINE_BUS));
    clk_dbg_print("[CLK_DUMP] AUD_VOW_BUS ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_AUD_VOW_BUS));
    clk_dbg_print("[CLK_DUMP] FAST_DMA ck: %d %d", 2, hal_clock_is_enabled(HAL_CLOCK_CG_FAST_DMA0), hal_clock_is_enabled(HAL_CLOCK_CG_FAST_DMA1));
    clk_dbg_print("[CLK_DUMP] SPIMST_BUS ck: %d %d %d", 3, hal_clock_is_enabled(HAL_CLOCK_CG_SPIMST0_BUS), hal_clock_is_enabled(HAL_CLOCK_CG_SPIMST1_BUS), hal_clock_is_enabled(HAL_CLOCK_CG_SPIMST2_BUS));
    clk_dbg_print("[CLK_DUMP] I3C_DMA ck: %d %d", 2, hal_clock_is_enabled(HAL_CLOCK_CG_I3C_DMA0), hal_clock_is_enabled(HAL_CLOCK_CG_I3C_DMA1));
    clk_dbg_print("[CLK_DUMP] I2C_DMA ck: %d %d %d", 3, hal_clock_is_enabled(HAL_CLOCK_CG_I2C_DMA0), hal_clock_is_enabled(HAL_CLOCK_CG_I2C_DMA1), hal_clock_is_enabled(HAL_CLOCK_CG_I2C_DMA2));
    clk_dbg_print("[CLK_DUMP] SECURITY_PD ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_SECURITY_PD));
    clk_dbg_print("[CLK_DUMP] AESOTF&ESC ck: %d %d", 2, hal_clock_is_enabled(HAL_CLOCK_CG_AESOTF), hal_clock_is_enabled(HAL_CLOCK_CG_AESOTF_ESC));
    clk_dbg_print("[CLK_DUMP] CRYPTO ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_CRYPTO));
    clk_dbg_print("[CLK_DUMP] TRNG ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_TRNG));
    clk_dbg_print("[CLK_DUMP] SPISLV_BUS ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_SPISLV_BUS));
    clk_dbg_print("[CLK_DUMP] SDIOMST0 ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_SDIOMST0));
    clk_dbg_print("[CLK_DUMP] USB_BUS ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_USB_BUS));
    clk_dbg_print("[CLK_DUMP] USB_DMA ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_USB_DMA));
    clk_dbg_print("[CLK_DUMP] SECURITY_PERI ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_SECURITY_PERI));
    clk_dbg_print("[CLK_DUMP] BUS_ERR ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_BUS_ERR));
    clk_dbg_print("[CLK_DUMP] SEC_MON ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_SEC_MON));
    clk_dbg_print("[CLK_DUMP] CMSYS_ROM ck %d", 1, hal_clock_is_enabled(HAL_CLOCK_CG_CMSYS_ROM));

    // ------------------------------------------------------------------------
    clk_dbg_print("[CLK_DUMP] OSC1_DIV D2 D3 D5 D12: %d %d %d %d %d", 5, CKSYS_CLK_DIV_REG.CLK_OSC1_DIV_EN, CKSYS_CLK_DIV_REG.CLK_OSC1_D2_EN,
                  CKSYS_CLK_DIV_REG.CLK_OSC1_D3_EN, CKSYS_CLK_DIV_REG.CLK_OSC1_D5_EN, CKSYS_CLK_DIV_REG.CLK_OSC1_D12_EN);
    clk_dbg_print("[CLK_DUMP] OSC2_DIV D2 D3 D5 D12: %d %d %d %d %d", 5, CKSYS_CLK_DIV_REG.CLK_OSC2_DIV_EN, CKSYS_CLK_DIV_REG.CLK_OSC2_D2_EN,
                  CKSYS_CLK_DIV_REG.CLK_OSC2_D3_EN, CKSYS_CLK_DIV_REG.CLK_OSC2_D5_EN, CKSYS_CLK_DIV_REG.CLK_OSC2_D12_EN);
    clk_dbg_print("[CLK_DUMP] UPLL_DIV D2 D3 D5: %d %d %d %d", 4, CKSYS_CLK_DIV_REG.CLK_PLL1_DIV_EN,
                  CKSYS_CLK_DIV_REG.CLK_PLL1_D2_EN, CKSYS_CLK_DIV_REG.CLK_PLL1_D3_EN, CKSYS_CLK_DIV_REG.CLK_PLL1_D5_EN);
#endif /* #if !defined(FPGA_ENV) && !defined(__EXT_BOOTLOADER__) && !defined(__EXT_DA__) */
}

/* (*): OSC1 related freqs depends on whether AIR_RFI_SUPPRESS_DISABLE is defined
 *      when AIR_RFI_SUPPRESS_DISABLE is     defined: freq is the higher value
 *                                       not defined: freq is the lower value
 *    OSC1 freq. code comment format
 *    -> "defined freq"/"not defined freq"
 */
ATTR_TEXT_IN_TCM const static clock_tbl_t clock_tbl = {
    .mux_tbl = {
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},     //CLK_SYS_SEL        // 0 : F_FXO_CK     26MHz
                {CLK_OSC1, CLK_DIV_NONE, MUX_NOT_ALLOW},                // 1 : OSC_26       26MHz
                {CLK_OSC1, CLK_DIV_D2, MUX_ALLOW},                      // 2 : OSC1_D2      156/149.625MHz (*)
                {CLK_OSC1, CLK_DIV_D3, MUX_ALLOW},                      // 3 : OSC1_D3      104/99.75MHz   (*)
                {CLK_OSC1, CLK_DIV_D2, MUX_ALLOW},                      // 4 : OSC1_D2_D2   78/74.8125MHz  (*)
                {CLK_OSC2, CLK_DIV_D2, MUX_ALLOW},                      // 5 : OSC2_D2      260MHz
                {CLK_OSC2, CLK_DIV_D2, MUX_ALLOW},                      // 6 : OSC2_D2_D2   130MHz
                {CLK_UPLL, CLK_DIV_D2, MUX_ALLOW}                       // 7 : UPLL_D2      156MHz
            },
            .mux_type = PHYSICAL_N_TO_1_MUX,
            .curr_sel_ptr = BYTE_REG(CKSYS_CLK_CFG_REG.CLK_SYS_SEL),
            .cg_info = { .cg_id = HAL_CLOCK_CG_CMSYS_BUS, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},     //CLK_DSP_SEL        // 0 : F_FXO_CK     26MHz
                {CLK_OSC1, CLK_DIV_NONE, MUX_NOT_ALLOW},                // 1 : OSC_26       26MHz
                {CLK_OSC1, CLK_DIV_NONE, MUX_ALLOW},                    // 2 : OSC1         312/299.25MHz   (*)
                {CLK_OSC1, CLK_DIV_D2, MUX_ALLOW},                      // 3 : OSC1_D2      156/149.625MHz  (*)
                {CLK_OSC1, CLK_DIV_D3, MUX_ALLOW},                      // 4 : OSC1_D3      104/99.75MHz    (*)
                {CLK_OSC2, CLK_DIV_NONE, MUX_ALLOW},                    // 5 : OSC2         520MHz
                {CLK_OSC2, CLK_DIV_D5, MUX_ALLOW},                      // 6 : OSC2_D5      104MHz
                {CLK_UPLL, CLK_DIV_D2, MUX_ALLOW}                       // 7 : UPLL_D2      156MHz
            },
            .mux_type = PHYSICAL_N_TO_1_MUX,
            .curr_sel_ptr = BYTE_REG(CKSYS_CLK_CFG_REG.CLK_DSP_SEL),
            .cg_info = { .cg_id = HAL_CLOCK_CG_DSP, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO,  CLK_DIV_NONE, MUX_ALLOW},//CLK_SFC_SEL            // 0 : F_FXO_CK     26MHz
                {CLK_OSC1, CLK_DIV_NONE, MUX_NOT_ALLOW},                // 1 : OSC_26       26MHz
                {CLK_OSC1, CLK_DIV_D3, MUX_ALLOW},                      // 2 : OSC1_D3      104/99.75MHz   (*)
                {CLK_OSC1, CLK_DIV_D3, MUX_ALLOW},                      // 3 : OSC1_D3_D2   52/49.875MHz   (*)
                {CLK_OSC1, CLK_DIV_D8, MUX_ALLOW},                      // 4 : OSC1_D8      39/37.40625MHz (*)
                {CLK_OSC2, CLK_DIV_D3, MUX_NOT_ALLOW},                  // 5 : OSC2_D3      173.33MHz
                {CLK_OSC2, CLK_DIV_D5, MUX_NOT_ALLOW},                  // 6 : OSC2_D5      104MHz
                {CLK_UPLL, CLK_DIV_D3, MUX_ALLOW}                       // 7 : UPLL_D3      104MHz
            },
            .mux_type = PHYSICAL_N_TO_1_MUX,
            .curr_sel_ptr = BYTE_REG(CKSYS_CLK_CFG_REG.CLK_SFC_SEL),
            .cg_info = { .cg_id = HAL_CLOCK_CG_SFC, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO,  CLK_DIV_NONE, MUX_ALLOW},//CLK_ESC_SEL            // 0 : F_FXO_CK     26MHz
                {CLK_OSC1, CLK_DIV_D3, MUX_ALLOW},                      // 1 : OSC1_D3      104/99.75MHz    (*)
                {CLK_OSC1, CLK_DIV_D2, MUX_ALLOW},                      // 2 : OSC1_D2_D2   78/74.8125MHz   (*)
                {CLK_OSC1, CLK_DIV_D3, MUX_ALLOW},                      // 3 : OSC1_D3_D2   52/49.875MHz    (*)
                {CLK_OSC1, CLK_DIV_D8, MUX_ALLOW},                      // 4 : OSC1_D8      39/37.40625MHz  (*)
                {CLK_OSC2, CLK_DIV_D5, MUX_NOT_ALLOW},                  // 5 : OSC2_D5      104MHz
                {CLK_OSC2, CLK_DIV_D3, MUX_NOT_ALLOW},                  // 6 : OSC2_D3_D2   86.67MHz
                {CLK_UPLL, CLK_DIV_D3, MUX_ALLOW}                       // 7 : UPLL_D3      104MHz
            },
            .mux_type = PHYSICAL_N_TO_1_MUX,
            .curr_sel_ptr = BYTE_REG(CKSYS_CLK_CFG_REG.CLK_ESC_SEL),
            .cg_info = { .cg_id = HAL_CLOCK_CG_ESC, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},    //CLK_SPIMST0_SEL     // 0 : F_FXO_CK     26MHz
                {CLK_OSC1, CLK_DIV_D3, MUX_ALLOW},                      // 1 : OSC1_D3      104/99.75MHz    (*)
                {CLK_OSC1, CLK_DIV_D2, MUX_ALLOW},                      // 2 : OSC1_D2_D2   78/74.8125MHz   (*)
                {CLK_OSC1, CLK_DIV_D3, MUX_ALLOW},                      // 3 : OSC1_D3_D2   52/49.875MHz    (*)
                {CLK_OSC1, CLK_DIV_D8, MUX_ALLOW},                      // 4 : OSC1_D8      39/37.40625MHz  (*)
                {CLK_OSC2, CLK_DIV_D5, MUX_NOT_ALLOW},                  // 5 : OSC2_D5      104MHz
                {CLK_OSC2, CLK_DIV_D3, MUX_NOT_ALLOW},                  // 6 : OSC2_D3_D2   86.67MHz
                {CLK_UPLL, CLK_DIV_D3, MUX_ALLOW}                       // 7 : UPLL_D3      104MHz
            },
            .mux_type = PHYSICAL_N_TO_1_MUX,
            .curr_sel_ptr = BYTE_REG(CKSYS_CLK_CFG_REG.CLK_SPIMST0_SEL),
            .cg_info = { .cg_id = HAL_CLOCK_CG_SPIMST0_BUS, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},    //CLK_SPIMST1_SEL     // 0 : F_FXO_CK     26MHz
                {CLK_OSC1, CLK_DIV_D3, MUX_ALLOW},                      // 1 : OSC1_D3      104/99.75MHz    (*)
                {CLK_OSC1, CLK_DIV_D2, MUX_ALLOW},                      // 2 : OSC1_D2_D2   78/74.8125MHz   (*)
                {CLK_OSC1, CLK_DIV_D3, MUX_ALLOW},                      // 3 : OSC1_D3_D2   52/49.875MHz    (*)
                {CLK_OSC1, CLK_DIV_D8, MUX_ALLOW},                      // 4 : OSC1_D8      39/37.40625MHz  (*)
                {CLK_OSC2, CLK_DIV_D5, MUX_NOT_ALLOW},                  // 5 : OSC2_D5      104MHz
                {CLK_OSC2, CLK_DIV_D3, MUX_NOT_ALLOW},                  // 6 : OSC2_D3_D2   86.67MHz
                {CLK_UPLL, CLK_DIV_D3, MUX_ALLOW}                       // 7 : UPLL_D3      104MHz
            },
            .mux_type = PHYSICAL_N_TO_1_MUX,
            .curr_sel_ptr = BYTE_REG(CKSYS_CLK_CFG_REG.CLK_SPIMST1_SEL),
            .cg_info = { .cg_id = HAL_CLOCK_CG_SPIMST1_BUS, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},    //CLK_SPIMST2_SEL     // 0 : F_FXO_CK     26MHz
                {CLK_OSC1, CLK_DIV_D3, MUX_ALLOW},                      // 1 : OSC1_D3      104/99.75MHz    (*)
                {CLK_OSC1, CLK_DIV_D2, MUX_ALLOW},                      // 2 : OSC1_D2_D2   78/74.8125MHz   (*)
                {CLK_OSC1, CLK_DIV_D3, MUX_ALLOW},                      // 3 : OSC1_D3_D2   52/49.875MHz    (*)
                {CLK_OSC1, CLK_DIV_D8, MUX_ALLOW},                      // 4 : OSC1_D8      39/37.40625MHz  (*)
                {CLK_OSC2, CLK_DIV_D5, MUX_NOT_ALLOW},                  // 5 : OSC2_D5      104MHz
                {CLK_OSC2, CLK_DIV_D3, MUX_NOT_ALLOW},                  // 6 : OSC2_D3_D2   86.67MHz
                {CLK_UPLL, CLK_DIV_D3, MUX_ALLOW}                       // 7 : UPLL_D3      104MHz
            },
            .mux_type = PHYSICAL_N_TO_1_MUX,
            .curr_sel_ptr = BYTE_REG(CKSYS_CLK_CFG_REG.CLK_SPIMST2_SEL),
            .cg_info = { .cg_id = HAL_CLOCK_CG_SPIMST2_BUS, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},    //CLK_SPISLV_SEL     // 0 : F_FXO_CK     26MHz
                {CLK_OSC1, CLK_DIV_D3, MUX_ALLOW},                     // 1 : OSC1_D3      104/99.75MHz    (*)
                {CLK_OSC1, CLK_DIV_D2, MUX_ALLOW},                     // 2 : OSC1_D2_D2   78/74.8125MHz   (*)
                {CLK_OSC1, CLK_DIV_D3, MUX_ALLOW},                     // 3 : OSC1_D3_D2   52/49.875MHz    (*)
                {CLK_OSC1, CLK_DIV_D8, MUX_ALLOW},                     // 4 : OSC1_D8      39/37.40625MHz  (*)
                {CLK_OSC2, CLK_DIV_D5, MUX_NOT_ALLOW},                 // 5 : OSC2_D5      104MHz
                {CLK_OSC2, CLK_DIV_D3, MUX_NOT_ALLOW},                 // 6 : OSC2_D3_D2   86.67MHz
                {CLK_UPLL, CLK_DIV_D3, MUX_ALLOW}                      // 7 : UPLL_D3      104MHz
            },
            .mux_type = PHYSICAL_N_TO_1_MUX,
            .curr_sel_ptr = BYTE_REG(CKSYS_CLK_CFG_REG.CLK_SPISLV_SEL),
            .cg_info = { .cg_id = HAL_CLOCK_CG_SPISLV_BUS, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},    //CLK_SDIOMST0_SEL    // 0 : F_FXO_CK     26MHz
                {CLK_OSC1, CLK_DIV_D3, MUX_ALLOW},                      // 1 : OSC1_D3_D2   52/49.875MHz      (*)
                {CLK_OSC1, CLK_DIV_D8, MUX_ALLOW},                      // 2 : OSC1_D8      39/37.40625MHz    (*)
                {CLK_OSC1, CLK_DIV_D12, MUX_ALLOW},                     // 3 : OSC1_D12     26/24.9375MHz     (*)
                {CLK_OSC1, CLK_DIV_D8, MUX_ALLOW},                      // 4 : OSC1_D8_D2   19.5/18.703125MHz (*)
                {CLK_OSC2, CLK_DIV_D5, MUX_NOT_ALLOW},                  // 5 : OSC2_D5_D2   52MHz
                {CLK_OSC2, CLK_DIV_D12, MUX_NOT_ALLOW},                 // 6 : OSC2_D12     43.33MHz
                {CLK_UPLL, CLK_DIV_D3, MUX_ALLOW}                       // 7 : UPLL_D3      104MHz
            },
            .mux_type = PHYSICAL_N_TO_1_MUX,
            .curr_sel_ptr = BYTE_REG(CKSYS_CLK_CFG_REG.CLK_SDIOMST0_SEL),
            .cg_info = { .cg_id = HAL_CLOCK_CG_SDIOMST0, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},    //CLK_USB_SEL         // 0 : F_FXO_CK     26MHz
                {CLK_OSC1, CLK_DIV_D12, MUX_ALLOW},                     // 1 : OSC1_D12     26/24.9375MHz    (*)
                {CLK_OSC2, CLK_DIV_D5, MUX_NOT_ALLOW},                  // 2 : OSC2_D5_D2   52MHz
                {CLK_UPLL, CLK_DIV_D5, MUX_ALLOW},                      // 3 : UPLL_D5      62.4MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PHYSICAL_N_TO_1_MUX,
            .curr_sel_ptr = BYTE_REG(CKSYS_CLK_CFG_REG.CLK_USB_SEL),
            .cg_info = { .cg_id = HAL_CLOCK_CG_USB, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},    //CLK_I3C_SEL         // 0 : F_FXO_CK     26MHz
                {CLK_OSC1, CLK_DIV_D3, MUX_ALLOW},                      // 1 : OSC1_D3      104/99.75MHz    (*)
                {CLK_OSC1, CLK_DIV_D3, MUX_NOT_ALLOW},                  // 2 : OSC1_D3_D2   52/49.875MHz    (*)
                {CLK_OSC2, CLK_DIV_D5, MUX_ALLOW},                      // 3 : OSC2_D5      104MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PHYSICAL_N_TO_1_MUX,
            .curr_sel_ptr = BYTE_REG(CKSYS_CLK_CFG_REG.CLK_I3C_SEL),
            .cg_info = { .cg_id = HAL_CLOCK_CG_I3C, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},    //CLK_BT_HOP_SEL      // 0 : F_FXO_CK     26MHz
                {CLK_OSC1, CLK_DIV_D3, MUX_ALLOW},                      // 1 : OSC1_D3      104/99.75MHz    (*)
                {CLK_OSC1, CLK_DIV_D3, MUX_ALLOW},                      // 2 : OSC1_D3_D2   52/49.875MHz    (*)
                {CLK_OSC2, CLK_DIV_D5, MUX_NOT_ALLOW},                  // 3 : OSC2_D5      104MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PHYSICAL_N_TO_1_MUX,
            .curr_sel_ptr = BYTE_REG(CKSYS_CLK_CFG_REG.CLK_BT_HOP_SEL),
            .cg_info = { .cg_id = HAL_CLOCK_CG_BT_HOP, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW}, //CLK_AUD_BUS_SEL        // 0 : F_FXO_CK     26MHz
                {CLK_OSC1, CLK_DIV_D2, MUX_ALLOW},                      // 1 : OSC1_D2      156/149.625MHz  (*)
                {CLK_OSC1, CLK_DIV_D3, MUX_ALLOW},                      // 2 : OSC1_D3      104/99.75MHz    (*)
                {CLK_OSC1, CLK_DIV_D2, MUX_ALLOW},                      // 3 : OSC1_D2_D2   78/74.8125MHz   (*)
                {CLK_OSC1, CLK_DIV_D3, MUX_ALLOW},                      // 4 : OSC1_D3_D2   52/49.875MHz    (*)
                {CLK_OSC2, CLK_DIV_D2, MUX_NOT_ALLOW},                  // 5 : OSC2_D2      260MHz
                {CLK_OSC2, CLK_DIV_D3, MUX_NOT_ALLOW},                  // 6 : OSC2_D3      173.33MHz
                {CLK_OSC2, CLK_DIV_D2, MUX_NOT_ALLOW}                   // 7 : OSC2_D2_D2   130MHz
            },
            .mux_type = PHYSICAL_N_TO_1_MUX,
            .curr_sel_ptr = BYTE_REG(CKSYS_CLK_CFG_REG.CLK_AUD_BUS_SEL),
            .cg_info = { .cg_id = HAL_CLOCK_CG_AUD_INTBUS, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},    //CLK_AUD_GPSRC_SEL  // 0 : F_FXO_CK     26MHz
                {CLK_OSC1, CLK_DIV_D2, MUX_ALLOW},                     // 1 : OSC1_D2      156/149.625MHz   (*)
                {CLK_OSC1, CLK_DIV_D3, MUX_ALLOW},                     // 2 : OSC1_D3      104/99.75MHz     (*)
                {CLK_OSC1, CLK_DIV_D2, MUX_ALLOW},                     // 3 : OSC1_D2_D2   78/74.8125MHz    (*)
                {CLK_OSC1, CLK_DIV_D3, MUX_ALLOW},                     // 4 : OSC1_D3_D2   52/49.875MHz     (*)
                {CLK_OSC2, CLK_DIV_D2, MUX_NOT_ALLOW},                 // 5 : OSC2_D2      260MHz
                {CLK_OSC2, CLK_DIV_D3, MUX_NOT_ALLOW},                 // 6 : OSC2_D3      173.33MHz
                {CLK_OSC2, CLK_DIV_D2, MUX_ALLOW}                      // 7 : OSC2_D2_D2   130MHz
            },
            .mux_type = PHYSICAL_N_TO_1_MUX,
            .curr_sel_ptr = BYTE_REG(CKSYS_CLK_CFG_REG.CLK_AUD_GPSRC_SEL),
            .cg_info = { .cg_id = HAL_CLOCK_CG_AUD_GPSRC, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},     //CLK_AUD_ULCK_SEL   // 0 : F_FXO_CK     26MHz
                {CLK_OSC1, CLK_DIV_NONE, MUX_ALLOW},                    // 1 : OSC1         312/299.25MHz   (*)
                {CLK_OSC1, CLK_DIV_D2, MUX_ALLOW},                      // 2 : OSC1_D2      156/149.625MHz  (*)
                {CLK_OSC2, CLK_DIV_D2, MUX_NOT_ALLOW},                  // 3 : OSC2_D2      260MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PHYSICAL_N_TO_1_MUX,
            .curr_sel_ptr = BYTE_REG(CKSYS_CLK_CFG_REG.CLK_AUD_ULCK_SEL),
            .cg_info = { .cg_id = HAL_CLOCK_CG_AUD_UPLINK, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},     //CLK_AUD_DLCK_SEL   // 0 : F_FXO_CK     26MHz
                {CLK_OSC1, CLK_DIV_D2, MUX_ALLOW},                      // 1 : OSC1_D2      156/149.625MHz  (*)
                {CLK_OSC1, CLK_DIV_D2, MUX_ALLOW},                      // 2 : OSC1_D2_D2   78/74.8125MHz   (*)
                {CLK_OSC2, CLK_DIV_D2, MUX_NOT_ALLOW},                  // 3 : OSC2_D2_D2   130MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PHYSICAL_N_TO_1_MUX,
            .curr_sel_ptr = BYTE_REG(CKSYS_CLK_CFG_REG.CLK_AUD_DLCK_SEL),
            .cg_info = { .cg_id = HAL_CLOCK_CG_AUD_DWLINK, .cg_type = PHYSICAL_CG}
        },

        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},      //CLK_AUD_INTERFACE0_SEL // 0 : F_FXO_CK     13MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},                       // 1 : Reserved
                {CLK_APLL1, CLK_DIV_NONE, MUX_ALLOW},                        // 2 : APLL1        22.579MHz
                {CLK_APLL2, CLK_DIV_NONE, MUX_ALLOW},                        // 3 : APLL2        24.576MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PHYSICAL_NORM_MUX,
            .curr_sel_ptr = BYTE_REG(CKSYS_CLK_CFG_REG.CLK_AUD_INTF0_SEL),
            .cg_info = { .cg_id = HAL_CLOCK_CG_AUD_INTF0, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},      //CLK_AUD_INTERFACE1_SEL // 0 : F_FXO_CK     13MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},                       // 1 : Reserved
                {CLK_APLL1, CLK_DIV_NONE, MUX_ALLOW},                        // 2 : APLL1        22.579MHz
                {CLK_APLL2, CLK_DIV_NONE, MUX_ALLOW},                        // 3 : APLL2        24.576MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PHYSICAL_NORM_MUX,
            .curr_sel_ptr = BYTE_REG(CKSYS_CLK_CFG_REG.CLK_AUD_INTF1_SEL),
            .cg_info = { .cg_id = HAL_CLOCK_CG_AUD_INTF1, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},     // CLK_26M_SEL           // 0 : F_FXO_CK     26MHz
                {CLK_OSC1, CLK_DIV_26M, MUX_NOT_ALLOW},                     // 1 : OSC_26       26/24.9375MHz  (*)
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PHYSICAL_NORM_MUX,
            .curr_sel_ptr = BYTE_REG(CKSYS_CLK_CFG_REG.CLK_26M_SEL),
            .cg_info = { .cg_id = HAL_CLOCK_CG_PSUEDO_CLK_26M, .cg_type = PSUEDO_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},     //CLK_AUD_ENGINE_SEL 0 : // 0 : F_FXO_CK     26MHz
                {CLK_OSC1, CLK_DIV_26M, MUX_NOT_ALLOW},                     // 1 : OSC_26       26/24.9375MHz  (*)
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PHYSICAL_NORM_MUX,
            .curr_sel_ptr = BYTE_REG(CKSYS_CLK_CFG_REG.CLK_AUD_ENGINE_SEL),
            .cg_info = { .cg_id = HAL_CLOCK_CG_AUD_ENGINE_BUS, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},    //CLK_AUD_VOW_SEL     // 0 : F_FXO_CK     13MHz
                {CLK_OSC2, CLK_DIV_26M, MUX_NOT_ALLOW},                 // 1 : OSC_26       13/12.46875MHz  (*)
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PHYSICAL_NORM_MUX,
            .curr_sel_ptr = BYTE_REG(CKSYS_CLK_CFG_REG.CLK_VOW_SEL),
            .cg_info = { .cg_id = HAL_CLOCK_CG_AUD_VOW_BUS, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_OSC1, CLK_DIV_D12, MUX_NOT_ALLOW},   // CLK_OSC_26M_SEL    // 0 : OSC1_D12     26/24.9375MHz (*)
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},                      // 1 : Reserved
                {CLK_OSC2, CLK_DIV_D5, MUX_NOT_ALLOW},                      // 2 : OSC2_D5_D2   26MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PHYSICAL_NORM_MUX,
            .curr_sel_ptr = BYTE_REG(CKSYS_CLK_CFG_REG.CLK_26M_SEL),
            .cg_info = { .cg_id = HAL_CLOCK_CG_PSUEDO_CLK_OSC_26M, .cg_type = PSUEDO_CG}
        },
        /* MCLK_SEL: special case, enable psuedo CG upon 1st mux_sel call (for I2S?) */
        {
            .sels = {{CLK_APLL1, CLK_DIV_NONE, MUX_ALLOW},     //CLK_MCLK_SEL   // 0 : APLL1        45.1584MHz
                {CLK_APLL2, CLK_DIV_NONE, MUX_ALLOW},                   // 1 : APLL2        49.152MHz
                {CLK_XO, CLK_DIV_NONE, MUX_ALLOW},                      // 2 : F_FXO_CK     26MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PSUEDO_MUX,
            .curr_sel_ptr = &psuedo_mux_arr[CLK_MCLK_SEL - CLK_MCLK_SEL],
            .cg_info = { .cg_id = HAL_CLOCK_CG_PSUEDO_MCLK, .cg_type = PSUEDO_CG}
        },
        /* SPDIF doesn't have a physical mux, add this definition to enable UPLL 624MHz for SPDIF */
        {
            .sels = {{CLK_UPLL, CLK_DIV_NONE, MUX_ALLOW},          //CLK_SPDIF_SEL   0 : UPLL    624MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PSUEDO_MUX,
            .curr_sel_ptr = &psuedo_mux_arr[CLK_SPDIF_SEL - CLK_MCLK_SEL],
            .cg_info = { .cg_id = HAL_CLOCK_CG_SPDIF, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},     //CLK_PWM0_SEL      // 0 : F_FXO_CK     26MHz
                {CLK_XO, CLK_DIV_NONE, MUX_ALLOW},                      // 1 : F_FRTC_CK    0.032MHz
                {CLK_OSC1, CLK_DIV_D8, MUX_ALLOW},                      // 2 : OSC1_D8      39/37.40625MHz    (*)
                {CLK_OSC2, CLK_DIV_D3, MUX_ALLOW},                      // 3 : OSC2_D3_D2   86.67MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PSUEDO_MUX,
            .curr_sel_ptr = &psuedo_mux_arr[CLK_PWM0_SEL - CLK_MCLK_SEL],
            .cg_info = { .cg_id = HAL_CLOCK_CG_PWM0, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},     //CLK_PWM1_SEL      // 0 : F_FXO_CK     26MHz
                {CLK_XO, CLK_DIV_NONE, MUX_ALLOW},                      // 1 : F_FRTC_CK    0.032MHz
                {CLK_OSC1, CLK_DIV_D8, MUX_ALLOW},                      // 2 : OSC1_D8      39/37.40625MHz    (*)
                {CLK_OSC2, CLK_DIV_D3, MUX_ALLOW},                      // 3 : OSC2_D3_D2   86.67MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PSUEDO_MUX,
            .curr_sel_ptr = &psuedo_mux_arr[CLK_PWM1_SEL - CLK_MCLK_SEL],
            .cg_info = { .cg_id = HAL_CLOCK_CG_PWM1, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},     //CLK_PWM2_SEL      // 0 : F_FXO_CK     26MHz
                {CLK_XO, CLK_DIV_NONE, MUX_ALLOW},                      // 1 : F_FRTC_CK    0.032MHz
                {CLK_OSC1, CLK_DIV_D8, MUX_ALLOW},                      // 2 : OSC1_D8      39/37.40625MHz    (*)
                {CLK_OSC2, CLK_DIV_D3, MUX_ALLOW},                      // 3 : OSC2_D3_D2   86.67MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PSUEDO_MUX,
            .curr_sel_ptr = &psuedo_mux_arr[CLK_PWM2_SEL - CLK_MCLK_SEL],
            .cg_info = { .cg_id = HAL_CLOCK_CG_PWM2, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},     //CLK_PWM3_SEL      // 0 : F_FXO_CK     26MHz
                {CLK_XO, CLK_DIV_NONE, MUX_ALLOW},                      // 1 : F_FRTC_CK    0.032MHz
                {CLK_OSC1, CLK_DIV_D8, MUX_ALLOW},                      // 2 : OSC1_D8      39/37.40625MHz    (*)
                {CLK_OSC2, CLK_DIV_D3, MUX_ALLOW},                      // 3 : OSC2_D3_D2   86.67MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PSUEDO_MUX,
            .curr_sel_ptr = &psuedo_mux_arr[CLK_PWM3_SEL - CLK_MCLK_SEL],
            .cg_info = { .cg_id = HAL_CLOCK_CG_PWM3, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},     //CLK_PWM4_SEL      // 0 : F_FXO_CK     26MHz
                {CLK_XO, CLK_DIV_NONE, MUX_ALLOW},                      // 1 : F_FRTC_CK    0.032MHz
                {CLK_OSC1, CLK_DIV_D8, MUX_ALLOW},                      // 2 : OSC1_D8      39/37.40625MHz    (*)
                {CLK_OSC2, CLK_DIV_D3, MUX_ALLOW},                      // 3 : OSC2_D3_D2   86.67MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PSUEDO_MUX,
            .curr_sel_ptr = &psuedo_mux_arr[CLK_PWM4_SEL - CLK_MCLK_SEL],
            .cg_info = { .cg_id = HAL_CLOCK_CG_PWM4, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},     //CLK_PWM5_SEL      // 0 : F_FXO_CK     26MHz
                {CLK_XO, CLK_DIV_NONE, MUX_ALLOW},                      // 1 : F_FRTC_CK    0.032MHz
                {CLK_OSC1, CLK_DIV_D8, MUX_ALLOW},                      // 2 : OSC1_D8      39/37.40625MHz    (*)
                {CLK_OSC2, CLK_DIV_D3, MUX_ALLOW},                      // 3 : OSC2_D3_D2   86.67MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PSUEDO_MUX,
            .curr_sel_ptr = &psuedo_mux_arr[CLK_PWM5_SEL - CLK_MCLK_SEL],
            .cg_info = { .cg_id = HAL_CLOCK_CG_PWM5, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},     //CLK_PWM6_SEL      // 0 : F_FXO_CK     26MHz
                {CLK_XO, CLK_DIV_NONE, MUX_ALLOW},                      // 1 : F_FRTC_CK    0.032MHz
                {CLK_OSC1, CLK_DIV_D8, MUX_ALLOW},                      // 2 : OSC1_D8      39/37.40625MHz    (*)
                {CLK_OSC2, CLK_DIV_D3, MUX_ALLOW},                      // 3 : OSC2_D3_D2   86.67MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PSUEDO_MUX,
            .curr_sel_ptr = &psuedo_mux_arr[CLK_PWM6_SEL - CLK_MCLK_SEL],
            .cg_info = { .cg_id = HAL_CLOCK_CG_PWM6, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},     //CLK_PWM7_SEL      // 0 : F_FXO_CK     26MHz
                {CLK_XO, CLK_DIV_NONE, MUX_ALLOW},                      // 1 : F_FRTC_CK    0.032MHz
                {CLK_OSC1, CLK_DIV_D8, MUX_ALLOW},                      // 2 : OSC1_D8      39/37.40625MHz  (*)
                {CLK_OSC2, CLK_DIV_D3, MUX_ALLOW},                      // 3 : OSC2_D3_D2   86.67MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PSUEDO_MUX,
            .curr_sel_ptr = &psuedo_mux_arr[CLK_PWM7_SEL - CLK_MCLK_SEL],
            .cg_info = { .cg_id = HAL_CLOCK_CG_PWM7, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},     //CLK_PWM8_SEL      // 0 : F_FXO_CK     26MHz
                {CLK_XO, CLK_DIV_NONE, MUX_ALLOW},                      // 1 : F_FRTC_CK    0.032MHz
                {CLK_OSC1, CLK_DIV_D8, MUX_ALLOW},                      // 2 : OSC1_D8      39/37.40625MHz  (*)
                {CLK_OSC2, CLK_DIV_D3, MUX_ALLOW},                      // 3 : OSC2_D3_D2   86.67MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PSUEDO_MUX,
            .curr_sel_ptr = &psuedo_mux_arr[CLK_PWM8_SEL - CLK_MCLK_SEL],
            .cg_info = { .cg_id = HAL_CLOCK_CG_PWM8, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},     //CLK_PWM9_SEL      // 0 : F_FXO_CK     26MHz
                {CLK_XO, CLK_DIV_NONE, MUX_ALLOW},                      // 1 : F_FRTC_CK    0.032MHz
                {CLK_OSC1, CLK_DIV_D8, MUX_ALLOW},                      // 2 : OSC1_D8      39/37.40625MHz  (*)
                {CLK_OSC2, CLK_DIV_D3, MUX_ALLOW},                      // 3 : OSC2_D3_D2   86.67MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PSUEDO_MUX,
            .curr_sel_ptr = &psuedo_mux_arr[CLK_PWM9_SEL - CLK_MCLK_SEL],
            .cg_info = { .cg_id = HAL_CLOCK_CG_PWM9, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},     //CLK_PWM10_SEL     // 0 : F_FXO_CK     26MHz
                {CLK_XO, CLK_DIV_NONE, MUX_ALLOW},                      // 1 : F_FRTC_CK    0.032MHz
                {CLK_OSC1, CLK_DIV_D8, MUX_ALLOW},                      // 2 : OSC1_D8      39/37.40625MHz  (*)
                {CLK_OSC2, CLK_DIV_D3, MUX_ALLOW},                      // 3 : OSC2_D3_D2   86.67MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PSUEDO_MUX,
            .curr_sel_ptr = &psuedo_mux_arr[CLK_PWM10_SEL - CLK_MCLK_SEL],
            .cg_info = { .cg_id = HAL_CLOCK_CG_PWM10, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},     //CLK_PWM11_SEL     // 0 : F_FXO_CK     26MHz
                {CLK_XO, CLK_DIV_NONE, MUX_ALLOW},                      // 1 : F_FRTC_CK    0.032MHz
                {CLK_OSC1, CLK_DIV_D8, MUX_ALLOW},                      // 2 : OSC1_D8      39/37.40625MHz  (*)
                {CLK_OSC2, CLK_DIV_D3, MUX_ALLOW},                      // 3 : OSC2_D3_D2   86.67MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PSUEDO_MUX,
            .curr_sel_ptr = &psuedo_mux_arr[CLK_PWM11_SEL - CLK_MCLK_SEL],
            .cg_info = { .cg_id = HAL_CLOCK_CG_PWM11, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},     //CLK_PWM12_SEL     // 0 : F_FXO_CK     26MHz
                {CLK_XO, CLK_DIV_NONE, MUX_ALLOW},                      // 1 : F_FRTC_CK    0.032MHz
                {CLK_OSC1, CLK_DIV_D8, MUX_ALLOW},                      // 2 : OSC1_D8      39/37.40625MHz  (*)
                {CLK_OSC2, CLK_DIV_D3, MUX_ALLOW},                      // 3 : OSC2_D3_D2   86.67MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PSUEDO_MUX,
            .curr_sel_ptr = &psuedo_mux_arr[CLK_PWM12_SEL - CLK_MCLK_SEL],
            .cg_info = { .cg_id = HAL_CLOCK_CG_PWM12, .cg_type = PHYSICAL_CG}
        },
        {
            .sels = {{CLK_XO, CLK_DIV_NONE, MUX_ALLOW},     //CLK_PWM13_SEL     // 0 : F_FXO_CK     26MHz
                {CLK_XO, CLK_DIV_NONE, MUX_ALLOW},                      // 1 : F_FRTC_CK    0.032MHz
                {CLK_OSC1, CLK_DIV_D8, MUX_ALLOW},                      // 2 : OSC1_D8      39/37.40625MHz  (*)
                {CLK_OSC2, CLK_DIV_D3, MUX_ALLOW},                      // 3 : OSC2_D3_D2   86.67MHz
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW},
                {CLK_XO, CLK_DIV_NONE, MUX_NOT_ALLOW}
            },
            .mux_type = PSUEDO_MUX,
            .curr_sel_ptr = &psuedo_mux_arr[CLK_PWM13_SEL - CLK_MCLK_SEL],
            .cg_info = { .cg_id = HAL_CLOCK_CG_PWM13, .cg_type = PHYSICAL_CG}
        },
    }
};
#endif /* HAL_CLOCK_MODULE_ENABLED */


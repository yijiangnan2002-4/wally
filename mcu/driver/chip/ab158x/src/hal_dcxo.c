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

#include <stdio.h>
#include <stdlib.h>
#include "hal_feature_config.h"
#include "hal.h"

#if defined(__EXT_BOOTLOADER__)
#include "bl_common.h"
#define clk_dbg_log(dbg_msg, param_cnt, ...)
#define clk_dbg_print(dbg_msg, param_cnt, ...) bl_print(LOG_DEBUG, dbg_msg, ##__VA_ARGS__)
#elif defined(__EXT_DA__)
/* need to add da env corresponding debug log function call */
#define clk_dbg_log(dbg_msg, param_cnt, ...)
#define clk_dbg_print(dbg_msg, param_cnt, ...)
#else
#include "syslog.h"
/* dbg_log, added in critical section, only enable when we need to debug stuff */
#define clk_dbg_log(dbg_msg, param_cnt, ...)
#define clk_dbg_print(dbg_msg, param_cnt, ...)     log_hal_msgid_info(dbg_msg, param_cnt, ##__VA_ARGS__)
#endif

#ifdef MTK_NVDM_ENABLE
#include "nvdm.h"
#include "nvkey_id_list.h"
#include "nvkey.h"
#include "hal_dcxo_nvkey_struct.h"
#endif
#ifdef HAL_SLEEP_MANAGER_ENABLED
#include "hal_sleep_manager.h"
#include "hal_sleep_manager_internal.h"
#endif
#ifdef HAL_DCXO_MODULE_ENABLED
#include "hal_clock_internal.h"
#include "hal_nvic_internal.h"

#define __EOSC_32K__
unsigned int dcxo_capid = 0;

#if !defined(__EXT_BOOTLOADER__) && !defined(__EXT_DA__)
#include "dcxo_capid.h"  /* function only defined in mainbin */
#endif
#define CAPID_MASK    0x1FF    /*  Dcxo capid range : 0 ~ 511. */

#ifdef HAL_SLEEP_MANAGER_ENABLED
static uint8_t backup_EN_26M_FPM, backup_BT_26M_EN;
#endif

#define APPLY_FPM_CAPID_ISEL_TO_LPM

static uint8_t dcxo_fpm_lock_src;

/* Auto switch to LPM after Unlock FPM and no one Lock (Lock Source = 0) */
#define SWITCH2LPM_AFTER_UNLOCK_FPM_AND_NONE_LOCK


void set_capid(void)
{
    /* Note: CAPID settings can only take affect during DCXO FPM */
    if (dcxo_capid) {
        DCXO_PCON_REG.DCXO_CAPID_EFUSE = dcxo_capid;
        DCXO_PCON_REG.DCXO_CAPID_EFUSE_SEL = 0x1;
        clk_dbg_log("[DCXO] Load FPM CAPID done, CAPID RG = 0x%x\r\n", 1, dcxo_capid);
    }
}

uint32_t get_capid(void)
{
    return dcxo_capid;
}

#if 0
/**
 * internal DCXO pwr mode switch (FPM/LPM)
 * some configs require FPM/LPM switch for settings to take affect
 * this function is for this specific purpose (dcxo driver internal use only)
 */
static void _internal_dcxo_lp_mode(dcxo_mode_t mode)
{
    DCXO_PCON_REG.EN_26M_FPM = mode;
    DCXO_PCON_REG.BT_26M_EN = mode;
}
#endif

/**
 * DCXO pwr mode switch (FPM/LPM)
 * (for AIR_BTA_IC_PREMIUM_G3, maintaining dcxo as FPM is feasible)
 * -> when user calls this function, it always sets DCXO to FPM
 */
ATTR_TEXT_IN_TCM void dcxo_lp_mode(dcxo_mode_t mode)
{
#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE) || defined(AIR_HEARTHROUGH_VIVID_PT_ENABLE)  /* allow users to dynamically switch between normal/low power mode */
    if((mode == DCXO_LP_MODE) && dcxo_fpm_lock_src) {
        mode = DCXO_NORMAL_MODE;
        clk_dbg_log("DCXO LPM not allow to set, FPM Lock Src %x\r\n", 1, dcxo_fpm_lock_src);
    }

  #ifdef APPLY_FPM_CAPID_ISEL_TO_LPM
    if(mode == DCXO_LP_MODE) {
        DCXO_CFG->DCXO_CTUNE_LPM_b.DCXO_CDAC_LPM = DCXO_PCON_REG.DCXO_CAPID_EFUSE;
        DCXO_CFG->DCXO_CORE_ISEL_2_b.DCXO_CORE_ISEL_LPM = DCXO_CFG->DCXO_CORE_ISEL_1_b.DCXO_CORE_ISEL_FPM_G2;
    }
  #endif

    DCXO_PCON_REG.EN_26M_FPM = mode;
    DCXO_PCON_REG.BT_26M_EN = mode;
#else
    /* fixed to FPM for PREMIUM_G3
     * change parameter to "mode" for dynamic mode switching
     */
    DCXO_PCON_REG.EN_26M_FPM = DCXO_NORMAL_MODE;
    DCXO_PCON_REG.BT_26M_EN = DCXO_NORMAL_MODE;
#endif
}

ATTR_TEXT_IN_SYSRAM void dcxo_fpm_lock_ctrl(dcxo_fpm_lock_src_t lock_src, dcxo_fpm_lock_op_t lock_op)
{
    if(lock_src >= DCXO_FPM_LOCK_MAX) {
        clk_dbg_print("DCXO FPM Lock Ctrl %d, %d (Invalid Src)\r\n", 2, lock_src, lock_op);
        return;
    }

#if !defined(__EXT_BOOTLOADER__) && !defined(__EXT_DA__)
    uint32_t irq_mask = 0;
    hal_nvic_save_and_set_interrupt_mask(&irq_mask);
    /* ================ Critical Section Start ======================== */
#endif

    if(lock_op == DCXO_FPM_LOCK) {
        dcxo_fpm_lock_src |= (1 << lock_src);
    }
    else { /* DCXO_FPM_UNLOCK */
        dcxo_fpm_lock_src &= ~(1 << lock_src);
    }

#if !defined(__EXT_BOOTLOADER__) && !defined(__EXT_DA__)
    /* ================ Critical Section End ======================== */
    hal_nvic_restore_interrupt_mask(irq_mask);
#endif

    if(dcxo_fpm_lock_src) {
        if((DCXO_PCON_REG.EN_26M_FPM==0) && (DCXO_PCON_REG.BT_26M_EN==0)) {
            dcxo_lp_mode(DCXO_NORMAL_MODE);
        }
    }
#ifdef SWITCH2LPM_AFTER_UNLOCK_FPM_AND_NONE_LOCK
    else {
        dcxo_lp_mode(DCXO_LP_MODE);
    }
#endif
    clk_dbg_log("DCXO FPM Lock Ctrl %d, %d, CurSrc %x, Mode %d\r\n", 4, lock_src, lock_op, dcxo_fpm_lock_src, dcxo_current_mode());
}

uint8_t dcxo_fpm_lock_get_src(void)
{
    return dcxo_fpm_lock_src;
}

/* dcxo_32k_ctrl: API for enabling/disabling DCXO 32K */
void dcxo_32k_ctrl(uint8_t dcxo32k_en)
{
     DCXO_PCON_REG.DCXO32K_EN = dcxo32k_en;
}

/* API that returns current dcxo mode */
dcxo_mode_t dcxo_current_mode(void)
{
    if (DCXO_PCON_REG.EN_26M_FPM == 0 && DCXO_PCON_REG.BT_26M_EN == 0) {
        return DCXO_LP_MODE;
    } else {
        return DCXO_NORMAL_MODE;
    }
}

/* API that checks if VBG_CAL calibration value exists, and apply it to VBC */
void set_vref_v2i_sel(void)
{
    uint8_t efuse_vbg_val = EFUSE->M_ANA_CFG_FT_BTRF1_b.VBG_CAL;
    if (efuse_vbg_val != 0) {
        /* value exists, apply setting to register (truncate efuse value to 6 bits) */
        DCXO_CFG->BGCORE_CTRL0_b.VREF_V2I_SEL = (efuse_vbg_val & 0x3F);
        clk_dbg_log("[DCXO] VBG_CAL value applied to VREF_V2I_SEL = 0x%x\r\n", 1, DCXO_CFG->BGCORE_CTRL0_b.VREF_V2I_SEL);
    } else {
        clk_dbg_print("[DCXO] Error, VBG_CAL value does not exist!\r\n", 0);
    }
}

/* API that checks if VBG_CAL calibration value exists, and apply it accordingly */
void set_vref_dcxo_sel(void)
{
    /* Apply VBG calibration results (if calibration results exist) */
    uint8_t efuse_vbg_val = EFUSE->M_ANA_CFG_FT_BTRF1_b.VBG_CAL;
    if (efuse_vbg_val != 0) {
        /* value exists, apply setting to register (truncate efuse value to 6 bits)
         * Need to switch from LPM -> FPM once for settings to apply
         */
        DCXO_CFG->BGCORE_CTRL1_b.VREF_DCXO_SEL = (efuse_vbg_val & 0x3F);
        dcxo_lp_mode(DCXO_LP_MODE);
        hal_gpt_delay_us(200);
        dcxo_lp_mode(DCXO_NORMAL_MODE);
        hal_gpt_delay_us(300);

        clk_dbg_log("[DCXO] VBG_CAL value applied to VREF_DCXO_SEL = 0x%x\r\n", 1, DCXO_CFG->BGCORE_CTRL1_b.VREF_DCXO_SEL);
    } else {
        clk_dbg_print("[DCXO] Error, VBG_CAL value does not exist!\r\n", 0);
    }
}

#if 0
/* function for debugging DCXO macro rg settings */
static void dcxo_macro_rg_dbg_dump(void)
{
    clk_dbg_log("RG dump 12:0x%x\r\n", 1, *(volatile uint32_t *)(DCXO_CFG_BASE + 12));
    clk_dbg_log("RG dump 40:0x%x\r\n", 1, *(volatile uint32_t *)(DCXO_CFG_BASE + 40));
    clk_dbg_log("RG dump 44:0x%x\r\n", 1, *(volatile uint32_t *)(DCXO_CFG_BASE + 44));
    clk_dbg_log("RG dump 48:0x%x\r\n", 1, *(volatile uint32_t *)(DCXO_CFG_BASE + 48));
    clk_dbg_log("RG dump 52:0x%x\r\n", 1, *(volatile uint32_t *)(DCXO_CFG_BASE + 52));
    clk_dbg_log("RG dump 64:0x%x\r\n", 1, *(volatile uint32_t *)(DCXO_CFG_BASE + 64));
    clk_dbg_log("RG dump 68:0x%x\r\n", 1, *(volatile uint32_t *)(DCXO_CFG_BASE + 68));
    clk_dbg_log("RG dump 72:0x%x\r\n", 1, *(volatile uint32_t *)(DCXO_CFG_BASE + 72));
    clk_dbg_log("RG dump 76:0x%x\r\n", 1, *(volatile uint32_t *)(DCXO_CFG_BASE + 76));
    clk_dbg_log("RG dump 84:0x%x\r\n", 1, *(volatile uint32_t *)(DCXO_CFG_BASE + 84));
    clk_dbg_log("RG dump 92:0x%x\r\n", 1, *(volatile uint32_t *)(DCXO_CFG_BASE + 92));
    clk_dbg_log("RG dump 156:0x%x\r\n", 1, *(volatile uint32_t *)(DCXO_CFG_BASE + 156));
}
#endif

/* DCXO MACRO RG settings update
 * (some may be overridden in later init settings)
 */
static void set_dcxo_macro_rg(void)
{
    /* DCXO_MACRO RG settings */
    if (((*(volatile uint32_t *)0x420C0208 >> 5) & 0x7) == 0x2) {
        /* E1/E2 settings */
        DCXO_CFG->DCXO_CTUNE_LPM = 0x0040;
        DCXO_CFG->DCXO_CORE_ISEL_1 = 0xFFFF;
        DCXO_CFG->DCXO_CORE_ISEL_2 = 0xFFA0;
        DCXO_CFG->DCXO_FPM_LDO2 = 0x7776;
        DCXO_CFG->DCXO_FPM_LDO3 = 0x7675;
        DCXO_CFG->DCXO_FPM_LDO4 = 0x7576;
        DCXO_CFG->BBPLL_XOPS = 0x0087;
        DCXO_CFG->DCXO_LPM_LDO = 0x0005;
        DCXO_CFG->DCXO_32KDIV_LPM = 0x1E83;
        DCXO_CFG->DCXO_MAN2 = 0x75F1;
    } else {
        /* E3 settings */
        DCXO_CFG->DCXO_CTUNE_LPM = 0x0040;
        DCXO_CFG->DCXO_CORE_ISEL_1 = 0xFFFF;
        DCXO_CFG->DCXO_CORE_ISEL_2 = 0xFFA0;
        DCXO_CFG->BGCORE_CTRL0_b.BG_FC_OFF = 0x1; /* bit default value was changed to 0 */
        DCXO_CFG->DCXO_FPM_LDO2 = 0x7676;
        DCXO_CFG->DCXO_FPM_LDO3 = 0x7675;
        DCXO_CFG->DCXO_FPM_LDO4 = 0x7575;
        DCXO_CFG->DCXO_LPM_LDO = 0x0005;
        DCXO_CFG->DCXO_32KDIV_LPM = 0x1E83;
        DCXO_CFG->DCXO_MAN2 = 0x75F1;
    }
    clk_dbg_log("[DCXO] LPM CAPID = 0x%x\r\n", 1, DCXO_CFG->DCXO_CTUNE_LPM);
    //dcxo_macro_rg_dbg_dump();
}

static void xops_disable(void)
{
    DCXO_CFG->DCXO_XOPS_b.RG_DCXO_XOPS_START = 0;
    DCXO_CFG->BBPLL_XOPS_b.BBPLL_XOPS_EN_PO_MAN = 0;
}

#ifdef HAL_SLEEP_MANAGER_ENABLED
void dcxo_resume_callback(void)
{
#if 0
    /* Enable clock to BTRF during resume */
    DCXO_PCON_REG.BT_26M_EN = 1;
#else
    DCXO_PCON_REG.EN_26M_FPM = backup_EN_26M_FPM;
    DCXO_PCON_REG.BT_26M_EN = backup_BT_26M_EN;
#endif
}

void dcxo_suspend_callback(void)
{
    backup_EN_26M_FPM = DCXO_PCON_REG.EN_26M_FPM;
    backup_BT_26M_EN = DCXO_PCON_REG.BT_26M_EN;

    /* Disable clock to BTRF before sleep */
    DCXO_PCON_REG.BT_26M_EN = 0;
}
#endif

/**
 * dcxo_pwr_ctrl setting
 *
 */
void hal_dcxo_init(void)
{
#if !defined(__EXT_BOOTLOADER__) && !defined(__EXT_DA__)
    dcxo_load_capid(); /* function only defined in mainbin */
#endif

    DCXO_PCON_REG.GSM_DCXO_CTL_EN = 0x1;   /* Enable baseband control */
    DCXO_PCON_REG.EXT_DCXO_CTL_EN = 0x1;   /* Enable external control */

    DCXO_PCON_REG.DCXO_CK_RDY_COMP_VREF_SEL = 0x1;   /* For DCXO DIG rg reset */
    DCXO_PCON_REG.DCXO_26M_RDY_EN = 0x0; /* Skip DCXO wakeup by 26M_DBB_CK_RDY */
    set_capid();
    set_dcxo_macro_rg();

    xops_disable();
    set_vref_v2i_sel();
    set_vref_dcxo_sel();

#ifdef __EOSC_32K__
    /* Does not use DCXO32K wakeup timing config*/
    /* Note: since wakeup timing of DCXO is a bit longer than when DCXO32K is enabled, the longer timing
     *       is also suitable for when DCXO32K is dynamically enabled/disabled
     *       (DCXO32K is currently dynamically enabled/disabled in RTC driver by setting DCXO32K_EN directly)
     */
    DCXO_PCON_REG.DCXO_PWR_EN_TD = 0x1;
    DCXO_PCON_REG.DCXO_EN_TD = 0x1;
    DCXO_PCON_REG.DCXO_BUF_EN_TD = 0x51;
#else //32kless
    /* Use DCXO32K */
    DCXO_PCON_REG.DCXO32K_EN = 0x1;
    DCXO_PCON_REG.DCXO_PWR_EN_TD = 0x1;
    DCXO_PCON_REG.DCXO_EN_TD = 0x1;
    DCXO_PCON_REG.DCXO_BUF_EN_TD = 0x14;
#endif
    DCXO_PCON_REG.DCXO_ISO_EN_TD = 0x6;
    DCXO_PCON_REG.DCXO_SLEEP_TD = 0xA;

    /* Since dcxo_init() is currently the last clock-related init
     * -> call system freq debug log
     */
    hal_clock_get_freq_meter(FQMTR_SYS_DBG, 1000);

#ifdef HAL_SLEEP_MANAGER_ENABLED
    sleep_management_register_all_secure_suspend_callback(SLEEP_BACKUP_RESTORE_DCXO_CFG, (sleep_management_suspend_callback_t)dcxo_suspend_callback, NULL);
    sleep_management_register_all_secure_resume_callback(SLEEP_BACKUP_RESTORE_DCXO_CFG, (sleep_management_resume_callback_t)dcxo_resume_callback, NULL);
#endif
}

/* common API to set FPM capid RG */
void set_capid_rg(uint32_t capid_val)
{
    DCXO_PCON_REG.DCXO_CAPID_EFUSE = capid_val;
    DCXO_PCON_REG.DCXO_CAPID_EFUSE_SEL = 0x1;
    clk_dbg_log("[DCXO] Set FPM CAPID done, CAPID RG = 0x%x\r\n", 1, DCXO_PCON_REG.DCXO_CAPID_EFUSE);
}

/* common API to get FPM capid RG (returns capid value) */
uint32_t get_capid_rg(void)
{
    return DCXO_PCON_REG.DCXO_CAPID_EFUSE;
}

/* common API to set capid value stored in NVDM (returns nvkey write api returned status) */
int set_capid_nvdm(uint32_t capid_val)
{
    int result = 0;

    capid_val = capid_val & CAPID_MASK;
#ifdef MTK_NVDM_ENABLE
    uint8_t nvkey[4] = {0};
    uint32_t size = sizeof(xo_info_t);

    if (((nvkey_status_t)nvkey_read_data(NVID_CAL_XO_26M_CRTSTAL_TRIM, (uint8_t *)nvkey, &size)) == NVKEY_STATUS_OK) {
        (((xo_info_t *)nvkey) -> cap_value) = capid_val;
        result = nvkey_write_data(NVID_CAL_XO_26M_CRTSTAL_TRIM, (uint8_t *)nvkey, size);
        clk_dbg_log("CAPID item in NVDM(nvkey) is set(0x%x).\r\n", 1, (((xo_info_t *)nvkey) -> cap_value));
    } else {
        clk_dbg_print("CAPID item in  NVDM(nvkey) is empty, use default value.\r\n", 0);
    }
#endif
    return result;
}

/* common API to get capid value stored in NVDM (returns retrieved capid value) */
uint32_t get_capid_nvdm(void)
{
    uint32_t dcxo_capid = 0;

#ifdef MTK_NVDM_ENABLE
    uint32_t size = 0;
    uint8_t nvkey[4] = {0};
    size = sizeof(xo_info_t);
    if (nvkey_read_data(NVID_CAL_XO_26M_CRTSTAL_TRIM, nvkey, &size) == NVKEY_STATUS_OK) {
        dcxo_capid = (((xo_info_t *)nvkey) -> cap_value) & CAPID_MASK;
        clk_dbg_log("CAPID item in NVDM(nvkey) is available(0x%x).\r\n", 1, dcxo_capid);
    } else {
        clk_dbg_print("CAPID item in  NVDM(nvkey) is empty, use default value.\r\n", 0);
    }
#endif

    return dcxo_capid;
}

ATTR_TEXT_IN_TCM void set_dcxo_mode_4testonly(dcxo_mode_t mode)
{
    if((mode == DCXO_LP_MODE) && dcxo_fpm_lock_src) {
        mode = DCXO_NORMAL_MODE;
        clk_dbg_log("DCXO LPM not allow to set, FPM Lock Src %x\r\n", 1, dcxo_fpm_lock_src);
    }

#ifdef APPLY_FPM_CAPID_ISEL_TO_LPM
    if(mode == DCXO_LP_MODE) {
        DCXO_CFG->DCXO_CTUNE_LPM_b.DCXO_CDAC_LPM = DCXO_PCON_REG.DCXO_CAPID_EFUSE;
        DCXO_CFG->DCXO_CORE_ISEL_2_b.DCXO_CORE_ISEL_LPM = DCXO_CFG->DCXO_CORE_ISEL_1_b.DCXO_CORE_ISEL_FPM_G2;
    }
#endif

    DCXO_PCON_REG.EN_26M_FPM = mode;
    DCXO_PCON_REG.BT_26M_EN = mode;
}

/* ============================ Called by AT Command - Clock ============================ */
#include <string.h>
#include "hal_gpt.h"

#define UNUSED(x)  ((void)(x))

/* "Toggle Mode Periodically" Time Interval: 2ms ~ 10s */
#define DCXO_TOGGLE_MODE_TIME_INV_MS_MIN  2
#define DCXO_TOGGLE_MODE_TIME_INV_MS_MAX  10000

#define DCXO_GPT_PORT                     HAL_GPT_7
#define DCXO_RPT_TIMER_2TARGET_CAPID_US   30

static dcxo_mode_t dcxo_cur_mode = DCXO_NORMAL_MODE;
static uint32_t dcxo_target_capid;

static void dcxo_toggle_mode(void *user_data)
{
    UNUSED(user_data);
    dcxo_cur_mode ^= 1;
    set_dcxo_mode_4testonly(dcxo_cur_mode);
    clk_dbg_log("dcxo_cur_mode %d\r\n", 1, dcxo_current_mode());
}

static void dcxo_fpm_step2target_capid(void *user_data)
{
    UNUSED(user_data);
    if(DCXO_PCON_REG.DCXO_CAPID_EFUSE < dcxo_target_capid) {
        DCXO_PCON_REG.DCXO_CAPID_EFUSE_SEL = 0x1;
        DCXO_PCON_REG.DCXO_CAPID_EFUSE ++;
    } else if(DCXO_PCON_REG.DCXO_CAPID_EFUSE > dcxo_target_capid) {
        DCXO_PCON_REG.DCXO_CAPID_EFUSE_SEL = 0x1;
        DCXO_PCON_REG.DCXO_CAPID_EFUSE --;
    } else {
        hal_gpt_stop_timer(DCXO_GPT_PORT);
        hal_gpt_deinit(DCXO_GPT_PORT);
        clk_dbg_log("dcxo_fpm_step2target_capid %d done\r\n", 1, (uint16_t)dcxo_target_capid);
    }
}

/*
Test:
AT+BTCMIT=BT_STANDBY
AT+CLOCK=4,mode,0
AT+CLOCK=4,mode,1
AT+CLOCK=4,toggle,2
AT+CLOCK=4,toggle,10000
AT+CLOCK=4,step,0
AT+CLOCK=4,step,511
*/
void hal_dcxo_at_cmd(char *param_0, char *param_1, char *rsp_buf)
{
    if(param_0 && param_1) {
        if(!strcmp(param_0, "mode")) {  /* Set DCXO Mode */
            dcxo_mode_t mode = (dcxo_mode_t)(param_1[0]=='0'? DCXO_LP_MODE:DCXO_NORMAL_MODE);
            set_dcxo_mode_4testonly(mode);
            sprintf(rsp_buf, "dcxo mode: %s\r\n", (dcxo_current_mode()==DCXO_NORMAL_MODE)? "Normal":"LP");
        } else if(!strcmp(param_0, "toggle")) {  /* Toggle DCXO Mode Periodically */
            uint32_t intervalMs = strtol(param_1, NULL, 10);
            if((intervalMs >= DCXO_TOGGLE_MODE_TIME_INV_MS_MIN) && (intervalMs <= DCXO_TOGGLE_MODE_TIME_INV_MS_MAX) && (hal_gpt_init(DCXO_GPT_PORT)==HAL_GPT_STATUS_OK)) {
                hal_gpt_register_callback(DCXO_GPT_PORT, dcxo_toggle_mode, NULL);
                hal_gpt_start_timer_ms(DCXO_GPT_PORT, intervalMs, HAL_GPT_TIMER_TYPE_REPEAT);
                sprintf(rsp_buf, "dcxo toggle per %dms\r\n", (uint16_t)intervalMs);
            } else {
                hal_gpt_stop_timer(DCXO_GPT_PORT);
                hal_gpt_deinit(DCXO_GPT_PORT);
                strcpy(rsp_buf, "dcxo toggle/step stop\r\n");
            }
        } else if(!strcmp(param_0, "step")) {  /* Step to Target Capid (0~511) */
            if(hal_gpt_init(DCXO_GPT_PORT)==HAL_GPT_STATUS_OK) {
                dcxo_target_capid = strtol(param_1, NULL, 10);
                if(dcxo_target_capid > 511) {
                    dcxo_target_capid = 511;
                }
                set_dcxo_mode_4testonly(DCXO_NORMAL_MODE);
                hal_gpt_register_callback(DCXO_GPT_PORT, dcxo_fpm_step2target_capid, NULL);
                hal_gpt_start_timer_us(DCXO_GPT_PORT, DCXO_RPT_TIMER_2TARGET_CAPID_US, HAL_GPT_TIMER_TYPE_REPEAT);
                sprintf(rsp_buf, "dcxo start step to target capid %d\r\n", (uint16_t)dcxo_target_capid);
            } else {
                hal_gpt_stop_timer(DCXO_GPT_PORT);
                hal_gpt_deinit(DCXO_GPT_PORT);
                strcpy(rsp_buf, "dcxo toggle/step stop\r\n");
            }
        } else if(!strcmp(param_0, "fpmlock")) {  /* FPM Lock Control (1:Lock, 2:Get Lock Source, Others:Unlock) */
            uint8_t op = strtol(param_1, NULL, 10);
            if(op == 1) {
                dcxo_fpm_lock_ctrl(DCXO_FPM_LOCK_ATC, DCXO_FPM_LOCK);
            }
            else if(op != 2) {
                dcxo_fpm_lock_ctrl(DCXO_FPM_LOCK_ATC, DCXO_FPM_UNLOCK);
            }
            sprintf(rsp_buf, "LockSrc %x, dcxo mode: %s\r\n", dcxo_fpm_lock_src, (dcxo_current_mode()==DCXO_NORMAL_MODE)? "Normal":"LP");
        }
    }
}

#endif /*HAL_DCXO_MODULE_ENABLED*/


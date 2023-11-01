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
#ifdef HAL_PMU_MODULE_ENABLED
#include "hal_nvic_internal.h"
#include "memory_attribute.h"
#include "hal_pmu_ddie.h"
#include "hal_pmu_ddie_platform.h"
uint8_t pmu_charger_type = NON_STD_CHARGER;                                                                           /*DATA : restore charger type*/
uint8_t dpmu_init_flag = 0;                                              /* FLAG : check init setting is done */
/*=========[CAPTOUCH]=========*/
/**********************
*MUX : CAP_CON
*[4]:pmu_iso_en
*[3]:lsh_en_cpt
*[2]:ana_pwr_en_cpt
*[1]:cap2core_iso_en
*[0]:cap_rstb
***********************/
#ifdef AIR_BTA_IC_PREMIUM_G3_TYPE_D
#if !defined(__EXT_BOOTLOADER__)
static const char *power_on_reason[5] = {
    "Reserve",
    "Chip_en high",
    "Power on from rtc mode",
    "Over tempature",
    "WDT"
};
static const char *power_off_reason[13] = {
    "CHIP_EN low in normal mode",
    "VCORE PG in normal mode",
    "VSRAM PG in normal mode",
    "VRF PG in normal mode",
    "VDD18_UVLO PG in normal mode",
    "rg_exception_off PG in normal mode",
    "Over tempature in normal mode",
    "Cold WDT in normal mode",
    "go_rtc in normal mode",
    "ADIE reset",
    "CHIP_EN L in RTC OFF mode",
    "CHIP_EN L in OT OFF mode",
    "CHIP_EN L in WDT OFF mode",
};
#endif
#endif
void pmu_cap_touch_config(pmu_strup_mux_t strup_mux, int cap_rstb, int cap2core_iso_en, int ana_pwr_en_cpt, int lsh_en_cpt, int cap_in_en)
{
    uint16_t data;
    data = cap_rstb | cap2core_iso_en << 1 | ana_pwr_en_cpt << 2 | lsh_en_cpt << 3 | cap_in_en << 4;
    pmu_strup_latch_register(data, strup_mux);
}

uint8_t pmu_enable_captouch_power_ddie(pmu_power_operate_t oper)
{
    uint32_t ste = 0;
    if (oper) {
        pmu_cap_touch_config(PMU_STRUP_CAPTOUCH, PMU_OFF, PMU_ON, PMU_ON, PMU_OFF, PMU_OFF);  /*ana_pwr_en_cpt = 1*/
        hal_gpt_delay_us(5);
        pmu_cap_touch_config(PMU_STRUP_CAPTOUCH, PMU_OFF, PMU_ON, PMU_ON, PMU_ON, PMU_OFF);   /*lsh_en_cpt = 1*/
        hal_gpt_delay_us(5);
        pmu_cap_touch_config(PMU_STRUP_CAPTOUCH, PMU_OFF, PMU_OFF, PMU_ON, PMU_ON, PMU_OFF);    /*cap2core_iso_en = 0, cap_in_en=0*/
        hal_gpt_delay_us(5);
        pmu_cap_touch_config(PMU_STRUP_CAPTOUCH, PMU_ON, PMU_OFF, PMU_ON, PMU_ON, PMU_OFF);    /*cap_rstb = 1*/
        hal_gpt_delay_us(5);
        pmu_cap_touch_config(PMU_STRUP_CAPTOUCH, PMU_ON, PMU_OFF, PMU_ON, PMU_ON, PMU_ON);    /*cap_in_en = 1*/
        hal_gpt_delay_us(5);
    } else {
        pmu_cap_touch_config(PMU_STRUP_CAPTOUCH, PMU_ON, PMU_OFF, PMU_ON, PMU_ON, PMU_OFF);  /*cap_in_en = 1*/
        hal_gpt_delay_us(5);
        pmu_cap_touch_config(PMU_STRUP_CAPTOUCH, PMU_OFF, PMU_OFF, PMU_ON, PMU_ON, PMU_OFF);   /*cap_rstb = 1*/
        hal_gpt_delay_us(5);
        pmu_cap_touch_config(PMU_STRUP_CAPTOUCH, PMU_OFF, PMU_ON, PMU_ON, PMU_ON, PMU_OFF);    /*cap2core_iso_en = 1, pmu_iso_en=1*/
        hal_gpt_delay_us(5);
        pmu_cap_touch_config(PMU_STRUP_CAPTOUCH, PMU_OFF, PMU_ON, PMU_ON, PMU_OFF, PMU_OFF);    /*lsh_en_cpt = 1*/
        hal_gpt_delay_us(5);
        pmu_cap_touch_config(PMU_STRUP_CAPTOUCH, PMU_OFF, PMU_ON, PMU_OFF, PMU_OFF, PMU_OFF);    /*pwr_en_cpt = 1*/
        hal_gpt_delay_us(5);
    }
    return ste;
}

/*=========[USB : VBUS]=========*/

/* * * * * * * VBUS¡@VOSEL
 * 0x0:1.0825V ; 0x2:1.1075V ; 0x4:1.1325V ; 0x8:1.1825V  ; 0xA:1.2075V ;
 * 0xB:1.22V   ; 0xD:1.245V  ; 0xF:1.27V   ; 0x10:1.2825V ; 0x12:1.3075 ;
 * */
void pmu_select_vbus_vosel(uint32_t val)
{
    pmu_set_register_ddie(PMU_DIG_USB1, PMU_VUSB_VOSEL_MASK, PMU_VUSB_VOSEL_SHIFT, val);
}

uint8_t pmu_select_vbus_vosel_ddie(void)
{
    return pmu_get_register_ddie(PMU_DIG_USB0, PMU_AD_VUSB_PG_STATUS_MASK, PMU_AD_VUSB_PG_STATUS_SHIFT);
}

uint8_t pmu_enable_usb_power_ddie(pmu_power_operate_t oper)
{
    if (oper) {
        pmu_set_register_ddie(PMU_DIG_USB2, PMU_VUSB_EN_MASK, PMU_VUSB_EN_SHIFT, 0x1);
        hal_gpt_delay_us(300);
        pmu_set_register_ddie(PMU_DIG_USB2, PMU_USB18_SW_EN_MASK, PMU_USB18_SW_EN_SHIFT, 0x1);
        hal_gpt_delay_us(100);
        pmu_set_register_ddie(PMU_DIG_USB2, PMU_USB_RSTB_MASK, PMU_USB_RSTB_SHIFT, 0x1);
        hal_gpt_delay_us(100);
    } else {
        pmu_set_register_ddie(PMU_DIG_USB2, PMU_USB_RSTB_MASK, PMU_USB_RSTB_SHIFT, 0x0);
        hal_gpt_delay_us(100);
        pmu_set_register_ddie(PMU_DIG_USB2, PMU_USB18_SW_EN_MASK, PMU_USB18_SW_EN_SHIFT, 0x0);
        hal_gpt_delay_us(100);
        pmu_set_register_ddie(PMU_DIG_USB2, PMU_VUSB_EN_MASK, PMU_VUSB_EN_SHIFT, 0x0);
        hal_gpt_delay_us(300);
    }
    return pmu_get_register_ddie(PMU_DIG_USB0, PMU_AD_VUSB_PG_STATUS_MASK, PMU_AD_VUSB_PG_STATUS_SHIFT);
}

/*=========[Power]=========*/

void pmu_select_vsram_vosel_ddie(pmu_power_stage_t mode, pmu_vsram_voltage_t val)
{
    switch (mode) {
        case PMU_SLEEP:
            pmu_set_register_ddie(PMU_DIG_SLP0, PMU_VSRAM_VOSEL_SLEEP_MASK, PMU_VSRAM_VOSEL_SLEEP_SHIFT, val);
            log_hal_msgid_info("[DPMU]VSRAM Sleep Voltage :%x", 1, pmu_get_register_ddie(PMU_DIG_SLP0, PMU_VSRAM_VOSEL_SLEEP_MASK, PMU_VSRAM_VOSEL_SLEEP_SHIFT));
            break;
        case PMU_NORMAL:
            pmu_set_register_ddie(PMU_DIG_VSRAM4, PMU_VSRAM_VOSEL_NORMAL_MASK, PMU_VSRAM_VOSEL_NORMAL_SHIFT, val);
            log_hal_msgid_info("[DPMU]VSRAM Normal Voltage :%x", 1, pmu_get_register_ddie(PMU_DIG_VSRAM4, PMU_VSRAM_VOSEL_NORMAL_MASK, PMU_VSRAM_VOSEL_NORMAL_SHIFT));
            break;
    }
}

uint32_t pmu_get_vsram_vosel_ddie(pmu_power_stage_t mode)
{
    uint32_t temp_value = 0;
    switch (mode) {
        case PMU_SLEEP:
            temp_value = pmu_get_register_ddie(PMU_DIG_SLP0, PMU_VSRAM_VOSEL_SLEEP_MASK, PMU_VSRAM_VOSEL_SLEEP_SHIFT);
            break;
        case PMU_NORMAL:
            temp_value = pmu_get_register_ddie(PMU_DIG_VSRAM4, PMU_VSRAM_VOSEL_NORMAL_MASK, PMU_VSRAM_VOSEL_NORMAL_SHIFT);
            break;
    }
    return temp_value;
}

void pmu_enable_power_ddie(pmu_power_domain_t domain, pmu_power_operate_t oper)
{
    switch (domain) {
        case PMU_LDO_VRF:
            pmu_set_register_ddie(PMU_DIG_VRF3, PMU_SW_VRF_EN_MASK, PMU_SW_VRF_EN_SHIFT, oper);
            hal_gpt_delay_ms(1);
            break;
    }
}
void pmu_set_power_state_ddie(pmu_power_stage_t ste)
{
    switch (ste) {
        case PMU_NORMAL:
            log_hal_msgid_info("[DPMU]DPMU Normal mode,no need setting", 0);
            break;
        case PMU_SLEEP:
            log_hal_msgid_info("[DPMU]DPMU Sleep mode,no need setting", 0);
            break;
        case PMU_RTC:
            log_hal_msgid_info("[DPMU]DPMU Enter RTC mode", 0);
            pmu_latch_con2(PMU_OFF, PMU_OFF, PMU_OFF, PMU_ON); //rg_strup_lvsh_rstb==1
            pmu_set_bgrbuf(PMU_ON, PMU_OFF, PMU_ON, PMU_OFF, PMU_ON, PMU_ON);
            pmu_latch_con2(PMU_OFF, PMU_OFF, PMU_OFF, PMU_OFF); //rg_strup_lvsh_rstb==0
            log_hal_msgid_info("[DPMU]PMU_DIG_BGRBUF0 :%x", 1, pmu_get_register_ddie(PMU_DIG_BGRBUF0, 0xffff, 0));
            hal_gpt_delay_ms(1);
#ifdef AIR_BTA_IC_PREMIUM_G3_TYPE_D
            pmu_set_register_ddie(PMU_DIG_GO_RTC, PMU_GO_RTC_MASK, PMU_GO_RTC_SHIFT, 0x1);
#endif
            break;
    }
}
void pmu_set_bgrbuf(int vref06_pl_dis, int bgr_buf_forceen, int hpbg_chop_en, int lpf_rsel, int bypass_mode_en, int zero_comp_en)
{
    uint32_t sta = 0;
    uint32_t data = 0 ;
    data = (vref06_pl_dis | (bgr_buf_forceen << 1) | (hpbg_chop_en << 2) | (lpf_rsel << 3) | (bypass_mode_en << 5) | (zero_comp_en << 6));
    pmu_set_register_ddie(PMU_DIG_BGRBUF0, 0x3f, 0, data);
    sta = (pmu_get_register_ddie(PMU_DIG_BGRBUF1, 0xff, 0) & 0x3ff);
    pmu_set_register_ddie(PMU_DIG_BGRBUF1, 0xffff, 0, (0x8000 | sta));
    pmu_set_register_ddie(PMU_DIG_BGRBUF1, 0xffff, 0, sta);
}
/*=========[BASIC]=========*/
/*If want to write strup register need
 * 1. select strup mux
 * 2. write latch data
 * 3. latch en
 * 4. check latch data and clean
 * The API pmu_strup_latch_register combin 1 and 2 for reduce the time
 * sel_date is latch data and select mux.
 * ==========b[7,6,5 , 4,3,2,1]=======
 * ===sel_date[select;  Data ]=======
 * */
void pmu_strup_latch_register(uint16_t data, pmu_strup_mux_t sel)
{
    uint16_t sel_date = 0;
    sel_date = (data | sel << 8);
    //log_hal_msgid_info("[DPMU]sel_date :%x",1,sel_date);
    pmu_set_register_ddie(PMU_DIG_STRUP_LATCH_DATA, 0xffff, 0, sel_date);
    pmu_set_register_ddie(PMU_DIG_STRUP_LATCH_SET, PMU_STRUP_RG_EN_MASK, PMU_STRUP_RG_EN_SHIFT, 0X1);
    pmu_set_register_ddie(PMU_DIG_STRUP_LATCH_SET, PMU_STRUP_RG_EN_MASK, PMU_STRUP_RG_EN_SHIFT, 0X0);
    do {
    } while (pmu_get_register_ddie(PMU_DIG_STRUP_LAT_READ_DATA, 0xf, PMU_STRUP_LATCH_READ_DATA_SHIFT) != (data&0xf));//E3
    pmu_set_register_ddie(PMU_DIG_STRUP_LATCH_DATA, 0xffff, 0, 0x0);
}
uint8_t pmu_get_power_off_reason_ddie(void) {
    uint8_t reason;
    reason = pmu_get_register_ddie(PMU_DIG_STRUP_MON2, 0xf, 0);
#ifdef AIR_BTA_IC_PREMIUM_G3_TYPE_D
#if !defined(__EXT_BOOTLOADER__)
    if (!dpmu_init_flag) {
        log_hal_info("[DPMU]power_off_reason : %s[%d]",power_off_reason[reason],reason);
    }
#endif
#endif
    return (reason & 0x0F);
}
uint8_t pmu_get_power_on_reason_ddie(void)
{
    uint8_t reason;
    reason = pmu_get_register_ddie(PMU_DIG_STRUP_MON2, 0x7, 4);
#ifdef AIR_BTA_IC_PREMIUM_G3_TYPE_D
#if !defined(__EXT_BOOTLOADER__)
    uint i = 0;
    if (!dpmu_init_flag) {
        for (i = 0; i < 5; i++) {
            if (reason & (0x1 << i)) {
                log_hal_info("[DPMU]power_on_reason : %s[%d]",power_on_reason[i],i);
            }
        }
    }
#endif
#endif
    return ((reason & 0x70) >> 4);
}
/*PMU IRQ*/
void pmu_eint_isr(void)
{
    log_hal_msgid_info("[DPMU]pmu_eint_isr", 0);
}
void pmu_ddie_eint_init(void)
{
    hal_nvic_register_isr_handler(PMU_IRQ_PRIORITY, (hal_nvic_isr_t)pmu_eint_isr);
}
void pmu_init_ddie(void)
{
    log_hal_msgid_info("[DPMU]pmu_init_ddie", 0);
    pmu_latch_con2(PMU_OFF, PMU_OFF, PMU_OFF, PMU_ON); //rg_strup_lvsh_rstb==1
    /*BOND_AD 0: D-die only; 1: with A-die pmu
     *BOND_USE_VRF 0: WO VRF ; 1:WI VRF */
    if (pmu_get_register_ddie(PMU_DIG_STRUP_MON0, PMU_BOND_AD_MASK, PMU_BOND_AD_SHIFT)) {
        log_hal_msgid_info("[DPMU]With D+A die PMIC", 0);
        pmu_strup_latch_register(0x0, PMU_STRUP_LATCH_CON0);
        if (pmu_get_register_ddie(PMU_DIG_STRUP_MON0, PMU_BOND_USE_VRF_MASK, PMU_BOND_USE_VRF_SHIFT)) {
            pmu_set_register_ddie(PMU_DIG_STRUP_RG_CON1, PMU_STRUP_RG_CON1_MASK, PMU_STRUP_RG_CON1_SHIFT, 0x34);
            pmu_set_register_ddie(PMU_DIG_VRF3, PMU_VRF_NDIS_EN_MASK, PMU_VRF_NDIS_EN_SHIFT, 0x1);
            log_hal_msgid_info("[DPMU]With VRF", 0);
        } else {
            pmu_set_register_ddie(PMU_DIG_STRUP_RG_CON1, PMU_STRUP_RG_CON1_MASK, PMU_STRUP_RG_CON1_SHIFT, 0x30);
            log_hal_msgid_info("[DPMU]Without VRF", 0);
        }
        /* turn off UVLO and AVDD18 in*/
        pmu_set_register_ddie(PMU_DIG_UVLO_VRTC, PMU_PMU_DIG_UVLO_VRTC_MASK, PMU_PMU_DIG_UVLO_VRTC_SHIFT, 0x0);
        pmu_set_register_ddie(PMU_DIG_UVLO_VRTC, PMU_PMU_DIG_UVLO_VRTC_MASK, PMU_PMU_DIG_UVLO_VRTC_SHIFT, 0x2);
        pmu_set_register_ddie(PMU_DIG_UVLO_VRTC, PMU_PMU_DIG_UVLO_VRTC_MASK, PMU_PMU_DIG_UVLO_VRTC_SHIFT, 0x0);
        pmu_set_register_ddie(PMU_DIG_UVLO_18IN, PMU_PMU_DIG_UVLO_18IN_MASK, PMU_PMU_DIG_UVLO_18IN_SHIFT, 0x0);
        pmu_set_register_ddie(PMU_DIG_UVLO_18IN, PMU_PMU_DIG_UVLO_18IN_MASK, PMU_PMU_DIG_UVLO_18IN_SHIFT, 0x2);
        pmu_set_register_ddie(PMU_DIG_UVLO_18IN, PMU_PMU_DIG_UVLO_18IN_MASK, PMU_PMU_DIG_UVLO_18IN_SHIFT, 0x0);
    } else { /*D only*/
        pmu_strup_latch_register(0x8, PMU_STRUP_LATCH_CON0);
        log_hal_msgid_info("[DPMU]D-die only", 0);
        pmu_set_register_ddie(PMU_DIG_VRF3, PMU_VRF_NDIS_EN_MASK, PMU_VRF_NDIS_EN_SHIFT, 0x1);
    }
    pmu_set_register_ddie(PMU_DIG_CTRL, PMU_WDTRSTB_COLD_EN_MASK, PMU_WDTRSTB_COLD_EN_SHIFT, 0x1);
    pmu_set_bgrbuf(PMU_ON, PMU_OFF, PMU_ON, PMU_OFF, PMU_OFF, PMU_ON);
    pmu_set_register_ddie(PMU_DIG_VSRAM2, PMU_BGR_RSV_MASK, PMU_BGR_RSV_SHIFT, 0x80); /* VSRAM reference from HPV2I.*/
    pmu_set_register_ddie(PMU_DIG_GO_SLP_SEL, PMU_SW_GO_SLP_SEL_MASK, PMU_SW_GO_SLP_SEL_SHIFT, 0x0); /* GO_SLEEP selection from spm.*/
#ifdef AIR_BTA_IC_PREMIUM_G3_TYPE_D
    pmu_set_register_ddie(PMU_DIG_GO_WDT_B_SEL, PMU_SW_RGU_WDT_B_SEL_MASK, PMU_SW_RGU_WDT_B_SEL_SHIFT, 0x0);
#else
    pmu_set_register_ddie(PMU_DIG_GO_WDT_B_SEL, PMU_SW_RGU_WDT_B_SEL_MASK, PMU_SW_RGU_WDT_B_SEL_SHIFT, 0x1);
#endif
    pmu_select_vsram_vosel_ddie(PMU_SLEEP, 0x48); /* VSRAM LDO voltage for sleep mode : 0.95V*/
    pmu_set_register_ddie(PMU_DIG_VSRAM2, 0xffff, 0, 0x1080); /* VSRAM LDO VOUT trim bit*/
    pmu_ddie_eint_init();
    pmu_set_register_ddie(PMU_DIG_VRF3, PMU_VRF_VOSEL_MASK, PMU_VRF_VOSEL_SHIFT, 0x18); //Set VRF from 0.8V to 0.9V
    pmu_enable_power_ddie(PMU_LDO_VRF, PMU_ON);
    pmu_latch_con2(PMU_OFF, PMU_OFF, PMU_OFF, PMU_OFF); //rg_strup_lvsh_rstb==0
    pmu_efuse_init_ddie();
    pmu_get_power_off_reason_ddie();
    pmu_get_power_on_reason_ddie();
    dpmu_init_flag=1;
}
/*==========[PMU Debug GPIO]==========*/
//00: normal
//01: pmu debug
//02: rtc debug
//03: cap debug
/*
 *
 * example : pmu_set_debug_mode(PMU,PMU,PMU,PMU);*/
void pmu_set_debug_mode(int gpio0_mode, int gpio1_mode, int gpio2_mode, int gpio3_mode)
{
    int data;
    data = gpio0_mode | gpio1_mode << 2 | gpio2_mode << 4 | gpio3_mode << 6;
    pmu_strup_latch_register(data, PMU_STRUP_LATCH_CON1); //select debug mode

}
/* Note: ¡§g¡¨ = 0 is analog mode and only for CAPTOUCH usage.
 * For output/debug mode, please set ¡§e¡¨ = 1. For input/RTC_EINT mode, please set to 0
*/
void pmu_rtc_gpio_config(pmu_strup_mux_t strup_mux, uint8_t data)
{
    pmu_strup_latch_register(data, strup_mux);
}

uint32_t pmu_get_rtc_gpio_status(pmu_strup_mux_t strup_mux)
{
    uint32_t temp_status=0;
    switch(strup_mux){
    case PMU_STRUP_RTC_GPIO0:
        pmu_set_register_ddie(PMU_DIG_STRUP_LATCH_DATA, 0xffff, 0, 0x300);
        break;
    case PMU_STRUP_RTC_GPIO1:
        pmu_set_register_ddie(PMU_DIG_STRUP_LATCH_DATA, 0xffff, 0, 0x400);
            break;
    case PMU_STRUP_RTC_GPIO2:
        pmu_set_register_ddie(PMU_DIG_STRUP_LATCH_DATA, 0xffff, 0, 0x500);
            break;
    case PMU_STRUP_RTC_GPIO3:
        pmu_set_register_ddie(PMU_DIG_STRUP_LATCH_DATA, 0xffff, 0, 0x600);
            break;
    }
    temp_status = pmu_get_register_ddie(PMU_DIG_STRUP_LAT_READ_DATA, 0xffff, 0);
    log_hal_msgid_info("[DPMU]GPIO[%d]GPIO READ_DAT:%x Status[%x]", 3,(strup_mux-PMU_STRUP_RTC_GPIO0),temp_status,(temp_status>>7));
    temp_status = (temp_status>>7);
    return temp_status;
}
void pmu_select_rtc_voltage_ddie(pmu_vrtc_voltage_t vol)
{
    pmu_set_register_ddie(PMU_DIG_VRTC1, PMU_VRTC_VOSEL_MASK, PMU_VRTC_VOSEL_SHIFT, vol);
}
void pmu_latch_con2(int rg_mon_sel, int rg_mon_en, int rg_dbg_high_byte_sel, int stup_lvsh_en)
{
    int data;
    data = rg_mon_sel | rg_mon_en << 3 | rg_dbg_high_byte_sel << 4 | stup_lvsh_en << 7;
    pmu_strup_latch_register(data, PMU_STRUP_LATCH_CON2);
}
pmu_operate_status_t pmu_set_register_ddie(uint32_t address, short int mask, short int shift, short int value)
{
    uint32_t mask_buffer, target_value;
    mask_buffer = (~(mask << shift));
    target_value = *((volatile uint32_t *)(address));
    target_value &= mask_buffer;
    target_value |= (value << shift);
    *((volatile uint32_t *)(address)) = target_value;
    return PMU_OK;
}
uint32_t pmu_get_register_ddie(uint32_t address, short int mask, short int shift)
{
    uint32_t change_value, mask_buffer;
    mask_buffer = (mask << shift);
    change_value = *((volatile uint32_t *)(address));
    change_value &= mask_buffer;
    change_value = (change_value >> shift);
    return change_value;
}

/*==========[Efuse]==========*/
uint8_t efuse_status = 0;
void pmu_efuse_init_ddie(void)
{
    uint32_t temp_val = 0;
    efuse_status = pmu_get_register_value_ddie(PMU_M_ANA_CFG_DPMU0, PMU_BROM_DPMU_TRIM_EN_MASK, PMU_BROM_DPMU_TRIM_EN_SHIFT);
    if (efuse_status) {
        log_hal_msgid_info("[DPMU]Start DPMU Slim efuse setting:%d", 1, efuse_status);
        log_hal_msgid_info("[DPMU]PMU_M_ANA_CFG_DPMU0:%x", 1, *((volatile uint32_t *)0x420C0348));
        log_hal_msgid_info("[DPMU]PMU_M_ANA_CFG_DPMU1:%x", 1, *((volatile uint32_t *)0x420C034C));
        log_hal_msgid_info("[DPMU]PMU_M_ANA_CFG_DPMU2:%x", 1, *((volatile uint32_t *)0x420C0350));
        log_hal_msgid_info("[DPMU]PMU_M_ANA_CFG_DPMU3:%x", 1, *((volatile uint32_t *)0x420C0354));
        pmu_latch_con2(PMU_OFF, PMU_OFF, PMU_OFF, PMU_ON); //rg_strup_lvsh_rstb==1
        temp_val = pmu_get_register_value_ddie(PMU_M_ANA_CFG_DPMU0, PMU_LPBG_VREF0P45TRIM_TARGET_MASK, PMU_LPBG_VREF0P45TRIM_TARGET_SHIFT);
        pmu_set_register_value_ddie(PMU_DIG_LPBG4, PMU_LPBG_VREF0P45TRIM_MASK, PMU_LPBG_VREF0P45TRIM_SHIFT, temp_val);
        temp_val = pmu_get_register_value_ddie(PMU_M_ANA_CFG_DPMU0, PMU_LPBG_VREF0P6TRIM_TARGET_MASK, PMU_LPBG_VREF0P6TRIM_TARGET_SHIFT);
        pmu_set_register_value_ddie(PMU_DIG_LPBG4, PMU_LPBG_VREF0P6TRIM_MASK, PMU_LPBG_VREF0P6TRIM_SHIFT, temp_val);
        temp_val = pmu_get_register_value_ddie(PMU_M_ANA_CFG_DPMU0, PMU_LPBG_VREF0P85TRIM_TARGET_MASK, PMU_LPBG_VREF0P85TRIM_TARGET_SHIFT);
        pmu_set_register_value_ddie(PMU_DIG_LPBG5, PMU_LPBG_VREF0P85TRIM_MASK, PMU_LPBG_VREF0P85TRIM_SHIFT, temp_val);
        temp_val = pmu_get_register_value_ddie(PMU_M_ANA_CFG_DPMU0, PMU_LPBG_IBTRIM_TARGET_MASK, PMU_LPBG_IBTRIM_TARGET_SHIFT);
        pmu_set_register_value_ddie(PMU_DIG_LPBG0, PMU_LPBG_IBTRIM_MASK, PMU_LPBG_IBTRIM_SHIFT, temp_val);
        temp_val = pmu_get_register_value_ddie(PMU_M_ANA_CFG_DPMU1, PMU_VREF0P6_TRIM_HPBG_TARGET_MASK, PMU_VREF0P6_TRIM_HPBG_TARGET_SHIFT);
        pmu_set_register_value_ddie(PMU_DIG_HPBG3, PMU_VREF0P6_TRIM_HPBG_MASK, PMU_VREF0P6_TRIM_HPBG_SHIFT, temp_val);
        temp_val = pmu_get_register_value_ddie(PMU_M_ANA_CFG_DPMU1, PMU_VREF0P45_TRIM_HPBG_TARGET_MASK, PMU_VREF0P45_TRIM_HPBG_TARGET_SHIFT);
        pmu_set_register_value_ddie(PMU_DIG_HPBG4, PMU_VREF0P45_TRIM_HPBG_MASK, PMU_VREF0P45_TRIM_HPBG_SHIFT, temp_val);
        temp_val = pmu_get_register_value_ddie(PMU_M_ANA_CFG_DPMU1, PMU_VREF0P85_TRIM_HPBG_TARGET_MASK, PMU_VREF0P85_TRIM_HPBG_TARGET_SHIFT);
        pmu_set_register_value_ddie(PMU_DIG_HPBG2, PMU_VREF0P85_TRIM_HPBG_MASK, PMU_VREF0P85_TRIM_HPBG_SHIFT, temp_val);
        temp_val = pmu_get_register_value_ddie(PMU_M_ANA_CFG_DPMU1, PMU_VREF1P205_TRIM_HPBG_TARGET_MASK, PMU_VREF1P205_TRIM_HPBG_TARGET_SHIFT);
        pmu_set_register_value_ddie(PMU_DIG_HPBG2, PMU_VREF1P205_TRIM_HPBG_MASK, PMU_VREF1P205_TRIM_HPBG_SHIFT, temp_val);
        temp_val = pmu_get_register_value_ddie(PMU_M_ANA_CFG_DPMU1, PMU_IBIAS_TRIM_HPBG_TARGET_MASK, PMU_IBIAS_TRIM_HPBG_TARGET_SHIFT);
        pmu_set_register_value_ddie(PMU_DIG_HPBG0, PMU_IBIAS_TRIM_HPBG_MASK, PMU_IBIAS_TRIM_HPBG_SHIFT, temp_val);
        pmu_set_register_value_ddie(PMU_DIG_HPBG0, PMU_BGR_TCTRIM_HPBG_MASK, PMU_BGR_TCTRIM_HPBG_SHIFT, 0xD);
        temp_val = pmu_get_register_value_ddie(PMU_M_ANA_CFG_DPMU2, PMU_VRF_VOTRIM_TARGET_MASK, PMU_VRF_VOTRIM_TARGET_SHIFT);
        pmu_set_register_value_ddie(PMU_DIG_VRF2, PMU_VRF_VOTRIM_MASK, PMU_VRF_VOTRIM_SHIFT, temp_val);
        temp_val = pmu_get_register_value_ddie(PMU_M_ANA_CFG_DPMU2, PMU_VSRAM_VOTRIM_TARGET_MASK, PMU_VSRAM_VOTRIM_TARGET_SHIFT);
        pmu_set_register_value_ddie(PMU_DIG_VSRAM2, PMU_VSRAM_VOTRIM_MASK, PMU_VSRAM_VOTRIM_SHIFT, temp_val);
        temp_val = pmu_get_register_value_ddie(PMU_M_ANA_CFG_DPMU2, PMU_VUSB_VOTRIM_TARGET_MASK, PMU_VUSB_VOTRIM_TARGET_SHIFT);
        pmu_set_register_value_ddie(PMU_DIG_USB1, PMU_VUSB_VOTRIM_MASK, PMU_VUSB_VOTRIM_SHIFT, temp_val);
        temp_val = pmu_get_register_value_ddie(PMU_M_ANA_CFG_DPMU3, PMU_VRTC_VOTRIM_TARGET_MASK, PMU_VRTC_VOTRIM_TARGET_SHIFT);
        pmu_set_register_value_ddie(PMU_DIG_VRTC2, PMU_VRTC_VOTRIM_MASK, PMU_VRTC_VOTRIM_SHIFT, temp_val);
        pmu_set_register_value_ddie(PMU_DIG_LPBG5, PMU_SET_LPBG_VREFTRIM_MASK, PMU_SET_LPBG_VREFTRIM_SHIFT, 0x1);
        pmu_set_register_value_ddie(PMU_DIG_HPBG4, PMU_SET_LPBG_IBTRIM_MASK, PMU_SET_LPBG_IBTRIM_SHIFT, 0x1);
        pmu_set_register_value_ddie(PMU_DIG_LPBG1, PMU_SET_LPBG_TCTRIM_MASK, PMU_SET_LPBG_TCTRIM_SHIFT, 0x1);
        pmu_set_register_value_ddie(PMU_DIG_HPBG4, PMU_SET_VREF_TRIM_HPBG_MASK, PMU_SET_VREF_TRIM_HPBG_SHIFT, 0x1);
        pmu_set_register_value_ddie(PMU_DIG_HPBG1, PMU_SET_TCTRIM_HPBG_MASK, PMU_SET_TCTRIM_HPBG_SHIFT, 0x1);
        pmu_set_register_value_ddie(PMU_DIG_LPBG5, PMU_SET_LPBG_VREFTRIM_MASK, PMU_SET_LPBG_VREFTRIM_SHIFT, 0x0);
        pmu_set_register_value_ddie(PMU_DIG_HPBG4, PMU_SET_LPBG_IBTRIM_MASK, PMU_SET_LPBG_IBTRIM_SHIFT, 0x0);
        pmu_set_register_value_ddie(PMU_DIG_LPBG1, PMU_SET_LPBG_TCTRIM_MASK, PMU_SET_LPBG_TCTRIM_SHIFT, 0x0);
        pmu_set_register_value_ddie(PMU_DIG_HPBG4, PMU_SET_VREF_TRIM_HPBG_MASK, PMU_SET_VREF_TRIM_HPBG_SHIFT, 0x0);
        pmu_set_register_value_ddie(PMU_DIG_HPBG1, PMU_SET_TCTRIM_HPBG_MASK, PMU_SET_TCTRIM_HPBG_SHIFT, 0x0);
        pmu_latch_con2(PMU_OFF, PMU_OFF, PMU_OFF, PMU_OFF); //rg_strup_lvsh_rstb==0
        log_hal_msgid_info("[DPMU]PMU_DIG_HPBG0 [%x]", 1, pmu_get_register_value_ddie(PMU_DIG_HPBG0, 0xffff, 0));
        log_hal_msgid_info("[DPMU]PMU_DIG_HPBG1 [%x]", 1, pmu_get_register_value_ddie(PMU_DIG_HPBG1, 0xffff, 0));
        log_hal_msgid_info("[DPMU]PMU_DIG_HPBG2 [%x]", 1, pmu_get_register_value_ddie(PMU_DIG_HPBG2, 0xffff, 0));
        log_hal_msgid_info("[DPMU]PMU_DIG_HPBG3 [%x]", 1, pmu_get_register_value_ddie(PMU_DIG_HPBG3, 0xffff, 0));
        log_hal_msgid_info("[DPMU]PMU_DIG_HPBG4 [%x]", 1, pmu_get_register_value_ddie(PMU_DIG_HPBG4, 0xffff, 0));
    } else {
        log_hal_msgid_info("[DPMU]DPMU Slim efuse not ready:%d", 1, efuse_status);
    }

}

/*==========[BC 1.2 behavior]==========*/
void pmu_bc12_init(void)
{
    pmu_set_register_value_ddie(PMU_BC12_IBIAS_EN_V12_ADDR, PMU_BC12_IBIAS_EN_V12_MASK, PMU_BC12_IBIAS_EN_V12_SHIFT, 0x1);
    hal_gpt_delay_ms(10);
    pmu_set_register_value_ddie(PMU_BC12_SRC_EN_V12_ADDR, PMU_BC12_SRC_EN_V12_MASK, PMU_BC12_SRC_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_VREF_VTH_EN_V12_ADDR, PMU_BC12_VREF_VTH_EN_V12_MASK, PMU_BC12_VREF_VTH_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_CMP_EN_V12_ADDR, PMU_BC12_CMP_EN_V12_MASK, PMU_BC12_CMP_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_IPU_EN_V12_ADDR, PMU_BC12_IPU_EN_V12_MASK, PMU_BC12_IPU_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_IPD_EN_V12_ADDR, PMU_BC12_IPD_EN_V12_MASK, PMU_BC12_IPD_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_IPD_HALF_EN_V12_ADDR, PMU_BC12_IPD_HALF_EN_V12_MASK, PMU_BC12_IPD_HALF_EN_V12_SHIFT, 0);
}

void pmu_bc12_end(void)
{
    pmu_set_register_value_ddie(PMU_BC12_IBIAS_EN_V12_ADDR, PMU_BC12_IBIAS_EN_V12_MASK, PMU_BC12_IBIAS_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_SRC_EN_V12_ADDR, PMU_BC12_SRC_EN_V12_MASK, PMU_BC12_SRC_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_VREF_VTH_EN_V12_ADDR, PMU_BC12_VREF_VTH_EN_V12_MASK, PMU_BC12_VREF_VTH_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_CMP_EN_V12_ADDR, PMU_BC12_CMP_EN_V12_MASK, PMU_BC12_CMP_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_IPU_EN_V12_ADDR, PMU_BC12_IPU_EN_V12_MASK, PMU_BC12_IPU_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_IPD_EN_V12_ADDR, PMU_BC12_IPD_EN_V12_MASK, PMU_BC12_IPD_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_IPD_HALF_EN_V12_ADDR, PMU_BC12_IPD_HALF_EN_V12_MASK, PMU_BC12_IPD_HALF_EN_V12_SHIFT, 0);
}
uint8_t pmu_bc12_dcd(void)
{
    uint8_t value = 0xFF;
#ifdef PMU_BC12_DCD_ENABLE
    pmu_set_register_value_ddie(PMU_BC12_SRC_EN_V12_ADDR, PMU_BC12_SRC_EN_V12_MASK, PMU_BC12_SRC_EN_V12_SHIFT, 2);
    hal_gpt_delay_ms(500);
#endif
    pmu_set_register_value_ddie(PMU_BC12_SRC_EN_V12_ADDR, PMU_BC12_SRC_EN_V12_MASK, PMU_BC12_SRC_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_VREF_VTH_EN_V12_ADDR, PMU_BC12_VREF_VTH_EN_V12_MASK, PMU_BC12_VREF_VTH_EN_V12_SHIFT, 1);
    pmu_set_register_value_ddie(PMU_BC12_CMP_EN_V12_ADDR, PMU_BC12_CMP_EN_V12_MASK, PMU_BC12_CMP_EN_V12_SHIFT, 2);
    pmu_set_register_value_ddie(PMU_BC12_IPU_EN_V12_ADDR, PMU_BC12_IPU_EN_V12_MASK, PMU_BC12_IPU_EN_V12_SHIFT, 2);
    pmu_set_register_value_ddie(PMU_BC12_IPD_EN_V12_ADDR, PMU_BC12_IPD_EN_V12_MASK, PMU_BC12_IPD_EN_V12_SHIFT, 1);
    pmu_set_register_value_ddie(PMU_BC12_IPD_HALF_EN_V12_ADDR, PMU_BC12_IPD_HALF_EN_V12_MASK, PMU_BC12_IPD_HALF_EN_V12_SHIFT, 0);
    hal_gpt_delay_ms(10);
    value = pmu_get_register_value_ddie(PMU_AQ_QI_BC12_CMP_OUT_V12_ADDR, PMU_AQ_QI_BC12_CMP_OUT_V12_MASK, PMU_AQ_QI_BC12_CMP_OUT_V12_SHIFT);;
    hal_gpt_delay_ms(10);
    return value;
}

uint8_t pmu_bc12_primary_detction(void)
{
    uint8_t compareResult = 0xFF;
    pmu_set_register_value_ddie(PMU_BC12_SRC_EN_V12_ADDR, PMU_BC12_SRC_EN_V12_MASK, PMU_BC12_SRC_EN_V12_SHIFT, 2);
    pmu_set_register_value_ddie(PMU_BC12_VREF_VTH_EN_V12_ADDR, PMU_BC12_VREF_VTH_EN_V12_MASK, PMU_BC12_VREF_VTH_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_CMP_EN_V12_ADDR, PMU_BC12_CMP_EN_V12_MASK, PMU_BC12_CMP_EN_V12_SHIFT, 1);
    pmu_set_register_value_ddie(PMU_BC12_IPU_EN_V12_ADDR, PMU_BC12_IPU_EN_V12_MASK, PMU_BC12_IPU_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_IPD_EN_V12_ADDR, PMU_BC12_IPD_EN_V12_MASK, PMU_BC12_IPD_EN_V12_SHIFT, 1);
    pmu_set_register_value_ddie(PMU_BC12_IPD_HALF_EN_V12_ADDR, PMU_BC12_IPD_HALF_EN_V12_MASK, PMU_BC12_IPD_HALF_EN_V12_SHIFT, 0);
    hal_gpt_delay_ms(40);
    compareResult = pmu_get_register_value_ddie(PMU_AQ_QI_BC12_CMP_OUT_V12_ADDR, PMU_AQ_QI_BC12_CMP_OUT_V12_MASK, PMU_AQ_QI_BC12_CMP_OUT_V12_SHIFT);
    hal_gpt_delay_ms(20);
    return compareResult;
}

uint8_t pmu_bc12_secondary_detection(void)
{
    uint8_t compareResult = 0xFF;
    pmu_set_register_value_ddie(PMU_BC12_SRC_EN_V12_ADDR, PMU_BC12_SRC_EN_V12_MASK, PMU_BC12_SRC_EN_V12_SHIFT, 1);
    pmu_set_register_value_ddie(PMU_BC12_VREF_VTH_EN_V12_ADDR, PMU_BC12_VREF_VTH_EN_V12_MASK, PMU_BC12_VREF_VTH_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_CMP_EN_V12_ADDR, PMU_BC12_CMP_EN_V12_MASK, PMU_BC12_CMP_EN_V12_SHIFT, 2);
    pmu_set_register_value_ddie(PMU_BC12_IPU_EN_V12_ADDR, PMU_BC12_IPU_EN_V12_MASK, PMU_BC12_IPU_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_IPD_EN_V12_ADDR, PMU_BC12_IPD_EN_V12_MASK, PMU_BC12_IPD_EN_V12_SHIFT, 2);
    pmu_set_register_value_ddie(PMU_BC12_IPD_HALF_EN_V12_ADDR, PMU_BC12_IPD_HALF_EN_V12_MASK, PMU_BC12_IPD_HALF_EN_V12_SHIFT, 0);
    hal_gpt_delay_ms(40);
    compareResult = pmu_get_register_value_ddie(PMU_AQ_QI_BC12_CMP_OUT_V12_ADDR, PMU_AQ_QI_BC12_CMP_OUT_V12_MASK, PMU_AQ_QI_BC12_CMP_OUT_V12_SHIFT);
    hal_gpt_delay_ms(20);
    return compareResult;
}

uint8_t pmu_bc12_check_DCP(void)
{
    uint8_t compareResult = 0xFF;
    pmu_set_register_value_ddie(PMU_BC12_SRC_EN_V12_ADDR, PMU_BC12_SRC_EN_V12_MASK, PMU_BC12_SRC_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_VREF_VTH_EN_V12_ADDR, PMU_BC12_VREF_VTH_EN_V12_MASK, PMU_BC12_VREF_VTH_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_CMP_EN_V12_ADDR, PMU_BC12_CMP_EN_V12_MASK, PMU_BC12_CMP_EN_V12_SHIFT, 1);
    pmu_set_register_value_ddie(PMU_BC12_IPU_EN_V12_ADDR, PMU_BC12_IPU_EN_V12_MASK, PMU_BC12_IPU_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_IPD_EN_V12_ADDR, PMU_BC12_IPD_EN_V12_MASK, PMU_BC12_IPD_EN_V12_SHIFT, 2);
    pmu_set_register_value_ddie(PMU_BC12_IPD_HALF_EN_V12_ADDR, PMU_BC12_IPD_HALF_EN_V12_MASK, PMU_BC12_IPD_HALF_EN_V12_SHIFT, 0);
    hal_gpt_delay_ms(10);
    pmu_set_register_value_ddie(PMU_BC12_IPD_EN_V12_ADDR, PMU_BC12_IPD_EN_V12_MASK, PMU_BC12_IPD_EN_V12_SHIFT, 0);
    hal_gpt_delay_us(200);
    compareResult = pmu_get_register_value_ddie(PMU_AQ_QI_BC12_CMP_OUT_V12_ADDR, PMU_AQ_QI_BC12_CMP_OUT_V12_MASK, PMU_AQ_QI_BC12_CMP_OUT_V12_SHIFT);
    return compareResult;
}

uint8_t pmu_bc12_check_dp(void)
{
    uint8_t compareResult = 0xFF;
    pmu_set_register_value_ddie(PMU_BC12_SRC_EN_V12_ADDR, PMU_BC12_SRC_EN_V12_MASK, PMU_BC12_SRC_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_VREF_VTH_EN_V12_ADDR, PMU_BC12_VREF_VTH_EN_V12_MASK, PMU_BC12_VREF_VTH_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_CMP_EN_V12_ADDR, PMU_BC12_CMP_EN_V12_MASK, PMU_BC12_CMP_EN_V12_SHIFT, 2);
    pmu_set_register_value_ddie(PMU_BC12_IPU_EN_V12_ADDR, PMU_BC12_IPU_EN_V12_MASK, PMU_BC12_IPU_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_IPD_EN_V12_ADDR, PMU_BC12_IPD_EN_V12_MASK, PMU_BC12_IPD_EN_V12_SHIFT, 2);
    pmu_set_register_value_ddie(PMU_BC12_IPD_HALF_EN_V12_ADDR, PMU_BC12_IPD_HALF_EN_V12_MASK, PMU_BC12_IPD_HALF_EN_V12_SHIFT, 0);
    hal_gpt_delay_ms(10);
    compareResult = pmu_get_register_value_ddie(PMU_AQ_QI_BC12_CMP_OUT_V12_ADDR, PMU_AQ_QI_BC12_CMP_OUT_V12_MASK, PMU_AQ_QI_BC12_CMP_OUT_V12_SHIFT);
    hal_gpt_delay_ms(10);
    return compareResult;
}

uint8_t pmu_bc12_check_dm(void)
{
    uint8_t compareResult = 0xFF;
    pmu_set_register_value_ddie(PMU_BC12_SRC_EN_V12_ADDR, PMU_BC12_SRC_EN_V12_MASK, PMU_BC12_SRC_EN_V12_SHIFT, 0);
    // pmu_bc12_set_vref_vth_en_v12(0);
    pmu_set_register_value_ddie(PMU_BC12_CMP_EN_V12_ADDR, PMU_BC12_CMP_EN_V12_MASK, PMU_BC12_CMP_EN_V12_SHIFT, 1);
    pmu_set_register_value_ddie(PMU_BC12_IPU_EN_V12_ADDR, PMU_BC12_IPU_EN_V12_MASK, PMU_BC12_IPU_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_IPD_EN_V12_ADDR, PMU_BC12_IPD_EN_V12_MASK, PMU_BC12_IPD_EN_V12_SHIFT, 1);
    pmu_set_register_value_ddie(PMU_BC12_IPD_HALF_EN_V12_ADDR, PMU_BC12_IPD_HALF_EN_V12_MASK, PMU_BC12_IPD_HALF_EN_V12_SHIFT, 0);
    hal_gpt_delay_ms(10);
    compareResult = pmu_get_register_value_ddie(PMU_AQ_QI_BC12_CMP_OUT_V12_ADDR, PMU_AQ_QI_BC12_CMP_OUT_V12_MASK, PMU_AQ_QI_BC12_CMP_OUT_V12_SHIFT);
    hal_gpt_delay_ms(10);
    return compareResult;
}

uint8_t pmu_bc12_check_floating(void)
{
    uint8_t compareResult = 0xFF;
    pmu_set_register_value_ddie(PMU_BC12_SRC_EN_V12_ADDR, PMU_BC12_SRC_EN_V12_MASK, PMU_BC12_SRC_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_VREF_VTH_EN_V12_ADDR, PMU_BC12_VREF_VTH_EN_V12_MASK, PMU_BC12_VREF_VTH_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_CMP_EN_V12_ADDR, PMU_BC12_CMP_EN_V12_MASK, PMU_BC12_CMP_EN_V12_SHIFT, 1);
    pmu_set_register_value_ddie(PMU_BC12_IPU_EN_V12_ADDR, PMU_BC12_IPU_EN_V12_MASK, PMU_BC12_IPU_EN_V12_SHIFT, 0);
    pmu_set_register_value_ddie(PMU_BC12_IPD_EN_V12_ADDR, PMU_BC12_IPD_EN_V12_MASK, PMU_BC12_IPD_EN_V12_SHIFT, 1);
    pmu_set_register_value_ddie(PMU_BC12_IPD_HALF_EN_V12_ADDR, PMU_BC12_IPD_HALF_EN_V12_MASK, PMU_BC12_IPD_HALF_EN_V12_SHIFT, 1);
    hal_gpt_delay_ms(10);
    compareResult = pmu_get_register_value_ddie(PMU_AQ_QI_BC12_CMP_OUT_V12_ADDR, PMU_AQ_QI_BC12_CMP_OUT_V12_MASK, PMU_AQ_QI_BC12_CMP_OUT_V12_SHIFT);
    hal_gpt_delay_ms(10);
    return compareResult;
}

uint8_t pmu_get_bc12_charger_type(void)
{
    pmu_bc12_init();
    if (pmu_bc12_dcd() == 0) {                       //SDP/CDP/DCP/S charger
        if (pmu_bc12_primary_detction() == 0) {      //SDP
            pmu_bc12_end();
            log_hal_msgid_info("[DPMU]BC12 SDP Charger\r\n", 0);
            pmu_charger_type = SDP_CHARGER;
            return SDP_CHARGER;
        } else {                                                    //CDP/DCP/S charger
            if (pmu_bc12_secondary_detection() == 0) { //CDP
                pmu_bc12_end();
                log_hal_msgid_info("[DPMU]BC12 CDP Charger\r\n", 0);
                pmu_charger_type = CDP_CHARGER;
                return CDP_CHARGER;
            } else {
                if (pmu_bc12_check_DCP() == 0) {      //DCP
                    pmu_bc12_end();
                    log_hal_msgid_info("[DPMU]BC12 DCP Charger\r\n", 0);
                    pmu_charger_type = DCP_CHARGER;
                    return DCP_CHARGER;
                } else {                                            //S charger
                    pmu_bc12_end();
                    log_hal_msgid_info("[DPMU]BC12 SS Charger\r\n", 0);
                    pmu_charger_type = SS_TABLET_CHARGER;
                    return SS_TABLET_CHARGER;
                }
            }
        }
    } else {                                                        //apple /non-standard/ DP&DM floating charger
        uint8_t dp_out, dm_out;
        dp_out = pmu_bc12_check_dp();
        dm_out = pmu_bc12_check_dm();
        if ((dp_out == 0) && (dm_out == 0)) {
            if (pmu_bc12_check_floating() == 0) {             //DP&DM floating charger
                pmu_bc12_end();
                log_hal_msgid_info("[DPMU]BC12 DP&DM Floating\r\n", 0);
                pmu_charger_type = DP_DM_FLOATING;
                return DP_DM_FLOATING;
            } else {                                                     //non-standard charger
                pmu_bc12_end();
                log_hal_msgid_info("[DPMU]BC12 NON-STD charger\r\n", 0);
                pmu_charger_type = NON_STD_CHARGER;
                return NON_STD_CHARGER;
            }
        } else if ((dp_out == 0) && (dm_out == 1)) {                  //apple 5V 1A charger
            pmu_bc12_end();
            log_hal_msgid_info("[DPMU]BC12 IPHONE_5V_1A Charger\r\n", 0);
            pmu_charger_type = IPHONE_5V_1A_CHARGER;
            return IPHONE_5V_1A_CHARGER;
        } else {                                                     //apple ipad2/ipad4 charger
            pmu_bc12_end();
            log_hal_msgid_info("[DPMU]BC12 IPAD2_IPAD4 Charger\r\n", 0);
            pmu_charger_type = IPAD2_IPAD4_CHARGER;
            return IPAD2_IPAD4_CHARGER;
        }
    }
}

void pmu_dump_rg_ddie(void)
{
    uint32_t addr;
    uint32_t value __unused;
    const uint32_t addr_start = PMU_DIG_RSV0;
    const uint32_t addr_end = PMU_DIG_ATPG;

    /**
     * NOTE: Adie address is 32 bits wide
     */
    for (addr = addr_start; addr <= addr_end; addr += 4) {
        value = *((volatile uint32_t *)addr);
        log_hal_msgid_info("[DPMU]pmu_dump_rg_ddie, DPMU Addr[0x%08X] Value[0x%08X]", 2, addr, value);
    }
}

#ifdef PMU_SLT_ENV
void dpmu_performance_level(pmu_slt_mode_t mode)
{
    uint32_t temp = 0;
    switch (mode){
        case PMU_HIGH_LEVEL:
            //VSRAM
            pmu_set_register_ddie(PMU_DIG_VSRAM2, PMU_BGR_RSV_MASK, PMU_BGR_RSV_SHIFT, 0x80);
            pmu_set_register_ddie(PMU_DIG_VSRAM3, 0xFFFF, 0x0, 0x0110);
            pmu_set_register_ddie(PMU_DIG_STRUP_RG_CON0, 0x1, 5, 0x1);
            pmu_set_register_ddie(PMU_DIG_VSRAM4, PMU_VSRAM_VOSEL_NORMAL_MASK, PMU_VSRAM_VOSEL_NORMAL_SHIFT, 0x4C);
            //VRF
            pmu_set_register_ddie(PMU_DIG_STRUP_RG_CON0, 0x1, 7, 0x1);
            pmu_set_register_ddie(PMU_DIG_VRF3, PMU_VRF_VOSEL_MASK, PMU_VRF_VOSEL_SHIFT, 0x1C);
            //VA12
            pmu_set_register_ddie(PMU_DIG_USB2, 0x1, 8, 0x1);
            pmu_set_register_ddie(PMU_DIG_ATSTSEL0, 0x1, 6, 0x1);
            pmu_set_register_value_ddie(PMU_DIG_USB1, PMU_VUSB_VOSEL_MASK, PMU_VUSB_VOSEL_SHIFT, 0xF);
            //VRTC
            pmu_set_register_ddie(PMU_DIG_VRTC1, 0x1F, 0, 0xF);
            pmu_set_register_ddie(PMU_DIG_VRTC3, 0x1, 4, 0x0);
            pmu_set_register_ddie(PMU_DIG_VRTC3, 0x1, 4, 0x1);
            pmu_set_register_ddie(PMU_DIG_VRTC3, 0x1, 4, 0x0);
        break;
        case PMU_LOW_LEVEL:
            //VSRAM
            pmu_set_register_ddie(PMU_DIG_VSRAM2, PMU_BGR_RSV_MASK, PMU_BGR_RSV_SHIFT, 0x80);
            pmu_set_register_ddie(PMU_DIG_VSRAM3, 0xFFFF, 0x0, 0x0110);
            pmu_set_register_ddie(PMU_DIG_STRUP_RG_CON0, 0x1, 5, 0x1);
            pmu_set_register_ddie(PMU_DIG_VSRAM4, PMU_VSRAM_VOSEL_NORMAL_MASK, PMU_VSRAM_VOSEL_NORMAL_SHIFT, 0x43);
            //VRF
            pmu_set_register_ddie(PMU_DIG_STRUP_RG_CON0, 0x1, 7, 0x1);
            pmu_set_register_ddie(PMU_DIG_VRF3, PMU_VRF_VOSEL_MASK, PMU_VRF_VOSEL_SHIFT, 0x13);
            //VA12
            pmu_set_register_ddie(PMU_DIG_USB2, 0x1, 8, 0x1);
            pmu_set_register_ddie(PMU_DIG_ATSTSEL0, 0x1, 6, 0x1);
            pmu_set_register_value_ddie(PMU_DIG_USB1, PMU_VUSB_VOSEL_MASK, PMU_VUSB_VOSEL_SHIFT, 0x7);
            //VRTC
            pmu_set_register_ddie(PMU_DIG_VRTC1, 0x1F, 0, 0x5);
            pmu_set_register_ddie(PMU_DIG_VRTC3, 0x1, 4, 0x0);
            pmu_set_register_ddie(PMU_DIG_VRTC3, 0x1, 4, 0x1);
            pmu_set_register_ddie(PMU_DIG_VRTC3, 0x1, 4, 0x0);
        break;
        default:
        break;
    }
}
#endif

#endif /* HAL_PMU_MODULE_ENABLED */

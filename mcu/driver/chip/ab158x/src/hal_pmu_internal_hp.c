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
#include <string.h>
#include "hal_i2c_master.h"
#include "hal_nvic_internal.h"
#include "assert.h"
#include "syslog.h"
#include "hal_pmu.h"
#include "hal_pmu_internal_hp.h"
#include "hal_pmu_charger_hp.h"
#include "hal_pmu_auxadc_hp.h"
#include "hal_pmu_hp_platform.h"

#ifndef HAL_PMU_NO_RTOS
#include "FreeRTOS.h"
#include "portmacro.h"
#endif

#ifdef HAL_SLEEP_MANAGER_ENABLED
#include "hal_sleep_manager_platform.h"
#include "hal_sleep_manager_internal.h"
#include "hal_sleep_manager.h"
#endif
#ifdef HAL_RTC_MODULE_ENABLED
#include "hal_rtc.h"
#include "hal_rtc_internal.h"
#endif
#ifndef HAL_PMU_NO_RTOS
extern BaseType_t xTaskResumeAll(void);
extern void vTaskSuspendAll(void);
#endif
#if defined(AIR_1WIRE_ENABLE)
#define PMU_VBUS_UART
#endif
#define POWER_ON_SIZE 16
#define POWER_OFF_SIZE 24
const char *power_on_reason[POWER_ON_SIZE] = {
    "pwrkey, Press power key to power on",
    "rtca, Power on from rtc mode",
    "chrin, Power on by charger in",
    "chrout, Power on by charger out",
    "rboot, Power on for cold reset",
    "Reserve",
    "Reserve",
    "Reserve",
    "ponsts_clr, Clear PONSTS",
    "Reserve",
    "Reserve",
    "Reserve",
    "Reserve",
    "Reserve",
    "Reserve",
    "boot_mode, indicate of booting mode",
};
const char *power_off_reason[POWER_OFF_SIZE] = {
    "No Power, Enter power off or VBAT remove",
    "PWRHOLD, Enter RTC mode",
    "UVLO, Battery voltage is below UVLO ",
    "THRDN, Thermal shutdown",
    "Reserve",
    "SW CRST, SW CRST",
    "Reserve",
    "Reserve",
    "WDT CRST, WDT reset",
    "Reserve",
    "LPSD, Power key Long press shutdown",
    "PUPSRC, Power source is disappear",
    "KEYPWR, KEYPWR",
    "SYSRSTB CRST, HW Reset key ",
    "CAP LPSD, Captouch LPSD shutdown ",
    "reset pattern, vbus pattern reset ",
    "VCORE PG", "VIO18 PG", "VAUD18 PG", "VRF PG", "VPA PG", "VA18 PG", "VLDO31 PG", "VSRAM PG"
};
volatile uint8_t pmu_handle;                                            /* FLAG : PMU Sleep handle */
volatile int pmu_i2c_init_sta = 0;                                      /* FLAG : init setting flag */
int pk_first_boot_up = 0;                                               /* FLAG : PK check is turned on for the first time */
uint8_t pmu_charger_status;                                             /* FLAG : for vbus exist */
uint8_t pmu_init_flag = 0;                                              /* FLAG : check init setting is done */
uint8_t va18_flag = 0;                                                  /* FLAG : When VA18 doesn't want to be disable */
pmu_dynamic_debug_t pmu_dynamic_debug = { .b = { .vcore=0, .other=1 } };/* FLAG : default only vcore disabled */
uint8_t pmu_vrf_in_sleep_flag = 0;                                      /* FLAG : Set VRF on insleep state */
uint32_t pmu_va18_rd = 0;                                               /* FLAG : VA18 status*/
uint32_t pmu_va18_state = 0;                                            /* FLAG : VA18 implement state*/
uint32_t pmu_va18_count = 0;                                            /* FLAG : VA18 status*/
uint32_t vsram_trim = 0;
int old_index = 0;                                                      /* DATA : restore old vcore voltage index */
int pmu_switch_case = 0;                                                /* DATA : pmu lock vcore case */
int pk_next = PMU_PK_PRESS;                                             /* DATA : power key default value*/
int pmic_irq0 = -1, pmic_irq1 = -1;                                     /* DATA : restore irq status */
uint8_t audio_mode = 0;                                                 /* DATA : audio class mode */
uint8_t pmu_basic_index = 0;                                            /* DATA : project init basic vocre index */
uint8_t pmu_lock_status = 0;                                            /* DATA : lock status*/
uint8_t pmic_iv_version = 0;                                            /* DATA : PMIC IC version*/
uint8_t event_con0 = 0, event_con1 = 0, event_con2 = 0, event_con3 = 0; /* DATA : restore irq times */

uint32_t pmu_register_interrupt ;                                       /* DATA : Record irq index 0~31 */
uint32_t pmu_irq_enable_com0 = 0;                                       /* DATA : restore irq enable status,con0 */
uint32_t pmu_irq_enable_com1 = 0;                                       /* DATA : restore irq enable status,con1 */
uint32_t va18_count = 0;                                                /* DATA : VA18 check buck status times*/
pmu_function_t pmu_function_table[PMU_INT_MAX];                         /* DATA : restore callback function */
pmu_dummy_load_str dummy_loading_str;                                   /* Data : efuse struct*/
extern auxadc_efuse_data_stru gAuxadcEfuseInfo;                         /* DATA : get auxadc data struct*/
extern uint16_t pmu_cc_cur[137];
static volatile unsigned char vcore_resource_ctrl[7];                   /* resource control : use in pmu_lock_vcore for resource control */
static volatile unsigned int micbias_ldo_resource[4];                            /* resource control : use in pmu_lock_vcore for resource control */
static volatile int micbias_vout_arr[3] = {PMU_3VVREF_2P78_V, PMU_3VVREF_2P78_V, PMU_3VVREF_2P78_V};
static volatile unsigned int vaud18_vsel_arr[3] = {PMU_VAUD18_0P9_V, PMU_VAUD18_1P3_V, PMU_VAUD18_1P8_V};
uint32_t rg_chk_timer;
uint16_t addr_start, addr_end, rg_time;
#define VAUD18_DEF_SEL           0x73
#define VAUD18_DEF_VOLT          1800
#define VAUD18_VOLT_MIN          650
#define VAUD18_VOLT_MAX          1920
#ifdef __EXT_BOOTLOADER__
#include "bl_common.h"
#define pmu_dbg_printf(...) bl_print(LOG_DEBUG, __VA_ARGS__)
#endif
pmu_pwrkey_config_t pmu_pk_config={0};
/*==========[BUCK/LDO]==========*/

pmu_power_vcore_voltage_t pmu_lock_vcore_hp(pmu_power_stage_t mode, pmu_power_vcore_voltage_t vol, pmu_lock_parameter_t lock)
{
    if(pmu_init_flag==0){
       pmu_set_vsram_voltage_hp(VSRAM_VOLTAGE_0P92);
    }
    if (pmu_dynamic_debug.b.vcore) {
        log_hal_msgid_info("[PMU_PWR][PMU_LOCK_S]old_index %d, next lock %d, lock %d[0:lock 1:unlock]\r\n", 3, old_index, vol, lock);
    }
    int i = 0;
    int temp = 0;
    int vol_index = 0;
    uint32_t mask_pri;
    if (vol >= PMIC_VCORE_FAIL_V) {
        return PMIC_VCORE_FAIL_V;
    }
    hal_nvic_save_and_set_interrupt_mask_special(&mask_pri);
    if (lock == PMU_LOCK) {
        vcore_resource_ctrl[vol]++;
    } else {
        if (vcore_resource_ctrl[vol] != 0) {
            vcore_resource_ctrl[vol]--;
        }
    }
    /*Find Highest Vcore Voltage*/
    for (vol_index = PMIC_VCORE_0P85_V; vol_index >= PMIC_VCORE_0P525_V; vol_index--) {
        if (vcore_resource_ctrl[vol_index] != 0) {
            break;
        }
    }
    for (i = PMIC_VCORE_0P525_V; i <= PMIC_VCORE_0P85_V; i++) {
        temp += vcore_resource_ctrl[i];
    }
    if (temp > 25) {
    #ifndef HAL_PMU_NO_RTOS
        configASSERT(0 && "[PMU_PWR]ERROR!!!! PMU Lock /unlock times > 25");
    #endif
        assert(0);
    }
    pmu_lock_status = temp;/*if not module lock ,return default setting*/
    if (old_index < vol_index) { //rising
        if ((old_index < PMIC_VCORE_0P65_V) && (vol_index == PMIC_VCORE_0P65_V)) {
            pmu_switch_case = PMIC_RISING_0P55_0P65;
        } else if ((old_index == PMIC_VCORE_0P65_V) && (vol_index > PMIC_VCORE_0P65_V)) {
            pmu_switch_case = PMIC_RISING_0P65_0P8;
        } else {
            pmu_switch_case = PMIC_RISING_0P55_0P8;
        }
    } else if (old_index > vol_index) { //falling
        if ((old_index > PMIC_VCORE_0P65_V) && (vol_index == PMIC_VCORE_0P65_V)) {
            pmu_switch_case = PMIC_FALLING_0P8_0P65;
        } else if ((old_index == PMIC_VCORE_0P65_V) && (vol_index < PMIC_VCORE_0P65_V)) {
            pmu_switch_case = PMIC_FALLING_0P65_0P55;
        } else {
            pmu_switch_case = PMIC_FALLING_0P8_0P55;
        }
    } else {
        pmu_switch_case = PMIC_DVS_DEFAULT;
        if (pmu_dynamic_debug.b.vcore) {
            log_hal_msgid_info("[PMU_PWR]Same or higher requirements\r\n", 0);
        }
        old_index = vol_index;
        hal_nvic_restore_interrupt_mask_special(mask_pri);
        if (pmu_dynamic_debug.b.vcore) {
            log_hal_msgid_info("[PMU_PWR][PMU_LOCK_E]vcore lock %d, lock state %d, switch_case %d, lock_status %d\r\n", 4,
                               pmu_get_vcore_voltage_hp(), temp, pmu_switch_case, pmu_lock_status);
        }
        return vol_index;
    }

    switch (pmu_switch_case) {
        case PMIC_RISING_0P55_0P65:
            /*VSRAM*/
            pmu_ddie_sram_setting(PMIC_RISING_0P55_0P65, vol_index);
            /*VCORE*/
            pmu_select_vcore_voltage_hp(PMU_DVS, PMIC_VCORE_0P65_V); /*VCORE limiation , Switch to 0.65V first to 0.65V and then wait 150us */
            hal_gpt_delay_us(150);
            pmu_vcore_ipeak(vol_index);
            break;
        case PMIC_RISING_0P65_0P8:
            /*VSRAM*/
            pmu_ddie_sram_setting(PMIC_RISING_0P65_0P8, vol_index);
            /*VCORE*/
            pmu_select_vcore_voltage_hp(PMU_DVS, vol_index);
            hal_gpt_delay_us(150);
            pmu_vcore_ipeak(vol_index);
            break;
        case PMIC_RISING_0P55_0P8:
            /*VCORE*/
            pmu_select_vcore_voltage_hp(PMU_DVS, PMIC_VCORE_0P65_V); /*VCORE limiation , Switch to 0.65V first to 0.65V and then wait 150us */
            hal_gpt_delay_us(150);
            /*VSRAM*/
            pmu_ddie_sram_setting(PMIC_RISING_0P55_0P8, vol_index);
            pmu_select_vcore_voltage_hp(PMU_DVS, vol_index);
            hal_gpt_delay_us(150);
            pmu_vcore_ipeak(vol_index);
            break;
        case PMIC_FALLING_0P8_0P65:
            /*VCORE*/
            pmu_set_register_value_hp(PMU_BUCK_VCORE_CTRL1, 0x3, 0, 0x2); //Reduce VCORE waiting time
            pmu_select_vcore_voltage_hp(PMU_DVS, PMIC_VCORE_0P65_V);
            hal_gpt_delay_us(150);
            pmu_select_vcore_voltage_hp(PMU_DVS, vol_index);
            hal_gpt_delay_us(150);
            pmu_set_register_value_hp(PMU_BUCK_VCORE_CTRL1, 0x3, 0, 0x0); //Reduce VCORE waiting time
            /*VSRAM*/
            pmu_ddie_sram_setting(PMIC_FALLING_0P8_0P65, vol_index);
            pmu_vcore_ipeak(vol_index);
            break;
        case PMIC_FALLING_0P65_0P55:
            /*VCORE*/
            pmu_select_vcore_voltage_hp(PMU_DVS, vol_index);
            /*VSRAM*/
            pmu_ddie_sram_setting(PMIC_FALLING_0P65_0P55, vol_index);
            pmu_vcore_ipeak(vol_index);
            break;
        case PMIC_FALLING_0P8_0P55:
            /*VCORE*/
            pmu_set_register_value_hp(PMU_BUCK_VCORE_CTRL1, 0x3, 0, 0x2); //Reduce VCORE waiting time
            pmu_select_vcore_voltage_hp(PMU_DVS, PMIC_VCORE_0P65_V);
            hal_gpt_delay_us(150);
            /*VSRAM*/
            pmu_ddie_sram_setting(PMIC_FALLING_0P8_0P55, vol_index);
            pmu_select_vcore_voltage_hp(PMU_DVS, vol_index);
            hal_gpt_delay_us(150);
            pmu_set_register_value_hp(PMU_BUCK_VCORE_CTRL1, 0x3, 0, 0x0); //Reduce VCORE waiting time
            pmu_vcore_ipeak(vol_index);
            break;
        case PMIC_DVS_DEFAULT:
            /*VCORE*/
            pmu_set_register_value_hp(PMU_BUCK_VCORE_CTRL1, 0x3, 0, 0x2); //Reduce VCORE waiting time
            pmu_select_vcore_voltage_hp(PMU_DVS, PMIC_VCORE_0P65_V);
            hal_gpt_delay_us(150);
            /*VSRAM*/
            pmu_ddie_sram_setting(PMIC_FALLING_0P8_0P55, vol_index);
            pmu_select_vcore_voltage_hp(PMU_DVS, vol_index);
            hal_gpt_delay_us(150);
            pmu_set_register_value_hp(PMU_BUCK_VCORE_CTRL1, 0x3, 0, 0x0); //Reduce VCORE waiting time
            pmu_vcore_ipeak(vol_index);
            break;
    }
    old_index = vol_index;
    hal_nvic_restore_interrupt_mask_special(mask_pri);
    if (pmu_dynamic_debug.b.vcore) {
        log_hal_msgid_info("[PMU_PWR][PMU_LOCK_E]vcore lock %d, lock state %d, switch_case %d, lock_status %d\r\n", 4,
                           pmu_get_vcore_voltage_hp(), temp, pmu_switch_case, pmu_lock_status);
    }
    return vol_index;
}

pmu_power_vcore_voltage_t pmu_get_vcore_voltage_hp(void)
{
    uint32_t temp = 0;
    temp = pmu_get_register_value_hp(PMU_BUCK_VCORE_CTRL4, RG_BUCK_VCORE_VSEL_MASK, RG_BUCK_VCORE_VSEL_SHIFT);
    return pmu_get_vcore_setting_index(temp);
}

pmu_power_vcore_voltage_t pmu_get_vcore_setting_index(uint16_t vcore)
{
    uint8_t vcbuck_voval[6] = { 0x1e, 0x28, 0x50, 0x78, 0x84, 0x98 };
    int vosel = 0;
    for (vosel = 0; vosel < 6; vosel++) {
        if (vcore == vcbuck_voval[vosel]) {
            return ((pmu_power_vcore_voltage_t)(vosel));
        }
    }
    return (PMU_ERROR);
}
void pmu_ddie_sram_setting(pmu_vsram_dvs_state_t ste, pmu_power_vcore_voltage_t vol)
{
    if (!((ste == PMIC_RISING_0P55_0P65) || (ste == PMIC_FALLING_0P65_0P55))) {
        pmu_set_register_value_hp(PMU_LDO_VSRAM_CON1, 0x3, 2, 0x3); //Reduce VSRAM waiting time
    }
    if (vol <= PMIC_VCORE_0P65_V) {
        pmu_set_vsram_voltage_hp(VSRAM_VOLTAGE_0P8);
    } else {
        pmu_set_vsram_voltage_hp(VSRAM_VOLTAGE_0P92);
    }
    hal_gpt_delay_us(150);
    if (!((ste == PMIC_RISING_0P55_0P65) || (ste == PMIC_FALLING_0P65_0P55))) {
        pmu_set_register_value_hp(PMU_LDO_VSRAM_CON1, 0x3, 2, 0x0); //Reduce VSRAM waiting time
    }
}

void pmu_enable_power_hp(pmu_power_domain_t pmu_pdm, pmu_power_operate_t operate)
{
    switch (pmu_pdm) {
        case PMU_BUCK_VCORE:
            pmu_switch_power(PMU_BUCK_VCORE, PMU_NORMAL_MODE, operate);
            break;
        case PMU_BUCK_VIO18:
            pmu_switch_power(PMU_BUCK_VIO18, PMU_NORMAL_MODE, operate);
            break;
        case PMU_BUCK_VRF:
#ifndef PMU_WO_DCXO
            operate = PMU_ON;
#endif
            pmu_switch_power(PMU_BUCK_VRF, PMU_NORMAL_MODE, operate);
            break;
        case PMU_BUCK_VAUD18:
            pmu_switch_power(PMU_BUCK_VAUD18, PMU_NORMAL_MODE, operate);
            break;
        case PMU_BUCK_VPA:
            pmu_switch_power(PMU_BUCK_VPA, PMU_NORMAL_MODE, operate);
            break;
        case PMU_LDO_VA18:
            pmu_switch_power(PMU_LDO_VA18, PMU_NORMAL_MODE, operate);
            break;
        case PMU_LDO_VLDO31:
            pmu_switch_power(PMU_LDO_VLDO31, PMU_NORMAL_MODE, operate);
            break;
        case PMU_LDO_VSRAM:
            pmu_switch_power(PMU_LDO_VSRAM, PMU_NORMAL_MODE, operate);
            break;
    }
}
void pmu_switch_power(pmu_power_domain_t pmu_pdm, pmu_buckldo_stage_t mode, pmu_power_operate_t operate)
{
    //uint32_t mask_pri;
    uint8_t temp = 0;
    switch (pmu_pdm) {
        case PMU_BUCK_VCORE:
            if (mode == PMU_LOWPOWER_MODE) {
                pmu_set_register_value_hp(PMU_BUCK_VCORE_CTRL0, RG_BUCK_VCORE_SLP_EN_MASK, RG_BUCK_VCORE_SLP_EN_SHIFT, operate); //SLEEP
            } else {
                pmu_set_register_value_hp(PMU_BUCK_VCORE_CTRL0, RG_BUCK_VCORE_ACT_EN_MASK, RG_BUCK_VCORE_ACT_EN_SHIFT, operate); //Normal
            }
            log_hal_msgid_info("[PMU_PWR]After VCORE status :%x", 1, pmu_get_register_value_hp(PMU_BUCK_VAUD18_STATUS1, DA_VCORE_EN_MASK, DA_VCORE_EN_SHIFT)); /*VCORE address in VAUD18_status1(special case)*/
            break;
        case PMU_BUCK_VIO18:
            if (mode == PMU_LOWPOWER_MODE) {
                pmu_set_register_value_hp(PMU_BUCK_VIO18_CTRL0, RG_BUCK_VIO18_SLP_EN_MASK, RG_BUCK_VIO18_SLP_EN_SHIFT, operate); //SLEEP
            } else {
                pmu_set_register_value_hp(PMU_BUCK_VIO18_CTRL0, RG_BUCK_VIO18_ACT_EN_MASK, RG_BUCK_VIO18_ACT_EN_SHIFT, operate); //Normal
            }
            log_hal_msgid_info("[PMU_PWR]After Vio18 status :%x", 1, pmu_get_register_value_hp(PMU_BUCK_VIO18_STATUS, DA_VIO18_EN_MASK, DA_VIO18_EN_SHIFT));
            break;
        case PMU_BUCK_VRF:
            if (mode == PMU_LOWPOWER_MODE) {
                pmu_set_register_value_hp(PMU_BUCK_VRF_CTRL0, RG_BUCK_VRF_SLP_EN_MASK, RG_BUCK_VRF_SLP_EN_SHIFT, operate); //SLEEP
            } else {
                pmu_set_register_value_hp(PMU_BUCK_VRF_CTRL0, RG_BUCK_VRF_ACT_EN_MASK, RG_BUCK_VRF_ACT_EN_SHIFT, operate); //Normal
            }
            log_hal_msgid_info("[PMU_PWR]After VRF status :%x", 1, pmu_get_register_value_hp(PMU_BUCK_VRF_STATUS, DA_VRF_EN_MASK, DA_VRF_EN_SHIFT));
            break;
        case PMU_BUCK_VAUD18:
            pmu_set_register_value_hp(PMU_BUCK_VAUD18_CTRL0, RG_BUCK_VAUD18_ACT_EN_MASK, RG_BUCK_VAUD18_ACT_EN_SHIFT, operate); //Normal
            hal_gpt_delay_ms(1);
            log_hal_msgid_info("[PMU_PWR]After VAUD18 status :%x", 1, pmu_get_register_value_hp(PMU_BUCK_VAUD18_STATUS0, DA_VAUD18_EN_MASK, DA_VAUD18_EN_SHIFT));
            break;
        case PMU_BUCK_VPA:
            if (mode == PMU_LOWPOWER_MODE) {
                pmu_set_register_value_hp(PMU_BUCK_VPA_CTRL0, RG_BUCK_VPA_SLP_EN_MASK, RG_BUCK_VPA_SLP_EN_SHIFT, operate); //SLEEP
            } else {
                pmu_set_register_value_hp(PMU_BUCK_VPA_CTRL0, RG_BUCK_VPA_ACT_EN_MASK, RG_BUCK_VPA_ACT_EN_SHIFT, operate); //Normal
            }
            log_hal_msgid_info("[PMU_PWR]After VPA status :%x", 1, pmu_get_register_value_hp(PMU_BUCK_VPA_STATUS, DA_VPA_EN_MASK, DA_VPA_EN_SHIFT));
            break;

        case PMU_LDO_VA18:;
        #ifndef HAL_PMU_NO_RTOS
            int irq_table[1] = {PMU_IRQn};
            vPortDisableSchAndIrq(irq_table, 1);
        #endif
            if (operate == PMU_ON) {
                va18_count = 0;
                if (pmu_va18_count++ == 0) {
                    pmu_va18_state = PMU_POWER_TRY_TO_ENABLE;
                    pmu_set_register_value_hp(PMU_LDO_VA18_CON0, RG_SMPS_CK_SW_EN_MASK, RG_SMPS_CK_SW_EN_SHIFT, 0x1); //Normal
                    pmu_set_register_value_hp(PMU_LDO_VA18_CON0, RG_LDO_VA18_ACT_EN_MASK, RG_LDO_VA18_ACT_EN_SHIFT, 0x1); //Normal
                #ifndef HAL_PMU_NO_RTOS
                    vPortEnableSchAndIrq(irq_table, 1);
                #endif

                } else {
                #ifndef HAL_PMU_NO_RTOS
                    vPortEnableSchAndIrq(irq_table, 1);
                #endif
                }
                while (pmu_va18_state != PMU_POWER_STABLE) {
                    if (pmu_va18_state == PMU_POWER_TRY_TO_DISABLE) {
                        assert(0);
                    }
                    temp = pmu_get_register_value_hp(PMU_LDO_VA18_CON1, DA_QI_VA18_STB_MASK, DA_QI_VA18_STB_SHIFT);
                    if (temp) {
                        pmu_va18_state = PMU_POWER_STABLE;
                        va18_count = 0;
                        break;
                    }
                    pmu_set_register_value_hp(PMU_LDO_VA18_CON0, RG_SMPS_CK_SW_EN_MASK, RG_SMPS_CK_SW_EN_SHIFT, 0x1); //Normal
                    pmu_set_register_value_hp(PMU_LDO_VA18_CON0, RG_LDO_VA18_ACT_EN_MASK, RG_LDO_VA18_ACT_EN_SHIFT, 0x1); //Normal
                    hal_gpt_delay_ms(1);
                    va18_count++;
                    if (va18_count >= 15) {
                    #ifndef HAL_PMU_NO_RTOS
                        configASSERT(0 && "[PMU_PWR]Warning : VA18 Can't be enable");
                    #endif
                        assert(0);
                    }
                }
            } else {
            #ifndef HAL_PMU_NO_RTOS
                vPortEnableSchAndIrq(irq_table, 1);
            #endif
                if (!(HAL_NVIC_QUERY_EXCEPTION_NUMBER > HAL_NVIC_NOT_EXCEPTION)) {
#ifndef HAL_PMU_NO_RTOS
                    vTaskSuspendAll();
#endif
                }
            #ifndef HAL_PMU_NO_RTOS
                vPortDisableSchAndIrq(irq_table, 1);
            #endif
                --pmu_va18_count;
                if ((va18_flag == 0) && (pmu_va18_count == 0)) {
                    pmu_va18_count = 0;
                    pmu_va18_state = PMU_POWER_TRY_TO_DISABLE;
                    pmu_set_register_value_hp(PMU_LDO_VA18_CON0, RG_LDO_VA18_ACT_EN_MASK, RG_LDO_VA18_ACT_EN_SHIFT, 0x0); //Normal
                    pmu_set_register_value_hp(PMU_LDO_VA18_CON0, RG_SMPS_CK_SW_EN_MASK, RG_SMPS_CK_SW_EN_SHIFT, 0x0); //Normal
                #ifndef HAL_PMU_NO_RTOS
                    vPortEnableSchAndIrq(irq_table, 1);
                #endif
                    pmu_va18_state = PMU_POWER_STABLE;
                    if (!(HAL_NVIC_QUERY_EXCEPTION_NUMBER > HAL_NVIC_NOT_EXCEPTION)) {
#ifndef HAL_PMU_NO_RTOS

                        xTaskResumeAll();
#endif
                    }
                } else {
                #ifndef HAL_PMU_NO_RTOS
                    vPortEnableSchAndIrq(irq_table, 1);
                #endif
                    if (!(HAL_NVIC_QUERY_EXCEPTION_NUMBER > HAL_NVIC_NOT_EXCEPTION)) {
#ifndef HAL_PMU_NO_RTOS
                        xTaskResumeAll();
#endif
                    }
                }
            }
            //log_hal_msgid_info("[PMU_PWR]After VA18 status :%x", 1,pmu_get_register_value_hp(PMU_LDO_VA18_CON1,DA_QI_VA18_EN_MASK,DA_QI_VA18_EN_SHIFT));
            break;
        case PMU_LDO_VLDO31:
            if (mode == PMU_LOWPOWER_MODE) {
                pmu_set_register_value_hp(PMU_LDO_VLDO31_CON0, RG_LDO_VLDO31_SLP_EN_MASK, RG_LDO_VLDO31_SLP_EN_SHIFT, operate); //SLEEP
            } else {
                pmu_set_register_value_hp(PMU_LDO_VLDO31_CON0, RG_LDO_VLDO31_ACT_EN_MASK, RG_LDO_VLDO31_ACT_EN_SHIFT, operate); //Normal
            }
            log_hal_msgid_info("[PMU_PWR]After VLDO31 status :%x", 1, pmu_get_register_value_hp(PMU_LDO_VLDO31_CON0, DA_QI_VLDO31_EN_MASK, DA_QI_VLDO31_EN_SHIFT));
            break;
        case PMU_LDO_VSRAM:
            if (mode == PMU_LOWPOWER_MODE) {
                pmu_set_register_value_hp(PMU_LDO_VSRAM_CON0, RG_LDO_VSRAM_SLP_EN_MASK, RG_LDO_VSRAM_SLP_EN_SHIFT, operate); //SLEEP
            } else {
                pmu_set_register_value_hp(PMU_LDO_VSRAM_CON0, RG_LDO_VSRAM_ACT_EN_MASK, RG_LDO_VSRAM_ACT_EN_SHIFT, operate); //Normal
            }
            log_hal_msgid_info("[PMU_PWR]After VSRAM status :%x", 1, pmu_get_register_value_hp(PMU_LDO_VSRAM_CON0, DA_QI_VSRAM_EN_MASK, DA_QI_VSRAM_EN_SHIFT));
            break;
    }
}
uint8_t pmu_get_power_status_hp(pmu_power_domain_t pmu_pdm)
{
    uint8_t sta = 0;
    switch (pmu_pdm) {
        case PMU_BUCK_VIO18:
            sta = pmu_get_register_value_hp(PMU_BUCK_VIO18_STATUS, DA_VIO18_EN_MASK, DA_VIO18_EN_SHIFT);
            break;
        case PMU_BUCK_VRF:
            sta = pmu_get_register_value_hp(PMU_BUCK_VRF_STATUS, DA_VRF_EN_MASK, DA_VRF_EN_SHIFT);
            break;
        case PMU_BUCK_VAUD18:
            sta = pmu_get_register_value_hp(PMU_BUCK_VAUD18_STATUS0, DA_VAUD18_EN_MASK, DA_VAUD18_EN_SHIFT);
            break;
        case PMU_LDO_VA18:
            sta = pmu_get_register_value_hp(PMU_LDO_VA18_CON1, DA_QI_VA18_EN_MASK, DA_QI_VA18_EN_SHIFT);
            break;
        case PMU_LDO_VLDO31:
            sta = pmu_get_register_value_hp(PMU_LDO_VLDO31_CON1, DA_QI_VLDO31_EN_MASK, DA_QI_VLDO31_EN_SHIFT);
            break;
        case PMU_LDO_VSRAM:
            sta = pmu_get_register_value_hp(PMU_LDO_VSRAM_CON1, DA_QI_VSRAM_EN_MASK, DA_QI_VSRAM_EN_SHIFT);
            break;
    }
    log_hal_msgid_info("[PMU_PWR][BUCK:%x][%x]", 2, pmu_pdm, sta);
    return sta;
}
/*[VCORE]*/
void pmu_vcore_ipeak(int vol_index)
{
    switch (vol_index) {
        case PMIC_VCORE_0P525_V:
            pmu_set_register_value_hp(PMU_BUCK_VCORE_CTRL4, RG_BUCK_VCORE_IPEAK_VTUNE_MASK, RG_BUCK_VCORE_IPEAK_VTUNE_SHIFT, 0x1);
            break;
        case PMIC_VCORE_0P55_V:
            pmu_set_register_value_hp(PMU_BUCK_VCORE_CTRL4, RG_BUCK_VCORE_IPEAK_VTUNE_MASK, RG_BUCK_VCORE_IPEAK_VTUNE_SHIFT, 0x1);
            break;
        case PMIC_VCORE_0P65_V:
            pmu_set_register_value_hp(PMU_BUCK_VCORE_CTRL4, RG_BUCK_VCORE_IPEAK_VTUNE_MASK, RG_BUCK_VCORE_IPEAK_VTUNE_SHIFT, 0x3);
            break;
        case PMIC_VCORE_0P75_V:
            pmu_set_register_value_hp(PMU_BUCK_VCORE_CTRL4, RG_BUCK_VCORE_IPEAK_VTUNE_MASK, RG_BUCK_VCORE_IPEAK_VTUNE_SHIFT, 0x3);
            break;
        case PMIC_VCORE_0P80_V:
            pmu_set_register_value_hp(PMU_BUCK_VCORE_CTRL4, RG_BUCK_VCORE_IPEAK_VTUNE_MASK, RG_BUCK_VCORE_IPEAK_VTUNE_SHIFT, 0x4);
            break;
        case PMIC_VCORE_0P85_V:
            pmu_set_register_value_hp(PMU_BUCK_VCORE_CTRL4, RG_BUCK_VCORE_IPEAK_VTUNE_MASK, RG_BUCK_VCORE_IPEAK_VTUNE_SHIFT, 0x4);
            break;
        default:
            pmu_set_register_value_hp(PMU_BUCK_VCORE_CTRL4, RG_BUCK_VCORE_IPEAK_VTUNE_MASK, RG_BUCK_VCORE_IPEAK_VTUNE_SHIFT, 0x4);
    }
}


pmu_operate_status_t pmu_select_vcore_voltage_hp(pmu_power_stage_t mode, pmu_power_vcore_voltage_t vol)
{
    if ((mode > PMU_DVS) | (mode < PMU_SLEEP) | (vol > PMIC_VCORE_0P85_V) | (vol < PMIC_VCORE_0P525_V)) {
        log_hal_msgid_error("[PMU_PWR] vcore_voltage Error input", 0);
        return PMU_ERROR;
    }
    if ((mode != PMU_DVS) && (pmu_lock_status > 0)) {
        log_hal_msgid_error("[PMU_PWR] VCORE in locked ", 0);
        return PMU_ERROR;
    }

    switch (vol) {
        case PMIC_VCORE_0P525_V:
            pmu_set_register_value_hp(PMU_BUCK_VCORE_CTRL4, RG_BUCK_VCORE_VSEL_MASK, RG_BUCK_VCORE_VSEL_SHIFT, 0x1e);
            break;
        case PMIC_VCORE_0P55_V:
            pmu_set_register_value_hp(PMU_BUCK_VCORE_CTRL4, RG_BUCK_VCORE_VSEL_MASK, RG_BUCK_VCORE_VSEL_SHIFT, 0x28);
            break;
        case PMIC_VCORE_0P65_V:
            pmu_set_register_value_hp(PMU_BUCK_VCORE_CTRL4, RG_BUCK_VCORE_VSEL_MASK, RG_BUCK_VCORE_VSEL_SHIFT, 0x50);
            break;
        case PMIC_VCORE_0P75_V:
            pmu_set_register_value_hp(PMU_BUCK_VCORE_CTRL4, RG_BUCK_VCORE_VSEL_MASK, RG_BUCK_VCORE_VSEL_SHIFT, 0x78);
            break;
        case PMIC_VCORE_0P80_V:
            pmu_set_register_value_hp(PMU_BUCK_VCORE_CTRL4, RG_BUCK_VCORE_VSEL_MASK, RG_BUCK_VCORE_VSEL_SHIFT, 0x84);
            break;
        case PMIC_VCORE_0P85_V:
            pmu_set_register_value_hp(PMU_BUCK_VCORE_CTRL4, RG_BUCK_VCORE_VSEL_MASK, RG_BUCK_VCORE_VSEL_SHIFT, 0x98);
            break;
        default:
            pmu_set_register_value_hp(PMU_BUCK_VCORE_CTRL4, RG_BUCK_VCORE_VSEL_MASK, RG_BUCK_VCORE_VSEL_SHIFT, 0x84);
    }
    return PMU_OK;
}
/*[VAUD18]*/
void pmu_set_vaud18_voltage_hp(pmu_vaud18_vsel_t vsel, uint16_t volt)
{
	if ((volt < VAUD18_VOLT_MIN) || (volt > VAUD18_VOLT_MAX)) {
        log_hal_msgid_error("[PMU_AUDIO]set_vaud18_volt fail, volt[%d]", 1, volt);
        assert(0);
    }
    uint8_t sel = VAUD18_DEF_SEL;//rg of 1800mV
    if (volt > VAUD18_DEF_VOLT) {
        sel += ((volt - VAUD18_DEF_VOLT) / 10);
    } else {
        sel -= ((VAUD18_DEF_VOLT - volt) / 10);
    }
	log_hal_msgid_info("[PMU_AUDIO]vsel[%d], volt[%d]", 2,vsel, volt);
    if (vsel == PMU_VAUD18_VSEL_L) {
        pmu_set_register_value_hp(PMU_BUCK_VAUD18_CTRL4, RG_BUCK_VAUD18_VSEL_L_MASK, RG_BUCK_VAUD18_VSEL_L_SHIFT, sel);
    } else if (vsel == PMU_VAUD18_VSEL_M) {
        pmu_set_register_value_hp(PMU_BUCK_VAUD18_CTRL3, RG_BUCK_VAUD18_VSEL_M_MASK, RG_BUCK_VAUD18_VSEL_M_SHIFT, sel);
    } else if (vsel == PMU_VAUD18_VSEL_H) {
		pmu_set_register_value_hp(PMU_BUCK_VAUD18_CTRL2, RG_BUCK_VAUD18_VSEL_H_MASK, RG_BUCK_VAUD18_VSEL_H_SHIFT, sel);
	}else {
        log_hal_msgid_error("[PMU_AUDIO]set_vaud18_volt fail, vsel[%d]", 1, vsel);
        assert(0);
    }
}
/*[VPA]*/
void pmu_set_vpa_voltage_hp(pmu_power_vpa_voltage_t oper)
{
    switch (oper) {
        case PMU_VPA_1p2V:
            pmu_set_register_value_hp(PMU_BUCK_VPA_CTRL4, RG_BUCK_VPA_VOUTSEL_MASK, RG_BUCK_VPA_VOUTSEL_SHIFT, 0x4); //Normal
            break;
        case PMU_VPA_1p4V:
            pmu_set_register_value_hp(PMU_BUCK_VPA_CTRL4, RG_BUCK_VPA_VOUTSEL_MASK, RG_BUCK_VPA_VOUTSEL_SHIFT, 0x6); //Normal
            break;
        case PMU_VPA_1p6V:
            pmu_set_register_value_hp(PMU_BUCK_VPA_CTRL4, RG_BUCK_VPA_VOUTSEL_MASK, RG_BUCK_VPA_VOUTSEL_SHIFT, 0x8); //Normal
            break;
        case PMU_VPA_1p8V:
            pmu_set_register_value_hp(PMU_BUCK_VPA_CTRL4, RG_BUCK_VPA_VOUTSEL_MASK, RG_BUCK_VPA_VOUTSEL_SHIFT, 0xA); //Normal
            break;
        case PMU_VPA_2p2V:
            pmu_set_register_value_hp(PMU_BUCK_VPA_CTRL4, RG_BUCK_VPA_VOUTSEL_MASK, RG_BUCK_VPA_VOUTSEL_SHIFT, 0xE); //Normal
            break;
    }
    if (oper > PMU_VPA_1p8V) {
        pmu_set_register_value_hp(PMU_BUCK_VPA_CTRL4, RG_BUCK_VPA_IPEAK_VTUNE_MASK, RG_BUCK_VPA_IPEAK_VTUNE_SHIFT, 0x3); //Normal
    } else {
        pmu_set_register_value_hp(PMU_BUCK_VPA_CTRL4, RG_BUCK_VPA_IPEAK_VTUNE_MASK, RG_BUCK_VPA_IPEAK_VTUNE_SHIFT, 0x2); //Normal
    }
    log_hal_msgid_info("[PMU_PWR][VPA VOUTSEL :%x][%x]", 2, oper,
                       pmu_get_register_value_hp(PMU_BUCK_VPA_CTRL4, RG_BUCK_VPA_VOUTSEL_MASK, RG_BUCK_VPA_VOUTSEL_SHIFT));
}

void pmu_lock_va18_hp(int oper)
{
#ifndef HAL_PMU_NO_RTOS
    int irq_table[1] = {PMU_IRQn};
    vPortDisableSchAndIrq(irq_table, 1);
#endif
    if (oper) {
        if(va18_flag++==0){
#ifdef HAL_SLEEP_MANAGER_ENABLED
           hal_sleep_manager_lock_sleep(pmu_handle);
#endif
        }
    #ifndef HAL_PMU_NO_RTOS
        vPortEnableSchAndIrq(irq_table, 1);
    #endif
    } else {
        if (va18_flag != 0) {
            va18_flag--;
        }
    #ifndef HAL_PMU_NO_RTOS
        vPortEnableSchAndIrq(irq_table, 1);
    #endif
        if(va18_flag==0){
#ifdef HAL_SLEEP_MANAGER_ENABLED
           hal_sleep_manager_unlock_sleep(pmu_handle);
#endif
        }
    }
}
/*[VRF]*/
void pmu_vrf_keep_nm_hp(void)
{
    pmu_set_register_value_hp(PMU_BUCK_VRF_CTRL0, RG_BUCK_VRF_SLP_LP_MASK, RG_BUCK_VRF_SLP_LP_SHIFT, 0x0);
    pmu_set_register_value_hp(PMU_BUCK_VRF_CTRL0, RG_BUCK_VRF_SLP_EN_MASK, RG_BUCK_VRF_SLP_EN_SHIFT, 0x1);
}
/*[VSRAM]*/
void pmu_set_vsram_voltage_hp(pmu_vsram_voltage_t val)
{
    pmu_set_register_value_hp(PMU_LDO_SRAM_CON3, RG_VSRAM_VOSEL_MASK, RG_VSRAM_VOSEL_SHIFT, val);
}
uint32_t pmu_get_vsram_voltage_hp(void)
{
    return pmu_get_register_value_hp(PMU_LDO_SRAM_CON3, RG_VSRAM_VOSEL_MASK, RG_VSRAM_VOSEL_SHIFT);
}
/*==========[Audio/MICBIAS]==========*/
pmu_operate_status_t pmu_set_micbias_vout_hp(pmu_micbias_index_t index, pmu_3vvref_voltage_t vol)
{
    if ((index >= PMIC_MICBIAS0) && (index <= PMIC_MICBIAS2) && (vol >= PMU_3VVREF_1P8_V) && (vol <= PMU_3VVREF_2P5_V)) {
        micbias_vout_arr[index] = vol;
        log_hal_msgid_info("[PMU_AUDIO]micbias_vout_arr[%d][%d][%d]", 3, micbias_vout_arr[0], micbias_vout_arr[1], micbias_vout_arr[2]);
        return PMU_OK;
    } else {
        log_hal_msgid_info("[PMU_AUDIO]micbias vol input error", 0);
        return PMU_INVALID;
    }
}

pmu_operate_status_t pmu_set_vaud18_vout_hp(pmu_vaud18_voltage_t lv, pmu_vaud18_voltage_t mv, pmu_vaud18_voltage_t hv)
{
    log_hal_info("[PMU_AUDIO]VAUD18 vol before[%d][%d][%d]", vaud18_vsel_arr[0], vaud18_vsel_arr[1], vaud18_vsel_arr[2]);
    vaud18_vsel_arr[0] = lv;
    vaud18_vsel_arr[1] = mv;
    vaud18_vsel_arr[2] = hv;
    log_hal_info("[PMU_AUDIO]VAUD18 vol after[%d][%d][%d]", vaud18_vsel_arr[0], vaud18_vsel_arr[1], vaud18_vsel_arr[2]);
    return PMU_OK;
}

void pmu_enable_micbias_inrush(pmu_micbias_index_t index, uint8_t operate)
{
    switch (index) {
        case PMIC_MICBIAS0:
            pmu_set_register_value_hp(PMU_MICBIAS0_CON, RG_AUDPWDBMICBIAS0_MASK, RG_AUDPWDBMICBIAS0_SHIFT, operate);
            break;
        case PMIC_MICBIAS1:
            pmu_set_register_value_hp(PMU_MICBIAS1_CON, RG_AUDPWDBMICBIAS1_MASK, RG_AUDPWDBMICBIAS1_SHIFT, operate);
            break;
        case PMIC_MICBIAS2:
            pmu_set_register_value_hp(PMU_MICBIAS2_CON, RG_AUDPWDBMICBIAS2_MASK, RG_AUDPWDBMICBIAS2_SHIFT, operate);
            break;
    }
    hal_gpt_delay_us(50);
}

void pmu_set_micbias_ldo_vout(pmu_micbias_index_t index)
{
    switch (index) {
        case PMIC_MICBIAS0:
            pmu_set_register_value_hp(PMU_MICBIAS0_CON, RG_AUDMICBIAS0_3VVREF_MASK, RG_AUDMICBIAS0_3VVREF_SHIFT, micbias_vout_arr[0]);
            break;
        case PMIC_MICBIAS1:
            pmu_set_register_value_hp(PMU_MICBIAS1_CON, RG_AUDMICBIAS1_3VVREF_MASK, RG_AUDMICBIAS1_3VVREF_SHIFT, micbias_vout_arr[1]);
            break;
        case PMIC_MICBIAS2:
            pmu_set_register_value_hp(PMU_MICBIAS2_CON, RG_AUDMICBIAS2_3VVREF_MASK, RG_AUDMICBIAS2_3VVREF_SHIFT, micbias_vout_arr[2]);
            break;
    }
    hal_gpt_delay_us(50);
}
void pmu_set_micbias_pulllow(pmu_micbias_index_t index, pmu_power_operate_t oper)
{
    switch (index) {
        case PMIC_MICBIAS0:
            pmu_set_register_value_hp(PMU_MICBIAS0_CON, RG_AUDACCDETMICBIAS0_PULLLOW_MASK, RG_AUDACCDETMICBIAS0_PULLLOW_SHIFT, oper);
            break;
        case PMIC_MICBIAS1:
            pmu_set_register_value_hp(PMU_MICBIAS1_CON, RG_AUDACCDETMICBIAS1_PULLLOW_MASK, RG_AUDACCDETMICBIAS1_PULLLOW_SHIFT, oper);
            break;
        case PMIC_MICBIAS2:
            pmu_set_register_value_hp(PMU_MICBIAS2_CON, RG_AUDACCDETMICBIAS2_PULLLOW_MASK, RG_AUDACCDETMICBIAS2_PULLLOW_SHIFT, oper);
            break;
    }
}
void pmu_enable_micbias_ldo(pmu_micbias_ldo_t ldo, pmu_micbias_mode_t mode)
{
    uint8_t ldo_nm = 1;
    if (mode != PMIC_MICBIAS_LP) {
        ldo_nm = 0;
    }
    switch (ldo) {
        case PMIC_MICBIAS_LDO0:
            pmu_set_register_value_hp(PMU_MICBIAS0_CON, RG_AUDPWDBMICBIAS0_3VEN_MASK, RG_AUDPWDBMICBIAS0_3VEN_SHIFT, 0x1);
            pmu_set_register_value_hp(PMU_MICBIAS0_CON, RG_AUDMICBIAS0_3VBYPASSEN_MASK, RG_AUDMICBIAS0_3VBYPASSEN_SHIFT, 0x0);
            pmu_set_register_value_hp(PMU_MICBIAS0_CON, RG_AUDMICBIAS0_3VLOWPEN_MASK, RG_AUDMICBIAS0_3VLOWPEN_SHIFT, ldo_nm);
            if ((micbias_vout_arr[0] == PMU_3VVREF_2P78_V) || (micbias_vout_arr[0]) > PMU_3VVREF_2P0_V) {
                pmu_set_register_value_hp(PMU_MICBIAS0_CON, RG_AUDMICBIAS0_3VVREF_MASK, RG_AUDMICBIAS0_3VVREF_SHIFT, PMU_3VVREF_2P0_V);
            } else {
                pmu_set_register_value_hp(PMU_MICBIAS0_CON, RG_AUDMICBIAS0_3VVREF_MASK, RG_AUDMICBIAS0_3VVREF_SHIFT, PMU_3VVREF_1P8_V);
            }
            micbias_ldo_resource[PMIC_MICBIAS_LDO0]++;
            break;
        case PMIC_MICBIAS_LDO1:
            pmu_set_register_value_hp(PMU_MICBIAS1_CON, RG_AUDPWDBMICBIAS1_3VEN_MASK, RG_AUDPWDBMICBIAS1_3VEN_SHIFT, 0x1);
            pmu_set_register_value_hp(PMU_MICBIAS1_CON, RG_AUDMICBIAS1_3VBYPASSEN_MASK, RG_AUDMICBIAS1_3VBYPASSEN_SHIFT, 0x0);
            pmu_set_register_value_hp(PMU_MICBIAS1_CON, RG_AUDMICBIAS1_3VLOWPEN_MASK, RG_AUDMICBIAS1_3VLOWPEN_SHIFT, ldo_nm);
            if ((micbias_vout_arr[1] == PMU_3VVREF_2P78_V) || (micbias_vout_arr[1]) > PMU_3VVREF_2P0_V) {
                pmu_set_register_value_hp(PMU_MICBIAS1_CON, RG_AUDMICBIAS1_3VVREF_MASK, RG_AUDMICBIAS1_3VVREF_SHIFT, PMU_3VVREF_2P0_V);
            } else {
                pmu_set_register_value_hp(PMU_MICBIAS1_CON, RG_AUDMICBIAS1_3VVREF_MASK, RG_AUDMICBIAS1_3VVREF_SHIFT, PMU_3VVREF_1P8_V);
            }
            micbias_ldo_resource[PMIC_MICBIAS_LDO1]++;
            break;
        case PMIC_MICBIAS_LDO2:
            pmu_set_register_value_hp(PMU_MICBIAS2_CON, RG_AUDPWDBMICBIAS2_3VEN_MASK, RG_AUDPWDBMICBIAS2_3VEN_SHIFT, 0x1);
            pmu_set_register_value_hp(PMU_MICBIAS2_CON, RG_AUDMICBIAS2_3VBYPASSEN_MASK, RG_AUDMICBIAS2_3VBYPASSEN_SHIFT, 0x0);
            pmu_set_register_value_hp(PMU_MICBIAS2_CON, RG_AUDMICBIAS2_3VLOWPEN_MASK, RG_AUDMICBIAS2_3VLOWPEN_SHIFT, ldo_nm);
            if ((micbias_vout_arr[2] == PMU_3VVREF_2P78_V) || (micbias_vout_arr[2]) > PMU_3VVREF_2P0_V) {
                pmu_set_register_value_hp(PMU_MICBIAS2_CON, RG_AUDMICBIAS2_3VVREF_MASK, RG_AUDMICBIAS2_3VVREF_SHIFT, PMU_3VVREF_2P0_V);
            } else {
                pmu_set_register_value_hp(PMU_MICBIAS2_CON, RG_AUDMICBIAS2_3VVREF_MASK, RG_AUDMICBIAS2_3VVREF_SHIFT, PMU_3VVREF_1P8_V);
            }
            micbias_ldo_resource[PMIC_MICBIAS_LDO2]++;
            break;
    }
    hal_gpt_delay_us(50);
}
void pmu_enable_micbias_mode(pmu_micbias_ldo_t ldo, pmu_micbias_mode_t mode)
{
    switch (ldo) {
        case PMIC_MICBIAS_LDO0:
            if (mode == PMIC_MICBIAS_HPM) {
                pmu_set_register_value_hp(PMU_MICBIAS0_CON, RG_MBIAS0_HPM_MASK, RG_MBIAS0_HPM_SHIFT, 0x1);
            } else {
                pmu_set_register_value_hp(PMU_MICBIAS0_CON, RG_MBIAS0_HPM_MASK, RG_MBIAS0_HPM_SHIFT, 0x0);
            }
            break;
        case PMIC_MICBIAS_LDO1:
            if (mode == PMIC_MICBIAS_HPM) {
                pmu_set_register_value_hp(PMU_MICBIAS1_CON, RG_MBIAS1_HPM_MASK, RG_MBIAS1_HPM_SHIFT, 0x1);
            } else {
                pmu_set_register_value_hp(PMU_MICBIAS1_CON, RG_MBIAS1_HPM_MASK, RG_MBIAS1_HPM_SHIFT, 0x0);
            }
            break;
        case PMIC_MICBIAS_LDO2:
            if (mode == PMIC_MICBIAS_HPM) {
                pmu_set_register_value_hp(PMU_MICBIAS2_CON, RG_MBIAS2_HPM_MASK, RG_MBIAS2_HPM_SHIFT, 0x1);
            } else {
                pmu_set_register_value_hp(PMU_MICBIAS2_CON, RG_MBIAS2_HPM_MASK, RG_MBIAS2_HPM_SHIFT, 0x0);
            }
            break;
    }
}

void pmu_set_micbias_ldo_mode_hp(pmu_micbias_ldo_t ldo, pmu_micbias_mode_t mode)
{
    log_hal_msgid_info("[PMU_AUDIO] LDO%d, mode%d",2, ldo, mode);
    switch (ldo) {
        case PMIC_MICBIAS_LDO0:
            if (mode == PMIC_MICBIAS_HPM) {
                pmu_set_register_value_hp(PMU_MICBIAS0_CON, RG_AUDMICBIAS0_3VLOWPEN_MASK, RG_AUDMICBIAS0_3VLOWPEN_SHIFT, 0x0);
                pmu_set_register_value_hp(PMU_MICBIAS0_CON, RG_MBIAS0_HPM_MASK, RG_MBIAS0_HPM_SHIFT, 0x1);
            } else if (mode == PMIC_MICBIAS_LP) {
                pmu_set_register_value_hp(PMU_MICBIAS0_CON, RG_MBIAS0_HPM_MASK, RG_MBIAS0_HPM_SHIFT, 0x0);
                pmu_set_register_value_hp(PMU_MICBIAS0_CON, RG_AUDMICBIAS0_3VLOWPEN_MASK, RG_AUDMICBIAS0_3VLOWPEN_SHIFT, 0x1);
            } else {
                pmu_set_register_value_hp(PMU_MICBIAS0_CON, RG_MBIAS0_HPM_MASK, RG_MBIAS0_HPM_SHIFT, 0x0);
                pmu_set_register_value_hp(PMU_MICBIAS0_CON, RG_AUDMICBIAS0_3VLOWPEN_MASK, RG_AUDMICBIAS0_3VLOWPEN_SHIFT, 0x0);
            }
            break;
        case PMIC_MICBIAS_LDO1:
            if (mode == PMIC_MICBIAS_HPM) {
                pmu_set_register_value_hp(PMU_MICBIAS1_CON, RG_AUDMICBIAS1_3VLOWPEN_MASK, RG_AUDMICBIAS1_3VLOWPEN_SHIFT, 0x0);
                pmu_set_register_value_hp(PMU_MICBIAS1_CON, RG_MBIAS1_HPM_MASK, RG_MBIAS1_HPM_SHIFT, 0x1);
            } else if (mode == PMIC_MICBIAS_LP) {
                pmu_set_register_value_hp(PMU_MICBIAS1_CON, RG_MBIAS1_HPM_MASK, RG_MBIAS1_HPM_SHIFT, 0x0);
                pmu_set_register_value_hp(PMU_MICBIAS1_CON, RG_AUDMICBIAS1_3VLOWPEN_MASK, RG_AUDMICBIAS1_3VLOWPEN_SHIFT, 0x1);
            } else {
                pmu_set_register_value_hp(PMU_MICBIAS1_CON, RG_MBIAS1_HPM_MASK, RG_MBIAS1_HPM_SHIFT, 0x0);
                pmu_set_register_value_hp(PMU_MICBIAS1_CON, RG_AUDMICBIAS1_3VLOWPEN_MASK, RG_AUDMICBIAS1_3VLOWPEN_SHIFT, 0x0);
            }
            break;
        case PMIC_MICBIAS_LDO2:
            if (mode == PMIC_MICBIAS_HPM) {
                pmu_set_register_value_hp(PMU_MICBIAS2_CON, RG_AUDMICBIAS2_3VLOWPEN_MASK, RG_AUDMICBIAS2_3VLOWPEN_SHIFT, 0x0);
                pmu_set_register_value_hp(PMU_MICBIAS2_CON, RG_MBIAS2_HPM_MASK, RG_MBIAS2_HPM_SHIFT, 0x1);
            } else if (mode == PMIC_MICBIAS_LP) {
                pmu_set_register_value_hp(PMU_MICBIAS2_CON, RG_MBIAS2_HPM_MASK, RG_MBIAS2_HPM_SHIFT, 0x0);
                pmu_set_register_value_hp(PMU_MICBIAS2_CON, RG_AUDMICBIAS2_3VLOWPEN_MASK, RG_AUDMICBIAS2_3VLOWPEN_SHIFT, 0x1);
            } else {
                pmu_set_register_value_hp(PMU_MICBIAS2_CON, RG_MBIAS2_HPM_MASK, RG_MBIAS2_HPM_SHIFT, 0x0);
                pmu_set_register_value_hp(PMU_MICBIAS2_CON, RG_AUDMICBIAS2_3VLOWPEN_MASK, RG_AUDMICBIAS2_3VLOWPEN_SHIFT, 0x0);
            }
            break;
    }
}
void pmu_micbias_resource_ctrl(pmu_micbias_ldo_t ldo)
{
    if (micbias_ldo_resource[ldo] > 0) {
        micbias_ldo_resource[ldo]--;
    }
    if (micbias_ldo_resource[ldo] == 0) {
        switch (ldo) {
            case PMIC_MICBIAS_LDO0:
                pmu_set_register_value_hp(PMU_MICBIAS_MISC, 0xffff, 0, 0x0);
                pmu_enable_micbias_inrush(ldo, PMU_OFF);
                pmu_set_register_value_hp(PMU_MICBIAS0_CON, RG_AUDPWDBMICBIAS0_3VEN_MASK, RG_AUDPWDBMICBIAS0_3VEN_SHIFT, 0x0);
                pmu_set_register_value_hp(PMU_MICBIAS0_CON, RG_AUDMICBIAS0_3VLOWPEN_MASK, RG_AUDMICBIAS0_3VLOWPEN_SHIFT, 0x0);
                break;
            case PMIC_MICBIAS_LDO1:
                pmu_enable_micbias_inrush(ldo, PMU_OFF);
                pmu_set_register_value_hp(PMU_MICBIAS1_CON, RG_AUDPWDBMICBIAS1_3VEN_MASK, RG_AUDPWDBMICBIAS1_3VEN_SHIFT, 0x0);
                pmu_set_register_value_hp(PMU_MICBIAS1_CON, RG_AUDMICBIAS1_3VLOWPEN_MASK, RG_AUDMICBIAS1_3VLOWPEN_SHIFT, 0x0);
                break;
            case PMIC_MICBIAS_LDO2:
                pmu_enable_micbias_inrush(ldo, PMU_OFF);
                pmu_set_register_value_hp(PMU_MICBIAS2_CON, RG_AUDPWDBMICBIAS2_3VEN_MASK, RG_AUDPWDBMICBIAS2_3VEN_SHIFT, 0x0);
                pmu_set_register_value_hp(PMU_MICBIAS2_CON, RG_AUDMICBIAS2_3VLOWPEN_MASK, RG_AUDMICBIAS2_3VLOWPEN_SHIFT, 0x0);
                break;
        }
    }
}
/* exampel :
 *  pmu_enable_micbias_hp(PMIC_MICBIAS_LDO0,PMIC_MICBIAS0,PMIC_MICBIAS_NM,PMU_ON);
 *  pmu_enable_micbias_hp(PMIC_MICBIAS_LDO1,PMIC_MICBIAS1,PMIC_MICBIAS_NM,PMU_ON);
 *  pmu_enable_micbias_hp(PMIC_MICBIAS_LDO2,PMIC_MICBIAS2,PMIC_MICBIAS_NM,PMU_ON);
 */
void pmu_enable_micbias_hp(pmu_micbias_ldo_t ldo, pmu_micbias_index_t index, pmu_micbias_mode_t mode, pmu_power_operate_t operate)
{
    log_hal_msgid_info("[PMU_AUDIO] ldo = %d, micbias = %d mode = %d operate =%d \r\n", 4, ldo, index, mode, operate);
    pmu_set_register_value_hp(PMU_MICBIAS_MISC, RG_MBIAS_VIO18_MASK, RG_MBIAS_VIO18_SHIFT, 0x0);
    if (operate) {
        if (index == PMIC_MICBIAS_ALL) {
            pmu_set_micbias_pulllow(PMIC_MICBIAS_LDO0, PMU_OFF);
            pmu_set_register_value_hp(PMU_MICBIAS1_CON, 0xffff, 0, 0x0);
            pmu_set_register_value_hp(PMU_MICBIAS2_CON, 0xffff, 0, 0x0);
            pmu_enable_micbias_ldo(PMIC_MICBIAS_LDO0, mode);
            pmu_enable_micbias_inrush(PMIC_MICBIAS_LDO0, 0x2);
            pmu_set_register_value_hp(PMU_MICBIAS_MISC, RG_AUDPWDBMICBIAS0_1_MASK, RG_AUDPWDBMICBIAS0_1_SHIFT, 0x2);
            pmu_set_register_value_hp(PMU_MICBIAS_MISC, RG_AUDPWDBMICBIAS0_2_MASK, RG_AUDPWDBMICBIAS0_2_SHIFT, 0x2);
            pmu_set_micbias_ldo_vout(ldo);
            pmu_enable_micbias_inrush(PMIC_MICBIAS_LDO0, 0x1);
            pmu_set_register_value_hp(PMU_MICBIAS_MISC, RG_AUDPWDBMICBIAS0_1_MASK, RG_AUDPWDBMICBIAS0_1_SHIFT, 0x1);
            pmu_set_register_value_hp(PMU_MICBIAS_MISC, RG_AUDPWDBMICBIAS0_2_MASK, RG_AUDPWDBMICBIAS0_2_SHIFT, 0x1);
            pmu_set_micbias_pulllow(PMIC_MICBIAS_LDO0, PMU_ON);
            pmu_set_micbias_pulllow(PMIC_MICBIAS_LDO1, PMU_ON);
            pmu_set_micbias_pulllow(PMIC_MICBIAS_LDO2, PMU_ON);
            pmu_enable_micbias_mode(ldo, mode);
        } else if (index == PMIC_MICBIAS_0_1) {
            pmu_set_micbias_pulllow(PMIC_MICBIAS_LDO0, PMU_OFF);
            pmu_set_register_value_hp(PMU_MICBIAS1_CON, 0xffff, 0, 0x0);
            pmu_enable_micbias_ldo(PMIC_MICBIAS_LDO0, mode);
            pmu_enable_micbias_inrush(PMIC_MICBIAS_LDO0, 0x2);
            pmu_set_register_value_hp(PMU_MICBIAS_MISC, RG_AUDPWDBMICBIAS0_1_MASK, RG_AUDPWDBMICBIAS0_1_SHIFT, 0x2);
            pmu_set_micbias_ldo_vout(ldo);
            pmu_enable_micbias_inrush(PMIC_MICBIAS_LDO0, 0x1);
            pmu_set_register_value_hp(PMU_MICBIAS_MISC, RG_AUDPWDBMICBIAS0_1_MASK, RG_AUDPWDBMICBIAS0_1_SHIFT, 0x1);
            pmu_set_micbias_pulllow(PMIC_MICBIAS_LDO0, PMU_ON);
            pmu_set_micbias_pulllow(PMIC_MICBIAS_LDO1, PMU_ON);
            pmu_enable_micbias_mode(ldo, mode);
        } else if (index == PMIC_MICBIAS0_SHARE_ENABLE) {
            pmu_set_micbias_pulllow(PMIC_MICBIAS_LDO0, PMU_OFF);
            pmu_set_register_value_hp(PMU_MICBIAS1_CON, 0xffff, 0, 0x0);
            pmu_set_register_value_hp(PMU_MICBIAS2_CON, 0xffff, 0, 0x0);
            pmu_enable_micbias_ldo(PMIC_MICBIAS_LDO0, mode);
            pmu_enable_micbias_inrush(PMIC_MICBIAS_LDO0, 0x2);
            //pmu_set_register_value_hp(PMU_MICBIAS_MISC, RG_AUDPWDBMICBIAS0_1_MASK, RG_AUDPWDBMICBIAS0_1_SHIFT, 0x2);
            //pmu_set_register_value_hp(PMU_MICBIAS_MISC, RG_AUDPWDBMICBIAS0_2_MASK, RG_AUDPWDBMICBIAS0_2_SHIFT, 0x2);
            pmu_set_micbias_ldo_vout(ldo);
            pmu_enable_micbias_inrush(PMIC_MICBIAS_LDO0, 0x1);
            //pmu_set_register_value_hp(PMU_MICBIAS_MISC, RG_AUDPWDBMICBIAS0_1_MASK, RG_AUDPWDBMICBIAS0_1_SHIFT, 0x1);
            //pmu_set_register_value_hp(PMU_MICBIAS_MISC, RG_AUDPWDBMICBIAS0_2_MASK, RG_AUDPWDBMICBIAS0_2_SHIFT, 0x1);
            pmu_set_micbias_pulllow(PMIC_MICBIAS_LDO0, PMU_ON);
            pmu_set_micbias_pulllow(PMIC_MICBIAS_LDO1, PMU_ON);
            pmu_set_micbias_pulllow(PMIC_MICBIAS_LDO2, PMU_ON);
            pmu_enable_micbias_mode(ldo, mode);
        } else if (index == PMIC_MICBIAS0_SHARE_1) {
            pmu_set_register_value_hp(PMU_MICBIAS_MISC, RG_AUDPWDBMICBIAS0_1_MASK, RG_AUDPWDBMICBIAS0_1_SHIFT, 0x1);
        } else if (index == PMIC_MICBIAS0_SHARE_2) {
            pmu_set_register_value_hp(PMU_MICBIAS_MISC, RG_AUDPWDBMICBIAS0_2_MASK, RG_AUDPWDBMICBIAS0_2_SHIFT, 0x1);
        } else {
            pmu_set_micbias_pulllow(ldo, PMU_OFF);
            pmu_enable_micbias_ldo(ldo, mode);
            pmu_enable_micbias_inrush(index, 0x2);
            pmu_set_micbias_ldo_vout(ldo);
            pmu_enable_micbias_inrush(index, 0x1);
            pmu_set_micbias_pulllow(ldo, PMU_ON);
            pmu_enable_micbias_mode(ldo, mode);
        }
    } else {
        if (index == PMIC_MICBIAS0_SHARE_1) {
            pmu_set_register_value_hp(PMU_MICBIAS_MISC, RG_AUDPWDBMICBIAS0_1_MASK, RG_AUDPWDBMICBIAS0_1_SHIFT, 0x0);
        } else if (index == PMIC_MICBIAS0_SHARE_2) {
            pmu_set_register_value_hp(PMU_MICBIAS_MISC, RG_AUDPWDBMICBIAS0_2_MASK, RG_AUDPWDBMICBIAS0_2_SHIFT, 0x0);
        } else {
            pmu_micbias_resource_ctrl(ldo);
        }
    }
    log_hal_msgid_info("[PMU_AUDIO] MICBIAS_MISC = %x MICBIAS0_CON = %x, MICBIAS1_CON = %x MICBIAS2_CON = %x ", 4,
                       pmu_get_register_value_hp(PMU_MICBIAS_MISC, 0xffff, 0),
                       pmu_get_register_value_hp(PMU_MICBIAS0_CON, 0xffff, 0),
                       pmu_get_register_value_hp(PMU_MICBIAS1_CON, 0xffff, 0),
                       pmu_get_register_value_hp(PMU_MICBIAS2_CON, 0xffff, 0));
}
void pmu_set_audio_mode_hp(pmu_audio_mode_t mode, pmu_power_operate_t operate)
{
    if (operate) {
        pmu_set_audio_mode_init(mode);
#ifndef AIR_AUDIO_EXT_DAC_ENABLE
        pmu_enable_power_hp(PMU_BUCK_VAUD18, PMU_ON);
        do {
        } while (pmu_get_register_value_hp(PMU_BUCK_VAUD18_STATUS0, DA_VAUD18_SSH_MASK, DA_VAUD18_SSH_SHIFT) != 1);
#endif
        pmu_set_register_value_hp(PMU_VAUD18DRV_MODE, RG_VAUD18DRV_VOSEL0_HW_MODE_MASK, RG_VAUD18DRV_VOSEL0_HW_MODE_SHIFT, 0x1);
    } else {
        pmu_set_register_value_hp(PMU_VAUD18DRV_MODE, RG_VAUD18DRV_VOSEL0_HW_MODE_MASK, RG_VAUD18DRV_VOSEL0_HW_MODE_SHIFT, 0x0);
        pmu_set_register_value_hp(PMU_VAUD18DRV_MODE, RG_VAUD18DRV_VOSEL0_SW_MASK, RG_VAUD18DRV_VOSEL0_SW_SHIFT, 0x1);
        pmu_enable_power_hp(PMU_BUCK_VAUD18, PMU_OFF);
    }

}

void pmu_set_audio_mode_init(pmu_audio_mode_t mode)
{
    audio_mode = mode;
    pmu_set_register_value_hp(PMU_VAUD18DRV_MODE, RG_VAUD18DRV_VOSEL0_HW_MODE_MASK, RG_VAUD18DRV_VOSEL0_HW_MODE_SHIFT, 0x0);
    pmu_set_register_value_hp(PMU_VAUD18DRV_MODE, RG_VAUD18DRV_VOSEL0_SW_MASK, RG_VAUD18DRV_VOSEL0_SW_SHIFT, 0x1);
    pmu_set_register_value_hp(PMU_BUCK_VAUD18_CTRL5, RG_AUDIO_MODE_MASK, RG_AUDIO_MODE_SHIFT, mode);
    switch (mode) {
        case PMU_CLASSAB:
            pmu_set_register_value_hp(PMU_BUCK_VAUD18_CTRL2, RG_BUCK_VAUD18_IPEAK_VTUNE_H_MASK, RG_BUCK_VAUD18_IPEAK_VTUNE_H_SHIFT, 0x4);
            pmu_set_vaud18_voltage_hp(PMU_VAUD18_VSEL_H, vaud18_vsel_arr[2]);
            log_hal_msgid_info("[PMU_AUDIO] Audio mode = CLASS AB \r\n", 0);
            break;
        case PMU_CLASSG2:
            pmu_set_register_value_hp(PMU_BUCK_VAUD18_CTRL2, RG_BUCK_VAUD18_IPEAK_VTUNE_H_MASK, RG_BUCK_VAUD18_IPEAK_VTUNE_H_SHIFT, 0x4);
            pmu_set_vaud18_voltage_hp(PMU_VAUD18_VSEL_L, vaud18_vsel_arr[0]);
            pmu_set_vaud18_voltage_hp(PMU_VAUD18_VSEL_H, vaud18_vsel_arr[2]);
            log_hal_msgid_info("[PMU_AUDIO] Audio mode = CLASS G2 \r\n", 0);
            break;
        case PMU_CLASSG3:
            pmu_set_register_value_hp(PMU_BUCK_VAUD18_CTRL2, RG_BUCK_VAUD18_IPEAK_VTUNE_H_MASK, RG_BUCK_VAUD18_IPEAK_VTUNE_H_SHIFT, 0x4);
            pmu_set_vaud18_voltage_hp(PMU_VAUD18_VSEL_L, vaud18_vsel_arr[0]);
            pmu_set_vaud18_voltage_hp(PMU_VAUD18_VSEL_M, vaud18_vsel_arr[1]);
            pmu_set_vaud18_voltage_hp(PMU_VAUD18_VSEL_H, vaud18_vsel_arr[2]);
            log_hal_msgid_info("[PMU_AUDIO] Audio mode = CLASS G3 \r\n", 0);
            break;
        case PMU_CLASSD:
            pmu_set_register_value_hp(PMU_BUCK_VAUD18_CTRL2, RG_BUCK_VAUD18_IPEAK_VTUNE_H_MASK, RG_BUCK_VAUD18_IPEAK_VTUNE_H_SHIFT, 0x1);
            if(vaud18_vsel_arr[2]==PMU_VAUD18_1P1_V){
               pmu_set_vaud18_voltage_hp(PMU_VAUD18_VSEL_H, PMU_VAUD18_1P1_V);
            }else{
               pmu_set_vaud18_voltage_hp(PMU_VAUD18_VSEL_H, PMU_VAUD18_1P77_V);
            }
            log_hal_msgid_info("[PMU_AUDIO] Audio mode = CLASS D \r\n", 0);
            break;
    }
    log_hal_msgid_info("[PMU_AUDIO] Audio mode = %x \r\n", 1, pmu_get_register_value_hp(PMU_BUCK_VAUD18_CTRL5, RG_AUDIO_MODE_MASK, RG_AUDIO_MODE_SHIFT));
    log_hal_msgid_info("[PMU_AUDIO] VSEL_L= 0x%x IPEAK_L= 0x%x\r\n", 2, pmu_get_register_value_hp(PMU_BUCK_VAUD18_CTRL4, RG_BUCK_VAUD18_VSEL_L_MASK, RG_BUCK_VAUD18_VSEL_L_SHIFT),
                       pmu_get_register_value_hp(PMU_BUCK_VAUD18_CTRL4, RG_BUCK_VAUD18_IPEAK_VTUNE_L_MASK, RG_BUCK_VAUD18_IPEAK_VTUNE_L_SHIFT));
    log_hal_msgid_info("[PMU_AUDIO] VSEL_M= 0x%x IPEAK_M= 0x%x\r\n", 2, pmu_get_register_value_hp(PMU_BUCK_VAUD18_CTRL3, RG_BUCK_VAUD18_VSEL_M_MASK, RG_BUCK_VAUD18_VSEL_M_SHIFT),
                       pmu_get_register_value_hp(PMU_BUCK_VAUD18_CTRL3, RG_BUCK_VAUD18_IPEAK_VTUNE_M_MASK, RG_BUCK_VAUD18_IPEAK_VTUNE_M_SHIFT));
    log_hal_msgid_info("[PMU_AUDIO] VSEL_H= 0x%x IPEAK_H= 0x%x\r\n", 2, pmu_get_register_value_hp(PMU_BUCK_VAUD18_CTRL2, RG_BUCK_VAUD18_VSEL_H_MASK, RG_BUCK_VAUD18_VSEL_H_SHIFT),
                       pmu_get_register_value_hp(PMU_BUCK_VAUD18_CTRL2, RG_BUCK_VAUD18_IPEAK_VTUNE_H_MASK, RG_BUCK_VAUD18_IPEAK_VTUNE_H_SHIFT));
}
/*==========[Interrupt]==========*/
pmu_status_t pmu_register_callback(pmu_interrupt_index_t pmu_int_ch, pmu_callback_t callback, void *user_data)
{
    pmu_status_t status = PMU_STATUS_ERROR;
    if (pmu_int_ch >= PMU_INT_MAX || callback == NULL) {
        return PMU_STATUS_INVALID_PARAMETER;
    }
    pmu_function_table[pmu_int_ch].init_status = 1;
    pmu_function_table[pmu_int_ch].pmu_callback = callback;
    pmu_function_table[pmu_int_ch].user_data = user_data;
    pmu_function_table[pmu_int_ch].isMask = false;
    pmu_register_interrupt |= (1 << pmu_int_ch) ;
    pmu_enable_interrupt(pmu_int_ch, 1);
    pmu_mask_interrupt(pmu_int_ch, 0);
    status = PMU_STATUS_SUCCESS;
    return status;
}
pmu_status_t pmu_deregister_callback(pmu_interrupt_index_t pmu_int_ch)
{
    pmu_status_t status = PMU_STATUS_ERROR;

    if (pmu_int_ch >= PMU_INT_MAX) {
        return PMU_STATUS_INVALID_PARAMETER;
    }
    pmu_function_table[pmu_int_ch].init_status = 0;
    pmu_function_table[pmu_int_ch].pmu_callback = NULL;
    pmu_function_table[pmu_int_ch].user_data = NULL;
    pmu_function_table[pmu_int_ch].isMask = true;
    pmu_register_interrupt |= (~(1 << pmu_int_ch)) ;
    pmu_enable_interrupt(pmu_int_ch, 0);
    pmu_mask_interrupt(pmu_int_ch, 1);
    status = PMU_STATUS_SUCCESS;

    return status;
}
void pmu_get_all_int_status(void)
{
    if (pmu_irq_enable_com0 != 0) {
        pmic_irq0 = pmu_get_register_value_hp(PMU_INT_STATUS0, 0xffff, 0);
    }

    if (pmu_irq_enable_com1 != 0) {
        pmic_irq1 = pmu_get_register_value_hp(PMU_INT_STATUS1, 0xffff, 0);
    }
    if (pmu_init_flag == 0) {
        log_hal_msgid_info("[PMU_BASIC]IRQ0[%x];IRQ1[%x]", 2,pmu_get_register_value_hp(PMU_INT_STATUS0, 0xffff, 0),pmu_get_register_value_hp(PMU_INT_STATUS1, 0xffff, 0));
    }
}
int pmu_get_status_interrupt(pmu_interrupt_index_t int_channel)
{
    int statusValue = -1;
    if ((int_channel >= RG_INT_PWRKEY) && (int_channel <= RG_INT_BATOV)) {
        statusValue = pmu_get_register_value_ddie((uint32_t)&pmic_irq0, 1, int_channel);
    } else if (int_channel == RG_INT_CHRDET) {
        statusValue = pmu_get_register_value_hp(PMU_INT_STATUS0, RG_INT_STATUS_CHRDET_MASK, RG_INT_STATUS_CHRDET_SHIFT);
    } else if ((int_channel >= RG_INT_ChgStatInt) && (int_channel <= RG_INT_SAFETY_TIMEOUT)) {
        statusValue = pmu_get_register_value_ddie((uint32_t)&pmic_irq0, 1, int_channel);
    } else if ((int_channel >= RG_INT_AD_LBAT_LV) && (int_channel <= RG_INT_JEITA_TO_NORMAL)) {
        statusValue = pmu_get_register_value_ddie((uint32_t)&pmic_irq1, 1, (int_channel - RG_INT_AD_LBAT_LV));
    } else {
        log_hal_msgid_info("[PMU_BASIC]Error interrupt index", 0);
        return PMU_STATUS_INVALID_PARAMETER;
    }
    return statusValue;
}
pmu_status_t pmu_mask_interrupt(pmu_interrupt_index_t int_channel, int isEnable)
{
    if ((int_channel >= RG_INT_PWRKEY) && (int_channel <= RG_INT_SAFETY_TIMEOUT)) {
        pmu_set_register_value_hp(PMU_INT_MASK_CON0, 0x1, int_channel, isEnable);
    } else if ((int_channel >= RG_INT_AD_LBAT_LV) && (int_channel <= RG_INT_JEITA_TO_NORMAL)) {
        pmu_set_register_value_hp(PMU_INT_MASK_CON1, 0x1, (int_channel - RG_INT_AD_LBAT_LV), isEnable);
    }  else {
        log_hal_msgid_info("[PMU_BASIC]Error interrupt index", 0);
        return PMU_STATUS_ERROR;
    }
    return PMU_STATUS_SUCCESS;
}
void pmu_irq_count(int int_channel)
{
    if ((int_channel >= RG_INT_PWRKEY) && (int_channel <= RG_INT_SAFETY_TIMEOUT)) {
        event_con0 = 1;
    } else if ((int_channel >= RG_INT_AD_LBAT_LV) && (int_channel <= RG_INT_JEITA_TO_NORMAL)) {
        event_con1 = 1;
    }  else {
        log_hal_msgid_info("[PMU_BASIC]Error interrupt index", 0);
    }
}
void pmu_scan_interrupt_status(void)
{
    uint8_t index = 0xFF;
    uint8_t value = 0;
    for (index = RG_INT_PWRKEY; index < PMU_INT_MAX; index++) {
        value = pmu_get_status_interrupt(index);
        if (value == 1) {
            if (pmu_function_table[index].isMask == false) {
                if (pmu_function_table[index].pmu_callback) {
                    pmu_function_table[index].pmu_callback();
                }
                //Clear Interrupt
                pmu_enable_interrupt(index, 0);
                pmu_enable_interrupt(index, 1);
            }
        }
    }
}
void pmu_irq_init(void)
{
    if (pmu_init_flag == 0) {
        pmu_set_register_value_hp(PMU_INT_CON0, 0xfffe, 0, 0);
        pmu_set_register_value_hp(PMU_INT_CON1, 0xffff, 0, 0);
        pmu_irq_enable_com0 = 0;
        pmu_irq_enable_com1 = 0;
    }
}
pmu_status_t pmu_enable_interrupt(pmu_interrupt_index_t int_channel, int isEnable)
{
    if ((int_channel >= RG_INT_PWRKEY) && (int_channel <= RG_INT_SAFETY_TIMEOUT)) {

        pmu_set_register_value_hp(PMU_INT_CON0, 0x1, int_channel, isEnable);
        if (isEnable) {
            pmu_irq_enable_com0 |= 0x1 << int_channel;
        } else {
            pmu_set_register_value_ddie((uint32_t)&pmu_irq_enable_com0, 0x1, int_channel, 0);
        }
    } else if ((int_channel >= RG_INT_AD_LBAT_LV) && (int_channel <= RG_INT_JEITA_TO_NORMAL)) {
        pmu_set_register_value_hp(PMU_INT_CON1, 0x1, (int_channel - RG_INT_AD_LBAT_LV), isEnable);
        if (isEnable) {
            pmu_irq_enable_com1 |= 0x1 << (int_channel - RG_INT_AD_LBAT_LV);
        } else {
            pmu_set_register_value_ddie((uint32_t)&pmu_irq_enable_com1, 0x1, (int_channel - RG_INT_AD_LBAT_LV), 0);
        }
    } else {
        log_hal_msgid_info("[PMU_BASIC]Error interrupt index", 0);
        return PMU_STATUS_ERROR;
    }
    return PMU_STATUS_SUCCESS;
}
pmu_status_t pmu_clear_interrupt(pmu_interrupt_index_t int_channel)
{
    if (int_channel == PMU_INT_MAX) {
        if ((pmu_irq_enable_com0 != 0) || (event_con0 != 0)) {
            pmu_set_register_value_hp(PMU_INT_STATUS0, 0xffff, 0, 0xffff);
        }
        if ((pmu_irq_enable_com1 != 0) || (event_con1 != 0)) {
            pmu_set_register_value_hp(PMU_INT_STATUS1, 0xffff, 0, 0xffff);
        }
    } else {
        pmu_enable_interrupt(int_channel, 0);
        hal_gpt_delay_us(150);
        pmu_enable_interrupt(int_channel, 1);
    }
    return PMU_STATUS_SUCCESS;
}
pmu_operate_status_t pmu_pk_filter(uint8_t pk_sta) {
    if (pk_sta == 0) { //Press section : pk_next should be Press
        pmu_pk_config.callback1();
        pk_next = PMU_PK_RELEASE;
        pk_first_boot_up=1;
        return PMU_OK;
    } else if (pk_sta == 1) { //Release section : pk_next should be Press
        if(pk_first_boot_up==0){
            log_hal_msgid_info("[PMU_BASIC]PMIC INT[0]  [0]",0);
            pmu_pk_config.callback1();
        }
        pk_first_boot_up = 1;
        pmu_pk_config.callback2();
        pk_next = PMU_PK_PRESS;
        return PMU_OK;
    }else{
        log_hal_msgid_info("[PMU_BASIC]PMIC POWERKEY HW error [%d]", 1,pk_sta);
        return PMU_ERROR;
    }
    return PMU_OK;
}
void pmu_eint_handler(void *parameter)
{
    uint8_t pk_sta;
    int index = 0;
    uint32_t unmask_index;
    hal_eint_mask(HAL_EINT_PMU);
    unmask_index = pmu_register_interrupt;
    pmu_get_all_int_status();
    pk_sta = pmu_get_register_value_hp(PMU_DBG_STS, PWRKEY_VAL_MASK, PWRKEY_VAL_SHIFT);
    for (index = 0; index < PMU_INT_MAX; index++) {
        if ((unmask_index >> index) & (pmu_get_status_interrupt(index))) {
            log_hal_msgid_info("[PMU_BASIC]PMIC INT[%d] [%d]", 2, index, pk_sta);
            if (pmu_function_table[index].init_status != 0) {
                if (index == 0) {
                    pmu_pk_filter(pk_sta);
                }else{
                    pmu_function_table[index].pmu_callback();
            }
            }
            pmu_irq_count(index);
        }
    }
    pmu_clear_interrupt(PMU_INT_MAX);
    hal_eint_unmask(HAL_EINT_PMU);
}
void pmu_eint_init(void)
{
    hal_eint_config_t config;
    config.trigger_mode = HAL_EINT_LEVEL_LOW;
    config.debounce_time = 0;
    hal_eint_init(HAL_EINT_PMU, &config); /*set EINT trigger mode and debounce time.*/
    hal_eint_register_callback(HAL_EINT_PMU, pmu_eint_handler, NULL); /*register a user callback.*/
    hal_eint_unmask(HAL_EINT_PMU);
}
pmu_operate_status_t pmu_pwrkey_normal_key_init_hp(pmu_pwrkey_config_t *config)
{
    pmu_pk_config =*config;
    pmu_status_t status = PMU_STATUS_ERROR;
    status = pmu_register_callback(RG_INT_PWRKEY, config->callback1, config->user_data1);
    if (status != PMU_STATUS_SUCCESS) {
        return PMU_STATUS_ERROR;
    }
    return PMU_STATUS_SUCCESS;
}
/*==========[Power key & Cap touch]==========*/
void pmu_enable_pk_lpsd_hp(pmu_lpsd_time_t tmr, pmu_power_operate_t oper)
{
    log_hal_msgid_info("[PMU_BASIC]Power key lpsd time:%d enable:%d", 2, tmr, oper);
    pmu_set_register_value_hp(PMU_RSTCFG3, RG_PWRKEY_RST_TD_MASK, RG_PWRKEY_RST_TD_SHIFT, tmr);
    pmu_set_register_value_hp(PMU_RSTCFG3, RG_PWRKEY_RST_EN_MASK, RG_PWRKEY_RST_EN_SHIFT, oper);
}

pmu_operate_status_t pmu_lpsd_function_sel(pmu_lpsd_scenario_t oper)
{
    if (oper == PMU_RESET_DEFAULT) {
        pmu_set_register_value_hp(PMU_RSTCFG3, RG_STRUP_LONG_PRESS_EXT_EN_MASK, RG_STRUP_LONG_PRESS_EXT_EN_SHIFT, PMU_ON);
        return pmu_set_register_value_hp(PMU_RSTCFG3, RG_STRUP_LONG_PRESS_EXT_SEL_MASK, RG_STRUP_LONG_PRESS_EXT_SEL_SHIFT, 0);
    } else {
        pmu_set_register_value_hp(PMU_RSTCFG3, RG_STRUP_LONG_PRESS_EXT_EN_MASK, RG_STRUP_LONG_PRESS_EXT_EN_SHIFT, PMU_ON);
        return pmu_set_register_value_hp(PMU_RSTCFG3, RG_STRUP_LONG_PRESS_EXT_SEL_MASK, RG_STRUP_LONG_PRESS_EXT_SEL_SHIFT, oper);
    }
}

void pmu_enable_cap_lpsd_hp(pmu_power_operate_t oper)
{
    log_hal_msgid_info("[PMU_BASIC]Cap touch enable :%d ", 1, oper);
    pmu_set_register_value_hp(PMU_CAP_LPSD_CON0, RG_CAP_LPSD_EN_MASK, RG_CAP_LPSD_EN_SHIFT, oper);
    pmu_set_register_value_hp(PMU_CAP_LPSD_CON0, RG_CAP_LPSD_EN_LATCH_MASK, RG_CAP_LPSD_EN_LATCH_SHIFT, 0x1);
}

void pmu_set_cap_duration_time_hp(pmu_lpsd_time_t tmr)
{
    pmu_set_register_value_hp(PMU_CAP_LPSD_CON0, RG_CAP_LPSD_TD_MASK, RG_CAP_LPSD_TD_SHIFT, tmr);
    pmu_set_register_value_hp(PMU_CAP_LPSD_CON0, RG_CAP_LPSD_TD_LATCH_MASK, RG_CAP_LPSD_TD_LATCH_SHIFT, 0x1);
}

void pmu_set_cap_wo_vbus_hp(pmu_power_operate_t oper)
{
    if (oper) {
        pmu_set_register_value_hp(PMU_AO_RSV, RG_AO_RSV2_MASK, RG_AO_RSV2_SHIFT, 0x1);
        pmu_set_register_value_hp(PMU_AO_RSV, RG_AO_RSV2_MASK, RG_AO_RSV2_SHIFT, 0x3);
        pmu_set_register_value_hp(PMU_AO_RSV, RG_AO_RSV2_MASK, RG_AO_RSV2_SHIFT, 0x0);
    } else {
        pmu_set_register_value_hp(PMU_AO_RSV, RG_AO_RSV2_MASK, RG_AO_RSV2_SHIFT, 0x0);
        pmu_set_register_value_hp(PMU_AO_RSV, RG_AO_RSV2_MASK, RG_AO_RSV2_SHIFT, 0x2);
        pmu_set_register_value_hp(PMU_AO_RSV, RG_AO_RSV2_MASK, RG_AO_RSV2_SHIFT, 0x0);
    }
}

/* API return 1:press ; 0 :release */
bool pmu_get_pwrkey_state_hp(void)
{
    int status = pmu_get_register_value_hp(PMU_DBG_STS, PWRKEY_VAL_MASK, PWRKEY_VAL_SHIFT);
    return (!status);
}
/*==========[Efuse]==========*/
void pmu_efuse_enable_reading(void)
{
    //Get Original MatchKey
    gAuxadcEfuseInfo.matchKey = pmu_get_register_value_hp(PMU_OTP_CON7, RG_OTP_RD_PKEY_MASK, RG_OTP_RD_PKEY_SHIFT);
    // RG_EFUSE_CK_PDN
    pmu_set_register_value_hp(PMU_CKCFG2, RG_EFUSE_CK_PDN_MASK, RG_EFUSE_CK_PDN_SHIFT, 0);
    //RG_EFUSE_CK_PDN_HWEN
    pmu_set_register_value_hp(PMU_CKCFG1, RG_EFUSE_CK_PDN_HWEN_MASK, RG_EFUSE_CK_PDN_HWEN_SHIFT, 0);
    // OTP set Match Key
    pmu_set_register_value_hp(PMU_OTP_CON7, RG_OTP_RD_PKEY_MASK, RG_OTP_RD_PKEY_SHIFT, 0x0289);
    /*Set SW trigger read mode
     * Set HW Mode ;
     * 0: non SW trigger read mode
     * 1: SW trigger read mode*/
    pmu_set_register_value_hp(PMU_OTP_CON11, RG_OTP_RD_SW_MASK, RG_OTP_RD_SW_SHIFT, 1);
}

void pmu_efuse_disable_reading(void)
{
    // RG_EFUSE_CK_PDN
    pmu_set_register_value_hp(PMU_CKCFG2, RG_EFUSE_CK_PDN_MASK, RG_EFUSE_CK_PDN_SHIFT, 1);
    //RG_EFUSE_CK_PDN_HWEN
    pmu_set_register_value_hp(PMU_CKCFG1, RG_EFUSE_CK_PDN_HWEN_MASK, RG_EFUSE_CK_PDN_HWEN_SHIFT, 1);
    // OTP set Original Match Key
    pmu_set_register_value_hp(PMU_OTP_CON7, RG_OTP_RD_PKEY_MASK, RG_OTP_RD_PKEY_SHIFT, gAuxadcEfuseInfo.matchKey);
    /*Set SW trigger read mode
     * Set HW Mode ;
     * 0: non SW trigger read mode
     * 1: SW trigger read mode*/
    pmu_set_register_value_hp(PMU_OTP_CON11, RG_OTP_RD_SW_MASK, RG_OTP_RD_SW_SHIFT, 0);
}

/*==========[Basic]==========*/
/*[Power on/ Power on]*/
void pmu_power_off_sequence_hp(pmu_power_stage_t stage)
{
    switch (stage) {
        case PMU_PWROFF:
            log_hal_msgid_info("[PMU_BASIC]PMU power off ", 0);
            hal_gpt_delay_ms(1);
            pmu_set_register_value_hp(PMU_TPO_CON0, RG_PWROFF_MODE_MASK, RG_PWROFF_MODE_SHIFT, 1); //Power OFF
            break;
        case PMU_RTC:
            log_hal_msgid_info("[PMU_BASIC]PMU Enter RTC mode ", 0);
            hal_gpt_delay_ms(1);
            pmu_set_register_value_hp(PMU_VREF_Interface_ANA_CON2, RG_POFFSTS_CLR_MASK, RG_POFFSTS_CLR_SHIFT, 0x4400);
            pmu_set_register_value_hp(PMU_TPO_CON0, RG_PWROFF_MODE_MASK, RG_PWROFF_MODE_SHIFT, 0); //Power OFF
            pmu_set_register_value_hp(PMU_PWRHOLD, RG_PWRHOLD_MASK, RG_PWRHOLD_SHIFT, 0); //Power hold release
            break;
    }
}

void pmu_latch_power_key_for_bootloader_hp(void)
{
    pmu_set_register_value_hp(PMU_I2C_OUT_TYPE, I2C_CONFIG_MASK, I2C_CONFIG_SHIFT, 1); //D2D need to setting in PP mode, first priority, AB1555 no need, AB1558 D2D nessary
    pmu_set_register_value_hp(PMU_IOCFG3, RG_I2C_CLK_IC_MASK, RG_I2C_CLK_IC_SHIFT, 0x4); //turn on smitt trigger for stability
    pmu_set_register_value_hp(PMU_IOCFG3, RG_I2C_DAT_IC_MASK, RG_I2C_DAT_IC_SHIFT, 0x4); //turn on smitt trigger for stability
    pmu_set_register_value_hp(PMU_PWRHOLD, RG_PWRHOLD_MASK, RG_PWRHOLD_SHIFT, 1); //Power hold
    pmu_set_register_value_hp(PMU_RSTCFG1, RG_WDTRSTB_EN_MASK, RG_WDTRSTB_EN_SHIFT, 0x1); // Enable WDT
    /*Setting for get vbux auxadc*/
    pmu_set_register_value_hp(PMU_AUXADC_AD_CON0, AUXADC_CK_AON_MASK, AUXADC_CK_AON_SHIFT, 0x0);
    pmu_set_register_value_hp(PMU_AUXADC_AD_CON3, AUXADC_SPL_NUM_LARGE_MASK, AUXADC_SPL_NUM_LARGE_SHIFT, 0x7F);
    pmu_set_register_value_hp(PMU_AUXADC_AD_CON5, AUXADC_SPL_NUM_SEL_MASK, AUXADC_SPL_NUM_SEL_SHIFT, 0x6);
#ifdef PMU_BOOTLOADER_ENABLE_CHARGER
    pmu_set_register_value_hp(PMU_LCHR_DIG_CON14, 0xfff, 0, pmu_cc_cur[PMU_BL_PRECC]); //precc
    pmu_set_register_value_hp(PMU_LCHR_DIG_CON15, 0xfff, 0, pmu_cc_cur[PMU_BL_CC]); //cc
    pmu_set_register_value_hp(PMU_CHR_AO_VCV, RG_VCV_VTH_MASK, RG_VCV_VTH_SHIFT, 0);
    pmu_set_register_value_hp(PMU_CORE_CORE_ANA_CON9, RG_LOOP_CHRLDO_SB_DIS_MASK, RG_LOOP_CHRLDO_SB_DIS_SHIFT, 0x80);
    pmu_set_register_value_hp(PMU_CORE_CORE_ANA_CON8, RG_SYSDPM_STATUS_SEL_MASK, RG_SYSDPM_STATUS_SEL_SHIFT, 0);
    //pmu_set_register_value_hp(PMU_CORE_CORE_ANA_CON8, RG_BUSDPM_DELTA_VTH_MASK, RG_BUSDPM_DELTA_VTH_SHIFT, 0x1);/*Set BUSDPM 200mV*/
    pmu_set_register_value_hp(PMU_LCHR_DIG_CON4, RG_EN_CHR_MASK, RG_EN_CHR_SHIFT, 0x1);
#endif

}
uint8_t pmu_get_power_on_reason_hp(void)
{
    int i = 0;
    uint8_t reason;
    reason = pmu_get_register_value_hp(PMU_PONSTS, 0x1F, 0);
    if (!pmu_init_flag) {
        for (i = 0; i < 15; i++) {
            if (reason & (0x1 << i)) {
                log_hal_info("[PMU_BASIC]power_on_reason : %s[%d]", power_on_reason[i], i);
            }
        }
    }
    return reason;
}

uint8_t pmu_get_power_off_reason_hp(void)
{
    uint8_t reason;
    reason = pmu_get_register_value_hp(PMU_VREF_Interface_ANA_CON3, RGS_POFFSTS_MASK, RGS_POFFSTS_SHIFT);
    if (!pmu_init_flag) {
        log_hal_info("[PMU_BASIC]power_off_reason : %s[%d]", power_off_reason[reason], reason);
    }
    return reason;
}
/* 0: efuse not ready; 1 : efuse ready*/
bool pmu_get_efuse_status(void)
{
    return pmu_get_register_value_hp(PMU_EFUSE_RSV, RG_AO_EFUSE_RSV_MASK, RG_AO_EFUSE_RSV_SHIFT);
}
/*when boot up,press power key need more than the specific time*/
void pmu_press_pk_time(void)
{
    uint8_t pk_sta;
    uint32_t pmu_gpt_start, pmu_get_press_time, pmu_get_duration_time = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &pmu_gpt_start);
    while (1) {
        pk_sta = pmu_get_register_value(PMU_DBG_STS, PWRKEY_VAL_MASK, PWRKEY_VAL_SHIFT);
        if (pk_sta == 0) {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &pmu_get_press_time);
            hal_gpt_get_duration_count(pmu_gpt_start, pmu_get_press_time, &pmu_get_duration_time);
            pmu_set_register_value_hp(PMU_PWRHOLD, RG_PWRHOLD_MASK, RG_PWRHOLD_SHIFT, 0x1); //Power hold
        } else {
            pmu_get_press_time = 0;
            log_hal_msgid_error("[PMU_BASIC]ON[%x]OFF[%x]DT[%d]PT[%d]", 4, pmu_get_power_on_reason_hp(), pmu_get_power_off_reason_hp(), pmu_get_duration_time, PMU_PRESS_PK_TIME);
            log_hal_msgid_error("[PMU_BASIC]Boot up fail , press pk need more than the specific time %d or PMIC OP", 1, PMU_PRESS_PK_TIME);
            hal_gpt_delay_ms(1);
#ifdef HAL_RTC_MODULE_ENABLED
            hal_rtc_init();
            hal_rtc_enter_rtc_mode();
#endif
        }
        if ((pmu_get_duration_time > PMU_PRESS_PK_TIME)) {
            log_hal_msgid_error("[PMU_BASIC]Boot on!! , press pk time %d", 1, pmu_get_duration_time);
            break;
        }
    }
}
/* Get usb put in status*/

uint8_t pmu_get_usb_input_status_hp(void)
{
    return pmu_get_register_value_hp(PMU_CHR_AO_DBG0, DA_QI_CHR_REF_EN_MASK, DA_QI_CHR_REF_EN_SHIFT);
}

void hal_pmu_sleep_suspend(void)
{
}
void hal_pmu_sleep_resume(void)
{
}

void pmu_set_rstpat_hp(pmu_power_operate_t oper, pmu_rstpat_src_t src)
{
    if (oper > PMU_ON || oper < PMU_OFF) {
        return;
    }
    if (src > PMU_RSTPAT_SRC_VBUS_UART || src < PMU_RSTPAT_SRC_VBUS) {
        return;
    }

    pmu_set_register_value_hp(PMU_VBUS_UART_DIG2, RG_RSTPAT_EN_MASK, RG_RSTPAT_EN_SHIFT, oper);
    pmu_set_register_value_hp(PMU_CORE_CORE_ANA_CON14, RG_PAT_SRC_SEL1_MASK, RG_PAT_SRC_SEL1_SHIFT, src);
}

void pmu_safety_confirmation(void)
{
    if (pmu_get_pmic_version() && (pmu_get_register_value_hp(PMU_EFUSE_RSV, 0x1, 0) != 0x1)) {
        log_hal_msgid_error("[PMU_BASIC]Please Check PMIC Efuse and Version", 0);
        hal_gpt_delay_ms(1);
#ifdef HAL_RTC_MODULE_ENABLED
        hal_rtc_init();
        hal_rtc_enter_rtc_mode();
#endif
    }
    if(pmu_get_register_value_hp(PMU_CORE_CORE_ELR_2, 0xF, 0)!=0){
       log_hal_msgid_error("[PMU_BASIC]Untrimmed SYSLDO value [PMU_CORE_CORE_ELR_2 :%x]", 1,pmu_get_register_value_hp(PMU_CORE_CORE_ELR_2, 0xF, 0));
    }else{
       log_hal_msgid_info("[PMU_BASIC]trimmed SYSLDO value [PMU_CORE_CORE_ELR_2 :%x]", 1,pmu_get_register_value_hp(PMU_CORE_CORE_ELR_2, 0xF, 0));
    }
}
uint8_t pmu_get_pmic_version_hp(void)
{
    pmic_iv_version = pmu_get_register_value_hp(PMU_STRUP_Interface_ANA_CON1, RGS_ANA_CHIP_ID_MASK, RGS_ANA_CHIP_ID_SHIFT);
    //log_hal_msgid_info("PMIC IC version : %d", 1,pmic_iv_version);
    return pmic_iv_version;
}
/*[PMU INIT]*/
void pmu_init_hp(void)
{
    pmu_set_register_value_hp(PMU_IOCFG3, RG_I2C_CLK_IC_MASK, RG_I2C_CLK_IC_SHIFT, 0x4);
    pmu_set_register_value_hp(PMU_IOCFG3, RG_I2C_DAT_IC_MASK, RG_I2C_DAT_IC_SHIFT, 0x4);
    pmu_set_register_value_hp(PMU_CKCFG2, RG_CLK_TRIM_F128K_CK_PDN_MASK, RG_CLK_TRIM_F128K_CK_PDN_SHIFT, 0x1);
    pmu_get_power_off_reason_hp();
    pmu_get_power_on_reason_hp();
    pmu_set_register_value_hp(PMU_RSTCFG1, RG_WDTRSTB_EN_MASK, RG_WDTRSTB_EN_SHIFT, 0x1);
    pmu_set_register_value_hp(PMU_RSTCFG3, RG_STRUP_LONG_PRESS_EXT_RTCA_CTRL_MASK, RG_STRUP_LONG_PRESS_EXT_RTCA_CTRL_SHIFT, 0x1);
    pmu_set_register_value_hp(PMU_RSTCFG3, RG_STRUP_LONG_PRESS_EXT_PWRKEY_CTRL_MASK, RG_STRUP_LONG_PRESS_EXT_PWRKEY_CTRL_SHIFT, 0x1);
    pmu_set_register_value_hp(PMU_RSTCFG3, RG_STRUP_LONG_PRESS_EXT_CHR_CTRL_MASK, RG_STRUP_LONG_PRESS_EXT_CHR_CTRL_SHIFT, 0x1);
    pmu_set_register_value_hp(PMU_VAUD18DRV_MODE, RG_VAUD18DRV_VOSEL0_HW_MODE_MASK, RG_VAUD18DRV_VOSEL1_HW_MODE_SHIFT, 0x1);
    pmu_set_register_value_hp(PMU_VAUD18DRV_MODE, RG_VAUD18DRV_VOSEL0_HW_MODE_MASK, RG_VAUD18DRV_VOSEL1_HW_MODE_SHIFT, 0x1);
    pmu_set_register_value_hp(PMU_INT_CON0, RG_INT_EN_CHGSTATINT_MASK, RG_INT_EN_CHGSTATINT_SHIFT, 0x1);
    pmu_set_register_value_hp(PMU_INT_MASK_CON0, RG_INT_MASK_PWRKEY_MASK, RG_INT_MASK_PWRKEY_SHIFT, 0x0);
    pmu_set_register_value_hp(PMU_TPO_CON1, RG_SRCLKEN_HW_MODE_MASK, RG_SRCLKEN_HW_MODE_SHIFT, 0x1);
#if PMU_AVOID_ACCIDENTAL_BOOTUP
    if (pmu_get_power_on_reason_hp() == 0x1 && pmu_get_power_off_reason_hp() != 0xd && pmu_get_power_off_reason_hp() != 0x8) {
        pmu_press_pk_time();
    } else {
        pmu_set_register_value_hp(PMU_PWRHOLD, RG_PWRHOLD_MASK, RG_PWRHOLD_SHIFT, 0x1); //Power hold
    }
#else
    pmu_set_register_value_hp(PMU_PWRHOLD, RG_PWRHOLD_MASK, RG_PWRHOLD_SHIFT, 0x1); //Power hold
#endif
    pmu_set_register_value_hp(PMU_STRUP_CON1, RG_STRUP_AUXADC_RPCNT_MAX_MASK, RG_STRUP_AUXADC_RPCNT_MAX_SHIFT, 0x42);
    pmu_set_register_value_hp(PMU_STRUP_CON1, RG_STRUP_AUXADC_RSTB_SEL_MASK, RG_STRUP_AUXADC_RSTB_SEL_SHIFT, 0x1);
    pmu_set_register_value_hp(PMU_LDO_TOP_CLK_VA18_CON0, RG_LDO_VA18_CK_SW_MODE_MASK, RG_LDO_VA18_CK_SW_MODE_SHIFT, 0x0);
    pmu_set_register_value_hp(PMU_LDO_TOP_CLK_VLDO31_CON0, RG_LDO_VLDO31_CK_SW_MODE_MASK, RG_LDO_VLDO31_CK_SW_MODE_SHIFT, 0x0);
    pmu_set_register_value_hp(PMU_LDO_TOP_CLK_VSRAM_CON0, RG_LDO_VSRAM_CK_SW_MODE_MASK, RG_LDO_VSRAM_CK_SW_MODE_SHIFT, 0x0);
    pmu_set_register_value_hp(PMU_VSRAM_SPW_CON1, RG_PSW_CTRL_CK_SW_MODE_MASK, RG_PSW_CTRL_CK_SW_MODE_SHIFT, 0x0); /*VSRAM power source in slp*/
    pmu_set_register_value_hp(PMU_AUXADC_AD_CON0, AUXADC_CK_AON_MASK, AUXADC_CK_AON_SHIFT, 0x0);
    pmu_set_register_value_hp(PMU_AUXADC_AD_CON3, AUXADC_SPL_NUM_LARGE_MASK, AUXADC_SPL_NUM_LARGE_SHIFT, 0x7F);
    pmu_set_register_value_hp(PMU_AUXADC_AD_CON5, AUXADC_SPL_NUM_SEL_MASK, AUXADC_SPL_NUM_SEL_SHIFT, 0x6);
    pmu_set_register_value_hp(PMU_CORE_CORE_ANA_CON9, RG_LOOP_CHRLDO_SB_DIS_MASK, RG_LOOP_CHRLDO_SB_DIS_SHIFT, 0x80);
    pmu_set_register_value_hp(PMU_VREF_Interface_ANA_CON2, RG_AO_TRIM_REG_RELOAD_MASK, RG_AO_TRIM_REG_RELOAD_SHIFT, 0x1);
    pmu_set_register_value_hp(PMU_VREF_Interface_ANA_CON2, RG_AO_TRIM_REG_RELOAD_MASK, RG_AO_TRIM_REG_RELOAD_SHIFT, 0x0);
    pmu_set_register_value_hp(PMU_TPO_CON5, RG_AO_TRIM_SEL_MASK, RG_AO_TRIM_SEL_SHIFT, 0x1);
    pmu_set_register_value_hp(PMU_TPO_CON5, RG_AO_TRIM_SEL_LATCH_MASK, RG_AO_TRIM_SEL_LATCH_SHIFT, 0x1);
    pmu_set_register_value_hp(PMU_TPO_CON5, 0x3f, RG_AO_TRIM_SEL_LATCH_SHIFT, 0x0);
    pmu_set_register_value_hp(PMU_TPO_CON5, RG_AO_TRIM_SEL_LATCH_MASK, RG_AO_TRIM_SEL_LATCH_SHIFT, 0x1);
    pmu_auxadc_init();
    if (pmu_get_register_value_hp(PMU_INT_STATUS0, RG_INT_STATUS_CHRDET_MASK, RG_INT_STATUS_CHRDET_SHIFT)) {
        pmu_charger_status = pmu_get_usb_input_status_hp();
    }
    pmu_eint_init();
    pmu_get_all_int_status();
    pmu_irq_init();
#ifdef HAL_SLEEP_MANAGER_ENABLED
    sleep_management_register_all_secure_suspend_callback(SLEEP_BACKUP_RESTORE_PMU, (sleep_management_suspend_callback_t)hal_pmu_sleep_suspend, NULL);
    sleep_management_register_all_secure_resume_callback(SLEEP_BACKUP_RESTORE_PMU, (sleep_management_resume_callback_t)hal_pmu_sleep_resume, NULL);
#endif
    pmu_safety_confirmation();
    pmu_get_pmic_version();
    if (pmic_iv_version) { //E1 need to disable
        pmu_set_register_value_hp(PMU_TPO_CON6, RG_SMPS_CK_EN_DCM_MODE_MASK, RG_SMPS_CK_EN_DCM_MODE_SHIFT, 0x1); //E1 disable
    }
    /*feature setting*/
    pmu_lpsd_function_sel(PMU_REPRESS_PWRKEY);

    /*Setting for RF tx power*/
    pmu_set_register_value_hp(PMU_BUCK_VRF_CTRL4, RG_BUCK_VRF_VOUTSEL_MASK, RG_BUCK_VRF_VOUTSEL_SHIFT, 0x2); //1223 VRF
    pmu_set_register_value_hp(PMU_BUCK_VRF_CTRL4, RG_BUCK_VRF_IPEAK_VTUNE_MASK, RG_BUCK_VRF_IPEAK_VTUNE_SHIFT, 0x1);
    /*Add for buck ripple control */
    pmu_ripple_init_hp(PMU_ON);
    pmu_set_register_value_hp(PMU_VREF_Interface_ANA_CON0, RG_BGR_RSEL_MASK, RG_BGR_RSEL_SHIFT, 0x0); //1115 init setting
    pmu_set_register_value_hp(PMU_VPA_ANA_TRIM, RG_VPA_ZXOS_TRIM_LV_MASK, RG_VPA_ZXOS_TRIM_LV_SHIFT, 0x3); //1115 init setting
    pmu_efuse_init_hp();
#ifdef PMU_BOOTLOADER_ENABLE_CHARGER
    log_hal_msgid_info("[PMU_BASIC]PMU_BOOTLOADER_ENABLE_CHARGER ON", 0);
#endif
#ifdef PMU_SLT_ENV
    vsram_trim = pmu_get_register_value_hp(PMU_LDOTOP_ELR_3, RG_VSRAM_VOTRIM_MASK, RG_VSRAM_VOTRIM_SHIFT);
    log_hal_msgid_info("[PMU_BASIC][PMU_SLT]VSRAM_VOTRIM[%x]", 1, vsram_trim);
#endif
#ifdef AIR_SYS32K_CLOCK_SOURCE_DCXO
    pmu_switch_power(PMU_BUCK_VRF, PMU_LOWPOWER_MODE, PMU_ON);
#endif
    pmu_init_flag = 1;
}
void pmu_efuse_init_hp(void)
{
    uint32_t efuse_18 = 0, efuse_19 = 0, efuse_20 = 0, temp_value = 0;
    pmu_efuse_enable_reading();
    efuse_18 = pmu_core_efuse_get_efuse_data_by_address(18);
    efuse_19 = pmu_core_efuse_get_efuse_data_by_address(19);
    efuse_20 = pmu_core_efuse_get_efuse_data_by_address(20);
    log_hal_msgid_info("[PMU_BASIC]Efuse data[%x][%x][%x]", 3, efuse_18, efuse_19, efuse_20);
    temp_value = pmu_get_register_value_ddie((uint32_t)&efuse_18, 0xf, 3);
    pmu_set_register_value_hp(PMU_VBUS_UART_ER0, RG_EFUSE_UART_ICL_RFB_TRIM_MASK, RG_EFUSE_UART_ICL_RFB_TRIM_SHIFT, temp_value);
    //log_hal_msgid_info("[PMU]EFUSE_UART_ICL_RFB_TRIM [%x]", 1,temp_value);
    temp_value = pmu_get_register_value_ddie((uint32_t)&efuse_18, 0x1, 7) + (pmu_get_register_value_ddie((uint32_t)&efuse_19, 0x7, 1) << 1);
    pmu_set_register_value_hp(PMU_VBUS_UART_ER0, RG_EFUSE_UART_ICL_RREF_TRIM_MASK, RG_EFUSE_UART_ICL_RREF_TRIM_SHIFT, temp_value);
    //log_hal_msgid_info("[PMU]RG_EFUSE_UART_ICL_RREF_TRIM [%x]", 1,temp_value);
    temp_value = pmu_get_register_value_ddie((uint32_t)&efuse_19, 0x1f, 3) + (pmu_get_register_value_ddie((uint32_t)&efuse_20, 0x1, 0) << 5);
    pmu_set_register_value_hp(PMU_VBUS_UART_ER0, RG_EFUSE_UART_IDAC_REVFET_TRIM_MASK, RG_EFUSE_UART_IDAC_REVFET_TRIM_SHIFT, temp_value);
    //log_hal_msgid_info("[PMU]RG_EFUSE_UART_IDAC_REVFET_TRIM [%x]", 1,temp_value);
    temp_value = pmu_get_register_value_ddie((uint32_t)&efuse_20, 0x3, 1);
    pmu_set_register_value_hp(PMU_VBUS_UART_ER1, RG_EFUSE_UART_PREG_VTRIM_MASK, RG_EFUSE_UART_PREG_VTRIM_SHIFT, temp_value);
    //log_hal_msgid_info("[PMU]RG_EFUSE_UART_PREG_VTRIM [%x]", 1,temp_value);
    temp_value = pmu_get_register_value_ddie((uint32_t)&efuse_20, 0xf, 3);
    pmu_set_register_value_hp(PMU_VBUS_UART_ER1, RG_EFUSE_UART_PREG_TCTRIM_MASK, RG_EFUSE_UART_PREG_TCTRIM_SHIFT, temp_value);
    //log_hal_msgid_info("[PMU]RG_EFUSE_UART_PREG_TCTRIM [%x]", 1,temp_value);
    pmu_efuse_disable_reading();
#ifdef HAL_SLEEP_MANAGER_ENABLED
    pmu_handle = hal_sleep_manager_set_sleep_handle("pmu");
#endif
    gAuxadcEfuseInfo.isInit = 1;
}
uint32_t pmu_d2d_i2c_read(unsigned char *ptr_send, unsigned char *ptr_read, int type)
{
    hal_i2c_send_to_receive_config_t config;
    unsigned char retry_cnt = 0, result_read;
    if (type == 1) {
        *(ptr_send) = *(ptr_send) | 0x40;
        config.receive_length = 1;
    } else {
        config.receive_length = 2;
    }
    config.slave_address = PMIC_SLAVE_ADDR;
    config.send_data = ptr_send;
    config.send_length = 2;
    config.receive_buffer = ptr_read;
    do {
        result_read = hal_i2c_master_send_to_receive_polling(HAL_I2C_MASTER_AO, &config);
        retry_cnt++;
        if (retry_cnt == 59) {
        #ifndef HAL_PMU_NO_RTOS
            configASSERT(0 && "[PMU_BASIC]ERROR I2c access PMIC fail");
        #endif
            assert(0);
        }
    } while ((result_read != 0) && (retry_cnt <= 60));
    return (retry_cnt);
}

uint32_t pmu_get_register_value_hp(uint32_t address, uint32_t mask, uint32_t shift)
{
    unsigned char send_buffer[4], receive_buffer[2];
    uint32_t value;
    pmic_i2c_init();
    send_buffer[1] = address & 0x00FF; //D2D 2Byte
    send_buffer[0] = ((address >> 8) & 0x00FF) & 0x0F;
    pmu_d2d_i2c_read(send_buffer, receive_buffer, 2);
    value = (receive_buffer[1] << 8) + receive_buffer[0];
    return ((value >> shift) & mask);
}

pmu_operate_status_t pmu_set_register_value_hp(uint32_t address, uint32_t mask, uint32_t shift, uint32_t value)
{
    unsigned char send_buffer[4], receive_buffer[2];
    uint32_t data;
    pmic_i2c_init();
    send_buffer[1] = address & 0x00FF;
    send_buffer[0] = ((address >> 8) & 0x00FF) & 0x0F;
    pmu_d2d_i2c_read(send_buffer, receive_buffer, 2);

    data = receive_buffer[1];
    data = (data << 8) | receive_buffer[0];
    data &= (~(mask << shift));
    data = data | (value << shift);

    send_buffer[0] = ((address >> 8) & 0x00FF) | 0x00;
    send_buffer[1] = (address) & 0x00FF;
    send_buffer[2] = (data & 0xFF);
    send_buffer[3] = ((data >> 8) & 0xFF);

    unsigned char retry_cnt = 0, result_read;
    do {
        result_read = hal_i2c_master_send_polling(HAL_I2C_MASTER_AO, PMIC_SLAVE_ADDR, send_buffer, 4);
        retry_cnt++;
        if (retry_cnt == 59) {
        #ifndef HAL_PMU_NO_RTOS
            configASSERT(0 && "[PMU_BASIC]ERROR I2c access PMIC fail");
        #endif
            assert(0);
        }
    } while ((result_read != 0) && (retry_cnt <= 60));
    return PMU_OK;
}

void pmic_i2c_init(void)
{
    if (pmu_i2c_init_sta == 1) {
        return;
    }
    int status;
    hal_i2c_config_t config;
#ifdef AB1555
    config.frequency = HAL_I2C_FREQUENCY_400K;
#else
    config.frequency = HAL_I2C_FREQUENCY_2M;
#endif
    status = hal_i2c_master_init(HAL_I2C_MASTER_AO, &config);
    if (status != HAL_I2C_STATUS_OK) {
        assert(0);
    }
    hal_i2c_master_set_io_config(HAL_I2C_MASTER_AO, HAL_I2C_IO_PUSH_PULL);
    pmu_i2c_init_sta = 1;
}

void pmu_select_wdt_mode_hp(pmu_wdtrstb_act_t sel)
{
    /*Check low power diff*/
    pmu_set_register_value_hp(PMU_RSTCFG1, RG_WDTRST_ACT_MASK, RG_WDTRST_ACT_SHIFT, sel);
}
void pmu_get_pmic_info(void)
{
    log_hal_msgid_info("[PMU_BASIC]=====[PMU Info]===== ", 0);
    log_hal_msgid_info("[PMU_BASIC]=====[IRQ]===== ", 0);
    log_hal_msgid_info("[PMU_BASIC][PMU_IRQ]SW_com0[%x]SW_com1[%x]", 2, pmu_irq_enable_com0, pmu_irq_enable_com1);
    log_hal_msgid_info("[PMU_BASIC][PMU_IRQ]INT_MASK_CON0[%x]INT_MASK_CON1[%x]", 2, pmu_get_register_value_hp(PMU_INT_MASK_CON0, 0xffff, 0), pmu_get_register_value_hp(PMU_INT_MASK_CON1, 0xffff, 0));
    log_hal_msgid_info("[PMU_BASIC][PMU_IRQ]INT_CON0[%x]INT_CON1[%x]", 2, pmu_get_register_value_hp(PMU_INT_CON0, 0xffff, 0), pmu_get_register_value_hp(PMU_INT_CON1, 0xffff, 0));
    log_hal_msgid_info("[PMU_BASIC][PMU_IRQ]INT_STATUS0[%x]INT_STATUS1[%x]", 2, pmu_get_register_value_hp(PMU_INT_STATUS0, 0xffff, 0), pmu_get_register_value_hp(PMU_INT_STATUS1, 0xffff, 0));
    log_hal_msgid_info("[PMU_BASIC][PMU_IRQ]INT_RAW_STATUS0[%x]INT_RAW_STATUS1[%x]", 2, pmu_get_register_value_hp(PMU_INT_RAW_STATUS0, 0xffff, 0), pmu_get_register_value_hp(PMU_INT_RAW_STATUS1, 0xffff, 0));
}

void pmu_get_lock_info(void)
{
    int vol_index = 0;
    for (vol_index = 0; vol_index < PMIC_VCORE_FAIL_V; vol_index++) {
        log_hal_msgid_info("[PMU_BASIC]vcore_resource_ctrl[%d]:%d", 2, vol_index, vcore_resource_ctrl[vol_index]);
    }
}
void pmu_ripple_init_hp(pmu_power_operate_t oper)
{
    if (oper == PMU_ON) {
        pmu_select_buck_ripple(PMU_BUCK_VAUD18, PMU_ON);
        pmu_select_buck_ripple(PMU_BUCK_VPA, PMU_ON);
        log_hal_msgid_info("[PMU_PWR]PMU Ripple on", 0);
    } else {
        pmu_select_buck_ripple(PMU_BUCK_VCORE, PMU_OFF);
        pmu_select_buck_ripple(PMU_BUCK_VIO18, PMU_OFF);
        pmu_select_buck_ripple(PMU_BUCK_VAUD18, PMU_OFF);
        pmu_select_buck_ripple(PMU_BUCK_VPA, PMU_OFF);
        log_hal_msgid_info("[PMU_PWR]PMU Ripple off", 0);
    }
    log_hal_msgid_info("[PMU_PWR]PMU_VCORE_ANA_CTRL1 :%x", 1, pmu_get_register_value_hp(PMU_VCORE_ANA_CTRL1, 0xffff, 0));
    log_hal_msgid_info("[PMU_PWR]PMU_VPA_ANA_RSV0 :%x", 1, pmu_get_register_value_hp(PMU_VPA_ANA_RSV0, 0xffff, 0));
    log_hal_msgid_info("[PMU_PWR]PMU_VAUD18_ANA_CTRL1 :%x", 1, pmu_get_register_value_hp(PMU_VAUD18_ANA_CTRL1, 0xffff, 0));
    log_hal_msgid_info("[PMU_PWR]PMU_VAUD18_ANA_TRIM :%x", 1, pmu_get_register_value_hp(PMU_VAUD18_ANA_TRIM, 0xffff, 0));
    log_hal_msgid_info("[PMU_PWR]PMU_VIO18_ANA_CTRL1 :%x", 1, pmu_get_register_value_hp(PMU_VIO18_ANA_CTRL1, 0xffff, 0));
}

void pmu_select_buck_ripple(pmu_power_domain_t pmu_pdm, pmu_power_operate_t oper)
{
    uint32_t temp_value = 0;
    switch (pmu_pdm) {
        case PMU_BUCK_VCORE:
            if (oper) {
                pmu_set_register_value_hp(PMU_VCORE_ANA_CTRL1, RG_VCORE_DYNAMIC_IPEAK_MASK, RG_VCORE_DYNAMIC_IPEAK_SHIFT, 0xE3);
            } else {
                temp_value = pmu_get_register_value_hp(PMU_VCORE_ANA_CTRL1, RG_VCORE_DYNAMIC_IPEAK_MASK, RG_VCORE_DYNAMIC_IPEAK_SHIFT);
                temp_value &= 0xfe;
                pmu_set_register_value_hp(PMU_VCORE_ANA_CTRL1, RG_VCORE_DYNAMIC_IPEAK_MASK, RG_VCORE_DYNAMIC_IPEAK_SHIFT, temp_value);
            }
            break;
        case PMU_BUCK_VIO18:
            if (oper) {
                /*Default Enable*/
            } else {
                temp_value = pmu_get_register_value_hp(PMU_VIO18_ANA_CTRL1, RG_VIO18_DYNAMIC_IPEAK_MASK, RG_VIO18_DYNAMIC_IPEAK_SHIFT);
                temp_value &= 0xfe;
                pmu_set_register_value_hp(PMU_VIO18_ANA_CTRL1, RG_VIO18_DYNAMIC_IPEAK_MASK, RG_VIO18_DYNAMIC_IPEAK_SHIFT, temp_value);
            }
            break;
        case PMU_BUCK_VAUD18:
            if (oper) {
                pmu_set_register_value_hp(PMU_VAUD18_ANA_CTRL1, RG_VAUD18_DYNAMIC_IPEAK_MASK, RG_VAUD18_DYNAMIC_IPEAK_SHIFT, 0x1);  //VAUD18
                if (pmu_get_register_value_hp(PMU_VAUD18_ANA_TRIM, RG_VAUD18_RC_R_TRIM_MASK, RG_VAUD18_RC_R_TRIM_SHIFT) == 0) {
                    pmu_set_register_value_hp(PMU_VAUD18_ANA_TRIM, RG_VAUD18_RC_R_TRIM_MASK, RG_VAUD18_RC_R_TRIM_SHIFT, 0x7);  //VAUD18
                }
            } else {
                temp_value = pmu_get_register_value_hp(PMU_VAUD18_ANA_CTRL1, RG_VAUD18_DYNAMIC_IPEAK_MASK, RG_VAUD18_DYNAMIC_IPEAK_SHIFT);
                temp_value &= 0xfe;
                pmu_set_register_value_hp(PMU_VAUD18_ANA_CTRL1, RG_VAUD18_DYNAMIC_IPEAK_MASK, RG_VAUD18_DYNAMIC_IPEAK_SHIFT, temp_value);  //VAUD18
            }
            break;
        case PMU_BUCK_VPA:
            if (oper) {
                pmu_set_register_value_hp(PMU_VPA_ANA_RSV0, RG_VPA_RVD2_MASK, RG_VPA_RVD2_SHIFT, 0x7);
                pmu_set_register_value_hp(PMU_VPA_ANA_RSV0, RG_VPA_RVD_MASK, RG_VPA_RVD_SHIFT, 0x71);
            } else {
                temp_value = pmu_get_register_value_hp(PMU_VPA_ANA_RSV0, RG_VPA_RVD2_MASK, RG_VPA_RVD2_SHIFT);
                temp_value &= 0xfd;
                pmu_set_register_value_hp(PMU_VPA_ANA_RSV0, RG_VPA_RVD2_MASK, RG_VPA_RVD2_SHIFT, temp_value);
            }
            break;
        default:
            log_hal_msgid_info("[PMU_PWR]pmu_pdm input error", 0);
            break;
    }

}
pmu_status_t pmu_check_dummy_trim_value(uint32_t *dlh_value, uint32_t *dll_value)
{
    if (((*dlh_value & 0x8000) >> 15) & (*dll_value & 0x8000) >> 15) { //Check efuse status
        log_hal_msgid_info("[PMU_PWR][Dummy_loading]Check dummy loading trim value is pass", 0);
        *dlh_value &= 0x7fff;
        *dll_value &= 0x7fff;
        return PMU_STATUS_SUCCESS;
    } else {
        log_hal_msgid_error("[PMU_PWR][Dummy_loading]dummy loading trim value is not exist in efuse", 0);
        return PMU_STATUS_ERROR;
    }
}
void pmu_set_dummy_load_hp(pmu_power_domain_t domain, uint32_t loading_value)
{
    uint32_t DLH_value = 0, DLL_value = 0;
    uint32_t h_value = 0, m_value = 0, l_value = 0;
    memset(&dummy_loading_str, 0, sizeof(pmu_dummy_load_str));
    switch (domain) {
        case PMU_BUCK_VCORE:
            if (loading_value == 0) {
                pmu_set_register_value_hp(PMU_VCORE_ANA_RSV0, 0x1, 12, 0x0);  //Disable PMU dummy load
                log_hal_msgid_info("[PMU_PWR][Dummy_loading]Disable DL PMU_VCORE_ANA_RSV0[%x]", 1, pmu_get_register_value_hp(PMU_VCORE_ANA_RSV0, 0xffff, 0));
            } else {
                pmu_efuse_enable_reading();
                DLH_value = pmu_core_efuse_get_efuse_data_by_address(88);
                DLL_value = pmu_core_efuse_get_efuse_data_by_address(86);
                if (pmu_check_dummy_trim_value(&DLH_value, &DLL_value) != PMU_STATUS_SUCCESS) {
                    break;
                }
                log_hal_msgid_info("[PMU_PWR][Dummy_loading][VCORE]loading_value [%d]", 1, loading_value);
                log_hal_msgid_info("[PMU_PWR][Dummy_loading][VCORE]Efuse H[%d]L[%d]", 2, DLH_value, DLL_value);
                dummy_loading_str.vcore_val = ((loading_value - (DLL_value - ((DLH_value - DLL_value) / 30))) / ((DLH_value - DLL_value) / 30));
                dummy_loading_str.vcore_val = (dummy_loading_str.vcore_val + 0.5);
                h_value = (dummy_loading_str.vcore_val >> 2);
                l_value = (dummy_loading_str.vcore_val & 0x3);
                log_hal_msgid_info("[PMU_PWR][Dummy_loading]EfuseH[%d]L[%d]", 2, h_value, l_value);
                pmu_set_register_value_hp(PMU_VCORE_ANA_RSV0, 0x1, 12, 0x1);  //Enable PMU dummy load
                pmu_set_register_value_hp(PMU_VCORE_ANA_RSV0, 0x3, 10, l_value);
                pmu_set_register_value_hp(PMU_VCORE_ANA_RSV0, 0x7, 13, h_value);
                log_hal_msgid_info("[PMU_PWR][Dummy_loading]PMU_VCORE_ANA_RSV0[%x]", 1, pmu_get_register_value_hp(PMU_VCORE_ANA_RSV0, 0xffff, 0));
            }
            break;
        case PMU_BUCK_VIO18:
            if (loading_value == 0) {
                pmu_set_register_value_hp(PMU_VIO18_ANA_RSV0, 0x1, 10, 0x0);  //Disable PMU dummy load
                log_hal_msgid_info("[PMU_PWR][Dummy_loading]Disable DL PMU_VIO18_ANA_RSV0[%x]", 1, pmu_get_register_value_hp(PMU_VIO18_ANA_RSV0, 0xffff, 0));
            } else {
                pmu_efuse_enable_reading();
                DLH_value = pmu_core_efuse_get_efuse_data_by_address(96);
                DLL_value = pmu_core_efuse_get_efuse_data_by_address(94);
                if (pmu_check_dummy_trim_value(&DLH_value, &DLL_value) != PMU_STATUS_SUCCESS) {
                    break;
                }
                log_hal_msgid_info("[PMU_PWR][Dummy_loading][VIO18]loading_value [%d]", 1, loading_value);
                log_hal_msgid_info("[PMU_PWR][Dummy_loading][VIO18]Efuse H[%d]L[%d]", 2, DLH_value, DLL_value);
                dummy_loading_str.vio18_val = ((loading_value - (DLL_value - ((DLH_value - DLL_value) / 30))) / ((DLH_value - DLL_value) / 30));
                dummy_loading_str.vio18_val = (dummy_loading_str.vio18_val + 0.5);
                log_hal_msgid_info("[PMU_PWR][Dummy_loading]dummy_loading_str.vio18_val[%x]", 1, dummy_loading_str.vio18_val);
                pmu_set_register_value_hp(PMU_VIO18_ANA_RSV0, 0x1, 10, 0x1);  //Enable PMU dummy load
                pmu_set_register_value_hp(PMU_VIO18_ANA_RSV0, 0x1f, 11, dummy_loading_str.vio18_val);
                log_hal_msgid_info("[PMU_PWR][Dummy_loading]PMU_VIO18_ANA_RSV0[%x]", 1, pmu_get_register_value_hp(PMU_VIO18_ANA_RSV0, 0xffff, 0));
            }

            break;
        case PMU_BUCK_VRF:
            if (loading_value == 0) {
                pmu_set_register_value_hp(PMU_VRF_ANA_RSV0, 0x1, 12, 0x0);  //Disable PMU dummy load
                log_hal_msgid_info("[PMU_PWR][Dummy_loading]Disable DL PMU_VRF_ANA_RSV0[%x]", 1, pmu_get_register_value_hp(PMU_VRF_ANA_RSV0, 0xffff, 0));
            } else {
                pmu_efuse_enable_reading();
                DLH_value = pmu_core_efuse_get_efuse_data_by_address(100);
                DLL_value = pmu_core_efuse_get_efuse_data_by_address(98);
                if (pmu_check_dummy_trim_value(&DLH_value, &DLL_value) != PMU_STATUS_SUCCESS) {
                    break;
                }
                log_hal_msgid_info("[PMU_PWR][Dummy_loading][VRF]loading_value [%d]", 1, loading_value);
                log_hal_msgid_info("[PMU_PWR][Dummy_loading][VRF]Efuse H[%d]L[%d]", 2, DLH_value, DLL_value);
                dummy_loading_str.vrf_val = ((loading_value - (DLL_value - ((DLH_value - DLL_value) / 30))) / ((DLH_value - DLL_value) / 30));
                dummy_loading_str.vrf_val = (dummy_loading_str.vrf_val + 0.5);
                log_hal_msgid_info("[PMU_PWR][Dummy_loading]dummy_loading_str.vrf_val[%x]", 1, dummy_loading_str.vrf_val);
                h_value = (dummy_loading_str.vrf_val >> 2);
                l_value = (dummy_loading_str.vrf_val & 0x3);
                log_hal_msgid_info("[PMU_PWR][Dummy_loading]Efuse H[%d]L[%d]", 2, h_value, l_value);
                pmu_set_register_value_hp(PMU_VRF_ANA_RSV0, 0x1, 12, 0x1);  //Enable PMU dummy load
                pmu_set_register_value_hp(PMU_VRF_ANA_RSV0, 0x3, 10, l_value);
                pmu_set_register_value_hp(PMU_VRF_ANA_RSV0, 0x7, 13, h_value);
                log_hal_msgid_info("[PMU_PWR][Dummy_loading]PMU_VRF_ANA_RSV0[%x]", 1, pmu_get_register_value_hp(PMU_VRF_ANA_RSV0, 0xffff, 0));
            }

            break;
        case PMU_BUCK_VAUD18:
            if (loading_value == 0) {
                pmu_set_register_value_hp(PMU_VAUD18_ANA_RSV0, 0x1, 10, 0x0);  //Disable PMU dummy load
                log_hal_msgid_info("[PMU_PWR][Dummy_loading]Disable DL PMU_VAUD18_ANA_RSV0[%x]", 1, pmu_get_register_value_hp(PMU_VAUD18_ANA_RSV0, 0xffff, 0));
            } else {
                pmu_efuse_enable_reading();
                DLH_value = pmu_core_efuse_get_efuse_data_by_address(92);
                DLL_value = pmu_core_efuse_get_efuse_data_by_address(90);
                if (pmu_check_dummy_trim_value(&DLH_value, &DLL_value) != PMU_STATUS_SUCCESS) {
                    break;
                }
                log_hal_msgid_info("[PMU_PWR][Dummy_loading][VAUD18]loading_value [%d]", 1, loading_value);
                log_hal_msgid_info("[PMU_PWR][Dummy_loading][VAUD18]Efuse H[%d]L[%d]", 2, DLH_value, DLL_value);
                dummy_loading_str.vaud18_val = ((loading_value - (DLL_value - ((DLH_value - DLL_value) / 30))) / ((DLH_value - DLL_value) / 30));
                dummy_loading_str.vaud18_val = (dummy_loading_str.vaud18_val + 0.5);
                log_hal_msgid_info("[PMU_PWR][Dummy_loading]dummy_loading_str.vaud18_val[%x]", 1, dummy_loading_str.vaud18_val);
                pmu_set_register_value_hp(PMU_VAUD18_ANA_RSV0, 0x1, 10, 0x1);  //Enable PMU dummy load
                pmu_set_register_value_hp(PMU_VAUD18_ANA_RSV0, 0x1f, 11, dummy_loading_str.vaud18_val);
                log_hal_msgid_info("[PMU_PWR][Dummy_loading]PMU_VAUD18_ANA_RSV0[%x]", 1, pmu_get_register_value_hp(PMU_VAUD18_ANA_RSV0, 0xffff, 0));
            }
            break;
        case PMU_BUCK_VPA:
            if (loading_value == 0) {
                pmu_set_register_value_hp(PMU_VPA_ANA_RSV0, 0x1, 13, 0x0);  //Disable PMU dummy load
                log_hal_msgid_info("[PMU_PWR][Dummy_loading]Disable DL PMU_VRF_ANA_RSV0[%x]", 1, pmu_get_register_value_hp(PMU_VRF_ANA_RSV0, 0xffff, 0));
            } else {
                pmu_efuse_enable_reading();
                DLH_value = pmu_core_efuse_get_efuse_data_by_address(104);
                DLL_value = pmu_core_efuse_get_efuse_data_by_address(102);
                if (pmu_check_dummy_trim_value(&DLH_value, &DLL_value) != PMU_STATUS_SUCCESS) {
                    break;
                }
                log_hal_msgid_info("[PMU_PWR][Dummy_loading][VPA]loading_value [%d]", 1, loading_value);
                log_hal_msgid_info("[PMU_PWR][Dummy_loading][VPA]Efuse H[%d]L[%d]", 2, DLH_value, DLL_value);
                dummy_loading_str.vpa_val = ((loading_value - (DLL_value - ((DLH_value - DLL_value) / 30))) / ((DLH_value - DLL_value) / 30));
                dummy_loading_str.vpa_val = (dummy_loading_str.vpa_val + 0.5);
                log_hal_msgid_info("[PMU_PWR][Dummy_loading]dummy_loading_str.vpa_val[%x]", 1, dummy_loading_str.vpa_val);
                h_value = ((dummy_loading_str.vpa_val & 0x10) >> 4);
                m_value = ((dummy_loading_str.vpa_val & 0xC) >> 2);
                l_value = (dummy_loading_str.vpa_val & 0x3);
                log_hal_msgid_info("[PMU_PWR][Dummy_loading]Efuse H[%d]M[%d]L[%d]", 3, h_value, m_value, l_value);
                pmu_set_register_value_hp(PMU_VPA_ANA_RSV0, 0x1, 13, 0x1);  //Enable PMU dummy load
                pmu_set_register_value_hp(PMU_VPA_ANA_RSV0, 0x3, 11, l_value);
                pmu_set_register_value_hp(PMU_VPA_ANA_RSV0, 0x3, 14, m_value);
                pmu_set_register_value_hp(PMU_VPA_ANA_RSV0, 0x1, 7, h_value);
                log_hal_msgid_info("[PMU_PWR][Dummy_loading]PMU_VRF_ANA_RSV0[%x]", 1, pmu_get_register_value_hp(PMU_VPA_ANA_RSV0, 0xffff, 0));
            }
            break;
    }
    pmu_efuse_disable_reading();
}

#ifdef PMU_SLT_ENV
void pmu_set_vaud18_mode(uint8_t val, uint8_t mode)
{
    pmu_set_register_value_hp(PMU_VAUD18DRV_MODE, RG_VAUD18DRV_VOSEL0_HW_MODE_MASK, RG_VAUD18DRV_VOSEL1_HW_MODE_SHIFT, val);
    pmu_set_register_value_hp(PMU_VAUD18DRV_MODE, RG_VAUD18DRV_VOSEL0_HW_MODE_MASK, RG_VAUD18DRV_VOSEL1_HW_MODE_SHIFT, val);
    switch (mode) {
        case 0:
            pmu_set_register_value_hp(PMU_VAUD18DRV_MODE, RG_VAUD18DRV_VOSEL0_SW_MASK, RG_VAUD18DRV_VOSEL0_SW_SHIFT, 0x0);
            pmu_set_register_value_hp(PMU_VAUD18DRV_MODE, RG_VAUD18DRV_VOSEL1_SW_MASK, RG_VAUD18DRV_VOSEL1_SW_SHIFT, 0x0);
            log_hal_msgid_info("[PMU_SLT]Vaud18 L", 0);
            break;
        case 1:
            pmu_set_register_value_hp(PMU_VAUD18DRV_MODE, RG_VAUD18DRV_VOSEL0_SW_MASK, RG_VAUD18DRV_VOSEL0_SW_SHIFT, 0x1);
            pmu_set_register_value_hp(PMU_VAUD18DRV_MODE, RG_VAUD18DRV_VOSEL1_SW_MASK, RG_VAUD18DRV_VOSEL1_SW_SHIFT, 0x0);
            log_hal_msgid_info("[PMU_SLT]Vaud18 M", 0);
            break;
        case 3:
            pmu_set_register_value_hp(PMU_VAUD18DRV_MODE, RG_VAUD18DRV_VOSEL0_SW_MASK, RG_VAUD18DRV_VOSEL0_SW_SHIFT, 0x1);
            pmu_set_register_value_hp(PMU_VAUD18DRV_MODE, RG_VAUD18DRV_VOSEL1_SW_MASK, RG_VAUD18DRV_VOSEL1_SW_SHIFT, 0x1);
            log_hal_msgid_info("[PMU_SLT]Vaud18 H", 0);
            break;
    }

}
uint32_t pmu_modify_4bit_trim_hp(pmu_slt_mode_t oper, uint32_t buck_value, uint32_t divv_value)
{
    log_hal_msgid_info("[PMU_SLT]4bit[%d][%x][%x]", 3, oper, buck_value, divv_value);
    uint32_t temp = 0;
    if (oper == PMU_HIGH_LEVEL) { //oper==2 high
        if (buck_value >= 0x8) {
            temp = buck_value - divv_value;
            if (temp <= 0x8) {
                temp = 0x8;
            }
        } else if (buck_value == 0) {
            temp = 0xf - divv_value;
            if (temp <= 0x8) {
                temp = 0x8;
            }
        } else {
            temp = buck_value - divv_value;
            if (temp <= 0) {
                temp = temp + 0x10;
            }
        }
    } else if (oper == PMU_LOW_LEVEL) {  //oper ==0 low
        if (buck_value >= 0x8) {
            temp = buck_value + divv_value;
            if (temp >= 0xf) {
                temp = temp - 0x10;
            }
        } else if (buck_value == 0) {
            temp = divv_value;
            if (temp >= 0x7) {
                temp = 0x7;
            }
        } else {
            temp = buck_value + divv_value;
            if (temp >= 0x7) {
                temp = 0x7;
            }
        }
    } else {
        printf("Normal");
    }
    return temp;
}
uint32_t pmu_modify_5bit_trim_hp(pmu_slt_mode_t oper, uint32_t buck_value, uint32_t divv_value)
{
    log_hal_msgid_info("[PMU_SLT]5bit[%d][%x][%x]", 3, oper, buck_value, divv_value);
    uint32_t temp = 0;
    if (oper == PMU_HIGH_LEVEL) { //oper==2 high
        if (buck_value >= 0xF) {
            temp = buck_value - divv_value;
            if (temp <= 0xF) {
                temp = 0xF;
            }
        } else if (buck_value == 0) {
            temp = 0x20 - divv_value;
            if (temp <= 0xf) {
                temp = 0xf;
            }
        } else {
            temp = buck_value - divv_value;
            if (temp <= 0) {
                temp = temp + 0x20;
            }
        }
    } else if (oper == PMU_LOW_LEVEL) {  //oper ==0 low
        if (buck_value >= 0xf) {
            temp = buck_value + divv_value;
            if (temp >= 0xf) {
                temp = temp - 0x20;
            }
        } else if (buck_value == 0) {
            temp = divv_value;
            if (temp >= 0xf) {
                temp = 0xf;
            }
        } else {
            temp = buck_value + divv_value;
            if (temp >= 0xf) {
                temp = 0xf;
            }
        }
    } else {
        printf("Normal");
    }
    return temp;
}
void pmu_switch_output_mode_hp(pmu_power_domain_t pmu_pdm, uint32_t divv_value, pmu_slt_mode_t oper)
{
    uint32_t temp_value = 0;
    switch (pmu_pdm) {
        case PMU_BUCK_VCORE:
            temp_value = pmu_get_register_value_hp(PMU_BUCK_VCORE_CTRL4, RG_BUCK_VCORE_VSEL_MASK, RG_BUCK_VCORE_VSEL_SHIFT);
            log_hal_msgid_info("[PMU_SLT]PMU_BUCK_VCORE B:%x", 1, temp_value);
            if (oper == PMU_HIGH_LEVEL) {
                temp_value += divv_value;
            } else if (oper == PMU_LOW_LEVEL) {
                temp_value -= (divv_value - 3);
            } else {
            }
            pmu_set_register_value_hp(PMU_BUCK_VCORE_CTRL4, RG_BUCK_VCORE_VSEL_MASK, RG_BUCK_VCORE_VSEL_SHIFT, temp_value);
            log_hal_msgid_info("[PMU_SLT]PMU_BUCK_VCORE A:%x", 1, temp_value);
            break;
        case PMU_BUCK_VIO18:
            temp_value = pmu_get_register_value_hp(PMU_BUCK_VIO18_CTRL4, RG_BUCK_VIO18_VSEL_MASK, RG_BUCK_VIO18_VSEL_SHIFT);
            log_hal_msgid_info("[PMU_SLT]PMU_BUCK_VIO18 B:%x", 1, temp_value);
            if (oper == PMU_HIGH_LEVEL) {
                temp_value += divv_value;
            } else if (oper == PMU_LOW_LEVEL) {
                temp_value -= divv_value;
            } else {
            }
            pmu_set_register_value_hp(PMU_BUCK_VIO18_CTRL4, RG_BUCK_VIO18_VSEL_MASK, RG_BUCK_VIO18_VSEL_SHIFT, temp_value);
            log_hal_msgid_info("[PMU_SLT]PMU_BUCK_VIO18 A:%x", 1, temp_value);
            break;
        case PMU_BUCK_VRF:
            temp_value = pmu_get_register_value_hp(PMU_BUCK_VRF_CTRL4, RG_BUCK_VRF_VSEL_MASK, RG_BUCK_VRF_VSEL_SHIFT);
            log_hal_msgid_info("[PMU_SLT]PMU_BUCK_VRF B:%x", 1, temp_value);
            if (oper == PMU_HIGH_LEVEL) {
                temp_value += divv_value;
            } else if (oper == PMU_LOW_LEVEL) {
                temp_value -= divv_value;
            } else {
            }
            pmu_set_register_value_hp(PMU_BUCK_VRF_CTRL4, RG_BUCK_VRF_VSEL_MASK, RG_BUCK_VRF_VSEL_SHIFT, temp_value);
            log_hal_msgid_info("[PMU_SLT]PMU_BUCK_VRF A:%x", 1, temp_value);
            break;
        case PMU_BUCK_VPA:
            temp_value = pmu_get_register_value_hp(PMU_BUCK_VPA_CTRL4, RG_BUCK_VPA_VOUTSEL_MASK, RG_BUCK_VPA_VOUTSEL_SHIFT);
            uint32_t temp_trim = pmu_get_register_value_hp(PMU_SMPS_ELR_1, RG_VPA_TRIMH_MASK, RG_VPA_TRIMH_SHIFT);
            log_hal_msgid_info("[PMU_SLT]PMU_BUCK_VPA B:%x", 1, temp_trim);
            if (temp_value <= 0x6) { //1.2~1.4V
                if (oper == PMU_HIGH_LEVEL) {
                    temp_trim -= 6;
                } else if (oper == PMU_LOW_LEVEL) {
                    temp_trim += 4;
                } else {}
            } else if (temp_value == 0x8) { //1.6V
                if (oper == PMU_HIGH_LEVEL) {
                    temp_trim -= 5;
                } else if (oper == PMU_LOW_LEVEL) {
                    temp_trim += 5;
                } else {}
            } else if (temp_value == 0xA) { //1.8V
                if (oper == PMU_HIGH_LEVEL) {
                    temp_trim -= 4;

                } else if (oper == PMU_LOW_LEVEL) {
                    temp_trim += 6;
                } else {}
            } else if (temp_value == 0xE) { //2.2V
                if (oper == PMU_HIGH_LEVEL) {
                    temp_trim -= 3;
                } else if (oper == PMU_LOW_LEVEL) {
                    temp_trim += 7;
                } else {}
            } else {
            }
            if (temp_trim >= 0x1f) {
                temp_trim = 0x1f;
            } else if (temp_trim <= 0) {
                temp_trim = 0;
            } else {}
            pmu_set_register_value_hp(PMU_SMPS_ELR_1, RG_VPA_TRIMH_MASK, RG_VPA_TRIMH_SHIFT, temp_trim);
            log_hal_msgid_info("[PMU_SLT]PMU_BUCK_VPA A:%x", 1, temp_trim);
            break;
        case PMU_BUCK_VAUD18:
            temp_value = pmu_get_register_value_hp(PMU_BUCK_VAUD18_CTRL2, RG_BUCK_VAUD18_VSEL_H_MASK, RG_BUCK_VAUD18_VSEL_H_SHIFT);
            log_hal_msgid_info("[PMU_SLT]PMU_BUCK_VAUD18 H B:%x", 1, temp_value);
            if (oper) {
                temp_value += 6;
            } else {
                temp_value -= 6;
            }
            pmu_set_register_value_hp(PMU_BUCK_VAUD18_CTRL2, RG_BUCK_VAUD18_VSEL_H_MASK, RG_BUCK_VAUD18_VSEL_H_SHIFT, temp_value);
            log_hal_msgid_info("[PMU_SLT]PMU_BUCK_VAUD18 H A:%x", 1, temp_value);
            /*************************************************************************/
            temp_value = pmu_get_register_value_hp(PMU_BUCK_VAUD18_CTRL3, RG_BUCK_VAUD18_VSEL_M_MASK, RG_BUCK_VAUD18_VSEL_M_SHIFT);
            log_hal_msgid_info("[PMU_SLT]PMU_BUCK_VAUD18 M B:%x", 1, temp_value);
            if (oper) {
                temp_value += 4;
            } else {
                temp_value -= 4;
            }
            pmu_set_register_value_hp(PMU_BUCK_VAUD18_CTRL3, RG_BUCK_VAUD18_VSEL_M_MASK, RG_BUCK_VAUD18_VSEL_M_SHIFT, temp_value);
            log_hal_msgid_info("[PMU_SLT]PMU_BUCK_VAUD18 M A:%x", 1, temp_value);
            /*************************************************************************/
            temp_value = pmu_get_register_value_hp(PMU_BUCK_VAUD18_CTRL4, RG_BUCK_VAUD18_VSEL_L_MASK, RG_BUCK_VAUD18_VSEL_L_SHIFT);
            log_hal_msgid_info("[PMU_SLT]PMU_BUCK_VAUD18 L B:%x", 1, temp_value);
            if (oper) {
                temp_value += 3;
            } else {
                temp_value -= 3;
            }
            pmu_set_register_value_hp(PMU_BUCK_VAUD18_CTRL4, RG_BUCK_VAUD18_VSEL_L_MASK, RG_BUCK_VAUD18_VSEL_L_SHIFT, temp_value);
            log_hal_msgid_info("[PMU_SLT]PMU_BUCK_VAUD18 L A:%x", 1, temp_value);
            break;
        case PMU_LDO_VSRAM:
            pmu_set_register_value_hp(PMU_LDOTOP_ELR_3, RG_VSRAM_VOTRIM_MASK, RG_VSRAM_VOTRIM_SHIFT, vsram_trim);
            if (0x50 >= pmu_get_register_value_hp(PMU_BUCK_VCORE_CTRL4, RG_BUCK_VCORE_VSEL_MASK, RG_BUCK_VCORE_VSEL_SHIFT)) {
                if (oper) {
                    pmu_set_vsram_voltage_hp(VSRAM_VOLTAGE_0P84);
                } else {
                    pmu_set_vsram_voltage_hp(VSRAM_VOLTAGE_0P78);
                    temp_value = vsram_trim;
                    log_hal_msgid_info("[PMU_SLT]PMU_LDO_VSRAM trim B: %x", 1, temp_value);
                    temp_value = pmu_modify_5bit_trim_hp(oper, temp_value, divv_value);
                    pmu_set_register_value_hp(PMU_LDOTOP_ELR_3, RG_VSRAM_VOTRIM_MASK, RG_VSRAM_VOTRIM_SHIFT, temp_value);
                    log_hal_msgid_info("[PMU_SLT]PMU_LDO_VSRAM trim A: %x", 1, temp_value);
                }
            } else {
                if (oper) {
                    pmu_set_vsram_voltage_hp(VSRAM_VOLTAGE_0P97);
                } else {
                    pmu_set_vsram_voltage_hp(VSRAM_VOLTAGE_0P88);
                }
            }
            break;

        case PMU_LDO_VLDO31:
            log_hal_msgid_info("[PMU_SLT]PMU_LDO_VLDO31", 0);
            temp_value = pmu_get_register_value_hp(PMU_LDOTOP_ELR_0, RG_VLDO31_VOTRIM_MASK, RG_VLDO31_VOTRIM_SHIFT);
            log_hal_msgid_info("[PMU_SLT]PMU_LDO_VLDO31 B: %x", 1, temp_value);
            temp_value = pmu_modify_4bit_trim_hp(oper, temp_value, divv_value);
            log_hal_msgid_info("[PMU_SLT]PMU_LDO_VLDO31 A: %x", 1, temp_value);
            pmu_set_register_value_hp(PMU_LDOTOP_ELR_0, RG_VLDO31_VOTRIM_MASK, RG_VLDO31_VOTRIM_SHIFT, temp_value);
            break;
        case PMU_VDIG18:
            log_hal_msgid_info("[PMU_SLT]PMU_VDIG18", 0);
            temp_value = pmu_get_register_value_hp(PMU_VDIG18_ELR_0, RG_VDIG18_VOTRIM_MASK, RG_VDIG18_VOTRIM_SHIFT);
            log_hal_msgid_info("[PMU_SLT]PMU_VDIG18, B: %x", 1, temp_value);
            temp_value = pmu_modify_4bit_trim_hp(oper, temp_value, divv_value);
            log_hal_msgid_info("[PMU_SLT]PMU_VDIG18, A: %x", 1, temp_value);
            pmu_set_register_value_hp(PMU_VDIG18_ELR_0, RG_VDIG18_VOTRIM_MASK, RG_VDIG18_VOTRIM_SHIFT, temp_value);
            break;
        case PMU_LDO_VA18:
            log_hal_msgid_info("[PMU_SLT]PMU_LDO_VA18", 0);
            temp_value = pmu_get_register_value_hp(PMU_LDOTOP_ELR_1, RG_VA18_VOTRIM_MASK, RG_VA18_VOTRIM_SHIFT);
            log_hal_msgid_info("[PMU_SLT]PMU_LDO_VA18, B: %x", 1, temp_value);
            temp_value = pmu_modify_4bit_trim_hp(oper, temp_value, divv_value);
            log_hal_msgid_info("[PMU_SLT]PMU_LDO_VA18, A: %x", 1, temp_value);
            pmu_set_register_value_hp(PMU_LDOTOP_ELR_1, RG_VA18_VOTRIM_MASK, RG_VA18_VOTRIM_SHIFT, temp_value);
            break;
    }
}

void pmu_performance_level(pmu_slt_mode_t mode, uint32_t divv_value)
{
    //divv_value = 0 mean is fixed value
    switch (mode) {
        case PMU_LOW_LEVEL:
            pmu_switch_output_mode_hp(PMU_BUCK_VCORE, 16, PMU_LOW_LEVEL);
            pmu_switch_output_mode_hp(PMU_BUCK_VIO18, 24, PMU_LOW_LEVEL);
            pmu_switch_output_mode_hp(PMU_BUCK_VRF, 12, PMU_LOW_LEVEL);
            pmu_switch_output_mode_hp(PMU_BUCK_VPA, 0, PMU_LOW_LEVEL);
            pmu_switch_output_mode_hp(PMU_BUCK_VAUD18, 0, PMU_LOW_LEVEL);
            pmu_switch_output_mode_hp(PMU_LDO_VSRAM, 0, PMU_LOW_LEVEL);
            pmu_switch_output_mode_hp(PMU_LDO_VLDO31, 4, PMU_LOW_LEVEL);
            pmu_switch_output_mode_hp(PMU_VDIG18, 9, PMU_LOW_LEVEL);
            //pmu_switch_output_mode(PMU_LDO_VA18, 4, PMU_LOW_LEVEL);
            break;
        case PMU_NORMAL_LEVEL:
            break;
        case PMU_HIGH_LEVEL:
            pmu_switch_output_mode_hp(PMU_LDO_VSRAM, 0, PMU_HIGH_LEVEL);
            pmu_switch_output_mode_hp(PMU_BUCK_VCORE, 16, PMU_HIGH_LEVEL);
            pmu_switch_output_mode_hp(PMU_BUCK_VIO18, 24, PMU_HIGH_LEVEL);
            pmu_switch_output_mode_hp(PMU_BUCK_VRF, 12, PMU_HIGH_LEVEL);
            pmu_switch_output_mode_hp(PMU_BUCK_VPA, 0, PMU_HIGH_LEVEL);
            pmu_switch_output_mode_hp(PMU_BUCK_VAUD18, 0, PMU_HIGH_LEVEL);

            pmu_switch_output_mode_hp(PMU_LDO_VLDO31, 6, PMU_HIGH_LEVEL);
            pmu_switch_output_mode_hp(PMU_VDIG18, 9, PMU_HIGH_LEVEL);
            //pmu_switch_output_mode(PMU_LDO_VA18, 4, PMU_HIGH_LEVEL);
            break;
    }
}
#endif

void pmu_dump_otp_hp(void)
{
    log_hal_msgid_info("[PMU_CAL]pmu_dump_otp_hp, adie high dosen't use otp", 0);
}

#ifdef AIR_NVDM_ENABLE
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "hal_pmu_nvkey_struct.h"

/*
#define PMU_GET_U08(addr, offset) (*((uint8_t  *)(addr + offset)))
#define PMU_GET_U16(addr, offset) (*((uint16_t *)(addr + offset)))
#define PMU_GET_U32(addr, offset) (*((uint32_t *)(addr + offset)))
*/

pmu_status_t pmu_get_nvkey(uint16_t id, uint8_t *ptr, uint32_t size)
{
    uint32_t len = 0;
    nvkey_status_t status = nvkey_data_item_length(id, &len);
    if ((status != NVKEY_STATUS_OK) || (len != size)) {
        log_hal_msgid_error("[PMU_CAL]pmu_cal, get_nvkey fail, id[0x%X], status[%d], len[%d], size[%d]", 4,
                            id, status, len, size);
        return PMU_STATUS_ERROR;
    }
    status = nvkey_read_data(id, ptr, &len);
    if (status != NVKEY_STATUS_OK) {
        log_hal_msgid_error("[PMU_CAL]pmu_cal, get_nvkey fail, id[0x%X], status[%d]", 2, id, status);
        return PMU_STATUS_ERROR;
    }
    return PMU_STATUS_SUCCESS;
}

void pmu_dump_nvkey_hp(void)
{
    nvid_cal_general_cfg_t nvid_cal_general_cfg;
    nvid_cal_chg_ctl_cfg_t nvid_cal_chg_ctl_cfg;
    nvid_cal_bat_mgmt_basic_t nvid_cal_bat_mgmt_basic;
    nvid_cal_bat_mgmt_chr_t nvid_cal_bat_mgmt_chr;

    uint16_t addr;
    pmu_status_t status;

    addr = NVID_CAL_GENERAL_CFG;
    status = pmu_get_nvkey(addr, (uint8_t *)&nvid_cal_general_cfg, sizeof(nvid_cal_general_cfg));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp, addr[0x%04X] NVID_CAL_GENERAL_CFG", 1, addr);
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp,   PWRHOLD_Check[%d], VBUS debounce[%d]",
                           2, nvid_cal_general_cfg.pwrhold_check, nvid_cal_general_cfg.vbus_debounce);
    }

    addr = NVID_CAL_BAT_MGMT_BASIC;
    status = pmu_get_nvkey(addr, (uint8_t *)&nvid_cal_bat_mgmt_basic, sizeof(nvid_cal_bat_mgmt_basic));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp, addr[0x%04X] NVID_CAL_BAT_MGMT_BASIC", 1, addr);
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp,   "
                           "field_0[%d], field_1[%d], field_2[%d], "
                           "field_3[%d], field_4[%d], field_5[%d]",
                           6,
                           nvid_cal_bat_mgmt_basic.field_arr[0], nvid_cal_bat_mgmt_basic.field_arr[1], nvid_cal_bat_mgmt_basic.field_arr[2],
                           nvid_cal_bat_mgmt_basic.field_arr[3], nvid_cal_bat_mgmt_basic.field_arr[4], nvid_cal_bat_mgmt_basic.field_arr[5]);
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp,   "
                           "field_6[%d], field_7[%d], field_8[%d], "
                           "field_9[%d], field_10[%d], field_11[%d]",
                           6,
                           nvid_cal_bat_mgmt_basic.field_arr[6], nvid_cal_bat_mgmt_basic.field_arr[7], nvid_cal_bat_mgmt_basic.field_arr[8],
                           nvid_cal_bat_mgmt_basic.field_arr[9], nvid_cal_bat_mgmt_basic.field_arr[10], nvid_cal_bat_mgmt_basic.field_arr[11]);
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp,   "
                           "full_bat: volt[%d], adc[%d], offset_volt[%d], offset_adc[%d]",
                           4,
                           nvid_cal_bat_mgmt_basic.full_bat_voltage, nvid_cal_bat_mgmt_basic.full_bat_adc,
                           nvid_cal_bat_mgmt_basic.full_bat_voltage_offset, nvid_cal_bat_mgmt_basic.full_bat_adc_offset);
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp,   "
                           "recharge: volt[%d], adc[%d], offset_volt[%d], offset_adc[%d]",
                           4,
                           nvid_cal_bat_mgmt_basic.recharge_voltage, nvid_cal_bat_mgmt_basic.recharge_adc,
                           nvid_cal_bat_mgmt_basic.recharge_voltage_offset, nvid_cal_bat_mgmt_basic.recharge_adc_offset);
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp,   "
                           "PRECC CurrentSetting[%d], CV Termination Current[%d], "
                           "S1 volt[%d], multi lvl[%d], S2 volt[%d], multi lvl[%d]",
                           6,
                           nvid_cal_bat_mgmt_basic.precc_current_setting, nvid_cal_bat_mgmt_basic.cv_termination_current, nvid_cal_bat_mgmt_basic.s1_voltage,
                           nvid_cal_bat_mgmt_basic.s1_multi_level, nvid_cal_bat_mgmt_basic.s2_voltage, nvid_cal_bat_mgmt_basic.s2_multi_level);
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp,   "
                           "cool charger current[%d], cool cv flag[%d], "
                           "warm charger current[%d], warm cv flag[%d]",
                           4,
                           nvid_cal_bat_mgmt_basic.cool_charger_current, nvid_cal_bat_mgmt_basic.cool_cv_flag,
                           nvid_cal_bat_mgmt_basic.warm_charger_current, nvid_cal_bat_mgmt_basic.warm_cv_flag);
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp,   "
                           "iterm irq[%d], S0 voltage[%d], S0 current[%d], Batterycategory[%d]",
                           4,
                           nvid_cal_bat_mgmt_basic.iterm_irq, nvid_cal_bat_mgmt_basic.s0_voltage,
                           nvid_cal_bat_mgmt_basic.s0_current, nvid_cal_bat_mgmt_basic.battery_category);
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp,   2-step: "
                           "Iterm_IRQ_1[%d], CV_1[%d], Iterm_IRQ_2[%d], CV_2[%d], "
                           "CV Enable[%d], CC Enable[%d]",
                           6,
                           nvid_cal_bat_mgmt_basic._2step_iterm_irq1, nvid_cal_bat_mgmt_basic._2step_cv1,
                           nvid_cal_bat_mgmt_basic._2step_iterm_irq2, nvid_cal_bat_mgmt_basic._2step_cv2,
                           nvid_cal_bat_mgmt_basic._2step_cv_enable, nvid_cal_bat_mgmt_basic._2step_cc_enable);
    }

    addr = NVID_CAL_BAT_MGMT_CHR;
    status = pmu_get_nvkey(addr, (uint8_t *)&nvid_cal_bat_mgmt_chr, sizeof(nvid_cal_bat_mgmt_chr));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp, addr[0x%04X] NVID_CAL_BAT_MGMT_CHR", 1, addr);
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp,   "
                           "K flag[%d], initial bat[%d], initial bat ADC[%d], "
                           "low bat[%d], low bat ADC[%d], Shutdown bat[%d], Shutdown bat ADC[%d]",
                           7,
                           nvid_cal_bat_mgmt_chr.k_flag, nvid_cal_bat_mgmt_chr.initial_bat, nvid_cal_bat_mgmt_chr.initial_bat_adc,
                           nvid_cal_bat_mgmt_chr.low_bat, nvid_cal_bat_mgmt_chr.low_bat_adc,
                           nvid_cal_bat_mgmt_chr.shutdown_bat, nvid_cal_bat_mgmt_chr.shutdown_bat_adc);
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp,   "
                           "chk1_volt[%d], chk1_adc[%d], chk2_volt[%d], chk2_adc[%d], chk3_volt[%d], chk3_adc[%d]",
                           6,
                           nvid_cal_bat_mgmt_chr.check_point_arr[0].check_point, nvid_cal_bat_mgmt_chr.check_point_arr[0].adc,
                           nvid_cal_bat_mgmt_chr.check_point_arr[1].check_point, nvid_cal_bat_mgmt_chr.check_point_arr[1].adc,
                           nvid_cal_bat_mgmt_chr.check_point_arr[2].check_point, nvid_cal_bat_mgmt_chr.check_point_arr[2].adc);
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp,   "
                           "chk4_volt[%d], chk4_adc[%d], chk5_volt[%d], chk5_adc[%d], chk6_volt[%d], chk6_adc[%d]",
                           6,
                           nvid_cal_bat_mgmt_chr.check_point_arr[3].check_point, nvid_cal_bat_mgmt_chr.check_point_arr[3].adc,
                           nvid_cal_bat_mgmt_chr.check_point_arr[4].check_point, nvid_cal_bat_mgmt_chr.check_point_arr[4].adc,
                           nvid_cal_bat_mgmt_chr.check_point_arr[5].check_point, nvid_cal_bat_mgmt_chr.check_point_arr[5].adc);
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp,   "
                           "chk7_volt[%d], chk7_adc[%d], chk8_volt[%d], chk8_adc[%d], chk9_volt[%d], chk9_adc[%d]",
                           6,
                           nvid_cal_bat_mgmt_chr.check_point_arr[6].check_point, nvid_cal_bat_mgmt_chr.check_point_arr[6].adc,
                           nvid_cal_bat_mgmt_chr.check_point_arr[7].check_point, nvid_cal_bat_mgmt_chr.check_point_arr[7].adc,
                           nvid_cal_bat_mgmt_chr.check_point_arr[8].check_point, nvid_cal_bat_mgmt_chr.check_point_arr[8].adc);
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp,   "
                           "rsv1_volt[%d], rsv1_adc[%d]",
                           2,
                           nvid_cal_bat_mgmt_chr.rsv1, nvid_cal_bat_mgmt_chr.rsv1_adc);
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp,   "
                           "Cold[%d], cool_cc[%d], COOL[%d], cool_cv[%d],",
                           4,
                           nvid_cal_bat_mgmt_chr.cold, nvid_cal_bat_mgmt_chr.cool_cc,
                           nvid_cal_bat_mgmt_chr.cool, nvid_cal_bat_mgmt_chr.cool_cv);
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp,   "
                           "WARM[%d], warm_cc[%d], HOT[%d], warm_cv[%d],",
                           4,
                           nvid_cal_bat_mgmt_chr.warm, nvid_cal_bat_mgmt_chr.warm_cc,
                           nvid_cal_bat_mgmt_chr.hot, nvid_cal_bat_mgmt_chr.warm_cv);
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp,   "
                           "rsv6[%d], rsv6_adc[%d], rsv7[%d], rsv7_adc[%d], "
                           "rsv8[%d], rsv8_adc[%d], rsv9[%d], rsv9_adc[%d]",
                           8,
                           nvid_cal_bat_mgmt_chr.rsv6, nvid_cal_bat_mgmt_chr.rsv6_adc, nvid_cal_bat_mgmt_chr.rsv7, nvid_cal_bat_mgmt_chr.rsv7_adc,
                           nvid_cal_bat_mgmt_chr.rsv8, nvid_cal_bat_mgmt_chr.rsv8_adc, nvid_cal_bat_mgmt_chr.rsv9, nvid_cal_bat_mgmt_chr.rsv9_adc);
    }

    addr = NVID_CAL_CHG_CTL_CFG;
    status = pmu_get_nvkey(addr, (uint8_t *)&nvid_cal_chg_ctl_cfg, sizeof(nvid_cal_chg_ctl_cfg));
    if (status == PMU_STATUS_SUCCESS) {
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp, addr[0x%04X] NVID_CAL_CHG_CTL_CFG", 1, addr);
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp,   "
                           "BC1.2 support[%d], ICL config[%d], JEITA EN[%d], Warm/Cool EN[%d]",
                           4,
                           nvid_cal_chg_ctl_cfg.bc12_support, nvid_cal_chg_ctl_cfg.icl_config,
                           nvid_cal_chg_ctl_cfg.jeita_en, nvid_cal_chg_ctl_cfg.warm_cool_en);
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp,   "
                           "PRECC timer EN[%d], PRECC safety time[%d], FASTCC timer EN[%d], FASTCC safety time[%d]",
                           4,
                           nvid_cal_chg_ctl_cfg.precc_timer_en, nvid_cal_chg_ctl_cfg.precc_safety_time,
                           nvid_cal_chg_ctl_cfg.fastcc_timer_en, nvid_cal_chg_ctl_cfg.fastcc_safety_time);
        log_hal_msgid_info("[PMU_CAL]pmu_dump_nvkey_hp,   Extend charging[%d]",
                           1,
                           nvid_cal_chg_ctl_cfg.extend_charging);
    }
}
#endif  /* AIR_NVDM_ENABLE */

void pmu_dump_rg_hp(void)
{
    uint16_t addr;
    uint16_t value __unused;
    const uint16_t addr_start = PMU_PMU_HWCID;
    const uint16_t addr_end = PMU_LCHR_DIG_RSV;

    /**
     * NOTE: Adie address is 16 bits wide
     */
    for (addr = addr_start; addr <= addr_end; addr += 2) {
        value = pmu_get_register_value_hp(addr, 0xffff, 0);
        log_hal_msgid_info("[PMU_BASIC]pmu_dump_rg_hp, APMU Addr[0x%04X] Value[0x%04X]", 2, addr, value);
    }
}

void pmu_rg_timer_cb(uint16_t *user_data)
{
    uint16_t addr;
    for(addr = addr_start; addr <= addr_end; addr += 2){
        log_hal_msgid_info("[PMU_BASIC]dump_rg, APMU Addr[0x%04X] Value[0x%04X]", 2, addr, pmu_get_register_value_hp(addr, 0xffff, 0));
    }
    hal_gpt_sw_start_timer_ms(rg_chk_timer, rg_time,
                              (hal_gpt_callback_t)pmu_rg_timer_cb, NULL);
}

void pmu_check_rg_timer_hp(uint16_t start, uint16_t end, uint16_t timer)
{
    uint16_t addr;
    hal_gpt_status_t gpt_status;
    if(timer == 0){
        hal_gpt_sw_stop_timer_ms(rg_chk_timer);
        for(addr = start; addr <= end; addr += 2){
            log_hal_msgid_info("[PMU_BASIC]dump_rg, APMU Addr[0x%04X] Value[0x%04X]", 2, addr, pmu_get_register_value_hp(addr, 0xffff, 0));
        }
    }else{
        gpt_status = hal_gpt_sw_get_timer(&rg_chk_timer);
        if (gpt_status == HAL_GPT_STATUS_OK) {
            addr_start = start;
            addr_end = end;
            rg_time = timer;
            hal_gpt_sw_start_timer_ms(rg_chk_timer, rg_time,
                                    (hal_gpt_callback_t)pmu_rg_timer_cb, NULL);
        } else {
            log_hal_msgid_error("[PMU_BASIC]pmu_rg_timer_init, rg_chk_timer fail, status[%d]", 1, gpt_status);
        }
    }
}

void pmu_config_hp(void)
{
#ifdef AIR_NVDM_ENABLE
    // read nvkey and set reset pattern
    pmu_rst_pat_cfg_t rst_pat_cfg;
    pmu_get_nvkey(NVID_PMU_RST_PAT_CFG, (uint8_t *)&rst_pat_cfg, sizeof(rst_pat_cfg));
    log_hal_msgid_info("[PMU_CAL]pmu reset pattern hp, enable[%d], rst_pat[%d], cfg_en[%d], cfg[0x%x][0x%x][0x%x][0x%x]", 7,
                       rst_pat_cfg.enable, rst_pat_cfg.rst_pat, rst_pat_cfg.cfg_en,
                       rst_pat_cfg.cfg.ll_low, rst_pat_cfg.cfg.ll_hgh, rst_pat_cfg.cfg.hh_low, rst_pat_cfg.cfg.hh_hgh);
    if (rst_pat_cfg.enable) {
        pmu_set_register_value_hp(PMU_VBUS_UART_DIG2, RG_RSTPAT_EN_MASK, RG_RSTPAT_EN_SHIFT, 1);
    } else {
        pmu_set_register_value_hp(PMU_VBUS_UART_DIG2, RG_RSTPAT_EN_MASK, RG_RSTPAT_EN_SHIFT, 0);
    }
    if (rst_pat_cfg.rst_pat) {
        log_hal_msgid_info("[PMU_BASIC]VBUS UART project", 0);
        pmu_set_register_value_hp(PMU_CORE_CORE_ANA_CON14, RG_PAT_SRC_SEL1_MASK, RG_PAT_SRC_SEL1_SHIFT, 1); // VBUS UART
    } else {
        log_hal_msgid_info("[PMU_BASIC]VBUS project", 0);
        pmu_set_register_value_hp(PMU_CORE_CORE_ANA_CON14, RG_PAT_SRC_SEL1_MASK, RG_PAT_SRC_SEL1_SHIFT, 0); // VBUS
        pmu_set_register_value_hp(PMU_CORE_CORE_ANA_CON14, RG_PAT_SRC_SEL1_LAT_RELOAD_MASK, RG_PAT_SRC_SEL1_LAT_RELOAD_SHIFT, 0x1);
        pmu_set_register_value_hp(PMU_CORE_CORE_ANA_CON14, RG_PAT_SRC_SEL1_LAT_RELOAD_MASK, RG_PAT_SRC_SEL1_LAT_RELOAD_SHIFT, 0x0);
        pmu_set_register_value_hp(PMU_VBUS_UART_CON1, RG_UART_PSW_EN_MASK, RG_UART_PSW_EN_SHIFT, 0x0);
        pmu_set_register_value_hp(PMU_VBUS_UART_CON1, RG_UART_PSW_EN_LAT_RELOAD_MASK, RG_UART_PSW_EN_LAT_RELOAD_SHIFT, 0x1);
        pmu_set_register_value_hp(PMU_VBUS_UART_CON1, RG_UART_PSW_EN_LAT_RELOAD_MASK, RG_UART_PSW_EN_LAT_RELOAD_SHIFT, 0x0);
    }
    if (rst_pat_cfg.cfg_en) {
        pmu_set_register_value_hp(PMU_VBUS_UART_DIG0, RG_PAT_30MS_LL_LOW_MASK, RG_PAT_30MS_LL_LOW_SHIFT, rst_pat_cfg.cfg.ll_low);
        pmu_set_register_value_hp(PMU_VBUS_UART_DIG0, RG_PAT_30MS_LL_HGH_MASK, RG_PAT_30MS_LL_HGH_SHIFT, rst_pat_cfg.cfg.ll_hgh);
        pmu_set_register_value_hp(PMU_VBUS_UART_DIG0, RG_PAT_30MS_HH_LOW_MASK, RG_PAT_30MS_HH_LOW_SHIFT, rst_pat_cfg.cfg.hh_low);
        pmu_set_register_value_hp(PMU_VBUS_UART_DIG0, RG_PAT_30MS_HH_HGH_MASK, RG_PAT_30MS_HH_HGH_SHIFT, rst_pat_cfg.cfg.hh_hgh);
    }
#endif
}
#endif /* HAL_PMU_MODULE_ENABLED */

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
#include "hal_i2c_master.h"
#include "hal_i2c_master_internal.h"
#include "hal_nvic_internal.h"
#include "assert.h"
#include "syslog.h"
#include "hal_pmu_internal_lp.h"
#include "hal_pmu_charger_lp.h"
#include "hal_pmu_auxadc_lp.h"
#include "hal_pmu_cal_lp.h"
#include "hal_pmu_lp_platform.h"
#include "hal_flash_disk_internal.h"
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


#define UNUSED(x)  ((void)(x))
#define AD_REGEN_DEB             (1<<1)
#define PMU_INT_TIMEOUT          (1000)

#if defined(AIR_1WIRE_ENABLE)
#define PMU_VBUS_UART
#endif

typedef enum {
    PWRKEY_NONE,
    PWRKEY_PRESS,
    PWRKEY_RELEASE,
} pmu_pwrkey_int_t;

volatile int pmu_i2c_init_sta = 0;                                      /* FLAG : init setting flag */
uint8_t pmu_init_flag = 0;                                              /* FLAG : check init setting is done */
pmu_dynamic_debug_t pmu_dynamic_debug = { .b = { .vcore=0, .other=1 } };/* FLAG : default only vcore disabled */
int old_index = 0;                                                      /* DATA : restore old vcore voltage index */
int pmu_switch_case = 0;                                                /* DATA : pmu lock vcore case */
int pk_next = PMU_PK_PRESS;                                             /* DATA : power key default value*/
int pmic_irq0 = -1, pmic_irq1 = -1;                                     /* DATA : restore irq status */
uint8_t audio_mode = 0;                                                 /* DATA : audio class mode */
uint8_t pmu_basic_index = 0;                                            /* DATA : project init basic vocre index */
uint8_t pmu_lock_status = 0;                                            /* DATA : lock status*/
uint8_t pmic_iv_version = 0;                                            /* DATA : PMIC IC version*/
uint8_t event_con0 = 0, event_con1 = 0, event_con2 = 0, event_con3 = 0; /* DATA : restore irq times */
uint16_t power_on_reason = 0;
uint32_t pmu_register_interrupt ;                                       /* DATA : Record irq index 0~31 */
uint32_t pmu_irq_enable_com0 = 0;                                       /* DATA : restore irq enable status,con0 */
uint32_t pmu_irq_enable_com1 = 0;                                       /* DATA : restore irq enable status,con1 */
uint8_t pmu_get_ver_flag = 0;

pmu_function_t pmu_function_table_lp[PMU_INT_MAX_LP];                /* DATA : restore callback function */
static volatile unsigned char Vcore_Resource_Ctrl[7];                   /* resource control : use in pmu_lock_vcore for resource control */
static volatile unsigned int micbias_ldo_Resource[4];                            /* resource control : use in pmu_lock_vcore for resource control */
volatile unsigned int vaud18_vsel_arr[3] = {PMU_VAUD18_0P9_V, PMU_VAUD18_1P3_V, PMU_VAUD18_1P8_V};
static volatile int micbias_vout_arr[3] = {PMU_3VVREF_2P78_V, PMU_3VVREF_2P78_V, PMU_3VVREF_2P78_V};
uint8_t pmu_power_off_reason = 0;
uint8_t pmic_ver = 0xFF;
uint8_t ft_ver = 0x0;
bool rst_adie_clk = TRUE;
uint32_t rg_chk_timer;
uint16_t addr_start, addr_end, rg_time;
#define VAUD18_DEF_SEL           0x73
#define VAUD18_DEF_VOLT          1800
#define VAUD18_VOLT_MIN          650
#define VAUD18_VOLT_MAX          1920
extern uint32_t pmu_fast_buffer_tick;
extern uint8_t hpbg_trim_sel;

#define FAST_BUFFER_DELAY_US     30000
/*==========[BUCK/LDO]==========*/
pmu_power_vcore_voltage_t pmu_lock_vcore_lp(pmu_power_stage_t mode, pmu_power_vcore_voltage_t vol, pmu_lock_parameter_t lock)
{
    if(pmu_init_flag==0){
       pmu_set_vsram_voltage_lp(VSRAM_VOLTAGE_0P92);
    }
    if (pmu_dynamic_debug.b.vcore) {
        log_hal_msgid_info("[PMU_PWR]lock_vcore, old_index[%d], next lock[%d], lock[%d][0:lock 1:unlock]", 3, old_index, vol, lock);
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
        Vcore_Resource_Ctrl[vol]++;
    } else {
        if (Vcore_Resource_Ctrl[vol] != 0) {
            Vcore_Resource_Ctrl[vol]--;
        }
    }
    /* Find Highest Vcore Voltage */
    for (vol_index = PMIC_VCORE_0P85_V; vol_index >= PMIC_VCORE_0P525_V; vol_index--) {
        if (Vcore_Resource_Ctrl[vol_index] != 0) {
            break;
        }
    }
    for (i = PMIC_VCORE_0P525_V; i <= PMIC_VCORE_0P85_V; i++) {
        temp += Vcore_Resource_Ctrl[i];
    }
    if (temp > 25) {
        //log_hal_msgid_error("[PMU_PWR]lock_vcore fail, PMU Lock /unlock isn't match more ten times", 0);
        assert(0);
    }
    pmu_lock_status = temp; /* if not module lock ,return default setting */

    if (old_index < vol_index) { /* rising */
        if ((old_index < PMIC_VCORE_0P65_V) && (vol_index == PMIC_VCORE_0P65_V)) {
            pmu_switch_case = PMIC_RISING_0P55_0P65;
        } else if ((old_index == PMIC_VCORE_0P65_V) && (vol_index > PMIC_VCORE_0P65_V)) {
            pmu_switch_case = PMIC_RISING_0P65_0P8;
        } else {
            pmu_switch_case = PMIC_RISING_0P55_0P8;
        }
    } else if (old_index > vol_index) { /* falling */
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
            log_hal_msgid_info("[PMU_PWR]lock_vcore, same or higher requirements", 0);
        }
        old_index = vol_index;
        hal_nvic_restore_interrupt_mask_special(mask_pri);
        if (pmu_dynamic_debug.b.vcore) {
            log_hal_msgid_info("[PMU_PWR]vcore lock[%d], lock state[%d], switch_case[%d], lock_status[%d]", 4,
                               pmu_get_vcore_voltage_lp(), temp, pmu_switch_case, pmu_lock_status);
        }
        /* old_index == vol_index, do not switch vcore */
        return vol_index;
    }

    switch (pmu_switch_case) {
        case PMIC_RISING_0P55_0P65:
            /* VSRAM */
            pmu_ddie_sram_setting(PMIC_RISING_0P55_0P65, vol_index);
            /* VCORE */
            /* VCORE limiation, Switch to 0.65V first to 0.65V and then wait 150us */
            pmu_select_vcore_voltage_lp(PMU_DVS, PMIC_VCORE_0P65_V);
            hal_gpt_delay_us(150);
            pmu_vcore_ipeak(vol_index);

            break;
        case PMIC_RISING_0P65_0P8:
            /* VSRAM */
            pmu_ddie_sram_setting(PMIC_RISING_0P65_0P8, vol_index);
            /* VCORE */
            pmu_select_vcore_voltage_lp(PMU_DVS, vol_index);
            hal_gpt_delay_us(150);
            pmu_vcore_ipeak(vol_index);
            break;
        case PMIC_RISING_0P55_0P8:
            /* VCORE */
            /* VCORE limiation, Switch to 0.65V first to 0.65V and then wait 150us */
            pmu_select_vcore_voltage_lp(PMU_DVS, PMIC_VCORE_0P65_V);
            hal_gpt_delay_us(150);
            /* VSRAM */
            pmu_ddie_sram_setting(PMIC_RISING_0P55_0P8, vol_index);
            pmu_select_vcore_voltage_lp(PMU_DVS, vol_index);
            hal_gpt_delay_us(150);
            pmu_vcore_ipeak(vol_index);
            break;
        case PMIC_FALLING_0P8_0P65:
            /* VCORE */
            /* Buck_VCORE discharge enable, reduce VCORE waiting time */
            pmu_set_register_value_lp(VCORE_DISQ_SW_ADDR, VCORE_DISQ_SW_MASK, VCORE_DISQ_SW_SHIFT, 0x01);
            pmu_set_register_value_lp(VCORE_DISQ_HW_MODE_ADDR, VCORE_DISQ_HW_MODE_MASK, VCORE_DISQ_HW_MODE_SHIFT, 0x00);
            /* VCORE limiation, Switch to 0.65V first to 0.65V and then wait 150us */
            pmu_select_vcore_voltage_lp(PMU_DVS, PMIC_VCORE_0P65_V);
            hal_gpt_delay_us(150);
            pmu_select_vcore_voltage_lp(PMU_DVS, vol_index);
            hal_gpt_delay_us(150);
            /* Buck_VCORE discharge disable */
            pmu_set_register_value_lp(VCORE_DISQ_SW_ADDR, VCORE_DISQ_SW_MASK, VCORE_DISQ_SW_SHIFT, 0x00);
            pmu_set_register_value_lp(VCORE_DISQ_HW_MODE_ADDR, VCORE_DISQ_HW_MODE_MASK, VCORE_DISQ_HW_MODE_SHIFT, 0x01);
            /* VSRAM */
            pmu_ddie_sram_setting(PMIC_FALLING_0P8_0P65, vol_index);
            pmu_vcore_ipeak(vol_index);
            break;
        case PMIC_FALLING_0P65_0P55:
            /* VCORE */
            pmu_select_vcore_voltage_lp(PMU_DVS, vol_index);
            /* VSRAM */
            pmu_ddie_sram_setting(PMIC_FALLING_0P65_0P55, vol_index);
            pmu_vcore_ipeak(vol_index);
            break;
        case PMIC_FALLING_0P8_0P55:
            /* VCORE */
            /* Buck_VCORE discharge enable, reduce VCORE waiting time */
            pmu_set_register_value_lp(VCORE_DISQ_SW_ADDR, VCORE_DISQ_SW_MASK, VCORE_DISQ_SW_SHIFT, 0x01);
            pmu_set_register_value_lp(VCORE_DISQ_HW_MODE_ADDR, VCORE_DISQ_HW_MODE_MASK, VCORE_DISQ_HW_MODE_SHIFT, 0x00);
            /* VCORE limiation, Switch to 0.65V first to 0.65V and then wait 150us */
            pmu_select_vcore_voltage_lp(PMU_DVS, PMIC_VCORE_0P65_V);
            hal_gpt_delay_us(150);
            /* VSRAM */
            pmu_ddie_sram_setting(PMIC_FALLING_0P8_0P55, vol_index);
            pmu_select_vcore_voltage_lp(PMU_DVS, vol_index);
            hal_gpt_delay_us(150);
            /* Buck_VCORE discharge disable */
            pmu_set_register_value_lp(VCORE_DISQ_SW_ADDR, VCORE_DISQ_SW_MASK, VCORE_DISQ_SW_SHIFT, 0x00);
            pmu_set_register_value_lp(VCORE_DISQ_HW_MODE_ADDR, VCORE_DISQ_HW_MODE_MASK, VCORE_DISQ_HW_MODE_SHIFT, 0x01);
            pmu_vcore_ipeak(vol_index);
            break;
        case PMIC_DVS_DEFAULT:
            /* VCORE */
            /* Buck_VCORE discharge enable, reduce VCORE waiting time */
            pmu_set_register_value_lp(VCORE_DISQ_SW_ADDR, VCORE_DISQ_SW_MASK, VCORE_DISQ_SW_SHIFT, 0x01);
            pmu_set_register_value_lp(VCORE_DISQ_HW_MODE_ADDR, VCORE_DISQ_HW_MODE_MASK, VCORE_DISQ_HW_MODE_SHIFT, 0x00);
            pmu_select_vcore_voltage_lp(PMU_DVS, PMIC_VCORE_0P65_V);
            hal_gpt_delay_us(150);
            /* VSRAM */
            pmu_ddie_sram_setting(PMIC_FALLING_0P8_0P55, vol_index);
            pmu_select_vcore_voltage_lp(PMU_DVS, vol_index);
            hal_gpt_delay_us(150);
            /* Buck_VCORE discharge disable */
            pmu_set_register_value_lp(VCORE_DISQ_SW_ADDR, VCORE_DISQ_SW_MASK, VCORE_DISQ_SW_SHIFT, 0x00);
            pmu_set_register_value_lp(VCORE_DISQ_HW_MODE_ADDR, VCORE_DISQ_HW_MODE_MASK, VCORE_DISQ_HW_MODE_SHIFT, 0x01);
            pmu_vcore_ipeak(vol_index);
            break;
    }
    old_index = vol_index;
    hal_nvic_restore_interrupt_mask_special(mask_pri);

    if (pmu_dynamic_debug.b.vcore) {
        log_hal_msgid_info("[PMU_PWR]vcore lock[%d], lock state[%d], switch_case[%d], lock_status[%d]", 4,
                           pmu_get_vcore_voltage_lp(), temp, pmu_switch_case, pmu_lock_status);
    }
    return vol_index;
}

pmu_power_vcore_voltage_t pmu_get_vcore_voltage_lp(void)
{
    uint32_t temp = 0;
    temp = pmu_get_register_value_lp(VCORE_VSEL_NM_ADDR, VCORE_VSEL_NM_MASK, VCORE_VSEL_NM_SHIFT);
    return pmu_get_vcore_setting_index(temp);
}

pmu_power_vcore_voltage_t pmu_get_vcore_setting_index(uint16_t vcore)
{
    const uint8_t vcbuck_voval[6] = {0x1e, 0x28, 0x50, 0x78, 0x84, 0x98};
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
        // Reduce VSRAM waiting time
        pmu_set_register_value_lp(VSRAM_NDIS_EN_SW_ADDR, VSRAM_NDIS_EN_SW_MASK, VSRAM_NDIS_EN_SW_SHIFT, 1);
        pmu_set_register_value_lp(VSRAM_NDIS_EN_HW_MODE_ADDR, VSRAM_NDIS_EN_HW_MODE_MASK, VSRAM_NDIS_EN_HW_MODE_SHIFT, 0);
    }
    if (vol <= PMIC_VCORE_0P65_V) {
        pmu_select_vsram_vosel(PMU_NORMAL, VSRAM_VOLTAGE_0P8);
    } else {
        pmu_select_vsram_vosel(PMU_NORMAL, VSRAM_VOLTAGE_0P92);
    }
    hal_gpt_delay_us(150);
    if (!((ste == PMIC_RISING_0P55_0P65) || (ste == PMIC_FALLING_0P65_0P55))) {
        // Reduce VSRAM waiting time
        pmu_set_register_value_lp(VSRAM_NDIS_EN_SW_ADDR, VSRAM_NDIS_EN_SW_MASK, VSRAM_NDIS_EN_SW_SHIFT, 0);
        pmu_set_register_value_lp(VSRAM_NDIS_EN_HW_MODE_ADDR, VSRAM_NDIS_EN_HW_MODE_MASK, VSRAM_NDIS_EN_HW_MODE_SHIFT, 1);
    }
}

void pmu_enable_power_lp(pmu_power_domain_t pmu_pdm, pmu_power_operate_t operate)
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
    uint8_t temp __attribute__((unused)) = 0;
    switch (pmu_pdm) {
        case PMU_BUCK_VCORE:
            if (mode == PMU_LOWPOWER_MODE) {
                pmu_set_register_value_lp(VCORE_SLP_EN_ADDR, VCORE_SLP_EN_MASK, VCORE_SLP_EN_SHIFT, operate);
            } else if (mode == PMU_NORMAL_MODE) {
                pmu_set_register_value_lp(VCORE_ACT_EN_ADDR, VCORE_ACT_EN_MASK, VCORE_ACT_EN_SHIFT, operate);
            }
            temp = pmu_get_register_value_lp(DA_VCORE_EN_ADDR, DA_VCORE_EN_MASK, DA_VCORE_EN_SHIFT);
            log_hal_msgid_info("[PMU_PWR]switch_power, After VCORE status[0x%02X]", 1, temp);
            break;
        case PMU_BUCK_VIO18:
            if (mode == PMU_LOWPOWER_MODE) {
                pmu_set_register_value_lp(VIO18_SLP_EN_ADDR, VIO18_SLP_EN_MASK, VIO18_SLP_EN_SHIFT, operate);
            } else if (mode == PMU_NORMAL_MODE) {
                pmu_set_register_value_lp(VIO18_ACT_EN_ADDR, VIO18_ACT_EN_MASK, VIO18_ACT_EN_SHIFT, operate);
            }
            temp = pmu_get_register_value_lp(DA_VIO18_EN_ADDR, DA_VIO18_EN_MASK, DA_VIO18_EN_SHIFT);
            log_hal_msgid_info("[PMU_PWR]switch_power, After VIO18 status[0x%02X]", 1, temp);
            break;
        case PMU_BUCK_VRF:
            if (mode == PMU_LOWPOWER_MODE) {
                pmu_set_register_value_lp(VRF_SLP_EN_ADDR, VRF_SLP_EN_MASK, VRF_SLP_EN_SHIFT, operate);
            } else if (mode == PMU_NORMAL_MODE) {
                pmu_set_register_value_lp(VRF_ACT_EN_ADDR, VRF_ACT_EN_MASK, VRF_ACT_EN_SHIFT, operate);
            }
            temp = pmu_get_register_value_lp(DA_VRF_EN_ADDR, DA_VRF_EN_MASK, DA_VRF_EN_SHIFT);
            log_hal_msgid_info("[PMU_PWR]switch_power, After VRF status[0x%02X]", 1, temp);
            break;
        case PMU_BUCK_VAUD18:
            pmu_set_register_value_lp(VAUD18_ACT_EN_ADDR, VAUD18_ACT_EN_MASK, VAUD18_ACT_EN_SHIFT, operate);
            hal_gpt_delay_ms(1);
            temp = pmu_get_register_value_lp(VAUD18_STATE_ADDR, VAUD18_STATE_MASK, VAUD18_STATE_SHIFT);
            log_hal_msgid_info("[PMU_PWR]switch_power, After VAUD18 status[0x%02X]", 1, temp);
            break;
        case PMU_LDO_VSRAM:
            if (mode == PMU_LOWPOWER_MODE) {
                pmu_set_register_value_lp(VSRAM_SLP_EN_ADDR, VSRAM_SLP_EN_MASK, VSRAM_SLP_EN_SHIFT, operate);
            } else if (mode == PMU_NORMAL_MODE) {
                pmu_set_register_value_lp(VSRAM_ACT_EN_ADDR, VSRAM_ACT_EN_MASK, VSRAM_ACT_EN_SHIFT, operate);
            }
            temp = pmu_get_register_value_lp(VSRAM_STATE_ADDR, VSRAM_STATE_MASK, VSRAM_STATE_SHIFT);
            log_hal_msgid_info("[PMU_PWR]switch_power, After VSRAM status[0x%02X]", 1, temp);
            break;
        default:
            log_hal_msgid_info("[PMU_PWR]switch_power, Power domain[%d] is not supported", 1, pmu_pdm);
            break;
    }
}

/*[VCORE]*/
void pmu_vcore_ipeak(int vol_index)
{
    switch (vol_index) {
        case PMIC_VCORE_0P525_V:
            pmu_set_register_value_lp(VCORE_IPEAK_VTUNE_NORM_ADDR, VCORE_IPEAK_VTUNE_NORM_MASK, VCORE_IPEAK_VTUNE_NORM_SHIFT, 0x1);
            break;
        case PMIC_VCORE_0P55_V:
            pmu_set_register_value_lp(VCORE_IPEAK_VTUNE_NORM_ADDR, VCORE_IPEAK_VTUNE_NORM_MASK, VCORE_IPEAK_VTUNE_NORM_SHIFT, 0x1);
            break;
        case PMIC_VCORE_0P65_V:
            pmu_set_register_value_lp(VCORE_IPEAK_VTUNE_NORM_ADDR, VCORE_IPEAK_VTUNE_NORM_MASK, VCORE_IPEAK_VTUNE_NORM_SHIFT, 0x3);
            break;
        case PMIC_VCORE_0P75_V:
            pmu_set_register_value_lp(VCORE_IPEAK_VTUNE_NORM_ADDR, VCORE_IPEAK_VTUNE_NORM_MASK, VCORE_IPEAK_VTUNE_NORM_SHIFT, 0x3);
            break;
        case PMIC_VCORE_0P80_V:
            pmu_set_register_value_lp(VCORE_IPEAK_VTUNE_NORM_ADDR, VCORE_IPEAK_VTUNE_NORM_MASK, VCORE_IPEAK_VTUNE_NORM_SHIFT, 0x4);
            break;
        case PMIC_VCORE_0P85_V:
            pmu_set_register_value_lp(VCORE_IPEAK_VTUNE_NORM_ADDR, VCORE_IPEAK_VTUNE_NORM_MASK, VCORE_IPEAK_VTUNE_NORM_SHIFT, 0x4);
            break;
        default:
            pmu_set_register_value_lp(VCORE_IPEAK_VTUNE_NORM_ADDR, VCORE_IPEAK_VTUNE_NORM_MASK, VCORE_IPEAK_VTUNE_NORM_SHIFT, 0x4);
    }
}

pmu_operate_status_t pmu_select_vcore_voltage_lp(pmu_power_stage_t mode, pmu_power_vcore_voltage_t vol)
{
    if ((mode > PMU_DVS) | (mode < PMU_SLEEP) | (vol > PMIC_VCORE_0P85_V) | (vol < PMIC_VCORE_0P525_V)) {
        log_hal_msgid_error("[PMU_PWR]select_vcore_voltage fail, error input", 0);
        return PMU_ERROR;
    }
    if ((mode != PMU_DVS) && (pmu_lock_status > 0)) {
        log_hal_msgid_error("[PMU_PWR]select_vcore_voltage is locked", 0);
        return PMU_ERROR;
    }

    switch (vol) {
        case PMIC_VCORE_0P525_V:
            pmu_set_register_value_lp(VCORE_VSEL_NM_ADDR, VCORE_VSEL_NM_MASK, VCORE_VSEL_NM_SHIFT, 0x1e);
            break;
        case PMIC_VCORE_0P55_V:
            pmu_set_register_value_lp(VCORE_VSEL_NM_ADDR, VCORE_VSEL_NM_MASK, VCORE_VSEL_NM_SHIFT, 0x28);
            break;
        case PMIC_VCORE_0P65_V:
            pmu_set_register_value_lp(VCORE_VSEL_NM_ADDR, VCORE_VSEL_NM_MASK, VCORE_VSEL_NM_SHIFT, 0x50);
            break;
        case PMIC_VCORE_0P75_V:
            pmu_set_register_value_lp(VCORE_VSEL_NM_ADDR, VCORE_VSEL_NM_MASK, VCORE_VSEL_NM_SHIFT, 0x78);
            break;
        case PMIC_VCORE_0P80_V:
            pmu_set_register_value_lp(VCORE_VSEL_NM_ADDR, VCORE_VSEL_NM_MASK, VCORE_VSEL_NM_SHIFT, 0x84);
            break;
        case PMIC_VCORE_0P85_V:
            pmu_set_register_value_lp(VCORE_VSEL_NM_ADDR, VCORE_VSEL_NM_MASK, VCORE_VSEL_NM_SHIFT, 0x98);
            break;
        default:
            pmu_set_register_value_lp(VCORE_VSEL_NM_ADDR, VCORE_VSEL_NM_MASK, VCORE_VSEL_NM_SHIFT, 0x84);
    }
    return PMU_OK;
}

/*[VAUD18]*/
void pmu_set_vaud18_voltage_lp(pmu_vaud18_vsel_t vsel, uint16_t volt)
{
    if ((volt < VAUD18_VOLT_MIN) || (volt > VAUD18_VOLT_MAX)) {
        //log_hal_msgid_error("[PMU_AUDIO]set_vaud18_volt fail, volt[%d]", 1, volt);
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
        pmu_set_register_value_lp(VAUD18_VSEL_L_ADDR, VAUD18_VSEL_L_MASK, VAUD18_VSEL_L_SHIFT, sel);
    } else if (vsel == PMU_VAUD18_VSEL_M) {
        pmu_set_register_value_lp(VAUD18_VSEL_M_ADDR, VAUD18_VSEL_M_MASK, VAUD18_VSEL_M_SHIFT, sel);
    } else if (vsel == PMU_VAUD18_VSEL_H) {
		pmu_set_register_value_lp(VAUD18_VSEL_H_ADDR, VAUD18_VSEL_H_MASK, VAUD18_VSEL_H_SHIFT, sel);
	}else {
        //log_hal_msgid_error("[PMU_AUDIO]set_vaud18_volt fail, vsel[%d]", 1, vsel);
        assert(0);
    }
}

/*[VSRAM]*/
void pmu_set_vsram_voltage_lp(pmu_vsram_voltage_t val)
{
    pmu_set_register_value_lp(VSRAM_VOSEL_NM_ADDR, VSRAM_VOSEL_NM_MASK, VSRAM_VOSEL_NM_SHIFT, val);
}

uint32_t pmu_get_vsram_voltage_lp(void)
{
    return pmu_get_register_value_lp(VSRAM_VOSEL_NM_ADDR, VSRAM_VOSEL_NM_MASK, VSRAM_VOSEL_NM_SHIFT);
}

/*[VRF]*/
void pmu_vrf_keep_nm_lp(void)
{
    pmu_set_register_value_lp(VRF_SLP_LP_ADDR, VRF_SLP_LP_MASK, VRF_SLP_LP_SHIFT, 0x0);
}

/*==========[Audio/MICBIAS]==========*/
pmu_operate_status_t pmu_set_micbias_vout_lp(pmu_micbias_index_t index, pmu_3vvref_voltage_t vol)
{
    if ((index >= PMIC_MICBIAS0) && (index <= PMIC_MICBIAS2) && (vol >= PMU_3VVREF_1P8_V) && (vol <= PMU_3VVREF_2P5_V)) {
        micbias_vout_arr[index] = vol;
        log_hal_msgid_info("[PMU_AUDIO]micbias_vout_arr[%d][%d][%d]", 3, micbias_vout_arr[0], micbias_vout_arr[1], micbias_vout_arr[2]);
        return PMU_OK;
    } else {
        log_hal_msgid_error("[PMU_AUDIO]micbias vol input error", 0);
        return PMU_INVALID;
    }
}

pmu_operate_status_t pmu_set_vaud18_vout_lp(pmu_vaud18_voltage_t lv, pmu_vaud18_voltage_t mv, pmu_vaud18_voltage_t hv)
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
            pmu_set_register_value_lp(LDO_MICBIAS_CON0, RG_AUDPWDBMICBIAS0_MASK, RG_AUDPWDBMICBIAS0_SHIFT, operate);
            break;
        case PMIC_MICBIAS1:
            pmu_set_register_value_lp(LDO_MICBIAS_CON1, RG_AUDPWDBMICBIAS1_MASK, RG_AUDPWDBMICBIAS1_SHIFT, operate);
            break;
        case PMIC_MICBIAS2:
            pmu_set_register_value_lp(LDO_MICBIAS_CON2, RG_AUDPWDBMICBIAS2_MASK, RG_AUDPWDBMICBIAS2_SHIFT, operate);
            break;
    }
    hal_gpt_delay_us(50);
}

void pmu_set_micbias_ldo_vout(pmu_micbias_index_t index)
{
    switch (index) {
        case PMIC_MICBIAS0:
            pmu_set_register_value_lp(LDO_MICBIAS_CON0, RG_AUDMICBIAS0_3VVREF_MASK, RG_AUDMICBIAS0_3VVREF_SHIFT, micbias_vout_arr[0]);
            break;
        case PMIC_MICBIAS1:
            pmu_set_register_value_lp(LDO_MICBIAS_CON1, RG_AUDMICBIAS1_3VVREF_MASK, RG_AUDMICBIAS1_3VVREF_SHIFT, micbias_vout_arr[1]);
            break;
        case PMIC_MICBIAS2:
            pmu_set_register_value_lp(LDO_MICBIAS_CON2, RG_AUDMICBIAS2_3VVREF_MASK, RG_AUDMICBIAS2_3VVREF_SHIFT, micbias_vout_arr[2]);
            break;
    }
    hal_gpt_delay_us(50);
}
void pmu_set_micbias_pulllow(pmu_micbias_index_t index,pmu_power_operate_t oper)
{
    switch (index) {
        case PMIC_MICBIAS0:
            pmu_set_register_value_lp(LDO_MICBIAS_CON0, RG_AUDACCDETMICBIAS0_PULLLOW_MASK, RG_AUDACCDETMICBIAS0_PULLLOW_SHIFT, oper);
            break;
        case PMIC_MICBIAS1:
            pmu_set_register_value_lp(LDO_MICBIAS_CON1, RG_AUDACCDETMICBIAS1_PULLLOW_MASK, RG_AUDACCDETMICBIAS1_PULLLOW_SHIFT, oper);
            break;
        case PMIC_MICBIAS2:
            pmu_set_register_value_lp(LDO_MICBIAS_CON2, RG_AUDACCDETMICBIAS2_PULLLOW_MASK, RG_AUDACCDETMICBIAS2_PULLLOW_SHIFT, oper);
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
            pmu_set_register_value_lp(LDO_MICBIAS_CON0, RG_AUDPWDBMICBIAS0_3VEN_MASK, RG_AUDPWDBMICBIAS0_3VEN_SHIFT, 0x1);
            pmu_set_register_value_lp(LDO_MICBIAS_CON0, RG_AUDMICBIAS0_3VBYPASSEN_MASK, RG_AUDMICBIAS0_3VBYPASSEN_SHIFT, 0x0);
            pmu_set_register_value_lp(LDO_MICBIAS_CON0, RG_AUDMICBIAS0_3VLOWPEN_MASK, RG_AUDMICBIAS0_3VLOWPEN_SHIFT, ldo_nm);
            if ((micbias_vout_arr[0] == PMU_3VVREF_2P78_V) || (micbias_vout_arr[0]) > PMU_3VVREF_2P0_V) {
                pmu_set_register_value_lp(LDO_MICBIAS_CON0, RG_AUDMICBIAS0_3VVREF_MASK, RG_AUDMICBIAS0_3VVREF_SHIFT, PMU_3VVREF_2P0_V);
            } else {
                pmu_set_register_value_lp(LDO_MICBIAS_CON0, RG_AUDMICBIAS0_3VVREF_MASK, RG_AUDMICBIAS0_3VVREF_SHIFT, PMU_3VVREF_1P8_V);
            }
            micbias_ldo_Resource[PMIC_MICBIAS_LDO0]++;
            break;
        case PMIC_MICBIAS_LDO1:
            pmu_set_register_value_lp(LDO_MICBIAS_CON1, RG_AUDPWDBMICBIAS1_3VEN_MASK, RG_AUDPWDBMICBIAS1_3VEN_SHIFT, 0x1);
            pmu_set_register_value_lp(LDO_MICBIAS_CON1, RG_AUDMICBIAS1_3VBYPASSEN_MASK, RG_AUDMICBIAS1_3VBYPASSEN_SHIFT, 0x0);
            pmu_set_register_value_lp(LDO_MICBIAS_CON1, RG_AUDMICBIAS1_3VLOWPEN_MASK, RG_AUDMICBIAS1_3VLOWPEN_SHIFT, ldo_nm);
            if ((micbias_vout_arr[1] == PMU_3VVREF_2P78_V) || (micbias_vout_arr[1]) > PMU_3VVREF_2P0_V) {
                pmu_set_register_value_lp(LDO_MICBIAS_CON1, RG_AUDMICBIAS1_3VVREF_MASK, RG_AUDMICBIAS1_3VVREF_SHIFT, PMU_3VVREF_2P0_V);
            } else {
                pmu_set_register_value_lp(LDO_MICBIAS_CON1, RG_AUDMICBIAS1_3VVREF_MASK, RG_AUDMICBIAS1_3VVREF_SHIFT, PMU_3VVREF_1P8_V);
            }
            micbias_ldo_Resource[PMIC_MICBIAS_LDO1]++;
            break;
        case PMIC_MICBIAS_LDO2:
            pmu_set_register_value_lp(LDO_MICBIAS_CON2, RG_AUDPWDBMICBIAS2_3VEN_MASK, RG_AUDPWDBMICBIAS2_3VEN_SHIFT, 0x1);
            pmu_set_register_value_lp(LDO_MICBIAS_CON2, RG_AUDMICBIAS2_3VBYPASSEN_MASK, RG_AUDMICBIAS2_3VBYPASSEN_SHIFT, 0x0);
            pmu_set_register_value_lp(LDO_MICBIAS_CON2, RG_AUDMICBIAS2_3VLOWPEN_MASK, RG_AUDMICBIAS2_3VLOWPEN_SHIFT, ldo_nm);
            if ((micbias_vout_arr[2] == PMU_3VVREF_2P78_V) || (micbias_vout_arr[2]) > PMU_3VVREF_2P0_V) {
                pmu_set_register_value_lp(LDO_MICBIAS_CON2, RG_AUDMICBIAS2_3VVREF_MASK, RG_AUDMICBIAS2_3VVREF_SHIFT, PMU_3VVREF_2P0_V);
            } else {
                pmu_set_register_value_lp(LDO_MICBIAS_CON2, RG_AUDMICBIAS2_3VVREF_MASK, RG_AUDMICBIAS2_3VVREF_SHIFT, PMU_3VVREF_1P8_V);
            }
            micbias_ldo_Resource[PMIC_MICBIAS_LDO2]++;
            break;
    }
}
void pmu_enable_micbias_mode(pmu_micbias_ldo_t ldo, pmu_micbias_mode_t mode)
{
    switch (ldo) {
        case PMIC_MICBIAS_LDO0:
            if (mode == PMIC_MICBIAS_HPM) {
                pmu_set_register_value_lp(LDO_MICBIAS_CON0, RG_MBIAS0_HPM_MASK, RG_MBIAS0_HPM_SHIFT, 0x1);
            } else {
                pmu_set_register_value_lp(LDO_MICBIAS_CON0, RG_MBIAS0_HPM_MASK, RG_MBIAS0_HPM_SHIFT, 0x0);
            }
            break;
        case PMIC_MICBIAS_LDO1:
            if (mode == PMIC_MICBIAS_HPM) {
                pmu_set_register_value_lp(LDO_MICBIAS_CON1, RG_MBIAS1_HPM_MASK, RG_MBIAS1_HPM_SHIFT, 0x1);
            } else {
                pmu_set_register_value_lp(LDO_MICBIAS_CON1, RG_MBIAS1_HPM_MASK, RG_MBIAS1_HPM_SHIFT, 0x0);
            }
            break;
        case PMIC_MICBIAS_LDO2:
            if (mode == PMIC_MICBIAS_HPM) {
                pmu_set_register_value_lp(LDO_MICBIAS_CON2, RG_MBIAS2_HPM_MASK, RG_MBIAS2_HPM_SHIFT, 0x1);
            } else {
                pmu_set_register_value_lp(LDO_MICBIAS_CON2, RG_MBIAS2_HPM_MASK, RG_MBIAS2_HPM_SHIFT, 0x0);
            }
            break;
    }
}

void pmu_set_micbias_ldo_mode_lp(pmu_micbias_ldo_t ldo, pmu_micbias_mode_t mode)
{
    log_hal_msgid_info("[PMU_AUDIO] LDO%d, mode%d",2, ldo, mode);
    switch(ldo) {
        case PMIC_MICBIAS_LDO0:
            if (mode == PMIC_MICBIAS_HPM){
                pmu_set_register_value_lp(LDO_MICBIAS_CON0, RG_AUDMICBIAS0_3VLOWPEN_MASK, RG_AUDMICBIAS0_3VLOWPEN_SHIFT, 0x0);
                pmu_set_register_value_lp(LDO_MICBIAS_CON0, RG_MBIAS0_HPM_MASK, RG_MBIAS0_HPM_SHIFT, 0x1);
            } else if (mode == PMIC_MICBIAS_LP){
                pmu_set_register_value_lp(LDO_MICBIAS_CON0, RG_MBIAS0_HPM_MASK, RG_MBIAS0_HPM_SHIFT, 0x0);
                pmu_set_register_value_lp(LDO_MICBIAS_CON0, RG_AUDMICBIAS0_3VLOWPEN_MASK, RG_AUDMICBIAS0_3VLOWPEN_SHIFT, 0x1);
            } else {
                pmu_set_register_value_lp(LDO_MICBIAS_CON0, RG_MBIAS0_HPM_MASK, RG_MBIAS0_HPM_SHIFT, 0x0);
                pmu_set_register_value_lp(LDO_MICBIAS_CON0, RG_AUDMICBIAS0_3VLOWPEN_MASK, RG_AUDMICBIAS0_3VLOWPEN_SHIFT, 0x0);
            }
        break;
        case PMIC_MICBIAS_LDO1:
            if (mode == PMIC_MICBIAS_HPM){
                pmu_set_register_value_lp(LDO_MICBIAS_CON1, RG_AUDMICBIAS1_3VLOWPEN_MASK, RG_AUDMICBIAS1_3VLOWPEN_SHIFT, 0x0);
                pmu_set_register_value_lp(LDO_MICBIAS_CON1, RG_MBIAS1_HPM_MASK, RG_MBIAS1_HPM_SHIFT, 0x1);
            } else if (mode == PMIC_MICBIAS_LP){
                pmu_set_register_value_lp(LDO_MICBIAS_CON1, RG_MBIAS1_HPM_MASK, RG_MBIAS1_HPM_SHIFT, 0x0);
                pmu_set_register_value_lp(LDO_MICBIAS_CON1, RG_AUDMICBIAS1_3VLOWPEN_MASK, RG_AUDMICBIAS1_3VLOWPEN_SHIFT, 0x1);
            } else {
                pmu_set_register_value_lp(LDO_MICBIAS_CON1, RG_MBIAS1_HPM_MASK, RG_MBIAS1_HPM_SHIFT, 0x0);
                pmu_set_register_value_lp(LDO_MICBIAS_CON1, RG_AUDMICBIAS1_3VLOWPEN_MASK, RG_AUDMICBIAS1_3VLOWPEN_SHIFT, 0x0);
            }
        break;
        case PMIC_MICBIAS_LDO2:
            if (mode == PMIC_MICBIAS_HPM){
                pmu_set_register_value_lp(LDO_MICBIAS_CON2, RG_AUDMICBIAS2_3VLOWPEN_MASK, RG_AUDMICBIAS2_3VLOWPEN_SHIFT, 0x0);
                pmu_set_register_value_lp(LDO_MICBIAS_CON2, RG_MBIAS2_HPM_MASK, RG_MBIAS2_HPM_SHIFT, 0x1);
            } else if (mode == PMIC_MICBIAS_LP){
                pmu_set_register_value_lp(LDO_MICBIAS_CON2, RG_MBIAS2_HPM_MASK, RG_MBIAS2_HPM_SHIFT, 0x0);
                pmu_set_register_value_lp(LDO_MICBIAS_CON2, RG_AUDMICBIAS2_3VLOWPEN_MASK, RG_AUDMICBIAS2_3VLOWPEN_SHIFT, 0x1);
            } else {
                pmu_set_register_value_lp(LDO_MICBIAS_CON2, RG_MBIAS2_HPM_MASK, RG_MBIAS2_HPM_SHIFT, 0x0);
                pmu_set_register_value_lp(LDO_MICBIAS_CON2, RG_AUDMICBIAS2_3VLOWPEN_MASK, RG_AUDMICBIAS2_3VLOWPEN_SHIFT, 0x0);
            }
        break;
    }
}
void pmu_micbias_resource_ctrl(pmu_micbias_ldo_t ldo)
{
    if (micbias_ldo_Resource[ldo] > 0) {
        micbias_ldo_Resource[ldo]--;
    }
    if (micbias_ldo_Resource[ldo] == 0) {
        switch (ldo) {
            case PMIC_MICBIAS_LDO0:
                pmu_set_register_value_lp(LDO_MICBIAS_CON3, 0xffff, 0, 0x0);
                pmu_enable_micbias_inrush(ldo, PMU_OFF);
                pmu_set_register_value_lp(LDO_MICBIAS_CON0, RG_AUDPWDBMICBIAS0_3VEN_MASK, RG_AUDPWDBMICBIAS0_3VEN_SHIFT, 0x0);
                pmu_set_register_value_lp(LDO_MICBIAS_CON0, RG_AUDMICBIAS0_3VLOWPEN_MASK, RG_AUDMICBIAS0_3VLOWPEN_SHIFT, 0x0);
                break;
            case PMIC_MICBIAS_LDO1:
                pmu_enable_micbias_inrush(ldo, PMU_OFF);
                pmu_set_register_value_lp(LDO_MICBIAS_CON1, RG_AUDPWDBMICBIAS1_3VEN_MASK, RG_AUDPWDBMICBIAS1_3VEN_SHIFT, 0x0);
                pmu_set_register_value_lp(LDO_MICBIAS_CON1, RG_AUDMICBIAS1_3VLOWPEN_MASK, RG_AUDMICBIAS1_3VLOWPEN_SHIFT, 0x0);
                break;
            case PMIC_MICBIAS_LDO2:
                pmu_enable_micbias_inrush(ldo, PMU_OFF);
                pmu_set_register_value_lp(LDO_MICBIAS_CON2, RG_AUDPWDBMICBIAS2_3VEN_MASK, RG_AUDPWDBMICBIAS2_3VEN_SHIFT, 0x0);
                pmu_set_register_value_lp(LDO_MICBIAS_CON2, RG_AUDMICBIAS2_3VLOWPEN_MASK, RG_AUDMICBIAS2_3VLOWPEN_SHIFT, 0x0);
                break;
        }
    }
}
/* exampel :
 *  pmu_enable_micbias_lp(PMIC_MICBIAS_LDO0,PMIC_MICBIAS0,PMIC_MICBIAS_NM,PMU_ON);
 *  pmu_enable_micbias_lp(PMIC_MICBIAS_LDO1,PMIC_MICBIAS1,PMIC_MICBIAS_NM,PMU_ON);
 *  pmu_enable_micbias_lp(PMIC_MICBIAS_LDO2,PMIC_MICBIAS2,PMIC_MICBIAS_NM,PMU_ON);
 */
void pmu_enable_micbias_lp(pmu_micbias_ldo_t ldo, pmu_micbias_index_t index, pmu_micbias_mode_t mode, pmu_power_operate_t operate)
{
    log_hal_msgid_info("[PMU_AUDIO] ldo = %d, micbias = %d mode = %d operate =%d \r\n", 4, ldo, index, mode, operate);
    pmu_set_register_value_lp(LDO_MICBIAS_CON3, RG_MBIAS_VIO18_MASK, RG_MBIAS_VIO18_SHIFT, 0x0);
    if (operate) {
        if (index == PMIC_MICBIAS_ALL) {
            pmu_set_micbias_pulllow(PMIC_MICBIAS_LDO0,PMU_OFF);
            pmu_set_register_value_lp(LDO_MICBIAS_CON1, 0xffff, 0, 0x0);
            pmu_set_register_value_lp(LDO_MICBIAS_CON2, 0xffff, 0, 0x0);
            pmu_enable_micbias_ldo(PMIC_MICBIAS_LDO0, mode);
            pmu_enable_micbias_inrush(PMIC_MICBIAS_LDO0, 0x2);
            pmu_set_register_value_lp(LDO_MICBIAS_CON3, RG_AUDPWDBMICBIAS0_1_MASK, RG_AUDPWDBMICBIAS0_1_SHIFT,0x2);
            pmu_set_register_value_lp(LDO_MICBIAS_CON3, RG_AUDPWDBMICBIAS0_2_MASK, RG_AUDPWDBMICBIAS0_2_SHIFT,0x2);
            pmu_set_micbias_ldo_vout(ldo);
            pmu_enable_micbias_inrush(PMIC_MICBIAS_LDO0, 0x1);
            pmu_set_register_value_lp(LDO_MICBIAS_CON3, RG_AUDPWDBMICBIAS0_1_MASK, RG_AUDPWDBMICBIAS0_1_SHIFT,0x1);
            pmu_set_register_value_lp(LDO_MICBIAS_CON3, RG_AUDPWDBMICBIAS0_2_MASK, RG_AUDPWDBMICBIAS0_2_SHIFT,0x1);
            pmu_set_micbias_pulllow(PMIC_MICBIAS_LDO0,PMU_ON);
            pmu_set_micbias_pulllow(PMIC_MICBIAS_LDO1,PMU_ON);
            pmu_set_micbias_pulllow(PMIC_MICBIAS_LDO2,PMU_ON);
            pmu_enable_micbias_mode(ldo, mode);
        } else if (index == PMIC_MICBIAS_0_1) {
            pmu_set_micbias_pulllow(PMIC_MICBIAS_LDO0,PMU_OFF);
            pmu_set_register_value_lp(LDO_MICBIAS_CON1, 0xffff, 0, 0x0);
            pmu_enable_micbias_ldo(PMIC_MICBIAS_LDO0, mode);
            pmu_enable_micbias_inrush(PMIC_MICBIAS_LDO0, 0x2);
            pmu_set_register_value_lp(LDO_MICBIAS_CON3, RG_AUDPWDBMICBIAS0_1_MASK, RG_AUDPWDBMICBIAS0_1_SHIFT,0x2);
            pmu_set_micbias_ldo_vout(ldo);
            pmu_enable_micbias_inrush(PMIC_MICBIAS_LDO0, 0x1);
            pmu_set_register_value_lp(LDO_MICBIAS_CON3, RG_AUDPWDBMICBIAS0_1_MASK, RG_AUDPWDBMICBIAS0_1_SHIFT,0x1);
            pmu_set_micbias_pulllow(PMIC_MICBIAS_LDO0,PMU_ON);
            pmu_set_micbias_pulllow(PMIC_MICBIAS_LDO1,PMU_ON);
            pmu_enable_micbias_mode(ldo, mode);
        } else if (index == PMIC_MICBIAS0_SHARE_ENABLE) {
            pmu_set_micbias_pulllow(PMIC_MICBIAS_LDO0,PMU_OFF);
            pmu_set_register_value_lp(LDO_MICBIAS_CON1, 0xffff, 0, 0x0);
            pmu_set_register_value_lp(LDO_MICBIAS_CON2, 0xffff, 0, 0x0);
            pmu_enable_micbias_ldo(PMIC_MICBIAS_LDO0, mode);
            pmu_enable_micbias_inrush(PMIC_MICBIAS_LDO0, 0x2);
            //pmu_set_register_value_lp(LDO_MICBIAS_CON3, RG_AUDPWDBMICBIAS0_1_MASK, RG_AUDPWDBMICBIAS0_1_SHIFT,0x2);
            //pmu_set_register_value_lp(LDO_MICBIAS_CON3, RG_AUDPWDBMICBIAS0_2_MASK, RG_AUDPWDBMICBIAS0_2_SHIFT,0x2);
            pmu_set_micbias_ldo_vout(ldo);
            pmu_enable_micbias_inrush(PMIC_MICBIAS_LDO0, 0x1);
            //pmu_set_register_value_lp(LDO_MICBIAS_CON3, RG_AUDPWDBMICBIAS0_1_MASK, RG_AUDPWDBMICBIAS0_1_SHIFT,0x1);
            //pmu_set_register_value_lp(LDO_MICBIAS_CON3, RG_AUDPWDBMICBIAS0_2_MASK, RG_AUDPWDBMICBIAS0_2_SHIFT,0x1);
            pmu_set_micbias_pulllow(PMIC_MICBIAS_LDO0,PMU_ON);
            pmu_set_micbias_pulllow(PMIC_MICBIAS_LDO1,PMU_ON);
            pmu_set_micbias_pulllow(PMIC_MICBIAS_LDO2,PMU_ON);
            pmu_enable_micbias_mode(ldo, mode);
        } else if (index == PMIC_MICBIAS0_SHARE_1) {
            pmu_set_register_value_lp(LDO_MICBIAS_CON3, RG_AUDPWDBMICBIAS0_1_MASK, RG_AUDPWDBMICBIAS0_1_SHIFT, 0x1);
        } else if (index == PMIC_MICBIAS0_SHARE_2) {
            pmu_set_register_value_lp(LDO_MICBIAS_CON3, RG_AUDPWDBMICBIAS0_2_MASK, RG_AUDPWDBMICBIAS0_2_SHIFT, 0x1);
        } else {
            pmu_set_micbias_pulllow(ldo,PMU_OFF);
            pmu_enable_micbias_ldo(ldo, mode);
            pmu_enable_micbias_inrush(index, 0x2);
            pmu_set_micbias_ldo_vout(ldo);
            pmu_enable_micbias_inrush(index, 0x1);
            pmu_set_micbias_pulllow(ldo,PMU_ON);
            pmu_enable_micbias_mode(ldo, mode);
        }
    } else {
        if (index == PMIC_MICBIAS0_SHARE_1){
            pmu_set_register_value_lp(LDO_MICBIAS_CON3, RG_AUDPWDBMICBIAS0_1_MASK, RG_AUDPWDBMICBIAS0_1_SHIFT, 0x0);
        }else if (index == PMIC_MICBIAS0_SHARE_2){
            pmu_set_register_value_lp(LDO_MICBIAS_CON3, RG_AUDPWDBMICBIAS0_2_MASK, RG_AUDPWDBMICBIAS0_2_SHIFT, 0x0);
        }else{
            pmu_micbias_resource_ctrl(ldo);
        }
    }
    log_hal_msgid_info("[PMU_AUDIO] MICBIAS_MISC = %x MICBIAS0_CON = %x, MICBIAS1_CON = %x MICBIAS2_CON = %x ", 4,
                       pmu_get_register_value_lp(LDO_MICBIAS_CON3, 0xffff, 0),
                       pmu_get_register_value_lp(LDO_MICBIAS_CON0, 0xffff, 0),
                       pmu_get_register_value_lp(LDO_MICBIAS_CON1, 0xffff, 0),
                       pmu_get_register_value_lp(LDO_MICBIAS_CON2, 0xffff, 0));
}

void pmu_set_audio_mode_lp(pmu_audio_mode_t mode, pmu_power_operate_t operate)
{
    if (operate) {
        pmu_set_audio_mode_init(mode);
#ifndef AIR_AUDIO_EXT_DAC_ENABLE
        pmu_enable_power_lp(PMU_BUCK_VAUD18, PMU_ON);
#endif
        pmu_set_register_value_lp(AD_VAUD18DRV_VOSEL1_SW_MODE_ADDR, AD_VAUD18DRV_VOSEL1_SW_MODE_MASK, AD_VAUD18DRV_VOSEL1_SW_MODE_SHIFT, 0x0);
        pmu_set_register_value_lp(AD_VAUD18DRV_VOSEL0_SW_MODE_ADDR, AD_VAUD18DRV_VOSEL0_SW_MODE_MASK, AD_VAUD18DRV_VOSEL0_SW_MODE_SHIFT, 0x0);
    } else {
        pmu_enable_power_lp(PMU_BUCK_VAUD18, PMU_OFF);
        pmu_set_register_value_lp(AD_VAUD18DRV_VOSEL1_SW_MODE_ADDR, AD_VAUD18DRV_VOSEL1_SW_MODE_MASK, AD_VAUD18DRV_VOSEL1_SW_MODE_SHIFT, 0x1);
        pmu_set_register_value_lp(AD_VAUD18DRV_VOSEL0_SW_MODE_ADDR, AD_VAUD18DRV_VOSEL0_SW_MODE_MASK, AD_VAUD18DRV_VOSEL0_SW_MODE_SHIFT, 0x1);
        pmu_set_register_value_lp(AD_VAUD18DRV_VOSEL1_ADDR, AD_VAUD18DRV_VOSEL1_MASK, AD_VAUD18DRV_VOSEL1_SHIFT, 0x1);
        pmu_set_register_value_lp(AD_VAUD18DRV_VOSEL0_ADDR, AD_VAUD18DRV_VOSEL0_MASK, AD_VAUD18DRV_VOSEL0_SHIFT, 0x1);
    }
}

void pmu_set_vaud18_ripple_lp(pmu_audio_mode_t mode, pmu_ripple_operate_t operate)
{
    if (operate == PMU_RIPPLE_OFF) {
        pmu_set_register_value_lp(RG_VAUD18_DYNAMIC_IPEAK_ADDR, 0x07, 5, 0b010);
        return;
    }

    switch (mode) {
        case PMU_CLASSAB:
            pmu_set_register_value_lp(RG_VAUD18_DYNAMIC_IPEAK_ADDR, 0x07, 5, 0b011);
            break;
        case PMU_CLASSG2:
            pmu_set_register_value_lp(RG_VAUD18_DYNAMIC_IPEAK_ADDR, 0x07, 5, 0b111);
            break;
        case PMU_CLASSG3:
            pmu_set_register_value_lp(RG_VAUD18_DYNAMIC_IPEAK_ADDR, 0x07, 5, 0b111);
            break;
        case PMU_CLASSD:
            pmu_set_register_value_lp(RG_VAUD18_DYNAMIC_IPEAK_ADDR, 0x07, 5, 0b101);
            break;
    }
}

extern uint8_t ripple_vaud18_en;
void pmu_set_audio_mode_init(pmu_audio_mode_t mode)
{
    audio_mode = mode;
    pmu_set_register_value_lp(AD_VAUD18DRV_VOSEL1_SW_MODE_ADDR, AD_VAUD18DRV_VOSEL1_SW_MODE_MASK, AD_VAUD18DRV_VOSEL1_SW_MODE_SHIFT, 0x1);
    pmu_set_register_value_lp(AD_VAUD18DRV_VOSEL0_SW_MODE_ADDR, AD_VAUD18DRV_VOSEL0_SW_MODE_MASK, AD_VAUD18DRV_VOSEL0_SW_MODE_SHIFT, 0x1);
    pmu_set_register_value_lp(AD_VAUD18DRV_VOSEL1_ADDR, AD_VAUD18DRV_VOSEL1_MASK, AD_VAUD18DRV_VOSEL1_SHIFT, 0x1);
    pmu_set_register_value_lp(AD_VAUD18DRV_VOSEL0_ADDR, AD_VAUD18DRV_VOSEL0_MASK, AD_VAUD18DRV_VOSEL0_SHIFT, 0x1);
    pmu_set_register_value_lp(AUDIO_MODE_ADDR, AUDIO_MODE_MASK, AUDIO_MODE_SHIFT, mode);
#ifdef PMU_SLT_ENV
    pmu_set_vaud18_ripple_lp(mode, ripple_vaud18_en);
    log_hal_msgid_info("[PMU_AUDIO] using SLT voltage setting", 0);
#else
    switch (mode) {
        case PMU_CLASSAB:
            pmu_set_vaud18_voltage_lp(PMU_VAUD18_VSEL_H, vaud18_vsel_arr[2]);
            pmu_set_vaud18_ripple_lp(mode, ripple_vaud18_en);
            log_hal_msgid_info("[PMU_AUDIO]set_audio_mode, CLASS AB", 0);
            break;
        case PMU_CLASSG2:
            pmu_set_vaud18_voltage_lp(PMU_VAUD18_VSEL_L, vaud18_vsel_arr[0]);
            pmu_set_vaud18_voltage_lp(PMU_VAUD18_VSEL_H, vaud18_vsel_arr[2]);
            pmu_set_vaud18_ripple_lp(mode, ripple_vaud18_en);
            log_hal_msgid_info("[PMU_AUDIO]set_audio_mode, CLASS G2", 0);
            break;
        case PMU_CLASSG3:
            pmu_set_vaud18_voltage_lp(PMU_VAUD18_VSEL_L, vaud18_vsel_arr[0]);
            pmu_set_vaud18_voltage_lp(PMU_VAUD18_VSEL_M, vaud18_vsel_arr[1]);
            pmu_set_vaud18_voltage_lp(PMU_VAUD18_VSEL_H, vaud18_vsel_arr[2]);
            pmu_set_vaud18_ripple_lp(mode, ripple_vaud18_en);
            log_hal_msgid_info("[PMU_AUDIO]set_audio_mode, CLASS G3", 0);
            break;
        case PMU_CLASSD:
            if(vaud18_vsel_arr[2]==PMU_VAUD18_1P1_V){
               pmu_set_vaud18_voltage_lp(PMU_VAUD18_VSEL_H, PMU_VAUD18_1P1_V);
            }else{
               pmu_set_vaud18_voltage_lp(PMU_VAUD18_VSEL_H, PMU_VAUD18_1P77_V);
            }
            pmu_set_vaud18_ripple_lp(mode, ripple_vaud18_en); // need to change ripple for different voltage?
            log_hal_msgid_info("[PMU_AUDIO]set_audio_mode, CLASS D", 0);
            break;
    }
#endif
    log_hal_msgid_info("[PMU_AUDIO]set_audio_mode, AUDIO_MODE[0x%X]", 1, pmu_get_register_value_lp(AUDIO_MODE_ADDR, AUDIO_MODE_MASK, AUDIO_MODE_SHIFT));
    log_hal_msgid_info("[PMU_AUDIO]set_audio_mode, VSEL_L[0x%X], IPEAK_L[0x%X]", 2, pmu_get_register_value_lp(VAUD18_VSEL_L_ADDR, VAUD18_VSEL_L_MASK, VAUD18_VSEL_L_SHIFT),
                       pmu_get_register_value_lp(VAUD18_IPEAK_VTUNE_L_ADDR, VAUD18_IPEAK_VTUNE_L_MASK, VAUD18_IPEAK_VTUNE_L_SHIFT));
    log_hal_msgid_info("[PMU_AUDIO]set_audio_mode, VSEL_M[0x%X], IPEAK_M[0x%X]", 2, pmu_get_register_value_lp(VAUD18_VSEL_M_ADDR, VAUD18_VSEL_M_MASK, VAUD18_VSEL_M_SHIFT),
                       pmu_get_register_value_lp(VAUD18_IPEAK_VTUNE_M_ADDR, VAUD18_IPEAK_VTUNE_M_MASK, VAUD18_IPEAK_VTUNE_M_SHIFT));
    log_hal_msgid_info("[PMU_AUDIO]set_audio_mode, VSEL_H[0x%X], IPEAK_H[0x%X]", 2, pmu_get_register_value_lp(VAUD18_VSEL_H_ADDR, VAUD18_VSEL_H_MASK, VAUD18_VSEL_H_SHIFT),
                       pmu_get_register_value_lp(VAUD18_IPEAK_VTUNE_H_ADDR, VAUD18_IPEAK_VTUNE_H_MASK, VAUD18_IPEAK_VTUNE_H_SHIFT));
}

/*==========[Interrupt]==========*/
void pmu_eint_init_lp(void)
{
    hal_eint_config_t config;
    config.trigger_mode = HAL_EINT_LEVEL_LOW;
    config.debounce_time = 0;
    hal_eint_init(HAL_EINT_PMU, &config);
    hal_eint_register_callback(HAL_EINT_PMU, pmu_eint_handler_lp, NULL);
    hal_eint_unmask(HAL_EINT_PMU);
}

void pmu_eint_handler_lp(void *parameter)
{
    UNUSED(parameter);

    hal_eint_mask(HAL_EINT_PMU);

    uint32_t rg_092 = pmu_get_register_value_lp(0x092, 0xFFFF, 0);
    uint16_t pwrkey_flag = ((rg_092 & 0x180) >> 7);
    uint16_t chg_flag = ((rg_092 & 0x7C) >> 2);
    uint32_t time_s = 0, time_e = 0;

    log_hal_msgid_info("[PMU_BASIC]pmu_eint, rg_092[0x%X], pwrkey_flag[0x%X], chg_flag[0x%X]", 3, rg_092, pwrkey_flag, chg_flag);

    pmu_force_set_register_value_lp(0x092, rg_092);

    if (pwrkey_flag) {
        pmu_pwrkey_hdlr(pwrkey_flag);
    }
    if (chg_flag) {
#ifndef AIR_PMU_DISABLE_CHARGER
        pmu_chg_hdlr(chg_flag);
#endif
    }

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &time_s);
    while (1) {
        rg_092 = pmu_get_register_value_lp(0x092, 0xFFFF, 0);
        if (!rg_092) {
            break;
        } else {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &time_e);
            if (time_e - time_s > PMU_INT_TIMEOUT) {
                log_hal_msgid_error("[PMU_BASIC]pmu_eint fail, time[%dus], rg_092[0x%X]", 2, (time_e - time_s), rg_092);
                break;
            }
        }
    }
    hal_eint_unmask(HAL_EINT_PMU);
}

void pmu_pwrkey_hdlr(uint16_t pwrkey_flag)
{
    if (pwrkey_flag & PWRKEY_PRESS) {
        if (pmu_function_table_lp[RG_REGEN_IRQ_RISE_FLAG].pmu_callback) {
            log_hal_msgid_info("[PMU_BASIC]pwrkey_hdlr, press", 0);
            pmu_function_table_lp[RG_REGEN_IRQ_RISE_FLAG].pmu_callback();
        }
        if ((pwrkey_flag & PWRKEY_RELEASE) && (!pmu_get_pwrkey_state_lp())) {
            if (pmu_function_table_lp[RG_REGEN_IRQ_FALL_FLAG].pmu_callback) {
                log_hal_msgid_info("[PMU_BASIC]pwrkey_hdlr, release", 0);
                pmu_function_table_lp[RG_REGEN_IRQ_FALL_FLAG].pmu_callback();
            }
        }
    } else if (pwrkey_flag & PWRKEY_RELEASE) {
        if (pmu_function_table_lp[RG_REGEN_IRQ_FALL_FLAG].pmu_callback) {
            log_hal_msgid_info("[PMU_BASIC]pwrkey_hdlr, release", 0);
            pmu_function_table_lp[RG_REGEN_IRQ_FALL_FLAG].pmu_callback();
        }
    } else {
        log_hal_msgid_error("[PMU_BASIC]pwrkey_hdlr fail, pwrkey_flag[0x%X]", 1, pwrkey_flag);
    }
}

void pmu_clear_all_intr(void)
{
    pmu_force_set_register_value_lp(0x084, pmu_get_register_value_lp(0x084, 0xFFFF, 0));
    pmu_force_set_register_value_lp(0x092, pmu_get_register_value_lp(0x092, 0xFFFF, 0));
}

pmu_status_t pmu_register_callback_lp(pmu_interrupt_index_lp_t pmu_int_ch, pmu_callback_t callback, void *user_data)
{
    if (pmu_int_ch >= PMU_INT_MAX_LP || callback == NULL) {
        log_hal_msgid_error("[PMU_BASIC]register_callback fail, pmu_int_ch[%d]", 1, pmu_int_ch);
        return PMU_STATUS_INVALID_PARAMETER;
    }
#ifdef AIR_PMU_DISABLE_CHARGER
    if ((pmu_int_ch >= RG_CHG_IN_INT_FLAG) && (pmu_int_ch <= RG_CHG_RECHG_INT_FLAG)) {
        log_hal_msgid_error("[PMU_BASIC]register_callback, bypass for dongle", 0);
        //assert(0);
    }
#endif

    pmu_function_table_lp[pmu_int_ch].init_status = 1;
    pmu_function_table_lp[pmu_int_ch].pmu_callback = callback;
    pmu_function_table_lp[pmu_int_ch].user_data = user_data;
    pmu_function_table_lp[pmu_int_ch].isMask = false;

    log_hal_msgid_info("[PMU_BASIC]register_callback, pmu_int_ch[%d]", 1, pmu_int_ch);

    return PMU_STATUS_SUCCESS;
}
pmu_status_t pmu_deregister_callback_lp(pmu_interrupt_index_lp_t pmu_int_ch)
{
    if (pmu_int_ch >= PMU_INT_MAX_LP) {
        log_hal_msgid_error("[PMU_BASIC]deregister_callback fail, pmu_int_ch[%d]", 1, pmu_int_ch);
        return PMU_STATUS_INVALID_PARAMETER;
    }
#ifdef AIR_PMU_DISABLE_CHARGER
    if ((pmu_int_ch >= RG_CHG_IN_INT_FLAG) && (pmu_int_ch <= RG_CHG_RECHG_INT_FLAG)) {
        log_hal_msgid_error("[PMU_BASIC]deregister_callback, bypass for dongle", 0);
        //assert(0);
    }
#endif

    pmu_function_table_lp[pmu_int_ch].init_status = 0;
    pmu_function_table_lp[pmu_int_ch].pmu_callback = NULL;
    pmu_function_table_lp[pmu_int_ch].user_data = NULL;
    pmu_function_table_lp[pmu_int_ch].isMask = true;

    log_hal_msgid_info("[PMU_BASIC]deregister_callback, pmu_int_ch[%d]", 1, pmu_int_ch);

    return PMU_STATUS_SUCCESS;
}

/*==========[pwrkey & captouch & usb]==========*/
pmu_operate_status_t pmu_pwrkey_normal_key_init_lp(pmu_pwrkey_config_t *config)
{
    pmu_status_t status = PMU_STATUS_ERROR;
    status = pmu_register_callback_lp(RG_REGEN_IRQ_RISE_FLAG, config->callback1, config->user_data1);
    if (status != PMU_STATUS_SUCCESS) {
        return status;
    }
    status = pmu_register_callback_lp(RG_REGEN_IRQ_FALL_FLAG, config->callback2, config->user_data2);
    return status;
}

void pmu_latch_power_key_for_bootloader_lp(void)
{
}

bool pmu_get_pwrkey_state_lp(void)
{
    bool ret = FALSE;

    uint32_t rg_608 = pmu_get_register_value_lp(0x608, 0xFFFF, 0);
    if (rg_608 & AD_REGEN_DEB) {
        ret = TRUE;
    }

    log_hal_msgid_info("[PMU_BASIC]get_pwrkey_state, rg_608[0x%X], ret[%d]", 2, rg_608, ret);

    return ret;
}

void pmu_enable_pk_lpsd_lp(pmu_lpsd_time_t tmr, pmu_power_operate_t oper)
{
    log_hal_msgid_info("[PMU_BASIC]enable_pk_lpsd, time[%d], enable[%d]", 2, tmr, oper);
    pmu_set_register_value_lp(REGEN_LPSD_DEB_SEL_ADDR, REGEN_LPSD_DEB_SEL_MASK, REGEN_LPSD_DEB_SEL_SHIFT, tmr);
    pmu_set_register_value_lp(REGEN_LPSD_EN_ADDR, REGEN_LPSD_EN_MASK, REGEN_LPSD_EN_SHIFT, oper);
}

void pmu_enable_cap_lpsd_lp(pmu_power_operate_t oper)
{
    log_hal_msgid_info("[PMU_BASIC]enable_cap_lpsd, oper[%d]", 1, oper);
    pmu_set_register_value_lp(CAP_LPSD_EN_ADDR, CAP_LPSD_EN_MASK, CAP_LPSD_EN_SHIFT, oper);
}

void pmu_set_cap_duration_time_lp(pmu_lpsd_time_t tmr)
{
    pmu_set_register_value_lp(CAP_LPSD_DEB_SEL_ADDR, CAP_LPSD_DEB_SEL_MASK, CAP_LPSD_DEB_SEL_SHIFT, tmr);
}

void pmu_set_cap_wo_vbus_lp(pmu_power_operate_t oper)
{
    if (oper) {
        pmu_set_register_value_lp(SPARE_REG_ADDR, 0x1, 0x0, 0);
    } else {
        pmu_set_register_value_lp(SPARE_REG_ADDR, 0x1, 0x0, 1);
    }
}

uint8_t pmu_get_usb_input_status_lp(void)
{
    return (uint8_t)pmu_get_chr_detect_value_lp();
}

/*==========[Basic]==========*/
void pmu_config_lp(void)
{
    pmu_cal_init();
    pmu_set_init();
#ifndef AIR_PMU_DISABLE_CHARGER
    pmu_chg_init();
    pmu_bat_init();
    pmu_rst_pat_init();
#endif
    pmu_ntc_init();
}

void pmu_init_lp(void)
{
    pmu_safety_confirmation_lp();
    uint32_t pon_reason = 0, poff_reason = 0;
    poff_reason = pmu_get_power_off_reason_lp();
    pon_reason = pmu_get_power_on_reason_lp();

    if(pon_reason == 0 && poff_reason == 0 && pmu_get_chr_detect_value_lp() == 0){
        log_hal_msgid_info("[PMU_BASIC] no power on/off reason & chg not exist, enter off mode", 0);
        pmu_power_off_sequence_lp(PMU_PWROFF);
    }

    /* open WD_RST_EN, SYS_RST_EN, REGEN_LPSD_EN, CAP_LPSD_EN */
    pmu_set_register_value_lp(0x082, 0xF, 3, 0xF);

    pmu_set_register_value_lp(0x004, SRCLK_EN_IC_MASK, 12, 0);

    /* Aviod sw do power off when chg plug in */
    pmu_set_register_value_lp(0x09E, OFF_PROT_EN_CHGFLO_B_MASK, 1, 1);
    pmu_set_register_value_lp(0x09E, OFF_PROT_EN_CHG_PLUG_MASK, 0, 1);

    /* latch LSW_OCP_EN to VCHG_VSYS_MAX domain */
    pmu_set_register_value_lp(0x32C, RG_DATA_SHIPPING_MASK_MASK, 3, 0);
    pmu_set_register_value_lp(0x32C, RG_SET_SHIPPING_MASK_MASK, 2, 0);
    pmu_set_register_value_lp(0x32C, RG_SET_SHIPPING_MASK_MASK, 2, 1);
    pmu_set_register_value_lp(0x32C, RG_BAT_SYS_COMP_LPM_VCHGON_MASK, 1, 0);
    pmu_set_register_value_lp(0x32C, RG_CHG_SYS_COMP_LPM_MASK, 0, 0);

    pmu_set_register_value_lp(0x328, RG_SW_OCP_ENB_MASK, 7, 0);
    pmu_set_register_value_lp(0x11C, VIO18_IPEAK_VTUNE_NORM_MASK, 0, 3);
    pmu_set_register_value_lp(0x116, RG_VIO18_DYNAMIC_IPEAK_MASK, 0, 0xFB);

#ifndef AIR_PMU_DISABLE_CHARGER
    pmu_eoc_ctrl(PMU_OFF);
    if (pmu_get_chr_detect_value_lp()) {
        pmu_uart_psw_cl(PMU_OFF);
    } else {
        pmu_uart_psw_cl(PMU_ON);
    }
    pmu_uart_psw(PMU_ON);
#endif
    pmu_vio18_pull_up(PMU_OFF);

    /* move to pmu_rst_pat_init */
    pmu_force_set_register_value_lp(RST_PAT_CON0, 0x1724);
    pmu_force_set_register_value_lp(RST_PAT_CON1, 0x1724);
    pmu_set_register_value_lp(0x082, RSTPAT_EN_MASK, 7, 1);

    /* Switch Vrf to 0.9V */
    pmu_set_register_value_lp(0x18E, VRF_VOUTSEL_NORM_MASK, 0, 2);
    pmu_set_register_value_lp(0x18E, VRF_VOUTSEL_LP_MASK, 4, 2);

    pmu_fast_buffer_disable();
    pmu_set_register_value_lp(0x00C, RG_BGHP_ENB_MASK, 0, 0);
    pmu_set_register_value_lp(0x00E, RG_BGR_TRIM_ENB_MASK, 8, 0);
    log_hal_msgid_info("[PMU_BASIC]pmu_init, rg_012[0x%X], rg_082[0x%X], rg_092[0x%X], rg_09A[0x%X], rg_324[0x%X], rg_606[0x%X], rg_608[0x%X]", 7,
        pmu_get_register_value_lp(0x012, 0xFFFF, 0), pmu_get_register_value_lp(0x082, 0xFFFF, 0), pmu_get_register_value_lp(0x092, 0xFFFF, 0),
        pmu_get_register_value_lp(0x09A, 0xFFFF, 0), pmu_get_register_value_lp(0x324, 0xFFFF, 0),
        pmu_get_register_value_lp(0x606, 0xFFFF, 0), pmu_get_register_value_lp(0x608, 0xFFFF, 0));

    pmu_eint_init_lp();
    pmu_auxadc_init();
#ifdef HAL_SLEEP_MANAGER_ENABLED
    sleep_management_register_all_secure_suspend_callback(SLEEP_BACKUP_RESTORE_PMU, (sleep_management_suspend_callback_t)pmu_sleep_suspend_lp, NULL);
    sleep_management_register_all_secure_resume_callback(SLEEP_BACKUP_RESTORE_PMU, (sleep_management_suspend_callback_t)pmu_sleep_resume_lp, NULL);
#endif
#ifdef AIR_SYS32K_CLOCK_SOURCE_DCXO
    pmu_switch_power(PMU_BUCK_VRF, PMU_LOWPOWER_MODE, PMU_ON);
#endif
    pmu_init_flag = 1;
    pmu_force_set_register_value_lp(PMU_CON2, 0xffff); // clear power on/off reason
}

void pmu_power_off_sequence_lp(pmu_power_stage_t stage)
{
    switch (stage) {
        case PMU_PWROFF:
            pmu_auxadc_get_channel_value(PMU_AUX_VCHG);
            pmu_set_register_value_lp(0x32C, RG_DATA_SHIPPING_MASK_MASK, 3, 0x1);
            pmu_set_register_value_lp(0x32C, RG_SET_SHIPPING_MASK_MASK, 2, 0x0);
            pmu_set_register_value_lp(0x32C, RG_SET_SHIPPING_MASK_MASK, 2, 0x1);
            log_hal_msgid_info("[PMU_BASIC]power_off_sequence [OFF], rg_012[0x%X], rg_084[0x%X], rg_092[0x%X], rg_09A[0x%X], rg_32C[0x%X], rg_608[0x%X], rg_60C[0x%X]", 6,
                               pmu_get_register_value_lp(0x012, 0xFFFF, 0), pmu_get_register_value_lp(0x084, 0xFFFF, 0),
                               pmu_get_register_value_lp(0x092, 0xFFFF, 0), pmu_get_register_value_lp(0x09A, 0xFFFF, 0), pmu_get_register_value_lp(0x32C, 0xFFFF, 0),
                               pmu_get_register_value_lp(0x608, 0xFFFF, 0), pmu_get_register_value_lp(0x60C, 0xFFFF, 0));
            pmu_set_register_value_lp(0x086, PWR_OFF_MASK, 0, 0x1);
            break;
        case PMU_RTC:
            pmu_set_register_value_lp(0x012, RG_UARTPSW_ENB_MASK, 0, 0x1);
            pmu_set_register_value_lp(0x328, RG_SW_OCP_ENB_MASK, 7, 0x1);
            pmu_set_register_value_lp(0x32C, RG_CHG_SYS_COMP_LPM_MASK, 0, 0x1);
            pmu_set_register_value_lp(0x32C, RG_BAT_SYS_COMP_LPM_VCHGON_MASK, 1, 0x1);
            pmu_set_register_value_lp(0x00C, RG_BGHP_ENB_MASK, 0, 0x1);
            pmu_set_register_value_lp(0x00E, RG_FAST_BUFFER_ENB_MASK, 2, 0x1);
            pmu_set_register_value_lp(0x00E, RG_BGR_TRIM_ENB_MASK, 8, 0x1);

#ifndef AIR_PMU_DISABLE_CHARGER
            pmu_enable_charger_lp(PMU_OFF);
            if (pmu_auxadc_get_channel_value_lp(PMU_AUX_VBAT) > 3200) {
                pmu_eoc_ctrl(PMU_ON);
                pmu_uart_psw(PMU_OFF);
                pmu_uart_psw_cl(PMU_OFF);
            }
            else {
                pmu_eoc_ctrl(PMU_OFF);
                pmu_uart_psw(PMU_ON);
                pmu_uart_psw_cl(PMU_ON);
            }
#endif
            pmu_clear_all_intr();

            log_hal_msgid_info("[PMU_BASIC]power_off_sequence [RTC], rg_012[0x%X], rg_084[0x%X], rg_092[0x%X], rg_09A[0x%X], rg_608[0x%X], rg_60C[0x%X]", 6,
                               pmu_get_register_value_lp(0x012, 0xFFFF, 0), pmu_get_register_value_lp(0x084, 0xFFFF, 0),
                               pmu_get_register_value_lp(0x092, 0xFFFF, 0), pmu_get_register_value_lp(0x09A, 0xFFFF, 0),
                               pmu_get_register_value_lp(0x608, 0xFFFF, 0), pmu_get_register_value_lp(0x60C, 0xFFFF, 0));

            pmu_select_vcore_voltage_lp(PMU_DVS, PMIC_VCORE_0P80_V);
            pmu_set_register_value_lp(0x086, RTC_MODE_MASK, 1, 0x1);
            break;
        case PMU_SLEEP:
            pmu_set_register_value_lp(0x00E, RG_FAST_BUFFER_ENB_MASK, 2, 0x1);
            pmu_set_register_value_lp(0x00E, RG_BGR_TRIM_ENB_MASK, 8, 0x0);
            break;
        case PMU_NORMAL:
            break;
        case PMU_DVS:
            break;
    }
}

void pmu_bg_trim(uint8_t sel)
{
    pmu_set_register_value_lp(RG_BGR_TRIM_ADDR, RG_BGR_TRIM_MASK, RG_BGR_TRIM_SHIFT, sel);
}

void pmu_fast_buffer_disable(void)
{
    uint32_t tick_e = 0, diff = 0, delay = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &tick_e);
    diff = tick_e - pmu_fast_buffer_tick;
    if ((pmu_fast_buffer_tick > 0) && (diff < FAST_BUFFER_DELAY_US)) {
        delay = FAST_BUFFER_DELAY_US - diff;
        hal_gpt_delay_us(FAST_BUFFER_DELAY_US - delay);
    }
    pmu_set_register_value_lp(RG_FAST_BUFFER_ENB_ADDR, RG_FAST_BUFFER_ENB_MASK, RG_FAST_BUFFER_ENB_SHIFT, 1);
    pmu_bg_trim(hpbg_trim_sel);

    log_hal_msgid_info("[PMU_BASIC] fast_buffer_disable, tick_s[%d], tick_e[%d], diff[%d], delay[%d]", 4,
                       pmu_fast_buffer_tick, tick_e, diff, delay);
}

uint8_t pmu_get_power_on_reason_lp(void)
{
    uint16_t rg_084;
    if (pmu_init_flag == 0) {
        rg_084 = pmu_get_register_value_lp(PMU_CON2, 0xFFFF, 0);

        if (rg_084 & PMU_REGEN_PON_FLAG) {
            power_on_reason |= PMU_KEY;
            power_on_reason |= BOOT_MODE;
            log_hal_msgid_info("[PMU_BASIC]power_on_reason : REGEN_PON", 0);
        }
        if (rg_084 & PMU_RTC_ALARM_FLAG) {
            power_on_reason |= PMU_RTCA;
            log_hal_msgid_info("[PMU_BASIC]power_on_reason : RTC_ALARM", 0);
        }
        if (rg_084 & PMU_CHG_PON_FLAG) {
            power_on_reason |= PMU_CHRIN;
            power_on_reason |= BOOT_MODE;
            log_hal_msgid_info("[PMU_BASIC]power_on_reason : CHG_PON", 0);
        }
        if (rg_084 & PMU_CHG_ALARM_FLAG) {
            power_on_reason |= PMU_CHRIN;
            log_hal_msgid_info("[PMU_BASIC]power_on_reason : CHG_ALARM", 0);
        }
        if (rg_084 & PMU_REGEN_ALARM_FLAG) {
            power_on_reason |= PMU_KEY;
            log_hal_msgid_info("[PMU_BASIC]power_on_reason : REGEN_ALARM", 0);
        }

        //log_hal_msgid_info("[PMU_BASIC]power_on_reason, rg_084[0x%X], reason[0x%X]", 2, rg_084, pmu_power_on_reason.value);
    }
    return power_on_reason;
}

uint8_t pmu_get_power_off_reason_lp(void)
{
    uint16_t rg_084;
    if (pmu_init_flag == 0) {
        rg_084 = pmu_get_register_value_lp(PMU_CON2, 0xFFFF, 0);

        if (rg_084 & PMU_RTC_MODE_FLAG) {
            pmu_power_off_reason = 1;
            log_hal_msgid_info("[PMU_BASIC]power_off_reason : RTC_MODE", 0);
        }
        if (rg_084 & PMU_CAP_LPSD_FLAG) {
            pmu_power_off_reason = 14;
            log_hal_msgid_info("[PMU_BASIC]power_off_reason : CAP_LPSD", 0);
        }
        if (rg_084 & PMU_REGEN_LPSD_FLAG) {
            pmu_power_off_reason = 10;
            log_hal_msgid_info("[PMU_BASIC]power_off_reason : REGEN_LPSD", 0);
        }
        if (rg_084 & PMU_SYS_RST_FLAG) {
            pmu_power_off_reason = 13;
            log_hal_msgid_info("[PMU_BASIC]power_off_reason : SYS_RST", 0);
        }
        if (rg_084 & PMU_WD_RST_FLAG) {
            pmu_power_off_reason = 8;
            log_hal_msgid_info("[PMU_BASIC]power_off_reason : WD_RST", 0);
        }
        if (rg_084 & PMU_VBUS_PAT_RST_FLAG) {
            pmu_power_off_reason = 15;
            log_hal_msgid_info("[PMU_BASIC]power_off_reason : VBUS_PAT_RST", 0);
        }
        if (rg_084 & PMU_OFF2IDLE_FLAG) {
            pmu_power_off_reason = 4;
            log_hal_msgid_info("[PMU_BASIC]power_off_reason : OFF2IDLE", 0);
        }
        if (pmu_power_off_reason == 0) {
            log_hal_msgid_info("[PMU_BASIC]power_off_reason : OFF_MODE", 0);
        }

        //log_hal_msgid_info("[PMU_BASIC]power_off_reason, rg_084[0x%X], reason[%d]", 2, rg_084, pmu_power_off_reason);
    }

    return pmu_power_off_reason;
}

uint8_t pmu_get_pmic_version_lp(void)
{
    if (pmu_init_flag == 0)
    {
        pmu_get_otp(OTP_ADIE_VER_ADDR, (uint8_t *)&pmic_ver, sizeof(pmic_ver));
        if (pmic_ver == 0xFF) {
            log_hal_msgid_error("[PMU_BASIC]adie_version fail", 0);
        }
        else {
            ft_ver = (pmic_ver >> 4) & 0xF;
            pmic_ver = pmic_ver & 0xF;
            log_hal_msgid_info("[PMU_BASIC]adie_version[%d]", 1, pmic_ver);
        }
    }

    return pmic_ver;
}

void pmu_safety_confirmation_lp(void)
{
#ifdef HAL_FLASH_MODULE_ENABLED
    hal_flash_init();
    if (pmu_get_pmic_version_lp() == 0xFF) {
#ifdef HAL_RTC_MODULE_ENABLED
        log_hal_msgid_error("[PMU_BASIC]pmu_safety fail, enter RTC mode", 0);
        hal_rtc_init();
        hal_rtc_enter_rtc_mode();
#endif
    }
#endif
}

void pmu_sleep_suspend_lp(void)
{
    pmu_power_off_sequence_lp(PMU_SLEEP);
    //pmic_i2c_deinit();
}
void pmu_sleep_resume_lp(void)
{
}

void pmu_set_rstpat_lp(pmu_power_operate_t oper, pmu_rstpat_src_t src)
{
    if (oper > PMU_ON || oper < PMU_OFF) {
        return;
    }
    if (src > PMU_RSTPAT_SRC_VBUS_UART || src < PMU_RSTPAT_SRC_VBUS) {
        return;
    }

    pmu_set_register_value_lp(RSTPAT_EN_ADDR, RSTPAT_EN_MASK, RSTPAT_EN_SHIFT, oper);
    pmu_set_register_value_lp(RG_PAT_SRC_SEL1_ADDR, RG_PAT_SRC_SEL1_MASK, RG_PAT_SRC_SEL1_SHIFT, src);
}

void pmu_dump_rg_lp(void)
{
    uint16_t addr;
    uint16_t value __unused;
    const uint16_t addr_start = IO_CFG0;
    const uint16_t addr_end = PMU_TEST6;

    /**
     * NOTE: Adie address is 16 bits wide
     */
    for (addr = addr_start; addr <= addr_end; addr += 2) {
        value = pmu_get_register_value_lp(addr, 0xffff, 0);
        log_hal_msgid_info("[PMU_BASIC]dump_rg, APMU Addr[0x%04X] Value[0x%04X]", 2, addr, value);
    }
}

void pmu_rg_timer_cb(uint16_t *user_data)
{
    uint16_t addr;
    for(addr = addr_start; addr <= addr_end; addr += 2){
        log_hal_msgid_info("[PMU_BASIC]dump_rg, APMU Addr[0x%04X] Value[0x%04X]", 2, addr, pmu_get_register_value_lp(addr, 0xffff, 0));
    }
    hal_gpt_sw_start_timer_ms(rg_chk_timer, rg_time,
                              (hal_gpt_callback_t)pmu_rg_timer_cb, NULL);
}

void pmu_check_rg_timer_lp(uint16_t start, uint16_t end, uint16_t timer)
{
    uint16_t addr;
    hal_gpt_status_t gpt_status;
    if(timer == 0){
        hal_gpt_sw_stop_timer_ms(rg_chk_timer);
        for(addr = start; addr <= end; addr += 2){
            log_hal_msgid_info("[PMU_BASIC]dump_rg, APMU Addr[0x%04X] Value[0x%04X]", 2, addr, pmu_get_register_value_lp(addr, 0xffff, 0));
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
            //log_hal_msgid_error("[PMU_BASIC]d2d_i2c_read, ERROR I2c access PMIC fail", 0);
            assert(0);
        }
    } while ((result_read != 0) && (retry_cnt <= 60));
    return (retry_cnt);
}

uint32_t pmu_get_register_value_lp(uint32_t address, uint32_t mask, uint32_t shift)
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

void pmu_toggle_i2c_clk()
{
    if (rst_adie_clk) {
        log_hal_msgid_info("[PMU_BASIC]toggle_i2c_clk, init 0x420D0064[%x], DIN_GPIO", 1, *(volatile uint32_t *)(0x420D0064));
        *((volatile uint32_t *)0x420F0060) = 0x2D000000;
        *(volatile uint32_t *)(0x420D011C) = 0x11101100; // set gpio 60 to mode 0 (GPIO)
        *(volatile uint32_t *)(0x420D0004) = 0x10000000; // set direction
        log_hal_msgid_info("[PMU_BASIC]toggle_i2c_clk, low 0x420D0064[%x], DIN_GPIO", 1, *(volatile uint32_t *)(0x420D0064));
        hal_gpt_delay_us(100);
        *(volatile uint32_t *)(0x420D0034) = 0x10000000; // gpio 60 output 1
        log_hal_msgid_info("[PMU_BASIC]toggle_i2c_clk, high 0x420D0064[%x], DIN_GPIO", 1, *(volatile uint32_t *)(0x420D0064));
        *(volatile uint32_t *)(0x420D011C) = 0x11111100; // set gpio 60 to mode 1 (i2c clk)
        rst_adie_clk = FALSE;
    }
}
pmu_operate_status_t pmu_set_register_value_lp(uint32_t address, uint32_t mask, uint32_t shift, uint32_t value)
{
    pmu_toggle_i2c_clk();
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
            //log_hal_msgid_error("[PMU_BASIC]set_register_value fail, I2c access PMIC fail", 0);
            assert(0);
        }
    } while ((result_read != 0) && (retry_cnt <= 60));
    return PMU_OK;
}

pmu_operate_status_t pmu_force_set_register_value_lp(uint32_t address, uint32_t value)
{
    unsigned char send_buffer[4];
    unsigned char retry_cnt = 0, result_read;

    pmic_i2c_init();
    send_buffer[0] = ((address >> 8) & 0x00FF) | 0x00;
    send_buffer[1] = (address) & 0x00FF;
    send_buffer[2] = (value & 0xFF);
    send_buffer[3] = ((value >> 8) & 0xFF);

    do {
        result_read = hal_i2c_master_send_polling(HAL_I2C_MASTER_AO, PMIC_SLAVE_ADDR, send_buffer, 4);
        retry_cnt++;
    } while ((result_read != 0) && (retry_cnt <= 60));

    if (result_read != 0) {
        //log_hal_msgid_error("[PMU_BASIC]force_set_register_value fail, addr[0x%X], val[0x%X]", 2, address, value);
        assert(0);
    }

    return PMU_OK;
}

pmu_operate_status_t pmu_i2c_dummy(void)
{
    unsigned char send_buffer[4], receive_buffer[2];
    //uint32_t value;
    //pmic_i2c_init();
    send_buffer[1] = 0x18 & 0x00FF; //D2D 2Byte
    send_buffer[0] = ((0x18 >> 8) & 0x00FF) & 0x0F;
    pmu_d2d_i2c_read(send_buffer, receive_buffer, 2);
    //value = (receive_buffer[1] << 8) + receive_buffer[0];
    //return ((value >> shift) & mask);

    return PMU_OK;
}

pmu_operate_status_t pmu_i2c_push_pull(void)
{
    unsigned char send_buffer[4];
    uint32_t data;
    unsigned char retry_cnt = 0, result_read;

    data = 0x00;//receive_buffer[1];
    data = (data << 8) | 0x07;//receive_buffer[0];
    data &= (~(I2C_DRV_SEL_MASK << I2C_DRV_SEL_SHIFT));
    data = data | (0x1 << I2C_DRV_SEL_SHIFT);

    send_buffer[0] = ((I2C_DRV_SEL_ADDR >> 8) & 0x00FF) | 0x00;
    send_buffer[1] = (I2C_DRV_SEL_ADDR) & 0x00FF;
    send_buffer[2] = (data & 0xFF);
    send_buffer[3] = ((data >> 8) & 0xFF);
    /*log_hal_msgid_info("[PMU_BASIC]i2c_push_pull, send_buffer3[0x%x], send_buffer2[0x%x], send_buffer1[0x%x], send_buffer0[0x%x]",
        4, send_buffer[3], send_buffer[2], send_buffer[1], send_buffer[0]);*/

    do {
        result_read = hal_i2c_master_send_polling(HAL_I2C_MASTER_AO, PMIC_SLAVE_ADDR, send_buffer, 4);
        retry_cnt++;
    } while ((result_read != 0) && (retry_cnt <= 60));

    if (result_read != 0) {
        //log_hal_msgid_error("[PMU_BASIC]i2c_push_pull fail", 0);
        assert(0);
    }
    return PMU_OK;
}
void pmic_i2c_init(void)
{
    if (pmu_i2c_init_sta == 1) {
        return;
    }
    int status;
    hal_i2c_config_t config;
    config.frequency = HAL_I2C_FREQUENCY_400K;
    status = hal_i2c_master_init(HAL_I2C_MASTER_AO, &config);
    if (status != HAL_I2C_STATUS_OK) {
        assert(0);
    }
    pmu_i2c_init_sta = 1;
    pmu_i2c_dummy();
#ifdef HAL_FLASH_MODULE_ENABLED
    if(pmu_get_ver_flag == 0) {
        hal_flash_init();
        pmu_get_pmic_version_lp();
        pmu_get_ver_flag = 1;
    }
    if(pmic_ver != 3){
        pmu_set_register_value_lp(IO_CFG2, 0x7, 4, 0x5); //clk
        pmu_set_register_value_lp(IO_CFG2, 0x7, 0, 0x5); //data
    }
#endif
    pmu_i2c_push_pull();
    hal_gpt_delay_us(200);
    hal_i2c_master_set_io_config(HAL_I2C_MASTER_AO, HAL_I2C_IO_PUSH_PULL);
    //i2c_config_speed(HAL_I2C_MASTER_AO, 400000, 0); // 400K for testing
    i2c_config_speed(HAL_I2C_MASTER_AO, 2000000, 0);
    pmu_i2c_init_sta = 1;
}
void pmic_i2c_deinit(void)
{
    /*Check low power diff*/
    //uint32_t mask_pri;
    //hal_nvic_save_and_set_interrupt_mask_special(&mask_pri);
    hal_i2c_master_deinit(HAL_I2C_MASTER_AO);
    pmu_i2c_init_sta = 0;
    //hal_nvic_restore_interrupt_mask_special(mask_pri);
}
#ifdef AIR_NVDM_ENABLE
#include "nvkey_id_list.h"
#include "nvkey.h"
void pmu_set_dummy_load_lp(pmu_power_domain_t domain, uint32_t loading_value)
{
    int32_t DLH_value = 0, DLL_value = 0;
    int32_t tuning_bit = 0;
    switch (domain) {
        case PMU_BUCK_VCORE:
            if (loading_value == 0){
                pmu_set_register_value_lp(RG_VCORE_RVD_ADDR, 0x1, 10, 0);  //Disable PMU dummy load
                log_hal_msgid_info("[PMU_PWR][Dummy_loading]Disable DL RG_VCORE_RVD[%x]", 1, pmu_get_register_value_lp(RG_VCORE_RVD_ADDR, 0xffff, 0));
            }else{
                otp_buck_dl_t otp;
                if (pmu_get_otp(OTP_BUCK_DL_ADDR, (uint8_t *)&otp, sizeof(otp)) != PMU_STATUS_SUCCESS) {
                    return;
                }
                DLL_value = otp.vcore_dl_k[0].curr;
                DLH_value = otp.vcore_dl_k[1].curr;
                tuning_bit = (30 * loading_value - ( 30 * DLL_value - (DLH_value - DLL_value) ))  / (DLH_value - DLL_value);
                if (tuning_bit < 0) {
                    tuning_bit = 0;
                } else if (tuning_bit > 31) {
                    tuning_bit = 31;
                }
                pmu_set_register_value_lp(RG_VCORE_RVD_ADDR, 0x1F, 11, tuning_bit);
                pmu_set_register_value_lp(RG_VCORE_RVD_ADDR, 0x1, 10, 1);
                log_hal_msgid_info("[PMU_PWR][Dummy_loading][VCORE]loading[%d]DLH[%d]DLL[%d]tuning[%x]RG_VCORE_RVD[%x]", 5 
                                                                ,loading_value, DLH_value, DLL_value, tuning_bit, pmu_get_register_value_lp(RG_VCORE_RVD_ADDR, 0xffff, 0));
            }
            break;
        case PMU_BUCK_VIO18:
            if(loading_value == 0){
                pmu_set_register_value_lp(RG_VIO18_RVD_ADDR, 0x1, 10, 0); //Disable PMU dummy load
                log_hal_msgid_info("[PMU_PWR][Dummy_loading]Disable DL RG_VIO18_RVD[%x]", 1, pmu_get_register_value_lp(RG_VIO18_RVD_ADDR, 0xffff, 0));
            }else{
                otp_buck_dl_t otp;
                if (pmu_get_otp(OTP_BUCK_DL_ADDR, (uint8_t *)&otp, sizeof(otp)) != PMU_STATUS_SUCCESS) {
                    return;
                }
                DLL_value = otp.vio18_dl_k[0].curr;
                DLH_value = otp.vio18_dl_k[1].curr;
                tuning_bit = (30 * loading_value - ( 30 * DLL_value - (DLH_value - DLL_value) ))  / (DLH_value - DLL_value);
                if (tuning_bit < 0) {
                    tuning_bit = 0;
                } else if (tuning_bit > 31) {
                    tuning_bit = 31;
                }
                pmu_set_register_value_lp(RG_VIO18_RVD_ADDR, 0x1F, 11, tuning_bit);
                pmu_set_register_value_lp(RG_VIO18_RVD_ADDR, 0x1, 10, 1);
                log_hal_msgid_info("[PMU_PWR][Dummy_loading][VIO18]loading[%d]DLH[%d]DLL[%d]tuning[%x]RG_VIO18_RVD[%x]", 5
                                                                ,loading_value, DLH_value, DLL_value, tuning_bit, pmu_get_register_value_lp(RG_VIO18_RVD_ADDR, 0xffff, 0));
            }
            break;
        case PMU_BUCK_VRF:
            if(loading_value == 0){
                pmu_set_register_value_lp(RG_VRF_RVD_ADDR, 0x1, 4, 0); //Disable PMU dummy load
                log_hal_msgid_info("[PMU_PWR][Dummy_loading]Disable DL RG_VRF_RVD[%x]", 1, pmu_get_register_value_lp(RG_VRF_RVD_ADDR, 0xffff, 0));
            }else{
                otp_sido_3_t otp;
                if (pmu_get_otp(OTP_SIDO_3_ADDR, (uint8_t *)&otp, sizeof(otp)) != PMU_STATUS_SUCCESS) {
                    return;
                }
                DLL_value = otp.vrf_dl_k[0].curr;
                DLH_value = otp.vrf_dl_k[1].curr;
                tuning_bit = (14 * loading_value - ( 14 * DLL_value - (DLH_value - DLL_value) ))  / (DLH_value - DLL_value);
                if (tuning_bit < 0) {
                    tuning_bit = 0;
                } else if (tuning_bit > 15) {
                    tuning_bit = 15;
                }
                pmu_set_register_value_lp(RG_VRF_RVD2_ADDR, 0xF, 12, tuning_bit);
                pmu_set_register_value_lp(RG_VRF_RVD_ADDR, 0x1, 4, 1);
                log_hal_msgid_info("[PMU_PWR][Dummy_loading][VRF]loading[%d]DLH[%d]DLL[%d]tuning[%x]RG_VRF_RVD[%x]", 5
                                                            , loading_value, DLH_value, DLL_value, pmu_get_register_value_lp(RG_VRF_RVD_ADDR, 0xffff, 0));
            }
            break;
        case PMU_BUCK_VAUD18:
            if(loading_value == 0){
                pmu_set_register_value_lp(RG_VRF_RVD_ADDR, 0x1, 5, 0); //Disable PMU dummy load
                log_hal_msgid_info("[PMU_PWR][Dummy_loading]Disable DL RG_VAUD18_RVD[%x]", 1, pmu_get_register_value_lp(RG_VRF_RVD_ADDR, 0xffff, 0));
            }else{
                otp_sido_3_t otp;
                if (pmu_get_otp(OTP_SIDO_3_ADDR, (uint8_t *)&otp, sizeof(otp)) != PMU_STATUS_SUCCESS) {
                    return;
                }
                DLL_value = otp.vaud18_dl_k[0].curr;
                DLH_value = otp.vaud18_dl_k[1].curr;
                tuning_bit = (14 * loading_value - ( 14 * DLL_value - (DLH_value - DLL_value) ))  / (DLH_value - DLL_value);
                if (tuning_bit < 0) {
                    tuning_bit = 0;
                } else if (tuning_bit > 15) {
                    tuning_bit = 15;
                }
                pmu_set_register_value_lp(RG_VRF_RVD2_ADDR, 0xF, 8, tuning_bit);
                pmu_set_register_value_lp(RG_VRF_RVD_ADDR, 0x1, 5, 1);
                log_hal_msgid_info("[PMU_PWR][Dummy_loading][VAUD18]loading[%d]DLH[%d]DLL[%d]tuning[%x]RG_VAUD18_RVD[%x]", 5
                                                                , loading_value, DLH_value, DLL_value, tuning_bit, pmu_get_register_value_lp(RG_VRF_RVD_ADDR, 0xffff, 0));
            }
            break;
    }
}
#endif /*AIR_NVDM_ENABLE*/
#ifdef PMU_SLT_ENV
void pmu_set_vaud18_pinout(pmu_audio_pinout_t mode)
{
    pmu_set_register_value_lp(AD_VAUD18DRV_VOSEL0_SW_MODE_ADDR, AD_VAUD18DRV_VOSEL0_SW_MODE_MASK, AD_VAUD18DRV_VOSEL0_SW_MODE_SHIFT, 1);
    pmu_set_register_value_lp(AD_VAUD18DRV_VOSEL1_SW_MODE_ADDR, AD_VAUD18DRV_VOSEL1_SW_MODE_MASK, AD_VAUD18DRV_VOSEL1_SW_MODE_SHIFT, 1);
    switch (mode) {
        case PMU_AUDIO_PINOUT_L:
            pmu_set_register_value_lp(AD_VAUD18DRV_VOSEL0_SW_MODE_ADDR, AD_VAUD18DRV_VOSEL0_SW_MODE_MASK, AD_VAUD18DRV_VOSEL0_SW_MODE_SHIFT, 0x0);
            pmu_set_register_value_lp(AD_VAUD18DRV_VOSEL1_SW_MODE_ADDR, AD_VAUD18DRV_VOSEL1_SW_MODE_MASK, AD_VAUD18DRV_VOSEL1_SW_MODE_SHIFT, 0x0);
            log_hal_msgid_info("[PMU_SLT]Vaud18 L", 0);
            break;
        case PMU_AUDIO_PINOUT_M:
            pmu_set_register_value_lp(AD_VAUD18DRV_VOSEL0_SW_MODE_ADDR, AD_VAUD18DRV_VOSEL0_SW_MODE_MASK, AD_VAUD18DRV_VOSEL0_SW_MODE_SHIFT, 0x1);
            pmu_set_register_value_lp(AD_VAUD18DRV_VOSEL1_SW_MODE_ADDR, AD_VAUD18DRV_VOSEL1_SW_MODE_MASK, AD_VAUD18DRV_VOSEL1_SW_MODE_SHIFT, 0x0);
            log_hal_msgid_info("[PMU_SLT]Vaud18 M", 0);
            break;
        case PMU_AUDIO_PINOUT_H:
            pmu_set_register_value_lp(AD_VAUD18DRV_VOSEL0_SW_MODE_ADDR, AD_VAUD18DRV_VOSEL0_SW_MODE_MASK, AD_VAUD18DRV_VOSEL0_SW_MODE_SHIFT, 0x1);
            pmu_set_register_value_lp(AD_VAUD18DRV_VOSEL1_SW_MODE_ADDR, AD_VAUD18DRV_VOSEL1_SW_MODE_MASK, AD_VAUD18DRV_VOSEL1_SW_MODE_SHIFT, 0x1);
            log_hal_msgid_info("[PMU_SLT]Vaud18 H", 0);
            break;
    }
}

uint32_t pmu_modify_4bit_trim_lp(pmu_slt_mode_t oper, uint32_t buck_value, uint32_t divv_value)
{
    log_hal_msgid_info("[PMU_SLT]modify_4bit_trim, 4bit[%d][%x][%x]", 3, oper, buck_value, divv_value);
    uint32_t temp = 0;
    if (oper == PMU_HIGH_LEVEL) { // oper==2 high
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
    } else if (oper == PMU_LOW_LEVEL) { // oper ==0 low
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

uint32_t pmu_modify_5bit_trim_lp(pmu_slt_mode_t oper, uint32_t buck_value, uint32_t divv_value)
{
    log_hal_msgid_info("[PMU_SLT]modify_5bit_trim, 5bit[%d][%x][%x]", 3, oper, buck_value, divv_value);
    uint32_t temp = 0;
    if (oper == PMU_HIGH_LEVEL) { // oper==2 high
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
    } else if (oper == PMU_LOW_LEVEL) { // oper ==0 low
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

/**
 * @brief Switch all power domain to LV, NV, HV mode for SLT.
 *
 * @param[in] mode Select LV, NV, HV mode.
 *   - PMU_LOW_LEVEL:    LV
 *   - PMU_NORMAL_LEVEL: NV
 *   - PMU_HIGH_LEVEL:   HV
 *
 * @remarks
 *    SLT need control Vcore, Vsram by dvfs api.
 *    And control audio pinout, Vpa vlot.
 *
 * Vsram  use 4 bits trim value (2's complement), use pmu_modify_4bit_trim_hp to get new trim value.
 *
 * Vdd31, Vdig18 has different design in Adie low and high.
 * Adie high use trim value. Adie low only have vosel.
 */
void pmu_switch_performance_level(pmu_slt_mode_t mode)
{
    static bool first_time        = true;
    const uint8_t vcbuck_voval[6] = {0x1e, 0x28, 0x50, 0x78, 0x84, 0x98};
    uint8_t vol_index;
    uint32_t temp_value = 0;

    static uint8_t vcore_val    = 0;
    static uint8_t vsram_val    = 0;
    static uint8_t vio18_val    = 0;
    static uint8_t vrf_val      = 0;
    static uint8_t vaud18_h_val = 0;
    static uint8_t vaud18_m_val = 0;
    static uint8_t vaud18_l_val = 0;
    static uint8_t vdig18_val   = 0;
    static uint8_t vdd33_val    = 0;

    const uint8_t vcore_div     = 8;
    const uint8_t vsram_div     = 6;
    const uint8_t vio18_div     = 24;
    const uint8_t vrf_div       = 12;
    const uint8_t vaud18_h_div  = 6;
    const uint8_t vaud18_m_div  = 4;
    const uint8_t vaud18_l_div  = 3;
    const uint8_t vdig18_div    = 14;
    const uint8_t vdd33_div     = 10;

    if (first_time) {
        vsram_val    = pmu_get_register_value_lp(RG_VSRAM_VOTRIM_ADDR, RG_VSRAM_VOTRIM_MASK, RG_VSRAM_VOTRIM_SHIFT);
        vio18_val    = pmu_get_register_value_lp(VIO18_VSEL_NM_ADDR, VIO18_VSEL_NM_MASK, VIO18_VSEL_NM_SHIFT);
        vrf_val      = pmu_get_register_value_lp(VRF_VSEL_NM_ADDR, VRF_VSEL_NM_MASK, VRF_VSEL_NM_SHIFT);
        vaud18_h_val = pmu_get_register_value_lp(VAUD18_VSEL_H_ADDR, VAUD18_VSEL_H_MASK, VAUD18_VSEL_H_SHIFT);
        vaud18_m_val = pmu_get_register_value_lp(VAUD18_VSEL_M_ADDR, VAUD18_VSEL_M_MASK, VAUD18_VSEL_M_SHIFT);
        vaud18_l_val = pmu_get_register_value_lp(VAUD18_VSEL_L_ADDR, VAUD18_VSEL_L_MASK, VAUD18_VSEL_L_SHIFT);
        vdig18_val   = pmu_get_register_value_lp(RG_VDIG18_SEL_ADDR, RG_VDIG18_SEL_MASK, RG_VDIG18_SEL_SHIFT);
        vdd33_val    = pmu_get_register_value_lp(REGHV_SEL_NM_ADDR, REGHV_SEL_NM_MASK, REGHV_SEL_NM_SHIFT);
        first_time   = false;
    }

    /* find vcore lock voltage */
    for (vol_index = PMIC_VCORE_0P85_V; vol_index >= PMIC_VCORE_0P525_V; vol_index--) {
        if (Vcore_Resource_Ctrl[vol_index] != 0) {
            break;
        }
    }
    vcore_val = vcbuck_voval[vol_index];

    if (mode == PMU_NORMAL_LEVEL) {
        pmu_set_register_value_lp(VCORE_VSEL_NM_ADDR, VCORE_VSEL_NM_MASK, VCORE_VSEL_NM_SHIFT, vcore_val);
        pmu_set_register_value_lp(RG_VSRAM_VOTRIM_ADDR, RG_VSRAM_VOTRIM_MASK, RG_VSRAM_VOTRIM_SHIFT, vsram_val);
        pmu_set_register_value_lp(VIO18_VSEL_NM_ADDR, VIO18_VSEL_NM_MASK, VIO18_VSEL_NM_SHIFT, vio18_val);
        pmu_set_register_value_lp(VRF_VSEL_NM_ADDR, VRF_VSEL_NM_MASK, VRF_VSEL_NM_SHIFT, vrf_val);
        pmu_set_register_value_lp(VAUD18_VSEL_H_ADDR, VAUD18_VSEL_H_MASK, VAUD18_VSEL_H_SHIFT, vaud18_h_val);
        pmu_set_register_value_lp(VAUD18_VSEL_M_ADDR, VAUD18_VSEL_M_MASK, VAUD18_VSEL_M_SHIFT, vaud18_m_val);
        pmu_set_register_value_lp(VAUD18_VSEL_L_ADDR, VAUD18_VSEL_L_MASK, VAUD18_VSEL_L_SHIFT, vaud18_l_val);
        pmu_set_register_value_lp(RG_VDIG18_SEL_ADDR, RG_VDIG18_SEL_MASK, RG_VDIG18_SEL_SHIFT, vdig18_val);
        pmu_set_register_value_lp(REGHV_SEL_NM_ADDR, REGHV_SEL_NM_MASK, REGHV_SEL_NM_SHIFT, vdd33_val);
        log_hal_msgid_info("[PMU_SLT]PMU_NORMAL_LEVEL", 0);
    }
    else if (mode == PMU_HIGH_LEVEL) {
        pmu_set_register_value_lp(VCORE_VSEL_NM_ADDR, VCORE_VSEL_NM_MASK, VCORE_VSEL_NM_SHIFT, vcore_val + vcore_div);
        temp_value = pmu_modify_5bit_trim_lp(mode, vsram_val, vsram_div);
        pmu_set_register_value_lp(RG_VSRAM_VOTRIM_ADDR, RG_VSRAM_VOTRIM_MASK, RG_VSRAM_VOTRIM_SHIFT, temp_value);
        pmu_set_register_value_lp(VIO18_VSEL_NM_ADDR, VIO18_VSEL_NM_MASK, VIO18_VSEL_NM_SHIFT, vio18_val + vio18_div);
        pmu_set_register_value_lp(VRF_VSEL_NM_ADDR, VRF_VSEL_NM_MASK, VRF_VSEL_NM_SHIFT, vrf_val + vrf_div);
        pmu_set_register_value_lp(VAUD18_VSEL_H_ADDR, VAUD18_VSEL_H_MASK, VAUD18_VSEL_H_SHIFT, vaud18_h_val + vaud18_h_div);
        pmu_set_register_value_lp(VAUD18_VSEL_M_ADDR, VAUD18_VSEL_M_MASK, VAUD18_VSEL_M_SHIFT, vaud18_m_val + vaud18_m_div);
        pmu_set_register_value_lp(VAUD18_VSEL_L_ADDR, VAUD18_VSEL_L_MASK, VAUD18_VSEL_L_SHIFT, vaud18_l_val + vaud18_l_div);
        pmu_set_register_value_lp(RG_VDIG18_SEL_ADDR, RG_VDIG18_SEL_MASK, RG_VDIG18_SEL_SHIFT, vdig18_val + vdig18_div);
        pmu_set_register_value_lp(REGHV_SEL_NM_ADDR, REGHV_SEL_NM_MASK, REGHV_SEL_NM_SHIFT, vdd33_val + vdd33_div);
        log_hal_msgid_info("[PMU_SLT]PMU_HIGH_LEVEL", 0);
    }
    else if (mode == PMU_LOW_LEVEL) {
        pmu_set_register_value_lp(VCORE_VSEL_NM_ADDR, VCORE_VSEL_NM_MASK, VCORE_VSEL_NM_SHIFT, vcore_val - vcore_div);
        temp_value = pmu_modify_5bit_trim_lp(mode, vsram_val, vsram_div);
        pmu_set_register_value_lp(RG_VSRAM_VOTRIM_ADDR, RG_VSRAM_VOTRIM_MASK, RG_VSRAM_VOTRIM_SHIFT, temp_value);
        pmu_set_register_value_lp(VIO18_VSEL_NM_ADDR, VIO18_VSEL_NM_MASK, VIO18_VSEL_NM_SHIFT, vio18_val - vio18_div);
        pmu_set_register_value_lp(VRF_VSEL_NM_ADDR, VRF_VSEL_NM_MASK, VRF_VSEL_NM_SHIFT, vrf_val - vrf_div);
        pmu_set_register_value_lp(VAUD18_VSEL_H_ADDR, VAUD18_VSEL_H_MASK, VAUD18_VSEL_H_SHIFT, vaud18_h_val - vaud18_h_div);
        pmu_set_register_value_lp(VAUD18_VSEL_M_ADDR, VAUD18_VSEL_M_MASK, VAUD18_VSEL_M_SHIFT, vaud18_m_val - vaud18_m_div);
        pmu_set_register_value_lp(VAUD18_VSEL_L_ADDR, VAUD18_VSEL_L_MASK, VAUD18_VSEL_L_SHIFT, vaud18_l_val - vaud18_l_div);
        pmu_set_register_value_lp(RG_VDIG18_SEL_ADDR, RG_VDIG18_SEL_MASK, RG_VDIG18_SEL_SHIFT, vdig18_val - vdig18_div);
        pmu_set_register_value_lp(REGHV_SEL_NM_ADDR, REGHV_SEL_NM_MASK, REGHV_SEL_NM_SHIFT, vdd33_val - vdd33_div);
        log_hal_msgid_info("[PMU_SLT]PMU_LOW_LEVEL", 0);
    }
    else {
        log_hal_msgid_error("[PMU_SLT]pmu_switch_performance_level unknown mode[%d]", 1, mode);
        assert(0);
    }

    temp_value = pmu_get_register_value_lp(VCORE_VSEL_NM_ADDR, VCORE_VSEL_NM_MASK, VCORE_VSEL_NM_SHIFT);
    log_hal_msgid_info("[PMU_SLT]VCORE, (0x%08x)[0x%X]", 2, VCORE_VSEL_NM_ADDR, temp_value);
    temp_value = pmu_get_register_value_lp(RG_VSRAM_VOTRIM_ADDR, RG_VSRAM_VOTRIM_MASK, RG_VSRAM_VOTRIM_SHIFT);
    log_hal_msgid_info("[PMU_SLT]VSRAM, (0x%08x)[0x%X]", 2, RG_VSRAM_VOTRIM_ADDR, temp_value);
    temp_value = pmu_get_register_value_lp(VIO18_VSEL_NM_ADDR, VIO18_VSEL_NM_MASK, VIO18_VSEL_NM_SHIFT);
    log_hal_msgid_info("[PMU_SLT]VIO18, (0x%08x)[0x%X]", 2, VIO18_VSEL_NM_ADDR, temp_value);
    temp_value = pmu_get_register_value_lp(VRF_VSEL_NM_ADDR, VRF_VSEL_NM_MASK, VRF_VSEL_NM_SHIFT);
    log_hal_msgid_info("[PMU_SLT]VRF, (0x%08x)[0x%X]", 2, VRF_VSEL_NM_ADDR, temp_value);
    temp_value = pmu_get_register_value_lp(VAUD18_VSEL_H_ADDR, VAUD18_VSEL_H_MASK, VAUD18_VSEL_H_SHIFT);
    log_hal_msgid_info("[PMU_SLT]VAUD18_H, (0x%08x)[0x%X]", 2, VAUD18_VSEL_H_ADDR, temp_value);
    temp_value = pmu_get_register_value_lp(VAUD18_VSEL_M_ADDR, VAUD18_VSEL_M_MASK, VAUD18_VSEL_M_SHIFT);
    log_hal_msgid_info("[PMU_SLT]VAUD18_M, (0x%08x)[0x%X]", 2, VAUD18_VSEL_M_ADDR, temp_value);
    temp_value = pmu_get_register_value_lp(VAUD18_VSEL_L_ADDR, VAUD18_VSEL_L_MASK, VAUD18_VSEL_L_SHIFT);
    log_hal_msgid_info("[PMU_SLT]VAUD18_L, (0x%08x)[0x%X]", 2, VAUD18_VSEL_L_ADDR, temp_value);
    temp_value = pmu_get_register_value_lp(RG_VDIG18_SEL_ADDR, RG_VDIG18_SEL_MASK, RG_VDIG18_SEL_SHIFT);
    log_hal_msgid_info("[PMU_SLT]VDIG18, (0x%08x)[0x%X]", 2, RG_VDIG18_SEL_ADDR, temp_value);
    temp_value = pmu_get_register_value_lp(REGHV_SEL_NM_ADDR, REGHV_SEL_NM_MASK, REGHV_SEL_NM_SHIFT);
    log_hal_msgid_info("[PMU_SLT]VDD33, (0x%08x)[0x%X]", 2, REGHV_SEL_NM_ADDR, temp_value);
}
#endif /* PMU_SLT_ENV */
#endif /* HAL_PMU_MODULE_ENABLED */

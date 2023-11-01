/* Copyright Statement:
 *
 * (C) 2022  Airoha Technology Corp. All rights reserved.
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

/* Includes ------------------------------------------------------------------*/
#include "hal_audio.h"
#include "assert.h"
#include <string.h>
#ifdef HAL_PMU_MODULE_ENABLED
#include "hal_pmu.h"
#endif
#if defined(HAL_SLEEP_MANAGER_ENABLED)
    #include "hal_core_status.h"
    #include "hal_spm.h"
    #include "memory_map.h"
#endif
#include "hal_rtc_internal.h"
#include "hal_clock.h"

#if defined(HAL_AUDIO_MODULE_ENABLED)
#include "hal_audio_internal_nvkey_struct.h"
/* Private define ------------------------------------------------------------*/
#define HAL_AUDIO_POWER_SLIM_ENABLED
#define AUDIO_HW_I2S_MASTER_BLOCK_NUMBER   (4)
/* Private typedef -----------------------------------------------------------*/
typedef struct {
    afe_apll_source_t apll_clk;
    uint32_t          in_low_jitter[(AUDIO_SCEANRIO_TYPE_MAX + 31) / 32];
    uint32_t          out_low_jitter[(AUDIO_SCEANRIO_TYPE_MAX + 31) / 32];
} apll_clock_i2s_master_t;

typedef enum {
    APLL_IN_LOW_JITTER,
    APLL_OUT_LOW_JITTER
} apll_clock_i2s_master_in_out_t;

/** @brief Define hires clock frequency. */
typedef enum {
    DL_HIRES_NONE       = 0,
    DL_HIRES_96KHZ      = 1,
    DL_HIRES_192KHZ     = 2,
} hires_clock_frequency_dl_t;

typedef enum {
    HWSRC_HIRES_96KHZ      = 0,
    HWSRC_HIRES_192KHZ     = 1,
} hires_clock_frequency_hwsrc_t;

typedef struct {
    apll_clock_i2s_master_t i2s_master[AUDIO_HW_I2S_MASTER_BLOCK_NUMBER]; /**< Specifies the I2S0/1/2/3 Master Low jitter mode.*/
    bool hi_res_on_hwsrc;                                               /**< Specifies the hires ON/OFF status of hwsrc.*/
    hires_clock_frequency_dl_t clk_mux_hires_dl;                        /**< Specifies the hires clock frequency of downlink hires.*/
    hires_clock_frequency_hwsrc_t clk_mux_hires_hwsrc;                  /**< Specifies the hires clock frequency of hwsrc.*/
    uint32_t amp_delay_off_timer_start_gpt_cnt;
    uint32_t amp_delay_off_timer_stop_gpt_cnt;
    bool     first_boot_flag;
} audio_clock_control_t;


/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static audio_clock_control_t g_audio_clock_control;
/* ------------------------------------------------ Clock Gate Control ---------------------------------------------- */
// example: HAL_AUDIO_DEVICE_DAC_DUAL, HAL_AUDIO_DEVICE_I2S_MASTER  --> AUDIO_AMP_DELAY_OFF_DEVICE_NUMBER (2)
#define AUDIO_AMP_DELAY_OFF_DEVICE_NUMBER  (1)
static const hal_audio_device_t amp_lock_device[AUDIO_AMP_DELAY_OFF_DEVICE_NUMBER] = {
    HAL_AUDIO_DEVICE_DAC_DUAL
    // ,HAL_AUDIO_DEVICE_I2S_MASTER
};
static uint32_t g_dsp_controller_cg[AUDIO_POWER_END][(AUDIO_SCEANRIO_TYPE_MAX + 31) / 32] = {0};
static uint32_t g_dsp_controller_cg_control_bit[AUDIO_SCENARIO_TYPE_END] = {0}; // log the setting of each scenario type
/* ------------------------------------------------------------------------------------------------------------------- */
/* Public variables ----------------------------------------------------------*/
extern HAL_DSP_PARA_AU_AFE_CTRL_t audio_nvdm_HW_config;

/* Private functions ---------------------------------------------------------*/
static uint32_t hal_audio_status_get_clock_gate_setting(audio_scenario_type_t type);
bool hal_audio_status_query_running_flag_except(audio_scenario_type_t type);
/* Public functions ----------------------------------------------------------*/
extern afe_apll_source_t afe_get_apll_by_samplerate(uint32_t samplerate);

/********************************************* Clock CG Usage Example **************************************************
*    1. First enable power
*    if (control) {
*       2. Check the count of clock gate
*       hal_audio_status_check_clock_gate(type, CLOCK_CG, control);
*       if (!hal_audio_status_get_clock_gate_status(CLOCK_CG)) {
*          // Power on Clock CG
*          // Enable Power
*       }
*       hal_audio_status_set_clock_gate(type, CLOCK_CG, control);
*       // hal_audio_status_log_clock_gate_setting(type, CLOCK_CG, control);
*    } else {
*        uint32_t count = hal_audio_status_get_clock_gate_setting(type);
*        if (count & (1UL << CLOCK_CG)) {
*            3. Check the count of clock gate
*            hal_audio_status_check_clock_gate(type, CLOCK_CG, control);
*            // hal_audio_status_log_clock_gate_setting(type, CLOCK_CG, control);
*            hal_audio_status_set_clock_gate(type, CLOCK_CG, control);
*            4. Last disable power
*            if (!hal_audio_status_get_clock_gate_status(CLOCK_CG)) {
*                // Disable Power
*                // Code
*            }
*        }
*    }
***********************************************************************************************************************/

typedef enum {
    CHECK_MODE_EQUAL = 0,      /* equal */
    CHECK_MODE_LESS,           /* less than */
    CHECK_MODE_MORE,           /* more than */
    CHECK_MODE_LESS_EQUAL,     /* less than and equal */
    CHECK_MODE_MORE_EQUAL      /* more than and equal */
} rg_check_mode_t;

/**
 * @brief     This function is used to check the register value and the input value.
 * @param[in] rg_addr is to indicate register address.
 * @param[in] rg_value is input value.
 * @param[in] bit_mask indicate which bits of register are cared.
 * @param[in] mode comparison mode (equal/less than/more than/less than and equal/more than and equal).
 * @return    NONE.
 */
void hal_audio_status_check_rg_value(uint32_t rg_addr, uint32_t rg_value, uint32_t bit_mask, rg_check_mode_t mode)
{
    uint32_t value = *((volatile uint32_t *)(rg_addr));

    switch (mode) {
        case CHECK_MODE_EQUAL:
            if ((value & bit_mask) == (rg_value & bit_mask)) {
                return;
            }
            break;
        case CHECK_MODE_LESS:
            if ((value & bit_mask) < (rg_value & bit_mask)) {
                return;
            }
            break;
        case CHECK_MODE_MORE:
            if ((value & bit_mask) > (rg_value & bit_mask)) {
                return;
            }
            break;
        case CHECK_MODE_LESS_EQUAL:
            if ((value & bit_mask) <= (rg_value & bit_mask)) {
                return;
            }
            break;
        case CHECK_MODE_MORE_EQUAL:
            if ((value & bit_mask) >= (rg_value & bit_mask)) {
                return;
            }
            break;
        default:
            log_hal_msgid_error("[Audio Clock] ERROR: check mode error", 0);
            assert(0);
            break;
    }
    log_hal_msgid_error("[Audio Clock] ERROR:rg[0x%x] check fail! cur_value[0x%x] target value[0x%x] bit mask[0x%x] mode %d", 5,
                        rg_addr,
                        value,
                        rg_value,
                        bit_mask,
                        mode
                       );
    assert(0);
}

static void hal_audio_status_clear_dsp_controlller_i2s_low_jitter(audio_scenario_type_t type, apll_clock_i2s_master_in_out_t in_out)
{
    if ((type < 0) || (type >= AUDIO_SCENARIO_TYPE_END)) {
        log_hal_msgid_error("[Audio Clock] ERROR: type %d", 1,
                            type
                           );
        assert(0);
        return;
    }
    uint32_t block_number = (AUDIO_SCEANRIO_TYPE_MAX + 31) / 32;
    // clear g_audio_clock_control flag
    for (uint32_t j = 0; j < AUDIO_HW_I2S_MASTER_BLOCK_NUMBER; j ++) {
        bool low_jitter = false;
        if (in_out == APLL_IN_LOW_JITTER) {
            g_audio_clock_control.i2s_master[j].in_low_jitter[type / 32] &= ~(1 << (type % 32));
        } else {
            g_audio_clock_control.i2s_master[j].out_low_jitter[type / 32] &= ~(1 << (type % 32));
        }
        for (uint32_t k = 0; k < 2 * block_number; k ++) {
            if (k < block_number) { // 0-3 in
                if (g_audio_clock_control.i2s_master[j].in_low_jitter[k] != 0) {
                    low_jitter = true;
                    break;
                }
            } else { // 4-7 out
                if (g_audio_clock_control.i2s_master[j].out_low_jitter[k - block_number] != 0) {
                    low_jitter = true;
                    break;
                }
            }
        }
        if (!low_jitter) { // i2s master is not using apll, clear it
            g_audio_clock_control.i2s_master[j].apll_clk = AFE_APLL_NONE;
        }
    }
}

/**
 * @brief     This function is used to enable or disable the assigned clock gate when this scenario start!
 * @param[in] type is to indicate the audio scenario which will enable or disable the clock gate.
 * @param[in] cg_type is the clock gate that will be enable or disable.
 * @param[in] control enable or disable clock gate.
 * @return    NONE.
 */
static void hal_audio_status_set_clock_gate(audio_scenario_type_t type, audio_clock_setting_type_t cg_type, bool control)
{
    if ((type < 0) || (type >= AUDIO_SCENARIO_TYPE_END) || (cg_type >= AUDIO_POWER_END) || (cg_type < 0)) {
        log_hal_msgid_error("[Audio Clock] ERROR: type %d cg_type %d", 2,
                            type,
                            cg_type
                           );
        assert(0);
        return;
    }
    // get index
    uint32_t index = type / 32;
    uint32_t bit_mask = type % 32;
    if (control) {
        if (g_dsp_controller_cg[cg_type][index] & (1 << bit_mask)) {
            log_hal_msgid_error("[Audio Clock] ERROR: clock [%d] type [%d] is already enable", 2,
                                cg_type,
                                type);
            log_hal_msgid_error("[Audio Clock] CG %d: [0x%x][0x%x][0x%x][0x%x]", 5,
                                cg_type,
                                g_dsp_controller_cg[cg_type][3],
                                g_dsp_controller_cg[cg_type][2],
                                g_dsp_controller_cg[cg_type][1],
                                g_dsp_controller_cg[cg_type][0]
                               );
            assert(0);
            return;
        }
        g_dsp_controller_cg[cg_type][index] |= (1 << bit_mask);
        g_dsp_controller_cg_control_bit[type] |= (1 << cg_type);
    } else {
        if ((g_dsp_controller_cg[cg_type][index] & (1 << bit_mask)) == 0) {
            log_hal_msgid_error("[Audio Clock] ERROR: clock [%d] type [%d] is already disable", 2,
                                cg_type,
                                type);
            log_hal_msgid_error("[Audio Clock] CG %d: [0x%x][0x%x][0x%x][0x%x]", 5,
                                cg_type,
                                g_dsp_controller_cg[cg_type][3],
                                g_dsp_controller_cg[cg_type][2],
                                g_dsp_controller_cg[cg_type][1],
                                g_dsp_controller_cg[cg_type][0]
                               );
            assert(0);
            return;
        }
        g_dsp_controller_cg[cg_type][index] &= ~(1 << bit_mask);
        g_dsp_controller_cg_control_bit[type] &= ~(1 << cg_type);
    }
}

/**
 * @brief     This function is used to get the status of the clock gate!
 * @param[in] cg_type cg_type is the clock gate.
 * @return    Return #true when this clock gate is used by some scenario.
 *            Return #false when this clock gate is not used by any scenario.
 */
bool hal_audio_status_get_clock_gate_status(audio_clock_setting_type_t cg_type)
{
    for (uint32_t i = 0; i <= (AUDIO_SCENARIO_TYPE_END / 32); i++) {
        if (g_dsp_controller_cg[cg_type][i] != 0) {
            return true; // means this clock gate is enabled by some scenario
        }
    }
    return false; // this clock gate is not used
}

/**
 * @brief     This function is used to watch whether this clock gate is used by this scenario!
 * @param[in] type is to indicate the audio scenario.
 * @param[in] cg_type is the clock gate.
 * @return    Return #true when this clock gate is used by this scenario.
 *            Return #false when this clock gate is not used by this scenario.
 */
bool hal_audio_status_check_clock_gate_status(audio_scenario_type_t type, audio_clock_setting_type_t cg_type)
{
    uint32_t index = type / 32;
    uint32_t bit_mask = type % 32;
    if (g_dsp_controller_cg[cg_type][index] & (1 << bit_mask)) {
        return true;
    }
    return false; // this clock gate is not used
}

/**
 * @brief     This function is used to logging the clock setting of scenario.
 * @param[in] type is to indicate the audio scenario type.
 * @param[in] cg_type is to indicate clock setting type.
 * @param[in] control enable or disable.
 * @return    NONE.
 */
void hal_audio_status_log_clock_gate_setting(audio_scenario_type_t type, audio_clock_setting_type_t cg_type, bool control)
{
    // get the count
    uint32_t cnt = type;
    if (control) {
        g_dsp_controller_cg_control_bit[cnt] |= (1 << cg_type);
    } else {
        g_dsp_controller_cg_control_bit[cnt] &= ~(1 << cg_type);
    }
}

/**
 * @brief     This function is used to get the clock settings of scenario.
 * @param[in] type is to indicate the audio scenario type.
 * @return    The clock settings of scenario.
 */
static uint32_t hal_audio_status_get_clock_gate_setting(audio_scenario_type_t type)
{
    uint32_t cnt = type;
    return g_dsp_controller_cg_control_bit[cnt];
}

/**
 * @brief     This function is used to control dac mode directly.
 * @param[in] control enable or disable.
 * @return    NONE.
 */

void hal_audio_status_enable_dac(bool control)
{

    bool vaud18_use_flag = false;   // only open VAUD18 when DAC is used.

    if(control) {
        switch (audio_nvdm_HW_config.adc_dac_config.ADDA_DAC_Mode_Sel) {
            // 0x1: PMU_Class_AB
            case 0x1:
                pmu_set_vaud18_vout(PMU_VAUD18_0P9_V, PMU_VAUD18_1P3_V, PMU_VAUD18_1P8_V);
                pmu_set_audio_mode(PMU_CLASSAB, PMU_ON);
                vaud18_use_flag = true;
                break;
            // 0x2: PMU_Class_D
            case 0x2:
                pmu_set_vaud18_vout(PMU_VAUD18_0P9_V, PMU_VAUD18_1P3_V, PMU_VAUD18_1P8_V);
                pmu_set_audio_mode(PMU_CLASSD, PMU_ON);
                vaud18_use_flag = true;
                break;
            // 0x3: PMU_Class_G3
            case 0x3:
                pmu_set_vaud18_vout(PMU_VAUD18_0P9_V,PMU_VAUD18_1P3_V,PMU_VAUD18_1P8_V);
                pmu_set_audio_mode(PMU_CLASSG3, PMU_ON);
                vaud18_use_flag = true;
                break;
            // 0x4: Open Class_D
            case 0x4:
                pmu_set_vaud18_vout(PMU_VAUD18_0P9_V,PMU_VAUD18_1P2_V,PMU_VAUD18_1P1_V);
                pmu_set_audio_mode(PMU_CLASSD, PMU_ON);
                vaud18_use_flag = true;
                break;
            // 0x0: PMU_CLASSG2
            case 0x0:
                pmu_set_vaud18_vout(PMU_VAUD18_0P9_V, PMU_VAUD18_1P3_V, PMU_VAUD18_1P8_V);
                pmu_set_audio_mode(PMU_CLASSG2, PMU_ON);
                vaud18_use_flag = true;
                break;
            default:
                // TODO: verify the power consumption
                log_hal_msgid_warning("[Audio Clock] dac class nvkey invalid %d",
                                      1, audio_nvdm_HW_config.adc_dac_config.ADDA_DAC_Mode_Sel);
                // pmu_set_audio_mode(PMU_CLASSG2, PMU_ON);
                break;
        }
#ifdef HAL_PMU_MODULE_ENABLED
#ifndef AIR_AUDIO_EXT_DAC_ENABLE
        if (vaud18_use_flag) {
            pmu_enable_power(PMU_BUCK_VAUD18, PMU_ON);
        }
#else
        (void)(vaud18_use_flag);
#endif /* AIR_AUDIO_EXT_DAC_ENABLE */
#endif /* HAL_PMU_MODULE_ENABLED */

    } else {
        switch (audio_nvdm_HW_config.adc_dac_config.ADDA_DAC_Mode_Sel) {
            case 0x1:
                pmu_set_audio_mode(PMU_CLASSAB, PMU_OFF);
                vaud18_use_flag = true;
                break;
            case 0x2:
                pmu_set_audio_mode(PMU_CLASSD, PMU_OFF);
                vaud18_use_flag = true;
                break;
            case 0x3:
                pmu_set_audio_mode(PMU_CLASSG3, PMU_OFF);
                vaud18_use_flag = true;
                break;
            case 0x4:
                pmu_set_audio_mode(PMU_CLASSD, PMU_OFF);
                vaud18_use_flag = true;
                break;
            case 0x0:
                pmu_set_audio_mode(PMU_CLASSG2, PMU_OFF);
                vaud18_use_flag = true;
                break;
            default:
                //pmu_set_audio_mode(PMU_CLASSG2, PMU_OFF);
                break;
        }
#ifdef HAL_PMU_MODULE_ENABLED
#ifndef AIR_AUDIO_EXT_DAC_ENABLE
        if (vaud18_use_flag) {
            pmu_enable_power(PMU_BUCK_VAUD18, PMU_OFF);
        }
#else
        (void)(vaud18_use_flag);
#endif /* AIR_AUDIO_EXT_DAC_ENABLE */
#endif /* HAL_PMU_MODULE_ENABLED */
    }
}


/**
 * @brief     This function is used to control the releated clock settings or power of dac by scenario.
 * @param[in] type is to indicate the audio scenario type.
 * @param[in] param is the pointer of open parameters.
 * @param[in] control enable or disable.
 * @return    NONE.
 */
void hal_audio_status_set_dac(audio_scenario_type_t type, mcu2dsp_open_param_t *param, bool control)
{
    if ((param == NULL) && (control)) {
        return;
    }
//    bool vaud18_use_flag = false; // only open VAUD18 when DAC is used.
    if (control) {
        // check stream_out type
        if (param->param.stream_out != STREAM_OUT_AFE) {
            return;
        }
        hal_audio_device_t out_device = param->stream_out_param.afe.audio_device;
        if (out_device & HAL_AUDIO_DEVICE_DAC_DUAL) {
            if (!hal_audio_status_get_clock_gate_status(AUDIO_POWER_DAC)) { // Power on DAC
                hal_audio_status_enable_dac(true);
                log_hal_msgid_info("[Audio Clock] enable dac type %d", 1,
                                   type);
            }
            hal_audio_status_set_clock_gate(type, AUDIO_POWER_DAC, true);
        }
    } else {
        uint32_t count = hal_audio_status_get_clock_gate_setting(type);
        if (count & (1 << AUDIO_POWER_DAC)) {
            hal_audio_status_set_clock_gate(type, AUDIO_POWER_DAC, false);
            log_hal_msgid_info("[Audio Clock] disable dac type %d", 1,
                               type);
            if (!hal_audio_status_get_clock_gate_status(AUDIO_POWER_DAC)) { // Power down DAC
                hal_audio_status_enable_dac(false);
            }
        }
    }
}

/**
 * @brief     This function is used to control the releated clock settings or power of micbias by scenario.
 * @param[in] type is to indicate the audio scenario type.
 * @param[in] param is the pointer of open parameters.
 * @param[in] control enable or disable.
 * @return    NONE.
 */
void hal_audio_status_set_micbias(audio_scenario_type_t type, mcu2dsp_open_param_t *param, bool control)
{
    if ((param == NULL) && (control)) {
        return;
    }
    uint32_t ldo_voltage[AUDIO_POWER_MICBIAS_2_NM - AUDIO_POWER_MICBIAS_0_NM + 1];
    ldo_voltage[0] = audio_nvdm_HW_config.adc_dac_config.ADDA_Voice_Bias0_Level;
    ldo_voltage[1] = audio_nvdm_HW_config.adc_dac_config.ADDA_Voice_Bias1_Level;
    ldo_voltage[2] = audio_nvdm_HW_config.adc_dac_config.ADDA_Voice_Bias2_Level;
    uint32_t micbias_block = AUDIO_POWER_MICBIAS_2_NM - AUDIO_POWER_MICBIAS_0_NM + 1;
    bool share_ldo_flag = audio_nvdm_HW_config.adc_dac_config.ADDA_Voice_Bias012_share_LDO;
    pmu_micbias_mode_t pmu_bias_mode = PMIC_MICBIAS_NM;
    bool mic_used_flag = false;
    if (control) {
        hal_audio_device_t *in_device_ptr = &(param->stream_in_param.afe.audio_device);
        hal_audio_micbias_mode_t bias_mode = param->stream_in_param.afe.with_bias_lowpower;
        uint8_t bias_mode_check = (bias_mode == AUDIO_MICBIAS_HIGH_MODE) ? 1 :
                                    (bias_mode == AUDIO_MICBIAS_NORMAL_MODE) ? 2 :
                                        (bias_mode == AUDIO_MICBIAS_LOW_POWER_MODE) ? 3 : 0;
        uint8_t bias_select = param->stream_in_param.afe.bias_select;
        // check device is mic or not? Only amic/dmic will use micbias.
        if (param->param.stream_in == STREAM_IN_AFE) {
            for (uint8_t i = 0; i < 8; i ++) {
                hal_audio_device_t in_device = *(hal_audio_device_t *)(in_device_ptr + i);
                if ((in_device & HAL_AUDIO_DEVICE_MAIN_MIC_DUAL) || (in_device & HAL_AUDIO_DEVICE_DIGITAL_MIC_DUAL)) {
                    mic_used_flag = true;
                    break;
                }
            }
        }
        if (!mic_used_flag) {
            return;
        }
        pmu_bias_mode = (bias_mode == AUDIO_MICBIAS_NORMAL_MODE) ? PMIC_MICBIAS_NM :
                        (bias_mode == AUDIO_MICBIAS_HIGH_MODE)   ? PMIC_MICBIAS_HPM :
                                                                   PMIC_MICBIAS_LP;
        for (uint8_t i = 0; i < micbias_block; i++) { // micbias total number: 3
            pmu_micbias_mode_t current_bias_mode = PMIC_MICBIAS_LP;
            bool micbias_enable = false;
            if (bias_select & (1 << i)) {
                for (uint8_t j = 0; j < bias_mode_check; j ++) {
                    // someone already use higher priority mode for micbias
                    if (hal_audio_status_get_clock_gate_status(AUDIO_POWER_MICBIAS_0_HP + micbias_block * j + i)) {
                        micbias_enable = true;
                        current_bias_mode = (j == 0) ? PMIC_MICBIAS_HPM :
                                            (j == 1) ? PMIC_MICBIAS_NM:
                                                       PMIC_MICBIAS_LP;
                        break;
                    }
                }
                if (!micbias_enable) {
                    // NOTE: We must set voltage and then power on the LDO.
                    pmu_set_micbias_vout(PMIC_MICBIAS0 + i, ldo_voltage[i]);
                }
                // change micbias mode
                if (pmu_bias_mode > current_bias_mode) {
                    // we should maintain the count of pmu_enable_micbias
                    current_bias_mode = pmu_bias_mode;
                }
                if (share_ldo_flag) {
                    pmu_set_micbias_ldo_mode(PMIC_MICBIAS_LDO0, current_bias_mode);
                    if (!hal_audio_status_get_clock_gate_status(AUDIO_POWER_MICBIAS_0_HP + micbias_block * (2 - pmu_bias_mode) + i)) {
                        pmu_enable_micbias(PMIC_MICBIAS_LDO0, PMIC_MICBIAS0_SHARE_ENABLE, current_bias_mode, PMU_ON);
                        if (i == 1) {
                            pmu_enable_micbias(PMIC_MICBIAS_LDO0, PMIC_MICBIAS0_SHARE_1, current_bias_mode, PMU_ON);
                            if (!hal_audio_status_get_clock_gate_status(AUDIO_POWER_MICBIAS_0_HP + micbias_block * (2 - pmu_bias_mode))) {
                                hal_audio_status_set_clock_gate(type, AUDIO_POWER_MICBIAS_0_HP + micbias_block * (2 - pmu_bias_mode), control);
                            }
                        } else if (i == 2) {
                            pmu_enable_micbias(PMIC_MICBIAS_LDO0, PMIC_MICBIAS0_SHARE_2, current_bias_mode, PMU_ON);
                            if (!hal_audio_status_get_clock_gate_status(AUDIO_POWER_MICBIAS_0_HP + micbias_block * (2 - pmu_bias_mode))) {
                                hal_audio_status_set_clock_gate(type, AUDIO_POWER_MICBIAS_0_HP + micbias_block * (2 - pmu_bias_mode), control);
                            }
                        } else if (i !=0 ) {
                            assert(0 && "[Audio Clock] only support 3 micbias!");
                        }
                    }
                } else {
                    pmu_set_micbias_ldo_mode(PMIC_MICBIAS_LDO0 + i, current_bias_mode);
                    if (!hal_audio_status_get_clock_gate_status(AUDIO_POWER_MICBIAS_0_HP + micbias_block * (2 - pmu_bias_mode) + i)) {
                        pmu_enable_micbias(PMIC_MICBIAS_LDO0 + i, PMIC_MICBIAS0 + i, current_bias_mode, PMU_ON); // care count
                    }
                }

                hal_audio_status_set_clock_gate(type, AUDIO_POWER_MICBIAS_0_HP + micbias_block * (2 - pmu_bias_mode) + i, control); // only set bit
                log_hal_msgid_info("[Audio Clock] micbias_%d type %d control %d bias_select 0x%x voltage 0x%x share_ldo0 0x%x cur_mode %d bias_mode %d power %d enable %d", 10,
                        i,
                        type,
                        control,
                        bias_select,
                        ldo_voltage[i],
                        audio_nvdm_HW_config.adc_dac_config.ADDA_Voice_Bias012_share_LDO,
                        param->stream_in_param.afe.with_bias_lowpower,
                        current_bias_mode,
                        AUDIO_POWER_MICBIAS_0_HP + micbias_block * (2 - pmu_bias_mode) + i,
                        micbias_enable
                        );
            }
        }
        if (share_ldo_flag) {
            hal_audio_status_set_clock_gate(type, AUDIO_POWER_MICBIAS_SHARE, control);
        }
    } else {
        /* Attention: we have never close share1/2 */
        uint32_t count = hal_audio_status_get_clock_gate_setting(type);
        bool share_ldo_flag = hal_audio_status_check_clock_gate_status(type, AUDIO_POWER_MICBIAS_SHARE);
        if (share_ldo_flag) {
            hal_audio_status_set_clock_gate(type, AUDIO_POWER_MICBIAS_SHARE, control);
        }
        for (uint8_t i = 0; i < micbias_block; i++) { // micbias total number: 3
            if (count & (1UL << (AUDIO_POWER_MICBIAS_0_HP + i))) {
                hal_audio_status_set_clock_gate(type, AUDIO_POWER_MICBIAS_0_HP + i, control);
                if (!hal_audio_status_get_clock_gate_status(AUDIO_POWER_MICBIAS_0_HP + i)) {
                    // check lower priority mode
                    if (hal_audio_status_get_clock_gate_status(AUDIO_POWER_MICBIAS_0_NM + i)) {
                        log_hal_msgid_info("[Audio Clock] change micbias_%d mode from hp to nm share %d", 2, i, share_ldo_flag);
                        if (share_ldo_flag) {
                            pmu_set_micbias_ldo_mode(PMIC_MICBIAS_LDO0, PMIC_MICBIAS_NM);
                        } else {
                            pmu_set_micbias_ldo_mode(PMIC_MICBIAS_LDO0 + i, PMIC_MICBIAS_NM);
                        }
                    } else if (hal_audio_status_get_clock_gate_status(AUDIO_POWER_MICBIAS_0_LP + i)) {
                        log_hal_msgid_info("[Audio Clock] change micbias_%d mode from hp to lp share %d", 2, i, share_ldo_flag);
                        if (share_ldo_flag) {
                            pmu_set_micbias_ldo_mode(PMIC_MICBIAS_LDO0, PMIC_MICBIAS_LP);
                        } else {
                            pmu_set_micbias_ldo_mode(PMIC_MICBIAS_LDO0 + i, PMIC_MICBIAS_LP);
                        }
                    } else {
                        if (!share_ldo_flag) {
                            pmu_enable_micbias(PMIC_MICBIAS_LDO0 + i, PMIC_MICBIAS0 + i, PMIC_MICBIAS_HPM, PMU_OFF);
                        } else {
                            pmu_enable_micbias(PMIC_MICBIAS_LDO0, PMIC_MICBIAS0_SHARE_ENABLE, PMIC_MICBIAS_HPM, PMU_OFF);
                        }
                    }
                }
            }
            if (count & (1UL << (AUDIO_POWER_MICBIAS_0_NM + i))) {
                hal_audio_status_set_clock_gate(type, AUDIO_POWER_MICBIAS_0_NM + i, control);
                if (!hal_audio_status_get_clock_gate_status(AUDIO_POWER_MICBIAS_0_NM + i)) {
                    // check lower priority mode
                    if (hal_audio_status_get_clock_gate_status(AUDIO_POWER_MICBIAS_0_LP + i)) {
                        log_hal_msgid_info("[Audio Clock] change micbias_%d mode from nm to lp share %d", 2, i, share_ldo_flag);
                        if (share_ldo_flag) {
                            pmu_set_micbias_ldo_mode(PMIC_MICBIAS_LDO0, PMIC_MICBIAS_LP);
                        } else {
                            pmu_set_micbias_ldo_mode(PMIC_MICBIAS_LDO0 + i, PMIC_MICBIAS_LP);
                        }
                    } else {
                        if (!share_ldo_flag) {
                            pmu_enable_micbias(PMIC_MICBIAS_LDO0 + i, PMIC_MICBIAS0 + i, PMIC_MICBIAS_NM, PMU_OFF);
                        } else {
                            pmu_enable_micbias(PMIC_MICBIAS_LDO0, PMIC_MICBIAS0_SHARE_ENABLE, PMIC_MICBIAS_NM, PMU_OFF);
                        }
                    }
                }
            }
            if (count & (1UL << (AUDIO_POWER_MICBIAS_0_LP + i))) {
                hal_audio_status_set_clock_gate(type, AUDIO_POWER_MICBIAS_0_LP + i, control);
                if (!hal_audio_status_get_clock_gate_status(AUDIO_POWER_MICBIAS_0_LP + i)) {
                    if (!share_ldo_flag) {
                        pmu_enable_micbias(PMIC_MICBIAS_LDO0 + i, PMIC_MICBIAS0 + i, PMIC_MICBIAS_LP, PMU_OFF);
                    } else {
                        pmu_enable_micbias(PMIC_MICBIAS_LDO0, PMIC_MICBIAS0_SHARE_ENABLE, PMIC_MICBIAS_LP, PMU_OFF);
                    }
                }
            }
        }
    }
}

/**
 * @brief     This function is used to control the releated clock settings or power of hi-res in Uplink by scenario.
 * @param[in] type is to indicate the audio scenario type.
 * @param[in] param is the pointer of open parameters.
 * @param[in] control enable or disable.
 * @return    NONE.
 */
void hal_audio_status_set_ul_hires(audio_scenario_type_t type, mcu2dsp_open_param_t *param, bool control)
{
    if ((param == NULL) && (control)) {
        return;
    }
    bool hires_flag = false;
    if (control) {
        // check stream_in type, avoid downlink run this code!
        // Only ADC 0/1 support Hi-Res!
        if ((param->param.stream_in != STREAM_IN_AFE) || (param->stream_in_param.afe.audio_interface != HAL_AUDIO_INTERFACE_2)) {
            return;
        }
        uint32_t device_in_rate = param->stream_in_param.afe.sampling_rate;
        hal_audio_device_t in_device = param->stream_in_param.afe.audio_device;
        if (((in_device & (HAL_AUDIO_DEVICE_MAIN_MIC_DUAL|HAL_AUDIO_DEVICE_LINEINPLAYBACK_DUAL|HAL_AUDIO_DEVICE_DIGITAL_MIC_DUAL)) && (device_in_rate > 48000)) /* ||
            (hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_UPLINK)) */) {
            if (!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_UPLINK)) {
#if defined(HAL_DVFS_MODULE_ENABLED)
                hal_dvfs_lock_control(HAL_DVFS_OPP_MID, HAL_DVFS_LOCK);
                log_hal_msgid_info("[Audio Clock] ul hi-res: type %d lock frequency to 0.8V, rate %u, device 0x%x", 3,
                                   type,
                                   device_in_rate,
                                   in_device);
#endif
                // use lock dvfs to ensure clk source
                // clock_mux_sel(CLK_AUD_ULCK_SEL, 1); /* MPLL_D3, 208 MHz */
                hal_clock_enable(HAL_CLOCK_CG_AUD_UPLINK);
            }
            hal_audio_status_set_clock_gate(type, AUDIO_CLOCK_UPLINK, control);
        }
    } else {
        uint32_t count = hal_audio_status_get_clock_gate_setting(type);
        if (count & (1UL << AUDIO_CLOCK_UPLINK)) {
            hal_audio_status_set_clock_gate(type, AUDIO_CLOCK_UPLINK, control);
            if (!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_UPLINK)) {
                // Check UL Hi-res mode, AFE_ADDA_UL_SRC_CON0[19:17] <= 3
                // hal_audio_status_check_rg_value(AFE_ADDA_UL_SRC_CON0, 0x30000, 0xE0000, CHECK_MODE_LESS_EQUAL);
                hal_clock_disable(HAL_CLOCK_CG_AUD_UPLINK);
#if defined(HAL_DVFS_MODULE_ENABLED)
                hal_dvfs_lock_control(HAL_DVFS_OPP_MID, HAL_DVFS_UNLOCK);
                log_hal_msgid_info("[Audio Clock] ul hi-res: type %d unlock frequency 0.8V", 1,
                                   type);
#endif
            }
        }
    }
    if (hires_flag) {
        log_hal_msgid_info("[Audio Clock] ul hi-res: type %d control %d", 2,
                           type,
                           control);
    }
}

/**
 * @brief     This function is used to control the releated clock settings or power of hi-res in Downlink by scenario.
 * @param[in] type is to indicate the audio scenario type.
 * @param[in] param is the pointer of open parameters.
 * @param[in] control enable or disable.
 * @return    NONE.
 */
void hal_audio_status_set_dl_hires(audio_scenario_type_t type, mcu2dsp_open_param_t *param, bool control)
{
    if ((param == NULL) && (control)) {
        return;
    }
    bool hires_flag = false;
    /* NOTE: We should disable dl hi-res after closing dac! */
    if (control) {
        // check stream_out type, avoid uplink run this code!
        if (param->param.stream_out != STREAM_OUT_AFE) {
            return;
        }
        uint32_t device_out_rate = param->stream_out_param.afe.sampling_rate;
        hal_audio_device_t out_device = param->stream_out_param.afe.audio_device;
        if (((out_device & HAL_AUDIO_DEVICE_DAC_DUAL) && (device_out_rate > 48000)) ||
            (hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_DWLINK))) { // hi-res
            hires_flag = true;
            if (device_out_rate > 96000) {
                if (g_audio_clock_control.clk_mux_hires_dl == DL_HIRES_96KHZ) {
                    log_hal_msgid_error("[Audio Clock] ERROR: DL Hi-Res 96KHz is already on, type %d sample rate %d device 0x%x", 3,
                                        type,
                                        device_out_rate,
                                        out_device);
                    // assert(0);
                }
                g_audio_clock_control.clk_mux_hires_dl = DL_HIRES_192KHZ;
                g_audio_clock_control.clk_mux_hires_hwsrc = HWSRC_HIRES_192KHZ;
                /* MPLL_D3_D2, 104 MHz */
                // USE LOCK dvfs to ensure CLK source.
                // clock_mux_sel(CLK_AUD_GPSRC_SEL, 2);
            } else {
                if (g_audio_clock_control.clk_mux_hires_dl == DL_HIRES_192KHZ) {
                    log_hal_msgid_error("[Audio Clock] ERROR: DL Hi-Res 192KHz is already on, type %d sample rate %d device 0x%x", 3,
                                        type,
                                        device_out_rate,
                                        out_device);
                    // assert(0);
                }
                g_audio_clock_control.clk_mux_hires_dl = DL_HIRES_96KHZ;
                g_audio_clock_control.clk_mux_hires_hwsrc = HWSRC_HIRES_96KHZ;
                /* MPLL_D3_D04, 52 MHz */
                // USE LOCK dvfs to ensure CLK source.
                // clock_mux_sel(CLK_AUD_GPSRC_SEL, 1);
            }
            if (!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_DWLINK)) { // Enable Hi-Res
                if (g_audio_clock_control.clk_mux_hires_dl == DL_HIRES_192KHZ) {
#if defined(HAL_DVFS_MODULE_ENABLED)
                    hal_dvfs_lock_control(HAL_DVFS_OPP_MID, HAL_DVFS_LOCK);
                    log_hal_msgid_info("[Audio Clock] dl hi-res: type %d lock frequency to 0.8V, rate %u, device 0x%x", 3,
                                       type,
                                       device_out_rate,
                                       out_device);
#endif
                } else if (g_audio_clock_control.clk_mux_hires_dl == DL_HIRES_96KHZ) {
#if defined(HAL_DVFS_MODULE_ENABLED)
                    hal_dvfs_lock_control(HAL_DVFS_OPP_LOW, HAL_DVFS_LOCK);
                    log_hal_msgid_info("[Audio Clock] dl hi-res: type %d lock frequency to 0.7V, rate %u, device 0x%x", 3,
                                       type,
                                       device_out_rate,
                                       out_device);
#endif
                } else {
                    log_hal_msgid_error("[Audio Clock] ERROR: abnormal case!", 0);
                    assert(0);
                }
                g_audio_clock_control.hi_res_on_hwsrc = control;
                log_hal_msgid_info("[Audio Clock] dl hi-res: enable hi-res hwsrc!", 0);
                /* MPLL_D5, 124.8 MHz */
                // USE LOCK dvfs to ensure CLK source.
                // clock_mux_sel(CLK_AUD_DLCK_SEL, 1);
                hal_clock_enable(HAL_CLOCK_CG_AUD_DWLINK);
            }
            hal_audio_status_set_clock_gate(type, AUDIO_CLOCK_DWLINK, control);
        }
    } else {
        uint32_t count = hal_audio_status_get_clock_gate_setting(type);
        if (count & (1UL << AUDIO_CLOCK_DWLINK)) {
            hires_flag = true;
            hal_audio_status_set_clock_gate(type, AUDIO_CLOCK_DWLINK, control);
            if (!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_DWLINK)) { // Disable Hi-Res

                // Check DL Hi-res mode, AFE_ADDA_DL_SRC2_CON0[31:28] <= 8
                // hal_audio_status_check_rg_value(AFE_ADDA_DL_SRC2_CON0, 0x80000000, 0xF0000000, CHECK_MODE_LESS_EQUAL);

                if (g_audio_clock_control.clk_mux_hires_dl == DL_HIRES_192KHZ) {
#if defined(HAL_DVFS_MODULE_ENABLED)
                    hal_dvfs_lock_control(HAL_DVFS_OPP_MID, HAL_DVFS_UNLOCK);
                    log_hal_msgid_info("[Audio Clock] dl hi-res: type %d unlock frequency 0.8V", 1,
                                       type);
#endif
                } else if (g_audio_clock_control.clk_mux_hires_dl == DL_HIRES_96KHZ) {
#if defined(HAL_DVFS_MODULE_ENABLED)
                    hal_dvfs_lock_control(HAL_DVFS_OPP_LOW, HAL_DVFS_UNLOCK);
                    log_hal_msgid_info("[Audio Clock] dl hi-res: type %d unlock frequency 0.7V", 1,
                                       type);
#endif
                } else {
                    log_hal_msgid_error("[Audio Clock] ERROR: abnormal case!", 0);
                    assert(0);
                }
                hal_clock_disable(HAL_CLOCK_CG_AUD_DWLINK);
                g_audio_clock_control.clk_mux_hires_dl = DL_HIRES_NONE;
                if (g_audio_clock_control.hi_res_on_hwsrc) {
                    /* F_FXO_CK, 26 MHz */
                    // clock_mux_sel(CLK_AUD_GPSRC_SEL, 0);
                    g_audio_clock_control.hi_res_on_hwsrc = control;
                    log_hal_msgid_info("[Audio Clock] dl hi-res: disable hi-res hwsrc!", 0);
                }
            }
        }
    }
    if (hires_flag) {
        log_hal_msgid_info("[Audio Clock] dl hi-res: type %d control %d", 2,
                           type,
                           control);
    }
}

/**
 * @brief     This function is used to control the releated clock settings or power of APLL in Uplink by scenario.
 * @param[in] type is to indicate the audio scenario type.
 * @param[in] param is the pointer of open parameters.
 * @param[in] control enable or disable.
 * @return    NONE.
 */
void hal_audio_status_set_i2s_master_in_low_jitter_mode(audio_scenario_type_t type, mcu2dsp_open_param_t *param, bool control)
{
    if ((param == NULL) && (control)) {
        return;
    }
    if (control) {
        // check stream_in type, avoid downlink run this code!
        if (param->param.stream_in != STREAM_IN_AFE) {
            return;
        }
        uint8_t intf0_used_flag = 0;
        uint8_t intf1_used_flag = 0;
        uint32_t device_in_rate = param->stream_in_param.afe.sampling_rate;
        uint32_t i2s_in_rate[4] = {0};
        i2s_in_rate[0] = param->stream_in_param.afe.i2s_master_sampling_rate[0];
        i2s_in_rate[1] = param->stream_in_param.afe.i2s_master_sampling_rate[1];
        i2s_in_rate[2] = param->stream_in_param.afe.i2s_master_sampling_rate[2];
        i2s_in_rate[3] = param->stream_in_param.afe.i2s_master_sampling_rate[3];
        hal_audio_device_t in_device = param->stream_in_param.afe.audio_device;
        hal_audio_interface_t device_interface = param->stream_in_param.afe.audio_interface;
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
        hal_audio_device_t in_device1 = param->stream_in_param.afe.audio_device1;
        hal_audio_device_t in_device2 = param->stream_in_param.afe.audio_device2;
        hal_audio_device_t in_device3 = param->stream_in_param.afe.audio_device3;
        hal_audio_interface_t device_interface1 = param->stream_in_param.afe.audio_interface1;
        hal_audio_interface_t device_interface2 = param->stream_in_param.afe.audio_interface2;
        hal_audio_interface_t device_interface3 = param->stream_in_param.afe.audio_interface3;
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
        hal_audio_device_t in_device4 = param->stream_in_param.afe.audio_device4;
        hal_audio_device_t in_device5 = param->stream_in_param.afe.audio_device5;
        hal_audio_device_t in_device6 = param->stream_in_param.afe.audio_device6;
        hal_audio_device_t in_device7 = param->stream_in_param.afe.audio_device7;
        hal_audio_interface_t device_interface4 = param->stream_in_param.afe.audio_interface4;
        hal_audio_interface_t device_interface5 = param->stream_in_param.afe.audio_interface5;
        hal_audio_interface_t device_interface6 = param->stream_in_param.afe.audio_interface6;
        hal_audio_interface_t device_interface7 = param->stream_in_param.afe.audio_interface7;
#endif
#endif
        /* -1-  Get I2S Master Interface ---------------------------------------------------------------------------- */
        int32_t interface_number = -1; // -1 is used for error check
        hal_audio_interface_t interface[8] = {0};
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
        if ((device_interface7 != HAL_AUDIO_INTERFACE_NONE) &&
            ((in_device7 == HAL_AUDIO_DEVICE_I2S_MASTER)    ||
             (in_device7 == HAL_AUDIO_DEVICE_I2S_MASTER_L)   ||
             (in_device7 == HAL_AUDIO_DEVICE_I2S_MASTER_R))) {
            interface_number += 1;
            interface[interface_number] = device_interface7;
        }
        if ((device_interface6 != HAL_AUDIO_INTERFACE_NONE) &&
            ((in_device6 == HAL_AUDIO_DEVICE_I2S_MASTER)    ||
             (in_device6 == HAL_AUDIO_DEVICE_I2S_MASTER_L)   ||
             (in_device6 == HAL_AUDIO_DEVICE_I2S_MASTER_R))) {
            interface_number += 1;
            interface[interface_number] = device_interface6;
        }
        if ((device_interface5 != HAL_AUDIO_INTERFACE_NONE) &&
            ((in_device5 == HAL_AUDIO_DEVICE_I2S_MASTER)    ||
             (in_device5 == HAL_AUDIO_DEVICE_I2S_MASTER_L)   ||
             (in_device5 == HAL_AUDIO_DEVICE_I2S_MASTER_R))) {
            interface_number += 1;
            interface[interface_number] = device_interface5;
        }
        if ((device_interface4 != HAL_AUDIO_INTERFACE_NONE) &&
            ((in_device4 == HAL_AUDIO_DEVICE_I2S_MASTER)    ||
             (in_device4 == HAL_AUDIO_DEVICE_I2S_MASTER_L)   ||
             (in_device4 == HAL_AUDIO_DEVICE_I2S_MASTER_R))) {
            interface_number += 1;
            interface[interface_number] = device_interface4;
        }
#endif
        if ((device_interface3 != HAL_AUDIO_INTERFACE_NONE) &&
            ((in_device3 == HAL_AUDIO_DEVICE_I2S_MASTER)    ||
             (in_device3 == HAL_AUDIO_DEVICE_I2S_MASTER_L)   ||
             (in_device3 == HAL_AUDIO_DEVICE_I2S_MASTER_R))) {
            interface_number += 1;
            interface[interface_number] = device_interface3;
        }
        if ((device_interface2 != HAL_AUDIO_INTERFACE_NONE) &&
            ((in_device2 == HAL_AUDIO_DEVICE_I2S_MASTER)    ||
             (in_device2 == HAL_AUDIO_DEVICE_I2S_MASTER_L)   ||
             (in_device2 == HAL_AUDIO_DEVICE_I2S_MASTER_R))) {
            interface_number += 1;
            interface[interface_number] = device_interface2;
        }
        if ((device_interface1 != HAL_AUDIO_INTERFACE_NONE) &&
            ((in_device1 == HAL_AUDIO_DEVICE_I2S_MASTER)    ||
             (in_device1 == HAL_AUDIO_DEVICE_I2S_MASTER_L)   ||
             (in_device1 == HAL_AUDIO_DEVICE_I2S_MASTER_R))) {
            interface_number += 1;
            interface[interface_number] = device_interface1;
        }
#endif
        if ((device_interface != HAL_AUDIO_INTERFACE_NONE) &&
            ((in_device == HAL_AUDIO_DEVICE_I2S_MASTER)    ||
             (in_device == HAL_AUDIO_DEVICE_I2S_MASTER_L)   ||
             (in_device == HAL_AUDIO_DEVICE_I2S_MASTER_R))) {
            interface_number += 1;
            interface[interface_number] = device_interface;
        }
        if (interface_number == -1) {
            log_hal_msgid_info("[Audio Clock] low jitter mode: i2s in device is not suitable, type %d device 0x%x", 2,
                               type,
                               in_device);
            return;
        }
        for (uint32_t cnt = 0; cnt <= interface_number; cnt ++) {
            uint8_t i2s_number = 0;
            switch (interface[cnt]) {
                case HAL_AUDIO_INTERFACE_1: // I2S MASTER 0
                    i2s_number = 0;
                    break;
                case HAL_AUDIO_INTERFACE_2: // I2S MASTER 1
                    i2s_number = 1;
                    break;
                case HAL_AUDIO_INTERFACE_3: // I2S MASTER 2
                    i2s_number = 2;
                    break;
                case HAL_AUDIO_INTERFACE_4: // I2S MASTER 3
                    i2s_number = 3;
                    break;
                default:
                    log_hal_msgid_error("[Audio Clock] ERROR: low jitter mode, i2s in interface error. type %d interface %d", 2,
                                        type,
                                        interface[cnt]);
                    assert(0);
                    return;
            }
            uint32_t valid_rate = i2s_in_rate[i2s_number] == 0 ? device_in_rate : i2s_in_rate[i2s_number];
            bool is_low_jitter_current_i2s = false;
            // Now i2s master's apll clk source is already used!
            afe_apll_source_t apll_clk = afe_get_apll_by_samplerate(valid_rate);
            bool is_vp_exist = hal_audio_status_check_clock_gate_status(AUDIO_SCENARIO_TYPE_VP, AUDIO_CLOCK_INT);
            if ((!is_vp_exist) && (g_audio_clock_control.i2s_master[i2s_number].apll_clk > 0) && (g_audio_clock_control.i2s_master[i2s_number].apll_clk != apll_clk)) {
                log_hal_msgid_warning("[Audio Clock] WARNING: low jitter mode, i2s[%d]_in is already used by apll%d scenario type[%d] fs %d", 4,
                                        i2s_number,
                                        g_audio_clock_control.i2s_master[i2s_number].apll_clk,
                                        type,
                                        valid_rate
                                        );
                if (valid_rate > 48000) {
                    log_hal_msgid_error("[Audio Clock] ERROR: low jitter mode error", 0);
                    //assert(0);
                    return;
                } else {
                    return;
                }
            }
            if (g_audio_clock_control.i2s_master[i2s_number].apll_clk != AFE_APLL_NONE) {
                is_low_jitter_current_i2s = true;
            }
            if ((is_low_jitter_current_i2s) || (valid_rate > 48000)) {
                param->stream_in_param.afe.is_low_jitter[i2s_number] = true;
            }
            if (!param->stream_in_param.afe.is_low_jitter[i2s_number]) {
                continue;
            }
            log_hal_msgid_info("[Audio Clock] low jitter mode: i2s in type %d interface 0x%x i2s_id %d rate %d", 4,
                               type,
                               interface[cnt],
                               i2s_number,
                               valid_rate);
            /* Choose APLL 1/2 */
            audio_clock_setting_type_t cg_type = (apll_clk == AFE_APLL1) ? AUDIO_CLOCK_INTF1_IN : AUDIO_CLOCK_INTF0_IN;
            g_audio_clock_control.i2s_master[i2s_number].in_low_jitter[type / 32] |= 1 << (type % 32); // log the i2s setting of each scenario
            if (g_audio_clock_control.i2s_master[i2s_number].apll_clk != apll_clk) {
                log_hal_msgid_warning("[Audio Clock] i2s_%d apll clk will be overwrite by %d != %d", 3, i2s_number, apll_clk,
                    g_audio_clock_control.i2s_master[i2s_number].apll_clk);
            }
            g_audio_clock_control.i2s_master[i2s_number].apll_clk = apll_clk; // overwrite it
            if (cg_type == AUDIO_CLOCK_INTF1_IN) {
                intf1_used_flag = 1 << i2s_number;
            } else {
                intf0_used_flag = 1 << i2s_number;
            }
        }
        if (intf0_used_flag) {
            if ((!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_INTF0_IN)) &&
                (!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_INTF0_IN + 2))) {
                clock_mux_sel(CLK_AUD_INTERFACE0_SEL, 3); // 3:APLL2_CK, 24.576 / 22.579 MHz
                log_hal_msgid_info("[Audio Clock] enable APLL2", 0);
                hal_clock_enable(HAL_CLOCK_CG_AUD_INTF0);
            }
            hal_audio_status_set_clock_gate(type, AUDIO_CLOCK_INTF0_IN, control);
        }
        if (intf1_used_flag) {
            if ((!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_INTF1_IN)) &&
                (!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_INTF1_IN + 2))) {
                clock_mux_sel(CLK_AUD_INTERFACE1_SEL, 2);
                log_hal_msgid_info("[Audio Clock] enable APLL1", 0);
                hal_clock_enable(HAL_CLOCK_CG_AUD_INTF1);
            }
            hal_audio_status_set_clock_gate(type, AUDIO_CLOCK_INTF1_IN, control);
        }
    } else {
        uint32_t count = hal_audio_status_get_clock_gate_setting(type);
        for (uint32_t i = 0; i < (AUDIO_CLOCK_INTF1_IN - AUDIO_CLOCK_INTF0_IN + 1); i ++) {
            if (count & (1UL << (AUDIO_CLOCK_INTF0_IN + i))) {
                hal_audio_status_set_clock_gate(type, AUDIO_CLOCK_INTF0_IN + i, control);
                if ((!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_INTF0_IN + i)) &&
                    (!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_INTF0_OUT + i))) {
                    if (i == 0) {
                        // Check APLL2 mode, AUDIO_TOP_CON0[9] = 1 && AFE_APLL2_TUNER_CFG[0] = 0
                        hal_audio_status_check_rg_value(AUDIO_TOP_CON0, 0x200, 0x200, CHECK_MODE_EQUAL);
                        hal_audio_status_check_rg_value(AFE_APLL2_TUNER_CFG, 0, 0, CHECK_MODE_EQUAL);
                        log_hal_msgid_info("[Audio Clock] disable APLL2", 0);
                        hal_clock_disable(HAL_CLOCK_CG_AUD_INTF0);
                    } else if (i == 1) {
                        // Check APLL1 mode, AUDIO_TOP_CON0[8] = 1 && AFE_APLL1_TUNER_CFG[0] = 0
                        hal_audio_status_check_rg_value(AUDIO_TOP_CON0, 0x100, 0x100, CHECK_MODE_EQUAL);
                        hal_audio_status_check_rg_value(AFE_APLL1_TUNER_CFG, 0, 0, CHECK_MODE_EQUAL);
                        log_hal_msgid_info("[Audio Clock] disable APLL1", 0);
                        hal_clock_disable(HAL_CLOCK_CG_AUD_INTF1);
                    } else {
                        log_hal_msgid_error("[Audio Clock] low jitter mode: i2s in enum error %d", 1,
                                            i);
                        assert(0);
                    }
                }
            }
        }
        // clear g_audio_clock_control flag
        hal_audio_status_clear_dsp_controlller_i2s_low_jitter(type, APLL_IN_LOW_JITTER);
    }
}

/**
 * @brief     This function is used to control the releated clock settings or power of APLL in Downlink by scenario.
 * @param[in] type is to indicate the audio scenario type.
 * @param[in] param is the pointer of open parameters.
 * @param[in] control enable or disable.
 * @return    NONE.
 */
void hal_audio_status_set_i2s_master_out_low_jitter_mode(audio_scenario_type_t type, mcu2dsp_open_param_t *param, bool control)
{
    if ((param == NULL) && (control)) {
        return;
    }
    if (control) {
        // check stream_out type, avoid uplink run this code!
        if (param->param.stream_out != STREAM_OUT_AFE) {
            return;
        }
        uint8_t intf0_used_flag = 0;
        uint8_t intf1_used_flag = 0;
        uint32_t device_out_rate = param->stream_out_param.afe.sampling_rate;
        uint32_t i2s_out_rate[4] = {0};
        i2s_out_rate[0] = param->stream_out_param.afe.i2s_master_sampling_rate[0];
        i2s_out_rate[1] = param->stream_out_param.afe.i2s_master_sampling_rate[1];
        i2s_out_rate[2] = param->stream_out_param.afe.i2s_master_sampling_rate[2];
        i2s_out_rate[3] = param->stream_out_param.afe.i2s_master_sampling_rate[3];
        hal_audio_device_t out_device = param->stream_out_param.afe.audio_device;
        hal_audio_interface_t device_interface = param->stream_out_param.afe.audio_interface;
        if (type == AUDIO_SCENARIO_TYPE_SPDIF) {
            // SPDIF USE I2S0 Master out
            out_device = HAL_AUDIO_DEVICE_I2S_MASTER;
            device_interface = HAL_AUDIO_INTERFACE_1;
        }
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
        hal_audio_device_t out_device1 = param->stream_out_param.afe.audio_device1;
        hal_audio_device_t out_device2 = param->stream_out_param.afe.audio_device2;
        hal_audio_device_t out_device3 = param->stream_out_param.afe.audio_device3;
        hal_audio_interface_t device_interface1 = param->stream_out_param.afe.audio_interface1;
        hal_audio_interface_t device_interface2 = param->stream_out_param.afe.audio_interface2;
        hal_audio_interface_t device_interface3 = param->stream_out_param.afe.audio_interface3;
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
        hal_audio_device_t out_device4 = param->stream_out_param.afe.audio_device4;
        hal_audio_device_t out_device5 = param->stream_out_param.afe.audio_device5;
        hal_audio_device_t out_device6 = param->stream_out_param.afe.audio_device6;
        hal_audio_device_t out_device7 = param->stream_out_param.afe.audio_device7;
        hal_audio_interface_t device_interface4 = param->stream_out_param.afe.audio_interface4;
        hal_audio_interface_t device_interface5 = param->stream_out_param.afe.audio_interface5;
        hal_audio_interface_t device_interface6 = param->stream_out_param.afe.audio_interface6;
        hal_audio_interface_t device_interface7 = param->stream_out_param.afe.audio_interface7;
#endif
#endif
        /* -1-  Get I2S Master Interface -------------------------------------------------------------------------------- */
        int32_t interface_number = -1; // -1 is used for error check
        hal_audio_interface_t interface[8] = {0};
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
        if ((device_interface7 != HAL_AUDIO_INTERFACE_NONE) &&
            ((out_device7 == HAL_AUDIO_DEVICE_I2S_MASTER)   ||
             (out_device7 == HAL_AUDIO_DEVICE_I2S_MASTER_L)  ||
             (out_device7 == HAL_AUDIO_DEVICE_I2S_MASTER_R))) {
            interface_number += 1;
            interface[interface_number] = device_interface7;
        }
        if ((device_interface6 != HAL_AUDIO_INTERFACE_NONE) &&
            ((out_device6 == HAL_AUDIO_DEVICE_I2S_MASTER)   ||
             (out_device6 == HAL_AUDIO_DEVICE_I2S_MASTER_L)  ||
             (out_device6 == HAL_AUDIO_DEVICE_I2S_MASTER_R))) {
            interface_number += 1;
            interface[interface_number] = device_interface6;
        }
        if ((device_interface5 != HAL_AUDIO_INTERFACE_NONE) &&
            ((out_device5 == HAL_AUDIO_DEVICE_I2S_MASTER)   ||
             (out_device5 == HAL_AUDIO_DEVICE_I2S_MASTER_L)  ||
             (out_device5 == HAL_AUDIO_DEVICE_I2S_MASTER_R))) {
            interface_number += 1;
            interface[interface_number] = device_interface5;
        }
        if ((device_interface4 != HAL_AUDIO_INTERFACE_NONE) &&
            ((out_device4 == HAL_AUDIO_DEVICE_I2S_MASTER)   ||
             (out_device4 == HAL_AUDIO_DEVICE_I2S_MASTER_L)  ||
             (out_device4 == HAL_AUDIO_DEVICE_I2S_MASTER_R))) {
            interface_number += 1;
            interface[interface_number] = device_interface4;
        }
#endif
        if ((device_interface3 != HAL_AUDIO_INTERFACE_NONE) &&
            ((out_device3 == HAL_AUDIO_DEVICE_I2S_MASTER)   ||
             (out_device3 == HAL_AUDIO_DEVICE_I2S_MASTER_L)  ||
             (out_device3 == HAL_AUDIO_DEVICE_I2S_MASTER_R))) {
            interface_number += 1;
            interface[interface_number] = device_interface3;
        }
        if ((device_interface2 != HAL_AUDIO_INTERFACE_NONE) &&
            ((out_device2 == HAL_AUDIO_DEVICE_I2S_MASTER)   ||
             (out_device2 == HAL_AUDIO_DEVICE_I2S_MASTER_L)  ||
             (out_device2 == HAL_AUDIO_DEVICE_I2S_MASTER_R))) {
            interface_number += 1;
            interface[interface_number] = device_interface2;
        }
        if ((device_interface1 != HAL_AUDIO_INTERFACE_NONE) &&
            ((out_device1 == HAL_AUDIO_DEVICE_I2S_MASTER)   ||
             (out_device1 == HAL_AUDIO_DEVICE_I2S_MASTER_L)  ||
             (out_device1 == HAL_AUDIO_DEVICE_I2S_MASTER_R))) {
            interface_number += 1;
            interface[interface_number] = device_interface1;
        }
#endif
        if ((device_interface != HAL_AUDIO_INTERFACE_NONE) &&
            ((out_device == HAL_AUDIO_DEVICE_I2S_MASTER)   ||
             (out_device == HAL_AUDIO_DEVICE_I2S_MASTER_L)  ||
             (out_device == HAL_AUDIO_DEVICE_I2S_MASTER_R))) {
            interface_number += 1;
            interface[interface_number] = device_interface;
        }
        if (interface_number == -1) {
            log_hal_msgid_info("[Audio Clock] low jitter mode: i2s_out device is not suitable, type %d device 0x%x", 2,
                               type,
                               out_device);
            return;
        }
        for (uint32_t cnt = 0; cnt <= interface_number; cnt ++) {
            uint8_t i2s_number = 0;
            switch (interface[cnt]) {
                case HAL_AUDIO_INTERFACE_1: // I2S MASTER 0
                    i2s_number = 0;
                    break;
                case HAL_AUDIO_INTERFACE_2: // I2S MASTER 1
                    i2s_number = 1;
                    break;
                case HAL_AUDIO_INTERFACE_3: // I2S MASTER 2
                    i2s_number = 2;
                    break;
                case HAL_AUDIO_INTERFACE_4: // I2S MASTER 3
                    i2s_number = 3;
                    break;
                default:
                    log_hal_msgid_error("[Audio Clock] ERROR: low jitter mode, i2s_out interface error. type %d interface %d", 2,
                                        type,
                                        interface[cnt]);
                    assert(0);
                    return;
            }
            uint32_t valid_rate = i2s_out_rate[i2s_number] == 0 ? device_out_rate : i2s_out_rate[i2s_number];
            bool is_low_jitter_current_i2s = false;
            // Now i2s master's apll clk source is already used!
            afe_apll_source_t apll_clk = afe_get_apll_by_samplerate(valid_rate);
            bool is_vp_exist = hal_audio_status_check_clock_gate_status(AUDIO_SCENARIO_TYPE_VP, AUDIO_CLOCK_INT);
            if ((!is_vp_exist) && (g_audio_clock_control.i2s_master[i2s_number].apll_clk > 0) && (g_audio_clock_control.i2s_master[i2s_number].apll_clk != apll_clk)) {
                log_hal_msgid_warning("[Audio Clock] WARNING: low jitter mode, i2s[%d]_out is already used by apll%d scenario type[%d] fs %d", 4,
                                        i2s_number,
                                        g_audio_clock_control.i2s_master[i2s_number].apll_clk,
                                        type,
                                        valid_rate
                                        );
                if (valid_rate > 48000) {
                    log_hal_msgid_error("[Audio Clock] ERROR: low jitter mode error", 0);
                    //assert(0);
                    return;
                } else {
                    return;
                }
            }
            if (g_audio_clock_control.i2s_master[i2s_number].apll_clk != AFE_APLL_NONE) {
                is_low_jitter_current_i2s = true;
            }
            if ((is_low_jitter_current_i2s) || (valid_rate > 48000)) {
                param->stream_out_param.afe.is_low_jitter[i2s_number] = true;
            }
            if (!param->stream_out_param.afe.is_low_jitter[i2s_number]) {
                continue;
            }
            log_hal_msgid_info("[Audio Clock] low jitter mode: i2s_out type %d interface %d i2s_%d rate %d", 4,
                               type,
                               interface[cnt],
                               i2s_number,
                               valid_rate);
            /* Choose APLL 1/2 */
            audio_clock_setting_type_t cg_type = (apll_clk == AFE_APLL1) ? AUDIO_CLOCK_INTF1_OUT : AUDIO_CLOCK_INTF0_OUT;
            g_audio_clock_control.i2s_master[i2s_number].out_low_jitter[type / 32] |= 1 << (type % 32); // log the i2s setting of each scenario
            if (g_audio_clock_control.i2s_master[i2s_number].apll_clk != apll_clk) {
                log_hal_msgid_warning("[Audio Clock] i2s_%d apll clk will be overwrite by %d != %d", 3, i2s_number, apll_clk,
                    g_audio_clock_control.i2s_master[i2s_number].apll_clk);
            }
            g_audio_clock_control.i2s_master[i2s_number].apll_clk = apll_clk; // overwrite it
            if (cg_type == AUDIO_CLOCK_INTF1_OUT) {
                intf1_used_flag = 1 << i2s_number;
            } else {
                intf0_used_flag = 1 << i2s_number;
            }
        }
        if (intf0_used_flag) {
            if ((!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_INTF0_OUT)) &&
                (!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_INTF0_OUT + 2))) {
                clock_mux_sel(CLK_AUD_INTERFACE0_SEL, 3); // 3:APLL2_CK, 24.576 / 22.579 MHz
                log_hal_msgid_info("[Audio Clock] enable APLL2", 0);
                hal_clock_enable(HAL_CLOCK_CG_AUD_INTF0);
            }
            hal_audio_status_set_clock_gate(type, AUDIO_CLOCK_INTF0_OUT, control);
        }
        if (intf1_used_flag) {
            if ((!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_INTF1_OUT)) &&
                (!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_INTF1_OUT + 2))) {
                clock_mux_sel(CLK_AUD_INTERFACE1_SEL, 2);
                log_hal_msgid_info("[Audio Clock] enable APLL1", 0);
                hal_clock_enable(HAL_CLOCK_CG_AUD_INTF1);
            }
            hal_audio_status_set_clock_gate(type, AUDIO_CLOCK_INTF1_OUT, control);
        }
    } else {
        uint32_t count = hal_audio_status_get_clock_gate_setting(type);
        for (uint32_t i = 0; i < (AUDIO_CLOCK_INTF1_OUT - AUDIO_CLOCK_INTF0_OUT + 1); i ++) {
            if (count & (1UL << (AUDIO_CLOCK_INTF0_OUT + i))) {
                hal_audio_status_set_clock_gate(type, AUDIO_CLOCK_INTF0_OUT + i, control);
                if ((!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_INTF0_OUT + i)) &&
                    (!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_INTF0_IN + i))) {
                    if (i == 0) {
                        // Check APLL2 mode, AUDIO_TOP_CON0[9] = 1 && AFE_APLL2_TUNER_CFG[0] = 0
                        hal_audio_status_check_rg_value(AUDIO_TOP_CON0, 0x200, 0x200, CHECK_MODE_EQUAL);
                        hal_audio_status_check_rg_value(AFE_APLL2_TUNER_CFG, 0, 0, CHECK_MODE_EQUAL);
                        log_hal_msgid_info("[Audio Clock] disable APLL2", 0);
                        hal_clock_disable(HAL_CLOCK_CG_AUD_INTF0);
                    } else if (i == 1) {
                        // Check APLL1 mode, AUDIO_TOP_CON0[8] = 1 && AFE_APLL1_TUNER_CFG[0] = 0
                        hal_audio_status_check_rg_value(AUDIO_TOP_CON0, 0x100, 0x100, CHECK_MODE_EQUAL);
                        hal_audio_status_check_rg_value(AFE_APLL1_TUNER_CFG, 0, 0, CHECK_MODE_EQUAL);
                        log_hal_msgid_info("[Audio Clock] disable APLL1", 0);
                        hal_clock_disable(HAL_CLOCK_CG_AUD_INTF1);
                    } else {
                        log_hal_msgid_error("[Audio Clock] low jitter mode: i2s out enum error %d", 1,
                                            i);
                        assert(0);
                    }
                }
            }
        }
        // clear g_audio_clock_control flag
        hal_audio_status_clear_dsp_controlller_i2s_low_jitter(type, APLL_OUT_LOW_JITTER);
    }
}

/**
 * @brief     This function is used to control the releated clock settings or power of APLL in MCLK by scenario.
 * @param[in] type is to indicate the audio scenario type.
 * @param[in] param is the pointer of open parameters.
 * @param[in] control enable or disable.
 * @return    NONE.
 */
void hal_audio_status_set_i2s_mclk_low_jitter_mode(audio_scenario_type_t type, mcu2dsp_open_param_t *param, bool control)
{
    if ((param == NULL) && (control)) {
        return;
    }
    if (type == AUDIO_SCENARIO_TYPE_MCLK) {
        if (control) {
            uint32_t device_out_rate = param->stream_out_param.afe.sampling_rate;
            afe_apll_source_t apll_clk = afe_get_apll_by_samplerate(device_out_rate);
            audio_clock_setting_type_t cg_type = (apll_clk == AFE_APLL1) ? AUDIO_CLOCK_INTF1_OUT : AUDIO_CLOCK_INTF0_OUT;
            bool intf0_used_flag = false;
            bool intf1_used_flag = false;
            if (cg_type == AUDIO_CLOCK_INTF1_OUT) {
                intf1_used_flag = true;
            } else {
                intf0_used_flag = true;
            }
        if (intf0_used_flag) {
            if ((!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_INTF0_OUT)) &&
                (!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_INTF0_OUT + 2))) {
                clock_mux_sel(CLK_AUD_INTERFACE0_SEL, 3); // 3:APLL2_CK, 24.576 / 22.579 MHz
                log_hal_msgid_info("[Audio Clock] enable APLL2", 0);
                hal_clock_enable(HAL_CLOCK_CG_AUD_INTF0);
            }
            hal_audio_status_set_clock_gate(type, AUDIO_CLOCK_INTF0_OUT, control);
        }
        if (intf1_used_flag) {
            if ((!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_INTF1_OUT)) &&
                (!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_INTF1_OUT + 2))) {
                clock_mux_sel(CLK_AUD_INTERFACE1_SEL, 2);
                log_hal_msgid_info("[Audio Clock] enable APLL1", 0);
                hal_clock_enable(HAL_CLOCK_CG_AUD_INTF1);
            }
            hal_audio_status_set_clock_gate(type, AUDIO_CLOCK_INTF1_OUT, control);
        }
        } else {
            uint32_t count = hal_audio_status_get_clock_gate_setting(type);
            for (uint32_t i = 0; i < (AUDIO_CLOCK_INTF1_OUT - AUDIO_CLOCK_INTF0_OUT + 1); i ++) {
                if (count & (1UL << (AUDIO_CLOCK_INTF0_OUT + i))) {
                    hal_audio_status_set_clock_gate(type, AUDIO_CLOCK_INTF0_OUT + i, control);
                    if ((!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_INTF0_OUT + i)) &&
                        (!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_INTF0_IN + i))) {
                        if (i == 0) {
                            // // Check APLL2 mode, AUDIO_TOP_CON0[9] = 1 && AFE_APLL2_TUNER_CFG[0] = 0
                            // hal_audio_status_check_rg_value(AUDIO_TOP_CON0, 0x200, 0x200, CHECK_MODE_EQUAL);
                            // hal_audio_status_check_rg_value(AFE_APLL2_TUNER_CFG, 0, 0, CHECK_MODE_EQUAL);
                            log_hal_msgid_info("[Audio Clock] disable APLL2", 0);
                            hal_clock_disable(HAL_CLOCK_CG_AUD_INTF0);
                        } else if (i == 1) {
                            // // Check APLL1 mode, AUDIO_TOP_CON0[8] = 1 && AFE_APLL1_TUNER_CFG[0] = 0
                            // hal_audio_status_check_rg_value(AUDIO_TOP_CON0, 0x100, 0x100, CHECK_MODE_EQUAL);
                            // hal_audio_status_check_rg_value(AFE_APLL1_TUNER_CFG, 0, 0, CHECK_MODE_EQUAL);
                            log_hal_msgid_info("[Audio Clock] disable APLL1", 0);
                            hal_clock_disable(HAL_CLOCK_CG_AUD_INTF1);
                        } else {
                            log_hal_msgid_error("[Audio Clock] low jitter mode: i2s out enum error %d", 1,
                                                    i);
                            assert(0);
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief     This function is used to lock some clock or power for AMP, to implement delay-off timer mechanism.
 * @param[in] type is to indicate the audio scenario type.
 * @param[in] param is the pointer of open parameters.
 * @param[in] control enable or disable.
 * @return    NONE.
 */
void hal_audio_status_set_amp_lock(audio_scenario_type_t type, mcu2dsp_open_param_t *param, bool control)
{
    if (((param == NULL) && (control)) || (!control)) {
        return;
    }
    // check stream_out type
    if (param->param.stream_out != STREAM_OUT_AFE) {
        return;
    }
    /* AMP control should align with DSP side. */
    /* We can add DAC and I2S out into amp control mechanism, default: DAC */
    hal_audio_device_t out_device = param->stream_out_param.afe.audio_device;
    uint32_t i = 0;
    hal_audio_device_t tmp_device = HAL_AUDIO_DEVICE_NONE;
    for (i = 0; i < AUDIO_AMP_DELAY_OFF_DEVICE_NUMBER; i ++) {
        tmp_device |= amp_lock_device[i];
    }
    if (out_device & tmp_device) {
        audio_clock_setting_type_t cg_amp;       // DAC -> AUDIO_POWER_DAC; I2S -> AUDIO_POWER_I2S
        audio_clock_setting_type_t cg_amp_check[2] = {AUDIO_POWER_END, AUDIO_POWER_END}; // DAC -> HI-RES; I2S -> LOW-JITTER
        if (out_device & HAL_AUDIO_DEVICE_DAC_DUAL) {
            cg_amp = AUDIO_POWER_DAC;
            cg_amp_check[0] = AUDIO_CLOCK_DWLINK;
            cg_amp_check[1] = AUDIO_POWER_END; // ignore
        } else if (out_device & HAL_AUDIO_DEVICE_I2S_MASTER) {
            /* I2S MST : should lock low-jitter mode */
            cg_amp = AUDIO_POWER_I2S;
            cg_amp_check[0] = AUDIO_CLOCK_INTF0_OUT;
            cg_amp_check[1] = AUDIO_CLOCK_INTF1_OUT;
        } else {
            (void)cg_amp;
            (void)cg_amp_check;
            log_hal_msgid_error("[Audio Clock][AMP] please implement the code!", 0);
            assert(0);
            return;
        }
        if (!hal_audio_status_check_clock_gate_status(AUDIO_SCENARIO_TYPE_AMP, cg_amp)) {
            hal_audio_status_set_clock_gate(AUDIO_SCENARIO_TYPE_AMP, cg_amp, control);
            log_hal_msgid_info("[Audio Clock][AMP] lock clock [%d]!", 1, cg_amp);
        }

        /* AMP lock for SPM state1 to avoid pop noise of ending */
        if ((hal_audio_status_get_clock_gate_status(AUDIO_DSP_SPM_STATE1)) &&
            (!hal_audio_status_check_clock_gate_status(AUDIO_SCENARIO_TYPE_AMP, AUDIO_DSP_SPM_STATE1))) { // DAC is already on
            hal_audio_status_set_clock_gate(AUDIO_SCENARIO_TYPE_AMP, AUDIO_DSP_SPM_STATE1, control);
            log_hal_msgid_info("[Audio Clock][AMP] lock SPM state1", 0);
        }

        /* AMP lock special clock */
        /* DAC     : should lock hi-res */
        /* I2S MST : should lock low-jitter mode */
        for (i = 0; i < (sizeof(cg_amp_check)/sizeof(audio_clock_setting_type_t)); i ++) {
            if (cg_amp_check[i] == AUDIO_POWER_END) {
                continue;
            }
            if ((hal_audio_status_get_clock_gate_status(cg_amp_check[i])) &&
                (!hal_audio_status_check_clock_gate_status(AUDIO_SCENARIO_TYPE_AMP, cg_amp_check[i]))) {
                hal_audio_status_set_clock_gate(AUDIO_SCENARIO_TYPE_AMP, cg_amp_check[i], control);
                log_hal_msgid_info("[Audio Clock][AMP] lock special clock [%d]!", 1, cg_amp_check[i]);
            }
        }

        /* AMP lock audio common clock */
        /* We should power off audio clock after closing DAC when DAC is alredy on! */
        // AMP is for AMP delay, we can only power off DAC after DSP notifying!
        if ((!hal_audio_status_check_clock_gate_status(AUDIO_SCENARIO_TYPE_AMP, AUDIO_CLOCK_GPSRC)) &&
            (!hal_audio_status_check_clock_gate_status(AUDIO_SCENARIO_TYPE_AMP, AUDIO_CLOCK_INT)) &&
            (!hal_audio_status_check_clock_gate_status(AUDIO_SCENARIO_TYPE_AMP, AUDIO_CLOCK_ENGINE))) {
            hal_audio_status_set_clock_gate(AUDIO_SCENARIO_TYPE_AMP, AUDIO_CLOCK_GPSRC, control);
            hal_audio_status_set_clock_gate(AUDIO_SCENARIO_TYPE_AMP, AUDIO_CLOCK_INT, control);
            hal_audio_status_set_clock_gate(AUDIO_SCENARIO_TYPE_AMP, AUDIO_CLOCK_ENGINE, control);
            log_hal_msgid_info("[Audio Clock][AMP] lock common audio clock", 0);
        }
        return;
    }
}

/**
 * @brief     This function is used to control the releated clock settings or power of audio by scenario(VOW/SPDIF .etc).
 * @param[in] type is to indicate the audio scenario type.
 * @param[in] param is the pointer of open parameters.
 * @param[in] control enable or disable.
 * @return    NONE.
 */
void hal_audio_status_set_audio_clock_gate(audio_scenario_type_t type, mcu2dsp_open_param_t *param, bool control)
{
    /* -1-  Hi-res check  ------------------------------------------------------------------------------------------- */
    hal_audio_status_set_dl_hires(type, param, control);
    hal_audio_status_set_ul_hires(type, param, control);
    /* -2-  Low jitter mode check  ---------------------------------------------------------------------------------- */
    hal_audio_status_set_i2s_master_in_low_jitter_mode(type, param, control);
    hal_audio_status_set_i2s_master_out_low_jitter_mode(type, param, control);
    hal_audio_status_set_i2s_mclk_low_jitter_mode(type, param, control);
    /* -3-  Check Common Clock Gate --------------------------------------------------------------------------------- */
    if (control) {
        /* NOTE: Special Clock Gate Setting: SPDIF & VOW !!! */
        bool spdif_flag = false;
        if (param) {
            hal_audio_device_t out_device = param->stream_out_param.afe.audio_device;
            if ((param->param.stream_out == STREAM_OUT_AFE) && (out_device == HAL_AUDIO_DEVICE_SPDIF)) {
                spdif_flag = true; // USE SPDIF
            }
        }

        if ((type == AUDIO_SCENARIO_TYPE_SPDIF) || (spdif_flag)) {
            hal_clock_enable(HAL_CLOCK_CG_SPDIF);
            hal_audio_status_set_clock_gate(type, AUDIO_CLOCK_SPDIF, control);
            log_hal_msgid_info("[Audio Clock] enable spdif", 0);
        }
        if (type == AUDIO_SCENARIO_TYPE_VOW) {
            hal_clock_enable(HAL_CLOCK_CG_AUD_VOW_BUS);
            hal_audio_status_set_clock_gate(type, AUDIO_CLOCK_VOW, control);
            log_hal_msgid_info("[Audio Clock] enable vow", 0);
        }
        /* dsp low power control */
#ifdef HAL_AUDIO_POWER_SLIM_ENABLED
        if ((type == AUDIO_SCENARIO_TYPE_ANC)
#ifdef HAL_PURE_LINEIN_PLAYBACK_ENABLE
            || (type == AUDIO_SCENARIO_TYPE_LINE_IN)
#endif /* HAL_PURE_LINEIN_PLAYBACK_ENABLE */
        ) {
            if (!hal_audio_status_get_clock_gate_status(AUDIO_DSP_SPM_STATE4)) {
                log_hal_msgid_info("[Audio Clock] lock SPM state4", 0);
                spm_audio_lowpower_setting(SPM_STATE4, SPM_ENABLE); // DSP can sleep
            }
            hal_audio_status_set_clock_gate(type, AUDIO_DSP_SPM_STATE4, control);
        } else
#endif /* HAL_AUDIO_POWER_SLIM_ENABLED */
        {
#ifdef AIR_KEEP_I2S_ENABLE
            if ((type == AUDIO_SCENARIO_TYPE_DEVICE)) {
                if (!hal_audio_status_get_clock_gate_status(AUDIO_DSP_SPM_STATE4)) {
                    log_hal_msgid_info("[Audio Clock] lock SPM state4", 0);
                    spm_audio_lowpower_setting(SPM_STATE4, SPM_ENABLE); // DSP can sleep
                }
                hal_audio_status_set_clock_gate(type, AUDIO_DSP_SPM_STATE4, control);
            } else
#endif
            {
                if (!hal_audio_status_get_clock_gate_status(AUDIO_DSP_SPM_STATE1)) {
                    log_hal_msgid_info("[Audio Clock] lock SPM state1", 0);
                    spm_audio_lowpower_setting(SPM_STATE1, SPM_ENABLE); // DSP can't sleep
                }
                hal_audio_status_set_clock_gate(type, AUDIO_DSP_SPM_STATE1, control);
            }
        }

        if ((!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_GPSRC)) &&
            (!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_INT)) &&
            (!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_ENGINE))) { // Power on
            /* NOTE: We must enable INTBUS/ENGINE CG before mtcmos control!!! */
            hal_clock_enable(HAL_CLOCK_CG_AUD_INTBUS);
            hal_clock_enable(HAL_CLOCK_CG_AUD_GPSRC);
            hal_clock_enable(HAL_CLOCK_CG_AUD_ENGINE_BUS);
            // power on mt mos
#ifdef HAL_SLEEP_MANAGER_ENABLED
            spm_control_mtcmos(SPM_MTCMOS_AUDIO_SYS, SPM_MTCMOS_PWR_ENABLE);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO_ANC, SPM_MTCMOS_PWR_ENABLE);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO_HS,  SPM_MTCMOS_PWR_ENABLE);
#endif /* HAL_SLEEP_MANAGER_ENABLED */
            // // PDN Control
            *((volatile uint32_t*)(0xC0000000)) = 0x3FCCC344; // pdn AUDIO_TOP_CON0
            *((volatile uint32_t*)(0xC0000004)) = 0x709b000f; // PDN AUDIO_TOP_CON1
        }
        hal_audio_status_set_clock_gate(type, AUDIO_CLOCK_GPSRC, control);
        hal_audio_status_set_clock_gate(type, AUDIO_CLOCK_INT, control);
        hal_audio_status_set_clock_gate(type, AUDIO_CLOCK_ENGINE, control);
    } else {
        /* NOTE: Special Clock Gate Setting: SPDIF & VOW !!! */
        uint32_t count = hal_audio_status_get_clock_gate_setting(type);
        if ((type == AUDIO_SCENARIO_TYPE_SPDIF) && (count & (1 << AUDIO_CLOCK_SPDIF))) {
            // Check SPDIF, AFE_SPDIFIN_CFG0[0] = 0
            hal_audio_status_check_rg_value(AFE_SPDIFIN_CFG0, 0, 0x1, CHECK_MODE_EQUAL);

            hal_clock_disable(HAL_CLOCK_CG_SPDIF);
            hal_audio_status_set_clock_gate(type, AUDIO_CLOCK_SPDIF, control);
        }
        if ((type == AUDIO_SCENARIO_TYPE_VOW) && (count & (1 << AUDIO_CLOCK_VOW))) {
            // Check VOW, AFE_VOW_TOP_CON1[0] = 0 && AFE_VOW_TOP_CON2[0] = 0
            hal_audio_status_check_rg_value(AFE_VOW_TOP_CON1, 0, 0x1, CHECK_MODE_EQUAL);
            hal_audio_status_check_rg_value(AFE_VOW_TOP_CON2, 0, 0x1, CHECK_MODE_EQUAL);

            hal_clock_disable(HAL_CLOCK_CG_AUD_VOW_BUS);
            hal_audio_status_set_clock_gate(type, AUDIO_CLOCK_VOW, control);
        }

        hal_audio_status_set_clock_gate(type, AUDIO_CLOCK_GPSRC, control);
        hal_audio_status_set_clock_gate(type, AUDIO_CLOCK_INT, control);
        hal_audio_status_set_clock_gate(type, AUDIO_CLOCK_ENGINE, control);
        if ((!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_GPSRC)) &&
            (!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_INT)) &&
            (!hal_audio_status_get_clock_gate_status(AUDIO_CLOCK_ENGINE))) { // Power off
            // Check AFE Power, AUDIO_TOP_CON0[2] = 1
            hal_audio_status_check_rg_value(AUDIO_TOP_CON0, 0x4, 0x4, CHECK_MODE_EQUAL);
#ifdef HAL_SLEEP_MANAGER_ENABLED
            spm_control_mtcmos(SPM_MTCMOS_AUDIO_SYS, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO_ANC, SPM_MTCMOS_PWR_DISABLE);
            spm_control_mtcmos(SPM_MTCMOS_AUDIO_HS,  SPM_MTCMOS_PWR_DISABLE);
#endif /* HAL_SLEEP_MANAGER_ENABLED */
            /* NOTE: We must disable INTBUS/ENGINE CG after mtcmos control!!! */
            hal_clock_disable(HAL_CLOCK_CG_AUD_INTBUS);
            hal_clock_disable(HAL_CLOCK_CG_AUD_GPSRC);
            hal_clock_disable(HAL_CLOCK_CG_AUD_ENGINE_BUS);
        }
        /* dsp low power control */
        if (count & (1 << AUDIO_DSP_SPM_STATE1)) {
            hal_audio_status_set_clock_gate(type, AUDIO_DSP_SPM_STATE1, control);
            if (!hal_audio_status_get_clock_gate_status(AUDIO_DSP_SPM_STATE1)) {
                log_hal_msgid_info("[Audio Clock] unlock SPM state1", 0);
                spm_audio_lowpower_setting(SPM_STATE1, SPM_DISABLE);
            }
        }
        if (count & (1 << AUDIO_DSP_SPM_STATE4)) {
            hal_audio_status_set_clock_gate(type, AUDIO_DSP_SPM_STATE4, control);
            if (!hal_audio_status_get_clock_gate_status(AUDIO_DSP_SPM_STATE4)) {
                log_hal_msgid_info("[Audio Clock] unlock SPM state4", 0);
                spm_audio_lowpower_setting(SPM_STATE4, SPM_DISABLE);
            }
        }

        /* special power control */
        if (count & (1 << AUDIO_POWER_I2S)) {
            hal_audio_status_set_clock_gate(type, AUDIO_POWER_I2S, false);
            if (!hal_audio_status_get_clock_gate_status(AUDIO_POWER_I2S)) { // Power down I2S
                /* pass-through */
            }
        }
    }
    /* -4-  Check power of amp lock    ------------------------------------------------------------------------------ */
    hal_audio_status_set_amp_lock(type, param, control);
    /* -5-  Show detail info  --------------------------------------------------------------------------------------- */
    log_hal_msgid_info("[Audio Clock] type [%d] cg_setting [0x%x] control [0x%x]", 3,
                       type,
                       hal_audio_status_get_clock_gate_setting(type),
                       control);
    log_hal_msgid_info("[Audio Clock] DAC [0x%x][0x%x][0x%x][0x%x] INT [0x%x][0x%x][0x%x][0x%x] ENGINE [0x%x][0x%x][0x%x][0x%x] GPSRC [0x%x][0x%x][0x%x][0x%x]", 16,
                       g_dsp_controller_cg[AUDIO_POWER_DAC][3], g_dsp_controller_cg[AUDIO_POWER_DAC][2], g_dsp_controller_cg[AUDIO_POWER_DAC][1], g_dsp_controller_cg[AUDIO_POWER_DAC][0],
                       g_dsp_controller_cg[AUDIO_CLOCK_INT][3], g_dsp_controller_cg[AUDIO_CLOCK_INT][2], g_dsp_controller_cg[AUDIO_CLOCK_INT][1], g_dsp_controller_cg[AUDIO_CLOCK_INT][0],
                       g_dsp_controller_cg[AUDIO_CLOCK_ENGINE][3], g_dsp_controller_cg[AUDIO_CLOCK_ENGINE][2], g_dsp_controller_cg[AUDIO_CLOCK_ENGINE][1], g_dsp_controller_cg[AUDIO_CLOCK_ENGINE][0],
                       g_dsp_controller_cg[AUDIO_CLOCK_GPSRC][3], g_dsp_controller_cg[AUDIO_CLOCK_GPSRC][2], g_dsp_controller_cg[AUDIO_CLOCK_GPSRC][1], g_dsp_controller_cg[AUDIO_CLOCK_GPSRC][0]
                      );
    log_hal_msgid_info("[Audio Clock] UL  [0x%x][0x%x][0x%x][0x%x] DL  [0x%x][0x%x][0x%x][0x%x] SPDIF  [0x%x][0x%x][0x%x][0x%x] TEST  [0x%x][0x%x][0x%x][0x%x]", 16,
                       g_dsp_controller_cg[AUDIO_CLOCK_UPLINK][3], g_dsp_controller_cg[AUDIO_CLOCK_UPLINK][2], g_dsp_controller_cg[AUDIO_CLOCK_UPLINK][1], g_dsp_controller_cg[AUDIO_CLOCK_UPLINK][0],
                       g_dsp_controller_cg[AUDIO_CLOCK_DWLINK][3], g_dsp_controller_cg[AUDIO_CLOCK_DWLINK][2], g_dsp_controller_cg[AUDIO_CLOCK_DWLINK][1], g_dsp_controller_cg[AUDIO_CLOCK_DWLINK][0],
                       g_dsp_controller_cg[AUDIO_CLOCK_SPDIF][3], g_dsp_controller_cg[AUDIO_CLOCK_SPDIF][2], g_dsp_controller_cg[AUDIO_CLOCK_SPDIF][1], g_dsp_controller_cg[AUDIO_CLOCK_SPDIF][0],
                       g_dsp_controller_cg[AUDIO_CLOCK_TEST][3], g_dsp_controller_cg[AUDIO_CLOCK_TEST][2], g_dsp_controller_cg[AUDIO_CLOCK_TEST][1], g_dsp_controller_cg[AUDIO_CLOCK_TEST][0]
                      );
    log_hal_msgid_info("[Audio Clock] APLL2 IN [0x%x][0x%x][0x%x][0x%x] APLL1 IN [0x%x][0x%x][0x%x][0x%x] APLL2 OUT [0x%x][0x%x][0x%x][0x%x] APLL1 OUT [0x%x][0x%x][0x%x][0x%x]", 16,
                       g_dsp_controller_cg[AUDIO_CLOCK_INTF0_IN][3], g_dsp_controller_cg[AUDIO_CLOCK_INTF0_IN][2], g_dsp_controller_cg[AUDIO_CLOCK_INTF0_IN][1], g_dsp_controller_cg[AUDIO_CLOCK_INTF0_IN][0],
                       g_dsp_controller_cg[AUDIO_CLOCK_INTF1_IN][3], g_dsp_controller_cg[AUDIO_CLOCK_INTF1_IN][2], g_dsp_controller_cg[AUDIO_CLOCK_INTF1_IN][1], g_dsp_controller_cg[AUDIO_CLOCK_INTF1_IN][0],
                       g_dsp_controller_cg[AUDIO_CLOCK_INTF0_OUT][3], g_dsp_controller_cg[AUDIO_CLOCK_INTF0_OUT][2], g_dsp_controller_cg[AUDIO_CLOCK_INTF0_OUT][1], g_dsp_controller_cg[AUDIO_CLOCK_INTF0_OUT][0],
                       g_dsp_controller_cg[AUDIO_CLOCK_INTF1_OUT][3], g_dsp_controller_cg[AUDIO_CLOCK_INTF1_OUT][2], g_dsp_controller_cg[AUDIO_CLOCK_INTF1_OUT][1], g_dsp_controller_cg[AUDIO_CLOCK_INTF1_OUT][0]
                      );
    log_hal_msgid_info("[Audio Clock] Bias0_HP [0x%x][0x%x][0x%x][0x%x] Bias1_HP [0x%x][0x%x][0x%x][0x%x] Bias2_HP [0x%x][0x%x][0x%x][0x%x] Bias0_NM [0x%x][0x%x][0x%x][0x%x]", 16,
                       g_dsp_controller_cg[AUDIO_POWER_MICBIAS_0_HP][3], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_0_HP][2], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_0_HP][1], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_0_HP][0],
                       g_dsp_controller_cg[AUDIO_POWER_MICBIAS_1_HP][3], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_1_HP][2], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_1_HP][1], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_1_HP][0],
                       g_dsp_controller_cg[AUDIO_POWER_MICBIAS_2_HP][3], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_2_HP][2], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_2_HP][1], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_2_HP][0],
                       g_dsp_controller_cg[AUDIO_POWER_MICBIAS_0_NM][3], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_0_NM][2], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_0_NM][1], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_0_NM][0]
                      );
    log_hal_msgid_info("[Audio Clock] Bias1_NM [0x%x][0x%x][0x%x][0x%x] Bias2_NM [0x%x][0x%x][0x%x][0x%x] Bias0_LP [0x%x][0x%x][0x%x][0x%x] Bias1_LP [0x%x][0x%x][0x%x][0x%x]", 16,
                       g_dsp_controller_cg[AUDIO_POWER_MICBIAS_1_NM][3], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_1_NM][2], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_1_NM][1], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_1_NM][0],
                       g_dsp_controller_cg[AUDIO_POWER_MICBIAS_2_NM][3], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_2_NM][2], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_2_NM][1], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_2_NM][0],
                       g_dsp_controller_cg[AUDIO_POWER_MICBIAS_0_LP][3], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_0_LP][2], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_0_LP][1], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_0_LP][0],
                       g_dsp_controller_cg[AUDIO_POWER_MICBIAS_1_LP][3], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_1_LP][2], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_1_LP][1], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_1_LP][0]
                      );
    log_hal_msgid_info("[Audio Clock] Bias2_LP [0x%x][0x%x][0x%x][0x%x] VOW [0x%x][0x%x][0x%x][0x%x]", 8,
                       g_dsp_controller_cg[AUDIO_POWER_MICBIAS_2_LP][3], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_2_LP][2], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_2_LP][1], g_dsp_controller_cg[AUDIO_POWER_MICBIAS_2_LP][0],
                       g_dsp_controller_cg[AUDIO_CLOCK_VOW][3], g_dsp_controller_cg[AUDIO_CLOCK_VOW][2], g_dsp_controller_cg[AUDIO_CLOCK_VOW][1], g_dsp_controller_cg[AUDIO_CLOCK_VOW][0]
                      );
    log_hal_msgid_info("[Audio Clock] SPM State1 [0x%x][0x%x][0x%x][0x%x] State3 [0x%x][0x%x][0x%x][0x%x] State4 [0x%x][0x%x][0x%x][0x%x]", 16,
                       g_dsp_controller_cg[AUDIO_DSP_SPM_STATE1][3], g_dsp_controller_cg[AUDIO_DSP_SPM_STATE1][2], g_dsp_controller_cg[AUDIO_DSP_SPM_STATE1][1], g_dsp_controller_cg[AUDIO_DSP_SPM_STATE1][0],
                       g_dsp_controller_cg[AUDIO_DSP_SPM_STATE3][3], g_dsp_controller_cg[AUDIO_DSP_SPM_STATE3][2], g_dsp_controller_cg[AUDIO_DSP_SPM_STATE3][1], g_dsp_controller_cg[AUDIO_DSP_SPM_STATE3][0],
                       g_dsp_controller_cg[AUDIO_DSP_SPM_STATE4][3], g_dsp_controller_cg[AUDIO_DSP_SPM_STATE4][2], g_dsp_controller_cg[AUDIO_DSP_SPM_STATE4][1], g_dsp_controller_cg[AUDIO_DSP_SPM_STATE4][0],
                       g_dsp_controller_cg[AUDIO_POWER_I2S][3], g_dsp_controller_cg[AUDIO_POWER_I2S][2], g_dsp_controller_cg[AUDIO_POWER_I2S][1], g_dsp_controller_cg[AUDIO_POWER_I2S][0]
                      );
}

void hal_audio_status_get_amp_delay_off_timer_get_gpt_count(audio_scenario_type_t type, bool control)
{
    bool cg_cmp_used_exclude_amp_flag = false; // cg_cmp is used by other scenario not AMP.
    if (control) {
        return;
    }

    /* amp timer off gpt count check */
    /* The time delay between the last closing scenario and the amp off should be bigger than the threhold. */
    if (type == AUDIO_SCENARIO_TYPE_AMP) {
        /* MCU close AMP, only AMP is on now */
        cg_cmp_used_exclude_amp_flag = g_dsp_controller_cg[AUDIO_CLOCK_INT][3] ||
                                        g_dsp_controller_cg[AUDIO_CLOCK_INT][2] ||
                                        g_dsp_controller_cg[AUDIO_CLOCK_INT][1] ||
                                        (g_dsp_controller_cg[AUDIO_CLOCK_INT][0] & 0xFFFFFFFD);
        if (!cg_cmp_used_exclude_amp_flag) {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &g_audio_clock_control.amp_delay_off_timer_stop_gpt_cnt);
            log_hal_msgid_info("[Audio Clock] AMP delay off stop time %u", 1, g_audio_clock_control.amp_delay_off_timer_stop_gpt_cnt);
        }
    } else {
        /* MCU close the last audio scenario, only AMP and current scenario are on now */
        uint32_t index1    = type / 32;
        uint32_t bit_mask1 = type % 32;
        uint32_t index2    = AUDIO_SCENARIO_TYPE_AMP / 32;
        uint32_t bit_mask2 = AUDIO_SCENARIO_TYPE_AMP % 32;
        for (uint32_t i = 0; i < 4; i ++) {
            if ((i == index1) && (i == index2)) {
                cg_cmp_used_exclude_amp_flag = cg_cmp_used_exclude_amp_flag || (g_dsp_controller_cg[AUDIO_CLOCK_INT][i] & (~(1<<bit_mask1)) & (~(1<<bit_mask2)));
            } else if (i == index1) {
                cg_cmp_used_exclude_amp_flag = cg_cmp_used_exclude_amp_flag || (g_dsp_controller_cg[AUDIO_CLOCK_INT][i] & (~(1<<bit_mask1)));
            } else if (i == index2) {
                cg_cmp_used_exclude_amp_flag = cg_cmp_used_exclude_amp_flag || (g_dsp_controller_cg[AUDIO_CLOCK_INT][i] & (~(1<<bit_mask2)));
            } else {
                cg_cmp_used_exclude_amp_flag = cg_cmp_used_exclude_amp_flag || g_dsp_controller_cg[AUDIO_CLOCK_INT][i];
            }
        }
        if (!cg_cmp_used_exclude_amp_flag) {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &g_audio_clock_control.amp_delay_off_timer_start_gpt_cnt);
            log_hal_msgid_info("[Audio Clock] AMP delay off start time %u", 1, g_audio_clock_control.amp_delay_off_timer_start_gpt_cnt);
        }
    }
}

/**
 * @brief     This function is used to do special handle for AMP, to avoid timing issue of AMP.
 * @param[in] type is to indicate the audio scenario type.
 * @param[in] param is the pointer of open parameters.
 * @param[in] control enable or disable.
 * @return    bool.
              true : need do nothing.
              false: need to ignore this running flag setting.
 */
bool hal_audio_status_check_amp_lock(audio_scenario_type_t type, mcu2dsp_open_param_t *param, bool control)
{
    uint32_t i = 0;
    audio_clock_setting_type_t check_cg = AUDIO_POWER_END;
    bool cg_cmp_used_exclude_amp_flag = false; // cg_cmp is used by other scenario not AMP.
    uint32_t amp_delay_off_timer_cnt = 0;
    if (control) {
        if (type == AUDIO_SCENARIO_TYPE_DC_COMPENSATION) {
            g_audio_clock_control.first_boot_flag = true; // ignore amp off time check
        } else {
            g_audio_clock_control.first_boot_flag = false;
        }
        return true;
    }
    for (i = 0; i < AUDIO_AMP_DELAY_OFF_DEVICE_NUMBER; i ++) {
        if (amp_lock_device[i] == HAL_AUDIO_DEVICE_DAC_DUAL) {
            check_cg = AUDIO_POWER_DAC;
        } else if (amp_lock_device[i] == HAL_AUDIO_DEVICE_I2S_MASTER) {
            check_cg = AUDIO_POWER_I2S;
        } else {
            continue;
        }
        // 0xFFFFFFFD: it means all the scenarios exclude AMP.
        cg_cmp_used_exclude_amp_flag |= g_dsp_controller_cg[check_cg][3] ||
                                        g_dsp_controller_cg[check_cg][2] ||
                                        g_dsp_controller_cg[check_cg][1] ||
                                        (g_dsp_controller_cg[check_cg][0] & 0xFFFFFFFD);
    }
    if (type == AUDIO_SCENARIO_TYPE_AMP) {
        if ((hal_audio_status_query_running_flag_except(AUDIO_SCENARIO_TYPE_AMP)) &&
            cg_cmp_used_exclude_amp_flag) {
            return false;
        }
        if (!g_audio_clock_control.first_boot_flag) {
            g_audio_clock_control.first_boot_flag = false;
            hal_gpt_get_duration_count(g_audio_clock_control.amp_delay_off_timer_start_gpt_cnt, g_audio_clock_control.amp_delay_off_timer_stop_gpt_cnt, &amp_delay_off_timer_cnt);
            /* DSP and MCU is asynchronous, so we should do some protection to avoid some error casuing fast actions. */
            if ((AUDIO_AMP_DELAY_OFF_TIMER_DURATION_MS != 0) &&
                (amp_delay_off_timer_cnt < ((AUDIO_AMP_DELAY_OFF_TIMER_DURATION_MS - 300) * 1000))) {
                log_hal_msgid_info("[Audio Clock] AMP delay off time is abnormal %d %d", 2, amp_delay_off_timer_cnt, (AUDIO_AMP_DELAY_OFF_TIMER_DURATION_MS - 300) * 1000);
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief     This function is used to control all the releated clock settings or power of audio by scenario. This is the common api for users.
 * @param[in] type is to indicate the audio scenario type.
 * @param[in] param is the pointer of open parameters.
 * @param[in] is_running enable or disable.
 * @return    NONE.
 */
void hal_audio_status_set_running_flag(audio_scenario_type_t type, mcu2dsp_open_param_t *param, bool is_running)
{
#ifndef FPGA_ENV
    /* NOTE: This API should be called by each audio user! */
    log_hal_msgid_info("[Audio Clock] scenario type = [%d] param = 0x%x enable = [%d] ", 3,
                       type,
                       param,
                       is_running);
    // AMP delay off timer check
    hal_audio_status_get_amp_delay_off_timer_get_gpt_count(type, is_running);

    /* Special handle for AMP, to avoid timing issue of AMP */
    /* We must ignore the AMP signal when DAC is still lock by some scenario excluding AMP */
    if (!hal_audio_status_check_amp_lock(type, param, is_running)) {
        log_hal_msgid_warning("[Audio Clock] Warning: other scenario is still on, ignore amp!", 0);
        return;
    }
    /* -1-  Check power of DAC -------------------------------------------------------------------------------------- */
    hal_audio_status_set_dac(type, param, is_running);
    /* -2-  Check power of micbias ---------------------------------------------------------------------------------- */
    hal_audio_status_set_micbias(type, param, is_running);
    /* -3-  Check power of Audio Clock Gate ------------------------------------------------------------------------- */
    hal_audio_status_set_audio_clock_gate(type, param, is_running);
#endif /* FPGA_ENV */
}

bool hal_audio_status_query_running_flag(audio_scenario_type_t type)
{
    if (g_dsp_controller_cg_control_bit[type] != 0) {
        return true;
    } else {
        return false;
    }
}

bool hal_audio_status_query_running_flag_except(audio_scenario_type_t type)
{
    for (uint32_t i = 0; i < AUDIO_SCENARIO_TYPE_END; i++) {
        if (i == type) {
            continue;
        }
        if (g_dsp_controller_cg_control_bit[i] != 0) {
            return true;
        }
    }
    return false;
}

uint16_t hal_audio_status_query_running_flag_value()
{
    for (uint32_t i = 0; i < AUDIO_SCENARIO_TYPE_END; i++) {
        if (g_dsp_controller_cg_control_bit[i] != 0) {
            return 1;
        }
    }
    return false;
}

void hal_audio_status_clock_control_init(void)
{
    memset(&g_dsp_controller_cg[0][0], 0, sizeof(uint32_t) * AUDIO_POWER_END * ((AUDIO_SCEANRIO_TYPE_MAX + 31) / 32));
    memset(g_dsp_controller_cg_control_bit, 0, sizeof(uint32_t) * AUDIO_SCENARIO_TYPE_END);
    memset(&g_audio_clock_control, 0, sizeof(audio_clock_control_t));
}

#endif /*  */

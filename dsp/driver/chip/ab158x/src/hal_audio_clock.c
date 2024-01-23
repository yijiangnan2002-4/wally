/* Copyright Statement:
 *
 * (C) 2018  Airoha Technology Corp. All rights reserved.
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


#include "hal_audio_clock.h"
#include "hal_audio_register.h"
#include "hal_audio_control.h"
#include "hal_audio_driver.h"
#if 0//2822 clock control on CM4, modify for ab1568
#include "hal_spm.h"
#endif

#ifdef HAL_AUDIO_MODULE_ENABLED

#define HAL_AUDIO_CLOCK_CONTROL


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Functiion Declaration //////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern bool hal_audio_sub_component_id_resource_management(hal_audio_device_agent_t device_agent, hal_audio_sub_agent_t sub_agent, bool control);
extern bool hal_audio_status_get_all_agent_status();

void hal_audio_clock_initialize(void)
{

#if AB1568_BRING_UP_DSP_DEFAULT_HW_LOOPBACK

    // MTCMOS_AUDIO_ON TEMP!!!!
#define INFRA_MISC_CFG 0xA2200000
#define INFRA_CFG_MTCMOS3           ((volatile uint32_t*)(INFRA_MISC_CFG + 0x004C))
#define SPM_BASE    (0xA2110000)
#define SPM_AUDIO_PWR_CON                          ((volatile uint32_t*)(SPM_BASE + 0x0210))
#define SPM_AUDIO_G_CONTROL                        ((volatile uint32_t*)(SPM_BASE + 0x0498))
#define SPM_AUDIO_DFX_CH01_CONTROL                 ((volatile uint32_t*)(SPM_BASE + 0x049C))
#define SPM_AUDIO_DFX_CH23_CONTROL                 ((volatile uint32_t*)(SPM_BASE + 0x0500))
#define SPM_AUDIO_DFX_CH45_CONTROL                 ((volatile uint32_t*)(SPM_BASE + 0x0504))
#define SPM_AUDIO_UL1_CONTROL                      ((volatile uint32_t*)(SPM_BASE + 0x0508))
#define SPM_AUDIO_UL2_CONTROL                      ((volatile uint32_t*)(SPM_BASE + 0x050C))
#define SPM_AUDIO_UL3_CONTROL                      ((volatile uint32_t*)(SPM_BASE + 0x0510))
#define SPM_AUDIO_UL4_CONTROL                      ((volatile uint32_t*)(SPM_BASE + 0x0514))
#define SPM_AUDIO_DL_CONTROL                       ((volatile uint32_t*)(SPM_BASE + 0x0518))
#define SPM_AUDIO_AFE_MEM_IF_CONTROL_1             ((volatile uint32_t*)(SPM_BASE + 0x0520))
#define SPM_AUDIO_ASRC1_CONTROL_1                  ((volatile uint32_t*)(SPM_BASE + 0x530))
#define SPM_AUDIO_ASRC2_CONTROL_1                  ((volatile uint32_t*)(SPM_BASE + 0x540))

    //TINFO = " --- AUDIO power on by CM4 API "
    //*AUDIO_PWR_CON = 0x12; sleep
    *SPM_AUDIO_PWR_CON = 0x16; // [2]: pwr_on = 1
    *SPM_AUDIO_PWR_CON = 0x1E; // [3]: pwr_on_2nd = 1

    // --- AUDIO SRAM begin ------
    *SPM_AUDIO_AFE_MEM_IF_CONTROL_1 = 0x01000300;//[24]:MEM_ISO_EN_B [16]:RET2N [8]:RET1N [0]:PGEN
    *SPM_AUDIO_ASRC1_CONTROL_1      = 0x01000300;//[24]:MEM_ISO_EN_B [16]:RET2N [8]:RET1N [0]:PGEN
    *SPM_AUDIO_ASRC2_CONTROL_1      = 0x01000300;//[24]:MEM_ISO_EN_B [16]:RET2N [8]:RET1N [0]:PGEN

    *SPM_AUDIO_G_CONTROL        = 0x01000100;//[24]:MEM_ISO_EN_B [16]:RET2N [8]:RET1N [0]:PGEN
    *SPM_AUDIO_DFX_CH01_CONTROL     = 0x01000300;//[24]:MEM_ISO_EN_B [16]:RET2N [8]:RET1N [0]:PGEN
    *SPM_AUDIO_DFX_CH23_CONTROL     = 0x01000300;//[24]:MEM_ISO_EN_B [16]:RET2N [8]:RET1N [0]:PGEN
    *SPM_AUDIO_DFX_CH45_CONTROL     = 0x01000300;//[24]:MEM_ISO_EN_B [16]:RET2N [8]:RET1N [0]:PGEN
    *SPM_AUDIO_UL1_CONTROL      = 0x01000100;//[24]:MEM_ISO_EN_B [16]:RET2N [8]:RET1N [0]:PGEN
    *SPM_AUDIO_UL2_CONTROL      = 0x01000100;//[24]:MEM_ISO_EN_B [16]:RET2N [8]:RET1N [0]:PGEN
    *SPM_AUDIO_UL3_CONTROL      = 0x01000100;//[24]:MEM_ISO_EN_B [16]:RET2N [8]:RET1N [0]:PGEN
    *SPM_AUDIO_UL4_CONTROL      = 0x01000100;//[24]:MEM_ISO_EN_B [16]:RET2N [8]:RET1N [0]:PGEN
    *SPM_AUDIO_DL_CONTROL       = 0x01000100;//[24]:MEM_ISO_EN_B [16]:RET2N [8]:RET1N [0]:PGEN

    // --- AUDIO SRAM end ------
    *SPM_AUDIO_PWR_CON = 0xE;   // [4]: clk_dis = 0
    *SPM_AUDIO_PWR_CON = 0x1E;  // [4]: clk_dis = 1
    *SPM_AUDIO_PWR_CON = 0x1C;  // [4]: clk_dis = 1, iso = 0
    *SPM_AUDIO_PWR_CON = 0x1D;  // [0]: rstb = 1
    *SPM_AUDIO_PWR_CON = 0xD;   // [4]: clk_dis = 0

    //turn AUDIO protect_en=0
    *INFRA_CFG_MTCMOS3 = 0x0;
    //wait AUDIO protect ready=0
    //while(*INFRA_CFG_MTCMOS3!=0x0);//For ic

    //enable apll tuner
    AFE_SET_REG(AFE_APLL1_TUNER_CFG, 1, 1);
    //enable apll tuner
    AFE_SET_REG(AFE_APLL2_TUNER_CFG, 1, 1);

    AFE_WRITE(AUDIO_TOP_CON0, AUDIO_TOP_CON0_PDN_ALL_MASK);
    AFE_WRITE(AUDIO_TOP_CON1, AUDIO_TOP_CON1_PDN_ALL_MASK);
#endif



}

#if 0
int32_t hal_audio_critical_count_16bit(int16_t *counter, int32_t addition)
{
    int16_t value;
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    value = *counter + addition;
    if (value < 0) {
        value = 0;
    }
    *counter = value;
    hal_nvic_restore_interrupt_mask(mask);
    return value;
}

void hal_audio_clock_enable_afe(bool enable)
{
    if (enable) {
        afe_clock_afe_counter++;
        if (afe_clock_afe_counter == 1) {
            AFE_SET_REG(AUDIO_TOP_CON0, 0 << AUDIO_TOP_CON0_PDN_AFE_POS, AUDIO_TOP_CON0_PDN_AFE_MASK);
        }
    } else {
        afe_clock_afe_counter--;
        if (afe_clock_afe_counter == 0) {
            AFE_SET_REG(AUDIO_TOP_CON0, 1 << AUDIO_TOP_CON0_PDN_AFE_POS, AUDIO_TOP_CON0_PDN_AFE_MASK);
        } else if (afe_clock_afe_counter < 0) {
            afe_clock_afe_counter = 0;
        }
    }
}
#endif

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void hal_audio_clock_enable_i2s_slave_hclk(hal_audio_device_agent_t device_agent, bool enable)
{
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (hal_audio_sub_component_id_resource_management(device_agent, HAL_AUDIO_AFE_CLOCK_I2S_SLV_HCLK, enable)) {
        if (enable) {
            AFE_SET_REG(AUDIO_TOP_CON0, 0 << AUDIO_TOP_CON0_PDN_I2S_SLV_HCLK_POS, AUDIO_TOP_CON0_PDN_I2S_SLV_HCLK_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        } else {
            AFE_SET_REG(AUDIO_TOP_CON0, 1 << AUDIO_TOP_CON0_PDN_I2S_SLV_HCLK_POS, AUDIO_TOP_CON0_PDN_I2S_SLV_HCLK_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        }
        HAL_AUDIO_LOG_INFO("[Audio Agent] DSP - HAL_AUDIO_AFE_CLOCK_22M:%d", 1, enable);
    } else {
        hal_nvic_restore_interrupt_mask(mask);
    }
}


ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void hal_audio_clock_enable_22m(hal_audio_device_agent_t device_agent, bool enable)
{
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (hal_audio_sub_component_id_resource_management(device_agent, HAL_AUDIO_AFE_CLOCK_22M, enable)) {
        if (enable) {
            //Setting audio clock, engen 22M
            AFE_SET_REG(AUDIO_TOP_CON0, 0 << AUDIO_TOP_CON0_PDN_22M_POS, AUDIO_TOP_CON0_PDN_22M_MASK);
            //Enable HD engen 22m on
            AFE_SET_REG(AFE_HD_ENGEN_ENABLE, 1 << 0, 1 << 0);
            hal_nvic_restore_interrupt_mask(mask);
        } else {
            AFE_SET_REG(AFE_HD_ENGEN_ENABLE, 0 << 0, 1 << 0);
            AFE_SET_REG(AUDIO_TOP_CON0, 1 << AUDIO_TOP_CON0_PDN_22M_POS, AUDIO_TOP_CON0_PDN_22M_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        }
        HAL_AUDIO_LOG_INFO("[Audio Agent] DSP - HAL_AUDIO_AFE_CLOCK_22M:%d", 1, enable);
    } else {
        hal_nvic_restore_interrupt_mask(mask);
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void hal_audio_clock_enable_24m(hal_audio_device_agent_t device_agent, bool enable)
{
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (hal_audio_sub_component_id_resource_management(device_agent, HAL_AUDIO_AFE_CLOCK_24M, enable)) {
        if (enable) {
            //Setting audio clock, engen 24M
            AFE_SET_REG(AUDIO_TOP_CON0, 0 << AUDIO_TOP_CON0_PDN_24M_POS, AUDIO_TOP_CON0_PDN_24M_MASK);
            //Enable HD engen 24m on
            AFE_SET_REG(AFE_HD_ENGEN_ENABLE, 1 << 1, 1 << 1);
            hal_nvic_restore_interrupt_mask(mask);
        } else {
            AFE_SET_REG(AFE_HD_ENGEN_ENABLE, 0 << 1, 1 << 1);
            AFE_SET_REG(AUDIO_TOP_CON0, 1 << AUDIO_TOP_CON0_PDN_24M_POS, AUDIO_TOP_CON0_PDN_24M_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        }
        HAL_AUDIO_LOG_INFO("[Audio Agent] DSP - HAL_AUDIO_AFE_CLOCK_24M:%d", 1, enable);
    } else {
        hal_nvic_restore_interrupt_mask(mask);
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void hal_audio_clock_enable_apll2(hal_audio_device_agent_t device_agent, bool enable)
{
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (hal_audio_sub_component_id_resource_management(device_agent, HAL_AUDIO_AFE_CLOCK_APLL2, enable)) {
        if (enable) {
            AFE_SET_REG(AUDIO_TOP_CON0, 0 << AUDIO_TOP_CON0_PDN_APLL2_TUNER_POS, AUDIO_TOP_CON0_PDN_APLL2_TUNER_MASK);
            //Enable tuner
            AFE_SET_REG(AFE_APLL2_TUNER_CFG, 0x00000435, 0x0000FFFF);
            hal_nvic_restore_interrupt_mask(mask);
        } else {
            AFE_SET_REG(AFE_APLL2_TUNER_CFG, 0x0, 0x1);
            AFE_SET_REG(AUDIO_TOP_CON0, 1 << AUDIO_TOP_CON0_PDN_APLL2_TUNER_POS, AUDIO_TOP_CON0_PDN_APLL2_TUNER_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        }
        HAL_AUDIO_LOG_INFO("[Audio Agent] DSP - HAL_AUDIO_AFE_CLOCK_APLL2:%d", 1, enable);
    } else {
        hal_nvic_restore_interrupt_mask(mask);
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void hal_audio_clock_enable_apll(hal_audio_device_agent_t device_agent, bool enable)
{
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (hal_audio_sub_component_id_resource_management(device_agent, HAL_AUDIO_AFE_CLOCK_APLL, enable)) {
        if (enable) {
            AFE_SET_REG(AUDIO_TOP_CON0, 0 << AUDIO_TOP_CON0_PDN_APLL_TUNER_POS, AUDIO_TOP_CON0_PDN_APLL_TUNER_MASK);
            //Enable tuner
            AFE_SET_REG(AFE_APLL1_TUNER_CFG, 0x00000433, 0x0000FFFF);
            hal_nvic_restore_interrupt_mask(mask);
        } else {
            AFE_SET_REG(AFE_APLL1_TUNER_CFG, 0x0, 0x1);
            AFE_SET_REG(AUDIO_TOP_CON0, 1 << AUDIO_TOP_CON0_PDN_APLL_TUNER_POS, AUDIO_TOP_CON0_PDN_APLL_TUNER_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        }
        HAL_AUDIO_LOG_INFO("[Audio Agent] DSP - HAL_AUDIO_AFE_CLOCK_APLL:%d", 1, enable);
    } else {
        hal_nvic_restore_interrupt_mask(mask);
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void hal_audio_clock_enable_adc3(hal_audio_device_agent_t device_agent, bool enable)
{
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (hal_audio_sub_component_id_resource_management(device_agent, HAL_AUDIO_AFE_CLOCK_ADC45, enable)) {
        if (enable) {
            AFE_SET_REG(AUDIO_TOP_CON0, 0 << AUDIO_TOP_CON0_PDN_ADC3_POS, AUDIO_TOP_CON0_PDN_ADC3_MASK);
            AFE_SET_REG(AUDIO_TOP_CON1, 0 << AUDIO_TOP_CON1_PDN_ADDA6_POS, AUDIO_TOP_CON1_PDN_ADDA6_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        } else {
            AFE_SET_REG(AUDIO_TOP_CON1, 1 << AUDIO_TOP_CON1_PDN_ADDA6_POS, AUDIO_TOP_CON1_PDN_ADDA6_MASK);
            AFE_SET_REG(AUDIO_TOP_CON0, 1 << AUDIO_TOP_CON0_PDN_ADC3_POS, AUDIO_TOP_CON0_PDN_ADC3_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        }
        HAL_AUDIO_LOG_INFO("[Audio Agent] DSP - HAL_AUDIO_AFE_CLOCK_ADC45:%d", 1, enable);
    } else {
        hal_nvic_restore_interrupt_mask(mask);
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void hal_audio_clock_enable_adc2(hal_audio_device_agent_t device_agent, bool enable)
{
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (hal_audio_sub_component_id_resource_management(device_agent, HAL_AUDIO_AFE_CLOCK_ADC23, enable)) {
        if (enable) {
            AFE_SET_REG(AUDIO_TOP_CON0, 0 << AUDIO_TOP_CON0_PDN_ADC2_POS, AUDIO_TOP_CON0_PDN_ADC2_MASK);
            AFE_SET_REG(AUDIO_TOP_CON1, 0 << AUDIO_TOP_CON1_PDN_ADDA2_POS, AUDIO_TOP_CON1_PDN_ADDA2_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        } else {
            AFE_SET_REG(AUDIO_TOP_CON1, 1 << AUDIO_TOP_CON1_PDN_ADDA2_POS, AUDIO_TOP_CON1_PDN_ADDA2_MASK);
            AFE_SET_REG(AUDIO_TOP_CON0, 1 << AUDIO_TOP_CON0_PDN_ADC2_POS, AUDIO_TOP_CON0_PDN_ADC2_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        }
        HAL_AUDIO_LOG_INFO("[Audio Agent] DSP - HAL_AUDIO_AFE_CLOCK_ADC23:%d", 1, enable);
    } else {
        hal_nvic_restore_interrupt_mask(mask);
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void hal_audio_clock_enable_adc(hal_audio_device_agent_t device_agent, bool enable)
{
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (hal_audio_sub_component_id_resource_management(device_agent, HAL_AUDIO_AFE_CLOCK_ADC_COMMON, enable)) {
        if (enable) {
            AFE_SET_REG(AUDIO_TOP_CON0, 0 << AUDIO_TOP_CON0_PDN_ADC_POS, AUDIO_TOP_CON0_PDN_ADC_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        } else {
            AFE_SET_REG(AUDIO_TOP_CON0, 1 << AUDIO_TOP_CON0_PDN_ADC_POS, AUDIO_TOP_CON0_PDN_ADC_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        }
        HAL_AUDIO_LOG_INFO("[Audio Agent] DSP - HAL_AUDIO_AFE_CLOCK_ADC_COMMON:%d", 1, enable);
    } else {
        hal_nvic_restore_interrupt_mask(mask);
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void hal_audio_clock_enable_dac(hal_audio_device_agent_t device_agent, bool enable)
{
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (hal_audio_sub_component_id_resource_management(device_agent, HAL_AUDIO_AFE_CLOCK_DAC, enable)) {
        if (enable) {
            AFE_SET_REG(AUDIO_TOP_CON0, (0 << AUDIO_TOP_CON0_PDN_DAC_POS) | (0 << AUDIO_TOP_CON0_PDN_DAC_PREDIS_POS), AUDIO_TOP_CON0_PDN_DAC_MASK | AUDIO_TOP_CON0_PDN_DAC_PREDIS_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        } else {
            AFE_SET_REG(AUDIO_TOP_CON0, (1 << AUDIO_TOP_CON0_PDN_DAC_POS) | (1 << AUDIO_TOP_CON0_PDN_DAC_PREDIS_POS), AUDIO_TOP_CON0_PDN_DAC_MASK | AUDIO_TOP_CON0_PDN_DAC_PREDIS_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        }
        HAL_AUDIO_LOG_INFO("[Audio Agent] DSP - HAL_AUDIO_AFE_CLOCK_DAC:%d", 1, enable);
    } else {
        hal_nvic_restore_interrupt_mask(mask);
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void hal_audio_clock_enable_i2s0(hal_audio_device_agent_t device_agent, bool enable)
{
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (hal_audio_sub_component_id_resource_management(device_agent, HAL_AUDIO_AFE_CLOCK_I2S0, enable)) {
        if (enable) {
            AFE_SET_REG(AUDIO_TOP_CON1, 0 << AUDIO_TOP_CON1_PDN_I2S0_POS, AUDIO_TOP_CON1_PDN_I2S0_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        } else {
            AFE_SET_REG(AUDIO_TOP_CON1, 1 << AUDIO_TOP_CON1_PDN_I2S0_POS, AUDIO_TOP_CON1_PDN_I2S0_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        }
        HAL_AUDIO_LOG_INFO("[Audio Agent] DSP - HAL_AUDIO_AFE_CLOCK_I2S0:%d", 1, enable);
    } else {
        hal_nvic_restore_interrupt_mask(mask);
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void hal_audio_clock_enable_i2s1(hal_audio_device_agent_t device_agent, bool enable)
{
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (hal_audio_sub_component_id_resource_management(device_agent, HAL_AUDIO_AFE_CLOCK_I2S1, enable)) {
        if (enable) {
            AFE_SET_REG(AUDIO_TOP_CON1, 0 << AUDIO_TOP_CON1_PDN_I2S1_POS, AUDIO_TOP_CON1_PDN_I2S1_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        } else {
            AFE_SET_REG(AUDIO_TOP_CON1, 1 << AUDIO_TOP_CON1_PDN_I2S1_POS, AUDIO_TOP_CON1_PDN_I2S1_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        }
        HAL_AUDIO_LOG_INFO("[Audio Agent] DSP - HAL_AUDIO_AFE_CLOCK_I2S1:%d", 1, enable);
    } else {
        hal_nvic_restore_interrupt_mask(mask);
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void hal_audio_clock_enable_i2s2(hal_audio_device_agent_t device_agent, bool enable)
{
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (hal_audio_sub_component_id_resource_management(device_agent, HAL_AUDIO_AFE_CLOCK_I2S2, enable)) {
        if (enable) {
            AFE_SET_REG(AUDIO_TOP_CON1, 0 << AUDIO_TOP_CON1_PDN_I2S2_POS, AUDIO_TOP_CON1_PDN_I2S2_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        } else {
            AFE_SET_REG(AUDIO_TOP_CON1, 1 << AUDIO_TOP_CON1_PDN_I2S2_POS, AUDIO_TOP_CON1_PDN_I2S2_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        }
        HAL_AUDIO_LOG_INFO("[Audio Agent] DSP - HAL_AUDIO_AFE_CLOCK_I2S2:%d", 1, enable);
    } else {
        hal_nvic_restore_interrupt_mask(mask);
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void hal_audio_clock_enable_i2s3(hal_audio_device_agent_t device_agent, bool enable)
{
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (hal_audio_sub_component_id_resource_management(device_agent, HAL_AUDIO_AFE_CLOCK_I2S3, enable)) {
        if (enable) {
            AFE_SET_REG(AUDIO_TOP_CON1, 0 << AUDIO_TOP_CON1_PDN_I2S3_POS, AUDIO_TOP_CON1_PDN_I2S3_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        } else {
            AFE_SET_REG(AUDIO_TOP_CON1, 1 << AUDIO_TOP_CON1_PDN_I2S3_POS, AUDIO_TOP_CON1_PDN_I2S3_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        }
        HAL_AUDIO_LOG_INFO("[Audio Agent] DSP - HAL_AUDIO_AFE_CLOCK_I2S3:%d", 1, enable);
    } else {
        hal_nvic_restore_interrupt_mask(mask);
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void hal_audio_clock_enable_adc_hires(hal_audio_device_agent_t device_agent, bool enable)
{
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (hal_audio_sub_component_id_resource_management(device_agent, HAL_AUDIO_AFE_CLOCK_ADC_HIRES, enable)) {
        if (enable) {
            AFE_SET_REG(AUDIO_TOP_CON1, 0 << AUDIO_TOP_CON1_PDN_ADC_HIRES_POS, AUDIO_TOP_CON1_PDN_ADC_HIRES_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        } else {
            AFE_SET_REG(AUDIO_TOP_CON1, 1 << AUDIO_TOP_CON1_PDN_ADC_HIRES_POS, AUDIO_TOP_CON1_PDN_ADC_HIRES_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        }
        HAL_AUDIO_LOG_INFO("[Audio Agent] DSP - HAL_AUDIO_AFE_CLOCK_ADC_HIRES:%d", 1, enable);
    } else {
        hal_nvic_restore_interrupt_mask(mask);
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void hal_audio_clock_enable_adda_anc(hal_audio_device_agent_t device_agent, bool enable)
{
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (hal_audio_sub_component_id_resource_management(device_agent, HAL_AUDIO_AFE_CLOCK_ANC, enable)) {
        if (enable) {
            AFE_SET_REG(AUDIO_TOP_CON1, 0 << AUDIO_TOP_CON1_PDN_ADDA_ANC_POS, AUDIO_TOP_CON1_PDN_ADDA_ANC_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        } else {
            AFE_SET_REG(AUDIO_TOP_CON1, 1 << AUDIO_TOP_CON1_PDN_ADDA_ANC_POS, AUDIO_TOP_CON1_PDN_ADDA_ANC_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        }
        HAL_AUDIO_LOG_INFO("[Audio Agent] DSP - HAL_AUDIO_AFE_CLOCK_ANC:%d", 1, enable);
    } else {
        hal_nvic_restore_interrupt_mask(mask);
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void hal_audio_clock_enable_dac_hires(hal_audio_device_agent_t device_agent, bool enable)
{
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (hal_audio_sub_component_id_resource_management(device_agent, HAL_AUDIO_AFE_CLOCK_DAC_HIRES, enable)) {
        if (enable) {
            AFE_SET_REG(AUDIO_TOP_CON1, 0 << AUDIO_TOP_CON1_PDN_DAC_HIRES_POS, AUDIO_TOP_CON1_PDN_DAC_HIRES_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        } else {
            AFE_SET_REG(AUDIO_TOP_CON1, 1 << AUDIO_TOP_CON1_PDN_DAC_HIRES_POS, AUDIO_TOP_CON1_PDN_DAC_HIRES_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        }
        HAL_AUDIO_LOG_INFO("[Audio Agent] DSP - HAL_AUDIO_AFE_CLOCK_DAC_HIRES:%d", 1, enable);
    } else {
        hal_nvic_restore_interrupt_mask(mask);
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void hal_audio_clock_enable_src(afe_mem_asrc_id_t asrc_id, bool enable)
{
    uint32_t mask;
    hal_audio_device_agent_t device_agent = HAL_AUDIO_DEVICE_AGENT_DEVICE_HWSRC1;
    if (asrc_id == AFE_MEM_ASRC_1) {
        device_agent = HAL_AUDIO_DEVICE_AGENT_DEVICE_HWSRC1;
    } else if (asrc_id == AFE_MEM_ASRC_2) {
        device_agent = HAL_AUDIO_DEVICE_AGENT_DEVICE_HWSRC2;
    } else {
        HAL_AUDIO_LOG_ERROR(" SRC id is wrong = %d", 1, asrc_id);
        AUDIO_ASSERT(0);
    }
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (hal_audio_sub_component_id_resource_management(device_agent, HAL_AUDIO_AFE_CLOCK_SRC_COMMON, enable)) {
        if (enable) {
            AFE_SET_REG(AUDIO_TOP_CON1, 0 << AUDIO_TOP_CON1_PDN_DRAM_BRIDGE_POS, AUDIO_TOP_CON1_PDN_DRAM_BRIDGE_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        } else {
#ifdef ENABLE_HWSRC_ON_MAIN_STREAM
            U32 loop_count = 0;
            while ((AFE_GET_REG(MEM_ASRC_TOP_MON0) & 0xF) != 7) {
                if (loop_count >= 200 && loop_count % 100 == 0) {
                    HAL_AUDIO_LOG_INFO("DSP - src  loop_count:%d, 0x1018:0x%x, 0x101C:0x%x, 0x1150:0x%x, 0x1140:0x%x, 0x1160:0x%x, 0x1170:0x%x, 0x11E8:0x%x, 0x11EC:0x%x, 0x11F8:0x%x, 0x11FC:0x%x, 0x0010:0x%x,0x0FDC:0x%x,0x1100:0x%x,0x0000:0x%x,0x0004:0x%x,0x1120:0x%x,0x1124:0x%x,0x1128:0x%x,0x112C:0x%x", 20, loop_count,
                                       AFE_GET_REG(MEM_ASRC_TOP_MON0), AFE_GET_REG(MEM_ASRC_TOP_MON1), AFE_GET_REG(ASM_CH01_IBUF_WRPNT), AFE_GET_REG(ASM_CH01_IBUF_RDPNT), AFE_GET_REG(ASM_CH01_OBUF_WRPNT), AFE_GET_REG(ASM_CH01_OBUF_RDPNT),
                                       AFE_GET_REG(ASM_IN_BUF_MON0), AFE_GET_REG(ASM_IN_BUF_MON1), AFE_GET_REG(ASM_OUT_BUF_MON0), AFE_GET_REG(ASM_OUT_BUF_MON1), AFE_GET_REG(AFE_DAC_CON0), AFE_GET_REG(AFE_AUDIO_BT_SYNC_MON2), AFE_GET_REG(ASM_GEN_CONF), AFE_GET_REG(AUDIO_TOP_CON0), AFE_GET_REG(AUDIO_TOP_CON1),
                                       AFE_GET_REG(ASM_FREQUENCY_0), AFE_GET_REG(ASM_FREQUENCY_1), AFE_GET_REG(ASM_FREQUENCY_2), AFE_GET_REG(ASM_FREQUENCY_3));
                    assert(FALSE);
                }
                loop_count++;
                HAL_AUDIO_DELAY_US(10);
            }
            //HAL_AUDIO_LOG_INFO(" SRC JUMP loop_count = %d", 1, loop_count);
#endif
            AFE_SET_REG(AUDIO_TOP_CON1, 1 << AUDIO_TOP_CON1_PDN_DRAM_BRIDGE_POS, AUDIO_TOP_CON1_PDN_DRAM_BRIDGE_MASK); //BTA-39732 HWSRC hang issue
            hal_nvic_restore_interrupt_mask(mask);
        }
        HAL_AUDIO_LOG_INFO("[Audio Agent] DSP - HAL_AUDIO_AFE_CLOCK_SRC_COMMON:%d", 1, enable);
    } else {
        hal_nvic_restore_interrupt_mask(mask);
    }
}

void hal_audio_clock_enable_src1(afe_mem_asrc_id_t asrc_id, bool enable)
{
    hal_audio_device_agent_t device_agent = HAL_AUDIO_DEVICE_AGENT_DEVICE_HWSRC1;
    if (asrc_id == AFE_MEM_ASRC_1) {
        device_agent = HAL_AUDIO_DEVICE_AGENT_DEVICE_HWSRC1;
    } else if (asrc_id == AFE_MEM_ASRC_2) {
        device_agent = HAL_AUDIO_DEVICE_AGENT_DEVICE_HWSRC2;
    } else {
        HAL_AUDIO_LOG_ERROR(" SRC id is wrong = %d", 1, asrc_id);
        AUDIO_ASSERT(0);
    }
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (hal_audio_sub_component_id_resource_management(device_agent, HAL_AUDIO_AFE_CLOCK_SRC1, enable)) {
        if (enable) {
            AFE_SET_REG(AUDIO_TOP_CON1, 0 << AUDIO_TOP_CON1_PDN_ASRC1_POS, AUDIO_TOP_CON1_PDN_ASRC1_MASK);
            afe_src_configuration_t src_configuration;
            memset(&src_configuration, 0, sizeof(afe_src_configuration_t));
            src_configuration.id = AFE_MEM_ASRC_1;
            hal_src_set_irq_enable(&src_configuration, false);
            hal_nvic_restore_interrupt_mask(mask);
        } else {
#ifdef ENABLE_HWSRC_ON_MAIN_STREAM
            U32 loop_count = 0;
            while ((AFE_GET_REG(ASM_OUT_BUF_MON0) & 0xFF) != 0XF1) {
                if ((loop_count % 5) == 0) {
                    HAL_AUDIO_LOG_INFO("DSP - src1 28bit loop_count:%d, 0x1018:0x%x, 0x101C:0x%x, 0x1150:0x%x, 0x1140:0x%x, 0x1160:0x%x, 0x1170:0x%x, 0x11E8:0x%x, 0x11EC:0x%x, 0x11F8:0x%x, 0x11FC:0x%x, 0x0010:0x%x,0x0FDC:0x%x,0x1100:0x%x,0x0000:0x%x,0x0004:0x%x,0x1120:0x%x,0x1124:0x%x,0x1128:0x%x,0x112C:0x%x", 20, loop_count,
                                       AFE_GET_REG(MEM_ASRC_TOP_MON0), AFE_GET_REG(MEM_ASRC_TOP_MON1), AFE_GET_REG(ASM_CH01_IBUF_WRPNT), AFE_GET_REG(ASM_CH01_IBUF_RDPNT), AFE_GET_REG(ASM_CH01_OBUF_WRPNT), AFE_GET_REG(ASM_CH01_OBUF_RDPNT),
                                       AFE_GET_REG(ASM_IN_BUF_MON0), AFE_GET_REG(ASM_IN_BUF_MON1), AFE_GET_REG(ASM_OUT_BUF_MON0), AFE_GET_REG(ASM_OUT_BUF_MON1), AFE_GET_REG(AFE_DAC_CON0), AFE_GET_REG(AFE_AUDIO_BT_SYNC_MON2), AFE_GET_REG(ASM_GEN_CONF), AFE_GET_REG(AUDIO_TOP_CON0), AFE_GET_REG(AUDIO_TOP_CON1),
                                       AFE_GET_REG(ASM_FREQUENCY_0), AFE_GET_REG(ASM_FREQUENCY_1), AFE_GET_REG(ASM_FREQUENCY_2), AFE_GET_REG(ASM_FREQUENCY_3));
                }
                if (loop_count >= 200 && loop_count % 100 == 0) {
                    HAL_AUDIO_LOG_INFO("DSP - src1 28bit loop_count:%d, 0x1018:0x%x, 0x101C:0x%x, 0x1150:0x%x, 0x1140:0x%x, 0x1160:0x%x, 0x1170:0x%x, 0x11E8:0x%x, 0x11EC:0x%x, 0x11F8:0x%x, 0x11FC:0x%x, 0x0010:0x%x,0x0FDC:0x%x,0x1100:0x%x,0x0000:0x%x,0x0004:0x%x,0x1120:0x%x,0x1124:0x%x,0x1128:0x%x,0x112C:0x%x", 20, loop_count,
                                       AFE_GET_REG(MEM_ASRC_TOP_MON0), AFE_GET_REG(MEM_ASRC_TOP_MON1), AFE_GET_REG(ASM_CH01_IBUF_WRPNT), AFE_GET_REG(ASM_CH01_IBUF_RDPNT), AFE_GET_REG(ASM_CH01_OBUF_WRPNT), AFE_GET_REG(ASM_CH01_OBUF_RDPNT),
                                       AFE_GET_REG(ASM_IN_BUF_MON0), AFE_GET_REG(ASM_IN_BUF_MON1), AFE_GET_REG(ASM_OUT_BUF_MON0), AFE_GET_REG(ASM_OUT_BUF_MON1), AFE_GET_REG(AFE_DAC_CON0), AFE_GET_REG(AFE_AUDIO_BT_SYNC_MON2), AFE_GET_REG(ASM_GEN_CONF), AFE_GET_REG(AUDIO_TOP_CON0), AFE_GET_REG(AUDIO_TOP_CON1),
                                       AFE_GET_REG(ASM_FREQUENCY_0), AFE_GET_REG(ASM_FREQUENCY_1), AFE_GET_REG(ASM_FREQUENCY_2), AFE_GET_REG(ASM_FREQUENCY_3));
                    assert(FALSE);
                }
                loop_count++;
                HAL_AUDIO_DELAY_US(10);
            }
            hal_nvic_restore_interrupt_mask(mask);
            HAL_AUDIO_LOG_INFO(" SRC1 JUMP loop_count = %d", 1, loop_count);
#endif
            AFE_SET_REG(AUDIO_TOP_CON1, 1 << AUDIO_TOP_CON1_PDN_ASRC1_POS, AUDIO_TOP_CON1_PDN_ASRC1_MASK);
            HAL_AUDIO_DELAY_US(5);
            AFE_SET_REG(MEM_ASRC_TOP_CON0, 1 << MEM_ASRC_TOP_CON0_MASM1_RST_POS, MEM_ASRC_TOP_CON0_MASM1_RST_MASK);
            AFE_SET_REG(MEM_ASRC_TOP_CON0, 0 << MEM_ASRC_TOP_CON0_MASM1_RST_POS, MEM_ASRC_TOP_CON0_MASM1_RST_MASK);
        }
        HAL_AUDIO_LOG_INFO("[Audio Agent] DSP - HAL_AUDIO_AFE_CLOCK_SRC1:%d", 1, enable);
    } else {
        hal_nvic_restore_interrupt_mask(mask);
    }
    HAL_AUDIO_DELAY_US(5);
}

void hal_audio_clock_enable_src2(afe_mem_asrc_id_t asrc_id, bool enable)
{
    hal_audio_device_agent_t device_agent = HAL_AUDIO_DEVICE_AGENT_DEVICE_HWSRC1;
    if (asrc_id == AFE_MEM_ASRC_1) {
        device_agent = HAL_AUDIO_DEVICE_AGENT_DEVICE_HWSRC1;
    } else if (asrc_id == AFE_MEM_ASRC_2) {
        device_agent = HAL_AUDIO_DEVICE_AGENT_DEVICE_HWSRC2;
    } else {
        HAL_AUDIO_LOG_ERROR(" SRC id is wrong = %d", 1, asrc_id);
        AUDIO_ASSERT(0);
    }
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (hal_audio_sub_component_id_resource_management(device_agent, HAL_AUDIO_AFE_CLOCK_SRC2, enable)) {
        if (enable) {
            AFE_SET_REG(AUDIO_TOP_CON1, 0 << AUDIO_TOP_CON1_PDN_ASRC2_POS, AUDIO_TOP_CON1_PDN_ASRC2_MASK);
            afe_src_configuration_t src_configuration;
            memset(&src_configuration, 0, sizeof(afe_src_configuration_t));
            src_configuration.id = AFE_MEM_ASRC_2;
            hal_src_set_irq_enable(&src_configuration, false);
            hal_nvic_restore_interrupt_mask(mask);
        } else {
#ifdef ENABLE_HWSRC_ON_MAIN_STREAM
            U32 loop_count = 0;
            while ((AFE_GET_REG(ASM2_OUT_BUF_MON0) & 0xFF) != 0XF1) {
                if (loop_count >= 200 && loop_count % 100 == 0) {
                    HAL_AUDIO_LOG_INFO("DSP - src2 28bit loop_count:%d, 0x1018:0x%x, 0x101C:0x%x, 0x1250:0x%x, 0x1240:0x%x, 0x1260:0x%x, 0x1270:0x%x, 0x12F8:0x%x, 0x12FC:0x%x, 0x12F8:0x%x, 0x12FC:0x%x, 0x0010:0x%x,0x0FDC:0x%x,0x1200:0x%x,0x0000:0x%x,0x0004:0x%x,0x1220:0x%x,0x1224:0x%x,0x1228:0x%x,0x122C:0x%x", 20, loop_count,
                                       AFE_GET_REG(MEM_ASRC_TOP_MON0), AFE_GET_REG(MEM_ASRC_TOP_MON1), AFE_GET_REG(ASM2_CH01_IBUF_WRPNT), AFE_GET_REG(ASM2_CH01_IBUF_RDPNT), AFE_GET_REG(ASM2_CH01_OBUF_WRPNT), AFE_GET_REG(ASM2_CH01_OBUF_RDPNT),
                                       AFE_GET_REG(ASM2_IN_BUF_MON0), AFE_GET_REG(ASM2_IN_BUF_MON1), AFE_GET_REG(ASM2_OUT_BUF_MON0), AFE_GET_REG(ASM2_OUT_BUF_MON1), AFE_GET_REG(AFE_DAC_CON0), AFE_GET_REG(AFE_AUDIO_BT_SYNC_MON2), AFE_GET_REG(ASM2_GEN_CONF), AFE_GET_REG(AUDIO_TOP_CON0), AFE_GET_REG(AUDIO_TOP_CON1),
                                       AFE_GET_REG(ASM2_FREQUENCY_0), AFE_GET_REG(ASM2_FREQUENCY_1), AFE_GET_REG(ASM2_FREQUENCY_2), AFE_GET_REG(ASM2_FREQUENCY_3));
                    assert(FALSE);
                }
                loop_count++;
                HAL_AUDIO_DELAY_US(10);
            }
            //HAL_AUDIO_LOG_INFO(" SRC2 JUMP loop_count = %d", 1, loop_count);
#endif
            AFE_SET_REG(AUDIO_TOP_CON1, 1 << AUDIO_TOP_CON1_PDN_ASRC2_POS, AUDIO_TOP_CON1_PDN_ASRC2_MASK);
            HAL_AUDIO_DELAY_US(5);
            AFE_SET_REG(MEM_ASRC_TOP_CON0, 1 << MEM_ASRC_TOP_CON0_MASM2_RST_POS, MEM_ASRC_TOP_CON0_MASM2_RST_MASK);
            AFE_SET_REG(MEM_ASRC_TOP_CON0, 0 << MEM_ASRC_TOP_CON0_MASM2_RST_POS, MEM_ASRC_TOP_CON0_MASM2_RST_MASK);
            hal_nvic_restore_interrupt_mask(mask);
        }
        HAL_AUDIO_LOG_INFO("[Audio Agent] DSP - HAL_AUDIO_AFE_CLOCK_SRC2:%d", 1, enable);
    } else {
        hal_nvic_restore_interrupt_mask(mask);
    }
    HAL_AUDIO_DELAY_US(5);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void hal_audio_afe_set_enable(bool enable)
{
    uint32_t mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if (hal_audio_sub_component_id_resource_management(HAL_AUDIO_DEVICE_AGENT_DEVICE_AFE, HAL_AUDIO_AFE_CLOCK_AFE, enable)) {
        if (enable) {
            //Default clock setting
            AFE_WRITE(AUDIO_TOP_CON0, AUDIO_TOP_CON0_PDN_ALL_MASK);
            AFE_WRITE(AUDIO_TOP_CON1, AUDIO_TOP_CON1_PDN_ALL_MASK);
            //Enable clock
            AFE_SET_REG(AUDIO_TOP_CON0, 0 << AUDIO_TOP_CON0_PDN_AFE_POS, AUDIO_TOP_CON0_PDN_AFE_MASK);
            //hal_audio_clock_enable_afe(enable);
            AFE_SET_REG(AFE_DAC_CON0, 1 << AFE_DAC_CON0_AFE_ON_POS, AFE_DAC_CON0_AFE_ON_MASK);
        } else {
            AFE_SET_REG(AFE_DAC_CON0, 0 << AFE_DAC_CON0_AFE_ON_POS, AFE_DAC_CON0_AFE_ON_MASK);
            //Disable clock
            //hal_audio_clock_enable_afe(enable);
            AFE_SET_REG(AUDIO_TOP_CON0, 1 << AUDIO_TOP_CON0_PDN_AFE_POS, AUDIO_TOP_CON0_PDN_AFE_MASK);
        }
        hal_nvic_restore_interrupt_mask(mask);
        HAL_AUDIO_LOG_INFO("[Audio Agent] DSP - Hal Audio AFE control:%d", 1, enable);
    } else {
        hal_nvic_restore_interrupt_mask(mask);
    }
}
/*
uint32_t hal_audio_afe_get_counter(void)
{
    return (uint32_t)afe_enable_afe_on_counter;
}
*/
#if 0
void hal_audio_afe_enable_clksq(bool enable)
{
    if (enable) {
        afe_enable_clksq_counter++;
        if (afe_enable_clksq_counter == 1) {
            ANA_SET_REG((AUDDEC_ANA_CON10),
                        ((1 << AUDDEC_ANA_CON10_1P5V_LDO_SEL_POS) | (1 << AUDDEC_ANA_CON10_0P8V_LDO_SEL_POS) | (3 << AUDDEC_ANA_CON10_GLB_RSV_POS)),
                        (AUDDEC_ANA_CON10_1P5V_LDO_SEL_MASK | AUDDEC_ANA_CON10_0P8V_LDO_SEL_MASK | AUDDEC_ANA_CON10_GLB_RSV_MASK));
            ANA_SET_REG((AUDDEC_ANA_CON9),
                        ((0 << AUDDEC_ANA_CON9_26M_CLK_SRC_POS) | (0 << AUDDEC_ANA_CON9_13M_DIV_POS) | (2 << AUDDEC_ANA_CON9_13M_D5NS_DELAY_POS) | (0 << AUDDEC_ANA_CON9_13M_D5NS_INV_POS) | (0 << AUDDEC_ANA_CON9_13M_DUTY_POS) | (0 << AUDDEC_ANA_CON9_CLKGEN_RSV_POS)),
                        (AUDDEC_ANA_CON9_26M_CLK_SRC_MASK | AUDDEC_ANA_CON9_13M_DIV_MASK | AUDDEC_ANA_CON9_13M_D5NS_DELAY_MASK | AUDDEC_ANA_CON9_13M_D5NS_INV_MASK | AUDDEC_ANA_CON9_13M_DUTY_MASK | AUDDEC_ANA_CON9_CLKGEN_RSV_MASK));
            ANA_SET_REG(AUDDEC_ANA_CON9, 1 << AUDDEC_ANA_CON9_DECODER_RST_POS, AUDDEC_ANA_CON9_DECODER_RST_MASK);           //Reset clock driver
        }
    } else {
        afe_enable_clksq_counter--;
        if (afe_enable_clksq_counter == 0) {
            ANA_SET_REG(AUDDEC_ANA_CON9, 0, (AUDDEC_ANA_CON9_26M_CLK_SRC_MASK | AUDDEC_ANA_CON9_13M_DIV_MASK | AUDDEC_ANA_CON9_13M_D5NS_DELAY_MASK | AUDDEC_ANA_CON9_13M_D5NS_INV_MASK | AUDDEC_ANA_CON9_13M_DUTY_MASK | AUDDEC_ANA_CON9_CLKGEN_RSV_MASK));
            ANA_SET_REG(AUDDEC_ANA_CON9, 0 << AUDDEC_ANA_CON9_DECODER_RST_POS, AUDDEC_ANA_CON9_DECODER_RST_MASK);           //Reset clock driver
            ANA_SET_REG(AUDDEC_ANA_CON10, 0, (AUDDEC_ANA_CON10_1P5V_LDO_SEL_MASK | AUDDEC_ANA_CON10_0P8V_LDO_SEL_MASK | AUDDEC_ANA_CON10_GLB_RSV_MASK));
        } else if (afe_enable_clksq_counter < 0) {
            afe_enable_clksq_counter = 0;
        }
    }
    HAL_AUDIO_LOG_INFO("DSP - Hal Audio AFE CLKSQ control:%d, cnt:%d", 2, enable, afe_enable_clksq_counter);
}
#endif

void hal_audio_clock_set_dac(hal_audio_device_agent_t device_agent, bool enable)
{
    hal_audio_clock_enable_dac(device_agent, enable);
    //hal_audio_clock_enable_adda2(enable);
    //hal_audio_clock_enable_adda6(enable);
}



#endif /*HAL_AUDIO_MODULE_ENABLED*/

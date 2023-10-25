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
#include "hal_pmu.h"
#include "hal_pmu_internal_hp.h"
#include "hal_pmu_auxadc_hp.h"
#include "hal_pmu_charger_hp.h"
#include "hal_pmu_hp_platform.h"
#include "hal_hw_semaphore.h"
#include "hal_resource_assignment.h"
#define AUXADC_CH0_BASE 0                  /* Data : Index for auxadc channel 0 debug info*/
#define AUXADC_CH1_BASE 10                 /* Data : Index for auxadc channel 1 debug info*/
#define AUXADC_CH2_BASE 20                 /* Data : Index for auxadc channel 2 debug info*/
#define AUXADC_CH3_BASE 30                 /* Data : Index for auxadc channel 3 debug info*/
#define AUXADC_CH4_BASE 40                 /* Data : Index for auxadc channel 4 debug info*/
#define AUXADC_CHECK_TIMES 50              /* Data : Request auxadc times*/
#define AUXADC_CHECK_MAX_TIMES 120         /* Data : Request auxadc max times*/

uint8_t re_request_flag = 0;               /* Flag : Request auxadc flag*/
uint8_t auxadc_status_index = 0;           /* Data : data for auxadc request status*/
uint32_t ex_vbatout = 0;                   /* Data : previous vbat adc value*/
auxadc_efuse_data_stru gAuxadcEfuseInfo;   /* Data : auxadc efuse struct*/
/*==========[Basic]==========*/
void pmu_auxadc_init(void)
{
    pmu_set_register_value_hp(PMU_AUXADC_AD_CON0, AUXADC_CK_AON_MASK, AUXADC_CK_AON_SHIFT, 0); //setting of dynamic CLK Managerment
    pmu_set_register_value_hp(PMU_STRUP_CON1, RG_STRUP_AUXADC_RSTB_SW_MASK, RG_STRUP_AUXADC_RSTB_SW_SHIFT, 1);//auxadc not being reset on sleep mode
    pmu_set_register_value_hp(PMU_STRUP_CON1, RG_STRUP_AUXADC_RSTB_SEL_MASK, RG_STRUP_AUXADC_RSTB_SEL_SHIFT, 1);
}
void pmu_thermal_parameter_init_hp(void)
{
    memset(&gAuxadcEfuseInfo, 0, sizeof(auxadc_efuse_data_stru));
    pmu_efuse_enable_reading();
    gAuxadcEfuseInfo.byte0 = pmu_core_efuse_get_efuse_data_by_address(30);
    gAuxadcEfuseInfo.byte1 = pmu_core_efuse_get_efuse_data_by_address(32);
    pmu_efuse_disable_reading();
    gAuxadcEfuseInfo.isInit = 1;
}
uint32_t pmu_auxadc_get_channel_value_hp(pmu_adc_channel_t Channel)
{
    uint32_t adc_result = -1;
    uint32_t adc_out = -1;
    pmu_lock_va18_hp(PMU_ON);
    pmu_enable_power(PMU_LDO_VA18, PMU_ON);
    if (Channel >= PMU_AUX_MAX) {
        log_hal_msgid_info("[PMU_ADC]error auxadc channel\r\n", 0);
        return (0xFFFFFFFF);    //Channel error
    }
    switch (Channel) {
        case PMU_AUX_PN_ZCV:
            adc_out = pmu_get_register_value_hp(PMU_AUXADC_AD_ADC20, AUXADC_ADC_OUT_PWRON_PCHR_MASK, AUXADC_ADC_OUT_PWRON_PCHR_SHIFT);
            adc_result = (adc_out * 1800 * 3) / 32768;
            break;

        case PMU_AUX_WK_ZCV:
            adc_out = pmu_get_register_value_hp(PMU_AUXADC_AD_ADC22, AUXADC_ADC_OUT_WAKEUP_PCHR_MASK, AUXADC_ADC_OUT_WAKEUP_PCHR_SHIFT);
            adc_result = (adc_out * 1800 * 3) / 32768;
            break;

        case PMU_AUX_BATSNS:
            if (pmu_auxadc_channel_request(0) != PMU_STATUS_SUCCESS) {
                log_hal_msgid_info("[PMU_ADC]auxadc channel error %d use ex-vbat adc\r\n", 1, PMU_AUX_BATSNS);
                adc_out = ex_vbatout;
            }else{
                adc_out = pmu_get_register_value_hp(PMU_AUXADC_AD_ADC0, AUXADC_ADC_OUT_CH0_MASK, AUXADC_ADC_OUT_CH0_SHIFT);
                ex_vbatout = adc_out;
            }
            adc_result = ((float)adc_out * 1800 * 3) / 32768;
            break;
        case PMU_AUX_VBUS_UART:
            pmu_set_register_value_hp(PMU_CORE_CORE_AO_CTRL_0, RG_BUS_SNS_LOGIC_SEL_MASK, RG_BUS_SNS_LOGIC_SEL_SHIFT, 0);
            if (pmu_auxadc_channel_request(1) != PMU_STATUS_SUCCESS) {
                log_hal_msgid_info("[PMU_ADC]auxadc channel error %d\r\n", 1, PMU_AUX_BATSNS);
            }
            adc_out = pmu_get_register_value_hp(PMU_AUXADC_AD_ADC1, AUXADC_ADC_OUT_CH1_MASK, AUXADC_ADC_OUT_CH1_SHIFT);
            adc_result = (adc_out * 1800 * 8) / 4096;
            pmu_set_register_value_hp(PMU_CORE_CORE_AO_CTRL_0, RG_BUS_SNS_LOGIC_SEL_MASK, RG_BUS_SNS_LOGIC_SEL_SHIFT, 1);
            break;

        case PMU_AUX_BAT_RECHARGER:
            if (pmu_auxadc_channel_request(0) != PMU_STATUS_SUCCESS) {
                log_hal_msgid_info("[PMU_ADC]auxadc channel error %d\r\n", 1, PMU_AUX_BAT_RECHARGER);
            }
            adc_out = pmu_get_register_value_hp(PMU_AUXADC_AD_ADC37, AUXADC_ADC_OUT_VBATSNS_DET_MASK, AUXADC_ADC_OUT_VBATSNS_DET_SHIFT);
            adc_result = (adc_out * 1800 * 3) / 4096;
            break;
        case PMU_AUX_VBUS:
            pmu_set_register_value_hp(PMU_CORE_CORE_AO_CTRL_0, RG_BUS_SNS_LOGIC_SEL_MASK, RG_BUS_SNS_LOGIC_SEL_SHIFT, 0);
            if (pmu_auxadc_channel_request(2) != PMU_STATUS_SUCCESS) {
                log_hal_msgid_info("[PMU_ADC]auxadc channel error %d\r\n", 1, PMU_AUX_VBUS);
            }
            adc_out = pmu_get_register_value_hp(PMU_AUXADC_AD_ADC2, AUXADC_ADC_OUT_CH2_MASK, AUXADC_ADC_OUT_CH2_SHIFT);
            adc_result = (adc_out * 1800 * 8) / 4096;
            pmu_set_register_value_hp(PMU_CORE_CORE_AO_CTRL_0, RG_BUS_SNS_LOGIC_SEL_MASK, RG_BUS_SNS_LOGIC_SEL_SHIFT, 1);
            break;
        case PMU_AUX_CHR_THM:
            pmu_set_register_value_hp(PMU_AUXADC_AD_RQST0, AUXADC_RQST_CH3_MASK, AUXADC_RQST_CH3_SHIFT, 1);
            do {
            } while (pmu_get_register_value_hp(PMU_AUXADC_AD_ADC3, AUXADC_ADC_RDY_CH3_MASK, AUXADC_ADC_RDY_CH3_SHIFT) != 1);
            adc_out = pmu_get_register_value_hp(PMU_AUXADC_AD_ADC3, AUXADC_ADC_OUT_CH3_MASK, AUXADC_ADC_OUT_CH3_SHIFT);
            adc_result = (adc_out * 1800) / 4096;
            break;

        case PMU_AUX_HW_JEITA:
            adc_out = pmu_get_register_value_hp(PMU_AUXADC_AD_ADC35, AUXADC_ADC_OUT_JEITA_MASK, AUXADC_ADC_OUT_JEITA_SHIFT);
            adc_result = (adc_out * 1800) / 4096;
            break;
        case PMU_AUX_PMIC_AP:
            if (pmu_auxadc_channel_request(4) != PMU_STATUS_SUCCESS) {
                log_hal_msgid_info("[PMU_ADC]auxadc channel error %d\r\n", 1, PMU_AUX_PMIC_AP);
            }
            adc_out = pmu_get_register_value_hp(PMU_AUXADC_AD_ADC4, AUXADC_ADC_OUT_CH4_MASK, AUXADC_ADC_OUT_CH4_SHIFT);
            adc_result = adc_out;
            break;
    }
    pmu_lock_va18_hp(PMU_OFF);
    pmu_enable_power(PMU_LDO_VA18, PMU_OFF);
    if (re_request_flag) {
        log_hal_msgid_info("[PMU_ADC]re_request happen", 0);
        re_request_flag = 0;
    }
    log_hal_msgid_info("[PMU_ADC]auxadc_status :%d Ch:%d Val:%d adc_result:%x", 4, auxadc_status_index, Channel, adc_result, adc_out);
    return adc_result;
}
int8_t pmu_auxadc_channel_request(uint8_t channel)
{
    pmu_status_t request_state = -1;
    uint8_t channel_status = 0;
    switch (channel) {
        case 0:
            pmu_set_register_value_hp(PMU_AUXADC_AD_RQST0, AUXADC_RQST_CH0_MASK, AUXADC_RQST_CH0_SHIFT, 1);
            do {
                if (channel_status > AUXADC_CHECK_TIMES) {  //Re-request
                    pmu_set_register_value_hp(PMU_AUXADC_AD_RQST0, AUXADC_RQST_CH0_MASK, AUXADC_RQST_CH0_SHIFT, 1);
                    re_request_flag = 1;
                }
                if (channel_status > AUXADC_CHECK_MAX_TIMES) { //Request fail
                    request_state = PMU_STATUS_ERROR;
                    return request_state;
                }
                channel_status++;
                auxadc_status_index = AUXADC_CH0_BASE + channel_status;
            } while (pmu_get_register_value_hp(PMU_AUXADC_AD_ADC0, AUXADC_ADC_RDY_CH0_MASK, AUXADC_ADC_RDY_CH0_SHIFT) != 1);
            request_state = PMU_STATUS_SUCCESS;
            break;
        case 1:
            pmu_set_register_value_hp(PMU_AUXADC_AD_RQST0, AUXADC_RQST_CH1_MASK, AUXADC_RQST_CH1_SHIFT, 1);
            do {
                if (channel_status > AUXADC_CHECK_TIMES) {  //Re-request
                    pmu_set_register_value_hp(PMU_AUXADC_AD_RQST0, AUXADC_RQST_CH1_MASK, AUXADC_RQST_CH1_SHIFT, 1);
                    re_request_flag = 1;
                }
                if (channel_status > AUXADC_CHECK_MAX_TIMES) { //Request fail
                    request_state = PMU_STATUS_ERROR;
                    return request_state;
                }
                channel_status++;
                auxadc_status_index = AUXADC_CH1_BASE + channel_status;
            } while (pmu_get_register_value_hp(PMU_AUXADC_AD_ADC1, AUXADC_ADC_RDY_CH1_MASK, AUXADC_ADC_RDY_CH1_SHIFT) != 1);
            request_state = PMU_STATUS_SUCCESS;
            break;
        case 2:
            pmu_set_register_value_hp(PMU_AUXADC_AD_RQST0, AUXADC_RQST_CH2_MASK, AUXADC_RQST_CH2_SHIFT, 1);
            do {
                if (channel_status > AUXADC_CHECK_TIMES) {  //Re-request
                    pmu_set_register_value_hp(PMU_AUXADC_AD_RQST0, AUXADC_RQST_CH2_MASK, AUXADC_RQST_CH2_SHIFT, 1);
                    re_request_flag = 1;
                }
                if (channel_status > AUXADC_CHECK_MAX_TIMES) { //Request fail
                    request_state = PMU_STATUS_ERROR;
                    return request_state;
                }
                channel_status++;
                auxadc_status_index = AUXADC_CH2_BASE + channel_status;
            } while (pmu_get_register_value_hp(PMU_AUXADC_AD_ADC2, AUXADC_ADC_RDY_CH2_MASK, AUXADC_ADC_RDY_CH2_SHIFT) != 1);
            request_state = PMU_STATUS_SUCCESS;
            break;

        case 3:
            pmu_set_register_value_hp(PMU_AUXADC_AD_RQST0, AUXADC_RQST_CH3_MASK, AUXADC_RQST_CH3_SHIFT, 1);
            do {
                if (channel_status > AUXADC_CHECK_TIMES) {  //Re-request
                    pmu_set_register_value_hp(PMU_AUXADC_AD_RQST0, AUXADC_RQST_CH3_MASK, AUXADC_RQST_CH3_SHIFT, 1);
                    re_request_flag = 1;
                }
                if (channel_status > AUXADC_CHECK_MAX_TIMES) { //Request fail
                    request_state = PMU_STATUS_ERROR;
                    return request_state;
                }
                channel_status++;
                auxadc_status_index = AUXADC_CH3_BASE + channel_status;
            } while (pmu_get_register_value_hp(PMU_AUXADC_AD_ADC3, AUXADC_ADC_RDY_CH3_MASK, AUXADC_ADC_RDY_CH3_SHIFT) != 1);
            request_state = PMU_STATUS_SUCCESS;
            break;
        case 4:
            pmu_set_register_value_hp(PMU_AUXADC_AD_RQST0, AUXADC_RQST_CH4_MASK, AUXADC_RQST_CH4_SHIFT, 1);
            do {
                if (channel_status > AUXADC_CHECK_TIMES) {  //Re-request
                    pmu_set_register_value_hp(PMU_AUXADC_AD_RQST0, AUXADC_RQST_CH4_MASK, AUXADC_RQST_CH4_SHIFT, 1);
                    re_request_flag = 1;
                }
                if (channel_status > AUXADC_CHECK_MAX_TIMES) { //Request fail
                    request_state = PMU_STATUS_ERROR;
                    return request_state;
                }
                channel_status++;
                auxadc_status_index = AUXADC_CH4_BASE + channel_status;
            } while (pmu_get_register_value_hp(PMU_AUXADC_AD_ADC4, AUXADC_ADC_RDY_CH4_MASK, AUXADC_ADC_RDY_CH4_SHIFT) != 1);
            request_state = PMU_STATUS_SUCCESS;
            break;
        default:
            request_state = PMU_STATUS_INVALID_PARAMETER;
            break;
    }
    return request_state;
}
/*==========[Auxadc Transfer Algorithm]==========*/

uint32_t pmu_auxadc_voltage_transfer(int16_t value)
{
    uint32_t miniVoltage = -1;
    miniVoltage = (value * 1800) / 4096;
    return miniVoltage;
}

/*
Tcurrent = DEGC / 2 + (Ycurr - VBE_T)*1e-3 / {-[0.0016754 + (1 - 2 * O_SLOPE_SIGN)*O_SLOPE / 100000]}
Note: O_SLOPE is signed integer.
*/
int32_t pmu_auxadc_calculate_current_temperature(int16_t slope_value, int32_t vts, int32_t adc_Out, int8_t degcValue)
{
    log_hal_msgid_info("[PMU_ADC]slope[%d]vts[%d]Ch4[%d]degc[%d]\r\n", 5, slope_value, vts, adc_Out, degcValue);
    int32_t temp_value = 0;
    float temp1 = 0;
    float PmicTemp = 0;
    temp1 = (float)(1 / (-0.0016754) + (((1 - 2 * slope_value) * slope_value) * 100000));
    PmicTemp = (float)degcValue / 2 + ((((float)(adc_Out - vts) / 4096) * (float)1.8) * (float)temp1);
    if (PmicTemp < 0) {
        PmicTemp = (float)((PmicTemp * 100) - (float)0.5) / 100;
    } else {
        PmicTemp = (float)((PmicTemp * 100) + (float)0.5) / 100;
    }
    PmicTemp = (float)((PmicTemp * 100) + (float)0.5) / 100;
    temp_value = (int32_t)PmicTemp;
    return temp_value;
}

int16_t pmu_auxadc_get_slope_value(void)
{
    int16_t slope = 0;
    if (pmu_auxadc_get_slope_sign_value() == 1) {
        slope = ((int16_t) pmu_auxadc_get_efuse_slope_value()) * (-1);
    } else {
        slope = ((int16_t) pmu_auxadc_get_efuse_slope_value()) * (1);
    }
    return slope;
}

int8_t pmu_core_auxadc_get_degc_value(void)
{
    int8_t degc = 0;
    if (pmu_auxadc_get_degc_cali() < 38 || pmu_auxadc_get_degc_cali() > 60) {
        degc = 53;
    } else {
        degc = pmu_auxadc_get_degc_cali();
    }
    return degc;
}

int32_t pmu_get_pmic_temperature_hp(void)
{
    uint32_t adc_Out = 0;
    uint32_t vts_value = 0;
    if (!gAuxadcEfuseInfo.isInit) {
        log_hal_msgid_info("[PMU_ADC]Efuse Data Is Not Init\r\n", 0);
    }

    if (!pmu_auxadc_get_adc_cali_en()) {
        log_hal_msgid_info("[PMU_ADC]TempValue in Efuse is Not Calibration\r\n", 0);
    }
    pmu_set_register_value_hp(PMU_AUXADC_AD_CON12, RG_VBUF_BYP_MASK, RG_VBUF_BYP_SHIFT, 0);
    adc_Out = pmu_auxadc_get_channel_value(PMU_AUX_PMIC_AP);
    vts_value = pmu_auxadc_get_vts();
    pmu_set_register_value_hp(PMU_AUXADC_AD_CON12, RG_VBUF_BYP_MASK, RG_VBUF_BYP_SHIFT, 1);
    return pmu_auxadc_calculate_current_temperature(pmu_auxadc_get_slope_value(), (int32_t) vts_value, (int32_t) adc_Out, pmu_core_auxadc_get_degc_value());
}

bool pmu_core_efuse_rd_trigger(void)
{
    uint8_t rgTrigValue = 0xFF;
    rgTrigValue = pmu_get_register_value_hp(PMU_OTP_CON8, RG_OTP_RD_TRIG_MASK, RG_OTP_RD_TRIG_SHIFT);
    if (rgTrigValue == 0xFF) {
        log_hal_msgid_info("[PMU_ADC]Efuse RD Trig Failed\r\n", 0);
        return false;
    }
    if (rgTrigValue == 0) {
        pmu_set_register_value_hp(PMU_OTP_CON8, RG_OTP_RD_TRIG_MASK, RG_OTP_RD_TRIG_SHIFT, 1);
        return true;
    } else if (rgTrigValue == 1) {
        pmu_set_register_value_hp(PMU_OTP_CON8, RG_OTP_RD_TRIG_MASK, RG_OTP_RD_TRIG_SHIFT, 0);
        return true;
    } else {
        return false;
    }
}

uint16_t pmu_core_efuse_get_efuse_data_by_address(uint16_t efuseAddress)
{
    pmu_set_register_value_hp(PMU_OTP_CON0, RG_OTP_PA_MASK, RG_OTP_PA_SHIFT, efuseAddress);
    if (!pmu_core_efuse_rd_trigger()) {
        return 0;
    }
    hal_gpt_delay_ms(5);
    while (pmu_get_register_value_hp(PMU_OTP_CON13, RG_OTP_RD_BUSY_MASK, RG_OTP_RD_BUSY_SHIFT)) {
        log_hal_msgid_info("[PMU_ADC]Efuse Data Not Ready\r\n", 0);
    };
    return pmu_get_register_value_hp(PMU_OTP_CON12, RG_OTP_DOUT_SW_MASK, RG_OTP_DOUT_SW_SHIFT);
}

/*========[Auxadc info for Algorithm]========*/
uint8_t pmu_auxadc_get_slope_sign_value(void)
{
    return ((gAuxadcEfuseInfo.byte1 & AUXADC_EFUSE_O_SLOPE_SIGNED_IN_B1) >> 10);
}

uint8_t pmu_auxadc_get_efuse_slope_value(void)
{
    return ((gAuxadcEfuseInfo.byte1 & AUXADC_EFUSE_O_SLOPE_IN_B1) >> 4);
}

uint8_t pmu_auxadc_get_id(void)
{
    return ((gAuxadcEfuseInfo.byte1 & AUXADC_EFUSE_ID_IN_B1) >> 11);
}

uint16_t pmu_auxadc_get_vts(void)
{
    return ((gAuxadcEfuseInfo.byte0 & AUXADC_EFUSE_MASK_OVTS_IN_BO) >> 7) + ((gAuxadcEfuseInfo.byte1 & AUXADC_EFUSE_MASK_OVTS_IN_B1) << 9);
}

uint8_t pmu_auxadc_get_degc_cali(void)
{
    return (gAuxadcEfuseInfo.byte0 & AUXADC_EFUSE_MASK_DEGC_CALI_IN_B0);
}

uint8_t pmu_auxadc_get_adc_cali_en(void)
{
    return ((gAuxadcEfuseInfo.byte0 & AUXADC_EFUSE_MASK_ADC_CALI_EN_IN_B0) >> 6);
}

uint8_t pmu_core_auxadc_check_id(void)
{
    return ((gAuxadcEfuseInfo.byte1 & AUXADC_EFUSE_ID_IN_B1) >> 11);
}
#endif /* HAL_PMU_MODULE_ENABLED */

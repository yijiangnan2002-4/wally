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

#include "audio_set_driver.h"
#ifdef AIR_DAC_MODE_RUNTIME_CHANGE
#ifdef MTK_ANC_SURROUND_MONITOR_ENABLE
#include "anc_monitor.h"
#else
#ifdef MTK_ANC_ENABLE
#ifdef MTK_ANC_V2
#include "anc_control_api.h"
#else
#include "anc_control.h"
#endif
#endif
#endif
#include "hal_audio_internal.h"
#include "audio_anc_psap_control_internal.h"
#endif


#ifndef UNUSED
#define UNUSED(x)  ((void)(x))
#endif

#ifdef AIR_KEEP_I2S_ENABLE
void audio_driver_set_device(hal_audio_device_t set_device, hal_audio_interface_t set_interface, bool set_enable)
{
    bt_sink_srv_am_feature_t am_feature;
    memset(&am_feature, 0, sizeof(bt_sink_srv_am_feature_t));
    am_feature.type_mask = AM_AUDIO_DEVICE;
    am_feature.feature_param.audio_driver_param.enable = set_enable;
    am_feature.feature_param.audio_driver_param.audio_device = set_device;
    am_feature.feature_param.audio_driver_param.audio_interface= set_interface;
    am_audio_set_feature(FEATURE_NO_NEED_ID, &am_feature);
}

void hal_audio_set_audio_device(hal_audio_device_t set_device, hal_audio_interface_t set_interface, bool set_enable) {
    hal_audio_mclk_enable(true, AFE_MCLK_PIN_FROM_I2S0, AFE_APLL2, 0); //MCLK
    mcu2dsp_open_param_t open_param;
    void *p_param_share;
    memset(&open_param, 0, sizeof(mcu2dsp_open_param_t));
    //hal_audio_get_stream_in_setting_config(AU_DSP_AUDIO, &open_param.stream_in_param);
    //open_param.stream_in_param.afe.audio_device = ami_feature->feature_param.audio_driver_param.audio_device;
    //open_param.stream_in_param.afe.audio_interface = ami_feature->feature_param.audio_driver_param.audio_interface;
    hal_audio_get_stream_out_setting_config(AU_DSP_AUDIO, &open_param.stream_out_param);
    open_param.audio_scenario_type = AUDIO_SCENARIO_TYPE_DEVICE;
    open_param.stream_out_param.afe.audio_device = set_device;
    open_param.stream_out_param.afe.audio_interface = set_interface;
    open_param.stream_out_param.afe.i2s_master_sampling_rate[0] = 48000; //default value

    p_param_share = hal_audio_dsp_controller_put_paramter(&open_param, sizeof(mcu2dsp_open_param_t), AUDIO_MESSAGE_TYPE_COMMON);
    if (set_enable) {
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_DEVICE, &open_param, set_enable);
    }
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SET_DRIVER_PARAM,
                                          AUDIO_DRIVER_SET_DEVICE | (set_enable<<15),
                                          (uint32_t)p_param_share,
                                          true);
    if (!set_enable) {
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_DEVICE, &open_param, set_enable);
    }
}
#endif

#if defined(MTK_AMP_DC_COMPENSATION_ENABLE)


static void sort(unsigned int *p, unsigned int entries)
{
    unsigned int i, j, swap;
    unsigned int swapped = 0;

    if (entries == 0) {
        return;
    }

    for (i = 0; i < (entries - 1); i++) {
        for (j = 0; j < (entries - 1) - i; j++) {
            if (p[j] > p[j + 1]) {
                swap = p[j];
                p[j] = p[j + 1];
                p[j + 1] = swap;
                swapped = 1;
            }
        }
        if (!swapped) {
            break;
        }
    }
}

#define ReadREG(_addr)          (*(volatile uint32_t *)(_addr))
#define AFE_READ(addr)          *((volatile uint32_t *)(addr))
#define AFE_WRITE(addr, val)    *((volatile uint32_t *)(addr)) = val
#define AFE_SET_REG(addr, val, msk)  AFE_WRITE((addr), ((AFE_READ(addr) & (~(msk))) | ((val) & (msk))))
#define setting_count       (100)
#define abandon_head_count  (5)
#define abandon_tail_count  (5)
#define shrinkvalue         (0.125893)

static uint16_t get_dc_compensation_value()
{
    bool SignBitL = 0, SignBitR = 0;
    uint8_t count = 0, loop = 4, TrimL = 0, TrimR = 0, FineTrimL = 0, FineTrimR = 0;
    int channel_sel[4] = {0x00000071, 0x00000074, 0x00000072, 0x00000073}; //Trimming buffer mux selection with trimming buffer gain 18dB
    int SUM_HPLP = 0, SUM_HPLN = 0, SUM_HPRP = 0, SUM_HPRN = 0;
    uint16_t total = 0;
#ifdef DC_COMPENSATION_PARA_ENABLE
    double fAVG_HPLP = 0, fAVG_HPLN = 0, fAVG_HPRP = 0, fAVG_HPRN = 0, fValueL = 0, fValueR = 0, TrimStep = 0.18, FineTrimStep = 0.05, fcount = 0;
#else
    double fAVG_HPLP = 0, fAVG_HPLN = 0, fAVG_HPRP = 0, fAVG_HPRN = 0, fValueL = 0, fValueR = 0, TrimStep = 0.15, FineTrimStep = 0.04, fcount = 0;
#endif
    unsigned int **data = (unsigned int **)pvPortMalloc(2 * sizeof(unsigned int *));
    if (data == NULL) {
        AUDIO_ASSERT(0 && "DC compensation error, pvPortMalloc **data failed.");
        return 1;
    }
    for (count = 0; count < 2; count++) {

        data[count] = pvPortMalloc(setting_count * sizeof(unsigned int));

        if (data[count] == NULL) {
            vPortFree(data);
            AUDIO_ASSERT(0 && "DC compensation error, pvPortMalloc data[count] failed.");
            return 1;
        }
    }

#if defined(AIR_BTA_IC_PREMIUM_G3)
        //Here Set Analog - HPTRIM
        AFE_WRITE(ABB_BASE + 0x17C, 0x00001461);
        AFE_WRITE(ABB_BASE + 0x118, 0x00000040);
        AFE_WRITE(ABB_BASE + 0x118, 0x00000070);
        //Here Set Analog - AuxADC
        AFE_WRITE(AUXADC_BASE + 0x78, 0x00000101);
        hal_gpt_delay_us(5);
        AFE_WRITE(AUXADC_BASE + 0x74, 0x00000001);
        hal_gpt_delay_us(45);
        AFE_WRITE(AUXADC_BASE + 0x8C, 0x00000001);
        AFE_WRITE(AUXADC_BASE + 0x88, 0x00000001);
        AFE_WRITE(AUXADC_BASE + 0x8, 0x00000100);
        AFE_WRITE(AUXADC_BASE + 0x8, 0x00000000);
#elif defined(AIR_BTA_IC_STEREO_HIGH_G3)
    //Here Set Analog - HPTRIM
    AFE_WRITE(ABB_BASE + 0x12C, 0x0000FFF4);
    AFE_WRITE(ABB_BASE + 0x118, 0x00000040);
    AFE_WRITE(ABB_BASE + 0x118, 0x00000070);
    //Here Set Analog - AuxADC
    AFE_WRITE(AUXADC_BASE + 0x78, 0x00000101);
    hal_gpt_delay_us(5);
    AFE_WRITE(AUXADC_BASE + 0x74, 0x00000001);
    hal_gpt_delay_us(45);
    AFE_WRITE(AUXADC_BASE + 0x8C, 0x00000001);
    AFE_WRITE(AUXADC_BASE + 0x88, 0x00000001);
    AFE_WRITE(AUXADC_BASE + 0x8, 0x00000100);
    AFE_WRITE(AUXADC_BASE + 0x8, 0x00000000);
#endif

#ifndef FPGA_ENV
    hal_adc_init();
#endif
    //Auxadc measure value, trimming buffer mux selection 0x71:LP, 0x74:LN, 0x72:RP, 0x73:RN
    for (count = 0; count < setting_count; count++) {
        for (loop = 0; loop < 2; loop++) {
#ifdef DC_COMPENSATION_PARA_ENABLE
            AFE_SET_REG(ABB_BASE + 0x118,  channel_sel[loop],  0xffffffff);
#else
            AFE_SET_REG(ABB_BASE + 0x218,  channel_sel[loop],  0xffffffff);
#endif
            ADC->AUXADC_CON1 = 0;
            //hal_gpt_delay_us(10);
            ADC->AUXADC_CON1 = (1 << 8);
            // Wait until the module status is idle
            while (ADC->AUXADC_CON3_UNION.AUXADC_CON3_CELLS.ADC_STAT & AUXADC_CON3_ADC_STA_MASK);
#ifdef DC_COMPENSATION_PARA_ENABLE
            //hal_gpt_delay_us(10);
            data[loop][count] = ReadREG(AUXADC_BASE + 0x30);
            //printf("Ldata[%d][%d]:0x%x RG(AUXADC_BASE+0x4):0x%x RG(ABB_BASE+0x12C):0x%x\r\n",loop,count,data[loop][count], ReadREG(AUXADC_BASE+0x4),ReadREG(ABB_BASE+0x12C));
#else
            data[loop][count] = ReadREG(0xA0170030);
#endif

        }
    }

    //Abandon critical value and calculate sum of ordered data to get average value
    for (loop = 0; loop < 2; loop++) {
        sort(&data[loop][0], setting_count);
        if (loop == 0) {
            for (count = 0 + abandon_head_count; count < setting_count - abandon_tail_count; count++) {
                SUM_HPLP = SUM_HPLP + data[loop][count];
            }
            fAVG_HPLP = (float)SUM_HPLP / (float)(setting_count - abandon_head_count - abandon_tail_count);
        }
        if (loop == 1) {
            for (count = 0 + abandon_head_count; count < setting_count - abandon_tail_count; count++) {
                SUM_HPLN = SUM_HPLN + data[loop][count];
            }
            fAVG_HPLN = (float)SUM_HPLN / (float)(setting_count - abandon_head_count - abandon_tail_count);
        }
    }

    //Auxadc measure value, trimming buffer mux selection 0x71:LP, 0x74:LN, 0x72:RP, 0x73:RN
    for (count = 0; count < setting_count; count++) {
        for (loop = 0; loop < 2; loop++) {
#ifdef DC_COMPENSATION_PARA_ENABLE
            AFE_SET_REG(ABB_BASE + 0x118,  channel_sel[loop + 2],  0xffffffff);
#else
            AFE_SET_REG(ABB_BASE + 0x218,  channel_sel[loop + 2],  0xffffffff);
#endif
            ADC->AUXADC_CON1 = 0;
            //hal_gpt_delay_us(10);
            ADC->AUXADC_CON1 = (1 << 8);
            // Wait until the module status is idle
            while (ADC->AUXADC_CON3_UNION.AUXADC_CON3_CELLS.ADC_STAT & AUXADC_CON3_ADC_STA_MASK);
#ifdef DC_COMPENSATION_PARA_ENABLE
            //hal_gpt_delay_us(10);
            data[loop][count] = ReadREG(AUXADC_BASE + 0x30);
            //printf("Rdata[%d][%d]:0x%x RG(AUXADC_BASE+0x4):0x%x RG(ABB_BASE+12C):0x%x\r\n",loop,count,data[loop][count], ReadREG(AUXADC_BASE+0x4),ReadREG(ABB_BASE+0x12C));
#else
            data[loop][count] = ReadREG(0xA0170030);
#endif


        }
    }

    //Abandon critical value and calculate sum of ordered data to get average value
    for (loop = 0; loop < 2; loop++) {
        sort(&data[loop][0], setting_count);
        if (loop == 0) {
            for (count = 0 + abandon_head_count; count < setting_count - abandon_tail_count; count++) {
                SUM_HPRP = SUM_HPRP + data[loop][count];
            }
            fAVG_HPRP = (float)SUM_HPRP / (float)(setting_count - abandon_head_count - abandon_tail_count);
        }
        if (loop == 1) {
            for (count = 0 + abandon_head_count; count < setting_count - abandon_tail_count; count++) {
                SUM_HPRN = SUM_HPRN + data[loop][count];
            }
            fAVG_HPRN = (float)SUM_HPRN / (float)(setting_count - abandon_head_count - abandon_tail_count);
        }
    }
#ifndef FPGA_ENV
    hal_adc_deinit();
#endif
    for (count = 0; count < 2; count++) {
        vPortFree(data[count]);
    }
    vPortFree(data);

    fValueL = fAVG_HPLP - fAVG_HPLN;
    fValueR = fAVG_HPRP - fAVG_HPRN;

    //Auxadc reference voltage from 0~1.4V, express with 12bit
    fValueL = 1400 * fValueL / 4096; // mV (w 18dB)
    fValueR = 1400 * fValueR / 4096; // mV (w 18dB)

    //Without trimming buffer gain 18dB
    fValueL = fValueL * shrinkvalue; // mV (wo 18dB)
    fValueR = fValueR * shrinkvalue; // mV (wo 18dB)

    //printf("fValueL %f fValueR %f fAVG_HPLP %f fAVG_HPLN %f fAVG_HPRP %f fAVG_HPRN %f\r\n",fValueL,fValueR,fAVG_HPLP,fAVG_HPLN,fAVG_HPRP,fAVG_HPRN);
    //printf("fValueL = LP - LN = %f - %f = %f\r\n",fAVG_HPLP,fAVG_HPLN,fValueL);
    //printf("fValueR = RP - RN = %f - %f = %f\r\n",fAVG_HPRP,fAVG_HPRN,fValueR);
    //audio_src_srv_report("fValueL = LP - LN = %f - %f = %f", 3,fAVG_HPLP,fAVG_HPLN,fValueL);
    //audio_src_srv_report("fValueR = RP - RN = %f - %f = %f", 3,fAVG_HPRP,fAVG_HPRN,fValueR);

    SignBitL = (fValueL > 0) ? 1 : 0;
    SignBitR = (fValueR > 0) ? 1 : 0;

    if (SignBitL == 1) {
        for (fcount = fValueL; fcount >= TrimStep; fcount -= TrimStep) {
            fValueL -= TrimStep;
            TrimL += 1;
        }
        for (fcount = fValueL; fcount >= FineTrimStep; fcount -= FineTrimStep) {
            fValueL -= FineTrimStep;
            FineTrimL += 1;
        }
    } else {
        for (fcount = fValueL; fcount <= (-TrimStep); fcount += TrimStep) {
            fValueL += TrimStep;
            TrimL += 1;
        }
        for (fcount = fValueL; fcount <= (-FineTrimStep); fcount += FineTrimStep) {
            fValueL += FineTrimStep;
            FineTrimL += 1;
        }
    }

    if (SignBitR == 1) {
        for (fcount = fValueR; fcount >= TrimStep; fcount -= TrimStep) {
            fValueR -= TrimStep;
            TrimR += 1;
        }
        for (fcount = fValueR; fcount >= FineTrimStep; fcount -= FineTrimStep) {
            fValueR -= FineTrimStep;
            FineTrimR += 1;
        }
    } else {
        for (fcount = fValueR; fcount <= (-TrimStep); fcount += TrimStep) {
            fValueR += TrimStep;
            TrimR += 1;
        }
        for (fcount = fValueR; fcount <= (-FineTrimStep); fcount += FineTrimStep) {
            fValueR += FineTrimStep;
            FineTrimR += 1;
        }
    }

    total = SignBitR << 15 | FineTrimR << 13 | SignBitR << 12 | TrimR << 8 | SignBitL << 7 | FineTrimL << 5 | SignBitL << 4 | TrimL << 0;
    //printf("total 0x%x\r\n",total);

#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
    //Here Set Analog - AuxADC
    AFE_WRITE(AUXADC_BASE + 0x8, 0x00000100);
    AFE_WRITE(AUXADC_BASE + 0x8, 0x00000000);
    AFE_WRITE(AUXADC_BASE + 0x8C, 0x00000000);
    AFE_WRITE(AUXADC_BASE + 0x88, 0x00000000);
    hal_gpt_delay_us(45);
    AFE_WRITE(AUXADC_BASE + 0x74, 0x00000000);
    hal_gpt_delay_us(5);
    //Here Set Analog -HPTRIM & AuxADC
    AFE_WRITE(AUXADC_BASE + 0x78, 0x00000000);
    AFE_WRITE(ABB_BASE + 0x12C, 0x0000FFF0);
    AFE_WRITE(ABB_BASE + 0x118, 0x00000000);
#elif defined(AIR_BTA_IC_PREMIUM_G3)
        //Here Set Analog - AuxADC
        AFE_WRITE(AUXADC_BASE + 0x8, 0x00000100);
        AFE_WRITE(AUXADC_BASE + 0x8, 0x00000000);
        AFE_WRITE(AUXADC_BASE + 0x8C, 0x00000000);
        AFE_WRITE(AUXADC_BASE + 0x88, 0x00000000);
        AFE_WRITE(AUXADC_BASE + 0x74, 0x00000000);
        hal_gpt_delay_us(45);
        AFE_WRITE(AUXADC_BASE + 0x78, 0x00000100);
        AFE_WRITE(ABB_BASE + 0x400, 0x00000000);
        hal_gpt_delay_us(5);
        //Here Set Analog -HPTRIM & AuxADC
        AFE_WRITE(ABB_BASE + 0x118, 0x00000070);
        AFE_WRITE(ABB_BASE + 0x17C, 0x00001460);
        AFE_WRITE(ABB_BASE + 0x118, 0x00000000);
#endif

    return total;
}

void audio_driver_dc_compensation(void)
{
    bt_sink_srv_am_feature_t *feature_param = (bt_sink_srv_am_feature_t *)pvPortMalloc(sizeof(bt_sink_srv_am_feature_t));
    feature_param->type_mask = DC_COMPENSATION;
    am_audio_set_feature(FEATURE_NO_NEED_ID, feature_param);
    vPortFree(feature_param);
}

#if defined(AIR_DAC_MODE_RUNTIME_CHANGE)
extern HAL_DSP_PARA_AU_AFE_CTRL_t audio_nvdm_HW_config;
#endif

void hal_audio_set_dc_compensation(void) {
        audio_src_srv_report("[Sink][AM] Init: DC Compensation\n", 0);
        uint16_t dc_compensation_value = 0xffff;
        mcu2dsp_open_param_t open_param;
        void *p_param_share;
        memset(&open_param, 0, sizeof(mcu2dsp_open_param_t));
        hal_audio_get_stream_out_setting_config(AU_DSP_AUDIO, &open_param.stream_out_param);
        open_param.param.stream_out = STREAM_OUT_AFE;
        open_param.stream_out_param.afe.audio_device = HAL_AUDIO_DEVICE_DAC_DUAL;
        open_param.stream_out_param.afe.sampling_rate = 48000;
#if defined(AIR_DAC_MODE_RUNTIME_CHANGE)
        open_param.stream_out_param.afe.CLD_align_gain = audio_nvdm_HW_config.adc_dac_config.ADDA_DAC_CLD_Gain_Compensation;
#endif
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_DC_COMPENSATION, &open_param, true);
        //get DAC Class G/Class AB type and send to DSP with start & stop msg
        p_param_share = hal_audio_dsp_controller_put_paramter(&open_param, sizeof(mcu2dsp_open_param_t), AUDIO_MESSAGE_TYPE_COMMON);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_DC_COMPENSATION_START, 0, (uint32_t)p_param_share, true);

        // Calculate dc compensation value
        dc_compensation_value = get_dc_compensation_value();
        // Close Amp and send dc compensation value to DSP
        memset(&open_param, 0, sizeof(mcu2dsp_open_param_t));
        hal_audio_get_stream_out_setting_config(AU_DSP_AUDIO, &open_param.stream_out_param);
#ifdef AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE
        //no delay flag for dual chip slave side
        open_param.stream_out_param.afe.misc_parms = true; // // just reuse this parameter to disable AMP Delay 2s
#endif
        p_param_share = hal_audio_dsp_controller_put_paramter(&open_param, sizeof(mcu2dsp_open_param_t), AUDIO_MESSAGE_TYPE_COMMON);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_DC_COMPENSATION_STOP, dc_compensation_value, (uint32_t)p_param_share, true);
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_DC_COMPENSATION, NULL, false);
        audio_src_srv_report("[Sink][AM] Init Finish: DC Compensation, cal value 0x%x\n", 1, dc_compensation_value);
        if (dc_compensation_value == 0) {
            audio_src_srv_err("[Sink][AM] Error Attention : please check the DC Compensation flow!", 0);
        }
}
#endif


#if defined(AIR_DAC_MODE_RUNTIME_CHANGE)
#define DAC_CONTROL_RULE        (2)
extern uint8_t default_dac_mode;
extern uint8_t ha_dac_mode_flag;
#ifndef MTK_ANC_SURROUND_MONITOR_ENABLE
void hal_audio_status_get_anc_type(uint8_t *enable, audio_anc_type_t *anc_type, uint32_t *misc1, uint32_t *misc2)
{
    uint8_t                            anc_enable;
    audio_anc_control_filter_id_t      anc_current_filter_id;
    audio_anc_control_type_t           anc_current_type;
    int16_t                            anc_runtime_gain;
    uint8_t                            support_hybrid_enable;
    audio_anc_control_misc_t           local_misc;
    audio_anc_type_t                   local_anc_type = 0; //0 for ANC, 1 for PT, 2 for Sidetone
    memset(&local_misc, 0, sizeof(audio_anc_control_misc_t));
    audio_anc_control_get_status(&anc_enable, &anc_current_filter_id, &anc_current_type, &anc_runtime_gain, &support_hybrid_enable, &local_misc);
    if (enable != NULL) {
        *enable = anc_enable;
    }

#ifdef AIR_ANC_V3
    if ((local_misc.extend_use_parameters != 0) && (anc_enable == 0)) {
        // Sidetone enable only
        local_anc_type = AUDIO_ANC_TYPE_SIDETONE;
    } else if ((local_misc.extend_use_parameters != 0) && (anc_enable != 0)) {
        if (anc_current_filter_id <= 4/*AUDIO_ANC_CONTROL_FILTER_ID_ANC_END*/) {
            // Sidetone enable & ANC enable
            local_anc_type = AUDIO_ANC_TYPE_ANC;
        } else {
            // Sidetone enable & PT enable
            local_anc_type = AUDIO_ANC_TYPE_PT;
        }
    } else if ((local_misc.extend_use_parameters == 0) && (anc_enable != 0)) {
        if (anc_current_filter_id <= 4/*AUDIO_ANC_CONTROL_FILTER_ID_ANC_END*/) {
            // ANC enable only
            local_anc_type = AUDIO_ANC_TYPE_ANC;
        } else {
            // PT enable only
            local_anc_type = AUDIO_ANC_TYPE_PT;
        }
    } else if ((local_misc.extend_use_parameters == 0) && (anc_enable == 0)) {
        // all off
    }
#else
    if ((anc_current_filter_id == AUDIO_ANC_CONTROL_PASS_THRU_SIDETONE_FILTER_DEFAULT) && (anc_enable != 0)) {
        // Sidetone enable
        //V2 don't have pure sidetone
        if ((anc_current_type == AUDIO_ANC_CONTROL_TYPE_PASSTHRU_FF)||(anc_current_type == AUDIO_ANC_CONTROL_TYPE_PT_HYBRID)||(anc_current_type == AUDIO_ANC_CONTROL_TYPE_PT_FB)) {
            local_anc_type = AUDIO_ANC_TYPE_PT;
        } else {
            local_anc_type = AUDIO_ANC_TYPE_ANC;
        }
    } else if (anc_enable != 0) {
        if (anc_current_filter_id <= 4/*AUDIO_ANC_CONTROL_FILTER_ID_ANC_END*/) {
            // ANC enable only
            local_anc_type = AUDIO_ANC_TYPE_ANC;
        } else {
            // PT enable only
            local_anc_type = AUDIO_ANC_TYPE_PT;
        }
    } else if (anc_enable == 0) {
        // all off
    }
#endif
    #ifdef AIR_ANC_ADAP_PT_ENABLE //special case for ADAP_PT
    //anc_current_type = anc_get_official_type_from_internal_type(anc_current_type);
    if ((anc_enable != 0) && ((anc_current_type & 0xFFFF0000) == AUDIO_ANC_CONTROL_TYPE_PT_ADAP)) {
        local_anc_type = AUDIO_ANC_TYPE_PT;
    }
    #endif

    #ifdef AIR_HW_VIVID_PT_ENABLE
    if ((anc_enable != 0) && ((anc_current_type & 0xFFFF0000) == AUDIO_ANC_CONTROL_TYPE_PT_HW_VIVID)) {
        //Enable HW vivid PT
        local_anc_type = AUDIO_ANC_TYPE_HW_VIVID_PT;
    }
    #endif

    if ((anc_enable != 0) && ((anc_current_type & 0xFFFF0000) == AUDIO_ANC_CONTROL_TYPE_PT_HA_PSAP)) {
        //Enable PSAP/HA
        local_anc_type = AUDIO_ANC_TYPE_HA_PSAP;
    }

    if ((anc_enable != 0) && ((anc_current_type & 0xFFFF0000) == AUDIO_ANC_CONTROL_TYPE_PT_SW_VIVID)) {
        //Enable SW vivid PT
        local_anc_type = AUDIO_ANC_TYPE_SW_VIVID_PT;
    }

    if (anc_type != NULL) {
        *anc_type = local_anc_type;
    }
}
#endif

void hal_audio_status_send_update_dac_mode_event_to_am(hal_audio_ha_dac_flag_t ha_dac_flag,  bool enable)
{
    //send to AM
    bt_sink_srv_am_feature_t feature_para;
    memset(&feature_para, 0, sizeof(bt_sink_srv_am_feature_t));
    feature_para.type_mask = AM_AUDIO_UPDATE_DAC_MODE;
    feature_para.feature_param.dac_mode_update_param.event_type = ha_dac_flag;
    feature_para.feature_param.dac_mode_update_param.param = enable;
    audio_src_srv_report("[AMI][DAC Change Mode] send am event type:0x%x , param:0x%x", 2, feature_para.feature_param.dac_mode_update_param.event_type,feature_para.feature_param.dac_mode_update_param.param);
    am_audio_set_feature(FEATURE_NO_NEED_ID,&feature_para);
}

void hal_audio_status_update_dac_mode(void)
{
    uint8_t tar_mode;
    uint8_t ha_dl_enable;
#if defined(MTK_ANC_SURROUND_MONITOR_ENABLE)
    audio_anc_monitor_scenario_type_t anc_type;
#else
    audio_anc_type_t anc_type;
#endif

    if(default_dac_mode == HAL_AUDIO_DAC_MODE_OLCLASSD) {
#if (DAC_CONTROL_RULE == 0)
        tar_mode = HAL_AUDIO_DAC_MODE_CLASSD;

        /* Confirm not the hearing test mode */
        if(!(ha_dac_mode_flag & HAL_AUDIO_HA_DAC_FLAG_HEARING_TEST)) {
            tar_mode = default_dac_mode;// open loop
        }

#elif (DAC_CONTROL_RULE == 1)
        /* phase 2.0 */
        tar_mode = HAL_AUDIO_DAC_MODE_CLASSD;

#if defined(MTK_ANC_SURROUND_MONITOR_ENABLE)
        audio_anc_monitor_get_anc_status(&ha_dl_enable, &anc_type, NULL, NULL);
        audio_anc_psap_control_get_switch_status(&ha_dl_enable);
        /* Confirm ha exist */
        if( hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_ANC) && (anc_type == AUDIO_ANC_MONI_SCENARIO_TYPE_HA_PSAP) ){
#else
        hal_audio_status_get_anc_type(&ha_dl_enable, &anc_type, NULL, NULL);
        audio_anc_psap_control_get_switch_status(&ha_dl_enable);
        /* Confirm ha exist */
        if( hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_ANC) && (anc_type == AUDIO_ANC_TYPE_HA_PSAP) ){
#endif
            /* Confirm not the hearing test mode */
            if(!(ha_dac_mode_flag & HAL_AUDIO_HA_DAC_FLAG_HEARING_TEST)) {
                /* Confirm not the ha dl have sound */
                if(ha_dl_enable){
                    tar_mode = default_dac_mode;// open loop
                }
            }
        }

        audio_src_srv_report("[AM][DAC Change Mode]update dac mode. anc_runing_flag:%x , anc_type:%x, ha_dac_mode_flag:%x, ha_dl_enable:%x", 4,
                hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_ANC), anc_type, ha_dac_mode_flag, ha_dl_enable);
#else
        /* phase 2.1 */
        tar_mode = HAL_AUDIO_DAC_MODE_CLASSD;

        bool hearing_test_on    = (ha_dac_mode_flag & HAL_AUDIO_HA_DAC_FLAG_HEARING_TEST) ? 1 : 0;
        bool a2dp_mix_mode_on   = (ha_dac_mode_flag & HAL_AUDIO_HA_DAC_FLAG_A2DP_MIX_MODE) ? 1 : 0;
        bool sco_mix_mode_on    = (ha_dac_mode_flag & HAL_AUDIO_HA_DAC_FLAG_SCO_MIX_MODE) ? 1 : 0;

#if defined(MTK_ANC_SURROUND_MONITOR_ENABLE)
        audio_anc_monitor_get_anc_status(&ha_dl_enable, &anc_type, NULL, NULL);
        if( hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_ANC) && (anc_type == AUDIO_ANC_MONI_SCENARIO_TYPE_HA_PSAP) ){

#else
        hal_audio_status_get_anc_type(&ha_dl_enable, &anc_type, NULL, NULL);
        if( hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_ANC) && (anc_type == AUDIO_ANC_TYPE_HA_PSAP) ){

#endif
            /* Confirm not the hearing test mode */
            if(!hearing_test_on) {
                /* Confirm not the ha dl have sound */
#if (defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_ULL_BLE_HEADSET_ENABLE))
                if(((hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_A2DP) || (hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_BLE_DL) && !(g_prCurrent_player->local_context.ble_format.ble_codec.context_type & AUDIO_CONTENT_TYPE_CONVERSATIONAL))) && !a2dp_mix_mode_on) ||
                    ((hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_HFP_DL) || (hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_BLE_DL ) && (g_prCurrent_player->local_context.ble_format.ble_codec.context_type & AUDIO_CONTENT_TYPE_CONVERSATIONAL)))  && !sco_mix_mode_on)){
#else
                if((hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_A2DP) && !a2dp_mix_mode_on) ||
                    (hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_HFP_DL) && !sco_mix_mode_on)){
#endif
                     /*do nothing. this case we need dac mode be close loop class d */
                } else {
                    tar_mode = default_dac_mode;// open loop
                }
            }
        }
//        audio_src_srv_report("[AM][DAC Change Mode]update dac mode. anc_runing_flag:%x, anc_type:%x, hearing_test:%d, a2dp_mix:%d, sco_mix:%d", 5,
//                                    hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_ANC), anc_type, hearing_test_on, a2dp_mix_mode_on, sco_mix_mode_on);
#endif
        if(hal_audio_status_query_running_flag_except(AUDIO_SCENARIO_TYPE_AMP)){
            audio_src_srv_report("[AM][DAC Change Mode]update dac mode. anc_runing_flag:%x, anc_type:%x, hearing_test:%d, a2dp_mix:%d, sco_mix:%d", 5,
                                        hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_ANC), anc_type, hearing_test_on, a2dp_mix_mode_on, sco_mix_mode_on);
            hal_audio_status_change_dac_mode(tar_mode);
        }
    }

}

void hal_audio_status_set_ha_flag_and_update_dac_mode(hal_audio_ha_dac_flag_t ha_dac_flag,  bool enable)
{
    if(enable){
         ha_dac_mode_flag |= ha_dac_flag;
     } else {
         ha_dac_mode_flag &= ~(ha_dac_flag);
     }
     hal_audio_status_update_dac_mode();
    return;
}
#endif

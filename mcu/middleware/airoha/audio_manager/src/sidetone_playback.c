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

#include "sidetone_control.h"
#include "sidetone_playback.h"
#include "fixrate_control.h"

extern volatile uint32_t g_am_task_mask;
extern HAL_DSP_PARA_AU_AFE_CTRL_t audio_nvdm_HW_config;
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
extern bt_media_handle_t *g_prUllBleUl_media_handle;
#endif
extern bt_media_handle_t *g_prHfp_media_handle;

extern int g_side_tone_gain_common;
extern int g_side_tone_gain_hfp;
uint8_t *pSidetone_FLT_Buf = NULL;

extern sidetone_control_t g_sidetone_control;


/**************************************************************************
                                    Const table
***************************************************************************/

// zhong Yi default
/*const afe_sidetone_biquad_coef_t g_low_band_coef[10] = {
    {0x14F707CD,0xF04E11A8,0x105EEBF2,0x885F2E81,0x74868185},
    {0x7FFFFFFF,0x988325E8,0x58703556,0xC938790E,0x2C9FC4B2},
    {0x7FFFFFFF,0x805F282C,0x7F41DCFA,0x801E21C6,0x7FC3C0FE},
    {0x7FFFFFFF,0x99D63635,0x5342FCC1,0xC6492502,0x21AB79D6},
    {0x7FFFFFFF,0xC91F2D2D,0x17873CC6,0x00471AD4,0x00002780},
    {0x7FFFFFFF,0x81DE4C3F,0x7C789C4F,0x80D0272E,0x7E955865},
    {0x7FFFFFFF,0xE6F0D575,0x41A498F9,0xE3F35526,0x58BFEFCB},
    {0x7FFFFFFF,0xD01C4F49,0x433D25A8,0xE39F497D,0x3EE73335},
    {0x7FFFFFFF,0x85C4E524,0x7478FE2A,0x8575BAB4,0x7517072B},
    {0x7FFFFFFF,0x00000000,0x80000000,0x803CD3E5,0x7F8D2058}
};
*/

/* Default biqaud coef table. It can handle 16/32/48/96/192 kHz */
const afe_sidetone_biquad_coef_t g_sidetone_biqaud_default_coef[10] = {
    {0x7FFFFFFF,0x0,0x0,0x0,0x0},
    {0x7FFFFFFF,0x0,0x0,0x0,0x0},
    {0x7FFFFFFF,0x0,0x0,0x0,0x0},
    {0x7FFFFFFF,0x0,0x0,0x0,0x0},
    {0x7FFFFFFF,0x0,0x0,0x0,0x0},
    {0x7FFFFFFF,0x0,0x0,0x0,0x0},
    {0x7FFFFFFF,0x0,0x0,0x0,0x0},
    {0x7FFFFFFF,0x0,0x0,0x0,0x0},
    {0x7FFFFFFF,0x0,0x0,0x0,0x0},
    {0x7FFFFFFF,0x0,0x0,0x0,0x0}
};




/**************************************************************************
                                    Local function
***************************************************************************/



extern void aud_dl_resume(void);
extern void aud_dl_suspend(void);
extern void am_set_HFP_volume(bt_hfp_audio_codec_type_t type, bt_sink_srv_am_device_set_t in_audio_device, bt_sink_srv_am_volume_level_in_t in_audio_volume,
                             bt_sink_srv_am_device_set_t out_audio_device, bt_sink_srv_am_volume_level_out_t out_audio_volume, bool set_in_vol, bool set_out_vol);


hal_audio_device_t aud_sidetone_mic_convert_input_device(afe_sidetone_mic_sel_t mic)
{
    hal_audio_device_t input_device = HAL_AUDIO_DEVICE_NONE;
    switch (mic) {
        case AFE_SIDETONE_AMIC0_L:
        case AFE_SIDETONE_AMIC1_L:
        case AFE_SIDETONE_AMIC2_L:
            input_device = HAL_AUDIO_DEVICE_MAIN_MIC_L;
            break;
        case AFE_SIDETONE_AMIC0_R:
        case AFE_SIDETONE_AMIC1_R:
        case AFE_SIDETONE_AMIC2_R:
            input_device = HAL_AUDIO_DEVICE_MAIN_MIC_R;
            break;
        case AFE_SIDETONE_DMIC0_L:
        case AFE_SIDETONE_DMIC1_L:
        case AFE_SIDETONE_DMIC2_L:
            input_device = HAL_AUDIO_DEVICE_DIGITAL_MIC_L;
            break;
        case AFE_SIDETONE_DMIC0_R:
        case AFE_SIDETONE_DMIC1_R:
        case AFE_SIDETONE_DMIC2_R:
            input_device = HAL_AUDIO_DEVICE_DIGITAL_MIC_R;
            break;
        default:
            break;
    }
    return input_device;
}

hal_audio_interface_t aud_sidetone_mic_convert_input_interface(afe_sidetone_mic_sel_t mic)
{
    hal_audio_interface_t input_interface = HAL_AUDIO_INTERFACE_NONE;
    switch (mic) {
        case AFE_SIDETONE_AMIC0_L:
        case AFE_SIDETONE_AMIC0_R:
        case AFE_SIDETONE_DMIC0_L:
        case AFE_SIDETONE_DMIC0_R:
            input_interface = HAL_AUDIO_INTERFACE_1;
            break;
        case AFE_SIDETONE_AMIC1_L:
        case AFE_SIDETONE_AMIC1_R:
        case AFE_SIDETONE_DMIC1_L:
        case AFE_SIDETONE_DMIC1_R:
            input_interface = HAL_AUDIO_INTERFACE_2;
            break;
        case AFE_SIDETONE_AMIC2_L:
        case AFE_SIDETONE_AMIC2_R:
        case AFE_SIDETONE_DMIC2_L:
        case AFE_SIDETONE_DMIC2_R:
            input_interface = HAL_AUDIO_INTERFACE_3;
            break;
        default:
            break;
    }
    return input_interface;
}


/*****************************************************************************
 * FUNCTION
 *  aud_side_tone_control
 * DESCRIPTION
 *  This function is used to contorl DL 1 suspend / resume when side tone enable / disable
 * PARAMETERS
 *  isEnable        [IN]
 * RETURNS
 *  void
 *****************************************************************************/
#if !defined(FIXED_SAMPLING_RATE_TO_48KHZ) && !defined(AIR_FIXED_DL_SAMPLING_RATE_TO_96KHZ)

static void aud_side_tone_control(bool isEnable)
{
    if (g_prCurrent_player && (g_prCurrent_player->type != NONE)) {

        if ((g_prCurrent_player->type == HFP) || (g_prCurrent_player->type == PCM) || (g_prCurrent_player->type == ULL_BLE_DL) || (g_prCurrent_player->type == BLE) ) {
        } else {
            if (isEnable) {
                aud_dl_suspend();
            } else {
                if (!(g_am_task_mask & AM_TASK_MASK_VP_HAPPENING)) {
                    aud_dl_resume();
                }
            }
        }
    }
}

#endif

sidetone_scenario_t g_sidetone_request_scenario;
uint16_t g_sidetone_user = 0;
static void audio_side_tone_callback(hal_audio_event_t event, void *data)
{
    if (event == HAL_AUDIO_EVENT_END) {
        /*SideTone Stop ack.*/
        ami_set_audio_mask(AM_TASK_MASK_SIDE_TONE_ENABLE, false);
        ami_set_audio_mask(AM_TASK_MASK_SIDE_TONE_WAITING_STOP, false);
#if defined(MTK_EXTERNAL_DSP_NEED_SUPPORT)
        ami_set_afe_param(STREAM_OUT, HAL_AUDIO_SAMPLING_RATE_16KHZ, false);
#endif
#if !defined(FIXED_SAMPLING_RATE_TO_48KHZ) && !defined(AIR_FIXED_DL_SAMPLING_RATE_TO_96KHZ)
        aud_side_tone_control(false);
#endif
        if(ami_hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_SIDETONE)){
            ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_SIDETONE, NULL, false);
        }

#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
        ami_execute_vendor_se(EVENT_SIDETONE_STOP);
#endif

        if (g_am_task_mask & AM_TASK_MASK_SIDE_TONE_REQUEST) {
            bt_sink_srv_am_amm_struct am_amm;
            am_amm.background_info.local_feature.feature_param.sidetone_param.scenario = g_sidetone_request_scenario;
            audio_side_tone_enable_hdlr(&am_amm);
            ami_set_audio_mask(AM_TASK_MASK_SIDE_TONE_REQUEST, false);
        }

    }
}

void audio_side_tone_enable_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
#if defined(MTK_AVM_DIRECT)
    mcu2dsp_sidetone_param_t *sidetone;
    void *p_param_share;
    static mcu2dsp_open_stream_in_param_t *in_device;
    static mcu2dsp_open_stream_out_param_t *out_device;
    mcu2dsp_open_param_t *open_param;

    open_param = (mcu2dsp_open_param_t *)pvPortMalloc(sizeof(mcu2dsp_open_param_t));
    in_device = (mcu2dsp_open_stream_in_param_t *)pvPortMalloc(sizeof(mcu2dsp_open_stream_in_param_t));
    out_device = (mcu2dsp_open_stream_out_param_t *)pvPortMalloc(sizeof(mcu2dsp_open_stream_out_param_t));
    sidetone = (mcu2dsp_sidetone_param_t *)pvPortMalloc(sizeof(mcu2dsp_sidetone_param_t));

    if((open_param == NULL) || (in_device == NULL) || (out_device == NULL) || (sidetone == NULL)){
        AUDIO_ASSERT(0 && "[Sink][AM] audio_side_tone_enable_hdlr init malloc fail.");
    }

    memset(open_param, 0, sizeof(mcu2dsp_open_param_t));
    memset(in_device, 0, sizeof(mcu2dsp_open_stream_in_param_t));
    memset(out_device, 0, sizeof(mcu2dsp_open_stream_out_param_t));
    memset(sidetone, 0, sizeof(mcu2dsp_sidetone_param_t));
    sidetone_scenario_t scenario;
    if(amm_ptr){
        scenario = amm_ptr->background_info.local_feature.feature_param.sidetone_param.scenario;
    }else{
        scenario = SIDETONE_SCENARIO_COMMON;
    }

    audio_src_srv_report("[Sink][AM]audio_side_tone_enable_hdlr\n", 0);
    if ((g_am_task_mask & AM_TASK_MASK_VP_HAPPENING)
        || (g_am_task_mask & AM_TASK_MASK_SIDE_TONE_WAITING_STOP)) {
        g_sidetone_request_scenario = scenario;
        audio_src_srv_report("[Sink][AM]side tone enable Fail: VP happen first,scenario %d\n", 1, g_sidetone_request_scenario);
        ami_set_audio_mask(AM_TASK_MASK_SIDE_TONE_REQUEST, true);
    } else {
        g_sidetone_user = g_sidetone_user | (1<<scenario);
        audio_src_srv_report("[Sink][AM]audio_side_tone_enable_hdlr,user %d\n", 1, g_sidetone_user);
        if (!(g_am_task_mask & AM_TASK_MASK_SIDE_TONE_ENABLE)) {
#ifndef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
            if ((audio_nvdm_HW_config.Voice_Sidetone_EN == false) && (g_prHfp_media_handle != NULL)) {
#else
            //if ((audio_nvdm_HW_config.voice_scenario.Voice_Side_Tone_Enable == 0) && (g_prHfp_media_handle != NULL)) {
            if (audio_nvdm_HW_config.voice_scenario.Voice_Side_Tone_Enable == 0) {
#endif
                audio_src_srv_report("[Sink][AM]side tone enable Fail: HFP sidetone disable.\n", 0);
                vPortFree(open_param);
                vPortFree(in_device);
                vPortFree(out_device);
                vPortFree(sidetone);
                return;
            }
#ifdef MTK_VENDOR_SOUND_EFFECT_ENABLE
            ami_execute_vendor_se(EVENT_SIDETONE_START);
#endif
            ami_set_audio_mask(AM_TASK_MASK_SIDE_TONE_ENABLE, true);
#if !defined(FIXED_SAMPLING_RATE_TO_48KHZ) && !defined(AIR_FIXED_DL_SAMPLING_RATE_TO_96KHZ)
            aud_side_tone_control(true);
#endif
            hal_audio_service_hook_callback(AUDIO_MESSAGE_TYPE_SIDETONE, audio_side_tone_callback, NULL);
            hal_audio_get_stream_out_setting_config(AU_DSP_SIDETONE, out_device); /*Expect stream out channel is default*/
            hal_audio_get_stream_in_setting_config(AU_DSP_VOICE, in_device);   /*Sidetone.channel should be mic channel setting.*/

            /* Sidetone Mic selection */
            if(g_sidetone_control.sidetone_source_sel_en) {
                sidetone->in_device     = aud_sidetone_mic_convert_input_device((afe_sidetone_mic_sel_t)g_sidetone_control.sideTone_source_sel_0);
                sidetone->in_interface  = aud_sidetone_mic_convert_input_interface((afe_sidetone_mic_sel_t)g_sidetone_control.sideTone_source_sel_0);
            } else {
                sidetone->in_device     = in_device->afe.audio_device;
                sidetone->in_interface  = in_device->afe.audio_interface;
            }

            if(sidetone->in_interface == HAL_AUDIO_INTERFACE_1){
                sidetone->dmic_pin_sel = in_device->afe.dmic_selection[0];
            } else if (sidetone->in_interface == HAL_AUDIO_INTERFACE_2) {
                sidetone->dmic_pin_sel = in_device->afe.dmic_selection[2];
            } else if (sidetone->in_interface == HAL_AUDIO_INTERFACE_3){
                sidetone->dmic_pin_sel = in_device->afe.dmic_selection[4];
            } else {
                audio_src_srv_err("[audio_side_tone_enable_hdlr] dmic interface fail IF:%d.",1,sidetone->in_interface);
            }

            sidetone->in_channel        = in_device->afe.stream_channel;
            sidetone->in_misc_parms     = in_device->afe.misc_parms;
            sidetone->out_device        = out_device->afe.audio_device;
            sidetone->out_interface     = out_device->afe.audio_interface;
            sidetone->out_misc_parms    = out_device->afe.misc_parms;

#ifdef AIR_SIDETONE_CUSTOMIZE_ENABLE
            //audio_src_srv_report("[SIDETONE] AIR_SIDETONE_CUSTOMIZE_ENABLE",0);
            if (sidetone->in_device == HAL_AUDIO_DEVICE_MAIN_MIC_L && sidetone->in_interface == HAL_AUDIO_INTERFACE_1) {
                sidetone->ul_adc_mode = in_device->afe.ul_adc_mode[0];
                sidetone->iir_filter = in_device->afe.iir_filter[0];
            } else if (sidetone->in_device == HAL_AUDIO_DEVICE_MAIN_MIC_R && sidetone->in_interface == HAL_AUDIO_INTERFACE_1) {
                sidetone->ul_adc_mode = in_device->afe.ul_adc_mode[1];
                sidetone->iir_filter = in_device->afe.iir_filter[0];
            } else if (sidetone->in_device == HAL_AUDIO_DEVICE_MAIN_MIC_L && sidetone->in_interface == HAL_AUDIO_INTERFACE_2) {
                sidetone->ul_adc_mode = in_device->afe.ul_adc_mode[2];
                sidetone->iir_filter = in_device->afe.iir_filter[1];
            } else if (sidetone->in_device == HAL_AUDIO_DEVICE_MAIN_MIC_R && sidetone->in_interface == HAL_AUDIO_INTERFACE_2) {
                sidetone->ul_adc_mode = in_device->afe.ul_adc_mode[3];
                sidetone->iir_filter = in_device->afe.iir_filter[1];
            } else if (sidetone->in_device == HAL_AUDIO_DEVICE_MAIN_MIC_L && sidetone->in_interface == HAL_AUDIO_INTERFACE_3) {
                sidetone->ul_adc_mode = in_device->afe.ul_adc_mode[4];
                sidetone->iir_filter = in_device->afe.iir_filter[2];
            } else if (sidetone->in_device == HAL_AUDIO_DEVICE_MAIN_MIC_R && sidetone->in_interface == HAL_AUDIO_INTERFACE_3) {
                sidetone->ul_adc_mode = in_device->afe.ul_adc_mode[5];
                sidetone->iir_filter = in_device->afe.iir_filter[2];
            } else {
                sidetone->ul_adc_mode = in_device->afe.ul_adc_mode[0];
                sidetone->iir_filter = in_device->afe.iir_filter[0];
                audio_src_srv_report("[Sink][AM]side tone enable get wrong mic\n", 0);
            }
            sidetone->bias1_2_with_LDO0               = in_device->afe.bias1_2_with_LDO0;
            sidetone->in_device_sample_rate           = 16000;
            audio_src_srv_report("[SIDETONE] ul_adc_mode%d,iir_filter%d, bias1_2_with_LDO0%d, performance%d", 4, sidetone->ul_adc_mode, sidetone->iir_filter, sidetone->bias1_2_with_LDO0, sidetone->performance);
#endif

            sidetone->performance                     = in_device->afe.performance;

            if (aud_fixrate_get_downlink_rate(AUDIO_SCENARIO_TYPE_COMMON) == FIXRATE_NONE) {
#ifdef AIR_BT_BLE_SWB_ENABLE
                if(g_prCurrent_player != NULL && g_prCurrent_player->type == BLE){
                    sidetone->sample_rate                   = 32000;
                }else
#endif
                {
                    sidetone->sample_rate                     = 16000;
                }
#if AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
#ifndef AIR_ULL_AUDIO_V2_DONGLE_ENABLE
                if((g_prUllBleUl_media_handle != NULL) || (scenario == SIDETONE_SCENARIO_ULL_V2)){
                    sidetone->sample_rate                     = 96000;
                }
#endif
#endif
            } else {
                sidetone->sample_rate = aud_fixrate_get_downlink_rate(AUDIO_SCENARIO_TYPE_COMMON);
            }

#ifdef AIR_SIDETONE_VERIFY_ENABLE
            sidetone->sample_rate_in                  = audio_nvdm_HW_config.sidetone_config.fs_in * 1000;
            sidetone->sample_rate                     = audio_nvdm_HW_config.sidetone_config.fs_out * 1000;
#endif
            sidetone->on_delay_time = audio_nvdm_HW_config.sidetone_config.SideTone_On_Delay;
            audio_src_srv_report("[SIDETONE] Sidetone Sample Rate :%d",1 ,sidetone->sample_rate);

//            audio_src_srv_report("[SIDETONE] audio_side_tone_enable_hdlr on_delay_time %d",1,sidetone->on_delay_time);

            sysram_status_t nvdm_status;
            uint32_t nvkey_length = 0;//31*sizeof(uint16_t);
#if defined(BASE_STEREO_HIGH_G3_TYPE_77)
            uint32_t Coef_NVkey_ID;
            uint32_t default_coef = false;

            sidetone->filter_type  = am_audio_side_tone_query_method();
            if (sidetone->filter_type == HAL_AUDIO_SIDETONE_FLT_FIR ){
#endif
                nvdm_status = flash_memory_query_nvdm_data_length(NVID_DSP_ALG_SIDETONE_FIR_COEF, &nvkey_length);
                if (nvdm_status || !nvkey_length) {
                    audio_src_srv_report("[audio_side_tone_enable_hdlr] Read Nvkey length Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_SIDETONE_FIR_COEF, nvdm_status);
                }
                pSidetone_FLT_Buf = (uint8_t *)pvPortMallocNC(nvkey_length);
                if (pSidetone_FLT_Buf) {
                    nvdm_status = flash_memory_read_nvdm_data(NVID_DSP_ALG_SIDETONE_FIR_COEF, (uint8_t *)pSidetone_FLT_Buf, &nvkey_length);
                    if (nvdm_status || !nvkey_length) {
                        audio_src_srv_report("[audio_side_tone_enable_hdlr] Read Nvkey length Fail id:0x%x, status:%d ", 2, NVID_DSP_ALG_SIDETONE_FIR_COEF, nvdm_status);
                    }
                } else {
                    audio_src_srv_report("[audio_side_tone_enable_hdlr] fail to malloc pFILTERBuf", 0);
                }
#if defined(BASE_STEREO_HIGH_G3_TYPE_77)
            } else {    // HAL_AUDIO_SIDETONE_FLT_IIR
                if(sidetone->sample_rate == 16000) {
                    Coef_NVkey_ID = NVID_DSP_ALG_SIDETONE_IIR_COEF_16K;
                } else if(sidetone->sample_rate == 32000){
                    Coef_NVkey_ID = NVID_DSP_ALG_SIDETONE_IIR_COEF_32K;
                } else if(sidetone->sample_rate == 48000) {
                    Coef_NVkey_ID = NVID_DSP_ALG_SIDETONE_IIR_COEF_48K;
                } else if(sidetone->sample_rate == 96000) {
                    Coef_NVkey_ID = NVID_DSP_ALG_SIDETONE_IIR_COEF_96K;
                } else if(sidetone->sample_rate == 192000) {
                    Coef_NVkey_ID = NVID_DSP_ALG_SIDETONE_IIR_COEF_192K;
                } else{
                    sidetone->sample_rate = 16000;
                    Coef_NVkey_ID = NVID_DSP_ALG_SIDETONE_IIR_COEF_16K;
                    audio_src_srv_report("[audio_side_tone_enable_hdlr] undefined sample rate!",0);
                }

                nvdm_status = flash_memory_query_nvdm_data_length(Coef_NVkey_ID, &nvkey_length);
                if (nvdm_status || !nvkey_length) {
                    audio_src_srv_report("[audio_side_tone_enable_hdlr] Read Nvkey length Fail id:0x%x, status:%d ", 2, Coef_NVkey_ID, nvdm_status);
                    default_coef = true;
                }
                pSidetone_FLT_Buf = (uint8_t *)pvPortMallocNC(nvkey_length);
                if (pSidetone_FLT_Buf) {
                    nvdm_status = flash_memory_read_nvdm_data(Coef_NVkey_ID, (uint8_t *)pSidetone_FLT_Buf, &nvkey_length);
                    if (nvdm_status || !nvkey_length) {
                        audio_src_srv_report("[audio_side_tone_enable_hdlr] Read Nvkey Fail id:0x%x, status:%d ", 2, Coef_NVkey_ID, nvdm_status);
                        default_coef = true;
                    }
                } else {
                    audio_src_srv_report("[audio_side_tone_enable_hdlr] fail to malloc pFILTERBuf", 0);
                }

                if(default_coef) {
                    vPortFreeNC(pSidetone_FLT_Buf);
                    afe_sidetone_biquad_t* biquad_parem = (afe_sidetone_biquad_t *)pvPortMallocNC(sizeof(afe_sidetone_biquad_t));

                    biquad_parem->flag  = 0;
                    biquad_parem->band_num = SIDETONE_BIQUAD_BAND_NUM_MAX;
                    biquad_parem->biquad_out_gain = 0x8000;

                    memcpy(biquad_parem->biquad, g_sidetone_biqaud_default_coef, sizeof(afe_sidetone_biquad_coef_t) * biquad_parem->band_num);

                    pSidetone_FLT_Buf = (uint8_t *)biquad_parem;
                }
            }
#endif
#if defined(BASE_STEREO_HIGH_G3_TYPE_77)
            sidetone->FILTER_nvdm_param = (uint32_t *)pSidetone_FLT_Buf;
#else
            sidetone->FIR_nvdm_param = (uint16_t *)pSidetone_FLT_Buf;
#endif

//            sidetone->FILTER_nvdm_param = (uint32_t *)hal_memview_cm4_to_dsp0((uint32_t)pSidetone_FLT_Buf);


#ifndef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
            if (g_prHfp_media_handle != NULL) {
                //HFP sidetone gain
                if (audio_nvdm_HW_config.Voice_Sidetone_Gain >> 7) {
                    sidetone->gain   = (0xFFFFFF00 | audio_nvdm_HW_config.Voice_Sidetone_Gain) * 100;      /*Default: -100 /100 = -1dB*/
                } else {
                    sidetone->gain   = (audio_nvdm_HW_config.Voice_Sidetone_Gain) * 100;
                }
            } else {
                //Others sidetone gain
                if (audio_nvdm_HW_config.Reserve_Sidetone_Gain >> 7) {
                    sidetone->gain   = (0xFFFFFF00 | audio_nvdm_HW_config.Reserve_Sidetone_Gain) * 100;      /*Default: 800 /100 = 8dB change to -100 /100 = -1dB. Because record mic would sync analog gain to eSCO.*/
                } else {
                    sidetone->gain   = (audio_nvdm_HW_config.Reserve_Sidetone_Gain) * 100;
                }
            }
#else
            if (g_prHfp_media_handle != NULL) {
                //HFP sidetone gain
                if (audio_nvdm_HW_config.sidetone_config.SideTone_Gain >> 7) {
                    sidetone->gain   = (0xFFFFFF00 | audio_nvdm_HW_config.sidetone_config.SideTone_Gain) * 100;      /*Default: -100 /100 = -1dB*/
                } else {
                    sidetone->gain   = (audio_nvdm_HW_config.sidetone_config.SideTone_Gain) * 100;
                }

                if (g_side_tone_gain_hfp == SIDETONE_GAIN_MAGIC_NUM) {
                    g_side_tone_gain_hfp = ((int32_t)sidetone->gain) / 100;
                } else {
                    sidetone->gain = g_side_tone_gain_hfp * 100;
                    audio_src_srv_report("[Sink][AM]User defined side tone gain value for HFP: %d\n", 1, g_side_tone_gain_hfp);
                }
            } else {
                //Others sidetone gain,reserve config same as HFP sidetone gain val
                audio_nvdm_HW_config.sidetone_config.SideTone_Reserved[0] = audio_nvdm_HW_config.sidetone_config.SideTone_Gain;
                if (audio_nvdm_HW_config.sidetone_config.SideTone_Reserved[0] >> 7) {
                    sidetone->gain   = (0xFFFFFF00 | audio_nvdm_HW_config.sidetone_config.SideTone_Reserved[0]) * 100;      /*Default: 800 /100 = 8dB change to -100 /100 = -1dB. Because record mic would sync analog gain to eSCO.*/
                } else {
                    sidetone->gain   = (audio_nvdm_HW_config.sidetone_config.SideTone_Reserved[0]) * 100;
                }
                if (g_side_tone_gain_common == SIDETONE_GAIN_MAGIC_NUM) {
                    g_side_tone_gain_common = ((int32_t)sidetone->gain) / 100;
                } else {
                    sidetone->gain = g_side_tone_gain_common * 100;
                    audio_src_srv_report("[Sink][AM]User defined side tone gain value for none-HFP: %d\n", 1, g_side_tone_gain_common);
                }
            }
            sidetone->gain = ami_sidetone_adjust_gain_by_user_config(sidetone->gain);

            bt_sink_srv_audio_setting_vol_info_t *vol_info;
            vol_info = (bt_sink_srv_audio_setting_vol_info_t *)pvPortMalloc(sizeof(bt_sink_srv_audio_setting_vol_info_t));
            memset(vol_info, 0, sizeof(bt_sink_srv_audio_setting_vol_info_t));
            if (hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_RECORD) || hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_WWE) || (g_prHfp_media_handle != NULL)) {
                ;
            } else {
                vol_info->type = VOL_HFP;
                vol_info->vol_info.hfp_vol_info.codec = BT_HFP_CODEC_TYPE_MSBC;
                vol_info->vol_info.hfp_vol_info.dev_in = HAL_AUDIO_DEVICE_MAIN_MIC;
                vol_info->vol_info.hfp_vol_info.dev_out = HAL_AUDIO_DEVICE_HEADSET;
                vol_info->vol_info.hfp_vol_info.lev_in = 0;
                vol_info->vol_info.hfp_vol_info.lev_out = 0;
                extern void bt_sink_srv_am_set_volume(bt_sink_srv_am_stream_type_t in_out, bt_sink_srv_audio_setting_vol_info_t *vol_info);
                bt_sink_srv_am_set_volume(STREAM_IN, vol_info);
            }
#endif
#if defined(MTK_EXTERNAL_DSP_NEED_SUPPORT)
            ami_set_afe_param(STREAM_OUT, HAL_AUDIO_SAMPLING_RATE_16KHZ, true);
#endif
            p_param_share = hal_audio_dsp_controller_put_paramter(sidetone, sizeof(mcu2dsp_sidetone_param_t), AUDIO_MESSAGE_TYPE_SIDETONE);
            open_param->param.stream_in = STREAM_IN_AFE;
            open_param->stream_in_param.afe.audio_device = sidetone->in_device;
            open_param->stream_in_param.afe.bias_select = in_device->afe.bias_select;
            open_param->param.stream_out = STREAM_OUT_AFE;
            open_param->stream_out_param.afe.audio_device = sidetone->out_device;
            open_param->stream_out_param.afe.audio_interface = sidetone->out_interface;
            open_param->stream_out_param.afe.sampling_rate = sidetone->sample_rate;
            if(!ami_hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_SIDETONE)){
                ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_SIDETONE, open_param, true);
            }
            audio_src_srv_report("[Sink][AM]side tone enable:side tone gain = %d", 1, sidetone->gain / 100);
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SIDETONE_START, 0, (uint32_t)p_param_share, true);   //Sidetone start ack will ack first, and then begin ramp up.
            vPortFree(vol_info);
        } else {
            audio_src_srv_report("[Sink][AM]side tone enable Fail: Already enabled\n", 0);
        }
    }
    vPortFree(open_param);
    vPortFree(in_device);
    vPortFree(out_device);
    vPortFree(sidetone);
#endif
}

void audio_side_tone_set_volume_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
    uint32_t sidetone_gain = 0;
    sidetone_scenario_t scenario;

    sidetone_gain   = amm_ptr->background_info.local_feature.feature_param.sidetone_param.side_tone_gain;
    if(sidetone_gain == SIDETONE_GAIN_MAGIC_NUM) {
#ifndef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
        if(audio_nvdm_HW_config.Voice_Sidetone_Gain >> 7){
            sidetone_gain   = (0xFFFFFF00 | audio_nvdm_HW_config.Voice_Sidetone_Gain);      /*Default: -100 /100 = -1dB*/
        } else {
            sidetone_gain   = (audio_nvdm_HW_config.Voice_Sidetone_Gain);
        }
#else
        if (audio_nvdm_HW_config.sidetone_config.SideTone_Gain >> 7) {
            sidetone_gain   = (0xFFFFFF00 | audio_nvdm_HW_config.sidetone_config.SideTone_Gain);      /*Default: -100 /100 = -1dB*/
        } else {
            sidetone_gain   = (audio_nvdm_HW_config.sidetone_config.SideTone_Gain);
        }
#endif
    }
    scenario = amm_ptr->background_info.local_feature.feature_param.sidetone_param.scenario;
    audio_src_srv_report("[Sink][AM]side tone set volume:sidetone_gain = %d, scenario = %d", 2, sidetone_gain, (int)scenario);
    if (g_am_task_mask & AM_TASK_MASK_SIDE_TONE_ENABLE) {
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SIDETONE_SET_VOLUME, 0, sidetone_gain * 100, false);
    } else {
        audio_src_srv_err("[Sink][AM]side tone set volume error,side tone not enable!", 0);
    }
    //record the sidetone gain no matter the sidetone is enabled or not
    if (amm_ptr->background_info.local_feature.feature_param.sidetone_param.scenario == SIDETONE_SCENARIO_HFP) {
        g_side_tone_gain_hfp = sidetone_gain;
        g_side_tone_gain_common = g_side_tone_gain_hfp;//Temp, currently all the sidetone is set by type HFP
    } else {
        g_side_tone_gain_common = sidetone_gain;
    }
}

void audio_side_tone_disable_hdlr(bt_sink_srv_am_amm_struct *amm_ptr)
{
#if defined(MTK_AVM_DIRECT)
    mcu2dsp_sidetone_param_t *sidetone;
    void *p_param_share;
    mcu2dsp_open_stream_in_param_t *in_device;
    mcu2dsp_open_stream_out_param_t *out_device;

    in_device = (mcu2dsp_open_stream_in_param_t *)pvPortMalloc(sizeof(mcu2dsp_open_stream_in_param_t));
    out_device = (mcu2dsp_open_stream_out_param_t *)pvPortMalloc(sizeof(mcu2dsp_open_stream_out_param_t));
    sidetone = (mcu2dsp_sidetone_param_t *)pvPortMalloc(sizeof(mcu2dsp_sidetone_param_t));

    if((in_device == NULL) || (out_device == NULL) || (sidetone == NULL)){
        AUDIO_ASSERT(0 && "[Sink][AM] audio_side_tone_disable_hdlr init malloc fail.");
    }

    memset(in_device, 0, sizeof(mcu2dsp_open_stream_in_param_t));
    memset(out_device, 0, sizeof(mcu2dsp_open_stream_out_param_t));
    memset(sidetone, 0, sizeof(mcu2dsp_sidetone_param_t));
    sidetone_scenario_t scenario;

    if(amm_ptr){
        scenario = amm_ptr->background_info.local_feature.feature_param.sidetone_param.scenario;
    }else{
        scenario = SIDETONE_SCENARIO_COMMON;
    }
    g_sidetone_user = g_sidetone_user & (~(1<<scenario));
    audio_src_srv_report("[Sink][AM]audio_side_tone_disable_hdlr, user %d\n", 1, g_sidetone_user);
    ami_set_audio_mask(AM_TASK_MASK_SIDE_TONE_REQUEST, false);
    if ((g_am_task_mask & AM_TASK_MASK_SIDE_TONE_ENABLE)
        && (!(g_am_task_mask & AM_TASK_MASK_SIDE_TONE_WAITING_STOP))
        &&(!g_sidetone_user)) {
#if 0
        sidetone->out_device                      = HAL_AUDIO_DEVICE_DAC_DUAL;
        sidetone->out_interface                   = HAL_AUDIO_INTERFACE_NONE;
        sidetone->out_misc_parms                  = DOWNLINK_PERFORMANCE_NORMAL;
        sidetone->in_device                       = HAL_AUDIO_DEVICE_MAIN_MIC_DUAL;
        sidetone->in_interface                    = HAL_AUDIO_INTERFACE_1;
        sidetone->in_channel                      = HAL_AUDIO_DIRECT;
        sidetone->in_misc_parms                   = MICBIAS_SOURCE_ALL | MICBIAS3V_OUTVOLTAGE_1p85v;
        sidetone->sample_rate                     = 16000;
#else
        hal_audio_get_stream_out_setting_config(AU_DSP_SIDETONE, out_device); /*Expect stream out channel is default*/
        hal_audio_get_stream_in_setting_config(AU_DSP_VOICE, in_device);   /*Sidetone.channel should be mic channel setting.*/
        sidetone->out_device                      = out_device->afe.audio_device;
        sidetone->out_interface                   = out_device->afe.audio_interface;
        sidetone->out_misc_parms                  = out_device->afe.misc_parms;
        sidetone->in_device                       = in_device->afe.audio_device;
        sidetone->in_interface                    = in_device->afe.audio_interface;
        sidetone->in_channel                      = in_device->afe.stream_channel;
        sidetone->in_misc_parms                   = in_device->afe.misc_parms;
        if (aud_fixrate_get_downlink_rate(AUDIO_SCENARIO_TYPE_COMMON) == FIXRATE_NONE) {
            sidetone->sample_rate                     = 16000;
        } else {
            sidetone->sample_rate = aud_fixrate_get_downlink_rate(AUDIO_SCENARIO_TYPE_COMMON);
        }
#endif
        sidetone->gain           = 0;
        p_param_share = hal_audio_dsp_controller_put_paramter(sidetone, sizeof(mcu2dsp_sidetone_param_t), AUDIO_MESSAGE_TYPE_SIDETONE);
        audio_src_srv_report("[Sink][AM]side tone disable\n", 0);
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SIDETONE_STOP, 0, (uint32_t)p_param_share, false);
        //Not wait Sidetone stop ack, because sidetone ramp down would block AM task.
        ami_set_audio_mask(AM_TASK_MASK_SIDE_TONE_WAITING_STOP, true);
        vPortFreeNC(pSidetone_FLT_Buf);
#if 0
        ami_set_audio_mask(AM_TASK_MASK_SIDE_TONE_ENABLE, false);
        aud_side_tone_control(false);
        ami_hal_audio_status_set_running_flag(AUDIO_MESSAGE_TYPE_SIDETONE, false);
#endif
    } else {
        audio_src_srv_report("[Sink][AM]side tone disable Fail: Have not enabled\n", 0);
    }
    vPortFree(out_device);
    vPortFree(in_device);
    vPortFree(sidetone);
#endif
}

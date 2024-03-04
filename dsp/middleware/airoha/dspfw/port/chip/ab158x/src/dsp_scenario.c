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

/******************************************************************************
* Include
******************************************************************************/
#include "string.h"
#include "dsp_scenario.h"
#include "bt_interface.h"
#include "stream.h"
#ifdef AIR_BT_HFP_ENABLE
#include "stream_n9sco.h"
#endif
#include "stream_n9_a2dp.h"
#include "hal_audio_afe_define.h"
#include "hal_audio_afe_control.h"
#include "hal_sleep_manager.h"
#include "dsp_audio_msg_define.h"
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
#include "audio_transmitter_mcu_dsp_common.h"
#endif
#include "dsp_drv_afe.h"
#include "dllt.h"
#include "dsp_audio_ctrl.h"
#include "dsp_share_memory.h"
#include "dsp_temp.h"
#include "audio_nvdm_common.h"
#ifdef AIR_BT_CLK_SKEW_ENABLE
#include "clk_skew.h"
#include "long_term_clk_skew.h"
#endif
#ifdef AIR_VOICE_NR_ENABLE
#include "voice_nr_interface.h"
#endif
#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
#include "peq_interface.h"
#endif
#include "dsp_dump.h"
#include "sfr_bt.h"
#ifdef MTK_LEAKAGE_DETECTION_ENABLE
#include "leakage_compensation.h"
#endif
#ifdef MTK_WWE_ENABLE
#include "wwe_interface.h"
#endif
#ifdef MTK_SENSOR_SOURCE_ENABLE
#include "bsp_multi_axis_sensor.h"
#endif
#ifdef MTK_AUDIO_PLC_ENABLE
#include "audio_plc_interface.h"
#endif
#include "dsp_audio_msg_define.h"
#include "dsp_audio_process.h"
#ifdef MTK_SLT_AUDIO_HW
#include "dsp_audio_slt.h"
#endif
#if defined(AIR_DRC_ENABLE) || defined(AIR_VOICE_DRC_ENABLE)
#include "compander_interface.h"
#endif
#ifdef MTK_BT_A2DP_ENABLE
#define PLAY_EN_CHECK_TIMER
#endif /* MTK_BT_A2DP_ENABLE */

#define TRIGGER_A2DP_PROCSESS_TIMER
#if defined(PLAY_EN_CHECK_TIMER) || defined(TRIGGER_A2DP_PROCSESS_TIMER)
#include "timers.h"
#endif

#ifdef MTK_CELT_DEC_ENABLE
#include "celt_dec_interface.h"
#endif

#ifdef AIR_BT_CODEC_BLE_ENABLED
#include "stream_n9ble.h"
#include "common.h"
#include "hal_audio_message_struct_common.h"
#ifdef AIR_BT_LE_LC3_ENABLE
#include "lc3_dec_interface.h"
#include "lc3_enc_interface.h"
#include "lc3_codec_portable.h"
#endif
#include "dsp_memory_region.h"

#endif

#if defined(AIR_BT_LE_LC3PLUS_ENABLE) || defined(AIR_BT_A2DP_LC3PLUS_ENABLE)
#include "lc3plus_dec_interface.h"
#endif

#ifdef AIR_CELT_DEC_V2_ENABLE
#include "celt_dec_interface_v2.h"
#endif

#ifdef AIR_AUDIO_ULD_DECODE_ENABLE
#include "uld_dec_interface.h"
#endif

#ifdef AIR_AUDIO_ULD_ENCODE_ENABLE
#include "uld_enc_interface.h"
#endif

#ifdef MTK_LINEIN_INS_ENABLE
#include "ins_interface.h"
#endif

#ifdef MTK_AUDIO_LOOPBACK_TEST_ENABLE
#include "audio_loopback_test_interface.h"
#endif

#ifdef AIR_SOFTWARE_SRC_ENABLE
#include "sw_src_interface.h"
#endif
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
#include "clk_skew_sw.h"
#endif
#ifdef AIR_SOFTWARE_GAIN_ENABLE
#include "sw_gain_interface.h"
#endif
#ifdef AIR_ANC_WIND_DETECTION_ENABLE
#include "wind_interface.h"
#endif
#if defined(AIR_WIRED_AUDIO_ENABLE)
#include "scenario_wired_audio.h"
#endif
#ifdef MTK_ANC_ENABLE
#include "anc_api.h"
#endif
#ifdef MTK_USER_TRIGGER_FF_ENABLE
#include "user_trigger_adaptive_ff.h"
#endif
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
#include "user_unaware_interface.h"
#endif

#ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
#include "environment_detection_interface.h"
#endif
#include "hal_resource_assignment.h"
#ifdef AIR_ADVANCED_PASSTHROUGH_ENABLE
#include "scenario_advanced_passthrough.h"
#endif

#ifdef AIR_DCHS_MODE_ENABLE
#include "stream_dchs.h"
#include "hal_audio_driver.h"
#endif

#ifdef AIR_SILENCE_DETECTION_ENABLE
#include "silence_detection_interface.h"
#endif

#ifdef AIR_FULL_ADAPTIVE_ANC_ENABLE
#include "full_adapt_anc_api.h"
#endif

#ifdef AIR_HW_VIVID_PT_ENABLE
#include "hw_vivid_passthru_api.h"
#endif

#ifdef AIR_ADAPTIVE_EQ_ENABLE
#include "adaptive_eq_interface.h"
#endif

#if defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
#include "stream_llf.h"
#endif
#if defined(AIR_CUSTOMIZED_LLF_ENABLE)
#include "low_latency_framework_exsample.h"
#endif
#if defined(AIR_HEARING_AID_ENABLE)
#include "hearing_aid_interface.h"
#elif defined(AIR_HEARTHROUGH_PSAP_ENABLE)
#include "hearthrough_psap_interface.h"
#endif
#ifdef AIR_HEARTHROUGH_VIVID_PT_ENABLE
#include "vivid_passthru_interface.h"
#include "llf_vivid_pt.h"
#endif
#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
#include "llf_psap.h"
#endif

#ifdef AIR_SOFTWARE_DRC_ENABLE
#include "compander_interface_sw.h"
#endif
#if defined (AIR_DCHS_MODE_ENABLE)
#include "dsp_mux_uart.h"
#include "voice_nr_driver.h"
#include "stream_dchs.h"
#include "hal_gpt.h"
#endif

#if defined(AIR_FIXED_RATIO_SRC)
#include "src_fixed_ratio_interface.h"
#endif

#ifdef AIR_MIXER_STREAM_ENABLE
#include "stream_mixer.h"
#endif

/******************************************************************************
* Define
******************************************************************************/
#define eSCO_UT_FILL_SHAREBUF
// #define eSCO_UT_WETH_MEMORY
#define MTK_AUDIO_DSP_SYNC_REQUEST_DEBUG (0)

#define eSCO_UT_FILL_SHAREBUF
#define DSP_REMAP_SHARE_INFO(para,type)  ({  \
         para = (type)hal_memview_cm4_to_dsp0((uint32_t)para); \
         /*((n9_dsp_share_info_ptr) para)->startaddr = hal_memview_cm4_to_dsp0(((SHARE_BUFFER_INFO_PTR) para)->startaddr);*/})
#define abs32(x) ( (x >= 0) ? x : (-x) )

EXTERN audio_hardware afe_get_audio_hardware_by_au_afe_open_param(au_afe_open_param_t *afe_open);
EXTERN audio_instance afe_get_audio_instance_by_au_afe_open_param(au_afe_open_param_t *afe_open);
EXTERN audio_channel afe_get_audio_channel_by_au_afe_open_param(au_afe_open_param_t *afe_open);
EXTERN afe_t afe;
EXTERN hal_nvic_status_t hal_nvic_save_and_set_interrupt_mask_special(uint32_t *mask);
EXTERN hal_nvic_status_t hal_nvic_restore_interrupt_mask_special(uint32_t mask);
#ifdef AIR_MCU_DSP_DEPENDECY_CHECK_ENABLE
audio_clock_share_buffer_p audio_clock_param;
#endif

/******************************************************************************
* Global variable
******************************************************************************/
CONNECTION_IF playback_if, n9_a2dp_if, record_if, linein_playback_if;
#ifdef AIR_BT_CODEC_BLE_ENABLED
CONNECTION_IF n9_ble_ul_if, n9_ble_dl_if;
#endif
#ifdef MTK_PROMPT_SOUND_ENABLE
CONNECTION_IF playback_vp_if;
#endif
#ifdef MTK_PROMPT_SOUND_DUMMY_SOURCE_ENABLE
CONNECTION_IF playback_vp_dummy_source_if;
#endif
#ifdef MTK_SENSOR_SOURCE_ENABLE
CONNECTION_IF sensor_src_if[AUDIO_TRANSMITTER_GSENSOR_SUB_ID_MAX] = {
    /*source,      sink,  transform, pfeature_table;*/
    {NULL,      NULL,       NULL,     AudioFeatureList_GSensorMotionDetect_virtual},  //no need to send data to MCU
    {NULL,      NULL,       NULL,     AudioFeatureList_GSensorMotionDetect}, //send sensor data to MCU
    {NULL,      NULL,       NULL,     AudioFeatureList_GSensorMotionDetect_virtual},
};
#endif

#if defined(MTK_ANC_SURROUND_MONITOR_ENABLE)
CONNECTION_IF sensor_anc_monitor_if = {NULL,      NULL,       NULL,     stream_featuremulti_anc_monitor};
#endif

#if defined(AIR_ADAPTIVE_EQ_ENABLE)
CONNECTION_IF sensor_adaptive_eq_monitor_if = {NULL,      NULL,       NULL,     stream_featuremulti_adaptive_eq_monitor};
#endif
#ifdef AIR_DCHS_MODE_ENABLE
CONNECTION_IF dchs_uart_ul_left_if_wb = {NULL,      NULL,       NULL,     stream_feature_list_dchs_uart_ul_left_wb};
CONNECTION_IF dchs_uart_ul_left_if_swb = {NULL,      NULL,       NULL,     stream_feature_list_dchs_uart_ul_left_swb};
CONNECTION_IF dchs_uart_ul_left_if_wb_sw_gain = {NULL,      NULL,       NULL,     stream_feature_list_dchs_uart_ul_left_wb_sw_gain};
CONNECTION_IF dchs_uart_ul_left_if_swb_sw_gain = {NULL,      NULL,       NULL,     stream_feature_list_dchs_uart_ul_left_swb_sw_gain};
CONNECTION_IF dchs_uart_ul_right_if = {NULL,      NULL,       NULL,     stream_feature_list_dchs_uart_ul_right};
uint8_t dchs_voice_mode;
uint8_t dchs_ul_scenario_type;
#endif
#if defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
dsp_llf_feature_entry_t LLF_feature_entry_table[AUDIO_LLF_TYPE_ALL] = {
    /* feature_list_entry, init_entry, deinit_entry, suspend_entry, resume_entry, runtime_config_entry, set_dl_state_entry */
#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
    {psap_features_get_list, psap_features_init, psap_features_deinit, psap_suspend, psap_resume, psap_runtime_config, psap_set_dl_state,},//AUDIO_LLF_TYPE_HEARING_AID
#else
    {NULL, NULL, NULL, NULL, NULL, NULL, NULL,},//AUDIO_LLF_TYPE_HEARING_AID
#endif
#ifdef AIR_HEARTHROUGH_VIVID_PT_ENABLE
    {dsp_vivid_PT_features_get_list, dsp_vivid_PT_feature_init, NULL, dsp_vivid_PT_suspend, dsp_vivid_PT_resume, dsp_vivid_PT_runtime_config, NULL,},//AUDIO_LLF_TYPE_VIVID_PT
#else
    {NULL, NULL, NULL, NULL, NULL, NULL, NULL,},//AUDIO_LLF_TYPE_VIVID_PT
#endif
    {NULL, NULL, NULL, NULL, NULL, NULL, NULL,},//AUDIO_LLF_TYPE_RESERVED_1
    {NULL, NULL, NULL, NULL, NULL, NULL, NULL,},//AUDIO_LLF_TYPE_RESERVED_2
#if defined(AIR_CUSTOMIZED_LLF_ENABLE)
    {llf_example_features_get_list, NULL, NULL, NULL, NULL, NULL, NULL,},//AUDIO_LLF_TYPE_SAMPLE
#else
    {NULL, NULL, NULL, NULL, NULL, NULL, NULL,},//AUDIO_LLF_TYPE_SAMPLE
#endif
};
#endif
#if 0
void afe_print_reg(void)
{
    DSP_MW_LOG_I("AUDIO_TOP_CON0=%x\r\n", 1, AFE_READ(AUDIO_TOP_CON0));
    DSP_MW_LOG_I("AUDIO_TOP_CON1=%x\r\n", 1, AFE_READ(AUDIO_TOP_CON1));
    DSP_MW_LOG_I("AFE_DAC_CON0=%x\r\n", 1, AFE_READ(AFE_DAC_CON0));
    DSP_MW_LOG_I("AFE_DAC_CON1=%x\r\n", 1, AFE_READ(AFE_DAC_CON1));
    DSP_MW_LOG_I("AFE_DAC_CON2=%x\r\n", 1, AFE_READ(AFE_DAC_CON2));
    DSP_MW_LOG_I("AFE_ADDA_TOP_CON0=%x\r\n", 1, AFE_READ(AFE_ADDA_TOP_CON0));
    DSP_MW_LOG_I("AFE_ADDA_DL_SRC2_CON0=%x\r\n", 1, AFE_READ(AFE_ADDA_DL_SRC2_CON0));
    DSP_MW_LOG_I("AFE_ADDA_DL_SRC2_CON1=%x\r\n", 1, AFE_READ(AFE_ADDA_DL_SRC2_CON1));
    DSP_MW_LOG_I("AFE_ADDA_UL_DL_CON0=%x\r\n", 1, AFE_READ(AFE_ADDA_UL_DL_CON0));
    DSP_MW_LOG_I("AFE_ADDA_PREDIS_CON0=%x\r\n", 1, AFE_READ(AFE_ADDA_PREDIS_CON0));
    DSP_MW_LOG_I("AFE_ADDA_PREDIS_CON1=%x\r\n", 1, AFE_READ(AFE_ADDA_PREDIS_CON1));
    DSP_MW_LOG_I("AFE_CLASSG_LPSRCH_CFG0=%x\r\n", 1, AFE_READ(AFE_CLASSG_LPSRCH_CFG0));
    DSP_MW_LOG_I("ZCD_CON2=%x\r\n", 1, AFE_READ(ZCD_CON2));
    DSP_MW_LOG_I("ZCD_CON0=%x\r\n", 1, AFE_READ(ZCD_CON0));
    //DSP_MW_LOG_I("ZCD_CON1=%x\r\n", 1, AFE_READ(ZCD_CON1));
    DSP_MW_LOG_I("AUDDEC_ANA_CON0=%x\r\n", 1, ANA_READ(AUDDEC_ANA_CON0));
    DSP_MW_LOG_I("AUDDEC_ANA_CON1=%x\r\n", 1, ANA_READ(AUDDEC_ANA_CON1));
    DSP_MW_LOG_I("AUDDEC_ANA_CON2=%x\r\n", 1, ANA_READ(AUDDEC_ANA_CON2));
    DSP_MW_LOG_I("AUDDEC_ANA_CON3=%x\r\n", 1, ANA_READ(AUDDEC_ANA_CON3));
    DSP_MW_LOG_I("AUDDEC_ANA_CON4=%x\r\n", 1, ANA_READ(AUDDEC_ANA_CON4));
    DSP_MW_LOG_I("AUDDEC_ANA_CON5=%x\r\n", 1, ANA_READ(AUDDEC_ANA_CON5));
    DSP_MW_LOG_I("AUDDEC_ANA_CON6=%x\r\n", 1, ANA_READ(AUDDEC_ANA_CON6));
    DSP_MW_LOG_I("AUDDEC_ANA_CON7=%x\r\n", 1, ANA_READ(AUDDEC_ANA_CON7));
    DSP_MW_LOG_I("AUDDEC_ANA_CON8=%x\r\n", 1, ANA_READ(AUDDEC_ANA_CON8));
    DSP_MW_LOG_I("AUDDEC_ANA_CON9=%x\r\n", 1, ANA_READ(AUDDEC_ANA_CON9));
    DSP_MW_LOG_I("AUDDEC_ANA_CON10=%x\r\n", 1, ANA_READ(AUDDEC_ANA_CON10));
    DSP_MW_LOG_I("AUDDEC_ANA_CON11=%x\r\n", 1, ANA_READ(AUDDEC_ANA_CON11));
    DSP_MW_LOG_I("AUDDEC_ANA_CON12=%x\r\n", 1, ANA_READ(AUDDEC_ANA_CON12));
    DSP_MW_LOG_I("AUDDEC_ANA_CON13=%x\r\n", 1, ANA_READ(AUDDEC_ANA_CON13));
    DSP_MW_LOG_I("AUDDEC_ANA_CON14=%x\r\n", 1, ANA_READ(AUDDEC_ANA_CON14));
}
#endif
bool BleDlStopFlag;
bool ULL_NrOffloadFlag;
audio_dsp_ull_start_ctrl_param_t audio_headset_ull_ctrl;
#define ULL_BASIC_LATENCY 20000

#ifdef AIR_3RD_PARTY_AUDIO_PLATFORM_ENABLE
mcu_dsp_audio_platform_share_buffer_info_t *audio_platform_share_buffer;
#endif

uint8_t dsp_stream_algo_nvkey_loading_status = 0;       // This global is used to protect race condition between anc-monitor task and non realtime rx task.

/******************************************************************************
* Function
******************************************************************************/
static void dsp_audio_msg_ack(uint32_t msg_id, bool from_isr);
U32 dsp_calcuate_number_of_bit(U32 value)
{
    U32 number_of_bit;
    value = value - ((value >> 1) & 0x55555555);
    value = (value & 0x33333333) + ((value >> 2) & 0x33333333);
    number_of_bit = (((value + (value >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
    return number_of_bit;
}

#ifdef AIR_AUDIO_HARDWARE_ENABLE
SOURCE dsp_open_stream_in_afe(mcu2dsp_open_param_p open_param)
{
    SOURCE source;
    bool echo_path = false;
    //DSP_MW_LOG_I("Stream in afe\r\n", 0);
    DSP_MW_LOG_I("afe in device:%d, channel:%d, memory:%d, interface:%d hw_gain:%d\r\n", 5, open_param->stream_in_param.afe.audio_device,
                 open_param->stream_in_param.afe.stream_channel,
                 open_param->stream_in_param.afe.memory,
                 open_param->stream_in_param.afe.audio_interface,
                 open_param->stream_in_param.afe.hw_gain);

#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    DSP_MW_LOG_I("[2A2D] afe in audio_device: %d, %d, %d, %d, %d, %d, %d\r\n", 7, open_param->stream_in_param.afe.audio_device, open_param->stream_in_param.afe.audio_device1, open_param->stream_in_param.afe.audio_device2, open_param->stream_in_param.afe.audio_device3, open_param->stream_in_param.afe.audio_device4, open_param->stream_in_param.afe.audio_device5, open_param->stream_in_param.afe.audio_device6);
    DSP_MW_LOG_I("[2A2D] afe in audio_interface: %d, %d, %d, %d, %d, %d, %d\r\n", 7, open_param->stream_in_param.afe.audio_interface, open_param->stream_in_param.afe.audio_interface1, open_param->stream_in_param.afe.audio_interface2, open_param->stream_in_param.afe.audio_interface3, open_param->stream_in_param.afe.audio_device4, open_param->stream_in_param.afe.audio_device5, open_param->stream_in_param.afe.audio_device6);
#endif

    AudioAfeConfiguration(AUDIO_SOURCE_DATA_FORMAT, open_param->stream_in_param.afe.format);

    if (open_param->stream_in_param.afe.sampling_rate != 0) {
        AudioAfeConfiguration(AUDIO_SOURCE_IRQ_RATE, open_param->stream_in_param.afe.sampling_rate);
    }
#if defined AIR_HWSRC_RX_TRACKING_ENABLE || defined AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE
    if (open_param->stream_in_param.afe.stream_out_sampling_rate != 0) {
        AudioAfeConfiguration(AUDIO_SRC_RATE, open_param->stream_in_param.afe.stream_out_sampling_rate);
        DSP_MW_LOG_I("afe in hwsrc output rate = %d", 1, open_param->stream_in_param.afe.stream_out_sampling_rate);
    }
#endif
    AudioAfeConfiguration(AUDIO_SOURCE_IRQ_PERIOD, open_param->stream_in_param.afe.irq_period);

    /*echo path*/
    echo_path = (open_param->stream_in_param.afe.memory & HAL_AUDIO_MEM3)/* && (dsp_calcuate_number_of_bit(open_param->stream_in_param.afe.audio_interface) == 1)*/;
    if (echo_path) {
        open_param->stream_in_param.afe.memory &= ~HAL_AUDIO_MEM3;//echo path
    }
	DSP_MW_LOG_I("afe in echo path enable:%d, memory:%d\r\n", 2,echo_path, open_param->stream_in_param.afe.memory);


#if 0

    /* 4 A-MIC Verification */
    open_param->stream_in_param.afe.audio_device        = HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_L;
    open_param->stream_in_param.afe.audio_device1       = HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_R;
    open_param->stream_in_param.afe.audio_device2       = HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_L;
    open_param->stream_in_param.afe.audio_device3       = HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_R;

    open_param->stream_in_param.afe.audio_interface     = HAL_AUDIO_INTERFACE_1;
    open_param->stream_in_param.afe.audio_interface1    = HAL_AUDIO_INTERFACE_1;
    open_param->stream_in_param.afe.audio_interface2    = HAL_AUDIO_INTERFACE_2;
    open_param->stream_in_param.afe.audio_interface3    = HAL_AUDIO_INTERFACE_2;

#endif

    AudioAfeConfiguration(AUDIO_SOURCE_CLKSKEW_MODE, open_param->stream_in_param.afe.clkskew_mode);
    DSP_MW_LOG_I("afe source clkskew_mode:%d", 1, open_param->stream_in_param.afe.clkskew_mode);

    AudioAfeConfiguration(AUDIO_SOURCE_DEVICE, open_param->stream_in_param.afe.audio_device);
    DSP_MW_LOG_I("audio_device 0x%x", 1, open_param->stream_in_param.afe.audio_device);
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    AudioAfeConfiguration(AUDIO_SOURCE_DEVICE1, open_param->stream_in_param.afe.audio_device1);
    AudioAfeConfiguration(AUDIO_SOURCE_DEVICE2, open_param->stream_in_param.afe.audio_device2);
    AudioAfeConfiguration(AUDIO_SOURCE_DEVICE3, open_param->stream_in_param.afe.audio_device3);
    DSP_MW_LOG_I("audio_device1 0x%x device2 0x%x device3 0x%x", 3, open_param->stream_in_param.afe.audio_device1,open_param->stream_in_param.afe.audio_device2,open_param->stream_in_param.afe.audio_device3);
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    AudioAfeConfiguration(AUDIO_SOURCE_DEVICE4, open_param->stream_in_param.afe.audio_device4);
    AudioAfeConfiguration(AUDIO_SOURCE_DEVICE5, open_param->stream_in_param.afe.audio_device5);
    AudioAfeConfiguration(AUDIO_SOURCE_DEVICE6, open_param->stream_in_param.afe.audio_device6);
    AudioAfeConfiguration(AUDIO_SOURCE_DEVICE7, open_param->stream_in_param.afe.audio_device7);
	DSP_MW_LOG_I("audio_device4 0x%x audio_device5 0x%x audio_device6 0x%x audio_device7 0x%x", 4, open_param->stream_in_param.afe.audio_device4,open_param->stream_in_param.afe.audio_device5,open_param->stream_in_param.afe.audio_device6,open_param->stream_in_param.afe.audio_device7);
#endif
#endif
    AudioAfeConfiguration(AUDIO_SOURCE_CHANNEL, open_param->stream_in_param.afe.stream_channel);
    AudioAfeConfiguration(AUDIO_SOURCE_MEMORY, open_param->stream_in_param.afe.memory);
    AudioAfeConfiguration(AUDIO_SOURCE_INTERFACE, open_param->stream_in_param.afe.audio_interface);
    DSP_MW_LOG_I("audio_interface 0x%x", 1, open_param->stream_in_param.afe.audio_interface);
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    AudioAfeConfiguration(AUDIO_SOURCE_INTERFACE1, open_param->stream_in_param.afe.audio_interface1);
    AudioAfeConfiguration(AUDIO_SOURCE_INTERFACE2, open_param->stream_in_param.afe.audio_interface2);
    AudioAfeConfiguration(AUDIO_SOURCE_INTERFACE3, open_param->stream_in_param.afe.audio_interface3);
    DSP_MW_LOG_I("audio_interface1 0x%x audio_interface2 0x%x audio_interface3 0x%x", 3,open_param->stream_in_param.afe.audio_interface1,open_param->stream_in_param.afe.audio_interface2, open_param->stream_in_param.afe.audio_interface3);
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    AudioAfeConfiguration(AUDIO_SOURCE_INTERFACE4, open_param->stream_in_param.afe.audio_interface4);
    AudioAfeConfiguration(AUDIO_SOURCE_INTERFACE5, open_param->stream_in_param.afe.audio_interface5);
    AudioAfeConfiguration(AUDIO_SOURCE_INTERFACE6, open_param->stream_in_param.afe.audio_interface6);
    AudioAfeConfiguration(AUDIO_SOURCE_INTERFACE7, open_param->stream_in_param.afe.audio_interface7);
    DSP_MW_LOG_I("audio_interface4 0x%x audio_interface5 0x%x audio_interface6 0x%x audio_interface7 0x%x", 4,open_param->stream_in_param.afe.audio_interface1,open_param->stream_in_param.afe.audio_interface2, open_param->stream_in_param.afe.audio_interface3);
#endif
#endif
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    AudioAfeConfiguration(AUDIO_SOURCE_ADC_MODE, open_param->stream_in_param.afe.ul_adc_mode[0]);
    AudioAfeConfiguration(AUDIO_SOURCE_ADC_MODE1, open_param->stream_in_param.afe.ul_adc_mode[1]);
    AudioAfeConfiguration(AUDIO_SOURCE_ADC_MODE2, open_param->stream_in_param.afe.ul_adc_mode[2]);
    AudioAfeConfiguration(AUDIO_SOURCE_ADC_MODE3, open_param->stream_in_param.afe.ul_adc_mode[3]);
    AudioAfeConfiguration(AUDIO_SOURCE_ADC_MODE4, open_param->stream_in_param.afe.ul_adc_mode[4]);
    AudioAfeConfiguration(AUDIO_SOURCE_ADC_MODE5, open_param->stream_in_param.afe.ul_adc_mode[5]);
    AudioAfeConfiguration(AUDIO_SOURCE_ADC_MODE6, open_param->stream_in_param.afe.ul_adc_mode[6]);
    AudioAfeConfiguration(AUDIO_SOURCE_ADC_MODE7, open_param->stream_in_param.afe.ul_adc_mode[7]);

    AudioAfeConfiguration(AUDIO_SOURCE_ADC_TYPE, open_param->stream_in_param.afe.amic_type[0]);
    AudioAfeConfiguration(AUDIO_SOURCE_ADC_TYPE1, open_param->stream_in_param.afe.amic_type[1]);
    AudioAfeConfiguration(AUDIO_SOURCE_ADC_TYPE2, open_param->stream_in_param.afe.amic_type[2]);
    AudioAfeConfiguration(AUDIO_SOURCE_ADC_TYPE3, open_param->stream_in_param.afe.amic_type[3]);
    AudioAfeConfiguration(AUDIO_SOURCE_ADC_TYPE4, open_param->stream_in_param.afe.amic_type[4]);
    AudioAfeConfiguration(AUDIO_SOURCE_ADC_TYPE5, open_param->stream_in_param.afe.amic_type[5]);
    AudioAfeConfiguration(AUDIO_SOURCE_ADC_TYPE6, open_param->stream_in_param.afe.amic_type[6]);
    AudioAfeConfiguration(AUDIO_SOURCE_ADC_TYPE7, open_param->stream_in_param.afe.amic_type[7]);

    AudioAfeConfiguration(AUDIO_SOURCE_BIAS_VOLTAGE, open_param->stream_in_param.afe.bias_voltage[0]);
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    AudioAfeConfiguration(AUDIO_SOURCE_BIAS_VOLTAGE1, open_param->stream_in_param.afe.bias_voltage[1]);
    AudioAfeConfiguration(AUDIO_SOURCE_BIAS_VOLTAGE2, open_param->stream_in_param.afe.bias_voltage[2]);
    AudioAfeConfiguration(AUDIO_SOURCE_BIAS_VOLTAGE3, open_param->stream_in_param.afe.bias_voltage[3]);
    AudioAfeConfiguration(AUDIO_SOURCE_BIAS_VOLTAGE4, open_param->stream_in_param.afe.bias_voltage[4]);
#endif
    AudioAfeConfiguration(AUDIO_SOURCE_BIAS_SELECT, open_param->stream_in_param.afe.bias_select);
    AudioAfeConfiguration(AUDIO_SOURCE_WITH_EXTERNAL_BIAS, open_param->stream_in_param.afe.with_external_bias);
    AudioAfeConfiguration(AUDIO_SOURCE_WITH_BIAS_LOWPOWER, open_param->stream_in_param.afe.with_bias_lowpower);
    AudioAfeConfiguration(AUDIO_SOURCE_BIAS1_BIAS2_WITH_LDO0, open_param->stream_in_param.afe.bias1_2_with_LDO0);
    AudioAfeConfiguration(AUDIO_SOURCE_DMIC_SELECT, open_param->stream_in_param.afe.dmic_selection[0]);
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    AudioAfeConfiguration(AUDIO_SOURCE_DMIC_SELECT1, open_param->stream_in_param.afe.dmic_selection[1]);
    AudioAfeConfiguration(AUDIO_SOURCE_DMIC_SELECT2, open_param->stream_in_param.afe.dmic_selection[2]);
    AudioAfeConfiguration(AUDIO_SOURCE_DMIC_SELECT3, open_param->stream_in_param.afe.dmic_selection[3]);
    AudioAfeConfiguration(AUDIO_SOURCE_DMIC_SELECT4, open_param->stream_in_param.afe.dmic_selection[4]);
    AudioAfeConfiguration(AUDIO_SOURCE_DMIC_SELECT5, open_param->stream_in_param.afe.dmic_selection[5]);
    AudioAfeConfiguration(AUDIO_SOURCE_DMIC_SELECT6, open_param->stream_in_param.afe.dmic_selection[6]);
    AudioAfeConfiguration(AUDIO_SOURCE_DMIC_SELECT7, open_param->stream_in_param.afe.dmic_selection[7]);
    DSP_MW_LOG_I("dmic_selection=%d,%d,%d,%d,%d,%d,%d,%d", 8, open_param->stream_in_param.afe.dmic_selection[0],
                 open_param->stream_in_param.afe.dmic_selection[1],
                 open_param->stream_in_param.afe.dmic_selection[2],
                 open_param->stream_in_param.afe.dmic_selection[3],
                 open_param->stream_in_param.afe.dmic_selection[4],
                 open_param->stream_in_param.afe.dmic_selection[5],
                 open_param->stream_in_param.afe.dmic_selection[6],
                 open_param->stream_in_param.afe.dmic_selection[7]);
#endif
    AudioAfeConfiguration(AUDIO_SOURCE_UL_IIR, open_param->stream_in_param.afe.iir_filter[0]);
    AudioAfeConfiguration(AUDIO_SOURCE_DMIC_CLOCK, open_param->stream_in_param.afe.dmic_clock_rate[0]);

#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
    AudioAfeConfiguration(AUDIO_SOURCE_UL_IIR1, open_param->stream_in_param.afe.iir_filter[1]);
    AudioAfeConfiguration(AUDIO_SOURCE_UL_IIR2, open_param->stream_in_param.afe.iir_filter[2]);
    AudioAfeConfiguration(AUDIO_SOURCE_DMIC_CLOCK1, open_param->stream_in_param.afe.dmic_clock_rate[1]);
    AudioAfeConfiguration(AUDIO_SOURCE_DMIC_CLOCK2, open_param->stream_in_param.afe.dmic_clock_rate[2]);
#endif
    AudioAfeConfiguration(AUDIO_SOURCE_UL_PERFORMANCE, open_param->stream_in_param.afe.performance);
    if ((open_param->stream_in_param.afe.audio_device | open_param->stream_in_param.afe.audio_device1 |
         open_param->stream_in_param.afe.audio_device2 | open_param->stream_in_param.afe.audio_device3 |
         open_param->stream_in_param.afe.audio_device4 | open_param->stream_in_param.afe.audio_device5) &
        (HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER | HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_L | HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_R)) {
        AudioAfeConfiguration(AUDIO_SOURCE_I2S_MASTER_SAMPLING_RATE, open_param->stream_in_param.afe.i2s_master_sampling_rate[0]);
        AudioAfeConfiguration(AUDIO_SOURCE_I2S_MASTER_SAMPLING_RATE1, open_param->stream_in_param.afe.i2s_master_sampling_rate[1]);
        AudioAfeConfiguration(AUDIO_SOURCE_I2S_MASTER_SAMPLING_RATE2, open_param->stream_in_param.afe.i2s_master_sampling_rate[2]);
        AudioAfeConfiguration(AUDIO_SOURCE_I2S_MASTER_SAMPLING_RATE3, open_param->stream_in_param.afe.i2s_master_sampling_rate[3]);
        AudioAfeConfiguration(AUDIO_SOURCE_I2S_MASTER_FORMAT, open_param->stream_in_param.afe.i2s_master_format[0]);
        AudioAfeConfiguration(AUDIO_SOURCE_I2S_MASTER_FORMAT1, open_param->stream_in_param.afe.i2s_master_format[1]);
        AudioAfeConfiguration(AUDIO_SOURCE_I2S_MASTER_FORMAT2, open_param->stream_in_param.afe.i2s_master_format[2]);
        AudioAfeConfiguration(AUDIO_SOURCE_I2S_MASTER_FORMAT3, open_param->stream_in_param.afe.i2s_master_format[3]);
        AudioAfeConfiguration(AUDIO_SOURCE_I2S_MASTER_WORD_LENGTH, open_param->stream_in_param.afe.i2s_master_word_length[0]);
        AudioAfeConfiguration(AUDIO_SOURCE_I2S_MASTER_WORD_LENGTH1, open_param->stream_in_param.afe.i2s_master_word_length[1]);
        AudioAfeConfiguration(AUDIO_SOURCE_I2S_MASTER_WORD_LENGTH2, open_param->stream_in_param.afe.i2s_master_word_length[2]);
        AudioAfeConfiguration(AUDIO_SOURCE_I2S_MASTER_WORD_LENGTH3, open_param->stream_in_param.afe.i2s_master_word_length[3]);
        AudioAfeConfiguration(AUDIO_SOURCE_I2S_MASTER_LOW_JITTER, open_param->stream_in_param.afe.is_low_jitter[0]);
        AudioAfeConfiguration(AUDIO_SOURCE_I2S_MASTER_LOW_JITTER1, open_param->stream_in_param.afe.is_low_jitter[1]);
        AudioAfeConfiguration(AUDIO_SOURCE_I2S_MASTER_LOW_JITTER2, open_param->stream_in_param.afe.is_low_jitter[2]);
        AudioAfeConfiguration(AUDIO_SOURCE_I2S_MASTER_LOW_JITTER3, open_param->stream_in_param.afe.is_low_jitter[3]);

        for (U32 i = 0 ; i < sizeof(open_param->stream_in_param.afe.i2s_master_sampling_rate) / sizeof(open_param->stream_in_param.afe.i2s_master_sampling_rate[0]) ; ++i) {
            DSP_MW_LOG_I("i2s master configuration[%d]=%d,%d,%d,%d", 5,
                i,
                open_param->stream_in_param.afe.i2s_master_sampling_rate[i],
                open_param->stream_in_param.afe.i2s_master_format[i],
                open_param->stream_in_param.afe.i2s_master_word_length[i],
                open_param->stream_in_param.afe.is_low_jitter[i]);
        }
    } else if (open_param->stream_in_param.afe.audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
        AudioAfeConfiguration(AUDIO_SOURCE_I2S_FORMAT, open_param->stream_in_param.afe.i2s_format);
        AudioAfeConfiguration(AUDIO_SOURCE_I2S_SLAVE_TDM, open_param->stream_in_param.afe.i2S_Slave_TDM);
        AudioAfeConfiguration(AUDIO_SOURCE_I2S_WORD_LENGTH, open_param->stream_in_param.afe.i2s_word_length);
    }
#endif

    AudioAfeConfiguration(AUDIO_SOURCE_HW_GAIN, open_param->stream_in_param.afe.hw_gain);
#if AUTO_ERROR_SUPPRESSION
    AudioAfeConfiguration(AUDIO_SOURCE_MISC_PARMS_I2S_CLK, open_param->stream_in_param.afe.misc_parms.I2sClkSourceType);
    AudioAfeConfiguration(AUDIO_SOURCE_MISC_PARMS_MICBIAS, open_param->stream_in_param.afe.misc_parms.MicbiasSourceType);
#endif
    AudioAfeConfiguration(AUDIO_SOURCE_MISC_PARMS, open_param->stream_in_param.afe.misc_parms);
    AudioAfeConfiguration(AUDIO_SOURCE_ECHO_REFERENCE, echo_path);
#ifdef AIR_HFP_DNN_PATH_ENABLE
    AudioAfeConfiguration(AUDIO_SOURCE_DNN_PATH_ENABLE, open_param->stream_in_param.afe.enable_ul_dnn);
#endif
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
    AudioAfeConfiguration(AUDIO_SOURCE_MAX_CHANNEL_NUM, open_param->stream_in_param.afe.max_channel_num);
#endif

    AudioAfeConfiguration(AUDIO_SOURCE_DEVICE_INPUT_RATE, open_param->stream_in_param.afe.audio_device_input_rate[0]);
    AudioAfeConfiguration(AUDIO_SOURCE_DEVICE_INPUT_RATE1, open_param->stream_in_param.afe.audio_device_input_rate[1]);
    AudioAfeConfiguration(AUDIO_SOURCE_DEVICE_INPUT_RATE2, open_param->stream_in_param.afe.audio_device_input_rate[2]);
    AudioAfeConfiguration(AUDIO_SOURCE_DEVICE_INPUT_RATE3, open_param->stream_in_param.afe.audio_device_input_rate[3]);
    AudioAfeConfiguration(AUDIO_SOURCE_DEVICE_INPUT_RATE4, open_param->stream_in_param.afe.audio_device_input_rate[4]);
    AudioAfeConfiguration(AUDIO_SOURCE_DEVICE_INPUT_RATE5, open_param->stream_in_param.afe.audio_device_input_rate[5]);
    AudioAfeConfiguration(AUDIO_SOURCE_DEVICE_INPUT_RATE6, open_param->stream_in_param.afe.audio_device_input_rate[6]);
    AudioAfeConfiguration(AUDIO_SOURCE_DEVICE_INPUT_RATE7, open_param->stream_in_param.afe.audio_device_input_rate[7]);

    DSP_MW_LOG_I("afe in format:%d, sampling rate:%d, IRQ period:%d mise:0x%x\r\n", 4, open_param->stream_in_param.afe.format,
                 gAudioCtrl.Afe.AfeULSetting.rate,
                 open_param->stream_in_param.afe.irq_period,
                 open_param->stream_in_param.afe.misc_parms);

    //TEMP!! should remove Audio_Default_setting_init
    Audio_setting->Audio_source.Buffer_Frame_Num = open_param->stream_in_param.afe.frame_number;
    Audio_setting->Audio_source.Frame_Size       = open_param->stream_in_param.afe.frame_size;

    if (open_param->stream_in_param.afe.memory & HAL_AUDIO_MEM_SUB) {
        //Sub-Source
        source = StreamAudioAfeSubSource(afe_get_audio_hardware_by_au_afe_open_param(&(open_param->stream_in_param.afe)),
                                         afe_get_audio_instance_by_au_afe_open_param(&(open_param->stream_in_param.afe)),
                                         afe_get_audio_channel_by_au_afe_open_param(&(open_param->stream_in_param.afe)));
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
    } else if (open_param->stream_in_param.afe.memory & HAL_AUDIO_MEM7) {
        //Tdm-Source
        source = StreamAudioAfeTdmSource(afe_get_audio_hardware_by_au_afe_open_param(&(open_param->stream_in_param.afe)),
                                         afe_get_audio_instance_by_au_afe_open_param(&(open_param->stream_in_param.afe)),
                                         afe_get_audio_channel_by_au_afe_open_param(&(open_param->stream_in_param.afe)));
#endif
#ifdef AIR_I2S_SLAVE_ENABLE
    } else if (open_param->stream_in_param.afe.memory & HAL_AUDIO_MEM2) {
        source = StreamAudioAfe2Source(afe_get_audio_hardware_by_au_afe_open_param(&(open_param->stream_in_param.afe)),
                                       afe_get_audio_instance_by_au_afe_open_param(&(open_param->stream_in_param.afe)),
                                       afe_get_audio_channel_by_au_afe_open_param(&(open_param->stream_in_param.afe)));
#endif
    } else {
        source = StreamAudioAfeSource(afe_get_audio_hardware_by_au_afe_open_param(&(open_param->stream_in_param.afe)),
                                      afe_get_audio_instance_by_au_afe_open_param(&(open_param->stream_in_param.afe)),
                                      afe_get_audio_channel_by_au_afe_open_param(&(open_param->stream_in_param.afe)));
    }

    return source;
}
#endif /* AIR_AUDIO_HARDWARE_ENABLE */

#ifdef AIR_VOICE_NR_ENABLE
#ifdef AIR_AIRDUMP_ENABLE
extern AIRDUMPCTRL_t *rAirDumpCtrl;
#endif
#endif

U16 g_cpd_nv_length;
U8 g_cpd_hse_mode;
#if defined(AIR_BTA_IC_PREMIUM_G2) && defined(AIR_HEARING_PROTECTION_ENABLE)
extern bool g_hearing_protection_enable;
#endif
#ifdef AIR_BT_HFP_ENABLE

CONNECTION_IF n9_sco_dl_if;

SOURCE dsp_open_stream_in_hfp(mcu2dsp_open_param_p open_param)
{
    SOURCE source;
    DSP_MW_LOG_I("Stream in hfp in Codec:%d\r\n", 1, open_param->stream_in_param.hfp.codec_type);

    if (open_param->stream_in_param.hfp.codec_type == BT_HFP_CODEC_CVSD) {
        DSP_ALG_UpdateEscoRxMode(VOICE_NB);
        n9_sco_dl_if.pfeature_table = (stream_feature_list_ptr_t)&stream_feature_list_hfp_downlink_cvsd;
    } else if (open_param->stream_in_param.hfp.codec_type  == BT_HFP_CODEC_mSBC) {
        DSP_ALG_UpdateEscoRxMode(VOICE_WB);
        n9_sco_dl_if.pfeature_table = (stream_feature_list_ptr_t)&stream_feature_list_hfp_downlink_msbc;
    } else {
        //not support codec type
    }

    source = StreamN9ScoSource(open_param->stream_in_param.hfp.p_share_info);
    if (source != NULL) {
        source->streamBuffer.AVMBufferInfo.SampleRate = 16000;
        source->param.n9sco.share_info_base_addr->SampleRate = 16000;
        AudioAfeConfiguration(AUDIO_SINK_IRQ_RATE, 16000);
    } else {
        DSP_MW_LOG_E("DSP source create fail\r\n", 0);
    }

#ifdef AIR_VOICE_NR_ENABLE
#ifdef AIR_AIRDUMP_ENABLE
    rAirDumpCtrl = (AIRDUMPCTRL_t *)open_param->stream_in_param.hfp.p_air_dump_buf;
#endif
#endif
    g_cpd_nv_length = open_param->stream_in_param.hfp.cpd_param.cpd_nv_length;
    g_cpd_hse_mode = open_param->stream_in_param.hfp.cpd_param.cpd_hse_mode;
#if defined(AIR_BTA_IC_PREMIUM_G2) && defined(AIR_HEARING_PROTECTION_ENABLE)
    g_hearing_protection_enable = true;
#endif
    DSP_MW_LOG_I("[HFP][CPD] HSE mode %d, g_cpd_nv_length:%d", 2, g_cpd_hse_mode, g_cpd_nv_length);

    return source;
}
#endif

#ifdef AIR_BT_CODEC_BLE_ENABLED
bool g_n9_ble_dl_ll_flag = false;
#ifdef AIR_BT_LE_LC3_ENABLE
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
GroupMemoryInfo_t lc3_fft_memory_addr;
#endif
#endif

SOURCE dsp_open_stream_in_ble(mcu2dsp_open_param_p open_param)
{
    SOURCE source;
#ifdef AIR_BT_LE_LC3_ENABLE
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    DspMemoryManagerReturnStatus_t memory_return_status;
    memset(&lc3_fft_memory_addr, 0, sizeof(GroupMemoryInfo_t));
#endif
    void *code_addr_fft = NULL;
    void *data_addr_fft = NULL;
    uint32_t pic_dram_usage = 0;
#endif

    DSP_MW_LOG_I("[BLE] Stream in ble, Codec:%d", 1, open_param->stream_in_param.ble.codec_type);
    if(open_param->stream_in_param.ble.context_type == BLE_CONTENT_TYPE_ULL_BLE) {
        n9_ble_dl_if.pfeature_table = AudioFeatureList_ULL_BLE_DL;
#ifdef AIR_BT_LE_LC3PLUS_ENABLE
        if (open_param->stream_in_param.ble.codec_type == BT_BLE_CODEC_LC3PLUS)
        {
            stream_feature_configure_type(n9_ble_dl_if.pfeature_table, CODEC_DECODER_LC3PLUS, CONFIG_DECODER);
            stream_feature_configure_resolution((stream_feature_list_ptr_t)n9_ble_dl_if.pfeature_table, RESOLUTION_32BIT, CONFIG_DECODER);
        }
#endif
#ifdef AIR_CELT_DEC_V2_ENABLE
        if (open_param->stream_in_param.ble.codec_type == BT_BLE_CODEC_VENDOR)
        {
            stream_feature_configure_type(n9_ble_dl_if.pfeature_table, CODEC_DECODER_OPUS_V2, CONFIG_DECODER);
            stream_feature_configure_resolution((stream_feature_list_ptr_t)n9_ble_dl_if.pfeature_table, RESOLUTION_16BIT, CONFIG_DECODER);
        }
#endif
#ifdef AIR_AUDIO_ULD_DECODE_ENABLE
        if (open_param->stream_in_param.ble.codec_type == BT_BLE_CODEC_ULD)
        {
            stream_feature_configure_type(n9_ble_dl_if.pfeature_table, CODEC_DECODER_ULD, CONFIG_DECODER);
            stream_feature_configure_resolution((stream_feature_list_ptr_t)n9_ble_dl_if.pfeature_table, RESOLUTION_32BIT, CONFIG_DECODER);
        }
#endif
    } else

    {
        if ((open_param->stream_in_param.ble.codec_type == BT_BLE_CODEC_LC3)||(open_param->stream_in_param.ble.codec_type == BT_BLE_CODEC_LC3PLUS)) {
#ifdef AIR_BT_LE_LC3_ENABLE
            g_n9_ble_dl_ll_flag = FALSE;
            if (open_param->stream_in_param.ble.decode_mode == CODEC_AUDIO_MODE) {
                n9_ble_dl_if.pfeature_table = AudioFeatureList_BLE_Music_DL;
            } else if (open_param->stream_in_param.ble.decode_mode == CODEC_LL_MODE){
                n9_ble_dl_if.pfeature_table = AudioFeatureList_BLE_LL_DL;
                g_n9_ble_dl_ll_flag = TRUE;
            } else {
                n9_ble_dl_if.pfeature_table = AudioFeatureList_BLE_Call_DL;
                #ifdef AIR_BT_BLE_SWB_ENABLE
                DSP_ALG_UpdateEscoTxMode(VOICE_SWB);         /*32K sample rate. Need SWB algorithm*/
                DSP_ALG_UpdateEscoRxMode(VOICE_SWB);         /*32K sample rate. Need SWB algorithm*/
                #else
                DSP_ALG_UpdateEscoTxMode(VOICE_WB);         /*16K sample rate. Need WB algorithm*/
                DSP_ALG_UpdateEscoRxMode(VOICE_WB);         /*16K sample rate. Need WB algorithm*/
                #endif
                g_cpd_nv_length = open_param->stream_in_param.ble.cpd_param.cpd_nv_length;
                g_cpd_hse_mode = open_param->stream_in_param.ble.cpd_param.cpd_hse_mode;
                #if defined(AIR_BTA_IC_PREMIUM_G2) && defined(AIR_HEARING_PROTECTION_ENABLE)
                    g_hearing_protection_enable = true;
                #endif
                DSP_MW_LOG_I("[BLE][CPD] HSE mode %d, g_cpd_nv_length:%d", 2, g_cpd_hse_mode, g_cpd_nv_length);
            }
            if (open_param->stream_in_param.ble.codec_type == BT_BLE_CODEC_LC3PLUS)
            {
                stream_feature_configure_type(n9_ble_dl_if.pfeature_table, CODEC_DECODER_LC3PLUS, CONFIG_DECODER);
                stream_feature_configure_resolution((stream_feature_list_ptr_t)n9_ble_dl_if.pfeature_table, RESOLUTION_32BIT, CONFIG_DECODER);
            }
            else
            {
                stream_feature_configure_type(n9_ble_dl_if.pfeature_table, CODEC_DECODER_LC3, CONFIG_DECODER);
                lc3_dual_decode_mode_set(open_param->stream_in_param.ble.channel_num == 2);
                pic_dram_usage = 0;

#ifdef PRELOADER_ENABLE
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
                memory_return_status = DspMemoryManager_AcquireGroupMemory(Component_LE_CALL,SubComponent_LECALL_LC3_FFT,&lc3_fft_memory_addr);
                code_addr_fft = lc3_fft_memory_addr.IramAddr;
                data_addr_fft = lc3_fft_memory_addr.DramAddr;
#endif
                if (open_param->stream_in_param.ble.frame_ms == 10000) {
                    lc3i_fft10ms_library_load(code_addr_fft,data_addr_fft,&pic_dram_usage);
                } else if (open_param->stream_in_param.ble.frame_ms == 7500) {
                    lc3i_fft7_5ms_library_load(code_addr_fft,data_addr_fft,&pic_dram_usage);
                } else {
                    DSP_MW_LOG_E("[BLE] unsupported DL frame interval %d", 1, open_param->stream_in_param.ble.frame_ms);
                }
#endif
            }
#endif
        }

    }
#ifdef AIR_VOICE_NR_ENABLE
#ifdef AIR_AIRDUMP_ENABLE
        rAirDumpCtrl = (AIRDUMPCTRL_t *)open_param->stream_in_param.ble.p_air_dump_buf;
#endif
#endif

    source = StreamN9BleSource(open_param);

    return source;
}
#endif

#ifdef MTK_BT_A2DP_ENABLE
SOURCE dsp_open_stream_in_a2dp(mcu2dsp_open_param_p open_param)
{
    SOURCE source;
    U32 samplerate = 0, channel = 0;
    bt_codec_capability_t *codec_cap_ptr;

    DSP_MW_LOG_I("Stream In A2DP\r\n", 0);
    source  = StreamN9A2dpSource(&open_param->stream_in_param.a2dp);

    if (source != NULL) {
        /* parse codec info */
        samplerate  = a2dp_get_samplingrate(source);
        channel     = a2dp_get_channel(source);
        codec_cap_ptr = &(source->param.n9_a2dp.codec_info.codec_cap);

        DSP_MW_LOG_I("A2DP codec type: %d, sr: %d, SL: %d\r\n", 3, codec_cap_ptr->type, samplerate, source->param.n9_a2dp.sink_latency);
        n9_a2dp_if.pfeature_table = stream_feature_list_a2dp;
        if (codec_cap_ptr->type == BT_A2DP_CODEC_VENDOR) {

            DSP_UpdateA2DPCodec(BT_A2DP_CODEC_VENDOR);

            DSP_MW_LOG_I("A2DP codec id :%x", 1, codec_cap_ptr->codec.vend.codec_id);
            if (codec_cap_ptr->codec.vend.codec_id == BT_A2DP_CODEC_LHDC_CODEC_ID){
#if !defined(MTK_BT_A2DP_VENDOR_2_ENABLE)
            DSP_MW_LOG_E("A2DP request unsupported codec", 0);
            AUDIO_ASSERT(0);
#endif
                stream_feature_configure_type(n9_a2dp_if.pfeature_table, CODEC_DECODER_VENDOR_2, CONFIG_DECODER);

#ifdef MTK_AUDIO_PLC_ENABLE
                Audio_PLC_ctrl(open_param->stream_in_param.a2dp.audio_plc);
#endif
            }else if (codec_cap_ptr->codec.vend.codec_id == BT_A2DP_CODEC_LC3PLUS_CODEC_ID){
#if !defined(AIR_BT_A2DP_LC3PLUS_ENABLE)
            DSP_MW_LOG_E("A2DP request unsupported codec", 0);
            AUDIO_ASSERT(0);
#endif
                stream_feature_configure_type(n9_a2dp_if.pfeature_table, CODEC_DECODER_LC3PLUS, CONFIG_DECODER);

#ifdef MTK_AUDIO_PLC_ENABLE
                Audio_PLC_ctrl(open_param->stream_in_param.a2dp.audio_plc);
#endif
            } else{
#if !defined(MTK_BT_A2DP_VENDOR_ENABLE) && !defined(MTK_BT_A2DP_VENDOR_1_ENABLE)
            configASSERT(0 && "A2DP request unsupported codec");
#endif
#if defined(MTK_BT_A2DP_VENDOR_BC_ENABLE) || defined(MTK_BT_A2DP_VENDOR_1_BC_ENABLE)
            if (source->param.n9_a2dp.sink_latency == 0) {
                n9_a2dp_if.pfeature_table = stream_feature_list_vend_a2dp;
            }
#endif
#ifdef MTK_BT_A2DP_VENDOR_1_ENABLE
            stream_feature_configure_type(n9_a2dp_if.pfeature_table, CODEC_DECODER_VENDOR_1, CONFIG_DECODER);
#else
            stream_feature_configure_type(n9_a2dp_if.pfeature_table, CODEC_DECODER_VENDOR, CONFIG_DECODER);
#endif
            //not support codec type
#ifdef MTK_AUDIO_PLC_ENABLE
            Audio_PLC_ctrl(open_param->stream_in_param.a2dp.audio_plc);
#endif
            }
        }else if (codec_cap_ptr->type == BT_A2DP_CODEC_SBC) {
            DSP_UpdateA2DPCodec(BT_A2DP_CODEC_SBC);
            stream_feature_configure_type(n9_a2dp_if.pfeature_table, CODEC_DECODER_SBC, CONFIG_DECODER);
#ifdef MTK_AUDIO_PLC_ENABLE
            Audio_PLC_ctrl(open_param->stream_in_param.a2dp.audio_plc);
#endif
        } else if (codec_cap_ptr->type == BT_A2DP_CODEC_AAC) {
            DSP_UpdateA2DPCodec(BT_A2DP_CODEC_AAC);
            stream_feature_configure_type(n9_a2dp_if.pfeature_table, CODEC_DECODER_AAC, CONFIG_DECODER);
#ifdef MTK_AUDIO_PLC_ENABLE
            Audio_PLC_ctrl(open_param->stream_in_param.a2dp.audio_plc);
#endif
        }
#ifdef MTK_GAMING_MODE_HEADSET
        else if (codec_cap_ptr->type == BT_A2DP_CODEC_AIRO_CELT) {
            samplerate = 48000;
            DSP_UpdateA2DPCodec(BT_A2DP_CODEC_AIRO_CELT);
            stream_feature_configure_type(n9_a2dp_if.pfeature_table, CODEC_DECODER_CELT_HD, CONFIG_DECODER);
        }
#endif

        source->streamBuffer.AVMBufferInfo.SampleRate = samplerate;
        memcpy(&(((n9_dsp_share_info_ptr)(source->param.n9_a2dp.share_info_base_addr))->sampling_rate), &(source->streamBuffer.AVMBufferInfo.SampleRate), 4);/* update sample_rate */
#ifdef AIR_AUDIO_HARDWARE_ENABLE
        AudioAfeConfiguration(AUDIO_SINK_IRQ_RATE, samplerate);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */

#ifdef AIR_BT_CLK_SKEW_ENABLE
        /* long-term clock skew */
        lt_clk_skew_reset_info();
        lt_clk_skew_set_sample_rate(samplerate);
        lt_clk_skew_set_asi_buf(source->param.n9_a2dp.asi_buf);
        lt_clk_skew_set_min_gap_buf(source->param.n9_a2dp.min_gap_buf);
        lt_clk_skew_set_sink_latency(source->param.n9_a2dp.sink_latency);
#endif /* AIR_BT_CLK_SKEW_ENABLE */

        source->param.n9_a2dp.p_afe_buf_report = open_param->stream_in_param.a2dp.p_afe_buf_report;// store afe buffer report instead
#ifdef AIR_BT_A2DP_LC3PLUS_ENABLE
        source->param.n9_a2dp.plc_state_len = sizeof(lc3plus_dec_frame_status_t);
#endif
#ifdef MTK_PEQ_ENABLE
        PEQ_Reset_Info();
#endif

    }

    return source;
}
#endif /* MTK_BT_A2DP_ENABLE */

SOURCE dsp_open_stream_in_playback(mcu2dsp_open_param_p open_param)
{
    SOURCE source;
    U32 sample_rate;
    DSP_MW_LOG_I("Stream in playback\r\n", 0);

    source = StreamCM4PlaybackSource(&open_param->stream_in_param.playback);

    if (source) {
        sample_rate = source->param.cm4_playback.info.sampling_rate;

        /* Yo: Should switch to VP afe sink later */
        source->streamBuffer.ShareBufferInfo.sampling_rate = sample_rate;
#ifdef AIR_AUDIO_HARDWARE_ENABLE
        AudioAfeConfiguration(AUDIO_SINK_IRQ_RATE, sample_rate);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
        playback_if.pfeature_table = (stream_feature_list_ptr_t)&stream_feature_list_playback;
        stream_feature_configure_type(playback_if.pfeature_table, CODEC_PCM_COPY, CONFIG_DECODER);
    }

    return source;
}

SOURCE dsp_open_stream_in_vp(mcu2dsp_open_param_p open_param)
{
    SOURCE source = NULL;
    U32 sample_rate;
    DSP_MW_LOG_I("Stream in vp\r\n", 0);
#ifdef MTK_PROMPT_SOUND_ENABLE
    source = StreamCM4VPPlaybackSource(&open_param->stream_in_param.playback);
#else
    UNUSED(open_param);
#endif
    if (source) {
        sample_rate = source->param.cm4_playback.info.sampling_rate;

        /* Yo: Should switch to VP afe sink later */
        source->streamBuffer.ShareBufferInfo.sampling_rate = sample_rate;
#ifdef AIR_AUDIO_HARDWARE_ENABLE
        AudioAfeConfiguration(AUDIO_SINK_IRQ_RATE, sample_rate);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */

        stream_feature_configure_type(stream_feature_list_prompt, CODEC_PCM_COPY, CONFIG_DECODER);
    }

    return source;
}

#ifdef MTK_PROMPT_SOUND_DUMMY_SOURCE_ENABLE
SOURCE dsp_open_stream_in_vp_dummy_source(mcu2dsp_open_param_p open_param)
{
    SOURCE source;
    U32 sample_rate;
    DSP_MW_LOG_I("Stream in VP Dummy Source\r\n", 0);

    source = StreamCM4VPDummySourcePlaybackSource(&open_param->stream_in_param.playback);

    if (source) {
        sample_rate = source->param.cm4_playback.info.sampling_rate;

        /* Should switch to VP afe sink later */
        source->streamBuffer.ShareBufferInfo.sample_rate = sample_rate;
        AudioAfeConfiguration(AUDIO_SINK_IRQ_RATE, sample_rate);

        stream_feature_configure_type(stream_feature_list_prompt_dummy_source, CODEC_PCM_COPY, CONFIG_DECODER);
    }

    return source;
}
#endif

#ifdef MTK_SENSOR_SOURCE_ENABLE
SOURCE dsp_open_stream_in_gsensor(mcu2dsp_open_param_p open_param)
{
    SOURCE source;

    open_param = open_param;

    source = StreamGsensorSource();

    return source;
}
#endif

#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
SOURCE dsp_open_stream_in_audio_transmitter(mcu2dsp_open_param_p open_param)
{
    SOURCE source;

    DSP_MW_LOG_I("Stream in aduio transmitter\r\n", 0);

    source = StreamAudioTransmitterSource(&(open_param->stream_in_param.data_dl));

    return source;
}
#endif /* AIR_AUDIO_TRANSMITTER_ENABLE */

#ifdef AIR_AUDIO_BT_COMMON_ENABLE
SOURCE dsp_open_stream_in_bt_common(mcu2dsp_open_param_p open_param)
{
    SOURCE source;

    DSP_MW_LOG_I("Stream in bt common\r\n", 0);

    source = StreamBTCommonSource(&(open_param->stream_in_param.bt_common));

    return source;
}
#endif /* AIR_AUDIO_BT_COMMON_ENABLE */

SOURCE dsp_open_stream_in_virtual(mcu2dsp_open_param_p open_param)
{
    SOURCE source;
    DSP_MW_LOG_I("Stream in virtual\r\n", 0);
    UNUSED(open_param);
    U8 channel_num = 0;
    U32 data_size = 0;
    U32 samplingrate = 0;
    VIRTUAL_SOURCE_TYPE type = VIRTUAL_SOURCE_ZERO_DATA;
#if defined(AIR_WIRED_AUDIO_ENABLE)
    if (open_param->audio_scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_OUT_IEM) {
        channel_num = 2;
        extern uint32_t g_usb_in_out_pre_sampling_rate;
        samplingrate = g_usb_in_out_pre_sampling_rate;
        data_size = open_param->stream_out_param.afe.frame_size * ((open_param->stream_out_param.afe.format > HAL_AUDIO_PCM_FORMAT_U16_BE)?4:2);
        extern CONNECTION_IF g_usb_in_out_local_streams_iem;
        stream_feature_configure_resolution((stream_feature_list_ptr_t)g_usb_in_out_local_streams_iem.pfeature_table, RESOLUTION_32BIT, CONFIG_DECODER);
        type = VIRTUAL_SOURCE_WIRED_AUDIO_USB_OUT;
    } else if (open_param->audio_scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_MAINSTREAM) {
        channel_num = 2;
        samplingrate = open_param->stream_out_param.afe.sampling_rate/1000;
        data_size = open_param->stream_out_param.afe.frame_size * ((open_param->stream_out_param.afe.format > HAL_AUDIO_PCM_FORMAT_U16_BE)?4:2);
        extern CONNECTION_IF g_wired_audio_main_stream;
        stream_feature_configure_resolution((stream_feature_list_ptr_t)g_wired_audio_main_stream.pfeature_table, RESOLUTION_32BIT, CONFIG_DECODER);
        type = VIRTUAL_SOURCE_ZERO_DATA;
    }
#endif
    source = StreamVirtualSource(NULL, type, open_param->stream_in_param.virtual_param.virtual_mem_size, channel_num, data_size, samplingrate);
    if (source != NULL) {
#if defined(AIR_WIRED_AUDIO_ENABLE)
        if (open_param->audio_scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_OUT_IEM){
            wired_audio_usb_in_out_source_init(source);
        } else if (open_param->audio_scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_MAINSTREAM) {
            wired_audio_main_stream_source_init(source);
        }
#endif
    } else {
        DSP_MW_LOG_E("DSP virtual source create fail\r\n", 0);
    }


    return source;

}

const dsp_stream_open_in_entry_table_t dsp_stream_open_in_entry_table[] = {
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    {STREAM_IN_AFE, dsp_open_stream_in_afe},
#endif
#ifdef AIR_BT_HFP_ENABLE
    {STREAM_IN_HFP, dsp_open_stream_in_hfp},
#endif
#ifdef AIR_BT_CODEC_BLE_ENABLED
    {STREAM_IN_BLE, dsp_open_stream_in_ble},
#endif
#ifdef MTK_BT_A2DP_ENABLE
    {STREAM_IN_A2DP, dsp_open_stream_in_a2dp},
#endif
    {STREAM_IN_PLAYBACK, dsp_open_stream_in_playback},
    {STREAM_IN_VP, dsp_open_stream_in_vp},
#ifdef MTK_PROMPT_SOUND_DUMMY_SOURCE_ENABLE
    {STREAM_IN_VP_DUMMY_SOURCE, dsp_open_stream_in_vp_dummy_source},
#endif
#ifdef MTK_SENSOR_SOURCE_ENABLE
    {STREAM_IN_GSENSOR, dsp_open_stream_in_gsensor},
#endif
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
    {STREAM_IN_AUDIO_TRANSMITTER, dsp_open_stream_in_audio_transmitter},
#endif /* AIR_AUDIO_TRANSMITTER_ENABLE */
#ifdef AIR_AUDIO_BT_COMMON_ENABLE
    {STREAM_IN_BT_COMMON, dsp_open_stream_in_bt_common},
#endif /* AIR_AUDIO_BT_COMMON_ENABLE */
#ifdef AIR_FULL_ADAPTIVE_ANC_ENABLE
    {STREAM_IN_ADAPT_ANC, dsp_open_stream_in_adapt_anc},
#endif
#if defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
    {STREAM_IN_LLF, dsp_open_stream_in_LLF},
#endif
#ifdef AIR_HW_VIVID_PT_ENABLE
    {STREAM_IN_HW_VIVID_PT, dsp_open_stream_in_hw_vivid_pt},
#endif
    {STREAM_IN_VIRTUAL, dsp_open_stream_in_virtual},
#ifdef AIR_MIXER_STREAM_ENABLE
    {STREAM_IN_MIXER, dsp_open_stream_in_mixer},
#endif
};
SOURCE dsp_open_stream_in(mcu2dsp_open_param_p open_param)
{
    SOURCE source = NULL;
    uint32_t number, i;
    if (open_param != NULL) {
#ifdef AIR_AUDIO_HARDWARE_ENABLE
        AudioAfeConfiguration(AUDIO_SOURCE_SCENARIO_TYPE, open_param->audio_scenario_type);
#endif
        number = sizeof(dsp_stream_open_in_entry_table)/sizeof(dsp_stream_open_in_entry_table_t);
        for (i = 0 ; i < number ; i++) {
            if (dsp_stream_open_in_entry_table[i].stream_in == open_param->param.stream_in) {
                source = dsp_stream_open_in_entry_table[i].entry(open_param);
                break;
        }
    }
    }
    if (source) {
        source->scenario_type= open_param->audio_scenario_type;
    } else {
        DSP_MW_LOG_E("DSP source create fail\r\n", 0);
    }
    return source;
}

#ifdef AIR_BT_HFP_ENABLE
CONNECTION_IF n9_sco_ul_if;

SINK dsp_open_stream_out_hfp(mcu2dsp_open_param_p open_param)
{
    SINK sink;
    DSP_MW_LOG_I("Stream out hfp Codec:%d\r\n", 1, open_param->stream_out_param.hfp.codec_type);

    if (open_param->stream_out_param.hfp.codec_type == BT_HFP_CODEC_CVSD) {
        DSP_ALG_UpdateEscoTxMode(VOICE_NB);
        /* Downsample from 16K to 8K, and encode in controller side */
#ifdef AIR_DCHS_MODE_ENABLE
        if(dchs_get_device_mode() == DCHS_MODE_SINGLE){
            stream_feature_configure_type(stream_feature_list_hfp_uplink_dchs, FUNC_SRC_FIXED_RATIO, CONFIG_ENCODER_REPLACE);
        }else{
            stream_feature_configure_type(stream_feature_list_hfp_uplink, FUNC_SRC_FIXED_RATIO, CONFIG_ENCODER_REPLACE);
        }
#else
        stream_feature_configure_type(stream_feature_list_hfp_uplink, FUNC_SRC_FIXED_RATIO, CONFIG_ENCODER_REPLACE);
#endif
    } else if (open_param->stream_out_param.hfp.codec_type  == BT_HFP_CODEC_mSBC) {
        DSP_ALG_UpdateEscoTxMode(VOICE_WB);
#ifdef AIR_DCHS_MODE_ENABLE
        if(dchs_get_device_mode() == DCHS_MODE_SINGLE){
            stream_feature_configure_type(stream_feature_list_hfp_uplink_dchs, CODEC_ENCODER_MSBC, CONFIG_ENCODER_REPLACE);
        }else{
            stream_feature_configure_type(stream_feature_list_hfp_uplink, CODEC_ENCODER_MSBC, CONFIG_ENCODER_REPLACE);
        }
#else
        stream_feature_configure_type(stream_feature_list_hfp_uplink, CODEC_ENCODER_MSBC, CONFIG_ENCODER_REPLACE);
#endif
    } else {
        //not support codec type
    }
#ifdef AIR_DCHS_MODE_ENABLE
    if(dchs_get_device_mode() == DCHS_MODE_SINGLE){
        n9_sco_ul_if.pfeature_table = (stream_feature_list_ptr_t)&stream_feature_list_hfp_uplink_dchs;
    }else{
        n9_sco_ul_if.pfeature_table = (stream_feature_list_ptr_t)&stream_feature_list_hfp_uplink;
    }
#else
    n9_sco_ul_if.pfeature_table = (stream_feature_list_ptr_t)&stream_feature_list_hfp_uplink;
#endif

    sink = StreamN9ScoSink(open_param->stream_out_param.hfp.p_share_info);

    return sink;
}
#endif

#ifdef AIR_WIRELESS_MIC_TX_ENABLE
#define WIRELESS_MIC_TX_RESERVED_PROCESS_TIME          2000//2000us for 208MHz; 3ms for 104MHz
#ifdef AIR_LD_NR_ENABLE
    #include "ld_nr_interface.h"
    void *p_wireless_mic_ld_nr_key;
#endif
#if defined(AIR_AUDIO_VOLUME_MONITOR_ENABLE)
#include "volume_estimator_interface.h"
#include "stream_nvkey_struct.h"
#include "preloader_pisplit.h"
#include "audio_nvdm_common.h"
void *p_wireless_mic_tx_volume_monitor_nvkey_buf = NULL;
void *p_wireless_mic_tx_volume_monitor_queue_buf = NULL;
#endif
#endif

#ifdef AIR_BT_CODEC_BLE_ENABLED
SINK dsp_open_stream_out_ble(mcu2dsp_open_param_p open_param)
{
    SINK sink;

    DSP_MW_LOG_I("[BLE] Stream out ble, Codec:%d", 1, open_param->stream_out_param.ble.codec_type);
#ifdef AIR_BT_LE_LC3_ENABLE
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    DspMemoryManagerReturnStatus_t memory_return_status;
    memset(&lc3_plc_memory_addr, 0, sizeof(GroupMemoryInfo_t));
    memset(&lc3_fft_memory_addr, 0, sizeof(GroupMemoryInfo_t));
#endif
    void *code_addr_fft = NULL;
    void *data_addr_fft = NULL;
    uint32_t pic_dram_usage = 0;
#endif

    /*  Tx Rx mode must be aligned otherwise it would cause NVkey assert*/
    /*  Currently Rx mode dominates the initialization process*/
#ifdef AIR_BT_BLE_SWB_ENABLE
    DSP_ALG_UpdateEscoTxMode(VOICE_SWB);         /*32K sample rate. Need SWB algorithm*/
    DSP_ALG_UpdateEscoRxMode(VOICE_SWB);         /*32K sample rate. Need SWB algorithm*/
#else
    DSP_ALG_UpdateEscoTxMode(VOICE_WB);         /*16K sample rate. Need WB algorithm*/
    DSP_ALG_UpdateEscoRxMode(VOICE_WB);         /*16K sample rate. Need WB algorithm*/
#endif

    if (open_param->stream_out_param.ble.context_type == BLE_CONTENT_TYPE_ULL_BLE)
    {
        if (open_param->stream_out_param.ble.codec_type == BT_BLE_CODEC_LC3PLUS)
        {
#ifdef AIR_BT_LE_LC3PLUS_ENABLE
            ULL_NrOffloadFlag = open_param->stream_out_param.ble.nr_offload_flag;
#ifdef AIR_ECNR_PREV_PART_ENABLE
#ifndef AIR_ECNR_SEPARATE_MODE_ENABLE
            voice_ecnr_ec_config_period(5000); /* config the EC to work with 5ms */
#endif
#endif
            n9_ble_ul_if.pfeature_table = (ULL_NrOffloadFlag) ? AudioFeatureList_ULL_BLE_UL_NR_OFFLOAD : AudioFeatureList_ULL_BLE_UL;
            stream_feature_configure_type(n9_ble_ul_if.pfeature_table, CODEC_ENCODER_LC3PLUS, CONFIG_ENCODER);
            #if defined(AIR_BT_ULL_SWB_ENABLE)
            DSP_ALG_UpdateEscoTxMode(VOICE_SWB);         /*32K sample rate. Need SWB algorithm*/
            DSP_ALG_UpdateEscoRxMode(VOICE_SWB);         /*32K sample rate. Need SWB algorithm*/
            #else
            DSP_ALG_UpdateEscoTxMode(VOICE_WB);         /*16K sample rate. Need WB algorithm*/
            DSP_ALG_UpdateEscoRxMode(VOICE_WB);         /*16K sample rate. Need WB algorithm*/
            #endif
            #ifdef AIR_BT_ULL_FB_ENABLE
            if (open_param->stream_in_param.afe.sampling_rate == 48000) {
                DSP_ALG_UpdateEscoTxMode(VOICE_FB);         /*48K sample rate. Need FB algorithm*/
                DSP_ALG_UpdateEscoRxMode(VOICE_FB);         /*48K sample rate. Need FB algorithm*/
                stream_feature_configure_resolution((stream_feature_list_ptr_t)n9_ble_ul_if.pfeature_table, RESOLUTION_32BIT, CONFIG_DECODER);
                DSP_MW_LOG_I("[BLE] FullBand sr: %d, set 32bit", 1, open_param->stream_in_param.afe.sampling_rate);
            }
            #endif
#endif
        }
#ifdef AIR_CELT_ENC_V2_ENABLE
        else if (open_param->stream_out_param.ble.codec_type == BT_BLE_CODEC_VENDOR)
        {
            n9_ble_ul_if.pfeature_table = AudioFeatureList_ULL_BLE_UL;
            stream_feature_configure_type(n9_ble_ul_if.pfeature_table, CODEC_ENCODER_OPUS_V2, CONFIG_ENCODER);
            DSP_ALG_UpdateEscoTxMode(VOICE_WB);         /*16K sample rate. Need WB algorithm*/
            DSP_ALG_UpdateEscoRxMode(VOICE_WB);         /*16K sample rate. Need WB algorithm*/
        }
#endif
#ifdef AIR_WIRELESS_MIC_TX_ENABLE
    } else if (open_param->stream_out_param.ble.context_type == BLE_CONTENT_TYPE_WIRELESS_MIC) {
        uint16_t channel_num = 1;
        if (n9_ble_ul_if.source->param.audio.channel_num >= open_param->stream_out_param.ble.channel_num){
            n9_ble_ul_if.pfeature_table = AudioFeatureList_WirelessMic_chsel_in_front;
            channel_num = open_param->stream_out_param.ble.channel_num;
        } else {
            n9_ble_ul_if.pfeature_table = AudioFeatureList_WirelessMic_chsel_in_back;
            channel_num = n9_ble_ul_if.source->param.audio.channel_num;
        }
        if (open_param->stream_out_param.ble.codec_type == BT_BLE_CODEC_ULD) {
            stream_feature_configure_type(n9_ble_ul_if.pfeature_table, CODEC_ENCODER_ULD, CONFIG_ENCODER);
        } else if (open_param->stream_out_param.ble.codec_type == BT_BLE_CODEC_LC3PLUS) {
#ifdef AIR_BT_LE_LC3PLUS_ENABLE
            stream_feature_configure_type(n9_ble_ul_if.pfeature_table, CODEC_ENCODER_LC3PLUS, CONFIG_ENCODER);
#endif
        }
        stream_feature_configure_resolution((stream_feature_list_ptr_t)n9_ble_ul_if.pfeature_table, RESOLUTION_32BIT, CONFIG_DECODER);
#ifdef AIR_AUDIO_ULD_ENCODE_ENABLE
        uld_enc_port_config_t uld_enc_config;
        uld_enc_port_t *uld_port = stream_codec_encoder_uld_get_port(n9_ble_ul_if.source);
        AUDIO_ASSERT(uld_port != NULL);
        uld_enc_config.bit_rate = 200000;
        uld_enc_config.interval = open_param->stream_out_param.ble.frame_ms;
        uld_enc_config.in_channel_num = open_param->stream_out_param.ble.channel_num;
        uld_enc_config.resolution = RESOLUTION_32BIT;
        uld_enc_config.sample_rate = n9_ble_ul_if.source->param.audio.rate;
        stream_codec_encoder_uld_init(uld_port, &uld_enc_config);
#endif /* AIR_AUDIO_ULD_ENCODE_ENABLE */
#ifdef AIR_LD_NR_ENABLE
        ld_nr_port_t *ld_nr_port;
        ld_nr_config_t ld_nr_config;
        ld_nr_port = stream_function_ld_nr_get_port(n9_ble_ul_if.source);
        ld_nr_config.channel_num = channel_num;
        ld_nr_config.frame_size  = open_param->stream_out_param.ble.frame_ms * open_param->stream_out_param.ble.sampling_frequency /1000 / 1000 * 4;//RESOLUTION_32BIT;
        ld_nr_config.resolution  = RESOLUTION_32BIT;
        ld_nr_config.sample_rate = n9_ble_ul_if.source->param.audio.rate;

        if (p_wireless_mic_ld_nr_key == NULL) {
            configASSERT(0 && "ld_dr key = null");
        }

        ld_nr_config.nvkey_para = p_wireless_mic_ld_nr_key;
        ld_nr_config.background_process_enable = true;
        ld_nr_config.background_process_fr_num = 2;
        stream_function_ld_nr_init(ld_nr_port, &ld_nr_config);
        DSP_MW_LOG_I("[LD_NR]p_wireless_mic_ld_nr_key 0x%x channel_num=%d, resolution:%d", 3, p_wireless_mic_ld_nr_key, n9_ble_ul_if.source->param.audio.channel_num, RESOLUTION_32BIT);
#endif /* AIR_LD_NR_ENABLE */
#ifdef AIR_SOFTWARE_DRC_ENABLE
    /* sw drc init */
    sw_compander_config_t drc_config;
    drc_config.mode = SW_COMPANDER_AUDIO_MODE;
    drc_config.channel_num = channel_num;
    drc_config.sample_rate = n9_ble_ul_if.source->param.audio.rate;
    drc_config.frame_base = 8;
    drc_config.recovery_gain = 4; /* 0dB */
    drc_config.vol_default_gain = 0x08000000; /* 0dB */
    drc_config.default_nvkey_mem = NULL;
    drc_config.default_nvkey_id = NVKEY_DSP_PARA_MIC_AU_CPD;
    sw_compander_port_t *drc_port = stream_function_sw_compander_get_port(n9_ble_ul_if.source);
    stream_function_sw_compander_init(drc_port, &drc_config);
    DSP_MW_LOG_I("[wireless_mic]sw drc 0x%x info, %d, %d, %d, %d, 0x%x, 0x%x, 0x%x\r\n", 11,
                drc_port,
                drc_config.mode,
                drc_config.channel_num,
                drc_config.sample_rate,
                drc_config.frame_base,
                drc_config.recovery_gain,
                drc_config.vol_default_gain,
                drc_config.default_nvkey_id);
#endif
#if defined(AIR_AUDIO_VOLUME_MONITOR_ENABLE)
        /* init volume estimator port */
        p_wireless_mic_tx_volume_monitor_nvkey_buf = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, sizeof(audio_spectrum_meter_nvkey_t));
        if (p_wireless_mic_tx_volume_monitor_nvkey_buf == NULL) {
            AUDIO_ASSERT(0);
        }
        if (nvkey_read_full_key(NVKEY_DSP_PARA_SILENCE_DETECTION2, p_wireless_mic_tx_volume_monitor_nvkey_buf, sizeof(audio_spectrum_meter_nvkey_t)) != NVDM_STATUS_NAT_OK) {
            AUDIO_ASSERT(0);
        }
        volume_estimator_config_t config;
        volume_estimator_port_t *meter_port = volume_estimator_get_port(n9_ble_ul_if.source);
        if (meter_port == NULL) {
            AUDIO_ASSERT(0 && "[ULL Audio V2][DL] Audio Spectrum memter port is null.");
        }
        config.resolution = RESOLUTION_32BIT;
        config.frame_size = /*open_param->stream_out_param.ble.frame_ms*/2500 * open_param->stream_out_param.ble.sampling_frequency /1000 / 1000 * 4;//RESOLUTION_32BIT;
        config.channel_num = 1;
        config.mode = VOLUME_ESTIMATOR_CHAT_INSTANT_MODE;
        config.sample_rate = n9_ble_ul_if.source->param.audio.rate;
        config.nvkey_para = (void *)&(((audio_spectrum_meter_nvkey_t *)p_wireless_mic_tx_volume_monitor_nvkey_buf)->chat_vol_nvkey);
        config.internal_buffer = NULL;
        config.internal_buffer_size = 0;
        if (open_param->stream_out_param.ble.codec_type == BT_BLE_CODEC_ULD) {
            config.internal_buffer_size = open_param->stream_out_param.ble.frame_ms * open_param->stream_out_param.ble.sampling_frequency /1000 / 1000 * 4 * 5;
            p_wireless_mic_tx_volume_monitor_queue_buf = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, config.internal_buffer_size);
            config.internal_buffer = p_wireless_mic_tx_volume_monitor_queue_buf;
            if (p_wireless_mic_tx_volume_monitor_queue_buf == NULL) {
                AUDIO_ASSERT(0);
            }
        }
        volume_estimator_init(meter_port, &config);
        DSP_MW_LOG_I("[wireless_mic]volume estimator 0x%x info, %d, %d, %d,internal_buffer 0x%x, 0x%x\r\n", 6,
                    meter_port,
                    config.frame_size,
                    config.channel_num,
                    config.sample_rate,
                    config.internal_buffer,
                    config.internal_buffer_size);
#endif
#endif
    } else {
    if (open_param->stream_out_param.ble.codec_type == BT_BLE_CODEC_LC3) {
            #ifdef AIR_BT_LE_LC3_ENABLE
            n9_ble_ul_if.pfeature_table = AudioFeatureList_BLE_Call_UL;
        stream_feature_configure_type(n9_ble_ul_if.pfeature_table, CODEC_ENCODER_LC3, CONFIG_ENCODER);
#ifdef PRELOADER_ENABLE
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
        memory_return_status = DspMemoryManager_AcquireGroupMemory(Component_LE_CALL,SubComponent_LECALL_LC3_PLC_STD,&lc3_plc_memory_addr);
        code_addr_plc = lc3_plc_memory_addr.IramAddr;
        data_addr_plc = lc3_plc_memory_addr.DramAddr;
        memory_return_status = DspMemoryManager_AcquireGroupMemory(Component_LE_CALL,SubComponent_LECALL_LC3_FFT,&lc3_fft_memory_addr);
        code_addr_fft = lc3_fft_memory_addr.IramAddr;
        data_addr_fft = lc3_fft_memory_addr.DramAddr;
#endif

        if(open_param->stream_out_param.ble.frame_ms == 10000){
            lc3i_fft10ms_library_load(code_addr_fft,data_addr_fft,&pic_dram_usage);
        }else if(open_param->stream_out_param.ble.frame_ms == 7500){
            lc3i_fft7_5ms_library_load(code_addr_fft,data_addr_fft,&pic_dram_usage);
        }else{
            DSP_MW_LOG_E("[BLE] unsupported UL frame interval %d",1,open_param->stream_out_param.ble.frame_ms);
        }
#endif
#endif
    }
    }

    sink = StreamN9BleSink(open_param);

    return sink;
}
#endif

#ifdef AIR_AUDIO_HARDWARE_ENABLE
SINK dsp_open_stream_out_afe(mcu2dsp_open_param_p open_param)
{
    SINK sink = NULL;

    //[TEMP]: Add AT Cmd to switch I2S mode
    if (((*((volatile uint32_t *)(0xA2120B04)) >> 2) & 0x01) == 1) {
        open_param->stream_out_param.afe.audio_device = HAL_AUDIO_DEVICE_SPDIF;//HAL_AUDIO_DEVICE_I2S_MASTER;
        open_param->stream_out_param.afe.audio_interface = HAL_AUDIO_INTERFACE_1;
    }

    //DSP_MW_LOG_I("Stream out afe\r\n", 1);
#ifdef ENABLE_HWSRC_CLKSKEW
    DSP_MW_LOG_I("afe out device:%d, channel:%d, memory:%d, interface:%d, hw_gain:%d, adc_mode:%d, performance:%d,clkskew_mode:%d \r\n", 8, open_param->stream_out_param.afe.audio_device,
                 open_param->stream_out_param.afe.stream_channel,
                 open_param->stream_out_param.afe.memory,
                 open_param->stream_out_param.afe.audio_interface,
                 open_param->stream_out_param.afe.hw_gain,
                 open_param->stream_out_param.afe.adc_mode,
                 open_param->stream_out_param.afe.performance,
                 open_param->stream_out_param.afe.clkskew_mode);
#else
    DSP_MW_LOG_I("afe out device:%d, channel:%d, memory:%d, interface:%d, hw_gain:%d, adc_mode:%d, performance:%d \r\n", 8, open_param->stream_out_param.afe.audio_device,
                 open_param->stream_out_param.afe.stream_channel,
                 open_param->stream_out_param.afe.memory,
                 open_param->stream_out_param.afe.audio_interface,
                 open_param->stream_out_param.afe.hw_gain,
                 open_param->stream_out_param.afe.adc_mode,
                 open_param->stream_out_param.afe.performance);
#endif

    AudioAfeConfiguration(AUDIO_SINK_DATA_FORMAT, open_param->stream_out_param.afe.format);
    if (open_param->stream_out_param.afe.sampling_rate != 0) {
        AudioAfeConfiguration(AUDIO_SINK_IRQ_RATE, open_param->stream_out_param.afe.sampling_rate);
    }

#if defined(ENABLE_HWSRC_ON_MAIN_STREAM) || defined(MTK_HWSRC_IN_STREAM)
    AudioAfeConfiguration(AUDIO_SRC_RATE, open_param->stream_out_param.afe.stream_out_sampling_rate);
    AudioAfeConfiguration(AUDIO_SINK_HWSRC_TYPE, open_param->stream_out_param.afe.hwsrc_type);
#endif
    AudioAfeConfiguration(AUDIO_SINK_IRQ_PERIOD, open_param->stream_out_param.afe.irq_period);

    AudioAfeConfiguration(AUDIO_SINK_DEVICE, open_param->stream_out_param.afe.audio_device);
    AudioAfeConfiguration(AUDIO_SINK_DEVICE1, open_param->stream_out_param.afe.audio_device1);
    AudioAfeConfiguration(AUDIO_SINK_DEVICE2, open_param->stream_out_param.afe.audio_device2);
    AudioAfeConfiguration(AUDIO_SINK_CHANNEL, open_param->stream_out_param.afe.stream_channel);
    AudioAfeConfiguration(AUDIO_SINK_MEMORY, open_param->stream_out_param.afe.memory);
    AudioAfeConfiguration(AUDIO_SINK_INTERFACE, open_param->stream_out_param.afe.audio_interface);
    AudioAfeConfiguration(AUDIO_SINK_INTERFACE1, open_param->stream_out_param.afe.audio_interface1);
    AudioAfeConfiguration(AUDIO_SINK_INTERFACE2, open_param->stream_out_param.afe.audio_interface2);
    AudioAfeConfiguration(AUDIO_SINK_HW_GAIN, open_param->stream_out_param.afe.hw_gain);

    AudioAfeConfiguration(AUDIO_SINK_CLKSKEW_MODE, open_param->stream_out_param.afe.clkskew_mode);
    DSP_MW_LOG_I("afe sink clkskew_mode:%d", 1, open_param->stream_out_param.afe.clkskew_mode);

#ifdef HAL_AUDIO_ENABLE_PATH_MEM_DEVICE
    AudioAfeConfiguration(AUDIO_SINK_ADC_MODE, open_param->stream_out_param.afe.dl_dac_mode);
    if ((open_param->stream_out_param.afe.audio_device & (HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER | HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_L | HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_R))
        ||open_param->stream_out_param.afe.audio_device1 & (HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER | HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_L | HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_R)) {
        AudioAfeConfiguration(AUDIO_SINK_I2S_MASTER_SAMPLING_RATE, open_param->stream_out_param.afe.i2s_master_sampling_rate[0]);
        AudioAfeConfiguration(AUDIO_SINK_I2S_MASTER_SAMPLING_RATE1, open_param->stream_out_param.afe.i2s_master_sampling_rate[1]);
        AudioAfeConfiguration(AUDIO_SINK_I2S_MASTER_SAMPLING_RATE2, open_param->stream_out_param.afe.i2s_master_sampling_rate[2]);
        AudioAfeConfiguration(AUDIO_SINK_I2S_MASTER_SAMPLING_RATE3, open_param->stream_out_param.afe.i2s_master_sampling_rate[3]);
        AudioAfeConfiguration(AUDIO_SINK_I2S_MASTER_FORMAT, open_param->stream_out_param.afe.i2s_master_format[0]);
        AudioAfeConfiguration(AUDIO_SINK_I2S_MASTER_FORMAT1, open_param->stream_out_param.afe.i2s_master_format[1]);
        AudioAfeConfiguration(AUDIO_SINK_I2S_MASTER_FORMAT2, open_param->stream_out_param.afe.i2s_master_format[2]);
        AudioAfeConfiguration(AUDIO_SINK_I2S_MASTER_FORMAT3, open_param->stream_out_param.afe.i2s_master_format[3]);
        AudioAfeConfiguration(AUDIO_SINK_I2S_MASTER_WORD_LENGTH, open_param->stream_out_param.afe.i2s_master_word_length[0]);
        AudioAfeConfiguration(AUDIO_SINK_I2S_MASTER_WORD_LENGTH1, open_param->stream_out_param.afe.i2s_master_word_length[1]);
        AudioAfeConfiguration(AUDIO_SINK_I2S_MASTER_WORD_LENGTH2, open_param->stream_out_param.afe.i2s_master_word_length[2]);
        AudioAfeConfiguration(AUDIO_SINK_I2S_MASTER_WORD_LENGTH3, open_param->stream_out_param.afe.i2s_master_word_length[3]);
        AudioAfeConfiguration(AUDIO_SINK_I2S_MASTER_LOW_JITTER, open_param->stream_out_param.afe.is_low_jitter[0]);
        AudioAfeConfiguration(AUDIO_SINK_I2S_MASTER_LOW_JITTER1, open_param->stream_out_param.afe.is_low_jitter[1]);
        AudioAfeConfiguration(AUDIO_SINK_I2S_MASTER_LOW_JITTER2, open_param->stream_out_param.afe.is_low_jitter[2]);
        AudioAfeConfiguration(AUDIO_SINK_I2S_MASTER_LOW_JITTER3, open_param->stream_out_param.afe.is_low_jitter[3]);

        for (U32 i = 0 ; i < sizeof(open_param->stream_out_param.afe.i2s_master_sampling_rate) / sizeof(open_param->stream_out_param.afe.i2s_master_sampling_rate[0]) ; ++i) {
            DSP_MW_LOG_I("i2s master configuration[%d]=%d,%d,%d,%d", 5,
                i,
                open_param->stream_out_param.afe.i2s_master_sampling_rate[i],
                open_param->stream_out_param.afe.i2s_master_format[i],
                open_param->stream_out_param.afe.i2s_master_word_length[i],
                open_param->stream_out_param.afe.is_low_jitter[i]);
        }
    } else if ((open_param->stream_out_param.afe.audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE)
            ||(open_param->stream_out_param.afe.audio_device1 == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE)) {
        AudioAfeConfiguration(AUDIO_SINK_I2S_FORMAT, open_param->stream_out_param.afe.i2s_format);
        AudioAfeConfiguration(AUDIO_SINK_I2S_SLAVE_TDM, open_param->stream_out_param.afe.i2S_Slave_TDM);
        AudioAfeConfiguration(AUDIO_SINK_I2S_WORD_LENGTH, open_param->stream_out_param.afe.i2s_word_length);
    }

    AudioAfeConfiguration(AUDIO_SINK_DAC_PERFORMANCE, open_param->stream_out_param.afe.performance);
#endif
#ifdef AIR_LINE_IN_MIX_ENABLE
if((open_param->stream_out_param.afe.audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER)
    || (open_param->stream_out_param.afe.audio_device1 == HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER)) {
    AudioAfeConfiguration(AUDIO_SINK_I2S_MASTER_FORMAT, open_param->stream_out_param.afe.i2s_master_format[0]);
    AudioAfeConfiguration(AUDIO_SINK_I2S_MASTER_WORD_LENGTH, open_param->stream_out_param.afe.i2s_master_word_length[0]);
    AudioAfeConfiguration(AUDIO_SINK_I2S_MASTER_LOW_JITTER, open_param->stream_out_param.afe.is_low_jitter[0]);
}
#endif
#if AUTO_ERROR_SUPPRESSION
    AudioAfeConfiguration(AUDIO_SINK_MISC_PARMS_I2S_CLK, open_param->stream_out_param.afe.misc_parms.I2sClkSourceType);
    AudioAfeConfiguration(AUDIO_SINK_MISC_PARMS_MICBIAS, open_param->stream_out_param.afe.misc_parms.MicbiasSourceType);
#endif
    AudioAfeConfiguration(AUDIO_SINK_ECHO_REFERENCE, false);
#ifdef AIR_HFP_DNN_PATH_ENABLE
    AudioAfeConfiguration(AUDIO_SINK_DNN_PATH_ENABLE, open_param->stream_out_param.afe.enable_ul_dnn);
#endif
    DSP_MW_LOG_I("afe out format:%d, src out rate:%d, IRQ period:%d src in rate:%d\r\n", 4, open_param->stream_out_param.afe.format,
                 gAudioCtrl.Afe.AfeDLSetting.rate,
                 (uint32_t)(open_param->stream_out_param.afe.irq_period), // can't print float
                 gAudioCtrl.Afe.AfeDLSetting.src_rate);


#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
    if (open_param->stream_out_param.afe.memory == HAL_AUDIO_MEM1 || open_param->stream_out_param.afe.memory == HAL_AUDIO_MEM6) {
#else
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
        if (((open_param->audio_scenario_type != AUDIO_SCENARIO_TYPE_WIRED_AUDIO_MAINSTREAM) && (open_param->stream_out_param.afe.memory & HAL_AUDIO_MEM1)) || open_param->stream_out_param.afe.memory == HAL_AUDIO_MEM6 || open_param->stream_out_param.afe.memory == HAL_AUDIO_MEM7) {
#else
    if (open_param->stream_out_param.afe.memory == HAL_AUDIO_MEM1 || open_param->stream_out_param.afe.memory == HAL_AUDIO_MEM6 || open_param->stream_out_param.afe.memory == HAL_AUDIO_MEM7) {
#endif
#endif
        AudioAfeConfiguration(AUDIO_SINK_SW_CHANNELS, open_param->stream_out_param.afe.sw_channels);
        Audio_setting->Audio_sink.Buffer_Frame_Num = open_param->stream_out_param.afe.frame_number;
        Audio_setting->Audio_sink.Frame_Size       = open_param->stream_out_param.afe.frame_size;
        sink = StreamAudioAfeSink(afe_get_audio_hardware_by_au_afe_open_param(&(open_param->stream_out_param.afe)),
                                  afe_get_audio_instance_by_au_afe_open_param(&(open_param->stream_out_param.afe)),
                                  afe_get_audio_channel_by_au_afe_open_param(&(open_param->stream_out_param.afe)));

    } else if (open_param->stream_out_param.afe.memory == HAL_AUDIO_MEM2) {
        Audio_setting->Audio_VP.Buffer_Frame_Num = open_param->stream_out_param.afe.frame_number;
        Audio_setting->Audio_VP.Frame_Size = open_param->stream_out_param.afe.frame_size;
        // VP/RT memory path DL2_data
#ifdef MTK_PROMPT_SOUND_ENABLE
        sink = StreamAudioAfe2Sink(afe_get_audio_hardware_by_au_afe_open_param(&(open_param->stream_out_param.afe)),
                                   afe_get_audio_instance_by_au_afe_open_param(&(open_param->stream_out_param.afe)),
                                   afe_get_audio_channel_by_au_afe_open_param(&(open_param->stream_out_param.afe)));
#endif
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
    } else if (open_param->stream_out_param.afe.memory == HAL_AUDIO_MEM7) {
        //Tdm-Sink
        AudioAfeConfiguration(AUDIO_SINK_SW_CHANNELS, open_param->stream_out_param.afe.sw_channels);
        Audio_setting->Audio_sink.Buffer_Frame_Num = open_param->stream_out_param.afe.frame_number;
        Audio_setting->Audio_sink.Frame_Size       = open_param->stream_out_param.afe.frame_size;
        sink = StreamAudioAfeTdmSink(afe_get_audio_hardware_by_au_afe_open_param(&(open_param->stream_out_param.afe)),
                                     afe_get_audio_instance_by_au_afe_open_param(&(open_param->stream_out_param.afe)),
                                     afe_get_audio_channel_by_au_afe_open_param(&(open_param->stream_out_param.afe)));
#endif
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
    } else if (open_param->stream_out_param.afe.memory & HAL_AUDIO_MEM3) {
#else
    } else if (open_param->stream_out_param.afe.memory == HAL_AUDIO_MEM3) {
#endif
#if defined (AIR_WIRED_AUDIO_ENABLE) || defined (AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined (AIR_PROMPT_SOUND_DUMMY_SOURCE_ENABLE) || defined(AIR_HFP_DNN_PATH_ENABLE) || defined(AIR_DCHS_MODE_ENABLE) || defined(AIR_WIRELESS_MIC_RX_ENABLE) || defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
        AudioAfeConfiguration(AUDIO_SINK_SW_CHANNELS, open_param->stream_out_param.afe.sw_channels);
        Audio_setting->Audio_sink.Buffer_Frame_Num = open_param->stream_out_param.afe.frame_number;
        Audio_setting->Audio_sink.Frame_Size       = open_param->stream_out_param.afe.frame_size;
        sink = StreamAudioAfe3Sink(afe_get_audio_hardware_by_au_afe_open_param(&(open_param->stream_out_param.afe)),
                                   afe_get_audio_instance_by_au_afe_open_param(&(open_param->stream_out_param.afe)),
                                   afe_get_audio_channel_by_au_afe_open_param(&(open_param->stream_out_param.afe)));
#endif
    }
#if defined (AIR_WIRED_AUDIO_ENABLE) || defined (AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined (AIR_DCHS_MODE_ENABLE) || defined(AIR_MIXER_STREAM_ENABLE)
    else if (open_param->stream_out_param.afe.memory == HAL_AUDIO_MEM4) {
        AudioAfeConfiguration(AUDIO_SINK_SW_CHANNELS, open_param->stream_out_param.afe.sw_channels);
        Audio_setting->Audio_sink.Buffer_Frame_Num = open_param->stream_out_param.afe.frame_number;
        Audio_setting->Audio_sink.Frame_Size       = open_param->stream_out_param.afe.frame_size;
        sink = StreamAudioAfe12Sink(afe_get_audio_hardware_by_au_afe_open_param(&(open_param->stream_out_param.afe)),
                                    afe_get_audio_instance_by_au_afe_open_param(&(open_param->stream_out_param.afe)),
                                    afe_get_audio_channel_by_au_afe_open_param(&(open_param->stream_out_param.afe)));
    }
#endif /* AIR_ADVANCED_PASSTHROUGH_ENABLE */

    return sink;

}
#endif /* AIR_AUDIO_HARDWARE_ENABLE */

SINK dsp_open_stream_out_record(mcu2dsp_open_param_p open_param)
{
    SINK sink;
    DSP_MW_LOG_I("Stream out record\r\n", 0);

    sink = StreamCm4RecordSink(&(open_param->stream_out_param.record));

    return sink;
}

SINK dsp_open_stream_out_virtual(mcu2dsp_open_param_p open_param)
{
    SINK sink;
    DSP_MW_LOG_I("Stream out virtual\r\n", 0);
    UNUSED(open_param);
    sink = StreamVirtualSink(NULL, NULL);

    return sink;
}

#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
SINK dsp_open_stream_out_audio_transmitter(mcu2dsp_open_param_p open_param)
{
    SINK sink;

    sink = StreamAudioTransmitterSink(&(open_param->stream_out_param.data_ul));

    return sink;
}
#endif /* AIR_AUDIO_TRANSMITTER_ENABLE */

#ifdef AIR_AUDIO_BT_COMMON_ENABLE
SINK dsp_open_stream_out_bt_common(mcu2dsp_open_param_p open_param)
{
    SINK sink;

    DSP_MW_LOG_I("Stream out bt common\r\n", 0);

    sink = StreamBTCommonSink(&(open_param->stream_out_param.bt_common));

    return sink;
}
#endif /* AIR_AUDIO_BT_COMMON_ENABLE */


const dsp_stream_open_out_entry_table_t dsp_stream_open_out_entry_table[] = {
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    {STREAM_OUT_AFE, dsp_open_stream_out_afe},
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
#ifdef AIR_BT_HFP_ENABLE
    {STREAM_OUT_HFP, dsp_open_stream_out_hfp},
#endif
#ifdef AIR_BT_CODEC_BLE_ENABLED
    {STREAM_OUT_BLE, dsp_open_stream_out_ble},
#endif
    {STREAM_OUT_RECORD, dsp_open_stream_out_record},
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
    {STREAM_OUT_AUDIO_TRANSMITTER, dsp_open_stream_out_audio_transmitter},
#endif /* AIR_AUDIO_TRANSMITTER_ENABLE */
    {STREAM_OUT_VIRTUAL, dsp_open_stream_out_virtual},
#ifdef AIR_AUDIO_BT_COMMON_ENABLE
    {STREAM_OUT_BT_COMMON, dsp_open_stream_out_bt_common},
#endif /* AIR_AUDIO_BT_COMMON_ENABLE */
#if defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
    {STREAM_OUT_LLF, dsp_open_stream_out_LLF},
#endif
};
SINK dsp_open_stream_out(mcu2dsp_open_param_p open_param)
{
    SINK sink = NULL;
    uint32_t number, i;
    if (open_param != NULL) {
#ifdef AIR_AUDIO_HARDWARE_ENABLE
                AudioAfeConfiguration(AUDIO_SINK_SCENARIO_TYPE, open_param->audio_scenario_type);
#endif
        number = sizeof(dsp_stream_open_out_entry_table)/sizeof(dsp_stream_open_out_entry_table_t);
        for (i = 0 ; i < number ; i++) {
            if (dsp_stream_open_out_entry_table[i].stream_out == open_param->param.stream_out) {
                sink = dsp_stream_open_out_entry_table[i].entry(open_param);
                break;
        }
    }
    }

    if (sink) {
        sink->scenario_type = open_param->audio_scenario_type;
    } else {
        DSP_MW_LOG_E("DSP sink create fail\r\n", 0);
    }
    return sink;
}

#ifdef AIR_AUDIO_HARDWARE_ENABLE
void dsp_start_stream_in_afe(mcu2dsp_start_param_p start_param, SOURCE source)
{
    source->param.audio.AfeBlkControl.u4awsflag = start_param->stream_in_param.afe.mce_flag;
    DSP_MW_LOG_I("Stream in afe start MCE:%d\r\n", 1, source->param.audio.AfeBlkControl.u4awsflag);
}
#endif /* AIR_AUDIO_HARDWARE_ENABLE */

#ifdef PLAY_EN_CHECK_TIMER
#ifdef AIR_CPU_IN_SECURITY_MODE
#define CONN_BT_TIMCON_BASE 0xA0000000
#else
#define CONN_BT_TIMCON_BASE 0xB0000000
#endif

static TimerHandle_t playen_check_timer = NULL;
static void playen_check_timer_callback(TimerHandle_t xTimer)
{
    UNUSED(xTimer);
    DSP_MW_LOG_I("PLANEN_CHECK_TIMER", 0);
    if ((n9_a2dp_if.transform != NULL) && (n9_a2dp_if.sink->param.audio.irq_exist == FALSE)) {
        DSP_MW_LOG_E("AFE wait play en trigger re-sync", 0);
#ifdef MTK_BT_A2DP_ENABLE
        Au_DL_send_reinit_request(MSG2_DSP2CN4_REINIT_AFE_ABNORMAL, FALSE);
#endif /* MTK_BT_A2DP_ENABLE */
    } else {
        xTimerStop(playen_check_timer, 0);
    }
}
#endif
U32 initial_asi = 0;
#ifdef MTK_BT_A2DP_ENABLE
#ifdef TRIGGER_A2DP_PROCSESS_TIMER
static TimerHandle_t trig_a2dp_proc_timer = NULL;

static void afe_prefill_for_src_out(U16 fill_sample_num, SINK sink)
{
    DSP_MW_LOG_I("prefill samples:%d,remain space:%d", 2, fill_sample_num, SinkSlack(sink));
    if ((fill_sample_num * sink->param.audio.format_bytes) > SinkSlack(sink)) {
        return;
    }
    U32 buffer_per_channel_shift = ((sink->param.audio.channel_num >= 2) && (sink->buftype == BUFFER_TYPE_INTERLEAVED_BUFFER))
                                   ? 1
                                   : 0;
    SinkFlush(sink, fill_sample_num * sink->param.audio.format_bytes);
    sink->param.audio.sram_empty_fill_size = (fill_sample_num * sink->param.audio.format_bytes) << buffer_per_channel_shift;
}

static void trigger_a2dp_proc_timer_callback(TimerHandle_t xTimer)
{
    UNUSED(xTimer);

    if ((n9_a2dp_if.source != NULL) && (n9_a2dp_if.sink != NULL) && (n9_a2dp_if.transform != NULL) && (n9_a2dp_if.sink->param.audio.irq_exist == FALSE)) {
        DSP_MW_LOG_I("trigger_a2dp_proc_timer_callback start", 0);
        n9_a2dp_if.source->transform->Handler(n9_a2dp_if.source, n9_a2dp_if.sink);

        vTaskResume(n9_a2dp_if.source->taskId);
    } else {
        DSP_MW_LOG_I("trigger_a2dp_proc_timer_callback stop", 0);
        xTimerStop(trig_a2dp_proc_timer, 0);
    }
}
#endif

void dsp_start_stream_in_a2dp(mcu2dsp_start_param_p start_param, SOURCE source)
{
    source->param.n9_a2dp.cp_exist = FALSE;
    source->param.n9_a2dp.predict_asi = start_param->stream_in_param.a2dp.start_asi;
    source->param.n9_a2dp.latency_monitor = start_param->stream_in_param.a2dp.latency_monitor_enable;
    source->param.n9_a2dp.DspReportStartId = 0xFFFF;
    source->param.n9_a2dp.alc_monitor = !start_param->stream_in_param.a2dp.alc_monitor;// If Host already enabled ALC, DSP not to trigger ALC mode

    //source->param.n9_a2dp.latency_monitor = TRUE;
    U32 bt_clk,sample,clk_offset, codec_id;
    U16 intra_clk;
    U32 prefill_samples;
#ifdef AIR_A2DP_REINIT_V2_ENABLE
    if(n9_a2dp_if.source != NULL){
        if(start_param->stream_in_param.a2dp.time_stamp_ratio == IS_SPECIAL_DEVICE){
            A2DP_SpeDev_Setting(&(n9_a2dp_if.source->param.n9_a2dp),true);
        }else{
            A2DP_SpeDev_Setting(&(n9_a2dp_if.source->param.n9_a2dp),false);
        }
    }
#endif

#ifdef MTK_GAMING_MODE_HEADSET
    if (source->param.n9_a2dp.codec_info.codec_cap.type == BT_A2DP_CODEC_AIRO_CELT) {
        /* ULL should wait for play en parameter from controller */
        if (audio_headset_ull_ctrl.is_play_en_ready) {
            audio_headset_ull_ctrl.play_en.bt_clock += ((source->param.n9_a2dp.sink_latency - ULL_BASIC_LATENCY) * 8 / 2500); /* 8/2500 = 312.5 for frame based  */

            a2dp_ull_get_proper_play_en(audio_headset_ull_ctrl.play_en.bt_clock,
                                        audio_headset_ull_ctrl.play_en.sequence_number,
                                        &audio_headset_ull_ctrl.play_en.bt_clock,
                                        (U8 *)&audio_headset_ull_ctrl.play_en.sequence_number);

            a2dp_ull_set_init_seq_no(audio_headset_ull_ctrl.play_en.sequence_number);

            a2dp_ull_drop_expired_pkt(source);

            /* AVM buffer needs to update SampleRate as 48k, this is temp */
            source->streamBuffer.AVMBufferInfo.SampleRate = 48000;
            if ((n9_a2dp_if.sink != NULL) && (n9_a2dp_if.sink->type == SINK_TYPE_AUDIO)) {
                n9_a2dp_if.sink->streamBuffer.BufferInfo.WriteOffset = 720;// temp modify prefill samples for ULL
            }
            MCE_TransBT2NativeClk(audio_headset_ull_ctrl.play_en.bt_clock, 0, &bt_clk, &intra_clk, ULL_CLK_Offset);
#ifdef AIR_AUDIO_HARDWARE_ENABLE
            hal_audio_afe_set_play_en(bt_clk, intra_clk);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
            DSP_MW_LOG_I("A2DP DL - A2DP Set Play En Clk:0x%x Intra:0x%x Modify AFE prefill offest to 720", 2, (uint32_t)bt_clk, (uint32_t)intra_clk);
        }
        audio_headset_ull_ctrl.is_a2dp_started = TRUE;
    } else
#endif
    {
#ifdef AIR_DCHS_MODE_ENABLE
        if(dchs_get_device_mode() == DCHS_MODE_SINGLE){
            MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&intra_clk, BT_CLK_Offset);
        }else{
            MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&intra_clk, DCHS_CLK_Offset);
        }
#else
        MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&intra_clk, BT_CLK_Offset);
#endif
        DSP_MW_LOG_I("Get host set asi:0x%x b:0x%x i:0x%x  local clk, n:0x%x b:0x%x i:0x%x latency_monitor :%d ALC mode :%d", 8, start_param->stream_in_param.a2dp.start_asi, start_param->stream_in_param.a2dp.start_bt_clk, start_param->stream_in_param.a2dp.start_bt_intra_clk,
                                                                                                                                rBb->rClkCtl.rNativeClock, bt_clk, intra_clk,
                                                                                                                                start_param->stream_in_param.a2dp.latency_monitor_enable, start_param->stream_in_param.a2dp.alc_monitor);
        start_param->stream_in_param.a2dp.start_bt_intra_clk = (start_param->stream_in_param.a2dp.start_bt_intra_clk & 0xFFFF) << 1;

        if (abs32((S32)(start_param->stream_in_param.a2dp.start_bt_clk - bt_clk)) >  0x2000) { // Maximum tolerence time difference = 0x2000*312.5(us) = 2.56(s)
            DSP_MW_LOG_I("Host notify play time abnormal, bt_clk: 0x%x play_clk: 0x%x", 2, bt_clk, start_param->stream_in_param.a2dp.start_bt_clk);
            start_param->stream_in_param.a2dp.start_bt_clk = bt_clk + 0x100;
            //AUDIO_ASSERT(0);
        }

        if ((S32)(start_param->stream_in_param.a2dp.start_bt_clk - bt_clk) <= 0x10) { // check play time in time
            U32 temp_start_bt_clk = start_param->stream_in_param.a2dp.start_bt_clk;

            codec_id = source->param.n9_a2dp.codec_info.codec_cap.codec.vend.codec_id;
            if ((source->param.n9_a2dp.codec_info.codec_cap.type == BT_A2DP_CODEC_VENDOR) && ((codec_id == BT_A2DP_CODEC_LHDC_CODEC_ID) || (codec_id == BT_A2DP_CODEC_LC3PLUS_CODEC_ID))){
                sample = 960;
                clk_offset = 768000;
            }else {
                sample = 1024;
                clk_offset = 3276800;
            }
            while ((S32)(temp_start_bt_clk - bt_clk) < 0x80) {
                source->param.n9_a2dp.predict_asi += sample;
                temp_start_bt_clk +=  clk_offset / (source->streamBuffer.AVMBufferInfo.SampleRate); // roughly calc
            }
            start_param->stream_in_param.a2dp.start_bt_clk += ((((source->param.n9_a2dp.predict_asi - start_param->stream_in_param.a2dp.start_asi) << 5) * 100 / (source->streamBuffer.AVMBufferInfo.SampleRate)) & 0xFFFFFFC);
            start_param->stream_in_param.a2dp.start_bt_intra_clk += ((((source->param.n9_a2dp.predict_asi - start_param->stream_in_param.a2dp.start_asi)) * 20000) / (source->streamBuffer.AVMBufferInfo.SampleRate / 100)) % 2500;
            DSP_MW_LOG_I("Host notify play time too early, modify play en b:0x%x i:0x%x", 2, start_param->stream_in_param.a2dp.start_bt_clk, start_param->stream_in_param.a2dp.start_bt_intra_clk);
        }
        initial_asi = source->param.n9_a2dp.predict_asi;
        #ifdef AIR_DCHS_MODE_ENABLE
        if(dchs_get_device_mode() == DCHS_MODE_SINGLE){
            MCE_TransBT2NativeClk(start_param->stream_in_param.a2dp.start_bt_clk, start_param->stream_in_param.a2dp.start_bt_intra_clk, &bt_clk, &intra_clk, BT_CLK_Offset);
            DSP_MW_LOG_I("Play en b:0x%x i:0x%x n:0x%x asi: 0x%x", 4, bt_clk, intra_clk, rBb->rClkCtl.rNativeClock, source->param.n9_a2dp.predict_asi);
        }else{
            MCE_TransBT2NativeClk(start_param->stream_in_param.a2dp.start_bt_clk, start_param->stream_in_param.a2dp.start_bt_intra_clk, &bt_clk, &intra_clk, DCHS_CLK_Offset);
            DSP_MW_LOG_I("[DCHS DL] a2dp Play en native clk:%u,play phase:%u,cur native clk:%u clk offset: %u", 4, bt_clk, intra_clk, rBb->rClkCtl.rNativeClock, *((volatile uint32_t *)(0xA0010974)));
            dchs_dl_uart_relay_play_en_info(start_param->stream_in_param.a2dp.start_bt_clk, start_param->stream_in_param.a2dp.start_bt_intra_clk);
        }
        #else
        MCE_TransBT2NativeClk(start_param->stream_in_param.a2dp.start_bt_clk, start_param->stream_in_param.a2dp.start_bt_intra_clk, &bt_clk, &intra_clk, BT_CLK_Offset);
        DSP_MW_LOG_I("Play en b:0x%x i:0x%x n:0x%x asi: 0x%x", 4, bt_clk, intra_clk, rBb->rClkCtl.rNativeClock, source->param.n9_a2dp.predict_asi);
        #endif
#ifdef AIR_AUDIO_HARDWARE_ENABLE
        #ifdef AIR_MIXER_STREAM_ENABLE
        mixer_stream_setup_play_en(bt_clk, intra_clk, source, NULL);
        #else
        hal_audio_afe_set_play_en(bt_clk, intra_clk);
        #endif
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
#ifdef TRIGGER_A2DP_PROCSESS_TIMER
        U32 bt_clk_cur;
        U16 intra_clk_cur;
        MCE_GetBtClk((BTCLK *)&bt_clk_cur, (BTPHASE *)&intra_clk_cur, BT_CLK_Offset);
        U32 bt_clk_duration = start_param->stream_in_param.a2dp.start_bt_clk - bt_clk_cur;
        U32 duration_ms = ((bt_clk_duration >> 1) * 625) / 1000;
        DSP_MW_LOG_I("trigger_a2dp_proc_timer, bclk_play_en:%d, bclk_cur:%d, timer_duration:%d ms", 3, start_param->stream_in_param.a2dp.start_bt_clk, bt_clk_cur, duration_ms);
        if (duration_ms > 10) { /* the tick unit of DSP is 10ms*/
            if (trig_a2dp_proc_timer == NULL) {
                trig_a2dp_proc_timer = xTimerCreate("TRIGGER_A2DP_PROCSESS_TIMER", pdMS_TO_TICKS(duration_ms), pdFALSE, 0, trigger_a2dp_proc_timer_callback);
            }
            if (!trig_a2dp_proc_timer) {
                DSP_MW_LOG_I("trigger_a2dp_proc_timer create timer FAIL", 0);
            } else {
                xTimerStart(trig_a2dp_proc_timer, 0);
            }
        }
        if(source->param.n9_a2dp.codec_info.codec_cap.type == BT_A2DP_CODEC_VENDOR) {
            prefill_samples = (2 * (U32)n9_a2dp_if.sink->param.audio.period * (n9_a2dp_if.sink->param.audio.rate / 1000));
            prefill_samples = prefill_samples - (prefill_samples % source->streamBuffer.AVMBufferInfo.FrameSampleNum);
        } else {
            prefill_samples = ((2 * (U32)n9_a2dp_if.sink->param.audio.period * (n9_a2dp_if.sink->param.audio.rate / 1000) + 256) & 0xFF00);
        }
        afe_prefill_for_src_out(prefill_samples, n9_a2dp_if.sink);

#endif /*TRIGGER_A2DP_PROCSESS_TIMER*/
#ifdef PLAY_EN_CHECK_TIMER
        if (playen_check_timer == NULL) {
            playen_check_timer = xTimerCreate("PLANEN_CHECK_TIMER", pdMS_TO_TICKS(700), pdTRUE, 0, playen_check_timer_callback);
        }
        if (!playen_check_timer) {
            DSP_MW_LOG_I("playen_check_timer create timer FAIL", 0);
        } else {
            xTimerChangePeriod(playen_check_timer, pdMS_TO_TICKS(700), 0);
        }
#endif
    }
    MCE_Initial_Aud_Cnt_from_Controller();
}
#endif /* MTK_BT_A2DP_ENABLE */

#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
void dsp_start_stream_in_audio_transmitter(mcu2dsp_start_param_p start_param, SOURCE source)
{
    UNUSED(start_param);

    DSP_MW_LOG_I("Stream in audio transmitter start:%d, %d\r\n", 2, source->param.data_dl.scenario_type, source->param.data_dl.scenario_sub_id);

    switch (source->param.data_dl.scenario_type) {
#if defined(MTK_GAMING_MODE_HEADSET) || defined(AIR_GAMING_MODE_DONGLE_ENABLE)
        case AUDIO_TRANSMITTER_GAMING_MODE:
            if ((source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0) ||
                (source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_1)) {
                /* start the first timer */
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.timer.timestamp);
                // hal_gpt_sw_start_timer_us(source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.timer.handle,
                //                           source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.timer.period,
                //                           source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.timer.callback,
                //                           (void *)source);

                /* set timer start timestamp as the first data timestamp */
                source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.data_timestamp = source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.timer.timestamp;
            }

            break;
#endif /* MTK_GAMING_MODE_HEADSET || AIR_GAMING_MODE_DONGLE_ENABLE */

#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
        case AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE:
            if ((source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0) ||
                (source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1)) {
                /* get the first timestamp */
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.data_timestamp);
            }

            break;
#endif /* AIR_BLE_AUDIO_DONGLE_ENABLE */

        default:
            break;
    }
}
#endif /* AIR_AUDIO_TRANSMITTER_ENABLE */

#ifdef AIR_AUDIO_BT_COMMON_ENABLE
void dsp_start_stream_in_bt_common(mcu2dsp_start_param_p start_param, SOURCE source)
{
    UNUSED(start_param);

    DSP_MW_LOG_I("Stream in bt common start:%d, %d\r\n", 2, source->param.bt_common.scenario_type, source->param.bt_common.scenario_sub_id);

    switch (source->param.bt_common.scenario_type) {
#if defined(MTK_GAMING_MODE_HEADSET) || defined(AIR_GAMING_MODE_DONGLE_ENABLE)
        case AUDIO_TRANSMITTER_GAMING_MODE:
            if (source->param.bt_common.scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_USB_OUT) {
                /* start the first timer */
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.timer.timestamp);
                // hal_gpt_sw_start_timer_us(source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.timer.handle,
                //                         source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.timer.period,
                //                         source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.timer.callback,
                //                         (void *)source);

                /* set timer start timestamp as the first data timestamp */
                source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.data_timestamp = source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.timer.timestamp;
            }
#ifdef AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE
            else if (source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_DONGLE_LINE_OUT) {
                /* start the first timer */
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.timer.timestamp);
                /* set timer start timestamp as the first data timestamp */
                source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.data_timestamp = source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.timer.timestamp;
            }
#endif /* AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE */
            break;
#endif /* MTK_GAMING_MODE_HEADSET || AIR_GAMING_MODE_DONGLE_ENABLE */

        default:
            break;
    }

}
#endif /* AIR_AUDIO_BT_COMMON_ENABLE */

const dsp_stream_start_in_entry_table_t dsp_stream_start_in_entry_table[] = {
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    {STREAM_IN_AFE, dsp_start_stream_in_afe},
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
#ifdef MTK_BT_A2DP_ENABLE
    {STREAM_IN_A2DP, dsp_start_stream_in_a2dp},
#endif /* MTK_BT_A2DP_ENABLE */
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
    {STREAM_IN_AUDIO_TRANSMITTER, dsp_start_stream_in_audio_transmitter},
#endif /* AIR_AUDIO_TRANSMITTER_ENABLE */
#ifdef AIR_AUDIO_BT_COMMON_ENABLE
    {STREAM_IN_BT_COMMON, dsp_start_stream_in_bt_common},
#endif /* AIR_AUDIO_BT_COMMON_ENABLE */
#ifdef AIR_MIXER_STREAM_ENABLE
    {STREAM_IN_MIXER, dsp_start_stream_in_mixer},
#endif
};
void dsp_start_stream_in(mcu2dsp_start_param_p start_param, SOURCE source)
{
    uint32_t unmber, i;
    if (start_param != NULL) {
        unmber = sizeof(dsp_stream_start_in_entry_table)/sizeof(dsp_stream_start_in_entry_table_t);
        for (i = 0 ; i < unmber ; i++) {
            if (dsp_stream_start_in_entry_table[i].stream_in == start_param->param.stream_in) {
                dsp_stream_start_in_entry_table[i].entry(start_param, source);
                break;
        }
    }
    }
}

#ifdef AIR_AUDIO_HARDWARE_ENABLE
void dsp_start_stream_out_afe(mcu2dsp_start_param_p start_param, SINK sink)
{
    sink->param.audio.AfeBlkControl.u4awsflag = start_param->stream_out_param.afe.mce_flag;
    //sink->param.audio.AfeBlkControl.u4awsflag = TRUE;// synchornize headset project & MCE
    sink->param.audio.aws_sync_request        = start_param->stream_out_param.afe.aws_sync_request;
    sink->param.audio.aws_sync_time           = start_param->stream_out_param.afe.aws_sync_time;
    //DSP_MW_LOG_I("Stream out afe start MCE:%d\r\n", 1, sink->param.audio.AfeBlkControl.u4awsflag);
}
#endif /* AIR_AUDIO_HARDWARE_ENABLE */

#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
void dsp_start_stream_out_audio_transmitter(mcu2dsp_start_param_p start_param, SINK sink)
{
    UNUSED(start_param);
    UNUSED(sink);
    DSP_MW_LOG_I("Stream out audio transmitter start:%d, %d\r\n", 2, sink->param.data_ul.scenario_type, sink->param.data_ul.scenario_sub_id);
}
#endif /* AIR_AUDIO_TRANSMITTER_ENABLE */

#ifdef AIR_AUDIO_BT_COMMON_ENABLE
void dsp_start_stream_out_bt_common(mcu2dsp_start_param_p start_param, SINK sink)
{
    UNUSED(start_param);
    UNUSED(sink);
    DSP_MW_LOG_I("Stream out bt common start:%d, %d\r\n", 2, sink->param.bt_common.scenario_type, sink->param.bt_common.scenario_sub_id);
}
#endif /* AIR_AUDIO_BT_COMMON_ENABLE */


const dsp_stream_start_out_entry_table_t dsp_stream_start_out_entry_table[] = {
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    {STREAM_OUT_AFE, dsp_start_stream_out_afe},
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
    {STREAM_OUT_AUDIO_TRANSMITTER, dsp_start_stream_out_audio_transmitter},
#endif /* AIR_AUDIO_TRANSMITTER_ENABLE */
#ifdef AIR_AUDIO_BT_COMMON_ENABLE
    {STREAM_OUT_BT_COMMON, dsp_start_stream_out_bt_common},
#endif /* AIR_AUDIO_BT_COMMON_ENABLE */
};

void dsp_start_stream_out(mcu2dsp_start_param_p start_param, SINK sink)
{
    uint32_t unmber, i;
    if (start_param != NULL) {
        unmber = sizeof(dsp_stream_start_out_entry_table)/sizeof(dsp_stream_start_out_entry_table_t);
        for (i = 0 ; i < unmber ; i++) {
            if (dsp_stream_start_out_entry_table[i].stream_out == start_param->param.stream_out) {
                dsp_stream_start_out_entry_table[i].entry(start_param, sink);
                break;
        }
    }
    }
}


void dsp_trigger_suspend(SOURCE source, SINK sink)
{
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    if (source != NULL) {
        audio_ops_trigger(source, AFE_PCM_TRIGGER_SUSPEND);
    }

    if (sink != NULL) {
        audio_ops_trigger(sink, AFE_PCM_TRIGGER_SUSPEND);
    }
#else
    UNUSED(source);
    UNUSED(sink);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
}

void dsp_trigger_resume(SOURCE source, SINK sink)
{
#ifdef AIR_AUDIO_PATH_CUSTOMIZE_ENABLE
    gAudioCtrl.Afe.AfeDLSetting.scenario_type = sink->scenario_type;
#endif
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    if (source != NULL) {
        if (!audio_ops_trigger(source, AFE_PCM_TRIGGER_RESUME)) {
            source->param.audio.mute_flag = TRUE;
            source->param.audio.pop_noise_pkt_num = 0;
        }
    }

    if (sink != NULL) {
        audio_ops_trigger(sink, AFE_PCM_TRIGGER_RESUME);
    }
#else
    UNUSED(source);
    UNUSED(sink);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
}
#ifdef MTK_BT_A2DP_ENABLE
/* A2DP CCNI callback function */
void CB_N9_A2DP_OPEN(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
#if 0
    //return;
#else
    UNUSED(ack);
    /* remap to non-cacheable address */
    mcu2dsp_open_param_p open_param;
    open_param = (mcu2dsp_open_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    DSP_REMAP_SHARE_INFO(open_param->stream_in_param.a2dp.p_share_info, avm_share_buf_info_ptr);
    DSP_MW_LOG_I("A2DP OPEN share info %X\r\n", 1, open_param->stream_in_param.a2dp.p_share_info->StartAddr);
    DSP_REMAP_SHARE_INFO(open_param->stream_in_param.a2dp.p_asi_buf, uint32_t *);
    DSP_REMAP_SHARE_INFO(open_param->stream_in_param.a2dp.p_min_gap_buf, uint32_t *);
    DSP_REMAP_SHARE_INFO(open_param->stream_in_param.a2dp.p_pcdc_anchor_info_buf, uint32_t *);
    DSP_REMAP_SHARE_INFO(open_param->stream_in_param.a2dp.p_current_bit_rate, uint32_t *);
    DSP_REMAP_SHARE_INFO(open_param->stream_in_param.a2dp.bt_inf_address, uint32_t);
    DSP_REMAP_SHARE_INFO(open_param->stream_in_param.a2dp.p_afe_buf_report, uint32_t *);

#ifdef MTK_GAMING_MODE_HEADSET
    DSP_REMAP_SHARE_INFO(open_param->stream_in_param.a2dp.ull_clk_info_address, uint32_t *);
    MCE_Update_BtClkOffsetAddr(open_param->stream_in_param.a2dp.ull_clk_info_address, ULL_CLK_Offset);
    DSP_MW_LOG_I("[TEST]ull_clk_info_address:0x%x", 1, open_param->stream_in_param.a2dp.ull_clk_info_address);
#endif
    n9_a2dp_if.source   = dsp_open_stream_in(open_param);
    n9_a2dp_if.sink     = dsp_open_stream_out(open_param);
    n9_a2dp_if.transform = NULL;

    DSP_Callback_PreloaderConfig(n9_a2dp_if.pfeature_table, open_param->audio_scenario_type);

#ifdef AIR_ANC_ADAPTIVE_CLOCK_CONTROL_ENABLE
    dsp_adapt_anc_send_unstable_status(AUDIO_ADAPTIVE_ANC_CLK_CTRL_TYPE_DL_A2DP, 1);
#endif

    DSP_MW_LOG_I("A2DP OPEN Finish\r\n", 0);
    //PIC
#endif
}

void CB_N9_A2DP_START(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
#if 0
    //aud_msg_ack(MSG_MCU2DSP_BT_AUDIO_DL_START | 0x8000, FALSE);
    //return;
#else
    U32 gpt_timer;
    UNUSED(ack);
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_timer);
    DSP_MW_LOG_I("A2DP START Time:%d\r\n", 1, gpt_timer);

    /* remap to non-cacheable address */
    mcu2dsp_start_param_p start_param;
    start_param = (mcu2dsp_start_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);


    dsp_start_stream_in(start_param, n9_a2dp_if.source);
    dsp_start_stream_out(start_param, n9_a2dp_if.sink);
    n9_a2dp_if.sink->param.audio.AfeBlkControl.u4awsflag = 1;
    DSP_MW_LOG_I("A2DP notify cnt address 0x%x Start Ro :%d ,A2DP latency :%d", 3, &(n9_a2dp_if.source->streamBuffer.AVMBufferInfo.NotifyCount), n9_a2dp_if.source->streamBuffer.AVMBufferInfo.ReadIndex,n9_a2dp_if.source->param.n9_a2dp.sink_latency);
#if defined(MTK_BT_A2DP_VENDOR_BC_ENABLE) || defined(MTK_BT_A2DP_VENDOR_1_BC_ENABLE)
    if ((n9_a2dp_if.source->param.n9_a2dp.codec_info.codec_cap.type == BT_A2DP_CODEC_VENDOR) && (n9_a2dp_if.source->param.n9_a2dp.sink_latency == 0)) {
        n9_a2dp_if.sink->param.audio.AfeBlkControl.u4awsflag = FALSE;
        DSP_MW_LOG_I("vendor BC force aws flag to be false", 0);
    }
#endif
    //n9_a2dp_if.sink->param.audio.AfeBlkControl.u4awsflag = FALSE;

    n9_a2dp_if.source->param.n9_a2dp.mce_flag = n9_a2dp_if.sink->param.audio.AfeBlkControl.u4awsflag;



    stream_feature_configure_resolution((stream_feature_list_ptr_t)n9_a2dp_if.pfeature_table, RESOLUTION_32BIT, CONFIG_DECODER);
    DSP_MW_LOG_I("a2dp hwsrc_type:%d",1,n9_a2dp_if.sink->param.audio.hwsrc_type);
#ifdef MTK_HWSRC_IN_STREAM
    if(n9_a2dp_if.sink->param.audio.hwsrc_type == HAL_AUDIO_HWSRC_IN_STREAM){
        U32 samplerate;//modify for ASRC
        samplerate = (n9_a2dp_if.sink->param.audio.rate) / 1000;
        DSP_MW_LOG_I("hwsrc_in_stream afe_get_audio_device_samplerate = %d\r\n codec:%d fixed point %d", 3, samplerate,n9_a2dp_if.source->param.n9_a2dp.codec_info.codec_cap.type ,2048); //modify for ASRC
        stream_feature_configure_src(n9_a2dp_if.pfeature_table, RESOLUTION_32BIT, RESOLUTION_32BIT, samplerate, 2048);
    }
#endif

    n9_a2dp_if.transform = TrasformAudio2Audio(n9_a2dp_if.source, n9_a2dp_if.sink, (stream_feature_list_ptr_t)n9_a2dp_if.pfeature_table);
    n9_a2dp_if.source->param.n9_a2dp.DspReportStartId = msg.ccni_message[0] >> 16 | 0x8000;
    n9_a2dp_if.sink->param.audio.afe_wait_play_en_cnt = 0;
//#ifdef AIR_AUDIO_HARDWARE_ENABLE
    //afe_enable_audio_irq(afe_irq_request_number(AUDIO_DIGITAL_BLOCK_MEM_DL1), n9_a2dp_if.sink->param.audio.rate, n9_a2dp_if.sink->param.audio.count);
//#endif /* AIR_AUDIO_HARDWARE_ENABLE */
    if (n9_a2dp_if.transform == NULL) {
        DSP_MW_LOG_E("A2DP START transform failed", 0);
    }
    // *(volatile uint32_t *)0x70000F58 = 0x00000208; //ZCD_CON2
#if defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
    dsp_llf_set_audio_dl_status(LLF_DL_MIX_TYPE_A2DP, true);
#endif
#endif
}
#ifdef MTK_GAMING_MODE_HEADSET
static uint32_t g_play_en_gpt_handler = NULL;
#endif


void CB_N9_A2DP_STOP(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("A2DP STOP\r\n", 0);
#ifdef TRIGGER_A2DP_PROCSESS_TIMER
    if (trig_a2dp_proc_timer) {
        xTimerStop(trig_a2dp_proc_timer, 0);
    }
#endif /*TRIGGER_A2DP_PROCSESS_TIMER*/
#ifdef PLAY_EN_CHECK_TIMER
    if (playen_check_timer) {
        xTimerStop(playen_check_timer, 0);
    }
#endif
#ifdef MTK_GAMING_MODE_HEADSET
    if (g_play_en_gpt_handler) {
        hal_gpt_sw_stop_timer_us(g_play_en_gpt_handler);
    }
#endif
    if (n9_a2dp_if.transform != NULL) {
        StreamDSPClose(n9_a2dp_if.transform->source, n9_a2dp_if.transform->sink, msg.ccni_message[0] >> 16 | 0x8000);
    }
    n9_a2dp_if.transform = NULL;
    DSP_MW_LOG_I("A2DP STOP Finish\r\n", 0);

}

void CB_N9_A2DP_CLOSE(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{

    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("A2DP CLOSE\r\n", 0);
    DSP_Callback_UnloaderConfig(n9_a2dp_if.pfeature_table, n9_a2dp_if.source->scenario_type);
    SourceClose(n9_a2dp_if.source);
    SinkClose(n9_a2dp_if.sink);
    memset(&n9_a2dp_if,0,sizeof(CONNECTION_IF));
#ifdef AIR_ANC_ADAPTIVE_CLOCK_CONTROL_ENABLE
    dsp_adapt_anc_send_unstable_status(AUDIO_ADAPTIVE_ANC_CLK_CTRL_TYPE_DL_A2DP, 0);
#endif
#if defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
        dsp_llf_set_audio_dl_status(LLF_DL_MIX_TYPE_A2DP, false);
#endif
    DSP_MW_LOG_I("A2DP CLOSE Finish\r\n", 0);
}

void CB_N9_A2DP_CLOCKSKEW(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    //clock skew to adjust hw src
}

void CB_N9_A2DP_SUSPEND(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("A2DP SUSPEND\r\n", 0);

    dsp_trigger_suspend(n9_a2dp_if.source, n9_a2dp_if.sink);


}

void CB_N9_A2DP_RESUME(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("A2DP RESUME\r\n", 0);


    dsp_trigger_resume(n9_a2dp_if.source, n9_a2dp_if.sink);
}

#ifdef AIR_A2DP_REINIT_V2_ENABLE
void Au_DL_receive_reinit_request(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    UNUSED(msg);
    if(n9_a2dp_if.source != NULL){
        n9_a2dp_if.source->param.n9_a2dp.reinit_request= true;
    }
}
#endif

/* End A2DP CCNI callback function */
#endif /* MTK_BT_A2DP_ENABLE */

#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
extern void Source_device_set_para(hal_audio_device_parameter_t *device_handle);

void dsp_detachable_config(CONNECTION_IF *application_ptr, hal_ccni_message_t msg)
{
    audio_transmitter_scenario_type_t scenario_id;
    audio_transmitter_scenario_sub_id_t sub_id;
    mcu2dsp_open_param_p open_param;
    SOURCE source;
    audio_channel pre_ch, ch;
    au_afe_open_param_t afe_open;
    scenario_id = (msg.ccni_message[0] >> 8) & 0xFF;
    sub_id.scenario_id  = (msg.ccni_message[0] & 0xFF);
    DSP_MW_LOG_I("dsp_detachable_config", 0);
    if (msg.ccni_message[1] != NULL) {
        open_param = (mcu2dsp_open_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
        open_param->stream_in_param.afe.stream_channel = application_ptr->source->param.audio.stream_channel;
        open_param->stream_in_param.afe.memory = application_ptr->source->param.audio.memory;
        if (application_ptr->source->param.audio.echo_reference) {
            open_param->stream_in_param.afe.memory |= HAL_AUDIO_MEM3;
        }
        // re-use parameters
        open_param->stream_in_param.afe.irq_period               = application_ptr->source->param.audio.period;
        open_param->stream_in_param.afe.sampling_rate            = application_ptr->source->param.audio.rate;
        open_param->stream_in_param.afe.stream_out_sampling_rate = application_ptr->source->param.audio.src_rate;
        open_param->stream_in_param.afe.format                   = application_ptr->source->param.audio.format;
        open_param->stream_in_param.afe.frame_number             = application_ptr->source->param.audio.Buffer_Frame_Num;
        open_param->stream_in_param.afe.frame_size               = application_ptr->source->param.audio.Frame_Size;
        open_param->audio_scenario_type                          = application_ptr->source->scenario_type;

        AudioAfeConfiguration(AUDIO_SOURCE_SCENARIO_TYPE, application_ptr->source->scenario_type);
        AudioAfeConfiguration(AUDIO_SINK_SCENARIO_TYPE, application_ptr->sink->scenario_type);

        afe_open.stream_channel = open_param->stream_in_param.afe.stream_channel;
        afe_open.audio_device  = open_param->stream_in_param.afe.audio_device;
        afe_open.audio_device1 = open_param->stream_in_param.afe.audio_device1;
        afe_open.audio_device2 = open_param->stream_in_param.afe.audio_device2;
        afe_open.audio_device3 = open_param->stream_in_param.afe.audio_device3;
        afe_open.audio_device4 = open_param->stream_in_param.afe.audio_device4;
        afe_open.audio_device5 = open_param->stream_in_param.afe.audio_device5;
        pre_ch = afe_get_audio_channel_by_au_afe_open_param(&(afe_open));
        afe_open.audio_device  = open_param->stream_in_param.afe.audio_device;
        afe_open.audio_device1 = open_param->stream_in_param.afe.audio_device1;
        afe_open.audio_device2 = open_param->stream_in_param.afe.audio_device2;
        afe_open.audio_device3 = open_param->stream_in_param.afe.audio_device3;
        afe_open.audio_device4 = open_param->stream_in_param.afe.audio_device4;
        afe_open.audio_device5 = open_param->stream_in_param.afe.audio_device5;
        ch = afe_get_audio_channel_by_au_afe_open_param(&(afe_open));
        if (pre_ch != ch) {
            HAL_AUDIO_LOG_ERROR("pre_ch=%d != ch=%d", 2, pre_ch, ch);
            platform_assert("please check open paraters.", __FILE__, __LINE__);
        }
        source = dsp_open_stream_in_afe(open_param);
        hal_audio_path_parameter_t   *path_handle   = &source->param.audio.path_handle;
        hal_audio_device_parameter_t *device_handle = &source->param.audio.device_handle;
        hal_audio_memory_parameter_t *mem_handle    = &source->param.audio.mem_handle;     //modify for ab1568
        AUDIO_PARAMETER              *pAudPara              = &source->param.audio;

#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
        hal_audio_device_parameter_t *device_handle1        = &source->param.audio.device_handle1;
        hal_audio_device_parameter_t *device_handle2        = &source->param.audio.device_handle2;
        hal_audio_device_parameter_t *device_handle3        = &source->param.audio.device_handle3;
        hal_audio_device_parameter_t *device_handle4        = &source->param.audio.device_handle4;
        hal_audio_device_parameter_t *device_handle5        = &source->param.audio.device_handle5;
        hal_audio_device_parameter_t *device_handle6        = &source->param.audio.device_handle6;
        hal_audio_device_parameter_t *device_handle7        = &source->param.audio.device_handle7;
#endif

        pAudPara->audio_device = gAudioCtrl.Afe.AfeULSetting.audio_device;
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
        pAudPara->audio_device1 = gAudioCtrl.Afe.AfeULSetting.audio_device1;
        pAudPara->audio_device2 = gAudioCtrl.Afe.AfeULSetting.audio_device2;
        pAudPara->audio_device3 = gAudioCtrl.Afe.AfeULSetting.audio_device3;
        pAudPara->audio_device4 = gAudioCtrl.Afe.AfeULSetting.audio_device4;
        pAudPara->audio_device5 = gAudioCtrl.Afe.AfeULSetting.audio_device5;
        pAudPara->audio_device6 = gAudioCtrl.Afe.AfeULSetting.audio_device6;
        pAudPara->audio_device7 = gAudioCtrl.Afe.AfeULSetting.audio_device7;
#endif
        pAudPara->audio_interface = gAudioCtrl.Afe.AfeULSetting.audio_interface;
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
        pAudPara->audio_interface1 = gAudioCtrl.Afe.AfeULSetting.audio_interface1;
        pAudPara->audio_interface2 = gAudioCtrl.Afe.AfeULSetting.audio_interface2;
        pAudPara->audio_interface3 = gAudioCtrl.Afe.AfeULSetting.audio_interface3;
        pAudPara->audio_interface4 = gAudioCtrl.Afe.AfeULSetting.audio_interface4;
        pAudPara->audio_interface5 = gAudioCtrl.Afe.AfeULSetting.audio_interface5;
        pAudPara->audio_interface6 = gAudioCtrl.Afe.AfeULSetting.audio_interface6;
        pAudPara->audio_interface7 = gAudioCtrl.Afe.AfeULSetting.audio_interface7;
#endif
        uint32_t i;
        hal_audio_path_port_parameter_t input_port_parameters, output_port_parameters;
        input_port_parameters.device_interface = pAudPara->audio_interface;
        #ifdef AIR_ECHO_MEMIF_IN_ORDER_ENABLE
        output_port_parameters.memory_select = mem_handle->memory_select;
        #else
        output_port_parameters.memory_select = mem_handle->memory_select & (~HAL_AUDIO_MEMORY_UL_AWB2);
        #endif
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
        hal_audio_device_t path_audio_device[HAL_AUDIO_PATH_SUPPORT_SEQUENCE] = {
            pAudPara->audio_device, pAudPara->audio_device1, pAudPara->audio_device2, pAudPara->audio_device3
            #ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
            , pAudPara->audio_device4, pAudPara->audio_device5, pAudPara->audio_device6, pAudPara->audio_device7
            #endif /* MTK_AUDIO_HW_IO_CONFIG_ENHANCE */
        };
        hal_audio_interface_t device_interface[HAL_AUDIO_PATH_SUPPORT_SEQUENCE]= {
            pAudPara->audio_interface, pAudPara->audio_interface1, pAudPara->audio_interface2, pAudPara->audio_interface3
            #ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
            , pAudPara->audio_interface4, pAudPara->audio_interface5, pAudPara->audio_interface6, pAudPara->audio_interface7
            #endif /* MTK_AUDIO_HW_IO_CONFIG_ENHANCE */
        };
        hal_audio_memory_selection_t memory_select[HAL_AUDIO_PATH_SUPPORT_SEQUENCE]= {
            HAL_AUDIO_MEMORY_UL_VUL1, HAL_AUDIO_MEMORY_UL_VUL1, HAL_AUDIO_MEMORY_UL_VUL2, HAL_AUDIO_MEMORY_UL_VUL2
            #ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
            , HAL_AUDIO_MEMORY_UL_VUL3, HAL_AUDIO_MEMORY_UL_VUL3, HAL_AUDIO_MEMORY_UL_AWB, HAL_AUDIO_MEMORY_UL_AWB
            #endif /* MTK_AUDIO_HW_IO_CONFIG_ENHANCE */
        };
#else
        hal_audio_device_t path_audio_device[HAL_AUDIO_PATH_SUPPORT_SEQUENCE] = {
            pAudPara->audio_device, pAudPara->audio_device
        };
        hal_audio_interface_t device_interface[HAL_AUDIO_PATH_SUPPORT_SEQUENCE] = {
            pAudPara->audio_interface, pAudPara->audio_interface
        };
        hal_audio_memory_selection_t memory_select[HAL_AUDIO_PATH_SUPPORT_SEQUENCE] = {
            output_port_parameters.memory_select, output_port_parameters.memory_select
        };
#endif
        for (i = 0 ; i < path_handle->connection_number ; i++) {
            input_port_parameters.device_interface    = device_interface[i];
            output_port_parameters.memory_select      = memory_select[i];
            #ifdef AIR_ECHO_MEMIF_IN_ORDER_ENABLE
            if (pAudPara->echo_reference && (i == path_handle->connection_number-1)) {
                #ifdef AIR_DCHS_MODE_ENABLE
                if (path_handle->scenario_type == AUDIO_SCENARIO_TYPE_ANC_MONITOR_STREAM) {
                    path_handle->input.interconn_sequence[i]  =  HAL_AUDIO_INTERCONN_SELECT_INPUT_DOWN_SAMPLER01_CH1;
                } else {
                    path_handle->input.interconn_sequence[i]  =  HAL_AUDIO_INTERCONN_SELECT_INPUT_DOWN_SAMPLER23_CH1;
                }
                #else
                    path_handle->input.interconn_sequence[i]  =  HAL_AUDIO_INTERCONN_SELECT_INPUT_DOWN_SAMPLER23_CH1;
                #endif
                    path_handle->output.interconn_sequence[i] =  HAL_AUDIO_INTERCONN_SELECT_OUTPUT_MEMORY_VUL1_CH1+i;
            } else
            #endif
            {
                path_handle->input.interconn_sequence [i] = stream_audio_convert_control_to_interconn(path_audio_device[i], input_port_parameters, i, true);
                path_handle->output.interconn_sequence[i] = stream_audio_convert_control_to_interconn(HAL_AUDIO_CONTROL_MEMORY_INTERFACE, output_port_parameters, i, true);
            }
        }
        if ((pAudPara->audio_device) & (HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_DUAL |
                                        HAL_AUDIO_CONTROL_DEVICE_LINE_IN_DUAL |
                                        HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_DUAL |
                                        HAL_AUDIO_CONTROL_DEVICE_ANC |
                                        HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER |
                                        HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE)
) {
            device_handle->common.rate = pAudPara->rate;
            device_handle->common.device_interface = pAudPara->audio_interface;
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
            device_handle1->common.rate = pAudPara->rate;
            device_handle1->common.device_interface = pAudPara->audio_interface1;
            device_handle2->common.rate = pAudPara->rate;
            device_handle2->common.device_interface = pAudPara->audio_interface2;
            device_handle3->common.rate = pAudPara->rate;
            device_handle3->common.device_interface = pAudPara->audio_interface3;
            device_handle4->common.rate = pAudPara->rate;
            device_handle4->common.device_interface = pAudPara->audio_interface4;
            device_handle5->common.rate = pAudPara->rate;
            device_handle5->common.device_interface = pAudPara->audio_interface5;
            device_handle6->common.rate = pAudPara->rate;
            device_handle6->common.device_interface = pAudPara->audio_interface6;
            device_handle7->common.rate = pAudPara->rate;
            device_handle7->common.device_interface = pAudPara->audio_interface7;
#endif
            DSP_MW_LOG_I("set device common.rate %d,source rate %d",2,device_handle->common.rate,pAudPara->rate);
        }

        if (pAudPara->audio_device == HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE) {
            device_handle->i2s_slave.memory_select = mem_handle->memory_select;
        }

        device_handle->common.audio_device = pAudPara->audio_device;
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
        device_handle1->common.audio_device = pAudPara->audio_device1;
        device_handle2->common.audio_device = pAudPara->audio_device2;
        device_handle3->common.audio_device = pAudPara->audio_device3;
        device_handle4->common.audio_device = pAudPara->audio_device4;
        device_handle5->common.audio_device = pAudPara->audio_device5;
        device_handle6->common.audio_device = pAudPara->audio_device6;
        device_handle7->common.audio_device = pAudPara->audio_device7;
#endif
        Source_device_set_para(device_handle);
#ifdef AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
        Source_device_set_para(device_handle1);
        Source_device_set_para(device_handle2);
        Source_device_set_para(device_handle3);
        Source_device_set_para(device_handle4);
        Source_device_set_para(device_handle5);
        Source_device_set_para(device_handle6);
        Source_device_set_para(device_handle7);
#endif
    }
    DSP_MW_LOG_I("dsp_detachable_config finish", 0);
}
#endif

#ifdef MTK_ANC_ENABLE
uint8_t anc_talk_mic_hfp_flag = 0;
#endif

#ifdef AIR_BT_HFP_ENABLE
bool ScoDlStopFlag;
bool g_esco_dl_open_flag = false;
#ifdef AIR_HFP_DNN_PATH_ENABLE
CONNECTION_IF sco_ul_dnn_if;
#endif

/* eSCO CCNI callback function */
void CB_N9_SCO_UL_OPEN(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    U32 scenario = (msg.ccni_message[0] & 0x00ff);
    DSP_MW_LOG_I("eSCO UL OPEN, scenario:%d", 1, scenario);
#if defined (AIR_DCHS_MODE_ENABLE)
    if(dchs_get_device_mode() != DCHS_MODE_SINGLE){
        dsp_uart_ul_open();
        dsp_uart_ul_clear_rx_buffer();
    }
#endif
    mcu2dsp_open_param_p open_param;
    open_param = (mcu2dsp_open_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    DSP_REMAP_SHARE_INFO(open_param->stream_out_param.hfp.p_share_info, avm_share_buf_info_ptr);

    if ((scenario == AUDIO_DSP_CODEC_TYPE_CVSD) || (scenario == AUDIO_DSP_CODEC_TYPE_MSBC)) {
        n9_sco_ul_if.source   = dsp_open_stream_in(open_param);
        n9_sco_ul_if.sink     = dsp_open_stream_out(open_param);
        n9_sco_ul_if.transform = NULL;
        DSP_Callback_PreloaderConfig(n9_sco_ul_if.pfeature_table, open_param->audio_scenario_type);
        //DSP_MW_LOG_I("eSCO UL OPEN Finish", 0);
    }
#ifdef AIR_HFP_DNN_PATH_ENABLE
    else if (scenario == AUDIO_DSP_CODEC_TYPE_DNN) {
        sco_ul_dnn_if.source   = dsp_open_stream_in(open_param);
        sco_ul_dnn_if.sink     = dsp_open_stream_out(open_param);
        sco_ul_dnn_if.transform = NULL;
        sco_ul_dnn_if.pfeature_table = (stream_feature_list_ptr_t)&stream_feature_list_hfp_uplink_dnn;
        DSP_Callback_PreloaderConfig(sco_ul_dnn_if.pfeature_table, open_param->audio_scenario_type);
        //DSP_MW_LOG_I("eSCO UL DNN Open Finish", 0);
        //*((volatile uint32_t *)(0xc00001dc)) = 0x1;
        //*((volatile uint32_t *)(0xc00001f0)) = 0x4A42A42;
    }
#endif
#ifdef MTK_ANC_ENABLE
    anc_talk_mic_hfp_flag = 1;
#endif
}

void CB_N9_SCO_UL_START(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    U32 scenario = (msg.ccni_message[0] & 0x00ff);
    U32 gpt_timer;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_timer);
    DSP_MW_LOG_I("eSCO UL START, scenario:%d", 1, scenario);

    mcu2dsp_start_param_p start_param;
    start_param = (mcu2dsp_start_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);

    if (scenario == 0) {
        dsp_start_stream_in(start_param, n9_sco_ul_if.source);
        dsp_start_stream_out(start_param, n9_sco_ul_if.sink);
        n9_sco_ul_if.source->param.audio.AfeBlkControl.u4awsflag = 1;
#if defined(AIR_UL_FIX_RESOLUTION_32BIT)
        stream_feature_configure_resolution((stream_feature_list_ptr_t)n9_sco_ul_if.pfeature_table, RESOLUTION_32BIT, 0);
#else
        stream_feature_configure_resolution((stream_feature_list_ptr_t)n9_sco_ul_if.pfeature_table, RESOLUTION_16BIT, 0);
#endif
        n9_sco_ul_if.transform = TrasformAudio2Audio(n9_sco_ul_if.source, n9_sco_ul_if.sink, n9_sco_ul_if.pfeature_table);
        if (n9_sco_ul_if.transform == NULL) {
            DSP_MW_LOG_E("SCO UL transform failed", 0);
        }
        //DSP_MW_LOG_I("eSCO UL START finish",0);
    }
#ifdef AIR_HFP_DNN_PATH_ENABLE
    else if (scenario == AUDIO_DSP_CODEC_TYPE_DNN) {
        dsp_start_stream_in(start_param, sco_ul_dnn_if.source);
        dsp_start_stream_out(start_param, sco_ul_dnn_if.sink);
        sco_ul_dnn_if.source->param.audio.AfeBlkControl.u4awsflag = 0;
        stream_feature_configure_resolution((stream_feature_list_ptr_t)sco_ul_dnn_if.pfeature_table, RESOLUTION_16BIT, 0);
        sco_ul_dnn_if.transform = TrasformAudio2Audio(sco_ul_dnn_if.source, sco_ul_dnn_if.sink, sco_ul_dnn_if.pfeature_table);
        if (sco_ul_dnn_if.transform == NULL) {
            DSP_MW_LOG_E("SCO UL DNN transform failed", 0);
        }
        //DSP_MW_LOG_I("eSCO UL DNN START Finish", 0);
    }
#endif
}

void CB_N9_SCO_UL_STOP(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    U32 scenario = (msg.ccni_message[0] & 0x00ff);
    DSP_MW_LOG_I("eSCO UL STOP, scenario:%d", 1, scenario);

    if (scenario == 0) {
        if (n9_sco_ul_if.transform != NULL) {
            StreamDSPClose(n9_sco_ul_if.transform->source, n9_sco_ul_if.transform->sink, msg.ccni_message[0] >> 16 | 0x8000);
        }
        n9_sco_ul_if.transform = NULL;
    }
#ifdef AIR_HFP_DNN_PATH_ENABLE
    else if (scenario == AUDIO_DSP_CODEC_TYPE_DNN) {
        if (sco_ul_dnn_if.transform != NULL) {
            StreamDSPClose(sco_ul_dnn_if.transform->source, sco_ul_dnn_if.transform->sink, msg.ccni_message[0] >> 16 | 0x8000);
        }
        sco_ul_dnn_if.transform = NULL;
    }
#endif
    DSP_MW_LOG_I("eSCO UL  STOP Finish, scenario type :%d", 1,scenario);
}

void CB_N9_SCO_UL_CLOSE(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    U32 scenario = (msg.ccni_message[0] & 0x00ff);
#ifdef AIR_DCHS_MODE_ENABLE
    dsp_uart_ul_clear_rx_buffer();
    dchs_send_unlock_sleep_msg(false);
#endif
    if (scenario == 0) {
        DSP_Callback_UnloaderConfig(n9_sco_ul_if.pfeature_table, n9_sco_ul_if.source->scenario_type);
        SourceClose(n9_sco_ul_if.source);
        SinkClose(n9_sco_ul_if.sink);
        memset(&n9_sco_ul_if,0,sizeof(CONNECTION_IF));
#ifdef AIR_AUDIO_HARDWARE_ENABLE
#ifdef AIR_SOFTWARE_GAIN_ENABLE
        call_sw_gain_deinit();
#endif
#endif
    }
#ifdef AIR_HFP_DNN_PATH_ENABLE
    else if (scenario == AUDIO_DSP_CODEC_TYPE_DNN) {
        DSP_Callback_UnloaderConfig(sco_ul_dnn_if.pfeature_table, sco_ul_dnn_if.source->scenario_type);
        SourceClose(sco_ul_dnn_if.source);
        SinkClose(sco_ul_dnn_if.sink);
        memset(&sco_ul_dnn_if,0,sizeof(CONNECTION_IF));

    }
#endif
    DSP_MW_LOG_I("eSCO UL CLOSE Finish, scenario type :%d", 1,scenario);
#ifdef MTK_ANC_ENABLE
    anc_talk_mic_hfp_flag = 0;
#endif
}

void CB_N9_SCO_UL_PLAY(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);

    if (n9_sco_ul_if.transform != NULL) {
        DSP_MW_LOG_I("eSCO UL PLAY\r\n", 0);
        hal_audio_trigger_start_parameter_t start_parameter;
        start_parameter.memory_select = n9_sco_ul_if.source->param.audio.mem_handle.memory_select;
        start_parameter.enable = true;
        hal_audio_set_value((hal_audio_set_value_parameter_t *)&start_parameter, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &(n9_sco_ul_if.sink->param.n9sco.ul_play_gpt));
    } else {
        DSP_MW_LOG_I("eSCO UL PLAY when tansform not exist\r\n", 0);
    }
}

void CB_N9_SCO_UL_SUSPEND(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("eSCO UL SUSPEND\r\n", 0);
    dsp_trigger_suspend(n9_sco_ul_if.source, n9_sco_ul_if.sink);

}


void CB_N9_SCO_UL_RESUME(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("eSCO UL RESUME\r\n", 0);
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
    dsp_detachable_config(&n9_sco_ul_if, msg);
#endif
    dsp_trigger_resume(n9_sco_ul_if.source, n9_sco_ul_if.sink);
}

void CB_N9_SCO_DL_OPEN(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    mcu2dsp_open_param_p open_param;
    open_param = (mcu2dsp_open_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    DSP_REMAP_SHARE_INFO(open_param->stream_in_param.hfp.p_share_info, avm_share_buf_info_ptr);
    DSP_MW_LOG_I("eSCO DL OPEN share info->StartAddr: 0x%x, forwarder: 0x%x\r\n", 2, open_param->stream_in_param.hfp.p_share_info->StartAddr, open_param->stream_in_param.hfp.p_share_info->ForwarderAddr);
    DSP_REMAP_SHARE_INFO(open_param->stream_in_param.hfp.bt_inf_address, uint32_t);
    DSP_REMAP_SHARE_INFO(open_param->stream_in_param.hfp.clk_info_address, uint32_t *);
#ifdef AIR_AIRDUMP_ENABLE
    DSP_REMAP_SHARE_INFO(open_param->stream_in_param.hfp.p_air_dump_buf, uint32_t *);
#endif

    n9_sco_dl_if.source   = dsp_open_stream_in(open_param);
    g_esco_dl_open_flag = true;
    n9_sco_dl_if.sink     = dsp_open_stream_out(open_param);
    g_esco_dl_open_flag = false;
    n9_sco_dl_if.transform = NULL;
    DSP_Callback_PreloaderConfig(n9_sco_dl_if.pfeature_table, open_param->audio_scenario_type);
    //DSP_MW_LOG_I("eSCO DL OPEN Finish\r\n", 0);
#ifdef AIR_ANC_ADAPTIVE_CLOCK_CONTROL_ENABLE
    dsp_adapt_anc_send_unstable_status(AUDIO_ADAPTIVE_ANC_CLK_CTRL_TYPE_DL_HFP, 1);
#endif
}

void CB_N9_SCO_DL_START(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("eSCO DL START\r\n", 0);

    mcu2dsp_start_param_p start_param;
    start_param = (mcu2dsp_start_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);


    dsp_start_stream_in(start_param, n9_sco_dl_if.source);
    dsp_start_stream_out(start_param, n9_sco_dl_if.sink);


    Forwarder_Rx_Buf_Ctrl(TRUE);
#if 1
    n9_sco_dl_if.sink->param.audio.AfeBlkControl.u4awsflag = TRUE;
#else
    n9_sco_dl_if.sink->param.audio.AfeBlkControl.u4awsflag = FALSE; // Open DL AFE directly
#endif
    n9_sco_dl_if.transform = TrasformAudio2Audio(n9_sco_dl_if.source, n9_sco_dl_if.sink, n9_sco_dl_if.pfeature_table);
    ScoDlStopFlag = FALSE;

    if (n9_sco_dl_if.transform == NULL) {
        DSP_MW_LOG_E("SCO DL transform failed", 0);
    } else {
        n9_sco_dl_if.source->param.n9sco.rx_forwarder_en = TRUE;
        //DSP_MW_LOG_E("SCO DL n9_sco_dl_if.sink->param.audio.rx_forwarder_en: %d ", 1, n9_sco_dl_if.source->param.n9sco.rx_forwarder_en);
        Forwarder_Rx_Intr_HW_Handler();
        Forwarder_Rx_Intr_Ctrl(TRUE);

#if defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
        dsp_llf_set_audio_dl_status(LLF_DL_MIX_TYPE_ESCO, true);
#endif
    }
}

void CB_N9_SCO_DL_STOP(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("eSCO DL STOP\r\n", 0);
    ScoDlStopFlag = TRUE;
    if (n9_sco_dl_if.transform != NULL) {
        StreamDSPClose(n9_sco_dl_if.transform->source, n9_sco_dl_if.transform->sink, msg.ccni_message[0] >> 16 | 0x8000);
    }
    n9_sco_dl_if.transform = NULL;
    DSP_MW_LOG_I("eSCO DL STOP Finish\r\n", 0);


#if defined(MTK_ANC_ENABLE) && defined(AIR_ANC_SCENARIO_CONTROL_GAIN_ENABLE)
    //anc call mode off
    dsp_anc_apply_ramp_gain_by_audio_scenario(DSP_ANC_CONTROL_AUDIO_SCENARIO_TYPE_CALL, false, NULL);
#endif

}

void CB_N9_SCO_DL_CLOSE(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("eSCO DL CLOSE\r\n", 0);
    DSP_Callback_UnloaderConfig(n9_sco_dl_if.pfeature_table, n9_sco_dl_if.source->scenario_type);
    SourceClose(n9_sco_dl_if.source);
    SinkClose(n9_sco_dl_if.sink);
    memset(&n9_sco_dl_if,0,sizeof(CONNECTION_IF));
    DSP_MW_LOG_I("eSCO DL CLOSE Finish\r\n", 0);
#ifdef AIR_ANC_ADAPTIVE_CLOCK_CONTROL_ENABLE
    dsp_adapt_anc_send_unstable_status(AUDIO_ADAPTIVE_ANC_CLK_CTRL_TYPE_DL_HFP, 0);
#endif
#if defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
    dsp_llf_set_audio_dl_status(LLF_DL_MIX_TYPE_ESCO, false);
#endif
#ifdef AIR_HEARING_PROTECTION_ENABLE
        stream_function_cpd_deinitialize(true);
#ifdef AIR_BTA_IC_PREMIUM_G2
        g_hearing_protection_enable = false;
#endif
#endif
}

void CB_N9_SCO_CLOCKSKEW(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    //TBD
}

void CB_N9_SCO_DL_SUSPEND(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("eSCO DL SUSPEND\r\n", 0);

    dsp_trigger_suspend(n9_sco_dl_if.source, n9_sco_dl_if.sink);

}


void CB_N9_SCO_DL_RESUME(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("eSCO DL RESUME\r\n", 0);

    dsp_trigger_resume(n9_sco_dl_if.source, n9_sco_dl_if.sink);
}

void CB_N9_SCO_ULIRQ(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    //printf("CB_N9_SCO_ULIRQ\r\n");
    //n9_sco_ul_if.source->transform->Handler(n9_sco_ul_if.source,n9_sco_ul_if.sink);
    //vTaskResume(DAV_TASK_ID);
}

void CB_N9_SCO_DLIRQ(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    if ((Source_blks[SOURCE_TYPE_N9SCO] == NULL) || (n9_sco_dl_if.transform == NULL)) {
        DSP_MW_LOG_I("Unexpected N9 DL IRQ", 0);
        return;
    }
    if (Source_blks[SOURCE_TYPE_N9SCO]->param.n9sco.share_info_base_addr->ReadIndex == Source_blks[SOURCE_TYPE_N9SCO]->param.n9sco.share_info_base_addr->WriteIndex) {
#ifdef PT_bufferfull
        Source_blks[SOURCE_TYPE_N9SCO]->param.n9sco.share_info_base_addr->bBufferIsFull = TRUE;
#endif
        DSP_MW_LOG_I("SCO DL bufferfull, ro:%d, wo:%d", 2, Source_blks[SOURCE_TYPE_N9SCO]->param.n9sco.share_info_base_addr->ReadIndex, Source_blks[SOURCE_TYPE_N9SCO]->param.n9sco.share_info_base_addr->WriteIndex);
    }
    if (n9_sco_dl_if.source->param.n9sco.IsFirstIRQ) {
        n9_sco_dl_if.source->param.n9sco.IsFirstIRQ = FALSE;
        U32 gpt_timer;
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_timer);
        DSP_MW_LOG_I("CB_N9_SCO_DLIRQ, First Wo:%d, Ro:%d,GPT : %d\r\n", 3, Source_blks[SOURCE_TYPE_N9SCO]->param.n9sco.share_info_base_addr->WriteIndex, Source_blks[SOURCE_TYPE_N9SCO]->param.n9sco.share_info_base_addr->ReadIndex, gpt_timer);
        if ((n9_sco_dl_if.source != NULL) && (n9_sco_dl_if.source->transform->Handler != NULL)) {
            n9_sco_dl_if.source->transform->Handler(n9_sco_dl_if.source, n9_sco_dl_if.sink);
            vTaskResume(n9_sco_dl_if.source->taskId);
        }
    }
    if (ScoDlStopFlag == TRUE) {
        vTaskResume(n9_sco_dl_if.source->taskId);
    }
}

void CB_N9_SCO_MICIRQ(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    //printf("CB_N9_SCO_MICIRQ\r\n");
    //n9_sco_ul_if.source->transform->Handler(n9_sco_ul_if.source,n9_sco_ul_if.sink);
    //vTaskResume(DAV_TASK_ID);
}
#endif /* End eSCO CCNI callback function */

void CB_AUDIO_DUMP_INIT(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    uint16_t u2SendID;
    uint32_t i;
    UNUSED(ack);

    u2SendID = (msg.ccni_message[0] & 0x00ff);
    DSP_MW_LOG_I("[Audio Dump] CCNI ID: %x, Dump Mask: %x, SendID: %x", 3, msg.ccni_message[0], msg.ccni_message[1], u2SendID);
    if (u2SendID == 0) {
        memcpy(AudioDumpCfgIDs,(void *)msg.ccni_message[1],sizeof(AudioDumpCfgIDs));
        //reset dump ids to accum bytes array
        memset(DumpIDsAccumBytes, 0, sizeof(DumpIDsAccumBytes));
        for(i = 0;i < AUDIO_DUMP_CONFIG_MAX_NUM;i++){
            if(AudioDumpCfgIDs[i] != 0){
                DumpIDsAccumBytes[i].dump_id = AudioDumpCfgIDs[i];
                DSP_MW_LOG_I("[Audio Dump] Cfg Audio Dump ID: %d", 1, AudioDumpCfgIDs[i]);
            }
        }
    } else if (u2SendID == 1) {
        DumpIdCfgVersion = msg.ccni_message[1];
        DSP_MW_LOG_I("[Audio Dump] DumpIdCfgVersion = %d",1,DumpIdCfgVersion);
    } else if (u2SendID == 2) {
        AudioDumpDevice = msg.ccni_message[1];
        DSP_MW_LOG_I("[Audio Dump] AudioDumpDevice = %d",1,AudioDumpDevice);
    } else if(u2SendID == 3){ //race-cmd/at-cmd cfg runtime

        U32 cfgValue = msg.ccni_message[1];
        U16 dumpID   = cfgValue & 0x0000FFFF;
        U8  enable   = (cfgValue >> 16) & 0x00FF;
        DSP_MW_LOG_I("[Audio Dump] get runtime cfg, dump id=%d , enable=%d, cfgValue=0x%x", 3, dumpID, enable, cfgValue);
        if(enable){
            if(!check_cfg_dump_id(dumpID)){ //if no cfg, add it
                add_cfg_dump_id(dumpID);
                AudioDumpDevice  = AUDIO_DUMP_DEVICE_LOGGING;
                DumpIdCfgVersion = DUMP_ID_VERSION;
            }
        }else{
            if(check_cfg_dump_id(dumpID)){ //if have cfg, remove it
                remove_cfg_dump_id(dumpID);
            }
        }
    } else if(u2SendID == 4) { //enable/disable block dump.
        U8  enable = msg.ccni_message[1];
        audio_dump_set_block(enable);
    }

#ifdef AIR_AUDIO_DUMP_BY_SPDIF_ENABLE
    audio_dump_init();
#endif
}

#ifdef AIR_BT_CODEC_BLE_ENABLED
static uint32_t g_dl_sw_timer_handle, g_ul_sw_timer_handle;
static BTTIME_STRU g_ble_dl_play_en_time;
static void ble_set_dl_play_en_time(BTTIME_STRU_PTR p_play_en_time) {
    g_ble_dl_play_en_time.period = p_play_en_time->period;
    g_ble_dl_play_en_time.phase = p_play_en_time->phase;
}
static bool ble_check_dl_play_en_time(void) {
    if ((g_ble_dl_play_en_time.period == 0) && (g_ble_dl_play_en_time.phase == 0)) {
        /* UL only */
        return false;
    } else {
        /* DL is opening */
        return true;
    }
}
static void ble_reset_dl_play_en_time(void) {
    g_ble_dl_play_en_time.period = 0;
    g_ble_dl_play_en_time.phase = 0;
}

/* BLE CCNI callback function */
void CB_N9_BLE_UL_OPEN(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("[BLE] UL OPEN\r\n", 0);
#if defined (AIR_DCHS_MODE_ENABLE)
    if(dchs_get_device_mode() != DCHS_MODE_SINGLE){
        dsp_uart_ul_open();
        dsp_uart_ul_clear_tx_buffer();
        dsp_uart_ul_clear_rx_buffer();
    }
#endif
    mcu2dsp_open_param_p open_param;
    open_param = (mcu2dsp_open_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    DSP_REMAP_SHARE_INFO(open_param->stream_out_param.ble.p_share_info, n9_dsp_share_info_ptr);

    n9_ble_ul_if.source   = dsp_open_stream_in(open_param);
    n9_ble_ul_if.sink     = dsp_open_stream_out(open_param);
    n9_ble_ul_if.transform = NULL;

    DSP_Callback_PreloaderConfig(n9_ble_ul_if.pfeature_table, open_param->audio_scenario_type);
    DSP_MW_LOG_I("[BLE] UL OPEN Finish\r\n", 0);
}

void CB_N9_BLE_UL_START(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    U32 gpt_timer;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_timer);
    DSP_MW_LOG_I("[BLE] UL START gpt %d\r\n", 1, gpt_timer);

    mcu2dsp_start_param_p start_param;
    start_param = (mcu2dsp_start_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);

    dsp_start_stream_in(start_param, n9_ble_ul_if.source);
    dsp_start_stream_out(start_param, n9_ble_ul_if.sink);

    n9_ble_ul_if.source->param.audio.AfeBlkControl.u4awsflag = 1;
#ifdef AIR_UL_FIX_RESOLUTION_32BIT
    stream_feature_configure_resolution((stream_feature_list_ptr_t)n9_ble_ul_if.pfeature_table, RESOLUTION_32BIT, 0);
#endif
    n9_ble_ul_if.transform = TrasformAudio2Audio(n9_ble_ul_if.source, n9_ble_ul_if.sink, n9_ble_ul_if.pfeature_table);
    if (n9_ble_ul_if.transform == NULL) {
        DSP_MW_LOG_E("[BLE] UL transform failed", 0);
    }
    DSP_MW_LOG_I("[BLE] UL START Finish\r\n", 0);
}

void CB_N9_BLE_UL_STOP(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);

    DSP_MW_LOG_I("[BLE] UL STOP\r\n", 0);

    if (g_ul_sw_timer_handle != 0) {
        hal_gpt_sw_stop_timer_us(g_ul_sw_timer_handle);
        hal_gpt_sw_free_timer(g_ul_sw_timer_handle);
        g_ul_sw_timer_handle = 0;
    }

    if (n9_ble_ul_if.transform != NULL) {
        StreamDSPClose(n9_ble_ul_if.transform->source, n9_ble_ul_if.transform->sink, msg.ccni_message[0] >> 16 | 0x8000);
    }
    n9_ble_ul_if.transform = NULL;

#if defined(MTK_ANC_ENABLE) && defined(AIR_ANC_SCENARIO_CONTROL_GAIN_ENABLE)
    //anc call mode off
    dsp_anc_apply_ramp_gain_by_audio_scenario(DSP_ANC_CONTROL_AUDIO_SCENARIO_TYPE_CALL, false, NULL);
#endif

    DSP_MW_LOG_I("[BLE] UL STOP Finish\r\n", 0);
}

#ifdef AIR_BT_LE_LC3_ENABLE
//EXTERN void LC3_TabDeinit(void);
//EXTERN void LC3_MemDeinit (void);
EXTERN void LC3_Deinit (void);
EXTERN U8 g_lc3_user_cnt;
#endif

void CB_N9_BLE_UL_CLOSE(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("[BLE] UL CLOSE\r\n", 0);
#ifdef AIR_DCHS_MODE_ENABLE
    dsp_uart_ul_clear_tx_buffer();
    dsp_uart_ul_clear_rx_buffer();
    dchs_send_unlock_sleep_msg(false);
#endif
    DSP_Callback_UnloaderConfig(n9_ble_ul_if.pfeature_table, n9_ble_ul_if.source->scenario_type);
#ifdef AIR_WIRELESS_MIC_TX_ENABLE
#ifdef AIR_AUDIO_ULD_ENCODE_ENABLE
        uld_enc_port_t *uld_port = stream_codec_encoder_uld_get_port(n9_ble_ul_if.source);
        AUDIO_ASSERT(uld_port != NULL);
        stream_codec_encoder_uld_deinit(uld_port);
#endif
#ifdef AIR_LD_NR_ENABLE
        ld_nr_port_t *ld_nr_port;
        ld_nr_port = stream_function_ld_nr_get_port(n9_ble_ul_if.source);
        stream_function_ld_nr_deinit(ld_nr_port);
        preloader_pisplit_free_memory(p_wireless_mic_ld_nr_key);
        p_wireless_mic_ld_nr_key = NULL;
        DSP_MW_LOG_I("[LD_NR]p_wireless_mic_ld_nr_key free",0);
#endif /* AIR_LD_NR_ENABLE */
#ifdef AIR_SOFTWARE_DRC_ENABLE
        sw_compander_port_t *drc_port;
        drc_port = stream_function_sw_compander_get_port(n9_ble_ul_if.source);
        stream_function_sw_compander_deinit(drc_port);
#endif
#if defined(AIR_AUDIO_VOLUME_MONITOR_ENABLE)
        volume_estimator_port_t *meter_port = volume_estimator_get_port(n9_ble_ul_if.source);
        volume_estimator_deinit(meter_port);
        if (p_wireless_mic_tx_volume_monitor_nvkey_buf) {
            preloader_pisplit_free_memory(p_wireless_mic_tx_volume_monitor_nvkey_buf);
            p_wireless_mic_tx_volume_monitor_nvkey_buf = NULL;
        }
        if (p_wireless_mic_tx_volume_monitor_queue_buf) {
            preloader_pisplit_free_memory(p_wireless_mic_tx_volume_monitor_queue_buf);
            p_wireless_mic_tx_volume_monitor_queue_buf = NULL;
        }
#endif
#endif /* AIR_WIRELESS_MIC_TX_ENABLE */
#ifdef PRELOADER_ENABLE
    //DSP_PIC_FeatureDeinit(n9_ble_ul_if.pfeature_table);
#endif
#ifdef AIR_BT_LE_LC3_ENABLE
    if (n9_ble_ul_if.sink->param.n9ble.codec_type == BT_BLE_CODEC_LC3) {
        LC3_Enc_Deinit();
        if(!g_lc3_user_cnt){
            #ifndef AIR_DSP_MEMORY_REGION_ENABLE
            EXTERN U8 DSPMEM_Free(TaskHandle_t  DSPTask, VOID *usingPtr);
            DSPMEM_Free(DAV_TASK_ID, (VOID*)LE_AUDIO_MAGIC_STREAM_PTR);//for UL/DL on different tasks
            #endif
            #ifdef AIR_DSP_MEMORY_REGION_ENABLE
            DspMemoryManagerReturnStatus_t memory_return_status;
            memory_return_status = DspMemoryManager_ReleaseGroupMemory(Component_LE_CALL, SubComponent_LECALL_LC3_PLC_STD, &lc3_plc_memory_addr);
            if(lc3_ul_pic_type = LC3I_UL_CALL_10_MS){
                memory_return_status = DspMemoryManager_ReleaseGroupMemory(Component_LE_CALL, SubComponent_LECALL_LC3_FFT, &lc3_fft_memory_addr);
            }else{
                memory_return_status = DspMemoryManager_ReleaseGroupMemory(Component_LE_CALL, SubComponent_LECALL_LC3_FFT, &lc3_fft_memory_addr);
            }
            #endif
        }
    }
#endif
    SourceClose(n9_ble_ul_if.source);
    SinkClose(n9_ble_ul_if.sink);

    if (g_ul_sw_timer_handle != 0) {
        hal_gpt_sw_stop_timer_us(g_ul_sw_timer_handle);
        hal_gpt_sw_free_timer(g_ul_sw_timer_handle);
        g_ul_sw_timer_handle = 0;
    }

    memset(&n9_ble_ul_if,0,sizeof(CONNECTION_IF));
#ifdef AIR_AUDIO_HARDWARE_ENABLE
#ifdef AIR_SOFTWARE_GAIN_ENABLE
        call_sw_gain_deinit();
#endif
#endif
    DSP_MW_LOG_I("[BLE] UL CLOSE Finish\r\n", 0);
}

void CB_N9_BLE_UL_PLAY(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);

    if (n9_ble_ul_if.transform != NULL) {
        DSP_MW_LOG_I("[BLE] UL PLAY\r\n", 0);
#if 0
        afe_set_memory_path_enable(AUDIO_DIGITAL_BLOCK_MEM_VUL1, true, true);
        //afe_enable_audio_irq(afe_irq_request_number(AUDIO_DIGITAL_BLOCK_MEM_VUL1), n9_ble_ul_if.source->param.audio.rate, n9_ble_ul_if.source->param.audio.count);

        if (n9_ble_ul_if.source->param.audio.echo_reference) {
            afe_set_memory_path_enable(AUDIO_DIGITAL_BLOCK_MEM_AWB, true, true);
        }
#endif
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &(n9_ble_ul_if.sink->param.n9ble.ul_play_gpt));
    } else {
        DSP_MW_LOG_I("[BLE] UL PLAY when tansform not exist\r\n", 0);
    }
}

void CB_N9_BLE_UL_SUSPEND(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("[BLE] UL SUSPEND\r\n", 0);
    dsp_trigger_suspend(n9_ble_ul_if.source, n9_ble_ul_if.sink);

}

void CB_N9_BLE_UL_RESUME(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("[BLE] UL RESUME\r\n", 0);
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
    dsp_detachable_config(&n9_ble_ul_if, msg);
#endif
    dsp_trigger_resume(n9_ble_ul_if.source, n9_ble_ul_if.sink);
}

void CB_N9_BLE_UL_SET_VOLUME(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{

#if defined(AIR_BLE_UL_SW_GAIN_CONTROL_ENABLE) && defined(AIR_SOFTWARE_GAIN_ENABLE)
    N9Ble_UL_Set_SW_Gain((int32_t)(msg.ccni_message[1]));
#endif
    UNUSED(msg);
    UNUSED(ack);
}

bool g_n9_ble_dl_open_flag = false;
extern uint32_t ble_dl_gpt_start;

void CB_N9_BLE_DL_OPEN(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("[BLE] DL OPEN\r\n", 0);
    ble_dl_gpt_start = 0;

    mcu2dsp_open_param_p open_param;
    open_param = (mcu2dsp_open_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    DSP_MW_LOG_I("[BLE] share info add: 0x%x\r\n", 1, open_param->stream_in_param.ble.p_share_info);
    DSP_REMAP_SHARE_INFO(open_param->stream_in_param.ble.p_share_info, n9_dsp_share_info_ptr);
    DSP_MW_LOG_I("[BLE] share info add: 0x%x ,share buffer startadd: 0x%x\r\n", 2,open_param->stream_in_param.ble.p_share_info, open_param->stream_in_param.ble.p_share_info->start_addr);
#ifdef AIR_AIRDUMP_ENABLE
        DSP_REMAP_SHARE_INFO(open_param->stream_in_param.ble.p_air_dump_buf, uint32_t *);
        DSP_MW_LOG_I("[BLE] airdump addr: 0x%x", 1,open_param->stream_in_param.ble.p_air_dump_buf);
#endif
    n9_ble_dl_if.source   = dsp_open_stream_in(open_param);
    g_n9_ble_dl_open_flag = true;
    n9_ble_dl_if.sink     = dsp_open_stream_out(open_param);
    g_n9_ble_dl_open_flag = false;
    n9_ble_dl_if.transform = NULL;

    DSP_Callback_PreloaderConfig(n9_ble_dl_if.pfeature_table, open_param->audio_scenario_type);
#ifdef AIR_ANC_ADAPTIVE_CLOCK_CONTROL_ENABLE
    dsp_adapt_anc_send_unstable_status(AUDIO_ADAPTIVE_ANC_CLK_CTRL_TYPE_DL_BLE, 1);
#endif

    DSP_MW_LOG_I("[BLE] DL OPEN Finish\r\n", 0);
}

void CB_N9_BLE_DL_START(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("[BLE] DL START\r\n", 0);

    mcu2dsp_start_param_p start_param;
    start_param = (mcu2dsp_start_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);


    dsp_start_stream_in(start_param, n9_ble_dl_if.source);
    dsp_start_stream_out(start_param, n9_ble_dl_if.sink);
    n9_ble_dl_if.sink->param.audio.AfeBlkControl.u4awsflag = 1;
    n9_ble_dl_if.transform = TrasformAudio2Audio(n9_ble_dl_if.source, n9_ble_dl_if.sink, n9_ble_dl_if.pfeature_table);
    #ifdef AIR_BT_LE_LC3PLUS_ENABLE
    if (n9_ble_dl_if.source->param.n9ble.codec_type == BT_BLE_CODEC_LC3PLUS)
    {
        n9_ble_dl_if.source->param.n9ble.plc_state_len = sizeof(lc3plus_dec_frame_status_t);
        stream_feature_configure_resolution((stream_feature_list_ptr_t)n9_ble_dl_if.pfeature_table, RESOLUTION_32BIT, CONFIG_DECODER);
    }
    #endif
    #ifdef AIR_AUDIO_ULD_DECODE_ENABLE
    if (n9_ble_dl_if.source->param.n9ble.codec_type == BT_BLE_CODEC_ULD)
    {
        n9_ble_dl_if.source->param.n9ble.plc_state_len = sizeof(uld_dec_frame_status_t);
        stream_feature_configure_resolution((stream_feature_list_ptr_t)n9_ble_dl_if.pfeature_table, RESOLUTION_32BIT, CONFIG_DECODER);
    }
    #endif
    #ifdef AIR_CELT_DEC_V2_ENABLE
    if (n9_ble_dl_if.source->param.n9ble.codec_type == BT_BLE_CODEC_VENDOR)
    {
        n9_ble_dl_if.source->param.n9ble.plc_state_len = sizeof(celt_dec_frame_status_t);
        stream_feature_configure_resolution((stream_feature_list_ptr_t)n9_ble_dl_if.pfeature_table, RESOLUTION_16BIT, CONFIG_DECODER);
    }
    #endif
    BleDlStopFlag = FALSE;
    if (n9_ble_dl_if.transform == NULL) {
        DSP_MW_LOG_E("[BLE] DL transform failed", 0);
    }
    //*(volatile uint32_t *)0x70000F58 = 0x00000208; //ZCD_CON2
    #ifdef AIR_SILENCE_DETECTION_ENABLE
    Sink_Audio_SilenceDetection_Register(n9_ble_dl_if.sink);
    #endif
    DSP_MW_LOG_I("[BLE] DL START Finished\r\n", 0);
#if defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
    if (n9_ble_dl_if.source->param.n9ble.context_type == BLE_CONTEXT_CONVERSATIONAL) {
        dsp_llf_set_audio_dl_status(LLF_DL_MIX_TYPE_ESCO, true);
    } else {
        dsp_llf_set_audio_dl_status(LLF_DL_MIX_TYPE_A2DP, true);
    }
#endif

}

void CB_N9_BLE_DL_STOP(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("[BLE] DL STOP\r\n", 0);
    #ifdef AIR_SILENCE_DETECTION_ENABLE
    Sink_Audio_SilenceDetection_Unregister(n9_ble_dl_if.sink);
    #endif
    BleDlStopFlag = TRUE;
    if (n9_ble_dl_if.transform != NULL) {
        StreamDSPClose(n9_ble_dl_if.transform->source, n9_ble_dl_if.transform->sink, msg.ccni_message[0] >> 16 | 0x8000);
    }
    n9_ble_dl_if.transform = NULL;
    DSP_MW_LOG_I("[BLE] DL STOP Finish\r\n", 0);

#if defined(MTK_ANC_ENABLE) && defined(AIR_ANC_SCENARIO_CONTROL_GAIN_ENABLE)
    //anc call mode off
    dsp_anc_apply_ramp_gain_by_audio_scenario(DSP_ANC_CONTROL_AUDIO_SCENARIO_TYPE_CALL, false, NULL);
#endif

}

void CB_N9_BLE_DL_CLOSE(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("[BLE] DL CLOSE\r\n", 0);

    DSP_Callback_UnloaderConfig(n9_ble_dl_if.pfeature_table, n9_ble_dl_if.source->scenario_type);
#ifdef PRELOADER_ENABLE
    //DSP_PIC_FeatureDeinit(n9_ble_dl_if.pfeature_table);
#endif
#ifdef AIR_BT_LE_LC3_ENABLE
    if (n9_ble_dl_if.source->param.n9ble.codec_type == BT_BLE_CODEC_LC3) {
        LC3_Dec_Deinit();
#ifdef AIR_HEARING_PROTECTION_ENABLE
        stream_function_cpd_deinitialize(true);
#ifdef AIR_BTA_IC_PREMIUM_G2
        g_hearing_protection_enable = false;
#endif
#endif
        if(!g_lc3_user_cnt){
        #ifndef AIR_DSP_MEMORY_REGION_ENABLE
            EXTERN U8 DSPMEM_Free(TaskHandle_t  DSPTask, VOID *usingPtr);
            DSPMEM_Free(DAV_TASK_ID, (VOID*)LE_AUDIO_MAGIC_STREAM_PTR);//for UL/DL on different task
        #endif
        #ifdef AIR_DSP_MEMORY_REGION_ENABLE
            DspMemoryManagerReturnStatus_t memory_return_status;
            switch(lc3_dl_pic_type){
                case LC3I_DL_AUDIO_10_MS:
                    memory_return_status = DspMemoryManager_ReleaseGroupMemory(Component_LE_AUDIO, SubComponent_LEAUDIO_LC3_PLC_ADV, &lc3_plc_memory_addr);
                    memory_return_status = DspMemoryManager_ReleaseGroupMemory(Component_LE_AUDIO, SubComponent_LEAUDIO_LC3_FFT, &lc3_fft_memory_addr);
                    break;
                case LC3I_DL_AUDIO_7P5_MS:
                    memory_return_status = DspMemoryManager_ReleaseGroupMemory(Component_LE_AUDIO, SubComponent_LEAUDIO_LC3_PLC_ADV, &lc3_plc_memory_addr);
                    memory_return_status = DspMemoryManager_ReleaseGroupMemory(Component_LE_AUDIO, SubComponent_LEAUDIO_LC3_FFT, &lc3_fft_memory_addr);
                    break;
                case LC3I_DL_CALL_10_MS:
                    memory_return_status = DspMemoryManager_ReleaseGroupMemory(Component_LE_CALL, SubComponent_LECALL_LC3_PLC_STD, &lc3_plc_memory_addr);
                    memory_return_status = DspMemoryManager_ReleaseGroupMemory(Component_LE_CALL, SubComponent_LECALL_LC3_FFT, &lc3_fft_memory_addr);
                    break;
                case LC3I_DL_CALL_7P5_MS:
                    memory_return_status = DspMemoryManager_ReleaseGroupMemory(Component_LE_CALL, SubComponent_LECALL_LC3_PLC_STD, &lc3_plc_memory_addr);
                    memory_return_status = DspMemoryManager_ReleaseGroupMemory(Component_LE_CALL, SubComponent_LECALL_LC3_FFT, &lc3_fft_memory_addr);
                    break;
                default:
                    break;
            }
        #endif
        }
    }
#endif
    SourceClose(n9_ble_dl_if.source);
    SinkClose(n9_ble_dl_if.sink);
    memset(&n9_ble_dl_if,0,sizeof(CONNECTION_IF));
    ble_reset_dl_play_en_time();
#ifdef AIR_ANC_ADAPTIVE_CLOCK_CONTROL_ENABLE
    dsp_adapt_anc_send_unstable_status(AUDIO_ADAPTIVE_ANC_CLK_CTRL_TYPE_DL_BLE, 0);
#endif
#if defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
    if (n9_ble_dl_if.source->param.n9ble.context_type == BLE_CONTEXT_CONVERSATIONAL) {
        dsp_llf_set_audio_dl_status(LLF_DL_MIX_TYPE_ESCO, false);
    } else {
        dsp_llf_set_audio_dl_status(LLF_DL_MIX_TYPE_A2DP, false);
    }
#endif
    DSP_MW_LOG_I("[BLE] DL CLOSE Finish\r\n", 0);
}

void CB_N9_BLE_CLOCKSKEW(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    //TBD
}

void CB_N9_BLE_DL_SUSPEND(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("[BLE] DL SUSPEND\r\n", 0);

    dsp_trigger_suspend(n9_ble_dl_if.source, n9_ble_dl_if.sink);

}

void CB_N9_BLE_DL_RESUME(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("[BLE] DL RESUME\r\n", 0);

    dsp_trigger_resume(n9_ble_dl_if.source, n9_ble_dl_if.sink);
}

void CB_N9_BLE_ULIRQ(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    //printf("CB_N9_SCO_ULIRQ\r\n");
    //n9_sco_ul_if.source->transform->Handler(n9_sco_ul_if.source,n9_sco_ul_if.sink);
    //vTaskResume(DAV_TASK_ID);
}

extern bool N9_BLE_SOURCE_ROUTINE(void);
void CB_N9_BLE_DLIRQ(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    /*hal_gpio_set_output(HAL_GPIO_0, 1);
    hal_gpio_set_output(HAL_GPIO_0, 0);*/
    if (N9_BLE_SOURCE_ROUTINE()) {
        return;
    }

    UNUSED(msg);
    UNUSED(ack);
    if ((Source_blks[SOURCE_TYPE_N9BLE] == NULL) || (n9_ble_dl_if.transform == NULL)) {
        DSP_MW_LOG_I("[BLE] Unexpected N9 DL IRQ", 0);
        return;
    }
    if (Source_blks[SOURCE_TYPE_N9BLE]->param.n9ble.share_info_base_addr->read_offset == Source_blks[SOURCE_TYPE_N9BLE]->param.n9ble.share_info_base_addr->write_offset) {
        Source_blks[SOURCE_TYPE_N9BLE]->param.n9ble.share_info_base_addr->bBufferIsFull = TRUE;
        DSP_MW_LOG_I("[BLE] DL bufferfull, Wo:%d, Ro:%d", 2, Source_blks[SOURCE_TYPE_N9BLE]->param.n9ble.share_info_base_addr->write_offset, Source_blks[SOURCE_TYPE_N9BLE]->param.n9ble.share_info_base_addr->read_offset);
    }
    //DSP_MW_LOG_I("[BLE] CB_N9_BLE_DLIRQ, Wo:%d, Ro:%d\r\n", 3, Source_blks[SOURCE_TYPE_N9BLE]->param.n9ble.share_info_base_addr->WriteOffset, Source_blks[SOURCE_TYPE_N9BLE]->param.n9ble.share_info_base_addr->ReadOffset);
    if (n9_ble_dl_if.source->param.n9ble.IsFirstIRQ) {
        /*hal_gpio_set_output(HAL_GPIO_1, 1);
        hal_gpio_set_output(HAL_GPIO_1, 0);*/

        n9_ble_dl_if.source->param.n9ble.IsFirstIRQ = FALSE;
        memset((U8*)Source_blks[SOURCE_TYPE_N9BLE]->streamBuffer.ShareBufferInfo.start_addr, 0, N9BLE_setting.N9Ble_source.Frame_Size);

        U32 gpt_timer;
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_timer);
        DSP_MW_LOG_I("[BLE] CB_N9_BLE_DLIRQ, First Wo:%d, Ro:%d,GPT : %d\r\n", 3, Source_blks[SOURCE_TYPE_N9BLE]->param.n9ble.share_info_base_addr->write_offset, Source_blks[SOURCE_TYPE_N9BLE]->param.n9ble.share_info_base_addr->read_offset, gpt_timer);
        if ((n9_ble_dl_if.source != NULL) && (n9_ble_dl_if.source->transform->Handler != NULL)) {
            n9_ble_dl_if.source->transform->Handler(n9_ble_dl_if.source, n9_ble_dl_if.sink);
            vTaskResume(n9_ble_dl_if.source->taskId);
        }
    }
    if (BleDlStopFlag == TRUE) {
        vTaskResume(n9_ble_dl_if.source->taskId);
    }
}

void CB_N9_BLE_MICIRQ(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    //printf("[BLE] CB_N9_SCO_MICIRQ\r\n");
    //n9_sco_ul_if.source->transform->Handler(n9_sco_ul_if.source,n9_sco_ul_if.sink);
    //vTaskResume(DAV_TASK_ID);
}


#define BLE_ULL_UL_TIME_MIC_MUTE        1000
#define BLE_ULL_UL_TIME_AVM_MARGIN      1000
#define BLE_ULL_UL_TIME_STREAM_PROCESS  6000

#define BLE_UL_TIME_STREAM_PROCESS      15000

#if defined(AIR_BTA_IC_STEREO_HIGH_G3)
#ifdef AIR_BT_ULL_SWB_ENABLE
#define ULL_UL_TIME_STREAM_PROCESS      10000
#else
#define ULL_UL_TIME_STREAM_PROCESS      8000
#endif
#endif

#define BLE_TIME_BT_CLOCK           625 /* unit with 312.5us */
#ifdef AIR_DCHS_MODE_ENABLE
#define BLE_UL_TIME_PLAY_EN_MARGIN  8500
#define BLE_ULL_TIME_PLAY_EN_MARGIN  45000
#define BLE_UL_VUL1_DEALY_TIME_STREAM_PROCESS  5000
#else
#define BLE_UL_TIME_PLAY_EN_MARGIN  1000
#endif
#define BLE_DL_TIME_PLAY_EN_MARGIN  16000

#define BIT_MASK(n)         (1UL << (n))
#define BT_CLOCK_TICK_MASK  (0x0FFFFFFC)
#define BT_CLOCK_MAX_WRAP   (0x10000000)
#define BLE_CURR_TIME_CHECK (3000)
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ U32 LC_Get_CurrBtClk(BTCLK *pCurrCLK, BTPHASE *pCurrPhase)
{
    *pCurrCLK = rBb->rAudioCtl.rRxClk;
    *pCurrPhase = rBb->rAudioCtl.rRxPhs;
    U32 rg = *((volatile uint32_t *)(CONN_BT_TIMCON_BASE + 0x011C));
    return rg;
}

/******************************************************************************
 *  LC_IsBTTimeExpired
 *  Descprition :
 *  return TRUE  : CurrentBTTime >= ExpiredBTTime
 *  return FALSE : CurrentBTTime <  ExpiredBTTime
 *  This API had considered wrap around case, because it check MSB of Valid Clock bit.
 *******************************************************************************/
bool LC_BtClock_IsBTTimeExpired(BTTIME_STRU_PTR pExpiredBTTime, BTTIME_STRU_PTR pCurrentBTTime)
{
    BTCLK ExpiredCLK = pExpiredBTTime->period & 0xFFFFFFC;
    BTCLK CurrentCLK = pCurrentBTTime->period & 0xFFFFFFC;
    BTCLK DeltaTime;

    DeltaTime = CurrentCLK - ExpiredCLK;
    if ((DeltaTime & BIT_MASK(27)) > 0) {
        //CurrentCLK <  ExpiredTime
        return FALSE;
    } else {
        //CurrentCLK >= ExpiredTime
        if (CurrentCLK == ExpiredCLK) { //Same CLK, We have to compare Phase
            return (pCurrentBTTime->phase >= pExpiredBTTime->phase);
        } else {
            return TRUE;
        }
    }
}
/**
 * LC_Add_us_FromA
 *
 * Add us from a => b = a + n
 *
 * @Author : Wen <wen.huang@airoha.com.tw>
 *
 * @n : In offset unit us
 * @pa : In
 * @pb : out
 * @Return : b = a + n
 */
VOID LC_Add_us_FromA(U32 n, BTTIME_STRU_PTR pa, BTTIME_STRU_PTR pb)
{
    //get cur clk and phase
    BTCLK a_t0 = pa->period & 0xFFFFFFC;
    U32 m;
    m = pa->phase + (n << 1);
    pb->period = (a_t0 + m / 625) &BT_CLOCK_TICK_MASK;
    pb->phase = m % 2500;
}

/**
 * LC_Subtract_us_Fromb
 *
 * Subtract us from b => a = b - n
 *
 * @Author : Wen <wen.huang@airoha.com.tw>
 *
 * @n : In offset unit us
 * @pa : out
 * @pb : in
 * @Return : b - n = a
 */
VOID LC_Subtract_us_Fromb(U32 n, BTTIME_STRU_PTR pa, BTTIME_STRU_PTR pb)
{
    BTCLK b_t0 = pb->period & 0xFFFFFFC;
    BTCLK a_t0;
    U32 x;

    n = n << 1; //change to unit 0.5us
    if (pb->phase >= n) {
        pa->phase = pb->phase - n;
        pa->period = (b_t0 + pa->phase / 625) &BT_CLOCK_TICK_MASK;
    } else {
        x = (n - pb->phase) % 2500;
        pa->phase = (2500 - x) % 2500;
        a_t0 = (BT_CLOCK_MAX_WRAP + b_t0 - ((((n - pb->phase) / 2500) + (pa->phase > 0)) * 4)) & BT_CLOCK_TICK_MASK;
        pa->period = a_t0 + pa->phase / 625;
    }
}

/**
 * LC_Get_Offset_FromAB
 *
 * get offset us between a and b
 *
 * @Author : Wen <wen.huang@airoha.com.tw>
 *
 * @pa : In Point a
 * @pb : In Point b
 * @Return : b-a = ? us , maximum value 134217726us
 */
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ U32 LC_Get_Offset_FromAB(BTTIME_STRU_PTR pa, BTTIME_STRU_PTR pb)
{
    BTCLK a_t0 = pa->period & 0xFFFFFFC;
    BTCLK b_t0 = pb->period & 0xFFFFFFC;
    BTCLK CLKOffset;
    U32 Phase;
    if (pa->period <= pb->period) {
        CLKOffset = (b_t0 - a_t0);
    } else {
        CLKOffset = (0xFFFFFFF - a_t0 + b_t0 + 1);
    }
    Phase = (CLKOffset * 625) - pa->phase + pb->phase;
    return (Phase >> 1);
}
U32 LC_Get_period_Offset_FromAB(BTTIME_STRU_PTR pa, BTTIME_STRU_PTR pb)
{
    BTCLK a_t0 = pa->period & 0xFFFFFFC;
    BTCLK b_t0 = pb->period & 0xFFFFFFC;
    BTCLK CLKOffset;
    if (pa->period <= pb->period) {
        CLKOffset = (b_t0 - a_t0);
    } else {
        CLKOffset = (0xFFFFFFF - a_t0 + b_t0 + 1);
    }
    return (CLKOffset >> 1);
}

#if defined (AIR_DCHS_MODE_ENABLE)
uint32_t dchs_ble_ul_handle;
uint32_t le_ull_gpt_count;
uint32_t delay_time = 5000;
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void hal_audio_gpt_trigger_mem(void)
{
    uint32_t savedmask = 0;
    uint32_t curr_cnt  = 0;
    U32 cur_native_bt_clk = 0, cur_native_bt_phase = 0;

    hal_nvic_save_and_set_interrupt_mask_special(&savedmask); // enter cirtical code region

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
    if (le_ull_gpt_count > curr_cnt) { // gpt register does not overflow
        // DSP_MW_LOG_I("[DCHS UL][hfp set value] trigger %d curr_cnt %u  tar %d", 3, __LINE__, curr_cnt, le_ull_gpt_count);
        while (1) {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
            if (curr_cnt >= le_ull_gpt_count) { // expire at time
                break;
            }
        }
    } else if (curr_cnt - le_ull_gpt_count > 0x7fffffff) { // gpt register overflow
        // DSP_MW_LOG_I("[DCHS UL][hfp set value] trigger %d curr_cnt %u  tar %d", 3, __LINE__, curr_cnt, le_ull_gpt_count);
        while (1) {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
            if (curr_cnt >= le_ull_gpt_count) { // expire at time
                if ((curr_cnt & 0x80000000) == 0x0) {
                    break;
                }
            }
        }
    } else {
        DSP_MW_LOG_E("[DCHS UL][LE/ULL uplink]Warning: already expire\r\n", 0);
        // AUDIO_ASSERT(0);
    }

    hal_audio_trigger_start_parameter_t start_parameter;
    start_parameter.memory_select = n9_ble_ul_if.source->param.audio.mem_handle.memory_select;;
    start_parameter.enable = true;
    hal_audio_set_value((hal_audio_set_value_parameter_t *)&start_parameter, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
    MCE_GetBtClk((BTCLK *)&cur_native_bt_clk, (BTPHASE *)&cur_native_bt_phase, DCHS_CLK_Offset);
    DSP_MW_LOG_I("[DCHS UL]LE/ULL uplink Trigger Mem success cur_native_bt_clk:%u,cur_native_bt_phase:%u",2,cur_native_bt_clk,cur_native_bt_phase);
    hal_gpt_sw_free_timer(dchs_ble_ul_handle);
    hal_nvic_restore_interrupt_mask_special(savedmask);
}
#endif
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void ble_init_ul_timer_callback(void *user_data)
{
    hal_audio_trigger_start_parameter_t start_parameter;

    UNUSED(user_data);

    /* Enable UL AFE path */
    start_parameter.memory_select = n9_ble_ul_if.source->param.audio.mem_handle.memory_select;
    start_parameter.enable = true;
#if defined (AIR_DCHS_MODE_ENABLE)
    uint32_t count_1, hfp_delay_count;
    S32 cur_native_bt_clk = 0,cur_native_bt_phase = 0;
    hal_sw_gpt_absolute_parameter_t  dchs_hfp_absolute_parameter;

    hal_gpt_sw_get_timer(&dchs_ble_ul_handle);
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &count_1);
    MCE_GetBtClk((BTCLK *)&cur_native_bt_clk, (BTPHASE *)&cur_native_bt_phase, DCHS_CLK_Offset);

    hfp_delay_count = delay_time;
    le_ull_gpt_count = count_1 + hfp_delay_count;
    dchs_hfp_absolute_parameter.absolute_time_count = count_1 + hfp_delay_count - 600;
    dchs_hfp_absolute_parameter.callback = (void*)hal_audio_gpt_trigger_mem;
    dchs_hfp_absolute_parameter.maxdelay_time_count = hfp_delay_count;
    dps_uart_relay_ul_mem_sync_info(delay_time/1000, cur_native_bt_clk, cur_native_bt_phase);
    hal_gpt_sw_start_timer_for_absolute_tick_1M(dchs_ble_ul_handle,&dchs_hfp_absolute_parameter);
#else
    hal_audio_set_value((hal_audio_set_value_parameter_t *)&start_parameter, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
#endif
    DSP_MW_LOG_I("[BLE] ble first sw ul callback, for initial setting", 0);
    if (g_ul_sw_timer_handle != 0) {
        hal_gpt_sw_free_timer(g_ul_sw_timer_handle);
        g_ul_sw_timer_handle = 0;
    }
    if ((n9_ble_ul_if.source != NULL) && (n9_ble_ul_if.source->transform != NULL)) {
        AudioCheckTransformHandle(n9_ble_ul_if.source->transform);
    }

#ifdef MTK_BLE_LATENCY_GPIO_DEBUG
    hal_gpio_toggle_pin(HAL_GPIO_11);
#endif
}

#ifdef AIR_WIRELESS_MIC_TX_ENABLE
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void ble_init_wireless_timer_callback(void *user_data)
{
    hal_audio_trigger_start_parameter_t start_parameter;
    BTTIME_STRU curr_time;
    U32 mask;
    U32 cnt=0, gpt_timer, gpt_timer2;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_timer);
    hal_nvic_save_and_set_interrupt_mask(&mask);
    while(1){
        curr_time.period = rBb->rAudioCtl.rRxClk;
        curr_time.phase = rBb->rAudioCtl.rRxPhs;
        if(LC_BtClock_IsBTTimeExpired(user_data, &curr_time) == true){
            /* Enable UL AFE path */
            start_parameter.memory_select = n9_ble_ul_if.source->param.audio.mem_handle.memory_select;
            start_parameter.enable = true;
            hal_audio_set_value((hal_audio_set_value_parameter_t *)&start_parameter, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
            if (g_ul_sw_timer_handle != 0) {
                hal_gpt_sw_free_timer(g_ul_sw_timer_handle);
                g_ul_sw_timer_handle = 0;
            }
            if ((n9_ble_ul_if.source != NULL) && (n9_ble_ul_if.source->transform != NULL)) {
                AudioCheckTransformHandle(n9_ble_ul_if.source->transform);
            }
            break;
        }
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_timer2);
        hal_gpt_get_duration_count(gpt_timer, gpt_timer2, &cnt);
        if (cnt > 500){  /*start timer 150ns before play en enable, so if >500, bt is abnormal. alse disable irq no more than 5ms*/
            DSP_MW_LOG_E("[BLE] ble sw ul can`t start, cnt=%d, please check if bt is sleep or if bt link loss, curr=0x%x,0x%x", 3, cnt, curr_time.period,curr_time.phase);
            break;
        }
    }
    hal_nvic_restore_interrupt_mask(mask);
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_timer2);
    DSP_MW_LOG_I("[BLE] ble first sw ul callback, for initial setting, wireless mic cnt=%d  us=%d", 2, cnt, gpt_timer2-gpt_timer);
#ifdef MTK_BLE_LATENCY_GPIO_DEBUG
    hal_gpio_toggle_pin(HAL_GPIO_11);
#endif
}
#endif

static void ble_init_dl_timer_callback(void *user_data)
{
    UNUSED(user_data);

    if ((n9_ble_dl_if.source != NULL) && (n9_ble_dl_if.source->transform != NULL)) {
        AudioCheckTransformHandle(n9_ble_dl_if.source->transform);
    }

    DSP_MW_LOG_I("[BLE] ble first sw dl callback", 0);

    if (g_dl_sw_timer_handle != 0) {
        hal_gpt_sw_free_timer(g_dl_sw_timer_handle);
        g_dl_sw_timer_handle = 0;
    }


#ifdef MTK_BLE_LATENCY_GPIO_DEBUG
    hal_gpio_toggle_pin(HAL_GPIO_8);
#endif
}
#ifndef ALIGN_4
#define ALIGN_4(_value)             (((_value) + 3) & ~3u)
#endif

static BTTIME_STRU play_en_time;
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void ble_trigger_ul_stream(ble_init_play_info_t *play_info)
{
    uint16_t curr_intra_clk;
    uint32_t curr_bt_clk, during_time, offset_time = 0, relocate_avm_wptr;
    BTTIME_STRU curr_time, expect_time, first_anchor_time;
    U32 rg_bt_sleep;

    hal_gpt_sw_get_timer(&g_ul_sw_timer_handle);

    #if 0
    // separated anchor
    first_anchor_time.period = play_info->ul_timestamp;
    first_anchor_time.phase = play_info->dl_timestamp_phase;
    #else
    // aligned anchor
    first_anchor_time.period = play_info->ISOAnchorClk;
    first_anchor_time.phase = play_info->ISOAnchorPhase;
    #endif

    n9_ble_ul_if.sink->param.n9ble.iso_interval = (play_info->iso_interval >>1)*625;
    n9_ble_ul_if.sink->param.n9ble.frame_per_iso = n9_ble_ul_if.sink->param.n9ble.iso_interval / n9_ble_ul_if.sink->param.n9ble.frame_interval;

#ifdef AIR_WIRELESS_MIC_TX_ENABLE
    BTTIME_STRU play_en_time_2;
    if (n9_ble_ul_if.sink->param.n9ble.context_type == BLE_CONTENT_TYPE_WIRELESS_MIC) {

        if (n9_ble_ul_if.sink->param.n9ble.codec_type == BT_BLE_CODEC_ULD) {
            offset_time = n9_ble_ul_if.source->param.audio.period * 1000 + 500;
        } else if (n9_ble_ul_if.sink->param.n9ble.codec_type == BT_BLE_CODEC_LC3PLUS) {
            offset_time = n9_ble_ul_if.source->param.audio.period * 1000 + 2000;
        }
        DSP_MW_LOG_I("[BLE][wireless_mic] offset_time = %d", 1, offset_time);
        LC_Subtract_us_Fromb(offset_time, &play_en_time, &first_anchor_time);
        n9_ble_ul_if.sink->param.n9ble.predict_timestamp = play_info->ul_timestamp;
        n9_ble_ul_if.sink->param.n9ble.predict_timestamp -= ((n9_ble_ul_if.sink->param.n9ble.frame_interval << 1) / BLE_TIME_BT_CLOCK);
    } else
#endif
    if (n9_ble_ul_if.sink->param.n9ble.context_type == BLE_CONTENT_TYPE_ULL_BLE) {
#ifdef ULL_UL_TIME_STREAM_PROCESS
        offset_time = n9_ble_ul_if.source->param.audio.period * 1000 + ULL_UL_TIME_STREAM_PROCESS;
#else
        offset_time = n9_ble_ul_if.source->param.audio.period * 1000 + BLE_ULL_UL_TIME_STREAM_PROCESS + BLE_ULL_UL_TIME_MIC_MUTE + BLE_ULL_UL_TIME_AVM_MARGIN - 2000;
#endif
        if (n9_ble_ul_if.sink->param.n9ble.is_lightmode) {
            offset_time = n9_ble_ul_if.source->param.audio.period * 1000 + 15000;
        }
        DSP_MW_LOG_I("[BLE][ULL2.0] offset_time = %d", 1, offset_time);
        LC_Subtract_us_Fromb(offset_time, &play_en_time, &first_anchor_time);
        n9_ble_ul_if.sink->param.n9ble.predict_timestamp = play_info->ul_timestamp;
        n9_ble_ul_if.sink->param.n9ble.predict_timestamp -= ((n9_ble_ul_if.sink->param.n9ble.frame_interval << 1) / BLE_TIME_BT_CLOCK);
    }
    else {
#if defined (AIR_DCHS_MODE_ENABLE)
        offset_time = n9_ble_ul_if.source->param.audio.period * 1000 + BLE_ULL_UL_TIME_STREAM_PROCESS + BLE_ULL_UL_TIME_MIC_MUTE + BLE_ULL_UL_TIME_AVM_MARGIN + BLE_UL_VUL1_DEALY_TIME_STREAM_PROCESS;
#else
        offset_time = n9_ble_ul_if.source->param.audio.period * 1000 * ((n9_ble_ul_if.sink->param.n9ble.frame_per_iso +1 )>>1)  + BLE_UL_TIME_STREAM_PROCESS;
        if ((n9_ble_ul_if.sink->param.n9ble.frame_per_iso != 1)&&(n9_ble_ul_if.sink->param.n9ble.frame_interval == 10000)&&(n9_ble_ul_if.source->param.audio.period * 1000 == 15000))
        {offset_time += (5000 * n9_ble_ul_if.sink->param.n9ble.frame_per_iso);}
#endif
        /* set ul play en before first anchor*/
        LC_Subtract_us_Fromb(offset_time, &play_en_time, &first_anchor_time);
        n9_ble_ul_if.sink->param.n9ble.predict_timestamp = play_info->ul_timestamp - ((n9_ble_ul_if.sink->param.n9ble.iso_interval<<1)/BLE_TIME_BT_CLOCK);

        /*Workaround BTA-37629*/
        n9_ble_ul_if.source->streamBuffer.BufferInfo.ReadOffset -= 8;

    }

    rg_bt_sleep = LC_Get_CurrBtClk(&curr_bt_clk, &curr_intra_clk);
    if (rg_bt_sleep)
    {
        DSP_MW_LOG_E("[BLE][LC] DSP get current bt clk when sleep, 0x%x", 1, rg_bt_sleep);
    }
    curr_time.period = curr_bt_clk;
    curr_time.phase = curr_intra_clk;
#if defined (AIR_DCHS_MODE_ENABLE)
    if (n9_ble_ul_if.sink->param.n9ble.context_type == BLE_CONTENT_TYPE_ULL_BLE) {
        LC_Add_us_FromA(BLE_ULL_TIME_PLAY_EN_MARGIN, &curr_time, &expect_time);
    }else{
        LC_Add_us_FromA(BLE_UL_TIME_PLAY_EN_MARGIN, &curr_time, &expect_time);
    }
#else
    LC_Add_us_FromA(BLE_UL_TIME_PLAY_EN_MARGIN, &curr_time, &expect_time);
#endif

    relocate_avm_wptr = play_info->ul_packet_counter;
    while (LC_BtClock_IsBTTimeExpired(&play_en_time, &expect_time) == true) {
        if (BLE_CURR_TIME_CHECK < LC_Get_period_Offset_FromAB(&play_en_time, &expect_time)){
            DSP_MW_LOG_E("[BLE] UL clock abnormal %d %d", 2, play_en_time.period,expect_time.period);
            return;
        }
        LC_Add_us_FromA(n9_ble_ul_if.sink->param.n9ble.frame_interval, &play_en_time, &play_en_time);
        relocate_avm_wptr++;
        n9_ble_ul_if.sink->param.n9ble.predict_timestamp += ((n9_ble_ul_if.sink->param.n9ble.frame_interval << 1) / BLE_TIME_BT_CLOCK);
    }


    /* re-locate the RPTR of first frame */
    relocate_avm_wptr %= N9BLE_setting.N9Ble_sink.Buffer_Frame_Num;
    relocate_avm_wptr *= N9BLE_setting.N9Ble_sink.Frame_Size;
    N9Ble_update_writeoffset_share_information(n9_ble_ul_if.sink, relocate_avm_wptr);
    n9_ble_ul_if.sink->param.n9ble.crc_init = n9_ble_ul_if.sink->param.n9ble.share_info_base_addr->asi_cur;//CRC init value saved in asi_current
    DSP_MW_LOG_I("[BLE][sink] initial writeoffset %d ts %d crc_init %d source irq %d", 4, relocate_avm_wptr, n9_ble_ul_if.sink->param.n9ble.predict_timestamp,n9_ble_ul_if.sink->param.n9ble.crc_init,n9_ble_ul_if.source->param.audio.period);

    DSP_MW_LOG_I("[BLE] UL first_anchor_time %d, %d, curr_time %d, %d, expect_time %d, %d, play_en_time %d, %d", 8,
                 first_anchor_time.period, first_anchor_time.phase,
                 curr_time.period, curr_time.phase,
                 expect_time.period, expect_time.phase,
                 play_en_time.period, play_en_time.phase);
#ifdef AIR_WIRELESS_MIC_TX_ENABLE
    if (n9_ble_ul_if.sink->param.n9ble.context_type == BLE_CONTENT_TYPE_WIRELESS_MIC) {
        LC_Subtract_us_Fromb(150, &play_en_time_2, &play_en_time);
    }
#endif
    U32 mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    rg_bt_sleep = LC_Get_CurrBtClk(&curr_bt_clk, &curr_intra_clk);
    curr_time.period = curr_bt_clk;
    curr_time.phase = curr_intra_clk;
#ifdef AIR_WIRELESS_MIC_TX_ENABLE
    if (n9_ble_ul_if.sink->param.n9ble.context_type == BLE_CONTENT_TYPE_WIRELESS_MIC) {
        during_time = LC_Get_Offset_FromAB(&curr_time, &play_en_time_2);
        hal_gpt_sw_start_timer_us(g_ul_sw_timer_handle, during_time, ble_init_wireless_timer_callback, &play_en_time);
    } else
#endif
    {
#if defined (AIR_DCHS_MODE_ENABLE)
    during_time = LC_Get_Offset_FromAB(&curr_time, &play_en_time) - delay_time;
#else
    during_time = LC_Get_Offset_FromAB(&curr_time, &play_en_time);
#endif
    hal_gpt_sw_start_timer_us(g_ul_sw_timer_handle, during_time, ble_init_ul_timer_callback, NULL);
    }
    hal_nvic_restore_interrupt_mask(mask);
    if (rg_bt_sleep)
    {
        DSP_MW_LOG_E("[BLE][LC] DSP get current bt clk when sleep, 0x%x", 1, rg_bt_sleep);
        #ifdef AIR_WIRELESS_MIC_TX_ENABLE
        AUDIO_ASSERT(FALSE);
        #endif
    }
    DSP_MW_LOG_I("[BLE] ble_trigger_ul_stream(), during_time = %d", 1, during_time);
}
static TimerHandle_t ble_playen_check_timer = NULL;

static void ble_playen_check_timer_callback(TimerHandle_t xTimer)
{
    UNUSED(xTimer);
    //DSP_MW_LOG_I("ble PLANEN_CHECK_TIMER", 0);
    if ((Sink_blks[SINK_TYPE_AUDIO] != NULL)&&(Sink_blks[SINK_TYPE_AUDIO]->param.audio.irq_exist == FALSE)){
    //if ((n9_ble_dl_if.transform != NULL) && (n9_ble_dl_if.sink->param.audio.irq_exist == FALSE)) {
        DSP_MW_LOG_I("Play en check, native bt_clk 0x%x :0x%x, play en  bt_clk 0x%x :0x%x, enable :0x%x", 5, rBb->rClkCtl.rNativeClock, rBb->rClkCtl.rNativePhase, *((volatile uint32_t *)(CONN_BT_TIMCON_BASE + 0x0204)), *((volatile uint32_t *)(CONN_BT_TIMCON_BASE + 0x0208)), *((volatile uint8_t *)(CONN_BT_TIMCON_BASE + 0x0200)));

    } else {
        xTimerStop(ble_playen_check_timer, 0);
    }
}

#ifndef ALIGN_4
#define ALIGN_4(_value)             (((_value) + 3) & ~3u)
#endif

static void ble_trigger_dl_stream(ble_init_play_info_t *play_info)
{
    uint16_t native_intra_clk;
    uint32_t during_time, native_bt_clk, retransmission_window;
    BTTIME_STRU play_en_time, curr_time, expect_time;
    U8 *read_ptr;
    LE_AUDIO_HEADER *buf_header;
    U32 rg_bt_sleep;

    /*
      * decide the time of play en
      *     - retransmission window
      */
    retransmission_window = ((play_info->dl_retransmission_window_clk) >> 2) * 1250 + play_info->dl_retransmission_window_phase / 2;
    play_en_time.period = play_info->dl_timestamp_clk;
    play_en_time.phase = play_info->dl_timestamp_phase;
    LC_Add_us_FromA(retransmission_window, &play_en_time, &play_en_time);
    /* save dl play_en time */
    if (ble_check_dl_play_en_time() == FALSE) {
        ble_set_dl_play_en_time(&play_en_time);
    }

    rg_bt_sleep = LC_Get_CurrBtClk(&curr_time.period, &curr_time.phase);
    if (rg_bt_sleep)
    {
        DSP_MW_LOG_E("[BLE][LC] DSP get current bt clk when sleep, 0x%x", 1, rg_bt_sleep);
    }

    LC_Add_us_FromA(BLE_DL_TIME_PLAY_EN_MARGIN, &curr_time, &expect_time);
    if (n9_ble_dl_if.source->param.n9ble.dual_cis_status == DUAL_CIS_WAITING_SUB)
    {
#ifdef AIR_BTA_IC_PREMIUM_G3
        n9_ble_dl_if.source->param.n9ble.sub_share_info_base_addr = (n9_dsp_share_info_ptr)((n9_ble_dl_if.source->param.n9ble.sub_share_info_base_addr->start_addr) | 0x60000000);
#else
        n9_ble_dl_if.source->param.n9ble.sub_share_info_base_addr = (n9_dsp_share_info_ptr)hal_memview_cm4_to_dsp0((U32)n9_ble_dl_if.source->param.n9ble.sub_share_info_base_addr->start_addr);
#endif
        /* Init TimeStamp & _reserved_byte_0Dh */
        DSP_MW_LOG_I("[BLE] Init Sub TimeStamp & _reserved_byte_0Dh", 0);
        U32 block_size = ALIGN_4(n9_ble_dl_if.source->param.n9ble.frame_length + BLE_AVM_FRAME_HEADER_SIZE);
        for (int i = 0; i < N9BLE_setting.N9Ble_source.Buffer_Frame_Num; i++) {
            read_ptr = (U8 *)((U8 *)n9_ble_dl_if.source->param.n9ble.sub_share_info_base_addr + i * block_size);
            buf_header = (LE_AUDIO_HEADER *)read_ptr;
            buf_header->TimeStamp = BLE_AVM_INVALID_TIMESTAMP;
            buf_header->_reserved_byte_0Dh = 0x00;
        }
        U32 shift_offset = ((play_info->dl_packet_counter + n9_ble_dl_if.source->param.n9ble.predict_frame_counter)%N9BLE_setting.N9Ble_source.Buffer_Frame_Num);
        while ((play_info->dl_timestamp_clk +  ((U32)(play_info->dl_ft)*play_info->iso_interval) + 4) < n9_ble_dl_if.source->param.n9ble.predict_timestamp)
        {
            play_info->dl_timestamp_clk += play_info->iso_interval;
            shift_offset += n9_ble_dl_if.source->param.n9ble.frame_per_iso;
        }
        shift_offset += N9BLE_setting.N9Ble_source.Buffer_Frame_Num - (n9_ble_dl_if.source->streamBuffer.ShareBufferInfo.read_offset/(ALIGN_4(n9_ble_dl_if.source->param.n9ble.frame_length + BLE_AVM_FRAME_HEADER_SIZE)));
        shift_offset %= N9BLE_setting.N9Ble_source.Buffer_Frame_Num;
        n9_ble_dl_if.source->param.n9ble.dual_cis_buffer_offset = (((play_info->dl_timestamp_clk +  ((U32)(play_info->dl_ft)*play_info->iso_interval) + 4 - n9_ble_dl_if.source->param.n9ble.predict_timestamp)*625/2/n9_ble_dl_if.source->param.n9ble.frame_interval)*(U32)(ALIGN_4(n9_ble_dl_if.source->param.n9ble.frame_length + BLE_AVM_FRAME_HEADER_SIZE)))%n9_ble_dl_if.source->streamBuffer.ShareBufferInfo.length;
        //Default offset, could be wrong but will be correct later
        //DSP_MW_LOG_I("[BLE] Sub CIS dual_cis_buffer_offset : %d mat : %d %d ", 3,n9_ble_dl_if.source->param.n9ble.dual_cis_buffer_offset,shift_offset,(ALIGN_4(n9_ble_dl_if.source->param.n9ble.frame_length + BLE_AVM_FRAME_HEADER_SIZE)));
        n9_ble_dl_if.source->param.n9ble.dual_cis_buffer_offset = (n9_ble_dl_if.source->streamBuffer.ShareBufferInfo.length - n9_ble_dl_if.source->param.n9ble.dual_cis_buffer_offset + (shift_offset*(U32)(ALIGN_4(n9_ble_dl_if.source->param.n9ble.frame_length + BLE_AVM_FRAME_HEADER_SIZE))))%n9_ble_dl_if.source->streamBuffer.ShareBufferInfo.length;
        DSP_MW_LOG_I("[BLE] Sub CIS start ts : %d offset : %d %d ", 3, play_info->dl_timestamp_clk,n9_ble_dl_if.source->param.n9ble.dual_cis_buffer_offset,n9_ble_dl_if.source->streamBuffer.ShareBufferInfo.read_offset);

        if ((play_info->dl_avm_info_addr&0xffff) == ((U32)n9_ble_dl_if.source->param.n9ble.share_info_base_addr&0xffff))
        {
            U32 swap_offset;
            swap_offset = n9_ble_dl_if.source->streamBuffer.ShareBufferInfo.read_offset;
            n9_ble_dl_if.source->streamBuffer.ShareBufferInfo.read_offset = (swap_offset + n9_ble_dl_if.source->param.n9ble.dual_cis_buffer_offset)%n9_ble_dl_if.source->streamBuffer.ShareBufferInfo.length;
            n9_ble_dl_if.source->param.n9ble.dual_cis_buffer_offset = (n9_ble_dl_if.source->streamBuffer.ShareBufferInfo.length - n9_ble_dl_if.source->param.n9ble.dual_cis_buffer_offset)%n9_ble_dl_if.source->streamBuffer.ShareBufferInfo.length;
            DSP_MW_LOG_I("[BLE] Sub CIS swap offset : %d %d", 2, n9_ble_dl_if.source->param.n9ble.dual_cis_buffer_offset,n9_ble_dl_if.source->streamBuffer.ShareBufferInfo.read_offset);
            N9Ble_update_readoffset_share_information(n9_ble_dl_if.source, n9_ble_dl_if.source->streamBuffer.ShareBufferInfo.read_offset);
        }
        n9_ble_dl_if.source->param.n9ble.dual_cis_status = DUAL_CIS_BOTH_ENABLED;
        return;
    }

    /* Init TimeStamp & _reserved_byte_0Dh */
    DSP_MW_LOG_I("[BLE] Init TimeStamp & _reserved_byte_0Dh current clk: %d", 1,curr_time.period);
    for (int i = 0; i < N9BLE_setting.N9Ble_source.Buffer_Frame_Num; i++) {
        read_ptr = (U8 *)((U8 *)n9_ble_dl_if.source->streamBuffer.ShareBufferInfo.start_addr + i * ALIGN_4(n9_ble_dl_if.source->param.n9ble.frame_length + BLE_AVM_FRAME_HEADER_SIZE));
        buf_header = (LE_AUDIO_HEADER *)read_ptr;
        buf_header->TimeStamp = BLE_AVM_INVALID_TIMESTAMP;
        buf_header->_reserved_byte_0Dh = 0x00;
    }

    n9_ble_dl_if.source->streamBuffer.ShareBufferInfo.read_offset = (play_info->dl_packet_counter%N9BLE_setting.N9Ble_source.Buffer_Frame_Num)*(U32)(ALIGN_4(n9_ble_dl_if.source->param.n9ble.frame_length + BLE_AVM_FRAME_HEADER_SIZE));
    DSP_MW_LOG_I("[BLE] init readoffset : %d %d %d ",3,n9_ble_dl_if.source->streamBuffer.ShareBufferInfo.read_offset,play_info->dl_packet_counter,(play_info->dl_packet_counter%N9BLE_setting.N9Ble_source.Buffer_Frame_Num));
    while (LC_BtClock_IsBTTimeExpired(&play_en_time, &expect_time) == true) {
        if (BLE_CURR_TIME_CHECK < LC_Get_period_Offset_FromAB(&play_en_time, &expect_time)){
            DSP_MW_LOG_E("[BLE] DL clock abnormal %d %d", 2, play_en_time.period,expect_time.period);
            return;
        }
        LC_Add_us_FromA((play_info->iso_interval * BLE_TIME_BT_CLOCK) >> 1, &play_en_time, &play_en_time);
        n9_ble_dl_if.source->param.n9ble.predict_timestamp += play_info->iso_interval;
        N9Ble_SourceUpdateLocalReadOffset(n9_ble_dl_if.source, n9_ble_dl_if.source->param.n9ble.frame_per_iso);
    }
    N9Ble_update_readoffset_share_information(n9_ble_dl_if.source, n9_ble_dl_if.source->streamBuffer.ShareBufferInfo.read_offset);
    DSP_MW_LOG_I("[BLE] DL start anchor timestamp : %d readoffset : %d", 2,n9_ble_dl_if.source->param.n9ble.predict_timestamp,n9_ble_dl_if.source->streamBuffer.ShareBufferInfo.read_offset);

    /* turn on play_en */
    #ifdef AIR_DCHS_MODE_ENABLE
    if(dchs_get_device_mode() == DCHS_MODE_SINGLE){
        MCE_TransBT2NativeClk(play_en_time.period, play_en_time.phase, &native_bt_clk, &native_intra_clk, BT_CLK_Offset);
        hal_audio_afe_set_play_en(native_bt_clk, native_intra_clk);
    }else{
        U32 cur_native_clk = rBb->rClkCtl.rNativeClock & 0x0FFFFFFC;
        if(n9_ble_dl_if.source->param.n9ble.context_type == BLE_CONTEXT_CONVERSATIONAL || n9_ble_dl_if.source->param.n9ble.context_type == BLE_CONTEXT_MEDIA){
            MCE_TransBT2NativeClk(play_en_time.period, play_en_time.phase, &native_bt_clk, &native_intra_clk, BT_CLK_Offset);
            mixer_stream_setup_play_en(native_bt_clk, native_intra_clk, n9_ble_dl_if.source, NULL);
            DSP_MW_LOG_I("[DCHS DL][BLE] le audio Play en, play native clk:0x%x(%d),play phase:0x%x(%d),cur native clk:0x%x(%d)",6 ,native_bt_clk, native_bt_clk, native_intra_clk, native_intra_clk, cur_native_clk, cur_native_clk);
            DCHS_TransBT2NativeClk(native_bt_clk, native_intra_clk, &native_bt_clk, &native_intra_clk, DCHS_CLK_Offset);
            dchs_dl_uart_relay_play_en_info(native_bt_clk, native_intra_clk);
            DSP_MW_LOG_I("[DCHS DL][BLE] le audio relay Play en, play native clk:0x%x(%d),play phase:0x%x(%d),cur native clk:0x%x(%d)",6 ,native_bt_clk, native_bt_clk, native_intra_clk, native_intra_clk, cur_native_clk, cur_native_clk);
        }else{
            MCE_TransBT2NativeClk(play_en_time.period, play_en_time.phase, &native_bt_clk, &native_intra_clk, BT_CLK_Offset);
            dchs_dl_uart_relay_play_en_info(native_bt_clk, native_intra_clk);
            DSP_MW_LOG_I("[DCHS DL][BLE] ULL Play en, play native clk:0x%x(%d),play phase:0x%x(%d),cur native clk:0x%x(%d)",6 ,native_bt_clk, native_bt_clk, native_intra_clk, native_intra_clk, cur_native_clk, cur_native_clk);
            mixer_stream_setup_play_en(native_bt_clk, native_intra_clk, n9_ble_dl_if.source, NULL);
        }
    }
    #else //AIR_DCHS_MODE_ENABLE
    MCE_TransBT2NativeClk(play_en_time.period, play_en_time.phase, &native_bt_clk, &native_intra_clk, BT_CLK_Offset);
    #if AIR_MIXER_STREAM_ENABLE
    mixer_stream_setup_play_en(native_bt_clk, native_intra_clk, n9_ble_dl_if.source, NULL);
    #else
    hal_audio_afe_set_play_en(native_bt_clk, native_intra_clk);
    #endif
    #endif
    AFE_SET_REG(AFE_MEMIF_PBUF_SIZE, 0, 3 << 0); //reduce pbuffer size
    AFE_SET_REG(AFE_MEMIF_MINLEN, 1, 0XF);//reduce pbuffer size

    DSP_MW_LOG_I("[BLE] ble_trigger_dl_stream() native_bt_clk %d, native_intra_clk %d", 2, native_bt_clk, native_intra_clk);

    DSP_MW_LOG_I("[BLE] DL first_anchor_time %d, %d, curr_time %d, %d, expect_time %d, %d, play_en_time %d, %d", 8,
                 play_info->dl_timestamp_clk, play_info->dl_timestamp_phase,
                 curr_time.period, curr_time.phase,
                 expect_time.period, expect_time.phase,
                 play_en_time.period, play_en_time.phase);

    /* start timer for wakeup later stream process */
    during_time = LC_Get_Offset_FromAB(&curr_time, &play_en_time);
    DSP_MW_LOG_I("[BLE] ble_trigger_dl_stream(), during_time %d", 1, during_time);
    if (n9_ble_dl_if.source->param.n9ble.dual_cis_status == DUAL_CIS_WAITING_MAIN)
    {
        n9_ble_dl_if.source->param.n9ble.dual_cis_status = DUAL_CIS_WAITING_SUB;
    }
    if (ble_playen_check_timer == NULL) {
        ble_playen_check_timer = xTimerCreate("BLE PLANEN_CHECK_TIMER", pdMS_TO_TICKS(10), pdTRUE, 0, ble_playen_check_timer_callback);
    }
    if (!ble_playen_check_timer) {
        DSP_MW_LOG_I("playen_check_timer create timer FAIL", 0);
    } else {
        DSP_MW_LOG_I("playen_check_timer create timer PASS", 0);
        xTimerChangePeriod(ble_playen_check_timer, pdMS_TO_TICKS(10), 0);
    }
    if (during_time > 6000)
    {
        during_time -= 6000;
    }
    hal_gpt_sw_get_timer(&g_dl_sw_timer_handle);
    hal_gpt_sw_start_timer_us(g_dl_sw_timer_handle, during_time, ble_init_dl_timer_callback, NULL);
}

void CB_N9_BLE_INIT_PLAY_INFO(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    ble_init_play_info_t *play_info;

    UNUSED(ack);

    if (n9_ble_dl_if.source != NULL)
    {
    DSP_MW_LOG_I("[BLE] CB_N9_BLE_INIT_PLAY_INFO called cis status : %d", 1,n9_ble_dl_if.source->param.n9ble.dual_cis_status);
    }
    /* As the play info don't go through the host check flow, so the ble flow may has closed when run here */
    /*if (n9_ble_dl_if.source == NULL) {
        DSP_MW_LOG_W("[BLE] already close ble flow", 0);
        return;
    }*/

    play_info = (ble_init_play_info_t *)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);

    DSP_MW_LOG_I("[BLE] play_info->iso_interval %d", 1, play_info->iso_interval);
    if (n9_ble_dl_if.source == NULL || !play_info->dl_ft) {
        DSP_MW_LOG_W("[BLE] DL already close ble flow", 0);
    } else {
        if ((n9_ble_dl_if.source->param.n9ble.IsPlayInfoReceived == FALSE) || (n9_ble_dl_if.source->param.n9ble.IsSubPlayInfoReceived == FALSE))
        {
            play_info->dl_avm_info_addr = hal_memview_cm4_to_dsp0(play_info->dl_avm_info_addr);
            DSP_MW_LOG_I("[BLE] play_info->max_pdu_size %d", 1, ((n9_dsp_share_info_ptr)(play_info->dl_avm_info_addr))->anchor_clk);
            if(!((n9_dsp_share_info_ptr)(play_info->dl_avm_info_addr))->anchor_clk)
            {
                DSP_MW_LOG_W("[BLE] DL max_pdu_size %d skip play_info", 1, ((n9_dsp_share_info_ptr)(play_info->dl_avm_info_addr))->anchor_clk);
            }
            else
            {
                DSP_MW_LOG_I("[BLE] play_info->dl_retransmission_window_clk %d", 1, play_info->dl_retransmission_window_clk);
                DSP_MW_LOG_I("[BLE] play_info->dl_retransmission_window_phase %d", 1, play_info->dl_retransmission_window_phase);
                DSP_MW_LOG_I("[BLE] play_info->dl_timestamp_clk %d", 1, play_info->dl_timestamp_clk);
                DSP_MW_LOG_I("[BLE] play_info->dl_timestamp_phase %d", 1, play_info->dl_timestamp_phase);
                DSP_MW_LOG_I("[BLE] play_info->dl_packet_counter %d", 1, play_info->dl_packet_counter);
                DSP_MW_LOG_I("[BLE] play_info->dl_avm_info_addr %x", 1, play_info->dl_avm_info_addr);
                n9_ble_dl_if.source->param.n9ble.iso_interval = (play_info->iso_interval >> 1) * 625;
                n9_ble_dl_if.source->param.n9ble.ret_window_len = ((play_info->iso_interval + 1) >> 1) * 625;
                n9_ble_dl_if.source->param.n9ble.ft = play_info->dl_ft;
                n9_ble_dl_if.source->param.n9ble.seq_miss_cnt = 0;
                if (n9_ble_dl_if.source->param.n9ble.dual_cis_status != DUAL_CIS_WAITING_SUB)
                {
                    n9_ble_dl_if.source->param.n9ble.predict_frame_counter = 0;
                    n9_ble_dl_if.source->param.n9ble.predict_timestamp = play_info->dl_timestamp_clk + (play_info->dl_ft*play_info->iso_interval);
                    n9_ble_dl_if.source->param.n9ble.frame_per_iso = n9_ble_dl_if.source->param.n9ble.iso_interval / n9_ble_dl_if.source->param.n9ble.frame_interval;
                    DSP_MW_LOG_I("[le audio DSP] frame per iso %d",1,n9_ble_dl_if.source->param.n9ble.frame_per_iso) ;
                    n9_ble_dl_if.source->param.n9ble.IsPlayInfoReceived = TRUE; // It is better to add avm buffer compare
                }
                else
                {
                    n9_ble_dl_if.source->param.n9ble.IsSubPlayInfoReceived = TRUE; // It is better to add avm buffer compare
                }
                //if(n9_ble_dl_if.source->param.n9ble.context_type != BLE_CONTENT_TYPE_ULL_BLE){
                    n9_ble_dl_if.source->param.n9ble.frame_length = ((n9_dsp_share_info_ptr)(play_info->dl_avm_info_addr))->anchor_clk;
                //}
                SourceInitBleAvm(n9_ble_dl_if.source,n9_ble_ul_if.sink);
                ble_trigger_dl_stream(play_info);
            }
        }
    }

    if (n9_ble_ul_if.sink == NULL) {
        DSP_MW_LOG_W("[BLE] UL already close ble flow", 0);
    } else if (((play_info->ul_timestamp | play_info->ul_ft | play_info->ul_packet_counter) && (n9_ble_ul_if.sink != NULL))&&(n9_ble_ul_if.sink->param.n9ble.IsPlayInfoReceived == FALSE))  {
        DSP_MW_LOG_I("[BLE] play_info->ul_timestamp %d", 1, play_info->ul_timestamp);
        DSP_MW_LOG_I("[BLE] play_info->ul_packet_counter %d", 1, play_info->ul_packet_counter);
        DSP_MW_LOG_I("[BLE] play_info->ul_avm_info_addr %x", 1, play_info->ul_avm_info_addr);
        play_info->ul_avm_info_addr = hal_memview_cm4_to_dsp0(play_info->ul_avm_info_addr);
        n9_ble_ul_if.sink->param.n9ble.frame_length = ((n9_dsp_share_info_ptr)(play_info->ul_avm_info_addr))->anchor_clk;
        DSP_MW_LOG_I("[BLE] play_info->ul_max_pdu_size %d", 1, n9_ble_ul_if.sink->param.n9ble.frame_length);
        if(!n9_ble_ul_if.sink->param.n9ble.frame_length)
        {
            DSP_MW_LOG_W("[BLE] UL max_pdu_size %d skip play_info", 1, ((n9_dsp_share_info_ptr)(play_info->dl_avm_info_addr))->anchor_clk);
        }
        else
        {
            SourceInitBleAvm(n9_ble_dl_if.source,n9_ble_ul_if.sink);
            ble_trigger_ul_stream(play_info);
            n9_ble_ul_if.sink->param.n9ble.IsPlayInfoReceived = TRUE;
        }
    } else {
    #if 1
        DSP_MW_LOG_W("[BLE] UL already open ble flow", 0);
        DSP_MW_LOG_W("[BLE] play_info->ul_timestamp %d", 1, play_info->ul_timestamp);
        DSP_MW_LOG_W("[BLE] play_info->ul_timestamp %d", 1, play_info->ul_timestamp);
        DSP_MW_LOG_W("[BLE] play_info->ul_packet_counter %d", 1, play_info->ul_packet_counter);
        DSP_MW_LOG_W("[BLE] play_info->ul_avm_info_addr %x", 1, play_info->ul_avm_info_addr);
        DSP_MW_LOG_W("[BLE] play_info->ul_max_pdu_size %d", 1, n9_ble_ul_if.sink->param.n9ble.frame_length);
    #endif
    }

}

/* End BLE CCNI callback function */
#endif //AIR_BT_CODEC_BLE_ENABLED

/* CM4 Playback CCNI callback function */
void CB_CM4_PLAYBACK_OPEN(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("[CM4_PB] Open", 0);


    /* remap to non-cacheable address */
    mcu2dsp_open_param_p open_param;
    open_param = (mcu2dsp_open_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    DSP_REMAP_SHARE_INFO(open_param->stream_in_param.playback.p_share_info, n9_dsp_share_info_ptr);
    memset(&playback_if, 0 ,sizeof(CONNECTION_IF) );
    playback_if.source   = dsp_open_stream_in(open_param);
    playback_if.sink     = dsp_open_stream_out(open_param);
    DSP_Callback_PreloaderConfig(playback_if.pfeature_table, open_param->audio_scenario_type);
}


void CB_CM4_PLAYBACK_START(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("[CM4_PB] Start", 0);
    mcu2dsp_start_param_p start_param;
    start_param = (mcu2dsp_start_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);

    dsp_start_stream_in(start_param, playback_if.source);
    dsp_start_stream_out(start_param, playback_if.sink);

    playback_if.transform = TrasformAudio2Audio(playback_if.source, playback_if.sink, playback_if.pfeature_table);
    if (playback_if.transform == NULL) {
        DSP_MW_LOG_E("CM4 Playback transform failed", 0);
    }
}


void CB_CM4_PLAYBACK_STOP(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("[CM4_PB] Stop", 0);
    if (playback_if.transform != NULL) {
        StreamDSPClose(playback_if.transform->source, playback_if.transform->sink, msg.ccni_message[0] >> 16 | 0x8000);
    }
    playback_if.transform = NULL;
}

void CB_CM4_PLAYBACK_CLOSE(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("[CM4_PB] Close", 0);
    DSP_Callback_UnloaderConfig(playback_if.pfeature_table, playback_if.source->scenario_type);
    SourceClose(playback_if.source);
    SinkClose(playback_if.sink);
    memset(&playback_if,0,sizeof(CONNECTION_IF));
}

void CB_CM4_PLAYBACK_SUSPEND(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("[CM4_PB]  SUSPEND\r\n", 0);

    dsp_trigger_suspend(playback_if.source, playback_if.sink);
}


void CB_CM4_PLAYBACK_RESUME(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("[CM4_PB]  RESUME\r\n", 0);

    dsp_trigger_resume(playback_if.source, playback_if.sink);

}

/* CM4 Line-in Playback CCNI callback function */
void CB_CM4_LINEIN_PLAYBACK_OPEN(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("LINEIN PLAYBACK OPEN\r\n", 0);


    /* remap to non-cacheable address */
    mcu2dsp_open_param_p open_param;
    open_param = (mcu2dsp_open_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    //DSP_REMAP_SHARE_INFO(open_param->stream_in_param.playback.share_info_base_addr, uint32_t);
    memset(&linein_playback_if, 0, sizeof(CONNECTION_IF));
    linein_playback_if.pfeature_table = (stream_feature_list_ptr_t)&stream_feature_list_linein;
    linein_playback_if.source   = dsp_open_stream_in(open_param);
    linein_playback_if.sink     = dsp_open_stream_out(open_param);

    linein_playback_if.source->param.audio.linein_scenario_flag = 1;
    linein_playback_if.sink->param.audio.linein_scenario_flag = 1;

#ifdef AIR_AUDIO_HARDWARE_ENABLE
    Source_Audio_BufferInfo_Rst(linein_playback_if.source, 0);
    Sink_Audio_BufferInfo_Rst(linein_playback_if.sink, 0);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */

    DSP_Callback_PreloaderConfig(linein_playback_if.pfeature_table, open_param->audio_scenario_type);
    DSP_MW_LOG_I("LINEIN PLAYBACK OPEN Finish\r\n", 0);
}

void CB_CM4_LINEIN_PLAYBACK_START(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("LINEIN PLAYBACK START\r\n", 0);
    mcu2dsp_start_param_p start_param;
    start_param = (mcu2dsp_start_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);

    dsp_start_stream_in(start_param, linein_playback_if.source);
    hal_gpt_delay_us(2000);
    dsp_start_stream_out(start_param, linein_playback_if.sink);

#ifndef AIR_BT_CODEC_BLE_ENABLED
    stream_feature_configure_resolution((stream_feature_list_ptr_t)linein_playback_if.pfeature_table, RESOLUTION_32BIT, CONFIG_DECODER);
#endif
    linein_playback_if.transform = TrasformAudio2Audio(linein_playback_if.source, linein_playback_if.sink, linein_playback_if.pfeature_table);
    if (linein_playback_if.transform == NULL) {
        DSP_MW_LOG_E("CM4 Line-in Playback transform failed", 0);
    }
}

void CB_CM4_LINEIN_PLAYBACK_STOP(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);

    DSP_MW_LOG_I("LINEIN PLAYBACK STOP\r\n", 0);
    if (linein_playback_if.transform != NULL) {
        StreamDSPClose(linein_playback_if.transform->source, linein_playback_if.transform->sink, msg.ccni_message[0] >> 16 | 0x8000);
    }
    linein_playback_if.transform = NULL;
    DSP_MW_LOG_I("LINEIN PLAYBACK STOP Finish\r\n", 0);
}

void CB_CM4_LINEIN_PLAYBACK_CLOSE(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("LINEIN PLAYBACK CLOSE\r\n", 0);

    //to cancel Sink_Audio_FlushBuffer mute flag for issue BTA-9374 Line-in switch to BT no sound
    hal_audio_volume_digital_gain_parameter_t           digital_gain;
    memset(&digital_gain, 0, sizeof(hal_audio_volume_digital_gain_parameter_t));
    digital_gain.memory_select = (hal_audio_memory_selection_t)linein_playback_if.sink->param.audio.memory;
    digital_gain.mute_control = HAL_AUDIO_VOLUME_MUTE_FRAMEWORK;
    digital_gain.mute_enable = false;
    digital_gain.is_mute_control = true;
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    hal_audio_set_value((hal_audio_set_value_parameter_t *)&digital_gain, HAL_AUDIO_SET_VOLUME_HW_DIGITAL_GAIN);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
    DSP_Callback_UnloaderConfig(linein_playback_if.pfeature_table, linein_playback_if.source->scenario_type);
    SourceClose(linein_playback_if.source);
    SinkClose(linein_playback_if.sink);
    //DSP_MW_LOG_I("feature PIC unload : %d",1,*(linein_playback_if.pfeature_table));
    memset(&linein_playback_if, 0, sizeof(CONNECTION_IF));

    DSP_MW_LOG_I("LINEIN PLAYBACK CLOSE Finish\r\n", 0);
}

void CB_CM4_LINEIN_PLAYBACK_SUSPEND(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("LINEIN PLAYBACK SUSPEND\r\n", 0);

    dsp_trigger_suspend(linein_playback_if.source, linein_playback_if.sink);
}

void CB_CM4_LINEIN_PLAYBACK_RESUME(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("LINEIN PLAYBACK RESUME\r\n", 0);

    dsp_trigger_resume(linein_playback_if.source, linein_playback_if.sink);
}

afe_loopback_param_t dsp_afe_loopack;
//extern hal_audio_device_parameter_vow_t vow_control;

void CB_CM4_TRULY_LINEIN_PLAYBACK_OPEN(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    DSP_MW_LOG_I("Cm4 Truly Line-in Playback Open", 0);
    mcu2dsp_open_param_p open_param;
    UNUSED(ack);

#ifdef AB1568_BRING_UP_DSP_DEFAULT_HW_LOOPBACK
    dsp_afe_loopack.in_device = (hal_audio_device_t)HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_DUAL;//HAL_AUDIO_CONTROL_DEVICE_LINE_IN_DUAL;
    dsp_afe_loopack.in_interface = HAL_AUDIO_INTERFACE_1;
    //dsp_afe_loopack.in_misc_parms = 259;
    dsp_afe_loopack.out_device = (hal_audio_device_t)HAL_AUDIO_CONTROL_DEVICE_INTERNAL_DAC_DUAL;//HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER;
    dsp_afe_loopack.out_interface = HAL_AUDIO_INTERFACE_1;
    //dsp_afe_loopack.out_misc_parms = 1;
    dsp_afe_loopack.sample_rate = 48000;
    dsp_afe_loopack.with_hw_gain = true;
    dsp_afe_loopack.stream_channel = HAL_AUDIO_DIRECT;
    dsp_afe_loopack.format = HAL_AUDIO_PCM_FORMAT_S32_LE;
#else
    open_param = (mcu2dsp_open_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);

    dsp_afe_loopack.in_device = (hal_audio_device_t)open_param->stream_in_param.afe.audio_device;
    dsp_afe_loopack.in_interface = (hal_audio_interface_t)open_param->stream_in_param.afe.audio_interface;
    dsp_afe_loopack.in_misc_parms.I2sClkSourceType = open_param->stream_in_param.afe.misc_parms;
    dsp_afe_loopack.out_device = (hal_audio_device_t)open_param->stream_out_param.afe.audio_device;
    dsp_afe_loopack.out_interface = (hal_audio_interface_t)open_param->stream_out_param.afe.audio_interface;
    dsp_afe_loopack.out_misc_parms.I2sClkSourceType = open_param->stream_out_param.afe.misc_parms;
    dsp_afe_loopack.sample_rate = open_param->stream_out_param.afe.sampling_rate;
    dsp_afe_loopack.with_hw_gain = open_param->stream_out_param.afe.hw_gain;
    dsp_afe_loopack.stream_channel = open_param->stream_out_param.afe.stream_channel;
    dsp_afe_loopack.format = open_param->stream_out_param.afe.format;

#endif
#ifdef LINE_IN_PURE_FOR_AMIC_CLASS_G_HQA
    DSP_MW_LOG_I("dsp_afe_loopack.in_device %d, adc_mode %d, adc_type %d\r\n", 3, dsp_afe_loopack.in_device, open_param->stream_in_param.afe.adc_mode, open_param->stream_in_param.afe.adc_type);
    if (dsp_afe_loopack.in_device & (hal_audio_device_t)HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_DUAL) {
        dsp_afe_loopack.device_handle_in.analog_mic.adc_parameter.performance = open_param->stream_in_param.afe.performance;
        dsp_afe_loopack.device_handle_in.analog_mic.bias_voltage[0] = open_param->stream_in_param.afe.bias_voltage[0];
        dsp_afe_loopack.device_handle_in.analog_mic.bias_voltage[1] = open_param->stream_in_param.afe.bias_voltage[1];
        dsp_afe_loopack.device_handle_in.analog_mic.bias_voltage[2] = open_param->stream_in_param.afe.bias_voltage[2];
        dsp_afe_loopack.device_handle_in.analog_mic.bias_voltage[3] = open_param->stream_in_param.afe.bias_voltage[3];
        dsp_afe_loopack.device_handle_in.analog_mic.bias_voltage[4] = open_param->stream_in_param.afe.bias_voltage[4];
        dsp_afe_loopack.device_handle_in.analog_mic.bias_select = open_param->stream_in_param.afe.bias_select;
        dsp_afe_loopack.device_handle_in.analog_mic.iir_filter = open_param->stream_in_param.afe.iir_filter[0];
        dsp_afe_loopack.device_handle_in.analog_mic.adc_parameter.adc_mode = open_param->stream_in_param.afe.adc_mode;
        dsp_afe_loopack.device_handle_in.analog_mic.adc_parameter.adc_type = open_param->stream_in_param.afe.adc_type;
        dsp_afe_loopack.device_handle_in.analog_mic.mic_interface = (hal_audio_interface_t)open_param->stream_in_param.afe.audio_interface;
    } else if (dsp_afe_loopack.in_device & (hal_audio_device_t)HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_DUAL) {
        dsp_afe_loopack.device_handle_in.digital_mic.dmic_clock_rate = open_param->stream_in_param.afe.performance;
        dsp_afe_loopack.device_handle_in.digital_mic.bias_voltage[0] = open_param->stream_in_param.afe.bias_voltage[0];
        dsp_afe_loopack.device_handle_in.digital_mic.bias_voltage[1] = open_param->stream_in_param.afe.bias_voltage[1];
        dsp_afe_loopack.device_handle_in.digital_mic.bias_voltage[2] = open_param->stream_in_param.afe.bias_voltage[2];
        dsp_afe_loopack.device_handle_in.digital_mic.bias_voltage[3] = open_param->stream_in_param.afe.bias_voltage[3];
        dsp_afe_loopack.device_handle_in.digital_mic.bias_voltage[4] = open_param->stream_in_param.afe.bias_voltage[4];
        dsp_afe_loopack.device_handle_in.digital_mic.bias_select = open_param->stream_in_param.afe.bias_select;
        dsp_afe_loopack.device_handle_in.digital_mic.iir_filter = open_param->stream_in_param.afe.iir_filter[0];
        dsp_afe_loopack.device_handle_in.digital_mic.mic_interface = (hal_audio_interface_t)open_param->stream_in_param.afe.audio_interface;
        dsp_afe_loopack.device_handle_in.digital_mic.dmic_selection = open_param->stream_in_param.afe.dmic_selection[0];
        DSP_MW_LOG_I("dsp_afe_loopack dmic_selection %d, clk %d", 2, dsp_afe_loopack.device_handle_in.digital_mic.dmic_selection,
                     open_param->stream_in_param.afe.adc_mode);
    } else if (dsp_afe_loopack.in_device & (hal_audio_device_t)HAL_AUDIO_CONTROL_DEVICE_LINE_IN_DUAL) {
        dsp_afe_loopack.device_handle_in.linein.adc_parameter.performance = open_param->stream_in_param.afe.performance;
        dsp_afe_loopack.device_handle_in.linein.bias_voltage[0] = open_param->stream_in_param.afe.bias_voltage[0];
        dsp_afe_loopack.device_handle_in.linein.bias_voltage[1] = open_param->stream_in_param.afe.bias_voltage[1];
        dsp_afe_loopack.device_handle_in.linein.bias_voltage[2] = open_param->stream_in_param.afe.bias_voltage[2];
        dsp_afe_loopack.device_handle_in.linein.bias_voltage[3] = open_param->stream_in_param.afe.bias_voltage[3];
        dsp_afe_loopack.device_handle_in.linein.bias_voltage[4] = open_param->stream_in_param.afe.bias_voltage[4];
        dsp_afe_loopack.device_handle_in.linein.bias_select = open_param->stream_in_param.afe.bias_select;
        dsp_afe_loopack.device_handle_in.linein.iir_filter = open_param->stream_in_param.afe.iir_filter[0];
        dsp_afe_loopack.device_handle_in.linein.adc_parameter.adc_mode = open_param->stream_in_param.afe.adc_mode;
        //DSP_MW_LOG_I("dsp_afe_loopack linein performance %d, bias_voltagea %d %d %d %d %d, bias_select %d, iir_filter %d, adc_mode %d",9,dsp_afe_loopack.device_handle_in.linein.adc_parameter.performance,
        //    dsp_afe_loopack.device_handle_in.linein.bias_voltage[0],dsp_afe_loopack.device_handle_in.linein.bias_voltage[1],dsp_afe_loopack.device_handle_in.linein.bias_voltage[2],dsp_afe_loopack.device_handle_in.linein.bias_voltage[3],
        //    dsp_afe_loopack.device_handle_in.linein.bias_voltage[4],dsp_afe_loopack.device_handle_in.linein.bias_select,dsp_afe_loopack.device_handle_in.linein.iir_filter,dsp_afe_loopack.device_handle_in.linein.adc_parameter.adc_mode);
    } else if ((dsp_afe_loopack.in_device == (hal_audio_device_t)HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER) || (dsp_afe_loopack.in_device == (hal_audio_device_t)HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_L) || (dsp_afe_loopack.in_device == (hal_audio_device_t)HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER_R)) {
        dsp_afe_loopack.device_handle_in.i2s_master.i2s_interface = (hal_audio_interface_t)open_param->stream_in_param.afe.audio_interface;
    }
    if(dsp_afe_loopack.out_device == (hal_audio_device_t)HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE){
        dsp_afe_loopack.device_handle_out.i2s_slave.i2s_interface = (hal_audio_interface_t)open_param->stream_out_param.afe.audio_interface;
    }
#if defined(AIR_BTA_IC_PREMIUM_G2)
    dsp_afe_loopack.device_handle_out.dac.with_high_performance = open_param->stream_out_param.afe.performance;
#else
    dsp_afe_loopack.device_handle_out.dac.performance = open_param->stream_out_param.afe.performance;
#endif
    DSP_MW_LOG_I("dsp_afe_loopack in_device %d in_interface %d in_misc_parms %d out_device %d out_interface %d out_misc_parms %d sample_rate %d with_hw_gain %d stream_channel %d format %d adc_mode %d adc_type %d", 12,
                 dsp_afe_loopack.in_device, dsp_afe_loopack.in_interface, dsp_afe_loopack.in_misc_parms, dsp_afe_loopack.out_device, dsp_afe_loopack.out_interface,
                 dsp_afe_loopack.out_misc_parms, dsp_afe_loopack.sample_rate, dsp_afe_loopack.with_hw_gain, dsp_afe_loopack.stream_channel, dsp_afe_loopack.format,
                 dsp_afe_loopack.device_handle_in.analog_mic.adc_parameter.adc_mode, dsp_afe_loopack.device_handle_in.analog_mic.adc_parameter.adc_type);
#endif
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    afe_set_loopback_enable(true, &dsp_afe_loopack);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
#if 0

    memset(&dsp_vow_control, 0, sizeof(hal_audio_device_parameter_vow_t));
    dsp_vow_control.audio_device = HAL_AUDIO_CONTROL_DEVICE_VOW;
    dsp_vow_control.dma_irq_threshold = VOW_SRAM_COPY_SIZE;
    dsp_vow_control.snr_threshold = 0x7373;
    dsp_vow_control.alpha_rise = 0x7;
    dsp_vow_control.dmic_selection = HAL_AUDIO_DMIC_GPIO_DMIC0;
    //dsp_vow_control.suspend_mic = true;
    dsp_vow_control.suspend_mic = false;
    dsp_vow_control.input_device = HAL_AUDIO_CONTROL_DEVICE_VOW;
    //dsp_vow_control.mic_selection = HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_DUAL;
    dsp_vow_control.mic_selection = HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_DUAL;
    dsp_vow_control.bias_select = HAL_AUDIO_BIAS_SELECT_ALL;
    dsp_vow_control.bias_voltage = HAL_AUDIO_BIAS_VOLTAGE_1_85V;
    dsp_vow_control.mic_interface = HAL_AUDIO_INTERFACE_1;
    dsp_vow_control.memory_select = HAL_AUDIO_MEMORY_UL_VUL1;
    dsp_vow_control.adc_parameter.performance = AFE_PEROFRMANCE_ULTRA_LOW_POWER_MODE;
    dsp_vow_control.rate = 16000;
    dsp_vow_control.vow_detection_done_entry = dsp_vow_isr_handler;
    //dsp_vow_control.vow_mode = AFE_VOW_PHASE0;
    dsp_vow_control.vow_mode = AFE_VOW_PHASE1;
    dsp_vow_control.vow_with_hpf = true;

    vow_sink(NULL, NULL);
    vow_enable(&dsp_vow_control, NULL);


#endif


}

void CB_CM4_TRULY_LINEIN_PLAYBACK_CLOSE(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    DSP_MW_LOG_I("Cm4 Truly Line-in Playback Close", 0);
    UNUSED(msg);
    UNUSED(ack);
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    afe_set_loopback_enable(false, &dsp_afe_loopack);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
}
/* End CM4 Line-in Playback CCNI callback function */

/* CM4 Record CCNI callback function */
#ifdef MTK_CM4_RECORD_ENABLE
extern bool CM4_Record_air_dump;
extern U8   CM4_Record_air_dump_scenario;
#ifdef MTK_LEAKAGE_DETECTION_ENABLE
extern bool CM4_Record_leakage_enable;
#endif
#ifdef MTK_USER_TRIGGER_FF_ENABLE
extern bool utff_enable;
#endif
#ifdef MTK_WWE_ENABLE
hal_audio_device_t  wwe_audio_device;
#endif
//SWSRC for record
#if defined(AIR_FIXED_RATIO_SRC) && (defined(AIR_UL_FIX_SAMPLING_RATE_32K) || defined(AIR_UL_FIX_SAMPLING_RATE_48K))
#define DSP_RECORD_SMP_PORT_NUM  2
uint8_t record_smp_port_use_cnt = 0;
src_fixed_ratio_port_t *record_smp_port[DSP_RECORD_SMP_PORT_NUM];
#endif //SWSRC for record
void CB_CM4_RECORD_OPEN(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("Cm4 record Open", 0);

    /* remap to non-cacheable address */
    mcu2dsp_open_param_p open_param;
    open_param = (mcu2dsp_open_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    DSP_REMAP_SHARE_INFO(open_param->stream_out_param.record.p_share_info, n9_dsp_share_info_t *);
    open_param->stream_out_param.record.p_share_info->start_addr = hal_memview_cm4_to_dsp0(open_param->stream_out_param.record.p_share_info->start_addr);
    memset(&record_if, 0, sizeof(CONNECTION_IF));
    audio_dsp_codec_type_t audio_dsp_codec_type;
    audio_dsp_codec_type = msg.ccni_message[0] & 0x0fff;
#ifdef MTK_WWE_ENABLE
    g_wwe_mode = ((msg.ccni_message[0] & 0xf000) >> 12);
    DSP_MW_LOG_I("Cm4 record Open wwe_mode = %d", 1, g_wwe_mode);
    wwe_audio_device = (hal_audio_device_t)open_param->stream_in_param.afe.audio_device;
#endif
    DSP_MW_LOG_I("Cm4 record Open msg.ccni_message[0]:0x%x codec type:0x%3x", 2, msg.ccni_message[0],audio_dsp_codec_type);
    switch (audio_dsp_codec_type) {
#ifdef MTK_RECORD_OPUS_ENABLE
        case AUDIO_DSP_CODEC_TYPE_OPUS: {
            record_if.pfeature_table = (stream_feature_list_ptr_t)&stream_feature_list_opus_mic_record;
            break;
        }
#endif
#ifdef MTK_LEAKAGE_DETECTION_ENABLE
        case AUDIO_DSP_CODEC_TYPE_ANC_LC: {
            CM4_Record_leakage_enable = true;
            record_if.pfeature_table = (stream_feature_list_ptr_t)&stream_feature_list_leakage_compensation;
            break;
        }
#endif
#ifdef MTK_USER_TRIGGER_FF_ENABLE
        case AUDIO_DSP_CODEC_TYPE_ANC_USER_TRIGGER_FF_SZ: {
            utff_enable = true;
            record_if.pfeature_table = (stream_feature_list_ptr_t)&stream_feature_list_user_trigger_adaptive_ff_sz;
            DSP_MW_LOG_I("[user_trigger_ff]iir_filter[0]:0x%x, iir_filter[1]:0x%x, iir_filter[2]:0x%x", 3, open_param->stream_in_param.afe.iir_filter[0], open_param->stream_in_param.afe.iir_filter[1], open_param->stream_in_param.afe.iir_filter[2]);
            break;
        }
        case AUDIO_DSP_CODEC_TYPE_ANC_USER_TRIGGER_FF_PZ: {
            utff_enable = true;
            record_if.pfeature_table = (stream_feature_list_ptr_t)&stream_feature_list_user_trigger_adaptive_ff_pz;
            DSP_MW_LOG_I("[user_trigger_ff]iir_filter[0]:0x%x, iir_filter[1]:0x%x, iir_filter[2]:0x%x", 3, open_param->stream_in_param.afe.iir_filter[0], open_param->stream_in_param.afe.iir_filter[1], open_param->stream_in_param.afe.iir_filter[2]);
            break;
        }
        case AUDIO_DSP_CODEC_TYPE_ANC_USER_TRIGGER_FF_PZ_FIR: {
            utff_enable = true;
            record_if.pfeature_table = (stream_feature_list_ptr_t)&stream_feature_list_user_trigger_adaptive_ff_pz_fir;
            DSP_MW_LOG_I("[user_trigger_ff]iir_filter[0]:0x%x, iir_filter[1]:0x%x, iir_filter[2]:0x%x", 3, open_param->stream_in_param.afe.iir_filter[0], open_param->stream_in_param.afe.iir_filter[1], open_param->stream_in_param.afe.iir_filter[2]);
            break;
        }
#endif
        case AUDIO_DSP_CODEC_TYPE_PCM: {
            record_if.pfeature_table = (stream_feature_list_ptr_t)&stream_feature_list_mic_record;
            break;
        }
        default:
            if ((audio_dsp_codec_type & 0xff00) == 0xff00) { /*Record air dump.*/
                CM4_Record_air_dump = true;
                CM4_Record_air_dump_scenario = audio_dsp_codec_type & 0x000f;
                record_if.pfeature_table = (stream_feature_list_ptr_t)&stream_feature_list_mic_record_airdump;
            } else {
                record_if.pfeature_table = (stream_feature_list_ptr_t)&stream_feature_list_mic_record;
            }
            break;
    }
#ifdef MTK_WWE_ENABLE
    if (g_wwe_mode != WWE_MODE_NONE) {
        record_if.pfeature_table = (stream_feature_list_ptr_t)&stream_feature_list_wwe_mic_record;
        g_mcu2dsp_vad_param = (mcu2dsp_vad_param_p)open_param->stream_out_param.record.p_share_info->start_addr;
    }
#endif
    record_if.source   = dsp_open_stream_in(open_param);
    record_if.sink     = dsp_open_stream_out(open_param);

    DSP_Callback_PreloaderConfig(record_if.pfeature_table, open_param->audio_scenario_type);
#ifdef MTK_WWE_ENABLE
    wwe_processing_init();
#endif
#if defined(AIR_FIXED_RATIO_SRC) && (defined(AIR_UL_FIX_SAMPLING_RATE_32K) || defined(AIR_UL_FIX_SAMPLING_RATE_48K))
    //SWARC
    if (AUDIO_DSP_CODEC_TYPE_PCM == audio_dsp_codec_type) {
        src_fixed_ratio_config_t smp_config;
        U32 fs;

        memset(&smp_config, 0, sizeof(src_fixed_ratio_config_t));
        fs = open_param->stream_in_param.afe.sampling_rate;
        switch (fs) {
            case 16000:
            case 32000:
            case 48000: {
                smp_config.channel_number = 0;
                smp_config.in_sampling_rate = open_param->stream_in_param.afe.sampling_rate;
                smp_config.out_sampling_rate = 16000;
                smp_config.resolution = RESOLUTION_16BIT;
                smp_config.multi_cvt_mode = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_SINGLE;
                smp_config.cvt_num = 1;

                if (record_smp_port_use_cnt < DSP_RECORD_SMP_PORT_NUM) {
                    record_smp_port[record_smp_port_use_cnt] = stream_function_src_fixed_ratio_get_port(record_if.source);
                    stream_function_src_fixed_ratio_init(record_smp_port[record_smp_port_use_cnt], &smp_config);
                    record_smp_port_use_cnt++;
                } else {
                    DSP_MW_LOG_E("[CB_CM4_RECORD_OPEN] missing record smp port, record_smp_port_use_cnt: %u, max_port_num: %u", 2, record_smp_port_use_cnt, DSP_RECORD_SMP_PORT_NUM);
                    AUDIO_ASSERT(FALSE);
                }
                DSP_MW_LOG_I("[CB_CM4_RECORD_OPEN]smp port0:  in_sampling_rate: %u, out_sampling_rate: %u", 2, smp_config.in_sampling_rate, smp_config.out_sampling_rate);
                break;
            }
            case 96000: {
                smp_config.channel_number = 0;
                smp_config.in_sampling_rate = open_param->stream_in_param.afe.sampling_rate;
                smp_config.out_sampling_rate = 48000;
                smp_config.resolution = RESOLUTION_16BIT;
                smp_config.multi_cvt_mode = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_CONSECUTIVE;
                smp_config.cvt_num = 2;

                if (record_smp_port_use_cnt < DSP_RECORD_SMP_PORT_NUM) {
                    record_smp_port[record_smp_port_use_cnt] = stream_function_src_fixed_ratio_get_port(record_if.source);
                    stream_function_src_fixed_ratio_init(record_smp_port[record_smp_port_use_cnt], &smp_config);
                    record_smp_port_use_cnt++;
                } else {
                    DSP_MW_LOG_E("[CB_CM4_RECORD_OPEN] missing record smp port, record_smp_port_use_cnt: %u, max_port_num: %u", 2, record_smp_port_use_cnt, DSP_RECORD_SMP_PORT_NUM);
                    AUDIO_ASSERT(FALSE);
                }
                DSP_MW_LOG_I("[CB_CM4_RECORD_OPEN]smp port0:  in_sampling_rate: %u, out_sampling_rate: %u", 2, smp_config.in_sampling_rate, smp_config.out_sampling_rate);

                smp_config.channel_number = 0;
                smp_config.in_sampling_rate = 48000;
                smp_config.out_sampling_rate = 16000;
                smp_config.resolution = RESOLUTION_16BIT;
                smp_config.multi_cvt_mode = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_CONSECUTIVE;
                smp_config.cvt_num = 2;

                if (record_smp_port_use_cnt < DSP_RECORD_SMP_PORT_NUM) {
                    record_smp_port[record_smp_port_use_cnt] = stream_function_src_fixed_ratio_get_2nd_port(record_if.source);
                    stream_function_src_fixed_ratio_init(record_smp_port[record_smp_port_use_cnt], &smp_config);
                    record_smp_port_use_cnt++;
                } else {
                    DSP_MW_LOG_E("[CB_CM4_RECORD_OPEN] missing record smp port, record_smp_port_use_cnt: %u, max_port_num: %u", 2, record_smp_port_use_cnt, DSP_RECORD_SMP_PORT_NUM);
                    AUDIO_ASSERT(FALSE);
                }
                DSP_MW_LOG_I("[CB_CM4_RECORD_OPEN]smp port1:  in_sampling_rate: %u, out_sampling_rate: %u", 2, smp_config.in_sampling_rate, smp_config.out_sampling_rate);
                break;
            }
            default: {
                DSP_MW_LOG_E("[CB_CM4_RECORD_OPEN] Not support sample rate, sample_rate: %u", 1, fs);
                break;
            }
        }
    }
#endif
}


void CB_CM4_RECORD_START(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("Cm4 record Start", 0);
    mcu2dsp_start_param_p start_param;
    start_param = (mcu2dsp_start_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);

    dsp_start_stream_in(start_param, record_if.source);
    dsp_start_stream_out(start_param, record_if.sink);
#ifdef AIR_UL_FIX_RESOLUTION_32BIT
    stream_feature_configure_resolution((stream_feature_list_ptr_t)record_if.pfeature_table, RESOLUTION_32BIT, 0);
#else
    stream_feature_configure_resolution((stream_feature_list_ptr_t)record_if.pfeature_table, RESOLUTION_16BIT, 0);
#endif
    record_if.source->param.audio.AfeBlkControl.u4awsflag = 1;
    record_if.transform = TrasformAudio2Audio(record_if.source, record_if.sink, record_if.pfeature_table);
    hal_audio_trigger_start_parameter_t sw_trigger_start;
    sw_trigger_start.enable = true;
    sw_trigger_start.memory_select = record_if.source->param.audio.mem_handle.memory_select;
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    hal_audio_set_value((hal_audio_set_value_parameter_t *)&sw_trigger_start, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
#endif
}


void CB_CM4_RECORD_STOP(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("Cm4 record Stop", 0);
#ifdef MTK_WWE_ENABLE
    /*must deinit hwvad first to resume mic ,then stop the record*/
    wwe_hwvad_deinit();
#endif
    if (record_if.transform != NULL) {
        StreamDSPClose(record_if.transform->source, record_if.transform->sink, msg.ccni_message[0] >> 16 | 0x8000);
    } else {
        DSP_MW_LOG_E("Cm4 record not exit, just ack.", 0);
        aud_msg_ack(msg.ccni_message[0] >> 16 | 0x8000, FALSE);
    }
    record_if.transform = NULL;
}

void CB_CM4_RECORD_CLOSE(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("Cm4 record Close", 0);
#ifdef MTK_WWE_ENABLE
    wwe_processing_deinit();
#endif

#if defined(AIR_FIXED_RATIO_SRC) && (defined(AIR_UL_FIX_SAMPLING_RATE_32K) || defined(AIR_UL_FIX_SAMPLING_RATE_48K))
    uint8_t i;
    for (i = 0; i < record_smp_port_use_cnt; i++) {
        if (record_smp_port[i]) {
            stream_function_src_fixed_ratio_deinit(record_smp_port[i]);
            record_smp_port[i] = NULL;
        }
    }
    record_smp_port_use_cnt = 0;
#endif
    DSP_Callback_UnloaderConfig(record_if.pfeature_table, record_if.source->scenario_type);
    SourceClose(record_if.source);
    SinkClose(record_if.sink);
    memset(&record_if,0,sizeof(CONNECTION_IF));
    CM4_Record_air_dump = false;
#ifdef MTK_LEAKAGE_DETECTION_ENABLE
    CM4_Record_leakage_enable = false;
#endif
#ifdef MTK_USER_TRIGGER_FF_ENABLE
    utff_enable = false;
#endif
}

void CB_CM4_RECORD_SUSPEND(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("Cm4 recordSUSPEND\r\n", 0);
    dsp_trigger_suspend(record_if.source, record_if.sink);

}

void CB_CM4_RECORD_RESUME(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("Cm4 record RESUME\r\n", 0);
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
    dsp_detachable_config(&record_if, msg);
#endif
    dsp_trigger_resume(record_if.source, record_if.sink);
}
#endif /* MTK_CM4_RECORD_ENABLE */
/* End Playback CCNI callback function */

#ifdef MTK_LEAKAGE_DETECTION_ENABLE
void CB_CM4_RECORD_LC_SET_PARAM_ACK(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    anc_leakage_compensation_parameters_nvdm_p leakage_compensation_param;
    void *share_ptr = (void *)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    leakage_detection_load_nvkey(share_ptr);

    leakage_compensation_param = (anc_leakage_compensation_parameters_nvdm_p)share_ptr;
    leakage_detection_set_duration(leakage_compensation_param->report_thd, leakage_compensation_param->no_response_thd);
    DSP_MW_LOG_I("[RECORD_LC]CB_CM4_RECORD_LC_SET_PARAM_ACK, ld_thrd:0x%x, RXIN_TXREF_DELAY:0x%x, DIGITAL_GAIN:0x%x, report_thd:%u, no_response_thd:%u\r\n", 5, leakage_compensation_param->LD_para.ld_thrd, leakage_compensation_param->LD_para.RXIN_TXREF_DELAY, leakage_compensation_param->LD_para.DIGITAL_GAIN, leakage_compensation_param->report_thd, leakage_compensation_param->no_response_thd);
}
#endif


#if defined(AIR_DAC_MODE_RUNTIME_CHANGE)
void dsp_dac_enter_deactive_mode(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
//    UNUSED(msg);
    UNUSED(ack);

    uint32_t gain_compensation = msg.ccni_message[0];

    /*pass message to dtm task and call  "hal_audio_device_enter_dac_deactive_mode(true)" */
    DTM_enqueue(DTM_EVENT_ID_DAC_DEACTIVE_MODE_ENTER, gain_compensation, false);
}

void dsp_dac_exit_deactive_mode(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);


    uint32_t dac_info = msg.ccni_message[0] & 0xFF;
    dac_info |= (msg.ccni_message[1] & 0x3F) << 16;


    /*pass message to dtm task and call  "hal_audio_device_enter_dac_deactive_mode(false)" */
    DTM_enqueue(DTM_EVENT_ID_DAC_DEACTIVE_MODE_EXIT, dac_info, false);
}
#endif


#ifdef AIR_SIDETONE_ENABLE
afe_sidetone_param_t dsp_afe_sidetone;
afe_sidetone_param_extension_t dsp_afe_sidetone_extension;
void dsp_sidetone_start(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);

    U32 gpt_timer;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_timer);
    DSP_MW_LOG_I("SideTone Start gpt : %d\r\n", 1, gpt_timer);

    mcu2dsp_sidetone_param_p start_param;
    //mcu2dsp_sidetone_param_t sidetone;


#if 0
    dsp_afe_sidetone.in_device      = HAL_AUDIO_DEVICE_MAIN_MIC_DUAL;
    dsp_afe_sidetone.in_interface   = HAL_AUDIO_INTERFACE_1;
    dsp_afe_sidetone.out_device     = HAL_AUDIO_DEVICE_DAC_DUAL;
    dsp_afe_sidetone.out_interface  = HAL_AUDIO_INTERFACE_1;
    dsp_afe_sidetone.channel        = HAL_AUDIO_DIRECT;
    dsp_afe_sidetone.gain           = 600;
#else
    start_param = (mcu2dsp_sidetone_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    memcpy(&dsp_afe_sidetone, start_param, sizeof(mcu2dsp_sidetone_param_t));
#endif
    memset(&dsp_afe_sidetone_extension, 0, sizeof(dsp_afe_sidetone_extension));
    //afe_set_sidetone_enable(true, dsp_afe_sidetone);

#ifdef AIR_AUDIO_HARDWARE_ENABLE
    afe_set_sidetone_enable_flag(true, dsp_afe_sidetone.gain);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */

    DSP_MW_LOG_I("SideTone Start gain %d\r\n", 1, dsp_afe_sidetone.gain);
    DTM_enqueue(DTM_EVENT_ID_SIDETONE_START, 0, false);
}

void dsp_sidetone_stop(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("SideTone Stop\r\n", 0);
#if 0
    mcu2dsp_sidetone_param_p start_param;
    mcu2dsp_sidetone_param_t sidetone;
    start_param = (mcu2dsp_sidetone_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    memcpy(&sidetone, start_param, sizeof(mcu2dsp_sidetone_param_t));
#endif
    //afe_set_sidetone_enable(false, dsp_afe_sidetone);
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    afe_set_sidetone_enable_flag(false, dsp_afe_sidetone.gain);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
#if 0//modify for ab1568
#ifdef ENABLE_SIDETONE_RAMP_TIMER
    fw_sidetone_set_ramp_timer(FW_SIDETONE_MUTE_GAIN);
#else
    DTM_enqueue(DTM_EVENT_ID_SIDETONE_STOP, 0, false);
#endif
#else
    DTM_enqueue(DTM_EVENT_ID_SIDETONE_STOP_RAMP, 0, false);
    //hal_audio_set_device(&(dsp_afe_sidetone.device_handle_in_side_tone), dsp_afe_sidetone.device_handle_in_side_tone.sidetone.audio_device, HAL_AUDIO_CONTROL_OFF);
    DSP_MW_LOG_I("sidetone device 0x%x off\r\n", 1, dsp_afe_sidetone_extension.device_handle_in_side_tone.sidetone.audio_device);
#endif
}

void dsp_sidetone_set_volume(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("SideTone Set_volume\r\n", 0);
    int32_t sidetone_gain = (int32_t)msg.ccni_message[1];
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    afe_set_sidetone_volume(sidetone_gain);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
}

void dsp_sidetone_start_volume_set(void)
{
    if (afe_get_sidetone_enable_flag() == true) {
        //hal_gpt_delay_ms(200);
        vTaskDelay(pdMS_TO_TICKS(200));
#ifdef AIR_AUDIO_HARDWARE_ENABLE
        afe_set_sidetone_volume(afe_get_sidetone_gain());
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
    }
}
#endif

#define AIR_MONITOR_DUBUG_RG_ENABLE (0)
#if AIR_MONITOR_DUBUG_RG_ENABLE
uint32_t rg_debug_timer_handle;
static void rg_debug_timer_callback(void *user_data)
{
    UNUSED(user_data);
    uint32_t reg_value = AFE_READ(ASM_OUT_BUF_MON0);

    HAL_AUDIO_LOG_INFO("rg_debug_timer_callback, ASM_OUT_BUF_MON0:0x%x", 1, reg_value);

    hal_gpt_sw_start_timer_ms(rg_debug_timer_handle, 500, rg_debug_timer_callback, NULL);
}
#endif

#ifdef AIR_MCU_DSP_DEPENDECY_CHECK_ENABLE
void dsp_a2dp_clock_dependency_check(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("dsp_a2dp_clock_dependency_check\r\n", 0);

    audio_clock_param = (audio_clock_share_buffer_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
}
#endif

void dsp_dc_compensation_start(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
#if AIR_MONITOR_DUBUG_RG_ENABLE
    hal_gpt_status_t gpt_status;
    gpt_status=hal_gpt_sw_get_timer(&rg_debug_timer_handle);
    hal_gpt_sw_start_timer_ms(rg_debug_timer_handle, 500, rg_debug_timer_callback, NULL);
#endif

#ifdef AIR_AUDIO_HARDWARE_ENABLE
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("[DC COMPENSATION] Start\r\n", 0);
    hal_audio_device_parameter_t device_handle;
    hal_audio_set_value_parameter_t set_val_handle;
    memset(&device_handle, 0, sizeof(hal_audio_device_parameter_t));
    memset(&set_val_handle, 0, sizeof(hal_audio_set_value_parameter_t));
    mcu2dsp_open_param_p open_param;
    open_param = (mcu2dsp_open_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    bool dac_performance = open_param->stream_out_param.afe.performance;
    hal_audio_analog_mdoe_t dac_mode = open_param->stream_out_param.afe.dl_dac_mode;
    set_val_handle.analog_output_mode.dac_mode = dac_mode;
    set_val_handle.analog_output_mode.gain_select = AFE_HW_ANALOG_GAIN_OUTPUT;
    hal_audio_set_value(&set_val_handle, HAL_AUDIO_SET_VOLUME_OUTPUT_ANALOG_SETTING);
#if defined(AIR_BTA_IC_PREMIUM_G2)
    if ((dac_mode == HAL_AUDIO_ANALOG_OUTPUT_CLASSAB) ||
        (dac_mode == HAL_AUDIO_ANALOG_OUTPUT_CLASSG)) {
#else
    if ((dac_mode == HAL_AUDIO_ANALOG_OUTPUT_CLASSAB) ||
        (dac_mode == HAL_AUDIO_ANALOG_OUTPUT_CLASSG2) ||
        (dac_mode == HAL_AUDIO_ANALOG_OUTPUT_CLASSG3)) {
#endif
        device_handle.dac.rate = 48000;
        device_handle.dac.dac_mode = dac_mode;

        device_handle.dac.dc_compensation_value = afe.stream_out.dc_compensation_value;
#if defined(AIR_BTA_IC_PREMIUM_G2)
        device_handle.dac.with_high_performance = dac_performance;
#else
        device_handle.dac.performance = dac_performance;
#endif
        device_handle.dac.with_force_change_rate = false;

        hal_audio_set_device(&device_handle, HAL_AUDIO_CONTROL_DEVICE_INTERNAL_DAC_DUAL, HAL_AUDIO_CONTROL_ON);

        AFE_WRITE(0xC0000EDC, 0x11004089);              // Setting voltage high to 0.9v
#if 1//modfiy for ab1568
        //ANA_SET_REG (0xA207012C, 0x1<<2, 0x1<<2); // Enable trim buffer's reference voltage
        ANA_SET_REG(AUDDEC_ANA_CON11, 0x1 << 2, 0x1 << 2); // Enable trim buffer's reference voltage
        DSP_MW_LOG_I("AUDDEC_ANA_CON11 addr:0x%x val:0x%x\r\n", 2, AUDDEC_ANA_CON11, AFE_READ(AUDDEC_ANA_CON11));
#endif
    } else {
        device_handle.analog_mic.audio_device = HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_DUAL;
        device_handle.analog_mic.rate = 48000;
        device_handle.analog_mic.mic_interface = HAL_AUDIO_INTERFACE_1;
        device_handle.analog_mic.bias_select = HAL_AUDIO_BIAS_SELECT_BIAS0;
        #ifdef AIR_BTA_IC_STEREO_HIGH_G3
        device_handle.analog_mic.bias_voltage[0] = HAL_AUDIO_BIAS_VOLTAGE_1_90V;
        #else
        device_handle.analog_mic.bias_voltage[0] = HAL_AUDIO_BIAS_VOLTAGE_1_85V;
        #endif
        device_handle.analog_mic.iir_filter = false;
        device_handle.analog_mic.with_external_bias = false;
        device_handle.analog_mic.with_bias_lowpower = false;
        device_handle.analog_mic.with_phase_inverse = false;
        device_handle.analog_mic.adc_parameter.adc_mode = HAL_AUDIO_ANALOG_INPUT_ACC10K;              //AU_AFE_IN_MODE_t
        device_handle.analog_mic.adc_parameter.performance = dac_performance;  //AU_AFE_POWER_SETTING_t
        device_handle.analog_mic.adc_parameter.is_fifo_sync_clock_ch1 = false;
        device_handle.analog_mic.adc_parameter.with_au_in_swap = false;
        hal_audio_set_device(&device_handle, device_handle.analog_mic.audio_device, HAL_AUDIO_CONTROL_ON);
    }
#if defined(AIR_DAC_MODE_RUNTIME_CHANGE)
    AFE_SET_REG(AFE_ADDA_DL_SDM_DCCOMP_CON, (open_param->stream_out_param.afe.CLD_align_gain & 0x3F) << AFE_ADDA_DL_SDM_DCCOMP_CON_ATTGAIN_CTL_POS, AFE_ADDA_DL_SDM_DCCOMP_CON_ATTGAIN_CTL_MASK);
#endif
//#endif
#else
    UNUSED(msg);
    UNUSED(ack);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
}

void dsp_dc_compensation_stop(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("[DC COMPENSATION] Stop\r\n", 0);
    afe.stream_out.dc_compensation_value = (uint16_t)msg.ccni_message[0];
    hal_audio_device_parameter_t device_handle;
    memset(&device_handle, 0, sizeof(hal_audio_device_parameter_t));
    mcu2dsp_open_param_p open_param;
    open_param = (mcu2dsp_open_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    bool dac_performance = open_param->stream_out_param.afe.performance;
    hal_audio_analog_mdoe_t dac_mode = open_param->stream_out_param.afe.dl_dac_mode;
    bool no_delay_flag = open_param->stream_out_param.afe.misc_parms; // just reuse this parameter
    afe.stream_out.dac_mode = dac_mode;
#if defined(AIR_BTA_IC_PREMIUM_G2)
    if ((dac_mode == HAL_AUDIO_ANALOG_OUTPUT_CLASSAB) ||
        (dac_mode == HAL_AUDIO_ANALOG_OUTPUT_CLASSG)) {
#else
    if ((dac_mode == HAL_AUDIO_ANALOG_OUTPUT_CLASSAB) ||
        (dac_mode == HAL_AUDIO_ANALOG_OUTPUT_CLASSG2) ||
        (dac_mode == HAL_AUDIO_ANALOG_OUTPUT_CLASSG3)) {
#endif
        ANA_WRITE(AUDDEC_ANA_CON6, 0x0070);
        //ANA_SET_REG (AUDDEC_ANA_CON10, 0, 0x1<<2);  // Disable trim buffer's reference voltage
        ANA_SET_REG(AUDDEC_ANA_CON11, 0, 0x1 << 2); // Disable trim buffer's reference voltage
        ANA_WRITE(AUDDEC_ANA_CON6, 0x0030);

        AFE_WRITE(0xC0000EDC, 0x11004004);          // Setting voltage low to 0.42v // Reset classg
        device_handle.dac.rate = 48000;
        device_handle.dac.dac_mode = dac_mode;
#if defined(AIR_BTA_IC_PREMIUM_G2)
        device_handle.dac.with_high_performance = dac_performance;
#else
        device_handle.dac.performance = dac_performance;
#endif
        device_handle.dac.dc_compensation_value = afe.stream_out.dc_compensation_value;
        device_handle.dac.with_force_change_rate = false;
        hal_audio_set_device(&device_handle, HAL_AUDIO_CONTROL_DEVICE_INTERNAL_DAC_DUAL, HAL_AUDIO_CONTROL_OFF);
    } else {
        hal_audio_analog_mdoe_t dl_analog_mode = HAL_AUDIO_ANALOG_OUTPUT_CLASSD;
        device_handle.analog_mic.audio_device = HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_DUAL;
        device_handle.analog_mic.rate = 48000;
        device_handle.analog_mic.mic_interface = HAL_AUDIO_INTERFACE_1;
        device_handle.analog_mic.bias_select = HAL_AUDIO_BIAS_SELECT_BIAS0;
        #ifdef AIR_BTA_IC_STEREO_HIGH_G3
        device_handle.analog_mic.bias_voltage[0] = HAL_AUDIO_BIAS_VOLTAGE_1_90V;
        #else
        device_handle.analog_mic.bias_voltage[0] = HAL_AUDIO_BIAS_VOLTAGE_1_85V;
        #endif
        device_handle.analog_mic.iir_filter = false;
        device_handle.analog_mic.with_external_bias = false;
        device_handle.analog_mic.with_bias_lowpower = false;
        device_handle.analog_mic.with_phase_inverse = false;
        device_handle.analog_mic.adc_parameter.adc_mode = HAL_AUDIO_ANALOG_INPUT_ACC10K;              //AU_AFE_IN_MODE_t
        device_handle.analog_mic.adc_parameter.performance = dac_performance;//AU_AFE_POWER_SETTING_t
        device_handle.analog_mic.adc_parameter.is_fifo_sync_clock_ch1 = false;
        device_handle.analog_mic.adc_parameter.with_au_in_swap = false;
        afe.stream_out.dc_compensation_value = hal_audio_get_value((hal_audio_get_value_parameter_t *)&dl_analog_mode, HAL_AUDIO_GET_DEVICE_DAC_COMPENSATION_VALUE);
        hal_audio_set_device(&device_handle, device_handle.analog_mic.audio_device, HAL_AUDIO_CONTROL_OFF);
    }
//#endif

    DSP_MW_LOG_I("DC COMPENSATION val:0x%x, no_delay_flag = 0x%x\r\n", 2, afe.stream_out.dc_compensation_value, no_delay_flag);

    uint32_t value = 0;
#ifndef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
    if (no_delay_flag == 0) {
        value = 2000;
    }
#endif
    hal_audio_set_value((hal_audio_set_value_parameter_t *)&value, HAL_AUDIO_SET_DEVICE_AMP_DELAY_TIMER_MS);
    if (dac_mode == HAL_AUDIO_ANALOG_OUTPUT_CLASSD) {
        DSP_MW_LOG_I("[DC COMPENSATION] classd unlock amp", 0);
        afe_send_amp_status_ccni(false);
    }
#else
    UNUSED(msg);
    UNUSED(ack);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
}

#ifdef MTK_BT_A2DP_ENABLE
void dsp_alc_switch(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
#ifndef AIR_AUDIO_HARDWARE_ENABLE
    UNUSED(msg);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
    DSP_MW_LOG_I("a2dp alc switch :%d\r\n", 1, msg.ccni_message[1]);
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    Audio_setting->Audio_sink.alc_enable = (bool)(uint32_t)(msg.ccni_message[1]);
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
}
#endif /* MTK_BT_A2DP_ENABLE */

#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
void dsp_peq_set_param(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    BOOL BypassTimestamp = FALSE;
    mcu2dsp_peq_param_p peq_param;
    peq_param = (mcu2dsp_peq_param_p)hal_memview_mcu_to_dsp0(msg.ccni_message[1]);

    if (((msg.ccni_message[0] & 0xFFFF) == PEQ_DISABLE_ALL) || ((msg.ccni_message[0] & 0xFFFF) == PEQ_ON_ALL)) {
        Audio_Peq_Enable_Control(msg);
        return;
    }

    DSP_STREAMING_PARA_PTR  pStream;
    if (peq_param->setting_mode == PEQ_SYNC) {
        if (n9_a2dp_if.source != NULL && n9_a2dp_if.sink != NULL) {
            pStream = DSP_Streaming_Get(n9_a2dp_if.source, n9_a2dp_if.sink);
            if (pStream == NULL) {
                BypassTimestamp = FALSE;
#ifdef MTK_CELT_DEC_ENABLE
            } else if (pStream->callback.FeatureTablePtr->ProcessEntry == stream_codec_decoder_celt_process) {
                BypassTimestamp = TRUE;
#endif
            }
        }else{
            BypassTimestamp = TRUE;
        }
    }

    DSP_MW_LOG_I("PEQ set param with phase %d,BypassTimestamp:%d\r\n", 2, peq_param->phase_id,BypassTimestamp);
    PEQ_Set_Param(msg, ack, BypassTimestamp);

#if defined(AIR_DRC_ENABLE)
    if ((msg.ccni_message[0] & 0xFFFF) == PEQ_AUDIO_PATH_A2DP) {
        Audio_CPD_Enable(peq_param->drc_enable, peq_param->phase_id, peq_param->drc_force_disable, PEQ_AUDIO_PATH_A2DP, peq_param->peq_nvkey_id);
    } else if ((msg.ccni_message[0] & 0xFFFF) == PEQ_AUDIO_PATH_LINEIN) {
        Audio_CPD_Enable(peq_param->drc_enable, peq_param->phase_id, peq_param->drc_force_disable, PEQ_AUDIO_PATH_LINEIN, peq_param->peq_nvkey_id);
#ifdef AIR_VP_PEQ_ENABLE
    } else if ((msg.ccni_message[0] & 0xFFFF) == PEQ_AUDIO_PATH_VP) {
        Audio_CPD_Enable(peq_param->drc_enable, peq_param->phase_id, peq_param->drc_force_disable, PEQ_AUDIO_PATH_VP, peq_param->peq_nvkey_id);
#ifdef AIR_ADAPTIVE_EQ_ENABLE
    } else if ((msg.ccni_message[0] & 0xFFFF) == PEQ_AUDIO_PATH_VP_AEQ) {
        Audio_CPD_Enable(peq_param->drc_enable, peq_param->phase_id, peq_param->drc_force_disable, PEQ_AUDIO_PATH_VP_AEQ, peq_param->peq_nvkey_id);
#endif
#endif
    } else if ((msg.ccni_message[0] & 0xFFFF) == PEQ_AUDIO_PATH_USB) {
        Audio_CPD_Enable(peq_param->drc_enable, peq_param->phase_id, peq_param->drc_force_disable, PEQ_AUDIO_PATH_USB, peq_param->peq_nvkey_id);
    } else {
        Audio_CPD_Enable(peq_param->drc_enable, peq_param->phase_id, peq_param->drc_force_disable, PEQ_AUDIO_PATH_ADAPTIVE_EQ, peq_param->peq_nvkey_id);
    }
#endif
}
#endif

#if defined(AIR_ADAPTIVE_EQ_ENABLE) && defined(MTK_PEQ_ENABLE)
void dsp_adaptive_eq_set_param(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    uint8_t adaptive_eq_number;
    adaptive_eq_number = (msg.ccni_message[0] & 0xFFFF);

    if(adaptive_eq_number == AEQ_GET_INDEX_INFO){
        Adaptive_Eq_Set_Share_Info(msg, ack);
    }else if(adaptive_eq_number == AEQ_SET_DETECT_BYPASS_ENABLE){
        Adaptive_Eq_Set_Detect_Bypass(true);
    }else if(adaptive_eq_number == AEQ_SET_DETECT_BYPASS_DISABLE){
        Adaptive_Eq_Set_Detect_Bypass(false);
    }else{
        DSP_MW_LOG_I("adaptive peq set param number %d\r\n", 1, adaptive_eq_number);
        Adaptive_Eq_Set_Param(msg, ack);
    }
}
#endif

#ifdef MTK_GAMING_MODE_HEADSET

static void trigger_a2dp_airoha_proc_timer_callback(void *user_data)
{
    UNUSED(user_data);
//    hal_gpio_set_output(HAL_GPIO_9, 1);//debug for play en decode 1st frame
    U32 bt_clk_cur;
    U16 intra_clk_cur;
    MCE_GetBtClk((BTCLK *)&bt_clk_cur, (BTPHASE *)&intra_clk_cur, ULL_CLK_Offset);
    DSP_MW_LOG_I("[AiroGaming]Play En Timer Callback Decode 1st Frame,Clk:0x%x,phase:0x%x", 2, bt_clk_cur, intra_clk_cur);
    if ((n9_a2dp_if.source != NULL) && (n9_a2dp_if.sink != NULL) && (n9_a2dp_if.transform != NULL) && (n9_a2dp_if.sink->param.audio.irq_exist == FALSE)) {
        n9_a2dp_if.source->transform->Handler(n9_a2dp_if.source, n9_a2dp_if.sink);

        xTaskResumeFromISR(n9_a2dp_if.source->taskId);
    } else {
        DSP_MW_LOG_W("[AiroGaming] Trigger 1ST Decode faill", 0);
    }
    hal_gpt_sw_free_timer(g_play_en_gpt_handler);
    g_play_en_gpt_handler = NULL;
//    hal_gpio_set_output(HAL_GPIO_9, 0);//debug for play en decode 1st frame
}

void dsp_a2dp_set_dl_play_en(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    BTCLK bt_clk;
    BTPHASE bt_phase;
    audio_dsp_a2dp_dl_play_en_param_p play_en_param;

    play_en_param = (audio_dsp_a2dp_dl_play_en_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    if (n9_a2dp_if.sink->param.audio.channel_num >= 2) {
        n9_a2dp_if.sink->streamBuffer.BufferInfo.WriteOffset = (120 << 2) << 1;
    } else {
        n9_a2dp_if.sink->streamBuffer.BufferInfo.WriteOffset = (120 << 2);
    }
    AFE_SET_REG(AFE_MEMIF_PBUF_SIZE, 0, 3 << 0); //reduce pbuffer size
    AFE_SET_REG(AFE_MEMIF_MINLEN, 1, 0XF);//reduce pbuffer size
    DSP_MW_LOG_I("A2DP DL - Play En SeqNo:%d, BtClk:0x%x", 2, play_en_param->sequence_number, play_en_param->bt_clock);

    audio_headset_ull_ctrl.play_en.sequence_number = play_en_param->sequence_number;
    //audio_headset_ull_ctrl.play_en.bt_clock = ((play_en_param->bt_clock + 28 /* 8.75ms DL reserve time */) & BTCLK_27_0_MASK);
    audio_headset_ull_ctrl.play_en.bt_clock = ((play_en_param->bt_clock + 22 - 2 /* 6.5ms DL reserve time */) & BTCLK_27_0_MASK);
    //================debug test===================
    BTCLK bt_clk_1, bt_clk_2, bt_clk_3, bt_clk_4;
    BTPHASE bt_phase_1, bt_phase_2, bt_phase_3, bt_phase_4;
    MCE_Get_BtClkOffset(&bt_clk_1, &bt_phase_1, ULL_CLK_Offset);
    MCE_Get_BtClkOffset(&bt_clk_2, &bt_phase_2, BT_CLK_Offset);

    MCE_GetBtClk(&bt_clk_3, &bt_phase_3, ULL_CLK_Offset);
    MCE_GetBtClk(&bt_clk_4, &bt_phase_4, BT_CLK_Offset);
	DSP_MW_LOG_I("[MCE]ULL_CLK_Offset != BT_CLK_Offset , ULL_CLK:0x%x, BT_CLK:0x%x", 2, bt_clk_3, bt_clk_4);
    if ((bt_clk_1 != bt_clk_2) && (rBb_ull->rValid)) {
        AUDIO_ASSERT(0);
    }
    //==========================================
    if (audio_headset_ull_ctrl.is_a2dp_started) {

        MCE_GetBtClk(&bt_clk, &bt_phase, ULL_CLK_Offset);
        audio_headset_ull_ctrl.play_en.bt_clock += ((n9_a2dp_if.source->param.n9_a2dp.sink_latency - ULL_BASIC_LATENCY) * 8 / 2500); /* 8/2500 = 312.5 for frame based  */

        DSP_MW_LOG_I("A2DP DL - A2DP Start First, Current BtClk:0x%x, Phase:0x%x", 2, bt_clk, bt_phase);

        a2dp_ull_get_proper_play_en(audio_headset_ull_ctrl.play_en.bt_clock,
                                    audio_headset_ull_ctrl.play_en.sequence_number,
                                    &audio_headset_ull_ctrl.play_en.bt_clock,
                                    (U8 *)&audio_headset_ull_ctrl.play_en.sequence_number);
        a2dp_ull_set_init_seq_no(audio_headset_ull_ctrl.play_en.sequence_number);

        MCE_TransBT2NativeClk(audio_headset_ull_ctrl.play_en.bt_clock, 625/*625 1 bt clk,22-1+1 */, &bt_clk, &bt_phase, ULL_CLK_Offset);

        hal_audio_afe_set_play_en(bt_clk, bt_phase);
        DSP_MW_LOG_I("[AiroGaming]AFE Set Val: bt_clk:0x%x, bt_phase:0x%x", 2, bt_clk, bt_phase);
#ifdef TRIGGER_A2DP_PROCSESS_TIMER
        U32 bt_clk_cur;
        U16 intra_clk_cur;

        if (g_play_en_gpt_handler == NULL) {
            if (hal_gpt_sw_get_timer((uint32_t *)&g_play_en_gpt_handler) != HAL_GPT_STATUS_OK) {
                DSP_MW_LOG_W("[AiroGaming]play en get timer fail", 0);
                AUDIO_ASSERT(0);
            }
        }
        MCE_GetBtClk((BTCLK *)&bt_clk_cur, (BTPHASE *)&intra_clk_cur, ULL_CLK_Offset);
        U32 FirstDecodeCnt = ((((audio_headset_ull_ctrl.play_en.bt_clock) - (bt_clk_cur)) >> 1) * 625 - (intra_clk_cur >> 1) - 625);
        hal_gpt_sw_start_timer_us(g_play_en_gpt_handler, FirstDecodeCnt, trigger_a2dp_airoha_proc_timer_callback, NULL);
        DSP_MW_LOG_I("[AiroGaming]bclk_play_en:0x%x, bclk_cur:0x%x, timer_duration:%d us", 3, audio_headset_ull_ctrl.play_en.bt_clock, bt_clk_cur, FirstDecodeCnt);

        /*        hal_gpio_init(HAL_GPIO_9);                    //debug for play en decode 1st frame
                hal_pinmux_set_function(HAL_GPIO_9, 0);
                hal_gpio_set_direction(HAL_GPIO_9, HAL_GPIO_DIRECTION_OUTPUT);
                hal_gpio_set_output(HAL_GPIO_9, 0);*/

#endif
    } else {
        DSP_MW_LOG_I("A2DP DL - A2DP Play En First", 0);
    }

    audio_headset_ull_ctrl.is_play_en_ready = TRUE;

    UNUSED(ack);
}
#endif

void dsp_set_sysram_addr(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("DSP NVDM init\r\n", 0);

    U32 sysram_start_addr;

    sysram_start_addr = hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    dsp0_nvkey_init(sysram_start_addr, NAT_MEM_Size);// SysRam fixed 3k size
}

void dsp_streaming_deinit_all(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    bool enable = (bool)(uint16_t)msg.ccni_message[1];
    audio_stream_deinit_id_t stream_deinit = (audio_stream_deinit_id_t)(msg.ccni_message[0] & 0xFFFF);

    UNUSED(enable);

    DSP_MW_LOG_I("DSP stream deinit,mute status :%d, ID:%d \r\n", 2, enable, stream_deinit);

    if (stream_deinit == AUDIO_STRAM_DEINIT_VOICE_AEC) {
#ifdef AIR_VOICE_NR_ENABLE
        stream_function_aec_nr_deinitialize(true);
#endif
    } else if (stream_deinit == AUDIO_STRAM_DEINIT_ANC_MONITOR) {
#ifdef AIR_ANC_WIND_DETECTION_ENABLE
        stream_function_wind_reset_status();
#endif
#ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
        stream_function_environment_detection_reset_status();
#endif
    } else if(stream_deinit == AUDIO_STRAM_DEINIT_VOICE_CPD){
#if defined(AIR_DRC_ENABLE) || defined(AIR_VOICE_DRC_ENABLE)
            stream_function_cpd_deinitialize(true);
#endif
    } else if(stream_deinit == AUDIO_STRAM_DEINIT_ULL_DL){
        dsp_stream_deinit_all();
        #if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
        DLLT_StreeamingDeinitAll();
        #endif

        #if defined(AIR_DRC_ENABLE) || defined(AIR_VOICE_DRC_ENABLE)
        stream_function_cpd_deinitialize(true);
        #endif
    } else {
        dsp_stream_deinit_all();
        #if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
        DLLT_StreeamingDeinitAll();
        #endif
        //hal_audio_mute_stream_out(enable);
#ifndef AIR_DCHS_MODE_ENABLE
#ifdef AIR_VOICE_NR_ENABLE
        stream_function_aec_nr_deinitialize(false);
#if defined(AIR_3RD_PARTY_NR_ENABLE) && !defined(AIR_BTA_IC_STEREO_HIGH_G3)
        stream_function_tx_eq_deinitialize(false);
#endif
#endif
#if defined(AIR_DRC_ENABLE) || defined(AIR_VOICE_DRC_ENABLE)
        stream_function_cpd_deinitialize(false);
#endif
#ifdef MTK_LINEIN_INS_ENABLE
        stream_function_ins_deinitialize();
#endif
#endif
    }
}

#ifdef AIR_VOICE_NR_ENABLE
void CB_CM4_SCO_DL_AVC_STATUS_UPDATE(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    U16 ctrl_type = (msg.ccni_message[0] & 0x00FF);
    if(ctrl_type == 0){ //update volume
        voice_avc_vol_receive_update(msg.ccni_message[1]);
    } else if(ctrl_type == 1) { //enable/disable ctrl
        voice_avc_set_enable(msg.ccni_message[1]);
    }
}

void dsp_get_reference_gain(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("DSP get reference gain\r\n", 0);
    S16 *Refgain_start_addr;
    Refgain_start_addr = (S16 *)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    voice_nr_get_ref_gain(Refgain_start_addr);
}

#ifdef AIR_AIRDUMP_ENABLE
void CB_CM4_SCO_AIRDUMP_EN(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    if (msg.ccni_message[1] == 1) {
        DSP_MW_LOG_I("[AirDump][DSP] AIRDUMP Start\r\n", 0);
        voice_nr_airdump_on_off(TRUE);
    } else if (msg.ccni_message[1] == 0) {
        DSP_MW_LOG_I("[AirDump][DSP] AIRDUMP Stop\r\n", 0);
        voice_nr_airdump_on_off(FALSE);
    }
}
#endif

#endif

void CB_CM4_SCO_DL_LEQ_UPDATE(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);

#if defined (AIR_VOICE_DRC_ENABLE) && defined(AIR_HEARING_PROTECTION_ENABLE)
    voice_leq_receive_update(msg.ccni_message[1]);
#else
    UNUSED(msg);
#endif
}

typedef struct {
    uint32_t type;
    uint32_t length;
    uint8_t version[32];
} race_dsprealtime_query_lib_version_noti_struct_t;

void dsp_query_lib_version(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    race_dsprealtime_query_lib_version_noti_struct_t *ver_struct;

    UNUSED(ack);

    ver_struct = (race_dsprealtime_query_lib_version_noti_struct_t *)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    if (ver_struct->type == DSP_ALG_LIB_TYPE_IGO) {
#ifdef AIR_VOICE_NR_ENABLE
        ver_struct->length = voice_nr_query_version(ver_struct->type, ver_struct->version);
#else
        ver_struct->length = 0;
#endif
    } else {
        ver_struct->length = 0;
        DSP_MW_LOG_I("DSP query lib version fail, type %d\r\n", 1, ver_struct->type);
        return;
    }

    DSP_MW_LOG_I("DSP query lib version success\r\n", 0);
}

#ifdef AIR_LD_NR_ENABLE
#include "audio_nvdm_common.h"
#include "preloader_pisplit.h"
#include "ld_nr_interface.h"
#define LD_NR_NVKEY_SIZE                sizeof(DSP_PARA_AT_LD_STRU)
void *mic_ld_nr_features_set_param(U16 nvkey_id, void *p_nvkey, void *p_NVKEY)
{
    if (p_nvkey == NULL) {
        //PSAP_LOG_E(g_PSAP_msg_id_string_0, "hearing-aid feature set param fail, p_nvkey==NULL", 0);
        return 0;
    }
    void *p = NULL;
    switch (nvkey_id) {
        case NVKEY_DSP_PARA_MIC1_LD_NR_PARAMETER1:
        case NVKEY_DSP_PARA_MIC1_LD_NR_PARAMETER2:
        case NVKEY_DSP_PARA_MIC2_LD_NR_PARAMETER1:
        case NVKEY_DSP_PARA_MIC2_LD_NR_PARAMETER2: {
            if (p_NVKEY == NULL) {
                p = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, LD_NR_NVKEY_SIZE);
            } else {
                DSP_MW_LOG_I("[LD_NR] key buffer already malloc:0x%x", 1, p_NVKEY);
                p = p_NVKEY;
                //configASSERT(0);
            }
            memcpy((U8 *)p, (U8 *)p_nvkey, LD_NR_NVKEY_SIZE);
            break;
        }
        default:
            //PSAP_LOG_E(g_PSAP_msg_id_string_6, "hearing-aid feature set param fail, nvkey 0x%x not found", 1, nvkey_id);
            break;
    }
    return p;
}
#endif
void dsp_set_algorithm_param(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    void *share_ptr = (void *)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    uint16_t nvkey_id = (uint16_t)(msg.ccni_message[0] & 0xFFFF);
    DSP_MW_LOG_I("DSP set algorithm parameters nvkey_ID:0x%x, ptr:0x%x", 2, nvkey_id, share_ptr);

    if (0) {
#ifdef AIR_HEARTHROUGH_VIVID_PT_ENABLE
    } else if (nvkey_id == NVID_DSP_ALG_VIVID_PT_LDNR){
        stream_function_vivid_passthru_load_nvkey(NVID_DSP_ALG_VIVID_PT_LDNR, share_ptr);
#endif
#ifdef AIR_ANC_WIND_DETECTION_ENABLE
    } else if ((nvkey_id == NVID_DSP_ALG_ANC_WIND_DET) ||
               (nvkey_id == NVID_DSP_ALG_ANC_WIND_DET_PT) ||
               (nvkey_id == NVID_DSP_ALG_ANC_WIND_DET_SIDETONE) ||
               (nvkey_id == NVID_DSP_ALG_ANC_WIND_DET_HW_VIVID_PT) ||
               (nvkey_id == NVID_DSP_ALG_ANC_WIND_DET_HA_PSAP) ||
               (nvkey_id == NVID_DSP_ALG_ANC_WIND_DET_SW_VIVID_PT) ||
               (nvkey_id == NVID_DSP_ALG_ANC_WIND_DET_LLF)) {
        stream_function_wind_load_nvkey(share_ptr);
#endif
#ifdef AIR_AUDIO_HARDWARE_ENABLE
#ifdef AIR_SILENCE_DETECTION_ENABLE
    } else if (nvkey_id == NVID_DSP_ALG_SIL_DET){
        Sink_Audio_SilenceDetection_Load_Nvkey(share_ptr);
#endif
#endif
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
    } else if (nvkey_id == NVID_DSP_ALG_ANC_USR_UNAWARE) {
        stream_function_user_unaware_load_nvkey(share_ptr);
#endif
#ifdef MTK_USER_TRIGGER_FF_ENABLE
    } else if (nvkey_id == NVKEY_DSP_PARA_ADAPTIVE_FF) {
        stream_function_user_trigger_ff_load_nvkey(share_ptr);
#endif
#ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
    } else if ((nvkey_id == NVID_DSP_ALG_ANC_ENVIRONMENT_DETECTION) ||
               (nvkey_id == NVID_DSP_ALG_ANC_ENVIRONMENT_DETECTION_PT) ||
               (nvkey_id == NVID_DSP_ALG_ANC_ENVIRONMENT_DETECTION_SIDETONE) ||
               (nvkey_id == NVID_DSP_ALG_ANC_ENVIRONMENT_DETECTION_HW_VIVID_PT) ||
               (nvkey_id == NVID_DSP_ALG_ANC_ENVIRONMENT_DETECTION_HA_PSAP) ||
               (nvkey_id == NVID_DSP_ALG_ANC_ENVIRONMENT_DETECTION_SW_VIVID_PT) ||
               (nvkey_id == NVID_DSP_ALG_ANC_ENVIRONMENT_DETECTION_LLF)) {
        stream_function_environment_detection_load_nvkey(share_ptr);
#endif
#ifdef AIR_ADVANCED_PASSTHROUGH_ENABLE
    } else if ((nvkey_id >= NVKEY_DSP_PARA_AFC) && (nvkey_id <= NVKEY_DSP_PARA_ADVANCED_PASSTHROUGH_MISC)){
        advanced_passthrough_hearing_aid_features_set_param(nvkey_id, share_ptr);
#endif
#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
    } else if (((nvkey_id >= NVID_DSP_ALG_HA_MP_DATA) && (nvkey_id <= NVID_DSP_ALG_HA_CUS_SETTING_BK)) || (nvkey_id == HA_RUNTIME_SYNC_PARA_ID)){
        stream_function_hearing_aid_load_nvkey(nvkey_id, share_ptr);
#endif
#ifdef AIR_LD_NR_ENABLE
    } else if ((nvkey_id >= NVKEY_DSP_PARA_MIC1_LD_NR_PARAMETER1) && (nvkey_id <= NVKEY_DSP_PARA_MIC2_LD_NR_PARAMETER2)){
#ifdef AIR_RECORD_ADVANCED_ENABLE
        extern void *p_advanced_record_ld_nr_key;
        p_advanced_record_ld_nr_key = mic_ld_nr_features_set_param(nvkey_id, share_ptr, p_advanced_record_ld_nr_key);
#endif
#if defined(AIR_WIRED_AUDIO_ENABLE) && defined(AIR_WIRELESS_MIC_TX_ENABLE)
        extern void *p_wired_audio_usb_out_ld_nr_key;
        p_wired_audio_usb_out_ld_nr_key = mic_ld_nr_features_set_param(nvkey_id, share_ptr, p_wired_audio_usb_out_ld_nr_key);
#endif
#if defined(AIR_WIRELESS_MIC_TX_ENABLE)
        extern void *p_wireless_mic_ld_nr_key;
        p_wireless_mic_ld_nr_key = mic_ld_nr_features_set_param(nvkey_id, share_ptr, p_wireless_mic_ld_nr_key);
#endif
#endif
    } else {
        DSP_MW_LOG_E("DSP set algorithm parameters nvkey id is unsupported 0x%x", 1, nvkey_id);
        UNUSED(share_ptr);
        UNUSED(nvkey_id);
    }
    UNUSED(ack);
}

#ifdef AIR_ANC_USER_UNAWARE_ENABLE
audio_extend_gain_control_t g_audio_extend_gain_ctrl[AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_NUM];
#endif
void dsp_anc_monitor_extend_gain_ctrl_init(void)
{
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
    memset(g_audio_extend_gain_ctrl, 0, sizeof(g_audio_extend_gain_ctrl));
#endif
}

void dsp_anc_monitor_extend_gain_ctrl_set(audio_extend_gain_control_ptr_t gain_control)
{
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
    if ((gain_control->gain_type < AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_NUM)
        && (gain_control->gain_type != AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_USER_UNAWARE)) {
        memcpy(&g_audio_extend_gain_ctrl[gain_control->gain_type], gain_control, sizeof(audio_extend_gain_control_t));
    }
#else
    UNUSED(gain_control);
#endif
}

void dsp_anc_monitor_extend_gain_ctrl_get_total(int16_t *get_total_gain0, int16_t *get_total_gain1)
{
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
    int16_t total_gain[2] = {0, 0};
    for (int i = 0; i < AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_NUM; i++) {
        if (i == AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_USER_UNAWARE) {
            continue;
        }
        total_gain[0] += g_audio_extend_gain_ctrl[i].gain[0];
        total_gain[1] += g_audio_extend_gain_ctrl[i].gain[1];
    }
    *get_total_gain0 = total_gain[0];
    *get_total_gain1 = total_gain[1];
#else
    UNUSED(get_total_gain0);
    UNUSED(get_total_gain1);
#endif
}

void dsp_ccni_set_extend_gain(audio_extend_gain_control_ptr_t gain_control)
{
    hal_ccni_message_t ccni_msg;
    memset((void *)&ccni_msg, 0, sizeof(hal_ccni_message_t));
    ccni_msg.ccni_message[0] = ((MSG_DSP2MCU_COMMON_AUDIO_ANC_ADAPTIVE << 16) |
                                (gain_control->gain_type & 0xFF) |
                                (((U16)gain_control->misc) << 8));
    ccni_msg.ccni_message[1] = (((gain_control->gain[1] & 0xFFFF) << 16) |
                                (gain_control->gain[0] & 0xFFFF));
    aud_msg_tx_handler(ccni_msg, 0, FALSE);

    dsp_anc_monitor_extend_gain_ctrl_set(gain_control);
}

#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
void dsp_set_adaptive_anc(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    uint32_t set_type;
    uint32_t data;
    set_type = (uint32_t)(msg.ccni_message[0] & 0xFFFF);
    data = msg.ccni_message[1];
    UNUSED(ack);
    if (0) {
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
    } else if (set_type < AUDIO_ANC_MONITOR_SET_USER_UNAWARE_MAX) {
        stream_function_user_unaware_inform_handler(set_type, data);
#endif
#ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
    } else if (set_type < AUDIO_ANC_MONITOR_SET_ENVIRONMENT_DETECTION_MAX) {
        stream_function_environment_detection_set_value(set_type, data);
#endif
#ifdef AIR_ANC_WIND_DETECTION_ENABLE
    } else if (set_type < AUDIO_ANC_MONITOR_SET_WIND_DETECT_MAX) {
        stream_function_wind_detection_set_value(set_type, data);
#endif
    } else {
        DSP_MW_LOG_I("DSP set adaptive anc, not support type:0x%x", 1, set_type);
    }
}

void dsp_get_adaptive_anc(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    uint32_t get_type;
    uint32_t *data_addr;

    get_type = (uint32_t)(msg.ccni_message[0] & 0xFFFF);
    data_addr = (uint32_t *)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    if (0) {
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
    } else if (get_type == AUDIO_ANC_MONITOR_GET_USER_UNAWARE_GAIN_INFO) {
        stream_function_user_unaware_get_info((int32_t *)data_addr);
    } else if (get_type == AUDIO_ANC_MONITOR_GET_USER_UNAWARE_ENABLE_STAT) {
        *data_addr = (uint32_t)stream_function_user_unaware_get_enable_state();
#endif
#ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
    } else if (get_type == AUDIO_ANC_MONITOR_GET_ENVIRONMENT_DETECTION_STATIONARY_NOISE) {
        *data_addr = (uint32_t)stream_function_environment_detection_get_value(get_type);
#endif
    } else {
        DSP_MW_LOG_I("DSP get adaptive anc, not support type:0x%x", 1, get_type);
    }
}
#else
void dsp_set_adaptive_anc(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
}
void dsp_get_adaptive_anc(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
}
#endif

#ifdef AIR_ANC_USER_UNAWARE_ENABLE
const uint32_t gUser_Unaware_en = 0x00270027;
void dsp_get_user_unaware_info(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    uint16_t type = (uint16_t)(msg.ccni_message[0] & 0xFFFF);
    S32 *UU_info_start_addr;
    UU_info_start_addr = (S32 *)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    if (type == 0) {
        stream_function_user_unaware_get_info(UU_info_start_addr);
    } else if (type == 1) {
        *UU_info_start_addr = (U32)stream_function_user_unaware_get_enable_state();
    }
}
#else
const uint32_t gUser_Unaware_en = 0;

#endif

#ifdef MTK_PROMPT_SOUND_ENABLE
#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
#define DSP_VP_POLLING_TIME 200  //us
#else
#define DSP_VP_POLLING_TIME 600  //us
#endif
uint32_t gDSP_VP_GPT_TIMER_HANDLE = 0;
uint32_t gDSP_VP_GPT_TARGET_TIME = 0;
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void CB_CM4_VP_PLAYBACK_GPT_CALLBACK(void *user_data)
{
    UNUSED(user_data);
    uint32_t savedmask;
    uint32_t curr_cnt = 0;
    U32 bt_clk;
    U16 intra_clk;
    hal_nvic_save_and_set_interrupt_mask_special(&savedmask); // Critical region: over 100us.
    //hal_gpio_set_output(HAL_GPIO_2, 0);
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
    //DSP_MW_LOG_I("[VPC][SYNC]Enter VP DSP polling!!, curr_cnt = %d.\n", 1, curr_cnt);
    if (gDSP_VP_GPT_TARGET_TIME > curr_cnt) {
        while (1) {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
            //DSP_MW_LOG_I("[VPC][SYNC]polling!!, curr_cnt = %d, Target_cnt = %d.\n", 2, curr_cnt, gDSP_VP_GPT_TARGET_TIME);
            if (curr_cnt >= gDSP_VP_GPT_TARGET_TIME) {
                break;
            }
        }
        // *((volatile uint32_t *)(0xC0000010)) |= 0x00000004; //Patch test
        //hal_gpio_set_output(HAL_GPIO_2, 1);
        hal_audio_trigger_start_parameter_t sw_trigger_start;
        sw_trigger_start.enable = true;
        sw_trigger_start.memory_select = playback_vp_if.sink->param.audio.mem_handle.memory_select;
        hal_audio_set_value((hal_audio_set_value_parameter_t *)&sw_trigger_start, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
        MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&intra_clk, BT_CLK_Offset);
        //hal_gpio_set_output(HAL_GPIO_2, 0);
    } else {
        if ((curr_cnt - gDSP_VP_GPT_TARGET_TIME) > 0x80000000) {
            //gDSP_VP_GPT_TARGET_TIME overflow
            while (1) {
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
                //DSP_MW_LOG_I("[VPC][SYNC]polling!!, curr_cnt = %d, Target_cnt = %d.\n", 2, curr_cnt, gDSP_VP_GPT_TARGET_TIME);
                if (curr_cnt >= gDSP_VP_GPT_TARGET_TIME) {
                    if ((curr_cnt & 0x80000000) == 0x0) {
                        break;
                    }
                }
            }
            // *((volatile uint32_t *)(0xC0000010)) |= 0x00000004; //Patch test
            //hal_gpio_set_output(HAL_GPIO_2, 1);
            hal_audio_trigger_start_parameter_t sw_trigger_start;
            sw_trigger_start.enable = true;
            sw_trigger_start.memory_select = playback_vp_if.sink->param.audio.mem_handle.memory_select;
            hal_audio_set_value((hal_audio_set_value_parameter_t *)&sw_trigger_start, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
            MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&intra_clk, BT_CLK_Offset);
            //hal_gpio_set_output(HAL_GPIO_2, 0);
        } else {
            //hal_gpio_set_output(HAL_GPIO_2, 1);
            // *((volatile uint32_t *)(0xC0000010)) |= 0x00000004;
            hal_audio_trigger_start_parameter_t sw_trigger_start;
            sw_trigger_start.enable = true;
            sw_trigger_start.memory_select = playback_vp_if.sink->param.audio.mem_handle.memory_select;
            hal_audio_set_value((hal_audio_set_value_parameter_t *)&sw_trigger_start, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
            //hal_gpio_set_output(HAL_GPIO_2, 0);
            MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&intra_clk, BT_CLK_Offset);
            hal_nvic_restore_interrupt_mask_special(savedmask);
            DSP_MW_LOG_E("[CM4_VP_PB][ERROR] polling callback t(%d) < c(%d) cnt error", 2, gDSP_VP_GPT_TARGET_TIME, curr_cnt);
            hal_nvic_save_and_set_interrupt_mask_special(&savedmask);
        }
    }
    hal_nvic_restore_interrupt_mask_special(savedmask);

    hal_gpt_status_t gpt_status = hal_gpt_sw_free_timer(gDSP_VP_GPT_TIMER_HANDLE);
    if (HAL_GPT_STATUS_OK != gpt_status) {
        DSP_MW_LOG_I("[VPC][SYNC]VP free one_shot Timer error!!, error id = %d.\n", 1, gpt_status);
    }
    DSP_MW_LOG_I("[VPC][SYNC]Polling end!!, curr_cnt = %d,target_cnt = %d. bt clk 0x%x phase 0x%x\n", 4, curr_cnt, gDSP_VP_GPT_TARGET_TIME, bt_clk, intra_clk);
    gDSP_VP_GPT_TIMER_HANDLE = 0;
    gDSP_VP_GPT_TARGET_TIME = 0;
    dsp_audio_msg_ack(MCU2DSP_SYNC_REQUEST_VP << 8 | MCU2DSP_SYNC_REQUEST_START, true); // replace with volume sync mechanism
}

/* CM4 VP Playback CCNI callback function */
volatile uint32_t vp_config_flag = 0;
extern volatile uint32_t vp_sram_empty_flag;
void CB_CM4_VP_PLAYBACK_OPEN(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("[CM4_VP_PB] Open", 0);
    vp_config_flag = 0;
    vp_sram_empty_flag = 0;
    uint32_t frame_size = 0;
    /* remap to non-cacheable address */
    mcu2dsp_open_param_p open_param;
    open_param = (mcu2dsp_open_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);

#ifdef ENABLE_HWSRC_CLKSKEW
#ifdef AIR_HWSRC_TX_TRACKING_ENABLE
    open_param->stream_out_param.afe.clkskew_mode = CLK_SKEW_V1;
#endif
#endif
    DSP_REMAP_SHARE_INFO(open_param->stream_in_param.playback.p_share_info, n9_dsp_share_info_ptr);

    playback_vp_if.source = dsp_open_stream_in(open_param);//StreamCM4VPPlaybackSource(share_info);
    playback_vp_if.sink   = dsp_open_stream_out(open_param);//StreamAudioAfe2Sink(AUDIO_HARDWARE_PCM, INSTANCE_A, AUDIO_CHANNEL_A_AND_B);
    playback_vp_if.transform = NULL;
    playback_vp_if.pfeature_table = stream_feature_list_prompt;

    DSP_Callback_PreloaderConfig(playback_vp_if.pfeature_table, open_param->audio_scenario_type);
#ifdef AIR_VP_PEQ_ENABLE
    frame_size = 512; //PEQ support 44.1 and 48 sampling rate
#else
    frame_size = playback_vp_if.sink->param.audio.format_bytes * playback_vp_if.sink->param.audio.src_rate * playback_vp_if.sink->param.audio.period / 1000;
    frame_size &= (~7UL);  //alignment
#endif

#ifdef AIR_VP_PEQ_ENABLE
    if ((frame_size & (uint8_t)(~0x80)) != 0) {
        // Assert if frame size is not divisible by 32*4 for VP PEQ/DRC post-processing
        AUDIO_ASSERT(0);
    }
#endif

    playback_vp_if.source->sif.SourceConfigure(playback_vp_if.source, AUDIO_SOURCE_FRAME_SIZE, frame_size);
    //stream_feature_configure_type(stream_feature_list_prompt, CODEC_PCM_COPY, CONFIG_DECODER);
#ifdef AIR_ANC_ADAPTIVE_CLOCK_CONTROL_ENABLE
    dsp_adapt_anc_send_unstable_status(AUDIO_ADAPTIVE_ANC_CLK_CTRL_TYPE_DL_VP, 1);
#endif
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void CB_CM4_VP_PLAYBACK_START(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("[CM4_VP_PB] Start", 0);
    bool time_out_flag  = 0;
    uint32_t curr_cnt   = 0;
    uint32_t target_cnt = 0;
    uint32_t time_out   = 0;
    uint32_t savedmask;
    uint32_t err_flag   = 0;
    hal_gpt_status_t gpt_status;
    mcu2dsp_start_param_p start_param;
    start_param = (mcu2dsp_start_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);

    dsp_start_stream_in(start_param, playback_vp_if.source);
    dsp_start_stream_out(start_param, playback_vp_if.sink);

    playback_vp_if.transform = TrasformAudio2Audio(playback_vp_if.source, playback_vp_if.sink, stream_feature_list_prompt);
    if (playback_vp_if.transform == NULL)
    {
        DSP_MW_LOG_E("[CM4_VP_PB] transform failed", 0);
    }
    hal_nvic_save_and_set_interrupt_mask_special(&savedmask);
    if (playback_vp_if.sink->param.audio.aws_sync_request) {
        //hal_gpio_set_output(HAL_GPIO_2, 1);
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
        target_cnt = playback_vp_if.sink->param.audio.aws_sync_time;
        if (target_cnt < curr_cnt) {
            if ((curr_cnt - target_cnt) > 0x80000000) {
                //target_cnt overflow
                time_out = (0xFFFFFFFF - curr_cnt) + target_cnt - DSP_VP_POLLING_TIME;
            } else {
                time_out_flag = true;
                /* err step 0 */
                err_flag |= 1 << 0;
            }
        } else if ((target_cnt - curr_cnt) < 600) {
            time_out_flag = true;
            /* err step 1 */
            err_flag |= 1 << 1;
        } else {
            time_out = (target_cnt - curr_cnt) - DSP_VP_POLLING_TIME;
        }
        if (!time_out_flag) {
            gDSP_VP_GPT_TARGET_TIME = target_cnt;
            if (gDSP_VP_GPT_TIMER_HANDLE == 0) {
                gpt_status = hal_gpt_sw_get_timer(&gDSP_VP_GPT_TIMER_HANDLE);
                if (HAL_GPT_STATUS_OK != gpt_status) {
                    /* err step 2 */
                    err_flag |= 1 << 2;
                }
            }
            //hal_gpio_set_output(HAL_GPIO_2, 0);
            gpt_status = hal_gpt_sw_start_timer_us(gDSP_VP_GPT_TIMER_HANDLE, time_out, CB_CM4_VP_PLAYBACK_GPT_CALLBACK, NULL);
            //hal_gpio_set_output(HAL_GPIO_2, 1);
            if (HAL_GPT_STATUS_OK != gpt_status) {
                /* err step 3 */
                err_flag |= 1 << 3;
            }
        } else {
            if (afe.dl2_enable) {
                // *((volatile uint32_t *)(0xC0000010)) |= 0x00000004;
                hal_audio_trigger_start_parameter_t sw_trigger_start;
                sw_trigger_start.enable = true;
                sw_trigger_start.memory_select = playback_vp_if.sink->param.audio.mem_handle.memory_select;
                hal_audio_set_value((hal_audio_set_value_parameter_t *)&sw_trigger_start, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
            } else {
                /* err step 3 */
                err_flag |= 1 << 4; // no vp path!
            }
        }
    }
    hal_nvic_restore_interrupt_mask_special(savedmask);
    if (time_out_flag) {
        xTaskResumeFromISR(playback_vp_if.source->taskId);
        portYIELD_FROM_ISR(pdTRUE); // force to do context switch
    }
    /* error handling */
    if (err_flag) {
        DSP_MW_LOG_E("[CM4_VP_PB] Start info request error flag 0x%x, gpt status %d", 2, err_flag, gpt_status);
    }
    DSP_MW_LOG_I("[CM4_VP_PB] Start info request(%d), c_cnt(%d), t_cnt(%d), time_out(%d)", 4, playback_vp_if.sink->param.audio.aws_sync_request, curr_cnt, target_cnt, time_out);
#if defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
    dsp_llf_set_audio_dl_status(LLF_DL_MIX_TYPE_VP, true);
#endif
}


void CB_CM4_VP_PLAYBACK_STOP(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    if (gDSP_VP_GPT_TIMER_HANDLE != 0) {
        hal_gpt_status_t gpt_status = hal_gpt_sw_stop_timer_us(gDSP_VP_GPT_TIMER_HANDLE);
        if (HAL_GPT_STATUS_OK != gpt_status) {
            DSP_MW_LOG_I("[VPC][SYNC]VP stop one_shot Timer error!!, error id = %d.\n", 1, gpt_status);
        }
        gpt_status = hal_gpt_sw_free_timer(gDSP_VP_GPT_TIMER_HANDLE);
        if (HAL_GPT_STATUS_OK != gpt_status) {
            DSP_MW_LOG_I("[VPC][SYNC]VP free2 one_shot Timer error!!, error id = %d.\n", 1, gpt_status);
        }
        gDSP_VP_GPT_TIMER_HANDLE = 0;
    }
    DSP_MW_LOG_I("[CM4_VP_PB] Stop", 0);
    if (playback_vp_if.transform != NULL) {
        StreamDSPClose(playback_vp_if.transform->source, playback_vp_if.transform->sink, msg.ccni_message[0] >> 16 | 0x8000);
    }
    playback_vp_if.transform = NULL;

}

void CB_CM4_VP_PLAYBACK_CONFIG(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    uint16_t vp_stop_immediately = hal_memview_cm4_to_dsp0(msg.ccni_message[0]);
    UNUSED(ack);
    /*
      Todo:
      Notify DSP framework to push remained pcm data into AFE.
    */

    /* Set flag to notify APP to stop VP */
    if (playback_vp_if.transform != NULL) {
#if 0 //VP log slim
        DSP_MW_LOG_I("[CM4_VP_PB] Config", 0);
#endif
        vp_config_flag = 1;
        if (vp_stop_immediately == 0x55) {
            // clear audio buffer
            DSP_CALLBACK_PTR callback_ptr = DSP_Callback_Get(playback_vp_if.source, playback_vp_if.sink);
            if (callback_ptr != NULL) {
                memset(callback_ptr->EntryPara.in_ptr[0], 0, callback_ptr->EntryPara.in_malloc_size);
                memset(callback_ptr->EntryPara.out_ptr[0], 0, callback_ptr->EntryPara.out_malloc_size);
            }
#if defined(AIR_BTA_IC_PREMIUM_G2)
            // Clear SRC IN
            memset((uint32_t *)(playback_vp_if.sink->param.audio.AfeBlkControl.phys_buffer_addr + playback_vp_if.sink->param.audio.AfeBlkControl.u4asrc_buffer_size), 0, AFE_INTERNAL_SRAM_VP_SIZE - playback_vp_if.sink->param.audio.AfeBlkControl.u4asrc_buffer_size);
            // Clear SRC OUT
            memset((uint32_t *)playback_vp_if.sink->param.audio.AfeBlkControl.phys_buffer_addr, 0, playback_vp_if.sink->param.audio.AfeBlkControl.u4asrc_buffer_size);
#endif
            DSP_MW_LOG_I("[CM4_VP_PB] config stop immediately", 0);
        }
    } else {
        DSP_MW_LOG_I("[CM4_VP_PB] Config duplicate.", 0);
    }
}

void CB_CM4_VP_PLAYBACK_CLOSE(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("[CM4_VP_PB] Close", 0);
    DSP_Callback_UnloaderConfig(playback_vp_if.pfeature_table, playback_vp_if.source->scenario_type);
    SourceClose(playback_vp_if.source);
    SinkClose(playback_vp_if.sink);
    memset(&playback_vp_if,0,sizeof(CONNECTION_IF));
#ifdef AIR_ANC_ADAPTIVE_CLOCK_CONTROL_ENABLE
    dsp_adapt_anc_send_unstable_status(AUDIO_ADAPTIVE_ANC_CLK_CTRL_TYPE_DL_VP, 0);
#endif
#if defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
    dsp_llf_set_audio_dl_status(LLF_DL_MIX_TYPE_VP, false);
#endif

}

void CB_CM4_VP_PLAYBACK_TRIGGER(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("[CM4_VP_PB] Trigger Just return", 0);
    return;
}
#endif

#ifdef MTK_PROMPT_SOUND_DUMMY_SOURCE_ENABLE
/* VP_DUMMY Playback CCNI callback function */
extern n9_dsp_share_info_t DUMMY_buff_info;
void CB_CM4_VP_DUMMY_SOURCE_PLAYBACK_OPEN(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("[CM4_VP_DUMMY] Open", 0);

    /* remap to non-cacheable address */
    mcu2dsp_open_param_p open_param;
    open_param = (mcu2dsp_open_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    //DSP_REMAP_SHARE_INFO(open_param->stream_in_param.playback.share_info_base_addr);
    open_param->stream_in_param.playback.share_info_base_addr = &DUMMY_buff_info;
    playback_vp_dummy_source_if.source = dsp_open_stream_in(open_param);
    playback_vp_dummy_source_if.sink   = dsp_open_stream_out(open_param);
    playback_vp_dummy_source_if.transform = NULL;
    DSP_Callback_PreloaderConfig(playback_vp_dummy_source_if.pfeature_table, open_param->audio_scenario_type);
}

void CB_CM4_VP_DUMMY_SOURCE_PLAYBACK_START(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("[CM4_VP_DUMMY] Start", 0);
    mcu2dsp_start_param_p start_param;
    start_param = (mcu2dsp_start_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);

    dsp_start_stream_in(start_param, playback_vp_dummy_source_if.source);
    dsp_start_stream_out(start_param, playback_vp_dummy_source_if.sink);
    //Rdebug- Needcheck
    //DSP_ConfigFeatureListCodecResolution(playback_vp_dummy_source_if.pfeature_table, Res_32bit, CONFIG_DECODER); /*Set Feature Codec 32 Resolution.*/

    playback_vp_dummy_source_if.transform = TrasformAudio2Audio(playback_vp_dummy_source_if.source, playback_vp_dummy_source_if.sink, stream_feature_list_prompt_dummy_source);
    if (playback_vp_dummy_source_if.transform == NULL) {
        DSP_MW_LOG_I("[CM4_VP_DUMMY] transform failed", 0);
    }
}


void CB_CM4_VP_DUMMY_SOURCE_PLAYBACK_STOP(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("[CM4_VP_DUMMY] Stop", 0);
    if (playback_vp_dummy_source_if.transform != NULL) {
        StreamDSPClose(playback_vp_dummy_source_if.transform->source, playback_vp_dummy_source_if.transform->sink, msg.ccni_message[0] >> 16 | 0x8000);
    }
    playback_vp_dummy_source_if.transform = NULL;
}

void CB_CM4_VP_DUMMY_SOURCE_PLAYBACK_CLOSE(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
    DSP_MW_LOG_I("[CM4_VP_DUMMY] Close", 0);
    DSP_Callback_UnloaderConfig(playback_vp_dummy_source_if.pfeature_table, playback_vp_dummy_source_if.source->scenario_type);
    SourceClose(playback_vp_dummy_source_if.source);
    SinkClose(playback_vp_dummy_source_if.sink);
    memset(&playback_vp_dummy_source_if,0,sizeof(CONNECTION_IF));
}

extern void cm4_vp_dummy_source_playback_set_param(uint8_t mode, uint8_t index);
void CB_CM4_VP_DUMMY_SOURCE_PLAYBACK_CHANGE_FEATURE(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    DSP_MW_LOG_I("[CM4_VP_DUMMY] Change feature", 0);
    uint8_t mode = (uint8_t)msg.ccni_message[1];
    uint8_t index = (uint8_t)(msg.ccni_message[1] >> 16);
    cm4_vp_dummy_source_playback_set_param(mode, index);
}
#endif

#ifdef AIR_BT_CLK_SKEW_ENABLE
void CB_N9_CLK_SKEW_LAG(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    UNUSED(msg);
    //reserve
}

void CB_N9_CLK_SKEW_LEAD(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    UNUSED(msg);
    //reserve
}
#endif

void CB_CM4_AUDIO_AMP_FORCE_CLOSE(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    UNUSED(msg);
    DSP_MW_LOG_I("[CM4_AUDIO] AMP Force Close", 0);
#ifdef ENABLE_AMP_TIMER
    fw_amp_force_close();
#endif
}

void dsp_set_audio_device_parameters(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    audio_driver_set_info_t set_type;
    uint32_t data;
    set_type = (audio_driver_set_info_t)(msg.ccni_message[0] & 0x7FFF);
    data = msg.ccni_message[1];
#ifdef AIR_KEEP_I2S_ENABLE
    bool enable = ((msg.ccni_message[0]>>15)&0x1);
#endif
    UNUSED(ack);
    if (0) {
#ifdef AIR_COMPONENT_CALIBRATION_ENABLE
    } else if (set_type == AUDIO_DRIVER_SET_GAIN_OFFSET) {
#ifdef AIR_AUDIO_HARDWARE_ENABLE
        hal_audio_set_gain_offset((hal_audio_calibration_t *)hal_memview_cm4_to_dsp0(data));
#endif
#endif
#ifdef AIR_3RD_PARTY_AUDIO_PLATFORM_ENABLE
    } else if (set_type == AUDIO_DRIVER_SET_3RD_PARTY_PLATFORM_SHARE_INFO) {
        audio_platform_share_buffer = (mcu_dsp_audio_platform_share_buffer_info_t *)hal_memview_cm4_to_dsp0(data);
#endif
#ifdef AIR_KEEP_I2S_ENABLE
    } else  if (set_type == AUDIO_DRIVER_SET_DEVICE) {
        mcu2dsp_open_param_p open_param;
        open_param = (mcu2dsp_open_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);

#if 0
        DSP_MW_LOG_I("[dsp_set_audio_device_parameters] device:0x%x, interface:0x%x, enable:%d", 3, open_param->stream_in_param.afe.audio_device, open_param->stream_in_param.afe.audio_interface, enable);
        hal_audio_device_parameter_t     device_handle_in;
        if (open_param->stream_in_param.afe.audio_device & HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_DUAL) {
            device_handle_in.analog_mic.adc_parameter.performance = open_param->stream_in_param.afe.performance;
            device_handle_in.analog_mic.bias_voltage[0] = open_param->stream_in_param.afe.bias_voltage[0];
            device_handle_in.analog_mic.bias_voltage[1] = open_param->stream_in_param.afe.bias_voltage[1];
            device_handle_in.analog_mic.bias_voltage[2] = open_param->stream_in_param.afe.bias_voltage[2];
            device_handle_in.analog_mic.bias_voltage[3] = open_param->stream_in_param.afe.bias_voltage[3];
            device_handle_in.analog_mic.bias_voltage[4] = open_param->stream_in_param.afe.bias_voltage[4];
            device_handle_in.analog_mic.bias_select = open_param->stream_in_param.afe.bias_select;
            device_handle_in.analog_mic.iir_filter = open_param->stream_in_param.afe.iir_filter[0];
            device_handle_in.analog_mic.adc_parameter.adc_mode = open_param->stream_in_param.afe.adc_mode;
            device_handle_in.analog_mic.adc_parameter.adc_type = open_param->stream_in_param.afe.adc_type;
            device_handle_in.analog_mic.mic_interface = open_param->stream_in_param.afe.audio_interface;
        } else if (open_param->stream_in_param.afe.audio_device & HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_DUAL) {
            device_handle_in.digital_mic.dmic_clock_rate = open_param->stream_in_param.afe.performance;
            device_handle_in.digital_mic.bias_voltage[0] = open_param->stream_in_param.afe.bias_voltage[0];
            device_handle_in.digital_mic.bias_voltage[1] = open_param->stream_in_param.afe.bias_voltage[1];
            device_handle_in.digital_mic.bias_voltage[2] = open_param->stream_in_param.afe.bias_voltage[2];
            device_handle_in.digital_mic.bias_voltage[3] = open_param->stream_in_param.afe.bias_voltage[3];
            device_handle_in.digital_mic.bias_voltage[4] = open_param->stream_in_param.afe.bias_voltage[4];
            device_handle_in.digital_mic.bias_select = open_param->stream_in_param.afe.bias_select;
            device_handle_in.digital_mic.iir_filter = open_param->stream_in_param.afe.iir_filter[0];
            device_handle_in.digital_mic.mic_interface = open_param->stream_in_param.afe.audio_interface;
            device_handle_in.digital_mic.dmic_selection = open_param->stream_in_param.afe.dmic_selection[0];
        } else if (open_param->stream_in_param.afe.audio_device & HAL_AUDIO_CONTROL_DEVICE_LINE_IN_DUAL) {
            device_handle_in.linein.adc_parameter.performance = open_param->stream_in_param.afe.performance;
            device_handle_in.linein.bias_voltage[0] = open_param->stream_in_param.afe.bias_voltage[0];
            device_handle_in.linein.bias_voltage[1] = open_param->stream_in_param.afe.bias_voltage[1];
            device_handle_in.linein.bias_voltage[2] = open_param->stream_in_param.afe.bias_voltage[2];
            device_handle_in.linein.bias_voltage[3] = open_param->stream_in_param.afe.bias_voltage[3];
            device_handle_in.linein.bias_voltage[4] = open_param->stream_in_param.afe.bias_voltage[4];
            device_handle_in.linein.bias_select = open_param->stream_in_param.afe.bias_select;
            device_handle_in.linein.iir_filter = open_param->stream_in_param.afe.iir_filter[0];
        }
        hal_audio_set_device(&device_handle_in, (hal_audio_control_t)open_param->stream_in_param.afe.audio_device, enable);
#else
        DSP_MW_LOG_I("[dsp_set_audio_device_parameters] device:0x%x, interface:0x%x, enable:%d", 3, open_param->stream_out_param.afe.audio_device, open_param->stream_out_param.afe.audio_interface, enable);
        hal_audio_device_parameter_t     device_handle_out;
        if (open_param->stream_out_param.afe.audio_device & HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER) {
            device_handle_out.i2s_master.i2s_interface = open_param->stream_out_param.afe.audio_interface;
            device_handle_out.i2s_master.rate = open_param->stream_out_param.afe.i2s_master_sampling_rate[0];
            device_handle_out.i2s_master.scenario_type  = open_param->audio_scenario_type;
            device_handle_out.i2s_master.is_tx = true;
            device_handle_out.i2s_master.i2s_format = HAL_AUDIO_I2S_I2S;
            device_handle_out.i2s_master.word_length = HAL_AUDIO_I2S_WORD_LENGTH_32BIT;
            device_handle_out.i2s_master.with_mclk = true;
            device_handle_out.i2s_master.is_low_jitter = false;
        }
        hal_audio_set_device(&device_handle_out, (hal_audio_control_t)open_param->stream_out_param.afe.audio_device, enable);
#endif
#endif
    }
    else {
        DSP_MW_LOG_I("DSP set audio device parameters, not support type:0x%x", 1, set_type);
    }
}


#ifdef MTK_SENSOR_SOURCE_ENABLE
#include "bsp_multi_axis_sensor.h"
extern bool bsp_multi_axis_read_register(uint32_t addr, uint8_t *data);
extern bool bsp_multi_axis_write_register(uint32_t addr, uint8_t *data);

void CB_GSENSOR_DETECT_READ_RG(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    uint8_t data = 0;

    ack = ack;

    if (bsp_multi_axis_read_register((uint32_t)msg.ccni_message[0] & 0xffff, &data) == false) {
        log_hal_msgid_error("[G-sensor] CB_CM4_GSENSOR_READ_RG fail", 0);
        return;
    }
    log_hal_msgid_info("[G-sensor]CB_CM4_GSENSOR_READ_RG,addr = 0x%08x,data = 0x%08x", 2, msg.ccni_message[0] & 0xffff, data);
}

void CB_GSENSOR_DETECT_WRITE_RG(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    uint8_t data = 0;

    ack = ack;

    data = (uint8_t)msg.ccni_message[1];
    if (bsp_multi_axis_write_register((uint32_t)msg.ccni_message[0] & 0xffff, &data) == false) {
        log_hal_msgid_error("[G-sensor] CB_CM4_GSENSOR_WRITE_RG fail", 0);
        return;
    }

    log_hal_msgid_info("[G-sensor]CB_CM4_GSENSOR_WRITE_RG,addr = 0x%08x,data = 0x%08x", 2, msg.ccni_message[0] & 0xffff, data);
}
#endif
#ifdef MTK_AUDIO_PLC_ENABLE
void DSP_AUDIO_PLC_CONTROL(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("DSP_AUDIO_PLC_CONTROL\r\n", 0);

    uint32_t enable = (uint32_t)(msg.ccni_message[1]);
    dsp_audio_plc_ctrl_t audio_plc_ctrl;
    audio_plc_ctrl.enable = enable;
    Audio_PLC_ctrl(audio_plc_ctrl);
    DSP_MW_LOG_I("DSP_AUDIO_PLC_CONTROL ENABLE %d\r\n", 1, enable);
}
#endif

#ifdef MTK_DSP_SHUTDOWN_SPECIAL_CONTROL_ENABLE
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void DSP_DUMMY_SHUTDOWN(void)
{
    bool ack = false;
    uint32_t savedmask;
    uint16_t count = 1;
    hal_nvic_save_and_set_interrupt_mask(&savedmask);
    //**Special control.
    //**To avoid DSP task HW semaphore when CM4 disable DSP power by using SPM control.
    hal_ccni_message_t msg;
    memset((void *)&msg, 0, sizeof(hal_ccni_message_t));
    msg.ccni_message[0] = 0x804b << 16;
    while (1) {
        count ++;
        if ((count % 1000) == 0) {
            if (!ack) {
                if (hal_ccni_set_event(AUDIO_CM4_TX_EVENT, &msg) == HAL_CCNI_STATUS_OK) { /*Remove all OS API, just call driver.*/
                    ack = true;
                }
            }
        }
    }
}
#endif

#ifdef AIR_AUDIO_TRANSMITTER_ENABLE

#ifdef AIR_MULTI_MIC_STREAM_ENABLE
extern stream_feature_list_t stream_featuremulti_mic_function_a[];
extern stream_feature_list_t stream_featuremulti_mic_function_b[];
extern stream_feature_list_t stream_featuremulti_mic_function_c[];
extern stream_feature_list_t stream_featuremulti_mic_function_f[];

CONNECTION_IF g_multi_mic_streams[AUDIO_TRANSMITTER_MULTI_MIC_STREAM_SUB_ID_MAX] = {
    /* source     sink            transform     pfeature_table */
    {NULL,      NULL,       NULL,        stream_featuremulti_mic_function_a},  //AUDIO_SCENARIO_MULTI_MIC_STREAM_FUNCTION_A
    {NULL,      NULL,       NULL,        stream_featuremulti_mic_function_b},  //AUDIO_SCENARIO_MULTI_MIC_STREAM_FUNCTION_B
    {NULL,      NULL,       NULL,        stream_featuremulti_mic_function_c},  //AUDIO_SCENARIO_MULTI_MIC_STREAM_FUNCTION_C
    {NULL,      NULL,       NULL,        stream_featuremulti_mic_function_f},  //AUDIO_SCENARIO_MULTI_MIC_STREAM_FUNCTION_F
};
#endif

#ifdef MTK_SENSOR_SOURCE_ENABLE
CONNECTION_IF g_sensor_streams = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       AudioFeatureList_GSensorMotionDetect
};
#endif

#ifdef MTK_AUDIO_LOOPBACK_TEST_ENABLE
extern stream_feature_list_t stream_feature_list_audio_loopback_test[];
CONNECTION_IF audio_loopback_test_if = {NULL,      NULL,       NULL,     stream_feature_list_audio_loopback_test};
#endif

#if defined(MTK_GAMING_MODE_HEADSET) || defined(AIR_GAMING_MODE_DONGLE_ENABLE)
extern stream_feature_list_t stream_feature_list_usb_in_broadcast_0[];
CONNECTION_IF g_usb_in_broadcast_streams_0 = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_usb_in_broadcast_0
};

extern stream_feature_list_t stream_feature_list_usb_in_broadcast_1[];
CONNECTION_IF g_usb_in_broadcast_streams_1 = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_usb_in_broadcast_1
};

extern stream_feature_list_t stream_feature_list_usb_out_broadcast[];
CONNECTION_IF g_usb_out_broadcast_streams = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_usb_out_broadcast
};

#ifdef AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE
extern stream_feature_list_t stream_feature_list_game_line_in_broadcast[];
CONNECTION_IF g_gaming_line_in_broadcast_streams = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_game_line_in_broadcast
};
#endif /* AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE */

#ifdef AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE
extern stream_feature_list_t stream_feature_list_game_line_out_broadcast[];
CONNECTION_IF g_gaming_line_out_broadcast_streams = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_game_line_out_broadcast
};
#endif /* AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE */

#ifdef AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE
extern stream_feature_list_t stream_feature_list_game_i2s_in_broadcast[];
CONNECTION_IF g_gaming_i2s_in_broadcast_streams = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_game_i2s_in_broadcast
};
#endif /* AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE */

#endif /* MTK_GAMING_MODE_HEADSET || AIR_GAMING_MODE_DONGLE_ENABLE */

#if defined(MTK_GAMING_MODE_HEADSET) || defined(AIR_GAMING_MODE_DONGLE_ENABLE)
extern stream_feature_list_t stream_feature_list_game_headset_ul[];
CONNECTION_IF g_game_headset_ul_streams = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_game_headset_ul
};
#endif /* MTK_GAMING_MODE_HEADSET || AIR_GAMING_MODE_DONGLE_ENABLE */

#ifdef AIR_AUDIO_I2S_SLAVE_TDM_ENABLE
extern stream_feature_list_t stream_feature_list_tdm[];
CONNECTION_IF tdm_if = {NULL,      NULL,       NULL,     stream_feature_list_tdm};
#endif

#if defined(AIR_WIRED_AUDIO_ENABLE)
extern stream_feature_list_t stream_feature_list_wired_audio_usb_in_0_seperate_peq[];
CONNECTION_IF g_usb_in_local_streams_0 = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_wired_audio_usb_in_0_seperate_peq
};
extern stream_feature_list_t stream_feature_list_wired_audio_usb_in_1_seperate_peq[];
CONNECTION_IF g_usb_in_local_streams_1 = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_wired_audio_usb_in_1_seperate_peq
};
extern stream_feature_list_t stream_feature_list_wired_audio_line_in[];
CONNECTION_IF g_line_in_local_streams = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_wired_audio_line_in
};
extern stream_feature_list_t stream_feature_list_wired_audio_usb_out[];
CONNECTION_IF g_usb_out_local_streams = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_wired_audio_usb_out
};
extern stream_feature_list_t stream_feature_list_wired_audio_line_out[];
CONNECTION_IF g_line_out_local_streams = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_wired_audio_line_out
};
extern stream_feature_list_t stream_feature_list_wired_audio_usb_out_iem[];
CONNECTION_IF g_usb_out_local_streams_iem = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_wired_audio_usb_out_iem
};
extern stream_feature_list_t stream_feature_list_wired_audio_usb_in_out_iem[];
CONNECTION_IF g_usb_in_out_local_streams_iem = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_wired_audio_usb_in_out_iem
};
extern stream_feature_list_t stream_feature_list_wired_audio_main_stream[];
CONNECTION_IF g_wired_audio_main_stream = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_wired_audio_main_stream
};
#endif /*AIR_WIRED_AUDIO_ENABLE*/

#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE)
extern stream_feature_list_t stream_feature_list_advanced_passthrough[];
CONNECTION_IF advanced_passthrough_if = {NULL,      NULL,       NULL,     stream_feature_list_advanced_passthrough};
#endif /* AIR_ADVANCED_PASSTHROUGH_ENABLE */

#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
extern stream_feature_list_t stream_feature_list_ble_audio_dongle_usb_in_broadcast_0[];
CONNECTION_IF g_ble_audio_dongle_usb_in_broadcast_streams_0 = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_ble_audio_dongle_usb_in_broadcast_0
};

extern stream_feature_list_t stream_feature_list_ble_audio_dongle_usb_in_broadcast_1[];
CONNECTION_IF g_ble_audio_dongle_usb_in_broadcast_streams_1 = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_ble_audio_dongle_usb_in_broadcast_1
};

#ifdef AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
extern stream_feature_list_t stream_feature_list_ble_audio_dongle_line_in_broadcast[];
CONNECTION_IF g_ble_audio_dongle_line_in_broadcast_streams = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_ble_audio_dongle_line_in_broadcast
};
#endif /* AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE */

#ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
extern stream_feature_list_t stream_feature_list_ble_audio_dongle_i2s_in_broadcast[];
CONNECTION_IF g_ble_audio_dongle_i2s_in_broadcast_streams = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_ble_audio_dongle_i2s_in_broadcast
};
#endif /* AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE */

extern stream_feature_list_t stream_feature_list_ble_audio_dongle_usb_out_broadcast[];
CONNECTION_IF g_ble_audio_dongle_usb_out_broadcast_streams = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_ble_audio_dongle_usb_out_broadcast
};
#endif /* AIR_BLE_AUDIO_DONGLE_ENABLE */

#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_usb_in_broadcast_0[];
CONNECTION_IF g_ull_audio_v2_dongle_usb_in_broadcast_streams_0 = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_ull_audio_v2_dongle_usb_in_broadcast_0
};

extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_usb_in_broadcast_1[];
CONNECTION_IF g_ull_audio_v2_dongle_usb_in_broadcast_streams_1 = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_ull_audio_v2_dongle_usb_in_broadcast_1
};

extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_usb_out_broadcast[];
CONNECTION_IF g_ull_audio_v2_dongle_usb_out_broadcast_streams = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_ull_audio_v2_dongle_usb_out_broadcast
};

#ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_line_in_broadcast[];
CONNECTION_IF g_ull_audio_v2_dongle_line_in_broadcast_streams = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_ull_audio_v2_dongle_line_in_broadcast
};
#endif /* AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE */

#if defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE
extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_i2s_in_broadcast[];
CONNECTION_IF g_ull_audio_v2_dongle_i2s_in_broadcast_streams = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_ull_audio_v2_dongle_i2s_in_broadcast
};
#endif /* AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE || AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE */

#if defined(AIR_DONGLE_LINE_OUT_ENABLE)
extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_line_out[];
CONNECTION_IF g_ull_audio_v2_dongle_line_out_streams_0 = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_ull_audio_v2_dongle_line_out
};
#endif /* defined(AIR_DONGLE_LINE_OUT_ENABLE) */

#if defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_i2s_mst_out[];
CONNECTION_IF g_ull_audio_v2_dongle_i2s_mst_out_streams_0 = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_ull_audio_v2_dongle_i2s_mst_out
};
#endif /* defined(AIR_DONGLE_I2S_MST_OUT_ENABLE) */

#if defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE)
extern stream_feature_list_t stream_feature_list_ull_audio_v2_dongle_i2s_slv_out[];
CONNECTION_IF g_ull_audio_v2_dongle_i2s_slv_out_streams_0 = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_ull_audio_v2_dongle_i2s_slv_out
};
#endif /* defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) */

#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE */

#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
extern stream_feature_list_t stream_feature_list_wireless_mic_rx_usb_out[];
CONNECTION_IF g_wireless_mic_rx_usb_out_streams_0 = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_wireless_mic_rx_usb_out
};

extern stream_feature_list_t stream_feature_list_wireless_mic_rx_line_out[];
CONNECTION_IF g_wireless_mic_rx_line_out_streams_0 = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_wireless_mic_rx_line_out
};

extern stream_feature_list_t stream_feature_list_wireless_mic_rx_i2s_slv_out[];
CONNECTION_IF g_wireless_mic_rx_i2s_slv_out_streams_0 = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_wireless_mic_rx_i2s_slv_out
};
#endif /* AIR_WIRELESS_MIC_RX_ENABLE */

#ifdef AIR_BT_AUDIO_DONGLE_ENABLE
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
CONNECTION_IF g_bt_audio_dongle_usb_in_broadcast_streams_0 = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_bt_audio_dongle_usb_in_a2dp
};

CONNECTION_IF g_bt_audio_dongle_usb_in_broadcast_streams_1 = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_bt_audio_dongle_usb_in_a2dp
};

CONNECTION_IF g_bt_audio_dongle_usb_out_broadcast_streams_0 = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_bt_audio_dongle_usb_out_hfp_msbc
};

CONNECTION_IF g_bt_audio_dongle_usb_out_broadcast_streams_1 = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_bt_audio_dongle_usb_out_hfp_msbc
};
#endif /* AIR_BT_AUDIO_DONGLE_USB_ENABLE */

#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
EXTERN stream_feature_list_t stream_feature_list_bt_audio_dongle_afe_in_a2dp_0[];
EXTERN stream_feature_list_t stream_feature_list_bt_audio_dongle_afe_in_a2dp_1[];
EXTERN stream_feature_list_t stream_feature_list_bt_audio_dongle_afe_in_a2dp_2[];
EXTERN stream_feature_list_t stream_feature_list_bt_audio_dongle_afe_in_hfp_msbc_0[];
EXTERN stream_feature_list_t stream_feature_list_bt_audio_dongle_afe_in_hfp_msbc_1[];
EXTERN stream_feature_list_t stream_feature_list_bt_audio_dongle_afe_in_hfp_msbc_2[];
EXTERN stream_feature_list_t stream_feature_list_bt_audio_dongle_afe_in_hfp_cvsd_0[];
EXTERN stream_feature_list_t stream_feature_list_bt_audio_dongle_afe_in_hfp_cvsd_1[];
EXTERN stream_feature_list_t stream_feature_list_bt_audio_dongle_afe_in_hfp_cvsd_2[];
CONNECTION_IF g_bt_audio_dongle_afe_in_broadcast_streams_0 = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_bt_audio_dongle_afe_in_a2dp_0
};

CONNECTION_IF g_bt_audio_dongle_afe_in_broadcast_streams_1 = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_bt_audio_dongle_afe_in_a2dp_1
};

CONNECTION_IF g_bt_audio_dongle_afe_in_broadcast_streams_2 = {
    /* source     sink            transform     pfeature_table */
    NULL,        NULL,       NULL,       stream_feature_list_bt_audio_dongle_afe_in_a2dp_2
};
#endif /* afe in */

#endif /* AIR_BT_AUDIO_DONGLE_ENABLE */

#if defined(AIR_RECORD_ADVANCED_ENABLE)
extern stream_feature_list_t stream_feature_list_advanced_record_n_mic[];
CONNECTION_IF advanced_record_n_mic_if = {NULL,      NULL,       NULL,     stream_feature_list_advanced_record_n_mic};
#endif

#ifdef AIR_AUDIO_HW_LOOPBACK_ENABLE
afe_loopback_param_t dsp_audio_hw_loopback_spk;
afe_loopback_param_t dsp_audio_hw_loopack_mic;
afe_loopback_param_t dsp_audio_hw_loopack_linein;
void audio_hw_loopback_open(hal_ccni_message_t msg,audio_transmitter_scenario_sub_id_t sub_id)
{
    afe_loopback_param_t *dsp_audio_hw_loopback = NULL;
    mcu2dsp_open_param_p open_param;
    open_param = (mcu2dsp_open_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    if(sub_id.audio_hw_loopback_id == AUDIO_TRANSMITTER_AUDIO_HW_LOOPBACK_I2S0_TO_DAC){
        dsp_audio_hw_loopback = &dsp_audio_hw_loopback_spk;
    }
    else if(sub_id.audio_hw_loopback_id == AUDIO_TRANSMITTER_AUDIO_HW_LOOPBACK_ADC_TO_I2S0){
        dsp_audio_hw_loopback = &dsp_audio_hw_loopack_mic;
    }
    else if(sub_id.audio_hw_loopback_id == AUDIO_TRANSMITTER_AUDIO_HW_LOOPBACK_LINEIN_TO_I2S2){
        dsp_audio_hw_loopback = &dsp_audio_hw_loopack_linein;
    }
    else{
        DSP_MW_LOG_E("[audio_transmitter] audio hw loopback open fail,scenario id is not found:%d", 1, sub_id.audio_hw_loopback_id);
    }
        dsp_audio_hw_loopback->in_device = (hal_audio_device_t)open_param->stream_in_param.afe.audio_device;
        dsp_audio_hw_loopback->in_interface = open_param->stream_in_param.afe.audio_interface;
        dsp_audio_hw_loopback->in_misc_parms.I2sClkSourceType = open_param->stream_in_param.afe.misc_parms;
        dsp_audio_hw_loopback->out_device = (hal_audio_device_t)open_param->stream_out_param.afe.audio_device;
        dsp_audio_hw_loopback->out_interface = open_param->stream_out_param.afe.audio_interface;
        dsp_audio_hw_loopback->out_misc_parms.I2sClkSourceType = open_param->stream_out_param.afe.misc_parms;
        dsp_audio_hw_loopback->sample_rate = open_param->stream_out_param.afe.sampling_rate;
        dsp_audio_hw_loopback->with_hw_gain = open_param->stream_out_param.afe.hw_gain;
        dsp_audio_hw_loopback->stream_channel = open_param->stream_out_param.afe.stream_channel;
        dsp_audio_hw_loopback->format = open_param->stream_out_param.afe.format;
        if(dsp_audio_hw_loopback->in_device & HAL_AUDIO_CONTROL_DEVICE_I2S_MASTER){
            dsp_audio_hw_loopback->device_handle_in.i2s_master.i2s_interface = (hal_audio_interface_t)open_param->stream_in_param.afe.audio_interface;
        }else if(dsp_audio_hw_loopback->in_device & (HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_DUAL|HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_L|HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_R| \
        HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_DUAL|HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_L|HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_R)){
            dsp_audio_hw_loopback->device_handle_in.digital_mic.bias_voltage[0] = open_param->stream_in_param.afe.bias_voltage[0];
            dsp_audio_hw_loopback->device_handle_in.digital_mic.bias_voltage[1] = open_param->stream_in_param.afe.bias_voltage[1];
            dsp_audio_hw_loopback->device_handle_in.digital_mic.bias_voltage[2] = open_param->stream_in_param.afe.bias_voltage[2];
            dsp_audio_hw_loopback->device_handle_in.digital_mic.bias_voltage[3] = open_param->stream_in_param.afe.bias_voltage[3];
            dsp_audio_hw_loopback->device_handle_in.digital_mic.bias_voltage[4] = open_param->stream_in_param.afe.bias_voltage[4];
            dsp_audio_hw_loopback->in_misc_parms.MicbiasSourceType = open_param->stream_in_param.afe.misc_parms;
            dsp_audio_hw_loopback->device_handle_in.analog_mic.mic_interface = (hal_audio_interface_t)open_param->stream_in_param.afe.audio_interface;
            dsp_audio_hw_loopback->device_handle_in.analog_mic.adc_parameter.performance = open_param->stream_in_param.afe.performance;
            dsp_audio_hw_loopback->device_handle_in.analog_mic.bias_select = open_param->stream_in_param.afe.bias_select;
            dsp_audio_hw_loopback->device_handle_in.analog_mic.iir_filter = open_param->stream_in_param.afe.iir_filter[0];
            dsp_audio_hw_loopback->device_handle_in.analog_mic.adc_parameter.adc_mode = open_param->stream_in_param.afe.adc_mode;
        }else if(dsp_audio_hw_loopback->in_device & HAL_AUDIO_CONTROL_DEVICE_LINE_IN_DUAL){
            dsp_audio_hw_loopback->device_handle_in.linein.adc_parameter.performance = open_param->stream_in_param.afe.performance;
            dsp_audio_hw_loopback->device_handle_in.linein.bias_voltage[0] = open_param->stream_in_param.afe.bias_voltage[0];
            dsp_audio_hw_loopback->device_handle_in.linein.bias_voltage[1] = open_param->stream_in_param.afe.bias_voltage[1];
            dsp_audio_hw_loopback->device_handle_in.linein.bias_voltage[2] = open_param->stream_in_param.afe.bias_voltage[2];
            dsp_audio_hw_loopback->device_handle_in.linein.bias_voltage[3] = open_param->stream_in_param.afe.bias_voltage[3];
            dsp_audio_hw_loopback->device_handle_in.linein.bias_voltage[4] = open_param->stream_in_param.afe.bias_voltage[4];
            dsp_audio_hw_loopback->device_handle_in.linein.bias_select = open_param->stream_in_param.afe.bias_select;
            dsp_audio_hw_loopback->device_handle_in.linein.iir_filter = open_param->stream_in_param.afe.iir_filter[0];
            dsp_audio_hw_loopback->device_handle_in.linein.adc_parameter.adc_mode = open_param->stream_in_param.afe.adc_mode;
        }
        #if defined(AIR_BTA_IC_PREMIUM_G2)
        dsp_audio_hw_loopback->device_handle_out.dac.with_high_performance = open_param->stream_out_param.afe.performance;
        #else
        dsp_audio_hw_loopback->device_handle_out.dac.performance = open_param->stream_out_param.afe.performance;
        #endif
        DSP_MW_LOG_I("dsp_audio_hw_loopback in_device %d in_interface %d in_misc_parms %d out_device %d out_interface %d out_misc_parms %d sample_rate %d with_hw_gain %d stream_channel %d format %d adc_mode %d",11,
        dsp_audio_hw_loopback->in_device,dsp_audio_hw_loopback->in_interface,dsp_audio_hw_loopback->in_misc_parms,dsp_audio_hw_loopback->out_device,dsp_audio_hw_loopback->out_interface,
        dsp_audio_hw_loopback->out_misc_parms,dsp_audio_hw_loopback->sample_rate,dsp_audio_hw_loopback->with_hw_gain,dsp_audio_hw_loopback->stream_channel,dsp_audio_hw_loopback->format,
        dsp_audio_hw_loopback->device_handle_in.analog_mic.adc_parameter.adc_mode);
        afe_set_loopback_enable(HAL_AUDIO_CONTROL_ON, dsp_audio_hw_loopback);
        if(sub_id.audio_hw_loopback_id == AUDIO_TRANSMITTER_AUDIO_HW_LOOPBACK_I2S0_TO_DAC){
            audio_hw_loopback_echo_enable(HAL_AUDIO_CONTROL_ON);
        }
}

void audio_hw_loopback_close(hal_ccni_message_t msg,audio_transmitter_scenario_sub_id_t sub_id)
{
    UNUSED(msg);
    if(sub_id.audio_hw_loopback_id == AUDIO_TRANSMITTER_AUDIO_HW_LOOPBACK_I2S0_TO_DAC){
        afe_set_loopback_enable(HAL_AUDIO_CONTROL_OFF, &dsp_audio_hw_loopback_spk);
        audio_hw_loopback_echo_enable(HAL_AUDIO_CONTROL_OFF);
    }
    else if(sub_id.audio_hw_loopback_id == AUDIO_TRANSMITTER_AUDIO_HW_LOOPBACK_LINEIN_TO_I2S2){
        afe_set_loopback_enable(HAL_AUDIO_CONTROL_OFF, &dsp_audio_hw_loopack_linein);
    }
    else if(sub_id.audio_hw_loopback_id == AUDIO_TRANSMITTER_AUDIO_HW_LOOPBACK_ADC_TO_I2S0){
        afe_set_loopback_enable(HAL_AUDIO_CONTROL_OFF, &dsp_audio_hw_loopack_mic);
    }
    else{
        DSP_MW_LOG_E("[audio_transmitter] audio hw loopback close fail,scenario id is not found:%d", 1, sub_id.audio_hw_loopback_id);
    }
}
#endif

CONNECTION_IF *port_audio_transmitter_get_connection_if(audio_transmitter_scenario_type_t scenario_id, audio_transmitter_scenario_sub_id_t sub_id)
{
    CONNECTION_IF *application_ptr = NULL;

    if (scenario_id == AUDIO_TRANSMITTER_A2DP_SOURCE) {


    }
#ifdef AIR_MULTI_MIC_STREAM_ENABLE
    else if (scenario_id == AUDIO_TRANSMITTER_MULTI_MIC_STREAM) {
        application_ptr = &g_multi_mic_streams[sub_id.multimic_id];
    }
#endif
#ifdef MTK_SENSOR_SOURCE_ENABLE
    else if (scenario_id == AUDIO_TRANSMITTER_GSENSOR) {
        application_ptr = &sensor_src_if[sub_id.gsensor_id];
    }
#endif
#if defined(MTK_GAMING_MODE_HEADSET) || defined(AIR_GAMING_MODE_DONGLE_ENABLE)
    else if (scenario_id == AUDIO_TRANSMITTER_GAMING_MODE) {
        switch (sub_id.gamingmode_id) {
            case AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0:
                application_ptr = &g_usb_in_broadcast_streams_0;
                break;
            case AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_1:
                application_ptr = &g_usb_in_broadcast_streams_1;
                break;
            case AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_USB_OUT:
                application_ptr = &g_usb_out_broadcast_streams;
                break;
            case AUDIO_TRANSMITTER_GAMING_MODE_VOICE_HEADSET:
                application_ptr = &g_game_headset_ul_streams;
                break;
            #ifdef AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE
                case AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_LINE_IN:
                    application_ptr = &g_gaming_line_in_broadcast_streams;
                    break;
            #endif /* AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE */
            #ifdef AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE
                case AUDIO_TRANSMITTER_GAMING_MODE_VOICE_DONGLE_LINE_OUT:
                    application_ptr = &g_gaming_line_out_broadcast_streams;
                    break;
            #endif /* AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE */
            #ifdef AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE
                case AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_I2S_IN:
                    application_ptr = &g_gaming_i2s_in_broadcast_streams;
                    break;
            #endif
            default:
                DSP_MW_LOG_W("[audio_transmitter] get connection_if not found app_ptr %d-%d", 2, scenario_id, sub_id.gamingmode_id);
                break;
        }
    }
#endif /* MTK_GAMING_MODE_HEADSET || AIR_GAMING_MODE_DONGLE_ENABLE */
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
    else if (scenario_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE) {

        if (sub_id.ble_audio_dongle_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0) {
            application_ptr = &g_ble_audio_dongle_usb_in_broadcast_streams_0;
        } else if (sub_id.ble_audio_dongle_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1) {
            application_ptr = &g_ble_audio_dongle_usb_in_broadcast_streams_1;
        } else if (sub_id.ble_audio_dongle_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_VOICE_USB_OUT) {
            application_ptr = &g_ble_audio_dongle_usb_out_broadcast_streams;
        }
        #ifdef AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
            else if (sub_id.ble_audio_dongle_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_LINE_IN) {
                application_ptr = &g_ble_audio_dongle_line_in_broadcast_streams;
            }
        #endif /* AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE */
        #ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
            else if (sub_id.ble_audio_dongle_id == AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_I2S_IN) {
                application_ptr = &g_ble_audio_dongle_i2s_in_broadcast_streams;
            }
        #endif /* AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE */
    }
#endif /* AIR_BLE_AUDIO_DONGLE_ENABLE */
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
    else if (scenario_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE) {
        if(sub_id.ull_audio_v2_dongle_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0)
        {
            application_ptr = &g_ull_audio_v2_dongle_usb_in_broadcast_streams_0;
        }
        else if(sub_id.ull_audio_v2_dongle_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1)
        {
            application_ptr = &g_ull_audio_v2_dongle_usb_in_broadcast_streams_1;
        }
        else if(sub_id.ull_audio_v2_dongle_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_USB_OUT_0)
        {
            application_ptr = &g_ull_audio_v2_dongle_usb_out_broadcast_streams;
        }
#ifdef AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
        else if (sub_id.ull_audio_v2_dongle_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_LINE_IN) {
            application_ptr = &g_ull_audio_v2_dongle_line_in_broadcast_streams;
        }
#endif /* AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE */
#if defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE
        else if ((sub_id.ull_audio_v2_dongle_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_MST_IN_0) ||
            (sub_id.ull_audio_v2_dongle_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_DL_I2S_SLV_IN_0)) {
            application_ptr = &g_ull_audio_v2_dongle_i2s_in_broadcast_streams;
        }
#endif /* AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE || AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE */
#if defined(AIR_DONGLE_LINE_OUT_ENABLE)
        else if (sub_id.ull_audio_v2_dongle_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_LINE_OUT) {
            application_ptr = &g_ull_audio_v2_dongle_line_out_streams_0;
        }
#endif /* defined(AIR_DONGLE_LINE_OUT_ENABLE) */
#if defined(AIR_DONGLE_I2S_MST_OUT_ENABLE)
        else if (sub_id.ull_audio_v2_dongle_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_MST_OUT_0) {
            application_ptr = &g_ull_audio_v2_dongle_i2s_mst_out_streams_0;
        }
#endif /* defined(AIR_DONGLE_I2S_MST_OUT_ENABLE) */
#if defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE)
        else if (sub_id.ull_audio_v2_dongle_id == AUDIO_TRANSMITTER_ULL_AUDIO_V2_DONGLE_UL_I2S_SLV_OUT_0) {
            application_ptr = &g_ull_audio_v2_dongle_i2s_slv_out_streams_0;
        }
#endif /* defined(AIR_DONGLE_I2S_SLV_OUT_ENABLE) */
    }
#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE */
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
    else if (scenario_id == AUDIO_TRANSMITTER_WIRELESS_MIC_RX) {
        if(sub_id.wireless_mic_rx_id == AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_USB_OUT_0)
        {
            application_ptr = &g_wireless_mic_rx_usb_out_streams_0;
        }
        else if(sub_id.wireless_mic_rx_id == AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_LINE_OUT)
        {
            application_ptr = &g_wireless_mic_rx_line_out_streams_0;
        }
        else if(sub_id.wireless_mic_rx_id == AUDIO_TRANSMITTER_WIRELESS_MIC_RX_UL_I2S_SLV_OUT_0)
        {
            application_ptr = &g_wireless_mic_rx_i2s_slv_out_streams_0;
        }
    }
#endif /* AIR_WIRELESS_MIC_RX_ENABLE */
#if defined(MTK_ANC_SURROUND_MONITOR_ENABLE)
    else if (scenario_id == AUDIO_TRANSMITTER_ANC_MONITOR_STREAM) {
        application_ptr = &sensor_anc_monitor_if;
    }
#endif
    else if (scenario_id == AUDIO_TRANSMITTER_TEST) {
        if (sub_id.test_id == AUDIO_TRANSMITTER_TEST_AUDIO_LOOPBACK) {
#if defined(MTK_AUDIO_LOOPBACK_TEST_ENABLE)
            application_ptr = &audio_loopback_test_if;
            stream_feature_configure_resolution((stream_feature_list_ptr_t)application_ptr->pfeature_table, RESOLUTION_32BIT, CONFIG_DECODER);
#endif
        }
    }
#if defined(AIR_AUDIO_I2S_SLAVE_TDM_ENABLE)
    else if (scenario_id == AUDIO_TRANSMITTER_TDM) {
        application_ptr = &tdm_if;
        stream_feature_configure_resolution((stream_feature_list_ptr_t)application_ptr->pfeature_table, RESOLUTION_32BIT, CONFIG_DECODER);

        if ((tdm_if.source != 0) && (tdm_if.sink != 0)) {
            tdm_if.source->param.audio.linein_scenario_flag = 1;
            tdm_if.sink->param.audio.linein_scenario_flag = 1;
            Source_Audio_BufferInfo_Rst(tdm_if.source, 0);
#ifndef AIR_AUDIO_I2S_SLAVE_TDM_VIRTUAL_OUT_ENABLE
            if (!tdm_if.sink->param.virtual_para.user_count) {
                Sink_Audio_BufferInfo_Rst(tdm_if.sink, 0);
            }
#endif
        }
    }
#endif
#if defined(AIR_WIRED_AUDIO_ENABLE)
    else if (scenario_id == AUDIO_TRANSMITTER_WIRED_AUDIO) {
        //if(sub_id.test_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT){
        //    DSP_MW_LOG_E("[audio_transmitter] get connection_if AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT", 0);
        //    DSP_ALG_UpdateEscoRxMode(VOICE_WB);
        //    application_ptr = &g_usb_out_local_streams;
        //}
        //else
        if ((sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) || (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1)) {
            if (((sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) && (g_application_ptr_usb_in_0 == NULL))
                || ((sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1) && (g_application_ptr_usb_in_1 == NULL))) {
                if (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) {
                    application_ptr = &g_usb_in_local_streams_0;
                    g_application_ptr_usb_in_0 = application_ptr;
                } else if (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1) {
                    application_ptr = &g_usb_in_local_streams_1;
                    g_application_ptr_usb_in_1 = application_ptr;
                }
            } else {
                if (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) {
                    application_ptr = g_application_ptr_usb_in_0;
                } else if (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1) {
                    application_ptr = g_application_ptr_usb_in_1;
                }
            }
        } else if ((sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT) || (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT) || (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER) || (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE)) {
            DSP_MW_LOG_I("[audio_transmitter] get connection_if AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT/AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT/AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER/AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE", 0);
            if (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT) {
                application_ptr = &g_usb_out_local_streams;
            } else {
                application_ptr = &g_line_out_local_streams;
            }
        } else if ((sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN) || (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER)) {
            DSP_MW_LOG_I("[audio_transmitter] get connection_if AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN/AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER", 0);
            application_ptr = &g_line_in_local_streams;
        } else if (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_OUT_IEM){
            application_ptr = &g_usb_in_out_local_streams_iem;
        } else if (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_MAINSTREAM){
            application_ptr = &g_wired_audio_main_stream;
        }
    }
#endif /*AIR_WIRED_AUDIO_ENABLE*/
#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE)
    else if (scenario_id == AUDIO_TRANSMITTER_ADVANCED_PASSTHROUGH) {
        if (sub_id.advanced_passthrough_id == AUDIO_TRANSMITTER_ADVANCED_PASSTHROUGH_HEARING_AID) {
            application_ptr = &advanced_passthrough_if;
        }
    }
#endif /* AIR_ADVANCED_PASSTHROUGH_ENABLE */
#if defined(AIR_ADAPTIVE_EQ_ENABLE)
    else if (scenario_id == AUDIO_TRANSMITTER_ADAPTIVE_EQ_MONITOR_STREAM){
        application_ptr = &sensor_adaptive_eq_monitor_if;
    }
#endif
#if defined (AIR_DCHS_MODE_ENABLE)
    else if (scenario_id == AUDIO_TRANSMITTER_DCHS){
        if(sub_id.dchs_id == AUDIO_TRANSMITTER_DCHS_UART_UL){
            if(dchs_get_device_mode() == DCHS_MODE_RIGHT){
                application_ptr = &dchs_uart_ul_right_if;
            }else{
                if(dchs_voice_mode == VOICE_WB){
                    if((dchs_ul_scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT) || (dchs_ul_scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_OUT)){
                        application_ptr = &dchs_uart_ul_left_if_wb_sw_gain;
                    }else{
                        application_ptr = &dchs_uart_ul_left_if_wb;
                    }
                }else{
                    if((dchs_ul_scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT) || (dchs_ul_scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_OUT)){
                        application_ptr = &dchs_uart_ul_left_if_swb_sw_gain;
                    }else{
                        application_ptr = &dchs_uart_ul_left_if_swb;
                    }
                }
            }
        }
    }
#endif
#if defined(AIR_RECORD_ADVANCED_ENABLE)
    else if (scenario_id == AUDIO_TRANSMITTER_ADVANCED_RECORD){
        if(sub_id.advanced_record_id == AUDIO_TRANSMITTER_ADVANCED_RECORD_N_MIC){
            application_ptr = &advanced_record_n_mic_if;
        }
    }
#endif
#ifdef AIR_BT_AUDIO_DONGLE_ENABLE
    else if (scenario_id == AUDIO_TRANSMITTER_BT_AUDIO_DONGLE) {
        switch (sub_id.bt_audio_dongle_id) {
#ifdef AIR_BT_AUDIO_DONGLE_USB_ENABLE
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
                application_ptr = &g_bt_audio_dongle_usb_in_broadcast_streams_0;
                break;
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
                application_ptr = &g_bt_audio_dongle_usb_in_broadcast_streams_1;
                break;
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_0:
                application_ptr = &g_bt_audio_dongle_usb_out_broadcast_streams_0;
                break;
            case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_UL_HFP_USB_OUT_1:
                application_ptr = &g_bt_audio_dongle_usb_out_broadcast_streams_1;
                break;
#endif /* AIR_BT_AUDIO_DONGLE_USB_ENABLE */
#if defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
                case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
                case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
                    application_ptr = &g_bt_audio_dongle_afe_in_broadcast_streams_0;
                    break;
                case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
                case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
                    application_ptr = &g_bt_audio_dongle_afe_in_broadcast_streams_1;
                    break;
                case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
                case AUDIO_TRANSMITTER_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
                    application_ptr = &g_bt_audio_dongle_afe_in_broadcast_streams_2;
                    break;
#endif /* afe in */
            default:
                break;
        }
    }
#endif /* AIR_BT_AUDIO_DONGLE_ENABLE */

    if (!application_ptr) {
        DSP_MW_LOG_E("[audio_transmitter] get connection_if failed, scenario_id %d, sub_id %d", 2, scenario_id, sub_id);
        AUDIO_ASSERT(0);
    }
    return application_ptr;
}



void audio_transmitter_configure_task(CONNECTION_IF *application_ptr, audio_transmitter_scenario_type_t scenario_id, audio_transmitter_scenario_sub_id_t sub_id)
{
    if (0) {
        UNUSED(application_ptr);
        UNUSED(scenario_id);
        UNUSED(sub_id);
#if defined(MTK_ANC_SURROUND_MONITOR_ENABLE) || defined(AIR_ADAPTIVE_EQ_ENABLE)
    } else if ((scenario_id == AUDIO_TRANSMITTER_ANC_MONITOR_STREAM)||(scenario_id == AUDIO_TRANSMITTER_ADAPTIVE_EQ_MONITOR_STREAM)){
        application_ptr->source->taskId = DPR_TASK_ID;
        application_ptr->sink->taskid   = DPR_TASK_ID;
#endif
    }

}
#if defined (AIR_DCHS_MODE_ENABLE)
hal_audio_memory_selection_t dchs_sub_ul_mem;
#endif
void audio_transmitter_open(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    CONNECTION_IF *application_ptr;
    mcu2dsp_open_param_p open_param;
    audio_transmitter_scenario_type_t scenario_id;
    audio_transmitter_scenario_sub_id_t sub_id;

    UNUSED(ack);

    scenario_id = (msg.ccni_message[0] >> 8) & 0xFF;
    sub_id.scenario_id = (msg.ccni_message[0] & 0xFF);
    DSP_MW_LOG_I("[audio_transmitter] open, scenario type %d, scenario sub id %d\r\n", 2, scenario_id, sub_id.scenario_id);
#ifdef  AIR_AUDIO_HW_LOOPBACK_ENABLE
    if(scenario_id == AUDIO_TRANSMITTER_AUDIO_HW_LOOPBACK){
        DSP_MW_LOG_I("[audio_transmitter] audio hw loopback open", 0);
        audio_hw_loopback_open(msg,sub_id);
        return;
    }
#endif
    open_param = (mcu2dsp_open_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);

#if defined (AIR_DCHS_MODE_ENABLE)
    if (scenario_id == AUDIO_TRANSMITTER_DCHS){
        if(sub_id.dchs_id == AUDIO_TRANSMITTER_DCHS_UART_UL){
            dchs_ul_scenario_type = open_param->stream_in_param.afe.dchs_ul_scenario_type;
            dsp_uart_ul_open();
            dsp_uart_ul_clear_rx_buffer();
            dsp_uart_ul_clear_tx_buffer();
            DSP_MW_LOG_I("[DCHS UL] open_param sampling_rate:%d,dchs_ul_scenario_type:%d,codec_type:%d", 3,open_param->stream_in_param.afe.sampling_rate,dchs_ul_scenario_type,open_param->stream_in_param.afe.codec_type);
            if(open_param->stream_in_param.afe.sampling_rate == 16000){
                if(open_param->stream_in_param.afe.codec_type == BT_HFP_CODEC_CVSD){
                    DSP_ALG_UpdateEscoTxMode(VOICE_NB);         /*16K sample rate. Need NB algorithm*/
                    DSP_ALG_UpdateEscoRxMode(VOICE_NB);         /*16K sample rate. Need NB algorithm*/
                }else{
                    DSP_ALG_UpdateEscoTxMode(VOICE_WB);         /*16K sample rate. Need WB algorithm*/
                    DSP_ALG_UpdateEscoRxMode(VOICE_WB);         /*16K sample rate. Need WB algorithm*/
                }
                dchs_voice_mode = VOICE_WB;
            }else if(open_param->stream_in_param.afe.sampling_rate == 32000){
#ifdef AIR_BT_BLE_SWB_ENABLE
                DSP_ALG_UpdateEscoTxMode(VOICE_SWB);         /*32K sample rate. Need SWB algorithm*/
                DSP_ALG_UpdateEscoRxMode(VOICE_SWB);         /*32K sample rate. Need SWB algorithm*/
                dchs_voice_mode = VOICE_SWB;
#endif
            }
        }
    }
#endif

    application_ptr = port_audio_transmitter_get_connection_if(scenario_id, sub_id);
    application_ptr->source     = dsp_open_stream_in(open_param);
    application_ptr->sink       = dsp_open_stream_out(open_param);
    application_ptr->transform = NULL;
    audio_transmitter_configure_task(application_ptr, scenario_id, sub_id);

#if defined (AIR_DCHS_MODE_ENABLE)
    if (scenario_id == AUDIO_TRANSMITTER_DCHS){
        if(sub_id.dchs_id == AUDIO_TRANSMITTER_DCHS_UART_UL){
            dchs_sub_ul_mem = application_ptr->source->param.audio.mem_handle.memory_select;
        }
    }
#endif
    /* do scenario's portable open opeartion here */
    extern void port_audio_transmitter_open(audio_transmitter_scenario_type_t scenario_id, audio_transmitter_scenario_sub_id_t sub_id, mcu2dsp_open_param_p open_param, SOURCE source, SINK sink);
    port_audio_transmitter_open(scenario_id, sub_id, open_param, application_ptr->source, application_ptr->sink);

    DSP_Callback_PreloaderConfig(application_ptr->pfeature_table, open_param->audio_scenario_type);
    DSP_MW_LOG_I("[audio_transmitter] open finish", 0);
}

void audio_transmitter_start(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    CONNECTION_IF *application_ptr;
    uint32_t gpt_timer;
    mcu2dsp_start_param_p start_param;
    audio_transmitter_scenario_type_t scenario_id;
    audio_transmitter_scenario_sub_id_t sub_id;

    UNUSED(ack);

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_timer);

    scenario_id = (msg.ccni_message[0] >> 8) & 0xFF;
    sub_id.scenario_id  = (msg.ccni_message[0] & 0xFF);
    DSP_MW_LOG_I("[audio_transmitter] start, gpt_timer %d, scenario type %d, scenario sub id %d\r\n", 3, gpt_timer, scenario_id, sub_id);
#ifdef  AIR_AUDIO_HW_LOOPBACK_ENABLE
    if(scenario_id == AUDIO_TRANSMITTER_AUDIO_HW_LOOPBACK){
        DSP_MW_LOG_I("[audio_transmitter] audio hw loopback start", 0);
        return;
    }
#endif
    application_ptr = port_audio_transmitter_get_connection_if(scenario_id, sub_id);

    start_param = (mcu2dsp_start_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    dsp_start_stream_in(start_param, application_ptr->source);
    dsp_start_stream_out(start_param, application_ptr->sink);
#if defined (AIR_DCHS_MODE_ENABLE)
    if (scenario_id == AUDIO_TRANSMITTER_DCHS){
        if(sub_id.dchs_id == AUDIO_TRANSMITTER_DCHS_UART_UL){
            application_ptr->source->param.audio.AfeBlkControl.u4awsflag = 1;
        }
    }
#endif
    application_ptr->transform = TrasformAudio2Audio(application_ptr->source, application_ptr->sink, application_ptr->pfeature_table);
    if (application_ptr->transform == NULL) {
        AUDIO_ASSERT(0);
    }

    /* do scenario's portable start opeartion here */
    extern void port_audio_transmitter_start(audio_transmitter_scenario_type_t scenario_id, audio_transmitter_scenario_sub_id_t sub_id, mcu2dsp_start_param_p start_param, SOURCE source, SINK sink);
    port_audio_transmitter_start(scenario_id, sub_id, start_param, application_ptr->source, application_ptr->sink);

    DSP_MW_LOG_I("[audio_transmitter] start finish", 0);
}

void audio_transmitter_stop(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    CONNECTION_IF *application_ptr;
    audio_transmitter_scenario_type_t scenario_id;
    audio_transmitter_scenario_sub_id_t sub_id;

    UNUSED(ack);

    scenario_id = (msg.ccni_message[0] >> 8) & 0xFF;
    sub_id.scenario_id  = (msg.ccni_message[0] & 0xFF);
    DSP_MW_LOG_I("[audio_transmitter] stop, scenario type %d, scenario id %d\r\n", 2, scenario_id, sub_id);
#ifdef  AIR_AUDIO_HW_LOOPBACK_ENABLE
    if(scenario_id == AUDIO_TRANSMITTER_AUDIO_HW_LOOPBACK){
        DSP_MW_LOG_I("[audio_transmitter] audio hw loopback stop", 0);
        aud_msg_ack(MSG_MCU2DSP_AUDIO_TRANSMITTER_STOP, FALSE);
        return;
    }
#endif
    application_ptr = port_audio_transmitter_get_connection_if(scenario_id, sub_id);

    /* do scenario's portable stop opeartion here */
    extern void port_audio_transmitter_stop(audio_transmitter_scenario_type_t scenario_id, audio_transmitter_scenario_sub_id_t sub_id, SOURCE source, SINK sink);
    port_audio_transmitter_stop(scenario_id, sub_id, application_ptr->source, application_ptr->sink);

    if (application_ptr->transform != NULL) {
        StreamDSPClose(application_ptr->transform->source, application_ptr->transform->sink, (msg.ccni_message[0] >> 16) | 0x8000);
    }
    application_ptr->transform = NULL;

    DSP_MW_LOG_I("[audio_transmitter] stop finish", 0);
}

void audio_transmitter_close(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    CONNECTION_IF *application_ptr;
    audio_transmitter_scenario_type_t scenario_id;
    audio_transmitter_scenario_sub_id_t sub_id;

    UNUSED(ack);

    scenario_id = (msg.ccni_message[0] >> 8) & 0xFF;
    sub_id.scenario_id  = (msg.ccni_message[0] & 0xFF);
    DSP_MW_LOG_I("[audio_transmitter] close, scenario type %d, scenario id %d\r\n", 2, scenario_id, sub_id);
#ifdef  AIR_AUDIO_HW_LOOPBACK_ENABLE
    if(scenario_id == AUDIO_TRANSMITTER_AUDIO_HW_LOOPBACK){
        DSP_MW_LOG_I("[audio_transmitter] audio hw loopback close", 0);
        audio_hw_loopback_close(msg,sub_id);
        return;
    }
#endif

    application_ptr = port_audio_transmitter_get_connection_if(scenario_id, sub_id);

    /* do scenario's portable close opeartion here */
    extern void port_audio_transmitter_close(audio_transmitter_scenario_type_t scenario_id, audio_transmitter_scenario_sub_id_t sub_id, SOURCE source, SINK sink);
    port_audio_transmitter_close(scenario_id, sub_id, application_ptr->source, application_ptr->sink);

#if defined (AIR_DCHS_MODE_ENABLE)
    if (scenario_id == AUDIO_TRANSMITTER_DCHS){
        if(sub_id.dchs_id == AUDIO_TRANSMITTER_DCHS_UART_UL){
            dsp_uart_ul_clear_rx_buffer();
            dsp_uart_ul_clear_tx_buffer();
            dchs_send_unlock_sleep_msg(false);
        }
    }
#endif
    DSP_Callback_UnloaderConfig(application_ptr->pfeature_table, application_ptr->source->scenario_type);
    SourceClose(application_ptr->source);
    application_ptr->source = NULL;
    SinkClose(application_ptr->sink);
    application_ptr->sink = NULL;
    DSP_MW_LOG_I("[audio_transmitter] close finish", 0);
}

void audio_transmitter_config(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    CONNECTION_IF *application_ptr;
    audio_transmitter_scenario_type_t scenario_id;
    audio_transmitter_scenario_sub_id_t sub_id;
    void *config_param;

    UNUSED(ack);

    scenario_id = (msg.ccni_message[0] >> 8) & 0xFF;
    sub_id.scenario_id  = (msg.ccni_message[0] & 0xFF);
    DSP_MW_LOG_I("[audio transmitter]: config, scenario type %d, scenario id %d\r\n", 2, scenario_id, sub_id);

    application_ptr = port_audio_transmitter_get_connection_if(scenario_id, sub_id);

    config_param = (void *)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);

    extern bool port_audio_transmitter_scenario_config(audio_transmitter_scenario_type_t scenario_id, audio_transmitter_scenario_sub_id_t sub_id, void *config_param);
    port_audio_transmitter_scenario_config(scenario_id, sub_id, config_param);

    DSP_MW_LOG_I("[audio transmitter]: config finish", 0);
}

void audio_transmitter_suspend(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    CONNECTION_IF *application_ptr;
    audio_transmitter_scenario_type_t scenario_id;
    audio_transmitter_scenario_sub_id_t sub_id;

    UNUSED(ack);

    scenario_id = (msg.ccni_message[0] >> 8) & 0xFF;
    sub_id.scenario_id  = (msg.ccni_message[0] & 0xFF);
    DSP_MW_LOG_I("[audio_transmitter] suspend, scenario type %d, scenario id %d\r\n", 2, scenario_id, sub_id);

    application_ptr = port_audio_transmitter_get_connection_if(scenario_id, sub_id);
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
    dsp_trigger_suspend(application_ptr->source, application_ptr->sink);
#endif
    //TODO
    DSP_MW_LOG_I("[audio_transmitter] suspend finish", 0);
}

void audio_transmitter_resume(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    CONNECTION_IF *application_ptr;
    audio_transmitter_scenario_type_t scenario_id;
    audio_transmitter_scenario_sub_id_t sub_id;

    UNUSED(ack);

    scenario_id = (msg.ccni_message[0] >> 8) & 0xFF;
    sub_id.scenario_id  = (msg.ccni_message[0] & 0xFF);
    DSP_MW_LOG_I("[audio_transmitter] resume, scenario type %d, scenario id %d\r\n", 2, scenario_id, sub_id);

    application_ptr = port_audio_transmitter_get_connection_if(scenario_id, sub_id);
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
    dsp_detachable_config(application_ptr, msg);
    dsp_trigger_resume(application_ptr->source, application_ptr->sink);
#endif
    //TODO
    DSP_MW_LOG_I("[audio_transmitter] resume finish", 0);
}

void audio_transmitter_send_message(audio_transmitter_scenario_type_t scenario_type, audio_transmitter_scenario_sub_id_t scenario_id, uint32_t message)
{
    hal_ccni_message_t msg;

    memset((void *)&msg, 0, sizeof(hal_ccni_message_t));
    msg.ccni_message[0] = (MSG_DSP2MCU_AUDIO_TRANSMITTER_DATA_DIRECT << 16) | (scenario_type << 8) | scenario_id.scenario_id;
    msg.ccni_message[1] = message;
    aud_msg_tx_handler(msg, 0, FALSE);

    DSP_MW_LOG_I("[audio_transmitter] send message, scenario type %d, scenario id %d, message 0x%08x", 3, scenario_type, scenario_id, message);
}

#ifdef MTK_GAMING_MODE_HEADSET
void gaming_headset_uplink_enable_irq(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);

    hal_audio_trigger_start_parameter_t start_parameter;
    start_parameter.memory_select = HAL_AUDIO_MEMORY_UL_VUL1;
    start_parameter.enable = true;
    DSP_MW_LOG_I("[audio transmitter] Enable vul interrupt", 0);
    hal_audio_set_value((hal_audio_set_value_parameter_t *)&start_parameter, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
}
#endif /* MTK_GAMING_MODE_HEADSET */
#endif

#ifdef MTK_ANC_ENABLE
extern void anc_passthru_stop_callback(void);
extern void anc_passthru_switch_type_callback(void);
#endif
void DSP_LOCAL_TX_HANDEL(hal_ccni_message_t msg)
{
    switch (msg.ccni_message[0] & 0xFFFF) {
#ifdef MTK_ANC_ENABLE
        case 0x0E06: {
            if (msg.ccni_message[1] == 0x0) {
                anc_passthru_stop_callback();
            } else if (msg.ccni_message[1] == 0x1) {
                anc_passthru_switch_type_callback();
            }
            break;
        }
#endif
        default:
            break;
    }
}

#ifdef MTK_ANC_ENABLE
mcu2dsp_open_adda_param_t g_audio_adda_param;
void DSP_AUDIO_ADDA_NOTIFY(uint32_t memory_addr)
{
    mcu2dsp_open_param_p adda_param_p;
    adda_param_p = (mcu2dsp_open_param_p)hal_memview_cm4_to_dsp0(memory_addr);
    memcpy(&g_audio_adda_param.adc_setting, &(adda_param_p->stream_in_param.afe), sizeof(au_afe_open_param_t));
    memcpy(&g_audio_adda_param.dac_setting, &(adda_param_p->stream_out_param.afe), sizeof(au_afe_open_param_t));
}

hal_audio_device_t DSP_OPEN_DAC(bool enable)
{
    hal_audio_device_t open_device = 0;
    hal_audio_device_parameter_t *device_handle;
    device_handle = (hal_audio_device_parameter_t *)pvPortMalloc(sizeof(hal_audio_device_parameter_t));
    if (device_handle != NULL) {
        memset(device_handle, 0, sizeof(hal_audio_device_parameter_t));
        device_handle->dac.audio_device = (hal_audio_device_t)g_audio_adda_param.dac_setting.audio_device;
        device_handle->dac.rate         = g_audio_adda_param.dac_setting.sampling_rate;
        device_handle->dac.dc_compensation_value  = afe.stream_out.dc_compensation_value;
        device_handle->dac.with_force_change_rate = false;
        device_handle->dac.dac_mode = g_audio_adda_param.dac_setting.dl_dac_mode;
#if defined(AIR_BTA_IC_PREMIUM_G2)
        device_handle->dac.with_high_performance = g_audio_adda_param.dac_setting.performance;
#else
        device_handle->dac.performance = g_audio_adda_param.dac_setting.performance;
#endif
        device_handle->common.scenario_type = AUDIO_SCENARIO_TYPE_ANC;
        hal_audio_set_device(device_handle, device_handle->common.audio_device, enable ? HAL_AUDIO_CONTROL_ON : HAL_AUDIO_CONTROL_OFF);
        if (enable) {
            open_device = device_handle->common.audio_device;
        } else {
            open_device = 0;
        }
        vPortFree(device_handle);
    } else {
        DSP_MW_LOG_E("[ANC API] Error!!! malloc fail for anc_enable_dac", 0);
    }
    return open_device;
}

void DSP_OPEN_ADC(hal_audio_control_t *device, hal_audio_interface_t *dev_interface, hal_audio_device_parameter_t *device_handle)
{
    uint8_t index = 0;
    if (*device & HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_DUAL) {
        device_handle->digital_mic.audio_device  = *device;
        device_handle->digital_mic.mic_interface = *dev_interface;
        device_handle->digital_mic.rate            = g_audio_adda_param.adc_setting.sampling_rate;
        device_handle->digital_mic.bias_select     = g_audio_adda_param.adc_setting.bias_select;
        device_handle->digital_mic.bias_voltage[0] = g_audio_adda_param.adc_setting.bias_voltage[0];
        device_handle->digital_mic.bias_voltage[1] = g_audio_adda_param.adc_setting.bias_voltage[1];
        device_handle->digital_mic.bias_voltage[2] = g_audio_adda_param.adc_setting.bias_voltage[2];
        device_handle->digital_mic.bias_voltage[3] = g_audio_adda_param.adc_setting.bias_voltage[3];
        device_handle->digital_mic.bias_voltage[4] = g_audio_adda_param.adc_setting.bias_voltage[4];
        device_handle->digital_mic.with_external_bias = g_audio_adda_param.adc_setting.with_external_bias;
        device_handle->digital_mic.with_bias_lowpower = g_audio_adda_param.adc_setting.with_bias_lowpower;
        device_handle->digital_mic.bias1_2_with_LDO0  = g_audio_adda_param.adc_setting.bias1_2_with_LDO0;
        switch (device_handle->digital_mic.audio_device) {
            case HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_DUAL:
            case HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_L:
                index = (device_handle->analog_mic.mic_interface == HAL_AUDIO_INTERFACE_1) ? 0 : ((device_handle->analog_mic.mic_interface == HAL_AUDIO_INTERFACE_2) ? 2 : 4);
                device_handle->digital_mic.dmic_selection = g_audio_adda_param.adc_setting.dmic_selection[index];
                break;
            case HAL_AUDIO_CONTROL_DEVICE_DIGITAL_MIC_R:
                index = (device_handle->analog_mic.mic_interface == HAL_AUDIO_INTERFACE_1) ? 1 : ((device_handle->analog_mic.mic_interface == HAL_AUDIO_INTERFACE_2) ? 3 : 5);
                device_handle->digital_mic.dmic_selection = g_audio_adda_param.adc_setting.dmic_selection[index];
                break;
            default:
                device_handle->digital_mic.dmic_selection = HAL_AUDIO_DMIC_GPIO_DMIC0;
                break;
        }
        switch (device_handle->digital_mic.mic_interface) {
            case HAL_AUDIO_INTERFACE_1:
                device_handle->digital_mic.iir_filter = g_audio_adda_param.adc_setting.iir_filter[0];
                break;
            case HAL_AUDIO_INTERFACE_2:
                device_handle->digital_mic.iir_filter = g_audio_adda_param.adc_setting.iir_filter[1];
                break;
            case HAL_AUDIO_INTERFACE_3:
                device_handle->digital_mic.iir_filter = g_audio_adda_param.adc_setting.iir_filter[2];
                break;
            default:
                device_handle->digital_mic.iir_filter = HAL_AUDIO_UL_IIR_5HZ_AT_48KHZ;
                break;
        }
        //DSP_MW_LOG_I("[ANC API] DMIC Enable, device(0x%x), interface(%d), dmic_select(%d)", 3, device_handle->digital_mic.audio_device, device_handle->digital_mic.mic_interface, device_handle->digital_mic.dmic_selection);
    } else {
        device_handle->analog_mic.audio_device  = *device;
        device_handle->analog_mic.mic_interface = *dev_interface;
        device_handle->analog_mic.rate            = g_audio_adda_param.adc_setting.sampling_rate;
        device_handle->analog_mic.bias_select     = g_audio_adda_param.adc_setting.bias_select;
        device_handle->analog_mic.bias_voltage[0] = g_audio_adda_param.adc_setting.bias_voltage[0];
        device_handle->analog_mic.bias_voltage[1] = g_audio_adda_param.adc_setting.bias_voltage[1];
        device_handle->analog_mic.bias_voltage[2] = g_audio_adda_param.adc_setting.bias_voltage[2];
        device_handle->analog_mic.bias_voltage[3] = g_audio_adda_param.adc_setting.bias_voltage[3];
        device_handle->analog_mic.bias_voltage[4] = g_audio_adda_param.adc_setting.bias_voltage[4];
        device_handle->analog_mic.with_external_bias        = g_audio_adda_param.adc_setting.with_external_bias;
        device_handle->analog_mic.adc_parameter.performance = g_audio_adda_param.adc_setting.performance;
        device_handle->analog_mic.with_bias_lowpower        = g_audio_adda_param.adc_setting.with_bias_lowpower;
        device_handle->analog_mic.bias1_2_with_LDO0         = g_audio_adda_param.adc_setting.bias1_2_with_LDO0;
        switch (device_handle->analog_mic.audio_device) {
            case HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_DUAL:
            case HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_L:
                index = (device_handle->analog_mic.mic_interface == HAL_AUDIO_INTERFACE_1) ? 0 : ((device_handle->analog_mic.mic_interface == HAL_AUDIO_INTERFACE_2) ? 2 : 4);
                device_handle->analog_mic.adc_parameter.adc_mode = g_audio_adda_param.adc_setting.ul_adc_mode[index];
                break;
            case HAL_AUDIO_CONTROL_DEVICE_ANALOG_MIC_R:
                index = (device_handle->analog_mic.mic_interface == HAL_AUDIO_INTERFACE_1) ? 1 : ((device_handle->analog_mic.mic_interface == HAL_AUDIO_INTERFACE_2) ? 3 : 5);
                device_handle->analog_mic.adc_parameter.adc_mode = g_audio_adda_param.adc_setting.ul_adc_mode[index];
                break;
            default:
                device_handle->analog_mic.adc_parameter.adc_mode = HAL_AUDIO_ANALOG_INPUT_ACC10K;
                break;
        }
        switch (device_handle->analog_mic.mic_interface) {
            case HAL_AUDIO_INTERFACE_1:
                device_handle->analog_mic.iir_filter = g_audio_adda_param.adc_setting.iir_filter[0];
                break;
            case HAL_AUDIO_INTERFACE_2:
                device_handle->analog_mic.iir_filter = g_audio_adda_param.adc_setting.iir_filter[1];
                break;
            case HAL_AUDIO_INTERFACE_3:
                device_handle->analog_mic.iir_filter = g_audio_adda_param.adc_setting.iir_filter[2];
                break;
            default:
                device_handle->analog_mic.iir_filter = HAL_AUDIO_UL_IIR_25HZ_AT_48KHZ;
                break;
        }
        //DSP_MW_LOG_I("[ANC API] AMIC Enable, device(0x%x), interface(%d), dmic_select(%d)", 3, device_handle->analog_mic.audio_device, device_handle->analog_mic.mic_interface, 99);
    }
}

void DSP_LIB_AUDIO_DUMP(U8 *audio, U32 audio_size, U32 dumpID)
{
#ifdef AIR_AUDIO_DUMP_ENABLE
    LOG_AUDIO_DUMP(audio,  audio_size, (DSP_DATADUMP_MASK_BIT)dumpID);
#else
    UNUSED(audio);
    UNUSED(audio_size);
    UNUSED(dumpID);
#endif
}
#endif

#ifdef MTK_SLT_AUDIO_HW
void AUDIO_SLT_START(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    uint32_t result_total;
    uint32_t rate;
    uint32_t test_case_idx = (msg.ccni_message[0] & 0xffff);

    DSP_MW_LOG_I("SLT test case:%d rate= %d", 2, test_case_idx,msg.ccni_message[1]);
    rate = msg.ccni_message[1];

    result_total = audio_slt_test_case(rate, test_case_idx);

    DSP_MW_LOG_I("AUDIO_SLT_START", 0);
#if 1
    //hal_ccni_message_t msg;
    memset((void *)&msg, 0, sizeof(hal_ccni_message_t));
    msg.ccni_message[0] = 0x75 << 16;               //  MSG_DSP2MCU_COMMON_AUDIO_SLT_RESULT
    DSP_MW_LOG_I("result_total 0x%x", 1, result_total);
    msg.ccni_message[1] = result_total;//0x123;
    aud_msg_tx_handler(msg, 0, FALSE);
#endif
}
#endif

#ifdef MTK_AUDIO_LOOPBACK_TEST_ENABLE
void DSP_AUDIO_LOOPBACK_TEST_CONFIG(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    DSP_MW_LOG_I("[AUDIO LOOPBACK TEST] CONFIG", 0);
    uint32_t addr = (uint32_t)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    audio_loopback_test_inform_result_addr(addr);
    Audio_setting->Audio_source.Pga_mux = HAL_AUDIO_LOOPBACK_TEST_PGA_NULL;
}
#endif

// for volume setting sync
static timer_list_node_t *g_sync_timer_list = NULL;
static aud_msg_cb_node_t *g_dsp_request_sync_cb_front = NULL;
static uint32_t gpt_sync_timer_handle = 0;
hal_gpt_callback_t dsp_audio_request_sync_timer_callback(void *user_data);

// porting function
static aud_msg_cb_node_t *_msg_queue_get_middle(aud_msg_cb_node_t *front, aud_msg_cb_node_t *rear)
{
    if (front == NULL) {
        return NULL;
    }

    aud_msg_cb_node_t *step1 = front;
    aud_msg_cb_node_t *step2 = front->next;

    while (step2 != rear) {
        step2 = step2->next;
        if (step2 != rear) {
            step1 = step1->next;
            step2 = step2->next;
        }
    }

    return step1;
}

static aud_msg_cb_info_t *_msg_queue_search(aud_msg_cb_node_t *front, uint16_t ID)
{
    /* Simple binary search */

    aud_msg_cb_node_t *cb = NULL;
    aud_msg_cb_node_t *rear = NULL;

    do {
        cb = _msg_queue_get_middle(front, rear);

        if (cb == NULL) {
            return NULL;
        } else if (cb->cb_info.msg_id == ID) {
            return &(cb->cb_info);
        } else if (cb->cb_info.msg_id < ID) {
            front = cb->next;
        } else {
            rear = cb;
        }

    } while (front != rear);

    return NULL;
}
static void dsp_audio_msg_ack(uint32_t msg_id, bool from_isr)
{
    // send ACK to cm4 side
    uint32_t ack_msg = 0;
    ack_msg = (uint16_t)msg_id;
    ack_msg |= ((MSG_DSP2MCU_AUDIO_SYNC_DONE | 0x8000) << 16);
    DTM_enqueue(DTM_EVENT_ID_AUDIO_SYNC_END, ack_msg, from_isr);
    DSP_MW_LOG_E("[DSP SYNC] enqueue to dtm task, id [0x%x]\r\n", 1, ack_msg);
}
// msg_callback and ack the msg id
static void dsp_audio_msg_queue_handle(uint32_t msg_id, cm4_dsp_audio_sync_request_param_t *param)
{
    dsp_sync_callback_t callback;
    // msg.ccni_message[0] structure: [CM42DSP_SYNC_ID 16bit]  [scenario_id 8bit]  [action_id 8bit]
    uint32_t scenario_id = (msg_id & 0xff00) >> 8;
    uint32_t action_id = msg_id & 0xff; // the last 8-bit
    aud_msg_cb_info_t *cb = _msg_queue_search(g_dsp_request_sync_cb_front, scenario_id); // [WARNING]
    if (cb != NULL) {
        if (cb->cb_func != NULL) {
            callback = cb->cb_func;
            callback(action_id, param);
        } else {
            // error
            DSP_MW_LOG_E("[DSP SYNC] id [0x%x] [0x%x] no related callback\r\n", 2, scenario_id, action_id);
        }
    } else {
        DSP_MW_LOG_E("[DSP SYNC] id [0x%x] [0x%x] no related register\r\n", 2, scenario_id, action_id);
    }
}

// gpt count compare
static int8_t gpt_count_compare(uint32_t count1, uint32_t count2)
{
    uint32_t tmp1 = count1 > count2 ? count1 : count2;
    uint32_t tmp2 = count1 > count2 ? count2 : count1;
    uint32_t delt = tmp1 - tmp2;
    int8_t sign = delt > 0x7fffffff ? -1 : 1;
    if (count1 > count2) {
        return 1 * sign;
    } else if (count1 < count2) {
        return -1 * sign;
    } else {
        return 0;
    }
}

static aud_msg_status_t dsp_audio_request_sync_timer_list_insert(timer_list_info_t *info)
{
    timer_list_node_t *cur = g_sync_timer_list;
    timer_list_node_t *tmp = NULL;
    if (cur == NULL) {
        timer_list_node_t *cb = (timer_list_node_t *)pvPortMalloc(sizeof(timer_list_node_t));
        if (cb == NULL) {
            DSP_MW_LOG_E("[DSP SYNC] timer_list_node_t malloc failed 1!!\r\n", 0);
            return AUDIO_MSG_STATUS_ERROR;
        }
        memcpy(&(cb->info), info, sizeof(timer_list_info_t));
        cb->next = NULL;
        g_sync_timer_list = cb;
        return AUDIO_MSG_STATUS_OK;
    }
    while ((cur->next != NULL) && (info->mirror_count > cur->next->info.mirror_count)) {  /* Search the the closest node */
        cur = cur->next;
    }

    /* Replace or insert the node */
    timer_list_node_t *cb = (timer_list_node_t *)pvPortMalloc(sizeof(timer_list_node_t));
    if (cb == NULL) {
        DSP_MW_LOG_E("[DSP SYNC] timer_list_node_t malloc failed 2!!\r\n", 0);
        return AUDIO_MSG_STATUS_ERROR;
    }
    memcpy(&(cb->info), info, sizeof(timer_list_info_t));

    if (info->mirror_count > cur->info.mirror_count) {
        tmp = cur->next;
        cur->next = cb;
        cb->next = tmp;
    } else {
        tmp = cur;
        cur = cb;
        cb->next = tmp;
    }

    /*  Updated the front of queue pointer */
    if (g_sync_timer_list->info.mirror_count >= cb->info.mirror_count) { // For the same count, latest is the first!
        // And there is memory leak without the modification.
        g_sync_timer_list = cb;
    }
    return AUDIO_MSG_STATUS_OK;
}

// count the item of timer list
static uint32_t dsp_audio_request_sync_timer_list_get_length(void)
{
    uint32_t cnt = 0;
    timer_list_node_t *timer = (timer_list_node_t *)g_sync_timer_list;
    while (timer) {
        cnt++;
        timer = timer->next;
    }
    return cnt;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void dsp_audio_request_sync_process(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    uint8_t              trigger_flag = 0; // excute callback immediately
    uint32_t             savedmask    = 0; // interrupt mask
    uint32_t             curr_cnt     = 0;
    uint32_t             curr_cnt_tmp = 0;
    int32_t              time_out     = 0;
    hal_gpt_status_t     gpt_status   = HAL_GPT_STATUS_OK;
    cm4_dsp_audio_sync_request_param_t *para_addr   = NULL;
    // for show log
    uint32_t             disable_int_time = 0;
    uint32_t             enable_int_time  = 0;
    uint32_t             total_time       = 0;
    uint64_t             deviation        = 0;
    timer_list_info_t info;
    memset(&info, 0, sizeof(timer_list_info_t));
#ifdef MTK_AUDIO_DSP_SYNC_REQUEST_GPIO_DEBUG
    DSP_MW_LOG_I("[DSP SYNC] init gpio test", 0);
    hal_gpio_set_direction(HAL_GPIO_23, 1);
    hal_gpio_disable_pull(HAL_GPIO_23);
    hal_pinmux_set_function(HAL_GPIO_23, 0);
    hal_gpio_set_output(HAL_GPIO_23, 0);
#endif /* MTK_AUDIO_DSP_SYNC_REQUEST_GPIO_DEBUG */
    /* -1- parse msg info --------------------------------------------------------------------*/
    uint32_t msg_id      = (uint32_t)msg.ccni_message[0];
    info.msg_id          = msg_id;
    para_addr            = (cm4_dsp_audio_sync_request_param_t *)hal_memview_cm4_to_dsp0(msg.ccni_message[1]); // dsp must remap the address
    memcpy(&info.param, para_addr, sizeof(cm4_dsp_audio_sync_request_param_t));
    // dsp_audio_msg_ack(info.msg_id, false);
    /* -2- check the time --------------------------------------------------------------------*/
//#ifdef MTK_AUDIO_DSP_SYNC_REQUEST_DEBUG
    DSP_MW_LOG_I("[DSP SYNC] dsp receive info: id0 = 0x%x, para_addr = 0x%x, channel = %d, vol_gain = %d, t_cnt = %u, nvkey_addr = 0x%x\r\n", 6,
                 msg_id, para_addr, info.param.vol_gain_info.gain_select, info.param.vol_gain_info.gain, info.param.gpt_count, info.param.nvkey_addr);
//#endif
    hal_nvic_save_and_set_interrupt_mask(&savedmask);
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
    curr_cnt_tmp = curr_cnt;
    disable_int_time = curr_cnt;
    time_out = info.param.gpt_count - curr_cnt;
    if (time_out < AUDIO_DSP_SYNC_MIN_TIME_OUT) { // < 1ms
        // AUDIO_ASSERT(0);
        trigger_flag = 1;
        goto ERROR_HANDLE;
    }
    if (time_out > 5000000) { // 5s, Abnormal Case!
        hal_nvic_restore_interrupt_mask(savedmask);
        DSP_MW_LOG_W("[DSP SYNC] ATTENTION: delay time is so big!", 0);
        hal_nvic_save_and_set_interrupt_mask(&savedmask);
        // do nothing
    }
    deviation = time_out;
    info.mirror_count  = deviation + curr_cnt;
    /* -3- append timer list ------------------------------------------------------------------*/
    if (dsp_audio_request_sync_timer_list_insert(&info) != AUDIO_MSG_STATUS_OK) {
        trigger_flag |= 1 << 1;
        goto ERROR_HANDLE;
    }

    /* -4- re-start timer ---------------------------------------------------------------------*/
    if (info.param.gpt_count == g_sync_timer_list->info.param.gpt_count) {
        // restart timer
        if (gpt_sync_timer_handle == 0) {
            gpt_status = hal_gpt_sw_get_timer(&gpt_sync_timer_handle);
            if (gpt_status != HAL_GPT_STATUS_OK) {
                trigger_flag |= 1 << 2;
                // DSP_MW_LOG_W("[DSP SYNC] get gpt_status = %d\r\n", 1, gpt_status);
                goto ERROR_HANDLE;
            }
        } else {
            gpt_status = hal_gpt_sw_stop_timer_us(gpt_sync_timer_handle);
            if (gpt_status != HAL_GPT_STATUS_OK) {
                // trigger_flag |= 1 << 3;
                // goto ERROR_HANDLE;
            }
        }
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
        time_out = info.param.gpt_count - curr_cnt - AUDIO_DSP_SYNC_FIXED_POLLING_TIME; // [TODO] check over flow
        // avoid negative number
        if (time_out < 0) {
            time_out = 0;
        }
        gpt_status = hal_gpt_sw_start_timer_us(gpt_sync_timer_handle, time_out, (hal_gpt_callback_t)dsp_audio_request_sync_timer_callback, NULL);
        if (gpt_status != HAL_GPT_STATUS_OK) {
            trigger_flag |= 1 << 4;
            // DSP_MW_LOG_W("[DSP SYNC] start gpt_status = %d\r\n", 1, gpt_status);
            goto ERROR_HANDLE;
        } else {
            // if (g_sync_timer_list != NULL) {
            //     if (g_sync_timer_list->next == NULL) {
            //         // boost the frequency
            //         dvfs_lock_control("DSP_SYNC", DVFS_78M_SPEED, DVFS_LOCK);
            //     }
            // }
        }
    }
ERROR_HANDLE:
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &enable_int_time);
    hal_nvic_restore_interrupt_mask(savedmask);
    hal_gpt_get_duration_count(disable_int_time, enable_int_time, &total_time);
    DSP_MW_LOG_I("[DSP SYNC] msg process, exit critical region! total time = %d\r\n", 1, total_time);
    /* -5- excute callback immediately -----------------------------------------------------------*/
    if (trigger_flag != 0) {
        // excute callback immediately, search the related scenario's callback
        dsp_audio_msg_queue_handle(info.msg_id, &(info.param));
        dsp_audio_msg_ack(info.msg_id, false);
        DSP_MW_LOG_W("[DSP SYNC] abnormal condition occur, error = %d gpt_status %d\r\n", 2, trigger_flag, gpt_status);
        if (trigger_flag & 0x1C) { // avoid memory leak
            timer_list_node_t *tmp = g_sync_timer_list->next;
            vPortFree(g_sync_timer_list);
            g_sync_timer_list = tmp;
        }
    }
    uint32_t number = dsp_audio_request_sync_timer_list_get_length();
    UNUSED(number);
    if (g_sync_timer_list != NULL) { // show detail information
        DSP_MW_LOG_I("[DSP SYNC] timer list  list_len = %d, msg0 = 0x%x, vol_ch = %d, vol_gain = %d, nvkey_addr = 0x%x, t_cnt = %u, c_cnt = %u, t_out = %d, m_cnt = %u\r\n", 9,
                     number, g_sync_timer_list->info.msg_id, g_sync_timer_list->info.param.vol_gain_info.gain_select, g_sync_timer_list->info.param.vol_gain_info.gain,
                     g_sync_timer_list->info.param.nvkey_addr, g_sync_timer_list->info.param.gpt_count, curr_cnt_tmp, time_out, g_sync_timer_list->info.mirror_count);
    } else {
        DSP_MW_LOG_I("[DSP SYNC] timer list msg0 = 0x%x, vol_ch = %d, vol_gain = %d, nvkey_addr = 0x%x,t_cnt = %u, c_cnt = %u, t_out = %d, m_cnt = %u, number = 0\r\n",
                     8, info.msg_id, info.param.vol_gain_info.gain_select, info.param.vol_gain_info.gain,  info.param.nvkey_addr,  info.param.gpt_count, curr_cnt_tmp, time_out, info.mirror_count);
    }
}

aud_msg_status_t dsp_audio_request_sync_register_callback(cm4_dsp_audio_sync_scenario_type_t request_scenario_id, dsp_sync_callback_t *sync_callback)
{
    uint8_t ack_opt = 0; // means ignore this parameter
    aud_msg_cb_node_t *cur = g_dsp_request_sync_cb_front;
    aud_msg_cb_node_t *tmp = NULL;

    if (cur == NULL) {   /* Nothing in the queue */
        aud_msg_cb_node_t *cb = (aud_msg_cb_node_t *)pvPortMalloc(sizeof(aud_msg_cb_node_t));
        if (cb == NULL) {
            DSP_MW_LOG_E("DSP SYNC CB malloc failed 1!!\r\n", 0);
            return AUDIO_MSG_STATUS_ERROR;
        }
        cb->cb_info.msg_id = request_scenario_id;
        cb->cb_info.cb_func = sync_callback;
        cb->cb_info.ack_option = ack_opt;
        cb->next = NULL;
        g_dsp_request_sync_cb_front = cb;
        return AUDIO_MSG_STATUS_OK;
    }

    while ((cur->next != NULL) && (request_scenario_id >= cur->next->cb_info.msg_id)) {  /* Search the the closest node */
        cur = cur->next;
    }

    /* Replace or insert the node */
    if (request_scenario_id == cur->cb_info.msg_id) {
        cur->cb_info.cb_func = sync_callback;
        cur->cb_info.ack_option = ack_opt;
        return AUDIO_MSG_STATUS_OK;
    } else {
        aud_msg_cb_node_t *cb = (aud_msg_cb_node_t *)pvPortMalloc(sizeof(aud_msg_cb_node_t));
        if (cb == NULL) {
            DSP_MW_LOG_E("DSP SYNC CB malloc failed 2!!\r\n", 0);
            return AUDIO_MSG_STATUS_ERROR;
        }
        cb->cb_info.msg_id = request_scenario_id;
        cb->cb_info.cb_func = sync_callback;
        cb->cb_info.ack_option = ack_opt;

        if (request_scenario_id > cur->cb_info.msg_id) {
            tmp = cur->next;
            cur->next = cb;
            cb->next = tmp;
        } else {
            tmp = cur;
            cur = cb;
            cb->next = tmp;
        }

        /*  Updated the front of queue pointer */
        if (g_dsp_request_sync_cb_front->cb_info.msg_id >= cb->cb_info.msg_id) {
            g_dsp_request_sync_cb_front = cb;
        }
        if (g_dsp_request_sync_cb_front->cb_info.msg_id == cb->cb_info.msg_id) { // one id can only has one callback func.
            DSP_MW_LOG_E("[DSP SYNC] ERROR: register callback, ID 0x%x 0x%x", 2, g_dsp_request_sync_cb_front->cb_info.msg_id,
                         cb->cb_info.msg_id);
            AUDIO_ASSERT(0);
            return AUDIO_MSG_STATUS_ERROR;
        }
    }
    return AUDIO_MSG_STATUS_OK;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ hal_gpt_callback_t dsp_audio_request_sync_timer_callback(void *user_data)
{
    UNUSED(user_data);
    timer_list_info_t info;
    uint32_t savedmask = 0;
    uint32_t curr_cnt  = 0;
    uint32_t time_out  = 0;
    // for show log
    uint32_t          disable_int_time = 0;
    uint32_t          enable_int_time  = 0;
    uint32_t          total_time       = 0;
    U32               bt_clk           = 0;
    U16               intra_clk        = 0;
    timer_list_node_t *tmp             = NULL;  // buffer the next timer list pointer before freeing the first item pointer of timer list
    memset(&info, 0, sizeof(timer_list_info_t));
    hal_nvic_save_and_set_interrupt_mask_special(&savedmask); // enter cirtical code region
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &disable_int_time);
#ifdef MTK_AUDIO_DSP_SYNC_REQUEST_GPIO_DEBUG
    hal_gpio_set_output(HAL_GPIO_2, 0);
#endif /* MTK_AUDIO_DSP_SYNC_REQUEST_GPIO_DEBUG */
LOOP:
#ifdef MTK_AUDIO_DSP_SYNC_REQUEST_DEBUG
    hal_nvic_restore_interrupt_mask_special(savedmask);
    DSP_MW_LOG_I("[DSP SYNC] loop 0x%x %d %d 0x%x %d\r\n", 5, g_sync_timer_list->info.msg_id, g_sync_timer_list->info.param.vol_gain_info.gain_select,
                     g_sync_timer_list->info.param.vol_gain_info.gain, g_sync_timer_list->info.param.nvkey_addr, g_sync_timer_list->info.param.gpt_count);
    hal_nvic_save_and_set_interrupt_mask_special(&savedmask);
#endif /* MTK_AUDIO_DSP_SYNC_REQUEST_DEBUG */
    /* -1- free expired timer ----------------------------------------------------------------*/
    tmp = g_sync_timer_list->next;
    memcpy(&info, &(g_sync_timer_list->info), sizeof(timer_list_info_t));
    /* -2- polling tick and excute expired timer's callback  ---------------------------------*/
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
    if (info.param.gpt_count > curr_cnt) { // gpt register does not overflow
#ifdef MTK_AUDIO_DSP_SYNC_REQUEST_DEBUG
        hal_nvic_restore_interrupt_mask_special(savedmask);
        DSP_MW_LOG_I("[DSP SYNC] trigger %d curr_cnt %u  tar %d", 3, __LINE__, curr_cnt, info.param.gpt_count);
        hal_nvic_save_and_set_interrupt_mask_special(&savedmask);
#endif /* MTK_AUDIO_DSP_SYNC_REQUEST_DEBUG */
        while (1) {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
            if (curr_cnt >= info.param.gpt_count) { // expire at time
                break;
            }
        }
    } else if (curr_cnt - info.param.gpt_count > 0x7fffffff) { // gpt register overflow
#ifdef MTK_AUDIO_DSP_SYNC_REQUEST_DEBUG
        hal_nvic_restore_interrupt_mask_special(savedmask);
        DSP_MW_LOG_I("[DSP SYNC] trigger %d curr_cnt %u  tar %d", 3, __LINE__, curr_cnt, info.param.gpt_count);
        hal_nvic_save_and_set_interrupt_mask_special(&savedmask);
#endif /* MTK_AUDIO_DSP_SYNC_REQUEST_DEBUG */
        while (1) {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
            if (curr_cnt >= info.param.gpt_count) { // expire at time
                if ((curr_cnt & 0x80000000) == 0x0) {
                    break;
                }
            }
        }
    } else {
        hal_nvic_restore_interrupt_mask_special(savedmask);
        DSP_MW_LOG_E("[DSP SYNC]Warning: already expire\r\n", 0);
        hal_nvic_save_and_set_interrupt_mask_special(&savedmask);
        // AUDIO_ASSERT(0);
    }
    // callback and ack
    hal_nvic_restore_interrupt_mask_special(savedmask);
    dsp_audio_msg_queue_handle(info.msg_id, &(info.param));
    MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&intra_clk, BT_CLK_Offset);
    dsp_audio_msg_ack(info.msg_id, true);
    DSP_MW_LOG_I("[DSP SYNC] Trigger, polling end! c_cnt = %u, t_cnt = %u, bt clk 0x%x, phase 0x%x\r\n", 4,
        curr_cnt,
        info.param.gpt_count,
        bt_clk,
        intra_clk
        );
    hal_nvic_save_and_set_interrupt_mask_special(&savedmask); // enter cirtical code region
    vPortFree(g_sync_timer_list);
    g_sync_timer_list = tmp;
#ifdef MTK_AUDIO_DSP_SYNC_REQUEST_DEBUG
    if (g_sync_timer_list != NULL) { // show detail information
        hal_nvic_restore_interrupt_mask_special(savedmask);
        DSP_MW_LOG_I("[DSP SYNC] next timer list msg0 = 0x%x, nvkey_addr = 0x%x, vol_ch = %d, vol_gain = %d, t_cnt = %u, list_len = %d\r\n", 6, g_sync_timer_list->info.msg_id,
                             g_sync_timer_list->info.param.nvkey_addr, g_sync_timer_list->info.param.vol_gain_info.gain_select, g_sync_timer_list->info.param.vol_gain_info.gain, g_sync_timer_list->info.param.gpt_count,
                     dsp_audio_request_sync_timer_list_get_length());
        hal_nvic_save_and_set_interrupt_mask_special(&savedmask);
    }
#endif /* MTK_AUDIO_DSP_SYNC_REQUEST_DEBUG */
    /* -3- check and start next timer  -------------------------------------------------------*/
    if (g_sync_timer_list != NULL) {
        // while (g_sync_timer_list->next) {
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
        if (gpt_count_compare(g_sync_timer_list->info.param.gpt_count, curr_cnt) != 1) {
            // trigger immedately
            hal_nvic_restore_interrupt_mask_special(savedmask);
            DSP_MW_LOG_D("[DSP SYNC] Trigger immediately, goto loop\r\n", 0);
            hal_nvic_save_and_set_interrupt_mask_special(&savedmask);
            goto LOOP; // about 30-40us delay
        } else {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
            time_out = g_sync_timer_list->info.param.gpt_count - curr_cnt - AUDIO_DSP_SYNC_FIXED_POLLING_TIME; // [TODO] check overflow
            int8_t res = gpt_count_compare(g_sync_timer_list->info.param.gpt_count, curr_cnt + AUDIO_DSP_SYNC_REQUEST_MIN_REQUEST_INTERVAL);
            if (res == 1) {
                // start timer
                hal_gpt_status_t gpt_status = hal_gpt_sw_start_timer_us(gpt_sync_timer_handle, time_out, (hal_gpt_callback_t) dsp_audio_request_sync_timer_callback, NULL);
                if (gpt_status != HAL_GPT_STATUS_OK) { // [TODO]
                    hal_nvic_restore_interrupt_mask_special(savedmask);
                    DSP_MW_LOG_E("[DSP SYNC] timer start error\r\n", 0);
                    hal_nvic_save_and_set_interrupt_mask_special(&savedmask);
                    // assert
                }
                // break;
            } else {
                // start polling
                hal_nvic_restore_interrupt_mask_special(savedmask);
                DSP_MW_LOG_D("[DSP SYNC] goto loop\r\n", 0);
                hal_nvic_save_and_set_interrupt_mask_special(&savedmask);
                goto LOOP; // ATTENTION: maybe exist some abnormal sitution
            }
        }
        //}
    }
#ifdef MTK_AUDIO_DSP_SYNC_REQUEST_DEBUG
    hal_nvic_restore_interrupt_mask_special(savedmask);
    DSP_MW_LOG_I("[DSP SYNC] exit loop\r\n", 0);
    hal_nvic_save_and_set_interrupt_mask_special(&savedmask);
#endif /* MTK_AUDIO_DSP_SYNC_REQUEST_DEBUG */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &enable_int_time);
    hal_nvic_restore_interrupt_mask_special(savedmask);
    hal_gpt_get_duration_count(disable_int_time, enable_int_time, &total_time);
    DSP_MW_LOG_I("[DSP SYNC] timer callback, exit citical region! total time = %d\r\n", 1, total_time);
    // release gpt timer handle
    if (g_sync_timer_list == NULL) {
        // down frequency
        // dvfs_lock_control("DSP_SYNC", DVFS_78M_SPEED, DVFS_UNLOCK); // NO API
        if (hal_gpt_sw_free_timer(gpt_sync_timer_handle) != HAL_GPT_STATUS_OK) {
            DSP_MW_LOG_E("[DSP SYNC] gpt timer release error\r\n", 0);
        } else {
            DSP_MW_LOG_I("[DSP SYNC] release timer, timer list len = %d\r\n", 1, dsp_audio_request_sync_timer_list_get_length());
        }
        gpt_sync_timer_handle = 0;
    }
    return 0;
}

#ifdef AIR_AUDIO_HARDWARE_ENABLE
extern void dsp_sync_callback_a2dp(cm4_dsp_audio_sync_action_type_t request_action_id, void *user_data);
extern void dsp_sync_callback_hfp(cm4_dsp_audio_sync_action_type_t request_action_id, void *user_data);
extern void dsp_sync_callback_vp(cm4_dsp_audio_sync_action_type_t request_action_id, void *user_data);
extern void dsp_sync_callback_ble(cm4_dsp_audio_sync_action_type_t request_action_id, void *user_data);
#ifdef AIR_ANC_ADAPTIVE_CLOCK_CONTROL_ENABLE
extern void dsp_sync_callback_adapt_anc(cm4_dsp_audio_sync_action_type_t request_action_id, void * user_data);
#endif
#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
extern void dsp_sync_callback_llf(cm4_dsp_audio_sync_action_type_t request_action_id, void *user_data);
#endif

void dsp_audio_request_sync_initialization(void)
{
    //Register callback
    dsp_audio_request_sync_register_callback(MCU2DSP_SYNC_REQUEST_HFP, (dsp_sync_callback_t*)dsp_sync_callback_hfp);
    //dsp_audio_request_sync_register_callback((cm4_dsp_audio_sync_action_type_t)MCU2DSP_SYNC_REQUEST_A2DP, dsp_sync_callback_a2dp);
    //dsp_audio_request_sync_register_callback((cm4_dsp_audio_sync_action_type_t)MCU2DSP_SYNC_REQUEST_VP, dsp_sync_callback_vp);
    dsp_audio_request_sync_register_callback(MCU2DSP_SYNC_REQUEST_BLE, (dsp_sync_callback_t*)dsp_sync_callback_ble);
#ifdef AIR_ANC_ADAPTIVE_CLOCK_CONTROL_ENABLE
    dsp_audio_request_sync_register_callback((cm4_dsp_audio_sync_scenario_type_t)MCU2DSP_SYNC_REQUEST_ADAPT_ANC, (dsp_sync_callback_t*)dsp_sync_callback_adapt_anc);
#endif
#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
    dsp_audio_request_sync_register_callback(MCU2DSP_SYNC_REQUEST_LLF, (dsp_sync_callback_t*)dsp_sync_callback_llf);
#endif
}
#endif

#if defined(AIR_LE_AUDIO_ENABLE) || defined(AIR_LE_AUDIO_DONGLE_ENABLE) || defined(AIR_BT_LE_LC3PLUS_ENABLE)
uint32_t g_plc_mode = 1;
typedef enum {
    LC3_PARA_TYPE_PLC_MODE = 0,
    LC3_PARA_TYPE_MAX = 0xFFFF,
} lc3_param_type_t;

void lc3_set_param(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    lc3_param_type_t msg16 = (lc3_param_type_t)(msg.ccni_message[0] & 0xFFFF);
    uint32_t msg32 = msg.ccni_message[1];

    UNUSED(ack);

    switch (msg16) {
        case LC3_PARA_TYPE_PLC_MODE:
            g_plc_mode = msg32;
            DSP_MW_LOG_I("[lc3] AT_CMD SET PLC %d",1,g_plc_mode);
            break;
        default:
            return;
    }
}
#endif

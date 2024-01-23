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


#include "race_cmd_feature.h"
#ifdef RACE_DSP_REALTIME_CMD_ENABLE
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "hal.h"
#include "race_cmd.h"
#include "race_cmd_dsprealtime.h"
#include "race_xport.h"
#include "race_noti.h"
#if defined(HAL_AUDIO_MODULE_ENABLED)
#include "hal_ccni.h"
#include "memory_attribute.h"
#include "hal_audio_cm4_dsp_message.h"
#include "hal_audio_message_struct.h"
#include "hal_audio_internal.h"
#include "bt_sink_srv_ami.h"
#include "bt_sink_srv_common.h"
#include "bt_connection_manager_internal.h"
#include "hal_resource_assignment.h"
#include "peq_setting.h"
#endif
#ifndef MTK_ANC_V2
#include "at_command_audio_ata_test.h"
#endif
#include <math.h>
#ifdef AIR_AIRDUMP_ENABLE_MIC_RECORD
#include "record_control.h"
#include "hal_dvfs.h"
#endif
#ifdef MTK_LEAKAGE_DETECTION_ENABLE
#include "race_event.h"
#include "leakage_detection_control.h"
#endif
#if defined(MTK_USER_TRIGGER_FF_ENABLE) || defined(AIR_ANC_USER_UNAWARE_ENABLE) || defined(AIR_ANC_WIND_DETECTION_ENABLE) || defined(AIR_ANC_ENVIRONMENT_DETECTION_ENABLE)
#include "race_cmd_relay_cmd.h"
#ifdef MTK_USER_TRIGGER_ADAPTIVE_FF_V2
#include "user_trigger_adaptive_ff.h"
//#include "apps_config_vp_manager.h"
#include "voice_prompt_api.h"
#endif
#endif
#if defined(AIR_ANC_USER_UNAWARE_ENABLE) || defined(AIR_ANC_WIND_DETECTION_ENABLE) || defined(AIR_ANC_ENVIRONMENT_DETECTION_ENABLE)
#include "anc_monitor.h"
#ifdef ADAPIVE_ANC_STREAM_CONTROL
extern bool g_adaptive_ANC_stream_MP_control_flag; //true for adaptive ANC is currently under MP mode.
extern bool g_adaptive_ANC_stream_suspend_control_flag; ////true for adaptive ANC stream is suspended by race
extern audio_anc_monitor_adaptive_anc_status_t g_adaptive_anc_status;
#endif
#endif
#include "race_event.h"
#include "race_event_internal.h"
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
#include "audio_transmitter_internal.h"
#endif
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)||defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) || defined(AIR_ADAPTIVE_EQ_ENABLE)
#include "race_cmd_co_sys.h"
#endif
#include "race_cmd_hostaudio.h"


#ifdef AIR_COMPONENT_CALIBRATION_ENABLE
#include "audio_calibration.h"
#endif

#ifdef AIR_DCHS_MODE_ENABLE
#include "scenario_dchs.h"
#include "mux_ll_uart_latch.h"
#endif


////////////////////////////////////////////////////////////////////////////////
// Constant Definitions ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define dsp_realtime_min(x,y) (x > y) ? y : x


//////////////////////////////////////////////////////////////////////////////
// Global Variables ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)||defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
static SemaphoreHandle_t g_race_dsprealtime_mutex = NULL;
#endif
#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
#define PEQ_NVKEY_BUF_SIZE (sizeof(AMI_AWS_MCE_PEQ_PACKT_t) + sizeof(DSP_PEQ_NVKEY_t))
uint8_t g_peq_nvkey[PEQ_NVKEY_BUF_SIZE];
uint8_t g_peq_nvkey_available = 1;
#endif
#ifdef AIR_ADVANCED_PASSTHROUGH_ENABLE_V2
static uint8_t s_self_fitting_race_ch_id;
#endif
#ifdef MTK_ANC_ENABLE
static uint8_t g_anc_race_ch_id;
#if 1   //application sync workaround.
extern volatile uint8_t g_app_sync_status;
uint8_t g_app_sync_flash_no = 0xF;
#endif
#if !defined(MTK_ANC_V2) | defined(MTK_LEAKAGE_DETECTION_ENABLE)
static SemaphoreHandle_t g_race_anc_mutex = NULL;
#endif
#ifndef MTK_ANC_V2
static uint32_t g_anc_notify_on; //bit[0]:agent_done, bit[1]:partner_done, bit[16:23]:agent_filter, bit[24:31]:parnter_filter
static uint32_t g_anc_notify_off; //bit[0]:agent_done, bit[1]:partner_done, bit[31]: request_from_race
static uint32_t g_anc_set_agent_vol; //anc_sw_gain_t
static uint32_t g_anc_set_partner_vol; //anc_sw_gain_t
#endif
#endif
#ifdef AIR_AIRDUMP_ENABLE
uint8_t g_airdump_race_ch_id;
uint8_t g_airdump_cnt_past = 0;
static TimerHandle_t g_airdump_timer = NULL;
#endif
#ifdef AIR_AIRDUMP_ENABLE_MIC_RECORD
uint8_t g_airdump_common_race_ch_id;
uint8_t g_airdump_common_race_request;
uint8_t g_airdump_common_cnt = 0;
TimerHandle_t g_airdump_common_timer = NULL;
int16_t g_record_airdump_data[256];  /*TODO: Change to dynamic alloc.*/
extern uint16_t g_dump;
extern bool g_record_airdump;
#if 0
const int16_t g_128_1ktone[128] = {
    0, 6284, 11612, 15173, 16423, 15171, 11612, 6284, -1, -6284, -11613, -15172, -16422, -15173, -11613, -6284,
    0, 6284, 11612, 15173, 16423, 15171, 11612, 6284, -1, -6284, -11613, -15172, -16422, -15173, -11613, -6284,
    0, 6284, 11612, 15173, 16423, 15171, 11612, 6284, -1, -6284, -11613, -15172, -16422, -15173, -11613, -6284,
    0, 6284, 11612, 15173, 16423, 15171, 11612, 6284, -1, -6284, -11613, -15172, -16422, -15173, -11613, -6284,
    0, 6284, 11612, 15173, 16423, 15171, 11612, 6284, -1, -6284, -11613, -15172, -16422, -15173, -11613, -6284,
    0, 6284, 11612, 15173, 16423, 15171, 11612, 6284, -1, -6284, -11613, -15172, -16422, -15173, -11613, -6284,
    0, 6284, 11612, 15173, 16423, 15171, 11612, 6284, -1, -6284, -11613, -15172, -16422, -15173, -11613, -6284,
    0, 6284, 11612, 15173, 16423, 15171, 11612, 6284, -1, -6284, -11613, -15172, -16422, -15173, -11613, -6284,
};
const int16_t g_128_2ktone[128] = {
    0, 11612, 16422, 11612, 0, -11612, -16422, -11612, 1, 11612, 16422, 11613, -1, -11612, -16422, -11613,
    0, 11612, 16422, 11612, 0, -11612, -16422, -11612, 1, 11612, 16422, 11613, -1, -11612, -16422, -11613,
    0, 11612, 16422, 11612, 0, -11612, -16422, -11612, 1, 11612, 16422, 11613, -1, -11612, -16422, -11613,
    0, 11612, 16422, 11612, 0, -11612, -16422, -11612, 1, 11612, 16422, 11613, -1, -11612, -16422, -11613,
    0, 11612, 16422, 11612, 0, -11612, -16422, -11612, 1, 11612, 16422, 11613, -1, -11612, -16422, -11613,
    0, 11612, 16422, 11612, 0, -11612, -16422, -11612, 1, 11612, 16422, 11613, -1, -11612, -16422, -11613,
    0, 11612, 16422, 11612, 0, -11612, -16422, -11612, 1, 11612, 16422, 11613, -1, -11612, -16422, -11613,
    0, 11612, 16422, 11612, 0, -11612, -16422, -11612, 1, 11612, 16422, 11613, -1, -11612, -16422, -11613,
};
#endif
#endif
#ifdef MTK_LEAKAGE_DETECTION_ENABLE
uint8_t g_leakage_detection_race_ch_id = 0;
static uint16_t g_LD_result_agent = 0;  //bit[0:7]:result, bit[15]:done_or_not
static uint16_t g_LD_result_partner = 0;//bit[0:7]:result, bit[15]:done_or_not
static TimerHandle_t   s_xLeakageDetectionOneShotTimer = NULL;
#ifdef AIR_FADP_ANC_COMPENSATION_ENABLE
uint8_t g_fadp_anc_compensation_race_ch_id = 0;
int16_t g_SzD_result_L = 0;
int16_t g_SzD_result_R = 0;
static TimerHandle_t s_xFADPANCCompensationOneShotTimer = NULL;
extern uint8_t g_FADP_USE_DEFAULT_FIR;
#endif
#endif
#ifdef MTK_USER_TRIGGER_FF_ENABLE
#ifndef MTK_USER_TRIGGER_ADAPTIVE_FF_V2
#define USER_TRIGGER_FF_STATUS_RUNNING 2
#define USER_TRIGGER_FF_STATUS_DONE 3
#define USER_TRIGGER_FF_STATUS_ABORT 4
#define USER_TRIGGER_FF_STATUS_SMALL_VOLUME 5
#define USER_TRIGGER_FF_STATUS_Compare 6
#endif
#endif

extern void *pvPortMallocNC(size_t xWantedSize);
extern void vPortFreeNC(void *pv);
//////////////////////////////////////////////////////////////////////////////
// FUNCTION DECLARATIONS /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)||defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
bool race_dsprealtime_relay_handler(ptr_race_pkt_t pRaceHeaderCmd, uint16_t cmd_length, uint8_t channel_id, uint8_t send_to_follower, uint8_t cosys_result);
bool race_dsprealtime_relay_check(ptr_race_pkt_t pRaceHeaderCmd);
#endif

#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)||defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
static void race_dsprealtime_mutex_take(void)
{
    if (g_race_dsprealtime_mutex == NULL) {
        g_race_dsprealtime_mutex = xSemaphoreCreateMutex();
    }
    if (g_race_dsprealtime_mutex != NULL) {
        xSemaphoreTake(g_race_dsprealtime_mutex, portMAX_DELAY);
    }
}
static void race_dsprealtime_mutex_give(void)
{
    if (g_race_dsprealtime_mutex != NULL) {
        xSemaphoreGive(g_race_dsprealtime_mutex);
    }
}
#endif

#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
/**
 * race_dsprt_peq_report
 *
 * For agent and partner, g_peq_nvkey buffer is used to store new PEQ coefficients and parameters.
 * Release g_peq_nvkey with set g_peq_nvkey_available as 1.
 *
 * @pu2PeqParam : reserved.
 */
void race_dsprt_peq_report(uint16_t *pu2PeqParam)
{
    g_peq_nvkey_available = 1;
}

/**
 * race_dsprt_peq_realtime_data
 *
 * For agent, prepare AMI parameters for ami event, AMI_EVENT_PEQ_REALTIME, and send IF packet which contain PEQ data to partner.
 * For partner, prepare AMI paramters for ami event, AMI_EVENT_PEQ_REALTIME.
 *
 * @setting_mode  : [Agent only] 0:PEQ_DIRECT  1:PEQ_SYNC.
 * @target_bt_clk   : [Agent only] a bt clock for agent/partner to apply PEQ simultaneously.
 * @p_coef            : [Agent only] new PEQ coefficients.
 * @coef_size        : [Agent only] new PEQ coefficient size (bytes).
 */
uint32_t race_dsprt_peq_realtime_data(uint8_t phase_id, uint8_t setting_mode, uint32_t target_bt_clk, uint8_t *p_coef, uint32_t coef_size, am_feature_type_t audio_path_id)
{
    uint32_t ret = 0;
    uint8_t *peq_coef_buf;
    bt_sink_srv_am_feature_t am_feature;
    RACE_LOG_MSGID_I("race_dsprt_peq_realtime_data phase_id:%d,setting_mode:%d,target_bt_clk:%d,audio_path_id:%x\n", 4, phase_id,setting_mode,target_bt_clk,audio_path_id);
    if (bt_connection_manager_device_local_info_get_aws_role() == BT_AWS_MCE_ROLE_AGENT) { //Agent (Transmitter)
        AMI_AWS_MCE_PEQ_PACKT_t *peq_packet = (AMI_AWS_MCE_PEQ_PACKT_t *)race_mem_alloc(sizeof(AMI_AWS_MCE_PEQ_PACKT_t) + coef_size);
        if (peq_packet) {
            uint32_t size = ((uint32_t)&peq_packet->peq_data.peq_data_rt.peq_coef - (uint32_t)peq_packet) + coef_size;
            peq_packet->phase_id = phase_id;
            peq_packet->setting_mode = setting_mode;
            peq_packet->target_bt_clk = target_bt_clk;
            peq_packet->peq_data.peq_data_rt.peq_coef_size = coef_size;
            memcpy((void *)&peq_packet->peq_data.peq_data_rt.peq_coef, (void *)p_coef, coef_size);
            bt_sink_srv_aws_mce_ami_data(AMI_EVENT_PEQ_REALTIME, (uint8_t *)peq_packet, size, false);
            race_mem_free(peq_packet);
        } else {
            configASSERT(0 && "peq realtime : 1st malloc failed");
        }
    }
    peq_coef_buf = race_mem_alloc(coef_size);
    if (peq_coef_buf) {
        memcpy((void *)peq_coef_buf, (void *)p_coef, coef_size);
        memset(&am_feature, 0, sizeof(bt_sink_srv_am_feature_t));
        am_feature.type_mask                             = audio_path_id;
        am_feature.feature_param.peq_param.enable        = 1;
        am_feature.feature_param.peq_param.sound_mode    = PEQ_SOUND_MODE_REAlTIME;
        am_feature.feature_param.peq_param.u2ParamSize   = (uint16_t)coef_size;
        am_feature.feature_param.peq_param.pu2Param      = (uint16_t *)peq_coef_buf;
        am_feature.feature_param.peq_param.target_bt_clk = target_bt_clk;
        am_feature.feature_param.peq_param.setting_mode  = setting_mode;
        am_feature.feature_param.peq_param.phase_id      = phase_id;
        am_feature.feature_param.peq_param.peq_notify_cb = race_dsprt_peq_report;
        ret = am_audio_set_feature(FEATURE_NO_NEED_ID, &am_feature);
    } else {
        ret = -1;
        configASSERT(0 && "peq realtime : 2nd malloc failed");
    }
    return ret;
}

/**
 * race_dsprt_peq_change_mode_data
 *
 * For agent, prepare AMI parameters for ami event, AMI_EVENT_PEQ_CHANGE_MODE, and send IF packet which contain PEQ data to partner.
 * For partner, prepare AMI paramters for ami event, AMI_EVENT_PEQ_CHANGE_MODE.
 *
 * @setting_mode  : [Agent only] 0:PEQ_DIRECT  1:PEQ_SYNC.
 * @target_bt_clk   : [Agent only] a bt clock for agent/partner to apply PEQ simultaneously.
 * @enable            : [Agent only] going to enable PEQ or disable PEQ.
 * @sound_mode    : [Agent only] going to change to sound_mode mode.
 */
uint32_t race_dsprt_peq_change_mode_data(uint8_t phase_id, uint8_t setting_mode, uint32_t target_bt_clk, uint8_t enable, uint8_t sound_mode, am_feature_type_t audio_path_id)
{
    uint32_t ret = 0;
    bt_sink_srv_am_feature_t am_feature;
    RACE_LOG_MSGID_I("race_dsprt_peq_change_mode_data enable:%d,sound_mode:%d,setting_mode:%d,target_bt_clk:%d\n", 4, enable, sound_mode,setting_mode,target_bt_clk);
    if ((bt_connection_manager_device_local_info_get_aws_role() == BT_AWS_MCE_ROLE_AGENT)&&(audio_path_id != AM_AUDIO_AEQ)) { //Agent (Transmitter)
        AMI_AWS_MCE_PEQ_PACKT_t *peq_packet = (AMI_AWS_MCE_PEQ_PACKT_t *)race_mem_alloc(sizeof(AMI_AWS_MCE_PEQ_PACKT_t));
        uint32_t size = (uint32_t)&peq_packet->peq_data.peq_data_cm.reserved - (uint32_t)peq_packet;
        if (peq_packet) {
            peq_packet->type_mask = 0;
            peq_packet->phase_id = phase_id;
            peq_packet->setting_mode = setting_mode;
            peq_packet->target_bt_clk = target_bt_clk;
            peq_packet->peq_data.peq_data_cm.enable = enable;
            peq_packet->peq_data.peq_data_cm.sound_mode = sound_mode;
            bt_sink_srv_aws_mce_ami_data(AMI_EVENT_PEQ_CHANGE_MODE, (uint8_t *)peq_packet, size, false);
            race_mem_free(peq_packet);
        } else {
            configASSERT(0 && "peq change : 1st malloc failed");
        }
    }
    memset(&am_feature, 0, sizeof(bt_sink_srv_am_feature_t));
    am_feature.type_mask                             = audio_path_id;
    am_feature.feature_param.peq_param.enable        = enable;
    am_feature.feature_param.peq_param.sound_mode    = sound_mode;
    am_feature.feature_param.peq_param.u2ParamSize   = 0;
    am_feature.feature_param.peq_param.pu2Param      = 0;
    am_feature.feature_param.peq_param.target_bt_clk = target_bt_clk;
    am_feature.feature_param.peq_param.setting_mode  = setting_mode;
    am_feature.feature_param.peq_param.phase_id      = phase_id;
    am_feature.feature_param.peq_param.peq_notify_cb = race_dsprt_peq_report;
    ret = am_audio_set_feature(FEATURE_NO_NEED_ID, &am_feature);
    return ret;
}

#ifdef MTK_BT_SPEAKER_ENABLE
uint32_t race_dsprt_peq_change_sound_mode(uint8_t sound_mode)
{
    bt_clock_t target_bt_clk = {0};
    uint8_t setting_mode = PEQ_DIRECT;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

#ifdef MTK_AWS_MCE_ENABLE
    race_dsprt_peq_get_target_bt_clk(role, &setting_mode, &target_bt_clk);
#endif
    uint32_t ret = 0;
    bt_sink_srv_am_feature_t am_feature;
    RACE_LOG_MSGID_I("[SPK]peq_change_sound_mode sound_mode:%d,setting_mode:%d,target_bt_clk:%d\n", 3, sound_mode,setting_mode,target_bt_clk);

    memset(&am_feature, 0, sizeof(bt_sink_srv_am_feature_t));
    am_feature.type_mask                             = AM_A2DP_PEQ;
    am_feature.feature_param.peq_param.enable        = 1;
    am_feature.feature_param.peq_param.sound_mode    = sound_mode;
    am_feature.feature_param.peq_param.u2ParamSize   = 0;
    am_feature.feature_param.peq_param.pu2Param      = 0;
    am_feature.feature_param.peq_param.target_bt_clk = target_bt_clk.nclk;
    am_feature.feature_param.peq_param.setting_mode  = setting_mode;
    am_feature.feature_param.peq_param.phase_id      = 0;
    am_feature.feature_param.peq_param.peq_notify_cb = race_dsprt_peq_report;
    ret = am_audio_set_feature(FEATURE_NO_NEED_ID, &am_feature);
    return ret;
}
#endif

#ifdef AIR_ADAPTIVE_EQ_ENABLE
uint32_t race_dsprt_aeq_change_mode_data(uint8_t enable, uint8_t sound_mode, uint8_t flag)
{
    uint32_t ret = 0;
    bt_clock_t target_bt_clk = {0};
    uint8_t setting_mode = PEQ_DIRECT;
#ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    race_dsprt_peq_get_target_bt_clk(role, &setting_mode, &target_bt_clk);
#endif
    bt_sink_srv_am_feature_t aeq_am_feature;
    RACE_LOG_MSGID_I("race_dsprt_aeq_change_mode_data enable:%d,sound_mode:%d, target_bt_clk:%d, flag:%d\n", 4, enable, sound_mode, target_bt_clk.nclk,flag);

    AMI_AWS_MCE_PEQ_PACKT_t *peq_packet = (AMI_AWS_MCE_PEQ_PACKT_t *)race_mem_alloc(sizeof(AMI_AWS_MCE_PEQ_PACKT_t));
    uint32_t size = (uint32_t)&peq_packet->peq_data.peq_data_cm.reserved - (uint32_t)peq_packet;
    if (peq_packet) {
        peq_packet->type_mask = flag;
        peq_packet->phase_id = 0;
        peq_packet->setting_mode = setting_mode;
        peq_packet->target_bt_clk = target_bt_clk.nclk;
        peq_packet->peq_data.peq_data_cm.enable = enable;
        peq_packet->peq_data.peq_data_cm.sound_mode = sound_mode;
        bt_sink_srv_aws_mce_ami_data(AMI_EVENT_PEQ_CHANGE_MODE, (uint8_t *)peq_packet, size, false);
        race_mem_free(peq_packet);
    } else {
        RACE_LOG_MSGID_E("peq change : 1st malloc failed, size:%d\n", 1, sizeof(AMI_AWS_MCE_PEQ_PACKT_t));
        configASSERT(0);
    }
    if(flag != 1){
        return ret;
    }
    memset(&aeq_am_feature, 0, sizeof(bt_sink_srv_am_feature_t));
    aeq_am_feature.type_mask                             = AM_AUDIO_AEQ;
    aeq_am_feature.feature_param.peq_param.enable        = enable;
    aeq_am_feature.feature_param.peq_param.sound_mode    = sound_mode;
    aeq_am_feature.feature_param.peq_param.u2ParamSize   = 0;
    aeq_am_feature.feature_param.peq_param.pu2Param      = 0;
    aeq_am_feature.feature_param.peq_param.target_bt_clk = target_bt_clk.nclk;
    aeq_am_feature.feature_param.peq_param.setting_mode  = setting_mode;
    aeq_am_feature.feature_param.peq_param.phase_id      = 0;
    aeq_am_feature.feature_param.peq_param.peq_notify_cb = race_dsprt_peq_report;
    ret = am_audio_set_feature(FEATURE_NO_NEED_ID, &aeq_am_feature);
    return ret;
}
#endif

#ifdef MTK_AWS_MCE_ENABLE
/**
 * race_dsprt_peq_collect_data
 *
 * For partner, to collect IF packets which contain PEQ data and call function to prepare AMI paramters according to ami event ...
 *
 * @info : report info from aws_mce_report service
 */
int32_t race_dsprt_peq_collect_data(bt_aws_mce_report_info_t *info)
{
    AMI_AWS_MCE_PACKET_HDR_t *ami_pkt_header = (AMI_AWS_MCE_PACKET_HDR_t *)info->param;
    AMI_AWS_MCE_PEQ_PACKT_t *peq_param = (AMI_AWS_MCE_PEQ_PACKT_t *)((uint8_t *)info->param + sizeof(AMI_AWS_MCE_PACKET_HDR_t));

    static ami_event_t _ami_event = 0;
    static uint32_t _save_len = 0;
    static uint32_t _total_pkt = 0;
    static int32_t _pre_pkt = 0;

    uint32_t temp_size;
    int32_t ret = 1;

    if ((ami_pkt_header->ami_event != AMI_EVENT_PEQ_REALTIME) && (ami_pkt_header->ami_event != AMI_EVENT_PEQ_CHANGE_MODE)) {
        return -2;
    }

    if (ami_pkt_header->SubPktId == 0) {
        _ami_event = ami_pkt_header->ami_event;
        _save_len = 0;
        _total_pkt = (uint32_t)ami_pkt_header->numSubPkt;
        _pre_pkt = -1;
        g_peq_nvkey_available = 0;
    }

    if ((ami_pkt_header->SubPktId == (_pre_pkt + 1)) && (ami_pkt_header->numSubPkt == _total_pkt) && (ami_pkt_header->ami_event == _ami_event) && (_total_pkt >= 1)) {
        temp_size = (uint32_t)info->param_len - sizeof(AMI_AWS_MCE_PACKET_HDR_t);
        memcpy(g_peq_nvkey + _save_len, peq_param, temp_size);
        _save_len += temp_size;
        _pre_pkt = (uint32_t)ami_pkt_header->SubPktId;
        if (_save_len > PEQ_NVKEY_BUF_SIZE) {
            configASSERT(0 && "PEQ collect data from IF");
        }
    } else {
        return -2;
    }

    if ((_pre_pkt + 1) == _total_pkt) {
        bt_sink_srv_am_feature_t am_feature;
        memset(&am_feature, 0, sizeof(bt_sink_srv_am_feature_t));
        switch (ami_pkt_header->ami_event) {
            case AMI_EVENT_PEQ_REALTIME: {
                AMI_AWS_MCE_PEQ_PACKT_t *peq_packet = (AMI_AWS_MCE_PEQ_PACKT_t *)g_peq_nvkey;
                uint32_t size = ((uint32_t)&peq_packet->peq_data.peq_data_rt.peq_coef - (uint32_t)peq_packet) + peq_packet->peq_data.peq_data_rt.peq_coef_size;
                if (_save_len == size) {
                    uint16_t peq_coef_buf_size = peq_packet->peq_data.peq_data_rt.peq_coef_size;
                    uint8_t *peq_coef_buf = (uint8_t *)race_mem_alloc(peq_coef_buf_size);
                    if (peq_coef_buf) {
                        memcpy(peq_coef_buf, &peq_packet->peq_data.peq_data_rt.peq_coef, peq_coef_buf_size);
                        am_feature.type_mask                             = AM_A2DP_PEQ;
                        am_feature.feature_param.peq_param.enable        = 1;
                        am_feature.feature_param.peq_param.sound_mode    = PEQ_SOUND_MODE_REAlTIME;
                        am_feature.feature_param.peq_param.pu2Param      = (uint16_t *)peq_coef_buf;
                        am_feature.feature_param.peq_param.u2ParamSize   = peq_coef_buf_size;
                        am_feature.feature_param.peq_param.target_bt_clk = peq_packet->target_bt_clk;
                        am_feature.feature_param.peq_param.setting_mode  = peq_packet->setting_mode;
                        am_feature.feature_param.peq_param.phase_id      = peq_packet->phase_id;
                        am_feature.feature_param.peq_param.peq_notify_cb = race_dsprt_peq_report;
                        ret = am_audio_set_feature(FEATURE_NO_NEED_ID, &am_feature);
                    } else {
                        configASSERT(0 && "peq collect data - realtime malloc fail");
                        ret = -1;
                    }
                    g_peq_nvkey_available = 1;
                } else {
                    ret = -1;
                    configASSERT(0 && "PEQ acc packet size is wrong");
                }
                break;
            }
            case AMI_EVENT_PEQ_CHANGE_MODE: {
                AMI_AWS_MCE_PEQ_PACKT_t *peq_packet = (AMI_AWS_MCE_PEQ_PACKT_t *)g_peq_nvkey;
                uint32_t size = (uint32_t)&peq_packet->peq_data.peq_data_cm.reserved - (uint32_t)peq_packet;
                if (_save_len == size) {
                    if(peq_packet->type_mask == 0){
                    am_feature.type_mask                             = AM_A2DP_PEQ;
#ifdef AIR_ADAPTIVE_EQ_ENABLE
                    }else if(peq_packet->type_mask == 1){
                        am_feature.type_mask                             = AM_AUDIO_AEQ;
                    }else if(peq_packet->type_mask == 2){
                        ret = aud_set_aeq_detect_bypass(0);
                        g_peq_nvkey_available = 1;
                        break;
                    }else if(peq_packet->type_mask == 3){
                        ret = aud_set_aeq_detect_bypass(1);
                        g_peq_nvkey_available = 1;
                        break;
#endif
                    }
                    am_feature.feature_param.peq_param.enable        = peq_packet->peq_data.peq_data_cm.enable;
                    am_feature.feature_param.peq_param.sound_mode    = peq_packet->peq_data.peq_data_cm.sound_mode;
                    am_feature.feature_param.peq_param.pu2Param      = 0;
                    am_feature.feature_param.peq_param.u2ParamSize   = 0;
                    am_feature.feature_param.peq_param.target_bt_clk = peq_packet->target_bt_clk;
                    am_feature.feature_param.peq_param.setting_mode  = peq_packet->setting_mode;
                    am_feature.feature_param.peq_param.phase_id      = peq_packet->phase_id;
                    am_feature.feature_param.peq_param.peq_notify_cb = race_dsprt_peq_report;
                    ret = am_audio_set_feature(FEATURE_NO_NEED_ID, &am_feature);
                    g_peq_nvkey_available = 1;
                } else {
                    ret = -1;
                    configASSERT(0 && "PEQ packet size is wrong");
                }
                break;
            }
            default: {
                ret = -1;
                break;
            }
        }
        _ami_event = 0;
        _save_len = 0;
        _total_pkt = 0;
        _pre_pkt = 0;
    }

    if ((ret != 0) && (ret != 1)) {
        g_peq_nvkey_available = 1;
        RACE_LOG_MSGID_E("race_dsprt_peq_collect_data FAIL\n", 0);
    }

    return ret;
}

int32_t race_dsprt_peq_get_target_bt_clk(bt_aws_mce_role_t role, uint8_t *setting_mode, bt_clock_t *target_bt_clk)
{
    bt_clock_t current_bt_clk = {0};
    // int32_t diff = 0;
    uint32_t ret;
    if (role == BT_AWS_MCE_ROLE_AGENT) {
        *setting_mode = PEQ_SYNC;
        ret = bt_sink_srv_bt_clock_addition(&current_bt_clk, NULL, 0);
        ret = bt_sink_srv_bt_clock_addition(target_bt_clk, NULL, PEQ_FW_LATENCY * 1000);

        // diff = (((int32_t)target_bt_clk->nclk - (int32_t)current_bt_clk.nclk) * 625 / 2 + ((int32_t)target_bt_clk->nclk_intra - (int32_t)current_bt_clk.nclk_intra));
        if (ret == BT_STATUS_FAIL) { // for agent only case
            *setting_mode = PEQ_DIRECT;
        }
    } else {
        *setting_mode = PEQ_DIRECT;
    }
    return 0;
}
#endif
#endif

#ifdef MTK_ANC_ENABLE
static void race_dsp_realtime_send_notify_msg(uint16_t race_id, uint16_t race_event, uint16_t result, uint16_t length, uint8_t *payload)
{
    RACE_ERRCODE notify_ret = RACE_ERRCODE_FAIL;
    race_dsprealtime_notify_struct *notify_param = (race_dsprealtime_notify_struct *)race_mem_alloc(sizeof(race_dsprealtime_notify_struct) + length);
    if (notify_param) {
        notify_param->dsp_realtime_race_id   = race_id;
        notify_param->dsp_realtime_race_evet = race_event;
        notify_param->result                 = result;
        notify_param->length                 = length;
        notify_param->race_channel           = g_anc_race_ch_id;
#if 0
        if (race_event == RACE_ANC_ON) {
            RACE_CMD_ANC_PASSTHRU_ON_V3_PARAM *local_payload = (RACE_CMD_ANC_PASSTHRU_ON_V3_PARAM *)payload;
            RACE_LOG_MSGID_I("[RRdebug]send_notify, race_id(%d), length(%d), ANC_ON flash_no(%d), type(%d), sync_mode(%d)", 5, notify_param->race_channel, length, local_payload->flash_no, local_payload->ancType, local_payload->syncMode);
        }
#endif
        if (payload) {
            memcpy(notify_param->payload, payload, length);
        }
    }
    notify_ret = race_send_event_notify_msg(RACE_EVENT_TYPE_AUDIO_DSP_REALTIME, notify_param);
    if (RACE_ERRCODE_SUCCESS != notify_ret) {
        if (notify_param) {
            race_mem_free(notify_param);
        }
    }
}

#if !defined(MTK_ANC_V2) | defined(MTK_LEAKAGE_DETECTION_ENABLE)
static void race_anc_mutex_take(void)
{
    if (g_race_anc_mutex == NULL) {
        g_race_anc_mutex = xSemaphoreCreateMutex();
        if (g_race_anc_mutex == NULL) {
            RACE_LOG_MSGID_E("g_race_anc_mutex create error\r\n", 0);
        }
    }
    if (g_race_anc_mutex != NULL) {
        xSemaphoreTake(g_race_anc_mutex, portMAX_DELAY);
    }
}
static void race_anc_mutex_give(void)
{
    if (g_race_anc_mutex != NULL) {
        xSemaphoreGive(g_race_anc_mutex);
    }
}
#endif

void race_dsprt_anc_notify(audio_anc_control_event_t event_id, uint8_t fromPartner, uint32_t arg)
{
    UNUSED(fromPartner);
    UNUSED(arg);
    void *pEvt = NULL;
    RACE_MMI_GET_ENUM_EVT_STRU *pEvt_MMI = NULL;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    RACE_MMI_ANC_STATUS_STRU debug;

    if (role != BT_AWS_MCE_ROLE_AGENT && role != BT_AWS_MCE_ROLE_NONE) {
        return;
    }
    uint8_t                  anc_enable;
    audio_anc_control_type_t anc_type;
    audio_anc_control_get_status(&anc_enable, NULL, &anc_type, NULL, NULL, NULL);

    if ((anc_enable == 0) || (anc_type < AUDIO_ANC_CONTROL_TYPE_MASK_BEGIN) || ((anc_type & 0xFFFF0000) == AUDIO_ANC_CONTROL_TYPE_PT_ADAP) || ((anc_type & 0xFFFF0000) == AUDIO_ANC_CONTROL_TYPE_PT_HW_VIVID)) {
        pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_NOTIFICATION, (uint16_t)RACE_MMI_GET_ENUM, (uint16_t)sizeof(RACE_MMI_GET_ENUM_EVT_STRU) + sizeof(RACE_MMI_ANC_STATUS_STRU), g_anc_race_ch_id);
        if (pEvt) {
            memset(pEvt, 0, sizeof(RACE_MMI_GET_ENUM_EVT_STRU)+sizeof(RACE_MMI_ANC_STATUS_STRU));
            pEvt_MMI = (RACE_MMI_GET_ENUM_EVT_STRU *)pEvt;
            pEvt_MMI->module = RACE_MMI_MODULE_ANC_STATUS;
            race_mmi_get_anc_status(&pEvt_MMI->data[0], &pEvt_MMI->status);

            memcpy(&debug, &pEvt_MMI->data[0], sizeof(RACE_MMI_ANC_STATUS_STRU));
            race_status_t race_status = race_flush_packet((void *)pEvt_MMI, g_anc_race_ch_id);
            RACE_LOG_MSGID_I("[anc_notify]race_dsprt_anc_notify, race_status %d, filter_id %d, runtime_gain %d, type %d", 4, race_status, debug.anc_flash_id, debug.anc_runtime_gain, debug.anc_type);
        }
    }

    return;
}


#ifndef MTK_ANC_V2
#ifdef MTK_AWS_MCE_ENABLE
uint32_t race_dsprt_anc_feedback_data(aws_mce_report_anc_param_t *anc_param)
{
    race_general_msg_t msg_queue_item;
    PTR_RACE_COMMON_HDR_STRU race_pkt_hdr;
    race_dsprealtime_anc_struct *anc_r_nv_cmd;
    uint8_t channel_id = (uint8_t)(anc_param->ch_info & 0xFF);
    RACE_ERRCODE error;

    switch (channel_id) {
        case RACE_SERIAL_PORT_TYPE_UART:
            msg_queue_item.dev_t = SERIAL_PORT_DEV_UART_0;
            break;
        case RACE_SERIAL_PORT_TYPE_SPP:
            msg_queue_item.dev_t = SERIAL_PORT_DEV_BT_SPP;
            break;
        case RACE_SERIAL_PORT_TYPE_BLE:
            msg_queue_item.dev_t = SERIAL_PORT_DEV_BT_LE;
            break;
        case RACE_SERIAL_PORT_TYPE_AIRUPDATE:
            msg_queue_item.dev_t = SERIAL_PORT_DEV_BT_AIRUPDATE;
            break;
        default:
            msg_queue_item.dev_t = SERIAL_PORT_DEV_UNDEFINED;
            break;
    }
    //msg_queue_item.dev_t = SERIAL_PORT_DEV_UART_0;
    msg_queue_item.msg_id = MSG_ID_RACE_LOCAL_RSP_NOTIFY_IND;
    msg_queue_item.msg_data = race_mem_alloc(sizeof(RACE_COMMON_HDR_STRU) + sizeof(race_dsprealtime_anc_struct));
    if (msg_queue_item.msg_data) {
        race_pkt_hdr = (PTR_RACE_COMMON_HDR_STRU)msg_queue_item.msg_data;
        race_pkt_hdr->pktId.value = (uint8_t)(anc_param->ch_info >> 8);
        race_pkt_hdr->type = RACE_TYPE_COMMAND;
        race_pkt_hdr->length = sizeof(RACE_COMMON_HDR_STRU) + sizeof(race_dsprealtime_anc_struct) - 4;
        race_pkt_hdr->id = RACE_DSPREALTIME_ANC;
        anc_r_nv_cmd = (race_dsprealtime_anc_struct *)(msg_queue_item.msg_data + sizeof(RACE_COMMON_HDR_STRU));
        anc_r_nv_cmd->status = 0x80 | (anc_param->ch_info & 0xFF); //channel_id
        anc_r_nv_cmd->anc_id = RACE_ANC_READ_PARTNER_NVDM;
        anc_r_nv_cmd->param.gain.val = anc_param->arg;
        error = race_send_msg(&msg_queue_item);
    } else {
        configASSERT(0);
        error = RACE_ERRCODE_FAIL;
    }
    return error;
}
#endif

void race_dsprt_anc_notify(anc_control_event_t event_id, uint8_t fromPartner, uint32_t arg)
{
    if (anc_get_sync_time() == 0) {
        bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
        race_anc_mutex_take();
        if ((role == BT_AWS_MCE_ROLE_AGENT) || (role == BT_AWS_MCE_ROLE_NONE)) {
            if (event_id == ANC_CONTROL_EVENT_ON) {
                uint32_t filter = (g_anc_notify_on >> 8) & 0xFF;
                if ((filter == 0) || (arg == 0) || (filter != arg)) {
                    race_anc_mutex_give();
                    return;
                }
                if (fromPartner == 0) {
                    g_anc_notify_on |= RACE_ANC_AGENT;
                    g_anc_notify_on |= (arg << 16);
                    if ((g_anc_notify_on & RACE_ANC_PARTNER) && (filter != ((g_anc_notify_on >> 24) & 0xFF))) {
                        g_anc_notify_on &= (~RACE_ANC_PARTNER);
                        g_anc_notify_on &= (~(0xFF << 24));
                    }
                } else {
                    g_anc_notify_on |= RACE_ANC_PARTNER;
                    g_anc_notify_on |= (arg << 24);
                    if ((g_anc_notify_on & RACE_ANC_AGENT) && (filter != ((g_anc_notify_on >> 16) & 0xFF))) {
                        g_anc_notify_on &= (~RACE_ANC_AGENT);
                        g_anc_notify_on &= (~(0xFF << 16));
                    }
                }
#ifdef MTK_AWS_MCE_ENABLE
                if ((g_anc_notify_on & RACE_ANC_BOTH) == RACE_ANC_BOTH)
#else
                if (g_anc_notify_on & RACE_ANC_AGENT)
#endif
                {
                    race_dsprealtime_anc_struct *pEvt = RACE_ClaimPacket(RACE_TYPE_NOTIFICATION, RACE_DSPREALTIME_ANC, sizeof(race_dsprealtime_anc_struct), g_anc_race_ch_id);
                    if (pEvt != NULL) {
                        memset(pEvt, 0, sizeof(race_dsprealtime_anc_struct));
                        pEvt->anc_id = RACE_ANC_ON;
                        pEvt->param.anc_on_param.anc_filter_type = (uint8_t)(arg & ANC_FILTER_TYPE_MASK);
                        pEvt->param.anc_on_param.anc_mode = (uint8_t)((arg & ANC_FF_ONLY_BIT_MASK) ? 1 : (arg & ANC_FB_ONLY_BIT_MASK) ? 2 : 0);
                        race_flush_packet((void *)pEvt, g_anc_race_ch_id);
                        g_anc_notify_on = 0;
                    }
                }
            } else if (event_id == ANC_CONTROL_EVENT_SET_VOLUME) {
                uint32_t *p_set_vol = (fromPartner == 0) ? (&g_anc_set_agent_vol) : (&g_anc_set_partner_vol);
                if ((*p_set_vol != arg)) {
                    race_anc_mutex_give();
                    return;
                } else {
                    race_dsprealtime_anc_struct *pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE, RACE_DSPREALTIME_ANC, sizeof(race_dsprealtime_anc_struct), g_anc_race_ch_id);
                    if (pEvt != NULL) {
                        memset(pEvt, 0, sizeof(race_dsprealtime_anc_struct));
                        pEvt->param.gain.val = arg;
                        if (fromPartner == 0) {
                            pEvt->anc_id = RACE_ANC_SET_AGENT_VOL;
                        } else {
                            pEvt->anc_id = RACE_ANC_SET_PARTNER_VOL;
                        }
                        race_flush_packet((void *)pEvt, g_anc_race_ch_id);
                        *p_set_vol = 0;
                    }
                }
            } else if (event_id == ANC_CONTROL_EVENT_OFF) {
                if (g_anc_notify_off == 0) {
                    race_anc_mutex_give();
                    return;
                }
                if (fromPartner == 0) {
                    g_anc_notify_off |= RACE_ANC_AGENT;
                } else {
                    g_anc_notify_off |= RACE_ANC_PARTNER;
                }
#ifdef MTK_AWS_MCE_ENABLE
                if ((g_anc_notify_off & RACE_ANC_BOTH) == RACE_ANC_BOTH)
#else
                if (g_anc_notify_off & RACE_ANC_AGENT)
#endif
                {
                    race_dsprealtime_anc_struct *pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE, RACE_DSPREALTIME_ANC, sizeof(race_dsprealtime_anc_struct), g_anc_race_ch_id);
                    if (pEvt != NULL) {
                        memset(pEvt, 0, sizeof(race_dsprealtime_anc_struct));
                        pEvt->anc_id = RACE_ANC_OFF;
                        race_flush_packet((void *)pEvt, g_anc_race_ch_id);
                        g_anc_notify_off = 0;
                    }
                }
            }
        }
#ifdef MTK_AWS_MCE_ENABLE
        else { //partner
            if (event_id == ANC_CONTROL_EVENT_ON) {
                if (arg == 0) {
                    race_anc_mutex_give();
                    return;
                }
                anc_send_aws_mce_param(ANC_DIRECT, ANC_CONTROL_EVENT_ON, (uint32_t)arg, 0);
#ifndef MTK_ANC_V2
            } else if (event_id == ANC_CONTROL_EVENT_SET_VOLUME) {
                anc_send_aws_mce_param(ANC_DIRECT, ANC_CONTROL_EVENT_SET_VOLUME, (uint32_t)arg, 0);
#endif
            } else if (event_id == ANC_CONTROL_EVENT_OFF) {
                anc_send_aws_mce_param(ANC_DIRECT, ANC_CONTROL_EVENT_OFF, (uint32_t)arg, 0);
            }
        }
#endif
        race_anc_mutex_give();
    }
}

#ifdef MTK_USER_TRIGGER_FF_ENABLE
#ifndef MTK_USER_TRIGGER_ADAPTIVE_FF_V2
static void anc_user_trigger_adaptive_ff_callback(int32_t Cal_status)
{
    extern anc_user_trigger_ff_param_t g_user_trigger_ff_info;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_status_t ret = BT_STATUS_FAIL;

    if (Cal_status != USER_TRIGGER_FF_STATUS_DONE) {
        typedef struct {
            uint8_t status;
            uint8_t mode;
        } PACKED RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_EVT_STRU;

        RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_EVT_STRU *pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                                                              RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK,
                                                                              sizeof(RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_EVT_STRU),
                                                                              g_anc_race_ch_id);

        audio_anc_user_trigger_ff_stop();
        audio_anc_user_trigger_ff_recover_anc(Cal_status);

        if (pEvt) {
            pEvt->status = Cal_status;
            pEvt->mode = ADAPTIVE_FF_ANC_MODE;

            if ((role == BT_AWS_MCE_ROLE_AGENT) || (role == BT_AWS_MCE_ROLE_NONE)) {
                race_flush_packet((void *)pEvt, g_anc_race_ch_id);

            } else {/*partner send agent result*/
                race_send_pkt_t *pSndPkt;
                pSndPkt = (void *)race_pointer_cnv_pkt_to_send_pkt((void *)pEvt);
                race_pkt_t      *pret;
                race_send_pkt_t *psend;
                psend = (race_send_pkt_t *)pSndPkt;
                pret = &psend->race_data;
#if (RACE_DEBUG_PRINT_ENABLE)
                race_dump((uint8_t *)pret, RACE_DBG_EVT);
#endif

                if (pSndPkt) {
                    #ifdef RACE_RELAY_CMD_ENABLE
                    ret = bt_send_aws_mce_race_cmd_data(&pSndPkt->race_data, pSndPkt->length, g_anc_race_ch_id, RACE_CMD_RSP_FROM_PARTNER, 0);
                    #endif
                    if (ret != BT_STATUS_SUCCESS) {
                        RACE_LOG_MSGID_I("[relay_cmd][user_trigger_ff]partner send relay req FAIL \n", 0);
                    } else {
                        RACE_LOG_MSGID_I("[relay_cmd][user_trigger_ff]anc_user_trigger_adaptive_ff_callback, send status:%d success", pEvt->status);
//                        peq_relay_dbg.send_idx++;
                    }
                    race_mem_free(pSndPkt);
                }
            }
        }

    } else {
        typedef struct {
            uint8_t status;
            uint8_t mode;
            int32_t array[300];
        } PACKED RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_EVT_STRU;

        RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_EVT_STRU *pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                                                              RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK,
                                                                              sizeof(RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_EVT_STRU),
                                                                              g_anc_race_ch_id);

        if (pEvt) {
            pEvt->status = Cal_status;
            pEvt->mode = ADAPTIVE_FF_ANC_MODE;

            memcpy(pEvt->array, (g_user_trigger_ff_info.report_array + 1), sizeof(int32_t) * 300);

            race_send_pkt_t *pSndPkt;
            uint32_t port_handle, ret_size, size;
            uint8_t *ptr;

            if ((role == BT_AWS_MCE_ROLE_AGENT) || (role == BT_AWS_MCE_ROLE_NONE)) {
                pSndPkt = race_pointer_cnv_pkt_to_send_pkt((void *)pEvt);
                port_handle = race_get_port_handle_by_channel_id(g_anc_race_ch_id);
                ret_size = race_port_send_data(port_handle, (uint8_t *)&pSndPkt->race_data, pSndPkt->length);

                size = pSndPkt->length;
                ptr = (uint8_t *)&pSndPkt->race_data;
                size -= ret_size;
                ptr += ret_size;
                while (size > 0) {
                    ret_size = race_port_send_data(port_handle, ptr, size);
                    size -= ret_size;
                    ptr += ret_size;
                }
                race_mem_free(pSndPkt);

            } else {
                pSndPkt = race_pointer_cnv_pkt_to_send_pkt((void *)pEvt);
                race_pkt_t      *pret;
                race_send_pkt_t *psend;
                psend = (race_send_pkt_t *)pSndPkt;
                pret = &psend->race_data;
                race_dump((uint8_t *)pret, RACE_DBG_EVT);
                if (pSndPkt) {
                    #ifdef RACE_RELAY_CMD_ENABLE
                    ret = bt_send_aws_mce_race_cmd_data(&pSndPkt->race_data, pSndPkt->length, g_anc_race_ch_id, RACE_CMD_RSP_FROM_PARTNER, 0);
                    #endif
                    if (ret != BT_STATUS_SUCCESS) {
                        RACE_LOG_MSGID_I("[relay_cmd][user_trigger_ff]partner send relay req FAIL \n", 0);
                    } else {
                        RACE_LOG_MSGID_I("[relay_cmd][user_trigger_ff]anc_user_trigger_adaptive_ff_callback, send status:%d success", pEvt->status);

                    }
                    race_mem_free(pSndPkt);
                }
            }
        }
    }

}

static void anc_user_trigger_adaptive_ff_receive_filter_callback(int32_t Cal_status)
{
    extern anc_user_trigger_ff_param_t g_user_trigger_ff_info;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_status_t ret = BT_STATUS_FAIL;

    typedef struct {
        uint8_t status;
        uint8_t mode;
    } PACKED RACE_DSPREALTIME_ANC_ADAPTIVE_RESULT_EVT_STRU;

    RACE_DSPREALTIME_ANC_ADAPTIVE_RESULT_EVT_STRU *pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                                                           RACE_DSPREALTIME_ANC_ADAPTIVE_RESULT,
                                                                           sizeof(RACE_DSPREALTIME_ANC_ADAPTIVE_RESULT_EVT_STRU),
                                                                           g_anc_race_ch_id);

    if (pEvt) {
        pEvt->status = (uint8_t)Cal_status;
        pEvt->mode = ADAPTIVE_FF_ANC_MODE;
        if ((role == BT_AWS_MCE_ROLE_AGENT) || (role == BT_AWS_MCE_ROLE_NONE)) {
            race_flush_packet((void *)pEvt, g_anc_race_ch_id);

        } else {
            race_send_pkt_t *pSndPkt;
            pSndPkt = (void *)race_pointer_cnv_pkt_to_send_pkt((void *)pEvt);
            race_pkt_t      *pret;
            race_send_pkt_t *psend;
            psend = (race_send_pkt_t *)pSndPkt;
            pret = &psend->race_data;
#if (RACE_DEBUG_PRINT_ENABLE)
            race_dump((uint8_t *)pret, RACE_DBG_EVT);
#endif

            if (pSndPkt) {
                #ifdef RACE_RELAY_CMD_ENABLE
                ret = bt_send_aws_mce_race_cmd_data(&pSndPkt->race_data, pSndPkt->length, g_anc_race_ch_id, RACE_CMD_RSP_FROM_PARTNER, 0);
                #endif
                if (ret != BT_STATUS_SUCCESS) {
                    RACE_LOG_MSGID_I("[relay_cmd][user_trigger_ff]partner send relay req FAIL \n", 0);
                } else {
                    RACE_LOG_MSGID_I("[relay_cmd][user_trigger_ff]anc_user_trigger_adaptive_ff_receive_filter_callback, send status:%d success", pEvt->status);

                }
                race_mem_free(pSndPkt);
            }
        }
    }

    audio_anc_user_trigger_ff_stop();
    /*turn on ANC if needed*/
    audio_anc_user_trigger_ff_recover_anc(Cal_status);
    race_send_event_notify_msg(RACE_EVENT_TYPE_ANC_ADAPTIVE_FF_END, NULL);
}
#endif
#endif

#else

#ifdef MTK_USER_TRIGGER_FF_ENABLE
void user_trigger_adaptive_ff_racecmd_response(uint8_t mode, uint8_t data_id, uint8_t *data_buff, uint16_t data_len, uint8_t start_or_stop, uint8_t seq_num, uint32_t completed_len)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_status_t ret = BT_STATUS_FAIL;
    RACE_LOG_MSGID_I("[user_trigger_ff] user_trigger_adaptive_ff_racecmd_response 1, mode:%d, data_id:%d data_len:%d data_buff:0x%x, start_or_stop:%d, seq_num:%d, completed_len:%d", 7, mode, data_id, data_len, data_buff, start_or_stop, seq_num, completed_len);
    typedef struct {
        adaptive_check_notify_t header;
        uint8_t data_id;
        uint8_t seq_num;
        uint8_t data[data_len];
    } PACKED user_trigger_adaptive_ff_notify_t;

        user_trigger_adaptive_ff_notify_t *pEvt = RACE_ClaimPacket(RACE_TYPE_NOTIFICATION,
                                                                   RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK,
                                                                   sizeof(user_trigger_adaptive_ff_notify_t),
                                                                   g_anc_race_ch_id);
        if (pEvt) {
            pEvt->header.start_or_stop     = start_or_stop;
            pEvt->header.mode              = ADAPTIVE_FF_ANC_MODE;
            pEvt->header.bit_per_sample    = 2;
            pEvt->header.channel_num       = 0;
            pEvt->header.frame_size        = 0;
            pEvt->header.seq_num           = 0;
            pEvt->header.total_data_length = data_len + 2;
            pEvt->header.data_length       = data_len;
            pEvt->data_id = data_id;
            pEvt->seq_num = seq_num;
            memcpy(pEvt->data, data_buff, data_len);

            race_send_pkt_t *pSndPkt;
            pSndPkt = race_pointer_cnv_pkt_to_send_pkt((void *)pEvt);

            if (pSndPkt) {
                if ((role == BT_AWS_MCE_ROLE_AGENT) || (role == BT_AWS_MCE_ROLE_NONE)) {
                uint32_t port_handle, ret_size, size;
                uint8_t *ptr;
                uint32_t send_res = 0;
                port_handle = race_get_port_handle_by_channel_id(g_anc_race_ch_id);

                size = pSndPkt->length;
                ptr = (uint8_t *)&pSndPkt->race_data;
                size -= completed_len;
                ptr += completed_len;

                #if (RACE_DEBUG_PRINT_ENABLE)
                race_dump((uint8_t *)ptr, RACE_DBG_EVT);
                #endif
                ret_size = race_port_send_data(port_handle, ptr, size);
                send_res = (ret_size & (1 << 31));
                ret_size &= ~(1 << 31);

                    RACE_LOG_MSGID_I("[user_trigger_ff] (Agent)send_size:%u, send_res:%u, ret_size:%u, data_id:%d, seq_num:%d", 5, size, send_res, ret_size, data_id, seq_num);
                } else {
                    race_pkt_t      *pret;
                    race_send_pkt_t *psend;
                    psend = (race_send_pkt_t *)pSndPkt;
                    pret = &psend->race_data;
                    #if (RACE_DEBUG_PRINT_ENABLE)
                    race_dump((uint8_t *)pret, RACE_DBG_EVT);
                    #endif

                    if (pSndPkt) {
                        #ifdef RACE_RELAY_CMD_ENABLE
                        ret = bt_send_aws_mce_race_cmd_data(&pSndPkt->race_data, pSndPkt->length, g_anc_race_ch_id, RACE_CMD_RSP_FROM_PARTNER, 0);
                        #endif
                        if (ret != BT_STATUS_SUCCESS) {
                            RACE_LOG_MSGID_E("[relay_cmd][user_trigger_ff]partner send relay req FAIL \n", 0);
                        } else {
        //                    audio_user_trigger_adaptive_ff_set_sending_data_status(true);
                            RACE_LOG_MSGID_I("[relay_cmd][user_trigger_ff]partner send relay req, send id:%d, seq_num:%d success", 2, pEvt->data_id, pEvt->seq_num);
                        }
                        race_mem_free(pSndPkt);
                    }
                }
            }
        }
}

void user_trigger_adaptive_ff_racecmd_response_extend(uint8_t race_type, uint8_t status, uint16_t data_len, void *data_buff)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_status_t ret = BT_STATUS_FAIL;
    RACE_LOG_MSGID_I("[user_trigger_ff] user_trigger_adaptive_ff_racecmd_response_extend, race_type:%x, status:%d data_len:%d data_buff:0x%x", 4, race_type, status, data_len, data_buff);

    typedef struct {
        uint8_t status;
        uint8_t mode;
        uint16_t data_len;
        uint8_t data;
    } PACKED user_trigger_ff_extend_cmd_resp_t;

    user_trigger_ff_extend_cmd_resp_t *pEvt = RACE_ClaimPacket(race_type,
                                                               RACE_DSPREALTIME_ANC_ADAPTIVE_EXTEND,
                                                               sizeof(user_trigger_ff_extend_cmd_resp_t) + data_len - 1,
                                                               g_anc_race_ch_id);
    if (pEvt) {
        pEvt->status = status;
        pEvt->mode = ADAPTIVE_FF_ANC_MODE;
        pEvt->data_len = data_len;
        memcpy(&pEvt->data, data_buff, data_len);
//        vPortFree(data_buff);

        if ((role == BT_AWS_MCE_ROLE_AGENT) || (role == BT_AWS_MCE_ROLE_NONE)) {
            race_send_pkt_t *pSndPkt;
            uint32_t port_handle, ret_size, size;
            uint8_t *ptr;
            uint32_t send_res = 0;

            pSndPkt = race_pointer_cnv_pkt_to_send_pkt((void *)pEvt);
            port_handle = race_get_port_handle_by_channel_id(g_anc_race_ch_id);

            size = pSndPkt->length;
            ptr = (uint8_t *)&pSndPkt->race_data;

#if (RACE_DEBUG_PRINT_ENABLE)
            race_dump((uint8_t *)ptr, RACE_DBG_EVT);
#endif

            ret_size = race_port_send_data(port_handle, ptr, size);
            send_res = (ret_size & (1 << 31));
            ret_size &= ~(1 << 31);

            RACE_LOG_MSGID_I("[user_trigger_ff] (Agent)send_size:%u, send_res:%u, ret_size:%u", 3, size, send_res, ret_size);
            race_mem_free(pSndPkt);

        } else {//role == BT_AWS_MCE_ROLE_PARTNER
            race_send_pkt_t *pSndPkt;

            pSndPkt = race_pointer_cnv_pkt_to_send_pkt((void *)pEvt);
            race_pkt_t      *pret;
            race_send_pkt_t *psend;
            psend = (race_send_pkt_t *)pSndPkt;
            pret = &psend->race_data;
#if (RACE_DEBUG_PRINT_ENABLE)
            race_dump((uint8_t *)pret, RACE_DBG_EVT);
#endif
            if (pSndPkt) {
                #ifdef RACE_RELAY_CMD_ENABLE
                ret = bt_send_aws_mce_race_cmd_data(&pSndPkt->race_data, pSndPkt->length, g_anc_race_ch_id, RACE_CMD_RSP_FROM_PARTNER, 0);
                #endif
                if (ret != BT_STATUS_SUCCESS) {
                    RACE_LOG_MSGID_E("[relay_cmd][user_trigger_ff]partner send relay req FAIL \n", 0);
                }
                race_mem_free(pSndPkt);
            }
        }
    }
}

#endif

#endif

#ifdef MTK_LEAKAGE_DETECTION_ENABLE
void audio_anc_leakage_detection_timer_check_result(void);

void anc_leakage_detection_racecmd_callback(uint16_t leakage_status)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    RACE_LOG_MSGID_I("[RECORD_LC] anc_leakage_detection_racecmd_callback result:%d for role:0x%x", 2, leakage_status, role);

    audio_anc_leakage_compensation_stop();

    if (leakage_status == LD_STATUS_PASS) {
        leakage_status = 0; //re-map
    }

    if ((role == BT_AWS_MCE_ROLE_AGENT) || (role == BT_AWS_MCE_ROLE_NONE)) {
        anc_leakage_detection_racecmd_response(leakage_status, 0);

    } else {
#ifndef MTK_ANC_V2
        anc_send_aws_mce_param(ANC_DIRECT, ANC_CONTROL_EVENT_LD_START, (uint32_t)leakage_status, 0);
#else
//        audio_anc_control_cap_t anc_cap;
//        anc_cap.target_device = TO_THEOTHER;
//        anc_cap.control_misc.extend_use_parameters = (uint32_t)leakage_status;
//        anc_send_aws_mce_param(ANC_DIRECT, ANC_CONTROL_EVENT_LD_START, &anc_cap, 0);
        audio_anc_leakage_detection_sync_para_t arg;
        arg.target_device = TO_THEOTHER;
        arg.extend_param = (uint32_t)leakage_status;
        audio_anc_leakage_detection_send_aws_mce_param(LD_DIRECT, AUDIO_LEAKAGE_DETECTION_CONTROL_EVENT_START, &arg, 0);
#endif
    }

    //no need to resume a2dp because a2dp will be resumed by AM when VP close.
    //am_audio_dl_resume();

}

void anc_leakage_detection_racecmd_response(uint16_t leakage_status, uint8_t fromPartner)
{
    typedef struct {
        adaptive_check_notify_t header;
        uint8_t agent_status;
        uint8_t partner_status;
    } PACKED leakage_detection_notify_t;

    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    race_anc_mutex_take();
    if (fromPartner == 0) {
        g_LD_result_agent = leakage_status;
        g_LD_result_agent |= 0x8000;
    } else {
        g_LD_result_partner = leakage_status;
        g_LD_result_partner |= 0x8000;
    }

    if ((role == BT_AWS_MCE_ROLE_NONE)
        || ((bt_sink_srv_cm_get_special_aws_device() != NULL) && (bt_sink_srv_cm_get_aws_link_state() != BT_AWS_MCE_AGENT_STATE_ATTACHED))
        || ((g_LD_result_agent & 0x8000) && (g_LD_result_partner & 0x8000))) {
        leakage_detection_notify_t *pEvt = RACE_ClaimPacket(RACE_TYPE_NOTIFICATION,
                                                            RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK,
                                                            sizeof(leakage_detection_notify_t),
                                                            g_leakage_detection_race_ch_id);

        if ((g_LD_result_partner & 0x8000) == 0) {
            g_LD_result_partner = LD_STATUS_ABSENT;
        }
        if (pEvt) {
            pEvt->header.start_or_stop     = 1;
            pEvt->header.mode              = LEAKAGE_DETECTION_MODE;
            pEvt->header.bit_per_sample    = 2;
            pEvt->header.channel_num       = 0;
            pEvt->header.frame_size        = 0;
            pEvt->header.seq_num           = 0;
            pEvt->header.total_data_length = 2;
            pEvt->header.data_length       = 2;
            pEvt->agent_status             = (uint8_t)(g_LD_result_agent & 0xFF);
            pEvt->partner_status           = (uint8_t)(g_LD_result_partner & 0xFF);
            RACE_LOG_MSGID_I("[RECORD_LC] Send LD result to APK : %d %d", 2, pEvt->agent_status, pEvt->partner_status);
            race_flush_packet((void *)pEvt, g_leakage_detection_race_ch_id);
        }
        g_LD_result_agent = 0;
        g_LD_result_partner = 0;

#ifdef MTK_ANC_V2
        audio_anc_leakage_detection_resume_dl();
#endif
    } else {
        //waiting the result of the other device for 1 sec
        audio_anc_leakage_detection_timer_check_result();
    }
    race_anc_mutex_give();
}

void audio_anc_leakage_detection_oneshot_timer_callback(TimerHandle_t xTimer)
{
    if (((g_LD_result_agent & 0x8000) != 0) && ((g_LD_result_partner & 0x8000) == 0)) {
        anc_leakage_detection_racecmd_response(LD_STATUS_ABSENT, 1);
    }
}

void audio_anc_leakage_detection_timer_check_result(void)
{
    S8 err_code = 0;
    if (s_xLeakageDetectionOneShotTimer == NULL) {
        s_xLeakageDetectionOneShotTimer = xTimerCreate("LeakageDetOneShot",
                                                       1000 / portTICK_PERIOD_MS,
                                                       pdFALSE,
                                                       0,
                                                       audio_anc_leakage_detection_oneshot_timer_callback);
        if (s_xLeakageDetectionOneShotTimer == NULL) {
            err_code = -1;
        } else {
            if (xTimerStart(s_xLeakageDetectionOneShotTimer, 0) != pdPASS) {
                err_code = -2;
            }
        }
    } else {
        if (xTimerReset(s_xLeakageDetectionOneShotTimer, 0) != pdPASS) {
            err_code = -3;
        }
    }
    if (err_code != 0) {
        RACE_LOG_MSGID_E("[RECORD_LC]Timer error code(%d).\r\n", 1, err_code);
    }

}

#ifdef AIR_FADP_ANC_COMPENSATION_ENABLE
static void audio_fadp_anc_compensation_oneshot_timer_callback(TimerHandle_t xTimer)
{
    RACE_LOG_MSGID_I("[RECORD_SzD]audio_anc_fadp_compensation_oneshot_timer_callback\r\n", 0);
    if ((g_SzD_result_L != LD_STATUS_FANC_PASS) && (g_SzD_result_R != LD_STATUS_FANC_PASS)) {
        RACE_LOG_MSGID_I("[RECORD_SzD]stop waiting for FADP ANC result\r\n", 0);
        audio_fadp_anc_compensation_fanc_stop(LD_STATUS_FANC_PASS);
        anc_fadp_compensation_racecmd_response(LD_STATUS_FANC_PASS, 0);
    }
}

void audio_fadp_anc_compensation_timer_check_result(void)
{
    if (s_xFADPANCCompensationOneShotTimer == NULL) {
        s_xFADPANCCompensationOneShotTimer = xTimerCreate("FADPANCOneShot",
                                                       3000 / portTICK_PERIOD_MS,
                                                       pdFALSE,
                                                       0,
                                                       audio_fadp_anc_compensation_oneshot_timer_callback);
        if (s_xFADPANCCompensationOneShotTimer == NULL) {
            RACE_LOG_MSGID_I("[RECORD_SzD]create one_shot Timer error.\n", 0);

        } else {
            if (xTimerStart(s_xFADPANCCompensationOneShotTimer, 0) != pdPASS) {
                RACE_LOG_MSGID_I("[RECORD_SzD]Timer start error.\r\n", 0);

            }
        }
    } else {
        if (xTimerReset(s_xFADPANCCompensationOneShotTimer, 0) != pdPASS) {
            RACE_LOG_MSGID_I("[RECORD_SzD]Timer reset error.\r\n", 0);

        }
    }

}

void anc_fadp_compensation_racecmd_callback(uint16_t leakage_status)
{
    RACE_LOG_MSGID_I("[RECORD_SzD] anc_leakage_detection_racecmd_callback result:%d", 1, leakage_status);

    if (leakage_status == LD_STATUS_SZD_PASS) {
        audio_fadp_anc_compensation_timer_check_result();
        audio_fadp_anc_compensation_szd_stop();
    } else if (leakage_status == LD_STATUS_FANC_PASS) {
        audio_fadp_anc_compensation_fanc_stop(LD_STATUS_FANC_PASS);
        anc_fadp_compensation_racecmd_response(LD_STATUS_FANC_PASS, 0);
    } else {
        audio_fadp_anc_compensation_stop();
        anc_fadp_compensation_racecmd_response(leakage_status, 0);
    }
}

void anc_fadp_compensation_racecmd_response(uint16_t leakage_status, uint8_t state_ctrl)
{
    typedef struct {
        adaptive_check_notify_t header;
        uint8_t agent_status;
        uint8_t partner_status;
    } PACKED leakage_detection_notify_t;

    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    race_anc_mutex_take();

    g_SzD_result_L = leakage_status;
    g_SzD_result_R = leakage_status;
    RACE_LOG_MSGID_I("[RECORD_SzD] State:%d, Get SzD result:%d, race_ch_id:%d", 3, state_ctrl, leakage_status, g_leakage_detection_race_ch_id);

    if (role != BT_AWS_MCE_ROLE_NONE) {
        leakage_detection_notify_t *pEvt = RACE_ClaimPacket(RACE_TYPE_NOTIFICATION,
                                                            RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK,
                                                            sizeof(leakage_detection_notify_t),
                                                            g_leakage_detection_race_ch_id);
        if (pEvt) {
            pEvt->header.start_or_stop     = 1;
            pEvt->header.mode              = FADP_ANC_COMPENSATION_MODE;
            pEvt->header.bit_per_sample    = 2;
            pEvt->header.channel_num       = 0;
            pEvt->header.frame_size        = 0;
            pEvt->header.seq_num           = 0;
            pEvt->header.total_data_length = 2;
            pEvt->header.data_length       = 2;
            pEvt->agent_status             = (uint8_t)(g_SzD_result_L & 0xFF);
            pEvt->partner_status           = (uint8_t)(g_SzD_result_R & 0xFF);
            RACE_LOG_MSGID_I("[RECORD_SzD] Send SzD result to APK : %d %d", 2, pEvt->agent_status, pEvt->partner_status);
            race_flush_packet((void *)pEvt, g_fadp_anc_compensation_race_ch_id);
        }
        g_SzD_result_L= 0;
        g_SzD_result_R = 0;
        if (state_ctrl == 0) {
            audio_fadp_anc_compensation_resume_dl();
        }
    }
    race_anc_mutex_give();
}
#endif

#endif
#endif

/**
 * RACE_DSPREALTIME_AUDIO_COMMON_HDR
 *
 * RACE_DSPREALTIME_AUDIO_COMMON_HDR Handler
 *
 * @pCmdMsg : pointer of ptr_race_pkt_t
 *
 */
void *RACE_DSPREALTIME_AUDIO_COMMON_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    race_dsprealtime_audio_common_struct *pEvt = NULL;

    race_dsprealtime_audio_common_struct *pAudio_common_cmd;
    pAudio_common_cmd = (race_dsprealtime_audio_common_struct *)pCmdMsg->payload;

    RACE_LOG_MSGID_I("RACE_DSPREALTIME_AUDIO_COMMON_HDR msg_length:%d payload: %d %d\n",3,pCmdMsg->hdr.length,pAudio_common_cmd->param.header.status,pAudio_common_cmd->param.header.AudioCommonId);

    if (pCmdMsg->hdr.length >= 4) { //hdr.length = hdr.id(2bytes) + pAudio_common_cmd->status(1byte) + pAudio_common_cmd->AudioCommonId(1byte) + pAudio_common_cmd->param(0/1/2/4 bytes)
        switch (pAudio_common_cmd->param.header.AudioCommonId) {
            case RACE_AUDIO_COMMON_QUERY_FEATURE: {
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_AUDIO_COMMON,
                                        sizeof(RACE_RSP_AUDIO_COMMON_QUERY_FEATURE_PARAM),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_AUDIO_COMMON_QUERY_FEATURE_PARAM));
                } else {
                    goto _HDR_END;
                }
                pEvt->param.queryCapRsp.header.AudioCommonId  = pAudio_common_cmd->param.queryCapCmd.header.AudioCommonId;
                pEvt->param.queryCapRsp.header.status         = RACE_ERRCODE_SUCCESS;

                /* Get ANC Capability. */
#if defined(MTK_ANC_ENABLE) && defined(MTK_ANC_V2)
                uint8_t support_hybrid_enable;
                audio_anc_control_get_status(NULL, NULL, NULL, NULL, &support_hybrid_enable, NULL);
#if defined(AIR_ANC_V3) && defined(AIR_FULL_ADAPTIVE_ANC_ENABLE)
                pEvt->param.queryCapRsp.CAP_ANC               = 0x02;
                pEvt->param.queryCapRsp.CAP_FADP_Monitor      = 0x01;
#else
                if (support_hybrid_enable) {
                    pEvt->param.queryCapRsp.CAP_ANC           = 0x01;
                } else {
                    pEvt->param.queryCapRsp.CAP_ANC           = 0x03;
                }
#endif
#endif
                /* Get Airothru Capability. */
#if defined(MTK_ANC_ENABLE) && defined(MTK_ANC_V2)
#if defined(AIR_ANC_V3) && defined(AIR_HW_VIVID_PT_ENABLE)
                pEvt->param.queryCapRsp.CAP_Airothru      = 0x04;
#elif defined(AIR_ANC_ADAP_PT_ENABLE)
                pEvt->param.queryCapRsp.CAP_Airothru      = 0x03;
#elif defined(AIR_HYBRID_PT_ENABLE)
                pEvt->param.queryCapRsp.CAP_Airothru      = 0x02;
#else
                pEvt->param.queryCapRsp.CAP_Airothru      = 0x01;
#endif
#endif
                /* Get HA Capability. */
#ifdef AIR_HEARING_AID_ENABLE
                pEvt->param.queryCapRsp.CAP_HA                = 0x03;
#elif defined(AIR_HEARTHROUGH_PSAP_ENABLE)
                pEvt->param.queryCapRsp.CAP_HA                = 0x02;
#elif defined(AIR_HEARTHROUGH_VIVID_PT_ENABLE)
                pEvt->param.queryCapRsp.CAP_HA                = 0x01;
#endif

                /* Get WND Capability. */
#ifdef AIR_ANC_WIND_DETECTION_ENABLE
                pEvt->param.queryCapRsp.CAP_WND               = 0x01;
#endif

                /* Get ED Capability. */
#ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
                pEvt->param.queryCapRsp.CAP_ED                = 0x01;
#endif

                /* Get AEQ Capability. */
#ifdef AIR_ADAPTIVE_EQ_ENABLE
                pEvt->param.queryCapRsp.CAP_AEQ               = 0x01;
#endif
                /* Get ANC sync mode. */
#if defined(MTK_ANC_ENABLE) && defined(MTK_ANC_V2)
                pEvt->param.queryCapRsp.CAP_ANC_SYNC_MODE     = audio_anc_control_get_attach_enable();
#endif
                break;
            }
            case RACE_AUDIO_COMMON_READ_DEBUG_REG: {
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_AUDIO_COMMON,
                                        sizeof(RACE_RSP_AUDIO_COMMON_READ_DEBUG_REG_PARAM),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_AUDIO_COMMON_READ_DEBUG_REG_PARAM));
                } else {
                    goto _HDR_END;
                }
                pEvt->param.readRegRsp.header.AudioCommonId  = pAudio_common_cmd->param.queryCapCmd.header.AudioCommonId;
                pEvt->param.readRegRsp.header.status         = RACE_ERRCODE_SUCCESS;
                pEvt->param.readRegRsp.REG_Value             = *((volatile unsigned int *)(pAudio_common_cmd->param.readRegCmd.Read_REG));
                RACE_LOG_MSGID_I("[Race] Reg read 0x%x is 0x%x", 2, pAudio_common_cmd->param.readRegCmd.Read_REG, pEvt->param.readRegRsp.REG_Value);
                break;
            }
            case RACE_AUDIO_COMMON_WRITE_DEBUG_REG: {
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_AUDIO_COMMON,
                                        sizeof(RACE_RSP_AUDIO_COMMON_WRITE_DEBUG_REG_PARAM),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_AUDIO_COMMON_WRITE_DEBUG_REG_PARAM));
                } else {
                    goto _HDR_END;
                }
                pEvt->param.writeRegRsp.header.AudioCommonId  = pAudio_common_cmd->param.queryCapCmd.header.AudioCommonId;
                pEvt->param.writeRegRsp.header.status         = RACE_ERRCODE_SUCCESS;
                *((volatile uint32_t *)(pAudio_common_cmd->param.writeRegCmd.Write_REG)) = pAudio_common_cmd->param.writeRegCmd.Write_Value;
                pEvt->param.writeRegRsp.Write_REG             = pAudio_common_cmd->param.writeRegCmd.Write_REG;
                pEvt->param.writeRegRsp.Write_Value           = *((volatile unsigned int *)(pAudio_common_cmd->param.writeRegCmd.Write_REG));
                RACE_LOG_MSGID_I("[Race] Reg set 0x%x is 0x%x", 2, pAudio_common_cmd->param.writeRegRsp.Write_REG, pEvt->param.writeRegRsp.Write_Value);
                break;
            }
            default: {
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_AUDIO_COMMON,
                                        sizeof(RACE_AUDIO_COMMON_HEADER),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_AUDIO_COMMON_HEADER));
                } else {
                    goto _HDR_END;
                }
                pEvt->param.header.status = RACE_ERRCODE_NOT_SUPPORT;
                return pEvt;
                break;
            }
        }
    } else {
        pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                RACE_DSPREALTIME_AUDIO_COMMON,
                                sizeof(RACE_AUDIO_COMMON_HEADER),
                                channel_id);
        if (pEvt != NULL) {
            memset(pEvt, 0, sizeof(RACE_AUDIO_COMMON_HEADER));
        } else {
            goto _HDR_END;
        }
        pEvt->param.header.status = RACE_ERRCODE_PARAMETER_ERROR;
    }

    if (pEvt != NULL) {
        if (pEvt->param.header.status != RACE_ERRCODE_SUCCESS) {
            RACE_LOG_MSGID_E("RACE_DSPREALTIME_AUDIO_COMMON_HDR pEvt->status: %d \n", 1, pEvt->param.header.status);
        }
    }
_HDR_END:
    return pEvt;
}

/**
 * RACE_DSPREALTIME_SUSPEND_HDR
 *
 * RACE_DSPREALTIME_SUSPEND_HDR Handler
 *
 * @pCmdMsg : pointer of ptr_race_pkt_t
 *
 */
void *RACE_DSPREALTIME_SUSPEND_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        uint8_t Status;
    } PACKED RACE_DSPREALTIME_SUSPEND_EVT_STRU;

    RACE_DSPREALTIME_SUSPEND_EVT_STRU *pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)RACE_DSPREALTIME_SUSPEND, (uint16_t)sizeof(RACE_DSPREALTIME_SUSPEND_EVT_STRU), channel_id);
    bt_sink_srv_am_result_t am_status;

    if (pEvt != NULL) {
        am_status = am_audio_set_pause();
        pEvt->Status = am_status != AUD_EXECUTION_SUCCESS ? RACE_ERRCODE_FAIL : RACE_ERRCODE_SUCCESS;
    }

#if defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
    llf_control_suspend();
#endif
    return pEvt;
}


/**
 * RACE_DSPREALTIME_RESUME_HDR
 *
 * RACE_DSPREALTIME_RESUME_HDR Handler
 *
 * @pCmdMsg : pointer of ptr_race_pkt_t
 *
 */
void *RACE_DSPREALTIME_RESUME_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        uint8_t Status;
    } PACKED RACE_DSPREALTIME_RESUME_EVT_STRU;

    RACE_DSPREALTIME_RESUME_EVT_STRU *pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)RACE_DSPREALTIME_RESUME, (uint16_t)sizeof(RACE_DSPREALTIME_RESUME_EVT_STRU), channel_id);
    bt_sink_srv_am_result_t am_status;


    if (pEvt != NULL) {
        //API
        am_status = am_audio_set_resume();
        if (am_status != AUD_EXECUTION_SUCCESS) {
            pEvt->Status = (uint8_t)RACE_ERRCODE_FAIL;
        } else {
            pEvt->Status = (uint8_t)RACE_ERRCODE_SUCCESS;
        }
    }
#if defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
        llf_control_resume();
#endif

    return pEvt;
}

#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
#if defined AIR_ADAPTIVE_EQ_ENABLE
void race_adaptive_eq_notify_to_app(uint8_t peq_PhaseAndPath,uint8_t action,uint8_t channel_id)
{
    bt_clock_t target_bt_clk = {0};
    uint8_t setting_mode = PEQ_DIRECT;
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
#ifdef MTK_AWS_MCE_ENABLE
    race_dsprt_peq_get_target_bt_clk(role, &setting_mode, &target_bt_clk);
#endif
    if ((role == BT_AWS_MCE_ROLE_AGENT) || (role == BT_AWS_MCE_ROLE_NONE)) {
        aeq_control_param_t *param = (aeq_control_param_t *)pvPortMalloc(sizeof(aeq_control_param_t));
        if (param != NULL) {
            param->type = peq_PhaseAndPath;
            param->action = action;
            param->setting_mode = setting_mode;
            param->target_bt_clk = target_bt_clk.nclk;
            param->channel_id = channel_id;
            race_send_event_notify_msg(RACE_EVENT_TYPE_AEQ_CONTROL, (void *)param);
        }else{
            configASSERT(0 && "adaptive eq notify app malloc fail");
        }
    }
}
#endif
/**
 * RACE_DSPREALTIME_PEQ_HDR
 *
 * RACE_DSPREALTIME_PEQ_HDR Handler
 *
 * @pCmdMsg : pointer of ptr_race_pkt_t
 *
 */
void *RACE_DSPREALTIME_PEQ_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint8_t peq_PhaseAndPath;    //0x00:a2dp pre peq  0x01:a2dp  post peq  0x10:LINE_IN pre peq  0x11:LINE_IN post peq  0x03:adaptive eq 0x04:usbin1 eq  0x05:mic eq 0x06 advanced record eq
        uint8_t Peq_param[1];
    } PACKED RACE_DSPREALTIME_PEQ_CMD_STRU;

    typedef struct {
        uint8_t Status;
    } PACKED RACE_DSPREALTIME_PEQ_EVT_STRU;

    RACE_DSPREALTIME_PEQ_CMD_STRU *pCmd = (RACE_DSPREALTIME_PEQ_CMD_STRU *)pCmdMsg;
    uint16_t Peq_param_length = (pCmd->Hdr.length >= 3) ? (pCmd->Hdr.length - 3) : 0; //Hdr.id: 2bytes, peq_phase: 1byte
    uint32_t ret = 1;
    RACE_LOG_MSGID_I("RACE_DSPREALTIME_PEQ_HDR Peq_param_length=%d,peq_PhaseAndPath:%d\n", 2, Peq_param_length,pCmd->peq_PhaseAndPath);

#if defined AIR_ADAPTIVE_EQ_ENABLE
    if((pCmd->peq_PhaseAndPath == PEQ_AEQ_CONTROL)||(pCmd->peq_PhaseAndPath == PEQ_AEQ_DETECT_CONTROL)){
        race_adaptive_eq_notify_to_app(pCmd->peq_PhaseAndPath,pCmd->Peq_param[0],channel_id);
        return NULL;
    }
#endif

    RACE_DSPREALTIME_PEQ_EVT_STRU *pEvt = (RACE_DSPREALTIME_PEQ_EVT_STRU *)RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)RACE_DSPREALTIME_PEQ, (uint16_t)sizeof(RACE_DSPREALTIME_PEQ_EVT_STRU), channel_id);
    if (((pCmd->peq_PhaseAndPath != 0) && (pCmd->peq_PhaseAndPath != 1) && (pCmd->peq_PhaseAndPath != 0x10) && (pCmd->peq_PhaseAndPath != 0x11) && (pCmd->peq_PhaseAndPath != 0x03)
        && (pCmd->peq_PhaseAndPath != 0x04) && (pCmd->peq_PhaseAndPath != 0x05) && (pCmd->peq_PhaseAndPath != 0x06) && (pCmd->peq_PhaseAndPath != PEQ_SPECIAL_PHASE)
        && (pCmd->peq_PhaseAndPath != PEQ_AEQ_CONTROL) && (pCmd->peq_PhaseAndPath != PEQ_AEQ_DETECT_CONTROL))
        || (((pCmd->peq_PhaseAndPath != PEQ_SPECIAL_PHASE)&&(pCmd->peq_PhaseAndPath != PEQ_AEQ_CONTROL)&&(pCmd->peq_PhaseAndPath != PEQ_AEQ_DETECT_CONTROL)) && (Peq_param_length < (5 * 1 * 2 + 2 + 1)))
        || (((pCmd->peq_PhaseAndPath == PEQ_SPECIAL_PHASE)||(pCmd->peq_PhaseAndPath == PEQ_AEQ_CONTROL)||(pCmd->peq_PhaseAndPath == PEQ_AEQ_DETECT_CONTROL)) && (Peq_param_length < 1))) {
        if (pEvt != NULL) {
            pEvt->Status = (uint8_t)RACE_ERRCODE_FAIL;
        }
        return pEvt;
    }else{
        if (pEvt != NULL) {
            pEvt->Status = (uint8_t)RACE_ERRCODE_FAIL;
        }
    }

    if (pEvt != NULL) {
        bt_clock_t target_bt_clk = {0};
        uint8_t setting_mode = PEQ_DIRECT;
        bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
        bt_sink_srv_am_type_t cur_type = NONE;
        am_feature_type_t audio_path_id = 0;

#ifdef MTK_AWS_MCE_ENABLE
        race_dsprt_peq_get_target_bt_clk(role, &setting_mode, &target_bt_clk);
#endif
        cur_type = bt_sink_srv_ami_get_current_scenario();
#ifdef AIR_WIRED_AUDIO_ENABLE
        uint8_t LINE_INENABLE = 0, LINE_OUT_ENABLE = 0, USB_IN_ENABLE = 0;
        audio_transmitter_scenario_list_t audio_transmitter_scenario_list[]  =  {
            {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN},
            {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER},
            {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0},
        };
        audio_transmitter_scenario_list_t audio_transmitter_scenario_list_out[]  =  {
            {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT},
        };

        audio_transmitter_scenario_list_t audio_transmitter_scenario_list_usb_in[]  =  {
            {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1},
        };

        LINE_INENABLE = audio_transmitter_get_is_running_by_scenario_list(audio_transmitter_scenario_list, sizeof(audio_transmitter_scenario_list) / sizeof(audio_transmitter_scenario_list_t));
        LINE_OUT_ENABLE = audio_transmitter_get_is_running_by_scenario_list(audio_transmitter_scenario_list_out, sizeof(audio_transmitter_scenario_list_out) / sizeof(audio_transmitter_scenario_list_t));
        USB_IN_ENABLE = audio_transmitter_get_is_running_by_scenario_list(audio_transmitter_scenario_list_usb_in, sizeof(audio_transmitter_scenario_list_usb_in) / sizeof(audio_transmitter_scenario_list_t));
#endif
#ifdef AIR_RECORD_ADVANCED_ENABLE
        uint8_t ADVANCED_RECORD_ENBALE = 0;
        audio_transmitter_scenario_list_t audio_transmitter_scenario_list_advanced_record[]  =  {
            {AUDIO_TRANSMITTER_ADVANCED_RECORD, AUDIO_TRANSMITTER_ADVANCED_RECORD_N_MIC},
        };
        ADVANCED_RECORD_ENBALE = audio_transmitter_get_is_running_by_scenario_list(audio_transmitter_scenario_list_advanced_record, sizeof(audio_transmitter_scenario_list_advanced_record) / sizeof(audio_transmitter_scenario_list_t));
#endif

        if ((cur_type == A2DP || cur_type == BLE || cur_type == ULL_BLE_DL) && (pCmd->peq_PhaseAndPath == 0x00 || pCmd->peq_PhaseAndPath == 0x01)) {
            audio_path_id = AM_A2DP_PEQ;
        } else if ((cur_type == LINE_IN) && (pCmd->peq_PhaseAndPath == 0x10)) {
            //compatible for old version of tool of line in realtime PEQ setting
            audio_path_id = AM_LINEIN_PEQ;
        } else if ((cur_type == A2DP || cur_type == BLE || cur_type == ULL_BLE_DL) && (pCmd->peq_PhaseAndPath == 0x03)) {
            audio_path_id = AM_AUDIO_AEQ;
#ifdef AIR_WIRED_AUDIO_ENABLE
        } else if (LINE_INENABLE == true && (pCmd->peq_PhaseAndPath == 0x10 || pCmd->peq_PhaseAndPath == 0x11)) {
            audio_path_id = AM_LINEIN_PEQ;
        } else if ((USB_IN_ENABLE == true) && (pCmd->peq_PhaseAndPath == 0x04 || pCmd->peq_PhaseAndPath == 0x10)) {
            audio_path_id = AM_USB_IN_PEQ;
        } else if (((LINE_OUT_ENABLE == true)||(ami_hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_BLE_UL)==true)) && (pCmd->peq_PhaseAndPath == 0x05)) {
            audio_path_id = AM_MIC_PEQ;
#endif
#ifdef AIR_RECORD_ADVANCED_ENABLE
        } else if ((ADVANCED_RECORD_ENBALE == true) && (pCmd->peq_PhaseAndPath == 0x06)) {
            audio_path_id = AM_ADVANCED_RECORD_PEQ;
#endif
        } else {
            audio_path_id = AM_A2DP_PEQ;//to fix return fail while nothing playing
            RACE_LOG_MSGID_E("Un-supported scenario: %d\n", 1, cur_type);
        }
        RACE_LOG_MSGID_I("Realtime PEQ peq_PhaseAndPath:%d,cur_type:%d",2,pCmd->peq_PhaseAndPath,cur_type);
        if ((role == BT_AWS_MCE_ROLE_AGENT) || (role == BT_AWS_MCE_ROLE_NONE)) {
            if (pCmd->peq_PhaseAndPath == PEQ_SPECIAL_PHASE) {
                ret = race_dsprt_peq_change_mode_data(0, setting_mode, target_bt_clk.nclk, pCmd->Peq_param[0], PEQ_SOUND_MODE_FORCE_DRC, audio_path_id);
                if (audio_path_id == AM_A2DP_PEQ) {
#ifndef MTK_ANC_ENABLE
                    ret = race_dsprt_peq_change_mode_data(1, setting_mode, target_bt_clk.nclk, pCmd->Peq_param[0], PEQ_SOUND_MODE_FORCE_DRC, audio_path_id);
#else
                    if (pCmd->Peq_param[0] == 1) {
#ifndef MTK_ANC_V2
                        uint8_t anc_enable;
                        uint8_t hybrid_enable;
                        anc_get_status(&anc_enable, NULL, &hybrid_enable);
                        if ((anc_enable > 0) && (hybrid_enable > 0)) {
                            ret = race_dsprt_peq_change_mode_data(1, setting_mode, target_bt_clk.nclk, pCmd->Peq_param[0], PEQ_SOUND_MODE_FORCE_DRC, audio_path_id);
                        } else {
                            ret = race_dsprt_peq_change_mode_data(1, setting_mode, target_bt_clk.nclk, 0, PEQ_SOUND_MODE_UNASSIGNED, audio_path_id);
                        }
#else
                        ret = race_dsprt_peq_change_mode_data(1, setting_mode, target_bt_clk.nclk, pCmd->Peq_param[0], PEQ_SOUND_MODE_FORCE_DRC, audio_path_id);
#endif
                    } else {
                        ret = race_dsprt_peq_change_mode_data(1, setting_mode, target_bt_clk.nclk, pCmd->Peq_param[0], PEQ_SOUND_MODE_FORCE_DRC, audio_path_id);
                    }
#endif
                }
            } else {
                if (pCmd->peq_PhaseAndPath == 0x00 || pCmd->peq_PhaseAndPath == 0x10) {
#ifdef AIR_A2DP_LINEIN_SEPERATE_PEQ_ENABLE
                    ret = race_dsprt_peq_realtime_data(0, setting_mode, target_bt_clk.nclk, (uint8_t *)&pCmd->Peq_param, (uint32_t)Peq_param_length, audio_path_id);
#else
                    if (cur_type == A2DP || cur_type == BLE || cur_type == ULL_BLE_DL) {
                        ret = race_dsprt_peq_realtime_data(0, setting_mode, target_bt_clk.nclk, (uint8_t *)&pCmd->Peq_param, (uint32_t)Peq_param_length, AM_A2DP_PEQ);
                    } else if (cur_type == LINE_IN) {
                        ret = race_dsprt_peq_realtime_data(0, setting_mode, target_bt_clk.nclk, (uint8_t *)&pCmd->Peq_param, (uint32_t)Peq_param_length, AM_LINEIN_PEQ);
                    } else if (cur_type == USB_AUDIO_IN) {
                        ret = race_dsprt_peq_realtime_data(0, setting_mode, target_bt_clk.nclk, (uint8_t *)&pCmd->Peq_param, (uint32_t)Peq_param_length, AM_A2DP_PEQ);
#ifdef AIR_WIRED_AUDIO_ENABLE
                    } else if (LINE_INENABLE == true) {
                        ret = race_dsprt_peq_realtime_data(0, setting_mode, target_bt_clk.nclk, (uint8_t *)&pCmd->Peq_param, (uint32_t)Peq_param_length, AM_LINEIN_PEQ);
#endif
                    }
#endif
                } else if(pCmd->peq_PhaseAndPath == 0x03){
                    ret = race_dsprt_peq_realtime_data(0, setting_mode, target_bt_clk.nclk,(uint8_t *)&pCmd->Peq_param,(uint32_t)Peq_param_length, AM_AUDIO_AEQ);
                } else if((pCmd->peq_PhaseAndPath == 0x04)||(pCmd->peq_PhaseAndPath == 0x05)||(pCmd->peq_PhaseAndPath == 0x06)){
                    ret = race_dsprt_peq_realtime_data(0, setting_mode, target_bt_clk.nclk,(uint8_t *)&pCmd->Peq_param,(uint32_t)Peq_param_length, audio_path_id);
                }
                else{
                    ret = race_dsprt_peq_realtime_data(1, setting_mode, target_bt_clk.nclk, (uint8_t *)&pCmd->Peq_param, (uint32_t)Peq_param_length, audio_path_id);
                }
            }
        } else {
            ret = 1;
        }

        if (ret == 0) {
            pEvt->Status = (uint8_t)RACE_ERRCODE_SUCCESS;
        } else {
            pEvt->Status = (uint8_t)RACE_ERRCODE_FAIL;
        }
    }

    return pEvt;
}
#endif

/**
 * RACE_DSPREALTIME_GET_REFRENCE_GAIN_HDR
 *
 * RACE_DSPREALTIME_GET_REFRENCE_GAIN_HDR Handler
 *
 * @pCmdMsg : pointer of ptr_race_pkt_t
 *
 */
void *RACE_DSPREALTIME_GET_REFRENCE_GAIN_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        uint8_t status;
    } PACKED RSP;

    int32_t ret = RACE_ERRCODE_FAIL;
    RSP *pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                 RACE_DSPREALTIME_GET_REFRENCE_GAIN,
                                 sizeof(RSP),
                                 channel_id);
    if (pEvt) {
        race_dsprealtime_get_refrence_gain_noti_struct *noti = NULL;

        /* A1. Execute the cmd. */
        uint8_t *ref_gain = (uint8_t *)hal_audio_query_race_cmd_audio_buf();

        if (ref_gain) {
            memset(ref_gain, 0, sizeof(race_dsprealtime_get_refrence_gain_noti_struct));
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_REQ_GET_REALTIME_REF_GAIN,
                                                  0,
                                                  (uint32_t)ref_gain,
                                                  true);
            /* A2. Create the noti. */
            noti = (void *)RACE_ClaimPacket(RACE_TYPE_NOTIFICATION,
                                            RACE_DSPREALTIME_GET_REFRENCE_GAIN,
                                            sizeof(race_dsprealtime_get_refrence_gain_noti_struct),
                                            channel_id);
            if (noti) {
                /* A3. Set the noti parameters with the cmd results.  */
                memcpy(noti->Data, ref_gain, sizeof(noti->Data));

                /* A4. Send the noti. */
                ret = race_noti_send(noti, channel_id, TRUE);
                if (RACE_ERRCODE_SUCCESS != ret) {
                    /* A5. Free the noti if needed. */
                    RACE_FreePacket(noti);
                    noti = NULL;
                }
            } else {
                ret = RACE_ERRCODE_NOT_ENOUGH_MEMORY;
            }
        }

        pEvt->status = ret;
    }

    return pEvt;
}

#ifdef AIR_AIRDUMP_ENABLE_MIC_RECORD
void airdump_common_timer_callback(void);
airdump_timer_result_t airdump_common_timer_create(void)
{
    airdump_timer_result_t ret = AIRDUMP_EXECUTION_SUCCESS;
    int32_t duration_ms = 8;
    if (g_airdump_common_timer == NULL) {
        g_airdump_common_timer = xTimerCreate("AirCMDumpTimer",
                                              (duration_ms / portTICK_PERIOD_MS),
                                              pdTRUE,
                                              0,
                                              (TimerCallbackFunction_t)airdump_common_timer_callback);
        if (g_airdump_common_timer == NULL) {
            ret = AIRDUMP_EXECUTION_FAIL;
        } else {
            ret = AIRDUMP_EXECUTION_SUCCESS;
        }
    }
    return ret;
}

void airdump_common_timer_delete(void)
{
    if (g_airdump_common_timer != NULL) {
        if (pdPASS == xTimerDelete(g_airdump_common_timer, 0)) {
            g_airdump_common_timer = NULL;
        }
    }
}
void airdump_common_timer_callback(void)
{
#if 0
    uint32_t curr_cnt = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
#endif
    race_dsprealtime_airdump_common_notify_struct *pRacePkt;
#if 1
    //send race cmd back to APP
    uint32_t frameSize = 128 * sizeof(uint16_t);
    uint32_t dspBufsize = audio_record_control_get_share_buf_data_byte_count();
    record_control_result_t result = RECORD_CONTROL_EXECUTION_FAIL;
    if (dspBufsize >= frameSize) {
        result = audio_record_control_read_data(&g_record_airdump_data, frameSize);
        if (result != RECORD_CONTROL_EXECUTION_SUCCESS) {
            RACE_LOG_MSGID_E("[AirDump][COMMON] audio_record_control_read_data fail\n", 0);
        }
        pRacePkt = RACE_ClaimPacket(RACE_TYPE_NOTIFICATION, (uint16_t)RACE_DSPREALTIME_AIRDUMP_ENTRY, sizeof(race_dsprealtime_airdump_common_notify_struct), g_airdump_common_race_ch_id);
        pRacePkt->airdump_id = Airdump_mic_record;
        pRacePkt->param.air_dump_mic_record_notify.Data_sequence_number = g_airdump_common_cnt;
        pRacePkt->param.air_dump_mic_record_notify.Data_size_per_sample = 2;
        pRacePkt->param.air_dump_mic_record_notify.Dump_Request = g_airdump_common_race_request;
        pRacePkt->param.air_dump_mic_record_notify.Data_length = 128;
        memcpy((int16_t *)pRacePkt->airdump_data, (int16_t *)&g_record_airdump_data, pRacePkt->param.air_dump_mic_record_notify.Data_length * sizeof(uint16_t) * 2);
        race_flush_packet((void *)pRacePkt, g_airdump_common_race_ch_id);
    } else {
        RACE_LOG_MSGID_I("[AirDump][COMMON] Airdump bufer size low %d", 1, dspBufsize);
        return;
    }
#else
    pRacePkt = RACE_ClaimPacket(RACE_TYPE_NOTIFICATION, (uint16_t)RACE_DSPREALTIME_AIRDUMP_ENTRY, sizeof(race_dsprealtime_airdump_common_notify_struct), g_airdump_common_race_ch_id);
    if (pRacePkt != NULL) {
        pRacePkt->airdump_id = Airdump_mic_record;
        pRacePkt->param.air_dump_mic_record_notify.Data_sequence_number = g_airdump_common_cnt;
        pRacePkt->param.air_dump_mic_record_notify.Data_size_per_sample = 2;
        pRacePkt->param.air_dump_mic_record_notify.Dump_Request = g_airdump_common_race_request;
        pRacePkt->param.air_dump_mic_record_notify.Data_length = 128;
        memcpy((int16_t *)pRacePkt->airdump_data, (int16_t *)&g_128_1ktone, pRacePkt->param.air_dump_mic_record_notify.Data_length * sizeof(uint16_t));
        memcpy((int16_t *)pRacePkt->airdump_data + (pRacePkt->param.air_dump_mic_record_notify.Data_length), (uint8_t *)&g_128_2ktone, pRacePkt->param.air_dump_mic_record_notify.Data_length * sizeof(uint16_t));
        race_flush_packet((void *)pRacePkt, g_airdump_common_race_ch_id);
    }
#endif
    g_airdump_common_cnt++;
#if 0
    /*For debug dump.*/
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_cnt);
    RACE_LOG_MSGID_I("[AirDump][COMMON] hisr(%d)(%d): timer end);\r\n", 2, (curr_cnt / 1000), curr_cnt);
#endif
}

/**
 * RACE_DSPREALTIME_AIRDUMP_COMMON_HDR
 *
 * RACE_DSPREALTIME_AIRDUMP_COMMON_HDR Handler
 *
 * @pCmdMsg : pointer of ptr_race_pkt_t
 *
 */
void *RACE_DSPREALTIME_AIRDUMP_COMMON_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    race_dsprealtime_airdump_common_response_struct *pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                             (uint16_t)RACE_DSPREALTIME_AIRDUMP_ENTRY,
                                                                             (uint16_t)sizeof(race_dsprealtime_airdump_common_response_struct),
                                                                             channel_id);
    g_airdump_common_race_ch_id = channel_id;
    record_control_result_t Airdump_common_ret = RECORD_CONTROL_EXECUTION_FAIL;
    if (pEvt) {
        race_dsprealtime_airdump_common_request_struct *pAirdump_common_Cmd;
        //uint16_t Airdump_common_param_length;
        pAirdump_common_Cmd = (race_dsprealtime_airdump_common_request_struct *)pCmdMsg->payload;
        memset(pEvt, 0, sizeof(race_dsprealtime_airdump_common_response_struct));
        RACE_LOG_MSGID_I("RACE_DSPREALTIME_AIRDUMP_COMMON_HDR payload: %d %d %d\n", 3, pAirdump_common_Cmd->airdump_id, pAirdump_common_Cmd->param.air_dump_mic_record_request.Dump_Enable, pAirdump_common_Cmd->param.air_dump_mic_record_request.Dump_Request);
        switch (pAirdump_common_Cmd->airdump_id) {
            case Airdump_mic_record:
                if (pAirdump_common_Cmd->param.air_dump_mic_record_request.Dump_Enable == 1) {
                    g_airdump_common_race_request = pAirdump_common_Cmd->param.air_dump_mic_record_request.Dump_Request;
                    hal_audio_set_dvfs_control(HAL_AUDIO_DVFS_MAX_SPEED, HAL_AUDIO_DVFS_LOCK);
                    /*For debug dump.*/
                    //g_dump = true;
                    g_record_airdump = true;
                    Airdump_common_ret = audio_record_control_airdump(true, pAirdump_common_Cmd->param.air_dump_mic_record_request.Dump_Request);
                    airdump_common_timer_create();
                    xTimerStart(g_airdump_common_timer, 0);
                } else if (pAirdump_common_Cmd->param.air_dump_mic_record_request.Dump_Enable == 0) {
                    Airdump_common_ret = audio_record_control_airdump(false, pAirdump_common_Cmd->param.air_dump_mic_record_request.Dump_Request);
                    airdump_common_timer_delete();
                    /*For debug dump.*/
                    //g_dump = false;
                    g_record_airdump = false;
                    hal_audio_set_dvfs_control(HAL_AUDIO_DVFS_MAX_SPEED, HAL_AUDIO_DVFS_UNLOCK);
                    g_airdump_common_cnt = 0;
                } else {
                    pEvt->status = RACE_ERRCODE_PARAMETER_ERROR;
                    return pEvt;
                }
                break;
            default:
                pEvt->status = RACE_ERRCODE_PARAMETER_ERROR;
                return pEvt;
        }
        pEvt->status = (Airdump_common_ret == RECORD_CONTROL_EXECUTION_SUCCESS) ? RACE_ERRCODE_SUCCESS : RACE_ERRCODE_FAIL;
    }
    return pEvt;
}
#endif

#ifdef AIR_AIRDUMP_ENABLE
airdump_timer_result_t airdump_timer_create(void)
{
    airdump_timer_result_t ret = AIRDUMP_EXECUTION_SUCCESS;
    int32_t duration = 20000;
    if (g_airdump_timer == NULL) {
        g_airdump_timer = xTimerCreate("AirDumpTimer",
                                       (duration / 1000 / portTICK_PERIOD_MS),
                                       pdTRUE,
                                       0,
                                       (TimerCallbackFunction_t)airdump_timer_callback);
        if (g_airdump_timer == NULL) {
            RACE_LOG_MSGID_I("[AirDump][CM4] create timer failed\n", 0);
            ret = AIRDUMP_EXECUTION_FAIL;
        } else {
            ret = AIRDUMP_EXECUTION_SUCCESS;
        }
    }
    return ret;
}

void airdump_timer_delete(void)
{
    if (g_airdump_timer != NULL) {
        if (pdPASS == xTimerDelete(g_airdump_timer, 0)) {
            g_airdump_timer = NULL;
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_AIRDUMP_EN, 0, 0, false);
        }
    }
}

void airdump_timer_callback(void)
{
    //RACE_LOG_MSGID_I("[AirDump][CM4] airdump_timer_callback start\n", 0);
    AIRDUMP_BUFFER_INFO *buffer;
    buffer = (AIRDUMP_BUFFER_INFO *)hal_audio_query_hfp_air_dump();
    uint8_t airdump_cnt_now = buffer->notify_count;
    if (g_airdump_cnt_past != airdump_cnt_now) {
        //send race cmd back to APP
        RACE_AirDump_Send((uint8_t)g_airdump_race_ch_id,
                          (uint8_t)RACE_TYPE_NOTIFICATION,
                          (uint8_t *)(&buffer->data[0] + buffer->read_offset),
                          (uint16_t)sizeof(RACE_DSPREALTIME_AIRDUMP_EVT_NOTI_STRU));

        uint32_t ro = (buffer->read_offset + AEC_AIRDUMP_FRAME_SIZE * AEC_AIRDUMP_FRAME_NUM_HALF) % buffer->length; // move frame size
        buffer->read_offset = ro;
        //RACE_LOG_MSGID_I("[AirDump][CM4] update ro: %d, wo:%d, cnt_past:%d, cnt_now:%d",2,buffer->read_offset, buffer->write_offset, g_airdump_cnt_past, airdump_cnt_now);
        g_airdump_cnt_past = airdump_cnt_now;
    }
}

void RACE_AirDump_Send(uint8_t channelId, uint8_t RaceType, uint8_t *ptr, uint16_t len)
{
    RACE_DSPREALTIME_AIRDUMP_EVT_NOTI_STRU *pRacePkt = RACE_ClaimPacket(RaceType, (uint16_t)RACE_DSPREALTIME_AIRDUMP_ON_OFF, len, channelId);
    if (pRacePkt != NULL) {
        memcpy((uint8_t *)pRacePkt->data, ptr, len);
        race_flush_packet((void *)pRacePkt, g_airdump_race_ch_id);
    }
}

/**
 * RACE_DSPREALTIME_AIRDUMP_HDR
 *
 * RACE_DSPREALTIME_AIRDUMP_HDR Handler
 *
 * @pCmdMsg : pointer of ptr_race_pkt_t
 *
 */
void *RACE_DSPREALTIME_AIRDUMP_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    g_airdump_race_ch_id = channel_id;
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint8_t AirDumpEn;
    } PACKED RACE_DSPREALTIME_AIRDUMP_CMD_STRU;

    typedef struct {
        uint8_t status;
    } PACKED RACE_DSPREALTIME_AIRDUMP_EVT_RES_STRU;

    bt_bd_addr_t addr;
    uint32_t num = bt_cm_get_connected_devices(~BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS), &addr, 1);
    bt_gap_connection_handle_t gap_handle = bt_cm_get_gap_handle(addr);
    bt_gap_link_policy_setting_t setting;

    RACE_DSPREALTIME_AIRDUMP_EVT_RES_STRU *pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                   (uint16_t)RACE_DSPREALTIME_AIRDUMP_ON_OFF,
                                                                   (uint16_t)sizeof(RACE_DSPREALTIME_AIRDUMP_EVT_RES_STRU),
                                                                   channel_id);
    if (pEvt) {
        RACE_DSPREALTIME_AIRDUMP_CMD_STRU *pCmd = (RACE_DSPREALTIME_AIRDUMP_CMD_STRU *)pCmdMsg;
        if (pCmd) {
            if (pCmd->AirDumpEn == 1) {
                setting.sniff_mode = BT_GAP_LINK_POLICY_DISABLE;
                if (num > 0) {
                    bt_gap_exit_sniff_mode(gap_handle);
                    bt_gap_write_link_policy(gap_handle, &setting);
                }
                bt_sink_srv_am_type_t type = bt_sink_srv_ami_get_current_scenario();
                if (type == HFP || type == BLE) {
                    airdump_timer_create();
                    xTimerStart(g_airdump_timer, 0);
                    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_AIRDUMP_EN, 0, pCmd->AirDumpEn, false);
                } else {
                    RACE_LOG_MSGID_I("[RACE_DSPREALTIME_AIRDUMP_HDR] Please Start eSCO/LE call first", 0);
                }
            } else if (pCmd->AirDumpEn == 0) {
                airdump_timer_delete();
                //hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_AIRDUMP_EN, 0, pCmd->AirDumpEn, false);
                setting.sniff_mode = BT_GAP_LINK_POLICY_ENABLE ;
                bt_gap_write_link_policy(gap_handle, &setting);
            }
        }

        pEvt->status = (uint8_t)RACE_ERRCODE_SUCCESS;
    }

    return pEvt;
}
#endif

/**
 * RACE_DSPREALTIME_AUDIO_DUMP_HDR
 *
 * RACE_DSPREALTIME_AUDIO_DUMP_HDR Handler
 *
 * @pCmdMsg : pointer of ptr_race_pkt_t
 *
 */
void *RACE_DSPREALTIME_AUDIO_DUMP_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint16_t AudioDumpID;
        uint8_t  Enable;
    } PACKED RACE_DSPREALTIME_AUDIO_DUMP_CMD_STRU;

    typedef struct {
        uint8_t status;
    } PACKED RACE_DSPREALTIME_AUDIO_DUMP_EVT_RES_STRU;

    RACE_DSPREALTIME_AUDIO_DUMP_EVT_RES_STRU *pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                   (uint16_t)RACE_DSPREALTIME_AUDIO_DUMP_ON_OFF,
                                                                   (uint16_t)sizeof(RACE_DSPREALTIME_AUDIO_DUMP_EVT_RES_STRU),
                                                                   channel_id);
    if (pEvt) {
        pEvt->status = (uint8_t)RACE_ERRCODE_FAIL;
        RACE_DSPREALTIME_AUDIO_DUMP_CMD_STRU *pCmd = (RACE_DSPREALTIME_AUDIO_DUMP_CMD_STRU *)pCmdMsg;
        if(pCmd) {
            if(pCmd->AudioDumpID){
                U32 CfgValue = 0;
                CfgValue |= pCmd->AudioDumpID;
                CfgValue |= (pCmd->Enable << 16);
                RACE_LOG_MSGID_I("[RACE_DSPREALTIME_AUDIO_DUMP_HDR] dump id:%d*(0x%x), enable:%d, cfgVaue:0x%x", 4, pCmd->AudioDumpID, pCmd->AudioDumpID, pCmd->Enable, CfgValue);
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_AUDIO_DUMP_MASK, 3, CfgValue, false);
                pEvt->status = (uint8_t)RACE_ERRCODE_SUCCESS;
            }
        }
        return pEvt;
    }
    return NULL;
}

/*static mic_swap_e g_mic_swap_array[] = {
    MIC_L_ONLY,
    MIC_R_ONLY,
    MIC_3_ONLY,
    MIC_4_ONLY,
    MIC_5_ONLY,
    MIC_6_ONLY,
};*/

/**
 * RACE_DSPREALTIME_2MIC_SWAP_HDR
 *
 * RACE_DSPREALTIME_2MIC_SWAP_HDR Handler
 *
 * @pCmdMsg : pointer of ptr_race_pkt_t
 *
 */
void *RACE_DSPREALTIME_2MIC_SWAP_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint8_t MicSwapSel;
    } PACKED RACE_DSPREALTIME_MIC_SWAP_CMD_STRU;

    typedef struct {
        uint8_t status;
    } PACKED RACE_DSPREALTIME_MIC_SWAP_EVT_RES_STRU;

    RACE_DSPREALTIME_MIC_SWAP_EVT_RES_STRU *pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                    (uint16_t)RACE_DSPREALTIME_2MIC_SWAP,
                                                                    (uint16_t)sizeof(RACE_DSPREALTIME_MIC_SWAP_EVT_RES_STRU),
                                                                    channel_id);
    mic_swap_e mic_select;
    if (pEvt) {
        RACE_DSPREALTIME_MIC_SWAP_CMD_STRU *pCmd = (RACE_DSPREALTIME_MIC_SWAP_CMD_STRU *)pCmdMsg;

        if (pCmd) {
            if ((pCmd->MicSwapSel <= 5) || (pCmd->MicSwapSel==0xFF)) {
                mic_select = pCmd->MicSwapSel;
                if (mic_select != MIC_NOT_USED) {
                    mic_select += MIC_INDEX_START;
                }
                RACE_LOG_MSGID_I("[RACE_DSPREALTIME_MIC_SWAP] SWAP MIC CMD:%d MicSwapSel:%d", 2, pCmd->MicSwapSel, mic_select);
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, 1, mic_select, false);
            } else {
                RACE_LOG_MSGID_I("[RACE_DSPREALTIME_MIC_SWAP] SWAP to Multi MIC %d wrong number!!!", 1, pCmd->MicSwapSel);
            }
        }

        pEvt->status = (uint8_t)RACE_ERRCODE_SUCCESS;
    }

    return pEvt;
}

/**
 * RACE_DSPREALTIME_2MIC_SWAP_HDR_EXTEND
 *
 * RACE_DSPREALTIME_2MIC_SWAP_HDR_EXTEND Handler
 *
 * @pCmdMsg : pointer of ptr_race_pkt_t
 *
 */
void *RACE_DSPREALTIME_2MIC_SWAP_HDR_EXTEND(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint8_t MicSwapScenario;
        uint8_t MicSwapSel;
    } PACKED RACE_DSPREALTIME_MIC_SWAP_CMD_STRU;

    typedef struct {
        uint8_t status;
    } PACKED RACE_DSPREALTIME_MIC_SWAP_EVT_RES_STRU;

    RACE_DSPREALTIME_MIC_SWAP_EVT_RES_STRU *pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                    (uint16_t)RACE_DSPREALTIME_2MIC_SWAP,
                                                                    (uint16_t)sizeof(RACE_DSPREALTIME_MIC_SWAP_EVT_RES_STRU),
                                                                    channel_id);
    if (pEvt) {
        RACE_DSPREALTIME_MIC_SWAP_CMD_STRU *pCmd = (RACE_DSPREALTIME_MIC_SWAP_CMD_STRU *)pCmdMsg;

        if (pCmd) {
            if (pCmd->MicSwapSel == 1) {
                RACE_LOG_MSGID_I("[RACE_DSPREALTIME_MIC_SWAP] SWAP to R MIC, MicSwapScenario: %d, MicSwapSel:%d", 2, pCmd->MicSwapScenario, pCmd->MicSwapSel);
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, pCmd->MicSwapScenario, MIC_R_ONLY, false);
            } else if (pCmd->MicSwapSel == 0) {
                RACE_LOG_MSGID_I("[RACE_DSPREALTIME_MIC_SWAP] SWAP to L MIC, MicSwapScenario: %d, MicSwapSel:%d", 2, pCmd->MicSwapScenario, pCmd->MicSwapSel);
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, pCmd->MicSwapScenario, MIC_L_ONLY, false);
            }
#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE)
            else if (pCmd->MicSwapSel == 2) {
                RACE_LOG_MSGID_I("[RACE_DSPREALTIME_MIC_SWAP] SWAP to Multi MIC, MicSwapScenario: %d, MicSwapSel:%d", 2, pCmd->MicSwapScenario, pCmd->MicSwapSel);
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, pCmd->MicSwapScenario, MIC_3_ONLY, false);
            } else if (pCmd->MicSwapSel == 3) {
                RACE_LOG_MSGID_I("[RACE_DSPREALTIME_MIC_SWAP] SWAP to Multi MIC, MicSwapScenario: %d, MicSwapSel:%d", 2, pCmd->MicSwapScenario, pCmd->MicSwapSel);
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, pCmd->MicSwapScenario, MIC_4_ONLY, false);
            } else if (pCmd->MicSwapSel == 4) {
                RACE_LOG_MSGID_I("[RACE_DSPREALTIME_MIC_SWAP] SWAP to Multi MIC, MicSwapScenario: %d, MicSwapSel:%d", 2, pCmd->MicSwapScenario, pCmd->MicSwapSel);
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, pCmd->MicSwapScenario, MIC_5_ONLY, false);
            } else if (pCmd->MicSwapSel == 5) {
                RACE_LOG_MSGID_I("[RACE_DSPREALTIME_MIC_SWAP] SWAP to Multi MIC, MicSwapScenario: %d, MicSwapSel:%d", 2, pCmd->MicSwapScenario, pCmd->MicSwapSel);
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_CHANGE_DSP_CHANEL, pCmd->MicSwapScenario, MIC_6_ONLY, false);
            }
#endif
            else {
                RACE_LOG_MSGID_I("[RACE_DSPREALTIME_MIC_SWAP] SWAP to Multi MIC %d %d wrong number!!!", 2, pCmd->MicSwapScenario, pCmd->MicSwapSel);
            }
        }

        pEvt->status = (uint8_t)RACE_ERRCODE_SUCCESS;
    }

    return pEvt;
}

/**
 * RACE_DSPREALTIME_OPEN_ALL_MIC_HDR
 *
 * RACE_DSPREALTIME_OPEN_ALL_MIC_HDR Handler
 *
 * @pCmdMsg : pointer of ptr_race_pkt_t
 * @channel_id : race cmd channel id
 *
 */
void *RACE_DSPREALTIME_OPEN_ALL_MIC_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint8_t OpenAllMic;
    } PACKED RACE_DSPREALTIME_OPEN_ALL_MIC_CMD_STRU;

    typedef struct {
        uint8_t status;
    } PACKED RACE_DSPREALTIME_OPEN_ALL_MIC_EVT_RES_STRU;

    RACE_DSPREALTIME_OPEN_ALL_MIC_EVT_RES_STRU *pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                        (uint16_t)RACE_DSPREALTIME_OPEN_ALL_MIC,
                                                                        (uint16_t)sizeof(RACE_DSPREALTIME_OPEN_ALL_MIC_EVT_RES_STRU),
                                                                        channel_id);
    if (pEvt) {
        RACE_DSPREALTIME_OPEN_ALL_MIC_CMD_STRU *pCmd = (RACE_DSPREALTIME_OPEN_ALL_MIC_CMD_STRU *)pCmdMsg;

        if (pCmd) {
            if (pCmd->OpenAllMic == 1) {
                if (AUD_EXECUTION_SUCCESS != ami_set_audio_device(STREAM_IN, AU_DSP_VOICE, HAL_AUDIO_DEVICE_MAIN_MIC_DUAL, HAL_AUDIO_INTERFACE_1, NOT_REWRITE)) {
                    RACE_LOG_MSGID_E("[RACE_DSPREALTIME_OPEN_ALL_MIC] Open All Mic Fail!!!!!!", 0);
                }
            } else if (pCmd->OpenAllMic == 0) {
                /*if(AUD_EXECUTION_FAIL == audio_nvdm_configure_init()){
                    RACE_LOG_MSGID_E("RACE_DSPREALTIME_OPEN_ALL_MIC]] Default Mic Setting, OpenAllMic Faill!!!!!!",0);
                }*/
            }
        }

        pEvt->status = (uint8_t)RACE_ERRCODE_SUCCESS;
    }

    return pEvt;
}
#if defined(HAL_AUDIO_MODULE_ENABLED)
/**
 * RACE_DSPREALTIME_OPEN_ALL_MIC_HDR_EXTEND
 *
 * RACE_DSPREALTIME_OPEN_ALL_MIC_HDR_EXTEND Handler
 *
 * @pCmdMsg : pointer of ptr_race_pkt_t
 * @channel_id : race cmd channel id
 *
 */
void *RACE_DSPREALTIME_OPEN_ALL_MIC_HDR_EXTEND(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        multic_mic_config_param_t mic_param;
    } PACKED RACE_DSPREALTIME_OPEN_ALL_MIC_EXTEND_CMD_STRU;

    typedef struct {
        uint8_t status;
    } PACKED RACE_DSPREALTIME_OPEN_ALL_MIC_EXTEND_EVT_RES_STRU;

    RACE_DSPREALTIME_OPEN_ALL_MIC_EXTEND_EVT_RES_STRU *pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                               (uint16_t)RACE_DSPREALTIME_OPEN_ALL_MIC_EXTEND,
                                                                               (uint16_t)sizeof(RACE_DSPREALTIME_OPEN_ALL_MIC_EXTEND_EVT_RES_STRU),
                                                                               channel_id);
    if (pEvt) {
        RACE_DSPREALTIME_OPEN_ALL_MIC_EXTEND_CMD_STRU *pCmd = (RACE_DSPREALTIME_OPEN_ALL_MIC_EXTEND_CMD_STRU *)pCmdMsg;

        if (pCmd) {
            if (pCmd->Hdr.length != 8) {
                pEvt->status = (uint8_t)RACE_ERRCODE_FAIL;
                return pEvt;
            }
            if (ami_set_audio_device_extend(STREAM_IN, AU_DSP_VOICE, &pCmd->mic_param, false) != AUD_EXECUTION_SUCCESS) {
                RACE_LOG_MSGID_E("[RACE_DSPREALTIME_OPEN_ALL_MIC_HDR_EXTEND] Open Mic Extend Fail!", 0);
            }
        }
        pEvt->status = (uint8_t)RACE_ERRCODE_SUCCESS;
    }

    return pEvt;
}
#endif

/**
 * RACE_DSPREALTIME_AECNR_ON_OFF_HDR
 *
 * RACE_DSPREALTIME_AECNR_ON_OFF_HDR Handler
 *
 * @pCmdMsg : pointer of ptr_race_pkt_t
 *
 */
void *RACE_DSPREALTIME_AECNR_ON_OFF_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint8_t AecNrEn;
    } PACKED RACE_DSPREALTIME_AECNR_ON_OFF_CMD_STRU;

    typedef struct {
        uint8_t status;
    } PACKED RACE_DSPREALTIME_AECNR_ON_OFF_EVT_RES_STRU;

    RACE_DSPREALTIME_AECNR_ON_OFF_EVT_RES_STRU *pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                        (uint16_t)RACE_DSPREALTIME_AECNR_ON_OFF,
                                                                        (uint16_t)sizeof(RACE_DSPREALTIME_AECNR_ON_OFF_EVT_RES_STRU),
                                                                        channel_id);
    if (pEvt) {
        RACE_DSPREALTIME_AECNR_ON_OFF_CMD_STRU *pCmd = (RACE_DSPREALTIME_AECNR_ON_OFF_CMD_STRU *)pCmdMsg;

        if (pCmd) {
            if (pCmd->AecNrEn == 1) {
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_AEC_NR_EN, 0, AEC_NR_ENABLE, false);
            } else if (pCmd->AecNrEn == 0) {
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_AEC_NR_EN, 0, AEC_NR_DISABLE, false);
            }
        }

        pEvt->status = (uint8_t)RACE_ERRCODE_SUCCESS;
    }

    return pEvt;
}

/**
 * RACE_DSPREALTIME_AECNR_AVC_ON_OFF_HDR
 *
 * RACE_DSPREALTIME_AECNR_AVC_ON_OFF_HDR Handler
 *
 * @pCmdMsg : pointer of ptr_race_pkt_t
 *
 */
void *RACE_DSPREALTIME_AECNR_AVC_ON_OFF_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint8_t AvcEn;
    } PACKED RACE_DSPREALTIME_AECNR_AVC_ON_OFF_CMD_STRU;

    typedef struct {
        uint8_t status;
    } PACKED RACE_DSPREALTIME_AECNR_AVC_ON_OFF_EVT_RES_STRU;

    RACE_DSPREALTIME_AECNR_AVC_ON_OFF_EVT_RES_STRU *pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                        (uint16_t)RACE_DSPREALTIME_AECNR_AVC_ON_OFF,
                                                                        (uint16_t)sizeof(RACE_DSPREALTIME_AECNR_AVC_ON_OFF_EVT_RES_STRU),
                                                                        channel_id);
    if (pEvt) {
        RACE_DSPREALTIME_AECNR_AVC_ON_OFF_CMD_STRU *pCmd = (RACE_DSPREALTIME_AECNR_AVC_ON_OFF_CMD_STRU *)pCmdMsg;

        if (pCmd) {
            RACE_LOG_MSGID_I("[Race Cmd] AVC On/Off:%d", 1, pCmd->AvcEn);
            extern void voice_set_avc_enable(bool enable);
            voice_set_avc_enable(pCmd->AvcEn);
            pEvt->status = (uint8_t)RACE_ERRCODE_SUCCESS;
        }   
    }
    return pEvt;
}

#ifdef AIR_PSAP_ENABLE
/**
 * RACE_DSPREALTIME_SET_PSAP_MODE_HDR
 *
 * RACE_DSPREALTIME_SET_PSAP_MODE_HDR Handler
 *
 * @pCmdMsg : pointer of ptr_race_pkt_t
 *
 */
void *RACE_DSPREALTIME_SET_PSAP_MODE_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint8_t scenario_type;
        uint8_t psap_mode;
    } PACKED RACE_DSPREALTIME_SET_PSAP_MODE_CMD_STRU;

    typedef struct {
        uint8_t status;
        uint8_t scenario_type;
    } PACKED RACE_DSPREALTIME_SET_PSAP_MODE_EVT_RES_STRU;

    RACE_DSPREALTIME_SET_PSAP_MODE_EVT_RES_STRU *pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                        (uint16_t)RACE_DSPREALTIME_SET_PSAP_MODE,
                                                                        (uint16_t)sizeof(RACE_DSPREALTIME_SET_PSAP_MODE_EVT_RES_STRU),
                                                                        channel_id);
    if (pEvt) {
        RACE_DSPREALTIME_SET_PSAP_MODE_CMD_STRU *pCmd = (RACE_DSPREALTIME_SET_PSAP_MODE_CMD_STRU *)pCmdMsg;

        if (pCmd) {
            if (pCmd->scenario_type == 1) {
                hfp_set_feature_mode(pCmd->psap_mode);
            }else if(pCmd->scenario_type == 2){ //LE CALL
#ifdef AIR_BLE_FEATURE_MODE_ENABLE
                ble_set_feature_mode(pCmd->psap_mode);
#endif
            } else if (pCmd->scenario_type == 3 || pCmd->scenario_type == 4) {
                music_set_feature_mode(pCmd->psap_mode);
            }
            pEvt->scenario_type = pCmd->scenario_type;
        } else {
            pEvt->scenario_type = 0xFF;
        }

        pEvt->status = (uint8_t)RACE_ERRCODE_SUCCESS;
    }

    return pEvt;
}

/**
 * RACE_DSPREALTIME_GET_PSAP_MODE_HDR
 *
 * RACE_DSPREALTIME_GET_PSAP_MODE_HDR Handler
 *
 * @pCmdMsg : pointer of ptr_race_pkt_t
 *
 */
void *RACE_DSPREALTIME_GET_PSAP_MODE_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        uint8_t status;
        uint8_t scenario_type;
        uint8_t psap_mode;
    } PACKED RSP;
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint8_t scenario_type;
    } PACKED RACE_DSPREALTIME_GET_PSAP_MODE_CMD_STRU;

    int32_t ret = RACE_ERRCODE_FAIL;
    RSP *pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                 RACE_DSPREALTIME_GET_PSAP_MODE,
                                 sizeof(RSP),
                                 channel_id);
    if (pEvt) {
        RACE_DSPREALTIME_GET_PSAP_MODE_CMD_STRU *pCmd;

        /* A1. Execute the cmd. */
        pCmd = (RACE_DSPREALTIME_GET_PSAP_MODE_CMD_STRU *)pCmdMsg;
        if (pCmd != NULL) {
            if (pCmd->scenario_type == 1) {
                pEvt->psap_mode = hfp_get_feature_mode();
                ret = RACE_ERRCODE_SUCCESS;
            }else if(pCmd->scenario_type == 2){
#ifdef AIR_BLE_FEATURE_MODE_ENABLE
                pEvt->psap_mode = ble_get_feature_mode();
#endif
                ret = RACE_ERRCODE_SUCCESS;
            } else if (pCmd->scenario_type == 3 || pCmd->scenario_type == 4) {
                pEvt->psap_mode = music_get_feature_mode();
                ret = RACE_ERRCODE_SUCCESS;
            }
            pEvt->scenario_type = pCmd->scenario_type;
        } else {
            pEvt->scenario_type = 0xFF;
        }
    }
    pEvt->status = ret;

    return pEvt;
}
#endif

#ifdef AIR_ADVANCED_PASSTHROUGH_ENABLE_V2
void *RACE_DSPREALTIME_SELF_FITTING_CONFIG_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        uint16_t config_type;
        uint8_t  data[0];
    } PACKED RACE_DSPREALTIME_SELF_FITTING_CONTROL_CMD;

    s_self_fitting_race_ch_id = channel_id;

    uint16_t race_id    = pCmdMsg->hdr.id;
    uint16_t data_lenth = pCmdMsg->hdr.length - 4; //4 bytes = race_id(2 bytes) + config_type(2 bytes);

    RACE_DSPREALTIME_SELF_FITTING_CONTROL_CMD *pCmd = (RACE_DSPREALTIME_SELF_FITTING_CONTROL_CMD *)pCmdMsg->payload;

    self_fitting_config_cmd_t *param = (self_fitting_config_cmd_t *)pvPortMalloc(sizeof(self_fitting_config_cmd_t) + data_lenth);
    if (param != NULL) {
        param->config_type = pCmd->config_type;
        param->data_lenth  = data_lenth;
        memcpy(param->data, pCmd->data, data_lenth);
        if (race_id == RACE_DSPREALTIME_SET_SELF_FITTING_CONFIG) {
            race_send_event_notify_msg(RACE_EVENT_TYPE_SET_SELF_FITTING_CONFIG, (void *)param);
        } else {
            race_send_event_notify_msg(RACE_EVENT_TYPE_GET_SELF_FITTING_CONFIG, (void *)param);
        }
    }

    return NULL;
}


#endif

/**
 * RACE_DSPREALTIME_QUERY_LIB_VERSION_HDR
 *
 * RACE_DSPREALTIME_QUERY_LIB_VERSION_HDR Handler
 *
 * @pCmdMsg : pointer of ptr_race_pkt_t
 *
 */
void *RACE_DSPREALTIME_QUERY_LIB_VERSION_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        uint8_t status;
    } PACKED RSP;
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint8_t lib_type;
    } PACKED RACE_DSPREALTIME_QUERY_LIB_VERSION_CMD_STRU;

    int32_t ret = RACE_ERRCODE_FAIL;
    RSP *pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                 RACE_DSPREALTIME_QUERY_LIB_VERSION,
                                 sizeof(RSP),
                                 channel_id);
    if (pEvt) {
        race_dsprealtime_query_lib_version_noti_struct_t *ver_buf;
        race_dsprealtime_query_lib_version_noti_struct_t *noti = NULL;
        RACE_DSPREALTIME_QUERY_LIB_VERSION_CMD_STRU *pCmd;

        /* A1. Execute the cmd. */
        ver_buf = (race_dsprealtime_query_lib_version_noti_struct_t *)hal_audio_query_race_cmd_audio_buf();

        if (ver_buf) {
            pCmd = (RACE_DSPREALTIME_QUERY_LIB_VERSION_CMD_STRU *)pCmdMsg;
            if (pCmd != NULL) {
                ver_buf->type = pCmd->lib_type;
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_REQ_GET_REALTIME_LIB_VERSION,
                                                      0,
                                                      (uint32_t)ver_buf,
                                                      true);
                /* A2. Create the noti. */
                noti = (void *)RACE_ClaimPacket(RACE_TYPE_NOTIFICATION,
                                                RACE_DSPREALTIME_QUERY_LIB_VERSION,
                                                sizeof(race_dsprealtime_query_lib_version_noti_struct_t),
                                                channel_id);
                if (noti) {
                    /* A3. Set the noti parameters with the cmd results.  */
                    memcpy((uint8_t *)noti, (uint8_t *)ver_buf, 8 + ver_buf->length);

                    /* A4. Send the noti. */
                    ret = race_noti_send(noti, channel_id, TRUE);
                    if (RACE_ERRCODE_SUCCESS != ret) {
                        /* A5. Free the noti if needed. */
                        RACE_FreePacket(noti);
                        noti = NULL;
                    }
                } else {
                    ret = RACE_ERRCODE_NOT_ENOUGH_MEMORY;
                }
            }
        }

        pEvt->status = ret;
    }

    return pEvt;
}

/**
 * RACE_DSPREALTIME_SWGAIN_BYPASS_HDR
 *
 * RACE_DSPREALTIME_SWGAIN_BYPASS_HDR Handler
 *
 * @pCmdMsg : pointer of ptr_race_pkt_t
 *
 */
void *RACE_DSPREALTIME_SWGAIN_EN_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint8_t SW_GAIN_EN;
    } PACKED RACE_DSPREALTIME_SWGAIN_EN_CMD_STRU;

    typedef struct {
        uint8_t status;
    } PACKED RACE_DSPREALTIME_SWGAIN_EN_EVT_RES_STRU;

    RACE_DSPREALTIME_SWGAIN_EN_EVT_RES_STRU *pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                     (uint16_t)RACE_DSPREALTIME_SWGAIN_BYPASS,
                                                                     (uint16_t)sizeof(RACE_DSPREALTIME_SWGAIN_EN_EVT_RES_STRU),
                                                                     channel_id);
    if (pEvt) {
        RACE_DSPREALTIME_SWGAIN_EN_CMD_STRU *pCmd = (RACE_DSPREALTIME_SWGAIN_EN_CMD_STRU *)pCmdMsg;

        if (pCmd) {
            if (pCmd->SW_GAIN_EN == 0) {
                RACE_LOG_MSGID_I("[RACE_DSPREALTIME_SWGAIN_BYPASS] Bypass SW Gain, ByPassStatus:%d\n", 1, pCmd->SW_GAIN_EN);
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SW_GAIN_EN, 0, SW_GAIN_DISABLE, false);
            } else if (pCmd->SW_GAIN_EN == 1) {
                RACE_LOG_MSGID_I("[RACE_DSPREALTIME_SWGAIN_BYPASS] Don't Bypass SW Gain, ByPassStatus:%d\n", 1, pCmd->SW_GAIN_EN);
                hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_SW_GAIN_EN, 0, SW_GAIN_ENABLE, false);
            }
        }

        pEvt->status = (uint8_t)RACE_ERRCODE_SUCCESS;
    }

    return pEvt;
}

#ifdef MTK_ANC_ENABLE
#ifndef MTK_ANC_V2
/**
 * RACE_DSPREALTIME_ANC_HDR
 *
 * RACE_DSPREALTIME_ANC_HDR Handler
 *
 * @pCmdMsg : pointer of ptr_race_pkt_t
 *
 */
void *RACE_DSPREALTIME_ANC_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    race_dsprealtime_anc_struct *pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                                         RACE_DSPREALTIME_ANC,
                                                         sizeof(race_dsprealtime_anc_struct),
                                                         channel_id);
    g_anc_race_ch_id = channel_id;
    if (pEvt) {
        race_dsprealtime_anc_struct *pAnc_cmd;
        anc_control_result_t anc_ret = ANC_CONTROL_EXECUTION_NONE;
        uint16_t anc_param_length;

        pAnc_cmd = (race_dsprealtime_anc_struct *)pCmdMsg->payload;
        memset(pEvt, 0, sizeof(race_dsprealtime_anc_struct));
        pEvt->anc_id = pAnc_cmd->anc_id;

        if (pCmdMsg->hdr.length >= 4) { //hdr.length = hdr.id(2bytes) + pAnc_cmd->status(1byte) + pAnc_cmd->anc_id(1byte) + pAnc_cmd->param(0/1/2/4 bytes)
            anc_param_length = pCmdMsg->hdr.length - 4;
            switch (pAnc_cmd->anc_id) {
                case RACE_ANC_ON:
                    if (anc_param_length > 1) {
                        anc_filter_type_t filter_type = (anc_filter_type_t)pAnc_cmd->param.anc_on_param.anc_filter_type;
                        pEvt->param.anc_on_param = pAnc_cmd->param.anc_on_param;
                        if (pAnc_cmd->param.anc_on_param.anc_mode == 1) {
                            filter_type |= ANC_FF_ONLY_BIT_MASK;
                        } else if (pAnc_cmd->param.anc_on_param.anc_mode == 2) {
                            filter_type |= ANC_FB_ONLY_BIT_MASK;
                        }
                        race_anc_mutex_take();
                        g_anc_notify_on = ((uint32_t)filter_type << 8);
                        g_anc_notify_off = 0;
                        race_anc_mutex_give();
                        anc_ret = audio_anc_enable(filter_type, ANC_UNASSIGNED_GAIN, NULL);
                    } else if (anc_param_length > 0) {
                        anc_filter_type_t filter_type = (anc_filter_type_t)pAnc_cmd->param.anc_on_param.anc_filter_type;
                        pEvt->param.anc_on_param.anc_filter_type = pAnc_cmd->param.anc_on_param.anc_filter_type;
                        race_anc_mutex_take();
                        g_anc_notify_on = ((uint32_t)filter_type << 8);
                        g_anc_notify_off = 0;
                        race_anc_mutex_give();
                        anc_ret = audio_anc_enable(filter_type, ANC_UNASSIGNED_GAIN, NULL);
                    }
                    break;
                case RACE_ANC_OFF:
                    race_anc_mutex_take();
                    g_anc_notify_off |= (1 << 31);
                    g_anc_notify_on = 0;
                    race_anc_mutex_give();
                    anc_ret = audio_anc_disable(NULL);
                    if (anc_get_sync_time() == 0) {
                        RACE_FreePacket(pEvt);
                        return NULL;
                    }
                    break;
                case RACE_ANC_SET_VOL:
                    if (anc_param_length >= 4) {
                        pEvt->param.gain.val = pAnc_cmd->param.gain.val;
                        anc_ret = audio_anc_set_volume(pAnc_cmd->param.gain, TO_BOTH);
                    }
                    break;
                case RACE_ANC_SET_RUNTIME_VOL:
                    if (anc_param_length >= 2) {
                        pEvt->param.runtime_gain = pAnc_cmd->param.runtime_gain;
                        anc_ret = audio_anc_set_runtime_volume(pAnc_cmd->param.runtime_gain);
                    }
                    break;
                case RACE_ANC_READ_NVDM:
                    anc_ret = audio_anc_read_volume_nvdm(&pEvt->param.gain, TO_AGENT);
                    break;
                case RACE_ANC_WRITE_NVDM:
                    if (anc_param_length >= 4) {
                        pEvt->param.gain.val = pAnc_cmd->param.gain.val;
                        anc_ret = audio_anc_write_volume_nvdm(&pAnc_cmd->param.gain, TO_AGENT);
                    }
                    break;
#ifdef MTK_AWS_MCE_ENABLE
                case RACE_ANC_SET_AGENT_VOL:
                    if (anc_param_length >= 4) {
                        pEvt->param.gain.val = pAnc_cmd->param.gain.val;
                        race_anc_mutex_take();
                        g_anc_set_agent_vol = pAnc_cmd->param.gain.val;
                        race_anc_mutex_give();
                        anc_ret = audio_anc_set_volume(pAnc_cmd->param.gain, TO_AGENT);
                        if (anc_get_sync_time() == 0) {
                            RACE_FreePacket(pEvt);
                            return NULL;
                        }
                    }
                    break;
                case RACE_ANC_SET_PARTNER_VOL:
                    if (anc_param_length >= 4) {
                        pEvt->param.gain.val = pAnc_cmd->param.gain.val;
                        race_anc_mutex_take();
                        g_anc_set_partner_vol = pAnc_cmd->param.gain.val;
                        race_anc_mutex_give();
                        anc_ret = audio_anc_set_volume(pAnc_cmd->param.gain, TO_PARTNER);
                        if (anc_get_sync_time() == 0) {
                            RACE_FreePacket(pEvt);
                            return NULL;
                        }
                    }
                    break;
                case RACE_ANC_READ_PARTNER_NVDM:
                    if (pAnc_cmd->status == 0) {
                        uint32_t pktId = (uint32_t)pCmdMsg->hdr.pktId.value;
                        if ((bt_sink_srv_cm_get_special_aws_device() != NULL) && (bt_sink_srv_cm_get_aws_link_state() == BT_AWS_MCE_AGENT_STATE_ATTACHED)) {
                            anc_ret = anc_send_aws_mce_param(ANC_DIRECT, ANC_CONTROL_EVENT_READ_VOLUME_NVDM, 0, ((pktId << 8) | channel_id));
                            RACE_FreePacket(pEvt);
                            return NULL;
                        } else {
                            pEvt->status = RACE_ERRCODE_FAIL;
                            return pEvt;
                        }
                    } else {
                        uint8_t true_channel_id = pAnc_cmd->status & (~0x80);
                        pEvt->status = 0;
                        pEvt->param.gain.val = pAnc_cmd->param.gain.val;
                        race_flush_packet((void *)pEvt, true_channel_id);
                        return NULL;
                    }
                case RACE_ANC_WRITE_PARTNER_NVDM:
                    if (anc_param_length >= 4) {
                        pEvt->param.gain.val = pAnc_cmd->param.gain.val;
                        anc_ret = audio_anc_write_volume_nvdm(&pAnc_cmd->param.gain, TO_PARTNER);
                    }
                    break;
                case RACE_ANC_SET_SYNC_TIME:
                    if (anc_param_length >= 2) {
                        pEvt->param.u2param = pAnc_cmd->param.u2param;
                        anc_set_sync_time(pAnc_cmd->param.u2param);
                        anc_ret = ANC_CONTROL_EXECUTION_SUCCESS;
                        if ((bt_sink_srv_cm_get_special_aws_device() != NULL) && (bt_sink_srv_cm_get_aws_link_state() == BT_AWS_MCE_AGENT_STATE_ATTACHED)) {
                            anc_ret = anc_send_aws_mce_param(ANC_DIRECT, ANC_CONTROL_EVENT_SET_SYNC_TIME, (uint32_t)pAnc_cmd->param.u2param, 0);
                        } else {
                            pEvt->status = RACE_ERRCODE_FAIL;
                            return pEvt;
                        }
                    }
                    break;
#endif
                case RACE_ANC_GET_HYBRID_CAP:
                    anc_ret = anc_get_hybrid_capability(&pEvt->param.hybrid_cap);
                    break;
                default:
                    pEvt->status = RACE_ERRCODE_NOT_SUPPORT;
                    return pEvt;
            }
            pEvt->status = (anc_ret == ANC_CONTROL_EXECUTION_SUCCESS) ? RACE_ERRCODE_SUCCESS : RACE_ERRCODE_FAIL;
        } else {
            pEvt->status = RACE_ERRCODE_PARAMETER_ERROR;
        }
    }

    return pEvt;
}

/**
 * RACE_DSPREALTIME_PASSTHRU_HDR
 *
 * RACE_DSPREALTIME_PASSTHRU_HDR Handler
 *
 * @pCmdMsg : pointer of ptr_race_pkt_t
 *
 */
void *RACE_DSPREALTIME_PASSTHRU_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint8_t enable;
    } PACKED RACE_DSPREALTIME_PASSTHRU_CMD_STRU;

    typedef struct {
        uint8_t status;
    } PACKED RACE_DSPREALTIME_PASSTHRU_EVT_STRU;

    RACE_DSPREALTIME_PASSTHRU_CMD_STRU *pCmd = (RACE_DSPREALTIME_PASSTHRU_CMD_STRU *)pCmdMsg;
    RACE_DSPREALTIME_PASSTHRU_EVT_STRU *pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                                                pCmd->Hdr.id,
                                                                sizeof(RACE_DSPREALTIME_PASSTHRU_EVT_STRU),
                                                                channel_id);
    if (pEvt) {
        anc_control_result_t anc_ret = ANC_CONTROL_EXECUTION_NONE;
        if (pCmd->Hdr.id == RACE_DSPREALTIME_PASS_THROUGH_ON_OFF) {
            if (pCmd->enable == 0) {
                uint8_t enable;
                uint32_t runtime_info;
                anc_get_status(&enable, &runtime_info, NULL);
                if ((enable == 1) && ((runtime_info & ANC_FILTER_TYPE_MASK) != FILTER_5)) {
                    anc_ret = ANC_CONTROL_EXECUTION_SUCCESS;
                } else {
                    anc_ret = audio_anc_disable(NULL);
                }
            } else if (pCmd->enable == 1) {
                anc_ret = audio_anc_enable(FILTER_5, ANC_UNASSIGNED_GAIN, NULL);
            }
        }
        pEvt->status = anc_ret;
    }

    return pEvt;
}

void *RACE_DSPREALTIME_PASSTHRU_TEST_MUTE_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint8_t LR_mic;
    } PACKED RACE_DSPREALTIME_PASSTHRU_TEST_CMD_STRU;

    typedef struct {
        uint8_t status;
    } PACKED RACE_DSPREALTIME_PASSTHRU_TEST_EVT_RES_STRU;

    typedef struct {
        uint32_t unknown;
        int db_value;
        uint32_t freq;
    } PACKED RACE_DSPREALTIME_PASSTHRU_TEST_NOTIFICATION_STRU;


    int32_t ret = RACE_ERRCODE_FAIL;
    RACE_DSPREALTIME_PASSTHRU_TEST_EVT_RES_STRU *pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                                                         RACE_DSPREALTIME_PASS_THROUGH_TEST,
                                                                         sizeof(RACE_DSPREALTIME_PASSTHRU_TEST_EVT_RES_STRU),
                                                                         channel_id);
    RACE_DSPREALTIME_PASSTHRU_TEST_CMD_STRU *pCmd = (RACE_DSPREALTIME_PASSTHRU_TEST_CMD_STRU *)pCmdMsg;

    if (pEvt) {
        PTT_u4Freq_Mag_data result = {0, 0};

        if (pCmd) {
            RACE_DSPREALTIME_PASSTHRU_TEST_NOTIFICATION_STRU *noti = NULL;

            /* A1. Execute the cmd. */
            pass_through_test(PTT_AMIC_DCC, pCmd->LR_mic, PTT_MUTE, &result);

            /* A2. Create the noti. */
            noti = (void *)RACE_ClaimPacket(RACE_TYPE_NOTIFICATION,
                                            RACE_DSPREALTIME_PASS_THROUGH_TEST,
                                            sizeof(RACE_DSPREALTIME_PASSTHRU_TEST_NOTIFICATION_STRU),
                                            channel_id);
            if (noti) {
                /* A3. Set the noti parameters with the cmd results.  */
                noti->db_value = (int)round(result.db_data);
                noti->freq = result.freq_data;

                /* A4. Send the noti. */
                ret = race_noti_send(noti, channel_id, TRUE);
                if (RACE_ERRCODE_SUCCESS != ret) {
                    /* A5. Free the noti if needed. */
                    RACE_FreePacket(noti);
                    noti = NULL;
                    pEvt->status = ret;
                    return pEvt;
                }
            } else {
                ret = RACE_ERRCODE_NOT_ENOUGH_MEMORY;
                pEvt->status = ret;
                return pEvt;
            }


        }

        pEvt->status = ret;
    }

    return pEvt;

}


#ifdef MTK_USER_TRIGGER_FF_ENABLE
#ifndef MTK_USER_TRIGGER_ADAPTIVE_FF_V2
void race_dsprealtime_anc_adaptive_trigger_ff()
{
    audio_anc_user_trigger_ff_start(anc_user_trigger_adaptive_ff_callback);
}

void race_dsprealtime_anc_adaptive_notify_end()
{
    race_send_event_notify_msg(RACE_EVENT_TYPE_ANC_ADAPTIVE_FF_END, NULL);
}

void race_dsprealtime_anc_adaptive_abort()
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_status_t ret = BT_STATUS_FAIL;

    typedef struct {
        uint8_t status;
        uint8_t mode;
    } PACKED RACE_DSPREALTIME_ANC_ADAPTIVE_RESULT_EVT_STRU;

    RACE_DSPREALTIME_ANC_ADAPTIVE_RESULT_EVT_STRU *pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                                                           RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK,
                                                                           sizeof(RACE_DSPREALTIME_ANC_ADAPTIVE_RESULT_EVT_STRU),
                                                                           g_anc_race_ch_id);

    if (pEvt) {
        pEvt->status = USER_TRIGGER_FF_STATUS_ABORT;
        pEvt->mode = ADAPTIVE_FF_ANC_MODE;
        if ((role == BT_AWS_MCE_ROLE_AGENT) || (role == BT_AWS_MCE_ROLE_NONE)) {
            race_flush_packet((void *)pEvt, g_anc_race_ch_id);
        } else {/*partner send agent result*/
            race_send_pkt_t *pSndPkt;
            pSndPkt = (void *)race_pointer_cnv_pkt_to_send_pkt((void *)pEvt);
            race_pkt_t      *pret;
            race_send_pkt_t *psend;
            psend = (race_send_pkt_t *)pSndPkt;
            pret = &psend->race_data;
#if (RACE_DEBUG_PRINT_ENABLE)
            race_dump((uint8_t *)pret, RACE_DBG_EVT);
#endif

            if (pSndPkt) {
                #ifdef RACE_RELAY_CMD_ENABLE
                ret = bt_send_aws_mce_race_cmd_data(&pSndPkt->race_data, pSndPkt->length, g_anc_race_ch_id, RACE_CMD_RSP_FROM_PARTNER, 0);
                #endif
                if (ret != BT_STATUS_SUCCESS) {
                    RACE_LOG_MSGID_I("[relay_cmd][user_trigger_ff]partner send relay req FAIL \n", 0);
                } else {
                    RACE_LOG_MSGID_I("[relay_cmd][user_trigger_ff]race_dsprealtime_anc_adaptive_abort, send status:%d success", pEvt->status);
                }
                race_mem_free(pSndPkt);
            }
        }
    }

}
#endif
#endif


#else
/**
 * RACE_DSPREALTIME_ANC_HDR
 *
 * RACE_DSPREALTIME_ANC_HDR Handler
 *
 * @pCmdMsg : pointer of ptr_race_pkt_t
 *
 */
extern uint16_t g_anc_race_cmd_length;
void *RACE_DSPREALTIME_ANC_PASSTHRU_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    race_dsprealtime_anc_struct *pEvt = NULL;
    g_anc_race_ch_id = channel_id;

    race_dsprealtime_anc_struct *pAnc_cmd;
    audio_anc_control_result_t anc_ret = AUDIO_ANC_CONTROL_EXECUTION_NONE;
    pAnc_cmd = (race_dsprealtime_anc_struct *)pCmdMsg->payload;
    g_anc_race_cmd_length = pCmdMsg->hdr.length - 4;
    //RACE_LOG_MSGID_I("RACE_DSPREALTIME_ANC_HDR msg_length:%d payload: %d %d\n",3,pCmdMsg->hdr.length,pAnc_cmd->param.header.ancId,pAnc_cmd->param.anc_on_param.anc_filter_type);
    RACE_LOG_MSGID_E("[RACE][ANC_PTHR]channel_id: %d, ANC_id: %d\n", 2, channel_id, pAnc_cmd->param.header.ancId);

    if (pCmdMsg->hdr.length >= 4) { //hdr.length = hdr.id(2bytes) + pAnc_cmd->status(1byte) + pAnc_cmd->anc_id(1byte) + pAnc_cmd->param(0/1/2/4 bytes)
        switch (pAnc_cmd->param.header.ancId) {
            case RACE_ANC_ON: {
#ifdef AIR_ANC_V3
//errrrrrrrrrrrrrrrrrrr
                if (pAnc_cmd->param.onV3Cmd.syncMode == 2) {
#else
errrrrrrrrrrrr
                if (pAnc_cmd->param.onCmd.syncMode   == 2) {
#endif
#if 1   //application sync workaround.
#ifdef AIR_ANC_V3
                    g_app_sync_flash_no = pAnc_cmd->param.onV3Cmd.flash_no;
#else
                    g_app_sync_flash_no = pAnc_cmd->param.onCmd.flash_no;
#endif
                    g_app_sync_status   = 1;
#endif
                    RACE_LOG_MSGID_I("RACE_DSPREALTIME_ANC_PASSTHRU_HDR,111 send g_app_sync_flash_no:%d ", 1,g_app_sync_flash_no);

                    race_dsp_realtime_send_notify_msg(RACE_DSPREALTIME_ANC, RACE_ANC_ON, anc_ret, g_anc_race_cmd_length + 2, (uint8_t *)pAnc_cmd);
                } else {
                    anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_RACE, 0, (void *)pAnc_cmd);
                    /* Notify APP Layer ANC on */
                    race_dsp_realtime_send_notify_msg(RACE_DSPREALTIME_ANC, RACE_ANC_ON, anc_ret, 0, NULL);

                    if (g_anc_race_cmd_length <= sizeof(RACE_RSP_ANC_PASSTHRU_ON_PARAM)) {
                    RACE_LOG_MSGID_I("RACE_DSPREALTIME_ANC_PASSTHRU_HDR,222 g_anc_race_cmd_length=:%d ",1, g_anc_race_cmd_length);
                      
                    pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                            RACE_DSPREALTIME_ANC,
                                            sizeof(RACE_RSP_ANC_PASSTHRU_ON_PARAM),
                                            channel_id);
                    if (pEvt != NULL) {
                        memset(pEvt, 0, sizeof(RACE_RSP_ANC_PASSTHRU_ON_PARAM));
                    } else {
                        goto _HDR_END;
                    }
                    pEvt->param.onRsp.header.ancId  = pAnc_cmd->param.onCmd.header.ancId;
                    pEvt->param.onRsp.header.status = anc_ret;
                    pEvt->param.onRsp.flash_no      = pAnc_cmd->param.onCmd.flash_no;
                        if (pAnc_cmd->param.onCmd.flash_no < AUDIO_ANC_CONTROL_FILTER_ID_PASS_THRU_1) {
                        pEvt->param.onRsp.ancType   = pAnc_cmd->param.onCmd.ancType;
                     RACE_LOG_MSGID_I("RACE_DSPREALTIME_ANC_PASSTHRU_HDR,333 pEvt->param.onRsp.ancType=:%d ", 1,pEvt->param.onRsp.ancType);
                   } else {
                        pEvt->param.onRsp.ancType   = AUDIO_ANC_CONTROL_TYPE_PASSTHRU_FF;
                    RACE_LOG_MSGID_I("RACE_DSPREALTIME_ANC_PASSTHRU_HDR,444  pEvt->param.onRsp.ancType=:%d ",1,  pEvt->param.onRsp.ancType);
                    }
                    pEvt->param.onRsp.syncMode      = pAnc_cmd->param.onCmd.syncMode;
                    RACE_LOG_MSGID_I("RACE_DSPREALTIME_ANC_PASSTHRU_HDR,444111   pEvt->param.onRsp.syncMode:%d ",1,   pEvt->param.onRsp.syncMode);
                    RACE_LOG_MSGID_I("RACE_DSPREALTIME_ANC_PASSTHRU_HDR,444112  ancId:%d anc_ret=%d, pAnc_cmd->param.onCmd.flash_no=%d",3,   pAnc_cmd->param.onCmd.header.ancId,anc_ret,pAnc_cmd->param.onCmd.flash_no);
#ifdef AIR_ANC_V3
                    } else {
                        pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                                RACE_DSPREALTIME_ANC,
                                                sizeof(RACE_RSP_ANC_PASSTHRU_ON_V3_PARAM),
                                                channel_id);
                        if (pEvt != NULL) {
                            memset(pEvt, 0, sizeof(RACE_RSP_ANC_PASSTHRU_ON_V3_PARAM));
                        } else {
                            goto _HDR_END;
                        }
                        pEvt->param.onV3Rsp.header.ancId  = pAnc_cmd->param.onV3Cmd.header.ancId;
                        pEvt->param.onV3Rsp.header.status = anc_ret;
                        pEvt->param.onV3Rsp.flash_no      = pAnc_cmd->param.onV3Cmd.flash_no;
                        if (pAnc_cmd->param.onV3Cmd.flash_no < AUDIO_ANC_CONTROL_PASS_THRU_FILTER_DEFAULT) {
                            pEvt->param.onV3Rsp.ancType   = pAnc_cmd->param.onV3Cmd.ancType;
                    RACE_LOG_MSGID_I("RACE_DSPREALTIME_ANC_PASSTHRU_HDR,555  pEvt->param.onV3Rsp.ancType=:%d ",1,  pEvt->param.onV3Rsp.ancType);
                        } else {
                            pEvt->param.onV3Rsp.ancType   = AUDIO_ANC_CONTROL_TYPE_PASSTHRU_FF;
                    RACE_LOG_MSGID_I("RACE_DSPREALTIME_ANC_PASSTHRU_HDR,666  pEvt->param.onV3Rsp.ancType=:%d ",1,  pEvt->param.onV3Rsp.ancType);
                        }
                        pEvt->param.onV3Rsp.syncMode      = pAnc_cmd->param.onV3Cmd.syncMode;
                        pEvt->param.onV3Rsp.sub_ID        = pAnc_cmd->param.onV3Cmd.sub_ID;
                    RACE_LOG_MSGID_I("RACE_DSPREALTIME_ANC_PASSTHRU_HDR,666111  pEvt->param.onV3Rsp.ancType=:%d,pAnc_cmd->param.onV3Cmd.sub_ID=%d ",2, pEvt->param.onV3Rsp.syncMode,pAnc_cmd->param.onV3Cmd.sub_ID);
#endif
                    }
                }
                break;
            }
            case RACE_ANC_OFF: {
                if (pAnc_cmd->param.offCmd.syncMode == 2) {
                    race_dsp_realtime_send_notify_msg(RACE_DSPREALTIME_ANC, RACE_ANC_OFF, anc_ret, g_anc_race_cmd_length + 2, (uint8_t *)pAnc_cmd);
                } else {
                    anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_RACE, 0, (void *)pAnc_cmd);
                    /* Notify ANC off */
                    race_dsp_realtime_send_notify_msg(RACE_DSPREALTIME_ANC, RACE_ANC_OFF, anc_ret, 0, NULL);

                    pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                            RACE_DSPREALTIME_ANC,
                                            sizeof(RACE_RSP_ANC_PASSTHRU_OFF_PARAM),
                                            channel_id);
                    if (pEvt != NULL) {
                        memset(pEvt, 0, sizeof(RACE_RSP_ANC_PASSTHRU_OFF_PARAM));
                    } else {
                        goto _HDR_END;
                    }
                    pEvt->param.offRsp.header.ancId  = pAnc_cmd->param.offCmd.header.ancId;
                    pEvt->param.offRsp.header.status = anc_ret;
                    pEvt->param.offRsp.syncMode      = pAnc_cmd->param.offCmd.syncMode;
                    RACE_LOG_MSGID_I("RACE_DSPREALTIME_ANC_PASSTHRU_HDR,444113  ancId:%d anc_ret=%d, pAnc_cmd->param.offCmd.syncMode=%d",3,   pAnc_cmd->param.offCmd.header.ancId,anc_ret,pAnc_cmd->param.offCmd.syncMode);
                    
                    #if 0
                    if (audio_anc_control_get_sync_time() == 0) {
                        RACE_FreePacket(pEvt);
                        return NULL;
                    }
                    #endif
                }
                break;
            }
            case RACE_ANC_SET_VOL: {
                anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_RACE, 0, (void *)pAnc_cmd);
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_ANC,
                                        sizeof(RACE_RSP_ANC_PASSTHRU_SET_VOL_PARAM),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_ANC_PASSTHRU_SET_VOL_PARAM));
                } else {
                    goto _HDR_END;
                }
                pEvt->param.gainRsp.header.ancId  = pAnc_cmd->param.gainCmd.header.ancId;
                pEvt->param.gainRsp.header.status = anc_ret;
                pEvt->param.gainRsp.gainFFl       = pAnc_cmd->param.gainCmd.gainFFl;
                pEvt->param.gainRsp.gainFBl       = pAnc_cmd->param.gainCmd.gainFBl;
                pEvt->param.gainRsp.gainFFr       = pAnc_cmd->param.gainCmd.gainFFr;
                pEvt->param.gainRsp.gainFBr       = pAnc_cmd->param.gainCmd.gainFBr;
                break;
            }
            case RACE_ANC_READ_NVDM: {
                race_dsprealtime_anc_struct pAnc_rsp;
                pAnc_rsp.param.readNVDMRsp.header.ancId = pAnc_cmd->param.readNVDMCmd.header.ancId;
                anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_RACE, 0, (void *)&pAnc_rsp);
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_ANC,
                                        sizeof(RACE_RSP_ANC_PASSTHRU_READ_NVDM),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_ANC_PASSTHRU_READ_NVDM));
                } else {
                    goto _HDR_END;
                }
                pEvt->param.readNVDMRsp.header.ancId  = pAnc_rsp.param.readNVDMRsp.header.ancId;
                pEvt->param.readNVDMRsp.header.status = anc_ret;
                pEvt->param.readNVDMRsp.gainFFl       = pAnc_rsp.param.readNVDMRsp.gainFFl;
                pEvt->param.readNVDMRsp.gainFBl       = pAnc_rsp.param.readNVDMRsp.gainFBl;
                pEvt->param.readNVDMRsp.gainFFr       = pAnc_rsp.param.readNVDMRsp.gainFFr;
                pEvt->param.readNVDMRsp.gainFBr       = pAnc_rsp.param.readNVDMRsp.gainFBr;
                break;
            }
            case RACE_ANC_WRITE_NVDM: {
                anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_RACE, 0, (void *)pAnc_cmd);
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_ANC,
                                        sizeof(RACE_RSP_ANC_PASSTHRU_WRITE_NVDM),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_ANC_PASSTHRU_WRITE_NVDM));
                } else {
                    goto _HDR_END;
                }
                pEvt->param.writeNVDMRsp.header.ancId  = pAnc_cmd->param.writeNVDMCmd.header.ancId;
                pEvt->param.writeNVDMRsp.header.status = anc_ret;
                pEvt->param.writeNVDMRsp.gainFFl       = pAnc_cmd->param.writeNVDMCmd.gainFFl;
                pEvt->param.writeNVDMRsp.gainFBl       = pAnc_cmd->param.writeNVDMCmd.gainFBl;
                pEvt->param.writeNVDMRsp.gainFFr       = pAnc_cmd->param.writeNVDMCmd.gainFFr;
                pEvt->param.writeNVDMRsp.gainFBr       = pAnc_cmd->param.writeNVDMCmd.gainFBr;
                break;
            }
            case RACE_ANC_SET_RUNTIME_VOL: {
                if (pAnc_cmd->param.runtimeGainCmd.syncMode == 2) {
                    race_dsp_realtime_send_notify_msg(RACE_DSPREALTIME_ANC, RACE_ANC_SET_RUNTIME_VOL, anc_ret, g_anc_race_cmd_length + 2, (uint8_t *)pAnc_cmd);
                } else {
                    anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_RACE, 0, (void *)pAnc_cmd);
                    pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                            RACE_DSPREALTIME_ANC,
                                            sizeof(RACE_RSP_ANC_PASSTHRU_SET_RUNTIME_VOL),
                                            channel_id);
                    if (pEvt != NULL) {
                        memset(pEvt, 0, sizeof(RACE_RSP_ANC_PASSTHRU_SET_RUNTIME_VOL));
                    } else {
                        goto _HDR_END;
                    }
                    pEvt->param.runtimeGainRsp.header.ancId  = pAnc_cmd->param.runtimeGainCmd.header.ancId;
                    pEvt->param.runtimeGainRsp.header.status = anc_ret;
                    pEvt->param.runtimeGainRsp.gain          = pAnc_cmd->param.runtimeGainCmd.gain;
                    pEvt->param.runtimeGainRsp.syncMode      = pAnc_cmd->param.runtimeGainCmd.syncMode;
                }
                break;
            }
            case RACE_ANC_SET_RAMP_CAP_BY_CH: {
                //printf("[53debug]anc_race_cmd_hdlr, filter_mask 0x%x, gain_value %d, delay %d, up_step %d, dn_step %d", pAnc_cmd->param.RampCapCmd.filter_mask, pAnc_cmd->param.RampCapCmd.ramp_rate.gain_value, pAnc_cmd->param.RampCapCmd.ramp_rate.delay_time, pAnc_cmd->param.RampCapCmd.ramp_rate.ramp_up_dly_step, pAnc_cmd->param.RampCapCmd.ramp_rate.ramp_dn_dly_step);
                anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_RACE, 0, (void *)pAnc_cmd);
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_ANC,
                                        sizeof(RACE_RSP_ANC_PASSTHRU_RAMP_CAP_PARAM),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_ANC_PASSTHRU_RAMP_CAP_PARAM));
                } else {
                    goto _HDR_END;
                }

                pEvt->param.RampCapCmd.header.ancId  = pAnc_cmd->param.RampCapCmd.header.ancId;
                pEvt->param.RampCapCmd.header.status = anc_ret;
                pEvt->param.RampCapCmd.filter_mask = pAnc_cmd->param.RampCapCmd.filter_mask;
                pEvt->param.RampCapCmd.ramp_rate.gain_value = pAnc_cmd->param.RampCapCmd.ramp_rate.gain_value;
                pEvt->param.RampCapCmd.ramp_rate.delay_time = pAnc_cmd->param.RampCapCmd.ramp_rate.delay_time;
                pEvt->param.RampCapCmd.ramp_rate.ramp_up_dly_step = pAnc_cmd->param.RampCapCmd.ramp_rate.ramp_up_dly_step;
                pEvt->param.RampCapCmd.ramp_rate.ramp_dn_dly_step = pAnc_cmd->param.RampCapCmd.ramp_rate.ramp_dn_dly_step;

                //save to nvdm
                //if (anc_ret == AUDIO_ANC_CONTROL_EXECUTION_SUCCESS) {
                //    audio_anc_control_set_ramp_capability_into_flash(pAnc_cmd->param.RampCapCmd.filter_mask, &pAnc_cmd->param.RampCapCmd.ramp_rate, NULL);
                //}
                break;
            }
            case RACE_ANC_WRITE_RAMP_CAP_BY_CH_NVDM: {
                anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_RACE, 0, (void *)pAnc_cmd);
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_ANC,
                                        sizeof(RACE_RSP_ANC_PASSTHRU_RAMP_CAP_PARAM),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_ANC_PASSTHRU_RAMP_CAP_PARAM));
                } else {
                    goto _HDR_END;
                }

                pEvt->param.RampCapCmd.header.ancId  = pAnc_cmd->param.RampCapCmd.header.ancId;
                pEvt->param.RampCapCmd.header.status = anc_ret;
                pEvt->param.RampCapCmd.filter_mask = pAnc_cmd->param.RampCapCmd.filter_mask;
                pEvt->param.RampCapCmd.ramp_rate.gain_value = pAnc_cmd->param.RampCapCmd.ramp_rate.gain_value;
                pEvt->param.RampCapCmd.ramp_rate.delay_time = pAnc_cmd->param.RampCapCmd.ramp_rate.delay_time;
                pEvt->param.RampCapCmd.ramp_rate.ramp_up_dly_step = pAnc_cmd->param.RampCapCmd.ramp_rate.ramp_up_dly_step;
                pEvt->param.RampCapCmd.ramp_rate.ramp_dn_dly_step = pAnc_cmd->param.RampCapCmd.ramp_rate.ramp_dn_dly_step;

                break;
            }
            case RACE_ANC_READ_RAMP_CAP_BY_CH: {
                anc_ret = AUDIO_ANC_CONTROL_EXECUTION_SUCCESS;
                audio_anc_control_ramp_cap_t local_ramp_cap;
                memset(&local_ramp_cap, 0, sizeof(local_ramp_cap));
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_ANC,
                                        sizeof(RACE_RSP_ANC_PASSTHRU_READ_RAMP_CAP),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_ANC_PASSTHRU_READ_RAMP_CAP));
                } else {
                    goto _HDR_END;
                }

                for (uint8_t filter_mask = 0; filter_mask <= 3; filter_mask++) {
                    anc_ret |= audio_anc_control_get_ramp_capability((1 << filter_mask), &local_ramp_cap.gain_value, &local_ramp_cap.delay_time, &local_ramp_cap.ramp_up_dly_step, &local_ramp_cap.ramp_dn_dly_step, NULL);
                    pEvt->param.RampCapReadCmd.ramp_rate[filter_mask].gain_value = local_ramp_cap.gain_value;
                    pEvt->param.RampCapReadCmd.ramp_rate[filter_mask].delay_time = local_ramp_cap.delay_time;
                    pEvt->param.RampCapReadCmd.ramp_rate[filter_mask].ramp_up_dly_step = local_ramp_cap.ramp_up_dly_step;
                    pEvt->param.RampCapReadCmd.ramp_rate[filter_mask].ramp_dn_dly_step = local_ramp_cap.ramp_dn_dly_step;
                    RACE_LOG_MSGID_I("[ANC_Ramp_Cap] READ_RAMP_CAP_BY_CH, anc_ret %d, filter_mask 0x%x, gain_value %d, delay %d, up_tep %d, dn_step %d", 6, anc_ret, filter_mask, pEvt->param.RampCapReadCmd.ramp_rate[filter_mask].gain_value, pEvt->param.RampCapReadCmd.ramp_rate[filter_mask].delay_time,
                        pEvt->param.RampCapReadCmd.ramp_rate[filter_mask].ramp_up_dly_step, pEvt->param.RampCapReadCmd.ramp_rate[filter_mask].ramp_dn_dly_step);
                }
                pEvt->param.RampCapReadCmd.header.ancId  = pAnc_cmd->param.RampCapCmd.header.ancId;
                pEvt->param.RampCapReadCmd.header.status = anc_ret;

                break;
            }
#if 0//def MTK_AWS_MCE_ENABLE
            case RACE_ANC_SET_AGENT_VOL:
            case RACE_ANC_SET_PARTNER_VOL:
            case RACE_ANC_READ_PARTNER_NVDM:
            case RACE_ANC_WRITE_PARTNER_NVDM:
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_ANC,
                                        sizeof(RACE_ANC_PASSTHRU_HEADER),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_ANC_PASSTHRU_HEADER));
                } else {
                    goto _HDR_END;
                }
                pEvt->param.header.status = RACE_ERRCODE_NOT_SUPPORT;
                return pEvt;
                break;
#endif
            case RACE_ANC_SET_SYNC_TIME: {
                if (pAnc_cmd->param.setSyncTimeCmd.sync_time != audio_anc_control_get_sync_time()) {
                    audio_anc_control_set_sync_time(pAnc_cmd->param.setSyncTimeCmd.sync_time);
                }
                audio_anc_control_set_attach_enable((pAnc_cmd->param.setSyncTimeCmd.sync_time == 0) ? AUDIO_ANC_CONTROL_ATTACH_MODE_OFF : AUDIO_ANC_CONTROL_ATTACH_MODE_1);
                anc_ret = AUDIO_ANC_CONTROL_EXECUTION_SUCCESS;
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_ANC,
                                        sizeof(RACE_RSP_ANC_PASSTHRU_SET_SYNC_TIME),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_ANC_PASSTHRU_SET_SYNC_TIME));
                } else {
                    goto _HDR_END;
                }
                pEvt->param.setSyncTimeRsp.header.ancId  = pAnc_cmd->param.setSyncTimeCmd.header.ancId;
                pEvt->param.setSyncTimeRsp.header.status = anc_ret;
                pEvt->param.setSyncTimeRsp.sync_time     = pAnc_cmd->param.setSyncTimeCmd.sync_time;
                break;
            }
            case RACE_ANC_GET_HYBRID_CAP_V2: {
                uint8_t support_hybrid;
                audio_anc_control_get_status(NULL, NULL, NULL, NULL, &support_hybrid, NULL);
                anc_ret = AUDIO_ANC_CONTROL_EXECUTION_SUCCESS;
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_ANC,
                                        sizeof(RACE_RSP_ANC_PASSTHRU_GET_HYBRID_CAP),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_ANC_PASSTHRU_GET_HYBRID_CAP));
                } else {
                    goto _HDR_END;
                }
                pEvt->param.getHybridRsp.header.ancId  = pAnc_cmd->param.header.ancId;
                pEvt->param.getHybridRsp.header.status = anc_ret;
                pEvt->param.getHybridRsp.hybridEnable  = support_hybrid;
                break;
            }
            case RACE_ANC_REATIME_UPDATE_COEF: {
                anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_RACE, 0, (void *)pAnc_cmd);
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_ANC,
                                        sizeof(RACE_RSP_ANC_PASSTHRU_SET_COEF),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_ANC_PASSTHRU_SET_COEF));
                } else {
                    goto _HDR_END;
                }
                pEvt->param.setCoefRsp.header.ancId  = pAnc_cmd->param.setCoefCmd.header.ancId;
                pEvt->param.setCoefRsp.header.status = anc_ret;
                break;
            }
            case RACE_ANC_FLEXIBLE_REATIME_UPDATE_COEF: {
                anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_RACE, 0, (void *)pAnc_cmd);
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_ANC,
                                        sizeof(RACE_RSP_ANC_PASSTHRU_SET_COEF),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_ANC_PASSTHRU_SET_COEF));
                } else {
                    goto _HDR_END;
                }
                pEvt->param.setCoefRsp.header.ancId  = pAnc_cmd->param.setCoefCmd.header.ancId;
                pEvt->param.setCoefRsp.header.status = anc_ret;
                break;
            }
            case RACE_ANC_ENTER_MP_MODE: {
                anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_RACE, 0, (void *)pAnc_cmd);
                /* Enter MP mode, set anc sync time to 0. */
                audio_anc_control_set_sync_time(0);

                /* Enter MP mode, disable Adaptive ANC. */
                /* Important, DO NOT SET any NVDM. MP mode was temp control flow for MP test.*/
                //;
                //if anc is enabled --> turn off adaptive ANC stream
#ifdef ADAPIVE_ANC_STREAM_CONTROL
                RACE_LOG_MSGID_I("[RACE][Adaptive ANC Stream] Enter MP Mode, suspend Adaptive ANC Stream.", 0);
                audio_anc_monitor_adaptive_anc_stream_enable(FALSE); //suspend Adaptive ANC stream
                g_adaptive_ANC_stream_MP_control_flag = true; //enter MP mode
#endif
                /* Enter MP mode, reset extend gain. */
                audio_anc_control_extend_ramp_cap_t init_ramp_cap = {0};
                init_ramp_cap.gain_type = AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_WIND_NOISE;
                anc_ret = audio_anc_control_set_extend_gain(init_ramp_cap.gain_type, &init_ramp_cap, NULL);
                init_ramp_cap.gain_type = AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_USER_UNAWARE;
                anc_ret = audio_anc_control_set_extend_gain(init_ramp_cap.gain_type, &init_ramp_cap, NULL);
                init_ramp_cap.gain_type = AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_ENVIRONMENT_DETECTION;
                anc_ret = audio_anc_control_set_extend_gain(init_ramp_cap.gain_type, &init_ramp_cap, NULL);

                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_ANC,
                                        sizeof(RACE_CMD_ANC_PASSTHRU_ENTER_LEAVE_MP_MODE),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_CMD_ANC_PASSTHRU_ENTER_LEAVE_MP_MODE));
                } else {
                    goto _HDR_END;
                }
                pEvt->param.MPmodeRsp.header.ancId  = pAnc_cmd->param.MPmodeCmd.header.ancId;
                pEvt->param.MPmodeRsp.header.status = anc_ret;
                break;
            }
            case RACE_ANC_LEAVE_MP_MODE: {
                anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_RACE, 0, (void *)pAnc_cmd);
                /* Leave MP mode, reset anc sync time to default. */
                audio_anc_control_set_sync_time(500);

                /* Leave MP mode, enable Adaptive ANC. */
                /* Important, DO NOT SET any NVDM. MP mode was temp control flow for MP test.*/
                //;
#ifdef ADAPIVE_ANC_STREAM_CONTROL
                RACE_LOG_MSGID_I("[RACE][Adaptive ANC Stream] Leave MP Mode, resume Adaptive ANC Stream.", 0);
                g_adaptive_ANC_stream_MP_control_flag = false; //leave MP mode
                audio_anc_monitor_adaptive_anc_stream_enable(TRUE); //resume Adaptive ANC Stream
#endif

                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_ANC,
                                        sizeof(RACE_CMD_ANC_PASSTHRU_ENTER_LEAVE_MP_MODE),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_CMD_ANC_PASSTHRU_ENTER_LEAVE_MP_MODE));
                } else {
                    goto _HDR_END;
                }
                pEvt->param.MPmodeRsp.header.ancId  = pAnc_cmd->param.MPmodeCmd.header.ancId;
                pEvt->param.MPmodeRsp.header.status = anc_ret;
                break;
            }
            case RACE_ANC_EM_CMD: {
                anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_RACE, 0, (void *)pAnc_cmd);
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_ANC,
                                        sizeof(RACE_RSP_ANC_PASSTHRU_EM_MODE_PARAM),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_ANC_PASSTHRU_EM_MODE_PARAM));
                } else {
                    goto _HDR_END;
                }
                pEvt->param.EMCmdRsp.header.ancId  = pAnc_cmd->param.EMCmd.header.ancId;
                pEvt->param.EMCmdRsp.header.status = anc_ret;
                break;
            }
            default: {
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_ANC,
                                        sizeof(RACE_ANC_PASSTHRU_HEADER),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_ANC_PASSTHRU_HEADER));
                } else {
                    goto _HDR_END;
                }
                pEvt->param.header.status = RACE_ERRCODE_NOT_SUPPORT;
                return pEvt;
                break;
            }
        }
        if (pEvt != NULL) {
            pEvt->param.header.status = (anc_ret == AUDIO_ANC_CONTROL_EXECUTION_SUCCESS) ? RACE_ERRCODE_SUCCESS : RACE_ERRCODE_FAIL;
        }
    } else {
        pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                RACE_DSPREALTIME_ANC,
                                sizeof(RACE_ANC_PASSTHRU_HEADER),
                                channel_id);
        if (pEvt != NULL) {
            memset(pEvt, 0, sizeof(RACE_ANC_PASSTHRU_HEADER));
        } else {
            goto _HDR_END;
        }
        pEvt->param.header.status = RACE_ERRCODE_PARAMETER_ERROR;
    }

    if (pEvt != NULL) {
        if (pEvt->param.header.status != RACE_ERRCODE_SUCCESS) {
            RACE_LOG_MSGID_E("RACE_DSPREALTIME_ANC_HDR pEvt->status: %d  anc_ret: %d\n", 2, pEvt->param.header.status, anc_ret);
        }
    }
_HDR_END:
    return pEvt;
}

#if defined(AIR_ANC_V3) && defined(AIR_FULL_ADAPTIVE_ANC_ENABLE)
extern void audio_anc_control_get_full_adaptive_info(uint8_t *control_state, uint8_t *control_scenario, uint16_t *rate, audio_anc_control_misc_t *control_misc);
extern void audio_anc_control_get_full_adaptive_process_status(uint8_t *process_state_l, uint8_t *process_state_r, audio_anc_control_misc_t *control_misc);
extern void audio_anc_control_dump_full_adaptive_coef(uint8_t *coef, uint8_t *param1, uint8_t *param2, audio_anc_control_misc_t *control_misc);
#ifdef AIR_ANC_ADAPTIVE_EVENT_DRIVEN_ENABLE
extern volatile uint16_t g_FADP_ADAPTIVE_TIME_MS;
#endif
int32_t g_race_fadp_fir_coef[300] = {0};
#define RACE_FADP_DUMP_COEF_DEBUG true
static sysram_status_t race_dsprealtime_full_adaptive_get_iir_ui_coef(uint16_t nvid, uint8_t *nvid_info, uint16_t *nvid_length)
{
    uint32_t nvkey_length = 0;
    uint8_t *malloc_ptr = NULL;
    uint16_t nvkey_id = nvid; //assign 0xEE40, 0xEE42, 0xEE44, 0xEE46
    sysram_status_t nvdm_status;

    nvdm_status = flash_memory_query_nvdm_data_length(nvkey_id, &nvkey_length);
    RACE_LOG_MSGID_I("[FADP_get]get_iir_ui_coef, nvdm_status %d, nvkey_id 0x%x, nvkey_length %d", 3, nvdm_status, nvkey_id, nvkey_length);
    malloc_ptr = pvPortMalloc(nvkey_length);
    if (malloc_ptr) {
        nvdm_status = flash_memory_read_nvdm_data(nvkey_id, (uint8_t *)malloc_ptr, &nvkey_length);
        if (nvdm_status || !nvkey_length) {
            RACE_LOG_MSGID_E("[FADP_get]get_iir_ui_coef, Read Nvkey data Fail id:0x%x, status:%d ", 2, nvkey_id, nvdm_status);
            vPortFree(malloc_ptr);
            return nvdm_status;
        }
        if (nvid_length) {
            *nvid_length = nvkey_length;
        }
        if (nvid_info) {
            memcpy(nvid_info, malloc_ptr, nvkey_length);
        }

        #if RACE_FADP_DUMP_COEF_DEBUG //debug use
        for (int dump_i = 0; dump_i < 100; dump_i += 4) {
            //hal_gpt_delay_us(150);
            RACE_LOG_MSGID_I("[FADP_get]get_iir_ui_coef, dump IIR UI Coef, index(%d): %x, %x, %x, %x", 5, dump_i, *(malloc_ptr+dump_i), *(malloc_ptr+dump_i+1), *(malloc_ptr+dump_i+2), *(malloc_ptr+dump_i+3));
        }
        #endif
        vPortFree(malloc_ptr);
    } else {
        if (nvdm_status == -4) { //sysram_status_t, the UI NVKEY setting might be In_nvdm="False"
            RACE_LOG_MSGID_E("[FADP_get]get_iir_ui_coef, nvkey(0x%x) not found! the UI NVKEY setting might be In_nvdm=False!", 1, nvkey_id);
            nvkey_length = 100;
            if (nvid_info) {
                memset(nvid_info, 0, nvkey_length);
            }
        } else {
            configASSERT(0 && "[FADP_get]get_iir_ui_coef, malloc Fail");
        }
    }
    return nvdm_status;
}

void race_dsprealtime_full_adaptive_get_coef_response(uint8_t race_type, int8_t status, uint16_t real_data_len, RACE_RSP_FADP_ANC_QUERY_COEF_PARAM *data_buff)
{
    if (data_buff == NULL){
        RACE_LOG_MSGID_E("[FADP_get]get_coef_response, data_buff NULL!",0);
        return;
    }
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_status_t ret = BT_STATUS_FAIL;
    uint16_t data_len = sizeof(RACE_RSP_FADP_ANC_QUERY_COEF_PARAM) + real_data_len - 1;
    //RACE_LOG_MSGID_I("[FADP_get]get_coef_response, race_type:%x, status:%d, data_len:%d, data_buff:0x%x", 4, race_type, status, data_len, data_buff);
    RACE_RSP_FADP_ANC_QUERY_COEF_PARAM *pEvt = RACE_ClaimPacket(race_type,
                                                                RACE_DSPREALTIME_FADP_ANC,
                                                                data_len,
                                                                g_anc_race_ch_id);
    RACE_LOG_MSGID_I("[FADP_get]get_coef_response, race_type %x, status %d, anc_id %d, data_len %d, sub_mode %d, seq_num %d", 6, race_type, data_buff->header.status, data_buff->header.ancId, data_len, data_buff->sub_mode, data_buff->seq_num);
    if (pEvt) {
        memcpy(pEvt, data_buff, data_len);
        if (status != 0) {
            pEvt->header.status = 0xF;
        }

        if ((role == BT_AWS_MCE_ROLE_AGENT) || (role == BT_AWS_MCE_ROLE_NONE)) {
            race_send_pkt_t *pSndPkt;
            uint32_t port_handle, ret_size, size;
            uint8_t *ptr;
            uint32_t send_res = 0;

            pSndPkt = race_pointer_cnv_pkt_to_send_pkt((void *)pEvt);
            port_handle = race_get_port_handle_by_channel_id(g_anc_race_ch_id);

            size = pSndPkt->length;
            ptr = (uint8_t *)&pSndPkt->race_data;

            #if (RACE_DEBUG_PRINT_ENABLE)
            race_dump((uint8_t *)ptr, RACE_DBG_EVT);
            #endif

            ret_size = race_port_send_data(port_handle, ptr, size);
            send_res = (ret_size & (1 << 31));
            ret_size &= ~(1 << 31);

            RACE_LOG_MSGID_I("[FADP_get]get_coef_response, (Agent)send_size:%u, send_res:%u, ret_size:%u", 3, size, send_res, ret_size);
            race_mem_free(pSndPkt);

        } else {//role == BT_AWS_MCE_ROLE_PARTNER
            race_send_pkt_t *pSndPkt;
            pSndPkt = race_pointer_cnv_pkt_to_send_pkt((void *)pEvt);
            race_pkt_t      *pret;
            race_send_pkt_t *psend;
            psend = (race_send_pkt_t *)pSndPkt;
            pret = &psend->race_data;

            #if (RACE_DEBUG_PRINT_ENABLE)
            race_dump((uint8_t *)pret, RACE_DBG_EVT);
            #endif

            if (pSndPkt) {
                #ifdef RACE_RELAY_CMD_ENABLE
                ret = bt_send_aws_mce_race_cmd_data(&pSndPkt->race_data, pSndPkt->length, g_anc_race_ch_id, RACE_CMD_RSP_FROM_PARTNER, 0);
                #endif
                if (ret != BT_STATUS_SUCCESS) {
                    RACE_LOG_MSGID_E("[relay_cmd][FADP_get]get_coef_response, partner send relay req FAIL \n", 0);
                }
                race_mem_free(pSndPkt);
            }
        }
    }
}

static audio_anc_control_result_t race_dsprealtime_full_adaptive_get_coef_cmd_hdr(RACE_CMD_FADP_ANC_QUERY_COEF_PARAM *p_data)
{
    if (p_data == NULL) {
        RACE_LOG_MSGID_E("[FADP_get]cmd NULL!", 0);
        return AUDIO_ANC_CONTROL_EXECUTION_NOT_ALLOWED;
    }

    audio_anc_control_result_t anc_ret = AUDIO_ANC_CONTROL_EXECUTION_SUCCESS;
    RACE_CMD_FADP_ANC_QUERY_COEF_PARAM *p_cmd_data = p_data;
    RACE_LOG_MSGID_I("[FADP_get]get_coef_cmd_hdr, status %d, anc_id 0x%x, data_len %d, sub_mode %d, seq_num %d", 5, p_cmd_data->header.status, p_cmd_data->header.ancId, p_cmd_data->data_len, p_cmd_data->sub_mode, p_cmd_data->seq_num);
    uint16_t resp_data_len = 0;
    RACE_RSP_FADP_ANC_QUERY_COEF_PARAM* p_resp_data = NULL;

    switch (p_cmd_data->sub_mode) {
        case RACE_FADP_ANC_QUERY_COEF_CMD_SUB_MODE_IIR_UI_COEF: {
            typedef struct {
                uint8_t type;
                uint16_t length;
                uint8_t nv_data[100];
            } PACKED data_type_t;
            typedef struct {
                data_type_t data[4];
            } PACKED real_data_t; //103*4 = 412bytes

            real_data_t* real_data = (real_data_t*)pvPortMalloc(sizeof(real_data_t));

            resp_data_len = sizeof(RACE_RSP_FADP_ANC_QUERY_COEF_PARAM) + sizeof(real_data_t)-1; //9+412-1=420
            p_resp_data = (RACE_RSP_FADP_ANC_QUERY_COEF_PARAM*)pvPortMalloc(resp_data_len);
            if ((p_resp_data == NULL) || (real_data == NULL)) {
                anc_ret = AUDIO_ANC_CONTROL_EXECUTION_FAIL;
                RACE_LOG_MSGID_E("[FADP_get] malloc fail!", 0);
                if (p_resp_data) {
                    vPortFree(p_resp_data);
                }
                if (real_data) {
                    vPortFree(real_data);
                }
                goto _HDR_END;
            }

            memset(real_data, 0, sizeof(real_data_t));
            memset(p_resp_data, 0, resp_data_len);
            p_resp_data->data_len = resp_data_len - 4; //4=sizeof(header)+sizeof(p_resp_data->data_len)
            p_resp_data->sub_mode = RACE_FADP_ANC_QUERY_COEF_CMD_SUB_MODE_IIR_UI_COEF;
            p_resp_data->seq_num = 0;
            p_resp_data->real_data_len = sizeof(real_data_t); //412

            uint16_t nvid[4] = {0xEE40, 0xEE42, 0xEE44, 0xEE46};
            sysram_status_t nvdm_status[4] = {0};

            for (int i = 0; i < 4; i++) {
                nvdm_status[i] = race_dsprealtime_full_adaptive_get_iir_ui_coef(nvid[i], real_data->data[i].nv_data, &real_data->data[i].length);
                real_data->data[i].type = i+1;
            }
            if ((nvdm_status[0] != 0)||(nvdm_status[1] != 0)||(nvdm_status[2] != 0)||(nvdm_status[3] != 0)) {
                RACE_LOG_MSGID_E("[FADP_get]get_iir_nvdm err:%d, %d, %d, %d", 4, nvdm_status[0], nvdm_status[1], nvdm_status[2], nvdm_status[3]);
                anc_ret = AUDIO_ANC_CONTROL_EXECUTION_FAIL;
            }

            memcpy(&(p_resp_data->real_data), real_data, sizeof(real_data_t));
            p_resp_data->header.status = anc_ret;
            p_resp_data->header.ancId = RACE_FADP_ANC_QUERY_COEF;

            //RESPONSE TO SP APP BEFORE FREE!!
            race_dsprealtime_full_adaptive_get_coef_response(RACE_TYPE_RESPONSE, anc_ret, p_resp_data->real_data_len, p_resp_data);

            if (p_resp_data) {
                vPortFree(p_resp_data);
            }
            if (real_data) {
                vPortFree(real_data);
            }
            break;
        }
        case RACE_FADP_ANC_QUERY_COEF_CMD_SUB_MODE_FIR_COEF: {
            typedef struct {
                int32_t fir_coef[160]; //160 OK, 170 NG
            } PACKED fir_real_data1_t; //160*4=640
            typedef struct {
                int32_t fir_coef[140];
                int32_t reserved[20];
            } PACKED fir_real_data2_t; //160*4=640

            if (p_cmd_data->seq_num == 1) {
                memset(&g_race_fadp_fir_coef, 0, sizeof(g_race_fadp_fir_coef));
                audio_anc_control_dump_full_adaptive_coef((uint8_t*)&g_race_fadp_fir_coef, NULL, NULL, NULL);

                #if RACE_FADP_DUMP_COEF_DEBUG //debug use
                for (int dump_i = 0; dump_i < (300); dump_i += 4) {
                    //hal_gpt_delay_us(150);
                    RACE_LOG_MSGID_I("[FADP_get]get_fir_coef, index(%d): %d, %d, %d, %d", 5, dump_i, g_race_fadp_fir_coef[dump_i], g_race_fadp_fir_coef[dump_i+1], g_race_fadp_fir_coef[dump_i+2], g_race_fadp_fir_coef[dump_i+3]);
                }
                #endif

                resp_data_len = sizeof(RACE_RSP_FADP_ANC_QUERY_COEF_PARAM)+sizeof(fir_real_data1_t)-1; //9+4*190-1=768
                p_resp_data = (RACE_RSP_FADP_ANC_QUERY_COEF_PARAM*)pvPortMalloc(resp_data_len);
                if (p_resp_data == NULL) {
                    anc_ret = AUDIO_ANC_CONTROL_EXECUTION_FAIL;
                    RACE_LOG_MSGID_E("[FADP_get] malloc fail!", 0);
                    goto _HDR_END;
                }

                memset(p_resp_data, 0, resp_data_len);
                p_resp_data->data_len = resp_data_len - 4; //4=sizeof(header)+sizeof(p_resp_data->data_len)
                p_resp_data->real_data_len = sizeof(fir_real_data1_t);
                memcpy(&p_resp_data->real_data, &g_race_fadp_fir_coef, sizeof(fir_real_data1_t));

            } else if (p_cmd_data->seq_num == 2) {
                resp_data_len = sizeof(RACE_RSP_FADP_ANC_QUERY_COEF_PARAM)+sizeof(fir_real_data2_t)-1; //7+4*150-1=608
                p_resp_data = (RACE_RSP_FADP_ANC_QUERY_COEF_PARAM*)pvPortMalloc(resp_data_len);
                if (p_resp_data == NULL) {
                    anc_ret = AUDIO_ANC_CONTROL_EXECUTION_FAIL;
                    RACE_LOG_MSGID_E("[FADP_get] malloc fail!", 0);
                    goto _HDR_END;
                }

                memset(p_resp_data, 0, resp_data_len);
                p_resp_data->data_len = resp_data_len - 4; //4=sizeof(header)+sizeof(p_resp_data->data_len)
                p_resp_data->real_data_len = sizeof(fir_real_data2_t);
                memcpy(&p_resp_data->real_data, &g_race_fadp_fir_coef[160], sizeof(int)*140);
            } else {
                RACE_LOG_MSGID_E("[FADP_get]seq_num err: %d ", 1, p_cmd_data->seq_num);
                anc_ret = AUDIO_ANC_CONTROL_EXECUTION_FAIL;
                goto _HDR_END;
            }

            //RACE_LOG_MSGID_I("[FADP_get] get_coef_cmd_hdr, resp_data_len %d, data_len %d, real_data_len %d", 3, resp_data_len, p_resp_data->data_len, p_resp_data->real_data_len);
            p_resp_data->sub_mode = RACE_FADP_ANC_QUERY_COEF_CMD_SUB_MODE_FIR_COEF;
            p_resp_data->seq_num = p_cmd_data->seq_num;
            p_resp_data->header.status = anc_ret;
            p_resp_data->header.ancId = RACE_FADP_ANC_QUERY_COEF;
            //RESPONSE TO SP APP BEFORE FREE!!
            race_dsprealtime_full_adaptive_get_coef_response(RACE_TYPE_RESPONSE, anc_ret, p_resp_data->real_data_len, p_resp_data);
            if (p_resp_data) {
                vPortFree(p_resp_data);
            }
            break;
        }
        default:
            anc_ret = AUDIO_ANC_CONTROL_EXECUTION_NOT_ALLOWED;
            break;
    }
_HDR_END:
    return anc_ret;
}


void *RACE_DSPREALTIME_FADP_ANC_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    race_dsprealtime_anc_struct *pEvt = NULL;
    g_anc_race_ch_id = channel_id;

    race_dsprealtime_anc_struct *pAnc_cmd;
    audio_anc_control_result_t anc_ret = AUDIO_ANC_CONTROL_EXECUTION_NONE;
    pAnc_cmd = (race_dsprealtime_anc_struct *)pCmdMsg->payload;
    g_anc_race_cmd_length = pCmdMsg->hdr.length - 4;
    //RACE_LOG_MSGID_I("RACE_DSPREALTIME_ANC_HDR msg_length:%d payload: %d %d %d\n",4,pCmdMsg->hdr.length,pAnc_cmd->status,pAnc_cmd->param.header.ancId,pAnc_cmd->param.anc_on_param.anc_filter_type);
    RACE_LOG_MSGID_E("[RACE][ANC_PTHR]channel_id: %d, ANC_id: %d\n", 2, channel_id, pAnc_cmd->param.header.ancId);

    if (pCmdMsg->hdr.length >= 4) { //hdr.length = hdr.id(2bytes) + pAnc_cmd->status(1byte) + pAnc_cmd->anc_id(1byte) + pAnc_cmd->param(0/1/2/4 bytes)
        switch (pAnc_cmd->param.header.ancId) {
            case RACE_FADP_ANC_ENABLE: {
                pAnc_cmd->param.header.ancId =  (0x80 | pAnc_cmd->param.header.ancId);
                anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_RACE, 0, (void *)pAnc_cmd);
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_FADP_ANC,
                                        sizeof(RACE_RSP_FADP_ANC_ENABLE_PARAM),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_FADP_ANC_ENABLE_PARAM));
                } else {
                    goto _HDR_END;
                }
                pEvt->param.enableRsp.header.ancId  = RACE_FADP_ANC_ENABLE;
                pEvt->param.enableRsp.header.status = anc_ret;
                break;
            }
            case RACE_FADP_ANC_FREEZE: {
                pAnc_cmd->param.header.ancId =  (0x80 | pAnc_cmd->param.header.ancId);
                anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_RACE, 0, (void *)pAnc_cmd);
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_FADP_ANC,
                                        sizeof(RACE_RSP_FADP_ANC_FREEZE_PARAM),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_FADP_ANC_FREEZE_PARAM));
                } else {
                    goto _HDR_END;
                }
                pEvt->param.freezeRsp.header.ancId  = RACE_FADP_ANC_FREEZE;
                pEvt->param.freezeRsp.header.status = anc_ret;
                break;
            }
            case RACE_FADP_ANC_DISABLE: {
                pAnc_cmd->param.header.ancId =  (0x80 | pAnc_cmd->param.header.ancId);
                anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_RACE, 0, (void *)pAnc_cmd);
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_FADP_ANC,
                                        sizeof(RACE_RSP_FADP_ANC_DISABLE_PARAM),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_FADP_ANC_DISABLE_PARAM));
                } else {
                    goto _HDR_END;
                }
                pEvt->param.disableRsp.header.ancId  = RACE_FADP_ANC_DISABLE;
                pEvt->param.disableRsp.header.status = anc_ret;
                break;
            }
            case RACE_FADP_ANC_GET_INFO: {
                pAnc_cmd->param.header.ancId =  (0x80 | pAnc_cmd->param.header.ancId);
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_FADP_ANC,
                                        sizeof(RACE_RSP_FADP_ANC_GET_INFO_PARAM),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_FADP_ANC_GET_INFO_PARAM));
                } else {
                    goto _HDR_END;
                }
                anc_ret = AUDIO_ANC_CONTROL_EXECUTION_SUCCESS;
                audio_anc_control_get_full_adaptive_info(&(pEvt->param.getInfoRsp.FADP_ANC_state), &(pEvt->param.getInfoRsp.sub_event),
                                                         &(pEvt->param.getInfoRsp.rate), NULL);
                #ifdef AIR_ANC_ADAPTIVE_EVENT_DRIVEN_ENABLE
                pEvt->param.getInfoRsp.Reserve_3 = g_FADP_ADAPTIVE_TIME_MS;
                #endif
                #ifdef AIR_FADP_ANC_COMPENSATION_ENABLE
                pEvt->param.getInfoRsp.lc_setting = g_FADP_USE_DEFAULT_FIR;
                #else
                pEvt->param.getInfoRsp.lc_setting = 1;
                #endif
                pEvt->param.getInfoRsp.header.ancId   = RACE_FADP_ANC_GET_INFO;
                pEvt->param.getInfoRsp.header.status  = anc_ret;
                break;
            }
            case RACE_FADP_ANC_QUERY_STATUS: {
                pAnc_cmd->param.header.ancId =  (0x80 | pAnc_cmd->param.header.ancId);
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_FADP_ANC,
                                        sizeof(RACE_RSP_FADP_ANC_QUERY_STATUS_PARAM),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_FADP_ANC_QUERY_STATUS_PARAM));
                } else {
                    goto _HDR_END;
                }
                anc_ret = AUDIO_ANC_CONTROL_EXECUTION_SUCCESS;

                audio_anc_control_get_full_adaptive_process_status(&(pEvt->param.queryStatusRsp.Process_state_l),
                                                                   &(pEvt->param.queryStatusRsp.Process_state_r),
                                                                   NULL);

                pEvt->param.getInfoRsp.header.ancId   = RACE_FADP_ANC_QUERY_STATUS;
                pEvt->param.getInfoRsp.header.status  = anc_ret;
                break;
            }
            case RACE_FADP_ANC_QUERY_COEF: {
                //request FIR coef or IIR UI coef
                pAnc_cmd->param.header.ancId = (0x80 | pAnc_cmd->param.header.ancId);
                RACE_CMD_FADP_ANC_QUERY_COEF_PARAM *queryCoefCmd = (RACE_CMD_FADP_ANC_QUERY_COEF_PARAM *)pAnc_cmd;

                anc_ret = race_dsprealtime_full_adaptive_get_coef_cmd_hdr(queryCoefCmd);

                    goto _HDR_END;
                break;
            }
#ifdef AIR_HW_VIVID_PT_ENABLE
            case RACE_HW_VIVID_PT_ENABLE: {
                pAnc_cmd->param.header.ancId =  (0x80 | pAnc_cmd->param.header.ancId);
                anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_RACE, 0, (void *)pAnc_cmd);
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_FADP_ANC,
                                        sizeof(RACE_RSP_FADP_ANC_ENABLE_PARAM),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_FADP_ANC_ENABLE_PARAM));
                } else {
                    goto _HDR_END;
                }
                pEvt->param.enableRsp.header.ancId  = RACE_HW_VIVID_PT_ENABLE;
                pEvt->param.enableRsp.header.status = anc_ret;
                break;
            }
            case RACE_HW_VIVID_PT_FREEZE: {
                pAnc_cmd->param.header.ancId =  (0x80 | pAnc_cmd->param.header.ancId);
                anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_RACE, 0, (void *)pAnc_cmd);
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_FADP_ANC,
                                        sizeof(RACE_RSP_FADP_ANC_FREEZE_PARAM),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_FADP_ANC_FREEZE_PARAM));
                } else {
                    goto _HDR_END;
                }
                pEvt->param.freezeRsp.header.ancId  = RACE_HW_VIVID_PT_FREEZE;
                pEvt->param.freezeRsp.header.status = anc_ret;
                break;
            }
            case RACE_HW_VIVID_PT_DISABLE: {
                pAnc_cmd->param.header.ancId =  (0x80 | pAnc_cmd->param.header.ancId);
                anc_ret = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_RACE, 0, (void *)pAnc_cmd);
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_FADP_ANC,
                                        sizeof(RACE_RSP_FADP_ANC_DISABLE_PARAM),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_FADP_ANC_DISABLE_PARAM));
                } else {
                    goto _HDR_END;
                }
                pEvt->param.disableRsp.header.ancId  = RACE_HW_VIVID_PT_DISABLE;
                pEvt->param.disableRsp.header.status = anc_ret;
                break;
            }
            case RACE_HW_VIVID_PT_GET_INFO: {
                pAnc_cmd->param.header.ancId =  (0x80 | pAnc_cmd->param.header.ancId);
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_FADP_ANC,
                                        sizeof(RACE_RSP_FADP_ANC_GET_INFO_PARAM),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_FADP_ANC_GET_INFO_PARAM));
                } else {
                    goto _HDR_END;
                }
                anc_ret = AUDIO_ANC_CONTROL_EXECUTION_NOT_ALLOWED;

                pEvt->param.getInfoRsp.header.ancId   = RACE_HW_VIVID_PT_GET_INFO;
                pEvt->param.getInfoRsp.header.status  = anc_ret;
                break;
            }
            case RACE_HW_VIVID_PT_QUERY_STATUS: {
                pAnc_cmd->param.header.ancId =  (0x80 | pAnc_cmd->param.header.ancId);
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_FADP_ANC,
                                        sizeof(RACE_RSP_FADP_ANC_QUERY_STATUS_PARAM),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_RSP_FADP_ANC_QUERY_STATUS_PARAM));
                } else {
                    goto _HDR_END;
                }
                anc_ret = AUDIO_ANC_CONTROL_EXECUTION_NOT_ALLOWED;

                pEvt->param.getInfoRsp.header.ancId   = RACE_HW_VIVID_PT_QUERY_STATUS;
                pEvt->param.getInfoRsp.header.status  = anc_ret;
                break;
            }
            case RACE_HW_VIVID_PT_QUERY_COEF: {
                //request FIR coef or IIR UI coef
                pAnc_cmd->param.header.ancId = (0x80 | pAnc_cmd->param.header.ancId);
                    goto _HDR_END;
                break;
            }
#endif
            default: {
                pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                        RACE_DSPREALTIME_FADP_ANC,
                                        sizeof(RACE_ANC_PASSTHRU_HEADER),
                                        channel_id);
                if (pEvt != NULL) {
                    memset(pEvt, 0, sizeof(RACE_ANC_PASSTHRU_HEADER));
                } else {
                    goto _HDR_END;
                }
                pEvt->param.header.status = RACE_ERRCODE_NOT_SUPPORT;
                return pEvt;
                break;
            }
        }
        if (pEvt != NULL) {
            pEvt->param.header.status = (anc_ret == AUDIO_ANC_CONTROL_EXECUTION_SUCCESS) ? RACE_ERRCODE_SUCCESS : RACE_ERRCODE_FAIL;
        }
    } else {
        pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                RACE_DSPREALTIME_FADP_ANC,
                                sizeof(RACE_ANC_PASSTHRU_HEADER),
                                channel_id);
        if (pEvt != NULL) {
            memset(pEvt, 0, sizeof(RACE_ANC_PASSTHRU_HEADER));
        } else {
            goto _HDR_END;
        }
        pEvt->param.header.status = RACE_ERRCODE_PARAMETER_ERROR;
    }

    if (pEvt != NULL) {
        if (pEvt->param.header.status != RACE_ERRCODE_SUCCESS) {
            RACE_LOG_MSGID_E("RACE_DSPREALTIME_ANC_HDR pEvt->status: %d  anc_ret: %d\n", 2, pEvt->param.header.status, anc_ret);
        }
    }
_HDR_END:
    return pEvt;
}
#endif
#endif


#ifdef ADAPIVE_ANC_STREAM_CONTROL
static void race_dsprealtime_adaptive_anc_stream_control_send_notify(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    //send 5D
    typedef struct {
        adaptive_check_notify_t header;
        uint8_t status;
    } PACKED RACE_DSPREALTIME_USER_UNAWARE_NOTI_STRU;

    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint8_t mode;
        uint8_t enable;
    } PACKED RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_CMD_STRU;

    RACE_DSPREALTIME_USER_UNAWARE_NOTI_STRU *pNoti = NULL;
    RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_CMD_STRU *pCmd = (RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_CMD_STRU *)pCmdMsg;
    bt_aws_mce_role_t role;
    pNoti = RACE_ClaimPacket(RACE_TYPE_NOTIFICATION, pCmd->Hdr.id, sizeof(RACE_DSPREALTIME_USER_UNAWARE_NOTI_STRU), channel_id);
    if (pNoti) {
        memset((uint8_t *) & (pNoti->header.start_or_stop), 0, sizeof(RACE_DSPREALTIME_USER_UNAWARE_NOTI_STRU));
        pNoti->header.mode = pCmd->mode; //ADAPTIVE_ANC_STREAM_MODE;
        pNoti->header.total_data_length = 1;
        pNoti->header.data_length = 1;
        pNoti->status = 0;
        role = bt_connection_manager_device_local_info_get_aws_role();
        if ((role == BT_AWS_MCE_ROLE_AGENT) || (role == BT_AWS_MCE_ROLE_NONE)) {
            race_flush_packet((void *)pNoti, channel_id);
        } else {
            race_send_pkt_t *pSndPkt;
            pSndPkt = race_pointer_cnv_pkt_to_send_pkt((void *)pNoti);
            race_pkt_t      *pret;
            race_send_pkt_t *psend;
            psend = (race_send_pkt_t *)pSndPkt;
            pret = &psend->race_data;
#if (RACE_DEBUG_PRINT_ENABLE)
            race_dump((uint8_t *)pret, RACE_DBG_EVT);
#endif
            if (pSndPkt) {
                bt_status_t ret = RACE_ERRCODE_SUCCESS;
                #ifdef RACE_RELAY_CMD_ENABLE
                ret = bt_send_aws_mce_race_cmd_data(&pSndPkt->race_data, pSndPkt->length, channel_id, RACE_CMD_RSP_FROM_PARTNER, 0);
                #endif
                if (ret != BT_STATUS_SUCCESS) {
                    RACE_LOG_MSGID_E("[relay_cmd][Adapt_ANC]partner send relay req FAIL \n", 0);
                }
                race_mem_free(pSndPkt);
            }
        }
    }
}
#endif


void *RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint8_t mode;
        uint8_t enable;
    } PACKED RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_CMD_STRU;

    typedef struct {
        uint8_t status;
        uint8_t mode;
        uint8_t enable;
    } PACKED RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_EVT_STRU;

    RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_CMD_STRU *pCmd = (RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_CMD_STRU *)pCmdMsg;
    RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_EVT_STRU *pEvt = NULL;

    switch (pCmd->mode) {
#ifdef MTK_LEAKAGE_DETECTION_ENABLE
        case LEAKAGE_DETECTION_MODE: {
            S8 terminate_cause = 0;
            g_leakage_detection_race_ch_id = channel_id;
            audio_anc_leakage_detection_send_aws_mce_race_ch_id(g_leakage_detection_race_ch_id);
            //send race ch id to partner to prevent from sending race cmd fail in RHO case.

            race_anc_mutex_take();
            g_LD_result_agent = 0;
            g_LD_result_partner = 0;
            race_anc_mutex_give();
            pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE, pCmd->Hdr.id, sizeof(RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_EVT_STRU), channel_id);
            if (pEvt) {
                pEvt->mode = LEAKAGE_DETECTION_MODE;
                pEvt->enable = pCmd->enable;
                pEvt->status = 0;
                if (pCmd->enable == 1) {
                    if ((bt_sink_srv_ami_get_current_scenario() == HFP) ||
                        (ami_hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_BLE_UL))) {
                        terminate_cause = -1;
                        race_flush_packet((void *)pEvt, g_leakage_detection_race_ch_id);
                        anc_leakage_detection_racecmd_response(LD_STATUS_TERMINATE, 0);
                        anc_leakage_detection_racecmd_response(LD_STATUS_TERMINATE, 1);
                        pEvt = NULL;
                    } else {
#ifndef MTK_ANC_V2
                        audio_anc_control_result_t anc_ret = audio_anc_leakage_detection_start(anc_leakage_detection_racecmd_callback);
#else
                        audio_anc_control_result_t anc_ret = audio_anc_leakage_detection_prepare(anc_leakage_detection_racecmd_callback);
#endif
                        if (anc_ret != AUDIO_ANC_CONTROL_EXECUTION_SUCCESS) {
                            terminate_cause = -2;
                            anc_leakage_detection_racecmd_response(LD_STATUS_TERMINATE, 0);
                        }
                        pEvt->status = (anc_ret == AUDIO_ANC_CONTROL_EXECUTION_SUCCESS) ? RACE_ERRCODE_SUCCESS : RACE_ERRCODE_FAIL;
                    }
                } else {
                    audio_anc_control_result_t anc_ret = audio_anc_leakage_detection_stop();
                    pEvt->status = (anc_ret == AUDIO_ANC_CONTROL_EXECUTION_SUCCESS) ? RACE_ERRCODE_SUCCESS : RACE_ERRCODE_FAIL;
                    terminate_cause = -3;
                }
                if (terminate_cause != 0) {
                    RACE_LOG_MSGID_I("[RECORD_LC] anc leakage detection is terminated by %d", 1, terminate_cause);
                }
            }
            break;
        }
#ifdef AIR_FADP_ANC_COMPENSATION_ENABLE
        case FADP_ANC_COMPENSATION_MODE: {
            g_fadp_anc_compensation_race_ch_id = channel_id;
            race_anc_mutex_take();
            g_SzD_result_L = 0;
            g_SzD_result_R = 0;
            race_anc_mutex_give();
            pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE, pCmd->Hdr.id, sizeof(RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_EVT_STRU), channel_id);
            if (pEvt) {
                pEvt->mode = FADP_ANC_COMPENSATION_MODE;
                pEvt->enable = pCmd->enable;
                pEvt->status = 0;
                if (pCmd->enable == 1) {
                    if (bt_sink_srv_ami_get_current_scenario() == HFP) {
                        RACE_LOG_MSGID_I("[RECORD_SzD] FADP ANC compensation is terminated by HFP", 0);
                        race_flush_packet((void *)pEvt, g_fadp_anc_compensation_race_ch_id);
                        anc_fadp_compensation_racecmd_response(LD_STATUS_TERMINATE, 0);
                        pEvt = NULL;
                    } else {
                        RACE_LOG_MSGID_I("[RECORD_SzD] receive race cmd", 0);
                        audio_anc_leakage_detection_execution_t anc_ret = audio_fadp_anc_compensation_prepare(anc_fadp_compensation_racecmd_callback);

                        if (anc_ret != AUDIO_LEAKAGE_DETECTION_EXECUTION_SUCCESS) {
                            if (anc_ret == AUDIO_LEAKAGE_DETECTION_EXECUTION_NOT_ALLOWED) {
                                anc_fadp_compensation_racecmd_response(LD_STATUS_TERMINATE, LD_STATUS_TERMINATE);
                                RACE_LOG_MSGID_I("[RECORD_SzD] FADP ANC compensation is not allowed:%d", 1, anc_ret);
                            } else {
                                anc_fadp_compensation_racecmd_response(LD_STATUS_TERMINATE, 0);
                                RACE_LOG_MSGID_I("[RECORD_SzD] FADP ANC compensation is terminate:%d", 1, anc_ret);
                            }
                        }
                        pEvt->status = (anc_ret == AUDIO_LEAKAGE_DETECTION_EXECUTION_SUCCESS) ? RACE_ERRCODE_SUCCESS : RACE_ERRCODE_FAIL;
                    }
                } else {
                    audio_anc_control_result_t anc_ret = audio_fadp_anc_compensation_send_stop();
                    pEvt->status = (anc_ret == AUDIO_ANC_CONTROL_EXECUTION_SUCCESS) ? RACE_ERRCODE_SUCCESS : RACE_ERRCODE_FAIL;
                    RACE_LOG_MSGID_I("[RECORD_SzD] FADP ANC compensation is terminated by end-user", 0);
                }
            }
            break;
        }
        case ANC_DEFAULT_FIR_MODE: {
            pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE, pCmd->Hdr.id, sizeof(RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_EVT_STRU), channel_id);
            if (pEvt) {
                pEvt->mode = ANC_DEFAULT_FIR_MODE;
                pEvt->enable = pCmd->enable;
                pEvt->status = 0;
                if (pCmd->enable == 1) {
                    g_FADP_USE_DEFAULT_FIR = 1;
                    RACE_LOG_MSGID_I("[RECORD_SzD] ANC use default FIR: %d", 1, g_FADP_USE_DEFAULT_FIR);
                } else {
                    g_FADP_USE_DEFAULT_FIR = 0;
                    RACE_LOG_MSGID_I("[RECORD_SzD] ANC use default FIR: %d", 1, g_FADP_USE_DEFAULT_FIR);
                }
            }
            break;
        }
#endif
#endif
#ifdef MTK_USER_TRIGGER_FF_ENABLE
#ifndef MTK_USER_TRIGGER_ADAPTIVE_FF_V2
        case ADAPTIVE_FF_ANC_MODE: {
            g_anc_race_ch_id = channel_id;
            if (pCmd->enable) {
                /*
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &user_trigger_ff_start_count);
                audio_anc_user_trigger_ff_start(anc_user_trigger_adaptive_ff_callback);
                */
                race_send_event_notify_msg(RACE_EVENT_TYPE_ANC_ADAPTIVE_FF_START, NULL); //Add new api to app do start
                RACE_FreePacket(pEvt);
                pEvt = NULL;
            } else {/*cancelled by user*/
                if (audio_anc_user_trigger_ff_get_status()) {
                    audio_anc_user_trigger_ff_stop();
                    audio_anc_user_trigger_ff_recover_anc(ANC_K_STATUS_KEEP_DEFAULT);
                    pEvt->status = 0;
                    race_send_event_notify_msg(RACE_EVENT_TYPE_ANC_ADAPTIVE_FF_CANCEL, NULL);
                }
            }
            break;
        }
#else
        case ADAPTIVE_FF_ANC_MODE: {
            g_anc_race_ch_id = channel_id;
#if 0
            pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE, pCmd->Hdr.id, sizeof(RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_EVT_STRU), channel_id);
            if (pEvt) {
                pEvt->mode = ADAPTIVE_FF_ANC_MODE;
                pEvt->enable = pCmd->enable;
                pEvt->status = 0;
                if (pCmd->enable) {
                    audio_user_trigger_adaptive_ff_register_vp_callback(user_trigger_adaptive_ff_vp_callback);

                    user_trigger_adaptive_ff_result_t reg_cb_res = audio_user_trigger_adaptive_ff_register_event_callback((anc_user_trigger_ff_event_callback_t)user_trigger_adaptive_ff_event_callback, USER_TRIGGER_ADAPTIVE_FF_EVENT_ALL);

                    user_trigger_adaptive_ff_result_t res = audio_user_trigger_adaptive_ff_request((anc_user_trigger_ff_callback_t)user_trigger_adaptive_ff_racecmd_response);
                    pEvt->status = res;

                } else {/*cancelled by user*/
                    audio_user_trigger_adaptive_ff_terminate();
                }
            }
#endif
            if (pCmd->enable) {
                race_send_event_notify_msg(RACE_EVENT_TYPE_ANC_ADAPTIVE_FF_START, NULL);
            } else {
                race_send_event_notify_msg(RACE_EVENT_TYPE_ANC_ADAPTIVE_FF_CANCEL, NULL);
            }
            pEvt = NULL;
            break;
        }
#endif
#endif
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
        case USER_UNAWARE_DUBUG_MODE: {
            typedef struct {
                uint8_t status;
                uint8_t mode;
                uint8_t enable;
                user_unaware_info data;
            } PACKED RACE_DSPREALTIME_USER_UNAWARE_EVT_STRU;

            typedef struct {
                adaptive_check_notify_t header;
                uint8_t status;
                user_unaware_info data;
            } PACKED RACE_DSPREALTIME_USER_UNAWARE_NOTI_STRU;

            uint8_t *uu_info = NULL;
            bt_aws_mce_role_t role;
            bt_status_t ret = BT_STATUS_FAIL;
            //send 5B
            pEvt = NULL;
            RACE_DSPREALTIME_USER_UNAWARE_EVT_STRU *pEvtUU = NULL;
            pEvtUU = RACE_ClaimPacket(RACE_TYPE_RESPONSE, pCmd->Hdr.id, sizeof(RACE_DSPREALTIME_USER_UNAWARE_EVT_STRU), channel_id);
            if (pEvtUU) {
                pEvtUU->mode = USER_UNAWARE_DUBUG_MODE;
                pEvtUU->enable = pCmd->enable;
                pEvtUU->status = 0;

                uu_info = pvPortMallocNC(sizeof(user_unaware_info));
                if (uu_info) {
                    memset(uu_info, 0, sizeof(user_unaware_info));
                    audio_user_unaware_get_gain_info(uu_info);
                    memcpy((uint8_t *) & (pEvtUU->data), uu_info, sizeof(user_unaware_info));
                } else {
                    pEvtUU->status = 0xFF;
                    memset((uint8_t *) & (pEvtUU->data), 0, sizeof(user_unaware_info));
                }
                role = bt_connection_manager_device_local_info_get_aws_role();
                if ((role == BT_AWS_MCE_ROLE_AGENT) || (role == BT_AWS_MCE_ROLE_NONE)) {
                    race_flush_packet((void *)pEvtUU, channel_id);
                } else {
                    race_send_pkt_t *pSndPkt;
                    pSndPkt = race_pointer_cnv_pkt_to_send_pkt((void *)pEvtUU);
                    race_pkt_t      *pret;
                    race_send_pkt_t *psend;
                    psend = (race_send_pkt_t *)pSndPkt;
                    pret = &psend->race_data;
#if (RACE_DEBUG_PRINT_ENABLE)
                    race_dump((uint8_t *)pret, RACE_DBG_EVT);
#endif
                    if (pSndPkt) {
                        #ifdef RACE_RELAY_CMD_ENABLE
                        ret = bt_send_aws_mce_race_cmd_data(&pSndPkt->race_data, pSndPkt->length, channel_id, RACE_CMD_RSP_FROM_PARTNER, 0);
                        #endif
                        if (ret != BT_STATUS_SUCCESS) {
                            RACE_LOG_MSGID_E("[relay_cmd][User Unaware]partner send relay req FAIL \n", 0);
                        }
                        race_mem_free(pSndPkt);
                    }
                }
            }
            //send 5D
            RACE_DSPREALTIME_USER_UNAWARE_NOTI_STRU *pNotiUU = RACE_ClaimPacket(RACE_TYPE_NOTIFICATION, pCmd->Hdr.id, sizeof(RACE_DSPREALTIME_USER_UNAWARE_NOTI_STRU), channel_id);
            if (pNotiUU) {
                memset((uint8_t *) & (pNotiUU->header.start_or_stop), 0, sizeof(RACE_DSPREALTIME_USER_UNAWARE_NOTI_STRU));
                pNotiUU->header.mode = USER_UNAWARE_DUBUG_MODE;
                pNotiUU->header.total_data_length = sizeof(user_unaware_info) + 1;
                pNotiUU->header.data_length = sizeof(user_unaware_info) + 1;

                if (uu_info) {
                    pNotiUU->status = 0;
                    memcpy((uint8_t *) & (pNotiUU->data), uu_info, sizeof(user_unaware_info));
                } else {
                    pNotiUU->status = 0xFF;
                    memset((uint8_t *) & (pNotiUU->data), 0, sizeof(user_unaware_info));
                }
                role = bt_connection_manager_device_local_info_get_aws_role();
                if ((role == BT_AWS_MCE_ROLE_AGENT) || (role == BT_AWS_MCE_ROLE_NONE)) {
                    race_flush_packet((void *)pNotiUU, channel_id);
                } else {
                    race_send_pkt_t *pSndPkt;
                    pSndPkt = race_pointer_cnv_pkt_to_send_pkt((void *)pNotiUU);
                    race_pkt_t      *pret;
                    race_send_pkt_t *psend;
                    psend = (race_send_pkt_t *)pSndPkt;
                    pret = &psend->race_data;
#if (RACE_DEBUG_PRINT_ENABLE)
                    race_dump((uint8_t *)pret, RACE_DBG_EVT);
#endif
                    if (pSndPkt) {
                        #ifdef RACE_RELAY_CMD_ENABLE
                        ret = bt_send_aws_mce_race_cmd_data(&pSndPkt->race_data, pSndPkt->length, channel_id, RACE_CMD_RSP_FROM_PARTNER, 0);
                        #endif
                        if (ret != BT_STATUS_SUCCESS) {
                            RACE_LOG_MSGID_E("[relay_cmd][User Unaware]partner send relay req FAIL \n", 0);
                        }
                        race_mem_free(pSndPkt);
                    }
                }
            }
            if (uu_info) {
                vPortFreeNC(uu_info);
            }
            return NULL;
            break;
        }
        case USER_UNAWARE_MODE: {
            bt_status_t ret1 = BT_SINK_SRV_STATUS_FAIL;
            bt_status_t ret2 = BT_SINK_SRV_STATUS_FAIL;
            bt_aws_mce_role_t role;
            bt_status_t ret = BT_STATUS_FAIL;
            //send 5B
            pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE, pCmd->Hdr.id, sizeof(RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_EVT_STRU), channel_id);
            if (pEvt) {
                pEvt->enable = pCmd->enable;
                pEvt->mode = pCmd->mode;
                pEvt->status = 0;
                if (pCmd->enable) {
                    //Enable UU
                    audio_user_unaware_save_enable_status_to_nvdm(true);
                    audio_user_unaware_enable(true);
                } else {
                    //Disable UU
                    audio_user_unaware_save_enable_status_to_nvdm(false);
                    audio_user_unaware_enable(false);
                }
                role = bt_connection_manager_device_local_info_get_aws_role();
                if ((role == BT_AWS_MCE_ROLE_AGENT) || (role == BT_AWS_MCE_ROLE_NONE)) {
                    race_flush_packet((void *)pEvt, channel_id);
                } else {
                    race_send_pkt_t *pSndPkt;
                    pSndPkt = race_pointer_cnv_pkt_to_send_pkt((void *)pEvt);
                    race_pkt_t      *pret;
                    race_send_pkt_t *psend;
                    psend = (race_send_pkt_t *)pSndPkt;
                    pret = &psend->race_data;
#if (RACE_DEBUG_PRINT_ENABLE)
                    race_dump((uint8_t *)pret, RACE_DBG_EVT);
#endif
                    if (pSndPkt) {
                        #ifdef RACE_RELAY_CMD_ENABLE
                        ret = bt_send_aws_mce_race_cmd_data(&pSndPkt->race_data, pSndPkt->length, channel_id, RACE_CMD_RSP_FROM_PARTNER, 0);
                        #endif
                        if (ret != BT_STATUS_SUCCESS) {
                            RACE_LOG_MSGID_E("[relay_cmd][User Unaware]partner send relay req FAIL \n", 0);
                        }
                        race_mem_free(pSndPkt);
                    }
                }
                pEvt = NULL;
            }
            //send 5D
            typedef struct {
                adaptive_check_notify_t header;
                uint8_t status;
            } PACKED RACE_DSPREALTIME_USER_UNAWARE_NOTI_STRU;
            RACE_DSPREALTIME_USER_UNAWARE_NOTI_STRU *pNoti = NULL;
            pNoti = RACE_ClaimPacket(RACE_TYPE_NOTIFICATION, pCmd->Hdr.id, sizeof(RACE_DSPREALTIME_USER_UNAWARE_NOTI_STRU), channel_id);
            if (pNoti) {
                memset((uint8_t *) & (pNoti->header.start_or_stop), 0, sizeof(RACE_DSPREALTIME_USER_UNAWARE_NOTI_STRU));
                pNoti->header.mode = USER_UNAWARE_MODE;
                pNoti->header.total_data_length = 1;
                pNoti->header.data_length = 1;
                pNoti->status = 0;
                role = bt_connection_manager_device_local_info_get_aws_role();
                if ((role == BT_AWS_MCE_ROLE_AGENT) || (role == BT_AWS_MCE_ROLE_NONE)) {
                    race_flush_packet((void *)pNoti, channel_id);
                } else {
                    race_send_pkt_t *pSndPkt;
                    pSndPkt = race_pointer_cnv_pkt_to_send_pkt((void *)pNoti);
                    race_pkt_t      *pret;
                    race_send_pkt_t *psend;
                    psend = (race_send_pkt_t *)pSndPkt;
                    pret = &psend->race_data;
#if (RACE_DEBUG_PRINT_ENABLE)
                    race_dump((uint8_t *)pret, RACE_DBG_EVT);
#endif
                    if (pSndPkt) {
                        #ifdef RACE_RELAY_CMD_ENABLE
                        ret = bt_send_aws_mce_race_cmd_data(&pSndPkt->race_data, pSndPkt->length, channel_id, RACE_CMD_RSP_FROM_PARTNER, 0);
                        #endif
                        if (ret != BT_STATUS_SUCCESS) {
                            RACE_LOG_MSGID_E("[relay_cmd][User Unaware]partner send relay req FAIL \n", 0);
                        }
                        race_mem_free(pSndPkt);
                    }
                }
            }

            RACE_LOG_MSGID_I("[User Unaware]enable %d by race, send relay req status %d, %d", 3, pCmd->enable, ret1, ret2);
            break;
        }
#endif
#ifdef ADAPIVE_ANC_STREAM_CONTROL
        case ADAPTIVE_ANC_STREAM_MODE: {
            bt_aws_mce_role_t role;
            //send 5B
            pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE, pCmd->Hdr.id, sizeof(RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_EVT_STRU), channel_id);
            if (pEvt) {
                pEvt->enable = pCmd->enable;
                pEvt->mode = pCmd->mode;
                pEvt->status = 0;
                if (pCmd->enable) { //enable stream
                    g_adaptive_ANC_stream_suspend_control_flag = false; //resume Adaptive ANC Stream

#ifdef AIR_ANC_WIND_DETECTION_ENABLE
                    //wind enable
                    audio_wind_detection_enable(true);
                    audio_anc_monitor_adaptive_anc_stream_enable(TRUE);
#else
                    audio_anc_monitor_adaptive_anc_stream_enable(TRUE);
#endif
                } else { //suspend
#ifdef AIR_ANC_WIND_DETECTION_ENABLE
                    //wind disable
                    audio_wind_detection_enable(false);
                    audio_anc_monitor_adaptive_anc_stream_enable(FALSE);
#else
                    audio_anc_monitor_adaptive_anc_stream_enable(FALSE);
                    g_adaptive_ANC_stream_suspend_control_flag = true; //suspend Adaptive ANC Stream
#endif
                    //g_adaptive_ANC_stream_suspend_control_flag = true; //suspend Adaptive ANC Stream
#if 0
                    /* suspend Adaptive ANC Stream , reset extend gain. */
                    audio_anc_control_result_t anc_ret = AUDIO_ANC_CONTROL_EXECUTION_SUCCESS;
                    audio_anc_control_extend_ramp_cap_t init_ramp_cap;
                    init_ramp_cap.extend_gain_1 = 0;
                    init_ramp_cap.extend_gain_2 = 0;
                    init_ramp_cap.gain_type = AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_WIND_NOISE;
                    anc_ret = audio_anc_control_set_extend_gain(init_ramp_cap.gain_type, &init_ramp_cap, NULL);
                    init_ramp_cap.gain_type = AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_USER_UNAWARE;
                    anc_ret = audio_anc_control_set_extend_gain(init_ramp_cap.gain_type, &init_ramp_cap, NULL);
                    init_ramp_cap.gain_type = AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE_ENVIRONMENT_DETECTION;
                    anc_ret = audio_anc_control_set_extend_gain(init_ramp_cap.gain_type, &init_ramp_cap, NULL);
#endif
                }
                role = bt_connection_manager_device_local_info_get_aws_role();
                if ((role == BT_AWS_MCE_ROLE_AGENT) || (role == BT_AWS_MCE_ROLE_NONE)) {
                    race_flush_packet((void *)pEvt, channel_id);
                } else {
                    race_send_pkt_t *pSndPkt;
                    pSndPkt = race_pointer_cnv_pkt_to_send_pkt((void *)pEvt);
                    race_pkt_t      *pret;
                    race_send_pkt_t *psend;
                    psend = (race_send_pkt_t *)pSndPkt;
                    pret = &psend->race_data;
#if (RACE_DEBUG_PRINT_ENABLE)
                    race_dump((uint8_t *)pret, RACE_DBG_EVT);
#endif
                    if (pSndPkt) {
                        bt_status_t ret = BT_STATUS_FAIL;
                        #ifdef RACE_RELAY_CMD_ENABLE
                        ret = bt_send_aws_mce_race_cmd_data(&pSndPkt->race_data, pSndPkt->length, channel_id, RACE_CMD_RSP_FROM_PARTNER, 0);
                        #endif
                        if (ret != BT_STATUS_SUCCESS) {
                            RACE_LOG_MSGID_E("[relay_cmd][Adapt_ANC]partner send relay req FAIL \n", 0);
                        }
                        race_mem_free(pSndPkt);
                    }
                }
                pEvt = NULL;
            }
            //send 5D
            typedef struct {
                adaptive_check_notify_t header;
                uint8_t status;
            } PACKED RACE_DSPREALTIME_USER_UNAWARE_NOTI_STRU;
            RACE_DSPREALTIME_USER_UNAWARE_NOTI_STRU *pNoti = NULL;
            pNoti = RACE_ClaimPacket(RACE_TYPE_NOTIFICATION, pCmd->Hdr.id, sizeof(RACE_DSPREALTIME_USER_UNAWARE_NOTI_STRU), channel_id);
            if (pNoti) {
                memset((uint8_t *) & (pNoti->header.start_or_stop), 0, sizeof(RACE_DSPREALTIME_USER_UNAWARE_NOTI_STRU));
                pNoti->header.mode = ADAPTIVE_ANC_STREAM_MODE;
                pNoti->header.total_data_length = 1;
                pNoti->header.data_length = 1;
                pNoti->status = 0;
                role = bt_connection_manager_device_local_info_get_aws_role();
                if ((role == BT_AWS_MCE_ROLE_AGENT) || (role == BT_AWS_MCE_ROLE_NONE)) {
                    race_flush_packet((void *)pNoti, channel_id);
                } else {
                    race_send_pkt_t *pSndPkt;
                    pSndPkt = race_pointer_cnv_pkt_to_send_pkt((void *)pNoti);
                    race_pkt_t      *pret;
                    race_send_pkt_t *psend;
                    psend = (race_send_pkt_t *)pSndPkt;
                    pret = &psend->race_data;
#if (RACE_DEBUG_PRINT_ENABLE)
                    race_dump((uint8_t *)pret, RACE_DBG_EVT);
#endif
                    if (pSndPkt) {
                        bt_status_t ret = BT_STATUS_FAIL;
                        #ifdef RACE_RELAY_CMD_ENABLE
                        ret = bt_send_aws_mce_race_cmd_data(&pSndPkt->race_data, pSndPkt->length, channel_id, RACE_CMD_RSP_FROM_PARTNER, 0);
                        #endif
                        if (ret != BT_STATUS_SUCCESS) {
                            RACE_LOG_MSGID_E("[relay_cmd][Adapt_ANC]partner send relay req FAIL \n", 0);
                        }
                        race_mem_free(pSndPkt);
                    }
                }
            }
            break;
        }
        case ADAPTIVE_ANC_STREAM_STATUS: {
            bt_aws_mce_role_t role;
            uint8_t adapt_anc_stream_status;
            bt_status_t ret = RACE_ERRCODE_SUCCESS;
            //send 5B
            pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE, pCmd->Hdr.id, sizeof(RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_EVT_STRU), channel_id);
            if (pEvt) {
                pEvt->enable = pCmd->enable;
                pEvt->mode = pCmd->mode;
                pEvt->status = 0;
                audio_anc_monitor_adaptive_anc_stream_get_status(&adapt_anc_stream_status);
                pEvt->enable = adapt_anc_stream_status;

                role = bt_connection_manager_device_local_info_get_aws_role();
                if ((role == BT_AWS_MCE_ROLE_AGENT) || (role == BT_AWS_MCE_ROLE_NONE)) {
                    race_flush_packet((void *)pEvt, channel_id);
                } else {
                    race_send_pkt_t *pSndPkt;
                    pSndPkt = race_pointer_cnv_pkt_to_send_pkt((void *)pEvt);
                    race_pkt_t      *pret;
                    race_send_pkt_t *psend;
                    psend = (race_send_pkt_t *)pSndPkt;
                    pret = &psend->race_data;
#if (RACE_DEBUG_PRINT_ENABLE)
                    race_dump((uint8_t *)pret, RACE_DBG_EVT);
#endif
                    if (pSndPkt) {
                        //bt_status_t ret = BT_STATUS_FAIL;
                        #ifdef RACE_RELAY_CMD_ENABLE
                        ret = bt_send_aws_mce_race_cmd_data(&pSndPkt->race_data, pSndPkt->length, channel_id, RACE_CMD_RSP_FROM_PARTNER, 0);
                        #endif
                        race_mem_free(pSndPkt);
                    }
                }
                pEvt = NULL;
            }
            RACE_LOG_MSGID_I("[RACE][Adapt_ANC] Query Adaptive ANC Stream Status %d, ret %d", 2, adapt_anc_stream_status, ret);
            race_dsprealtime_adaptive_anc_stream_control_send_notify(pCmdMsg, channel_id); //send 5D
            break;
        }
#endif
        default: {
            RACE_LOG_MSGID_E("[RACE] RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_HDR, no such cmd.", 0);
            break;
        }
    }

    return pEvt;
}

#ifdef MTK_USER_TRIGGER_ADAPTIVE_FF_V2
void race_dsprealtime_anc_adaptive_response(bool success, bool enable)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_status_t ret = BT_STATUS_FAIL;

    typedef struct {
        uint8_t status;
        uint8_t mode;
        uint8_t enable;
    } PACKED RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_EVT_STRU;

    RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_EVT_STRU *pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                                                          RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK,
                                                                          sizeof(RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_EVT_STRU),
                                                                          g_anc_race_ch_id);

    if (pEvt) {
        if (success) {
            pEvt->status = 0x00;
        } else {
            pEvt->status = 0xFF;
        }
        pEvt->mode = ADAPTIVE_FF_ANC_MODE;
        pEvt->enable = enable;
        if ((role == BT_AWS_MCE_ROLE_AGENT) || (role == BT_AWS_MCE_ROLE_NONE)) {
            race_flush_packet((void *)pEvt, g_anc_race_ch_id);
        } else {/*partner send agent result*/
            race_send_pkt_t *pSndPkt;
            pSndPkt = (void *)race_pointer_cnv_pkt_to_send_pkt((void *)pEvt);
            race_pkt_t      *pret;
            race_send_pkt_t *psend;
            psend = (race_send_pkt_t *)pSndPkt;
            pret = &psend->race_data;
#if (RACE_DEBUG_PRINT_ENABLE)
            race_dump((uint8_t *)pret, RACE_DBG_EVT);
#endif

            if (pSndPkt) {
                #ifdef RACE_RELAY_CMD_ENABLE
                ret = bt_send_aws_mce_race_cmd_data(&pSndPkt->race_data, pSndPkt->length, g_anc_race_ch_id, RACE_CMD_RSP_FROM_PARTNER, 0);
                #endif
                RACE_LOG_MSGID_I("[relay_cmd][user_trigger_ff]race_dsprealtime_anc_adaptive_response, send status:%d, pEvt_status=%d", 2, ret, pEvt->status);
                race_mem_free(pSndPkt);
            }
        }

        //send 5D
        user_trigger_adaptive_ff_racecmd_response(0, 0xF, NULL, 0, 0, enable, 0);
    }
}
#endif

void *RACE_DSPREALTIME_ANC_ADAPTIVE_RESULT_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint8_t status;
        uint8_t mode;
#ifdef MTK_USER_TRIGGER_ADAPTIVE_FF_V2
        uint8_t data0_id;
        uint16_t data0_len;
        short mobile_status;
        uint8_t data1_id;
        uint16_t data1_len;
        int32_t UU_alpha_para;
#endif
        uint8_t filter[1];
    } PACKED RACE_DSPREALTIME_ANC_ADAPTIVE_RESULT_CMD_STRU;

    typedef struct {
        uint8_t status;
        uint8_t mode;
#ifdef MTK_USER_TRIGGER_ADAPTIVE_FF_V2
        uint8_t filter_selection;
#endif
    } PACKED RACE_DSPREALTIME_ANC_ADAPTIVE_RESULT_EVT_STRU;

    RACE_DSPREALTIME_ANC_ADAPTIVE_RESULT_CMD_STRU *pCmd = (RACE_DSPREALTIME_ANC_ADAPTIVE_RESULT_CMD_STRU *)pCmdMsg;
    RACE_DSPREALTIME_ANC_ADAPTIVE_RESULT_EVT_STRU *pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                                                           pCmd->Hdr.id,
                                                                           sizeof(RACE_DSPREALTIME_ANC_ADAPTIVE_RESULT_EVT_STRU),
                                                                           channel_id);
    g_anc_race_ch_id = channel_id;

    if (pEvt) {
        pEvt->status = 0xFF;
        switch (pCmd->mode) {
#ifdef MTK_USER_TRIGGER_FF_ENABLE
#ifndef MTK_USER_TRIGGER_ADAPTIVE_FF_V2
            case ADAPTIVE_FF_ANC_MODE: {

                if (audio_anc_user_trigger_ff_get_status()) {
                    if (pCmd->status == ANC_K_STATUS_Compare) {
                        uint16_t filter_length = (pCmd->Hdr.length >= 4) ? (pCmd->Hdr.length - 4) : 0; //Hdr.id: 2bytes, status: 1byte, mode: 1byte
#ifdef MTK_USER_TRIGGER_FF_ENABLE
                        audio_anc_user_trigger_ff_receive_filter(anc_user_trigger_adaptive_ff_receive_filter_callback, (uint8_t *)&pCmd->filter, (uint32_t)filter_length);
                        RACE_FreePacket(pEvt);
                        pEvt = NULL;
#endif
                    } else {
                        pEvt->status = ANC_K_STATUS_KEEP_DEFAULT;
                        pEvt->mode = ADAPTIVE_FF_ANC_MODE;

                        //test, force to compare
                        //                    uint32_t buf_size = sizeof(anc_fwd_iir_t);
                        //                    anc_fwd_iir_t *tmp_buffer = (anc_fwd_iir_t *)pvPortMalloc(buf_size);
                        //                    flash_memory_read_nvdm_data(NVKEY_DSP_PARA_ANC_L_FILTER2, (uint8_t *)tmp_buffer, &buf_size);
                        //                    audio_anc_user_trigger_ff_receive_filter(NULL, (uint8_t *)tmp_buffer, buf_size);

                        audio_anc_user_trigger_ff_stop();
                        audio_anc_user_trigger_ff_recover_anc(ANC_K_STATUS_KEEP_DEFAULT);
                        race_send_event_notify_msg(RACE_EVENT_TYPE_ANC_ADAPTIVE_FF_END, NULL);
                    }

                    /*user trigger FF was terminated*/
                } else {
                    pEvt->status = ANC_K_STATUS_ABORT;
                    pEvt->mode = ADAPTIVE_FF_ANC_MODE;
                }
            }
            break;
#else
            case ADAPTIVE_FF_ANC_MODE: {

                pEvt->mode = ADAPTIVE_FF_ANC_MODE;

                if (pCmd->data0_id == USER_TRIGGER_ADAPTIVE_FF_DATA_MOBILE_STAT) {
                    if (pCmd->status == 0) {


                        if (pCmd->mobile_status == 0) {//change coef
                            if (pCmd->data1_len >= 188) {
                                typedef struct {
                                    uint8_t rw_filter;
                                    uint8_t *filter;
                                    S32 UU_alpha_par;
                                } user_trigger_adaptive_ff_rw_anc_filter;
                                user_trigger_adaptive_ff_rw_anc_filter rw_filter_cmd;
                                rw_filter_cmd.rw_filter = USER_TRIGGER_FF_WRITE_ANC_COEF;
                                rw_filter_cmd.filter = (uint8_t *)&pCmd->filter;
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
                                rw_filter_cmd.UU_alpha_par = pCmd->UU_alpha_para;
#else
                                rw_filter_cmd.UU_alpha_par = -1;
#endif
                                int res = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_UTFF, 0, &rw_filter_cmd);
                                if (res > 0) {
                                    pEvt->status = (uint8_t)(res & 0xF);
                                    pEvt->filter_selection = (uint8_t)((res & 0xF0) >> 4);
                                } else {
                                    pEvt->status = 0xFF;
                                    pEvt->filter_selection = 0;
                                }

                                //                            rw_filter_cmd.rw_filter = USER_TRIGGER_FF_READ_ANC_COEF;
                                //                            rw_filter_cmd.filter = NULL;
                                //                            audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_UTFF, 0, &rw_filter_cmd);

                            } else {
                                pEvt->status = 0xFF;
                            }
                        } else if (pCmd->mobile_status == 1) {
                            pEvt->status = 0;
                        }
                    }
                } else if (pCmd->data0_id == USER_TRIGGER_ADAPTIVE_FF_DATA_FACTORY_SETTING) {
                    typedef struct {
                        uint8_t rw_filter;
                        uint8_t *filter;
                        S32 UU_alpha_par;
                    } user_trigger_adaptive_ff_rw_anc_filter;
                    user_trigger_adaptive_ff_rw_anc_filter rw_filter_cmd;
                    rw_filter_cmd.rw_filter = USER_TRIGGER_FF_FACTORY_SETTING;
                    rw_filter_cmd.filter = NULL;
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
                    rw_filter_cmd.UU_alpha_par = 0;
#else
                    rw_filter_cmd.UU_alpha_par = -1;
#endif
                    int res = audio_anc_control_command_handler(AUDIO_ANC_CONTROL_SOURCE_FROM_UTFF, 0, &rw_filter_cmd);
                    pEvt->status = (uint8_t)(res & 0xF);
                    pEvt->filter_selection = (uint8_t)((res & 0xF0) >> 4);

                } else {
                    pEvt->status = 0xFF;
                }
                RACE_LOG_MSGID_I("[user_trigger_ff]RACE_DSPREALTIME_ANC_ADAPTIVE_RESULT_HDR, status:%d, data0_id:0x%x, data0_len=%d, data1_id:0x%x, data1_len=%d, mobile_status=%d, status=%d, filter_selection=%d", 8,
                                        pCmd->status, pCmd->data0_id, pCmd->data0_len, pCmd->data1_id, pCmd->data1_len,
                                        pCmd->mobile_status, pEvt->status, pEvt->filter_selection);
                break;
            }

#endif
#endif
            default:
                break;
        }
    }

    return pEvt;
}


typedef enum {
    RACE_ANC_NOTIFY_GAIN_TYPE_USER_UNAWARE   = 0,
    RACE_ANC_NOTIFY_GAIN_TYPE_HOWLING_CONTROL,
    RACE_ANC_NOTIFY_GAIN_TYPE_WIND_NOISE,
    RACE_ANC_NOTIFY_GAIN_TYPE_ENVIRONMENT_DETECTION
} anc_notfiy_gain_type_t;

void *RACE_DSPREALTIME_ANC_GAIN_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        uint8_t type;
    } PACKED RACE_DSPREALTIME_ANC_GAIN_CMD_EVT_STRU;

    g_anc_race_ch_id = channel_id;

    RACE_DSPREALTIME_ANC_GAIN_CMD_EVT_STRU *pCmd = (RACE_DSPREALTIME_ANC_GAIN_CMD_EVT_STRU *)pCmdMsg->payload;
    RACE_LOG_MSGID_I("[anc_gain] recv ANC_GAIN RACE CMD, type=%d", 1, pCmd->type);
    switch (pCmd->type) {
        case RACE_ANC_NOTIFY_GAIN_TYPE_USER_UNAWARE: {
            race_send_event_notify_msg(RACE_EVENT_TYPE_ANC_GAIN_USER_UNAWARE, NULL);
            break;
        }
        case RACE_ANC_NOTIFY_GAIN_TYPE_WIND_NOISE: {
            race_send_event_notify_msg(RACE_EVENT_TYPE_ANC_GAIN_WIND_NOISE, NULL);
            break;
        }
        case RACE_ANC_NOTIFY_GAIN_TYPE_ENVIRONMENT_DETECTION: {
            race_send_event_notify_msg(RACE_EVENT_TYPE_ANC_GAIN_ENVIRONMENT_DETECTION, NULL);
            break;
        }
        default:
            break;
    }

    return NULL;
}

void *RACE_DSPREALTIME_ANC_GAIN_CONTROL_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        uint8_t type;
        uint8_t action;
    } PACKED RACE_DSPREALTIME_ANC_GAIN_CONTROL_CMD_EVT_STRU;

    g_anc_race_ch_id = channel_id;

    RACE_DSPREALTIME_ANC_GAIN_CONTROL_CMD_EVT_STRU *pCmd = (RACE_DSPREALTIME_ANC_GAIN_CONTROL_CMD_EVT_STRU *)pCmdMsg->payload;
    RACE_LOG_MSGID_I("[anc_gain] recv ANC_GAIN_CONTROL RACE CMD, type=%d action=%d",
                     2, pCmd->type, pCmd->action);

    anc_gain_control_param_t *param = (anc_gain_control_param_t *)pvPortMalloc(sizeof(anc_gain_control_param_t));
    if (param != NULL) {
        param->type = pCmd->type;
        param->action = pCmd->action;
        race_send_event_notify_msg(RACE_EVENT_TYPE_ANC_GAIN_CONTROL, (void *)param);
    }
    return NULL;
}

void *RACE_DSPREALTIME_ANC_ADAPTIVE_EXTEND_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint8_t mode;
        uint16_t data_length;
        uint8_t data[1];
    } PACKED RACE_DSPREALTIME_ANC_ADAPTIVE_EXTEND_CMD_STRU;

    typedef struct {
        uint8_t status;
        uint8_t mode;
        uint16_t data_length;
        uint8_t data[1];
    } PACKED RACE_DSPREALTIME_ANC_ADAPTIVE_EXTEND_EVT_STRU;

    RACE_DSPREALTIME_ANC_ADAPTIVE_EXTEND_CMD_STRU *pCmd = (RACE_DSPREALTIME_ANC_ADAPTIVE_EXTEND_CMD_STRU *)pCmdMsg;
    RACE_DSPREALTIME_ANC_ADAPTIVE_EXTEND_EVT_STRU *pEvt = NULL;

    g_anc_race_ch_id = channel_id;
    switch (pCmd->mode) {
#ifdef MTK_USER_TRIGGER_ADAPTIVE_FF_V2
        case ADAPTIVE_FF_ANC_MODE: {
            race_dsprealtime_anc_adaptive_extend_t *pEvt2 = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                                                             RACE_DSPREALTIME_ANC_ADAPTIVE_EXTEND,
                                                                             sizeof(race_dsprealtime_anc_adaptive_extend_t),
                                                                             channel_id);
            RACE_FreePacket(pEvt2);
            audio_user_trigger_adaptive_ff_extend_cmd_hdr(pCmd->data_length, pCmd->data, (anc_user_trigger_ff_extend_cmd_callback_t)user_trigger_adaptive_ff_racecmd_response_extend);
            break;
        }
#endif
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
        case USER_UNAWARE_MODE: {
            bt_status_t ret = BT_STATUS_FAIL;
            race_dsprealtime_anc_adaptive_extend_t *pCmd2 = (race_dsprealtime_anc_adaptive_extend_t *)pCmdMsg->payload;
            if (pCmd2->param.UUCmd.sub_mode == AUDIO_USER_UNAWARE_RACE_EXTEND_SUBMODE_GET_EN_STAT) {
                RACE_RSP_ANC_ADAPTIVE_EXTEND_USER_UNAWARE_GET_EN_STAT_T *pEvt2 = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                                                                                  RACE_DSPREALTIME_ANC_ADAPTIVE_EXTEND,
                                                                                                  sizeof(RACE_RSP_ANC_ADAPTIVE_EXTEND_USER_UNAWARE_GET_EN_STAT_T),
                                                                                                  channel_id);
                if(pEvt2 == NULL){
                    RACE_LOG_MSGID_E("[relay_cmd][User Unaware]Claim Packet FAIL \n", 0);
                    break;
                }
                pEvt2->hdr.status = 0;
                pEvt2->hdr.mode = pCmd2->param.UUCmd.hdr.mode;
                pEvt2->hdr.data_len = sizeof(RACE_RSP_ANC_ADAPTIVE_EXTEND_USER_UNAWARE_GET_EN_STAT_T) - sizeof(RACE_RSP_ANC_ADAPTIVE_EXTEND_HDR_T);
                pEvt2->sub_mode = pCmd2->param.UUCmd.sub_mode;
                pEvt2->real_data_length = 1;
                pEvt2->enable = audio_user_unaware_get_enable_state();

                RACE_RSP_ANC_ADAPTIVE_EXTEND_USER_UNAWARE_GET_EN_STAT_T *pNoti2 = RACE_ClaimPacket(RACE_TYPE_NOTIFICATION,
                                                                                                   RACE_DSPREALTIME_ANC_ADAPTIVE_EXTEND,
                                                                                                   sizeof(RACE_RSP_ANC_ADAPTIVE_EXTEND_USER_UNAWARE_GET_EN_STAT_T),
                                                                                                   channel_id);
                memcpy((uint8_t *) & (pNoti2->hdr.status), (uint8_t *) & (pEvt2->hdr.status), sizeof(RACE_RSP_ANC_ADAPTIVE_EXTEND_USER_UNAWARE_GET_EN_STAT_T));

                bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
                if ((role == BT_AWS_MCE_ROLE_AGENT) || (role == BT_AWS_MCE_ROLE_NONE)) {
                    race_flush_packet((void *)pEvt2, channel_id);
                    race_flush_packet((void *)pNoti2, channel_id);
                } else {
                    race_send_pkt_t *pSndPkt;
                    //send 5B
                    pSndPkt = race_pointer_cnv_pkt_to_send_pkt((void *)pEvt2);
                    race_pkt_t      *pret;
                    race_send_pkt_t *psend;
                    psend = (race_send_pkt_t *)pSndPkt;
                    pret = &psend->race_data;
#if (RACE_DEBUG_PRINT_ENABLE)
                    race_dump((uint8_t *)pret, RACE_DBG_EVT);
#endif
                    if (pSndPkt) {
                        #ifdef RACE_RELAY_CMD_ENABLE
                        ret = bt_send_aws_mce_race_cmd_data(&pSndPkt->race_data, pSndPkt->length, channel_id, RACE_CMD_RSP_FROM_PARTNER, 0);
                        #endif
                        if (ret != BT_STATUS_SUCCESS) {
                            RACE_LOG_MSGID_E("[relay_cmd][User Unaware]partner send relay req FAIL \n", 0);
                        }
                        race_mem_free(pSndPkt);
                    }
                    //send 5D
                    pSndPkt = race_pointer_cnv_pkt_to_send_pkt((void *)pNoti2);
                    psend = (race_send_pkt_t *)pSndPkt;
                    pret = &psend->race_data;
#if (RACE_DEBUG_PRINT_ENABLE)
                    race_dump((uint8_t *)pret, RACE_DBG_EVT);
#endif
                    if (pSndPkt) {
                        #ifdef RACE_RELAY_CMD_ENABLE
                        ret = bt_send_aws_mce_race_cmd_data(&pSndPkt->race_data, pSndPkt->length, channel_id, RACE_CMD_RSP_FROM_PARTNER, 0);
                        #endif
                        if (ret != BT_STATUS_SUCCESS) {
                            RACE_LOG_MSGID_E("[relay_cmd][User Unaware]partner send relay req FAIL \n", 0);
                        }
                        race_mem_free(pSndPkt);
                    }

                }
            }
            break;
        }
#endif

        default:
            break;
    }

    return pEvt;
}
#endif


#ifdef AIR_DCHS_MODE_ENABLE

dsp_realtime_calibration_e race_dsprealtime_dchs_cosys_ctrl_cmd(ptr_race_pkt_t pRaceHeaderCmd, uint16_t length, uint8_t channel_id)
{
    dsp_realtime_status_e result;
    result = DSP_REALTIME_STATUS_SUCCESS;

    audio_dchs_relay_cmd_param_t dchs_relay_cmd_param;
    dchs_mode_t dchs_mode;
    dchs_mode = dchs_get_device_mode();

    if (!pRaceHeaderCmd) {
        assert(0);
    }



    switch (pRaceHeaderCmd->hdr.id) {
#ifdef AIR_COMPONENT_CALIBRATION_ENABLE
        case RACE_DSPREALTIME_CALIBRATION_GAIN : {
                RACE_DSPREALTIME_CALIBRATION_CMD_STRU *calibration_cmd = (RACE_DSPREALTIME_CALIBRATION_CMD_STRU *)pRaceHeaderCmd;
                if ((calibration_cmd->mode == DSP_REALTIME_CALIBRATION_MODE_UPDATE) ||
                    (calibration_cmd->mode == DSP_REALTIME_CALIBRATION_MODE_STORE)) {
                    result = DSP_REALTIME_STATUS_PROCESS_AND_RELAY_COMMAND;
                } else if (((dchs_mode == DCHS_MODE_RIGHT) && calibration_cmd->component_id&AUDIO_CALIBRATION_COMPONENT_DCHS_LEFT_CUP_MASK) ||
                           ((dchs_mode == DCHS_MODE_LEFT) && calibration_cmd->component_id&AUDIO_CALIBRATION_COMPONENT_DCHS_RIGHT_CUP_MASK)){
                    result = DSP_REALTIME_STATUS_RELAY_COMMAND;
                }
            }
            break;
#endif


        default:
            break;
    }


    if (result == DSP_REALTIME_STATUS_RELAY_COMMAND || result == DSP_REALTIME_STATUS_PROCESS_AND_RELAY_COMMAND) {
        length = length + sizeof(RACE_COMMON_HDR_STRU);// Length =  header + payload
        assert(sizeof(dchs_relay_cmd_param.payload) >= length);

        dchs_relay_cmd_param.header.ctrl_type = AUDIO_UART_COSYS_RACE_CMD;
        dchs_relay_cmd_param.header.param_size =  sizeof(audio_dchs_relay_cmd_param_t) - sizeof(audio_uart_cmd_header_t);
        memcpy(dchs_relay_cmd_param.payload, pRaceHeaderCmd, length);
        dchs_relay_cmd_param.channel_id = channel_id;
        dchs_relay_cmd_param.is_ack_cmd = false;
        dchs_relay_cmd_param.payload_length = length;
        mcu_uart_tx(UART_AUDIO_CMD, (U8 *)&dchs_relay_cmd_param, sizeof(audio_dchs_relay_cmd_param_t));
    }
    return result;
}
#endif

/**
 * RACE_DSPREALTIME_CALIBRATION_HDR
 *
 * RACE_DSPREALTIME_CALIBRATION_HDR Handler
 *
 * @pCmdMsg : pointer of ptr_race_pkt_t
 * @channel_id : channel ID
 */
void *RACE_DSPREALTIME_CALIBRATION_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
#ifdef AIR_COMPONENT_CALIBRATION_ENABLE

    RACE_DSPREALTIME_CALIBRATION_EVT_RES_STRU *pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE,
                                                                     (uint16_t)RACE_DSPREALTIME_CALIBRATION_GAIN,
                                                                     (uint16_t)sizeof(RACE_DSPREALTIME_CALIBRATION_EVT_RES_STRU),
                                                                     channel_id);
    if (pEvt) {
        RACE_DSPREALTIME_CALIBRATION_CMD_STRU *pCmd = (RACE_DSPREALTIME_CALIBRATION_CMD_STRU *)pCmdMsg;

        if (pCmd) {
            pEvt->status = (uint8_t)RACE_ERRCODE_SUCCESS;
            pEvt->component_id = pCmd->component_id;

            if (pCmd->mode == DSP_REALTIME_CALIBRATION_MODE_GET) {
                pEvt->gain = audio_calibration_get_component_gain_offset((audio_calibration_component_t) pCmd->component_id&(AUDIO_CALIBRATION_COMPONENT_MASK));
            } else if (pCmd->mode == DSP_REALTIME_CALIBRATION_MODE_SET) {
                pEvt->gain = pCmd->gain;
                audio_calibration_set_component_gain_offset((audio_calibration_component_t) pCmd->component_id&(AUDIO_CALIBRATION_COMPONENT_MASK), pEvt->gain);
            } else if (pCmd->mode == DSP_REALTIME_CALIBRATION_MODE_UPDATE) {
                audio_calibration_update_gain_offset_to_dsp();
            } else if (pCmd->mode == DSP_REALTIME_CALIBRATION_MODE_STORE) {
                audio_calibration_write_nvkey();
            } else {
                pEvt->status = (uint8_t)RACE_ERRCODE_PARAMETER_ERROR;
            }

        }
    }
    return pEvt;
#else
    UNUSED(channel_id);
    UNUSED(pCmdMsg);
    return NULL;
#endif

}


#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)||defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
typedef struct {
    uint8_t status;
    uint8_t ancId;
} PACKED RACE_ANC_PASSTHRU_COSYS_HEADER;

typedef union RACE_ANC_PASSTHRU_COSYS {
    RACE_ANC_PASSTHRU_COSYS_HEADER header;
} PACKED RACE_ANC_PASSTHRU_COSYS_UNION;

typedef struct {
    RACE_ANC_PASSTHRU_COSYS_UNION param;
} PACKED race_dsprealtime_anc_cosys_struct;

static uint8_t g_anc_race_cosys_ch_id;
uint8_t        g_mmi_anc_race_cosys_ch_id;
uint16_t       g_anc_race_cosys_cmd_length = 0;
uint8_t        g_anc_race_cosys_enable_state = 0;
extern void race_mmi_get_anc_status(uint8_t *anc_status, uint8_t *status);
extern void race_mmi_get_passthru_gain(uint8_t *gain, uint8_t *status);
void RACE_DSPREALTIME_COSYS_GET_PARAM(am_feature_type_t audio_feature, void *data)
{
    switch (audio_feature) {
        case AM_ANC: {
            uint8_t *p_get_status = data;
            race_dsprealtime_mutex_take();
            if (p_get_status != NULL) {
                *p_get_status = g_anc_race_cosys_enable_state;
            }
            race_dsprealtime_mutex_give();
        }
        break;
        default:
            RACE_LOG_MSGID_E("[RACE][COSYS] Audio GET_PARAM(%d) error.\n", 1, audio_feature);
            break;
    }
}
void RACE_DSPREALTIME_ANC_PASSTHRU_RELAY_COSYS_CALLBACK(bool is_critical, uint8_t *buff, uint32_t len)
{
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)
#ifdef MTK_ANC_ENABLE
#ifdef MTK_ANC_V2
    /* Master: Get relay co-sys race cmd and process ANC event. */
    race_pkt_t *pCmdMsg = (ptr_race_pkt_t)buff;
    ptr_race_send_pkt_t pEvt = NULL;
    if (pCmdMsg->hdr.id == RACE_DSPREALTIME_ANC) {
        pEvt = RACE_DSPREALTIME_ANC_PASSTHRU_HDR((ptr_race_pkt_t)buff, g_anc_race_cosys_ch_id);
    } else {
        RACE_MMI_GET_ENUM_CMD_STRU *pCmd = (RACE_MMI_GET_ENUM_CMD_STRU *)buff;
        RACE_MMI_GET_ENUM_EVT_STRU *pEvt_MMI = NULL;
        switch (pCmd->module) {
            case RACE_MMI_MODULE_ANC_STATUS: {
                pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)RACE_MMI_GET_ENUM, (uint16_t)sizeof(RACE_MMI_GET_ENUM_EVT_STRU) + sizeof(uint16_t), g_mmi_anc_race_cosys_ch_id);
                if (pEvt) {
                    pEvt_MMI = (RACE_MMI_GET_ENUM_EVT_STRU *)pEvt;
                    pEvt_MMI->module = RACE_MMI_MODULE_ANC_STATUS;
                    race_mmi_get_anc_status(&pEvt_MMI->data[0], &pEvt_MMI->status);
                }
                break;
            }
            case RACE_MMI_MODULE_PASSTHRU_GAIN: {
                pEvt = RACE_ClaimPacket((uint8_t)RACE_TYPE_RESPONSE, (uint16_t)RACE_MMI_GET_ENUM, (uint16_t)sizeof(RACE_MMI_GET_ENUM_EVT_STRU) + sizeof(uint8_t), g_mmi_anc_race_cosys_ch_id);
                if (pEvt) {
                    pEvt_MMI = (RACE_MMI_GET_ENUM_EVT_STRU *)pEvt;
                    pEvt_MMI->module = RACE_MMI_MODULE_PASSTHRU_GAIN;
                    race_mmi_get_passthru_gain(&pEvt_MMI->data[0], &pEvt_MMI->status);
                }
                break;
            }
            default:
                RACE_LOG_MSGID_E("[RACE][COSYS] Audio MMI module(%d) error.\n", 1, pCmd->module);
                break;
        }
    }
    /* Master: Send response to slave and ack to source. */
    if (pEvt) {
        pEvt = (void *)race_pointer_cnv_pkt_to_send_pkt(pEvt);

        race_cosys_send_data(RACE_COSYS_MODULE_ID_ANC_PASSTHROUGH, FALSE, (uint8_t *) & (pEvt->race_data), pEvt->race_data.hdr.length + 4);
        race_mem_free(pEvt); //Need free the space the ANC process malloc.
    } else {
        RACE_LOG_MSGID_E("[RACE][DSPREALTIME] ANC_PASSTHRU_RELAY_COSYS_CALLBACK malloc fail!", 0);
    }

#endif
#else
    RACE_LOG_MSGID_I("[RACE][COSYS] callback not enable ANC\n", 0);
#endif
#elif defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
    /* Slave: Get master response and ack to source. */
    ptr_race_send_pkt_t pEvt = NULL;
    uint8_t race_ch_id = 0;
    ptr_race_pkt_t response = (ptr_race_pkt_t)buff;
    RACE_LOG_MSGID_I("[RACE][COSYS] response (0x%x) (0x%x) (0x%x) (0x%x)\n", 4, response->hdr.pktId, response->hdr.type, response->hdr.length, response->hdr.id);
    if (response->hdr.id == RACE_DSPREALTIME_ANC) {
        if (response->hdr.type == RACE_TYPE_RESPONSE) {
            race_ch_id = g_anc_race_cosys_ch_id;
            pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                    RACE_DSPREALTIME_ANC,
                                    response->hdr.length - 2,
                                    race_ch_id);
        } else {
            RACE_DSPREALTIME_COSYS_EVT_STRU *cosys_anc_event = (RACE_DSPREALTIME_COSYS_EVT_STRU *)response->payload;
            race_dsprealtime_mutex_take();
            if (cosys_anc_event->event_id == 1 << 0) {
                g_anc_race_cosys_enable_state = 1; //anc_enable
            } else {
                g_anc_race_cosys_enable_state = 0; //anc_disable
            }
            race_dsprealtime_mutex_give();
            bt_sink_srv_am_feature_t feature_param;
            memset(&feature_param, 0, sizeof(bt_sink_srv_am_feature_t));
            feature_param.type_mask = AM_AUDIO_COSYS;
            feature_param.feature_param.cosys_param.sub_type    = AM_ANC;
            feature_param.feature_param.cosys_param.cosys_event = cosys_anc_event->event_id;
            am_audio_set_feature(FEATURE_NO_NEED_ID, &feature_param);
        }
    } else {
        race_ch_id = g_mmi_anc_race_cosys_ch_id;
        pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                RACE_MMI_GET_ENUM,
                                response->hdr.length - 2,
                                race_ch_id);
    }
    if (pEvt) {
        memcpy(pEvt, &(response->payload), response->hdr.length - 2);
        race_flush_packet((void *)pEvt, race_ch_id);
    }
#endif
    return;
}

/**
 * RACE_DSPREALTIME_ANC_COSYS_HDR
 *
 * RACE_DSPREALTIME_ANC_COSYS_HDR Handler
 *
 * @pCmdMsg : pointer of ptr_race_pkt_t
 *
 */
void *RACE_DSPREALTIME_ANC_PASSTHRU_COSYS_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    /* Slave: no define ANC enable, send relay co-sys race cmd to master. */
    bool ret = FALSE;
    g_anc_race_cosys_ch_id = channel_id;

    race_dsprealtime_anc_cosys_struct *pAnc_cmd;
    pAnc_cmd = (race_dsprealtime_anc_cosys_struct *)pCmdMsg->payload;
    g_anc_race_cosys_cmd_length = pCmdMsg->hdr.length - 4;
    ret = race_cosys_send_data(RACE_COSYS_MODULE_ID_ANC_PASSTHROUGH, FALSE, (uint8_t *)pCmdMsg, pCmdMsg->hdr.length + 4);
    RACE_LOG_MSGID_I("[RACE][ANC_PTHR][COSYS]send cosys data ret:%d, type(0x%x) msg_length:%d anc_id:%d\n", 4, ret, pCmdMsg->hdr.type, pCmdMsg->hdr.length, pAnc_cmd->param.header.ancId);
    return NULL;
}


void RACE_DSPREALTIME_callback(uint32_t event_id)
{
    ptr_race_send_pkt_t pEvt = NULL;
    RACE_DSPREALTIME_COSYS_EVT_STRU *pEvt_cosys = NULL;
    pEvt = RACE_ClaimPacket(RACE_TYPE_COMMAND,
                            RACE_DSPREALTIME_ANC,
                            sizeof(RACE_DSPREALTIME_COSYS_EVT_STRU),
                            0);
    if (pEvt) {
        pEvt_cosys = (RACE_DSPREALTIME_COSYS_EVT_STRU *)pEvt;
        pEvt_cosys->event_id = event_id;
        pEvt = (void *)race_pointer_cnv_pkt_to_send_pkt(pEvt);

        race_cosys_send_data(RACE_COSYS_MODULE_ID_ANC_PASSTHROUGH, FALSE, (uint8_t *) & (pEvt->race_data), pEvt->race_data.hdr.length + 4);
        race_mem_free(pEvt); //Need free the space the ANC process malloc.
    } else {
        RACE_LOG_MSGID_E("[RACE][DSPREALTIME] DSPREALTIME_callback malloc fail!", 0);
    }

}
#endif

void *race_cmd_dsprealtime_handler(ptr_race_pkt_t pRaceHeaderCmd, uint16_t length, uint8_t channel_id)
{
    void *ptr = NULL;

    RACE_LOG_MSGID_I("RACE_CmdHandler_DSPREALTIME, type[0x%X], race_id[0x%X], channel_id[%d]", 3,
                     pRaceHeaderCmd->hdr.type, pRaceHeaderCmd->hdr.id, channel_id);

    switch (pRaceHeaderCmd->hdr.id) {
        case RACE_DSPREALTIME_AUDIO_COMMON: {
            ptr = RACE_DSPREALTIME_AUDIO_COMMON_HDR(pRaceHeaderCmd, channel_id);
        }
        break;

        case RACE_DSPREALTIME_SUSPEND: {
            ptr = RACE_DSPREALTIME_SUSPEND_HDR(pRaceHeaderCmd, channel_id);
        }
        break;

        case RACE_DSPREALTIME_RESUME: {
            ptr = RACE_DSPREALTIME_RESUME_HDR(pRaceHeaderCmd, channel_id);
        }
        break;

#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
        case RACE_DSPREALTIME_PEQ: {
            ptr = RACE_DSPREALTIME_PEQ_HDR(pRaceHeaderCmd, channel_id);
        }
        break;
#endif

        case RACE_DSPSOUND_EFFECT: {
            // reserved.
        }
        break;

        case RACE_DSPREALTIME_GET_REFRENCE_GAIN: {
            ptr = RACE_DSPREALTIME_GET_REFRENCE_GAIN_HDR(pRaceHeaderCmd, channel_id);
        }
        break;
#ifdef AIR_AIRDUMP_ENABLE_MIC_RECORD
        case RACE_DSPREALTIME_AIRDUMP_ENTRY: {
            ptr = RACE_DSPREALTIME_AIRDUMP_COMMON_HDR(pRaceHeaderCmd, channel_id);
        }
        break;
#endif
#ifdef AIR_AIRDUMP_ENABLE
        case RACE_DSPREALTIME_AIRDUMP_ON_OFF: {
            ptr = RACE_DSPREALTIME_AIRDUMP_HDR(pRaceHeaderCmd, channel_id);
        }
        break;
#endif
        case RACE_DSPREALTIME_2MIC_SWAP: {
            ptr = RACE_DSPREALTIME_2MIC_SWAP_HDR(pRaceHeaderCmd, channel_id);
        }
        break;

        case RACE_DSPREALTIME_2MIC_SWAP_EXTEND: {
            ptr = RACE_DSPREALTIME_2MIC_SWAP_HDR_EXTEND(pRaceHeaderCmd, channel_id);
        }
        break;

        case RACE_DSPREALTIME_AECNR_ON_OFF: {
            ptr = RACE_DSPREALTIME_AECNR_ON_OFF_HDR(pRaceHeaderCmd, channel_id);
        }
        break;

		case RACE_DSPREALTIME_AECNR_AVC_ON_OFF: {
            ptr = RACE_DSPREALTIME_AECNR_AVC_ON_OFF_HDR(pRaceHeaderCmd, channel_id);
        }
        break;

        case RACE_DSPREALTIME_QUERY_LIB_VERSION: {
            ptr = RACE_DSPREALTIME_QUERY_LIB_VERSION_HDR(pRaceHeaderCmd, channel_id);
        }
        break;

        case RACE_DSPREALTIME_SWGAIN_BYPASS: {
            ptr = RACE_DSPREALTIME_SWGAIN_EN_HDR(pRaceHeaderCmd, channel_id);
        }
        break;

        case RACE_DSPREALTIME_OPEN_ALL_MIC: {
            ptr = RACE_DSPREALTIME_OPEN_ALL_MIC_HDR(pRaceHeaderCmd, channel_id);
        }
        break;

#if defined(HAL_AUDIO_MODULE_ENABLED)
        case RACE_DSPREALTIME_OPEN_ALL_MIC_EXTEND: {
            ptr = RACE_DSPREALTIME_OPEN_ALL_MIC_HDR_EXTEND(pRaceHeaderCmd, channel_id);
        }
        break;
#endif

#ifdef AIR_PSAP_ENABLE
        case RACE_DSPREALTIME_SET_PSAP_MODE:
            ptr = RACE_DSPREALTIME_SET_PSAP_MODE_HDR(pRaceHeaderCmd, channel_id);
            break;

        case RACE_DSPREALTIME_GET_PSAP_MODE:
            ptr = RACE_DSPREALTIME_GET_PSAP_MODE_HDR(pRaceHeaderCmd, channel_id);
            break;
#endif
#ifdef AIR_ADVANCED_PASSTHROUGH_ENABLE_V2
        case RACE_DSPREALTIME_SET_SELF_FITTING_CONFIG:
        case RACE_DSPREALTIME_GET_SELF_FITTING_CONFIG: {
            ptr = RACE_DSPREALTIME_SELF_FITTING_CONFIG_HDR(pRaceHeaderCmd, channel_id);
            break;
        }
#endif
#ifdef MTK_ANC_ENABLE
#ifdef MTK_ANC_V2
        case RACE_DSPREALTIME_ANC: {
            ptr = RACE_DSPREALTIME_ANC_PASSTHRU_HDR(pRaceHeaderCmd, channel_id);
        }
        break;
#if defined(AIR_ANC_V3) && defined(AIR_FULL_ADAPTIVE_ANC_ENABLE)
        case RACE_DSPREALTIME_FADP_ANC: {
            ptr = RACE_DSPREALTIME_FADP_ANC_HDR(pRaceHeaderCmd, channel_id);
        }
        break;
#endif
#else
        case RACE_DSPREALTIME_ANC: {
            ptr = RACE_DSPREALTIME_ANC_HDR(pRaceHeaderCmd, channel_id);
        }
        break;

        case RACE_DSPREALTIME_PASS_THROUGH_ON_OFF:
        case RACE_DSPREALTIME_PASS_THROUGH_TEST: {
            ptr = RACE_DSPREALTIME_PASSTHRU_HDR(pRaceHeaderCmd, channel_id);
        }
        break;

        case RACE_DSPREALTIME_PASS_THROUGH_TEST_MUTE: {
            ptr = RACE_DSPREALTIME_PASSTHRU_TEST_MUTE_HDR(pRaceHeaderCmd, channel_id);
        }
        break;

#endif
        case RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK: {
            ptr = RACE_DSPREALTIME_ANC_ADAPTIVE_CHECK_HDR(pRaceHeaderCmd, channel_id);
        }
        break;

        case RACE_DSPREALTIME_ANC_ADAPTIVE_RESULT: {
            ptr = RACE_DSPREALTIME_ANC_ADAPTIVE_RESULT_HDR(pRaceHeaderCmd, channel_id);
        }
        break;

        case RACE_DSPREALTIME_ANC_NOTIFY_GAIN: {
            ptr = RACE_DSPREALTIME_ANC_GAIN_HDR(pRaceHeaderCmd, channel_id);
        }
        break;

        case RACE_DSPREALTIME_ANC_ADAPTIVE_EXTEND: {
            ptr = RACE_DSPREALTIME_ANC_ADAPTIVE_EXTEND_HDR(pRaceHeaderCmd, channel_id);
        }
        break;

        case RACE_DSPREALTIME_ANC_GAIN_CONTROL: {
            ptr = RACE_DSPREALTIME_ANC_GAIN_CONTROL_HDR(pRaceHeaderCmd, channel_id);
        }
        break;
#else
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)||defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
        case RACE_DSPREALTIME_ANC: {
            ptr = RACE_DSPREALTIME_ANC_PASSTHRU_COSYS_HDR(pRaceHeaderCmd, channel_id);
        }
        break;
#endif
#endif
        case RACE_DSPREALTIME_AUDIO_DUMP_ON_OFF: {
            ptr = RACE_DSPREALTIME_AUDIO_DUMP_HDR(pRaceHeaderCmd, channel_id);
        }
        break;
#ifdef AIR_COMPONENT_CALIBRATION_ENABLE
        case RACE_DSPREALTIME_CALIBRATION_GAIN :
            ptr = RACE_DSPREALTIME_CALIBRATION_HDR(pRaceHeaderCmd, channel_id);
            break;
#endif
        default:
            break;
    }

    return ptr;
}


void *RACE_CmdHandler_DSPREALTIME(ptr_race_pkt_t pRaceHeaderCmd, uint16_t length, uint8_t channel_id)
{

#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)||defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
errrrrrrrrrrrrrr
    if (race_dsprealtime_relay_check(pRaceHeaderCmd)) {
        //Relay cmd
        race_dsprealtime_relay_handler(pRaceHeaderCmd, length, channel_id, true, (uint8_t)RACE_ERRCODE_SUCCESS);
        return NULL;
    } else {
        return race_cmd_dsprealtime_handler(pRaceHeaderCmd, length, channel_id);
    }
#elif defined(AIR_DCHS_MODE_ENABLE)
errrrrrrrrrrrrrrrrr
    void *ptr = NULL;
    dsp_realtime_status_e realtime_status;
    realtime_status = race_dsprealtime_dchs_cosys_ctrl_cmd(pRaceHeaderCmd, length, channel_id);

    if (realtime_status != DSP_REALTIME_STATUS_RELAY_COMMAND){
        ptr = race_cmd_dsprealtime_handler(pRaceHeaderCmd, length, channel_id);
    }

    //Drop the ack and wait the ack from the follower.
    if (realtime_status == DSP_REALTIME_STATUS_PROCESS_AND_RELAY_COMMAND) {
        //Free evt mem
        ptr = (void *)race_pointer_cnv_pkt_to_send_pkt(ptr);
        race_mem_free(ptr);
        ptr = NULL;
    }

    return ptr;
#else
//errrrrrrrrrrrrrrrrrrrr
    return race_cmd_dsprealtime_handler(pRaceHeaderCmd, length, channel_id);
#endif
}

#ifdef AIR_ADAPTIVE_EQ_ENABLE
typedef struct {
    uint8_t     status;
    uint8_t     type;
} PACKED RACE_DSPREALTIME_ADAPTIVE_EQ_CONTROL_RSP_EVT_STRU;

void race_dsprealtime_adaptive_eq_index_response(int16_t agent_index, int16_t partner_index, uint8_t channel_id)
{
    RACE_MMI_GET_AEQ_ENUM_EVT_STRU *event = RACE_ClaimPacket(RACE_TYPE_NOTIFICATION,
                                                         RACE_MMI_GET_ENUM,
                                                         sizeof(RACE_MMI_GET_AEQ_ENUM_EVT_STRU),
                                                         channel_id);
    if (event != NULL) {
        event->module  = RACE_MMI_MODULE_AEQ_INDEX;
        event->status  = 0;
        event->data[0] = agent_index;
        event->data[1] = partner_index;
        race_status_t race_status = race_flush_packet((void *)event, channel_id);
        RACE_LOG_MSGID_I("[ADAPTIVE_EQ] adaptive_eq_index_response, race_status=%d,agent_index:%d,partner_index:%d", 3, race_status,agent_index,partner_index);
    }
}

void race_dsprealtime_adaptive_eq_control_response(uint8_t status, uint8_t type, uint8_t channel_id)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    if (role != BT_AWS_MCE_ROLE_AGENT && role != BT_AWS_MCE_ROLE_NONE) {
        return;
    }

    RACE_DSPREALTIME_ADAPTIVE_EQ_CONTROL_RSP_EVT_STRU *event = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                                                             RACE_DSPREALTIME_PEQ,
                                                                             sizeof(RACE_DSPREALTIME_ADAPTIVE_EQ_CONTROL_RSP_EVT_STRU),
                                                                             channel_id);
    if (event != NULL) {
        event->status = status;
        event->type = type;
        race_flush_packet((void *)event, channel_id);
    }
}

void race_dsprealtime_adaptive_eq_detect_status_response(bool is_response, bool is_start, uint8_t status, uint8_t channel_id)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    if (role != BT_AWS_MCE_ROLE_AGENT && role != BT_AWS_MCE_ROLE_NONE) {
        return;
    }

    uint8_t race_type = is_response ? RACE_TYPE_RESPONSE : RACE_TYPE_NOTIFICATION;

    RACE_MMI_GET_ENUM_EVT_STRU *event = RACE_ClaimPacket(race_type,
                                                         RACE_MMI_GET_ENUM,
                                                         sizeof(RACE_MMI_GET_ENUM_EVT_STRU),
                                                         channel_id);
    if (event != NULL) {
        event->module  = RACE_MMI_MODULE_AEQ_DETECT_RUNTIME_STATUS;
        event->status  = 0;
        event->data[0] = status;
       race_flush_packet((void *)event, channel_id);
    }
}
#endif

#ifdef AIR_ADVANCED_PASSTHROUGH_ENABLE_V2
void race_dsprealtime_self_fitting_status_response(bool is_set_config, uint8_t status, uint16_t config_type, uint16_t data_lenth, uint8_t *data)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    RACE_LOG_MSGID_I("[SELF_FITTING] status_response, [%02X] is_set_config=%d, status=%d, config_type=%d, data_lenth=%d",
                     5, role, is_set_config, status, config_type, data_lenth);
    if (role != BT_AWS_MCE_ROLE_AGENT && role != BT_AWS_MCE_ROLE_NONE) {
        return;
    }

    typedef struct {
        uint8_t     status;
        uint16_t    config_type;
        uint8_t     data[0];
    } PACKED RACE_DSPREALTIME_SELF_FITTING_RSP_STRU;

    uint16_t race_id   = is_set_config ? RACE_DSPREALTIME_SET_SELF_FITTING_CONFIG : RACE_DSPREALTIME_GET_SELF_FITTING_CONFIG;

    RACE_DSPREALTIME_SELF_FITTING_RSP_STRU *event = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                                                     race_id,
                                                                     sizeof(RACE_DSPREALTIME_SELF_FITTING_RSP_STRU)+data_lenth,
                                                                     s_self_fitting_race_ch_id);
    if (event != NULL) {
        event->status      = status;
        event->config_type = config_type;
        if (data_lenth != 0 && data != NULL) {
            memcpy(event->data, data, data_lenth);
        }
        race_flush_packet((void *)event, s_self_fitting_race_ch_id);
    }
}
#endif

#ifdef MTK_ANC_ENABLE

typedef struct {
    uint8_t     status;
    uint8_t     type;
    int16_t     gain_1;
    int16_t     gain_2;
    uint8_t     level;
    int16_t     local_stationary_noise;
    int16_t     peer_stationary_noise;
} PACKED RACE_DSPREALTIME_ANC_REPLY_GAIN_EVT_STRU;

typedef struct {
    uint8_t     type;
    int16_t     gain_1;
    int16_t     gain_2;
    uint8_t     level;
    int16_t     local_stationary_noise;
    int16_t     peer_stationary_noise;
} PACKED RACE_DSPREALTIME_ANC_NOTIFY_GAIN_EVT_STRU;

typedef struct {
    uint8_t     status;
    uint8_t     type;
    bool        enable;
} PACKED RACE_DSPREALTIME_ANC_GAIN_CONTROL_RSP_EVT_STRU;

void race_dsprealtime_anc_notify_gain_error(uint8_t status, uint8_t event_id)
{
    uint8_t type = 0;
    if (event_id == RACE_EVENT_TYPE_ANC_GAIN_USER_UNAWARE) {
        type = RACE_ANC_NOTIFY_GAIN_TYPE_USER_UNAWARE;
    } else if (event_id == RACE_EVENT_TYPE_ANC_GAIN_WIND_NOISE) {
        type = RACE_ANC_NOTIFY_GAIN_TYPE_WIND_NOISE;
    } else if (event_id == RACE_EVENT_TYPE_ANC_GAIN_ENVIRONMENT_DETECTION) {
        type = RACE_ANC_NOTIFY_GAIN_TYPE_ENVIRONMENT_DETECTION;
    } else {
        //RACE_LOG_MSGID_E("[anc_gain] anc_notify_gain_error, invalid event_id=%d", 1, event_id);
        return;
    }

    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    RACE_LOG_MSGID_I("[anc_gain] anc_notify_gain_error, [%02X] status=%d type=%d", 3, role, status, type);
    // if (role != BT_AWS_MCE_ROLE_AGENT && role != BT_AWS_MCE_ROLE_NONE) {
    //     return;
    // }

    RACE_DSPREALTIME_ANC_REPLY_GAIN_EVT_STRU *event = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                                                       RACE_DSPREALTIME_ANC_NOTIFY_GAIN,
                                                                       sizeof(RACE_DSPREALTIME_ANC_REPLY_GAIN_EVT_STRU),
                                                                       g_anc_race_ch_id);
    if (event != NULL) {
        event->status = status;
        event->type = type;
        event->gain_1 = 0;
        event->gain_2 = 0;
        event->level = 0;
        event->local_stationary_noise = 0;
        event->peer_stationary_noise = 0;
        race_flush_packet((void *)event, g_anc_race_ch_id);
    }
}

void race_dsprealtime_anc_gain_control_response(uint8_t status, uint8_t type, bool enable)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    RACE_LOG_MSGID_I("[anc_gain] anc_gain_control_response, [%02X] status=%d type=%d enable=%d",
                     4, role, status, type, enable);

    RACE_DSPREALTIME_ANC_GAIN_CONTROL_RSP_EVT_STRU *event = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                                            RACE_DSPREALTIME_ANC_GAIN_CONTROL,
                                                            sizeof(RACE_DSPREALTIME_ANC_GAIN_CONTROL_RSP_EVT_STRU),
                                                            g_anc_race_ch_id);

    if (event != NULL) {
        event->status = status;
        event->type = type;
        event->enable = enable;
        race_flush_packet((void *)event, g_anc_race_ch_id);
    }
}

void race_dsprealtime_anc_gain_control_notify(uint8_t status, uint8_t type, bool enable)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    RACE_LOG_MSGID_I("[anc_gain] anc_gain_control_notify, [%02X] status=%d type=%d enable=%d",
                     4, role, status, type, enable);

    RACE_DSPREALTIME_ANC_GAIN_CONTROL_RSP_EVT_STRU *event = RACE_ClaimPacket(RACE_TYPE_NOTIFICATION,
                                                            RACE_DSPREALTIME_ANC_GAIN_CONTROL,
                                                            sizeof(RACE_DSPREALTIME_ANC_GAIN_CONTROL_RSP_EVT_STRU),
                                                            g_anc_race_ch_id);

    if (event != NULL) {
        event->status = status;
        event->type = type;
        event->enable = enable;
        race_flush_packet((void *)event, g_anc_race_ch_id);
    }
}

#if (defined AIR_ANC_USER_UNAWARE_ENABLE) || (defined AIR_ANC_WIND_DETECTION_ENABLE) || (defined AIR_ANC_ENVIRONMENT_DETECTION_ENABLE)
static void race_dsprealtime_anc_notify_gain_result(anc_notfiy_gain_type_t type,
                                                    bool is_response,
                                                    int16_t gain_1,
                                                    int16_t gain_2,
                                                    uint8_t level,
                                                    int16_t local_stationary_noise,
                                                    int16_t peer_stationary_noise)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    RACE_LOG_MSGID_I("[anc_gain] anc_notify_gain, [%02X] type=%d is_response=%d gain_1=%d gain_2=%d level=%d local_stationary_noise=%d peer_stationary_noise=%d",
                     8, role, type, is_response, gain_1, gain_2, level, local_stationary_noise, peer_stationary_noise);

    // if (role != BT_AWS_MCE_ROLE_AGENT && role != BT_AWS_MCE_ROLE_NONE) {
    //     return;
    // }

    if (is_response) {
        RACE_DSPREALTIME_ANC_REPLY_GAIN_EVT_STRU *event = RACE_ClaimPacket(RACE_TYPE_RESPONSE,
                                                                           RACE_DSPREALTIME_ANC_NOTIFY_GAIN,
                                                                           sizeof(RACE_DSPREALTIME_ANC_REPLY_GAIN_EVT_STRU),
                                                                           g_anc_race_ch_id);
        if (event != NULL) {
            event->status = RACE_ANC_GAIN_STATUS_SUCCESS;
            event->type = type;
            event->gain_1 = gain_1;
            event->gain_2 = gain_2;
            event->level = level;
            event->local_stationary_noise = local_stationary_noise;
            event->peer_stationary_noise = peer_stationary_noise;
            race_flush_packet((void *)event, g_anc_race_ch_id);
        }
    } else {
        RACE_DSPREALTIME_ANC_NOTIFY_GAIN_EVT_STRU *event = RACE_ClaimPacket(RACE_TYPE_NOTIFICATION,
                                                                            RACE_DSPREALTIME_ANC_NOTIFY_GAIN,
                                                                            sizeof(RACE_DSPREALTIME_ANC_NOTIFY_GAIN_EVT_STRU),
                                                                            g_anc_race_ch_id);
        if (event != NULL) {
            event->type = type;
            event->gain_1 = gain_1;
            event->gain_2 = gain_2;
            event->level = level;
            event->local_stationary_noise = local_stationary_noise;
            event->peer_stationary_noise = peer_stationary_noise;
            race_flush_packet((void *)event, g_anc_race_ch_id);
        }
    }

}
#endif

// User Unaware
#ifdef AIR_ANC_USER_UNAWARE_ENABLE
void race_dsprealtime_anc_user_unaware_response(int16_t gain)
{
    race_dsprealtime_anc_notify_gain_result(RACE_ANC_NOTIFY_GAIN_TYPE_USER_UNAWARE, TRUE, gain, 0, 0, 0, 0);
}

void race_dsprealtime_anc_user_unaware_notify(int16_t gain)
{
    race_dsprealtime_anc_notify_gain_result(RACE_ANC_NOTIFY_GAIN_TYPE_USER_UNAWARE, FALSE, gain, 0, 0, 0, 0);
}
#endif

#ifdef AIR_ANC_WIND_DETECTION_ENABLE
void race_dsprealtime_anc_wind_noise_response(int16_t gain)
{
    race_dsprealtime_anc_notify_gain_result(RACE_ANC_NOTIFY_GAIN_TYPE_WIND_NOISE, TRUE, gain, 0, 0, 0, 0);
}

void race_dsprealtime_anc_wind_noise_notify(int16_t gain)
{
    race_dsprealtime_anc_notify_gain_result(RACE_ANC_NOTIFY_GAIN_TYPE_WIND_NOISE, FALSE, gain, 0, 0, 0, 0);
}
#endif

// Environment Detection
#ifdef AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
void race_dsprealtime_anc_environment_detection_response(uint8_t level, int16_t ff_gain, int16_t fb_gain,
                                                         int16_t local_stationary_noise, int16_t peer_stationary_noise)
{
    race_dsprealtime_anc_notify_gain_result(RACE_ANC_NOTIFY_GAIN_TYPE_ENVIRONMENT_DETECTION, TRUE,
                                            ff_gain, fb_gain, level,
                                            local_stationary_noise, peer_stationary_noise);
}

void race_dsprealtime_anc_environment_detection_notify(uint8_t level, int16_t ff_gain, int16_t fb_gain,
                                                       int16_t local_stationary_noise, int16_t peer_stationary_noise)
{
    race_dsprealtime_anc_notify_gain_result(RACE_ANC_NOTIFY_GAIN_TYPE_ENVIRONMENT_DETECTION,
                                            FALSE, ff_gain, fb_gain, level,
                                            local_stationary_noise, peer_stationary_noise);
}
#endif

#endif /* MTK_ANC_ENABLE */

#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)||defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
static const uint16_t g_race_dsprealtime_relay_id_lists[] = RACE_DSPREALTIME_RELAY_ID_LIST;
#if 0
static const uint8_t g_race_dsprealtime_relay_0xE06_subid_lists[] = RACE_DSPREALTIME_RELAY_0x0E06_SUBID_LIST;
#endif
bool race_dsprealtime_relay_check(ptr_race_pkt_t pRaceHeaderCmd)
{
    uint32_t i, loop_count;
    bool found_in_list = false;
#if 0
    if (pRaceHeaderCmd->hdr.id == RACE_DSPREALTIME_ANC) {
        race_dsprealtime_anc_struct *pAnc_cmd;
        pAnc_cmd = (race_dsprealtime_anc_struct *)pRaceHeaderCmd->payload;
        loop_count = sizeof(g_race_dsprealtime_relay_0xE06_subid_lists) / sizeof(uint8_t);
        for (i = 0 ; i < loop_count ; i++) {
            if (pAnc_cmd->param.header.ancId == g_race_dsprealtime_relay_0xE06_subid_lists[i]) {
                found_in_list = true;
                break;
            }
        }
#else
    if (0) {
#endif
    } else {

        loop_count = sizeof(g_race_dsprealtime_relay_id_lists) / sizeof(uint16_t);
        for (i = 0 ; i < loop_count ; i++) {
            if (pRaceHeaderCmd->hdr.id == g_race_dsprealtime_relay_id_lists[i]) {
                found_in_list = true;
                break;
            }
        }
    }
    return found_in_list;
}

uint8_t race_dsprealtime_relay_check_evt_result(void *p_evt)
{
    typedef struct {
        uint8_t Status;
    } PACKED RACE_DSPREALTIME_EVT_STRU;
    RACE_DSPREALTIME_EVT_STRU *evt_ptr = p_evt;
    if (evt_ptr->Status == (uint8_t)RACE_ERRCODE_SUCCESS) {
        return RACE_ERRCODE_SUCCESS;
    } else {
        return RACE_ERRCODE_FAIL;
    }
}

bool race_dsprealtime_relay_handler(ptr_race_pkt_t pRaceHeaderCmd, uint16_t cmd_length, uint8_t channel_id, uint8_t send_to_follower, uint8_t cosys_result)
{
    race_dsprealtime_relay_packet_ptr_t relay_ptr;
    uint32_t relay_length = sizeof(race_dsprealtime_relay_packet_t) + cmd_length;

    relay_ptr = (race_dsprealtime_relay_packet_ptr_t)race_mem_alloc(relay_length);

    if (relay_ptr) {
        relay_ptr->source_channel_id = channel_id;
        relay_ptr->send_to_follower = send_to_follower;
        relay_ptr->follower_result = cosys_result;
        memcpy(&relay_ptr->race_cmd_pkt, pRaceHeaderCmd, sizeof(pRaceHeaderCmd) + cmd_length);
        if (!race_cosys_send_data(RACE_COSYS_MODULE_ID_DSP_COMMON, FALSE, (uint8_t *)relay_ptr, relay_length)) {
            assert(0);
        }
        RACE_LOG_MSGID_I("[RACE][DSPREALTIME][COSYS] Send cosys cmd ID:%d, relay_length:%d \n", 2, pRaceHeaderCmd->hdr.id, relay_length);
        race_mem_free(relay_ptr);
    } else {
        assert(0);
    }
    return true;
}

void race_dsprealtime_cosys_relay_callback(bool is_critical, uint8_t *buff, uint32_t len)
{
    uint8_t ret_check = 0;
    void *ptr;
    race_dsprealtime_relay_packet_ptr_t relay_ptr = (race_dsprealtime_relay_packet_ptr_t)buff;
    ptr_race_pkt_t cmd_ptr = &relay_ptr->race_cmd_pkt;

    if (!relay_ptr->send_to_follower) {
        /* Receive get response from  Follower*/
        if (relay_ptr->follower_result) {
            ret_check |= 1 << 0;
        }
        {
            //Process CMD
            ptr = race_cmd_dsprealtime_handler(cmd_ptr, cmd_ptr->hdr.length, relay_ptr->source_channel_id);
            if (ptr) {
                if (race_dsprealtime_relay_check_evt_result(ptr)) {
                    ret_check |= 1 << 1;
                }
                //Send result to cmd source
                race_send_pkt_t *pEvt;
                uint32_t send_length;
                pEvt = (void *)race_pointer_cnv_pkt_to_send_pkt(ptr);
                send_length = race_port_send_data(race_get_port_handle_by_channel_id(relay_ptr->source_channel_id), (uint8_t *)&pEvt->race_data, (uint32_t)pEvt->length);
                if (send_length != pEvt->length) {
                    ret_check |= 1 << 2;
                }
                race_mem_free(pEvt);
            } else {
                ret_check |= 1 << 3;
            }
        }
    } else {
        /* Follower receive the CMD */

        //Process CMD
        ptr = race_cmd_dsprealtime_handler(cmd_ptr, cmd_ptr->hdr.length, relay_ptr->source_channel_id);
        if (ptr) {
            relay_ptr->follower_result = race_dsprealtime_relay_check_evt_result(ptr);
            if (relay_ptr->follower_result) {
                ret_check |= 1 << 4;
            }
            //Reply to received CMD chip
            race_dsprealtime_relay_handler(cmd_ptr, cmd_ptr->hdr.length, relay_ptr->source_channel_id, false, relay_ptr->follower_result);

            //Free evt mem
            ptr = (void *)race_pointer_cnv_pkt_to_send_pkt(ptr);
            race_mem_free(ptr);
        } else {
            ret_check |= 1 << 5;
        }
    }
    RACE_LOG_MSGID_I("[RACE][DSPREALTIME][COSYS]callback id:%d, is_follower:%d error(0x%x)\n", 3, cmd_ptr->hdr.id, relay_ptr->send_to_follower, ret_check);
}
#endif

#endif /* RACE_DSP_REALTIME_CMD_ENABLE */


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

#ifndef UNUSED
#define UNUSED(x)  ((void)(x))
#endif

uint8_t g_ami_sidetone_use_PT_enable_state = 0;
#if defined(MTK_ANC_ENABLE) && defined(MTK_ANC_V2)
static int32_t g_ami_sidetone_use_PT_gain_HFP = 0;
static int32_t g_ami_sidetone_use_PT_gain_Reserve = 0;
#endif
extern void    *AUDIO_SRC_SRV_AM_TASK;
extern void *pxCurrentTCB;
extern HAL_DSP_PARA_AU_AFE_CTRL_t audio_nvdm_HW_config;
extern void audio_side_tone_enable_hdlr(bt_sink_srv_am_amm_struct *amm_ptr);
extern void audio_side_tone_disable_hdlr(bt_sink_srv_am_amm_struct *amm_ptr);
sidetone_scenario_t g_AM_sidetone_scenario = SIDETONE_SCENARIO_COMMON;
/*****************************************************************************
 * FUNCTION
 *  am_audio_side_tone_enable_by_scenario
 * DESCRIPTION
 *  Enable side tone by scenario
 * PARAMETERS
 *
 * RETURNS
 *  sidetone_control_result_t
 *****************************************************************************/
sidetone_control_result_t am_audio_side_tone_enable_by_scenario(sidetone_scenario_t scenario){
        audio_src_srv_report("[AMI] am_audio_side_tone_enable_by_scenario(%d)", 1, scenario);
        g_AM_sidetone_scenario = scenario;
        am_audio_side_tone_enable();
        return ST_EXECUTION_SUCCESS;
}
/*****************************************************************************
 * FUNCTION
 *  am_audio_side_tone_disable_by_scenario
 * DESCRIPTION
 *  Disable side tone by scenario
 * PARAMETERS
 *
 * RETURNS
 *  sidetone_control_result_t
 *****************************************************************************/
sidetone_control_result_t am_audio_side_tone_disable_by_scenario(sidetone_scenario_t scenario){
        audio_src_srv_report("[AMI] am_audio_side_tone_disable_by_scenario(%d)", 1, scenario);
        g_AM_sidetone_scenario = scenario;
        am_audio_side_tone_disable();
        return ST_EXECUTION_SUCCESS;
}

/*****************************************************************************
 * FUNCTION
 *  am_audio_side_tone_enable
 * DESCRIPTION
 *  Enable side tone
 * PARAMETERS
 *
 * RETURNS
 *  sidetone_control_result_t
 *****************************************************************************/
sidetone_control_t g_sidetone_control;

static sidetone_control_result_t am_audio_side_tone_get_nvdm_param(sidetone_nvdm_param_t *sidetone_param)
{
    sidetone_control_result_t ret = ST_EXECUTION_SUCCESS;
    uint32_t length = sizeof(sidetone_nvdm_param_t);
    sysram_status_t status;
    uint32_t tableSize = 0;
    if (sidetone_param != NULL) {
        status = flash_memory_query_nvdm_data_length(NVID_DSP_ALG_SIDETONE_PARA, &tableSize);
        if (status || !tableSize) {
            audio_src_srv_report("[AMI] am_audio_side_tone_get_nvdm_param fail, Status:%u Len:%lu\n", 2, status, tableSize);
            ret = ST_EXECUTION_FAIL;
        }
        if (tableSize != length) {
            audio_src_srv_report("[AMI] am_audio_side_tone_get_nvdm_param size not match. NVDM tableSize (%lu) != (%d)\n", 2, tableSize, length);
            ret = ST_EXECUTION_FAIL;
        }
        status = flash_memory_read_nvdm_data(NVID_DSP_ALG_SIDETONE_PARA, (uint8_t *)sidetone_param, &length);
    } else {
        ret = ST_EXECUTION_FAIL;
    }
    return ret;
}

sidetone_control_result_t am_audio_side_tone_init(void)
{
    sidetone_control_result_t  ret = ST_EXECUTION_SUCCESS;
    sidetone_nvdm_param_t pram_local;

    memset(&g_sidetone_control, 0, sizeof(sidetone_control_t));

    ret = am_audio_side_tone_get_nvdm_param(&pram_local);
    if (ret == ST_EXECUTION_SUCCESS) {
        g_sidetone_control.sidetone_method          = pram_local.sidetone_method;
        g_sidetone_control.sidetone_source_sel_en   = pram_local.sidetone_source_sel_en;
        g_sidetone_control.sideTone_source_sel_0    = pram_local.sideTone_source_sel_0;
        g_sidetone_control.sideTone_source_sel_1    = pram_local.sideTone_source_sel_1;
    }
    return ret;
}

sidetone_mechanism_t am_audio_side_tone_query_method(void)
{
    return g_sidetone_control.sidetone_method;
}

sidetone_control_result_t am_audio_side_tone_enable(void)
{
    bool is_am_task = false;
    uint32_t savedmask;
#ifndef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    if (audio_nvdm_HW_config.Voice_Sidetone_EN == false) {
#else
    if (audio_nvdm_HW_config.voice_scenario.Voice_Side_Tone_Enable == 0) {
#endif
        audio_src_srv_report("[AMI] am_audio_side_tone_enable fail, sidetone EN was false", 0);
        return ST_EXECUTION_FAIL;
    }
    hal_nvic_save_and_set_interrupt_mask(&savedmask);
    if (AUDIO_SRC_SRV_AM_TASK == pxCurrentTCB) {
        is_am_task = true;
    } else {
        is_am_task = false;
    }
    hal_nvic_restore_interrupt_mask(savedmask);
    if (is_am_task) {
        if ((ST_MECH_FIR_IND == am_audio_side_tone_query_method()) || (ST_MECH_IIR_IND == am_audio_side_tone_query_method())) {
            bt_sink_srv_am_amm_struct am_amm;
            am_amm.background_info.local_feature.feature_param.sidetone_param.scenario = g_AM_sidetone_scenario;
            audio_src_srv_report("[AMI] am_audio_side_tone_enable(%d)", 1, g_AM_sidetone_scenario);
            audio_side_tone_enable_hdlr(&am_amm);
            g_AM_sidetone_scenario = SIDETONE_SCENARIO_COMMON;
        } else if ((ST_MECH_FIR_EXTEND == am_audio_side_tone_query_method()) || (ST_MECH_IIR_EXTEND == am_audio_side_tone_query_method())) {
#if 0
            AUDIO_ASSERT(0);
#else
#if defined(MTK_ANC_ENABLE) && (defined(MTK_ANC_V2) || defined(AIR_ANC_V3))
            audio_anc_control_misc_t control_misc_sidetone;
            memset(&control_misc_sidetone, 0, sizeof(audio_anc_control_misc_t));
            if(bt_sink_srv_ami_get_current_scenario() == HFP) {
                control_misc_sidetone.sidetone_scenario_gain = g_ami_sidetone_use_PT_gain_HFP * 100;
            } else {
                control_misc_sidetone.sidetone_scenario_gain = g_ami_sidetone_use_PT_gain_Reserve * 100;
            }
            audio_anc_control_sidetone_enable(am_audio_side_tone_query_method(), &control_misc_sidetone);
#endif
#endif
        }
    } else {
        if ((ST_MECH_FIR_IND == am_audio_side_tone_query_method()) || (ST_MECH_IIR_IND == am_audio_side_tone_query_method())) {
            bt_sink_srv_am_background_t temp_background_t = {0};
            temp_background_t.local_feature.feature_param.sidetone_param.scenario = g_AM_sidetone_scenario;
            audio_src_srv_report("[AMI] am_audio_side_tone_enable2(%d)", 1, g_AM_sidetone_scenario);
            bt_sink_srv_ami_send_amm(MOD_AM, MOD_AMI, AUD_SELF_CMD_REQ,
                                     MSG_ID_AUDIO_SIDE_TONE_ENABLE,  &temp_background_t,
                                     FALSE, NULL);
            g_AM_sidetone_scenario = SIDETONE_SCENARIO_COMMON;
        } else if ((ST_MECH_FIR_EXTEND == am_audio_side_tone_query_method()) || (ST_MECH_IIR_EXTEND == am_audio_side_tone_query_method())) {
#if defined(MTK_ANC_ENABLE) && (defined(MTK_ANC_V2) || defined(AIR_ANC_V3))
            audio_anc_control_misc_t control_misc_sidetone;
            memset(&control_misc_sidetone, 0, sizeof(audio_anc_control_misc_t));
            if(bt_sink_srv_ami_get_current_scenario() == HFP) {
                control_misc_sidetone.sidetone_scenario_gain = g_ami_sidetone_use_PT_gain_HFP * 100;
            } else {
                control_misc_sidetone.sidetone_scenario_gain = g_ami_sidetone_use_PT_gain_Reserve * 100;
            }
            audio_anc_control_sidetone_enable(am_audio_side_tone_query_method(), &control_misc_sidetone);
#else
#if 0//defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
            ptr_race_pkt_t pLocalHeaderCmd = pvPortMalloc(sizeof(race_pkt_t) + 5);
            if (pLocalHeaderCmd != NULL) {
                //pLocalHeaderCmd->hdr.pktId  = 0x05;
                pLocalHeaderCmd->hdr.type   = 0x5A;
                pLocalHeaderCmd->hdr.length = 7; //5+2
                pLocalHeaderCmd->hdr.id     = 0xE06;
                pLocalHeaderCmd->payload[0] = 0x0;
                pLocalHeaderCmd->payload[1] = 10;
                pLocalHeaderCmd->payload[2] = 11; //flash_no
                pLocalHeaderCmd->payload[3] = 4; //ancType
                pLocalHeaderCmd->payload[4] = 1;
                void *ptr = RACE_DSPREALTIME_ANC_PASSTHRU_COSYS_HDR(pLocalHeaderCmd, 0);
                vPortFree(pLocalHeaderCmd);
            }
#else
            audio_src_srv_report("[AMI] am_audio_side_tone_enable fail, not support local device", 0);
#endif
#endif
        }
    }

    audio_src_srv_report("[AMI] am_audio_side_tone_enable method(%d)", 1, am_audio_side_tone_query_method());
    return ST_EXECUTION_SUCCESS;
}

/**
 * @brief The API is use to get the side tone gain value with db
 *
 * @param scenario
 * @return side tone gain with db
 */
int32_t g_side_tone_gain_common = SIDETONE_GAIN_MAGIC_NUM;
int32_t g_side_tone_gain_hfp = SIDETONE_GAIN_MAGIC_NUM;

int32_t am_audio_side_tone_get_volume_by_scenario(sidetone_scenario_t scenario)
{
    int32_t local_sidetone_volume = 0;
    if ((ST_MECH_FIR_IND == am_audio_side_tone_query_method()) || (ST_MECH_IIR_IND == am_audio_side_tone_query_method())) {
        if (scenario == SIDETONE_SCENARIO_HFP) {
            if (g_side_tone_gain_hfp == SIDETONE_GAIN_MAGIC_NUM) {
                if (audio_nvdm_HW_config.sidetone_config.SideTone_Gain >> 7) {
                    g_side_tone_gain_hfp   = 0xFFFFFF00 | audio_nvdm_HW_config.sidetone_config.SideTone_Gain;
                } else {
                    g_side_tone_gain_hfp   = audio_nvdm_HW_config.sidetone_config.SideTone_Gain;
                }
            }
            local_sidetone_volume = g_side_tone_gain_hfp;
        } else {
            if (g_side_tone_gain_common == SIDETONE_GAIN_MAGIC_NUM) {
                if (audio_nvdm_HW_config.sidetone_config.SideTone_Reserved[0] >> 7) {
                    g_side_tone_gain_common   = 0xFFFFFF00 | audio_nvdm_HW_config.sidetone_config.SideTone_Reserved[0];
                } else {
                    g_side_tone_gain_common   = audio_nvdm_HW_config.sidetone_config.SideTone_Reserved[0];
                }
            }
            local_sidetone_volume = g_side_tone_gain_common;
        }
    } else if ((ST_MECH_FIR_EXTEND == am_audio_side_tone_query_method()) || (ST_MECH_IIR_EXTEND == am_audio_side_tone_query_method())) {
#if defined(MTK_ANC_ENABLE) && (defined(MTK_ANC_V2) || defined(AIR_ANC_V3))
        if (scenario == SIDETONE_SCENARIO_RESERVE) {
            local_sidetone_volume = g_ami_sidetone_use_PT_gain_Reserve;
        } else {
            local_sidetone_volume = g_ami_sidetone_use_PT_gain_HFP;
        }
#else
        audio_src_srv_report("[AMI] am_audio_side_tone_get_volume_by_scenario fail, not support local device", 0);
#endif
    }
    return local_sidetone_volume;
}

extern void audio_side_tone_set_volume_hdlr(bt_sink_srv_am_amm_struct *amm_ptr);
/**
 * @brief The API is use to change the side tone gain value with db
 *
 * @param side_tone_gain
 * @param scenario
 * @return sidetone_control_result_t
 */

sidetone_control_result_t am_audio_side_tone_set_volume_by_scenario(sidetone_scenario_t scenario, int32_t side_tone_gain)
{
    bool is_am_task = false;
    uint32_t savedmask;
    bt_sink_srv_am_background_t temp_background_t = {0};
    bt_sink_srv_am_amm_struct am_amm;

    hal_nvic_save_and_set_interrupt_mask(&savedmask);
    if (AUDIO_SRC_SRV_AM_TASK == pxCurrentTCB) {
        is_am_task = true;
    } else {
        is_am_task = false;
    }
    hal_nvic_restore_interrupt_mask(savedmask);
    if (is_am_task) {
        if ((ST_MECH_FIR_IND == am_audio_side_tone_query_method()) || (ST_MECH_IIR_IND == am_audio_side_tone_query_method())) {
            am_amm.background_info.local_feature.feature_param.sidetone_param.scenario = scenario;
            am_amm.background_info.local_feature.feature_param.sidetone_param.side_tone_gain = side_tone_gain;
            audio_side_tone_set_volume_hdlr(&am_amm);
        } else if ((ST_MECH_FIR_EXTEND == am_audio_side_tone_query_method()) || (ST_MECH_IIR_EXTEND == am_audio_side_tone_query_method())) {
#if defined(MTK_ANC_ENABLE) && (defined(MTK_ANC_V2) || defined(AIR_ANC_V3))
            audio_anc_control_sidetone_set_volume((audio_anc_control_gain_t)(side_tone_gain * 100), NULL);
            if (scenario == SIDETONE_SCENARIO_RESERVE) {
                g_ami_sidetone_use_PT_gain_Reserve = side_tone_gain;
            } else {
                g_ami_sidetone_use_PT_gain_HFP     = side_tone_gain;
            }
#else
            assert(0);
#endif
        }
    } else {
        if ((ST_MECH_FIR_IND == am_audio_side_tone_query_method()) || (ST_MECH_IIR_IND == am_audio_side_tone_query_method())) {
            temp_background_t.local_feature.feature_param.sidetone_param.scenario = scenario;
            temp_background_t.local_feature.feature_param.sidetone_param.side_tone_gain = side_tone_gain;
            bt_sink_srv_ami_send_amm(MOD_AM, MOD_AMI, AUD_SELF_CMD_REQ,
                                     MSG_ID_AUDIO_SET_SIDE_TONE_VOLUME, &temp_background_t,
                                     FALSE, ptr_callback_amm);
        } else if ((ST_MECH_FIR_EXTEND == am_audio_side_tone_query_method()) || (ST_MECH_IIR_EXTEND == am_audio_side_tone_query_method())) {
#if defined(MTK_ANC_ENABLE) && (defined(MTK_ANC_V2) || defined(AIR_ANC_V3))
            audio_anc_control_sidetone_set_volume((audio_anc_control_gain_t)(side_tone_gain * 100), NULL);
            if (scenario == SIDETONE_SCENARIO_RESERVE) {
                g_ami_sidetone_use_PT_gain_Reserve = side_tone_gain;
            } else {
                g_ami_sidetone_use_PT_gain_HFP     = side_tone_gain;
            }
#else
            assert(0);
#endif
        }
    }

    audio_src_srv_report("[AMI] side tone set volume:sidetone_gain = %d, scenario = %d", 2, side_tone_gain, (int)scenario);
    return ST_EXECUTION_SUCCESS;
}

/*****************************************************************************
 * FUNCTION
 *  am_audio_side_tone_disable
 * DESCRIPTION
 *  Disable side tone
 * PARAMETERS
 *
 * RETURNS
 *  sidetone_control_result_t
 *****************************************************************************/
sidetone_control_result_t am_audio_side_tone_disable(void)
{
    if ((ST_MECH_FIR_IND == am_audio_side_tone_query_method()) || (ST_MECH_IIR_IND == am_audio_side_tone_query_method())) {
        bt_sink_srv_am_background_t temp_background_t = {0};
        temp_background_t.local_feature.feature_param.sidetone_param.scenario = g_AM_sidetone_scenario;
        audio_src_srv_report("[AMI] am_audio_side_tone_disable(%d)", 1, g_AM_sidetone_scenario);
        bt_sink_srv_ami_send_amm(MOD_AM, MOD_AMI, AUD_SELF_CMD_REQ,
                                 MSG_ID_AUDIO_SIDE_TONE_DISABLE, &temp_background_t,
                                 FALSE, NULL);
        g_AM_sidetone_scenario = SIDETONE_SCENARIO_COMMON;
    } else if ((ST_MECH_FIR_EXTEND == am_audio_side_tone_query_method()) || (ST_MECH_IIR_EXTEND == am_audio_side_tone_query_method())) {
#if defined(MTK_ANC_ENABLE) && (defined(MTK_ANC_V2) || defined(AIR_ANC_V3))
        audio_anc_control_sidetone_disable(NULL);
#else
#if 0//defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
        ptr_race_pkt_t pLocalHeaderCmd = pvPortMalloc(sizeof(race_pkt_t) + 6);
        if (pLocalHeaderCmd != NULL) {
            //pLocalHeaderCmd->hdr.pktId  = 0x05;
            pLocalHeaderCmd->hdr.type   = 0x5A;
            pLocalHeaderCmd->hdr.length = 8; //6+2
            pLocalHeaderCmd->hdr.id     = 0xE06;
            pLocalHeaderCmd->payload[0] = 0x0;
            pLocalHeaderCmd->payload[1] = 11;
            pLocalHeaderCmd->payload[2] = 1;
            void *ptr = RACE_DSPREALTIME_ANC_PASSTHRU_COSYS_HDR(pLocalHeaderCmd, 0);
            vPortFree(pLocalHeaderCmd);
        }
#else
        audio_src_srv_report("[AMI] am_audio_side_tone_disable fail, not support local device", 0);
#endif
#endif
    }

    audio_src_srv_report("[AMI] am_audio_side_tone_disable method(%d)", 1, am_audio_side_tone_query_method());
    return ST_EXECUTION_SUCCESS;
}

static ami_user_side_tone_volume_t s_user_side_tone_volume;

static sidetone_control_result_t ami_sidetone_enable_user_config(int32_t user_volume)
{
    //audio_src_srv_report("[test] enable user_sidetone_volume. user_volume: %d", 1, user_volume);
    s_user_side_tone_volume.enable = true;
    s_user_side_tone_volume.user_volume = user_volume;
    return ST_EXECUTION_SUCCESS;
}

static sidetone_control_result_t ami_sidetone_disable_user_config(void)
{
    //audio_src_srv_report("[test] disable user_sidetone_volume.", 0);
    s_user_side_tone_volume.enable = false;
    return ST_EXECUTION_SUCCESS;
}

sidetone_control_result_t ami_sidetone_enable_user_config_mode(int32_t user_volume)
{
    //audio_src_srv_report("[test] enable temporary_sidetone_user_volume. user_volume: %d", 1, user_volume);
    s_user_side_tone_volume.temporary_mode = true;
    return ami_sidetone_enable_user_config(user_volume);
}

sidetone_control_result_t ami_sidetone_disable_user_config_mode(void)
{
    if (s_user_side_tone_volume.temporary_mode) {
        ami_sidetone_disable_user_config();
        s_user_side_tone_volume.temporary_mode = false;
        //audio_src_srv_report("[test] disable temporary_sidetone_user_volume", 0);
        return ST_EXECUTION_SUCCESS;
    }
    //audio_src_srv_report("[test] temporary_sidetone_user_volume disabled yet", 0);
    return ST_EXECUTION_FAIL;
}

uint32_t ami_sidetone_adjust_gain_by_user_config(uint32_t sidetone_gain)
{
    if (s_user_side_tone_volume.enable) {
        sidetone_gain = s_user_side_tone_volume.user_volume * 100;
        audio_src_srv_report("[Sink][AMI] User_sidetone_volume is enabled. volume: %d", 1, s_user_side_tone_volume.user_volume);
        ami_sidetone_disable_user_config_mode();
    } else {
        //audio_src_srv_report("[test] User_sidetone_volume is disabled", 0);
    }
    return sidetone_gain;
}
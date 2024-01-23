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
#include "peq_setting.h"
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
#include "audio_transmitter_internal.h"
#endif
#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
uint8_t g_peq_init_flag = 0;
peq_nvdm_misc_param_t g_peq_handle;
#ifdef AIR_VP_PEQ_ENABLE
peq_nvdm_misc_param_t g_vp_peq_handle;
#endif
#endif
#ifdef AIR_ADAPTIVE_EQ_ENABLE
#define AEQ_GET_INDEX_INFO 0xF
#define AEQ_SET_DETECT_BYPASS_ENABLE 0xE
#define AEQ_SET_DETECT_BYPASS_DISABLE 0xD
uint8_t g_adaptive_eq_golden_index;
#endif
#ifdef AIR_PSAP_ENABLE
static uint8_t g_music_psap_feature_mode = 0;
static int32_t aud_set_psap_peq_param(bt_sink_srv_am_peq_param_t *ami_peq_param);
#endif

#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)||defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
#include "race_cmd_dsprealtime.h"
extern void RACE_DSPREALTIME_COSYS_GET_PARAM(am_feature_type_t audio_feature, void *data);
#endif

#ifndef _UNUSED
#define _UNUSED(x)  ((void)(x))
#endif

#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE) || defined(AIR_ADVANCED_PASSTHROUGH_ENABLE)
void aud_peq_init(void)
{
    uint32_t length = sizeof(peq_nvdm_misc_param_t);
    sysram_status_t status;
    memset(&g_peq_handle, 0, sizeof(peq_nvdm_misc_param_t));
    status = flash_memory_read_nvdm_data(NVKEY_DSP_PARA_PEQ_MISC_PARA, (uint8_t *)&g_peq_handle, &length);
    if ((status != NVDM_STATUS_NAT_OK) || (length != sizeof(peq_nvdm_misc_param_t))) {
        audio_src_srv_err("[Sink][AM]Read PEQ misc param error, error(%d), length:%d %d\n", 3, status, length, sizeof(peq_nvdm_misc_param_t));
    } else {
#ifdef AIR_ADAPTIVE_EQ_ENABLE
        g_adaptive_eq_golden_index = g_peq_handle.adaptive_peq_sound_mode;
#endif
        g_peq_init_flag = 1;
    }
#ifdef AIR_PSAP_ENABLE
    music_get_feature_mode();
#endif
#ifdef AIR_VP_PEQ_ENABLE
    memcpy(&g_vp_peq_handle, &g_peq_handle, sizeof(peq_nvdm_misc_param_t));
#endif

}

void aud_peq_save_misc_param(void)
{
    if (flash_memory_write_nvdm_data(NVKEY_DSP_PARA_PEQ_MISC_PARA, (uint8_t *)&g_peq_handle, sizeof(peq_nvdm_misc_param_t)) != NVDM_STATUS_NAT_OK) {
        audio_src_srv_err("[Sink][AM]Write PEQ misc param error\n", 0);
    }
}

static sysram_status_t aud_peq_put_nvkey(uint8_t *pDstBuf, uint32_t *buffer_size, uint16_t keyid, uint8_t *pSrcBuf)
{
    sysram_status_t status;
    uint16_t chksum = 0;
    nat_nvdm_info_t *p_nat_nvdm_info;
    uint8_t *pCoef;
    int32_t j;

    if (pDstBuf == NULL || buffer_size == NULL || keyid == 0) {
        return NVDM_STATUS_ERROR;
    }

    p_nat_nvdm_info = (nat_nvdm_info_t *)pDstBuf;
    p_nat_nvdm_info->offset = sizeof(nat_nvdm_info_t);
    *buffer_size -= sizeof(nat_nvdm_info_t);

    pCoef = pDstBuf + p_nat_nvdm_info->offset;
    if (pSrcBuf != NULL) {
        memcpy(pCoef, pSrcBuf, *buffer_size);
    } else {
        status = flash_memory_read_nvdm_data(keyid, pCoef, buffer_size);
        if (status != NVDM_STATUS_NAT_OK) {
            return status;
        }
    }

    for (j = chksum = 0 ; j < *buffer_size ; j++) {
        chksum += (uint16_t)(*pCoef++);
    }

    p_nat_nvdm_info->nvdm_id = keyid;
    p_nat_nvdm_info->length = *buffer_size;
    p_nat_nvdm_info->chksum = chksum;

    //audio_src_srv_report("[Sink][AM] aud_peq_put_nvkey [DstBuf]0x%x [SrcBuf]0x%x [keyid]0x%x [length]%d [chksum]%d [offset]%d\n", 6, pDstBuf, pSrcBuf, p_nat_nvdm_info->nvdm_id, p_nat_nvdm_info->length, p_nat_nvdm_info->chksum, p_nat_nvdm_info->offset);
    return NVDM_STATUS_NAT_OK;
}

int32_t aud_peq_realtime_update(mcu2dsp_peq_param_t *peq_param, bt_sink_srv_am_peq_param_t *ami_peq_param, peq_audio_path_id_t audio_path)
{
    sysram_status_t status = NVDM_STATUS_NAT_OK;
    uint32_t nvkey_buf_len = ((uint32_t)ami_peq_param->u2ParamSize) + sizeof(nat_nvdm_info_t);
    uint8_t *nvkey_buf = NULL;

    if (ami_peq_param->u2ParamSize == 0 || ami_peq_param->pu2Param == NULL) {
        audio_src_srv_report("[Sink][AM]Set realtime PEQ error, invalid param for realtime mode: 0x%x 0x%x\n", 2, ami_peq_param->u2ParamSize, ami_peq_param->pu2Param);
        audio_src_srv_report("[AM]invalid PEQ param for realtime mode", 0);
        AUDIO_ASSERT(0);
        return -1;
    }
#ifdef MTK_AWS_MCE_ENABLE
    bt_clock_t target_bt_clk = {0};
    U32 target_gpt_time = 0;
    bt_sink_srv_am_type_t cur_type = NONE;
    target_bt_clk.nclk = ami_peq_param->target_bt_clk;
    cur_type = bt_sink_srv_ami_get_current_scenario();
    if(cur_type == BLE){
        bt_sink_srv_convert_bt_clock_2_gpt_count(&target_bt_clk,&target_gpt_time);
        ami_peq_param->target_bt_clk = target_gpt_time;
        peq_param->gpt_time_sync = true;
        audio_src_srv_report("ble realtime peq target_gpt_time:%d",1,target_gpt_time);
    }
#endif
    nvkey_buf = (uint8_t *)pvPortMallocNC(nvkey_buf_len);
    if (nvkey_buf != NULL) {
        memset(nvkey_buf, 0, nvkey_buf_len);
        status = aud_peq_put_nvkey(nvkey_buf, &nvkey_buf_len, NVKEY_DSP_PARA_PEQ, (uint8_t *)ami_peq_param->pu2Param);
        if (status == NVDM_STATUS_NAT_OK) {
            void *p_param_share;
            peq_param->peq_nvkey_id = NVKEY_DSP_PARA_PEQ;
            peq_param->drc_enable = 1;
            peq_param->setting_mode = ami_peq_param->setting_mode;
            peq_param->target_bt_clk = ami_peq_param->target_bt_clk;
            peq_param->phase_id = ami_peq_param->phase_id;
            peq_param->nvkey_addr = (uint8_t *)((uint32_t)nvkey_buf);

            audio_src_srv_report("[Sink][AM]Set PEQ realtime update phase_id:%d nvkeyID:0x%x setting_mode:%d target_bt_clk:%d\n", 4, peq_param->phase_id, peq_param->peq_nvkey_id, peq_param->setting_mode, peq_param->target_bt_clk);
            p_param_share = hal_audio_dsp_controller_put_paramter(peq_param, sizeof(mcu2dsp_peq_param_t), AUDIO_MESSAGE_TYPE_COMMON);

            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_PEQ_SET_PARAM, audio_path, (uint32_t)p_param_share, true);
            LOG_W(MPLOG, "PEQ phase:%d enable:%d realtime", peq_param->phase_id, (peq_param->peq_nvkey_id == 0) ? 0 : 1);
        } else {
            audio_src_srv_err("[Sink][AM]Set PEQ param error, write to sysram error for realtime mode: %d\n", 1, status);
        }
    } else {
        audio_src_srv_err("[Sink][AM]Set realtime PEQ error, allocate NC memory fail, length:0x%x\n", 1, nvkey_buf_len);
        status = -1;
    }

    if (ami_peq_param->peq_notify_cb != NULL) {
        ami_peq_param->peq_notify_cb(ami_peq_param->pu2Param);
    }

    vPortFree(ami_peq_param->pu2Param);
    ami_peq_param->pu2Param = NULL;
    if (nvkey_buf != NULL) {
        vPortFreeNC(nvkey_buf);
    }

    return status;
}

peq_audio_path_id_t aud_get_peq_audio_path(bt_sink_srv_am_type_t type)
{
    peq_audio_path_id_t peq_audio_path = PEQ_AUDIO_PATH_A2DP;
    switch (type) {
        case A2DP:
        case AWS:
            peq_audio_path = PEQ_AUDIO_PATH_A2DP;
            break;
        default:
            break;
    }
    return peq_audio_path;
}

static uint32_t aud_peq_query_full_set_size(peq_audio_path_id_t audio_path, uint16_t *nvkey_id, uint32_t *nvkey_size)
{
    sysram_status_t status = NVDM_STATUS_NAT_OK;
    peq_audio_path_table_t *audio_path_tbl;
    uint32_t audio_path_tbl_size;
    uint32_t full_set_size;
    uint16_t keyid = 0;
    uint32_t i;
    uint32_t ret = -1;

#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_ADAPTIVE_EQ_ENABLE) || defined(AIR_WIRED_AUDIO_ENABLE)|| defined(AIR_WIRELESS_MIC_TX_ENABLE) || defined(AIR_HEARTHROUGH_VIVID_PT_ENABLE)
    if ((audio_path == PEQ_AUDIO_PATH_ADVANCED_PASSTHROUGH) ||(audio_path == PEQ_AUDIO_PATH_ADAPTIVE_EQ)
    ||(audio_path == PEQ_AUDIO_PATH_MIC)
    ||(audio_path == PEQ_AUDIO_PATH_ADVANCED_RECORD)
    ||(audio_path == PEQ_AUDIO_PATH_VIVID_PT)) {
        if(audio_path == PEQ_AUDIO_PATH_ADVANCED_PASSTHROUGH){
        keyid = NVKEY_DSP_PARA_ADVANCED_PASSTHROUGH_PRE_PEQ_TABLE;
        }else if(audio_path == PEQ_AUDIO_PATH_ADAPTIVE_EQ){
        keyid = NVKEY_DSP_PARA_AEQ_PATH;
        }else if(audio_path == PEQ_AUDIO_PATH_MIC){
            keyid = NVKEY_DSP_PARA_MIC_PEQ_PATH;
        }else if(audio_path == PEQ_AUDIO_PATH_ADVANCED_RECORD){
            keyid = NVKEY_DSP_PARA_ADVANCED_RECORD_PEQ_PATH;
        }else if(audio_path == PEQ_AUDIO_PATH_VIVID_PT){
            keyid = NVID_DSP_ALG_VIVID_PT_PEQ_PATH;
        }
        status = flash_memory_query_nvdm_data_length(keyid, &full_set_size);
        if (status == NVDM_STATUS_NAT_OK) {
            if (nvkey_id != NULL) {
                *nvkey_id = keyid;
            }
            if (nvkey_size != NULL) {
                *nvkey_size = full_set_size;
            }
            ret = 0;
        } else {
            audio_src_srv_err("[Sink][AM]Query PEQ full set size error, error:%d, keyid:0x%x\n", 2, status, keyid);
        }
        return ret;
    }
#endif

#ifdef AIR_VP_PEQ_ENABLE
    if ((audio_path == PEQ_AUDIO_PATH_VP) || (audio_path == PEQ_AUDIO_PATH_VP_AEQ)) {
        audio_path = PEQ_AUDIO_PATH_A2DP;
    }
#endif

    /*Get audio path table*/
    status = flash_memory_query_nvdm_data_length(NVKEY_DSP_PARA_PEQ, &audio_path_tbl_size);
    if (status != NVDM_STATUS_NAT_OK) {
        audio_src_srv_err("[Sink][AM]Query PEQ audio table size error, error:%d, keyid:0x%x\n", 2, status, NVKEY_DSP_PARA_PEQ);
        return -1;
    }
    audio_path_tbl = (peq_audio_path_table_t *)pvPortMalloc(audio_path_tbl_size);
    if (audio_path_tbl != NULL) {
        status = flash_memory_read_nvdm_data(NVKEY_DSP_PARA_PEQ, (uint8_t *)audio_path_tbl, &audio_path_tbl_size);
        if (status == NVDM_STATUS_NAT_OK) {
            /*Get peq full set nvkey and its size for audio_path*/
            for (i = 0; i < audio_path_tbl->numOfAudioPath; i++) {
                if (audio_path_tbl->audioPathList[i].audioPathID == audio_path) {
                    keyid = audio_path_tbl->audioPathList[i].nvkeyID;
                    break;
                }
            }
            if (keyid != 0) {
                status = flash_memory_query_nvdm_data_length(keyid, &full_set_size);
                if (status == NVDM_STATUS_NAT_OK) {
                    if (nvkey_id != NULL) {
                        *nvkey_id = keyid;
                    }
                    if (nvkey_size != NULL) {
                        *nvkey_size = full_set_size;
                    }
                    ret = 0;
                } else {
                    audio_src_srv_err("[Sink][AM]Query PEQ full set size error, error:%d, keyid:0x%x\n", 2, status, keyid);
                }
            } else {
                audio_src_srv_err("[Sink][AM]Parse PEQ nvkey error, can't find audio path:%d in keyid:0x%x\n", 2, audio_path, NVKEY_DSP_PARA_PEQ);
            }
        } else {
            audio_src_srv_err("[Sink][AM]Parse PEQ nvkey error, read nvdm from flash error:%d, keyid:0x%x\n", 2, status, NVKEY_DSP_PARA_PEQ);
        }
        vPortFree(audio_path_tbl);
    } else {
        audio_src_srv_err("[Sink][AM]Query PEQ full set size malloc fail, size:%d\n", 1, audio_path_tbl_size);
    }
    return ret;
}

static int32_t aud_peq_find_sound_mode(peq_audio_path_id_t audio_path, bt_sink_srv_am_peq_param_t *ami_peq_param)
{
    peq_full_set_t *full_set = NULL;
    peq_single_set_t *single_set;
    uint32_t i, j;
    uint16_t target_nvkey_id = ami_peq_param->u2ParamSize;
    uint16_t full_set_nvkey_id;
    uint32_t full_set_size;

    if (aud_peq_query_full_set_size(audio_path, &full_set_nvkey_id, &full_set_size) == 0) {
        full_set = (peq_full_set_t *)pvPortMalloc(full_set_size);
        if (full_set == NULL) {
            audio_src_srv_err("[Sink][AM]Find PEQ sound mode error, pvPortMalloc failed.\n", 0);
            return -1;
        } else {
            sysram_status_t status = flash_memory_read_nvdm_data(full_set_nvkey_id, (uint8_t *)full_set, &full_set_size);
            if (status != NVDM_STATUS_NAT_OK) {
                vPortFree(full_set);
                audio_src_srv_err("[Sink][AM]set PEQ param error, read nvdm from flash error:%d, keyid:0x%x\n", 2, status, full_set_nvkey_id);
                return -1;
            }
        }
    } else {
        return -1;
    }

    if (full_set != NULL) {
        single_set = (peq_single_set_t *)full_set->setList;
        for (i = 0; i < full_set->numOfSet; i++) {
            peq_single_phase_t *single_phase = single_set->phaseList;
            for (j = 0; j < single_set->numOfPhase; j++) {
                if ((single_phase->phaseID == ami_peq_param->phase_id) && (single_phase->nvkeyID == target_nvkey_id)) {
                    vPortFree(full_set);
                    ami_peq_param->sound_mode = (1 + i);
                    return 0;
                }
            }
            single_set = (peq_single_set_t *)((uint8_t *)single_set + (sizeof(peq_single_phase_t) * single_set->numOfPhase + 2));
        }
        vPortFree(full_set);
        audio_src_srv_err("[Sink][AM]Find PEQ sound mode error, can't find nvkey id:%d in audio path:%d\n", 2, target_nvkey_id, audio_path);
    }

    audio_src_srv_err("[Sink][AM]Find PEQ sound mode error, no such nvkey id: 0x%x\n", 1, target_nvkey_id);
    return -1;
}

int32_t aud_set_peq_param(peq_audio_path_id_t audio_path, bt_sink_srv_am_peq_param_t *ami_peq_param)
{
    sysram_status_t status = NVDM_STATUS_NAT_OK;
#ifdef AIR_PSAP_ENABLE
    if((ami_peq_param->phase_id == 0) && (audio_path == PEQ_AUDIO_PATH_A2DP)){
        if(g_music_psap_feature_mode != MUSIC_FEATURE_MODE_NORMAL){
            status = aud_set_psap_peq_param(ami_peq_param);
            return status;
        }
    }
#endif
    uint8_t sound_mode = ami_peq_param->sound_mode;
    mcu2dsp_peq_param_t peq_param;
    void *p_param_share;
    uint32_t nvkey_buf_len = 0;
    uint8_t *nvkey_buf = NULL;

    memset(&peq_param, 0, sizeof(mcu2dsp_peq_param_t));
    if((sound_mode == PEQ_DISABLE_ALL)||(sound_mode == PEQ_ON_ALL)){
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_PEQ_SET_PARAM, sound_mode, 0, true);
        return 0;
    }

    peq_param.drc_force_disable = 0;
    if (sound_mode == PEQ_SOUND_MODE_FORCE_DRC) {
        if (ami_peq_param->enable == 0) {
            peq_param.peq_nvkey_id = 0;
            peq_param.drc_enable = 0;
            peq_param.setting_mode = ami_peq_param->setting_mode;
            peq_param.target_bt_clk = ami_peq_param->target_bt_clk;
            peq_param.phase_id = ami_peq_param->phase_id;
            peq_param.drc_force_disable = 1;
        } else {
            sound_mode = PEQ_SOUND_MODE_UNASSIGNED;
        }
    }

    if (ami_peq_param->enable == 0) {
        peq_param.peq_nvkey_id = 0;
        peq_param.drc_enable = 0;
        peq_param.setting_mode = ami_peq_param->setting_mode;
        peq_param.target_bt_clk = ami_peq_param->target_bt_clk;
        peq_param.phase_id = ami_peq_param->phase_id;
    } else if (sound_mode == PEQ_SOUND_MODE_REAlTIME) {
        return aud_peq_realtime_update(&peq_param, ami_peq_param, audio_path);
    } else if (sound_mode == PEQ_SOUND_MODE_SAVE) {
        return aud_peq_find_sound_mode(audio_path, ami_peq_param);
    } else {
        peq_full_set_t *full_set = NULL;
        peq_single_set_t *single_set;
        uint16_t keyid = 0;
        uint16_t full_set_nvkey_id;
        uint32_t full_set_size;
        uint32_t i;

#ifdef MTK_DEQ_ENABLE
        if (ami_peq_param->phase_id == 2) {
            if (sound_mode == DEQ_AUDIO_SOUND_MODE) {
                keyid = DEQ_AUDIO_NVKEY;
            } else {
                audio_src_srv_err("[Sink][AM]Un-supported sound mode :%d for DEQ.\n", 1, sound_mode);
                status = -1;
                goto __EXIT;
            }
        } else
#endif
        {
            if (sound_mode == PEQ_SOUND_MODE_UNASSIGNED) {
                sound_mode = (ami_peq_param->phase_id == 0) ? g_peq_handle.a2dp_pre_peq_sound_mode : g_peq_handle.a2dp_post_peq_sound_mode;
                ami_peq_param->sound_mode = sound_mode;
            }

            if (aud_peq_query_full_set_size(audio_path, &full_set_nvkey_id, &full_set_size) == 0) {
                full_set = (peq_full_set_t *)pvPortMalloc(full_set_size);
                if (full_set == NULL) {
                    audio_src_srv_err("[Sink][AM]set PEQ param error, pvPortMalloc failed with size:%d\n", 1, full_set_size);
                    status = -1;
                    goto __EXIT;
                }
                status = flash_memory_read_nvdm_data(full_set_nvkey_id, (uint8_t *)full_set, &full_set_size);
                if (status != NVDM_STATUS_NAT_OK) {
                    audio_src_srv_err("[Sink][AM]set PEQ param error, read nvdm from flash error:%d, keyid:0x%x\n", 2, status, full_set_nvkey_id);
                    status = -1;
                    goto __EXIT;
                }
            } else {
                status = -1;
                goto __EXIT;
            }

            if (sound_mode > full_set->numOfSet) {
                audio_src_srv_err("[Sink][AM]Set PEQ param error, invalid sound mode:%d %d\n", 2, sound_mode, full_set->numOfSet);
                status = -1;
                goto __EXIT;
            }

            single_set = (peq_single_set_t *)full_set->setList;
            i = sound_mode - 1;
            while (i != 0) {
                single_set = (peq_single_set_t *)((uint8_t *)single_set + (sizeof(peq_single_phase_t) * single_set->numOfPhase + 2));
                i--;
            }

            keyid = 0;
            for (i = 0; i < single_set->numOfPhase; i++) {
                if (single_set->phaseList[i].phaseID == ami_peq_param->phase_id) {
                    keyid = single_set->phaseList[i].nvkeyID;
                    break;
                }
            }
            if (keyid == 0) {
                audio_src_srv_err("[Sink][AM]Set PEQ param error, can't find phase_id(%d) error, keyid:0x%x\n", 2, ami_peq_param->phase_id, keyid);
                status = -1;
                goto __EXIT;
            }
        }

        status = flash_memory_query_nvdm_data_length(keyid, &nvkey_buf_len);
        if (status != NVDM_STATUS_NAT_OK) {
            audio_src_srv_err("[Sink][AM]Set PEQ param error, query nvkey length error:%d, keyid:0x%x\n", 2, status, keyid);
            goto __EXIT;
        }
        nvkey_buf_len += sizeof(nat_nvdm_info_t);
        nvkey_buf = (uint8_t *)pvPortMallocNC(nvkey_buf_len);
        if (nvkey_buf != NULL) {
            memset(nvkey_buf, 0, nvkey_buf_len);
        } else {
            audio_src_srv_err("[Sink][AM]Set PEQ param error, allocate NC memory fail, keyid:0x%x, length:0x%x\n", 2, keyid, nvkey_buf_len);
            status = -1;
            goto __EXIT;
        }

        status = aud_peq_put_nvkey(nvkey_buf, &nvkey_buf_len, keyid, NULL);
        if (status != NVDM_STATUS_NAT_OK) {
            audio_src_srv_err("[Sink][AM]Set PEQ param error, write to sysram error:%d, keyid:0x%x\n", 2, status, keyid);
            goto __EXIT;
        }
#ifdef MTK_AWS_MCE_ENABLE
        bt_clock_t target_bt_clk = {0};
        U32 target_gpt_time = 0;
        bt_sink_srv_am_type_t cur_type = NONE;
        target_bt_clk.nclk = ami_peq_param->target_bt_clk;
        cur_type = bt_sink_srv_ami_get_current_scenario();
        if(cur_type == BLE){
            bt_sink_srv_convert_bt_clock_2_gpt_count(&target_bt_clk,&target_gpt_time);
            ami_peq_param->target_bt_clk = target_gpt_time;
            peq_param.gpt_time_sync = true;
            audio_src_srv_report("ble change peq target_gpt_time:%d",1,target_gpt_time);
        }
#endif
        peq_param.peq_nvkey_id = keyid;
        peq_param.drc_enable = 1;
        peq_param.setting_mode = ami_peq_param->setting_mode;
        peq_param.target_bt_clk = ami_peq_param->target_bt_clk;
        peq_param.phase_id = ami_peq_param->phase_id;
        peq_param.nvkey_addr = (uint8_t *)((uint32_t)nvkey_buf);

__EXIT:
        if (full_set != NULL) {
            vPortFree(full_set);
        }
        if (status != 0) {
            if (nvkey_buf != NULL) {
                vPortFreeNC(nvkey_buf);
            }
            return status;
        }
    }

    audio_src_srv_report("[Sink][AM]Set PEQ phase_id:%d enable:%d, sound mode:%d, nvkeyID:%x setting_mode:%d target_bt_clk:%d\n", 6, ami_peq_param->phase_id, ami_peq_param->enable, sound_mode, peq_param.peq_nvkey_id, ami_peq_param->setting_mode, ami_peq_param->target_bt_clk);
    p_param_share = hal_audio_dsp_controller_put_paramter(&peq_param, sizeof(mcu2dsp_peq_param_t), AUDIO_MESSAGE_TYPE_COMMON);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_PEQ_SET_PARAM, audio_path, (uint32_t)p_param_share, true);
    LOG_W(MPLOG, "PEQ phase:%d enable:%d nvkey:0x%x", peq_param.phase_id, (peq_param.peq_nvkey_id == 0) ? 0 : 1, peq_param.peq_nvkey_id);

    if (nvkey_buf != NULL) {
        vPortFreeNC(nvkey_buf);
    }
    return status;
}

#ifdef AIR_ADAPTIVE_EQ_ENABLE

void aud_aeq_save_misc()
{
    uint32_t length = sizeof(peq_nvdm_misc_param_t);
    peq_nvdm_misc_param_t payload;
    sysram_status_t status;
    memset(&payload, 0, sizeof(peq_nvdm_misc_param_t));
    status = flash_memory_read_nvdm_data(NVKEY_DSP_PARA_PEQ_MISC_PARA, (uint8_t *)&payload, &length);
    if ((status != NVDM_STATUS_NAT_OK) || (length != sizeof(peq_nvdm_misc_param_t))) {
        audio_src_srv_err("[Sink][AM]Read PEQ misc param error, error(%d), length:%d %d\n", 3, status, length, sizeof(peq_nvdm_misc_param_t));
    } else {
        payload.adaptive_peq_enable      = g_peq_handle.adaptive_peq_enable;
        payload.adaptive_peq_sound_mode  = g_peq_handle.adaptive_peq_sound_mode;
        if (flash_memory_write_nvdm_data(NVKEY_DSP_PARA_PEQ_MISC_PARA, (uint8_t *)&payload, sizeof(peq_nvdm_misc_param_t)) != NVDM_STATUS_NAT_OK) {
            audio_src_srv_err("[Sink][AM]Write PEQ misc param error\n", 0);
        }
    }
}

void aud_send_aeq_share_info()
{
    aeq_share_info_t *share_addr;
    share_addr = (aeq_share_info_t *)hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_AEQ_MONITOR_ADAPTIVE_EQ);
    memset(share_addr, 0, SHARE_BUFFER_AEQ_INDEX_INFO_SIZE);

    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_ADEQ_SET_PARAM, AEQ_GET_INDEX_INFO, (uint32_t)share_addr, true);
}

int32_t aud_set_adaptive_aeq_param()
{
    sysram_status_t status = NVDM_STATUS_NAT_OK;
    sysram_status_t nvk_status = NVDM_STATUS_NAT_OK;
    void *p_param_share;
    uint32_t nvkey_buf_len = 0;
    uint8_t *nvkey_buf = NULL;
    peq_full_set_t *full_set = NULL;
    peq_single_set_t *single_set;
    uint16_t keyid = 0;
    uint16_t full_set_nvkey_id;
    uint32_t full_set_size;
    uint32_t eq_num = 0;
    uint32_t i, j;
    uint8_t adaptive_eq_number = 0;

    aud_send_aeq_share_info();
    if (aud_peq_query_full_set_size(PEQ_AUDIO_PATH_ADAPTIVE_EQ, &full_set_nvkey_id, &full_set_size) == 0) {
        full_set = (peq_full_set_t *)pvPortMalloc(full_set_size);
        if (full_set == NULL) {
            audio_src_srv_err("[Sink][AM]set PEQ param error, pvPortMalloc failed with size:%d\n", 1, full_set_size);
            status = -1;
            return status;
        }
        status = flash_memory_read_nvdm_data(full_set_nvkey_id, (uint8_t *)full_set, &full_set_size);
        if (status != NVDM_STATUS_NAT_OK) {
            audio_src_srv_err("[Sink][AM]set PEQ param error, read nvdm from flash error:%d, keyid:0x%x\n", 2, status, full_set_nvkey_id);
            status = -1;
            if (full_set != NULL) {
                vPortFree(full_set);
            }
            return status;
        }
    } else {
        status = -1;
        return status;
    }
    adaptive_eq_number = full_set->numOfSet;
    mcu2dsp_peq_param_t *peq_param;
    peq_param = (mcu2dsp_peq_param_t *)pvPortMalloc((adaptive_eq_number) * sizeof(mcu2dsp_peq_param_t));
    memset(peq_param, 0, (adaptive_eq_number)*sizeof(mcu2dsp_peq_param_t));

    for (eq_num = 0; eq_num < adaptive_eq_number; eq_num++) {
        single_set = (peq_single_set_t *)full_set->setList;
        j = eq_num;
        while (j != 0) {
            single_set = (peq_single_set_t *)((uint8_t *)single_set + (sizeof(peq_single_phase_t) * single_set->numOfPhase + 2));
            j--;
        }

        keyid = 0;
        for (i = 0; i < single_set->numOfPhase; i++) {
            if (single_set->phaseList[i].phaseID == 0) {
                keyid = single_set->phaseList[i].nvkeyID;
                break;
            }
        }
        if (keyid == 0) {
            audio_src_srv_err("[Sink][AM]Set AEQ param error, can't find phase_id(0) error, keyid:0x%x\n", 1, keyid);
        }

        status = flash_memory_query_nvdm_data_length(keyid, &nvkey_buf_len);
        if (status != NVDM_STATUS_NAT_OK) {
            audio_src_srv_err("[Sink][AM]Set AEQ param error, query nvkey length error:%d, keyid:0x%x\n", 2, status, keyid);
        }
        nvkey_buf_len += sizeof(nat_nvdm_info_t);
        nvkey_buf = (uint8_t *)pvPortMallocNC(nvkey_buf_len);
        if (nvkey_buf != NULL) {
            memset(nvkey_buf, 0, nvkey_buf_len);
        } else {
            audio_src_srv_err("[Sink][AM]Set AEQ param error, allocate NC memory fail, keyid:0x%x, length:0x%x\n", 2, keyid, nvkey_buf_len);
            status = -1;
        }

        nvk_status = aud_peq_put_nvkey(nvkey_buf, &nvkey_buf_len, keyid, NULL);
        if (nvk_status != NVDM_STATUS_NAT_OK) {
            audio_src_srv_err("[Sink][AM]Set AEQ param error, write to sysram error:%d, keyid:0x%x\n", 2, status, keyid);
        }

        if ((status != 0) || (nvk_status != 0)) {
            for (j = 0; j <= eq_num; j++) {
                vPortFreeNC((void*)peq_param[j].nvkey_addr);
            }
            if (peq_param != NULL) {
                vPortFree(peq_param);
            }
            if (full_set != NULL) {
                vPortFree(full_set);
            }
            status = -1;
            return status;
        }

        peq_param[eq_num].peq_nvkey_id = keyid;
        peq_param[eq_num].drc_enable = 1;
        peq_param[eq_num].setting_mode = PEQ_DIRECT;
        peq_param[eq_num].target_bt_clk = 0;
        peq_param[eq_num].phase_id = 0;
        peq_param[eq_num].nvkey_addr = (uint8_t *)((uint32_t)nvkey_buf);
    }

    if (full_set != NULL) {
        vPortFree(full_set);
    }

    p_param_share = hal_audio_dsp_controller_put_paramter(peq_param, (adaptive_eq_number) * sizeof(mcu2dsp_peq_param_t), AUDIO_MESSAGE_TYPE_COMMON);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_ADEQ_SET_PARAM, adaptive_eq_number, (uint32_t)p_param_share, true);

    for (eq_num = 0; eq_num < adaptive_eq_number; eq_num++) {
        vPortFreeNC((void*)peq_param[eq_num].nvkey_addr);
    }

    if (peq_param != NULL) {
        vPortFree(peq_param);
    }

    return status;
}
#endif

int32_t aud_set_special_peq_param(uint8_t special_peq_audio_id, uint16_t keyid)
{
    sysram_status_t status = NVDM_STATUS_NAT_OK;
    mcu2dsp_peq_param_t peq_param;
    void *p_param_share;
    uint32_t nvkey_buf_len = 0;
    uint8_t *nvkey_buf = NULL;

    memset(&peq_param, 0, sizeof(mcu2dsp_peq_param_t));

    status = flash_memory_query_nvdm_data_length(keyid, &nvkey_buf_len);
    if (status != NVDM_STATUS_NAT_OK) {
        audio_src_srv_err("[Sink][AM]Set PEQ param error, query nvkey length error:%d, keyid:0x%x\n", 2, status, keyid);
        goto __EXIT;
    }
    nvkey_buf_len += sizeof(nat_nvdm_info_t);
    nvkey_buf = (uint8_t *)pvPortMallocNC(nvkey_buf_len);
    if (nvkey_buf != NULL) {
        memset(nvkey_buf, 0, nvkey_buf_len);
    } else {
        audio_src_srv_err("[Sink][AM]Set PEQ param error, allocate NC memory fail, keyid:0x%x, length:0x%x\n", 2, keyid, nvkey_buf_len);
        status = -1;
        goto __EXIT;
    }

    status = aud_peq_put_nvkey(nvkey_buf, &nvkey_buf_len, keyid, NULL);
    if (status != NVDM_STATUS_NAT_OK) {
        audio_src_srv_err("[Sink][AM]Set PEQ param error, write to sysram error:%d, keyid:0x%x\n", 2, status, keyid);
        goto __EXIT;
    }

    peq_param.peq_nvkey_id = keyid;
    peq_param.drc_enable = 1;
    peq_param.setting_mode = PEQ_DIRECT;
    peq_param.target_bt_clk = 0;
    peq_param.phase_id = 0;
    peq_param.nvkey_addr = (uint8_t *)((uint32_t)nvkey_buf);

__EXIT:
    if (status != 0) {
        if (nvkey_buf != NULL) {
            vPortFreeNC(nvkey_buf);
        }
        return status;
    }

    audio_src_srv_report("[Sink][AM]Set PEQ phase_id:%d nvkeyID:%x \n", 2, peq_param.phase_id, peq_param.peq_nvkey_id);
    p_param_share = hal_audio_dsp_controller_put_paramter(&peq_param, sizeof(mcu2dsp_peq_param_t), AUDIO_MESSAGE_TYPE_COMMON);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_PEQ_SET_PARAM, special_peq_audio_id, (uint32_t)p_param_share, true);
    LOG_W(MPLOG, "PEQ phase:%d enable:%d nvkey:0x%x", peq_param.phase_id, (peq_param.peq_nvkey_id == 0) ? 0 : 1, peq_param.peq_nvkey_id);

    if (nvkey_buf != NULL) {
        vPortFreeNC(nvkey_buf);
    }
    return status;
}


#ifdef SUPPORT_PEQ_NVKEY_UPDATE
static uint16_t aud_get_nvkey_id_by_bit(uint32_t bit)
{
    if (bit < 7) {
        return (NVKEY_DSP_PARA_PEQ + bit);
    } else if (bit < 12) {
        return (NVKEY_DSP_PARA_PEQ_COEF_29 + bit - 8);
    } else if (bit < 16) {
        return (NVKEY_PEQ_UI_DATA_01 + bit - 12);
    } else {
        return 0;
    }
}

static uint32_t aud_get_bit_by_nvkey_id(uint16_t nvkey_id)
{
    if ((nvkey_id >= NVKEY_DSP_PARA_PEQ) && (nvkey_id <= NVKEY_DSP_PARA_PEQ_PATH_5)) {
        return (nvkey_id - NVKEY_DSP_PARA_PEQ);
    } else if ((nvkey_id >= NVKEY_DSP_PARA_PEQ_COEF_29) && (nvkey_id <= NVKEY_DSP_PARA_PEQ_COEF_32)) {
        return (nvkey_id - NVKEY_DSP_PARA_PEQ_COEF_29 + 8);
    } else if ((nvkey_id >= NVKEY_PEQ_UI_DATA_01) && (nvkey_id <= NVKEY_PEQ_UI_DATA_04)) {
        return (nvkey_id - NVKEY_PEQ_UI_DATA_01 + 12);
    } else {
        return 0xFFFFFFFF;
    }
}

void aud_peq_chk_nvkey(uint16_t nvkey_id)
{
    if ((bt_connection_manager_device_local_info_get_aws_role() == BT_AWS_MCE_ROLE_AGENT) && (bt_sink_srv_cm_get_special_aws_device() != NULL) && (bt_sink_srv_cm_get_aws_link_state() != BT_AWS_MCE_AGENT_STATE_ATTACHED)) {
        uint32_t changed_bit = aud_get_bit_by_nvkey_id(nvkey_id);
        if (changed_bit != 0xFFFFFFFF) {
            g_peq_handle.nvkey_change_mask |= (1 << changed_bit);
        }
    }
}

uint32_t aud_peq_get_changed_nvkey(uint8_t **packet, uint32_t *total_size)
{
    AMI_AWS_MCE_ATTACH_NVDM_PACKT_t *attach_nvdm_pkt;
    ami_aws_mce_attach_peq_nvdm_t *peq_nvdm;
    uint32_t temp_mask = (uint32_t)g_peq_handle.nvkey_change_mask;
    uint32_t bit = 0;
    uint32_t num = 0;
    uint16_t nvkey_array[16];
    uint32_t length;
    uint8_t *payload;

    *packet = NULL;
    *total_size = 0;

    while (temp_mask) {
        if (temp_mask & (1 << bit)) {
            temp_mask &= (~(1 << bit));
            nvkey_array[num] = aud_get_nvkey_id_by_bit(bit);
            if (nvkey_array[num] != 0) {
                num++;
            }
        }
        bit++;
    }
    if (num == 0) {
        attach_nvdm_pkt = (AMI_AWS_MCE_ATTACH_NVDM_PACKT_t *)pvPortMalloc(sizeof(AMI_AWS_MCE_ATTACH_NVDM_PACKT_t));
        if (attach_nvdm_pkt) {
            memset(attach_nvdm_pkt, 0, sizeof(AMI_AWS_MCE_ATTACH_NVDM_PACKT_t));
            *packet = (uint8_t *)attach_nvdm_pkt;
            *total_size = sizeof(AMI_AWS_MCE_ATTACH_NVDM_PACKT_t);
        }
        return 0;
    }

    temp_mask = sizeof(AMI_AWS_MCE_ATTACH_NVDM_PACKT_t);
    for (bit = 0; bit < num; bit++) {
        if (flash_memory_query_nvdm_data_length(nvkey_array[bit], &length) == NVDM_STATUS_NAT_OK) {
            temp_mask += length;
        } else {
            audio_src_srv_err("[mce_ami] aud_peq_get_changed_nvkey error for get nvdm length : nvkey:0x%x \n", 1, nvkey_array[bit]);
            nvkey_array[bit] = 0;
        }
    }

    payload = (uint8_t *)pvPortMalloc(temp_mask);
    if (payload) {
        attach_nvdm_pkt = (AMI_AWS_MCE_ATTACH_NVDM_PACKT_t *)payload;
        payload = (uint8_t *)attach_nvdm_pkt + sizeof(AMI_AWS_MCE_ATTACH_NVDM_PACKT_t);
        temp_mask -= sizeof(AMI_AWS_MCE_ATTACH_NVDM_PACKT_t);
        *packet = (uint8_t *)attach_nvdm_pkt;
        *total_size = sizeof(AMI_AWS_MCE_ATTACH_NVDM_PACKT_t);
        attach_nvdm_pkt->peq_nvkey_num = 0;
        attach_nvdm_pkt->peq_nvkey_slot_num = num;
        attach_nvdm_pkt->peq_nvkey_mask = g_peq_handle.nvkey_change_mask;
        for (bit = 0; bit < num; bit++) {
            if (nvkey_array[bit] != 0) {
                length = temp_mask;
                if (flash_memory_read_nvdm_data(nvkey_array[bit], payload, &length) == NVDM_STATUS_NAT_OK) {
                    peq_nvdm = &attach_nvdm_pkt->peq_nvdm[attach_nvdm_pkt->peq_nvkey_num];
                    peq_nvdm->nvkey_id = nvkey_array[bit];
                    peq_nvdm->nvkey_length = (uint16_t)length;
                    attach_nvdm_pkt->peq_nvkey_num++;
                    payload += length;
                    temp_mask -= length;
                    *total_size += length;
                } else {
                    audio_src_srv_report("[mce_ami] get changed nvkey, read nvdm error, keyid:0x%x buffer_remain_size:%d \n", 2, nvkey_array[bit], length);
                }
            }
        }
        audio_src_srv_report("[mce_ami] send attach nvdm info: peq mask:%x/%x %d %d\n", 4, attach_nvdm_pkt->peq_nvkey_mask, g_peq_handle.nvkey_change_mask, attach_nvdm_pkt->peq_nvkey_num, *total_size);
        g_peq_handle.nvkey_change_mask = 0;
        return 0;
    }

    audio_src_srv_err("[mce_ami] aud_peq_get_changed_nvkey error for malloc, size:%d\n", 1, temp_mask);
    return -1;
}

uint32_t aud_peq_save_changed_nvkey(AMI_AWS_MCE_ATTACH_NVDM_PACKT_t *attach_nvdm_pkt, uint32_t data_size)
{
    ami_aws_mce_attach_peq_nvdm_t *peq_nvdm;
    uint32_t num;
    uint8_t *payload;
    uint32_t used_size;
    uint32_t i;

    if ((attach_nvdm_pkt == NULL) || (data_size == 0) || (attach_nvdm_pkt->peq_nvkey_num > PEQ_ATTACH_NVKEY_MAX) || (attach_nvdm_pkt->peq_nvkey_slot_num > PEQ_ATTACH_NVKEY_MAX)) {
        audio_src_srv_err("[mce_ami] aud_peq_save_changed_nvkey error for abnormal param\n", 0);
        return -1;
    }

    num = attach_nvdm_pkt->peq_nvkey_num;
    used_size = sizeof(AMI_AWS_MCE_ATTACH_NVDM_PACKT_t);
    payload = (uint8_t *)attach_nvdm_pkt + used_size;

    for (i = 0; i < num; i++) {
        peq_nvdm = &attach_nvdm_pkt->peq_nvdm[i];
        if (used_size + peq_nvdm->nvkey_length > data_size) {
            audio_src_srv_err("[mce_ami] aud_peq_save_changed_nvkey error for abnormal data size:%d, used size:%d, next_nvkey_length:%d\n", 3, data_size, used_size, peq_nvdm->nvkey_length);
            return -1;
        }
        if (flash_memory_write_nvdm_data(peq_nvdm->nvkey_id, payload, peq_nvdm->nvkey_length) != NVDM_STATUS_NAT_OK) {
            audio_src_srv_err("[mce_ami] aud_peq_save_changed_nvkey error for save nvkey:0x%x length:%d\n", 2, peq_nvdm->nvkey_id, peq_nvdm->nvkey_length);
        } else {
            uint32_t changed_bit = aud_get_bit_by_nvkey_id(peq_nvdm->nvkey_id);
            audio_src_srv_report("[mce_ami] update nvkey:0x%x length:%d success\n", 2, peq_nvdm->nvkey_id, peq_nvdm->nvkey_length);
            if (changed_bit != 0xFFFFFFFF) {
                g_peq_handle.nvkey_change_mask &= (~(1 << changed_bit));
            }
        }
        used_size += peq_nvdm->nvkey_length;
        payload += peq_nvdm->nvkey_length;
    }
    if (num > 0) {
        return 1;
    }
    return 0;
}
#endif

uint32_t aud_peq_get_sound_mode(bt_sink_srv_am_type_t type, uint8_t *peq_info)
{
    uint32_t ret = -1;
    if ((type == A2DP) || (type == AWS)) {
        peq_info[0] = g_peq_handle.a2dp_pre_peq_enable;
        peq_info[1] = g_peq_handle.a2dp_pre_peq_sound_mode;
        peq_info[2] = g_peq_handle.a2dp_post_peq_enable;
        peq_info[3] = g_peq_handle.a2dp_post_peq_sound_mode;
        ret = 0;
    } else if (type == LINE_IN) {
        peq_info[0] = g_peq_handle.linein_pre_peq_enable;
        peq_info[1] = g_peq_handle.linein_pre_peq_sound_mode;
        peq_info[2] = g_peq_handle.linein_post_peq_enable;
        peq_info[3] = g_peq_handle.linein_post_peq_sound_mode;
        ret = 0;
    }
    return ret;
}

#ifdef AIR_ADAPTIVE_EQ_ENABLE
extern int16_t s_agent_adaptive_eq_index;
extern int16_t s_partner_adaptive_eq_index;

int32_t aud_get_aeq_sound_mode(bt_sink_srv_am_type_t type, uint8_t *peq_info)
{
    aeq_share_info_t *share_addr;
    share_addr = (aeq_share_info_t *)hal_audio_query_share_info(AUDIO_MESSAGE_TYPE_AEQ_MONITOR_ADAPTIVE_EQ);
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    int32_t ret = -1;
    bt_sink_srv_am_type_t cur_type = NONE;
    cur_type = bt_sink_srv_ami_get_current_scenario();

    if(cur_type == A2DP){
        if ((type == A2DP) || (type == AWS)) {
            if(g_peq_handle.adaptive_peq_enable == 0){
                peq_info[0] = g_peq_handle.adaptive_peq_enable;
                peq_info[1] = g_peq_handle.adaptive_peq_sound_mode;
            }else{
                peq_info[0] = g_peq_handle.adaptive_peq_enable;
                peq_info[1] = share_addr->aeq_sound_mode;
                if (role == BT_AWS_MCE_ROLE_AGENT || role == BT_AWS_MCE_ROLE_NONE) {
                    s_agent_adaptive_eq_index = share_addr->aeq_sound_mode;
                } else {
                    s_partner_adaptive_eq_index = share_addr->aeq_sound_mode;
                }
            }
            ret = 0;
        }
    }else{
        peq_info[0] = g_peq_handle.adaptive_peq_enable;
        peq_info[1] = g_peq_handle.adaptive_peq_sound_mode;
        if (role == BT_AWS_MCE_ROLE_AGENT || role == BT_AWS_MCE_ROLE_NONE) {
            s_agent_adaptive_eq_index = share_addr->aeq_sound_mode;
        } else {
            s_partner_adaptive_eq_index = share_addr->aeq_sound_mode;
        }
        ret = 0;
    }
    return ret;
}

uint8_t aud_get_aeq_detect_status(void)
{
    uint8_t ret = -1, aeq_detect_status = 0;
    sysram_status_t status;
    uint32_t length = sizeof(uint8_t);
    status = flash_memory_read_nvdm_data(NVKEY_DSP_PARA_AEQ_MISC, (uint8_t *)&aeq_detect_status, &length);
    if ((status != NVDM_STATUS_NAT_OK)) {
        audio_src_srv_err("[Sink][AM]Read aeq detect status error(%d), length:%d\n", 2, status,sizeof(uint8_t));
    } else {
        ret = aeq_detect_status;
    }
    return ret;
}

uint8_t aud_set_aeq_detect_ip(uint8_t aeq_ip_id)
{
    uint8_t ret = 0;
    AEQ_SZ_PARA_t aeq_para;
    sysram_status_t status;
    uint32_t length = sizeof(AEQ_SZ_PARA_t);
    status = flash_memory_read_nvdm_data(NVKEY_DSP_PARA_AEQ_PARA, (uint8_t *)&aeq_para, &length);
    if ((status != NVDM_STATUS_NAT_OK)) {
        audio_src_srv_err("[Sink][AM]Read aeq set ip read nvk error(%d), length:%d\n", 2, status, sizeof(AEQ_SZ_PARA_t));
    }
    if(aeq_ip_id){
        aeq_para.set_option_s |= (1<<7);
    }else{
        aeq_para.set_option_s &= ~(1<<7);
    }
    if (flash_memory_write_nvdm_data(NVKEY_DSP_PARA_AEQ_PARA, (uint8_t *)&aeq_para, sizeof(AEQ_SZ_PARA_t)) != NVDM_STATUS_NAT_OK) {
        audio_src_srv_err("[Sink][AM]Write AEQ detect set ip error\n", 0);
        ret = -1;
    }
    return ret;
}

uint8_t aud_get_aeq_detect_ip_status(void)
{
    uint8_t ret = -1;
    AEQ_SZ_PARA_t aeq_para;
    sysram_status_t status;
    uint32_t length = sizeof(AEQ_SZ_PARA_t);
    status = flash_memory_read_nvdm_data(NVKEY_DSP_PARA_AEQ_PARA, (uint8_t *)&aeq_para, &length);
    if ((status != NVDM_STATUS_NAT_OK)) {
        audio_src_srv_err("[Sink][AM]Read aeq set detect ip nvk error(%d), length:%d\n", 2, status, sizeof(AEQ_SZ_PARA_t));
    } else {
        ret = (uint8_t)((aeq_para.set_option_s>>7)&1);
    }
    return ret;
}

uint8_t aud_get_aeq_detect_bypass_status(void)
{
    uint8_t ret = -1;
    AEQ_SZ_PARA_t aeq_para;
    sysram_status_t status;
    uint32_t length = sizeof(AEQ_SZ_PARA_t);
    status = flash_memory_read_nvdm_data(NVKEY_DSP_PARA_AEQ_PARA, (uint8_t *)&aeq_para, &length);
    if ((status != NVDM_STATUS_NAT_OK)) {
        audio_src_srv_err("[Sink][AM]Read AEQ detect bypass error(%d), length:%d\n", 2, status, sizeof(AEQ_SZ_PARA_t));
    } else {
        ret = aeq_para.detect_bypass_s;
    }
    return ret;
}

uint8_t aud_set_aeq_detect_bypass(uint8_t aeq_detect_bypass_id)
{
    uint8_t ret = 0;
    AEQ_SZ_PARA_t aeq_para;
    sysram_status_t status;
    uint32_t length = sizeof(AEQ_SZ_PARA_t);
    status = flash_memory_read_nvdm_data(NVKEY_DSP_PARA_AEQ_PARA, (uint8_t *)&aeq_para, &length);
    if ((status != NVDM_STATUS_NAT_OK)) {
        audio_src_srv_err("[Sink][AM]Read aeq set detect bypass nvk error(%d), length:%d\n", 2, status, sizeof(AEQ_SZ_PARA_t));
        ret = -1;
    }

    aeq_para.detect_bypass_s = aeq_detect_bypass_id;
    if (flash_memory_write_nvdm_data(NVKEY_DSP_PARA_AEQ_PARA, (uint8_t *)&aeq_para, sizeof(AEQ_SZ_PARA_t)) != NVDM_STATUS_NAT_OK) {
        audio_src_srv_err("[Sink][AM]Write AEQ set detect bypass error\n", 0);
        ret = -1;
    }
    if(ret == 0){
        if(aeq_detect_bypass_id){
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_ADEQ_SET_PARAM, AEQ_SET_DETECT_BYPASS_ENABLE, 0, true);
        }else{
            hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_ADEQ_SET_PARAM, AEQ_SET_DETECT_BYPASS_DISABLE, 0, true);
        }
    }
    return ret;
}
#endif

uint32_t aud_peq_reinit_nvdm(void)
{
#define CHECK_KEEP(nvkey_id) ((nvkey_id >= NVKEY_DSP_PARA_PEQ_PATH_0) && (nvkey_id < NVKEY_DSP_PARA_PEQ_COEF_29)) ? 1 : 0
    sysram_status_t status;
    uint32_t ret = -1;
    uint8_t T_NVDM_F232[] =
    {
        0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x01,0x01,0x03,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,
    };
    peq_full_set_t *full_set = NULL, *full_set2 = NULL;
    peq_single_set_t *single_set, *single_set2;
    peq_single_phase_t *single_phase, *single_phase2;
    uint32_t i, j;
    uint16_t full_set_nvkey_id;
    uint32_t full_set_size;

    status = flash_memory_write_nvdm_data(NVKEY_DSP_PARA_PEQ_MISC_PARA, (uint8_t *)T_NVDM_F232, sizeof(T_NVDM_F232));
    if (status != NVDM_STATUS_NAT_OK) {
        audio_src_srv_err("[Sink][AM] peq failed to reinit 0x%x from nvdm - err(%d)\r\n", 2, NVKEY_DSP_PARA_PEQ_MISC_PARA, status);
    }

    if (aud_peq_query_full_set_size(PEQ_AUDIO_PATH_A2DP, &full_set_nvkey_id, &full_set_size) == 0) {
        full_set = (peq_full_set_t *)pvPortMalloc(full_set_size);
        full_set2 = (peq_full_set_t *)pvPortMalloc(full_set_size);
        if ((full_set == NULL) || (full_set2 == NULL)) {
            audio_src_srv_err("[Sink][AM] aud_peq_reinit_nvdm, pvPortMalloc failed.\n", 0);
            goto __END;
        } else {
            memset(full_set, 0, full_set_size);
            memset(full_set2, 0, full_set_size);
            status = flash_memory_read_nvdm_data(full_set_nvkey_id, (uint8_t *)full_set, &full_set_size);
            if (status != NVDM_STATUS_NAT_OK) {
                audio_src_srv_err("[Sink][AM]aud_peq_reinit_nvdm, read nvdm from flash error:%d, keyid:0x%x\n", 2, status, full_set_nvkey_id);
                goto __END;
            }
        }
    } else {
        goto __END;
    }

    single_set = &full_set->setList[0];
    single_set2 = &full_set2->setList[0];
    full_set2->numOfSet = 0;
    full_set_size = sizeof(uint16_t);
    for (i = 0; i < full_set->numOfSet; i++) {
        single_phase = &single_set->phaseList[0];
        single_phase2 = &single_set2->phaseList[0];
        single_set2->numOfPhase = 0;
        for (j = 0; j < single_set->numOfPhase; j++) {
            if (CHECK_KEEP(single_phase->nvkeyID)) {
                single_phase2->phaseID = single_phase->phaseID;
                single_phase2->nvkeyID = single_phase->nvkeyID;
                single_set2->numOfPhase++;
                single_phase2++;
                full_set_size += sizeof(peq_single_phase_t);
            }
            single_phase++;
        }
        if (single_set2->numOfPhase > 0) {
            full_set2->numOfSet++;
            full_set_size += sizeof(uint16_t);
        } else {
            break;
        }
        single_set = (peq_single_set_t *)((uint32_t)single_set + sizeof(peq_single_phase_t) * single_set->numOfPhase + sizeof(uint16_t));
        single_set2 = (peq_single_set_t *)((uint32_t)single_set2 + sizeof(peq_single_phase_t) * single_set2->numOfPhase + sizeof(uint16_t));
    }

    status = flash_memory_write_nvdm_data(full_set_nvkey_id, (uint8_t *)full_set2, full_set_size);
    if (status != NVDM_STATUS_NAT_OK) {
        audio_src_srv_err("[Sink][AM] peq failed to reinit 0x%x from nvdm - err(%d)\r\n", 2, full_set_nvkey_id, status);
        goto __END;
    }

    ret = 0;

__END:
    vPortFree(full_set);
    vPortFree(full_set2);
    return ret;
}

uint8_t aud_peq_get_peq_status(peq_audio_path_id_t audio_path, uint8_t phase_id)
{
#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
    if(g_peq_init_flag == 0){
        aud_peq_init();
    }
#endif
    uint8_t ret = -1;
    if (audio_path == PEQ_AUDIO_PATH_A2DP) {
        if (phase_id == 0) {
            ret = g_peq_handle.a2dp_pre_peq_enable;
            return ret;
        } else if (phase_id == 1) {
            ret = g_peq_handle.a2dp_post_peq_enable;
            return ret;
        }
    } else if (audio_path == PEQ_AUDIO_PATH_LINEIN) {
        if (phase_id == 0) {
            ret = g_peq_handle.linein_pre_peq_enable;
            return ret;
        } else if (phase_id == 1) {
            ret = g_peq_handle.linein_post_peq_enable;
            return ret;
        }
#ifdef AIR_ADAPTIVE_EQ_ENABLE
    } else if (audio_path == PEQ_AUDIO_PATH_ADAPTIVE_EQ) {
        if (phase_id == 0) {
            ret = g_peq_handle.adaptive_peq_enable;
            return ret;
        }
#endif
    }
    audio_src_srv_err("[Sink][AM]aud_peq_get_peq_status error\n", 0);
    return ret;
}

uint8_t aud_peq_get_current_sound_mode(peq_audio_path_id_t audio_path, uint8_t phase_id)
{
    uint8_t ret = -1;
    if (audio_path == PEQ_AUDIO_PATH_A2DP) {
        if (phase_id == 0) {
            ret = g_peq_handle.a2dp_pre_peq_sound_mode;
            return ret;
        } else if (phase_id == 1) {
            ret = g_peq_handle.a2dp_post_peq_sound_mode;
            return ret;
        }
    } else if (audio_path == PEQ_AUDIO_PATH_LINEIN) {
        if (phase_id == 0) {
            ret = g_peq_handle.linein_pre_peq_sound_mode;
            return ret;
        } else if (phase_id == 1) {
            ret = g_peq_handle.linein_post_peq_sound_mode;
            return ret;
        }
    }
    audio_src_srv_err("[Sink][AM]aud_peq_get_current_sound_mode error\n", 0);
    return ret;
}

int32_t aud_peq_get_total_mode(peq_audio_path_id_t audio_path, uint8_t phase_id)
{
    int32_t total_mode = -1;
    if (audio_path == PEQ_AUDIO_PATH_A2DP) {
        peq_full_set_t *full_set = NULL;
        uint16_t full_set_nvkey_id;
        uint32_t full_set_size;
        if (aud_peq_query_full_set_size(audio_path, &full_set_nvkey_id, &full_set_size) == 0) {
            full_set = (peq_full_set_t *)pvPortMalloc(full_set_size);
            if (full_set == NULL) {
                audio_src_srv_err("[Sink][AM]aud_peq_get_total_mode error, pvPortMalloc failed.\n", 0);
                return -1;
            } else {
                sysram_status_t status = flash_memory_read_nvdm_data(full_set_nvkey_id, (uint8_t *)full_set, &full_set_size);
                if (status == NVDM_STATUS_NAT_OK) {
                    uint32_t i, j;
                    uint16_t *p_cur = (uint16_t *)&full_set->setList[0];
                    uint16_t phase_num;
                    total_mode = 0;
                    for (i = 0; i < full_set->numOfSet; i++) {
                        phase_num = *p_cur;
                        p_cur++;
                        for (j = 0; j < phase_num; j++) {
                            if (*p_cur == phase_id) {
                                total_mode++;
                            }
                            p_cur += 2;
                        }
                    }
                } else {
                    audio_src_srv_err("[Sink][AM]aud_peq_get_total_mode error, read nvdm from flash error:%d, keyid:0x%x\n", 2, status, full_set_nvkey_id);
                }
            }
        }
        if (full_set != NULL) {
            vPortFree(full_set);
        }
    }
    audio_src_srv_report("[Sink][AM]aud_peq_get_total_mode, auido_path:%d phase_id:%d total sound mode:%d\n", 3, audio_path, phase_id, total_mode);
    return total_mode;
}

#endif

void audio_set_anc_compensate(bt_sink_srv_am_type_t type, uint32_t event, bt_sink_srv_am_type_t *cur_type)
{
    uint32_t voice_eq_nvkey_is_ready = 0;
#ifdef MTK_VOICE_ANC_EQ
    uint8_t *nvkey_buf = NULL;
#endif
#ifdef MTK_ANC_ENABLE
#ifndef MTK_ANC_V2
    uint32_t on_event = ANC_CONTROL_EVENT_ON;
#else
    uint32_t on_event = AUDIO_ANC_CONTROL_EVENT_ON;
#endif
#else
    uint32_t on_event = 1 << 0;
#endif

    if (type == HFP) {
        voice_eq_nvkey_is_ready = 1;
    } else if (type == NONE) {
        type = (g_prCurrent_player != NULL) ? g_prCurrent_player->type : NONE;
    }
#ifdef AIR_WIRED_AUDIO_ENABLE
    uint8_t LINE_INENABLE = 0;
    audio_transmitter_scenario_list_t audio_transmitter_scenario_list[]  =  {
        {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN},
    };

    LINE_INENABLE = audio_transmitter_get_is_running_by_scenario_list(audio_transmitter_scenario_list, sizeof(audio_transmitter_scenario_list) / sizeof(audio_transmitter_scenario_list_t));
    if(LINE_INENABLE == true){
        type = LINE_IN;
    }
#endif

    if (event == 0) {
        uint8_t anc_enable = 0;
#ifdef MTK_ANC_ENABLE
#ifndef MTK_ANC_V2
        uint8_t hybrid_enable;
        anc_get_status(&anc_enable, NULL, &hybrid_enable);
        if ((anc_enable > 0) && (hybrid_enable > 0)) {
            event = ANC_CONTROL_EVENT_ON;
        } else {
            event = ANC_CONTROL_EVENT_OFF;
        }
#else
        audio_anc_control_get_status(&anc_enable, NULL, NULL, NULL, NULL, NULL);
        if (anc_enable > 0) {
            event = AUDIO_ANC_CONTROL_EVENT_ON;
        } else {
            event = AUDIO_ANC_CONTROL_EVENT_OFF;
        }
#endif
#else
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)||defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
        RACE_DSPREALTIME_COSYS_GET_PARAM(AM_ANC, &anc_enable);
        if (anc_enable > 0) {
            event = 1 << 0;
        } else {
            event = 1 << 1;
        }
#else
        _UNUSED(anc_enable);
        event = 1 << 1;
#endif
#endif
    }

    if (cur_type != NULL) {
        *cur_type = type;
    }
    switch (type) {
        case A2DP:
        case AWS: {
#ifdef MTK_PEQ_ENABLE
            bt_sink_srv_am_peq_param_t peq_param;
            memset(&peq_param, 0, sizeof(bt_sink_srv_am_peq_param_t));
            /* post PEQ */
            peq_param.phase_id = 1;
#ifdef MTK_ANC_ENABLE
#ifndef MTK_ANC_V2
            if (event == on_event) {
                peq_param.enable = 1;
                peq_param.sound_mode = POST_PEQ_FBANC_SOUND_MODE;
            } else {
                peq_param.enable = POST_PEQ_DEFAULT_ENABLE;
                peq_param.sound_mode = POST_PEQ_DEFAULT_SOUND_MODE;
            }
            aud_set_peq_param(aud_get_peq_audio_path(A2DP), &peq_param);
            g_peq_handle.a2dp_post_peq_enable = peq_param.enable;
            if (peq_param.enable) {
                g_peq_handle.a2dp_post_peq_sound_mode = peq_param.sound_mode;
            }
#else
            if (event == on_event) {
                peq_param.enable = g_peq_handle.a2dp_post_peq_enable;
                peq_param.sound_mode = POST_PEQ_FBANC_SOUND_MODE;
            } else {
                peq_param.enable = g_peq_handle.a2dp_post_peq_enable;
                peq_param.sound_mode = g_peq_handle.a2dp_post_peq_sound_mode;
            }
            aud_set_peq_param(aud_get_peq_audio_path(A2DP), &peq_param);
#endif
#else
            if (event == on_event) {
                peq_param.enable = g_peq_handle.a2dp_post_peq_enable;
                peq_param.sound_mode = POST_PEQ_FBANC_SOUND_MODE;
            } else {
                peq_param.enable = g_peq_handle.a2dp_post_peq_enable;
                peq_param.sound_mode = g_peq_handle.a2dp_post_peq_sound_mode;
            }
            aud_set_peq_param(aud_get_peq_audio_path(A2DP), &peq_param);
#endif
#endif
            break;
        }
        case LINE_IN:
        case USB_AUDIO_IN:{
#ifdef MTK_PEQ_ENABLE
            bt_sink_srv_am_peq_param_t peq_param;
            memset(&peq_param, 0, sizeof(bt_sink_srv_am_peq_param_t));
            /* post PEQ */
            peq_param.phase_id = 1;
#ifdef MTK_ANC_ENABLE
#ifndef MTK_ANC_V2
            if (event == on_event) {
                peq_param.enable = 1;
                peq_param.sound_mode = POST_PEQ_FBANC_SOUND_MODE;
            } else {
                peq_param.enable = POST_PEQ_DEFAULT_ENABLE;
                peq_param.sound_mode = POST_PEQ_DEFAULT_SOUND_MODE;
            }
            aud_set_peq_param(PEQ_AUDIO_PATH_LINEIN, &peq_param);
            g_peq_handle.linein_post_peq_enable = peq_param.enable;
            if (peq_param.enable) {
                g_peq_handle.linein_post_peq_sound_mode = peq_param.sound_mode;
            }
#else
            if (event == on_event) {
                peq_param.enable = g_peq_handle.linein_post_peq_enable;
                peq_param.sound_mode = POST_PEQ_FBANC_SOUND_MODE;
            } else {
                peq_param.enable = g_peq_handle.linein_post_peq_enable;
                peq_param.sound_mode = g_peq_handle.linein_post_peq_sound_mode;
            }
            aud_set_peq_param(PEQ_AUDIO_PATH_LINEIN, &peq_param);
#endif
#else
            if (event == on_event) {
                peq_param.enable = g_peq_handle.linein_post_peq_enable;
                peq_param.sound_mode = POST_PEQ_FBANC_SOUND_MODE;
            } else {
                peq_param.enable = g_peq_handle.linein_post_peq_enable;
                peq_param.sound_mode = g_peq_handle.linein_post_peq_sound_mode;
            }
            aud_set_peq_param(PEQ_AUDIO_PATH_LINEIN, &peq_param);
#endif
#endif
            break;
        }
        case HFP: {
#ifdef MTK_VOICE_ANC_EQ
            /* Voice receiving EQ */
            if (voice_eq_nvkey_is_ready == 0) {
                mem_nvdm_info_t flash_nvdm;
                sysram_status_t status;
                uint16_t nvkey1, nvkey2, target_nvkey = 0;
                if (g_prCurrent_player->local_context.hfp_format.hfp_codec.type == BT_HFP_CODEC_TYPE_CVSD) {
                    nvkey1 = NVKEY_DSP_PARA_NB_RX_EQ;
                    nvkey2 = NVKEY_DSP_PARA_NB_RX_EQ_2ND;
                    if (event == on_event) {
                        flash_nvdm.nvdm_id = NVKEY_DSP_PARA_NB_RX_EQ_2ND;
                    } else {
                        flash_nvdm.nvdm_id = NVKEY_DSP_PARA_NB_RX_EQ;
                    }
                } else {
                    nvkey1 = NVKEY_DSP_PARA_WB_RX_EQ;
                    nvkey2 = NVKEY_DSP_PARA_WB_RX_EQ_2ND;
                    if (event == on_event) {
                        flash_nvdm.nvdm_id = NVKEY_DSP_PARA_WB_RX_EQ_2ND;
                    } else {
                        flash_nvdm.nvdm_id = NVKEY_DSP_PARA_WB_RX_EQ;
                    }
                }
                flash_nvdm.length = 0; //dummy
                status = flash_memory_query_nvdm_data_length(flash_nvdm.nvdm_id, &flash_nvdm.length);
                if (status != NVDM_STATUS_NAT_OK) {
                    audio_src_srv_err("change RX EQ error: query nvkey length error status:%d ", 1, status);
                    break;
                } else {
                    target_nvkey = flash_nvdm.nvdm_id;
                }
                nvkey_buf = (uint8_t *)pvPortMalloc(flash_nvdm.length);
                if (nvkey_buf == NULL) {
                    audio_src_srv_err("change RX EQ error: malloc fail size:%d ", 1, flash_nvdm.length);
                    break;
                }
                status = flash_memory_read_nvdm_data(flash_nvdm.nvdm_id, nvkey_buf, &flash_nvdm.length);
                if (status != NVDM_STATUS_NAT_OK) {
                    audio_src_srv_err("change RX EQ error: read nvkey error status:%d ", 1, status);
                    break;
                }
                flash_nvdm.mem_pt = nvkey_buf;
                flash_nvdm.nvdm_id = nvkey1;
                status = nat_table_write_audio_nvdm_data(flash_nvdm, c_sram_mode);
                if (status != NVDM_STATUS_NAT_OK) {
                    flash_nvdm.nvdm_id = nvkey2;
                    status = nat_table_write_audio_nvdm_data(flash_nvdm, c_sram_mode);
                }
                if (status != NVDM_STATUS_NAT_OK) {
                    audio_src_srv_err("set HFP AEC NR error, param_type: AEC_NR_PARAM_TYPE_RX_EQ, status:%d", 1, status);
                } else {
                    audio_src_srv_report("set HFP AEC NR success, param_type: AEC_NR_PARAM_TYPE_RX_EQ, nvkey:0x%x", 1, target_nvkey);
                    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_AEC_NR_SET_PARAM, AEC_NR_PARAM_TYPE_RX_EQ, 0, true);
                }
            }
#else
            _UNUSED(voice_eq_nvkey_is_ready);
#endif
            break;
        }
        case NONE:
            break;
    }
#ifdef MTK_VOICE_ANC_EQ
    if (nvkey_buf != NULL) {
        vPortFree(nvkey_buf);
    }
#endif
    _UNUSED(on_event);
}

#ifdef MTK_ANC_ENABLE
void audio_set_anc_compensate_phase2(bt_sink_srv_am_type_t type, uint32_t event)
{
    _UNUSED(type);
    _UNUSED(event);
#ifndef MTK_ANC_V2
#ifdef MTK_DEQ_ENABLE
    bt_sink_srv_am_peq_param_t peq_param;
    memset(&peq_param, 0, sizeof(bt_sink_srv_am_peq_param_t));
    if (type == NONE) {
        type = (g_prCurrent_player != NULL) ? g_prCurrent_player->type : NONE;
    }
    if ((type != A2DP) && (type != AWS)) { //DEQ only used in A2DP case now.
        return;
    }
    if (event == 0) {
        uint8_t anc_enable, hybrid_enable;
        anc_get_status(&anc_enable, NULL, &hybrid_enable);
        if ((anc_enable > 0) && (hybrid_enable > 0)) {
            event = ANC_CONTROL_EVENT_ON;
        } else {
            event = ANC_CONTROL_EVENT_OFF;
        }
    }
    peq_param.phase_id = 2;
    if (event == ANC_CONTROL_EVENT_OFF) {
        peq_param.enable = 0;
    } else if (event == ANC_CONTROL_EVENT_ON) {
        peq_param.enable = anc_get_deq_enable();
        peq_param.sound_mode = DEQ_AUDIO_SOUND_MODE;
        hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_DEQ_SET_PARAM, 0, anc_get_deq_param(), true);
    }
    aud_set_peq_param(aud_get_peq_audio_path(A2DP), &peq_param);
#endif
#else
    //No need to use PEQ as DEQ.
#endif
}

#endif

#ifdef AIR_PSAP_ENABLE
int32_t music_get_feature_mode(void)
{
    uint8_t music_psap_mode = 0;
    sysram_status_t status;
    uint32_t length = sizeof(uint8_t);
    status = flash_memory_read_nvdm_data(NVKEY_DSP_PARA_PSAP_AUDIO_PEQ_MISC, (uint8_t *)&music_psap_mode, &length);
    if ((status != NVDM_STATUS_NAT_OK)) {
        audio_src_srv_err("[Sink][AM]Read music psap mode error(%d), length:%d\n", 2, status,sizeof(uint8_t));
    }
    audio_src_srv_report("get music psap feature mode:%d", 1, music_psap_mode);
    g_music_psap_feature_mode = music_psap_mode;
    return music_psap_mode;
}

bool music_set_feature_mode(uint8_t mode)
{
    uint8_t music_psap_mode = mode;

    if (mode >= MUSIC_FEATURE_MODE_MAX) {
        audio_src_srv_report("music psap feature mode: invalid mode: %d", 1, mode);
        return false;
    }
    if ((mode == MUSIC_FEATURE_MODE_NORMAL) && (g_music_psap_feature_mode == MUSIC_FEATURE_MODE_NORMAL)) {
        audio_src_srv_report("music psap feature mode: no mode cahnge: %d", 1, mode);
        return false;
    }

    g_music_psap_feature_mode = mode;
    if (flash_memory_write_nvdm_data(NVKEY_DSP_PARA_PSAP_AUDIO_PEQ_MISC, (uint8_t *)&music_psap_mode, sizeof(uint8_t)) != NVDM_STATUS_NAT_OK) {
        audio_src_srv_err("[Sink][AM]Write music psap mode error\n", 0);
    }
    audio_src_srv_report("music psap feature mode: set mode done: %d", 1, mode);

#ifdef MTK_PEQ_ENABLE
    bt_sink_srv_am_peq_param_t am_peq_param;
    memset(&am_peq_param, 0, sizeof(bt_sink_srv_am_peq_param_t));
    /* set pre PEQ*/
    am_peq_param.phase_id = 0;
    am_peq_param.enable = g_peq_handle.a2dp_pre_peq_enable;
    am_peq_param.sound_mode = g_peq_handle.a2dp_pre_peq_sound_mode;

    aud_set_peq_param(PEQ_AUDIO_PATH_A2DP, &am_peq_param);
#endif
    return true;
}

static int32_t aud_set_psap_peq_param(bt_sink_srv_am_peq_param_t *ami_peq_param)
{
    sysram_status_t status = NVDM_STATUS_NAT_OK;
    uint8_t sound_mode = ami_peq_param->sound_mode;
    mcu2dsp_peq_param_t peq_param;
    void *p_param_share;
    uint32_t nvkey_buf_len = 0;
    uint8_t *nvkey_buf = NULL;

    memset(&peq_param, 0, sizeof(mcu2dsp_peq_param_t));

    if (ami_peq_param->enable == 0) {
        peq_param.peq_nvkey_id = 0;
        peq_param.drc_enable = 0;
        peq_param.setting_mode = ami_peq_param->setting_mode;
        peq_param.target_bt_clk = ami_peq_param->target_bt_clk;
        peq_param.phase_id = ami_peq_param->phase_id;
    } else if (sound_mode == PEQ_SOUND_MODE_REAlTIME) {
        return aud_peq_realtime_update(&peq_param, ami_peq_param, PEQ_AUDIO_PATH_A2DP);
    } else {

        uint16_t keyid = NVID_DSP_ALG_PSAP_AUDIO_PEQ;

        status = flash_memory_query_nvdm_data_length(keyid, &nvkey_buf_len);
        if (status != NVDM_STATUS_NAT_OK) {
            audio_src_srv_err("[Sink][AM]Set music psap peq param error, query nvkey length error:%d, keyid:0x%x\n", 2, status, keyid);
        }

        nvkey_buf_len += sizeof(nat_nvdm_info_t);
        nvkey_buf = (uint8_t *)pvPortMallocNC(nvkey_buf_len);
        if (nvkey_buf != NULL) {
            memset(nvkey_buf, 0, nvkey_buf_len);
        } else {
            audio_src_srv_err("[Sink][AM]Set music psap peq param error, allocate NC memory fail, keyid:0x%x, length:0x%x\n", 2, keyid, nvkey_buf_len);
            status = -1;
            goto __EXIT;
        }

        status = aud_peq_put_nvkey(nvkey_buf, &nvkey_buf_len, keyid, NULL);
        if (status != NVDM_STATUS_NAT_OK) {
            audio_src_srv_err("[Sink][AM]Set music psap peq param error, write to sysram error:%d, keyid:0x%x\n", 2, status, keyid);
            goto __EXIT;
        }

        peq_param.peq_nvkey_id = keyid;
        peq_param.drc_enable = 1;
        peq_param.setting_mode = ami_peq_param->setting_mode;
        peq_param.target_bt_clk = ami_peq_param->target_bt_clk;
        peq_param.phase_id = ami_peq_param->phase_id;
        peq_param.nvkey_addr = (uint8_t *)((uint32_t)nvkey_buf);

__EXIT:
        if (status != 0) {
            if (nvkey_buf != NULL) {
                vPortFreeNC(nvkey_buf);
            }
            return status;
        }
    }

    audio_src_srv_report("[Sink][AM]Set music psap peq phase_id:%d enable:%d, nvkeyID:%x setting_mode:%d target_bt_clk:%d\n", 5, ami_peq_param->phase_id, ami_peq_param->enable, peq_param.peq_nvkey_id, ami_peq_param->setting_mode, ami_peq_param->target_bt_clk);
    p_param_share = hal_audio_dsp_controller_put_paramter(&peq_param, sizeof(mcu2dsp_peq_param_t), AUDIO_MESSAGE_TYPE_COMMON);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_COMMON_PEQ_SET_PARAM, PEQ_AUDIO_PATH_A2DP, (uint32_t)p_param_share, true);
    LOG_W(MPLOG, "psap peq phase:%d enable:%d nvkey:0x%x", peq_param.phase_id, (peq_param.peq_nvkey_id == 0) ? 0 : 1, peq_param.peq_nvkey_id);

    if (nvkey_buf != NULL) {
        vPortFreeNC(nvkey_buf);
    }
    return status;
}
#endif /* AIR_PSAP_ENABLE */

#if defined(AIR_HEARTHROUGH_VIVID_PT_ENABLE)
int32_t aud_set_vivid_pt_peq_param(void)
{
    bt_sink_srv_am_peq_param_t am_peq_param;

    am_peq_param.phase_id = 0;
    am_peq_param.enable     = g_peq_handle.vivid_pt_peq_enable;
    am_peq_param.sound_mode = g_peq_handle.vivid_pt_peq_sound_mode;
    am_peq_param.setting_mode = PEQ_DIRECT;
    aud_set_peq_param(PEQ_AUDIO_PATH_VIVID_PT, &am_peq_param);

    return 0;
}
#endif

void am_voice_sent_leq_gain(void)
{
    uint16_t leq_gain;
    leq_gain = hal_audio_dsp2mcu_data_get();
 #ifdef MTK_AWS_MCE_ENABLE
    bt_aws_mce_report_info_t info;
    info.module_id = BT_AWS_MCE_REPORT_MODULE_VOICE_LEQ;
    info.param_len = sizeof(uint16_t);
    info.param = &leq_gain;
    bt_aws_mce_report_send_event(&info);
    audio_src_srv_report("[NDVC] Agent sent IF pkt, leq_gain: %d", 1, leq_gain);
#endif

    // Sent leq_gain back to agent to update agent's leq_gain
    bt_sink_srv_am_feature_t feature_param;
    feature_param.type_mask = AM_VOICE_LEQ;
    feature_param.feature_param.cpd_param.leq_gain = leq_gain;
    am_audio_set_feature_ISR(FEATURE_NO_NEED_ID, &feature_param);
    //audio_src_srv_report("[NDVC] Agent sent IF pkt done", 0);
}

#ifdef MTK_AWS_MCE_ENABLE
void am_voice_leq_callback(bt_aws_mce_report_info_t *para)
{
    if (para != NULL) {
        uint32_t *leq_gain = para->param;
        bt_sink_srv_am_feature_t feature_param;
        feature_param.type_mask = AM_VOICE_LEQ;
        feature_param.feature_param.cpd_param.leq_gain = *leq_gain;
        audio_src_srv_report("[NDVC] Partner receive IF pkt, leq_gain: %d", 1, leq_gain);
        am_audio_set_feature_ISR(FEATURE_NO_NEED_ID, &feature_param);
    } else {
        audio_src_srv_report("[am_voice_leq_callback para] if NULL", 0);
    }
}
#endif

bt_sink_srv_am_cpd_param_t g_cpd_control;
static bt_sink_srv_am_result_t am_audio_cpd_get_nvdm_param(cpd_nvdm_param_t *cpd_param)
{
    bt_sink_srv_am_result_t ret = AUD_EXECUTION_SUCCESS;
    uint32_t length = sizeof(cpd_nvdm_param_t);
    sysram_status_t status;
    uint32_t tableSize = 0;
    if (cpd_param != NULL) {
        status = flash_memory_query_nvdm_data_length(NVID_DSP_ALG_VOICE_PEQ_PARA, &tableSize);
        if (status || !tableSize) {
            audio_src_srv_report("[AMI] am_audio_cpd_get_nvdm_param fail, Status:%u Len:%lu\n", 2, status, tableSize);
            ret = AUD_EXECUTION_FAIL;
        }
        if (tableSize != length) {
            audio_src_srv_report("[AMI] am_audio_cpd_get_nvdm_param size not match. NVDM tableSize (%lu) != (%d)\n", 2, tableSize, length);
            ret = AUD_EXECUTION_FAIL;
        }
        status = flash_memory_read_nvdm_data(NVID_DSP_ALG_VOICE_PEQ_PARA, (uint8_t *)cpd_param, &length);
    } else {
        ret = AUD_EXECUTION_FAIL;
    }
    return ret;
}

bt_sink_srv_am_result_t am_audio_cpd_init(void)
{
    bt_sink_srv_am_result_t  ret = AUD_EXECUTION_SUCCESS;
    cpd_nvdm_param_t pram_local;

    memset(&g_cpd_control, 0, sizeof(bt_sink_srv_am_cpd_param_t));

    ret = am_audio_cpd_get_nvdm_param(&pram_local);
    if (ret == AUD_EXECUTION_SUCCESS) {
        g_cpd_control.hse_mode = pram_local.hse_mode;
    }
    return ret;
}

voice_cpd_hse_mode_t am_audio_cpd_get_hse_mode(void)
{
    audio_src_srv_report("[CPD][AM] am_audio_cpd_get_hse_mode %d",1,g_cpd_control.hse_mode);

    return g_cpd_control.hse_mode;
}

void am_audio_cpd_set_hse_mode(voice_cpd_hse_mode_t mode)
{
    g_cpd_control.hse_mode = mode;
}

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

#include "bt_sink_srv_a2dp.h"
#include "bt_sink_srv_avrcp.h"
#include "bt_sink_srv_music.h"
#include "bt_sdp.h"
#include "bt_di.h"
#include "hal_audio.h"
#include "hal_audio_message_struct.h"
#include "bt_sink_srv_utils.h"
#include "avm_direct.h"
#ifdef MTK_BT_SPEAKER_ENABLE
#ifdef MTK_BT_SPEAKER_FEC_ENABLE
#include "speaker_fec.h"
#endif
#endif
#ifdef __BT_AWS_MCE_A2DP_SUPPORT__
#include "bt_sink_srv_aws_mce_a2dp.h"
#endif
#include "bt_avm.h"
#ifdef MTK_BT_A2DP_VENDOR_CODEC_BC_ENABLE
#include "hal_audio_message_struct.h"
#include "hal_audio_internal.h"
#endif
#include "bt_sink_srv_ami.h"
#include "bt_device_manager.h"
#include "nvkey.h"
#include "nvkey_id_list.h"
#include "bt_device_manager_internal.h"
#include "bt_iot_device_white_list.h"
#include "bt_gap.h"
#ifdef AIR_LE_AUDIO_ENABLE
#include "bt_sink_srv_le_cap.h"
#endif

#ifdef BT_SINK_DUAL_ANT_ENABLE
#include "bt_sink_srv_dual_ant.h"
#endif
#include "bt_utils.h"
#ifdef MTK_BT_A2DP_AAC_ENABLE
#define BT_SINK_SRV_A2DP_AAC_SUPPORT
#endif
#include "bt_sink_srv_nvkey_struct.h"
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
#include "bt_ull_le_audio_manager.h"
#endif
#ifdef AIR_BT_A2DP_LC3PLUS_ENABLE
#include "bt_a2dp_vend_lc3plus.h"
#endif
#ifdef AIR_BT_A2DP_VENDOR_2_ENABLE
#include "bt_a2dp_vend_lhdc_v5.h"
#endif

/* currently care sink role */
#define SBC_CODEC_NUM (0x01)

#ifdef BT_SINK_SRV_A2DP_AAC_SUPPORT
#define AAC_CODEC_NUM   (0x01)
#else
#define AAC_CODEC_NUM   (0x00)
#endif

#ifdef AIR_BT_A2DP_VENDOR_CODEC_SUPPORT
#define VENDOR_CODEC_NUM   (0x01)
#else
#define VENDOR_CODEC_NUM   (0x00)
#endif

#define BT_SINK_SRV_A2DP_CODEC_MAX_NUM (SBC_CODEC_NUM+AAC_CODEC_NUM+VENDOR_CODEC_NUM)

bt_sink_srv_a2dp_pseudo_handle_t g_a2dp_pse_hd[BT_SINK_SRV_A2DP_PSEUDO_COUNT + BT_SINK_SRV_MCE_A2DP_PSEUDO_COUNT] = {{0}};

bt_a2dp_codec_capability_t g_bt_sink_srv_a2dp_codec_list[BT_SINK_SRV_A2DP_CODEC_MAX_NUM];

#ifdef BT_SINK_SRV_A2DP_AAC_SUPPORT
static bool g_bt_sink_srv_aac_open = true;
#endif
#ifdef AIR_BT_A2DP_VENDOR_CODEC_SUPPORT
static bool g_bt_sink_srv_vendor_codec_open = true;
#endif
static bool g_bt_sink_srv_a2dp_init_complete = false;
uint8_t g_sn_wrap_count = 0;
#ifdef AIR_BT_A2DP_LC3PLUS_ENABLE
bt_sink_srv_a2dp_lc3plus_change_t g_bt_sink_srv_a2dp_lc3plus_change = {0};
#endif

extern void bt_sink_srv_music_a2dp_common_ami_hdr(bt_sink_srv_am_id_t aud_id, 
    bt_sink_srv_am_cb_msg_class_t msg_id, bt_sink_srv_am_cb_sub_msg_t sub_msg, void *param);
audio_src_srv_handle_t *bt_sink_srv_a2dp_alloc_pseudo_handle(void);
static int32_t bt_sink_srv_a2dp_handle_start_streaming_ind(bt_a2dp_start_streaming_ind_t *start_ind, bool is_cnf);
static void bt_sink_srv_a2dp_handle_media_data_received_ind(uint32_t gap_handle, bt_sink_srv_music_data_info_t *media_info);
static void bt_sink_srv_a2dp_set_sta(bt_sink_srv_music_set_sta_param_t *sta_parameters, bool need_timer);
#ifdef AIR_A2DP_SYNC_STOP_ENABLE
extern void bt_sink_srv_music_sync_stop_done_handler(hal_audio_event_t event, void *data);
#endif


#define BT_SINK_SRV_AWS_MCE_SRV_MODE_INVALID 0xff
bt_status_t bt_sink_srv_a2dp_get_params_by_mode(bt_aws_mce_srv_mode_t mode, bt_a2dp_init_params_t *param)
{
    int32_t ret = BT_STATUS_FAIL;
    uint32_t num = 0;

    if (param) {
        bt_codec_sbc_t sbc_cap = {
            8,  // min_bit_pool
            75,  // max_bit_pool
            0xf, // block_len: all
            0xf, // subband_num: all
            0x3, // both snr/loudness
            0xf, // sample_rate: all
            0xf  // channel_mode: all
        };
        /* init sink sep */
        bt_sink_srv_a2dp_sbc_config_parameter_t sbc_config = {
            .min_bitpool = 8,
            .max_bitpool = 75,
        };
        uint32_t read_size = sizeof(sbc_config);
        nvkey_status_t nvdm_ret = nvkey_read_data(NVID_BT_HOST_MUSIC_SBC_CONFIG, (uint8_t *)(&sbc_config), &read_size);
        (void)nvdm_ret;
        sbc_cap.min_bit_pool = sbc_config.min_bitpool;
        sbc_cap.max_bit_pool = sbc_config.max_bitpool;
        bt_sink_srv_report_id("[sink][music][a2dp] sbc,max_bit_rate:0x%02x, min_bit_rate:0x%02x, read_size:%d, nvdm_ret:%d", 4, sbc_config.max_bitpool, sbc_config.min_bitpool,
                              read_size, nvdm_ret);
        BT_A2DP_MAKE_SBC_CODEC(g_bt_sink_srv_a2dp_codec_list + num, BT_A2DP_SINK,
                               sbc_cap.min_bit_pool, sbc_cap.max_bit_pool,
                               sbc_cap.block_length, sbc_cap.subband_num,
                               sbc_cap.alloc_method, sbc_cap.sample_rate,
                               sbc_cap.channel_mode);
#ifdef MTK_BT_SPEAKER_ENABLE
        /* Limit max bitpool to 35 due to controller bandwidth limitation under SPK broadcast mode */
        if (BT_SINK_SRV_MUSIC_MODE_BROADCAST == mode || BT_SINK_SRV_MUSIC_MODE_BROADCAST == bt_sink_srv_music_get_spk_mode()) {
            uint32_t max_bp = BT_SINK_SRV_A2DP_SBC_MAX_BP_IN_SPEAKER_MODE;
            bt_a2dp_codec_capability_t *sbc_codec = &g_bt_sink_srv_a2dp_codec_list[num];
            sbc_codec->codec.sbc.max_bitpool = (max_bp & 0xFF);
            bt_sink_srv_report_id("[sink][music][a2dp] switch to broadcast mode:0x%02x, change max bitpool:0x%02x", 2, mode, sbc_codec->codec.sbc.max_bitpool);
        }
#endif
        num++;
#ifdef BT_SINK_SRV_A2DP_AAC_SUPPORT
        if (g_bt_sink_srv_aac_open) {
            bt_codec_aac_t aac_cap = {
                true,    /*VBR         */
                false,   /*DRC         */
                0x40,    /*Object type */
                0x0c,    /*Channels    */
                0x0018,  /*Sample_rate */
                0x60000  /*bit_rate, 384 Kbps */
            };
            bt_sink_srv_a2dp_aac_config_parameter_t aac_config = {
                .aac_type = 0,
                .bit_rate = 0x60000,
            };
            read_size = sizeof(aac_config);
            nvdm_ret = nvkey_read_data(NVID_BT_HOST_MUSIC_AAC_CONFIG, (uint8_t *)(&aac_config), &read_size);
            aac_cap.bit_rate = aac_config.bit_rate;
            bt_sink_srv_report_id("[sink][music][a2dp] aac,bit_rate:0x%08x, type:0x%02x, read_size:%d, nvdm_ret:%d, object_type:0x%02x", 5, aac_config.bit_rate, aac_config.aac_type,
                                  read_size, nvdm_ret, aac_cap.object_type);
            if (aac_config.aac_type == BT_SINK_SRV_A2DP_MPEG_2_LC) {
                aac_cap.object_type = 0x40;
            } else if (aac_config.aac_type == BT_SINK_SRV_A2DP_MPEG_2_AND_4_LC) {
                aac_cap.object_type = 0x60;
            }
            BT_A2DP_MAKE_AAC_CODEC(g_bt_sink_srv_a2dp_codec_list + num, BT_A2DP_SINK,
                                   aac_cap.vbr, aac_cap.drc, aac_cap.object_type, aac_cap.channels,
                                   aac_cap.sample_rate, aac_cap.bit_rate);
            num++;
        }
#endif


#ifdef AIR_BT_A2DP_VENDOR_ENABLE
        if (g_bt_sink_srv_vendor_codec_open) {
            bool is_add_vendor_codec = false;
#ifdef MTK_BT_SPEAKER_ENABLE
            if (BT_AWS_MCE_SRV_MODE_NORMAL == mode || BT_AWS_MCE_SRV_MODE_SINGLE == mode) {
                is_add_vendor_codec = true;
            }
            else
#endif
            {
                if (BT_SINK_SRV_AWS_MCE_SRV_MODE_INVALID == mode)
                {
                    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
                    if (role == BT_AWS_MCE_ROLE_NONE
#ifndef MTK_BT_SPEAKER_ENABLE
                        || role == BT_AWS_MCE_ROLE_AGENT || role == BT_AWS_MCE_ROLE_PARTNER
#endif
                       ) {
                        is_add_vendor_codec = true;
                    }
                }
            }
            if (is_add_vendor_codec) {
                BT_A2DP_MAKE_VENDOR_CODEC(g_bt_sink_srv_a2dp_codec_list + num, BT_A2DP_SINK,
                                              0x0000012d, 0x00aa, 0x3c, 0x07);
                num++;
            }
        }
#endif
#ifdef AIR_BT_A2DP_VENDOR_2_ENABLE
        if (g_bt_sink_srv_vendor_codec_open) {
            BT_A2DP_MAKE_VENDOR_CODEC_2(g_bt_sink_srv_a2dp_codec_list + num, BT_A2DP_SINK,
                                        0x0000053a, BT_A2DP_CODEC_LHDC_CODEC_ID, 0x35, 0x06);
            num++;
        }

#endif
#ifdef AIR_BT_A2DP_LC3PLUS_ENABLE
        if (g_bt_sink_srv_vendor_codec_open) {
            uint8_t frame_duration = 0x60;
            uint16_t sample_frequency = 0x0180;
            if(g_bt_sink_srv_a2dp_lc3plus_change.changed_flag) {
                frame_duration = g_bt_sink_srv_a2dp_lc3plus_change.frame_duration;
                sample_frequency = g_bt_sink_srv_a2dp_lc3plus_change.sample_frequency;
            }
            BT_A2DP_MAKE_VENDOR_CODEC_3(g_bt_sink_srv_a2dp_codec_list + num, BT_A2DP_SINK,
                                        0x000008a9, BT_A2DP_CODEC_LC3PLUS_CODEC_ID, frame_duration, sample_frequency);
            num++;
        }
#endif
        param->sink_feature = 0x0F;
        param->source_feature = 0x00;
        param->codec_number = num;
        param->codec_list = g_bt_sink_srv_a2dp_codec_list;
        param->customer_feature_option = 0;
#ifdef AIR_FEATURE_SINK_MHDT_SUPPORT
        param->customer_feature_option |= 0x01;
#endif
#ifdef __BT_SINK_SRV_A2DP_V13_SUPPORT__
        param->sink_delay = BT_SINK_SRV_A2DP_DELAY;
#endif
        ret = BT_STATUS_SUCCESS;

        bt_sink_srv_report_id("[sink][music][a2dp] get_params_by_mode: %d, num:%d, mode:0x%02x", 3, ret, param->codec_number, mode);
    }

    return ret;
}

bool bt_a2dp_validate_vendor_codec(const bt_a2dp_vendor_codec_t *codec, uint32_t length)
{
    bool ret = false;
#ifdef AIR_BT_A2DP_LC3PLUS_ENABLE
    if(codec->vendor_id == A2DP_LC3PLUS_VENDOR_ID && codec->codec_id == A2DP_LC3PLUS_CODEC_ID) {
        ret = bt_a2dp_validate_lc3_plus_vendor_codec(codec, length);
    }
#endif
#ifdef AIR_BT_A2DP_VENDOR_2_ENABLE
    if(codec->vendor_id == A2DP_LHDC_V5_VENDOR_ID && codec->codec_id == A2DP_LHDC_V5_CODEC_ID) {
        ret = bt_a2dp_validate_lhdc_vendor_codec(codec, length);
    }
#endif
    bt_sink_srv_report_id("[sink][music][a2dp] validate vendor codec, ret: %d", 1, ret);
    return ret;
}

bool bt_a2dp_negotiate_vendor_codec(bt_a2dp_vendor_codec_t *result, const bt_a2dp_vendor_codec_t *local, const bt_a2dp_vendor_codec_t *remote)
{
    bool ret = false;
#ifdef AIR_BT_A2DP_LC3PLUS_ENABLE
    if(local->vendor_id == A2DP_LC3PLUS_VENDOR_ID && local->codec_id == A2DP_LC3PLUS_CODEC_ID) {
        ret = bt_a2dp_negotiate_lc3_plus_vendor_codec(result, local, remote);
    }
#endif
#ifdef AIR_BT_A2DP_VENDOR_2_ENABLE
    if(local->vendor_id == A2DP_LHDC_V5_VENDOR_ID && local->codec_id == A2DP_LHDC_V5_CODEC_ID) {
        ret = bt_a2dp_negotiate_lhdc_vendor_codec(result, local, remote);
    }
#endif
    bt_sink_srv_report_id("[sink][music][a2dp] negotiate vendor codec, ret: %d", 1, ret);
    return ret;
}

#ifdef MTK_BT_SPEAKER_ENABLE
bt_status_t bt_sink_srv_a2dp_mode_changed_handle_callback(bt_cm_event_t event_id, void *params, uint32_t params_len)
{
    int32_t ret = BT_STATUS_FAIL;
    if (BT_AWS_MCE_SRV_EVENT_MODE_CHANGED_IND == event_id) {
        bt_aws_mce_srv_mode_changed_ind_t *mode_cnf = (bt_aws_mce_srv_mode_changed_ind_t *)params;
        bt_a2dp_init_params_t codec_param = {0};
        ret = bt_sink_srv_a2dp_get_params_by_mode(mode_cnf->mode, &codec_param);
        if (BT_STATUS_SUCCESS == ret) {
            bt_a2dp_update_sep(&codec_param);
        }
        if (BT_AWS_MCE_SRV_MODE_BROADCAST == mode_cnf->mode) {
            bt_a2dp_set_mtu_size(672);
            bt_sink_srv_music_set_sink_latency(BT_SINK_SRV_A2DP_SPEAKER_MODE_DEFAULT_SINK_LATENCY);
        } else if (BT_AWS_MCE_SRV_MODE_DOUBLE == mode_cnf->mode
                    || BT_AWS_MCE_SRV_MODE_NORMAL == mode_cnf->mode
                    || BT_AWS_MCE_SRV_MODE_SINGLE == mode_cnf->mode) {
            bt_a2dp_set_mtu_size(895);
            bt_sink_srv_music_set_sink_latency(BT_SINK_SRV_A2DP_DEFAULT_SINK_LATENCY);
        }
        bt_sink_srv_report_id("[sink][music][a2dp] mode changed to 0x%02x, ret: %d", 2, mode_cnf->mode, ret);
    }

    return ret;
}
#endif /*MTK_BT_SPEAKER_ENABLE*/
bt_status_t bt_sink_srv_a2dp_get_init_params(bt_a2dp_init_params_t *param)
{
    int32_t ret = BT_STATUS_FAIL;

    bt_utils_assert(param);
    ret = bt_sink_srv_a2dp_get_params_by_mode(BT_SINK_SRV_AWS_MCE_SRV_MODE_INVALID, param);
    g_bt_sink_srv_a2dp_init_complete = true;
    
    bt_sink_srv_report_id("[sink][music][a2dp] get_init_params: %d, param:%d\n", 2, ret, param);
    return ret;
}

#ifdef BT_SINK_SRV_A2DP_AAC_SUPPORT
void bt_sink_srv_a2dp_enable_aac(bool enable_flag)
{
    bt_sink_srv_report_id("[sink][music][a2dp] a2dp_enable_aac: %d=>%d", 2, g_bt_sink_srv_aac_open, enable_flag);

    if (enable_flag != g_bt_sink_srv_aac_open) {
        g_bt_sink_srv_aac_open = enable_flag;
        if (g_bt_sink_srv_a2dp_init_complete) {
            bt_a2dp_init_params_t init_param = {0};
            bt_sink_srv_a2dp_get_init_params(&init_param);
            bt_a2dp_update_sep(&init_param);
        }
    }
}
#endif

#ifdef AIR_BT_A2DP_VENDOR_CODEC_SUPPORT
void bt_sink_srv_a2dp_enable_vendor_codec(bool enable_flag)
{
    bt_sink_srv_report_id("[sink][music][a2dp] enable_vendor_codec--%d=>%d", 2, g_bt_sink_srv_vendor_codec_open, enable_flag);

    if (enable_flag != g_bt_sink_srv_vendor_codec_open) {
#ifdef __BT_AWS_MCE_A2DP_SUPPORT__
        if (BT_AWS_MCE_ROLE_AGENT == bt_connection_manager_device_local_info_get_aws_role()) {
            bt_sink_srv_aws_mce_vendor_codec_config_sync(enable_flag);
        }
#endif
        g_bt_sink_srv_vendor_codec_open = enable_flag;
        if (g_bt_sink_srv_a2dp_init_complete) {
            bt_a2dp_init_params_t init_param = {0};
            bt_sink_srv_a2dp_get_init_params(&init_param);
            bt_a2dp_update_sep(&init_param);
        }
    }
}

bool bt_sink_srv_a2dp_get_vendor_codec_config(void)
{
    return g_bt_sink_srv_vendor_codec_open;
}
#endif

#ifdef AIR_BT_A2DP_LC3PLUS_ENABLE
void bt_sink_srv_a2dp_change_lc3plus_param(uint16_t sample_frequency, uint8_t frame_duration)
{
    bt_sink_srv_report_id("[sink][music][a2dp] change_lc3plus_param, sample_frequency:0x%x, frame_duration: 0x%x", 2, 
        sample_frequency, frame_duration);
    g_bt_sink_srv_a2dp_lc3plus_change.changed_flag = true;
    g_bt_sink_srv_a2dp_lc3plus_change.sample_frequency = sample_frequency;
    g_bt_sink_srv_a2dp_lc3plus_change.frame_duration = frame_duration;

    if (g_bt_sink_srv_a2dp_init_complete) {
        bt_a2dp_init_params_t init_param = {0};
        bt_sink_srv_a2dp_get_init_params(&init_param);
        bt_a2dp_update_sep(&init_param);
    }
}
#endif

static void bt_sink_srv_a2dp_init()
{
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();

    ctx->a2dp_aid = bt_sink_srv_ami_audio_open(AUD_MIDDLE, bt_sink_srv_music_a2dp_common_ami_hdr);
    avm_direct_clear_drift_parameters();
#ifdef MTK_BT_SPEAKER_ENABLE
    if (BT_SINK_SRV_MUSIC_MODE_BROADCAST == bt_sink_srv_music_get_spk_mode()) {
#ifdef MTK_BT_SPEAKER_FEC_ENABLE
        speaker_fec_init();
#endif
    }
#endif
    bt_sink_srv_report_id("[sink][music][a2dp] init-a2dp_aid: %d", 1, ctx->a2dp_aid);
}

#ifdef MTK_BT_SPEAKER_ENABLE
void bt_sink_srv_a2dp_update_base_timer(uint32_t timer_id, uint32_t data)
{
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_sink_srv_report("[sink][music][a2dp] bt_sink_srv_a2dp_update_base_timer, update_base_count:%d", 1, ctx->update_base_count);
    if (ctx->run_dev) {
        ctx->update_base_count++;
        bt_sink_srv_aws_mce_a2dp_send_eir(BT_SINK_SRV_AWS_MCE_A2DP_EVT_UPDATE_BASE, (void *)(&(ctx->run_dev->a2dp_hd)));
        if (ctx->update_base_count < BT_SINK_SRV_MUSIC_MAX_UPDATE_BASE_COUNT)
            bt_timer_ext_start(BT_SINK_SRV_A2DP_SEND_BASE_IND_TIMER_ID, 0,
                               BT_SINK_SRV_A2DP_SEND_BASE_IND_TIMER_DUR, bt_sink_srv_a2dp_update_base_timer);
    }
}
#endif

void bt_sink_srv_a2dp_ami_hdr(bt_sink_srv_am_id_t aud_id, bt_sink_srv_am_cb_msg_class_t msg_id, bt_sink_srv_am_cb_sub_msg_t sub_msg, void *param)
{
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_sink_srv_music_device_t *run_dev = ctx->run_dev;

    if ((NULL == run_dev) || (ctx->a2dp_aid != aud_id)) {
        return;
    }

    bt_sink_srv_report_id("[sink][music][a2dp] ami_hdr--msg_id:%d,sub_msg:%d,run_dev:0x%x-0x%x-0x%x-0x%x-0x%x-0x%x",
        8, msg_id, sub_msg, run_dev, run_dev->flag, run_dev->handle->substate, 
        run_dev->codec.type,run_dev->codec.delay_report, run_dev->codec.sec_type);

    switch (msg_id) {
        case AUD_SINK_OPEN_CODEC: {
            if (run_dev->flag & BT_SINK_SRV_MUSIC_FLAG_WAIT_AMI_OPEN_CODEC) {
                /* Save codec handle */
                bt_sink_srv_memcpy(&(run_dev->med_handle), param, sizeof(bt_sink_srv_am_media_handle_t));
                BT_SINK_SRV_REMOVE_FLAG(run_dev->flag, BT_SINK_SRV_MUSIC_FLAG_WAIT_AMI_OPEN_CODEC);
                BT_SINK_SRV_SET_FLAG(run_dev->op, BT_SINK_SRV_MUSIC_OP_CODEC_OPEN);
                if (run_dev->flag & BT_SINK_SRV_MUSIC_WAIT_SET_VOLUME) {
                    bt_sink_srv_music_set_am_volume(ctx->vol_lev);
                    BT_SINK_SRV_REMOVE_FLAG(run_dev->flag, BT_SINK_SRV_MUSIC_WAIT_SET_VOLUME);
                }
                if (BT_SINK_SRV_MUSIC_TRANSIENT_STATE_CLEAR_CODEC == run_dev->handle->substate 
                    || (run_dev->flag & BT_SINK_SRV_MUSIC_FLAG_RHO_HAPPEN_DURING_STARTING_PLAY)) {
                    if (!(run_dev->conn_bit & BT_SINK_SRV_MUSIC_A2DP_CONN_BIT)) {
                        bt_sink_srv_cm_profile_status_notify(&run_dev->dev_addr, BT_SINK_SRV_PROFILE_A2DP_SINK, BT_SINK_SRV_PROFILE_CONNECTION_STATE_DISCONNECTED, BT_STATUS_SUCCESS);
                    }
                    bt_sink_srv_music_state_machine_handle(run_dev, BT_SINK_SRV_MUSIC_EVT_CODEC_OPEN, NULL);
                    if ((run_dev->flag & BT_SINK_SRV_MUSIC_FLAG_RHO_HAPPEN_DURING_STARTING_PLAY)
                        || (run_dev->op & BT_SINK_SRV_MUSIC_WAITING_REINITIAL_SYNC)) {
                        bt_sink_srv_a2dp_reinitial_sync();
                        BT_SINK_SRV_REMOVE_FLAG(run_dev->op, BT_SINK_SRV_MUSIC_WAITING_REINITIAL_SYNC);
                        BT_SINK_SRV_REMOVE_FLAG(run_dev->flag, BT_SINK_SRV_MUSIC_FLAG_RHO_HAPPEN_DURING_STARTING_PLAY);
                    }
                    break;
                }
                uint32_t buffer_size = SHARE_BUFFER_BT_AUDIO_DL_SIZE;
#ifdef MTK_BT_A2DP_VENDOR_CODEC_BC_ENABLE
                if (run_dev->codec.type == BT_A2DP_CODEC_VENDOR) {
                    buffer_size = SHARE_BUFFER_VENDOR_CODEC_SIZE;
                }
#endif
                bt_sink_srv_music_update_audio_buffer(buffer_size);
                /*set n pkt should before rsp to sp*/
#ifdef __BT_SINK_SRV_A2DP_AVM_DIRECT_SUPPORT__
#if defined (AIR_LE_AUDIO_ENABLE) && defined (AIR_LE_AUDIO_CIS_ENABLE) && !defined(AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE)
                bt_handle_t le_handle = bt_sink_srv_cap_stream_get_ble_link_with_cis_established();
                if (le_handle != BT_HANDLE_INVALID) {
                    bt_sink_srv_cap_stream_release_autonomously(le_handle, 0xFF, false, 0);
                }
#endif
                uint32_t gap_hd = bt_sink_srv_cm_get_gap_handle(&(run_dev->dev_addr));
                bt_utils_assert(gap_hd);
                bt_sink_srv_music_set_music_enable(gap_hd, BT_AVM_ROLE_AWS_MCE_AGENT, true);

                uint32_t n_value = BT_SINK_SRV_A2DP_N_PACKET_NOTIFY;
                bool ret = bt_iot_device_white_list_check_iot_case((bt_bd_addr_t *)&run_dev->dev_addr, BT_IOT_MUSIC_NOTIFY_VENDOR_CODEC_BY_N_PACKET);
                bt_sink_srv_music_mode_t music_mode = bt_sink_srv_music_get_mode(&run_dev->dev_addr);
                if (run_dev->codec.type == BT_A2DP_CODEC_VENDOR || ret || music_mode == BT_SINK_SRV_MUSIC_GAME_MODE) {
                    n_value = BT_SINK_SRV_A2DP_N_PACKET_NOTIFY_VENDOR_CODEC;
#ifdef AIR_BT_A2DP_VENDOR_2_ENABLE
                } else if (run_dev->codec.codec.vendor.codec_id == 0x4c33 || (run_dev->codec.codec.vendor.codec_id == BT_A2DP_CODEC_LHDC_CODEC_ID)) {
                    n_value =  BT_SINK_SRV_A2DP_VENDOR_2_N_PACKET_NOTIFY;
#endif
                }
                if (run_dev->codec.sec_type) {
                    n_value |= BT_SINK_SRV_MUSIC_CONTENT_PROTECTION_MASK;
                }
#ifdef AIR_A2DP_REINIT_V2_ENABLE
                if (run_dev->codec.type == BT_A2DP_CODEC_SBC
                    && bt_iot_device_white_list_check_iot_case((bt_bd_addr_t *)&run_dev->dev_addr, BT_IOT_MUSIC_IS_WALKMAN_SET_RATIO_AND_PCDC_OBSERVATION)) {
                    n_value |= BT_SINK_SRV_A2DP_SHORT_PCDC_OBSERVATION;
                    bt_media_handle_t *med_hd = run_dev->med_handle.med_hd;
                    med_hd->set_ts_ratio(med_hd, 0xfffffffe);
                }
#endif
                bt_avm_set_a2dp_notify_condition(gap_hd, n_value);

#endif
                    if (BT_SINK_SRV_MUSIC_TRANSIENT_STATE_CLEAR_CODEC != run_dev->handle->substate) {
#ifdef __BT_SINK_SRV_A2DP_V13_SUPPORT__
                        if (run_dev->codec.delay_report) {
                            if (!(run_dev->flag & BT_SINK_SRV_MUSIC_FLAG_REINITIAL_SYNC) || (run_dev->flag & BT_SINK_SRV_MUSIC_FLAG_ALC_REINITIAL_SYNC)) {
                                BT_SINK_SRV_REMOVE_FLAG(run_dev->flag, BT_SINK_SRV_MUSIC_FLAG_ALC_REINITIAL_SYNC);
                                uint32_t delay_report = avm_direct_get_sink_latency();
                                delay_report = delay_report / 100;
                            bt_a2dp_set_delay_report(run_dev->a2dp_hd, delay_report);
                            }
                        }

                        if (run_dev->codec.sec_type) {
                            bt_sink_srv_am_media_handle_t *med_handle = &(run_dev->med_handle);
                            med_handle->med_hd->set_content_protection(med_handle->med_hd, true);
                        }
#endif
                    }

                    bt_sink_srv_music_state_machine_handle(run_dev, BT_SINK_SRV_MUSIC_EVT_CODEC_OPEN, NULL);
#ifdef MTK_BT_A2DP_VENDOR_CODEC_BC_ENABLE
                    if (run_dev->codec.type == BT_A2DP_CODEC_VENDOR) {
                        bt_sink_srv_music_drv_play(ctx->run_dev);
                        bt_clock_t pta = {0};
                        bt_avm_set_audio_tracking_time(gap_hd, BT_AVM_TYPE_A2DP, &pta);
                        bt_sink_srv_music_state_machine_handle(ctx->run_dev, BT_SINK_SRV_MUSIC_EVT_PLAYING, NULL);
                    }
#endif
            }
            break;
        }
        case AUD_A2DP_PROC_IND: {
            if (run_dev->handle->substate != BT_SINK_SRV_MUSIC_TRANSIENT_STATE_CLEAR_CODEC) {
                switch (sub_msg) {
                    case AUD_A2DP_DL_REINIT_REQUEST: {
                        bt_sink_srv_music_dsp_parameter_t *dsp_parameter = (bt_sink_srv_music_dsp_parameter_t *)param;
                        if (*dsp_parameter == BT_SINK_SRV_MUSIC_REINIT_PARTNER_LATER_JOIN) {
                            BT_SINK_SRV_SET_FLAG(run_dev->op, BT_SINK_SRV_MUSIC_REINIT_ON_PARTNER_LATER_JOIN_FLAG);
                            break;
                        }
                        if (!ctx->rho_flag) {
                            bt_sink_srv_a2dp_reinitial_sync();
                        }
                        bt_sink_srv_report_id("[sink][music][a2dp] AUD_A2DP_DL_REINIT_REQUEST-dsp_parameter:0x%x", 1,
                                                  *dsp_parameter);
                        break;
                    }

                    case AUD_A2DP_ACTIVE_LATENCY_REQUEST: {
                        bool vendor_codec = (run_dev->codec.type == BT_A2DP_CODEC_VENDOR);
                        uint32_t latency_val = bt_sink_srv_get_latency(&run_dev->dev_addr, vendor_codec, true, false);
                        uint32_t original_latency = bt_sink_srv_music_get_sink_latency();
                        if (BT_SINK_SRV_MUSIC_MODE_BROADCAST == bt_sink_srv_music_get_spk_mode()) {
                            latency_val = bt_sink_srv_get_latency(&run_dev->dev_addr, vendor_codec, true, true);
                        }
                        if (!ctx->rho_flag) {
                            if (bt_sink_srv_music_get_ALC_enable()) {
                                if (bt_iot_device_white_list_check_iot_case((bt_bd_addr_t *)&run_dev->dev_addr, BT_IOT_MUSIC_SET_LATENCY_TO_ORIGINAL)) {
                                    latency_val = original_latency;
                                } else if(latency_val < BT_SINK_SRV_A2DP_SPECIAL_DEVICE_LATENCY_170MS
                                    && bt_iot_device_white_list_check_iot_case((bt_bd_addr_t *)&run_dev->dev_addr, BT_IOT_MUSIC_SET_LATENCY_TO_170)) {
                                    latency_val = BT_SINK_SRV_A2DP_SPECIAL_DEVICE_LATENCY_170MS;
                                } else if(bt_iot_device_white_list_check_iot_case((bt_bd_addr_t *)&ctx->run_dev->dev_addr, BT_IOT_MUSIC_SET_LATENCY_TO_250)) {
                                    latency_val = BT_SINK_SRV_A2DP_SPECIAL_DEVICE_LATENCY;
                                }
                            }
                            bt_sink_srv_music_set_sink_latency(latency_val);
                            bt_sink_srv_a2dp_reinitial_sync();
                            BT_SINK_SRV_SET_FLAG(run_dev->flag, BT_SINK_SRV_MUSIC_FLAG_ALC_REINITIAL_SYNC);
                        }
                        bt_sink_srv_report_id("[sink][music][a2dp] AUD_A2DP_ACTIVE_LATENCY_REQUEST, new_latency: %d, ori_latency: %d",
                                              2, latency_val, original_latency);
                        break;
                    }

                    default:
                        break;
                }
            }
            break;
        }
        default:
            break;
    }
}

void bt_sink_srv_a2dp_initial_avrcp_timer(uint32_t timer_id, uint32_t data)
{
    bt_sink_srv_music_device_t *dev = (bt_sink_srv_music_device_t *)data;
    bt_utils_assert(dev && "initial_avrcp_timer");
    if (dev->avrcp_hd == BT_SINK_SRV_MUSIC_INVALID_HD &&
        !(dev->conn_bit & BT_SINK_SRV_MUSIC_AVRCP_CONN_BIT)) {
        uint32_t hd = BT_SINK_SRV_MUSIC_INVALID_HD;
        bt_sink_srv_init_role(BT_AVRCP_ROLE_CT);//[TD-PLAYER]
        bt_status_t ret = bt_avrcp_connect(&hd, (const bt_bd_addr_t *)(&(dev->dev_addr)));
        if (BT_STATUS_SUCCESS == ret) {
            BT_SINK_SRV_SET_FLAG(dev->flag, BT_SINK_SRV_MUSIC_FLAG_WAIT_AVRCP_CONN_RESULT);
            dev->avrcp_hd = hd;
            dev->avrcp_role = BT_AVRCP_ROLE_CT;//[TD-PLAYER]
        } else {
            bt_timer_ext_start(BT_SINK_SRV_AVRCP_CONNECTION_TIMER_ID, (uint32_t)dev,
                               BT_SINK_SRV_AVRCP_CONNECTION_TIMER_DUR, bt_sink_srv_a2dp_initial_avrcp_timer);
        }
        bt_sink_srv_report_id("[sink][music][a2dp] connect_cnf-ret: %d, avrcp_hd: 0x%x", 2, ret, dev->avrcp_hd);
    }
}

void bt_sink_srv_a2dp_disconnect_avrcp_timer(uint32_t timer_id, uint32_t data)
{
    bt_sink_srv_music_device_t *dev = (bt_sink_srv_music_device_t *)data;
    if ((dev->conn_bit & BT_SINK_SRV_MUSIC_AVRCP_CONN_BIT) 
        && !(dev->conn_bit & BT_SINK_SRV_MUSIC_A2DP_CONN_BIT)) {
        bt_status_t ret = bt_avrcp_disconnect(dev->avrcp_hd);
        (void)ret;
        bt_sink_srv_report_id("[sink][music][a2dp] disconnect_avrcp_timer ret:0x%08x", 1, ret);
    }
}

#ifdef MTK_BT_SPEAKER_ENABLE
void bt_sink_srv_a2dp_send_play_ind_timer(uint32_t timer_id, uint32_t data)
{
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_sink_srv_report_id("[sink][music][a2dp] send_play_ind_timer-mode:0x%x, run_dev:0x%x", 2, bt_sink_srv_music_get_spk_mode(),ctx->run_dev);
    if (BT_SINK_SRV_MUSIC_MODE_BROADCAST == bt_sink_srv_music_get_spk_mode()
        && (ctx->run_dev)
        && (ctx->run_dev->conn_bit & BT_SINK_SRV_MUSIC_A2DP_CONN_BIT)) {
        bt_sink_srv_aws_mce_a2dp_send_eir(BT_SINK_SRV_AWS_MCE_A2DP_EVT_PLAY_IND, (void *)(&(ctx->run_dev->a2dp_hd)));
        bt_timer_ext_start(BT_SINK_SRV_A2DP_SEND_PLAY_IND_TIMER_ID, 0,
                           BT_SINK_SRV_A2DP_SEND_PLAY_IND_TIMER_DUR, bt_sink_srv_a2dp_send_play_ind_timer);
    }
}
#endif

int32_t bt_sink_srv_a2dp_handle_connect_cnf(bt_a2dp_connect_cnf_t *conn_cnf)
{
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_A2DP_HD, (void *)(&(conn_cnf->handle)));
    int32_t ret = 0;

    if (!dev) {
        return ret;
    }
    dev->a2dp_status = BT_SINK_SRV_A2DP_STATUS_SUSPEND;
    if (conn_cnf->status == BT_STATUS_SUCCESS) {
        /* Save codec capability */
        memcpy(&(dev->codec), (conn_cnf->codec_cap), sizeof(bt_a2dp_codec_capability_t));
        BT_SINK_SRV_SET_FLAG(dev->conn_bit, BT_SINK_SRV_MUSIC_A2DP_CONN_BIT);
        bool need_delay_connect_avrcp = true;
        if (BT_SINK_SRV_MUSIC_FLAG_DEV_TO_CON_AVRCP & dev->flag) {
            if (dev->avrcp_hd == BT_SINK_SRV_MUSIC_INVALID_HD) {
                bt_sink_srv_init_role(BT_AVRCP_ROLE_CT);//[TD-PLAYER]
                ret = bt_avrcp_connect(&(dev->avrcp_hd), (const bt_bd_addr_t *)(&(dev->dev_addr)));
                if (BT_STATUS_SUCCESS == ret) {
                    BT_SINK_SRV_SET_FLAG(dev->flag, BT_SINK_SRV_MUSIC_FLAG_WAIT_AVRCP_CONN_RESULT);
                    dev->avrcp_role = BT_AVRCP_ROLE_CT;//[TD-PLAYER]
                    need_delay_connect_avrcp = false;
                }
            }
            BT_SINK_SRV_REMOVE_FLAG(dev->flag, BT_SINK_SRV_MUSIC_FLAG_DEV_TO_CON_AVRCP);
        }
        if (need_delay_connect_avrcp) {
            if ((!(dev->conn_bit & BT_SINK_SRV_MUSIC_AVRCP_CONN_BIT))
                && (0 == bt_sink_srv_music_get_context()->rho_flag)) {
                bt_timer_ext_start(BT_SINK_SRV_AVRCP_CONNECTION_TIMER_ID, (uint32_t)dev,
                                   BT_SINK_SRV_AVRCP_CONNECTION_TIMER_DUR, bt_sink_srv_a2dp_initial_avrcp_timer);
            }
        }

        bt_sink_srv_cm_profile_status_notify(&(dev->dev_addr), BT_SINK_SRV_PROFILE_A2DP_SINK, BT_SINK_SRV_PROFILE_CONNECTION_STATE_CONNECTED, conn_cnf->status);
        bt_sink_srv_music_state_machine_handle(dev, BT_SINK_SRV_MUSIC_EVT_READY, NULL);
        bt_sink_srv_report_id("[sink][music][a2dp] connect_cnf-con_bit:0x%04x,avrcp_hd:0x%x,a2dp_hd:0x%x,flag:0x%08x,codec:0x%x-0x%x, ret:0x%x", 7,
                      dev->conn_bit,dev->avrcp_hd,dev->a2dp_hd,dev->flag,dev->codec.type,conn_cnf->codec_cap->type, ret);
    } else {
        /* State machine handle */
        bt_sink_srv_music_state_machine_handle(dev, BT_A2DP_CONNECT_CNF, conn_cnf);
        bt_sink_srv_cm_profile_status_notify(&(dev->dev_addr), BT_SINK_SRV_PROFILE_A2DP_SINK, BT_SINK_SRV_PROFILE_CONNECTION_STATE_DISCONNECTED, conn_cnf->status);
        BT_SINK_SRV_REMOVE_FLAG(dev->conn_bit, BT_SINK_SRV_MUSIC_A2DP_CONN_BIT);
        BT_SINK_SRV_REMOVE_FLAG(dev->flag, BT_SINK_SRV_MUSIC_INITIAL_A2DP_BY_DEVICE|BT_SINK_SRV_MUSIC_FLAG_DEV_TO_CON_AVRCP);
        if (!(dev->conn_bit) && dev->avrcp_hd == BT_SINK_SRV_MUSIC_INVALID_HD) {
            bt_sink_srv_a2dp_free_pseudo_handle(dev->handle);
            dev->handle = NULL;
            bt_sink_srv_music_reset_device(dev);
        } else {
            dev->a2dp_hd = BT_SINK_SRV_MUSIC_INVALID_HD;
        }
    }
    return ret;
}


int32_t bt_sink_srv_a2dp_handle_connect_ind(bt_a2dp_connect_ind_t *conn_ind)
{
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_ADDR_AVRCP, (void *)(conn_ind->address));
    int32_t ret = 0;
    uint8_t *addr = (uint8_t *)conn_ind->address;
    (void)addr;
    /* A2dp connected */
    if (NULL == dev) {
        dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_ADDR_A2DP, (void *)(conn_ind->address));
        if (NULL == dev) {
        dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_UNUSED, AUDIO_SRC_SRV_PSEUDO_DEVICE_A2DP);
            bt_utils_assert(dev && "Error: a2dp dev NULL");
            dev->handle = bt_sink_srv_a2dp_alloc_pseudo_handle();
            dev->handle->type = AUDIO_SRC_SRV_PSEUDO_DEVICE_A2DP;
            dev->handle->dev_id = bt_sink_srv_music_convert_btaddr_to_devid(conn_ind->address);
            dev->handle->priority = AUDIO_SRC_SRV_PRIORITY_NORMAL;
        }
    }

    /* Init pse handle */
    dev->a2dp_hd = conn_ind->handle;
    dev->role = conn_ind->role;
    memcpy(&(dev->dev_addr), conn_ind->address, sizeof(bt_bd_addr_t));
    ret = bt_a2dp_connect_response(conn_ind->handle, true);
    bt_sink_srv_music_state_machine_handle(dev, BT_A2DP_CONNECT_IND, conn_ind);

    bt_sink_srv_report_id("[sink][music][a2dp] connect_ind-a2dp_hd: 0x%x, dev:0x%x, addr--0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x",
        8, dev->a2dp_hd, dev, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
    return ret;
}

static int32_t bt_sink_srv_a2dp_handle_disconnect_handler(uint32_t handle, uint32_t evt_id, bt_status_t reason)
{
    bt_sink_srv_music_device_t *a2dp_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_A2DP_HD, (void *)(&handle));
    int32_t ret = 0;

    bt_timer_ext_stop(BT_SINK_SRV_AVRCP_CONNECTION_TIMER_ID);
    if (a2dp_dev && a2dp_dev->handle) {
        bt_sink_srv_report_id("[sink][music][a2dp] disconnect_handler-hd: 0x%x, a2dp_dev:0x%x, flag:0x%x, op:0x%x, conn_bit:0x%x, substate:0x%x",
            6, handle, a2dp_dev, a2dp_dev->flag, a2dp_dev->op, a2dp_dev->conn_bit, a2dp_dev->handle->substate);
        /* force set a2dp suspend when disconnect */
        a2dp_dev->a2dp_status = BT_SINK_SRV_A2DP_STATUS_SUSPEND;
        audio_src_srv_del_waiting_list(a2dp_dev->handle);
        bt_sink_srv_set_must_play_tone_flag(&a2dp_dev->dev_addr, BT_SINK_SRV_INIT_NOTIFICATION_VOICE, false);
        BT_SINK_SRV_REMOVE_FLAG(a2dp_dev->conn_bit, BT_SINK_SRV_MUSIC_A2DP_CONN_BIT);
        BT_SINK_SRV_REMOVE_FLAG(a2dp_dev->flag, BT_SINK_SRV_MUSIC_FLAG_WAIT_LIST_SINK_PLAY
                                              | BT_SINK_SRV_MUSIC_STOP_MUSIC_PENDING
                                              | BT_SINK_SRV_MUSIC_INITIAL_A2DP_BY_DEVICE
                                              | BT_SINK_SRV_MUSIC_FLAG_DEV_TO_CON_AVRCP
                                              | BT_SINK_SRV_MUSIC_NEED_TO_RESPONSE_A2DP_START
                                              | BT_SINK_SRV_MUSIC_FLAG_WAITING_START
                                              | BT_SINK_SRV_MUSIC_FLAG_REINITIAL_SYNC);

        a2dp_dev->a2dp_hd = BT_SINK_SRV_MUSIC_INVALID_HD;

        if (BT_SINK_SRV_MUSIC_TRANSIENT_STATE_CLEAR_CODEC != a2dp_dev->handle->substate) {
            bt_sink_srv_cm_profile_status_notify(&(a2dp_dev->dev_addr), BT_SINK_SRV_PROFILE_A2DP_SINK, BT_SINK_SRV_PROFILE_CONNECTION_STATE_DISCONNECTED, reason);
        }

        /* Music state machine handle first */
        bt_sink_srv_music_state_machine_handle(a2dp_dev, evt_id, NULL);
        if (BT_A2DP_DISCONNECT_IND == evt_id) {
            if (a2dp_dev->conn_bit & BT_SINK_SRV_MUSIC_AVRCP_CONN_BIT) {
                if (a2dp_dev->avrcp_flag & BT_SINK_SRV_INIT_AVRCP_BY_REMOTE_DEVICE) {
                    bt_timer_ext_start(BT_SINK_SRV_AVRCP_DISCONNECT_TIMER_ID, (uint32_t)a2dp_dev,
                               BT_SINK_SRV_AVRCP_DISCONNECT_TIMER_DUR, bt_sink_srv_a2dp_disconnect_avrcp_timer);
                } else {
                    ret = bt_avrcp_disconnect(a2dp_dev->avrcp_hd);
                }
            } else if ((a2dp_dev->flag & BT_SINK_SRV_MUSIC_FLAG_WAIT_AVRCP_CONN_RESULT)) {
                BT_SINK_SRV_SET_FLAG(a2dp_dev->op, BT_SINK_SRV_MUSIC_INIT_DISCONNNECT_AVRCP_TIMER_FLAG);
            }
        }

        /* Deinit pse handle */
        if (a2dp_dev->handle && !(a2dp_dev->conn_bit) &&
            BT_SINK_SRV_MUSIC_TRANSIENT_STATE_CLEAR_CODEC != a2dp_dev->handle->substate
            && a2dp_dev->avrcp_hd == BT_SINK_SRV_MUSIC_INVALID_HD) {
            bt_sink_srv_a2dp_free_pseudo_handle(a2dp_dev->handle);
            a2dp_dev->handle = NULL;
            bt_sink_srv_music_reset_device(a2dp_dev);
        }
    }

    return ret;
}

static int32_t bt_sink_srv_a2dp_handle_start_streaming_cnf(bt_a2dp_start_streaming_cnf_t *start_cnf)
{
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_A2DP_HD, (void *)(&(start_cnf->handle)));
    if (dev && start_cnf->status == BT_STATUS_SUCCESS) {
        bt_a2dp_start_streaming_ind_t start_ind = {0};
        start_ind.handle = start_cnf->handle;
        start_ind.codec_cap = start_cnf->codec_cap;
        bt_sink_srv_a2dp_handle_start_streaming_ind(&start_ind, true);
    }
    bt_sink_srv_report_id("[sink][music][a2dp] start_cnf, dev:0x%08x, status:0x%08x", 2, dev, start_cnf->status);
    return 0;
}

static bt_status_t bt_sink_srv_a2dp_handle_start_streaming_ind(bt_a2dp_start_streaming_ind_t *start_ind, bool is_cnf)
{
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_A2DP_HD, (void *)(&(start_ind->handle)));
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_status_t ret = BT_STATUS_SUCCESS;

    bt_utils_assert(dev);
    bt_sink_srv_memcpy(&(dev->codec), start_ind->codec_cap, sizeof(bt_a2dp_codec_capability_t));
    uint32_t old_a2dp_status = dev->a2dp_status;
    dev->a2dp_status = BT_SINK_SRV_A2DP_STATUS_STREAMING;

#if (BT_A2DP_TOTAL_LINK_NUM>3)
    dev->a2dp_start_count = bt_sink_srv_state_manager_get_play_count();
#endif
    if (BT_SINK_SRV_MUSIC_TRANSIENT_STATE_PREPARE_BUFFER != dev->handle->substate) {
        /* Disable media data channel */
        uint32_t gap_hd = bt_sink_srv_cm_get_gap_handle(&(dev->dev_addr));
        bt_utils_assert(dev);
        bt_avm_set_music_enable(gap_hd, BT_AVM_ROLE_AWS_MCE_AGENT, false);
        /* set must play flag */
        if ((!(dev->avrcp_flag & BT_SINK_SRV_AVRCP_MUST_PLAY_RING_TONE_FLAG))
            && (ctx->context_flag & BT_SINK_SRV_CNTX_FLAG_MUST_PLAY_RING_TONE_FLAG)) {
            BT_SINK_SRV_SET_FLAG(dev->avrcp_flag, BT_SINK_SRV_AVRCP_MUST_PLAY_RING_TONE_FLAG);
            BT_SINK_SRV_REMOVE_FLAG(ctx->context_flag, BT_SINK_SRV_CNTX_FLAG_MUST_PLAY_RING_TONE_FLAG);
        }

        bt_sink_srv_music_state_change_handler(dev, old_a2dp_status, dev->avrcp_status);
    }

    if (!is_cnf) {
        bt_a2dp_start_streaming_response(dev->a2dp_hd, true);
    }

    bt_sink_srv_report_id("[sink][music][a2dp] start_streaming_ind-ret:dev:0x%x, a2dp status: 0x%x=>0x%x, substate:0x%x",
        4, dev, old_a2dp_status, dev->a2dp_status, dev->handle->substate);

    return ret;
}

static int32_t bt_sink_srv_a2dp_handle_suspend_streaming_ind(bt_a2dp_suspend_streaming_ind_t *suspend_ind, bool need_response)
{
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_sink_srv_music_device_t *a2dp_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_A2DP_HD, (void *)(&(suspend_ind->handle)));
    int32_t ret = 0;

    if (a2dp_dev) {
        bt_sink_srv_report_id("[sink][music][a2dp] suspend_streaming_ind--a2dp_dev:0x%x, flag:0x%x, op:0x%x", 3, a2dp_dev, a2dp_dev->flag, a2dp_dev->op);
        a2dp_dev->a2dp_status = BT_SINK_SRV_A2DP_STATUS_SUSPEND;
        if (!(a2dp_dev->flag & BT_SINK_SRV_MUSIC_FLAG_A2DP_INTERRUPT)) {
            audio_src_srv_del_waiting_list(a2dp_dev->handle);
        }
        BT_SINK_SRV_REMOVE_FLAG(a2dp_dev->flag, BT_SINK_SRV_MUSIC_NEED_TO_RESPONSE_A2DP_START

                                               | BT_SINK_SRV_MUSIC_STOP_MUSIC_PENDING
                                               | BT_SINK_SRV_MUSIC_FLAG_WAITING_START
                                               | BT_SINK_SRV_MUSIC_FLAG_ALC_REINITIAL_SYNC);

        if (a2dp_dev->flag & BT_SINK_SRV_MUSIC_FLAG_WAIT_LIST_SINK_PLAY) {
            BT_SINK_SRV_REMOVE_FLAG(a2dp_dev->flag, BT_SINK_SRV_MUSIC_FLAG_WAIT_LIST_SINK_PLAY);
            bt_sink_srv_report_id("[sink][music][a2dp] flag wait list sink play", 0);
        }

        bt_a2dp_suspend_streaming_response(suspend_ind->handle, true);
        if (a2dp_dev == ctx->run_dev) {
            BT_SINK_SRV_REMOVE_FLAG(a2dp_dev->flag, BT_SINK_SRV_MUSIC_FLAG_REINITIAL_SYNC);
            audio_src_srv_del_waiting_list(a2dp_dev->handle);
            bt_sink_srv_music_state_machine_handle(a2dp_dev, BT_A2DP_SUSPEND_STREAMING_IND, NULL);
        }
    }

    return ret;
}

static int32_t bt_sink_srv_a2dp_handle_suspend_streaming_cnf(bt_a2dp_suspend_streaming_cnf_t *suspend_cnf)
{
    bt_a2dp_suspend_streaming_ind_t suspend_streaming_ind = {0};
    suspend_streaming_ind.handle = suspend_cnf->handle;

    return bt_sink_srv_a2dp_handle_suspend_streaming_ind(&suspend_streaming_ind, false);
}

static int32_t bt_sink_srv_a2dp_handle_reconfigure_cnf(bt_a2dp_reconfigure_cnf_t *reconfigure_cnf)
{
    return 0;
}

static int32_t bt_sink_srv_a2dp_handle_reconfigure_ind(bt_a2dp_reconfigure_ind_t *reconfigure_ind)
{
    bt_sink_srv_music_device_t *a2dp_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_A2DP_HD, (void *)(&(reconfigure_ind->handle)));

    if (a2dp_dev) {
        memcpy(&(a2dp_dev->codec), reconfigure_ind->codec_cap, sizeof(bt_a2dp_codec_capability_t));
    }
    int32_t ret = bt_a2dp_reconfigure_response(reconfigure_ind->handle, true);

    return ret;
}

audio_src_srv_handle_t *bt_sink_srv_a2dp_alloc_pseudo_handle(void)
{
    uint32_t i = 0;
    audio_src_srv_handle_t *hd = NULL;

    for (i = 0; i < BT_SINK_SRV_A2DP_PSEUDO_COUNT + BT_SINK_SRV_MCE_A2DP_PSEUDO_COUNT; ++i) {
        if (!(g_a2dp_pse_hd[i].flag & BT_SINK_SRV_A2DP_PSEUDO_FLAG_USEED)) {
            hd = g_a2dp_pse_hd[i].hd;
            BT_SINK_SRV_SET_FLAG(g_a2dp_pse_hd[i].flag, BT_SINK_SRV_A2DP_PSEUDO_FLAG_USEED);
            bt_sink_srv_music_fill_audio_src_callback(hd);
            break;
        }
    }

    bt_sink_srv_report_id("[sink][music][a2dp] alloc_pseudo_handle--hd: 0x%x", 1, hd);

    bt_utils_assert(hd);
    return hd;
}


void bt_sink_srv_a2dp_free_pseudo_handle(audio_src_srv_handle_t *hd)
{
    uint32_t i = 0;

    if (hd) {
    for (i = 0; i < BT_SINK_SRV_A2DP_PSEUDO_COUNT + BT_SINK_SRV_MCE_A2DP_PSEUDO_COUNT; ++i) {
            if ((g_a2dp_pse_hd[i].flag & BT_SINK_SRV_A2DP_PSEUDO_FLAG_USEED) && (g_a2dp_pse_hd[i].hd == hd)) {
            BT_SINK_SRV_REMOVE_FLAG(g_a2dp_pse_hd[i].flag, BT_SINK_SRV_A2DP_PSEUDO_FLAG_USEED);
            break;
        }
    }
    }
    bt_sink_srv_report_id("[sink][music][a2dp] free_pseudo_handle--hd: 0x%x", 1, hd);
}

bt_status_t bt_sink_srv_a2dp_action_handler(bt_sink_srv_action_t action, void *param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_sink_srv_music_device_t *dev = NULL;
    bt_sink_srv_music_context_t *cntx = bt_sink_srv_music_get_context();
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_sink_srv_report_id("[sink][music][a2dp] process_a2dp_action[s]-action: 0x%x, param:0x%x, role:0x%x",
        3, action, param, role);

    (void)ret;
    if (BT_SINK_SRV_ACTION_SET_LATENCY == action) {
        if (param) {
            uint32_t *latency_val = (uint32_t *)param;
            ret = bt_sink_srv_music_set_sink_latency(*latency_val);
        }
    } else {
        if (BT_AWS_MCE_ROLE_AGENT == role || BT_AWS_MCE_ROLE_NONE == role) {
            dev = cntx->run_dev;
            switch (action) {
                case BT_SINK_SRV_ACTION_VOLUME_UP: {
                    if (param) {
                        bt_sink_srv_action_volume_up_t *sink_srv_dev = (bt_sink_srv_action_volume_up_t*)param;
                        if (BT_SINK_SRV_DEVICE_EDR == sink_srv_dev->type) {
                            dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_ADDR_A2DP, (void *)(&sink_srv_dev->address));
                        }
                    }
                    if (dev) {
                        bt_sink_srv_a2dp_change_volume(VOLUME_UP, 1, 0, dev);
                    } else {
                        ret = BT_STATUS_FAIL;
                    }
                    break;
                }

                case BT_SINK_SRV_ACTION_VOLUME_DOWN: {
                    if (param) {
                        bt_sink_srv_action_volume_down_t *sink_srv_dev = (bt_sink_srv_action_volume_up_t*)param;
                        if (BT_SINK_SRV_DEVICE_EDR == sink_srv_dev->type) {
                            dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_ADDR_A2DP, (void *)(&sink_srv_dev->address));
                        }
                    }
                    if (dev) {
                        bt_sink_srv_a2dp_change_volume(VOLUME_DOWN, 1, 0, dev);
                    } else {
                        ret = BT_STATUS_FAIL;
                    }
                    break;
                }

                case BT_SINK_SRV_ACTION_SET_VOLUME: {
                    uint8_t *volume_value = (uint8_t *)param;
                    if (dev) {
                        bt_sink_srv_a2dp_change_volume(VOLUME_VALUE, 1, *volume_value, dev);
                    } else {
                        ret = BT_STATUS_FAIL;
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    return ret;
}


#ifdef MTK_AWS_MCE_ENABLE
static void bt_sink_srv_a2dp_set_special_dev_mtu(bt_device_manager_db_remote_pnp_info_t *device_id, bt_bd_addr_t addr, bool is_acl_connected)
{
    uint32_t mtu_size = BT_SINK_SRV_A2DP_SPECIAL_DEVICE_MTU_SIZE;
    bool ret = bt_iot_device_white_list_check_iot_case((bt_bd_addr_t *)addr, BT_IOT_MUSIC_SET_SPECIAL_MTU);
    if (ret) {
        if (!is_acl_connected) {
            mtu_size = BT_SINK_SRV_A2DP_NORMAL_MTU_SIZE;
        }
        bt_a2dp_set_mtu_size(mtu_size);
    }
    bt_sink_srv_report_id("[sink][music][a2dp] set special MTU size: %d, ret:0x%x, is_acl_connected:0x%x", 3, mtu_size, ret, is_acl_connected);
}

static bt_status_t bt_sink_srv_a2dp_link_status_change_handler(bt_gap_link_status_updated_ind_t *link_data)
{
    bt_sink_srv_report_id("[sink][music][a2dp] link status change handle, link_status: 0x%x", 1, link_data->link_status);
    bt_device_manager_db_remote_pnp_info_t dev_id_p = {0, 0};

    bt_device_manager_remote_find_pnp_info((void *)(link_data->address), &dev_id_p);
    if (dev_id_p.product_id != 0 && dev_id_p.vender_id != 0) {
        if (link_data->link_status == BT_GAP_LINK_STATUS_CONNECTED_0) {
            bt_sink_srv_a2dp_set_special_dev_mtu(&dev_id_p, (void *)(link_data->address), true);
        } else if (link_data->link_status == BT_GAP_LINK_STATUS_DISCONNECTED) {
            bt_sink_srv_a2dp_set_special_dev_mtu(&dev_id_p, (void *)(link_data->address), false);
        }
    }
    return BT_STATUS_SUCCESS;
}

static void bt_sink_srv_a2dp_sdp_attribute_handler(void *buffer)
{
    bt_sdpc_attribute_cnf_t *attr_result = (bt_sdpc_attribute_cnf_t *)buffer;
    bt_device_manager_db_remote_pnp_info_t device_id = {0, 0};
    uint8_t *parse_result = NULL;
    uint8_t *vendor_id = NULL;
    uint16_t vendor_id_len = 0;
    uint16_t result_len = 0;

    bt_sdpc_parse_attribute(&parse_result, &result_len, BT_DI_SDP_ATTRIBUTE_VENDOR_ID, 0, attr_result->length, attr_result->attribute_data);
    bt_sink_srv_report_id("[sink][music][a2dp] BT_SDPC_SEARCH_ATTRIBUTE_CNF-result_len:0x%x",
        1, result_len);

    if (parse_result) {
        bt_sdpc_parse_next_value(&vendor_id, &vendor_id_len, parse_result, result_len);
        if (vendor_id_len) {
            device_id.vender_id = (vendor_id[0] << 8 | vendor_id[1]);
        }
        bt_sink_srv_report_id("[sink][music][a2dp] 1-vendor_id_len:0x%x, vender_id:0x%x", 2, vendor_id_len, device_id.vender_id);
            bt_sdpc_parse_attribute(&parse_result, &result_len, BT_DI_SDP_ATTRIBUTE_PRODUCT_ID, 0, attr_result->length, attr_result->attribute_data);
        bt_sink_srv_report_id("[sink][music][a2dp] 2-result_len:0x%x, parse_result:0x%x",
            2, result_len, parse_result[0] << 8 | parse_result[1]);
        bt_sdpc_parse_next_value(&vendor_id, &vendor_id_len, parse_result, result_len);
        if (vendor_id_len) {
            device_id.product_id = (vendor_id[0] << 8 | vendor_id[1]);
        }
        bt_sink_srv_report_id("[sink][music][a2dp] 3-vendor_id_len:0x%x, product_id:0x%x", 2, vendor_id_len, device_id.product_id);
        if (device_id.product_id && device_id.vender_id) {
            bt_sink_srv_a2dp_set_special_dev_mtu(&device_id, (void *)(attr_result->user_data), true);
        }
    }
}
#endif

bt_status_t bt_sink_srv_a2dp_common_callback(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    uint32_t moduel = msg & 0xFF000000;
#ifdef AIR_FEATURE_SINK_AVRCP_BQB
    if (bt_sink_srv_avrcp_bqb_in_progress()) {
        return ret;
    }
#endif

    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    if ((role == BT_AWS_MCE_ROLE_AGENT) || (role == BT_AWS_MCE_ROLE_NONE)) {
        switch (msg) {
            case BT_A2DP_CONNECT_CNF: {
                bt_a2dp_connect_cnf_t *conn_cnf = (bt_a2dp_connect_cnf_t *)buffer;
                ret = bt_sink_srv_a2dp_handle_connect_cnf(conn_cnf);
                break;
            }

            case BT_A2DP_CONNECT_IND: {
                bt_a2dp_connect_ind_t *conn_ind = (bt_a2dp_connect_ind_t *)buffer;
                ret = bt_sink_srv_a2dp_handle_connect_ind(conn_ind);
                break;
            }

            case BT_A2DP_DISCONNECT_CNF: {
                bt_a2dp_disconnect_cnf_t *disconn_cnf = (bt_a2dp_disconnect_cnf_t *)buffer;
                ret = bt_sink_srv_a2dp_handle_disconnect_handler(disconn_cnf->handle, msg, disconn_cnf->status);
                break;
            }

            case BT_A2DP_DISCONNECT_IND: {
                bt_a2dp_disconnect_ind_t *disconn_ind = (bt_a2dp_disconnect_ind_t *)buffer;
                ret = bt_sink_srv_a2dp_handle_disconnect_handler(disconn_ind->handle, msg, BT_STATUS_SUCCESS);
                break;
            }

            case BT_A2DP_START_STREAMING_CNF: {
                bt_a2dp_start_streaming_cnf_t *start_cnf = (bt_a2dp_start_streaming_cnf_t *)buffer;
                ret = bt_sink_srv_a2dp_handle_start_streaming_cnf(start_cnf);
                break;
            }

            case BT_A2DP_START_STREAMING_IND: {
                bt_a2dp_start_streaming_ind_t *start_ind = (bt_a2dp_start_streaming_ind_t *)buffer;
                        bt_sink_srv_report_id("[sink][music][a2dp] start_ind-buff:0x%x,handle:0x%x,cap:0x%x,type:0x%x,sec_type:0x%x,delay_report:0x%x,length:0x%x",
                            7,buffer,start_ind->handle,start_ind->codec_cap,start_ind->codec_cap->type,start_ind->codec_cap->sec_type,start_ind->codec_cap->delay_report,start_ind->codec_cap->length);

                if (start_ind->codec_cap->type == BT_A2DP_CODEC_SBC) {
                            bt_sink_srv_report_id("[sink][music][a2dp] start_ind(sbc)--1: %d, 2: %d, 3: %d, 4: %d, 5: %d, 6: %d, 7: %d", 7,
                                          start_ind->codec_cap->codec.sbc.channel_mode,
                                          start_ind->codec_cap->codec.sbc.sample_freq,
                                          start_ind->codec_cap->codec.sbc.alloc_method,
                                          start_ind->codec_cap->codec.sbc.subbands,
                                          start_ind->codec_cap->codec.sbc.block_len,
                                          start_ind->codec_cap->codec.sbc.min_bitpool,
                                          start_ind->codec_cap->codec.sbc.max_bitpool);
                } else if (start_ind->codec_cap->type == BT_A2DP_CODEC_AAC) {
                            bt_sink_srv_report_id("[sink][music][a2dp] start_ind(aac)--1: %d, 2: %d, 3: %d, 4: %d, 5: %d, 6: %d, 7: %d, 8: %d, 9: %d", 9,
                                          start_ind->codec_cap->codec.aac.object_type,
                                          start_ind->codec_cap->codec.aac.drc,
                                          start_ind->codec_cap->codec.aac.freq_h,
                                          start_ind->codec_cap->codec.aac.channels,
                                          start_ind->codec_cap->codec.aac.freq_l,
                                          start_ind->codec_cap->codec.aac.br_h,
                                          start_ind->codec_cap->codec.aac.vbr,
                                          start_ind->codec_cap->codec.aac.br_m,
                                          start_ind->codec_cap->codec.aac.br_l);
                }

                ret = bt_sink_srv_a2dp_handle_start_streaming_ind(start_ind, false);
                break;
            }

            case BT_A2DP_SUSPEND_STREAMING_CNF: {
                bt_a2dp_suspend_streaming_cnf_t *suspend_cnf = (bt_a2dp_suspend_streaming_cnf_t *)buffer;

                ret = bt_sink_srv_a2dp_handle_suspend_streaming_cnf(suspend_cnf);
                break;
            }

            case BT_A2DP_SUSPEND_STREAMING_IND: {
                bt_a2dp_suspend_streaming_ind_t *suspend_ind = (bt_a2dp_suspend_streaming_ind_t *)buffer;

                ret = bt_sink_srv_a2dp_handle_suspend_streaming_ind(suspend_ind, true);
                break;
            }

            case BT_A2DP_RECONFIGURE_CNF: {
                bt_a2dp_reconfigure_cnf_t *reconfigure_cnf = (bt_a2dp_reconfigure_cnf_t *)buffer;

                ret = bt_sink_srv_a2dp_handle_reconfigure_cnf(reconfigure_cnf);
                break;
            }

            case BT_A2DP_RECONFIGURE_IND: {
                bt_a2dp_reconfigure_ind_t *reconfigure_ind = (bt_a2dp_reconfigure_ind_t *)buffer;

                ret = bt_sink_srv_a2dp_handle_reconfigure_ind(reconfigure_ind);
                break;
            }

            case BT_AVM_MEDIA_DATA_RECEIVED_IND: {
                bt_avm_a2dp_media_info_t *media_info = (bt_avm_a2dp_media_info_t *)buffer;
                bt_sink_srv_music_data_info_t media_data_info = {0};
                media_data_info.asi = media_info->asi;
                media_data_info.clock.nclk = media_info->clock.nclk;
                media_data_info.clock.nclk_intra = media_info->clock.nclk_intra;
                media_data_info.ratio = (media_info->ratio == 0xff ? 0xffffffff : media_info->ratio);
                media_data_info.samples = media_info->samples;
                bt_sink_srv_a2dp_handle_media_data_received_ind(media_info->gap_handle, &media_data_info);
                break;
            }
            case BT_AVM_SEQUENCE_NUM_WRAP_COUNT_IND: {
                bt_avm_a2dp_sn_wrap_count_t *wrap_count_info = (bt_avm_a2dp_sn_wrap_count_t *)buffer;
                bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
                uint32_t gap_hd = bt_sink_srv_cm_get_gap_handle(&(ctx->run_dev->dev_addr));
                if (gap_hd == wrap_count_info->gap_handle) {
                    g_sn_wrap_count = wrap_count_info->wrap_count;
                }
                bt_sink_srv_report_id("[sink][music][a2dp] sequence num wrap count:%d, handle:0x%08x, gap_handle:0x%08x", 1, g_sn_wrap_count, gap_hd, wrap_count_info->gap_handle);
                break;
            }

            case BT_AVM_SET_LOCAL_ASI_FLAG: {
                bt_avm_a2dp_local_asi_ind_t *media_info = (bt_avm_a2dp_local_asi_ind_t *)buffer;
                bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
                ret = bt_sink_srv_music_set_nvdm_data(&ctx->run_dev->dev_addr, BT_SINK_SRV_MUSIC_DATA_LOCAL_ASI_FLAG, (void *)(&media_info->local_asi_flag));
                break;
            }

#ifdef MTK_AWS_MCE_ENABLE
            case BT_GAP_LINK_STATUS_UPDATED_IND: {
                bt_gap_link_status_updated_ind_t *link_data = (bt_gap_link_status_updated_ind_t *)buffer;
                ret = bt_sink_srv_a2dp_link_status_change_handler(link_data);
                break;
            }
            case BT_SDPC_SEARCH_ATTRIBUTE_CNF: {
                bt_sink_srv_a2dp_sdp_attribute_handler(buffer);
                break;
            }
#endif

            default:
                break;
        }
    }
    moduel = msg & 0xFF000000;
    if((moduel == BT_MODULE_A2DP) || (moduel == BT_MODULE_AVM) 
#ifdef MTK_AWS_MCE_ENABLE
        || (msg == BT_GAP_LINK_STATUS_UPDATED_IND) || (msg == BT_SDPC_SEARCH_ATTRIBUTE_CNF)
#endif
        ) {
        bt_sink_srv_report_id("[sink][music][a2dp] common_hdr-msg: 0x%x, status: 0x%x, ret: 0x%x", 3, ret, msg, status);
    }
    return ret;
}

#ifdef MTK_AUDIO_SYNC_ENABLE
bt_status_t bt_sink_srv_a2dp_change_sync_volume(uint8_t type, uint8_t notify_avrcp, uint32_t volume_value)
{
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_sink_srv_music_device_t *run_dev = ctx->run_dev;
    uint8_t vol = ctx->vol_lev;
    bt_status_t ret = BT_STATUS_FAIL;

    if ((VOLUME_UP == type) && (vol < BT_SINK_SRV_A2DP_MAX_VOL_LEV)) {
        vol++;
    } else if ((VOLUME_DOWN == type) && (vol > BT_SINK_SRV_A2DP_MIN_VOL_LEV)) {
        vol--;
    } else if ((VOLUME_VALUE == type) && (volume_value >= BT_SINK_SRV_A2DP_MIN_VOL_LEV)
               && (volume_value <= BT_SINK_SRV_A2DP_MAX_VOL_LEV)) {
        vol = volume_value;
    }

    if ((vol != ctx->vol_lev) && run_dev && (run_dev->op & BT_SINK_SRV_MUSIC_OP_DRV_PLAY)) {
        bt_sink_srv_audio_sync_music_data_t music_data = {0};
        bt_sink_srv_get_sync_data_parameter_t sync_data = {0};
        music_data.volume = vol;
        if (notify_avrcp) {
            music_data.volume_type = type;
        }
        sync_data.type = BT_SINK_SRV_A2DP_VOLUME_TYPE;
        sync_data.length = sizeof(bt_sink_srv_audio_sync_music_data_t);
        sync_data.duration = 200000;
        sync_data.timeout_duration = 0xffffffff;
        sync_data.data = &music_data;
        ctx->vol_lev = vol;
        ret = bt_sink_srv_music_set_nvdm_data(&(run_dev->dev_addr), BT_SINK_SRV_MUSIC_DATA_VOLUME, &ctx->vol_lev);
        ret = bt_sink_srv_request_sync_gpt(&sync_data);
    }

    return ret;
}
#endif

int32_t bt_sink_srv_a2dp_change_volume(uint8_t type, uint8_t sync, uint32_t volume_value, bt_sink_srv_music_device_t *dev)
{
    bt_sink_srv_mutex_lock();

    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_sink_srv_music_device_t *run_dev = ctx->run_dev;
    bt_sink_srv_am_id_t ami_ret = 0;
    uint8_t vol = 0, old_vol = 0;
    (void)ami_ret;

    bt_utils_assert(dev && "please input valid dev!");
    bt_sink_srv_music_get_nvdm_data(&(dev->dev_addr), BT_SINK_SRV_MUSIC_DATA_VOLUME, &old_vol);
    vol = old_vol;
#ifdef MTK_AUDIO_SYNC_ENABLE
    if (run_dev == dev && (run_dev->op & BT_SINK_SRV_MUSIC_OP_DRV_PLAY) && sync) {
        bt_aws_mce_agent_state_type_t aws_state = BT_AWS_MCE_AGENT_STATE_NONE;
        uint32_t aws_handle = bt_sink_srv_aws_mce_get_handle(&(run_dev->dev_addr));
        if (aws_handle) {
            aws_state = bt_sink_srv_aws_mce_get_aws_state_by_handle(aws_handle);
        }

        if (aws_handle && (aws_state == BT_AWS_MCE_AGENT_STATE_ATTACHED)) {
            bt_sink_srv_a2dp_change_sync_volume(type, 1, volume_value);
            bt_sink_srv_mutex_unlock();
            return BT_STATUS_SUCCESS;
        }
    }
#endif

    /* volume up */
    if (VOLUME_UP == type) {
        if (vol < BT_SINK_SRV_A2DP_MAX_VOL_LEV) {
            vol = vol + 1;
        }
#ifdef MTK_BT_SPEAKER_ENABLE
        if (run_dev == dev && sync && (BT_SINK_SRV_MUSIC_MODE_BROADCAST == bt_sink_srv_music_get_spk_mode())) {
            bt_sink_srv_aws_mce_a2dp_send_eir(BT_SINK_SRV_AWS_MCE_A2DP_EVT_VOL_UP, (void *)(&(run_dev->a2dp_hd)));
        }
#endif
    } else if (VOLUME_DOWN == type) {
        if (vol > BT_SINK_SRV_A2DP_MIN_VOL_LEV) {
            vol = vol - 1;
        }
#ifdef MTK_BT_SPEAKER_ENABLE
        if (run_dev == dev && sync && (BT_SINK_SRV_MUSIC_MODE_BROADCAST == bt_sink_srv_music_get_spk_mode())) {
            bt_sink_srv_aws_mce_a2dp_send_eir(BT_SINK_SRV_AWS_MCE_A2DP_EVT_VOL_DOWN, (void *)(&(run_dev->a2dp_hd)));
        }
#endif
    } else if (VOLUME_VALUE == type) {
        if (volume_value <= BT_SINK_SRV_A2DP_MAX_VOL_LEV) {
            vol = volume_value;
        }
    }

    if ((vol != old_vol) && dev) {
        ctx->vol_lev = vol;
        bt_sink_srv_music_set_am_volume(ctx->vol_lev);
        if (sync) {
            bt_sink_srv_music_set_nvdm_data(&(dev->dev_addr), BT_SINK_SRV_MUSIC_DATA_VOLUME, &vol);
        }
#ifdef MTK_BT_SPEAKER_ENABLE
        else {
            uint16_t music_volume;
            music_volume = BT_SINK_SRV_A2DP_MAGIC_CODE | vol;
            bt_connection_manager_device_local_info_set_local_music_volume(&music_volume);
        }
#endif
        bt_sink_srv_avrcp_volume_notification(dev->avrcp_hd, vol, type);

#ifdef __BT_AWS_MCE_A2DP_SUPPORT__
        if (sync && (run_dev == dev)
#ifdef MTK_BT_SPEAKER_ENABLE
            && BT_SINK_SRV_MUSIC_MODE_BROADCAST != bt_sink_srv_music_get_spk_mode()
#endif
           ) {
            bt_sink_srv_aws_mce_a2dp_send_eir(BT_SINK_SRV_AWS_MCE_A2DP_EVT_VOL_SYNC, (void *)(&(ctx->run_dev->a2dp_hd)));
        }
#endif
    }
    bt_sink_srv_report_id("[sink][music][a2dp] change_volume-ami_ret: %d, vol: %d, dev:0x%x, run_dev:0x%x, old_vol:%d, vol:%d, type:0x%x, sync:%d",
        9, ami_ret, ctx->vol_lev, dev, run_dev, old_vol, vol, type, sync);
    bt_sink_srv_mutex_unlock();

    return BT_STATUS_SUCCESS;
}

#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
static void bt_sink_srv_a2dp_resume_timer_handler(uint32_t timer_id, uint32_t data)
{
    bt_sink_srv_music_device_t *a2dp_dev = (bt_sink_srv_music_device_t*)data;
    bt_sink_srv_report_id("[sink][music][a2dp] resume_timer_handler--a2dp_dev:0x%x", 1,
                          a2dp_dev);

    if ((a2dp_dev->conn_bit & BT_SINK_SRV_MUSIC_A2DP_CONN_BIT)
        && (BT_SINK_SRV_A2DP_STATUS_STREAMING != a2dp_dev->a2dp_status)) {
        bt_sink_srv_music_state_machine_handle(a2dp_dev, BT_A2DP_SUSPEND_STREAMING_IND, NULL);
    }
}
#endif

void bt_sink_srv_a2dp_play(audio_src_srv_handle_t *handle)
{
    bt_sink_srv_mutex_lock();
    bt_sink_srv_music_device_t *a2dp_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_PSE_HD, (void *)handle);
    bt_sink_srv_am_audio_capability_t aud_cap = {0};
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    int32_t ret = 0;

    bt_utils_assert(a2dp_dev);
    BT_SINK_SRV_REMOVE_FLAG(a2dp_dev->flag, BT_SINK_SRV_MUSIC_FLAG_WAIT_LIST_SINK_PLAY);
    bt_sink_srv_report_id("[sink][music][a2dp] play(s)--a2dp_dev:0x%x, hd: 0x%08x, type: %d, flag: 0x%08x, op: 0x%08x", 5,
                          a2dp_dev, handle, handle->type, a2dp_dev->flag, a2dp_dev->op);
#ifndef BT_SINK_SRV_IMPROVE_RESYNC
#ifdef AIR_LE_AUDIO_ENABLE
    if (a2dp_dev->flag & BT_SINK_SRV_MUSIC_FLAG_REINITIAL_SYNC) {
        bt_sink_srv_cap_am_enable_waiting_list();
    }
#endif
#endif

    bt_sink_srv_music_stop_vp_detection(a2dp_dev->handle);
    if (a2dp_dev->flag & BT_SINK_SRV_MUSIC_FLAG_A2DP_INTERRUPT) {
        BT_SINK_SRV_REMOVE_FLAG(a2dp_dev->flag, BT_SINK_SRV_MUSIC_FLAG_A2DP_INTERRUPT);
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
        bt_sink_srv_config_t ue_config = {0};
        bt_sink_srv_state_manager_get_resume_config_t get_resume_config = {0};
        get_resume_config.type = BT_SINK_SRV_DEVICE_EDR;
        bt_sink_srv_memcpy(&(get_resume_config.address),&(a2dp_dev->dev_addr),sizeof(bt_bd_addr_t));
        if (BT_STATUS_SUCCESS == bt_sink_srv_state_manager_get_config(BT_SINK_SRV_GET_RESUME_CONFIG, 
                                                                        (void*)(&get_resume_config),
                                                                        &ue_config)) {
            if (BT_SINK_SRV_INTERRUPT_OPERATION_PLAY_MUSIC == ue_config.resume_config.resume_operation) {
                if (BT_AVRCP_STATUS_PLAY_PAUSED == a2dp_dev->avrcp_status) {
                    bt_timer_ext_start(BT_SINK_SRV_RESUME_SRC_WAITING_TIMER, (uint32_t)a2dp_dev,
                           BT_SINK_SRV_AVRCP_PLAY_PAUSE_ACTION_TIMER_DUR, bt_sink_srv_a2dp_resume_timer_handler);
                    ret = bt_sink_srv_avrcp_play_music(a2dp_dev);
                }
            }
        }
#else
        if (BT_AVRCP_STATUS_PLAY_PAUSED == a2dp_dev->avrcp_status) {
            ret = bt_sink_srv_avrcp_play_music(a2dp_dev);
        }
#endif
    }

    bt_sink_srv_music_get_nvdm_data(&(a2dp_dev->dev_addr), BT_SINK_SRV_MUSIC_DATA_VOLUME, &ctx->vol_lev);
#ifdef __BT_AWS_MCE_A2DP_SUPPORT__
    if (a2dp_dev->a2dp_status == BT_SINK_SRV_A2DP_STATUS_STREAMING) {
        bt_sink_srv_aws_mce_a2dp_send_eir(BT_SINK_SRV_AWS_MCE_A2DP_EVT_START_STREAMING, (void *)(&(a2dp_dev->a2dp_hd)));
    }
#endif

#ifdef AIR_MULTI_POINT_ENABLE
    bt_cm_connect_t conn_req = {0};
    bt_sink_srv_memcpy(&(conn_req.address), &(a2dp_dev->dev_addr), sizeof(bt_bd_addr_t));
    conn_req.profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS);
    bt_cm_connect(&conn_req);
#endif
    /* Update run device */
    bt_sink_srv_music_update_run_device(a2dp_dev);

    uint16_t a2dp_mtu = bt_a2dp_get_mtu_size(a2dp_dev->a2dp_hd);
    bt_sink_srv_music_fill_am_aud_param(&aud_cap, &a2dp_dev->codec, BT_A2DP_SINK, a2dp_mtu);
    ctx->am_vol_lev = ctx->vol_lev;
#ifdef AIR_A2DP_SYNC_STOP_ENABLE
    bt_sink_srv_ami_audio_service_hook_callback(AUDIO_MESSAGE_TYPE_BT_A2DP_DL, bt_sink_srv_music_sync_stop_done_handler, NULL);
#endif

    if (!(a2dp_dev->flag & BT_SINK_SRV_MUSIC_FLAG_REINITIAL_SYNC)) {
        uint32_t latency_val = 0;
        if (BT_SINK_SRV_MUSIC_MODE_BROADCAST != bt_sink_srv_music_get_spk_mode()) {
            if (a2dp_dev->codec.type != BT_A2DP_CODEC_VENDOR) {
                bt_sink_srv_music_mode_t music_mode = bt_sink_srv_music_get_mode(&a2dp_dev->dev_addr);
                latency_val = bt_sink_srv_get_latency(&a2dp_dev->dev_addr, false, false, false);
                if (BT_STATUS_SUCCESS == ret
                    && latency_val < BT_SINK_SRV_A2DP_SPECIAL_DEVICE_LATENCY
                    && (music_mode != BT_SINK_SRV_MUSIC_GAME_MODE)){
                    if(bt_iot_device_white_list_check_iot_case((bt_bd_addr_t *)&ctx->run_dev->dev_addr, BT_IOT_MUSIC_SET_LATENCY_TO_250)) {
                        latency_val = BT_SINK_SRV_A2DP_SPECIAL_DEVICE_LATENCY;
                    } else if(latency_val < BT_SINK_SRV_A2DP_SPECIAL_DEVICE_LATENCY_170MS
                        && bt_iot_device_white_list_check_iot_case((bt_bd_addr_t *)&ctx->run_dev->dev_addr, BT_IOT_MUSIC_SET_LATENCY_TO_170)) {
                        latency_val = BT_SINK_SRV_A2DP_SPECIAL_DEVICE_LATENCY_170MS;
                    }
                }
            } else {
#ifdef AIR_BT_A2DP_VENDOR_2_ENABLE
                if(bt_sink_srv_music_is_lhdc_ll_mode(&(a2dp_dev->codec))) {
                    latency_val = BT_SINK_SRV_A2DP_LHDC_LL_MODE_LATENCY;
                } else 
#endif
                {
                    latency_val = bt_sink_srv_get_latency(&a2dp_dev->dev_addr, true, false, false);
                }
            }
        } else {
            latency_val = bt_sink_srv_get_latency(&a2dp_dev->dev_addr, false, false, true);
        }
        bt_sink_srv_music_set_sink_latency(latency_val);
    }
#if defined (AIR_LE_AUDIO_ENABLE) && defined (__BT_AWS_MCE_A2DP_SUPPORT__) && defined (AIR_LE_AUDIO_CIS_ENABLE)
    bt_handle_t le_handle = bt_sink_srv_cap_get_link_handle(0xFF);
    if (BT_HANDLE_INVALID != le_handle) {
        bt_sink_srv_cap_update_connection_interval(le_handle, 0x30);
    }
#endif

    bt_sink_srv_set_clock_offset_ptr_to_dsp((const bt_bd_addr_t *)(&(a2dp_dev->dev_addr)));

#ifdef BT_SINK_DUAL_ANT_ENABLE
    bt_sink_srv_dual_ant_data_t notify = {0};
    notify.type = BT_SINK_DUAL_ANT_TYPE_MUSIC;
    notify.music_info.a2dp_state = true;
    bt_sink_srv_dual_ant_notify(false, &notify);
#endif
    bt_gap_reset_sniff_timer(BT_SINK_SRV_A2DP_MIN_SNIFF_DUR);

    BT_SINK_SRV_SET_FLAG(a2dp_dev->flag, BT_SINK_SRV_MUSIC_FLAG_WAIT_AMI_OPEN_CODEC);
    bt_sink_srv_am_result_t am_ret = bt_sink_srv_ami_audio_play(ctx->a2dp_aid, &aud_cap);
    bt_sink_srv_update_last_device(&(a2dp_dev->dev_addr), BT_SINK_SRV_PROFILE_A2DP_SINK);

    if (a2dp_dev->flag & BT_SINK_SRV_MUSIC_NEED_TO_RESPONSE_A2DP_START) {
        BT_SINK_SRV_REMOVE_FLAG(a2dp_dev->flag, BT_SINK_SRV_MUSIC_NEED_TO_RESPONSE_A2DP_START);
        bt_a2dp_start_streaming_response(a2dp_dev->a2dp_hd, true);
    }

    bt_utils_assert(AUD_EXECUTION_SUCCESS == am_ret);

    if (a2dp_dev->handle->substate != BT_SINK_SRV_MUSIC_TRANSIENT_STATE_PREPARE_CODEC) {
        bt_sink_srv_music_state_machine_handle(a2dp_dev, BT_SINK_SRV_MUSIC_EVT_PREPARE_CODEC, NULL);
    }
    bt_sink_srv_mutex_unlock();
}
static void bt_sink_srv_a2dp_imp_stop(bt_sink_srv_music_device_t *a2dp_dev)
{
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();

    bt_utils_assert(a2dp_dev);
    bt_sink_srv_report_id("[sink][music][a2dp] imp stop(s)--a2dp_dev: 0x%x, flag: 0x%x, op: 0x%x", 3,
                      a2dp_dev, a2dp_dev->flag, a2dp_dev->op);
#ifdef __BT_AWS_MCE_A2DP_SUPPORT__
    if (a2dp_dev->conn_bit & BT_SINK_SRV_MUSIC_A2DP_CONN_BIT) {
        bt_sink_srv_aws_mce_a2dp_send_eir(BT_SINK_SRV_AWS_MCE_A2DP_EVT_SUSPEND_STREAMING, (void *)(&(a2dp_dev->a2dp_hd)));
    }
#endif

    /* Clear codec */
#ifdef AIR_A2DP_SYNC_STOP_ENABLE
    uint32_t aws_handle = bt_sink_srv_aws_mce_get_handle(&(a2dp_dev->dev_addr));
    bt_aws_mce_agent_state_type_t aws_state = bt_sink_srv_aws_mce_get_aws_state_by_handle(aws_handle);
    if ((a2dp_dev->state == AUDIO_SRC_SRV_STATE_PLAYING) && (aws_state == BT_AWS_MCE_AGENT_STATE_ATTACHED)) {
        if (BT_STATUS_SUCCESS != bt_sink_srv_music_request_sync_stop()) {
            bt_sink_srv_music_set_force_stop(true);
            bt_sink_srv_music_stop(a2dp_dev, ctx->a2dp_aid);
        } else {
            bt_sink_srv_music_set_force_stop(false);
        }
    } else {
        bt_sink_srv_music_stop(a2dp_dev, ctx->a2dp_aid);
    }
#else
    bt_sink_srv_music_stop(a2dp_dev, ctx->a2dp_aid);
#endif
}



void bt_sink_srv_a2dp_stop(audio_src_srv_handle_t *handle)
{
    bt_sink_srv_music_device_t *a2dp_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_PSE_HD, (void *)handle);

    bt_utils_assert(a2dp_dev);

    bt_sink_srv_report_id("[sink][music][a2dp] stop(s)-handle:0x%x", 1, handle);
    BT_SINK_SRV_SET_FLAG(a2dp_dev->flag, BT_SINK_SRV_MUSIC_NORMAL_STOP_FLAG);
#if defined (AIR_LE_AUDIO_ENABLE) && defined (__BT_AWS_MCE_A2DP_SUPPORT__) && defined (AIR_LE_AUDIO_CIS_ENABLE)
    bt_handle_t le_handle = bt_sink_srv_cap_get_link_handle(0xFF);
    if (BT_HANDLE_INVALID != le_handle) {
        bt_sink_srv_cap_update_connection_interval(le_handle, 0x30);
    }
#endif
    bt_sink_srv_a2dp_imp_stop(a2dp_dev);
    /* When interrupt is comming during prepare codec, need to switch to int-dev after codec done, the running dev
       is still this dev in suspend function, so can not add running dev to waiting list at AM side. */
    if ((a2dp_dev->flag & BT_SINK_SRV_MUSIC_FLAG_A2DP_INTERRUPT) && a2dp_dev->handle) {
        if (!(a2dp_dev->handle->flag & AUDIO_SRC_SRV_FLAG_WAITING)) {
            bt_sink_srv_a2dp_add_waitinglist(a2dp_dev->handle);
        }
    }
}

void bt_sink_srv_a2dp_suspend(audio_src_srv_handle_t *handle, audio_src_srv_handle_t *int_hd)
{
    bt_sink_srv_mutex_lock();
    bt_sink_srv_music_device_t *a2dp_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_PSE_HD, (void *)handle);
    bt_sink_srv_music_device_t *int_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_PSE_HD, (void *)int_hd);

    bt_utils_assert(a2dp_dev && handle && int_hd);
    BT_SINK_SRV_REMOVE_FLAG(a2dp_dev->flag, BT_SINK_SRV_MUSIC_FLAG_REINITIAL_SYNC);
    /*GVA-13775, if a2dp suspend come before open codec done and esco need to suspend a2dp, just do nothing.*/
    if (a2dp_dev->a2dp_status == BT_SINK_SRV_A2DP_STATUS_SUSPEND
        && BT_SINK_SRV_MUSIC_TRANSIENT_STATE_CLEAR_CODEC == a2dp_dev->handle->substate) {
        bt_sink_srv_mutex_unlock();
        return;
    }

    /* Clear codec */
    if (a2dp_dev->handle->substate != BT_SINK_SRV_MUSIC_TRANSIENT_STATE_CLEAR_CODEC) {
        bt_sink_srv_music_state_machine_handle(a2dp_dev, BT_SINK_SRV_MUSIC_EVT_PREPARE_CLEAR, NULL);
    }
    bt_sink_srv_a2dp_imp_stop(a2dp_dev);
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
    bt_sink_srv_config_t ue_config = {0};
    bt_sink_srv_state_manager_get_suspend_config_t get_suspend_config = {0};
    get_suspend_config.type = BT_SINK_SRV_DEVICE_EDR;
    bt_sink_srv_memcpy(&(get_suspend_config.address),&(a2dp_dev->dev_addr),sizeof(bt_bd_addr_t));
    get_suspend_config.suspend_handle = int_hd;
    bt_sink_srv_state_manager_get_config(BT_SINK_SRV_GET_SUSPEND_CONFIG, 
                                        (void*)(&get_suspend_config),
                                        &ue_config);
#endif
    int32_t ret = 0;
    (void)ret;
    if ((handle->dev_id == int_hd->dev_id) &&
        (int_hd->type == AUDIO_SRC_SRV_PSEUDO_DEVICE_HFP)) {
        if (a2dp_dev->avrcp_status == BT_AVRCP_STATUS_PLAY_PLAYING) {
            /* WSAP00041710 Nexus 5, HF don't interrupt A2DP */
            /* Add self in waiting list */
            bt_sink_srv_a2dp_add_waitinglist(handle);
        }
    } else if ((int_hd->type == AUDIO_SRC_SRV_PSEUDO_DEVICE_A2DP) || ((int_hd->type == AUDIO_SRC_SRV_PSEUDO_DEVICE_BLE) && (int_hd->priority == AUDIO_SRC_SRV_PRIORITY_NORMAL))) {
        /* PartyMode interrupt */
        if (int_dev || (int_hd->type == AUDIO_SRC_SRV_PSEUDO_DEVICE_BLE)) {
            bt_sink_srv_a2dp_add_waitinglist(handle);
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
            if ((BT_SINK_SRV_INTERRUPT_OPERATION_PAUSE_MUSIC == ue_config.suspend_config.suspend_operation)
                && (BT_AVRCP_STATUS_PLAY_PLAYING == a2dp_dev->avrcp_status)) {
                bt_sink_srv_music_pause_remote_music(a2dp_dev);
                if (ue_config.suspend_config.will_suspend_resume) {
                    BT_SINK_SRV_SET_FLAG(a2dp_dev->flag, BT_SINK_SRV_MUSIC_FLAG_A2DP_INTERRUPT);
                }
            }
            if (ue_config.suspend_config.will_suspend_resume) {
                if (ue_config.suspend_config.suspend_resume_timeout) {
                    bt_sink_srv_music_start_vp_detection(a2dp_dev->handle, ue_config.suspend_config.suspend_resume_timeout);
                }
            }
#else
            bt_sink_srv_music_pause_remote_music(a2dp_dev);
#endif
        }
    } else {
        /* Add self in waiting list */
        if (!(a2dp_dev->avrcp_flag & BT_SINK_SRV_AVRCP_MUST_PLAY_RING_TONE_FLAG)) {
            bt_sink_srv_a2dp_add_waitinglist(handle);
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
            if ((BT_SINK_SRV_INTERRUPT_OPERATION_PAUSE_MUSIC == ue_config.suspend_config.suspend_operation)
                && (BT_AVRCP_STATUS_PLAY_PLAYING == a2dp_dev->avrcp_status)) {
                bt_sink_srv_music_pause_remote_music(a2dp_dev);
                if (ue_config.suspend_config.will_suspend_resume) {
                    BT_SINK_SRV_SET_FLAG(a2dp_dev->flag, BT_SINK_SRV_MUSIC_FLAG_A2DP_INTERRUPT);
                }
            }
#else
            ret = bt_sink_srv_music_pause_remote_music(a2dp_dev);
#endif
        }
    }
    bt_sink_srv_mutex_unlock();
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
    bt_sink_srv_report_id("[sink][music][a2dp] suspend--hd: ret:0x%x, will_suspend_resume:0x%x, suspend_operation:0x%x, flag:0x%x", 4,
                          ret, ue_config.suspend_config.will_suspend_resume, ue_config.suspend_config.suspend_operation, a2dp_dev->flag);
#else
    bt_sink_srv_report_id("[sink][music][a2dp] suspend--ret:0x%x", 1, ret);
#endif
}


void bt_sink_srv_a2dp_reject(audio_src_srv_handle_t *handle)
{
    bt_sink_srv_mutex_lock();
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_PSE_HD, (void *)handle);
    const audio_src_srv_handle_t *rej_handle = audio_src_srv_get_runing_pseudo_device();

    bt_utils_assert(dev && rej_handle && handle);
    bt_sink_srv_report_id("[sink][music][a2dp] reject(s)--hd: 0x%x, type: %d, dev_id: 0x%x-0x%x", 4,
                          handle, handle->type, rej_handle->dev_id, handle->dev_id);
    
    if (dev->flag & BT_SINK_SRV_MUSIC_NEED_TO_RESPONSE_A2DP_START) {
        bt_a2dp_start_streaming_response(dev->a2dp_hd, true);
        BT_SINK_SRV_REMOVE_FLAG(dev->flag, BT_SINK_SRV_MUSIC_NEED_TO_RESPONSE_A2DP_START);
    }

    bt_gap_connection_handle_t gap_hd = bt_sink_srv_cm_get_gap_handle(&(dev->dev_addr));
    bt_utils_assert(gap_hd);
    bt_sink_srv_music_set_music_enable(gap_hd, BT_AVM_ROLE_AWS_MCE_AGENT, false);
    bt_sink_srv_music_state_machine_handle(dev, BT_SINK_SRV_MUSIC_EVT_REJECT, NULL);

    if (rej_handle->dev_id == handle->dev_id) {
        BT_SINK_SRV_SET_FLAG(dev->flag, BT_SINK_SRV_MUSIC_FLAG_WAIT_LIST_SINK_PLAY);
        audio_src_srv_add_waiting_list(handle);
    } else {
#ifdef AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
        bt_sink_srv_config_t ue_config = {0};
        bt_sink_srv_state_manager_get_reject_config_t get_reject_config = {0};
        get_reject_config.type = BT_SINK_SRV_DEVICE_EDR;
        bt_sink_srv_memcpy(&(get_reject_config.address),&(dev->dev_addr),sizeof(bt_bd_addr_t));
        if (BT_STATUS_SUCCESS == bt_sink_srv_state_manager_get_config(BT_SINK_SRV_GET_REJECT_CONFIG, 
                                            (void*)(&get_reject_config),
                                            &ue_config)) {
            bt_sink_srv_a2dp_add_waitinglist(dev->handle);
            if ((BT_SINK_SRV_INTERRUPT_OPERATION_PAUSE_MUSIC == ue_config.reject_config.reject_operation)
                && (BT_AVRCP_STATUS_PLAY_PLAYING == dev->avrcp_status)) {
                bt_sink_srv_music_pause_remote_music(dev);
                if (ue_config.reject_config.will_reject_resume) {
                    BT_SINK_SRV_SET_FLAG(dev->flag, BT_SINK_SRV_MUSIC_FLAG_A2DP_INTERRUPT);
                }
            }
        }
#else
        bt_sink_srv_music_pause_remote_music(dev);
#endif
    }

    /* Notify state machine reject reason */
    bt_sink_srv_mutex_unlock();
}

void bt_sink_srv_a2dp_create_pse_handle(void)
{
    int32_t i = 0;

    for (i = 0; i < BT_SINK_SRV_A2DP_PSEUDO_COUNT + BT_SINK_SRV_MCE_A2DP_PSEUDO_COUNT; ++i) {
        g_a2dp_pse_hd[i].hd = audio_src_srv_construct_handle(AUDIO_SRC_SRV_PSEUDO_DEVICE_A2DP);
    }
}

void bt_sink_srv_a2dp_destroy_pse_handle(void)
{
    int32_t i = 0;

    for (i = 0; i < BT_SINK_SRV_A2DP_PSEUDO_COUNT + BT_SINK_SRV_MCE_A2DP_PSEUDO_COUNT; ++i) {
        audio_src_srv_destruct_handle(g_a2dp_pse_hd[i].hd);
    }
}

int32_t bt_sink_srv_a2dp_set_volume(uint8_t volume, bt_sink_srv_music_device_t *sp_dev)
{
    bt_sink_srv_music_context_t *cntx = bt_sink_srv_music_get_context();;
    bt_sink_srv_music_device_t *run_dev = cntx->run_dev;
    uint8_t new_vol = bt_sink_srv_avrcp_get_volume_level(volume);
    uint8_t old_vol = cntx->vol_lev;
    int32_t ret = BT_STATUS_SUCCESS;
    bt_sink_srv_am_id_t ami_ret = 0;
    (void)ami_ret;

    bt_utils_assert(sp_dev && "sp_dev is NULL why?");

#ifdef MTK_AUDIO_SYNC_ENABLE
    if (run_dev && (run_dev->op & BT_SINK_SRV_MUSIC_OP_DRV_PLAY)) {

        bt_aws_mce_agent_state_type_t aws_state = BT_AWS_MCE_AGENT_STATE_NONE;
        uint32_t aws_handle = bt_sink_srv_aws_mce_get_handle(&(run_dev->dev_addr));
        if (aws_handle) {
            aws_state = bt_sink_srv_aws_mce_get_aws_state_by_handle(aws_handle);
        }
        if (aws_handle && (aws_state == BT_AWS_MCE_AGENT_STATE_ATTACHED)) {
            bt_sink_srv_a2dp_change_sync_volume(VOLUME_VALUE, 0, new_vol);
            return ret;
        }
    }
#endif

    ret = bt_sink_srv_music_set_nvdm_data(&(sp_dev->dev_addr), BT_SINK_SRV_MUSIC_DATA_VOLUME, &new_vol);

    if ((old_vol != new_vol) && run_dev && run_dev == sp_dev) {
        cntx->vol_lev = new_vol;
        if (run_dev->op & BT_SINK_SRV_MUSIC_OP_CODEC_OPEN) {
            bt_sink_srv_music_set_am_volume(cntx->vol_lev);
        } else {
            BT_SINK_SRV_SET_FLAG(run_dev->flag, BT_SINK_SRV_MUSIC_WAIT_SET_VOLUME);
        }
#ifdef __BT_AWS_MCE_A2DP_SUPPORT__
        if (run_dev && (run_dev->op & BT_SINK_SRV_MUSIC_OP_DRV_PLAY)) {
#ifdef MTK_BT_SPEAKER_ENABLE
            if (BT_SINK_SRV_MUSIC_MODE_BROADCAST == bt_sink_srv_music_get_spk_mode()) {
                if (new_vol > old_vol) {
                    bt_sink_srv_aws_mce_a2dp_send_eir(BT_SINK_SRV_AWS_MCE_A2DP_EVT_VOL_UP, (void *)(&(run_dev->a2dp_hd)));
                } else {
                    bt_sink_srv_aws_mce_a2dp_send_eir(BT_SINK_SRV_AWS_MCE_A2DP_EVT_VOL_DOWN, (void *)(&(run_dev->a2dp_hd)));
                }
            } else {
                bt_sink_srv_aws_mce_a2dp_send_eir(BT_SINK_SRV_AWS_MCE_A2DP_EVT_VOL_SYNC, (void *)(&(run_dev->a2dp_hd)));
            }
#else
            bt_sink_srv_aws_mce_a2dp_send_eir(BT_SINK_SRV_AWS_MCE_A2DP_EVT_VOL_SYNC, (void *)(&(run_dev->a2dp_hd)));
#endif
        }
#endif
    }

    bt_sink_srv_report_id("[sink][music][a2dp] set_volume-new_vol: %d, old_vol: %d, ami_ret: %d, ret: %d, vol: %d", 5,
                          new_vol, old_vol, ami_ret, ret, cntx->vol_lev);
    return ret;
}

static bool bt_sink_srv_a2dp_optimize_base_pta_by_asi(bt_sink_srv_music_optimize_pta_param_t *optimize_parameter)
{
    avm_direct_optimize_pta_param_t optimize_para = {0};
    bt_clock_t rec_clk_cur = {0};

    bt_sink_srv_memcpy(&rec_clk_cur, &optimize_parameter->cur_recv_clock, sizeof(bt_clock_t));
    avm_direct_bt_clock_normalize(&rec_clk_cur, &rec_clk_cur, BT_ROLE_SLAVE);
    optimize_para.base_asi = optimize_parameter->base_asi;
    optimize_para.frequence = optimize_parameter->frequence;
    optimize_para.cur_asi = optimize_parameter->cur_asi;
    optimize_para.latency_clock = optimize_parameter->latency_clock;
    optimize_para.cur_recv_clock = optimize_parameter->cur_recv_clock;
    optimize_para.pta = optimize_parameter->pta;
    avm_direct_optimize_base_pta(&optimize_para);
    return true;
}

static void bt_sink_srv_a2dp_sta_timer(uint32_t timer_id, uint32_t data)
{
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_sink_srv_music_set_sta_param_t sta_parameters = {0};
    sta_parameters.frequence = ctx->freq;
    sta_parameters.gap_handle = data;
    sta_parameters.packet_count = ctx->packet_count;
    sta_parameters.packet_list = ctx->media_data_list;
    sta_parameters.pta = &ctx->bt_clk;
    bt_sink_srv_a2dp_set_sta(&sta_parameters, false);
}

#define BT_INTRA_PRECISE_4_TICK (1250)
static bool bt_sink_srv_a2dp_get_duration(uint32_t nclk_base, uint32_t nclk_target, uint32_t *duration)
{
    uint32_t dur_nclk = nclk_target - nclk_base;
    bt_sink_srv_report_id("[sink][music][a2dp] get_duration, base:0x%08x, target:0x%08x", 2, nclk_base, nclk_target);
    if (dur_nclk & 0x80000000) {
        return false;
    }

    *duration = (dur_nclk * BT_INTRA_PRECISE_4_TICK) >> 2;
    return true;
}

#ifdef AIR_DCHS_MODE_ENABLE
extern void DCHS_bt_pka_get_master_native_clk(U32 *NativeClk);
#endif
static void bt_sink_srv_a2dp_set_sta(bt_sink_srv_music_set_sta_param_t *sta_parameters, bool need_timer)
{
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_clock_t *pta = sta_parameters->pta;
    bt_clock_t current_clock = {0};
    bt_clock_t min_sta_clock = {0};
    bt_clock_t max_sta_clock = {0};
    bt_clock_t min_dur_clock = {BT_SINK_SRV_A2DP_MIN_STA_DUR_TO_PLAY, 0};
    bt_clock_t max_dur_clock = {BT_SINK_SRV_A2DP_AVM_TIMER_MIN_DUR_TO_PLAY_BY_TICK, 0};

    //Just workround, set current bt clock to be packet rec time + 12 ticks.
    //bt_get_bt_clock(sta_parameters->gap_handle, &current_clock);
#ifdef AIR_DCHS_MODE_ENABLE
    // N-done is using native clock for DCHS products
    if(dchs_get_device_mode() != DCHS_MODE_SINGLE) {
        DCHS_bt_pka_get_master_native_clk((uint32_t*)&current_clock);
        bt_sink_srv_report_id("[sink][music][a2dp] DCHS get native clock:0x%x, cur_intra:0x%x",
                              2, current_clock.nclk, current_clock.nclk_intra);
    } else {
        bt_get_bt_clock(sta_parameters->gap_handle, &current_clock);
    }

#else
    bt_get_bt_clock(sta_parameters->gap_handle, &current_clock);
#endif
    avm_direct_bt_clock_add_duration(&max_sta_clock, &current_clock, &min_dur_clock, BT_ROLE_SLAVE);
    avm_direct_bt_clock_add_duration(&min_sta_clock, &current_clock, &max_dur_clock, BT_ROLE_SLAVE);
    uint32_t duration = 0;
    bool ret = bt_sink_srv_a2dp_get_duration(min_sta_clock.nclk, pta->nclk, &duration);
    duration = duration / 1000;
    bt_sink_srv_report_id("[sink][music][a2dp] set_sta, ret:%d, duration:0x%08x, pta-nclk:0x%08x, pta-intra:0x%04x, packet_count:%d", 5, ret, duration,
                          sta_parameters->pta->nclk, sta_parameters->pta->nclk_intra, sta_parameters->packet_count);
    if (ret && need_timer) {
        bt_timer_ext_stop(BT_SINK_SRV_STA_TIMER_ID);
        bt_timer_ext_start(BT_SINK_SRV_STA_TIMER_ID, sta_parameters->gap_handle,
                           duration, bt_sink_srv_a2dp_sta_timer);
    } else {
        if (ret) {
            return;
        }
        bt_timer_ext_stop(BT_SINK_SRV_STA_TIMER_ID);
#ifdef __BT_AWS_MCE_A2DP_SUPPORT__
        bt_sink_srv_aws_mce_a2dp_send_eir(BT_SINK_SRV_AWS_MCE_A2DP_EVT_PLAY_IND, (void *)(&(ctx->run_dev->a2dp_hd)));
#endif
        bt_sink_srv_report_id("[sink][music][a2dp] play_info, cur_nclk:0x%08x, cur_intra:0x%04x, pta-nclk:0x%08x, pta-intra:0x%04x, base_asi:0x%08x",
                              5, current_clock.nclk, current_clock.nclk_intra, pta->nclk, pta->nclk_intra, ctx->ts);
        bt_sink_srv_music_trigger_play(sta_parameters->gap_handle, ctx->run_dev, pta, ctx->ts, ctx->ts);
        ctx->packet_count = 0;
    }
}

static void bt_sink_srv_a2dp_handle_media_data_received_ind(uint32_t gap_handle, bt_sink_srv_music_data_info_t *media_info)
{
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_sink_srv_music_data_info_t *packet_list = ctx->media_data_list;
    bt_sink_srv_music_device_t *run_dev = ctx->run_dev;
    bt_clock_t recv_clk = {0};
    bt_clock_t original_pta = {0};
    if (!run_dev || run_dev->op & BT_SINK_SRV_MUSIC_OP_DRV_PLAY || !(run_dev->op & BT_SINK_SRV_MUSIC_OP_CODEC_OPEN)) {
        return;
    }
    uint32_t gap_hd = bt_sink_srv_cm_get_gap_handle(&(run_dev->dev_addr));
    if (gap_hd != gap_handle) {
        return;
    }

    bt_media_handle_t *med_hd = run_dev->med_handle.med_hd;
    media_info->asi = media_info->asi & BT_SINK_SRV_A2DP_ASI_MASK;
    bt_sink_srv_music_fill_recevied_media_data(media_info);
    bt_sink_srv_memcpy(&recv_clk, &media_info->clock, sizeof(bt_clock_t));
    bt_clock_t *clk_latency = avm_direct_get_sink_latency_by_tick();
    avm_direct_bt_clock_normalize(&recv_clk, &recv_clk, BT_ROLE_SLAVE);
    if (ctx->packet_count == 1) {
        avm_direct_bt_clock_normalize(&recv_clk, &recv_clk, BT_ROLE_SLAVE);
        avm_direct_bt_clock_add_duration(&ctx->bt_clk, &recv_clk, clk_latency, BT_ROLE_SLAVE);
        ctx->freq = med_hd->get_sampling_rate(med_hd);
        ctx->ts = media_info->asi;
        ctx->ratio = media_info->ratio;
        ctx->samples = media_info->samples;
    } else {
        bt_sink_srv_music_optimize_pta_param_t optimize_parameters;
        optimize_parameters.frequence = ctx->freq;
        optimize_parameters.base_asi = packet_list[0].asi;
        optimize_parameters.cur_asi = media_info->asi;
        optimize_parameters.latency_clock = clk_latency;
        optimize_parameters.cur_recv_clock = &media_info->clock;
        optimize_parameters.pta = &ctx->bt_clk;
        bt_sink_srv_memcpy(&original_pta, &ctx->bt_clk, sizeof(bt_clock_t));
        bt_sink_srv_a2dp_optimize_base_pta_by_asi(&optimize_parameters);
    }
    bt_sink_srv_music_set_sta_param_t sta_parameters;

    sta_parameters.frequence = ctx->freq;
    sta_parameters.gap_handle = gap_hd;
    sta_parameters.packet_count = ctx->packet_count;
    sta_parameters.packet_list = ctx->media_data_list;
    sta_parameters.pta = &ctx->bt_clk;
    bool need_timer = false;
    bt_sink_srv_report_id("[sink][music][a2dp] media_data_received_ind, base_asi:0x%08x, base_nclk:0x%08x, base_intra:0x%04x", 3,
                          ctx->ts, sta_parameters.pta->nclk, sta_parameters.pta->nclk_intra);
    if (ctx->bt_clk.nclk != original_pta.nclk) {
        need_timer = true;
    }
    bt_sink_srv_a2dp_set_sta(&sta_parameters, need_timer);
}

extern void bt_driver_set_no_retransmission_mode(uint16_t seq_num, uint16_t length);
void bt_sink_srv_a2dp_set_no_retransmission_mode(uint16_t seq_num, uint16_t length)
{
    bt_driver_set_no_retransmission_mode(seq_num, length);
}

#ifdef __BT_SINK_SRV_A2DP_AVM_DIRECT_SUPPORT__
bt_status_t bt_sink_srv_a2dp_reinitial_sync()
{
    bt_sink_srv_mutex_lock();
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_sink_srv_music_device_t *run_dev = ctx->run_dev;
    uint32_t flag = 0;
    (void)flag;

    if (run_dev) {
        flag = run_dev->flag;
        if ((!(run_dev->flag & BT_SINK_SRV_MUSIC_FLAG_REINITIAL_SYNC)) && (!(run_dev->flag & BT_SINK_SRV_MUSIC_FLAG_WAIT_AMI_OPEN_CODEC))) {
        BT_SINK_SRV_REMOVE_FLAG(run_dev->flag, BT_SINK_SRV_MUSIC_FLAG_ALC_REINITIAL_SYNC);
        BT_SINK_SRV_SET_FLAG(run_dev->flag, BT_SINK_SRV_MUSIC_FLAG_REINITIAL_SYNC);
#ifndef BT_SINK_SRV_IMPROVE_RESYNC
        uint32_t device_list[BT_SINK_SRV_MUISC_DEV_COUNT];
        uint32_t device_count = 0;
        bt_sink_srv_music_get_waiting_list_devices(device_list, &device_count);
        bt_sink_srv_music_device_waiting_list_operation(device_list, device_count, false);

#ifdef AIR_LE_AUDIO_ENABLE
        bt_sink_srv_cap_am_disable_waiting_list();
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
        bt_ull_le_am_operate_waiting_list_for_a2dp_resync(false);
#endif
#endif
        bt_sink_srv_music_state_machine_handle(run_dev, BT_A2DP_SUSPEND_STREAMING_IND, NULL);
#ifndef BT_SINK_SRV_IMPROVE_RESYNC
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
        bt_ull_le_am_operate_waiting_list_for_a2dp_resync(true);
#endif
        bt_sink_srv_music_device_waiting_list_operation(device_list, device_count, true);
#endif
        } else if (run_dev->flag & BT_SINK_SRV_MUSIC_FLAG_WAIT_AMI_OPEN_CODEC) {
            BT_SINK_SRV_SET_FLAG(run_dev->op, BT_SINK_SRV_MUSIC_WAITING_REINITIAL_SYNC);
            ret = BT_STATUS_FAIL;
        }
    }
    bt_sink_srv_report_id("[sink][music][a2dp] To reinitial sync, run_dev:0x%08x, flag:0x%08x, ret:0x%08x",
        3, run_dev, flag, ret);
    bt_sink_srv_mutex_unlock();
    return ret;
}

bt_status_t bt_sink_srv_a2dp_get_volume(bt_bd_addr_t *bd_addr, uint32_t *volume)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_sink_srv_music_device_t *run_dev = ctx->run_dev;
    bt_bd_addr_t *remote_addr = bd_addr;
    bt_status_t ret = BT_STATUS_FAIL;

    if (!remote_addr) {
        if (run_dev) {
            remote_addr = &(run_dev->dev_addr);
        }
    }

    if (remote_addr) {
        if (role == BT_AWS_MCE_ROLE_NONE || role == BT_AWS_MCE_ROLE_AGENT) {
            ret = bt_sink_srv_music_get_nvdm_data(remote_addr, BT_SINK_SRV_MUSIC_DATA_VOLUME, volume);
        } else {
            ret = BT_STATUS_SUCCESS;
            *volume = ctx->vol_lev;
        }
    }

    bt_sink_srv_report_id("[sink][music][a2dp] get_volume, volume:0x%08x, role:0x%02x, remote_addr:0x%08x, run_dev:0x%08x", 4, *volume, role, remote_addr, run_dev);

    return ret;
}



#endif

void bt_sink_srv_a2dp_add_waitinglist(audio_src_srv_handle_t *handle)
{
    audio_src_srv_add_waiting_list(handle);
    bt_sink_srv_report_id("[sink][music][a2dp] add_waitinglist, handle: 0x%08x, flag: 0x%08x, ret:0x%x",
        2, handle, handle->flag);
}

bt_status_t bt_sink_srv_a2dp_get_codec_parameters(bt_sink_srv_a2dp_basic_config_t *config_data)
{
    bt_sink_srv_music_device_t *run_dev = bt_sink_srv_music_get_context()->run_dev;
    bt_status_t ret = BT_STATUS_FAIL;
    bt_utils_assert(config_data);
    config_data->status = 0x01;
    if (run_dev) {
        bt_gap_connection_handle_t gap_hd = bt_sink_srv_cm_get_gap_handle(&(run_dev->dev_addr));
        extern uint8_t bt_avm_get_rate(uint32_t handle);
        config_data->enable_3M = bt_avm_get_rate(gap_hd);
        config_data->latency = avm_direct_get_sink_latency();
        config_data->codec_type = run_dev->codec.type;
        config_data->min_bit_pool = 0xff;
        config_data->max_bit_pool = 0xff;
        config_data->bit_rate = 0xffffffff;
        if (config_data->codec_type == BT_A2DP_CODEC_SBC) {
            config_data->min_bit_pool = run_dev->codec.codec.sbc.min_bitpool;
            config_data->max_bit_pool = run_dev->codec.codec.sbc.max_bitpool;
        } else if (config_data->codec_type == BT_A2DP_CODEC_AAC) {
            bt_a2dp_aac_codec_t *aac_codec = &(run_dev->codec.codec.aac);
            config_data->bit_rate = (aac_codec->br_h << 16 | aac_codec->br_m << 8 | aac_codec->br_l);
        }
        config_data->status = 0x00;
        ret = BT_STATUS_SUCCESS;
    }
    bt_sink_srv_report_id("[sink][music][a2dp] get_codec_parameters status:0x%02x, 3M:0x%02x, latency:%d, codec_type:0x%02x, min:0x%02x, max:0x%02x, bit_rate:0x%08x", 7,
                          config_data->status, config_data->enable_3M, config_data->latency, config_data->codec_type, config_data->min_bit_pool, config_data->max_bit_pool, config_data->bit_rate);

    return ret;
}

bt_status_t bt_sink_srv_a2dp_get_codec_parameters_ext(bt_sink_srv_a2dp_basic_config_2_t *config_data)
{
    bt_sink_srv_music_device_t *run_dev = bt_sink_srv_music_get_context()->run_dev;
    bt_status_t ret = BT_STATUS_FAIL;
    bt_utils_assert(config_data);
    config_data->status = 0x01;
    if (run_dev) {
        config_data->status = 0x00;
#ifdef AIR_BT_A2DP_VENDOR_2_ENABLE
        config_data->lossless_enable = (run_dev->codec.codec.vendor.value[3]&0x80) ? 1 : 0;
#endif
#ifdef AIR_BT_A2DP_VENDOR_CODEC_SUPPORT
        config_data->codec_id = run_dev->codec.codec.vendor.codec_id;
        config_data->vendor_id = run_dev->codec.codec.vendor.vendor_id;
#endif
        ret = BT_STATUS_SUCCESS;
    }
    bt_sink_srv_report_id("[sink][music][a2dp] get_codec_parameters_ext status:0x%02x, lossless_enable:0x%02x, vendor_id:0x%08x, codec_id:0x%04x", 4,
                          config_data->status, config_data->lossless_enable, config_data->vendor_id, config_data->codec_id);

    return ret;
}

bt_status_t bt_sink_srv_a2dp_get_codec_type(bt_sink_srv_a2dp_codec_name_t *codec_data)
{
    bt_sink_srv_music_device_t *run_dev = bt_sink_srv_music_get_context()->run_dev;
    bt_status_t ret = BT_STATUS_FAIL;
    if (run_dev) {
        if(run_dev->codec.type == BT_A2DP_CODEC_SBC) {
            strcpy((char*)codec_data->name, "SBC\n");
            codec_data->name_len = 4;
        } else if (run_dev->codec.type == BT_A2DP_CODEC_AAC) {
            strcpy(codec_data->name, "AAC\n");
            codec_data->name_len = 4;
        } else if (run_dev->codec.type == BT_A2DP_CODEC_VENDOR) {
            uint32_t vendor_id = run_dev->codec.codec.vendor.vendor_id;
            uint16_t codec_id = run_dev->codec.codec.vendor.codec_id;
            if (vendor_id == 0x0000012d && codec_id == 0x00aa) {
                strcpy(codec_data->name, "LDAC\n");
                codec_data->name_len = 5;
            } else if (vendor_id == 0x0000053a && codec_id == 0x4C35) {
                strcpy(codec_data->name, "LHDC\n");
                codec_data->name_len = 5;
            } else if (vendor_id == 0x000008a9 && codec_id == 0x0001) {
                strcpy(codec_data->name, "LC3+\n");
                codec_data->name_len = 5;
            }
        }
        ret = BT_STATUS_SUCCESS;
    } else {
        strcpy(codec_data->name, "None\n");
        codec_data->name_len = 5;
    }
    return ret;
}
bt_status_t  bt_sink_srv_a2dp_cm_callback_handler(bt_cm_profile_service_handle_t type, void *data)
{
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_status_t status = BT_STATUS_SUCCESS;
    uint8_t *address = NULL;
    bt_sink_srv_report_id("[sink][music][a2dp] cm_callback_handler type:0x%02x", 1, type);
    switch (type) {
        case BT_CM_PROFILE_SERVICE_HANDLE_POWER_ON: {
            //firstly init context
            bt_sink_srv_music_init_context();
            //secondary init a2dp profile
            bt_sink_srv_a2dp_init();
#ifdef MTK_AUDIO_SYNC_ENABLE
            bt_sink_srv_register_sync_callback(BT_SINK_SRV_A2DP_VOLUME_TYPE, bt_sink_srv_music_volume_sync_callback);
#ifdef AIR_A2DP_SYNC_STOP_ENABLE
            extern void bt_sink_srv_music_stop_sync_callback(bt_sink_srv_sync_status_t status, bt_sink_srv_sync_callback_data_t *sync_data);
            bt_sink_srv_register_sync_callback(BT_SINK_SRV_MUSIC_STOP_TYPE, bt_sink_srv_music_stop_sync_callback);
#endif
#endif
#ifdef __BT_AWS_MCE_A2DP_SUPPORT__
            bt_sink_srv_aws_mce_a2dp_init();
#endif
            break;
        }
        case BT_CM_PROFILE_SERVICE_HANDLE_POWER_OFF: {
                    BT_SINK_SRV_SET_FLAG(ctx->context_flag, BT_SINK_SRV_CNTX_FLAG_POWER_OFF);
            if (!(ctx->run_dev && (ctx->run_dev->flag & BT_SINK_SRV_MUSIC_FLAG_WAIT_AMI_OPEN_CODEC))) {
                if (ctx->a2dp_aid != BT_SINK_SRV_INVALID_AID) {
                    bt_sink_srv_ami_audio_close(ctx->a2dp_aid);
                    ctx->a2dp_aid = BT_SINK_SRV_INVALID_AID;
                }
#if defined(__BT_AWS_MCE_A2DP_SUPPORT__)
                if (ctx->aws_aid != BT_SINK_SRV_INVALID_AID) {
                    bt_sink_srv_ami_audio_close(ctx->aws_aid);
                    ctx->aws_aid = BT_SINK_SRV_INVALID_AID;
            }
#endif
#ifdef MTK_AUDIO_SYNC_ENABLE
            bt_sink_srv_deregister_sync_callback(BT_SINK_SRV_A2DP_VOLUME_TYPE);
#endif
            }
            break;
        }
        case BT_CM_PROFILE_SERVICE_HANDLE_CONNECT: {
            bt_sink_srv_mutex_lock();
            address = (uint8_t *)data;
            bt_utils_assert(address);
            bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
            bt_sink_srv_report_id("[sink][music][a2dp] connect role:0x%x,a2dp addr :0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",
                7, role, address[0], address[1], address[2],address[3], address[4], address[5]);
            if (role != BT_AWS_MCE_ROLE_AGENT && role != BT_AWS_MCE_ROLE_NONE) {
                status = BT_STATUS_UNSUPPORTED;
                bt_sink_srv_mutex_unlock();
                break;
            }
            if (ctx->run_dev && bt_sink_srv_memcmp(ctx->run_dev->dev_addr, address, sizeof(bt_bd_addr_t)) == 0) {
                status = BT_STATUS_FAIL;
                bt_sink_srv_mutex_unlock();
                break;
            }
            uint32_t hd = 0;
            status = bt_a2dp_connect(&hd, (const bt_bd_addr_t *)address, BT_A2DP_SINK);
            if (BT_STATUS_SUCCESS == status) {
                bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_ADDR_A2DP, (void *)address);
                if (!dev) {
                    dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_ADDR_AVRCP, (void *)address);
                }

                if (!dev) {
                    dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_UNUSED, NULL);
                    bt_utils_assert(dev && "Error: a2dp dev NULL");

                    dev->handle = bt_sink_srv_a2dp_alloc_pseudo_handle();
                    dev->handle->type = AUDIO_SRC_SRV_PSEUDO_DEVICE_A2DP;
                    dev->handle->dev_id = bt_sink_srv_music_convert_btaddr_to_devid((bt_bd_addr_t *)address);
                    dev->handle->priority = AUDIO_SRC_SRV_PRIORITY_NORMAL;
                }
                dev->a2dp_hd = hd;
                dev->role = BT_A2DP_SINK;
                BT_SINK_SRV_SET_FLAG(dev->flag, BT_SINK_SRV_MUSIC_INITIAL_A2DP_BY_DEVICE
                                              | BT_SINK_SRV_MUSIC_FLAG_DEV_TO_CON_AVRCP);
                bt_sink_srv_memcpy(&(dev->dev_addr), address, sizeof(bt_bd_addr_t));
            }
            bt_sink_srv_mutex_unlock();
            break;
        }
        case BT_CM_PROFILE_SERVICE_HANDLE_DISCONNECT: {
            bt_sink_srv_mutex_lock();
            address = (uint8_t *)data;
            bt_utils_assert(address);
            bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
            bt_sink_srv_report_id("[sink][music][a2dp] disconnect role:0x%x, a2dp addr :0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",
                7, role, address[0], address[1], address[2],
                                  address[3], address[4], address[5]);
            if (role != BT_AWS_MCE_ROLE_AGENT && role != BT_AWS_MCE_ROLE_NONE) {
                bt_sink_srv_mutex_unlock();
                break;
            }
            bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_ADDR_A2DP, (void *)address);
            if (dev) {
                BT_SINK_SRV_REMOVE_FLAG(dev->flag, BT_SINK_SRV_MUSIC_FLAG_DEV_TO_CON_AVRCP);
                if (dev->conn_bit & BT_SINK_SRV_MUSIC_A2DP_CONN_BIT) {
                    bt_timer_ext_stop(BT_SINK_SRV_AVRCP_CONNECTION_TIMER_ID);
                    status = bt_a2dp_disconnect(dev->a2dp_hd);
                    if (BT_SINK_SRV_MUSIC_FLAG_WAIT_AVRCP_CONN_RESULT & dev->flag) {
                        BT_SINK_SRV_SET_FLAG(dev->op, BT_SINK_SRV_MUSIC_INIT_DISCONNNECT_AVRCP_TIMER_FLAG);
                    }
                } else {
                    status = BT_STATUS_CONNECTION_NOT_FOUND;
                }
            } else {
                status = BT_STATUS_CONNECTION_NOT_FOUND;
            }
            bt_sink_srv_mutex_unlock();
            break;
        }
        default:
            break;
    }
    return status;
}

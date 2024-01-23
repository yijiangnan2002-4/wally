/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
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
#include "bt_spp.h"
#include "bt_ull_aws_mce.h"
#include "bt_ull_utility.h"
#include "bt_device_manager.h"
#include "bt_connection_manager_internal.h"
#include "bt_utils.h"




void bt_ull_aws_mce_packet_callback(bt_aws_mce_report_info_t *param)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    if (NULL == param) {
        ull_report_error("[ULL][AWS] packet callback param is null !!!", 0);
        return;
    }
    if (BT_AWS_MCE_REPORT_MODULE_ULL != param->module_id) {
        ull_report_error("[ULL][AWS] Packet callback module is not ULL!!!", 0);
        return;
    }
    if (sizeof(bt_ull_aws_mce_eir_info_t) == param->param_len) {
        bt_ull_aws_mce_eir_info_t *eir_info = (bt_ull_aws_mce_eir_info_t *)param->param;
        ull_report("[ULL][AWS] Receive AWS MCE event:0x%x", 1, eir_info->event);
        switch (eir_info->event) {
            case BT_ULL_AWS_MCE_EVT_CONNECTION: {
                memcpy(&(ctx->remode_device), &(eir_info->param.aws_connection.remode_device), sizeof(ctx->remode_device));
                ctx->dl_speaker.original_volume.vol_left = eir_info->param.aws_connection.dl_streaming.volume.vol_left;
                ctx->dl_speaker.original_volume.vol_right = eir_info->param.aws_connection.dl_streaming.volume.vol_right;
                ctx->dl_speaker.is_mute = eir_info->param.aws_connection.dl_streaming.is_mute;
                ctx->dl_speaker.is_streaming = eir_info->param.aws_connection.dl_streaming.is_streaming;

                ctx->ul_microphone.original_volume.vol_left = eir_info->param.aws_connection.ul_streaming.volume.vol_left;
                ctx->ul_microphone.original_volume.vol_right = eir_info->param.aws_connection.ul_streaming.volume.vol_right;
                ctx->ul_microphone.is_mute = eir_info->param.aws_connection.ul_streaming.is_mute;
                ctx->ul_microphone.is_streaming = eir_info->param.aws_connection.ul_streaming.is_streaming;

                ctx->is_ull_connected = eir_info->param.aws_connection.is_ull_connected;
                ctx->codec_type = eir_info->param.aws_connection.codec_type;
                ctx->dl_latency = eir_info->param.aws_connection.dl_latency;
                ctx->ul_latency = eir_info->param.aws_connection.ul_latency;
                /*update partner mic frame size*/
                bt_ull_set_mic_frame_size(eir_info->param.aws_connection.ul_frame_size);

                if (ctx->dl_latency <= BT_ULL_DOWNLINK_LIMITATION_MULTILINK) {
                    /* ull profile connected we should disable page scan && inquiry scan due to bandwidth limiation */
                    bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_DISABLE, BT_CM_COMMON_TYPE_DISABLE);
                }
                /* set codec info */
                if (BT_ULL_OPUS_CODEC == ctx->codec_type) {
                    /* save peer encode codec info (for our decodec) */
                    memset(&(ctx->codec_cap), 0x00, sizeof(bt_a2dp_codec_capability_t));
                    ctx->codec_cap.type = BT_ULL_CODEC_AIRO_CELT;
                    ctx->codec_cap.sep_type = BT_A2DP_SINK;
                } else {
                    bt_utils_assert(0 && "unknown codec type");
                }
                bt_ull_aws_mce_handle_connection(ctx->is_ull_connected);
                if (ctx->dl_speaker.is_streaming || ctx->ul_microphone.is_streaming) {
                    bt_ull_am_play();
                }
                break;
            }
            case BT_ULL_AWS_MCE_EVT_PLAY: {
                ctx->dl_speaker.original_volume.vol_left = eir_info->param.aws_play.dl_streaming.volume.vol_left;
                ctx->dl_speaker.original_volume.vol_right = eir_info->param.aws_play.dl_streaming.volume.vol_right;
                ctx->dl_speaker.is_mute = eir_info->param.aws_play.dl_streaming.is_mute;
                ctx->dl_speaker.is_streaming = eir_info->param.aws_play.dl_streaming.is_streaming;

                ctx->ul_microphone.original_volume.vol_left = eir_info->param.aws_play.ul_streaming.volume.vol_left;
                ctx->ul_microphone.original_volume.vol_right = eir_info->param.aws_play.ul_streaming.volume.vol_right;
                ctx->ul_microphone.is_mute = eir_info->param.aws_play.ul_streaming.is_mute;
                ctx->ul_microphone.is_streaming = eir_info->param.aws_play.ul_streaming.is_streaming;

                ctx->dl_latency = eir_info->param.aws_play.dl_latency;
                ctx->ul_latency = eir_info->param.aws_play.ul_latency;

                if (ctx->dl_latency <= BT_ULL_DOWNLINK_LIMITATION_MULTILINK) {
                    /* ull profile connected we should disable page scan && inquiry scan due to bandwidth limiation */
                    bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_DISABLE, BT_CM_COMMON_TYPE_DISABLE);
                }
                if (ctx->dl_speaker.is_streaming || ctx->ul_microphone.is_streaming) {
                    bt_ull_am_play();
                }
                break;
            }
            case BT_ULL_AWS_MCE_EVT_STOP: {
                ctx->dl_speaker.is_streaming = eir_info->param.aws_stop.dl_is_streaming;
                ctx->ul_microphone.is_streaming = eir_info->param.aws_stop.ul_is_streaming;
                //if (false == ctx->dl_speaker.is_streaming) {
                bt_ull_am_stop();
                //}
                break;
            }
            case BT_ULL_AWS_MCE_EVT_VOL_SYNC: {
                ull_report("[ULL][AWS] EVT_VOL_SYNC, old_vol:0x%x, new_vol:0x%x, old_mute:0x%x, new_mute:0x%x!!!", 4,
                           ctx->dl_speaker.volume.vol_left, eir_info->param.aws_vol_sync.vol_lev, ctx->dl_speaker.is_mute, eir_info->param.aws_vol_sync.is_mute);
                /* set volume */
                if (ctx->dl_speaker.volume.vol_left != eir_info->param.aws_vol_sync.vol_lev) {
                    bt_ull_am_set_volume(eir_info->param.aws_vol_sync.vol_lev);
                }
                if (ctx->dl_speaker.is_mute != eir_info->param.aws_vol_sync.is_mute) {
                    bt_ull_am_set_mute(eir_info->param.aws_vol_sync.is_mute);
                }
                break;
            }
            /* agent receive partner report info */
            case BT_ULL_AWS_MCE_REPORT_VOL_SYNC: {
                ull_report("[ULL][AWS] REPORT_VOL_SYNC, current_vol:0x%x, req_if:0x%x, req_action:0x%x, req_vol:0x%x!!!", 4,
                           ctx->dl_speaker.volume.vol_left, eir_info->param.aws_vol_report.streaming.streaming_interface, eir_info->param.aws_vol_report.action, eir_info->param.aws_vol_report.volume);
                if (BT_AWS_MCE_ROLE_AGENT == bt_device_manager_aws_local_info_get_role()) {
                    /* sync volume operation to service */
                    if (BT_SPP_INVALID_HANDLE != ctx->spp_handle && ctx->is_ull_connected) {
                        bt_ull_req_t request;
                        memset(&request, 0x00, sizeof(request));
                        request.event = BT_ULL_EVENT_VOLUME_ACTION;
                        bt_ull_volume_t *vol = (bt_ull_volume_t *) & (request.vol_action);
                        memcpy(vol, &(eir_info->param.aws_vol_report), sizeof(bt_ull_volume_t));
                        bt_ull_send_data(ctx->spp_handle, (uint8_t *)&request, sizeof(request));
                    }
                }
                break;
            }
            default:
                break;
        }
    } else {
        ull_report_error("[ULL][AWS] Packet length is wrong!!!", 0);
    }
}


void bt_ull_aws_mce_send_eir(void *param, uint32_t param_len, bool urgent)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_aws_mce_report_info_t aws_report = {0};

    aws_report.module_id = BT_AWS_MCE_REPORT_MODULE_ULL;
    aws_report.is_sync = false;
    aws_report.sync_time = 0;
    aws_report.param_len = param_len;
    aws_report.param = param;
    if (urgent) {
        ret = bt_aws_mce_report_send_urgent_event(&aws_report);
    } else {
        ret = bt_aws_mce_report_send_event(&aws_report);
    }

    bt_ull_aws_mce_eir_info_t *eir_info = (bt_ull_aws_mce_eir_info_t *)param;
    ull_report("[ULL][AWS] send_eir--event: 0x%08x, param:0x%08x, urgent:0x%x, ret:0x%x", 4,
               eir_info->event, param, urgent, ret);
}


void bt_ull_aws_mce_handle_connection(bool is_ull_connected)
{
    bt_ull_context_t *ctx = bt_ull_get_context();
    ull_report("[ULL][AWS] aws_mce_handle_connection, is_ull_connected: 0x%x, ctx->is_ull_connected:0x%x, dl_latency:%d, ul_latency:%d", 4,
               is_ull_connected, ctx->is_ull_connected, ctx->dl_latency, ctx->ul_latency);
    bt_utils_assert(BT_ULL_ROLE_CLIENT == ctx->ull_role);
    /* 1. set share buffer address to controller */
    if (is_ull_connected) {
        bt_ull_update_audio_buffer(ctx->ull_role, BT_ULL_EARBUDS_CLIENT);
        if (ctx->dl_latency <= BT_ULL_DOWNLINK_LIMITATION_MULTILINK) {
            /* ull profile connected we shuld disable page scan && inquiry scan due to bandwidth limiation */
            bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_DISABLE, BT_CM_COMMON_TYPE_DISABLE);
        }
        if (NULL == ctx->am_handle) {
            ctx->am_handle = bt_ull_am_init();
        }
        bt_utils_assert(NULL != ctx->am_handle && "fail create pseudo device!");
        bt_ull_init_transimter(ctx->codec_type, BT_ULL_MIC_TRANSMITTER);
    } else {
        if (ctx->is_ull_connected) {
            /* partner disconnect */
            /* enable page scan after ull disconnect */
            bt_cm_write_scan_mode(BT_CM_COMMON_TYPE_UNKNOW, BT_CM_COMMON_TYPE_ENABLE);
            ctx->is_ull_connected = false;
            ctx->spp_handle = BT_SPP_INVALID_HANDLE;
            ctx->ul_microphone.is_streaming = false;
            /* close transmitter */
            if (ctx->ul_microphone.is_transmitter_start) {
                /* stop transmitter & disable sidetone */
                am_audio_side_tone_disable();
                bt_ull_stop_transmitter(&ctx->ul_microphone);
            }
            /* clear last connection info */
            bt_ull_clear_last_connected_device_info();
            bt_ull_am_deinit();
        }
    }
}

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

#include "bt_sink_srv_music.h"
#include "hal_audio.h"
#include "hal_audio_message_struct.h"
#include "bt_sink_srv.h"
#include "bt_sink_srv_aws_mce_a2dp.h"
#include "bt_aws_mce.h"
#include "audio_src_srv.h"
#include "bt_sink_srv_utils.h"
#include "bt_sink_srv_a2dp.h"
#include "bt_sink_srv_avrcp.h"
#include "bt_sink_srv_state_notify.h"
#include "bt_sink_srv_ami.h"
#include "bt_sink_srv_aws_mce.h"
#include "avm_direct.h"
#include "bt_aws_mce_report.h"
#ifdef __BT_SINK_SRV_A2DP_AVM_DIRECT_SUPPORT__
#include "bt_avm.h"
#endif
#include "bt_device_manager.h"
#include "bt_device_manager_internal.h"
#include "bt_iot_device_white_list.h"
#ifdef AIR_MCSYNC_SHARE_ENABLE
#include "bt_mcsync_share_internal_msg.h"
#include "bt_mcsync_share.h"
#endif
#ifdef AIR_LE_AUDIO_ENABLE
#include "bt_sink_srv_le_cap_stream.h"
#endif
#include "bt_utils.h"
#include "bt_connection_manager_internal.h"
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
#include "bt_ull_le_audio_manager.h"
#endif

#define BT_SINK_SRV_AWS_MCE_A2DP_PLAY_INFO_MAGIC    (0xffff0000)
#define BT_SINK_SRV_AWS_MCE_A2DP_UPDATE_BASE_MAGIC  (0xfffe0000)

extern void bt_sink_srv_music_a2dp_common_ami_hdr(bt_sink_srv_am_id_t aud_id, bt_sink_srv_am_cb_msg_class_t msg_id, bt_sink_srv_am_cb_sub_msg_t sub_msg, void
                                                  *param);
/* Static function declare */
static int32_t bt_sink_srv_aws_mce_a2dp_handle_disconnect_ind(uint32_t handle, bool free_handle);
static bt_status_t bt_sink_srv_aws_mce_a2dp_send_action(uint8_t action_type, uint8_t action_value, void *param, uint32_t param_length);
static void bt_sink_srv_aws_mce_a2dp_set_sta(uint32_t gap_handle);
static void bt_sink_srv_aws_mce_a2dp_handle_new_play_ind(uint32_t handle, bt_sink_srv_aws_mce_a2dp_base_eir_t *a2dp_eir);
#ifdef AIR_A2DP_SYNC_STOP_ENABLE
extern void bt_sink_srv_music_sync_stop_done_handler(hal_audio_event_t event, void *data);
#endif
static void bt_sink_srv_aws_mce_init_basic_eir(bt_sink_srv_aws_mce_a2dp_base_eir_t *eir_buf);

void bt_sink_srv_aws_mce_a2dp_app_report_callback(bt_aws_mce_report_info_t *para)
{
    bt_sink_srv_aws_mce_a2dp_action_t *action_data = (bt_sink_srv_aws_mce_a2dp_action_t *)para->param;
    uint8_t type = action_data->action_type, val = action_data->action_value;
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_bd_addr_t *device_p = bt_sink_srv_cm_get_aws_connected_device();
    bt_sink_srv_music_device_t *a2dp_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_ADDR_AVRCP, (void *)(device_p));
    if (!a2dp_dev) {
        a2dp_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_ADDR_A2DP, (void *)(device_p));
    }

    bt_sink_srv_report_id("[sink][music][mce_a2dp] app_report-module: 0x%0x, data_len:0x%08x, type:0x%02x, len: 0x%02x, val: 0x%02x, a2dp_dev:0x%x", 6,
                          para->module_id, para->param_len,
                          type, action_data->action_length, val, a2dp_dev);

    if (type == BT_SINK_SRV_MUSIC_TYPE_CONTROL) {
#ifdef AIR_BT_A2DP_VENDOR_CODEC_SUPPORT
        if (BT_SINK_SRV_MUSIC_TYPE_SYNC_VENDOR_CODEC_CONFIG == val) {
            bt_sink_srv_a2dp_enable_vendor_codec(action_data->data[0]);
            return;
        }
#endif
        if (a2dp_dev == NULL) {
            return;
        }

        switch (val) {
            case BT_SINK_SRV_MUSIC_TYPE_CONTROL_VAL_PLAY: {
                bt_sink_srv_avrcp_play_music(a2dp_dev);
                break;
            }

            case BT_SINK_SRV_MUSIC_TYPE_CONTROL_VAL_PAUSE: {
                bt_sink_srv_music_pause_remote_music(a2dp_dev);
                break;
            }
            case BT_SINK_SRV_MUSIC_TYPE_CONTROL_VAL_PLAY_PAUSE: {
                bt_sink_srv_music_decide_play_pause_action(a2dp_dev);
                break;
            }

            case BT_SINK_SRV_MUSIC_TYPE_CONTROL_VAL_PRE_TRC: {
                bt_sink_srv_avrcp_change_track(a2dp_dev->avrcp_hd, true);
                break;
            }

            case BT_SINK_SRV_MUSIC_TYPE_CONTROL_VAL_NEXT_TRC: {
                bt_sink_srv_avrcp_change_track(a2dp_dev->avrcp_hd, false);
                break;
            }

            case BT_SINK_SRV_MUSIC_TYPE_CONTROL_TRIGGER_REINITIAL_SYNC: {
                if (ctx->run_dev) {
#ifdef AIR_A2DP_SYNC_STOP_ENABLE
                    if (ctx->run_dev->handle->substate == BT_SINK_SRV_MUSIC_TRANSIENT_STATE_CLEAR_CODEC) {
                        break;
                    }
#endif
                    if (ctx->run_dev->op & BT_SINK_SRV_MUSIC_OP_DRV_PLAY) {
                        bt_sink_srv_a2dp_reinitial_sync();
                    }
                }
                break;
            }

            case BT_SINK_SRV_MUSIC_TYPE_CONTROL_TRIGGER_ALC: {
                if (ctx->run_dev && (BT_SINK_SRV_MUSIC_TRANSIENT_STATE_CLEAR_CODEC != ctx->run_dev->handle->substate)) {
                    bool vendor_codec = (a2dp_dev->codec.type == BT_A2DP_CODEC_VENDOR);
                    uint32_t latency_val = bt_sink_srv_get_latency(&a2dp_dev->dev_addr, vendor_codec, true, false);
                    uint32_t original_latency = bt_sink_srv_music_get_sink_latency();
                    if (BT_SINK_SRV_MUSIC_MODE_BROADCAST == bt_sink_srv_music_get_spk_mode()) {
                        latency_val = bt_sink_srv_get_latency(&a2dp_dev->dev_addr, vendor_codec, true, true);
                    }

                    if (bt_sink_srv_music_get_ALC_enable()) {
                        if (bt_iot_device_white_list_check_iot_case((bt_bd_addr_t *)&ctx->run_dev->dev_addr, BT_IOT_MUSIC_SET_LATENCY_TO_ORIGINAL)) {
                            latency_val = original_latency;
                            bt_sink_srv_report_id("[sink][music][mce_a2dp] Partner trigger ALC, special deivce, new_latency: %d", 1, latency_val);
                        } else if(bt_iot_device_white_list_check_iot_case((bt_bd_addr_t *)&ctx->run_dev->dev_addr, BT_IOT_MUSIC_SET_LATENCY_TO_250)) {
                            latency_val = BT_SINK_SRV_A2DP_SPECIAL_DEVICE_LATENCY;
                        }
                    }

                    bt_sink_srv_report_id("[sink][music][mce_a2dp] Partner trigger ALC, rho_flag:0x%02x, new_latency: %d, ori_latency: %d",
                                          3, ctx->rho_flag, latency_val, original_latency);

                    if (!ctx->rho_flag && (ctx->run_dev->op & BT_SINK_SRV_MUSIC_OP_DRV_PLAY)) {
                        bt_sink_srv_music_set_sink_latency(latency_val);
                        bt_sink_srv_a2dp_reinitial_sync();
                    }
                }
                break;
            }

            case BT_SINK_SRV_MUSIC_TYPE_AVRCP_PASS_THROUGH_COMMAND: {
                bt_sink_srv_aws_mce_avrcp_pass_through_action_t *operation_data = (bt_sink_srv_aws_mce_avrcp_pass_through_action_t *)(action_data->data);
                bt_avrcp_operation_id_t operation_id = 0;
                bt_avrcp_operation_state_t state = 0;
                bt_status_t ret = BT_STATUS_FAIL;

                if (a2dp_dev->conn_bit & BT_SINK_SRV_MUSIC_AVRCP_CONN_BIT) {
                    if (operation_data->avrcp_action == BT_SINK_SRV_ACTION_FAST_FORWARD) {
                        operation_id = BT_AVRCP_OPERATION_ID_FAST_FORWARD;
                    } else if (operation_data->avrcp_action == BT_SINK_SRV_ACTION_REWIND) {
                        operation_id = BT_AVRCP_OPERATION_ID_REWIND;
                    }

                    if (operation_data->operation_state == BT_SINK_SRV_AVRCP_OPERATION_PRESS) {
                        state = BT_AVRCP_OPERATION_STATE_PUSH;
                        a2dp_dev->operation_action = operation_id;
                        a2dp_dev->operation_state = state;
                        BT_SINK_SRV_SET_FLAG(a2dp_dev->avrcp_flag, BT_SINK_SRV_DURING_PARTNER_TRIGGER_PASS_THROUGH);
                    } else if (operation_data->operation_state == BT_SINK_SRV_AVRCP_OPERATION_RELEASE) {
                        state = BT_AVRCP_OPERATION_STATE_RELEASED;
                        a2dp_dev->operation_state = BT_SINK_SRV_MUSIC_AVRCP_NO_STATE;
                        a2dp_dev->operation_action = BT_SINK_SRV_MUSIC_AVRCP_NO_OPERATION;
                        BT_SINK_SRV_REMOVE_FLAG(a2dp_dev->avrcp_flag, BT_SINK_SRV_DURING_PARTNER_TRIGGER_PASS_THROUGH);
                    }
                    ret = bt_sink_srv_avrcp_send_pass_through(a2dp_dev->avrcp_hd, operation_id, state);
                    (void)ret;
                }
                bt_sink_srv_report_id("[sink][music][mce_a2dp] Partner trigger pass through, conn_bit:0x%04x, op_id : 0x%02x, op_state: 0x%02x, ret:0x%08x",
                                      4, a2dp_dev->conn_bit, operation_id, state, ret);
                break;
            }
#ifdef AIR_A2DP_REINIT_V2_ENABLE
            case BT_SINK_SRV_MUSIC_TYPE_BUF_ABNORMAL_SYNC: {
                bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
                if ((ctx->run_dev) && (ctx->run_dev->op & BT_SINK_SRV_MUSIC_OP_CODEC_OPEN)) {
                    if (ctx->run_dev->med_handle.med_hd) {
                        ctx->run_dev->med_handle.med_hd->bt_partner_reinit_request(ctx->run_dev->med_handle.med_hd);
                    }
                }
                break;
            }
#endif
            default:
                break;
        }
    } else if (type == BT_SINK_SRV_MUSIC_TYPE_VOL) {
        if (ctx->run_dev) {
            switch (val) {
                case BT_SINK_SRV_MUSIC_TYPE_VOL_VAL_UP: {
                    bt_sink_srv_a2dp_change_volume(VOLUME_UP, 1, 0, ctx->run_dev);
                    break;
                }

                case BT_SINK_SRV_MUSIC_TYPE_VOL_VAL_DOWN: {
                    bt_sink_srv_a2dp_change_volume(VOLUME_DOWN, 1, 0, ctx->run_dev);
                    break;
                }

                case BT_SINK_SRV_MUSIC_TYPE_VOL_VAL_SET: {
                    uint8_t *volume = &action_data->data[0];
                    bt_sink_srv_a2dp_change_volume(VOLUME_VALUE, 1, *volume, ctx->run_dev);
                    break;
                }

                default:
                    break;
            }
        }
    } 
}

#ifdef AIR_MCSYNC_SHARE_ENABLE
bt_status_t bt_sink_srv_a2dp_mcsync_share_event_callback(bt_mcsync_share_event_t event, bt_status_t status, void *data)
{
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_mcsync_share_action_info_t *p_data = (bt_mcsync_share_action_info_t *)data;

    if ((event == BT_MCSYNC_SHARE_EVENT_ACTION_IND)
        && ctx->run_dev
        && p_data
        && (p_data->action == BT_MCSYNC_SHARE_CMD_FOLLOWER_SYNC_RETRIGGER_FOLLOWER_A2DP_TRANSFORM)) {
        if (p_data->data[1] & (1 << 0)) {
            bt_sink_srv_aws_mce_a2dp_send_eir(BT_SINK_SRV_AWS_MCE_A2DP_EVT_PLAY_IND, (void *)(&ctx->run_dev->a2dp_hd));
        } else if (p_data->data[1] & (1 << 1)) {
            bt_sink_srv_aws_mce_a2dp_send_eir(BT_SINK_SRV_AWS_MCE_A2DP_EVT_UPDATE_BASE, (void *)(&(ctx->run_dev->a2dp_hd)));
        }

        bt_sink_srv_report_id("[sink][music][mce_a2dp] bt_sink_srv_a2dp_mcsync_share_event_callback-event:0x%02x, action:0x%02x, data[1]:0x%02x", 3, event, p_data->action,
                              p_data->data[1]);
    }

    return BT_STATUS_SUCCESS;
}
#endif

void bt_sink_srv_aws_mce_a2dp_init()
{
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    ctx->ratio = BT_SINK_SRV_MUSIC_RATIO_MAGIC_CODE;
    ctx->aws_aid = bt_sink_srv_ami_audio_open(AUD_MIDDLE, bt_sink_srv_music_a2dp_common_ami_hdr);
    bt_sink_srv_memset(&ctx->a2dp_eir, 0, sizeof(bt_sink_srv_aws_mce_a2dp_base_eir_t));
#ifdef AIR_MCSYNC_SHARE_ENABLE
    bt_mcsync_share_register_callback(BT_MCSYNC_SHARE_MODULE_SINK_MUSIC, bt_sink_srv_a2dp_mcsync_share_event_callback);
#endif
    bt_sink_srv_report_id("[sink][music][mce_a2dp] a2dp_init-aws_aid: %d", 1, ctx->aws_aid);
}


static void bt_sink_srv_aws_mce_a2dp_dump_eir_info(bt_sink_srv_aws_mce_a2dp_base_eir_t *a2dp_eir)
{
#ifdef MTK_BT_SPEAKER_ENABLE
    bt_sink_srv_report_id("[sink][music][mce_a2dp] dump_eir-state: %d, vol: %d, nclk: 0x%08x, nclk_intra: 0x%08x, ts: 0x%08x, ratio:0x%02x, samples:0x%04x, latency:0x%08x, special_device_flag:0x%02x, dev_flag:0x%02x, a2dp_mtu:0x%04x, speaker_compatible_flag:0x%08x", 12,
                          a2dp_eir->state, a2dp_eir->vol_lev, a2dp_eir->nclk,
                          a2dp_eir->nclk_intra,
                          a2dp_eir->ts, a2dp_eir->ratio,
                          a2dp_eir->sample_count,
                          a2dp_eir->latency_val,
                          a2dp_eir->special_device_flag,
                          a2dp_eir->flag,
                          a2dp_eir->a2dp_mtu,
                          a2dp_eir->speaker_compatible_flag);
#else
    bt_sink_srv_report_id("[sink][music][mce_a2dp] dump_eir-state: %d, vol: %d, nclk: 0x%08x, nclk_intra: 0x%08x, ts: 0x%08x, ratio:0x%02x, samples:0x%04x, latency:0x%08x, special_device_flag:0x%02x, dev_flag:0x%02x, a2dp_mtu:0x%04x", 11,
                          a2dp_eir->state, a2dp_eir->vol_lev, a2dp_eir->nclk,
                          a2dp_eir->nclk_intra,
                          a2dp_eir->ts, a2dp_eir->ratio,
                          a2dp_eir->sample_count,
                          a2dp_eir->latency_val,
                          a2dp_eir->special_device_flag,
                          a2dp_eir->flag,
                          a2dp_eir->a2dp_mtu);

#endif
}

#ifdef AIR_BT_A2DP_VENDOR_CODEC_SUPPORT
void bt_sink_srv_aws_mce_vendor_codec_config_sync(bool opened)
{
    bt_sink_srv_report_id("[sink][music][mce_a2dp] vendor_codec_config_sync--opened: %d", 1, opened);
    bt_sink_srv_aws_mce_a2dp_send_action(BT_SINK_SRV_MUSIC_TYPE_CONTROL, BT_SINK_SRV_MUSIC_TYPE_SYNC_VENDOR_CODEC_CONFIG, (void *)(&opened), sizeof(opened));
}
#endif

void bt_sink_srv_aws_mce_trigger_agent_reinitial_sync(uint8_t reinit_type)
{
    bt_sink_srv_music_device_t *aws_a2dp_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AWS, (void *)AUDIO_SRC_SRV_PSEUDO_DEVICE_AWS_A2DP);

    if (aws_a2dp_dev) {
        bt_sink_srv_aws_mce_a2dp_send_action(BT_SINK_SRV_MUSIC_TYPE_CONTROL, reinit_type, NULL, 0);
    }

    bt_sink_srv_report_id("[sink][music][mce_a2dp] Trigger agent reinitial sync--aws_dev:0x%08x", 1, aws_a2dp_dev);
}


void bt_sink_srv_aws_mce_a2dp_ami_hdr(bt_sink_srv_am_id_t aud_id, bt_sink_srv_am_cb_msg_class_t msg_id, bt_sink_srv_am_cb_sub_msg_t sub_msg, void *param)
{
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();;
    bt_sink_srv_music_device_t *run_dev = ctx->run_dev;;

    if ((NULL == run_dev) || (ctx->aws_aid != aud_id)) {
        return;
    }

    bt_sink_srv_report_id("[sink][music][mce_a2dp] ami_hdr--msg_id:%d,sub_msg:%d, run_dev:0x%x", 3, msg_id, sub_msg, run_dev);

    switch (msg_id) {
        case AUD_SINK_OPEN_CODEC: {
            if (run_dev->flag & BT_SINK_SRV_MUSIC_FLAG_WAIT_AMI_OPEN_CODEC) {
                bt_sink_srv_report_id("[sink][music][mce_a2dp] open codec success--flag:0x%x, op:0x%x, substate:0x%x, sec_type:0x%x",
                    4, run_dev->flag, run_dev->op, run_dev->handle->substate, run_dev->codec.sec_type);
                /* Save codec handle */
#if defined (AIR_LE_AUDIO_ENABLE) && defined (AIR_LE_AUDIO_CIS_ENABLE) && !defined(AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE)
                bt_handle_t le_handle = bt_sink_srv_cap_stream_get_ble_link_with_cis_established();
                if (le_handle != BT_HANDLE_INVALID) {
                    bt_sink_srv_cap_stream_release_autonomously(le_handle, 0xFF, false, 0);
                }
#endif
                bt_sink_srv_memcpy(&(run_dev->med_handle), param, sizeof(bt_sink_srv_am_media_handle_t));
                BT_SINK_SRV_REMOVE_FLAG(run_dev->flag, BT_SINK_SRV_MUSIC_FLAG_WAIT_AMI_OPEN_CODEC);
                /* Set codec open flag */
                BT_SINK_SRV_SET_FLAG(run_dev->op, BT_SINK_SRV_MUSIC_OP_CODEC_OPEN);
                if (BT_SINK_SRV_MUSIC_TRANSIENT_STATE_CLEAR_CODEC == run_dev->handle->substate) {
                    BT_SINK_SRV_REMOVE_FLAG(run_dev->flag, BT_SINK_SRV_MUSIC_WAIT_SET_VOLUME);
                    bt_sink_srv_music_state_machine_handle(run_dev, BT_SINK_SRV_MUSIC_EVT_CODEC_OPEN, NULL);
                    break;
                }

                if (run_dev->flag & BT_SINK_SRV_MUSIC_WAIT_SET_VOLUME) {
                    bt_sink_srv_music_set_am_volume(ctx->vol_lev);
                    BT_SINK_SRV_REMOVE_FLAG(run_dev->flag, BT_SINK_SRV_MUSIC_WAIT_SET_VOLUME);
                }

                if (run_dev->codec.sec_type) {
                    bt_sink_srv_report_id("[sink][music][mce_a2dp] set content protection", 0);
                    run_dev->med_handle.med_hd->set_content_protection(run_dev->med_handle.med_hd, true);
                }

                uint32_t gap_hd = bt_sink_srv_cm_get_gap_handle(&(run_dev->dev_addr));
                bt_sink_srv_music_update_audio_buffer(SHARE_BUFFER_BT_AUDIO_DL_SIZE);
                bt_sink_srv_music_set_music_enable(run_dev->aws_hd, BT_AVM_ROLE_AWS_MCE_PARTNER, true);
                uint32_t n_value = BT_SINK_SRV_MCE_A2DP_N_PACKET_NOTIFY;
                if (run_dev->codec.type == BT_A2DP_CODEC_VENDOR || ctx->a2dp_eir.special_device_flag == BT_SINK_SRV_MUSIC_NOTIFY_N9_NO_SLEEP) {
                    n_value = BT_SINK_SRV_MCE_A2DP_N_PACKET_NOTIFY_VENDOR_CODEC;
                }

                if (run_dev->codec.sec_type) {
                    n_value |= BT_SINK_SRV_MUSIC_CONTENT_PROTECTION_MASK;
                }

#ifdef AIR_A2DP_REINIT_V2_ENABLE
                if ((run_dev->codec.type == BT_A2DP_CODEC_SBC) && (ctx->a2dp_eir.special_device_flag & BT_SINK_SRV_MUSIC_SHORT_PCDC_OBSERVATION)) {
                    n_value |= BT_SINK_SRV_A2DP_SHORT_PCDC_OBSERVATION;
                    bt_media_handle_t *med_hd = run_dev->med_handle.med_hd;
                    med_hd->set_ts_ratio(med_hd, 0xfffffffe);
                }
#endif
                bt_avm_set_a2dp_notify_condition(gap_hd, n_value);
                bt_sink_srv_music_state_machine_handle(run_dev, BT_SINK_SRV_MUSIC_EVT_CODEC_OPEN, NULL);
                if (run_dev->op & BT_SINK_SRV_MUSIC_OP_PLAY_IND) {
                    uint32_t gap_hd = bt_sink_srv_cm_get_gap_handle(&(run_dev->dev_addr));
                    bt_utils_assert(gap_hd);
                    bt_sink_srv_aws_mce_a2dp_set_sta(gap_hd);
                }
            }
            break;
        }

        case AUD_A2DP_PROC_IND: {
            switch (sub_msg) {
                case AUD_A2DP_DL_REINIT_REQUEST: {
                    bt_sink_srv_music_dsp_parameter_t *para_data = (bt_sink_srv_music_dsp_parameter_t *)param;
                    bt_utils_assert(param);
                    bt_aws_mce_srv_mode_t mode = bt_sink_srv_music_get_spk_mode();
                    bt_sink_srv_report_id("[sink][music][mce_a2dp] reinitial sync para_data:%d, mode:0x%x", 2, *para_data, mode);
                    if (*para_data == BT_SINK_SRV_MUSIC_REINIT_AFE_ABNORMAL) {
                        if (BT_SINK_SRV_MUSIC_MODE_BROADCAST != mode) {
                            bt_sink_srv_aws_mce_trigger_agent_reinitial_sync(BT_SINK_SRV_MUSIC_TYPE_CONTROL_TRIGGER_REINITIAL_SYNC);
                        }
#ifdef MTK_BT_SPEAKER_ENABLE
                        else {
                            BT_SINK_SRV_SET_FLAG(ctx->context_flag, BT_SINK_SRV_CNTX_FLAG_RECONNECT_AGENT_FLAG);
                        }
#endif
                    } else {
#ifdef AIR_A2DP_REINIT_V2_ENABLE
                        if (*para_data == BT_SINK_SRV_MUSIC_REINIT_BUF_ABNORMAL) {
                            bt_sink_srv_aws_mce_trigger_agent_reinitial_sync(BT_SINK_SRV_MUSIC_TYPE_BUF_ABNORMAL_SYNC);
                        }
#endif
                    }
                    break;
                }
                case AUD_A2DP_ACTIVE_LATENCY_REQUEST: {
                    if (BT_SINK_SRV_MUSIC_MODE_BROADCAST != bt_sink_srv_music_get_spk_mode()) {
                        bt_sink_srv_aws_mce_trigger_agent_reinitial_sync(BT_SINK_SRV_MUSIC_TYPE_CONTROL_TRIGGER_ALC);
                    }
                    break;
                }
#ifdef MTK_BT_SPEAKER_ENABLE
                case AUD_A2DP_RECONNECT_REQUEST: {
                    if (BT_SINK_SRV_MUSIC_MODE_BROADCAST == bt_sink_srv_music_get_spk_mode()) {
                        bt_sink_srv_report_id("[sink][music][mce_a2dp] Client side trigger reconnect", 0);
                        BT_SINK_SRV_SET_FLAG(ctx->context_flag, BT_SINK_SRV_CNTX_FLAG_RECONNECT_AGENT_FLAG);
                        bt_cm_connect_t dis_info;
                        bt_sink_srv_memcpy(dis_info.address, ctx->run_dev->dev_addr, 6);
                        dis_info.profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS);
                        bt_cm_disconnect(&dis_info);
                    }
                    break;
                }
#endif
            }
            break;
        }

        default:
            break;
    }
}

static void bt_sink_srv_aws_mce_a2dp_handle_state_change_ind(bt_aws_mce_state_change_ind_t *state_ind, bool need_change_state)
{

    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_bd_addr_t *local_addr = bt_connection_manager_device_local_info_get_local_address();
    uint32_t aws_handle = bt_sink_srv_music_get_aws_handle_by_addr(local_addr);

    bt_sink_srv_report_id("[sink][music][mce_a2dp] state_change(s)-aws_hd: 0x%x, aws_handle:0x%x, state:0x%0x, role:0x%x, need_change_state:0x%x", 5,
                          state_ind->handle, aws_handle, state_ind->state, role, need_change_state);
    if (state_ind->handle == aws_handle || BT_AWS_MCE_ROLE_AGENT != role) {
        return;
    }
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AWS_A2DP_HD, (void *)(&(state_ind->handle)));
    bt_utils_assert(dev);
    bt_sink_srv_music_device_t *sp_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_ADDR_AVRCP, (void *)(&dev->dev_addr));
    if (!sp_dev) {
        sp_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_ADDR_A2DP, (void *)(&dev->dev_addr));
    }
#ifndef BT_AWS_MCE_FAST_SWITCH
    if (state_ind->state == BT_AWS_MCE_AGENT_STATE_ATTACHED) {
#else
    if (state_ind->state == BT_AWS_MCE_AGENT_STATE_ACTIVE) {
#endif
    BT_SINK_SRV_SET_FLAG(dev->conn_bit, BT_SINK_SRV_MUSIC_AWS_CONN_BIT);
    if (need_change_state) {
        bt_sink_srv_music_state_machine_handle(dev, BT_SINK_SRV_MUSIC_EVT_READY, NULL);
    }
    if (sp_dev && (sp_dev->conn_bit & BT_SINK_SRV_MUSIC_A2DP_CONN_BIT)) {
        if (sp_dev->op & BT_SINK_SRV_MUSIC_OP_CODEC_OPEN) {
            /* Case 4. AWS: SP2 dynamic plug in */
            if (sp_dev->state == AUDIO_SRC_SRV_STATE_PLAYING) {
                if (sp_dev->op & BT_SINK_SRV_MUSIC_REINIT_ON_PARTNER_LATER_JOIN_FLAG) {
                    BT_SINK_SRV_REMOVE_FLAG(sp_dev->op, BT_SINK_SRV_MUSIC_REINIT_ON_PARTNER_LATER_JOIN_FLAG);
                    bt_sink_srv_a2dp_reinitial_sync();
                } else {
                    bt_sink_srv_aws_mce_a2dp_send_eir(BT_SINK_SRV_AWS_MCE_A2DP_EVT_PLAY_IND, (void *)(&(sp_dev->a2dp_hd)));
                }
            } else {
                bt_sink_srv_aws_mce_a2dp_send_eir(BT_SINK_SRV_AWS_MCE_A2DP_EVT_START_STREAMING, (void *)(&(sp_dev->a2dp_hd)));
            }
        } else {
            if (sp_dev->flag & BT_SINK_SRV_MUSIC_FLAG_WAIT_AMI_OPEN_CODEC) {
                bt_sink_srv_aws_mce_a2dp_send_eir(BT_SINK_SRV_AWS_MCE_A2DP_EVT_START_STREAMING, (void *)(&(sp_dev->a2dp_hd)));
            }
        }
    }
#ifdef AIR_BT_A2DP_VENDOR_CODEC_SUPPORT
        bool opened = bt_sink_srv_a2dp_get_vendor_codec_config();
        bt_sink_srv_aws_mce_vendor_codec_config_sync(opened);
#endif
    } else if (state_ind->state == BT_AWS_MCE_AGENT_STATE_INACTIVE) {
        if (sp_dev && (sp_dev->avrcp_flag & BT_SINK_SRV_DURING_PARTNER_TRIGGER_PASS_THROUGH)
            && (sp_dev->conn_bit & BT_SINK_SRV_MUSIC_AVRCP_CONN_BIT)
            && (sp_dev->operation_action != BT_SINK_SRV_MUSIC_AVRCP_NO_OPERATION)
            && (sp_dev->operation_state == BT_AVRCP_OPERATION_STATE_PUSH)) {
            bt_sink_srv_avrcp_send_pass_through(sp_dev->avrcp_hd, sp_dev->operation_action, BT_AVRCP_OPERATION_STATE_RELEASED);
            BT_SINK_SRV_REMOVE_FLAG(sp_dev->avrcp_flag, BT_SINK_SRV_DURING_PARTNER_TRIGGER_PASS_THROUGH);
            sp_dev->operation_state = BT_SINK_SRV_MUSIC_AVRCP_NO_STATE;
            sp_dev->operation_action = BT_SINK_SRV_MUSIC_AVRCP_NO_OPERATION;
        }
        if (need_change_state) {
            bt_sink_srv_aws_mce_a2dp_handle_disconnect_ind(state_ind->handle, false);
        }
    }
}

static int32_t bt_sink_srv_aws_mce_a2dp_handle_connect_ind(bt_aws_mce_connected_t *conn_ind)
{
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AWS_A2DP_HD, (void *)(&(conn_ind->handle)));
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_sink_srv_music_context_t *ctx  = bt_sink_srv_music_get_context();
    int32_t ret = 0;
    uint8_t *addr = (uint8_t *)conn_ind->address;

    bt_sink_srv_report_id("[sink][music][mce_a2dp] connect_cnf(s)-aws_hd: 0x%x, conn_ind->handle, addr--0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x",
        7, conn_ind->handle, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
    bt_bd_addr_t *local_addr = bt_connection_manager_device_local_info_get_local_address();
    if (bt_sink_srv_memcmp(local_addr, (void *)addr, sizeof(bt_bd_addr_t)) == 0) {
        bt_sink_srv_report_id("[sink][music][mce_a2dp] It is same with local addr", 0);
        return ret;
    }
    if (!dev) {
        dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_UNUSED, (void *)AUDIO_SRC_SRV_PSEUDO_DEVICE_AWS_A2DP);
    } else {
        if (role == BT_AWS_MCE_ROLE_CLINET || role == BT_AWS_MCE_ROLE_PARTNER) {
            BT_SINK_SRV_SET_FLAG(dev->conn_bit, BT_SINK_SRV_MUSIC_AWS_CONN_BIT);
        }
        return ret;
    }

    bt_utils_assert(dev);

    dev->aws_hd = conn_ind->handle;
    bt_sink_srv_memcpy(&(dev->dev_addr), (void *)addr, sizeof(bt_bd_addr_t));
    /* Init pse handle */
    dev->handle = bt_sink_srv_a2dp_alloc_pseudo_handle();
    dev->handle->type = AUDIO_SRC_SRV_PSEUDO_DEVICE_AWS_A2DP;
    dev->handle->priority = AUDIO_SRC_SRV_PRIORITY_NORMAL;
    dev->handle->dev_id = bt_sink_srv_music_convert_btaddr_to_devid(&(dev->dev_addr));

    if (role != BT_AWS_MCE_ROLE_AGENT) {
        BT_SINK_SRV_SET_FLAG(dev->conn_bit, BT_SINK_SRV_MUSIC_AWS_CONN_BIT);
        BT_SINK_SRV_SET_FLAG(dev->op, BT_SINK_SRV_MUSIC_NO_STREAMING_STATE_UPDATE_FLAG);
        bt_sink_srv_music_state_machine_handle(dev, BT_SINK_SRV_MUSIC_EVT_READY, NULL);
        bt_sink_srv_memset(&ctx->a2dp_eir, 0, sizeof(bt_sink_srv_aws_mce_a2dp_base_eir_t));
    }
    bt_sink_srv_report_id("[sink][music][mce_a2dp] connect_cnf(e)-ret: %d, dev:0x%x", 2, ret, dev);

    return ret;
}

#ifdef MTK_BT_SPEAKER_ENABLE
static void bt_sink_srv_aws_mce_a2dp_handle_reconnect(uint32_t timer_id, uint32_t data)
{
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    if ((ctx->context_flag & BT_SINK_SRV_CNTX_FLAG_RECONNECT_AGENT_FLAG) &&
        (BT_SINK_SRV_MUSIC_MODE_BROADCAST == bt_sink_srv_music_get_spk_mode())
        && (BT_AWS_MCE_ROLE_CLINET == bt_connection_manager_device_local_info_get_aws_role())) {
        bt_sink_srv_report_id("[sink][music][mce_a2dp] Client side trigger connect", 0);
        bt_cm_connect_t dis_info;
        bt_bd_addr_t *addr = bt_device_manager_aws_local_info_get_peer_address();
        bt_sink_srv_memcpy(&(dis_info.address), addr, 6);
        dis_info.profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS);
        bt_cm_connect(&dis_info);
        BT_SINK_SRV_REMOVE_FLAG(ctx->context_flag, BT_SINK_SRV_CNTX_FLAG_RECONNECT_AGENT_FLAG);
    }
}
#endif

static int32_t bt_sink_srv_aws_mce_a2dp_handle_disconnect_ind(uint32_t handle, bool free_handle)
{
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AWS_A2DP_HD, (void *)(&handle));

    if (dev) {
        bt_sink_srv_report_id("[sink][music][mce_a2dp] disconnect_ind[s]-aws_hd: 0x%x, free_handle:0x%x, flag:0x%x, substate:0x%x, context_flag:0x%x",
            5, handle, free_handle, dev->flag, dev->handle->substate, ctx->context_flag);
        BT_SINK_SRV_REMOVE_FLAG(dev->conn_bit, BT_SINK_SRV_MUSIC_AWS_CONN_BIT);
        audio_src_srv_del_waiting_list(dev->handle);
        if (dev->flag & BT_SINK_SRV_MUSIC_FLAG_WAIT_LIST_SINK_PLAY) {
            BT_SINK_SRV_REMOVE_FLAG(dev->flag, BT_SINK_SRV_MUSIC_FLAG_WAIT_LIST_SINK_PLAY);
        }

        bt_sink_srv_music_state_machine_handle(dev, BT_AWS_MCE_DISCONNECTED, NULL);
        if (free_handle
            && dev->handle
            && (dev->handle->substate != BT_SINK_SRV_MUSIC_TRANSIENT_STATE_CLEAR_CODEC)) {
            /* Deinit pse handle */
            dev->aws_hd = BT_SINK_SRV_MUSIC_INVALID_HD;
            bt_sink_srv_a2dp_free_pseudo_handle(dev->handle);
            dev->handle = NULL;
            if (!(dev->conn_bit)) {
                bt_sink_srv_music_reset_device(dev);
                bt_sink_srv_memset(&ctx->a2dp_eir, 0, sizeof(bt_sink_srv_aws_mce_a2dp_base_eir_t));
            }
        } else if (free_handle && dev->handle
                   && (dev->handle->substate == BT_SINK_SRV_MUSIC_TRANSIENT_STATE_CLEAR_CODEC)
                   && (!(dev->flag & BT_SINK_SRV_MUSIC_FLAG_WAIT_AMI_OPEN_CODEC))) {
            bt_sink_srv_music_stop(dev, ctx->aws_aid);
        }
#ifdef MTK_BT_SPEAKER_ENABLE
        if ((ctx->context_flag & BT_SINK_SRV_CNTX_FLAG_RECONNECT_AGENT_FLAG) &&
            (BT_SINK_SRV_MUSIC_MODE_BROADCAST == bt_sink_srv_music_get_spk_mode())
            && (BT_AWS_MCE_ROLE_CLINET == bt_connection_manager_device_local_info_get_aws_role())) {
            bt_sink_srv_report_id("[sink][music][mce_a2dp] Client side trigger connect", 0);
            bt_timer_ext_start(BT_SINK_SRV_RECONNECT_AGENT_TIMER_ID, 0,
                               BT_SINK_SRV_RECONNECT_AGENT_TIMER_DUR, bt_sink_srv_aws_mce_a2dp_handle_reconnect);
        }
#endif
    }
    bt_sink_srv_report_id("[sink][music][mce_a2dp] disconnect_ind[e]-dev: 0x%x", 1, dev);
    return 0;
}

static void bt_sink_srv_aws_mce_a2dp_handle_a2dp_start_ind(uint32_t handle, bt_sink_srv_aws_mce_a2dp_base_eir_t *a2dp_eir)
{
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AWS_A2DP_HD, (void *)(&handle));
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    ctx->ratio = BT_SINK_SRV_MUSIC_RATIO_MAGIC_CODE;

    if (dev) {
        bt_sink_srv_report_id("[sink][music][mce_a2dp] start_ind-hd: 0x%08x, flag: 0x%08x, op: 0x%08x, substate:0x%x",
            4, handle, dev->flag, dev->op, dev->handle->substate);
        if ((BT_SINK_SRV_MUSIC_MODE_BROADCAST != bt_sink_srv_music_get_spk_mode())) {
            ctx->vol_lev = a2dp_eir->vol_lev;
        }
        bt_sink_srv_memcpy(&(dev->codec), &(a2dp_eir->codec), sizeof(bt_a2dp_codec_capability_t));
        if (dev->handle->substate == BT_SINK_SRV_MUSIC_TRANSIENT_STATE_CLEAR_CODEC) {
            BT_SINK_SRV_SET_FLAG(dev->flag, BT_SINK_SRV_MUSIC_FLAG_WAITING_START);
            BT_SINK_SRV_REMOVE_FLAG(dev->op, BT_SINK_SRV_MUSIC_OP_PLAY_IND);
        } else {
        if (!(dev->op & BT_SINK_SRV_MUSIC_OP_CODEC_OPEN)) {
            bt_sink_srv_music_set_sink_latency(a2dp_eir->latency_val);
            bt_sink_srv_music_state_machine_handle(dev, BT_SINK_SRV_MUSIC_EVT_START_IND, NULL);
        }
    }
    }
    bt_sink_srv_report_id("[sink][music][mce_a2dp] strat_ind-hd(e)-dev:0x%x", 1, dev);
}


static void bt_sink_srv_aws_mce_a2dp_handle_a2dp_suspend_ind(uint32_t handle, bt_sink_srv_aws_mce_a2dp_base_eir_t *a2dp_eir)
{
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AWS_A2DP_HD, (void *)(&handle));
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();

    /* clear fec cached media data && fec data when a2dp suspend */
#ifdef MTK_BT_SPEAKER_ENABLE
#ifdef MTK_BT_SPEAKER_FEC_ENABLE
    if (BT_SINK_SRV_MUSIC_MODE_BROADCAST == bt_sink_srv_music_get_spk_mode()) {
        speaker_fec_init_context();
    }
#endif
#endif

    if (dev) {
        bt_sink_srv_report_id("[sink][music][mce_a2dp] suspend_ind-hd(s)-flag:0x%08x, run_dev:0x%x", 2, dev->flag, ctx->run_dev);
        if (dev->flag & BT_SINK_SRV_MUSIC_FLAG_WAIT_LIST_SINK_PLAY) {
        audio_src_srv_del_waiting_list(dev->handle);
        if (ctx->a2dp_eir.flag & BT_SINK_SRV_AWS_MCE_A2DP_REINIT_FLAG) {
            BT_SINK_SRV_SET_FLAG(dev->flag, BT_SINK_SRV_MUSIC_FLAG_REINITIAL_SYNC);
        } else {
            BT_SINK_SRV_REMOVE_FLAG(dev->flag, BT_SINK_SRV_MUSIC_FLAG_REINITIAL_SYNC);
        }
        BT_SINK_SRV_REMOVE_FLAG(dev->flag, BT_SINK_SRV_MUSIC_FLAG_WAIT_LIST_SINK_PLAY);
        BT_SINK_SRV_REMOVE_FLAG(dev->op, BT_SINK_SRV_MUSIC_OP_PLAY_IND);
        } else if ((ctx->run_dev) && (dev == ctx->run_dev)) {
#ifdef AIR_A2DP_SYNC_STOP_ENABLE
        if (0xEE == a2dp_eir->special_device_flag) {
            bt_sink_srv_music_set_force_stop(true);
        } else {
            bt_sink_srv_music_set_force_stop(false);
        }
#endif
        BT_SINK_SRV_REMOVE_FLAG(dev->op, BT_SINK_SRV_MUSIC_OP_PLAY_IND);
        bt_sink_srv_music_state_machine_handle(dev, BT_SINK_SRV_MUSIC_EVT_SUSPEND_IND, NULL);
    }
    }
    bt_sink_srv_report_id("[sink][music][mce_a2dp] suspend_ind-hd-dev:0x%08x", 1, dev);
}


static void bt_sink_srv_aws_mce_a2dp_handle_update_base(uint32_t handle, bt_sink_srv_aws_mce_a2dp_base_eir_t *a2dp_eir)
{
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AWS_A2DP_HD, (void *)(&handle));
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();

    if (dev) {
        bt_sink_srv_report_id("[sink][music][mce_a2dp] update_base-hd: 0x%08x, flag: 0x%08x, op: 0x%08x", 3, handle, dev->flag, dev->op);
        int16_t drift_val = (int16_t)a2dp_eir->drift_val;
        ctx->ts = a2dp_eir->ts;
        ctx->bt_clk.nclk = a2dp_eir->nclk;
        ctx->bt_clk.nclk_intra = a2dp_eir->nclk_intra;
        avm_direct_set_drift(drift_val);
        bt_sink_srv_music_update_base_parameters_to_dsp(drift_val, a2dp_eir->nclk, a2dp_eir->ts, a2dp_eir->ts);
    }
    bt_sink_srv_report_id("[sink][music][mce_a2dp] update_base(e)--dev:0x%x, state: 0x%02x, us_asi:0x%08x, nclk: 0x%08x, intra: 0x%08x, drift:0x%04x", 6,
                          dev, a2dp_eir->state, a2dp_eir->ts, a2dp_eir->nclk,
                          a2dp_eir->nclk_intra, a2dp_eir->drift_val);
}


static void bt_sink_srv_aws_mce_a2dp_handle_sync_vol(uint32_t handle, bt_sink_srv_aws_mce_a2dp_base_eir_t *a2dp_eir)
{
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AWS_A2DP_HD, (void *)(&handle));;
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();;
    bt_sink_srv_am_id_t ami_ret = 0;
    (void)ami_ret;

    if (dev) {
        if (a2dp_eir->vol_lev == BT_SINK_SRV_AWS_A2DP_VOL_CHANGE_UP) {
            bt_sink_srv_a2dp_change_volume(VOLUME_UP, 0, 0, dev);
        } else if (a2dp_eir->vol_lev == BT_SINK_SRV_AWS_A2DP_VOL_CHANGE_DOWN) {
            bt_sink_srv_a2dp_change_volume(VOLUME_DOWN, 0, 0, dev);
        } else {
            ctx->vol_lev = a2dp_eir->vol_lev;
        }
        if (dev->op & BT_SINK_SRV_MUSIC_OP_CODEC_OPEN) {
            bt_sink_srv_music_set_am_volume(ctx->vol_lev);
        } else {
            BT_SINK_SRV_SET_FLAG(dev->flag, BT_SINK_SRV_MUSIC_WAIT_SET_VOLUME);
        }
    }

    bt_sink_srv_report_id("[sink][music][mce_a2dp] sync_vol-dev:0x%x, ami_ret: %d, vol: %d", 3, dev, ami_ret, ctx->vol_lev);
}


static uint32_t bt_sink_srv_aws_mce_a2dp_get_eir_event(uint32_t handle, bt_sink_srv_aws_mce_a2dp_base_eir_t *a2dp_new_info)
{
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AWS_A2DP_HD, (void *)(&handle));
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_sink_srv_aws_mce_a2dp_base_eir_t *original_info = &ctx->a2dp_eir;
    uint32_t event = 0;

    if (dev) {
        switch (a2dp_new_info->state) {
            case BT_SINK_SRV_A2DP_UP_BASE: {
                event = BT_SINK_SRV_AWS_MCE_A2DP_EVT_UPDATE_BASE;
                break;
            }

            case BT_SINK_SRV_A2DP_STATE_READY: {
                if (a2dp_new_info->ts == BT_SINK_SRV_AWS_MCE_A2DP_MAGIC_CODE_2) {
                    event = BT_SINK_SRV_AWS_MCE_A2DP_EVT_SUSPEND_STREAMING;
                } else {
                    bt_sink_srv_report_id("[sink][music][mce_a2dp] get_eir_event(unknown ready event)", 0);
                }
                break;
            }

            case BT_SINK_SRV_A2DP_STATE_PLAY: {
                /* Notice: Don't change following check flow */
                if (a2dp_new_info->ts == BT_SINK_SRV_AWS_MCE_A2DP_MAGIC_CODE_3) {
                    event = BT_SINK_SRV_AWS_MCE_A2DP_EVT_START_STREAMING;
                    break;
                }

                if ((a2dp_new_info->vol_lev == BT_SINK_SRV_AWS_A2DP_VOL_CHANGE_UP)
                    || (a2dp_new_info->vol_lev == BT_SINK_SRV_AWS_A2DP_VOL_CHANGE_DOWN)
                    || (a2dp_new_info->ts == BT_SINK_SRV_AWS_MCE_A2DP_MAGIC_CODE_4)) {
                    event = BT_SINK_SRV_AWS_MCE_A2DP_EVT_VOL_SYNC;
                    break;
                }

                if ((a2dp_new_info->vol_lev != original_info->vol_lev) && a2dp_new_info->nclk == 0
#ifdef MTK_BT_SPEAKER_ENABLE
                    && BT_SINK_SRV_MUSIC_MODE_BROADCAST != bt_sink_srv_music_get_spk_mode()
#endif
                   ) {
                    event |= BT_SINK_SRV_AWS_MCE_A2DP_EVT_VOL_SYNC;
                    break;
                }

                if (!(dev->op & BT_SINK_SRV_MUSIC_OP_CODEC_OPEN) && !(dev->flag & BT_SINK_SRV_MUSIC_FLAG_WAIT_LIST_SINK_PLAY)
                    && (!(dev->flag & BT_SINK_SRV_MUSIC_FLAG_WAIT_LIST_SINK_PLAY))) {
                    event = (BT_SINK_SRV_AWS_MCE_A2DP_EVT_START_STREAMING | BT_SINK_SRV_AWS_MCE_A2DP_EVT_PLAY_IND);
                    break;
                }

                if (!(a2dp_new_info->nclk == original_info->nclk && a2dp_new_info->nclk_intra == original_info->nclk_intra && a2dp_new_info->ts == original_info->ts)) {
                    if (dev->handle->state != AUDIO_SRC_SRV_STATE_PLAYING) {
                        event = BT_SINK_SRV_AWS_MCE_A2DP_EVT_PLAY_IND;
                    } else if (!((original_info->nclk == 0) && (original_info->nclk_intra == 0) && (original_info->ts == 0))) {
                        event = BT_SINK_SRV_AWS_MCE_A2DP_EVT_REINIT;
                    }
                }

                if (a2dp_new_info->vol_lev != original_info->vol_lev
#ifdef MTK_BT_SPEAKER_ENABLE
                    && BT_SINK_SRV_MUSIC_MODE_BROADCAST != bt_sink_srv_music_get_spk_mode()
#endif
                   ) {
                    event |= BT_SINK_SRV_AWS_MCE_A2DP_EVT_VOL_SYNC;
                }
                if ((original_info->ts == BT_SINK_SRV_AWS_MCE_A2DP_MAGIC_CODE_3) && (a2dp_new_info->vol_lev == original_info->vol_lev)
                    && (dev->handle->state != AUDIO_SRC_SRV_STATE_PLAYING)) {
                    event |= BT_SINK_SRV_AWS_MCE_A2DP_EVT_PLAY_IND;
                }
                break;
            }
            default:
                break;
        }
        bt_sink_srv_report_id("[sink][music][mce_a2dp] get_eir_event(e)-hd: 0x%08x, flag:0x%x, op: 0x%x,dev--state:0x%08x, substate:%d, state:0x%02x, event:0x%08x", 7,
                              handle, dev->flag, dev->op, dev->handle->state,
                              dev->handle->substate, a2dp_new_info->state, event);
    }
    return event;
}


static bt_status_t bt_sink_srv_aws_mce_a2dp_handle_if_data_ind(bt_sink_srv_aws_mce_a2dp_if_data_ind_t *if_data_ind)
{
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AWS_A2DP_HD, (void *)(&(if_data_ind->handle)));
    bt_sink_srv_aws_mce_a2dp_base_eir_t *a2dp_new_info = (bt_sink_srv_aws_mce_a2dp_base_eir_t *)(if_data_ind->data);
    bt_sink_srv_aws_mce_a2dp_base_eir_t *real_a2dp_new_info = bt_sink_srv_memory_alloc(sizeof(bt_sink_srv_aws_mce_a2dp_base_eir_t));
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    uint32_t aws_hd = if_data_ind->handle;
    uint32_t event = 0;
    bt_utils_assert(dev);

    bt_sink_srv_memcpy(real_a2dp_new_info, a2dp_new_info, sizeof(bt_sink_srv_aws_mce_a2dp_base_eir_t));

#ifdef MTK_BT_SPEAKER_ENABLE
    if (real_a2dp_new_info->speaker_compatible_flag != 0xabcd5678) {
        real_a2dp_new_info->special_device_flag = 0;
        real_a2dp_new_info->flag = 0;
        real_a2dp_new_info->vol_change_count = 0;
        real_a2dp_new_info->a2dp_mtu = 895;
    }
#endif
    bt_sink_srv_aws_mce_a2dp_dump_eir_info(&(ctx->a2dp_eir));
    bt_sink_srv_aws_mce_a2dp_dump_eir_info(real_a2dp_new_info);
    real_a2dp_new_info->seq_magic = 0;

    if (bt_sink_srv_memcmp(&(ctx->a2dp_eir), real_a2dp_new_info, sizeof(bt_sink_srv_aws_mce_a2dp_base_eir_t)) == 0) {
        bt_sink_srv_report_id("[sink][music][mce_a2dp] It`s same with old eir, handle_eir[e]", 0);
        bt_sink_srv_memory_free(real_a2dp_new_info);
        return BT_STATUS_SUCCESS;
    }

    event = bt_sink_srv_aws_mce_a2dp_get_eir_event(aws_hd, real_a2dp_new_info);
    if ((event & BT_SINK_SRV_AWS_MCE_A2DP_EVT_UPDATE_BASE) &&
        !(dev->op & BT_SINK_SRV_MUSIC_OP_DRV_PLAY)) {
        int16_t drift_val = real_a2dp_new_info->drift_val;
        avm_direct_set_drift(drift_val);
#ifndef MTK_BT_SPEAKER_ENABLE
        ctx->a2dp_eir.nclk = real_a2dp_new_info->nclk;
        ctx->a2dp_eir.nclk_intra = real_a2dp_new_info->nclk_intra;
#endif
        bt_sink_srv_report_id("[sink][music][mce_a2dp] not start yet, not to update base", 0);
        bt_sink_srv_memory_free(real_a2dp_new_info);
        return BT_STATUS_SUCCESS;
    }

    bt_sink_srv_memcpy(&(ctx->a2dp_eir), real_a2dp_new_info, sizeof(bt_sink_srv_aws_mce_a2dp_base_eir_t));

    if (event & BT_SINK_SRV_AWS_MCE_A2DP_EVT_START_STREAMING) {
        bt_sink_srv_aws_mce_a2dp_handle_a2dp_start_ind(aws_hd, real_a2dp_new_info);
    }

    if (event & BT_SINK_SRV_AWS_MCE_A2DP_EVT_PLAY_IND) {
        int16_t drift_val = (int16_t)real_a2dp_new_info->drift_val;
        avm_direct_set_drift(drift_val);
        bt_sink_srv_aws_mce_a2dp_handle_new_play_ind(aws_hd, real_a2dp_new_info);
    }

    if (event & BT_SINK_SRV_AWS_MCE_A2DP_EVT_SUSPEND_STREAMING) {
        bt_sink_srv_aws_mce_a2dp_handle_a2dp_suspend_ind(aws_hd, real_a2dp_new_info);
    }

    if (event & BT_SINK_SRV_AWS_MCE_A2DP_EVT_UPDATE_BASE) {
        bt_sink_srv_aws_mce_a2dp_handle_update_base(aws_hd, real_a2dp_new_info);
    }

    if (event & BT_SINK_SRV_AWS_MCE_A2DP_EVT_REINIT) {
        bt_sink_srv_aws_mce_a2dp_handle_a2dp_suspend_ind(aws_hd, real_a2dp_new_info);
        bt_sink_srv_aws_mce_a2dp_handle_a2dp_start_ind(aws_hd, real_a2dp_new_info);
        int16_t drift_val = (int16_t)real_a2dp_new_info->drift_val;
        avm_direct_set_drift(drift_val);
        bt_sink_srv_aws_mce_a2dp_handle_new_play_ind(aws_hd, real_a2dp_new_info);
    }

    if (event & BT_SINK_SRV_AWS_MCE_A2DP_EVT_VOL_SYNC) {
        bt_sink_srv_aws_mce_a2dp_handle_sync_vol(aws_hd, real_a2dp_new_info);
    }

    bt_sink_srv_memory_free(real_a2dp_new_info);

    return BT_STATUS_SUCCESS;
}

static bt_status_t bt_sink_srv_aws_mce_a2dp_handle_if_ind(bt_aws_mce_information_ind_t *mce_if_ind)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    if (BT_AWS_MCE_ROLE_AGENT != role) { /* Partner handle */
        if (BT_AWS_MCE_INFORMATION_A2DP == mce_if_ind->packet.type) {
            bt_sink_srv_aws_mce_a2dp_if_data_ind_t if_data_ind;

            if_data_ind.handle = mce_if_ind->handle;
            if_data_ind.data = mce_if_ind->packet.data;
            if_data_ind.length = mce_if_ind->packet.data_length;

            bt_sink_srv_aws_mce_a2dp_handle_if_data_ind(&if_data_ind);
        }
    } else {
        bt_sink_srv_report_id("[sink][music][mce_a2dp] if_ind-unexpected case", 0);
    }

    return BT_STATUS_SUCCESS;
}

static void bt_sink_srv_aws_mce_a2dp_set_sta(uint32_t gap_handle)
{
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_sink_srv_music_data_info_t *packet_list = ctx->media_data_list;
    bt_sink_srv_music_device_t *run_dev = ctx->run_dev;

    if (run_dev && (run_dev->op & BT_SINK_SRV_MUSIC_OP_PLAY_IND) && (run_dev->op & BT_SINK_SRV_MUSIC_OP_CODEC_OPEN)) {
        bt_media_handle_t *media_hd = run_dev->med_handle.med_hd;
        ctx->freq = media_hd->get_sampling_rate(media_hd);
        if (ctx->packet_count) {
            bt_clock_t pta;
            uint32_t asi_base = ctx->ts & BT_SINK_SRV_A2DP_ASI_MASK;
            uint32_t asi_dur = (packet_list[0].asi - asi_base) & BT_SINK_SRV_A2DP_ASI_MASK;
            if (!(asi_dur & 0x02000000)) {
                avm_direct_calculate_pta_according_base(ctx->freq, asi_dur, &ctx->bt_clk, &pta);

                ctx->ts = packet_list[0].asi;
                bt_sink_srv_memcpy(&ctx->bt_clk, &pta, sizeof(bt_clock_t));
            }
            bt_sink_srv_music_trigger_play(gap_handle, run_dev, &ctx->bt_clk, asi_base, ctx->ts);
            bt_sink_srv_report_id("[sink][music][mce_a2dp] play_info, pta-nclk:0x%08x, pta-intra:0x%04x, asi:0x%08x", 3,
                                  ctx->bt_clk.nclk, ctx->bt_clk.nclk_intra, ctx->ts);
        }
        bt_sink_srv_music_clear_media_packet_list();
    }
}

static void bt_sink_srv_aws_mce_a2dp_handle_media_data_received_ind(uint32_t gap_handle, bt_sink_srv_music_data_info_t *media_info)
{
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_sink_srv_music_device_t *run_dev = ctx->run_dev;
    if ((!run_dev) || run_dev->op & BT_SINK_SRV_MUSIC_OP_DRV_PLAY) {
        return;
    }
    bt_sink_srv_report_id("[sink][music][mce_a2dp] media_data_received_ind, op_flag:0x%08x, cur_asi:0x%08x", 2, run_dev->op, media_info->asi);

    bt_sink_srv_music_fill_recevied_media_data(media_info);
    if ((!(run_dev->op & BT_SINK_SRV_MUSIC_OP_PLAY_IND)) || (!(run_dev->op & BT_SINK_SRV_MUSIC_OP_CODEC_OPEN))) {
        return;
    }

    bt_sink_srv_aws_mce_a2dp_set_sta(gap_handle);
}

static void bt_sink_srv_aws_mce_a2dp_handle_new_play_ind(uint32_t handle, bt_sink_srv_aws_mce_a2dp_base_eir_t *a2dp_eir)
{
    bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AWS_A2DP_HD, (void *)(&handle));
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_sink_srv_music_device_t *run_dev = ctx->run_dev;

    if (dev) {
        BT_SINK_SRV_SET_FLAG(dev->op, BT_SINK_SRV_MUSIC_OP_PLAY_IND);
    }
    ctx->bt_clk.nclk_intra = a2dp_eir->nclk_intra;
    ctx->bt_clk.nclk = a2dp_eir->nclk;
    ctx->ts = a2dp_eir->ts;
    if (a2dp_eir->ratio == BT_SINK_SRV_IPHONE_TYPE_IN_RATIO) {
        ctx->ratio = 0xffffffff;
    } else {
        ctx->ratio = a2dp_eir->ratio;
    }

    if (run_dev) {
    uint32_t gap_hd = bt_sink_srv_cm_get_gap_handle(&(run_dev->dev_addr));
    bt_utils_assert(gap_hd);
    bt_sink_srv_aws_mce_a2dp_set_sta(gap_hd);
    }
}

bt_status_t bt_sink_srv_aws_mce_a2dp_common_callback(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    int32_t ret = 0;
    
    switch (msg) {
        case BT_AWS_MCE_CONNECTED: {
            bt_aws_mce_connected_t *conn_ind = (bt_aws_mce_connected_t *)buffer;

            ret = bt_sink_srv_aws_mce_a2dp_handle_connect_ind(conn_ind);
            break;
        }

        case BT_AWS_MCE_DISCONNECTED: {
            bt_aws_mce_disconnected_t *disconn_ind = (bt_aws_mce_disconnected_t *)buffer;

            ret = bt_sink_srv_aws_mce_a2dp_handle_disconnect_ind(disconn_ind->handle, true);
            break;
        }

        case  BT_AWS_MCE_STATE_CHANGED_IND: {
            bt_aws_mce_state_change_ind_t *state_change = (bt_aws_mce_state_change_ind_t *)buffer;
            bt_aws_mce_state_change_ind_t state_info;
            bool need_change_state = true;
            bt_sink_srv_memcpy(&state_info, state_change, sizeof(bt_aws_mce_state_change_ind_t));
            bt_sink_srv_music_device_t *dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AWS_A2DP_HD, (void *)(&(state_change->handle)));
            if (dev) {
#ifdef AIR_MCSYNC_SHARE_ENABLE
            if (dev->aws_state > state_change->state) {
                if ((state_change->state & 0x3f) > 0) {
                    need_change_state = false;
                }
                state_info.state = BT_AWS_MCE_AGENT_STATE_INACTIVE;
            } else {
                if ((state_change->state & 0x3f) != 0x20) {
                    need_change_state = false;
                }
#ifndef BT_AWS_MCE_FAST_SWITCH
                state_info.state = BT_AWS_MCE_AGENT_STATE_ATTACHED;
#else            
                state_info.state = BT_AWS_MCE_AGENT_STATE_ACTIVE;
#endif
            }
#endif
            bt_sink_srv_aws_mce_a2dp_handle_state_change_ind(&state_info, need_change_state);
            }
#ifdef MTK_BT_SPEAKER_ENABLE
            if (BT_AWS_MCE_AGENT_STATE_CONNECTABLE == state_info.state) {
                bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
                if (ctx->run_dev
                    && (ctx->run_dev->conn_bit & BT_SINK_SRV_MUSIC_A2DP_CONN_BIT)) {
                    /* spk mode is updated later in DM */
                    bt_timer_ext_start(BT_SINK_SRV_A2DP_SEND_PLAY_IND_TIMER_ID, 0, 1,
                        bt_sink_srv_a2dp_send_play_ind_timer);
                }
            }
#endif

            break;
        }
#ifdef MTK_BT_SPEAKER_ENABLE
        case BT_AWS_MCE_MODE_CHANGED: {
            bt_aws_mce_set_mode_cnf_t *mode_cnf = buffer;
            /* BT_AWS_MCE_MODE_BROADCAST is mapping to BT_SINK_SRV_MUSIC_MODE_BROADCAST */
            if (BT_AWS_MCE_MODE_BROADCAST == mode_cnf->mode) {
                bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
                if (ctx->run_dev
                    && (ctx->run_dev->conn_bit & BT_SINK_SRV_MUSIC_A2DP_CONN_BIT)) {
                    bt_sink_srv_aws_mce_a2dp_send_eir(BT_SINK_SRV_AWS_MCE_A2DP_EVT_PLAY_IND, (void *)(&(ctx->run_dev->a2dp_hd)));
                    bt_timer_ext_start(BT_SINK_SRV_A2DP_SEND_PLAY_IND_TIMER_ID, 0, BT_SINK_SRV_A2DP_SEND_PLAY_IND_TIMER_DUR,
                        bt_sink_srv_a2dp_send_play_ind_timer);
                }
            }
            break;
        }
#endif
        case BT_AWS_MCE_INFOMATION_PACKET_IND: {
            bt_aws_mce_information_ind_t *mce_if_ind = (bt_aws_mce_information_ind_t *)buffer;
            bt_sink_srv_aws_mce_a2dp_handle_if_ind(mce_if_ind);
            break;
        }

        case BT_AVM_MEDIA_DATA_RECEIVED_IND: {
            if ((role == BT_AWS_MCE_ROLE_AGENT) || (role == BT_AWS_MCE_ROLE_NONE)) {
                break;
            }
            bt_avm_a2dp_media_info_t *media_info = (bt_avm_a2dp_media_info_t *)buffer;
            bt_sink_srv_music_data_info_t media_data_info;
            media_data_info.asi = media_info->asi;
            media_data_info.clock.nclk = media_info->clock.nclk;
            media_data_info.clock.nclk_intra = media_info->clock.nclk_intra;
            media_data_info.ratio = (media_info->ratio == 0xff ? 0xffffffff : media_info->ratio);
            media_data_info.samples = media_info->samples;
            bt_sink_srv_aws_mce_a2dp_handle_media_data_received_ind(media_info->gap_handle, &media_data_info);
            break;
        }

        case BT_AVM_SET_LOCAL_ASI_FLAG: {
            bt_avm_a2dp_local_asi_ind_t *media_info = (bt_avm_a2dp_local_asi_ind_t *)buffer;
            bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
            bt_sink_srv_music_set_nvdm_data(&ctx->run_dev->dev_addr, BT_SINK_SRV_MUSIC_DATA_LOCAL_ASI_FLAG, (void *)(&media_info->local_asi_flag));
            break;
        }

        default:
            break;
    }
    bt_sink_srv_report_id("[sink][music][mce_a2dp] common_hdr-msg: 0x%x, status: %d, role:0x%02x, ret:0x%08x", 4, msg, status, role, ret);
    return ret;
}


int32_t bt_sink_srv_aws_mce_a2dp_change_volume(uint8_t type)
{
    bt_sink_srv_music_device_t *aws_a2dp_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AWS, (void *)AUDIO_SRC_SRV_PSEUDO_DEVICE_AWS_A2DP);

    if (aws_a2dp_dev) {
        uint8_t vol_lev = AUD_VOL_OUT_LEVEL0;
        bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
        vol_lev = ctx->vol_lev;
        /* Volume up */
        if (VOLUME_UP == type) {
            if (vol_lev < BT_SINK_SRV_A2DP_MAX_VOL_LEV) {
                vol_lev = vol_lev + 1;
            }
        } else if (VOLUME_DOWN == type) {
            if (vol_lev > BT_SINK_SRV_A2DP_MIN_VOL_LEV) {
                vol_lev = vol_lev - 1;
            } 
        }

        if (vol_lev != ctx->vol_lev) {
            ctx->vol_lev = vol_lev;
            bt_sink_srv_music_set_am_volume(ctx->vol_lev);
        }
        bt_sink_srv_report_id("[sink][music][mce_a2dp] change_volume-vol: %d, vol_lev:0x%x, type:0x%x", 3, ctx->vol_lev, vol_lev, type);
    }

    return 0;
}

#ifdef MTK_BT_SPEAKER_ENABLE
static void bt_sink_srv_aws_mce_a2dp_set_speaker_action(uint8_t action_type, uint8_t action_value)
{
    bt_sink_srv_music_device_t *run_dev = bt_sink_srv_music_get_context()->run_dev;

    if (run_dev) {
        if ((BT_SINK_SRV_MUSIC_MODE_BROADCAST == bt_sink_srv_music_get_spk_mode()) && (action_type == BT_SINK_SRV_MUSIC_TYPE_VOL)) {
            if (action_value == BT_SINK_SRV_MUSIC_TYPE_VOL_VAL_UP) {
                bt_sink_srv_a2dp_change_volume(VOLUME_UP, 0, 0, run_dev);
            } else if (action_value == BT_SINK_SRV_MUSIC_TYPE_VOL_VAL_DOWN) {
                bt_sink_srv_a2dp_change_volume(VOLUME_DOWN, 0, 0, run_dev);
            }
        }
    }
}
#endif

static bt_status_t bt_sink_srv_aws_mce_a2dp_send_action(uint8_t action_type, uint8_t action_value, void *param, uint32_t param_len)
{
    bt_sink_srv_music_device_t *aws_a2dp_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_AWS, (void *)AUDIO_SRC_SRV_PSEUDO_DEVICE_AWS_A2DP);
    int32_t ret = 0;

    if (aws_a2dp_dev) {
#ifdef MTK_BT_SPEAKER_ENABLE
        if (BT_SINK_SRV_MUSIC_MODE_BROADCAST == bt_sink_srv_music_get_spk_mode()) {
            bt_sink_srv_aws_mce_a2dp_set_speaker_action(action_type, action_value);
            return ret;
        }
#endif
        uint32_t data_len = (param_len ? param_len : 1);
        bt_sink_srv_aws_mce_a2dp_action_t *action_data = (bt_sink_srv_aws_mce_a2dp_action_t *)bt_sink_srv_memory_alloc(sizeof(bt_sink_srv_aws_mce_a2dp_action_t) + data_len - 1);
        bt_utils_assert(action_data);
        bt_aws_mce_report_info_t app_report;
        action_data->action_type = action_type;
        action_data->action_length = BT_SINK_SRV_MUSIC_TYPE_CONTROL_VAL_LEN;
        action_data->action_value = action_value;
        if (param) {
            bt_sink_srv_memcpy(&action_data->data, param, data_len);
        }
        app_report.module_id = BT_AWS_MCE_REPORT_MODULE_SINK_MUSIC;
        app_report.param_len = sizeof(bt_sink_srv_aws_mce_a2dp_action_t) + data_len - 1;
        app_report.param = (void *)action_data;
        ret = bt_aws_mce_report_send_event(&app_report);
        bt_sink_srv_memory_free(action_data);
    }
    bt_sink_srv_report_id("[sink][music][mce_a2dp] control_music--aws_a2dp_dev:0x%x, ret: %d, action_type:%d, action_value:%d, firmware_type:%d",
                          5, aws_a2dp_dev, ret, action_type, action_value, bt_sink_srv_music_get_spk_mode());
    return ret;
}


bt_status_t bt_sink_srv_aws_mce_a2dp_action_handler(bt_sink_srv_action_t action, void *param)
{
    bt_sink_srv_report_id("[sink][music][mce_a2dp] action[s]-action: 0x%x, play: 0x%x", 2, action, BT_SINK_SRV_ACTION_PLAY);

    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    if (role == BT_AWS_MCE_ROLE_AGENT || role == BT_AWS_MCE_ROLE_NONE) {
        /* SP connection, ignore it */
        return BT_STATUS_SUCCESS;
    }

    switch (action) {
        case BT_SINK_SRV_ACTION_PLAY_PAUSE: {
            bt_sink_srv_aws_mce_a2dp_send_action(BT_SINK_SRV_MUSIC_TYPE_CONTROL, BT_SINK_SRV_MUSIC_TYPE_CONTROL_VAL_PLAY_PAUSE, NULL, 0);
            break;
        }

        case BT_SINK_SRV_ACTION_PLAY: {
            bt_sink_srv_aws_mce_a2dp_send_action(BT_SINK_SRV_MUSIC_TYPE_CONTROL, BT_SINK_SRV_MUSIC_TYPE_CONTROL_VAL_PLAY, NULL, 0);
            break;
        }

        case BT_SINK_SRV_ACTION_PAUSE: {
            bt_sink_srv_aws_mce_a2dp_send_action(BT_SINK_SRV_MUSIC_TYPE_CONTROL, BT_SINK_SRV_MUSIC_TYPE_CONTROL_VAL_PAUSE, NULL, 0);
            break;
        }

        case BT_SINK_SRV_ACTION_NEXT_TRACK: {
            bt_sink_srv_aws_mce_a2dp_send_action(BT_SINK_SRV_MUSIC_TYPE_CONTROL, BT_SINK_SRV_MUSIC_TYPE_CONTROL_VAL_NEXT_TRC, NULL, 0);
            break;
        }

        case BT_SINK_SRV_ACTION_PREV_TRACK: {
            bt_sink_srv_aws_mce_a2dp_send_action(BT_SINK_SRV_MUSIC_TYPE_CONTROL, BT_SINK_SRV_MUSIC_TYPE_CONTROL_VAL_PRE_TRC, NULL, 0);
            break;
        }

        case BT_SINK_SRV_ACTION_VOLUME_UP: {
            bt_sink_srv_aws_mce_a2dp_send_action(BT_SINK_SRV_MUSIC_TYPE_VOL, BT_SINK_SRV_MUSIC_TYPE_VOL_VAL_UP, NULL, 0);
            break;
        }

        case BT_SINK_SRV_ACTION_VOLUME_DOWN: {
            bt_sink_srv_aws_mce_a2dp_send_action(BT_SINK_SRV_MUSIC_TYPE_VOL, BT_SINK_SRV_MUSIC_TYPE_VOL_VAL_DOWN, NULL, 0);
            break;
        }

        case BT_SINK_SRV_ACTION_SET_VOLUME: {
            uint8_t *volume_value = (uint8_t *)param;
            bt_sink_srv_aws_mce_a2dp_send_action(BT_SINK_SRV_MUSIC_TYPE_VOL, BT_SINK_SRV_MUSIC_TYPE_VOL_VAL_SET, volume_value, sizeof(uint8_t));
            break;
        }

        case BT_SINK_SRV_ACTION_FAST_FORWARD:
        case BT_SINK_SRV_ACTION_REWIND: {
            bt_sink_srv_avrcp_operation_state_t *operation_state = (bt_sink_srv_avrcp_operation_state_t *)param;
            bt_sink_srv_aws_mce_avrcp_pass_through_action_t action_data;
            action_data.avrcp_action = action;
            action_data.operation_state = *operation_state;

            bt_sink_srv_aws_mce_a2dp_send_action(BT_SINK_SRV_MUSIC_TYPE_CONTROL, BT_SINK_SRV_MUSIC_TYPE_AVRCP_PASS_THROUGH_COMMAND, (void *)&action_data, sizeof(action_data));
        }

        default:
            break;
    }


    return BT_STATUS_SUCCESS;
}

void bt_sink_srv_aws_mce_a2dp_play(audio_src_srv_handle_t *handle)
{
    bt_sink_srv_music_device_t *aws_a2dp_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_PSE_HD, (void *)handle);
    bt_sink_srv_am_audio_capability_t aud_cap = {0};
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    bt_utils_assert(aws_a2dp_dev);

    if (aws_a2dp_dev->handle->substate != BT_SINK_SRV_MUSIC_TRANSIENT_STATE_PREPARE_CODEC) {
        bt_sink_srv_music_state_machine_handle(aws_a2dp_dev, BT_SINK_SRV_MUSIC_EVT_PREPARE_CODEC, NULL);
    }
    bt_sink_srv_report_id("[sink][music][mce_a2dp] play(s)--hd: 0x%x, type: %d, flag: 0x%x, op: 0x%x, role:0x%x, aws_aid:0x%x", 6,
                          handle, handle->type, aws_a2dp_dev->flag, aws_a2dp_dev->op, role, ctx->aws_aid);
#ifndef BT_SINK_SRV_IMPROVE_RESYNC
    if (aws_a2dp_dev->flag&BT_SINK_SRV_MUSIC_FLAG_REINITIAL_SYNC) {
#ifdef AIR_LE_AUDIO_ENABLE
        bt_sink_srv_cap_am_enable_waiting_list();
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
        bt_ull_le_am_operate_waiting_list_for_a2dp_resync(true);
#endif
    }
#endif
    /* add for partner BLe advertising feature, to disconnect BLE on partner */
    if (role == BT_AWS_MCE_ROLE_PARTNER) {
        /* disconnect BLE */
        bt_sink_srv_event_callback(BT_SINK_SRV_EVENT_LE_DISCONNECT, NULL, 0);
    }

    /* Audio source accept play request */
    BT_SINK_SRV_SET_FLAG(aws_a2dp_dev->flag, BT_SINK_SRV_MUSIC_FLAG_WAIT_AMI_OPEN_CODEC);
    /* Update run device */
    bt_sink_srv_music_update_run_device(aws_a2dp_dev);
#ifdef MTK_BT_SPEAKER_ENABLE
    if ((BT_SINK_SRV_MUSIC_MODE_BROADCAST == bt_sink_srv_music_get_spk_mode())) {
        bt_sink_srv_music_get_nvdm_data(&(aws_a2dp_dev->dev_addr), BT_SINK_SRV_MUSIC_DATA_VOLUME, &ctx->vol_lev);
    }
#endif
    /* 1. Open A2DP codec */
    uint16_t a2dp_mtu = ctx->a2dp_eir.a2dp_mtu;
    bt_sink_srv_music_fill_am_aud_param(&aud_cap, &aws_a2dp_dev->codec, BT_AWS_ROLE_SINK, a2dp_mtu);
    ctx->am_vol_lev = ctx->vol_lev;
    BT_SINK_SRV_REMOVE_FLAG(aws_a2dp_dev->flag, BT_SINK_SRV_MUSIC_FLAG_WAIT_LIST_SINK_PLAY);
#ifdef AIR_A2DP_SYNC_STOP_ENABLE
    bt_sink_srv_ami_audio_service_hook_callback(AUDIO_MESSAGE_TYPE_BT_A2DP_DL, bt_sink_srv_music_sync_stop_done_handler, NULL);
#endif
    bt_sink_srv_set_clock_offset_ptr_to_dsp((const bt_bd_addr_t *)(&(aws_a2dp_dev->dev_addr)));
    bt_sink_srv_am_result_t am_ret = bt_sink_srv_ami_audio_play(ctx->aws_aid, &aud_cap);
    if (am_ret != AUD_EXECUTION_SUCCESS) {
        /* Exception: play fail */
        BT_SINK_SRV_REMOVE_FLAG(aws_a2dp_dev->flag, BT_SINK_SRV_MUSIC_FLAG_WAIT_AMI_OPEN_CODEC);
        bt_sink_srv_music_update_run_device(NULL);
        bt_sink_srv_music_state_machine_handle(aws_a2dp_dev, BT_SINK_SRV_MUSIC_EVT_PREPARE_FAIL, NULL);
        bt_sink_srv_report_id("[sink][music][mce_a2dp] aws_play(error)--am_ret: %d", 1, am_ret);
    }
}


void bt_sink_srv_aws_mce_a2dp_stop(audio_src_srv_handle_t *handle)
{
    bt_sink_srv_music_device_t *aws_a2dp_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_PSE_HD, (void *)handle);;
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    bt_utils_assert(aws_a2dp_dev);
    bt_sink_srv_report_id("[sink][music][mce_a2dp] stop(s)--hd: 0x%x, type: %d, flag: 0x%x, op: 0x%x, role:0x%x", 5,
                          handle, handle->type, aws_a2dp_dev->flag, aws_a2dp_dev->op, role);
    BT_SINK_SRV_SET_FLAG(aws_a2dp_dev->flag, BT_SINK_SRV_MUSIC_NORMAL_STOP_FLAG);
#ifndef BT_SINK_SRV_IMPROVE_RESYNC
    if (aws_a2dp_dev->flag&BT_SINK_SRV_MUSIC_FLAG_REINITIAL_SYNC) {
#ifdef AIR_LE_AUDIO_ENABLE
        bt_sink_srv_cap_am_disable_waiting_list();
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
        bt_ull_le_am_operate_waiting_list_for_a2dp_resync(false);
#endif
    }
#endif
#ifdef AIR_A2DP_SYNC_STOP_ENABLE
    if ((aws_a2dp_dev->state != AUDIO_SRC_SRV_STATE_PLAYING) || (!(aws_a2dp_dev->conn_bit & BT_SINK_SRV_MUSIC_AWS_CONN_BIT))
        || (bt_sink_srv_music_get_force_stop())) {
        bt_sink_srv_music_set_force_stop(false);
        bt_sink_srv_music_stop(aws_a2dp_dev, ctx->aws_aid);
    }
#else
    bt_sink_srv_music_stop(aws_a2dp_dev, ctx->aws_aid);
#endif

    /* add for partner BLe advertising feature, to start BLE adv about tile */
    if (role == BT_AWS_MCE_ROLE_PARTNER) {
        /* enable conn BLE adv*/
        bt_sink_srv_event_callback(BT_SINK_SRV_EVENT_LE_START_CONNECTABLE_ADV, NULL, 0);
    }
}


void bt_sink_srv_aws_mce_a2dp_suspend(audio_src_srv_handle_t *handle, audio_src_srv_handle_t *int_hd)
{
    bt_sink_srv_music_device_t *aws_a2dp_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_PSE_HD, (void *)handle);
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();

    bt_utils_assert(aws_a2dp_dev && "aws_a2dp_dev NULL");
    bt_sink_srv_report_id("[sink][music][mce_a2dp] suspend(s)--hd:0x%x, type:%d, int:0x%x, type:%d, aws_a2dp_dev:0x%x, flag: 0x%x, op: 0x%x, substate:0x%x, role:0x%x",
        8, handle, handle->type, int_hd, int_hd->type, aws_a2dp_dev, aws_a2dp_dev->flag, aws_a2dp_dev->op, aws_a2dp_dev->handle->substate, role);
    BT_SINK_SRV_REMOVE_FLAG(aws_a2dp_dev->flag, BT_SINK_SRV_MUSIC_FLAG_REINITIAL_SYNC);
    if (BT_SINK_SRV_MUSIC_TRANSIENT_STATE_CLEAR_CODEC == aws_a2dp_dev->handle->substate) {
        return;
    }
    /* Clear codec */
#ifdef AIR_A2DP_SYNC_STOP_ENABLE
    if (aws_a2dp_dev->state != AUDIO_SRC_SRV_STATE_PLAYING || (bt_sink_srv_music_get_force_stop())) {
        bt_sink_srv_music_set_force_stop(false);
        bt_sink_srv_music_stop(aws_a2dp_dev, ctx->aws_aid);
    }
#else
    bt_sink_srv_music_stop(aws_a2dp_dev, ctx->aws_aid);
#endif
    /* add for partner BLe advertising feature, to start BLE adv about tile */
    if (role == BT_AWS_MCE_ROLE_PARTNER) {
        /* enable conn BLE adv*/
        bt_sink_srv_event_callback(BT_SINK_SRV_EVENT_LE_START_CONNECTABLE_ADV, NULL, 0);
    }
}


void bt_sink_srv_aws_mce_a2dp_reject(audio_src_srv_handle_t *handle)
{
    bt_sink_srv_mutex_lock();
    bt_sink_srv_music_device_t *aws_a2dp_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_PSE_HD, (void *)handle);

    bt_utils_assert(aws_a2dp_dev);
    audio_src_srv_add_waiting_list(handle);
    bt_sink_srv_report_id("[sink][music][mce_a2dp] reject(s)--hd: 0x%x, type: %d, flag: 0x%x, op: 0x%x", 4,
                          handle, handle->type, aws_a2dp_dev->flag, aws_a2dp_dev->op);
    BT_SINK_SRV_SET_FLAG(aws_a2dp_dev->flag, BT_SINK_SRV_MUSIC_FLAG_WAIT_LIST_SINK_PLAY);
    /* Notify state machine reject reason */
    bt_sink_srv_music_set_music_enable(aws_a2dp_dev->aws_hd, BT_AVM_ROLE_AWS_MCE_PARTNER, false);
    bt_sink_srv_music_state_machine_handle(aws_a2dp_dev, BT_SINK_SRV_MUSIC_EVT_REJECT, NULL);
    bt_sink_srv_mutex_unlock();
}

static void bt_sink_srv_aws_mce_a2dp_send_eir_data(bt_bd_addr_t *dev_addr, bt_sink_srv_aws_mce_a2dp_base_eir_t *a2dp_eir)
{
    if (!dev_addr || !a2dp_eir) {
        bt_sink_srv_report_id("[sink][music][mce_a2dp] Invalid parameters", 0);
        return;
    }
    uint32_t aws_handle = bt_sink_srv_music_get_aws_handle_by_addr(dev_addr);
    if (!aws_handle) {
        bt_sink_srv_report_id("[sink][music][mce_a2dp] Get aws handle fail", 0);
        return;
    }

#ifdef MTK_BT_SPEAKER_ENABLE
    /* Fill magic code for speaker compatible feature*/
    a2dp_eir->speaker_compatible_flag = 0xabcd5678;
#endif
    bt_aws_mce_agent_state_type_t aws_state = bt_sink_srv_aws_mce_get_aws_state_by_handle(aws_handle);
    bt_aws_mce_information_t mce_if;

    bt_sink_srv_report_id("[sink][music][mce_a2dp] send_if_packet(s), aws_handle:0x%08x,  aws_state:0x%02x", 2, aws_handle, aws_state);
    if (BT_SINK_SRV_MUSIC_MODE_BROADCAST != bt_sink_srv_music_get_spk_mode()
        && BT_AWS_MCE_ROLE_AGENT == bt_connection_manager_device_local_info_get_aws_role()
        && BT_AWS_MCE_AGENT_STATE_CONNECTABLE == aws_state) {
        /* Temp solution, need to optimize by uniform API from AWS report */
        bt_sink_srv_report_id("[sink][music][mce_a2dp] Do not allow to send if packet when not broadcast & Agent & connectable state", 0);
        return;
    }

    mce_if.type = BT_AWS_MCE_INFORMATION_A2DP;
    mce_if.data = (uint8_t *)a2dp_eir;
    mce_if.data_length = sizeof(bt_sink_srv_aws_mce_a2dp_base_eir_t);

    if ((aws_handle && (aws_state & 0x3f))
#ifdef MTK_BT_SPEAKER_ENABLE
        || (aws_handle && (BT_SINK_SRV_MUSIC_MODE_BROADCAST == bt_sink_srv_music_get_spk_mode()))
#endif
       ) {
        bt_aws_mce_send_information(aws_handle, &mce_if, true);
    }

    bt_sink_srv_report_id("[sink][music][mce_a2dp] send_eir_play_ind-1:0x%x,2:0x%x,3:0x%x,4:0x%x,5:0x%x,6:0x%x,7:0x%x,8:0x%x,9:0x%x,10:0x%x,11:0x%x,12:0x%x,13:0x%x",
        13, a2dp_eir->seq_magic,a2dp_eir->state,a2dp_eir->vol_lev,a2dp_eir->sample_count,a2dp_eir->drift_val,a2dp_eir->ratio,
        a2dp_eir->nclk_intra,a2dp_eir->nclk,a2dp_eir->ts,a2dp_eir->latency_val,a2dp_eir->special_device_flag,a2dp_eir->flag,a2dp_eir->a2dp_mtu);
}

static void bt_sink_srv_aws_mce_a2dp_send_eir_start_streaming(uint32_t a2dp_hd)
{
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_sink_srv_music_device_t *a2dp_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_A2DP_HD, (void *)(&a2dp_hd));
    bt_sink_srv_aws_mce_a2dp_base_eir_t a2dp_base_eir = {0};

    bt_utils_assert(a2dp_dev);
    bt_sink_srv_aws_mce_init_basic_eir(&a2dp_base_eir);

    /* Fill EIR A2DP info */
    a2dp_base_eir.state = BT_SINK_SRV_A2DP_STATE_PLAY;
    a2dp_base_eir.vol_lev = ctx->vol_lev;
    a2dp_base_eir.latency_val = bt_sink_srv_music_get_sink_latency();
    if (a2dp_dev->flag & BT_SINK_SRV_MUSIC_FLAG_REINITIAL_SYNC) {
        a2dp_base_eir.flag |= BT_SINK_SRV_AWS_MCE_A2DP_REINIT_FLAG;
    }
    bt_sink_srv_music_mode_t music_mode = bt_sink_srv_music_get_mode(&a2dp_dev->dev_addr);
    if (bt_iot_device_white_list_check_iot_case((bt_bd_addr_t *)&a2dp_dev->dev_addr, BT_IOT_MUSIC_NOTIFY_N9_NO_SLEEP)
        || (music_mode == BT_SINK_SRV_MUSIC_GAME_MODE)) {
        a2dp_base_eir.special_device_flag = BT_SINK_SRV_MUSIC_NOTIFY_N9_NO_SLEEP;
    }
#ifdef AIR_A2DP_REINIT_V2_ENABLE
    if (a2dp_dev->codec.type == BT_A2DP_CODEC_SBC
        && bt_iot_device_white_list_check_iot_case((bt_bd_addr_t *)&a2dp_dev->dev_addr, BT_IOT_MUSIC_IS_WALKMAN_SET_RATIO_AND_PCDC_OBSERVATION)) {
        a2dp_base_eir.special_device_flag |= BT_SINK_SRV_MUSIC_SHORT_PCDC_OBSERVATION;
    }
#endif
    /* Fill magic code */
    a2dp_base_eir.ts = BT_SINK_SRV_AWS_MCE_A2DP_MAGIC_CODE_3;
    a2dp_base_eir.a2dp_mtu = bt_a2dp_get_mtu_size(a2dp_dev->a2dp_hd);
    bt_sink_srv_memcpy(&(a2dp_base_eir.codec), &(a2dp_dev->codec), sizeof(bt_a2dp_codec_capability_t));
    /* Send A2DP EIR packet */
    bt_sink_srv_aws_mce_a2dp_send_eir_data(&(a2dp_dev->dev_addr), &a2dp_base_eir);
    bt_sink_srv_report_id("[sink][music][mce_a2dp] send_eir_start_streaming", 0);
}


static void bt_sink_srv_aws_mce_a2dp_send_eir_suspend_streaming(uint32_t a2dp_hd)
{
    bt_sink_srv_music_device_t *a2dp_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_A2DP_HD, (void *)(&a2dp_hd));
    bt_sink_srv_aws_mce_a2dp_base_eir_t a2dp_base_eir = {0};

    bt_utils_assert(a2dp_dev);
    /* Fill EIR A2DP info */
    a2dp_base_eir.state = BT_SINK_SRV_A2DP_STATE_READY;
    /* Fill magic code */
    a2dp_base_eir.ts = BT_SINK_SRV_AWS_MCE_A2DP_MAGIC_CODE_2;
    if (a2dp_dev->flag & BT_SINK_SRV_MUSIC_FLAG_REINITIAL_SYNC) {
        a2dp_base_eir.flag |= BT_SINK_SRV_AWS_MCE_A2DP_REINIT_FLAG;
    }
#ifdef AIR_A2DP_SYNC_STOP_ENABLE
    if (bt_sink_srv_music_get_force_stop()) {
        a2dp_base_eir.special_device_flag = 0xEE; //partner should force stop
    }
#endif
    /* Send A2DP EIR packet */
    bt_sink_srv_memcpy(&(a2dp_base_eir.codec), &(a2dp_dev->codec), sizeof(bt_a2dp_codec_capability_t));
    bt_sink_srv_aws_mce_a2dp_send_eir_data(&(a2dp_dev->dev_addr), &a2dp_base_eir);
    bt_sink_srv_report_id("[sink][music][mce_a2dp] send_eir_suspend_streaming", 0);
    /* clear fec cached media data && fec data when a2dp suspend */
#ifdef MTK_BT_SPEAKER_ENABLE
#ifdef MTK_BT_SPEAKER_FEC_ENABLE
    if (BT_SINK_SRV_MUSIC_MODE_BROADCAST == bt_sink_srv_music_get_spk_mode()) {
        speaker_fec_init_context();
    }
#endif
#endif
}

static uint16_t update_base_number = 1;
static void bt_sink_srv_aws_mce_a2dp_send_eir_update_base(uint32_t a2dp_hd)
{
    bt_sink_srv_aws_mce_a2dp_base_eir_t a2dp_base_eir = {0};
    bt_sink_srv_music_device_t *a2dp_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_A2DP_HD, (void *)(&a2dp_hd));
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();

    bt_utils_assert(a2dp_dev);
    /* Fill EIR A2DP info */
    a2dp_base_eir.ts = ctx->ts;
    a2dp_base_eir.nclk = ctx->bt_clk.nclk;
    a2dp_base_eir.nclk_intra = ctx->bt_clk.nclk_intra;
    a2dp_base_eir.sample_count = ctx->samples;
    a2dp_base_eir.state = BT_SINK_SRV_A2DP_UP_BASE;
    a2dp_base_eir.drift_val = (int16_t)avm_direct_get_drift();
    if ((BT_SINK_SRV_MUSIC_MODE_BROADCAST != bt_sink_srv_music_get_spk_mode())) {
        a2dp_base_eir.vol_lev = ctx->vol_lev;
    }
    a2dp_base_eir.ratio = (0xffffffff == ctx->ratio) ? BT_SINK_SRV_IPHONE_TYPE_IN_RATIO : ctx->ratio;
    /* Codec info */
    bt_sink_srv_memcpy(&(a2dp_base_eir.codec), &(a2dp_dev->codec), sizeof(bt_a2dp_codec_capability_t));
    a2dp_base_eir.seq_magic = (BT_SINK_SRV_AWS_MCE_A2DP_UPDATE_BASE_MAGIC | update_base_number);
    update_base_number++;
    if (update_base_number == 0) {
        update_base_number++;
    }
    /* Send A2DP EIR packet */
    bt_sink_srv_aws_mce_a2dp_send_eir_data(&(a2dp_dev->dev_addr), &a2dp_base_eir);
    bt_sink_srv_report_id("[sink][music][mce_a2dp] send_eir_update_base-update_base_number:0x%x", 1, update_base_number);
}

void bt_sink_srv_aws_mce_init_basic_eir(bt_sink_srv_aws_mce_a2dp_base_eir_t *eir_buf)
{
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();

    /* Fill EIR A2DP info */
#ifdef __BT_SINK_SRV_A2DP_AVM_DIRECT_SUPPORT__
    eir_buf->seq_magic = 0;
    eir_buf->ts = ctx->ts;
    eir_buf->nclk = ctx->bt_clk.nclk;
    eir_buf->nclk_intra = ctx->bt_clk.nclk_intra;
    eir_buf->ratio = ctx->ratio;
    if (0xffffffff == ctx->ratio) {
        eir_buf->ratio = BT_SINK_SRV_IPHONE_TYPE_IN_RATIO;
    }
    eir_buf->vol_lev = ctx->vol_lev;
    eir_buf->sample_count = ctx->samples;
    eir_buf->drift_val = (int16_t)avm_direct_get_drift();
    eir_buf->latency_val = bt_sink_srv_music_get_sink_latency();
#endif /* __BT_SINK_SRV_A2DP_AVM_DIRECT_SUPPORT__ */
}


static void bt_sink_srv_aws_mce_a2dp_send_eir_vol_sync(uint32_t a2dp_hd, uint8_t change_type)
{
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bt_sink_srv_music_device_t *a2dp_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_A2DP_HD, (void *)(&a2dp_hd));
    bt_sink_srv_aws_mce_a2dp_base_eir_t a2dp_base_eir = {0};

    bt_utils_assert(a2dp_dev);
    bt_sink_srv_aws_mce_init_basic_eir(&a2dp_base_eir);
    if (!(a2dp_dev->op & BT_SINK_SRV_MUSIC_OP_DRV_PLAY)) {
        a2dp_base_eir.ts = BT_SINK_SRV_AWS_MCE_A2DP_MAGIC_CODE_4;
    }

    /* Fill EIR A2DP info */
    a2dp_base_eir.state = BT_SINK_SRV_A2DP_STATE_PLAY;
    if (change_type == BT_SINK_SRV_AWS_A2DP_VOL_CHANGE_UP || change_type == BT_SINK_SRV_AWS_A2DP_VOL_CHANGE_DOWN) {
        a2dp_base_eir.vol_lev = change_type;
#ifdef MTK_BT_SPEAKER_ENABLE
        a2dp_base_eir.vol_change_count = (ctx->volume_sync_count_flag)++;
#endif
    } else {
        a2dp_base_eir.vol_lev = ctx->vol_lev;
    }
    bt_sink_srv_memcpy(&(a2dp_base_eir.codec), &(a2dp_dev->codec), sizeof(bt_a2dp_codec_capability_t));
    /* Send A2DP EIR packet */
    bt_sink_srv_aws_mce_a2dp_send_eir_data(&(a2dp_dev->dev_addr), &a2dp_base_eir);
    bt_sink_srv_report_id("[sink][music][mce_a2dp] send_eir_vol_sync", 0);
}

static uint16_t play_info_number = 1;

static void bt_sink_srv_aws_mce_a2dp_send_eir_play_ind(uint32_t a2dp_hd)
{
    bt_sink_srv_music_device_t *a2dp_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_A2DP_HD, (void *)(&a2dp_hd));
    bt_sink_srv_aws_mce_a2dp_base_eir_t a2dp_base_eir = {0};
    bt_utils_assert(a2dp_dev);

    bt_sink_srv_aws_mce_init_basic_eir(&a2dp_base_eir);
    /* Fill EIR A2DP info */
#ifdef __BT_SINK_SRV_A2DP_AVM_DIRECT_SUPPORT__
    if (bt_iot_device_white_list_check_iot_case((bt_bd_addr_t *)&a2dp_dev->dev_addr, BT_IOT_MUSIC_SET_SPECIAL_SAMPLE_COUNT_TO_MAGIC_NUMBER)) {
        a2dp_base_eir.sample_count = BT_SINK_SRV_SPECIAL_DEV_MAGIC_NUMBER;
    }
    a2dp_base_eir.state = BT_SINK_SRV_A2DP_STATE_PLAY;
    if (a2dp_dev->flag & BT_SINK_SRV_MUSIC_FLAG_REINITIAL_SYNC) {
        a2dp_base_eir.flag |= BT_SINK_SRV_AWS_MCE_A2DP_REINIT_FLAG;
    }

    bt_sink_srv_music_mode_t music_mode = BT_SINK_SRV_MUSIC_NORMAL_MODE;
    music_mode = bt_sink_srv_music_get_mode(&a2dp_dev->dev_addr);
    if (music_mode == BT_SINK_SRV_MUSIC_GAME_MODE) {
        a2dp_base_eir.flag |= BT_SINK_SRV_AWS_MCE_A2DP_ALC_FLAG;
    }
#ifdef AIR_BT_A2DP_VENDOR_2_ENABLE
    if(bt_sink_srv_music_is_lhdc_ll_mode(&(a2dp_dev->codec))){
        a2dp_base_eir.flag |= BT_SINK_SRV_AWS_MCE_A2DP_LHDC_LL_MODE_FLAG;
    }
#endif
    if (bt_iot_device_white_list_check_iot_case((bt_bd_addr_t *)&a2dp_dev->dev_addr, BT_IOT_MUSIC_NOTIFY_N9_NO_SLEEP)) {
        a2dp_base_eir.special_device_flag = BT_SINK_SRV_MUSIC_NOTIFY_N9_NO_SLEEP;
    } else {
        a2dp_base_eir.special_device_flag = 0;
    }
#ifdef AIR_A2DP_REINIT_V2_ENABLE
    if (a2dp_dev->codec.type == BT_A2DP_CODEC_SBC
        && bt_iot_device_white_list_check_iot_case((bt_bd_addr_t *)&a2dp_dev->dev_addr, BT_IOT_MUSIC_IS_WALKMAN_SET_RATIO_AND_PCDC_OBSERVATION)) {
        a2dp_base_eir.special_device_flag |= BT_SINK_SRV_MUSIC_SHORT_PCDC_OBSERVATION;
    }
#endif
#endif /* __BT_SINK_SRV_A2DP_AVM_DIRECT_SUPPORT__ */
    a2dp_base_eir.a2dp_mtu = bt_a2dp_get_mtu_size(a2dp_dev->a2dp_hd);
    a2dp_base_eir.seq_magic = (BT_SINK_SRV_AWS_MCE_A2DP_PLAY_INFO_MAGIC | play_info_number);
    play_info_number++;
    if (play_info_number == 0) {
        play_info_number++;
    }
    extern uint8_t g_sn_wrap_count;
    a2dp_base_eir.ts = (g_sn_wrap_count << 28) | (a2dp_base_eir.ts);
    /* Codec info */
    bt_sink_srv_memcpy(&(a2dp_base_eir.codec), &(a2dp_dev->codec), sizeof(bt_a2dp_codec_capability_t));
    /* Send A2DP EIR packet */
    bt_sink_srv_aws_mce_a2dp_send_eir_data(&(a2dp_dev->dev_addr), &a2dp_base_eir);
    bt_sink_srv_report_id("[sink][music][mce_a2dp] send_eir_play_ind-play_info_number:0x%x", 1, play_info_number);
}

#ifdef MTK_BT_SPEAKER_ENABLE
#define BT_SINK_SRV_SEND_EIR_RETRY_MAX_TIME 3
void bt_sink_srv_a2dp_retry_send_eir_timer(uint32_t timer_id, uint32_t data)
{
    bt_sink_srv_music_device_t *a2dp_dev = NULL;
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    bool need_retry = false;

    bt_sink_srv_report_id("[sink][music][mce_a2dp] retry_send_eir, data:0x%x, retry cmd:0x%x, retry cnt:0x%x",
                          3, data, ctx->retry_send_eir_cmd, ctx->retry_send_eir_cnt);
    switch (ctx->retry_send_eir_cmd) {
        case BT_SINK_SRV_AWS_MCE_A2DP_EVT_SUSPEND_STREAMING: {
            a2dp_dev = bt_sink_srv_music_get_device(BT_SINK_SRV_MUSIC_DEVICE_A2DP_HD, (void *)(&data));
            if (a2dp_dev) {
                bt_sink_srv_aws_mce_a2dp_send_eir_suspend_streaming(data);
                need_retry = true;
            }
            break;
        }

        default:
            break;
    }
    ctx->retry_send_eir_cnt++;
    if (ctx->retry_send_eir_cnt > BT_SINK_SRV_SEND_EIR_RETRY_MAX_TIME) {
        need_retry = false;
    }
    if (need_retry) {
        if (BT_SINK_SRV_MUSIC_MODE_BROADCAST == bt_sink_srv_music_get_spk_mode()) {
            bt_timer_ext_start(BT_SINK_SRV_A2DP_SEND_RETRY_SEND_EIR_TIMER_ID, data,
                               BT_SINK_SRV_A2DP_RETRY_SEND_EIR_TIMER_DUR, bt_sink_srv_a2dp_retry_send_eir_timer);
        }
    }
}
#endif

void bt_sink_srv_aws_mce_a2dp_send_eir(uint32_t event, void *param)
{
    uint32_t *a2dp_hd = NULL;

    bt_sink_srv_report_id("[sink][music][mce_a2dp] send_eir--event: 0x%08x, param:0x%08x", 2,
                          event, param);
    switch (event) {
        case BT_SINK_SRV_AWS_MCE_A2DP_EVT_START_STREAMING: {
            a2dp_hd = (uint32_t *)param;
            bt_sink_srv_aws_mce_a2dp_send_eir_start_streaming(*a2dp_hd);
            break;
        }

        case BT_SINK_SRV_AWS_MCE_A2DP_EVT_SUSPEND_STREAMING: {
            a2dp_hd = (uint32_t *)param;
            bt_sink_srv_aws_mce_a2dp_send_eir_suspend_streaming(*a2dp_hd);
            break;
        }

        case BT_SINK_SRV_AWS_MCE_A2DP_EVT_UPDATE_BASE: {
            a2dp_hd = (uint32_t *)param;
            bt_sink_srv_aws_mce_a2dp_send_eir_update_base(*a2dp_hd);
            break;
        }

        case BT_SINK_SRV_AWS_MCE_A2DP_EVT_VOL_SYNC: {
            a2dp_hd = (uint32_t *)param;
            bt_sink_srv_aws_mce_a2dp_send_eir_vol_sync(*a2dp_hd, 0);
            break;
        }

        case BT_SINK_SRV_AWS_MCE_A2DP_EVT_PLAY_IND: {
            a2dp_hd = (uint32_t *)param;
            bt_sink_srv_aws_mce_a2dp_send_eir_play_ind(*a2dp_hd);
            break;
        }
        case BT_SINK_SRV_AWS_MCE_A2DP_EVT_VOL_UP: {
            a2dp_hd = (uint32_t *)param;
            bt_sink_srv_aws_mce_a2dp_send_eir_vol_sync(*a2dp_hd, BT_SINK_SRV_AWS_A2DP_VOL_CHANGE_UP);
            break;
        }
        case BT_SINK_SRV_AWS_MCE_A2DP_EVT_VOL_DOWN: {
            a2dp_hd = (uint32_t *)param;
            bt_sink_srv_aws_mce_a2dp_send_eir_vol_sync(*a2dp_hd, BT_SINK_SRV_AWS_A2DP_VOL_CHANGE_DOWN);
            break;
        }

        default:
            break;
    }
#ifdef MTK_BT_SPEAKER_ENABLE
    bt_timer_ext_stop(BT_SINK_SRV_A2DP_SEND_RETRY_SEND_EIR_TIMER_ID);
    bt_sink_srv_music_context_t *ctx = bt_sink_srv_music_get_context();
    if (BT_SINK_SRV_MUSIC_MODE_BROADCAST == bt_sink_srv_music_get_spk_mode()) {
        if (BT_SINK_SRV_AWS_MCE_A2DP_EVT_SUSPEND_STREAMING == event) {
            ctx->retry_send_eir_cmd = event;
            ctx->retry_send_eir_cnt = 0;
            bt_timer_ext_start(BT_SINK_SRV_A2DP_SEND_RETRY_SEND_EIR_TIMER_ID, *a2dp_hd,
                               BT_SINK_SRV_A2DP_RETRY_SEND_EIR_TIMER_DUR, bt_sink_srv_a2dp_retry_send_eir_timer);
        }
    }
#endif
}


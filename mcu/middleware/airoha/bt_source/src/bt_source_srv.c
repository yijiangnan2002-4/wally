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

#include "bt_source_srv.h"
#ifdef AIR_SOURCE_SRV_HFP_ENABLE
#include "bt_source_srv_call.h"
#endif
#ifdef AIR_SOURCE_SRV_MUSIC_ENABLE
#include "bt_source_srv_a2dp.h"
#endif
#include "bt_callback_manager.h"
#include "bt_source_srv_utils.h"
#include "bt_source_srv_internal.h"
#include "bt_source_srv_common.h"
#if defined(MTK_AVM_DIRECT)
#include "bt_avm.h"
#endif
#include "bt_sink_srv_ami.h"

#if _MSC_VER >= 1500
#pragma comment(linker, "/alternatename:_bt_source_srv_event_callback=_default_bt_source_srv_event_callback")
#pragma comment(linker, "/alternatename:_bt_source_srv_get_feature_config=_default_bt_source_srv_get_feature_config")
#pragma comment(linker, "/alternatename:_bt_source_srv_get_phone_card_information=_default_bt_source_srv_get_phone_card_information")
#pragma comment(linker, "/alternatename:_bt_source_srv_get_audio_codec_type=_default_bt_source_srv_get_audio_codec_type")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__CC_ARM)
#pragma weak bt_source_srv_event_callback = default_bt_source_srv_event_callback
#pragma weak bt_source_srv_get_feature_config = default_bt_source_srv_get_feature_config
#pragma weak bt_source_srv_get_phone_card_information = default_bt_source_srv_get_phone_card_information
#pragma weak bt_source_srv_get_audio_codec_type = default_bt_source_srv_get_audio_codec_type
#else
#error "Unsupported Platform"
#endif

extern void bt_source_srv_at_cmd_init(void);

static void default_bt_source_srv_event_callback(bt_source_srv_event_t event_id, void *parameter, uint32_t length)
{
    LOG_MSGID_I(source_srv, "[SOURCE][SRV] the user didn't implement event callback, please implement", 0);
}

static bt_status_t default_bt_source_srv_get_feature_config(bt_source_srv_t type, void *feature)
{
    LOG_MSGID_I(source_srv, "[SOURCE][SRV] the user didn't implement get feature config, please implement", 0);
    return BT_STATUS_FAIL;
}

static uint32_t default_bt_source_srv_get_phone_card_information(bt_source_srv_t type, bt_source_srv_phone_card_info_t *phone_card, uint32_t phone_card_num)
{
    LOG_MSGID_I(source_srv, "[SOURCE][SRV] the user didn't implement get phone card information, please implement", 0);
    return 0;
}

static bt_source_srv_codec_t default_bt_source_srv_get_audio_codec_type(bt_source_srv_t type, const bt_addr_t *peer_address)
{
    LOG_MSGID_I(source_srv, "[SOURCE][SRV] the user didn't implement get audio codec type, please implement", 0);
    return BT_SOURCE_SRV_CODEC_TYPE_NONE;
}

static bt_status_t bt_source_srv_common_callback(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    bt_status_t result = BT_STATUS_SUCCESS;
    uint32_t module = msg & 0xFF000000;

#ifdef  AIR_SOURCE_SRV_AVRCP_CT_BQB_ENABLE
    bool bt_srv_avrcp_bqb_in_progress(void);
    bool is_in_bqb = bt_srv_avrcp_bqb_in_progress();
    bt_status_t bt_srv_avrcp_bqb_common_cb(bt_msg_type_t msg, bt_status_t status, void *buff);
#endif

    switch (module) {
#ifdef AIR_SOURCE_SRV_HFP_ENABLE
        case BT_MODULE_HSP:
        case BT_MODULE_HFP: {
            result = bt_source_srv_call_common_callback(msg, status, buffer);
        }
        break;
#endif
#ifdef AIR_SOURCE_SRV_HFP_ENABLE
        case BT_MODULE_A2DP: {

#ifdef  AIR_SOURCE_SRV_AVRCP_CT_BQB_ENABLE
            if (is_in_bqb) {
                return bt_srv_avrcp_bqb_common_cb(msg, status, buffer);
            }
#endif
            {
                result = bt_source_srv_a2dp_common_callback(msg, status, buffer);
            }
        }
        break;
        case BT_MODULE_AVRCP: {
#ifdef  AIR_SOURCE_SRV_AVRCP_CT_BQB_ENABLE
            if (is_in_bqb) {

                return bt_srv_avrcp_bqb_common_cb(msg, status, buffer);
            }
#endif
            result = bt_source_srv_avrcp_callback(msg, status, buffer);
        }
        break;
#endif
        default:
            break;
    }
    return result;
}

static bt_status_t bt_source_srv_register_callback_init(void)
{
    bt_status_t status = BT_STATUS_FAIL;
    status = bt_callback_manager_register_callback(bt_callback_type_app_event,
             (uint32_t)(MODULE_MASK_GAP | MODULE_MASK_SYSTEM | MODULE_MASK_HFP |
                        MODULE_MASK_A2DP | MODULE_MASK_AVRCP), (void *)bt_source_srv_common_callback);

    return status;
}

bt_status_t bt_source_srv_set_clock_offset_ptr_to_dsp(bt_bd_addr_t *address)
{
    bt_source_srv_assert(address && "ERROR: address NULL");
    uint32_t handle = bt_cm_get_gap_handle(*address);
    if (handle == 0) {
        LOG_MSGID_E(source_srv, "[SOURCE][SRV] GAP handle is NULL", 0);
        return BT_SINK_SRV_STATUS_INVALID_PARAM;
    }

    const void *clk_offset_buf = bt_avm_get_clock_offset_address(handle);
    if (clk_offset_buf == NULL) {
        LOG_MSGID_E(source_srv, "[SOURCE][SRV] get clock offset ptr buff is NULL.", 0);
        return BT_SINK_SRV_STATUS_FAIL;
    }

    bt_sink_srv_ami_set_bt_inf_address((bt_sink_srv_am_bt_audio_param_t)clk_offset_buf);
    LOG_MSGID_I(source_srv, "[SOURCE][SRV] set clock offset ptr, handle:0x%08x, buf ptr:0x%08x", 2,
                handle, clk_offset_buf);
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_source_srv_init(const bt_source_srv_init_parameter_t *init_param)
{
    bt_status_t status = BT_STATUS_FAIL;
    /* register callback init. */
    status = bt_source_srv_register_callback_init();

    bt_source_srv_at_cmd_init();

    bt_source_srv_common_init();

#ifdef AIR_SOURCE_SRV_HFP_ENABLE
    /* HFP/HSP call init. */
    bt_source_srv_call_init_parameter_t call_init_param = {
        .hfp_init_param = (bt_source_srv_hfp_init_parameter_t *) &init_param->hfp_init_parameter
    };
    status = bt_source_srv_call_init(&call_init_param);
#endif

#ifdef AIR_SOURCE_SRV_MUSIC_ENABLE
    bt_avrcp_init_t avrcp_param = {0};

    bt_source_srv_a2dp_init();

    avrcp_param.role = BT_AVRCP_ROLE_TG;
    avrcp_param.support_browse = false;
    bt_source_srv_avrcp_init(&avrcp_param);
#endif

    LOG_MSGID_I(source_srv, "[SOURCE][SRV] init status = %02x", 1, status);
    return status;
}

bt_status_t bt_source_srv_send_action(bt_source_srv_action_t action, void *parameter, uint32_t length)
{
    bt_source_srv_mutex_lock();
    bt_status_t status = BT_STATUS_FAIL;
    uint32_t module = (action & BT_SOURCE_MODULE_MASK);
    LOG_MSGID_I(source_srv, "[SOURCE][SRV] send action = %02x", 1, action);
    switch (module) {
        case BT_SOURCE_MODULE_COMMON: {
            status = bt_source_srv_common_send_action(action, parameter, length);
        }
        break;
#ifdef AIR_SOURCE_SRV_HFP_ENABLE
        case BT_SOURCE_MODULE_CALL: {
            status = bt_source_srv_call_send_action(action, parameter, length);
        }
        break;
#endif
#ifdef AIR_SOURCE_SRV_MUSIC_ENABLE
        case BT_SOURCE_MODULE_MUSIC: {
            status = bt_source_srv_a2dp_action_handler(action, parameter, length);
            if (status != BT_STATUS_SUCCESS) {
                status = bt_source_srv_avrcp_action_handler(action, parameter, length);
            }
        }
        break;
#endif
        default:
            break;
    }
    bt_source_srv_mutex_unlock();
    return status;
}


bool bt_source_srv_get_remote_absolute_volume_information(const bt_bd_addr_t *address)
{
    return true;
}

bt_status_t bt_source_srv_get_playing_device_codec(bt_source_srv_get_playing_device_codec_t *playing_codec)
{
    if (playing_codec == NULL) {
        return BT_STATUS_FAIL;
    }

    if (bt_source_srv_common_get_playing_device() == BT_SOURCE_SRV_TYPE_HFP) {
        playing_codec->source_type = BT_SOURCE_SRV_TYPE_HFP;
        playing_codec->hfp_codec = bt_source_srv_call_get_playing_device_codec();
        return BT_STATUS_SUCCESS;
    }
    return BT_STATUS_FAIL;
}
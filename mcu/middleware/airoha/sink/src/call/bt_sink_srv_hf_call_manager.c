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

#include "bt_sink_srv_hf.h"
#include "bt_sink_srv_utils.h"
#include "bt_connection_manager_internal.h"
#include "bt_device_manager_internal.h"
#include "bt_sink_srv_common.h"
#include "bt_sink_srv_call_pseudo_dev.h"
#include "bt_sink_srv_call_pseudo_dev_mgr.h"

#define BT_SINK_SRV_HF_TRIGGER_VR_TIMEOUT   (1000)
#define BT_SINK_SRV_HF_CALL_HOLD_TIMEOUT   (1000)

extern bt_sink_srv_hf_context_t g_sink_srv_hf_context[BT_SINK_SRV_HF_LINK_NUM];
extern bt_sink_srv_hf_context_t *g_sink_srv_hf_missed_call_device_p;
extern bt_sink_srv_hf_context_t *g_sink_srv_hf_last_device_p;
extern bool g_sink_srv_pts_on;

extern void bt_sink_srv_hf_set_highlight_device(bt_sink_srv_hf_context_t *device);
extern void bt_sink_srv_hf_update_last_context(bt_sink_srv_hf_context_t *context, bool is_add);

static bool bt_sink_srv_hf_is_feature_support(bt_sink_srv_action_t action, bt_sink_srv_hf_context_t *device)
{
    bool result = false;
    bt_sink_srv_hf_link_context_t *link_cntx = &(device->link);
    switch (action) {
        case BT_SINK_SRV_ACTION_QUERY_CALL_LIST:
            if (link_cntx->ag_featues & (BT_HFP_AG_FEATURE_3_WAY | BT_HFP_AG_FEATURE_ENHANCED_CALL_STATUS)) {
                result = true;
            }
            break;
        case BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD:
            if ((link_cntx->ag_featues & (BT_HFP_AG_FEATURE_3_WAY | BT_HFP_AG_FEATURE_ENHANCED_CALL_STATUS))
                && (link_cntx->ag_chld_feature & BT_HFP_AG_HOLD_FEATURE_RELEASE_HELD_CALL)) {
                result = true;
            }
            break;
        case BT_SINK_SRV_ACTION_3WAY_RELEASE_ACTIVE_ACCEPT_OTHER:
            if ((link_cntx->ag_featues & (BT_HFP_AG_FEATURE_3_WAY | BT_HFP_AG_FEATURE_ENHANCED_CALL_STATUS))
                && (link_cntx->ag_chld_feature & BT_HFP_AG_HOLD_FEATURE_RELEASE_ACTIVE_CALL)) {
                result = true;
            }
            break;
        case BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER:
            if ((link_cntx->ag_featues & (BT_HFP_AG_FEATURE_3_WAY | BT_HFP_AG_FEATURE_ENHANCED_CALL_STATUS))
                && (link_cntx->ag_chld_feature & BT_HFP_AG_HOLD_FEATURE_HOLD_ACTIVE_CALL)) {
                result = true;
            }
            break;
        case BT_SINK_SRV_ACTION_3WAY_RELEASE_SPECIAL:
            if ((link_cntx->ag_featues & (BT_HFP_AG_FEATURE_3_WAY | BT_HFP_AG_FEATURE_ENHANCED_CALL_CTRL))
                && (link_cntx->ag_chld_feature & BT_HFP_AG_HOLD_FEATURE_RELEASE_SPECIFIC_CALL)) {
                result = true;
            }
            break;
        case BT_SINK_SRV_ACTION_3WAY_HOLD_SPECIAL:
            if ((link_cntx->ag_featues & (BT_HFP_AG_FEATURE_3_WAY | BT_HFP_AG_FEATURE_ENHANCED_CALL_CTRL))
                && (link_cntx->ag_chld_feature & BT_HFP_AG_HOLD_FEATURE_HOLD_SPECIFIC_CALL)) {
                result = true;
            }
            break;
        case BT_SINK_SRV_ACTION_3WAY_ADD_HELD_CALL_TO_CONVERSATION:
            if (link_cntx->ag_featues & BT_HFP_AG_FEATURE_3_WAY &&
                link_cntx->ag_chld_feature & BT_HFP_AG_HOLD_FEATURE_ADD_HELD_CALL) {
                result = true;
            }
            break;
        case BT_SINK_SRV_ACTION_3WAY_EXPLICIT_CALL_TRANSFER:
            if ((link_cntx->ag_featues & BT_HFP_AG_FEATURE_3_WAY) &&
                (link_cntx->ag_chld_feature & BT_HFP_AG_HOLD_FEATURE_CALL_TRANSFER)) {
                result = true;
            }
            break;
        case BT_SINK_SRV_ACTION_VOICE_RECOGNITION_ACTIVATE:
            if (link_cntx->ag_featues & BT_HFP_AG_FEATURE_VOICE_RECOGNITION) {
                result = true;
            }
            break;
        case BT_SINK_SRV_ACTION_HF_ECNR_ACTIVATE:
            if (link_cntx->ag_featues & BT_HFP_AG_FEATURE_ECHO_NOISE) {
                result = true;
            }
            break;
        default:
            break;
    }
    return result;
}

void bt_sink_srv_hf_call_set_state(bt_sink_srv_hf_context_t *link, bt_sink_srv_hf_call_state_t new_state)
{
    if (link) {
        bt_sink_srv_hf_link_context_t *cur_link = &link->link;
        bt_sink_srv_report_id("[CALL][HF][MGR]previous:0x%x, now:0x%x", 2, cur_link->call_state, new_state);
        if (cur_link->call_state != new_state) {
            bt_sink_srv_hf_call_state_t previous = cur_link->call_state;
            cur_link->call_state = new_state;
            bt_sink_srv_hf_mp_state_change(link);
            bt_sink_srv_hf_call_state_change(link, previous, new_state);
        }
    }
}

void bt_sink_srv_hf_handle_setup_ind(bt_sink_srv_hf_context_t *link, bt_hfp_ciev_call_setup_state_t setup)
{
    if (link) {
        bt_sink_srv_hf_link_context_t *cur_link = &link->link;
        switch (setup) {
            case BT_HFP_CIEV_CALL_SETUP_STATE_NONE:
                switch (cur_link->call_state) {
                    case BT_SINK_SRV_HF_CALL_STATE_INCOMING:
                    case BT_SINK_SRV_HF_CALL_STATE_OUTGOING:
                        bt_sink_srv_hf_call_set_state(link, BT_SINK_SRV_HF_CALL_STATE_IDLE);
                        break;
                    case BT_SINK_SRV_HF_CALL_STATE_TWC_INCOMING:
                        BT_SINK_SRV_HF_QUERY_CALL_LIST(cur_link->handle);
                        bt_sink_srv_hf_call_set_state(link, BT_SINK_SRV_HF_CALL_STATE_ACTIVE);
                        break;
                    case BT_SINK_SRV_HF_CALL_STATE_TWC_OUTGOING:
                        bt_sink_srv_hf_call_set_state(link, BT_SINK_SRV_HF_CALL_STATE_HELD_ACTIVE);
                        break;
                    default:
                        break;
                }
                break;
            case BT_HFP_CIEV_CALL_SETUP_STATE_INCOMING:
                switch (cur_link->call_state) {
                    case BT_SINK_SRV_HF_CALL_STATE_IDLE: {
#if defined(MTK_BT_CM_SUPPORT) && defined(AIR_MULTI_POINT_ENABLE)
                        bt_sink_srv_hf_context_t *call_context
                            = bt_sink_srv_hf_find_device_by_call_state(BT_SINK_SRV_HF_CALL_STATE_ALL);
                        bt_sink_srv_hf_context_t *esco_context
                            = bt_sink_srv_hf_get_context_by_flag(BT_SINK_SRV_HF_FLAG_SCO_ACTIVE);
                        if ((call_context == NULL) && (esco_context == NULL)) {
                            bt_cm_connect_t conn_req = {{0}};
                            conn_req.profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS);
                            bt_sink_srv_memcpy(&conn_req.address, &link->link.address, sizeof(bt_bd_addr_t));
                            bt_status_t status = bt_cm_connect(&conn_req);
                            (void)status;
                            bt_sink_srv_report_id("[CALL][HF][MGR]handle_setup_ind, switch AWS link status:0x%x", 1, status);
                        }
#endif
                        bt_sink_srv_hf_call_set_state(link, BT_SINK_SRV_HF_CALL_STATE_INCOMING);
                        break;
                    }
                    case BT_SINK_SRV_HF_CALL_STATE_ACTIVE:
                    case BT_SINK_SRV_HF_CALL_STATE_HELD_ACTIVE:
                    case BT_SINK_SRV_HF_CALL_STATE_HELD_REMAINING:
                    case BT_SINK_SRV_HF_CALL_STATE_MULTIPARTY:
                        bt_sink_srv_hf_call_set_state(link, BT_SINK_SRV_HF_CALL_STATE_TWC_INCOMING);
                        break;
                    default:
                        break;
                }
                break;
            case BT_HFP_CIEV_CALL_SETUP_STATE_OUTGOING:
            case BT_HFP_CIEV_CALL_SETUP_STATE_REMOTE_ALERT: {
                bt_sink_srv_hf_update_last_context(link, true);
                switch (cur_link->call_state) {
                    case BT_SINK_SRV_HF_CALL_STATE_IDLE:
                        bt_sink_srv_hf_call_set_state(link, BT_SINK_SRV_HF_CALL_STATE_OUTGOING);
                        break;
                    case BT_SINK_SRV_HF_CALL_STATE_ACTIVE:
                    case BT_SINK_SRV_HF_CALL_STATE_HELD_ACTIVE:
                    case BT_SINK_SRV_HF_CALL_STATE_HELD_REMAINING:
                        bt_sink_srv_hf_call_set_state(link, BT_SINK_SRV_HF_CALL_STATE_TWC_OUTGOING);
                        break;
                    default:
                        break;
                }
                break;
            }
            default:
                break;
        }
    }
}


void bt_sink_srv_hf_handle_call_ind(bt_sink_srv_hf_context_t *link, bt_hfp_ciev_call_state_t call)
{
    if (link) {
        bt_sink_srv_hf_link_context_t *cur_link = &link->link;
        switch (call) {
            case BT_HFP_CIEV_CALL_STATE_NONE:
                switch (cur_link->call_state) {
                    case BT_SINK_SRV_HF_CALL_STATE_IDLE:
                        break;
                    case BT_SINK_SRV_HF_CALL_STATE_TWC_INCOMING:
                        bt_sink_srv_hf_call_set_state(link, BT_SINK_SRV_HF_CALL_STATE_INCOMING);
                        break;
                    case BT_SINK_SRV_HF_CALL_STATE_TWC_OUTGOING:
                        bt_sink_srv_hf_call_set_state(link, BT_SINK_SRV_HF_CALL_STATE_OUTGOING);
                        break;
                    default:
                        bt_sink_srv_hf_call_set_state(link, BT_SINK_SRV_HF_CALL_STATE_IDLE);
                        break;
                }
                break;
            case BT_HFP_CIEV_CALL_STATE_CALL:
                switch (cur_link->call_state) {
                    case BT_SINK_SRV_HF_CALL_STATE_IDLE:
                    case BT_SINK_SRV_HF_CALL_STATE_INCOMING:
                    case BT_SINK_SRV_HF_CALL_STATE_OUTGOING: {
                        bt_sink_srv_hf_call_set_state(link, BT_SINK_SRV_HF_CALL_STATE_ACTIVE);
                        break;
                    }
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
}

#ifdef MTK_BT_TIMER_EXTERNAL_ENABLE
static void bt_sink_srv_hf_wait_call_ind_timeout_handler(uint32_t timer_id, uint32_t data)
{
    bt_sink_srv_hf_context_t *hf_context = (bt_sink_srv_hf_context_t *)data;
    bt_sink_srv_report_id("[CALL][HF][MGR]Call ind timeout, hf_contex:0x%x", 1, hf_context);
    if (NULL != hf_context) {
        if (hf_context->link.call_state == BT_SINK_SRV_HF_CALL_STATE_HELD_REMAINING) {
            if (((hf_context->link.flag & BT_SINK_SRV_HF_FLAG_SCO_CREATED) != 0) &&
                ((hf_context->link.flag & BT_SINK_SRV_HF_FLAG_SCO_ACTIVE) == 0)) {
                bt_sink_srv_call_psd_state_machine(hf_context->device, BT_SINK_SRV_CALL_EVENT_SCO_ACTIVATE_REQ, NULL);
            }
            bt_sink_srv_hf_call_set_state(hf_context, BT_SINK_SRV_HF_CALL_STATE_ACTIVE);
        }
    }
    return;
}
#endif /*MTK_BT_TIMER_EXTERNAL_ENABLE*/

void bt_sink_srv_hf_handle_held_ind(bt_sink_srv_hf_context_t *link, bt_hfp_ciev_call_hold_state_t held)
{
    if (link) {
        bt_sink_srv_hf_link_context_t *cur_link = &link->link;
        switch (held) {
            case BT_HFP_CIEV_CALL_HOLD_STATE_NONE:
                switch (cur_link->call_state) {
                    case BT_SINK_SRV_HF_CALL_STATE_TWC_OUTGOING:
                        bt_sink_srv_hf_call_set_state(link, BT_SINK_SRV_HF_CALL_STATE_OUTGOING);
                        break;
                    case BT_SINK_SRV_HF_CALL_STATE_HELD_REMAINING: {
                        //bt_sink_srv_hf_call_set_state(link, BT_SINK_SRV_HF_CALL_STATE_ACTIVE);
#ifdef MTK_BT_TIMER_EXTERNAL_ENABLE
                        bt_timer_ext_t *timer_ext = bt_timer_ext_find(BT_SINK_SRV_TIMER_HF_WAIT_CALL_IND);
                        if (timer_ext) {
                            bt_timer_ext_stop(BT_SINK_SRV_TIMER_HF_WAIT_CALL_IND);
                        }
                        bt_timer_ext_start(BT_SINK_SRV_TIMER_HF_WAIT_CALL_IND,
                                           (uint32_t)link,
                                           BT_SINK_SRV_TIMER_HF_WAIT_CALL_IND_DUR,
                                           bt_sink_srv_hf_wait_call_ind_timeout_handler);
#endif/*MTK_BT_TIMER_EXTERNAL_ENABLE*/
                    }
                    break;
                    case BT_SINK_SRV_HF_CALL_STATE_HELD_ACTIVE:
                        BT_SINK_SRV_HF_QUERY_CALL_LIST(cur_link->handle);
                        bt_sink_srv_hf_call_set_state(link, BT_SINK_SRV_HF_CALL_STATE_ACTIVE);
                        break;
                    default:
                        break;
                }
                break;
            case BT_HFP_CIEV_CALL_HOLD_STATE_SOME:
                switch (cur_link->call_state) {
                    case BT_SINK_SRV_HF_CALL_STATE_ACTIVE:
                    case BT_SINK_SRV_HF_CALL_STATE_TWC_INCOMING:
                    case BT_SINK_SRV_HF_CALL_STATE_HELD_REMAINING:
                    case BT_SINK_SRV_HF_CALL_STATE_MULTIPARTY:
                        bt_sink_srv_hf_call_set_state(link, BT_SINK_SRV_HF_CALL_STATE_HELD_ACTIVE);
                        break;
                    default:
                        break;
                }
                break;
            case BT_HFP_CIEV_CALL_HOLD_STATE_ALL:
                switch (cur_link->call_state) {
                    case BT_SINK_SRV_HF_CALL_STATE_ACTIVE:
                    //case BT_SINK_SRV_HF_CALL_STATE_TWC_OUTGOING:
                    case BT_SINK_SRV_HF_CALL_STATE_HELD_ACTIVE:
                    case BT_SINK_SRV_HF_CALL_STATE_MULTIPARTY:
                        bt_sink_srv_hf_call_set_state(link, BT_SINK_SRV_HF_CALL_STATE_HELD_REMAINING);
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
}


void bt_sink_srv_hf_handle_call_info_ind(bt_sink_srv_hf_context_t *link, bt_hfp_call_list_ind_t *clcc)
{
    if (clcc->multiple_party) {
        bt_sink_srv_hf_call_set_state(link, BT_SINK_SRV_HF_CALL_STATE_MULTIPARTY);
    }
}

bt_sink_srv_hf_context_t *bt_sink_srv_hf_find_device_by_call_state(bt_sink_srv_hf_call_state_t call_state)
{
    uint32_t i;
    bt_sink_srv_hf_context_t *device = NULL, *highlight = NULL;
    highlight = bt_sink_srv_hf_get_highlight_device();
    if (NULL != highlight && highlight->link.call_state & call_state) {
        device = highlight;
    } else {
        for (i = 0; i < BT_SINK_SRV_HF_LINK_NUM; i++) {
            if (highlight != &g_sink_srv_hf_context[i] &&
                call_state & g_sink_srv_hf_context[i].link.call_state) {
                device = &g_sink_srv_hf_context[i];
                break;
            }
        }
    }
    return device;
}

bt_sink_srv_hf_context_t *bt_sink_srv_hf_find_device_with_call_idle_palying(void)
{
    uint8_t i = 0;
    bt_sink_srv_hf_context_t *device = NULL;
    for (i = 0; i < BT_SINK_SRV_HF_LINK_NUM; i++) {
        if ((BT_SINK_SRV_HF_CALL_STATE_IDLE == g_sink_srv_hf_context[i].link.call_state) && \
            (bt_sink_srv_call_psd_is_playing(g_sink_srv_hf_context[i].device))) {
            device = &g_sink_srv_hf_context[i];
            break;
        }
    }
    return device;
}

void bt_sink_srv_hf_answer_call(bool accept)
{
    bt_sink_srv_hf_context_t *device;
    bt_sink_srv_hf_context_t *palying_context = NULL;
    if (NULL != (device = bt_sink_srv_hf_find_device_by_call_state(BT_SINK_SRV_HF_CALL_STATE_INCOMING))) {
        if (device != bt_sink_srv_hf_get_highlight_device()) {
            bt_sink_srv_hf_mp_answer(device, accept);
        } else {
            if (accept) {
                /* Solve the problem of not sending call status information to device for computers. */
                palying_context = bt_sink_srv_hf_find_device_with_call_idle_palying();
                if (palying_context != NULL) {
                    bt_sink_srv_call_psd_state_event_notify(palying_context->device, BT_SINK_SRV_CALL_EVENT_SCO_DEACTIVATE_REQ, NULL);
                }
                BT_SINK_SRV_HF_ANSWER_CALL(device->link.handle);
            } else {
                device->link.flag |= BT_SINK_SRV_HF_FLAG_USER_REJECT;
                BT_SINK_SRV_HF_REJECT_CALL(device->link.handle);
            }
        }
    }
}

void bt_sink_srv_hf_terminate_call(void)
{
    bt_sink_srv_hf_context_t *device = bt_sink_srv_hf_get_highlight_device();

    if (device == NULL) {
        return;
    }

    switch (device->link.call_state) {   
        case BT_SINK_SRV_HF_CALL_STATE_OUTGOING:  
        case BT_SINK_SRV_HF_CALL_STATE_ACTIVE: {
            BT_SINK_SRV_HF_TERMINATE_CALL(device->link.handle);
            break;
        }
        
        case BT_SINK_SRV_HF_CALL_STATE_TWC_INCOMING:
        case BT_SINK_SRV_HF_CALL_STATE_TWC_OUTGOING:
        case BT_SINK_SRV_HF_CALL_STATE_HELD_ACTIVE:
        case BT_SINK_SRV_HF_CALL_STATE_MULTIPARTY: {
            BT_SINK_SRV_HF_HOLD_CALL(device->link.handle, BT_SINK_SRV_HF_CHLD_RELEASE_ACTIVE_ACCEPT_OTHER);
            break;
        }
        
        case BT_SINK_SRV_HF_CALL_STATE_HELD_REMAINING: {
            BT_SINK_SRV_HF_HOLD_CALL(device->link.handle, BT_SINK_SRV_HF_CHLD_RELEASE_HELD_REJECT_WAITING);
            break;
        }
        
        default: {
            break;
        }
    }    
#if 0
    bt_sink_srv_hf_context_t *device;
    if (NULL != (device = bt_sink_srv_hf_find_device_by_call_state((bt_sink_srv_hf_call_state_t)
                                                                   (BT_SINK_SRV_HF_CALL_STATE_OUTGOING | BT_SINK_SRV_HF_CALL_STATE_ACTIVE)))) {
        BT_SINK_SRV_HF_TERMINATE_CALL(device->link.handle);
    } else if (NULL != (device = bt_sink_srv_hf_find_device_by_call_state((bt_sink_srv_hf_call_state_t)
                                                                          (BT_SINK_SRV_HF_CALL_STATE_TWC_INCOMING | BT_SINK_SRV_HF_CALL_STATE_TWC_OUTGOING |
                                                                           BT_SINK_SRV_HF_CALL_STATE_HELD_ACTIVE | BT_SINK_SRV_HF_CALL_STATE_MULTIPARTY)))) {
        BT_SINK_SRV_HF_HOLD_CALL(device->link.handle, BT_SINK_SRV_HF_CHLD_RELEASE_ACTIVE_ACCEPT_OTHER);
    } else if (NULL != (device = bt_sink_srv_hf_find_device_by_call_state(BT_SINK_SRV_HF_CALL_STATE_HELD_REMAINING))) {
        BT_SINK_SRV_HF_HOLD_CALL(device->link.handle, BT_SINK_SRV_HF_CHLD_RELEASE_HELD_REJECT_WAITING);
    }
#endif

}

void bt_sink_srv_hf_release_all_held_call(void)
{
    bt_sink_srv_hf_context_t *device;
    if (NULL != (device = bt_sink_srv_hf_find_device_by_call_state(BT_SINK_SRV_HF_CALL_STATE_INCOMING))) {
        bt_sink_srv_hf_answer_call(false);
    } else if (NULL != (device = bt_sink_srv_hf_find_device_by_call_state((bt_sink_srv_hf_call_state_t)
                                                                          (BT_SINK_SRV_HF_CALL_STATE_TWC_INCOMING | BT_SINK_SRV_HF_CALL_STATE_HELD_ACTIVE |
                                                                           BT_SINK_SRV_HF_CALL_STATE_HELD_REMAINING | BT_SINK_SRV_HF_CALL_STATE_TWC_OUTGOING)))) {
        if (bt_sink_srv_hf_is_feature_support(BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD, device)) {
            BT_SINK_SRV_HF_HOLD_CALL(device->link.handle, BT_SINK_SRV_HF_CHLD_RELEASE_HELD_REJECT_WAITING);
        }
    } else {
        bt_sink_srv_hf_context_t *outgoing_device;
        outgoing_device = bt_sink_srv_hf_find_device_by_call_state(BT_SINK_SRV_HF_CALL_STATE_OUTGOING);
        device = bt_sink_srv_hf_find_device_by_call_state(BT_SINK_SRV_HF_CALL_STATE_ACTIVE | BT_SINK_SRV_HF_CALL_STATE_HELD_REMAINING);
        if (outgoing_device && device && (outgoing_device != device)) {
            BT_SINK_SRV_HF_TERMINATE_CALL(device->link.handle);
        }
    }
}

void bt_sink_srv_hf_release_all_active_accept_others(void)
{
    bt_sink_srv_hf_context_t *device;
    device = bt_sink_srv_hf_find_device_by_call_state((bt_sink_srv_hf_call_state_t)
                                                      (BT_SINK_SRV_HF_CALL_STATE_TWC_INCOMING | BT_SINK_SRV_HF_CALL_STATE_HELD_ACTIVE |
                                                       BT_SINK_SRV_HF_CALL_STATE_HELD_REMAINING));
    if (NULL != device) {
        if (bt_sink_srv_hf_is_feature_support(BT_SINK_SRV_ACTION_3WAY_RELEASE_ACTIVE_ACCEPT_OTHER, device)) {
            BT_SINK_SRV_HF_HOLD_CALL(device->link.handle, BT_SINK_SRV_HF_CHLD_RELEASE_ACTIVE_ACCEPT_OTHER);
        }
    } else {
        bt_sink_srv_hf_context_t *incoming_device = bt_sink_srv_hf_find_device_by_call_state(BT_SINK_SRV_HF_CALL_STATE_INCOMING);
        device = bt_sink_srv_hf_find_device_by_call_state(BT_SINK_SRV_HF_CALL_STATE_ACTIVE | BT_SINK_SRV_HF_CALL_STATE_OUTGOING);
        if (incoming_device && device && (incoming_device != device)) {
            BT_SINK_SRV_HF_TERMINATE_CALL(device->link.handle);
            bt_sink_srv_hf_set_highlight_device(incoming_device);
            BT_SINK_SRV_HF_ANSWER_CALL(incoming_device->link.handle);
        }
    }
}

#ifdef MTK_BT_TIMER_EXTERNAL_ENABLE
void bt_sink_srv_hf_hold_call_timeout_handler(uint32_t timer_id, uint32_t data)
{
    bt_sink_srv_hf_context_t *context = (bt_sink_srv_hf_context_t *)data;

    if (context != NULL &&
        bt_sink_srv_hf_is_feature_support(BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER, context)) {
        BT_SINK_SRV_HF_HOLD_CALL(context->link.handle, BT_SINK_SRV_HF_CHLD_HOLD_ACTIVE_ACCEPT_OTHER);
    }
}
#endif

void bt_sink_srv_hf_hold_call_ext(bt_sink_srv_hf_context_t *device)
{
#ifdef MTK_BT_TIMER_EXTERNAL_ENABLE
    uint32_t timer_id = BT_SINK_SRV_HF_HOLD_TIMER_ID_START + device - g_sink_srv_hf_context;

    if (bt_timer_ext_find(timer_id) == NULL) {
        bt_timer_ext_start(timer_id,
                           (uint32_t)device,
                           BT_SINK_SRV_HF_CALL_HOLD_TIMEOUT,
                           bt_sink_srv_hf_hold_call_timeout_handler);
    } else {
        bt_sink_srv_report_id("[CALL][HF][MGR]Call hold is pending", 0);
    }
#else
    BT_SINK_SRV_HF_HOLD_CALL(device->link.handle, BT_SINK_SRV_HF_CHLD_HOLD_ACTIVE_ACCEPT_OTHER);
#endif
}


void bt_sink_srv_hf_hold_all_active_accept_others(void)
{
#ifdef AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE
    bt_sink_srv_hf_context_t *device;
    bt_sink_srv_hf_context_t *held;
    if (NULL != (device = bt_sink_srv_hf_find_device_by_call_state(BT_SINK_SRV_HF_CALL_STATE_INCOMING))) {
        bt_sink_srv_hf_answer_call(true);
    } else if ((bt_sink_srv_hf_find_device_by_call_state(BT_SINK_SRV_HF_CALL_STATE_ALL) ==
                (device = bt_sink_srv_hf_find_device_by_call_state(BT_SINK_SRV_HF_CALL_STATE_ACTIVE))) &&
               (NULL != (held = bt_sink_srv_hf_find_device_by_call_state(BT_SINK_SRV_HF_CALL_STATE_HELD_REMAINING | BT_SINK_SRV_HF_CALL_STATE_OUTGOING)))) {
        // Swap on different smart phone, high light is active only, the held in the another
        bt_sink_srv_hf_mp_swap(device, held);
    } else if ((NULL != (device = bt_sink_srv_hf_find_device_by_call_state((bt_sink_srv_hf_call_state_t)
                                                                           (BT_SINK_SRV_HF_CALL_STATE_TWC_INCOMING | BT_SINK_SRV_HF_CALL_STATE_HELD_ACTIVE |
                                                                            BT_SINK_SRV_HF_CALL_STATE_HELD_REMAINING | BT_SINK_SRV_HF_CALL_STATE_ACTIVE)))) &&
               (NULL == bt_sink_srv_hf_find_device_by_call_state(BT_SINK_SRV_HF_CALL_STATE_OUTGOING))) {
        if (bt_sink_srv_hf_is_feature_support(BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER, device)) {
            bt_sink_srv_hf_hold_call_ext(device);

        }
    }
#else
    bt_sink_srv_hf_context_t *device = bt_sink_srv_hf_get_highlight_device();
    if (NULL == device) {
        bt_sink_srv_report_id("[CALL][HF][MGR] cant found HF context", 0);
        return;
    }
    switch (device->link.call_state) {      
        case BT_SINK_SRV_HF_CALL_STATE_INCOMING: {
           BT_SINK_SRV_HF_ANSWER_CALL(device->link.handle);
           break;
        }
        
        case BT_SINK_SRV_HF_CALL_STATE_TWC_INCOMING:
        case BT_SINK_SRV_HF_CALL_STATE_ACTIVE:
        case BT_SINK_SRV_HF_CALL_STATE_HELD_ACTIVE:
        case BT_SINK_SRV_HF_CALL_STATE_HELD_REMAINING: {
          if (bt_sink_srv_hf_is_feature_support(BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER, device)) {
              bt_sink_srv_hf_hold_call_ext(device);
          }
           break;
        }
        
        default: {
           break;
        }
    }
#endif
}

void bt_sink_srv_hf_hold_device(bt_sink_srv_hf_context_t *device)
{
    if (device && bt_sink_srv_hf_is_feature_support(BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER, device)) {
        if (device->link.handle) {
            bt_sink_srv_hf_hold_call_ext(device);
        }
    }
}

void bt_sink_srv_hf_release_special(uint8_t index)
{
    bt_sink_srv_hf_context_t *device;
    device = bt_sink_srv_hf_find_device_by_call_state(BT_SINK_SRV_HF_CALL_STATE_ACTIVE | BT_SINK_SRV_HF_CALL_STATE_MULTIPARTY);
    if (NULL != device && bt_sink_srv_hf_is_feature_support(BT_SINK_SRV_ACTION_3WAY_RELEASE_SPECIAL, device)) {
        char buffer[BT_SINK_SRV_HF_CMD_LENGTH];
        snprintf((char *)buffer,
                 BT_SINK_SRV_HF_CMD_LENGTH,
                 "AT+CHLD=1%d",
                 index);
        BT_SINK_SRV_HF_HOLD_CALL(device->link.handle, buffer);
    }
}

void bt_sink_srv_hf_hold_special(uint8_t index)
{
    bt_sink_srv_hf_context_t *device;
    device = bt_sink_srv_hf_find_device_by_call_state(BT_SINK_SRV_HF_CALL_STATE_ACTIVE | BT_SINK_SRV_HF_CALL_STATE_MULTIPARTY);
    if (NULL != device && bt_sink_srv_hf_is_feature_support(BT_SINK_SRV_ACTION_3WAY_HOLD_SPECIAL, device)) {
        char buffer[BT_SINK_SRV_HF_CMD_LENGTH];
        snprintf((char *)buffer,
                 BT_SINK_SRV_HF_CMD_LENGTH,
                 "AT+CHLD=2%d",
                 index);
        BT_SINK_SRV_HF_HOLD_CALL(device->link.handle, buffer);
    }
}

void bt_sink_srv_hf_add_held_to_conversation(void)
{
    bt_sink_srv_hf_context_t *device;
    device = bt_sink_srv_hf_find_device_by_call_state((bt_sink_srv_hf_call_state_t)
                                                      (BT_SINK_SRV_HF_CALL_STATE_HELD_ACTIVE |
                                                       BT_SINK_SRV_HF_CALL_STATE_MULTIPARTY));
    if (NULL != device && bt_sink_srv_hf_is_feature_support(BT_SINK_SRV_ACTION_3WAY_ADD_HELD_CALL_TO_CONVERSATION, device)) {
        BT_SINK_SRV_HF_HOLD_CALL(device->link.handle, BT_SINK_SRV_HF_CHLD_ADD_HELD_TO_CONVERSATION);
    }
}

void bt_sink_srv_hf_explicit_call_transfer(void)
{
    bt_sink_srv_hf_context_t *device;
    device = bt_sink_srv_hf_find_device_by_call_state((bt_sink_srv_hf_call_state_t)
                                                      (BT_SINK_SRV_HF_CALL_STATE_HELD_ACTIVE |
                                                       BT_SINK_SRV_HF_CALL_STATE_MULTIPARTY));
    if (NULL != device && bt_sink_srv_hf_is_feature_support(BT_SINK_SRV_ACTION_3WAY_EXPLICIT_CALL_TRANSFER, device)) {
        BT_SINK_SRV_HF_HOLD_CALL(device->link.handle, BT_SINK_SRV_HF_CHLD_EXPLICIT_CALL_TRANSFER);
    }
}

#ifdef MTK_BT_TIMER_EXTERNAL_ENABLE
void bt_sink_srv_hf_trigger_vr_timeout_handler(uint32_t timer_id, uint32_t data)
{
    bt_sink_srv_voice_recognition_status_t status = {{0}};
    bt_sink_srv_hf_context_t *context = (bt_sink_srv_hf_context_t *)data;

    bt_sink_srv_report_id("[CALL][HF][MGR]trigger VR timeout", 0);

    if (context != NULL) {
        if ((context->link.flag & BT_SINK_SRV_HF_FLAG_SCO_CREATED) == 0 &&
            context->link.call_state == BT_SINK_SRV_HF_CALL_STATE_IDLE) {
            status.enable = false;
            bt_sink_srv_memcpy(status.address, context->link.address, sizeof(bt_bd_addr_t));
            bt_sink_srv_event_callback(BT_SINK_SRV_EVENT_HF_VOICE_RECOGNITION_CHANGED, &status, sizeof(status));
        }
    }
}
#endif /* MTK_BT_TIMER_EXTERNAL_ENABLE */

void bt_sink_srv_hf_voice_recognition_activate(bool active)
{
    bt_bd_addr_t addr_list[BT_SINK_SRV_HF_LINK_NUM];
    if (bt_sink_hf_get_connected_device_list(addr_list) > 0) {
        bt_sink_srv_hf_context_t *bt_sink_srv_hf_context_p = (bt_sink_srv_hf_context_t *)bt_sink_srv_hf_get_context_by_address(&addr_list[0]);
        if (NULL != bt_sink_srv_hf_context_p) {
            if (NULL == bt_sink_srv_hf_find_device_by_call_state(BT_SINK_SRV_HF_CALL_STATE_ALL) &&
                bt_sink_srv_hf_is_feature_support(BT_SINK_SRV_ACTION_VOICE_RECOGNITION_ACTIVATE, bt_sink_srv_hf_context_p)) {
                char buffer[BT_SINK_SRV_HF_CMD_LENGTH];
                snprintf((char *)buffer,
                         BT_SINK_SRV_HF_CMD_LENGTH,
                         "AT+BVRA=%d",
                         active);
                BT_SINK_SRV_HF_VOICE_RECOGNITION(bt_sink_srv_hf_context_p->link.handle, buffer);
#ifdef MTK_BT_TIMER_EXTERNAL_ENABLE
                if (bt_timer_ext_find(BT_SINK_SRV_HF_TRIGGER_VR_TIMER_ID) != NULL) {
                    bt_timer_ext_stop(BT_SINK_SRV_HF_TRIGGER_VR_TIMER_ID);
                }
                if (active) {
                    bt_timer_ext_start(BT_SINK_SRV_HF_TRIGGER_VR_TIMER_ID,
                                       (uint32_t)bt_sink_srv_hf_context_p,
                                       BT_SINK_SRV_HF_TRIGGER_VR_TIMEOUT,
                                       bt_sink_srv_hf_trigger_vr_timeout_handler);
                }
#endif /* MTK_BT_TIMER_EXTERNAL_ENABLE */
            }
        }
    }
}

void bt_sink_srv_hf_voice_recognition_activate_ext(bt_sink_srv_action_voice_recognition_activate_ext_t *params)
{
    bt_sink_srv_hf_context_t *context = NULL;
    char buffer[BT_SINK_SRV_HF_CMD_LENGTH] = {0};

    if (params == NULL || params->type != BT_SINK_SRV_DEVICE_EDR) {
        return;
    }

    bt_sink_srv_report_id("[CALL][HF][MGR]voice activate ext, address: 0x%x-%x-%x-%x-%x-%x", 6,
                          params->address[0], params->address[1], params->address[2],
                          params->address[3], params->address[4], params->address[5]);

    context = bt_sink_srv_hf_get_context_by_address(&params->address);

    if (!bt_sink_srv_hf_check_is_connected_by_context(context) ||
        !bt_sink_srv_hf_is_feature_support(BT_SINK_SRV_ACTION_VOICE_RECOGNITION_ACTIVATE, context)) {
        return;
    }

    if (bt_sink_srv_hf_find_device_by_call_state(BT_SINK_SRV_HF_CALL_STATE_ALL) == NULL) {
        snprintf(buffer, BT_SINK_SRV_HF_CMD_LENGTH, "AT+BVRA=%d", params->activate);
        BT_SINK_SRV_HF_VOICE_RECOGNITION(context->link.handle, buffer);

#ifdef MTK_BT_TIMER_EXTERNAL_ENABLE
        if (bt_timer_ext_find(BT_SINK_SRV_HF_TRIGGER_VR_TIMER_ID) != NULL) {
            bt_timer_ext_stop(BT_SINK_SRV_HF_TRIGGER_VR_TIMER_ID);
        }

        if (params->activate) {
            bt_timer_ext_start(
                BT_SINK_SRV_HF_TRIGGER_VR_TIMER_ID,
                (uint32_t)context,
                BT_SINK_SRV_HF_TRIGGER_VR_TIMEOUT,
                bt_sink_srv_hf_trigger_vr_timeout_handler);
        }
#endif
    }
}

void bt_sink_srv_hf_ncer_activate(bool active)
{
    bt_bd_addr_t addr_list[BT_SINK_SRV_HF_LINK_NUM];
    if (bt_sink_hf_get_connected_device_list(addr_list) > 0) {
        bt_sink_srv_hf_context_t *bt_sink_srv_hf_context_p = (bt_sink_srv_hf_context_t *)bt_sink_srv_hf_get_context_by_address(&addr_list[0]);
        if (NULL != bt_sink_srv_hf_context_p) {
            if (NULL == bt_sink_srv_hf_find_device_by_call_state(BT_SINK_SRV_HF_CALL_STATE_ALL) &&
                bt_sink_srv_hf_is_feature_support(BT_SINK_SRV_ACTION_HF_ECNR_ACTIVATE, bt_sink_srv_hf_context_p)) {
                char buffer[BT_SINK_SRV_HF_CMD_LENGTH];
                snprintf((char *)buffer,
                         BT_SINK_SRV_HF_CMD_LENGTH,
                         "AT+NREC=%d",
                         active);
                BT_SINK_SRV_HF_VOICE_RECOGNITION(bt_sink_srv_hf_context_p->link.handle, buffer);
            }
        }
    }
}


void bt_sink_srv_hf_dial_last_number(bt_sink_srv_dial_last_number_t *parameters)
{
    bt_sink_srv_hf_context_t *context= NULL;

    if(parameters != NULL && parameters->type == BT_SINK_SRV_DEVICE_EDR) {
        context = bt_sink_srv_hf_get_context_by_address(&parameters->address);
    } else {
        context = g_sink_srv_hf_last_device_p;
    }

    if (bt_sink_srv_hf_check_is_connected_by_context(context)) {
        BT_SINK_SRV_HF_DIAL_LAST_NUMBER(context->link.handle);
    } else {
        bt_sink_srv_report_id("[CALL][HF][MGR]Dial last, not found the context!", 0);
    }
}

void bt_sink_srv_hf_dial_number(char *number)
{
    if (g_sink_srv_hf_last_device_p && g_sink_srv_hf_last_device_p->link.handle != 0
        && number && bt_sink_srv_strlen(number) > 0) {
        char buffer[BT_SINK_SRV_HF_DIAL_CMD_LENGTH];
        snprintf((char *)buffer,
                 BT_SINK_SRV_HF_DIAL_CMD_LENGTH,
                 "ATD%s;",
                 number);
        BT_SINK_SRV_HF_DIAL_NUMBER(g_sink_srv_hf_last_device_p->link.handle, buffer);
    }
}

void bt_sink_srv_hf_dial_number_ext(bt_sink_srv_dial_number_t *params)
{
    bt_sink_srv_hf_context_t *bt_sink_srv_hf_context_p = NULL;

    if(NULL != params) {
        bt_sink_srv_hf_context_p = bt_sink_srv_hf_get_context_by_address(&params->address);
    }

    if (bt_sink_srv_hf_context_p == NULL) {
        if (g_sink_srv_hf_last_device_p != NULL) {
            bt_sink_srv_hf_context_p = g_sink_srv_hf_last_device_p;
        } else {
            return;
        }
    }

    if (bt_sink_srv_hf_context_p != NULL && params != NULL ) {
        if (bt_sink_srv_hf_context_p->link.handle != 0 &&
        params->number && bt_sink_srv_strlen(params->number) > 0) {
            char buffer[BT_SINK_SRV_HF_DIAL_CMD_LENGTH];
            snprintf((char *)buffer,
                     BT_SINK_SRV_HF_DIAL_CMD_LENGTH,
                     "ATD%s;",
                     params->number);
            BT_SINK_SRV_HF_DIAL_NUMBER(bt_sink_srv_hf_context_p->link.handle, buffer);
        }
    }
}

void bt_sink_srv_hf_dial_missed(void)
{
    if (NULL != g_sink_srv_hf_missed_call_device_p
        && g_sink_srv_hf_missed_call_device_p->link.handle != 0
        && bt_sink_srv_strlen((char *)g_sink_srv_hf_missed_call_device_p->link.caller.number) > 0) {
        char buffer[BT_SINK_SRV_HF_DIAL_CMD_LENGTH];
        snprintf((char *)buffer,
                 BT_SINK_SRV_HF_DIAL_CMD_LENGTH,
                 "ATD%s;",
                 g_sink_srv_hf_missed_call_device_p->link.caller.number);
        BT_SINK_SRV_HF_DIAL_NUMBER(g_sink_srv_hf_missed_call_device_p->link.handle, buffer);
    }
}
void bt_sink_srv_hf_dial_memory(uint8_t *memory)
{
    if (g_sink_srv_hf_last_device_p && g_sink_srv_hf_last_device_p->link.handle != 0
        && memory) {
        char buffer[BT_SINK_SRV_HF_DIAL_CMD_LENGTH];
        snprintf((char *)buffer,
                 BT_SINK_SRV_HF_DIAL_CMD_LENGTH,
                 "ATD>%d;",
                 *memory);
        BT_SINK_SRV_HF_DIAL_MEMORY(g_sink_srv_hf_last_device_p->link.handle, buffer);
    } else {
        bt_sink_srv_report_id("[CALL][HF][MG]Not found the context!", 0);
    }
}

void bt_sink_srv_hf_attach_voice_tag()
{
    if (g_sink_srv_hf_last_device_p && g_sink_srv_hf_last_device_p->link.handle != 0) {
        BT_SINK_SRV_HF_ATTACH_VOICE_TAG(g_sink_srv_hf_last_device_p->link.handle);
    }

}

void bt_sink_srv_hf_switch_audio_path(bt_bd_addr_t *address)
{
    bt_sink_srv_hf_context_t *bt_sink_srv_hf_context_p = NULL;

    if (NULL != address) {
        bt_sink_srv_hf_context_p = bt_sink_srv_hf_get_context_by_address(address);
    }
    if (bt_sink_srv_hf_context_p == NULL) {
        bt_sink_srv_hf_context_p = bt_sink_srv_hf_get_highlight_device();
    }
    
    if(!g_sink_srv_pts_on) {
        if (NULL != bt_sink_srv_hf_context_p && BT_SINK_SRV_HF_CALL_STATE_IDLE != bt_sink_srv_hf_context_p->link.call_state) {

            //avoid QA invallid operation by sink IT cmd.
            if (bt_sink_srv_hf_context_p->link.call_state == BT_SINK_SRV_HF_CALL_STATE_INCOMING) {
                if (!(bt_sink_srv_hf_context_p->link.ag_featues & BT_HFP_AG_FEATURE_IN_BAND_RING)) {
                    return;
                }
            }

            if (bt_sink_srv_hf_context_p->link.flag & BT_SINK_SRV_HF_FLAG_SCO_CREATED) {
                bt_hfp_audio_transfer(bt_sink_srv_hf_context_p->link.handle, BT_HFP_AUDIO_TO_AG);
            } else {
                bt_hfp_audio_transfer(bt_sink_srv_hf_context_p->link.handle, BT_HFP_AUDIO_TO_HF);
            }
        }
    } else {
        if (bt_sink_srv_hf_context_p != NULL) {
            if (bt_sink_srv_hf_context_p->link.flag & BT_SINK_SRV_HF_FLAG_SCO_CREATED) {
                bt_hfp_audio_transfer(bt_sink_srv_hf_context_p->link.handle, BT_HFP_AUDIO_TO_AG);
            } else {
                bt_hfp_audio_transfer(bt_sink_srv_hf_context_p->link.handle, BT_HFP_AUDIO_TO_HF);
            }
        }
    }
}

void bt_sink_srv_hf_switch_audio_device(void)
{
    bt_sink_srv_hf_mp_switch_audio();
}

void bt_sink_srv_hf_query_call_list(bt_bd_addr_t *address)
{
    bt_sink_srv_hf_context_t *bt_sink_srv_hf_context_p = NULL;
    bt_bd_addr_t *address_p = address;
    if (NULL == address_p) {
        address_p = bt_sink_srv_cm_last_connected_device();
    }
    bt_sink_srv_hf_context_p = bt_sink_srv_hf_get_context_by_address(address);
    if (NULL != bt_sink_srv_hf_context_p) {
        if (!(bt_sink_srv_hf_context_p->link.flag & BT_SINK_SRV_HF_FLAG_QUERY_LIST)) {
            bt_sink_srv_hf_context_p->link.flag |= BT_SINK_SRV_HF_FLAG_QUERY_LIST;
            BT_SINK_SRV_HF_QUERY_CALL_LIST(bt_sink_srv_hf_context_p->link.handle);
        }
    }
}

void bt_sink_srv_hf_send_dtmf(bt_sink_srv_send_dtmf_t *request)
{
    bt_sink_srv_hf_context_t *bt_sink_srv_hf_context_p = NULL;
    if (NULL != request) {
        bt_sink_srv_hf_context_p = bt_sink_srv_hf_get_context_by_address(&request->address);
        if ((request->code == '*' || request->code == '#' || (request->code <= '9' && request->code >= '0'))) {
            if (NULL != bt_sink_srv_hf_context_p) {
                char buffer[BT_SINK_SRV_HF_CMD_LENGTH];
                snprintf((char *)buffer,
                         BT_SINK_SRV_HF_CMD_LENGTH,
                         "AT+VTS=%c",
                         request->code);
                BT_SINK_SRV_HF_SEND_DTMF(bt_sink_srv_hf_context_p->link.handle, buffer);
            }
        }
    }
}

void bt_sink_srv_hf_enable_apl_custom_commands(uint32_t handle, const bt_sink_srv_hf_custom_command_xapl_params_t *params)
{
    char buffer[BT_SINK_SRV_HF_LONG_CMD_LENGTH];
    /*  Enable custom AT commands */
    /* Format: AT+XAPL=vendorID-productID-version,features */
    snprintf((char *)buffer,
             BT_SINK_SRV_HF_LONG_CMD_LENGTH,
             "AT+XAPL=%s,%d",
             params->vendor_infomation,
             params->features);
    BT_SINK_SRV_HF_ENABLE_CUSTOM_CMDS(handle, buffer);
}

bt_status_t bt_sink_srv_hf_report_battery(bt_sink_srv_hf_context_t *context, uint8_t battery_level, bool use_hf_indicators)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    char buffer[BT_SINK_SRV_HF_LONG_CMD_LENGTH] = {0};
    if (use_hf_indicators) {
        snprintf(buffer, BT_SINK_SRV_HF_LONG_CMD_LENGTH, "AT+BIEV=2,%d", battery_level);
    } else {
        snprintf(buffer, BT_SINK_SRV_HF_LONG_CMD_LENGTH, "AT+IPHONEACCEV=1,1,%d", battery_level);
    }
    if (bt_sink_srv_hf_check_is_connected_by_context(context)) {
        status = BT_SINK_SRV_HF_ENABLE_CUSTOM_CMDS(context->link.handle, buffer);
    }
    return status;
}

bt_status_t bt_sink_srv_hf_apl_report_battery(uint8_t battery_level)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_sink_srv_report_id("[CALL][HF][MGR]battery: %d", 1, battery_level);
    if (battery_level > 9) {
        return BT_SINK_SRV_STATUS_INVALID_PARAM;
    }
    for (uint32_t i = 0; i < BT_SINK_SRV_HF_LINK_NUM; i++) {
        status = bt_sink_srv_hf_report_battery(&g_sink_srv_hf_context[i], battery_level, false);
    }
    return status;
}

bt_status_t bt_sink_srv_hf_report_battery_ext(uint8_t battery_level)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_sink_srv_report_id("[CALL][HF][MGR]report battery ext, battery_level %d", 1, battery_level);
    if (battery_level > 100) {
        return BT_STATUS_FAIL;
    }
    for (uint32_t i = 0; i < BT_SINK_SRV_HF_LINK_NUM; i++) {
        if (g_sink_srv_hf_context[i].link.hf_indicators_feature & BT_HFP_HF_INDICATORS_FEATURE_BATTERY_LEVEL) {
            status = bt_sink_srv_hf_report_battery(&g_sink_srv_hf_context[i], battery_level, true);
        } else {
            uint8_t level = battery_level / 10;
            status = bt_sink_srv_hf_report_battery(&g_sink_srv_hf_context[i], level > 9 ? 9 : level, false);
        }
    }
    return status;
}

void bt_sink_srv_hf_apl_siri(void)
{
    char buffer[BT_SINK_SRV_HF_LONG_CMD_LENGTH] = {"AT+APLSIRI?"};
    for (uint32_t i = 0; i < BT_SINK_SRV_HF_LINK_NUM; i++) {
        if (g_sink_srv_hf_context[i].link.handle) {
            BT_SINK_SRV_HF_ENABLE_CUSTOM_CMDS(g_sink_srv_hf_context[i].link.handle, buffer);
        }
    }
}

bt_status_t bt_sink_srv_hf_xiaomi_custom(const char *parameter, uint32_t parameter_length)
{
    const char *header = "AT+XIAOMI=";
    const uint32_t header_length = bt_sink_srv_strlen((char *)header);
    bt_status_t status = BT_STATUS_FAIL;
    uint8_t *buffer = bt_sink_srv_memory_alloc(header_length + parameter_length);
    if (buffer == NULL) {
        return BT_STATUS_OUT_OF_MEMORY;
    }
    bt_sink_srv_memcpy(buffer, header, header_length);
    bt_sink_srv_memcpy(buffer + header_length, parameter, parameter_length);
    for (uint32_t i = 0; i < BT_SINK_SRV_CM_MAX_DEVICE_NUMBER; i++) {
        if ((g_sink_srv_hf_context[i].link.handle != BT_HFP_INVALID_HANDLE)
            && (bt_sink_srv_hf_check_is_connected_by_context(&g_sink_srv_hf_context[i]))) {
            bt_status_t send_status = BT_STATUS_FAIL;
            bt_device_manager_db_remote_pnp_info_t info = {0};
            bt_device_manager_remote_find_pnp_info(g_sink_srv_hf_context[i].link.address, &info);
            if (info.vender_id == 0x038F) {
                send_status = bt_hfp_send_command(
                                  g_sink_srv_hf_context[i].link.handle, buffer, header_length + parameter_length);
            }
            if (send_status == BT_STATUS_SUCCESS) {
                status = BT_STATUS_SUCCESS;
            }
            bt_sink_srv_report_id("[CALL][HF][MGR]xiaomi custom, send_status:0x%x",
                                  1, send_status);
        }
    }
    bt_sink_srv_memory_free(buffer);
    return status;
}

bt_status_t bt_sink_srv_hf_mtk_custom(const char *parameter, uint32_t parameter_length)
{
    return bt_sink_srv_hf_mtk_custom_ext(NULL, parameter, parameter_length);
}

bt_status_t bt_sink_srv_hf_mtk_custom_ext(bt_bd_addr_t *address, const char *parameter, uint32_t parameter_length)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_sink_srv_hf_context_t *context = NULL;

    const char *header = "AT+MTK=";
    const uint32_t header_length = bt_sink_srv_strlen((char *)header);
    uint8_t *buffer = bt_sink_srv_memory_alloc(header_length + parameter_length);

    if (buffer == NULL) {
        return BT_STATUS_OUT_OF_MEMORY;
    }

    bt_sink_srv_memcpy(buffer, header, header_length);
    bt_sink_srv_memcpy(buffer + header_length, parameter, parameter_length);

    if (address != NULL) {
        context = bt_sink_srv_hf_get_context_by_address(address);
        if (bt_sink_srv_hf_check_is_connected_by_context(context)) {
            status = bt_hfp_send_command(context->link.handle, buffer, header_length + parameter_length);
        }
    } else {
        for (uint32_t i = 0; i < BT_SINK_SRV_HF_LINK_NUM; i++) {
            context = &g_sink_srv_hf_context[i];
            if (bt_sink_srv_hf_check_is_connected_by_context(context) &&
                bt_hfp_send_command(context->link.handle, buffer, header_length + parameter_length) == BT_STATUS_SUCCESS) {
                status = BT_STATUS_SUCCESS;
            }
        }
    }

    bt_sink_srv_memory_free(buffer);
    bt_sink_srv_report_id("[CALL][HF][MGR]mtk custom, send 0x%x", 1, status);
    return status;
}

bt_status_t bt_sink_srv_hf_custom_command(bt_bd_addr_t *address, const char *command, uint32_t command_length)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_sink_srv_hf_context_t *context = bt_sink_srv_hf_get_context_by_address(address);

    if (bt_sink_srv_hf_check_is_connected_by_context(context) && 0 != command_length) {
        status = bt_hfp_send_command(context->link.handle, (uint8_t *)command, command_length);
    }

    bt_sink_srv_report_id("[CALL][HF][MGR]custom command, status: 0x%x", 1, status);
    return status;
}

bt_sink_srv_state_t bt_sink_srv_hf_get_call_state(void)
{
    bt_sink_srv_hf_context_t *context = bt_sink_srv_hf_get_highlight_device();

    if (context != NULL) {
        return context->link.call_state;
    }

    return BT_SINK_SRV_STATE_NONE;
}

bt_status_t bt_sink_srv_hf_set_mute(bt_sink_srv_mute_t type, bool mute)
{
    bt_status_t status = BT_STATUS_FAIL;
    bt_sink_srv_call_pseudo_dev_t *device = NULL;
    bt_sink_srv_hf_context_t *context = bt_sink_srv_hf_get_highlight_device();

    if (NULL == context || NULL == context->device) {
        return BT_STATUS_FAIL;
    }

    device = (bt_sink_srv_call_pseudo_dev_t *)context->device;
    status = bt_sink_srv_call_audio_set_mute(device->audio_id, type, mute);

#if defined(AIR_BT_INTEL_EVO_ENABLE)
    if (BT_SINK_SRV_MUTE_MICROPHONE == type && bt_sink_srv_hf_check_is_connected_by_context(context)) {
        const char *command = NULL;

        if (mute) {
            command = "AT+VGM=0";
        } else {
            command = "AT+VGM=15";
        }

        BT_SINK_SRV_HF_SYNC_MIC_GAIN(context->link.handle, command);
    }
#endif

    bt_sink_srv_report_id("[CALL][HF]set mute, status: 0x%x", 1, status);
    return status;
}


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


#include "bt_sink_srv_le_call.h"
#include "bt_sink_srv_le_cap.h"
#include "bt_sink_srv_le_volume.h"
#include "bt_sink_srv_le.h"
#include "bt_le_audio_sink.h"

#if defined (AIR_LE_AUDIO_ENABLE) && defined (AIR_LE_AUDIO_CIS_ENABLE)

/**************************************************************************************************
* Define
**************************************************************************************************/
#define BT_SINK_SRV_LE_CALL_MAX_NUM                         (5)
#define BT_SINK_SRV_LE_LINK_MAX_NUM                         (3)


#define BT_SINK_SRV_LE_CALL_SEARCH_IGNORE_LINK_HANDLE       (0x0001)
#define BT_SINK_SRV_LE_CALL_SEARCH_IGNORE_SERVICE_INDEX     (0x0002)
#define BT_SINK_SRV_LE_CALL_SEARCH_IGNORE_CALL_INDEX        (0x0004)
#define BT_SINK_SRV_LE_CALL_SEARCH_IGNORE_CALL_STATE        (0x0008)

#define BT_SINK_SRV_LE_CALL_SEARCH_ALL                      (0xFFFF)
#define BT_SINK_SRV_LE_CALL_SEARCH_BY_HANDLE                (~BT_SINK_SRV_LE_CALL_SEARCH_IGNORE_LINK_HANDLE)
#define BT_SINK_SRV_LE_CALL_SEARCH_BY_STATE                 (~BT_SINK_SRV_LE_CALL_SEARCH_IGNORE_CALL_STATE)
#define BT_SINK_SRV_LE_CALL_SEARCH_BY_HANDLE_SERVICE_INDEX  \
    (BT_SINK_SRV_LE_CALL_SEARCH_IGNORE_CALL_INDEX | BT_SINK_SRV_LE_CALL_SEARCH_IGNORE_CALL_STATE)
#define BT_SINK_SRV_LE_CALL_SEARCH_BY_HANDLE_CALL_STATE     \
    (BT_SINK_SRV_LE_CALL_SEARCH_IGNORE_SERVICE_INDEX | BT_SINK_SRV_LE_CALL_SEARCH_IGNORE_CALL_INDEX)



/**************************************************************************************************
* Structure
**************************************************************************************************/
typedef struct {
    bt_handle_t connect_handle;
    uint8_t service_index;
    uint8_t uri_length;
    uint8_t uri[BT_SINK_SRV_LE_CALL_MAX_URI_LENGTH];
} bt_sink_srv_le_call_originate_info_t;

typedef struct {
    bt_sink_srv_le_call_info_t *call_list;
    bt_sink_srv_le_call_originate_info_t last_call;
    bt_sink_srv_le_call_originate_info_t missed_call;
    bt_handle_t le_link_handle[BT_SINK_SRV_LE_LINK_MAX_NUM];
} bt_sink_srv_le_call_manager_t;

typedef struct {
    bt_le_audio_call_state_t focus_state;
    bt_le_audio_call_state_t other_state;
    uint32_t sink_action;
    bt_le_audio_sink_action_t focus_action;
    bt_le_audio_sink_action_t other_action;
} bt_sink_srv_le_call_dispatch_table_t;

typedef struct {
    bt_le_audio_call_state_t focus_state;
    bt_le_audio_call_state_t other_state;
    bt_sink_srv_state_t sink_state;
} bt_sink_srv_le_call_state_map_t;

/**************************************************************************************************
* Variables
**************************************************************************************************/

static bt_sink_srv_le_call_info_t g_le_call_container[BT_SINK_SRV_LE_CALL_MAX_NUM];
static bt_sink_srv_le_call_manager_t g_le_call_manager;


static const bt_sink_srv_le_call_dispatch_table_t g_le_call_dispatch_action_tbl[] = {
    /* ---------------- single call ------------------ */
    {
        BLE_TBS_STATE_INCOMING,
        BLE_TBS_STATE_INVALID,
        BT_SINK_SRV_ACTION_ANSWER,
        BT_LE_AUDIO_SINK_ACTION_CALL_ACCEPT,
        BT_LE_AUDIO_SINK_ACTION_INVALID
    },

    {
        BLE_TBS_STATE_INCOMING,
        BLE_TBS_STATE_INVALID,
        BT_SINK_SRV_ACTION_REJECT,
        BT_LE_AUDIO_SINK_ACTION_CALL_TERMINATE,
        BT_LE_AUDIO_SINK_ACTION_INVALID
    },

    {
        BLE_TBS_STATE_INCOMING,
        BLE_TBS_STATE_INVALID,
        BT_SINK_SRV_ACTION_HANG_UP,
        BT_LE_AUDIO_SINK_ACTION_CALL_TERMINATE,
        BT_LE_AUDIO_SINK_ACTION_INVALID
    },

    {
        BLE_TBS_STATE_DIALING,
        BLE_TBS_STATE_INVALID,
        BT_SINK_SRV_ACTION_HANG_UP,
        BT_LE_AUDIO_SINK_ACTION_CALL_TERMINATE,
        BT_LE_AUDIO_SINK_ACTION_INVALID
    },

    {
        BLE_TBS_STATE_ACTIVE,
        BLE_TBS_STATE_INVALID,
        BT_SINK_SRV_ACTION_HANG_UP,
        BT_LE_AUDIO_SINK_ACTION_CALL_TERMINATE,
        BT_LE_AUDIO_SINK_ACTION_INVALID
    },

    {
        BLE_TBS_STATE_ACTIVE,
        BLE_TBS_STATE_INVALID,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_LE_AUDIO_SINK_ACTION_CALL_HOLD,
        BT_LE_AUDIO_SINK_ACTION_INVALID
    },

    {
        BLE_TBS_STATE_LOCALLY_HELD,
        BLE_TBS_STATE_INVALID,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_LE_AUDIO_SINK_ACTION_CALL_RETRIEVE,
        BT_LE_AUDIO_SINK_ACTION_INVALID
    },

    {
        BLE_TBS_STATE_LOCALLY_HELD,
        BLE_TBS_STATE_INVALID,
        BT_SINK_SRV_ACTION_HANG_UP,
        BT_LE_AUDIO_SINK_ACTION_CALL_TERMINATE,
        BT_LE_AUDIO_SINK_ACTION_INVALID
    },

    /* ------------- multi call ------------------ */
    {
        BLE_TBS_STATE_INCOMING,
        BLE_TBS_STATE_INCOMING,
        BT_SINK_SRV_ACTION_ANSWER,
        BT_LE_AUDIO_SINK_ACTION_CALL_ACCEPT,
        BT_LE_AUDIO_SINK_ACTION_INVALID
    },

    {
        BLE_TBS_STATE_INCOMING,
        BLE_TBS_STATE_INCOMING,
        BT_SINK_SRV_ACTION_REJECT,
        BT_LE_AUDIO_SINK_ACTION_CALL_TERMINATE,
        BT_LE_AUDIO_SINK_ACTION_INVALID
    },

    {
        BLE_TBS_STATE_INCOMING,
        BLE_TBS_STATE_DIALING,
        BT_SINK_SRV_ACTION_HANG_UP,
        BT_LE_AUDIO_SINK_ACTION_INVALID,
        BT_LE_AUDIO_SINK_ACTION_CALL_TERMINATE
    },

    {
        BLE_TBS_STATE_INCOMING,
        BLE_TBS_STATE_DIALING,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_LE_AUDIO_SINK_ACTION_CALL_ACCEPT,
        BT_LE_AUDIO_SINK_ACTION_CALL_TERMINATE
    },

    {
        BLE_TBS_STATE_INCOMING,
        BLE_TBS_STATE_DIALING,
        BT_SINK_SRV_ACTION_ANSWER,
        BT_LE_AUDIO_SINK_ACTION_CALL_ACCEPT,
        BT_LE_AUDIO_SINK_ACTION_INVALID
    },

    {
        BLE_TBS_STATE_INCOMING,
        BLE_TBS_STATE_DIALING,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        BT_LE_AUDIO_SINK_ACTION_CALL_TERMINATE,
        BT_LE_AUDIO_SINK_ACTION_INVALID
    },

    {
        BLE_TBS_STATE_INCOMING,
        BLE_TBS_STATE_ACTIVE,
        BT_SINK_SRV_ACTION_HANG_UP,
        BT_LE_AUDIO_SINK_ACTION_INVALID,
        BT_LE_AUDIO_SINK_ACTION_CALL_TERMINATE
    },

    {
        BLE_TBS_STATE_INCOMING,
        BLE_TBS_STATE_ACTIVE,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_LE_AUDIO_SINK_ACTION_CALL_ACCEPT,
        BT_LE_AUDIO_SINK_ACTION_CALL_HOLD
    },

    {
        BLE_TBS_STATE_INCOMING,
        BLE_TBS_STATE_ACTIVE,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        BT_LE_AUDIO_SINK_ACTION_CALL_TERMINATE,
        BT_LE_AUDIO_SINK_ACTION_INVALID
    },

    {
        BLE_TBS_STATE_INCOMING,
        BLE_TBS_STATE_LOCALLY_HELD,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_LE_AUDIO_SINK_ACTION_CALL_ACCEPT,
        BT_LE_AUDIO_SINK_ACTION_INVALID
    },

    {
        BLE_TBS_STATE_INCOMING,
        BLE_TBS_STATE_LOCALLY_HELD,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        BT_LE_AUDIO_SINK_ACTION_CALL_TERMINATE,
        BT_LE_AUDIO_SINK_ACTION_INVALID
    },

    {
        BLE_TBS_STATE_DIALING,
        BLE_TBS_STATE_DIALING,
        BT_SINK_SRV_ACTION_HANG_UP,
        BT_LE_AUDIO_SINK_ACTION_CALL_TERMINATE,
        BT_LE_AUDIO_SINK_ACTION_INVALID
    },

    {
        BLE_TBS_STATE_DIALING,
        BLE_TBS_STATE_ACTIVE,
        BT_SINK_SRV_ACTION_HANG_UP,
        BT_LE_AUDIO_SINK_ACTION_CALL_TERMINATE,
        BT_LE_AUDIO_SINK_ACTION_INVALID
    },

    {
        BLE_TBS_STATE_DIALING,
        BLE_TBS_STATE_ACTIVE,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_LE_AUDIO_SINK_ACTION_INVALID,
        BT_LE_AUDIO_SINK_ACTION_CALL_HOLD
    },

    {
        BLE_TBS_STATE_DIALING,
        BLE_TBS_STATE_LOCALLY_HELD,
        BT_SINK_SRV_ACTION_HANG_UP,
        BT_LE_AUDIO_SINK_ACTION_CALL_TERMINATE,
        BT_LE_AUDIO_SINK_ACTION_INVALID
    },

    {
        BLE_TBS_STATE_DIALING,
        BLE_TBS_STATE_LOCALLY_HELD,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        BT_LE_AUDIO_SINK_ACTION_INVALID,
        BT_LE_AUDIO_SINK_ACTION_CALL_TERMINATE
    },

    {
        BLE_TBS_STATE_ACTIVE,
        BLE_TBS_STATE_ACTIVE,
        BT_SINK_SRV_ACTION_HANG_UP,
        BT_LE_AUDIO_SINK_ACTION_CALL_TERMINATE,
        BT_LE_AUDIO_SINK_ACTION_INVALID
    },

    {
        BLE_TBS_STATE_ACTIVE,
        BLE_TBS_STATE_ACTIVE,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_LE_AUDIO_SINK_ACTION_CALL_HOLD,
        BT_LE_AUDIO_SINK_ACTION_INVALID
    },

    {
        BLE_TBS_STATE_ACTIVE,
        BLE_TBS_STATE_LOCALLY_HELD,
        BT_SINK_SRV_ACTION_HANG_UP,
        BT_LE_AUDIO_SINK_ACTION_CALL_TERMINATE,
        BT_LE_AUDIO_SINK_ACTION_INVALID
    },

    {
        BLE_TBS_STATE_ACTIVE,
        BLE_TBS_STATE_LOCALLY_HELD,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_LE_AUDIO_SINK_ACTION_CALL_HOLD,
        BT_LE_AUDIO_SINK_ACTION_CALL_RETRIEVE
    },

    {
        BLE_TBS_STATE_ACTIVE,
        BLE_TBS_STATE_LOCALLY_HELD,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        BT_LE_AUDIO_SINK_ACTION_INVALID,
        BT_LE_AUDIO_SINK_ACTION_CALL_TERMINATE
    },

    {
        BLE_TBS_STATE_LOCALLY_HELD,
        BLE_TBS_STATE_LOCALLY_HELD,
        BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER,
        BT_LE_AUDIO_SINK_ACTION_CALL_RETRIEVE,
        BT_LE_AUDIO_SINK_ACTION_INVALID
    },

    {
        BLE_TBS_STATE_LOCALLY_HELD,
        BLE_TBS_STATE_LOCALLY_HELD,
        BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD,
        BT_LE_AUDIO_SINK_ACTION_CALL_TERMINATE,
        BT_LE_AUDIO_SINK_ACTION_INVALID
    },

};

static const bt_sink_srv_le_call_state_map_t g_le_call_state_map_tbl[] = {
    /* --------------- single state ----------------- */
    {
        BLE_TBS_STATE_INCOMING,
        BLE_TBS_STATE_INVALID,
        BT_SINK_SRV_STATE_INCOMING
    },

    {
        BLE_TBS_STATE_DIALING,
        BLE_TBS_STATE_INVALID,
        BT_SINK_SRV_STATE_OUTGOING
    },

    {
        BLE_TBS_STATE_ACTIVE,
        BLE_TBS_STATE_INVALID,
        BT_SINK_SRV_STATE_ACTIVE
    },

    {
        BLE_TBS_STATE_LOCALLY_HELD,
        BLE_TBS_STATE_INVALID,
        BT_SINK_SRV_STATE_HELD_REMAINING
    },

    /* ---------------- multi state ----------------- */
    {
        BLE_TBS_STATE_INCOMING,
        BLE_TBS_STATE_INCOMING,
        BT_SINK_SRV_STATE_INCOMING
    },

    {
        BLE_TBS_STATE_INCOMING,
        BLE_TBS_STATE_DIALING,
        BT_SINK_SRV_STATE_TWC_INCOMING
    },

    {
        BLE_TBS_STATE_INCOMING,
        BLE_TBS_STATE_ACTIVE,
        BT_SINK_SRV_STATE_TWC_INCOMING
    },

    {
        BLE_TBS_STATE_INCOMING,
        BLE_TBS_STATE_LOCALLY_HELD,
        BT_SINK_SRV_STATE_TWC_INCOMING
    },

    {
        BLE_TBS_STATE_DIALING,
        BLE_TBS_STATE_DIALING,
        BT_SINK_SRV_STATE_OUTGOING
    },

    {
        BLE_TBS_STATE_DIALING,
        BLE_TBS_STATE_ACTIVE,
        BT_SINK_SRV_STATE_TWC_OUTGOING
    },

    {
        BLE_TBS_STATE_DIALING,
        BLE_TBS_STATE_LOCALLY_HELD,
        BT_SINK_SRV_STATE_TWC_OUTGOING
    },

    {
        BLE_TBS_STATE_ACTIVE,
        BLE_TBS_STATE_ACTIVE,
        BT_SINK_SRV_STATE_ACTIVE
    },

    {
        BLE_TBS_STATE_ACTIVE,
        BLE_TBS_STATE_LOCALLY_HELD,
        BT_SINK_SRV_STATE_HELD_ACTIVE
    },

    {
        BLE_TBS_STATE_LOCALLY_HELD,
        BLE_TBS_STATE_LOCALLY_HELD,
        BT_SINK_SRV_STATE_HELD_REMAINING
    }
};

/**************************************************************************************************
* Prototype
**************************************************************************************************/
extern bool bt_sink_srv_aird_support_call_mode(bt_handle_t handle);

static bt_sink_srv_le_call_info_t *bt_sink_srv_le_get_empty_info(void);
static bt_sink_srv_le_call_info_t *bt_sink_srv_le_get_last_node(void);
static void bt_sink_srv_le_add_node(bt_sink_srv_le_call_info_t *node);
static void bt_sink_srv_le_remove_node(bt_sink_srv_le_call_info_t *node);
static bt_sink_srv_le_call_info_t *bt_sink_srv_le_search_call(
    uint16_t ignore, bt_handle_t handle, uint8_t service_idx, uint8_t call_idx, uint8_t call_state, uint8_t search_idx);
static void bt_sink_srv_le_clear_info_by_handle(bt_handle_t handle);
static void bt_sink_srv_le_update_originate_info(bt_sink_srv_le_call_originate_info_t *mini_info, bt_sink_srv_le_call_info_t *node);
static void bt_sink_srv_le_update_call_state(
    bt_handle_t handle, uint8_t service_idx, uint8_t call_idx, uint8_t call_state, uint16_t uri_len, uint8_t *uri);
static uint8_t bt_sink_srv_le_get_opcode_by_action(bt_le_audio_sink_action_t le_action);
static bt_status_t bt_sink_srv_le_handle_gtbs_call_action(bt_handle_t handle, uint8_t call_index, bt_le_audio_sink_action_t le_action);
static bt_status_t bt_sink_srv_le_gtbs_originate_call(bt_handle_t handle, uint8_t *uri, uint16_t len);
static bt_le_audio_call_state_t bt_sink_srv_le_switch_tbs_state(bt_le_audio_call_state_t state_in);
static bt_status_t bt_sink_srv_le_get_action_by_state(
    uint32_t sink_action,
    bt_le_audio_call_state_t focus_state,
    bt_le_audio_call_state_t other_state,
    bt_le_audio_sink_action_t *focus_le_action,
    bt_le_audio_sink_action_t *other_le_action);
static bool bt_sink_srv_le_check_action(bt_le_audio_sink_action_t le_action, bt_sink_srv_le_call_info_t *call);
static bt_status_t bt_sink_srv_le_handle_call_action(uint32_t action, void *param);
static bt_handle_t bt_sink_srv_le_call_get_volume_config_handle(void);

#define bt_sink_srv_le_search_first_call_by_handle(conn_handle)         \
    bt_sink_srv_le_search_call(BT_SINK_SRV_LE_CALL_SEARCH_BY_HANDLE, (conn_handle), 0, 0, 0, 0)
#define bt_sink_srv_le_search_first_call_by_handle_and_state(conn_handle, call_state)         \
    bt_sink_srv_le_search_call(BT_SINK_SRV_LE_CALL_SEARCH_BY_HANDLE, (conn_handle), 0, 0, (call_state), 0)
#define bt_sink_srv_le_search_first_incoming_call()                     \
    bt_sink_srv_le_search_call(BT_SINK_SRV_LE_CALL_SEARCH_BY_STATE, 0, 0, 0, BLE_TBS_STATE_INCOMING, 0)
#define bt_sink_srv_le_search_first_active_call()                     \
    bt_sink_srv_le_search_call(BT_SINK_SRV_LE_CALL_SEARCH_BY_STATE, 0, 0, 0, BLE_TBS_STATE_ACTIVE, 0)
#define bt_sink_srv_le_search_first_call()                     \
    bt_sink_srv_le_search_call(BT_SINK_SRV_LE_CALL_SEARCH_ALL, 0, 0, 0, 0, 0)
#define bt_sink_srv_le_search_call_by_handle(conn_handle, search_idx)       \
    bt_sink_srv_le_search_call(BT_SINK_SRV_LE_CALL_SEARCH_BY_HANDLE, (conn_handle), 0, 0, 0, (search_idx))

#define bt_sink_srv_le_update_last_call(node)  bt_sink_srv_le_update_originate_info(&g_le_call_manager.last_call, (node))
#define bt_sink_srv_le_update_missed_call(node)  bt_sink_srv_le_update_originate_info(&g_le_call_manager.missed_call, (node))


/**************************************************************************************************
* Static Functions
**************************************************************************************************/
static bool default_bt_sink_srv_aird_support_call_mode(bt_handle_t handle)
{
    return false;
}

#if _MSC_VER >= 1500
#pragma comment(linker, "/altername:_bt_sink_srv_aird_support_call_mode=_default_bt_sink_srv_aird_support_call_mode")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__CC_ARM)
#pragma weak bt_sink_srv_aird_support_call_mode = default_bt_sink_srv_aird_support_call_mode
#else
#error "Unsupported Platform"
#endif



static bt_sink_srv_le_call_info_t *bt_sink_srv_le_get_empty_info(void)
{
    uint16_t i;
    for (i = 0; i < BT_SINK_SRV_LE_CALL_MAX_NUM; i++) {
        if (false == g_le_call_container[i].used) {
            memset(&g_le_call_container[i], 0, sizeof(bt_sink_srv_le_call_info_t));
            g_le_call_container[i].used = true;
            g_le_call_container[i].call_state = BLE_TBS_STATE_INVALID;
            return &g_le_call_container[i];
        }
    }
    return NULL;
}

static bt_sink_srv_le_call_info_t *bt_sink_srv_le_get_last_node(void)
{
    bt_sink_srv_le_call_info_t *res = g_le_call_manager.call_list;
    while(res) {
        if (NULL == res->next) {
            break;
        } else {
            res = res->next;
        }
    }
    return res;
}

static void bt_sink_srv_le_add_node(bt_sink_srv_le_call_info_t *node)
{
    // add node to end
    bt_sink_srv_le_call_info_t *last_node = bt_sink_srv_le_get_last_node();
    if (NULL != node) {
        node->next = NULL;
        if (NULL == last_node) {
            g_le_call_manager.call_list = node;
        } else {
            last_node->next = node;
        }
#if 0  // add node to head
        node->next = g_le_call_manager.call_list;
        g_le_call_manager.call_list = node;
#endif
        //bt_sink_srv_report_id("[Sink][LE][Call][Node] add", 0);
    }
}

static void bt_sink_srv_le_remove_node(bt_sink_srv_le_call_info_t *node)
{
    if (NULL != node) {
        if (node == g_le_call_manager.call_list) {
            g_le_call_manager.call_list = node->next;
        } else {
            bt_sink_srv_le_call_info_t *pre = g_le_call_manager.call_list;
            bt_sink_srv_le_call_info_t *now = pre->next;
            while (NULL != now) {
                if (node == now) {
                    pre->next = now->next;
                    break;
                } else {
                    pre = now;
                    now = now->next;
                }
            }
        }
        memset(node, 0, sizeof(bt_sink_srv_le_call_info_t));
        //bt_sink_srv_report_id("[Sink][LE][Call][Node] remove", 0);
    }
}

static bt_sink_srv_le_call_info_t *bt_sink_srv_le_search_call(
    uint16_t ignore, bt_handle_t handle, uint8_t service_idx, uint8_t call_idx, uint8_t call_state, uint8_t search_idx)
{
    bt_sink_srv_le_call_info_t *node = g_le_call_manager.call_list;
    uint8_t match_idx = 0;
    while (NULL != node) {
        if (((ignore & BT_SINK_SRV_LE_CALL_SEARCH_IGNORE_LINK_HANDLE) || handle == node->connect_handle)
            && ((ignore & BT_SINK_SRV_LE_CALL_SEARCH_IGNORE_SERVICE_INDEX) || service_idx == node->service_index)
            && ((ignore & BT_SINK_SRV_LE_CALL_SEARCH_IGNORE_CALL_INDEX) || call_idx == node->call_index)
            && ((ignore & BT_SINK_SRV_LE_CALL_SEARCH_IGNORE_CALL_STATE) || call_state == node->call_state)) {
            if (match_idx == search_idx) {
                return node;
            } else if (match_idx < search_idx) {
                match_idx++;
                node = node->next;
            }
        } else {
            node = node->next;
        }
    }
    return NULL;
}


static void bt_sink_srv_le_clear_info_by_handle(bt_handle_t handle)
{
    uint16_t i;
    bt_sink_srv_le_call_info_t *p_info = bt_sink_srv_le_search_first_call_by_handle(handle);
    while (NULL != p_info) {
        bt_sink_srv_le_remove_node(p_info);
        p_info = bt_sink_srv_le_search_first_call_by_handle(handle);
    }
    if (handle == g_le_call_manager.last_call.connect_handle) {
        memset(&g_le_call_manager.last_call, 0, sizeof(bt_sink_srv_le_call_originate_info_t));
    }
    if (handle == g_le_call_manager.missed_call.connect_handle) {
        memset(&g_le_call_manager.missed_call, 0, sizeof(bt_sink_srv_le_call_originate_info_t));
    }
    for (i = 0; i < BT_SINK_SRV_LE_LINK_MAX_NUM; i++) {
        if (handle == g_le_call_manager.le_link_handle[i]) {
            g_le_call_manager.le_link_handle[i] = 0;
        }
    }
}

static void bt_sink_srv_le_update_originate_info(bt_sink_srv_le_call_originate_info_t *mini_info, bt_sink_srv_le_call_info_t *node)
{
    if (NULL == node || NULL == mini_info) {
        return ;
    }
    mini_info->connect_handle = node->connect_handle;
    mini_info->service_index = node->service_index;
    mini_info->uri_length = 0;
    if (BT_SINK_SRV_LE_CALL_MAX_URI_LENGTH > node->uri_length && 0 < node->uri_length) {
        mini_info->uri_length = node->uri_length;
        memcpy(&mini_info->uri[0], &node->uri[0], node->uri_length);
    }
}

static void bt_sink_srv_le_update_call_state(
    bt_handle_t handle, uint8_t service_idx, uint8_t call_idx, uint8_t call_state, uint16_t uri_len, uint8_t *uri)
{
    bt_sink_srv_le_call_info_t* node = bt_sink_srv_le_search_call(BT_SINK_SRV_LE_CALL_SEARCH_IGNORE_CALL_STATE, handle, service_idx, call_idx, 0, 0);

    bt_sink_srv_report_id("[Sink][LE][Call] update call, handle: 0x%x, service index: 0x%x, call index:0x%x, call state: 0x%x, uri_len:%d",
        5, handle, service_idx, call_idx, call_state, uri_len);
    if (NULL != node) {
        if (BLE_TBS_STATE_INVALID == call_state || BLE_TBS_STATE_IDLE == call_state) {
            // call end
            bt_sink_srv_le_update_last_call(node);
            if (BLE_TBS_STATE_INCOMING == node->call_state) {
                bt_sink_srv_le_update_missed_call(node);
            }
            bt_sink_srv_le_remove_node(node);
            return ;
        } else {
            node->call_state = call_state;
        }
    } else {
        if (BLE_TBS_INVALID_CALL_INDEX == call_idx || BLE_TBS_STATE_INVALID == call_state || BLE_TBS_STATE_IDLE == call_state) {
            //bt_sink_srv_report_id("[Sink][LE][Call] invalid call, don't need malloc info", 0);
            return;
        }
        // new call
        node = bt_sink_srv_le_get_empty_info();
        if (NULL != node) {
            node->connect_handle = handle;
            node->service_index = service_idx;
            node->call_index = call_idx;
            node->call_state = call_state;
        } else {
            bt_sink_srv_report_id("[Sink][LE][Call][Error] call info malloc fail", 0);
            return ;
        }
        bt_sink_srv_le_add_node(node);
    }
    // update uri
    if (0 < uri_len && BT_SINK_SRV_LE_CALL_MAX_URI_LENGTH >= uri_len && NULL != uri) {
        memcpy(&node->uri[0], uri, uri_len);
        node->uri_length = uri_len;
    }
}

static uint8_t bt_sink_srv_le_get_opcode_by_action(bt_le_audio_sink_action_t le_action)
{
    const bt_le_audio_sink_action_t tbl[] = {
        BT_LE_AUDIO_SINK_ACTION_CALL_ACCEPT,
        BT_LE_AUDIO_SINK_ACTION_CALL_TERMINATE,
        BT_LE_AUDIO_SINK_ACTION_CALL_HOLD,
        BT_LE_AUDIO_SINK_ACTION_CALL_RETRIEVE,
        BT_LE_AUDIO_SINK_ACTION_CALL_ORIGINATE,
        BT_LE_AUDIO_SINK_ACTION_CALL_JOIN
        };
    uint8_t opcode;
    uint16_t tbl_size = sizeof(tbl) / sizeof(tbl[0]);
    for (opcode = 0; opcode < tbl_size; opcode++) {
        if (tbl[opcode] == le_action) {
            return opcode;
        }
    }
    return 0xFF;
}

static bt_status_t bt_sink_srv_le_handle_gtbs_call_action(bt_handle_t handle, uint8_t call_index, bt_le_audio_sink_action_t le_action)
{
    bt_le_audio_sink_call_action_param_t le_param = {
        .service_idx = BLE_CCP_SERVICE_INDEX_GTBS,
        .length = 2,
    };
    uint8_t buf[2] = {0};
    uint8_t opcode = bt_sink_srv_le_get_opcode_by_action(le_action);

    if (BLE_TBS_INVALID_CALL_INDEX == call_index || 0xFF == opcode) {
        return BT_STATUS_FAIL;
    }

    bt_sink_srv_report_id("[Sink][LE][Call] gtbs action:%x, handle:%x, call index:%x", 3, le_action, handle, call_index);
    le_param.call_control_point = (ble_tbs_call_control_point_t *)buf;
    le_param.call_control_point->opcode = opcode;
    le_param.call_control_point->params.call_index = call_index;
    return bt_le_audio_sink_send_action(handle, le_action, &le_param);
}

static bt_status_t bt_sink_srv_le_gtbs_originate_call(bt_handle_t handle, uint8_t *uri, uint16_t len)
{
    bt_le_audio_sink_call_action_param_t le_param = {
        .service_idx = BLE_CCP_SERVICE_INDEX_GTBS,
        .length = len + 1,  /* +1 is opcode */
    };
    uint8_t buf[BT_SINK_SRV_LE_CALL_MAX_URI_LENGTH + 1] = {0};

    if (NULL == uri || 0 >= len || BT_SINK_SRV_LE_CALL_MAX_URI_LENGTH < len) {
        return BT_STATUS_FAIL;
    }

    bt_sink_srv_report_id("[Sink][LE][Call] gtbs originate call, handle:0x%x, uri len:%d", 2, handle, len);
    le_param.call_control_point = (ble_tbs_call_control_point_t *)buf;
    le_param.call_control_point->opcode = BLE_TBS_CALL_CONTROL_OPCODE_TYPE_ORIGINATE;
    memcpy(&le_param.call_control_point->params.originate_call.uri[0], uri, len);
    return bt_le_audio_sink_send_action(handle, BT_LE_AUDIO_SINK_ACTION_CALL_ORIGINATE, &le_param);
}

static bt_le_audio_call_state_t bt_sink_srv_le_switch_tbs_state(bt_le_audio_call_state_t state_in)
{
    bt_le_audio_call_state_t state_out;
    // switch alerting to dialing
    state_out = (state_in == BLE_TBS_STATE_ALERTING) ? BLE_TBS_STATE_DIALING : state_in;
    // switch locally and remotely held and remotely held to locally held
    state_out = (state_out == BLE_TBS_STATE_LOCALLY_AND_REMOTELY_HELD || state_out == BLE_TBS_STATE_REMOTELY_HELD) ? BLE_TBS_STATE_LOCALLY_HELD : state_out;
    // switch idle to invalid
    state_out = (state_out == BLE_TBS_STATE_IDLE) ? BLE_TBS_STATE_INVALID : state_out;
    return state_out;
}

static bt_status_t bt_sink_srv_le_get_action_by_state(
    uint32_t sink_action,
    bt_le_audio_call_state_t focus_state,
    bt_le_audio_call_state_t other_state,
    bt_le_audio_sink_action_t *focus_le_action,
    bt_le_audio_sink_action_t *other_le_action)
{
    // focus le action or other le action can be NULL, to return single call remapping result
    if (NULL == focus_le_action && NULL == other_le_action) {
        return BT_STATUS_FAIL;
    }

    bt_le_audio_call_state_t state1 = bt_sink_srv_le_switch_tbs_state(focus_state);
    bt_le_audio_call_state_t state2 = bt_sink_srv_le_switch_tbs_state(other_state);
    uint16_t tbl_size = sizeof(g_le_call_dispatch_action_tbl) / sizeof(g_le_call_dispatch_action_tbl[0]);
    uint16_t i;

    for (i = 0; i < tbl_size; i++) {
        if (g_le_call_dispatch_action_tbl[i].sink_action == sink_action) {
            if(g_le_call_dispatch_action_tbl[i].focus_state == state1 && g_le_call_dispatch_action_tbl[i].other_state == state2) {
                if (NULL != focus_le_action) {
                    *focus_le_action = g_le_call_dispatch_action_tbl[i].focus_action;
                }
                if (NULL != other_le_action) {
                    *other_le_action = g_le_call_dispatch_action_tbl[i].other_action;
                }
                return BT_STATUS_SUCCESS;
            } else if (g_le_call_dispatch_action_tbl[i].focus_state == state2 && g_le_call_dispatch_action_tbl[i].other_state == state1) {
                // swap focus state and other state
                if (NULL != focus_le_action) {
                    *focus_le_action = g_le_call_dispatch_action_tbl[i].other_action;
                }
                if (NULL != other_le_action) {
                    *other_le_action = g_le_call_dispatch_action_tbl[i].focus_action;
                }
                return BT_STATUS_SUCCESS;
            }
        }
    }
    return BT_STATUS_FAIL;
}

static bool bt_sink_srv_le_check_action(bt_le_audio_sink_action_t le_action, bt_sink_srv_le_call_info_t *call)
{
    if (NULL == call || BT_LE_AUDIO_SINK_ACTION_INVALID == le_action) {
        return false;
    }
    if (BLE_TBS_INVALID_CALL_INDEX == call->call_index || BLE_TBS_STATE_INVALID == call->call_state || BLE_TBS_STATE_IDLE == call->call_state) {
        return false;
    }
    // remotely held call cannot retrieve by local
    if (BT_LE_AUDIO_SINK_ACTION_CALL_RETRIEVE == le_action && BLE_TBS_STATE_REMOTELY_HELD == call->call_state) {
        return false;
    }
    return true;
}

static bt_status_t bt_sink_srv_le_handle_call_action(uint32_t action, void *param)
{
    (void)param;
    bt_le_audio_sink_action_t le_action[3] = { // [0]: focus, [1]: other, [2]: temp for swap
        BT_LE_AUDIO_SINK_ACTION_INVALID,
        BT_LE_AUDIO_SINK_ACTION_INVALID,
        BT_LE_AUDIO_SINK_ACTION_INVALID};
    bt_le_audio_call_state_t focus_state, other_state;
    bt_sink_srv_le_call_info_t *call[3] = {NULL, NULL, NULL}; // [0]: focus, [1]: other, [2]: temp for swap

    call[0] = g_le_call_manager.call_list;
    if (NULL == call[0]) {
        return BT_STATUS_FAIL;
    }
    call[1] = call[0]->next;
    focus_state = call[0]->call_state;
    other_state = BLE_TBS_STATE_INVALID;
    if (NULL != call[1]) {
#if defined(AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE)
        /* customized UI: always operate the second call */
        other_state = call[1]->call_state;
#else
        /* sdk UI: operate the second call only if the second call is from the same Source of the first call */
        if (call[0]->connect_handle == call[1]->connect_handle) {
            other_state = call[1]->call_state;
        }
#endif
    }
    if (BT_STATUS_SUCCESS == bt_sink_srv_le_get_action_by_state(action, focus_state, other_state, &le_action[0], &le_action[1])) {
        if (BT_LE_AUDIO_SINK_ACTION_INVALID != le_action[1]) {
            if (BT_LE_AUDIO_SINK_ACTION_CALL_ACCEPT == le_action[0] || BT_LE_AUDIO_SINK_ACTION_CALL_RETRIEVE == le_action[0]) {
                // swap action order
                le_action[2] = le_action[0];
                le_action[0] = le_action[1];
                le_action[1] = le_action[2];
                call[2] = call[0];
                call[0] = call[1];
                call[1] = call[2];
                call[2] = NULL;
            }
        }
        uint8_t i;
        bt_status_t res = BT_STATUS_SUCCESS;
        for (i = 0; i < 2; i++) {
            if (true == bt_sink_srv_le_check_action(le_action[i], call[i])) {
                bt_status_t resi = bt_sink_srv_le_handle_gtbs_call_action(call[i]->connect_handle, call[i]->call_index, le_action[i]);
                res = resi == BT_STATUS_FAIL ? resi : res;
            }
        }
        return res;
    } else {
        bt_sink_srv_report_id("[Sink][LE][Call] invalid action", 0);
        return BT_STATUS_FAIL;
    }
}

static bt_handle_t bt_sink_srv_le_call_get_volume_config_handle(void)
{
    bt_sink_srv_cap_am_mode mode = bt_sink_srv_cap_am_get_current_mode();
    bt_handle_t handle = bt_sink_srv_cap_get_ble_link_by_streaming_mode(mode);
    if (mode > CAP_AM_UNICAST_CALL_MODE_END) {
        bt_sink_srv_le_call_info_t *node = bt_sink_srv_le_search_first_active_call();
        if (NULL == node) {
            node = bt_sink_srv_le_search_first_incoming_call();
        }
        if (NULL == node) {
            node = bt_sink_srv_le_search_first_call();
        }
        if (NULL != node) {
            handle = node->connect_handle;
        } else {
            uint16_t i;
            for (i = 0; i < BT_SINK_SRV_LE_LINK_MAX_NUM; i++) {
                if (0 != g_le_call_manager.le_link_handle[i]) {
                    handle = g_le_call_manager.le_link_handle[i];
                }
            }
        }
    }
    return handle;
}

/**************************************************************************************************
* Public Functions
**************************************************************************************************/

bt_status_t bt_sink_srv_le_call_action_handler(uint32_t action, void *param)
{
    bt_status_t ret = BT_STATUS_FAIL;
    uint32_t le_action = bt_sink_srv_cap_get_le_audio_action(action);

    switch (action) {
        /* User Event */
        case BT_SINK_SRV_ACTION_ANSWER:
        case BT_SINK_SRV_ACTION_REJECT:
        case BT_SINK_SRV_ACTION_HANG_UP:
        case BT_SINK_SRV_ACTION_3WAY_RELEASE_ALL_HELD:
        case BT_SINK_SRV_ACTION_3WAY_HOLD_ACTIVE_ACCEPT_OTHER: {
            ret = bt_sink_srv_le_handle_call_action(action, param);
            break;
        }
        case BT_SINK_SRV_ACTION_DIAL_LAST: {
            bt_sink_srv_dial_last_number_t *dial = NULL;
            if (bt_sink_srv_aird_support_call_mode(g_le_call_manager.last_call.connect_handle)) {
                return BT_STATUS_FAIL;
            }
            if (param) {
                dial = (bt_sink_srv_dial_last_number_t *)param;
                bt_sink_srv_report_id("[Sink][LE][Call] dial last action type: %d", 1, dial->type);
                if (dial->type != BT_SINK_SRV_DEVICE_LE) {
                    return BT_STATUS_FAIL;
                }
            }
            ret = bt_sink_srv_le_gtbs_originate_call(
                g_le_call_manager.last_call.connect_handle, g_le_call_manager.last_call.uri, g_le_call_manager.last_call.uri_length);
            break;
        }
        case BT_SINK_SRV_ACTION_DIAL_MISSED: {
            ret = bt_sink_srv_le_gtbs_originate_call(
                g_le_call_manager.missed_call.connect_handle, g_le_call_manager.missed_call.uri, g_le_call_manager.missed_call.uri_length);
            break;
        }

        case BT_SINK_SRV_ACTION_CALL_VOLUME_UP:
        case BT_SINK_SRV_ACTION_CALL_VOLUME_DOWN:
        case BT_SINK_SRV_ACTION_CALL_VOLUME_MAX:
        case BT_SINK_SRV_ACTION_CALL_VOLUME_MIN:
        case BT_SINK_SRV_ACTION_CALL_SET_VOLUME: {
            bt_handle_t handle = bt_sink_srv_le_call_get_volume_config_handle();
            ret = bt_sink_srv_le_volume_vcp_send_action(handle, le_action, NULL);
            break;
        }
        default: {
            //bt_sink_srv_report_id("[CALL][HF]Unexcepted action:0x%x", 1, action);
            ret = BT_STATUS_FAIL;
            break;
        }
    }
    return ret;
}

bt_status_t bt_sink_srv_le_call_init(void)
{
    memset(&g_le_call_container[0], 0, sizeof(g_le_call_container));
    memset(&g_le_call_manager, 0, sizeof(g_le_call_manager));
    return BT_STATUS_SUCCESS;
}

void le_sink_srv_le_call_event_callback(uint16_t event_id, void *p_msg)
{
    //bt_sink_srv_report_id("[Sink][LE][Call] event callback:0x%x", 1, event_id);

    switch (event_id) {
        case BT_LE_AUDIO_SINK_EVENT_CALL_SERVICE_READY: {
            bt_le_audio_sink_event_service_ready_t *noti = (bt_le_audio_sink_event_service_ready_t *)p_msg;
            if (BT_STATUS_SUCCESS == noti->status) {
                uint16_t i;
                for (i = 0; i < BT_SINK_SRV_LE_LINK_MAX_NUM; i++) {
                    if (0 == g_le_call_manager.le_link_handle[i]) {
                        g_le_call_manager.le_link_handle[i] = noti->handle;
                        break;
                    }
                }
            }
            break;
        }

        case BT_LE_AUDIO_SINK_EVENT_DISCONNECTED: {
            if (NULL != p_msg) {
                bt_le_audio_sink_event_disconnected_t *noti = (bt_le_audio_sink_event_disconnected_t *)p_msg;
                bt_sink_srv_le_clear_info_by_handle(noti->handle);
            }
            break;
        }

        case BT_LE_AUDIO_SINK_EVENT_CALL_STATE: {
            bt_le_audio_sink_event_call_state_t *noti = (bt_le_audio_sink_event_call_state_t *)p_msg;
            if (NULL == noti) {
                return ;
            }
            bt_sink_srv_le_update_call_state(noti->handle, noti->service_idx, noti->call_index, noti->cur_state, noti->uri_length, noti->uri);
            break;
        }

        default:
            break;
    }

}
#if 0
bt_status_t bt_sink_srv_le_call_ccp_callback(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    // no longer used
    return BT_STATUS_SUCCESS;
}
#endif

bt_sink_srv_state_t bt_sink_srv_le_remap_call_state(bt_le_audio_call_state_t le_state1, bt_le_audio_call_state_t le_state2)
{
    bt_sink_srv_state_t res = BT_SINK_SRV_STATE_NONE;
    bt_le_audio_call_state_t state1 = bt_sink_srv_le_switch_tbs_state(le_state1);
    bt_le_audio_call_state_t state2 = bt_sink_srv_le_switch_tbs_state(le_state2);
    uint16_t i;
    uint16_t tbl_size = sizeof(g_le_call_state_map_tbl) / sizeof(g_le_call_state_map_tbl[0]);

    for (i = 0; i < tbl_size;i++) {
        if ((g_le_call_state_map_tbl[i].focus_state == state1 && g_le_call_state_map_tbl[i].other_state == state2)
            || (g_le_call_state_map_tbl[i].focus_state == state2 && g_le_call_state_map_tbl[i].other_state == state1)) {
            return g_le_call_state_map_tbl[i].sink_state;
        }
    }
    return res;
}

bt_sink_srv_state_t bt_sink_srv_le_get_call_state_by_handle(bt_handle_t handle)
{
    bt_sink_srv_le_call_info_t *call[2] = {NULL, NULL};
    bt_le_audio_call_state_t state[2] = {BLE_TBS_STATE_INVALID, BLE_TBS_STATE_INVALID};
    uint8_t i;
    for (i = 0; i < 2; i++) {
        call[i] = bt_sink_srv_le_search_call_by_handle(handle, i);
        if (NULL != call[i]) {
            state[i] = call[i]->call_state;
        }
    }
    return bt_sink_srv_le_remap_call_state(state[0], state[1]);
}

bt_status_t bt_sink_srv_le_place_call(uint8_t *uri, uint16_t uri_len)
{
    uint16_t i;
    for (i = 0; i < BT_SINK_SRV_LE_LINK_MAX_NUM; i++) {
        if (0 != g_le_call_manager.le_link_handle[i]) {
            return bt_sink_srv_le_gtbs_originate_call(g_le_call_manager.le_link_handle[i], uri, uri_len);
        }
    }
    return BT_STATUS_FAIL;
}


#endif





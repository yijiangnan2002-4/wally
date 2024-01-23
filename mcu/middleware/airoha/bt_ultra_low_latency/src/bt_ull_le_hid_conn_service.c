/* Copyright Statement:
 *
 * (C) 2023  Airoha Technology Corp. All rights reserved.
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

#include "bt_ull_le_hid_conn_service.h"
#include "bt_ull_le_utility.h"

#include "bt_callback_manager.h"
#include "bt_ull_le_hid_device_manager.h"

#define BT_ULL_LE_HID_CONN_SRV_DEBUG

#define BT_ULL_LE_HID_CONN_LOG   "[ULL][HID][CONN]"

#define MAKE_CMD_PARAM(N, T) T *N = (T*)pvPortMalloc(sizeof(T))
#define BT_ULL_LE_HID_MAKE_CMD_PARAM(name, CMD) MAKE_CMD_PARAM(name, CMD##_T)
#define BT_ULL_LE_HID_GET_CMD_PARAM_LEN(CMD) sizeof(CMD##_T)
#define BT_ULL_LE_HID_GET_CMD_PARAM(CMD, node) (CMD##_T *)node->cache_item.data
#define BT_ULL_LE_HID_CONN_SRV_AIR_PARAMS_MAX_NUM           13
#define BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM                  0x04
#define BT_ULL_LE_HID_CONN_SRV_INVALID                     (0xFF)
#define BT_ULL_LE_HID_CONN_SRV_LOCK                        (0x01)

#define BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTED        (0x00)
#define BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CONNECTING          (0x01)
#define BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CANCEL_CONNECTING   (0x02)
#define BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CONNECTED           (0x03)
#define BT_ULL_LE_HID_CONN_SRV_AIR_CIS_SET_DATA_PATH       (0x04)
#define BT_ULL_LE_HID_CONN_SRV_AIR_CIS_STREAMING           (0x05)
#define BT_ULL_LE_HID_CONN_SRV_AIR_CIS_REMOVE_DATA_PATH    (0x06)
#define BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTING       (0x07)
#define BT_ULL_LE_HID_CONN_SRV_AIR_CIS_STATE_MAX           (0x08)
typedef uint8_t bt_ull_le_hid_conn_srv_air_cis_state_t;

#define BT_ULL_LE_HID_CONN_SRV_CIG_STATE_NONE              (0x00)
#define BT_ULL_LE_HID_CONN_SRV_CIG_STATE_CREATING          (0x01)
#define BT_ULL_LE_HID_CONN_SRV_CIG_STATE_CREATED           (0x02)
#define BT_ULL_LE_HID_CONN_SRV_CIG_STATE_REMOVING          (0x03)
typedef uint8_t bt_ull_le_hid_conn_srv_cig_state_t;

#define BT_ULL_LE_HID_CONN_SRV_SWITCH_SCENARIO_STATE_NONE              (0x00)
#define BT_ULL_LE_HID_CONN_SRV_SWITCH_SCENARIO_STATE_CREATING          (0x01)
#define BT_ULL_LE_HID_CONN_SRV_SWITCH_SCENARIO_STATE_CREATED           (0x02)
#define BT_ULL_LE_HID_CONN_SRV_SWITCH_SCENARIO_STATE_REMOVING          (0x03)
typedef uint8_t bt_ull_le_hid_conn_srv_change_scenario_state_t;


#define BT_ULL_LE_HID_CONN_SRV_CIG_ID_NONE                 (0x00)    /* none */
#define BT_ULL_LE_HID_CONN_SRV_CIG_ID_1                    (0x01)    /* unidirectional (c->s) */

#define BT_ULL_LE_HID_CONN_SRV_FT_0                        (0x00)
#define BT_ULL_LE_HID_CONN_SRV_FT_1                        (0x01)
#define BT_ULL_LE_HID_CONN_SRV_FT_2                        (0x02)
#define BT_ULL_LE_HID_CONN_SRV_FT_3                        (0x03)
#define BT_ULL_LE_HID_CONN_SRV_FT_4                        (0x04)
#define BT_ULL_LE_HID_CONN_SRV_FT_5                        (0x05)
typedef uint8_t bt_ull_le_conn_srv_flush_timeout_t; /*times*/


typedef uint16_t bt_ull_le_hid_conn_srv_cmd_lock;

typedef struct {
    uint8_t           cis_id;
    uint8_t           device_type;
    bt_handle_t       cis_handle;
    bt_handle_t       acl_handle;
    uint8_t           state;
    uint32_t          conn_to;
    bt_addr_t         peer_addr;
} bt_ull_le_hid_conn_srv_cis_link_t;

typedef struct {
    uint8_t                          cis_count; /*set by ull le service*/
    bt_ull_le_srv_phy_t              phy;
    bt_ull_le_codec_t                codec;
    bt_ull_le_hid_srv_app_scenario_t scenario;
} bt_ull_le_hid_conn_srv_cig_info_t;

typedef enum {
    BT_ULL_LE_HID_CONN_SRV_CMD_REMOVE_CIG,
    BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS,
    BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_CREATE_CIS,
    BT_ULL_LE_HID_CONN_SRV_CMD_SYNC_CIS,
    BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_SYNC_CIS,
    BT_ULL_LE_HID_CONN_SRV_CMD_DISCONNECT_CIS,
    BT_ULL_LE_HID_CONN_SRV_CMD_ACTIVE_STREAMING,
    BT_ULL_LE_HID_CONN_SRV_CMD_DEACTIVE_STREAMING,
    BT_ULL_LE_HID_CONN_SRV_CMD_UNMUTE_CIS,
    BT_ULL_LE_HID_CONN_SRV_CMD_MAX
} bt_ull_le_hid_conn_srv_cmd_enum;

typedef struct {
    bt_ull_le_hid_conn_srv_cmd_enum cmd;
    uint16_t                        data_len;
    void                            *data;
} bt_ull_le_hid_conn_srv_cmd_cache_item_t;

typedef struct bt_ull_le_hid_conn_srv_cache_cmd_node_t bt_ull_le_hid_conn_srv_cache_cmd_node_t;
struct bt_ull_le_hid_conn_srv_cache_cmd_node_t {
    bt_ull_le_hid_conn_srv_cache_cmd_node_t *next;
    bt_ull_le_hid_conn_srv_cmd_cache_item_t cache_item;
};

typedef struct {
    bt_ull_le_hid_conn_srv_cmd_enum cmd;
    bt_ull_le_hid_conn_srv_cmd_lock lock;
} bt_ull_le_hid_conn_srv_cmd_lock_t;

typedef struct {
    bt_ull_le_hid_srv_device_t device_type;
} BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_CREATE_CIS_T, BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_SYNC_CIS_T;

typedef struct {
    bt_handle_t                acl_handle;
    uint8_t                    reason;
} BT_ULL_LE_HID_CONN_SRV_CMD_DISCONNECT_CIS_T;

typedef struct {
    bt_handle_t                acl_handle;
} BT_ULL_LE_HID_CONN_SRV_CMD_ACTIVE_STREAMING_T,
BT_ULL_LE_HID_CONN_SRV_CMD_DEACTIVE_STREAMING_T, BT_ULL_LE_HID_CONN_SRV_CMD_UNMUTE_CIS_T;

typedef struct {
    bt_ull_le_hid_srv_device_t device_type;
    bt_addr_t                  peer_addr;
} BT_ULL_LE_HID_CONN_SRV_CMD_SYNC_CIS_T;

typedef struct {
    uint8_t data;
} BT_ULL_LE_HID_CONN_SRV_CMD_REMOVE_CIG_T;

typedef struct {
    bt_ull_le_hid_srv_conn_params_t *hs;
    bt_ull_le_hid_srv_conn_params_t *kb;
    bt_ull_le_hid_srv_conn_params_t *ms;
} BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS_T;

typedef struct {
    uint8_t                                  cig_id;
    uint8_t                                  cis_count; /*set by ull le service*/
    uint8_t                                  cig_state;
    struct {
        bool                                     is_switch_scenaro;
    } switch_scenario_state;
    bt_ull_le_srv_phy_t                      phy;
    bt_ull_le_codec_t                        codec;
    bt_ull_le_hid_srv_app_scenario_t         scenario;
    bt_ull_le_hid_srv_idle_time_t            idle_time;
    uint8_t                                  rr_level;
    bt_ull_role_t                            role;
    bt_ull_le_hid_conn_srv_callback_t            cb;
    bt_ull_le_hid_conn_srv_cache_cmd_node_t  *cmd_list_header;
    bt_ull_le_hid_conn_srv_cis_link_t        cis_info[BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM];
    BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS_T  cis_params_bk; //backup for change scenario when creating cis
} bt_ull_le_hid_conn_srv_contex_t;

typedef struct {
    bt_ull_le_hid_srv_device_t              dev_type;
    uint8_t                                 cis_id;
    uint8_t                                 data_path_id;
    uint32_t                                sdu_interval_m_to_s;
    uint32_t                                sdu_interval_s_to_m;
    uint8_t                                 share_num;
    uint8_t                                 uplink_num;
    uint32_t                                sdu_m_to_s;
    uint32_t                                sdu_s_to_m;
    bt_ull_le_srv_phy_t                     phy_m_to_s;
    bt_ull_le_srv_phy_t                     phy_s_to_m;
    uint8_t                                 ft_m_to_s;
    uint8_t                                 ft_s_to_m;
} bt_ull_le_hid_conn_srv_dev_params_t;

typedef struct {
    bt_ull_le_hid_srv_app_scenario_t        scenario;
    uint16_t                                slave_latency;
    uint32_t                                iso_interval;
    uint8_t                                 default_se;
    uint8_t                                 cis_count;
    bt_ull_le_hid_conn_srv_dev_params_t     tab[3];
} bt_ull_le_hid_conn_srv_cig_params_t;

static bt_ull_le_hid_conn_srv_contex_t g_ull_hid_srv_ctx;
static uint16_t g_ull_hid_conn_timeout = 500;
static uint16_t g_ull_hid_create_cis_timeout = 0;
static bt_ull_le_hid_conn_srv_cmd_lock_t g_bt_ull_hid_cmd_lock_table[BT_ULL_LE_HID_CONN_SRV_CMD_MAX + 1] = {
    {BT_ULL_LE_HID_CONN_SRV_CMD_REMOVE_CIG, 0},
    {BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS, 0},
    {BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_CREATE_CIS, 0},
    {BT_ULL_LE_HID_CONN_SRV_CMD_SYNC_CIS, 0},
    {BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_SYNC_CIS, 0},
    {BT_ULL_LE_HID_CONN_SRV_CMD_DISCONNECT_CIS, 0},
    {BT_ULL_LE_HID_CONN_SRV_CMD_ACTIVE_STREAMING, 0},
    {BT_ULL_LE_HID_CONN_SRV_CMD_DEACTIVE_STREAMING, 0},
    {BT_ULL_LE_HID_CONN_SRV_CMD_MAX, 0} /*max*/
};

bt_ull_le_hid_conn_srv_cig_params_t g_air_cig_params_tab[BT_ULL_LE_HID_CONN_SRV_AIR_PARAMS_MAX_NUM] = {
    {
        BT_ULL_LE_HID_SRV_APP_SCENARIO_1,
        0x0,
        5000,
        0x3,
        0x3,
        {
            {
                BT_ULL_LE_HID_SRV_DEVICE_HEADSET,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK1,
                BT_ULL_LE_SRV_DATA_PATH_ID_SPK_SPECIAL,
                5000,
                5000,
                0x02,
                0x01,
                BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_126,
                BT_ULL_LE_DEFAULT_UPLINK_SDU_SIZE_20,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {
                BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK2,
                BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE,
                5000,
                5000,
                0x02,
                0x02,
                0x7,
                0x7,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {
                BT_ULL_LE_HID_SRV_DEVICE_MOUSE,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK3,
                BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE,
                5000,
                5000,
                0x02,
                0x02,
                0x7,
                0x7,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            }
        }
    },
    {
        BT_ULL_LE_HID_SRV_APP_SCENARIO_2,
        0x0,
        5000,
        3,
        3,
        {
            {
                BT_ULL_LE_HID_SRV_DEVICE_HEADSET,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK1,
                BT_ULL_LE_SRV_DATA_PATH_ID_SPK_SPECIAL,
                5000,
                5000,
                0x02,
                0x01,
                0x35,
                14,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {
                BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK2,
                BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE,
                5000,
                5000,
                0x02,
                0x01,
                0x7,
                0x7,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {
                BT_ULL_LE_HID_SRV_DEVICE_MOUSE,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK3,
                BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE,
                5000,
                5000,
                0x02,
                0x01,
                0x7,
                0x7,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            }
        }

    },
    {
        BT_ULL_LE_HID_SRV_APP_SCENARIO_3,
        0x0,
        664,
        0x1,
        0x2,
        {
            {
                BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK1,
                BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE,
                664,
                664,
                0x01,
                0x01,
                0x7,
                0x7,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {
                BT_ULL_LE_HID_SRV_DEVICE_MOUSE,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK2,
                BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE,
                664,
                664,
                0x01,
                0x02,
                0x7,
                0x7,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {0}
        }
    },
    {
        BT_ULL_LE_HID_SRV_APP_SCENARIO_4,
        0x20,
        500,
        0x1,
        0x1,
        {
            {
                BT_ULL_LE_HID_SRV_DEVICE_MOUSE,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK1,
                BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE,
                500,
                500,
                0x01,
                0x02,
                0x7,
                0x7,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {0},
            {0}
        }
    },
    {
        BT_ULL_LE_HID_SRV_APP_SCENARIO_5,
        0x10,
        1000,
        0x1,
        0x2,
        {
            {
                BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK1,
                BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE,
                1000,
                1000,
                0x01,
                0x02,
                0x2F,
                0x7,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {
                BT_ULL_LE_HID_SRV_DEVICE_MOUSE,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK2,
                BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE,
                1000,
                1000,
                0x01,
                0x02,
                0x2F,
                0x7,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {0}
        }
    },
    {
        BT_ULL_LE_HID_SRV_APP_SCENARIO_6,
        0x8,
        2000,
        0x1,
        0x2,
        {
            {
                BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK1,
                BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE,
                2000,
                2000,
                0x01,
                0x02,
                0x86,
                0x7,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {
                BT_ULL_LE_HID_SRV_DEVICE_MOUSE,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK2,
                BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE,
                2000,
                2000,
                0x01,
                0x02,
                0x86,
                0x7,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {0}
        }
    },
    {
        BT_ULL_LE_HID_SRV_APP_SCENARIO_7,
        0x8,
        2000,
        0x1,
        0x2,
        {
            {
                BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK1,
                BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE,
                2000,
                2000,
                0x01,
                0x01,
                0x86,
                0x7,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {
                BT_ULL_LE_HID_SRV_DEVICE_MOUSE,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK2,
                BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE,
                2000,
                2000,
                0x01,
                0x01,
                0x86,
                0x7,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {0}
        }
    },
    {
        BT_ULL_LE_HID_SRV_APP_SCENARIO_8,
        0x4,
        2000,
        0x1,
        0x2,
        {
            {
                BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK1,
                BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE,
                2000,
                2000,
                0x01,
                0x01,
                0x86,
                0x7,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {
                BT_ULL_LE_HID_SRV_DEVICE_MOUSE,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK2,
                BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE,
                2000,
                2000,
                0x01,
                0x01,
                0x86,
                0x7,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {0}
        }
    },
    {
        BT_ULL_LE_HID_SRV_APP_SCENARIO_9,
        0x2,
        2000,
        0x1,
        0x2,
        {
            {
                BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK1,
                BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE,
                2000,
                2000,
                0x01,
                0x01,
                0x86,
                0x7,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {
                BT_ULL_LE_HID_SRV_DEVICE_MOUSE,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK2,
                BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE,
                2000,
                2000,
                0x01,
                0x01,
                0x86,
                0x7,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {0}
        }
    },
    {
        BT_ULL_LE_HID_SRV_APP_SCENARIO_10,
        0x0,
        5000,
        0x1,
        0x3,
        {
            {
                BT_ULL_LE_HID_SRV_DEVICE_HEADSET,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK1,
                BT_ULL_LE_SRV_DATA_PATH_ID_SPK_SPECIAL,
                5000,
                5000,
                0x01,
                0x01,
                BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_100,
                BT_ULL_LE_DEFAULT_UPLINK_SDU_SIZE_20,
                BT_ULL_LE_SRV_PHY_EDR_LE_4M,
                BT_ULL_LE_SRV_PHY_EDR_LE_4M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {
                BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK2,
                BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE,
                1000,
                1000,
                0x01,
                0x01,
                0x7,
                0x7,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {
                BT_ULL_LE_HID_SRV_DEVICE_MOUSE,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK3,
                BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE,
                1000,
                1000,
                0x01,
                0x01,
                0x7,
                0x7,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            }
        }
    },
    {
        BT_ULL_LE_HID_SRV_APP_SCENARIO_11,
        0x0,
        5000,
        0x1,
        0x3,
        {
            {
                BT_ULL_LE_HID_SRV_DEVICE_HEADSET,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK1,
                BT_ULL_LE_SRV_DATA_PATH_ID_SPK_SPECIAL,
                5000,
                5000,
                0x01,
                0x01,
                BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_190,
                BT_ULL_LE_DEFAULT_UPLINK_SDU_SIZE_40,
                BT_ULL_LE_SRV_PHY_EDR_LE_8M,
                BT_ULL_LE_SRV_PHY_EDR_LE_8M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {
                BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK2,
                BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE,
                1000,
                1000,
                0x01,
                0x01,
                0x1E,
                0x7,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {
                BT_ULL_LE_HID_SRV_DEVICE_MOUSE,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK3,
                BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE,
                1000,
                1000,
                0x01,
                0x01,
                0x1E,
                0x7,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            }
        }
    },
    {
        BT_ULL_LE_HID_SRV_APP_SCENARIO_12,
        0x0,
        5000,
        0x1,
        0x2,
        {
            {
                BT_ULL_LE_HID_SRV_DEVICE_HEADSET,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK1,
                BT_ULL_LE_SRV_DATA_PATH_ID_SPK_SPECIAL,
                5000,
                5000,
                0x01,
                0x01,
                BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_100,
                BT_ULL_LE_DEFAULT_UPLINK_SDU_SIZE_20,
                BT_ULL_LE_SRV_PHY_EDR_LE_4M,
                BT_ULL_LE_SRV_PHY_EDR_LE_4M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {
                BT_ULL_LE_HID_SRV_DEVICE_MOUSE,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK2,
                BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE,
                1000,
                1000,
                0x01,
                0x01,
                0x18,
                0x14,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {0}
        }
    },
    {
        BT_ULL_LE_HID_SRV_APP_SCENARIO_13,
        0x0,
        5000,
        0x1,
        0x2,
        {
            {
                BT_ULL_LE_HID_SRV_DEVICE_HEADSET,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK1,
                BT_ULL_LE_SRV_DATA_PATH_ID_SPK_SPECIAL,
                5000,
                5000,
                0x01,
                0x01,
                BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_190,
                BT_ULL_LE_DEFAULT_UPLINK_SDU_SIZE_40,
                BT_ULL_LE_SRV_PHY_EDR_LE_8M,
                BT_ULL_LE_SRV_PHY_EDR_LE_8M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {
                BT_ULL_LE_HID_SRV_DEVICE_MOUSE,
                BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK2,
                BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE,
                1000,
                1000,
                0x01,
                0x01,
                0x28,
                0x14,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_SRV_PHY_LE_2M,
                BT_ULL_LE_HID_CONN_SRV_FT_1,
                BT_ULL_LE_HID_CONN_SRV_FT_1
            },
            {0}
        }
    }
};

static uint8_t bt_ull_le_hid_conn_srv_get_sub_event(uint8_t report_rate);
static void bt_ull_le_hid_conn_srv_notify_msg(bt_ull_le_hid_conn_srv_msg_t msg, void *data);
static void bt_ull_le_hid_conn_srv_create_cig_params_cnf_hdl(bt_status_t status, bt_ull_le_set_air_hid_cig_cnf_t *cnf);
static void bt_ull_le_hid_conn_srv_cis_established_hdl(bt_status_t status, bt_ull_le_air_hid_cis_established_ind_t *ind);
static void bt_ull_le_hid_conn_srv_cis_disconnect_complete_hdl(bt_status_t status, bt_ull_le_air_hid_cis_disconnect_complete_ind_t *ind);
static void bt_ull_le_hid_conn_srv_remove_cig_params_cnf_hdl(bt_status_t status, bt_ull_le_remove_air_hid_cig_cnf_t *cnf);
static void bt_ull_le_hid_conn_srv_create_cis_cnf_hdl(bt_status_t status, void *cnf);
static void bt_ull_le_hid_conn_srv_sync_cis_cnf_hdl(bt_status_t status, void *cnf);
static void bt_ull_le_hid_conn_srv_cancel_create_cnf_hdl(bt_status_t status, void *cnf);
static void bt_ull_le_hid_conn_srv_cancel_sync_cnf_hdl(bt_status_t status, void *cnf);
static void bt_ull_le_hid_conn_srv_active_streaming_cnf_hdl(bt_status_t status, bt_ull_le_setup_air_hid_iso_data_path_cnf_t *cnf);
static void bt_ull_le_hid_conn_srv_deactive_streaming_cnf_hdl(bt_status_t status, bt_ull_le_remove_air_hid_iso_data_path_cnf_t *cnf);
static void bt_ull_le_hid_conn_srv_disconnect_cis_cnf_hdl(bt_status_t status, void *cnf);
static bt_status_t bt_ull_le_hid_conn_srv_next_action_hdl(bt_ull_le_hid_conn_srv_cmd_enum next_act, void *data);
static bt_status_t bt_ull_le_hid_conn_srv_event_callback(bt_msg_type_t msg, bt_status_t ret, void *data);
static bt_ull_le_hid_conn_srv_cig_params_t * bt_ull_le_hid_conn_srv_get_cig_params(bt_ull_le_hid_srv_app_scenario_t scenario);

void bt_ull_le_hid_conn_srv_set_cis_connection_timeout(uint16_t conn_timeout)
{
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_set_cig_state, conn_timeout: %d", 1, conn_timeout);
    g_ull_hid_conn_timeout = conn_timeout;
}

void bt_ull_le_hid_conn_srv_set_create_cis_timeout(uint16_t create_cis_timeout)
{
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_set_cig_state, create_cis_timeout: %d", 1, create_cis_timeout);
    g_ull_hid_create_cis_timeout = create_cis_timeout;
}

static void bt_ull_le_hid_conn_srv_set_cig_state(uint8_t state)
{
     ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_set_cig_state, cig state:0x%x -> 0x%x", 2, g_ull_hid_srv_ctx.cig_state, state);
     g_ull_hid_srv_ctx.cig_state = state;
}

static uint8_t bt_ull_le_hid_conn_srv_get_cig_state(void)
{
    return g_ull_hid_srv_ctx.cig_state;
}

static void bt_ull_le_hid_conn_srv_set_scenario_change_state(bool is_switch)
{
    g_ull_hid_srv_ctx.switch_scenario_state.is_switch_scenaro = is_switch;
}

static bool bt_ull_le_hid_conn_srv_get_scenario_change_state(void)
{
    return g_ull_hid_srv_ctx.switch_scenario_state.is_switch_scenaro;
}

static uint8_t bt_ull_le_hid_conn_srv_get_scenario(void)
{
    return g_ull_hid_srv_ctx.scenario;
}

static bt_ull_le_hid_conn_srv_contex_t *bt_ull_le_hid_conn_srv_get_ctx(void)
{
    return &g_ull_hid_srv_ctx;
}

/*
static uint8_t bt_ull_le_hid_conn_srv_get_codec(void)
{
    return g_ull_hid_srv_ctx.codec;
}

static bt_ull_le_srv_phy_t bt_ull_le_hid_conn_srv_get_phy(void)
{
    return g_ull_hid_srv_ctx.phy;
}

static uint8_t bt_ull_le_hid_conn_srv_get_cis_count(void)
{
    return g_ull_hid_srv_ctx.cis_count;
}
*/
static void bt_ull_le_hid_conn_srv_set_phy(bt_ull_le_srv_phy_t phy)
{
    g_ull_hid_srv_ctx.phy = phy;
}

static bt_ull_role_t bt_ull_le_hid_conn_srv_get_role(void)
{
    return g_ull_hid_srv_ctx.role;
}

static void bt_ull_le_hid_conn_srv_set_role(bt_ull_role_t role)
{
    g_ull_hid_srv_ctx.role = role;
}

static uint8_t bt_ull_le_hid_conn_srv_get_rr_level(void)
{
    return g_ull_hid_srv_ctx.rr_level;
}

static void bt_ull_le_hid_conn_srv_set_rr_level(uint8_t rr_level)
{
    g_ull_hid_srv_ctx.rr_level = rr_level;
}

static bt_ull_role_t bt_ull_le_hid_conn_srv_get_cig_id(void)
{
    return g_ull_hid_srv_ctx.cig_id;
}

static void bt_ull_le_hid_conn_srv_set_cig_id(uint8_t cig_id)
{
    g_ull_hid_srv_ctx.cig_id = cig_id;
}

static void bt_ull_le_hid_conn_srv_set_peer_addr(uint8_t idx, bt_addr_t *addr)
{
    if (idx >= BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_set_peer_addr, invalid idx!", 0);
        return;
    }
    bt_ull_le_srv_memcpy(&g_ull_hid_srv_ctx.cis_info[idx].peer_addr, addr, sizeof(bt_addr_t));
}

/*
static bt_addr_t* bt_ull_le_hid_conn_srv_get_peer_addr(uint8_t idx)
{
    if (idx >= BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_get_peer_addr, invalid idx!", 0);
        return NULL;
    }
    return &g_ull_hid_srv_ctx.cis_info[idx].peer_addr;
}
*/
static void bt_ull_le_hid_conn_srv_set_cis_state(uint8_t idx, uint8_t state)
{
    if (idx >= BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_set_cis_state, invalid idx!", 0);
        return;
    }
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_set_cis_state, idx: 0x%x, state:0x%x -> 0x%x", 3, idx, g_ull_hid_srv_ctx.cis_info[idx].state, state);
    g_ull_hid_srv_ctx.cis_info[idx].state = state;

}

static uint8_t bt_ull_le_hid_conn_srv_get_cis_state(uint8_t idx)
{
    if (idx >= BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_get_cis_state, invalid idx!", 0);
        return BT_ULL_LE_HID_CONN_SRV_INVALID;
    }
    return g_ull_hid_srv_ctx.cis_info[idx].state;
}

static void bt_ull_le_hid_conn_srv_set_device_type(uint8_t idx, uint8_t device_type)
{
    if (idx >= BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_set_device_type, invalid idx!", 0);
        return;
    }
    g_ull_hid_srv_ctx.cis_info[idx].device_type = device_type;
}

static uint8_t bt_ull_le_hid_conn_srv_get_device_type(uint8_t idx)
{
    if (idx >= BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_get_device_type, invalid idx!", 0);
        return BT_ULL_LE_HID_CONN_SRV_INVALID;
    }
    return g_ull_hid_srv_ctx.cis_info[idx].device_type;
}

static void bt_ull_le_hid_conn_srv_set_cis_id(uint8_t idx, uint8_t cis_id)
{
    if (idx >= BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_set_cis_id, invalid idx!", 0);
        return;
    }
    g_ull_hid_srv_ctx.cis_info[idx].cis_id = cis_id;
}

static uint8_t bt_ull_le_hid_conn_srv_get_cis_id(uint8_t idx)
{
    if (idx >= BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_get_cis_id, invalid idx!", 0);
        return BT_ULL_LE_HID_CONN_SRV_INVALID;
    }
    return g_ull_hid_srv_ctx.cis_info[idx].cis_id;
}

/*
static uint8_t bt_ull_le_hid_conn_srv_get_timeout(uint8_t idx)
{
    if (idx >= BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_get_timeout, invalid idx!", 0);
        return BT_ULL_LE_HID_CONN_SRV_INVALID;
    }
    return g_ull_hid_srv_ctx.cis_info[idx].conn_to;
}
*/
static void bt_ull_le_hid_conn_srv_set_conn_timout(uint8_t idx, uint32_t timeout)
{
    if (idx >= BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_set_conn_timout, invalid idx!", 0);
        return;
    }
    g_ull_hid_srv_ctx.cis_info[idx].conn_to = timeout;
}

static void bt_ull_le_hid_conn_srv_set_cis_handle(uint8_t idx, bt_handle_t handle)
{
    if (idx >= BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_set_cis_handle, invalid idx!", 0);
        return;
    }
    g_ull_hid_srv_ctx.cis_info[idx].cis_handle = handle;
}
/*
static bt_handle_t bt_ull_le_hid_conn_srv_get_cis_handle(uint8_t idx)
{
    if (idx >= BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_get_cis_handle, invalid idx!", 0);
        return BT_HANDLE_INVALID;
    }
    return g_ull_hid_srv_ctx.cis_info[idx].cis_handle;
}
*/
static void bt_ull_le_hid_conn_srv_set_acl_handle(uint8_t idx, bt_handle_t handle)
{
    if (idx >= BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_set_acl_handle, invalid idx!", 0);
        return;
    }
    g_ull_hid_srv_ctx.cis_info[idx].acl_handle = handle;
}

static bt_handle_t bt_ull_le_hid_conn_srv_get_acl_handle(uint8_t idx)
{
    if (idx >= BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_get_acl_handle, invalid idx!", 0);
        return BT_HANDLE_INVALID;
    }
    return g_ull_hid_srv_ctx.cis_info[idx].acl_handle;
}

static bt_handle_t bt_ull_le_hid_conn_srv_get_cis_handle(uint8_t idx)
{
    if (idx >= BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_get_cis_handle, invalid idx!", 0);
        return BT_HANDLE_INVALID;
    }
    return g_ull_hid_srv_ctx.cis_info[idx].cis_handle;
}

static uint8_t bt_ull_le_hid_conn_srv_get_idx_by_cis_handle(bt_handle_t handle)
{
    uint8_t i = 0;
    for (i = 0; i < BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM; i ++) {
        if (g_ull_hid_srv_ctx.cis_info[i].cis_handle == handle) {
            return i;
        }
    }
    ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_get_idx_by_cis_handle, can not found link!", 0);
    return BT_ULL_LE_HID_CONN_SRV_INVALID;
}

static uint8_t bt_ull_le_hid_conn_srv_get_idx_by_acl_handle(bt_handle_t handle)
{
    uint8_t i = 0;
    for (i = 0; i < BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM; i ++) {
        if (g_ull_hid_srv_ctx.cis_info[i].acl_handle == handle) {
            return i;
        }
    }
    ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_get_idx_by_cis_handle, can not found link!", 0);
    return BT_ULL_LE_HID_CONN_SRV_INVALID;
}
static uint8_t bt_ull_le_hid_conn_srv_get_idx_by_cis_id(uint8_t cis_id)
{
    uint8_t i = 0;
    for (i = 0; i < BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM; i ++) {
        if (g_ull_hid_srv_ctx.cis_info[i].cis_id == cis_id && cis_id != BT_ULL_LE_CONN_SRV_AIR_CIS_ID_INVANLID) {
            return i;
        }
    }
    ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_get_idx_by_cis_id, can not found link!", 0);
    return BT_ULL_LE_HID_CONN_SRV_INVALID;
}


static uint8_t bt_ull_le_hid_conn_srv_get_idx_by_type(uint8_t device_type)
{
    uint8_t i = 0;
    for (i = 0; i < BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM; i ++) {
        if (g_ull_hid_srv_ctx.cis_info[i].device_type == device_type) {
            return i;
        }
    }
    ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_get_idx_by_type, invalid cis type!", 0);
    return BT_ULL_LE_HID_CONN_SRV_INVALID;
}

/*
static bt_ull_le_hid_conn_srv_cis_link_t * bt_ull_le_hid_conn_srv_get_link_info_by_type(uint8_t device_type)
{
    uint8_t i = 0;
    for (i = 0; i < BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM; i ++) {
        if (g_ull_hid_srv_ctx.cis_info[i].device_type == device_type) {
            return &g_ull_hid_srv_ctx.cis_info[i];
        }
    }
    ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_get_link_info_by_type, invalid cis type!", 0);
    return NULL;
}
*/
/*
static bt_ull_le_hid_conn_srv_cis_link_t * bt_ull_le_hid_conn_srv_get_link_info_by_dix(uint8_t idx)
{
    if (idx >= BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_get_link_info_by_dix, invalid idx!", 0);
        return NULL;
    }
    return &g_ull_hid_srv_ctx.cis_info[idx];
}
*/
static uint8_t bt_ull_le_hid_conn_srv_get_empty_link(void)
{
    uint8_t i = 0;
    for (i = 0; i < BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM; i ++) {
        if (g_ull_hid_srv_ctx.cis_info[i].state == BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTED) {
            return i;
        }
    }
    ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_get_empty_link, not resource!", 0);
    return BT_ULL_LE_HID_CONN_SRV_INVALID;
}

static uint8_t bt_ull_le_hid_conn_srv_get_connected_link(void)
{
    uint8_t i = 0;
    uint8_t count = 0;
    for (i = 0; i < BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM; i ++) {
        if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CONNECTED <= g_ull_hid_srv_ctx.cis_info[i].state \
            && BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTING >= g_ull_hid_srv_ctx.cis_info[i].state) {
            count ++;
        }
    }
    return count;
}

static bool bt_ull_le_hid_conn_srv_cis_link_is_connectded(uint8_t idx)
{
    if (idx >= BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_cis_link_is_connectded, invalid idx!", 0);
        return false;
    }

    if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CONNECTED <= g_ull_hid_srv_ctx.cis_info[idx].state \
        && BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTING >= g_ull_hid_srv_ctx.cis_info[idx].state) {
        return true;
    }
    return false;
}

static uint8_t bt_ull_le_hid_conn_srv_get_data_path_id(uint8_t cis_id, bt_ull_le_hid_srv_device_t device_type)
{
    uint8_t scenario = bt_ull_le_hid_conn_srv_get_scenario();
    bt_ull_le_hid_conn_srv_cig_params_t *cig_params = bt_ull_le_hid_conn_srv_get_cig_params(scenario);
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_get_data_path_id, dt: %d, sc: %d, cis_id: %d", 3, device_type, scenario, cis_id);
    if (!cig_params) {
        return BT_ULL_LE_HID_CONN_SRV_INVALID;
    }
    if (BT_ULL_ROLE_CLIENT == bt_ull_le_hid_conn_srv_get_role()) {
        return BT_ULL_LE_SRV_DATA_PATH_ID_SPK_SPECIAL;
    } else if (BT_ULL_ROLE_SERVER == bt_ull_le_hid_conn_srv_get_role()) {
        uint8_t i = 0;
        for (i = 0; i < cig_params->cis_count; i ++) {
            if (cig_params->tab[i].cis_id == cis_id && cig_params->tab[i].dev_type == device_type) {
                return cig_params->tab[i].data_path_id;
            }
        }
    } else {
        assert(0);
    }
    return BT_ULL_LE_HID_CONN_SRV_INVALID;
}

static void bt_ull_le_hid_conn_srv_unlock_cmd(bt_ull_le_hid_conn_srv_cmd_enum cmd)
{
    uint8_t i = 0;
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_unlock_cmd, type: 0x%x", 1, cmd);
    for (i = 0; i < BT_ULL_LE_HID_CONN_SRV_CMD_MAX ; i ++) {
        if (g_bt_ull_hid_cmd_lock_table[i].cmd == cmd) {
            g_bt_ull_hid_cmd_lock_table[i].lock = 0;
        }
    }
}

static void bt_ull_le_hid_conn_srv_lock_cmd(bt_ull_le_hid_conn_srv_cmd_enum cmd, uint16_t lock)
{
    uint8_t i = 0;
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_lock_cmd, type: 0x%x", 1, cmd);
    for (i = 0; i < BT_ULL_LE_HID_CONN_SRV_CMD_MAX ; i ++) {
        if (g_bt_ull_hid_cmd_lock_table[i].cmd == cmd) {
            g_bt_ull_hid_cmd_lock_table[i].lock = lock;
        }
    }
}

static bt_ull_le_hid_conn_srv_cmd_cache_item_t* bt_ull_le_hid_conn_srv_create_cmd_node(bt_ull_le_hid_conn_srv_cmd_enum cmd)
{
    bt_ull_le_hid_conn_srv_cache_cmd_node_t *node;
    bt_ull_le_hid_conn_srv_cache_cmd_node_t *new_node;
    bt_ull_le_hid_conn_srv_cache_cmd_node_t *last_node;
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_create_cache_cmd_node, type: 0x%x", 1, cmd);
    new_node = (bt_ull_le_hid_conn_srv_cache_cmd_node_t *)bt_ull_le_srv_memory_alloc(sizeof(bt_ull_le_hid_conn_srv_cache_cmd_node_t));
    if (NULL == new_node) {
        return NULL;
    }
    bt_ull_le_srv_memset(new_node, 0, sizeof(bt_ull_le_hid_conn_srv_cache_cmd_node_t));
    new_node->next = NULL;
    new_node->cache_item.cmd = cmd;

    node = g_ull_hid_srv_ctx.cmd_list_header;
    last_node = node;
    if (node == NULL) {
        g_ull_hid_srv_ctx.cmd_list_header = new_node;
    } else {
        while (node) {
            last_node = node;
            node = (bt_ull_le_hid_conn_srv_cache_cmd_node_t *)node->next;
        }
        last_node->next = new_node;
    }
    return &new_node->cache_item;
}

static bt_ull_le_hid_conn_srv_cache_cmd_node_t* bt_ull_le_hid_conn_srv_create_cmd_node_ex(bt_ull_le_hid_conn_srv_cmd_enum cmd)
{
    bt_ull_le_hid_conn_srv_cache_cmd_node_t *node;
    bt_ull_le_hid_conn_srv_cache_cmd_node_t *new_node;
    bt_ull_le_hid_conn_srv_cache_cmd_node_t *last_node;
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_create_cmd_node_ex, type: 0x%x", 1, cmd);
    new_node = (bt_ull_le_hid_conn_srv_cache_cmd_node_t *)bt_ull_le_srv_memory_alloc(sizeof(bt_ull_le_hid_conn_srv_cache_cmd_node_t));
    if (NULL == new_node) {
        return NULL;
    }
    bt_ull_le_srv_memset(new_node, 0, sizeof(bt_ull_le_hid_conn_srv_cache_cmd_node_t));
    new_node->next = NULL;
    new_node->cache_item.cmd = cmd;

    node = g_ull_hid_srv_ctx.cmd_list_header;
    last_node = node;
    if (node == NULL) {
        g_ull_hid_srv_ctx.cmd_list_header = new_node;
    } else {
        while (node) {
            last_node = node;
            node = (bt_ull_le_hid_conn_srv_cache_cmd_node_t *)node->next;
        }
        last_node->next = new_node;
    }
    return new_node;
}

static uint8_t bt_ull_le_hid_conn_srv_delete_cmd_node(bt_ull_le_hid_conn_srv_cache_cmd_node_t *delete_node)
{
    if (delete_node == NULL) {
        return true;
    }
#ifdef BT_ULL_LE_CONN_SRV_DEBUG
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_conn_srv_delete_cache_cmd_node, node addr: 0x%x", 1, delete_node);
#endif
    bt_ull_le_hid_conn_srv_cache_cmd_node_t *header = g_ull_hid_srv_ctx.cmd_list_header;
    bt_ull_le_hid_conn_srv_cache_cmd_node_t *node = header;
    bt_ull_le_hid_conn_srv_cache_cmd_node_t *prv_node = header;
    while (node) {
        if (node == delete_node) {
            if (node == header) {
                g_ull_hid_srv_ctx.cmd_list_header = (bt_ull_le_hid_conn_srv_cache_cmd_node_t *)node->next;
            } else {
                prv_node->next = node->next;
            }
            bt_ull_le_srv_memory_free(node);
            return true;
        }
        prv_node = node;
        node = (bt_ull_le_hid_conn_srv_cache_cmd_node_t *)node->next;
    }
    return false;

}

static void bt_ull_le_hid_conn_srv_delete_all_cmd_node(void)
{
    bt_ull_le_hid_conn_srv_cache_cmd_node_t *header = g_ull_hid_srv_ctx.cmd_list_header;
    bt_ull_le_hid_conn_srv_cache_cmd_node_t *node = header;
    bt_ull_le_hid_conn_srv_cache_cmd_node_t *next_node = node;
    if (NULL == header) {
        return;
    }
    while (node) {
        next_node = node->next;
        bt_ull_le_srv_memory_free(node);
        node = next_node;
    }
    g_ull_hid_srv_ctx.cmd_list_header = NULL;
}

static uint16_t bt_ull_le_hid_conn_srv_cmd_is_lock(bt_ull_le_hid_conn_srv_cmd_enum cmd)
{
    uint8_t i = 0;
    for (i = 0; i < BT_ULL_LE_HID_CONN_SRV_CMD_MAX ; i ++) {
        if (g_bt_ull_hid_cmd_lock_table[i].cmd == cmd) {
            return g_bt_ull_hid_cmd_lock_table[i].lock;
        }
    }
    return 0;
}

static bt_status_t bt_ull_le_hid_conn_srv_register_callback(bt_ull_le_hid_conn_srv_callback_t cb)
{
    if (!cb) {
        return BT_STATUS_FAIL;
    }
    g_ull_hid_srv_ctx.cb = cb;
    return BT_STATUS_SUCCESS;
}

static void bt_ull_le_hid_conn_srv_clear_cis_info(uint8_t idx)
{
    if (idx >= BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_reset_cis_info, invalid idx!", 0);
        return;
    }
    g_ull_hid_srv_ctx.cis_info[idx].device_type = BT_ULL_LE_HID_SRV_DEVICE_NONE;
    g_ull_hid_srv_ctx.cis_info[idx].cis_id = 0x0;
    g_ull_hid_srv_ctx.cis_info[idx].acl_handle = BT_HANDLE_INVALID;
    g_ull_hid_srv_ctx.cis_info[idx].cis_handle = BT_HANDLE_INVALID;
    g_ull_hid_srv_ctx.cis_info[idx].state = BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTED;
    g_ull_hid_srv_ctx.cis_info[idx].conn_to = 0x00;
    bt_ull_le_srv_memset(&g_ull_hid_srv_ctx.cis_info[idx].peer_addr, 0, sizeof(bt_addr_t));
}

static void bt_ull_le_hid_conn_srv_notify_msg(bt_ull_le_hid_conn_srv_msg_t msg, void *data)
{
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_notify_msg, event: 0x%x", 1, msg);
    if (g_ull_hid_srv_ctx.cb) {
        g_ull_hid_srv_ctx.cb(msg, data);
    } else {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_notify_msg error, event: 0x%x", 1, msg);
    }
}

static bt_ull_le_hid_conn_srv_cache_cmd_node_t* bt_ull_le_hid_conn_srv_search_cmd_node_by_type(bt_ull_le_hid_conn_srv_cmd_enum cmd)
{
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_conn_srv_search_cache_cmd_node_by_type, type: 0x%x", 1, cmd);
    bt_ull_le_hid_conn_srv_cache_cmd_node_t *node;
    node = g_ull_hid_srv_ctx.cmd_list_header;
    while (node) {
        if (node->cache_item.cmd == cmd) {
            return node;
        }
        node = (bt_ull_le_hid_conn_srv_cache_cmd_node_t *)node->next;
    }
    return NULL;
}

/*
bt_ull_le_hid_conn_srv_cache_cmd_node_t* bt_ull_le_hid_conn_srv_search_cache_cmd_by_handle(bt_ull_le_conn_srv_cmd_enum cmd, bt_handle_t handle)
{
    bt_ull_le_hid_conn_srv_cache_cmd_node_t *node;
    node = g_ull_hid_srv_ctx.cmd_list_header;
    while (node) {
        if (node->cache_item.handle == handle && node->cache_item.cmd == cmd) {
            return node;
        }
        node = (bt_ull_le_hid_conn_srv_cache_cmd_node_t *)node->next;
    }
    return NULL;
}

void bt_ull_le_hid_conn_srv_delete_all_cache_cmd_by_handle(bt_handle_t acl_handle)
{
    if (BT_ULL_HEADSET_CLIENT == bt_ull_le_conn_srv_get_client_type()) {
        bt_ull_le_conn_srv_delete_all_cmd_node();
    } else {
        uint8_t idx = bt_ull_le_conn_srv_get_idx_by_acl_handle(acl_handle);
        if (BT_ULL_LE_HID_CONN_SRV_INVALID == idx) {
            return;
        }
        bt_handle_t cis_handle = bt_ull_le_conn_srv_get_cis_handle(idx);

        bt_ull_le_hid_conn_srv_cache_cmd_node_t *header = g_ull_hid_srv_ctx.cmd_list_header;
        bt_ull_le_hid_conn_srv_cache_cmd_node_t *node = header;
        bt_ull_le_hid_conn_srv_cache_cmd_node_t *prv_node = header;
        while (node) {
            if (node->cache_item.handle == cis_handle) {
                ull_report(BT_ULL_LE_HID_CONN_LOG" Delete cache CMD: 0x%x", 1, node->cache_item.cmd);
                if (node == header) {
                    g_ull_hid_srv_ctx.cmd_list_header = (bt_ull_le_hid_conn_srv_cache_cmd_node_t *)node->next;
                    node = g_ull_hid_srv_ctx.cmd_list_header;
                    prv_node = node;
                } else {
                    prv_node->next = node->next;
                    bt_ull_le_srv_memory_free(node);
                    node = prv_node->next;
                }
            } else {
                prv_node = node;
                node = (bt_ull_le_hid_conn_srv_cache_cmd_node_t *)node->next;
            }
        }
    }
    return;
}
*/

bt_status_t bt_ull_le_hid_conn_srv_set_idle_time(bt_ull_le_hid_srv_idle_time_t idle_time)
{
    bt_ull_le_hid_conn_srv_contex_t *ctx = bt_ull_le_hid_conn_srv_get_ctx();
    if (idle_time > 200) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" error idle_time!", 0);
        ctx->idle_time = BT_ULL_LE_HID_CONN_SRV_INVALID;
        return BT_STATUS_FAIL;
    }

    ctx->idle_time = idle_time;
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_ull_le_hid_conn_srv_change_scenario(bt_ull_le_hid_srv_app_scenario_t scenario)
{
    uint8_t cig_state = bt_ull_le_hid_conn_srv_get_cig_state();
    bt_ull_le_hid_conn_srv_contex_t *ctx = bt_ull_le_hid_conn_srv_get_ctx();
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_change_scenario, switch_sc: %d, curr_sc: %d, cig_state: %d", 3,\
        scenario, ctx->scenario, cig_state);
    bt_status_t status = BT_STATUS_SUCCESS;
    if (BT_ULL_LE_HID_SRV_APP_SCENARIO_NONE == scenario) {
        return BT_STATUS_FAIL;
    }
    if (scenario == ctx->scenario) {
        return BT_STATUS_SUCCESS;
    }
    ctx->scenario = scenario;
    if (BT_ULL_LE_HID_CONN_SRV_CIG_STATE_NONE == cig_state || BT_ULL_LE_HID_CONN_SRV_CIG_STATE_REMOVING == cig_state) {
        return BT_STATUS_SUCCESS;
    } else if (BT_ULL_LE_HID_CONN_SRV_CIG_STATE_CREATED == cig_state) {
        bt_ull_le_hid_conn_srv_set_scenario_change_state(true);
        status = bt_ull_le_hid_conn_srv_remove_air_cig();
    } else if (BT_ULL_LE_HID_CONN_SRV_CIG_STATE_CREATING == cig_state) {
        bt_ull_le_hid_conn_srv_set_scenario_change_state(true);
        status = BT_STATUS_SUCCESS;
    }
    return status;
}

bt_status_t bt_ull_le_hid_conn_srv_create_air_cig(bt_ull_le_hid_srv_app_scenario_t scenario)
{
    uint8_t i = 0;
    bt_status_t status = BT_STATUS_FAIL;
    uint8_t cis_state = bt_ull_le_hid_conn_srv_get_cig_state();
    bt_ull_role_t role = bt_ull_le_hid_conn_srv_get_role();
    if (BT_ULL_LE_HID_CONN_SRV_CIG_STATE_CREATED == cis_state) {
        return BT_STATUS_SUCCESS;
    } else if (BT_ULL_LE_HID_CONN_SRV_CIG_STATE_CREATING == cis_state) {
        return BT_STATUS_FAIL;
    }
    bt_ull_le_hid_conn_srv_cig_params_t *cig_params = bt_ull_le_hid_conn_srv_get_cig_params(scenario);
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_create_air_cig, sc: %d", 1, scenario);
    if (!cig_params) {
        return BT_STATUS_FAIL;
    }

    bt_ull_le_set_air_hid_cig_t hid_cig;
    bt_ull_le_air_hid_cis_params_t *cis_list = bt_ull_le_srv_memory_alloc(sizeof(bt_ull_le_air_hid_cis_params_t) * cig_params->cis_count);
    if (BT_ULL_ROLE_SERVER != bt_ull_le_hid_conn_srv_get_role() || !cig_params->cis_count) {
        ull_assert(0 && BT_ULL_LE_HID_CONN_LOG" invalid role or cis count!");
        return BT_STATUS_FAIL;
    }
/*
    uint8_t codec = bt_ull_le_hid_conn_srv_get_codec();
    bt_ull_le_codec_param_t *spk = bt_ull_le_srv_get_codec_param(BT_ULL_ROLE_SERVER, BT_ULL_GAMING_TRANSMITTER);
    bt_ull_le_codec_param_t *mic = bt_ull_le_srv_get_codec_param(BT_ULL_ROLE_SERVER, BT_ULL_MIC_TRANSMITTER);
    if (!spk || !mic) {
        ull_report(BT_ULL_LE_HID_CONN_LOG" invalid auido params!", 0);
        return BT_STATUS_FAIL;
    }
*/
    if (!cis_list) {
        ull_assert(0 && BT_ULL_LE_HID_CONN_LOG" out of memory!");
        return BT_STATUS_OUT_OF_MEMORY;
    } else {
        bt_ull_le_srv_memset(cis_list, 0, sizeof(bt_ull_le_air_hid_cis_params_t) * cig_params->cis_count);
    }
    for (i = 0; i < cig_params->cis_count; i++) {
        cis_list[i].cis_id = cig_params->tab[i].cis_id;
        cis_list[i].slave_type = cig_params->tab[i].dev_type;
        bt_ull_le_hid_conn_srv_set_cis_id(i, cis_list[i].cis_id );
        bt_ull_le_hid_conn_srv_set_device_type(i, cis_list[i].slave_type);
        cis_list[i].sdu_interval_m_to_s = cig_params->tab[i].sdu_interval_m_to_s;
        cis_list[i].sdu_interval_s_to_m = cig_params->tab[i].sdu_interval_s_to_m;
        cis_list[i].share_num = cig_params->tab[i].share_num;
        cis_list[i].max_uplink_num  = cig_params->tab[i].uplink_num;
        cis_list[i].phy_m_to_s = cig_params->tab[i].phy_m_to_s;
        cis_list[i].phy_s_to_m = cig_params->tab[i].phy_s_to_m;
        cis_list[i].ft_m_to_s = cig_params->tab[i].ft_m_to_s;
        cis_list[i].ft_s_to_m = cig_params->tab[i].ft_s_to_m;

        if (BT_ULL_LE_HID_SRV_DEVICE_HEADSET == cig_params->tab[i].dev_type \
            || BT_ULL_LE_HID_SRV_DEVICE_EARBUDS == cig_params->tab[i].dev_type) {
            cis_list[i].max_sdu_m_to_s = (bt_ull_le_srv_get_sdu_size(false, role));
            cis_list[i].max_sdu_s_to_m = bt_ull_le_srv_get_sdu_size(true, role);
        } else {
            cis_list[i].max_sdu_m_to_s = cig_params->tab[i].sdu_m_to_s;
            cis_list[i].max_sdu_s_to_m = cig_params->tab[i].sdu_s_to_m;

        }
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
        ull_report(BT_ULL_LE_HID_CONN_LOG"[CMD] bt_ull_le_hid_conn_srv_create_air_cig, param(%d): %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d", 13, \
            cis_list[i].slave_type,
            cis_list[i].cis_id,
            cis_list[i].slave_type,
            cis_list[i].sdu_interval_m_to_s,
            cis_list[i].sdu_interval_s_to_m,
            cis_list[i].share_num,
            cis_list[i].max_uplink_num,
            cis_list[i].max_sdu_m_to_s,
            cis_list[i].max_sdu_s_to_m,
            cis_list[i].phy_m_to_s,
            cis_list[i].phy_s_to_m,
            cis_list[i].ft_m_to_s,
            cis_list[i].ft_s_to_m);
#endif

    }
    uint8_t sub_event = bt_ull_le_hid_conn_srv_get_sub_event(bt_ull_le_hid_conn_srv_get_rr_level());
    bt_ull_le_hid_conn_srv_contex_t *ctx = bt_ull_le_hid_conn_srv_get_ctx();
    hid_cig.cig_id = BT_ULL_LE_HID_CONN_SRV_CIG_ID_1;
    bt_ull_le_hid_conn_srv_set_cig_id(BT_ULL_LE_HID_CONN_SRV_CIG_ID_1);
    hid_cig.scnario = scenario;
    hid_cig.slave_latency = ((ctx->scenario < BT_ULL_LE_HID_SRV_APP_SCENARIO_4) || (ctx->idle_time == BT_ULL_LE_HID_CONN_SRV_INVALID)) ? cig_params->slave_latency : ((ctx->idle_time * 1000)/cig_params->iso_interval); //iso_interval unit is us.

    if (BT_ULL_LE_HID_SRV_APP_SCENARIO_1 == scenario || BT_ULL_LE_HID_SRV_APP_SCENARIO_2 == scenario) {
        hid_cig.d4_sub_event = sub_event;
    } else {
        hid_cig.d4_sub_event = cig_params->default_se;
    }
    hid_cig.iso_interval = cig_params->iso_interval;
    hid_cig.cis_count = cig_params->cis_count;
    hid_cig.cis_list = cis_list;
    status = bt_ull_le_set_air_hid_cig_parameters(&hid_cig);
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
    ull_report(BT_ULL_LE_HID_CONN_LOG"[CMD] bt_ull_le_hid_conn_srv_create_air_cig, status: %x, sc: %d, sl: %d, d4_sub_event: %d, iso_interval: %d, cis_num: %d", 6, \
        status, hid_cig.scnario, hid_cig.slave_latency, hid_cig.d4_sub_event, hid_cig.iso_interval, hid_cig.cis_count);
#endif
    if (BT_STATUS_SUCCESS == status) {
        bt_ull_le_hid_conn_srv_set_cig_state(BT_ULL_LE_HID_CONN_SRV_CIG_STATE_CREATING);
    }
    //ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_create_air_cig-1, status: %x", 1, status);
    bt_ull_le_srv_memory_free(cis_list);
    //ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_create_air_cig-2, status: %x", 1, status);
    return status;
}

bt_status_t bt_ull_le_hid_conn_srv_remove_air_cig(void)
{
    uint8_t i = 0;
    bool rm_now = true;
    bt_status_t status = BT_STATUS_SUCCESS;
    uint8_t cig_id = bt_ull_le_hid_conn_srv_get_cig_id();
    uint8_t cig_state = bt_ull_le_hid_conn_srv_get_cig_state();
    bool is_siwtch_sc = bt_ull_le_hid_conn_srv_get_scenario_change_state();
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_remove_air_cig, cig id: %d, cig_state: %d, is_switch_sc: %d", 3, \
        cig_id, cig_state, is_siwtch_sc);
    if (BT_ULL_LE_HID_CONN_SRV_CIG_ID_1 != cig_id || BT_ULL_LE_HID_CONN_SRV_CIG_STATE_CREATED != cig_state) {
        return BT_STATUS_FAIL;
    }
    if (bt_ull_le_hid_conn_srv_get_role() != BT_ULL_ROLE_SERVER) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" error role!", 0);
        return BT_STATUS_FAIL;
    }
    bt_ull_le_hid_conn_srv_contex_t *ctx = bt_ull_le_hid_conn_srv_get_ctx();
    uint16_t q_total_len = 0;
    uint16_t q_comm_len = sizeof(bt_ull_le_hid_srv_conn_params_t) - 1;
    bt_ull_le_hid_conn_srv_cache_cmd_node_t* create_node = bt_ull_le_hid_conn_srv_create_cmd_node_ex(BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS);
    BT_ULL_LE_HID_MAKE_CMD_PARAM(create, BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS);

    if (is_siwtch_sc) {
        if (!create_node || !create) {
            ull_report_error(BT_ULL_LE_HID_CONN_LOG" oom!", 0);
            return BT_STATUS_FAIL;
        }
        create_node->cache_item.cmd = BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS;
        create->hs = NULL;
        create->kb = NULL;
        create->ms = NULL;
    }
    for (i = 0; i < BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM; i ++) {
        if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTED != bt_ull_le_hid_conn_srv_get_cis_state(i)) {
            rm_now = false;
            uint8_t device_type = bt_ull_le_hid_conn_srv_get_device_type(i);
            if (bt_ull_le_hid_conn_srv_cis_link_is_connectded(i)) {
                if (is_siwtch_sc) {
                    bt_ull_le_hid_srv_conn_params_t *conn_params = bt_ull_le_srv_memory_alloc(q_comm_len + sizeof(bt_addr_t));
                    if (conn_params) {
                        conn_params->device_type = device_type;
                        conn_params->list_num = 0x1;
                        bt_ull_le_srv_memcpy(&conn_params->peer_addr_list, &ctx->cis_info[i].peer_addr, sizeof(bt_addr_t));
                        uint16_t len = q_comm_len + sizeof(bt_addr_t);
                        switch (conn_params->device_type) {
                             case BT_ULL_LE_HID_SRV_DEVICE_HEADSET: {
                                 create->hs = conn_params;
                                 break;
                             }
                             case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
                                 create->kb = conn_params;
                                 break;
                             }
                             case BT_ULL_LE_HID_SRV_DEVICE_MOUSE: {
                                 create->ms = conn_params;
                                 break;
                             }
                             default:
                                break;

                        }
                        q_total_len += len;
                    }
                }
                bt_ull_le_hid_conn_srv_disconnect_air_cis(bt_ull_le_hid_conn_srv_get_acl_handle(i), BT_HCI_STATUS_REMOTE_USER_TERMINATED_CONNECTION);
            }
            if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CONNECTING == bt_ull_le_hid_conn_srv_get_cis_state(i)) {
                if (is_siwtch_sc) {
                    bt_ull_le_hid_srv_conn_params_t *conn_params;                   
                    uint16_t len = 0;
                    switch (device_type) {
                         case BT_ULL_LE_HID_SRV_DEVICE_HEADSET: {
                             conn_params = bt_ull_le_srv_memory_alloc(q_comm_len + sizeof(bt_addr_t) * ctx->cis_params_bk.hs->list_num);
                             if (conn_params) {
                                 conn_params->list_num = ctx->cis_params_bk.hs->list_num;
                                 conn_params->device_type = device_type;                             
                                 len = q_comm_len + sizeof(bt_addr_t) * conn_params->list_num;
                                 bt_ull_le_srv_memcpy(&conn_params->peer_addr_list, &(ctx->cis_params_bk.hs->peer_addr_list), sizeof(bt_addr_t) * conn_params->list_num);
                                 create->hs = conn_params;
                             }
                             break;
                         }
                         case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD: {
                             conn_params = bt_ull_le_srv_memory_alloc(q_comm_len + sizeof(bt_addr_t) * ctx->cis_params_bk.kb->list_num);
                             if (conn_params) {
                                 conn_params->list_num = ctx->cis_params_bk.kb->list_num;
                                 conn_params->device_type = device_type;
                                 len = q_comm_len + sizeof(bt_addr_t) * conn_params->list_num;
                                 bt_ull_le_srv_memcpy(&conn_params->peer_addr_list, &(ctx->cis_params_bk.kb->peer_addr_list), sizeof(bt_addr_t) * conn_params->list_num);
                                 create->kb = conn_params;
                             }
                             break;
                         }
                         case BT_ULL_LE_HID_SRV_DEVICE_MOUSE: {
                             conn_params = bt_ull_le_srv_memory_alloc(q_comm_len + sizeof(bt_addr_t) * ctx->cis_params_bk.ms->list_num);
                             if (conn_params) {
                                 conn_params->list_num = ctx->cis_params_bk.ms->list_num;
                                 conn_params->device_type = device_type;
                                 len = q_comm_len + sizeof(bt_addr_t) * conn_params->list_num;
                                 bt_ull_le_srv_memcpy(&conn_params->peer_addr_list, &(ctx->cis_params_bk.ms->peer_addr_list), sizeof(bt_addr_t) * conn_params->list_num);
                                 create->ms = conn_params;
                             }
                             break;
                         }
                         default:
                            break;
                    }
                    q_total_len += len;
                }
                bt_ull_le_hid_conn_srv_cancel_create_air_cis(bt_ull_le_hid_conn_srv_get_device_type(i));
            }
        }
        create_node->cache_item.data_len = q_total_len;
        create_node->cache_item.data = create;
    }
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_remove_air_cig, hs 0x%x, kb 0x%x, ms 0x%x", 3, create->hs, create->kb, create->ms);
    if (!rm_now) {
        ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_remove_air_cig, need disconnect or cancel cis", 0);

        bt_ull_le_hid_conn_srv_cmd_cache_item_t* node = bt_ull_le_hid_conn_srv_create_cmd_node(BT_ULL_LE_HID_CONN_SRV_CMD_REMOVE_CIG);
        node->cmd = BT_ULL_LE_HID_CONN_SRV_CMD_REMOVE_CIG;
        node->data_len = 0x00;
        node->data = NULL;
        return BT_STATUS_PENDING;
    } else {
        bt_ull_le_srv_memory_free(create);
        bt_ull_le_hid_conn_srv_delete_cmd_node(create_node);
    }
    bt_ull_le_remove_air_hid_cig_t params;
    params.cig_id = cig_id;
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
    ull_report(BT_ULL_LE_HID_CONN_LOG"[CMD] bt_ull_le_hid_conn_srv_remove_air_cig, cig id: %x", 1, \
            params.cig_id);
#endif
    status = bt_ull_le_remove_air_hid_cig_parameters(&params);
    if (BT_STATUS_SUCCESS == status) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_remove_air_cig, hci status error: %x!", 1, status);
        bt_ull_le_hid_conn_srv_set_cig_state(BT_ULL_LE_HID_CONN_SRV_CIG_STATE_REMOVING);
    }


    return status;
}

static bt_ull_le_create_air_hid_cis_params_t *bt_ull_le_hid_conn_srv_create_conn_params(bt_ull_le_hid_srv_conn_params_t *param, uint8_t *ltk, uint8_t *skd, uint8_t *iv, uint32_t *len)
{
    uint8_t idx = bt_ull_le_hid_conn_srv_get_idx_by_type(param->device_type);
    if (BT_ULL_LE_HID_CONN_SRV_INVALID == idx) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" invalid link!", 0);
        return NULL;
    }
    bt_ull_le_create_air_hid_cis_params_t *conn = NULL;
    uint32_t comm_len = sizeof(bt_ull_le_create_air_hid_cis_params_t) - 1;
    uint8_t i = 0;
    bool set_key = false;
    uint32_t list_len = 0x0;
    if (bt_ull_le_hid_conn_srv_get_cis_state(idx) != BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTED) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_create_conn_params, (%d) is connecting!", 1, param->device_type);
    } else {
        list_len = comm_len + param->list_num * sizeof(bt_ull_le_air_hid_cis_dev_info_t);
        *len = list_len;
        conn = bt_ull_le_srv_memory_alloc(list_len);
        if (!conn) {
            ull_assert(0 && BT_ULL_LE_HID_CONN_LOG" headset OOM!");
        }
        conn->cis_connection_handle = bt_ull_le_hid_conn_srv_get_cis_handle(idx);
        conn->connection_timeout = g_ull_hid_conn_timeout;
        bt_ull_le_hid_conn_srv_set_conn_timout(idx, conn->connection_timeout);
        conn->dev_count = param->list_num;
        bt_ull_le_hid_dm_device_info_t *headset_info = NULL;
        bt_ull_le_air_hid_cis_dev_info_t *dev_list = (bt_ull_le_air_hid_cis_dev_info_t *)&conn->dev_list;
        for (i = 0; i < param->list_num; i ++) {
            headset_info = bt_ull_le_hid_dm_read_device_info(param->device_type, (bt_addr_t *)&param->peer_addr_list + i);
            if (headset_info == NULL) {
                ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_create_conn_params(%d), NO Boding!", 1, param->device_type);
                bt_ull_le_hid_srv_print_addr((bt_addr_t *)&param->peer_addr_list + i);
                continue;
            }
            if (!set_key) {
                set_key = true;
                bt_ull_le_srv_memcpy(ltk, &headset_info->ltk, BT_ULL_LE_HID_DM_LTK_LEN);
                bt_ull_le_srv_memcpy(skd, &headset_info->skd, BT_ULL_LE_HID_DM_SKD_LEN);
                bt_ull_le_srv_memcpy(iv, &headset_info->iv, BT_ULL_LE_HID_DM_IV_LEN);
            }
            bt_ull_le_srv_memcpy(&dev_list[i].uni_aa, &headset_info->uni_aa, BT_ULL_LE_HID_DM_UNI_AA_LEN);
            bt_ull_le_srv_memcpy(&dev_list[i].peer_addr, (bt_addr_t *)&param->peer_addr_list + i, sizeof(bt_addr_t));
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
            ull_report(BT_ULL_LE_HID_CONN_LOG"[CMD] create_air_cis(%d), uni_aa: %x-%x-%x-%x", 5, \
                param->device_type,
                dev_list[i].uni_aa[0],
                dev_list[i].uni_aa[1],
                dev_list[i].uni_aa[2],
                dev_list[i].uni_aa[3]
                );
            bt_ull_le_hid_srv_print_addr(&dev_list[i].peer_addr);
#endif
        }
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
ull_report(BT_ULL_LE_HID_CONN_LOG"[CMD] create_air_cis, param(%d): Cis_Handle: %x, Conn_TO:%d, Dev_Count: %d", 4, \
    param->device_type,
    conn->cis_connection_handle,
    conn->connection_timeout,
    conn->dev_count
    );
#endif
        bt_ull_le_hid_conn_srv_set_cis_state(idx, BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CONNECTING);
    }
    return conn;
}

static void bt_ull_le_hid_conn_srv_alloc_cis_params_bk(bt_ull_le_hid_srv_conn_params_t *hs, \
    bt_ull_le_hid_srv_conn_params_t *kb, bt_ull_le_hid_srv_conn_params_t *ms)
{ 
    bt_ull_le_hid_conn_srv_contex_t *ctx = bt_ull_le_hid_conn_srv_get_ctx();
    uint16_t q_comm_len = sizeof(bt_ull_le_hid_srv_conn_params_t) - 1;    
    uint16_t len = 0;
    if (hs) {
        len = q_comm_len + sizeof(bt_addr_t) * (hs->list_num);
        ctx->cis_params_bk.hs = bt_ull_le_srv_memory_alloc(len);
        if (ctx->cis_params_bk.hs) {
            memcpy(ctx->cis_params_bk.hs, hs, len);
        }
    }
    if (kb) {
        len = q_comm_len + sizeof(bt_addr_t) * (kb->list_num);
        ctx->cis_params_bk.kb = bt_ull_le_srv_memory_alloc(len);
        if (ctx->cis_params_bk.kb) {
            memcpy(ctx->cis_params_bk.kb, kb, len);
        }
    }
    if (ms) {
        len = q_comm_len + sizeof(bt_addr_t) * (ms->list_num);
        ctx->cis_params_bk.ms = bt_ull_le_srv_memory_alloc(len);
        if (ctx->cis_params_bk.ms) {
            memcpy(ctx->cis_params_bk.ms, ms, len);
        }
    }
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_alloc_cis_params_bk, hs_bk: 0x%x, kb_bk: 0x%x, ms_bk: 0x%x", 3, ctx->cis_params_bk.hs, ctx->cis_params_bk.kb, ctx->cis_params_bk.ms);

    return;
}

static void bt_ull_le_hid_conn_srv_free_cis_params_bk(bt_ull_le_hid_srv_device_t device_type)
{ 
    bt_ull_le_hid_conn_srv_contex_t *ctx = bt_ull_le_hid_conn_srv_get_ctx();
    void *p_mem = NULL;
    switch (device_type) {
        case BT_ULL_LE_HID_SRV_DEVICE_HEADSET:
            p_mem = ctx->cis_params_bk.hs;
            ctx->cis_params_bk.hs = NULL;
            break;
        case BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD:
            p_mem = ctx->cis_params_bk.kb;
            ctx->cis_params_bk.kb = NULL;
            break;
        case BT_ULL_LE_HID_SRV_DEVICE_MOUSE:
            p_mem = ctx->cis_params_bk.ms;
            ctx->cis_params_bk.ms = NULL;
            break;
        default:
            break;
    }
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_free_cis_params_bk, free 0x%x, device_type: %x", 2, p_mem, device_type);
    if (p_mem != NULL) {      
        bt_ull_le_srv_memory_free(p_mem);
    } else {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_free_cis_params_bk, already free!", 0);
    }

    return;
}

bt_status_t bt_ull_le_hid_conn_srv_create_air_cis(bt_ull_le_hid_srv_conn_params_t *hs, \
    bt_ull_le_hid_srv_conn_params_t *kb, bt_ull_le_hid_srv_conn_params_t *ms)
{
    bt_status_t status = BT_STATUS_FAIL;
    uint8_t cis_count = 0;
    uint8_t idx = BT_ULL_LE_HID_CONN_SRV_INVALID;
    uint32_t hs_list_len = 0x0, kb_list_len = 0x0, ms_list_len = 0x0;
    bt_ull_le_create_air_hid_cis_params_t *headset = NULL;
    bt_ull_le_create_air_hid_cis_params_t *keyboard = NULL;
    bt_ull_le_create_air_hid_cis_params_t *mouse = NULL;
    //uint32_t comm_len = sizeof(bt_ull_le_create_air_hid_cis_params_t) - 1;
    uint8_t     ltk[16] = {0};
    uint8_t     skd[16] = {0};
    uint8_t     iv[8] = {0};
    uint8_t cig_state = bt_ull_le_hid_conn_srv_get_cig_state();
    
    if (BT_ULL_LE_HID_CONN_SRV_CIG_STATE_CREATED != cig_state || bt_ull_le_hid_conn_srv_cmd_is_lock(BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS)) {
        bt_ull_le_hid_conn_srv_cmd_cache_item_t* node = bt_ull_le_hid_conn_srv_create_cmd_node(BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS);
        BT_ULL_LE_HID_MAKE_CMD_PARAM(create, BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS);
        node->cmd = BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS;
        uint16_t q_total_len = 0;
        uint16_t q_comm_len = sizeof(bt_ull_le_hid_srv_conn_params_t) - 1;
        if (hs) {
            uint16_t hs_len = q_comm_len + sizeof(bt_addr_t) * (hs->list_num);
            bt_ull_le_hid_srv_conn_params_t *hs_q = bt_ull_le_srv_memory_alloc(hs_len);
            if (!hs_q) {
                ull_assert(0 && BT_ULL_LE_HID_CONN_LOG" headset OOM!");
            }
            bt_ull_le_srv_memcpy(hs_q, hs, hs_len);
            q_total_len += hs_len;
            create->hs = hs_q;
        } else {
            create->hs = NULL;
        }

        if (ms) {
            uint16_t ms_len = q_comm_len + sizeof(bt_addr_t) * (ms->list_num);
            bt_ull_le_hid_srv_conn_params_t *ms_q = bt_ull_le_srv_memory_alloc(ms_len);
            if (!ms_q) {
                ull_assert(0 && BT_ULL_LE_HID_CONN_LOG" mouse OOM!");
            }
            bt_ull_le_srv_memcpy(ms_q, ms, ms_len);
            q_total_len += ms_len;
            create->ms = ms_q;
        } else {
            create->ms = NULL;
        }
        if (kb) {
            uint16_t kb_len = q_comm_len + sizeof(bt_addr_t) * (kb->list_num);
            bt_ull_le_hid_srv_conn_params_t *kb_q = bt_ull_le_srv_memory_alloc(kb_len);
            if (!kb_q) {
                ull_assert(0 && BT_ULL_LE_HID_CONN_LOG" keyboard OOM!");
            }
            bt_ull_le_srv_memcpy(kb_q, kb, kb_len);
            q_total_len += kb_len;
            create->kb = kb_q;
        } else {
            create->kb = NULL;
        }
        node->data_len = q_total_len;
        node->data = create;

        if (BT_ULL_LE_HID_CONN_SRV_CIG_STATE_CREATED != cig_state) {
            bt_ull_le_hid_srv_app_scenario_t scen = bt_ull_le_hid_conn_srv_get_scenario();
            //bt_ull_le_srv_phy_t phy = bt_ull_le_hid_conn_srv_get_phy();
            //uint8_t cis_count = bt_ull_le_hid_conn_srv_get_cis_count();

            status = bt_ull_le_hid_conn_srv_create_air_cig(scen);
            if (BT_STATUS_SUCCESS != status) {
                if (create) {
                    if (create->hs) {
                        bt_ull_le_srv_memory_free(create->hs);
                    }
                    if (create->ms) {
                        bt_ull_le_srv_memory_free(create->ms);
                    }
                    if (create->kb) {
                        bt_ull_le_srv_memory_free(create->kb);
                    }
                    bt_ull_le_srv_memory_free(create);
                }
                bt_ull_le_hid_conn_srv_cache_cmd_node_t *create = bt_ull_le_hid_conn_srv_search_cmd_node_by_type(BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS);
                if (create) {
                    bt_ull_le_hid_conn_srv_delete_cmd_node(create);
                }
                ull_report(BT_ULL_LE_HID_CONN_LOG" Create CIG Fail!", 0);
                return status;
            }
        }
        return BT_STATUS_PENDING;
    }
    //bool set_key = false;
    if (hs) {
        headset = bt_ull_le_hid_conn_srv_create_conn_params(hs, ltk, skd, iv, &hs_list_len);
        if (headset) {
            cis_count ++;
        }
    }
    if (kb) {
        keyboard = bt_ull_le_hid_conn_srv_create_conn_params(kb, ltk, skd, iv, &kb_list_len);
        if (keyboard) {
            cis_count ++;
        }
    }
    if (ms) {
        mouse = bt_ull_le_hid_conn_srv_create_conn_params(ms, ltk, skd, iv, &ms_list_len);
        if (mouse) {
            cis_count ++;
        }
    }
    bt_ull_le_hid_conn_srv_alloc_cis_params_bk(hs, kb, ms);
    uint32_t total_len = sizeof(bt_ull_le_create_air_hid_cis_t) -1 + kb_list_len + ms_list_len + hs_list_len;
    bt_ull_le_create_air_hid_cis_t *hid_cis = \
            (bt_ull_le_create_air_hid_cis_t *)bt_ull_le_srv_memory_alloc(total_len); //kb + ms + hs total size
    if (!hid_cis) {
        ull_assert(0 && BT_ULL_LE_HID_CONN_LOG" hid_cis OOM!");
    }
    uint8_t *start_cpy_addr = hid_cis->cis_list;
    uint8_t offset = 0x0;
    if (headset) {
        memcpy(start_cpy_addr + offset, headset, hs_list_len);
        offset += hs_list_len;
    }
    if (keyboard) {
        memcpy(start_cpy_addr + offset, keyboard, kb_list_len);
        offset += kb_list_len;
    }
    if (mouse) {
        memcpy(start_cpy_addr + offset, mouse, ms_list_len);
        offset += ms_list_len;
    }
    hid_cis->create_timeout = g_ull_hid_create_cis_timeout;
    bt_ull_le_srv_memcpy(&hid_cis->ltk, ltk, BT_ULL_LE_HID_DM_LTK_LEN);
    bt_ull_le_srv_memcpy(&hid_cis->skd, skd, BT_ULL_LE_HID_DM_SKD_LEN);
    bt_ull_le_srv_memcpy(&hid_cis->iv, iv, BT_ULL_LE_HID_DM_IV_LEN);
    hid_cis->cis_count = cis_count;
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
    ull_report(BT_ULL_LE_HID_CONN_LOG"[CMD] create_air_cis, ALL: cre_to: %d, ltk:%x-%x..%x, skd: %x-%x-..%x, iv: %x-%x-..%x, cis count: %d", 11, \
        hid_cis->create_timeout,
        hid_cis->ltk[0],
        hid_cis->ltk[1],
        hid_cis->ltk[15],
        hid_cis->skd[0],
        hid_cis->skd[1],
        hid_cis->skd[15],
        hid_cis->iv[0],
        hid_cis->iv[1],
        hid_cis->iv[7],
        hid_cis->cis_count
        );
#endif

    bt_ull_le_hid_conn_srv_lock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS, BT_ULL_LE_HID_CONN_SRV_LOCK);
    if (headset || keyboard || mouse) {
        status = bt_ull_le_create_air_hid_cis(hid_cis);
    } else {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" Create CIS Fail!", 0);
        bt_ull_le_hid_conn_srv_unlock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS);
        if (hid_cis) {
            bt_ull_le_srv_memory_free(hid_cis);
        }
        return status;
    }
    if (BT_STATUS_SUCCESS != status) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" create_air_cis, hci status error: %x!", 1, status);
        bt_ull_le_hid_conn_srv_unlock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS);
        if (hs) {
            bt_ull_le_hid_conn_srv_free_cis_params_bk(BT_ULL_LE_HID_SRV_DEVICE_HEADSET);
            idx = bt_ull_le_hid_conn_srv_get_idx_by_type(BT_ULL_LE_HID_SRV_DEVICE_HEADSET);
            bt_ull_le_hid_conn_srv_set_cis_state(idx, BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTED);
        }
        if (kb) {
            bt_ull_le_hid_conn_srv_free_cis_params_bk(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
            idx = bt_ull_le_hid_conn_srv_get_idx_by_type(BT_ULL_LE_HID_SRV_DEVICE_KEYBOARD);
            bt_ull_le_hid_conn_srv_set_cis_state(idx, BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTED);
        }
        if (ms) {
            bt_ull_le_hid_conn_srv_free_cis_params_bk(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
            idx = bt_ull_le_hid_conn_srv_get_idx_by_type(BT_ULL_LE_HID_SRV_DEVICE_MOUSE);
            bt_ull_le_hid_conn_srv_set_cis_state(idx, BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTED);
        }
    }
    if (headset) {
        bt_ull_le_srv_memory_free(headset);
    }
    if (keyboard) {
        bt_ull_le_srv_memory_free(keyboard);
    }
    if (mouse) {
        bt_ull_le_srv_memory_free(mouse);
    }
    if (hid_cis) {
        bt_ull_le_srv_memory_free(hid_cis);
    }
    return status;

}

bt_status_t bt_ull_le_hid_conn_srv_cancel_create_air_cis(bt_ull_le_hid_srv_device_t device_type)
{
    bt_status_t status = BT_STATUS_FAIL;
    uint8_t idx = bt_ull_le_hid_conn_srv_get_idx_by_type(device_type);
    if (BT_ULL_LE_HID_CONN_SRV_INVALID == idx || BT_ULL_ROLE_SERVER != bt_ull_le_hid_conn_srv_get_role()) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_cancel_create_air_cis, invalid cis type!", 0);
        return status;
    }
    uint8_t state = bt_ull_le_hid_conn_srv_get_cis_state(idx);
    uint16_t is_lock = bt_ull_le_hid_conn_srv_cmd_is_lock(BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_CREATE_CIS);
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
    ull_report(BT_ULL_LE_HID_CONN_LOG"[CMD] bt_ull_le_hid_conn_srv_cancel_create_air_cis, statue: %d, is_lock: %d", 2, \
        state,
        is_lock);
#endif
    if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CONNECTING != state) {
        return status;
    }
    if (is_lock) {
        bt_ull_le_hid_conn_srv_cmd_cache_item_t* node = bt_ull_le_hid_conn_srv_create_cmd_node(BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_CREATE_CIS);
        BT_ULL_LE_HID_MAKE_CMD_PARAM(cancel, BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_CREATE_CIS);
        cancel->device_type = device_type;
        node->cmd = BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_CREATE_CIS;
        node->data_len = BT_ULL_LE_HID_GET_CMD_PARAM_LEN(BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_CREATE_CIS);
        node->data = cancel;
        return BT_STATUS_PENDING;
    }

    bt_ull_le_hid_conn_srv_set_cis_state(idx, BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CANCEL_CONNECTING);
    bt_ull_le_cancel_create_air_hid_cis_params_t params;
    params.cis_handle = bt_ull_le_hid_conn_srv_get_cis_handle(idx);
    params.reason = BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST;
    bt_ull_le_hid_conn_srv_lock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_CREATE_CIS, BT_ULL_LE_HID_CONN_SRV_LOCK);
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
    ull_report(BT_ULL_LE_HID_CONN_LOG"[CMD] bt_ull_le_hid_conn_srv_cancel_create_air_cis, cis_handle: %x, reason: %d", 2, \
        params.cis_handle,
        params.reason);
#endif
    status = bt_ull_le_cancel_creating_air_hid_cis(&params);
    if (BT_STATUS_SUCCESS != status) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_cancel_create_air_cis, error status: %x!", 1, status);
        bt_ull_le_hid_conn_srv_set_cis_state(idx, state);
        bt_ull_le_hid_conn_srv_unlock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_CREATE_CIS);
    }
    return status;
}

bt_status_t bt_ull_le_hid_conn_srv_sync_air_cis(bt_ull_le_hid_srv_device_t type, bt_addr_t *addr)
{
    bt_status_t status = BT_STATUS_FAIL;
    if (BT_ULL_ROLE_CLIENT != bt_ull_le_hid_conn_srv_get_role()) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_sync_air_cis, invalid role!", 0);
        return status;
    }
    bt_ull_le_hid_dm_device_info_t *device_info = bt_ull_le_hid_dm_read_device_info(type, addr);
    if (!device_info) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_sync_air_cis, No Bonding! %2x-%2x-%2x-%2x-%2x-%2x", 6, \
            addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3], addr->addr[4], addr->addr[5]);
        return status;
    }
    uint8_t idx = bt_ull_le_hid_conn_srv_get_idx_by_type(type);
    if (BT_ULL_LE_HID_CONN_SRV_INVALID != idx) {
        if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTED != bt_ull_le_hid_conn_srv_get_cis_state(idx)) {
            ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_sync_air_cis, idex %d, busy state: %x", 2, \
                             idx, bt_ull_le_hid_conn_srv_get_cis_state(idx));
            return status;
        }
    } else {
        idx = bt_ull_le_hid_conn_srv_get_empty_link();
    }

    if (BT_ULL_LE_HID_CONN_SRV_INVALID == idx) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_sync_air_cis, no link resource!", 0);
        return status;
    }
    if (bt_ull_le_hid_conn_srv_cmd_is_lock(BT_ULL_LE_HID_CONN_SRV_CMD_SYNC_CIS)) {
        bt_ull_le_hid_conn_srv_cmd_cache_item_t* node = bt_ull_le_hid_conn_srv_create_cmd_node(BT_ULL_LE_HID_CONN_SRV_CMD_SYNC_CIS);
        BT_ULL_LE_HID_MAKE_CMD_PARAM(sync, BT_ULL_LE_HID_CONN_SRV_CMD_SYNC_CIS);
        sync->device_type = type;
        bt_ull_le_srv_memcpy(&sync->peer_addr, addr, sizeof(bt_addr_t));
        node->cmd = BT_ULL_LE_HID_CONN_SRV_CMD_SYNC_CIS;
        node->data_len = BT_ULL_LE_HID_GET_CMD_PARAM_LEN(BT_ULL_LE_HID_CONN_SRV_CMD_SYNC_CIS);
        node->data = sync;
        return BT_STATUS_PENDING;
    }

    bt_ull_le_hid_conn_srv_set_cis_state(idx, BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CONNECTING);
    bt_ull_le_hid_conn_srv_set_device_type(idx, type);
    bt_ull_le_sync_air_hid_cis_t params;
    params.sync_timeout = 3000;
    params.connection_timeout = 500;
    params.slave_type = type;
    bt_ull_le_srv_memcpy(&params.ltk, &device_info->ltk, BT_ULL_LE_HID_DM_LTK_LEN);
    bt_ull_le_srv_memcpy(&params.skd, &device_info->skd, BT_ULL_LE_HID_DM_SKD_LEN);
    bt_ull_le_srv_memcpy(&params.iv, &device_info->iv, BT_ULL_LE_HID_DM_IV_LEN);
    bt_ull_le_srv_memcpy(&params.peer_addr, addr, sizeof(bt_addr_t));
    bt_ull_le_srv_memcpy(&params.uni_aa, &device_info->uni_aa, BT_ULL_LE_HID_DM_UNI_AA_LEN);
    bt_ull_le_hid_conn_srv_lock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_SYNC_CIS, BT_ULL_LE_HID_CONN_SRV_LOCK);
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
    bt_ull_le_hid_srv_print_addr(&params.peer_addr);
    ull_report(BT_ULL_LE_HID_CONN_LOG"[CMD] sync_air_cis, param(HS): Sync_To: %d, Conn_To: %d, ST: %d, LTK: %x-%x-..%x, SKD: %x-%x-..%x, IV: %x-%x-..%x, AA: %x-%x-%x-%x", 16, \
        params.sync_timeout,
        params.connection_timeout,
        params.slave_type,
        params.ltk[0],
        params.ltk[1],
        params.ltk[15],
        params.skd[0],
        params.skd[1],
        params.skd[15],
        params.iv[0],
        params.iv[1],
        params.iv[7],
        device_info->uni_aa[0],
        device_info->uni_aa[1],
        device_info->uni_aa[2],
        device_info->uni_aa[3]
        );
#endif

    status = bt_ull_le_sync_air_hid_cis(&params);
    if (BT_STATUS_SUCCESS != status) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_sync_air_cis, hci status error: %d!", 1, status);
        bt_ull_le_hid_conn_srv_set_cis_state(idx, BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTED);
        bt_ull_le_hid_conn_srv_set_device_type(idx, BT_ULL_LE_HID_SRV_DEVICE_NONE);
        bt_ull_le_hid_conn_srv_unlock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_SYNC_CIS);
    } else {
        bt_ull_le_hid_conn_srv_set_peer_addr(idx, addr);
    }
    return status;
}

bt_status_t bt_ull_le_hid_conn_srv_cancel_sync_air_cis(bt_ull_le_hid_srv_device_t device_type)
{
    bt_status_t status = BT_STATUS_FAIL;
    uint8_t idx = bt_ull_le_hid_conn_srv_get_idx_by_type(device_type);
    if (BT_ULL_LE_HID_CONN_SRV_INVALID == idx || BT_ULL_ROLE_CLIENT != bt_ull_le_hid_conn_srv_get_role()) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_cancel_sync_air_cis, invalid role!", 0);
        return status;
    }
    uint8_t state = bt_ull_le_hid_conn_srv_get_cis_state(idx);
    if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CONNECTING != state) {
        ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_cancel_sync_air_cis, not in creating state!", 0);
        return status;
    }
    bool is_lock =bt_ull_le_hid_conn_srv_cmd_is_lock(BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_SYNC_CIS);
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_cancel_sync_air_cis, state: %d, cmd_is_lock: %d!", 2, \
         state, is_lock);

#endif

    if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CANCEL_CONNECTING == state) {
        return BT_STATUS_SUCCESS;
    }
    if (is_lock) {
        bt_ull_le_hid_conn_srv_cmd_cache_item_t* node = bt_ull_le_hid_conn_srv_create_cmd_node(BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_SYNC_CIS);
        BT_ULL_LE_HID_MAKE_CMD_PARAM(cancel, BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_SYNC_CIS);
        cancel->device_type = device_type;
        node->cmd = BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_SYNC_CIS;
        node->data_len = BT_ULL_LE_HID_GET_CMD_PARAM_LEN(BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_SYNC_CIS);
        node->data = cancel;
        return BT_STATUS_PENDING;
    }
    bt_ull_le_hid_conn_srv_set_cis_state(idx, BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CANCEL_CONNECTING);
    bt_ull_le_hid_conn_srv_lock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_SYNC_CIS, BT_ULL_LE_HID_CONN_SRV_LOCK);
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
    ull_report(BT_ULL_LE_HID_CONN_LOG"[CMD] bt_ull_le_hid_conn_srv_cancel_sync_air_cis", 0);
#endif
    status = bt_ull_le_cancel_sync_air_hid_cis();
    if (BT_STATUS_SUCCESS != status) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_cancel_sync_air_cis, hci status error: %x!", 1, status);
        bt_ull_le_hid_conn_srv_unlock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_SYNC_CIS);
        bt_ull_le_hid_conn_srv_set_cis_state(idx, state);
    }
    return status;
}
bt_status_t bt_ull_le_hid_conn_srv_disconnect_air_cis(bt_handle_t acl_handle, uint8_t reason)
{
    bt_status_t status = BT_STATUS_FAIL;
    uint8_t idx = bt_ull_le_hid_conn_srv_get_idx_by_acl_handle(acl_handle);
    if (BT_ULL_LE_HID_CONN_SRV_INVALID == idx) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_disconnect_air_cis, invalid cis type!", 0);
        return status;
    }
    uint8_t state = bt_ull_le_hid_conn_srv_get_cis_state(idx);
    if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CONNECTED > state) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_disconnect_air_cis, not connected link!", 0);
        return status;
    }
    uint16_t is_lock = bt_ull_le_hid_conn_srv_cmd_is_lock(BT_ULL_LE_HID_CONN_SRV_CMD_DISCONNECT_CIS);
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_disconnect_air_cis, state: %d, cmd_is_lock: %d!", 2, \
         state, is_lock);
#endif
    if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTING == state) {
        return BT_STATUS_SUCCESS;
    }
    if (is_lock) {
        bt_ull_le_hid_conn_srv_cmd_cache_item_t* node = bt_ull_le_hid_conn_srv_create_cmd_node(BT_ULL_LE_HID_CONN_SRV_CMD_DISCONNECT_CIS);
        BT_ULL_LE_HID_MAKE_CMD_PARAM(disconnect, BT_ULL_LE_HID_CONN_SRV_CMD_DISCONNECT_CIS);
        disconnect->acl_handle = acl_handle;
        disconnect->reason = reason;
        node->cmd = BT_ULL_LE_HID_CONN_SRV_CMD_DISCONNECT_CIS;
        node->data_len = BT_ULL_LE_HID_GET_CMD_PARAM_LEN(BT_ULL_LE_HID_CONN_SRV_CMD_DISCONNECT_CIS);
        node->data = disconnect;
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_disconnect_air_cis, pending!", 0);
        return BT_STATUS_PENDING;
    }
    bt_ull_le_hid_conn_srv_set_cis_state(idx, BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTING);
    bt_ull_le_hid_conn_srv_lock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_DISCONNECT_CIS, BT_ULL_LE_HID_CONN_SRV_LOCK);
    bt_ull_le_air_hid_cis_disconnect_t params;
    params.connection_handle = bt_ull_le_hid_conn_srv_get_cis_handle(idx);
    params.reason = reason;
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
    ull_report(BT_ULL_LE_HID_CONN_LOG"[CMD] bt_ull_le_hid_conn_srv_disconnect_air_cis, cis_handle: %x, reason: %x!", 2, \
         params.connection_handle, params.reason);
#endif

    status = bt_ull_le_disconnect_air_hid_cis(&params);
    if (BT_STATUS_SUCCESS != status) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_disconnect_air_cis, hci status error: %x!", 1, status);
        bt_ull_le_hid_conn_srv_unlock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_DISCONNECT_CIS);
        bt_ull_le_hid_conn_srv_set_cis_state(idx, state);
    }
    return status;

}

static uint8_t bt_ull_le_hid_conn_srv_get_sub_event(uint8_t report_rate)
{
    uint8_t sub_evt = 0x00;
    if (BT_ULL_LE_HID_SRV_REPORT_RATE_LEVEL_1 == report_rate) {
        sub_evt  = 0x03;
    } else if (BT_ULL_LE_HID_SRV_REPORT_RATE_LEVEL_2 == report_rate) {
        sub_evt  = 0x02;
    } else {
       sub_evt = BT_ULL_LE_HID_CONN_SRV_INVALID;
    }
    return sub_evt ;
}

bt_status_t bt_ull_le_hid_conn_srv_set_report_rate(uint8_t rr_level)
{
    bt_status_t status = BT_STATUS_FAIL;
    uint8_t sub_evt = 0x00;
    bt_ull_le_hid_conn_srv_set_rr_level(rr_level);
    uint8_t cig_state = bt_ull_le_hid_conn_srv_get_cig_state();
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_set_report_rate, state: %d", 1, \
         cig_state);
#endif

    if (BT_ULL_LE_HID_CONN_SRV_CIG_STATE_CREATED != cig_state)
    {
        return BT_STATUS_SUCCESS;
    }
    sub_evt = bt_ull_le_hid_conn_srv_get_sub_event(rr_level);
    bt_ull_le_set_air_hid_cis_sub_event_t params;
    params.cig_id = bt_ull_le_hid_conn_srv_get_cig_id();
    params.sub_event = sub_evt;
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_set_report_rate, cig_id: %d, se: %d", 2, \
         params.cig_id,
         params.sub_event);
#endif

    status = bt_ull_le_set_air_hid_cis_sub_event(&params);
    if (BT_STATUS_SUCCESS != status) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_set_report_rate, hci status error: %x!", 1, status);
    }
    return status;
}
bt_status_t bt_ull_le_hid_conn_srv_active_streaming(bt_handle_t acl_handle)
{
    bt_status_t status = BT_STATUS_FAIL;
    uint8_t idx = bt_ull_le_hid_conn_srv_get_idx_by_acl_handle(acl_handle);
    if (BT_ULL_LE_HID_CONN_SRV_INVALID == idx) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_active_streaming, invalid handle!", 0);
        return status;
    }
    uint8_t device_type = bt_ull_le_hid_conn_srv_get_device_type(idx);
    if (BT_ULL_LE_HID_SRV_DEVICE_HEADSET != device_type && BT_ULL_LE_HID_SRV_DEVICE_EARBUDS != device_type) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_active_streaming, invalid cis type!", 0);
        return status;
    }
    uint8_t state = bt_ull_le_hid_conn_srv_get_cis_state(idx);
    if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CONNECTED > state || BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTING == state) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_active_streaming, not connected link!", 0);
        return status;
    }
    uint16_t is_lock = bt_ull_le_hid_conn_srv_cmd_is_lock(BT_ULL_LE_HID_CONN_SRV_CMD_ACTIVE_STREAMING);
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_set_report_rate, state: %d, is_lock: %d", 2, \
         state,
         is_lock);
#endif
    if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_STREAMING == state) {
        return BT_STATUS_SUCCESS;
    }
    if (is_lock) {
        bt_ull_le_hid_conn_srv_cmd_cache_item_t* node = bt_ull_le_hid_conn_srv_create_cmd_node(BT_ULL_LE_HID_CONN_SRV_CMD_ACTIVE_STREAMING);
        BT_ULL_LE_HID_MAKE_CMD_PARAM(active, BT_ULL_LE_HID_CONN_SRV_CMD_ACTIVE_STREAMING);
        active->acl_handle = acl_handle;
        node->cmd = BT_ULL_LE_HID_CONN_SRV_CMD_ACTIVE_STREAMING;
        node->data_len = BT_ULL_LE_HID_GET_CMD_PARAM_LEN(BT_ULL_LE_HID_CONN_SRV_CMD_ACTIVE_STREAMING);
        node->data = active;
        return BT_STATUS_PENDING;
    }

    uint8_t data_path_id = bt_ull_le_hid_conn_srv_get_data_path_id(bt_ull_le_hid_conn_srv_get_cis_id(idx), device_type);
    if (BT_ULL_LE_HID_CONN_SRV_INVALID == data_path_id) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_active_streaming, invalid data path id!", 0);
        return status;
    }
    bt_ull_le_hid_conn_srv_set_cis_state(idx, BT_ULL_LE_HID_CONN_SRV_AIR_CIS_SET_DATA_PATH);
    bt_ull_le_hid_conn_srv_lock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_ACTIVE_STREAMING, BT_ULL_LE_HID_CONN_SRV_LOCK);
    bt_ull_le_setup_air_hid_iso_data_path_t params;
    params.cis_connection_handle = bt_ull_le_hid_conn_srv_get_cis_handle(idx);
    params.data_path_id = data_path_id;
    params.direction = 0;
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
    ull_report(BT_ULL_LE_HID_CONN_LOG"[CMD] bt_ull_le_hid_conn_srv_active_streaming, handle: %x, id: %d, direction: %d", 3, \
        params.cis_connection_handle,
        params.data_path_id,
        params.direction);
#endif
    status = bt_ull_le_setup_air_hid_iso_data_path(&params);
    if (BT_STATUS_SUCCESS != status) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_active_streaming, hci status error: %x!", 1, status);
        bt_ull_le_hid_conn_srv_unlock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_ACTIVE_STREAMING);
        bt_ull_le_hid_conn_srv_set_cis_state(idx, state);
    }

    return status;
}
bt_status_t bt_ull_le_hid_conn_srv_deactive_streaming(bt_handle_t acl_handle)
{
    bt_status_t status = BT_STATUS_FAIL;
    uint8_t idx = bt_ull_le_hid_conn_srv_get_idx_by_acl_handle(acl_handle);
    if (BT_ULL_LE_HID_CONN_SRV_INVALID == idx) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_deactive_streaming, invalid handle!", 0);
        return status;
    }
    uint8_t device_type = bt_ull_le_hid_conn_srv_get_device_type(idx);
    if (BT_ULL_LE_HID_SRV_DEVICE_HEADSET != device_type && BT_ULL_LE_HID_SRV_DEVICE_EARBUDS != device_type) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_deactive_streaming, invalid cis type!", 0);
        return status;
    }
    uint8_t state = bt_ull_le_hid_conn_srv_get_cis_state(idx);
    uint16_t is_lock = bt_ull_le_hid_conn_srv_cmd_is_lock(BT_ULL_LE_HID_CONN_SRV_CMD_DEACTIVE_STREAMING);
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_deactive_streaming, state: %d, is_lock: %d", 2, \
         state,
         is_lock);
#endif

    if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_REMOVE_DATA_PATH == state) {
        return BT_STATUS_SUCCESS;
    }
    if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_STREAMING > state ||  BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTING == state) {
        return status;
    }

    if (is_lock) {
        bt_ull_le_hid_conn_srv_cmd_cache_item_t* node = bt_ull_le_hid_conn_srv_create_cmd_node(BT_ULL_LE_HID_CONN_SRV_CMD_DEACTIVE_STREAMING);
        BT_ULL_LE_HID_MAKE_CMD_PARAM(deactive, BT_ULL_LE_HID_CONN_SRV_CMD_DEACTIVE_STREAMING);
        deactive->acl_handle = acl_handle;
        node->cmd = BT_ULL_LE_HID_CONN_SRV_CMD_DEACTIVE_STREAMING;
        node->data_len = BT_ULL_LE_HID_GET_CMD_PARAM_LEN(BT_ULL_LE_HID_CONN_SRV_CMD_DEACTIVE_STREAMING);
        node->data = deactive;
        return BT_STATUS_PENDING;
    }
    bt_ull_le_hid_conn_srv_set_cis_state(idx, BT_ULL_LE_HID_CONN_SRV_AIR_CIS_REMOVE_DATA_PATH);
    bt_ull_le_hid_conn_srv_lock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_DEACTIVE_STREAMING, BT_ULL_LE_HID_CONN_SRV_LOCK);
    bt_ull_le_remove_air_hid_iso_data_path_t params;
    params.handle = bt_ull_le_hid_conn_srv_get_cis_handle(idx);
    params.data_path_direction = 0x03;
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
    ull_report(BT_ULL_LE_HID_CONN_LOG"[CMD] bt_ull_le_hid_conn_srv_deactive_streaming, handle: %x, direction: %d", 2, \
        params.handle,
        params.data_path_direction);
#endif

    status = bt_ull_le_remove_air_hid_iso_data_path(&params);
    if (BT_STATUS_SUCCESS != status) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_deactive_streaming, hci status error: %x!", 1, status);
        bt_ull_le_hid_conn_srv_unlock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_DEACTIVE_STREAMING);
        bt_ull_le_hid_conn_srv_set_cis_state(idx, state);
    }
    return status;

}

static void bt_ull_le_hid_conn_srv_create_cig_params_cnf_hdl(bt_status_t status, bt_ull_le_set_air_hid_cig_cnf_t *cnf)
{
    uint8_t i = 0;
    uint8_t idx = 0x0;
    uint8_t cig_id = bt_ull_le_hid_conn_srv_get_cig_id();
    uint8_t cig_state = bt_ull_le_hid_conn_srv_get_cig_state();
    //bt_ull_le_hid_conn_srv_msg_ind_t msg;
    bool is_switch_scenario = bt_ull_le_hid_conn_srv_get_scenario_change_state();
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_create_cig_params_cnf_hdl, status : %x, cig_id: %d, cis_count: %d, is_switch_sc: %d", 4, \
        cnf->status, cnf->cig_id, cnf->cis_count, is_switch_scenario);

    bt_ull_le_hid_conn_srv_cache_cmd_node_t *create_cis = bt_ull_le_hid_conn_srv_search_cmd_node_by_type(BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS);
    if(BT_STATUS_SUCCESS != status) {
        bt_ull_le_hid_conn_srv_set_cig_id(BT_ULL_LE_HID_CONN_SRV_CIG_ID_NONE);
        if (BT_ULL_LE_HID_CONN_SRV_CIG_STATE_CREATING == cig_state) {
            bt_ull_le_hid_conn_srv_set_cig_state(BT_ULL_LE_HID_CONN_SRV_CIG_STATE_NONE);
        }
        for (i = 0; i < BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM; i ++) {
            bt_ull_le_hid_conn_srv_clear_cis_info(i);
        }
       // msg.status = status;
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_create_cig_params_cnf_hdl, status error!", 0);
    } else {

        if (cig_id == cnf->cig_id && BT_ULL_LE_HID_CONN_SRV_CIG_STATE_CREATING == cig_state) {
            if (cnf->cis_count > BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM) {
                assert(0);
            }
            if (!cnf->cis_count || !cnf->cis_info) {
                assert(0);
            }
            bt_ull_le_hid_conn_srv_set_cig_state(BT_ULL_LE_HID_CONN_SRV_CIG_STATE_CREATED);
            bt_ull_le_set_air_hid_cis_info_t *cis_info = cnf->cis_info;
            for (i = 0; i < cnf->cis_count; i ++) {
                idx = bt_ull_le_hid_conn_srv_get_idx_by_cis_id(cis_info[i].cis_id);
                if (BT_ULL_LE_HID_CONN_SRV_INVALID == idx) {
                    assert(0);
                }
                ull_report(BT_ULL_LE_HID_CONN_LOG" cis_id: %d, link idx: %d, CIS_Handle: %x", 3, cis_info[i].cis_id, idx, cis_info[i].cis_connection_handle);
                bt_ull_le_hid_conn_srv_set_cis_handle(idx, cis_info[i].cis_connection_handle);
            }
            //msg.status = status;
        } else {
            bt_ull_le_hid_conn_srv_set_cig_state(BT_ULL_LE_HID_CONN_SRV_CIG_STATE_NONE);
            for (i = 0; i < BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM; i ++) {
                bt_ull_le_hid_conn_srv_clear_cis_info(i);
            }
            //msg.status = BT_STATUS_FAIL;
            ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_create_cig_params_cnf_hdl, cig state or cig id error!", 0);
        }
    }
    if (is_switch_scenario) {
        status = bt_ull_le_hid_conn_srv_remove_air_cig();
        return;
    }

    if (create_cis) {
        BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS_T *data = BT_ULL_LE_HID_GET_CMD_PARAM(BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS, create_cis);
        if (BT_STATUS_SUCCESS == status) {
            bt_status_t ret = bt_ull_le_hid_conn_srv_next_action_hdl(BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS, data);
            ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_create_cig_params_cnf_hdl, ret: %d", 1, ret);
        }
        if (data) {
            if (data->hs) {
                //ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_conn_srv_set_air_cig_params_cnf_hdl-1", 0);
                bt_ull_le_srv_memory_free(data->hs);
                //ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_conn_srv_set_air_cig_params_cnf_hdl-2", 0);
            }
            if (data->ms) {
                bt_ull_le_srv_memory_free(data->ms);
            }
            if (data->kb) {
                bt_ull_le_srv_memory_free(data->kb);
            }
            bt_ull_le_srv_memory_free(data);
        }
        bt_ull_le_hid_conn_srv_delete_cmd_node(create_cis);
    }
    //bt_ull_le_hid_conn_srv_notify_msg(BT_ULL_LE_HID_CONN_SRV_MSG_CIG_CREATED_IND, &msg);
}

static void bt_ull_le_hid_conn_srv_cis_established_hdl(bt_status_t status, bt_ull_le_air_hid_cis_established_ind_t *ind)
{
    if (!ind) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_air_cis_established_hdl, ind is null!", 0);
        return;
    }

    bt_ull_role_t role = bt_ull_le_hid_conn_srv_get_role();
    bt_ull_le_hid_conn_srv_msg_ind_t msg;
    uint8_t idx = bt_ull_le_hid_conn_srv_get_idx_by_type(ind->slave_type);
    if (BT_ULL_LE_HID_CONN_SRV_INVALID == idx) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_cis_established_hdl, error idx! handle: %x", 1, ind->cis_connection_handle);
        return;
    }
    uint8_t cis_state = bt_ull_le_hid_conn_srv_get_cis_state(idx);
    bt_ull_le_hid_conn_srv_free_cis_params_bk(ind->slave_type);

    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_cis_established_hdl, status: %x, idx: %d, state: %x, dt: %d, cis_handle: %x, acl_handle: %x", 6, \
        status, idx, cis_state, ind->slave_type, ind->cis_connection_handle, ind->acl_connection_handle);
    if (BT_STATUS_SUCCESS != status) {
        if (BT_ULL_ROLE_SERVER == role) {
            if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CONNECTING == cis_state || BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CANCEL_CONNECTING == cis_state) {
                bt_ull_le_hid_conn_srv_set_cis_state(idx, BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTED);
            } else {
                return;
            }
            bt_ull_le_hid_conn_srv_cache_cmd_node_t *rm = bt_ull_le_hid_conn_srv_search_cmd_node_by_type(BT_ULL_LE_HID_CONN_SRV_CMD_REMOVE_CIG);
            if (rm) {
                BT_ULL_LE_HID_CONN_SRV_CMD_REMOVE_CIG_T *data = BT_ULL_LE_HID_GET_CMD_PARAM(BT_ULL_LE_HID_CONN_SRV_CMD_REMOVE_CIG, rm);
                bt_ull_le_hid_conn_srv_next_action_hdl(BT_ULL_LE_HID_CONN_SRV_CMD_REMOVE_CIG, data);
                if (data) {
                    bt_ull_le_srv_memory_free(data);
                }
                bt_ull_le_hid_conn_srv_delete_cmd_node(rm);
                return;
            }
            if (BT_HCI_STATUS_CONNECTION_FAILED_TO_BE_ESTABLISHED == status) {
                //TODO
            }
        } else {
            if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CONNECTING == cis_state || BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CANCEL_CONNECTING == cis_state) {
                bt_ull_le_hid_conn_srv_clear_cis_info(idx);
                if (!bt_ull_le_hid_conn_srv_get_connected_link()) {
                    bt_ull_le_hid_conn_srv_set_cig_id(BT_ULL_LE_HID_CONN_SRV_CIG_ID_NONE);
                    bt_ull_le_hid_conn_srv_set_cig_state(BT_ULL_LE_HID_CONN_SRV_CIG_STATE_NONE);
                }
            }
        }
        msg.status = status;
        msg.cis_connected.acl_handle = BT_HANDLE_INVALID;
        msg.cis_connected.device_type = ind->slave_type;
        msg.cis_connected.rr_level = bt_ull_le_hid_conn_srv_get_rr_level();
        bt_ull_le_srv_memcpy(&msg.cis_connected.peer_addr, &ind->peer_aadr, sizeof(bt_addr_t));
        bt_ull_le_hid_conn_srv_notify_msg(BT_ULL_LE_HID_CONN_SRV_MSG_CIS_CONNECTED_IND, &msg);
    } else {
        if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CONNECTING == cis_state) {
            bt_ull_le_hid_conn_srv_set_cis_state(idx, BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CONNECTED);
            bt_ull_le_hid_conn_srv_set_acl_handle(idx, ind->acl_connection_handle);
            bt_ull_le_hid_conn_srv_set_cis_handle(idx, ind->cis_connection_handle);
            bt_ull_le_hid_conn_srv_set_peer_addr(idx, &ind->peer_aadr);
            if (BT_ULL_ROLE_CLIENT == role) {
                bt_ull_le_hid_conn_srv_set_cig_id(BT_ULL_LE_HID_CONN_SRV_CIG_ID_1);
                bt_ull_le_hid_conn_srv_set_cig_state(BT_ULL_LE_HID_CONN_SRV_CIG_STATE_CREATED);
                bt_ull_le_hid_conn_srv_set_cis_id(idx, ind->cis_id);
            } else if (BT_ULL_ROLE_SERVER == role) {

            }
            msg.status = status;
            msg.cis_connected.acl_handle = ind->acl_connection_handle;
            msg.cis_connected.device_type = ind->slave_type;
            msg.cis_connected.rr_level = bt_ull_le_hid_conn_srv_get_rr_level();
            bt_ull_le_srv_memcpy(&msg.cis_connected.peer_addr, &ind->peer_aadr, sizeof(bt_addr_t));
            bt_ull_le_hid_conn_srv_notify_msg(BT_ULL_LE_HID_CONN_SRV_MSG_CIS_CONNECTED_IND, &msg);
        } else {
            ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_air_cis_established_hdl, error state: %d!", 1, cis_state);
            return;
        }
    }
}

static void bt_ull_le_hid_conn_srv_cis_disconnect_complete_hdl(bt_status_t status, bt_ull_le_air_hid_cis_disconnect_complete_ind_t *ind)
{
    if (!ind) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_cis_disconnect_complete_hdl, ind is null!", 0);
        return;
    }
    uint8_t idx = BT_ULL_LE_HID_CONN_SRV_INVALID;
    bt_ull_role_t role = bt_ull_le_hid_conn_srv_get_role();
    uint8_t cis_state = 0x0;
    bt_ull_le_hid_conn_srv_msg_ind_t msg;
    idx = bt_ull_le_hid_conn_srv_get_idx_by_cis_handle(ind->cis_connection_handle);
    bt_ull_le_hid_conn_srv_cache_cmd_node_t *rm = bt_ull_le_hid_conn_srv_search_cmd_node_by_type(BT_ULL_LE_HID_CONN_SRV_CMD_REMOVE_CIG);
    if (BT_ULL_LE_HID_CONN_SRV_INVALID == idx) {
        goto NEXT_ACTION;
        //return;
    }
    cis_state = bt_ull_le_hid_conn_srv_get_cis_state(idx);
    msg.cis_disconnected.acl_handle = bt_ull_le_hid_conn_srv_get_acl_handle(idx);
    msg.cis_disconnected.device_type = bt_ull_le_hid_conn_srv_get_device_type(idx);
    msg.cis_disconnected.reason = ind->reason;
    msg.status = status;
    if (status != BT_STATUS_SUCCESS) {
        if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTING == cis_state) {
            assert(0);
        }
    } else {
        if (role == BT_ULL_ROLE_SERVER) {
            bt_ull_le_hid_conn_srv_set_acl_handle(idx, BT_HANDLE_INVALID);
            bt_ull_le_hid_conn_srv_set_cis_state(idx, BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTED);
        } else {
            bt_ull_le_hid_conn_srv_clear_cis_info(idx);
            if (!bt_ull_le_hid_conn_srv_get_connected_link()) {
                bt_ull_le_hid_conn_srv_set_cig_id(BT_ULL_LE_HID_CONN_SRV_CIG_ID_NONE);
                bt_ull_le_hid_conn_srv_set_cig_state(BT_ULL_LE_HID_CONN_SRV_CIG_STATE_NONE);
            }
        }
    }
    bt_ull_le_hid_conn_srv_notify_msg(BT_ULL_LE_HID_CONN_SRV_MSG_CIS_DISCONNECTED_IND, &msg);

NEXT_ACTION:
    if (!bt_ull_le_hid_conn_srv_get_connected_link() && role == BT_ULL_ROLE_SERVER) {
        if (rm) {
            BT_ULL_LE_HID_CONN_SRV_CMD_REMOVE_CIG_T *data = BT_ULL_LE_HID_GET_CMD_PARAM(BT_ULL_LE_HID_CONN_SRV_CMD_REMOVE_CIG, rm);
            bt_ull_le_hid_conn_srv_next_action_hdl(BT_ULL_LE_HID_CONN_SRV_CMD_REMOVE_CIG, data);
            if (data) {
                bt_ull_le_srv_memory_free(data);
            }
            bt_ull_le_hid_conn_srv_delete_cmd_node(rm);
        }
    }
}

static void bt_ull_le_hid_conn_srv_remove_cig_params_cnf_hdl(bt_status_t status, bt_ull_le_remove_air_hid_cig_cnf_t *cnf)
{
    uint8_t i = 0;
    uint8_t cig_state = bt_ull_le_hid_conn_srv_get_cig_state();
    bool is_switch_scenario = bt_ull_le_hid_conn_srv_get_scenario_change_state();
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_remove_cig_params_cnf_hdl, cig state: %d, status: %d, is_switch_sc: %d", 3, \
        cig_state, status, is_switch_scenario);
    if (status != BT_STATUS_SUCCESS) {
        if (BT_ULL_LE_HID_CONN_SRV_CIG_STATE_REMOVING != cig_state) {
            return;
        }
        bt_ull_le_hid_conn_srv_set_cig_state(BT_ULL_LE_HID_CONN_SRV_CIG_STATE_CREATED);
        return;
    } else {
        if (BT_ULL_LE_HID_CONN_SRV_CIG_STATE_REMOVING != cig_state) {
            return;
        }
        bt_ull_le_hid_conn_srv_set_cig_state(BT_ULL_LE_HID_CONN_SRV_CIG_STATE_NONE);
        bt_ull_le_hid_conn_srv_set_cig_id(BT_ULL_LE_HID_CONN_SRV_CIG_ID_NONE);
        for (i = 0;i < BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM; i ++) {
            bt_ull_le_hid_conn_srv_clear_cis_info(i);
        }
        if (is_switch_scenario) {
            bt_ull_le_hid_conn_srv_set_scenario_change_state(false);
            bt_ull_le_hid_conn_srv_cache_cmd_node_t *create_cis = bt_ull_le_hid_conn_srv_search_cmd_node_by_type(BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS);
            if (create_cis) {
                BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS_T *data = BT_ULL_LE_HID_GET_CMD_PARAM(BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS, create_cis);
                bt_ull_le_hid_conn_srv_msg_ind_t msg;
                bt_ull_le_srv_memset(&msg, 0, sizeof(bt_ull_le_hid_conn_srv_msg_ind_t));
                msg.status = status;
                if (data) {
                    msg.scenario_changed.hs = data->hs;
                    msg.scenario_changed.kb = data->kb;
                    msg.scenario_changed.ms = data->ms;
                    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_conn_srv_set_air_cig_params_cnf_hdl, switch scenario, hs: %x, kb: %x, ms: %x", 3, \
                        data->hs, data->kb, data->ms);
                }
                bt_ull_le_hid_conn_srv_notify_msg(BT_ULL_LE_HID_CONN_SRV_MSG_SCENARIO_CHANGED_IND, &msg);

                if (data) {
                    if (data->hs) {
                        //ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_conn_srv_set_air_cig_params_cnf_hdl-1", 0);
                        bt_ull_le_srv_memory_free(data->hs);
                        //ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_conn_srv_set_air_cig_params_cnf_hdl-2", 0);
                    }
                    if (data->ms) {
                        bt_ull_le_srv_memory_free(data->ms);
                    }
                    if (data->kb) {
                        bt_ull_le_srv_memory_free(data->kb);
                    }
                    bt_ull_le_srv_memory_free(data);
                }
                bt_ull_le_hid_conn_srv_delete_cmd_node(create_cis);
            }

        } else {
            bt_ull_le_hid_conn_srv_msg_ind_t msg;
            msg.status = status;
            bt_ull_le_hid_conn_srv_notify_msg(BT_ULL_LE_HID_CONN_SRV_MSG_CIG_REMOVED_IND, &msg);
        }

    }

}

static void bt_ull_le_hid_conn_srv_create_cis_cnf_hdl(bt_status_t status, void *cnf)
{
    uint8_t i =0;
    bt_ull_le_hid_conn_srv_msg_ind_t msg;
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_create_cis_cnf_hdl, status: %d", 1, status);
    bt_ull_le_hid_conn_srv_unlock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS);
    if (status != BT_STATUS_SUCCESS) {
        for (i = 0; i < BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM; i ++) {
            if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CONNECTING == bt_ull_le_hid_conn_srv_get_cis_state(i)) {
                bt_ull_le_hid_conn_srv_set_cis_state(i, BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTED);
                msg.status = status;
                msg.cis_connected.acl_handle = BT_HANDLE_INVALID;
                msg.cis_connected.device_type = bt_ull_le_hid_conn_srv_get_device_type(i);
                msg.cis_connected.rr_level = bt_ull_le_hid_conn_srv_get_rr_level();
                bt_ull_le_hid_conn_srv_notify_msg(BT_ULL_LE_HID_CONN_SRV_MSG_CIS_CONNECTED_IND, &msg);
            }
        }
        bt_ull_le_hid_conn_srv_cache_cmd_node_t *rm = bt_ull_le_hid_conn_srv_search_cmd_node_by_type(BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS);
        if (rm) {
            BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS_T *data = BT_ULL_LE_HID_GET_CMD_PARAM(BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS, rm);
            bt_ull_le_hid_conn_srv_next_action_hdl(BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS, data);
            if (data) {
                bt_ull_le_srv_memory_free(data);
            }
            bt_ull_le_hid_conn_srv_delete_cmd_node(rm);
        }
    }
}

static void bt_ull_le_hid_conn_srv_sync_cis_cnf_hdl(bt_status_t status, void *cnf)
{
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_sync_cis_cnf_hdl, status: %d", 1, status);
    uint8_t i = 0;
    bt_ull_le_hid_conn_srv_msg_ind_t msg;
    bt_ull_le_hid_conn_srv_unlock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_SYNC_CIS);
    if (status != BT_STATUS_SUCCESS) {
        for (i = 0; i < BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM; i ++) {
            if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CONNECTING == bt_ull_le_hid_conn_srv_get_cis_state(i)) {
                bt_ull_le_hid_conn_srv_set_cis_state(i, BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTED);
                bt_ull_le_hid_conn_srv_set_device_type(i, BT_ULL_LE_HID_SRV_DEVICE_NONE);
                break;
            }
        }
        msg.status = status;
        msg.cis_connected.acl_handle = BT_HANDLE_INVALID;
        msg.cis_connected.device_type = bt_ull_le_hid_conn_srv_get_device_type(i);
        msg.cis_connected.rr_level = bt_ull_le_hid_conn_srv_get_rr_level();
        bt_ull_le_hid_conn_srv_notify_msg(BT_ULL_LE_HID_CONN_SRV_MSG_CIS_CONNECTED_IND, &msg);
    }
    bt_ull_le_hid_conn_srv_cache_cmd_node_t *sync = bt_ull_le_hid_conn_srv_search_cmd_node_by_type(BT_ULL_LE_HID_CONN_SRV_CMD_SYNC_CIS);
    if (sync) {
        BT_ULL_LE_HID_CONN_SRV_CMD_SYNC_CIS_T *data = BT_ULL_LE_HID_GET_CMD_PARAM(BT_ULL_LE_HID_CONN_SRV_CMD_SYNC_CIS, sync);
        bt_ull_le_hid_conn_srv_next_action_hdl(BT_ULL_LE_HID_CONN_SRV_CMD_SYNC_CIS, data);
        if (data) {
            bt_ull_le_srv_memory_free(data);
        }
        bt_ull_le_hid_conn_srv_delete_cmd_node(sync);
    }

}

static void bt_ull_le_hid_conn_srv_cancel_create_cnf_hdl(bt_status_t status, void *cnf)
{
    uint8_t i = 0;
    uint8_t idx = BT_ULL_LE_HID_CONN_SRV_INVALID;
    bt_ull_le_hid_conn_srv_unlock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_CREATE_CIS);
    for (i = 0; i < BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM; i ++) {
        if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CANCEL_CONNECTING == bt_ull_le_hid_conn_srv_get_cis_state(i)) {
            idx = i;
            break;
        }
    }
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_cancel_create_cnf_hdl, status: %d, idx: %d", 2, status, idx);
    if (BT_ULL_LE_HID_CONN_SRV_INVALID == idx) {
        return;
    }

    if (status != BT_STATUS_SUCCESS) {
        bt_ull_le_hid_conn_srv_set_cis_state(idx, BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CONNECTING);
    }
    bt_ull_le_hid_conn_srv_msg_ind_t msg;
    msg.status = status;
    msg.cancel_create.device_type = bt_ull_le_hid_conn_srv_get_device_type(idx);
    bt_ull_le_hid_conn_srv_notify_msg(BT_ULL_LE_HID_CONN_SRV_MSG_CIS_CANCEL_CREATE_IND, &msg);
    bt_ull_le_hid_conn_srv_cache_cmd_node_t *cancel = bt_ull_le_hid_conn_srv_search_cmd_node_by_type(BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_CREATE_CIS);
    if (cancel) {
        BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_SYNC_CIS_T *data = BT_ULL_LE_HID_GET_CMD_PARAM(BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_CREATE_CIS, cancel);
        bt_ull_le_hid_conn_srv_next_action_hdl(BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_CREATE_CIS, data);
        if (data) {
            bt_ull_le_srv_memory_free(data);
        }
        bt_ull_le_hid_conn_srv_delete_cmd_node(cancel);
    }

}

static void bt_ull_le_hid_conn_srv_cancel_sync_cnf_hdl(bt_status_t status, void *cnf)
{
    uint8_t i = 0;
    uint8_t idx = BT_ULL_LE_HID_CONN_SRV_INVALID;
    bt_ull_le_hid_conn_srv_unlock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_SYNC_CIS);
    for (i = 0; i < BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM; i ++) {
        if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CANCEL_CONNECTING == bt_ull_le_hid_conn_srv_get_cis_state(i)) {
            idx = i;
            break;
        }
    }
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_cancel_sync_cnf_hdl, status: %d, idx: %d", 2, status, idx);
    if (BT_ULL_LE_HID_CONN_SRV_INVALID == idx) {
        return;
    }

    if (status != BT_STATUS_SUCCESS) {
        bt_ull_le_hid_conn_srv_set_cis_state(idx, BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CONNECTING);
    }
    bt_ull_le_hid_conn_srv_msg_ind_t msg;
    msg.status = status;
    msg.cancel_sync.device_type = bt_ull_le_hid_conn_srv_get_device_type(idx);
    bt_ull_le_hid_conn_srv_notify_msg(BT_ULL_LE_HID_CONN_SRV_MSG_CIS_CANCEL_SYNC_IND, &msg);
    bt_ull_le_hid_conn_srv_cache_cmd_node_t *cancel = bt_ull_le_hid_conn_srv_search_cmd_node_by_type(BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_SYNC_CIS);
    if (cancel) {
        BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_SYNC_CIS_T *data = BT_ULL_LE_HID_GET_CMD_PARAM(BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_SYNC_CIS, cancel);
        bt_ull_le_hid_conn_srv_next_action_hdl(BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_SYNC_CIS, data);
        if (data) {
            bt_ull_le_srv_memory_free(data);
        }
        bt_ull_le_hid_conn_srv_delete_cmd_node(cancel);
    }

}

static void bt_ull_le_hid_conn_srv_active_streaming_cnf_hdl(bt_status_t status, bt_ull_le_setup_air_hid_iso_data_path_cnf_t *cnf)
{
    bt_ull_le_hid_conn_srv_unlock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_ACTIVE_STREAMING);
    bt_ull_le_hid_conn_srv_cache_cmd_node_t *active = bt_ull_le_hid_conn_srv_search_cmd_node_by_type(BT_ULL_LE_HID_CONN_SRV_CMD_ACTIVE_STREAMING);
    if (!cnf) {
        goto NEXT_ACTION;
        return;
    }
    uint8_t idx = bt_ull_le_hid_conn_srv_get_idx_by_cis_handle(cnf->handle);
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_active_streaming_cnf_hdl, status: %x, handle: %d, idx: %d", 3, status, cnf->handle, idx);
    if (BT_ULL_LE_HID_CONN_SRV_INVALID == idx) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_active_streaming_cnf_hdl, invalid idx!", 0);
        goto NEXT_ACTION;
        return;
    }
    uint8_t cis_state = bt_ull_le_hid_conn_srv_get_cis_state(idx);
    if (status != BT_STATUS_SUCCESS) {
        if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_SET_DATA_PATH == cis_state) {
            bt_ull_le_hid_conn_srv_set_cis_state(idx, BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CONNECTED);
        } else {
            ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_active_streaming_cnf_hdl, cis state error!", 0);
            goto NEXT_ACTION;
        }
    } else {
        if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_SET_DATA_PATH == cis_state) {
            bt_ull_le_hid_conn_srv_set_cis_state(idx, BT_ULL_LE_HID_CONN_SRV_AIR_CIS_STREAMING);
        } else {
            ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_active_streaming_cnf_hdl, cis state error-2!", 0);
            goto NEXT_ACTION;
        }
    }
    bt_ull_le_hid_conn_srv_msg_ind_t msg;
    msg.status = status;
    msg.streaming_active.acl_handle = bt_ull_le_hid_conn_srv_get_acl_handle(idx);
    msg.streaming_active.device_type= bt_ull_le_hid_conn_srv_get_device_type(idx);
    bt_ull_le_hid_conn_srv_notify_msg(BT_ULL_LE_HID_CONN_SRV_MSG_CIS_ACTIVE_STREAMING_IND, &msg);
NEXT_ACTION:

    if (active) {
        BT_ULL_LE_HID_CONN_SRV_CMD_ACTIVE_STREAMING_T *data = BT_ULL_LE_HID_GET_CMD_PARAM(BT_ULL_LE_HID_CONN_SRV_CMD_ACTIVE_STREAMING, active);
        bt_ull_le_hid_conn_srv_next_action_hdl(BT_ULL_LE_HID_CONN_SRV_CMD_ACTIVE_STREAMING, data);
        if (data) {
            bt_ull_le_srv_memory_free(data);
        }
        bt_ull_le_hid_conn_srv_delete_cmd_node(active);
    }
}

static void bt_ull_le_hid_conn_srv_deactive_streaming_cnf_hdl(bt_status_t status, bt_ull_le_remove_air_hid_iso_data_path_cnf_t *cnf)
{
    bt_ull_le_hid_conn_srv_unlock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_DEACTIVE_STREAMING);
    bt_ull_le_hid_conn_srv_cache_cmd_node_t *deactive = bt_ull_le_hid_conn_srv_search_cmd_node_by_type(BT_ULL_LE_HID_CONN_SRV_CMD_DEACTIVE_STREAMING);
    if (!cnf) {
        goto NEXT_ACTION;
        return;
    }
    uint8_t idx = bt_ull_le_hid_conn_srv_get_idx_by_cis_handle(cnf->handle);
    if (BT_ULL_LE_HID_CONN_SRV_INVALID == idx) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_deactive_streaming_cnf_hdl, invalid idx!", 0);
        goto NEXT_ACTION;
        return;
    }
    uint8_t cis_state = bt_ull_le_hid_conn_srv_get_cis_state(idx);
    if (status != BT_STATUS_SUCCESS) {
        if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_REMOVE_DATA_PATH == cis_state) {
            bt_ull_le_hid_conn_srv_set_cis_state(idx, BT_ULL_LE_HID_CONN_SRV_AIR_CIS_STREAMING);
        } else {
            ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_deactive_streaming_cnf_hdl, cis state error!", 0);
            goto NEXT_ACTION;
        }
    } else {
        if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_REMOVE_DATA_PATH == cis_state) {
            bt_ull_le_hid_conn_srv_set_cis_state(idx, BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CONNECTED);
        } else {
            ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_deactive_streaming_cnf_hdl, cis state error-2!", 0);
            goto NEXT_ACTION;
        }
    }
    bt_ull_le_hid_conn_srv_msg_ind_t msg;
    msg.status = status;
    msg.streaming_deactive.acl_handle = bt_ull_le_hid_conn_srv_get_acl_handle(idx);
    msg.streaming_deactive.device_type= bt_ull_le_hid_conn_srv_get_device_type(idx);
    bt_ull_le_hid_conn_srv_notify_msg(BT_ULL_LE_HID_CONN_SRV_MSG_CIS_INACTIVE_STREAMING_IND, &msg);


NEXT_ACTION:
    if (deactive) {
        BT_ULL_LE_HID_CONN_SRV_CMD_DEACTIVE_STREAMING_T *data = BT_ULL_LE_HID_GET_CMD_PARAM(BT_ULL_LE_HID_CONN_SRV_CMD_DEACTIVE_STREAMING, deactive);
        bt_ull_le_hid_conn_srv_next_action_hdl(BT_ULL_LE_HID_CONN_SRV_CMD_DEACTIVE_STREAMING, data);
        if (data) {
            bt_ull_le_srv_memory_free(data);
        }
        bt_ull_le_hid_conn_srv_delete_cmd_node(deactive);
    }

}

static void bt_ull_le_hid_conn_srv_disconnect_cis_cnf_hdl(bt_status_t status, void *cnf)
{
    bt_ull_le_hid_conn_srv_unlock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_DISCONNECT_CIS);
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_disconnect_cis_cnf_hdl, status: %x", 1, status);
    if (BT_STATUS_SUCCESS != status) {
        uint8_t i = 0;
        for (i = 0; i < BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM; i ++) {
            if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTING == bt_ull_le_hid_conn_srv_get_cis_state(i)) {
                bt_ull_le_hid_conn_srv_set_cis_state(i, BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CONNECTED);
            }
        }
        bt_ull_le_hid_conn_srv_cache_cmd_node_t *rm = bt_ull_le_hid_conn_srv_search_cmd_node_by_type(BT_ULL_LE_HID_CONN_SRV_CMD_REMOVE_CIG);
        if (rm) {
            BT_ULL_LE_HID_CONN_SRV_CMD_REMOVE_CIG_T *data = BT_ULL_LE_HID_GET_CMD_PARAM(BT_ULL_LE_HID_CONN_SRV_CMD_REMOVE_CIG, rm);
            bt_ull_le_hid_conn_srv_next_action_hdl(BT_ULL_LE_HID_CONN_SRV_CMD_REMOVE_CIG, data);
            if (data) {
                bt_ull_le_srv_memory_free(data);
            }
            bt_ull_le_hid_conn_srv_delete_cmd_node(rm);
        }
    } else {
        bt_ull_le_hid_conn_srv_cache_cmd_node_t *dis = bt_ull_le_hid_conn_srv_search_cmd_node_by_type(BT_ULL_LE_HID_CONN_SRV_CMD_DISCONNECT_CIS);
        if (dis) {
            BT_ULL_LE_HID_CONN_SRV_CMD_DISCONNECT_CIS_T *data = BT_ULL_LE_HID_GET_CMD_PARAM(BT_ULL_LE_HID_CONN_SRV_CMD_DISCONNECT_CIS, dis);
            bt_ull_le_hid_conn_srv_next_action_hdl(BT_ULL_LE_HID_CONN_SRV_CMD_DISCONNECT_CIS, data);
            if (data) {
                bt_ull_le_srv_memory_free(data);
            }
            bt_ull_le_hid_conn_srv_delete_cmd_node(dis);
        }
    }
}

static void bt_ull_le_hid_conn_srv_unmute_cis_cnf_hdl(bt_status_t status, void *cnf)
{
    bt_ull_le_hid_conn_srv_unlock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_UNMUTE_CIS);
    bt_ull_le_hid_conn_srv_cache_cmd_node_t *node = bt_ull_le_hid_conn_srv_search_cmd_node_by_type(BT_ULL_LE_HID_CONN_SRV_CMD_UNMUTE_CIS);
    if (status) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_unmute_cis_cnf_hdl, status error: %x!", 1, status);
    }

    if (node) {
        BT_ULL_LE_HID_CONN_SRV_CMD_UNMUTE_CIS_T *unmute = BT_ULL_LE_HID_GET_CMD_PARAM(BT_ULL_LE_HID_CONN_SRV_CMD_UNMUTE_CIS, node);
        bt_ull_le_hid_conn_srv_next_action_hdl(BT_ULL_LE_HID_CONN_SRV_CMD_UNMUTE_CIS, unmute);
        if (unmute) {
            bt_ull_le_srv_memory_free(unmute);
        }
        bt_ull_le_hid_conn_srv_delete_cmd_node(node);
    }
}

bt_status_t bt_ull_le_hid_conn_srv_unmute_air_cis(bt_handle_t acl_handle)
{

    bt_status_t status = BT_STATUS_FAIL;
    uint8_t idx = bt_ull_le_hid_conn_srv_get_idx_by_acl_handle(acl_handle);
    if (BT_ULL_LE_HID_CONN_SRV_INVALID == idx) {ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_unmute_air_cis, invalid handle!", 0);
        return status;

    }
    uint8_t device_type = bt_ull_le_hid_conn_srv_get_device_type(idx);
    if (BT_ULL_LE_HID_SRV_DEVICE_HEADSET != device_type && BT_ULL_LE_HID_SRV_DEVICE_EARBUDS != device_type) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_unmute_air_cis, invalid cis type!", 0);
        return status;
    }
    uint8_t state = bt_ull_le_hid_conn_srv_get_cis_state(idx);
    if (BT_ULL_LE_HID_CONN_SRV_AIR_CIS_CONNECTED > state || BT_ULL_LE_HID_CONN_SRV_AIR_CIS_DISCONNECTING == state) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_unmute_air_cis, not connected link!", 0);
        return status;
    }
    uint16_t is_lock = bt_ull_le_hid_conn_srv_cmd_is_lock(BT_ULL_LE_HID_CONN_SRV_CMD_UNMUTE_CIS);
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_unmute_air_cis, state: %d, is_lock: %d", 2, \
         state,
         is_lock);
#endif

    if (is_lock) {
        bt_ull_le_hid_conn_srv_cmd_cache_item_t* node = bt_ull_le_hid_conn_srv_create_cmd_node(BT_ULL_LE_HID_CONN_SRV_CMD_UNMUTE_CIS);
        BT_ULL_LE_HID_MAKE_CMD_PARAM(unmute, BT_ULL_LE_HID_CONN_SRV_CMD_UNMUTE_CIS);
        unmute->acl_handle = acl_handle;
        node->cmd = BT_ULL_LE_HID_CONN_SRV_CMD_UNMUTE_CIS;
        node->data_len = BT_ULL_LE_HID_GET_CMD_PARAM_LEN(BT_ULL_LE_HID_CONN_SRV_CMD_UNMUTE_CIS);
        node->data = unmute;
        return BT_STATUS_PENDING;
    }

    bt_ull_le_hid_conn_srv_lock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_UNMUTE_CIS, BT_ULL_LE_HID_CONN_SRV_LOCK);
    bt_ull_le_unmute_air_cis_t param;
    param.cis_connection_handle = bt_ull_le_hid_conn_srv_get_cis_handle(idx);
    status = bt_ull_le_unmute_air_cis(&param);

#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
    ull_report(BT_ULL_LE_HID_CONN_LOG"[CMD] bt_ull_le_hid_conn_srv_unmute_air_cis, cis handle: %x", 1, \
     param.cis_connection_handle);
#endif

    if (BT_STATUS_SUCCESS != status) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_unmute_air_cis, hci status error: %x!", 1, status);
        bt_ull_le_hid_conn_srv_unlock_cmd(BT_ULL_LE_HID_CONN_SRV_CMD_UNMUTE_CIS);
    }
    return status;
}

static bt_status_t bt_ull_le_hid_conn_srv_next_action_hdl(bt_ull_le_hid_conn_srv_cmd_enum next_act, void *data)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_next_action_hdl, NEXT ACTION: %d", 1, next_act);
    switch (next_act) {
        case BT_ULL_LE_HID_CONN_SRV_CMD_REMOVE_CIG: {
            status = bt_ull_le_hid_conn_srv_remove_air_cig();
            break;
        }
        case BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_CREATE_CIS: {
            if (!data) {
                status = BT_STATUS_FAIL;
            }
            BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_CREATE_CIS_T *cmd = (BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_CREATE_CIS_T *)data;
            status = bt_ull_le_hid_conn_srv_cancel_create_air_cis(cmd->device_type);
            break;
        }
        case BT_ULL_LE_HID_CONN_SRV_CMD_SYNC_CIS: {
            if (!data) {
                status = BT_STATUS_FAIL;
            }

            BT_ULL_LE_HID_CONN_SRV_CMD_SYNC_CIS_T *cmd = (BT_ULL_LE_HID_CONN_SRV_CMD_SYNC_CIS_T *)data;
            status = bt_ull_le_hid_conn_srv_sync_air_cis(cmd->device_type, &cmd->peer_addr);
            break;
        }
        case BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_SYNC_CIS: {
            if (!data) {
                status = BT_STATUS_FAIL;
            }
            BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_SYNC_CIS_T *cmd = (BT_ULL_LE_HID_CONN_SRV_CMD_CANCEL_SYNC_CIS_T *)data;

            status = bt_ull_le_hid_conn_srv_cancel_sync_air_cis(cmd->device_type);
            break;
        }
        case BT_ULL_LE_HID_CONN_SRV_CMD_ACTIVE_STREAMING: {
            if (!data) {
                status = BT_STATUS_FAIL;
            }
            BT_ULL_LE_HID_CONN_SRV_CMD_ACTIVE_STREAMING_T *cmd = (BT_ULL_LE_HID_CONN_SRV_CMD_ACTIVE_STREAMING_T *)data;

            status = bt_ull_le_hid_conn_srv_active_streaming(cmd->acl_handle);

            break;
        }
        case BT_ULL_LE_HID_CONN_SRV_CMD_DEACTIVE_STREAMING: {
            if (!data) {
                status = BT_STATUS_FAIL;
            }
            BT_ULL_LE_HID_CONN_SRV_CMD_DEACTIVE_STREAMING_T *cmd = (BT_ULL_LE_HID_CONN_SRV_CMD_DEACTIVE_STREAMING_T *)data;

            status = bt_ull_le_hid_conn_srv_deactive_streaming(cmd->acl_handle);
            break;
        }
        case BT_ULL_LE_HID_CONN_SRV_CMD_DISCONNECT_CIS: {
            if (!data) {
                status = BT_STATUS_FAIL;
            }
            BT_ULL_LE_HID_CONN_SRV_CMD_DISCONNECT_CIS_T *cmd = (BT_ULL_LE_HID_CONN_SRV_CMD_DISCONNECT_CIS_T *)data;

            status = bt_ull_le_hid_conn_srv_disconnect_air_cis(cmd->acl_handle, cmd->reason);
            break;
        }
        case BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS: {
            if (!data) {
                status = BT_STATUS_FAIL;
            }
            BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS_T *cmd = (BT_ULL_LE_HID_CONN_SRV_CMD_CREATE_CIS_T *)data;
            status = bt_ull_le_hid_conn_srv_create_air_cis(cmd->hs, cmd->kb, cmd->ms);
            break;
        }
        case BT_ULL_LE_HID_CONN_SRV_CMD_UNMUTE_CIS: {
            if (!data) {
                status = BT_STATUS_FAIL;
            }
            BT_ULL_LE_HID_CONN_SRV_CMD_UNMUTE_CIS_T *cmd = (BT_ULL_LE_HID_CONN_SRV_CMD_UNMUTE_CIS_T *)data;
            status = bt_ull_le_hid_conn_srv_unmute_air_cis(cmd->acl_handle);
            break;
        }
        default:
            break;
    }

    return status;
}

static bt_status_t bt_ull_le_hid_conn_srv_event_callback(bt_msg_type_t msg, bt_status_t ret, void *data)
{
    switch (msg) {
    case BT_ULL_LE_AIR_HID_CIS_ESTABLISHED_IND: {
        bt_ull_le_air_hid_cis_established_ind_t *event = (bt_ull_le_air_hid_cis_established_ind_t *)data;
        if (!event) {
             assert(0);
             return BT_STATUS_FAIL;
        }
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
        ull_report(BT_ULL_LE_HID_CONN_LOG"[EVT] cis established event: status: %x, acl_handle: %x, cis_handle: %x, cis_id: %d, slave_type: %d, uni_AA: %x-%x-%x-%x, addr: %x:%x:%x:%x:%x:%x", 15, \
            ret, event->acl_connection_handle, event->cis_connection_handle, \
            event->cis_id, event->slave_type, \
            event->uni_aa[0], event->uni_aa[1], event->uni_aa[2], event->uni_aa[3], \
            event->peer_aadr.addr[0], event->peer_aadr.addr[1], event->peer_aadr.addr[2], event->peer_aadr.addr[3], event->peer_aadr.addr[4], event->peer_aadr.addr[5]);

#endif
        bt_ull_le_hid_conn_srv_cis_established_hdl(ret, event);
        break;
    }
    case BT_ULL_LE_AIR_HID_CIS_DISCONNECT_COMPLETE_IND: {
        bt_ull_le_air_hid_cis_disconnect_complete_ind_t *event = (bt_ull_le_air_hid_cis_disconnect_complete_ind_t *)data;
        if (!event) {
             assert(0);
             return BT_STATUS_FAIL;
        }
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
        ull_report(BT_ULL_LE_HID_CONN_LOG"[EVT] cis disconnect complete event: status: %x, acl_handle: %x, reason: %x", 3, \
            ret, event->cis_connection_handle, event->reason);
#endif

        bt_ull_le_hid_conn_srv_cis_disconnect_complete_hdl(ret, event);
        break;
    }
    case BT_ULL_LE_SET_AIR_HID_CIG_PARAMS_CNF: {
        bt_ull_le_set_air_hid_cig_cnf_t *event = (bt_ull_le_set_air_hid_cig_cnf_t *)data;
        if (!event) {
             assert(0);
             return BT_STATUS_FAIL;
        }
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
        ull_report(BT_ULL_LE_HID_CONN_LOG"[EVT] set cig parameter cmd status: status: %x, cig_id: %x, cis_count: %x", 3, \
            ret, event->cig_id, event->cis_count);
#endif

        bt_ull_le_hid_conn_srv_create_cig_params_cnf_hdl(ret, event);
        break;
    }
    case BT_ULL_LE_REMOVE_AIR_HID_CIG_CNF: {
        bt_ull_le_remove_air_hid_cig_cnf_t *event = (bt_ull_le_remove_air_hid_cig_cnf_t *)data;
        if (!event) {
             assert(0);
             return BT_STATUS_FAIL;
        }
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
        ull_report(BT_ULL_LE_HID_CONN_LOG"[EVT] remove cig parameter cmd status: status: %x, cig_id: %x", 2, \
            ret, event->cig_id);
#endif

        bt_ull_le_hid_conn_srv_remove_cig_params_cnf_hdl(ret, event);
        break;
    }
    case BT_ULL_LE_CREATE_AIR_HID_CIS_CNF: {
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
        ull_report(BT_ULL_LE_HID_CONN_LOG"[EVT] create air cis cmd status: status: %x", 1, \
            ret);
#endif

        bt_ull_le_hid_conn_srv_create_cis_cnf_hdl(ret, data);
        break;
    }
    case BT_ULL_LE_SYNC_AIR_HID_CIS_CNF: {
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
        ull_report(BT_ULL_LE_HID_CONN_LOG"[EVT] sync air cis cmd status: status: %x", 1, \
            ret);
#endif

        bt_ull_le_hid_conn_srv_sync_cis_cnf_hdl(ret, data);
        break;
    }
    case BT_ULL_LE_CANCEL_CREATE_AIR_HID_CIS_CNF: {
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
        ull_report(BT_ULL_LE_HID_CONN_LOG"[EVT] cacnel create air cis cmd status: status: %x", 1, \
            ret);
#endif
        bt_ull_le_hid_conn_srv_cancel_create_cnf_hdl(ret, data);
        break;
    }
    case BT_ULL_LE_CANCEL_SYNC_AIR_HID_CIS_CNF: {
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
        ull_report(BT_ULL_LE_HID_CONN_LOG"[EVT] cacnel sync air cis cmd status: status: %x", 1, \
            ret);
#endif
        bt_ull_le_hid_conn_srv_cancel_sync_cnf_hdl(ret, data);
        break;
    }
    case BT_ULL_LE_SETUP_AIR_HID_ISO_DATA_PATH_CNF: {
        bt_ull_le_setup_air_hid_iso_data_path_cnf_t *event = (bt_ull_le_setup_air_hid_iso_data_path_cnf_t *)data;
        if (!event) {
             assert(0);
             return BT_STATUS_FAIL;
        }
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
        ull_report(BT_ULL_LE_HID_CONN_LOG"[EVT] set iso data path id compelte status: status: %x, handle: %x", 2, \
            ret, event->handle);
#endif
        bt_ull_le_hid_conn_srv_active_streaming_cnf_hdl(ret, event);
        break;
    }
    case BT_ULL_LE_REMOVE_AIR_HID_ISO_DATA_PATH_CNF: {
        bt_ull_le_remove_air_hid_iso_data_path_cnf_t *event = (bt_ull_le_remove_air_hid_iso_data_path_cnf_t *)data;
        if (!event) {
             assert(0);
             return BT_STATUS_FAIL;
        }
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
        ull_report(BT_ULL_LE_HID_CONN_LOG"[EVT] remove iso data path id cmd status: status: %x, handle: %x", 2, \
            ret, event->handle);
#endif

        bt_ull_le_hid_conn_srv_deactive_streaming_cnf_hdl(ret, data);
        break;
    }
    case BT_ULL_LE_DISCONNECT_AIR_HID_CIS_CNF: {
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
        ull_report(BT_ULL_LE_HID_CONN_LOG"[EVT] disconnect air cis cmd status: status: %x", 1, \
            ret);
#endif
        bt_ull_le_hid_conn_srv_disconnect_cis_cnf_hdl(ret, data);
        break;
    }
    case BT_ULL_LE_UNMUTE_AIR_CIS_CNF: {
#ifdef BT_ULL_LE_HID_CONN_SRV_DEBUG
        ull_report(BT_ULL_LE_HID_CONN_LOG"[EVT] unmute air cis cmd status: status: %x", 1, \
            ret);
#endif
        bt_ull_le_hid_conn_srv_unmute_cis_cnf_hdl(ret, data);
        break;
    }
    case BT_ULL_LE_SET_AIR_HID_CIS_SUB_EVENT_CNF:
    default:
        break;

    }
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_ull_le_hid_conn_srv_init(bt_ull_role_t role, bt_ull_le_hid_conn_srv_callback_t cb)
{
    ull_report(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_init, callback: 0x%x, role: %d", 2, cb, role);
    bt_status_t status = BT_STATUS_FAIL;
    bt_ull_le_srv_memset(&g_ull_hid_srv_ctx, 0, sizeof(bt_ull_le_hid_conn_srv_contex_t));
    if (BT_STATUS_SUCCESS != bt_ull_le_hid_conn_srv_register_callback(cb)) {
        ull_report_error(BT_ULL_LE_HID_CONN_LOG" bt_ull_le_hid_conn_srv_init, call back function is null!", 0);
        return BT_STATUS_FAIL;
    }

    g_ull_hid_srv_ctx.cmd_list_header = NULL;
    g_ull_hid_srv_ctx.scenario = BT_ULL_LE_HID_SRV_APP_SCENARIO_1;
    bt_ull_le_hid_conn_srv_set_role(role);
    bt_ull_le_hid_conn_srv_set_phy(BT_ULL_LE_SRV_PHY_LE_2M);
    bt_ull_le_init();
    status = bt_callback_manager_register_callback(bt_callback_type_app_event, MODULE_MASK_ULL, (void*)bt_ull_le_hid_conn_srv_event_callback);
    return status;
}

void bt_ull_le_hid_conn_srv_deinit(void)
{
    uint8_t i = 0;
    bt_ull_le_hid_conn_srv_delete_all_cmd_node();
    for (i = 0; i < BT_ULL_LE_HID_CONN_SRV_CIS_MAX_NUM; i ++) {
        bt_ull_le_srv_memset(&g_ull_hid_srv_ctx.cis_info[i].peer_addr, 0, sizeof(bt_addr_t));
    }
    for (i = 0; i < BT_ULL_LE_HID_CONN_SRV_CMD_MAX; i ++) {
        bt_ull_le_hid_conn_srv_unlock_cmd(i);
    }
    bt_ull_le_hid_conn_srv_set_cig_state(BT_ULL_LE_HID_CONN_SRV_CIG_STATE_NONE);
    bt_ull_le_hid_conn_srv_set_cig_id(BT_ULL_LE_HID_CONN_SRV_CIG_ID_NONE);
    g_ull_hid_srv_ctx.switch_scenario_state.is_switch_scenaro = false;
}

static bt_ull_le_hid_conn_srv_cig_params_t * bt_ull_le_hid_conn_srv_get_cig_params(bt_ull_le_hid_srv_app_scenario_t scenario)
{
    uint8_t i = 0;
    for (i = 0; i < BT_ULL_LE_HID_CONN_SRV_AIR_PARAMS_MAX_NUM; i ++) {
        if (g_air_cig_params_tab[i].scenario == scenario) {
            return &g_air_cig_params_tab[i];
        }
    }
    return NULL;
}


/* Copyright Statement:
*
* (C) 2022 Airoha Technology Corp. All rights reserved.
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
* the License Agreement ("Permitted User"). If you are not a Permitted User,
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

/* MediaTek restricted information */

#include "bt_ull_le_conn_service.h"
#include "stddef.h"
#include "bt_callback_manager.h"
#include "bt_ull_le_audio_manager.h"
//#include "FreeRTOS.h"
//#include "bt_ull_le_utility.h"

#define BT_ULL_LE_CONN_SRV_CMD_DEBUG
#define BT_ULL_LE_CONN_SRV_DEBUG

#define BT_ULL_LE_CONN_SRV_INVALID_IDX                             (0xFF)

#define BT_ULL_LE_CONN_SRV_MAX_CIG_NUM  BT_ULL_LE_CONN_SRV_LATENCY_MODE_MAX  /* Latency mode type max num*/

#define BT_ULL_LE_CONN_SRV_CIG_STATE_NONE              (0x00)
#define BT_ULL_LE_CONN_SRV_CIG_STATE_CREATING          (0x01)
#define BT_ULL_LE_CONN_SRV_CIG_STATE_CREATED           (0x02)
#define BT_ULL_LE_CONN_SRV_CIG_STATE_REMOVING          (0x03)

#define BT_ULL_LE_CONN_SRV_FT_1                        (0x01)
#define BT_ULL_LE_CONN_SRV_FT_2                        (0x02)
#define BT_ULL_LE_CONN_SRV_FT_3                        (0x03)
#define BT_ULL_LE_CONN_SRV_FT_4                        (0x04)
#define BT_ULL_LE_CONN_SRV_FT_5                        (0x05)
typedef uint8_t bt_ull_le_conn_srv_flush_timeout_t; /*times*/

/* CIS releated */
#define BT_ULL_LE_CONN_SRV_AIR_CIG_ID_NONE             (0x00)    /* none */
#define BT_ULL_LE_CONN_SRV_AIR_CIG_ID_1                (0x01)    /* unidirectional (c->s) */
#define BT_ULL_LE_CONN_SRV_AIR_CIG_ID_2                (0x02)    /* bidirectional (c->s) */
typedef uint8_t bt_ull_le_conn_srv_cig_id_t;

#define BT_ULL_LE_CONN_SRV_ACTION_NONE                    (0x00)
#define BT_ULL_LE_CONN_SRV_ACTION_ESTABLISH_AIR_CIS       (0x01)
#define BT_ULL_LE_CONN_SRV_ACTION_DESTROY_AIR_CIS         (0x02)
#define BT_ULL_LE_CONN_SRV_ACTION_SETUP_ISO_DATA_PATH     (0x03)
#define BT_ULL_LE_CONN_SRV_ACTION_REMOVE_ISO_DATA_PATH    (0x04)
#define BT_ULL_LE_CONN_SRV_ACTION_REMOVE_AIR_CIG_PARAMS   (0x05)
#define BT_ULL_LE_CONN_SRV_ACTION_SET_AIR_CIG_PARAMS      (0x06)
#define BT_ULL_LE_CONN_SRV_ACTION_UNMUTE_AIR_CIS          (0x07)
#define BT_ULL_LE_CONN_SRV_ACTION_ENABLE_UPLINK           (0x08)
#define BT_ULL_LE_CONN_SRV_ACTION_MAX                     (0x09)
typedef uint8_t bt_ull_le_conn_srv_action_t;

#define BT_ULL_LE_CONN_SRV_AIR_CIS_DISCONNECTED        (0x00)
#define BT_ULL_LE_CONN_SRV_AIR_CIS_ACTIVE_CONNECTING   (0x01)
#define BT_ULL_LE_CONN_SRV_AIR_CIS_PASSIVE_CONNECTING  (0x02)
#define BT_ULL_LE_CONN_SRV_AIR_CIS_CONNECTED           (0x03)
#define BT_ULL_LE_CONN_SRV_AIR_CIS_SET_DATA_PATH       (0x04)
#define BT_ULL_LE_CONN_SRV_AIR_CIS_ACTIVIATED          (0x05)
#define BT_ULL_LE_CONN_SRV_AIR_CIS_RM_DATA_PATH        (0x06)/*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: Add inactive ISO state*/
#define BT_ULL_LE_CONN_SRV_AIR_CIS_DISCONNECTING       (0x07)
#define BT_ULL_LE_CONN_SRV_AIR_CIS_STATE_MAX           (0x08)
typedef uint8_t bt_ull_le_conn_srv_air_cis_state_t;

#define BT_ULL_LE_CONN_SRV_ESTABLISHMENT_EVT_MASK   (0x01)
#define BT_ULL_LE_CONN_SRV_DISCONNECT_EVT_MASK      (0x02)
#define BT_ULL_LE_CONN_SRV_SET_ISO_DATA_PATH_EVT_MASK     (0x03)
#define BT_ULL_LE_CONN_SRV_ALL_MASK                 (0x04)
typedef uint8_t bt_ull_le_conn_srv_mask_t;

typedef uint16_t bt_ull_le_conn_srv_cmd_lock;

typedef enum {
    BT_ULL_LE_CONN_SRV_CMD_DISCONNECT,
    BT_ULL_LE_CONN_SRV_CMD_ESTABLISH,
    BT_ULL_LE_CONN_SRV_CMD_REMOVE_CIG,
    BT_ULL_LE_CONN_SRV_CMD_REMOVE_ISO_DATA_PATH,
    BT_ULL_LE_CONN_SRV_CMD_SET_ISO_DATA_PATH,
    BT_ULL_LE_CONN_SRV_CMD_UNMUTE,
    BT_ULL_LE_CONN_SRV_CMD_CHANGE_UL_PATH,
    BT_ULL_LE_CONN_SRV_CMD_CHANGE_LABEL,
    BT_ULL_LE_CONN_SRV_CMD_SET_TABLE,
    BT_ULL_LE_CONN_SRV_CMD_MAX
} bt_ull_le_conn_srv_cmd_enum;

#define BT_ULL_LE_CONN_SRV_AIR_PARAMS_MAX_NUM            0x04

typedef struct {
    bt_handle_t cis_handle;
    bt_handle_t acl_handle;
    bt_ull_le_srv_air_cis_id_enum cis_id;
    bt_ull_le_conn_srv_air_cis_state_t state;
    bool dl_enable; /*active or inactive*/
    bool ul_enable; /*active or inactive*/
} bt_ull_le_conn_srv_air_cis_link_info_t;

typedef struct {
    bt_ull_le_audio_location_t audio_location;
    bt_ull_le_srv_iso_data_path_t data_path_id;
    bt_ull_le_srv_air_cis_id_enum cis_id;
} bt_ull_le_conn_srv_cis_id_type_t;

typedef struct {
    bt_ull_le_conn_srv_cmd_enum cmd;
    bt_handle_t handle;
} bt_ull_le_conn_srv_cmd_cache_item_t;

typedef struct bt_ull_le_conn_srv_cache_cmd_node_t bt_ull_le_conn_srv_cache_cmd_node_t;
struct bt_ull_le_conn_srv_cache_cmd_node_t {
    bt_ull_le_conn_srv_cache_cmd_node_t *next;
    bt_ull_le_conn_srv_cmd_cache_item_t cache_item;
};

typedef struct {
    bt_ull_le_conn_srv_cmd_enum cmd;
    bt_ull_le_conn_srv_cmd_lock lock;
} bt_ull_le_conn_srv_cmd_lock_t;

typedef struct {
    uint8_t establish;
    uint8_t disconnect;
    uint8_t set_iso_data_path;
} bt_ull_le_conn_srv_evt_mask_t;

typedef struct {
    bt_ull_le_srv_latency_t                latency;
    bt_ull_le_conn_srv_flush_timeout_t     ft_m_to_s;
    bt_ull_le_conn_srv_flush_timeout_t     ft_s_to_m;
} bt_ull_le_conn_srv_frame_duration_param_t;

typedef struct {
    bt_ull_le_srv_audio_quality_t          audio_quality;  
    bt_ull_le_codec_t                      codec;
    bt_ull_le_conn_srv_phy_t               phy_m_2_s;
    bt_ull_le_conn_srv_phy_t               phy_s_2_m;
    uint8_t                                max_share_num;
    uint8_t                                max_ul_num;
    bt_ull_le_srv_iso_interval_t           iso_interval;
    uint32_t                               dl_bitrate;
    uint32_t                               ul_bitrate;
} bt_ull_le_conn_srv_air_cig_param_t;

typedef struct {
    bt_ull_le_srv_latency_t               latency;
    bt_ull_le_srv_audio_quality_t         audio_quality;
    uint8_t                               label_count;
    bt_ull_le_conn_srv_air_cig_param_t    *select_cig_params;
    bt_ull_le_codec_t                     codec;
} bt_ull_le_conn_srv_audio_ctrl_t;


typedef struct {
    bt_ull_role_t                          role;
    uint8_t                                cig_id;
    uint8_t                                cis_count; /*set by ull le service*/
    uint8_t                                cig_state;
    bt_ull_le_conn_srv_phy_t               phy;
    bt_ull_le_conn_srv_audio_ctrl_t        aud_ctrl;
    bt_ull_le_conn_srv_evt_mask_t          evt_mask;
    bt_ull_le_conn_callback_t              cb;
    bt_ull_le_conn_srv_cache_cmd_node_t    *cis_cmd_list_header;
    //bt_ull_le_conn_srv_stream_mode_t stream_mode; /*set by ull le service*/
    bt_ull_le_conn_srv_air_cis_link_info_t cis_info[BT_ULL_LE_AIR_CIS_MAX_NUM];
} bt_ull_le_conn_srv_context_t;

static bt_ull_le_conn_srv_frame_duration_param_t g_bt_ull_latency_ft_table [BT_ULL_LE_FT_LABEL_MAX] = {
    {BT_ULL_LE_SRV_LATENCY_SINGLE_LINK_MODE,                                     BT_ULL_LE_CONN_SRV_FT_1, BT_ULL_LE_CONN_SRV_FT_1},
    {BT_ULL_LE_SRV_LATENCY_MULTI_LINK_LEA_INTERVAL_MORE_THAN_30MS_STANDBY_MODE,  BT_ULL_LE_CONN_SRV_FT_2, BT_ULL_LE_CONN_SRV_FT_2},
    {BT_ULL_LE_SRV_LATENCY_MULTI_LINK_A2DP_STANDBY_MODE,                         BT_ULL_LE_CONN_SRV_FT_3, BT_ULL_LE_CONN_SRV_FT_3},
    {BT_ULL_LE_SRV_LATENCY_MULTI_LINK_CONNECTING_MODE,                           BT_ULL_LE_CONN_SRV_FT_5, BT_ULL_LE_CONN_SRV_FT_5}
};

static bt_ull_le_conn_srv_air_cig_param_t g_cig_param_table_opus[] = {
    {
        BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT,
        BT_ULL_LE_CODEC_OPUS,
        BT_ULL_LE_SRV_PHY_LE_2M,
        BT_ULL_LE_SRV_PHY_LE_2M,
        0x2,
        0x1,
        BT_ULL_LE_SRV_ISO_INTERVAL_5MS,
        BT_ULL_LE_DEFAULT_BITRATE_320_0_KBPS,
        BT_ULL_LE_DEFAULT_BITRATE_64_0_KBPS
    }
};

static bt_ull_le_conn_srv_air_cig_param_t g_cig_param_table_uld_wireless_mic[] = {
    {
        BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT,
        BT_ULL_LE_CODEC_ULD,
        BT_ULL_LE_SRV_PHY_LE_2M,
        BT_ULL_LE_SRV_PHY_LE_2M,
        0x2,
        0x1,
        BT_ULL_LE_SRV_ISO_INTERVAL_15MS,
        0x0,
        BT_ULL_LE_DEFAULT_BITRATE_200_0_KBPS
    }
};

static bt_ull_le_conn_srv_air_cig_param_t g_cig_param_table_uld_default[] = {
    {
        BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT,
        BT_ULL_LE_CODEC_ULD,
        BT_ULL_LE_SRV_PHY_LE_2M,
        BT_ULL_LE_SRV_PHY_LE_2M,
        0x2,
        0x1,
        BT_ULL_LE_SRV_ISO_INTERVAL_15MS,
        0x0,
        BT_ULL_LE_DEFAULT_BITRATE_200_0_KBPS
    }
};


static bt_ull_le_conn_srv_air_cig_param_t g_cig_param_table_lc3plus_wireless_mic[] = {
    {
        BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT,
        BT_ULL_LE_CODEC_LC3PLUS,
        BT_ULL_LE_SRV_PHY_LE_2M,
        BT_ULL_LE_SRV_PHY_LE_2M,
        0x2,
        0x1,
        BT_ULL_LE_SRV_ISO_INTERVAL_5MS,
        0x0,
        BT_ULL_LE_DEFAULT_BITRATE_201_6_KBPS
    }
};

static bt_ull_le_conn_srv_air_cig_param_t g_cig_param_table_lc3plus_low_power[] = {
    {
        BT_ULL_LE_SRV_AUDIO_QUALITY_LOW_POWER,
        BT_ULL_LE_CODEC_LC3PLUS,
        BT_ULL_LE_SRV_PHY_LE_2M,
        BT_ULL_LE_SRV_PHY_LE_2M,
        0x2,
        0x1,
        BT_ULL_LE_SRV_ISO_INTERVAL_5MS,
        BT_ULL_LE_DEFAULT_BITRATE_201_6_KBPS,
        BT_ULL_LE_DEFAULT_BITRATE_64_0_KBPS
    },
    {
        BT_ULL_LE_SRV_AUDIO_QUALITY_LOW_POWER,
        BT_ULL_LE_CODEC_LC3PLUS,
        BT_ULL_LE_CONN_SRV_PHY_EDR_LE_8M,
        BT_ULL_LE_CONN_SRV_PHY_EDR_LE_8M,
        0x2,
        0x1,
        BT_ULL_LE_SRV_ISO_INTERVAL_5MS,
        BT_ULL_LE_DEFAULT_BITRATE_940_8_KBPS,
        BT_ULL_LE_DEFAULT_BITRATE_104_0_KBPS
    }
};

static bt_ull_le_conn_srv_air_cig_param_t g_cig_param_table_lc3plus_dchs[] = {
    {
        BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT,
        BT_ULL_LE_CODEC_LC3PLUS,
        BT_ULL_LE_SRV_PHY_LE_2M,
        BT_ULL_LE_SRV_PHY_LE_2M,
        0x2,
        0x1,
        BT_ULL_LE_SRV_ISO_INTERVAL_5MS,
        BT_ULL_LE_DEFAULT_BITRATE_201_6_KBPS,
        BT_ULL_LE_DEFAULT_BITRATE_64_0_KBPS
    }
};

static bt_ull_le_conn_srv_air_cig_param_t g_cig_param_table_lc3plus_defualt[] = {
    {
        BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT,
        BT_ULL_LE_CODEC_LC3PLUS,
        BT_ULL_LE_SRV_PHY_LE_2M,
        BT_ULL_LE_SRV_PHY_LE_2M,
        0x2,
        0x1,
        BT_ULL_LE_SRV_ISO_INTERVAL_5MS,
        BT_ULL_LE_DEFAULT_BITRATE_304_0_KBPS,
        BT_ULL_LE_DEFAULT_BITRATE_64_0_KBPS
    },
    {
        BT_ULL_LE_SRV_AUDIO_QUALITY_TWO_STREAMING_HFP,
        BT_ULL_LE_CODEC_LC3PLUS,
        BT_ULL_LE_SRV_PHY_LE_2M,
        BT_ULL_LE_SRV_PHY_LE_2M,
        0x2,
        0x0,
        BT_ULL_LE_SRV_ISO_INTERVAL_5MS,
        BT_ULL_LE_DEFAULT_BITRATE_252_8_KBPS,
        0x0

    },
    {
        BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_QUALITY,
        BT_ULL_LE_CODEC_LC3PLUS,
        BT_ULL_LE_SRV_PHY_EDR_LE_4M,
        BT_ULL_LE_SRV_PHY_EDR_LE_4M,
        0x2,
        0x1,
        BT_ULL_LE_SRV_ISO_INTERVAL_5MS,
        BT_ULL_LE_DEFAULT_BITRATE_560_0_KBPS,
        BT_ULL_LE_DEFAULT_BITRATE_104_0_KBPS

    },
    {
        BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_RESOLUTION,
        BT_ULL_LE_CODEC_LC3PLUS,
        BT_ULL_LE_SRV_PHY_EDR_LE_8M,
        BT_ULL_LE_SRV_PHY_EDR_LE_8M,
        0x2,
        0x1,
        BT_ULL_LE_SRV_ISO_INTERVAL_5MS,
        BT_ULL_LE_DEFAULT_BITRATE_940_8_KBPS,
        BT_ULL_LE_DEFAULT_BITRATE_104_0_KBPS
    }
};

static bt_ull_le_conn_srv_air_cig_param_t g_cig_param_table_lc3plus_highres[] = {
    {
        BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT,
        BT_ULL_LE_CODEC_LC3PLUS,
        BT_ULL_LE_SRV_PHY_LE_2M,
        BT_ULL_LE_SRV_PHY_LE_2M,
        0x2,
        0x1,
        BT_ULL_LE_SRV_ISO_INTERVAL_5MS,
        BT_ULL_LE_DEFAULT_BITRATE_272_0_KBPS,
        BT_ULL_LE_DEFAULT_BITRATE_104_0_KBPS

    },
    {
        BT_ULL_LE_SRV_AUDIO_QUALITY_TWO_STREAMING_HFP,
        BT_ULL_LE_CODEC_LC3PLUS,
        BT_ULL_LE_SRV_PHY_LE_2M,
        BT_ULL_LE_SRV_PHY_LE_2M,
        0x2,
        0x0,
        BT_ULL_LE_SRV_ISO_INTERVAL_5MS,
        BT_ULL_LE_DEFAULT_BITRATE_252_8_KBPS,
        0x0

    },
    {
        BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_QUALITY,
        BT_ULL_LE_CODEC_LC3PLUS,
        BT_ULL_LE_SRV_PHY_EDR_LE_4M,
        BT_ULL_LE_SRV_PHY_EDR_LE_4M,
        0x2,
        0x1,
        BT_ULL_LE_SRV_ISO_INTERVAL_5MS,
        BT_ULL_LE_DEFAULT_BITRATE_560_0_KBPS,
        BT_ULL_LE_DEFAULT_BITRATE_104_0_KBPS

    },
    {
        BT_ULL_LE_SRV_AUDIO_QUALITY_HIGH_RESOLUTION,
        BT_ULL_LE_CODEC_LC3PLUS,
        BT_ULL_LE_SRV_PHY_EDR_LE_8M,
        BT_ULL_LE_SRV_PHY_EDR_LE_8M,
        0x2,
        0x1,
        BT_ULL_LE_SRV_ISO_INTERVAL_5MS,
        BT_ULL_LE_DEFAULT_BITRATE_940_8_KBPS,
        BT_ULL_LE_DEFAULT_BITRATE_104_0_KBPS
    }
};

static bt_ull_le_conn_srv_air_cig_param_t g_bt_ull_cig_param_table_uld_ull3[] = {
   {
       BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT,
       BT_ULL_LE_CODEC_DL_ULD_UL_LC3PLUS,
       BT_ULL_LE_CONN_SRV_PHY_EDR_LE_4M,
       BT_ULL_LE_CONN_SRV_PHY_EDR_LE_4M,
       BT_ULL_LE_DEFAULT_MAX_SHARE_NUM_2,
       0x1,
       BT_ULL_LE_SRV_ISO_INTERVAL_10MS,
       BT_ULL_LE_DEFAULT_BITRATE_400_0_KBPS,
       BT_ULL_LE_DEFAULT_BITRATE_64_0_KBPS
  }
};

static bt_ull_le_conn_srv_cmd_lock_t g_bt_ull_cmd_lock_table[BT_ULL_LE_CONN_SRV_CMD_MAX + 1] = {
    {BT_ULL_LE_CONN_SRV_CMD_DISCONNECT, 0},
    {BT_ULL_LE_CONN_SRV_CMD_ESTABLISH, 0},
    {BT_ULL_LE_CONN_SRV_CMD_REMOVE_CIG, 0},
    {BT_ULL_LE_CONN_SRV_CMD_REMOVE_ISO_DATA_PATH, 0},
    {BT_ULL_LE_CONN_SRV_CMD_SET_ISO_DATA_PATH, 0},
    {BT_ULL_LE_CONN_SRV_CMD_UNMUTE, 0},
    {BT_ULL_LE_CONN_SRV_CMD_CHANGE_UL_PATH, 0},
    {BT_ULL_LE_CONN_SRV_CMD_CHANGE_LABEL, 0},
    {BT_ULL_LE_CONN_SRV_CMD_SET_TABLE, 0},
    {BT_ULL_LE_CONN_SRV_CMD_MAX, 0} /*max*/
};

static const bt_ull_le_conn_srv_cis_id_type_t g_bt_ull_cis_id_table[] = {
    {BT_ULL_LE_AUDIO_LOCATION_FRONT_LEFT, BT_ULL_LE_SRV_DATA_PATH_ID_1, BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK1},
    {BT_ULL_LE_AUDIO_LOCATION_FRONT_RIGHT, BT_ULL_LE_SRV_DATA_PATH_ID_2, BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK2}
};

static const bt_ull_le_conn_srv_cis_id_type_t g_bt_ull_mic_cis_id_table[] = {
    {BT_ULL_LE_AUDIO_LOCATION_NONE, BT_ULL_LE_SRV_DATA_PATH_ID_1, BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK1},
    {BT_ULL_LE_AUDIO_LOCATION_NONE, BT_ULL_LE_SRV_DATA_PATH_ID_2, BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK2},
    {BT_ULL_LE_AUDIO_LOCATION_NONE, BT_ULL_LE_SRV_DATA_PATH_ID_3, BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK3},
    {BT_ULL_LE_AUDIO_LOCATION_NONE, BT_ULL_LE_SRV_DATA_PATH_ID_4, BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK4}
};

static const bt_ull_le_conn_srv_cis_id_type_t g_bt_ull_spk_cis_id_table[] = {
    {BT_ULL_LE_AUDIO_LOCATION_NONE, BT_ULL_LE_SRV_DATA_PATH_ID_SPK_SPECIAL, BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK1},
    {BT_ULL_LE_AUDIO_LOCATION_NONE, BT_ULL_LE_SRV_DATA_PATH_ID_SPK_SPECIAL, BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK2},
};

#ifdef AIR_LE_AUDIO_ENABLE
     extern void bt_sink_srv_cap_stream_reset_avm_buffer(void);
#endif
/*global variable*/
static bt_ull_le_conn_srv_context_t g_bt_ull_le_conn_ctrl;

static bt_status_t bt_ull_le_conn_srv_event_callback(bt_msg_type_t msg, bt_status_t ret, void *buff);

/*for internal use, handle ull profile event*/
static bt_status_t bt_ull_le_conn_srv_register_callback(bt_ull_le_conn_callback_t callback);

static void bt_ull_le_conn_srv_air_cis_connect_request_handler(bt_status_t status, bt_ull_le_air_cis_request_ind_t *ind);
static void bt_ull_le_conn_srv_air_cis_established_handler(bt_status_t status, bt_ull_le_air_cis_established_ind_t *ind);
static void bt_ull_le_conn_srv_air_cis_destroied_handler(bt_status_t status, bt_ull_le_air_cis_disconnect_complete_ind_t *ind);
static void bt_ull_le_conn_srv_air_cig_params_changed_handler(bt_status_t status, bt_ull_le_air_cig_params_changed_ind_t *ind);
static void bt_ull_le_conn_srv_air_cis_uplink_activiated_handler(bt_status_t status, bt_ull_le_air_cis_uplink_activiated_ind_t *ind);
static void bt_ull_le_conn_srv_qos_report_handler(bt_status_t status, bt_ull_le_qos_report_ind_t *report);

static void bt_ull_le_conn_srv_set_air_cig_params_cnf_handler(bt_status_t status, bt_ull_le_set_air_cig_params_cnf_t *cnf);
static void bt_ull_le_conn_srv_remove_air_cig_params_cnf_handler(bt_status_t status, bt_ull_le_remove_air_cig_cnf_t *cnf);
static void bt_ull_le_conn_srv_change_air_cig_params_cnf_handler(bt_status_t status, bt_ull_le_air_change_air_params_cnf_t *cnf);
static void bt_ull_le_conn_srv_create_air_cis_cnf_handler(bt_status_t status, void *cnf);
static void bt_ull_le_conn_srv_destroy_air_cis_cnf_handler(bt_status_t status, void *cnf);
static void bt_ull_le_conn_srv_reply_air_cis_request_cnf_handler(bt_status_t status, void *cnf);
static void bt_ull_le_conn_srv_set_air_iso_data_path_cnf_handler(bt_status_t status, bt_ull_le_setup_air_iso_data_path_cnf_t *cnf);
static void bt_ull_le_conn_srv_remove_air_iso_data_path_cnf_handler(bt_status_t status, bt_ull_le_remove_air_iso_data_path_cnf_t *cnf);
static void bt_ull_le_conn_srv_unmute_air_cis_cnf_handler(bt_status_t status, void *cnf);
static void bt_ull_le_conn_srv_activiate_uplink_cnf_handler(bt_status_t status, void *cnf);
static void bt_ull_le_conn_srv_set_air_params_table_cnf_handler(bt_status_t status, void *cnf);

static void bt_ull_le_conn_srv_notify(bt_ull_le_conn_srv_event_t event, void *msg);
/*for internal use, send cmd to ull profile*/
static bt_status_t bt_ull_le_conn_srv_create_air_cis_internal(bt_handle_t acl_handle, uint8_t cis_count);
static bt_status_t bt_ull_le_conn_srv_destroy_air_cis_internal(bt_handle_t cis_handle);
static bt_status_t bt_ull_le_conn_srv_setup_air_iso_data_path_internal(bt_handle_t cis_handle);
static bt_status_t bt_ull_le_conn_srv_remove_air_iso_data_path_internal(bt_handle_t cis_handle);
static bt_status_t bt_ull_le_conn_srv_accept_air_cis_request_internal(bt_handle_t cis_handle);
static bt_status_t bt_ull_le_conn_srv_reject_air_cis_request_internal(bt_handle_t cis_handle);
static bt_status_t bt_ull_le_conn_srv_unmute_air_cis_internal(bt_handle_t cis_handle);

static bt_status_t bt_ull_le_conn_srv_next_action_flow_handler(uint8_t next_action, void *data);

/*for internal use, operate g_bt_ull_le_conn_ctrl*/
static void bt_ull_le_conn_srv_clear_evt_mask(bt_ull_le_conn_srv_mask_t type);
static void bt_ull_le_conn_srv_reset_cis_info(uint8_t idx);
static void bt_ull_le_conn_srv_set_role(bt_ull_role_t role);
static void bt_ull_le_conn_srv_set_cig_id(uint8_t cig_id);
static void bt_ull_le_conn_srv_set_cig_state(uint8_t state);
static void bt_ull_le_conn_srv_set_cis_count(uint8_t cis_count);
static void bt_ull_le_conn_srv_set_phy(bt_ull_le_conn_srv_phy_t phy);
static void bt_ull_le_conn_srv_set_codec_type(bt_ull_le_codec_t codec);
static void bt_ull_le_conn_srv_set_dl_active_state(uint8_t idx, bool is_enable);
static void bt_ull_le_conn_srv_set_ul_active_state(uint8_t idx, bool is_enable);
static void bt_ull_le_conn_srv_set_acl_handle(uint8_t idx, bt_handle_t acl_handle);
static void bt_ull_le_conn_srv_set_cis_handle(uint8_t idx, bt_handle_t cis_handle);
static void bt_ull_le_conn_srv_set_cis_id(uint8_t idx, bt_ull_le_srv_air_cis_id_enum cis_id);
static void bt_ull_le_conn_srv_set_cis_state(uint8_t idx, bt_ull_le_conn_srv_air_cis_state_t state);

uint8_t bt_ull_le_conn_srv_get_phy(void);
static bt_ull_le_codec_t bt_ull_le_conn_srv_get_codec_type(void);
static bt_ull_le_conn_srv_context_t *bt_ull_le_conn_srv_get_ctx(void);
static uint8_t bt_ull_le_conn_srv_get_cig_id(void);
static uint8_t bt_ull_le_conn_srv_get_cig_state(void);
static uint8_t bt_ull_le_conn_srv_get_cis_count(void);
static uint8_t bt_ull_le_conn_srv_get_evt_mask(bt_ull_le_conn_srv_mask_t type);
static uint8_t bt_ull_le_conn_srv_get_audio_quality(void);
static uint8_t bt_ull_le_conn_srv_get_streaming_cis_num(void);
static uint8_t bt_ull_le_conn_srv_get_connected_cis_num(void);
static uint8_t bt_ull_le_conn_srv_get_connecting_cis_num(void);
static uint8_t bt_ull_le_conn_srv_get_cis_id(uint8_t idx);
static uint8_t bt_ull_le_conn_srv_get_cis_state(uint8_t idx);
static uint8_t bt_ull_le_conn_srv_get_ul_active_state(uint8_t idx);
uint8_t bt_ull_le_conn_srv_get_dl_active_state(uint8_t idx);
static uint8_t bt_ull_le_conn_srv_get_idx_by_cis_handle(bt_handle_t cis_handle);
static uint8_t bt_ull_le_conn_srv_get_data_path_id(bt_ull_le_srv_air_cis_id_enum cis_id, bt_ull_client_t ct);
static uint8_t bt_ull_le_conn_srv_cis_is_connected(uint8_t idx);
bt_ull_le_conn_srv_air_cig_param_t *bt_ull_le_conn_srv_get_air_cig_params(bt_ull_le_srv_audio_quality_t quality);
static bt_ull_le_conn_srv_frame_duration_param_t *bt_ull_le_conn_srv_get_frame_duration_params(bt_ull_le_srv_latency_t latency);

static uint8_t bt_ull_le_conn_srv_get_air_cig_tabel_idx(bt_ull_le_srv_audio_quality_t quality);
static uint8_t bt_ull_le_conn_srv_get_frame_duration_idx(bt_ull_le_srv_latency_t latency);

uint8_t bt_ull_le_conn_srv_get_latency(void);
static bt_handle_t bt_ull_le_conn_srv_get_acl_handle(uint8_t idx);
static bt_handle_t bt_ull_le_conn_srv_get_cis_handle(uint8_t idx);
static bt_ull_role_t bt_ull_le_conn_srv_get_role(void);
static bt_ull_client_t bt_ull_le_conn_srv_get_client_type(void);
bt_ull_le_conn_srv_air_cis_link_info_t *bt_ull_le_conn_srv_get_cis_info(uint8_t idx);
static uint8_t bt_ull_le_conn_srv_get_idx_by_location(bt_ull_le_audio_location_t location);

/*for cache hci cmd*/
static bt_ull_le_conn_srv_cmd_cache_item_t* bt_ull_le_conn_srv_new_cache_cmd_node(bt_ull_le_conn_srv_cmd_enum cmd);
static bt_ull_le_conn_srv_cache_cmd_node_t* bt_ull_le_conn_srv_search_cache_cmd_node_by_type(bt_ull_le_conn_srv_cmd_enum cmd);
bt_ull_le_conn_srv_cache_cmd_node_t* bt_ull_le_conn_srv_search_cache_cmd_by_handle(bt_ull_le_conn_srv_cmd_enum cmd, bt_handle_t handle);
static uint8_t bt_ull_le_conn_srv_delete_cache_cmd_node(bt_ull_le_conn_srv_cache_cmd_node_t *delete_node);
static void bt_ull_le_conn_srv_delete_all_cmd_node(void);

/*for lock hci cmd*/
static void bt_ull_le_conn_srv_lock_cmd(bt_ull_le_conn_srv_cmd_enum cmd, bt_handle_t handle);
static void bt_ull_le_conn_srv_unlock_cmd(bt_ull_le_conn_srv_cmd_enum cmd);
static uint16_t bt_ull_le_conn_srv_cmd_is_lock(bt_ull_le_conn_srv_cmd_enum cmd);
static bt_status_t bt_ull_le_conn_srv_register_callback(bt_ull_le_conn_callback_t callback)
{
    if (!callback) {
        return BT_STATUS_FAIL;
    }
    g_bt_ull_le_conn_ctrl.cb = callback;
    return BT_STATUS_SUCCESS;
}
bt_status_t bt_ull_le_conn_srv_deinit(void)
{
#ifdef BT_ULL_LE_CONN_SRV_DEBUG
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_deinit", 0);
#endif
    uint8_t link_num;
    bt_ull_le_conn_srv_delete_all_cmd_node();
    for (link_num = 0; link_num < BT_ULL_LE_AIR_CIS_MAX_NUM; link_num++) {
        bt_ull_le_conn_srv_reset_cis_info(link_num);
    }
    uint8_t i = 0;
    for (i = 0; i < BT_ULL_LE_CONN_SRV_CMD_MAX; i ++) {
        bt_ull_le_conn_srv_unlock_cmd(i);
    }
    g_bt_ull_le_conn_ctrl.cig_state = BT_ULL_LE_CONN_SRV_CIG_STATE_NONE;
    bt_ull_le_srv_memset(&g_bt_ull_le_conn_ctrl.evt_mask, 0, sizeof(bt_ull_le_conn_srv_evt_mask_t));
    return BT_STATUS_SUCCESS;
}

bt_status_t bt_ull_le_conn_srv_init(bt_ull_role_t role, bt_ull_le_conn_callback_t callback)
{
#ifdef BT_ULL_LE_CONN_SRV_DEBUG
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_init, callback: 0x%x, role: %d", 2, callback, role);
#endif
    uint8_t link_num;
    bt_ull_le_srv_memset(&g_bt_ull_le_conn_ctrl, 0, sizeof(bt_ull_le_conn_srv_context_t));
    if (BT_STATUS_SUCCESS != bt_ull_le_conn_srv_register_callback(callback)) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_init, call back function is null!", 0);
        return BT_STATUS_FAIL;
    }
    for (link_num = 0; link_num < BT_ULL_LE_AIR_CIS_MAX_NUM; link_num++) {
        bt_ull_le_conn_srv_reset_cis_info(link_num);
    }
    g_bt_ull_le_conn_ctrl.cis_cmd_list_header = NULL;
    g_bt_ull_le_conn_ctrl.aud_ctrl.audio_quality = BT_ULL_LE_SRV_AUDIO_QUALITY_TYPE_DEFAULT;
    g_bt_ull_le_conn_ctrl.aud_ctrl.latency = BT_ULL_LE_SRV_LATENCY_DEFAULT;
    g_bt_ull_le_conn_ctrl.aud_ctrl.label_count = 0x0;
    g_bt_ull_le_conn_ctrl.aud_ctrl.codec = BT_ULL_LE_CODEC_LC3PLUS;
    g_bt_ull_le_conn_ctrl.aud_ctrl.select_cig_params = NULL;
    bt_ull_le_conn_srv_set_role(role);
    bt_ull_le_init();
    bt_status_t status = bt_callback_manager_register_callback(bt_callback_type_app_event, MODULE_MASK_ULL, (void*)bt_ull_le_conn_srv_event_callback);
    return status;
}

static bt_status_t bt_ull_le_conn_srv_event_callback(bt_msg_type_t msg, bt_status_t ret, void *buff)
{
    switch (msg) {
    case BT_ULL_LE_AIR_CIS_CONNECT_REQUEST_IND: {
            if (!buff) {
                ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_event_callback, buff is null!", 0);
                return BT_STATUS_FAIL;
            }
            bt_ull_le_air_cis_request_ind_t *request = (bt_ull_le_air_cis_request_ind_t *)buff;
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
            ull_report("[ULL][LE][CONN][VENDOR EVT] air cis connect request evt, status: 0x%x, acl_handle: 0x%x, cis_handle: 0x%x, cig_id: 0x%x, cis_id: 0x%x", 5, \
                       ret,
                       request->acl_connection_handle,
                       request->cis_connection_handle,
                       request->cig_id,
                       request->cis_id
                      );
#endif
            bt_ull_le_conn_srv_air_cis_connect_request_handler(ret, request);
            break;
        }
    case BT_ULL_LE_AIR_CIS_ESTABLISHED_IND: {
            if (!buff) {
                ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_event_callback, buff is null!", 0);
                return BT_STATUS_FAIL;
            }

            bt_ull_le_air_cis_established_ind_t *established = (bt_ull_le_air_cis_established_ind_t *)buff;
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
            ull_report("[ULL][LE][CONN][VENDOR EVT] air cis establish complete evt, status: 0x%x, cis_handle: 0x%x, dl_active_state: 0x%x, ul_active_state: 0x%x, iso_intervel: 0x%x, nse: 0x%x, share_count: 0x%x", 7, \
                       ret,
                       established->cis_connection_handle,
                       established->dl_enable,
                       established->ul_enable,
                       established->iso_interval,
                       established->nse,
                       established->share_count
                      );
#endif

            bt_ull_le_conn_srv_air_cis_established_handler(ret, established);
            break;
        }
    case BT_ULL_LE_AIR_CIS_DISCONNECT_COMPLETE_IND: {
            if (!buff) {
                ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_event_callback, buff is null!", 0);
                return BT_STATUS_FAIL;
            }

            bt_ull_le_air_cis_disconnect_complete_ind_t *disconnect = (bt_ull_le_air_cis_disconnect_complete_ind_t *)buff;
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
            ull_report("[ULL][LE][CONN][VENDOR EVT] air cis disconnect complete evt, status: 0x%x, cis_handle: 0x%x, reason: 0x%x", 3, \
                       ret,
                       disconnect->cis_connection_handle,
                       disconnect->reason
                      );
#endif

            bt_ull_le_conn_srv_air_cis_destroied_handler(ret, disconnect);
            break;
        }
    case BT_ULL_LE_AIR_CIG_PARAMS_CHANGED_IND: {
            if (!buff) {
                ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_event_callback, buff is null!", 0);
                return BT_STATUS_FAIL;
            }

            bt_ull_le_air_cig_params_changed_ind_t *params_changed = (bt_ull_le_air_cig_params_changed_ind_t *)buff;
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
            ull_report("[ULL][LE][CONN][VENDOR EVT] air cig params change evt, status: 0x%x, latency: 0x%x, aud_quality: 0x%x", 3, \
                       ret,
                       params_changed->ft_label_index,
                       params_changed->label_index
                      );
#endif

            bt_ull_le_conn_srv_air_cig_params_changed_handler(ret, params_changed);
            break;
        }

    case BT_ULL_LE_AIR_CIS_UPLINK_ACTIVIATED_IND: {
            if (!buff) {
                ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_event_callback, buff is null!", 0);
                return BT_STATUS_FAIL;
            }

            bt_ull_le_air_cis_uplink_activiated_ind_t *ul_activiated = (bt_ull_le_air_cis_uplink_activiated_ind_t *)buff;
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
            ull_report("[ULL][LE][CONN][VENDOR EVT] air cis ul activiated evt, status: 0x%x, connection_handle: 0x%x, dl_enable: 0x%x, ul_enable: 0x%x", 4, \
                       ret,
                       ul_activiated->cis_connection_handle,
                       ul_activiated->dl_enable,
                       ul_activiated->ul_enable
                      );
#endif

            bt_ull_le_conn_srv_air_cis_uplink_activiated_handler(ret, ul_activiated);
            break;
        }

    case BT_ULL_LE_SET_AIR_CIG_PARAMS_CNF: {
            if (!buff) {
                ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_event_callback, buff is null!", 0);
                return BT_STATUS_FAIL;
            }

            bt_ull_le_set_air_cig_params_cnf_t *cig_cnf = (bt_ull_le_set_air_cig_params_cnf_t *)buff;
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
            ull_report("[ULL][LE][CONN][VENDOR EVT] set air cig parameter cmd complete evt, status: 0x%x, cig_id: 0x%x, cis_count: 0x%x, cis_handle: 0x%x", 4, \
                       ret,
                       cig_cnf->cig_id,
                       cig_cnf->cis_count,
                       cig_cnf->cis_connection_handle[0]
                      );
#endif

            bt_ull_le_conn_srv_set_air_cig_params_cnf_handler(ret, cig_cnf);
            break;
        }
    case BT_ULL_LE_CREATE_AIR_CIS_CNF: {
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
            ull_report("[ULL][LE][CONN][VENDOR EVT] create air cis cmd complete evt, status: 0x%x", 1, \
                       ret
                      );
#endif

            bt_ull_le_conn_srv_create_air_cis_cnf_handler(ret, buff);
            break;
        }
    case BT_ULL_LE_REMOVE_AIR_CIG_CNF: {
            if (!buff) {
                ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_event_callback, buff is null!", 0);
                return BT_STATUS_FAIL;
            }

            bt_ull_le_remove_air_cig_cnf_t *rm_cig_cnf = (bt_ull_le_remove_air_cig_cnf_t *)buff;
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
            ull_report("[ULL][LE][CONN][VENDOR EVT] remove air cig cmd complete evt, status: 0x%x, cig_id: 0x%x", 2, \
                       ret,
                       rm_cig_cnf->cig_id
                      );
#endif
            bt_ull_le_conn_srv_remove_air_cig_params_cnf_handler(ret, rm_cig_cnf);
            break;
        }
    case BT_ULL_LE_ACCEPT_AIR_CIS_CONNECT_REQUEST_CNF:
    case BT_ULL_LE_REJECT_AIR_CIS_CONNECT_REQUEST_CNF: {
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
            ull_report("[ULL][LE][CONN][VENDOR EVT] air cis connect request reply cmd complete evt, status: 0x%x", 1, \
                       ret
                      );
#endif

            bt_ull_le_conn_srv_reply_air_cis_request_cnf_handler(ret, buff);
            break;
        }
    case BT_ULL_LE_SETUP_AIR_ISO_DATA_PATH_CNF: {
            if (!buff) {
                ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_event_callback, buff is null!", 0);
                return BT_STATUS_FAIL;
            }

            bt_ull_le_setup_air_iso_data_path_cnf_t *iso_cnf = (bt_ull_le_setup_air_iso_data_path_cnf_t *)buff;
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
            ull_report("[ULL][LE][CONN][VENDOR EVT] set air iso data path cmd complete evt, status: 0x%x, cis_handle: 0x%x", 2, \
                       ret,
                       iso_cnf->handle
                      );
#endif

            bt_ull_le_conn_srv_set_air_iso_data_path_cnf_handler(ret, iso_cnf);
            break;
        }
    case BT_ULL_LE_REMOVE_AIR_ISO_DATA_PATH_CNF: {
            if (!buff) {
                ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_event_callback, buff is null!", 0);
                return BT_STATUS_FAIL;
            }

            bt_ull_le_setup_air_iso_data_path_cnf_t *iso_cnf = (bt_ull_le_setup_air_iso_data_path_cnf_t *)buff;
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
            ull_report("[ULL][LE][CONN][VENDOR EVT] remove air iso data path cmd complete evt, status: 0x%x, cis_handle: 0x%x", 2, \
                       ret,
                       iso_cnf->handle
                      );
#endif
            bt_ull_le_conn_srv_remove_air_iso_data_path_cnf_handler(ret, iso_cnf);
            break;
        }
    case BT_ULL_LE_DISCONNECT_AIR_CIS_CNF: {
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
            ull_report("[ULL][LE][CONN][VENDOR EVT] disconnect air cis connection cmd complete evt, status: 0x%x", 1, \
                       ret
                      );
#endif
            bt_ull_le_conn_srv_destroy_air_cis_cnf_handler(ret, buff);
            break;
        }
    case BT_ULL_LE_CHANGE_AIR_CIG_PARAMS_CNF: {
            bt_ull_le_air_change_air_params_cnf_t *chg_params_cnf = (bt_ull_le_air_change_air_params_cnf_t *)buff;
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
            ull_report("[ULL][LE][CONN][VENDOR EVT] change air cig params cmd complete evt, status: 0x%x", 1, \
                       ret
                      );
#endif
            bt_ull_le_conn_srv_change_air_cig_params_cnf_handler(ret, chg_params_cnf);
            break;
        }
    case BT_ULL_LE_UNMUTE_AIR_CIS_CNF: {
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
            ull_report("[ULL][LE][CONN][VENDOR EVT] unmute air cis cmd complete evt, status: 0x%x", 1, \
                       ret
                      );
#endif

            bt_ull_le_conn_srv_unmute_air_cis_cnf_handler(ret, buff);
            break;
        }
    case BT_ULL_LE_ACTIVATE_UPLINK_CNF: {
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
            ull_report("[ULL][LE][CONN][VENDOR EVT] activiate ul cmd complete evt, status: 0x%x", 1, \
                       ret
                      );
#endif
            bt_ull_le_conn_srv_activiate_uplink_cnf_handler(ret, buff);
            break;
        }
    case BT_ULL_LE_SET_AIR_CIG_PARAMS_TABLE_CNF: {
            bt_ull_le_air_set_air_cig_table_cnf_t *set_table = (bt_ull_le_air_set_air_cig_table_cnf_t*)buff;
            bt_ull_le_conn_srv_set_air_params_table_cnf_handler(ret, set_table);
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
            ull_report("[ULL][LE][CONN][VENDOR EVT] set air params table cmd complete evt, status: 0x%x", 1, \
                       ret);
#endif
            break;
        }
    case BT_ULL_LE_ENABLE_QOS_REPORT_CNF: {
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
            ull_report("[ULL][LE][CONN][VENDOR EVT] enable qos report, status: 0x%x", 1, \
                       ret);
#endif
            break;
        }
    case BT_ULL_LE_AIR_CIS_QOS_REPORT_IND: {
        if (!buff) {
            ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_event_callback(qos event), buff is null!", 0);
            return BT_STATUS_FAIL;
        }
            bt_ull_le_qos_report_ind_t *report = (bt_ull_le_qos_report_ind_t *)buff;
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
            ull_report("[ULL][LE][CONN][VENDOR EVT] air cis qos report, status: 0x%x, hdl: %x, crc: %d, rx_to: %d, ft: %d", 5, \
                       ret, report->acl_connection_handle, report->crc, report->rx_timeout, report->flush_timeout);
#endif
            bt_ull_le_conn_srv_qos_report_handler(ret, report);
            break;
        }
    default :
        break;

    }
    return BT_STATUS_SUCCESS;
}

static void bt_ull_le_conn_srv_qos_report_handler(bt_status_t status, bt_ull_le_qos_report_ind_t *report)
{
    if (!report) {
        return;
    }
    ull_report("[ULL][LE][CONN] QoS Report!!!, status: %x, handle: %d, crc: %d, rx to: %d, ft to: %d", 5, \
        status, report->acl_connection_handle, report->crc, report->rx_timeout, report->flush_timeout);
    if (BT_STATUS_SUCCESS != status) {
        return;
    }
    bt_ull_le_conn_srv_air_cis_evt_ind_t evt;
    evt.status = status;
    evt.qos_report.handle = report->acl_connection_handle;
    evt.qos_report.crc = report->crc;
    evt.qos_report.rx_to = report->rx_timeout;
    evt.qos_report.flush_to = report->flush_timeout;
    bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_QOS_REPORT_IND, &evt);
}

static void bt_ull_le_conn_srv_air_cis_connect_request_handler(bt_status_t status, bt_ull_le_air_cis_request_ind_t *ind)
{
    uint8_t accept = FALSE;
    uint8_t idx = bt_ull_le_conn_srv_get_avaliable_cis_idx();

    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_air_cis_connect_request_handler, idx: %d, cig_state: %d", 2, idx, bt_ull_le_conn_srv_get_cig_state());
    if (status != BT_STATUS_SUCCESS) {
        return;
    }
    if (idx == BT_ULL_LE_CONN_SRV_INVALID_IDX) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_air_cis_connect_request_handler, reject because of NO LINK RESOURCE!", 0);
        bt_ull_le_conn_srv_reject_air_cis_request_internal(ind->cis_connection_handle);
        return;
    }
    if (!bt_ull_le_am_is_allow_play()) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_air_cis_connect_request_handler, reject because of NO AUDIO RESOURCE!", 0);
        bt_ull_le_conn_srv_reject_air_cis_request_internal(ind->cis_connection_handle);
        return;
    }
    if (bt_ull_le_conn_srv_get_cig_state() == BT_ULL_LE_CONN_SRV_CIG_STATE_CREATED /*&& role = headset*/) {
        if (ind->cig_id == bt_ull_le_conn_srv_get_cig_id()) {
            accept = TRUE;
        } else {
            //todo
        }
    } else {
        accept = TRUE;
    }
#ifdef BT_ULL_LE_CONN_SRV_DEBUG
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_air_cis_connect_request_handler, reply: %d", 1, accept);
#endif
    if (accept) {
        bt_ull_le_conn_srv_set_acl_handle(idx, ind->acl_connection_handle);
        bt_ull_le_conn_srv_set_cis_handle(idx, ind->cis_connection_handle);
        bt_ull_le_conn_srv_set_cis_id(idx, ind->cis_id);
        bt_ull_le_conn_srv_set_cig_id(ind->cig_id);
        bt_ull_le_conn_srv_set_cig_state(BT_ULL_LE_CONN_SRV_CIG_STATE_CREATED);
        bt_ull_le_conn_srv_set_cis_state(idx, BT_ULL_LE_CONN_SRV_AIR_CIS_PASSIVE_CONNECTING);
        bt_ull_le_conn_srv_accept_air_cis_request_internal(ind->cis_connection_handle);
    } else {
        bt_ull_le_conn_srv_reject_air_cis_request_internal(ind->cis_connection_handle);
    }
}

static void bt_ull_le_conn_srv_air_cis_established_handler(bt_status_t status, bt_ull_le_air_cis_established_ind_t *ind)
{
    bt_ull_le_conn_srv_air_cis_evt_ind_t evt;
    uint8_t cis_state;
    uint8_t i;
    bool enable_streaming_now = false;
    bt_ull_client_t ct = bt_ull_le_conn_srv_get_client_type();
    uint8_t idx = bt_ull_le_conn_srv_get_idx_by_cis_handle(ind->cis_connection_handle);

    bt_ull_role_t role = bt_ull_le_conn_srv_get_role();
    if (idx == BT_ULL_LE_CONN_SRV_INVALID_IDX) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_air_cis_established_handler, invalid idx!", 0);
        return;
    }
    g_bt_ull_le_conn_ctrl.evt_mask.establish &= (~(1 << idx));
    cis_state = bt_ull_le_conn_srv_get_cis_state(idx);
    bt_ull_le_conn_srv_cache_cmd_node_t *activiate_uplink = bt_ull_le_conn_srv_search_cache_cmd_node_by_type(BT_ULL_LE_CONN_SRV_CMD_CHANGE_UL_PATH);    
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_air_cis_established_handler, status: %d, idx: %d, role: %d, cis_state: %d. cis_count: %d, evt_mask: 0x%x", 6, \
               status, idx, role, cis_state, bt_ull_le_conn_srv_get_cis_count(), g_bt_ull_le_conn_ctrl.evt_mask.establish);
    bt_handle_t acl_handle = bt_ull_le_conn_srv_get_acl_handle(idx);

    evt.established.handle = acl_handle;
    evt.established.dl_enable = ind->dl_enable;
    evt.established.ul_enable = ind->ul_enable;
    bt_ull_le_conn_srv_cache_cmd_node_t *remove_cig = bt_ull_le_conn_srv_search_cache_cmd_node_by_type(BT_ULL_LE_CONN_SRV_CMD_REMOVE_CIG);
    bt_ull_le_conn_srv_cache_cmd_node_t *change_label = bt_ull_le_conn_srv_search_cache_cmd_node_by_type(BT_ULL_LE_CONN_SRV_CMD_CHANGE_LABEL);

    if (status != BT_STATUS_SUCCESS || ind->status != BT_STATUS_SUCCESS) {
        /*notify ull service to retry or handle error*/
        evt.status = ind->status;
        bt_handle_t acl_handle = bt_ull_le_conn_srv_get_acl_handle(idx);
        if (cis_state == BT_ULL_LE_CONN_SRV_AIR_CIS_PASSIVE_CONNECTING && role == BT_ULL_ROLE_CLIENT) { /* client*/
            if (bt_ull_le_conn_srv_get_cis_count() == 0) {
                bt_ull_le_conn_srv_set_cig_id(BT_ULL_LE_CONN_SRV_AIR_CIG_ID_NONE);
                bt_ull_le_conn_srv_set_cig_state(BT_ULL_LE_CONN_SRV_CIG_STATE_NONE);
            }
            bt_ull_le_conn_srv_reset_cis_info(idx);
        } else if ((cis_state == BT_ULL_LE_CONN_SRV_AIR_CIS_ACTIVE_CONNECTING) && (role == BT_ULL_ROLE_SERVER)) { /*server: dongle*/
            bt_ull_le_conn_srv_set_acl_handle(idx, BT_HANDLE_INVALID);
            bt_ull_le_conn_srv_set_cis_state(idx, BT_ULL_LE_CONN_SRV_AIR_CIS_DISCONNECTED);
        } else if (BT_ULL_LE_CONN_SRV_AIR_CIS_DISCONNECTING == cis_state && BT_HCI_STATUS_OPERATION_CANCELLED_BY_HOST == ind->status) {
            //bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_ESTABLISHED_IND, &evt);
            ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_air_cis_established_handler, reason: %x", 1, ind->status);
        }
        if (BT_HCI_STATUS_CONNECTION_FAILED_TO_BE_ESTABLISHED == status && BT_ULL_HEADSET_CLIENT != ct && role == BT_ULL_ROLE_SERVER && !remove_cig) {
            ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_air_cis_established_handler, retry connect cis, handle: %d", 1, acl_handle);
            bt_ull_le_conn_srv_next_action_flow_handler(BT_ULL_LE_CONN_SRV_ACTION_ESTABLISH_AIR_CIS, &acl_handle);
        } else {
            /*slim code*/
            if (ct == BT_ULL_EARBUDS_CLIENT) {
                if (activiate_uplink) {
                    bt_ull_le_conn_srv_cache_cmd_node_t temp_node;
                    bt_ull_le_srv_memcpy(&temp_node, activiate_uplink, sizeof(bt_ull_le_conn_srv_cache_cmd_node_t));
                    if (acl_handle == temp_node.cache_item.handle) {
                        bt_ull_le_conn_srv_delete_cache_cmd_node(activiate_uplink);
                    }
                }
            }

            if (ct == BT_ULL_HEADSET_CLIENT) {
                if (bt_ull_le_conn_srv_get_connected_cis_num() != 0) { /* the first is establish success, but second is fail*/
                    if (BT_ULL_ROLE_SERVER == bt_ull_le_conn_srv_get_role()) {
                        for (i = 0; i < BT_ULL_LE_AIR_CIS_MAX_NUM; i ++) {
                            if (bt_ull_le_conn_srv_cis_is_connected(i)) {
                                bt_handle_t cis_handle = bt_ull_le_conn_srv_get_cis_handle(i);
                                bt_ull_le_conn_srv_next_action_flow_handler(BT_ULL_LE_CONN_SRV_ACTION_DESTROY_AIR_CIS, &cis_handle);
                            }
                        }
                    }
                }
                if (!bt_ull_le_conn_srv_get_evt_mask(BT_ULL_LE_CONN_SRV_ESTABLISHMENT_EVT_MASK)) {
            
                    bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_ESTABLISHED_IND, &evt);
                }
            } else {
            
                bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_ESTABLISHED_IND, &evt);
            }
        }
        if (remove_cig) {
            ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_air_cis_established_handler-2,Need Remove Air Cig!!", 1, 0);
            if (activiate_uplink) {
                bt_ull_le_conn_srv_delete_cache_cmd_node(activiate_uplink);
            }
            if (change_label) {
                bt_ull_le_conn_srv_delete_cache_cmd_node(change_label);
            }
            bt_ull_le_conn_srv_delete_cache_cmd_node(remove_cig);
            bt_ull_le_conn_srv_next_action_flow_handler(BT_ULL_LE_CONN_SRV_ACTION_REMOVE_AIR_CIG_PARAMS, NULL);
        }

        return;
    } else {
        evt.status = BT_STATUS_SUCCESS;
        if (cis_state == BT_ULL_LE_CONN_SRV_AIR_CIS_PASSIVE_CONNECTING && role == BT_ULL_ROLE_CLIENT) { /* client*/
            uint8_t cis_count = bt_ull_le_conn_srv_get_cis_count();
            cis_count ++;
            bt_ull_le_conn_srv_set_cis_count(cis_count);
        }
        bt_ull_le_conn_srv_set_cis_state(idx, BT_ULL_LE_CONN_SRV_AIR_CIS_CONNECTED);
        bt_ull_le_conn_srv_set_ul_active_state(idx, ind->ul_enable);
        bt_ull_le_conn_srv_set_dl_active_state(idx, ind->dl_enable);
        if (ct != BT_ULL_HEADSET_CLIENT) {
            enable_streaming_now = true;
            bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_ESTABLISHED_IND, &evt);
        } else {
            if (bt_ull_le_conn_srv_get_connected_cis_num() == 2 && !bt_ull_le_conn_srv_get_evt_mask(BT_ULL_LE_CONN_SRV_ESTABLISHMENT_EVT_MASK)) {
                for (i = 0; i < BT_ULL_LE_AIR_CIS_MAX_NUM; i ++) {
                    if (bt_ull_le_conn_srv_get_ul_active_state(i) == true) {
                        evt.established.ul_enable = true;
                        break;
                    }
                }
                enable_streaming_now = true;
                //ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_air_cis_established_handler -2, acl handle: %x, %x, dl enable: %d", 3, acl_handle, evt.established.handle, evt.established.dl_enable);
                bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_ESTABLISHED_IND, &evt);
            }
        }

    }
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_air_cis_established_handler,%x,%x, %x", 3, \
        remove_cig, activiate_uplink, enable_streaming_now);
    if (remove_cig) {
        ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_air_cis_established_handler,Need Remove Air Cig!!", 1, 0);
        if (activiate_uplink) {
            bt_ull_le_conn_srv_delete_cache_cmd_node(activiate_uplink);
        }
        bt_ull_le_conn_srv_delete_cache_cmd_node(remove_cig);
        bt_ull_le_conn_srv_next_action_flow_handler(BT_ULL_LE_CONN_SRV_ACTION_REMOVE_AIR_CIG_PARAMS, NULL);
        if (change_label) {
            bt_ull_le_conn_srv_delete_cache_cmd_node(change_label);
        }
        return;
    } else if (change_label) {
        bt_ull_le_conn_srv_delete_cache_cmd_node(change_label);
        if (BT_ULL_LE_CONN_SRV_CIG_STATE_CREATED == bt_ull_le_conn_srv_get_cig_state()) {
            uint8_t quality = bt_ull_le_conn_srv_get_audio_quality();
            uint8_t latency = bt_ull_le_conn_srv_get_latency();
            bt_ull_le_change_air_cig_params_t param;
            param.cig_id = bt_ull_le_conn_srv_get_cig_id();;
            param.cig_params_index = bt_ull_le_conn_srv_get_air_cig_tabel_idx(quality);
            param.ft_label_index = bt_ull_le_conn_srv_get_frame_duration_idx(latency);
            status = bt_ull_le_change_air_params(&param);
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
            ull_report("[ULL][LE][CONN][VENDOR CMD] CHANGE LABLE(change audio quality), cig id: %x, tabel idx: %x (quality: %d), latency: %d, status: 0x%x", 5, param.cig_id, param.cig_params_index, quality, param.ft_label_index, status);
#endif        
        }
    }
        
        if (ct == BT_ULL_EARBUDS_CLIENT) {
            if (activiate_uplink) {
                bt_ull_le_conn_srv_cache_cmd_node_t temp_node;
                bt_ull_le_srv_memcpy(&temp_node, activiate_uplink, sizeof(bt_ull_le_conn_srv_cache_cmd_node_t));
                if (acl_handle == temp_node.cache_item.handle) {
                    bt_ull_le_conn_srv_delete_cache_cmd_node(activiate_uplink);
                    bt_ull_le_conn_srv_next_action_flow_handler(BT_ULL_LE_CONN_SRV_ACTION_ENABLE_UPLINK, &temp_node.cache_item.handle);
                }
            }
    }

    if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
        /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: when aircis established, not set iso data path now*/
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_air_cis_established_handler, not set iso data path now!", 0);
    } else {
        if (status == BT_STATUS_SUCCESS) {
            if (((BT_ULL_ROLE_CLIENT == bt_ull_le_conn_srv_get_role() && bt_ull_le_am_is_allow_play()) \
                || BT_ULL_ROLE_SERVER == bt_ull_le_conn_srv_get_role()) && enable_streaming_now) {
                bt_handle_t cis_handle = BT_HANDLE_INVALID;
                if (BT_ULL_HEADSET_CLIENT == ct) {
                    uint8_t i = 0;
                    for (i = 0; i < BT_ULL_LE_AIR_CIS_MAX_NUM; i ++) {
                        cis_handle = bt_ull_le_conn_srv_get_cis_handle(i);
                        bt_ull_le_conn_srv_next_action_flow_handler(BT_ULL_LE_CONN_SRV_ACTION_SETUP_ISO_DATA_PATH, &cis_handle);
                    }
                } else {
                    cis_handle = ind->cis_connection_handle;
                    bt_ull_le_conn_srv_next_action_flow_handler(BT_ULL_LE_CONN_SRV_ACTION_SETUP_ISO_DATA_PATH, &cis_handle);
                }
            }
        }
    }
}

static void bt_ull_le_conn_srv_air_cis_destroied_handler(bt_status_t status, bt_ull_le_air_cis_disconnect_complete_ind_t *ind)
{
    bt_ull_le_conn_srv_air_cis_evt_ind_t evt;
    uint8_t cis_state;
    uint8_t role = bt_ull_le_conn_srv_get_role();
    uint8_t idx = bt_ull_le_conn_srv_get_idx_by_cis_handle(ind->cis_connection_handle);
    bt_ull_client_t ct = bt_ull_le_conn_srv_get_client_type();
    if (idx == BT_ULL_LE_CONN_SRV_INVALID_IDX) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_air_cis_destroied_handler, invalid idx!", 0);
        return;
    }
    cis_state = bt_ull_le_conn_srv_get_cis_state(idx);
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_air_cis_destroied_handler, status: %d, idx: %d, cis_state: %d, role: %d, cis_count: %d, evt_mask: 0x%x, ct: 0x%x", 7, \
               status, idx, cis_state, role, bt_ull_le_conn_srv_get_cis_count(), g_bt_ull_le_conn_ctrl.evt_mask.disconnect, ct);
    if (BT_ULL_LE_CONN_SRV_AIR_CIS_CONNECTED > cis_state) {
        return;
    }
    g_bt_ull_le_conn_ctrl.evt_mask.disconnect &= (~(1 << idx));

    if (status != BT_STATUS_SUCCESS || ind->status != BT_STATUS_SUCCESS) {
        if (bt_ull_le_conn_srv_get_connected_cis_num() == 0) {
            bt_ull_le_conn_srv_cache_cmd_node_t *remove_cig = bt_ull_le_conn_srv_search_cache_cmd_node_by_type(BT_ULL_LE_CONN_SRV_CMD_REMOVE_CIG);
            if (remove_cig) {
                bt_ull_le_conn_srv_delete_cache_cmd_node(remove_cig);
                bt_ull_le_conn_srv_next_action_flow_handler(BT_ULL_LE_CONN_SRV_ACTION_REMOVE_AIR_CIG_PARAMS, NULL);
            }
        }

        if (BT_ULL_HEADSET_CLIENT == ct) {
            if (!bt_ull_le_conn_srv_get_evt_mask(BT_ULL_LE_CONN_SRV_DISCONNECT_EVT_MASK)) {
                evt.status = BT_STATUS_FAIL;
                evt.destroied.handle = bt_ull_le_conn_srv_get_acl_handle(idx);
                bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_DESTROYED_IND, &evt);
                return;
            }
        } else {
            evt.status = BT_STATUS_FAIL;
            evt.destroied.handle = bt_ull_le_conn_srv_get_acl_handle(idx);
            evt.destroied.reason = ind->reason;
            bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_DESTROYED_IND, &evt);
        }
    } else {
        evt.status = BT_STATUS_SUCCESS;
        evt.destroied.handle = bt_ull_le_conn_srv_get_acl_handle(idx);
        evt.destroied.reason = ind->reason;
        if (role == BT_ULL_ROLE_SERVER) { /* client: dongle*/
            bt_ull_le_conn_srv_set_dl_active_state(idx, false);
            bt_ull_le_conn_srv_set_ul_active_state(idx, false);
            bt_ull_le_conn_srv_set_acl_handle(idx, BT_HANDLE_INVALID);
            bt_ull_le_conn_srv_set_cis_state(idx, BT_ULL_LE_CONN_SRV_AIR_CIS_DISCONNECTED);
            if (bt_ull_le_conn_srv_get_connected_cis_num() == 0) {
                bt_ull_le_conn_srv_cache_cmd_node_t *remove_cig = bt_ull_le_conn_srv_search_cache_cmd_node_by_type(BT_ULL_LE_CONN_SRV_CMD_REMOVE_CIG);
                if (remove_cig) {
                    bt_ull_le_conn_srv_delete_cache_cmd_node(remove_cig);
                    bt_ull_le_conn_srv_next_action_flow_handler(BT_ULL_LE_CONN_SRV_ACTION_REMOVE_AIR_CIG_PARAMS, NULL);
                }
            }
            if (BT_ULL_HEADSET_CLIENT == ct) {
                if (BT_HCI_STATUS_CONNECTION_TIMEOUT == ind->reason) {
                    ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_air_cis_destroied_handler, connection timeout!", 0);
                    uint8_t i =0;
                    for (i = 0;i < BT_ULL_LE_AIR_CIS_MAX_NUM; i ++) {
                        if (bt_ull_le_conn_srv_cis_is_connected(i)) {
                            bt_handle_t cis_handle = bt_ull_le_conn_srv_get_cis_handle(i);
                            bt_ull_le_conn_srv_next_action_flow_handler(BT_ULL_LE_CONN_SRV_ACTION_DESTROY_AIR_CIS, &cis_handle);
                        }
                    }
                }
                if (!bt_ull_le_conn_srv_get_evt_mask(BT_ULL_LE_CONN_SRV_DISCONNECT_EVT_MASK) && bt_ull_le_conn_srv_get_connected_cis_num() == 0) {
                    bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_DESTROYED_IND, &evt);
                }
            } else { /* ct is earbud*/
                bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_DESTROYED_IND, &evt);
            }
        } else { /*server: headset or earbud*/
            bt_ull_le_conn_srv_reset_cis_info(idx);
            uint8_t cis_count = bt_ull_le_conn_srv_get_cis_count();
            cis_count --;
            bt_ull_le_conn_srv_set_cis_count(cis_count);
            if (bt_ull_le_conn_srv_get_connected_cis_num() == 0) {
                bt_ull_le_conn_srv_set_cig_id(BT_ULL_LE_CONN_SRV_AIR_CIG_ID_NONE);
                bt_ull_le_conn_srv_set_cig_state(BT_ULL_LE_CONN_SRV_CIG_STATE_NONE);
                bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_DESTROYED_IND, &evt);
            } else {
                if (bt_ull_le_conn_srv_get_client_type() == BT_ULL_HEADSET_CLIENT) {
                    return;
                } else {
                    bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_DESTROYED_IND, &evt);
                }
            }
        }
    }
}

static void bt_ull_le_conn_srv_air_cig_params_changed_handler(bt_status_t status, bt_ull_le_air_cig_params_changed_ind_t *ind)
{
    ull_report("[ULL][LE][CONN] AIR CIG LABEL CHANGED, status: %x!, cig_idx: %d, latency_idx: %d", 3, \
        status, ind->label_index, ind->ft_label_index);
    if (status != BT_STATUS_SUCCESS) {
        return;
    }
    bt_ull_le_conn_srv_context_t *ctx = bt_ull_le_conn_srv_get_ctx();
    if (ind->ft_label_index >= BT_ULL_LE_FT_LABEL_MAX) {
        assert(0);
        return;
    }
    bt_ull_le_conn_srv_air_cis_evt_ind_t evt;
    evt.status = status;
    evt.label_changed.latency = g_bt_ull_latency_ft_table[ind->ft_label_index].latency;
#if defined(AIR_BT_ULL_FB_ENABLE)
    evt.label_changed.audio_quility = g_cig_param_table_lc3plus_highres[ind->label_index].audio_quality;
#else
    evt.label_changed.audio_quility = g_cig_param_table_lc3plus_defualt[ind->label_index].audio_quality;
#endif
    ctx->aud_ctrl.audio_quality = evt.label_changed.audio_quility;
    ctx->aud_ctrl.latency = evt.label_changed.latency;
    bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_LABEL_CHANGED_IND, &evt);
}

static void bt_ull_le_conn_srv_air_cis_uplink_activiated_handler(bt_status_t status, bt_ull_le_air_cis_uplink_activiated_ind_t *ind)
{
    uint8_t idx = bt_ull_le_conn_srv_get_idx_by_cis_handle(ind->cis_connection_handle);
    if (BT_ULL_HEADSET_CLIENT == bt_ull_le_conn_srv_get_client_type()) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_air_cis_uplink_activiated_handler, invalid client type!", 0);
        assert(0);
    }
    if (idx == BT_ULL_LE_CONN_SRV_INVALID_IDX) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_air_cis_uplink_activiated_handler, invalid idx!", 0);
        status = BT_STATUS_FAIL;
    }

    if (status != BT_STATUS_SUCCESS) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_air_cis_uplink_activiated_handler, status error: %x!", 1, status);
    } else {
        bt_ull_le_conn_srv_set_dl_active_state(idx, ind->dl_enable);
        bt_ull_le_conn_srv_set_ul_active_state(idx, ind->ul_enable);
        if (BT_ULL_ROLE_SERVER == bt_ull_le_conn_srv_get_role() && BT_ULL_EARBUDS_CLIENT == bt_ull_le_conn_srv_get_client_type()) {
            uint8_t i =0;
            for (i = 0; i < BT_ULL_LE_AIR_CIS_MAX_NUM; i ++ ) {
                if (bt_ull_le_conn_srv_get_cis_state(i) >= BT_ULL_LE_CONN_SRV_AIR_CIS_CONNECTED && \
                    i != idx) {
                    bt_ull_le_conn_srv_set_ul_active_state(i, !(ind->ul_enable));
                }
            }
        }
    }

    bt_ull_le_conn_srv_air_cis_evt_ind_t evt;
    evt.status = status;
    if (status == BT_STATUS_SUCCESS) {
        evt.ul_enabled.handle = bt_ull_le_conn_srv_get_acl_handle(idx);
        evt.ul_enabled.ul_enable = ind->ul_enable;
    }
    bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_UPLINK_ENABLED_IND, &evt);

}

static void bt_ull_le_conn_srv_set_air_cig_params_cnf_handler(bt_status_t status, bt_ull_le_set_air_cig_params_cnf_t *cnf)
{
    uint8_t i = 0;
    uint8_t cig_id = bt_ull_le_conn_srv_get_cig_id();
    uint8_t cig_state = bt_ull_le_conn_srv_get_cig_state();
    bt_ull_le_conn_srv_air_cis_evt_ind_t set_cig_msg;
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_set_air_cig_params_cnf_handler, status: 0x%x, cig_id: %d, cig_state: %d", 3, status, cig_id, cig_state);
    if (status != BT_STATUS_SUCCESS) {
        bt_ull_le_conn_srv_set_cig_id(BT_ULL_LE_CONN_SRV_AIR_CIG_ID_NONE);
        bt_ull_le_conn_srv_set_cig_state(BT_ULL_LE_CONN_SRV_CIG_STATE_NONE);
        for (i = 0; i < bt_ull_le_conn_srv_get_cis_count(); i ++) {
            bt_ull_le_conn_srv_reset_cis_info(i);
        }
        set_cig_msg.status = status;
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_set_air_cig_params_cnf_handler, status error: %x!", 1, status);
    } else {
        if (cig_id == cnf->cig_id && BT_ULL_LE_CONN_SRV_CIG_STATE_CREATING == cig_state) {
            if (cnf->cis_count > BT_ULL_LE_AIR_CIS_MAX_NUM) {
                ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_set_air_cig_params_cnf_handler, cis_count error!", 0);
                assert(0);
            }
            bt_ull_le_conn_srv_set_cig_state(BT_ULL_LE_CONN_SRV_CIG_STATE_CREATED);
            for (i = 0; i < cnf->cis_count; i ++) {
                bt_ull_le_conn_srv_set_cis_handle(i, cnf->cis_connection_handle[i]);
            }
            set_cig_msg.status = BT_STATUS_SUCCESS;
        } else {
            //todo
            bt_ull_le_conn_srv_set_cig_id(BT_ULL_LE_CONN_SRV_AIR_CIG_ID_NONE);
            for (i = 0; i < bt_ull_le_conn_srv_get_cis_count(); i ++) {
                bt_ull_le_conn_srv_reset_cis_info(i);
            }
            set_cig_msg.status = BT_STATUS_FAIL;
            ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_set_air_cig_params_cnf_handler, status error-2!", 0);
        }

    }

    bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_AIR_CIG_PARAMS_SET_DONE_IND, &set_cig_msg);

}

static void bt_ull_le_conn_srv_destroy_air_cis_cnf_handler(bt_status_t status, void *cnf)
{
    bt_ull_le_conn_srv_unlock_cmd(BT_ULL_LE_CONN_SRV_CMD_DISCONNECT);
    if (status != BT_STATUS_SUCCESS) {
        //todo
        bt_ull_le_conn_srv_cache_cmd_node_t *remove_cig = bt_ull_le_conn_srv_search_cache_cmd_node_by_type(BT_ULL_LE_CONN_SRV_CMD_REMOVE_CIG);
        if (remove_cig) {
            ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_destroy_air_cis_cnf_handler, Need Remove Air Cig!!", 1, 0);
            bt_ull_le_conn_srv_delete_cache_cmd_node(remove_cig);
            bt_ull_le_conn_srv_next_action_flow_handler(BT_ULL_LE_CONN_SRV_ACTION_REMOVE_AIR_CIG_PARAMS, NULL);
            return;
        }

        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_destroy_air_cis_cnf_handler, status error: %x!", 1, status);
    }
    bt_ull_le_conn_srv_cache_cmd_node_t *disconnect = bt_ull_le_conn_srv_search_cache_cmd_node_by_type(BT_ULL_LE_CONN_SRV_CMD_DISCONNECT);
    if (disconnect) {
        //bt_ull_le_conn_srv_destroy_air_cis_internal(disconnect->cmd.handle);
        bt_ull_le_conn_srv_cache_cmd_node_t temp_node;
        bt_ull_le_srv_memcpy(&temp_node, disconnect, sizeof(bt_ull_le_conn_srv_cache_cmd_node_t));
        bt_ull_le_conn_srv_delete_cache_cmd_node(disconnect);
        bt_ull_le_conn_srv_next_action_flow_handler(BT_ULL_LE_CONN_SRV_ACTION_DESTROY_AIR_CIS, &temp_node.cache_item.handle);
    }
}

static void bt_ull_le_conn_srv_create_air_cis_cnf_handler(bt_status_t status, void *cnf)
{
    uint8_t idx = 0;
    bt_ull_le_conn_srv_unlock_cmd(BT_ULL_LE_CONN_SRV_CMD_ESTABLISH);
    static uint8_t retry_count = 0x0;
    bt_ull_client_t ct = bt_ull_le_conn_srv_get_client_type();
    if (status != BT_STATUS_SUCCESS) {
        /* create cis fail */
        uint8_t connecting_cis = bt_ull_le_conn_srv_get_connecting_cis_num();
        uint8_t count = 0;
        bt_ull_le_conn_srv_clear_evt_mask(BT_ULL_LE_CONN_SRV_ESTABLISHMENT_EVT_MASK);
        for (idx = 0; idx < BT_ULL_LE_AIR_CIS_MAX_NUM; idx ++) {
            if (BT_ULL_LE_CONN_SRV_AIR_CIS_ACTIVE_CONNECTING == bt_ull_le_conn_srv_get_cis_state(idx)) {
                count ++;
                 ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_create_air_cis_cnf_handler, creating num: %d, count: %d", 2, connecting_cis, count);
                if (BT_ULL_HEADSET_CLIENT == ct) {
                    bt_ull_le_conn_srv_set_cis_state(idx, BT_ULL_LE_CONN_SRV_AIR_CIS_DISCONNECTED);
                } else {
                    if (count == connecting_cis) {
                        bt_ull_le_conn_srv_set_cis_state(idx, BT_ULL_LE_CONN_SRV_AIR_CIS_DISCONNECTED);
                        break;
                    }
                }
            }
        }
        bt_handle_t handle = BT_HANDLE_INVALID;
        if (BT_ULL_HEADSET_CLIENT == ct) {
            handle = bt_ull_le_conn_srv_get_acl_handle(1);
        } else {
            handle = bt_ull_le_conn_srv_get_acl_handle(idx);
        }

        if (BT_HCI_STATUS_COMMAND_DISALLOWED == status && retry_count < 0x3) {
            retry_count += 1;
            ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_create_air_cis_cnf_handler, retry connect: %d, handle: %d", 2, retry_count, handle);
            bt_ull_le_conn_srv_next_action_flow_handler(BT_ULL_LE_CONN_SRV_ACTION_ESTABLISH_AIR_CIS, &handle);
            return;
        } else {
            retry_count = 0x0;
            bt_ull_le_conn_srv_air_cis_evt_ind_t evt;
            evt.status = BT_STATUS_FAIL;
            evt.established.handle = handle;
            bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_ESTABLISHED_IND, &evt);
        }
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_create_air_cis_cnf_handler, status error: %x!", 1, status);
    } else {
        retry_count = 0x0;
    }
    bt_ull_le_conn_srv_cache_cmd_node_t *establish = bt_ull_le_conn_srv_search_cache_cmd_node_by_type(BT_ULL_LE_CONN_SRV_CMD_ESTABLISH);
    if (establish) {
        bt_ull_le_conn_srv_cache_cmd_node_t temp_node;
        bt_ull_le_srv_memcpy(&temp_node, establish, sizeof(bt_ull_le_conn_srv_cache_cmd_node_t));
        bt_ull_le_conn_srv_delete_cache_cmd_node(establish);
        bt_ull_le_conn_srv_next_action_flow_handler(BT_ULL_LE_CONN_SRV_ACTION_ESTABLISH_AIR_CIS, &temp_node.cache_item.handle);
    } else if (status != BT_STATUS_SUCCESS) {
        bt_ull_le_conn_srv_cache_cmd_node_t *remove_cig = bt_ull_le_conn_srv_search_cache_cmd_node_by_type(BT_ULL_LE_CONN_SRV_CMD_REMOVE_CIG);
        if (remove_cig) {
            ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_create_air_cis_cnf_handler, Need Remove Air Cig!!", 1, 0);
            bt_ull_le_conn_srv_delete_cache_cmd_node(remove_cig);
            bt_ull_le_conn_srv_next_action_flow_handler(BT_ULL_LE_CONN_SRV_ACTION_REMOVE_AIR_CIG_PARAMS, NULL);
            return;
        }

    }
}

static void bt_ull_le_conn_srv_remove_air_cig_params_cnf_handler(bt_status_t status, bt_ull_le_remove_air_cig_cnf_t *cnf)
{
    uint8_t i = 0;
    if (status != BT_STATUS_SUCCESS) {
        bt_ull_le_conn_srv_set_cig_state(BT_ULL_LE_CONN_SRV_CIG_STATE_CREATED);
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_remove_air_cig_params_cnf_handler, status error: %x!", 1, status);
        return;
    }
    bt_ull_le_conn_srv_set_cig_state(BT_ULL_LE_CONN_SRV_CIG_STATE_NONE);
    bt_ull_le_conn_srv_set_cig_id(BT_ULL_LE_CONN_SRV_AIR_CIG_ID_NONE);
    for (i = 0; i < BT_ULL_LE_AIR_CIS_MAX_NUM; i ++) {
        bt_ull_le_conn_srv_reset_cis_info(i);
    }
    bt_ull_le_conn_srv_air_cis_evt_ind_t cig_removed;
    cig_removed.status = status;
    bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_AIR_CIG_PARAMS_REMOVED_IND, &cig_removed);
    bt_ull_le_conn_srv_cache_cmd_node_t *change_label = bt_ull_le_conn_srv_search_cache_cmd_node_by_type(BT_ULL_LE_CONN_SRV_CMD_CHANGE_LABEL);
    if (change_label) {
        bt_ull_le_conn_srv_delete_cache_cmd_node(change_label);
    }
}

static void bt_ull_le_conn_srv_reply_air_cis_request_cnf_handler(bt_status_t status, void *cnf)
{
    if (status != BT_STATUS_SUCCESS) {
        //todo
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_reply_air_cis_request_cnf_handler, status error: %x!", 1, status);
    }
}

static void bt_ull_le_conn_srv_set_air_iso_data_path_cnf_handler(bt_status_t status, bt_ull_le_setup_air_iso_data_path_cnf_t *cnf)
{
    bt_ull_le_conn_srv_air_cis_evt_ind_t evt;
    uint8_t idx = bt_ull_le_conn_srv_get_idx_by_cis_handle(cnf->handle);
    bt_ull_client_t ct = bt_ull_le_conn_srv_get_client_type();
    bt_ull_le_conn_srv_unlock_cmd(BT_ULL_LE_CONN_SRV_CMD_SET_ISO_DATA_PATH);
    bt_ull_le_conn_srv_cache_cmd_node_t *set_iso = bt_ull_le_conn_srv_search_cache_cmd_node_by_type(BT_ULL_LE_CONN_SRV_CMD_SET_ISO_DATA_PATH);
    if (set_iso) {
        bt_ull_le_conn_srv_cache_cmd_node_t temp_node;
        bt_ull_le_srv_memcpy(&temp_node, set_iso, sizeof(bt_ull_le_conn_srv_cache_cmd_node_t));
        bt_ull_le_conn_srv_delete_cache_cmd_node(set_iso);
        bt_ull_le_conn_srv_next_action_flow_handler(BT_ULL_LE_CONN_SRV_ACTION_SETUP_ISO_DATA_PATH, &temp_node.cache_item.handle);
    }

    if (BT_ULL_LE_CONN_SRV_INVALID_IDX == idx) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_set_air_iso_data_path_cnf_handler, invalid idx!", 0);
        return;
    }
    uint8_t state = bt_ull_le_conn_srv_get_cis_state(idx);
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_set_air_iso_data_path_cnf_handler, status : %x!, state: %d", 2, status, state);
    if (BT_ULL_LE_CONN_SRV_AIR_CIS_SET_DATA_PATH != state) {
        return;
    }
    /*slim code*/
    evt.status = status;
    evt.cis_activiated.handle = bt_ull_le_conn_srv_get_acl_handle(idx);
    evt.cis_activiated.active_state = true;
    if (status != BT_STATUS_SUCCESS) {
        bt_ull_le_conn_srv_set_cis_state(idx, BT_ULL_LE_CONN_SRV_AIR_CIS_CONNECTED);
        bt_handle_t cis_handle = cnf->handle;
        bt_ull_le_conn_srv_next_action_flow_handler(BT_ULL_LE_CONN_SRV_ACTION_DESTROY_AIR_CIS, &cis_handle);
        bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_ACTIVATED_IND, &evt);
        return;
    } else {
        bt_ull_le_conn_srv_set_cis_state(idx, BT_ULL_LE_CONN_SRV_AIR_CIS_ACTIVIATED);
        if (ct == BT_ULL_HEADSET_CLIENT && bt_ull_le_conn_srv_get_streaming_cis_num() < 2) {
            ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_set_air_iso_data_path_cnf_handler, wait for the second streaming.", 0);
            return;
        }
        bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_ACTIVATED_IND, &evt);
    }

}

static void bt_ull_le_conn_srv_remove_air_iso_data_path_cnf_handler(bt_status_t status, bt_ull_le_remove_air_iso_data_path_cnf_t *cnf)
{
    bt_ull_le_conn_srv_unlock_cmd(BT_ULL_LE_CONN_SRV_CMD_REMOVE_ISO_DATA_PATH);
    bt_ull_client_t ct = bt_ull_le_conn_srv_get_client_type();
    bt_ull_le_conn_srv_cache_cmd_node_t *rm_iso = bt_ull_le_conn_srv_search_cache_cmd_node_by_type(BT_ULL_LE_CONN_SRV_CMD_REMOVE_ISO_DATA_PATH);
    if (rm_iso) {
        bt_ull_le_conn_srv_cache_cmd_node_t temp_node;
        bt_ull_le_srv_memcpy(&temp_node, rm_iso, sizeof(bt_ull_le_conn_srv_cache_cmd_node_t));
        bt_ull_le_conn_srv_delete_cache_cmd_node(rm_iso);
        bt_ull_le_conn_srv_next_action_flow_handler(BT_ULL_LE_CONN_SRV_ACTION_REMOVE_ISO_DATA_PATH, &temp_node.cache_item.handle);
    }

    uint8_t idx = bt_ull_le_conn_srv_get_idx_by_cis_handle(cnf->handle);
    bt_ull_le_conn_srv_air_cis_evt_ind_t evt;
    if (BT_ULL_LE_CONN_SRV_INVALID_IDX == idx) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_remove_air_iso_data_path_cnf_handler, invalid idx!", 0);
        bt_ull_le_conn_srv_set_cis_state(idx, BT_ULL_LE_CONN_SRV_AIR_CIS_ACTIVIATED);
        return;
    }
    evt.status = status;
    evt.cis_activiated.handle = bt_ull_le_conn_srv_get_acl_handle(idx);
    evt.cis_activiated.active_state = false;

    if (status != BT_STATUS_SUCCESS) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_remove_air_iso_data_path_cnf_handler, status error: %x!", 1, status);
        bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_ACTIVATED_IND, &evt);
        return;
    } else {
        bt_ull_le_conn_srv_set_cis_state(idx, BT_ULL_LE_CONN_SRV_AIR_CIS_CONNECTED);
        if (ct == BT_ULL_HEADSET_CLIENT && bt_ull_le_conn_srv_get_streaming_cis_num() > 0) {
            ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_remove_air_iso_data_path_cnf_handler, wait for the second remove streaming.", 0);
            return;
        }
        bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_ACTIVATED_IND, &evt);
    }
    


}

static void bt_ull_le_conn_srv_change_air_cig_params_cnf_handler(bt_status_t status, bt_ull_le_air_change_air_params_cnf_t *cnf)
{
    bt_ull_le_conn_srv_unlock_cmd(BT_ULL_LE_CONN_SRV_CMD_CHANGE_LABEL);
    bt_ull_le_conn_srv_context_t *ctx = bt_ull_le_conn_srv_get_ctx();
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_change_air_cig_params_cnf_handler, status: %x, aud_quality: %d, latency: %d", 3, \
        status, 
        ctx->aud_ctrl.select_cig_params[cnf->label_index].audio_quality,
        g_bt_ull_latency_ft_table[cnf->ft_label_index].latency);
    if (status != BT_STATUS_SUCCESS) {
        //bt_ull_le_conn_srv_air_cis_evt_ind_t latency_change;
        //latency_change.status = BT_STATUS_FAIL;
        //bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_LABEL_CHANGED_IND, &latency_change);
        if (BT_HCI_STATUS_UNSPECIFIED_ERROR == status) {
            bt_ull_le_conn_srv_cmd_cache_item_t *cache_cmd = bt_ull_le_conn_srv_new_cache_cmd_node(BT_ULL_LE_CONN_SRV_CMD_CHANGE_LABEL);
            cache_cmd->handle = 0xFF;
            ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_change_audio_quality, change label busy!", 0);
            return;
        }
    }
    if (cnf->cig_id != bt_ull_le_conn_srv_get_cig_id()) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_air_cig_params_changed_handler, cig id error!", 0);
        assert(0);
    }
    ctx->aud_ctrl.audio_quality = ctx->aud_ctrl.select_cig_params[cnf->label_index].audio_quality;
    ctx->aud_ctrl.latency = g_bt_ull_latency_ft_table[cnf->ft_label_index].latency;
    bt_ull_le_conn_srv_air_cis_evt_ind_t evt;
    evt.status = status;
    evt.label_changed.latency = g_bt_ull_latency_ft_table[cnf->ft_label_index].latency;
    evt.label_changed.audio_quility = ctx->aud_ctrl.select_cig_params[cnf->label_index].audio_quality;
    bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_LABEL_CHANGED_IND, &evt);

    bt_ull_le_conn_srv_cache_cmd_node_t *change_label = bt_ull_le_conn_srv_search_cache_cmd_node_by_type(BT_ULL_LE_CONN_SRV_CMD_CHANGE_LABEL);
    if (change_label) {
        bt_ull_le_conn_srv_delete_cache_cmd_node(change_label);
        uint8_t quality = bt_ull_le_conn_srv_get_audio_quality();
        uint8_t latency = bt_ull_le_conn_srv_get_latency();
        bt_ull_le_change_air_cig_params_t param = {0};
        if (BT_ULL_LE_CONN_SRV_CIG_STATE_CREATED == bt_ull_le_conn_srv_get_cig_state()) {
        param.cig_id = bt_ull_le_conn_srv_get_cig_id();;
        param.cig_params_index = bt_ull_le_conn_srv_get_air_cig_tabel_idx(quality);
        param.ft_label_index = bt_ull_le_conn_srv_get_frame_duration_idx(latency);
        status = bt_ull_le_change_air_params(&param);
        }
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
        ull_report("[ULL][LE][CONN][VENDOR CMD] CHANGE LABLE(change audio quality), cig id: %x, tabel idx: %x (quality: %d), latency: %d, status: 0x%x", 5, param.cig_id, param.cig_params_index, quality, param.ft_label_index, status);
#endif        
    }
}
static void bt_ull_le_conn_srv_unmute_air_cis_cnf_handler(bt_status_t status, void *cnf)
{
    bt_ull_le_conn_srv_unlock_cmd(BT_ULL_LE_CONN_SRV_CMD_UNMUTE);
    if (status != BT_STATUS_SUCCESS) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_unmute_air_cis_cnf_handler, status error: %x!", 1, status);
    }
    bt_ull_le_conn_srv_cache_cmd_node_t *unmute = bt_ull_le_conn_srv_search_cache_cmd_node_by_type(BT_ULL_LE_CONN_SRV_CMD_UNMUTE);
    if (unmute) {
        bt_ull_le_conn_srv_cache_cmd_node_t temp_node;
        bt_ull_le_srv_memcpy(&temp_node, unmute, sizeof(bt_ull_le_conn_srv_cache_cmd_node_t));
        bt_ull_le_conn_srv_delete_cache_cmd_node(unmute);
        bt_ull_le_conn_srv_next_action_flow_handler(BT_ULL_LE_CONN_SRV_ACTION_UNMUTE_AIR_CIS, &temp_node.cache_item.handle);
    }
}

static void bt_ull_le_conn_srv_activiate_uplink_cnf_handler(bt_status_t status, void *cnf)
{
    bt_ull_le_conn_srv_unlock_cmd(BT_ULL_LE_CONN_SRV_CMD_CHANGE_UL_PATH);
    if (status != BT_STATUS_SUCCESS) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_activiate_uplink_cnf_handler, status error: %x!", 1, status);
        bt_ull_le_conn_srv_air_cis_evt_ind_t mic_changed;
        mic_changed.status = BT_STATUS_FAIL;
        bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_UPLINK_ENABLED_IND, &mic_changed);
    }
    bt_ull_le_conn_srv_cache_cmd_node_t *activiate_uplink = bt_ull_le_conn_srv_search_cache_cmd_node_by_type(BT_ULL_LE_CONN_SRV_CMD_CHANGE_UL_PATH);
    if (activiate_uplink) {
        bt_ull_le_conn_srv_cache_cmd_node_t temp_node;
        bt_ull_le_srv_memcpy(&temp_node, activiate_uplink, sizeof(bt_ull_le_conn_srv_cache_cmd_node_t));
        bt_ull_le_conn_srv_delete_cache_cmd_node(activiate_uplink);
        bt_ull_le_conn_srv_next_action_flow_handler(BT_ULL_LE_CONN_SRV_ACTION_ENABLE_UPLINK, &temp_node.cache_item.handle);
    }
}

static void bt_ull_le_conn_srv_set_air_params_table_cnf_handler(bt_status_t status, void *cnf)
{
    bt_ull_le_conn_srv_unlock_cmd(BT_ULL_LE_CONN_SRV_CMD_SET_TABLE);
    bt_ull_le_conn_srv_air_cis_evt_ind_t air_cig_param_table_set = {0};
    bt_ull_le_air_set_air_cig_table_cnf_t *air_set_ind = (bt_ull_le_air_set_air_cig_table_cnf_t *)cnf;
    bt_ull_le_conn_srv_cache_cmd_node_t *set_tabel = bt_ull_le_conn_srv_search_cache_cmd_node_by_type(BT_ULL_LE_CONN_SRV_CMD_SET_TABLE);
    if (status != BT_STATUS_SUCCESS) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_set_air_params_table_cnf_handler, status error: %x!", 1, status);
    }
    air_cig_param_table_set.status = (BT_STATUS_SUCCESS == status) ? BT_STATUS_SUCCESS : BT_STATUS_FAIL;
    if (air_set_ind) {
        air_cig_param_table_set.cig_set.handle = air_set_ind->handle;
    }
    if (set_tabel) {
        bt_ull_le_conn_srv_cache_cmd_node_t temp_node;
        bt_ull_le_srv_memcpy(&temp_node, set_tabel, sizeof(bt_ull_le_conn_srv_cache_cmd_node_t));
        bt_ull_le_conn_srv_delete_cache_cmd_node(set_tabel);
        bt_ull_le_conn_srv_set_air_cig_params_table(set_tabel->cache_item.handle, bt_ull_le_conn_srv_get_cis_count(),bt_ull_le_conn_srv_get_codec_type());
    }
    bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_AIR_CIG_TABLE_SET_IND, &air_cig_param_table_set);
}

static void bt_ull_le_conn_srv_notify(bt_ull_le_conn_srv_event_t event, void *msg)
{
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_notify, event: 0x%x", 1, event);
    if (g_bt_ull_le_conn_ctrl.cb) {
        g_bt_ull_le_conn_ctrl.cb(event, msg);
    } else {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_notify error, event: 0x%x", 1, event);
    }
}

static bt_status_t bt_ull_le_conn_srv_remove_air_iso_data_path_internal(bt_handle_t cis_handle)
{
    bt_status_t status;
    uint8_t idx = bt_ull_le_conn_srv_get_idx_by_cis_handle(cis_handle);
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_remove_air_iso_data_path_internal, idx: 0x%x", 1, idx);
    if (BT_ULL_LE_CONN_SRV_INVALID_IDX == idx) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_remove_air_iso_data_path_internal, invalid handle!", 0);
        return BT_STATUS_FAIL;
    }
    uint8_t state = bt_ull_le_conn_srv_get_cis_state(idx);
    if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
        /*BT_ULL_LE_KEEP_CIS_ALWAYS_ALIVE: Add the inactive ISO state */
        ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_setup_air_iso_data_path_internal, idx: 0x%x, state: %d", 2, idx, state);
        if (BT_ULL_LE_CONN_SRV_AIR_CIS_RM_DATA_PATH == state) {
            return BT_STATUS_SUCCESS;
        }
    
        if (BT_ULL_LE_CONN_SRV_AIR_CIS_ACTIVIATED != state) {
            ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_setup_air_iso_data_path_internal, state error: %d!", 1, bt_ull_le_conn_srv_get_cis_state(idx));
            return BT_STATUS_FAIL;
        }        
    }
    if (bt_ull_le_conn_srv_cmd_is_lock(BT_ULL_LE_CONN_SRV_CMD_REMOVE_ISO_DATA_PATH)) {
        bt_ull_le_conn_srv_cmd_cache_item_t *cache_cmd = bt_ull_le_conn_srv_new_cache_cmd_node(BT_ULL_LE_CONN_SRV_CMD_REMOVE_ISO_DATA_PATH);
        cache_cmd->handle = cis_handle;
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_remove_air_iso_data_path_internal, remove air iso busy!", 0);
        return BT_STATUS_SUCCESS;
    }
    bt_ull_le_conn_srv_lock_cmd(BT_ULL_LE_CONN_SRV_CMD_REMOVE_ISO_DATA_PATH, cis_handle);
    if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
        bt_ull_le_conn_srv_set_cis_state(idx, BT_ULL_LE_CONN_SRV_AIR_CIS_RM_DATA_PATH);
    }
    bt_ull_le_remove_air_iso_data_path_t param;
    param.handle = cis_handle;
    param.data_path_direction = 0x03; /*bit 0: input, bit 1: uotput*/
    status = bt_ull_le_remove_air_iso_data_path(&param);
    if (BT_STATUS_SUCCESS != status) {
        bt_ull_le_conn_srv_unlock_cmd(BT_ULL_LE_CONN_SRV_CMD_REMOVE_ISO_DATA_PATH);
        if (bt_ull_le_srv_check_inactive_aircis_feature_on()) {
            bt_ull_le_conn_srv_set_cis_state(idx, state);
        }
    }
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
    ull_report("[ULL][LE][CONN][VENDOR CMD] vendor remove air iso data path cmd, status: 0x%x, connection handle: 0x%x, direction: 0x%x", 3, \
               status, param.handle, param.data_path_direction);
#endif
    return status;
}

static bt_status_t bt_ull_le_conn_srv_setup_air_iso_data_path_internal(bt_handle_t cis_handle)
{
    bt_status_t status;
    uint8_t idx = 0;
    uint8_t data_path_id;
    bt_ull_role_t role = bt_ull_le_conn_srv_get_role();
    bt_ull_client_t ct = bt_ull_le_conn_srv_get_client_type();
    idx = bt_ull_le_conn_srv_get_idx_by_cis_handle(cis_handle);
    if (BT_ULL_LE_CONN_SRV_INVALID_IDX == idx) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_setup_air_iso_data_path_internal, invalid idx!", 0);
        return BT_STATUS_FAIL;
    }
    uint8_t state = bt_ull_le_conn_srv_get_cis_state(idx);
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_setup_air_iso_data_path_internal, idx: 0x%x, state: %d", 2, idx, state);
    if (BT_ULL_LE_CONN_SRV_AIR_CIS_CONNECTED > state \
        || BT_ULL_LE_CONN_SRV_AIR_CIS_DISCONNECTING == state) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_setup_air_iso_data_path_internal, state error: %d!", 1, bt_ull_le_conn_srv_get_cis_state(idx));
        return BT_STATUS_FAIL;
    }
    if (BT_ULL_LE_CONN_SRV_AIR_CIS_SET_DATA_PATH == state \
        || BT_ULL_LE_CONN_SRV_AIR_CIS_ACTIVIATED == state) {
        return BT_STATUS_SUCCESS;
    }
#if 0
    if (BT_ULL_ROLE_CLIENT == bt_ull_le_conn_srv_get_role() \
        && ((BT_ULL_HEADSET_CLIENT == bt_ull_le_conn_srv_get_client_type() && idx == 0x0) \
        || (BT_ULL_EARBUDS_CLIENT == bt_ull_le_conn_srv_get_client_type()))) {
        ull_report("[ULL][LE][CONN] Reset AVM buffer, idx: 0x%x", 1, idx);
        //bt_sink_srv_cap_stream_reset_avm_buffer();
    }
#endif

    if (BT_ULL_ROLE_CLIENT == role) {
        if ((BT_ULL_HEADSET_CLIENT == ct && idx != 0x0) \
            || BT_ULL_MIC_CLIENT == ct \
            || BT_ULL_ULD_MIC_CLIENT == ct) {
            // not reset avm buffer.
        } else {
            if (BT_STATUS_SUCCESS != bt_ull_le_srv_set_avm_share_buffer(role, ct, bt_ull_le_conn_srv_get_cis_count())) {
                ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_setup_air_iso_data_path_internal, set avm buffer error!", 0);
            }
        }
    }

    if (BT_ULL_ROLE_SERVER == role \
        || BT_ULL_HEADSET_CLIENT == ct \
        || BT_ULL_SPEAKER_CLIENT == ct) {
        data_path_id = bt_ull_le_conn_srv_get_data_path_id(bt_ull_le_conn_srv_get_cis_id(idx), ct);
    } else {
        data_path_id = idx + 1;
    }

    if (BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE == data_path_id) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_setup_air_iso_data_path_internal, data path id error!", 0);
        return  BT_STATUS_FAIL;
    }
    if (bt_ull_le_conn_srv_cmd_is_lock(BT_ULL_LE_CONN_SRV_CMD_SET_ISO_DATA_PATH)) {
        bt_ull_le_conn_srv_cmd_cache_item_t *cache_cmd = bt_ull_le_conn_srv_new_cache_cmd_node(BT_ULL_LE_CONN_SRV_CMD_SET_ISO_DATA_PATH);
        cache_cmd->handle = cis_handle;
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_setup_air_iso_data_path_internal, set up iso data path busy!", 0);
        return BT_STATUS_SUCCESS;
    }

    bt_ull_le_conn_srv_set_cis_state(idx, BT_ULL_LE_CONN_SRV_AIR_CIS_SET_DATA_PATH);
    bt_ull_le_conn_srv_lock_cmd(BT_ULL_LE_CONN_SRV_CMD_SET_ISO_DATA_PATH, cis_handle);
    bt_ull_le_setup_air_iso_data_path_t param;
    bt_ull_le_srv_memset(&param, 0, sizeof(bt_ull_le_setup_air_iso_data_path_t));
    /*slim code*/
    param.cis_connection_handle = cis_handle;
    param.data_path_id = data_path_id;

    status = bt_ull_le_setup_air_iso_data_path(&param);
    if (BT_STATUS_SUCCESS != status) {
        bt_ull_le_conn_srv_unlock_cmd(BT_ULL_LE_CONN_SRV_CMD_SET_ISO_DATA_PATH);
    }
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
    ull_report("[ULL][LE][CONN][VENDOR CMD] vendor setup air iso data path cmd, status: 0x%x, connection handle: 0x%x, direction: 0x%x, data path id: 0x%x, codec format: 0x%x, company id: 0x%x, vendor codec id: 0x%x, controller delay: 0x%x", 7, \
               status,
               param.cis_connection_handle,
               param.direction,
               param.data_path_id,
               param.codec_format,
               param.company_id,
               param.vendor_codec_id,
               param.controller_delay
              );
#endif

    return status;
}

static bt_status_t bt_ull_le_conn_srv_accept_air_cis_request_internal(bt_handle_t cis_handle)
{
    bt_status_t status;
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_accept_air_cis_request_internal, cis handle: 0x%x", 1, cis_handle);
    if (BT_HANDLE_INVALID == cis_handle) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_accept_air_cis_request_internal, invalid handle!", 0);
        return BT_STATUS_FAIL;
    }

    bt_ull_le_reply_air_cis_request_t param;
    param.action = BT_ULL_LE_REPLY_AIR_CIS_ACTION_ACCEPT;
    param.accept.handle = cis_handle;
    status = bt_ull_le_reply_air_cis_connect_request(&param);
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
    ull_report("[ULL][LE][CONN][VENDOR CMD] vendor accpet air cis cmd, status: 0x%x, action: 0x%x, connction handle: 0x%x", 3, \
               status,
               param.action,
               param.accept.handle
              );
#endif

    return status;
}

static bt_status_t bt_ull_le_conn_srv_reject_air_cis_request_internal(bt_handle_t cis_handle)
{
    bt_status_t status;
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_reject_air_cis_request_internal, cis handle: 0x%x", 1, cis_handle);
    if (BT_HANDLE_INVALID == cis_handle) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_reject_air_cis_request_internal, invalid handle!", 0);
        return BT_STATUS_FAIL;
    }

    bt_ull_le_reply_air_cis_request_t param;
    param.action = BT_ULL_LE_REPLY_AIR_CIS_ACTION_REJECT;
    param.reject.handle = cis_handle;
    param.reject.reason = BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES;
    status = bt_ull_le_reply_air_cis_connect_request(&param);
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
    ull_report("[ULL][LE][CONN][VENDOR CMD] vendor reject air cis cmd, status: 0x%x, action: 0x%x, connction handle: 0x%x", 3, \
               status,
               param.action,
               param.accept.handle
              );
#endif
    return status;
}

bt_status_t bt_ull_le_conn_srv_set_air_cig_params(uint8_t cis_count)
{
    uint8_t count = 0;
    bt_status_t status = BT_STATUS_SUCCESS;

    bt_ull_le_srv_latency_t latency = bt_ull_le_conn_srv_get_latency();
    bt_ull_le_srv_audio_quality_t quality = bt_ull_le_conn_srv_get_audio_quality();
    bt_ull_client_t ct = bt_ull_le_conn_srv_get_client_type();
    uint8_t cig_state = bt_ull_le_conn_srv_get_cig_state();

    bt_ull_le_conn_srv_air_cig_param_t *cig_params = bt_ull_le_conn_srv_get_air_cig_params(quality);
    bt_ull_le_conn_srv_frame_duration_param_t *frame_duration = bt_ull_le_conn_srv_get_frame_duration_params(latency);
    bt_ull_le_codec_t codec = bt_ull_le_conn_srv_get_codec_type(); 
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_set_air_cig_params, cis_count: 0x%x, cig_state: 0x%x, ct: 0x%x, quality: 0x%x, codec: %d, latency: %d", 6, \
            cis_count,
            cig_state,
            ct,
            quality,
            codec,
            latency);
    if (cis_count != bt_ull_le_conn_srv_get_cis_count() || !cig_params || !frame_duration) {
        assert(0);
    }

    if (BT_ULL_LE_CONN_SRV_CIG_STATE_CREATED == cig_state) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_set_air_cig_params, cig has created!", 0);
        bt_ull_le_conn_srv_air_cis_evt_ind_t evt;
        evt.status = BT_STATUS_SUCCESS;
        bt_ull_le_conn_srv_notify(BT_ULL_LE_CONN_SRV_EVENT_AIR_CIG_PARAMS_SET_DONE_IND, &evt);
        return BT_STATUS_SUCCESS;
    }
    if (!cis_count || BT_ULL_ROLE_SERVER != bt_ull_le_conn_srv_get_role()) {
        ull_assert(0 && "[ULL][LE][CONN] invalid role!");
    }
    uint32_t cis_list_len = sizeof(bt_ull_le_air_cis_params_t) * cis_count;
    bt_ull_le_air_cis_params_t *cis_list = bt_ull_le_srv_memory_alloc(cis_list_len);
    if (!cis_list) {
        ull_assert(0 && "[ULL][LE][CONN] out of memory!");
        return BT_STATUS_OUT_OF_MEMORY;
    } else {
        bt_ull_le_srv_memset(cis_list, 0, cis_list_len);
    }
    bt_ull_le_conn_srv_set_phy(cig_params->phy_m_2_s);
    uint32_t m2s_sdu_interval = bt_ull_le_srv_get_sdu_interval(false, bt_ull_le_conn_srv_get_role());
    uint32_t s2m_sdu_interval = bt_ull_le_srv_get_sdu_interval(true, bt_ull_le_conn_srv_get_role());
    switch (ct)
    {
        case BT_ULL_SPEAKER_CLIENT: {
            for (count = 0; count < cis_count; count ++) {
                bt_ull_le_conn_srv_set_cis_id(count, g_bt_ull_spk_cis_id_table[count].cis_id);
                cis_list[count].cis_id = g_bt_ull_spk_cis_id_table[count].cis_id;
                cis_list[count].max_sdu_m_to_s =((cig_params->dl_bitrate / 100) * (m2s_sdu_interval / 10)) / 8 / 1000 / 2;
                cis_list[count].max_sdu_s_to_m = 0x0000;
                cis_list[count].phy_m_to_s = cig_params->phy_m_2_s;
                cis_list[count].phy_s_to_m = cig_params->phy_s_2_m;
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
                ull_report("[ULL][LE][CONN][VENDOR CMD] vendor set air cig cmd-cis_list[%d], cis_id: %d, max_sdu_m_to_s: 0x%x, max_sdu_s_to_m: 0x%x, phy_m_to_s: 0x%x, phy_s_to_m: 0x%x", 6, \
                           count,
                           cis_list[count].cis_id,
                           cis_list[count].max_sdu_m_to_s,
                           cis_list[count].max_sdu_s_to_m,
                           cis_list[count].phy_m_to_s,
                           cis_list[count].phy_s_to_m
                          );
#endif

            }

            break;
        }
        case BT_ULL_HEADSET_CLIENT: {
            for (count = 0; count < cis_count; count ++) {
                bt_ull_le_conn_srv_set_cis_id(count, g_bt_ull_cis_id_table[count].cis_id);
                cis_list[count].cis_id = g_bt_ull_cis_id_table[count].cis_id;
                cis_list[count].max_sdu_m_to_s = ((cig_params->dl_bitrate / 100)* (m2s_sdu_interval / 10)) / 8 / 1000 / 2;
                if (cis_list[count].cis_id == BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK2) {
                    cis_list[count].max_sdu_s_to_m = 0x0000;
                } else {
                    /*for AB156x ULL 2.0  uplink AINR Feature.*/
                    cis_list[count].max_sdu_s_to_m = ((cig_params->ul_bitrate / 100)* (s2m_sdu_interval / 10)) / 8 / 1000;
                }
                cis_list[count].phy_m_to_s = cig_params->phy_m_2_s;
                cis_list[count].phy_s_to_m = cig_params->phy_s_2_m;
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
                ull_report("[ULL][LE][CONN][VENDOR CMD] vendor set air cig cmd-cis_list[%d], cis_id: %d, max_sdu_m_to_s: 0x%x, max_sdu_s_to_m: 0x%x, phy_m_to_s: 0x%x, phy_s_to_m: 0x%x", 6, \
                           count,
                           cis_list[count].cis_id,
                           cis_list[count].max_sdu_m_to_s,
                           cis_list[count].max_sdu_s_to_m,
                           cis_list[count].phy_m_to_s,
                           cis_list[count].phy_s_to_m
                          );
#endif

            }
            break;
        }
        case BT_ULL_EARBUDS_CLIENT: {
            for (count = 0; count < cis_count; count ++) {
                bt_ull_le_conn_srv_set_cis_id(count, g_bt_ull_cis_id_table[count].cis_id);
                cis_list[count].cis_id = g_bt_ull_cis_id_table[count].cis_id;
                cis_list[count].max_sdu_m_to_s = ((cig_params->dl_bitrate / 100)* (m2s_sdu_interval / 10)) / 8 / 1000 / 2;
                /*for AB156x ULL 2.0  uplink AINR Feature.*/
                cis_list[count].max_sdu_s_to_m = ((cig_params->ul_bitrate / 100) * (s2m_sdu_interval / 10)) / 8 / 1000;
                cis_list[count].phy_m_to_s = cig_params->phy_m_2_s;
                cis_list[count].phy_s_to_m = cig_params->phy_s_2_m;
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
                ull_report("[ULL][LE][CONN][VENDOR CMD] vendor set air cig cmd-cis_list[%d], cis_id: %d, max_sdu_m_to_s: 0x%x, max_sdu_s_to_m: 0x%x, phy_m_to_s: 0x%x, phy_s_to_m: 0x%x", 6, \
                           count,
                           cis_list[count].cis_id,
                           cis_list[count].max_sdu_m_to_s,
                           cis_list[count].max_sdu_s_to_m,
                           cis_list[count].phy_m_to_s,
                           cis_list[count].phy_s_to_m
                          );
#endif
            }
            break;
        }
        case BT_ULL_MIC_CLIENT: {
            for (count = 0; count < cis_count; count ++) {
                bt_ull_le_conn_srv_set_cis_id(count, g_bt_ull_mic_cis_id_table[count].cis_id);
                cis_list[count].cis_id = g_bt_ull_mic_cis_id_table[count].cis_id;
                cis_list[count].max_sdu_m_to_s = 0x0000;
                cis_list[count].max_sdu_s_to_m = ((cig_params->ul_bitrate / 100) * (s2m_sdu_interval / 10)) / 8 / 1000;
                cis_list[count].phy_m_to_s = cig_params->phy_m_2_s;
                cis_list[count].phy_s_to_m = cig_params->phy_s_2_m;
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
                ull_report("[ULL][LE][CONN][VENDOR CMD] vendor set air cig cmd-cis_list[%d], cis_id: %d, max_sdu_m_to_s: 0x%x, max_sdu_s_to_m: 0x%x, phy_m_to_s: 0x%x, phy_s_to_m: 0x%x", 6, \
                           count,
                           cis_list[count].cis_id,
                           cis_list[count].max_sdu_m_to_s,
                           cis_list[count].max_sdu_s_to_m,
                           cis_list[count].phy_m_to_s,
                           cis_list[count].phy_s_to_m
                          );
#endif
            }
            break;
        }
        default: {
            ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_set_air_cig_params, unknown client type!", 0);
            bt_ull_le_srv_memory_free(cis_list);
            return BT_STATUS_FAIL;
        }

    }

    bt_ull_le_air_cig_params_t params;
    bt_ull_le_conn_srv_set_cig_id(BT_ULL_LE_CONN_SRV_AIR_CIG_ID_1);
    params.cig_id = BT_ULL_LE_CONN_SRV_AIR_CIG_ID_1;
    if (BT_ULL_MIC_CLIENT == ct && BT_ULL_LE_CODEC_ULD == cig_params->codec) {
        params.client_type = BT_ULL_ULD_MIC_CLIENT;
    } else {
        params.client_type = ct;
    }

    params.default_label_index = bt_ull_le_conn_srv_get_air_cig_tabel_idx(quality);
    switch (params.default_label_index) {
        case 0: {
            ull_report("[ULL][LE][CONN] AUDIO QUALITY TYPE DEFAULT is using", 0);
            break;
        }
        case 1: {
            ull_report("[ULL][LE][CONN] AUDIO QUALITY TWO STREAMING TYPE is using", 0);
            break;
        }
        case 2: {
            ull_report("[ULL][LE][CONN] AUDIO QUALITY HIGH RES 4M TYPE is using", 0);
            break;
        }
        case 3: {
            ull_report("[ULL][LE][CONN] AUDIO QUALITY HIGH RES 8M TYPE is using", 0);
            break;
        }
    }
    params.default_ft_index = bt_ull_le_conn_srv_get_frame_duration_idx(latency);
    params.max_share_count = cig_params->max_share_num;
    params.iso_interval = cig_params->iso_interval;
    params.sdu_interval_m_to_s = m2s_sdu_interval;
    params.sdu_interval_s_to_m = s2m_sdu_interval;
    params.sca = 0x00;
    if (BT_ULL_SPEAKER_CLIENT == ct) {
        params.max_uplink_num = 0x00;
    } else if (BT_ULL_MIC_CLIENT == ct || BT_ULL_ULD_MIC_CLIENT == ct) {
         params.max_uplink_num = cis_count;
    } else {
         params.max_uplink_num = cig_params->max_ul_num;
    }
    params.cis_count = cis_count;
    params.cis_list = cis_list;
    status = bt_ull_le_set_air_cig_parameters(&params);
    if (BT_STATUS_SUCCESS == status) {
        bt_ull_le_conn_srv_set_cig_state(BT_ULL_LE_CONN_SRV_CIG_STATE_CREATING);
    }
    bt_ull_le_srv_memory_free(cis_list);
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
    ull_report("[ULL][LE][CONN][VENDOR CMD] vendor set air cig cmd, status: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x", 12, \
                status,
                params.cig_id,
                params.client_type,
                params.default_label_index,
                params.default_ft_index,
                params.sdu_interval_m_to_s,
                params.sdu_interval_s_to_m,
                params.max_share_count,
                params.iso_interval,
                params.max_uplink_num,
                params.sca,
                params.cis_count
              );
#endif

    return status;
}

bt_status_t bt_ull_le_conn_srv_remove_air_cig_params(void)
{
    uint8_t i = 0;
    bt_status_t status = BT_STATUS_SUCCESS;
    uint8_t cig_id = bt_ull_le_conn_srv_get_cig_id();
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_remove_air_cig_params, cis id: 0x%x", 1, cig_id);
    if (cig_id == BT_ULL_LE_CONN_SRV_AIR_CIG_ID_NONE || BT_ULL_LE_CONN_SRV_CIG_STATE_CREATED != bt_ull_le_conn_srv_get_cig_state()) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_remove_air_cig_params, cig id error!", 0);
        return BT_STATUS_FAIL;
    }

    if (bt_ull_le_conn_srv_get_role() != BT_ULL_ROLE_SERVER) {
        return BT_STATUS_FAIL;
    }

    if (bt_ull_le_conn_srv_get_connected_cis_num() != 0x00) {
        if (!bt_ull_le_conn_srv_search_cache_cmd_node_by_type(BT_ULL_LE_CONN_SRV_CMD_REMOVE_CIG)) {
            bt_ull_le_conn_srv_new_cache_cmd_node(BT_ULL_LE_CONN_SRV_CMD_REMOVE_CIG);
        }
        for (i = 0; i < BT_ULL_LE_AIR_CIS_MAX_NUM; i ++) {
            if (bt_ull_le_conn_srv_cis_is_connected(i)) {
                ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_remove_air_cig_params, link: 0x%x is connected, state: %d", 2, i, g_bt_ull_le_conn_ctrl.cis_info[i].state);
                bt_handle_t cis_handle = bt_ull_le_conn_srv_get_cis_handle(i);
                status = bt_ull_le_conn_srv_next_action_flow_handler(BT_ULL_LE_CONN_SRV_ACTION_DESTROY_AIR_CIS, &cis_handle);
            }
        }
        return BT_STATUS_PENDING;
    }
    if (bt_ull_le_conn_srv_get_connecting_cis_num() != 0x00) {
        ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_remove_air_cig_params, pending!!", 0);
        if (!bt_ull_le_conn_srv_search_cache_cmd_node_by_type(BT_ULL_LE_CONN_SRV_CMD_REMOVE_CIG)) {
            bt_ull_le_conn_srv_new_cache_cmd_node(BT_ULL_LE_CONN_SRV_CMD_REMOVE_CIG);
        }
        return BT_STATUS_PENDING;
    }
    bt_ull_le_remove_air_cig_t param;
    param.cig_id = cig_id;
    status = bt_ull_le_remove_air_cig(&param);
    if (BT_STATUS_SUCCESS == status) {
        bt_ull_le_conn_srv_set_cig_state(BT_ULL_LE_CONN_SRV_CIG_STATE_REMOVING);
    }
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
    ull_report("[ULL][LE][CONN][VENDOR CMD] vendor remove air cig cmd, status: 0x%x, cig_id: 0x%x", 2, \
               status,
               param.cig_id
              );
#endif

    return status;
}
static bt_status_t bt_ull_le_conn_srv_create_air_cis_internal(bt_handle_t acl_handle, uint8_t cis_count)
{
    bt_status_t status;
    uint8_t i = 0;
    uint8_t ct;
    uint8_t cig_state = bt_ull_le_conn_srv_get_cig_state();
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_create_air_cis_internal, acl handle: 0x%x, cis count: 0x%x, cig_state: %d", 3, acl_handle, cis_count, cig_state);
    
    if (cis_count > bt_ull_le_conn_srv_get_cis_count() || cis_count == 0 || BT_ULL_LE_CONN_SRV_CIG_STATE_CREATED != cig_state) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_create_air_cis_internal, cis num error!", 0);
        return BT_STATUS_FAIL;
    }
    bt_ull_le_air_cis_set_t *cis_list = bt_ull_le_srv_memory_alloc(sizeof(bt_ull_le_air_cis_set_t) * cis_count);
    if (!cis_list) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_create_air_cis_internal, out of memory!", 0);
        return BT_STATUS_OUT_OF_MEMORY;
    }

    bt_ull_le_srv_memset(cis_list, 0, sizeof(bt_ull_le_air_cis_set_t) * cis_count);
    ct = bt_ull_le_conn_srv_get_client_type();
    switch (ct) {
        case BT_ULL_HEADSET_CLIENT: {
            for (i = 0; i < cis_count; i ++) {
                if (BT_ULL_LE_CONN_SRV_AIR_CIS_ACTIVE_CONNECTING < bt_ull_le_conn_srv_get_cis_state(i)) {
                    ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_create_air_cis_internal, invalid conn state!", 0);
                    bt_ull_le_srv_memory_free(cis_list);
                    if (i == 1) {
                        bt_ull_le_conn_srv_set_cis_state(i-1, BT_ULL_LE_CONN_SRV_AIR_CIS_DISCONNECTED);
                    }
                    return BT_STATUS_FAIL;
                } else if (BT_ULL_LE_CONN_SRV_AIR_CIS_ACTIVE_CONNECTING == bt_ull_le_conn_srv_get_cis_state(i)) {
                    ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_create_air_cis_internal, is connecting!", 0);
                    bt_ull_le_srv_memory_free(cis_list);
                    return BT_STATUS_SUCCESS;
                }
                cis_list[i].cis_connection_handle = bt_ull_le_conn_srv_get_cis_handle(i);
                cis_list[i].acl_connection_handle = acl_handle;
                cis_list[i].ul_enable = (bt_ull_le_conn_srv_get_cis_id(i) == BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK1) ? true : false;
                /*
                            if (bt_ull_le_conn_srv_get_cis_id(i) == BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK1) {
                                cis_list[i].ul_enable = true;
                            } else {
                                cis_list[i].ul_enable = false;
                            }
                            */
                bt_ull_le_conn_srv_set_acl_handle(i, acl_handle);
                bt_ull_le_conn_srv_set_cis_state(i, BT_ULL_LE_CONN_SRV_AIR_CIS_ACTIVE_CONNECTING);
                g_bt_ull_le_conn_ctrl.evt_mask.establish |= (1 << i);
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
                ull_report("[ULL][LE][CONN][VENDOR CMD] creat air cis(headset) cmd-cis_list[%d], cis_handle: 0x%x, acl_handle: 0x%x, active_uplink: 0x%x, evt mask: 0x%x", 5, \
                           i,
                           cis_list[i].cis_connection_handle,
                           cis_list[i].acl_connection_handle,
                           cis_list[i].ul_enable,
                           g_bt_ull_le_conn_ctrl.evt_mask.establish
                          );
#endif

            }
            break;
        }
        case BT_ULL_SPEAKER_CLIENT: {
            if (cis_count > 1) {
                ull_assert(0 && "CIS CONUT ERROR!");
            }
            uint8_t idx = bt_ull_le_conn_srv_get_avaliable_cis_idx();
            if (BT_ULL_LE_CONN_SRV_INVALID_IDX == idx) {
                ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_create_air_cis_internal, No link resource!", 0);
                bt_ull_le_srv_memory_free(cis_list);
                return BT_STATUS_FAIL;
            }

            cis_list->cis_connection_handle = bt_ull_le_conn_srv_get_cis_handle(idx);
            cis_list->acl_connection_handle = acl_handle;
            cis_list->ul_enable = false;
            bt_ull_le_conn_srv_set_acl_handle(idx, acl_handle);
            bt_ull_le_conn_srv_set_cis_state(idx, BT_ULL_LE_CONN_SRV_AIR_CIS_ACTIVE_CONNECTING);
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
            ull_report("[ULL][LE][CONN][VENDOR CMD] creat air cis(speaker) ,idx: 0x%x, ct: %d, cis_handle: 0x%x, acl_handle: 0x%x, ul_enable: 0x%x", 5, \
                       idx,
                       ct,
                       cis_list->cis_connection_handle,
                       cis_list->acl_connection_handle,
                       cis_list->ul_enable
                      );
#endif
            break;
        }

        case BT_ULL_EARBUDS_CLIENT: {
            if (cis_count > 1) {
                ull_assert(0 && "CIS CONUT ERROR!");
            }
            bt_ull_le_audio_location_t al = bt_ull_le_srv_get_audio_location_by_handle(acl_handle);
            uint8_t idx = bt_ull_le_conn_srv_get_idx_by_location(al);
            if (BT_ULL_LE_CONN_SRV_INVALID_IDX == idx) {
                idx = bt_ull_le_conn_srv_get_avaliable_cis_idx();
            } else {
                if (BT_ULL_LE_CONN_SRV_AIR_CIS_ACTIVE_CONNECTING < bt_ull_le_conn_srv_get_cis_state(idx)) {
                    ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_create_air_cis_internal, invalid conn state!", 0);
                    bt_ull_le_srv_memory_free(cis_list);
                    return BT_STATUS_FAIL;
                } else if (BT_ULL_LE_CONN_SRV_AIR_CIS_ACTIVE_CONNECTING == bt_ull_le_conn_srv_get_cis_state(idx)) {
                    ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_create_air_cis_internal, is connecting!", 0);
                    bt_ull_le_srv_memory_free(cis_list);
                    return BT_STATUS_FAIL;
                }
            }
            if (BT_ULL_LE_CONN_SRV_INVALID_IDX == idx) { 
                bt_ull_le_srv_memory_free(cis_list);
                return BT_STATUS_FAIL;
            }

            //uint8_t cis_id = bt_ull_le_conn_srv_get_cis_id(idx);
            cis_list->cis_connection_handle = bt_ull_le_conn_srv_get_cis_handle(idx);
            cis_list->acl_connection_handle = acl_handle;
            cis_list->ul_enable = bt_ull_le_srv_check_ul_activate_state_by_handle(acl_handle);
            bt_ull_le_conn_srv_set_acl_handle(idx, acl_handle);
            bt_ull_le_conn_srv_set_cis_state(idx, BT_ULL_LE_CONN_SRV_AIR_CIS_ACTIVE_CONNECTING);
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
            ull_report("[ULL][LE][CONN][VENDOR CMD] creat air cis(earbuds) ,idx: 0x%x, ct: %d, cis_handle: 0x%x, acl_handle: 0x%x, ul_enable: 0x%x", 5, \
                       idx,
                       ct,
                       cis_list->cis_connection_handle,
                       cis_list->acl_connection_handle,
                       cis_list->ul_enable
                      );
#endif
/*
            for (i = 0; i < cis_count; i ++) {
                uint8_t idx = bt_ull_le_conn_srv_get_idx_by_location();
                if (BT_ULL_LE_CONN_SRV_INVALID_IDX == avaliable_idx) {
                    ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_create_air_cis_internal, invalid idx!", 0);
                    return BT_STATUS_FAIL;
                }
                cis_list[i].cis_connection_handle = bt_ull_le_conn_srv_get_cis_handle(avaliable_idx);
                cis_list[i].acl_connection_handle = acl_handle;
                if (bt_ull_le_conn_srv_get_cis_id(avaliable_idx) == BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK1) {
                    cis_list[i].ul_enable = true;
                } else {
                    cis_list[i].ul_enable = false;
                }
                bt_ull_le_conn_srv_set_acl_handle(avaliable_idx, acl_handle);
                bt_ull_le_conn_srv_set_cis_state(avaliable_idx, BT_ULL_LE_CONN_SRV_AIR_CIS_ACTIVE_CONNECTING);
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
                ull_report("[ULL][LE][CONN][VENDOR CMD] creat air cis ,avaliable_idx: 0x%x, cmd-cis_list[%d], cis_handle: 0x%x, acl_handle: 0x%x, ul_enable: 0x%x", 5, \
                           avaliable_idx,
                           i,
                           cis_list[i].cis_connection_handle,
                           cis_list[i].acl_connection_handle,
                           cis_list[i].ul_enable
                          );
#endif

            }
*/
            break;
        }

        case BT_ULL_MIC_CLIENT: {
            for (i = 0; i < cis_count; i ++) {
                uint8_t avaliable_idx = bt_ull_le_conn_srv_get_avaliable_cis_idx();
                if (BT_ULL_LE_CONN_SRV_INVALID_IDX == avaliable_idx) {
                    ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_create_air_cis_internal, invalid idx!", 0);
                    bt_ull_le_srv_memory_free(cis_list);
                    return BT_STATUS_FAIL;
                }
                cis_list[i].cis_connection_handle = bt_ull_le_conn_srv_get_cis_handle(avaliable_idx);

                cis_list[i].acl_connection_handle = acl_handle;
                cis_list[i].ul_enable = true;
                bt_ull_le_conn_srv_set_acl_handle(avaliable_idx, acl_handle);
                bt_ull_le_conn_srv_set_cis_state(avaliable_idx, BT_ULL_LE_CONN_SRV_AIR_CIS_ACTIVE_CONNECTING);
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
                ull_report("[ULL][LE][CONN][VENDOR CMD] creat air cis cmd-cis_list[%d], cis_handle: 0x%x, acl_handle: 0x%x, ul_enable: 0x%x", 4, \
                           i,
                           cis_list[i].cis_connection_handle,
                           cis_list[i].acl_connection_handle,
                           cis_list[i].ul_enable
                          );
#endif

            }
            break;
        }
        default: {
            //todo
            ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_create_air_cis_internal, ct mode is error!", 0);
            bt_ull_le_srv_memory_free(cis_list);
            return BT_STATUS_FAIL;
        }

    }
    bt_ull_le_create_air_cis_t param;
    param.cis_count = cis_count;
    param.cis_list = cis_list;
    status = bt_ull_le_create_air_cis(&param);
    bt_ull_le_srv_memory_free(cis_list);
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
    ull_report("[ULL][LE][CONN][VENDOR CMD] creat air cis cmd, status: 0x%x, cis_count: 0x%x", 2, \
               status,
               param.cis_count
              );
#endif
    if (BT_STATUS_SUCCESS != status) {
        for (i = 0; i < cis_count; i ++) {
            bt_ull_le_conn_srv_set_acl_handle(i, BT_HANDLE_INVALID);
            bt_ull_le_conn_srv_set_cis_state(i, BT_ULL_LE_CONN_SRV_AIR_CIS_DISCONNECTED);
        }
    }

    return status;
}

static bt_status_t bt_ull_le_conn_srv_destroy_air_cis_internal(bt_handle_t cis_handle)
{
    bt_status_t status;
    bt_ull_le_conn_srv_air_cis_state_t temp_state = BT_ULL_LE_CONN_SRV_AIR_CIS_DISCONNECTED;
    uint8_t idx = bt_ull_le_conn_srv_get_idx_by_cis_handle(cis_handle);
    uint8_t current_state = bt_ull_le_conn_srv_get_cis_state(idx);
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_destroy_air_cis_internal, current_state: 0x%x, idx: 0x%x", 2, current_state, idx);
    if (BT_ULL_LE_CONN_SRV_INVALID_IDX == idx) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_destroy_air_cis_internal, invalid handle!", 0);
        return BT_STATUS_FAIL;
    }

    current_state = bt_ull_le_conn_srv_get_cis_state(idx);
    if (current_state == BT_ULL_LE_CONN_SRV_AIR_CIS_DISCONNECTING) {
        return BT_STATUS_SUCCESS;
    }

    if (current_state < BT_ULL_LE_CONN_SRV_AIR_CIS_ACTIVE_CONNECTING) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_destroy_air_cis_internal, cis has disconnected!", 0);
        return BT_STATUS_FAIL;
    }

    if (bt_ull_le_conn_srv_cmd_is_lock(BT_ULL_LE_CONN_SRV_CMD_DISCONNECT)) {
        bt_ull_le_conn_srv_cmd_cache_item_t *cache_cmd = bt_ull_le_conn_srv_new_cache_cmd_node(BT_ULL_LE_CONN_SRV_CMD_DISCONNECT);
        cache_cmd->handle = cis_handle;
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_destroy_air_cis_internal, destroy air cis busy!", 0);
        return BT_STATUS_SUCCESS;
    }
    temp_state = bt_ull_le_conn_srv_get_cis_state(idx);
    bt_ull_le_conn_srv_set_cis_state(idx, BT_ULL_LE_CONN_SRV_AIR_CIS_DISCONNECTING);
    g_bt_ull_le_conn_ctrl.evt_mask.disconnect |= (1 << idx);
    bt_ull_le_conn_srv_lock_cmd(BT_ULL_LE_CONN_SRV_CMD_DISCONNECT, cis_handle);
    bt_ull_le_air_cis_disconnect_t param;
    param.connection_handle = cis_handle;
    param.reason = BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST;
    status = bt_ull_le_disconnect_air_cis(&param);
    if (BT_STATUS_SUCCESS != status) {
        bt_ull_le_conn_srv_set_cis_state(idx, temp_state);
        bt_ull_le_conn_srv_unlock_cmd(BT_ULL_LE_CONN_SRV_CMD_DISCONNECT);
    }

#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
    ull_report("[ULL][LE][CONN][VENDOR CMD] vendor disconnect cmd, status: 0x%x, connection handle: 0x%x, reason: 0x%x", 3, \
               status, param.connection_handle, param.reason);
#endif
    return status;
}


static void bt_ull_le_conn_srv_set_cig_state(uint8_t state)
{
    g_bt_ull_le_conn_ctrl.cig_state = state;
}

static uint8_t bt_ull_le_conn_srv_get_cig_state(void)
{
    return g_bt_ull_le_conn_ctrl.cig_state;
}

static uint8_t bt_ull_le_conn_srv_get_idx_by_cis_handle(bt_handle_t cis_handle)
{
    uint8_t link_num;
    for (link_num = 0; link_num < BT_ULL_LE_AIR_CIS_MAX_NUM; link_num ++) {
        if (g_bt_ull_le_conn_ctrl.cis_info[link_num].cis_handle == cis_handle) {
            return link_num;
        }
    }
    ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_get_idx_by_cis_handle, invalid handle!", 0);
    return BT_ULL_LE_CONN_SRV_INVALID_IDX;
}

uint8_t bt_ull_le_conn_srv_get_avaliable_cis_idx(void)
{
    uint8_t link_num = 0;
    for (link_num = 0; link_num < BT_ULL_LE_AIR_CIS_MAX_NUM; link_num ++) {
        if (g_bt_ull_le_conn_ctrl.cis_info[link_num].state == BT_ULL_LE_CONN_SRV_AIR_CIS_DISCONNECTED) {
            return link_num;
        }
    }
    ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_get_avaliable_cis_idx, no avaliable idx!", 0);
    return BT_ULL_LE_CONN_SRV_INVALID_IDX;
}

static uint8_t bt_ull_le_conn_srv_get_idx_by_acl_handle(bt_handle_t acl_handle)
{
    uint8_t link_num;
    for (link_num = 0; link_num < BT_ULL_LE_AIR_CIS_MAX_NUM; link_num ++) {
        if (g_bt_ull_le_conn_ctrl.cis_info[link_num].acl_handle == acl_handle) {
            return link_num;
        }
    }

    ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_get_idx_by_acl_handle, invalid handle!", 0);
    return BT_ULL_LE_CONN_SRV_INVALID_IDX;

}

static void bt_ull_le_conn_srv_set_cis_id(uint8_t idx, bt_ull_le_srv_air_cis_id_enum cis_id)
{
    if (idx >= BT_ULL_LE_AIR_CIS_MAX_NUM ||
            cis_id >= BT_ULL_LE_CONN_SRV_AIR_CIS_ID_MAX || cis_id == BT_ULL_LE_CONN_SRV_AIR_CIS_ID_INVANLID) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_set_cis_id, invalid idx!", 0);
        return;
    }
    g_bt_ull_le_conn_ctrl.cis_info[idx].cis_id = cis_id;
}

static uint8_t bt_ull_le_conn_srv_get_cis_id(uint8_t idx)
{
    if (idx >= BT_ULL_LE_AIR_CIS_MAX_NUM) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_get_cis_id, invalid idx!", 0);
        return BT_ULL_LE_CONN_SRV_AIR_CIS_ID_INVANLID;
    }
    return g_bt_ull_le_conn_ctrl.cis_info[idx].cis_id;
}

bt_ull_le_conn_srv_air_cis_link_info_t *bt_ull_le_conn_srv_get_cis_info(uint8_t idx)
{
    if (idx >= BT_ULL_LE_AIR_CIS_MAX_NUM) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_get_cis_info, invalid idx!", 0);
        return NULL;
    }
    return &g_bt_ull_le_conn_ctrl.cis_info[idx];
}

static void bt_ull_le_conn_srv_reset_cis_info(uint8_t idx)
{
    if (idx >= BT_ULL_LE_AIR_CIS_MAX_NUM) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_reset_cis_info, invalid idx!", 0);
        return;
    }
    g_bt_ull_le_conn_ctrl.cis_info[idx].acl_handle = BT_HANDLE_INVALID;
    g_bt_ull_le_conn_ctrl.cis_info[idx].cis_handle = BT_HANDLE_INVALID;
    g_bt_ull_le_conn_ctrl.cis_info[idx].state = BT_ULL_LE_CONN_SRV_AIR_CIS_DISCONNECTED;
    g_bt_ull_le_conn_ctrl.cis_info[idx].cis_id = BT_ULL_LE_CONN_SRV_AIR_CIS_ID_INVANLID;
    g_bt_ull_le_conn_ctrl.cis_info[idx].dl_enable = false;
    g_bt_ull_le_conn_ctrl.cis_info[idx].ul_enable = false;
}

static void bt_ull_le_conn_srv_set_cis_state(uint8_t idx, bt_ull_le_conn_srv_air_cis_state_t state)
{
    if (idx >= BT_ULL_LE_AIR_CIS_MAX_NUM) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_set_cis_state, invalid idx!", 0);
        return;
    }
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_set_cis_state, idx: 0x%x, state:0x%x -> 0x%x", 3, idx, g_bt_ull_le_conn_ctrl.cis_info[idx].state, state);
    g_bt_ull_le_conn_ctrl.cis_info[idx].state = state;
}

static uint8_t bt_ull_le_conn_srv_get_cis_state(uint8_t idx)
{
    if (idx >= BT_ULL_LE_AIR_CIS_MAX_NUM) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_get_cis_state, invalid idx!", 0);
        return BT_ULL_LE_CONN_SRV_AIR_CIS_STATE_MAX;
    }
    return g_bt_ull_le_conn_ctrl.cis_info[idx].state;
}

static uint8_t bt_ull_le_conn_srv_cis_is_connected(uint8_t idx)
{
    if (idx >= BT_ULL_LE_AIR_CIS_MAX_NUM) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_cis_is_connected, invalid idx!", 0);
        return BT_ULL_LE_CONN_SRV_AIR_CIS_STATE_MAX;
    }
    if (g_bt_ull_le_conn_ctrl.cis_info[idx].state >= BT_ULL_LE_CONN_SRV_AIR_CIS_CONNECTED) {
        return true;
    }
    return false;
}

static uint8_t bt_ull_le_conn_srv_get_connected_cis_num(void)
{
    uint8_t i, num = 0;
    for (i = 0; i < BT_ULL_LE_AIR_CIS_MAX_NUM; i ++) {
        if (bt_ull_le_conn_srv_cis_is_connected(i)) {
            num ++;
        }
    }
    return num;
}

static uint8_t bt_ull_le_conn_srv_get_connecting_cis_num(void)
{
    uint8_t i, num = 0;
    for (i = 0; i < BT_ULL_LE_AIR_CIS_MAX_NUM; i ++) {
        if (BT_ULL_LE_CONN_SRV_AIR_CIS_ACTIVE_CONNECTING == g_bt_ull_le_conn_ctrl.cis_info[i].state \
            || BT_ULL_LE_CONN_SRV_AIR_CIS_PASSIVE_CONNECTING == g_bt_ull_le_conn_ctrl.cis_info[i].state) {
            num ++;
        }
    }
    return num;
}

static uint8_t bt_ull_le_conn_srv_get_streaming_cis_num(void)
{
    uint8_t i, num = 0;
    for (i = 0; i < BT_ULL_LE_AIR_CIS_MAX_NUM; i ++) {
        if (g_bt_ull_le_conn_ctrl.cis_info[i].state == BT_ULL_LE_CONN_SRV_AIR_CIS_ACTIVIATED) {
            num ++;
        }
    }
    return num;

}

static bt_handle_t bt_ull_le_conn_srv_get_acl_handle(uint8_t idx)
{
    if (idx >= BT_ULL_LE_AIR_CIS_MAX_NUM) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_get_acl_handle, invalid idx!", 0);
        return BT_HANDLE_INVALID;
    }
    return g_bt_ull_le_conn_ctrl.cis_info[idx].acl_handle;
}

static void bt_ull_le_conn_srv_set_acl_handle(uint8_t idx, bt_handle_t acl_handle)
{
    if (idx >= BT_ULL_LE_AIR_CIS_MAX_NUM) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_set_acl_handle, invalid idx!", 0);
        return;
    }
    g_bt_ull_le_conn_ctrl.cis_info[idx].acl_handle = acl_handle;
}

static void bt_ull_le_conn_srv_set_ul_active_state(uint8_t idx, bool is_enable)
{
    if (idx >= BT_ULL_LE_AIR_CIS_MAX_NUM || !bt_ull_le_conn_srv_cis_is_connected(idx)) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_set_ul_active_state, invalid idx!", 0);
        return;
    }
    g_bt_ull_le_conn_ctrl.cis_info[idx].ul_enable = is_enable;
}

static uint8_t bt_ull_le_conn_srv_get_ul_active_state(uint8_t idx)
{
    if (idx >= BT_ULL_LE_AIR_CIS_MAX_NUM || !bt_ull_le_conn_srv_cis_is_connected(idx)) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_set_ul_active_state, invalid idx!", 0);
        return BT_ULL_LE_CONN_SRV_INVALID_IDX;
    }
    return g_bt_ull_le_conn_ctrl.cis_info[idx].ul_enable;
}

static void bt_ull_le_conn_srv_set_dl_active_state(uint8_t idx, bool is_enable)
{
    if (idx >= BT_ULL_LE_AIR_CIS_MAX_NUM || !bt_ull_le_conn_srv_cis_is_connected(idx)) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_set_ul_active_state, invalid idx!", 0);
        return;
    }
    g_bt_ull_le_conn_ctrl.cis_info[idx].dl_enable = is_enable;
}

uint8_t bt_ull_le_conn_srv_get_dl_active_state(uint8_t idx)
{
    if (idx >= BT_ULL_LE_AIR_CIS_MAX_NUM || !bt_ull_le_conn_srv_cis_is_connected(idx)) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_set_ul_active_state, invalid idx!", 0);
        return BT_ULL_LE_CONN_SRV_INVALID_IDX;
    }
    return g_bt_ull_le_conn_ctrl.cis_info[idx].dl_enable;
}


static uint8_t bt_ull_le_conn_srv_get_cis_count(void)
{
    return g_bt_ull_le_conn_ctrl.cis_count;
}

static void bt_ull_le_conn_srv_set_cis_count(uint8_t cis_count)
{
    if (cis_count > BT_ULL_LE_AIR_CIS_MAX_NUM) {
        ull_report_error("[ULL][LE][CONN] cis_count error: 0x%x", 1, cis_count);
        return;
    }
    g_bt_ull_le_conn_ctrl.cis_count = cis_count;
}

void bt_ull_le_conn_srv_set_device_info(bt_ull_client_t ct, uint8_t cis_count)
{
    g_bt_ull_le_conn_ctrl.cis_count = cis_count;
}

static bt_ull_client_t bt_ull_le_conn_srv_get_client_type(void)
{
    return bt_ull_le_srv_get_client_type();
}
static void bt_ull_le_conn_srv_clear_evt_mask(bt_ull_le_conn_srv_mask_t type)
{
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_clear_evt_mask, type: 0x%x,", 1, type);
    if (BT_ULL_LE_CONN_SRV_ESTABLISHMENT_EVT_MASK == type) {
        g_bt_ull_le_conn_ctrl.evt_mask.establish = 0;
    } else if (BT_ULL_LE_CONN_SRV_DISCONNECT_EVT_MASK == type) {
        g_bt_ull_le_conn_ctrl.evt_mask.disconnect = 0;
    } else if (BT_ULL_LE_CONN_SRV_SET_ISO_DATA_PATH_EVT_MASK == type) {
        g_bt_ull_le_conn_ctrl.evt_mask.set_iso_data_path = 0;
    } else {
        bt_ull_le_srv_memset(&g_bt_ull_le_conn_ctrl.evt_mask, 0, sizeof(bt_ull_le_conn_srv_evt_mask_t));
    }
}

static bt_ull_le_conn_srv_context_t *bt_ull_le_conn_srv_get_ctx(void)
{
    return &g_bt_ull_le_conn_ctrl;
}

static void bt_ull_le_conn_srv_set_cig_id(uint8_t cig_id)
{
    g_bt_ull_le_conn_ctrl.cig_id = cig_id;
}

static uint8_t bt_ull_le_conn_srv_get_cig_id(void)
{
    return g_bt_ull_le_conn_ctrl.cig_id;
}

static void bt_ull_le_conn_srv_set_cis_handle(uint8_t idx, bt_handle_t cis_handle)
{
    if (idx >= BT_ULL_LE_AIR_CIS_MAX_NUM) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_set_acl_handle, invalid idx!", 0);
        return;
    }
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_set_cis_handle, idx: 0x%x, cis_handle: 0x%x", 2, idx, cis_handle);
    g_bt_ull_le_conn_ctrl.cis_info[idx].cis_handle = cis_handle;

}

static bt_handle_t bt_ull_le_conn_srv_get_cis_handle(uint8_t idx)
{
    if (idx >= BT_ULL_LE_AIR_CIS_MAX_NUM) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_get_cis_handle, invalid idx!", 0);
        return BT_HANDLE_INVALID;
    }
    return g_bt_ull_le_conn_ctrl.cis_info[idx].cis_handle;
}

static uint8_t bt_ull_le_conn_srv_get_evt_mask(bt_ull_le_conn_srv_mask_t type)
{
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_get_evt_mask, type: 0x%x,", 1, type);
    if (BT_ULL_LE_CONN_SRV_ESTABLISHMENT_EVT_MASK == type) {
        return g_bt_ull_le_conn_ctrl.evt_mask.establish;
    } else if (BT_ULL_LE_CONN_SRV_DISCONNECT_EVT_MASK == type) {
        return g_bt_ull_le_conn_ctrl.evt_mask.disconnect;
    } else if (BT_ULL_LE_CONN_SRV_SET_ISO_DATA_PATH_EVT_MASK == type) {
        return g_bt_ull_le_conn_ctrl.evt_mask.set_iso_data_path;
    } else {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_get_evt_mask, error type", 0);
        return 0xff;
    }
}

static bt_ull_role_t bt_ull_le_conn_srv_get_role(void)
{
    return g_bt_ull_le_conn_ctrl.role;
}

static void bt_ull_le_conn_srv_set_role(bt_ull_role_t role)
{
    g_bt_ull_le_conn_ctrl.role = role;
}

static uint8_t bt_ull_le_conn_srv_get_data_path_id(bt_ull_le_srv_air_cis_id_enum cis_id, bt_ull_client_t ct)
{
    uint8_t i = 0;
    if (BT_ULL_MIC_CLIENT == ct) {
    for (i = 0; i < BT_ULL_LE_AIR_CIS_MAX_NUM; i ++) {
        if (g_bt_ull_mic_cis_id_table[i].cis_id == cis_id) {
            return g_bt_ull_mic_cis_id_table[i].data_path_id;
        }
    }
    } else if (BT_ULL_EARBUDS_CLIENT == ct || BT_ULL_HEADSET_CLIENT == ct) {
    for (i = 0; i < BT_ULL_LE_AIR_CIS_MAX_NUM; i ++) {
        if (g_bt_ull_cis_id_table[i].cis_id == cis_id) {
            return g_bt_ull_cis_id_table[i].data_path_id;
        }
    }
    } else if (BT_ULL_SPEAKER_CLIENT == ct) {
        for (i = 0; i < BT_ULL_LE_AIR_CIS_MAX_NUM; i ++) {
            if (g_bt_ull_spk_cis_id_table[i].cis_id == cis_id) {
                return g_bt_ull_spk_cis_id_table[i].data_path_id;
            }
        }
    }
    ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_get_data_path_id, invalid cis id!", 0);
    return BT_ULL_LE_SRV_DATA_PATH_ID_DISABLE;

}

static void bt_ull_le_conn_srv_set_phy(bt_ull_le_conn_srv_phy_t phy)
{
    g_bt_ull_le_conn_ctrl.phy = phy;
}

static void bt_ull_le_conn_srv_set_codec_type(bt_ull_le_codec_t codec)
{
    g_bt_ull_le_conn_ctrl.aud_ctrl.codec = codec;
}

static bt_ull_le_codec_t bt_ull_le_conn_srv_get_codec_type(void)
{
    return g_bt_ull_le_conn_ctrl.aud_ctrl.codec;
}

uint8_t bt_ull_le_conn_srv_get_phy(void)
{
    return g_bt_ull_le_conn_ctrl.phy;
}

void bt_ull_le_conn_srv_set_latency(bt_ull_le_srv_latency_t latency)
{
    g_bt_ull_le_conn_ctrl.aud_ctrl.latency = latency;
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_set_latency, latency: %d", 1, latency);
}
uint8_t bt_ull_le_conn_srv_get_latency(void)
{
    return g_bt_ull_le_conn_ctrl.aud_ctrl.latency;
}

void bt_ull_le_conn_srv_set_audio_quality(bt_ull_le_srv_audio_quality_t quality)
{
    g_bt_ull_le_conn_ctrl.aud_ctrl.audio_quality = quality;
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_set_audio_quality, quality: %d", 1, quality);
}
static uint8_t bt_ull_le_conn_srv_get_audio_quality(void)
{
    return g_bt_ull_le_conn_ctrl.aud_ctrl.audio_quality;
}

static bt_ull_le_conn_srv_frame_duration_param_t *bt_ull_le_conn_srv_get_frame_duration_params(bt_ull_le_srv_latency_t latency) {
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_get_frame_duration_params, latency: %d", 1, latency);

    if (BT_ULL_LE_FT_LABEL_MAX <= latency) {
        return NULL;
    }
    uint8_t i = 0;
    for (i = 0; i < BT_ULL_LE_FT_LABEL_MAX; i ++) {
        if (latency == g_bt_ull_latency_ft_table[i].latency) {
            return &g_bt_ull_latency_ft_table[i];
        }
    }
    return NULL;
}

static uint8_t bt_ull_le_conn_srv_get_frame_duration_idx(bt_ull_le_srv_latency_t latency) {
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_get_frame_duration_params, latency: %d", 1, latency);

    if (BT_ULL_LE_FT_LABEL_MAX <= latency) {
        return BT_ULL_LE_CONN_SRV_INVALID_IDX;
    }
    uint8_t i = 0;
    for (i = 0; i < BT_ULL_LE_FT_LABEL_MAX; i ++) {
        if (latency == g_bt_ull_latency_ft_table[i].latency) {
            return i;
        }
    }
    return BT_ULL_LE_CONN_SRV_INVALID_IDX;
}

bt_ull_le_conn_srv_air_cig_param_t *bt_ull_le_conn_srv_get_air_cig_params(bt_ull_le_srv_audio_quality_t quality)
{
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_get_air_cig_params, quality: %d", 1, quality);
    bt_ull_le_conn_srv_context_t *ctx = bt_ull_le_conn_srv_get_ctx();

    if (!ctx->aud_ctrl.select_cig_params) {
        ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_get_air_cig_params, null table", 0);
        return NULL;
    } 
    uint8_t i = 0;
    for (i = 0; i < ctx->aud_ctrl.label_count; i ++) {
         if (ctx->aud_ctrl.select_cig_params[i].audio_quality == quality) {
             return &ctx->aud_ctrl.select_cig_params[i];
         }
    }
    return NULL;
}

static uint8_t bt_ull_le_conn_srv_get_air_cig_tabel_idx(bt_ull_le_srv_audio_quality_t quality)
{
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_get_air_cig_tabel_idx, quality: %d", 1, quality);
    bt_ull_le_conn_srv_context_t *ctx = bt_ull_le_conn_srv_get_ctx();

    if (!ctx->aud_ctrl.select_cig_params) {
        ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_get_air_cig_tabel_idx, null table", 0);
        return BT_ULL_LE_CONN_SRV_INVALID_IDX;
    } 
    uint8_t i = 0;
    for (i = 0; i < ctx->aud_ctrl.label_count; i ++) {
         if (ctx->aud_ctrl.select_cig_params[i].audio_quality == quality) {
             return i;
         }
    }
    return BT_ULL_LE_CONN_SRV_INVALID_IDX;
}

bt_status_t bt_ull_le_conn_srv_establish_air_cis(bt_handle_t handle)
{
    bt_status_t status;
    uint8_t cis_count = 0;
    uint8_t cig_state = bt_ull_le_conn_srv_get_cig_state();
    bt_ull_role_t role = bt_ull_le_conn_srv_get_role();
    bt_ull_client_t ct = bt_ull_le_conn_srv_get_client_type();
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_establish_air_cis, role: %d, cig_state: %d, ct: 0x%x, handle: 0x%x", 4, role, cig_state, ct, handle);
    if (role != BT_ULL_ROLE_SERVER) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_establish_air_cis, role error!", 0);
        return BT_STATUS_FAIL;
    }
    if (BT_ULL_LE_CONN_SRV_CIG_STATE_CREATED == cig_state) {
        if (bt_ull_le_conn_srv_cmd_is_lock(BT_ULL_LE_CONN_SRV_CMD_ESTABLISH)) {
            bt_ull_le_conn_srv_cmd_cache_item_t *cache_cmd = bt_ull_le_conn_srv_new_cache_cmd_node(BT_ULL_LE_CONN_SRV_CMD_ESTABLISH);
            cache_cmd->handle = handle;
            ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_establish_air_cis, establish air cis busy!", 0);
            return BT_STATUS_SUCCESS;
        }
        bt_ull_le_conn_srv_lock_cmd(BT_ULL_LE_CONN_SRV_CMD_ESTABLISH, handle);
        cis_count = (ct == BT_ULL_HEADSET_CLIENT) ? 2 : 1;
        status = bt_ull_le_conn_srv_create_air_cis_internal(handle, cis_count);
        if (BT_STATUS_SUCCESS != status) {
            bt_ull_le_conn_srv_unlock_cmd(BT_ULL_LE_CONN_SRV_CMD_ESTABLISH);
        }
        return status;
    } else {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_establish_air_cis, cig not set!", 0);
    }
    return BT_STATUS_FAIL;

}

bt_status_t bt_ull_le_conn_srv_destroy_air_cis(bt_handle_t handle)
{
    uint8_t i;
    uint8_t idx;
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_client_t ct = bt_ull_le_conn_srv_get_client_type();
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_destroy_air_cis, ct: 0x%x", 1, ct);
    if (ct == BT_ULL_HEADSET_CLIENT) {
        for (i = 0; i < BT_ULL_LE_AIR_CIS_MAX_NUM; i ++) {
            if (bt_ull_le_conn_srv_cis_is_connected(i) && (bt_ull_le_conn_srv_get_acl_handle(i) == handle)) {
                status = bt_ull_le_conn_srv_destroy_air_cis_internal(bt_ull_le_conn_srv_get_cis_handle(i));
            }
        }
        return status;
    } else if (ct == BT_ULL_EARBUDS_CLIENT \
        || ct == BT_ULL_MIC_CLIENT \
        || ct == BT_ULL_SPEAKER_CLIENT) {
        idx = bt_ull_le_conn_srv_get_idx_by_acl_handle(handle);
        if (bt_ull_le_conn_srv_cis_is_connected(idx)) {
            status = bt_ull_le_conn_srv_destroy_air_cis_internal(bt_ull_le_conn_srv_get_cis_handle(idx));
        }
        return status;
    }
    ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_destroy_air_cis, ct error!", 0);
    return BT_STATUS_FAIL;
}

bt_status_t bt_ull_le_conn_srv_switch_latency(bt_ull_le_srv_latency_t latency)
{
    bt_status_t status = BT_STATUS_FAIL;
    uint8_t cig_id = bt_ull_le_conn_srv_get_cig_id();

    if (cig_id == BT_ULL_LE_CONN_SRV_AIR_CIG_ID_NONE || BT_ULL_LE_CONN_SRV_CIG_STATE_CREATED != bt_ull_le_conn_srv_get_cig_state()) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_switch_latency, cig id error!", 0);
        return BT_STATUS_FAIL;
    }

    uint8_t frame_duration_idx = bt_ull_le_conn_srv_get_frame_duration_idx(latency);
    if (BT_ULL_LE_CONN_SRV_INVALID_IDX == frame_duration_idx) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_switch_latency, invalid label!", 0);
        return BT_STATUS_FAIL;
    }
    bt_ull_le_srv_latency_t temp_latency = bt_ull_le_conn_srv_get_latency();
    if (latency == temp_latency) {
        ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_switch_latency, same latency!", 0);
        return BT_STATUS_SUCCESS;
    }
    bt_ull_le_conn_srv_set_latency(latency);
    if (bt_ull_le_conn_srv_cmd_is_lock(BT_ULL_LE_CONN_SRV_CMD_CHANGE_LABEL)) {
        bt_ull_le_conn_srv_cmd_cache_item_t *cache_cmd = bt_ull_le_conn_srv_new_cache_cmd_node(BT_ULL_LE_CONN_SRV_CMD_CHANGE_LABEL);
        cache_cmd->handle = 0xFF;
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_switch latency, change label busy!", 0);
        return BT_STATUS_PENDING;
    }
    bt_ull_le_conn_srv_lock_cmd(BT_ULL_LE_CONN_SRV_CMD_CHANGE_LABEL, cig_id);

    bt_ull_le_change_air_cig_params_t param;
    param.cig_id = cig_id;
    param.cig_params_index = bt_ull_le_conn_srv_get_air_cig_tabel_idx(bt_ull_le_conn_srv_get_audio_quality());
    param.ft_label_index = frame_duration_idx;

    status = bt_ull_le_change_air_params(&param);
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
    ull_report("[ULL][LE][CONN] CHANGE LABLE(switch latency), cig id : %x, quality: %x, latency: %d, status: 0x%x", 4, cig_id, bt_ull_le_conn_srv_get_audio_quality(), latency, status);
#endif
    if (status) {
        bt_ull_le_conn_srv_set_latency(temp_latency);
    }
    return status;
}

bt_status_t bt_ull_le_conn_srv_change_audio_quality(bt_ull_le_srv_audio_quality_t quality)
{
    bt_status_t status = BT_STATUS_FAIL;
    uint8_t cig_id = bt_ull_le_conn_srv_get_cig_id();
    bt_ull_le_srv_audio_quality_t temp_aud_quality = bt_ull_le_conn_srv_get_audio_quality();
    if (quality == temp_aud_quality) {
        ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_change_audio_quality, same quality! %d, %d", 2, quality, temp_aud_quality);
        return BT_STATUS_SUCCESS;
    }
    bt_ull_le_conn_srv_set_audio_quality(quality);
    if (quality == BT_ULL_LE_SRV_AUDIO_QUALITY_LOW_POWER) {
        return BT_STATUS_SUCCESS;
    }
    if (cig_id == BT_ULL_LE_CONN_SRV_AIR_CIG_ID_NONE || BT_ULL_LE_CONN_SRV_CIG_STATE_CREATED != bt_ull_le_conn_srv_get_cig_state()) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_change_audio_quality, cig id error!", 0);
        return BT_STATUS_FAIL;
    }
    uint8_t tabel_idx = bt_ull_le_conn_srv_get_air_cig_tabel_idx(quality);

    if (BT_ULL_LE_CONN_SRV_INVALID_IDX == tabel_idx) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_change_audio_quality, quality error!", 0);
        return BT_STATUS_FAIL;
    }


    bt_ull_le_conn_srv_air_cig_param_t *cig_params = bt_ull_le_conn_srv_get_air_cig_params(quality);
    bt_ull_le_conn_srv_set_phy(cig_params->phy_m_2_s);
    if (bt_ull_le_conn_srv_cmd_is_lock(BT_ULL_LE_CONN_SRV_CMD_CHANGE_LABEL)) {
        bt_ull_le_conn_srv_cmd_cache_item_t *cache_cmd = bt_ull_le_conn_srv_new_cache_cmd_node(BT_ULL_LE_CONN_SRV_CMD_CHANGE_LABEL);
        cache_cmd->handle = 0xFF;
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_change_audio_quality, change label busy!", 0);
        return BT_STATUS_PENDING;
    }
    bt_ull_le_conn_srv_lock_cmd(BT_ULL_LE_CONN_SRV_CMD_CHANGE_LABEL, cig_id);
    bt_ull_le_change_air_cig_params_t param;
    param.cig_id = cig_id;
    param.cig_params_index = tabel_idx;
    param.ft_label_index = bt_ull_le_conn_srv_get_latency();
    status = bt_ull_le_change_air_params(&param);
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
    ull_report("[ULL][LE][CONN][VENDOR CMD] CHANGE LABLE(change audio quality), cig id: %x, tabel idx: %x (quality: %d), latency: %d, status: 0x%x", 5, cig_id, tabel_idx, quality, param.ft_label_index, status);
#endif
    if (status) {
        bt_ull_le_conn_srv_set_audio_quality(temp_aud_quality);
    }
    return status;
}

bt_status_t bt_ull_le_conn_srv_unmute_air_cis(bt_handle_t handle)
{
    uint8_t i;
    uint8_t idx;
    bt_handle_t cis_handle;
    bt_status_t status = BT_STATUS_SUCCESS;
    bt_ull_client_t ct = bt_ull_le_conn_srv_get_client_type();
#ifdef BT_ULL_LE_CONN_SRV_DEBUG
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_unmute_air_cis, ct: 0x%x", 1, ct);
#endif
    if (BT_ULL_HEADSET_CLIENT == ct) {
        for (i = 0; i < BT_ULL_LE_AIR_CIS_MAX_NUM; i ++) {
            if (bt_ull_le_conn_srv_cis_is_connected(i) && bt_ull_le_conn_srv_get_acl_handle(i) == handle) {
                cis_handle = bt_ull_le_conn_srv_get_cis_handle(i);
                status = bt_ull_le_conn_srv_unmute_air_cis_internal(cis_handle);
            }
        }
    } else {
        idx = bt_ull_le_conn_srv_get_idx_by_acl_handle(handle);
        if (BT_ULL_LE_CONN_SRV_INVALID_IDX != idx) {
            status = bt_ull_le_conn_srv_unmute_air_cis_internal(bt_ull_le_conn_srv_get_cis_handle(idx));
        } else {
            status = BT_STATUS_FAIL;
        }
    }
    return status;

}

static bt_status_t bt_ull_le_conn_srv_unmute_air_cis_internal(bt_handle_t cis_handle)
{
    bt_status_t status;
    uint8_t idx = bt_ull_le_conn_srv_get_idx_by_cis_handle(cis_handle);
    if (BT_ULL_LE_CONN_SRV_INVALID_IDX == idx || !bt_ull_le_conn_srv_cis_is_connected(idx)) {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_unmute_air_cis_internal, invalid idx!", 0);
        return BT_STATUS_FAIL;
    }
    if (bt_ull_le_conn_srv_cmd_is_lock(BT_ULL_LE_CONN_SRV_CMD_UNMUTE)) {
        bt_ull_le_conn_srv_cmd_cache_item_t *cache_cmd = bt_ull_le_conn_srv_new_cache_cmd_node(BT_ULL_LE_CONN_SRV_CMD_UNMUTE);
        cache_cmd->handle = cis_handle;
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_unmute_air_cis_internal, unmute cis busy!", 0);
        return BT_STATUS_SUCCESS;
    }
    bt_ull_le_conn_srv_lock_cmd(BT_ULL_LE_CONN_SRV_CMD_UNMUTE, cis_handle);

    bt_ull_le_unmute_air_cis_t param;
    param.cis_connection_handle = cis_handle;
    status = bt_ull_le_unmute_air_cis(&param);
    if (status != BT_STATUS_SUCCESS) {
        bt_ull_le_conn_srv_unlock_cmd(BT_ULL_LE_CONN_SRV_CMD_UNMUTE);
    }
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
        ull_report("[ULL][LE][CONN][VENDOR CMD] vendor unmute cmd, status: 0x%x, connection handle: 0x%x", 2, \
                   status,
                   cis_handle);
#endif
    return status;
}


bt_status_t bt_ull_le_conn_srv_activate_uplink(bt_handle_t handle)
{
    uint8_t i;
    bt_status_t status;
    bt_handle_t cis_handle = BT_HANDLE_INVALID;
    bt_ull_client_t ct = bt_ull_le_conn_srv_get_client_type();
#ifdef BT_ULL_LE_CONN_SRV_DEBUG
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_activate_uplink, ct: 0x%x", 1, ct);
#endif
    if (ct == BT_ULL_HEADSET_CLIENT) {
        for (i = 0; i < BT_ULL_LE_AIR_CIS_MAX_NUM; i ++) {
            if (bt_ull_le_conn_srv_cis_is_connected(i) && bt_ull_le_conn_srv_get_acl_handle(i) == handle
                    && !bt_ull_le_conn_srv_get_ul_active_state(i)) {
                cis_handle = bt_ull_le_conn_srv_get_cis_handle(i);
                break;
            }
        }
    } else if (ct == BT_ULL_EARBUDS_CLIENT \
        || ct == BT_ULL_MIC_CLIENT \
        || ct == BT_ULL_SPEAKER_CLIENT ) {
        uint8_t idx = bt_ull_le_conn_srv_get_idx_by_acl_handle(handle);
        if (BT_ULL_LE_CONN_SRV_INVALID_IDX == idx) {
            ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_activate_uplink, invalid idx!", 0);
            return BT_STATUS_FAIL;
        }
        uint8_t state = bt_ull_le_conn_srv_get_cis_state(idx);
        uint8_t ul_active = bt_ull_le_conn_srv_get_ul_active_state(idx);
        ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_activate_uplink, ul_enable: %d, state: %d!", 2, ul_active, state);
        if (BT_ULL_LE_CONN_SRV_AIR_CIS_DISCONNECTING == state) {
            return BT_STATUS_FAIL;
        }

        if (!ul_active) {
            cis_handle = bt_ull_le_conn_srv_get_cis_handle(idx);
        } else if (BT_ULL_LE_CONN_SRV_INVALID_IDX == ul_active) {
            if (BT_ULL_LE_CONN_SRV_AIR_CIS_ACTIVE_CONNECTING == state) {
                bt_ull_le_conn_srv_cmd_cache_item_t *cache_cmd = bt_ull_le_conn_srv_new_cache_cmd_node(BT_ULL_LE_CONN_SRV_CMD_CHANGE_UL_PATH);
                cache_cmd->handle = handle;
                ull_report_error("[ULL][LE][CONN] cis is not connected, wait for cis connect!", 0);
                return BT_STATUS_SUCCESS;
            } else {
                return BT_STATUS_FAIL;
            }
        } else {
            ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_activate_uplink, ul has enable!", 0);
            return BT_STATUS_SUCCESS;
        }
    } else {
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_activate_uplink, invalid ct!", 0);
        return BT_STATUS_FAIL;
    }
    if (bt_ull_le_conn_srv_cmd_is_lock(BT_ULL_LE_CONN_SRV_CMD_CHANGE_UL_PATH)) {
        bt_ull_le_conn_srv_cmd_cache_item_t *cache_cmd = bt_ull_le_conn_srv_new_cache_cmd_node(BT_ULL_LE_CONN_SRV_CMD_CHANGE_UL_PATH);
        cache_cmd->handle = handle;
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_activate_uplink, change ul path busy!", 0);
        return BT_STATUS_SUCCESS;
    }
    bt_ull_le_conn_srv_lock_cmd(BT_ULL_LE_CONN_SRV_CMD_CHANGE_UL_PATH, cis_handle);

    bt_ull_le_activate_uplink_t params;
    params.cis_connection_handle = cis_handle;
    status = bt_ull_le_activate_uplink(&params);
    if (status != BT_STATUS_SUCCESS) {
        bt_ull_le_conn_srv_unlock_cmd(BT_ULL_LE_CONN_SRV_CMD_CHANGE_UL_PATH);
    }
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_activate_uplink, status: 0x%x, cis handle: 0x%x, acl handle: 0x%x", 3, status, cis_handle, handle);
#endif
    return status;

}
bt_status_t bt_ull_le_conn_srv_activiate_air_cis(bt_handle_t handle)
{
    bt_handle_t cis_handle;
    bt_status_t status = BT_STATUS_SUCCESS;
    uint8_t i;
    bt_ull_client_t ct = bt_ull_le_conn_srv_get_client_type();
#ifdef BT_ULL_LE_CONN_SRV_DEBUG
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_activiate_air_cis, ct: 0x%x", 1, ct);
#endif
    if (ct == BT_ULL_HEADSET_CLIENT) {
        for (i = 0; i < BT_ULL_LE_AIR_CIS_MAX_NUM; i ++) {
            if (bt_ull_le_conn_srv_cis_is_connected(i) && bt_ull_le_conn_srv_get_acl_handle(i) == handle) {
                cis_handle = bt_ull_le_conn_srv_get_cis_handle(i);
                status = bt_ull_le_conn_srv_setup_air_iso_data_path_internal(cis_handle);
            }
        }
    } else {
        uint8_t idx = bt_ull_le_conn_srv_get_idx_by_acl_handle(handle);
        if (BT_ULL_LE_CONN_SRV_INVALID_IDX != idx) {
            status = bt_ull_le_conn_srv_setup_air_iso_data_path_internal(bt_ull_le_conn_srv_get_cis_handle(idx));
        } else {
            ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_activiate_air_cis, invalid idx!", 0);
            status = BT_STATUS_FAIL;
        }
    }
    return status;

}

bt_status_t bt_ull_le_conn_srv_deactivate_air_cis(bt_handle_t handle)
{
    bt_handle_t cis_handle;
    bt_status_t status = BT_STATUS_SUCCESS;
    uint8_t i;
    bt_ull_client_t ct = bt_ull_le_conn_srv_get_client_type();
#ifdef BT_ULL_LE_CONN_SRV_DEBUG
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_deactivate_air_cis, ct: 0x%x", 1, ct);
#endif
    if (ct == BT_ULL_HEADSET_CLIENT) {
        for (i = 0; i < BT_ULL_LE_AIR_CIS_MAX_NUM; i ++) {
            if (bt_ull_le_conn_srv_cis_is_connected(i) && bt_ull_le_conn_srv_get_acl_handle(i) == handle) {
                cis_handle = bt_ull_le_conn_srv_get_cis_handle(i);
                status = bt_ull_le_conn_srv_remove_air_iso_data_path_internal(cis_handle);
            }
        }
    } else {
        uint8_t idx = bt_ull_le_conn_srv_get_idx_by_acl_handle(handle);
        if (BT_ULL_LE_CONN_SRV_INVALID_IDX != idx) {
            status = bt_ull_le_conn_srv_remove_air_iso_data_path_internal(bt_ull_le_conn_srv_get_cis_handle(idx));
        } else {
            ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_deactivate_air_cis, invalid idx!", 0);
            status = BT_STATUS_FAIL;
        }
    }
    return status;

}

bt_status_t bt_ull_le_conn_srv_set_air_cig_params_table(bt_handle_t acl_handle, uint8_t cis_count, bt_ull_le_codec_t codec)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    uint8_t i = 0;

    uint8_t count = 0;
    bt_ull_le_conn_srv_air_cig_param_t *select_cig_params = NULL;
    bt_ull_client_t ct = bt_ull_le_conn_srv_get_client_type();
    //bt_ull_le_codec_param_t *ul_codec_param = bt_ull_le_srv_get_codec_param(BT_ULL_ROLE_SERVER, BT_ULL_GAMING_TRANSMITTER);
    //bt_ull_le_codec_param_t *dl_codec_param = bt_ull_le_srv_get_codec_param(BT_ULL_ROLE_SERVER, BT_ULL_MIC_TRANSMITTER);
    bt_ull_le_srv_capability_t *capability = bt_ull_le_srv_get_peer_capability(acl_handle);
    if (!capability) {
        ull_report_error("[ULL][LE][CONN] No Capability!", 0);
        return BT_STATUS_FAIL;
    }
    bt_ull_le_conn_srv_context_t *ctx = bt_ull_le_conn_srv_get_ctx();
    uint32_t ul_samplerate = bt_ull_le_srv_get_ul_sample_rate();
    uint32_t dl_samplerate = bt_ull_le_srv_get_dl_sample_rate();
    uint8_t aud_quality = bt_ull_le_conn_srv_get_audio_quality();
#ifdef BT_ULL_LE_CONN_SRV_DEBUG
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_set_air_cig_params_table, handle: 0x%x, ct: 0x%x, cis: %d, capability: %x, table: %x, codec: %d, ul samp: %d, dl samp: %d, aud_quality: %d", 9, \
        acl_handle, ct, cis_count, *capability, ctx->aud_ctrl.select_cig_params, codec, ul_samplerate, dl_samplerate, aud_quality);
#endif
    if (bt_ull_le_conn_srv_cmd_is_lock(BT_ULL_LE_CONN_SRV_CMD_SET_TABLE)) {
        bt_ull_le_conn_srv_cmd_cache_item_t *cache_cmd = bt_ull_le_conn_srv_new_cache_cmd_node(BT_ULL_LE_CONN_SRV_CMD_SET_TABLE);
        cache_cmd->handle = acl_handle;
        ull_report_error("[ULL][LE][CONN] bt_ull_le_conn_srv_set_air_cig_params_table, set tabel busy!", 0);
        return BT_STATUS_PENDING;
    }
    bt_ull_le_conn_srv_lock_cmd(BT_ULL_LE_CONN_SRV_CMD_SET_TABLE, acl_handle);

    if (BT_ULL_LE_CODEC_LC3PLUS == codec) {
        if (BT_ULL_MIC_CLIENT == ct) {
            ctx->aud_ctrl.select_cig_params = g_cig_param_table_lc3plus_wireless_mic;
            ctx->aud_ctrl.label_count = 0x01;
        } else {
            if (capability->dchs_support) {
                if (BT_ULL_LE_DEFAULT_SAMPLERATE_96K == dl_samplerate) {
                    if (BT_ULL_LE_DEFAULT_SAMPLERATE_48K == ul_samplerate) { /*support*/
                        ctx->aud_ctrl.select_cig_params = g_cig_param_table_lc3plus_highres;
                        ctx->aud_ctrl.label_count = 0x04;
                    } else {
                        ctx->aud_ctrl.select_cig_params = g_cig_param_table_lc3plus_defualt;
                        ctx->aud_ctrl.label_count = 0x04;
                    }
                } else if (BT_ULL_LE_DEFAULT_SAMPLERATE_48K == dl_samplerate) {
                    ctx->aud_ctrl.select_cig_params = g_cig_param_table_lc3plus_dchs;
                    ctx->aud_ctrl.label_count = 0x01;
                }
            } else {

                if (BT_ULL_LE_SRV_AUDIO_QUALITY_LOW_POWER == aud_quality) {
                    ctx->aud_ctrl.select_cig_params = g_cig_param_table_lc3plus_low_power;
                    ctx->aud_ctrl.label_count = 0x02;
                } else {
                    if (BT_ULL_LE_DEFAULT_SAMPLERATE_48K == ul_samplerate) { /*support*/
	                    ctx->aud_ctrl.select_cig_params = g_cig_param_table_lc3plus_highres;
	                    ctx->aud_ctrl.label_count = 0x04;
		            } else {
		                ctx->aud_ctrl.select_cig_params = g_cig_param_table_lc3plus_defualt;
		                ctx->aud_ctrl.label_count = 0x04;
		            }
            }
        }
        }
    } else if (BT_ULL_LE_CODEC_OPUS == codec) {
        ctx->aud_ctrl.select_cig_params = g_cig_param_table_opus;
        ctx->aud_ctrl.label_count = 0x01;
    } else if (BT_ULL_LE_CODEC_ULD ==  codec) {
        if (BT_ULL_MIC_CLIENT == ct) {
            ctx->aud_ctrl.select_cig_params = g_cig_param_table_uld_wireless_mic;
            ctx->aud_ctrl.label_count = 0x01;
        } else {
            ctx->aud_ctrl.select_cig_params = g_cig_param_table_uld_default;
            ctx->aud_ctrl.label_count = 0x01;
        }
    } else if (BT_ULL_LE_CODEC_DL_ULD_UL_LC3PLUS == codec) {
            ctx->aud_ctrl.select_cig_params = g_bt_ull_cig_param_table_uld_ull3;
            ctx->aud_ctrl.label_count = 0x01;
    }
#ifdef AIR_AUDIO_VEND_CODEC_ENABLE
    else if (BT_ULL_LE_CODEC_DL_ULD_UL_OPUS == codec) {
            g_bt_ull_cig_param_table_uld_ull3[0].codec = BT_ULL_LE_CODEC_DL_ULD_UL_OPUS;
            ctx->aud_ctrl.select_cig_params = g_bt_ull_cig_param_table_uld_ull3;
            ctx->aud_ctrl.label_count = 0x01;
    }
#endif
    else {
        ull_report_error("[ULL][LE][CONN] ERROR CODEC!", 0);
        bt_ull_le_conn_srv_unlock_cmd(BT_ULL_LE_CONN_SRV_CMD_SET_TABLE);
        return BT_STATUS_FAIL;
    }
    ull_report("[ULL][LE][CONN][TABLE] --Aud Quality--Codec--DL Samp--UL Samp--DL BR --UL BR", 0);
    for (i = 0; i < ctx->aud_ctrl.label_count; i ++) {
        ull_report("[ULL][LE][CONN][TABLE] --    %d     --  %d  -- %d -- %d --%d--%d", 6, \
            ctx->aud_ctrl.select_cig_params[i].audio_quality,
            codec,
            dl_samplerate,
            ul_samplerate,
            ctx->aud_ctrl.select_cig_params[i].dl_bitrate,
            ctx->aud_ctrl.select_cig_params[i].ul_bitrate);
    }
    bt_ull_le_conn_srv_set_codec_type(codec);
    bt_ull_le_conn_srv_set_cis_count(cis_count);
    bt_ull_le_set_air_label_params_table_t param;
    uint16_t cis_list_len = sizeof(bt_ull_le_air_cis_params_t) * cis_count; 
    uint16_t label_param_len = sizeof(bt_ull_le_air_label_params_t) - sizeof(param.label_param_list) + cis_list_len;
    uint8_t *table = bt_ull_le_srv_memory_alloc(label_param_len * ctx->aud_ctrl.label_count);
    bt_ull_le_air_cis_params_t *cis_list = bt_ull_le_srv_memory_alloc(cis_list_len);

    if (!table || !cis_list) {
        ull_assert(0 && "[ULL][LE][CONN] out of memory!");
        bt_ull_le_conn_srv_unlock_cmd(BT_ULL_LE_CONN_SRV_CMD_SET_TABLE);
        return BT_STATUS_OUT_OF_MEMORY;        
    } else {
        bt_ull_le_srv_memset(table, 0, label_param_len * ctx->aud_ctrl.label_count);
        bt_ull_le_srv_memset(cis_list, 0, cis_list_len);
    }


    uint32_t offset = 0;
    bt_ull_le_air_label_params_t *table_str = NULL;
    uint32_t m2s_sdu_interval = bt_ull_le_srv_get_sdu_interval(false, bt_ull_le_conn_srv_get_role());
    uint32_t s2m_sdu_interval = bt_ull_le_srv_get_sdu_interval(true, bt_ull_le_conn_srv_get_role());
    for (i = 0; i < ctx->aud_ctrl.label_count; i ++) {
        offset = (label_param_len * i);
        table_str = (bt_ull_le_air_label_params_t*)(table + offset);
        table_str->sdu_interval_m_to_s = m2s_sdu_interval;
        table_str->sdu_interval_s_to_m = s2m_sdu_interval;
        select_cig_params = &ctx->aud_ctrl.select_cig_params[i];
        if (!select_cig_params) {
            assert(0);
        }
        table_str->max_share_count = select_cig_params->max_share_num;
        table_str->iso_interval = select_cig_params->iso_interval;
        if (BT_ULL_SPEAKER_CLIENT == ct)
            table_str->max_uplink_num = 0x00;
        else if (BT_ULL_MIC_CLIENT == ct || BT_ULL_ULD_MIC_CLIENT == ct)
            table_str->max_uplink_num = cis_count;
        else
            table_str->max_uplink_num = select_cig_params->max_ul_num;
        table_str->cis_count = cis_count;
        if (ct == BT_ULL_HEADSET_CLIENT) {
            for (count = 0; count < cis_count; count ++) {
                cis_list[count].cis_id = g_bt_ull_cis_id_table[count].cis_id;
                cis_list[count].max_sdu_m_to_s = ((select_cig_params->dl_bitrate / 100) * (m2s_sdu_interval / 10)) / 8 / 1000 / 2;

                if (cis_list[count].cis_id == BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK2) { /* only heaset have only one bidirection  cis (cis id 2-->call only dl)*/
                    cis_list[count].max_sdu_s_to_m = 0x0000;
                } else {
                    cis_list[count].max_sdu_s_to_m = ((select_cig_params->ul_bitrate / 100) * (s2m_sdu_interval / 10)) / 8 / 1000;
                }
                cis_list[count].phy_m_to_s = select_cig_params->phy_m_2_s;
                cis_list[count].phy_s_to_m = select_cig_params->phy_s_2_m;
            }
    
        } else if (ct == BT_ULL_EARBUDS_CLIENT) {
            for (count = 0; count < cis_count; count ++) {
                cis_list[count].cis_id = g_bt_ull_cis_id_table[count].cis_id;
                cis_list[count].max_sdu_m_to_s = ((select_cig_params->dl_bitrate / 100) * (m2s_sdu_interval / 10)) / 8 / 1000 / 2;
                cis_list[count].max_sdu_s_to_m = ((select_cig_params->ul_bitrate / 100) * (s2m_sdu_interval / 10)) / 8 / 1000;
                cis_list[count].phy_m_to_s = select_cig_params->phy_m_2_s;
                cis_list[count].phy_s_to_m = select_cig_params->phy_s_2_m;
    
            }
        } else if (ct == BT_ULL_MIC_CLIENT) {
            for (count = 0; count < cis_count; count ++) {
                cis_list[count].cis_id = g_bt_ull_mic_cis_id_table[count].cis_id;
                cis_list[count].max_sdu_m_to_s = 0x0000;
                cis_list[count].max_sdu_s_to_m = ((select_cig_params->ul_bitrate / 100)* (s2m_sdu_interval / 10)) / 8 / 1000;
                cis_list[count].phy_m_to_s = select_cig_params->phy_m_2_s;
                cis_list[count].phy_s_to_m = select_cig_params->phy_s_2_m;
            }
        } else if (ct == BT_ULL_SPEAKER_CLIENT) {
            for (count = 0; count < cis_count; count ++) {
                cis_list[count].cis_id = g_bt_ull_spk_cis_id_table[count].cis_id;
                cis_list[count].max_sdu_m_to_s = ((select_cig_params->dl_bitrate / 100)* (m2s_sdu_interval / 10)) / 8 / 1000 / 2;
                cis_list[count].max_sdu_s_to_m = 0x00;
                cis_list[count].phy_m_to_s = select_cig_params->phy_m_2_s;
                cis_list[count].phy_s_to_m = select_cig_params->phy_s_2_m;
            }
        }

        bt_ull_le_srv_memcpy(&table_str->cis_list, cis_list, cis_list_len);
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
        ull_report("[ULL][LE][CONN][VENDOR CMD] vendor set air cig table[%d], sdu_interval_m_to_s: 0x%x, sdu_interval_s_to_m: 0x%x, iso_interval: 0x%x, max_uplink_num: 0x%x, max_share_count: 0x%x, cis_count: 0x%x, dl_br: %d, ul_br:%d", 9, \
                   i,
                   table_str->sdu_interval_m_to_s,
                   table_str->sdu_interval_s_to_m,
                   table_str->iso_interval,
                   table_str->max_uplink_num,
                   table_str->max_share_count,
                   table_str->cis_count,
                   select_cig_params->dl_bitrate,
                   select_cig_params->ul_bitrate
                  );
        for (count = 0; count < cis_count; count ++) {
            ull_report("[ULL][LE][CONN][VENDOR CMD] vendor set air cig cmd-cis_list[%d], cis_id: %d, max_sdu_m_to_s: 0x%x, max_sdu_s_to_m: 0x%x, phy_m_to_s: 0x%x, phy_s_to_m: 0x%x", 6, \
                       count,
                       cis_list[count].cis_id,
                       cis_list[count].max_sdu_m_to_s,
                       cis_list[count].max_sdu_s_to_m,
                       cis_list[count].phy_m_to_s,
                       cis_list[count].phy_s_to_m
                      );
        }
#endif
        
    }
    for (i = 0; i < BT_ULL_LE_FT_LABEL_MAX; i ++) {
        param.ft_label[i] = g_bt_ull_latency_ft_table[i].ft_m_to_s;
    }

    param.connection_handle = acl_handle;
    if (BT_ULL_LE_CODEC_ULD == codec) {
        param.client_type = ct + 1;
    } else {
    param.client_type = ct;
    }
    if (BT_ULL_LE_SRV_AUDIO_QUALITY_LOW_POWER == aud_quality) {
        param.dvfs_level = BT_ULL_LE_LEVEL_DVFS_LV;
    } else {
        param.dvfs_level = BT_ULL_LE_LEVEL_DVFS_HV;
    }
    param.label_count = ctx->aud_ctrl.label_count;
    param.label_param_list = (bt_ull_le_air_label_params_t*)table;
    status = bt_ull_le_set_air_cig_params_table(&param);
#ifdef BT_ULL_LE_CONN_SRV_CMD_DEBUG
    ull_report("[ULL][LE][CONN][VENDOR CMD] vendor set air params table cmd, status: 0x%x, handle: 0x%x, ct: %d, count: %d", 4, \
            status,
            param.connection_handle,
            param.client_type,
            param.label_count
            );
#endif

    bt_ull_le_srv_memory_free(table);
    bt_ull_le_srv_memory_free(cis_list);
    if (status != BT_STATUS_SUCCESS) {
        bt_ull_le_conn_srv_unlock_cmd(BT_ULL_LE_CONN_SRV_CMD_SET_TABLE);
    }
    return status;
}

static bt_status_t bt_ull_le_conn_srv_next_action_flow_handler(uint8_t next_action, void *data)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_next_action_flow_handler, next action: %d", 1, next_action);
    switch (next_action) {
    case BT_ULL_LE_CONN_SRV_ACTION_ESTABLISH_AIR_CIS: {
            status = bt_ull_le_conn_srv_establish_air_cis(*(bt_handle_t *)data);
            break;
        }

    case BT_ULL_LE_CONN_SRV_ACTION_SETUP_ISO_DATA_PATH: {
            status = bt_ull_le_conn_srv_setup_air_iso_data_path_internal(*(bt_handle_t *)data);
            break;
        }

    case BT_ULL_LE_CONN_SRV_ACTION_REMOVE_ISO_DATA_PATH: {
            status = bt_ull_le_conn_srv_remove_air_iso_data_path_internal(*(bt_handle_t *)data);
            break;
        }

    case BT_ULL_LE_CONN_SRV_ACTION_DESTROY_AIR_CIS: {
            status = bt_ull_le_conn_srv_destroy_air_cis_internal(*(bt_handle_t *)data);
            break;
        }

    case BT_ULL_LE_CONN_SRV_ACTION_SET_AIR_CIG_PARAMS: {
            status = bt_ull_le_conn_srv_set_air_cig_params(bt_ull_le_conn_srv_get_cis_count());
            break;
        }

    case BT_ULL_LE_CONN_SRV_ACTION_REMOVE_AIR_CIG_PARAMS: {
            status = bt_ull_le_conn_srv_remove_air_cig_params();
            break;
        }
    case BT_ULL_LE_CONN_SRV_ACTION_UNMUTE_AIR_CIS: {
            status = bt_ull_le_conn_srv_unmute_air_cis_internal(*(bt_handle_t *)data);
            break;
        }
    case BT_ULL_LE_CONN_SRV_ACTION_ENABLE_UPLINK: {
            status = bt_ull_le_conn_srv_activate_uplink(*(bt_handle_t *)data);
        }
    default:
        break;

    }

    return status;
}

static bt_ull_le_conn_srv_cmd_cache_item_t* bt_ull_le_conn_srv_new_cache_cmd_node(bt_ull_le_conn_srv_cmd_enum cmd)
{
    bt_ull_le_conn_srv_cache_cmd_node_t *node;
    bt_ull_le_conn_srv_cache_cmd_node_t *new_node;
    bt_ull_le_conn_srv_cache_cmd_node_t *last_node;
#ifdef BT_ULL_LE_CONN_SRV_DEBUG
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_new_cache_cmd_node, type: 0x%x", 1, cmd);
#endif
    new_node = (bt_ull_le_conn_srv_cache_cmd_node_t *)bt_ull_le_srv_memory_alloc(sizeof(bt_ull_le_conn_srv_cache_cmd_node_t));
    if (NULL == new_node) {
        return NULL;
    }
    bt_ull_le_srv_memset(new_node, 0, sizeof(bt_ull_le_conn_srv_cache_cmd_node_t));
    new_node->next = NULL;
    new_node->cache_item.cmd = cmd;

    node = g_bt_ull_le_conn_ctrl.cis_cmd_list_header;
    last_node = node;
    if (node == NULL) {
        g_bt_ull_le_conn_ctrl.cis_cmd_list_header = new_node;
    } else {
        while (node) {
            last_node = node;
            node = (bt_ull_le_conn_srv_cache_cmd_node_t *)node->next;
        }
        last_node->next = new_node;
    }
    return &new_node->cache_item;
}

static uint8_t bt_ull_le_conn_srv_delete_cache_cmd_node(bt_ull_le_conn_srv_cache_cmd_node_t *delete_node)
{
    if (delete_node == NULL) {
        return true;
    }
#ifdef BT_ULL_LE_CONN_SRV_DEBUG
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_delete_cache_cmd_node, node addr: 0x%x", 1, delete_node);
#endif
    bt_ull_le_conn_srv_cache_cmd_node_t *header = g_bt_ull_le_conn_ctrl.cis_cmd_list_header;
    bt_ull_le_conn_srv_cache_cmd_node_t *node = header;
    bt_ull_le_conn_srv_cache_cmd_node_t *prv_node = header;
    while (node) {
        if (node == delete_node) {
            if (node == header) {
                g_bt_ull_le_conn_ctrl.cis_cmd_list_header = (bt_ull_le_conn_srv_cache_cmd_node_t *)node->next;
            } else {
                prv_node->next = node->next;
            }
            bt_ull_le_srv_memory_free(node);
            return true;
        }
        prv_node = node;
        node = (bt_ull_le_conn_srv_cache_cmd_node_t *)node->next;
    }
    return false;

}

static void bt_ull_le_conn_srv_delete_all_cmd_node(void)
{
    bt_ull_le_conn_srv_cache_cmd_node_t *header = g_bt_ull_le_conn_ctrl.cis_cmd_list_header;
    bt_ull_le_conn_srv_cache_cmd_node_t *node = header;
    bt_ull_le_conn_srv_cache_cmd_node_t *next_node = node;
    if (NULL == header) {
        return;
    }
    while (node) {
        next_node = node->next;
        bt_ull_le_srv_memory_free(node);
        node = next_node;
    }
    g_bt_ull_le_conn_ctrl.cis_cmd_list_header = NULL;
}

static bt_ull_le_conn_srv_cache_cmd_node_t* bt_ull_le_conn_srv_search_cache_cmd_node_by_type(bt_ull_le_conn_srv_cmd_enum cmd)
{
#ifdef BT_ULL_LE_CONN_SRV_DEBUG
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_search_cache_cmd_node_by_type, type: 0x%x", 1, cmd);
#endif

    bt_ull_le_conn_srv_cache_cmd_node_t *node;
    node = g_bt_ull_le_conn_ctrl.cis_cmd_list_header;
    while (node) {
        if (node->cache_item.cmd == cmd) {
            return node;
        }
        node = (bt_ull_le_conn_srv_cache_cmd_node_t *)node->next;
    }
    return NULL;
}

bt_ull_le_conn_srv_cache_cmd_node_t* bt_ull_le_conn_srv_search_cache_cmd_by_handle(bt_ull_le_conn_srv_cmd_enum cmd, bt_handle_t handle)
{
    bt_ull_le_conn_srv_cache_cmd_node_t *node;
    node = g_bt_ull_le_conn_ctrl.cis_cmd_list_header;
    while (node) {
        if (node->cache_item.handle == handle && node->cache_item.cmd == cmd) {
            return node;
        }
        node = (bt_ull_le_conn_srv_cache_cmd_node_t *)node->next;
    }
    return NULL;
}

void bt_ull_le_conn_srv_delete_all_cache_cmd_by_handle(bt_handle_t acl_handle)
{
    if (BT_ULL_HEADSET_CLIENT == bt_ull_le_conn_srv_get_client_type()) {
        bt_ull_le_conn_srv_delete_all_cmd_node();
    } else {
        uint8_t idx = bt_ull_le_conn_srv_get_idx_by_acl_handle(acl_handle);
        if (BT_ULL_LE_CONN_SRV_INVALID_IDX == idx) {
            return;
        }
        bt_handle_t cis_handle = bt_ull_le_conn_srv_get_cis_handle(idx);

        bt_ull_le_conn_srv_cache_cmd_node_t *header = g_bt_ull_le_conn_ctrl.cis_cmd_list_header;
        bt_ull_le_conn_srv_cache_cmd_node_t *node = header;
        bt_ull_le_conn_srv_cache_cmd_node_t *prv_node = header;
        while (node) {
            if (node->cache_item.handle == cis_handle) {
                ull_report("[ULL][LE][CONN] Delete cache CMD: 0x%x", 1, node->cache_item.cmd);
                if (node == header) {
                    g_bt_ull_le_conn_ctrl.cis_cmd_list_header = (bt_ull_le_conn_srv_cache_cmd_node_t *)node->next;
                    node = g_bt_ull_le_conn_ctrl.cis_cmd_list_header;
                    prv_node = node;
                } else {
                    prv_node->next = node->next;
                    bt_ull_le_srv_memory_free(node);
                    node = prv_node->next;
                }
            } else {
                prv_node = node;
                node = (bt_ull_le_conn_srv_cache_cmd_node_t *)node->next;
            }
        }
    }
    return;
}
static void bt_ull_le_conn_srv_lock_cmd(bt_ull_le_conn_srv_cmd_enum cmd, bt_handle_t handle)
{
    uint8_t i = 0;
#ifdef BT_ULL_LE_CONN_SRV_DEBUG
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_lock_cmd, type: 0x%x", 1, cmd);
#endif
    for (i = 0; i < BT_ULL_LE_CONN_SRV_CMD_MAX ; i ++) {
        if (g_bt_ull_cmd_lock_table[i].cmd == cmd) {
            g_bt_ull_cmd_lock_table[i].lock = handle;
        }
    }
}

static void bt_ull_le_conn_srv_unlock_cmd(bt_ull_le_conn_srv_cmd_enum cmd)
{
    uint8_t i = 0;
#ifdef BT_ULL_LE_CONN_SRV_DEBUG
    ull_report("[ULL][LE][CONN] bt_ull_le_conn_srv_unlock_cmd, type: 0x%x", 1, cmd);
#endif
    for (i = 0; i < BT_ULL_LE_CONN_SRV_CMD_MAX ; i ++) {
        if (g_bt_ull_cmd_lock_table[i].cmd == cmd) {
            g_bt_ull_cmd_lock_table[i].lock = 0;
        }
    }
}

static uint16_t bt_ull_le_conn_srv_cmd_is_lock(bt_ull_le_conn_srv_cmd_enum cmd)
{
    uint8_t i = 0;
    for (i = 0; i < BT_ULL_LE_CONN_SRV_CMD_MAX ; i ++) {
        if (g_bt_ull_cmd_lock_table[i].cmd == cmd) {
            return g_bt_ull_cmd_lock_table[i].lock;
        }
    }
    return false;
}

uint8_t bt_ull_le_conn_srv_get_cis_id_by_location(bt_ull_le_audio_location_t location)
{
    uint8_t i = 0;
    bt_ull_client_t ct = bt_ull_le_conn_srv_get_client_type();
    for (i = 0; i < BT_ULL_LE_AIR_CIS_MAX_NUM; i ++) {
        if (BT_ULL_MIC_CLIENT == ct) {
            if (location == g_bt_ull_mic_cis_id_table[i].audio_location) {
                return g_bt_ull_mic_cis_id_table[i].cis_id;
            }
        } else if (BT_ULL_HEADSET_CLIENT == ct || BT_ULL_EARBUDS_CLIENT == ct) {
            if (location == g_bt_ull_cis_id_table[i].audio_location) {
                return g_bt_ull_cis_id_table[i].cis_id;
            }
        } else if (BT_ULL_SPEAKER_CLIENT == ct) {
            if (location == g_bt_ull_spk_cis_id_table[i].audio_location) {
                return g_bt_ull_spk_cis_id_table[i].cis_id;
            }
        }
    }
    ull_report_error("[ULL][LE][CONN] ivalid location or ct, location: %x, ct: %d!", 2, location, ct);
    return BT_ULL_LE_CONN_SRV_AIR_CIS_ID_INVANLID;
}
static uint8_t bt_ull_le_conn_srv_get_idx_by_location(bt_ull_le_audio_location_t location)
{
    uint8_t i = 0;
    bt_ull_client_t ct = bt_ull_le_conn_srv_get_client_type();
    for (i = 0; i < BT_ULL_LE_AIR_CIS_MAX_NUM; i ++) {
        if (BT_ULL_MIC_CLIENT == ct) {
            if (location == g_bt_ull_mic_cis_id_table[i].audio_location) {
                return i;
            }
        } else if (BT_ULL_HEADSET_CLIENT == ct || BT_ULL_EARBUDS_CLIENT == ct) {
            if (location == g_bt_ull_cis_id_table[i].audio_location) {
                return i;
            }
        } else if (BT_ULL_SPEAKER_CLIENT == ct) {
            if (location == g_bt_ull_spk_cis_id_table[i].audio_location) {
                return i;
            }
        }
    }
    ull_report_error("[ULL][LE][CONN] ivalid location or ct, location: %x, ct: %d!", 2, location, ct);
    return BT_ULL_LE_CONN_SRV_INVALID_IDX;


}
#ifdef AIR_WIRELESS_MIC_RX_ENABLE
uint8_t bt_ull_conn_srv_get_data_path_id_by_acl_handle(bt_handle_t handle)
{
    uint8_t cis_id;
    bt_ull_client_t ct = bt_ull_le_conn_srv_get_client_type();
    uint8_t idx = bt_ull_le_conn_srv_get_idx_by_acl_handle(handle);
    if (BT_ULL_LE_CONN_SRV_INVALID_IDX == idx) {
        return BT_ULL_LE_CONN_SRV_INVALID_IDX;
    }
    cis_id = bt_ull_le_conn_srv_get_cis_id(idx);
    return bt_ull_le_conn_srv_get_data_path_id(cis_id, ct);
}
#endif


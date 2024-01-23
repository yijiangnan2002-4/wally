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

#ifndef __BT_ULL_LE_CONN_SRV_H__
#define __BT_ULL_LE_CONN_SRV_H__

#include "bt_ull_le.h"
#include "bt_ull_utility.h"
#include "bt_ull_le_utility.h"

#define BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_ESTABLISHED_IND        0x00
#define BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_DESTROYED_IND          0x01
#define BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_ACTIVATED_IND          0x02
#define BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_DEACTIVIATED_IND       0x03
#define BT_ULL_LE_CONN_SRV_EVENT_AIR_CIG_PARAMS_SET_DONE_IND    0x04
#define BT_ULL_LE_CONN_SRV_EVENT_AIR_CIG_PARAMS_REMOVED_IND     0x05
#define BT_ULL_LE_CONN_SRV_EVENT_LABEL_CHANGED_IND              0x06
#define BT_ULL_LE_CONN_SRV_EVENT_UPLINK_ENABLED_IND             0x07
#define BT_ULL_LE_CONN_SRV_EVENT_AIR_CIG_TABLE_SET_IND          0x08
#define BT_ULL_LE_CONN_SRV_EVENT_QOS_REPORT_IND                 0x09
#define BT_ULL_LE_CONN_SRV_EVENT_MAX                            0xFF
typedef uint8_t bt_ull_le_conn_srv_event_t;

#define BT_ULL_LE_CONN_SRV_STREAM_MODE_NONE               0x00
#define BT_ULL_LE_CONN_SRV_STREAM_MEDIA_MODE              0x01
#define BT_ULL_LE_CONN_SRV_STREAM_CALL_MODE               0x02
#define BT_ULL_LE_CONN_SRV_STREAM_MIC_MODE                0x03
typedef uint8_t bt_ull_le_conn_srv_stream_mode_t;

typedef bt_ull_le_srv_phy_t bt_ull_le_conn_srv_phy_t;

typedef void (*bt_ull_le_conn_callback_t)(bt_ull_le_conn_srv_event_t event, void *data);

//typedef bt_avm_leaudio_buffer_info_t bt_ull_le_conn_srv_avm_buffer_info_t;

/**
 * @brief       BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_ESTABLISHED_IND msg event.
 */
typedef struct {
    bt_handle_t handle;                                   /*ACL connection handle*/
    bool        dl_enable;                              /*Downlink in cis of the acl is active or inactive*/
    bool        ul_enable;                              /*Uplink in cis of the acl is active or inactive*/
    bt_ull_le_srv_latency_t           latency;          /*new latency*/
    bt_ull_le_srv_audio_quality_t     audio_quility;

} bt_ull_le_conn_srv_established_evt_t;

/**
 * @brief       BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_DESTROYED_IND or  BT_ULL_LE_CONN_SRV_EVENT_AIR_CIG_TABLE_SET_IND msg event.
 */
typedef struct {
    bt_handle_t handle;                            /*ACL connection handle*/
} bt_ull_le_conn_srv_air_cig_table_set_t;

/**
 * @brief       BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_DESTROYED_IND  msg event.
 */
typedef struct {
    bt_handle_t handle;                            /*ACL connection handle*/
    bt_hci_disconnect_reason_t reason;             /*Disconnect reasson*/
} bt_ull_le_conn_srv_air_cis_destroied_evt_t;


/**
 * @brief       BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_ACTIVATED_IND msg event.
 */
typedef struct {
    bt_handle_t handle;                            /*ACL connection handle*/
    bool active_state;                              /*streaming active state*/
} bt_ull_le_conn_srv_air_cis_activiated_evt_t;


/**
 * @brief       BT_ULL_LE_CONN_SRV_EVENT_UPLINK_ENABLED_IND msg event.
 */
typedef struct {
    bt_handle_t handle;                            /*ACL connection handle*/
    bool        ul_enable;                             /*Mic in cis of the acl is active or inactive*/
} bt_ull_le_conn_srv_uplink_enabled_evt_t;

typedef struct {
    bt_ull_le_srv_latency_t           latency;                            /*new latency*/
    bt_ull_le_srv_audio_quality_t     audio_quility;
} bt_ull_le_conn_srv_latency_changed_t;

typedef struct {
    bt_handle_t                       handle;
    uint16_t                          crc;
    uint16_t                          rx_to;                             /* Rx Timeout*/
    uint16_t                          flush_to;                          /*Flush Timeout*/
} bt_ull_le_conn_srv_qos_report_t;

typedef struct {
    bt_status_t status;                                                       /*BT_ULL_LE_CONN_SRV_EVENT_AIR_CIG_PARAMS_REMOVED_IND and BT_ULL_LE_CONN_SRV_EVENT_AIR_CIG_REMOVED_IND only have status*/
    union {
        bt_ull_le_conn_srv_established_evt_t                 established;     /*BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_ESTABLISHED_IND*/
        bt_ull_le_conn_srv_uplink_enabled_evt_t              ul_enabled;      /*BT_ULL_LE_CONN_SRV_EVENT_UPLINK_ENABLED_IND*/
        bt_ull_le_conn_srv_air_cis_destroied_evt_t           destroied;       /*BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_DESTROYED_IND*/
        bt_ull_le_conn_srv_air_cis_activiated_evt_t          cis_activiated;  /*BT_ULL_LE_CONN_SRV_EVENT_AIR_CIS_ACTIVATED_IND*/
        bt_ull_le_conn_srv_latency_changed_t                 label_changed;   /*BT_ULL_LE_CONN_SRV_EVENT_LABEL_CHANGED_IND*/
        bt_ull_le_conn_srv_air_cig_table_set_t               cig_set;         /*BT_ULL_LE_CONN_SRV_EVENT_AIR_CIG_TABLE_SET_IND*/
        bt_ull_le_conn_srv_qos_report_t                      qos_report;      /*BT_ULL_LE_CONN_SRV_EVENT_QOS_REPORT_IND*/
    };
} bt_ull_le_conn_srv_air_cis_evt_ind_t;

bt_status_t bt_ull_le_conn_srv_init(bt_ull_role_t role, bt_ull_le_conn_callback_t callback);
bt_status_t bt_ull_le_conn_srv_deinit(void);
bt_status_t bt_ull_le_conn_srv_set_air_cig_params(uint8_t cis_count);
bt_status_t bt_ull_le_conn_srv_remove_air_cig_params(void);
bt_status_t bt_ull_le_conn_srv_establish_air_cis(bt_handle_t handle);
bt_status_t bt_ull_le_conn_srv_destroy_air_cis(bt_handle_t handle);
bt_status_t bt_ull_le_conn_srv_activiate_air_cis(bt_handle_t handle);
bt_status_t bt_ull_le_conn_srv_deactivate_air_cis(bt_handle_t handle);
bt_status_t bt_ull_le_conn_srv_unmute_air_cis(bt_handle_t handle);
bt_status_t bt_ull_le_conn_srv_switch_latency(bt_ull_le_srv_latency_t latency);
bt_status_t bt_ull_le_conn_srv_change_audio_quality(bt_ull_le_srv_audio_quality_t quality);
bt_status_t bt_ull_le_conn_srv_activate_uplink(bt_handle_t handle); /*for client type is earbuds*/
uint8_t bt_ull_le_conn_srv_get_cis_id_by_location(bt_ull_le_audio_location_t location);
bt_status_t bt_ull_le_conn_srv_set_air_cig_params_table(bt_handle_t acl_handle, uint8_t cis_count, bt_ull_le_codec_t codec);
void bt_ull_le_conn_srv_set_latency(bt_ull_le_srv_latency_t latency);
void bt_ull_le_conn_srv_set_audio_quality(bt_ull_le_srv_audio_quality_t quality);

#ifdef AIR_WIRELESS_MIC_RX_ENABLE
uint8_t bt_ull_conn_srv_get_data_path_id_by_acl_handle(bt_handle_t handle);
#endif
void bt_ull_le_conn_srv_delete_all_cache_cmd_by_handle(bt_handle_t acl_handle);
uint8_t bt_ull_le_conn_srv_get_avaliable_cis_idx(void);


#endif

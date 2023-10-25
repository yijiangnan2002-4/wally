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

#ifndef __APP_LE_AUDIO_CSIP_SET_COORDINATOR_H__
#define __APP_LE_AUDIO_CSIP_SET_COORDINATOR_H__

#ifdef AIR_LE_AUDIO_ENABLE
#include "FreeRTOS.h"
#include "timers.h"
#include "bt_type.h"
#include "nvkey.h"
#include "nvkey_id_list.h"

#include "ble_csip.h"
#include "bt_gap_le.h"
#ifdef MTK_BLE_GAP_SRV_ENABLE
#include "bt_gap_le_service.h"
#endif
#include "bt_le_audio_def.h"
#include "app_dongle_le_race.h"
#include "app_le_audio_vcp_volume_controller.h"
#include "app_le_audio_micp_micophone_controller.h"
/**************************************************************************************************
* Define
**************************************************************************************************/
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
#define APP_LE_AUDIO_UCST_LINK_MAX_NUM      4
#else
#define APP_LE_AUDIO_UCST_LINK_MAX_NUM      2
#endif
#define APP_LE_AUDIO_UCST_LINK_IDX_INVALID  0xFF

#define APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM       4
#define APP_LE_AUDIO_UCST_BONDED_LIST_IDX_INVALID   0xFF
#define APP_LE_AUDIO_UCST_BONDED_RECORD_INVALID     0x80

#define APP_LE_AUDIO_UCST_ASE_MAX_NUM       6

#define APP_LE_AUDIO_CONN_INTERVAL_STREAMING    0x30
#define APP_LE_AUDIO_CONN_INTERVAL_CONFIG       0x08

#define APP_LE_AUDIO_UCST_TIMER_CONN_INTERVAL_CONFIG_TIME_PERIOD    (6000) /* 6s */

/* set white list related */
#define APP_LE_AUDIO_UCST_SET_WHITE_LIST_NONE               0
#define APP_LE_AUDIO_UCST_SET_WHITE_LIST_ADD_ON_GOING       1
#define APP_LE_AUDIO_UCST_SET_WHITE_LIST_REMOVE_ON_GOING    2
#define APP_LE_AUDIO_UCST_SET_WHITE_LIST_COMPLETE           3
typedef uint8_t app_le_audio_ucst_set_white_list_state_t;

/* scan related */
#define APP_LE_AUDIO_UCST_SCAN_NONE                 0   /* scan disabled */
#define APP_LE_AUDIO_UCST_SCAN_ENABLING             1   /* scan enabling */
#define APP_LE_AUDIO_UCST_SCAN_ENABLED              2   /* scan enabled */
#define APP_LE_AUDIO_UCST_SCAN_DISABLING            3   /* scan disabling */
#define APP_LE_AUDIO_UCST_SCAN_CS_ENABLING          4   /* scan coordinated set enabling */
#define APP_LE_AUDIO_UCST_SCAN_CS_ENABLED           5   /* scan coordinated set enabled */
#define APP_LE_AUDIO_UCST_SCAN_CS_DISABLING         6   /* scan coordinated set disabling */
typedef uint8_t app_le_audio_ucst_scan_t;

/* create connection related */
#define APP_LE_AUDIO_UCST_CONN_NONE                     0
#define APP_LE_AUDIO_UCST_CONN_LEA_DEVICE               1   /* connect LE AUDIO device */
#define APP_LE_AUDIO_UCST_CONN_BONDED_DEVICE            2   /* connect bonded device */
#define APP_LE_AUDIO_UCST_CONN_COORDINATED_SET_BY_SIRK  3   /* connect coordinated set by SIRK. */
#define APP_LE_AUDIO_UCST_CONN_CS_AND_BONDED_DEVICE     4   /* connect coordinated set and bonded device. */
typedef uint8_t app_le_audio_ucst_conn_t;

/* group related: coordinated set */
#define APP_LE_AUDIO_UCST_GROUP_ID_0            0
#define APP_LE_AUDIO_UCST_GROUP_ID_1            1
#define APP_LE_AUDIO_UCST_GROUP_ID_2            2
#define APP_LE_AUDIO_UCST_GROUP_ID_3            3
#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
#define APP_LE_AUDIO_UCST_GROUP_ID_MAX          2
#else
#define APP_LE_AUDIO_UCST_GROUP_ID_MAX          4
#endif
#define APP_LE_AUDIO_UCST_GROUP_ID_INVALID      0xFF
typedef uint8_t app_le_audio_ucst_group_id_t;

#define APP_LE_AUDIO_UCST_GROUP_LINK_MAX_NUM    2

/* unicast link status */
enum{
    APP_LE_AUDIO_UCST_LINK_STATE_IDLE = 0x00,
    APP_LE_AUDIO_UCST_LINK_STATE_EXCHANGE_MTU,
    APP_LE_AUDIO_UCST_LINK_STATE_READ_AVAILABLE_AUDIO_CONTEXTS,
    APP_LE_AUDIO_UCST_LINK_STATE_READ_SUPPORTED_AUDIO_CONTEXTS,
    APP_LE_AUDIO_UCST_LINK_STATE_READ_SINK_PAC,
    APP_LE_AUDIO_UCST_LINK_STATE_READ_SINK_LOCATION,
    APP_LE_AUDIO_UCST_LINK_STATE_READ_SOURCE_PAC,
    APP_LE_AUDIO_UCST_LINK_STATE_READ_SOURCE_LOCATION,
    APP_LE_AUDIO_UCST_LINK_STATE_READ_ASE,
    APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_CODEC,
    APP_LE_AUDIO_UCST_LINK_STATE_CONFIG_ASE_QOS,//0x0A
    APP_LE_AUDIO_UCST_LINK_STATE_ENABLE_ASE,
    APP_LE_AUDIO_UCST_LINK_STATE_UPDATE_ASE_METADATA,
    APP_LE_AUDIO_UCST_LINK_STATE_CREATE_CIS,
    APP_LE_AUDIO_UCST_LINK_STATE_SETUP_ISO_DATA_PATH,
    APP_LE_AUDIO_UCST_LINK_STATE_SET_ASE_RECV_START_READY,
    APP_LE_AUDIO_UCST_LINK_STATE_STREAMING,//0x10
    APP_LE_AUDIO_UCST_LINK_STATE_DISABLE_ASE,
    APP_LE_AUDIO_UCST_LINK_STATE_SET_ASE_RECV_STOP_READY,
    APP_LE_AUDIO_UCST_LINK_STATE_DISCONNECT_CIS,
    APP_LE_AUDIO_UCST_LINK_STATE_DISCONNECT_ACL,
    APP_LE_AUDIO_UCST_LINK_STATE_MAX,
};
typedef uint8_t app_le_audio_ucst_link_state_t;

#define APP_LE_AUDIO_UCST_LCOK_STREAM_NONE              0x00
#define APP_LE_AUDIO_UCST_LCOK_STREAM_UNIDIRECTIONAL    0x01
#define APP_LE_AUDIO_UCST_LCOK_STREAM_ALL               0x02
#define APP_LE_AUDIO_UCST_LCOK_STREAM_EARPHONE_FOTA     0x04
typedef uint8_t app_le_audio_ucst_lock_stream_t;

#define APP_LE_AUDIO_UCST_PAUSE_STREAM_NONE             0x00
#define APP_LE_AUDIO_UCST_PAUSE_STREAM_UNIDIRECTIONAL   0x01    /* To do: only pause unidirectional stream */
#define APP_LE_AUDIO_UCST_PAUSE_STREAM_ALL              0x02    /* pause unidirectional & bidirectional stream */
#define APP_LE_AUDIO_UCST_PAUSE_STREAM_DONGLE_FOTA      0x04    /* pause all stream until dongle fota complete*/
typedef uint8_t app_le_audio_ucst_pause_stream_t;


/**************************************************************************************************
* Structure
**************************************************************************************************/
typedef struct {
    bt_addr_t addr;
    uint8_t group_id;       /* group id is valid when group size != 0 */
    uint8_t group_size;     /* group size: the number of devices in the group */
    uint8_t link_idx;       /* the link index if the device is connected */
    uint8_t in_white_list;
    uint8_t deleting;
} app_le_audio_ucst_bonded_device_t;

typedef struct {
    uint8_t num;                                                                        /* the number of bonded devices */
    app_le_audio_ucst_bonded_device_t device[APP_LE_AUDIO_UCST_BONDED_LIST_MAX_NUM];    /* bonded device list */
} app_le_audio_ucst_bonded_list_t;

typedef struct {
    uint8_t curr_idx;
    app_le_audio_ucst_set_white_list_state_t state;
} app_le_audio_ucst_set_white_list_t;

typedef struct {
    bt_key_t sirk;
    uint8_t size;
    uint8_t bond_num;
    uint8_t link_idx[APP_LE_AUDIO_UCST_GROUP_LINK_MAX_NUM];
} app_le_audio_ucst_group_info_t;

typedef struct {
    uint8_t num;
    bt_addr_t addr[APP_LE_AUDIO_UCST_GROUP_LINK_MAX_NUM];
} app_le_audio_ucst_active_group_addr_list_t;

typedef struct {
    uint8_t codec_id[AUDIO_CODEC_ID_SIZE];//ble_bap_codec_id_t codec_id;
    uint8_t supported_frame_durations;
    uint8_t supported_audio_channel_counts;
    uint8_t supported_codec_frame_blocks_per_sdu;
    uint16_t supported_sampling_frequencies;
    uint16_t supported_octets_per_codec_frame_min;
    uint16_t supported_octets_per_codec_frame_max;
    uint16_t preferred_audio_contexts;
} app_le_audio_pac_record_t;

typedef struct {
    app_le_audio_pac_record_t *pac_record;      /**< Data of PAC records. */
    //uint8_t pac_record_length;                /**< Length of PAC records. */
    uint8_t num_of_record;                      /**< Number of PAC records in this characteristic. */
    //uint8_t is_supported;
} app_le_audio_pac_t;

typedef struct {
    uint8_t framing;
    uint8_t preferred_phy;
    uint8_t preferred_retransmission_number;
    uint16_t max_transport_latency;
    uint32_t presentation_delay_min: 24;
    uint32_t presentation_delay_max: 24;
    uint32_t preferred_presentation_delay_min: 24;
    uint32_t preferred_presentation_delay_max: 24;
    uint8_t codec_id[AUDIO_CODEC_ID_SIZE];//ble_bap_codec_id_t codec_id;
    uint8_t frame_duration; /*0x00:7.5ms, 0x01:10ms*/
    uint8_t codec_frame_blocks_per_sdu;
    uint8_t sampling_frequency;
    uint16_t octets_per_codec_frame;
    uint16_t audio_contexts;//configed for which audio_contexts
    uint32_t audio_channel_allocation;
} app_le_audio_ase_codec_t;

typedef struct {
    uint8_t cig_id;
    uint8_t cis_id;
    uint32_t sdu_interval: 24;
    uint8_t framing;
    uint8_t phy;
    uint16_t max_sdu;
    uint8_t retransmission_number;
    uint16_t max_transport_latency;
    uint32_t presentation_delay: 24;
} app_le_audio_ase_qos_t;

typedef struct {
    uint8_t cig_id;
    uint8_t cis_id;
    uint16_t streaming_audio_contexts;
} app_le_audio_ase_metadata_t;

typedef struct {
    uint8_t id;
    uint8_t curr_state;
    bt_le_audio_direction_t direction;
    app_le_audio_ase_codec_t codec_state;
    app_le_audio_ase_qos_t qos_state;
    app_le_audio_ase_metadata_t metadata;
} app_le_audio_ase_t;

typedef struct {
    uint8_t wait_ase_event;                     /* Number of ASE events expected to receive.  */
    uint8_t wait_ase_cp_event;                  /* Number of ASE control point events expected to receive. */
} app_le_audio_wait_bap_event_t;

typedef struct {
    bt_addr_t addr;                             /* peer device address */
    bt_handle_t handle;                         /* connection handle */
    app_le_audio_ucst_link_state_t curr_state;
    app_le_audio_ucst_link_state_t next_state;
    uint8_t group_id;
    uint8_t group_size;
    uint16_t tmap_role;
    uint16_t curr_interval;
    uint16_t next_interval;
    app_le_audio_vcp_info_t vcp_info;
    app_le_audio_micp_info_t micp_info;

    app_le_audio_pac_t sink_pac;
    uint32_t sink_location;
    uint16_t sink_available_contexts;
    uint16_t sink_supported_contexts;
    uint8_t sink_ase_num;                       /* Number of sink ASEs  */
    uint8_t sink_location_num;
    uint8_t sink_audio_channel_num;

    app_le_audio_pac_t source_pac;
    uint32_t source_location;
    uint16_t source_available_contexts;
    uint16_t source_supported_contexts;
    uint8_t source_ase_num;                     /* Number of source ASEs  */
    uint8_t source_location_num;
    uint8_t source_audio_channel_num;
    uint8_t source_ase_idx;

    app_le_audio_ase_t ase[APP_LE_AUDIO_UCST_ASE_MAX_NUM];
    app_le_audio_wait_bap_event_t wait_event;
    bool release;
    TimerHandle_t le_connection_timer_handle;
    TimerHandle_t conn_interval_timer_handle;
    uint8_t read_sink_ase_num;
    uint8_t read_source_ase_num;
    uint8_t ase_releasing;
    uint8_t cis_num;

    uint8_t bond_idx;
    uint8_t add_white_list;
    app_le_audio_ucst_lock_stream_t lock_stream;
#ifdef AIR_SILENCE_DETECTION_ENABLE
    bool remote_device_bredr_connected; /* If the remote device connects to any BR/EDR device or not. */
    bool disconnect_cis_for_silence; /* If CIS need be disconnected when silence is detected. */
#endif
} app_le_audio_ucst_link_info_t;


/**************************************************************************************************
* Public function
**************************************************************************************************/
void app_le_audio_ucst_register_callback(app_dongle_le_race_event_callback_t callback);

bool app_le_audio_ucst_is_connected_device(const bt_addr_t *addr);

bool app_le_audio_ucst_is_active_group(uint8_t group_id);

bool app_le_audio_ucst_is_active_group_by_handle(bt_handle_t handle);

bool app_le_audio_ucst_is_group_device_all_connected(uint8_t group_id);

bt_status_t app_le_audio_ucst_stop_scan_all(void);  /* stop current scan (scan LE AUDIO devices or scan coordinated set devices)*/

void app_le_audio_ucst_set_sirk(bt_key_t *sirk, bool update_nvkey);

bt_key_t *app_le_audio_ucst_get_sirk(bool from_nvkey);

uint8_t app_le_audio_ucst_get_link_num(void);

uint8_t app_le_audio_ucst_get_link_idx(bt_handle_t handle);

uint8_t app_le_audio_ucst_get_link_idx_by_cis(bt_handle_t cis_handle, uint8_t *cis_idx);

app_le_audio_ucst_link_info_t *app_le_audio_ucst_get_link_info(bt_handle_t handle);

app_le_audio_ucst_link_info_t *app_le_audio_ucst_get_link_info_by_idx(uint8_t link_idx);

app_le_audio_ucst_link_info_t *app_le_audio_ucst_get_link_info_by_addr(bt_addr_t *addr);

app_le_audio_ucst_group_info_t *app_le_audio_ucst_get_group_info(uint8_t group_id);

uint8_t app_le_audio_ucst_get_active_group_address(app_le_audio_ucst_active_group_addr_list_t *addr_list);

app_le_audio_ucst_conn_t app_le_audio_ucst_get_curr_conn_type(void);

uint8_t app_le_audio_ucst_get_active_group(void);

void app_le_audio_ucst_reset_all_bonded_info(void);

void app_le_audio_ucst_get_bonded_list(app_le_audio_ucst_bonded_list_t *p_bonded_list);

void app_le_audio_ucst_check_group_device_bond(const bt_addr_t *addr);

void app_le_audio_ucst_handle_adv_report_ind(bt_status_t ret, bt_gap_le_ext_advertising_report_ind_t *ind);

bt_status_t app_le_audio_ucst_connect_coordinated_set(bool use_nvkey_sirk); /* scan and connect coordinated set devices */

bt_status_t app_le_audio_ucst_find_device(void);                /* scan and connect devices (bonded devices or coordinated set devices)*/

bt_status_t app_le_audio_ucst_connect_device(const bt_addr_t *addr);      /* connect device by bd_addr */

void app_le_audio_ucst_cancel_create_connection(void);
bt_status_t app_le_audio_ucst_disconnect(bt_handle_t handle);

bt_status_t app_le_audio_ucst_disconnect_device(bt_addr_t *addr);   /* disconnect device by bd_addr */

bool app_le_audio_ucst_disconnect_all_device(void);     /* disconnect all device */

bt_status_t app_le_audio_ucst_start_scan_device(void);  /* start scan LE AUDIO device */

bt_status_t app_le_audio_ucst_stop_scan_device(void);   /* stop scan LE AUDIO device */

#ifdef AIR_LE_AUDIO_MULTI_DEVICE_ENABLE
void app_le_audio_ucst_set_active_group(uint8_t group);
#endif
bt_status_t app_le_audio_ucst_check_adv_data(bt_gap_le_ext_advertising_report_ind_t *ind);

bt_status_t app_le_audio_ucst_update_connection_interval(app_le_audio_ucst_link_info_t *p_info, uint16_t interval);

void app_le_audio_ucst_increase_connection_config_speed(bt_handle_t handle);

void app_le_audio_ucst_increase_active_device_config_speed(void);

void app_le_audio_ucst_check_link_idle(bt_handle_t handle);

void app_le_audio_ucst_check_active_device_idle(void);

void app_le_audio_ucst_handle_disconnect_ind(bt_status_t ret, bt_hci_evt_disconnect_complete_t *ind);

void app_le_audio_ucst_handle_connection_update_ind(bt_status_t ret, bt_gap_le_connection_update_ind_t *ind);

void app_le_audio_csip_handle_power_on(void);

void app_le_audio_csip_handle_power_off(void);

void app_le_audio_ucst_handle_csip_evt(ble_csip_event_t event, void *msg);

void app_le_audio_handle_gap_le_srv_event_callback(bt_gap_le_srv_event_t event, void *data);

void app_le_audio_ucst_sync_lock_stream_flag(app_le_audio_ucst_link_info_t *p_info, app_le_audio_ucst_lock_stream_t lock, bool set);

void app_le_audio_csip_reset(void);

bool app_le_audio_ucst_is_connection_update_request_accepted(bt_handle_t handle, bt_gap_le_connection_update_param_t *connection_parameter);

#endif /* AIR_LE_AUDIO_ENABLE */
#endif /* __APP_LE_AUDIO_CSIP_SET_COORDINATOR_H__ */


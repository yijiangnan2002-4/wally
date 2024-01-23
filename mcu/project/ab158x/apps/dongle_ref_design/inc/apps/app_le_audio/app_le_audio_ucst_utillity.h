/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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

#ifndef __APP_LE_AUDIO_UCST_UTILITY_H__
#define __APP_LE_AUDIO_UCST_UTILITY_H__

#ifdef AIR_LE_AUDIO_ENABLE

#include "app_le_audio_utillity.h"
#include "app_le_audio_csip_set_coordinator.h"

#include "bt_le_audio_def.h"

/**************************************************************************************************
* Define
**************************************************************************************************/
/* unicast stream status */
#define APP_LE_AUDIO_UCST_STREAM_STATE_IDLE                            0x00
#define APP_LE_AUDIO_UCST_STREAM_STATE_START_AUDIO_STREAM              0x01
#define APP_LE_AUDIO_UCST_STREAM_STATE_SET_CIG_PARAMETER               0x02
#define APP_LE_AUDIO_UCST_STREAM_STATE_STREAMING                       0x03
#define APP_LE_AUDIO_UCST_STREAM_STATE_STOP_STREAMING                  0x04
#define APP_LE_AUDIO_UCST_STREAM_STATE_STOP_AUDIO_STREAM               0x05
#define APP_LE_AUDIO_UCST_STREAM_STATE_REMOVE_CIG                      0x06
#define APP_LE_AUDIO_UCST_STREAM_STATE_MAX                             0x07
typedef uint8_t app_le_audio_ucst_stream_state_t;

/* CIS status */
#define APP_LE_AUDIO_UCST_CIS_IDLE              0
#define APP_LE_AUDIO_UCST_CIS_CREATING          1
#define APP_LE_AUDIO_UCST_CIS_CREATED           2
#define APP_LE_AUDIO_UCST_CIS_SETUP_DATA_PATH   3
#define APP_LE_AUDIO_UCST_CIS_STREAMING         4
#define APP_LE_AUDIO_UCST_CIS_DISCONNECTING     5
typedef uint8_t app_le_audio_ucst_cis_status_t;

#define APP_LE_AUDIO_UCST_TARGET_NONE               0x00
#define APP_LE_AUDIO_UCST_TARGET_START_MEDIA_MODE   0x01
#define APP_LE_AUDIO_UCST_TARGET_STOP_MEDIA_MODE    0x02
#define APP_LE_AUDIO_UCST_TARGET_START_CALL_MODE    0x03
#define APP_LE_AUDIO_UCST_TARGET_STOP_CALL_MODE     0x04
#define APP_LE_AUDIO_UCST_TARGET_START_SPECIAL_SILENCE_DETECTION_MODE     0x05
#define APP_LE_AUDIO_UCST_TARGET_STOP_SPECIAL_SILENCE_DETECTION_MODE     0x06
typedef uint8_t app_le_audio_ucst_target_t;

/* stream port */
#define APP_LE_AUDIO_UCST_STREAM_PORT_SPK_0         0
#define APP_LE_AUDIO_UCST_STREAM_PORT_SPK_1         1
#define APP_LE_AUDIO_UCST_STREAM_PORT_MIC_0         2
#define APP_LE_AUDIO_UCST_STREAM_PORT_MAX           3
typedef uint8_t app_le_audio_ucst_stream_port_t;

#define APP_LE_AUDIO_UCST_CREATE_CIS_BY_USB_PORT            0x00    /* create cis base on usb port */
#define APP_LE_AUDIO_UCST_CREATE_CIS_ALWAYS_UNIDIRECTIONAL  0x01
#define APP_LE_AUDIO_UCST_CREATE_CIS_ALWAYS_BIDIRECTIONAL   0x02
#define APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_1              0x03    /* 1: Cis:1, H:[Sink:1(ch:1), Src:0      ] */
#define APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_2              0x04    /* 2: Cis:1, H:[Sink:0      , Src:1(ch:1)] */
#define APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_3              0x05    /* 3: Cis:1, H:[Sink:1(ch:1), Src:1(ch:1)] */
#define APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_4              0x06    /* 4: Cis:1, H:[Sink:1(ch:2), Src:0      ] */
//#define APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_5              0x07  /* 5: Cis:1, H:[Sink:1(ch:2), Src:1(ch:1)] */
#define APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_6_1            0x08    /* 6: Cis:2, H:[Sink:2(ch:1), Src:0      ] */
#define APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_6_2            0x09    /* 7: Cis:2, L:[Sink:1(ch:1), Src:0      ], R:[Sink:1(ch:1), Src:0      ] */
#define APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_7_1            0x0A    /* 8: Cis:2, H:[Sink:1(ch:1), Src:1(ch:1)] */
#define APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_7_2            0x0B    /* 9: Cis:2, L:[Sink:0      , Src:1(ch:1)], R:[Sink:1(ch:1), Src:0      ] */
#define APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_8_1            0x0C    /* 10: Cis:2, H:[Sink:2(ch:1), Src:1(ch:1)] */
#define APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_8_2            0x0D    /* 11: Cis:2, L:[Sink:1(ch:1), Src:1(ch:1)], R:[Sink:1(ch:1), Src:0      ] */
//#define APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_9_1            0x0E  /* 12: Cis:2, H:[Sink:0      , Src:2(ch:1)] */
#define APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_9_2            0x0F    /* 13: Cis:2, L:[Sink:0      , Src:1(ch:1)], R:[Sink:0      , Src:1(ch:1)] */
//#define APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_10             0x10  /* 14: Cis:1, H:[Sink:0      , Src:1(ch:2)] */
//#define APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_11_1           0x11  /* 15: Cis:2, H:[Sink:2(ch:1), Src:2(ch:1)] */
#define APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_11_2           0x12    /* 16: Cis:2, L:[Sink:1(ch:1), Src:1(ch:1)], R:[Sink:1(ch:1), Src:1(ch:1)] */
#define APP_LE_AUDIO_UCST_CREATE_CIS_MODE_MAX               0x13
typedef uint8_t app_le_audio_ucst_create_cis_mode_t;

#define APP_LE_AUDIO_UCST_TEST_MODE_ENABLE                  0x01
#define APP_LE_AUDIO_UCST_TEST_MODE_PAUSE_BEFORE_CONFIG_ASE 0x02
#define APP_LE_AUDIO_UCST_TEST_MODE_SCAN_RSI_DATA           0x04
#define APP_LE_AUDIO_UCST_TEST_MODE_CIG_PARAM               0x08

/**************************************************************************************************
* Structure
**************************************************************************************************/


typedef struct {
    bt_handle_t cis_handle;                     /* CIS connection handle */
    bt_handle_t acl_handle;                     /* ACL connection handle */
    app_le_audio_ucst_cis_status_t cis_status;
} app_le_audio_ucst_cis_info_t;

typedef struct {
    app_le_audio_ucst_scan_t curr_scan;
    app_le_audio_ucst_scan_t next_scan;
    app_le_audio_ucst_conn_t curr_conn;
    app_le_audio_ucst_target_t curr_target;
    app_le_audio_ucst_target_t next_target;
    app_le_audio_ucst_stream_state_t curr_stream_state;
    app_le_audio_ucst_stream_state_t next_stream_state;
    app_le_audio_qos_params_t qos_params_spk_0;
    app_le_audio_qos_params_t qos_params_spk_1;
    app_le_audio_qos_params_t qos_params_mic_0;
    app_le_audio_ucst_pause_stream_t pause_stream;
    app_le_audio_ucst_create_cis_mode_t create_cis_mode;
    uint8_t is_cig_created;
    uint8_t cig_id;
    uint8_t cis_num;
    uint8_t release;
    uint8_t set_endpoint_tx_ready;
    app_le_audio_ucst_group_id_t curr_group;        /* the group id of the current active group */
    app_le_audio_ucst_group_id_t next_group;        /* the group id of the next active group */
    app_le_audio_ucst_group_id_t latest_group;      /* the group id of the latest connected group */
    //uint8_t is_creating_connection;
    app_le_audio_qos_params_type_t qos_params_type;
#ifdef APP_LE_AUDIO_UCST_UPLINK_MIX_ENABLE
    bool uplink_mix_enable;
#endif
} app_le_audio_ucst_ctrl_t;

typedef struct {
    uint8_t bn;
    uint8_t nse;
    uint8_t ft;
    uint16_t iso_interval;                      /* ISO interval = SDU interval*bn, bn>=1 */
} app_le_audio_ucst_cig_params_test_t;



/**************************************************************************************************
* Public function
**************************************************************************************************/
void app_le_audio_ucst_set_qos_params(uint8_t sampling_rate, uint8_t sel_setting, uint8_t target_latency, app_le_audio_ucst_stream_port_t port);

void app_le_audio_ucst_set_create_cis_mode(app_le_audio_ucst_create_cis_mode_t create_cis_mode);


app_le_audio_ucst_target_t app_le_audio_ucst_get_curr_target(void);

app_le_audio_ucst_pause_stream_t app_le_audio_ucst_get_pause_stream_flag(void);

app_le_audio_ucst_create_cis_mode_t app_le_audio_ucst_get_create_cis_mode(void);

uint32_t app_le_audio_ucst_get_sampling_rate(bool mic);

uint16_t app_le_audio_ucst_get_sdu_size(bool mic);

uint32_t app_le_audio_ucst_get_sdu_interval(bool mic);

float app_le_audio_ucst_get_bitrate(bool mic);

uint32_t app_le_audio_ucst_get_location(uint8_t idx, uint32_t location);

uint8_t app_le_audio_ucst_get_location_count(uint32_t location);

uint32_t app_le_audio_ucst_get_available_channel(void);

void app_le_audio_ucst_reset_cig_parameter_test(void);



void app_le_audio_ucst_set_cig_parameter_test(uint8_t bn, uint8_t nse, uint8_t ft, uint16_t iso_interval);

void app_le_audio_ucst_set_ccid_list(uint8_t ccid_list_size, uint8_t *ccid_list);

app_le_audio_ucst_ctrl_t *app_le_audio_ucst_get_ctrl(void);

void app_le_audio_ucst_notify_mic_mute(bool mic_mute);

#ifdef APP_LE_AUDIO_UCST_UPLINK_MIX_ENABLE
void app_le_audio_ucst_set_uplink_mix_status(bool enable);

bool app_le_audio_ucst_get_uplink_mix_status(void);
#endif

bool app_le_audio_ucst_is_delay_disconnect_timer_exist(void);
void app_le_audio_ucst_start_delay_disconnect_timer(void);
void app_le_audio_ucst_stop_delay_disconnect_timer(void);

app_le_audio_qos_params_t *app_le_audio_ucst_get_qos_params(app_le_audio_ucst_stream_port_t port, app_le_audio_qos_params_type_t qos_params_type);

#ifdef AIR_LE_AUDIO_GMAP_ENABLE
void app_le_audio_ucst_get_qos_params_type(app_le_audio_qos_params_type_t *qos_params_type);

bool app_le_audio_ucst_gmap_set_qos_params(uint8_t sel_setting, uint8_t audio_config_level, app_le_audio_ucst_stream_port_t port);
#endif /* AIR_LE_AUDIO_GMAP_ENABLE */

#endif /* AIR_LE_AUDIO_ENABLE */
#endif /* __APP_LE_AUDIO_UCST_UTILITY_H__ */


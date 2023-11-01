/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#ifndef BT_TIMER_EXTERNAL_H
#define BT_TIMER_EXTERNAL_H
#include <stdbool.h>
#include <stdint.h>
#include "bt_platform.h"

#ifdef MTK_BT_TIMER_EXTERNAL_ENABLE
#define __BT_TIMER_EXT_SUPPORT__
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define BT_TIMER_EXT_INSTANCE_NUM (0xE)
#define BT_TIMER_EXT_EXPIRED      (0x80)

#define BT_TIMER_EXT_STATUS_SUCCESS  (0)
#define BT_TIMER_EXT_STATUS_FAIL     (-1)
#define BT_TIMER_EXT_STATUS_OOM      (-2)
typedef int32_t bt_timer_ext_status_t;

/* For BT Timer EXT ID Structure. */
/*
 * +---------+-------------+--------+
 * |Module ID| Group Mask  | Timer  |
 * +---------+-------------+--------+
 */

#define BT_MODULE_TIMER_EXT   0x88000000 /**< Prefix of the timer ext module. */

#define BT_TIMER_EXT_GROUP_SINK_MASK  0x00010000
#define BT_TIMER_EXT_GROUP_SOURCE_MASK  0x00020000
#define BT_TIMER_EXT_GROUP_CM_MASK      0x00040000
#define BT_TIMER_EXT_GROUP_ULL_MASK     0x00050000
#define BT_TIMER_EXT_GROUP_SERVICE_MASK 0x00060000
#define BT_TIMER_EXT_GROUP_GATTC_MASK   0x00070000
#define BT_TIMER_EXT_GROUP_INVALID_MASK 0x00FFFFFF

#define BT_TIMER_EXT_GROUP_SINK(timer) \
        (BT_MODULE_TIMER_EXT | BT_TIMER_EXT_GROUP_SINK_MASK | (timer))

#define BT_TIMER_EXT_GROUP_SOURCE(timer) \
        (BT_MODULE_TIMER_EXT | BT_TIMER_EXT_GROUP_SOURCE_MASK | (timer))

#define BT_TIMER_EXT_GROUP_CM(timer) \
        (BT_MODULE_TIMER_EXT | BT_TIMER_EXT_GROUP_CM_MASK | (timer))

#define BT_TIMER_EXT_GROUP_ULL(timer) \
            (BT_MODULE_TIMER_EXT | BT_TIMER_EXT_GROUP_ULL_MASK | (timer))

#define BT_TIMER_EXT_GROUP_SERVICE(timer) \
            (BT_MODULE_TIMER_EXT | BT_TIMER_EXT_GROUP_SERVICE_MASK | (timer))

#define BT_TIMER_EXT_GROUP_GATTC(timer) \
            (BT_MODULE_TIMER_EXT | BT_TIMER_EXT_GROUP_GATTC_MASK | (timer))

/**
  * @brief Define the BT external time ID.
  */
#define BT_SINK_SRV_TIMER_FLUSH_DB                        (BT_TIMER_EXT_GROUP_SINK(1))
#define BT_SINK_SRV_TIMER_CM_DISCOVERABLE                 (BT_TIMER_EXT_GROUP_SINK(2))
#define BT_SINK_SRV_TIMER_CM_REQUEST_DELAY                (BT_TIMER_EXT_GROUP_SINK(3))
#define BT_SINK_SRV_TIMER_CM_PROFILE_NOTIFY               (BT_TIMER_EXT_GROUP_SINK(4))
#define BT_SINK_SRV_TIMER_CM_PROFILE_ALREADY_EXIST_NOTIFY (BT_TIMER_EXT_GROUP_SINK(5))
#define BT_SINK_SRV_TIMER_HF_WAIT_CALL_IND                (BT_TIMER_EXT_GROUP_SINK(6))
#define BT_SINK_SRV_CM_TO_DO_BT_POWER_OFF_TIMER_ID        (BT_TIMER_EXT_GROUP_SINK(7))
#define BT_SINK_SRV_TIMER_ID_HS_WAIT_RING_IND             (BT_TIMER_EXT_GROUP_SINK(8))
#define BT_SINK_SRV_AVRCP_CONNECTION_TIMER_ID             (BT_TIMER_EXT_GROUP_SINK(9))
#define BT_SINK_SRV_CM_END_AIR_PAIRING_TIMER_ID           (BT_TIMER_EXT_GROUP_SINK(10))
#define BT_SINK_SRV_CM_TERMINATE_AIR_PAIRING_TIMER_ID     (BT_TIMER_EXT_GROUP_SINK(11))
#define BT_SINK_SRV_CM_AUTO_ROLE_CHANGE_TIMER_ID          (BT_TIMER_EXT_GROUP_SINK(12))
#ifdef MTK_BT_SPEAKER_ENABLE
#define BT_SINK_SRV_A2DP_SEND_RETRY_SEND_EIR_TIMER_ID     (BT_TIMER_EXT_GROUP_SINK(13))
#endif
#define BT_SINK_SRV_CM_STATE_EXT_ASYNC_TIMER_ID           (BT_TIMER_EXT_GROUP_SINK(14))
#define BT_SINK_SRV_AVRCP_DISCONNECT_TIMER_ID             (BT_TIMER_EXT_GROUP_SINK(15))
#define BT_SINK_SRV_SET_AUDIO_PLAY_EN_TIMER               (BT_TIMER_EXT_GROUP_SINK(16))
#define BT_SINK_SRV_CM_DISCONNECT_TIMER_ID                (BT_TIMER_EXT_GROUP_SINK(17))
#define BT_SINK_SRV_CM_DELAY_TO_SWITCH_ROLE_TIMER_ID      (BT_TIMER_EXT_GROUP_SINK(19))
#ifdef MTK_BT_SPEAKER_ENABLE
#define BT_SINK_SRV_A2DP_SEND_PLAY_IND_TIMER_ID           (BT_TIMER_EXT_GROUP_SINK(20))
#define BT_SINK_SRV_A2DP_SEND_BASE_IND_TIMER_ID           (BT_TIMER_EXT_GROUP_SINK(21))
#define BT_SINK_SRV_RECONNECT_AGENT_TIMER_ID              (BT_TIMER_EXT_GROUP_SINK(22))
#endif
#define BT_SINK_SRV_AVRCP_CONN_AS_TG_TIMER_ID              (BT_TIMER_EXT_GROUP_SINK(23))
#define BT_SINK_SRV_AVRCP_PLAY_PAUSE_ACTION_TIMER_ID_START (BT_TIMER_EXT_GROUP_SINK(24))
#define BT_SINK_SRV_AVRCP_PLAY_PAUSE_ACTION_TIMER_ID_END   (BT_TIMER_EXT_GROUP_SINK(26))

#define BT_SINK_SRV_MUSIC_SET_AM_TIMER_ID                   (BT_TIMER_EXT_GROUP_SINK(27))
#define BT_SINK_SRV_HF_SET_VOLUME_TIMER_ID                  (BT_TIMER_EXT_GROUP_SINK(30))

#define BT_SINK_SRV_AVRCP_PUSH_RELEASE_ACTION_TIMER_ID_START  (BT_TIMER_EXT_GROUP_SINK(31))
#define BT_SINK_SRV_AVRCP_PUSH_RELEASE_ACTION_TIMER_ID_END    (BT_TIMER_EXT_GROUP_SINK(33))
#define BT_SINK_SRV_AUDIO_SYNC_STOP_GUARD_TIMER_ID          (BT_TIMER_EXT_GROUP_SINK(34))

#ifdef AIR_LE_AUDIO_ENABLE
#define BT_SINK_SRV_CIS_DISCONNECT_TIMER_ID                 (BT_TIMER_EXT_GROUP_SINK(34))
#endif

#define BT_SINK_SRV_HF_DISABLE_SNIFF_TIMER_ID               (BT_TIMER_EXT_GROUP_SINK(35))

#ifdef AIR_LE_AUDIO_ENABLE
#define BT_SINK_SRV_CIS_DISCONNECT_TIMER_ID_2               (BT_TIMER_EXT_GROUP_SINK(36))
#endif

#ifdef BT_SINK_DUAL_ANT_ENABLE
#define BT_SINK_SRV_DUAL_ANT_SPK_DELAY_STOP_TIMER           (BT_TIMER_EXT_GROUP_SINK(37))
#endif
#ifdef MTK_BT_SPEAKER_ENABLE
#define BT_SINK_SRV_CALL_INFO_RETRY_TIMER_ID                (BT_TIMER_EXT_GROUP_SINK(38))
#endif

#ifdef AIR_LE_AUDIO_ENABLE
#define BT_SINK_SRV_SEND_ASE_STREAMING_STATE_TIMER_ID_1         (BT_TIMER_EXT_GROUP_SINK(39))
#define BT_SINK_SRV_SEND_ASE_STREAMING_STATE_TIMER_ID_2         (BT_TIMER_EXT_GROUP_SINK(40))
#define BT_SINK_SRV_SEND_ASE_STREAMING_STATE_TIMER_ID_3         (BT_TIMER_EXT_GROUP_SINK(43))
#define BT_SINK_SRV_SEND_ASE_STREAMING_STATE_TIMER_ID_4         (BT_TIMER_EXT_GROUP_SINK(44))
#define BT_SINK_SRV_AM_SUSPEND_CALLBACK_TIMER                   (BT_TIMER_EXT_GROUP_SINK(47))
#endif

#define BT_SINK_SRV_HF_TRIGGER_VR_TIMER_ID                  (BT_TIMER_EXT_GROUP_SINK(41))
#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
#define BT_SINK_SRV_A2DP_INT_USER_CONF_TIMER_ID             (BT_TIMER_EXT_GROUP_SINK(42))
#endif
#define BT_SINK_SRV_STA_TIMER_ID                          (BT_TIMER_EXT_GROUP_SINK(45))
#define BT_SINK_SRV_VP_DETECT_TEMER                   (BT_TIMER_EXT_GROUP_SINK(46))
#define BT_SINK_SRV_HF_HOLD_TIMER_ID_START                      (BT_TIMER_EXT_GROUP_SINK(48))
#define BT_SINK_SRV_HF_HOLD_TIMER_ID_END                        (BT_TIMER_EXT_GROUP_SINK(50))
#define BT_SINK_SRV_MUSIC_RHO_PENDING_CHECKING_TIMER_ID         (BT_TIMER_EXT_GROUP_SINK(51))

#ifdef AIR_LE_AUDIO_ENABLE
#ifdef AIR_BT_FAST_PAIR_SASS_ENABLE
#define BT_SINK_SRV_CAP_AM_LEA_INT_WAIT_USER_TIMER              (BT_TIMER_EXT_GROUP_SINK(49))
#endif
#define BT_SINK_SRV_CAP_AM_REJECT_TIMER              (BT_TIMER_EXT_GROUP_SINK(52))
#endif


#ifdef MTK_BT_CM_SUPPORT
#define BT_CM_CONNECTION_TIMER_ID                           (BT_TIMER_EXT_GROUP_CM(1))
#define BT_CM_FORCE_POWER_OFF_TIMER_ID                      (BT_TIMER_EXT_GROUP_CM(2))
#define BT_CM_SWITCH_ROLE_TIMER_ID                          (BT_TIMER_EXT_GROUP_CM(3))
#define BT_CM_DELAY_RECONNECT_TIMER_ID                      (BT_TIMER_EXT_GROUP_CM(4))
#define BT_CM_AWS_MCE_STATE_UPDATE_TIMER_ID               (BT_TIMER_EXT_GROUP_CM(7))
#define BT_CM_CANCEL_CONNECT_TIMER_ID                       (BT_TIMER_EXT_GROUP_CM(5))
#define BT_CM_AWS_MCE_CONNECTABLE_TIMER_ID                  (BT_TIMER_EXT_GROUP_CM(8))
#endif
#define BT_DM_PAIRED_INFOR_SYNC_TIMER_ID                    (BT_TIMER_EXT_GROUP_CM(6))

#if defined (AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
#define BT_ULL_CRITICAL_TX_RESULT_TIMER_ID                   (BT_TIMER_EXT_GROUP_ULL(1))
#define BT_ULL_CRITICAL_RX_IND_TIMER_ID                      (BT_TIMER_EXT_GROUP_ULL(2))
#define BT_ULL_ATCI_TEST_TIMER_ID                            (BT_TIMER_EXT_GROUP_ULL(3))
#define BT_ULL_DELAY_PLAY_TIMER_ID                           (BT_TIMER_EXT_GROUP_ULL(4))
#define BT_ULL_CONFLICT_RECONNECT_TIMER_ID                   (BT_TIMER_EXT_GROUP_ULL(5))
#endif
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
#define BT_ULL_LE_FIND_CS_TIMER_ID                           (BT_TIMER_EXT_GROUP_ULL(10))
#endif
#if defined (AIR_BLE_ULTRA_LOW_LATENCY_ENABLE) || defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
#define BT_ULL_LE_AM_UL_MUTE_TIMER_ID                        (BT_TIMER_EXT_GROUP_ULL(11))
#define BT_ULL_LE_CONN_WAITING_TIMER_ID                      (BT_TIMER_EXT_GROUP_ULL(12))
#define BT_ULL_LE_AUD_QOS_CHANGE_TIMER_ID                    (BT_TIMER_EXT_GROUP_ULL(13))
#define BT_ULL_LE_AUD_QOS_REPORT_ENABLE_TIMER_ID             (BT_TIMER_EXT_GROUP_ULL(14))
#endif

#if defined (AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE)
#define BT_ULL_LE_HID_SERVICE_CONNECT_TIMER_ID               (BT_TIMER_EXT_GROUP_ULL(30))
#define BT_ULL_LE_HID_SYNC_CONTEXT_INFO_TIMER_ID             (BT_TIMER_EXT_GROUP_ULL(31))
#define BT_ULL_LE_HID_BONDING_REQ_TIMER_ID                   (BT_TIMER_EXT_GROUP_ULL(32))
#define BT_ULL_LE_HID_SYNC_BONDING_INFO_TIMER_ID             (BT_TIMER_EXT_GROUP_ULL(33))
#define BT_ULL_LE_HID_FAST_PAIRING_MODE_TIMER_ID             (BT_TIMER_EXT_GROUP_ULL(34))
#define BT_ULL_LE_HID_FACTORY_TEST_MODE_TIMER_ID             (BT_TIMER_EXT_GROUP_ULL(35))
#define BT_ULL_LE_HID_CREAT_CIS_MODE_TIMER_ID                (BT_TIMER_EXT_GROUP_ULL(36))


#endif

#ifdef AIR_BLE_HRS_ENABLE
#define BLE_SERVICE_HRS_APP_HEART_RATE_MEASUREMENT_TIMER_ID                   (BT_TIMER_EXT_GROUP_SERVICE(1))
#endif

#ifdef AIR_BLE_PLXS_ENABLE
#define BLE_SERVICE_PLXS_APP_SPOT_CHECK_MEASUREMENT_STABLE_TIMEOUT_TIMER_ID   (BT_TIMER_EXT_GROUP_SERVICE(2))
#define BLE_SERVICE_PLXS_APP_SPOT_CHECK_MEASUREMENT_STABLE_TIMER_ID           (BT_TIMER_EXT_GROUP_SERVICE(3))
#define BLE_SERVICE_PLXS_APP_CONTINUOUS_MEASUREMENT_INTERVAL_TIMER_ID         (BT_TIMER_EXT_GROUP_SERVICE(4))

#endif

#ifdef AIR_BLE_PAMS_ENABLE
#define BLE_SERVICE_PAMS_APP_GASD_INDICATION_INTERVAL_TIMER_ID         (BT_TIMER_EXT_GROUP_SERVICE(5))
#define BLE_SERVICE_PAMS_APP_GAID_NOTIFICATION_INTERVAL_TIMER_ID       (BT_TIMER_EXT_GROUP_SERVICE(6))
#define BLE_SERVICE_PAMS_APP_CRASD_INDICATION_INTERVAL_TIMER_ID        (BT_TIMER_EXT_GROUP_SERVICE(7))
#define BLE_SERVICE_PAMS_APP_CRAID_NOTIFICATION_INTERVAL_TIMER_ID      (BT_TIMER_EXT_GROUP_SERVICE(8))
#define BLE_SERVICE_PAMS_APP_SCASD_INDICATION_INTERVAL_TIMER_ID        (BT_TIMER_EXT_GROUP_SERVICE(9))
#define BLE_SERVICE_PAMS_APP_SASD_INDICATION_INTERVAL_TIMER_ID         (BT_TIMER_EXT_GROUP_SERVICE(10))
#define BLE_SERVICE_PAMS_APP_SAID_NOTIFICATION_INTERVAL_TIMER_ID       (BT_TIMER_EXT_GROUP_SERVICE(11))
#define BLE_SERVICE_PAMS_APP_SEND_SEGEMENT_INTERVAL_TIMER_ID           (BT_TIMER_EXT_GROUP_SERVICE(12))
#endif

#ifdef AIR_BT_SOURCE_ENABLE
#define BT_SOURCE_SRV_RING_ALERTING_TIMER_ID                 (BT_TIMER_EXT_GROUP_SOURCE(1))
#define BT_SOURCE_SRV_CONNECT_ACTION_HANDLE_TIMER_ID         (BT_TIMER_EXT_GROUP_SOURCE(2))
#define BT_SOURCE_SRV_AVRCP_CONNECTION_TIMER_ID              (BT_TIMER_EXT_GROUP_SOURCE(3))
#define BT_SOURCE_SRV_CALL_AUDIO_STOP_TIMER_ID               (BT_TIMER_EXT_GROUP_SOURCE(4))
#define BT_SOURCE_SRV_MUSIC_DETECT_MEDIA_DATA_TIMER_ID       (BT_TIMER_EXT_GROUP_SOURCE(5))
#endif

#define BT_GATTC_DISCOVERY_TIMER_ID                                    (BT_TIMER_EXT_GROUP_GATTC(1))
#define BT_GATTC_TRIGGER_DISCOVERY_TIMER_ID                            (BT_TIMER_EXT_GROUP_GATTC(2))
/**
 * @brief                  Timeout callback function prototype
 * @param[in] timer_id     Timer ID
 * @param[in] data         User data saved in timer instance
 * @return                 None
 */
typedef void (*bt_timer_ext_timeout_callback_t)(uint32_t timer_id, uint32_t data);

/**
 * @brief                           Timeout instance structure
 */
BT_PACKED(
typedef struct _bt_timer_ext_t{
    uint32_t timer_id;                /**<  module id + module defined id */
    uint32_t data;                    /**<  user data */
    uint32_t time_tick;               /**<  timer timeout in tick */
    bt_timer_ext_timeout_callback_t cb;       /**<  timer timeout callback function */
}) bt_timer_ext_t;


/**
 * @brief   Function for initializing the BT external timer module. It is better to invoke this function when bootup and it shuold be called one time in a project.
 * @param[in] void.
 * @return    None
 */
void bt_timer_ext_init(void);

/**
 * @brief                   To start a timer
 * @param[in] timer_id      Timer ID
 * @param[in] data          User data saved in timer instance
 * @param[in] time_ms       timer timeout in ms
 * @param[in] cb            timer timeout callback function
 * @return                  bt_timer_ext_STATUS_SUCCESS if add timer success
                            bt_timer_ext_STATUS_OOM if timer reach max count
                            bt_timer_ext_STATUS_FAIL if double start
 */
bt_timer_ext_status_t bt_timer_ext_start(uint32_t timer_id, uint32_t data, uint32_t time_ms, bt_timer_ext_timeout_callback_t cb);

/**
 * @brief                   Stop a timer
 * @param[in] timer_id      Timer ID
 * @return                  bt_timer_ext_STATUS_SUCCESS if cancel timer success
                            bt_timer_ext_STATUS_FAIL if not found
 */
bt_timer_ext_status_t bt_timer_ext_stop(uint32_t timer_id);

/**
 * @brief                This function handle the bt external timer interrupt.
 * @return               None.
 */
void bt_timer_ext_handle_interrupt(void);

/**
 * @brief                   Find a timer
 * @param[in] timer_id      Timer ID
 * @return                  A pointer to the timer instance
 */
bt_timer_ext_t *bt_timer_ext_find(uint32_t timer_id);



#ifdef __cplusplus
}
#endif

#endif /* BT_TIMER_EXTERNAL_H */


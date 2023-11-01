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

#ifndef __APP_LEA_SERVICE_EVENT_H__
#define __APP_LEA_SERVICE_EVENT_H__

/**
 * File: app_lea_service_event.h
 *
 * Description: This file defines the UI Shell event for LE Audio.
 *
 */
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    EVENT_ID_LE_AUDIO_ADV_TIMER = 0,                    /**<  0. LE AUDIO ADV Timer ID. */
    EVENT_ID_LE_AUDIO_START_ADV,                        /**<  LE AUDIO Start ADV ID. */
    EVENT_ID_LE_AUDIO_STOP_ADV,                         /**<  LE AUDIO Stop ADV ID. */
    EVENT_ID_LE_AUDIO_RESTART_ADV,                      /**<  LE AUDIO Restart ADV ID. */
    EVENT_ID_LE_AUDIO_CONTROL_ADV_DATA,                 /**<  LE AUDIO Control ADV Data Event. */
    EVENT_ID_LE_AUDIO_SYNC_INFO,                        /**<  5. LE AUDIO Synchronize the ADV state and connection. */
    EVENT_ID_LE_AUDIO_SYNC_DISCONNECT_PARAM,            /**<  LE AUDIO Synchronize disconnect_parameter. */
    EVENT_ID_LE_AUDIO_RESET_DONGLE_BUSY_EVENT,
    EVENT_ID_LE_AUDIO_DELAY_RESTART_ADV,
    EVENT_ID_LE_AUDIO_SYNC_CONNECT_ADDR,

    EVENT_ID_LE_AUDIO_BIS_START_STREAMING,              /**<  10. LE AUDIO BIS start streaming. */
    EVENT_ID_LE_AUDIO_BIS_STOP_STREAMING,               /**<  LE AUDIO BIS stop streaming. */
    EVENT_ID_LE_AUDIO_BIS_SYNC_SRC_ADDR,                /**<  LE AUDIO BIS sync SRC addr. */
    EVENT_ID_LE_AUDIO_BIS_SCAN_TIMEOUT,                 /**<  LE AUDIO BIS scan timeout. */
    EVENT_ID_LE_AUDIO_BIS_SCAN_STOPPED,                 /**<  LE AUDIO BIS scan stopped. */
    EVENT_ID_LE_AUDIO_BIS_SCAN_WHITE_LIST,              /**<  15. LE AUDIO BIS scan white list. */
    EVENT_ID_LE_AUDIO_BIS_SYNC_TO_PEER,                 /**<  LE AUDIO BIS sync info to peer. */
    EVENT_ID_LE_AUDIO_BIS_SYNC_FEATURE,                 /**<  LE AUDIO BIS sync feature. */
    EVENT_ID_LE_AUDIO_BIS_ERROR,                        /**<  LE AUDIO BIS error event. */
    EVENT_ID_LE_AUDIO_NOTIFY_BATTERY,                   /**<  LE AUDIO Notify battery via HFP AT CMD. */
    EVENT_ID_LEA_ULL_PAIR_MODE,                         /**<  20. LE AUDIO Notify battery via HFP AT CMD. */
    EVENT_ID_LEA_ULL_RECONNECT_MODE,                    /**<  Set LE ULL reconnect flag in adv. */
    EVENT_ID_LE_AUDIO_BIS_STOP_PA,                      /**<  LE AUDIO BIS stop sync to pa. */
    EVENT_ID_LEA_SYNC_BOND_INFO,                        /**<  Sync LE AUDIO bond info list. */
    EVENT_ID_LEA_SYNC_IDA_INFO,                         /**<  Sync LE AUDIO Dual mode device list. */
    EVENT_ID_LEA_FORCE_UPDATE_ADV,                      /**<  25. Force Update ADV. */
    EVENT_ID_LEA_ALLOW_SET_RSL,                         /**<  Allow to set RSL after LE GAP SRV handle done. */
    EVENT_ID_LEA_CHANGE_ADV_SUB_MODE,                   /**<  Event of ADV sub mode change. */
    EVENT_ID_LEA_EVO_ADV_FAST_TIMEOUT,                  /**<  Event of Intel EVO ADV fast interval timeout. */
    EVENT_ID_LEA_SYNC_HFP_CONTEXT,                      /**<  Sync HFP context for MTK HFP AT CMD feature. */
    EVENT_ID_LE_AUDIO_DISCONNECT_DONE,                  /**<  30. Notify LE Audio disconnect done. */
    EVENT_ID_LEA_SYNC_FEATURE_STATE,                    /**<  Sync feature state to peer. */
    EVENT_ID_LEA_CLOSE_LID_ACTION,                      /**<  Event of close lid action. */
    EVENT_ID_LEA_SYNC_RECONNECT_TARGETED_ADDR,          /**<  Sync LE AUDIO remove unactive addr. */
};

#ifdef __cplusplus
}
#endif

#endif /* __APP_LEA_SERVICE_EVENT_H__ */

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

#ifndef __APP_BT_SOURCE_CONFIG_H__
#define __APP_BT_SOURCE_CONFIG_H__

/**
 * File: app_bt_source_config.h
 *
 * Description: This file defines the configure about BT Source APP.
 *
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif



#define APP_BT_SOURCE_SCAN_POLICY_MAX_RSSI                  0
#define APP_BT_SOURCE_SCAN_POLICY_FIRST_DEVICE              1
#define APP_BT_SOURCE_SCAN_POLICY_BY_AT_CMD                 2
#define APP_BT_SOURCE_SCAN_POLICY_BY_TOOL                   3

#define APP_BT_SOURCE_SCAN_RSSI_MAX_NUM                     5
#define APP_BT_SOURCE_SCAN_RSSI_CHECK_TIME                  (5 * 1000)      // only for SCAN_POLICY_MAX_RSSI
#define APP_BT_SOURCE_SCAN_RSSI_THRESHOLD                   (-55)           // only for SCAN_POLICY_MAX_RSSI, unit dBm, same as AIR_PAIRING

#define APP_BT_SOURCE_SCAN_TOOL_MAX_NUM                     10

/*
 * The BT Source APP scan policy.
 *
 * 0: Default (MAX RSSI)
 *    Filter those devices which its RSSI is lower than APP_BT_SOURCE_SCAN_RSSI_THRESHOLD.
 *    After APP_BT_SOURCE_SCAN_RSSI_CHECK_TIME or scaned num >= APP_BT_SOURCE_SCAN_RSSI_MAX_NUM, check all devices and select max RSSI device to connect
 *    After APP_BT_SOURCE_SCAN_RSSI_CHECK_TIME, if no device, rescan and check after APP_BT_SOURCE_SCAN_RSSI_CHECK_TIME
 * 1: FIRST_DEVICE
 *    Select first scanned device to connection
 * 2: Connect via AT CMD
 *    Cannot scan EDR device
 *    In either policy, disable scanning as long as input connect AT CMD
 * 3: Scan/Connect via PC Tool with RACE CMD
 */
#define APP_BT_SOURCE_SCAN_POLICY                           APP_BT_SOURCE_SCAN_POLICY_BY_TOOL

#define APP_BT_SOURCE_POWER_ON_RECONNECT_TIME               (60 * 1000)
#define APP_BT_SOURCE_LINK_LOST_RECONNECT_TIME              (2 * 60 * 1000)

#define APP_BT_SOURCE_RESET_RESTART_SCAN_TIME               (500)

#define APP_BT_SOURCE_CALL_MAX_NUM                          2

// Deprecated, always use dongle_bt_cm to control start/stop source even if only enable BT Source feature
//#if defined(AIR_BT_SOURCE_ENABLE) && !defined(AIR_LE_AUDIO_ENABLE) && !defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) && !defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
//#define APP_BT_SOURCE_ONLY_MODE
//#endif

#ifdef __cplusplus
}
#endif

#endif /* __APP_BT_SOURCE_CONFIG_H__ */

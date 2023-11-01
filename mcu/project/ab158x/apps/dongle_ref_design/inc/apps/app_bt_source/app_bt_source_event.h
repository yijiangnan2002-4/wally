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

#ifndef __APP_BT_SOURCE_EVENT_H__
#define __APP_BT_SOURCE_EVENT_H__

/**
 * File: app_bt_source.h
 *
 * Description: This file defines the interface of app_bt_source.c.
 *
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "app_bt_source_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BOOL
#define BOOL        bool
#endif

#ifndef TRUE
#define TRUE        true
#endif

#ifndef FALSE
#define FALSE       false
#endif

#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

enum {
    APP_BT_SOURCE_EVENT_CONNECT_DEVICE,                         /**<  The event to trigger connect device after BT Source scan ready. */
    APP_BT_SOURCE_EVENT_CHECK_RSSI_TIMEOUT,                     /**<  The event to trigger connect device after BT Source scan and APP_BT_SOURCE_SCAN_RSSI_CHECK_TIME timeout. */

    APP_BT_SOURCE_EVENT_RESET,                                  /**<  The event to trigger whole BT Source reset and restart scan. */
    APP_BT_SOURCE_EVENT_RESET_RESTART_SCAN,                     /**<  The event to trigger restart scan after RESET. */
    APP_BT_SOURCE_EVENT_ENABLE,                                 /**<  The event to trigger re-enable after ULL1/LEA/ULL2 Disable BT Source. */
    APP_BT_SOURCE_EVENT_DISABLE,                                /**<  The event to trigger disable BT Source when ULL1/LEA/ULL2 available. */
    APP_BT_SOURCE_EVENT_MUSIC_SUSPEND_STREAM_TIMER,             /**<  The event to start music SUSPEND_STREAM timer. */
    APP_BT_SOURCE_EVENT_TOOL_SCAN_TIMER,                        /**<  The event to start tool scan timer. */
    APP_BT_SOURCE_EVENT_TOOL_SCANNED,                           /**<  The event to start tool scanned. */
    APP_BT_SOURCE_EVENT_TOOL_READ_NAME,                         /**<  The event to start read name. */
    APP_BT_SOURCE_EVENT_TOOL_READ_NAME_TIMEOUT,                 /**<  The event to read name timer. */

    APP_BT_SOURCE_EVENT_RECONNECT_TIMEOUT = 0x100,              /**<  The event to trigger reconnect device timeout event. */
    APP_BT_SOURCE_EVENT_RECONNECT_TIMEOUT_END = 0x110,

    APP_BT_SOURCE_EVENT_PUBLIC_EVENT_BASE = 0x200,
    APP_BT_SOURCE_EVENT_NOTIFY_CONN_CONNECTED = APP_BT_SOURCE_EVENT_PUBLIC_EVENT_BASE,         /**<  The event to notify BT Source connected. */
    APP_BT_SOURCE_EVENT_NOTIFY_CONN_DISCONNECTED,               /**<  The event to notify BT Source disconnected. */
    APP_BT_SOURCE_EVENT_DELAY_HOLD,
};

#ifdef __cplusplus
}
#endif

#endif /* __APP_BT_SOURCE_EVENT_H__ */

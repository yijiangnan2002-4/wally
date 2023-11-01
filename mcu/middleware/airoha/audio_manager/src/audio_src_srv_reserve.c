/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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

/* Includes ------------------------------------------------------------------*/

#include <string.h>
#include "stdint.h"
#include "audio_src_srv.h"
#include "audio_src_srv_internal.h"
#include "audio_log.h"
#include "bt_sink_srv.h"
#include "bt_sink_srv_state_manager.h"

static bt_sink_srv_allow_result_t default_bt_sink_srv_state_manager_psedev_compare(audio_src_srv_handle_t *current, audio_src_srv_handle_t *coming);
static void deafult_bt_sink_srv_state_manager_running_psedev_change(audio_src_srv_handle_t *running);

#if _MSC_VER >= 1500
#pragma comment(linker, "/alternatename:_bt_sink_srv_state_manager_psedev_compare=_default_bt_sink_srv_state_manager_psedev_compare")
#pragma comment(linker, "/alternatename:_bt_sink_srv_state_manager_running_psedev_change=_default_bt_sink_srv_state_manager_running_psedev_change")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__ARMCC_VERSION) || defined(__CC_ARM)
#pragma weak bt_sink_srv_state_manager_psedev_compare = default_bt_sink_srv_state_manager_psedev_compare
#pragma weak bt_sink_srv_state_manager_running_psedev_change = deafult_bt_sink_srv_state_manager_running_psedev_change
#else
#error "Unsupported Platform"
#endif

bool audio_src_srv_psedev_compare(audio_src_srv_handle_t *cur, audio_src_srv_handle_t *coming)
{
    bt_sink_srv_allow_result_t result = bt_sink_srv_state_manager_psedev_compare(cur, coming);

    if (BT_SINK_SRV_ALLOW_RESULT_BYPASS == result) {
        #ifndef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
        if (cur->priority == coming->priority) {
            return (coming->play_count > cur->play_count);
        } else
        #endif
        {
            return (coming->priority > cur->priority);
        }
    }

    return (result == BT_SINK_SRV_ALLOW_RESULT_ALLOW);
}

void audio_src_srv_running_psedev_change(audio_src_srv_handle_t *running)
{
    /* workaround: suspend line out or usb out */
    #if defined(AIR_LINE_OUT_ENABLE)
    extern void app_line_out_unsafe_suspend_by_music(void);
    if (running && running->type == AUDIO_SRC_SRV_PSEUDO_DEVICE_A2DP) {
        app_line_out_unsafe_suspend_by_music();
    }
    #endif
    #if defined(AIR_USB_AUDIO_OUT_ENABLE)
    extern void app_usb_out_unsafe_suspend_by_music(void);
    if (running && running->type == AUDIO_SRC_SRV_PSEUDO_DEVICE_A2DP) {
        app_usb_out_unsafe_suspend_by_music();
    }
    #endif
    /* workaround end */
    bt_sink_srv_state_manager_running_psedev_change(running);
}

static bt_sink_srv_allow_result_t default_bt_sink_srv_state_manager_psedev_compare(audio_src_srv_handle_t *current, audio_src_srv_handle_t *coming)
{
    if (AUDIO_SRC_SRV_PSEUDO_DEVICE_HFP == current->type && AUDIO_SRC_SRV_PSEUDO_DEVICE_HFP == coming->type) {
        return BT_SINK_SRV_ALLOW_RESULT_DISALLOW;
    }

    return BT_SINK_SRV_ALLOW_RESULT_BYPASS;
}

static void deafult_bt_sink_srv_state_manager_running_psedev_change(audio_src_srv_handle_t *running)
{
    return;
}


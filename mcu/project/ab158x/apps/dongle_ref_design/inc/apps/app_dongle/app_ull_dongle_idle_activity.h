/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
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

#ifndef __APP_ULL_DONGLE_IDLE_ACTIVITY_H__
#define __APP_ULL_DONGLE_IDLE_ACTIVITY_H__
#include "stdint.h"
#include "ui_shell_activity.h"
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
#include "bt_ull_service.h"
#endif

#define APP_DONGLE_MODE_XBOX_PS               0
#define APP_DONGLE_MODE_PC                    1

/**
* @brief      This function is the interface of the app_ull_dongle_idle_activity, and is only called by ui_shell when events are sent.
* @param[in]  self, the context pointer of the activity.
* @param[in]  event_group, the current event group to be handled.
* @param[in]  event_id, the current event ID to be handled.
* @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
* @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
* @return     If return true, the current event cannot be handle by the next activity.
*/
bool app_ull_dongle_idle_activity_proc(ui_shell_activity_t *self,
                                       uint32_t event_group,
                                       uint32_t event_id,
                                       void *extra_data,
                                       size_t data_len);

typedef enum {
    ULL_EVT_DONGLE_MODE = 1,
    ULL_EVT_END,
} ull_user_evt_t;

typedef struct {
    ull_user_evt_t user_evt;
    uint16_t data_len;
    uint8_t data[1];
} app_ull_user_data_t;

#ifdef AIR_ULL_DONGLE_LINE_OUT_ENABLE
/**
 * @brief      The function to set the values of the ull streaming volume, which range is 0~100.
* @param[in]   value, the volume of line out streaming.
 */
void app_ull_dongle_set_lineout_volume_value(uint8_t value);
#endif

#ifdef AIR_ULL_DONGLE_LINE_IN_ENABLE
/**
 * @brief      The function to set the values of the ull streaming volume, which range is 0~100.
* @param[in]   value, the volume of line in streaming.
 */
void app_ull_dongle_set_linein_volume_value(uint8_t value);
/**
 * @brief      The function to change the level of the ull streaming volume.
 * @param[in]  up, if is true, which is up the volume.
 */
void app_ull_dongle_change_linein_volume_level(bool up);

/**
 * @brief      The function to change the mix ratio of the ull streaming.
 */
void app_ull_dongle_change_mix_ratio(void);
#endif

#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
/**
 * @brief      The function to change the level of the ull streaming volume.
 * @param[in]  up, if is true, which is up the volume.
 * @interface[in] interface, the interface that want to change.
 */
void app_ull_dongle_change_volume_level_for_interface(bool up, bt_ull_streaming_interface_t interface);
#endif

#ifdef AIR_MS_GIP_ENABLE
void dongle_switch_det_init(void);
#endif /* AIR_MS_GIP_ENABLE */

#endif /* __APP_ULL_DONGLE_IDLE_ACTIVITY_H__ */
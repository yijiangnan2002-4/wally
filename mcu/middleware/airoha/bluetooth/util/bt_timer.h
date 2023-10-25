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
/* MediaTek restricted information */

#ifndef __BT_TIMER_H__
#define __BT_TIMER_H__

#include "bt_platform_internal.h"
#include <stdbool.h>
#include"bt_linknode.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup Bluetooth
 * @{
 * @addtogroup Util
 * @{
 * @defgroup Timer
 * @{
 * This document describes Timer APIs.
 * Timer is created for Hummingbird internal use only, it provides start, cancel, cancel and callback APIs.
 * @section bt_timer_api_usage How to use this module
 *
 * - bt_timer_start() is used for starting a timer. Need to pass timer id, user data, timeout time, and timeout callback
 *  - Sample code:
 *     @code
 * bt_status_t bt_gap_le_bonding_reply(uint32_t handle, bt_gap_le_bonding_reply_t const *const rsp)
 * {
 *     BT_COLOR_SET(BT_COLOR_RED);
 *
 *     bt_status_t status = BT_STATUS_FAIL;
 *     if (bt_hummingbird.sm_session.connection == bt_find_conn_by_handle(handle, false)) {
 *         ...
 *         status = bt_timer_start(BT_GAP_LE_EVENT_APP_INPUT, BT_NULL, 0, bt_gap_le_bond_reply_proc);
 *     }
 *     return status;
 * }
 *     @endcode
 *
 *
 */

/**
 * @addtogroup bt_timer_typedef Typedef
 * @{
 */

/**
 * @brief                  Timeout callback function prototype
 * @param[in] is_timeout   True : the timeout callback is invoked by timer timeout, False : the timeout callback is invoked by cancel timer
 * @param[in] timer_id     Timer ID
 * @param[in] data         User data saved in timer instance
 * @param[in] arg          Function argument passed when invoke bt_timer_cancel_and_callback()
 * @return                 If the operation was successful, the returned value is #BT_STATUS_SUCCESS, otherwise the returned value is #BT_STATUS_FAIL.
 */
typedef bt_status_t (*bt_timeout_callback_t)(uint32_t is_timeout,
                                             uint32_t timer_id,
                                             uint32_t data,
                                             const void *arg);
/**
 * @}
 */

#define BT_TIMER_FLAG_RHO_NOT_SYNC  (0x00000001 << 0)   /**< The flag to indicate not sync to partner when RHO happened. */
typedef uint32_t bt_timer_flag_sets_t;

/**
 * @addtogroup bt_timer_struct Structure
 * @{
 */

/**
 * @brief                           Timeout instance structure
 */
typedef struct _bt_timer_t {
    struct _bt_timer_t *front;             /**<  link to next timer instance */
    uint32_t timer_id;              /**<  module id + module defined id */
    uint32_t data;                  /**<  user data */
    uint32_t time_ms;               /**<  timer timeout in ms */
    bt_timeout_callback_t cb;       /**<  timer timeout callback function */
    uint32_t flags_set;             /**<  bt timer related special flags set. */
} bt_timer_t;

/**
 * @}
 */


/**
 * @brief                   To start a timer, the timer will be inserted  into a timer list sorting by time_ms
 * @param[in] timer_id      Timer ID
 * @param[in] data          User data saved in timer instance
 * @param[in] time_ms       timer timeout in ms
 * @param[in] cb            timer timeout callback function
 * @return                  return  BT_STATUS_SUCCESS if add timer success, return BT_STATUS_OUT_OF_MEMORY if timer reach max count
 */
bt_status_t bt_timer_start(uint32_t timer_id, uint32_t data,
                           uint32_t time_ms, bt_timeout_callback_t cb);

/**
 * @brief                   Will extract the first timer instance with timer_id in the timer list
 * @param[in] timer_id      Timer ID
 * @return                  return  BT_STATUS_SUCCESS if add timer success, return BT_STATUS_TIMER_NOT_FOUND if timer instance not found
 */
bt_status_t bt_timer_cancel(uint32_t timer_id);

/**
 * @brief                   Will extract the first timer instance with timer_id in the timer list and invoke the timer timeout callback with the argument passed
 * @param[in] timer_id      Timer ID
 * @param[in] arg           Function argument passed to timer timeout callback function
 * @return                  return  BT_STATUS_SUCCESS if add timer success, return BT_STATUS_TIMER_NOT_FOUND if timer instance not found
 */
bt_status_t bt_timer_cancel_and_callback(uint32_t timer_id, const void *arg);


/**
 * @brief   To calculate time difference, extract timeout instance from the timer list and move to expire list.
 * @return  void
 */
void bt_timer_check_timeout_handler(void);

bt_timer_t *bt_timer_find(uint32_t timer_id);

bt_timer_t *bt_timer_find_with_cmp_callback(uint32_t timer_factor, bt_linknode_cmp_t callback);

/**
 * @brief   Stop all timer.
 * @return  void
 */
void bt_timer_stop_all_timer(void);

bt_timer_t *bt_timer_find(uint32_t timer_id);

bool bt_time_backward_cmp_duration(const bt_linknode_t *node, const void *data);
bool bt_time_backward_cmp_id(const bt_linknode_t *node, const void *data);
bt_status_t bt_timer_set_flags(uint32_t timer_id, bt_timer_flag_sets_t flags);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

/**
 * @}
 * @}
 * @}
 */

#endif /*__BT_TIMER_H__*/

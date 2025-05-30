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

/**
 * File: app_fota_idle_activity.h
 *
 * Description: This file defines the interface of app_fota_idle_activity.c.
 *
 * Note: See doc/AB1565_AB1568_Earbuds_Reference_Design_User_Guide.pdf for more detail.
 *
 */

#ifndef __UI_SHELL_FOTA_IDLE_ACTIVITY_H__
#define __UI_SHELL_FOTA_IDLE_ACTIVITY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ui_shell_activity.h"

/**
 *  @brief This enum defines the state of fota app.
 */
typedef enum {
    APP_FOTA_STATE_IDLE = 0,      /**<  Idle state and do nothing. */
    APP_FOTA_STATE_START,         /**<  Fota is downloading. */
    APP_FOTA_STATE_CANCELLING,    /**<  Fota is cancelling. */
    APP_FOTA_STATE_CANCELLED      /**<  Fota is cancelled. */
} app_fota_state_t;

// richard for customer UI spec.
typedef enum
{
    FOTA_STATE_IDLE = 0,
    FOTA_STATE_RUNNING,
} app_sensor_fota_state_t;

/**
 * @brief      This function is the interface of the app_fota_idle_activity, and is only called by ui_shell when events are sent.
 * @param[in]  self, the context pointer of the activity.
 * @param[in]  event_group, the current event group to be handled.
 * @param[in]  event_id, the current event ID to be handled.
 * @param[in]  extra_data, extra data pointer of the current event, NULL means there is no extra data.
 * @param[in]  data_len, the length of the extra data. 0 means extra_data is NULL.
 * @return     If return true, the current event cannot be handle by the next activity.
 */
bool app_fota_idle_activity_proc(struct _ui_shell_activity *self,
                                 uint32_t event_group,
                                 uint32_t event_id,
                                 void *extra_data,
                                 size_t data_len);

/**
 * @brief      This function set the FOTA ongoing state.
 * @param[in]  ongoing, the FOTA ongoing state.
 */
void app_fota_set_ota_ongoing(bool ongoing);

/**
 * @brief      This function get the FOTA ongoing state.
 * @return     If return true, the FOTA is ongoing.
 */
bool app_fota_get_ota_ongoing(void);

#ifdef MTK_AWS_MCE_ENABLE
/**
 * @brief      This function to exit bt sniff mode.
 * @return     None.
 */
void app_fota_bt_exit_sniff_mode(void);

/**
 * @brief      This function to switch bt sniff mode.
 * @param[in]  enable, if enable is true, which is enable sniff mode.
 * @return     None.
 */

void app_fota_bt_switch_sniff_mode(bool enable);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __UI_SHELL_FOTA_IDLE_ACTIVITY_H__ */


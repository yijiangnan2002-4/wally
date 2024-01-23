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

#ifdef AIR_LE_AUDIO_ENABLE
#include "app_le_audio.h"
#include "app_le_audio_utillity.h"
#include "bt_le_audio_msglog.h"

#ifdef AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
#include "apps_events_interaction_event.h"
#include "apps_events_line_in_event.h"

/**************************************************************************************************
* Define
**************************************************************************************************/


/**************************************************************************************************
* Structure
**************************************************************************************************/

/**************************************************************************************************
* Variable
**************************************************************************************************/



/**************************************************************************************************
* Prototype
**************************************************************************************************/


/**************************************************************************************************
* Static Functions
**************************************************************************************************/
static void app_le_audio_line_in_start(void)
{
    app_le_audio_set_streaming_port_mask(APP_LE_AUDIO_STREAM_PORT_MASK_LINE_IN);
    app_le_audio_start_streaming_port(APP_LE_AUDIO_STREAM_PORT_LINE_IN);
}

static void app_le_audio_line_in_stop(void)
{
    app_le_audio_clear_streaming_port_mask(APP_LE_AUDIO_STREAM_PORT_MASK_LINE_IN);
    app_le_audio_stop_streaming_port(APP_LE_AUDIO_STREAM_PORT_LINE_IN);
}

/**************************************************************************************************
* Public Functions
**************************************************************************************************/
void app_le_audio_idle_line_in_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    bt_status_t status = BT_STATUS_FAIL;
    LE_AUDIO_MSGLOG_I("[APP][LINE] line_in_event_proc, event_id:%x extra_data:%x", 2, event_id, (bool)extra_data);
    switch (event_id) {
        case APPS_EVENTS_INTERACTION_LINE_IN_STATUS: {
            bool line_in_status = (bool )extra_data;
            if (line_in_status) {
                app_le_audio_line_in_start();
            } else {
                app_le_audio_line_in_stop();
            }
            break;
        }
        case APPS_EVENTS_INTERACTION_LINE_IN_VOLUME: {
            app_line_in_volume_t* line_in_vol_data = (app_line_in_volume_t *)extra_data;
            if (line_in_vol_data->vol_src == APP_LINE_IN_VOL_SRC && line_in_vol_data->vol_action == APP_LINE_IN_VOL_SET) {
                LE_AUDIO_MSGLOG_I("[APP][LINE] line_in_event_proc, set line-in volum levek:%d", 1, line_in_vol_data->vol_level);
                status = app_le_audio_set_audio_transmitter_volume_level(APP_LE_AUDIO_STREAM_PORT_LINE_IN, line_in_vol_data->vol_level, line_in_vol_data->vol_level);
                if (status != BT_STATUS_SUCCESS) {
                    LE_AUDIO_MSGLOG_W("[APP][LINE] line_in_event_proc, set line-in volum fail, status:%d", 1, status);
                    return;
                }
            }
            break;
        }
        default:
            break;
    }

    return;
}
#endif

#endif


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

#ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
#include "apps_events_i2s_in_event.h"

/**************************************************************************************************
* Define
**************************************************************************************************/


/**************************************************************************************************
* Structure
**************************************************************************************************/

/**************************************************************************************************
* Variable
**************************************************************************************************/
//app_i2s_in_det_t g_i2s_in_params;

/**************************************************************************************************
* Prototype
**************************************************************************************************/


/**************************************************************************************************
* Static Functions
**************************************************************************************************/
static void app_le_audio_i2s_in_start(app_i2s_in_det_t *i2s_in_param)
{
    app_le_audio_set_streaming_port_mask(APP_LE_AUDIO_STREAM_PORT_MASK_I2S_IN);
    //memcpy(&g_i2s_in_params, i2s_in_param, sizeof(app_i2s_in_det_t));
    app_le_audio_start_streaming_port(APP_LE_AUDIO_STREAM_PORT_I2S_IN);
}

static void app_le_audio_i2s_in_stop(void)
{
    app_le_audio_clear_streaming_port_mask(APP_LE_AUDIO_STREAM_PORT_MASK_I2S_IN);
    app_le_audio_stop_streaming_port(APP_LE_AUDIO_STREAM_PORT_I2S_IN);
    //app_le_audio_stop_and_deinit_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_I2S_IN);
}

/**************************************************************************************************
* Public Functions
**************************************************************************************************/
void app_le_audio_idle_i2s_in_event_proc(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    if (extra_data == NULL) {
        return;
    }
    bt_status_t status = BT_STATUS_FAIL;
    LE_AUDIO_MSGLOG_I("[APP][I2S] i2s_in_event_proc, event_id:%x", 1, event_id);
    switch (event_id) {
        case APPS_EVENTS_I2S_IN_STATUS_CHANGE: {
            app_i2s_in_det_t* i2s_in_param = (app_i2s_in_det_t *)extra_data;
            LE_AUDIO_MSGLOG_I("[APP][I2S] i2s_in_event_proc, i2s_in_param.i2s_state = %d, i2s_device = %x i2s_interface = %x i2s_famart = %x", 4,
                    i2s_in_param->i2s_state, i2s_in_param->i2s_device, i2s_in_param->i2s_interface, i2s_in_param->i2s_famart);

            if (i2s_in_param->i2s_state) {
                app_le_audio_i2s_in_start(i2s_in_param);
            } else {
                app_le_audio_i2s_in_stop();
            }
            break;
        }
        case APPS_EVENTS_I2S_IN_VOLUME_CHANGE: {
            app_i2s_in_vol_t* i2s_in_vol = (app_i2s_in_vol_t *)extra_data;
            LE_AUDIO_MSGLOG_I("[APP][I2S] i2s_in_event_proc, i2s_in_vol.vol_port:%d, vol_action:%d, vol_level:%d, vol_src:%d", 4,
                    i2s_in_vol->vol_port, i2s_in_vol->vol_action, i2s_in_vol->vol_level, i2s_in_vol->vol_src);
            if (i2s_in_vol->vol_src == APP_I2S_IN_VOL_SRC && i2s_in_vol->vol_action == APP_I2S_IN_VOL_SET) {
                LE_AUDIO_MSGLOG_I("[APP][I2S] i2s_in_event_proc, set i2s volume level:%d", 1, i2s_in_vol->vol_level);
                status = app_le_audio_set_audio_transmitter_volume_level(APP_LE_AUDIO_STREAM_PORT_LINE_IN, i2s_in_vol->vol_level, i2s_in_vol->vol_level);
                if (status != BT_STATUS_SUCCESS) {
                    LE_AUDIO_MSGLOG_W("[APP][I2S] i2s_in_event_proc, set i2s volume level fail, status:%d", 1, status);
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


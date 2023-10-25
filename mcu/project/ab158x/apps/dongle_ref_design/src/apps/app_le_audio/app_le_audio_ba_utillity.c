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
#ifdef AIR_LE_AUDIO_ENABLE

#include "app_le_audio_ba_utillity.h"

#include "bt_le_audio_def.h"
#include "bt_le_audio_msglog.h"

/**************************************************************************************************
* Define
**************************************************************************************************/

/**************************************************************************************************
* Structure
**************************************************************************************************/

/**************************************************************************************************
* Variable
**************************************************************************************************/
static uint8_t g_lea_ba_mode = APP_LE_AUDIO_BA_NOT_SUPPORT_MODE;

static app_le_audio_ba_stream_info_t g_lea_ba_stream_info = {0};

static app_le_audio_ba_link_info_t g_lea_ba_link_info[APP_LE_AUDIO_BA_LINK_MAX_NUM] = {{0}};

static bool g_lea_ba_auto_play = true;

/**************************************************************************************************
* Prototype
**************************************************************************************************/

/**************************************************************************************************
* Static Functions
**************************************************************************************************/

/**************************************************************************************************
* Public Functions
**************************************************************************************************/
uint8_t app_le_audio_ba_check_state_if_exist(uint8_t ba_state)
{
    uint8_t i = 0;

    for (i = 0; i < APP_LE_AUDIO_BA_LINK_MAX_NUM; i++) {
        if (BT_HANDLE_INVALID != g_lea_ba_link_info[i].handle) {
            if (ba_state == g_lea_ba_link_info[i].ba_state) {
                return i;
            }
        }
    }

    return APP_LE_AUDIO_BA_INVALID_LINK_IDX;
}

uint8_t app_le_audio_ba_get_link_num(void)
{
    uint8_t i = 0, count = 0;

    for (i = 0; i < APP_LE_AUDIO_BA_LINK_MAX_NUM; i++) {
        if (BT_HANDLE_INVALID != g_lea_ba_link_info[i].handle) {
            count++;
        }
    }

    return count;
}

bt_handle_t app_le_audio_ba_get_handle_by_idx(uint8_t link_idx)
{
    if (APP_LE_AUDIO_BA_LINK_MAX_NUM > link_idx) {
        return g_lea_ba_link_info[link_idx].handle;
    }

    return BT_HANDLE_INVALID;
}

uint8_t app_le_audio_ba_get_link_idx_by_handle(bt_handle_t handle)
{
    uint8_t i = 0;

    if (BT_HANDLE_INVALID == handle) {
        return APP_LE_AUDIO_BA_INVALID_LINK_IDX;
    }

    for (i = 0; i < APP_LE_AUDIO_BA_LINK_MAX_NUM; i++) {
        if (handle == g_lea_ba_link_info[i].handle) {
            return i;
        }
    }

    return APP_LE_AUDIO_BA_INVALID_LINK_IDX;
}

app_le_audio_ba_link_info_t *app_le_audio_ba_get_link_info_by_handle(bt_handle_t handle)
{
    uint8_t i = 0;

    if (BT_HANDLE_INVALID == handle) {
        return NULL;
    }

    for (i = 0; i < APP_LE_AUDIO_BA_LINK_MAX_NUM; i++) {
        if (handle == g_lea_ba_link_info[i].handle) {
            return &g_lea_ba_link_info[i];
        }
    }

    return NULL;
}

app_le_audio_ba_link_info_t *app_le_audio_ba_get_link_info_by_idx(uint8_t link_idx)
{
    if (APP_LE_AUDIO_BA_LINK_MAX_NUM > link_idx) {
        return &g_lea_ba_link_info[link_idx];
    }
    return NULL;
}

app_le_audio_ba_stream_info_t *app_le_audio_ba_get_stream_info(void)
{
    return &g_lea_ba_stream_info;
}

void app_le_audio_ba_reset_link_info(uint8_t link_idx)
{
    if (APP_LE_AUDIO_BA_LINK_MAX_NUM > link_idx) {
        memset(&g_lea_ba_link_info[link_idx], 0, sizeof(app_le_audio_ba_link_info_t));
        g_lea_ba_link_info[link_idx].handle = BT_HANDLE_INVALID;
    }
}

uint8_t app_le_audio_ba_get_mode(void)
{
    return g_lea_ba_mode;
}

void app_le_audio_ba_set_mode(uint8_t mode)
{
    if (APP_LE_AUDIO_BA_COMMANDER_ONLY_MODE >= mode) {
        g_lea_ba_mode = mode;
    }
}

bool app_le_audio_ba_get_auto_play(void)
{
    return g_lea_ba_auto_play;
}

void app_le_audio_ba_set_auto_play(bool auto_play)
{
    g_lea_ba_auto_play = auto_play;
}

void app_le_audio_ba_utillity_init(void)
{
    uint8_t i = 0;

    /* reset link info */
    for (i = 0; i < APP_LE_AUDIO_BA_LINK_MAX_NUM; i++) {
        app_le_audio_ba_reset_link_info(i);
    }

    /* reset stream info */
    memset(&g_lea_ba_stream_info, 0, sizeof(app_le_audio_ba_stream_info_t));
}

#endif  /* AIR_LE_AUDIO_ENABLE */


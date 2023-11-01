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

/**
 * File: voice_prompt_local.c
 *
 * Description: This file provide implementation of VP local play and stop.
 *
 */

#include "hal.h"
#include "hal_gpt.h"
#include "hal_audio_internal.h"
#include "voice_prompt_nvdm.h"
#include "voice_prompt_local.h"
#include "prompt_control.h"
#include "rofs.h"
#include "bt_sink_srv_ami.h"
#include "voice_prompt_main.h"
#include "ui_realtime_task.h"
#include "syslog.h"

#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
#include "ui_shell_manager.h"
#include "app_hear_through_activity.h"
#include "apps_events_event_group.h"
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */

#define LOG_TAG "VP_LOCAL"

log_create_module(VOICE_PROMPT, PRINT_LEVEL_INFO);

static void voice_prompt_local_callback(prompt_control_event_t event_id)
{

    VP_LOG_MSGID_I(LOG_TAG" prompt_control callback event=%d", 1, event_id);

    switch (event_id) {
        case PROMPT_CONTROL_MEDIA_PLAY: {
            ui_realtime_send_msg(UI_REALTIME_MSG_TYPE_VP, UI_REALTIME_MSG_PLAY_START, NULL);
#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
            ui_shell_send_event(true,
                                EVENT_PRIORITY_HIGH,
                                EVENT_GROUP_UI_SHELL_HEAR_THROUGH,
                                APP_HEAR_THROUGH_EVENT_ID_VP_STREAMING_BEGIN,
                                NULL,
                                0,
                                NULL,
                                0);
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */
            break;
        }
        case PROMPT_CONTROL_MEDIA_END: {
            ui_realtime_send_msg(UI_REALTIME_MSG_TYPE_VP, UI_REALTIME_MSG_PLAY_END, NULL);
#ifdef AIR_HEARTHROUGH_MAIN_ENABLE
            ui_shell_send_event(true,
                                EVENT_PRIORITY_HIGH,
                                EVENT_GROUP_UI_SHELL_HEAR_THROUGH,
                                APP_HEAR_THROUGH_EVENT_ID_VP_STREAMING_END,
                                NULL,
                                0,
                                NULL,
                                0);
#endif /* AIR_HEARTHROUGH_MAIN_ENABLE */
            break;
        }
        default:
            break;
    }
}

voice_prompt_status_t voice_prompt_local_play(uint32_t vp_index, uint32_t target_gpt)
{
    uint8_t *tone_buf = NULL;
    uint32_t tone_size = 0;
    uint16_t file_id = 0;
    ROFS_FILEINFO_T *pMediaFile;
    uint32_t gpt_count = 0;
    prompt_control_tone_type_t codec_type = VPC_MP3;

    VP_LOG_MSGID_I(LOG_TAG" play index: 0x%x, gpt: (0x%08x)", 2, vp_index, target_gpt);

    /* Get ROFS file ID according to tone_index. */
    if (voice_prompt_nvdm_find_vp_file(vp_index, &file_id)) {
        /* Read VP file in ROFS. */
        pMediaFile = ROFS_fopen((unsigned short)file_id);
        if (pMediaFile) {
            tone_buf = (uint8_t *)ROFS_address(pMediaFile);
            tone_size = (uint32_t)pMediaFile->BasicInfo.ulFileSize;
            if (strstr(pMediaFile->szFileName, ".mp3")) {
                codec_type = VPC_MP3;
            } else if (strstr(pMediaFile->szFileName, ".wav")) {
                codec_type = VPC_WAV;
            } else if (strstr(pMediaFile->szFileName, ".opus")) {
                codec_type = VPC_OPUS;
            }  else {
                //VP_LOG_MSGID_W(LOG_TAG" play file type wrong", 0);
            }
            //VP_LOG_I(LOG_TAG" fn: %s, file id: %d, buf: 0x%08x, size: 0x%x", pMediaFile->szFileName, file_id, tone_buf, tone_size);
        } else {
            VP_LOG_MSGID_E(LOG_TAG" file non found with file id: 0x%x", 1, file_id);
            return VP_STATUS_FILE_NOT_FOUND;
        }
    } else {
        VP_LOG_MSGID_I(LOG_TAG" get file id fail with VP index %d", 1, vp_index);
        return VP_STATUS_FILE_NOT_FOUND;
    }

    if (tone_size != 0 && tone_buf != NULL) {
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);
        if (target_gpt != 0 && gpt_count >= target_gpt) {
            VP_LOG_MSGID_I(LOG_TAG" local play gpt(0x%08x) <= current(0x%08x)", 2, target_gpt, gpt_count);
            target_gpt = 0;
        }
        VP_LOG_MSGID_I(LOG_TAG" prompt_control_play_sync_tone +", 0);
#ifdef AIR_PROMPT_SOUND_ENABLE
        prompt_control_play_sync_tone(codec_type, tone_buf, tone_size, target_gpt, voice_prompt_local_callback);
#endif
        VP_LOG_MSGID_I(LOG_TAG" prompt_control_play_sync_tone -", 0);
        return VP_STATUS_SUCCESS;
    } else {
        return VP_STATUS_FAIL;
    }
}

voice_prompt_status_t voice_prompt_local_stop(uint32_t tar_gpt)
{
    uint32_t gpt_count = 0;
#ifdef AIR_BT_AUDIO_SYNC_ENABLE
    bt_sink_srv_am_audio_sync_capability_t cap = {0};
#endif

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);
    VP_LOG_MSGID_I(LOG_TAG" local stop gpt(0x%08x), current(0x%08x)", 2, tar_gpt, gpt_count);
    if (gpt_count >= tar_gpt) {
        tar_gpt = 0;
    }
#ifdef AIR_BT_AUDIO_SYNC_ENABLE
    if (tar_gpt != 0) {
        cap.sync_scenario_type = MCU2DSP_SYNC_REQUEST_VP;
        cap.sync_action_type = MCU2DSP_SYNC_REQUEST_STOP;
        cap.target_gpt_cnt = tar_gpt;
        bt_sink_srv_ami_audio_request_sync(FEATURE_NO_NEED_ID, &cap);
    }
#endif
#ifdef AIR_PROMPT_SOUND_ENABLE
    prompt_control_stop_tone();
#endif
    return VP_STATUS_SUCCESS;
}


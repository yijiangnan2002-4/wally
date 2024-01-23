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

#include "bt_sink_srv_call_pseudo_dev_mgr.h"
#include "bt_sink_srv_utils.h"
#include "bt_sink_srv_hf.h"
#include "bt_utils.h"
#include "bt_sink_srv_call.h"
#ifdef BT_SINK_DUAL_ANT_ENABLE
#include "bt_sink_srv_dual_ant.h"
#include "mux_ll_uart_latch.h"
#endif
bool bt_sink_srv_call_get_sidetone_enable_config(void);
static void bt_sink_srv_call_psd_play(audio_src_srv_handle_t *handle);
static void bt_sink_srv_call_psd_stop(audio_src_srv_handle_t *handle);
static void bt_sink_srv_call_psd_reject(audio_src_srv_handle_t *handle);
void bt_sink_srv_call_psd_suspend(audio_src_srv_handle_t *handle, audio_src_srv_handle_t *int_hd);
static void bt_sink_srv_call_psd_exception_handle(audio_src_srv_handle_t *handle, int32_t event, void *param);
static void bt_sink_srv_call_psd_run_next_state_in_state_ready(bt_sink_srv_call_pseudo_dev_t *dev);
static void bt_sink_srv_call_psd_set_sub_state(bt_sink_srv_call_pseudo_dev_t *dev, bt_sink_srv_call_pseudo_dev_sub_state_t sub_state);
static void bt_sink_srv_call_psd_stop_int(bt_sink_srv_call_pseudo_dev_t *dev);
void bt_sink_srv_call_psd_add_waiting_list(bt_sink_srv_call_pseudo_dev_t *device);
void bt_sink_srv_call_psd_del_waiting_list(bt_sink_srv_call_pseudo_dev_t *device);
void bt_sink_srv_call_psd_reset_device(bt_sink_srv_call_pseudo_dev_t *dev);
static void bt_sink_srv_call_psd_run_next_state_in_sub_state_playing_idle(bt_sink_srv_call_pseudo_dev_t *dev);
#ifdef BT_SINK_DUAL_ANT_ENABLE
extern bt_sink_srv_dual_ant_context_t bt_dual_ant_ctx;
#endif
extern bt_sink_srv_call_pseudo_dev_t bt_sink_srv_call_pseudo_dev[BT_SINK_SRV_CALL_PSD_NUM];

static void bt_sink_srv_call_psd_sub_state_mic_resource_taking(bt_sink_srv_call_pseudo_dev_t* dev,bt_sink_srv_call_state_event_t event,void* data);
static void bt_sink_srv_call_psd_sub_state_playing_idle( bt_sink_srv_call_pseudo_dev_t *dev, bt_sink_srv_call_state_event_t event,void *data);
static void bt_sink_srv_call_psd_sub_state_sco_activating( bt_sink_srv_call_pseudo_dev_t *dev, bt_sink_srv_call_state_event_t event,void *data);
static void bt_sink_srv_call_psd_sub_state_codec_starting( bt_sink_srv_call_pseudo_dev_t *dev, bt_sink_srv_call_state_event_t event,void *data);
static void bt_sink_srv_call_psd_sub_state_playing( bt_sink_srv_call_pseudo_dev_t *dev, bt_sink_srv_call_state_event_t event,void *data);
static void bt_sink_srv_call_psd_sub_state_codec_stopping( bt_sink_srv_call_pseudo_dev_t *dev, bt_sink_srv_call_state_event_t event,void *data);
static void bt_sink_srv_call_psd_sub_state_sco_deactivating( bt_sink_srv_call_pseudo_dev_t *dev, bt_sink_srv_call_state_event_t event,void *data);
static void bt_sink_srv_call_psd_state_none( bt_sink_srv_call_pseudo_dev_t *dev, bt_sink_srv_call_state_event_t event,void *data);
static void bt_sink_srv_call_psd_state_ready( bt_sink_srv_call_pseudo_dev_t *dev, bt_sink_srv_call_state_event_t event,void *data);
static void bt_sink_srv_call_psd_state_prepare_play( bt_sink_srv_call_pseudo_dev_t *dev, bt_sink_srv_call_state_event_t event,void *data);
static void bt_sink_srv_call_psd_state_playing( bt_sink_srv_call_pseudo_dev_t *dev, bt_sink_srv_call_state_event_t event,void *data);
static void bt_sink_srv_call_psd_state_prepare_stop( bt_sink_srv_call_pseudo_dev_t *dev, bt_sink_srv_call_state_event_t event,void *data);
static void bt_sink_srv_call_psd_run_next_state_in_prepare_stop(bt_sink_srv_call_pseudo_dev_t *dev);


const static bt_sink_srv_call_pseudo_event_notify_t playing_sub_state_fuc_list[] = {
    {BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE, NULL},
    {BT_SINK_SRV_CALL_PSD_SUB_STATE_MIC_RES_TAKING, bt_sink_srv_call_psd_sub_state_mic_resource_taking},
    {BT_SINK_SRV_CALL_PSD_SUB_STATE_PLAYING_IDLE, bt_sink_srv_call_psd_sub_state_playing_idle},
    {BT_SINK_SRV_CALL_PSD_SUB_STATE_SCO_ACTIVATING, bt_sink_srv_call_psd_sub_state_sco_activating},
    {BT_SINK_SRV_CALL_PSD_SUB_STATE_CODEC_STARTING, bt_sink_srv_call_psd_sub_state_codec_starting},
    {BT_SINK_SRV_CALL_PSD_SUB_STATE_PLAYING, bt_sink_srv_call_psd_sub_state_playing},
    {BT_SINK_SRV_CALL_PSD_SUB_STATE_CODEC_STOPPING, bt_sink_srv_call_psd_sub_state_codec_stopping},
    {BT_SINK_SRV_CALL_PSD_SUB_STATE_SCO_DEACTIVATING, bt_sink_srv_call_psd_sub_state_sco_deactivating}
};

const static bt_sink_srv_call_pseudo_event_notify_t psd_state_machine_fuc_list[] = {
    {AUDIO_SRC_SRV_STATE_OFF, NULL},
    {AUDIO_SRC_SRV_STATE_NONE, bt_sink_srv_call_psd_state_none},
    {AUDIO_SRC_SRV_STATE_READY, bt_sink_srv_call_psd_state_ready},
    {AUDIO_SRC_SRV_STATE_PREPARE_PLAY, bt_sink_srv_call_psd_state_prepare_play},
    {AUDIO_SRC_SRV_STATE_PLAYING, bt_sink_srv_call_psd_state_playing},
    {AUDIO_SRC_SRV_STATE_PREPARE_STOP, bt_sink_srv_call_psd_state_prepare_stop}
};

bool bt_sink_srv_call_psd_is_steady(bt_sink_srv_call_pseudo_dev_t *dev)
{
    bool result = true;
    bt_utils_assert(dev);
    if (dev->user_cb && dev->audio_src &&
        (dev->audio_src->state == AUDIO_SRC_SRV_STATE_PREPARE_PLAY ||
         dev->audio_src->state == AUDIO_SRC_SRV_STATE_PREPARE_STOP ||
         (dev->audio_src->substate != BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE &&
          dev->audio_src->substate != BT_SINK_SRV_CALL_PSD_SUB_STATE_PLAYING &&
          dev->audio_src->substate != BT_SINK_SRV_CALL_PSD_SUB_STATE_PLAYING_IDLE))) {
        result = false;
    }
    bt_sink_srv_report_id("[CALL][PSD][MGR]Is steady state:%d", 1, result);
    return result;
}

bool bt_sink_srv_call_psd_is_ready(void *device)
{
    bt_sink_srv_call_pseudo_dev_t *dev = (bt_sink_srv_call_pseudo_dev_t *)device;
    bool result = false;
    if (dev && dev->audio_src) {
        if (dev->audio_src->state != AUDIO_SRC_SRV_STATE_OFF &&
            dev->audio_src->state != AUDIO_SRC_SRV_STATE_NONE &&
            dev->audio_src->substate != BT_SINK_SRV_CALL_PSD_SUB_STATE_DISCONNECTING &&
            dev->audio_src->substate != BT_SINK_SRV_CALL_PSD_SUB_STATE_CONNECTING &&
            dev->next_state != BT_SINK_SRV_CALL_PSD_NEXT_STATE_NONE) {
            result = true;
        }
    }
    bt_sink_srv_report_id("[CALL][PSD][MGR]is connected:%d", 1, result);
    return result;
}

bool bt_sink_srv_call_psd_is_connecting(bt_sink_srv_call_pseudo_dev_t *dev)
{
    bool result = false;
    bt_sink_srv_call_pseudo_dev_t *device = (bt_sink_srv_call_pseudo_dev_t *)dev;

    if ((device != NULL) && (device->audio_src != NULL)) {
        if ((device->audio_src->state == AUDIO_SRC_SRV_STATE_NONE) &&
            (device->audio_src->substate == BT_SINK_SRV_CALL_PSD_SUB_STATE_CONNECTING)) {
            result = true;
        }
    }

    bt_sink_srv_report_id("[CALL][PSD][MGR]is connecting:%x", 1, result);
    return result;
}

bool bt_sink_srv_call_psd_is_playing(bt_sink_srv_call_pseudo_dev_t *dev)
{
    bool result = false;

    if ((dev != NULL) && (dev->audio_src != NULL)) {
        if ((dev->audio_src->state == AUDIO_SRC_SRV_STATE_PLAYING) &&
            ((dev->audio_src->substate == BT_SINK_SRV_CALL_PSD_SUB_STATE_SCO_ACTIVATING) ||
             (dev->audio_src->substate == BT_SINK_SRV_CALL_PSD_SUB_STATE_CODEC_STARTING) ||
             (dev->audio_src->substate == BT_SINK_SRV_CALL_PSD_SUB_STATE_PLAYING))) {
            result = true;
        }
    }

    bt_sink_srv_report_id("[CALL][PSD][MGR]is playing:%x", 1, result);
    return result;
}

bool bt_sink_srv_call_psd_get_playing_state(void)
{
    bool result = false;
    for (uint8_t i = 0; i < BT_SINK_SRV_CALL_PSD_NUM; i++) {
        if(bt_sink_srv_call_psd_is_playing(&bt_sink_srv_call_pseudo_dev[i]) == true) {
            result = true;
            break;
        }
    }
    bt_sink_srv_report_id("[CALL][PSD][MGR]get playing state, %d", 1, result);
    return result;
}

bool bt_sink_srv_call_psd_is_playing_codec(bt_sink_srv_call_pseudo_dev_t *dev)
{
    bool result = false;

    if ((dev != NULL) && (dev->audio_src != NULL)) {
        if ((dev->audio_src->state == AUDIO_SRC_SRV_STATE_PLAYING) &&
            (dev->audio_src->substate == BT_SINK_SRV_CALL_PSD_SUB_STATE_CODEC_STARTING)) {
            result = true;
        }
    }

    bt_sink_srv_report_id("[CALL][PSD][MGR]is paying codec:%x", 1, result);
    return result;
}

bool bt_sink_srv_call_psd_is_playing_idle(bt_sink_srv_call_pseudo_dev_t *dev)
{
    bool result = false;

    if ((dev != NULL) && (dev->audio_src != NULL)) {
        if ((dev->audio_src->state == AUDIO_SRC_SRV_STATE_PLAYING) &&
            (dev->audio_src->substate == BT_SINK_SRV_CALL_PSD_SUB_STATE_PLAYING_IDLE)) {
            result = true;
        }
    }

    bt_sink_srv_report_id("[CALL][PSD][MGR]is playing idle:%x", 1, result);
    return result;
}
static void bt_sink_srv_call_psd_update_state(audio_src_srv_handle_t *handle, audio_src_srv_event_t evt_id)
{
    audio_src_srv_update_state(handle, evt_id);
}

static void bt_sink_srv_call_psd_update_sub_state(audio_src_srv_handle_t *handle, bt_sink_srv_call_pseudo_dev_sub_state_t sub_state)
{
    audio_src_srv_set_substate(handle, sub_state);
}

static void bt_sink_srv_call_psd_set_next_state(bt_sink_srv_call_pseudo_dev_t *dev, bt_sink_srv_call_pseudo_dev_next_state_t next_state)
{
    bt_utils_assert(dev);
    bt_sink_srv_report_id("[CALL][PSD][MGR]Set next state, origin:0x%x ->new:0x%x", 2, dev->next_state, next_state);
    dev->next_state = next_state;
}

void bt_sink_srv_call_psd_alloc_audio_src(bt_sink_srv_call_pseudo_dev_t *dev)
{
    bt_utils_assert(dev);
    
    dev->audio_src = audio_src_srv_construct_handle(AUDIO_SRC_SRV_PSEUDO_DEVICE_HFP);
    bt_utils_assert(dev->audio_src);

#ifdef AIR_DCHS_MODE_ENABLE
    /*158x-dchs single mode ,no emp case,only single link,use dual-ant mic resource handle*/
     if (dchs_get_device_mode() == DCHS_MODE_SINGLE) {
        bt_sink_srv_report_id("[CALL][PSD]alloc_audio_src, 158x-dchs single mode", 0);
        dev->mic_audio_src = bt_dual_ant_ctx.mic_transmit.resource_handle;
      }
#endif

    dev->audio_src->priority = AUDIO_SRC_SRV_PRIORITY_HIGH;
    dev->audio_src->play = bt_sink_srv_call_psd_play;
    dev->audio_src->stop = bt_sink_srv_call_psd_stop;
    dev->audio_src->suspend = bt_sink_srv_call_psd_suspend;
    dev->audio_src->reject = bt_sink_srv_call_psd_reject;
    dev->audio_src->exception_handle = bt_sink_srv_call_psd_exception_handle;
}

void bt_sink_srv_call_psd_free_audio_src(bt_sink_srv_call_pseudo_dev_t *dev)
{
    bt_utils_assert(dev);
    bt_utils_assert(dev->audio_src);
    audio_src_srv_destruct_handle(dev->audio_src);
    dev->audio_src = NULL;
#ifdef AIR_DCHS_MODE_ENABLE
     /*158x-dchs single mode ,no need free mic_resource*/
     if (dchs_get_device_mode() == DCHS_MODE_SINGLE) {
        return;
      }
#endif    
    if (dev->mic_audio_src != NULL) {
        audio_src_srv_resource_manager_destruct_handle(dev->mic_audio_src);
        dev->mic_audio_src = NULL;
    }
}

uint64_t bt_sink_srv_call_psd_convert_btaddr_to_devid(bt_bd_addr_t *bd_addr)
{
    uint64_t dev_id = 0;
    uint32_t hdev = 0, ldev = 0;
    int32_t i = 0;
    uint8_t addr[16] = {0};
    (void)hdev;
    (void)ldev;

    bt_utils_assert(bd_addr);
    bt_sink_srv_memcpy(addr, bd_addr, sizeof(bt_bd_addr_t));
    for (i = 0; i < BT_BD_ADDR_LEN; ++i) {
        dev_id = ((dev_id << 8) | (addr[i]));
    }

    hdev = (dev_id >> 32 & 0xffffffff);
    ldev = (dev_id & 0xffffffff);
    bt_sink_srv_report_id("[CALL][PSD][MGR]convert_btaddr_to_devid--0x%x, ldev: 0x%x", 2, hdev, ldev);
    return dev_id;
}

void bt_sink_srv_call_psd_convert_devid_to_btaddr(uint64_t dev_id, bt_bd_addr_t *bd_addr)
{
    int32_t i = 0;
    bt_utils_assert(bd_addr);
    bt_utils_assert(dev_id);
    for (i = 0; i < BT_BD_ADDR_LEN; ++i) {
        (*bd_addr)[BT_BD_ADDR_LEN - 1 - i] = dev_id & 0xff;
        dev_id = (dev_id >> 8);
    }
    bt_sink_srv_report_id("[CALL][PSD][MGR]convert_devid_to_btaddr--address:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x", 6,
                          (*bd_addr)[5], (*bd_addr)[4], (*bd_addr)[3],
                          (*bd_addr)[2], (*bd_addr)[1], (*bd_addr)[0]);
}

void bt_sink_srv_call_psd_set_device_id(void *device, bt_bd_addr_t *bd_addr)
{
    bt_utils_assert(device);
    bt_utils_assert(bd_addr);
    bt_sink_srv_call_pseudo_dev_t *dev = (bt_sink_srv_call_pseudo_dev_t *)device;
    bt_utils_assert(dev->audio_src);
    dev->audio_src->dev_id = bt_sink_srv_call_psd_convert_btaddr_to_devid(bd_addr);
}

uint64_t bt_sink_srv_call_psd_get_device_id(void *device)
{
    bt_utils_assert(device);
    bt_sink_srv_call_pseudo_dev_t *dev = (bt_sink_srv_call_pseudo_dev_t *)device;
    bt_utils_assert(dev->audio_src);
    return dev->audio_src->dev_id;
}

void bt_sink_srv_call_psd_enable_sidetone(void *device)
{
    bt_sink_srv_call_pseudo_dev_t *dev = (bt_sink_srv_call_pseudo_dev_t *)device;

    bt_utils_assert(dev);
    bt_utils_assert(dev->audio_src);
    bt_sink_srv_report_id("[CALL][PSD][MGR]enable sidetone, device: 0x%x flag: 0x%x", 2, dev, dev->flag);

    if ((!bt_sink_srv_call_get_sidetone_enable_config()) ||
        (dev->flag & BT_SINK_SRV_CALL_PSD_FLAG_ENABLE_SIDETONE)) {
        return;
    }

    if (dev->audio_src->state == AUDIO_SRC_SRV_STATE_PLAYING &&
        (dev->audio_src->substate == BT_SINK_SRV_CALL_PSD_SUB_STATE_CODEC_STARTING || dev->audio_src->substate == BT_SINK_SRV_CALL_PSD_SUB_STATE_PLAYING)) {
        dev->flag |= BT_SINK_SRV_CALL_PSD_FLAG_ENABLE_SIDETONE;
        bt_sink_srv_call_audio_side_tone_enable();
    }
}

void bt_sink_srv_call_psd_disable_sidetone(void *device)
{
    bt_sink_srv_call_pseudo_dev_t *dev = (bt_sink_srv_call_pseudo_dev_t *)device;

    bt_utils_assert(dev);
    bt_sink_srv_report_id("[CALL][PSD][MGR]disable sidetone, device: 0x%x flag: 0x%x", 2, dev, dev->flag);

    if (dev->flag & BT_SINK_SRV_CALL_PSD_FLAG_ENABLE_SIDETONE) {
        dev->flag &= ~BT_SINK_SRV_CALL_PSD_FLAG_ENABLE_SIDETONE;
        bt_sink_srv_call_audio_side_tone_disable();
    }
}

static bt_sink_srv_call_pseudo_dev_t *bt_sink_srv_call_psd_get_ready_dev(void)
{
    bt_sink_srv_call_pseudo_dev_t *dev = NULL;
    uint8_t i = 0;
    for (i = 0; i < BT_SINK_SRV_CALL_PSD_NUM; ++i) {
        if (bt_sink_srv_call_pseudo_dev[i].audio_src &&
            bt_sink_srv_call_pseudo_dev[i].audio_src->state == AUDIO_SRC_SRV_STATE_READY) {
            dev = &bt_sink_srv_call_pseudo_dev[i];
            break;
        }
    }
    bt_sink_srv_report_id("[CALL][PSD][MGR]get ready dev, dev[%d] = 0x%0x", 2, i, dev);
    return dev;
}

 bt_sink_srv_call_pseudo_dev_t *bt_sink_srv_call_psd_get_dev_by_audio_src(audio_src_srv_handle_t *audio_src)
{
    bt_sink_srv_call_pseudo_dev_t *dev = NULL;
    uint8_t i = 0;
    for (i = 0; i < BT_SINK_SRV_CALL_PSD_NUM; ++i) {
        if (bt_sink_srv_call_pseudo_dev[i].audio_src == audio_src) {
            dev = &bt_sink_srv_call_pseudo_dev[i];
            break;
        }
    }
    bt_sink_srv_report_id("[CALL][PSD][MGR]get dev, dev[%d] = 0x%0x", 2, i, dev);
    return dev;
}

 bt_sink_srv_call_pseudo_dev_t *bt_source_srv_call_psd_find_device_by_handle( void *handle)
{
    uint32_t i = 0;
    for (i = 0; i < BT_SINK_SRV_CALL_PSD_NUM; i++) {
        bt_sink_srv_call_pseudo_dev_t *device = &bt_sink_srv_call_pseudo_dev[i];
        if (device->mic_audio_src == (audio_src_srv_resource_manager_handle_t *)handle) {
            return device;
        }
    }
    return NULL;
}

void bt_sink_srv_call_psd_audio_resource_callback(audio_src_srv_resource_manager_handle_t *handle,
        audio_src_srv_resource_manager_event_t event)
{
    bt_utils_assert(handle);
    bt_sink_srv_report_id("[CALL][PSD][MGR]audio_resource_callback,handle:0x%x,event:0x%x", 2, handle, event);
    bt_sink_srv_call_pseudo_dev_t *device = bt_source_srv_call_psd_find_device_by_handle(handle);
    if(!device) {
        return;
    }
    
    bt_sink_srv_sco_connection_state_t sco_state = BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED;
    bool sco_activated = false;
    switch (event) {
        case AUDIO_SRC_SRV_EVENT_TAKE_SUCCESS: {    

            device->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_SCO_STATE, (void *)device, (void *)&sco_state);
            device->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_IS_SCO_ACTIVATED, (void *)device, (void *)&sco_activated);
            bt_sink_srv_report_id("[CALL][PSD][MGR]audio_resource_callback, sco_state:0x%x sco_activated:0x%x", 2, sco_state, sco_activated);
            if (sco_state == BT_SINK_SRV_SCO_CONNECTION_STATE_CONNECTED) {
                device->next_state = BT_SINK_SRV_CALL_PSD_NEXT_STATE_ACTIVATE_SCO;
            } 
            if (sco_activated) {
                device->next_state = BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_SCO;
            }
            bt_sink_srv_call_psd_state_event_notify(device, BT_SINK_SRV_CALL_EVENT_MIC_RES_TAKE_SUCCES, NULL);     
            break;
        }
        case AUDIO_SRC_SRV_EVENT_TAKE_REJECT: {
            bt_sink_srv_call_psd_state_event_notify(device, BT_SINK_SRV_CALL_EVENT_MIC_RES_TAKE_REJECT, NULL);     
            break;
        }

        case AUDIO_SRC_SRV_EVENT_GIVE_SUCCESS: {
            if (device->flag & BT_SINK_SRV_CALL_PSD_FLAG_TAKED_MIC_RESOURCE) {
                device->flag &= ~BT_SINK_SRV_CALL_PSD_FLAG_TAKED_MIC_RESOURCE;
            }
            break;
        }

        case AUDIO_SRC_SRV_EVENT_SUSPEND: {
            bt_sink_srv_call_psd_suspend(device->audio_src, NULL);
            break;
        }
        default: {
            break;
        }
    }
}


static void bt_sink_srv_call_psd_play(audio_src_srv_handle_t *handle)
{
    bt_sink_srv_mutex_lock();
    bt_sink_srv_call_pseudo_dev_t *dev = bt_sink_srv_call_psd_get_dev_by_audio_src(handle);
    bt_utils_assert(dev);
    bt_utils_assert(dev->audio_src);
    
#ifndef BT_SINK_DUAL_ANT_ENABLE
        dev->mic_audio_src = audio_src_srv_resource_manager_construct_handle(AUDIO_SRC_SRV_RESOURCE_TYPE_MIC, AUDIO_SRC_SRV_RESOURCE_TYPE_MIC_USER_HFP);
        bt_utils_assert(dev->mic_audio_src);
        dev->mic_audio_src->priority =AUDIO_SRC_SRV_RESOURCE_TYPE_MIC_USER_HFP_PRIORIRT;
        dev->mic_audio_src->callback_func = bt_sink_srv_call_psd_audio_resource_callback;
#endif

    bt_sink_srv_report_id("[CALL][PSD][MGR]dev play, audio_type:0x%x, flag:0x%x", 2, dev->audio_type, dev->flag);
    if (dev->flag & BT_SINK_SRV_CALL_PSD_FLAG_ADD_WAIT_LIST) {
        bt_sink_srv_call_pseudo_dev_t *ready_dev = bt_sink_srv_call_psd_get_ready_dev();
        bt_sink_srv_sco_connection_state_t sco_state = BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED;
        bool sco_activated = false;
        dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_SCO_STATE, (void *)dev, (void *)&sco_state);
        dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_IS_SCO_ACTIVATED, (void *)dev, (void *)&sco_activated);
        if (sco_state == BT_SINK_SRV_SCO_CONNECTION_STATE_CONNECTED) {
            dev->audio_type = BT_SINK_SRV_CALL_AUDIO_SCO;
            if (!sco_activated) {
                dev->next_state = BT_SINK_SRV_CALL_PSD_NEXT_STATE_ACTIVATE_SCO;
            }
        } else {
            dev->audio_type = BT_SINK_SRV_CALL_AUDIO_NULL;
        }
        if (sco_activated) {
            dev->next_state = BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_SCO;
        }
        bt_sink_srv_call_psd_del_waiting_list(dev);
        if (ready_dev) {
            bt_sink_srv_sco_connection_state_t sco_state = BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED;
            bt_sink_srv_hf_call_state_t call_state = BT_SINK_SRV_HF_CALL_STATE_IDLE;
            ready_dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_SCO_STATE, (void *)ready_dev, (void *)&sco_state);
            ready_dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_CALL_STATE, (void *)ready_dev, (void *)&call_state);
            if (sco_state != BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED || call_state != BT_SINK_SRV_HF_CALL_STATE_IDLE) {
                bt_sink_srv_call_psd_add_waiting_list(ready_dev);
            }
        }
    }

    if (dev->flag & BT_SINK_SRV_CALL_PSD_FLAG_SUSPEND) {
        dev->flag &= ~BT_SINK_SRV_CALL_PSD_FLAG_SUSPEND;
    }

    dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_RESUME, (void *)dev, NULL);

#if defined (AIR_LE_AUDIO_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    if (dev->flag & BT_SINK_SRV_CALL_PSD_FLAG_IF_PENDING) {
        dev->flag &= ~BT_SINK_SRV_CALL_PSD_FLAG_IF_PENDING;
        dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_RESEND_CALL_INFO, (void *)dev, NULL);
    } 
#endif

    if (BT_SINK_SRV_CALL_AUDIO_SCO  == dev->audio_type ||
        BT_SINK_SRV_CALL_AUDIO_RING == dev->audio_type ||
        BT_SINK_SRV_CALL_AUDIO_NULL == dev->audio_type) {
        dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_SWITCH_AWS_LINK, dev, NULL);
        bt_sink_srv_call_psd_update_state(handle, AUDIO_SRC_SRV_EVT_PLAYING);
        bt_sink_srv_call_psd_update_sub_state(handle, BT_SINK_SRV_CALL_PSD_SUB_STATE_MIC_RES_TAKING);

#ifdef AIR_DCHS_MODE_ENABLE
    if (dchs_get_device_mode() == DCHS_MODE_SINGLE) { /* dchs single mode */
        bt_sink_srv_mutex_unlock();
        audio_src_srv_resource_manager_take(dev->mic_audio_src);
        return;
    }
#endif

#ifdef BT_SINK_DUAL_ANT_ROLE_SLAVE/* dual-chip */
    bt_sink_srv_call_psd_take_mic_resource();
#endif 


#ifndef BT_SINK_DUAL_ANT_ENABLE/* single chip */
    audio_src_srv_resource_manager_take(dev->mic_audio_src);
#endif

    } else {
        bt_sink_srv_assert(0);
    }

    bt_sink_srv_mutex_unlock();

}

static void bt_sink_srv_call_psd_reject(audio_src_srv_handle_t *handle)
{
    bt_sink_srv_call_pseudo_dev_t *dev = bt_sink_srv_call_psd_get_dev_by_audio_src(handle);
    bt_utils_assert(dev);
    bt_sink_srv_report_id("[CALL][PSD][MGR]dev reject: 0x%x", 1, dev->audio_type);
    bt_sink_srv_call_psd_add_waiting_list(dev);
    dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_REJECT, (void *)dev, NULL);
}

void bt_sink_srv_call_psd_suspend(audio_src_srv_handle_t *handle, audio_src_srv_handle_t *int_hd)
{
    bt_sink_srv_call_pseudo_dev_t *dev = bt_sink_srv_call_psd_get_dev_by_audio_src(handle);
    bt_utils_assert(dev);
    bt_sink_srv_report_id("[CALL][PSD][MGR]dev suspend: 0x%x", 1, dev->audio_type);
    if (!(dev->flag & BT_SINK_SRV_CALL_PSD_FLAG_SUSPEND)) {
        dev->flag |= BT_SINK_SRV_CALL_PSD_FLAG_SUSPEND;
        dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_SUSPEND, (void *)dev, (void *)int_hd);
    }
}

void bt_sink_srv_call_psd_release_mic_resource(bt_sink_srv_call_pseudo_dev_t *dev)
{
    bt_utils_assert(dev);
    bt_sink_srv_report_id("[CALL][PSD][MGR]release mic resource", 0);

#ifdef AIR_DCHS_MODE_ENABLE
    if (dchs_get_device_mode() == DCHS_MODE_SINGLE) { /* dchs single mode */
        audio_src_srv_resource_manager_give(dev->mic_audio_src);
    }   
#endif 
    
#ifdef BT_SINK_DUAL_ANT_ROLE_SLAVE /* dual-chip */

    bt_sink_srv_call_psd_give_mic_resource();
    if (dev->flag & BT_SINK_SRV_CALL_PSD_FLAG_TAKED_MIC_RESOURCE) {
       dev->flag &= ~BT_SINK_SRV_CALL_PSD_FLAG_TAKED_MIC_RESOURCE;
    }
    return;
#endif
    
#ifndef BT_SINK_DUAL_ANT_ENABLE/* single chip */
    audio_src_srv_resource_manager_give(dev->mic_audio_src);
    if (dev->mic_audio_src != NULL) {
        audio_src_srv_resource_manager_destruct_handle(dev->mic_audio_src);
        dev->mic_audio_src = NULL;
    }
#endif
}

static void bt_sink_srv_call_psd_stop(audio_src_srv_handle_t *handle)
{
    bt_sink_srv_call_pseudo_dev_t *dev = bt_sink_srv_call_psd_get_dev_by_audio_src(handle);
    bt_utils_assert(dev);
    bt_sink_srv_report_id("[CALL][PSD][MGR]dev stop, dev:0x%x, type:0x%x", 2,dev, dev->audio_type);
    dev->audio_type = BT_SINK_SRV_CALL_AUDIO_NONE;

    if (dev->flag & BT_SINK_SRV_CALL_PSD_FLAG_TAKED_MIC_RESOURCE) {
        bt_sink_srv_call_psd_release_mic_resource(dev);
    }
    
    bt_sink_srv_call_psd_update_state(handle, AUDIO_SRC_SRV_EVT_READY);

    /* If the device was interrupted, do not delete it from waiting list. */
    bt_sink_srv_hf_call_state_t call_state = BT_SINK_SRV_HF_CALL_STATE_IDLE;
    bt_sink_srv_sco_connection_state_t sco_state = BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED;
    if (dev->user_cb != NULL) {
        dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_CALL_STATE, dev, &call_state);
        dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_SCO_STATE, dev, &sco_state);
    } else {
        bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_NONE);
    }

    if ((call_state == BT_SINK_SRV_HF_CALL_STATE_IDLE)
        && (sco_state == BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED)) {
        bt_sink_srv_call_psd_del_waiting_list(dev);
    }
    if ((call_state != BT_SINK_SRV_HF_CALL_STATE_IDLE)
          || (sco_state != BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED)) {
          bt_sink_srv_call_psd_add_waiting_list(dev);
    }
    bt_sink_srv_call_psd_run_next_state_in_state_ready(dev);
}

static void bt_sink_srv_call_psd_exception_handle(audio_src_srv_handle_t *handle, int32_t event, void *param)
{
}

void bt_sink_srv_call_psd_add_waiting_list(bt_sink_srv_call_pseudo_dev_t *device)
{
    bt_utils_assert(device);
    bt_utils_assert(device->audio_src);
    bt_sink_srv_report_id("[CALL][PSD][MGR]add waiting list,dev:0x%x, flag:0x%x", 2, device, device->flag);
    if (!(device->flag & BT_SINK_SRV_CALL_PSD_FLAG_ADD_WAIT_LIST)) {
        device->flag |= BT_SINK_SRV_CALL_PSD_FLAG_ADD_WAIT_LIST;
        audio_src_srv_add_waiting_list(device->audio_src);
    }
}

void bt_sink_srv_call_psd_del_waiting_list(bt_sink_srv_call_pseudo_dev_t *device)
{
    bt_utils_assert(device);
    bt_utils_assert(device->audio_src);
    bt_sink_srv_report_id("[CALL][PSD][MGR]del waiting list, dev:0x%x, flag:0x%x", 2, device, device->flag);
    if (device->flag & BT_SINK_SRV_CALL_PSD_FLAG_ADD_WAIT_LIST) {
        audio_src_srv_del_waiting_list(device->audio_src);
        device->flag &= ~BT_SINK_SRV_CALL_PSD_FLAG_ADD_WAIT_LIST;
    }
}

static void bt_sink_srv_call_psd_stop_int(bt_sink_srv_call_pseudo_dev_t *dev)
{
    bt_utils_assert(dev);
    bt_utils_assert(dev->audio_src);
    bt_sink_srv_report_id("[CALL][PSD][MGR]dev stop int, type:0x%x", 1, dev->audio_type);
    bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_CODEC_STOPPING);
    if (dev->audio_type == BT_SINK_SRV_CALL_AUDIO_SCO) {
        bt_sink_srv_call_psd_disable_sidetone((void *)dev);
    }
    if (!bt_sink_srv_call_audio_stop(dev->audio_id)) {
        bt_utils_assert(0);
    }
}

static void bt_sink_srv_call_psd_play_int(bt_sink_srv_call_pseudo_dev_t *dev)
{
    bt_bd_addr_t address = {0};
    bt_utils_assert(dev);
    bt_utils_assert(dev->audio_src);

    bt_sink_srv_report_id("[CALL][PSD][MGR]dev play int, audio_type:0x%x", 1, dev->audio_type);

    bt_sink_srv_call_psd_convert_devid_to_btaddr(dev->audio_src->dev_id, &address);

    if (BT_SINK_SRV_CALL_AUDIO_SCO == dev->audio_type ) {
        bt_sink_srv_am_audio_capability_t audio_capability;
        bt_sink_srv_memset(&audio_capability, 0, sizeof(bt_sink_srv_am_audio_capability_t));
        bt_sink_srv_memcpy((void *)&audio_capability.dev_addr, (void *)&address, sizeof(bt_bd_addr_t));
        if (BT_SINK_SRV_CALL_AUDIO_SCO == dev->audio_type) {
            bt_sink_srv_call_audio_sco_parameter_init(&audio_capability, dev->codec, dev->spk_vol);
        } else {
            bt_sink_srv_call_audio_pcm_parameter_init(&audio_capability, dev->spk_vol);
        }

        bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_CODEC_STARTING);
        if (!bt_sink_srv_call_audio_play(dev->audio_id, &audio_capability)) {
            bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE);
            bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
        }
    } else {
        bt_utils_assert(0);
    }
}

static void bt_sink_srv_call_psd_run_next_state_in_sub_state_playing(bt_sink_srv_call_pseudo_dev_t *dev)
{
    bt_utils_assert(dev);
    bt_utils_assert(dev->audio_src);
    switch (dev->next_state) {
        case BT_SINK_SRV_CALL_PSD_NEXT_STATE_NONE:
        case BT_SINK_SRV_CALL_PSD_NEXT_STATE_CONNECTING:
        case BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY:
        case BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_NULL:
        case BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_SCO:
        case BT_SINK_SRV_CALL_PSD_NEXT_STATE_ACTIVATE_SCO:
        case BT_SINK_SRV_CALL_PSD_NEXT_STATE_POWER_OFF: {
            bt_sink_srv_call_psd_stop_int(dev);
        }
        break;

        default: {
            bt_sink_srv_report_id("[CALL][PSD][MGR]Not handle", 0);
        }
        break;
    }
}

static void bt_sink_srv_call_psd_run_next_state_in_state_ready(bt_sink_srv_call_pseudo_dev_t *dev)
{
    bt_utils_assert(dev);
    bt_utils_assert(dev->audio_src);

    switch (dev->next_state) {
        case BT_SINK_SRV_CALL_PSD_NEXT_STATE_NONE: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_INIT);
            bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_UNAVAILABLE);
        }
        break;

        case BT_SINK_SRV_CALL_PSD_NEXT_STATE_CONNECTING: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_INIT);
            bt_sink_srv_call_psd_set_sub_state(dev, BT_SINK_SRV_CALL_PSD_SUB_STATE_CONNECTING);
            bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_UNAVAILABLE);
        }
        break;

        case BT_SINK_SRV_CALL_PSD_NEXT_STATE_ACTIVATE_SCO: {
            bool is_highlight = false;
            dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_IS_HIGHLIGHT, (void *)dev, (void *)&is_highlight);

            if (is_highlight) {
                dev->audio_type = BT_SINK_SRV_CALL_AUDIO_SCO;
                bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_PREPARE_PLAY);
            } else {
                bt_sink_srv_call_psd_add_waiting_list(dev);
                dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_REJECT, (void *)dev, NULL);
            }
        }
        break;

        case BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_SCO: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_INIT);
            dev->audio_type = BT_SINK_SRV_CALL_AUDIO_SCO;
            bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_PREPARE_PLAY);
        }
        break;

        case BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_NULL: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_INIT);
            dev->audio_type = BT_SINK_SRV_CALL_AUDIO_NULL;
            bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_PREPARE_PLAY);
        }
        break;

        case BT_SINK_SRV_CALL_PSD_NEXT_STATE_POWER_OFF: {
            bt_sink_srv_call_psd_set_sub_state(dev, BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE);
            bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_UNAVAILABLE);
        }
        break;

        default: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_INIT);
        }
        break;
    }
}
#ifdef BT_SINK_DUAL_ANT_ENABLE 
bt_sink_srv_call_pseudo_dev_t* bt_sink_srv_call_psd_get_dev_is_taking_mic_resource()
{
    bt_sink_srv_call_pseudo_dev_t * dev = NULL;
    uint8_t i = 0;
    for (i = 0; i < BT_SINK_SRV_CALL_PSD_NUM; ++i) {
        if (bt_sink_srv_call_pseudo_dev[i].audio_src->substate == BT_SINK_SRV_CALL_PSD_SUB_STATE_MIC_RES_TAKING) {
            dev = &bt_sink_srv_call_pseudo_dev[i];
            break;
        }
    }
    bt_sink_srv_report_id("[CALL][PSD][MGR]get taking_mic_resource dev, dev[%d] = 0x%0x", 2, i, dev);
    return dev;
}

void bt_sink_srv_call_psd_take_mic_resource()
{
    bt_sink_srv_dual_ant_data_t notify;
    notify.type = BT_SINK_DUAL_ANT_TYPE_CALL;
    notify.call_info.esco_state = true;
    bt_sink_srv_dual_ant_notify(false, &notify);
    bt_sink_srv_report_id("[CALL][PSD][MGR]Take mic resource",0);
}

void bt_sink_srv_call_psd_give_mic_resource()
{
    bt_sink_srv_dual_ant_data_t notify;
    notify.type = BT_SINK_DUAL_ANT_TYPE_CALL;
    notify.call_info.esco_state = false;
    bt_sink_srv_dual_ant_notify(false, &notify);
    bt_sink_srv_report_id("[CALL][PSD][MGR]Give mic resource",0);
}
#endif

static void bt_sink_srv_call_psd_run_next_state_in_sub_state_playing_idle(bt_sink_srv_call_pseudo_dev_t *dev)
{
    bt_utils_assert(dev);
    bt_utils_assert(dev->audio_src);
    switch (dev->next_state) {
        case BT_SINK_SRV_CALL_PSD_NEXT_STATE_NONE: {
            bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE);
            bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
        }
        break;

        case BT_SINK_SRV_CALL_PSD_NEXT_STATE_CONNECTING: {
            bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_CONNECTING);
            bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
        }
        break;

        case BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_INIT);
            bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE);
            bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
        }
        break;

        case BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_NULL: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_INIT);
        }
        break;

        case BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_SCO: {
            dev->audio_type = BT_SINK_SRV_CALL_AUDIO_SCO;
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_INIT);
            bt_sink_srv_call_psd_play_int(dev);
        }
        break;

        case BT_SINK_SRV_CALL_PSD_NEXT_STATE_ACTIVATE_SCO: {
            dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_ACTIVATE_SCO, (void *)dev, NULL);
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_INIT);

            dev->audio_type = BT_SINK_SRV_CALL_AUDIO_SCO;
            bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_SCO_ACTIVATING);
        }
        break;

        case BT_SINK_SRV_CALL_PSD_NEXT_STATE_POWER_OFF: {
            bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE);
            bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
        }
        break;

        default: {
             bt_sink_srv_report_id("[CALL][PSD][MGR]Not handle", 0);
        }
        break;
    }
}

static void bt_sink_srv_call_psd_sub_state_connecting(
    bt_sink_srv_call_pseudo_dev_t *dev,
    bt_sink_srv_call_state_event_t event,
    void *data)
{
    bt_utils_assert(dev);
    bt_utils_assert(dev->audio_src);

    switch (event) {
        case BT_SINK_SRV_CALL_EVENT_LINK_CONNECTED: {
            bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE);
            bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_READY);
            bt_sink_srv_call_psd_run_next_state_in_state_ready(dev);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_ACTIVATE_REQ: {
            dev->audio_type = BT_SINK_SRV_CALL_AUDIO_SCO;
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_ACTIVATE_SCO);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_ACTIVATED: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_SCO);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_CALL_START_IND: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_NULL);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_CALL_END_IND: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_DISCONNECTED: {
            if (dev->next_state == BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_SCO ||
                dev->next_state == BT_SINK_SRV_CALL_PSD_NEXT_STATE_ACTIVATE_SCO) {
                bt_sink_srv_hf_call_state_t state = BT_SINK_SRV_HF_CALL_STATE_IDLE;
                dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_CALL_STATE, (void *)dev, (void *)&state);
                if (state != BT_SINK_SRV_HF_CALL_STATE_IDLE) {
                    bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_NULL);
                }
            }
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_LINK_DISCONNECTED: {
            bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE);
            bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_UNAVAILABLE);
        }
        break;

        default: {
            bt_sink_srv_report_id("[CALL][PSD][MGR]Unexception event!!!", 0);
        }
        break;
    }
}

static void bt_sink_srv_call_psd_state_none_sub_none(
    bt_sink_srv_call_pseudo_dev_t *dev,
    bt_sink_srv_call_state_event_t event,
    void *data)
{
    bt_utils_assert(dev);
    bt_utils_assert(dev->audio_src);

    switch (event) {
        case BT_SINK_SRV_CALL_EVENT_CONNECT_LINK_REQ: {
            bt_sink_srv_call_psd_set_sub_state(dev, BT_SINK_SRV_CALL_PSD_SUB_STATE_CONNECTING);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_CONNECT_LINK_REQ_IND: {
            bt_sink_srv_call_psd_set_sub_state(dev, BT_SINK_SRV_CALL_PSD_SUB_STATE_CONNECTING);
        }
        break;

        default: {
            bt_sink_srv_report_id("[CALL][PSD][MGR]Unexception event!!!", 0);
        }
        break;
    }
}

static void bt_sink_srv_call_psd_state_none(
    bt_sink_srv_call_pseudo_dev_t *dev,
    bt_sink_srv_call_state_event_t event,
    void *data)
{
    bt_utils_assert(dev);
    bt_utils_assert(dev->audio_src);

    switch (dev->audio_src->substate) {
        case BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE: {
            bt_sink_srv_call_psd_state_none_sub_none(dev, event, data);
        }
        break;

        case BT_SINK_SRV_CALL_PSD_SUB_STATE_CONNECTING: {
            bt_sink_srv_call_psd_sub_state_connecting(dev, event, data);
        }
        break;

        default: {
            bt_utils_assert(0);
        }
        break;
    }
}

static void bt_sink_srv_call_psd_sub_state_disconnecting(
    bt_sink_srv_call_pseudo_dev_t *dev,
    bt_sink_srv_call_state_event_t event,
    void *data)
{
    bt_utils_assert(dev);
    bt_utils_assert(dev->audio_src);

    switch (event) {
        case BT_SINK_SRV_CALL_EVENT_LINK_DISCONNECTED: {
            bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE);
            bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_UNAVAILABLE);
        }
        break;

        default: {
            bt_sink_srv_report_id("[CALL][PSD][MGR]Unexception event!!!", 0);
        }
        break;
    }

}

static void bt_sink_srv_call_psd_state_ready(
    bt_sink_srv_call_pseudo_dev_t *dev,
    bt_sink_srv_call_state_event_t event,
    void *data)
{
    bt_utils_assert(dev);
    bt_utils_assert(dev->audio_src);

    switch (dev->audio_src->substate) {
        case BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE: {
            switch (event) {
                case BT_SINK_SRV_CALL_EVENT_DISCONNECT_LINK_REQ: {
                    bt_sink_srv_call_psd_set_sub_state(dev, BT_SINK_SRV_CALL_PSD_SUB_STATE_DISCONNECTING);
                }
                break;

                case BT_SINK_SRV_CALL_EVENT_LINK_DISCONNECTED: {
                    /* Force del waiting list because AM may add waiting list in some cases. */
                    // bt_sink_srv_call_psd_del_waiting_list(dev);
                    audio_src_srv_del_waiting_list(dev->audio_src);
                    bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_UNAVAILABLE);
                }
                break;

                case BT_SINK_SRV_CALL_EVENT_SCO_DISCONNECTED: {
                    bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_INIT);
                    bt_sink_srv_hf_call_state_t state = BT_SINK_SRV_HF_CALL_STATE_IDLE;
                    dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_CALL_STATE, (void *)dev, (void *)&state);
                    dev->audio_type = BT_SINK_SRV_CALL_AUDIO_NULL;
                    if (state == BT_SINK_SRV_HF_CALL_STATE_IDLE) {
                        bt_sink_srv_call_psd_del_waiting_list(dev);
                    }
                }
                break;

                case BT_SINK_SRV_CALL_EVENT_SCO_ACTIVATE_REQ: {
                    dev->audio_type = BT_SINK_SRV_CALL_AUDIO_SCO;
                    bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_ACTIVATE_SCO);
                    bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_PREPARE_PLAY);
                }
                break;

                case BT_SINK_SRV_CALL_EVENT_SCO_ACTIVATED: {
                    dev->audio_type = BT_SINK_SRV_CALL_AUDIO_SCO;
                    bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_PREPARE_PLAY);
                }
                break;

                case BT_SINK_SRV_CALL_EVENT_CALL_START_IND: {
                    dev->audio_type = BT_SINK_SRV_CALL_AUDIO_NULL;
                    bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_PREPARE_PLAY);
                }
                break;

                case BT_SINK_SRV_CALL_EVENT_CALL_END_IND: {
                    bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_INIT);
                    bt_sink_srv_call_psd_del_waiting_list(dev);
                }
                break;

                default: {
					bt_sink_srv_report_id("[CALL][PSD][MGR]Unexception event!!!", 0);
                }
                break;
            }
        }
        break;

        case BT_SINK_SRV_CALL_PSD_SUB_STATE_DISCONNECTING: {
            bt_sink_srv_call_psd_sub_state_disconnecting(dev, event, data);
        }
        break;

        default: {
            bt_utils_assert(0);
        }
        break;
    }
}

static void bt_sink_srv_call_psd_state_prepare_play(
    bt_sink_srv_call_pseudo_dev_t *dev,
    bt_sink_srv_call_state_event_t event,
    void *data)
{
    bt_utils_assert(dev);
    bt_utils_assert(dev->audio_src);

    switch (event) {
        case BT_SINK_SRV_CALL_EVENT_LINK_DISCONNECTED: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_NONE);
            bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE);
            bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_DISCONNECTED: {
            if (dev->audio_type == BT_SINK_SRV_CALL_AUDIO_SCO) {
                bt_sink_srv_hf_call_state_t state = BT_SINK_SRV_HF_CALL_STATE_IDLE;
                dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_CALL_STATE, (void *)dev, (void *)&state);
                if (state != BT_SINK_SRV_HF_CALL_STATE_IDLE) {
                    bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_NULL);
                    bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE);
                } else {
                    bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
                    bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE);
                }
                dev->audio_type = BT_SINK_SRV_CALL_AUDIO_NULL;
            }
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_ACTIVATED: {
            dev->audio_type = BT_SINK_SRV_CALL_AUDIO_SCO;
            if (dev->next_state != BT_SINK_SRV_CALL_PSD_NEXT_STATE_INIT) {
                bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_SCO);
            }
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_CALL_END_IND: {
            bt_sink_srv_sco_connection_state_t sco_state = BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED;
            dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_SCO_STATE, (void *)dev, (void *)&sco_state);
            bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE);
            if (sco_state != BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED) {
                bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_SCO);
            } else {
                bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
            }
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_ACTIVATE_REQ: {
            dev->audio_type = BT_SINK_SRV_CALL_AUDIO_SCO;
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_ACTIVATE_SCO);
        }
        break;

        default: {
            bt_sink_srv_report_id("[CALL][PSD][MGR]Unexception event!!!", 0);
        }
        break;
    }
}

static void bt_sink_srv_call_psd_sub_state_mic_resource_taking(
        bt_sink_srv_call_pseudo_dev_t* dev,
        bt_sink_srv_call_state_event_t event,
        void* data)
{
    bt_sink_srv_report_id("[CALL][PSD][MGR]event:0x%x", 1, event);
    switch (event) {
        case BT_SINK_SRV_CALL_EVENT_MIC_RES_TAKE_SUCCES: {
            if (dev->flag & BT_SINK_SRV_CALL_PSD_FLAG_ADD_MIC_RES_WAIT_LIST) {
                dev->flag &= ~BT_SINK_SRV_CALL_PSD_FLAG_ADD_MIC_RES_WAIT_LIST;
            }
            
            if (!(dev->flag & BT_SINK_SRV_CALL_PSD_FLAG_TAKED_MIC_RESOURCE)) {
                  dev->flag |= BT_SINK_SRV_CALL_PSD_FLAG_TAKED_MIC_RESOURCE;
             }
             bt_sink_srv_call_psd_set_sub_state(dev, BT_SINK_SRV_CALL_PSD_SUB_STATE_PLAYING_IDLE);
             bt_sink_srv_call_psd_run_next_state_in_sub_state_playing_idle(dev);
        }
        break;
        
        case BT_SINK_SRV_CALL_EVENT_MIC_RES_TAKE_REJECT: {
            if (!(dev->flag & BT_SINK_SRV_CALL_PSD_FLAG_ADD_MIC_RES_WAIT_LIST)) {
                dev->flag |= BT_SINK_SRV_CALL_PSD_FLAG_ADD_MIC_RES_WAIT_LIST;
                audio_src_srv_resource_manager_add_waiting_list(dev->mic_audio_src);
            }
        }
        break;
        
        case BT_SINK_SRV_CALL_EVENT_SCO_ACTIVATED: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_SCO);
        }
        break;
        
        case BT_SINK_SRV_CALL_EVENT_LINK_DISCONNECTED: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_NONE);
           
        }
        break;
        
        case BT_SINK_SRV_CALL_EVENT_SCO_ACTIVATE_REQ: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_ACTIVATE_SCO);
        }
        break;
        
        case BT_SINK_SRV_CALL_EVENT_CALL_END_IND: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
        }
        break;
        
        case BT_SINK_SRV_CALL_EVENT_SCO_DISCONNECTED: {
            if (dev->audio_type == BT_SINK_SRV_CALL_AUDIO_SCO) {
                bt_sink_srv_hf_call_state_t state = BT_SINK_SRV_HF_CALL_STATE_IDLE;
                dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_CALL_STATE, (void *)dev, (void *)&state);
                if (state != BT_SINK_SRV_HF_CALL_STATE_IDLE) {
                    bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_NULL);
                } else {
                    bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
                }
            }
        }
        break;
        
        default:
        break;
        }
}
        
static void bt_sink_srv_call_psd_sub_state_playing_idle(
    bt_sink_srv_call_pseudo_dev_t *dev,
    bt_sink_srv_call_state_event_t event,
    void *data)
{
    switch (event) {
        case BT_SINK_SRV_CALL_EVENT_LINK_DISCONNECTED: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_NONE);
            bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE);
            bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_ACTIVATED: {
#ifdef AIR_FEATURE_SINK_AUDIO_SWITCH_SUPPORT
            bt_sink_srv_call_context_t *call_context = bt_sink_srv_call_get_context();
            if(call_context != NULL)  {
                if (BT_SINK_SRV_CALL_IS_FLAG_EXIST(call_context, BT_SINK_SRV_CALL_AUDIO_SWITCH)) {
                    bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_PLAYING_IDLE);
                    break;
                }
            }
#endif
            dev->audio_type = BT_SINK_SRV_CALL_AUDIO_SCO;
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_INIT);
            bt_sink_srv_call_psd_play_int(dev);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_ACTIVATE_REQ: {
            dev->audio_type = BT_SINK_SRV_CALL_AUDIO_SCO;
            dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_ACTIVATE_SCO, (void *)dev, NULL);
            bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_SCO_ACTIVATING);
        }
        break;


        case BT_SINK_SRV_CALL_EVENT_SCO_DEACTIVATED: {
            bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE);
            bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_CALL_END_IND: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_INIT);
            bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE);
            bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
        }
        break;

#ifdef AIR_FEATURE_SINK_AUDIO_SWITCH_SUPPORT
        case BT_SINK_SRV_CALL_EVENT_HF_SWITCH_START: {
            bt_sink_srv_sco_connection_state_t sco_state = BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED;
            dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_SCO_STATE, (void *)dev, (void *)&sco_state);
            if (sco_state == BT_SINK_SRV_SCO_CONNECTION_STATE_CONNECTED) {
                dev->audio_type = BT_SINK_SRV_CALL_AUDIO_SCO;
            }
            bt_sink_srv_call_psd_play_int(dev);
        }
        break;
#endif
        default: {
	bt_sink_srv_report_id("[CALL][PSD][MGR]Unexception event!!!", 0);
        }
        break;
    }
}

static void bt_sink_srv_call_psd_sub_state_playing(
    bt_sink_srv_call_pseudo_dev_t *dev,
    bt_sink_srv_call_state_event_t event,
    void *data)
{

    switch (event) {
        case BT_SINK_SRV_CALL_EVENT_LINK_DISCONNECTED: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_NONE);
            bt_sink_srv_call_psd_stop_int(dev);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_ACTIVATED: {
            if (dev->audio_type == BT_SINK_SRV_CALL_AUDIO_SCO) {
                bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_SCO);
                bt_sink_srv_call_psd_stop_int(dev);
            }
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_DISCONNECTED: {
            if (dev->audio_type == BT_SINK_SRV_CALL_AUDIO_SCO) {
                bt_sink_srv_hf_call_state_t state = BT_SINK_SRV_HF_CALL_STATE_IDLE;
                dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_CALL_STATE, (void *)dev, (void *)&state);
                if (state != BT_SINK_SRV_HF_CALL_STATE_IDLE) {
                    bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_NULL);
                } else {
                    bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
                }
                bt_sink_srv_call_psd_stop_int(dev);
            }
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_DEACTIVATE_REQ: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_DEACTIVATE_SCO);
            bt_sink_srv_call_psd_stop_int(dev);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_DEACTIVATED: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
            bt_sink_srv_call_psd_stop_int(dev);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_CALL_END_IND: {
            bt_sink_srv_sco_connection_state_t sco_state = BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED;
            dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_SCO_STATE, (void *)dev, (void *)&sco_state);
            if (sco_state == BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED) {
                bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
                bt_sink_srv_call_psd_stop_int(dev);
            }
        }
        break;

#ifdef AIR_FEATURE_SINK_AUDIO_SWITCH_SUPPORT
        case BT_SINK_SRV_CALL_EVENT_HF_SWITCH_STOP: {
            bt_sink_srv_call_psd_stop_int(dev);
        }
        break;
#endif
        default: {
	bt_sink_srv_report_id("[CALL][PSD][MGR]Unexception event!!!", 0);
        }
        break;
    }
}


static void bt_sink_srv_call_psd_sub_state_sco_activating(
    bt_sink_srv_call_pseudo_dev_t *dev,
    bt_sink_srv_call_state_event_t event,
    void *data)
{
    switch (event) {
        case BT_SINK_SRV_CALL_EVENT_SCO_ACTIVATED: {
#ifdef AIR_FEATURE_SINK_AUDIO_SWITCH_SUPPORT
            bt_sink_srv_call_context_t *call_context = bt_sink_srv_call_get_context();
            if(call_context != NULL)  {
                if (BT_SINK_SRV_CALL_IS_FLAG_EXIST(call_context, BT_SINK_SRV_CALL_AUDIO_SWITCH)) {
                     bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_PLAYING_IDLE);
                    bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_INIT);
                    break;
                }
            }
#endif
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_INIT);
            bt_sink_srv_call_psd_play_int(dev);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_DISCONNECTED: {
            if (dev->audio_type == BT_SINK_SRV_CALL_AUDIO_SCO || dev->next_state == BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_SCO) {
                bt_sink_srv_hf_call_state_t state = BT_SINK_SRV_HF_CALL_STATE_IDLE;
                dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_CALL_STATE, (void *)dev, (void *)&state);
                if (state != BT_SINK_SRV_HF_CALL_STATE_IDLE) {
                    bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_NULL);
                } else {
                    bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
                }
                bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_PLAYING_IDLE);
                bt_sink_srv_call_psd_run_next_state_in_sub_state_playing_idle(dev);
            }
        }
        break;
#ifdef AIR_FEATURE_SINK_AUDIO_SWITCH_SUPPORT
        case BT_SINK_SRV_CALL_EVENT_HF_SWITCH_STOP: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_NONE);
        }
        break;
#endif
        default: {
	bt_sink_srv_report_id("[CALL][PSD][MGR]Unexception event!!!", 0);
        }
        break;
    }
}

static void bt_sink_srv_call_psd_sub_state_sco_deactivating(
    bt_sink_srv_call_pseudo_dev_t *dev,
    bt_sink_srv_call_state_event_t event,
    void *data)
{
    switch (event) {
        case BT_SINK_SRV_CALL_EVENT_SCO_DEACTIVATED: {
            bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_PLAYING_IDLE);
            dev->audio_type = BT_SINK_SRV_CALL_AUDIO_NONE;
            bt_sink_srv_call_psd_run_next_state_in_sub_state_playing_idle(dev);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_DISCONNECTED: {
            bool is_highlight = false;
            bt_sink_srv_hf_call_state_t state = BT_SINK_SRV_HF_CALL_STATE_IDLE;
            dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_IS_HIGHLIGHT, (void *)dev, (void *)&is_highlight);
            dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_CALL_STATE, (void *)dev, (void *)&state);
            if (state != BT_SINK_SRV_HF_CALL_STATE_IDLE && is_highlight) {
                bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_NULL);
                bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_PLAYING_IDLE);
                bt_sink_srv_call_psd_run_next_state_in_sub_state_playing_idle(dev);
            } else {
                bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE);
                bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
            }
        }
        break;

        default: {
            bt_sink_srv_report_id("[CALL][PSD][MGR]Unexception event!!!", 0);
        }
        break;
    }
}

static void bt_sink_srv_call_psd_esco_disc_proc(bt_sink_srv_call_pseudo_dev_t *dev)
{
    if (dev->audio_type == BT_SINK_SRV_CALL_AUDIO_SCO || dev->next_state == BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_SCO) {
        bool is_highlight = false;
        bt_sink_srv_hf_call_state_t state = BT_SINK_SRV_HF_CALL_STATE_IDLE;
        dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_IS_HIGHLIGHT, (void *)dev, (void *)&is_highlight);
        dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_CALL_STATE, (void *)dev, (void *)&state);
        if (state != BT_SINK_SRV_HF_CALL_STATE_IDLE && is_highlight) {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_NULL);
        } else {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
        }
    }
}

static void bt_sink_srv_call_psd_sub_state_codec_starting(
    bt_sink_srv_call_pseudo_dev_t *dev,
    bt_sink_srv_call_state_event_t event,
    void *data)
{
    switch (event) {
        //play ok, switch sub state
        case BT_SINK_SRV_CALL_EVENT_PLAY_CODEC_IND: {
            bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_PLAYING);
            bt_sink_srv_call_psd_run_next_state_in_sub_state_playing(dev);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_LINK_CONNECTED: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
        }
        break;

        //Others, set next state flag.
        case BT_SINK_SRV_CALL_EVENT_LINK_DISCONNECTED: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_NONE);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_CONNECT_LINK_REQ: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_CONNECTING);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_DISCONNECTED: {
            bt_sink_srv_call_psd_esco_disc_proc(dev);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_ACTIVATE_REQ: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_ACTIVATE_SCO);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_ACTIVATED: {
            if ((dev->audio_type == BT_SINK_SRV_CALL_AUDIO_SCO) && (dev->next_state != BT_SINK_SRV_CALL_PSD_NEXT_STATE_INIT)) {
                /*eSCO was reconnected when starting codec.*/
                bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_SCO);
            }
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_DEACTIVATED: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_CALL_END_IND: {
            bt_sink_srv_sco_connection_state_t sco_state = BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED;
            dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_SCO_STATE, (void *)dev, (void *)&sco_state);
            if (sco_state == BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED) {
                bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
            }
        }
        break;
#ifdef AIR_FEATURE_SINK_AUDIO_SWITCH_SUPPORT
        case BT_SINK_SRV_CALL_EVENT_HF_SWITCH_STOP: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_NONE);
        }
        break;
#endif
        default: {
	bt_sink_srv_report_id("[CALL][PSD][MGR]Unexception event!!!", 0);
        }
        break;
    }
}

static void bt_sink_srv_call_psd_sub_state_codec_stopping(
    bt_sink_srv_call_pseudo_dev_t *dev,
    bt_sink_srv_call_state_event_t event,
    void *data)
{
    switch (event) {
        //stop ok, switch sub state
        case BT_SINK_SRV_CALL_EVENT_STOP_CODEC_IND: {
            if (BT_SINK_SRV_CALL_PSD_NEXT_STATE_DEACTIVATE_SCO == dev->next_state) {
                dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_DEACTIVATE_SCO, (void *)dev, NULL);
                bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_SCO_DEACTIVATING);
                bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
            } else {
                bt_sink_srv_call_psd_update_sub_state(dev->audio_src, BT_SINK_SRV_CALL_PSD_SUB_STATE_PLAYING_IDLE);
                if (dev->next_state == BT_SINK_SRV_CALL_PSD_NEXT_STATE_INIT && dev->audio_type == BT_SINK_SRV_CALL_AUDIO_SCO) {
                    bt_sink_srv_hf_call_state_t state = BT_SINK_SRV_HF_CALL_STATE_IDLE;
                    dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_CALL_STATE, (void *)dev, (void *)&state);
                    if (state == BT_SINK_SRV_HF_CALL_STATE_IDLE) {
                        bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
                    }
                }
                dev->audio_type = BT_SINK_SRV_CALL_AUDIO_NONE;
                bt_sink_srv_call_psd_run_next_state_in_sub_state_playing_idle(dev);
            }
        }
        break;

        //Others, set next state flag.
        case BT_SINK_SRV_CALL_EVENT_LINK_CONNECTED: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_LINK_DISCONNECTED: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_NONE);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_DISCONNECTED: {
            bt_sink_srv_call_psd_esco_disc_proc(dev);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_ACTIVATE_REQ: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_ACTIVATE_SCO);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_ACTIVATED: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_SCO);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_DEACTIVATED: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_CALL_END_IND: {
            bt_sink_srv_sco_connection_state_t sco_state = BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED;
            dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_SCO_STATE, (void *)dev, (void *)&sco_state);
            if (sco_state == BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED) {
                bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
            }
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_CALL_START_IND: {
            if (dev->next_state == BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY) {
                bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_PLAY_NULL);
            }
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_CONNECT_LINK_REQ: {
            if (dev->next_state == BT_SINK_SRV_CALL_PSD_NEXT_STATE_NONE) {
                bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_CONNECTING);
            }
        }
        break;

        default: {
            bt_sink_srv_report_id("[CALL][PSD][MGR]Unexception event!!! ", 0);
        }
        break;
    }
}

static void bt_sink_srv_call_psd_state_playing(
    bt_sink_srv_call_pseudo_dev_t *dev,
    bt_sink_srv_call_state_event_t event,
    void *data)
{
    bt_utils_assert(dev);
    bt_utils_assert(dev->audio_src);

    bt_sink_srv_report_id("[CALL][PSD][MGR]playing,event:0x%08x, substate:0x%x", 2, event, dev->audio_src->substate);

    uint8_t idx = 0;
    while (dev->audio_src->substate != playing_sub_state_fuc_list[idx].state) {
        idx++;
        }
    if(idx > 0 && idx < (sizeof(playing_sub_state_fuc_list)/sizeof(bt_sink_srv_call_pseudo_event_notify_t))) {
        playing_sub_state_fuc_list[idx].notify(dev, event, data);
    } else {
            bt_utils_assert(0);
        }
}

static void bt_sink_srv_call_psd_run_next_state_in_prepare_stop(bt_sink_srv_call_pseudo_dev_t *dev)
{
    bt_utils_assert(dev);
    bt_utils_assert(dev->audio_src);
    bt_sink_srv_report_id("[CALL][PSD][MGR]Next state:0x%x", 1, dev->next_state);
    switch (dev->next_state) {
        case BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY: {
            bt_sink_srv_hf_call_state_t call_state = BT_SINK_SRV_HF_CALL_STATE_IDLE;
            bt_sink_srv_sco_connection_state_t sco_state = BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED;
            dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_CALL_STATE, (void *)dev, (void *)&call_state);
            dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_SCO_STATE, (void *)dev, (void *)&sco_state);
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_INIT);
            bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_READY);
            if ((call_state != BT_SINK_SRV_HF_CALL_STATE_IDLE) ||
                (sco_state != BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED)) {
                bt_sink_srv_call_psd_add_waiting_list(dev);
            }
        }
        break;

        case BT_SINK_SRV_CALL_PSD_NEXT_STATE_DEACTIVATE_SCO: {
            dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_DEACTIVATE_SCO, dev, NULL);
            bt_sink_srv_call_psd_set_sub_state(dev, BT_SINK_SRV_CALL_PSD_SUB_STATE_SCO_DEACTIVATING);
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
        }
        break;

        case BT_SINK_SRV_CALL_PSD_NEXT_STATE_NONE: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_INIT);
            bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_READY);
            bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_UNAVAILABLE);
        }
        break;

        default: {
            bt_sink_srv_report_id("[CALL][PSD][MGR]Not handle:0x%x", 1, dev->next_state);
        }
        break;
    }
}

static void bt_sink_srv_call_psd_state_prepare_stop(
    bt_sink_srv_call_pseudo_dev_t *dev,
    bt_sink_srv_call_state_event_t event,
    void *data)
{
    switch (event) {
        case BT_SINK_SRV_CALL_EVENT_LINK_DISCONNECTED: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_NONE);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_STOP_CODEC_IND: {
            if (dev->flag & BT_SINK_SRV_CALL_PSD_FLAG_SCO_ACTIVED) {
                bt_sink_srv_call_psd_set_sub_state(dev, BT_SINK_SRV_CALL_PSD_SUB_STATE_SCO_DEACTIVATING);
                dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_DEACTIVATE_SCO, (void *)dev, NULL);
            } else {
                /*sco is disconnectd*/
                bt_sink_srv_call_psd_set_sub_state(dev, BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE);
                bt_sink_srv_call_psd_run_next_state_in_prepare_stop(dev);
            }
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_PLAY_CODEC_IND: {
            if(dev->flag & BT_SINK_SRV_CALL_PSD_FLAG_SUSPEND) {
                bt_sink_srv_call_psd_stop_int(dev);
            }
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_ACTIVATE_REQ: {
            bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_ACTIVATE_SCO);
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SUSPEND_REQ: {
            bt_sink_srv_hf_call_state_t call_state = BT_SINK_SRV_HF_CALL_STATE_IDLE;
            bt_sink_srv_sco_connection_state_t sco_state = BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED;
            dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_CALL_STATE, (void *)dev, (void *)&call_state);
            dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_GET_SCO_STATE, (void *)dev, &sco_state);
            if (dev->audio_src->substate == BT_SINK_SRV_CALL_PSD_SUB_STATE_PLAYING) {
                bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_DEACTIVATE_SCO);
                bt_sink_srv_call_psd_stop_int(dev);
            } else if (dev->audio_src->substate == BT_SINK_SRV_CALL_PSD_SUB_STATE_CODEC_STOPPING) {
                bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
            } else if (dev->audio_src->substate == BT_SINK_SRV_CALL_PSD_SUB_STATE_PLAYING_IDLE) {
                if (dev->flag & BT_SINK_SRV_CALL_PSD_FLAG_TAKED_MIC_RESOURCE) {
                    bt_sink_srv_call_psd_release_mic_resource(dev);
                }
                bt_sink_srv_call_psd_set_sub_state(dev, BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE);
                bt_sink_srv_call_psd_update_state(dev->audio_src, AUDIO_SRC_SRV_EVT_READY);
                if ((call_state != BT_SINK_SRV_HF_CALL_STATE_IDLE) ||
                    (sco_state != BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED)) {
                    bt_sink_srv_call_psd_add_waiting_list(dev);
                }
            } else if (dev->audio_src->substate == BT_SINK_SRV_CALL_PSD_SUB_STATE_SCO_ACTIVATING) {
                if (dev->flag & BT_SINK_SRV_CALL_PSD_FLAG_TAKED_MIC_RESOURCE) {
                    bt_sink_srv_call_psd_release_mic_resource(dev);
                }
                bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
                if (dev->flag & BT_SINK_SRV_CALL_PSD_FLAG_SCO_ACTIVED) {
                    bt_sink_srv_call_psd_set_sub_state(dev, BT_SINK_SRV_CALL_PSD_SUB_STATE_SCO_DEACTIVATING);
                    dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_DEACTIVATE_SCO, (void *)dev, NULL);
                }  
            } else if (dev->audio_src->substate == BT_SINK_SRV_CALL_PSD_SUB_STATE_CODEC_STARTING) {
                bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
                if (dev->flag & BT_SINK_SRV_CALL_PSD_FLAG_CODEC_STARTED) {
                    bt_sink_srv_call_psd_stop_int(dev);
                }
            }
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_DISCONNECTED: {
            if (dev->audio_src->substate == BT_SINK_SRV_CALL_PSD_SUB_STATE_PLAYING) {
                bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
                bt_sink_srv_call_psd_stop_int(dev);
            } else if (dev->audio_src->substate == BT_SINK_SRV_CALL_PSD_SUB_STATE_CODEC_STOPPING) {
                bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
            } else if (dev->audio_src->substate == BT_SINK_SRV_CALL_PSD_SUB_STATE_SCO_DEACTIVATING) {
                bt_sink_srv_call_psd_set_next_state(dev, BT_SINK_SRV_CALL_PSD_NEXT_STATE_READY);
                bt_sink_srv_call_psd_set_sub_state(dev, BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE);
                bt_sink_srv_call_psd_run_next_state_in_prepare_stop(dev);
            }
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_ACTIVATED: {
             if(dev->flag & BT_SINK_SRV_CALL_PSD_FLAG_SUSPEND) {
                bt_sink_srv_call_psd_set_sub_state(dev, BT_SINK_SRV_CALL_PSD_SUB_STATE_SCO_DEACTIVATING);
                dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_DEACTIVATE_SCO, (void *)dev, NULL);
             }
        }
        break;

        case BT_SINK_SRV_CALL_EVENT_SCO_DEACTIVATED: {
            bt_sink_srv_call_psd_set_sub_state(dev, BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE);
            bt_sink_srv_call_psd_run_next_state_in_prepare_stop(dev);
        }
        break;

        default: {
            bt_sink_srv_report_id("[CALL][PSD][MGR]Unexception event!!! dev:0x%08x, event:0x%x", 2, dev, event);
        }
        break;
    }
    return;
}

static void bt_sink_srv_call_psd_set_sub_state(
    bt_sink_srv_call_pseudo_dev_t *dev,
    bt_sink_srv_call_pseudo_dev_sub_state_t sub_state)
{
    bool is_valid = false;

    bt_utils_assert(dev);
    bt_utils_assert(dev->audio_src);

    bt_sink_srv_report_id("[CALL][PSD][MGR]Sub_state in: dev:0x%x, state:0x%x, sub_state:0x%x", 3, dev, dev->audio_src->state, dev->audio_src->substate);
    switch (sub_state) {
        case BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE:
            if (dev->audio_src->substate != BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE) {
                is_valid = true;
            }
            break;

        case BT_SINK_SRV_CALL_PSD_SUB_STATE_CONNECTING:
            if (dev->audio_src->state == AUDIO_SRC_SRV_STATE_NONE) {
                is_valid = true;
            }
            break;

        case BT_SINK_SRV_CALL_PSD_SUB_STATE_DISCONNECTING:
            if (dev->audio_src->state == AUDIO_SRC_SRV_STATE_READY) {
                is_valid = true;
            }
            break;

        case BT_SINK_SRV_CALL_PSD_SUB_STATE_PLAYING_IDLE:
            if (dev->audio_src->state == AUDIO_SRC_SRV_STATE_PLAYING||
                dev->audio_src->state == AUDIO_SRC_SRV_STATE_PREPARE_STOP ||
                dev->audio_src->substate == BT_SINK_SRV_CALL_PSD_SUB_STATE_MIC_RES_TAKING) {
                is_valid = true;
            }
            break;
        case BT_SINK_SRV_CALL_PSD_SUB_STATE_CODEC_STARTING:
        case BT_SINK_SRV_CALL_PSD_SUB_STATE_PLAYING:
        case BT_SINK_SRV_CALL_PSD_SUB_STATE_CODEC_STOPPING:
        case BT_SINK_SRV_CALL_PSD_SUB_STATE_SCO_DEACTIVATING:
            if (dev->audio_src->state == AUDIO_SRC_SRV_STATE_PREPARE_STOP) {
                is_valid = true;
            }
            break;

        default:
            break;
    }
    if (is_valid) {
        bt_sink_srv_call_psd_update_sub_state(dev->audio_src, sub_state);
    }
    bt_sink_srv_report_id("[CALL][PSD][MGR]Sub_state out: dev:0x%x, state:0x%x, sub_state:0x%x", 3, dev, dev->audio_src->state, dev->audio_src->substate);
}

void bt_sink_srv_call_psd_state_machine(
    bt_sink_srv_call_pseudo_dev_t *dev,
    bt_sink_srv_call_state_event_t event,
    void *data)
{
    bt_utils_assert(dev);
    bt_utils_assert(dev->audio_src);

    bt_sink_srv_report_id("[CALL][PSD][MGR]state machine IN:0x%x, state:(0x%x, 0x%x), event:0x%x", 4,
                          dev, dev->audio_src->state, dev->audio_src->substate, event);

    uint8_t idx = 0;
    while (dev->audio_src->state != psd_state_machine_fuc_list[idx].state) {
        idx++;
    }
    if(idx > 0 && idx < (sizeof(psd_state_machine_fuc_list)/sizeof(bt_sink_srv_call_pseudo_event_notify_t))) {
        psd_state_machine_fuc_list[idx].notify(dev, event, data);
    }

    bt_sink_srv_report_id("[CALL][PSD][MGR]state machine OUT: 0x%x, state:(0x%x, 0x%x), event:0x%x", 4,
                          dev, dev->audio_src->state, dev->audio_src->substate, event);

    //Need reset device after state is none.
    if (dev->audio_src->state == AUDIO_SRC_SRV_STATE_NONE &&
        dev->audio_src->substate == BT_SINK_SRV_CALL_PSD_SUB_STATE_NONE) {
        if (dev->flag & BT_SINK_SRV_CALL_PSD_FLAG_TAKED_MIC_RESOURCE) {
            bt_sink_srv_call_psd_release_mic_resource(dev);
        }
        bt_utils_assert(dev->user_cb);
        bt_sink_srv_call_psd_del_waiting_list(dev);
        dev->audio_src->dev_id = 0;
        dev->user_cb(BT_SINK_SRV_CALL_PSD_EVENT_DEINIT, (void *)dev, NULL);
        if (dev->next_state == BT_SINK_SRV_CALL_PSD_NEXT_STATE_POWER_OFF) {
            bt_sink_srv_call_psd_free_audio_src(dev);
            bt_sink_srv_call_psd_reset_device(dev);
        }
    }
}


bool audio_src_srv_call_psedev_compare(audio_src_srv_handle_t *coming, audio_src_srv_handle_t *cur)
{
    return false;
}


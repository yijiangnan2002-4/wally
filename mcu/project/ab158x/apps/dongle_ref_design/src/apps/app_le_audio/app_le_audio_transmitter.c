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
#include "bt_le_audio_def.h"
#include "app_le_audio.h"
#include "app_le_audio_usb.h"
#include "app_le_audio_utillity.h"
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
#include "app_le_audio_ucst.h"
#include "app_le_audio_ucst_utillity.h"
#endif
#ifdef AIR_LE_AUDIO_BIS_ENABLE
#include "app_le_audio_bcst.h"
#include "app_le_audio_bcst_utillity.h"
#endif
#include "bt_le_audio_source.h"

#include "apps_events_event_group.h"


#include "audio_transmitter_control.h"
#include "app_le_audio_transmitter.h"

#include "bt_avm.h"
#include "hal_usb.h"

#include "bt_le_audio_msglog.h"
#include "usbaudio_drv.h"
//#include "FreeRTOS.h"
//#include "timers.h"
//#include "bt_gap_le_audio.h"


/**************************************************************************************************
* Define
**************************************************************************************************/
#define APP_LE_AUDIO_AVM_BUFFER_NUM             2

/* Audio transmitter channel mode */
#define APP_LE_AUDIO_TRANSMITTER_CHANNEL_MODE_MONO        0x01
#define APP_LE_AUDIO_TRANSMITTER_CHANNEL_MODE_STEREO      0x02
#define APP_LE_AUDIO_TRANSMITTER_CHANNEL_MODE_DUAL_MONO   0x04

#define APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_NONE     0x00
#define APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_SET_VOL  0x01
#define APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_MIX      0x02
#define APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_UNMIX    0x03
#define APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_MUTE     0x04
#define APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_UNMUTE   0x05
#define APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_CHANNEL  0x06
#define APP_LE_AUDIO_TRANSMITTER_CURRENT_CONFIG_OP_MASK 0x0000000F

#define APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_SEND_TO_AIR_CH_0    0x00
#define APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_SEND_TO_AIR_CH_1    0x01
#define APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_RECV_FROM_AIR_CH_0  0x02
#define APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_RECV_FROM_AIR_CH_1  0x03
#define APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_MAX                 0x04
typedef uint8_t app_le_audio_transmitter_share_info_t;

#define APP_LE_AUDIO_TRANSMITTER_SHARE_BUFFER_SIZE  (5*1024)


/**************************************************************************************************
* Structure
**************************************************************************************************/
typedef struct  {
    uint16_t DataOffset;
    uint16_t _reserved_word_02h;
    uint32_t TimeStamp;
    uint16_t ConnEvtCnt;
    uint8_t SampleSeq;
    uint8_t _reserved_byte_0Bh;
    uint8_t PduHdrLo;
    uint8_t _reserved_byte_0Dh;
    uint8_t PduLen ;
    uint8_t _reserved_byte_0Fh;
    uint16_t DataLen;
    uint16_t _reserved_word_12h;
    uint32_t _reserved_long_0;
    uint32_t _reserved_long_1;
} LE_AUDIO_HEADER;

/**************************************************************************************************
* Variable
**************************************************************************************************/
static n9_dsp_share_info_t *g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_MAX];

extern app_le_audio_ctrl_t g_lea_ctrl;


/**************************************************************************************************
* Prototype
**************************************************************************************************/


/**************************************************************************************************
* Static Functions
**************************************************************************************************/

static void app_le_audio_handle_audio_transmitter_evt(audio_transmitter_event_t event, void *data, void *user_data)
{
    app_le_audio_stream_port_t port = (app_le_audio_stream_port_t)user_data;

    switch (event) {
        case AUDIO_TRANSMITTER_EVENT_START_SUCCESS: {
            LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] recv START_SUCCESS, port:%x", 1, port);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_AUDIO_TRANSMITTER,
                                APP_LE_AUDIO_EVENT_AUDIO_STREAM_START_SUCCESS_CNF, (void *)port, 0,
                                NULL, 0);
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_START_FAIL: {
            LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] recv START_FAIL, port:%x", 1, port);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_AUDIO_TRANSMITTER,
                                APP_LE_AUDIO_EVENT_AUDIO_STREAM_START_FAIL_CNF, (void *)port, 0,
                                NULL, 0);
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_STOP_SUCCESS: {
            LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] recv STOP_SUCCESS, port:%x", 1, port);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_AUDIO_TRANSMITTER,
                                APP_LE_AUDIO_EVENT_AUDIO_STREAM_STOP_CNF, (void *)port, 0,
                                NULL, 0);
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_DATA_NOTIFICATION: {
            LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] recv DATA_NOTIFICATION, port:%x", 1, port);
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_DATA_DIRECT: {
            LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] recv DATA_DIRECT, port:%x", 1, port);
            break;
        }
        case AUDIO_TRANSMITTER_EVENT_SET_RUNTIME_CONFIG_SUCCESS: {
            LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] recv RUNTIME_CONFIG_SUCCESS, port:%x", 1, port);
            ui_shell_send_event(false, EVENT_PRIORITY_HIGH, EVENT_GROUP_UI_SHELL_APP_AUDIO_TRANSMITTER,
                                APP_LE_AUDIO_EVENT_AUDIO_STREAM_CONFIG_CNF, (void *)port, 0,
                                NULL, 0);
            break;
        }
        default:
            break;
    }
}

static n9_dsp_share_info_t *app_le_audio_get_avm_buffer_info(app_le_audio_transmitter_share_info_t type, uint32_t bit_rate, uint32_t frame_interval, uint32_t frame_size)
{
    n9_dsp_share_info_t *p_dsp_info = NULL;
    uint32_t payload_size = 0;

    /* get codec frame size */
    payload_size = (bit_rate * frame_interval / 8 / 1000 / 1000);

    if ((0 == payload_size) || (payload_size != frame_size)) {
        LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] get_avm_buffer_info, type:%x bit_rate:%d frame_interval:%d frame_size:%d payload_size:%x", 5,
                          type,
                          bit_rate,
                          frame_interval,
                          frame_size,
                          payload_size);
        //LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] get_avm_buffer_info, error codec frame size:%d %d", 2, payload_size, frame_size);
        configASSERT(0);
    }

    /* get share buffer info */
    switch (type) {
        case APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_SEND_TO_AIR_CH_0: {
            p_dsp_info = (n9_dsp_share_info_t *)hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_BT_SEND_TO_AIR_0);
            break;
        }
        case APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_SEND_TO_AIR_CH_1: {
            p_dsp_info = (n9_dsp_share_info_t *)hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_BT_SEND_TO_AIR_1);
            break;
        }
        case APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_RECV_FROM_AIR_CH_0: {
            p_dsp_info = (n9_dsp_share_info_t *)hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_BT_RECEIVE_FROM_AIR_0);
            break;
        }
        case APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_RECV_FROM_AIR_CH_1: {
            p_dsp_info = (n9_dsp_share_info_t *)hal_audio_query_audio_transmitter_share_info(AUDIO_TRANSMITTER_SHARE_INFO_INDEX_BLE_AUDIO_DONGLE_BT_RECEIVE_FROM_AIR_1);
            break;
        }
        default:
            break;
    }

    if (NULL != p_dsp_info) {
        memset((void *)p_dsp_info->start_addr, 0, APP_LE_AUDIO_TRANSMITTER_SHARE_BUFFER_SIZE);
        p_dsp_info->read_offset = 0;
        p_dsp_info->write_offset = 0;
        p_dsp_info->sub_info.block_info.block_size = (payload_size + sizeof(LE_AUDIO_HEADER) + 3) / 4 * 4; //4B align
        p_dsp_info->sub_info.block_info.block_num   = APP_LE_AUDIO_TRANSMITTER_SHARE_BUFFER_SIZE / (p_dsp_info->sub_info.block_info.block_size);
    }

    LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] get_avm_buffer_info, type:%x info:%x", 2, type, p_dsp_info);

    return p_dsp_info;
}

static uint32_t app_le_audio_set_audio_transmitter_op_queue(uint32_t op_queue, uint8_t op_code)
{
    uint32_t tmp, i = 0;

    for (i = 0; i < 8; i++) {
        tmp = ((op_queue >> (4 * i)) & APP_LE_AUDIO_TRANSMITTER_CURRENT_CONFIG_OP_MASK);
        if (!tmp) {
            break;
        }
        if ((i != 0) && (tmp == op_code)) {
            LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] set_op, op_queue:%x op_code:%x i:%x", 3, op_queue, op_code, i);
            return op_queue;
        }
    }

    if (8 != i) {
        tmp = (op_queue | (op_code << (4 * i)));
        LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] set_op, op_queue:%x op_code:%x tmp:%x", 3, op_queue, op_code, tmp);
        return tmp;
    }

    LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] set_op, op_queue:%x op_code:%x i:%x", 3, op_queue, op_code, i);
    return op_queue;
}

static bt_status_t app_le_audio_set_audio_transmitter_volume_by_port(app_le_audio_stream_port_t port, app_le_audio_stream_info_t *p_stream_info)
{
    if ((APP_LE_AUDIO_STREAM_PORT_MAX <= port) || (NULL == p_stream_info)) {
        return BT_STATUS_FAIL;
    }

    ble_audio_dongle_runtime_config_operation_t opcode;
    audio_transmitter_runtime_config_t config;
    audio_transmitter_status_t ret;

    if (APP_LE_AUDIO_STREAM_PORT_MIC_0 == port) {
        opcode = BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_VOICE_DUL;
    } else {
        opcode = BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_MUSIC_DUL;
    }

    if (p_stream_info->mute) {
        config.ble_audio_dongle_runtime_config.vol_level.vol_level_l = 0;
        config.ble_audio_dongle_runtime_config.vol_level.vol_level_r = 0;
    } else {
        config.ble_audio_dongle_runtime_config.vol_level.vol_level_l = p_stream_info->vol_level_left;
        config.ble_audio_dongle_runtime_config.vol_level.vol_level_r = p_stream_info->vol_level_right;
    }

    config.ble_audio_dongle_runtime_config.vol_level.vol_ratio = 100;

    ret = audio_transmitter_set_runtime_config(p_stream_info->transmitter_id,
                                               opcode,
                                               &config);

    LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] set_vol, port:%x ret:%x mute:%x vol_level:%d %d", 5, port, ret, p_stream_info->mute,
                      config.ble_audio_dongle_runtime_config.vol_level.vol_level_l,
                      config.ble_audio_dongle_runtime_config.vol_level.vol_level_r);

    if (AUDIO_TRANSMITTER_STATUS_SUCCESS != ret) {
        return BT_STATUS_FAIL;
    }

    p_stream_info->wait_cnf |= APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_CONFIG;

    return BT_STATUS_SUCCESS;
}

static bt_status_t app_le_audio_set_audio_transmitter_channel(app_le_audio_stream_port_t port, app_le_audio_stream_info_t *p_stream_info)
{
    if ((APP_LE_AUDIO_STREAM_PORT_MAX <= port) || (NULL == p_stream_info)) {
        return BT_STATUS_FAIL;
    }

    ble_audio_dongle_runtime_config_operation_t opcode;
    audio_transmitter_runtime_config_t config;
    audio_transmitter_status_t ret;

    opcode = BLE_AUDIO_DONGLE_CONFIG_OP_SET_UL_CH1_INPUT_SOURCE;

    config.ble_audio_dongle_runtime_config.channel_enable = p_stream_info->mic_channel;

    ret = audio_transmitter_set_runtime_config(p_stream_info->transmitter_id,
                                               opcode,
                                               &config);

    LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] set_channel, port:%x ret:%x channel:%x", 3, port, ret, p_stream_info->mic_channel);

    if (AUDIO_TRANSMITTER_STATUS_SUCCESS != ret) {
        return BT_STATUS_FAIL;
    }

    p_stream_info->wait_cnf |= APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_CONFIG;

    return BT_STATUS_SUCCESS;
}

static void app_le_audio_set_audio_transmitter_share_buffer(void)
{
    bt_avm_leaudio_buffer_info_t *leaudio_buf;  /* ul: to controller, dl: from controller */

    if (NULL == (leaudio_buf = pvPortMalloc(APP_LE_AUDIO_AVM_BUFFER_NUM * sizeof(bt_leaudio_buffer_set_t) + sizeof(uint32_t)))) {
        configASSERT(leaudio_buf != NULL);
        LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] set_share_buffer, malloc fail", 0);
        return;
    }

    leaudio_buf->count = APP_LE_AUDIO_AVM_BUFFER_NUM;

    /* share buffer info (SEND_TO_AIR_CH_0) */
    leaudio_buf->buffer[0].ul_address = (uint32_t)g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_SEND_TO_AIR_CH_0];
    leaudio_buf->buffer[0].dl_address = (uint32_t)g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_SEND_TO_AIR_CH_0];
    /* share buffer info (SEND_TO_AIR_CH_1) */
    leaudio_buf->buffer[1].ul_address = (uint32_t)g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_SEND_TO_AIR_CH_1];
    leaudio_buf->buffer[1].dl_address = (uint32_t)g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_SEND_TO_AIR_CH_1];

    LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] set_share_buffer, (SEND_TO_AIR) CH_0:%x CH_1:%x", 2,
                      leaudio_buf->buffer[0].ul_address,
                      leaudio_buf->buffer[1].ul_address);

    if (g_lea_ctrl.bidirectional) {
        /* share buffer info (FROM_AIR_CH_0) */
        leaudio_buf->buffer[0].dl_address = (uint32_t)g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_RECV_FROM_AIR_CH_0];
        /* share buffer info (FROM_AIR_CH_1) */
        leaudio_buf->buffer[1].dl_address = (uint32_t)g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_RECV_FROM_AIR_CH_1];

        LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] set_share_buffer, (RECV_FROM_AIR) CH_0:%x CH_1:%x", 2,
                          leaudio_buf->buffer[0].dl_address,
                          leaudio_buf->buffer[1].dl_address);
    }

    /* set LE audio share buffer */
    bt_avm_set_leaudio_buffer(leaudio_buf);

    vPortFree(leaudio_buf);
}

static bt_status_t app_le_audio_mute_audio_transmitter_by_port(app_le_audio_stream_port_t port, app_le_audio_stream_info_t *p_stream_info)
{
    if ((APP_LE_AUDIO_STREAM_PORT_MAX <= port) || (NULL == p_stream_info)) {
        return BT_STATUS_FAIL;
    }

    ble_audio_dongle_runtime_config_operation_t opcode;
    audio_transmitter_runtime_config_t config;
    audio_transmitter_status_t ret;

    if (APP_LE_AUDIO_STREAM_PORT_MIC_0 == port) {
        opcode = BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_VOICE_DUL;
    } else {
        opcode = BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_MUSIC_DUL;
    }

    config.ble_audio_dongle_runtime_config.vol_level.vol_level_l = 0;
    config.ble_audio_dongle_runtime_config.vol_level.vol_level_r = 0;
    config.ble_audio_dongle_runtime_config.vol_level.vol_ratio = 100;

    ret = audio_transmitter_set_runtime_config(p_stream_info->transmitter_id,
                                               opcode,
                                               &config);

    LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] mute, port:%x ret:%x", 2, port, ret);

    if (AUDIO_TRANSMITTER_STATUS_SUCCESS != ret) {
        return BT_STATUS_FAIL;
    }

    p_stream_info->wait_cnf |= APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_CONFIG;

    return BT_STATUS_SUCCESS;
}

static bool app_le_audio_mix_audio_transmitter_by_port(app_le_audio_stream_port_t port, app_le_audio_stream_info_t *p_stream_info)
{
    audio_transmitter_runtime_config_t config;
    audio_transmitter_status_t ret;
    app_le_audio_stream_info_t *p_stream_info_tmp = NULL;

    if ((APP_LE_AUDIO_STREAM_PORT_MIC_0 == port) || (NULL == p_stream_info)) {
        return false;
    }

    if ((APP_LE_AUDIO_TRANSMITTER_STATE_STARTED != p_stream_info->curr_transmitter_state) ||
        (APP_LE_AUDIO_TRANSMITTER_STATE_IDLE != p_stream_info->next_transmitter_state)) {
        return false;
    }

    if (p_stream_info->is_mixed) {
        return true;
    }

    uint8_t streaming_port = app_le_audio_get_streaming_port();

    //mix other streaming port
    for(uint8_t i = 0; i < APP_LE_AUDIO_STREAM_PORT_MAX; i++) {
        if (i == port || i == APP_LE_AUDIO_STREAM_PORT_MIC_0) {
            continue;
        }
        p_stream_info_tmp = &g_lea_ctrl.stream_info[i];
        if (((streaming_port >> i) & 0x01) &&
            ((APP_LE_AUDIO_TRANSMITTER_STATE_STARTED == p_stream_info_tmp->curr_transmitter_state) &&
            (APP_LE_AUDIO_TRANSMITTER_STATE_IDLE == p_stream_info_tmp->next_transmitter_state))) {
            //other prot is streaming && at STARTED state
            config.ble_audio_dongle_runtime_config.dl_mixer_id = p_stream_info_tmp->transmitter_id;
            ret = audio_transmitter_set_runtime_config(p_stream_info->transmitter_id, BLE_AUDIO_DONGLE_CONFIG_OP_MUSIC_MIX, &config);
            LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] mix port:%d, ret:%x mixer_id:%x", 3, port, ret, config.ble_audio_dongle_runtime_config.dl_mixer_id);
            if (AUDIO_TRANSMITTER_STATUS_SUCCESS != ret) {
                LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] mix fail! port:%d", 1, port);
                return false;
            }else {
                p_stream_info->wait_cnf |= APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_CONFIG;
            }
        }
    }
    //mix self
    config.ble_audio_dongle_runtime_config.dl_mixer_id = p_stream_info->transmitter_id;
    ret = audio_transmitter_set_runtime_config(p_stream_info->transmitter_id, BLE_AUDIO_DONGLE_CONFIG_OP_MUSIC_MIX, &config);
    LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] mix self port:%d, ret:%x mixer_id:%x", 3, port, ret, config.ble_audio_dongle_runtime_config.dl_mixer_id);
    if (AUDIO_TRANSMITTER_STATUS_SUCCESS != ret) {
        LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] mix self fail! port:%d", 1, port);
        return false;
    }else {
        p_stream_info->wait_cnf |= APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_CONFIG;
    }
    p_stream_info->is_mixed = true;

    return true;
}

static bool app_le_audio_unmix_audio_transmitter_by_port(app_le_audio_stream_port_t port, app_le_audio_stream_info_t *p_stream_info)
{
    if ((APP_LE_AUDIO_STREAM_PORT_MAX <= port) || (APP_LE_AUDIO_STREAM_PORT_MIC_0 == port) || (NULL == p_stream_info)) {
        return false;
    }

    if (p_stream_info->is_mixed) {
        audio_transmitter_status_t ret;

        p_stream_info->is_mixed = false;
        ret = audio_transmitter_set_runtime_config(p_stream_info->transmitter_id, BLE_AUDIO_DONGLE_CONFIG_OP_MUSIC_UNMIX, NULL);
        LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] unmix SPK_%x, ret:%x", 2, port, ret);
        if (AUDIO_TRANSMITTER_STATUS_SUCCESS != ret) {
            p_stream_info->is_mixed = true;
            return false;
        }

        p_stream_info->wait_cnf |= APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_CONFIG;
        return true;
    }

    return false;
}

bt_status_t app_le_audio_init_audio_transmitter(app_le_audio_stream_port_t port)
{
    audio_transmitter_config_t config;
#ifdef AIR_SILENCE_DETECTION_ENABLE
    app_le_audio_silence_detection_mode_enum silence_detection_mode = app_le_audio_silence_detection_get_silence_detection_mode();
#endif
    app_le_audio_stream_info_t *p_stream_info = NULL;
    app_le_audio_usb_port_t lea_usb_port = APP_LE_AUDIO_CONVERT_STREAMING_PORT(port);
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    bool is_mic_port = (port == APP_LE_AUDIO_STREAM_PORT_MIC_0);
#endif

    if (APP_LE_AUDIO_STREAM_PORT_MAX <= port) {
        return BT_STATUS_FAIL;
    }

#ifndef AIR_LE_AUDIO_UNICAST_ENABLE
    if (APP_LE_AUDIO_STREAM_PORT_MIC_0 == port) {
        return BT_STATUS_UNSUPPORTED;
    }
#endif
    p_stream_info = &g_lea_ctrl.stream_info[port];

    if (APP_LE_AUDIO_TRANSMITTER_STATE_INIT <= p_stream_info->curr_transmitter_state) {
        return BT_STATUS_SUCCESS;
    }

    if (port <= APP_LE_AUDIO_STREAM_PORT_MIC_0) {//only useful for USB Port
        app_le_audio_usb_config_info_t usb_config_info = {
            .usb_sample_rate = 16000,
            .usb_sample_size = 0,
            .usb_channel = 1
        };

        if (!app_le_audio_usb_is_port_ready(lea_usb_port, &usb_config_info)) {
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
            if (APP_LE_AUDIO_UCST_CREATE_CIS_BY_USB_PORT == app_le_audio_ucst_get_create_cis_mode()) {
                return BT_STATUS_FAIL;
            }
#else
            return BT_STATUS_FAIL;
#endif
        }
        p_stream_info->usb_sample_rate = usb_config_info.usb_sample_rate;
        p_stream_info->usb_sample_size = usb_config_info.usb_sample_size;
        p_stream_info->usb_channel = usb_config_info.usb_channel;
    }



#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    uint32_t bitrate = 0, framesize = 0, frame_duration = 0, sampling_freq = 0;//sink
    uint32_t bitrate_source = 0, framesize_source = 0, frame_duration_source = 0, sampling_freq_source = 0;

    uint16_t context_type = app_le_audio_ucst_get_audio_context_type();
    uint16_t audio_location_channels = 0;
    app_le_audio_ase_codec_t *p_codec_state = NULL;
    app_le_audio_ase_codec_t *p_codec_state_source = NULL;
    if (APP_LE_AUDIO_MODE_UCST == g_lea_ctrl.curr_mode) {
        p_codec_state = app_le_audio_ucst_get_ase_codec_config(context_type, AUDIO_DIRECTION_SINK);
        p_codec_state_source = app_le_audio_ucst_get_ase_codec_config(context_type, AUDIO_DIRECTION_SOURCE);

        if (NULL != p_codec_state) {
            audio_location_channels = app_le_audio_ucst_get_location_count(p_codec_state->audio_channel_allocation);
            if (0 == audio_location_channels) {
                audio_location_channels = 1;
            }
            frame_duration = p_codec_state->frame_duration ? 10000 : 7500;

            framesize = p_codec_state->codec_frame_blocks_per_sdu * audio_location_channels * p_codec_state->octets_per_codec_frame;
            bitrate = (framesize * 8 * 1000 * 1000) / frame_duration;
            if ((framesize * 8 * 1000 * 1000) % frame_duration) {
                bitrate += 1;
            }
            sampling_freq =  app_le_audio_convert_sample_freq(p_codec_state->sampling_frequency);
        }

        if (APP_LE_AUDIO_STREAM_PORT_MIC_0 == port) {
            if (NULL != p_codec_state_source) {
                audio_location_channels = app_le_audio_ucst_get_location_count(p_codec_state_source->audio_channel_allocation);
                if (0 == audio_location_channels) {
                    audio_location_channels = 1;
                }
                frame_duration_source = p_codec_state_source->frame_duration ? 10000 : 7500;

                framesize_source = p_codec_state_source->codec_frame_blocks_per_sdu * audio_location_channels * p_codec_state_source->octets_per_codec_frame;
                bitrate_source = (framesize_source * 8 * 1000 * 1000) / frame_duration_source;
                if ((framesize_source * 8 * 1000 * 1000) % frame_duration_source) {
                    bitrate_source += 1;
                }
                sampling_freq_source =  app_le_audio_convert_sample_freq(p_codec_state_source->sampling_frequency);
            }
            else {
                return BT_STATUS_FAIL;
            }
        }

        if ((NULL == p_codec_state) && (NULL != p_codec_state_source)) {
            bitrate = bitrate_source;
            framesize = framesize_source;
            frame_duration = frame_duration_source;
            sampling_freq = sampling_freq_source;
        }

        if ((NULL == p_codec_state_source) && (NULL == p_codec_state)) {
            return BT_STATUS_FAIL;
        }

        if (APP_LE_AUDIO_STREAM_PORT_MIC_0 == port) {
            if ((0 == bitrate_source) || (0 == framesize_source) || (0 == frame_duration_source) || (0 == sampling_freq_source)) {
                LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] init fail, port:%x bitrate_source:%d framesize_source:%d frame_duration_source:%d sampling_freq_source:%d", 5,
                                    port,
                                    bitrate_source,
                                    framesize_source,
                                    frame_duration_source,
                                    sampling_freq_source);
                return BT_STATUS_FAIL;
            }
        }
        else {
            if (((0 == bitrate) || (0 == framesize) || (0 == frame_duration) || (0 == sampling_freq))) {
                LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] init fail, port:%x bitrate:%d framesize:%d frame_duration:%d sampling_freq:%d", 5,
                                    port,
                                    bitrate,
                                    framesize,
                                    frame_duration,
                                    sampling_freq);
                return BT_STATUS_FAIL;
            }
        }
    }
/*
    framesize = app_le_audio_ucst_get_sdu_size(false);

    if ((APP_LE_AUDIO_MODE_UCST == g_lea_ctrl.curr_mode) && (APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_4 == app_le_audio_ucst_get_create_cis_mode())) {
        framesize *= 2;
        bitrate = (framesize * 8 * 1000 * 1000) / app_le_audio_ucst_get_sdu_interval(false);
    } else {
        bitrate = (app_le_audio_ucst_get_bitrate(false) * 1000);
    }
*/
#endif

    /* Always set downlink AVM if it is not set because currently CIS always enable downlink.
        * Downlink AVM need be ready before CIS creation even when SPK USB port is not enabled or ready.
        */

    if (NULL == g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_SEND_TO_AIR_CH_0]) {
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
        if (APP_LE_AUDIO_MODE_UCST == g_lea_ctrl.curr_mode) {
            g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_SEND_TO_AIR_CH_0] = app_le_audio_get_avm_buffer_info(APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_SEND_TO_AIR_CH_0,
                                                                                                                      bitrate,
                                                                                                                      frame_duration,
                                                                                                                      framesize);
            g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_SEND_TO_AIR_CH_1] = app_le_audio_get_avm_buffer_info(APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_SEND_TO_AIR_CH_1,
                                                                                                                      bitrate,
                                                                                                                      frame_duration,
                                                                                                                      framesize);
        }
#endif
#ifdef AIR_LE_AUDIO_BIS_ENABLE
        if (APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.curr_mode &&
            APP_LE_AUDIO_STREAM_PORT_MIC_0 != port) {
            g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_SEND_TO_AIR_CH_0] = app_le_audio_get_avm_buffer_info(APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_SEND_TO_AIR_CH_0,
                                                                                                                      (app_le_audio_bcst_get_bitrate() * 1000),
                                                                                                                      app_le_audio_bcst_get_sdu_interval(),
                                                                                                                      app_le_audio_bcst_get_sdu_size());
            g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_SEND_TO_AIR_CH_1] = app_le_audio_get_avm_buffer_info(APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_SEND_TO_AIR_CH_1,
                                                                                                                      (app_le_audio_bcst_get_bitrate() * 1000),
                                                                                                                      app_le_audio_bcst_get_sdu_interval(),
                                                                                                                      app_le_audio_bcst_get_sdu_size());
        }
#endif
    }

#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    if ((APP_LE_AUDIO_MODE_UCST == g_lea_ctrl.curr_mode) &&
        (NULL == g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_RECV_FROM_AIR_CH_0]) &&
        (APP_LE_AUDIO_STREAM_PORT_MIC_0 == port)) {
        g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_RECV_FROM_AIR_CH_0] = app_le_audio_get_avm_buffer_info(APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_RECV_FROM_AIR_CH_0,
                                                                                                                    bitrate_source,
                                                                                                                    frame_duration_source,
                                                                                                                    framesize_source);
        g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_RECV_FROM_AIR_CH_1] = app_le_audio_get_avm_buffer_info(APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_RECV_FROM_AIR_CH_1,
                                                                                                                    bitrate_source,
                                                                                                                    frame_duration_source,
                                                                                                                    framesize_source);
    }
#endif

    /* Init audio stream */
    memset(&config, 0x00, sizeof(config));
    config.scenario_type = AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE;
    config.msg_handler = app_le_audio_handle_audio_transmitter_evt;
    config.user_data = (void *)port;
    //audio_transmitter_ble_audio_dongle_config_t
    music_ble_audio_dongle_config_t *p_ble_audio_dongle_config = &config.scenario_config.ble_audio_dongle_config.music_ble_audio_dongle_config;
    //voice_ble_audio_dongle_config_t is same to music_ble_audio_dongle_config_t:addr and structue
    //voice_ble_audio_dongle_config_t *p_ble_audio_dongle_config = &config.scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config;
    /* BT & Codec setting */
    p_ble_audio_dongle_config->channel_enable = APP_LE_AUDIO_TRANSMITTER_CHANNEL_DUAL;
#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    if (APP_LE_AUDIO_MODE_UCST == g_lea_ctrl.curr_mode) {
        /*
        if ((APP_LE_AUDIO_UCST_CREATE_CIS_WITH_AC_4 != app_le_audio_ucst_get_create_cis_mode())) {
            bitrate = (app_le_audio_ucst_get_bitrate(is_mic_port) * 1000);
            framesize = app_le_audio_ucst_get_sdu_size(is_mic_port);
        }
        */

        if ((is_mic_port && (160 > framesize_source)) || (!is_mic_port && (160 > framesize))) {
            p_ble_audio_dongle_config->codec_type = AUDIO_DSP_CODEC_TYPE_LC3;
            p_ble_audio_dongle_config->period = is_mic_port ? frame_duration_source : frame_duration;//app_le_audio_ucst_get_sdu_interval(is_mic_port);    /* To do: ISO interval */
            p_ble_audio_dongle_config->codec_param.lc3.sample_rate = is_mic_port ? sampling_freq_source : sampling_freq;//app_le_audio_ucst_get_sampling_rate(is_mic_port);
            p_ble_audio_dongle_config->codec_param.lc3.bit_rate = is_mic_port ? bitrate_source : bitrate;//bitrate;
            p_ble_audio_dongle_config->codec_param.lc3.channel_mode = APP_LE_AUDIO_TRANSMITTER_CHANNEL_MODE_DUAL_MONO;
            p_ble_audio_dongle_config->codec_param.lc3.frame_interval = is_mic_port ? frame_duration_source : frame_duration;//app_le_audio_ucst_get_sdu_interval(is_mic_port);
            p_ble_audio_dongle_config->codec_param.lc3.frame_size = is_mic_port ? framesize_source : framesize;//framesize;

        } else {
            p_ble_audio_dongle_config->codec_type = AUDIO_DSP_CODEC_TYPE_LC3PLUS;
            p_ble_audio_dongle_config->period = is_mic_port ? frame_duration_source : frame_duration;//app_le_audio_ucst_get_sdu_interval(is_mic_port);    /* To do: ISO interval */
            p_ble_audio_dongle_config->codec_param.lc3plus.sample_rate = is_mic_port ? sampling_freq_source : sampling_freq;//app_le_audio_ucst_get_sampling_rate(is_mic_port);
            p_ble_audio_dongle_config->codec_param.lc3plus.bit_rate = is_mic_port ? bitrate_source : bitrate;//bitrate;
            p_ble_audio_dongle_config->codec_param.lc3plus.channel_mode = APP_LE_AUDIO_TRANSMITTER_CHANNEL_MODE_DUAL_MONO;
            p_ble_audio_dongle_config->codec_param.lc3plus.frame_interval = is_mic_port ? frame_duration_source : frame_duration;//app_le_audio_ucst_get_sdu_interval(is_mic_port);
            p_ble_audio_dongle_config->codec_param.lc3plus.frame_size = is_mic_port ? framesize_source : framesize;//framesize;
        }
    }
#endif
#ifdef AIR_LE_AUDIO_BIS_ENABLE
    if (APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.curr_mode) {

        p_ble_audio_dongle_config->period = app_le_audio_bcst_get_sdu_interval();

        if (160 > app_le_audio_bcst_get_sdu_size()) {
            p_ble_audio_dongle_config->codec_type = AUDIO_DSP_CODEC_TYPE_LC3;
            p_ble_audio_dongle_config->codec_param.lc3.sample_rate = app_le_audio_bcst_get_sampling_rate();
            p_ble_audio_dongle_config->codec_param.lc3.bit_rate = (app_le_audio_bcst_get_bitrate() * 1000);
            p_ble_audio_dongle_config->codec_param.lc3.channel_mode = APP_LE_AUDIO_TRANSMITTER_CHANNEL_MODE_DUAL_MONO;
            p_ble_audio_dongle_config->codec_param.lc3.frame_interval = app_le_audio_bcst_get_sdu_interval();
            p_ble_audio_dongle_config->codec_param.lc3.frame_size = app_le_audio_bcst_get_sdu_size();
        } else {
            p_ble_audio_dongle_config->codec_type = AUDIO_DSP_CODEC_TYPE_LC3PLUS;
            p_ble_audio_dongle_config->codec_param.lc3plus.sample_rate = app_le_audio_bcst_get_sampling_rate();
            p_ble_audio_dongle_config->codec_param.lc3plus.bit_rate = (app_le_audio_bcst_get_bitrate() * 1000);
            p_ble_audio_dongle_config->codec_param.lc3plus.channel_mode = APP_LE_AUDIO_TRANSMITTER_CHANNEL_MODE_DUAL_MONO;
            p_ble_audio_dongle_config->codec_param.lc3plus.frame_interval = app_le_audio_bcst_get_sdu_interval();
            p_ble_audio_dongle_config->codec_param.lc3plus.frame_size = app_le_audio_bcst_get_sdu_size();
        }

    }
#endif
    /* Test mode setting */
    p_ble_audio_dongle_config->test_mode_enable = app_le_audio_get_test_mode();
#ifdef AIR_SILENCE_DETECTION_ENABLE
    /* Silence detection setting */
    p_ble_audio_dongle_config->without_bt_link_mode_enable = (APP_LE_AUDIO_SILENCE_DETECTION_MODE_SPECIAL == silence_detection_mode);
#endif
    /* Volume setting. Mute works only after audio transmitter start. Therefore, do not check mute status at the initial stage. */
    if (p_stream_info->mute) {
        p_ble_audio_dongle_config->vol_level.vol_level_l = 0;
        p_ble_audio_dongle_config->vol_level.vol_level_r = 0;
    } else {
        p_ble_audio_dongle_config->vol_level.vol_level_l = p_stream_info->vol_level_left;
        p_ble_audio_dongle_config->vol_level.vol_level_r = p_stream_info->vol_level_right;
    }
    p_ble_audio_dongle_config->vol_level.vol_ratio = 100;

    switch (port) {
#if defined AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE || defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
        case APP_LE_AUDIO_STREAM_PORT_LINE_IN: {
            config.scenario_sub_id = AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_LINE_IN;
            p_ble_audio_dongle_config->vol_level.vol_ratio = 70;
            break;
        }
        case APP_LE_AUDIO_STREAM_PORT_I2S_IN: {
            config.scenario_sub_id = AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_I2S_IN;
            p_ble_audio_dongle_config->vol_level.vol_ratio = 70;
            break;
        }
#endif
        case APP_LE_AUDIO_STREAM_PORT_SPK_0:
        case APP_LE_AUDIO_STREAM_PORT_SPK_1: {
            config.scenario_sub_id = (port == APP_LE_AUDIO_STREAM_PORT_SPK_0) ? AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0 : AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1;

            /* USB setting */
            p_ble_audio_dongle_config->usb_type = AUDIO_DSP_CODEC_TYPE_PCM;

            p_ble_audio_dongle_config->usb_param.pcm.sample_rate = p_stream_info->usb_sample_rate;
            p_ble_audio_dongle_config->usb_param.pcm.format = app_le_audio_usb_convert_sample_size(p_stream_info->usb_sample_size);
            p_ble_audio_dongle_config->usb_param.pcm.channel_mode = p_stream_info->usb_channel;
            break;
        }

#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
        case APP_LE_AUDIO_STREAM_PORT_MIC_0: {
            config.scenario_sub_id = AUDIO_TRANSMITTER_BLE_AUDIO_DONGLE_VOICE_USB_OUT;
            /* USB setting */
            p_ble_audio_dongle_config->usb_type = AUDIO_DSP_CODEC_TYPE_PCM;

            p_ble_audio_dongle_config->usb_param.pcm.sample_rate = p_stream_info->usb_sample_rate;
            p_ble_audio_dongle_config->usb_param.pcm.format = app_le_audio_usb_convert_sample_size(p_stream_info->usb_sample_size);
            p_ble_audio_dongle_config->usb_param.pcm.channel_mode = p_stream_info->usb_channel;
#if 0
            uint32_t channel = app_le_audio_ucst_get_available_channel();
            LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] init MIC_0, available channel %x", 1, channel);

            switch (channel) {
                case AUDIO_LOCATION_FRONT_LEFT:
                    config.scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.channel_enable = APP_LE_AUDIO_TRANSMITTER_CHANNEL_L;
                    break;
                case AUDIO_LOCATION_FRONT_RIGHT:
                    config.scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.channel_enable = APP_LE_AUDIO_TRANSMITTER_CHANNEL_R;
                    break;
                default:
                    config.scenario_config.ble_audio_dongle_config.voice_ble_audio_dongle_config.channel_enable = APP_LE_AUDIO_TRANSMITTER_CHANNEL_DUAL;
                    break;
            }
#endif

            break;
        }
#endif
        default:
            return BT_STATUS_FAIL;
    }

    LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] init, port:%x port_state:%x->%x", 3, port,
                      p_stream_info->curr_transmitter_state,
                      p_stream_info->next_transmitter_state);

    if ((APP_LE_AUDIO_TRANSMITTER_STATE_IDLE != p_stream_info->curr_transmitter_state) &&
        (APP_LE_AUDIO_TRANSMITTER_STATE_IDLE != p_stream_info->next_transmitter_state)) {
        return BT_STATUS_FAIL;
    }

    p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_INIT;

    if (0 > (p_stream_info->transmitter_id = audio_transmitter_init(&config))) {
        LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] init fail, port:%x", 1, port);
        p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_IDLE;
        return BT_STATUS_FAIL;
    }

    p_stream_info->curr_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_INIT;
    p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_IDLE;
    LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] init, port:%x transmitter_id:%x op_queue:%x wait_cnf:%x", 4,
                      port, p_stream_info->transmitter_id, p_stream_info->config_op_queue, p_stream_info->wait_cnf);

    return BT_STATUS_SUCCESS;
}


static bt_status_t app_le_audio_deinit_audio_transmitter(app_le_audio_stream_port_t port, app_le_audio_stream_info_t *p_stream_info)
{
    audio_transmitter_status_t ret;

    if (NULL == p_stream_info) {
        return BT_STATUS_FAIL;
    }

    ret = audio_transmitter_deinit(p_stream_info->transmitter_id);
    LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] deinit, port:%x transmitter_id:%x ret:%x op_queue:%x wait_cnf:%x", 5,
                      port, p_stream_info->transmitter_id, ret, p_stream_info->config_op_queue, p_stream_info->wait_cnf);

    p_stream_info->curr_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_IDLE;
    p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_IDLE;
    p_stream_info->is_mixed = false;
    p_stream_info->config_op_queue = 0;
    p_stream_info->transmitter_id = -1;
    p_stream_info->wait_cnf = 0;

    return BT_STATUS_SUCCESS;
}

static void app_le_audio_handle_audio_transmitter_start_cnf(bool is_success, app_le_audio_stream_port_t port)
{
    app_le_audio_stream_info_t *p_stream_info = NULL;

    if (APP_LE_AUDIO_STREAM_PORT_MAX <= port) {
        return;
    }

    p_stream_info = &g_lea_ctrl.stream_info[port];

    LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] handle_start_cnf, port:%x is_success:%x port_state:%x->%x op_queue:%x", 5,
                      port, is_success,
                      p_stream_info->curr_transmitter_state,
                      p_stream_info->next_transmitter_state,
                      p_stream_info->config_op_queue);

    p_stream_info->wait_cnf &= (~APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_START);
    if (!is_success) {
        p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_IDLE;
        return;
    }

    p_stream_info->curr_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_STARTING;

    if (!g_lea_ctrl.set_buffer) {
        if (g_lea_ctrl.bidirectional) {
            if (APP_LE_AUDIO_STREAM_PORT_MIC_0 == port) {
                g_lea_ctrl.set_buffer = true;
                app_le_audio_set_audio_transmitter_share_buffer();
            }
        } else {
            g_lea_ctrl.set_buffer = true;
            app_le_audio_set_audio_transmitter_share_buffer();
        }
    }

    /* check next transmitter state */
    if ((APP_LE_AUDIO_TRANSMITTER_STATE_STOP == p_stream_info->next_transmitter_state) ||
        (APP_LE_AUDIO_TRANSMITTER_STATE_DEINIT == p_stream_info->next_transmitter_state)) {
        if (!(APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_STOP & p_stream_info->wait_cnf)) {
            p_stream_info->config_op_queue = 0;

            if (!(APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_STOP & p_stream_info->wait_cnf)) {
                audio_transmitter_status_t ret = audio_transmitter_stop(p_stream_info->transmitter_id);

                LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] handle_start_cnf stop, port:%x transmitter_id:%x ret:%x", 3, port, p_stream_info->transmitter_id, ret);
                if (AUDIO_TRANSMITTER_STATUS_SUCCESS != ret) {
                    p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_IDLE;
                } else {
                    p_stream_info->wait_cnf |= APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_STOP;
                }
            }
        }
        return;
    } else {
        p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_SET_VOL;
        p_stream_info->config_op_queue = 0;

        if (BT_STATUS_SUCCESS != app_le_audio_set_audio_transmitter_volume_by_port(port, p_stream_info)) {
            p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_IDLE;
        }
    }

#ifdef AIR_SILENCE_DETECTION_ENABLE
    if (APP_LE_AUDIO_TRANSMITTER_STATE_STOP != p_stream_info->next_transmitter_state &&
        APP_LE_AUDIO_TRANSMITTER_STATE_DEINIT != p_stream_info->next_transmitter_state) {
        app_le_audio_silence_detection_mode_enum silence_detection_mode = app_le_audio_silence_detection_get_silence_detection_mode();

        if (APP_LE_AUDIO_SILENCE_DETECTION_MODE_NORMAL == silence_detection_mode ||
            APP_LE_AUDIO_SILENCE_DETECTION_MODE_SPECIAL == silence_detection_mode) {
            /* Start silence detetion */
            app_le_audio_silence_detection_start_by_port(port);
        }
    }
#endif

    uint8_t i;
    i = APP_LE_AUDIO_STREAM_PORT_MAX;
    while (i > 0) {
        i--;
        if ((APP_LE_AUDIO_TRANSMITTER_STATE_INIT == g_lea_ctrl.stream_info[i].curr_transmitter_state) &&
            ((APP_LE_AUDIO_TRANSMITTER_STATE_STARTING == g_lea_ctrl.stream_info[i].next_transmitter_state) ||
             (APP_LE_AUDIO_TRANSMITTER_STATE_STOP == g_lea_ctrl.stream_info[i].next_transmitter_state))) {
            return;
        }
    }

#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    if (APP_LE_AUDIO_MODE_UCST == g_lea_ctrl.curr_mode) {
        app_le_audio_ucst_open_audio_transmitter_cb();
    }
#endif
#ifdef AIR_LE_AUDIO_BIS_ENABLE
    if (APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.curr_mode) {
        app_le_audio_bcst_open_audio_transmitter_cb();
    }
#endif
}

static void app_le_audio_handle_audio_transmitter_stop_cnf(app_le_audio_stream_port_t port)
{
    app_le_audio_stream_info_t *p_stream_info;

    if (APP_LE_AUDIO_STREAM_PORT_MAX <= port) {
        return;
    }

    p_stream_info = &g_lea_ctrl.stream_info[port];

    LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] handle_stop_cnf, port:%x port_state:%x->%x op_queue:%x", 4,
                      port,
                      p_stream_info->curr_transmitter_state,
                      p_stream_info->next_transmitter_state,
                      p_stream_info->config_op_queue);


    p_stream_info->wait_cnf &= (~APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_STOP);
    p_stream_info->curr_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_STOP;

    /* check next transmitter state */
    if ((APP_LE_AUDIO_TRANSMITTER_STATE_STARTING == p_stream_info->next_transmitter_state) ||
        (APP_LE_AUDIO_TRANSMITTER_STATE_SET_VOL == p_stream_info->next_transmitter_state)) {
        p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_STARTING;

        if (!(APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_START & p_stream_info->wait_cnf)) {
            audio_transmitter_status_t ret = audio_transmitter_start(p_stream_info->transmitter_id);

            LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] handle_stop_cnf start, port:%x transmitter_id:%x ret:%x", 3, port, p_stream_info->transmitter_id, ret);
            if (AUDIO_TRANSMITTER_STATUS_SUCCESS != ret) {
                p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_IDLE;
            } else {
                p_stream_info->wait_cnf |= APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_START;
            }
        }
        return;
    }

    if (APP_LE_AUDIO_TRANSMITTER_STATE_DEINIT == p_stream_info->next_transmitter_state) {
        app_le_audio_deinit_audio_transmitter(port, p_stream_info);
    }

    p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_IDLE;
    p_stream_info->config_op_queue = 0;

    if (!g_lea_ctrl.open_audio_transmitter) {
        uint8_t i;
        i = APP_LE_AUDIO_STREAM_PORT_MAX;
        while (i > 0) {
            i--;
            if ((APP_LE_AUDIO_TRANSMITTER_STATE_IDLE != g_lea_ctrl.stream_info[i].curr_transmitter_state) ||
                (APP_LE_AUDIO_TRANSMITTER_STATE_IDLE != g_lea_ctrl.stream_info[i].next_transmitter_state)) {
                return;
            }
        }

        g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_SEND_TO_AIR_CH_0] = NULL;
        g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_SEND_TO_AIR_CH_1] = NULL;
        g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_RECV_FROM_AIR_CH_0] = NULL;
        g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_RECV_FROM_AIR_CH_1] = NULL;

#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
        if (APP_LE_AUDIO_MODE_UCST == g_lea_ctrl.curr_mode) {
            app_le_audio_ucst_close_audio_transmitter_cb();
            app_le_audio_ucst_check_active_device_idle();
        }
#endif
#ifdef AIR_LE_AUDIO_BIS_ENABLE
        if (APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.curr_mode) {
            app_le_audio_bcst_close_audio_transmitter_cb();
        }
#endif
    }
}

static void app_le_audio_handle_audio_transmitter_config_cnf(app_le_audio_stream_port_t port)
{
    app_le_audio_stream_info_t *p_stream_info = NULL;

    if (APP_LE_AUDIO_STREAM_PORT_MAX <= port) {
        return;
    }

    p_stream_info = &g_lea_ctrl.stream_info[port];

    LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] handle_config_cnf, port:%x port_state:%x->%x op_queue:%x", 4, port,
                      p_stream_info->curr_transmitter_state,
                      p_stream_info->next_transmitter_state,
                      p_stream_info->config_op_queue);

    p_stream_info->wait_cnf &= (~APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_CONFIG);
    if (APP_LE_AUDIO_TRANSMITTER_STATE_STARTING == p_stream_info->curr_transmitter_state) {
        p_stream_info->curr_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_STARTED;
    }
    switch (p_stream_info->next_transmitter_state) {
        case APP_LE_AUDIO_TRANSMITTER_STATE_STOP:
        case APP_LE_AUDIO_TRANSMITTER_STATE_DEINIT: {
            p_stream_info->config_op_queue = 0;
            if (p_stream_info->is_mixed) {
                p_stream_info->config_op_queue = APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_UNMIX;
                if (app_le_audio_unmix_audio_transmitter_by_port(port, p_stream_info)) {
                    return;
                }
                p_stream_info->config_op_queue = 0;
            }
            if (!(APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_STOP & p_stream_info->wait_cnf)) {
                audio_transmitter_status_t ret = audio_transmitter_stop(p_stream_info->transmitter_id);

                LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] handle_config_cnf stop, port:%x transmitter_id:%x ret:%x", 3, port, p_stream_info->transmitter_id, ret);
                if (AUDIO_TRANSMITTER_STATUS_SUCCESS != ret) {
                    p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_IDLE;
                    return;
                }
                p_stream_info->wait_cnf |= APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_STOP;
            }
            return;
        }
        case APP_LE_AUDIO_TRANSMITTER_STATE_STARTING: {
            return;
        }
        case APP_LE_AUDIO_TRANSMITTER_STATE_SET_VOL: {
            p_stream_info->curr_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_STARTED;
            p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_IDLE;
            if (APP_LE_AUDIO_STREAM_PORT_MIC_0 == port) {
                if (APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_NONE == p_stream_info->config_op_queue) {
                    p_stream_info->config_op_queue = APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_CHANNEL;
                    if (BT_STATUS_SUCCESS != app_le_audio_set_audio_transmitter_channel(APP_LE_AUDIO_STREAM_PORT_MIC_0, p_stream_info)) {
                        p_stream_info->config_op_queue = APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_NONE;
                    }
                } else {
                    p_stream_info->config_op_queue = app_le_audio_set_audio_transmitter_op_queue(p_stream_info->config_op_queue, APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_CHANNEL);
                }
                //app_le_audio_set_audio_transmitter_mic_channel(p_stream_info->mic_channel);
            } else {
                if (app_le_audio_mix_audio_transmitter_by_port(port, p_stream_info)) {
                    LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] mix port: %d success!", 1, port);
                }
            }
            return;
        }
        default:
            break;
    }

    if (!p_stream_info->config_op_queue) {
        return;
    }

    uint8_t op;
    p_stream_info->config_op_queue = (p_stream_info->config_op_queue >> 4);                     /* remove curr op */
    op = (p_stream_info->config_op_queue & APP_LE_AUDIO_TRANSMITTER_CURRENT_CONFIG_OP_MASK);    /* get next op */

    switch (op) {
        case APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_SET_VOL:
        case APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_UNMUTE: {
            app_le_audio_set_audio_transmitter_volume_by_port(port, p_stream_info);
            break;
        }
        case APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_MIX: {
            app_le_audio_mix_audio_transmitter_by_port(port, p_stream_info);
            break;
        }
        case APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_MUTE: {
            app_le_audio_mute_audio_transmitter_by_port(port, p_stream_info);
            break;
        }
        case APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_CHANNEL: {
            app_le_audio_set_audio_transmitter_channel(port, p_stream_info);
            break;
        }
        default:
            break;
    }

}

/**************************************************************************************************
* Public Functions
**************************************************************************************************/

bt_status_t app_le_audio_mute_audio_transmitter(app_le_audio_stream_port_t port)
{
    app_le_audio_stream_info_t *p_stream_info = NULL;

    if (APP_LE_AUDIO_STREAM_PORT_MAX <= port) {
        return BT_STATUS_FAIL;
    }

    p_stream_info = &g_lea_ctrl.stream_info[port];
    LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] mute, port:%x %x", 2, port, p_stream_info->mute);
    p_stream_info->mute = true;

    if ((APP_LE_AUDIO_TRANSMITTER_STATE_STARTED == p_stream_info->curr_transmitter_state) &&
        (APP_LE_AUDIO_TRANSMITTER_STATE_IDLE == p_stream_info->next_transmitter_state)) {

        if (APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_NONE == p_stream_info->config_op_queue &&
            0 == (APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_CONFIG & p_stream_info->wait_cnf)) {
            ble_audio_dongle_runtime_config_operation_t opcode;
            audio_transmitter_runtime_config_t config;
            audio_transmitter_status_t ret;

            p_stream_info->config_op_queue = APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_MUTE;
            if (APP_LE_AUDIO_STREAM_PORT_MIC_0 == port) {
                opcode = BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_VOICE_DUL;
            } else {
                opcode = BLE_AUDIO_DONGLE_CONFIG_OP_VOL_LEVEL_MUSIC_DUL;
            }

            config.ble_audio_dongle_runtime_config.vol_level.vol_level_l = 0;
            config.ble_audio_dongle_runtime_config.vol_level.vol_level_r = 0;
            config.ble_audio_dongle_runtime_config.vol_level.vol_ratio = 100;

            ret = audio_transmitter_set_runtime_config(p_stream_info->transmitter_id,
                                                       opcode,
                                                       &config);
            LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] mute, port:%x ret:%x", 2, port, ret);
            if (BT_STATUS_SUCCESS != ret) {
                p_stream_info->config_op_queue = APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_NONE;
            } else {
                p_stream_info->wait_cnf |= APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_CONFIG;
            }
        } else {
            p_stream_info->config_op_queue = app_le_audio_set_audio_transmitter_op_queue(p_stream_info->config_op_queue, APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_MUTE);
        }
    }

    return BT_STATUS_SUCCESS;
}

bt_status_t app_le_audio_unmute_audio_transmitter(app_le_audio_stream_port_t port)
{
    app_le_audio_stream_info_t *p_stream_info = NULL;

    if (APP_LE_AUDIO_STREAM_PORT_MAX <= port) {
        return BT_STATUS_FAIL;
    }

    p_stream_info = &g_lea_ctrl.stream_info[port];
    LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] unmute, port:%x %x", 2, port, p_stream_info->mute);
    p_stream_info->mute = false;

    if ((APP_LE_AUDIO_TRANSMITTER_STATE_STARTED == p_stream_info->curr_transmitter_state) &&
        (APP_LE_AUDIO_TRANSMITTER_STATE_IDLE == p_stream_info->next_transmitter_state)) {

        if (APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_NONE == p_stream_info->config_op_queue &&
            0 == (APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_CONFIG & p_stream_info->wait_cnf)) {
            p_stream_info->config_op_queue = APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_UNMUTE;
            if (BT_STATUS_SUCCESS != app_le_audio_set_audio_transmitter_volume_by_port(port, p_stream_info)) {
                p_stream_info->config_op_queue = APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_NONE;
            }
        } else {
            p_stream_info->config_op_queue = app_le_audio_set_audio_transmitter_op_queue(p_stream_info->config_op_queue, APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_UNMUTE);
        }
    }

    return BT_STATUS_SUCCESS;
}

bt_status_t app_le_audio_set_audio_transmitter_volume_level(app_le_audio_stream_port_t port, uint8_t vol_level_left, uint8_t vol_level_right)
{
    app_le_audio_stream_info_t *p_stream_info = NULL;

    if (APP_LE_AUDIO_STREAM_PORT_MAX <= port) {
        return BT_STATUS_FAIL;
    }

    if ((vol_level_left > APP_LE_AUDIO_VOL_LEVEL_MAX) || (vol_level_right > APP_LE_AUDIO_VOL_LEVEL_MAX)) {
        return BT_STATUS_FAIL;
    }
    p_stream_info = &g_lea_ctrl.stream_info[port];

    LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] set_vol_level, port:%x mute:%x vol_level:%d %d", 4, port, p_stream_info->mute, vol_level_left, vol_level_right);

    p_stream_info->vol_level_left = vol_level_left;
    p_stream_info->vol_level_right = vol_level_right;

    if ((APP_LE_AUDIO_TRANSMITTER_STATE_STARTED == p_stream_info->curr_transmitter_state) &&
        (APP_LE_AUDIO_TRANSMITTER_STATE_IDLE == p_stream_info->next_transmitter_state) &&
        (false == p_stream_info->mute)) {

        if (APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_NONE == p_stream_info->config_op_queue &&
            0 == (APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_CONFIG & p_stream_info->wait_cnf)) {
            p_stream_info->config_op_queue = APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_SET_VOL;
            if (BT_STATUS_SUCCESS != app_le_audio_set_audio_transmitter_volume_by_port(port, p_stream_info)) {
                p_stream_info->config_op_queue = APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_NONE;
            }
        } else {
            p_stream_info->config_op_queue = app_le_audio_set_audio_transmitter_op_queue(p_stream_info->config_op_queue, APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_SET_VOL);
        }
    }

    return BT_STATUS_SUCCESS;
}

bt_status_t app_le_audio_set_audio_transmitter_mic_channel(uint32_t channel)
{
    app_le_audio_stream_info_t *p_stream_info = NULL;
    p_stream_info = &g_lea_ctrl.stream_info[APP_LE_AUDIO_STREAM_PORT_MIC_0];

    LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] set_mic_channel, %x->%x", 2, p_stream_info->mic_channel, channel);

    if (p_stream_info->mic_channel == channel) {
        return BT_STATUS_SUCCESS;
    }

    p_stream_info->mic_channel = channel;

    if ((APP_LE_AUDIO_TRANSMITTER_STATE_STARTED == p_stream_info->curr_transmitter_state) &&
        (APP_LE_AUDIO_TRANSMITTER_STATE_IDLE == p_stream_info->next_transmitter_state)) {

        if (APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_NONE == p_stream_info->config_op_queue &&
            0 == (APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_CONFIG & p_stream_info->wait_cnf)) {
            p_stream_info->config_op_queue = APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_CHANNEL;
            if (BT_STATUS_SUCCESS != app_le_audio_set_audio_transmitter_channel(APP_LE_AUDIO_STREAM_PORT_MIC_0, p_stream_info)) {
                p_stream_info->config_op_queue = APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_NONE;
            }
        } else {
            p_stream_info->config_op_queue = app_le_audio_set_audio_transmitter_op_queue(p_stream_info->config_op_queue, APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_CHANNEL);
        }
    }

    return BT_STATUS_SUCCESS;
}

bt_status_t app_le_audio_start_audio_transmitter(app_le_audio_stream_port_t port)
{
    app_le_audio_stream_info_t *p_stream_info = NULL;

    if (APP_LE_AUDIO_STREAM_PORT_MAX <= port) {
        return BT_STATUS_FAIL;
    }

    p_stream_info = &g_lea_ctrl.stream_info[port];

    LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] start, port:%x port_state:%x->%x", 3, port,
                      p_stream_info->curr_transmitter_state,
                      p_stream_info->next_transmitter_state);

    switch (p_stream_info->curr_transmitter_state) {
        case APP_LE_AUDIO_TRANSMITTER_STATE_IDLE: {
            return BT_STATUS_FAIL;
        }
        case APP_LE_AUDIO_TRANSMITTER_STATE_INIT: {
            if (APP_LE_AUDIO_TRANSMITTER_STATE_IDLE == p_stream_info->next_transmitter_state) {
                p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_STARTING;

                if (!(APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_START & p_stream_info->wait_cnf)) {
                    audio_transmitter_status_t ret = audio_transmitter_start(p_stream_info->transmitter_id);
                    LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] start, port:%x transmitter_id:%x ret:%x", 3, port, p_stream_info->transmitter_id, ret);
                    if (AUDIO_TRANSMITTER_STATUS_SUCCESS != ret) {
                        p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_IDLE;
                        return BT_STATUS_FAIL;
                    }

                    p_stream_info->wait_cnf |= APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_START;
                }
            } else {
                /* wait handle_start_cnf */
                p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_STARTING;
            }
            break;
        }
        case APP_LE_AUDIO_TRANSMITTER_STATE_STARTING: {
            if ((APP_LE_AUDIO_TRANSMITTER_STATE_STOP == p_stream_info->next_transmitter_state) ||
                (APP_LE_AUDIO_TRANSMITTER_STATE_DEINIT == p_stream_info->next_transmitter_state)) {
                /* wait handle_stop_cnf */
                p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_SET_VOL;
            }
            break;
        }
        case APP_LE_AUDIO_TRANSMITTER_STATE_STARTED: {
            if ((APP_LE_AUDIO_TRANSMITTER_STATE_STOP == p_stream_info->next_transmitter_state) ||
                (APP_LE_AUDIO_TRANSMITTER_STATE_DEINIT == p_stream_info->next_transmitter_state)) {
                if (APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_NONE != p_stream_info->config_op_queue ||
                    p_stream_info->wait_cnf & APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_CONFIG) {
                    p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_IDLE;
                    if (APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_UNMIX == p_stream_info->config_op_queue) {
                        p_stream_info->config_op_queue = app_le_audio_set_audio_transmitter_op_queue(p_stream_info->config_op_queue, APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_MIX);
                    }
                } else {
                    /* wait handle_stop_cnf */
                    p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_STARTING;
                }
            }
            break;
        }
        default:
            break;
    }
    return BT_STATUS_SUCCESS;
}

/* Only stop port and do not deinit port */
bt_status_t app_le_audio_stop_audio_transmitter(app_le_audio_stream_port_t port)
{
    app_le_audio_stream_info_t *p_stream_info = NULL;

    if (APP_LE_AUDIO_STREAM_PORT_MAX <= port) {
        return BT_STATUS_FAIL;
    }
    p_stream_info = &g_lea_ctrl.stream_info[port];

    LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] stop, port:%x port_state:%x->%x", 3, port,
                      p_stream_info->curr_transmitter_state,
                      p_stream_info->next_transmitter_state);

    switch (p_stream_info->curr_transmitter_state) {
        case APP_LE_AUDIO_TRANSMITTER_STATE_IDLE: {
            return BT_STATUS_FAIL;
        }
        case APP_LE_AUDIO_TRANSMITTER_STATE_INIT: {
            if ((APP_LE_AUDIO_TRANSMITTER_STATE_STARTING == p_stream_info->next_transmitter_state) ||
                (APP_LE_AUDIO_TRANSMITTER_STATE_DEINIT == p_stream_info->next_transmitter_state)) {
                /* wait handle_start_cnf */
                p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_STOP;
            }
            break;
        }
        case APP_LE_AUDIO_TRANSMITTER_STATE_STARTING: {
            /* wait handle_config_cnf */
            p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_STOP;
            break;
        }
        case APP_LE_AUDIO_TRANSMITTER_STATE_STARTED: {
            if (APP_LE_AUDIO_TRANSMITTER_STATE_STOP == p_stream_info->next_transmitter_state) {
                break;
            }
            if (APP_LE_AUDIO_TRANSMITTER_STATE_DEINIT == p_stream_info->next_transmitter_state) {
                // p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_STOP;
                break;
            }
            p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_STOP;
            if (APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_NONE != p_stream_info->config_op_queue ||
                p_stream_info->wait_cnf & APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_CONFIG) {
                /* wait handle_config_cnf */
                break;
            }
            if ((APP_LE_AUDIO_STREAM_PORT_MIC_0 != port) && (p_stream_info->is_mixed)) {
                p_stream_info->config_op_queue = APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_UNMIX;
                if (app_le_audio_unmix_audio_transmitter_by_port(port, p_stream_info)) {
                    return BT_STATUS_SUCCESS;
                }
                p_stream_info->config_op_queue = APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_NONE;
            }

            if (!(APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_STOP & p_stream_info->wait_cnf)) {
                audio_transmitter_status_t ret = audio_transmitter_stop(p_stream_info->transmitter_id);
                LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] stop, port:%x transmitter_id:%x ret:%x", 3, port, p_stream_info->transmitter_id, ret);
                if (AUDIO_TRANSMITTER_STATUS_SUCCESS != ret) {
                    p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_IDLE;
                    return BT_STATUS_FAIL;
                }
                p_stream_info->wait_cnf |= APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_STOP;
            }
            break;
        }
    }
    return BT_STATUS_SUCCESS;
}

bt_status_t app_le_audio_stop_and_deinit_audio_transmitter(app_le_audio_stream_port_t port)
{
    app_le_audio_stream_info_t *p_stream_info = NULL;

    if (APP_LE_AUDIO_STREAM_PORT_MAX <= port) {
        return BT_STATUS_FAIL;
    }

    p_stream_info = &g_lea_ctrl.stream_info[port];

    LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] close, port:%x port_state:%x->%x op_queue:%x", 4, port,
                      p_stream_info->curr_transmitter_state,
                      p_stream_info->next_transmitter_state,
                      p_stream_info->config_op_queue);

    switch (p_stream_info->curr_transmitter_state) {
        case APP_LE_AUDIO_TRANSMITTER_STATE_IDLE: {
            return BT_STATUS_SUCCESS;
        }
        case APP_LE_AUDIO_TRANSMITTER_STATE_INIT: {
            if (APP_LE_AUDIO_TRANSMITTER_STATE_IDLE == p_stream_info->next_transmitter_state) {
                app_le_audio_deinit_audio_transmitter(port, p_stream_info);

            } else if ((APP_LE_AUDIO_TRANSMITTER_STATE_STARTING == p_stream_info->next_transmitter_state) ||
                       (APP_LE_AUDIO_TRANSMITTER_STATE_STOP == p_stream_info->next_transmitter_state)) {
                /* wait handle_start_cnf or handle_stop_cnf */
                p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_DEINIT;
            }
            break;
        }
        case APP_LE_AUDIO_TRANSMITTER_STATE_STARTING: {
            /* wait handle_config_cnf (set vol) */
            p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_DEINIT;
            break;
        }
        case APP_LE_AUDIO_TRANSMITTER_STATE_STARTED: {
            if (APP_LE_AUDIO_TRANSMITTER_STATE_DEINIT == p_stream_info->next_transmitter_state) {
                break;
            }
            if (APP_LE_AUDIO_TRANSMITTER_STATE_STOP == p_stream_info->next_transmitter_state) {
                p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_DEINIT;
                break;
            }
            p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_DEINIT;
            if (APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_NONE != p_stream_info->config_op_queue ||
                (p_stream_info->wait_cnf & APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_CONFIG)) {
                /* wait handle_config_cnf */
                /* op_queue of NONE does not mean there is no config on-going. */
                break;
            }
            if ((APP_LE_AUDIO_STREAM_PORT_MIC_0 != port) && (p_stream_info->is_mixed)) {
                p_stream_info->config_op_queue = APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_UNMIX;
                if (app_le_audio_unmix_audio_transmitter_by_port(port, p_stream_info)) {
                    /* wait handle_config_cnf (unmix) */
                    return BT_STATUS_SUCCESS;
                }
                p_stream_info->config_op_queue = APP_LE_AUDIO_TRANSMITTER_CONFIG_OP_NONE;
            }

            if (!(APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_STOP & p_stream_info->wait_cnf)) {
                audio_transmitter_status_t ret = audio_transmitter_stop(p_stream_info->transmitter_id);

                LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] close, stop port:%x transmitter_id:%x ret:%x", 3, port, p_stream_info->transmitter_id, ret);
                if (AUDIO_TRANSMITTER_STATUS_SUCCESS != ret) {
                    p_stream_info->next_transmitter_state = APP_LE_AUDIO_TRANSMITTER_STATE_IDLE;
                    return BT_STATUS_FAIL;
                }

                p_stream_info->wait_cnf |= APP_LE_AUDIO_TRANSMITTER_WAIT_CNF_STOP;
            }
            break;
        }
    }
    return BT_STATUS_SUCCESS;
}

/* Return success as long as at least one port is started successfully. */
bt_status_t app_le_audio_open_audio_transmitter(bool bidirectional, app_le_audio_stream_port_mask_t streaming_port)
{
    bt_status_t ret = BT_STATUS_FAIL, start_ret = BT_STATUS_FAIL;
    LE_AUDIO_MSGLOG_I("[APP][AUDIO_TRANS] open audio transmitter, streaming_port:%x ", 1, streaming_port);
    g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_SEND_TO_AIR_CH_0] = NULL;
    g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_SEND_TO_AIR_CH_1] = NULL;
    g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_RECV_FROM_AIR_CH_0] = NULL;
    g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_RECV_FROM_AIR_CH_1] = NULL;

    /* SPK_0 */
    if (streaming_port & APP_LE_AUDIO_STREAM_PORT_MASK_SPK_0) {
        if (BT_STATUS_SUCCESS == app_le_audio_init_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_SPK_0)) {
            ret = app_le_audio_start_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_SPK_0);
            if (BT_STATUS_SUCCESS != start_ret) {
                start_ret = ret;
            }
        }
    }

    /* SPK_1 */
    if (streaming_port & APP_LE_AUDIO_STREAM_PORT_MASK_SPK_1) {
        if (BT_STATUS_SUCCESS == app_le_audio_init_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_SPK_1)) {
            ret = app_le_audio_start_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_SPK_1);
            if (BT_STATUS_SUCCESS != start_ret) {
                start_ret = ret;
            }
        }
    }

    /* MIC_0 */
    if (bidirectional) {
        if (streaming_port & APP_LE_AUDIO_STREAM_PORT_MASK_MIC_0) {
            if (BT_STATUS_SUCCESS == app_le_audio_init_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_MIC_0)) {
                ret = app_le_audio_start_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_MIC_0);
                if (BT_STATUS_SUCCESS != start_ret) {
                    start_ret = ret;
                }
            }
        }
    }

#ifdef AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
    /* LINE_IN */
    if (streaming_port & APP_LE_AUDIO_STREAM_PORT_MASK_LINE_IN) {
        if (BT_STATUS_SUCCESS == app_le_audio_init_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_LINE_IN)) {
            ret = app_le_audio_start_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_LINE_IN);
            if (BT_STATUS_SUCCESS != start_ret) {
                start_ret = ret;
            }
        }
    }
#endif

#ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
    /* I2S_IN */
    if (streaming_port & APP_LE_AUDIO_STREAM_PORT_MASK_I2S_IN) {
        if (BT_STATUS_SUCCESS == app_le_audio_init_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_I2S_IN)) {
            ret = app_le_audio_start_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_I2S_IN);
            if (BT_STATUS_SUCCESS != start_ret) {
                start_ret = ret;
            }
        }
    }
#endif

    if (BT_STATUS_SUCCESS != start_ret) {
        return start_ret;
    }
    g_lea_ctrl.open_audio_transmitter = true;
    g_lea_ctrl.bidirectional = bidirectional;
    g_lea_ctrl.set_buffer = false;
    return BT_STATUS_SUCCESS;
}

/* Only stop port and deinit port */
bt_status_t app_le_audio_close_audio_transmitter(void)
{
    uint8_t i;

#ifdef AIR_SILENCE_DETECTION_ENABLE
    app_le_audio_silence_detection_stop_by_port(APP_LE_AUDIO_STREAM_PORT_SPK_0);
    app_le_audio_silence_detection_stop_by_port(APP_LE_AUDIO_STREAM_PORT_SPK_1);
#if 0//defined AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE || defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
    app_le_audio_silence_detection_stop_by_port(APP_LE_AUDIO_STREAM_PORT_LINE_IN);
    app_le_audio_silence_detection_stop_by_port(APP_LE_AUDIO_STREAM_PORT_I2S_IN);
#endif

#endif

    /* SPK_0 */
    app_le_audio_stop_and_deinit_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_SPK_0);
    /* SPK_1 */
    app_le_audio_stop_and_deinit_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_SPK_1);
    /* MIC_0 */
    app_le_audio_stop_and_deinit_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_MIC_0);
#ifdef AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
    /* Line-In */
    app_le_audio_stop_and_deinit_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_LINE_IN);
#endif

#ifdef AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
    /* I2S-In */
    app_le_audio_stop_and_deinit_audio_transmitter(APP_LE_AUDIO_STREAM_PORT_I2S_IN);
#endif

    g_lea_ctrl.open_audio_transmitter = false;
    g_lea_ctrl.set_buffer = false;
    g_lea_ctrl.bidirectional = false;

    i = APP_LE_AUDIO_STREAM_PORT_MAX;
    while (0 < i) {
        i--;
        if (APP_LE_AUDIO_TRANSMITTER_STATE_IDLE != g_lea_ctrl.stream_info[i].curr_transmitter_state) {
            return BT_STATUS_SUCCESS;
        }
    }

    g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_SEND_TO_AIR_CH_0] = NULL;
    g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_SEND_TO_AIR_CH_1] = NULL;
    g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_RECV_FROM_AIR_CH_0] = NULL;
    g_lea_share_info[APP_LE_AUDIO_TRANSMITTER_SHARE_INFO_RECV_FROM_AIR_CH_1] = NULL;

#ifdef AIR_LE_AUDIO_UNICAST_ENABLE
    if (APP_LE_AUDIO_MODE_UCST == g_lea_ctrl.curr_mode) {
        app_le_audio_ucst_close_audio_transmitter_cb();
        app_le_audio_ucst_check_active_device_idle();
    }
#endif
#ifdef AIR_LE_AUDIO_BIS_ENABLE
    if (APP_LE_AUDIO_MODE_BCST == g_lea_ctrl.curr_mode) {
        app_le_audio_bcst_close_audio_transmitter_cb();
    }
#endif
    return BT_STATUS_SUCCESS;
}


bool app_le_audio_handle_idle_transmitter_event(ui_shell_activity_t *self, uint32_t event_id, void *extra_data, size_t data_len)
{
    /* Do not return true due to other apps will listen these events. */
    bool ret = false;

    switch (event_id) {
        case APP_LE_AUDIO_EVENT_AUDIO_STREAM_START_SUCCESS_CNF: {
            app_le_audio_handle_audio_transmitter_start_cnf(true, (app_le_audio_stream_port_t)extra_data);
            break;
        }
        case APP_LE_AUDIO_EVENT_AUDIO_STREAM_START_FAIL_CNF: {
            app_le_audio_handle_audio_transmitter_start_cnf(false, (app_le_audio_stream_port_t)extra_data);
            break;
        }
        case APP_LE_AUDIO_EVENT_AUDIO_STREAM_STOP_CNF: {
            app_le_audio_handle_audio_transmitter_stop_cnf((app_le_audio_stream_port_t)extra_data);
            break;
        }
        case APP_LE_AUDIO_EVENT_AUDIO_STREAM_CONFIG_CNF: {
            app_le_audio_handle_audio_transmitter_config_cnf((app_le_audio_stream_port_t)extra_data);
            break;
        }
        default:
            break;
    }

    return ret;
}

uint32_t app_le_audio_get_usb_sample_rate_in_use(app_le_audio_stream_port_t port)
{
    app_le_audio_stream_info_t *p_stream_info = NULL;

    if (APP_LE_AUDIO_STREAM_PORT_MAX <= port) {
        return 0;
    }

    p_stream_info = &g_lea_ctrl.stream_info[port];
    if (APP_LE_AUDIO_TRANSMITTER_STATE_IDLE == p_stream_info->curr_transmitter_state) {
        return 0;
    }

    return p_stream_info->usb_sample_rate;
}


uint8_t app_le_audio_get_usb_sample_size_in_use(app_le_audio_stream_port_t port)
{
    app_le_audio_stream_info_t *p_stream_info = NULL;

    if (APP_LE_AUDIO_STREAM_PORT_MAX <= port) {
        return 0;
    }

    p_stream_info = &g_lea_ctrl.stream_info[port];
    if (APP_LE_AUDIO_TRANSMITTER_STATE_IDLE == p_stream_info->curr_transmitter_state) {
        return 0;
    }

    return p_stream_info->usb_sample_size;
}

uint8_t app_le_audio_get_usb_channel_in_use(app_le_audio_stream_port_t port)
{
    app_le_audio_stream_info_t *p_stream_info = NULL;

    if (APP_LE_AUDIO_STREAM_PORT_MAX <= port) {
        return 0;
    }

    p_stream_info = &g_lea_ctrl.stream_info[port];
    if (APP_LE_AUDIO_TRANSMITTER_STATE_IDLE == p_stream_info->curr_transmitter_state) {
        return 0;
    }

    return p_stream_info->usb_channel;
}

#endif


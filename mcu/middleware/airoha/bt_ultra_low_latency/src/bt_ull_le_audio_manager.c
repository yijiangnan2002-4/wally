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


#include "bt_ull_le_audio_manager.h"
#include "bt_type.h"
#include "bt_ull_utility.h"
#include "bt_ull_le_service.h"
#include "bt_ull_audio_manager.h"
//#include "bt_ull_le_audio_transmitter.h"
#include "audio_src_srv.h"
#include "bt_codec.h"
#include "bt_sink_srv_ami.h"
#include "bt_sink_srv_utils.h"
#include "bt_ull_le_utility.h"
#include "bt_sink_srv_state_notify.h"
#include "bt_timer_external.h"
//free RTOS
#include "task.h"
#include "task_def.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "bt_os_layer_api.h"
/**************************************************************************************************
* Define
**************************************************************************************************/

#define BT_ULL_LE_AM_MODE_INVALID       (-1)
#define BT_ULL_LE_MAX_DEVICE_NUM        (2)

/**
 * @brief Defines the flag of Audio manager for HS
 */
#define BT_ULL_LE_FLAG_WAIT_AM_START                    0x01
#define BT_ULL_LE_FLAG_WAIT_AM_OPEN_CODEC          ((BT_ULL_LE_FLAG_WAIT_AM_START) << 0)
#define BT_ULL_LE_FLAG_WAIT_AM_STOP_CODEC          ((BT_ULL_LE_FLAG_WAIT_AM_START) << 1)
#define BT_ULL_LE_FLAG_WAIT_AM_RESTART_CODEC      ((BT_ULL_LE_FLAG_WAIT_AM_START) << 2)
typedef uint8_t bt_ull_le_am_flag_t;

/**
 * @brief Defines the state of ULL LE audio manager
 */
#define BT_ULL_LE_AM_IDLE                   (0x00)
#define BT_ULL_LE_AM_READY                  (0x01)
#define BT_ULL_LE_AM_PLAYING                (0x02)
typedef uint8_t bt_ull_le_am_state_t;

/**
 * @brief Defines the sub state of ULL LE audio manager
 */
#define BT_ULL_LE_SUB_STATE_NONE                (0x00)
#define BT_ULL_LE_SUB_STATE_PREPARE_AUDIO_SRC    (0x01)
#define BT_ULL_LE_SUB_STATE_PREPARE_CODEC        (0x02)
#define BT_ULL_LE_SUB_STATE_CLEAR_CODEC          (0x03)
typedef uint8_t bt_ull_le_am_sub_state_t;


/**
 * @brief Defines the flag of UL mode on headset or earbuds
 */
#define BT_ULL_LE_AM_UL_FLAG               (0x01)
#define BT_ULL_LE_AM_UL_MIXED              ((BT_ULL_LE_AM_UL_FLAG) << 0)
#define BT_ULL_LE_AM_UL_STOPPING           ((BT_ULL_LE_AM_UL_FLAG) << 1)
#define BT_ULL_LE_AM_UL_STARTING           ((BT_ULL_LE_AM_UL_FLAG) << 2)
#define BT_ULL_LE_AM_UL_RECONFIG           ((BT_ULL_LE_AM_UL_FLAG) << 3)    /* transmitter shuold re-config due to sample rate changed */
typedef uint8_t bt_ull_le_am_ul_flag_t;


/**
 * @brief Defines the am ind event type.
 */
typedef uint8_t bt_ull_le_am_event_ind_t;
#define BT_ULL_LE_AM_DL_PLAY_IND          0x00
#define BT_ULL_LE_AM_DL_STOP_IND          0x01
#define BT_ULL_LE_AM_DL_SUSPEND_IND       0x02
#define BT_ULL_LE_AM_DL_REJECT_IND        0x03
#define BT_ULL_LE_AM_DL_EXCEPTION_IND     0x04
#define BT_ULL_LE_AM_DL_CALLBACK_IND      0x05


#define BT_ULL_LE_AM_UL_DELAY_MUTE_TIMER                100

/**
 * @brief Defines the message queue to switch task.
 */
#define BT_ULL_QUEUE_SIZE 5
#define BT_ULL_QUEUE_ITEM_SIZE sizeof(bt_ull_le_am_message_data_t)
/**************************************************************************************************
* Structure
**************************************************************************************************/

/**
 * @brief This structure defines the parameters of dl on HS side
 */
typedef struct {
    bool                            is_mute;
    bool                            is_start_request;
    bool                            is_playing;
    bool                            is_suspend;
    bool                            is_reject;
    bool                            is_waiting_play;
    bool                            is_add_waiting_list;
    bt_ull_le_am_state_t            am_state;               /**< audio manager state */
    bt_ull_le_am_sub_state_t        am_sub_state;
    bt_ull_le_am_flag_t             am_flag;                /**< audio manager flag */
    bt_sink_srv_am_id_t             audio_id;
    audio_src_srv_handle_t*         am_handle;              /**< Pseudo device handle */
    bt_ull_duel_volume_t            volume;
} bt_ull_le_am_dl_info_t;


/**
 * @brief Defines the parameter of ul on HS side
 */
typedef struct {
    bool                            is_mute;
    bool                            is_suspend;
    bool                            is_transmitter_start;
    bool                            is_request_transmitter_start;
    bool                            is_waiting_play;
    bt_ull_le_am_ul_flag_t          streaming_flag;
    bt_ull_duel_volume_t            volume;
    bt_sink_srv_am_id_t             audio_id;
    uint32_t                        sample_rate;
    audio_src_srv_resource_manager_handle_t *resource_handle;/**< audio source Manager handle> */
    bool                            sidetone_enable;
    
}bt_ull_le_am_ul_info_t;


/**
 * @brief Defines the contex of dl and ul on HS side.
 */
typedef struct {
    bt_ull_le_am_dl_info_t        dl_mode_ctx;
    bt_ull_le_am_ul_info_t        ul_mode_ctx;
} bt_ull_le_srv_am_info_t;

typedef struct {
    bt_ull_le_am_event_ind_t event;
    bt_sink_srv_am_id_t aud_id;
    bt_sink_srv_am_cb_msg_class_t msg_id;
    bt_sink_srv_am_cb_sub_msg_t sub_msg;
    void *parm;
    audio_src_srv_handle_t* handle;
    audio_src_srv_handle_t *int_hd;
} bt_ull_le_am_message_data_t;

/**************************************************************************************************
* Variable
**************************************************************************************************/
bt_ull_le_srv_am_info_t g_bt_ull_le_am_info;
bt_ull_le_am_callback_t g_bt_ull_le_am_callback = NULL;
audio_src_srv_priority_t g_bt_ull_le_am_priority = AUDIO_SRC_SRV_PRIORITY_NORMAL;

QueueHandle_t bt_ull_queue = NULL;
extern uint32_t bt_task_semaphore;

#ifdef AIR_BT_CODEC_BLE_ENABLED
extern void bt_ble_set_lightmode(bool is_lightmode);
#endif

/**************************************************************************************************
* Prototype
**************************************************************************************************/
//init
static void bt_ull_le_am_dl_init(void);
static void bt_ull_le_am_ul_init(void);
//state, only used by DL
static void bt_ull_le_am_dl_set_state(bt_ull_le_am_dl_info_t *ull_am_info, bt_ull_le_am_state_t state);
static bt_ull_le_am_state_t bt_ull_le_am_dl_get_state(bt_ull_le_am_dl_info_t *ull_am_info);
static void bt_ull_le_am_dl_set_sub_state(bt_ull_le_am_dl_info_t *ull_am_info, bt_ull_le_am_sub_state_t substate);
static bt_ull_le_am_state_t bt_ull_le_am_dl_get_sub_state(bt_ull_le_am_dl_info_t *ull_am_info);

//get mode, only used by DL
static bt_ull_le_am_mode_t bt_ull_le_am_get_mode_by_audio_handle(audio_src_srv_handle_t *p_handle);
static bt_ull_le_am_mode_t bt_ull_le_am_get_mode_by_audio_id(bt_sink_srv_am_id_t aud_id);

//add or remove waiting list, only used by DL
static void bt_ull_le_am_add_waiting_list(bt_ull_le_am_mode_t mode);
static void bt_ull_le_am_remove_waiting_list(bt_ull_le_am_mode_t mode);

//flag, only used by DL
static void bt_ull_le_set_am_dl_flag(bt_ull_le_am_flag_t flag);
static void bt_ull_le_remove_am_dl_flag(bt_ull_le_am_flag_t flag);
//flag, only used by UL
static void bt_ull_le_set_am_ul_flag(bt_ull_le_am_ul_flag_t flag);
static void bt_ull_le_remove_am_ul_flag(bt_ull_le_am_ul_flag_t flag);


//get contex
static bt_ull_le_srv_am_info_t* bt_ull_le_get_audio_manager_contex(void);

//event callback
static void bt_ull_le_am_event_callback(bt_ull_le_am_event_t event, void *param, uint32_t param_len);
//Callback related, only used by DL
static void bt_ull_le_am_dl_callback_ind(bt_sink_srv_am_id_t aud_id, bt_sink_srv_am_cb_msg_class_t msg_id, bt_sink_srv_am_cb_sub_msg_t sub_msg, void *param);
static void bt_ull_le_am_dl_play_ind(audio_src_srv_handle_t *handle);
static void bt_ull_le_am_dl_stop_ind(audio_src_srv_handle_t *handle);
static void bt_ull_le_am_dl_reject_ind(audio_src_srv_handle_t *handle);
static void bt_ull_le_am_dl_suspend_ind(audio_src_srv_handle_t *handle, audio_src_srv_handle_t *int_hd);
static void bt_ull_le_am_dl_exception_ind(audio_src_srv_handle_t *handle, int32_t event, void *param);
static void bt_ull_le_am_dl_callback_ind_handle_message(bt_sink_srv_am_id_t aud_id, bt_sink_srv_am_cb_msg_class_t msg_id, bt_sink_srv_am_cb_sub_msg_t sub_msg, void *param);
static void bt_ull_le_am_dl_play_ind_handle_message(audio_src_srv_handle_t *handle);
static void bt_ull_le_am_dl_stop_ind_handle_message(audio_src_srv_handle_t *handle);
static void bt_ull_le_am_dl_reject_ind_handle_message(audio_src_srv_handle_t *handle);
static void bt_ull_le_am_dl_suspend_ind_handle_message(audio_src_srv_handle_t *handle, audio_src_srv_handle_t *int_hd);
static void bt_ull_le_am_dl_exception_ind_handle_message(audio_src_srv_handle_t *handle);

void bt_ull_le_am_send_message(bt_ull_le_am_message_data_t *message);
void bt_ull_le_am_handle_receive_message(bt_ull_le_am_message_data_t *message);
void bt_ull_le_am_queue_handler(void);

static bt_sink_srv_am_volume_level_out_t bt_ull_le_am_get_volume_level(uint8_t volume);

//switch param
static le_audio_interval_t bt_ull_le_am_switch_sdu_interval(uint32_t sdu_interval);

//API for UL
static void bt_ull_le_am_ul_start_audio(bt_ull_le_am_ul_info_t* ull_am_info);
static bt_status_t bt_ull_le_am_ul_play(bt_sink_srv_am_id_t aud_id, bt_ull_le_codec_t codec_type);
static bt_status_t bt_ull_le_am_ul_stop_audio(bt_ull_le_am_ul_info_t* ull_am_info);
static void bt_ull_le_am_mic_resource_callback(audio_src_srv_resource_manager_handle_t *current_handle, audio_src_srv_resource_manager_event_t event);
static void bt_ull_le_am_ul_callback(bt_sink_srv_am_id_t aud_id, bt_sink_srv_am_cb_msg_class_t msg_id, bt_sink_srv_am_cb_sub_msg_t sub_msg, void *param);
static void bt_ull_le_am_ul_handle_start_success_cnf(bt_ull_le_am_ul_info_t* ull_am_info);
static void bt_ull_le_am_ul_handle_stop_success_or_start_fail_cnf(bt_ull_le_am_ul_info_t* ull_am_info);
static void bt_ull_le_am_mute_timeout_callbak(uint32_t timer_id, uint32_t data);

//internal play and stop
//static bt_status_t bt_ull_le_am_play_internal(bt_ull_le_am_mode_t mode, bt_ull_le_codec_t codec);
//static bt_status_t bt_ull_le_am_stop_internal(bt_ull_le_am_mode_t mode);

//extern function declair
extern bt_sink_srv_am_id_t ami_audio_open(bt_sink_srv_am_type_t pseudo_device_type,
                                          bt_sink_srv_am_notify_callback handler);
extern bt_sink_srv_am_result_t ami_audio_play(bt_sink_srv_am_id_t aud_id, bt_sink_srv_am_audio_capability_t *capability_t);
extern bt_sink_srv_am_result_t ami_audio_stop(bt_sink_srv_am_id_t aud_id);


/**************************************************************************************************
* Functions - Internal
**************************************************************************************************/

static void bt_ull_le_am_dl_init(void)
{
    bt_ull_le_srv_am_info_t *am_ctx = bt_ull_le_get_audio_manager_contex();
    bt_ull_le_am_dl_info_t *ull_dl_am_info = &(am_ctx->dl_mode_ctx); 
    /*para init*/
    ull_dl_am_info->audio_id = AUD_ID_INVALID;
    ull_dl_am_info->am_handle = audio_src_srv_construct_handle(AUDIO_SRC_SRV_PSEUDO_DEVICE_ULL_BLE);
    ull_dl_am_info->am_handle->type = AUDIO_SRC_SRV_PSEUDO_DEVICE_ULL_BLE;
    ull_dl_am_info->am_handle->dev_id = 0x0;
    ull_dl_am_info->am_handle->priority = g_bt_ull_le_am_priority;
    /*function init*/
    ull_dl_am_info->am_handle->play = bt_ull_le_am_dl_play_ind;
    ull_dl_am_info->am_handle->stop = bt_ull_le_am_dl_stop_ind;
    ull_dl_am_info->am_handle->suspend = bt_ull_le_am_dl_suspend_ind;
    ull_dl_am_info->am_handle->reject = bt_ull_le_am_dl_reject_ind;
    ull_dl_am_info->am_handle->exception_handle = bt_ull_le_am_dl_exception_ind;
    /*register callback*/
    //ull_dl_am_info->audio_id = bt_sink_srv_ami_audio_open(AUD_MIDDLE, bt_ull_le_am_dl_callback);
    if (AUD_ID_INVALID ==(ull_dl_am_info->audio_id = bt_sink_srv_ami_audio_open(AUD_MIDDLE, bt_ull_le_am_dl_callback_ind))) {
        //ull_report("[ULL][LE][AM]dl init, get audio id fail!!!!", 0);
        ull_assert(0 && "[ULL][LE][AM] fail to get DL audio id!!!");
        return;
    }
    audio_src_srv_update_state(ull_dl_am_info->am_handle, AUDIO_SRC_SRV_EVT_READY);
    bt_ull_le_am_dl_set_state(ull_dl_am_info, BT_ULL_LE_AM_READY);
    ull_report("[ULL][LE][AM] dl init, get audio id :%d", 1, ull_dl_am_info->audio_id);
}

static void bt_ull_le_am_ul_init(void)
{
    bt_sink_srv_am_id_t aud_id;
    bt_ull_le_srv_am_info_t *am_ctx = bt_ull_le_get_audio_manager_contex();
    bt_ull_le_am_ul_info_t *ull_ul_am_info = &(am_ctx->ul_mode_ctx);
    
    ull_ul_am_info->resource_handle = audio_src_srv_resource_manager_construct_handle(AUDIO_SRC_SRV_RESOURCE_TYPE_MIC,AUDIO_SRC_SRV_RESOURCE_TYPE_MIC_USER_ULL_UL);
    if (NULL != ull_ul_am_info->resource_handle) {
        ull_ul_am_info->resource_handle->state = AUDIO_SRC_SRV_EVENT_NONE;
        ull_ul_am_info->resource_handle->callback_func = bt_ull_le_am_mic_resource_callback;
        ull_ul_am_info->resource_handle->priority = AUDIO_SRC_SRV_RESOURCE_TYPE_MIC_USER_ULL_UL_PRIORITY;
        ull_report("[ULL][LE][AM] mic_resource_handle client resource_handle: 0x%04x, priority:0x%d ", 2, ull_ul_am_info->resource_handle,ull_ul_am_info->resource_handle->priority);
    } else {
        ull_assert(0 && "[ULL][LE][AM] fail to get mic_resource handle!!!");
    }
    
    /*get mic audio id*/
    if (AUD_ID_INVALID ==(aud_id = ami_audio_open(ULL_BLE_UL, bt_ull_le_am_ul_callback))) {
        //ull_report("[ULL][LE][AM]ul init, get audio id fail!!!!", 0);
        ull_assert(0 && "[ULL][LE][AM] fail to get UL audio id!!!");
        return;
    }
    ull_report("[ULL][LE][AM] ul init, get audio id :%d", 1, aud_id);
    ull_ul_am_info->audio_id = aud_id;
}

static void bt_ull_le_am_dl_set_state(bt_ull_le_am_dl_info_t *ull_am_info, bt_ull_le_am_state_t state)
{
    ull_report("[ULL][LE][AM] set state, state change: %d -> %d", 2,  ull_am_info->am_state, state);
    ull_am_info->am_state = state;
}

static bt_ull_le_am_state_t bt_ull_le_am_dl_get_state(bt_ull_le_am_dl_info_t *ull_am_info)
{
    return ull_am_info->am_state;
}

static void bt_ull_le_am_dl_set_sub_state(bt_ull_le_am_dl_info_t *ull_am_info, bt_ull_le_am_sub_state_t substate)
{
    ull_report("[ULL][LE][AM] set substate, substate change: %d -> %d", 2,  ull_am_info->am_sub_state, substate);
    ull_am_info->am_sub_state = substate;
}

static bt_ull_le_am_sub_state_t bt_ull_le_am_dl_get_sub_state(bt_ull_le_am_dl_info_t *ull_am_info)
{
    return ull_am_info->am_sub_state;
}

static bt_ull_le_am_mode_t bt_ull_le_am_get_mode_by_audio_handle(audio_src_srv_handle_t *p_handle)
{
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    if (p_handle == am_ctx->dl_mode_ctx.am_handle) {
        return BT_ULL_LE_AM_DL_MODE;
    }
    return BT_ULL_LE_AM_MODE_INVALID;
}

static bt_ull_le_am_mode_t bt_ull_le_am_get_mode_by_audio_id(bt_sink_srv_am_id_t aud_id)
{
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    if (aud_id == am_ctx->dl_mode_ctx.audio_id) {
        return BT_ULL_LE_AM_DL_MODE;
    }
    return BT_ULL_LE_AM_MODE_INVALID;   
}

static void bt_ull_le_am_add_waiting_list(bt_ull_le_am_mode_t mode)
{
    ull_report("[ULL][LE][AM] add waiting list, mode: %d", 1, mode);
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    am_ctx->dl_mode_ctx.is_add_waiting_list = true;
    audio_src_srv_add_waiting_list(am_ctx->dl_mode_ctx.am_handle);
}

static void bt_ull_le_am_remove_waiting_list(bt_ull_le_am_mode_t mode)
{
    ull_report("[ULL][LE][AM] remove waiting list, mode: %d", 1, mode);
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    am_ctx->dl_mode_ctx.is_add_waiting_list = false;
    audio_src_srv_del_waiting_list(am_ctx->dl_mode_ctx.am_handle);
}

static void bt_ull_le_set_am_dl_flag(bt_ull_le_am_flag_t flag)
{
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    am_ctx->dl_mode_ctx.am_flag |= flag;
    ull_report("[ULL][LE][AM] set am_dl_flag : 0x%x, current am_dl_flag: 0x%x", 2, flag, am_ctx->dl_mode_ctx.am_flag);
}

static void bt_ull_le_remove_am_dl_flag(bt_ull_le_am_flag_t flag)
{
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    am_ctx->dl_mode_ctx.am_flag &= ~flag;
    ull_report("[ULL][LE][AM] remove am_dl_flag : 0x%x, current am_dl_flag: 0x%x", 2, flag, am_ctx->dl_mode_ctx.am_flag);
}

static void bt_ull_le_set_am_ul_flag(bt_ull_le_am_ul_flag_t flag)
{
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    am_ctx->ul_mode_ctx.streaming_flag |= flag;
    ull_report("[ULL][LE][AM] set am_ul_flag : 0x%x, current am_ul_flag: 0x%x", 2, flag, am_ctx->ul_mode_ctx.streaming_flag);
}

static void bt_ull_le_remove_am_ul_flag(bt_ull_le_am_ul_flag_t flag)
{
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    am_ctx->ul_mode_ctx.streaming_flag &= ~flag;
    ull_report("[ULL][LE][AM] remove am_ul_flag : 0x%x, current am_ul_flag: 0x%x", 2, flag, am_ctx->ul_mode_ctx.streaming_flag);
}

static bt_ull_le_srv_am_info_t* bt_ull_le_get_audio_manager_contex(void)
{
    return &g_bt_ull_le_am_info;
}

static void bt_ull_le_am_event_callback(bt_ull_le_am_event_t event, void *param, uint32_t param_len)
{
    //ull_report("[ULL][LE][AM] bt_ull_le_am_event_callback. event:0x%x, param len:0x%x", 2, event, param_len);
    if (g_bt_ull_le_am_callback) {
        g_bt_ull_le_am_callback(event, param, param_len);
    }
}
static void bt_ull_le_am_dl_play_ind(audio_src_srv_handle_t *handle)
{
    bt_ull_le_am_message_data_t message = {0};
    bt_ull_le_srv_memset(&message, 0, sizeof(bt_ull_le_am_message_data_t));
    message.event = BT_ULL_LE_AM_DL_PLAY_IND;
    message.handle = handle;
    bt_ull_le_am_send_message(&message);
}

static void bt_ull_le_am_dl_stop_ind(audio_src_srv_handle_t *handle)
{
    bt_ull_le_am_message_data_t message = {0};
    bt_ull_le_srv_memset(&message, 0, sizeof(bt_ull_le_am_message_data_t));
    message.event = BT_ULL_LE_AM_DL_STOP_IND;
    message.handle = handle;
    bt_ull_le_am_send_message(&message);
}

static void bt_ull_le_am_dl_suspend_ind(audio_src_srv_handle_t *handle, audio_src_srv_handle_t *int_hd)
{
    bt_ull_le_am_message_data_t message = {0};
    bt_ull_le_srv_memset(&message, 0, sizeof(bt_ull_le_am_message_data_t));
    message.event = BT_ULL_LE_AM_DL_SUSPEND_IND;
    message.handle = handle;
    message.int_hd = int_hd;
    bt_ull_le_am_send_message(&message);
}

static void bt_ull_le_am_dl_reject_ind(audio_src_srv_handle_t *handle)
{
    bt_ull_le_am_message_data_t message = {0};
    bt_ull_le_srv_memset(&message, 0, sizeof(bt_ull_le_am_message_data_t));
    message.event = BT_ULL_LE_AM_DL_REJECT_IND;
    message.handle = handle;
    bt_ull_le_am_send_message(&message);
}

static void bt_ull_le_am_dl_exception_ind(audio_src_srv_handle_t *handle ,int32_t event, void *param)
{
    bt_ull_le_am_message_data_t message = {0};
    bt_ull_le_srv_memset(&message, 0, sizeof(bt_ull_le_am_message_data_t));
    message.event = BT_ULL_LE_AM_DL_EXCEPTION_IND;
    message.handle = handle;
    bt_ull_le_am_send_message(&message);
}

static void bt_ull_le_am_dl_callback_ind(bt_sink_srv_am_id_t aud_id, bt_sink_srv_am_cb_msg_class_t msg_id, bt_sink_srv_am_cb_sub_msg_t sub_msg, void *param)
{
    bt_ull_le_am_message_data_t message = {0};
    message.event = BT_ULL_LE_AM_DL_CALLBACK_IND;
    message.aud_id = aud_id;
    message.msg_id = msg_id;
    message.sub_msg = sub_msg;
    message.parm  = NULL;
    message.handle = NULL;
    bt_ull_le_am_send_message(&message);
}

static void bt_ull_le_am_dl_callback_ind_handle_message(bt_sink_srv_am_id_t aud_id, bt_sink_srv_am_cb_msg_class_t msg_id, bt_sink_srv_am_cb_sub_msg_t sub_msg, void *param)
{
    bt_ull_le_am_mode_t mode = bt_ull_le_am_get_mode_by_audio_id(aud_id);
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    bt_ull_le_am_dl_info_t *ull_am_info = &(am_ctx->dl_mode_ctx);
    bt_ull_le_am_state_t state = bt_ull_le_am_dl_get_state(ull_am_info);
    bt_ull_le_am_sub_state_t substate = bt_ull_le_am_dl_get_sub_state(ull_am_info);
    ull_report("[ULL][LE][AM] am callback, mode:%d, state:%d, substate:%d, flag:0x%x, aud_id:%d, msg_id:%d, sub_msg:%d, ull_am_info->audio_id:%d, is_start_request:%d", 9, 
        mode, state, substate, ull_am_info->am_flag, aud_id, msg_id, sub_msg, ull_am_info->audio_id, ull_am_info->is_start_request);
    
    BT_ULL_MUTEX_LOCK();

    if (ull_am_info->audio_id == aud_id) {
        switch (msg_id) {
            case AUD_SINK_OPEN_CODEC: {
                if(AUD_HFP_OPEN_CODEC_DONE == sub_msg) {
                    ull_report("[ULL][LE][AM] dl open codec success!! ", 0);
                    /*notify service play success*/
                    bt_ull_le_am_dl_set_state(ull_am_info, BT_ULL_LE_AM_PLAYING);
                    if (!(ull_am_info->am_flag & BT_ULL_LE_FLAG_WAIT_AM_RESTART_CODEC)) {
                        bt_ull_le_am_result_t result_notify;
                        result_notify.mode = BT_ULL_LE_AM_DL_MODE;
                        result_notify.result = BT_STATUS_SUCCESS;
                        bt_ull_le_am_event_callback(BT_ULL_LE_AM_PLAY_IND, &result_notify, sizeof(result_notify));
                    } else {
                        bt_ull_le_remove_am_dl_flag(BT_ULL_LE_FLAG_WAIT_AM_RESTART_CODEC);
                    }

                    if ((BT_ULL_LE_SUB_STATE_PREPARE_CODEC == substate) || (BT_ULL_LE_SUB_STATE_NONE == substate)) {
                        if (ull_am_info->am_flag & BT_ULL_LE_FLAG_WAIT_AM_OPEN_CODEC) {
                            /*1.clear flag and set state*/
                            bt_ull_le_remove_am_dl_flag(BT_ULL_LE_FLAG_WAIT_AM_OPEN_CODEC);
                            bt_ull_le_am_dl_set_sub_state(ull_am_info, BT_ULL_LE_SUB_STATE_NONE);
                            /*3.save codec handle*/
                            //memcpy(&(ull_am_info->med_handle), param, sizeof(bt_sink_srv_am_media_handle_t));
                            /*4.update status*/
                            audio_src_srv_update_state(ull_am_info->am_handle, AUDIO_SRC_SRV_EVT_PLAYING);
                            //bt_ull_le_am_dl_set_state(ull_am_info, BT_ULL_LE_AM_PLAYING);
                            bt_sink_srv_state_set(BT_SINK_SRV_STATE_STREAMING);
                            ull_am_info->is_playing = true;
                            /*4.set codec volume*/
                            //bt_sink_srv_ami_audio_set_volume(ull_am_info->audio_id, AUD_VOL_OUT_LEVEL15, STREAM_OUT);
                            if (ull_am_info->is_mute) {
                                bt_sink_srv_ami_audio_set_volume(ull_am_info->audio_id, 0, STREAM_OUT);
                            } else {
                                bt_sink_srv_ami_audio_set_volume(ull_am_info->audio_id, AUD_VOL_OUT_LEVEL15, STREAM_OUT);
                            }
                            #if 0
                            if ((0 == ull_am_info->volume.vol_left) && (0 != ull_am_info->volume.vol_right)) {
                                bt_sink_srv_ami_audio_set_volume(ull_am_info->audio_id, ull_am_info->volume.vol_right, STREAM_OUT);
                            } else if ((0 != ull_am_info->volume.vol_left) && (0 == ull_am_info->volume.vol_right)) {
                                bt_sink_srv_ami_audio_set_volume(ull_am_info->audio_id, ull_am_info->volume.vol_left, STREAM_OUT);
                            } else if ((0 != ull_am_info->volume.vol_left) && (0 != ull_am_info->volume.vol_right)) {
                                bt_sink_srv_ami_audio_set_volume(ull_am_info->audio_id, ull_am_info->volume.vol_left, STREAM_OUT);
                            }
                            #endif
                        }
                  } else if (BT_ULL_LE_SUB_STATE_CLEAR_CODEC == substate){
                        bt_ull_le_remove_am_dl_flag(BT_ULL_LE_FLAG_WAIT_AM_OPEN_CODEC);
                        if (bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK)) {
                            //in this corner case, if check uplink port is in streaming, not close downlink.
                            ull_report_error("[ULL][LE][AM] uplink port is in streaming, not close downlink", 0);
                            bt_ull_le_am_dl_set_sub_state(ull_am_info, BT_ULL_LE_SUB_STATE_NONE);
                        } else {
                        /*release audio source*/
                        audio_src_srv_update_state(ull_am_info->am_handle, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
                  }

                  }
                } else if (AUD_CMD_FAILURE == sub_msg) {
                      ull_report_error("[ULL][LE][AM] dl open codec fail", 0);
					  bt_ull_le_remove_am_dl_flag(BT_ULL_LE_FLAG_WAIT_AM_OPEN_CODEC);
                      /*notify service play fail*/
                      bt_ull_le_am_result_t result_notify;
                      result_notify.mode = BT_ULL_LE_AM_DL_MODE;
                      result_notify.result = BT_STATUS_FAIL;
                      bt_ull_le_am_event_callback(BT_ULL_LE_AM_PLAY_IND, &result_notify, sizeof(result_notify));
                       //ull_report("[ULL][LE][AM] remove dl am flag, flag: 0x%x, current dl am flag: 0x%x.", 2, BT_ULL_LE_FLAG_WAIT_AM_OPEN_CODEC, ull_am_info->am_flag);
                      /*release audio source*/
                      audio_src_srv_update_state(ull_am_info->am_handle, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
                } 
                break;
            }
            //DSP off
            case AUD_SELF_CMD_REQ: {
                if(AUD_CMD_COMPLETE == sub_msg){
                    ull_report("[ULL][LE][AM] dl close codec success!! ", 0);
                    audio_src_srv_update_state(ull_am_info->am_handle, AUDIO_SRC_SRV_EVT_READY);
                    if (am_ctx->dl_mode_ctx.is_suspend) {
                        bt_ull_le_am_dl_info_t *ull_am_info = &(am_ctx->dl_mode_ctx);
                        if (ull_am_info->is_start_request || bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) {
                            bt_ull_le_am_add_waiting_list(mode);
                        }
                    }					
                    if (!(ull_am_info->am_flag & BT_ULL_LE_FLAG_WAIT_AM_RESTART_CODEC)) {
                        bt_ull_le_am_result_t result_notify;
                        result_notify.mode = mode;
                        result_notify.result = BT_STATUS_SUCCESS;
                        bt_ull_le_am_event_callback(BT_ULL_LE_AM_STOP_IND, &result_notify, sizeof(result_notify));
                    }

                    if (BT_ULL_LE_SUB_STATE_CLEAR_CODEC == substate) {
                        audio_src_srv_update_state(ull_am_info->am_handle, AUDIO_SRC_SRV_EVT_READY);
                        bt_ull_le_am_dl_set_state(ull_am_info, BT_ULL_LE_AM_READY);
                        bt_ull_le_am_dl_set_sub_state(ull_am_info, BT_ULL_LE_SUB_STATE_NONE);
                        ull_am_info->is_playing = false;
                        bt_sink_srv_state_reset(BT_SINK_SRV_STATE_STREAMING);
                    }
                    if (ull_am_info->am_flag & BT_ULL_LE_FLAG_WAIT_AM_STOP_CODEC) {
                        /*check need deinit pseudo device*/
                        bt_ull_le_remove_am_dl_flag(BT_ULL_LE_FLAG_WAIT_AM_STOP_CODEC);
                        if(ull_am_info->am_handle) {
                            audio_src_srv_destruct_handle(ull_am_info->am_handle);
                            ull_am_info->am_handle = NULL;
                            bt_ull_le_am_dl_set_state(ull_am_info, BT_ULL_LE_AM_IDLE);
                        }
                    } else if (ull_am_info->am_flag & BT_ULL_LE_FLAG_WAIT_AM_RESTART_CODEC) {
                        /*check need restart codec*/
                        //bt_ull_le_remove_am_dl_flag(BT_ULL_LE_FLAG_WAIT_AM_RESTART_CODEC);
                        if(ull_am_info->is_start_request) {
                            bt_ull_le_am_play(mode, bt_ull_le_srv_get_codec_type(), false);
                        }
                    } else if (ull_am_info->is_start_request && !ull_am_info->is_suspend) {
                        bt_ull_le_am_play(mode, bt_ull_le_srv_get_codec_type(), false);
                    }
                } 
                break;
            }

            default:
                break;
        }
    }
    BT_ULL_MUTEX_UNLOCK();
}

static void bt_ull_le_am_dl_play_ind_handle_message(audio_src_srv_handle_t *handle)
{
    bt_ull_le_am_mode_t mode = bt_ull_le_am_get_mode_by_audio_handle(handle);
    bt_sink_srv_am_result_t am_ret;
    bt_sink_srv_am_audio_capability_t aud_cap = {0};
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    bt_ull_client_t client_type = bt_ull_le_srv_get_client_type();
 //   ull_report("[ULL][LE][AM] dl play ind, get client_type: %d.", 1, client_type);
    BT_ULL_MUTEX_LOCK();
    if(BT_ULL_LE_AM_DL_MODE == mode){
        bt_ull_le_am_dl_info_t *ull_am_info = &(am_ctx->dl_mode_ctx);
        bt_ull_le_am_state_t state = bt_ull_le_am_dl_get_state(ull_am_info);
        bt_ull_le_am_sub_state_t substate = bt_ull_le_am_dl_get_sub_state(ull_am_info);
    ull_report("[ULL][LE][AM] dl play ind, state:%d, substate:%d, is_streaming:%d, is supd: %d, suspend: %d, waiting_play: %d, is_start_request: %d", 7, \
            state, substate, ull_am_info->is_playing, am_ctx->dl_mode_ctx.am_sub_state, am_ctx->dl_mode_ctx.is_suspend, am_ctx->dl_mode_ctx.is_waiting_play, am_ctx->dl_mode_ctx.is_start_request);
        ull_am_info->is_add_waiting_list = false;
        if (am_ctx->dl_mode_ctx.is_suspend) {
            am_ctx->dl_mode_ctx.is_suspend = false;
            //bt_ull_le_srv_active_streaming(); /*set iso data path*/
            bt_ull_le_srv_notify_server_play_is_allow(BT_ULL_PLAY_ALLOW, BT_ULL_ALLOW_PLAY_REASON_AUDIO);
        }
        if (am_ctx->dl_mode_ctx.is_reject) {
            am_ctx->dl_mode_ctx.is_reject= false;
        }
        if (am_ctx->dl_mode_ctx.is_waiting_play) {
            am_ctx->dl_mode_ctx.is_waiting_play = false;
            //bt_ull_le_srv_active_streaming(); /*remove iso data path*/
            bt_ull_le_srv_notify_server_play_is_allow(BT_ULL_PLAY_ALLOW, BT_ULL_ALLOW_PLAY_REASON_AUDIO);
        }
        if (ull_am_info->am_handle && (ull_am_info->am_handle == handle)) {
            if ((BT_ULL_LE_SUB_STATE_NONE == substate) || (BT_ULL_LE_SUB_STATE_PREPARE_AUDIO_SRC == substate)) {
                bt_ull_le_am_dl_set_sub_state(ull_am_info, BT_ULL_LE_SUB_STATE_PREPARE_CODEC);//update status

                /*1.init am codec parameter*/
              //ull_report("[ULL][LE][AM] dl play ind, init am codec parameter and audio play", 0);
                memset(&aud_cap, 0x00, sizeof(bt_sink_srv_am_audio_capability_t));
                aud_cap.type = ULL_BLE_DL;
                if(BT_ULL_LE_CODEC_LC3PLUS == bt_ull_le_srv_get_codec_type()){
                    aud_cap.codec.ull_ble_dl_format.ble_codec.codec = BT_CODEC_TYPE_LE_AUDIO_LC3PULS;
                } else if (BT_ULL_LE_CODEC_OPUS == bt_ull_le_srv_get_codec_type()) {
                    aud_cap.codec.ull_ble_dl_format.ble_codec.codec = BT_CODEC_TYPE_LE_AUDIO_VENDOR;
                } else if (BT_ULL_LE_CODEC_DL_ULD_UL_LC3PLUS == bt_ull_le_srv_get_codec_type()) {
                    aud_cap.codec.ull_ble_dl_format.ble_codec.codec = BT_CODEC_TYPE_LE_AUDIO_ULD;
                }
                ull_report("[ULL][LE][AM] dl play ind, select codec_type: %d", 1, aud_cap.codec.ull_ble_dl_format.ble_codec.codec);
                aud_cap.codec.ull_ble_dl_format.ble_codec.sample_rate = bt_ull_le_srv_get_codec_sample_rate(0, false, BT_ULL_ROLE_CLIENT);
                if(BT_ULL_HEADSET_CLIENT == client_type || BT_ULL_SPEAKER_CLIENT == client_type) {
                    aud_cap.codec.ull_ble_dl_format.ble_codec.channel_num = BT_ULL_LE_CHANNEL_MODE_STEREO;
                } else if (BT_ULL_EARBUDS_CLIENT == client_type) {
                    aud_cap.codec.ull_ble_dl_format.ble_codec.channel_num = BT_ULL_LE_CHANNEL_MODE_MONO;
                } else {
                    ull_report_error("[ULL][LE][AM] client type is neither headset nor erabuds!!", 0);
                    aud_cap.codec.ull_ble_dl_format.ble_codec.channel_num = BT_ULL_LE_CHANNEL_MODE_STEREO;
                }
                uint16_t frame_size = bt_ull_le_srv_get_sdu_size(false, BT_ULL_ROLE_CLIENT);
                aud_cap.codec.ull_ble_dl_format.ble_codec.maxframe_payload_length = frame_size/2;
                aud_cap.codec.ull_ble_dl_format.ble_codec.context_type = 0;
                uint32_t sdu_interval = bt_ull_le_srv_get_sdu_interval(false, BT_ULL_ROLE_CLIENT);
                aud_cap.codec.ull_ble_dl_format.ble_codec.frame_duration = bt_ull_le_am_switch_sdu_interval(sdu_interval);
                aud_cap.audio_stream_out.audio_device = HAL_AUDIO_DEVICE_HEADSET;
                aud_cap.audio_stream_out.audio_volume = AUD_VOL_OUT_LEVEL15;
                aud_cap.audio_stream_out.audio_mute = false;
                aud_cap.audio_stream_in.audio_device = HAL_AUDIO_DEVICE_MAIN_MIC;
                aud_cap.audio_stream_in.audio_volume = AUD_VOL_OUT_LEVEL0;

                ull_report("[ULL][LE][AM] dl play ind, set codec param, sample_rate: %d, channel_num: %d, maxframe_payload_length: %d, context_type: %d, frame_duration: %d.", 5,
                    aud_cap.codec.ull_ble_dl_format.ble_codec.sample_rate,
                    aud_cap.codec.ull_ble_dl_format.ble_codec.channel_num,
                    aud_cap.codec.ull_ble_dl_format.ble_codec.maxframe_payload_length,
                    aud_cap.codec.ull_ble_dl_format.ble_codec.context_type,
                    aud_cap.codec.ull_ble_dl_format.ble_codec.frame_duration);

                /*2.update status*/
                bt_ull_le_set_am_dl_flag(BT_ULL_LE_FLAG_WAIT_AM_OPEN_CODEC);
                /*3.audio play*/
                am_ret = bt_sink_srv_ami_audio_play(ull_am_info->audio_id, &aud_cap);

                if (AUD_EXECUTION_SUCCESS != am_ret) {
                    ull_report_error("[ULL][LE][AM] dl play ind, bt_sink_srv_ami_audio_play fail: 0x%x, ", 1, am_ret);
                    bt_ull_le_remove_am_dl_flag(BT_ULL_LE_FLAG_WAIT_AM_OPEN_CODEC);
                    /*notify service play fail*/
                    bt_ull_le_am_result_t result_notify;
                    result_notify.mode = BT_ULL_LE_AM_DL_MODE;
                    result_notify.result = BT_STATUS_FAIL;
                    bt_ull_le_am_event_callback(BT_ULL_LE_AM_PLAY_IND, &result_notify, sizeof(result_notify));

                    /*release audio source*/
                    bt_ull_le_am_dl_set_sub_state(ull_am_info, BT_ULL_LE_SUB_STATE_NONE);
                    audio_src_srv_update_state(ull_am_info->am_handle, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
                } else {
                    //ull_report("[ULL][LE][AM] dl play ind, bt_sink_srv_ami_audio_play success!!", 0);
                }
            } else if (BT_ULL_LE_SUB_STATE_CLEAR_CODEC == substate){
                audio_src_srv_update_state(ull_am_info->am_handle, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
            }
        }
    } 
    BT_ULL_MUTEX_UNLOCK();
}

static void bt_ull_le_am_dl_stop_ind_handle_message(audio_src_srv_handle_t *handle)
{
    bt_ull_le_am_mode_t mode = bt_ull_le_am_get_mode_by_audio_handle(handle);
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    BT_ULL_MUTEX_LOCK();
    if (BT_ULL_LE_AM_DL_MODE == mode) {
        bt_ull_le_am_dl_info_t *ull_am_info = &(am_ctx->dl_mode_ctx);
        bt_ull_le_am_state_t state = bt_ull_le_am_dl_get_state(ull_am_info);
        bt_ull_le_am_sub_state_t substate = bt_ull_le_am_dl_get_sub_state(ull_am_info);
        ull_report("[ULL][LE][AM] dl stop ind, state:%d, substate:%d, is_start_request:%d, am_flag: 0x%x", 4, state, substate, ull_am_info->is_start_request, ull_am_info->am_flag);
        if (ull_am_info->am_handle && (ull_am_info->am_handle == handle)) {
            /*1.notify server am stop*/
            bt_ull_le_am_result_t result_notify;
            result_notify.mode = mode;
            result_notify.result = BT_STATUS_SUCCESS;
            bt_ull_le_am_event_callback(BT_ULL_LE_AM_STOP_IND, &result_notify, sizeof(result_notify));

            if (BT_ULL_LE_SUB_STATE_PREPARE_CODEC == substate) {
                /*codec is openning, should release after codec open*/
               //ull_report("[ULL][LE][AM] dl stop ind, codec is openning, should release after codec open !!", 0);
                bt_ull_le_am_dl_set_sub_state(ull_am_info, BT_ULL_LE_SUB_STATE_CLEAR_CODEC);
            } else if ((BT_ULL_LE_SUB_STATE_NONE == substate) || (BT_ULL_LE_SUB_STATE_CLEAR_CODEC == substate)) {
                if (BT_ULL_LE_AM_PLAYING == state) {
                    bt_sink_srv_am_result_t ret = bt_sink_srv_ami_audio_stop(ull_am_info->audio_id);
                    if(AUD_EXECUTION_FAIL == ret) {
                        ull_report_error("[ULL][LE][AM] dl stop ind, bt_sink_srv_ami_audio_stop fail !!", 0);
                        audio_src_srv_update_state(ull_am_info->am_handle, AUDIO_SRC_SRV_EVT_READY);
                    }
                } else {
                    bt_ull_le_am_dl_set_sub_state(ull_am_info, BT_ULL_LE_SUB_STATE_NONE);//BTA-34248
                }
            }
        }
    } 
    BT_ULL_MUTEX_UNLOCK();
}

static void bt_ull_le_am_dl_reject_ind_handle_message(audio_src_srv_handle_t *handle)
{
    bt_ull_le_am_mode_t mode = bt_ull_le_am_get_mode_by_audio_handle(handle);
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    BT_ULL_MUTEX_LOCK();
    if (BT_ULL_LE_AM_DL_MODE == mode) {
        bt_ull_le_am_dl_info_t *ull_am_info = &(am_ctx->dl_mode_ctx);
        bt_ull_le_am_state_t state = bt_ull_le_am_dl_get_state(ull_am_info);
        am_ctx->dl_mode_ctx.is_waiting_play = true;
        am_ctx->dl_mode_ctx.is_reject = true;
        //bt_ull_le_srv_deactive_streaming(); /*remove iso data path*/
        bt_ull_le_srv_notify_server_play_is_allow(BT_ULL_PLAY_DISALLOW, BT_ULL_ALLOW_PLAY_REASON_AUDIO);
        bt_ull_le_am_sub_state_t substate = bt_ull_le_am_dl_get_sub_state(ull_am_info);
        ull_report("[ULL][LE][AM] dl reject ind, state:%d, substate:%d, is_start_request:%d", 3, state, substate, ull_am_info->is_start_request);
        if (ull_am_info->am_handle && (ull_am_info->am_handle == handle)) {
            /*notify server play is reject*/
            bt_ull_le_am_result_t result_notify;
            result_notify.mode = mode;
            result_notify.result = BT_STATUS_FAIL;
            bt_ull_le_am_event_callback(BT_ULL_LE_AM_PLAY_IND, &result_notify, sizeof(result_notify));
            audio_src_srv_update_state(ull_am_info->am_handle, AUDIO_SRC_SRV_EVT_READY);
            bt_ull_le_am_dl_set_sub_state(ull_am_info, BT_ULL_LE_SUB_STATE_NONE);
            if (ull_am_info->is_start_request) {
                bt_ull_le_am_add_waiting_list(mode);
            }
        }
    } 
    BT_ULL_MUTEX_UNLOCK();
}

static void bt_ull_le_am_dl_suspend_ind_handle_message(audio_src_srv_handle_t *handle, audio_src_srv_handle_t *int_hd)
{
    bt_ull_le_am_mode_t mode = bt_ull_le_am_get_mode_by_audio_handle(handle);

    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();

    BT_ULL_MUTEX_LOCK();
    if (BT_ULL_LE_AM_DL_MODE == mode) {
        bt_ull_le_am_dl_info_t *ull_am_info = &(am_ctx->dl_mode_ctx);
        bt_ull_le_am_state_t state = bt_ull_le_am_dl_get_state(ull_am_info);
        bt_ull_le_am_sub_state_t substate = bt_ull_le_am_dl_get_sub_state(ull_am_info);
        //ull_report("[ULL][LE][AM]suspend ind, get audio source success, mode:%d, state:%d, substate:%d", 3, mode, state, substate);   
        ull_report("[ULL][LE][AM] dl suspend ind, state:%d, substate:%d, is_streaming:%d, int_hd_type: %d", 4, state, substate, ull_am_info->is_start_request, int_hd->type);
        if (ull_am_info->am_handle && (ull_am_info->am_handle == handle)) {
            am_ctx->dl_mode_ctx.is_suspend = true;
            //bt_ull_le_srv_deactive_streaming(); /*remove iso data path*/
            bt_ull_le_srv_notify_server_play_is_allow(BT_ULL_PLAY_DISALLOW, BT_ULL_ALLOW_PLAY_REASON_AUDIO);

            /*notify server play is suspend*/
            bt_ull_le_am_result_t result_notify;
            result_notify.mode = mode;
            result_notify.result = BT_STATUS_FAIL;
            bt_ull_le_am_event_callback(BT_ULL_LE_AM_PLAY_IND, &result_notify, sizeof(result_notify));       
            if (BT_ULL_LE_SUB_STATE_PREPARE_CODEC == substate) {
                bt_ull_le_am_dl_set_sub_state(ull_am_info, BT_ULL_LE_SUB_STATE_CLEAR_CODEC);
            } else if ((BT_ULL_LE_SUB_STATE_PREPARE_AUDIO_SRC == substate) || (BT_ULL_LE_SUB_STATE_NONE == substate)) {
                bt_ull_le_am_dl_set_state(ull_am_info, BT_ULL_LE_AM_READY);//BTA-45379, before stop DL, reset state
                bt_ull_le_am_dl_set_sub_state(ull_am_info, BT_ULL_LE_SUB_STATE_NONE);
                bt_sink_srv_ami_audio_stop(ull_am_info->audio_id);
                ull_am_info->is_playing = false;
                //audio_src_srv_update_state(ull_am_info->am_handle, AUDIO_SRC_SRV_EVT_READY);
                bt_sink_srv_state_reset(BT_SINK_SRV_STATE_STREAMING);
            }
            bt_ull_le_srv_deactive_streaming(); /*remove iso data path*/

        }
    }else{
        ull_report("[ULL][LE][AM] dl suspend ind, mode is invalid:%d", 1, mode);
    }
    BT_ULL_MUTEX_UNLOCK();
}

//exception ind
static void bt_ull_le_am_dl_exception_ind_handle_message(audio_src_srv_handle_t *handle)
{
    bt_ull_le_am_mode_t mode = bt_ull_le_am_get_mode_by_audio_handle(handle);
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    if (BT_ULL_LE_AM_DL_MODE == mode) {
        bt_ull_le_am_dl_info_t *ull_am_info = &(am_ctx->dl_mode_ctx);
        bt_ull_le_am_state_t state = bt_ull_le_am_dl_get_state(ull_am_info);
        bt_ull_le_am_sub_state_t substate = bt_ull_le_am_dl_get_sub_state(ull_am_info);
        ull_report("[ULL][LE][AM] dl exception ind, state:%d, substate:%d, is_streaming:%d", 3, state, substate, ull_am_info->is_start_request); 
    } 
}

static void bt_ull_le_am_ul_start_audio(bt_ull_le_am_ul_info_t* ull_am_info)
{
    ull_report("[ULL][LE][AM] bt_ull_le_am_ul_start_audio, audio_id: 0x%x, streaming_flag: 0x%x, is_transmitter_start: 0x%x, is_mute: 0x%x, is_request_transmitter_start: %d", 5, 
        ull_am_info->audio_id, ull_am_info->streaming_flag, ull_am_info->is_transmitter_start, ull_am_info->is_mute, ull_am_info->is_request_transmitter_start); 
    
    //ull_am_info->is_request_transmitter_start = true;
    if (false == ull_am_info->is_transmitter_start) {
        if (ull_am_info->is_request_transmitter_start) {
        if ((ull_am_info->streaming_flag & BT_ULL_LE_AM_UL_STOPPING)
            || (ull_am_info->streaming_flag & BT_ULL_LE_AM_UL_STARTING)) {
           ull_report("[ULL][LE][AM] bt_ull_le_am_ul_start_audio.ul audio is starting or stopping, just need wait", 0);
        } else {
            bt_ull_le_set_am_ul_flag(BT_ULL_LE_AM_UL_STARTING);
            if ((AUD_ID_INVALID != ull_am_info->audio_id)
                && (BT_STATUS_SUCCESS != bt_ull_le_am_ul_play(ull_am_info->audio_id, bt_ull_le_srv_get_codec_type()))) {
                //ull_report("[ULL][LE][AM] bt_ull_le_am_ul_start_audio fail", 0);
                bt_ull_le_remove_am_ul_flag(BT_ULL_LE_AM_UL_STARTING);
            }
        }
    	} else {
            if ((ull_am_info->resource_handle != NULL)
                && ((AUDIO_SRC_SRV_EVENT_TAKE_SUCCESS == ull_am_info->resource_handle->state)||(AUDIO_SRC_SRV_EVENT_SUSPEND == ull_am_info->resource_handle->state))) {
                audio_src_srv_resource_manager_give(ull_am_info->resource_handle);
            }
        }
    } 
}

static bt_status_t bt_ull_le_am_ul_play(bt_sink_srv_am_id_t aud_id, bt_ull_le_codec_t codec_type)
{
    bt_sink_srv_am_result_t ami_ret;
    bt_sink_srv_am_audio_capability_t aud_cap;
    memset(&aud_cap, 0, sizeof(bt_sink_srv_am_audio_capability_t));
    bt_ull_client_t client_type = bt_ull_le_srv_get_client_type();
    aud_cap.type = ULL_BLE_UL;
    //aud_cap.codec.ull_ble_dl_format.ble_codec.codec = (BT_ULL_LE_CODEC_LC3PLUS == codec_type) ? BT_CODEC_TYPE_LE_AUDIO_LC3PULS : BT_CODEC_TYPE_LE_AUDIO_VENDOR;
    if (BT_ULL_LE_CODEC_LC3PLUS == codec_type || BT_ULL_LE_CODEC_DL_ULD_UL_LC3PLUS == codec_type) {
        aud_cap.codec.ull_ble_dl_format.ble_codec.codec = BT_CODEC_TYPE_LE_AUDIO_LC3PULS;
    } else if (BT_ULL_LE_CODEC_ULD == codec_type) {
        aud_cap.codec.ull_ble_dl_format.ble_codec.codec = BT_CODEC_TYPE_LE_AUDIO_ULD;
    } else {
        aud_cap.codec.ull_ble_dl_format.ble_codec.codec = BT_CODEC_TYPE_LE_AUDIO_VENDOR;
    }

    ull_report("[ULL][LE][AM] bt_ull_le_am_ul_play, set codec type: %d.", 1, aud_cap.codec.ull_ble_dl_format.ble_codec.codec);
    if (BT_ULL_MIC_CLIENT == client_type) {
    #ifdef AIR_WIRELESS_MIC_ENABLE
        /*wireless mic*/
        aud_cap.codec.ull_ble_ul_format.ble_codec.sample_rate = bt_ull_le_srv_get_codec_sample_rate(2, true, BT_ULL_ROLE_CLIENT);
        aud_cap.codec.ull_ble_ul_format.ble_codec.channel_num = bt_ull_le_srv_get_channel_mode(2, true, BT_ULL_ROLE_CLIENT);
        aud_cap.codec.ull_ble_ul_format.ble_codec.frame_payload_length = bt_ull_le_srv_get_sdu_size(true, BT_ULL_ROLE_CLIENT);
        aud_cap.codec.ull_ble_ul_format.ble_codec.context_type = AUDIO_CONTENT_TYPE_WIRELESS_MIC;
        uint32_t sdu_interval = bt_ull_le_srv_get_sdu_interval(true, BT_ULL_ROLE_CLIENT);
        aud_cap.codec.ull_ble_ul_format.ble_codec.frame_duration = bt_ull_le_am_switch_sdu_interval(sdu_interval); 			
        aud_cap.audio_stream_out.audio_device = HAL_AUDIO_DEVICE_HEADSET;
        aud_cap.audio_stream_out.audio_volume = AUD_VOL_OUT_MAX;
        aud_cap.audio_stream_out.audio_mute = false;
        aud_cap.audio_stream_in.audio_device = HAL_AUDIO_DEVICE_MAIN_MIC;//mic
        aud_cap.audio_stream_in.audio_volume = AUD_VOL_OUT_MAX;
        ull_report("[ULL][LE][AM] bt_ull_le_am_ul_play, wireless_mic config audio codec. sample_rate: %d, channel_num: %d, maxframe_payload_length: %d, context_type: %d, frame_duration: %d.",5,
            aud_cap.codec.ull_ble_ul_format.ble_codec.sample_rate,
            aud_cap.codec.ull_ble_ul_format.ble_codec.channel_num,
            aud_cap.codec.ull_ble_ul_format.ble_codec.frame_payload_length,
            aud_cap.codec.ull_ble_ul_format.ble_codec.context_type,
            aud_cap.codec.ull_ble_ul_format.ble_codec.frame_duration);
        ami_ret = ami_audio_play(aud_id, &aud_cap);
        ull_report("[ULL][LE][AM] bt_ull_le_am_ul_play. ami_ret: %d.", 1, ami_ret);
        return (AUD_EXECUTION_SUCCESS == ami_ret) ? (BT_STATUS_SUCCESS) : (BT_STATUS_FAIL);
    #else
     //   ull_report_error("[ULL][LE][AM] the option - AIR_WIRELESS_MIC_ENABLE is not open.", 0);
    #endif
    } else if (BT_ULL_HEADSET_CLIENT == client_type \
        || BT_ULL_EARBUDS_CLIENT == client_type \
        || BT_ULL_SPEAKER_CLIENT == client_type) {

        /*ULL 2.0 headset uplink*/
        aud_cap.codec.ull_ble_ul_format.ble_codec.sample_rate = bt_ull_le_srv_get_codec_sample_rate(2, true, BT_ULL_ROLE_CLIENT);
        aud_cap.codec.ull_ble_ul_format.ble_codec.channel_num = BT_ULL_LE_CHANNEL_MODE_MONO;
        aud_cap.codec.ull_ble_ul_format.ble_codec.frame_payload_length = bt_ull_le_srv_get_sdu_size(true, BT_ULL_ROLE_CLIENT);
        aud_cap.codec.ull_ble_ul_format.ble_codec.context_type = AUDIO_CONTENT_TYPE_ULL_BLE;//use ull code to test wireless mic
#ifdef AIR_ULL_ECNR_PREV_PART_ENABLE
        /*for AB156x ULL 2.0  uplink AINR Feature.*/
        aud_cap.codec.ull_ble_ul_format.ble_codec.nr_offload_flag = true;
#endif
        uint32_t sdu_interval = bt_ull_le_srv_get_sdu_interval(true, BT_ULL_ROLE_CLIENT);
        aud_cap.codec.ull_ble_ul_format.ble_codec.frame_duration = bt_ull_le_am_switch_sdu_interval(sdu_interval);              
        aud_cap.audio_stream_out.audio_device = HAL_AUDIO_DEVICE_HEADSET;
        aud_cap.audio_stream_out.audio_volume = AUD_VOL_OUT_MAX;
        aud_cap.audio_stream_out.audio_mute = false;
        aud_cap.audio_stream_in.audio_device = HAL_AUDIO_DEVICE_MAIN_MIC;//mic
        aud_cap.audio_stream_in.audio_volume = AUD_VOL_OUT_MAX;
        ull_report("[ULL][LE][AM] bt_ull_le_am_ul_play, config audio codec. sample_rate: %d, channel_num: %d, maxframe_payload_length: %d, context_type: %d, frame_duration: %d.",5,
            aud_cap.codec.ull_ble_ul_format.ble_codec.sample_rate,
            aud_cap.codec.ull_ble_ul_format.ble_codec.channel_num,
            aud_cap.codec.ull_ble_ul_format.ble_codec.frame_payload_length,
            aud_cap.codec.ull_ble_ul_format.ble_codec.context_type,
            aud_cap.codec.ull_ble_ul_format.ble_codec.frame_duration);
        ami_ret = ami_audio_play(aud_id, &aud_cap);
        //ull_report("[ULL][LE][AM] bt_ull_le_am_ul_play. ami_ret: %d.", 1, ami_ret);
        return (AUD_EXECUTION_SUCCESS == ami_ret) ? (BT_STATUS_SUCCESS) : (BT_STATUS_FAIL);
    }
    return BT_STATUS_FAIL;
}

static bt_status_t bt_ull_le_am_ul_stop_audio(bt_ull_le_am_ul_info_t* ull_am_info)
{
    //ull_am_info->is_request_transmitter_start = false;
    bt_status_t ret = BT_STATUS_SUCCESS;
    ull_report("[ULL][LE][AM] bt_ull_le_am_ul_stop_audio. is_transmitter_start: %d", 1, ull_am_info->is_transmitter_start);
    if (true == ull_am_info->is_transmitter_start) {
        if ((ull_am_info->streaming_flag & BT_ULL_LE_AM_UL_STOPPING)
            || (ull_am_info->streaming_flag & BT_ULL_LE_AM_UL_STARTING)) {
            //ull_report("[ULL][LE][AM] bt_ull_le_am_ul_stop_audio. ul audio is starting or stopping, just need wait", 0);
        } else {
            bt_ull_le_set_am_ul_flag(BT_ULL_LE_AM_UL_STOPPING);
            if ((AUD_ID_INVALID != ull_am_info->audio_id)
                && (AUD_EXECUTION_SUCCESS != ami_audio_stop(ull_am_info->audio_id))) {
                //ull_report("[ULL][LE][AM] bt_ull_le_am_ul_stop_audio. ul audio stop fail", 0);
                bt_ull_le_remove_am_ul_flag(BT_ULL_LE_AM_UL_STOPPING);
                ret = BT_STATUS_FAIL;
            }
        }
    } else {
        if (ull_am_info->sidetone_enable && !bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK)) {
            am_audio_side_tone_disable_by_scenario(SIDETONE_SCENARIO_ULL_V2);
        }
    }
    return ret;
}

static void bt_ull_le_am_mic_resource_callback(audio_src_srv_resource_manager_handle_t *current_handle, audio_src_srv_resource_manager_event_t event)
{
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    bt_ull_le_am_ul_info_t *ull_ul_am_info = &(am_ctx->ul_mode_ctx);
    ull_report("[ULL][LE][AM][MIC_RESOURCE_CALLBACK] current resource_handle:0x%04x, resource_handle:0x%04x, ull event:0x%04x, %x, %x", 5, \
        current_handle, ull_ul_am_info->resource_handle, event, am_ctx->ul_mode_ctx.is_suspend, am_ctx->ul_mode_ctx.is_waiting_play);
    if (current_handle == ull_ul_am_info->resource_handle) {
        switch (event) {
            case AUDIO_SRC_SRV_EVENT_TAKE_SUCCESS: 
            case AUDIO_SRC_SRV_EVENT_TAKE_ALREADY: {
                if (am_ctx->ul_mode_ctx.is_suspend) {
                    am_ctx->ul_mode_ctx.is_suspend = false;
                    //bt_ull_le_srv_active_streaming(); /*set iso data path*/
                    bt_ull_le_srv_notify_server_play_is_allow(BT_ULL_PLAY_ALLOW, BT_ULL_ALLOW_PLAY_REASON_AUDIO);
                }
                if (am_ctx->ul_mode_ctx.is_waiting_play) {
                    am_ctx->ul_mode_ctx.is_waiting_play = false;
                    //bt_ull_le_srv_active_streaming(); /*remove iso data path*/
                    bt_ull_le_srv_notify_server_play_is_allow(BT_ULL_PLAY_ALLOW, BT_ULL_ALLOW_PLAY_REASON_AUDIO);
                }
                bt_ull_le_am_ul_start_audio(ull_ul_am_info);
                break;
            }
            case AUDIO_SRC_SRV_EVENT_TAKE_REJECT: {
                am_ctx->ul_mode_ctx.is_waiting_play = true;
                if (!bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) 
                {
                    bt_ull_le_srv_deactive_streaming(); /*remove iso data path*/
                    bt_ull_le_srv_notify_server_play_is_allow(BT_ULL_PLAY_DISALLOW, BT_ULL_ALLOW_PLAY_REASON_AUDIO);
                }
                audio_src_srv_resource_manager_add_waiting_list(ull_ul_am_info->resource_handle);
                break;
            }
            case AUDIO_SRC_SRV_EVENT_GIVE_SUCCESS: {
                break;
            }
            case AUDIO_SRC_SRV_EVENT_SUSPEND: {
                am_ctx->ul_mode_ctx.is_suspend = true;
                if (!bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_DOWNLINK)) 
                {
                    bt_ull_le_srv_deactive_streaming(); /*remove iso data path*/
                    bt_ull_le_srv_notify_server_play_is_allow(BT_ULL_PLAY_DISALLOW, BT_ULL_ALLOW_PLAY_REASON_AUDIO);
                }
                bt_ull_le_am_stop(BT_ULL_LE_AM_UL_MODE, false);
                //ull_ul_am_info->is_suspend = true;
                break;
            }
            default:{
                break;
            }
        }
    }
}

static void bt_ull_le_am_ul_callback(bt_sink_srv_am_id_t aud_id, bt_sink_srv_am_cb_msg_class_t msg_id, bt_sink_srv_am_cb_sub_msg_t sub_msg, void *param)
{
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    bt_ull_le_am_ul_info_t *ull_ul_am_info = &(am_ctx->ul_mode_ctx);
    ull_report("[ULL][LE][AM] bt_ull_le_am_ul_callback, flag:0x%x, aud_id:%d, msg_id:%d, sub_msg:%d, is_mute:%d", 5, ull_ul_am_info->streaming_flag, aud_id, msg_id, sub_msg, ull_ul_am_info->is_mute);

    if (ull_ul_am_info->audio_id == aud_id) {
        bt_ull_le_am_result_t result_notify;
        result_notify.mode = BT_ULL_LE_AM_UL_MODE;
        switch (msg_id) {
            case AUD_SINK_OPEN_CODEC: {
                if (AUD_HFP_OPEN_CODEC_DONE == sub_msg) {
                   ull_report("[ULL][LE][AM] ul open codec success!!", 0);
                    result_notify.result = BT_STATUS_SUCCESS;
                    bt_ull_le_am_event_callback(BT_ULL_LE_AM_PLAY_IND, &result_notify, sizeof(result_notify));
                    //am_audio_side_tone_enable();//enable side tone
                    ull_report("[ULL][LE][AM] check sidetone enable: %d", 1, ull_ul_am_info->sidetone_enable);
                    if (ull_ul_am_info->sidetone_enable) {
                        am_audio_side_tone_enable_by_scenario(SIDETONE_SCENARIO_ULL_V2);
                    } else {
                        am_audio_side_tone_disable_by_scenario(SIDETONE_SCENARIO_ULL_V2);
                    }                   
                    //BTA-32453
                    if (BT_ULL_MIC_CLIENT == bt_ull_le_srv_get_client_type()) {
                        if (bt_timer_ext_find(BT_ULL_LE_AM_UL_MUTE_TIMER_ID)) {
                            bt_timer_ext_stop(BT_ULL_LE_AM_UL_MUTE_TIMER_ID);
                        }
                        bt_sink_srv_ami_audio_set_mute(ull_ul_am_info->audio_id, false, STREAM_IN);
                    }
                    if (!ull_ul_am_info->is_mute) {
                        bt_sink_srv_ami_audio_set_mute(ull_ul_am_info->audio_id, false, STREAM_IN);
                    }
                    bt_ull_le_am_ul_handle_start_success_cnf(ull_ul_am_info);
                } else {
                   //ull_report_error("[ULL][LE][AM] ul open codec fail!!", 0);
                    bt_ull_le_am_ul_handle_stop_success_or_start_fail_cnf(ull_ul_am_info);
                }
                break;
            }
            case AUD_SELF_CMD_REQ: {
                if (AUD_CMD_COMPLETE == sub_msg) {
                    ull_report("[ULL][LE][AM] ul close codec success!!", 0);
                    /*uplink is closed, give mic resource firstly*/
                    if (ull_ul_am_info->resource_handle != NULL) {
                        if ((AUDIO_SRC_SRV_EVENT_TAKE_SUCCESS == ull_ul_am_info->resource_handle->state)
                            || (AUDIO_SRC_SRV_EVENT_SUSPEND == ull_ul_am_info->resource_handle->state)) {
                            audio_src_srv_resource_manager_give(ull_ul_am_info->resource_handle);
                        }
                    }
                    result_notify.result = BT_STATUS_SUCCESS;
                    bt_ull_le_am_event_callback(BT_ULL_LE_AM_STOP_IND, &result_notify, sizeof(result_notify));
                    if (BT_ULL_MIC_CLIENT == bt_ull_le_srv_get_client_type()) {
                        if (bt_timer_ext_find(BT_ULL_LE_AM_UL_MUTE_TIMER_ID)) {
                            bt_timer_ext_stop(BT_ULL_LE_AM_UL_MUTE_TIMER_ID);
                        }
                        //bt_sink_srv_ami_audio_set_mute(ull_ul_am_info->audio_id, false, STREAM_IN);
                    }
                    //am_audio_side_tone_disable();//disable side tone
                    am_audio_side_tone_disable_by_scenario(SIDETONE_SCENARIO_ULL_V2);
                    bt_ull_le_am_ul_handle_stop_success_or_start_fail_cnf(ull_ul_am_info);

                } else if (AUD_CMD_FAILURE == sub_msg) {
                    /*uplink is closed, give mic resource firstly*/
                    if (ull_ul_am_info->resource_handle != NULL) {
                        if ((AUDIO_SRC_SRV_EVENT_TAKE_SUCCESS == ull_ul_am_info->resource_handle->state)
                            || (AUDIO_SRC_SRV_EVENT_SUSPEND == ull_ul_am_info->resource_handle->state)) {
                            audio_src_srv_resource_manager_give(ull_ul_am_info->resource_handle);
                        }
                    }            
                    result_notify.result = BT_STATUS_FAIL;
                    bt_ull_le_am_event_callback(BT_ULL_LE_AM_PLAY_IND, &result_notify, sizeof(result_notify));
                    am_audio_side_tone_disable();//disable side tone
                    bt_ull_le_am_ul_handle_stop_success_or_start_fail_cnf(ull_ul_am_info);
                }
                break;
            }
            default:
                break;
        }
    }
}

static void bt_ull_le_am_ul_handle_start_success_cnf(bt_ull_le_am_ul_info_t* ull_am_info)
{
    ull_report("[ULL][LE][AM] handle ul start success cnf, is_request_transmitter_start = 0x%x, flag:0x%x", 2, 
        ull_am_info->is_request_transmitter_start, ull_am_info->streaming_flag);

    ull_am_info->is_transmitter_start = true;
    bt_ull_le_remove_am_ul_flag(BT_ULL_LE_AM_UL_STOPPING);
    bt_ull_le_remove_am_ul_flag(BT_ULL_LE_AM_UL_STARTING);

    if (ull_am_info->streaming_flag & BT_ULL_LE_AM_UL_RECONFIG) {
        bt_ull_le_am_stop(BT_ULL_LE_AM_UL_MODE, false);
    } else {
        if (false == ull_am_info->is_request_transmitter_start) {
            bt_ull_le_am_stop(BT_ULL_LE_AM_UL_MODE, false);
        }
        //am_audio_side_tone_enable(); // not need enable side tone
    }
}

static void bt_ull_le_am_ul_handle_stop_success_or_start_fail_cnf(bt_ull_le_am_ul_info_t* ull_am_info)
{
    ull_report("[ULL][LE][AM] handle ul stop success or start fail cnf, is_request_transmitter_start = 0x%x, flag:0x%x, mic_rsc_state: 0x%x, is_suspend: %d", 4, 
        ull_am_info->is_request_transmitter_start, ull_am_info->streaming_flag, ull_am_info->resource_handle->state, ull_am_info->is_suspend);

    bt_ull_le_remove_am_ul_flag(BT_ULL_LE_AM_UL_STARTING);
    bt_ull_le_remove_am_ul_flag(BT_ULL_LE_AM_UL_STOPPING);
    ull_am_info->is_transmitter_start = false;
    if (ull_am_info->streaming_flag & BT_ULL_LE_AM_UL_RECONFIG) {
        bt_ull_le_remove_am_ul_flag(BT_ULL_LE_AM_UL_RECONFIG);
        bt_sink_srv_am_result_t ami_ret = bt_sink_srv_ami_audio_close(ull_am_info->audio_id);
        //ull_report("[ULL][LE][AM] ul deinit :%d", 1, ami_ret);
        if (AUD_EXECUTION_SUCCESS == ami_ret) {
            ull_am_info->audio_id = AUD_ID_INVALID;
            bt_ull_le_am_ul_init();
            //ull_am_info->is_request_transmitter_start = true;
        }
    }

    if (ull_am_info->is_request_transmitter_start && !ull_am_info->is_mute && !ull_am_info->is_suspend) {
        bt_ull_le_am_play(BT_ULL_LE_AM_UL_MODE, bt_ull_le_srv_get_codec_type(), false);
    } else {
        if (ull_am_info->resource_handle != NULL) {
            if (ull_am_info->is_suspend) {
                //ull_am_info->is_suspend = false;
                audio_src_srv_resource_manager_add_waiting_list(ull_am_info->resource_handle);
            }
        }
    }
}
#if 0
static bt_status_t bt_ull_le_am_play_internal(bt_ull_le_am_mode_t mode, bt_ull_le_codec_t codec)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    if (BT_ULL_LE_AM_DL_MODE == mode) {
        bt_ull_le_am_dl_info_t *ull_dl_am_info = &(am_ctx->dl_mode_ctx);
        //ull_dl_am_info->codec_type = codec;
        bt_ull_le_am_state_t state = bt_ull_le_am_dl_get_state(ull_dl_am_info);

        bt_ull_le_am_sub_state_t substate = bt_ull_le_am_dl_get_sub_state(ull_dl_am_info);
        ull_report("[ULL][LE][AM] bt_ull_le_am_play_internal, mode: %d, state: %d, substate: %d", 3, mode, state, substate);
        bt_ull_le_am_remove_waiting_list(mode);
        if (BT_ULL_LE_AM_READY == state) {
            if (BT_ULL_LE_SUB_STATE_NONE == substate) {
                //bt_ull_le_am_set_sub_state(mode, BT_ULL_LE_SUB_STATE_PREPARE_AUDIO_SRC);
                bt_ull_le_am_dl_set_sub_state(ull_dl_am_info, BT_ULL_LE_SUB_STATE_PREPARE_AUDIO_SRC);
                audio_src_srv_update_state(ull_dl_am_info->am_handle, AUDIO_SRC_SRV_EVT_PREPARE_PLAY);
            } else if (BT_ULL_LE_SUB_STATE_CLEAR_CODEC == substate) {
                /*codec is closing, just wait*/
         //       ull_report("[ULL][LE][AM] bt_ull_le_am_play_internal, codec is closing, just wait", 0);
                bt_ull_le_am_dl_set_sub_state(ull_dl_am_info, BT_ULL_LE_SUB_STATE_PREPARE_CODEC);
            }
        }       
    
    } else if (BT_ULL_LE_AM_UL_MODE == mode) { 
        bt_ull_le_am_ul_info_t *ull_ul_am_info = &(am_ctx->ul_mode_ctx);
        //ull_ul_am_info->codec_type = codec;
        if (NULL != ull_ul_am_info->resource_handle) {
            ull_report("[ULL][LE][AM] bt_ull_le_am_play_internal, ul mic, take audio resource firstly.", 0);
            audio_src_srv_resource_manager_take(ull_ul_am_info->resource_handle);
        } else {
        //  ull_report("[ULL][LE][AM] bt_ull_le_am_play_internal, ul mic, resource handle is NULL!.", 0);
            ret = BT_STATUS_FAIL;
        }
    } 
    return ret;
}
#endif
#if 0 
static bt_status_t bt_ull_le_am_stop_internal(bt_ull_le_am_mode_t mode)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    if (BT_ULL_LE_AM_DL_MODE == mode) {
        bt_ull_le_am_dl_info_t *ull_dl_am_info = &(am_ctx->dl_mode_ctx);
        bt_ull_le_am_remove_waiting_list(mode);//need remove waiting list when stop
        bt_ull_le_am_state_t state = bt_ull_le_am_dl_get_state(ull_dl_am_info);
        bt_ull_le_am_sub_state_t substate = bt_ull_le_am_dl_get_sub_state(ull_dl_am_info);

        ull_report("[ULL][LE][AM] bt_ull_le_am_stop_internal, mode: %d, state: %d, substate: %d", 3, mode, state, substate);
        if ((BT_ULL_LE_SUB_STATE_PREPARE_CODEC == substate) || (BT_ULL_LE_SUB_STATE_PREPARE_AUDIO_SRC == substate)) {
            /*codec is opening, we just need wait*/
          //  ull_report("[ULL][LE][AM] bt_ull_le_am_stop_internal, codec is openning or stoping, just wait", 0);
            bt_ull_le_am_dl_set_sub_state(ull_dl_am_info, BT_ULL_LE_SUB_STATE_CLEAR_CODEC);
        } else if ((BT_ULL_LE_SUB_STATE_NONE == substate) || (BT_ULL_LE_AM_PLAYING == state)) {
            bt_ull_le_am_dl_set_sub_state(ull_dl_am_info, BT_ULL_LE_SUB_STATE_CLEAR_CODEC);
            audio_src_srv_update_state(ull_dl_am_info->am_handle, AUDIO_SRC_SRV_EVT_PREPARE_STOP);//release audio source
        }
    } else if (BT_ULL_LE_AM_UL_MODE == mode) {
        bt_ull_le_am_ul_info_t *ull_ul_am_info = &(am_ctx->ul_mode_ctx);
        ull_report("[ULL][LE][AM] bt_ull_le_am_ul_stop_audio, audio_id: 0x%x, streaming_flag: 0x%x, is_transmitter_start: 0x%x", 3, 
        ull_ul_am_info->audio_id, ull_ul_am_info->streaming_flag, ull_ul_am_info->is_transmitter_start);
        if (BT_STATUS_FAIL == bt_ull_le_am_ul_stop_audio(ull_ul_am_info)) {
            ret = BT_STATUS_FAIL;
        }
    } 
    return ret;
}
#endif
/**************************************************************************************************
* Functions - External
**************************************************************************************************/
void bt_ull_le_am_init(bt_ull_le_am_callback_t callback)
{
    /*init DL mode*/
    bt_ull_le_am_dl_init();
    /*init UL mode*/
    bt_ull_le_am_ul_init();
    g_bt_ull_le_am_callback = callback;
    //ull_report("[ULL][LE][AM] init done", 0);
}

void bt_ull_le_am_deinit(bt_ull_le_am_mode_t mode)
{
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    if (BT_ULL_LE_AM_DL_MODE == mode) {
        bt_ull_le_am_dl_info_t *ull_dl_am_info = &(am_ctx->dl_mode_ctx);
        bt_ull_le_am_state_t state = bt_ull_le_am_dl_get_state(ull_dl_am_info);
        bt_ull_le_am_sub_state_t substate = bt_ull_le_am_dl_get_sub_state(ull_dl_am_info);
        ull_report("[ULL][LE][AM] bt_ull_le_am_deinit, mode: %d, state: %d, substate: %d", 2, mode, state, substate);
        bt_ull_le_am_remove_waiting_list(mode);
        if ((BT_ULL_LE_SUB_STATE_PREPARE_CODEC == substate) || (BT_ULL_LE_SUB_STATE_PREPARE_AUDIO_SRC == substate)) {
            /*codec is opening, set flag and wait*/
			bt_ull_le_set_am_dl_flag(BT_ULL_LE_FLAG_WAIT_AM_STOP_CODEC);
            bt_ull_le_am_dl_set_sub_state(ull_dl_am_info, BT_ULL_LE_SUB_STATE_CLEAR_CODEC);
        } else if ((BT_ULL_LE_SUB_STATE_NONE == substate) && (BT_ULL_LE_AM_PLAYING == state)){
            bt_ull_le_set_am_dl_flag(BT_ULL_LE_FLAG_WAIT_AM_STOP_CODEC);
            bt_ull_le_am_dl_set_sub_state(ull_dl_am_info, BT_ULL_LE_SUB_STATE_CLEAR_CODEC);
            /* Release audio source */
            audio_src_srv_update_state(ull_dl_am_info->am_handle, AUDIO_SRC_SRV_EVT_PREPARE_STOP);
        } else {
            if (ull_dl_am_info->am_handle) {
                audio_src_srv_destruct_handle(ull_dl_am_info->am_handle);
                ull_dl_am_info->am_handle = NULL;
                bt_ull_le_am_dl_set_state(ull_dl_am_info, BT_ULL_LE_AM_IDLE);
            }
        }
    } else if (BT_ULL_LE_AM_UL_MODE == mode){
        bt_ull_le_am_ul_info_t *ull_ul_am_info = &(am_ctx->ul_mode_ctx);
        ull_report("[ULL][LE][AM] bt_ull_le_am_deinit, mode: %d", 1, mode);
        if (ull_ul_am_info->resource_handle != NULL) {
            if ((AUDIO_SRC_SRV_EVENT_TAKE_SUCCESS == ull_ul_am_info->resource_handle->state)
              || (AUDIO_SRC_SRV_EVENT_SUSPEND == ull_ul_am_info->resource_handle->state)) {
              audio_src_srv_resource_manager_give(ull_ul_am_info->resource_handle);
            }
            audio_src_srv_resource_manager_destruct_handle(ull_ul_am_info->resource_handle);
            ull_ul_am_info->resource_handle = NULL;         
        }
        bt_sink_srv_am_result_t ami_ret = bt_sink_srv_ami_audio_close(ull_ul_am_info->audio_id);
        ull_report("[ULL][LE][AM] ul deinit :%d", 1, ami_ret);
    }
}

bt_status_t bt_ull_le_am_play(bt_ull_le_am_mode_t mode, bt_ull_le_codec_t codec, bool is_app_request)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();

    if (BT_ULL_LE_AM_DL_MODE == mode) {
        bt_ull_le_am_dl_info_t *ull_dl_am_info = &(am_ctx->dl_mode_ctx);
        if (true == is_app_request) {
            ull_dl_am_info->is_start_request = true;
        }
        //ull_dl_am_info->codec_type = codec;
        bt_ull_le_am_state_t state = bt_ull_le_am_dl_get_state(ull_dl_am_info);

        bt_ull_le_am_sub_state_t substate = bt_ull_le_am_dl_get_sub_state(ull_dl_am_info);
        ull_report("[ULL][LE][AM] bt_ull_le_am_play, is_app_request: %d, mode: %d, state: %d, substate: %d, is_suspend: %d", 5, is_app_request, mode, state, substate, ull_dl_am_info->is_suspend);
        if (BT_ULL_LE_SUB_STATE_NONE == substate) {
            bt_ull_le_am_remove_waiting_list(mode);
        }
        if (BT_ULL_LE_AM_IDLE == state) {
        //ull_report("[ULL][LE][AM] am is not init !! mode: %d.", 1, mode);
            ret = BT_STATUS_FAIL;
        }
        if (BT_ULL_LE_AM_READY == state) {
            if (BT_ULL_LE_SUB_STATE_NONE == substate) {
#ifdef AIR_BT_CODEC_BLE_ENABLED
                bt_ull_le_srv_stream_context_t *stream_ctx = bt_ull_le_srv_get_stream_context();
                bt_ble_set_lightmode(BT_ULL_LE_SRV_AUDIO_QUALITY_LOW_POWER == stream_ctx->audio_quality ? true : false);
#endif
                //bt_ull_le_am_set_sub_state(mode, BT_ULL_LE_SUB_STATE_PREPARE_AUDIO_SRC);
                bt_ull_le_am_dl_set_sub_state(ull_dl_am_info, BT_ULL_LE_SUB_STATE_PREPARE_AUDIO_SRC);
                audio_src_srv_update_state(ull_dl_am_info->am_handle, AUDIO_SRC_SRV_EVT_PREPARE_PLAY);
            } else if (BT_ULL_LE_SUB_STATE_CLEAR_CODEC == substate && !ull_dl_am_info->is_suspend) {
                /*codec is closing, just wait*/
                //ull_report("[ULL][LE][AM] bt_ull_le_am_play, codec is closing, just wait", 0);
                bt_ull_le_am_dl_set_sub_state(ull_dl_am_info, BT_ULL_LE_SUB_STATE_PREPARE_CODEC);
            }
        }       
    
    } else if (BT_ULL_LE_AM_UL_MODE == mode) { 
        bt_ull_le_am_ul_info_t *ull_ul_am_info = &(am_ctx->ul_mode_ctx);
        if (true == is_app_request) {
            ull_ul_am_info->is_request_transmitter_start = true;
        }
        //ull_ul_am_info->codec_type = codec;
        if (NULL != ull_ul_am_info->resource_handle) {
            ull_report("[ULL][LE][AM] bt_ull_le_am_play, ul mic, is_app_request: %d, take audio resource firstly.", 1, is_app_request);
            audio_src_srv_resource_manager_take(ull_ul_am_info->resource_handle);
        } else {
            //ull_report("[ULL][LE][AM] bt_ull_le_am_play, ul mic, resource handle is NULL!.", 0);
            ret = BT_STATUS_FAIL;
        }
    } 
    return ret;
}

bt_status_t bt_ull_le_am_stop(bt_ull_le_am_mode_t mode, bool is_app_request)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    if (BT_ULL_LE_AM_DL_MODE == mode) {
        bt_ull_le_am_dl_info_t *ull_dl_am_info = &(am_ctx->dl_mode_ctx);
        if (true == is_app_request) {
            ull_dl_am_info->is_start_request = false;
        }
        bt_ull_le_am_state_t state = bt_ull_le_am_dl_get_state(ull_dl_am_info);
        bt_ull_le_am_sub_state_t substate = bt_ull_le_am_dl_get_sub_state(ull_dl_am_info);

        ull_report("[ULL][LE][AM] bt_ull_le_am_stop dl state: %d, substate: %d, is_add_waiting_list: %d", 3, state, substate, ull_dl_am_info->is_add_waiting_list);
        if (ull_dl_am_info->is_add_waiting_list) {
            bt_ull_le_am_remove_waiting_list(mode);//need remove waiting list when stop
        }
        if ((BT_ULL_LE_SUB_STATE_PREPARE_CODEC == substate) || (BT_ULL_LE_SUB_STATE_PREPARE_AUDIO_SRC == substate)) {
            /*codec is opening, we just need wait*/
            //ull_report("[ULL][LE][AM] bt_ull_le_am_stop, codec is openning or stoping, just wait", 0);
            bt_ull_le_am_dl_set_sub_state(ull_dl_am_info, BT_ULL_LE_SUB_STATE_CLEAR_CODEC);
        } else if (BT_ULL_LE_AM_PLAYING == state) {
            bt_ull_le_am_dl_set_sub_state(ull_dl_am_info, BT_ULL_LE_SUB_STATE_CLEAR_CODEC);
            audio_src_srv_update_state(ull_dl_am_info->am_handle, AUDIO_SRC_SRV_EVT_PREPARE_STOP);//release audio source
        }
    } else if (BT_ULL_LE_AM_UL_MODE == mode) {
        bt_ull_le_am_ul_info_t *ull_ul_am_info = &(am_ctx->ul_mode_ctx);
        if (true == is_app_request) {
            ull_ul_am_info->is_request_transmitter_start = false;
        }
          //ull_report("[ULL][LE][AM] bt_ull_le_am_ul_stop_audio, audio_id: 0x%x, streaming_flag: 0x%x, is_transmitter_start: 0x%x", 3, 
          //ull_ul_am_info->audio_id, ull_ul_am_info->streaming_flag, ull_ul_am_info->is_transmitter_start);
        if (BT_STATUS_FAIL == bt_ull_le_am_ul_stop_audio(ull_ul_am_info)) {
            ret = BT_STATUS_FAIL;
        }
    } 
    return ret;
}

//restart audio
bt_status_t bt_ull_le_am_restart(bt_ull_le_am_mode_t mode)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    if (BT_ULL_LE_AM_DL_MODE == mode) {
        bt_ull_le_am_dl_info_t *ull_dl_am_info = &(am_ctx->dl_mode_ctx);
        bt_ull_le_am_state_t state = bt_ull_le_am_dl_get_state(ull_dl_am_info);
        bt_ull_le_am_sub_state_t substate = bt_ull_le_am_dl_get_sub_state(ull_dl_am_info);
        ull_report("[ULL][LE][AM] bt_ull_le_am_restart, state: %d, substate: %d", 2, state, substate);      
        if (BT_ULL_LE_SUB_STATE_PREPARE_CODEC == substate) {
            /*codec is opening, need wait*/
            //ull_report("[ULL][LE][AM] bt_ull_le_am_restart, codec is openning, just wait", 0);
            bt_ull_le_set_am_dl_flag(BT_ULL_LE_FLAG_WAIT_AM_RESTART_CODEC);
            bt_ull_le_am_dl_set_sub_state(ull_dl_am_info, BT_ULL_LE_SUB_STATE_CLEAR_CODEC);
        } else if ((BT_ULL_LE_SUB_STATE_NONE == substate) && (BT_ULL_LE_AM_PLAYING == state)) {
            bt_ull_le_set_am_dl_flag(BT_ULL_LE_FLAG_WAIT_AM_RESTART_CODEC);
            bt_ull_le_am_dl_set_sub_state(ull_dl_am_info, BT_ULL_LE_SUB_STATE_CLEAR_CODEC);
            audio_src_srv_update_state(ull_dl_am_info->am_handle, AUDIO_SRC_SRV_EVT_PREPARE_STOP);//release audio source
        }
    }
    return ret;
}


bt_status_t bt_ull_le_am_suspend(bt_ull_le_am_mode_t mode)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    if (BT_ULL_LE_AM_DL_MODE == mode) {
        bt_ull_le_am_dl_info_t *ull_dl_am_info = &(am_ctx->dl_mode_ctx);
        bt_ull_le_am_state_t state = bt_ull_le_am_dl_get_state(ull_dl_am_info);
        bt_ull_le_am_sub_state_t substate = bt_ull_le_am_dl_get_sub_state(ull_dl_am_info);
        ull_report("[ULL][LE][AM] bt_ull_le_am_suspend, mode: %d, state: %d, substate: %d", 2, mode, state, substate);
    } else {
    //  ull_report("[ULL][LE][AM] bt_ull_le_am_suspend, invalid am mode!", 0);
        ret = BT_STATUS_FAIL;
    }
    return ret;
}


bt_status_t bt_ull_le_am_set_mute(bt_ull_le_am_mode_t mode, bool is_mute)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    bt_sink_srv_am_result_t am_ret = AUD_EXECUTION_SUCCESS;
    if (BT_ULL_LE_AM_DL_MODE == mode) {
        bt_ull_le_am_dl_info_t *ull_dl_am_info = &(am_ctx->dl_mode_ctx);
        bt_ull_le_am_state_t state = bt_ull_le_am_dl_get_state(ull_dl_am_info);
        bt_ull_le_am_sub_state_t substate = bt_ull_le_am_dl_get_sub_state(ull_dl_am_info);
        ull_report("[ULL][LE][AM] bt_ull_le_am_set_mute, mode: %d, state: %d, substate: %d, is_mute: %d, ull_dl_am_info->is_mute: %d, is_start_request: %d", 6, 
            mode, state, substate, is_mute, ull_dl_am_info->is_mute, ull_dl_am_info->is_start_request);            
        if ((BT_ULL_LE_AM_PLAYING == state) && ull_dl_am_info->is_start_request && is_mute && (!ull_dl_am_info->is_mute)) {
            /*mute: need stop dsp*/
            ull_dl_am_info->is_mute = is_mute;
            //bt_ull_le_am_stop(mode, false);
            am_ret = bt_sink_srv_ami_audio_set_volume(ull_dl_am_info->audio_id, 0, STREAM_OUT);
            ull_report("[ULL][LE][AM] set dl mute. ret = %d.", 1, am_ret);
            //ull_report("[ULL][LE][AM] after handle bt_ull_le_am_set_mute, current mute status:%d", 1, ull_dl_am_info->is_mute);
        } else if ((!is_mute) && ull_dl_am_info->is_mute) {
            ull_dl_am_info->is_mute = is_mute;
            //bt_ull_le_am_play(mode, bt_ull_le_srv_get_codec_type(), false);
            //am_ret = bt_sink_srv_ami_audio_set_volume(ull_dl_am_info->audio_id, ull_dl_am_info->volume.vol_left, STREAM_OUT);
            am_ret = bt_sink_srv_ami_audio_set_volume(ull_dl_am_info->audio_id, AUD_VOL_OUT_LEVEL15, STREAM_OUT);
            ull_report("[ULL][LE][AM] set dl unmute. ret = %d.", 1, am_ret);
            //ull_report("[ULL][LE][AM] after handle bt_ull_le_am_set_mute, current mute status:%d", 1, ull_dl_am_info->is_mute);
        }
    } else if (BT_ULL_LE_AM_UL_MODE == mode) {
        bt_ull_le_am_ul_info_t *ull_ul_am_info = &(am_ctx->ul_mode_ctx);
        ull_report("[ULL][LE][AM] bt_ull_le_am_set_mute, audio_id: 0x%x, streaming_flag: 0x%x, is_transmitter_start: 0x%x, is_mute: %d, ull_ul_am_info->is_mute: %d", 5, 
        ull_ul_am_info->audio_id, ull_ul_am_info->streaming_flag, ull_ul_am_info->is_transmitter_start, is_mute, ull_ul_am_info->is_mute);

        if (ull_ul_am_info->is_transmitter_start && is_mute && (!ull_ul_am_info->is_mute)) {
            ull_ul_am_info->is_mute = is_mute;
            if (BT_ULL_MIC_CLIENT == bt_ull_le_srv_get_client_type()) {
                if (bt_timer_ext_find(BT_ULL_LE_AM_UL_MUTE_TIMER_ID)) {
                    bt_timer_ext_stop(BT_ULL_LE_AM_UL_MUTE_TIMER_ID);
                }
                bt_sink_srv_am_result_t ret = bt_sink_srv_ami_audio_set_mute(ull_ul_am_info->audio_id, true, STREAM_IN);
                ull_report("[ULL][LE][AM] wireless mic set audio mute. ret = %d.", 1, ret);
                bt_timer_ext_start(BT_ULL_LE_AM_UL_MUTE_TIMER_ID, 0, BT_ULL_LE_AM_UL_DELAY_MUTE_TIMER, bt_ull_le_am_mute_timeout_callbak);
            } else {
                //bt_ull_le_am_ul_stop_audio(ull_ul_am_info);
                am_ret = bt_sink_srv_ami_audio_set_mute(ull_ul_am_info->audio_id, true, STREAM_IN);
                ull_report("[ULL][LE][AM] set ul mute. ret = %d.", 1, am_ret);
            }
            
        } else if (!is_mute && ull_ul_am_info->is_mute && ull_ul_am_info->resource_handle) {
            ull_ul_am_info->is_mute = is_mute;
            //BTA-31596, if ull is not connected, don't open ul streaming.
            if (!bt_ull_le_service_is_connected()) {
               ull_report_error("[ULL][LE][AM] bt_ull_le_am_set_mute, ull is not connected!!", 0); 
            } else {
                if (BT_ULL_MIC_CLIENT == bt_ull_le_srv_get_client_type()) {
                    if (!ull_ul_am_info->is_transmitter_start) {
                        ull_report("[ULL][LE][AM] after handle bt_ull_le_am_set_mute, current mute status:%d, ul mic, take audio resource firstly.", 1, ull_ul_am_info->is_mute);
                        ull_ul_am_info->is_mute = is_mute;
                        if (bt_timer_ext_find(BT_ULL_LE_AM_UL_MUTE_TIMER_ID)) {
                            bt_timer_ext_stop(BT_ULL_LE_AM_UL_MUTE_TIMER_ID);
                        }
                        audio_src_srv_resource_manager_take(ull_ul_am_info->resource_handle);
                    }
                } else {
                    ull_ul_am_info->is_mute = is_mute;
                    am_ret = bt_sink_srv_ami_audio_set_mute(ull_ul_am_info->audio_id, false, STREAM_IN);;
                    ull_report("[ULL][LE][AM] set ul unmute. ret = %d.", 1, am_ret);
                }
            }
        }else if (!ull_ul_am_info->is_transmitter_start) {
            //ul is not open, but need set is_mute flag
            ull_report("[ULL][LE][AM] ul is not open, but need set is_mute flag : %d.", 1, is_mute);
            ull_ul_am_info->is_mute = is_mute;
        }
    }
    return ret;
}
static bt_sink_srv_am_volume_level_out_t bt_ull_le_am_get_volume_level(uint8_t volume)
{
    /* From %(0 ~ 100) to AUD_VOL_OUT_XX(0~15) */
    float local_level_f = AUD_VOL_OUT_LEVEL0;
    float orignal_f = volume;
    bt_sink_srv_am_volume_level_t max_lev = AUD_VOL_OUT_LEVEL15;

    local_level_f = (orignal_f * max_lev) / BT_ULL_USB_VOLUME_MAX + 0.5f;
    ull_report("[ULL][LE][AM] get_volume_level[s]-orignal: %d, vol_level: %d", 2, volume, (uint8_t)local_level_f);
    return (uint8_t)local_level_f;

}

bt_status_t bt_ull_le_am_set_volume(bt_ull_le_am_mode_t mode, uint8_t volume, bt_ull_audio_channel_t channel)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    if (BT_ULL_LE_AM_DL_MODE == mode) {
        bt_ull_le_am_dl_info_t *ull_dl_am_info = &(am_ctx->dl_mode_ctx);
        bt_ull_le_am_state_t state = bt_ull_le_am_dl_get_state(ull_dl_am_info);
        bt_ull_le_am_sub_state_t substate = bt_ull_le_am_dl_get_sub_state(ull_dl_am_info);
        bt_sink_srv_am_volume_level_t vol_level = bt_ull_le_am_get_volume_level(volume);
        ull_report("[ULL][LE][AM] bt_ull_le_am_set_volume, state: %d, substate: %d, volume: %d, volume_level: %d, channel: %d", 5, state, substate, volume, vol_level, channel); 
        if (BT_ULL_AUDIO_CHANNEL_LEFT == channel) {
            ull_dl_am_info->volume.vol_left = vol_level;
        } else if (BT_ULL_AUDIO_CHANNEL_RIGHT == channel) {
            ull_dl_am_info->volume.vol_right = vol_level;
        } else if (BT_ULL_AUDIO_CHANNEL_DUAL == channel) {
            ull_dl_am_info->volume.vol_left = vol_level;
            ull_dl_am_info->volume.vol_right = vol_level;
        }
        if ((0 == ull_dl_am_info->volume.vol_left) && (0 == ull_dl_am_info->volume.vol_right)) {
            bt_ull_le_am_set_mute(mode, true);
        }  else { 
            if (ull_dl_am_info->is_mute) {
                bt_ull_le_am_set_mute(mode, false);
            }
        }
        bt_sink_srv_am_result_t am_ret = AUD_EXECUTION_SUCCESS;
        if (BT_ULL_LE_AM_PLAYING == state) {
            if ((0 == ull_dl_am_info->volume.vol_left) && (0 != ull_dl_am_info->volume.vol_right)) {
                am_ret = bt_sink_srv_ami_audio_set_volume(ull_dl_am_info->audio_id, ull_dl_am_info->volume.vol_right, STREAM_OUT);
            } else if ((0 != ull_dl_am_info->volume.vol_left) && (0 == ull_dl_am_info->volume.vol_right)) {
                am_ret = bt_sink_srv_ami_audio_set_volume(ull_dl_am_info->audio_id, ull_dl_am_info->volume.vol_left, STREAM_OUT);
            } else if ((0 != ull_dl_am_info->volume.vol_left) && (0 != ull_dl_am_info->volume.vol_right)) {
                am_ret = bt_sink_srv_ami_audio_set_volume(ull_dl_am_info->audio_id, vol_level, STREAM_OUT);
            }
        }
        return (AUD_EXECUTION_SUCCESS == am_ret) ? BT_STATUS_SUCCESS : BT_STATUS_FAIL;
    }
    return ret;
}

void bt_ull_le_am_sync_volume(bt_ull_original_duel_volume_t *volume)
{
    ull_report("[ULL][LE][AM] bt_ull_le_am_sync_volume, vol_left: %d, vol_right: %d", 2, volume->vol_left, volume->vol_right);
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    bt_ull_le_am_dl_info_t *ull_dl_am_info = &(am_ctx->dl_mode_ctx);
    ull_dl_am_info->volume.vol_left = (0 == volume->vol_left)?(AUD_VOL_OUT_LEVEL15):bt_ull_le_am_get_volume_level(volume->vol_left);
    ull_dl_am_info->volume.vol_right = (0 == volume->vol_right)?(AUD_VOL_OUT_LEVEL15):bt_ull_le_am_get_volume_level(volume->vol_right);
}

bool bt_ull_le_am_is_allow_play(void)
{
    bool is_allow;
    const audio_src_srv_handle_t* running = audio_src_srv_get_runing_pseudo_device();
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    bt_ull_le_am_dl_info_t *ull_dl_am_info = &(am_ctx->dl_mode_ctx);
    if (running && \
        (running->priority >= ull_dl_am_info->am_handle->priority) && \
        ((uint32_t)running != (uint32_t)ull_dl_am_info->am_handle)) {
        is_allow = false;
    } else {
        is_allow = true;
    }
    ull_report("[ULL][LE][AM] is allow play, %d, runing: %x, ull2: %x", 3, is_allow, (uint32_t)running, (uint32_t)(ull_dl_am_info->am_handle));
    return is_allow;
}

bool bt_ull_le_am_is_playing(bt_ull_le_am_mode_t mode)
{
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    if (BT_ULL_LE_AM_DL_MODE == mode) {
        return am_ctx->dl_mode_ctx.is_playing;
    } else if (BT_ULL_LE_AM_UL_MODE == mode){
        return am_ctx->ul_mode_ctx.is_transmitter_start;
    } 
    return false;
}

static le_audio_interval_t bt_ull_le_am_switch_sdu_interval(uint32_t sdu_interval)
{
    le_audio_interval_t ret = LE_AUDIO_interval_1_25;
    if(1250 == sdu_interval) {
        ret = LE_AUDIO_interval_1_25;
    } else if(2500 == sdu_interval) {
        ret = LE_AUDIO_interval_2_5;
    } else if(5000 == sdu_interval) {
        ret = LE_AUDIO_interval_5;
    } else if(7500 == sdu_interval) {
        ret = LE_AUDIO_interval_7_5;
    } else if(10000 == sdu_interval) {
        ret = LE_AUDIO_interval_10;
    } else if(1000 == sdu_interval) {
        ret = LE_AUDIO_interval_1;
    } else if(2000 == sdu_interval) {
        ret = LE_AUDIO_interval_2;
    } else {
      //ull_report_error("[ULL][LE][AM] sdu_interval is invalid!! sdu_interval: %d.", 1, sdu_interval);
        ull_assert(0 && "invalid sdu_interval");
    }
      //ull_report("[ULL][LE][AM] switch sdu interval, origin sdu interval: %d, switched sdu interval", 2, sdu_interval, ret);   
    return ret;
}

static void bt_ull_le_am_mute_timeout_callbak(uint32_t timer_id, uint32_t data)
{
    ull_report("[ULL][LE][AM] bt_ull_le_am_mute_timeout_callbak, timer_id : %d.", 1, timer_id);
    bt_ull_le_am_ul_info_t *ull_ul_am_info = &(g_bt_ull_le_am_info.ul_mode_ctx);
    bt_ull_le_am_ul_stop_audio(ull_ul_am_info);
}

extern void bt_ull_le_set_audio_manager_priority(audio_src_srv_priority_t priority);
void bt_ull_le_am_update_dl_priority(bool is_raise)
{
    ull_report("[ULL][LE][AM] bt_ull_le_am_update_dl_priority, is_raise_priority is %d", 1, is_raise);
    return is_raise ? bt_ull_le_set_audio_manager_priority(AUDIO_SRC_SRV_PRIORITY_HIGH) : bt_ull_le_set_audio_manager_priority(AUDIO_SRC_SRV_PRIORITY_NORMAL);
}

void bt_ull_le_am_set_sidetone_switch(bool is_sidetone_enable)
{
    bool is_uplink_port_streaming = bt_ull_le_srv_is_streaming(BT_ULL_LE_STREAM_MODE_UPLINK);
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    bt_ull_le_am_ul_info_t *ull_ul_am_info = &(am_ctx->ul_mode_ctx);

    ull_report("[ULL][LE][AM] bt_ull_le_am_set_sidetone_switch, is_sidetone_enable: %d, is_am_uplink_start: %d, is_uplink_port_streaming: %d", 3,
        is_sidetone_enable, ull_ul_am_info->is_transmitter_start, is_uplink_port_streaming);

    ull_ul_am_info->sidetone_enable = is_sidetone_enable;
    //ul is open before set sidetone status, just runtime config
    if(ull_ul_am_info->is_transmitter_start || is_uplink_port_streaming) {
        if (ull_ul_am_info->sidetone_enable) {
            am_audio_side_tone_enable_by_scenario(SIDETONE_SCENARIO_ULL_V2);
        } else {
            am_audio_side_tone_disable_by_scenario(SIDETONE_SCENARIO_ULL_V2);
        }
    }
}

void bt_ull_le_am_sync_sidetone_status(bool is_ul_port_streaming)
{
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    bt_ull_le_am_ul_info_t *ull_ul_am_info = &(am_ctx->ul_mode_ctx);    
    ull_report("[ULL][LE][AM] bt_ull_le_am_sync_sidetone_status, is_ul_port_streaming: %d, is_sidetone_enable: %d", 2, is_ul_port_streaming, ull_ul_am_info->sidetone_enable);
    if (is_ul_port_streaming) {
        if(ull_ul_am_info->sidetone_enable) {
            am_audio_side_tone_enable_by_scenario(SIDETONE_SCENARIO_ULL_V2);
        } else {
            am_audio_side_tone_disable_by_scenario(SIDETONE_SCENARIO_ULL_V2);
        }
    } else {
        am_audio_side_tone_disable_by_scenario(SIDETONE_SCENARIO_ULL_V2);
    }
}

void bt_ull_le_am_operate_waiting_list_for_a2dp_resync(bool add_waiting_list)
{
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    bt_ull_le_am_dl_info_t *ull_dl_am_info = &(am_ctx->dl_mode_ctx);
	ull_report("[ULL][LE][AM] a2dp resync handler, operate waiting list, is_reject: %d, add_waiting_list: %d", 2, ull_dl_am_info->is_reject, add_waiting_list);

    if (true == ull_dl_am_info->is_reject) {
        if (add_waiting_list) {
            bt_ull_le_am_add_waiting_list(BT_ULL_LE_AM_DL_MODE);
        } else {
            bt_ull_le_am_remove_waiting_list(BT_ULL_LE_AM_DL_MODE);
        }
    }
}

bool bt_ull_le_am_is_stop_ongoing(bt_ull_le_am_mode_t mode)
{
    bool ret = false;
    bt_ull_le_srv_am_info_t* am_ctx = bt_ull_le_get_audio_manager_contex();
    if (BT_ULL_LE_AM_UL_MODE == mode) {
        if (am_ctx->ul_mode_ctx.streaming_flag & BT_ULL_LE_AM_UL_STOPPING) {
            ret = true;
        }
    } else {
        if (BT_ULL_LE_SUB_STATE_CLEAR_CODEC == am_ctx->dl_mode_ctx.am_sub_state) {
            ret = true;
        }
    }
    ull_report("[ULL][LE][AM] bt_ull_le_am_is_stop_ongoing, ret : %d.", 1, ret);    
    return ret;
}

void bt_ull_le_am_send_message(bt_ull_le_am_message_data_t *message)
{
    TaskHandle_t xHandle;
    TaskStatus_t xTaskDetails;
    xHandle = xTaskGetCurrentTaskHandle();
    vTaskGetInfo(xHandle, &xTaskDetails, pdTRUE, eInvalid);
    if (0 != bt_ull_le_srv_memcmp(xTaskDetails.pcTaskName,"bt_ta", sizeof("bt_ta"))) {
        ull_report("[ULL][LE][AM]bt_ull_le_am_send_message in other task", 0);
        if (bt_ull_queue == NULL) {
            bt_ull_queue = xQueueCreate(BT_ULL_QUEUE_SIZE, BT_ULL_QUEUE_ITEM_SIZE);
        }

        if (bt_ull_queue) {
           ull_report("[ULL][LE][AM]bt_ull_queue get success", 0);
           xQueueSend(bt_ull_queue, message, 0);
           bt_os_layer_give_semaphore(bt_task_semaphore);
        }
    } else {
        ull_report("[ULL][LE][AM]bt_ull_le_am_send_message in bt_task", 0);
        bt_ull_le_am_handle_receive_message(message);
    }
}


void bt_ull_le_am_queue_handler(void)
{
   bt_ull_le_am_message_data_t data_t;
   while (xQueueReceive(bt_ull_queue, &data_t, 0) == pdPASS) {
       bt_ull_le_am_handle_receive_message(&data_t);
   }
}

void bt_ull_le_am_handle_receive_message(bt_ull_le_am_message_data_t *message) 
{
    ull_report("[ULL][LE][AM]bt_ull_le_am_handle_receive_message, event: 0x%02x", 1, message->event);

    switch (message->event) {
        case BT_ULL_LE_AM_DL_CALLBACK_IND: {
            bt_ull_le_am_dl_callback_ind_handle_message(message->aud_id, message->msg_id, message->sub_msg, message->parm);
            break;
        }
        case BT_ULL_LE_AM_DL_PLAY_IND: {
            bt_ull_le_am_dl_play_ind_handle_message(message->handle);
            break;
        }
        case BT_ULL_LE_AM_DL_STOP_IND: {
            bt_ull_le_am_dl_stop_ind_handle_message(message->handle);
            break;
        }
        case BT_ULL_LE_AM_DL_SUSPEND_IND: {
            bt_ull_le_am_dl_suspend_ind_handle_message(message->handle, message->int_hd);
            break;
        }
        case BT_ULL_LE_AM_DL_REJECT_IND: {
            bt_ull_le_am_dl_reject_ind_handle_message(message->handle);
            break;
        }
        case BT_ULL_LE_AM_DL_EXCEPTION_IND: {
            bt_ull_le_am_dl_exception_ind_handle_message(message->handle);
            break;
        }
        default:{
            break;
        }
    }
}



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

#include "voice_prompt_queue.h"
#include "voice_prompt_local.h"
#include "race_cmd_co_sys.h"
#include "voice_prompt_cosys.h"

#define LOG_TAG   "VP_COSYS"

#define VP_COSYS_FUNC_NONE 0
#define VP_COSYS_FUNC_PLAY 1
#define VP_COSYS_FUNC_UPDATE_CTRL 2
#define VP_COSYS_FUNC_STOP 3
#define VP_COSYS_FUNC_CLEAR 4
#define VP_COSYS_FUNC_PLAY_CALLBACK 0xff

typedef uint8_t func_type;
typedef struct {
    func_type type;
    uint8_t param[0];
} __attribute__((packed)) vp_race_cosys_remote_param;

typedef struct {
    uint32_t ref_index;
    voice_prompt_event_t err;
} __attribute__((packed)) vp_race_cosys_play_cb_param;

typedef struct {
    uint32_t ref_idx;
    voice_prompt_play_callback_t callback;
} vp_cosys_local_ctx_t;

typedef struct {
    uint16_t ref_idx;
    uint16_t play_id;
    uint32_t vp_index;
} vp_cosys_remote_ctx_t;

typedef struct {
    uint32_t                     vp_index;
    voice_prompt_play_callback_t callback;
    voice_prompt_control_mask_t  control;
    uint16_t                     ref_idx;
} vp_race_cosys_play_request;

typedef struct {
    uint32_t index;
    uint32_t ref_id;
} vp_race_cosys_stop_request;

static uint32_t vp_local_idx  = 0;
static uint32_t s_cur_vp_index = VOICE_PROMPT_VP_INDEX_INVALID;
#define VP_COSYS_MAX_CTX_NUMS 8
static vp_cosys_local_ctx_t s_cosys_local_ctxs[VP_COSYS_MAX_CTX_NUMS] = {{0}};
static vp_cosys_remote_ctx_t s_cosys_remote_ctxs[VP_COSYS_MAX_CTX_NUMS] = {{0}};

static vp_cosys_remote_ctx_t *__get_remote_ctxs_by_vp_idx(uint32_t vp_idx)
{
    for (uint32_t idx = 0; idx < VP_COSYS_MAX_CTX_NUMS; idx++) {
        if (s_cosys_remote_ctxs[idx].vp_index == vp_idx) {
            return &s_cosys_remote_ctxs[idx];
        }
    }
    VP_LOG_MSGID_E(LOG_TAG" __get_remote_ctxs_by_index fail", 0);
    return NULL;
}

static void vp_race_cosys_play_callback(uint32_t index, voice_prompt_event_t err)
{
    uint8_t *send_data = pvPortMalloc(sizeof(vp_race_cosys_remote_param) + sizeof(vp_race_cosys_play_cb_param));
    if (send_data == NULL) {
        VP_LOG_MSGID_E(LOG_TAG" vp_race_cosys_play_callback malloc fail", 0);
        return;
    }
    vp_race_cosys_remote_param *info = (vp_race_cosys_remote_param *)send_data;
    info->type = VP_COSYS_FUNC_PLAY_CALLBACK;
    vp_race_cosys_play_cb_param *cb_param = (vp_race_cosys_play_cb_param *)&info->param;
    memset(cb_param, 0, sizeof(vp_race_cosys_play_cb_param));

    for (uint32_t idx = 0; idx < VP_COSYS_MAX_CTX_NUMS; idx++) {
        if (s_cosys_remote_ctxs[idx].vp_index == index) {
            cb_param->ref_index = s_cosys_remote_ctxs[idx].ref_idx;
            if (err > VP_EVENT_START_PLAY || err == VP_EVENT_PLAY_END) {
                memset(&s_cosys_remote_ctxs[idx], 0, sizeof(vp_cosys_remote_ctx_t));
                //VP_LOG_MSGID_I(LOG_TAG" vp_race_cosys_play_callback clear idx=%d", 1, idx);
                bool ret = race_cosys_send_data(RACE_COSYS_MODULE_ID_VP, false, send_data, sizeof(vp_race_cosys_play_cb_param));
                VP_LOG_MSGID_I(LOG_TAG" vp_race_cosys_play_callback ret=%d", 1, ret);
                s_cur_vp_index = VOICE_PROMPT_VP_INDEX_INVALID;
            }
            break;
        }
    }

    vPortFree(send_data);
}

static void vp_race_cosys_data_callback_handler(bool from_irq, uint8_t *buff, uint32_t len)
{
    vp_race_cosys_remote_param *req = (vp_race_cosys_remote_param *)buff;
    VP_LOG_MSGID_I(LOG_TAG" vp_race_cosys_data_callback_handler type=%d", 1, req->type);
    switch (req->type) {
        case VP_COSYS_FUNC_PLAY: {
            vp_race_cosys_play_request *play_param = (vp_race_cosys_play_request *)&req->param;
            voice_prompt_param_t vp = {0};
            vp.callback = vp_race_cosys_play_callback;
            vp.vp_index = play_param->vp_index;
            vp.control = play_param->control;
            uint32_t idx = 0;
            for (; idx < VP_COSYS_MAX_CTX_NUMS; idx++) {
                if (s_cosys_remote_ctxs[idx].ref_idx == 0) {
                    s_cosys_remote_ctxs[idx].ref_idx = play_param->ref_idx;
                    s_cosys_remote_ctxs[idx].vp_index = play_param->vp_index;
                    break;
                }
            }
            if (idx >= VP_COSYS_MAX_CTX_NUMS) {
                //VP_LOG_MSGID_E(LOG_TAG" voice_prompt_cosys_call_remote no enough ctx for local ctx", 0);
                break;
            } else {
                VP_LOG_MSGID_I(LOG_TAG" vp_race_cosys_data_callback_handler use idx=%d", 1, idx);
            }
            voice_prompt_play(&vp, &s_cosys_remote_ctxs[idx].play_id);
            break;
        }
        case VP_COSYS_FUNC_STOP: {
            vp_race_cosys_stop_request *stop_req = (vp_race_cosys_stop_request *)&req->param;
            vp_cosys_remote_ctx_t *t_ctx = __get_remote_ctxs_by_vp_idx(stop_req->index);
            if (t_ctx == NULL) {
                break;
            }
            /* TODO: check that there is callback of stop. */
            voice_prompt_stop(t_ctx->vp_index, t_ctx->play_id, false);
            break;
        }
        case VP_COSYS_FUNC_CLEAR: {
            voice_prompt_clear_all();
            for (uint32_t idx = 0; idx < VP_COSYS_MAX_CTX_NUMS; idx++) {
                s_cosys_remote_ctxs[idx].ref_idx = 0;
            }
            break;
        }

        case VP_COSYS_FUNC_PLAY_CALLBACK: {
            vp_race_cosys_play_cb_param *cb_param = (vp_race_cosys_play_cb_param *)&req->param;
            uint32_t idx = 0;
            for (idx = 0; idx < VP_COSYS_MAX_CTX_NUMS; idx++) {
                if (s_cosys_local_ctxs[idx].ref_idx == cb_param->ref_index) {
                    if (s_cosys_local_ctxs[idx].callback != NULL) {
                        s_cosys_local_ctxs[idx].callback(cb_param->ref_index, cb_param->err);
                    }
                    if (cb_param->err > VP_EVENT_START_PLAY || cb_param->err == VP_EVENT_PLAY_END) {
                        memset(&s_cosys_local_ctxs[idx], 0, sizeof(vp_cosys_local_ctx_t));
                        VP_LOG_MSGID_I(LOG_TAG" vp_race_cosys_data_callback_handler clear idx=%d", 1, idx);
                    }
                    break;
                }
            }
            if (idx >= VP_COSYS_MAX_CTX_NUMS) {
                VP_LOG_MSGID_E(LOG_TAG" voice_prompt_cosys_call_remote no ctx found", 0);
            }
            break;
        }
    }
}

static bool voice_prompt_cosys_call_remote(func_type type, void *param, uint8_t param_len)
{
    uint32_t send_len = param_len + sizeof(vp_race_cosys_remote_param);
    uint8_t *send_data = pvPortMalloc(send_len);
    if (send_data == NULL) {
        VP_LOG_MSGID_E(LOG_TAG" voice_prompt_cosys_send_to_remote malloc fail", 0);
        return false;
    }
    vp_race_cosys_remote_param *info = (vp_race_cosys_remote_param *)send_data;
    info->type = type;
    memcpy(&info->param, param, param_len);
    bool ret = race_cosys_send_data(RACE_COSYS_MODULE_ID_VP, false, send_data, send_len);
    VP_LOG_MSGID_I(LOG_TAG" voice_prompt_cosys_call_remote ret=%d", 1, ret);

    if (type == VP_COSYS_FUNC_PLAY) {
        vp_race_cosys_play_request *req = (vp_race_cosys_play_request *)param;
        for (uint32_t idx = 0; idx < VP_COSYS_MAX_CTX_NUMS; idx++) {
            if (s_cosys_local_ctxs[idx].ref_idx == 0) {
                s_cosys_local_ctxs[idx].callback = req->callback;
                s_cosys_local_ctxs[idx].ref_idx = req->ref_idx;
                VP_LOG_MSGID_I(LOG_TAG"voice_prompt_cosys_call_remote use idx=%d", 1, idx);
                break;
            }
        }
    }

    vPortFree(send_data);
    return ret;
}

static uint16_t voice_prompt_cosys_gen_ref_idex()
{
    vp_local_idx++;
    if (vp_local_idx == 0) {
        vp_local_idx++;
    }

    return vp_local_idx;
}

uint32_t voice_prompt_cosys_get_cur_index()
{
    //return vp_local_idx;
    return s_cur_vp_index;
}

bool voice_prompt_cosys_remote_play(voice_prompt_param_t *vp, uint16_t *play_id)
{
    vp_race_cosys_play_request play_req = {
        .vp_index = vp->vp_index,
        .callback = vp->callback,
        .control  = vp->control,
        .ref_idx  = 0,
    };

    if (play_id != NULL) {
        *play_id = voice_prompt_cosys_gen_ref_idex();
        play_req.ref_idx = *play_id;
    } else {
        play_req.ref_idx = voice_prompt_cosys_gen_ref_idex();
    }

    s_cur_vp_index = vp->vp_index;
    return voice_prompt_cosys_call_remote(VP_COSYS_FUNC_PLAY, &play_req, sizeof(vp_race_cosys_play_request));
}

bool voice_prompt_cosys_remote_stop(uint32_t vp_index, uint16_t id)
{
    vp_race_cosys_stop_request stop_req = {
        .index = vp_index,
        .ref_id = id
    };
    return voice_prompt_cosys_call_remote(VP_COSYS_FUNC_STOP, &stop_req, sizeof(vp_race_cosys_stop_request));
}

bool voice_prompt_cosys_remote_clear_all(void)
{
    return voice_prompt_cosys_call_remote(VP_COSYS_FUNC_CLEAR, NULL, 0);
}

void voice_prompt_cosys_init()
{
    bool ret = race_cosys_register_data_callback(RACE_COSYS_MODULE_ID_VP, vp_race_cosys_data_callback_handler);
    VP_LOG_MSGID_I(LOG_TAG" voice_prompt_cosys_init ret=%d", 1, ret);
}


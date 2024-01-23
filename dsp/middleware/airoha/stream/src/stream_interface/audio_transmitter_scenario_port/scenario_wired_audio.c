/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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

#if defined (AIR_WIRED_AUDIO_ENABLE)

#include "scenario_audio_common.h"
#include "scenario_wired_audio.h"
#include "dsp_dump.h"
#include "dsp_share_memory.h"
#include "preloader_pisplit.h"

#ifdef AIR_SOFTWARE_DRC_ENABLE
#include "compander_interface_sw.h"
#endif

#ifdef AIR_DCHS_MODE_ENABLE
#include "stream_dchs.h"
#endif

#ifdef AIR_MUTE_MIC_DETECTION_ENABLE
#include "volume_estimator_interface.h"
#include "stream_nvkey_struct.h"
#include "preloader_pisplit.h"
#include "audio_nvdm_common.h"
#endif
#define WIRED_AUDIO_DEBUG_LOG

#ifdef AIR_BTA_IC_PREMIUM_G2
#define WIRED_AUDIO_ATTR_TEXT_IN_IRAM ATTR_TEXT_IN_IRAM
#else
#define WIRED_AUDIO_ATTR_TEXT_IN_IRAM
#endif

#define UL_CLOCK_SKEW_CHECK_COUNT            (4)
#define UL_CLOCK_SKEW_WATERMARK              (UL_PROCESS_SAMPLE_RATE * UL_PROCESS_INTERVAL)
#define USB_IRQ_INTERVAL                     1000     /* unit of us */

#define WIRED_AUDIO_USB_IN_PREFILL_TIME     1000  /* Unit of USB frame number, 1000 means 1 USB frame */
#define WIRED_AUDIO_USB_IN_THRESHOLD_NUMBER 1000  /* Unit of USB frame number, 1000 means 1 USB frame */
#define WIRED_AUDIO_USB_IN_CLKSKEW_CHECK_NUMBER 10

#define WIRED_AUDIO_MAX_SUPPORT_FIX_SRX_RATIO_CNT 3
#define WIRED_AUDIO_MAX_SUPPORT_INPUT_SAMPLE_RATE_CNT 6
#define WIRED_AUDIO_MAX_SUPPORT_OUTPUT_SAMPLE_RATE_CNT 3

/* Define the max time of USB out process, with unit of us.
 * It should be multiple of 0.5ms to reduce the risk of threshold check.
 */
#define WIRED_AUDIO_USB_OUT_PROCESS_THRESHOLD 5000

#define WIRED_AUDIO_LOG_MSGID_I(fmt, arg...)    LOG_MSGID_I(wired_audio, "[wired_audio]: "fmt, ##arg)
#define WIRED_AUDIO_LOG_MSGID_W(fmt, arg...)    LOG_MSGID_W(wired_audio, "[wired_audio]: "fmt, ##arg)
#define WIRED_AUDIO_LOG_MSGID_E(fmt, arg...)    LOG_MSGID_E(wired_audio, "[wired_audio]: "fmt, ##arg)
#define WIRED_AUDIO_DEBUG_LOG_MSGID_I(fmt, arg...)    LOG_MSGID_I(wired_audio_debug, "[wired_audio]: "fmt, ##arg)
#define WIRED_AUDIO_DEBUG_LOG_MSGID_W(fmt, arg...)    LOG_MSGID_W(wired_audio_debug, "[wired_audio]: "fmt, ##arg)
#define WIRED_AUDIO_DEBUG_LOG_MSGID_E(fmt, arg...)    LOG_MSGID_E(wired_audio_debug, "[wired_audio]: "fmt, ##arg)


#define SUB_STREAM_MIXER_MEMBER 3 /*usb in0, usb in1, usb out*/
log_create_module(wired_audio, PRINT_LEVEL_INFO);
log_create_module_variant(wired_audio_debug, DEBUG_LOG_OFF, PRINT_LEVEL_INFO);

#ifndef min
#define min(_a, _b)   (((_a)<(_b))?(_a):(_b))
#endif

#ifdef AIR_LD_NR_ENABLE
void *p_wired_audio_usb_out_ld_nr_key;
#endif

static wired_audio_handle_t *g_wired_audio_first_dl_handle = NULL;

bool g_wired_audio_main_stream_is_triggered;
static SOURCE g_wired_audio_main_stream_source = NULL;
sw_mixer_member_t *g_main_stream_mixer_member = NULL;
sw_mixer_member_t *g_sub_stream_mixer_member[SUB_STREAM_MIXER_MEMBER] = {NULL};

sw_mixer_member_t *g_usb_in_out_mixer_member;
static sw_src_port_t *g_usb_in_out_src_port;
static sw_src_config_t g_usb_in_out_sw_src_config;
uint32_t g_usb_in_out_pre_sampling_rate = WIRED_AUDIO_MIXING_MODE_USB_DL_OUT_SAMPLE_RATE;
sw_gain_port_t *g_usb_in_out_sw_gain_port;
copy_to_virtual_source_port_t *g_usb_in_out_copy_to_virtual_source_port;
SOURCE g_usb_in_out_source = NULL;

#ifdef AIR_MUTE_MIC_DETECTION_ENABLE
void *p_usb_out_vol_estimator_nvkey_buf = NULL;
volume_estimator_port_t *p_usb_out_meter_port;
extern bool g_call_mute_flag;
#endif

#if 0
#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
static wired_audio_usb_vol_balance_buffer_msg_t wired_audio_usb_vol_buffer_msg;
static wired_audio_usb_vol_balance_handle_t wired_audio_usb_vol_balance_handle;
#endif
#endif

int8_t usb_audio_start_number = 0;
CONNECTION_IF *g_application_ptr_usb_in_0 = NULL;
CONNECTION_IF *g_application_ptr_usb_in_1 = NULL;

extern stream_feature_list_t stream_feature_list_wired_audio_usb_in_0_seperate_peq[];
extern stream_feature_list_t stream_feature_list_wired_audio_usb_in_0_passthrough[];
extern stream_feature_list_t stream_feature_list_wired_audio_usb_in_1_seperate_peq[];
extern stream_feature_list_t stream_feature_list_wired_audio_usb_out[];
extern stream_feature_list_t stream_feature_list_wired_audio_usb_out_swb[];
extern stream_feature_list_t stream_feature_list_wired_audio_line_out[];
extern stream_feature_list_t stream_feature_list_wired_audio_line_out_swb[];
extern CONNECTION_IF g_usb_out_local_streams;
extern CONNECTION_IF g_line_out_local_streams;

static void wired_audio_main_stream_start(mcu2dsp_start_param_p start_param, SOURCE source, SINK sink);
void wired_audio_main_stream_trigger_active(void);

extern void afe_audio_init_input_channel(void *owner, uint32_t channel_number, DSP_STREAMING_PARA_PTR stream_ptr);
extern afe_input_digital_gain_t afe_audio_get_input_channel(void *owner, uint32_t index);
extern void afe_audio_deinit_input_channel(void *owner);
extern ATTR_TEXT_IN_IRAM_LEVEL_1 VOID audio_transmitter_share_information_update_write_offset(SINK sink, U32 WriteOffset);
extern ATTR_TEXT_IN_IRAM_LEVEL_1 VOID audio_transmitter_share_information_update_read_offset(SOURCE source, U32 ReadOffset);
extern ATTR_TEXT_IN_IRAM_LEVEL_1 VOID audio_transmitter_share_information_update_read_offset_for_sink(SINK sink, U32 ReadOffset);
extern ATTR_TEXT_IN_IRAM_LEVEL_1 VOID audio_transmitter_share_information_fetch(SOURCE source, SINK sink);
extern hal_nvic_status_t hal_nvic_save_and_set_interrupt_mask_special(uint32_t *mask);
extern hal_nvic_status_t hal_nvic_restore_interrupt_mask_special(uint32_t mask);

static uint32_t g_wired_audio_sw_src_in_sample_rate[WIRED_AUDIO_MAX_SUPPORT_INPUT_SAMPLE_RATE_CNT] = {
    16000,
    24000,
    32000,
    48000,
    96000,
    192000
};

static uint32_t g_wired_audio_sw_src_out_sample_rate[WIRED_AUDIO_MAX_SUPPORT_OUTPUT_SAMPLE_RATE_CNT] = {
    48000,
    96000,
    192000
};

static const int32_t g_wired_audio_sw_src_ratio[WIRED_AUDIO_MAX_SUPPORT_INPUT_SAMPLE_RATE_CNT][WIRED_AUDIO_MAX_SUPPORT_OUTPUT_SAMPLE_RATE_CNT][WIRED_AUDIO_MAX_SUPPORT_FIX_SRX_RATIO_CNT] = {
    {   /* 16K */
        {3, 0, 0}, /* 48K */
        {3, 2, 0}, /* 96K */
        {3, 2, 2}, /* 192K */
    },
    {   /* 24K */
        {2, 0, 0}, /* 48K */
        {2, 2, 0}, /* 96K */
        {2, 2, 2}, /* 192K */
    },
    {   /* 32K */
        {3, -2, 0}, /* 48K */
        {3, 0, 0}, /* 96K */
        {3, 2, 0}, /* 192K */
    },
    {   /* 48K */
        {1, 0, 0}, /* 48K */
        {2, 0, 0}, /* 48K */
        {2, 2, 0}, /* 192K */
    },
    {   /* 96K */
        {-2, 0, 0}, /* 48K */
        {1, 0, 0}, /* 96K */
        {2, 0, 0}, /* 192K */
    },
    {   /* 192K */
        {-2, -2, 0}, /* 48K */
        {-2, 0, 0}, /* 96K */
        {1, 0, 0}, /* 192K */
    },
};

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static wired_audio_handle_t *wired_audio_alloc_handle(void *owner)
{
    uint32_t saved_mask;
    int16_t count;
    wired_audio_handle_t *dongle_handle = NULL;
    wired_audio_handle_t *c_handle = NULL;

    dongle_handle = malloc(sizeof(wired_audio_handle_t));
    if (dongle_handle == NULL) {
        AUDIO_ASSERT(0);
    }
    memset(dongle_handle, 0, sizeof(wired_audio_handle_t));

    dongle_handle->total_number = -1;
    dongle_handle->index = -1;
    dongle_handle->owner = owner;
    dongle_handle->next_dl_handle = NULL;

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    if (g_wired_audio_first_dl_handle == NULL) {
        /* there is no item on the handle list */
        dongle_handle->total_number = 1;
        dongle_handle->index = 1;

        /* set this handle as the head of the handle list */
        g_wired_audio_first_dl_handle = dongle_handle;
    } else {
        /* there are other items on the handle list */
        count = 1;
        c_handle = g_wired_audio_first_dl_handle;
        while (c_handle != NULL) {
            count += 1;

            c_handle->total_number += 1;
            if (c_handle->total_number <= 0) {
                /* error status */
                AUDIO_ASSERT(0);
            }

            if (c_handle->next_dl_handle == NULL) {
                /* c_handle is the last item on the list now */
                dongle_handle->total_number = c_handle->total_number;
                dongle_handle->index = c_handle->total_number;

                /* dongle_handle is the last item on the list now */
                c_handle->next_dl_handle = dongle_handle;

                break;
            }

            c_handle = c_handle->next_dl_handle;
        }
        if ((c_handle == NULL) || (dongle_handle->total_number != count)) {
            AUDIO_ASSERT(0);
        }
    }

    hal_nvic_restore_interrupt_mask(saved_mask);

    return dongle_handle;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void wired_audio_release_handle(wired_audio_handle_t *handle)
{
    uint32_t saved_mask, i, count;
    wired_audio_handle_t *l_handle = NULL;
    wired_audio_handle_t *c_handle = NULL;
    wired_audio_handle_t *dongle_handle = NULL;

    if ((handle == NULL) || (g_wired_audio_first_dl_handle == NULL)) {
        AUDIO_ASSERT(0);
    }

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    if (g_wired_audio_first_dl_handle->total_number <= 0) {
        /* error status */
        AUDIO_ASSERT(0);
    } else if ((g_wired_audio_first_dl_handle->total_number == 1) &&
               (g_wired_audio_first_dl_handle == handle)) {
        /* this handle is only one item on handle list */
        if (g_wired_audio_first_dl_handle->next_dl_handle != NULL) {
            AUDIO_ASSERT(0);
        }

        /* there is no item on the handle list */
        g_wired_audio_first_dl_handle = NULL;
    } else if ((g_wired_audio_first_dl_handle->total_number > 1) &&
               (g_wired_audio_first_dl_handle == handle)) {
        /* this handle is the first item on handle list, but there are other handles on the list */
        c_handle = g_wired_audio_first_dl_handle;
        count = g_wired_audio_first_dl_handle->total_number;
        for (i = 0; i < count; i++) {
            c_handle->total_number -= 1;
            c_handle->index -= 1;

            c_handle = c_handle->next_dl_handle;
        }
        if (c_handle != NULL) {
            /* error status */
            AUDIO_ASSERT(0);
        }

        /* change the next iten to the head */
        g_wired_audio_first_dl_handle = handle->next_dl_handle;
    } else {
        /* this handle is the middle item on handle list */
        c_handle = g_wired_audio_first_dl_handle;
        count = g_wired_audio_first_dl_handle->total_number;
        if (count == 1) {
            /* error status */
            AUDIO_ASSERT(0);
        }
        for (i = 0; i < count; i++) {
            if (c_handle == handle) {
                /* find out the handle on the list */
                dongle_handle = handle;
                l_handle->next_dl_handle = c_handle->next_dl_handle;
            }

            if (dongle_handle == NULL) {
                c_handle->total_number -= 1;
            } else {
                c_handle->total_number -= 1;
                c_handle->index -= 1;
            }

            l_handle = c_handle;
            c_handle = c_handle->next_dl_handle;
        }
        if ((c_handle != NULL) || (dongle_handle == NULL)) {
            /* error status */
            AUDIO_ASSERT(0);
        }
    }

    hal_nvic_restore_interrupt_mask(saved_mask);

    free(handle);
}

// int8_t wired_audio_get_usb_audio_start_number(void)
// {
//     return usb_audio_start_number;
// }
// void wired_audio_add_usb_audio_start_number(void)
// {
//     usb_audio_start_number++;
// }
// void wired_audio_minus_usb_audio_start_number(void)
// {
//     usb_audio_start_number--;
// }

#ifdef AIR_SOFTWARE_MIXER_ENABLE
WIRED_AUDIO_ATTR_TEXT_IN_IRAM void wired_audio_main_stream_mixer_postcallback(sw_mixer_member_t *member, void *para, uint32_t *out_frame_size)
{
    UNUSED(member);
    UNUSED(para);

    int32_t i;
    //wired_audio_handle_t *c_handle = NULL;
    if ((*out_frame_size != 0)) {
        for (i = 0; i < SUB_STREAM_MIXER_MEMBER; i++) {
            if(g_sub_stream_mixer_member[i] != NULL) {
                //WIRED_AUDIO_LOG_MSGID_I("[main_stream_mixer_postcallback]g_sub_stream_mixer_member 0x%x",1,g_sub_stream_mixer_member[i]);
                stream_function_sw_mixer_member_input_buffers_clean(g_sub_stream_mixer_member[i], false);
            }
        }
        stream_function_sw_mixer_member_input_buffers_clean(g_main_stream_mixer_member, false);
    }
}
#endif /* AIR_SOFTWARE_MIXER_ENABLE */

#if 0
#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
void wired_audio_usb_in_vol_balance_init(SOURCE source)
{
    wired_audio_handle_t *handle;

    handle = (wired_audio_handle_t *)(source->param.data_dl.scenario_param.usb_in_local_param.usb_in_param.handle);
    if (source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) {
        wired_audio_usb_vol_balance_handle.chat_path_default_vol_level.gain[0] = handle->gain[0];
        wired_audio_usb_vol_balance_handle.chat_path_default_vol_level.gain[1] = handle->gain[1];
        DTM_enqueue(DTM_EVENT_ID_WIRED_AUDIO_USB_VOLUME_SMART_BALANCE_ENABLE, (uint32_t)source, false);
    } else if (source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1) {
        wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[0] = handle->gain[0];
        wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[1] = handle->gain[1];
        wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[0] = handle->gain[0];
        wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[1] = handle->gain[1];
        wired_audio_usb_vol_balance_handle.game_path_gain_port = handle->gain_port;
    }
}

void wired_audio_usb_in_vol_balance_deinit(SOURCE source)
{
    if (source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) {
        DTM_enqueue(DTM_EVENT_ID_WIRED_AUDIO_USB_VOLUME_SMART_BALANCE_DISABLE, 0, false);
    } else if (source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1) {
        wired_audio_usb_vol_balance_handle.game_path_gain_port = NULL;
    }
}

void wired_audio_usb_in_volume_smart_balance_enable(void *msg)
{
    uint32_t saved_mask;
    void *owner = msg;
    volume_estimator_port_t *port = NULL;
    volume_estimator_config_t config;
    void *data_buf;
    void *nvkey_buf;

    nvkey_buf = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, sizeof(wired_audio_usb_vol_balance_nvkey_t));
    if (nvkey_buf == NULL) {
        AUDIO_ASSERT(0);
    }
    if (nvkey_read_full_key(NVKEY_DSP_PARA_GAME_CHAT_VOLUME_SMART_BALANCE, nvkey_buf, sizeof(wired_audio_usb_vol_balance_nvkey_t)) != NVDM_STATUS_NAT_OK) {
        AUDIO_ASSERT(0);
    }

    /* get chat volume estimator port */
    port = volume_estimator_get_port(owner);
    if (port == NULL) {
        AUDIO_ASSERT(0);
    }

    /* init chat volume estimator port */
    config.channel_num = 2;
    config.frame_size = 480; /* 2.5ms*48K*stereo*16bit */
    config.resolution = RESOLUTION_16BIT;
    config.mode = VOLUME_ESTIMATOR_CHAT_INSTANT_MODE;
    config.sample_rate = 48000;
    config.nvkey_para = (void *) & (((wired_audio_usb_vol_balance_nvkey_t *)nvkey_buf)->chat_vol_nvkey);
    volume_estimator_init(port, &config);

    /* malloc memory for queue stereo pcm data */
    data_buf = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, 480);
    if (data_buf == NULL) {
        AUDIO_ASSERT(0);
    }

    /* update state machine */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (wired_audio_usb_vol_balance_handle.port != NULL) {
        AUDIO_ASSERT(0);
    }
    wired_audio_usb_vol_balance_handle.port = port;
    wired_audio_usb_vol_balance_handle.nvkey_buf = nvkey_buf;
    wired_audio_usb_vol_balance_handle.enable = (int16_t)(((wired_audio_usb_vol_balance_nvkey_t *)nvkey_buf)->enable);
    wired_audio_usb_vol_balance_handle.gain_setting_update_done = 0;
    wired_audio_usb_vol_balance_handle.current_db = -144 * 100;
    wired_audio_usb_vol_balance_handle.effective_threshold_db = ((wired_audio_usb_vol_balance_nvkey_t *)nvkey_buf)->effective_threshold_db;
    wired_audio_usb_vol_balance_handle.effective_delay_ms = ((wired_audio_usb_vol_balance_nvkey_t *)nvkey_buf)->effective_delay_ms;
    wired_audio_usb_vol_balance_handle.effective_delay_us_count = 0;
    wired_audio_usb_vol_balance_handle.failure_threshold_db = ((wired_audio_usb_vol_balance_nvkey_t *)nvkey_buf)->failure_threshold_db;
    wired_audio_usb_vol_balance_handle.failure_delay_ms = ((wired_audio_usb_vol_balance_nvkey_t *)nvkey_buf)->failure_delay_ms;
    wired_audio_usb_vol_balance_handle.failure_delay_us_count = 0;
    wired_audio_usb_vol_balance_handle.adjustment_amount_db = ((wired_audio_usb_vol_balance_nvkey_t *)nvkey_buf)->adjustment_amount_db;
    wired_audio_usb_vol_balance_handle.up_step_db = ((wired_audio_usb_vol_balance_nvkey_t *)nvkey_buf)->up_step_db;
    wired_audio_usb_vol_balance_handle.down_step_db = ((wired_audio_usb_vol_balance_nvkey_t *)nvkey_buf)->down_step_db;
    wired_audio_usb_vol_balance_handle.active_flag = false;
    wired_audio_usb_vol_balance_handle.data_size = 480;
    wired_audio_usb_vol_balance_handle.data_buf = data_buf;
    hal_nvic_restore_interrupt_mask(saved_mask);

    WIRED_AUDIO_LOG_MSGID_I("[Game/Chat balance][enable]:%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 16,
                 wired_audio_usb_vol_balance_handle.enable,
                 wired_audio_usb_vol_balance_handle.active_flag,
                 wired_audio_usb_vol_balance_handle.current_db,
                 wired_audio_usb_vol_balance_handle.effective_threshold_db,
                 wired_audio_usb_vol_balance_handle.effective_delay_ms,
                 wired_audio_usb_vol_balance_handle.failure_threshold_db,
                 wired_audio_usb_vol_balance_handle.failure_delay_ms,
                 wired_audio_usb_vol_balance_handle.adjustment_amount_db,
                 wired_audio_usb_vol_balance_handle.up_step_db,
                 wired_audio_usb_vol_balance_handle.down_step_db,
                 wired_audio_usb_vol_balance_handle.chat_path_default_vol_level.gain[0],
                 wired_audio_usb_vol_balance_handle.chat_path_default_vol_level.gain[1],
                 wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[0],
                 wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[1],
                 wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[0],
                 wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[1]);
}

void wired_audio_usb_in_volume_smart_balance_disable(void *msg)
{
    uint32_t saved_mask;
    volume_estimator_port_t *port;
    UNUSED(msg);

    /* check state machine */
    if ((wired_audio_usb_vol_balance_handle.port == NULL) || (wired_audio_usb_vol_balance_handle.data_buf == NULL) || (wired_audio_usb_vol_balance_handle.nvkey_buf == NULL) || (wired_audio_usb_vol_balance_handle.data_size != 480)) {
        WIRED_AUDIO_LOG_MSGID_I("[Game/Chat balance][disabel] error:%d,%d, %d, %d, ", 4,
                     wired_audio_usb_vol_balance_handle.port, wired_audio_usb_vol_balance_handle.data_buf, wired_audio_usb_vol_balance_handle.nvkey_buf, wired_audio_usb_vol_balance_handle.data_size);
        AUDIO_ASSERT(0);
    }
    port = wired_audio_usb_vol_balance_handle.port;

    /* deinit chat volume estimator port */
    volume_estimator_deinit(port);

    /* free memory */
    preloader_pisplit_free_memory(wired_audio_usb_vol_balance_handle.data_buf);
    preloader_pisplit_free_memory(wired_audio_usb_vol_balance_handle.nvkey_buf);

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if ((wired_audio_usb_vol_balance_handle.enable) && (wired_audio_usb_vol_balance_handle.game_path_gain_port != NULL)) {
        /* process L-channel */
        if (wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[0] != wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[0]) {
            /* recover gain to the default settings */
            stream_function_sw_gain_configure_gain_target(wired_audio_usb_vol_balance_handle.game_path_gain_port, 1, wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[0]);
            wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[0] = wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[0];
        }

        /* process R-channel */
        if (wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[1] != wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[1]) {
            /* recover gain to the default settings */
            stream_function_sw_gain_configure_gain_target(wired_audio_usb_vol_balance_handle.game_path_gain_port, 2, wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[1]);
            wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[1] = wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[1];
        }

        /* restore ramp-up and ramp-down setting */
        stream_function_sw_gain_configure_gain_up(wired_audio_usb_vol_balance_handle.game_path_gain_port,   1,  1, 2);
        stream_function_sw_gain_configure_gain_up(wired_audio_usb_vol_balance_handle.game_path_gain_port,   2,  1, 2);
        stream_function_sw_gain_configure_gain_down(wired_audio_usb_vol_balance_handle.game_path_gain_port, 1, -1, 2);
        stream_function_sw_gain_configure_gain_down(wired_audio_usb_vol_balance_handle.game_path_gain_port, 2, -1, 2);
    }

    /* reset state machine */
    wired_audio_usb_vol_balance_handle.port = NULL;
    wired_audio_usb_vol_balance_handle.nvkey_buf = NULL;
    wired_audio_usb_vol_balance_handle.enable = 0;
    wired_audio_usb_vol_balance_handle.gain_setting_update_done = 0;
    wired_audio_usb_vol_balance_handle.current_db = -144 * 100;
    wired_audio_usb_vol_balance_handle.effective_threshold_db = 0;
    wired_audio_usb_vol_balance_handle.effective_delay_ms = 0;
    wired_audio_usb_vol_balance_handle.effective_delay_us_count = 0;
    wired_audio_usb_vol_balance_handle.failure_threshold_db = 0;
    wired_audio_usb_vol_balance_handle.failure_delay_ms = 0;
    wired_audio_usb_vol_balance_handle.failure_delay_us_count = 0;
    wired_audio_usb_vol_balance_handle.adjustment_amount_db = 0;
    wired_audio_usb_vol_balance_handle.up_step_db = 0;
    wired_audio_usb_vol_balance_handle.down_step_db = 0;
    wired_audio_usb_vol_balance_handle.active_flag = 0;
    wired_audio_usb_vol_balance_handle.data_size = 0;
    wired_audio_usb_vol_balance_handle.data_buf = NULL;
    hal_nvic_restore_interrupt_mask(saved_mask);

    WIRED_AUDIO_LOG_MSGID_I("[Game/Chat balance][disable]:%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 16,
                 wired_audio_usb_vol_balance_handle.enable,
                 wired_audio_usb_vol_balance_handle.active_flag,
                 wired_audio_usb_vol_balance_handle.current_db,
                 wired_audio_usb_vol_balance_handle.effective_threshold_db,
                 wired_audio_usb_vol_balance_handle.effective_delay_ms,
                 wired_audio_usb_vol_balance_handle.failure_threshold_db,
                 wired_audio_usb_vol_balance_handle.failure_delay_ms,
                 wired_audio_usb_vol_balance_handle.adjustment_amount_db,
                 wired_audio_usb_vol_balance_handle.up_step_db,
                 wired_audio_usb_vol_balance_handle.down_step_db,
                 wired_audio_usb_vol_balance_handle.chat_path_default_vol_level.gain[0],
                 wired_audio_usb_vol_balance_handle.chat_path_default_vol_level.gain[1],
                 wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[0],
                 wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[1],
                 wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[0],
                 wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[1]);
}

void wired_audio_usb_in_volume_smart_balance_do_process(void *msg)
{
    uint32_t saved_mask;
    uint32_t samples;
    uint32_t remain_samples;
    volume_estimator_port_t *port;
    int32_t new_gain_1, new_gain_2;
    bool gain_change = false;
    wired_audio_usb_vol_balance_buffer_msg_t buf_msg;

    /* check state machine */
    if ((wired_audio_usb_vol_balance_handle.port == NULL) || (wired_audio_usb_vol_balance_handle.data_buf == NULL) || (wired_audio_usb_vol_balance_handle.nvkey_buf == NULL) || (wired_audio_usb_vol_balance_handle.data_size != 480)) {
        AUDIO_ASSERT(0);
    }
    port = wired_audio_usb_vol_balance_handle.port;

    /* check if there are new data on the chat path */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    buf_msg.data_buf_1_head = ((wired_audio_usb_vol_balance_buffer_msg_t *)msg)->data_buf_1_head;
    buf_msg.data_buf_1_tail = ((wired_audio_usb_vol_balance_buffer_msg_t *)msg)->data_buf_1_tail;
    buf_msg.data_buf_1_cur  = ((wired_audio_usb_vol_balance_buffer_msg_t *)msg)->data_buf_1_cur;
    buf_msg.data_buf_2_head = ((wired_audio_usb_vol_balance_buffer_msg_t *)msg)->data_buf_2_head;
    buf_msg.data_buf_2_tail = ((wired_audio_usb_vol_balance_buffer_msg_t *)msg)->data_buf_2_tail;
    buf_msg.data_buf_2_cur  = ((wired_audio_usb_vol_balance_buffer_msg_t *)msg)->data_buf_2_cur;
    hal_nvic_restore_interrupt_mask(saved_mask);

    for (size_t i = 0; i < 8; i++) {
        if ((buf_msg.data_buf_1_cur == NULL) && (buf_msg.data_buf_2_cur == NULL)) {
            /* there are not new data on the chat path */
            /* we will set zeros into internal buffer, and the wired_audio_usb_vol_balance_handle.current_db will be very low in 200ms later. Then we recover the volume on the game path */
            memset(wired_audio_usb_vol_balance_handle.data_buf, 0, wired_audio_usb_vol_balance_handle.data_size);
        } else {
            /* there are new data on the chat path */
            /* copy data into the internal buffer */
            /* buf_msg.data_buf_1_cur & buf_msg.data_buf_2_cur is pointer the frame end address */
            samples = 480 / 2 / sizeof(int16_t);
            if (((buf_msg.data_buf_1_cur - samples) < buf_msg.data_buf_1_head) || ((buf_msg.data_buf_2_cur - samples) < buf_msg.data_buf_2_head)) {
                /* wrapper case */
                remain_samples = buf_msg.data_buf_1_cur - buf_msg.data_buf_1_head;
                if (remain_samples > samples) {
                    AUDIO_ASSERT(0);
                }
                DSP_D2I_BufferCopy_16bit((U16 *)(wired_audio_usb_vol_balance_handle.data_buf),
                                         (buf_msg.data_buf_1_tail - (samples - remain_samples)),
                                         (buf_msg.data_buf_2_tail - (samples - remain_samples)),
                                         samples - remain_samples);
                DSP_D2I_BufferCopy_16bit((U16 *)(wired_audio_usb_vol_balance_handle.data_buf),
                                         (buf_msg.data_buf_1_head),
                                         (buf_msg.data_buf_2_head),
                                         remain_samples);
                buf_msg.data_buf_1_cur = buf_msg.data_buf_1_tail - (samples - remain_samples);
                buf_msg.data_buf_2_cur = buf_msg.data_buf_2_tail - (samples - remain_samples);
            } else {
                /* normal case */
                DSP_D2I_BufferCopy_16bit((U16 *)(wired_audio_usb_vol_balance_handle.data_buf),
                                         (buf_msg.data_buf_1_cur - samples),
                                         (buf_msg.data_buf_2_cur - samples),
                                         samples);
                buf_msg.data_buf_1_cur -= samples;
                buf_msg.data_buf_2_cur -= samples;
            }
        }

        /* update ramp-up/ramp-down settings at the first time */
        if ((wired_audio_usb_vol_balance_handle.enable) && (wired_audio_usb_vol_balance_handle.game_path_gain_port != NULL) && (wired_audio_usb_vol_balance_handle.gain_setting_update_done == 0)) {
            /* do ramp-up and ramp-down setting */
            stream_function_sw_gain_configure_gain_up(wired_audio_usb_vol_balance_handle.game_path_gain_port,   1, wired_audio_usb_vol_balance_handle.up_step_db,   2);
            stream_function_sw_gain_configure_gain_up(wired_audio_usb_vol_balance_handle.game_path_gain_port,   2, wired_audio_usb_vol_balance_handle.up_step_db,   2);
            stream_function_sw_gain_configure_gain_down(wired_audio_usb_vol_balance_handle.game_path_gain_port, 1, wired_audio_usb_vol_balance_handle.down_step_db, 2);
            stream_function_sw_gain_configure_gain_down(wired_audio_usb_vol_balance_handle.game_path_gain_port, 2, wired_audio_usb_vol_balance_handle.down_step_db, 2);

            /* set flag */
            wired_audio_usb_vol_balance_handle.gain_setting_update_done = 1;
        }

        /* get the latest volume on chat path and the current_db will be update every 200ms */
        if (volume_estimator_process(port, wired_audio_usb_vol_balance_handle.data_buf, 480, &wired_audio_usb_vol_balance_handle.current_db) != VOLUME_ESTIMATOR_STATUS_OK) {
            AUDIO_ASSERT(0);
        }
        //LOG_AUDIO_DUMP(wired_audio_usb_vol_balance_handle.data_buf, 480, AUDIO_CODEC_IN);

#ifdef WIRED_AUDIO_DEBUG_LOG
        WIRED_AUDIO_DEBUG_LOG_MSGID_I("[Game/Chat balance]volume_estimator_process %d, %d, %d, %d, %d, %d, %d, %d, %d, 0x%x, gain: %d, %d, %d, buf.msg:0x%x,0x%x", 15,
                     wired_audio_usb_vol_balance_handle.enable,
                     wired_audio_usb_vol_balance_handle.current_db,
                     wired_audio_usb_vol_balance_handle.effective_threshold_db,
                     wired_audio_usb_vol_balance_handle.effective_delay_us_count,
                     wired_audio_usb_vol_balance_handle.failure_threshold_db,
                     wired_audio_usb_vol_balance_handle.failure_delay_us_count,
                     wired_audio_usb_vol_balance_handle.adjustment_amount_db,
                     wired_audio_usb_vol_balance_handle.up_step_db,
                     wired_audio_usb_vol_balance_handle.down_step_db,
                     wired_audio_usb_vol_balance_handle.game_path_gain_port,
                     wired_audio_usb_vol_balance_handle.chat_path_default_vol_level.gain[0],
                     wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[0],
                     wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[0],
                     buf_msg.data_buf_1_cur,
                     buf_msg.data_buf_2_cur);
#endif /* WIRED_AUDIO_DEBUG_LOG */

        /* check if needs to do the balance */
        hal_nvic_save_and_set_interrupt_mask(&saved_mask);
        if ((wired_audio_usb_vol_balance_handle.enable) && (wired_audio_usb_vol_balance_handle.game_path_gain_port != NULL)) {
            if ((wired_audio_usb_vol_balance_handle.current_db >= wired_audio_usb_vol_balance_handle.effective_threshold_db) &&
                (wired_audio_usb_vol_balance_handle.effective_delay_us_count >= wired_audio_usb_vol_balance_handle.effective_delay_ms * 1000)) {
                /* there is chat voice on the chat path */
                /* process L-channel */
                new_gain_1 = (wired_audio_usb_vol_balance_handle.chat_path_default_vol_level.gain[0] + wired_audio_usb_vol_balance_handle.adjustment_amount_db);
                new_gain_1 = min(new_gain_1, wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[0]);
                if (wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[0] != new_gain_1) {
                    /* the default gain is larger than the target and the current gain is not equal to the target, so we need to ramp down the volume on the game path */
                    stream_function_sw_gain_configure_gain_target(wired_audio_usb_vol_balance_handle.game_path_gain_port, 1, new_gain_1);
                    wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[0] = new_gain_1;
                    gain_change = true;
                }

                /* process R-channel */
                new_gain_2 = (wired_audio_usb_vol_balance_handle.chat_path_default_vol_level.gain[1] + wired_audio_usb_vol_balance_handle.adjustment_amount_db);
                new_gain_2 = min(new_gain_2, wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[1]);
                if (wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[1] != new_gain_2) {
                    /* the default gain is larger than the target and the current gain is not equal to the target, so we need to ramp down the volume on the game path */
                    stream_function_sw_gain_configure_gain_target(wired_audio_usb_vol_balance_handle.game_path_gain_port, 2, new_gain_2);
                    wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[1] = new_gain_2;
                    gain_change = true;
                }

                /* set active flag */
                wired_audio_usb_vol_balance_handle.active_flag = true;

                /* reset failure_delay_us_count */
                wired_audio_usb_vol_balance_handle.failure_delay_us_count = 0;
            } else if ((wired_audio_usb_vol_balance_handle.current_db >= wired_audio_usb_vol_balance_handle.effective_threshold_db) &&
                       (wired_audio_usb_vol_balance_handle.effective_delay_us_count < wired_audio_usb_vol_balance_handle.effective_delay_ms * 1000)) {
                wired_audio_usb_vol_balance_handle.effective_delay_us_count += 2500;

                /* reset failure_delay_us_count */
                wired_audio_usb_vol_balance_handle.failure_delay_us_count = 0;
            } else if ((wired_audio_usb_vol_balance_handle.current_db <= wired_audio_usb_vol_balance_handle.failure_threshold_db) &&
                       (wired_audio_usb_vol_balance_handle.failure_delay_us_count >= wired_audio_usb_vol_balance_handle.failure_delay_ms * 1000)) {
                /* there is no chat voice on the chat path */
                /* process L-channel */
                if (wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[0] != wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[0]) {
                    /* recover gain to the default settings */
                    stream_function_sw_gain_configure_gain_target(wired_audio_usb_vol_balance_handle.game_path_gain_port, 1, wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[0]);
                    wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[0] = wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[0];
                    gain_change = true;
                }

                /* process R-channel */
                if (wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[1] != wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[1]) {
                    /* recover gain to the default settings */
                    stream_function_sw_gain_configure_gain_target(wired_audio_usb_vol_balance_handle.game_path_gain_port, 2, wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[1]);
                    wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[1] = wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[1];
                    gain_change = true;
                }

                /* set active flag */
                wired_audio_usb_vol_balance_handle.active_flag = false;

                /* reset effective_delay_us_count */
                wired_audio_usb_vol_balance_handle.effective_delay_us_count = 0;
            } else if ((wired_audio_usb_vol_balance_handle.current_db <= wired_audio_usb_vol_balance_handle.failure_threshold_db) &&
                       (wired_audio_usb_vol_balance_handle.failure_delay_us_count < wired_audio_usb_vol_balance_handle.failure_delay_ms * 1000)) {
                wired_audio_usb_vol_balance_handle.failure_delay_us_count += 2500;

                /* reset effective_delay_us_count */
                wired_audio_usb_vol_balance_handle.effective_delay_us_count = 0;

                /* process L-channel */
                if ((wired_audio_usb_vol_balance_handle.active_flag) && (wired_audio_usb_vol_balance_handle.chat_path_default_vol_level.gain[0] < wired_audio_usb_vol_balance_handle.effective_threshold_db)) {
                    /* In this stage, we should recover gain to the default settings because the chat gain is too less */
                    new_gain_1 = wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[0];
                    if (wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[0] != new_gain_1) {
                        stream_function_sw_gain_configure_gain_target(wired_audio_usb_vol_balance_handle.game_path_gain_port, 1, new_gain_1);
                        wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[0] = new_gain_1;
                        gain_change = true;
                    }
                } else if (wired_audio_usb_vol_balance_handle.active_flag) {
                    /* In this stage, we should set gain to the minimum in default gain and smart-target gain */
                    new_gain_1 = (wired_audio_usb_vol_balance_handle.chat_path_default_vol_level.gain[0] + wired_audio_usb_vol_balance_handle.adjustment_amount_db);
                    new_gain_1 = min(new_gain_1, wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[0]);
                    if (wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[0] != new_gain_1) {
                        /* set gain to the minimun settings */
                        stream_function_sw_gain_configure_gain_target(wired_audio_usb_vol_balance_handle.game_path_gain_port, 1, new_gain_1);
                        wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[0] = new_gain_1;
                        gain_change = true;
                    }
                } else {
                    if (wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[0] != wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[0]) {
                        /* recover gain to the default settings */
                        stream_function_sw_gain_configure_gain_target(wired_audio_usb_vol_balance_handle.game_path_gain_port, 1, wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[0]);
                        wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[0] = wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[0];
                        gain_change = true;
                    }
                }

                /* process R-channel */
                if ((wired_audio_usb_vol_balance_handle.active_flag) && (wired_audio_usb_vol_balance_handle.chat_path_default_vol_level.gain[1] < wired_audio_usb_vol_balance_handle.effective_threshold_db)) {
                    /* In this stage, we should recover gain to the default settings because the chat gain is too less */
                    new_gain_2 = wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[1];
                    if (wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[1] != new_gain_2) {
                        stream_function_sw_gain_configure_gain_target(wired_audio_usb_vol_balance_handle.game_path_gain_port, 2, new_gain_2);
                        wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[1] = new_gain_2;
                        gain_change = true;
                    }
                } else if (wired_audio_usb_vol_balance_handle.active_flag) {
                    new_gain_2 = (wired_audio_usb_vol_balance_handle.chat_path_default_vol_level.gain[1] + wired_audio_usb_vol_balance_handle.adjustment_amount_db);
                    new_gain_2 = min(new_gain_2, wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[1]);
                    if (wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[1] != new_gain_2) {
                        /* recover gain to the minimun settings */
                        stream_function_sw_gain_configure_gain_target(wired_audio_usb_vol_balance_handle.game_path_gain_port, 2, new_gain_2);
                        wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[1] = new_gain_2;
                        gain_change = true;
                    }
                } else {
                    if (wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[1] != wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[1]) {
                        /* recover gain to the default settings */
                        stream_function_sw_gain_configure_gain_target(wired_audio_usb_vol_balance_handle.game_path_gain_port, 2, wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[1]);
                        wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[1] = wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[1];
                        gain_change = true;
                    }
                }
            }
        }
        hal_nvic_restore_interrupt_mask(saved_mask);
    }
    if (gain_change) {
        WIRED_AUDIO_LOG_MSGID_I("[Game/Chat balance][change gain]:%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 13,
                     wired_audio_usb_vol_balance_handle.active_flag,
                     wired_audio_usb_vol_balance_handle.current_db,
                     wired_audio_usb_vol_balance_handle.effective_threshold_db,
                     wired_audio_usb_vol_balance_handle.effective_delay_us_count,
                     wired_audio_usb_vol_balance_handle.failure_threshold_db,
                     wired_audio_usb_vol_balance_handle.failure_delay_us_count,
                     wired_audio_usb_vol_balance_handle.adjustment_amount_db,
                     wired_audio_usb_vol_balance_handle.chat_path_default_vol_level.gain[0],
                     wired_audio_usb_vol_balance_handle.chat_path_default_vol_level.gain[1],
                     wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[0],
                     wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[1],
                     wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[0],
                     wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[1]);
    }
}
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */
#endif

#ifdef AIR_DCHS_MODE_ENABLE
static uint8_t *g_dchs_sink_afe_buffer = NULL;
#endif

static void wired_audio_usb_in_start(mcu2dsp_start_param_p start_param, SOURCE source, SINK sink)
{
    UNUSED(start_param);
    UNUSED(source);
    UNUSED(sink);

#ifdef AIR_DCHS_MODE_ENABLE
    /* Repalce the sink buffer here */
    uint8_t *p_sink_buffer;
    uint32_t i, dchs_buffer_size, dchs_buffer_number;
    usb_in_local_param_t *usb_in_param;
    uint32_t out_channel_num;

    usb_in_param = &source->param.data_dl.scenario_param.usb_in_local_param;
    out_channel_num = usb_in_param->usb_in_param.codec_param.pcm.channel_mode;
    dchs_buffer_size = sink->streamBuffer.BufferInfo.length;
    if (out_channel_num % 2) {
        dchs_buffer_number = out_channel_num / 2 + 1;
    } else {
        dchs_buffer_number = out_channel_num / 2;
    }
    p_sink_buffer = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, dchs_buffer_number * dchs_buffer_size);
    AUDIO_ASSERT(p_sink_buffer != NULL);
    memset(p_sink_buffer, 0, dchs_buffer_number * dchs_buffer_size);
    g_dchs_sink_afe_buffer = p_sink_buffer;

    for (i = 0; i < dchs_buffer_number; i++) {
        sink->streamBuffer.BufferInfo.startaddr[i] = p_sink_buffer;
        WIRED_AUDIO_LOG_MSGID_I("[config][start] DCHS mode, malloc the sink buffer %d, %d, %d", 3,
                                    i, out_channel_num, dchs_buffer_size);
        p_sink_buffer += dchs_buffer_size;
    }
    #ifdef AIR_DCHS_MODE_ENABLE
    #include "stream_mixer.h"
    source_ch_type_t ch_type = mixer_stream_get_source_ch_by_scenario(AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0);
    AUDIO_ASSERT(ch_type != MIX_SCENARIO_MAX && "don't get USB IN ch");
    if(mix_scenarios_msg[ch_type].sink_buf_info == NULL){
        mix_scenarios_msg[ch_type].sink_buf_info = &(sink->streamBuffer.BufferInfo);
        WIRED_AUDIO_LOG_MSGID_I("[DCHS DL] get usb buf info", 0);
        SOURCE mixer_source = Source_blks[SOURCE_TYPE_MIXER];
        if(mix_scenarios_msg[ch_type].sample_rate != mixer_source->param.audio.rate){
            mixer_stream_source_ch_hwsrc_cfg(ch_type, AFE_MEM_ASRC_1);
            mixer_stream_source_ch_hwsrc_driver_control(ch_type, true);
        }
    }
    #endif
#endif

    /* lock sleep because sleep wake-up time will consume the stream process time */
    //hal_sleep_manager_lock_sleep(SLEEP_LOCK_AUDIO);
}

static void wired_audio_usb_in_close(SOURCE source, SINK sink)
{
    UNUSED(source);
    UNUSED(sink);

#ifdef AIR_DCHS_MODE_ENABLE
    preloader_pisplit_free_memory(g_dchs_sink_afe_buffer);
    WIRED_AUDIO_LOG_MSGID_I("[config][start] DCHS mode, free the sink buffer.", 0);
#endif
}

#ifndef AIR_DCHS_MODE_ENABLE
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ static void wired_audio_usb_in_main_sub_stream_mixer_connect(wired_audio_handle_t *handle)
{
    uint32_t i;

    /* do connections */
    stream_function_sw_mixer_channel_connect(g_main_stream_mixer_member, 1, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, g_main_stream_mixer_member, 1);
    stream_function_sw_mixer_channel_connect(g_main_stream_mixer_member, 2, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, g_main_stream_mixer_member, 2);
    i = 0;
    while(g_sub_stream_mixer_member[i] != NULL){
        WIRED_AUDIO_LOG_MSGID_I("[transmitter][connect] mix member 0x%x.", 1,g_sub_stream_mixer_member[i]);
        stream_function_sw_mixer_channel_connect(g_sub_stream_mixer_member[i], 1, SW_MIXER_CHANNEL_ATTRIBUTE_FORCED_WAIT, g_main_stream_mixer_member, 1);
        stream_function_sw_mixer_channel_connect(g_sub_stream_mixer_member[i], 2, SW_MIXER_CHANNEL_ATTRIBUTE_FORCED_WAIT, g_main_stream_mixer_member, 2);
        i++;
    }

    /* update mixer status */
    //dongle_handle_0->mixer_status = WIRED_AUDIO_STREAM_MIXER_MIX;
    handle->mixer_status = WIRED_AUDIO_STREAM_MIXER_MIX;
}
#endif

void wired_audio_usb_in_source_init(SOURCE source, audio_transmitter_open_param_p open_param, AUDIO_TRANSMITTER_PARAMETER *source_param)
{
    stream_resolution_t resolution;
    usb_in_local_param_t *usb_in_param;
    wired_audio_handle_t *handle;
    audio_codec_pcm_t *codec_pcm;
    uint32_t i, sample_byte, sample_bits;
    uint32_t dsp_frame_sample, src_dsp_frame_sample, out_channel_num;
    sw_gain_config_t default_config;
    sw_clk_skew_config_t sw_clk_skew_config;
    sw_buffer_config_t buffer_config;

    UNUSED(source_param);
    UNUSED(src_dsp_frame_sample);

    if ((open_param->scenario_sub_id != AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) &&
        (open_param->scenario_sub_id != AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1)) {
        WIRED_AUDIO_LOG_MSGID_E("[USB_IN][source_init]: type %d, sub id invalid %d", 2, open_param->scenario_type, open_param->scenario_sub_id);
        AUDIO_ASSERT(0);
    }

    usb_in_param = &source->param.data_dl.scenario_param.usb_in_local_param;
    handle = wired_audio_alloc_handle(source);
    usb_in_param->usb_in_param.handle = (void *)handle;
    codec_pcm = &(usb_in_param->usb_in_param.codec_param.pcm);

    usb_in_param->is_use_afe_dl3 = true;
    usb_in_param->usb_in_param.codec_type = open_param->scenario_param.wired_audio_param.codec_type;
    memcpy(&usb_in_param->usb_in_param.codec_param.pcm, &open_param->scenario_param.wired_audio_param.codec_param.pcm, sizeof(audio_codec_pcm_t));

    memcpy(handle->gain, open_param->scenario_param.wired_audio_param.gain, 8 * sizeof(int32_t));
    handle->compen_count = 0;
    handle->compen_flag = 0;
    handle->total_compen_samples = 0;
    handle->compen_method = WIRED_AUDIO_COMPENSATORY_METHOD_DISABLE;

    dsp_frame_sample = (codec_pcm->sample_rate * codec_pcm->frame_interval) / (1000 * 1000);
    out_channel_num = codec_pcm->channel_mode;
    sample_bits = 32;
    sample_byte = 4;
    resolution = RESOLUTION_32BIT;

#if defined(AIR_DCHS_MODE_ENABLE)
    if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) {
        g_application_ptr_usb_in_0->pfeature_table = stream_feature_list_wired_audio_usb_in_0_passthrough;
        WIRED_AUDIO_LOG_MSGID_I("[USB_IN][source_init]: select feature list 0 with PASS THROUGH mode", 0);
    } else {
        AUDIO_ASSERT(0);
    }
#endif

    /* Must init it in the feature list before stream start */
    /* Config stream->callback.EntryPara.resolution.feature_res here */
    if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) {
        stream_feature_configure_resolution((stream_feature_list_ptr_t)g_application_ptr_usb_in_0->pfeature_table, resolution, CONFIG_DECODER);
    } else {
        stream_feature_configure_resolution((stream_feature_list_ptr_t)g_application_ptr_usb_in_1->pfeature_table, resolution, CONFIG_DECODER);
    }

    /* SW gain init */
    default_config.resolution = resolution;
    default_config.target_gain = handle->gain[0];
    // default_config.target_gain = 0;
    default_config.up_step = 1;
    default_config.up_samples_per_step = 2;
    default_config.down_step = -1;
    default_config.down_samples_per_step = 2;
    handle->gain_port = stream_function_sw_gain_get_port(source);
    WIRED_AUDIO_LOG_MSGID_I("[USB_IN][start] sw gain init: %d, %d, %d, %d, %d, %d, %d, %d", 8,
                                out_channel_num,
                                default_config.resolution,
                                handle->gain[0],
                                default_config.target_gain,
                                default_config.up_step,
                                default_config.up_samples_per_step,
                                default_config.down_step,
                                default_config.down_samples_per_step);
    stream_function_sw_gain_init(handle->gain_port, (uint16_t)out_channel_num, &default_config);
    for (i = 0; i < out_channel_num; i++) {
        WIRED_AUDIO_LOG_MSGID_I("[USB_IN][start] sw gain config for channel %d: %d*0.01dB", 2, i + 1, handle->gain[i]);
        stream_function_sw_gain_configure_gain_target(handle->gain_port, i + 1, handle->gain[i]);
    }

    /* SW clk skew init */
    handle->clk_skew_port = stream_function_sw_clk_skew_get_port(source);
    sw_clk_skew_config.channel = out_channel_num;
    sw_clk_skew_config.bits = sample_bits;
    if (resolution == RESOLUTION_32BIT) {
        sw_clk_skew_config.order = C_Flp_Ord_3;
    } else {
        sw_clk_skew_config.order = C_Flp_Ord_5;
    }
    sw_clk_skew_config.skew_io_mode = C_Skew_Inp;
    sw_clk_skew_config.skew_compensation_mode = SW_CLK_SKEW_COMPENSATION_1_SAMPLE_IN_8_FRAME;
    sw_clk_skew_config.skew_work_mode = SW_CLK_SKEW_CONTINUOUS;
    sw_clk_skew_config.continuous_frame_size = dsp_frame_sample * sample_byte;
    sw_clk_skew_config.max_output_size = sw_clk_skew_config.continuous_frame_size * sw_clk_skew_config.channel;
    WIRED_AUDIO_LOG_MSGID_I("[USB_IN][start] sw clkskew init: %d, %d, %d, %d, %d, %d, %d, %d, %d", 9,
                                handle->clk_skew_port,
                                sw_clk_skew_config.channel,
                                sw_clk_skew_config.bits,
                                sw_clk_skew_config.order,
                                sw_clk_skew_config.skew_io_mode,
                                sw_clk_skew_config.skew_compensation_mode,
                                sw_clk_skew_config.skew_work_mode,
                                sw_clk_skew_config.continuous_frame_size,
                                sw_clk_skew_config.max_output_size);
    stream_function_sw_clk_skew_init(handle->clk_skew_port, &sw_clk_skew_config);
    handle->clk_skew_compensation_mode = sw_clk_skew_config.skew_compensation_mode;

    /* SW buffer init */
    buffer_config.mode = SW_BUFFER_MODE_FIXED_OUTPUT_LENGTH;
    buffer_config.total_channels = out_channel_num;
    buffer_config.watermark_max_size = (dsp_frame_sample * sample_byte) * 4;
    buffer_config.watermark_min_size = 0;
    buffer_config.output_size = dsp_frame_sample * sample_byte;
    handle->buffer_port = stream_function_sw_buffer_get_port(source);
    WIRED_AUDIO_LOG_MSGID_I("[USB_IN][start] sw buffer init: %d, %d, %d, %d, %d, %d", 6,
                                handle->buffer_port,
                                buffer_config.mode,
                                buffer_config.total_channels,
                                buffer_config.watermark_max_size,
                                buffer_config.watermark_min_size,
                                buffer_config.output_size);
    stream_function_sw_buffer_init(handle->buffer_port, &buffer_config);

#ifndef AIR_DCHS_MODE_ENABLE
    /* SW src config */
    sw_src_config_t sw_src_config;
    AUDIO_ASSERT(handle->src1_port == NULL);
    sw_src_config.mode = SW_SRC_MODE_NORMAL;
    sw_src_config.channel_num = out_channel_num;
    sw_src_config.in_res = resolution;
    sw_src_config.in_sampling_rate  = codec_pcm->sample_rate;
    sw_src_config.in_frame_size_max = (sw_src_config.in_sampling_rate / 1000) * WIRED_AUDIO_USB_IN_PROCESS_PERIOD * sample_bits;
    sw_src_config.out_res = resolution;
    sw_src_config.out_sampling_rate = WIRED_AUDIO_MIXING_MODE_USB_DL_OUT_SAMPLE_RATE;
    sw_src_config.out_frame_size_max = (sw_src_config.out_sampling_rate / 1000) * WIRED_AUDIO_USB_IN_PROCESS_PERIOD * sample_bits;
    handle->src1_port = stream_function_sw_src_get_port(source);
    WIRED_AUDIO_LOG_MSGID_I("[USB_IN][start] sw src init: 0x%08x, %d, %d, %d, %d, %d, %d, %d, %d", 9,
                                handle->src1_port,
                                sw_src_config.mode,
                                sw_src_config.channel_num,
                                sw_src_config.in_res,
                                sw_src_config.in_sampling_rate,
                                sw_src_config.in_frame_size_max,
                                sw_src_config.out_res,
                                sw_src_config.out_sampling_rate,
                                sw_src_config.out_frame_size_max);
    stream_function_sw_src_init((sw_src_port_t *)handle->src1_port, &sw_src_config);

    /* SW mixer config, fix to 96K or 48K depend on WIRED_AUDIO_MIXING_MODE_USB_DL_OUT_SAMPLE_RATE */
    sw_mixer_callback_config_t       callback_config;
    sw_mixer_input_channel_config_t  in_ch_config;
    sw_mixer_output_channel_config_t out_ch_config;
    src_dsp_frame_sample = (WIRED_AUDIO_MIXING_MODE_USB_DL_OUT_SAMPLE_RATE * codec_pcm->frame_interval) / (1000 * 1000);

    callback_config.preprocess_callback = NULL;
    callback_config.postprocess_callback = NULL;//wired_audio_usb_in_mixer_postcallback;

    /* As PEQ out use only 32bit, so set sw mixer to 32bit */
    in_ch_config.total_channel_number = out_channel_num;
    in_ch_config.resolution = RESOLUTION_32BIT;
    in_ch_config.input_mode = SW_MIXER_CHANNEL_MODE_NORMAL;//SW_MIXER_CHANNEL_MODE_FORCED_ENOUGH;
    in_ch_config.buffer_size = sizeof(int32_t) * src_dsp_frame_sample; // 2.5ms * 48K
    out_ch_config.total_channel_number = in_ch_config.total_channel_number;
    out_ch_config.resolution = in_ch_config.resolution;
    stream_function_sw_mixer_init(SW_MIXER_PORT_0);

    WIRED_AUDIO_LOG_MSGID_I("[USB_IN][start] sw mixer init: %d, %d, %d, %d, %d, %d", 6,
                            in_ch_config.total_channel_number,
                            in_ch_config.resolution,
                            in_ch_config.input_mode,
                            in_ch_config.buffer_size,
                            out_ch_config.total_channel_number,
                            out_ch_config.resolution);

    // wired_audio_handle_t *dongle_handle_0;
    // wired_audio_handle_t *dongle_handle_1;

    handle->mixer_member = stream_function_sw_mixer_member_create((void *)source,
                                                                            SW_MIXER_MEMBER_MODE_BYPASS_FEATURES_SINK,
                                                                            &callback_config,
                                                                            &in_ch_config,
                                                                            &out_ch_config);
    for (i = 0; i < SUB_STREAM_MIXER_MEMBER; i++) {
        if(g_sub_stream_mixer_member[i] == NULL) {
            g_sub_stream_mixer_member[i] = handle->mixer_member;
            break;
        }
    }

    /* it is not the first dl stream, so we need to disconnect default connection */
    stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, handle->mixer_member, false);
    /* mix stream */
    //dongle_handle_0 = (wired_audio_handle_t *)(g_wired_audio_main_stream_source->param.data_dl.scenario_param.usb_in_local_param.usb_in_param.handle);
    //dongle_handle_1 = handle;

    wired_audio_usb_in_main_sub_stream_mixer_connect(handle);

    WIRED_AUDIO_LOG_MSGID_I("[config][start] mix done. \r\n", 0);

#if 0
#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
    wired_audio_usb_in_vol_balance_init(source);
#endif
#endif

#endif
}

void wired_audio_usb_in_out_source_init(SOURCE source)
{
    sw_mixer_callback_config_t       callback_config;
    sw_mixer_input_channel_config_t  in_ch_config;
    sw_mixer_output_channel_config_t out_ch_config;
    U8 i=0;

    g_usb_in_out_source = source;

    uint32_t src_dsp_frame_sample = (WIRED_AUDIO_MIXING_MODE_USB_DL_OUT_SAMPLE_RATE * WIRED_AUDIO_USB_IN_PROCESS_PERIOD) / (1000);
    stream_function_sw_mixer_init(SW_MIXER_PORT_0);

    callback_config.preprocess_callback = NULL;
    callback_config.postprocess_callback = NULL;

    /* As PEQ out use only 32bit, so set sw mixer to 32bit */
    in_ch_config.total_channel_number = 2;
    in_ch_config.resolution = RESOLUTION_32BIT;
    in_ch_config.input_mode = SW_MIXER_CHANNEL_MODE_NORMAL;//SW_MIXER_CHANNEL_MODE_FORCED_ENOUGH;
    in_ch_config.buffer_size = sizeof(int32_t) * src_dsp_frame_sample; // 2.5ms * 48K
    out_ch_config.total_channel_number = in_ch_config.total_channel_number;
    out_ch_config.resolution = in_ch_config.resolution;

    g_usb_in_out_mixer_member = stream_function_sw_mixer_member_create((void *)source,
                                                                        SW_MIXER_MEMBER_MODE_BYPASS_FEATURES_SINK,
                                                                        &callback_config,
                                                                        &in_ch_config,
                                                                        &out_ch_config);
    for (i = 0; i < SUB_STREAM_MIXER_MEMBER; i++) {
        if(g_sub_stream_mixer_member[i] == NULL) {
            g_sub_stream_mixer_member[i] =g_usb_in_out_mixer_member;
            break;
        }
    }

    stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, g_usb_in_out_mixer_member, true);
    stream_function_sw_mixer_channel_connect(g_main_stream_mixer_member, 1, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, g_main_stream_mixer_member, 1);
    stream_function_sw_mixer_channel_connect(g_main_stream_mixer_member, 2, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, g_main_stream_mixer_member, 2);
    i = 0;
    while(g_sub_stream_mixer_member[i] != NULL){
        WIRED_AUDIO_LOG_MSGID_I("[transmitter][connect] mix member 0x%x.", 1,g_sub_stream_mixer_member[i]);
        stream_function_sw_mixer_channel_connect(g_sub_stream_mixer_member[i], 1, SW_MIXER_CHANNEL_ATTRIBUTE_FORCED_WAIT, g_main_stream_mixer_member, 1);
        stream_function_sw_mixer_channel_connect(g_sub_stream_mixer_member[i], 2, SW_MIXER_CHANNEL_ATTRIBUTE_FORCED_WAIT, g_main_stream_mixer_member, 2);
        i++;
    }
    WIRED_AUDIO_LOG_MSGID_I("[USB_IN_OUT][config][start] mix. \r\n", 0);

    /* SW src config */
    sw_src_config_t sw_src_config;
    sw_src_config.mode = SW_SRC_MODE_NORMAL;
    sw_src_config.channel_num = 2;
    sw_src_config.in_res = RESOLUTION_32BIT;
    sw_src_config.in_sampling_rate  = g_usb_in_out_pre_sampling_rate;//WIRED_AUDIO_MIXING_MODE_USB_DL_OUT_SAMPLE_RATE;//first open, use default rate.
    sw_src_config.in_frame_size_max = (sw_src_config.in_sampling_rate / 1000) * WIRED_AUDIO_USB_IN_PROCESS_PERIOD * sizeof(int32_t);
    sw_src_config.out_res = RESOLUTION_32BIT;
    sw_src_config.out_sampling_rate = WIRED_AUDIO_MIXING_MODE_USB_DL_OUT_SAMPLE_RATE;
    sw_src_config.out_frame_size_max = (sw_src_config.out_sampling_rate / 1000) * WIRED_AUDIO_USB_IN_PROCESS_PERIOD * sizeof(int32_t);
    g_usb_in_out_src_port = stream_function_sw_src_get_port(source);
    WIRED_AUDIO_LOG_MSGID_I("[USB_IN_OUT][config][start] sw src init: 0x%08x, %d, %d, %d, %d, %d, %d, %d, %d", 9,
                                g_usb_in_out_src_port,
                                sw_src_config.mode,
                                sw_src_config.channel_num,
                                sw_src_config.in_res,
                                sw_src_config.in_sampling_rate,
                                sw_src_config.in_frame_size_max,
                                sw_src_config.out_res,
                                sw_src_config.out_sampling_rate,
                                sw_src_config.out_frame_size_max);
    stream_function_sw_src_init(g_usb_in_out_src_port, &sw_src_config);
    memcpy(&g_usb_in_out_sw_src_config, &sw_src_config, sizeof(sw_src_config_t));
    //g_usb_in_out_pre_sampling_rate = smp_config.in_sampling_rate; //for run time update rate.

    /* SW gain init */
    sw_gain_config_t default_config;
    default_config.resolution = RESOLUTION_32BIT;
    default_config.target_gain = 0;
    // default_config.target_gain = 0;
    default_config.up_step = 1;
    default_config.up_samples_per_step = 2;
    default_config.down_step = -1;
    default_config.down_samples_per_step = 2;
    g_usb_in_out_sw_gain_port = stream_function_sw_gain_get_port(source);
    WIRED_AUDIO_LOG_MSGID_I("[USB_IN_OUT][config][start] sw gain init: %d, %d, %d, %d, %d, %d, %d", 7,
                                2,
                                default_config.resolution,
                                default_config.target_gain,
                                default_config.up_step,
                                default_config.up_samples_per_step,
                                default_config.down_step,
                                default_config.down_samples_per_step);
    stream_function_sw_gain_init(g_usb_in_out_sw_gain_port, 2, &default_config);
}

void wired_audio_usb_in_out_source_close(SOURCE source)
{
    UNUSED(source);
    stream_function_sw_mixer_member_unregister(SW_MIXER_PORT_0, g_usb_in_out_mixer_member);
    stream_function_sw_mixer_member_delete(g_usb_in_out_mixer_member);
    uint8_t i;
    for (i = 0; i < SUB_STREAM_MIXER_MEMBER; i++) {
        if(g_sub_stream_mixer_member[i] == g_usb_in_out_mixer_member) {
            g_sub_stream_mixer_member[i] = NULL;
            break;
        }
    }
    //stream_function_sw_mixer_deinit(SW_MIXER_PORT_0);

    stream_function_sw_src_deinit(g_usb_in_out_src_port);

    stream_function_sw_gain_deinit(g_usb_in_out_sw_gain_port);

    g_usb_in_out_src_port = NULL;
    g_usb_in_out_mixer_member = NULL;
    g_usb_in_out_source = NULL;
}

WIRED_AUDIO_ATTR_TEXT_IN_IRAM bool wired_audio_usb_in_source_get_avail_size(SOURCE source, uint32_t *avail_size)
{
    wired_audio_handle_t *handle;
    audio_codec_pcm_t *codec_pcm;
    DSP_STREAMING_PARA_PTR stream;
    uint32_t avail_block_size, avail_data_sample_by_channel, check_data_sample_by_channel, truncate_data_sample_by_channel, dsp_frame_samples_by_channel, first_dsp_block_size;
    uint32_t dsp_process_block_size, sample_bytes, cache_buffer_samples_by_channel, usb_frame_samples_by_channel;

    handle = (wired_audio_handle_t *)(source->param.data_dl.scenario_param.usb_in_local_param.usb_in_param.handle);

    if ((source->param.data_dl.scenario_sub_id != AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) &&
        (source->param.data_dl.scenario_sub_id != AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1)) {
        WIRED_AUDIO_LOG_MSGID_E("[USB_IN][source_get_avail_size]: scenario_sub_id invalid %d", 1, source->param.data_dl.scenario_sub_id);
        AUDIO_ASSERT(0);
        return false;
    }

#if 0
    if (source->param.data_dl.scenario_param.usb_in_local_param.is_afe_irq_comming == false) {
        WIRED_AUDIO_LOG_MSGID_E("[USB_IN][source_get_avail_size][sub_id %d]: AFE IRQ is not triggered", 1, source->param.data_dl.scenario_sub_id);
        AUDIO_ASSERT(0);
        return false;
    }
#endif

    if (hal_nvic_query_exception_number() == -1) {
        stream = DSP_Streaming_Get(source, source->transform->sink);
#ifdef WIRED_AUDIO_DEBUG_LOG
        WIRED_AUDIO_DEBUG_LOG_MSGID_I("[USB_IN][source_get_avail_size][sub_id %d]: In task level, bypass, %d, %d, %d", 4, source->param.data_dl.scenario_sub_id, stream->streamingStatus, handle->process_status, handle->process_frame_size);
#endif
        if (handle->process_status == false) {
            /* The function is already called in DL IRQ, so just return the result. */
            *avail_size = handle->process_frame_size;
        } else {
            /* Skip multi-run case */
            *avail_size = 0;
        }
        return true;
    } else {
        /* reset the process status when AFE irq is triggered */
        handle->process_status = false;
    }

    codec_pcm = &(source->param.data_dl.scenario_param.usb_in_local_param.usb_in_param.codec_param.pcm);
    if (codec_pcm->format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
        sample_bytes = 4;
    } else {
        sample_bytes = 2;
    }
    first_dsp_block_size = ((codec_pcm->frame_interval + WIRED_AUDIO_USB_IN_PREFILL_TIME) * (uint32_t)source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size) / 1000;
    dsp_process_block_size = (codec_pcm->frame_interval * (uint32_t)source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size) / 1000;
    dsp_frame_samples_by_channel = (codec_pcm->sample_rate * codec_pcm->frame_interval) / (1000 * 1000);
    usb_frame_samples_by_channel = (USB_IRQ_INTERVAL * codec_pcm->sample_rate) / (1000 * 1000);

    /* Get the avail size in the AVM buffer */
    if (source->streamBuffer.ShareBufferInfo.read_offset < source->streamBuffer.ShareBufferInfo.write_offset) {
        avail_block_size = source->streamBuffer.ShareBufferInfo.write_offset - source->streamBuffer.ShareBufferInfo.read_offset;
    } else if (source->streamBuffer.ShareBufferInfo.read_offset == source->streamBuffer.ShareBufferInfo.write_offset) {
        if (source->streamBuffer.ShareBufferInfo.bBufferIsFull == true) {
            avail_block_size = source->streamBuffer.ShareBufferInfo.length;
            WIRED_AUDIO_LOG_MSGID_E("[USB_IN][source_get_avail_size][sub_id %d]: buffer is full, read_offset = write_offset = %d", 2, source->param.data_dl.scenario_sub_id, source->streamBuffer.ShareBufferInfo.read_offset);
        } else {
            avail_block_size = 0;
            WIRED_AUDIO_LOG_MSGID_E("[USB_IN][source_get_avail_size][sub_id %d]: buffer is empty, read_offset = write_offset = %d", 2, source->param.data_dl.scenario_sub_id, source->streamBuffer.ShareBufferInfo.read_offset);
        }
    } else {
        avail_block_size = source->streamBuffer.ShareBufferInfo.write_offset + source->streamBuffer.ShareBufferInfo.length - source->streamBuffer.ShareBufferInfo.read_offset;
    }
    handle->avail_block_size = avail_block_size;
    avail_data_sample_by_channel = (codec_pcm->sample_rate / 1000) * (avail_block_size / source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size);

    /* Decide the copy size of the current process period */
    if ((uint32_t)source->param.data_dl.current_notification_index == 0) {
        if (avail_block_size < first_dsp_block_size) {
            WIRED_AUDIO_LOG_MSGID_E("[USB_IN][source_get_avail_size][sub_id %d]: 1st, AVM buffer data is not enough, use dummy data, actual: %d, request: %d", 3,
                                        source->param.data_dl.scenario_sub_id,
                                        avail_block_size,
                                        first_dsp_block_size);
            source->param.data_dl.scenario_param.usb_in_local_param.is_dummy_data = true;
            handle->copy_block_size = 0;
            *avail_size = dsp_process_block_size;
            /* Always reset read_offset to keep only one latest frame in the AVM buffer */
            handle->avm_read_offset = (source->streamBuffer.ShareBufferInfo.write_offset + source->streamBuffer.ShareBufferInfo.length - source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size) % source->streamBuffer.ShareBufferInfo.length;
            handle->avm_write_offset = source->streamBuffer.ShareBufferInfo.write_offset;
        } else {
            WIRED_AUDIO_LOG_MSGID_W("[USB_IN][source_get_avail_size][sub_id %d]: 1st, AVM buffer data is enough, use real data, actual: %d, request: %d", 3,
                                        source->param.data_dl.scenario_sub_id,
                                        avail_block_size,
                                        first_dsp_block_size);
            source->param.data_dl.scenario_param.usb_in_local_param.is_dummy_data = false;
            handle->copy_block_size = first_dsp_block_size;
            *avail_size = handle->copy_block_size;
            handle->avm_read_offset = (source->streamBuffer.ShareBufferInfo.write_offset + source->streamBuffer.ShareBufferInfo.length - first_dsp_block_size) % source->streamBuffer.ShareBufferInfo.length;
            handle->avm_write_offset = source->streamBuffer.ShareBufferInfo.write_offset;
        }
    } else {
        cache_buffer_samples_by_channel = stream_function_sw_buffer_get_channel_used_size(handle->buffer_port, 1) / sizeof(int32_t);
        if (cache_buffer_samples_by_channel < usb_frame_samples_by_channel) {
            /* Copy one more USB frame to avoid the process frame size error */
            check_data_sample_by_channel = usb_frame_samples_by_channel;
        } else {
            check_data_sample_by_channel = 0;
        }
        if ((avail_data_sample_by_channel >= (check_data_sample_by_channel + dsp_frame_samples_by_channel - usb_frame_samples_by_channel)) &&
            (avail_data_sample_by_channel <= (check_data_sample_by_channel + dsp_frame_samples_by_channel + usb_frame_samples_by_channel))) {
            /* The data is enough, so copy data in */
            source->param.data_dl.scenario_param.usb_in_local_param.is_dummy_data = false;
            /* There has one more frame, need to remove it */
            if (avail_data_sample_by_channel == (dsp_frame_samples_by_channel + usb_frame_samples_by_channel * 2)) {
                truncate_data_sample_by_channel = usb_frame_samples_by_channel;
            } else {
                truncate_data_sample_by_channel = 0;
            }
            avail_data_sample_by_channel -= truncate_data_sample_by_channel;
            if (cache_buffer_samples_by_channel >= (((WIRED_AUDIO_USB_IN_PREFILL_TIME + WIRED_AUDIO_USB_IN_THRESHOLD_NUMBER) / 1000) * (codec_pcm->sample_rate / 1000))) {
                /* If the process interval is too small, the drift direction of IRQ maybe not stable, which may cause the sw buffer cache too many data. */
                handle->copy_block_size = dsp_process_block_size;
            } else {
                handle->copy_block_size = (avail_data_sample_by_channel / usb_frame_samples_by_channel) * source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size;
            }
            *avail_size = handle->copy_block_size;
            if (avail_data_sample_by_channel < dsp_frame_samples_by_channel) {
                WIRED_AUDIO_LOG_MSGID_W("[USB_IN][source_get_avail_size][sub_id %d]: AVM buffer avail data is 1 USB frame less, SW buffer %d", 2, source->param.data_dl.scenario_sub_id, cache_buffer_samples_by_channel);
            }
            if (avail_data_sample_by_channel > dsp_frame_samples_by_channel) {
                WIRED_AUDIO_LOG_MSGID_W("[USB_IN][source_get_avail_size][sub_id %d]: AVM buffer avail data is 1 USB frame more, SW buffer %d", 2, source->param.data_dl.scenario_sub_id, cache_buffer_samples_by_channel);
            }
            handle->avm_read_offset = source->streamBuffer.ShareBufferInfo.read_offset;
            handle->avm_write_offset = source->streamBuffer.ShareBufferInfo.write_offset;
        } else {
            /* The USB is pending to send data or USB is already closed, so just use dummy data for process */
            if (avail_data_sample_by_channel < (dsp_frame_samples_by_channel - usb_frame_samples_by_channel)) {
                /* Too less data */
                WIRED_AUDIO_LOG_MSGID_W("[USB_IN][source_get_avail_size][sub_id %d]: AVM buffer avail data is too less, actual avm sample: %d, %d, %d, %d", 5,
                                            source->param.data_dl.scenario_sub_id, avail_data_sample_by_channel, dsp_frame_samples_by_channel, usb_frame_samples_by_channel, check_data_sample_by_channel);
            } else {
                /* Too many data */
                WIRED_AUDIO_LOG_MSGID_W("[USB_IN][source_get_avail_size][sub_id %d]: AVM buffer avail data is too many, actual avm sample: %d, %d, %d, %d", 5,
                                            source->param.data_dl.scenario_sub_id, avail_data_sample_by_channel, dsp_frame_samples_by_channel, usb_frame_samples_by_channel, check_data_sample_by_channel);
            }
            source->param.data_dl.scenario_param.usb_in_local_param.is_dummy_data = true;
            handle->copy_block_size = 0;
            *avail_size = dsp_process_block_size;
            /* Always keep only one latest frame in the AVM buffer */
            handle->avm_read_offset = (source->streamBuffer.ShareBufferInfo.write_offset + source->streamBuffer.ShareBufferInfo.length - source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size) % source->streamBuffer.ShareBufferInfo.length;
            handle->avm_write_offset = source->streamBuffer.ShareBufferInfo.write_offset;
        }
    }
    *avail_size = (*avail_size / (uint32_t)source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size) * sample_bytes * codec_pcm->sample_rate / 1000;
    handle->process_frame_size = *avail_size;

#ifdef WIRED_AUDIO_DEBUG_LOG
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[USB_IN][source_get_avail_size][sub_id %d]: avail number %d, AVM process number %d, in size %d, read_offset %d, read_offset(fix) %d, write_offset %d, sw buffer %d", 8,
                                source->param.data_dl.scenario_sub_id,
                                handle->avail_block_size / (uint32_t)source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size,
                                handle->copy_block_size / (uint32_t)source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size,
                                handle->process_frame_size,
                                source->streamBuffer.ShareBufferInfo.read_offset,
                                handle->avm_read_offset,
                                source->streamBuffer.ShareBufferInfo.write_offset,
                                stream_function_sw_buffer_get_channel_used_size(handle->buffer_port, 1) / sizeof(int32_t));
#endif

    return true;
}

static void wired_audio_usb_in_source_clock_skew_check(SOURCE source)
{
    audio_codec_pcm_t *codec_pcm;
    wired_audio_handle_t *handle;
    uint32_t dsp_frame_size_by_channel, dsp_frame_samples_by_channel, sample_bytes, dsp_prefill_samples_by_channel, dsp_prefill_threshold_samples_by_channel;
    uint32_t diff_samples, usb_frame_samples_by_channel, avm_avail_samples_by_channel, cache_buffer_samples_by_channel;
    int32_t compensatory_samples, frac_rpt;

    handle = (wired_audio_handle_t *)(source->param.data_dl.scenario_param.usb_in_local_param.usb_in_param.handle);
    codec_pcm = &(source->param.data_dl.scenario_param.usb_in_local_param.usb_in_param.codec_param.pcm);
    if (codec_pcm->format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
        sample_bytes = 4;
    } else {
        sample_bytes = 2;
    }
    dsp_frame_size_by_channel = (codec_pcm->sample_rate * sizeof(int32_t) * codec_pcm->frame_interval) / (1000 * 1000);
    dsp_frame_samples_by_channel = dsp_frame_size_by_channel / sizeof(int32_t);
    avm_avail_samples_by_channel = handle->process_frame_size / sample_bytes;
    usb_frame_samples_by_channel = codec_pcm->sample_rate / 1000;
    dsp_prefill_samples_by_channel = (WIRED_AUDIO_USB_IN_PREFILL_TIME / 1000) * codec_pcm->sample_rate / 1000;
    dsp_prefill_threshold_samples_by_channel = (WIRED_AUDIO_USB_IN_THRESHOLD_NUMBER / 1000) * codec_pcm->sample_rate / 1000;

    if (source->param.data_dl.scenario_param.usb_in_local_param.is_dummy_data == true) {
        /* always output fix samples */
        stream_function_sw_buffer_config_channel_output_size(handle->buffer_port, 1, dsp_frame_size_by_channel);
        stream_function_sw_buffer_config_channel_output_size(handle->buffer_port, 2, dsp_frame_size_by_channel);
        /* bypass sw clk skew */
        stream_function_sw_clk_skew_configure_compensation_samples(handle->clk_skew_port, 0);

        return;
    }

    /* Check whether we need to restart the clkskew */
    if ((handle->total_compen_samples != 0) && (handle->compen_method == WIRED_AUDIO_COMPENSATORY_METHOD_DISABLE)) {
        handle->compen_method = WIRED_AUDIO_COMPENSATORY_METHOD_SW_CLK_SKEW;
    }

    if (handle->compen_method == WIRED_AUDIO_COMPENSATORY_METHOD_SW_CLK_SKEW) {
        if (handle->total_compen_samples > 0) {
            /* buffer output 479 samples, clk skew will change them to 480 samples */
            compensatory_samples = 1;
            stream_function_sw_clk_skew_get_frac_rpt(handle->clk_skew_port, 1, &frac_rpt);
            if (frac_rpt == (handle->clk_skew_compensation_mode - 1)) {
                handle->total_compen_samples -= 1;
                stream_function_sw_buffer_config_channel_output_size(handle->buffer_port, 1, (dsp_frame_samples_by_channel - 1) * sizeof(int32_t));
                stream_function_sw_buffer_config_channel_output_size(handle->buffer_port, 2, (dsp_frame_samples_by_channel - 1) * sizeof(int32_t));
            } else {
                stream_function_sw_buffer_config_channel_output_size(handle->buffer_port, 1, dsp_frame_samples_by_channel * sizeof(int32_t));
                stream_function_sw_buffer_config_channel_output_size(handle->buffer_port, 2, dsp_frame_samples_by_channel * sizeof(int32_t));
            }
        } else if (handle->total_compen_samples < 0) {
            /* buffer output 481 samples, clk skew will change them to 480 samples */
            compensatory_samples = -1;
            stream_function_sw_clk_skew_get_frac_rpt(handle->clk_skew_port, 1, &frac_rpt);
            if (frac_rpt == -(handle->clk_skew_compensation_mode - 1)) {
                handle->total_compen_samples += 1;
                stream_function_sw_buffer_config_channel_output_size(handle->buffer_port, 1, (dsp_frame_samples_by_channel + 1) * sizeof(int32_t));
                stream_function_sw_buffer_config_channel_output_size(handle->buffer_port, 2, (dsp_frame_samples_by_channel + 1) * sizeof(int32_t));
            } else {
                stream_function_sw_buffer_config_channel_output_size(handle->buffer_port, 1, dsp_frame_samples_by_channel * sizeof(int32_t));
                stream_function_sw_buffer_config_channel_output_size(handle->buffer_port, 2, dsp_frame_samples_by_channel * sizeof(int32_t));
            }
        } else {
            /* buffer output 480 samples */
            compensatory_samples = 0;
            stream_function_sw_buffer_config_channel_output_size(handle->buffer_port, 1, dsp_frame_samples_by_channel * sizeof(int32_t));
            stream_function_sw_buffer_config_channel_output_size(handle->buffer_port, 2, dsp_frame_samples_by_channel * sizeof(int32_t));
        }
        stream_function_sw_clk_skew_configure_compensation_samples(handle->clk_skew_port, compensatory_samples);

        WIRED_AUDIO_LOG_MSGID_I("[USB_IN][source_clock_skew_check][sub_id %d]: do clkskew, dir %d, left sample %d, sw buffer sample %d", 4,
                                    source->param.data_dl.scenario_sub_id,
                                    compensatory_samples,
                                    handle->total_compen_samples,
                                    stream_function_sw_buffer_get_channel_used_size(handle->buffer_port, 1));

        if (handle->total_compen_samples == 0) {
            handle->compen_method = WIRED_AUDIO_COMPENSATORY_METHOD_DISABLE;
            handle->compen_count = 0;
            handle->compen_flag = 0;
        }
    } else {
        if (handle->total_compen_samples != 0) {
            AUDIO_ASSERT(0);
        }
        cache_buffer_samples_by_channel = stream_function_sw_buffer_get_channel_used_size(handle->buffer_port, 1) / sizeof(int32_t);
        if ((cache_buffer_samples_by_channel + avm_avail_samples_by_channel) < dsp_frame_samples_by_channel) {
            AUDIO_ASSERT(0);
        }
        if (source->param.data_dl.current_notification_index == 0) {
            diff_samples = dsp_prefill_samples_by_channel;
        } else {
            diff_samples = cache_buffer_samples_by_channel + avm_avail_samples_by_channel - dsp_frame_samples_by_channel;
        }

#ifdef WIRED_AUDIO_DEBUG_LOG
        WIRED_AUDIO_DEBUG_LOG_MSGID_I("[USB_IN][source_clock_skew_check][sub_id %d]: monitor %d, %d, %d, %d, %d, %d, %d, %d", 9,
                                    source->param.data_dl.scenario_sub_id,
                                    cache_buffer_samples_by_channel, avm_avail_samples_by_channel, dsp_frame_samples_by_channel,
                                    diff_samples, dsp_prefill_samples_by_channel, dsp_prefill_threshold_samples_by_channel,
                                    handle->compen_count, handle->compen_flag);
#endif

        if (diff_samples <= (dsp_prefill_samples_by_channel - dsp_prefill_threshold_samples_by_channel)) {
            if (handle->compen_count < WIRED_AUDIO_USB_IN_CLKSKEW_CHECK_NUMBER) {
                handle->compen_count += 1;
                handle->compen_flag = 1;
            } else {
                handle->total_compen_samples = dsp_prefill_threshold_samples_by_channel;
                handle->compen_count = 0;
                handle->compen_flag = 0;
            }
        } else if ((handle->compen_flag == 1) && (diff_samples > (dsp_prefill_samples_by_channel - dsp_prefill_threshold_samples_by_channel))) {
            /* The +1ms compensatory is false alarm and the usb irq is just not stable in this time */
            if (handle->compen_count != 0) {
                handle->compen_count -= 1;
            }
            if (handle->compen_count == 0) {
                handle->compen_flag = 0;
            }
        } else if (diff_samples >= (dsp_prefill_samples_by_channel + dsp_prefill_threshold_samples_by_channel)) {
            if (handle->compen_count < WIRED_AUDIO_USB_IN_CLKSKEW_CHECK_NUMBER) {
                /* There may be -1ms compensatory but its persistent period is less than 50ms */
                handle->compen_count += 1;
                handle->compen_flag = -1;
            } else {
                /* There is -1ms compensatory and its persistent period is more than 50ms*/
                handle->total_compen_samples = -dsp_prefill_threshold_samples_by_channel;
                handle->compen_count = 0;
            }
        } else if ((handle->compen_flag == -1) && (diff_samples < (dsp_prefill_samples_by_channel + dsp_prefill_threshold_samples_by_channel))) {
            /* The -1ms compensatory is false alarm and the usb irq is just not stable in this time */
            if (handle->compen_count != 0) {
                handle->compen_count -= 1;
            }
            if (handle->compen_count == 0) {
                handle->compen_flag = 0;
            }
        }

        /* bypass sw clk skew operation. */
        stream_function_sw_buffer_config_channel_output_size(handle->buffer_port, 1, dsp_frame_size_by_channel);
        stream_function_sw_buffer_config_channel_output_size(handle->buffer_port, 2, dsp_frame_size_by_channel);

        stream_function_sw_clk_skew_configure_compensation_samples(handle->clk_skew_port, 0);
    }
}

WIRED_AUDIO_ATTR_TEXT_IN_IRAM uint32_t wired_audio_usb_in_source_copy_payload(SOURCE source, uint8_t *src_buf, uint8_t *dst_buf, uint32_t length)
{
    wired_audio_handle_t *handle;
    DSP_STREAMING_PARA_PTR stream;
    audio_codec_pcm_t *codec_pcm;
    uint32_t dsp_frame_size_by_channel, read_offset;
    uint32_t total_copied_samples, current_usb_frame_index, usb_frame_samples, usb_frame_count;

    UNUSED(src_buf);
    UNUSED(dst_buf);
    UNUSED(length);

    if ((source->param.data_dl.scenario_sub_id != AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) &&
        (source->param.data_dl.scenario_sub_id != AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1)) {
        WIRED_AUDIO_LOG_MSGID_E("[USB_IN][source_copy_payload]: scenario_sub_id invalid %d", 1, source->param.data_dl.scenario_sub_id);
        AUDIO_ASSERT(0);
        return 0;
    }

#if 0
    if (source->param.data_dl.scenario_param.usb_in_local_param.is_afe_irq_comming == false) {
        WIRED_AUDIO_LOG_MSGID_E("[USB_IN][source_get_avail_size][sub_id %d]: AFE IRQ is not triggered", 1, source->param.data_dl.scenario_sub_id);
        AUDIO_ASSERT(0);
        return false;
    }
#endif

    codec_pcm = &(source->param.data_dl.scenario_param.usb_in_local_param.usb_in_param.codec_param.pcm);
    dsp_frame_size_by_channel = (codec_pcm->sample_rate * sizeof(int32_t) * codec_pcm->frame_interval) / (1000 * 1000);

    stream = DSP_Streaming_Get(source, source->transform->sink);
    handle = (wired_audio_handle_t *)(source->param.data_dl.scenario_param.usb_in_local_param.usb_in_param.handle);

    total_copied_samples = 0;
    if (source->param.data_dl.scenario_param.usb_in_local_param.is_dummy_data == true) {
        for(uint32_t i = 0; i < codec_pcm->channel_mode; i++) {
            memset((U8 *)(stream->callback.EntryPara.in_ptr[i]), 0, dsp_frame_size_by_channel);
        }
        usb_frame_count = codec_pcm->frame_interval / 1000;
    } else {
        current_usb_frame_index = 0;
        usb_frame_count = handle->copy_block_size / ((uint32_t)source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size);
        read_offset = handle->avm_read_offset;
        do {
            src_buf = (uint8_t *)(source->streamBuffer.ShareBufferInfo.start_addr + read_offset);
            usb_frame_samples = codec_pcm->sample_rate / 1000;
            if (codec_pcm->channel_mode == 1) {
                if (codec_pcm->format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
                    ShareBufferCopy_I_24bit_to_D_32bit_1ch((uint8_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                     (uint32_t *)(stream->callback.EntryPara.in_ptr[0]) + total_copied_samples,
                                                     usb_frame_samples);
                } else {
                    ShareBufferCopy_D_16bit_to_D_32bit_1ch((uint16_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)), (uint32_t *)(stream->callback.EntryPara.in_ptr[0]) + total_copied_samples, usb_frame_samples);
                }
            } else if (codec_pcm->channel_mode == 2) {
                if (codec_pcm->format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
                    ShareBufferCopy_I_24bit_to_D_32bit_2ch((uint8_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                     (uint32_t *)(stream->callback.EntryPara.in_ptr[0]) + total_copied_samples,
                                                     (uint32_t *)(stream->callback.EntryPara.in_ptr[1]) + total_copied_samples,
                                                     usb_frame_samples);
                } else {
                    ShareBufferCopy_I_16bit_to_D_32bit_2ch((uint32_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                     (uint32_t *)(stream->callback.EntryPara.in_ptr[0]) + total_copied_samples,
                                                     (uint32_t *)(stream->callback.EntryPara.in_ptr[1]) + total_copied_samples,
                                                     (uint32_t)usb_frame_samples);
                }
            } else if (codec_pcm->channel_mode == 8) {
                if (codec_pcm->format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
                    ShareBufferCopy_I_24bit_to_D_32bit_8ch((uint32_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                     (uint32_t *)(stream->callback.EntryPara.in_ptr[0]) + total_copied_samples,
                                                     (uint32_t *)(stream->callback.EntryPara.in_ptr[1]) + total_copied_samples,
                                                     (uint32_t *)(stream->callback.EntryPara.in_ptr[2]) + total_copied_samples,
                                                     (uint32_t *)(stream->callback.EntryPara.in_ptr[3]) + total_copied_samples,
                                                     (uint32_t *)(stream->callback.EntryPara.in_ptr[4]) + total_copied_samples,
                                                     (uint32_t *)(stream->callback.EntryPara.in_ptr[5]) + total_copied_samples,
                                                     (uint32_t *)(stream->callback.EntryPara.in_ptr[6]) + total_copied_samples,
                                                     (uint32_t *)(stream->callback.EntryPara.in_ptr[7]) + total_copied_samples,
                                                     usb_frame_samples);
                } else {
                    ShareBufferCopy_I_16bit_to_D_32bit_8ch((uint32_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                     (uint32_t *)(stream->callback.EntryPara.in_ptr[0]) + total_copied_samples,
                                                     (uint32_t *)(stream->callback.EntryPara.in_ptr[1]) + total_copied_samples,
                                                     (uint32_t *)(stream->callback.EntryPara.in_ptr[2]) + total_copied_samples,
                                                     (uint32_t *)(stream->callback.EntryPara.in_ptr[3]) + total_copied_samples,
                                                     (uint32_t *)(stream->callback.EntryPara.in_ptr[4]) + total_copied_samples,
                                                     (uint32_t *)(stream->callback.EntryPara.in_ptr[5]) + total_copied_samples,
                                                     (uint32_t *)(stream->callback.EntryPara.in_ptr[6]) + total_copied_samples,
                                                     (uint32_t *)(stream->callback.EntryPara.in_ptr[7]) + total_copied_samples,
                                                     (uint32_t)usb_frame_samples);
                }
            }
            total_copied_samples += usb_frame_samples;
            read_offset = (read_offset + source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size) % source->streamBuffer.ShareBufferInfo.length;
        } while (++current_usb_frame_index < usb_frame_count);
    }

    wired_audio_usb_in_source_clock_skew_check(source);

    /* Reset stream status */
    stream->callback.EntryPara.in_size = usb_frame_count * (codec_pcm->sample_rate * sizeof(int32_t)) / 1000;

#ifdef AIR_AUDIO_DUMP_ENABLE
    uint32_t audio_dump_size;
    if (source->param.data_dl.scenario_param.usb_in_local_param.is_dummy_data == true) {
        audio_dump_size = dsp_frame_size_by_channel;
    } else {
        audio_dump_size = total_copied_samples * sizeof(int32_t);
    }
    if (source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) {
        for(uint32_t i = 0; i < codec_pcm->channel_mode; i++) {
            LOG_AUDIO_DUMP((uint8_t *)stream->callback.EntryPara.in_ptr[i], audio_dump_size, WIRED_AUDIO_USB_IN_I_1+i);
        }
    } else {
        for(uint32_t i = 0; i < codec_pcm->channel_mode; i++) {
            LOG_AUDIO_DUMP((uint8_t *)stream->callback.EntryPara.in_ptr[i], audio_dump_size, WIRED_AUDIO_USB_IN_2_I_1+i);
        }
    }
#endif

#ifdef WIRED_AUDIO_DEBUG_LOG
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[USB_IN][source_copy_payload][sub_id %d]: %d, 0x%08x, %d, %d, done", 5,
            source->param.data_dl.scenario_sub_id,
            source->param.data_dl.scenario_param.usb_in_local_param.is_dummy_data, src_buf, total_copied_samples, usb_frame_count);
#endif

    /* Workaround for audio transmitter check */
    return source->param.data_dl.max_payload_size;
}

WIRED_AUDIO_ATTR_TEXT_IN_IRAM uint32_t wired_audio_usb_in_source_get_new_read_offset(SOURCE source, U32 amount)
{
    n9_dsp_share_info_ptr ShareBufferInfo;
    uint32_t read_offset;
    wired_audio_handle_t *handle;

    UNUSED(amount);

    if ((source->param.data_dl.scenario_sub_id != AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) &&
        (source->param.data_dl.scenario_sub_id != AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1)) {
        WIRED_AUDIO_LOG_MSGID_E("[USB_IN][source_get_new_read_offset][sub_id %d]: scenario_sub_id invalid %d", 1, source->param.data_dl.scenario_sub_id);
        return 0;
    }

    ShareBufferInfo = &source->streamBuffer.ShareBufferInfo;
    handle = (wired_audio_handle_t *)(source->param.data_dl.scenario_param.usb_in_local_param.usb_in_param.handle);
    if (source->param.data_dl.scenario_param.usb_in_local_param.is_dummy_data == true) {
        /* Always keep latest 1 USB frame when the total data is not enough. */
        read_offset = handle->avm_read_offset;
    } else {
        read_offset = (handle->avm_read_offset + handle->copy_block_size) % ShareBufferInfo->length;
    }

#ifdef WIRED_AUDIO_DEBUG_LOG
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[USB_IN][source_get_new_read_offset][sub_id %d]: update read_offset %d, write_offset = %d", 3,
                                source->param.data_dl.scenario_sub_id, read_offset, ShareBufferInfo->write_offset);
#endif

    return read_offset;
}

WIRED_AUDIO_ATTR_TEXT_IN_IRAM void wired_audio_usb_in_source_drop_postprocess(SOURCE source, uint32_t amount)
{
    wired_audio_handle_t *handle;

    UNUSED(amount);

    if ((source->param.data_dl.scenario_sub_id != AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) &&
        (source->param.data_dl.scenario_sub_id != AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1)) {
        WIRED_AUDIO_LOG_MSGID_E("[USB_IN][source_drop_postprocess]: scenario_sub_id invalid %d", 1, source->param.data_dl.scenario_sub_id);
        return;
    }

    if (source->param.data_dl.scenario_param.usb_in_local_param.is_dummy_data == false) {
        if (source->param.data_dl.current_notification_index == 0) {
            /* DSP has process first frame from the AVM buffer, update the flag */
            source->param.data_dl.current_notification_index = 1;
        }
    }

    /* Set flag to wait for the next AFE IRQ trigger. */
    handle = (wired_audio_handle_t *)(source->param.data_dl.scenario_param.usb_in_local_param.usb_in_param.handle);
    handle->process_status = true;

#if 0
#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
    if (handle != NULL) {
        if ((source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) &&
            (source->param.data_dl.current_notification_index != 0) && (source->param.data_dl.scenario_param.usb_in_local_param.usb_in_param.process_frames != 0)) {
            wired_audio_usb_vol_buffer_msg.data_buf_1_cur  = (uint16_t *)stream_function_sw_buffer_get_channel_read_pointer(handle->buffer_port, 1);
            wired_audio_usb_vol_buffer_msg.data_buf_1_head = (uint16_t *)stream_function_sw_buffer_get_channel_start_pointer(handle->buffer_port, 1);
            wired_audio_usb_vol_buffer_msg.data_buf_1_tail = (uint16_t *)stream_function_sw_buffer_get_channel_end_pointer(handle->buffer_port, 1);
            wired_audio_usb_vol_buffer_msg.data_buf_2_cur  = (uint16_t *)stream_function_sw_buffer_get_channel_read_pointer(handle->buffer_port, 2);
            wired_audio_usb_vol_buffer_msg.data_buf_2_head = (uint16_t *)stream_function_sw_buffer_get_channel_start_pointer(handle->buffer_port, 2);
            wired_audio_usb_vol_buffer_msg.data_buf_2_tail = (uint16_t *)stream_function_sw_buffer_get_channel_end_pointer(handle->buffer_port, 2);
            DTM_enqueue(DTM_EVENT_ID_WIRED_AUDIO_USB_VOLUME_SMART_BALANCE_PROCESS, (uint32_t)&wired_audio_usb_vol_buffer_msg, false);
        } else if (((source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0)||(source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0)) &&
                    (source->param.data_dl.current_notification_index != 0) && (source->param.data_dl.scenario_param.usb_in_local_param.usb_in_param.process_frames == 0)) {
            wired_audio_usb_vol_buffer_msg.data_buf_1_cur = NULL;
            wired_audio_usb_vol_buffer_msg.data_buf_2_cur = NULL;
            DTM_enqueue(DTM_EVENT_ID_WIRED_AUDIO_USB_VOLUME_SMART_BALANCE_PROCESS, (uint32_t)&wired_audio_usb_vol_buffer_msg, false);
        }
    }
#endif
#endif

#ifdef WIRED_AUDIO_DEBUG_LOG
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[USB_IN][source_drop_postprocess][sub_id %d]: process done", 1, source->param.data_dl.scenario_sub_id);
#endif
}

bool wired_audio_usb_in_source_close(SOURCE source)
{
    if ((source->param.data_dl.scenario_sub_id != AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) &&
        (source->param.data_dl.scenario_sub_id != AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1)) {
        WIRED_AUDIO_LOG_MSGID_E("[USB_IN][source_close]: scenario_sub_id invalid %d", 1, source->param.data_dl.scenario_sub_id);
        return false;
    }

    /* application deinit */
    wired_audio_handle_t *dongle_handle = (wired_audio_handle_t *)(source->param.data_dl.scenario_param.usb_in_local_param.usb_in_param.handle);

    stream_function_sw_gain_deinit(dongle_handle->gain_port);

    stream_function_sw_clk_skew_deinit(dongle_handle->clk_skew_port);

    stream_function_sw_buffer_deinit(dongle_handle->buffer_port);

#ifndef AIR_DCHS_MODE_ENABLE
    stream_function_sw_src_deinit((sw_src_port_t *)dongle_handle->src1_port);

    uint8_t i;
    for (i = 0; i < SUB_STREAM_MIXER_MEMBER; i++) {
        if(g_sub_stream_mixer_member[i] == dongle_handle->mixer_member) {
            g_sub_stream_mixer_member[i] = NULL;
            break;
        }
    }
    stream_function_sw_mixer_member_unregister(SW_MIXER_PORT_0, dongle_handle->mixer_member);
    stream_function_sw_mixer_member_delete(dongle_handle->mixer_member);
    //stream_function_sw_mixer_deinit(SW_MIXER_PORT_0);

#if 0
#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
    wired_audio_usb_in_vol_balance_deinit(source);
#endif
#endif
#endif

    /* release handle */
    wired_audio_release_handle(dongle_handle);
    source->param.data_dl.scenario_param.usb_in_local_param.usb_in_param.handle = NULL;

    if (source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) {
        g_application_ptr_usb_in_0 = NULL;
    } else if (source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1) {
        g_application_ptr_usb_in_1 = NULL;
    }

    return true;
}

static int32_t wired_audio_usb_out_sink_clock_skew_check(SINK sink)
{
    int32_t compensatory_samples = 0;
    uint32_t remain_samples, frame_samples, input_frame;
    audio_codec_pcm_t *pcm_param;
    wired_audio_handle_t *handle;

    pcm_param = &sink->param.data_ul.scenario_param.usb_out_local_param.codec_param.pcm;
    handle = (wired_audio_handle_t *)sink->param.data_ul.scenario_param.usb_out_local_param.handle;

    frame_samples = (pcm_param->sample_rate * pcm_param->frame_interval) / (1000 * 1000);

    /* get remain samples */
    input_frame = 1;
    remain_samples = stream_function_sw_buffer_get_channel_used_size(handle->buffer_port, 1) / sizeof(int16_t);
    if ((remain_samples + input_frame * frame_samples) > handle->clkskew_water_mark) {
        if (handle->clkskew_monitor_count > -(handle->clkskew_monitor_threshold)) {
            handle->clkskew_monitor_count -= 1;
        }
    } else if ((remain_samples + input_frame * frame_samples) < handle->clkskew_water_mark) {
        if (handle->clkskew_monitor_count < handle->clkskew_monitor_threshold) {
            handle->clkskew_monitor_count += 1;
        }
    } else {
        handle->clkskew_monitor_count = 0;
    }

    if (handle->clkskew_monitor_count == handle->clkskew_monitor_threshold) {
        compensatory_samples = 1;
    } else if (handle->clkskew_monitor_count == -handle->clkskew_monitor_threshold) {
        compensatory_samples = -1;
    } else {
        compensatory_samples = 0;
    }

    return compensatory_samples;
}

static void wired_audio_usb_out_open( mcu2dsp_open_param_p open_param, SOURCE source, SINK sink)
{
    wired_audio_handle_t *handle;
    audio_codec_pcm_t *codec_pcm;
    stream_resolution_t resolution;
    uint32_t total_channels, sample_byte, sample_bits, out_frame_sample, process_frame_sample, process_sample_rate;
    sw_gain_config_t default_config;
    sw_src_config_t sw_src_config;
    sw_clk_skew_config_t sw_clk_skew_config;
    sw_buffer_config_t buffer_config;

    UNUSED(open_param);
    UNUSED(source);
    UNUSED(sink);

    handle = (wired_audio_handle_t *)sink->param.data_ul.scenario_param.usb_out_local_param.handle;

    codec_pcm = &(sink->param.data_ul.scenario_param.usb_out_local_param.codec_param.pcm);
    out_frame_sample = (codec_pcm->sample_rate * codec_pcm->frame_interval) / (1000 * 1000);
    if (codec_pcm->format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
        resolution = RESOLUTION_32BIT;
        sample_bits = 32;
        sample_byte = 4;
    } else {
        resolution = RESOLUTION_16BIT;
        sample_bits = 16;
        sample_byte = 2;
    }
    if (gDspAlgParameter.EscoMode.Tx == VOICE_SWB) {
        process_sample_rate = 32000;
    } else {
        process_sample_rate = 16000;
    }
    process_frame_sample = (process_sample_rate / 1000) * WIRED_AUDIO_USB_OUT_PROCESS_PERIOD;
#ifdef AIR_ECHO_MEMIF_IN_ORDER_ENABLE
    handle->out_channel_num = source->param.audio.channel_num;
#else
    if (source->param.audio.echo_reference) {
        handle->out_channel_num = source->param.audio.channel_num + 1;
    } else {
        handle->out_channel_num = source->param.audio.channel_num;
    }
#endif

    /* input SW SRC init */
    AUDIO_ASSERT(handle->src1_port == NULL);
    sw_src_config.mode = SW_SRC_MODE_NORMAL;
    sw_src_config.channel_num = handle->out_channel_num;
    sw_src_config.in_res = RESOLUTION_16BIT;
    sw_src_config.in_sampling_rate  = source->param.audio.src_rate;
    sw_src_config.in_frame_size_max = ((process_frame_sample * 1000) / process_sample_rate) * (sw_src_config.in_sampling_rate / 1000) * sizeof(int16_t);
    sw_src_config.out_res = RESOLUTION_16BIT;
    sw_src_config.out_sampling_rate = process_sample_rate;
    sw_src_config.out_frame_size_max = process_frame_sample * sizeof(int16_t);
    handle->src1_port = stream_function_sw_src_get_multi_port(sink);
    WIRED_AUDIO_LOG_MSGID_I("[USB_OUT][init] input sw src init: 0x%08x, %d, %d, %d, %d, %d, %d, %d, %d", 9,
                                handle->src1_port,
                                sw_src_config.mode,
                                sw_src_config.channel_num,
                                sw_src_config.in_res,
                                sw_src_config.in_sampling_rate,
                                sw_src_config.in_frame_size_max,
                                sw_src_config.out_res,
                                sw_src_config.out_sampling_rate,
                                sw_src_config.out_frame_size_max);
    stream_function_sw_src_init((sw_src_port_t *)handle->src1_port, &sw_src_config);

    /* SW GAIN init */
    AUDIO_ASSERT(handle->gain_port == NULL);
    default_config.resolution = RESOLUTION_16BIT;
    default_config.target_gain = 0;
    default_config.up_step = 1;
    default_config.up_samples_per_step = 2;
    default_config.down_step = -1;
    default_config.down_samples_per_step = 2;
#if defined(AIR_DCHS_MODE_ENABLE)
    total_channels = 1;
#elif defined(AIR_VOICE_NR_ENABLE)
    total_channels = 1;
#else
    total_channels = handle->out_channel_num;
#endif
    handle->gain_port = stream_function_sw_gain_get_port(sink);
    WIRED_AUDIO_LOG_MSGID_I("[USB_OUT][init] sw gain init: 0x%08x, %d, %d, %d, %d, %d, %d, %d", 8,
                                handle->gain_port,
                                total_channels,
                                default_config.resolution,
                                default_config.target_gain,
                                default_config.up_step,
                                default_config.up_samples_per_step,
                                default_config.down_step,
                                default_config.down_samples_per_step);
    stream_function_sw_gain_init(handle->gain_port, total_channels, &default_config);

    /* output SW SRC init */
    AUDIO_ASSERT(handle->src2_port == NULL);
    sw_src_config.mode = SW_SRC_MODE_NORMAL;
#ifdef AIR_VOICE_NR_ENABLE
    sw_src_config.channel_num = 1;
#else
    sw_src_config.channel_num = handle->out_channel_num;
#endif
    sw_src_config.in_res = RESOLUTION_16BIT;
    sw_src_config.in_sampling_rate  = process_sample_rate;
    sw_src_config.in_frame_size_max = process_frame_sample * sizeof(int16_t);
    sw_src_config.out_res = RESOLUTION_16BIT;
    sw_src_config.out_sampling_rate = sink->param.data_ul.scenario_param.usb_out_local_param.codec_param.pcm.sample_rate;
    sw_src_config.out_frame_size_max = (sw_src_config.out_sampling_rate * process_frame_sample * sizeof(int16_t)) / sw_src_config.in_sampling_rate;
    handle->src2_port = stream_function_sw_src_get_multi_port(sink);
    WIRED_AUDIO_LOG_MSGID_I("[USB_OUT][init] output sw src init: 0x%08x, %d, %d, %d, %d, %d, %d, %d, %d", 9,
                                handle->src2_port,
                                sw_src_config.mode,
                                sw_src_config.channel_num,
                                sw_src_config.in_res,
                                sw_src_config.in_sampling_rate,
                                sw_src_config.in_frame_size_max,
                                sw_src_config.out_res,
                                sw_src_config.out_sampling_rate,
                                sw_src_config.out_frame_size_max);
    stream_function_sw_src_init((sw_src_port_t *)handle->src2_port, &sw_src_config);

    /* VOLUME_MONITOR init */
#ifdef AIR_MUTE_MIC_DETECTION_ENABLE
    AUDIO_ASSERT(p_usb_out_meter_port == NULL);
    /* init volume estimator port */
    p_usb_out_vol_estimator_nvkey_buf = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, sizeof(audio_spectrum_meter_nvkey_t));
    if (p_usb_out_vol_estimator_nvkey_buf == NULL) {
        AUDIO_ASSERT(0);
    }
    if (nvkey_read_full_key(NVKEY_DSP_PARA_SILENCE_DETECTION2, p_usb_out_vol_estimator_nvkey_buf, sizeof(audio_spectrum_meter_nvkey_t)) != NVDM_STATUS_NAT_OK) {
        AUDIO_ASSERT(0);
    }
    volume_estimator_config_t config;
    p_usb_out_meter_port = volume_estimator_get_port(sink);
    if (p_usb_out_meter_port == NULL) {
        AUDIO_ASSERT(0 && "[audio_transmitter] Audio Spectrum memter port is null.");
    }
    config.resolution = RESOLUTION_16BIT;
    config.frame_size = process_frame_sample * sizeof(int16_t);
    config.channel_num = 1;
    config.mode = VOLUME_ESTIMATOR_CHAT_INSTANT_MODE;
    config.sample_rate = process_sample_rate;
    config.nvkey_para = (void *)&(((audio_spectrum_meter_nvkey_t *)p_usb_out_vol_estimator_nvkey_buf)->chat_vol_nvkey);
    config.internal_buffer = NULL;
    config.internal_buffer_size = 0;
    volume_estimator_init(p_usb_out_meter_port, &config);
    WIRED_AUDIO_LOG_MSGID_I("[USB_OUT][init] volume estimator 0x%x info, %d, %d, %d,internal_buffer 0x%x, 0x%x\r\n", 6,
                p_usb_out_meter_port,
                config.frame_size,
                config.channel_num,
                config.sample_rate,
                config.internal_buffer,
                config.internal_buffer_size);
#endif

    /* SW clk skew init */
    AUDIO_ASSERT(handle->clk_skew_port == NULL);
#ifdef AIR_VOICE_NR_ENABLE
    sw_clk_skew_config.channel = 1;
#else
    sw_clk_skew_config.channel = handle->out_channel_num;
#endif
    sw_clk_skew_config.bits = 16;
    sw_clk_skew_config.order = C_Flp_Ord_5;
    sw_clk_skew_config.skew_io_mode = C_Skew_Oup;
    sw_clk_skew_config.skew_compensation_mode = SW_CLK_SKEW_COMPENSATION_1_SAMPLE_IN_8_FRAME;
    sw_clk_skew_config.skew_work_mode = SW_CLK_SKEW_CONTINUOUS;
    sw_clk_skew_config.max_output_size = (sw_clk_skew_config.channel * (codec_pcm->sample_rate / 1000) * codec_pcm->frame_interval * sizeof(uint16_t)) / 1000;
    sw_clk_skew_config.continuous_frame_size = sw_clk_skew_config.max_output_size / sw_clk_skew_config.channel;
    handle->clk_skew_port = stream_function_sw_clk_skew_get_port(sink);
    handle->clkskew_monitor_count = 0;
    WIRED_AUDIO_LOG_MSGID_I("[USB_OUT][init] sw clkskew init: 0x%08x, %d, %d, %d, %d, %d, %d, %d, %d", 9,
                                handle->clk_skew_port,
                                sw_clk_skew_config.channel,
                                sw_clk_skew_config.bits,
                                sw_clk_skew_config.order,
                                sw_clk_skew_config.skew_io_mode,
                                sw_clk_skew_config.skew_compensation_mode,
                                sw_clk_skew_config.skew_work_mode,
                                sw_clk_skew_config.max_output_size,
                                sw_clk_skew_config.continuous_frame_size);
    stream_function_sw_clk_skew_init(handle->clk_skew_port, &sw_clk_skew_config);

    /* SW buffer init */
    AUDIO_ASSERT(handle->buffer_port == NULL);
    buffer_config.mode = SW_BUFFER_MODE_FIXED_OUTPUT_LENGTH;
#ifdef AIR_VOICE_NR_ENABLE
    buffer_config.total_channels = 1;
#else
    buffer_config.total_channels = handle->out_channel_num;
#endif
    buffer_config.watermark_max_size = out_frame_sample * sizeof(uint16_t) * 4;
    buffer_config.watermark_min_size = 0;
    buffer_config.output_size = out_frame_sample * sizeof(uint16_t);
    handle->buffer_port = stream_function_sw_buffer_get_port(sink);
    WIRED_AUDIO_LOG_MSGID_I("[USB_OUT][init] sw buffer init: 0x%08x, %d, %d, %d, %d, %d", 6,
                                handle->buffer_port,
                                buffer_config.mode,
                                buffer_config.total_channels,
                                buffer_config.watermark_max_size,
                                buffer_config.watermark_min_size,
                                buffer_config.output_size);
    stream_function_sw_buffer_init(handle->buffer_port, &buffer_config);
}

static void wired_audio_usb_out_iem_open( mcu2dsp_open_param_p open_param, SOURCE source, SINK sink)
{
    wired_audio_handle_t *handle;
    audio_codec_pcm_t *codec_pcm;
    stream_resolution_t resolution;
    uint32_t total_channels, sample_byte, sample_bits, out_frame_sample;
    sw_gain_config_t default_config;
    sw_clk_skew_config_t sw_clk_skew_config;
    sw_buffer_config_t buffer_config;
    uint8_t channel_num_sw_clk_skew = 1;
    uint8_t channel_num_sw_buffer = 1;

    UNUSED(open_param);

    extern CONNECTION_IF g_usb_out_local_streams;
    extern stream_feature_list_t stream_feature_list_wired_audio_usb_out_iem[];
    g_usb_out_local_streams.pfeature_table = stream_feature_list_wired_audio_usb_out_iem;

    if(g_usb_in_out_pre_sampling_rate != open_param->stream_in_param.afe.sampling_rate){
        if (g_usb_in_out_src_port != NULL) {
            stream_function_sw_src_deinit(g_usb_in_out_src_port);
            g_usb_in_out_sw_src_config.in_sampling_rate = open_param->stream_in_param.afe.sampling_rate;
            g_usb_in_out_sw_src_config.in_frame_size_max = (g_usb_in_out_sw_src_config.in_sampling_rate / 1000) * WIRED_AUDIO_USB_IN_PROCESS_PERIOD * sizeof(int32_t);
            stream_function_sw_src_init(g_usb_in_out_src_port, &g_usb_in_out_sw_src_config);
        }
        g_usb_in_out_pre_sampling_rate = open_param->stream_in_param.afe.sampling_rate;
    }

    extern CONNECTION_IF g_usb_out_local_streams;
    if(open_param->stream_in_param.afe.format <= HAL_AUDIO_PCM_FORMAT_U16_BE){
        stream_feature_configure_resolution((stream_feature_list_ptr_t)g_usb_out_local_streams.pfeature_table, RESOLUTION_16BIT, CONFIG_DECODER);
        resolution = RESOLUTION_16BIT;
    }
    else{
        stream_feature_configure_resolution((stream_feature_list_ptr_t)g_usb_out_local_streams.pfeature_table, RESOLUTION_32BIT, CONFIG_DECODER);
        resolution = RESOLUTION_32BIT;
    }
    DSP_MW_LOG_I("[LD_NR]wired_audio_usb_out channel_num=%d, resolution:%d", 2, source->param.audio.channel_num, resolution);
#ifdef AIR_LD_NR_ENABLE
    ld_nr_port_t *ld_nr_port;
    ld_nr_config_t ld_nr_config;
    ld_nr_port = stream_function_ld_nr_get_port(source);
    ld_nr_config.channel_num = source->param.audio.channel_num;
    ld_nr_config.frame_size  = 240 * ((resolution == RESOLUTION_16BIT)? 2:4);
    ld_nr_config.resolution  = resolution;
    ld_nr_config.sample_rate = source->param.audio.rate;

    if (p_wired_audio_usb_out_ld_nr_key == NULL) {
        configASSERT(0 && "ld_dr key = null");
    }

    ld_nr_config.nvkey_para = p_wired_audio_usb_out_ld_nr_key;
    ld_nr_config.background_process_enable = true;
    ld_nr_config.background_process_fr_num = 2;
    stream_function_ld_nr_init(ld_nr_port, &ld_nr_config);
    DSP_MW_LOG_I("[LD_NR]wired_audio_usb_out channel_num=%d, resolution:%d", 2, source->param.audio.channel_num, resolution);
#endif /* AIR_LD_NR_ENABLE */
#ifdef AIR_SOFTWARE_DRC_ENABLE
    /* sw drc init */
    sw_compander_config_t drc_config;
    drc_config.mode = SW_COMPANDER_AUDIO_MODE;
    drc_config.channel_num = source->param.audio.channel_num;
    drc_config.sample_rate = source->param.audio.rate;
    drc_config.frame_base = 8;
    drc_config.recovery_gain = 4; /* 0dB */
    drc_config.vol_default_gain = 0x08000000; /* 0dB */
    drc_config.default_nvkey_mem = NULL;
    drc_config.default_nvkey_id = NVKEY_DSP_PARA_MIC_AU_CPD;
    sw_compander_port_t *drc_port = stream_function_sw_compander_get_port(source);
    stream_function_sw_compander_init(drc_port, &drc_config);
    DSP_MW_LOG_I("[USB_OUT]sw drc 0x%x info, %d, %d, %d, %d, 0x%x, 0x%x, 0x%x\r\n", 11,
                drc_port,
                drc_config.mode,
                drc_config.channel_num,
                drc_config.sample_rate,
                drc_config.frame_base,
                drc_config.recovery_gain,
                drc_config.vol_default_gain,
                drc_config.default_nvkey_id);
#endif

    handle = (wired_audio_handle_t *)sink->param.data_ul.scenario_param.usb_out_local_param.handle;

    codec_pcm = &(sink->param.data_ul.scenario_param.usb_out_local_param.codec_param.pcm);
    out_frame_sample = (codec_pcm->sample_rate * codec_pcm->frame_interval) / (1000 * 1000);
    if (codec_pcm->format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
        resolution = RESOLUTION_32BIT;
        sample_bits = 32;
        sample_byte = 4;
    } else {
        resolution = RESOLUTION_16BIT;
        sample_bits = 16;
        sample_byte = 2;
    }
#ifdef AIR_ECHO_MEMIF_IN_ORDER_ENABLE
    handle->out_channel_num = source->param.audio.channel_num;
#else
    if (source->param.audio.echo_reference) {
        handle->out_channel_num = source->param.audio.channel_num + 1;
    } else {
        handle->out_channel_num = source->param.audio.channel_num;
    }
#endif
    channel_num_sw_clk_skew = handle->out_channel_num;
    channel_num_sw_buffer = handle->out_channel_num;

    /* SW GAIN init */
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    AUDIO_ASSERT(handle->gain_port == NULL);
    default_config.resolution = resolution;
    default_config.target_gain = 0;
    default_config.up_step = 1;
    default_config.up_samples_per_step = 2;
    default_config.down_step = -1;
    default_config.down_samples_per_step = 2;
    total_channels = handle->out_channel_num;
    handle->gain_port = stream_function_sw_gain_get_port(sink);
    WIRED_AUDIO_LOG_MSGID_I("[USB_OUT][start] sw gain init: 0x%08x, %d, %d, %d, %d, %d, %d, %d", 8,
                                handle->gain_port,
                                total_channels,
                                default_config.resolution,
                                default_config.target_gain,
                                default_config.up_step,
                                default_config.up_samples_per_step,
                                default_config.down_step,
                                default_config.down_samples_per_step);
    stream_function_sw_gain_init(handle->gain_port, total_channels, &default_config);

    /* SW clk skew init */
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    AUDIO_ASSERT(handle->clk_skew_port == NULL);
    sw_clk_skew_config.channel = channel_num_sw_clk_skew;
    sw_clk_skew_config.bits = sample_bits;;
    sw_clk_skew_config.order = C_Flp_Ord_5;
    sw_clk_skew_config.skew_io_mode = C_Skew_Oup;
    sw_clk_skew_config.skew_compensation_mode = SW_CLK_SKEW_COMPENSATION_1_SAMPLE_IN_8_FRAME;
    sw_clk_skew_config.skew_work_mode = SW_CLK_SKEW_CONTINUOUS;
    sw_clk_skew_config.max_output_size = sw_clk_skew_config.channel * codec_pcm->sample_rate / 1000 * codec_pcm->frame_interval / 1000 * sample_byte;
    sw_clk_skew_config.continuous_frame_size = sw_clk_skew_config.max_output_size / sw_clk_skew_config.channel;
    handle->clk_skew_port = stream_function_sw_clk_skew_get_port(sink);
    handle->clkskew_monitor_count = 0;
    WIRED_AUDIO_LOG_MSGID_I("[USB_OUT][start] sw clkskew init: 0x%08x, %d, %d, %d, %d, %d, %d, %d, %d", 9,
                                handle->clk_skew_port,
                                sw_clk_skew_config.channel,
                                sw_clk_skew_config.bits,
                                sw_clk_skew_config.order,
                                sw_clk_skew_config.skew_io_mode,
                                sw_clk_skew_config.skew_compensation_mode,
                                sw_clk_skew_config.skew_work_mode,
                                sw_clk_skew_config.max_output_size,
                                sw_clk_skew_config.continuous_frame_size);
    stream_function_sw_clk_skew_init(handle->clk_skew_port, &sw_clk_skew_config);
#endif

    /* SW buffer init */
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    AUDIO_ASSERT(handle->buffer_port == NULL);
    buffer_config.mode = SW_BUFFER_MODE_FIXED_OUTPUT_LENGTH;
    buffer_config.total_channels = channel_num_sw_buffer;
    buffer_config.watermark_max_size = out_frame_sample * sample_byte * 4;
    buffer_config.watermark_min_size = 0;
    buffer_config.output_size = out_frame_sample * sample_byte;
    handle->buffer_port = stream_function_sw_buffer_get_port(sink);
    WIRED_AUDIO_LOG_MSGID_I("[USB_OUT][start] sw buffer init: 0x%08x, %d, %d, %d, %d, %d", 6,
                                handle->buffer_port,
                                buffer_config.mode,
                                buffer_config.total_channels,
                                buffer_config.watermark_max_size,
                                buffer_config.watermark_min_size,
                                buffer_config.output_size);
    stream_function_sw_buffer_init(handle->buffer_port, &buffer_config);
#endif

    /*copy to virtual source init*/
    g_usb_in_out_copy_to_virtual_source_port = stream_function_copy_to_virtual_source_get_port(source);
    stream_function_copy_to_virtual_source_init(g_usb_in_out_copy_to_virtual_source_port,VIRTUAL_SOURCE_WIRED_AUDIO_USB_OUT);
}

static void wired_audio_usb_out_start(mcu2dsp_start_param_p start_param, SOURCE source, SINK sink)
{
    UNUSED(source);
    UNUSED(sink);
    UNUSED(start_param);
}

static void wired_audio_usb_out_iem_start(mcu2dsp_start_param_p start_param, SOURCE source, SINK sink)
{
    wired_audio_handle_t *handle;
    DSP_STREAMING_PARA_PTR ul_stream;
    int32_t digital_gain_times_of_db;
    uint32_t i, total_channels;

    UNUSED(source);
    UNUSED(sink);
    UNUSED(start_param);

    handle = (wired_audio_handle_t *)sink->param.data_ul.scenario_param.usb_out_local_param.handle;
    total_channels = handle->out_channel_num;
    ul_stream = DSP_Streaming_Get(source, sink);

    afe_audio_init_input_channel(sink, total_channels, ul_stream);
    for (i = 1; i <= total_channels; i++) {
#ifdef AIR_AUDIO_HARDWARE_ENABLE
        digital_gain_times_of_db = afe_audio_get_input_digital_gain(afe_audio_get_input_channel(sink, i));
#else
        digital_gain_times_of_db = 0;
#endif
        WIRED_AUDIO_LOG_MSGID_I("[USB_OUT][start] sw gain init: channel %d gain to %d * 0.01dB\r\n", 2, i, digital_gain_times_of_db);
        stream_function_sw_gain_configure_gain_target(handle->gain_port, i, digital_gain_times_of_db);
    }
#endif
}

static void wired_audio_usb_out_close(SOURCE source, SINK sink)
{
    UNUSED(source);
    UNUSED(sink);
}

static void wired_audio_usb_out_iem_close(SOURCE source, SINK sink)
{
    UNUSED(source);
    UNUSED(sink);
#ifdef AIR_LD_NR_ENABLE
        ld_nr_port_t *ld_nr_port;
        ld_nr_port = stream_function_ld_nr_get_port(source);
        stream_function_ld_nr_deinit(ld_nr_port);
        preloader_pisplit_free_memory(p_wired_audio_usb_out_ld_nr_key);
        p_wired_audio_usb_out_ld_nr_key = NULL;
#endif /* AIR_LD_NR_ENABLE */
#ifdef AIR_SOFTWARE_DRC_ENABLE
        sw_compander_port_t *drc_port;
        drc_port = stream_function_sw_compander_get_port(source);
        stream_function_sw_compander_deinit(drc_port);
#endif
}

#define WIRED_AUDIO_USB_OUT_CLKSKEW_THRESHOLD 4

void wired_audio_usb_out_sink_init(SINK sink, audio_transmitter_open_param_p open_param, AUDIO_TRANSMITTER_PARAMETER *sink_param)
{
    usb_out_local_param_t *usb_out_param;
    wired_audio_handle_t *handle;

    UNUSED(sink_param);

    if (open_param->scenario_sub_id != AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT) {
        WIRED_AUDIO_LOG_MSGID_I("[USB_OUT][init] wired_audio_usb_out_sink_init: type %d, sub id invalid %d", 2, open_param->scenario_type, open_param->scenario_sub_id);
        AUDIO_ASSERT(0);
    }

    usb_out_param = &sink->param.data_ul.scenario_param.usb_out_local_param;

    WIRED_AUDIO_LOG_MSGID_I("[USB_OUT][init] is_with_ecnr %d, is_with_swb %d, %d, %d, %d, %d, %d", 7,
                                open_param->scenario_param.wired_audio_param.is_with_ecnr,
                                open_param->scenario_param.wired_audio_param.is_with_swb,
                                open_param->scenario_param.gaming_mode_param.codec_type,
                                open_param->scenario_param.gaming_mode_param.codec_param.pcm.channel_mode,
                                open_param->scenario_param.gaming_mode_param.codec_param.pcm.format,
                                open_param->scenario_param.gaming_mode_param.codec_param.pcm.frame_interval,
                                open_param->scenario_param.gaming_mode_param.codec_param.pcm.sample_rate);

    sink->param.data_ul.scenario_param.usb_out_local_param.codec_type = open_param->scenario_param.gaming_mode_param.codec_type;
    memcpy(&usb_out_param->codec_param.pcm, &open_param->scenario_param.wired_audio_param.codec_param.pcm, sizeof(audio_codec_pcm_t));
    usb_out_param->is_with_ecnr = open_param->scenario_param.wired_audio_param.is_with_ecnr;
    usb_out_param->is_with_swb = open_param->scenario_param.wired_audio_param.is_with_swb;
    handle = (wired_audio_handle_t *)wired_audio_alloc_handle(sink);
    memcpy(handle->gain, open_param->scenario_param.wired_audio_param.gain, 8 * sizeof(int32_t));
    handle->clkskew_monitor_count = 0;
    handle->clkskew_monitor_threshold = WIRED_AUDIO_USB_OUT_CLKSKEW_THRESHOLD;
    handle->clkskew_water_mark = (usb_out_param->codec_param.pcm.sample_rate * usb_out_param->codec_param.pcm.frame_interval) / (1000 * 1000);
    handle->first_process_flag = true;
    handle->share_buffer_info = (n9_dsp_share_info_ptr)hal_memview_cm4_to_dsp0((uint32_t)(open_param->p_share_info));
    usb_out_param->handle = handle;

    if ((open_param->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT)&&(usb_out_param->is_with_ecnr == true)) {
        if (usb_out_param->is_with_swb) {
            DSP_ALG_UpdateEscoTxMode(VOICE_SWB);
            DSP_ALG_UpdateEscoRxMode(VOICE_SWB);
            g_usb_out_local_streams.pfeature_table = stream_feature_list_wired_audio_usb_out_swb;
        } else {
            DSP_ALG_UpdateEscoTxMode(VOICE_WB);
            DSP_ALG_UpdateEscoRxMode(VOICE_WB);
            g_usb_out_local_streams.pfeature_table = stream_feature_list_wired_audio_usb_out;
        }
    }
}

uint32_t wired_audio_usb_out_sink_copy_payload(SINK sink, uint8_t *src_buf, uint8_t *dst_buf, uint32_t length)
{
    audio_codec_pcm_t *codec_pcm;
    uint32_t channel_mode, actual_Sample;
    hal_audio_format_t source_format, sink_format;
    wired_audio_handle_t *handle;

    UNUSED(length);

    if (sink->param.data_ul.scenario_sub_id != AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT) {
        AUDIO_ASSERT(0);
    }

    sink_format = sink->param.data_ul.scenario_param.usb_out_local_param.codec_param.pcm.format;
    source_format = sink->transform->source->param.audio.format;
    channel_mode = sink->param.data_ul.scenario_param.usb_out_local_param.codec_param.pcm.channel_mode;

    codec_pcm = &(sink->param.data_ul.scenario_param.usb_out_local_param.codec_param.pcm);
    actual_Sample = (codec_pcm->sample_rate * codec_pcm->frame_interval) / (1000 * 1000);
    if (channel_mode == 1)
    {
        if(source_format == HAL_AUDIO_PCM_FORMAT_S16_LE) {
            if (sink_format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
                ShareBufferCopy_D_16bit_to_I_24bit_1ch((uint16_t *)src_buf, (uint8_t *)dst_buf, actual_Sample);
                length = actual_Sample * 3;
            } else if (sink_format == HAL_AUDIO_PCM_FORMAT_S16_LE) {
                memcpy(dst_buf, src_buf, actual_Sample * sizeof(uint16_t));
            }
        } else if(source_format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
            if (sink_format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
                ShareBufferCopy_D_32bit_to_D_24bit_1ch((uint32_t *)src_buf, (uint8_t *)dst_buf, actual_Sample);
                length = actual_Sample * 3;
            } else if (sink_format == HAL_AUDIO_PCM_FORMAT_S16_LE) {
                AUDIO_ASSERT(0); //not support yet
            }
        }
    } else if (channel_mode == 2) {
        DSP_STREAMING_PARA_PTR stream = DSP_Streaming_Get(sink->transform->source, sink);
        VOID *src_buf1, *src_buf2;
        src_buf1 = stream->callback.EntryPara.out_ptr[0];
        if (sink->transform->source->param.audio.channel_num == 1) {
            src_buf2 = stream->callback.EntryPara.out_ptr[0];
        }
        else if(sink->transform->source->param.audio.channel_num == 2) {
            if(stream->callback.EntryPara.out_ptr[1] == NULL)
                src_buf2 = stream->callback.EntryPara.out_ptr[0];
            else
                src_buf2 = stream->callback.EntryPara.out_ptr[1];
        }
        if(source_format == HAL_AUDIO_PCM_FORMAT_S16_LE) {
            if (sink_format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
                ShareBufferCopy_D_16bit_to_I_24bit_2ch((uint16_t *)src_buf1, (uint16_t *)src_buf2, (uint8_t *)dst_buf, actual_Sample);
                length = actual_Sample * 3 * 2;
            } else if (sink_format == HAL_AUDIO_PCM_FORMAT_S16_LE) {
                DSP_D2I_BufferCopy_16bit((uint16_t *)dst_buf, (uint16_t *)src_buf1, (uint16_t *)src_buf2, actual_Sample);
                length = length * 2;
            }
        } else if(source_format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
            if (sink_format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
                ShareBufferCopy_D_32bit_to_I_24bit_2ch((uint32_t *)src_buf1, (uint32_t *)src_buf2, (uint8_t *)dst_buf, length / sizeof(uint32_t));
                length = actual_Sample * 3 * 2;
            } else if (sink_format == HAL_AUDIO_PCM_FORMAT_S16_LE) {
                AUDIO_ASSERT(0); //not support yet
            }
        }
    } else {
        AUDIO_ASSERT(0); //not support yet
    }

#ifdef AIR_AUDIO_DUMP_ENABLE
    if(source_format == HAL_AUDIO_PCM_FORMAT_S16_LE) {
        LOG_AUDIO_DUMP(src_buf, actual_Sample * sizeof(uint16_t), WIRED_AUDIO_USB_OUT_O_1);
    } else if(source_format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
        LOG_AUDIO_DUMP(src_buf, actual_Sample * sizeof(uint32_t), WIRED_AUDIO_USB_OUT_O_1);
    }
    //LOG_AUDIO_DUMP(dst_buf, length, WIRED_AUDIO_USB_OUT_O_1);
#endif

#ifdef WIRED_AUDIO_DEBUG_LOG
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[USB_OUT][sink_copy_payload]: copy length %d, actual_Sample %d", 2, length, actual_Sample);
#endif
    /* Do clk skew adjustment */
    handle = (wired_audio_handle_t *)sink->param.data_ul.scenario_param.usb_out_local_param.handle;
    handle->compen_samples = wired_audio_usb_out_sink_clock_skew_check(sink);
    stream_function_sw_clk_skew_configure_compensation_samples(handle->clk_skew_port, handle->compen_samples);

    return length;
}

bool wired_audio_usb_out_sink_get_avail_size(SINK sink, uint32_t *avail_size)
{
    wired_audio_handle_t *handle;
    DSP_STREAMING_PARA_PTR stream;

    if (sink->param.data_ul.scenario_sub_id != AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT){
        AUDIO_ASSERT(0);
        return false;
    }

    handle = (wired_audio_handle_t *)(sink->param.data_ul.scenario_param.usb_out_local_param.handle);
    if (hal_nvic_query_exception_number() == -1) {
        stream = DSP_Streaming_Get(sink->transform->source, sink);
#ifdef WIRED_AUDIO_DEBUG_LOG
        WIRED_AUDIO_DEBUG_LOG_MSGID_I("[USB_OUT][sink_get_avail_size]: In task level, bypass, %d, %d, %d, %d, %d, 0x%08x, %d, %d, %d", 9,
                                stream->streamingStatus, handle->process_status, handle->process_frame_size,
                                sink->streamBuffer.ShareBufferInfo.notify_count,
                                sink->streamBuffer.ShareBufferInfo.anchor_clk,
                                sink->param.data_ul.share_info_base_addr,
                                sink->streamBuffer.ShareBufferInfo.read_offset,
                                sink->streamBuffer.ShareBufferInfo.write_offset,
                                sink->streamBuffer.ShareBufferInfo.bBufferIsFull);
#endif
        if (handle->process_status == false) {
            /* The function is already called in DL IRQ, so just return the result. */
            *avail_size = handle->process_frame_size;
        } else {
            /* Skip multi-run case */
            *avail_size = 0;
        }
        return true;
    } else {
        /* Reset the process status when AFE irq is triggered */
        handle->process_status = false;
        /* Always return fix avail size */
        handle->process_frame_size = sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size;
        hal_audio_format_t sink_format = sink->param.data_ul.scenario_param.usb_out_local_param.codec_param.pcm.format;
        hal_audio_format_t source_format = sink->transform->source->param.audio.format;
        handle->process_frame_size = (source_format >= HAL_AUDIO_PCM_FORMAT_S24_LE)?
                            ((sink_format<= HAL_AUDIO_PCM_FORMAT_U16_BE)? (handle->process_frame_size-4)*2: (handle->process_frame_size-4)*4/3)
                            : (handle->process_frame_size);
        *avail_size = handle->process_frame_size;
        //WIRED_AUDIO_DEBUG_LOG_MSGID_I("[USB_OUT][sink_get_avail_size]: IRQ level,source_format %d,sink_format %d *avail_size %d",3,source_format,sink_format,*avail_size);

    }

#ifdef WIRED_AUDIO_DEBUG_LOG
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[USB_OUT][sink_get_avail_size]: IRQ level, avail size %d, notify_count %d, anchor_clk %d, share_info_base_addr 0x%08x, read_offset %d, write_offset %d, bBufferIsFull %d", 7,
                                *avail_size, sink->streamBuffer.ShareBufferInfo.notify_count,
                                sink->streamBuffer.ShareBufferInfo.anchor_clk,
                                sink->param.data_ul.share_info_base_addr,
                                sink->streamBuffer.ShareBufferInfo.read_offset,
                                sink->streamBuffer.ShareBufferInfo.write_offset,
                                sink->streamBuffer.ShareBufferInfo.bBufferIsFull);
#endif

    return true;
}

bool wired_audio_usb_out_sink_query_notification(SINK sink, bool *notification_flag)
{
    uint32_t curr_gpt, during_gpt, delta_gpt, process_threshold;
    uint32_t avail_block_size, new_write_offset;
    wired_audio_handle_t *handle;
    DSP_STREAMING_PARA_PTR stream;
    n9_dsp_share_info_t *p_ShareBufferInfo;

    UNUSED(sink);
    UNUSED(notification_flag);

    /* Set flag to wait for the next AFE IRQ trigger. */
    handle = (wired_audio_handle_t *)(sink->param.data_ul.scenario_param.usb_out_local_param.handle);
    handle->process_status = true;

    /* Reset the out_channel_num, as the feature process has changed it */
    stream = DSP_Streaming_Get(sink->transform->source, sink);
    stream->callback.EntryPara.out_channel_num = handle->out_channel_num;

    p_ShareBufferInfo = &sink->streamBuffer.ShareBufferInfo;

    /* Detect the USB irq status
     * 1. bit 7: whether USB irq has been triggered
     * 2. bit 0 ~ bit 6: USB irq count
     */
    if (p_ShareBufferInfo->notify_count == 0) {
        /* USB irq is not triggered, just re-init the write_offset to init value to avoid of buffer full */
        audio_transmitter_share_information_update_write_offset(sink, p_ShareBufferInfo->sub_info.block_info.block_size);
    }

    /* If MCU don't fetch data in time, just adjust the RPTR to avoid of overflow */
    audio_transmitter_share_information_fetch(NULL, sink);
    if (p_ShareBufferInfo->read_offset < p_ShareBufferInfo->write_offset) {
        avail_block_size = p_ShareBufferInfo->write_offset - p_ShareBufferInfo->read_offset;
    } else if (p_ShareBufferInfo->read_offset == p_ShareBufferInfo->write_offset) {
        if (p_ShareBufferInfo->bBufferIsFull == true) {
            avail_block_size = p_ShareBufferInfo->length;
        } else {
            avail_block_size = 0;
        }
    } else {
        avail_block_size = p_ShareBufferInfo->write_offset + p_ShareBufferInfo->length - p_ShareBufferInfo->read_offset;
    }
    uint8_t check_num = 2;
    if(p_ShareBufferInfo->sub_info.block_info.block_num < 3){
        check_num = 1;
    }
    if (avail_block_size > (p_ShareBufferInfo->sub_info.block_info.block_size * check_num)) {
        WIRED_AUDIO_LOG_MSGID_W("[USB_OUT][sink_query_notification] detect AVM buffer almost full %d", 1, avail_block_size);
        new_write_offset = (p_ShareBufferInfo->read_offset + p_ShareBufferInfo->sub_info.block_info.block_size) % p_ShareBufferInfo->length;
        audio_transmitter_share_information_update_read_offset_for_sink(sink, new_write_offset);
    }

    /* Update the 1-st process time to MCU for latency control. */
    if (handle->first_process_flag == true) {
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_gpt);
        hal_gpt_get_duration_count(handle->first_afe_irq_gpt, curr_gpt, &during_gpt);
        process_threshold = (WIRED_AUDIO_USB_OUT_PROCESS_PERIOD * 1000 * WIRED_AUDIO_USB_OUT_PREFILL_SIZE) / 100;
        if (during_gpt <= process_threshold) {
            delta_gpt = process_threshold - during_gpt;
            handle->share_buffer_info->asi_cur = delta_gpt / 1000;
            if (delta_gpt % 1000) {
                handle->share_buffer_info->asi_cur++;
            }
        } else {
            handle->share_buffer_info->asi_cur = 0;
        }
        handle->first_process_flag = false;
        WIRED_AUDIO_LOG_MSGID_I("[USB_OUT][sink_query_notification] latency start control: delay %d ms (%d %d %d)", 4,
                            handle->share_buffer_info->asi_cur,
                            handle->first_afe_irq_gpt,
                            curr_gpt,
                            WIRED_AUDIO_USB_OUT_PROCESS_THRESHOLD);
    }

#ifdef WIRED_AUDIO_DEBUG_LOG
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[USB_OUT][sink_query_notification] bBufferIsFull %d, read_offset %d, write_offset %d", 3,
                                p_ShareBufferInfo->bBufferIsFull,
                                p_ShareBufferInfo->read_offset,
                                p_ShareBufferInfo->write_offset);
#endif

    return true;
}

bool wired_audio_usb_out_sink_close(SINK sink)
{
    wired_audio_handle_t *handle;

    handle = (wired_audio_handle_t *)sink->param.data_ul.scenario_param.usb_out_local_param.handle;

    if (handle->src1_port != NULL) {
        WIRED_AUDIO_LOG_MSGID_I("[USB_OUT][sink_close] [SW_SRC1] deinit", 0);
        stream_function_sw_src_deinit(handle->src1_port);
        handle->src1_port = NULL;
    }

    if (handle->gain_port != NULL) { //deinit SW gain port in transmitter close flow
        WIRED_AUDIO_LOG_MSGID_I("[USB_OUT][sink_close] [SW_GAIN] deinit", 0);
        stream_function_sw_gain_deinit(handle->gain_port);
        handle->gain_port = NULL;
        afe_audio_deinit_input_channel(sink);
    }

    if (handle->src2_port != NULL) {
        WIRED_AUDIO_LOG_MSGID_I("[USB_OUT][sink_close] [SW_SRC2] deinit", 0);
        stream_function_sw_src_deinit(handle->src2_port);
        handle->src2_port = NULL;
    }

#ifdef AIR_MUTE_MIC_DETECTION_ENABLE
        WIRED_AUDIO_LOG_MSGID_I("[audio_transmitter][volume estimator] deinit", 0);
        volume_estimator_deinit(p_usb_out_meter_port);
        if (p_usb_out_vol_estimator_nvkey_buf) {
            preloader_pisplit_free_memory(p_usb_out_vol_estimator_nvkey_buf);
            p_usb_out_vol_estimator_nvkey_buf = NULL;
        }
        p_usb_out_meter_port = NULL;
#endif

    if (handle->clk_skew_port != NULL) {
        WIRED_AUDIO_LOG_MSGID_I("[USB_OUT][sink_close] [SW_CLKSKEW] deinit", 0);
        stream_function_sw_clk_skew_deinit(handle->clk_skew_port);
        handle->clk_skew_port = NULL;
    }

    if (handle->buffer_port != NULL) {
        WIRED_AUDIO_LOG_MSGID_I("[USB_OUT][sink_close] [SW_BUFFER] deinit", 0);
        stream_function_sw_buffer_deinit(handle->buffer_port);
        handle->buffer_port = NULL;
    }

    wired_audio_release_handle(handle);
    sink->param.data_ul.scenario_param.usb_out_local_param.handle = NULL;

    return true;
}

bool wired_audio_usb_out_iem_sink_close(SINK sink)
{
    wired_audio_handle_t *handle;
    handle = (wired_audio_handle_t *)sink->param.data_ul.scenario_param.usb_out_local_param.handle;
    if (handle->gain_port != NULL) { //deinit SW gain port in transmitter close flow
        WIRED_AUDIO_LOG_MSGID_I("[USB_OUT][sink_close] [SW_GAIN] deinit", 0);
        stream_function_sw_gain_deinit(handle->gain_port);
        handle->gain_port = NULL;
        afe_audio_deinit_input_channel(sink);
    }
    if (handle->clk_skew_port != NULL) {
        WIRED_AUDIO_LOG_MSGID_I("[USB_OUT][sink_close] [SW_CLKSKEW] deinit", 0);
        stream_function_sw_clk_skew_deinit(handle->clk_skew_port);
        handle->clk_skew_port = NULL;
    }
    if (handle->buffer_port != NULL) {
        WIRED_AUDIO_LOG_MSGID_I("[USB_OUT][sink_close] [SW_BUFFER] deinit", 0);
        stream_function_sw_buffer_deinit(handle->buffer_port);
        handle->buffer_port = NULL;
    }
    wired_audio_release_handle(handle);
    sink->param.data_ul.scenario_param.usb_out_local_param.handle = NULL;

    stream_function_copy_to_virtual_source_deinit(g_usb_in_out_copy_to_virtual_source_port);
    g_usb_in_out_pre_sampling_rate = WIRED_AUDIO_MIXING_MODE_USB_DL_OUT_SAMPLE_RATE;

    return true;
}

enum{
    LINE_IN_PARAM_IDX,
    LINE_IN_MASTER_PARAM_IDX,
    LINE_IN_SLAVE_PARAM_IDX,
    LINE_IN_PARAM_NUM
};

static int32_t wired_audio_line_in_parm_idx_get(audio_scenario_type_t audio_scenario_type)
{
    switch(audio_scenario_type)
    {
        case AUDIO_SCENARIO_TYPE_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER:
            return LINE_IN_MASTER_PARAM_IDX;
        break;
        case AUDIO_SCENARIO_TYPE_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE:
            return LINE_IN_SLAVE_PARAM_IDX;
        break;

        default:
        case AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN:
            return LINE_IN_PARAM_IDX;
        break;
    }
}

static line_in_local_param_t *g_wired_audio_line_in_param[LINE_IN_PARAM_NUM];

static void wired_audio_line_in_open(mcu2dsp_open_param_p open_param, SOURCE source, SINK sink)
{
    line_in_local_param_t *line_in_param;
    audio_codec_pcm_t *codec_pcm;
    uint32_t sample_byte, sample_bits, total_channels;
    stream_resolution_t resolution;
    uint32_t dsp_frame_sample;
    wired_audio_handle_t *line_in_handle;
    int32_t line_in_param_idx;

    UNUSED(open_param);
    UNUSED(source);
    UNUSED(sink);

    #if defined(AIR_BTA_IC_STEREO_HIGH_G3) && defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
    if (source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE) {
        //disconnect DL3 echo path
        hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_DISCONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I22, AUDIO_INTERCONNECTION_OUTPUT_O38);
        hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_DISCONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I23, AUDIO_INTERCONNECTION_OUTPUT_O39);
    }

    if (source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN) {
        //disconnect HW Gain1 echo path
        hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_DISCONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I14, AUDIO_INTERCONNECTION_OUTPUT_O38);
        hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_DISCONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I15, AUDIO_INTERCONNECTION_OUTPUT_O39);
    }
    #endif

    line_in_param = malloc(sizeof(line_in_local_param_t));
    AUDIO_ASSERT(line_in_param != NULL);
    line_in_param_idx = wired_audio_line_in_parm_idx_get(source->scenario_type);
    memset(line_in_param, 0, sizeof(line_in_local_param_t));
    line_in_param->handle = wired_audio_alloc_handle(source);
    AUDIO_ASSERT(line_in_param->handle != NULL);
    line_in_param->codec_param.pcm.channel_mode = source->param.audio.channel_num;
    line_in_param->codec_param.pcm.format = source->param.audio.format;
    line_in_param->codec_param.pcm.frame_interval = source->param.audio.period;
    line_in_param->codec_param.pcm.sample_rate = (source->param.audio.mem_handle.pure_agent_with_src) ? source->param.audio.src_rate : source->param.audio.rate;
    g_wired_audio_line_in_param[line_in_param_idx] = line_in_param;

    codec_pcm = &(line_in_param->codec_param.pcm);
    dsp_frame_sample = (codec_pcm->sample_rate * codec_pcm->frame_interval) / (1000 * 1000);
    if (codec_pcm->format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
        resolution = RESOLUTION_32BIT;
        sample_bits = 24;
        sample_byte = 4;
    } else {
        resolution = RESOLUTION_16BIT;
        sample_bits = 16;
        sample_byte = 2;
    }
    if (codec_pcm->channel_mode >= 2) {
        total_channels = 2;
    } else {
        total_channels = 1;
    }

#if defined(AIR_FIXED_RATIO_SRC)
    int32_t i, j, curr_rate;
    int32_t *p_ratio_list;
    src_fixed_ratio_config_t smp_config = {0};

    p_ratio_list = NULL;
    for (i = 0; i < WIRED_AUDIO_MAX_SUPPORT_INPUT_SAMPLE_RATE_CNT; i++) {
        if (codec_pcm->sample_rate == g_wired_audio_sw_src_in_sample_rate[i]) {
            for (j = 0; j < WIRED_AUDIO_MAX_SUPPORT_OUTPUT_SAMPLE_RATE_CNT; j++) {
                if (sink->param.audio.rate == g_wired_audio_sw_src_out_sample_rate[j]) {
                    p_ratio_list = (int32_t *)&g_wired_audio_sw_src_ratio[i][j][0];
                    break;
                }
            }
        }
    }
    AUDIO_ASSERT(p_ratio_list != NULL);

    line_in_handle = (wired_audio_handle_t *)line_in_param->handle;
    curr_rate = codec_pcm->sample_rate;
    smp_config.cvt_num = 0;
    smp_config.channel_number = total_channels;
    smp_config.with_codec = false;
    smp_config.multi_cvt_mode = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_SINGLE;
    smp_config.resolution = resolution;
    for (i = 0; i < WIRED_AUDIO_MAX_SUPPORT_FIX_SRX_RATIO_CNT; i++) {
        if (p_ratio_list[i] != 0) {
            AUDIO_ASSERT(i < 2); /* The fix src ratio only support 2 stage SW SRC */
            smp_config.in_sampling_rate = curr_rate;
            if (p_ratio_list[i] > 0) {
                curr_rate *= p_ratio_list[i];
            } else {
                curr_rate /= -p_ratio_list[i];
            }
            smp_config.out_sampling_rate = curr_rate;
            smp_config.cvt_num++;
            if (smp_config.cvt_num == 1) {
                line_in_handle->src1_port = stream_function_src_fixed_ratio_get_port(source);
                stream_function_src_fixed_ratio_init((src_fixed_ratio_port_t *)line_in_handle->src1_port, &smp_config);
            } else if (smp_config.cvt_num == 2) {
                smp_config.multi_cvt_mode = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_CONSECUTIVE;
                line_in_handle->src2_port = stream_function_src_fixed_ratio_get_2nd_port(source);
                stream_function_src_fixed_ratio_init((src_fixed_ratio_port_t *)line_in_handle->src2_port, &smp_config);
            } else {
                WIRED_AUDIO_LOG_MSGID_E("[LINE_IN][open] src_fixed_ratio init: not support SRC transfer, in rate: %d, out rate: %d", 2, codec_pcm->sample_rate, sink->param.audio.rate);
            }
            WIRED_AUDIO_LOG_MSGID_I("[LINE_IN][open] src_fixed_ratio init: channel_number %d, in_sampling_rate %d, out_sampling_rate %d, resolution %d", 4,
                                    smp_config.channel_number,
                                    smp_config.in_sampling_rate,
                                    smp_config.out_sampling_rate,
                                    smp_config.resolution);
        }
    }
#endif
}

static void wired_audio_line_in_start(mcu2dsp_start_param_p start_param, SOURCE source, SINK sink)
{
    UNUSED(start_param);
    UNUSED(source);
    UNUSED(sink);

#ifndef AIR_DCHS_MODE_ENABLE
    if (source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN) {
        uint32_t afe_start_time;
        hal_audio_trigger_start_parameter_t start_parameter;
        hal_audio_current_offset_parameter_t offset_parameter;
        hal_audio_memory_irq_enable_parameter_t irq_enable;

        start_parameter.memory_select = sink->param.audio.mem_handle.memory_select | source->param.audio.mem_handle.memory_select;
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
        start_parameter.memory_select |= sink->param.audio.mem_handle1.memory_select | sink->param.audio.mem_handle2.memory_select;
        WIRED_AUDIO_LOG_MSGID_I("[LINE_IN][start] 0x%x,0x%x,0x%x,0x%x,", 4,
                                            start_parameter.memory_select,
                                            sink->param.audio.mem_handle.memory_select,
                                            sink->param.audio.mem_handle1.memory_select,
                                            sink->param.audio.mem_handle2.memory_select);
#endif

        start_parameter.enable = true;
        hal_audio_set_value((hal_audio_set_value_parameter_t *)&start_parameter, HAL_AUDIO_SET_TRIGGER_MEMORY_START);

        irq_enable.memory_select    = sink->param.audio.mem_handle.memory_select;
        irq_enable.irq_counter      = sink->param.audio.mem_handle.irq_counter;
        irq_enable.rate             = sink->param.audio.mem_handle.audio_path_rate;
        irq_enable.enable           = true;
        hal_audio_set_value((hal_audio_set_value_parameter_t *)&irq_enable, HAL_AUDIO_SET_MEMORY_IRQ_ENABLE);

        offset_parameter.memory_select = sink->param.audio.mem_handle.memory_select;
        offset_parameter.pure_agent_with_src = false;
        hal_audio_get_value((hal_audio_get_value_parameter_t *)&offset_parameter, HAL_AUDIO_GET_MEMORY_OUTPUT_CURRENT_OFFSET);
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &afe_start_time);
        WIRED_AUDIO_LOG_MSGID_I("[LINE_IN][start] start AFE agent %d, sink rptr %d, start time %d", 3,
                                    start_parameter.memory_select,
                                    offset_parameter.offset - offset_parameter.base_address,
                                    afe_start_time);
    }
#endif
}

static void wired_audio_line_in_close(SOURCE source, SINK sink)
{
    line_in_local_param_t *line_in_param;
    wired_audio_handle_t *line_in_handle;
    int32_t line_in_param_idx;

    UNUSED(source);
    UNUSED(sink);
    line_in_param_idx = wired_audio_line_in_parm_idx_get(source->scenario_type);

    line_in_param = g_wired_audio_line_in_param[line_in_param_idx];
    line_in_handle = (wired_audio_handle_t *)line_in_param->handle;

#if defined(AIR_FIXED_RATIO_SRC)
    stream_function_src_fixed_ratio_deinit((src_fixed_ratio_port_t *)line_in_handle->src1_port);
    if (line_in_handle->src2_port) {
        stream_function_src_fixed_ratio_deinit((src_fixed_ratio_port_t *)line_in_handle->src2_port);
    }
    line_in_handle->src1_port = NULL;
    line_in_handle->src2_port = NULL;
#endif

    if (line_in_handle != NULL) {
        wired_audio_release_handle(line_in_handle);
    }
    if (line_in_param != NULL) {
        free(line_in_param);
    }

    g_wired_audio_line_in_param[line_in_param_idx] = NULL;
}

static line_out_local_param_t *g_wired_audio_line_out_param;

static void wired_audio_line_out_open(mcu2dsp_open_param_p open_param, SOURCE source, SINK sink)
{
    uint32_t total_channels, process_frame_sample, process_sample_rate;
    line_out_local_param_t *line_out_param;
    sw_gain_config_t default_config;
    wired_audio_handle_t *handle;
    sw_src_config_t sw_src_config;

    UNUSED(open_param);
    UNUSED(sink);

    line_out_param = malloc(sizeof(line_out_local_param_t));
    if (line_out_param == NULL) {
        AUDIO_ASSERT(0);
    }
    memset(line_out_param, 0, sizeof(line_out_local_param_t));
    line_out_param->handle = wired_audio_alloc_handle(source);
    handle = (wired_audio_handle_t *)(line_out_param->handle);

    g_wired_audio_line_out_param = line_out_param;

    if (open_param->stream_in_param.afe.stream_process_sampling_rate == 32000) {
        DSP_ALG_UpdateEscoTxMode(VOICE_SWB);
        DSP_ALG_UpdateEscoRxMode(VOICE_SWB);
        g_line_out_local_streams.pfeature_table = stream_feature_list_wired_audio_line_out_swb;
    } else {
        DSP_ALG_UpdateEscoTxMode(VOICE_WB);
        DSP_ALG_UpdateEscoRxMode(VOICE_WB);
        g_line_out_local_streams.pfeature_table = stream_feature_list_wired_audio_line_out;
    }

    //disconnect DL1 echo path
    hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_DISCONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I18, AUDIO_INTERCONNECTION_OUTPUT_O34);
    hal_audio_path_set_interconnection(AUDIO_INTERCONNECTION_DISCONNECT, HAL_AUDIO_PATH_CHANNEL_DIRECT, AUDIO_INTERCONNECTION_INPUT_I19, AUDIO_INTERCONNECTION_OUTPUT_O35);

#ifdef AIR_ECHO_MEMIF_IN_ORDER_ENABLE
    handle->out_channel_num = source->param.audio.channel_num;
#else
    if (source->param.audio.echo_reference) {
        handle->out_channel_num = source->param.audio.channel_num + 1;
    } else {
        handle->out_channel_num = source->param.audio.channel_num;
    }
#endif
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) && defined(AIR_DUAL_CHIP_NR_ON_MASTER_ENABLE) /* && defined(AIR_BTA_IC_PREMIUM_G2) */
    process_sample_rate = source->param.audio.src_rate;
#else
    if (gDspAlgParameter.EscoMode.Tx == VOICE_SWB) {
        process_sample_rate = 32000;
    } else {
        process_sample_rate = 16000;
    }
#endif
    process_frame_sample = (process_sample_rate / 1000) * WIRED_AUDIO_LINE_OUT_PROCESS_PERIOD;

    /* input SW SRC init */
    AUDIO_ASSERT(handle->src1_port == NULL);
    sw_src_config.mode = SW_SRC_MODE_NORMAL;
    sw_src_config.channel_num = handle->out_channel_num;
    sw_src_config.in_res = RESOLUTION_16BIT;
    sw_src_config.in_sampling_rate  = source->param.audio.src_rate;
    sw_src_config.in_frame_size_max = ((process_frame_sample * 1000) / process_sample_rate) * (sw_src_config.in_sampling_rate / 1000) * sizeof(int16_t);
    sw_src_config.out_res = RESOLUTION_16BIT;
    sw_src_config.out_sampling_rate = process_sample_rate;
    sw_src_config.out_frame_size_max = process_frame_sample * sizeof(int16_t);
    handle->src1_port = stream_function_sw_src_get_multi_port(sink);
    WIRED_AUDIO_LOG_MSGID_I("[LINE_OUT][open] input sw src init: 0x%08x, %d, %d, %d, %d, %d, %d, %d, %d", 9,
                                handle->src1_port,
                                sw_src_config.mode,
                                sw_src_config.channel_num,
                                sw_src_config.in_res,
                                sw_src_config.in_sampling_rate,
                                sw_src_config.in_frame_size_max,
                                sw_src_config.out_res,
                                sw_src_config.out_sampling_rate,
                                sw_src_config.out_frame_size_max);
    stream_function_sw_src_init((sw_src_port_t *)handle->src1_port, &sw_src_config);

    /* output SW SRC init */
    AUDIO_ASSERT(handle->src2_port == NULL);
    sw_src_config.mode = SW_SRC_MODE_NORMAL;
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) && defined(AIR_DUAL_CHIP_NR_ON_MASTER_ENABLE) /* && defined(AIR_BTA_IC_PREMIUM_G2) */
    sw_src_config.channel_num = handle->out_channel_num;
#else
#ifdef AIR_VOICE_NR_ENABLE
    sw_src_config.channel_num = 1;
#else
    sw_src_config.channel_num = handle->out_channel_num;
#endif
#endif
    sw_src_config.in_res = RESOLUTION_16BIT;
    sw_src_config.in_sampling_rate  = process_sample_rate;
    sw_src_config.in_frame_size_max = process_frame_sample * sizeof(int16_t);
    sw_src_config.out_res = RESOLUTION_16BIT;
    sw_src_config.out_sampling_rate = sink->param.audio.src_rate;
    sw_src_config.out_frame_size_max = (sw_src_config.out_sampling_rate * process_frame_sample * sizeof(int16_t)) / sw_src_config.in_sampling_rate;
    handle->src2_port = stream_function_sw_src_get_multi_port(sink);
    WIRED_AUDIO_LOG_MSGID_I("[LINE_OUT][open] output sw src init: 0x%08x, %d, %d, %d, %d, %d, %d, %d, %d", 9,
                                handle->src2_port,
                                sw_src_config.mode,
                                sw_src_config.channel_num,
                                sw_src_config.in_res,
                                sw_src_config.in_sampling_rate,
                                sw_src_config.in_frame_size_max,
                                sw_src_config.out_res,
                                sw_src_config.out_sampling_rate,
                                sw_src_config.out_frame_size_max);
    stream_function_sw_src_init((sw_src_port_t *)handle->src2_port, &sw_src_config);

    /* SW GAIN init */
    AUDIO_ASSERT(handle->gain_port == NULL);
    default_config.resolution = RESOLUTION_16BIT;
    default_config.target_gain = 0;
    default_config.up_step = 1;
    default_config.up_samples_per_step = 2;
    default_config.down_step = -1;
    default_config.down_samples_per_step = 2;
#if defined(AIR_DCHS_MODE_ENABLE)
    total_channels = 1;
#elif defined(AIR_VOICE_NR_ENABLE)
    total_channels = 1;
#else
    total_channels = handle->out_channel_num;
#endif
    handle->gain_port = stream_function_sw_gain_get_port(sink);
    WIRED_AUDIO_LOG_MSGID_I("[LINE_OUT][open] sw gain init: 0x%08x, %d, %d, %d, %d, %d, %d, %d", 8,
                                handle->gain_port,
                                total_channels,
                                default_config.resolution,
                                default_config.target_gain,
                                default_config.up_step,
                                default_config.up_samples_per_step,
                                default_config.down_step,
                                default_config.down_samples_per_step);
    stream_function_sw_gain_init(handle->gain_port, total_channels, &default_config);
}

static void wired_audio_line_out_start(mcu2dsp_start_param_p start_param, SOURCE source, SINK sink)
{
    UNUSED(start_param);
    UNUSED(source);
    UNUSED(sink);

#ifndef AIR_DCHS_MODE_ENABLE
    if (source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_OUT) {
        uint32_t afe_start_time;
        hal_audio_trigger_start_parameter_t start_parameter;
        hal_audio_current_offset_parameter_t offset_parameter;
        hal_audio_memory_irq_enable_parameter_t irq_enable;

        start_parameter.memory_select = sink->param.audio.mem_handle.memory_select | source->param.audio.mem_handle.memory_select;
        start_parameter.enable = true;
        hal_audio_set_value((hal_audio_set_value_parameter_t *)&start_parameter, HAL_AUDIO_SET_TRIGGER_MEMORY_START);

        irq_enable.memory_select    = sink->param.audio.mem_handle.memory_select;
        irq_enable.irq_counter      = sink->param.audio.mem_handle.irq_counter;
        irq_enable.rate             = sink->param.audio.mem_handle.audio_path_rate;
        irq_enable.enable           = true;
        hal_audio_set_value((hal_audio_set_value_parameter_t *)&irq_enable, HAL_AUDIO_SET_MEMORY_IRQ_ENABLE);

        offset_parameter.memory_select = sink->param.audio.mem_handle.memory_select;
        offset_parameter.pure_agent_with_src = false;
        hal_audio_get_value((hal_audio_get_value_parameter_t *)&offset_parameter, HAL_AUDIO_GET_MEMORY_OUTPUT_CURRENT_OFFSET);
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &afe_start_time);
        WIRED_AUDIO_LOG_MSGID_I("[LINE_OUT][start] start AFE agent %d, sink rptr %d, start time %d", 3,
                                    start_parameter.memory_select,
                                    offset_parameter.offset - offset_parameter.base_address,
                                    afe_start_time);
    }
#endif
}


/* Do some operations after the stream process done */
void wired_audio_line_out_post_hook(SOURCE source, SINK sink)
{
    DSP_STREAMING_PARA_PTR ul_stream;

    /* As the ECNR will change the out_channel_num, so we need to reset it */
    ul_stream = DSP_Streaming_Get(source, sink);
    ul_stream->callback.EntryPara.out_channel_num = ul_stream->callback.EntryPara.in_channel_num;
}

static void wired_audio_line_out_close(SOURCE source, SINK sink)
{
    line_out_local_param_t *line_out_param;
    wired_audio_handle_t *line_out_handle;

    UNUSED(source);
    UNUSED(sink);

    line_out_param = g_wired_audio_line_out_param;
    line_out_handle = (wired_audio_handle_t *)line_out_param->handle;

    g_wired_audio_line_out_param = NULL;

    if (line_out_handle->gain_port != NULL) {
        stream_function_sw_gain_deinit(line_out_handle->gain_port);
        line_out_handle->gain_port = NULL;
    }

    if (line_out_handle->src1_port != NULL) {
        stream_function_sw_src_deinit((sw_src_port_t *)line_out_handle->src1_port);
        line_out_handle->src1_port = NULL;
    }

    if (line_out_handle->src2_port != NULL) {
        stream_function_sw_src_deinit((sw_src_port_t *)line_out_handle->src2_port);
        line_out_handle->src2_port = NULL;
    }

    if (line_out_handle != NULL) {
        wired_audio_release_handle(line_out_handle);
    }
    if (line_out_param != NULL) {
        free(line_out_param);
    }
}

void wired_audio_scenario_config(audio_transmitter_scenario_sub_id_t sub_id, void *config_param, SOURCE source, SINK sink)
{
    uint32_t i, irq_saved_mask;
    wired_audio_runtime_config_param_p p_config_param;
    DSP_STREAMING_PARA_PTR ul_stream;
    int32_t new_gain;
    int32_t new_gain_array[8];
    uint32_t input_channel_number;
    sw_gain_config_t old_config;
    sw_gain_port_t *gain_port;
    wired_audio_handle_t *out_handle;
#if 0
#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
    int32_t *p_new_chat_gain, *p_new_game_gain, *p_new_game_balance_gain;
#endif
#endif

    UNUSED(irq_saved_mask);
    UNUSED(sub_id);

    p_config_param = (wired_audio_runtime_config_param_p)config_param;
    WIRED_AUDIO_LOG_MSGID_I("[config] config_operation %d", 1, p_config_param->config_operation);
    switch (p_config_param->config_operation) {
        case WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_MUSIC_L:
        case WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_MUSIC_R:
        case WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_MUSIC_CH_3:
        case WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_MUSIC_CH_4:
        case WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_MUSIC_CH_5:
        case WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_MUSIC_CH_6:
        case WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_MUSIC_CH_7:
        case WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_MUSIC_CH_8:
            if ((sub_id.wiredaudio_id != AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) &&
                (sub_id.wiredaudio_id != AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1)) {
                WIRED_AUDIO_LOG_MSGID_W("[config] cmd %d, USB_IN scenario is not running, current scenario id %d", 2, p_config_param->config_operation, sub_id);
                return;
            }
            input_channel_number = source->param.data_dl.scenario_param.usb_in_local_param.usb_in_param.codec_param.pcm.channel_mode;
            if ((p_config_param->config_operation >= WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_MUSIC_CH_3) && (input_channel_number != 8)) {
                WIRED_AUDIO_LOG_MSGID_W("[config] cmd %d, USB_IN scenario channel not enough %d", 2, p_config_param->config_operation, input_channel_number);
                return;
            }

            gain_port = stream_function_sw_gain_get_port(source);
            new_gain = p_config_param->vol_level.gain[p_config_param->config_operation - WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_MUSIC_L];
            stream_function_sw_gain_get_config(gain_port, (p_config_param->config_operation + 1 - WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_MUSIC_L), &old_config);
            WIRED_AUDIO_LOG_MSGID_I("[config] wired_audio_usb_in change channel %d gain from %d*0.01dB to %d*0.01dB\r\n", 3,
                         (p_config_param->config_operation + 1 - WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_MUSIC_L),
                         old_config.target_gain,
                         new_gain);

#if 0
#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
            hal_nvic_save_and_set_interrupt_mask(&irq_saved_mask);

            p_new_chat_gain = &(wired_audio_usb_vol_balance_handle.chat_path_default_vol_level.gain[p_config_param->config_operation - WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_MUSIC_L]);
            p_new_game_gain = &(wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[p_config_param->config_operation - WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_MUSIC_L]);
            p_new_game_balance_gain = &(wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[p_config_param->config_operation - WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_MUSIC_L]);
            if (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) {
                *p_new_chat_gain = new_gain;
            } else if (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1) {
                *p_new_game_gain = new_gain;
                if (wired_audio_usb_vol_balance_handle.enable == true) {
                    if (*p_new_game_balance_gain < new_gain) {
                        WIRED_AUDIO_LOG_MSGID_W("[config] wired_audio_usb_in balance change channel %d gain from %d*0.01dB to %d*0.01dB\r\n", 3,
                                                    p_config_param->config_operation, new_gain, *p_new_game_balance_gain);
                        new_gain = *p_new_game_balance_gain;
                    } else {
                        *p_new_game_balance_gain = new_gain;
                    }
                } else {
                    *p_new_game_balance_gain = new_gain;
                }
            }

            hal_nvic_restore_interrupt_mask(irq_saved_mask);
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */
#endif
            stream_function_sw_gain_configure_gain_target(gain_port, (p_config_param->config_operation + 1 - WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_MUSIC_L), new_gain);
            break;

        case WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_MUSIC_DUL:
        case WIRED_AUDIO_CONFIG_OP_VOL_DB_MUSIC:
            if ((sub_id.wiredaudio_id != AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) &&
                (sub_id.wiredaudio_id != AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1)) {
                WIRED_AUDIO_LOG_MSGID_W("[config] cmd %d, USB_IN scenario is not running, current scenario id %d", 2, p_config_param->config_operation, sub_id);
                return;
            }

            input_channel_number = source->param.data_dl.scenario_param.usb_in_local_param.usb_in_param.codec_param.pcm.channel_mode;
            if (input_channel_number > 8) {
                WIRED_AUDIO_LOG_MSGID_W("[config] cmd %d, USB_IN scenario channel too many %d", 2, p_config_param->config_operation, input_channel_number);
                return;
            }

            gain_port = stream_function_sw_gain_get_port(source);
            for (i = 0; i < input_channel_number; i++) {
                new_gain_array[i] = p_config_param->vol_level.gain[i];
                stream_function_sw_gain_get_config(gain_port, i + 1, &old_config);
                WIRED_AUDIO_LOG_MSGID_I("[config] wired_audio_usb_in change channel %d gain from %d*0.01dB to %d*0.01dB\r\n", 3,
                             i + 1,
                             old_config.target_gain,
                             new_gain_array[i]);
            }

#if 0
#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
            hal_nvic_save_and_set_interrupt_mask(&irq_saved_mask);

            if (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) {
                for (i = 0; i < input_channel_number; i++) {
                    wired_audio_usb_vol_balance_handle.chat_path_default_vol_level.gain[i] = p_config_param->vol_level.gain[i];
                }
            } else if (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1) {
                for (i = 0; i < input_channel_number; i++) {
                    p_new_game_gain = &(wired_audio_usb_vol_balance_handle.game_path_default_vol_level.gain[i]);
                    p_new_game_balance_gain = &(wired_audio_usb_vol_balance_handle.game_path_balance_vol_level.gain[i]);
                    *p_new_game_gain = p_config_param->vol_level.gain[i];
                    if (wired_audio_usb_vol_balance_handle.enable == true) {
                        if (*p_new_game_balance_gain < p_config_param->vol_level.gain[i]) {
                            WIRED_AUDIO_LOG_MSGID_W("[config] wired_audio_usb_in balance change channel %d gain from %d*0.01dB to %d*0.01dB\r\n", 3,
                                                        p_config_param->config_operation, new_gain_array[i], *p_new_game_balance_gain);
                            new_gain_array[i] = *p_new_game_balance_gain;
                        } else {
                            *p_new_game_balance_gain = p_config_param->vol_level.gain[i];
                        }
                    } else {
                        *p_new_game_balance_gain = p_config_param->vol_level.gain[i];
                    }
                }
            }

            hal_nvic_restore_interrupt_mask(irq_saved_mask);
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */
#endif

            for (i = 0; i < input_channel_number; i++) {
                stream_function_sw_gain_configure_gain_target(gain_port, i + 1, new_gain_array[i]);
            }
            break;

        case WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_VOICE_DUL:
        case WIRED_AUDIO_CONFIG_OP_VOL_DB_VOICE:
            if ((sub_id.wiredaudio_id != AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT) &&
                (sub_id.wiredaudio_id != AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_OUT_IEM) &&
                (sub_id.wiredaudio_id != AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT) &&
                (sub_id.wiredaudio_id != AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER)) {
                WIRED_AUDIO_LOG_MSGID_W("[config] cmd %d, USB_OUT scenario is not running, current scenario id %d", 2, p_config_param->config_operation, sub_id);
                return;
            }
            ul_stream = DSP_Streaming_Get(source, sink);
            input_channel_number = ul_stream->callback.EntryPara.in_channel_num;
            if (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT) {
                out_handle = (wired_audio_handle_t *)sink->param.data_ul.scenario_param.usb_out_local_param.handle;
                gain_port = out_handle->gain_port;
            } else if (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_OUT_IEM) {
                gain_port = g_usb_in_out_sw_gain_port;
            } else {
                out_handle = (wired_audio_handle_t *)(g_wired_audio_line_out_param->handle);
                gain_port = out_handle->gain_port;
            }
            for (i = 0; i < input_channel_number; i++) {
                new_gain = p_config_param->vol_level.gain[0];
                stream_function_sw_gain_get_config(gain_port, i + 1, &old_config);
                WIRED_AUDIO_LOG_MSGID_I("[config] change channel%d gain from %d*0.01dB to %d*0.01dB\r\n", 3,
                             input_channel_number,
                             old_config.target_gain,
                             new_gain);
                stream_function_sw_gain_configure_gain_target(gain_port, i + 1, new_gain);
            }
#ifdef AIR_MUTE_MIC_DETECTION_ENABLE
            if(sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT){
                if(p_config_param->vol_level.gain[0] <= -9600){
                    g_call_mute_flag = true;
                }else{
                    g_call_mute_flag = false;
                }
            }
#endif
            break;

        default:
            WIRED_AUDIO_LOG_MSGID_E("[config] wired_audio_usb_in error config_operation: %d\r\n", 1,
                         p_config_param->config_operation);

    }
}

void wired_audio_configure_task(audio_transmitter_scenario_sub_id_t sub_id, SOURCE source, SINK sink)
{
    UNUSED(source);
    UNUSED(sink);

    if ((sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) ||
        (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1) ||
        (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_MAINSTREAM)) {
        source->taskId = DHP_TASK_ID;
    }
    else if(sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_OUT_IEM) {
        source->taskId = DAV_TASK_ID;
        sink->taskid = DAV_TASK_ID;
    }
}

void wired_audio_open(audio_transmitter_scenario_sub_id_t sub_id, mcu2dsp_open_param_p open_param, SOURCE source, SINK sink)
{
    if ((sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN) ||
        (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER)) {
        wired_audio_line_in_open(open_param, source, sink);
    } else if ((sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT) ||
                (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER) ||
        (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE)) {
        wired_audio_line_out_open(open_param, source, sink);
    } else if (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT) {
        if (open_param->stream_out_param.data_ul.scenario_param.wired_audio_param.is_with_ecnr == true) {
        wired_audio_usb_out_open(open_param, source, sink);
        } else {
            wired_audio_usb_out_iem_open(open_param, source, sink);
        }
    }
}

void wired_audio_para_setup(audio_transmitter_scenario_sub_id_wiredaudio_t sub_id, DSP_STREAMING_PARA_PTR stream, SOURCE source, SINK sink)
{
    uint32_t sample_bytes;
    audio_codec_pcm_t *pcm_para;

    if ((sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) ||
        (sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1)) {
        pcm_para = &(source->param.data_dl.scenario_param.usb_in_local_param.usb_in_param.codec_param.pcm);
        stream->callback.EntryPara.in_size = ((sizeof(int32_t) * pcm_para->sample_rate) / 1000) * ((pcm_para->frame_interval + WIRED_AUDIO_USB_IN_PREFILL_TIME) / 1000);
        stream->callback.EntryPara.in_channel_num = pcm_para->channel_mode;
        stream->callback.EntryPara.in_sampling_rate = pcm_para->sample_rate / 1000; /* 44.1/88.2 exclued */
        stream->callback.EntryPara.codec_out_sampling_rate  = stream->callback.EntryPara.in_sampling_rate;
        if ((g_wired_audio_main_stream_source != source)) {
            /* As the non 1st USB card use virtual sink type, so we must set the codec_out_size here. */
            stream->callback.EntryPara.codec_out_size = (WIRED_AUDIO_MIXING_MODE_USB_DL_OUT_SAMPLE_RATE * sizeof(uint32_t) * pcm_para->frame_interval) / (1000 * 1000);
            stream->callback.EntryPara.out_channel_num         = 2;
            //stream->callback.EntryPara.codec_out_sampling_rate = FS_RATE_16K;
        }
        WIRED_AUDIO_LOG_MSGID_I("[USB_IN] wired_audio_para_setup: sub id %d, %d, %d, %d, %d, %d", 6,
                                    sub_id, pcm_para->format,
                                    stream->callback.EntryPara.in_size,
                                    stream->callback.EntryPara.in_channel_num,
                                    stream->callback.EntryPara.in_sampling_rate,
                                    stream->callback.EntryPara.codec_out_sampling_rate);
    } else if (sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT) {
        pcm_para = &(sink->param.data_ul.scenario_param.usb_out_local_param.codec_param.pcm);
        if (pcm_para->format >= HAL_AUDIO_PCM_FORMAT_S24_LE) {
            sample_bytes = 4;
        } else {
            sample_bytes = 2;
        }
        if(stream->sink->param.data_ul.scenario_param.usb_out_local_param.is_with_ecnr == true) {
            stream->callback.EntryPara.out_channel_num = stream->callback.EntryPara.in_channel_num;
        } else {
            stream->callback.EntryPara.out_channel_num = stream->callback.EntryPara.in_channel_num - 1;// - echo channel;
        }
        stream->callback.EntryPara.codec_out_size = (sample_bytes * (pcm_para->sample_rate / 1000) * pcm_para->frame_interval) / 1000;
        stream->callback.EntryPara.codec_out_sampling_rate = pcm_para->sample_rate;
        WIRED_AUDIO_LOG_MSGID_I("[USB_OUT] wired_audio_para_setup: sub id %d, %d, %d, %d, %d", 5,
                                    sub_id, sample_bytes,
                                    stream->callback.EntryPara.out_channel_num,
                                    stream->callback.EntryPara.codec_out_size,
                                    stream->callback.EntryPara.codec_out_sampling_rate);
    }
}

void wired_audio_resolution_config(DSP_STREAMING_PARA_PTR stream)
{
    audio_codec_pcm_t *pcm_para;

    if ((stream->source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0) ||
               (stream->source->scenario_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_1)) {
        pcm_para = &(stream->source->param.data_dl.scenario_param.usb_in_local_param.usb_in_param.codec_param.pcm);
        stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_32BIT;
        stream->callback.EntryPara.resolution.feature_res = RESOLUTION_32BIT;
        WIRED_AUDIO_LOG_MSGID_I("[USB_IN] wired_audio_resolution_config: %d, %d, %d", 3, stream->source->scenario_type, pcm_para->format, stream->callback.EntryPara.resolution.source_in_res);
    } else if (stream->sink->param.data_ul.scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT) {
        if(stream->sink->param.data_ul.scenario_param.usb_out_local_param.is_with_ecnr == true){
            pcm_para = &(stream->sink->param.data_ul.scenario_param.usb_out_local_param.codec_param.pcm);
            stream->callback.EntryPara.resolution.sink_out_res = RESOLUTION_16BIT;
            WIRED_AUDIO_LOG_MSGID_I("[USB_OUT] wired_audio_resolution_config: %d, %d, %d", 3, stream->sink->scenario_type, pcm_para->format, stream->callback.EntryPara.resolution.sink_out_res);
        } else {
            pcm_para = &(stream->sink->param.data_ul.scenario_param.usb_out_local_param.codec_param.pcm);
            if (pcm_para->format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
                stream->callback.EntryPara.resolution.sink_out_res = RESOLUTION_32BIT;
                stream->callback.EntryPara.resolution.process_res = RESOLUTION_32BIT;
                stream->callback.EntryPara.resolution.source_in_res = RESOLUTION_32BIT;
                stream->callback.EntryPara.resolution.feature_res = RESOLUTION_32BIT;
            }
            WIRED_AUDIO_LOG_MSGID_I("[USB_OUT] wired_audio_resolution_config: %d, %d, %d", 3, stream->sink->scenario_type, pcm_para->format, stream->callback.EntryPara.resolution.sink_out_res);
        }
    }
}

void wired_audio_start(audio_transmitter_scenario_sub_id_t sub_id, mcu2dsp_start_param_p start_param, SOURCE source, SINK sink)
{
    UNUSED(start_param);

    if ((sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) ||
        (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1)) {
        wired_audio_usb_in_start(start_param, source, sink);
    } else if (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_MAINSTREAM) {
        wired_audio_main_stream_start(start_param, source, sink);
    } else if (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT) {
        if(sink->param.data_ul.scenario_param.usb_out_local_param.is_with_ecnr == true){
            wired_audio_usb_out_start(start_param, source, sink);
        } else {
            wired_audio_usb_out_iem_start(start_param, source, sink);
        }
    } else if ((sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN) ||
                (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER)) {
        wired_audio_line_in_start(start_param, source, sink);
    } else if ((sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT) ||
                (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER) ||
                (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE)) {
        wired_audio_line_out_start(start_param, source, sink);
    }
#if !defined (AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE)
    if ((g_wired_audio_main_stream_source != NULL) &&
        ((sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) ||
                (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1) ||
         (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_OUT_IEM)) &&
        (g_wired_audio_main_stream_is_triggered == false)) {
        wired_audio_main_stream_trigger_active();
        g_wired_audio_main_stream_is_triggered = true;
    }
#endif
}

void wired_audio_close(audio_transmitter_scenario_sub_id_t sub_id, SOURCE source, SINK sink)
{
    if ((sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) ||
        (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1)) {
        wired_audio_usb_in_close(source, sink);
    } else if (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT) {
        if(sink->param.data_ul.scenario_param.usb_out_local_param.is_with_ecnr == true){
            wired_audio_usb_out_close(source, sink);
        } else {
            wired_audio_usb_out_iem_close(source, sink);
        }
    } else if ((sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN) ||
                (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER)) {
        wired_audio_line_in_close(source, sink);
    } else if ((sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT) ||
                (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER) ||
                (sub_id.wiredaudio_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE)) {
        wired_audio_line_out_close(source, sink);
    }
}

void wired_audio_main_stream_trigger_active(void)
{
    SINK sink;
    hal_audio_trigger_start_parameter_t start_parameter;
    hal_audio_current_offset_parameter_t offset_parameter;

    sink = g_wired_audio_main_stream_source->transform->sink;
    start_parameter.memory_select = sink->param.audio.mem_handle.memory_select;
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
    start_parameter.memory_select |= sink->param.audio.mem_handle1.memory_select | sink->param.audio.mem_handle2.memory_select;
    WIRED_AUDIO_LOG_MSGID_I("[MAIN_STREAM][start] 0x%x,0x%x,0x%x,0x%x,", 4,
                                        start_parameter.memory_select,
                                        sink->param.audio.mem_handle.memory_select,
                                        sink->param.audio.mem_handle1.memory_select,
                                        sink->param.audio.mem_handle2.memory_select);
#endif
    start_parameter.enable = true;
    hal_audio_set_value((hal_audio_set_value_parameter_t *)&start_parameter, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
    offset_parameter.memory_select = sink->param.audio.mem_handle.memory_select;
    offset_parameter.pure_agent_with_src = false;
    hal_audio_get_value((hal_audio_get_value_parameter_t *)&offset_parameter, HAL_AUDIO_GET_MEMORY_OUTPUT_CURRENT_OFFSET);
    WIRED_AUDIO_LOG_MSGID_I("[MAIN_STREAM][start] start AFE agent %d, sink rptr %d, wptr %d", 3,
                                start_parameter.memory_select,
                                offset_parameter.offset - offset_parameter.base_address,
                                sink->streamBuffer.BufferInfo.WriteOffset);
}

static void wired_audio_main_stream_start(mcu2dsp_start_param_p start_param, SOURCE source, SINK sink)
{
    UNUSED(start_param);
    UNUSED(source);
    UNUSED(sink);
}

void wired_audio_main_stream_source_init(SOURCE source)
{
    sw_mixer_callback_config_t       callback_config;
    sw_mixer_input_channel_config_t  in_ch_config;
    sw_mixer_output_channel_config_t out_ch_config;

    g_wired_audio_main_stream_source = source;

    uint32_t src_dsp_frame_sample = (WIRED_AUDIO_MIXING_MODE_USB_DL_OUT_SAMPLE_RATE * WIRED_AUDIO_USB_IN_PROCESS_PERIOD) / (1000);
    stream_function_sw_mixer_init(SW_MIXER_PORT_0);

    callback_config.preprocess_callback = NULL;
    callback_config.postprocess_callback = wired_audio_main_stream_mixer_postcallback;

    /* As PEQ out use only 32bit, so set sw mixer to 32bit */
    in_ch_config.total_channel_number = 2;
    in_ch_config.resolution = RESOLUTION_32BIT;
    in_ch_config.input_mode = SW_MIXER_CHANNEL_MODE_NORMAL;//SW_MIXER_CHANNEL_MODE_FORCED_ENOUGH;
    in_ch_config.buffer_size = sizeof(int32_t) * src_dsp_frame_sample; // 2.5ms * 48K
    out_ch_config.total_channel_number = in_ch_config.total_channel_number;
    out_ch_config.resolution = in_ch_config.resolution;

    g_main_stream_mixer_member = stream_function_sw_mixer_member_create((void *)source,
                                                                        SW_MIXER_MEMBER_MODE_NO_BYPASS,
                                                                        &callback_config,
                                                                        &in_ch_config,
                                                                        &out_ch_config);
    /* it is the first dl stream */
    stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, g_main_stream_mixer_member, true);
    WIRED_AUDIO_LOG_MSGID_I("[MAIN_STREAM][config][start] mix first. \r\n", 0);
}

void wired_audio_main_stream_source_close(SOURCE source)
{
    UNUSED(source);
    stream_function_sw_mixer_member_unregister(SW_MIXER_PORT_0, g_main_stream_mixer_member);
    stream_function_sw_mixer_member_delete(g_main_stream_mixer_member);
    stream_function_sw_mixer_deinit(SW_MIXER_PORT_0);

    g_main_stream_mixer_member = NULL;
    g_wired_audio_main_stream_source = NULL;
    g_wired_audio_main_stream_is_triggered = false;
}

#endif


/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
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
#include "scenario_wired_audio.h"
#include "audio_transmitter_playback_port.h"
#ifdef AIR_AUDIO_DUMP_ENABLE
#include "audio_dump.h"
#endif
#include "bsp_audio_ext_codec.h"
#include "scenario_dongle_common.h"
#include "fixrate_control.h"

#define WIRED_AUDIO_UNUSED(x)  ((void)(x))

//#define AIR_WIRED_AUDIO_UT_DEBUG_ENABLE
#define WIRED_AUDIO_DEBUG_LOG

#ifdef HAL_DVFS_MODULE_ENABLED
#ifdef AIR_BTA_IC_PREMIUM_G2
#define WIRED_AUDIO_DVFS_SPEED              HAL_DVFS_HIGH_SPEED_208M    //
#if defined(AIR_LINE_IN_LATENCY_LOW)
#define WIRED_AUDIO_LINE_IN_DVFS_SPEED      HAL_DVFS_HIGH_SPEED_208M
#elif defined(AIR_LINE_IN_LATENCY_MEDIUM)
#define WIRED_AUDIO_LINE_IN_DVFS_SPEED      HAL_DVFS_FULL_SPEED_104M
#elif defined(AIR_LINE_IN_LATENCY_HIGH)
#define WIRED_AUDIO_LINE_IN_DVFS_SPEED      HAL_DVFS_HALF_SPEED_52M
#else
#define WIRED_AUDIO_LINE_IN_DVFS_SPEED      HAL_DVFS_HALF_SPEED_52M
#endif
#else
#define WIRED_AUDIO_DVFS_SPEED              HAL_DVFS_OPP_HIGH   //
#if defined(AIR_LINE_IN_LATENCY_LOW)
#define WIRED_AUDIO_LINE_IN_DVFS_SPEED      HAL_DVFS_OPP_HIGH
#elif defined(AIR_LINE_IN_LATENCY_MEDIUM)
#define WIRED_AUDIO_LINE_IN_DVFS_SPEED      HAL_DVFS_OPP_MID
#elif defined(AIR_LINE_IN_LATENCY_HIGH)
#define WIRED_AUDIO_LINE_IN_DVFS_SPEED      HAL_DVFS_OPP_LOW
#else
#define WIRED_AUDIO_LINE_IN_DVFS_SPEED      HAL_DVFS_OPP_LOW
#endif
#endif
#endif

#ifdef AIR_BTA_IC_PREMIUM_G2
#define WIRED_AUDIO_USB_IN_0_AVM_BUFFER_INDEX AUDIO_TRANSMITTER_SHARE_INFO_INDEX_WIRED_AUDIO_DSP_RECEIVE_FROM_MCU_0
#define WIRED_AUDIO_USB_IN_1_AVM_BUFFER_INDEX AUDIO_TRANSMITTER_SHARE_INFO_INDEX_WIRED_AUDIO_DSP_RECEIVE_FROM_MCU_1
#define WIRED_AUDIO_USB_OUT_AVM_BUFFER_INDEX AUDIO_TRANSMITTER_SHARE_INFO_INDEX_GAMING_MODE_DSP_SEND_TO_MCU
#else
#define WIRED_AUDIO_USB_IN_0_AVM_BUFFER_INDEX AUDIO_TRANSMITTER_SHARE_INFO_INDEX_WIRED_AUDIO_DSP_RECEIVE_FROM_MCU_0
#define WIRED_AUDIO_USB_IN_1_AVM_BUFFER_INDEX AUDIO_TRANSMITTER_SHARE_INFO_INDEX_WIRED_AUDIO_DSP_RECEIVE_FROM_MCU_1
#define WIRED_AUDIO_USB_OUT_AVM_BUFFER_INDEX AUDIO_TRANSMITTER_SHARE_INFO_INDEX_WIRED_AUDIO_DSP_SEND_TO_MCU
#endif

#define WIRED_AUDIO_LOG_MSGID_I(fmt, arg...)    LOG_MSGID_I(wired_audio, "[wired_audio]: "fmt, ##arg)
#define WIRED_AUDIO_LOG_MSGID_W(fmt, arg...)    LOG_MSGID_W(wired_audio, "[wired_audio]: "fmt, ##arg)
#define WIRED_AUDIO_LOG_MSGID_E(fmt, arg...)    LOG_MSGID_E(wired_audio, "[wired_audio]: "fmt, ##arg)
#define WIRED_AUDIO_DEBUG_LOG_MSGID_I(fmt, arg...)    LOG_MSGID_I(wired_audio_debug, "[wired_audio]: "fmt, ##arg)
#define WIRED_AUDIO_DEBUG_LOG_MSGID_W(fmt, arg...)    LOG_MSGID_W(wired_audio_debug, "[wired_audio]: "fmt, ##arg)
#define WIRED_AUDIO_DEBUG_LOG_MSGID_E(fmt, arg...)    LOG_MSGID_E(wired_audio_debug, "[wired_audio]: "fmt, ##arg)

log_create_module(wired_audio, PRINT_LEVEL_INFO);
log_create_module_variant(wired_audio_debug, DEBUG_LOG_OFF, PRINT_LEVEL_INFO);

static uint8_t is_ext_dac_initialized = false;
static bool usb_audio_line_in_started_flag = 0;
static bool usb_audio_line_out_started_flag = 0;

extern HAL_DSP_PARA_AU_AFE_CTRL_t audio_nvdm_HW_config;

int8_t mix_stream_start_number = 0;
// uint8_t usb_audio_use_afe_subid = 0;

uint8_t g_usb_volume_sub_id = 0;  // 1 for usb_in_0, 2 for usb_in_1
static uint8_t g_usb_in_0_volume_level = 0;
static uint8_t g_usb_in_1_volume_level = 0;

int8_t wired_audio_get_mix_stream_start_number(void)
{
    WIRED_AUDIO_LOG_MSGID_I("mix_stream_start_number = %d, get", 1, mix_stream_start_number);
    return mix_stream_start_number;
}
void wired_audio_add_mix_stream_start_number(void)
{
    mix_stream_start_number++;
    WIRED_AUDIO_LOG_MSGID_I("mix_stream_start_number = %d, ++", 1, mix_stream_start_number);
}
void wired_audio_minus_mix_stream_start_number(void)
{
    mix_stream_start_number--;
    WIRED_AUDIO_LOG_MSGID_I("mix_stream_start_number = %d, --", 1, mix_stream_start_number);
}
// uint8_t wired_audio_get_usb_use_afe_subid(void)
// {
//     WIRED_AUDIO_LOG_MSGID_I("usb_audio_use_afe_subid = %d, get", 1, usb_audio_use_afe_subid);
//     return usb_audio_use_afe_subid;
// }
// void wired_audio_set_usb_use_afe_subid(uint8_t sub_id)
// {
//     usb_audio_use_afe_subid = sub_id;
//     WIRED_AUDIO_LOG_MSGID_I("usb_audio_use_afe_subid = %d, set", 1, usb_audio_use_afe_subid);
// }

volatile bool g_is_usb_out_mix_usb_in = false;
bool wired_audio_get_usb_out_mix_usb_in_state(void)
{
    audio_src_srv_report("[USB_IN_OUT] wired_audio_get_usb_out_mix_usb_in_state: (%d)", 1, g_is_usb_out_mix_usb_in);
    return g_is_usb_out_mix_usb_in;
}

bool wired_audio_set_usb_out_mix_usb_in_state(bool state)
{
    g_is_usb_out_mix_usb_in = state;
    audio_src_srv_report("[USB_IN_OUT] wired_audio_set_usb_out_mix_usb_in_state: (%d)", 1, g_is_usb_out_mix_usb_in);
    return true;
}

#ifdef AIR_USB_AUDIO_ENABLE
#include "usbaudio_drv.h"

#define WIRED_AUDIO_USB_DEBUG_LANTENCY      1 /* For debug of latency for USB */

#define MAX_WIRED_AUDIO_USB_RX_EP_NUMBER    2
#define WIRED_AUDIO_USB_IRQ_INTERVAL        1000
#define WIRED_AUDIO_USB_OUT_MAX_SAMPLE_RATE 48000 /* Unit of USB frame number */
#ifdef AIR_BTA_IC_PREMIUM_G2
#define WIRED_AUDIO_USB_OUT_MAX_SAMPLE_BYTES  2 /* Support max 16bit */
#define WIRED_AUDIO_USB_OUT_MAX_CHANNEL       1
#else
#define WIRED_AUDIO_USB_OUT_MAX_SAMPLE_BYTES  3 /* Support max 24bit */
#define WIRED_AUDIO_USB_OUT_MAX_CHANNEL       2
#endif

#ifdef AIR_WIRELESS_MIC_TX_ENABLE
#define WIRED_AUDIO_USB_OUT_MAX_USB_FRAME_SIZE 288
#else
#define WIRED_AUDIO_USB_OUT_MAX_USB_FRAME_SIZE  ((WIRED_AUDIO_USB_OUT_MAX_SAMPLE_RATE / 1000) * WIRED_AUDIO_USB_OUT_MAX_CHANNEL * WIRED_AUDIO_USB_OUT_MAX_SAMPLE_BYTES)
#endif

static uint32_t g_usb_tx_cell_size = 0;
static uint32_t g_usb_rx_0_cell_size = 0;
static uint32_t g_usb_rx_1_cell_size = 0;
static uint32_t g_usb_rx_0_avm_buf_size = 0;
static uint32_t g_usb_rx_1_avm_buf_size = 0;
static bool g_usb_tx_copy_flag;
static audio_transmitter_block_header_t usb_stream_rx_header = {0, 0};
static bool g_usb_audio_first_tx_in_flag;
static bool g_usb_audio_first_rx_in_flag[MAX_WIRED_AUDIO_USB_RX_EP_NUMBER];
static uint32_t g_usb_tx_avm_payload_size;
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
static uint8_t g_all_zero_buffer[WIRED_AUDIO_USB_OUT_MAX_USB_FRAME_SIZE];
#endif
static uint8_t *s_usb_out_voice_data = NULL;

static bool usb_audio_usb_in_0_started_flag = 0;
static bool usb_audio_usb_in_1_started_flag = 0;
static bool usb_audio_usb_out_started_flag = 0;

static void usb_audio_rx_cb_wired_audio_0(void);
static void usb_audio_rx_cb_wired_audio_1(void);
static void usb_audio_tx_cb_wired_audio_0(void);

#ifdef AIR_WIRED_AUDIO_UT_DEBUG_ENABLE

#include "hal_gpt.h"

#define UT_WIRED_AUDIO_DEBUG_RX_BEGIN_MS 1123
#define UT_WIRED_AUDIO_DEBUG_TX_BEGIN_MS 1

static int16_t UT_sine_pcm[] = {
    0,
    12541,
    23167,
    30269,
    32767,
    30269,
    23167,
    12541,
    0,
    -12541,
    -23167,
    -30269,
    -32767,
    -30269,
    -23167,
    -12541
};

static uint32_t UT_USB_Audio_Get_Len_Received_Data(uint32_t port)
{
    if (port == 0) {
        return g_usb_rx_0_cell_size;
    } else {
        return g_usb_rx_1_cell_size;
    }
}

static void UT_USB_Audio_Rx_Buffer_Drop_Bytes(uint32_t port, uint32_t bytes)
{
    ((void)(port));
    ((void)(bytes));
}

static uint32_t UT_USB_Audio_Read_Data(uint32_t port, void *dst, uint32_t len)
{
    uint32_t i, cnt;
    uint8_t *p_buf;

    ((void)(port));

    cnt = len / sizeof(UT_sine_pcm);
    p_buf = (uint8_t *)dst;
    for (i = 0; i < cnt; i++) {
        memcpy(p_buf + i * sizeof(UT_sine_pcm), UT_sine_pcm, sizeof(UT_sine_pcm));
    }
    if (len % sizeof(UT_sine_pcm)) {
        memcpy(p_buf + i * sizeof(UT_sine_pcm), UT_sine_pcm, len % sizeof(UT_sine_pcm));
    }

    return len;
}

static uint32_t UT_USB_Audio_TX_SendData(uint8_t *data, uint8_t len)
{
    ((void)(data));

    return len;
}

static void UT_user_ut_rx_0_gpt_post_callback(void *user_data)
{
    usb_audio_rx_cb_wired_audio_0();
}

static void UT_user_ut_rx_1_gpt_post_callback(void *user_data)
{
    usb_audio_rx_cb_wired_audio_1();
}

static void UT_user_ut_tx_gpt_post_callback(void *user_data)
{
    usb_audio_tx_cb_wired_audio_0();
}

static void UT_user_ut_rx_0_gpt_prev_callback(void *user_data)
{
    hal_gpt_status_t status;

    status = hal_gpt_stop_timer(HAL_GPT_7);
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_stop_timer %d", 1, status);
    status = hal_gpt_deinit(HAL_GPT_7);
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_deinit %d", 1, status);
    status = hal_gpt_init(HAL_GPT_7);
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_init %d", 1, status);
    status = hal_gpt_register_callback(HAL_GPT_7, UT_user_ut_rx_0_gpt_post_callback, NULL);
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_register_callback %d", 1, status);
    status = hal_gpt_start_timer_ms(HAL_GPT_7, 1, HAL_GPT_TIMER_TYPE_REPEAT);
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_start_timer_ms %d", 1, status);
}

static void UT_user_ut_rx_1_gpt_prev_callback(void *user_data)
{
    hal_gpt_status_t status;

    status = hal_gpt_stop_timer(HAL_GPT_7);
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_stop_timer %d", 1, status);
    status = hal_gpt_deinit(HAL_GPT_7);
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_deinit %d", 1, status);
    status = hal_gpt_init(HAL_GPT_7);
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_init %d", 1, status);
    status = hal_gpt_register_callback(HAL_GPT_7, UT_user_ut_rx_1_gpt_post_callback, NULL);
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_register_callback %d", 1, status);
    status = hal_gpt_start_timer_ms(HAL_GPT_7, 1, HAL_GPT_TIMER_TYPE_REPEAT);
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_start_timer_ms %d", 1, status);
}

static void UT_usb_audio_rx_0_ut_init(void)
{
    hal_gpt_status_t status;

    status = hal_gpt_init(HAL_GPT_7);
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_init %d", 1, status);
    status = hal_gpt_register_callback(HAL_GPT_7, UT_user_ut_rx_0_gpt_prev_callback, NULL);
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_register_callback %d", 1, status);
    status = hal_gpt_start_timer_ms(HAL_GPT_7, UT_WIRED_AUDIO_DEBUG_RX_BEGIN_MS, HAL_GPT_TIMER_TYPE_ONE_SHOT);
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_start_timer_ms %d", 1, status);
}

static void UT_usb_audio_rx_1_ut_init(void)
{
    hal_gpt_status_t status;

    status = hal_gpt_init(HAL_GPT_7);
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_init %d", 1, status);
    status = hal_gpt_register_callback(HAL_GPT_7, UT_user_ut_rx_1_gpt_prev_callback, NULL);
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_register_callback %d", 1, status);
    status = hal_gpt_start_timer_ms(HAL_GPT_7, UT_WIRED_AUDIO_DEBUG_RX_BEGIN_MS, HAL_GPT_TIMER_TYPE_ONE_SHOT);
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_start_timer_ms %d", 1, status);
}

static void UT_usb_audio_tx_ut_init(void)
{
    hal_gpt_status_t status;

    status = hal_gpt_init(HAL_GPT_7);
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_init %d", 1, status);
    status = hal_gpt_register_callback(HAL_GPT_7, UT_user_ut_tx_gpt_post_callback, NULL);
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_register_callback %d", 1, status);
    status = hal_gpt_start_timer_ms(HAL_GPT_7, UT_WIRED_AUDIO_DEBUG_TX_BEGIN_MS, HAL_GPT_TIMER_TYPE_REPEAT);
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_start_timer_ms %d", 1, status);
}
#endif

static void internal_usb_audio_rx_cb_wired_audio(uint32_t ep_number)
{
    static uint32_t g_previous_count = 0;
    uint32_t curr_gpt_count, during_count, available_data_size, buf_size, rx_cell_size;
    uint8_t *p_source_buf = NULL;
    n9_dsp_share_info_t *p_dsp_info;

    if (ep_number == 0) {
        rx_cell_size = g_usb_rx_0_cell_size;
        p_dsp_info = (n9_dsp_share_info_t *)hal_audio_query_audio_transmitter_share_info(WIRED_AUDIO_USB_IN_0_AVM_BUFFER_INDEX);
    } else {
        rx_cell_size = g_usb_rx_1_cell_size;
        p_dsp_info = (n9_dsp_share_info_t *)hal_audio_query_audio_transmitter_share_info(WIRED_AUDIO_USB_IN_1_AVM_BUFFER_INDEX);
    }

    /* Query the size of USB buffer */
#ifdef AIR_WIRED_AUDIO_UT_DEBUG_ENABLE
    available_data_size = UT_USB_Audio_Get_Len_Received_Data(ep_number);
#else
    available_data_size = USB_Audio_Get_Len_Received_Data(ep_number);
#endif
    if (available_data_size != rx_cell_size) {
        WIRED_AUDIO_LOG_MSGID_E("[USB_IN][usb_irq] USB EP %d, USB buffer size check error, expect: %d, actual: %d\r\n", 3, ep_number, rx_cell_size, available_data_size);
    }

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_gpt_count);
    if (g_usb_audio_first_rx_in_flag[ep_number] == true) {
        /* Drop all data in the 1st IRQ handler */
#ifdef AIR_WIRED_AUDIO_UT_DEBUG_ENABLE
        UT_USB_Audio_Rx_Buffer_Drop_Bytes(ep_number, available_data_size);
#else
        USB_Audio_Rx_Buffer_Drop_Bytes(ep_number, available_data_size);
#endif
        g_usb_audio_first_rx_in_flag[ep_number] = false;
        WIRED_AUDIO_LOG_MSGID_I("[USB_IN][usb_irq] USB EP %d, 1st Rx IRQ is trigger, USB buffer size %d, cell size %d", 3, ep_number, available_data_size, rx_cell_size);
    } else {
        /* Check the USB IRQ interval */
        hal_gpt_get_duration_count(g_previous_count, curr_gpt_count, &during_count);
        if (during_count > 1500) {
            WIRED_AUDIO_LOG_MSGID_E("[USB_IN][usb_irq] USB EP %d, check irq period error: expect: %d duration = %d", 3, ep_number, WIRED_AUDIO_USB_IRQ_INTERVAL, during_count);
        }
    }
    g_previous_count = curr_gpt_count;

    /* Do loop to copy data from USB buffer to AVM buffer */
    p_dsp_info->bBufferIsFull = 0;
    while (available_data_size > 0) {
        /* Get the dst buffer address */
        hal_audio_buf_mgm_get_free_buffer(p_dsp_info, &p_source_buf, &buf_size);
#ifdef WIRED_AUDIO_DEBUG_LOG
        WIRED_AUDIO_DEBUG_LOG_MSGID_I("[USB_IN][usb_irq] USB EP %d, Copy data to AVM buffer, %d, %d, 0x%08x, 0x%08x, 0x%08x, %d, %d, %d, %d", 10,
                                    ep_number,
                                    rx_cell_size,
                                    buf_size,
                                    p_source_buf,
                                    (uint32_t)p_dsp_info,
                                    p_dsp_info->start_addr,
                                    p_dsp_info->read_offset,
                                    p_dsp_info->write_offset,
                                    p_dsp_info->bBufferIsFull,
                                    p_dsp_info->length);
#endif

        /* Update and copy header to AVM buffer  */
        usb_stream_rx_header.sequence_number++;
        usb_stream_rx_header.data_length = rx_cell_size;
        memcpy(p_source_buf, &usb_stream_rx_header, BLK_HEADER_SIZE);

        /* Copy the pcm audio data from USB buffer to AVM buffer */
#ifdef AIR_WIRED_AUDIO_UT_DEBUG_ENABLE
        UT_USB_Audio_Read_Data(ep_number, p_source_buf + BLK_HEADER_SIZE, rx_cell_size);
#else
        USB_Audio_Read_Data(ep_number, p_source_buf + BLK_HEADER_SIZE, rx_cell_size);
#endif
        if (available_data_size % rx_cell_size) {
            memset(p_source_buf + BLK_HEADER_SIZE, 0, rx_cell_size);
        }
#if WIRED_AUDIO_USB_DEBUG_LANTENCY
        audio_usb_rx_scenario_latency_debug(ep_number, p_source_buf + BLK_HEADER_SIZE);
#endif
        hal_audio_buf_mgm_get_write_data_done(p_dsp_info, rx_cell_size + BLK_HEADER_SIZE);

#ifdef AIR_AUDIO_DUMP_ENABLE
        //LOG_AUDIO_DUMP((uint8_t *)(p_source_buf + BLK_HEADER_SIZE), rx_cell_size, AUDIO_CPD_OUT_L);
#endif

        /* Query the latest size in the USB buffer */
#ifdef AIR_WIRED_AUDIO_UT_DEBUG_ENABLE
        available_data_size = 0;
#else
        available_data_size = USB_Audio_Get_Len_Received_Data(ep_number);
#endif
    }
}

static void usb_audio_rx_cb_wired_audio_0(void)
{
    internal_usb_audio_rx_cb_wired_audio(0);
}

static void usb_audio_rx_cb_wired_audio_1(void)
{
    internal_usb_audio_rx_cb_wired_audio(1);
}

static void usb_audio_tx_cb_wired_audio_0(void)
{
    static uint32_t g_previous_count = 0;
    static uint32_t g_saved_voice_data_len = 0;
    static uint32_t s_voice_data_offset = 0;
    static uint32_t s_voice_data_len = 0;
    n9_dsp_share_info_t *p_dsp_info;
    uint32_t curr_gpt_count, during_count, avail_size;

    p_dsp_info = (n9_dsp_share_info_t *)hal_audio_query_audio_transmitter_share_info(WIRED_AUDIO_USB_OUT_AVM_BUFFER_INDEX);

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &curr_gpt_count);
    if (g_usb_audio_first_tx_in_flag == true) {
        /* Check tirgger threshold */
        avail_size = hal_audio_buf_mgm_get_data_byte_count(p_dsp_info);
        if (avail_size == 0) {
            WIRED_AUDIO_LOG_MSGID_W("[USB_OUT][usb_irq] USB irq is triggered too early, query AVM buffer empty!!!", 0);
        } else {
            if (avail_size > (g_usb_tx_avm_payload_size + sizeof(audio_transmitter_block_header_t))) {
                hal_audio_buf_mgm_get_read_data_done(p_dsp_info, avail_size - (g_usb_tx_avm_payload_size + sizeof(audio_transmitter_block_header_t)));
                WIRED_AUDIO_LOG_MSGID_W("[USB_OUT][usb_irq] USB irq is triggered too late, avail_size %d, consume %d", 2, avail_size, avail_size - (g_usb_tx_avm_payload_size + sizeof(audio_transmitter_block_header_t)));
            }
        }
        p_dsp_info->notify_count = 1; /* Mark the USB irq has been triggered. */
        p_dsp_info->anchor_clk = 0;
        g_usb_audio_first_tx_in_flag = false;
        WIRED_AUDIO_LOG_MSGID_I("[USB_OUT][usb_irq] 1st USB Tx IRQ is trigger, cell size %d", 1, g_usb_tx_cell_size);
    } else {
        /* Check the USB IRQ interval */
        hal_gpt_get_duration_count(g_previous_count, curr_gpt_count, &during_count);
        if (during_count > 1500) {
            WIRED_AUDIO_LOG_MSGID_E("[USB_OUT][usb_irq] check USB irq period error: expect: %d duration = %d", 2, WIRED_AUDIO_USB_IRQ_INTERVAL, during_count);
        }
    }
    g_previous_count = curr_gpt_count;

    /* Check whether need to switch to the next DSP frame */
    if (s_usb_out_voice_data == NULL) {
        hal_audio_buf_mgm_get_data_buffer(p_dsp_info, &s_usb_out_voice_data, &g_saved_voice_data_len);
        s_voice_data_len = WIRED_AUDIO_USB_OUT_PROCESS_PERIOD * g_usb_tx_cell_size;
        s_usb_out_voice_data += sizeof(audio_transmitter_block_header_t);
        if (g_saved_voice_data_len == 0) {
            WIRED_AUDIO_LOG_MSGID_E("[USB_OUT][usb_irq] Not data in AVM share buffer, send silence data, p_dsp_info 0x%08x, read_offset %d, write_offset %d, bBufferIsFull %d", 4, (uint32_t)p_dsp_info, p_dsp_info->read_offset, p_dsp_info->write_offset, p_dsp_info->bBufferIsFull);
#ifndef AIR_WIRED_AUDIO_UT_DEBUG_ENABLE
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
            USB_Audio_TX_SendData(0, USB_Audio_Get_TX_Bytes(0), g_all_zero_buffer);
#endif
#endif
            //LOG_AUDIO_DUMP(g_all_zero_buffer, USB_Audio_Get_TX_Bytes(0), USB_AUDIO_TX1);
            s_usb_out_voice_data = NULL;
            /* Update the USB irq counter */
            p_dsp_info->anchor_clk++;
            return;
        }
        /* Latency control */
        if (g_usb_tx_copy_flag == true) {
            if (p_dsp_info->asi_cur--) {
                WIRED_AUDIO_LOG_MSGID_W("[USB_OUT][usb_irq] 1st MCU tx copy: bypass!!!", 0);
#ifndef AIR_WIRED_AUDIO_UT_DEBUG_ENABLE
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
                USB_Audio_TX_SendData(0, USB_Audio_Get_TX_Bytes(0), g_all_zero_buffer);
#endif
#endif
                s_usb_out_voice_data = NULL;
                /* Update the USB irq counter */
                p_dsp_info->anchor_clk++;
                return;
            } else {
                g_usb_tx_copy_flag = false;
            }
        }
        s_voice_data_offset = 0;
    }

    /* Send one USB frame from the DSP frame */
#ifdef AIR_WIRED_AUDIO_UT_DEBUG_ENABLE
    UT_USB_Audio_TX_SendData(s_usb_out_voice_data + s_voice_data_offset, g_usb_tx_cell_size);
#else
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
    if (USB_Audio_Get_TX_Bytes(0) != g_usb_tx_cell_size) {
        USB_Audio_TX_SendData(0, USB_Audio_Get_TX_Bytes(0), g_all_zero_buffer);
        WIRED_AUDIO_LOG_MSGID_E("[USB_OUT][usb_irq] size mismatch(request %d, actual %d), send silence data", 2, USB_Audio_Get_TX_Bytes(0), g_usb_tx_cell_size);
    } else {
        USB_Audio_TX_SendData(0, g_usb_tx_cell_size, s_usb_out_voice_data + s_voice_data_offset);
        //LOG_AUDIO_DUMP(s_usb_out_voice_data + s_voice_data_offset, g_usb_tx_cell_size, USB_AUDIO_TX1);
#if WIRED_AUDIO_USB_DEBUG_LANTENCY
        audio_usb_tx_scenario_latency_debug(0, s_usb_out_voice_data + s_voice_data_offset);
#endif
    }
#endif
#endif
    s_voice_data_offset += g_usb_tx_cell_size;

    /* One DSP frame has been consumed, then update RPTR of AVM buffer */
    if (s_voice_data_offset >= s_voice_data_len) {
        hal_audio_buf_mgm_get_read_data_done(p_dsp_info, g_usb_tx_avm_payload_size + sizeof(audio_transmitter_block_header_t));
        s_usb_out_voice_data = NULL;
        s_voice_data_offset = 0;
#ifdef WIRED_AUDIO_DEBUG_LOG
        WIRED_AUDIO_DEBUG_LOG_MSGID_I("[USB_OUT][usb_irq] Send one DSP frame with size %d done", 1, s_voice_data_len);
#endif
    }

    /* Update the USB irq counter */
    p_dsp_info->anchor_clk++;

#ifdef WIRED_AUDIO_DEBUG_LOG
    WIRED_AUDIO_DEBUG_LOG_MSGID_I("[USB_OUT][usb_irq] exit, bBufferIsFull %d, read_offset %d, write_offset %d", 3, p_dsp_info->bBufferIsFull, p_dsp_info->read_offset, p_dsp_info->write_offset);
#endif
}
#endif

static void input_parameter_check(audio_codec_pcm_t *pcm_para)
{
    if ((pcm_para->sample_rate != 16000) &&
        (pcm_para->sample_rate != 24000) &&
        (pcm_para->sample_rate != 32000) &&
        (pcm_para->sample_rate != 48000) &&
        (pcm_para->sample_rate != 96000) &&
        (pcm_para->sample_rate != 192000)) {
        WIRED_AUDIO_LOG_MSGID_E("Input parameter sample_rate invalid %d", 1, pcm_para->sample_rate);
        AUDIO_ASSERT(0);
        return;
    }
    if ((pcm_para->channel_mode != 1) && (pcm_para->channel_mode != 2) && (pcm_para->channel_mode != 8)) {
        WIRED_AUDIO_LOG_MSGID_E("Input parameter channel_mode invalid %d", 1, pcm_para->channel_mode);
        AUDIO_ASSERT(0);
        return;
    }
    if ((pcm_para->format != HAL_AUDIO_PCM_FORMAT_S16_LE) && (pcm_para->format != HAL_AUDIO_PCM_FORMAT_S24_LE) && (pcm_para->format != HAL_AUDIO_PCM_FORMAT_S32_LE)) {
        WIRED_AUDIO_LOG_MSGID_E("Input parameter format invalid %d", 1, pcm_para->format);
        AUDIO_ASSERT(0);
        return;
    }

    return;
}

static void line_in_external_device_config(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    WIRED_AUDIO_UNUSED(config);
    WIRED_AUDIO_UNUSED(open_param);

#if defined(MTK_EXTERNAL_DSP_NEED_SUPPORT)
    ami_set_afe_param(STREAM_OUT, hal_audio_sampling_rate_value_to_enum(open_param->stream_in_param.afe.sampling_rate), true);
#endif
}

static void line_out_external_device_config(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    if (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT) {
        // for special external DAC
#ifdef AIR_BTA_IC_PREMIUM_G2
#ifndef AIR_LINE_OUT_USE_DAC_ENABLE
        bsp_audio_ext_codec_dac_init(BSP_AUDIO_EXT_CODEC_TYPE_DA7212, NULL);
        bsp_audio_ext_codec_dac_enable(BSP_AUDIO_EXT_CODEC_TYPE_DA7212);
#endif
#endif
        is_ext_dac_initialized = true;
    }
}

void line_out_stream_out_afe_config(audio_transmitter_config_t *config, uint8_t scenario_sub_id, mcu2dsp_open_param_t *open_param)
{
    static afe_pcm_format_t open_afe_pcm_format;
    audio_codec_pcm_t *pcm;
#ifdef AIR_LINE_OUT_USE_DAC_ENABLE
    open_param->stream_out_param.afe.audio_device = HAL_AUDIO_DEVICE_DAC_DUAL;
#else
    open_param->stream_out_param.afe.audio_device = HAL_AUDIO_DEVICE_I2S_MASTER;
#endif
#if defined(AIR_BTA_IC_STEREO_HIGH_G3) && defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)
    open_param->stream_out_param.afe.audio_interface = HAL_AUDIO_INTERFACE_2;
#else
    open_param->stream_out_param.afe.audio_interface = HAL_AUDIO_INTERFACE_1;
#endif
    open_param->stream_out_param.afe.stream_channel = HAL_AUDIO_DIRECT;
#ifdef AIR_DCHS_MODE_ENABLE
    open_param->stream_out_param.afe.memory = HAL_AUDIO_MEM1;
#else
    open_param->stream_out_param.afe.memory = HAL_AUDIO_MEM4;
#endif

    if(config) {
        pcm = &(config->scenario_config.wired_audio_config.line_out_config.codec_param.pcm);
        open_param->stream_out_param.afe.format = pcm->format;
        open_afe_pcm_format = open_param->stream_out_param.afe.format;
    } else {
        open_param->stream_out_param.afe.format = open_afe_pcm_format;
    }


#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE)
    if (scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT) {
        if (config != NULL) {
            open_param->stream_out_param.afe.stream_out_sampling_rate = config->scenario_config.wired_audio_config.line_out_config.codec_param.pcm.sample_rate;
            open_param->stream_out_param.afe.sampling_rate = open_param->stream_out_param.afe.stream_out_sampling_rate;
        } else {
            open_param->stream_out_param.afe.stream_out_sampling_rate = open_param->stream_in_param.afe.sampling_rate;
            open_param->stream_out_param.afe.sampling_rate = open_param->stream_in_param.afe.sampling_rate;
        }
    } else if (scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER) {
        #if defined(AIR_FIXED_DL_SAMPLING_RATE_TO_96KHZ)
        open_param->stream_out_param.afe.stream_out_sampling_rate = 96000;
        open_param->stream_out_param.afe.sampling_rate = 96000;
        #else
        open_param->stream_out_param.afe.stream_out_sampling_rate = 48000;
        open_param->stream_out_param.afe.sampling_rate = 48000;
        #endif
    }
#else
    if (config != NULL) {
        open_param->stream_out_param.afe.stream_out_sampling_rate = config->scenario_config.wired_audio_config.line_out_config.codec_param.pcm.sample_rate;
        open_param->stream_out_param.afe.sampling_rate = open_param->stream_out_param.afe.stream_out_sampling_rate;
    } else {
        open_param->stream_out_param.afe.stream_out_sampling_rate = open_param->stream_in_param.afe.sampling_rate;
        open_param->stream_out_param.afe.sampling_rate = open_param->stream_in_param.afe.sampling_rate;
    }
#endif
    open_param->stream_out_param.afe.irq_period = WIRED_AUDIO_LINE_OUT_PROCESS_PERIOD;
    open_param->stream_out_param.afe.frame_size = (WIRED_AUDIO_LINE_OUT_PROCESS_PERIOD * open_param->stream_out_param.afe.sampling_rate) / 1000;
    open_param->stream_out_param.afe.frame_number = 3;
    open_param->stream_out_param.afe.hw_gain = false;
    ///TODO: should call API instead of using code flow here.
#ifdef MTK_AUDIO_HW_IO_CONFIG_ENHANCE
    open_param->stream_out_param.afe.performance = AFE_PEROFRMANCE_NORMAL_MODE;
    memset(open_param->stream_out_param.afe.i2s_master_format, 2, 4);//audio_nvdm_HW_config.I2SM_config.I2S0_Master_Format;
    memset(open_param->stream_out_param.afe.i2s_master_word_length, 1, 4);//audio_nvdm_HW_config.I2SM_config.I2S0_Master_Word_length;
#endif
    if (open_param->stream_out_param.afe.audio_device == HAL_AUDIO_DEVICE_I2S_MASTER) {
        open_param->stream_out_param.afe.misc_parms      = I2S_CLK_SOURCE_DCXO;
        //if (linein_sample_rate > HAL_AUDIO_SAMPLING_RATE_48KHZ) {
        //    open_param->stream_out_param.afe.misc_parms  = I2S_CLK_SOURCE_APLL;
        //}
    } else {
        open_param->stream_out_param.afe.misc_parms      = DOWNLINK_PERFORMANCE_NORMAL;
    }
}

#ifdef AIR_USB_AUDIO_ENABLE
static uint8_t wired_audio_get_sample_size(hal_audio_format_t format)
{
    if(format <= HAL_AUDIO_PCM_FORMAT_U8){
        return 1;
    } else if((format >= HAL_AUDIO_PCM_FORMAT_S16_LE)&&(format <= HAL_AUDIO_PCM_FORMAT_U16_BE)){
        return 2;
    } else if((format >= HAL_AUDIO_PCM_FORMAT_S24_LE)&&(format <= HAL_AUDIO_PCM_FORMAT_U24_BE)){
        return 3;
    } else if((format >= HAL_AUDIO_PCM_FORMAT_S32_LE)&&(format <= HAL_AUDIO_PCM_FORMAT_U32_BE)){
        return 4;
    } else {
        configASSERT(0);
        return 0;
    }
}
#endif

void audio_transmitter_wired_audio_open_playback(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param)
{
    audio_codec_pcm_t *pcm;
    sysram_status_t status;
    DSP_FEATURE_TYPE_LIST AudioFeatureList_wired_audio[3] = {FUNC_END};

    if (0) {
        /* Dummy if branch */
    }
#ifdef AIR_USB_AUDIO_ENABLE
    else if (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT) {        /* Parameter check */
        pcm = &(config->scenario_config.wired_audio_config.usb_out_config.codec_param.pcm);
        input_parameter_check(pcm);

        g_usb_tx_copy_flag = true;
        /* Calculate refer value for later init */
        if (pcm->format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
            g_usb_tx_cell_size = (pcm->sample_rate * pcm->channel_mode * 3) / 1000;
        } else {
            g_usb_tx_cell_size = (pcm->sample_rate * pcm->channel_mode * 2) / 1000;
        }

#if WIRED_AUDIO_USB_DEBUG_LANTENCY
        audio_usb_tx_scenario_latency_debug_init(0, g_usb_tx_cell_size, pcm->channel_mode, pcm->format, 5000);
#endif

        /* Config the feature list */
        if(config->scenario_config.wired_audio_config.usb_out_config.is_with_ecnr == false) {
            AudioFeatureList_wired_audio[0] = FUNC_END;
#ifdef AIR_LD_NR_ENABLE
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
            ami_mic_ld_nr_set_parameter(hal_audio_query_voice_mic_type());
#else
            ami_mic_ld_nr_set_parameter(VOICE_MIC_TYPE_FIXED);
#endif
#endif
#if defined(AIR_WIRELESS_MIC_TX_ENABLE)
            AudioFeatureList_wired_audio[0] = FUNC_WIRELESS_MIC;
            AudioFeatureList_wired_audio[1] = FUNC_END;
#endif
        } else if(config->scenario_config.wired_audio_config.usb_out_config.is_with_ecnr == true) {
            /* Config the feature list */
            if (config->scenario_config.wired_audio_config.usb_out_config.is_with_swb == true) {
                AudioFeatureList_wired_audio[0] = FUNC_RX_NR_SWB;
            } else {
                AudioFeatureList_wired_audio[0] = FUNC_RX_NR;
            }
#ifdef AIR_MUTE_MIC_DETECTION_ENABLE
            AudioFeatureList_wired_audio[1] = FUNC_SILENCE_DETECTION;
            AudioFeatureList_wired_audio[2] = FUNC_END;
#else
            AudioFeatureList_wired_audio[1] = FUNC_END;
#endif
    #ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
            voice_mic_type_t mic_cur_type = hal_audio_query_voice_mic_type();
            WIRED_AUDIO_LOG_MSGID_I("[USB_OUT]Wired line out mic_cur_type: 0x%x", 1, mic_cur_type);
            if (mic_cur_type == VOICE_MIC_TYPE_FIXED) {
                if (config->scenario_config.wired_audio_config.usb_out_config.is_with_swb == true) {
                    AudioFeatureList_wired_audio[0] = FUNC_RX_NR_SWB;
                } else {
                    AudioFeatureList_wired_audio[0] = FUNC_RX_NR;
                }
            } else if (mic_cur_type == VOICE_MIC_TYPE_DETACHABLE) {
                if (config->scenario_config.wired_audio_config.usb_out_config.is_with_swb == true) {
                    AudioFeatureList_wired_audio[0] = FUNC_SWB_BOOM_MIC;
                } else {
                    AudioFeatureList_wired_audio[0] = FUNC_WB_BOOM_MIC;
                }
            } else {
                WIRED_AUDIO_LOG_MSGID_E("[USB_OUT]Wired line out no this mic type - (%d)\r\n", 1, mic_cur_type);
                configASSERT(0);
            }
    #endif
        }

        /* Config the type of stream in and stream out */
        open_param->param.stream_in = STREAM_IN_AFE;
        open_param->param.stream_out = STREAM_OUT_AUDIO_TRANSMITTER;

        /* Config the structure of stream in */
        hal_audio_get_stream_in_setting_config(AU_DSP_VOICE, &open_param->stream_in_param);
        open_param->stream_in_param.afe.stream_channel  = HAL_AUDIO_DIRECT;
        open_param->stream_in_param.afe.memory          = HAL_AUDIO_MEM1 | HAL_AUDIO_MEM3;
        if(config->scenario_config.wired_audio_config.usb_out_config.is_with_ecnr == false) {
            open_param->stream_in_param.afe.memory          = HAL_AUDIO_MEM1;
            open_param->stream_in_param.afe.format          = config->scenario_config.wired_audio_config.usb_out_config.codec_param.pcm.format;
            open_param->stream_in_param.afe.sampling_rate   = config->scenario_config.wired_audio_config.usb_out_config.codec_param.pcm.sample_rate;
            open_param->stream_in_param.afe.irq_period      = WIRED_AUDIO_USB_OUT_PROCESS_PERIOD;
            open_param->stream_in_param.afe.frame_size      = (open_param->stream_in_param.afe.irq_period * open_param->stream_in_param.afe.sampling_rate) / 1000;
            config->scenario_config.wired_audio_config.usb_out_config.codec_param.pcm.frame_interval = WIRED_AUDIO_USB_OUT_PROCESS_PERIOD * 1000;
        } else if(config->scenario_config.wired_audio_config.usb_out_config.is_with_ecnr == true) {
            open_param->stream_in_param.afe.memory          = HAL_AUDIO_MEM1 | HAL_AUDIO_MEM3;
            open_param->stream_in_param.afe.format          = HAL_AUDIO_PCM_FORMAT_S16_LE;
#if defined(AIR_UL_FIX_SAMPLING_RATE_32K)
            open_param->stream_in_param.afe.sampling_rate   = 32000;
#elif defined(AIR_UL_FIX_SAMPLING_RATE_48K)
            open_param->stream_in_param.afe.sampling_rate   = 48000;
#else
            if (config->scenario_config.wired_audio_config.usb_out_config.is_with_swb == true) {
                open_param->stream_in_param.afe.sampling_rate   = 32000;
            } else {
                open_param->stream_in_param.afe.sampling_rate   = 16000;
            }
#endif
            open_param->stream_in_param.afe.irq_period      = WIRED_AUDIO_USB_OUT_PROCESS_PERIOD;
            open_param->stream_in_param.afe.frame_size      = (WIRED_AUDIO_USB_OUT_PROCESS_PERIOD * open_param->stream_in_param.afe.sampling_rate) / 1000;
        }
        open_param->stream_in_param.afe.frame_number    = 6;
        open_param->stream_in_param.afe.hw_gain         = false;

        /* Config the structure of stream out */
        if(config->scenario_config.wired_audio_config.usb_out_config.is_with_ecnr == false) {
            open_param->stream_out_param.data_ul.max_payload_size = open_param->stream_in_param.afe.frame_size * config->scenario_config.wired_audio_config.usb_out_config.codec_param.pcm.channel_mode
                                                                    * (wired_audio_get_sample_size(config->scenario_config.wired_audio_config.usb_out_config.codec_param.pcm.format));
        } else if(config->scenario_config.wired_audio_config.usb_out_config.is_with_ecnr == true) {
            open_param->stream_out_param.data_ul.max_payload_size = WIRED_AUDIO_USB_OUT_MAX_USB_FRAME_SIZE * WIRED_AUDIO_USB_OUT_PROCESS_PERIOD;
        }
        open_param->stream_out_param.data_ul.scenario_param.wired_audio_param.codec_param.pcm.frame_interval = WIRED_AUDIO_USB_OUT_PROCESS_PERIOD * 1000;
        open_param->stream_out_param.data_ul.scenario_type = config->scenario_type;
        open_param->stream_out_param.data_ul.scenario_sub_id = config->scenario_sub_id;
        open_param->stream_out_param.data_ul.p_share_info = hal_audio_query_audio_transmitter_share_info(WIRED_AUDIO_USB_OUT_AVM_BUFFER_INDEX);
        open_param->stream_out_param.data_ul.p_share_info->length = SHARE_BUFFER_BT_AUDIO_USB_OUT_SIZE;
        open_param->stream_out_param.data_ul.data_notification_frequency = 0;
        open_param->stream_out_param.data_ul.scenario_param.wired_audio_param.codec_type = config->scenario_config.wired_audio_config.usb_out_config.codec_type;
        open_param->stream_out_param.data_ul.scenario_param.wired_audio_param.codec_param.pcm.sample_rate = pcm->sample_rate;
        open_param->stream_out_param.data_ul.scenario_param.wired_audio_param.codec_param.pcm.channel_mode = pcm->channel_mode;
        open_param->stream_out_param.data_ul.scenario_param.wired_audio_param.codec_param.pcm.format = pcm->format;
        open_param->stream_out_param.data_ul.scenario_param.wired_audio_param.is_with_ecnr = config->scenario_config.wired_audio_config.usb_out_config.is_with_ecnr;
        open_param->stream_out_param.data_ul.scenario_param.wired_audio_param.is_with_swb = config->scenario_config.wired_audio_config.usb_out_config.is_with_swb;
        open_param->stream_out_param.data_ul.p_share_info->read_offset = 0;
        open_param->stream_out_param.data_ul.p_share_info->write_offset = 0;
        open_param->stream_out_param.data_ul.p_share_info->bBufferIsFull = false;
        open_param->stream_out_param.data_ul.p_share_info->notify_count = 0;
        open_param->stream_out_param.data_ul.p_share_info->anchor_clk = 0;
        audio_transmitter_modify_share_info_by_block(open_param->stream_out_param.data_ul.p_share_info, open_param->stream_out_param.data_ul.max_payload_size);

        /* init the global status */
        g_usb_tx_avm_payload_size = open_param->stream_out_param.data_ul.max_payload_size;

        WIRED_AUDIO_LOG_MSGID_I("[USB_OUT] init: type %d, sub_id %d, is_with_ecnr %d, is_with_swb %d, codec_type %d, sample_rate = %d, channel_mode %d, format %d, 0x%08x, 0x%08x, %d, %d, %d, %d, %d, %d, %d, %d", 18,
                                    open_param->stream_out_param.data_ul.scenario_type,
                                    open_param->stream_out_param.data_ul.scenario_sub_id,
                                    open_param->stream_out_param.data_ul.scenario_param.wired_audio_param.is_with_ecnr,
                                    open_param->stream_out_param.data_ul.scenario_param.wired_audio_param.is_with_swb,
                                    open_param->stream_out_param.data_ul.scenario_param.wired_audio_param.codec_type,
                                    open_param->stream_out_param.data_ul.scenario_param.wired_audio_param.codec_param.pcm.sample_rate,
                                    open_param->stream_out_param.data_ul.scenario_param.wired_audio_param.codec_param.pcm.channel_mode,
                                    open_param->stream_out_param.data_ul.scenario_param.wired_audio_param.codec_param.pcm.format,
                                    (uint32_t)open_param->stream_out_param.data_ul.p_share_info,
                                    open_param->stream_out_param.data_ul.p_share_info->start_addr,
                                    open_param->stream_out_param.data_ul.p_share_info->read_offset,
                                    open_param->stream_out_param.data_ul.p_share_info->write_offset,
                                    open_param->stream_out_param.data_ul.p_share_info->bBufferIsFull,
                                    open_param->stream_out_param.data_ul.max_payload_size,
                                    open_param->stream_out_param.data_ul.p_share_info->sub_info.block_info.block_size,
                                    open_param->stream_out_param.data_ul.p_share_info->sub_info.block_info.block_num,
                                    open_param->stream_out_param.data_ul.p_share_info->length,
                                    g_usb_tx_cell_size);
    } else if ((config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) || (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1)) {
        uint32_t usb_cell_size;
        uint32_t dl_process_period;

        /* Parameter check */
        pcm = &(config->scenario_config.wired_audio_config.usb_in_config.codec_param.pcm);
        input_parameter_check(pcm);

        /* Calculate refer value for later init */
        if (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) {
            if (pcm->format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
                g_usb_rx_0_cell_size = (pcm->sample_rate * pcm->channel_mode * 3) / 1000;
            } else {
                g_usb_rx_0_cell_size = (pcm->sample_rate * pcm->channel_mode * 2) / 1000;
            }
            usb_cell_size = g_usb_rx_0_cell_size;
        } else {
            if (pcm->format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
                g_usb_rx_1_cell_size = (pcm->sample_rate * pcm->channel_mode * 3) / 1000;
            } else {
                g_usb_rx_1_cell_size = (pcm->sample_rate * pcm->channel_mode * 2) / 1000;
            }
            usb_cell_size = g_usb_rx_1_cell_size;
        }

#if WIRED_AUDIO_USB_DEBUG_LANTENCY
        audio_usb_rx_scenario_latency_debug_init(config->scenario_sub_id - AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0,
                                                    usb_cell_size, pcm->channel_mode, pcm->format);
#endif

#if defined(AIR_DCHS_MODE_ENABLE)
        if (pcm->sample_rate >= 96000) {
            dl_process_period = WIRED_AUDIO_USB_IN_PROCESS_PERIOD / 2;
        } else {
            dl_process_period = WIRED_AUDIO_USB_IN_PROCESS_PERIOD;
        }
#else
        dl_process_period = WIRED_AUDIO_USB_IN_PROCESS_PERIOD;
#endif

        /* Config the feature list */
#ifdef MTK_LINEIN_PEQ_ENABLE
        AudioFeatureList_wired_audio[0] = FUNC_PEQ_LINEIN;
#else
        AudioFeatureList_wired_audio[0] = CODEC_NULL;
#endif
#ifdef MTK_LINEIN_INS_ENABLE
        AudioFeatureList_wired_audio[1] = FUNC_INS;
#else
        AudioFeatureList_wired_audio[0] = CODEC_NULL;
#endif
        AudioFeatureList_wired_audio[2] = FUNC_END;

        /* Config the type of stream in and stream out */
        open_param->param.stream_in = STREAM_IN_AUDIO_TRANSMITTER;
        // if (wired_audio_get_mix_stream_start_number() == 1) {
        //     open_param->param.stream_out = STREAM_OUT_AFE;
        // } else {
            open_param->param.stream_out = STREAM_OUT_VIRTUAL;
        // }

        /* Config the structure of stream in */
        open_param->stream_in_param.data_dl.scenario_type = config->scenario_type;
        open_param->stream_in_param.data_dl.scenario_sub_id = config->scenario_sub_id;
        if (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) {
            open_param->stream_in_param.data_dl.p_share_info = hal_audio_query_audio_transmitter_share_info(WIRED_AUDIO_USB_IN_0_AVM_BUFFER_INDEX);
            if (g_usb_rx_0_avm_buf_size == 0) {
                g_usb_rx_0_avm_buf_size = open_param->stream_in_param.data_dl.p_share_info->length; /* Save the length of AVM buffer */
            } else {
                open_param->stream_in_param.data_dl.p_share_info->length = g_usb_rx_0_avm_buf_size; /* Reset the length of AVM buffer */
            }
        } else {
            open_param->stream_in_param.data_dl.p_share_info = hal_audio_query_audio_transmitter_share_info(WIRED_AUDIO_USB_IN_1_AVM_BUFFER_INDEX);
            if (g_usb_rx_1_avm_buf_size == 0) {
                g_usb_rx_1_avm_buf_size = open_param->stream_in_param.data_dl.p_share_info->length; /* Save the length of AVM buffer */
            } else {
                open_param->stream_in_param.data_dl.p_share_info->length = g_usb_rx_1_avm_buf_size; /* Reset the length of AVM buffer */
            }
        }
        memset((void *)(open_param->stream_in_param.data_dl.p_share_info->start_addr), 0, open_param->stream_in_param.data_dl.p_share_info->length);

        open_param->stream_in_param.data_dl.data_notification_frequency = 0;
        open_param->stream_in_param.data_dl.max_payload_size = usb_cell_size;
        open_param->stream_in_param.data_dl.scenario_param.wired_audio_param.codec_type = config->scenario_config.wired_audio_config.usb_in_config.codec_type;
        open_param->stream_in_param.data_dl.scenario_param.wired_audio_param.codec_param.pcm.sample_rate = pcm->sample_rate;
        open_param->stream_in_param.data_dl.scenario_param.wired_audio_param.codec_param.pcm.channel_mode = pcm->channel_mode;
        open_param->stream_in_param.data_dl.scenario_param.wired_audio_param.codec_param.pcm.format = pcm->format;
        open_param->stream_in_param.data_dl.scenario_param.wired_audio_param.codec_param.pcm.frame_interval = dl_process_period * 1000;
        audio_transmitter_reset_share_info_by_block(open_param->stream_in_param.data_dl.p_share_info,
                                                    open_param->stream_in_param.data_dl.p_share_info->start_addr,
                                                    open_param->stream_in_param.data_dl.p_share_info->length,
                                                    usb_cell_size);

        /* Config the init volume */
        uint32_t i, gain_default;
        if (g_usb_volume_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) {
            g_usb_volume_sub_id = 0;
            gain_default = audio_get_gain_out_in_dB(g_usb_in_0_volume_level, GAIN_DIGITAL, VOL_USB_AUDIO_SW_IN);
        } else if (g_usb_volume_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1) {
            g_usb_volume_sub_id = 0;
            gain_default = audio_get_gain_out_in_dB(g_usb_in_1_volume_level, GAIN_DIGITAL, VOL_USB_AUDIO_SW_IN);
        } else {
            gain_default = audio_get_gain_out_in_dB(0, GAIN_DIGITAL, VOL_USB_AUDIO_SW_IN);
        }
        for (i = 0; i < 8; i++) {
            open_param->stream_in_param.data_dl.scenario_param.wired_audio_param.gain[i] = gain_default;
        }

        /* Config the structure of stream out */
#ifdef AIR_DCHS_MODE_ENABLE
        hal_audio_get_stream_out_setting_config(AU_DSP_AUDIO, &open_param->stream_out_param);
        open_param->param.stream_out = STREAM_OUT_AFE;
        WIRED_AUDIO_LOG_MSGID_I("[USB_IN] stream_out %d, STREAM_OUT_AFE %d, STREAM_OUT_VIRTUAL %d",3,open_param->param.stream_out,STREAM_OUT_AFE,STREAM_OUT_VIRTUAL);
        open_param->stream_out_param.afe.memory = HAL_AUDIO_MEM3;
        open_param->stream_out_param.afe.format = HAL_AUDIO_PCM_FORMAT_S24_LE; /* Always fix to 32bit during stream process */
#ifdef AIR_DCHS_MODE_ENABLE
        open_param->stream_out_param.afe.stream_out_sampling_rate = pcm->sample_rate;
        open_param->stream_out_param.afe.sampling_rate = pcm->sample_rate;
#else
        open_param->stream_out_param.afe.stream_out_sampling_rate = WIRED_AUDIO_MIXING_MODE_USB_DL_OUT_SAMPLE_RATE;
        open_param->stream_out_param.afe.sampling_rate = WIRED_AUDIO_MIXING_MODE_USB_DL_OUT_SAMPLE_RATE;
#endif
        open_param->stream_out_param.afe.frame_number = 3; /* DSP always use default frame number */
        open_param->stream_out_param.afe.irq_period = dl_process_period;
        open_param->stream_out_param.afe.frame_size = (dl_process_period * open_param->stream_out_param.afe.sampling_rate) / 1000;
        open_param->stream_out_param.afe.hw_gain = true;
#ifdef ENABLE_HWSRC_CLKSKEW
        open_param->stream_out_param.afe.clkskew_mode = CLK_SKEW_V1;
#endif
        if (open_param->stream_out_param.afe.audio_device == HAL_AUDIO_DEVICE_I2S_MASTER) {
            open_param->stream_out_param.afe.misc_parms      = I2S_CLK_SOURCE_DCXO;
        } else {
            open_param->stream_out_param.afe.misc_parms      = DOWNLINK_PERFORMANCE_NORMAL;
        }
#else /*AIR_DCHS_MODE_ENABLE*/
        open_param->param.stream_out = STREAM_OUT_VIRTUAL;
#endif /*AIR_DCHS_MODE_ENABLE*/

        /* init the global status */

        WIRED_AUDIO_LOG_MSGID_I("[USB_IN] init: type %d, sub_id %d, codec_type %d, sample_rate = %d, channel %d, format %d, 0x%08x, 0x%08x, %d, %d, %d, %d, %d, %d, %d, %d, %d", 17,
                                    open_param->stream_in_param.data_dl.scenario_type,
                                    open_param->stream_in_param.data_dl.scenario_sub_id,
                                    open_param->stream_in_param.data_dl.scenario_param.wired_audio_param.codec_type,
                                    open_param->stream_in_param.data_dl.scenario_param.wired_audio_param.codec_param.pcm.sample_rate,
                                    open_param->stream_in_param.data_dl.scenario_param.wired_audio_param.codec_param.pcm.channel_mode,
                                    open_param->stream_in_param.data_dl.scenario_param.wired_audio_param.codec_param.pcm.format,
                                    (uint32_t)open_param->stream_in_param.data_dl.p_share_info,
                                    open_param->stream_in_param.data_dl.p_share_info->start_addr,
                                    open_param->stream_in_param.data_dl.p_share_info->read_offset,
                                    open_param->stream_in_param.data_dl.p_share_info->write_offset,
                                    open_param->stream_in_param.data_dl.p_share_info->bBufferIsFull,
                                    open_param->stream_in_param.data_dl.max_payload_size,
                                    open_param->stream_in_param.data_dl.p_share_info->sub_info.block_info.block_size,
                                    open_param->stream_in_param.data_dl.p_share_info->sub_info.block_info.block_num,
                                    open_param->stream_in_param.data_dl.p_share_info->length,
                                    g_usb_rx_0_cell_size,
                                    g_usb_rx_1_cell_size);
    }
#endif
    else if ((config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT) || (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER)) {
        pcm = &(config->scenario_config.wired_audio_config.line_out_config.codec_param.pcm);

        /* Temp workaround for parameters */
        if (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT) {
            pcm->sample_rate = 16000;
            pcm->channel_mode = 2;
            pcm->format = HAL_AUDIO_PCM_FORMAT_S16_LE;
        } else if (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER) {
            #if defined(AIR_FIXED_DL_SAMPLING_RATE_TO_96KHZ)
            pcm->sample_rate = 96000;
            #else
            pcm->sample_rate = 48000;
            #endif
            pcm->channel_mode = 2;
            pcm->format = HAL_AUDIO_PCM_FORMAT_S16_LE;
        }

        /* Parameter check */
        input_parameter_check(pcm);

        /* Config the feature list */
        if (config->scenario_config.wired_audio_config.line_out_config.is_with_swb == true) {
            AudioFeatureList_wired_audio[0] = FUNC_RX_NR_SWB;
        } else {
            AudioFeatureList_wired_audio[0] = FUNC_RX_NR;
        }
        AudioFeatureList_wired_audio[1] = FUNC_END;
#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
        voice_mic_type_t mic_cur_type = hal_audio_query_voice_mic_type();
        WIRED_AUDIO_LOG_MSGID_I("[LINE_OUT]Wired line out mic_cur_type: 0x%x", 1, mic_cur_type);
        if (mic_cur_type == VOICE_MIC_TYPE_FIXED) {
            if (config->scenario_config.wired_audio_config.line_out_config.is_with_swb == true) {
                AudioFeatureList_wired_audio[0] = FUNC_RX_NR_SWB;
            } else {
                AudioFeatureList_wired_audio[0] = FUNC_RX_NR;
            }
        } else if (mic_cur_type == VOICE_MIC_TYPE_DETACHABLE) {
            if (config->scenario_config.wired_audio_config.line_out_config.is_with_swb == true) {
                AudioFeatureList_wired_audio[0] = FUNC_SWB_BOOM_MIC;
            } else {
                AudioFeatureList_wired_audio[0] = FUNC_WB_BOOM_MIC;
            }
        } else {
            WIRED_AUDIO_LOG_MSGID_E("[LINE_OUT]Wired line out no this mic type - (%d)\r\n", 1, mic_cur_type);
            configASSERT(0);
        }
#endif

        /* Config the type of stream in and stream out */
        open_param->param.stream_in = STREAM_IN_AFE;
        open_param->param.stream_out = STREAM_OUT_AFE;

        /* Config the structure of stream in */
        hal_audio_get_stream_in_setting_config(AU_DSP_VOICE, &open_param->stream_in_param);
#if AIR_BTA_IC_STEREO_HIGH_G3
        /* AB157x has only 3 UL channels, UL1(2ch) for input of line in, UL2(1ch) for input of line out */
        open_param->stream_in_param.afe.audio_device = HAL_AUDIO_DEVICE_MAIN_MIC_L;
        open_param->stream_in_param.afe.audio_device1 = HAL_AUDIO_DEVICE_NONE;
        open_param->stream_in_param.afe.audio_interface = HAL_AUDIO_INTERFACE_2;
        open_param->stream_in_param.afe.audio_interface1 = HAL_AUDIO_INTERFACE_NONE;
#endif
        open_param->stream_in_param.afe.stream_channel  = HAL_AUDIO_DIRECT;
        open_param->stream_in_param.afe.memory          = HAL_AUDIO_MEM1 | HAL_AUDIO_MEM3;
        open_param->stream_in_param.afe.format          = HAL_AUDIO_PCM_FORMAT_S16_LE;

        if (config->scenario_config.wired_audio_config.line_out_config.is_with_swb == true) {
            open_param->stream_in_param.afe.stream_process_sampling_rate   = 32000;
        } else {
            open_param->stream_in_param.afe.stream_process_sampling_rate   = 16000;
        }
#if defined(AIR_UL_FIX_SAMPLING_RATE_32K)
        open_param->stream_in_param.afe.sampling_rate = 32000;
#elif defined(AIR_UL_FIX_SAMPLING_RATE_48K)
        open_param->stream_in_param.afe.sampling_rate = 48000;
#else
        if (config->scenario_config.wired_audio_config.line_out_config.is_with_swb == true) {
            open_param->stream_in_param.afe.sampling_rate = 32000;
        } else {
            open_param->stream_in_param.afe.sampling_rate = 16000;
        }
#endif

        open_param->stream_in_param.afe.irq_period      = WIRED_AUDIO_LINE_OUT_PROCESS_PERIOD;
        open_param->stream_in_param.afe.frame_size      = (WIRED_AUDIO_LINE_OUT_PROCESS_PERIOD * open_param->stream_in_param.afe.sampling_rate) / 1000;
        open_param->stream_in_param.afe.frame_number    = 3;
        open_param->stream_in_param.afe.hw_gain         = false;

        /* Config the structure of stream out */
        ///TODO: read the audio_device/interface from line out nvkey.
        line_out_stream_out_afe_config(config, config->scenario_sub_id, open_param);

        /* Config the external LINE out device */
        line_out_external_device_config(config, open_param);

        WIRED_AUDIO_LOG_MSGID_I("[LINE_OUT] init: type %d, sub_id %d, is_with_swb %d, codec_type %d, sample_rate = %d, channel %d, format %d", 7,
                                    config->scenario_type,
                                    config->scenario_sub_id,
                                    config->scenario_config.wired_audio_config.line_out_config.is_with_swb,
                                    config->scenario_config.wired_audio_config.line_out_config.codec_type,
                                    config->scenario_config.wired_audio_config.line_out_config.codec_param.pcm.sample_rate,
                                    config->scenario_config.wired_audio_config.line_out_config.codec_param.pcm.channel_mode,
                                    config->scenario_config.wired_audio_config.line_out_config.codec_param.pcm.format);
    } else if ((config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN) || (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER) || (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE)) {
        pcm = &(config->scenario_config.wired_audio_config.line_in_config.codec_param.pcm);

        /* Temp workaround for parameters */
        if ((config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER) ||
            (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE)) {
#if defined(AIR_FIXED_DL_SAMPLING_RATE_TO_96KHZ)
            pcm->sample_rate = 96000;
#else
            pcm->sample_rate = 48000;
#endif
            pcm->channel_mode = 2;
            pcm->format = HAL_AUDIO_PCM_FORMAT_S32_LE;
        }

        /* Parameter check */
        input_parameter_check(pcm);

        #if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
        if(config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER)
        {
            audio_nvdm_HW_config.audio_scenario.Audio_Linein_Input_Path = 0x2; // I2S_Slave_In
            audio_nvdm_HW_config.audio_scenario.Audio_Linein_Input_I2S_Interface = 0x02; // I2S2
        }
        if(config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE)
        {
            audio_nvdm_HW_config.audio_scenario.Audio_Linein_Input_Path = 0x2; // I2S_Slave_In
            audio_nvdm_HW_config.audio_scenario.Audio_Linein_Input_I2S_Interface = 0x00; // I2S0
            audio_nvdm_HW_config.audio_scenario.Audio_Linein_Output_Path = 0x2; // 0x2: Analog_SPK_Out_DUAL
        }
        if(config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN)
        {
            audio_nvdm_HW_config.audio_scenario.Audio_Linein_Input_Path = 0x0; // Line-In
            audio_nvdm_HW_config.audio_scenario.Audio_Linein_Output_Path = 0x4; // I2S_Slave_Out
            audio_nvdm_HW_config.audio_scenario.Audio_Linein_Output_I2S_Interface = 0x00; // I2S1
        }

        TRANSMITTER_LOG_I("[DEBUG] audio_nvdm_HW_config.audio_scenario.Audio_Linein_Input_Path=0x%X", 1, audio_nvdm_HW_config.audio_scenario.Audio_Linein_Input_Path);
        TRANSMITTER_LOG_I("[DEBUG] audio_nvdm_HW_config.audio_scenario.Audio_Linein_Input_I2S_Interface=0x%X", 1, audio_nvdm_HW_config.audio_scenario.Audio_Linein_Input_I2S_Interface);
        TRANSMITTER_LOG_I("[DEBUG] audio_nvdm_HW_config.audio_scenario.Audio_Linein_Output_Path=0x%X", 1, audio_nvdm_HW_config.audio_scenario.Audio_Linein_Output_Path);
        TRANSMITTER_LOG_I("[DEBUG] audio_nvdm_HW_config.audio_scenario.Audio_Linein_Output_I2S_Interface=0x%X", 1, audio_nvdm_HW_config.audio_scenario.Audio_Linein_Output_I2S_Interface);
        #endif

        /* Config the feature list */
#ifdef MTK_LINEIN_PEQ_ENABLE
        AudioFeatureList_wired_audio[0] = FUNC_PEQ_LINEIN;
#else
        AudioFeatureList_wired_audio[0] = CODEC_NULL;
#endif

#ifdef MTK_LINEIN_INS_ENABLE
        AudioFeatureList_wired_audio[1] = FUNC_INS;
#else
        AudioFeatureList_wired_audio[0] = CODEC_NULL;
#endif
        AudioFeatureList_wired_audio[2] = FUNC_END;

        /* Config the type of stream in and stream out */
        open_param->param.stream_in = STREAM_IN_AFE;
        open_param->param.stream_out = STREAM_OUT_AFE;

        /* Config the structure of stream in */
        hal_audio_get_stream_in_setting_config(AU_DSP_LINEIN, &(open_param->stream_in_param));
        open_param->stream_in_param.afe.stream_channel = HAL_AUDIO_DIRECT;
        if (open_param->stream_in_param.afe.audio_device == HAL_AUDIO_DEVICE_I2S_SLAVE) {
#if defined(AIR_LINE_OUT_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
            open_param->stream_in_param.afe.memory = HAL_AUDIO_MEM_SUB;
#else
            open_param->stream_in_param.afe.memory = HAL_AUDIO_MEM1;
#endif
#if defined(AIR_BTA_IC_STEREO_HIGH_G3) && defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
        } else if (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN) {
            open_param->stream_in_param.afe.memory = HAL_AUDIO_MEM1;
#endif
        } else {
            open_param->stream_in_param.afe.memory = HAL_AUDIO_MEM_SUB;
        }
        pcm = &(config->scenario_config.wired_audio_config.line_in_config.codec_param.pcm);
        open_param->stream_in_param.afe.format = pcm->format;
        open_param->stream_in_param.afe.sampling_rate = pcm->sample_rate;
#if defined(AIR_DCHS_MODE_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
        if (pcm->sample_rate >= 96000) {
            open_param->stream_in_param.afe.irq_period = WIRED_AUDIO_LINE_IN_HIRES_PROCESS_TIME;
            open_param->stream_in_param.afe.frame_size = (WIRED_AUDIO_LINE_IN_HIRES_PROCESS_TIME * pcm->sample_rate) / 1000;
        } else {
            open_param->stream_in_param.afe.irq_period = WIRED_AUDIO_LINE_IN_PROCESS_TIME;
            open_param->stream_in_param.afe.frame_size = (WIRED_AUDIO_LINE_IN_PROCESS_TIME * pcm->sample_rate) / 1000;
        }
#else
        open_param->stream_in_param.afe.irq_period = 0;
        open_param->stream_in_param.afe.frame_size = (((WIRED_AUDIO_LINE_IN_PROCESS_SAMPLES * pcm->sample_rate) / 48000) / 8) * 8; /* about 2.67ms */
#endif
        open_param->stream_in_param.afe.frame_number = 3;
        open_param->stream_in_param.afe.hw_gain = false;
        if (open_param->stream_in_param.afe.audio_device == HAL_AUDIO_DEVICE_I2S_MASTER) {
            open_param->stream_in_param.afe.misc_parms      = I2S_CLK_SOURCE_DCXO;
            if (pcm->sample_rate > HAL_AUDIO_SAMPLING_RATE_48KHZ) {
                open_param->stream_in_param.afe.misc_parms  = I2S_CLK_SOURCE_APLL;
            }
        } else {
            if (pcm->channel_mode == 2) {
                open_param->stream_in_param.afe.misc_parms      = MICBIAS_SOURCE_ALL | MICBIAS3V_OUTVOLTAGE_2p4v;
            } else {
                open_param->stream_in_param.afe.misc_parms      = MICBIAS_SOURCE_ALL | MICBIAS3V_OUTVOLTAGE_1p85v;
            }
        }
#ifdef AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE
        if(config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE) {
            #if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) && defined(AIR_FIXED_DL_SAMPLING_RATE_TO_96KHZ)
            open_param->stream_in_param.afe.sampling_rate   = 96000;
            #else
            open_param->stream_in_param.afe.sampling_rate   = 48000;
            #endif
            open_param->stream_in_param.afe.stream_out_sampling_rate   = 48000;
            open_param->stream_in_param.afe.format          = HAL_AUDIO_PCM_FORMAT_S16_LE;
        }
#endif

#ifdef AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE
        if(config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE) {
            #if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) && defined(AIR_FIXED_DL_SAMPLING_RATE_TO_96KHZ)
            open_param->stream_in_param.afe.sampling_rate   = 96000;
            #else
            open_param->stream_in_param.afe.sampling_rate   = 48000;
            #endif
            open_param->stream_in_param.afe.stream_out_sampling_rate   = 48000;
            open_param->stream_in_param.afe.format          = HAL_AUDIO_PCM_FORMAT_S16_LE;
        }
#endif

        /* Config the structure of stream out */
        hal_audio_get_stream_out_setting_config(AU_DSP_LINEIN, &(open_param->stream_out_param));
        open_param->stream_out_param.afe.stream_channel = HAL_AUDIO_DIRECT;
#if defined(AIR_BTA_IC_STEREO_HIGH_G3) && defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
        if (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN) {
            open_param->stream_out_param.afe.memory = HAL_AUDIO_MEM1;
        } else {
            open_param->stream_out_param.afe.memory = HAL_AUDIO_MEM3;
        }
#else
        open_param->stream_out_param.afe.memory = HAL_AUDIO_MEM3;
#endif
        open_param->stream_out_param.afe.format = pcm->format;
        if (aud_fixrate_get_downlink_rate(open_param->audio_scenario_type) == FIXRATE_NONE) {
            open_param->stream_out_param.afe.sampling_rate = pcm->sample_rate;
        } else {
            open_param->stream_out_param.afe.sampling_rate = aud_fixrate_get_downlink_rate(open_param->audio_scenario_type);
        }

#ifdef AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE
        if(config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE) {
            //Fix AUX-out at 48KHz
            open_param->stream_out_param.afe.sampling_rate = HAL_AUDIO_FIXED_AFE_48K_SAMPLE_RATE;
        }

#endif
        open_param->stream_out_param.afe.stream_out_sampling_rate = open_param->stream_out_param.afe.sampling_rate;
#if defined(AIR_DCHS_MODE_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
        if (pcm->sample_rate >= 96000) {
            open_param->stream_out_param.afe.irq_period = WIRED_AUDIO_LINE_IN_HIRES_PROCESS_TIME;
            open_param->stream_out_param.afe.frame_size = WIRED_AUDIO_LINE_IN_HIRES_PROCESS_TIME * (open_param->stream_out_param.afe.sampling_rate / 1000);
        } else {
            open_param->stream_out_param.afe.irq_period = WIRED_AUDIO_LINE_IN_PROCESS_TIME;
            open_param->stream_out_param.afe.frame_size = WIRED_AUDIO_LINE_IN_PROCESS_TIME * (open_param->stream_out_param.afe.sampling_rate / 1000);
        }
#else
        open_param->stream_out_param.afe.irq_period = 0;
        open_param->stream_out_param.afe.frame_size = (((WIRED_AUDIO_LINE_IN_PROCESS_SAMPLES * open_param->stream_out_param.afe.sampling_rate) / 48000) / 8) * 8; /* about 2.67ms */
#endif
        open_param->stream_out_param.afe.frame_number = 3;
        open_param->stream_out_param.afe.hw_gain = true;
#ifdef AIR_AUDIO_MIXER_GAIN_ENABLE
        if(config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER)
            open_param->stream_out_param.afe.hw_gain = false;
#endif
#ifdef AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE
        if(config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE)
            open_param->stream_out_param.afe.hw_gain = false;
#endif

#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
        if((config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER)||(config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE))
        {
            // align setting to maindev
            open_param->stream_in_param.afe.irq_period = 0;
            open_param->stream_in_param.afe.frame_size = 128;
            open_param->stream_out_param.afe.irq_period = 0;
            open_param->stream_out_param.afe.frame_size = 128;

            // avoid sub-source offset overflow
            open_param->stream_in_param.afe.frame_number = 3;
        }
#endif

        open_param->stream_out_param.afe.adc_type = HAL_AUDIO_ANALOG_TYPE_SINGLE;
#ifdef ENABLE_HWSRC_CLKSKEW
        open_param->stream_out_param.afe.clkskew_mode = CLK_SKEW_V1;
#endif
        if (open_param->stream_out_param.afe.audio_device == HAL_AUDIO_DEVICE_I2S_MASTER) {
            open_param->stream_out_param.afe.misc_parms      = I2S_CLK_SOURCE_DCXO;
            if (pcm->sample_rate > HAL_AUDIO_SAMPLING_RATE_48KHZ) {
                open_param->stream_out_param.afe.misc_parms  = I2S_CLK_SOURCE_APLL;
            }
        } else {
            open_param->stream_out_param.afe.misc_parms      = DOWNLINK_PERFORMANCE_NORMAL;
        }

        /* Config the external LINE out device */
        line_in_external_device_config(config, open_param);
#if defined(AIR_HWSRC_IN_STREAM_ENABLE)
        open_param->stream_out_param.afe.hwsrc_type      = HAL_AUDIO_HWSRC_IN_STREAM;
#endif
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
        //hal_dvfs_lock_control(HAL_DVFS_OPP_HIGH, HAL_DVFS_LOCK);
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_1DAC_1I2S_ENABLE
        open_param->stream_out_param.afe.audio_device     = HAL_AUDIO_DEVICE_DAC_DUAL;
        open_param->stream_out_param.afe.audio_interface  = HAL_AUDIO_INTERFACE_1;
        open_param->stream_out_param.afe.audio_device1    = HAL_AUDIO_DEVICE_I2S_MASTER;
        open_param->stream_out_param.afe.audio_interface1 = HAL_AUDIO_INTERFACE_1;
        open_param->stream_out_param.afe.audio_device2    = HAL_AUDIO_DEVICE_NONE;
        open_param->stream_out_param.afe.audio_interface2 = HAL_AUDIO_DEVICE_NONE;
        open_param->stream_out_param.afe.memory           |= HAL_AUDIO_MEM1;
        open_param->stream_out_param.afe.is_low_jitter[0]    = true;
#elif AIR_AUDIO_MULTIPLE_STREAM_OUT_3I2S_ENABLE
        //open_param->stream_in_param.afe.frame_size = 512;
        //open_param->stream_out_param.afe.frame_size = 512;
        open_param->stream_out_param.afe.audio_device     = HAL_AUDIO_DEVICE_I2S_MASTER;
        open_param->stream_out_param.afe.audio_interface  = HAL_AUDIO_INTERFACE_1;
        open_param->stream_out_param.afe.audio_device1    = HAL_AUDIO_DEVICE_I2S_MASTER;
        open_param->stream_out_param.afe.audio_interface1 = HAL_AUDIO_INTERFACE_2;
        open_param->stream_out_param.afe.audio_device2    = HAL_AUDIO_DEVICE_I2S_MASTER;
        //open_param->stream_out_param.afe.audio_device2    = HAL_AUDIO_DEVICE_DAC_DUAL;
        open_param->stream_out_param.afe.audio_interface2 = HAL_AUDIO_INTERFACE_3;
        open_param->stream_out_param.afe.memory           |= HAL_AUDIO_MEM1|HAL_AUDIO_MEM4;
#endif
        WIRED_AUDIO_LOG_MSGID_I("out_device0(0x%x), channel(%d), interface0(0x%x)", 3, open_param->stream_out_param.afe.audio_device, open_param->stream_out_param.afe.stream_channel, open_param->stream_out_param.afe.audio_interface);
        WIRED_AUDIO_LOG_MSGID_I("out_device1(0x%x), channel(%d), interface1(0x%x)", 3, open_param->stream_out_param.afe.audio_device1, open_param->stream_out_param.afe.stream_channel, open_param->stream_out_param.afe.audio_interface1);
        WIRED_AUDIO_LOG_MSGID_I("out_device2(0x%x), channel(%d), interface2(0x%x)", 3, open_param->stream_out_param.afe.audio_device2, open_param->stream_out_param.afe.stream_channel, open_param->stream_out_param.afe.audio_interface2);
        WIRED_AUDIO_LOG_MSGID_I("memory(0x%x)", 1, open_param->stream_out_param.afe.memory);
#endif
        WIRED_AUDIO_LOG_MSGID_I("[LINE_IN] init: type %d, sub_id %d, codec_type %d, sample_rate = %d, channel %d, format %d, afe sample_rate %d, afe stream_sample_rate %d, afe format %d", 9,
                                    config->scenario_type,
                                    config->scenario_sub_id,
                                    config->scenario_config.wired_audio_config.line_in_config.codec_type,
                                    config->scenario_config.wired_audio_config.line_in_config.codec_param.pcm.sample_rate,
                                    config->scenario_config.wired_audio_config.line_in_config.codec_param.pcm.channel_mode,
                                    config->scenario_config.wired_audio_config.line_in_config.codec_param.pcm.format,
                                    open_param->stream_out_param.afe.sampling_rate,
                                    open_param->stream_out_param.afe.stream_out_sampling_rate,
                                    open_param->stream_out_param.afe.format);
    } else if (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_OUT_IEM) {
        open_param->param.stream_in = STREAM_IN_VIRTUAL;
        open_param->param.stream_out = STREAM_OUT_VIRTUAL;

        open_param->stream_in_param.virtual_param.virtual_mem_size = (WIRED_AUDIO_USB_IN_PROCESS_PERIOD * WIRED_AUDIO_MIXING_MODE_USB_DL_OUT_SAMPLE_RATE) / 1000 * 4;
    }

    if (AudioFeatureList_wired_audio[0] != FUNC_END) {
        audio_nvdm_reset_sysram();
        status = audio_nvdm_set_feature(2, AudioFeatureList_wired_audio);
        if (status != NVDM_STATUS_NAT_OK) {
            WIRED_AUDIO_LOG_MSGID_E("failed to set parameters to share memory - err(%d)\r\n", 1, status);
            AUDIO_ASSERT(0);
        }
    }
}

void audio_transmitter_wired_audio_start_playback(audio_transmitter_config_t *config, mcu2dsp_start_param_t *start_param)
{
    if (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT) {
        start_param->param.stream_in = STREAM_IN_AFE;
        start_param->param.stream_out = STREAM_OUT_AUDIO_TRANSMITTER;
    } else if (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) {
        start_param->param.stream_in = STREAM_IN_AUDIO_TRANSMITTER;
        start_param->param.stream_out = STREAM_OUT_AFE;
    } else if (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1) {
        start_param->param.stream_in = STREAM_IN_AUDIO_TRANSMITTER;
        start_param->param.stream_out = STREAM_OUT_AFE;
    } else if (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT) {
        start_param->param.stream_in = STREAM_IN_AFE;
        start_param->param.stream_out = STREAM_OUT_AFE;
    } else if (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER) {
        start_param->param.stream_in = STREAM_IN_AFE;
        start_param->param.stream_out = STREAM_OUT_AFE;
    } else if (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN) {
        start_param->param.stream_in = STREAM_IN_AFE;
        start_param->param.stream_out = STREAM_OUT_AFE;
    } else if ((config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER) || (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE)) {
        start_param->param.stream_in = STREAM_IN_AFE;
        start_param->param.stream_out = STREAM_OUT_AFE;
    }
    #ifdef AIR_DCHS_MODE_ENABLE
    audio_scenario_type_t scenario_type = AUDIO_SCENARIO_TYPE_COMMON;
    if (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN){
        scenario_type = AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN;
    }else if(config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0){
        scenario_type = AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0;
    }else if(config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT){
        scenario_type = AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT;
    }else if(config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT){
        scenario_type = AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_OUT;
    }
    dchs_cosys_ctrl_cmd_relay(AUDIO_UART_COSYS_UL_START, scenario_type, NULL, start_param);
    dchs_cosys_ctrl_cmd_relay(AUDIO_UART_COSYS_DL_START, scenario_type, NULL, start_param);
    #endif
}

typedef struct {
    int32_t gain[8];
} vol_gain_t;


#define GAIN_COMPENSATION_STEP 10
const static int16_t gain_compensation_table[GAIN_COMPENSATION_STEP + 1] = {
    /*
    Ratio |    db    | Compensation
    0%    |  -60db   | 0xE890
    10%   |  -20db   | 0xF830
    20%   | -13.98db | 0xFA8B
    30%   | -10.46db | 0xFBEB
    40%   |  -7.96db | 0xFCE5
    50%   |  -6.02db | 0xFDA6
    60%   |  -4.44db | 0xFE45
    70%   |  -3.1db  | 0xFECB
    80%   |  -1.94db | 0xFF3F
    90%   |  -0.92db | 0xFFA5
    100%  |     0db  | 0
    */
    0xE890,
    0xF830,
    0xFA8B,
    0xFBEB,
    0xFCE5,
    0xFDA6,
    0xFE45,
    0xFECB,
    0xFF3F,
    0xFFA5,
    0x0
};

extern void bt_sink_srv_am_set_volume(bt_sink_srv_am_stream_type_t in_out, bt_sink_srv_audio_setting_vol_info_t *vol_info);
#ifdef AIR_MUTE_MIC_DETECTION_ENABLE
bool g_usb_out_vad_enable = false;
#endif
audio_transmitter_status_t wired_audio_set_runtime_config_playback(audio_transmitter_config_t *config, audio_transmitter_runtime_config_type_t runtime_config_type, audio_transmitter_runtime_config_t *runtime_config, mcu2dsp_audio_transmitter_runtime_config_param_t *runtime_config_param)
{
    vol_gain_t vol_gain = {0};
    uint8_t volume_level, volume_ratio;
    uint32_t i, operation, digital_gain, analog_gain;
    bt_sink_srv_audio_setting_vol_info_t vol_info;
    bt_sink_srv_audio_setting_vol_t vol;

    operation = runtime_config_type;
    WIRED_AUDIO_LOG_MSGID_I("runtime_config: scenario_sub_id %d, operation %d", 2, config->scenario_sub_id, operation);

    switch (config->scenario_sub_id) {
        case AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0:
        case AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1:
            if ((operation < WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_MUSIC_L) || (operation > WIRED_AUDIO_CONFIG_OP_VOL_DB_MUSIC)) {
                WIRED_AUDIO_LOG_MSGID_W("scenario_sub_id %d, operation %d not support", 2, config->scenario_sub_id, operation);
                return AUDIO_TRANSMITTER_STATUS_FAIL;
            }

            if (operation == WIRED_AUDIO_CONFIG_OP_VOL_DB_MUSIC) {
                for (i = 0; i < 8; i++) {
                    vol_gain.gain[i] = runtime_config->wired_audio_runtime_config.vol_db.vol_db[i] / 100; /* Change to 0.01dB */
                    vol_gain.gain[i] += gain_compensation_table[runtime_config->wired_audio_runtime_config.vol_db.vol_ratio / GAIN_COMPENSATION_STEP];
                    WIRED_AUDIO_LOG_MSGID_I("scenario_sub_id %d, operation %d, ch %d, vol_db %d, vol_ratio %d, config vol_gain to %d * 0.01dB", 6,
                                                config->scenario_sub_id, operation, i,
                                                runtime_config->wired_audio_runtime_config.vol_db.vol_db[i],
                                                runtime_config->wired_audio_runtime_config.vol_db.vol_ratio,
                                                vol_gain.gain[i]);
                }
            } else {
                volume_ratio = runtime_config->wired_audio_runtime_config.vol_level.vol_ratio;
                if (volume_ratio > 100) {
                    WIRED_AUDIO_LOG_MSGID_W("scenario_sub_id %d, operation %d, Volume ratio should between 0 and 100, volume_ratio = %d", 3,
                                                config->scenario_sub_id, operation, volume_ratio);
                    volume_ratio = 100;
                }

                for (i = 0; i < 8; i++) {
                    if ((operation != WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_MUSIC_DUL) &&
                        (i != (operation - WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_MUSIC_L))) {
                        continue;
                    }
                    volume_level = runtime_config->wired_audio_runtime_config.vol_level.vol_level[i];
                    if (volume_level > bt_sink_srv_ami_get_usb_music_sw_max_volume_level()) {
                        volume_level = bt_sink_srv_ami_get_usb_music_sw_max_volume_level();
                        WIRED_AUDIO_LOG_MSGID_W("scenario_sub_id %d, operation %d, set channel %d, volume %d level more than max level %d\r\n", 5,
                                                    config->scenario_sub_id, operation,
                                                    i, volume_level, bt_sink_srv_ami_get_usb_music_sw_max_volume_level());
                    }
                    if (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) {
                        g_usb_in_0_volume_level = volume_level;
                    } else if (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1) {
                        g_usb_in_1_volume_level = volume_level;
                    }
                    vol_gain.gain[i] = audio_get_gain_out_in_dB(volume_level, GAIN_DIGITAL, VOL_USB_AUDIO_SW_IN);
                    vol_gain.gain[i] += gain_compensation_table[volume_ratio / GAIN_COMPENSATION_STEP];
                    WIRED_AUDIO_LOG_MSGID_I("scenario_sub_id %d, operation %d, config vol_gain to %d dB", 3,
                                                config->scenario_sub_id, operation, vol_gain.gain[i]);
                }
            }

            runtime_config_param->config_operation = operation;
            memcpy(runtime_config_param->config_param, &vol_gain, sizeof(vol_gain_t));

            break;

        case AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN:
        case AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER:
        case AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE:
            if ((operation != WIRED_AUDIO_CONFIG_OP_VOL_LEVEL_LINEIN) &&
                (operation != WIRED_AUDIO_CONFIG_OP_DEVICE_LINEIN)) {
                WIRED_AUDIO_LOG_MSGID_W("scenario_sub_id %d, operation %d not support", 2, config->scenario_sub_id, operation);
                return AUDIO_TRANSMITTER_STATUS_FAIL;
            }

            if (operation == WIRED_AUDIO_CONFIG_OP_DEVICE_LINEIN) {
                audio_nvdm_HW_config.audio_scenario.Audio_Linein_Input_Path = runtime_config->wired_audio_runtime_config.line_in_config.line_in_path;
                audio_nvdm_HW_config.audio_scenario.Audio_Linein_Input_I2S_Interface = runtime_config->wired_audio_runtime_config.line_in_config.line_in_i2s_interface;
                return AUDIO_TRANSMITTER_STATUS_FAIL; //not send runtime config msg to dsp!!
            }

            volume_level = runtime_config->wired_audio_runtime_config.vol_level.vol_level[0];
            WIRED_AUDIO_LOG_MSGID_I("scenario_sub_id %d, operation %d, set volume %d level \r\n", 3,
                                    config->scenario_sub_id, operation, volume_level);
            if (volume_level > bt_sink_srv_ami_get_lineIN_max_volume_level()) {
                volume_level = bt_sink_srv_ami_get_lineIN_max_volume_level();
                WIRED_AUDIO_LOG_MSGID_W("scenario_sub_id %d, operation %d, set volume %d level more than max level %d\r\n", 4,
                                            config->scenario_sub_id, operation, volume_level, bt_sink_srv_ami_get_lineIN_max_volume_level());
            }

#ifdef MTK_DEVELOPMENT_BOARD_HDK
#define AUDIO_SINK_SRV_LINE_IN_INPUT_DEVICE HAL_AUDIO_DEVICE_MAIN_MIC
#else
#define AUDIO_SINK_SRV_LINE_IN_INPUT_DEVICE HAL_AUDIO_DEVICE_SINGLE_DIGITAL_MIC
#endif
#if defined(__AFE_HS_DC_CALIBRATION__)
#define AUDIO_SINK_SRV_LINE_IN_OUTPUT_DEVICE HAL_AUDIO_DEVICE_HANDSET
#else
#define AUDIO_SINK_SRV_LINE_IN_OUTPUT_DEVICE HAL_AUDIO_DEVICE_HEADSET
#endif
            vol_info.type = VOL_LINE_IN;
            vol_info.vol_info.lineIN_vol_info.dev_in  = AUDIO_SINK_SRV_LINE_IN_INPUT_DEVICE;
            vol_info.vol_info.lineIN_vol_info.dev_out = AUDIO_SINK_SRV_LINE_IN_OUTPUT_DEVICE;
            vol_info.vol_info.lineIN_vol_info.lev_in  = 1;
            vol_info.vol_info.lineIN_vol_info.lev_out = volume_level;
            bt_sink_srv_audio_setting_get_vol(&vol_info, &vol);

            digital_gain = vol.vol.lineIN_vol.vol_out.digital;
            analog_gain = vol.vol.lineIN_vol.vol_out.analog_L;

#if defined(AIR_BTA_IC_STEREO_HIGH_G3) && defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE)
            if (config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN) {
                hal_audio_set_stream_out_volume(HAL_AUDIO_STREAM_OUT1, digital_gain, analog_gain);
            } else {
                hal_audio_set_stream_out_volume(HAL_AUDIO_STREAM_OUT3, digital_gain, analog_gain);
            }
#else
            hal_audio_set_stream_out_volume(HAL_AUDIO_STREAM_OUT3, digital_gain, analog_gain);
#endif

            WIRED_AUDIO_LOG_MSGID_I("scenario_sub_id %d, operation %d, volume_level %d, volume digital gain %x, analog gain %x \r\n", 5,
                                    config->scenario_sub_id, operation, volume_level, digital_gain, analog_gain);

            /* As we always call hal API to set the volume, so don't send runtime config msg to dsp!! */
            return AUDIO_TRANSMITTER_STATUS_FAIL;

        case AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT:
        case AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT:
        case AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_OUT_IEM:
        case AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER:
            if (operation > WIRED_AUDIO_CONFIG_OP_VOL_DB_VOICE) {
                WIRED_AUDIO_LOG_MSGID_W("scenario_sub_id %d, operation %d not support", 2, config->scenario_sub_id, operation);
                return AUDIO_TRANSMITTER_STATUS_FAIL;
            }

            if (operation == WIRED_AUDIO_CONFIG_OP_VOL_DB_VOICE) {
                for (i = 0; i < 8; i++) {
                    vol_gain.gain[i] = runtime_config->wired_audio_runtime_config.vol_db.vol_db[i] / 100; /* Change to 0.01dB */
                    WIRED_AUDIO_LOG_MSGID_I("scenario_sub_id %d, operation %d, ch %d, vol_db %d, config vol_gain to %d * 0.01dB", 5,
                                                config->scenario_sub_id, operation, i,
                                                runtime_config->wired_audio_runtime_config.vol_db.vol_db[i],
                                                vol_gain.gain[i]);
                }
                #ifdef AIR_MUTE_MIC_DETECTION_ENABLE
                if(config->scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT){
                    if(vol_gain.gain[0] <= -9600){
                        g_usb_out_vad_enable = true;
                    } else {
                        g_usb_out_vad_enable = false;
                    }
                }
                #endif
            } else {
                /* Always keep all channels to same gain */
                volume_level = runtime_config->wired_audio_runtime_config.vol_level.vol_level[0];
                if (volume_level > bt_sink_srv_ami_get_usb_voice_sw_max_volume_level()) {
                    volume_level = bt_sink_srv_ami_get_usb_voice_sw_max_volume_level();
                    WIRED_AUDIO_LOG_MSGID_W("scenario_sub_id %d, operation %d, set volume %d level more than max level %d\r\n", 4,
                                                config->scenario_sub_id, operation,
                                                volume_level, bt_sink_srv_ami_get_usb_music_sw_max_volume_level());
                }
                vol_gain.gain[0] = audio_get_gain_in_in_dB(volume_level, GAIN_DIGITAL, VOL_USB_VOICE_SW_OUT);
            }
            runtime_config_param->config_operation = operation;
            memcpy(runtime_config_param->config_param, &vol_gain, sizeof(vol_gain_t));
            WIRED_AUDIO_LOG_MSGID_I("scenario_sub_id %d, operation %d, config vol_gain to %d dB", 3,
                                        config->scenario_sub_id, operation, vol_gain.gain[0]);

            break;
    }

    return AUDIO_TRANSMITTER_STATUS_SUCCESS;
}

void wired_audio_state_starting_handler(uint8_t scenario_sub_id)
{
    WIRED_AUDIO_LOG_MSGID_I("[Wired_audio] starting", 1, scenario_sub_id);

    if (0) {
        /* Dummy if branch */
    }
#ifdef AIR_USB_AUDIO_ENABLE
    else if (scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) {
#ifdef HAL_DVFS_MODULE_ENABLED
        hal_dvfs_lock_control(WIRED_AUDIO_DVFS_SPEED, HAL_DVFS_LOCK);
#endif
        WIRED_AUDIO_LOG_MSGID_I("[USB_IN] 0 starting", 0);
    } else if (scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1) {
#ifdef HAL_DVFS_MODULE_ENABLED
        hal_dvfs_lock_control(WIRED_AUDIO_DVFS_SPEED, HAL_DVFS_LOCK);
#endif
        WIRED_AUDIO_LOG_MSGID_I("[USB_IN] 1 starting", 0);
    } else if (scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT){
#ifdef HAL_DVFS_MODULE_ENABLED
        hal_dvfs_lock_control(WIRED_AUDIO_DVFS_SPEED, HAL_DVFS_LOCK);
#endif
        WIRED_AUDIO_LOG_MSGID_I("[USB_OUT] starting", 0);
    }
#endif
    else if ((scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN) ||
        (scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER) ||
        (scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE)) {
#ifdef HAL_DVFS_MODULE_ENABLED
        hal_dvfs_lock_control(WIRED_AUDIO_LINE_IN_DVFS_SPEED, HAL_DVFS_LOCK);
#endif
        WIRED_AUDIO_LOG_MSGID_I("[LINE_IN] starting", 0);
    }
    else if ((scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT) ||
        (scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER)) {
#ifdef HAL_DVFS_MODULE_ENABLED
        hal_dvfs_lock_control(WIRED_AUDIO_DVFS_SPEED, HAL_DVFS_LOCK);
#endif
        WIRED_AUDIO_LOG_MSGID_I("[LINE_OUT] starting", 0);
    }
}

void wired_audio_state_started_handler(uint8_t scenario_sub_id)
{
    if (0) {
        /* Dummy if branch */
    }
#ifdef AIR_USB_AUDIO_ENABLE
    else if (scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) {
#ifdef AIR_USB_AUDIO_ENABLE
        g_usb_audio_first_rx_in_flag[0] = true;
#ifdef AIR_WIRED_AUDIO_UT_DEBUG_ENABLE
        UT_usb_audio_rx_0_ut_init();
#else
        USB_Audio_Register_Rx_Callback(0, usb_audio_rx_cb_wired_audio_0);
#endif
#endif
        usb_audio_usb_in_0_started_flag = true;
        WIRED_AUDIO_LOG_MSGID_I("[USB_IN] 0 started", 0);
    } else if (scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1) {
#ifdef AIR_USB_AUDIO_ENABLE
        g_usb_audio_first_rx_in_flag[1] = true;
#ifdef AIR_WIRED_AUDIO_UT_DEBUG_ENABLE
        UT_usb_audio_rx_1_ut_init();
#else
        USB_Audio_Register_Rx_Callback(1, usb_audio_rx_cb_wired_audio_1);
#endif
#endif
        usb_audio_usb_in_1_started_flag = true;
        WIRED_AUDIO_LOG_MSGID_I("[USB_IN] 1 started", 0);
    } else if (scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT){
        g_usb_audio_first_tx_in_flag = true;
        s_usb_out_voice_data = NULL;
#ifdef AIR_WIRED_AUDIO_UT_DEBUG_ENABLE
        UT_usb_audio_tx_ut_init();
#else
        USB_Audio_Register_Tx_Callback(0, usb_audio_tx_cb_wired_audio_0);
#endif
        usb_audio_usb_out_started_flag = true;
        WIRED_AUDIO_LOG_MSGID_I("[USB_OUT] started", 0);
#ifdef AIR_MUTE_MIC_DETECTION_ENABLE
        audio_volume_monitor_param_t param = {0};
        param.cb = mute_mic_detection;
        param.volume_len = 1;
        param.ch = 1;
        param.user_data  = NULL;
        audio_volume_monitor_start(AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT, &param);
#endif
    }
#endif
    else if ((scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN) ||
        (scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER) ||
        (scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE)) {
        usb_audio_line_in_started_flag = true;
        WIRED_AUDIO_LOG_MSGID_I("[LINE_IN] started", 0);
    }
    else if ((scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT) ||
        (scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER)) {
        usb_audio_line_out_started_flag = true;
        WIRED_AUDIO_LOG_MSGID_I("[LINE_OUT] started", 0);
    }
}

void wired_audio_state_idle_handler(uint8_t scenario_sub_id)
{
    if (0) {
        /* Dummy if branch */
    }
#ifdef AIR_USB_AUDIO_ENABLE
    else if (scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0) {
#ifdef AIR_USB_AUDIO_ENABLE
#ifdef AIR_WIRED_AUDIO_UT_DEBUG_ENABLE
        hal_gpt_status_t status;
        status = hal_gpt_stop_timer(HAL_GPT_7);
        WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_stop_timer %d", 1, status);
        status = hal_gpt_deinit(HAL_GPT_7);
        WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_deinit %d", 1, status);
#else
        USB_Audio_Register_Rx_Callback(0, NULL);
#endif
#endif
#ifdef HAL_DVFS_MODULE_ENABLED
        if (usb_audio_usb_in_0_started_flag == true) {
            hal_dvfs_lock_control(WIRED_AUDIO_DVFS_SPEED, HAL_DVFS_UNLOCK);
        }
#endif
        usb_audio_usb_in_0_started_flag = false;
        WIRED_AUDIO_LOG_MSGID_I("[USB_IN] 0 stoped", 0);
    } else if (scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1) {
#ifdef AIR_USB_AUDIO_ENABLE
#ifdef AIR_WIRED_AUDIO_UT_DEBUG_ENABLE
        hal_gpt_status_t status;
        status = hal_gpt_stop_timer(HAL_GPT_7);
        WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_stop_timer %d", 1, status);
        status = hal_gpt_deinit(HAL_GPT_7);
        WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_deinit %d", 1, status);
#else
        USB_Audio_Register_Rx_Callback(1, NULL);
#endif
#endif
#ifdef HAL_DVFS_MODULE_ENABLED
        if (usb_audio_usb_in_1_started_flag == true) {
            hal_dvfs_lock_control(WIRED_AUDIO_DVFS_SPEED, HAL_DVFS_UNLOCK);
        }
#endif
        usb_audio_usb_in_1_started_flag = false;
        WIRED_AUDIO_LOG_MSGID_I("[USB_IN] 1 stoped", 0);
    } else if (scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_USB_OUT) {
#ifdef AIR_WIRED_AUDIO_UT_DEBUG_ENABLE
        hal_gpt_status_t status;
        status = hal_gpt_stop_timer(HAL_GPT_7);
        WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_stop_timer %d", 1, status);
        status = hal_gpt_deinit(HAL_GPT_7);
        WIRED_AUDIO_DEBUG_LOG_MSGID_I("[UT_TEST]: hal_gpt_deinit %d", 1, status);
#else
        USB_Audio_Register_Tx_Callback(0, NULL);
#endif
#ifdef HAL_DVFS_MODULE_ENABLED
        if (usb_audio_usb_out_started_flag == true) {
            hal_dvfs_lock_control(WIRED_AUDIO_DVFS_SPEED, HAL_DVFS_UNLOCK);
        }
#endif
        usb_audio_usb_out_started_flag = false;
        WIRED_AUDIO_LOG_MSGID_I("[USB_OUT] stoped", 0);
#ifdef AIR_MUTE_MIC_DETECTION_ENABLE
        audio_volume_monitor_stop(AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_OUT);
#endif
    }
#endif
    else if ((scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_IN) ||
        (scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_MASTER) ||
        (scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_IN_SLAVE)) {
#ifdef HAL_DVFS_MODULE_ENABLED
        if (usb_audio_line_in_started_flag == true) {
            hal_dvfs_lock_control(WIRED_AUDIO_LINE_IN_DVFS_SPEED, HAL_DVFS_UNLOCK);
        }
#endif
        usb_audio_line_in_started_flag = false;
        WIRED_AUDIO_LOG_MSGID_I("[LINE_IN] stoped", 0);
    }
    else if ((scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_LINE_OUT) ||
        (scenario_sub_id == AUDIO_TRANSMITTER_WIRED_AUDIO_DUAL_CHIP_LINE_OUT_MASTER)) {
#ifdef HAL_DVFS_MODULE_ENABLED
        if (usb_audio_line_out_started_flag == true) {
            hal_dvfs_lock_control(WIRED_AUDIO_DVFS_SPEED, HAL_DVFS_UNLOCK);
        }
#endif
        if (is_ext_dac_initialized == true) {
            // for special external DAC
#ifdef AIR_BTA_IC_PREMIUM_G2
#ifndef AIR_LINE_OUT_USE_DAC_ENABLE
            bsp_audio_ext_codec_dac_disable(BSP_AUDIO_EXT_CODEC_TYPE_DA7212);
            bsp_audio_ext_codec_dac_deinit(BSP_AUDIO_EXT_CODEC_TYPE_DA7212);
#endif
#endif
            is_ext_dac_initialized = false;
        }
        usb_audio_line_out_started_flag = false;
        WIRED_AUDIO_LOG_MSGID_I("[LINE_OUT] stoped", 0);
    }
}


uint16_t wired_audio_restore_feature_mode_nvkey_id(uint16_t nvkey_id)
{
    uint16_t repalce_nvkey_id;
    repalce_nvkey_id = nvkey_id;
    WIRED_AUDIO_UNUSED(repalce_nvkey_id);
    WIRED_AUDIO_UNUSED(nvkey_id);

#ifdef AIR_AUDIO_DETACHABLE_MIC_ENABLE
        if (nvkey_id == NVKEY_DSP_PARA_AEC_BOOMIC) {
            repalce_nvkey_id = NVKEY_DSP_PARA_AEC;
        } if (nvkey_id == NVKEY_DSP_PARA_NR_BOOMIC) {
            repalce_nvkey_id = NVKEY_DSP_PARA_NR;
        } if (nvkey_id == NVKEY_DSP_PARA_INEAR_BOOMIC) {
            repalce_nvkey_id = NVKEY_DSP_PARA_INEAR;
        } else if (nvkey_id == NVKEY_DSP_PARA_NB_TX_EQ_BOOMIC) {
            repalce_nvkey_id = NVKEY_DSP_PARA_NB_TX_EQ;
        } else if (nvkey_id == NVKEY_DSP_PARA_WB_TX_EQ_BOOMIC) {
            repalce_nvkey_id = NVKEY_DSP_PARA_WB_TX_EQ;
        } else if (nvkey_id == NVKEY_DSP_PARA_WB_TX_VO_CPD_BOOMIC) {
            repalce_nvkey_id = NVKEY_DSP_PARA_WB_TX_VO_CPD;
        } else if (nvkey_id == NVKEY_DSP_PARA_NB_TX_FIR_EQ_BOOMIC) {
            repalce_nvkey_id = NVKEY_DSP_PARA_NB_TX_FIR_EQ;
        } else if (nvkey_id == NVKEY_DSP_PARA_WB_TX_FIR_EQ_BOOMIC) {
            repalce_nvkey_id = NVKEY_DSP_PARA_WB_TX_FIR_EQ;
        }
#endif
    return repalce_nvkey_id;
}




audio_transmitter_scenario_list_t audio_transmitter_wired_audio_mix_stream_list[] = {
                                                                                    #ifdef AIR_DCHS_MODE_ENABLE
                                                                                    #else
                                                                                    {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_0},
                                                                                    {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_1},
                                                                                    #endif
                                                                                    #ifdef AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE
                                                                                    {AUDIO_TRANSMITTER_WIRED_AUDIO, AUDIO_TRANSMITTER_WIRED_AUDIO_USB_IN_OUT_IEM},
                                                                                    #endif
                                                                                   };
bool audio_transmitter_wired_audio_check_mix_stream(audio_transmitter_scenario_type_t scenario_type, uint8_t scenario_sub_id)
{
    WIRED_AUDIO_LOG_MSGID_I("[MAIN STREAM] check_mix_stream %d, %d,%d",3, sizeof(audio_transmitter_wired_audio_mix_stream_list),scenario_type,scenario_sub_id);
    for (uint8_t i = 0; i < sizeof(audio_transmitter_wired_audio_mix_stream_list)/ sizeof(audio_transmitter_scenario_list_t); i++)
    {
        if((scenario_type == audio_transmitter_wired_audio_mix_stream_list[i].scenario_type) && (scenario_sub_id == audio_transmitter_wired_audio_mix_stream_list[i].scenario_sub_id)) {
            return true;
        }
    }
    return false;
}

#ifdef AIR_WIRED_AUDIO_SUB_STREAM_ENABLE
static bool is_wired_audio_set_substream_enable = false;
void audio_transmitter_wired_audio_set_substream(bool is_enable)
{
    is_wired_audio_set_substream_enable = is_enable;
    WIRED_AUDIO_LOG_MSGID_I("[SUB STREAM] is_enable %d", 1, is_enable);
}
bool audio_transmitter_wired_audio_get_substream(void)
{
    return is_wired_audio_set_substream_enable;
}
#endif

void audio_transmitter_wired_audio_main_stream_open_start_playback(void)
{
    mcu2dsp_open_param_t *open_param;
    void *p_param_share;
    open_param = (mcu2dsp_open_param_t *)pvPortMalloc(sizeof(mcu2dsp_open_param_t));
    memset(open_param, 0, sizeof(mcu2dsp_open_param_t));

    uint16_t scenario_and_id = (AUDIO_TRANSMITTER_WIRED_AUDIO << 8) + AUDIO_TRANSMITTER_WIRED_AUDIO_MAINSTREAM;

    WIRED_AUDIO_LOG_MSGID_I("[MAIN STREAM] open and start", 0);

    open_param->param.stream_in = STREAM_IN_VIRTUAL;
    open_param->param.stream_out = STREAM_OUT_AFE;
    open_param->audio_scenario_type = AUDIO_SCENARIO_TYPE_WIRED_AUDIO_MAINSTREAM;

    uint32_t dl_process_period = WIRED_AUDIO_USB_IN_PROCESS_PERIOD; // ms for low latency.

    open_param->stream_in_param.virtual_param.virtual_mem_size = (dl_process_period * WIRED_AUDIO_MIXING_MODE_USB_DL_OUT_SAMPLE_RATE) / 1000 * 4;

    /* Config the structure of stream out, same with USB IN */
    open_param->stream_out_param.afe.memory = HAL_AUDIO_MEM3;
    open_param->stream_out_param.afe.format = HAL_AUDIO_PCM_FORMAT_S24_LE;
    open_param->stream_out_param.afe.stream_out_sampling_rate = WIRED_AUDIO_MIXING_MODE_USB_DL_OUT_SAMPLE_RATE;
    open_param->stream_out_param.afe.sampling_rate = WIRED_AUDIO_MIXING_MODE_USB_DL_OUT_SAMPLE_RATE;

    open_param->stream_out_param.afe.frame_number = 4; /* DSP always use default frame number */
    open_param->stream_out_param.afe.irq_period = dl_process_period;
    open_param->stream_out_param.afe.frame_size = (dl_process_period * open_param->stream_out_param.afe.sampling_rate) / 1000;
    open_param->stream_out_param.afe.hw_gain = true;
#ifdef ENABLE_HWSRC_CLKSKEW
    open_param->stream_out_param.afe.clkskew_mode = CLK_SKEW_V1;
#endif


#ifdef AIR_WIRED_AUDIO_SUB_STREAM_ENABLE
        // hal_gpio_init(3);
        // hal_pinmux_set_function(3, 3);
        // hal_gpio_init(4);
        // hal_pinmux_set_function(4, 3);
        // hal_gpio_init(6);
        // hal_pinmux_set_function(6, 3);
    if(audio_transmitter_wired_audio_get_substream() == true){
        hal_audio_get_stream_out_setting_config(AU_DSP_LINEIN, &open_param->stream_out_param);
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_WIRED_AUDIO_SUBSTREAM, open_param, true);
        open_param->stream_out_param.afe.audio_device1 = open_param->stream_out_param.afe.audio_device;
        open_param->stream_out_param.afe.audio_interface1 = open_param->stream_out_param.afe.audio_interface;
    }
#endif
    hal_audio_get_stream_out_setting_config(AU_DSP_AUDIO, &open_param->stream_out_param);

#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
            //hal_dvfs_lock_control(HAL_DVFS_OPP_HIGH, HAL_DVFS_LOCK);
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_1DAC_1I2S_ENABLE
            open_param->stream_out_param.afe.audio_device     = HAL_AUDIO_DEVICE_DAC_DUAL;
            open_param->stream_out_param.afe.audio_interface  = HAL_AUDIO_INTERFACE_1;
            open_param->stream_out_param.afe.audio_device1    = HAL_AUDIO_DEVICE_I2S_MASTER;
            open_param->stream_out_param.afe.audio_interface1 = HAL_AUDIO_INTERFACE_1;
            open_param->stream_out_param.afe.audio_device2    = HAL_AUDIO_DEVICE_NONE;
            open_param->stream_out_param.afe.audio_interface2 = HAL_AUDIO_DEVICE_NONE;
            open_param->stream_out_param.afe.memory           |= HAL_AUDIO_MEM1;
            open_param->stream_out_param.afe.is_low_jitter[0]    = true;
#elif AIR_AUDIO_MULTIPLE_STREAM_OUT_3I2S_ENABLE
            //open_param->stream_in_param.afe.frame_size = 512;
            //open_param->stream_out_param.afe.frame_size = 512;
            open_param->stream_out_param.afe.audio_device     = HAL_AUDIO_DEVICE_I2S_MASTER;
            open_param->stream_out_param.afe.audio_interface  = HAL_AUDIO_INTERFACE_1;
            open_param->stream_out_param.afe.audio_device1    = HAL_AUDIO_DEVICE_I2S_MASTER;
            open_param->stream_out_param.afe.audio_interface1 = HAL_AUDIO_INTERFACE_2;
            open_param->stream_out_param.afe.audio_device2    = HAL_AUDIO_DEVICE_I2S_MASTER;
            //open_param->stream_out_param.afe.audio_device2    = HAL_AUDIO_DEVICE_DAC_DUAL;
            open_param->stream_out_param.afe.audio_interface2 = HAL_AUDIO_INTERFACE_3;
            open_param->stream_out_param.afe.memory           |= HAL_AUDIO_MEM1|HAL_AUDIO_MEM4;
#endif
            WIRED_AUDIO_LOG_MSGID_I("out_device0(0x%x), channel(%d), interface0(0x%x)", 3, open_param->stream_out_param.afe.audio_device, open_param->stream_out_param.afe.stream_channel, open_param->stream_out_param.afe.audio_interface);
            WIRED_AUDIO_LOG_MSGID_I("out_device1(0x%x), channel(%d), interface1(0x%x)", 3, open_param->stream_out_param.afe.audio_device1, open_param->stream_out_param.afe.stream_channel, open_param->stream_out_param.afe.audio_interface1);
            WIRED_AUDIO_LOG_MSGID_I("out_device2(0x%x), channel(%d), interface2(0x%x)", 3, open_param->stream_out_param.afe.audio_device2, open_param->stream_out_param.afe.stream_channel, open_param->stream_out_param.afe.audio_interface2);
            WIRED_AUDIO_LOG_MSGID_I("memory(0x%x)", 1, open_param->stream_out_param.afe.memory);
#endif

    ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_WIRED_AUDIO_MAINSTREAM, open_param, true);

    if ((open_param->stream_out_param.afe.audio_device == HAL_AUDIO_DEVICE_I2S_MASTER)||(open_param->stream_out_param.afe.audio_device1 == HAL_AUDIO_DEVICE_I2S_MASTER)) {
        open_param->stream_out_param.afe.misc_parms      = I2S_CLK_SOURCE_DCXO;
    } else {
        open_param->stream_out_param.afe.misc_parms      = DOWNLINK_PERFORMANCE_NORMAL;
    }

    p_param_share = hal_audio_dsp_controller_put_paramter(open_param, sizeof(mcu2dsp_open_param_t), AUDIO_MESSAGE_TYPE_AUDIO_TRANSMITTER);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_AUDIO_TRANSMITTER_OPEN, scenario_and_id, (uint32_t)p_param_share, true);
    vPortFree(open_param);

    mcu2dsp_start_param_t *start_param;
    start_param = (mcu2dsp_start_param_t *)pvPortMalloc(sizeof(mcu2dsp_start_param_t));
    memset(start_param, 0, sizeof(mcu2dsp_start_param_t));
    start_param->param.stream_in = STREAM_IN_VIRTUAL;
    start_param->param.stream_out = STREAM_OUT_AFE;
    p_param_share = hal_audio_dsp_controller_put_paramter(start_param, sizeof(mcu2dsp_start_param_t), AUDIO_MESSAGE_TYPE_AUDIO_TRANSMITTER);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_AUDIO_TRANSMITTER_START, scenario_and_id, (uint32_t)p_param_share, true);
    vPortFree(start_param);
}

void audio_transmitter_wired_audio_main_stream_stop_close_playback(void)
{
    uint16_t scenario_and_id = (AUDIO_TRANSMITTER_WIRED_AUDIO << 8) + AUDIO_TRANSMITTER_WIRED_AUDIO_MAINSTREAM;

    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_AUDIO_TRANSMITTER_STOP, scenario_and_id, 0, true);
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_AUDIO_TRANSMITTER_CLOSE, scenario_and_id, 0, true);

    ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_WIRED_AUDIO_MAINSTREAM, NULL, false);
#ifdef AIR_WIRED_AUDIO_SUB_STREAM_ENABLE
    if(ami_hal_audio_status_query_running_flag(AUDIO_SCENARIO_TYPE_WIRED_AUDIO_SUBSTREAM) == true) {
        ami_hal_audio_status_set_running_flag(AUDIO_SCENARIO_TYPE_WIRED_AUDIO_SUBSTREAM, NULL, false);
    }
#endif

    WIRED_AUDIO_LOG_MSGID_I("[MAIN STREAM] stop and close", 0);
}
#endif

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

#if defined(AIR_GAMING_MODE_DONGLE_ENABLE)

/* Includes ------------------------------------------------------------------*/
#include "types.h"
#include "source_inter.h"
#include "sink_inter.h"
#include "dsp_buffer.h"
#include "dsp_memory.h"
#include "dsp_callback.h"
#include "dsp_temp.h"
#include "dsp_dump.h"
#include "dsp_scenario.h"
#include "dsp_audio_msg_define.h"
#include "dsp_audio_process.h"
#include "stream_audio_transmitter.h"
#include "bt_types.h"
#include "hal_gpt.h"
#include "hal_sleep_manager.h"
#include "memory_attribute.h"
#include "scenario_ull_audio.h"
#include "stream_bt_common.h"
#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
#include "volume_estimator_interface.h"
#include "dtm.h"
#include "preloader_pisplit.h"
#include "audio_nvdm_common.h"
#include "stream_nvkey_struct.h"
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */
#ifdef MTK_CELT_DEC_ENABLE
#include "celt_dec_interface.h"
#endif /* MTK_CELT_DEC_ENABLE */
#include "audio_transmitter_mcu_dsp_common.h"
#include "hal_audio_message_struct_common.h"
#include "scenario_audio_common.h"
#include "hal_nvic_internal.h"

/* Private define ------------------------------------------------------------*/
#define GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG          0
#define GAMING_MODE_MUSIC_DONGLE_DEBUG_DUMP         1
#define GAMING_MODE_MUSIC_DONGLE_DEBUG_LANTENCY     0
#define GAMING_MODE_VOICE_DONGLE_DEBUG_LOG          0
#define GAMING_MODE_VOICE_DONGLE_DEBUG_DUMP         1

// #define GAMING_MODE_VOICE_PLAYEN_DELAY              (24*2)
// #define GAMING_MODE_VOICE_FIRST_PACKET_SAFE_DELAY   (24*2)
#define GAMING_MODE_VOICE_PLAYEN_DELAY              (24+13)
#define GAMING_MODE_VOICE_FIRST_PACKET_SAFE_DELAY   (24+8)
#define GAMING_MODE_VOICE_USB_FRAME_SAMPLES         (40)
#define GAMING_MODE_VOICE_BT_FRAME_SAMPLES          (120)
#define GAMING_MODE_VOICE_FRAME_INDEX_MAX           (3)

#ifndef min
#define min(_a, _b)   (((_a)<(_b))?(_a):(_b))
#endif

#define GAMING_MODE_AFE_LINEIN_PLAYEN_DELAY         (24*sizeof(int16_t))
log_create_module(ull_log, PRINT_LEVEL_INFO);
/* Private typedef -----------------------------------------------------------*/
typedef struct {
    int32_t gain_1;
    int32_t gain_2;
} op_vol_level_param_t;

typedef struct {
    uint8_t scenario_id;
    uint8_t sub_id;
} dl_mixer_param_t;

typedef struct {
    uint32_t latency_us;
} latency_param_t;

typedef union {
    op_vol_level_param_t      vol_level;
    dl_mixer_param_t          dl_mixer_param;
    latency_param_t           latency_param;
} gaming_mode_runtime_config_operation_param_t, *gaming_mode_runtime_config_operation_param_p;

typedef struct {
    gaming_mode_runtime_config_operation_t          config_operation;
    gaming_mode_runtime_config_operation_param_t    config_param;
} gaming_mode_runtime_config_param_t, *gaming_mode_runtime_config_param_p;

typedef struct {
    uint16_t DCXO_CAPID_EFUSE_SEL;
    uint16_t DCXO_CAPID_EFUSE;
} __attribute__((packed)) dcxo_capid_efuse_t;
#define DCXO_CAPID_EFUSE_REG            (*((dcxo_capid_efuse_t*)0xA2060018))

#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
typedef struct {
    int16_t enable;
    uint16_t gain_setting_update_done;
    volume_estimator_port_t *port;
    gaming_mode_dongle_dl_handle_t *chat_dongle_handle;
    sw_gain_port_t *game_path_gain_port;
    void *nvkey_buf;
    uint32_t frame_size;
    uint32_t data_size;
    uint32_t data_buf_size;
    void *data_buf;
    int32_t current_db;
    int32_t failure_threshold_db;
    int32_t effective_threshold_db;
    int32_t adjustment_amount_db;
    int32_t up_step_db;
    int32_t down_step_db;
    uint32_t effective_delay_ms;
    uint32_t effective_delay_us_count;
    uint32_t failure_delay_ms;
    uint32_t failure_delay_us_count;
    bool active_flag;
    op_vol_level_param_t chat_path_default_vol_level;
    op_vol_level_param_t game_path_default_vol_level;
    op_vol_level_param_t game_path_balance_vol_level;
} gaming_mode_vol_balance_handle_t;
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern const uint32_t crc32_tab[];
/* Public variables ----------------------------------------------------------*/
volatile uint32_t gaming_mode_music_ccni_bt_clk = 0;
volatile uint32_t gaming_mode_music_ccni_gpt_count = 0;
volatile uint32_t gaming_mode_music_data_in_gpt_count = 0;
gaming_mode_dongle_dl_handle_t *gaming_mode_dongle_first_dl_handle = NULL;
gaming_mode_dongle_ul_handle_t *gaming_mode_dongle_first_ul_handle = NULL;
#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LANTENCY
volatile uint32_t gaming_mode_music_ccni_gpt_count1 = 0;
volatile uint32_t gaming_mode_music_ccni_gpt_count2[2];
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LANTENCY */
volatile uint32_t gaming_mode_music_bt_out_gpt_count = 0;
volatile uint32_t gaming_mode_music_first_packet_bt_clk = 0;
volatile uint32_t gaming_mode_voice_usb_out_gpt_count = 0;
SINK gaming_mode_dongle_sink = NULL;
#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
gaming_mode_vol_balance_handle_t gaming_mode_vol_balance_handle;
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */

#ifdef AIR_GAMING_MODE_DONGLE_ECNR_ENABLE
static uint8_t g_post_ec_gain;
#endif /* AIR_GAMING_MODE_DONGLE_ECNR_ENABLE */

#ifdef AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE
static bool g_vdma_start_flag = false;
#endif

/* Private functions ---------------------------------------------------------*/
gaming_mode_dongle_dl_handle_t *gaming_mode_dongle_query_dl_handle(void *owner);
#ifdef AIR_GAMING_MODE_DONGLE_ECNR_ENABLE
uint8_t aec_nr_port_get_postec_gain(void)
{
    return g_post_ec_gain;
}
#endif /* AIR_GAMING_MODE_DONGLE_ECNR_ENABLE */

#ifdef AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE
// extern hal_gpt_status_t hal_gpt_start_timer_us(hal_gpt_port_t gpt_port, uint32_t timeout_time_us, hal_gpt_timer_type_t timer_type);
// extern hal_gpt_status_t hal_gpt_stop_timer(hal_gpt_port_t gpt_port);
// extern hal_gpt_status_t hal_gpt_register_callback(hal_gpt_port_t    gpt_port,
//         hal_gpt_callback_t   callback,
//         void                *user_data);
// extern hal_gpt_status_t hal_gpt_init(hal_gpt_port_t gpt_port);
// extern hal_gpt_status_t hal_gpt_deinit(hal_gpt_port_t gpt_port);
static uint32_t g_line_out_timer_handle = 0;
void gaming_mode_dongle_ul_afe_out_gpt_irq(void);
//== Ring buffer opeartion ==
/*@brief     circular buffer(ring buffer) implemented by mirroring, which keep an extra bit to distinguish empty and full situation. */
uint32_t ring_buffer_get_data_byte_count(ring_buffer_information_t *p_info)
{
    uint32_t buffer_byte_count = p_info->buffer_byte_count;
    uint32_t write_pointer     = p_info->write_pointer;
    uint32_t read_pointer      = p_info->read_pointer;
    uint32_t data_byte_count;
    if (write_pointer >= read_pointer) {
        data_byte_count = write_pointer - read_pointer;
    } else {
        data_byte_count = (buffer_byte_count << 1) - read_pointer + write_pointer;
    }
    return data_byte_count;
}

uint32_t ring_buffer_get_space_byte_count(ring_buffer_information_t *p_info)
{
    return p_info->buffer_byte_count - ring_buffer_get_data_byte_count(p_info);
}

void ring_buffer_get_write_information(ring_buffer_information_t *p_info, uint8_t **pp_buffer, uint32_t *p_byte_count)
{
    uint32_t buffer_byte_count = p_info->buffer_byte_count;
    uint32_t space_byte_count  = ring_buffer_get_space_byte_count(p_info);
    uint8_t *buffer_pointer    = p_info->buffer_base_pointer;
    uint32_t write_pointer     = p_info->write_pointer;
    uint32_t tail_byte_count;
    if (write_pointer < buffer_byte_count) {
        *pp_buffer = buffer_pointer + write_pointer;
        tail_byte_count = buffer_byte_count - write_pointer;
    } else {
        *pp_buffer = buffer_pointer + write_pointer - buffer_byte_count;
        tail_byte_count = (buffer_byte_count << 1) - write_pointer;
    }
    *p_byte_count = MINIMUM(space_byte_count, tail_byte_count);
    return;
}

void ring_buffer_get_read_information(ring_buffer_information_t *p_info, uint8_t **pp_buffer, uint32_t *p_byte_count)
{
    uint32_t buffer_byte_count = p_info->buffer_byte_count;
    uint32_t data_byte_count   = ring_buffer_get_data_byte_count(p_info);
    uint8_t *buffer_pointer    = p_info->buffer_base_pointer;
    uint32_t read_pointer      = p_info->read_pointer;
    uint32_t tail_byte_count;
    if (read_pointer < buffer_byte_count) {
        *pp_buffer = buffer_pointer + read_pointer;
        tail_byte_count = buffer_byte_count - read_pointer;
    } else {
        *pp_buffer = buffer_pointer + read_pointer - buffer_byte_count;
        tail_byte_count = (buffer_byte_count << 1) - read_pointer;
    }
    *p_byte_count = MINIMUM(data_byte_count, tail_byte_count);
    return;
}

void ring_buffer_write_done(ring_buffer_information_t *p_info, uint32_t write_byte_count)
{
    uint32_t buffer_byte_count = p_info->buffer_byte_count;
    uint32_t buffer_end        = buffer_byte_count << 1;
    uint32_t write_pointer     = p_info->write_pointer + write_byte_count;
    p_info->write_pointer = write_pointer >= buffer_end ? write_pointer - buffer_end : write_pointer;
    return;
}

void ring_buffer_read_done(ring_buffer_information_t *p_info, uint32_t read_byte_count)
{
    uint32_t buffer_byte_count = p_info->buffer_byte_count;
    uint32_t buffer_end        = buffer_byte_count << 1;
    uint32_t read_pointer      = p_info->read_pointer + read_byte_count;
    p_info->read_pointer = read_pointer >= buffer_end ? read_pointer - buffer_end : read_pointer;
    return;
}
#endif /* AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE */

static gaming_mode_dongle_ul_first_packet_status_t gaming_mode_dongle_ul_first_packet_check(gaming_mode_dongle_ul_handle_t *dongle_handle, uint32_t anchor)
{
    uint32_t bt_clk;
    uint16_t bt_phase;
    uint32_t anchor_safe;
    uint32_t anchor_last;
    UNUSED(dongle_handle);

    /* get current bt clock */
    extern VOID MCE_GetBtClk(BTCLK * pCurrCLK, BTPHASE * pCurrPhase, BT_CLOCK_OFFSET_SCENARIO type);
    MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&bt_phase, BT_CLK_Offset);

    anchor_safe = (anchor + GAMING_MODE_VOICE_FIRST_PACKET_SAFE_DELAY) & 0x0fffffff;
    anchor_last = (anchor + 0x10000000 - 24) & 0x0fffffff;

    if ((anchor_safe > anchor) && (anchor > anchor_last)) {
        if ((bt_clk >= anchor_last) && (bt_clk < anchor_safe)) {
            /* --------- ........ --------- anchor_last --------- anchor ---------- bt_clk ---------- anchor_safe --------- ........ -------- */
            return GAMING_MODE_DONGLE_UL_FIRST_PACKET_READY;
        } else {
            /* --------- ........ --------- anchor_last --------- anchor ---------- anchor_safe ---------- bt_clk --------- ........ -------- */
            ULL_LOG_E("[bt common][source]: first packet is timeout, 0x%x 0x%x", 2, anchor, bt_clk);
            return GAMING_MODE_DONGLE_UL_FIRST_PACKET_TIMEOUT;
        }
    } else if ((anchor_safe > anchor) && (anchor < anchor_last)) {
        if ((bt_clk >= anchor) && (bt_clk < anchor_safe)) {
            /* --------- ........ --------- anchor ---------- bt_clk ---------- anchor_safe --------- ........ --------- anchor_last -------- */
            return GAMING_MODE_DONGLE_UL_FIRST_PACKET_READY;
        } else if (bt_clk >= anchor_last) {
            /* --------- ........ --------- anchor  ---------- anchor_safe --------- ........ --------- anchor_last ---------- bt_clk -------- */
            return GAMING_MODE_DONGLE_UL_FIRST_PACKET_READY;
        } else if (bt_clk < anchor) {
            /* ---------- bt_clk --------- ........ --------- anchor  ---------- anchor_safe --------- ........ --------- anchor_last  -------- */
            return GAMING_MODE_DONGLE_UL_FIRST_PACKET_READY;
        } else {
            /* --------- ........ --------- anchor  ---------- anchor_safe ---------- bt_clk --------- ........ --------- anchor_last -------- */
            ULL_LOG_E("[bt common][source]: first packet is timeout, 0x%x 0x%x", 2, anchor, bt_clk);
            return GAMING_MODE_DONGLE_UL_FIRST_PACKET_TIMEOUT;
        }
    } else {
        if ((bt_clk >= anchor) || (bt_clk < anchor_safe)) {
            /* --------- anchor_safe --------- ........ ---------- ........ --------- anchor -------- bt_clk ---------- */
            return GAMING_MODE_DONGLE_UL_FIRST_PACKET_READY;
        } else if ((bt_clk < anchor) && (bt_clk >= anchor_last)) {
            /* --------- anchor_safe --------- ........ ---------- ........ --------- anchor_last -------- bt_clk --------- anchor  ---------- */
            return GAMING_MODE_DONGLE_UL_FIRST_PACKET_READY;
        } else {
            /* --------- anchor_safe --------- ........ -------- bt_clk ---------- ........ --------- anchor_last --------- anchor  ---------- */
            ULL_LOG_E("[bt common][source]: first packet is timeout, 0x%x 0x%x", 2, anchor, bt_clk);
            return GAMING_MODE_DONGLE_UL_FIRST_PACKET_TIMEOUT;
        }
    }

    ULL_LOG_E("[bt common][source]: first packet is not ready, 0x%x 0x%x", 2, anchor, bt_clk);
    return GAMING_MODE_DONGLE_UL_FIRST_PACKET_NOT_READY;
}

static uint32_t gaming_mode_dongle_ul_check_play(gaming_mode_dongle_ul_handle_t *dongle_handle)
{
    uint32_t bt_clk;
    uint16_t bt_phase;

    /* get current bt clock */
    extern VOID MCE_GetBtClk(BTCLK * pCurrCLK, BTPHASE * pCurrPhase, BT_CLOCK_OFFSET_SCENARIO type);
    MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&bt_phase, BT_CLK_Offset);

    /* Note: first_packet_bt_clk should be later than first_anchor_bt_clk */
    // if (bt_clk >= dongle_handle->first_packet_bt_clk)
    // {
    //     if ((bt_clk - dongle_handle->first_packet_bt_clk) >= GAMING_MODE_VOICE_PLAYEN_DELAY)
    //     {
    //         /* --------- ........ --------- first_packet_bt_clk ---------- >= GAMING_MODE_VOICE_PLAYEN_DELAY ---------- bt_clk --------- ........ -------- */
    //         return 1;
    //     }
    // }
    // else
    // {
    //     if ((bt_clk + 0x0fffffff + 1 - dongle_handle->first_packet_bt_clk) >= GAMING_MODE_VOICE_PLAYEN_DELAY)
    //     {
    //         /* --------- >= GAMING_MODE_VOICE_PLAYEN_DELAY --------- bt_clk ---------- ........ ---------- first_packet_bt_clk --------- ........ -------- */
    //         return 1;
    //     }
    // }

    if (dongle_handle->play_en_overflow) {
        if ((dongle_handle->play_en_bt_clk <= bt_clk) && (dongle_handle->first_anchor_bt_clk <= dongle_handle->first_packet_bt_clk) && (dongle_handle->first_anchor_bt_clk > bt_clk)) {
            /* --------- play_en_bt_clk --------- ........ ---------- bt_clk ---------- ........ --------- first_anchor_bt_clk -------- first_packet_bt_clk -------- */
            return 1;
        } else if ((dongle_handle->play_en_bt_clk <= bt_clk) && (dongle_handle->first_anchor_bt_clk > dongle_handle->first_packet_bt_clk) && (dongle_handle->first_packet_bt_clk > bt_clk)) {
            /* --------- play_en_bt_clk --------- ........ ---------- bt_clk ---------- ........ -------- first_packet_bt_clk --------- first_anchor_bt_clk  -------- */
            return 1;
        }
    } else {
        if (dongle_handle->play_en_bt_clk <= bt_clk) {
            /* --------- ........ --------- first_anchor_bt_clk -------- first_packet_bt_clk ---------- play_en_bt_clk ---------- bt_clk --------- ........ -------- */
            /* --------- ........ --------- first_packet_bt_clk -------- first_anchor_bt_clk ---------- play_en_bt_clk ---------- bt_clk --------- ........ -------- */
            return 1;
        } else if ((dongle_handle->play_en_bt_clk > bt_clk) && (dongle_handle->first_anchor_bt_clk > bt_clk) && (dongle_handle->first_packet_bt_clk > bt_clk)) {
            /* --------- bt_clk --------- ........ -------- first_packet_bt_clk ---------- first_anchor_bt_clk ---------- play_en_bt_clk --------- ........ -------- */
            return 1;
        }

    }

    return 0;
}

ATTR_TEXT_IN_IRAM_LEVEL_1 static uint32_t CRC32_Generate(uint8_t *ptr, uint32_t length, uint32_t crc_init)
{
    const uint8_t *p;

    p = ptr;
    crc_init = crc_init ^ ~0U;

    while (length--) {
        crc_init = crc32_tab[(crc_init ^ *p++) & 0xFF] ^ (crc_init >> 8);
    }

    return crc_init ^ ~0U;
}

ATTR_TEXT_IN_IRAM_LEVEL_1 void ShareBufferCopy_I2D_16bit(uint32_t *src_buf, uint16_t *dest_buf1, uint16_t *dest_buf2, uint32_t samples)
{
    uint32_t data;
    uint32_t i;

    for (i = 0; i < samples; i++) {
        data = src_buf[i];
        dest_buf1[i] = (uint16_t)(data >> 0);
        dest_buf2[i] = (uint16_t)(data >> 16);
    }
}

/* Public functions ----------------------------------------------------------*/
/******************************************************************************/
/*           Gaming mode dongle game/chat volume balance functions            */
/******************************************************************************/
#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void gaming_mode_dongle_game_chat_volume_smart_balance_enable(void)
{
    uint32_t saved_mask;
    void *owner;
    volume_estimator_port_t *port = NULL;
    volume_estimator_config_t config;
    uint32_t data_size;
    void *data_buf;
    void *nvkey_buf;
    gaming_mode_dongle_dl_handle_t *dongle_handle;

    nvkey_buf = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, sizeof(gaming_mode_vol_balance_nvkey_t));
    if (nvkey_buf == NULL) {
        AUDIO_ASSERT(0);
    }
    if (nvkey_read_full_key(NVKEY_DSP_PARA_GAME_CHAT_VOLUME_SMART_BALANCE, nvkey_buf, sizeof(gaming_mode_vol_balance_nvkey_t)) != NVDM_STATUS_NAT_OK) {
        AUDIO_ASSERT(0);
    }

    /* get handle */
    dongle_handle = gaming_mode_vol_balance_handle.chat_dongle_handle;

    /* get chat volume estimator port */
    owner = dongle_handle->owner;
    port = volume_estimator_get_port(owner);
    if (port == NULL) {
        AUDIO_ASSERT(0);
    }

    /* init chat volume estimator port */
    config.channel_num = 1;
    config.frame_size = 480; /* 2.5ms*48K*mono*32bit */
    config.resolution = RESOLUTION_32BIT;
    config.mode = VOLUME_ESTIMATOR_CHAT_INSTANT_MODE;
    config.sample_rate = 48000;
    config.nvkey_para = (void *) & (((gaming_mode_vol_balance_nvkey_t *)nvkey_buf)->chat_vol_nvkey);
    volume_estimator_init(port, &config);

    /* malloc memory for queue stereo pcm data */
    data_size = 480; /* 2.5ms*48K*mono*32bit */
    data_buf = preloader_pisplit_malloc_memory(PRELOADER_D_HIGH_PERFORMANCE, 480);
    if (data_buf == NULL) {
        AUDIO_ASSERT(0);
    }
    memset(data_buf, 0, data_size);

    /* update state machine */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (gaming_mode_vol_balance_handle.port != NULL) {
        AUDIO_ASSERT(0);
    }
    gaming_mode_vol_balance_handle.port = port;
    gaming_mode_vol_balance_handle.nvkey_buf = nvkey_buf;
    gaming_mode_vol_balance_handle.enable = (int16_t)(((gaming_mode_vol_balance_nvkey_t *)nvkey_buf)->enable);
    gaming_mode_vol_balance_handle.gain_setting_update_done = 0;
    gaming_mode_vol_balance_handle.current_db = -144 * 100;
    gaming_mode_vol_balance_handle.effective_threshold_db = ((gaming_mode_vol_balance_nvkey_t *)nvkey_buf)->effective_threshold_db;
    gaming_mode_vol_balance_handle.effective_delay_ms = ((gaming_mode_vol_balance_nvkey_t *)nvkey_buf)->effective_delay_ms;
    gaming_mode_vol_balance_handle.effective_delay_us_count = 0;
    gaming_mode_vol_balance_handle.failure_threshold_db = ((gaming_mode_vol_balance_nvkey_t *)nvkey_buf)->failure_threshold_db;
    gaming_mode_vol_balance_handle.failure_delay_ms = ((gaming_mode_vol_balance_nvkey_t *)nvkey_buf)->failure_delay_ms;
    gaming_mode_vol_balance_handle.failure_delay_us_count = 0;
    gaming_mode_vol_balance_handle.adjustment_amount_db = ((gaming_mode_vol_balance_nvkey_t *)nvkey_buf)->adjustment_amount_db;
    gaming_mode_vol_balance_handle.up_step_db = ((gaming_mode_vol_balance_nvkey_t *)nvkey_buf)->up_step_db;
    gaming_mode_vol_balance_handle.down_step_db = ((gaming_mode_vol_balance_nvkey_t *)nvkey_buf)->down_step_db;
    gaming_mode_vol_balance_handle.active_flag = false;
    gaming_mode_vol_balance_handle.frame_size = config.frame_size;
    gaming_mode_vol_balance_handle.data_size = 0;
    gaming_mode_vol_balance_handle.data_buf_size = data_size;
    gaming_mode_vol_balance_handle.data_buf = data_buf;
    hal_nvic_restore_interrupt_mask(saved_mask);

    ULL_LOG_I("[Game/Chat balance][enable]:%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 16,
                 gaming_mode_vol_balance_handle.enable,
                 gaming_mode_vol_balance_handle.active_flag,
                 gaming_mode_vol_balance_handle.current_db,
                 gaming_mode_vol_balance_handle.effective_threshold_db,
                 gaming_mode_vol_balance_handle.effective_delay_ms,
                 gaming_mode_vol_balance_handle.failure_threshold_db,
                 gaming_mode_vol_balance_handle.failure_delay_ms,
                 gaming_mode_vol_balance_handle.adjustment_amount_db,
                 gaming_mode_vol_balance_handle.up_step_db,
                 gaming_mode_vol_balance_handle.down_step_db,
                 gaming_mode_vol_balance_handle.chat_path_default_vol_level.gain_1,
                 gaming_mode_vol_balance_handle.chat_path_default_vol_level.gain_2,
                 gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_1,
                 gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_2,
                 gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1,
                 gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2);
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void gaming_mode_dongle_game_chat_volume_smart_balance_disable(void)
{
    uint32_t saved_mask;
    volume_estimator_port_t *port;

    /* check state machine */
    if ((gaming_mode_vol_balance_handle.port == NULL) || (gaming_mode_vol_balance_handle.data_buf == NULL) || (gaming_mode_vol_balance_handle.nvkey_buf == NULL)) {
        AUDIO_ASSERT(0);
    }
    port = gaming_mode_vol_balance_handle.port;

    /* deinit chat volume estimator port */
    volume_estimator_deinit(port);

    /* free memory */
    preloader_pisplit_free_memory(gaming_mode_vol_balance_handle.data_buf);
    preloader_pisplit_free_memory(gaming_mode_vol_balance_handle.nvkey_buf);

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if ((gaming_mode_vol_balance_handle.enable) && (gaming_mode_vol_balance_handle.game_path_gain_port != NULL)) {
        /* process L-channel */
        if (gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_1 != gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1) {
            /* recover gain to the default settings */
            stream_function_sw_gain_configure_gain_target(gaming_mode_vol_balance_handle.game_path_gain_port, 1, gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_1);
            gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1 = gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_1;
        }

        /* process R-channel */
        if (gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_2 != gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2) {
            /* recover gain to the default settings */
            stream_function_sw_gain_configure_gain_target(gaming_mode_vol_balance_handle.game_path_gain_port, 2, gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_2);
            gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2 = gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_2;
        }

        /* restore ramp-up and ramp-down setting */
        stream_function_sw_gain_configure_gain_up(gaming_mode_vol_balance_handle.game_path_gain_port,   1,  1, 2);
        stream_function_sw_gain_configure_gain_up(gaming_mode_vol_balance_handle.game_path_gain_port,   2,  1, 2);
        stream_function_sw_gain_configure_gain_down(gaming_mode_vol_balance_handle.game_path_gain_port, 1, -1, 2);
        stream_function_sw_gain_configure_gain_down(gaming_mode_vol_balance_handle.game_path_gain_port, 2, -1, 2);
    }

    /* reset state machine */
    gaming_mode_vol_balance_handle.port = NULL;
    gaming_mode_vol_balance_handle.nvkey_buf = NULL;
    gaming_mode_vol_balance_handle.enable = 0;
    gaming_mode_vol_balance_handle.gain_setting_update_done = 0;
    gaming_mode_vol_balance_handle.current_db = -144 * 100;
    gaming_mode_vol_balance_handle.effective_threshold_db = 0;
    gaming_mode_vol_balance_handle.effective_delay_ms = 0;
    gaming_mode_vol_balance_handle.effective_delay_us_count = 0;
    gaming_mode_vol_balance_handle.failure_threshold_db = 0;
    gaming_mode_vol_balance_handle.failure_delay_ms = 0;
    gaming_mode_vol_balance_handle.failure_delay_us_count = 0;
    gaming_mode_vol_balance_handle.adjustment_amount_db = 0;
    gaming_mode_vol_balance_handle.up_step_db = 0;
    gaming_mode_vol_balance_handle.down_step_db = 0;
    gaming_mode_vol_balance_handle.active_flag = false;
    gaming_mode_vol_balance_handle.data_size = 0;
    gaming_mode_vol_balance_handle.data_buf_size = 0;
    gaming_mode_vol_balance_handle.data_buf = NULL;
    hal_nvic_restore_interrupt_mask(saved_mask);

    ULL_LOG_I("[Game/Chat balance][disable]:%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 16,
                 gaming_mode_vol_balance_handle.enable,
                 gaming_mode_vol_balance_handle.active_flag,
                 gaming_mode_vol_balance_handle.current_db,
                 gaming_mode_vol_balance_handle.effective_threshold_db,
                 gaming_mode_vol_balance_handle.effective_delay_ms,
                 gaming_mode_vol_balance_handle.failure_threshold_db,
                 gaming_mode_vol_balance_handle.failure_delay_ms,
                 gaming_mode_vol_balance_handle.adjustment_amount_db,
                 gaming_mode_vol_balance_handle.up_step_db,
                 gaming_mode_vol_balance_handle.down_step_db,
                 gaming_mode_vol_balance_handle.chat_path_default_vol_level.gain_1,
                 gaming_mode_vol_balance_handle.chat_path_default_vol_level.gain_2,
                 gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_1,
                 gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_2,
                 gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1,
                 gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2);
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void gaming_mode_dongle_game_chat_volume_smart_balance_do_process(void)
{
    uint32_t saved_mask;
    uint32_t frame_num;
    volume_estimator_port_t *port;
    int32_t new_gain_1;
    int32_t new_gain_2;
    bool gain_change = false;

    /* check state machine */
    if ((gaming_mode_vol_balance_handle.port == NULL) || (gaming_mode_vol_balance_handle.data_buf == NULL) || (gaming_mode_vol_balance_handle.nvkey_buf == NULL) || (gaming_mode_vol_balance_handle.data_size != 480)) {
        AUDIO_ASSERT(0);
    }
    port = gaming_mode_vol_balance_handle.port;

    if (gaming_mode_vol_balance_handle.enable)
    {
        /* update ramp-up/ramp-down settings at the first time */
        if ((gaming_mode_vol_balance_handle.game_path_gain_port != NULL) && (gaming_mode_vol_balance_handle.gain_setting_update_done == 0)) {
            /* do ramp-up and ramp-down setting */
            stream_function_sw_gain_configure_gain_up(gaming_mode_vol_balance_handle.game_path_gain_port,   1, gaming_mode_vol_balance_handle.up_step_db,   2);
            stream_function_sw_gain_configure_gain_up(gaming_mode_vol_balance_handle.game_path_gain_port,   2, gaming_mode_vol_balance_handle.up_step_db,   2);
            stream_function_sw_gain_configure_gain_down(gaming_mode_vol_balance_handle.game_path_gain_port, 1, gaming_mode_vol_balance_handle.down_step_db, 2);
            stream_function_sw_gain_configure_gain_down(gaming_mode_vol_balance_handle.game_path_gain_port, 2, gaming_mode_vol_balance_handle.down_step_db, 2);
            /* set flag */
            gaming_mode_vol_balance_handle.gain_setting_update_done = 1;
        }

        for (frame_num = 0; frame_num < (gaming_mode_vol_balance_handle.data_size/gaming_mode_vol_balance_handle.frame_size); frame_num++)
        {
            /* get the latest volume on chat path */
            if (volume_estimator_process(port, (void *)((uint8_t *)(gaming_mode_vol_balance_handle.data_buf)+gaming_mode_vol_balance_handle.frame_size*frame_num), gaming_mode_vol_balance_handle.frame_size, &gaming_mode_vol_balance_handle.current_db) != VOLUME_ESTIMATOR_STATUS_OK) {
                AUDIO_ASSERT(0);
            }
            // ULL_LOG_I("[Game/Chat balance][get chat gain]:%d\r\n", 1, gaming_mode_vol_balance_handle.current_db);

            /* check if needs to do the balance */
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            if (gaming_mode_vol_balance_handle.game_path_gain_port != NULL)
            {
                if ((gaming_mode_vol_balance_handle.current_db >= gaming_mode_vol_balance_handle.effective_threshold_db) &&
                    (gaming_mode_vol_balance_handle.effective_delay_us_count >= gaming_mode_vol_balance_handle.effective_delay_ms * 1000)) {
                    /* there is chat voice on the chat path */
                    /* process L-channel */
                    new_gain_1 = (gaming_mode_vol_balance_handle.chat_path_default_vol_level.gain_1 + gaming_mode_vol_balance_handle.adjustment_amount_db);
                    new_gain_1 = min(new_gain_1, gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_1);
                    if (gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1 != new_gain_1) {
                        /* the default gain is larger than the target and the current gain is not equal to the target, so we need to ramp down the volume on the game path */
                        stream_function_sw_gain_configure_gain_target(gaming_mode_vol_balance_handle.game_path_gain_port, 1, new_gain_1);
                        gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1 = new_gain_1;
                        gain_change = true;
                    }

                    /* process R-channel */
                    new_gain_2 = (gaming_mode_vol_balance_handle.chat_path_default_vol_level.gain_2 + gaming_mode_vol_balance_handle.adjustment_amount_db);
                    new_gain_2 = min(new_gain_2, gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_2);
                    if (gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2 != new_gain_2) {
                        /* the default gain is larger than the target and the current gain is not equal to the target, so we need to ramp down the volume on the game path */
                        stream_function_sw_gain_configure_gain_target(gaming_mode_vol_balance_handle.game_path_gain_port, 2, new_gain_2);
                        gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2 = new_gain_2;
                        gain_change = true;
                    }

                    /* set active flag */
                    gaming_mode_vol_balance_handle.active_flag = true;

                    /* reset failure_delay_us_count */
                    gaming_mode_vol_balance_handle.failure_delay_us_count = 0;
                }
                else if ((gaming_mode_vol_balance_handle.current_db >= gaming_mode_vol_balance_handle.effective_threshold_db) &&
                        (gaming_mode_vol_balance_handle.effective_delay_us_count < gaming_mode_vol_balance_handle.effective_delay_ms * 1000))
                {
                    /* there is chat voice on the chat path but time is not enough */
                    gaming_mode_vol_balance_handle.effective_delay_us_count += 2500;

                    /* reset failure_delay_us_count */
                    gaming_mode_vol_balance_handle.failure_delay_us_count = 0;
                }
                else if ((gaming_mode_vol_balance_handle.current_db <= gaming_mode_vol_balance_handle.failure_threshold_db) &&
                        (gaming_mode_vol_balance_handle.failure_delay_us_count >= gaming_mode_vol_balance_handle.failure_delay_ms * 1000))
                {
                    /* there is no chat voice on the chat path */
                    /* process L-channel */
                    if (gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_1 != gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1) {
                        /* recover gain to the default settings */
                        stream_function_sw_gain_configure_gain_target(gaming_mode_vol_balance_handle.game_path_gain_port, 1, gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_1);
                        gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1 = gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_1;
                        gain_change = true;
                    }

                    /* process R-channel */
                    if (gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_2 != gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2) {
                        /* recover gain to the default settings */
                        stream_function_sw_gain_configure_gain_target(gaming_mode_vol_balance_handle.game_path_gain_port, 2, gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_2);
                        gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2 = gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_2;
                        gain_change = true;
                    }

                    /* set active flag */
                    gaming_mode_vol_balance_handle.active_flag = false;

                    /* reset effective_delay_us_count */
                    gaming_mode_vol_balance_handle.effective_delay_us_count = 0;
                }
                else if ((gaming_mode_vol_balance_handle.current_db <= gaming_mode_vol_balance_handle.failure_threshold_db) &&
                        (gaming_mode_vol_balance_handle.failure_delay_us_count < gaming_mode_vol_balance_handle.failure_delay_ms * 1000))
                {
                    /* there is no chat voice on the chat path but time is not enough */
                    gaming_mode_vol_balance_handle.failure_delay_us_count += 2500;

                    /* reset effective_delay_us_count */
                    gaming_mode_vol_balance_handle.effective_delay_us_count = 0;

                    /* process L-channel */
                    if ((gaming_mode_vol_balance_handle.active_flag) && (gaming_mode_vol_balance_handle.chat_path_default_vol_level.gain_1 < gaming_mode_vol_balance_handle.effective_threshold_db)) {
                        /* In this stage(chat/game balance is actived, but chat spk volume is adjusted by end-user), we should recover gain to the default settings because the chat gain is too less */
                        new_gain_1 = gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_1;
                        if (gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1 != new_gain_1) {
                            stream_function_sw_gain_configure_gain_target(gaming_mode_vol_balance_handle.game_path_gain_port, 1, new_gain_1);
                            gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1 = new_gain_1;
                            gain_change = true;
                        }
                    } else if (gaming_mode_vol_balance_handle.active_flag) {
                        /* In this stage(chat/game balance is actived, but game spk volume is adjusted by end-user), we should set gain to the minimum in default gain and smart-target gain */
                        new_gain_1 = (gaming_mode_vol_balance_handle.chat_path_default_vol_level.gain_1 + gaming_mode_vol_balance_handle.adjustment_amount_db);
                        new_gain_1 = min(new_gain_1, gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_1);
                        if (gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1 != new_gain_1) {
                            /* set gain to the minimun settings */
                            stream_function_sw_gain_configure_gain_target(gaming_mode_vol_balance_handle.game_path_gain_port, 1, new_gain_1);
                            gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1 = new_gain_1;
                            gain_change = true;
                        }
                    } else {
                        /* In normal default vol and balance vol should be the same, this flow is designed for preventing errors */
                        if (gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_1 != gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1) {
                            /* recover gain to the default settings */
                            stream_function_sw_gain_configure_gain_target(gaming_mode_vol_balance_handle.game_path_gain_port, 1, gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_1);
                            gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1 = gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_1;
                            gain_change = true;
                        }
                    }

                    /* process R-channel */
                    if ((gaming_mode_vol_balance_handle.active_flag) && (gaming_mode_vol_balance_handle.chat_path_default_vol_level.gain_2 < gaming_mode_vol_balance_handle.effective_threshold_db)) {
                        /* In this stage(chat/game balance is actived, but chat spk volume is adjusted by end-user), we should recover gain to the default settings because the chat gain is too less */
                        new_gain_2 = gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_2;
                        if (gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2 != new_gain_2) {
                            stream_function_sw_gain_configure_gain_target(gaming_mode_vol_balance_handle.game_path_gain_port, 2, new_gain_2);
                            gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2 = new_gain_2;
                            gain_change = true;
                        }
                    } else if (gaming_mode_vol_balance_handle.active_flag) {
                        /* In this stage(chat/game balance is actived, but game spk volume is adjusted by end-user), we should set gain to the minimum in default gain and smart-target gain */
                        new_gain_2 = (gaming_mode_vol_balance_handle.chat_path_default_vol_level.gain_2 + gaming_mode_vol_balance_handle.adjustment_amount_db);
                        new_gain_2 = min(new_gain_2, gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_2);
                        if (gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2 != new_gain_2) {
                            /* recover gain to the minimun settings */
                            stream_function_sw_gain_configure_gain_target(gaming_mode_vol_balance_handle.game_path_gain_port, 2, new_gain_2);
                            gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2 = new_gain_2;
                            gain_change = true;
                        }
                    } else {
                        /* In normal default vol and balance vol should be the same, this flow is designed for preventing errors */
                        if (gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_2 != gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2) {
                            /* recover gain to the default settings */
                            stream_function_sw_gain_configure_gain_target(gaming_mode_vol_balance_handle.game_path_gain_port, 2, gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_2);
                            gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2 = gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_2;
                            gain_change = true;
                        }
                    }
                }
            }
            hal_nvic_restore_interrupt_mask(saved_mask);
        }
    }

    if (gain_change) {
        ULL_LOG_I("[Game/Chat balance][change gain]:%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 13,
                     gaming_mode_vol_balance_handle.active_flag,
                     gaming_mode_vol_balance_handle.current_db,
                     gaming_mode_vol_balance_handle.effective_threshold_db,
                     gaming_mode_vol_balance_handle.effective_delay_us_count,
                     gaming_mode_vol_balance_handle.failure_threshold_db,
                     gaming_mode_vol_balance_handle.failure_delay_us_count,
                     gaming_mode_vol_balance_handle.adjustment_amount_db,
                     gaming_mode_vol_balance_handle.chat_path_default_vol_level.gain_1,
                     gaming_mode_vol_balance_handle.chat_path_default_vol_level.gain_2,
                     gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_1,
                     gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_2,
                     gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1,
                     gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2);
    }
}
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */

/******************************************************************************/
/*                  Gaming mode dongle music path functions                   */
/******************************************************************************/
#ifdef AIR_SOFTWARE_MIXER_ENABLE
#ifdef AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE
ring_buffer_information_t g_line_out_buffer[2];
ATTR_TEXT_IN_IRAM_LEVEL_1 void gaming_mode_dongle_ul_mixer_postcallback(sw_mixer_member_t *member, void *para, uint32_t *out_frame_size)
{
    SOURCE source = (SOURCE)(member->owner);
    DSP_STREAMING_PARA_PTR stream = NULL;
    gaming_mode_dongle_ul_handle_t *c_handle = source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle;
    UNUSED(member);
    bool src_copy_flag = false; // copy data to g_line_out_buffer
    bool sink_drop_flag = false; // copy data from g_line_out_buffer to feature list buffer
    /* change this handle data status */
    uint32_t cnt = gaming_mode_dongle_first_ul_handle->total_number;
    if (cnt == 0) {
        ULL_LOG_E("[ULL][Line Out] ERROR: scenario type %d there is no ul stream!", 1, source->scenario_type);
        AUDIO_ASSERT(0);
    }
    if (!c_handle->play_en_status) { // only copy data when stream is started
        ULL_LOG_I("[ULL][Line Out] scenario type %d ul stream is not started", 1, source->scenario_type);
        return;
    }
    /* get the stream */
    stream = DSP_Streaming_Get(source, source->transform->sink);

    if ((c_handle->bypass_source) && (source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_DONGLE_LINE_OUT)) {
        /* bypass source drop for line out */
        stream->callback.EntryPara.in_size = 0;
        *out_frame_size = 160;
    }

    uint32_t channel_number = stream_function_get_channel_number(para);
    if (channel_number > 2) {
        ULL_LOG_E("[ULL][Line Out] ERROR: scenario type %d channel number is error, %d!", 2, source->scenario_type, channel_number);
        AUDIO_ASSERT(0);
    }
    uint32_t i;
    if (cnt == 1) { // sink path
        src_copy_flag = true;
        if (source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_DONGLE_LINE_OUT) {
            if (ring_buffer_get_space_byte_count(&g_line_out_buffer[0]) < (g_line_out_buffer[0].buffer_byte_count/2)) {
                /* line out buffer has enough data, bigger than 7ms */
                /* usb data's interval is 2.5ms */
                /* line out data's interval is 5ms */
                sink_drop_flag = true;
            } else {
                /* data is not enough, bypass sink */
                /* for bypass all subsequent sink operations */
                stream->callback.EntryPara.encoder_out_size = 0;
                stream->callback.EntryPara.src_out_size = 0;
                stream->callback.EntryPara.codec_out_size = 0;
            }
        } else if (source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_DONGLE_USB_OUT) {
            /* in order to avoid buffer full, so drop one frame(240 bytes)*/
            if (ring_buffer_get_space_byte_count(&g_line_out_buffer[0]) < 80) { // buffer full
                for (uint32_t j = 0; j < channel_number; j ++) {
                    ring_buffer_read_done(&g_line_out_buffer[j], *out_frame_size);
                }
            }
        }
    } else if (cnt >= 2) {
        if (source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_DONGLE_LINE_OUT) {
            sink_drop_flag = true;
        } else if (source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_DONGLE_USB_OUT) {
            src_copy_flag = true;
        }
    } else {
        ULL_LOG_E("[ULL][Line Out] ERROR: scenario type %d, abnormal case", 1, source->scenario_type);
        AUDIO_ASSERT(0);
    }
    /* data handle */
    for (i = 0; i < channel_number; i++) {
        uint8_t *stream_buf = stream_function_get_inout_buffer(para, i + 1);
        if (src_copy_flag) {
            /* copy data to g_line_out_buffer */
            uint8_t *output_buf = NULL;
            uint32_t output_buf_size = 0;
            uint32_t copy_size = 0;
            ring_buffer_get_write_information(&g_line_out_buffer[i], &output_buf, &output_buf_size);
            if (!output_buf) {
                AUDIO_ASSERT(0);
            }
            if (output_buf_size < (*out_frame_size)) {
                ULL_LOG_E("[ULL][Line Out] source copy scenario type %d line out buffer[%d] is not enough this time, %d %d", 4,
                                source->scenario_type,
                                i,
                                output_buf_size,
                                *out_frame_size);
            }
            copy_size = MIN(output_buf_size, *out_frame_size);
            memcpy(output_buf, stream_buf, copy_size);
            ring_buffer_write_done(&g_line_out_buffer[i], copy_size);
        }
        if (sink_drop_flag) {
            /* drop data from g_line_out_buffer to feature list buffer */
            uint8_t *input_buf = NULL;
            uint32_t input_buf_size = 0;
            uint32_t copy_size = 0;
            ring_buffer_get_read_information(&g_line_out_buffer[i], &input_buf, &input_buf_size);
            if (!input_buf) {
                AUDIO_ASSERT(0);
            }
            if (input_buf_size < (*out_frame_size)) {
                ULL_LOG_E("[ULL][Line Out] sink copy scenraio type %d line out buffer[%d] is not enough this time, %d %d", 4,
                                source->scenario_type,
                                i,
                                input_buf_size,
                                *out_frame_size);
            }
            copy_size = MIN(input_buf_size, *out_frame_size);
            memcpy(stream_buf, input_buf, copy_size);
            ring_buffer_read_done(&g_line_out_buffer[i], copy_size);
            //LOG_AUDIO_DUMP(input_buf, copy_size, PROMPT_RT_PATTERN);
        }
    }
    //if (source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_DONGLE_LINE_OUT) {
        if (src_copy_flag) {
            ULL_LOG_I("[ULL][Line Out] source copy: scenario type %d buffer0 wo[%d] ro[%d], buffer1 wo[%d] ro[%d]", 5,
                    source->scenario_type,
                    g_line_out_buffer[0].write_pointer,
                    g_line_out_buffer[0].read_pointer,
                    g_line_out_buffer[1].write_pointer,
                    g_line_out_buffer[1].read_pointer);
        }
        if (sink_drop_flag) {
            ULL_LOG_I("[ULL][Line Out] sink drop: scenario type %d buffer0 wo[%d] ro[%d], buffer1 wo[%d] ro[%d]", 5,
                    source->scenario_type,
                    g_line_out_buffer[0].write_pointer,
                    g_line_out_buffer[0].read_pointer,
                    g_line_out_buffer[1].write_pointer,
                    g_line_out_buffer[1].read_pointer);
        }
    //}
}
#endif /* AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE */
ATTR_TEXT_IN_IRAM_LEVEL_1 void gaming_mode_dongle_mixer_postcallback(sw_mixer_member_t *member, void *para, uint32_t *out_frame_size)
{
    int32_t i;
    gaming_mode_dongle_dl_handle_t *c_handle = NULL;
    SOURCE source = NULL;
    bool all_streams_in_mixer = true;

    UNUSED(member);
    UNUSED(para);

    /* change this handle data status */
    source = (SOURCE)(member->owner);
    if ((source->scenario_type != AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_LINE_IN) &&
        (source->scenario_type != AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_I2S_IN)) {
        c_handle = (gaming_mode_dongle_dl_handle_t *)(source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.handle);
        c_handle->data_status = GAMING_MODE_STREAM_DATA_IN_MIXER;
    }
    else {
        c_handle = gaming_mode_dongle_query_dl_handle(source);
        c_handle->data_status = GAMING_MODE_STREAM_DATA_IN_MIXER;
    }

    /* check hanlde's mixer status and index */
    if (c_handle->mixer_status == GAMING_MODE_STREAM_MIXER_UNMIX) {
        /* clear this stream's input buffer */
        stream_function_sw_mixer_member_input_buffers_clean(c_handle->mixer_member, false);

        /* change this handle data status */
        c_handle->data_status = GAMING_MODE_STREAM_DATA_EMPTY;

        /* this stream is unmixed now, so we can directly return here */
        return;
    } else if (source->param.data_dl.current_notification_index == 0) {
        /* clear this stream's input buffer */
        stream_function_sw_mixer_member_input_buffers_clean(c_handle->mixer_member, false);

        /* change this handle data status */
        c_handle->data_status = GAMING_MODE_STREAM_DATA_EMPTY;

        /* in here, it means it is the first packet, we need to drop the later sink flow */
        *out_frame_size = 0;

        return;
    }

    /* check if all mixed stream data is in this mixer */
    c_handle = gaming_mode_dongle_first_dl_handle;
    for (i = 0; i < gaming_mode_dongle_first_dl_handle->total_number; i++) {
        source = (SOURCE)(c_handle->owner);
        if ((c_handle->data_status == GAMING_MODE_STREAM_DATA_IN_STREAM) &&
            (c_handle->mixer_status != GAMING_MODE_STREAM_MIXER_UNMIX) &&
            (source->param.data_dl.current_notification_index != 0)) {
            /* this stream should be mixed, but its data is not in this mixer now.
               so we need to bypass this mix result in this time */
            all_streams_in_mixer = false;
            break;
        }

        /* switch to the next dl stream */
        c_handle = c_handle->next_dl_handle;
    }

    /* check if the output data in this time includes all stream data */
    if ((*out_frame_size != 0) && (all_streams_in_mixer == true)) {
        /* all_streams_in_mixer is true, so all stream data is in this mixer.
           So we can clean all mixed streams' input buffers now and the mix result are sent to the sink */
        c_handle = gaming_mode_dongle_first_dl_handle;
        for (i = 0; i < gaming_mode_dongle_first_dl_handle->total_number; i++) {
            if (c_handle->mixer_status != GAMING_MODE_STREAM_MIXER_UNMIX) {
                /* clear this stream's input buffer */
                stream_function_sw_mixer_member_input_buffers_clean(c_handle->mixer_member, false);

                /* change this handle data status */
                c_handle->data_status = GAMING_MODE_STREAM_DATA_EMPTY;
            }

            /* switch to the next dl stream */
            c_handle = c_handle->next_dl_handle;
        }
    } else {
        /* all_streams_in_mixer is false, so some stream data is not in this mixer.
           So we need to bypass this mix result */
        *out_frame_size = 0;
    }
}
#endif /* AIR_SOFTWARE_MIXER_ENABLE */

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ gaming_mode_dongle_dl_handle_t *gaming_mode_dongle_get_dl_handle(void *owner)
{
    uint32_t saved_mask;
    int16_t count;
    gaming_mode_dongle_dl_handle_t *dongle_handle = NULL;
    gaming_mode_dongle_dl_handle_t *c_handle = NULL;

    dongle_handle = malloc(sizeof(gaming_mode_dongle_dl_handle_t));
    if (dongle_handle == NULL) {
        AUDIO_ASSERT(0);
    }
    memset(dongle_handle, 0, sizeof(gaming_mode_dongle_dl_handle_t));

    dongle_handle->total_number = -1;
    dongle_handle->index = -1;
    dongle_handle->owner = owner;
    dongle_handle->next_dl_handle = NULL;

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    if (gaming_mode_dongle_first_dl_handle == NULL) {
        /* there is no item on the handle list */
        dongle_handle->total_number = 1;
        dongle_handle->index = 1;

        /* set this handle as the head of the handle list */
        gaming_mode_dongle_first_dl_handle = dongle_handle;
    } else {
        /* there are other items on the handle list */
        count = 1;
        c_handle = gaming_mode_dongle_first_dl_handle;
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

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void gaming_mode_dongle_release_dl_handle(gaming_mode_dongle_dl_handle_t *handle)
{
    uint32_t saved_mask, i, count;
    gaming_mode_dongle_dl_handle_t *l_handle = NULL;
    gaming_mode_dongle_dl_handle_t *c_handle = NULL;
    gaming_mode_dongle_dl_handle_t *dongle_handle = NULL;

    if ((handle == NULL) || (gaming_mode_dongle_first_dl_handle == NULL)) {
        AUDIO_ASSERT(0);
    }

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    if (gaming_mode_dongle_first_dl_handle->total_number <= 0) {
        /* error status */
        AUDIO_ASSERT(0);
    } else if ((gaming_mode_dongle_first_dl_handle->total_number == 1) &&
               (gaming_mode_dongle_first_dl_handle == handle)) {
        /* this handle is only one item on handle list */
        if (gaming_mode_dongle_first_dl_handle->next_dl_handle != NULL) {
            AUDIO_ASSERT(0);
        }

        /* there is no item on the handle list */
        gaming_mode_dongle_first_dl_handle = NULL;
    } else if ((gaming_mode_dongle_first_dl_handle->total_number > 1) &&
               (gaming_mode_dongle_first_dl_handle == handle)) {
        /* this handle is the first item on handle list, but there are other handles on the list */
        c_handle = gaming_mode_dongle_first_dl_handle;
        count = gaming_mode_dongle_first_dl_handle->total_number;
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
        gaming_mode_dongle_first_dl_handle = handle->next_dl_handle;
    } else {
        /* this handle is the middle item on handle list */
        c_handle = gaming_mode_dongle_first_dl_handle;
        count = gaming_mode_dongle_first_dl_handle->total_number;
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

ATTR_TEXT_IN_IRAM_LEVEL_1 gaming_mode_dongle_dl_handle_t *gaming_mode_dongle_query_dl_handle(void *owner)
{
    uint32_t saved_mask, i, count;
    gaming_mode_dongle_dl_handle_t *dongle_handle = NULL;

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    dongle_handle = gaming_mode_dongle_first_dl_handle;
    if (dongle_handle != NULL)
    {
        count = dongle_handle->total_number;
        for (i=0; i < count; i++)
        {
            /* check owner */
            if (dongle_handle->owner == owner)
            {
                break;
            }
            else
            {
                dongle_handle = dongle_handle->next_dl_handle;
            }
        }
    }

    hal_nvic_restore_interrupt_mask(saved_mask);

    return dongle_handle;
}

void gaming_mode_dongle_dl_init(SOURCE source, audio_transmitter_open_param_p open_param)
{
    usb_in_parameter *usb_in_param;
    gaming_mode_dongle_dl_handle_t *dongle_handle;

    /* register ccni handler */
    audio_transmitter_register_isr_handler(0, gaming_mode_dongle_dl_ccni_handler);

    /* get handle for application config */
    dongle_handle = gaming_mode_dongle_get_dl_handle(source);
    source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.handle = (void *)dongle_handle;
    usb_in_param = &source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param;
    usb_in_param->codec_param.pcm.sample_rate = open_param->scenario_param.gaming_mode_param.codec_param.pcm.sample_rate;
    usb_in_param->codec_param.pcm.format = open_param->scenario_param.gaming_mode_param.codec_param.pcm.format;
    UNUSED(usb_in_param);

#if defined(AIR_FIXED_RATIO_SRC)
    src_fixed_ratio_config_t fix_src_config = {0};

    fix_src_config.channel_number = 2;
    fix_src_config.in_sampling_rate = usb_in_param->codec_param.pcm.sample_rate;
    fix_src_config.out_sampling_rate = 48000;
    fix_src_config.resolution = RESOLUTION_16BIT;
    fix_src_config.multi_cvt_mode = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_SINGLE;
    fix_src_config.cvt_num = 1;
    fix_src_config.with_codec = false;
    dongle_handle->src_fix_port = stream_function_src_fixed_ratio_get_port(source);
    ULL_LOG_I("[audio transmitter][config]scenario %d-%d, src_fixed_ratio init: channel_number %d, in_sampling_rate %d, out_sampling_rate %d, resolution %d", 6,
                                open_param->scenario_type,
                                open_param->scenario_sub_id,
                                fix_src_config.channel_number,
                                fix_src_config.in_sampling_rate,
                                fix_src_config.out_sampling_rate,
                                fix_src_config.resolution);
    stream_function_src_fixed_ratio_init(dongle_handle->src_fix_port, &fix_src_config);
#endif

#ifdef AIR_SOFTWARE_GAIN_ENABLE
    /* sw gain config */
    int32_t default_gain;
    sw_gain_config_t default_config;
    default_config.resolution = RESOLUTION_16BIT;
    default_config.target_gain = open_param->scenario_param.gaming_mode_param.gain_default_L;
    default_config.up_step = 1;
    default_config.up_samples_per_step = 2;
    default_config.down_step = -1;
    default_config.down_samples_per_step = 2;
    dongle_handle->gain_port = stream_function_sw_gain_get_port(source);
    stream_function_sw_gain_init(dongle_handle->gain_port, 2, &default_config);
    default_gain = open_param->scenario_param.gaming_mode_param.gain_default_L;
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 1, default_gain);
    ULL_LOG_I("[audio transmitter][config]scenario %d-%d change channel %d gain to %d*0.01dB\r\n", 4,
                 open_param->scenario_type,
                 open_param->scenario_sub_id,
                 1,
                 default_gain);
    default_gain = open_param->scenario_param.gaming_mode_param.gain_default_R;
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2, default_gain);
    ULL_LOG_I("[audio transmitter][config]scenario %d-%d change channel %d gain to %d*0.01dB\r\n", 4,
                 open_param->scenario_type,
                 open_param->scenario_sub_id,
                 2,
                 default_gain);
#endif /* AIR_SOFTWARE_GAIN_ENABLE */

#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
    if (open_param->scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0) {
        gaming_mode_vol_balance_handle.chat_path_default_vol_level.gain_1 = open_param->scenario_param.gaming_mode_param.gain_default_L;
        gaming_mode_vol_balance_handle.chat_path_default_vol_level.gain_2 = open_param->scenario_param.gaming_mode_param.gain_default_R;
        gaming_mode_vol_balance_handle.chat_dongle_handle = dongle_handle;
        DTM_enqueue(DTM_EVENT_ID_GAMING_MODE_VOLUME_SMART_BALANCE_ENABLE, AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_USB_IN_0, false);
    } else {
        gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_1 = open_param->scenario_param.gaming_mode_param.gain_default_L;
        gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_2 = open_param->scenario_param.gaming_mode_param.gain_default_R;
        gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1 = open_param->scenario_param.gaming_mode_param.gain_default_L;
        gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2 = open_param->scenario_param.gaming_mode_param.gain_default_R;
        gaming_mode_vol_balance_handle.game_path_gain_port = dongle_handle->gain_port;
    }
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */

#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    sw_clk_skew_config_t sw_clk_skew_config;
    /* clock skew config */
    source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.clk_skew.last_timestamp = 0;
    source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.clk_skew.act_time = 0;
    source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.clk_skew.act_count = 0;
    source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.clk_skew.act_diff = 0;
    source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.clk_skew.ave_diff = 0;
    source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.clk_skew.compen_samples = 0;
    dongle_handle->clk_skew_port = stream_function_sw_clk_skew_get_port(source);
    sw_clk_skew_config.channel = 2;
    sw_clk_skew_config.bits = 16;
    sw_clk_skew_config.order = C_Flp_Ord_5;
    sw_clk_skew_config.skew_io_mode = C_Skew_Inp;
    sw_clk_skew_config.skew_compensation_mode = SW_CLK_SKEW_COMPENSATION_1_SAMPLE_IN_1_FRAME;
    sw_clk_skew_config.skew_work_mode = SW_CLK_SKEW_CONTINUOUS;
    sw_clk_skew_config.max_output_size = 480;
    sw_clk_skew_config.continuous_frame_size = 120 * sizeof(int16_t);
    stream_function_sw_clk_skew_init(dongle_handle->clk_skew_port, &sw_clk_skew_config);
#endif /* AIR_SOFTWARE_CLK_SKEW_ENABLE */

#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    sw_buffer_config_t buffer_config;
    buffer_config.mode = SW_BUFFER_MODE_FIXED_OUTPUT_LENGTH;
    buffer_config.total_channels = 2;
    buffer_config.watermark_max_size = 10 * 48 * sizeof(int16_t); /* must be 2.5ms*N */
    buffer_config.watermark_min_size = 0;
    buffer_config.output_size = 120 * sizeof(int16_t);
    dongle_handle->buffer_port = stream_function_sw_buffer_get_port(source);
    stream_function_sw_buffer_init(dongle_handle->buffer_port, &buffer_config);
#endif /* AIR_SOFTWARE_BUFFER_ENABLE */

#ifdef AIR_SOFTWARE_MIXER_ENABLE
    /* sw mixer config */
    sw_mixer_callback_config_t       callback_config;
    sw_mixer_input_channel_config_t  in_ch_config;
    sw_mixer_output_channel_config_t out_ch_config;

    stream_function_sw_mixer_init(SW_MIXER_PORT_0);

    callback_config.preprocess_callback = NULL;
    extern void gaming_mode_dongle_mixer_postcallback(sw_mixer_member_t *member, void *para, uint32_t *out_frame_size);
    callback_config.postprocess_callback = gaming_mode_dongle_mixer_postcallback;
    in_ch_config.total_channel_number = 2;
    in_ch_config.resolution = RESOLUTION_16BIT;
    in_ch_config.input_mode = SW_MIXER_CHANNEL_MODE_FORCED_ENOUGH;
    in_ch_config.buffer_size = sizeof(int16_t) * 120; // 2.5ms * 48K
    out_ch_config.total_channel_number = 2;
    out_ch_config.resolution = RESOLUTION_16BIT;
    dongle_handle->mixer_member = stream_function_sw_mixer_member_create((void *)source,
                                                                         SW_MIXER_MEMBER_MODE_NO_BYPASS,
                                                                         &callback_config,
                                                                         &in_ch_config,
                                                                         &out_ch_config);

    if (dongle_handle->index == 1) {
        /* it is the first dl stream */
        stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, dongle_handle->mixer_member, true);
    } else {
        /* it is not the first dl stream, so we need to disconnect default connection */
        stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, dongle_handle->mixer_member, false);
    }
#endif /* AIR_SOFTWARE_MIXER_ENABLE */

#ifdef HAL_SLEEP_MANAGER_ENABLED
    /* lock sleep because sleep wake-up time will consume the stream process time */
    hal_sleep_manager_lock_sleep(SLEEP_LOCK_DONGLE_AUDIO);
#endif
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void gaming_mode_dongle_dl_deinit(SOURCE source, SINK sink)
{
    //uint32_t saved_mask;
    gaming_mode_dongle_dl_handle_t *dongle_handle = (gaming_mode_dongle_dl_handle_t *)(source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.handle);
    UNUSED(sink);

    /* application deinit */
    //hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    /* unregister ccni handler */
    if (dongle_handle->total_number == 1) {
        audio_transmitter_unregister_isr_handler(0, gaming_mode_dongle_dl_ccni_handler);
    }

    /* restore cap id to the default value */
    if ((dongle_handle->index == 1) && (dongle_handle->cap_id_default != 0)) {
        DCXO_CAPID_EFUSE_REG.DCXO_CAPID_EFUSE_SEL = 0x1;
        DCXO_CAPID_EFUSE_REG.DCXO_CAPID_EFUSE     = dongle_handle->cap_id_default;
    }

#if defined(AIR_FIXED_RATIO_SRC)
    stream_function_src_fixed_ratio_deinit(dongle_handle->src_fix_port);
#endif /* AIR_FIXED_RATIO_SRC */

#ifdef AIR_SOFTWARE_GAIN_ENABLE
    stream_function_sw_gain_deinit(dongle_handle->gain_port);
#endif /* AIR_SOFTWARE_GAIN_ENABLE */

#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    stream_function_sw_clk_skew_deinit(dongle_handle->clk_skew_port);
#endif /* AIR_SOFTWARE_CLK_SKEW_ENABLE */

#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    stream_function_sw_buffer_deinit(dongle_handle->buffer_port);
#endif /* AIR_SOFTWARE_BUFFER_ENABLE */

#ifdef AIR_SOFTWARE_MIXER_ENABLE
    stream_function_sw_mixer_member_unregister(SW_MIXER_PORT_0, dongle_handle->mixer_member);
    stream_function_sw_mixer_member_delete(dongle_handle->mixer_member);
    stream_function_sw_mixer_deinit(SW_MIXER_PORT_0);
#endif /* AIR_SOFTWARE_MIXER_ENABLE */

#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
    if (source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_1) {
        gaming_mode_vol_balance_handle.game_path_gain_port = NULL;
    }
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */

    /* release handle */
    gaming_mode_dongle_release_dl_handle(dongle_handle);
    source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.handle = NULL;

    //hal_nvic_restore_interrupt_mask(saved_mask);

#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
    if (source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0) {
        DTM_enqueue(DTM_EVENT_ID_GAMING_MODE_VOLUME_SMART_BALANCE_DISABLE, AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_USB_IN_0, false);
        gaming_mode_vol_balance_handle.chat_dongle_handle = NULL;
    }
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */

#ifdef HAL_SLEEP_MANAGER_ENABLED
    /* unlock sleep */
    hal_sleep_manager_unlock_sleep(SLEEP_LOCK_DONGLE_AUDIO);
#endif
}

ATTR_TEXT_IN_IRAM_LEVEL_1 void gaming_mode_dongle_dl_ccni_handler(hal_ccni_event_t event, void *msg)
{
    SOURCE source = NULL;
    uint32_t saved_mask = 0;
    uint32_t i, count;
    hal_ccni_message_t *ccni_msg = msg;
    gaming_mode_dongle_dl_handle_t *c_handle = NULL;
    #if (defined AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE)
    BUFFER_INFO *buffer_info = NULL;
    AUDIO_PARAMETER *runtime = NULL;
    uint32_t data_size;
    uint32_t buffer_per_channel_shift;
    #endif /* AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE || AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE */
    UNUSED(event);

    if (gaming_mode_dongle_first_dl_handle == NULL) {
        ULL_LOG_E("[audio transmitter][source]dl handle is NULL\r\n", 0);
        goto _ccni_return;
    }

    /* timestamp for debug */
    gaming_mode_music_ccni_bt_clk = ccni_msg->ccni_message[0];
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&gaming_mode_music_ccni_gpt_count);

    c_handle = gaming_mode_dongle_first_dl_handle;
    count = gaming_mode_dongle_first_dl_handle->total_number;
    for (i = 0; i < count; i++) {
        /* get source */
        source = (SOURCE)c_handle->owner;
        if ((source == NULL) || (source->transform == NULL)) {
            ULL_LOG_E("[audio transmitter][source]source or transform is NULL\r\n", 0);
            /* switch to the next dl stream */
            c_handle = c_handle->next_dl_handle;
            continue;
        }

        /* wakeup stream */
        if (source->param.data_dl.scenario_type == AUDIO_TRANSMITTER_A2DP_SOURCE) {

        }
#if defined(AIR_GAMING_MODE_DONGLE_ENABLE)
        else if (source->param.data_dl.scenario_type == AUDIO_TRANSMITTER_GAMING_MODE) {
            /* Dongle side, music path, usb in case */
            if ((source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0) ||
                (source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_1)) {
#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG
                uint32_t current_timestamp = 0;
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
                ULL_LOG_I("[audio transmitter][source][ccni callback entry]: %u, %d", 2, current_timestamp, hal_nvic_query_exception_number());
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG */

                hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                /* set fetch flag to indicate that the next flow can fetch packets form buffer */
                if (source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.fetch_flag == 0) {
                    source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.fetch_flag = 1;
                }
                /* send bt ccni clock stemp to bt common */
                if (source->transform->sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_status == 0) {
                    source->transform->sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_timestamp = ccni_msg->ccni_message[0];
                }
                hal_nvic_restore_interrupt_mask(saved_mask);

                /* Handler the stream */
                AudioCheckTransformHandle(source->transform);

#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LANTENCY
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&gaming_mode_music_ccni_gpt_count2[i]);
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LANTENCY */

#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
                ULL_LOG_I("[audio transmitter][source][ccni callback exit]: %u, %d", 2, current_timestamp, hal_nvic_query_exception_number());
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG */
            }
        }
#if defined AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE
        else if ((source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_LINE_IN) || (source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_I2S_IN)) {
            #if GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG
            uint32_t current_timestamp = 0;
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
            DSP_MW_LOG_I("[audio transmitter][source][ccni callback entry]: [%d-%d] %u, %d", 4,
                source->param.audio.scenario_id,
                source->param.audio.scenario_sub_id,
                current_timestamp,
                hal_nvic_query_exception_number());
            #endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG */

            // hal_nvic_save_and_set_interrupt_mask(&saved_mask);

            /* update data status */
            c_handle->data_status = GAMING_MODE_STREAM_DATA_EMPTY;
            if (c_handle->stream_status != GAMING_MODE_STREAM_STARTED) {
                /* stream is not started */
                // DSP_MW_LOG_E("[audio transmitter][source]stream is not started\r\n", 0);
                /* switch to the next dl stream */
                c_handle = c_handle->next_dl_handle;
                continue;
            } else {
                /* stream is started */
                hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                /* send bt ccni clock stemp to bt common */
                if (source->transform->sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_status == 0) {
                    source->transform->sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_timestamp = ccni_msg->ccni_message[0];
                }
                hal_nvic_restore_interrupt_mask(saved_mask);
                buffer_info = &source->streamBuffer.BufferInfo;
                runtime = &source->param.audio;
                /* update read & write pointer */
                c_handle->pre_write_offset  = buffer_info->WriteOffset;
                c_handle->pre_read_offset   = buffer_info->ReadOffset;
                c_handle->afe_vul_cur       = AFE_GET_REG(c_handle->afe_vul_cur_addr);
                c_handle->afe_vul_base      = AFE_GET_REG(c_handle->afe_vul_base_addr);
                c_handle->cur_write_offset  = c_handle->afe_vul_cur - c_handle->afe_vul_base;
                buffer_info->WriteOffset = c_handle->cur_write_offset;
                /* check if the samples are enough */
                buffer_per_channel_shift = ((source->param.audio.channel_num>=2) && (source->buftype == BUFFER_TYPE_INTERLEAVED_BUFFER )) ? 1 : 0;
                data_size = (c_handle->cur_write_offset >= c_handle->pre_read_offset)
                            ? (c_handle->cur_write_offset - c_handle->pre_read_offset)>>buffer_per_channel_shift
                            : (source->streamBuffer.BufferInfo.length - c_handle->pre_read_offset + c_handle->cur_write_offset)>>buffer_per_channel_shift;
                if (runtime->irq_exist == false) {
                    /* stream is played till the samples in AFE buffer is enough */
                    if (data_size >= (source->param.audio.frame_size + GAMING_MODE_AFE_LINEIN_PLAYEN_DELAY)) {
                        if (source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_I2S_IN) {
                            /* tracking mode should buffer some data to avoid irq period jitter */
                            #ifndef AIR_I2S_SLAVE_ENABLE
                            buffer_info->ReadOffset += ((data_size - (source->param.audio.frame_size + GAMING_MODE_AFE_LINEIN_PLAYEN_DELAY)) << buffer_per_channel_shift);
                            buffer_info->ReadOffset = (source->streamBuffer.BufferInfo.ReadOffset % (source->streamBuffer.BufferInfo.length));
                            c_handle->pre_read_offset = buffer_info->ReadOffset;
                            #endif
                        } else {
                            /* LINE IN only need the latest 2.5ms data */
                            buffer_info->ReadOffset += ((data_size - (source->param.audio.frame_size + GAMING_MODE_AFE_LINEIN_PLAYEN_DELAY)) << buffer_per_channel_shift);
                            buffer_info->ReadOffset = (source->streamBuffer.BufferInfo.ReadOffset % (source->streamBuffer.BufferInfo.length));
                            c_handle->pre_read_offset = buffer_info->ReadOffset;
                        }

                        /* samples are enough, so update data status */
                        c_handle->data_status = GAMING_MODE_STREAM_DATA_IN_STREAM;

                        /* update flag */
                        runtime->irq_exist = true;

                        /* Handler the stream */
                        AudioCheckTransformHandle(source->transform);
                    }
                } else {
                    if (data_size >= source->param.audio.frame_size) {
                        /* samples are enough, so update data status */
                        c_handle->data_status = GAMING_MODE_STREAM_DATA_IN_STREAM;
                    }

                    /* Handler the stream */
                    AudioCheckTransformHandle(source->transform);
                }
                #ifdef AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE
                if (!g_vdma_start_flag) {
                    /* Start I2S DMA */
                    g_vdma_start_flag = true;
                    vdma_channel_t dma_channel;
                    vdma_status_t i2s_vdma_status;
                    uint32_t port_tmp = 0;
                    extern vdma_channel_t g_i2s_slave_vdma_channel_infra[];
                    if (source->param.audio.audio_interface == HAL_AUDIO_INTERFACE_1) {
                        port_tmp = 0;
                    } else if (source->param.audio.audio_interface == HAL_AUDIO_INTERFACE_2) {
                        port_tmp = 1;
                    } else if (source->param.audio.audio_interface == HAL_AUDIO_INTERFACE_3) {
                        port_tmp = 2;
                    } else {
                        DSP_MW_LOG_I(" [ULL][I2S IN] start i2s slave audio_interface = %d fail", 1, source->param.audio.audio_interface);
                        configASSERT(0);
                    }
                    dma_channel     = g_i2s_slave_vdma_channel_infra[port_tmp * 2 + 1];
                    buffer_info->ReadOffset = buffer_info->WriteOffset; // clear the buffer
                    AFE_WRITE(ASM_CH01_OBUF_RDPNT, AFE_GET_REG(ASM_CH01_OBUF_WRPNT));
                    i2s_vdma_status = vdma_start(dma_channel);
                    if (i2s_vdma_status != VDMA_OK) {
                        DSP_MW_LOG_I(" [ULL][I2S IN] DSP UL start i2s slave set vdma_start fail %d", 1, i2s_vdma_status);
                        configASSERT(0);
                    }
                    DSP_MW_LOG_I("[ULL][I2S IN] VDMA Start", 0);
                }
                #endif /* AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE */
            }
            #if GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
            DSP_MW_LOG_I("[audio transmitter][source][ccni callback exit]: %u, %d", 2, current_timestamp, hal_nvic_query_exception_number());
            #endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG */
        }
#endif /* AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE||AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE */
#endif /* AIR_GAMING_MODE_DONGLE_ENABLE */
        else {
            AUDIO_ASSERT(0);
        }

        /* switch to the next dl stream */
        c_handle = c_handle->next_dl_handle;
    }
    if (c_handle != NULL) {
        AUDIO_ASSERT(0);
    }

_ccni_return:
#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LANTENCY
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&gaming_mode_music_ccni_gpt_count1);
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LANTENCY */

    return;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ bool gaming_mode_dongle_dl_config(SOURCE source, stream_config_type type, U32 value)
{
    uint32_t saved_mask = 0;
    extern CONNECTION_IF *port_audio_transmitter_get_connection_if(audio_transmitter_scenario_type_t scenario_id, audio_transmitter_scenario_sub_id_t sub_id);
    gaming_mode_runtime_config_param_p config_param = (gaming_mode_runtime_config_param_p)value;
    gaming_mode_dongle_dl_handle_t *dongle_handle = NULL;
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    int32_t new_gain;
    int32_t new_gain2;
    sw_gain_config_t old_config;
#endif /* AIR_SOFTWARE_GAIN_ENABLE */
#ifdef AIR_SOFTWARE_MIXER_ENABLE
    audio_transmitter_scenario_type_t scenario_id_0;
    audio_transmitter_scenario_sub_id_t sub_id_0;
    audio_transmitter_scenario_type_t scenario_id_1;
    audio_transmitter_scenario_sub_id_t sub_id_1;
    CONNECTION_IF *application_ptr_0 = NULL;
    CONNECTION_IF *application_ptr_1 = NULL;
    gaming_mode_dongle_dl_handle_t *dongle_handle_0;
    gaming_mode_dongle_dl_handle_t *dongle_handle_1;
#endif /* AIR_SOFTWARE_MIXER_ENABLE */
    UNUSED(dongle_handle);
    UNUSED(saved_mask);
    UNUSED(type);
    UNUSED(value);

    if ((source->scenario_type != AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_LINE_IN) &&
        (source->scenario_type != AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_I2S_IN)) {
        dongle_handle = (gaming_mode_dongle_dl_handle_t *)(source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.handle);
    } else {
        dongle_handle = gaming_mode_dongle_query_dl_handle(source);
    }

    switch (config_param->config_operation) {
        case GAMING_MODE_CONFIG_OP_VOL_LEVEL_MUSIC_L:
#ifdef AIR_SOFTWARE_GAIN_ENABLE
            /* left channel */
            new_gain = config_param->config_param.vol_level.gain_1;
            stream_function_sw_gain_get_config(dongle_handle->gain_port, 1, &old_config);
            ULL_LOG_I("[audio transmitter][config]scenario %d-%d change channel %d gain from %d*0.01dB to %d*0.01dB\r\n", 5,
                         source->param.data_dl.scenario_type,
                         source->param.data_dl.scenario_sub_id,
                         1,
                         old_config.target_gain,
                         new_gain);

#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            if (source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0) {
                gaming_mode_vol_balance_handle.chat_path_default_vol_level.gain_1 = config_param->config_param.vol_level.gain_1;
            } else {
                gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_1 = config_param->config_param.vol_level.gain_1;
                if ((gaming_mode_vol_balance_handle.enable) && (gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1 < config_param->config_param.vol_level.gain_1)) {
                    new_gain = gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1;
                } else if ((gaming_mode_vol_balance_handle.enable) && (gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1 >= config_param->config_param.vol_level.gain_1)) {
                    gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1 = config_param->config_param.vol_level.gain_1;
                } else {
                    gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1 = config_param->config_param.vol_level.gain_1;
                }
            }
            hal_nvic_restore_interrupt_mask(saved_mask);
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */

            stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 1, new_gain);
#endif /* AIR_SOFTWARE_GAIN_ENABLE */
            break;

        case GAMING_MODE_CONFIG_OP_VOL_LEVEL_MUSIC_R:
#ifdef AIR_SOFTWARE_GAIN_ENABLE
            new_gain = config_param->config_param.vol_level.gain_2;
            /* right channel */
            stream_function_sw_gain_get_config(dongle_handle->gain_port, 2, &old_config);
            ULL_LOG_I("[audio transmitter][config]scenario %d change channel %d gain from %d*0.01dB to %d*0.01dB\r\n", 4,
                         source->scenario_type,
                         2,
                         old_config.target_gain,
                         new_gain);

#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            if (source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0) {
                gaming_mode_vol_balance_handle.chat_path_default_vol_level.gain_2 = config_param->config_param.vol_level.gain_2;
            } else {
                gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_2 = config_param->config_param.vol_level.gain_2;
                if ((gaming_mode_vol_balance_handle.enable) && (gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2 < config_param->config_param.vol_level.gain_2)) {
                    new_gain = gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2;
                } else if ((gaming_mode_vol_balance_handle.enable) && (gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2 >= config_param->config_param.vol_level.gain_2)) {
                    gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2 = config_param->config_param.vol_level.gain_2;
                } else {
                    gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2 = config_param->config_param.vol_level.gain_2;
                }
            }
            hal_nvic_restore_interrupt_mask(saved_mask);
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */

            stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2, new_gain);
#endif /* AIR_SOFTWARE_GAIN_ENABLE */
            break;

        case GAMING_MODE_CONFIG_OP_VOL_LEVEL_MUSIC_DUL:
#ifdef AIR_SOFTWARE_GAIN_ENABLE
            /* left channel */
            new_gain = config_param->config_param.vol_level.gain_1;
            stream_function_sw_gain_get_config(dongle_handle->gain_port, 1, &old_config);
            ULL_LOG_I("[audio transmitter][config]scenario %d change channel %d gain from %d*0.01dB to %d*0.01dB\r\n", 4,
                         source->scenario_type,
                         1,
                         old_config.target_gain,
                         new_gain);

            /* right channel */
            stream_function_sw_gain_get_config(dongle_handle->gain_port, 2, &old_config);
            new_gain2 = config_param->config_param.vol_level.gain_2;
            ULL_LOG_I("[audio transmitter][config]scenario %d change channel %d gain from %d*0.01dB to %d*0.01dB\r\n", 4,
                         source->scenario_type,
                         2,
                         old_config.target_gain,
                         new_gain2);

#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            if (source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0) {
                gaming_mode_vol_balance_handle.chat_path_default_vol_level.gain_1 = config_param->config_param.vol_level.gain_1;
                gaming_mode_vol_balance_handle.chat_path_default_vol_level.gain_2 = config_param->config_param.vol_level.gain_2;
            } else if (source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_LINE_IN) {
                // do nothing
            } else if (source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_I2S_IN) {
                // do nothing
            } else if (source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_DONGLE_LINE_OUT) {
                // do nothing
            } else {
                gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_1 = config_param->config_param.vol_level.gain_1;
                if ((gaming_mode_vol_balance_handle.enable) && (gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1 < config_param->config_param.vol_level.gain_1)) {
                    new_gain = gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1;
                } else if ((gaming_mode_vol_balance_handle.enable) && (gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1 >= config_param->config_param.vol_level.gain_1)) {
                    gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1 = config_param->config_param.vol_level.gain_1;
                } else {
                    gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_1 = config_param->config_param.vol_level.gain_1;
                }
                gaming_mode_vol_balance_handle.game_path_default_vol_level.gain_2 = config_param->config_param.vol_level.gain_2;
                if ((gaming_mode_vol_balance_handle.enable) && (gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2 < config_param->config_param.vol_level.gain_2)) {
                    new_gain2 = gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2;
                } else if ((gaming_mode_vol_balance_handle.enable) && (gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2 >= config_param->config_param.vol_level.gain_2)) {
                    gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2 = config_param->config_param.vol_level.gain_2;
                } else {
                    gaming_mode_vol_balance_handle.game_path_balance_vol_level.gain_2 = config_param->config_param.vol_level.gain_2;
                }
            }
            hal_nvic_restore_interrupt_mask(saved_mask);
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */
            stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 1, new_gain);
            stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2, new_gain2);
#endif /* AIR_SOFTWARE_GAIN_ENABLE */
            break;

        case GAMING_MODE_CONFIG_OP_MUSIC_MIX:
#ifdef AIR_SOFTWARE_MIXER_ENABLE
            /* mix stream */
            scenario_id_0 = source->param.data_dl.scenario_type;
            sub_id_0.gamingmode_id = source->param.data_dl.scenario_sub_id;
            scenario_id_1 = config_param->config_param.dl_mixer_param.scenario_id;
            sub_id_1.gamingmode_id = config_param->config_param.dl_mixer_param.sub_id;
            if (source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_LINE_IN) {
                sub_id_0.gamingmode_id = AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_LINE_IN;
                scenario_id_0 = AUDIO_TRANSMITTER_GAMING_MODE;
            } else if (source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_I2S_IN) {
                sub_id_0.gamingmode_id = AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_I2S_IN;
                scenario_id_0 = AUDIO_TRANSMITTER_GAMING_MODE;
            }
            application_ptr_0 = port_audio_transmitter_get_connection_if(scenario_id_0, sub_id_0);
            application_ptr_1 = port_audio_transmitter_get_connection_if(scenario_id_1, sub_id_1);
            if ((application_ptr_0 == NULL) || (application_ptr_1 == NULL)) {
                ULL_LOG_E("[audio transmitter][config] fail %d %d, id 0 %d_%d id 1 %d_%d", 6,
                                application_ptr_0,
                                application_ptr_1,
                                scenario_id_0,
                                sub_id_0,
                                scenario_id_1,
                                sub_id_1
                                );
                AUDIO_ASSERT(0);
            }
            /* get application handle */
            if ((application_ptr_0->source->scenario_type != AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_LINE_IN) &&
                (application_ptr_0->source->scenario_type != AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_I2S_IN)) {
                dongle_handle_0 = (gaming_mode_dongle_dl_handle_t *)(application_ptr_0->source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.handle);
            } else {
                dongle_handle_0 = gaming_mode_dongle_query_dl_handle(application_ptr_0->source);
            }
            if ((application_ptr_1->source->scenario_type != AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_LINE_IN) &&
                (application_ptr_1->source->scenario_type != AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_I2S_IN)) {
                dongle_handle_1 = (gaming_mode_dongle_dl_handle_t *)(application_ptr_1->source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.handle);
            } else {
                dongle_handle_1 = gaming_mode_dongle_query_dl_handle(application_ptr_1->source);
            }

            hal_nvic_save_and_set_interrupt_mask(&saved_mask);

            /* do mix connections */
            if (dongle_handle_0 == dongle_handle_1) {
                stream_function_sw_mixer_channel_connect(dongle_handle_0->mixer_member, 1, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, dongle_handle_0->mixer_member, 1);
                stream_function_sw_mixer_channel_connect(dongle_handle_0->mixer_member, 2, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, dongle_handle_0->mixer_member, 2);
            } else {
                stream_function_sw_mixer_channel_connect(dongle_handle_0->mixer_member, 1, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, dongle_handle_0->mixer_member, 1);
                stream_function_sw_mixer_channel_connect(dongle_handle_0->mixer_member, 2, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, dongle_handle_0->mixer_member, 2);
                stream_function_sw_mixer_channel_connect(dongle_handle_1->mixer_member, 1, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, dongle_handle_0->mixer_member, 1);
                stream_function_sw_mixer_channel_connect(dongle_handle_1->mixer_member, 2, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, dongle_handle_0->mixer_member, 2);

                stream_function_sw_mixer_channel_connect(dongle_handle_1->mixer_member, 1, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, dongle_handle_1->mixer_member, 1);
                stream_function_sw_mixer_channel_connect(dongle_handle_1->mixer_member, 2, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, dongle_handle_1->mixer_member, 2);
                stream_function_sw_mixer_channel_connect(dongle_handle_0->mixer_member, 1, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, dongle_handle_1->mixer_member, 1);
                stream_function_sw_mixer_channel_connect(dongle_handle_0->mixer_member, 2, SW_MIXER_CHANNEL_ATTRIBUTE_NORMAL, dongle_handle_1->mixer_member, 2);
            }

            /* update mixer status */
            dongle_handle_0->mixer_status = GAMING_MODE_STREAM_MIXER_MIX;
            dongle_handle_1->mixer_status = GAMING_MODE_STREAM_MIXER_MIX;

            hal_nvic_restore_interrupt_mask(saved_mask);

            ULL_LOG_I("[audio transmitter][config] mix done. %d, %d, %d, %d\r\n", 4, scenario_id_0, sub_id_0.gamingmode_id, scenario_id_1, sub_id_1.gamingmode_id);
#endif /* AIR_SOFTWARE_MIXER_ENABLE */
            break;

        case GAMING_MODE_CONFIG_OP_MUSIC_UNMIX:
#ifdef AIR_SOFTWARE_MIXER_ENABLE
            /* unmix stream */
            scenario_id_0 = source->param.data_dl.scenario_type;
            sub_id_0.gamingmode_id = source->param.data_dl.scenario_sub_id;
            if (source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_LINE_IN) {
                sub_id_0.gamingmode_id = AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_LINE_IN;
                scenario_id_0 = AUDIO_TRANSMITTER_GAMING_MODE;
            } else if (source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_I2S_IN) {
                sub_id_0.gamingmode_id = AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_I2S_IN;
                scenario_id_0 = AUDIO_TRANSMITTER_GAMING_MODE;
            }
            application_ptr_0 = port_audio_transmitter_get_connection_if(scenario_id_0, sub_id_0);
            if (application_ptr_0 == NULL) {
                ULL_LOG_E("[audio transmitter][config] fail %d, id 0 %d_%d", 3,
                                application_ptr_0,
                                scenario_id_0,
                                sub_id_0
                                );
                AUDIO_ASSERT(0);
            }

            /* get application handle */
            if ((application_ptr_0->source->scenario_type != AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_LINE_IN) &&
                (application_ptr_0->source->scenario_type != AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_I2S_IN)) {
                dongle_handle_0 = (gaming_mode_dongle_dl_handle_t *)(application_ptr_0->source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.handle);
            } else {
                dongle_handle_0 = gaming_mode_dongle_query_dl_handle(application_ptr_0->source);
            }
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);

            /* update mixer status */
            dongle_handle_0->mixer_status = GAMING_MODE_STREAM_MIXER_UNMIX;

            /* do disconnections */
            stream_function_sw_mixer_channel_disconnect_all(dongle_handle_0->mixer_member);

            hal_nvic_restore_interrupt_mask(saved_mask);

            ULL_LOG_I("[audio transmitter][config] unmix done. %d, %d\r\n", 2, scenario_id_0, sub_id_0.gamingmode_id);
#endif /* AIR_SOFTWARE_MIXER_ENABLE */
            break;

        default:
            ULL_LOG_E("[audio transmitter][config] unknow cmd:%d\r\n", 1, config_param->config_operation);
            break;
    }

    return true;
}

ATTR_TEXT_IN_IRAM_LEVEL_1 bool gaming_mode_dongle_dl_source_get_avail_size(SOURCE source, uint32_t *avail_size)
{
    uint32_t frame_count = 0;
    gaming_mode_dongle_dl_handle_t *dongle_handle = (gaming_mode_dongle_dl_handle_t *)(source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.handle);
    uint32_t remain_samples = 0;
    usb_in_parameter *usb_in_param;

    if (source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.fetch_flag == 0) {
        *avail_size = 0;
    } else {
        if (source->streamBuffer.ShareBufferInfo.read_offset < source->streamBuffer.ShareBufferInfo.write_offset) {
            /* normal case */
            *avail_size = source->streamBuffer.ShareBufferInfo.write_offset - source->streamBuffer.ShareBufferInfo.read_offset;
        } else if (source->streamBuffer.ShareBufferInfo.read_offset == source->streamBuffer.ShareBufferInfo.write_offset) {
            if (source->streamBuffer.ShareBufferInfo.bBufferIsFull == true) {
                /* buffer is full, so ReadOffset == WriteOffset */
                *avail_size = source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num;
                ULL_LOG_E("[audio transmitter][source]: buffer is full, index = %u", 1, source->streamBuffer.ShareBufferInfo.read_offset);
            } else {
                /* buffer is empty, so ReadOffset == WriteOffset */
                *avail_size = 0;
                dongle_handle->process_frames = 0;

#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG
                uint32_t current_timestamp = 0;
                uint32_t duration = 0;
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
                hal_gpt_get_duration_count(source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.data_timestamp,
                                           current_timestamp,
                                           &duration);
                ULL_LOG_E("[audio transmitter][source]: buffer is empty in a long time, index = %u", 1, source->streamBuffer.ShareBufferInfo.read_offset);
                ULL_LOG_E("[audio transmitter][source]: duration = %u, last time = %u, current time = %u", 3,
                             duration,
                             source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.data_timestamp,
                             current_timestamp);
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG */
            }
        } else {
            /* buffer wrapper case */
            *avail_size = source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num
                          - source->streamBuffer.ShareBufferInfo.read_offset
                          + source->streamBuffer.ShareBufferInfo.write_offset;
        }

        if (*avail_size != 0) {
            /* if this is the first time, check if the data in usb buffer is larger than 4ms */
            if (((uint32_t)source->param.data_dl.current_notification_index == 0) &&
                (*avail_size < (4 * (uint32_t)source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size))) {
                /* the data is not enough, so change avail_size to zero */
                *avail_size = 0;
                if (hal_nvic_query_exception_number() > 0) {
                    dongle_handle->data_status = GAMING_MODE_STREAM_DATA_EMPTY;
                }
            } else {
                /* change avail_size to actual data size in the buffer */
                frame_count = *avail_size / source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size;
                *avail_size = *avail_size - frame_count * sizeof(audio_transmitter_frame_header_t);

                if (hal_nvic_query_exception_number() > 0) {
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
                    /* check if there are at least 2.5ms data */
                    remain_samples = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port, 1) / sizeof(int16_t);
#endif /* AIR_SOFTWARE_BUFFER_ENABLE */

                    usb_in_param = &source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param;
                    if ((frame_count * (usb_in_param->codec_param.pcm.sample_rate / 1000) + remain_samples) >= (((usb_in_param->codec_param.pcm.sample_rate / 1000) * 2500) / 1000)) {
                        /* update data status to indicate there are data in stream and the mixer needs to wait this stream */
                        dongle_handle->data_status = GAMING_MODE_STREAM_DATA_IN_STREAM;
                    } else {
#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG
                        /* When in irq level, there should be at least 2ms data in share buffer because the period of ccni is 2.5ms */
                        ULL_LOG_E("[audio transmitter][source]: data is not enough, %u", 1, *avail_size);
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG */

                        /* update data status to indicate there are not enough data in stream and the mixer do not need to wait this stream */
                        dongle_handle->data_status = GAMING_MODE_STREAM_DATA_EMPTY;
                        *avail_size = 0;
                        dongle_handle->process_frames = 0;
                    }
                }
            }
        }
    }

#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    ULL_LOG_I("[audio transmitter][source][get_avail_size]:%u, %u, %d, %u, %d", 5,
                 source->streamBuffer.ShareBufferInfo.read_offset,
                 source->streamBuffer.ShareBufferInfo.write_offset,
                 *avail_size,
                 current_timestamp,
                 hal_nvic_query_exception_number());
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG */

    return true;
}

ATTR_TEXT_IN_IRAM_LEVEL_1 uint32_t gaming_mode_dongle_dl_source_copy_payload(SOURCE source, uint8_t *src_buf, uint8_t *dst_buf, uint32_t length)
{
    DSP_STREAMING_PARA_PTR stream = NULL;
    audio_transmitter_frame_header_t *frame_header = NULL;
    uint32_t total_frames = 0;
    uint32_t total_samples = 0;
    uint32_t current_frame_samples = 0;
    uint32_t current_frame = 0;
    uint32_t avail_size = 0;
    uint32_t total_buffer_size = 0;
    uint32_t ReadOffset = 0;
    uint32_t sample_byte = 0;
    usb_in_parameter *usb_in_param;
    gaming_mode_dongle_dl_handle_t *dongle_handle = (gaming_mode_dongle_dl_handle_t *)(source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.handle);
    UNUSED(dongle_handle);
    UNUSED(dst_buf);

    usb_in_param = &source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param;
    if (usb_in_param->codec_param.pcm.format == HAL_AUDIO_PCM_FORMAT_S16_LE) {
        sample_byte = 2;
    } else if (usb_in_param->codec_param.pcm.format == HAL_AUDIO_PCM_FORMAT_S24_LE) {
        sample_byte = 3;
    } else {
        AUDIO_ASSERT(0 && "[audio transmitter][source]: error USB format parameters");
    }

    /* update data receive timestamp */
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.data_timestamp);

    /* get the stream */
    stream = DSP_Streaming_Get(source, source->transform->sink);

    /* get the total frame number and buffer size in share buffer */
    total_frames = length / (source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size - sizeof(audio_transmitter_frame_header_t));
    total_buffer_size = source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size *
                        source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num;

    /* copy the pcm data in share buffer into the stream buffers */
    if (source->param.data_dl.current_notification_index == 0) {
        gaming_mode_dongle_dl_source_get_avail_size(source, &avail_size);
        if (avail_size > length) {
            /* at the first time, maybe the actual data size is larger than stream->callback.EntryPara.in_malloc_size */
            /* in dsp_callback.c, DSP_Callback_Processing will cut length to the stream->callback.EntryPara.in_malloc_size */
            /* in here, we need to change length back to the actual data size */
            length = avail_size;
            total_frames = length / (source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size - sizeof(audio_transmitter_frame_header_t));
        }

        /* parameters check */
        if (total_frames < 4) {
            ULL_LOG_E("[audio transmitter][source]: error parameters %d, %d", 2, total_frames, length);
            AUDIO_ASSERT(0);
        }

        /* Only copy the last 3.5ms data from the share buffer and the remian data in the head are dropped */
        total_samples = 0;
        current_frame = 0;
        ReadOffset = (source->streamBuffer.ShareBufferInfo.read_offset +
                      source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * (total_frames - 4)
                     ) % total_buffer_size;
        src_buf = (uint8_t *)(source->streamBuffer.ShareBufferInfo.start_addr + ReadOffset);
        while (current_frame < 4) {
            /* get current frame info and current frame samples */
            frame_header = (audio_transmitter_frame_header_t *)src_buf;
            current_frame_samples = frame_header->payload_len / (sample_byte * 2);
            if (current_frame == 0) {
                /* only copy 0.5ms(48K*0.5ms=24samples) data at the first time */
                current_frame_samples = 24;
                /* offset 0.5ms data in src_buf */
                src_buf = src_buf + 24 * (sample_byte * 2);
            }

            /* copy usb audio data from share buffer */
            if (sample_byte == 2) {
                ShareBufferCopy_I2D_16bit((U32 *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                          (U16 *)(stream->callback.EntryPara.in_ptr[0]) + total_samples,
                                          (U16 *)(stream->callback.EntryPara.in_ptr[1]) + total_samples,
                                          current_frame_samples);
            } else {
                ShareBufferCopy_I_24bit_to_D_16bit_2ch((U8 *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                      (U16 *)(stream->callback.EntryPara.in_ptr[0]) + total_samples,
                                      (U16 *)(stream->callback.EntryPara.in_ptr[1]) + total_samples,
                                      current_frame_samples);
            }

            if (current_frame == 0) {
                /* offset back src_buf to the original position */
                src_buf = src_buf - 24 * (sample_byte * 2);
            }

            /* update total samples */
            total_samples += current_frame_samples;

            /* update copied frame number */
            current_frame += 1;

            /* change to the next frame */
            ReadOffset = (source->streamBuffer.ShareBufferInfo.read_offset +
                          source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * (total_frames - 4 + current_frame)
                         ) % total_buffer_size;
            src_buf = (uint8_t *)(source->streamBuffer.ShareBufferInfo.start_addr + ReadOffset);

#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG
            // ULL_LOG_I("[audio transmitter][source]:copy samples %u, %d, %d, %d, %d, %d", 6,
            //                 src_buf,
            //                 length,
            //                 total_frames,
            //                 current_frame,
            //                 current_frame_samples,
            //                 total_samples);
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG */
        }
        // if(total_samples != 168)
        // {
        //     ULL_LOG_E("[audio transmitter][source]: error copy samples %d, %d", 2, length, total_samples);
        //     AUDIO_ASSERT(0);
        // }

        /* update processed frames */
        source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.process_frames = total_frames;

#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
        /* get default capid */
        if (dongle_handle->index == 1) {
            dongle_handle->cap_id_default = DCXO_CAPID_EFUSE_REG.DCXO_CAPID_EFUSE;
            dongle_handle->cap_id_cur = dongle_handle->cap_id_default;
        } else {
            dongle_handle->cap_id_default = gaming_mode_dongle_first_dl_handle->cap_id_default;
            dongle_handle->cap_id_cur = gaming_mode_dongle_first_dl_handle->cap_id_cur;
        }
#endif /* AIR_SOFTWARE_CLK_SKEW_ENABLE */

#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG
        uint32_t current_timestamp = 0;
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
        ULL_LOG_I("[audio transmitter][source][copy_payload]: get first data packet time = %u.", 1, current_timestamp);
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG */
    } else {
        /* parameters check */
        if (total_frames > 5) {
            /* in here, it means there are at least 6 packet data in share buffer. */
            ULL_LOG_E("[audio transmitter][source]: too much data in share buffer %d, %d, %u, %u, 0x%x, 0x%x", 6,
                         total_frames,
                         length,
                         source->streamBuffer.ShareBufferInfo.read_offset,
                         source->streamBuffer.ShareBufferInfo.write_offset,
                         gaming_mode_music_data_in_gpt_count,
                         gaming_mode_music_ccni_gpt_count);

            // AUDIO_ASSERT(0);

            /* get the new read index */
            source->streamBuffer.ShareBufferInfo.read_offset = (source->streamBuffer.ShareBufferInfo.write_offset + total_buffer_size -
                                                               source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * 3
                                                              ) % total_buffer_size;

            /* set WriteOffset - 3 as the new ReadOffset, it will drop the other data in the share buffer */
            extern VOID audio_transmitter_share_information_update_read_offset(SOURCE source, U32 ReadOffset);
            audio_transmitter_share_information_update_read_offset(source, source->streamBuffer.ShareBufferInfo.read_offset);

            /* get new share buffer info*/
            extern VOID audio_transmitter_share_information_fetch(SOURCE source, SINK sink);
            audio_transmitter_share_information_fetch(source, NULL);

            /* only process the last 3 packet data */
            total_frames = 3;
            length = 3 * (source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size - sizeof(audio_transmitter_frame_header_t));

            ULL_LOG_E("[audio transmitter][source]: change status, %d, %d, %u, %u", 4,
                         total_frames,
                         length,
                         source->streamBuffer.ShareBufferInfo.read_offset,
                         source->streamBuffer.ShareBufferInfo.write_offset);
        }

#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
        int32_t compensatory_samples = 0;
        uint32_t remain_samples = 0;

        /* get remain samples */
        remain_samples = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port, 1) / sizeof(int16_t);

        /* choose which way to do compensatory */
        if (dongle_handle->index == 1) {
            dongle_handle->compen_method = GAMING_MODE_COMPENSATORY_METHOD_CAPID_ADJUST;
        } else if (dongle_handle->compen_method == GAMING_MODE_COMPENSATORY_METHOD_SW_CLK_SKEW) {
            /* do nothing because sw clk skew is under running */
        } else if ((gaming_mode_dongle_first_dl_handle->data_status == GAMING_MODE_STREAM_DATA_EMPTY) &&
                   (dongle_handle->compen_method == GAMING_MODE_COMPENSATORY_METHOD_DISABLE)) {
            if (dongle_handle->cap_id_count < 10) {
                dongle_handle->cap_id_count += 1;
            }

            if (dongle_handle->cap_id_count == 10) {
                dongle_handle->compen_method = GAMING_MODE_COMPENSATORY_METHOD_CAPID_ADJUST;
                dongle_handle->compen_count = 0;
                dongle_handle->compen_flag = 0;
            }
        } else if ((gaming_mode_dongle_first_dl_handle->data_status != GAMING_MODE_STREAM_DATA_EMPTY) &&
                   (dongle_handle->compen_method == GAMING_MODE_COMPENSATORY_METHOD_CAPID_ADJUST)) {
            dongle_handle->cap_id_count = 0;
            dongle_handle->compen_method = GAMING_MODE_COMPENSATORY_METHOD_DISABLE;
            dongle_handle->compen_count = 0;
            dongle_handle->compen_flag = 0;
            dongle_handle->total_compen_samples = 0;
        } else if ((gaming_mode_dongle_first_dl_handle->data_status != GAMING_MODE_STREAM_DATA_EMPTY) &&
                   (dongle_handle->total_compen_samples != 0) &&
                   (dongle_handle->compen_method == GAMING_MODE_COMPENSATORY_METHOD_DISABLE)) {
            dongle_handle->compen_method = GAMING_MODE_COMPENSATORY_METHOD_SW_CLK_SKEW;
        }

        /* do compensatory */
        if (dongle_handle->compen_method == GAMING_MODE_COMPENSATORY_METHOD_CAPID_ADJUST) {
            /* calculator compensatory samples */
            if ((dongle_handle->total_compen_samples == 0) && (total_frames == 2) && ((remain_samples + total_frames * 48 - 120) <= 0)) {
                if (dongle_handle->compen_count < 200) {
                    /* in here, it means there may be +1ms compensatory but its persistent period is less than 1 second */
                    dongle_handle->compen_count += 1;
                    dongle_handle->compen_flag = 1;
                } else {
                    /* in here, it means there is +1ms compensatory and its persistent period is more than 1 second */
                    /* So we need to adjust 26M clock to compensatory +1ms data as soon as possible */
                    dongle_handle->total_compen_samples = 48;
                    dongle_handle->compen_count = 0;
                    dongle_handle->compen_flag = 0;
                }
            } else if ((dongle_handle->total_compen_samples == 0) && (dongle_handle->compen_flag == 1) && (total_frames == 2) && ((remain_samples + total_frames * 48 - 120) > 0)) {
                /* in here, it means maybe the +1ms compensatory is false alarm and the usb irq is just not stable in this time */
                if (dongle_handle->compen_count != 0) {
                    dongle_handle->compen_count -= 1;
                }
                if (dongle_handle->compen_count == 0) {
                    dongle_handle->compen_flag = 0;
                }
            } else if ((dongle_handle->total_compen_samples > 0) && (total_frames == 2) && ((remain_samples + total_frames * 48 - 120) > 0)) {
                if (dongle_handle->compen_count < 200) {
                    /* in here, it means maybe the +1ms compensatory is finished but its persistent period is less than 1 second */
                    dongle_handle->compen_count += 1;
                } else {
                    /* in here, it means maybe the +1ms compensatory is finished and its persistent period is more than 1 second */
                    /* So we need to restore 26M clock source to default value */
                    dongle_handle->total_compen_samples = 0;
                    dongle_handle->compen_count = 0;
                    dongle_handle->compen_flag = 0;
                }
            } else if ((dongle_handle->total_compen_samples == 0) && (total_frames == 3) && ((remain_samples + total_frames * 48 - 120) >= 96)) {
                if (dongle_handle->compen_count < 200) {
                    /* in here, it means there may be -1ms compensatory but its persistent period is less than 1 second */
                    dongle_handle->compen_count += 1;
                    dongle_handle->compen_flag = -1;
                } else {
                    /* in here, it means there is -1ms compensatory and its persistent period is more than 1 second */
                    /* So we need to adjust 26M clock to compensatory -1ms data as soon as possible */
                    dongle_handle->total_compen_samples = -48;
                    dongle_handle->compen_count = 0;
                }
            } else if ((dongle_handle->total_compen_samples == 0) && (dongle_handle->compen_flag == -1) && (total_frames == 3) && ((remain_samples + total_frames * 48 - 120) < 96)) {
                /* in here, it means maybe the -1ms compensatory is false alarm and the usb irq is just not stable in this time */
                if (dongle_handle->compen_count != 0) {
                    dongle_handle->compen_count -= 1;
                }
                if (dongle_handle->compen_count == 0) {
                    dongle_handle->compen_flag = 0;
                }
            } else if ((dongle_handle->total_compen_samples < 0) && (total_frames == 3) && ((remain_samples + total_frames * 48 - 120) < 96)) {
                if (dongle_handle->compen_count < 200) {
                    /* in here, it means maybe the -1ms compensatory is finished but its persistent period is less than 1 second */
                    dongle_handle->compen_count += 1;
                } else {
                    /* in here, it means maybe the -1ms compensatory is finished and its persistent period is more than 1 second */
                    /* So we need to restore 26M clock source to default value */
                    dongle_handle->total_compen_samples = 0;
                    dongle_handle->compen_count = 0;
                    dongle_handle->compen_flag = 0;
                }
            }

            /* adjust 26M clock source by requirment */
            if (dongle_handle->total_compen_samples > 0) {
                /* adjust 26M clock source to the lowest and it will cause +1ms data after a time */
                DCXO_CAPID_EFUSE_REG.DCXO_CAPID_EFUSE_SEL = 0x1;
                DCXO_CAPID_EFUSE_REG.DCXO_CAPID_EFUSE     = 511;
                dongle_handle->cap_id_cur = 511;
            } else if (dongle_handle->total_compen_samples < 0) {
                /* adjust 26M clock source to the highest and it will cause -1ms data after a time */
                DCXO_CAPID_EFUSE_REG.DCXO_CAPID_EFUSE_SEL = 0x1;
                DCXO_CAPID_EFUSE_REG.DCXO_CAPID_EFUSE     = 1;
                dongle_handle->cap_id_cur = 1;
            } else {
                /* adjust 26M clock source to the default setting */
                if (DCXO_CAPID_EFUSE_REG.DCXO_CAPID_EFUSE != dongle_handle->cap_id_default) {
                    DCXO_CAPID_EFUSE_REG.DCXO_CAPID_EFUSE_SEL = 0x1;
                    DCXO_CAPID_EFUSE_REG.DCXO_CAPID_EFUSE     = dongle_handle->cap_id_default;
                    dongle_handle->cap_id_cur = dongle_handle->cap_id_default;
                }
            }

            /* always output 120 samples */
            stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 1, 120 * sizeof(int16_t));
            stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 2, 120 * sizeof(int16_t));

            /* bypass sw clk skew */
            compensatory_samples = 0;
            stream_function_sw_clk_skew_configure_compensation_samples(dongle_handle->clk_skew_port, compensatory_samples);
            source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.clk_skew.compen_samples = compensatory_samples;
        } else if (dongle_handle->compen_method == GAMING_MODE_COMPENSATORY_METHOD_SW_CLK_SKEW) {
            /* double check if the compen_samples is right */
            if ((dongle_handle->total_compen_samples > 0) && ((remain_samples + total_frames * 48 - 120) >= 48)) {
                dongle_handle->total_compen_samples = 48 - (remain_samples + total_frames * 48 - 120);
            } else if ((dongle_handle->total_compen_samples < 0) && ((remain_samples + total_frames * 48 - 120) <= 0)) {
                dongle_handle->total_compen_samples = 48 - (remain_samples + total_frames * 48 - 120);
            }

            if (dongle_handle->total_compen_samples > 0) {
                /* buffer output 119 samples, clk skew will change them to 120 samples */
                compensatory_samples = 1;
                dongle_handle->total_compen_samples -= 1;
                stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 1, 119 * sizeof(int16_t));
                stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 2, 119 * sizeof(int16_t));
            } else if (dongle_handle->total_compen_samples < 0) {
                /* buffer output 121 samples, clk skew will change them to 120 samples */
                compensatory_samples = -1;
                dongle_handle->total_compen_samples += 1;
                stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 1, 121 * sizeof(int16_t));
                stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 2, 121 * sizeof(int16_t));
            } else {
                /* buffer output 120 samples */
                compensatory_samples = 0;
                stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 1, 120 * sizeof(int16_t));
                stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 2, 120 * sizeof(int16_t));
            }
            stream_function_sw_clk_skew_configure_compensation_samples(dongle_handle->clk_skew_port, compensatory_samples);
            source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.clk_skew.compen_samples = compensatory_samples;

            if (dongle_handle->total_compen_samples == 0) {
                dongle_handle->compen_method = GAMING_MODE_COMPENSATORY_METHOD_DISABLE;
                dongle_handle->compen_count = 0;
                dongle_handle->compen_flag = 0;
            }
        } else {
            /* calculator compensatory samples */
            if ((dongle_handle->total_compen_samples == 0) && (total_frames == 2) && ((remain_samples + total_frames * 48 - 120) <= 0)) {
                if (dongle_handle->compen_count < 10) {
                    /* in here, it means there may be +1ms compensatory but its persistent period is less than 50ms */
                    dongle_handle->compen_count += 1;
                    dongle_handle->compen_flag = 1;
                } else {
                    /* in here, it means there is +1ms compensatory and its persistent period is more than 50ms */
                    dongle_handle->total_compen_samples = 48;
                    dongle_handle->compen_count = 0;
                    dongle_handle->compen_flag = 0;
                }
            } else if ((dongle_handle->total_compen_samples == 0) && (dongle_handle->compen_flag == 1) && (total_frames == 2) && ((remain_samples + total_frames * 48 - 120) > 0)) {
                /* in here, it means maybe the +1ms compensatory is false alarm and the usb irq is just not stable in this time */
                if (dongle_handle->compen_count != 0) {
                    dongle_handle->compen_count -= 1;
                }
                if (dongle_handle->compen_count == 0) {
                    dongle_handle->compen_flag = 0;
                }
            } else if ((dongle_handle->total_compen_samples == 0) && (total_frames == 3) && ((remain_samples + total_frames * 48 - 120) >= 96)) {
                if (dongle_handle->compen_count < 10) {
                    /* in here, it means there may be -1ms compensatory but its persistent period is less than 50ms */
                    dongle_handle->compen_count += 1;
                    dongle_handle->compen_flag = -1;
                } else {
                    /* in here, it means there is -1ms compensatory and its persistent period is more than 50ms*/
                    dongle_handle->total_compen_samples = 48 - (remain_samples + total_frames * 48 - 120);
                    dongle_handle->compen_count = 0;
                }
            } else if ((dongle_handle->total_compen_samples == 0) && (dongle_handle->compen_flag == -1) && (total_frames == 3) && ((remain_samples + total_frames * 48 - 120) < 96)) {
                /* in here, it means maybe the -1ms compensatory is false alarm and the usb irq is just not stable in this time */
                if (dongle_handle->compen_count != 0) {
                    dongle_handle->compen_count -= 1;
                }
                if (dongle_handle->compen_count == 0) {
                    dongle_handle->compen_flag = 0;
                }
            }

            /* always output 120 samples */
            stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 1, 120 * sizeof(int16_t));
            stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 2, 120 * sizeof(int16_t));

            /* bypass sw clk skew */
            compensatory_samples = 0;
            stream_function_sw_clk_skew_configure_compensation_samples(dongle_handle->clk_skew_port, compensatory_samples);
            source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.clk_skew.compen_samples = compensatory_samples;
        }
#endif /* AIR_SOFTWARE_CLK_SKEW_ENABLE */

        /* Copy all data from the share buffer */
        total_samples = 0;
        current_frame = 0;
        ReadOffset = source->streamBuffer.ShareBufferInfo.read_offset;
        src_buf = (uint8_t *)(source->streamBuffer.ShareBufferInfo.start_addr + ReadOffset);
        while (current_frame < total_frames) {
            /* get current frame info and current frame samples */
            frame_header = (audio_transmitter_frame_header_t *)src_buf;
            current_frame_samples = frame_header->payload_len / (sample_byte * 2);

            /* copy usb audio data from share buffer */
            if (sample_byte == 2) {
                ShareBufferCopy_I2D_16bit((U32 *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                          (U16 *)(stream->callback.EntryPara.in_ptr[0]) + total_samples,
                                          (U16 *)(stream->callback.EntryPara.in_ptr[1]) + total_samples,
                                          current_frame_samples);
            } else {
                ShareBufferCopy_I_24bit_to_D_16bit_2ch((U8 *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                          (U16 *)(stream->callback.EntryPara.in_ptr[0]) + total_samples,
                                          (U16 *)(stream->callback.EntryPara.in_ptr[1]) + total_samples,
                                          current_frame_samples);
            }

            /* update total samples */
            total_samples += current_frame_samples;

            /* update copied frame number */
            current_frame += 1;

            /* change to the next frame */
            ReadOffset = (source->streamBuffer.ShareBufferInfo.read_offset +
                          source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * current_frame
                         ) % total_buffer_size;
            src_buf = (uint8_t *)(source->streamBuffer.ShareBufferInfo.start_addr + ReadOffset);

#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG
            // ULL_LOG_I("[audio transmitter][source]:copy samples %u, %d, %d, %d, %d, %d", 6,
            //                 src_buf,
            //                 length,
            //                 total_frames,
            //                 current_frame,
            //                 current_frame_samples,
            //                 total_samples);
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG */
        }
        if (total_samples != ((total_frames * frame_header->payload_len) / (sample_byte * 2))) {
            ULL_LOG_E("[audio transmitter][source]: error copy samples %d, %d", 2, length, total_samples);
            AUDIO_ASSERT(0);
        }

        /* update processed frames */
        source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.process_frames = total_frames;
    }

    /* update stream status */
    stream->callback.EntryPara.in_size = total_samples * sizeof(S16); /* Always keep stream in with 16bit. Above process has transfered to 16bit */
    stream->callback.EntryPara.in_channel_num = 2;
    stream->callback.EntryPara.resolution.process_res = RESOLUTION_16BIT;
    if (usb_in_param->codec_param.pcm.sample_rate == 48000) {
        stream->callback.EntryPara.in_sampling_rate = FS_RATE_48K;
    } else if (usb_in_param->codec_param.pcm.sample_rate == 96000) {
        stream->callback.EntryPara.in_sampling_rate = FS_RATE_96K;
    }

    /* update state machine */
    dongle_handle = (gaming_mode_dongle_dl_handle_t *)(source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.handle);
    dongle_handle->compen_samples = source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.clk_skew.compen_samples;
    dongle_handle->process_frames = source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.process_frames;
    dongle_handle->read_offset    = source->streamBuffer.ShareBufferInfo.read_offset;

#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    ULL_LOG_I("[audio transmitter][source][copy_payload]: %d, %d, %u, %d", 4, total_samples, length, current_timestamp, hal_nvic_query_exception_number());
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG */

    /* avoid payload length check error in here */
    length = (length > source->param.data_dl.max_payload_size) ? source->param.data_dl.max_payload_size : length;

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&gaming_mode_music_data_in_gpt_count);
    dongle_handle->data_in_gpt_count = gaming_mode_music_data_in_gpt_count;

    return length;
}

ATTR_TEXT_IN_IRAM_LEVEL_1 uint32_t gaming_mode_dongle_dl_source_get_new_read_offset(SOURCE source, U32 amount)
{
    uint32_t total_buffer_size;
    n9_dsp_share_info_ptr ShareBufferInfo;
    uint32_t ReadOffset;
    UNUSED(amount);

    /* get new read offset */
    ShareBufferInfo = &source->streamBuffer.ShareBufferInfo;
    total_buffer_size = ShareBufferInfo->sub_info.block_info.block_size * ShareBufferInfo->sub_info.block_info.block_num;
    ReadOffset = (ShareBufferInfo->read_offset +
                  ShareBufferInfo->sub_info.block_info.block_size * source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.process_frames
                 ) % total_buffer_size;

#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    ULL_LOG_I("[audio transmitter][source][get_new_read_offset]: %u, %u, %d", 3, ReadOffset, current_timestamp, hal_nvic_query_exception_number());
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG */

    return ReadOffset;
}

ATTR_TEXT_IN_IRAM_LEVEL_1 void gaming_mode_dongle_dl_source_drop_postprocess(SOURCE source, uint32_t amount)
{
    uint32_t saved_mask = 0;
    UNUSED(amount);

    /* update index */
    if (source->param.data_dl.current_notification_index == 0) {
        source->param.data_dl.current_notification_index = 1;
    }

#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG
    extern CELT_ENC_PARA gCeltEncPara;
    uint32_t remain_samples = 0;
    gaming_mode_dongle_dl_handle_t *dongle_handle = (gaming_mode_dongle_dl_handle_t *)(source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.handle);
    remain_samples = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port, 1) / sizeof(int16_t);
    ULL_LOG_I("[audio transmitter][source][drop_postprocess]: %d, %d, %d, %d, %d", 5,
                 source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.clk_skew.compen_samples,
                 remain_samples,
                 gCeltEncPara.RemainSamples,
                 source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.process_frames,
                 amount);
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG */

    /* update state machine */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.fetch_flag = 0;
    source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.process_frames = 0;
    hal_nvic_restore_interrupt_mask(saved_mask);

#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    ULL_LOG_I("[audio transmitter][source][drop_postprocess]: %u, %d", 2, current_timestamp, hal_nvic_query_exception_number());
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG */
}

bool gaming_mode_dongle_dl_source_close(SOURCE source)
{
    if (source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.timer.handle) {
        hal_gpt_sw_stop_timer_us(source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.timer.handle);
        hal_gpt_sw_free_timer(source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.timer.handle);
        source->param.data_dl.scenario_param.usb_in_broadcast_param.usb_in_param.timer.handle = 0;
    }

#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    ULL_LOG_I("[audio transmitter][source][close]: %u, %d", 2, current_timestamp, hal_nvic_query_exception_number());
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG */

    return true;
}

ATTR_TEXT_IN_IRAM_LEVEL_1 bool gaming_mode_dongle_dl_sink_get_avail_size(SINK sink, uint32_t *avail_size)
{
    n9_dsp_share_info_t *share_buffer_info = &(sink->streamBuffer.ShareBufferInfo);
    if (share_buffer_info->read_offset < share_buffer_info->write_offset) {
        /* normal case */
        *avail_size = share_buffer_info->sub_info.block_info.block_size * share_buffer_info->sub_info.block_info.block_num
                      - share_buffer_info->write_offset + share_buffer_info->read_offset;
    } else if (share_buffer_info->read_offset == share_buffer_info->write_offset) {
        if (share_buffer_info->bBufferIsFull == true) {
            /* buffer is full, so ReadOffset == WriteOffset */
            /* In this case, overwrite is allowed*/
            *avail_size = share_buffer_info->sub_info.block_info.block_size;

#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG
            ULL_LOG_I("[bt common][sink]: buffer is full, index = %u, status = %u", 2,
                         share_buffer_info->read_offset,
                         sink->param.bt_common.status);
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG */
        } else {
            /* buffer is empty, so ReadOffset == WriteOffset */
            *avail_size = share_buffer_info->sub_info.block_info.block_size * share_buffer_info->sub_info.block_info.block_num;
        }
    } else {
        /* buffer wrapper case */
        *avail_size = share_buffer_info->read_offset - share_buffer_info->write_offset;
    }
    /* change avail_size to blk_size-sizeof(bt_common_gaming_mode_music_frame_header_t) if the avail_size is too large */
    if (*avail_size >= share_buffer_info->sub_info.block_info.block_size) {
        *avail_size = share_buffer_info->sub_info.block_info.block_size - sizeof(bt_common_gaming_mode_music_frame_header_t);
    }

#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    ULL_LOG_I("[bt common][sink][get_avail_size]: %d, %u, %d", 3, *avail_size, current_timestamp, hal_nvic_query_exception_number());
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG */

    return true;
}

ATTR_TEXT_IN_IRAM_LEVEL_1 uint32_t gaming_mode_dongle_dl_sink_copy_payload(SINK sink, uint8_t *src_buf, uint32_t length)
{
    uint32_t i = 0;
    uint32_t saved_mask = 0;
    uint32_t *dst_buf = NULL;
    uint32_t *src_buf0 = NULL;
    uint32_t diff_clk;
    /* make sure that game_frame_header's start address is 4B aligned on the stack for word accessing */
    __attribute__((__aligned__(4))) bt_common_gaming_mode_music_frame_header_t game_frame_header;
    uint32_t crc32;

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (gaming_mode_dongle_sink == NULL) {
        gaming_mode_dongle_sink = sink;
    } else if (gaming_mode_dongle_sink != sink) {
        /* in here, it means the sink has been changed because the mixer and we need sync sink setting at now for avoiding seq number is not continuous */
        sink->param.bt_common.current_notification_index                                         = gaming_mode_dongle_sink->param.bt_common.current_notification_index;
        sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.data_timestamp  = gaming_mode_dongle_sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.data_timestamp;
        sink->param.bt_common.seq_num                                                            = gaming_mode_dongle_sink->param.bt_common.seq_num;
        sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_timestamp = gaming_mode_dongle_sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_timestamp;
        sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_status    = gaming_mode_dongle_sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_status;
        sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_failcount = gaming_mode_dongle_sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_failcount;

        /* change gaming mode dongle sink to this current sink */
        gaming_mode_dongle_sink = sink;
    }

    /* check if it is not the first time */
    if (sink->param.bt_common.current_notification_index > 0) {
        /* get bt clk diff */
        if (gaming_mode_music_ccni_bt_clk >= sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.data_timestamp) {
            diff_clk = gaming_mode_music_ccni_bt_clk - sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.data_timestamp;
        } else {
            diff_clk = gaming_mode_music_ccni_bt_clk + (0x0fffffff - sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.data_timestamp + 1);
        }

        /* if the bt clock diff is larger than 2 packet, the seq number need to be compensatory */
        if (diff_clk >= 16) {
#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG
            ULL_LOG_I("[bt common][sink][copy_payload]old seq num = %u, last time = %u, current time = %u", 3,
                         sink->param.bt_common.seq_num,
                         sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.data_timestamp,
                         diff_clk);
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG */

            sink->param.bt_common.seq_num = sink->param.bt_common.seq_num  + diff_clk / 8 - 1;
            sink->param.bt_common.seq_num = sink->param.bt_common.seq_num & 0xff;

#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG
            ULL_LOG_I("[bt common][sink][copy_payload]new seq num = %u", 1, sink->param.bt_common.seq_num);
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG */
        }

        /* update time stamp */
        sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.data_timestamp = gaming_mode_music_ccni_bt_clk;
    } else {
        /* if it is first time, just update time stamp */
        sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.data_timestamp = gaming_mode_music_ccni_bt_clk;
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

    /* dst_buf and src_buf must be 4-byte aligned */
    dst_buf = (uint32_t *)(sink->streamBuffer.ShareBufferInfo.start_addr + sink->streamBuffer.ShareBufferInfo.write_offset);
    src_buf0 = (uint32_t *)src_buf;

    /* copy payload header */
    game_frame_header.seq_num = (uint8_t)(sink->param.bt_common.seq_num);
    game_frame_header.frame_size = 0;
    game_frame_header.check_sum = 0;
    for (; game_frame_header.frame_size < length; game_frame_header.frame_size++) {
        game_frame_header.check_sum += *src_buf++;
    }
    game_frame_header.check_sum += game_frame_header.seq_num;
    crc32 = 0xFFFFFF00;
    game_frame_header.crc32 = CRC32_Generate((uint8_t *)src_buf0, game_frame_header.frame_size, crc32);
    // memcpy(dst_buf, &game_frame_header, sizeof(bt_common_gaming_mode_music_frame_header_t));
    for (i = 0; i < (sizeof(bt_common_gaming_mode_music_frame_header_t) / 4); i++) {
        /* use word access for better performance */
        *dst_buf++ = *((uint32_t *)(&game_frame_header) + i);
    }

    /* copy payload */
    // memcpy(dst_buf + sizeof(bt_common_gaming_mode_music_frame_header_t), src_buf, length);
    if (length % 4) {
        AUDIO_ASSERT(0);
    }
    for (i = 0; i < (length / 4); i++) {
        /* use word access for better performance */
        *dst_buf++ = *src_buf0++;
    }

    /* update seq_num */
    sink->param.bt_common.seq_num += 1;
    sink->param.bt_common.seq_num = sink->param.bt_common.seq_num & 0xff;

#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    ULL_LOG_I("[bt common][sink][copy_payload]:%u, %d, %u, %d", 4, game_frame_header.seq_num, length, current_timestamp, hal_nvic_query_exception_number());
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG */

    return length;
}

ATTR_TEXT_IN_IRAM_LEVEL_1 bool gaming_mode_dongle_dl_sink_get_new_write_offset(SINK sink, U32 amount, uint32_t *new_write_offset)
{
    bool ret = false;
    uint32_t bt_clk;
    uint16_t bt_phase;
    uint32_t diff_clk;
    UNUSED(amount);
    UNUSED(new_write_offset);

    if (sink->param.bt_common.current_notification_index == 0) {
        /* get current bt clock */
        extern VOID MCE_GetBtClk(BTCLK * pCurrCLK, BTPHASE * pCurrPhase, BT_CLOCK_OFFSET_SCENARIO type);
        MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&bt_phase, BT_CLK_Offset);
        /* get bt clk diff */
        if (bt_clk >= sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_timestamp) {
            diff_clk = bt_clk - sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_timestamp;
        } else {
            diff_clk = bt_clk + (0x0fffffff - sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_timestamp + 1);
        }

        if (diff_clk > 2) {
            /* if the generation time of this packet is too larger(> 2*312.5us), drop this packet and not update write offset */
            *new_write_offset = sink->streamBuffer.ShareBufferInfo.write_offset;
            /* change event status to 0 to indicate that first packet has not been created */
            sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_status = 0;
            /* do not update write offset */
            ret = false;
            /* update fail count */
            sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_failcount += 1;
            if (sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_failcount > 100) {
                ULL_LOG_E("[bt common][sink]: get first packet error 0x%x, 0x%x.", 2,
                             sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_timestamp,
                             bt_clk);
                AUDIO_ASSERT(0);
            }
        } else {
            /* if the generation time of this packet is ok, send this packet as the first packet to bt controller */
            *new_write_offset = (sink->streamBuffer.ShareBufferInfo.write_offset + sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size) %
                                (sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num);
            /* change event status to 1 to indicate that first packet has been created */
            sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_status = 1;
            /* change event timestamp to current bt clock */
            sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_timestamp = bt_clk;
            /* update write offset */
            ret = true;
        }
        gaming_mode_music_first_packet_bt_clk = diff_clk;
    } else {
        *new_write_offset = (sink->streamBuffer.ShareBufferInfo.write_offset + sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size) %
                            (sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * sink->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num);
        /* update write offset */
        ret = true;
    }

    return ret;
}

ATTR_TEXT_IN_IRAM_LEVEL_1 bool gaming_mode_dongle_dl_sink_query_notification(SINK sink, bool *notification_flag)
{
    if (sink->param.bt_common.current_notification_index == 0) {
        if (sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_status == 0) {
            /* Becasue the first packet has not been created, so do not update index and send notification */
            /* the data in the buffer is not enough, so do not need to do notification */
            *notification_flag = false;
        } else {
            /* the data in the buffer is enough at first time, so do need to do notification */
            *notification_flag = true;
            /* increase the index */
            sink->param.bt_common.current_notification_index += 1;
        }
    } else {
        /* the data in the buffer is enough and notification has been sent, so do not need to do notification */
        *notification_flag = false;
    }

    return true;
}

ATTR_TEXT_IN_IRAM_LEVEL_1 bool gaming_mode_dongle_dl_sink_send_data_ready_notification(SINK sink)
{
    hal_ccni_message_t msg;
    UNUSED(msg);

    /* prepare message */
    msg.ccni_message[0] = AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0;
    /* in here, event_timestamp means the bt clock of the first packet flush and use it as the bt anchor time */
    msg.ccni_message[1] = sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_timestamp;

    /* send ccni messge to controller */
    hal_ccni_set_event(CCNI_DSP0_TO_CM4_AUDIO_BT_CONTROLLER_TX, &msg);
    ULL_LOG_I("[bt common][sink]: send ccni to bt controller done, %u.", 1,
                 sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_timestamp);

    return true;
}

ATTR_TEXT_IN_IRAM_LEVEL_1 void gaming_mode_dongle_dl_sink_flush_postprocess(SINK sink, uint32_t amount)
{
    UNUSED(amount);
    uint32_t bt_clk;
    uint16_t bt_phase;
    VOID MCE_GetBtClk(BTCLK * pCurrCLK, BTPHASE * pCurrPhase, BT_CLOCK_OFFSET_SCENARIO type);
    MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&bt_phase, BT_CLK_Offset);

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&gaming_mode_music_bt_out_gpt_count);

#if defined(AIR_SOFTWARE_MIXER_ENABLE) && defined(AIR_CELT_ENC_ENABLE)
    if (gaming_mode_music_first_packet_bt_clk >= 2) {
        ULL_LOG_W("[bt common][sink]: get first packet warning 0x%x, 0x%x, 0x%x, 0x%x, 0x%x.", 5,
                     gaming_mode_music_ccni_bt_clk,
                     bt_clk,
                     gaming_mode_music_ccni_gpt_count,
                     gaming_mode_music_data_in_gpt_count,
                     gaming_mode_music_bt_out_gpt_count);
        gaming_mode_music_first_packet_bt_clk = 0;
    }
#endif /* defined(AIR_SOFTWARE_MIXER_ENABLE) && defined(AIR_CELT_ENC_ENABLE) */

#ifdef AIR_AUDIO_DUMP_ENABLE
#if GAMING_MODE_MUSIC_DONGLE_DEBUG_DUMP
#ifdef AIR_CELT_ENC_ENABLE
    extern CELT_ENC_PARA gCeltEncPara;
    /* dump encoder's input data */
    LOG_AUDIO_DUMP((uint8_t *)(gCeltEncPara.RemainMemPtr),
                   gCeltEncPara.InputSamples * 4,
                   AUDIO_CODEC_IN);
#endif /* AIR_CELT_ENC_ENABLE */
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_DUMP */
#endif /* AIR_AUDIO_DUMP_ENABLE */

    int32_t i;
    uint32_t remain_samples = 0;
    uint32_t process_during_time;
    gaming_mode_dongle_dl_handle_t *c_handle = NULL;
    extern gaming_mode_dongle_dl_handle_t *gaming_mode_dongle_first_dl_handle;
    c_handle = gaming_mode_dongle_first_dl_handle;
    for (i = 0; i < gaming_mode_dongle_first_dl_handle->total_number; i++) {
        SOURCE source = (SOURCE)(c_handle->owner);

        if ((source->transform == NULL) || (source->transform->sink != sink)) {
            continue;
        }

        if ((source->transform != NULL) && (source->scenario_type != AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_LINE_IN) &&
            (source->scenario_type != AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_I2S_IN)) {
#ifdef AIR_SOFTWARE_BUFFER_ENABLE
            remain_samples = stream_function_sw_buffer_get_channel_used_size(c_handle->buffer_port, 1) / sizeof(int16_t);
#endif /* AIR_SOFTWARE_BUFFER_ENABLE */
            hal_gpt_get_duration_count(gaming_mode_music_ccni_gpt_count, gaming_mode_music_bt_out_gpt_count, &process_during_time);
            ULL_LOG_I("[bt common][sink][flush_postprocess]: handle 0x%x: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, 0x%x, 0x%x, 0x%x, 0x%x, %d", 16,
                         c_handle,
                         sink->param.bt_common.seq_num,
                         c_handle->compen_samples,
                         remain_samples,
                         c_handle->process_frames,
                         c_handle->cap_id_default,
                         c_handle->cap_id_cur,
                         c_handle->cap_id_count,
                         c_handle->total_compen_samples,
                         c_handle->compen_count,
                         c_handle->compen_method,
                         sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.data_timestamp,
                         bt_clk,
                         gaming_mode_music_ccni_gpt_count,
                         gaming_mode_music_bt_out_gpt_count,
                         process_during_time);

#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG
            extern volatile uint32_t celt_enc_finish_gpt_count;
            ULL_LOG_I("[bt common][sink][flush_postprocess]: handle 0x%x: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x", 9,
                         c_handle,
                         gaming_mode_music_ccni_gpt_count,
                         c_handle->data_in_gpt_count,
                         c_handle->gain_port->finish_gpt_count,
                         c_handle->buffer_port->finish_gpt_count,
                         c_handle->clk_skew_port->finish_gpt_count,
                         c_handle->mixer_member->finish_gpt_count,
                         celt_enc_finish_gpt_count,
                         gaming_mode_music_bt_out_gpt_count);
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG */

#ifdef AIR_AUDIO_DUMP_ENABLE
#if GAMING_MODE_MUSIC_DONGLE_DEBUG_DUMP
            uint32_t j, address, read_offset, total_buffer_size;
            total_buffer_size = source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_num;
            for (j = 0; j < c_handle->process_frames; j++) {
                read_offset = (c_handle->read_offset + source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size * j) % total_buffer_size;
                address = source->streamBuffer.ShareBufferInfo.start_addr + read_offset + sizeof(audio_transmitter_frame_header_t);
                /* dump source raw data */
                LOG_AUDIO_DUMP((uint8_t *)address,
                            source->streamBuffer.ShareBufferInfo.sub_info.block_info.block_size - sizeof(audio_transmitter_frame_header_t),
                            SOURCE_IN2 + (source->param.data_dl.scenario_sub_id - AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0));
            }
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_DUMP */
#endif /* AIR_AUDIO_DUMP_ENABLE */
#ifdef AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
            uint8_t *data_buf_1_head;
            uint8_t *data_buf_1_tail;
            uint32_t data_buf_1_size;
            if ((source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0) && (source->param.data_dl.current_notification_index != 0) && (c_handle->process_frames != 0)) {
                gaming_mode_vol_balance_handle.data_size = 120*sizeof(int32_t); /* 48K/mono/32bit/2.5ms */
                stream_function_sw_mixer_channel_input_get_data_info(c_handle->mixer_member, 1, &data_buf_1_head, &data_buf_1_tail, &data_buf_1_size);
                ShareBufferCopy_D_16bit_to_D_32bit_1ch((uint16_t* )data_buf_1_head, (uint32_t* )(gaming_mode_vol_balance_handle.data_buf), 120);
                DTM_enqueue(DTM_EVENT_ID_GAMING_MODE_VOLUME_SMART_BALANCE_PROCESS, AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_USB_IN_0, false);
            } else if ((source->param.data_dl.scenario_sub_id == AUDIO_TRANSMITTER_GAMING_MODE_MUSIC_DONGLE_USB_IN_0) && (source->param.data_dl.current_notification_index != 0) && (c_handle->process_frames == 0)) {
                gaming_mode_vol_balance_handle.data_size = 120*sizeof(int32_t); /* 48K/mono/32bit/2.5ms */
                memset(gaming_mode_vol_balance_handle.data_buf, 0, gaming_mode_vol_balance_handle.data_size);
                DTM_enqueue(DTM_EVENT_ID_GAMING_MODE_VOLUME_SMART_BALANCE_PROCESS, AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_USB_IN_0, false);
            }
#endif /* AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE */
        }
#if defined AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE
        else if ((source->transform != NULL) && ((source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_LINE_IN) ||
                (source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_I2S_IN))) {
            /* afe in case */
            c_handle->cur_read_offset = source->streamBuffer.BufferInfo.ReadOffset;
            ULL_LOG_I("[bt common][sink][flush_postprocess]: handle 0x%x: %d, 0x%x, 0x%x, %u, %u, %u, %u, 0x%x, 0x%x, 0x%x, 0x%x", 12,
                            c_handle,
                            sink->param.bt_common.seq_num,
                            c_handle->afe_vul_cur,
                            c_handle->afe_vul_base,
                            c_handle->pre_write_offset,
                            c_handle->cur_write_offset,
                            c_handle->pre_read_offset,
                            c_handle->cur_read_offset,
                            sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.data_timestamp,
                            bt_clk,
                            gaming_mode_music_ccni_gpt_count,
                            gaming_mode_music_bt_out_gpt_count);
        }
#endif /* AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE||AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE */
            /* sync sink settings */
            if (source->transform->sink != sink) {
                /* set current sink's settings into other sinks */
                source->transform->sink->param.bt_common.current_notification_index                                         = sink->param.bt_common.current_notification_index;
                source->transform->sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.data_timestamp  = sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.data_timestamp;
                source->transform->sink->param.bt_common.seq_num                                                            = sink->param.bt_common.seq_num;
                source->transform->sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_timestamp = sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_timestamp;
                source->transform->sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_status    = sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_status;
                source->transform->sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_failcount = sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.event_failcount;
            }
        /* switch to the next dl stream */
        c_handle = c_handle->next_dl_handle;
    }
#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    ULL_LOG_I("[bt common][sink][flush_postprocess]: %u, %d", 2, current_timestamp, hal_nvic_query_exception_number());
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG */
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ bool gaming_mode_dongle_dl_sink_close(SINK sink)
{
    extern gaming_mode_dongle_dl_handle_t *gaming_mode_dongle_first_dl_handle;
    SOURCE source;
    uint32_t saved_mask = 0;

    if (sink->param.bt_common.scenario_param.usb_in_broadcast_param.bt_out_param.codec_type == AUDIO_DSP_CODEC_TYPE_OPUS) {
#ifdef AIR_CELT_ENC_ENABLE
        stream_codec_encoder_celt_deinit();
#endif /* AIR_CELT_ENC_ENABLE */
    }

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (gaming_mode_dongle_sink == sink) {
        if (gaming_mode_dongle_first_dl_handle == NULL) {
            gaming_mode_dongle_sink = NULL;
        } else {
            source = (SOURCE)(gaming_mode_dongle_first_dl_handle->owner);
            gaming_mode_dongle_sink = source->transform->sink;
        }
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

#if GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    ULL_LOG_I("[bt common][sink][sink_close]: %u, %d", 2, current_timestamp, hal_nvic_query_exception_number());
#endif /* GAMING_MODE_MUSIC_DONGLE_DEBUG_LOG */

    return true;
}
/******************************************************************************/
/*                  Gaming mode dongle voice path functions                   */
/******************************************************************************/
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ gaming_mode_dongle_ul_handle_t *gaming_mode_dongle_get_ul_handle(void *owner)
{
    uint32_t saved_mask;
    int16_t count;
    gaming_mode_dongle_ul_handle_t *dongle_handle = NULL;
    gaming_mode_dongle_ul_handle_t *c_handle = NULL;

    dongle_handle = malloc(sizeof(gaming_mode_dongle_ul_handle_t));
    if (dongle_handle == NULL) {
        AUDIO_ASSERT(0);
    }
    memset(dongle_handle, 0, sizeof(gaming_mode_dongle_ul_handle_t));

    dongle_handle->total_number = -1;
    dongle_handle->index = -1;
    dongle_handle->owner = owner;
    dongle_handle->next_ul_handle = NULL;

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    if (gaming_mode_dongle_first_ul_handle == NULL) {
        /* there is no item on the handle list */
        dongle_handle->total_number = 1;
        dongle_handle->index = 1;

        /* set this handle as the head of the handle list */
        gaming_mode_dongle_first_ul_handle = dongle_handle;
    } else {
        /* there are other items on the handle list */
        count = 1;
        c_handle = gaming_mode_dongle_first_ul_handle;
        while (c_handle != NULL) {
            count += 1;

            c_handle->total_number += 1;
            if (c_handle->total_number <= 0) {
                /* error status */
                AUDIO_ASSERT(0);
            }

            if (c_handle->next_ul_handle == NULL) {
                /* c_handle is the last item on the list now */
                dongle_handle->total_number = c_handle->total_number;
                dongle_handle->index = c_handle->total_number;

                /* dongle_handle is the last item on the list now */
                c_handle->next_ul_handle = dongle_handle;

                break;
            }

            c_handle = c_handle->next_ul_handle;
        }
        if ((c_handle == NULL) || (dongle_handle->total_number != count)) {
            AUDIO_ASSERT(0);
        }
    }

    hal_nvic_restore_interrupt_mask(saved_mask);

    return dongle_handle;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ gaming_mode_dongle_ul_handle_t *gaming_mode_dongle_query_ul_handle_by_scenario_type(audio_scenario_type_t type)
{
    uint32_t saved_mask;
    uint32_t i;
    uint32_t count;
    gaming_mode_dongle_ul_handle_t *dongle_handle = NULL;
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    dongle_handle = gaming_mode_dongle_first_ul_handle;
    if (dongle_handle != NULL) {
        count = dongle_handle->total_number;
        for (i=0; i < count; i++) {
            /* check owner */
            if (((SOURCE)(dongle_handle->owner))->scenario_type == type) {
                break;
            } else {
                dongle_handle = dongle_handle->next_ul_handle;
            }
        }
    }
    hal_nvic_restore_interrupt_mask(saved_mask);
    return dongle_handle;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void gaming_mode_dongle_release_ul_handle(gaming_mode_dongle_ul_handle_t *handle)
{
    uint32_t saved_mask, i, count;
    gaming_mode_dongle_ul_handle_t *l_handle = NULL;
    gaming_mode_dongle_ul_handle_t *c_handle = NULL;
    gaming_mode_dongle_ul_handle_t *dongle_handle = NULL;

    if ((handle == NULL) || (gaming_mode_dongle_first_ul_handle == NULL)) {
        AUDIO_ASSERT(0);
    }

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    if (gaming_mode_dongle_first_ul_handle->total_number <= 0) {
        /* error status */
        AUDIO_ASSERT(0);
    } else if ((gaming_mode_dongle_first_ul_handle->total_number == 1) &&
               (gaming_mode_dongle_first_ul_handle == handle)) {
        /* this handle is only one item on handle list */
        if (gaming_mode_dongle_first_ul_handle->next_ul_handle != NULL) {
            AUDIO_ASSERT(0);
        }

        /* there is no item on the handle list */
        gaming_mode_dongle_first_ul_handle = NULL;
    } else if ((gaming_mode_dongle_first_ul_handle->total_number > 1) &&
               (gaming_mode_dongle_first_ul_handle == handle)) {
        /* this handle is the first item on handle list, but there are other handles on the list */
        c_handle = gaming_mode_dongle_first_ul_handle;
        count = gaming_mode_dongle_first_ul_handle->total_number;
        for (i = 0; i < count; i++) {
            c_handle->total_number -= 1;
            c_handle->index -= 1;

            c_handle = c_handle->next_ul_handle;
        }
        if (c_handle != NULL) {
            /* error status */
            AUDIO_ASSERT(0);
        }

        /* change the next iten to the head */
        gaming_mode_dongle_first_ul_handle = handle->next_ul_handle;
    } else {
        /* this handle is the middle item on handle list */
        c_handle = gaming_mode_dongle_first_ul_handle;
        count = gaming_mode_dongle_first_ul_handle->total_number;
        if (count == 1) {
            /* error status */
            AUDIO_ASSERT(0);
        }
        for (i = 0; i < count; i++) {
            if (c_handle == handle) {
                /* find out the handle on the list */
                dongle_handle = handle;
                l_handle->next_ul_handle = c_handle->next_ul_handle;
            }

            if (dongle_handle == NULL) {
                c_handle->total_number -= 1;
            } else {
                c_handle->total_number -= 1;
                c_handle->index -= 1;
            }

            l_handle = c_handle;
            c_handle = c_handle->next_ul_handle;
        }
        if ((c_handle != NULL) || (dongle_handle == NULL)) {
            /* error status */
            AUDIO_ASSERT(0);
        }
    }

    hal_nvic_restore_interrupt_mask(saved_mask);

    free(handle);
}

void gaming_mode_dongle_ul_init(SOURCE source, SINK sink, audio_transmitter_open_param_p audio_transmitter_open_param, bt_common_open_param_p bt_common_open_param)
{
    gaming_mode_dongle_ul_handle_t *dongle_handle;
    UNUSED(sink);

    /* register ccni handler */
    bt_common_register_isr_handler(gaming_mode_dongle_ul_ccni_handler);

    /* get handle for application config */
    dongle_handle = gaming_mode_dongle_get_ul_handle(source);
    source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle = (void *)dongle_handle;
    /* latency default is 40ms */
    dongle_handle->latency_us = 40000;

#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    sw_clk_skew_config_t sw_clk_skew_config;
    /* clock skew config */
    dongle_handle->compen_samples = 0;
    dongle_handle->clk_skew_count = 0;
    dongle_handle->clk_skew_port = stream_function_sw_clk_skew_get_port(source);
    sw_clk_skew_config.channel = 2;
    sw_clk_skew_config.bits = 16;
    sw_clk_skew_config.order = C_Flp_Ord_5;
    sw_clk_skew_config.skew_io_mode = C_Skew_Oup;
    sw_clk_skew_config.skew_compensation_mode = SW_CLK_SKEW_COMPENSATION_1_SAMPLE_IN_8_FRAME;
    sw_clk_skew_config.skew_work_mode = SW_CLK_SKEW_CONTINUOUS;
    sw_clk_skew_config.max_output_size = 15 * 16 * sizeof(int16_t); /* 15ms/16K buffer */
    sw_clk_skew_config.continuous_frame_size = 120 * sizeof(int16_t); /* fixed 7.5ms/16K input */
    stream_function_sw_clk_skew_init(dongle_handle->clk_skew_port, &sw_clk_skew_config);
#endif /* AIR_SOFTWARE_CLK_SKEW_ENABLE */

#ifdef AIR_SOFTWARE_BUFFER_ENABLE
    sw_buffer_config_t buffer_config;
#ifdef AIR_GAMING_MODE_DONGLE_ECNR_ENABLE
    /* init sw buffer list */
    dongle_handle->ecnr_buffer_list = stream_function_sw_buffer_get_list(source);
    stream_function_sw_buffer_list_init(dongle_handle->ecnr_buffer_list, 3);

    /* config the first buffer */
    buffer_config.mode = SW_BUFFER_MODE_MULTI_BUFFERS;
    buffer_config.total_channels = 2;
    buffer_config.watermark_max_size = 240 * sizeof(int16_t);
    buffer_config.watermark_min_size = 0;
    buffer_config.output_size = 240 * sizeof(int16_t); /* Output with 15ms */
    dongle_handle->ecnr_in_buffer_port = stream_function_sw_buffer_get_unused_port(source);
    stream_function_sw_buffer_init(dongle_handle->ecnr_in_buffer_port, &buffer_config);
    /* Prefill 12.5ms for ECNR process in the 1st time */
    stream_function_sw_buffer_config_channel_prefill_size(dongle_handle->ecnr_in_buffer_port, 1, 120 * sizeof(int16_t), true);
    stream_function_sw_buffer_config_channel_prefill_size(dongle_handle->ecnr_in_buffer_port, 2, 120 * sizeof(int16_t), true);
    stream_function_sw_buffer_list_insert_buffer(dongle_handle->ecnr_buffer_list, dongle_handle->ecnr_in_buffer_port, 0);

    /* config the second buffer */
    buffer_config.mode = SW_BUFFER_MODE_MULTI_BUFFERS;
    buffer_config.total_channels = 2;
    buffer_config.watermark_max_size = 360 * sizeof(int16_t); /* must be 2.5ms*N */
    buffer_config.watermark_min_size = 0;
    buffer_config.output_size = 120 * sizeof(int16_t);
    dongle_handle->ecnr_out_buffer_port = stream_function_sw_buffer_get_unused_port(source);
    stream_function_sw_buffer_init(dongle_handle->ecnr_out_buffer_port, &buffer_config);
    stream_function_sw_buffer_list_insert_buffer(dongle_handle->ecnr_buffer_list, dongle_handle->ecnr_out_buffer_port, 1);

    /* config the third buffer */
    buffer_config.mode = SW_BUFFER_MODE_MULTI_BUFFERS;
    buffer_config.total_channels = 2;
    buffer_config.watermark_max_size = 2400 * sizeof(int16_t); /* 150ms * 16K */
    buffer_config.watermark_min_size = 0;
    buffer_config.output_size = 120 * sizeof(int16_t);
    dongle_handle->buffer_port = stream_function_sw_buffer_get_unused_port(source);
    stream_function_sw_buffer_init(dongle_handle->buffer_port, &buffer_config);
    /* set default 15ms audio buffer */
    dongle_handle->buffer_watermark = 240;
    stream_function_sw_buffer_list_insert_buffer(dongle_handle->ecnr_buffer_list, dongle_handle->buffer_port, 2);
#else
    buffer_config.mode = SW_BUFFER_MODE_FIXED_OUTPUT_LENGTH;
    buffer_config.total_channels = 2;
    buffer_config.watermark_max_size = 2400 * sizeof(int16_t); /* 150ms * 16K */
    buffer_config.watermark_min_size = 0;
    buffer_config.output_size = 120 * sizeof(int16_t); /* 7.5ms * 16K */
    dongle_handle->buffer_port = stream_function_sw_buffer_get_port(source);
    stream_function_sw_buffer_init(dongle_handle->buffer_port, &buffer_config);
    /* set default 15ms audio buffer */
    dongle_handle->buffer_watermark = 240;
#endif /* AIR_GAMING_MODE_DONGLE_ECNR_ENABLE */
#endif /* AIR_SOFTWARE_BUFFER_ENABLE */
#ifdef AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE
    #ifdef AIR_SOFTWARE_MIXER_ENABLE
    /* sw mixer config */
    sw_mixer_callback_config_t       callback_config;
    sw_mixer_input_channel_config_t  in_ch_config;
    sw_mixer_output_channel_config_t out_ch_config;

    stream_function_sw_mixer_init(SW_MIXER_PORT_0);
    callback_config.preprocess_callback = NULL;
    callback_config.postprocess_callback = gaming_mode_dongle_ul_mixer_postcallback;
    in_ch_config.total_channel_number = 2;
    in_ch_config.resolution = RESOLUTION_16BIT;
    in_ch_config.input_mode = SW_MIXER_CHANNEL_MODE_OVERWRITE;
    in_ch_config.buffer_size = sizeof(int16_t) * 120; // 2.5ms * 48K
    out_ch_config.total_channel_number = 2;
    out_ch_config.resolution = RESOLUTION_16BIT;
    dongle_handle->mixer_member= stream_function_sw_mixer_member_create((void *)source,
                                                                        SW_MIXER_MEMBER_MODE_NO_BYPASS,
                                                                        &callback_config,
                                                                        &in_ch_config,
                                                                        &out_ch_config);
    if (dongle_handle->index == 1) {
        /* it is the first dl stream */
        stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, dongle_handle->mixer_member, true);
        // dual channels
        if ((g_line_out_buffer[0].buffer_base_pointer != NULL) || (g_line_out_buffer[1].buffer_base_pointer != NULL)) {
            DSP_MW_LOG_I("[ULL Dongle][Line Out] line out buffer addr error!", 0);
            configASSERT(0);
        }
        g_line_out_buffer[0].buffer_base_pointer = pvPortMalloc(sizeof(uint8_t) * 2 * 480); // 10ms 48K/2B
        g_line_out_buffer[1].buffer_base_pointer = pvPortMalloc(sizeof(uint8_t) * 2 * 480); // 10ms 48K/2B
        if ((!g_line_out_buffer[0].buffer_base_pointer) || (!g_line_out_buffer[1].buffer_base_pointer)) {
            DSP_MW_LOG_I("[ULL Dongle][Line Out] malloc fail!", 0);
            configASSERT(0);
        }
        g_line_out_buffer[0].write_pointer = 0;
        g_line_out_buffer[0].read_pointer = 0;
        g_line_out_buffer[0].buffer_byte_count = 2 * 480;

        g_line_out_buffer[1].write_pointer = 0;
        g_line_out_buffer[1].read_pointer = 0;
        g_line_out_buffer[1].buffer_byte_count = 2 * 480;
    } else {
        /* it is not the first dl stream, so we need to disconnect default connection */
        stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, dongle_handle->mixer_member, false);
    }
    stream_function_sw_mixer_channel_connect(dongle_handle->mixer_member, 1, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, dongle_handle->mixer_member, 1);
    stream_function_sw_mixer_channel_connect(dongle_handle->mixer_member, 2, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, dongle_handle->mixer_member, 2);
    #endif /* AIR_SOFTWARE_MIXER_ENABLE */
#endif /* AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE */
#ifdef AIR_SOFTWARE_SRC_ENABLE
    sw_src_config_t sw_src_config;
    sw_src_config.mode = SW_SRC_MODE_NORMAL;
    sw_src_config.channel_num = 1;
    sw_src_config.in_res = RESOLUTION_16BIT;
    sw_src_config.in_sampling_rate  = 16000;
    sw_src_config.in_frame_size_max = GAMING_MODE_VOICE_USB_FRAME_SAMPLES * sizeof(int16_t);
    sw_src_config.out_res           = RESOLUTION_16BIT;
    sw_src_config.out_sampling_rate = audio_transmitter_open_param->scenario_param.gaming_mode_param.codec_param.pcm.sample_rate;
    sw_src_config.out_frame_size_max = audio_transmitter_open_param->scenario_param.gaming_mode_param.codec_param.pcm.sample_rate / 16000 * GAMING_MODE_VOICE_USB_FRAME_SAMPLES * sizeof(int16_t);
    dongle_handle->src_port = stream_function_sw_src_get_port(source);
    stream_function_sw_src_init(dongle_handle->src_port, &sw_src_config);
    ULL_LOG_I("[bt common][config][SW_SRC][scenario %d-%d] %u, %u, %u, %u, %u, %u\r\n", 8,
                 audio_transmitter_open_param->scenario_type,
                 audio_transmitter_open_param->scenario_sub_id,
                 sw_src_config.in_res,
                 sw_src_config.in_sampling_rate,
                 sw_src_config.in_frame_size_max,
                 sw_src_config.out_res,
                 sw_src_config.out_sampling_rate,
                 sw_src_config.out_frame_size_max);
#endif /* AIR_SOFTWARE_SRC_ENABLE */

#ifdef AIR_SOFTWARE_GAIN_ENABLE
    /* configuare sw gain */
    int32_t default_gain;
    sw_gain_config_t default_config;
    default_config.resolution = RESOLUTION_16BIT;
    default_config.target_gain = bt_common_open_param->scenario_param.gaming_mode_param.gain_default_L;
    default_config.up_step = 25;
    default_config.up_samples_per_step = 16;
    default_config.down_step = -25;
    default_config.down_samples_per_step = 16;
    dongle_handle->gain_port = stream_function_sw_gain_get_port(source);
    stream_function_sw_gain_init(dongle_handle->gain_port, 2, &default_config);
    default_gain = bt_common_open_param->scenario_param.gaming_mode_param.gain_default_L;
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 1, default_gain);
    ULL_LOG_I("[bt common][config]change channel %d gain to %d*0.01dB\r\n", 2,
                 1,
                 default_gain);
    default_gain = bt_common_open_param->scenario_param.gaming_mode_param.gain_default_R;
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2, default_gain);
    ULL_LOG_I("[bt common][config]change channel %d gain to %d*0.01dB\r\n", 2,
                 2,
                 default_gain);
#endif /* AIR_SOFTWARE_GAIN_ENABLE */

#ifdef MTK_CELT_DEC_ENABLE
    /* configure codec version */
    extern uint32_t stream_codec_celt_set_version(uint32_t version);
    stream_codec_celt_set_version(bt_common_open_param->scenario_param.gaming_mode_param.codec_param.opus.version);
    ULL_LOG_I("[ULL][UL][config] codec version, 0x%x\r\n", 1, bt_common_open_param->scenario_param.gaming_mode_param.codec_param.opus.version);
    /* configure codec bit rate */
    if (bt_common_open_param->scenario_param.gaming_mode_param.codec_param.opus.bit_rate == 32000) {
        stream_codec_decoder_celt_set_input_frame_size(30);
        ULL_LOG_I("[bt common][config] codec frame size, %u\r\n", 1, 30);
    } else if (bt_common_open_param->scenario_param.gaming_mode_param.codec_param.opus.bit_rate == 50133) {
        stream_codec_decoder_celt_set_input_frame_size(47);
        ULL_LOG_I("[bt common][config] codec frame size, %u\r\n", 1, 47);
    } else {
        ULL_LOG_E("[bt common][config] error bitrate, %u\r\n", 1, bt_common_open_param->scenario_param.gaming_mode_param.codec_param.opus.bit_rate);
        AUDIO_ASSERT(0);
    }
#endif /* MTK_CELT_DEC_ENABLE */

#ifdef HAL_SLEEP_MANAGER_ENABLED
    /* lock sleep because sleep wake-up time will consume the stream process time */
    hal_sleep_manager_lock_sleep(SLEEP_LOCK_DONGLE_AUDIO);
#endif
}

void gaming_mode_dongle_ul_deinit(SOURCE source, SINK sink)
{
    gaming_mode_dongle_ul_handle_t *dongle_handle = (gaming_mode_dongle_ul_handle_t *)(source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle);
    UNUSED(sink);

    /* unregister ccni handler */
    bt_common_unregister_isr_handler(gaming_mode_dongle_ul_ccni_handler);

    /* application deinit */
#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    stream_function_sw_clk_skew_deinit(dongle_handle->clk_skew_port);
#endif /* AIR_SOFTWARE_CLK_SKEW_ENABLE */

#ifdef AIR_SOFTWARE_BUFFER_ENABLE
#ifdef AIR_GAMING_MODE_DONGLE_ECNR_ENABLE
    stream_function_sw_buffer_deinit(dongle_handle->buffer_port);
    stream_function_sw_buffer_deinit(dongle_handle->ecnr_in_buffer_port);
    stream_function_sw_buffer_deinit(dongle_handle->ecnr_out_buffer_port);
    stream_function_sw_buffer_list_deinit(dongle_handle->ecnr_buffer_list);
#else
    stream_function_sw_buffer_deinit(dongle_handle->buffer_port);
#endif /* AIR_GAMING_MODE_DONGLE_ECNR_ENABLE */
#endif /* AIR_SOFTWARE_BUFFER_ENABLE */
#ifdef AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE
    #ifdef AIR_SOFTWARE_MIXER_ENABLE
    stream_function_sw_mixer_member_unregister(SW_MIXER_PORT_0, dongle_handle->mixer_member);
    stream_function_sw_mixer_member_delete(dongle_handle->mixer_member);
    stream_function_sw_mixer_deinit(SW_MIXER_PORT_0);
    if (gaming_mode_dongle_first_ul_handle->total_number == 1) {
        vPortFree(g_line_out_buffer[0].buffer_base_pointer);
        g_line_out_buffer[0].buffer_base_pointer = NULL;
        vPortFree(g_line_out_buffer[1].buffer_base_pointer);
        g_line_out_buffer[1].buffer_base_pointer = NULL;
    }
    #endif /* AIR_SOFTWARE_MIXER_ENABLE */
#endif /* AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE */
#ifdef AIR_SOFTWARE_SRC_ENABLE
    stream_function_sw_src_deinit(dongle_handle->src_port);
#endif /* AIR_SOFTWARE_SRC_ENABLE */

#ifdef AIR_SOFTWARE_GAIN_ENABLE
    stream_function_sw_gain_deinit(dongle_handle->gain_port);
#endif /* AIR_SOFTWARE_GAIN_ENABLE */

    /* release handle */
    gaming_mode_dongle_release_ul_handle(dongle_handle);
    source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle = NULL;

#ifdef HAL_SLEEP_MANAGER_ENABLED
    /* unlock sleep */
    hal_sleep_manager_unlock_sleep(SLEEP_LOCK_DONGLE_AUDIO);
#endif
}

void gaming_mode_dongle_ul_start(SOURCE source)
{
    uint32_t saved_mask = 0;
    gaming_mode_dongle_ul_handle_t *dongle_handle = (gaming_mode_dongle_ul_handle_t *)(source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle);

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    dongle_handle->fetch_flag = 0;
    dongle_handle->play_en_status = 0;
    dongle_handle->stream_status = GAMING_MODE_UL_STREAM_INIT;
    dongle_handle->frame_count = 0;
    dongle_handle->frame_index = 1;
    dongle_handle->compen_samples = 0;

    hal_nvic_restore_interrupt_mask(saved_mask);
}

ATTR_TEXT_IN_IRAM_LEVEL_1 void gaming_mode_dongle_ul_ccni_handler(hal_ccni_event_t event, void *msg)
{
    gaming_mode_dongle_ul_handle_t *dongle_handle = gaming_mode_dongle_query_ul_handle_by_scenario_type(AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_DONGLE_USB_OUT);
    SOURCE source;
    uint32_t saved_mask;
    UNUSED(event);
    UNUSED(msg);

    if (dongle_handle == NULL) {
        ULL_LOG_E("[bt common][source]first_ul_handle is NULL\r\n", 0);
        goto _ccni_return;
    }

    source = (SOURCE)(dongle_handle->owner);
    if ((source == NULL) || (source->transform == NULL)) {
        ULL_LOG_E("[bt common][source]source or transform is NULL\r\n", 0);
        goto _ccni_return;
    }

    if (dongle_handle->stream_status != GAMING_MODE_UL_STREAM_DEINIT) {
        hal_nvic_save_and_set_interrupt_mask(&saved_mask);
        if (dongle_handle->fetch_flag == 0) {
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &dongle_handle->data_in_gpt_count);
        }
        /* set fetch flag to trigger stream flow */
        dongle_handle->fetch_flag += 1;
        hal_nvic_restore_interrupt_mask(saved_mask);

        /* Handler the stream */
        AudioCheckTransformHandle(source->transform);
    }

_ccni_return:
    return;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ bool gaming_mode_dongle_ul_config(SOURCE source, stream_config_type type, U32 value)
{
    uint32_t saved_mask;
    gaming_mode_runtime_config_param_p config_param = (gaming_mode_runtime_config_param_p)value;
    gaming_mode_dongle_ul_handle_t *dongle_handle = (gaming_mode_dongle_ul_handle_t *)(source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle);
    UNUSED(type);
    UNUSED(value);
#ifdef AIR_SOFTWARE_GAIN_ENABLE
    int32_t new_gain;
    sw_gain_config_t old_config;
#endif /* AIR_SOFTWARE_GAIN_ENABLE */
    UNUSED(dongle_handle);

    switch (config_param->config_operation) {
        case GAMING_MODE_CONFIG_OP_VOL_LEVEL_VOICE_L:
#ifdef AIR_SOFTWARE_GAIN_ENABLE
            /* left channel */
            new_gain = config_param->config_param.vol_level.gain_1;
            stream_function_sw_gain_get_config(dongle_handle->gain_port, 1, &old_config);
            ULL_LOG_I("[bt common][config]change channel%d gain from %d*0.01dB to %d*0.01dB\r\n", 3,
                         1,
                         old_config.target_gain,
                         new_gain);
            stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 1, new_gain);
#endif /* AIR_SOFTWARE_GAIN_ENABLE */
            break;

        case GAMING_MODE_CONFIG_OP_VOL_LEVEL_VOICE_R:
#ifdef AIR_SOFTWARE_GAIN_ENABLE
            /* right channel */
            new_gain = config_param->config_param.vol_level.gain_2;
            stream_function_sw_gain_get_config(dongle_handle->gain_port, 2, &old_config);
            ULL_LOG_I("[bt common][config]change channel%d gain from %d*0.01dB to %d*0.01dB\r\n", 3,
                         2,
                         old_config.target_gain,
                         new_gain);
            stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2, new_gain);
#endif /* AIR_SOFTWARE_GAIN_ENABLE */
            break;

        case GAMING_MODE_CONFIG_OP_VOL_LEVEL_VOICE_DUL:
#ifdef AIR_SOFTWARE_GAIN_ENABLE
            /* left channel */
            new_gain = config_param->config_param.vol_level.gain_1;
            stream_function_sw_gain_get_config(dongle_handle->gain_port, 1, &old_config);
            ULL_LOG_I("[bt common][config]change channel%d gain from %d*0.01dB to %d*0.01dB\r\n", 3,
                         1,
                         old_config.target_gain,
                         new_gain);
            stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 1, new_gain);

            /* right channel */
            new_gain = config_param->config_param.vol_level.gain_2;
            stream_function_sw_gain_get_config(dongle_handle->gain_port, 2, &old_config);
            ULL_LOG_I("[bt common][config]change channel%d gain from %d*0.01dB to %d*0.01dB\r\n", 3,
                         2,
                         old_config.target_gain,
                         new_gain);
            stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2, new_gain);
#endif /* AIR_SOFTWARE_GAIN_ENABLE */
            break;

        case GAMING_MODE_CONFIG_OP_LATENCY_SWITCH:
            if (config_param->config_param.latency_param.latency_us == 25000) {
                /* 15ms audio buffer */
                hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                dongle_handle->buffer_watermark = 200;
                hal_nvic_restore_interrupt_mask(saved_mask);
            } else if (config_param->config_param.latency_param.latency_us == 40000) {
                /* 15ms audio buffer */
                hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                dongle_handle->buffer_watermark = 240;
                hal_nvic_restore_interrupt_mask(saved_mask);
            } else if (config_param->config_param.latency_param.latency_us == 60000) {
                /* 60ms audio buffer */
                hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                dongle_handle->buffer_watermark = 960;
                hal_nvic_restore_interrupt_mask(saved_mask);
            } else {
                ULL_LOG_E("[bt common][config] unknow latency:%d\r\n", 1, config_param->config_param.latency_param.latency_us);
                break;
            }

            /* switch latency */
            ULL_LOG_I("[bt common][config]latency switch from %d us to %d us\r\n", 2,
                         dongle_handle->latency_us,
                         config_param->config_param.latency_param.latency_us);
            dongle_handle->latency_us = config_param->config_param.latency_param.latency_us;
            break;

        default:
            ULL_LOG_E("[bt common][config] unknow cmd:%d\r\n", 1, config_param->config_operation);
            break;
    }

    return true;
}

ATTR_TEXT_IN_IRAM_LEVEL_1 bool gaming_mode_dongle_ul_source_get_avail_size(SOURCE source, uint32_t *avail_size)
{
    gaming_mode_dongle_ul_handle_t *dongle_handle = (gaming_mode_dongle_ul_handle_t *)(source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle);

    if (dongle_handle->fetch_flag != 0) {
        /* If there is fetch requirement, we must wake up the stream even there is no packet on the share buffer */
        *avail_size = source->streamBuffer.AVMBufferInfo.MemBlkSize;
    } else if ((source->streamBuffer.AVMBufferInfo.ReadIndex != source->streamBuffer.AVMBufferInfo.WriteIndex) &&
               (dongle_handle->stream_status == GAMING_MODE_UL_STREAM_RUNNING)) {
        /* If there is data in the share buffer, we must wake up the stream to process it */
        *avail_size = source->streamBuffer.AVMBufferInfo.MemBlkSize;
    } else {
        *avail_size = 0;
    }

    if (*avail_size != 0) {
        /* change avail_size to actual frame size */
        *avail_size = source->streamBuffer.AVMBufferInfo.MemBlkSize - sizeof(bt_common_gaming_mode_voice_frame_header_t) - 1;

        /* update data receive timestamp */
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.data_timestamp);
    }

#if GAMING_MODE_VOICE_DONGLE_DEBUG_LOG
    uint32_t current_timestamp;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    ULL_LOG_I("[bt common][source][get_avail_size]: %u, %u, %u, %d, %u, %d", 6,
                 source->param.bt_common.seq_num,
                 source->streamBuffer.AVMBufferInfo.ReadIndex,
                 source->streamBuffer.AVMBufferInfo.WriteIndex,
                 *avail_size,
                 current_timestamp,
                 hal_nvic_query_exception_number());
#endif /* GAMING_MODE_VOICE_DONGLE_DEBUG_LOG */

    return true;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ uint32_t gaming_mode_dongle_ul_source_copy_payload(SOURCE source, uint8_t *dst_buf, uint32_t length)
{
    uint8_t *src_buf = NULL;
    uint32_t saved_mask;
    uint32_t remain_samples = 0;
    uint8_t seq_num = 0;
    uint16_t check_sum = 0;
    bt_common_gaming_mode_voice_frame_header_t frame_header;
    DSP_STREAMING_PARA_PTR stream = NULL;
    uint32_t total_frames = 0;
    gaming_mode_dongle_ul_first_packet_status_t first_packet_status;
    uint32_t bt_clk;
    uint16_t bt_phase;
    gaming_mode_dongle_ul_handle_t *dongle_handle = (gaming_mode_dongle_ul_handle_t *)(source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle);
    int32_t compensatory_samples = 0;
    uint16_t frame_index;
    static uint32_t count = 0;

    extern void bt_common_share_information_fetch(SOURCE source, SINK sink);
    extern void bt_common_share_information_update_read_offset(SOURCE source, U32 ReadOffset);
    extern VOID MCE_GetBtClk(BTCLK * pCurrCLK, BTPHASE * pCurrPhase, BT_CLOCK_OFFSET_SCENARIO type);

    /* dongle status check */
    switch (dongle_handle->stream_status) {
        /* In this status stage, it means the stream is not ready */
        case GAMING_MODE_UL_STREAM_DEINIT:
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            dongle_handle->fetch_flag = 0;
            hal_nvic_restore_interrupt_mask(saved_mask);
            break;

        /* In this status stage, we will update ReadIndex to the lastest WriteIndex and the old packets are dropped */
        case GAMING_MODE_UL_STREAM_INIT:
            /* get the latest share buffer info */
            bt_common_share_information_fetch(source, NULL);

            /* get the total frames in the buffer */
            total_frames = (source->streamBuffer.AVMBufferInfo.WriteIndex +
                            source->streamBuffer.AVMBufferInfo.MemBlkNum -
                            source->streamBuffer.AVMBufferInfo.ReadIndex) % source->streamBuffer.AVMBufferInfo.MemBlkNum;

            /* check if there is packet in the share buffer */
            if (total_frames != 0) {
                /* set WriteIndex as the new ReadIndex at the first time, it will drop the all data in the share buffer */
                bt_common_share_information_update_read_offset(source, source->streamBuffer.AVMBufferInfo.WriteIndex);

                /* update stream status */
                dongle_handle->stream_status = GAMING_MODE_UL_STREAM_START;

                /* in here, it means there is at least 1 packet in share buffer at the first time */
                ULL_LOG_I("[bt common][source]: there is new data in share buffer, seq_num = %u, r_index = %u, w_index = %u, status = %u", 4,
                             source->param.bt_common.seq_num,
                             source->streamBuffer.AVMBufferInfo.ReadIndex,
                             source->streamBuffer.AVMBufferInfo.WriteIndex,
                             dongle_handle->stream_status);
            }

            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            dongle_handle->fetch_flag = 0;
            hal_nvic_restore_interrupt_mask(saved_mask);
            break;

        /* In this status stage, we will check if the newest packet is suitable for playing */
        case GAMING_MODE_UL_STREAM_START:
            /* get the latest share buffer info */
            bt_common_share_information_fetch(source, NULL);

            /* get the total frames in the buffer */
            total_frames = (source->streamBuffer.AVMBufferInfo.WriteIndex +
                            source->streamBuffer.AVMBufferInfo.MemBlkNum -
                            source->streamBuffer.AVMBufferInfo.ReadIndex) % source->streamBuffer.AVMBufferInfo.MemBlkNum;

            /* check if this packet is suitable for playing */
            if (total_frames != 0) {
                /* get the first packet's payload header */
                src_buf = (uint8_t *)(source->streamBuffer.AVMBufferInfo.StartAddr
                                      + source->streamBuffer.AVMBufferInfo.ReadIndex * source->streamBuffer.AVMBufferInfo.MemBlkSize);
                memcpy(&frame_header, src_buf, sizeof(bt_common_gaming_mode_voice_frame_header_t));

                /* check if the first packet is suitable */
                /* the anchor time in frame is larger 24 than the true bt clock that the packet sent time */
                frame_header.anchor = (frame_header.anchor + 0x0fffffff + 1 - 24) & 0x0fffffff;
                first_packet_status = gaming_mode_dongle_ul_first_packet_check(dongle_handle, frame_header.anchor);
                switch (first_packet_status) {
                    case GAMING_MODE_DONGLE_UL_FIRST_PACKET_NOT_READY:
                        hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                        dongle_handle->fetch_flag = 0;
                        hal_nvic_restore_interrupt_mask(saved_mask);
                        break;

                    case GAMING_MODE_DONGLE_UL_FIRST_PACKET_TIMEOUT:
                        hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                        dongle_handle->fetch_flag = 0;
                        hal_nvic_restore_interrupt_mask(saved_mask);

                        /* drop this packet */
                        source->streamBuffer.AVMBufferInfo.ReadIndex = (source->streamBuffer.AVMBufferInfo.ReadIndex + 1) % source->streamBuffer.AVMBufferInfo.MemBlkNum;
                        bt_common_share_information_update_read_offset(source, source->streamBuffer.AVMBufferInfo.ReadIndex);
                        ULL_LOG_I("[bt common][source]: this first packet is dropped, seq_num = %u, r_index = %u, w_index = %u, status = %u", 4,
                                     source->param.bt_common.seq_num,
                                     source->streamBuffer.AVMBufferInfo.ReadIndex,
                                     source->streamBuffer.AVMBufferInfo.WriteIndex,
                                     dongle_handle->stream_status);
                        break;

                    case GAMING_MODE_DONGLE_UL_FIRST_PACKET_READY:
                        hal_nvic_save_and_set_interrupt_mask(&saved_mask);
                        dongle_handle->fetch_flag = 0;
                        hal_nvic_restore_interrupt_mask(saved_mask);

                        /* update stream status */
                        dongle_handle->stream_status = GAMING_MODE_UL_STREAM_RUNNING;

                        /* set the seq num to the first packet's seq num */
                        src_buf = (uint8_t *)(source->streamBuffer.AVMBufferInfo.StartAddr
                                              + source->streamBuffer.AVMBufferInfo.ReadIndex * source->streamBuffer.AVMBufferInfo.MemBlkSize);
                        seq_num = *(src_buf + sizeof(bt_common_gaming_mode_voice_frame_header_t));
                        source->param.bt_common.seq_num = seq_num;

                        /* get current bt clock */
                        MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&bt_phase, BT_CLK_Offset);

                        /* first_packet_bt_clk should be later than first_anchor_bt_clk */
                        dongle_handle->first_packet_bt_clk = bt_clk;
                        /* first_anchor_bt_clk is the packet sent time on headset/earbuds side */
                        dongle_handle->first_anchor_bt_clk = frame_header.anchor;
                        if ((dongle_handle->first_anchor_bt_clk + GAMING_MODE_VOICE_PLAYEN_DELAY) > 0x0fffffff) {
                            dongle_handle->play_en_overflow = 1;
                        } else {
                            dongle_handle->play_en_overflow = 0;
                        }
                        /* play_en_bt_clk is the play time */
                        dongle_handle->play_en_bt_clk = (dongle_handle->first_anchor_bt_clk + GAMING_MODE_VOICE_PLAYEN_DELAY) & 0x0fffffff;

                        ULL_LOG_I("[bt common][source]: play will at 0x%x, first anchor = 0x%x, cur bt clk = 0x%x, seq_num = %u, r_index = %u, w_index = %u, status = %u", 7,
                                     dongle_handle->play_en_bt_clk,
                                     dongle_handle->first_anchor_bt_clk,
                                     bt_clk,
                                     source->param.bt_common.seq_num,
                                     source->streamBuffer.AVMBufferInfo.ReadIndex,
                                     source->streamBuffer.AVMBufferInfo.WriteIndex,
                                     dongle_handle->stream_status);

                        /* reset counter */
                        count = 0;
                        break;

                    default:
                        AUDIO_ASSERT(0);
                        break;
                }
            } else {
                if ((count % 1000) == 0) {
                    ULL_LOG_E("[bt common][source]: there is no uplink packet, %d", 1, count);
                }
                count += 1;
            }
            break;

        /* In this status stage, the stream is started and we will set flag to fetch a new packet */
        case GAMING_MODE_UL_STREAM_RUNNING:
            break;

        /* Error status */
        default:
            AUDIO_ASSERT(0);
            break;
    }

    /* check if the play is started */
    if ((dongle_handle->stream_status == GAMING_MODE_UL_STREAM_RUNNING) && (dongle_handle->play_en_status == 0) && gaming_mode_dongle_ul_check_play(dongle_handle)) {
        dongle_handle->play_en_status = 1;

        /* get current bt clock */
        extern VOID MCE_GetBtClk(BTCLK * pCurrCLK, BTPHASE * pCurrPhase, BT_CLOCK_OFFSET_SCENARIO type);
        uint32_t bt_clk;
        uint16_t bt_phase;
        MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&bt_phase, BT_CLK_Offset);

        hal_nvic_save_and_set_interrupt_mask(&saved_mask);
        /* avoid fetch flag is too much here */
        dongle_handle->fetch_flag = 1;
        hal_nvic_restore_interrupt_mask(saved_mask);

        ULL_LOG_I("[bt common][source]: play at now 0x%x, seq_num = %u, r_index = %u, w_index = %u, status = %u", 5,
                     bt_clk,
                     source->param.bt_common.seq_num,
                     source->streamBuffer.AVMBufferInfo.ReadIndex,
                     source->streamBuffer.AVMBufferInfo.WriteIndex,
                     dongle_handle->stream_status);
#ifdef AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE
        // LINE OUT Reset AFE Buffer
        if (source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_DONGLE_LINE_OUT) {
            // hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            // source->transform->sink->streamBuffer.BufferInfo.WriteOffset = 0;
            // source->transform->sink->streamBuffer.BufferInfo.ReadOffset = 0;
            // source->transform->sink->streamBuffer.BufferInfo.bBufferIsFull = false;
            // hal_nvic_restore_interrupt_mask(saved_mask);
            extern U32 SinkSizeAudio(SINK sink);
            // reopen DL 1
            // hal_audio_memory_parameter_t *mem_handle = &source->transform->sink->param.audio.mem_handle;
            // //hal_audio_set_memory(mem_handle, HAL_AUDIO_CONTROL_MEMORY_INTERFACE, HAL_AUDIO_CONTROL_OFF);
            // hal_audio_memory_setting(mem_handle, HAL_AUDIO_CONTROL_MEMORY_INTERFACE, HAL_AUDIO_CONTROL_OFF);
            // mem_handle->buffer_addr = NULL;
            // mem_handle->sync_status = HAL_AUDIO_MEMORY_SYNC_NONE;
            // hal_audio_memory_setting(mem_handle, HAL_AUDIO_CONTROL_MEMORY_INTERFACE, HAL_AUDIO_CONTROL_ON);
            // ULL_LOG_I("[ULL Dongle][Line Out] sink free size %d", 1, SinkSizeAudio(source->transform->sink));

            /* stop gpt timer */
            hal_gpt_sw_stop_timer_us(g_line_out_timer_handle);
            hal_gpt_sw_free_timer(g_line_out_timer_handle);
            g_line_out_timer_handle = 0;
            /* trigger AFE irq */
            hal_audio_trigger_start_parameter_t sw_trigger_start;
            sw_trigger_start.enable = true;
            sw_trigger_start.memory_select = source->transform->sink->param.audio.mem_handle.memory_select;
            ULL_LOG_I("[ULL][Line Out] trigger agent, owo[%d] oro[%d] dro[%d]", 3, AFE_GET_REG(ASM_CH01_OBUF_WRPNT) - AFE_GET_REG(ASM_OBUF_SADR),
                            AFE_GET_REG(ASM_CH01_OBUF_RDPNT) - AFE_GET_REG(ASM_OBUF_SADR), AFE_GET_REG(AFE_DL1_CUR) - AFE_GET_REG(AFE_DL1_BASE));
            hal_audio_set_value((hal_audio_set_value_parameter_t *)&sw_trigger_start, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
        }
#endif
    }

    /* process the frame */
    source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.drop_frames = 0;
    if (dongle_handle->stream_status == GAMING_MODE_UL_STREAM_RUNNING) {
        /* get the process frames in the buffer */
        total_frames = (source->streamBuffer.AVMBufferInfo.WriteIndex +
                        source->streamBuffer.AVMBufferInfo.MemBlkNum -
                        source->streamBuffer.AVMBufferInfo.ReadIndex) % source->streamBuffer.AVMBufferInfo.MemBlkNum;
        if (total_frames > 2) {
            /* in here, it means there is at least 2 packet in share buffer. */
            ULL_LOG_I("[bt common][source]: there is too much data in share buffer, seq_num = %u, r_index = %u, w_index = %u", 3,
                         source->param.bt_common.seq_num,
                         source->streamBuffer.AVMBufferInfo.ReadIndex,
                         source->streamBuffer.AVMBufferInfo.WriteIndex);

            /* get the new read index */
            source->streamBuffer.AVMBufferInfo.ReadIndex = (source->streamBuffer.AVMBufferInfo.WriteIndex +
                                                            source->streamBuffer.AVMBufferInfo.MemBlkNum -
                                                            2) % source->streamBuffer.AVMBufferInfo.MemBlkNum;

            /* set WriteIndex - 2 as the new ReadIndex at the first time, it will drop the other data in the share buffer */
            extern void bt_common_share_information_update_read_offset(SOURCE source, U32 ReadOffset);
            bt_common_share_information_update_read_offset(source, source->streamBuffer.AVMBufferInfo.ReadIndex);

            /* get the latest share buffer info */
            extern void bt_common_share_information_fetch(SOURCE source, SINK sink);
            bt_common_share_information_fetch(source, NULL);

            /* update seq_num and process_frames */
            src_buf = (uint8_t *)(source->streamBuffer.AVMBufferInfo.StartAddr +
                                  source->streamBuffer.AVMBufferInfo.ReadIndex * source->streamBuffer.AVMBufferInfo.MemBlkSize);
            seq_num = *(src_buf + sizeof(bt_common_gaming_mode_voice_frame_header_t));
            source->param.bt_common.seq_num = seq_num;

            /* set process_frames */
            source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.process_frames = 2;

            ULL_LOG_I("[bt common][source]: change status, seq_num = %u, r_index = %u, w_index = %u", 3,
                         source->param.bt_common.seq_num,
                         source->streamBuffer.AVMBufferInfo.ReadIndex,
                         source->streamBuffer.AVMBufferInfo.WriteIndex);
        } else if (total_frames == 0) {
            /* set process_frames */
            source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.process_frames = 0;
        } else {
            /* set process_frames */
            source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.process_frames = total_frames;
        }

        /* check copy data or do PLC or bypass the decoder */
        source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.drop_frames = 0;
#ifdef AIR_GAMING_MODE_DONGLE_ECNR_ENABLE
        remain_samples = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port, 1) / sizeof(int16_t);
        if ((dongle_handle->play_en_status != 0) && (dongle_handle->fetch_flag != 0) && (remain_samples >= GAMING_MODE_VOICE_USB_FRAME_SAMPLES)) {
            /* speical flow: output voice data into the share buffer firstly, then process uplink packet from air */
            dongle_handle->data_status = GAMING_MODE_DONGLE_UL_DATA_BYPASS_DECODER;
            goto _do_packet_process;
        }
#endif
        if (source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.process_frames == 0) {
            remain_samples = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port, 1) / sizeof(int16_t);
            if (dongle_handle->fetch_flag != 0) {
                total_frames = 1;
            } else {
                total_frames = 0;
            }
            if (remain_samples >= GAMING_MODE_VOICE_USB_FRAME_SAMPLES * total_frames) {
                /* in this case, it means there is no new data but remain sample is enough, so we bypass the decoder */
                /* set flag to indicate that needs to bypass decoder */
                dongle_handle->data_status = GAMING_MODE_DONGLE_UL_DATA_BYPASS_DECODER;
            } else {
                /* in this case, it means there is no new data and remain sample is not enough, so we need to do PLC here to prevent buffer empty */
                /* set flag to indicate that there is the packet lost */
                dongle_handle->data_status = GAMING_MODE_DONGLE_UL_DATA_PLC;
                if (dongle_handle->play_en_status == 0) {
                    dongle_handle->data_status = GAMING_MODE_DONGLE_UL_DATA_BYPASS_DECODER;
                }
                source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.process_frames += 1;

                ULL_LOG_I("[bt common][source]: The data packet is not on-time arrived, current seq_num = %u, status = %d",
                             2,
                             source->param.bt_common.seq_num,
                             dongle_handle->data_status);

                if (dongle_handle->play_en_status != 0) {
                    source->param.bt_common.seq_num = (source->param.bt_common.seq_num + 1) & 0xff;
                }
            }

            /* do not update read index */
            source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.drop_frames = 0;
        } else {
            /* in this case, it means there is new data (at most 2 packet) */
            /* find out suitable packet in this while loop */
            dongle_handle->data_status = GAMING_MODE_DONGLE_UL_DATA_EMPTY;
            while (dongle_handle->data_status == GAMING_MODE_DONGLE_UL_DATA_EMPTY) {
                /* get the frame base address */
                src_buf = (uint8_t *)(source->streamBuffer.AVMBufferInfo.StartAddr
                                      + source->streamBuffer.AVMBufferInfo.ReadIndex * source->streamBuffer.AVMBufferInfo.MemBlkSize);

                /* get payload header */
                memcpy(&frame_header, src_buf, sizeof(bt_common_gaming_mode_voice_frame_header_t));
                seq_num = *(src_buf + sizeof(bt_common_gaming_mode_voice_frame_header_t));

                /* frame check */
                if ((frame_header.frame_num != 1) ||
                    (frame_header.frame_size != length) ||
                    (frame_header.frame_size > (source->streamBuffer.AVMBufferInfo.MemBlkSize - sizeof(bt_common_gaming_mode_voice_frame_header_t) - 1))) {
                    /* in here, it means the frame is not right */
                    ULL_LOG_E("[bt common][source]: The data packet frame is not right, read_index = %u, frame_num = %u, frame_size = %u, length = %u",
                                 4,
                                 source->streamBuffer.AVMBufferInfo.ReadIndex,
                                 frame_header.frame_num,
                                 frame_header.frame_size,
                                 length);
                    /* set flag to indicate that there is the packet lost */
                    dongle_handle->data_status = GAMING_MODE_DONGLE_UL_DATA_PLC;
                    /* drop this packet */
                    source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.drop_frames += 1;
                    source->param.bt_common.seq_num = (source->param.bt_common.seq_num + 1) & 0xff;
                    continue;
                    // AUDIO_ASSERT(0);
                }

                /* checksum check */
                check_sum = 0;
                for (; frame_header.frame_size > 0; frame_header.frame_size--) {
                    check_sum += *(src_buf + sizeof(bt_common_gaming_mode_voice_frame_header_t) + 1 + frame_header.frame_size - 1);
                }
                check_sum += seq_num;
                check_sum += (frame_header.anchor >>  0) & 0xff;
                check_sum += (frame_header.anchor >>  8) & 0xff;
                check_sum += (frame_header.anchor >> 16) & 0xff;
                check_sum += (frame_header.anchor >> 24) & 0xff;
                if (check_sum != frame_header.check_sum) {
                    /* in here, it means the data in this packet is not right */
                    ULL_LOG_E("[bt common][source]: The data packet checksum is not right, read_index = %u, local check_sum = %u, frame check_sum = %u",
                                 3,
                                 source->streamBuffer.AVMBufferInfo.ReadIndex,
                                 check_sum,
                                 frame_header.check_sum);
                    /* set flag to indicate that there is the packet lost */
                    dongle_handle->data_status = GAMING_MODE_DONGLE_UL_DATA_PLC;
                    /* drop this packet */
                    source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.drop_frames += 1;
                    source->param.bt_common.seq_num = (source->param.bt_common.seq_num + 1) & 0xff;
                    continue;
                    // AUDIO_ASSERT(0);
                }

                /* check if there is the packet lost */
                if (source->param.bt_common.seq_num == seq_num) {
                    /* The data packet is continuous */
                    dongle_handle->data_status = GAMING_MODE_DONGLE_UL_DATA_NORMAL;
                    /* drop this packet */
                    source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.drop_frames += 1;
                } else if (((seq_num > source->param.bt_common.seq_num) && ((seq_num - source->param.bt_common.seq_num) <= 1)) ||
                           ((seq_num < source->param.bt_common.seq_num) && ((seq_num + 256 - source->param.bt_common.seq_num) <= 1))) {
                    /* The data packet is dropped endurably that at most 1 lost pakcets are allowed */
                    /* Just do decode here, do not PLC */
                    dongle_handle->data_status = GAMING_MODE_DONGLE_UL_DATA_NORMAL;
                    ULL_LOG_E("[bt common][source]: The data packet is dropped, current seq_num = %u, latest seq_num = %u",
                                 2,
                                 source->param.bt_common.seq_num,
                                 seq_num);
                    /* set seq_num to the newest number for keeping data is continuous */
                    source->param.bt_common.seq_num = seq_num;
                    /* drop this packet */
                    source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.drop_frames += 1;
                } else if (((seq_num > source->param.bt_common.seq_num) && ((source->param.bt_common.seq_num + 256 - seq_num) <= 1)) ||
                           ((seq_num < source->param.bt_common.seq_num) && ((source->param.bt_common.seq_num - seq_num) <= 1))) {
                    /* The data packet is replicated */
                    ULL_LOG_E("[bt common][source]: The data packet is replicated, current seq_num = %u, latest seq_num = %u",
                                 2,
                                 source->param.bt_common.seq_num,
                                 seq_num);

                    /* get the next packet */
                    source->streamBuffer.AVMBufferInfo.ReadIndex = (source->streamBuffer.AVMBufferInfo.ReadIndex + 1) % source->streamBuffer.AVMBufferInfo.MemBlkNum;
                    source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.drop_frames += 1;

                    if (source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.process_frames > 1) {
                        source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.process_frames -= 1;
                    } else {
                        remain_samples = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port, 1) / sizeof(int16_t);
                        if (dongle_handle->fetch_flag != 0) {
                            total_frames = 1;
                        } else {
                            total_frames = 0;
                        }
                        if (remain_samples >= GAMING_MODE_VOICE_USB_FRAME_SAMPLES * total_frames) {
                            /* in this case, it means the only one packet is replicated but remain sample is enough, so we bypass the decoder */
                            /* set flag to indicate that needs to bypass */
                            dongle_handle->data_status = GAMING_MODE_DONGLE_UL_DATA_BYPASS_DECODER;
                            source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.process_frames -= 1;
                        } else {
                            /* in this case, it means only one packet is replicated and remain sample is not enough, so we need to do PLC here to prevent buffer empty */
                            /* set flag to indicate that there is the packet lost */
                            dongle_handle->data_status = GAMING_MODE_DONGLE_UL_DATA_PLC;
                            if (dongle_handle->play_en_status == 0) {
                                dongle_handle->data_status = GAMING_MODE_DONGLE_UL_DATA_BYPASS_DECODER;
                            }
                        }

                        ULL_LOG_E("[bt common][source]: The only one packet is replicated, %d", 1, dongle_handle->data_status);
                    }
                } else {
                    /* The data packet is dropped heavily */
                    /* Just do decode here, do not PLC */
                    dongle_handle->data_status = GAMING_MODE_DONGLE_UL_DATA_NORMAL;
                    ULL_LOG_E("[bt common][source]: The data packet is dropped heavily, current seq_num = %u, latest seq_num = %u",
                                 2,
                                 source->param.bt_common.seq_num,
                                 seq_num);
                    // AUDIO_ASSERT(0);
                    /* set seq_num to the newest number for keeping data is continuous */
                    source->param.bt_common.seq_num = seq_num;
                    /* drop this packet */
                    source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.drop_frames += 1;
                }
            }
        }

#ifdef AIR_GAMING_MODE_DONGLE_ECNR_ENABLE
_do_packet_process:
        /* Substact the gain byte */
        length -= 1;
#endif

        /* copy payload */
        if (dongle_handle->data_status == GAMING_MODE_DONGLE_UL_DATA_NORMAL) {
            /* The data packet is continuous or is dropped heavily */
            memcpy(dst_buf, src_buf + sizeof(bt_common_gaming_mode_voice_frame_header_t) + 1, length);

#ifdef AIR_GAMING_MODE_DONGLE_ECNR_ENABLE
            /* Get the Post EC gain from headset side */
            g_post_ec_gain = *(src_buf + sizeof(bt_common_gaming_mode_voice_frame_header_t) + 1 + length);
            ULL_LOG_I("[AEC] receive PostEC_Gain %d, length %d", 2, g_post_ec_gain, length);
#endif

            /* set bt clock for latency debug */
            dongle_handle->bt_clk = frame_header.anchor;

#ifdef AIR_AUDIO_DUMP_ENABLE
#if GAMING_MODE_VOICE_DONGLE_DEBUG_DUMP
            LOG_AUDIO_DUMP(dst_buf, length, VOICE_TX_MIC_0);
#endif /* GAMING_MODE_VOICE_DONGLE_DEBUG_DUMP */
#endif /* AIR_AUDIO_DUMP_ENABLE */
        } else if (dongle_handle->data_status == GAMING_MODE_DONGLE_UL_DATA_BYPASS_DECODER) {
            /* there is drop requirment */
            *(dst_buf + 0) = 0;
            *(dst_buf + 1) = 0;
            *(dst_buf + 2) = 0xa5;
            *(dst_buf + 3) = 0xa5;

            /* set bt clock for latency debug */
            dongle_handle->bt_clk = 0;
        } else {
            /* there is the packet lost */
            /* set zero to the first four byte of the dst_buf to indicate there is the packet lost */
            *(dst_buf + 0) = 0;
            *(dst_buf + 1) = 0;
            *(dst_buf + 2) = 0;
            *(dst_buf + 3) = 0;

            /* set bt clock for latency debug */
            dongle_handle->bt_clk = 0;

#ifdef AIR_AUDIO_DUMP_ENABLE
#if GAMING_MODE_VOICE_DONGLE_DEBUG_DUMP
            LOG_AUDIO_DUMP(dst_buf, length, VOICE_TX_MIC_0);
#endif /* GAMING_MODE_VOICE_DONGLE_DEBUG_DUMP */
#endif /* AIR_AUDIO_DUMP_ENABLE */
        }

        /* update stream status */
        stream = DSP_Streaming_Get(source, source->transform->sink);
        stream->callback.EntryPara.in_size = length;
        stream->callback.EntryPara.in_channel_num = 1;

#ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
        /* do clock skew if the play is started and there is new data */
        if ((dongle_handle->play_en_status != 0) && (dongle_handle->data_status != GAMING_MODE_DONGLE_UL_DATA_BYPASS_DECODER)) {
            /* get remain samples */
            remain_samples = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port, 1) / sizeof(int16_t);
            if (dongle_handle->fetch_flag != 0) {
                total_frames = 1;
                frame_index = dongle_handle->frame_index;
            } else {
                total_frames = 0;
                frame_index = (dongle_handle->frame_index + GAMING_MODE_VOICE_FRAME_INDEX_MAX - 1) % GAMING_MODE_VOICE_FRAME_INDEX_MAX;
            }
            /* calculator compensatory samples */
            compensatory_samples = (int32_t)remain_samples + GAMING_MODE_VOICE_BT_FRAME_SAMPLES - GAMING_MODE_VOICE_USB_FRAME_SAMPLES * total_frames;
            dongle_handle->compen_samples = 0;
            if (compensatory_samples > (int32_t)(dongle_handle->buffer_watermark + (GAMING_MODE_VOICE_FRAME_INDEX_MAX - 1 - frame_index)*GAMING_MODE_VOICE_USB_FRAME_SAMPLES)) {
                if (dongle_handle->clk_skew_count > -10) {
                    dongle_handle->clk_skew_count = dongle_handle->clk_skew_count - 1;
                    if ((dongle_handle->clk_skew_count > 0) && (dongle_handle->predicted_compen_times != 0)) {
                        dongle_handle->clk_skew_count = dongle_handle->clk_skew_count + 1;
                        dongle_handle->compen_samples = 1;
                        dongle_handle->predicted_compen_times -= 1;
                    } else if (dongle_handle->clk_skew_count <= -10) {
                        dongle_handle->predicted_compen_times = GAMING_MODE_VOICE_USB_FRAME_SAMPLES * dongle_handle->compensation_mode;
                    }
                }
                if (dongle_handle->clk_skew_count <= -10) {
                    /* we will do compensatory till the buffer water is dismatch */
                    dongle_handle->compen_samples = -1;
                    dongle_handle->predicted_compen_times -= 1;
                    if (dongle_handle->predicted_compen_times < 0) {
                        dongle_handle->predicted_compen_count += 1;
                        if (dongle_handle->predicted_compen_count > 8) {
                            /* in here, it means the current compensatroy mode is too slow, so we need to use the more aggressive compensatroy mode */
                            dongle_handle->compensation_mode = SW_CLK_SKEW_COMPENSATION_1_SAMPLE_IN_1_FRAME;
                            stream_function_sw_clk_skew_configure_compensation_mode(dongle_handle->clk_skew_port, dongle_handle->compensation_mode);
                            dongle_handle->predicted_compen_times = compensatory_samples - (int32_t)(dongle_handle->buffer_watermark + (GAMING_MODE_VOICE_FRAME_INDEX_MAX - 1 - frame_index) * GAMING_MODE_VOICE_USB_FRAME_SAMPLES);
                        } else {
                            dongle_handle->compensation_mode = SW_CLK_SKEW_COMPENSATION_1_SAMPLE_IN_8_FRAME;
                            stream_function_sw_clk_skew_configure_compensation_mode(dongle_handle->clk_skew_port, dongle_handle->compensation_mode);
                            dongle_handle->predicted_compen_times = (int32_t)(dongle_handle->buffer_watermark + (GAMING_MODE_VOICE_FRAME_INDEX_MAX - 1 - frame_index) * GAMING_MODE_VOICE_USB_FRAME_SAMPLES) - compensatory_samples;
                        }
                    }
                }
            } else if (compensatory_samples < (int32_t)(dongle_handle->buffer_watermark + (GAMING_MODE_VOICE_FRAME_INDEX_MAX - 1 - frame_index)*GAMING_MODE_VOICE_USB_FRAME_SAMPLES)) {
                if (dongle_handle->clk_skew_count < 10) {
                    dongle_handle->clk_skew_count = dongle_handle->clk_skew_count + 1;
                    if ((dongle_handle->clk_skew_count < 0) && (dongle_handle->predicted_compen_times != 0)) {
                        dongle_handle->clk_skew_count = dongle_handle->clk_skew_count - 1;
                        dongle_handle->compen_samples = -1;
                        dongle_handle->predicted_compen_times -= 1;
                    } else if (dongle_handle->clk_skew_count >= 10) {
                        dongle_handle->predicted_compen_times = GAMING_MODE_VOICE_USB_FRAME_SAMPLES * dongle_handle->compensation_mode;
                    }
                }
                if (dongle_handle->clk_skew_count >= 10) {
                    /* we will do compensatory till the buffer water is dismatch */
                    dongle_handle->compen_samples = 1;
                    dongle_handle->predicted_compen_times -= 1;
                    if (dongle_handle->predicted_compen_times < 0) {
                        dongle_handle->predicted_compen_count += 1;
                        if (dongle_handle->predicted_compen_count > 8) {
                            /* in here, it means the current compensatroy mode is too slow, so we need to use the more aggressive compensatroy mode  */
                            dongle_handle->compensation_mode = SW_CLK_SKEW_COMPENSATION_1_SAMPLE_IN_1_FRAME;
                            stream_function_sw_clk_skew_configure_compensation_mode(dongle_handle->clk_skew_port, dongle_handle->compensation_mode);
                            dongle_handle->predicted_compen_times = (int32_t)(dongle_handle->buffer_watermark + (GAMING_MODE_VOICE_FRAME_INDEX_MAX - 1 - frame_index) * GAMING_MODE_VOICE_USB_FRAME_SAMPLES) - compensatory_samples;
                        } else {
                            dongle_handle->compensation_mode = SW_CLK_SKEW_COMPENSATION_1_SAMPLE_IN_8_FRAME;
                            stream_function_sw_clk_skew_configure_compensation_mode(dongle_handle->clk_skew_port, dongle_handle->compensation_mode);
                            dongle_handle->predicted_compen_times = (int32_t)(dongle_handle->buffer_watermark + (GAMING_MODE_VOICE_FRAME_INDEX_MAX - 1 - frame_index) * GAMING_MODE_VOICE_USB_FRAME_SAMPLES) - compensatory_samples;
                        }
                    }
                }
            } else {
                dongle_handle->clk_skew_count = 0;
                dongle_handle->compen_samples = 0;
                dongle_handle->predicted_compen_times = 0;
                dongle_handle->predicted_compen_count = 0;
                dongle_handle->compensation_mode = SW_CLK_SKEW_COMPENSATION_1_SAMPLE_IN_8_FRAME;
                stream_function_sw_clk_skew_configure_compensation_mode(dongle_handle->clk_skew_port, dongle_handle->compensation_mode);
            }
            /* reset fetch frame count */
            dongle_handle->frame_count = 0;
        } else {
            dongle_handle->compen_samples = 0;
        }
        /* do compensatory */
        stream_function_sw_clk_skew_configure_compensation_samples(dongle_handle->clk_skew_port, dongle_handle->compen_samples);
#endif /* AIR_SOFTWARE_CLK_SKEW_ENABLE */
    } else {
        /* there is drop requirment */
        *(dst_buf + 0) = 0;
        *(dst_buf + 1) = 0;
        *(dst_buf + 2) = 0xa5;
        *(dst_buf + 3) = 0xa5;

        /* update stream status */
        stream = DSP_Streaming_Get(source, source->transform->sink);
        stream->callback.EntryPara.in_size = 0;
        stream->callback.EntryPara.in_channel_num = 1;
    }

    /* config output */
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (source->scenario_type != AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_DONGLE_LINE_OUT) {
        if (dongle_handle->fetch_flag != 0) {
    #ifdef AIR_SOFTWARE_BUFFER_ENABLE
            if (dongle_handle->play_en_status != 0) {
                /* only output 2.5ms*16K data to usb at every ccni interrupt trigger after the play is started*/
                stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 1, GAMING_MODE_VOICE_USB_FRAME_SAMPLES * sizeof(int16_t));
                stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 2, GAMING_MODE_VOICE_USB_FRAME_SAMPLES * sizeof(int16_t));
    #ifdef AIR_GAMING_MODE_DONGLE_ECNR_ENABLE
                uint32_t remain_samples1 = stream_function_sw_buffer_get_channel_used_size(dongle_handle->ecnr_out_buffer_port, 1) / sizeof(int16_t);
                uint32_t remain_samples2 = stream_function_sw_buffer_get_channel_used_size(dongle_handle->ecnr_in_buffer_port, 1) / sizeof(int16_t);
                if (dongle_handle->data_status != GAMING_MODE_DONGLE_UL_DATA_BYPASS_DECODER) {
                    if (remain_samples2 == 0) {
                        /* after this time process, there is 7.5ms data in ecnr_in_buffer and there is no data in ecnr_out_buffer */
                        stream_function_sw_buffer_config_channel_output_size(dongle_handle->ecnr_out_buffer_port, 1, remain_samples1 * sizeof(int16_t));
                        stream_function_sw_buffer_config_channel_output_size(dongle_handle->ecnr_out_buffer_port, 2, remain_samples1 * sizeof(int16_t));
                    } else {
                        /* after this time process, there is no data in ecnr_in_buffer and there is 7.5ms data in ecnr_out_buffer */
                        stream_function_sw_buffer_config_channel_output_size(dongle_handle->ecnr_out_buffer_port, 1, GAMING_MODE_VOICE_BT_FRAME_SAMPLES * sizeof(int16_t));
                        stream_function_sw_buffer_config_channel_output_size(dongle_handle->ecnr_out_buffer_port, 2, GAMING_MODE_VOICE_BT_FRAME_SAMPLES * sizeof(int16_t));
                    }
                } else {
                    /* do not output data in ecnr_out_buffer */
                    stream_function_sw_buffer_config_channel_output_size(dongle_handle->ecnr_out_buffer_port, 1, 0);
                    stream_function_sw_buffer_config_channel_output_size(dongle_handle->ecnr_out_buffer_port, 2, 0);
                }
    #endif /* AIR_GAMING_MODE_DONGLE_ECNR_ENABLE */
                dongle_handle->buffer_output_size = GAMING_MODE_VOICE_USB_FRAME_SAMPLES;
                dongle_handle->frame_count += 1;
                dongle_handle->frame_index = (dongle_handle->frame_index + 1) % GAMING_MODE_VOICE_FRAME_INDEX_MAX;

                /* decrease fetch flag */
                dongle_handle->fetch_flag -= 1;
            } else {
                /* do not output data to usb at every ccni interrupt trigger because the play is not started*/
                stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 1, 0);
                stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 2, 0);
    #ifdef AIR_GAMING_MODE_DONGLE_ECNR_ENABLE
                uint32_t remain_samples1 = stream_function_sw_buffer_get_channel_used_size(dongle_handle->ecnr_out_buffer_port, 1) / sizeof(int16_t);
                uint32_t remain_samples2 = stream_function_sw_buffer_get_channel_used_size(dongle_handle->ecnr_in_buffer_port, 1) / sizeof(int16_t);
                if (dongle_handle->data_status != GAMING_MODE_DONGLE_UL_DATA_BYPASS_DECODER) {
                    if (remain_samples2 == 0) {
                        /* after this time process, there is 7.5ms data in ecnr_in_buffer and there is no data in ecnr_out_buffer */
                        stream_function_sw_buffer_config_channel_output_size(dongle_handle->ecnr_out_buffer_port, 1, remain_samples1 * sizeof(int16_t));
                        stream_function_sw_buffer_config_channel_output_size(dongle_handle->ecnr_out_buffer_port, 2, remain_samples1 * sizeof(int16_t));
                    } else {
                        /* after this time process, there is no data in ecnr_in_buffer and there is 7.5ms data in ecnr_out_buffer */
                        stream_function_sw_buffer_config_channel_output_size(dongle_handle->ecnr_out_buffer_port, 1, GAMING_MODE_VOICE_BT_FRAME_SAMPLES * sizeof(int16_t));
                        stream_function_sw_buffer_config_channel_output_size(dongle_handle->ecnr_out_buffer_port, 2, GAMING_MODE_VOICE_BT_FRAME_SAMPLES * sizeof(int16_t));
                    }
                } else {
                    /* do not output data in ecnr_out_buffer */
                    stream_function_sw_buffer_config_channel_output_size(dongle_handle->ecnr_out_buffer_port, 1, 0);
                    stream_function_sw_buffer_config_channel_output_size(dongle_handle->ecnr_out_buffer_port, 2, 0);
                }
    #endif /* AIR_GAMING_MODE_DONGLE_ECNR_ENABLE */
                dongle_handle->buffer_output_size = 0;

                /* reset fetch flag */
                dongle_handle->fetch_flag = 0;
            }
    #endif /* AIR_SOFTWARE_BUFFER_ENABLE */
        } else {
    #ifdef AIR_SOFTWARE_BUFFER_ENABLE
            /* keep these data in buffer and not output them to usb */
            stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 1, 0);
            stream_function_sw_buffer_config_channel_output_size(dongle_handle->buffer_port, 2, 0);
    #ifdef AIR_GAMING_MODE_DONGLE_ECNR_ENABLE
            uint32_t remain_samples1 = stream_function_sw_buffer_get_channel_used_size(dongle_handle->ecnr_out_buffer_port, 1) / sizeof(int16_t);
            uint32_t remain_samples2 = stream_function_sw_buffer_get_channel_used_size(dongle_handle->ecnr_in_buffer_port, 1) / sizeof(int16_t);
            if (dongle_handle->data_status != GAMING_MODE_DONGLE_UL_DATA_BYPASS_DECODER) {
                if (remain_samples2 == 0) {
                    /* after this time process, there is 7.5ms data in ecnr_in_buffer and there is no data in ecnr_out_buffer */
                    stream_function_sw_buffer_config_channel_output_size(dongle_handle->ecnr_out_buffer_port, 1, remain_samples1 * sizeof(int16_t));
                    stream_function_sw_buffer_config_channel_output_size(dongle_handle->ecnr_out_buffer_port, 2, remain_samples1 * sizeof(int16_t));
                } else {
                    /* after this time process, there is no data in ecnr_in_buffer and there is 7.5ms data in ecnr_out_buffer */
                    stream_function_sw_buffer_config_channel_output_size(dongle_handle->ecnr_out_buffer_port, 1, GAMING_MODE_VOICE_BT_FRAME_SAMPLES * sizeof(int16_t));
                    stream_function_sw_buffer_config_channel_output_size(dongle_handle->ecnr_out_buffer_port, 2, GAMING_MODE_VOICE_BT_FRAME_SAMPLES * sizeof(int16_t));
                }
            } else {
                /* do not output data in ecnr_out_buffer */
                stream_function_sw_buffer_config_channel_output_size(dongle_handle->ecnr_out_buffer_port, 1, 0);
                stream_function_sw_buffer_config_channel_output_size(dongle_handle->ecnr_out_buffer_port, 2, 0);
            }
    #endif /* AIR_GAMING_MODE_DONGLE_ECNR_ENABLE */
            dongle_handle->buffer_output_size = 0;
    #endif /* AIR_SOFTWARE_BUFFER_ENABLE */
        }
    } else {
        dongle_handle->fetch_flag = 0; // clear fetch flag
        if ((dongle_handle->bypass_source) ) { // || (!dongle_handle->play_en_status)
            /* there is drop requirment */
            /* Note: stream_codec_modify_output_size will be 0, we should modify it to right value in SW Mixer preprocesscallback. */
            *(dst_buf+0) = 0;
            *(dst_buf+1) = 0;
            *(dst_buf+2) = 0xa5;
            *(dst_buf+3) = 0xa5;
        }
    }
    hal_nvic_restore_interrupt_mask(saved_mask);

#if GAMING_MODE_VOICE_DONGLE_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    ULL_LOG_I("[bt common][source][copy_payload]: %u, %u, %u, %d, %u, %d", 6,
                 source->param.bt_common.seq_num,
                 source->streamBuffer.AVMBufferInfo.ReadIndex,
                 source->streamBuffer.AVMBufferInfo.WriteIndex,
                 length,
                 current_timestamp,
                 hal_nvic_query_exception_number());
#endif /* GAMING_MODE_VOICE_DONGLE_DEBUG_LOG */

    return length;
}

bool gaming_mode_dongle_ul_source_get_new_read_offset(SOURCE source, U32 amount, U32 *new_read_offset)
{
    gaming_mode_dongle_ul_handle_t *dongle_handle = (gaming_mode_dongle_ul_handle_t *)(source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle);
    UNUSED(amount);

    if (dongle_handle->stream_status == GAMING_MODE_UL_STREAM_RUNNING) {
        /* there is no the packet lost, so needs to update the read offset */
        *new_read_offset = (source->streamBuffer.AVMBufferInfo.ReadIndex + source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.drop_frames) % source->streamBuffer.AVMBufferInfo.MemBlkNum;
    } else {
        /* do not update read index */
        *new_read_offset = source->streamBuffer.AVMBufferInfo.ReadIndex;
    }

    return true;
}

void gaming_mode_dongle_ul_source_drop_postprocess(SOURCE source, uint32_t amount)
{
    gaming_mode_dongle_ul_handle_t *dongle_handle = (gaming_mode_dongle_ul_handle_t *)(source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle);
    uint32_t remain_samples = stream_function_sw_buffer_get_channel_used_size(dongle_handle->buffer_port, 1) / sizeof(int16_t);
    UNUSED(amount);

    /* get current bt clock */
    extern VOID MCE_GetBtClk(BTCLK * pCurrCLK, BTPHASE * pCurrPhase, BT_CLOCK_OFFSET_SCENARIO type);
    uint32_t bt_clk;
    uint16_t bt_phase;
    MCE_GetBtClk((BTCLK *)&bt_clk, (BTPHASE *)&bt_phase, BT_CLK_Offset);

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&gaming_mode_voice_usb_out_gpt_count);

#ifdef AIR_GAMING_MODE_DONGLE_ECNR_ENABLE
    uint32_t remain_samples1 = stream_function_sw_buffer_get_channel_used_size(dongle_handle->ecnr_out_buffer_port, 1) / sizeof(int16_t);
    uint32_t remain_samples2 = stream_function_sw_buffer_get_channel_used_size(dongle_handle->ecnr_in_buffer_port, 1) / sizeof(int16_t);
    ULL_LOG_I("[bt common][source][drop_postprocess]: %u, %u, %u, %u, %u, %u, %u, %u, %u, %d, %d, %u, %u, %u, %d, %d, 0x%x, 0x%x, 0x%x, 0x%x", 20,
                 dongle_handle->stream_status,
                 dongle_handle->data_status,
                 source->param.bt_common.seq_num,
                 dongle_handle->fetch_flag,
                 source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.process_frames,
                 source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.drop_frames,
                 dongle_handle->buffer_output_size,
                 (dongle_handle->frame_index + GAMING_MODE_VOICE_FRAME_INDEX_MAX - 1) % GAMING_MODE_VOICE_FRAME_INDEX_MAX,
                 dongle_handle->frame_count,
                 dongle_handle->clk_skew_count,
                 dongle_handle->compen_samples,
                 remain_samples,
                 remain_samples1,
                 remain_samples2,
                 dongle_handle->predicted_compen_times,
                 dongle_handle->compensation_mode,
                 dongle_handle->bt_clk,
                 bt_clk,
                 dongle_handle->data_in_gpt_count,
                 gaming_mode_voice_usb_out_gpt_count);
#else
    ULL_LOG_I("[bt common][source][drop_postprocess]: %u, %u, %u, %u, %u, %u, %u, %u, %u, %d, %d, %u, %d, %d, 0x%x, 0x%x, 0x%x, 0x%x", 18,
                 dongle_handle->stream_status,
                 dongle_handle->data_status,
                 source->param.bt_common.seq_num,
                 dongle_handle->fetch_flag,
                 source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.process_frames,
                 source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.drop_frames,
                 dongle_handle->buffer_output_size,
                 (dongle_handle->frame_index + GAMING_MODE_VOICE_FRAME_INDEX_MAX - 1) % GAMING_MODE_VOICE_FRAME_INDEX_MAX,
                 dongle_handle->frame_count,
                 dongle_handle->clk_skew_count,
                 dongle_handle->compen_samples,
                 remain_samples,
                 dongle_handle->predicted_compen_times,
                 dongle_handle->compensation_mode,
                 dongle_handle->bt_clk,
                 bt_clk,
                 dongle_handle->data_in_gpt_count,
                 gaming_mode_voice_usb_out_gpt_count);
#endif

    /* update seq_num when the stream is running */
    if ((dongle_handle->stream_status == GAMING_MODE_UL_STREAM_RUNNING) &&
        (dongle_handle->data_status == GAMING_MODE_DONGLE_UL_DATA_NORMAL)) {
        source->param.bt_common.seq_num += 1;
        source->param.bt_common.seq_num = source->param.bt_common.seq_num & 0xff;
    }

#if GAMING_MODE_VOICE_DONGLE_DEBUG_LOG
    /* get the latest share buffer info */
    extern void bt_common_share_information_fetch(SOURCE source, SINK sink);
    bt_common_share_information_fetch(source, NULL);
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    ULL_LOG_I("[bt common][source][drop_postprocess]: %u, %u, %u, %u, %d", 5,
                 source->param.bt_common.seq_num,
                 source->streamBuffer.AVMBufferInfo.ReadIndex,
                 source->streamBuffer.AVMBufferInfo.WriteIndex,
                 current_timestamp,
                 hal_nvic_query_exception_number());
#endif /* GAMING_MODE_VOICE_DONGLE_DEBUG_LOG */
}

bool gaming_mode_dongle_ul_source_close(SOURCE source)
{
    if (source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.timer.handle) {
        hal_gpt_sw_stop_timer_us(source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.timer.handle);
        hal_gpt_sw_free_timer(source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.timer.handle);
        source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.timer.handle = 0;
    }

#if GAMING_MODE_VOICE_DONGLE_DEBUG_LOG
    uint32_t current_timestamp = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &current_timestamp);
    ULL_LOG_I("[bt common][source][close]: %u, %d", 2, current_timestamp, hal_nvic_query_exception_number());
#endif /* GAMING_MODE_VOICE_DONGLE_DEBUG_LOG */

    return true;
}

ATTR_TEXT_IN_IRAM_LEVEL_1 bool gaming_mode_dongle_ul_sink_get_avail_size(SINK sink, uint32_t *avail_size)
{
    n9_dsp_share_info_t *share_buffer_info = &(sink->streamBuffer.ShareBufferInfo);
    if (share_buffer_info->read_offset < share_buffer_info->write_offset) {
        /* ul normal case */
        *avail_size = share_buffer_info->sub_info.block_info.block_size * share_buffer_info->sub_info.block_info.block_num
                      - share_buffer_info->write_offset + share_buffer_info->read_offset;
    } else if (share_buffer_info->read_offset == share_buffer_info->write_offset) {
        if (share_buffer_info->bBufferIsFull == true) {
            /* buffer is full, so ReadOffset == WriteOffset */
            *avail_size = 0;
        } else {
            /* buffer is empty, so ReadOffset == WriteOffset */
            *avail_size = share_buffer_info->length;
        }
    } else {
        /* buffer wrapper case */
        *avail_size = share_buffer_info->read_offset - share_buffer_info->write_offset;
    }
    /* change avail_size to max_payload_size if the avail_size is too large */
    if (*avail_size > share_buffer_info->sub_info.block_info.block_size) {
        *avail_size = share_buffer_info->sub_info.block_info.block_size;
    }

    return true;
}

uint32_t gaming_mode_dongle_ul_sink_copy_payload(SINK sink, uint8_t *src_buf, uint8_t *dst_buf, uint32_t length)
{
    TRANSFORM transform =  sink->transform;
    DSP_STREAMING_PARA_PTR stream = NULL;
    UNUSED(src_buf);

    // ULL_LOG_I("[audio transmitter][source]: length = %d", 1, length);
    // length = length - sizeof(audio_transmitter_frame_header_t) - 1;

    /* copy usb auido data into the share buffer */
    // memcpy(dst_buf, src_buf, length);
    stream = DSP_Streaming_Get(transform->source, sink);

    if ((length % 2) != 0) {
        AUDIO_ASSERT(0);
    }
    // DSP_D2I_BufferCopy_16bit((U16 *)dst_buf,
    //                             stream->callback.EntryPara.out_ptr[0],
    //                             stream->callback.EntryPara.out_ptr[1],
    //                             length / 2);
    memcpy(dst_buf, stream->callback.EntryPara.out_ptr[0], length);

#ifdef AIR_AUDIO_DUMP_ENABLE
#if GAMING_MODE_VOICE_DONGLE_DEBUG_DUMP
    LOG_AUDIO_DUMP(dst_buf, length, VOICE_TX_MIC_1);
#endif /* GAMING_MODE_VOICE_DONGLE_DEBUG_DUMP */
#endif /* AIR_AUDIO_DUMP_ENABLE */

    // length = length * 2;

    return length;
}

bool gaming_mode_dongle_ul_sink_close(SINK sink)
{
    UNUSED(sink);

    /* application deinit */
    return true;
}

#if (defined AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE)
/****************************************************************************************************************************************************/
/*                                                       ULL Dongle AFE IN(I2S IN/LINE IN)                                                          */
/****************************************************************************************************************************************************/
void gaming_mode_dongle_dl_init_afe_in(SOURCE source, SINK sink, uint32_t sub_id)
{
    gaming_mode_dongle_dl_handle_t *dongle_handle;

    /* choose high priority stream */
    source->taskId = DHP_TASK_ID;
    sink->taskid   = DHP_TASK_ID;

    /* register ccni handler */
    audio_transmitter_register_isr_handler(0, gaming_mode_dongle_dl_ccni_handler);

    /* get handle for application config */
    dongle_handle = gaming_mode_dongle_get_dl_handle(source);

    #ifdef AIR_SOFTWARE_GAIN_ENABLE
    /* sw gain config */
    int32_t default_gain;
    sw_gain_config_t default_config;
    default_config.resolution = RESOLUTION_16BIT;
    default_config.target_gain = -384;
    default_config.up_step = 1;
    default_config.up_samples_per_step = 48;
    default_config.down_step = -1;
    default_config.down_samples_per_step = 48;
    dongle_handle->gain_port = stream_function_sw_gain_get_port(source);
    stream_function_sw_gain_init(dongle_handle->gain_port, 2, &default_config);
    default_gain = -384;
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 1, default_gain);
    ULL_LOG_I("[audio transmitter][config]scenario %d-%d change channel %d gain to %d*0.25dB\r\n", 4,
                source->scenario_type,
                sink->scenario_type,
                1,
                default_gain);
    default_gain = -384;
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2, default_gain);
    ULL_LOG_I("[audio transmitter][config]scenario %d-%d change channel %d gain to %d*0.25dB\r\n", 4,
                source->scenario_type,
                sink->scenario_type,
                2,
                default_gain);
    #endif /* AIR_SOFTWARE_GAIN_ENABLE */

    #ifdef AIR_SOFTWARE_MIXER_ENABLE
    /* sw mixer config */
    sw_mixer_callback_config_t       callback_config;
    sw_mixer_input_channel_config_t  in_ch_config;
    sw_mixer_output_channel_config_t out_ch_config;

    stream_function_sw_mixer_init(SW_MIXER_PORT_0);

    callback_config.preprocess_callback = NULL;
    extern void gaming_mode_dongle_mixer_postcallback(sw_mixer_member_t *member, void *para, uint32_t *out_frame_size);
    callback_config.postprocess_callback = gaming_mode_dongle_mixer_postcallback;
    in_ch_config.total_channel_number = 2;
    in_ch_config.resolution = RESOLUTION_16BIT;
    in_ch_config.input_mode = SW_MIXER_CHANNEL_MODE_FORCED_ENOUGH;
    in_ch_config.buffer_size = sizeof(int16_t) * 120; // 2.5ms * 48K
    out_ch_config.total_channel_number = 2;
    out_ch_config.resolution = RESOLUTION_16BIT;
    dongle_handle->mixer_member= stream_function_sw_mixer_member_create((void *)source,
                                                                        SW_MIXER_MEMBER_MODE_NO_BYPASS,
                                                                        &callback_config,
                                                                        &in_ch_config,
                                                                        &out_ch_config);

    if (dongle_handle->index == 1) {
        /* it is the first dl stream */
        stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, dongle_handle->mixer_member, true);
    } else {
        /* it is not the first dl stream, so we need to disconnect default connection */
        stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, dongle_handle->mixer_member, false);
    }
    #endif /* AIR_SOFTWARE_MIXER_ENABLE */

    #ifdef AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE
        g_vdma_start_flag = true; // It is only useful for i2s slave in dma mode.
    #endif
#ifdef HAL_SLEEP_MANAGER_ENABLED
    /* lock sleep because sleep wake-up time will consume the stream process time */
    hal_sleep_manager_lock_sleep(SLEEP_LOCK_DONGLE_AUDIO);
#endif

    dongle_handle->stream_status = GAMING_MODE_STREAM_INIT;
    ULL_LOG_I("[audio transmitter][afe in] stream init, [%d]-[%d]", 2,
        source->scenario_type,
        sink->scenario_type
        );
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void gaming_mode_dongle_dl_deinit_afe_in(SOURCE source, SINK sink)
{
    //uint32_t saved_mask;
    gaming_mode_dongle_dl_handle_t *dongle_handle = NULL;
    UNUSED(sink);

    /* application deinit */
    //hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    dongle_handle = gaming_mode_dongle_query_dl_handle(source);

    /* unregister ccni handler */
    if (dongle_handle->total_number == 1) {
        audio_transmitter_unregister_isr_handler(0, gaming_mode_dongle_dl_ccni_handler);
    }

    #ifdef AIR_SOFTWARE_GAIN_ENABLE
    stream_function_sw_gain_deinit(dongle_handle->gain_port);
    #endif /* AIR_SOFTWARE_GAIN_ENABLE */

    #ifdef AIR_SOFTWARE_MIXER_ENABLE
    stream_function_sw_mixer_member_unregister(SW_MIXER_PORT_0, dongle_handle->mixer_member);
    stream_function_sw_mixer_member_delete(dongle_handle->mixer_member);
    stream_function_sw_mixer_deinit(SW_MIXER_PORT_0);
    #endif /* AIR_SOFTWARE_MIXER_ENABLE */

    /* release handle */
    gaming_mode_dongle_release_dl_handle(dongle_handle);

    //hal_nvic_restore_interrupt_mask(saved_mask);

#ifdef HAL_SLEEP_MANAGER_ENABLED
    /* unlock sleep */
    hal_sleep_manager_unlock_sleep(SLEEP_LOCK_DONGLE_AUDIO);
#endif
    ULL_LOG_I("[audio transmitter][afe in] stream deinit, [%d]-[%d]", 2,
        source->scenario_type,
        sink->scenario_type
        );
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void gaming_mode_dongle_dl_stop_afe_in(SOURCE source, SINK sink)
{
    uint32_t gpt_count;
    uint32_t saved_mask;
    gaming_mode_dongle_dl_handle_t *dongle_handle = NULL;

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    dongle_handle = gaming_mode_dongle_query_dl_handle(source);

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);

    dongle_handle->stream_status = GAMING_MODE_STREAM_STOPED;
    hal_nvic_restore_interrupt_mask(saved_mask);

    ULL_LOG_I("[audio transmitter][afe in] stream stop, [%d]-[%d], gpt count = 0x%x", 3,
                source->scenario_type,
                sink->scenario_type,
                gpt_count
                );
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void gaming_mode_dongle_dl_start_afe_in(SOURCE source, SINK sink)
{
    uint32_t gpt_count;
    uint32_t saved_mask;
    hal_audio_memory_parameter_t *mem_handle = &source->param.audio.mem_handle;
    hal_audio_agent_t agent;
    if ((mem_handle->memory_select != HAL_AUDIO_MEMORY_UL_SLAVE_DMA) &&
        (mem_handle->memory_select != HAL_AUDIO_MEMORY_UL_SLAVE_TDM)) {
        // interconn mode
        agent = hal_memory_convert_agent(mem_handle->memory_select);
        /* disable AFE irq here */
        hal_memory_set_irq_enable(agent, HAL_AUDIO_CONTROL_OFF);
    }

    gaming_mode_dongle_dl_handle_t *dongle_handle = NULL;
    hal_audio_trigger_start_parameter_t start_parameter;

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    dongle_handle = gaming_mode_dongle_query_dl_handle(source);

    /* set flag */
    source->param.audio.drop_redundant_data_at_first_time = true;

    /* enable afe agent here */
    if (source->scenario_type == AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_I2S_IN) {
    #ifndef AIR_I2S_SLAVE_ENABLE
        if ((mem_handle->memory_select != HAL_AUDIO_MEMORY_UL_SLAVE_DMA) &&
            (mem_handle->memory_select != HAL_AUDIO_MEMORY_UL_SLAVE_TDM)) {
            start_parameter.memory_select = mem_handle->memory_select;
            start_parameter.enable = true;
            hal_audio_set_value((hal_audio_set_value_parameter_t *)&start_parameter, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
            /* set agent regsiters' address */
            switch (agent) {
                case HAL_AUDIO_AGENT_MEMORY_VUL1:
                    dongle_handle->afe_vul_cur_addr  = AFE_VUL_CUR;
                    dongle_handle->afe_vul_base_addr = AFE_VUL_BASE;
                    break;
                case HAL_AUDIO_AGENT_MEMORY_VUL2:
                    dongle_handle->afe_vul_cur_addr  = AFE_VUL2_CUR;
                    dongle_handle->afe_vul_base_addr = AFE_VUL2_BASE;
                    break;
                case HAL_AUDIO_AGENT_MEMORY_VUL3:
                    dongle_handle->afe_vul_cur_addr  = AFE_VUL3_CUR;
                    dongle_handle->afe_vul_base_addr = AFE_VUL3_BASE;
                    break;
                default:
                    ULL_LOG_E("[ULL][I2S IN] ERROR: unknow agent = 0x%x", 1, agent);
                    AUDIO_ASSERT(0);
            }
            /* set prefill size 0 */
            //Source_Audio_BufferInfo_Rst(source, 0);
            source->streamBuffer.BufferInfo.ReadOffset  = 0;
            source->streamBuffer.BufferInfo.WriteOffset = 0;
            source->streamBuffer.BufferInfo.bBufferIsFull = false;
        } else
    #endif
        {
            /* DMA copy buffer to HWSRC1 input buffer */
            if (mem_handle->memory_select == HAL_AUDIO_MEMORY_UL_VUL2) {
                ULL_LOG_I("[audio transmitter][afe in] ERROR: start with HWSRC2, this data path is not support, please check memory select", 0);
                dongle_handle->afe_vul_cur_addr  = ASM2_CH01_OBUF_WRPNT; // HWSRC2 OWO
                dongle_handle->afe_vul_base_addr = ASM2_OBUF_SADR;       // HWSRC2 Base Addr
                AUDIO_ASSERT(0);
            } else {
                // ULL_LOG_I("[ULL][I2S IN] start with HWSRC1", 0);
                dongle_handle->afe_vul_cur_addr  = ASM_CH01_OBUF_WRPNT;  // HWSRC OWO
                dongle_handle->afe_vul_base_addr = ASM_OBUF_SADR;        // HWSRC Base Addr
            }
            source->streamBuffer.BufferInfo.ReadOffset  = 0;
            source->streamBuffer.BufferInfo.WriteOffset = 0;
            source->streamBuffer.BufferInfo.bBufferIsFull = false;
            #ifdef AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE
                g_vdma_start_flag = false; // waiting ccni irq start vdma
            #endif
        }
    } else {
        /* enable afe agent here */
        start_parameter.memory_select = mem_handle->memory_select;
        start_parameter.enable = true;
        hal_audio_set_value((hal_audio_set_value_parameter_t *)&start_parameter, HAL_AUDIO_SET_TRIGGER_MEMORY_START);

        /* set agent regsiters' address */
        switch (agent) {
            case HAL_AUDIO_AGENT_MEMORY_VUL1:
                dongle_handle->afe_vul_cur_addr  = AFE_VUL_CUR;
                dongle_handle->afe_vul_base_addr = AFE_VUL_BASE;
                break;

            case HAL_AUDIO_AGENT_MEMORY_VUL2:
                dongle_handle->afe_vul_cur_addr  = AFE_VUL2_CUR;
                dongle_handle->afe_vul_base_addr = AFE_VUL2_BASE;
                break;

            case HAL_AUDIO_AGENT_MEMORY_VUL3:
                dongle_handle->afe_vul_cur_addr  = AFE_VUL3_CUR;
                dongle_handle->afe_vul_base_addr = AFE_VUL3_BASE;
                break;

            default:
                ULL_LOG_E("[audio transmitter][afe in] stream start, %d-%d unknow agent = 0x%x", 3,
                    source->scenario_type,
                    sink->scenario_type,
                    agent
                    );
                AUDIO_ASSERT(0);
        }

        /* set prefill size 0 */
        source->streamBuffer.BufferInfo.WriteOffset = 0;
        source->streamBuffer.BufferInfo.ReadOffset  = 0;
    }

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);

    dongle_handle->stream_status = GAMING_MODE_STREAM_STARTED;
    hal_nvic_restore_interrupt_mask(saved_mask);

    ULL_LOG_I("[audio transmitter][afe in] stream start, %d-%d gpt count = 0x%x, cur_addr = 0x%x, base_addr = 0x%x, cur_ro = 0x%x, cur_base = 0x%x", 7,
                source->scenario_type,
                sink->scenario_type,
                gpt_count,
                dongle_handle->afe_vul_cur_addr,
                dongle_handle->afe_vul_base_addr,
                AFE_GET_REG(dongle_handle->afe_vul_cur_addr),
                AFE_GET_REG(dongle_handle->afe_vul_base_addr)
                );
}
#endif /* AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE || AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE */

#ifdef AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE
/****************************************************************************************************************************************************/
/*                                                               ULL Dongle AFE OUT                                                                 */
/****************************************************************************************************************************************************/
void gaming_mode_dongle_ul_init_afe_out(SOURCE source, SINK sink, bt_common_open_param_p bt_common_open_param, uint32_t sub_id)
{
    gaming_mode_dongle_ul_handle_t *dongle_handle;
    UNUSED(sink);
    /* choose high priority stream */
    // source->taskId = DHP_TASK_ID;
    // sink->taskid   = DHP_TASK_ID;

    /* get handle for application config */
    dongle_handle = gaming_mode_dongle_get_ul_handle(source);
    source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle = (void *)dongle_handle;
    /* latency default is 40ms */
    dongle_handle->latency_us = 40000;

    #ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    sw_clk_skew_config_t sw_clk_skew_config;
    /* clock skew config */
    dongle_handle->compen_samples = 0;
    dongle_handle->clk_skew_count = 0;
    dongle_handle->clk_skew_port = stream_function_sw_clk_skew_get_port(source);
    sw_clk_skew_config.channel = 2;
    sw_clk_skew_config.bits = 16;
    sw_clk_skew_config.order = C_Flp_Ord_5;
    sw_clk_skew_config.skew_io_mode = C_Skew_Oup;
    sw_clk_skew_config.skew_compensation_mode = SW_CLK_SKEW_COMPENSATION_1_SAMPLE_IN_8_FRAME;
    sw_clk_skew_config.skew_work_mode = SW_CLK_SKEW_CONTINUOUS;
    sw_clk_skew_config.max_output_size = 15*16*sizeof(int16_t); /* 15ms/16K buffer */
    sw_clk_skew_config.continuous_frame_size = 120*sizeof(int16_t); /* fixed 7.5ms/16K input */
    stream_function_sw_clk_skew_init(dongle_handle->clk_skew_port, &sw_clk_skew_config);
    #endif /* AIR_SOFTWARE_CLK_SKEW_ENABLE */

    #ifdef AIR_SOFTWARE_BUFFER_ENABLE
    sw_buffer_config_t buffer_config;
    buffer_config.mode = SW_BUFFER_MODE_FIXED_OUTPUT_LENGTH;
    buffer_config.total_channels = 2;
    buffer_config.watermark_max_size = 1440*sizeof(int16_t); /* 90ms * 16K */
    buffer_config.watermark_min_size = 0;
    buffer_config.output_size = 80 * sizeof(int16_t); /* 5ms * 16K */ /* line out irq period = 5ms */
    dongle_handle->buffer_port = stream_function_sw_buffer_get_port(source);
    stream_function_sw_buffer_init(dongle_handle->buffer_port, &buffer_config);
    /* set default 15ms audio buffer */
    dongle_handle->buffer_watermark = 240;
    #endif /* AIR_SOFTWARE_BUFFER_ENABLE */

    #ifdef AIR_SOFTWARE_MIXER_ENABLE
    /* sw mixer config */
    sw_mixer_callback_config_t       callback_config;
    sw_mixer_input_channel_config_t  in_ch_config;
    sw_mixer_output_channel_config_t out_ch_config;

    stream_function_sw_mixer_init(SW_MIXER_PORT_0);
    callback_config.preprocess_callback = NULL;
    callback_config.postprocess_callback = gaming_mode_dongle_ul_mixer_postcallback;
    in_ch_config.total_channel_number = 2;
    in_ch_config.resolution = RESOLUTION_16BIT;
    in_ch_config.input_mode = SW_MIXER_CHANNEL_MODE_OVERWRITE;
    in_ch_config.buffer_size = sizeof(int16_t) * 120; // 2.5ms * 48K
    out_ch_config.total_channel_number = 2;
    out_ch_config.resolution = RESOLUTION_16BIT;
    dongle_handle->mixer_member= stream_function_sw_mixer_member_create((void *)source,
                                                                        SW_MIXER_MEMBER_MODE_NO_BYPASS,
                                                                        &callback_config,
                                                                        &in_ch_config,
                                                                        &out_ch_config);
    if (dongle_handle->index == 1) {
        /* it is the first dl stream */
        stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, dongle_handle->mixer_member, true);
        // stream_function_sw_mixer_channel_connect(dongle_handle->mixer_member, 1, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, dongle_handle->mixer_member, 1);
        // stream_function_sw_mixer_channel_connect(dongle_handle->mixer_member, 2, SW_MIXER_CHANNEL_ATTRIBUTE_MAIN, dongle_handle->mixer_member, 2);
        // dual channels
        if ((g_line_out_buffer[0].buffer_base_pointer != NULL) || (g_line_out_buffer[1].buffer_base_pointer != NULL)) {
            ULL_LOG_I("[audio transmitter][afe out] %d-%d buffer addr error!", 2,
                source->scenario_type,
                sink->scenario_type
                );
            AUDIO_ASSERT(0);
        }
        g_line_out_buffer[0].buffer_base_pointer = pvPortMalloc(sizeof(uint8_t) * 4 * 160); // 20ms 16K
        g_line_out_buffer[1].buffer_base_pointer = pvPortMalloc(sizeof(uint8_t) * 4 * 160); // 20ms 16K
        if ((!g_line_out_buffer[0].buffer_base_pointer) || (!g_line_out_buffer[1].buffer_base_pointer)) {
            ULL_LOG_I("[audio transmitter][afe out] %d-%d malloc fail!", 2,
                source->scenario_type,
                sink->scenario_type
                );
            AUDIO_ASSERT(0);
        }
        g_line_out_buffer[0].write_pointer = 0;
        g_line_out_buffer[0].read_pointer = 0;
        g_line_out_buffer[0].buffer_byte_count = 4 * 160;

        g_line_out_buffer[1].write_pointer = 0;
        g_line_out_buffer[1].read_pointer = 0;
        g_line_out_buffer[1].buffer_byte_count = 4 * 160;
    } else {
        /* it is not the first dl stream, so we need to disconnect default connection */
        stream_function_sw_mixer_member_register(SW_MIXER_PORT_0, dongle_handle->mixer_member, false);
    }
    #endif /* AIR_SOFTWARE_MIXER_ENABLE */
    #ifdef AIR_SOFTWARE_GAIN_ENABLE
    /* configuare sw gain */
    int32_t default_gain;
    sw_gain_config_t default_config;
    default_config.resolution = RESOLUTION_16BIT;
    default_config.target_gain = bt_common_open_param->scenario_param.gaming_mode_param.gain_default_L;
    default_config.up_step = 25;
    default_config.up_samples_per_step = 16;
    default_config.down_step = -25;
    default_config.down_samples_per_step = 16;
    dongle_handle->gain_port = stream_function_sw_gain_get_port(source);
    stream_function_sw_gain_init(dongle_handle->gain_port, 2, &default_config);
    default_gain = bt_common_open_param->scenario_param.gaming_mode_param.gain_default_L;
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 1, default_gain);
    ULL_LOG_I("[audio transmitter][config]scenario %d-%d change channel %d gain to %d*0.25dB\r\n", 4,
                source->scenario_type,
                sink->scenario_type,
                1,
                default_gain
                );
    default_gain = bt_common_open_param->scenario_param.gaming_mode_param.gain_default_R;
    stream_function_sw_gain_configure_gain_target(dongle_handle->gain_port, 2, default_gain);
    ULL_LOG_I("[audio transmitter][config]scenario %d-%d change channel %d gain to %d*0.25dB\r\n", 4,
                source->scenario_type,
                sink->scenario_type,
                2,
                default_gain
                );
    #endif /* AIR_SOFTWARE_GAIN_ENABLE */

    #ifdef MTK_CELT_DEC_ENABLE
    if (bt_common_open_param->scenario_param.gaming_mode_param.codec_param.opus.bit_rate == 32000) {
        stream_codec_decoder_celt_set_input_frame_size(30);
        ULL_LOG_I("[audio transmitter][afe out] %d-%d codec frame size, %u\r\n", 3,
            source->scenario_type,
            sink->scenario_type,
            30
            );
    } else if (bt_common_open_param->scenario_param.gaming_mode_param.codec_param.opus.bit_rate == 50133) {
        stream_codec_decoder_celt_set_input_frame_size(47);
        ULL_LOG_I("[audio transmitter][afe out] %d-%d codec frame size, %u\r\n", 3,
            source->scenario_type,
            sink->scenario_type,
            47
            );
    } else {
        ULL_LOG_E("[audio transmitter][afe out] %d-%d error bitrate, %u\r\n", 3,
            source->scenario_type,
            sink->scenario_type,
            bt_common_open_param->scenario_param.gaming_mode_param.codec_param.opus.bit_rate
            );
        AUDIO_ASSERT(0);
    }
    #endif /* MTK_CELT_DEC_ENABLE */

#ifdef HAL_SLEEP_MANAGER_ENABLED
    /* lock sleep because sleep wake-up time will consume the stream process time */
    hal_sleep_manager_lock_sleep(SLEEP_LOCK_DONGLE_AUDIO);
#endif
    ULL_LOG_I("[audio transmitter][afe out] stream init, %d-%d", 2,
        source->scenario_type,
        sink->scenario_type
        );
}

void gaming_mode_dongle_ul_deinit_afe_out(SOURCE source, SINK sink)
{
    gaming_mode_dongle_ul_handle_t *dongle_handle = (gaming_mode_dongle_ul_handle_t *)(source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle);
    UNUSED(sink);

    /* application deinit */
    #ifdef AIR_SOFTWARE_CLK_SKEW_ENABLE
    stream_function_sw_clk_skew_deinit(dongle_handle->clk_skew_port);
    #endif /* AIR_SOFTWARE_CLK_SKEW_ENABLE */

    #ifdef AIR_SOFTWARE_BUFFER_ENABLE
    stream_function_sw_buffer_deinit(dongle_handle->buffer_port);
    #endif /* AIR_SOFTWARE_BUFFER_ENABLE */

    // #ifdef AIR_SOFTWARE_SRC_ENABLE
    // stream_function_sw_src_deinit(dongle_handle->src_port);
    // #endif /* AIR_SOFTWARE_SRC_ENABLE */
    #ifdef AIR_SOFTWARE_MIXER_ENABLE
    stream_function_sw_mixer_member_unregister(SW_MIXER_PORT_0, dongle_handle->mixer_member);
    stream_function_sw_mixer_member_delete(dongle_handle->mixer_member);
    stream_function_sw_mixer_deinit(SW_MIXER_PORT_0);
    #endif /* AIR_SOFTWARE_MIXER_ENABLE */
    if (gaming_mode_dongle_first_ul_handle->total_number == 1) {
        vPortFree(g_line_out_buffer[0].buffer_base_pointer);
        g_line_out_buffer[0].buffer_base_pointer = NULL;
        vPortFree(g_line_out_buffer[1].buffer_base_pointer);
        g_line_out_buffer[1].buffer_base_pointer = NULL;
    }
    #ifdef AIR_SOFTWARE_GAIN_ENABLE
    stream_function_sw_gain_deinit(dongle_handle->gain_port);
    #endif /* AIR_SOFTWARE_GAIN_ENABLE */

    /* release handle */
    gaming_mode_dongle_release_ul_handle(dongle_handle);
    source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle = NULL;

#ifdef HAL_SLEEP_MANAGER_ENABLED
    /* unlock sleep */
    hal_sleep_manager_unlock_sleep(SLEEP_LOCK_DONGLE_AUDIO);
#endif
    ULL_LOG_I("[audio transmitter][afe out] stream deinit, %d-%d", 2,
        source->scenario_type,
        sink->scenario_type
        );
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void gaming_mode_dongle_ul_start_afe_out(SOURCE source, SINK sink)
{
    uint32_t saved_mask = 0;
    gaming_mode_dongle_ul_handle_t *dongle_handle = (gaming_mode_dongle_ul_handle_t *)(source->param.bt_common.scenario_param.usb_out_broadcast_param.bt_in_param.handle);

    hal_nvic_save_and_set_interrupt_mask(&saved_mask);

    dongle_handle->fetch_flag = 0;
    dongle_handle->play_en_status = 0;
    dongle_handle->stream_status = GAMING_MODE_UL_STREAM_INIT;
    dongle_handle->frame_count = 0;
    dongle_handle->frame_index = 0;
    dongle_handle->compen_samples = 0;
    hal_nvic_restore_interrupt_mask(saved_mask);
    /* start gpt irq to trigger souce check */
    hal_gpt_status_t status = HAL_GPT_STATUS_OK;
    // status = hal_gpt_init(HAL_GPT_5);
    // status = hal_gpt_register_callback(HAL_GPT_5, gaming_mode_dongle_ul_afe_out_gpt_irq, NULL);
    // status = hal_gpt_start_timer_ms(HAL_GPT_5, 1, HAL_GPT_TIMER_TYPE_REPEAT);
    if (g_line_out_timer_handle == 0) {
        hal_gpt_sw_get_timer(&g_line_out_timer_handle);
    } else {
        ULL_LOG_I("[audio transmitter][afe out] gpt timer handle is abnormal %d", 1, g_line_out_timer_handle);
    }
    hal_gpt_sw_start_timer_us(g_line_out_timer_handle, 2500, (hal_gpt_callback_t)gaming_mode_dongle_ul_afe_out_gpt_irq, NULL);
    ULL_LOG_I("[audio transmitter][afe out] stream start, %d-%d", 2,
        source->scenario_type,
        sink->scenario_type
        );
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void gaming_mode_dongle_ul_afe_out_gpt_irq(void)
{
    DSP_MW_LOG_I("Clear buffer, gpt irq", 0);
    volatile SINK sink = Sink_blks[SINK_TYPE_AUDIO];
    volatile SOURCE source;
    AUDIO_PARAMETER *runtime = &sink->param.audio;
    gaming_mode_dongle_ul_handle_t *dongle_handle = gaming_mode_dongle_query_ul_handle_by_scenario_type(sink->scenario_type);
    if (dongle_handle != NULL) {
        if (dongle_handle->stream_status != GAMING_MODE_UL_STREAM_DEINIT) {
            uint32_t saved_mask;
            hal_nvic_save_and_set_interrupt_mask(&saved_mask);
            /* set fetch flag to trigger stream flow */
            dongle_handle->fetch_flag = 1;
            hal_nvic_restore_interrupt_mask(saved_mask);
            // check stream number
            extern gaming_mode_dongle_ul_handle_t *gaming_mode_dongle_first_ul_handle;
            extern gaming_mode_dongle_ul_handle_t *gaming_mode_dongle_query_ul_handle_by_scenario_type(audio_scenario_type_t type);
            gaming_mode_dongle_ul_handle_t *usb_dongle = gaming_mode_dongle_query_ul_handle_by_scenario_type(AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_DONGLE_USB_OUT);
            uint32_t cnt = gaming_mode_dongle_first_ul_handle->total_number;
            if (usb_dongle != NULL) {
                if ((cnt > 1) && (usb_dongle->stream_status >= GAMING_MODE_UL_STREAM_INIT)/*&& (usb_dongle->play_en_status)*/) {
                    // usb out is on!
                    dongle_handle->bypass_source = true;
                    if (runtime->irq_exist == false) {
                        /* we should change the status to avoid that line out can't enter start status */
                        dongle_handle->stream_status = GAMING_MODE_UL_STREAM_RUNNING;
                    }
                } else {
                    if (cnt > 1) {
                        DSP_MW_LOG_I("Clear buffer st %d", 1, usb_dongle->stream_status);
                    }
                    dongle_handle->bypass_source = false;
                }
            }
            hal_gpt_sw_start_timer_us(g_line_out_timer_handle, 2500, (hal_gpt_callback_t)gaming_mode_dongle_ul_afe_out_gpt_irq, NULL);
            AudioCheckTransformHandle(sink->transform);
        }
    }
}
#endif /* AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE */

/****************************************************************************************************************************************************/
/*                                                               ULL Dongle Common API                                                              */
/****************************************************************************************************************************************************/
#if 0
void gaming_mode_dongle_init(SOURCE source, SINK sink, mcu2dsp_open_param_p *open_param)
{
    switch (source->scenario_type) {
        case AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_DONGLE_USB_OUT:
            break;
        case AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_GAMING_MODE_MUSIC_DONGLE_USB_IN_1:
            break;
        case AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_DONGLE_LINE_OUT:
            break;
        case AUDIO_SCENARIO_TYPE_GAMING_MODE_VOICE_DONGLE_I2S_OUT:
            break;

        default:
            break;
    }
}

void gaming_mode_dongle_start(SOURCE source, SINK sink)
{

}

void gaming_mode_dongle_stop(SOURCE source, SINK sink)
{

}

void gaming_mode_dongle_deinit(SOURCE source, SINK sink)
{

}

void gaming_mode_dongle_drop_flush_postprocess(SOURCE source, SINK sink, uint32_t amount)
{

}

void gaming_mode_dongle_get_avail_size(SOURCE source, SINK sink, uint32_t *avail_size)
{

}

void gaming_mode_dongle_copy_payload(SOURCE source, SINK sink, uint8_t *src_buf, uint8_t *dst_buf, uint32_t length)
{

}
#endif
#endif /* AIR_GAMING_MODE_DONGLE_ENABLE */

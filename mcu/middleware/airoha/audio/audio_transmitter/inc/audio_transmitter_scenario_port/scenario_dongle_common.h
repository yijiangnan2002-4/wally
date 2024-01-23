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

#ifndef _SCENARIO_DONGLE_COMMON_H_
#define _SCENARIO_DONGLE_COMMON_H_
/* Includes ------------------------------------------------------------------*/
#include "types.h"
#include "audio_transmitter_internal.h"
#include "hal_audio_message_struct_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define AIR_AUDIO_DONGLE_DEBUG_LANTENCY                              (1)

#if defined(AIR_BT_AUDIO_DONGLE_ENABLE)
#define AUDIO_DONGLE_USB_RX_PORT_TOTAL                               (2)
#define AUDIO_DONGLE_USB_TX_PORT_TOTAL                               (2)
#else
#define AUDIO_DONGLE_USB_RX_PORT_TOTAL                               (2)
#define AUDIO_DONGLE_USB_TX_PORT_TOTAL                               (1)
#endif

#define AUDIO_DONGLE_GAIN_COMPENSATION_STEP_MAX                      (10)

/* LHDC V5 */
#define AUDIO_DONGLE_VENDOR_CODEC_LHDC_V5_VENDOR_ID                  (0x053A)
#define AUDIO_DONGLE_VENDOR_CODEC_LHDC_V5_CODEC_ID                   (0x4C35)

typedef struct
{
    uint8_t                             first_time;
    uint8_t                             stream_is_started;
    uint32_t                            previous_gpt_count;
    audio_transmitter_block_header_t    usb_stream_header;
    uint32_t                            frame_interval; // uint: us
    uint32_t                            frame_size;
    audio_dsp_codec_type_t              usb_type;
    audio_codec_param_t                 usb_param;
    n9_dsp_share_info_t                 *p_dsp_info;
    uint8_t                             dongle_stream_status; // bit mask for sub id
} audio_dongle_usb_handle_t;

#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
typedef struct  {
    U16 DataOffset; /* offset of payload */
    U16 _reserved_word_02h;
    U32 TimeStamp; /* this CIS/BIS link's CLK, Unit:312.5us */
    U16 ConnEvtCnt; /* event count seem on airlog, for debug propose */
    U8 SampleSeq;  /* Sameple sequence of this SDU interval Ex:0,1,2... */
    U8 _reserved_byte_0Bh; /* valid packet: 0x01, invalid packet 0x00 */
    U8 PduHdrLo;
    U8 _reserved_byte_0Dh;
    U8 PduLen ; /* payload size */
    U8 _reserved_byte_0Fh;
    U16 DataLen;
    U16 _reserved_word_12h;
    U32 _reserved_long_0;
    U32 _reserved_long_1;
} LE_AUDIO_HEADER;
#endif /* AIR_BLE_AUDIO_DONGLE_ENABLE */

uint32_t ble_audio_codec_get_frame_size(audio_dsp_codec_type_t *codec_type, audio_codec_param_t *codec_param);

#if defined AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE || defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE || defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE || defined(AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE)
void audio_dongle_set_start_avm_config(audio_transmitter_config_t *config, mcu2dsp_start_param_t *start_param);
void audio_dongle_set_stream_in_afe(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param);
#endif

#if defined(AIR_USB_AUDIO_1_SPK_ENABLE)
void audio_dongle_usb0_rx_cb(void);
void audio_dongle_usb0_tx_cb(void);
#endif

#if defined(AIR_USB_AUDIO_2_SPK_ENABLE)
void audio_dongle_usb1_rx_cb(void);
#endif
bool audio_dongle_write_data_to_usb(uint8_t usb_port, uint8_t *data, uint32_t *length);
bool audio_dongle_read_data_from_usb(uint8_t usb_port, uint8_t *data, uint32_t *length);
uint32_t audio_dongle_get_codec_frame_size(audio_dsp_codec_type_t *codec_type, audio_codec_param_t *codec_param);
uint32_t audio_dongle_get_usb_audio_frame_size(audio_dsp_codec_type_t *usb_type, audio_codec_param_t *usb_param);
uint8_t audio_dongle_get_format_bytes(hal_audio_format_t pcm_format);
uint8_t audio_dongle_get_usb_format_bytes(hal_audio_format_t pcm_format);

#ifdef AIR_BT_AUDIO_DONGLE_LHDC_ENABLE
bool audio_dongle_vendor_parameter_parse(audio_codec_vendor_config_t *vendor, void *param);
#endif /* AIR_BT_AUDIO_DONGLE_LHDC_ENABLE */

void audio_usb_rx_scenario_latency_debug_control(uint32_t usb_port, bool enable, uint32_t gpio_num);
void audio_usb_rx_scenario_latency_debug_init(uint32_t usb_port, uint32_t frame_size, uint32_t channels, hal_audio_format_t format);
void audio_usb_rx_scenario_latency_debug(uint32_t usb_port, uint8_t *source_buf);
void audio_usb_tx_scenario_latency_debug_control(uint32_t usb_port, bool enable, uint32_t gpio_num, int32_t current_threshold);
void audio_usb_tx_scenario_latency_debug_init(uint32_t usb_port, uint32_t frame_size, uint32_t channels, hal_audio_format_t format, int32_t current_threshold);
void audio_usb_tx_scenario_latency_debug(uint32_t usb_port, uint8_t *source_buf);

#ifdef __cplusplus
}
#endif


#endif /* _SCENARIO_DONGLE_COMMON_H_ */
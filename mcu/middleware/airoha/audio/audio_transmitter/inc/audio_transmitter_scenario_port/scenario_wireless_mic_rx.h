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

#ifndef _SCENARIO_WIRELESS_MIC_RX_H_
#define _SCENARIO_WIRELESS_MIC_RX_H_

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(AIR_WIRELESS_MIC_RX_ENABLE)

/* Includes ------------------------------------------------------------------*/
#include "audio_log.h"
#include "FreeRTOS.h"
#include "bt_sink_srv_ami.h"
#include "audio_nvdm_coef.h"
#include "audio_nvdm_common.h"
#include "hal_audio_message_struct.h"
#include <stdlib.h>

/* Public define -------------------------------------------------------------*/
#define WIRELESS_MIC_RX_DATA_CHANNEL_NUMBER            4

/* Public typedef ------------------------------------------------------------*/
typedef struct  {
    U16 DataOffset; /* offset of payload */
    U16 _reserved_word_02h;
    U32 TimeStamp; /* this CIS/BIS link's CLK, Unit:312.5us */
    U16 ConnEvtCnt; /* event count seem on airlog, for debug propose */
    U8 SampleSeq;  /* Sameple sequence of this SDU interval Ex:0,1,2... */
    U8 _reserved_byte_0Bh;
    U8 PduHdrLo;
    U8 _reserved_byte_0Dh; /* valid packet: 0x01, invalid packet 0x00 */
    U8 PduLen ; /* payload size */
    U8 _reserved_byte_0Fh;
    U16 DataLen;
    U16 _reserved_word_12h;
    U32 _reserved_long_0;
    U32 _reserved_long_1;
} WIRELESS_MIC_RX_HEADER;

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
extern void wireless_mic_rx_open_playback(audio_transmitter_config_t *config, mcu2dsp_open_param_t *open_param);
extern void wireless_mic_rx_start_playback(audio_transmitter_config_t *config, mcu2dsp_start_param_t *start_param);
extern audio_transmitter_status_t wireless_mic_rx_set_runtime_config_playback(audio_transmitter_config_t *config, audio_transmitter_runtime_config_type_t runtime_config_type, audio_transmitter_runtime_config_t *runtime_config, mcu2dsp_audio_transmitter_runtime_config_param_t *runtime_config_param);
extern audio_transmitter_status_t wireless_mic_rx_get_runtime_config(uint8_t scenario_type, uint8_t scenario_sub_id, audio_transmitter_runtime_config_type_t runtime_config_type, audio_transmitter_runtime_config_t *runtime_config);
extern void wireless_mic_rx_state_started_handler(uint8_t scenario_sub_id);
extern void wireless_mic_rx_state_idle_handler(uint8_t scenario_sub_id);
extern void wireless_mic_rx_state_starting_handler(uint8_t scenario_sub_id);
extern void wireless_mic_rx_state_stoping_handler(uint8_t scenario_sub_id);
extern uint8_t *wireless_mic_rx_audio_connection_info_malloc(uint32_t info_size);
extern void wireless_mic_rx_audio_connection_info_free(uint8_t *info);
extern void wireless_mic_rx_audio_connection_info_get(uint32_t mode, uint8_t **info, uint32_t *info_size);
extern void wireless_mic_rx_audio_connection_info_parse(uint8_t *info, uint32_t info_size);

#endif /* AIR_WIRELESS_MIC_RX_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* _SCENARIO_WIRELESS_MIC_RX_H_ */

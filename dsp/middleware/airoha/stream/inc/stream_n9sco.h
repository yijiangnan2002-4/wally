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

#ifndef __STREAM_N9SCO_H__
#define __STREAM_N9SCO_H__

/*!
 *@file   stream.h
 *@brief  defines the heap management of system
 *
 @verbatim
 @endverbatim
 */

#include "config.h"
#include "types.h"
#include "source_inter.h"
#include "sink_inter.h"
#include "source.h"
#include "sink.h"
#include "config.h"
#include "types.h"
#include "source_inter.h"
#include "sink_inter.h"
#include "transform_inter.h"
#include "common.h"
#include "bt_types.h"

#define DL_TRIGGER_UL (1)

////////////////////////////////////////////////////////////////////////////////
// Type Defintions /////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

typedef enum {
    SCO_PKT_FREE,
    SCO_PKT_USED,
    SCO_PKT_LOST,
} sco_packet_state;

typedef enum {
    ENABLE_FORWARDER,
    DISABLE_FORWARDER,
} fowarder_ctrl;

typedef enum {
    RX_FORWARDER,
    TX_FORWARDER,
} fowarder_type;

typedef enum {
    SCO_PKT_2EV3 = 0x1,
    SCO_PKT_EV3  = 0x2,
    SCO_PKT_HV2  = 0x3,
    SCO_PKT_HV1  = 0x6,
    SCO_PKT_NULL = -1,
} sco_pkt_type;

typedef enum {
    SCO_PKT_2EV3_LEN = 60,
    SCO_PKT_EV3_LEN  = 30,
    SCO_PKT_HV2_LEN  = 20,
    SCO_PKT_HV1_LEN  = 10,
    SCO_PKT_NULL_LEN = -1,
} sco_pkt_len;


typedef struct N9Sco_Sink_Config_s {
    bool isEnable;
    uint16_t  Buffer_Frame_Num;
    uint16_t  Process_Frame_Num;
    uint16_t  Frame_Size;
    uint16_t  Output_sample_rate;
    uint16_t  N9_Ro_abnormal_cnt;
} N9Sco_Sink_config_t;

typedef struct N9Sco_Source_Config_s {
    bool isEnable;
    uint16_t  Buffer_Frame_Num;
    uint16_t  Process_Frame_Num;
    uint16_t  Frame_Size;
    uint16_t  Input_sample_rate;
} N9Sco_Source_config_t;

typedef struct Stream_n9sco_Config_s {
    N9Sco_Source_config_t N9Sco_source;
    N9Sco_Sink_config_t N9Sco_sink;
} Stream_n9sco_Config_t, *Stream_n9sco_Config_Ptr;

typedef struct Sco_Rx_InbandInfo_s {
    struct InbandInf_s {
        uint32_t OFFSET                     : 16;
        uint32_t RXED                       : 1;
        uint32_t IS_MUTE                    : 1;
        uint32_t HEC_ERR                    : 1;
        uint32_t HEC_FORCE_OK               : 1;
        uint32_t CRC_ERR                    : 1;
        uint32_t SNR_ERR                    : 1;
        uint32_t _RSVD_0_                   : 10;
    } InbandInf;
    struct PICOCLK_s {
        uint32_t PICO_CLK                   : 28;
        uint32_t _RSVD_0_                   : 4;
    } field2;
    struct SNR0_3_s {
        uint32_t SNR0                       : 5;
        uint32_t _RSVD_0_                   : 3;
        uint32_t SNR1                       : 5;
        uint32_t _RSVD_1_                   : 3;
        uint32_t SNR2                       : 5;
        uint32_t _RSVD_2_                   : 3;
        uint32_t SNR3                       : 5;
        uint32_t _RSVD_3_                   : 3;
    } SNR0_3;
    struct SNR4_7_s {
        uint32_t SNR4                       : 5;
        uint32_t _RSVD_0_                   : 3;
        uint32_t SNR5                       : 5;
        uint32_t _RSVD_1_                   : 3;
        uint32_t SNR6                       : 5;
        uint32_t _RSVD_2_                   : 3;
        uint32_t SNR7                       : 5;
        uint32_t _RSVD_3_                   : 3;
    } SNR4_7;
    struct SNR8_9_s {
        uint32_t SNR8                       : 5;
        uint32_t _RSVD_0_                   : 3;
        uint32_t SNR9                       : 5;
        uint32_t _RSVD_1_                   : 19;
    } SNR8_9;
} Sco_Rx_InbandInfo_t, *Sco_Rx_InbandInfo_ptr;

extern Stream_n9sco_Config_Ptr N9SCO_setting;

extern void SinkInitN9Sco(SINK sink);
extern void SourceInitN9Sco(SOURCE source);
#if defined(AIR_FIXED_RATIO_SRC)
extern void Sco_UL_Fix_Sample_Rate_Init(void);
extern void Sco_UL_Fix_Sample_Rate_Deinit(void);
extern void Sco_DL_Fix_Sample_Rate_Init(void);
extern void Sco_DL_Fix_Sample_Rate_Deinit(void);
#endif

#endif /* _STREAM_N9SCO_H_ */


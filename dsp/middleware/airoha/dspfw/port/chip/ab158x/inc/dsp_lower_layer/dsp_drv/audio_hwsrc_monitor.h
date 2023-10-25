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

#include "types.h"

/******************************************************************************
 * Type Definitions
 ******************************************************************************/

typedef enum {
    HWSRC_UNDERRUN_MONITOR_DISABLE_MODE,
    HWSRC_UNDERRUN_MONITOR_OBSERVE_MODE,
    HWSRC_UNDERRUN_MONITOR_ERROR_HANDLING_MODE,
} HWSRC_MONITOR_STATE_t;

typedef enum {
    HWSRC_UNDERRUN_MONITOR_DISABLE,
    HWSRC_UNDERRUN_MONITOR_V1,
    HWSRC_UNDERRUN_MONITOR_V2,
} HWSRC_UNDERRUN_MONITOR_MODE_t;

typedef struct hwsrc_underrun_recover_s {
    HWSRC_MONITOR_STATE_t src_monitor_state;
    HWSRC_UNDERRUN_MONITOR_MODE_t hwsrc_underrun_monitor_mode;
    U32 src_detect_handler;
    U32 underrun_recovery_size;
    U32 src_cnt;
    U8 buffer_low_level;
} HWSRC_UNDERRUN_RECOVER_t;

typedef struct hwsrc_compen_monitor_s {
    U32 i_rd_pre;
    U32 o_wr_pre;
    U32 i_rd_next;
    U32 o_wr_next;
    U32 o_diff_point;
    U32 i_diff_point;
    U32 i_samples;
    U32 irq_compensated_samples;
    S32 io_diff_point;
    U64 i_diff_point_remainder;
    U8  src_channel_num;
} HWSRC_COMPEN_MONITOR_t;

/******************************************************************************
 * External Function Prototypes
 ******************************************************************************/

void hal_audio_src_underrun_monitor_start(HWSRC_UNDERRUN_MONITOR_MODE_t hwsrc_underrun_monitor_mode);
void hal_audio_src_underrun_monitor_stop(void);
void hal_audio_src_underrun_recover(SINK sink, U32 input_size);
void hal_audio_src_underrun_monitor(BUFFER_INFO *buffer_info, SINK sink, U32 input_size);
U32 hal_audio_cal_compen_samples(BUFFER_INFO *buffer_info, SINK sink);
void hal_audio_cal_compen_samples_reset(BUFFER_INFO *buffer_info, SINK sink);
void hal_audio_get_compen_samples(S32 *compen_samples, U32 *input_samples, U8 *channel_num);
U32 hal_audio_get_irq_compen_samples(void *ptr);


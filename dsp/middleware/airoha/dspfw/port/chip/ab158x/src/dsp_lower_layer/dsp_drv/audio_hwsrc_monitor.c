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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "hal_audio_control.h"
#include "hal_audio_clock.h"
#include "hal_audio_driver.h"
#include "hal_audio_volume.h"
#include "hal_audio_register.h"
#include "sink_inter.h"
#include "source_inter.h"
#include "transform_.h"
#include "stream_config.h"
#include "transform_.h"
#include "sink_.h"
#include "source_.h"
#include "common.h"
#include "audio_hwsrc_monitor.h"
#include "hal_audio.h"
#include "bt_interface.h"

#define HWSRC_MONITOR_DEBUG 0

#define WAIT_FOR_WRITING_RG (800) //us
#define OUT_READ_OFFSET_RES (64) //Bytes

HWSRC_UNDERRUN_RECOVER_t recover_para = {0, 0, 0, 0, 0, 0};
HWSRC_COMPEN_MONITOR_t compen_para;

void hal_audio_src_underrun_monitor_start(HWSRC_UNDERRUN_MONITOR_MODE_t hwsrc_underrun_monitor_mode)
{
    hal_gpt_status_t gpt_status;
    DSP_MW_LOG_I("[HWSRC] Monitor Start, mode:V%d", 1, hwsrc_underrun_monitor_mode);
    //volatile SINK sink = Sink_blks[SINK_TYPE_AUDIO];
    if (recover_para.src_detect_handler == NULL) {
        gpt_status = hal_gpt_sw_get_timer(&(recover_para.src_detect_handler));
        if (gpt_status != HAL_GPT_STATUS_OK) {
            AUDIO_ASSERT(0 && "[HWSRC]src_detect_handler get timer fail");
        }
    }
    recover_para.hwsrc_underrun_monitor_mode = hwsrc_underrun_monitor_mode;
    recover_para.src_monitor_state = HWSRC_UNDERRUN_MONITOR_OBSERVE_MODE;
    recover_para.underrun_recovery_size = 0;
    recover_para.src_cnt = 0;
    recover_para.buffer_low_level = 0;
}

void hal_audio_src_underrun_monitor_stop(void)
{
    DSP_MW_LOG_I("[HWSRC] Monitor Stop", 0);
    recover_para.hwsrc_underrun_monitor_mode = HWSRC_UNDERRUN_MONITOR_DISABLE;
    recover_para.src_monitor_state = HWSRC_UNDERRUN_MONITOR_DISABLE_MODE;
    recover_para.underrun_recovery_size = 0;
    recover_para.src_cnt = 0;
    recover_para.buffer_low_level = 0;
    if (recover_para.src_detect_handler != NULL) {
        hal_gpt_sw_stop_timer_us(recover_para.src_detect_handler);
        hal_gpt_sw_free_timer(recover_para.src_detect_handler);
        recover_para.src_detect_handler = NULL;
    }
}

void hwsrc_detect_msg(U32 owo, U32 rwo, U32 src_out_size, BOOL underrun)
{
    U32 gpt_cnt = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_cnt);
    DSP_MW_LOG_E("[HWSRC] underrun:%d, trigger monitor:%d, owo:%d oro:%d, out size:%d, gpt_time:%d", 6, underrun, !underrun, owo, rwo, src_out_size, gpt_cnt);
}

static void hwsrc_detect_timer_callback(void *user_data)
{
    SINK sink = (SINK)user_data;
    U32 src_out_size = 0;
    U32 owo = AFE_GET_REG(ASM_CH01_OBUF_WRPNT) - AFE_GET_REG(ASM_OBUF_SADR);
    U32 rwo = AFE_GET_REG(ASM_CH01_OBUF_RDPNT) - AFE_GET_REG(ASM_OBUF_SADR);
    if (recover_para.src_monitor_state != HWSRC_UNDERRUN_MONITOR_DISABLE_MODE) {
        src_out_size = ((sink->param.audio.AfeBlkControl.u4asrc_buffer_size) + owo - rwo) % (sink->param.audio.AfeBlkControl.u4asrc_buffer_size);
        if (src_out_size < (sink->param.audio.AfeBlkControl.u4asrc_buffer_size >> 2) && src_out_size != 0) {
            hal_audio_src_underrun_monitor(&sink->streamBuffer.BufferInfo, sink, 0);
            hwsrc_detect_msg(owo, rwo, src_out_size, false);
            recover_para.buffer_low_level = 1;
        } else {
            hwsrc_detect_msg(owo, rwo, src_out_size, true);
            recover_para.src_monitor_state = HWSRC_UNDERRUN_MONITOR_ERROR_HANDLING_MODE;
            recover_para.buffer_low_level = 0;
        }
    }
}

void hwsrc_detect_timer_setup(SINK sink, U32 WO, U32 RO, U32 input_size)
{
    U32 remaing_time = 0;
    U32 play_nat_clk = 0;
    U32 play_intra_clk = 0;
    U32 NativeCLK = 0;
    U32 NativePhase = 0;
    U8  play_en = 0;
    U32 gpt_time_1 = 0, gpt_time_2 = 0;
    U32 output_rate = (sink->param.audio.rate) / 100;
    U32 input_rate = (sink->param.audio.src_rate) / 100;
    U8  channel_num = sink->param.audio.channel_num;
    U32 BytesPerSample = sink->param.audio.format_bytes;
    U32 output_buffer_size = sink->param.audio.AfeBlkControl.u4asrc_buffer_size;
    U32 src_out_size, src_convert_size, consumable_samples, duration_us;
#if HWSRC_MONITOR_DEBUG
    U32 src_out_size_org = 0;
#endif
    if (recover_para.buffer_low_level) {
        src_out_size = (output_buffer_size + WO - RO) % output_buffer_size;
        if (src_out_size >= (output_buffer_size >> 2)) {
            recover_para.src_monitor_state = HWSRC_UNDERRUN_MONITOR_ERROR_HANDLING_MODE;
            hwsrc_detect_msg(WO, RO, src_out_size, true);
        }
        recover_para.buffer_low_level = 0;
    }

    if (recover_para.src_monitor_state == HWSRC_UNDERRUN_MONITOR_OBSERVE_MODE) {

        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_time_1);
        hal_gpt_sw_get_remaining_time_us(recover_para.src_detect_handler, &remaing_time);

        //if ((WO == RO) && (((1000000 / 100) / output_rate) < remaing_time)) { //Troubleshoot buffer full
        if (WO == RO) {
            src_out_size = output_buffer_size;
        } else {
            src_out_size = (output_buffer_size + WO - RO) % output_buffer_size;
        }
#if HWSRC_MONITOR_DEBUG
        src_out_size_org = src_out_size;
#endif
        if (src_out_size > (OUT_READ_OFFSET_RES * 2)) {
            src_out_size -= (OUT_READ_OFFSET_RES * 2);//Check ahead to avoid corner situations
        } else if (src_out_size > OUT_READ_OFFSET_RES) {
            src_out_size -= OUT_READ_OFFSET_RES;//Check ahead to avoid corner situations
        } else if (input_size == 0) {
            recover_para.buffer_low_level = 1;
        }

        src_convert_size = (input_size * output_rate) / input_rate;
        consumable_samples = ((src_out_size + src_convert_size) / (channel_num * BytesPerSample));
        duration_us = (U32)(((U64)consumable_samples * (1000000 / 100)) / (U64)output_rate);

        if (!sink->param.audio.irq_exist) {
            hal_audio_afe_get_play_en(&play_nat_clk, &play_intra_clk, &play_en);

            if (play_en) { //If the playback isn't started, the data isn't consumed. So, the duration_us need to be postponed.
                NativeCLK = rBb->rClkCtl.rNativeClock & 0x0FFFFFFC;
                NativePhase = (U32)rBb->rClkCtl.rNativePhase;
                duration_us += (U32)((((U64)((play_nat_clk) & 0x0FFFFFFC) * 3125 + (U64)(play_intra_clk) * 5) - ((U64)(NativeCLK) * 3125 + (U64)NativePhase * 5)) / 10);
            }
        }

        if (recover_para.src_detect_handler != NULL) {
            if (remaing_time) {
                hal_gpt_sw_stop_timer_us(recover_para.src_detect_handler);
            }
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_time_2);
            if (duration_us > (gpt_time_2 - gpt_time_1)) {
                duration_us = duration_us - (gpt_time_2 - gpt_time_1);
            }
#if HWSRC_MONITOR_DEBUG
            DSP_MW_LOG_I("[HWSRC_DEBUG] duration_us:%d, consumable_samples,:%d, input_size:%d, src_out_size_org:%d, src_out_size:%d, gpt_time:%d", 6, duration_us, consumable_samples, input_size, src_out_size_org, src_out_size, gpt_time_2);
#endif
            hal_gpt_sw_start_timer_us(recover_para.src_detect_handler, duration_us, hwsrc_detect_timer_callback, (void *)sink);
        }
    }
}

void hal_audio_src_underrun_monitor(BUFFER_INFO *buffer_info, SINK sink, U32 input_size)
{
    UNUSED(buffer_info);
    U32 WO = AFE_GET_REG(ASM_CH01_OBUF_WRPNT) - AFE_GET_REG(ASM_OBUF_SADR);
    U32 RO = AFE_GET_REG(ASM_CH01_OBUF_RDPNT) - AFE_GET_REG(ASM_OBUF_SADR);

    if (recover_para.hwsrc_underrun_monitor_mode == HWSRC_UNDERRUN_MONITOR_DISABLE) {
        return;
    }

    switch (recover_para.src_monitor_state) {
        case HWSRC_UNDERRUN_MONITOR_ERROR_HANDLING_MODE:
            sink->param.audio.afe_wait_play_en_cnt = PLAY_EN_TRIGGER_REINIT_MAGIC_NUM;
            recover_para.src_monitor_state = HWSRC_UNDERRUN_MONITOR_OBSERVE_MODE;
            break;
        case HWSRC_UNDERRUN_MONITOR_OBSERVE_MODE:
            hwsrc_detect_timer_setup(sink, WO, RO, input_size);
            break;
        case HWSRC_UNDERRUN_MONITOR_DISABLE_MODE:
            break;
        default:
            AUDIO_ASSERT(0 && "[HWSRC] src monitor meet unknow state");
            break;
    }

}

U32 hal_audio_cal_compen_samples(BUFFER_INFO *buffer_info, SINK sink)
{
    U8 channel_num = sink->param.audio.channel_num;
    U32 output_rate = (sink->param.audio.rate) / 100;
    U32 input_rate = (sink->param.audio.src_rate) / 100;
    U32 BytesPerSample = sink->param.audio.format_bytes;
    U32 input_samples = 0;
    U32 remainder = 0;
    U32 mask;

    compen_para.src_channel_num = channel_num;

    compen_para.i_rd_next = AFE_GET_REG(ASM_CH01_IBUF_RDPNT);
    compen_para.o_wr_next = AFE_GET_REG(ASM_CH01_OBUF_WRPNT);

    if (compen_para.i_rd_pre != 0xFFFFFFFF && compen_para.o_wr_pre != 0xFFFFFFFF) {
        compen_para.i_diff_point = (compen_para.i_rd_next - compen_para.i_rd_pre + (buffer_info->length)) % (buffer_info->length);
        input_samples = compen_para.i_diff_point / (BytesPerSample * channel_num);
        compen_para.o_diff_point = (compen_para.o_wr_next - compen_para.o_wr_pre + (sink->param.audio.AfeBlkControl.u4asrc_buffer_size)) % (sink->param.audio.AfeBlkControl.u4asrc_buffer_size);
        //i_diff_point_remainder = (((U64)i_diff_point)*((U64)output_rate))%input_rate;

        compen_para.i_diff_point *= output_rate;
        remainder = compen_para.i_diff_point % input_rate;
        compen_para.i_diff_point -= remainder;
        compen_para.i_diff_point = compen_para.i_diff_point / input_rate;
        compen_para.i_diff_point_remainder += remainder;

        if (compen_para.i_diff_point_remainder >= input_rate) {
            compen_para.i_diff_point += (compen_para.i_diff_point_remainder / input_rate);

            hal_nvic_save_and_set_interrupt_mask(&mask);
            compen_para.irq_compensated_samples += (compen_para.i_diff_point_remainder / input_rate);
            hal_nvic_restore_interrupt_mask(mask);

            compen_para.i_diff_point_remainder = (compen_para.i_diff_point_remainder % input_rate);
        }
        compen_para.io_diff_point += ((S32)compen_para.o_diff_point - (S32)compen_para.i_diff_point);

        compen_para.i_samples += compen_para.i_diff_point / (BytesPerSample * channel_num);
    }

    compen_para.i_rd_pre = compen_para.i_rd_next;
    compen_para.o_wr_pre = compen_para.o_wr_next;

    return input_samples;
}

void hal_audio_cal_compen_samples_reset(BUFFER_INFO *buffer_info, SINK sink)
{
#ifdef MTK_DEBUG_LEVEL_INFO
    U32 BytesPerSample = sink->param.audio.format_bytes;
    U8 channel_num = sink->param.audio.channel_num;
#else
    UNUSED(buffer_info);
    UNUSED(sink);
#endif
    compen_para.i_rd_pre = AFE_GET_REG(ASM_CH01_IBUF_RDPNT);
    compen_para.o_wr_pre = AFE_GET_REG(ASM_CH01_OBUF_WRPNT);
    compen_para.i_rd_next = 0;
    compen_para.o_wr_next = 0;
    compen_para.o_diff_point = 0;
    compen_para.i_diff_point = 0;
    compen_para.i_diff_point_remainder = 0;
    compen_para.io_diff_point = 0;
    compen_para.i_samples = 0;
    compen_para.irq_compensated_samples = 0;
#ifdef MTK_DEBUG_LEVEL_INFO
    DSP_MW_LOG_I("[HWSRC_CLK_SKEW]in_buff:%d, out_buff:%d BytesPerSample:%d, channel_num:%d", 4, buffer_info->length, sink->param.audio.AfeBlkControl.u4asrc_buffer_size, BytesPerSample, channel_num);
#endif
}

void hal_audio_get_compen_samples(S32 *compen_samples, U32 *input_samples, U8 *channel_num)
{
    *compen_samples = compen_para.io_diff_point;
    *input_samples = compen_para.i_samples;
    *channel_num = compen_para.src_channel_num;
}

U32 hal_audio_get_irq_compen_samples(void* ptr)
{
    SINK sink = (SINK)ptr;
    U32 samples;
    U32 mask;
    U8 channel_num = sink->param.audio.channel_num;
    U32 BytesPerSample = sink->param.audio.format_bytes;

    hal_nvic_save_and_set_interrupt_mask(&mask);
    samples = compen_para.irq_compensated_samples / (BytesPerSample * channel_num);
    compen_para.irq_compensated_samples = compen_para.irq_compensated_samples % (BytesPerSample * channel_num);
    hal_nvic_restore_interrupt_mask(mask);

    return samples;
}



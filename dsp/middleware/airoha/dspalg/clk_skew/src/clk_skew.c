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

#include "dsp_feature_interface.h"
#include "dsp_audio_process.h"
#include "dsp_share_memory.h"
#include "compander_interface.h"
#include "bt_interface.h"
//#include "dsp_rom_table.h"
#include "dsp_audio_ctrl.h"
#include "dsp_control.h"
#include "dsp_memory.h"
#include "clk_skew.h"
#include "hal_audio_afe_control.h"
#include "hal_audio_afe_define.h"
#include "audio_afe_common.h"
#include "long_term_clk_skew.h"
#ifdef MTK_BT_CLK_SKEW_USE_PIC
#include "clk_skew_portable.h"
#endif
#include "dsp_dump.h"
#include "clk_skew_protect.h"

#ifdef AIR_BT_CODEC_BLE_ENABLED
#include "source_inter.h"
#endif

#ifdef MTK_CELT_DEC_ENABLE
#include "celt_dec_interface.h"
#endif

#include "ext_clk_skew.h"

#include "bt_interface.h"

#ifdef ENABLE_HWSRC_CLKSKEW
#include "audio_hwsrc_monitor.h"
#endif

#ifdef ENABLE_HWSRC_CLKSKEW
bool ClkSkewMode_g;
bool ClkSkewMode_isModify_g = false;
#endif
//#define DSP_CLK_SKEW_DEBUG_LOG 0

/**
 *
 * Function Prototype
 *
 */

#ifdef ENABLE_HWSRC_CLKSKEW
void ClkSkewMode_Selection(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    ClkSkewMode_isModify_g = TRUE;

    switch (msg.ccni_message[1]) {
        case CLK_SKEW_V1:
            ClkSkewMode_g = CLK_SKEW_V1;
            break;
        case CLK_SKEW_V2:
            ClkSkewMode_g = CLK_SKEW_V2;
            break;
    }
}
#endif

ATTR_TEXT_IN_IRAM_LEVEL_2 CLK_SKEW_FS_t clk_skew_fs_converter(stream_samplerate_t fs_in)
{
    CLK_SKEW_FS_t fs_out;

    switch (fs_in) {
        case FS_RATE_8K:
            fs_out = CLK_SKEW_FS_8K;
            break;
        case FS_RATE_16K:
            fs_out = CLK_SKEW_FS_16K;
            break;
        case FS_RATE_24K:
            fs_out = CLK_SKEW_FS_24K;
            break;
        case FS_RATE_32K:
            fs_out = CLK_SKEW_FS_32K;
            break;
        case FS_RATE_44_1K:
            fs_out = CLK_SKEW_FS_44_1K;
            break;
        case FS_RATE_48K:
            fs_out = CLK_SKEW_FS_48K;
            break;
        case FS_RATE_88_2K:
            fs_out = CLK_SKEW_FS_88_2K;
            break;
        case FS_RATE_96K:
            fs_out = CLK_SKEW_FS_96K;
            break;
        case FS_RATE_192K:
            fs_out = CLK_SKEW_FS_192K;
            break;
        default:
            fs_out = 0;
            break;
    }
    return fs_out;
}
ATTR_TEXT_IN_IRAM_LEVEL_2 S16 clk_skew_check_status_pcdc(DSP_CLOCK_SKEW_PCDC_CTRL_t* ClkSkewPCDCCtrl)
{
    S16 i2AfeModifySamples = 0;

    if(ClkSkewPCDCCtrl->accumulated_samples > 0 && ClkSkewPCDCCtrl->compensated_samples == 0){
        i2AfeModifySamples = 1;
        ClkSkewPCDCCtrl->accumulated_samples--;
        ClkSkewPCDCCtrl->compensated_samples++;
    }else if(ClkSkewPCDCCtrl->accumulated_samples < 0 && ClkSkewPCDCCtrl->compensated_samples ==0){
        i2AfeModifySamples = -1;
        ClkSkewPCDCCtrl->accumulated_samples++;
        ClkSkewPCDCCtrl->compensated_samples--;
    }

    return i2AfeModifySamples;    
}

void clk_skew_check_status_log(CLK_SKEW_DIRECTION_TYPE_t dir, S16 CompensatedPolarity)
{
    DSP_MW_LOG_I("[CLK_SKEW][%d] acu:%d", 2, dir, CompensatedPolarity);
}

ATTR_TEXT_IN_IRAM_LEVEL_2 S16 clk_skew_check_status_ecdc(DSP_CLOCK_SKEW_ECDC_CTRL_t* ClkSkewECDCCtrl, DSP_CLOCK_SKEW_SETUP_t* ClkSkewSetup)
{
    CLK_SKEW_DIRECTION_TYPE_t dir = ClkSkewSetup->clk_skew_dir;
    S32 i2AfeModifySamples = 0;
    U32 mask;

    if(dir == CLK_SKEW_UL){
        if(ClkSkewECDCCtrl->cp_samples >= 4 || ClkSkewECDCCtrl->cp_samples <= -4){
            hal_nvic_save_and_set_interrupt_mask(&mask);
            i2AfeModifySamples = (ClkSkewECDCCtrl->cp_samples / 4) * 4;
            ClkSkewECDCCtrl->cp_samples = ClkSkewECDCCtrl->cp_samples % 4;
            hal_nvic_restore_interrupt_mask(mask);
        }else{
            i2AfeModifySamples = 0;
        }
    }else{
        hal_nvic_save_and_set_interrupt_mask(&mask);
        i2AfeModifySamples = ClkSkewECDCCtrl->cp_samples;
        ClkSkewECDCCtrl->cp_samples = 0;
        hal_nvic_restore_interrupt_mask(mask);
    }

    return (S16)i2AfeModifySamples;
}

ATTR_TEXT_IN_IRAM_LEVEL_2 S16 clk_skew_check_status(DSP_CLOCK_SKEW_CTRL_t *pClkSkewCtrl, DSP_CLOCK_SKEW_SETUP_t* ClkSkewSetup)
{
    uint32_t mask;
    S16 i2AfeModifySamples = 0;
    CLK_SKEW_DIRECTION_TYPE_t dir = ClkSkewSetup->clk_skew_dir;

    if (!pClkSkewCtrl->Initialized) {
        return 0;
    }

    if ((pClkSkewCtrl->IntrDownCnt > 0) && (pClkSkewCtrl->CompensatedPolarity != 0)) {
        /* Audio Compensation Already Exists */
        pClkSkewCtrl->IntrDownCnt--;

        if ((pClkSkewCtrl->IntrDownCnt == 1) && (dir == CLK_SKEW_UL) && (pClkSkewCtrl->UL_Pol_Change_Samples == 1) && (pClkSkewCtrl->CompensatedPolarity < 0)) {
            pClkSkewCtrl->PollingFlag = TRUE;
            pClkSkewCtrl->UL_Pol_Change_Samples = 0;
        }

        if (pClkSkewCtrl->IntrDownCnt == 0) {
            if (pClkSkewCtrl->CompensatedPolarity > 0) {
                pClkSkewCtrl->CompensatedSamples += 1;
            } else {
                pClkSkewCtrl->CompensatedSamples += -1;
            }

            pClkSkewCtrl->CompensatedPolarity = 0;
        }
    } else {

        hal_nvic_save_and_set_interrupt_mask(&mask); // Protect pClkSkewCtrl->AccumulatedSamples

        /* Audio Compensation Does Not Exist */
        if ((pClkSkewCtrl->AccumulatedSamples > 0) && (pClkSkewCtrl->CompensatedSamples == 0)) {
            /* Audio Sample Leads */
            pClkSkewCtrl->AccumulatedSamples--;
            pClkSkewCtrl->IntrDownCnt = 2;

            if (dir == CLK_SKEW_DL) {
                pClkSkewCtrl->CompensatedPolarity = 1;
            } else if (dir == CLK_SKEW_UL) {
                pClkSkewCtrl->CompensatedPolarity = -1;
            }

            if (dir == CLK_SKEW_DL) {
                i2AfeModifySamples = 1;
            } else if (dir == CLK_SKEW_UL) {
                pClkSkewCtrl->InterruptHandleCnt++;
                if(pClkSkewCtrl->InterruptHandleCnt == 4){
                    i2AfeModifySamples = 4;
                    pClkSkewCtrl->UL_Pol_Change_Samples = 1;
                    pClkSkewCtrl->InterruptHandleCnt = 0;
                }
            }

            hal_nvic_restore_interrupt_mask(mask);

            if(!ClkSkewSetup->pcdc_en){
                clk_skew_check_status_log(dir, pClkSkewCtrl->CompensatedPolarity);
            }

            return i2AfeModifySamples; // Report to inform afe consume 1 more sample for next next ISR

        } else if ((pClkSkewCtrl->AccumulatedSamples < 0) && (pClkSkewCtrl->CompensatedSamples == 0)) {
            /* Audio Sample Lags */
            pClkSkewCtrl->AccumulatedSamples++;
            pClkSkewCtrl->IntrDownCnt = 2;

            if (dir == CLK_SKEW_DL) {
                pClkSkewCtrl->CompensatedPolarity = -1;
            } else if (dir == CLK_SKEW_UL) {
                pClkSkewCtrl->CompensatedPolarity = 1;
            }

            if (dir == CLK_SKEW_DL) {
                i2AfeModifySamples = -1;
            } else if (dir == CLK_SKEW_UL) {
                pClkSkewCtrl->InterruptHandleCnt--;
                if(pClkSkewCtrl->InterruptHandleCnt == -4){
                    i2AfeModifySamples = -4;
                    pClkSkewCtrl->InterruptHandleCnt = 0;
                }
            }

            hal_nvic_restore_interrupt_mask(mask);

            if(!ClkSkewSetup->pcdc_en){
                clk_skew_check_status_log(dir, pClkSkewCtrl->CompensatedPolarity);
            }

            return i2AfeModifySamples; // Report to inform afe fetch 1 less sample for next next ISR

        } else {
            /* Do nothing */
        }

        hal_nvic_restore_interrupt_mask(mask);
    }

    return 0;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ VOID clk_skew_finish_compensation(DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam, S16 CompensatedSamples)
{
    uint32_t mask;
    DSP_CLOCK_SKEW_CTRL_t* ClkSkewCtrl = &(ClkSkewParam->ClkSkewCtrl);
    DSP_CLOCK_SKEW_DEBUG_t* ClkSkewDebug = &(ClkSkewParam->ClkSkewDebug);

    hal_nvic_save_and_set_interrupt_mask(&mask);
    ClkSkewCtrl->CompensatedSamples -= CompensatedSamples;
    hal_nvic_restore_interrupt_mask(mask);

    if(ClkSkewCtrl->ClkSkewMode == CLK_SKEW_V1){
        ClkSkewDebug->ComSample_acc += CompensatedSamples;
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ VOID clk_skew_finish_compensation_pcdc(DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam, S16 CompensatedSamples)
{
    uint32_t mask;
    DSP_CLOCK_SKEW_CTRL_t* ClkSkewCtrl = &(ClkSkewParam->ClkSkewCtrl);
    DSP_CLOCK_SKEW_DEBUG_t* ClkSkewDebug = &(ClkSkewParam->ClkSkewDebug);
    DSP_CLOCK_SKEW_PCDC_CTRL_t* ClkSkewPCDCCtrl = &(ClkSkewParam->ClkSkewPCDCCtrl);

    hal_nvic_save_and_set_interrupt_mask(&mask);

    ClkSkewPCDCCtrl->compensated_samples -= CompensatedSamples;

    if(ClkSkewCtrl->ClkSkewMode == CLK_SKEW_V1){
        ClkSkewDebug->ComSample_acc += CompensatedSamples;
    }

    hal_nvic_restore_interrupt_mask(mask);
}

S16 clk_skew_get_comp_samples(DSP_CLOCK_SKEW_CTRL_t* ClkSkewCtrl)
{
    S16 CompensatedSamples = 0;

    if(ClkSkewCtrl->CompensatedSamples > 0){
        CompensatedSamples = 1;
    }else if(ClkSkewCtrl->CompensatedSamples < 0){
        CompensatedSamples = -1;
    }

    return CompensatedSamples;
}

S16 clk_skew_get_comp_bytes(DSP_CLOCK_SKEW_CTRL_t* ClkSkewCtrl)
{
    S16 CompensatedSamples = 0;

    if(ClkSkewCtrl->CompensatedSamples > 0){
        CompensatedSamples = 1;
    }else if(ClkSkewCtrl->CompensatedSamples < 0){
        CompensatedSamples = -1;
    }

    return CompensatedSamples * ClkSkewCtrl->BytesPerSample;
}

S16 clk_skew_get_comp_samples_pcdc(DSP_CLOCK_SKEW_PCDC_CTRL_t* ClkSkewPCDCCtrl)
{
    S16 CompensatedSamples = 0;

    if(ClkSkewPCDCCtrl->compensated_samples > 0){
        CompensatedSamples = 1;
    }else if(ClkSkewPCDCCtrl->compensated_samples < 0){
        CompensatedSamples = -1;
    }

    return CompensatedSamples;
}

S16 clk_skew_get_comp_bytes_pcdc(DSP_CLOCK_SKEW_CTRL_t* ClkSkewCtrl, DSP_CLOCK_SKEW_PCDC_CTRL_t* ClkSkewPCDCCtrl)
{
    S16 CompensatedSamples = 0;

    if(ClkSkewPCDCCtrl->compensated_samples > 0){
        CompensatedSamples = 1;
    }else if(ClkSkewPCDCCtrl->compensated_samples < 0){
        CompensatedSamples = -1;
    }

    return CompensatedSamples * ClkSkewCtrl->BytesPerSample;
}

clkskew_mode_t Clock_Skew_Get_Mode(void *para, DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam)
{
    UNUSED(para);
#ifdef ENABLE_HWSRC_CLKSKEW
    if(ClkSkewParam->ClkSkewSetup.clk_skew_dir == CLK_SKEW_DL){
        return ClkSkewMode_g;
    }else{
        return CLK_SKEW_V1;
    }
#else
    UNUSED(ClkSkewParam);
    return CLK_SKEW_V1;
#endif
}

/*
 * Clock_Skew_Para_Init
 *
 * This function is used to init clock skew parameters
 *
 */
VOID Clock_Skew_Para_Init(void *para, DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam)
{
    DSP_CLOCK_SKEW_CTRL_t* ClkSkewCtrl = &(ClkSkewParam->ClkSkewCtrl);
    DSP_CLOCK_SKEW_RCDC_CTRL_t* ClkSkewRCDCCtrl = &(ClkSkewParam->ClkSkewRCDCCtrl);
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) && defined(AIR_DUAL_CHIP_I2S_ENABLE)
    DSP_CLOCK_SKEW_ECDC_CTRL_t* ClkSkewECDCCtrl = &(ClkSkewParam->ClkSkewECDCCtrl);
#endif

    ClkSkewCtrl->Initialized = TRUE;
    ClkSkewCtrl->ClkSkewMode = Clock_Skew_Get_Mode(para, ClkSkewParam);

    ClkSkewRCDCCtrl->first_offset_flag = TRUE;
    ClkSkewRCDCCtrl->initial_offset_flag = TRUE;

    ClkSkewRCDCCtrl->bt_clk_offset_prev = BTCLK_INVALID_CLK;
    ClkSkewRCDCCtrl->bt_clk_offset_next = BTCLK_INVALID_CLK;
    ClkSkewRCDCCtrl->bt_intra_slot_offset_prev = BTINTRA_INVALID_CLK;
    ClkSkewRCDCCtrl->bt_intra_slot_offset_next = BTINTRA_INVALID_CLK;
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) && defined(AIR_DUAL_CHIP_I2S_ENABLE)
    ClkSkewECDCCtrl->isr_bt_clk_next = BTCLK_INVALID_CLK;
    ClkSkewECDCCtrl->isr_bt_phase_next = BTINTRA_INVALID_CLK;
    ClkSkewECDCCtrl->isr_bt_clk_prev = BTCLK_INVALID_CLK;
    ClkSkewECDCCtrl->isr_bt_phase_prev = BTINTRA_INVALID_CLK;
#endif
}

ATTR_TEXT_IN_IRAM_LEVEL_2 DSP_CLOCK_SKEW_PARAM_t* Clock_Skew_Get_Param_Ptr(void *para)
{
    return (DSP_CLOCK_SKEW_PARAM_t*)stream_function_get_working_buffer(para);
}

ATTR_TEXT_IN_IRAM_LEVEL_2 U8* Clock_Skew_Get_TempBuffer_Ptr(void *MemPtr)
{
    return (U8*)MemPtr + sizeof(DSP_CLOCK_SKEW_PARAM_t);
}

ATTR_TEXT_IN_IRAM_LEVEL_2 skew_ctrl_t_ptr Clock_Skew_Get_WorkingBuffer_Ptr(void *para, void *MemPtr, U8 Num)
{
    DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam = Clock_Skew_Get_Param_Ptr(para);
    DSP_CLOCK_SKEW_CTRL_t* ClkSkewCtrl = &(ClkSkewParam->ClkSkewCtrl);
    S16 FrameSize = ClkSkewCtrl->FrameSize;
    return (skew_ctrl_t_ptr)((MemPtr + ((sizeof(DSP_CLOCK_SKEW_PARAM_t) + FrameSize + DSP_RCDC_MAX_SAMPLE_DEVIATION * 4 + 15)&0xFFFFFFF0)) + (sizeof(skew_ctrl_t)*(Num -1)));
}

ATTR_TEXT_IN_IRAM_LEVEL_2 VOID *Clock_Skew_Get_Mem_From_SrcSnk(SOURCE source, SINK sink)
{
    DSP_STREAMING_PARA_PTR pStream = DSP_Streaming_Get(source,sink);
    DSP_FEATURE_TABLE_PTR feature_table_ptr = pStream->callback.FeatureTablePtr;
    VOID *MemPtr = NULL;
    
    if (feature_table_ptr != NULL){
        while (feature_table_ptr->ProcessEntry != NULL){

            if(feature_table_ptr->ProcessEntry ==stream_function_clock_skew_process){
                MemPtr = feature_table_ptr->MemPtr;
            }

            if(feature_table_ptr->ProcessEntry == stream_function_end_process){
                break;
            }
            feature_table_ptr++;
        }
    }
    return MemPtr;
}

void Clock_Skew_Mem_Check(void *para)
{
    DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam = Clock_Skew_Get_Param_Ptr(para);
    DSP_CLOCK_SKEW_SETUP_t* ClkSkewSetup = &(ClkSkewParam->ClkSkewSetup);

    U32 buffer_lenth = stream_function_get_working_buffer_length(para);
    S16 FrameSize = (S16)stream_function_get_output_size(para);
    S16 Channels = (S16)stream_function_get_channel_number(para);

    if((DSP_CLK_SKEW_MEMSIZE(Channels,FrameSize,ClkSkewSetup->pcdc_en)) > buffer_lenth){
        assert(0 && "[CLK_SKEW][%d]Mem size isn't enough !!!");
    }
}

void Clock_Skew_Get_Isr_Interval(void *para,DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam)
{
    DSP_STREAMING_PARA_PTR stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    DSP_CLOCK_SKEW_SETUP_t* ClkSkewSetup = &(ClkSkewParam->ClkSkewSetup);
    DSP_CLOCK_SKEW_CTRL_t* ClkSkewCtrl = &(ClkSkewParam->ClkSkewCtrl);
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) && defined(AIR_DUAL_CHIP_I2S_ENABLE)
    DSP_CLOCK_SKEW_ECDC_CTRL_t* ClkSkewECDCCtrl = &(ClkSkewParam->ClkSkewECDCCtrl);
#endif
    stream_ptr->source->param.audio.enable_clk_skew = true;
    U16 irq_counter;
    U32 fs;

    if(ClkSkewSetup->clk_skew_dir == CLK_SKEW_DL){
        irq_counter = stream_ptr->sink->param.audio.mem_handle.irq_counter;
        fs = stream_ptr->sink->param.audio.rate;
    }else{
        irq_counter = stream_ptr->source->param.audio.mem_handle.irq_counter;
        fs = stream_ptr->source->param.audio.rate;
    }
    ClkSkewCtrl->isr_interval = (S32)((((S64)irq_counter)*10000000)/(S64)fs);

#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) && defined(AIR_DUAL_CHIP_I2S_ENABLE)
    ClkSkewECDCCtrl->afe_interval_cnt = (ClkSkewCtrl->isr_interval / 3125) * 8125;
#endif

    DSP_MW_LOG_I("[ISR_CLK_SKEW][%d] irq_cnt:%d, fs:%d, interval:%d us",4,ClkSkewSetup->clk_skew_dir,irq_counter,fs,(ClkSkewCtrl->isr_interval)/10);
}

ATTR_TEXT_IN_IRAM_LEVEL_2 S16 Clock_Skew_Get_Comp_Samples(SOURCE source, SINK sink)
{
    DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam = (DSP_CLOCK_SKEW_PARAM_t*)Clock_Skew_Get_Mem_From_SrcSnk(source, sink);
    if(ClkSkewParam == NULL){
        return 0;
    }

    DSP_CLOCK_SKEW_CTRL_t* ClkSkewCtrl = &(ClkSkewParam->ClkSkewCtrl);

    return clk_skew_get_comp_samples(ClkSkewCtrl);
}

ATTR_TEXT_IN_IRAM_LEVEL_2 S16 Clock_Skew_Get_Comp_Bytes(SOURCE source, SINK sink)
{
    DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam = (DSP_CLOCK_SKEW_PARAM_t*)Clock_Skew_Get_Mem_From_SrcSnk(source, sink);
    if(ClkSkewParam == NULL){
        return 0;
    }

    DSP_CLOCK_SKEW_CTRL_t* ClkSkewCtrl = &(ClkSkewParam->ClkSkewCtrl);

    return clk_skew_get_comp_bytes(ClkSkewCtrl);
}

ATTR_TEXT_IN_IRAM_LEVEL_2 bool pcdc_asi_threshold_counter(SOURCE source, SINK sink, U32 sample_size, int fs)
{
    DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam = (DSP_CLOCK_SKEW_PARAM_t*)Clock_Skew_Get_Mem_From_SrcSnk(source, sink);
    if(ClkSkewParam == NULL){
        return false;
    }
    
    DSP_CLOCK_SKEW_SETUP_t* ClkSkewSetup = &(ClkSkewParam->ClkSkewSetup);
    
    if(ClkSkewSetup->pcdc_en){
        clk_skew_pcdc_asi_th_counter(ClkSkewParam, sample_size, fs);
        return true;
    }
    return false;
}

ATTR_TEXT_IN_IRAM_LEVEL_2 void pcdc_asi_count_init(SOURCE source, SINK sink, U32 add_amount, S32 sample_rate)
{
    U32 asi_old;
    DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam = (DSP_CLOCK_SKEW_PARAM_t*)Clock_Skew_Get_Mem_From_SrcSnk(source, sink);
    if(ClkSkewParam == NULL){
        return;
    }

    DSP_CLOCK_SKEW_SETUP_t* ClkSkewSetup = &(ClkSkewParam->ClkSkewSetup);
    DSP_CLOCK_SKEW_PCDC_CTRL_t* ClkSkewPCDCCtrl = &(ClkSkewParam->ClkSkewPCDCCtrl);

    if(ClkSkewSetup->pcdc_en && !ClkSkewPCDCCtrl->asi_cnt){
        asi_old = ClkSkewPCDCCtrl->asi_cnt;
        ClkSkewPCDCCtrl->asi_cnt += (S32)((S64)add_amount*(S64)sample_rate/(S64)lt_clk_skew_get_sample_rate());
        DSP_MW_LOG_I("[CLK_SKEW] PCDC asi add, asi_old:0x%x, asi_new:0x%x, add_amount:0x%x *(%d/%d)", 5, asi_old, ClkSkewPCDCCtrl->asi_cnt, add_amount, sample_rate, lt_clk_skew_get_sample_rate());
    }
}

ATTR_TEXT_IN_IRAM_LEVEL_2 VOID Clock_Skew_Offset_Update(BT_CLOCK_OFFSET_SCENARIO type, SOURCE source, SINK sink)
{
    DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam = (DSP_CLOCK_SKEW_PARAM_t*)Clock_Skew_Get_Mem_From_SrcSnk(source, sink);

    if(ClkSkewParam == NULL){
        return;
    }
    
    DSP_CLOCK_SKEW_RCDC_CTRL_t* ClkSkewRCDCCtrl = &(ClkSkewParam->ClkSkewRCDCCtrl);
    DSP_CLOCK_SKEW_SETUP_t* ClkSkewSetup    = &(ClkSkewParam->ClkSkewSetup);
    DSP_CLOCK_SKEW_CTRL_t* ClkSkewCtrl = &(ClkSkewParam->ClkSkewCtrl);
    uint32_t mask;
    uint32_t bt_clk_offset;
    uint16_t intra_clk_offset;
    uint32_t native_clk;
    uint32_t audio_clk;

    hal_nvic_save_and_set_interrupt_mask(&mask);
    MCE_Get_BtClkOffset((BTCLK *)&bt_clk_offset, (BTPHASE *)&intra_clk_offset, type);
    MCE_Get_NativeClk_from_Controller(&native_clk, &audio_clk);

    if (ClkSkewRCDCCtrl->first_offset_flag == TRUE) {
        if(ClkSkewSetup->clk_skew_dir == CLK_SKEW_DL){
            ClkSkewCtrl->initial_offset = -(AFE_GET_REG(AFE_AUDIO_BT_SYNC_MON1) & 0xFFFF); // initial offset
        }
        ClkSkewRCDCCtrl->bt_clk_offset_prev = bt_clk_offset;
        ClkSkewRCDCCtrl->bt_intra_slot_offset_prev = intra_clk_offset;
        if (native_clk != BTCLK_INVALID_CLK) {
            ClkSkewRCDCCtrl->n9_clk_prev = native_clk;
            ClkSkewRCDCCtrl->aud_clk_prev = audio_clk;
        }/*else{
            DSP_MW_LOG_I("[CLK_SKEW]Get INVALID CLK",0);
        }*/
    } else {
        ClkSkewRCDCCtrl->bt_clk_offset_next = bt_clk_offset;
        ClkSkewRCDCCtrl->bt_intra_slot_offset_next = intra_clk_offset;
        if (native_clk != BTCLK_INVALID_CLK) {
            ClkSkewRCDCCtrl->n9_clk_next = native_clk;
            ClkSkewRCDCCtrl->aud_clk_next = audio_clk;
        }/*else{
            DSP_MW_LOG_I("[CLK_SKEW]Get INVALID CLK",0);
       }*/
    }
    hal_nvic_restore_interrupt_mask(mask);

    if(ClkSkewRCDCCtrl->first_offset_flag == TRUE) {
        ClkSkewRCDCCtrl->first_offset_flag = FALSE;
    }
}

#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) && defined(AIR_DUAL_CHIP_I2S_ENABLE)
ATTR_TEXT_IN_IRAM_LEVEL_2 void Clock_Skew_Isr_Time_Update(SOURCE source, SINK sink, U32 bt_clk_next,U16 bt_phase_next)
{
    DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam = (DSP_CLOCK_SKEW_PARAM_t*)Clock_Skew_Get_Mem_From_SrcSnk(source, sink);
    uint32_t mask;

    if(ClkSkewParam == NULL){
        return;
    }

    DSP_CLOCK_SKEW_ECDC_CTRL_t* ClkSkewECDCCtrl = &(ClkSkewParam->ClkSkewECDCCtrl);
    hal_nvic_save_and_set_interrupt_mask(&mask);
    ClkSkewECDCCtrl->isr_bt_clk_next = bt_clk_next;
    ClkSkewECDCCtrl->isr_bt_phase_next = bt_phase_next;
    hal_nvic_restore_interrupt_mask(mask);
}
#else
ATTR_TEXT_IN_IRAM_LEVEL_2 void Clock_Skew_Isr_Time_Update(SOURCE source, SINK sink, U32 clk_next, U32 default_samples_cnt)
{
    DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam = (DSP_CLOCK_SKEW_PARAM_t*)Clock_Skew_Get_Mem_From_SrcSnk(source, sink);
    uint32_t mask;

    if(ClkSkewParam == NULL){
        return;
    }

    DSP_CLOCK_SKEW_ECDC_CTRL_t* ClkSkewECDCCtrl = &(ClkSkewParam->ClkSkewECDCCtrl);
    hal_nvic_save_and_set_interrupt_mask(&mask);
    if(ClkSkewECDCCtrl->update_cnt < 2){
        ClkSkewECDCCtrl->update_cnt++;
        if(ClkSkewECDCCtrl->update_cnt == 2){
            ClkSkewECDCCtrl->isr_clk_prev = ClkSkewECDCCtrl->isr_clk_next;
            ClkSkewECDCCtrl->observed_samples_acc += default_samples_cnt;
        }
    }else{
        ClkSkewECDCCtrl->observed_samples_acc += ClkSkewECDCCtrl->isr_samples_cnt[ClkSkewECDCCtrl->read_offset];
        ClkSkewECDCCtrl->read_offset = (ClkSkewECDCCtrl->read_offset +1) % isr_samples_cnt_buffer_size;
    }
//    DSP_MW_LOG_I("[SKEW_TEST] RO :%d va:%d acc:%d clk:%d",4,ClkSkewECDCCtrl->read_offset,ClkSkewECDCCtrl->isr_samples_cnt[ClkSkewECDCCtrl->read_offset],ClkSkewECDCCtrl->observed_samples_acc,clk_next);
	ClkSkewECDCCtrl->isr_clk_next = clk_next;
    hal_nvic_restore_interrupt_mask(mask);
}
#endif

ATTR_TEXT_IN_IRAM_LEVEL_2 void Clock_Skew_Samples_Cnt_Update(SOURCE source, SINK sink, U16 samples_cnt)
{
    DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam = (DSP_CLOCK_SKEW_PARAM_t*)Clock_Skew_Get_Mem_From_SrcSnk(source, sink);
    uint32_t mask;

    if(ClkSkewParam == NULL){
        return;
    }

    DSP_CLOCK_SKEW_ECDC_CTRL_t* ClkSkewECDCCtrl = &(ClkSkewParam->ClkSkewECDCCtrl);
    hal_nvic_save_and_set_interrupt_mask(&mask);
    ClkSkewECDCCtrl->isr_samples_cnt[ClkSkewECDCCtrl->write_offset] = samples_cnt;
    ClkSkewECDCCtrl->write_offset = (ClkSkewECDCCtrl->write_offset + 1) % isr_samples_cnt_buffer_size;
    hal_nvic_restore_interrupt_mask(mask);
}

ATTR_TEXT_IN_IRAM_LEVEL_2 BOOL Clock_Skew_HWSRC_Is_Enable(SOURCE source, SINK sink)
{
    BOOL IsEnable = false;

    DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam = (DSP_CLOCK_SKEW_PARAM_t*)Clock_Skew_Get_Mem_From_SrcSnk(source, sink);

    if(ClkSkewParam == NULL){
        return IsEnable;
    }

    DSP_CLOCK_SKEW_CTRL_t* ClkSkewCtrl = &(ClkSkewParam->ClkSkewCtrl);

    if(ClkSkewCtrl->ClkSkewMode == CLK_SKEW_V2){
        IsEnable = true;
    }
    return IsEnable;
}

ATTR_TEXT_IN_IRAM_LEVEL_2 BOOL Clock_Skew_ECDC_Is_Enable(SOURCE source, SINK sink)
{
    BOOL IsEnable = false;

    DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam = (DSP_CLOCK_SKEW_PARAM_t*)Clock_Skew_Get_Mem_From_SrcSnk(source, sink);

    if(ClkSkewParam == NULL){
        return IsEnable;
    }

    DSP_CLOCK_SKEW_SETUP_t* ClkSkewSetup = &(ClkSkewParam->ClkSkewSetup);

    if(ClkSkewSetup->ecdc_en){
        IsEnable = true;
    }
    return IsEnable;
}

ATTR_TEXT_IN_IRAM_LEVEL_2 BOOL Clock_Skew_Get_Polling_Flag(SOURCE source, SINK sink)
{
    DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam = (DSP_CLOCK_SKEW_PARAM_t*)Clock_Skew_Get_Mem_From_SrcSnk(source, sink);

    if(ClkSkewParam == NULL){
        return false;
    }

    DSP_CLOCK_SKEW_CTRL_t* ClkSkewCtrl = &(ClkSkewParam->ClkSkewCtrl);

    return ClkSkewCtrl->PollingFlag;
}

ATTR_TEXT_IN_IRAM_LEVEL_2 void Clock_Skew_Set_Polling_Flag(SOURCE source, SINK sink, BOOL flag)
{
    DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam = (DSP_CLOCK_SKEW_PARAM_t*)Clock_Skew_Get_Mem_From_SrcSnk(source, sink);

    if(ClkSkewParam == NULL){
        return;
    }

    DSP_CLOCK_SKEW_CTRL_t* ClkSkewCtrl = &(ClkSkewParam->ClkSkewCtrl);

    ClkSkewCtrl->PollingFlag = flag;
}

ATTR_TEXT_IN_IRAM_LEVEL_2 S16 Clock_Skew_Check_Status_From_SrcSnk(SOURCE source, SINK sink)
{
    DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam = (DSP_CLOCK_SKEW_PARAM_t*)Clock_Skew_Get_Mem_From_SrcSnk(source, sink);
    S16 cp_samples = 0,cp_samples_pcdc = 0,cp_samples_ecdc = 0;
    CLK_SKEW_DIRECTION_TYPE_t dir;
    S32 cp_samples_temp = 0;
    S32 remainder = 0;
    S32 input_rate = (S32)(sink->param.audio.src_rate);
    S32 output_rate = (S32)(sink->param.audio.rate);
    if(ClkSkewParam == NULL){
        return 0;
    }

    DSP_CLOCK_SKEW_CTRL_t* ClkSkewCtrl = &(ClkSkewParam->ClkSkewCtrl);
    DSP_CLOCK_SKEW_SETUP_t* ClkSkewSetup = &(ClkSkewParam->ClkSkewSetup);
    DSP_CLOCK_SKEW_PCDC_CTRL_t* ClkSkewPCDCCtrl = &(ClkSkewParam->ClkSkewPCDCCtrl);
    DSP_CLOCK_SKEW_ECDC_CTRL_t* ClkSkewECDCCtrl = &(ClkSkewParam->ClkSkewECDCCtrl);
    dir = ClkSkewSetup->clk_skew_dir;

    cp_samples = clk_skew_check_status(ClkSkewCtrl, ClkSkewSetup);

    if(ClkSkewSetup->pcdc_en){
        cp_samples_pcdc = clk_skew_check_status_pcdc(ClkSkewPCDCCtrl);
    }

    if(ClkSkewSetup->ecdc_en){
        cp_samples_ecdc = clk_skew_check_status_ecdc(ClkSkewECDCCtrl, ClkSkewSetup);
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) && defined(AIR_DUAL_CHIP_I2S_ENABLE)
        return cp_samples_ecdc;
#endif
    }

    if((dir == CLK_SKEW_DL) && (ClkSkewCtrl->ClkSkewMode == CLK_SKEW_V1) && (sink->type == SINK_TYPE_AUDIO) && (sink->param.audio.rate != sink->param.audio.src_rate)){
        cp_samples_temp = (S32)(cp_samples + cp_samples_pcdc);
        cp_samples_temp *= output_rate;
        remainder = cp_samples_temp % input_rate;
        cp_samples_temp = cp_samples_temp / input_rate;
        ClkSkewCtrl->IsrCpSamples_Remainder += remainder;
        if(ClkSkewCtrl->IsrCpSamples_Remainder >= input_rate){
            cp_samples_temp += ((ClkSkewCtrl->IsrCpSamples_Remainder) / input_rate);
            ClkSkewCtrl->IsrCpSamples_Remainder = (ClkSkewCtrl->IsrCpSamples_Remainder) % input_rate;
        }
        return (S16)cp_samples_temp + cp_samples_ecdc;
    }else{
        return cp_samples + cp_samples_pcdc + cp_samples_ecdc;
    }
}

ATTR_TEXT_IN_IRAM_LEVEL_2 void Clock_Skew_Check_Isr_Status_From_SrcSnk(SOURCE source, SINK sink, BTCLK bt_clk, BTPHASE bt_phase)
{
    DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam = (DSP_CLOCK_SKEW_PARAM_t*)Clock_Skew_Get_Mem_From_SrcSnk(source, sink);

    if(ClkSkewParam == NULL){
        return;
    }
    
    clk_skew_check_isr_status(ClkSkewParam, bt_clk, bt_phase);
}

ATTR_TEXT_IN_IRAM_LEVEL_2 U32 Clock_Skew_Asrc_Get_Input_SampleSize(SOURCE source, SINK sink)
{
    DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam = (DSP_CLOCK_SKEW_PARAM_t*)Clock_Skew_Get_Mem_From_SrcSnk(source, sink);

    if(ClkSkewParam == NULL){
        return 0;
    }
    DSP_CLOCK_SKEW_CTRL_t* ClkSkewCtrl = &(ClkSkewParam->ClkSkewCtrl);

    return (ClkSkewCtrl->FrameSize)/(ClkSkewCtrl->BytesPerSample);
}

VOID Clock_Skew_Alg_Init(void* para, DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam, U16 bits, U16 skew_io_mode, S16 Channels)
{
    DSP_CLOCK_SKEW_SETUP_t* ClkSkewSetup    = &(ClkSkewParam->ClkSkewSetup);
    skew_ctrl_t_ptr  skew_ctrl_ptr;
    U16 order = ClkSkewSetup->clk_skew_order;
    U8 lib_num = Channels;
    U8 i = 0;

    if(ClkSkewSetup->pcdc_en){
        lib_num *= 2;
    }

    for(i = 0; i < lib_num; i++){
        skew_ctrl_ptr = Clock_Skew_Get_WorkingBuffer_Ptr(para, (void*)ClkSkewParam, i+1);
        skew_ctrl_init(skew_ctrl_ptr, bits, skew_io_mode, order);
    }
    DSP_MW_LOG_I("[CLK_SKEW] SW Lib. Ver:0x%x, Lib. Num:%d", 2, get_skewctrl_version(), lib_num);
}

ATTR_TEXT_IN_IRAM_LEVEL_2 void clk_skew_buffer_shift(DSP_CLOCK_SKEW_CTRL_t* ClkSkewCtrl, U8* Des, S8* Src, S16 FrameSize, U8 BytesPerSample, S8 channel)
{
    S8* clk_skew_que_ptr = NULL;

    if (Des == NULL || Src == NULL) {
        assert(0 && "[CLK_SKEW] clk_skew_buffer_shift NULL ptr!!");
        return;
    }

    if (BytesPerSample == 2) {
        clk_skew_que_ptr = (S8*)&(ClkSkewCtrl->DataQue_16[channel]);
    } else { //BytesPerSample == 4
        clk_skew_que_ptr = (S8*)&(ClkSkewCtrl->DataQue_32[channel]);
    }

    memcpy(Des, clk_skew_que_ptr, BytesPerSample);
    memcpy(Des + BytesPerSample, Src, FrameSize);
    memcpy(clk_skew_que_ptr, Src + FrameSize - BytesPerSample, BytesPerSample);
}

ATTR_TEXT_IN_IRAM S16 clk_skew_process(skew_ctrl_t_ptr  skew_ctrl_ptr, U8 *inbuf, U8 *outbuf, S16 compensate_bytes, U16 skew_points)
{
    U16  in_len_in_byte, ou_len_in_byte;
    S16  process_complete;
    U16  skew_comp_inc, skew_comp_dec, skew_comp_pass;

    switch (skew_points) {
        case C_1_mode:
            skew_comp_inc = C_Skew_Inc_1;
            skew_comp_dec = C_Skew_Dec_1;
            skew_comp_pass = C_Skew_Pass;
            break;
        case C_div8_mode:
            skew_comp_inc = C_Skew_Inc_div8;
            skew_comp_dec = C_Skew_Dec_div8;
            skew_comp_pass = C_Skew_Pass;
            break;
        case C_div64_mode:
            skew_comp_inc = C_Skew_Inc_div64;
            skew_comp_dec = C_Skew_Dec_div64;
            skew_comp_pass = C_Skew_Pass;
            break;
        case C_div256_mode:
            skew_comp_inc = C_Skew_Inc_div256;
            skew_comp_dec = C_Skew_Dec_div256;
            skew_comp_pass = C_Skew_Pass;
            break;
        case C_div512_mode:
            skew_comp_inc = C_Skew_Inc_div512;
            skew_comp_dec = C_Skew_Dec_div512;
            skew_comp_pass = C_Skew_Pass;
            break;
    }

    if (compensate_bytes > 0) {
        process_complete = skew_ctrl_process(skew_ctrl_ptr, (U8 *)inbuf, &in_len_in_byte, (U8 *)outbuf, &ou_len_in_byte, skew_comp_inc);
    } else if (compensate_bytes < 0) {
        process_complete = skew_ctrl_process(skew_ctrl_ptr, (U8 *)inbuf, &in_len_in_byte, (U8 *)outbuf, &ou_len_in_byte, skew_comp_dec);
    } else {
        process_complete = skew_ctrl_process(skew_ctrl_ptr, (U8 *)inbuf, &in_len_in_byte, (U8 *)outbuf, &ou_len_in_byte, skew_comp_pass);
    }

#ifdef DSP_CLK_SKEW_DEBUG_LOG
    if (compensate_bytes) {
        DSP_MW_LOG_I("[CLK_SKEW]compensate_bytes:%d,process_complete:%d", 2, compensate_bytes, process_complete);
    }
#endif

    if ((S16)(ou_len_in_byte - in_len_in_byte) != compensate_bytes && !process_complete) {
#ifdef DSP_CLK_SKEW_DEBUG_LOG
        DSP_MW_LOG_I("[CLK_SKEW] DspFw bytes mismatch with DspAlg bytes: %d, %d", 2, (S16)(ou_len_in_byte - in_len_in_byte), compensate_bytes);
#endif
    }
    return process_complete;
}

ATTR_TEXT_IN_IRAM U16 SW_Clock_Skew_Process(void *para ,DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam, S16 Channels, S16 FrameSize)
{
    DSP_CLOCK_SKEW_CTRL_t* ClkSkewCtrl = &(ClkSkewParam->ClkSkewCtrl);
    DSP_CLOCK_SKEW_SETUP_t* ClkSkewSetup = &(ClkSkewParam->ClkSkewSetup);
    DSP_CLOCK_SKEW_PCDC_CTRL_t* ClkSkewPCDCCtrl = &(ClkSkewParam->ClkSkewPCDCCtrl);

    U8 *ClkSkewInBuf  = Clock_Skew_Get_TempBuffer_Ptr((void*)ClkSkewParam);
    skew_ctrl_t_ptr  skew_ctrl_ptr;
    U16 skew_mode = ClkSkewSetup->clk_skew_div_mode;
    U16 BytesPerSample = ClkSkewCtrl->BytesPerSample;
    S16 CompensateBytes = 0, LTCS_CompensateBytes = 0;
    S16 process_complete;
    S8 *Buf;
    U8 i = 0;

#ifdef AIR_AUDIO_DUMP_ENABLE
    Buf = (S8 *)stream_function_get_inout_buffer(para,1);
    LOG_AUDIO_DUMP((U8 *)Buf, (FrameSize), SOURCE_IN2);
#endif

    CompensateBytes = clk_skew_get_comp_bytes(ClkSkewCtrl);

    for(i = 0; i < Channels; i++){//processing for rcdc
        Buf = (S8 *)stream_function_get_inout_buffer(para,i + 1);
        skew_ctrl_ptr = Clock_Skew_Get_WorkingBuffer_Ptr(para, (void*)ClkSkewParam, i+1);

        if(skew_mode == C_1_mode){
            memcpy(ClkSkewInBuf, Buf, FrameSize);
        }else{
            clk_skew_buffer_shift(ClkSkewCtrl, ClkSkewInBuf, Buf, FrameSize, BytesPerSample, i);//keep the last data and send it to the beginning of next frame
        }

        skew_ctrl_set_input_framesize(skew_ctrl_ptr, FrameSize);

        process_complete = clk_skew_process(skew_ctrl_ptr, (U8 *)ClkSkewInBuf, (U8 *)Buf, CompensateBytes, skew_mode);
    }

    if (CompensateBytes && !process_complete) {
        clk_skew_finish_compensation(ClkSkewParam, CompensateBytes/BytesPerSample);
    } else {
        CompensateBytes = 0;
    }

    if(ClkSkewSetup->pcdc_en && skew_mode == C_1_mode){//PCDC only supported C_1_mode

        LTCS_CompensateBytes = clk_skew_get_comp_bytes_pcdc(ClkSkewCtrl,ClkSkewPCDCCtrl);

        for(i = 0; i < Channels; i++){//processing for rcdc
            Buf = (S8 *)stream_function_get_inout_buffer(para,i + 1);
            skew_ctrl_ptr = Clock_Skew_Get_WorkingBuffer_Ptr(para, (void*)ClkSkewParam, i+1+Channels);

            memcpy(ClkSkewInBuf, Buf, FrameSize + CompensateBytes);

            skew_ctrl_set_input_framesize(skew_ctrl_ptr, FrameSize + CompensateBytes);

            process_complete = clk_skew_process(skew_ctrl_ptr, (U8 *)ClkSkewInBuf, (U8 *)Buf, LTCS_CompensateBytes, skew_mode);
        }

        if (LTCS_CompensateBytes && !process_complete) {
            clk_skew_finish_compensation_pcdc(ClkSkewParam, LTCS_CompensateBytes/BytesPerSample);
        } else {
            LTCS_CompensateBytes = 0;
        }

    }

#ifdef AIR_AUDIO_DUMP_ENABLE
    Buf = (S8 *)stream_function_get_inout_buffer(para,1);
    LOG_AUDIO_DUMP((U8 *)Buf, (FrameSize + CompensateBytes + LTCS_CompensateBytes), SOURCE_IN1);
#endif
    return (U16)(FrameSize + CompensateBytes + LTCS_CompensateBytes);
}

/**
 * stream_function_clock_skew_process
 *
 * Clock skew compensation main process
 *
 *
 * @para : Default parameter of callback function
 *  type : Scneario type
 *
 */
ATTR_TEXT_IN_IRAM_LEVEL_2 bool stream_function_clock_skew_process(void *para)
{
    DSP_STREAMING_PARA_PTR stream_ptr;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    U32 fs_in = stream_ptr->sink->param.audio.src_rate;
    U32 fs_out = stream_ptr->sink->param.audio.rate;
    S16 FrameSize = (S16)stream_function_get_output_size(para);
    S16 Channels = (S16)stream_function_get_channel_number(para);
    U8 BytesPerSample = (stream_function_get_output_resolution(para) == RESOLUTION_32BIT) ? 4 : 2;
    CLK_SKEW_FS_t fs = clk_skew_fs_converter((stream_samplerate_t)(U8)stream_function_get_samplingrate(para));

    DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam = Clock_Skew_Get_Param_Ptr(para);;
    DSP_CLOCK_SKEW_CTRL_t* ClkSkewCtrl = &(ClkSkewParam->ClkSkewCtrl);
    DSP_CLOCK_SKEW_SETUP_t* ClkSkewSetup = &(ClkSkewParam->ClkSkewSetup);

    if (FrameSize == 0) {
        return FALSE;
    }

    if(ClkSkewSetup->clk_skew_dir == CLK_SKEW_DL){
        stream_function_end_process(para);//Check the resolution and align it to output
        FrameSize = (S16)stream_function_get_output_size(para);
        BytesPerSample = (stream_function_get_output_resolution(para) == RESOLUTION_32BIT) ? 4 : 2;
    }

    if ((ClkSkewCtrl->FrameSize == 0) || (ClkSkewCtrl->BytesPerSample == 0)) {//initialize
        ClkSkewCtrl->FrameSize = FrameSize;
        ClkSkewCtrl->BytesPerSample = BytesPerSample;
        Clock_Skew_Mem_Check(para);
        Clock_Skew_Get_Isr_Interval(para, ClkSkewParam);
        if(ClkSkewCtrl->ClkSkewMode == CLK_SKEW_V1){
            Clock_Skew_Alg_Init(para, ClkSkewParam, BytesPerSample * 8, C_Skew_Oup, Channels);//initialize sw clk skew lib.
        }
#ifdef ENABLE_HWSRC_CLKSKEW
        else{
            clk_skew_compensate_by_hwsrc_init();
        }
#endif
        DSP_MW_LOG_I("[CLK_SKEW][%d]Init,Mode_V%d, FrameSize:%d, BytesPerSample:%d, Channel:%d",5,ClkSkewSetup->clk_skew_dir,ClkSkewCtrl->ClkSkewMode + 1, ClkSkewCtrl->FrameSize, ClkSkewCtrl->BytesPerSample, Channels);
    }

    if(ClkSkewSetup->clk_skew_dir == CLK_SKEW_DL){
        fs_in = stream_ptr->sink->param.audio.src_rate;
        fs_out = stream_ptr->sink->param.audio.rate;
    }else{
        fs_in = stream_ptr->source->param.audio.rate;
        fs_out = stream_ptr->source->param.audio.src_rate;
    }

    clk_skew_get_accumulated_bytes(ClkSkewParam, fs, FrameSize, fs_in, fs_out);//calculate the amount of compensation

    if(ClkSkewCtrl->ClkSkewMode == CLK_SKEW_V1){
        FrameSize = SW_Clock_Skew_Process(para ,ClkSkewParam, Channels, FrameSize);//compensate samples by sw clk skew lib.
    }
#ifdef ENABLE_HWSRC_CLKSKEW
    else{
        clk_skew_finish_compensation(ClkSkewParam, clk_skew_get_comp_samples(ClkSkewCtrl));//CLK_SKEW_V2 uses HWSRC to compensate samples. So, clean CompensatedSamples.
    }
#endif

    stream_function_modify_output_size(para,FrameSize);

    return FALSE;
}

void Clock_Skew_Initialize(void *para, DSP_CLOCK_SKEW_SETUP_t* ClkSkewSetup, S32 ini_asi)
{
    DSP_CLOCK_SKEW_PARAM_t* ClkSkewParam = Clock_Skew_Get_Param_Ptr(para);
    DSP_STREAMING_PARA_PTR stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    S32 fs = (S32)clk_skew_fs_converter((stream_samplerate_t)(U8)stream_function_get_samplingrate(para));
#ifdef ENABLE_HWSRC_CLKSKEW
        DSP_CLOCK_SKEW_CTRL_t *ClkSkewCtrl = &(ClkSkewParam->ClkSkewCtrl);
        HWSRC_UNDERRUN_MONITOR_MODE_t hwsrc_underrun_monitor_mode;
#endif

    memcpy(&(ClkSkewParam->ClkSkewSetup), ClkSkewSetup, sizeof(DSP_CLOCK_SKEW_SETUP_t));
    Clock_Skew_Para_Init(para, ClkSkewParam);

    if(ClkSkewSetup->pcdc_en){
        pcdc_asi_count_init(stream_ptr->source,stream_ptr->sink, ini_asi, fs);
    }
#ifdef ENABLE_HWSRC_CLKSKEW
        if (ClkSkewCtrl->ClkSkewMode == CLK_SKEW_V2 && ClkSkewSetup->clk_skew_dir == CLK_SKEW_DL) {
            if (stream_ptr->source->type == SOURCE_TYPE_A2DP
#ifdef AIR_BT_CODEC_BLE_ENABLED
                || stream_ptr->source->type == SOURCE_TYPE_N9BLE
#endif
               ) {
                hwsrc_underrun_monitor_mode = HWSRC_UNDERRUN_MONITOR_V1;
            } else {
                hwsrc_underrun_monitor_mode = HWSRC_UNDERRUN_MONITOR_DISABLE;
            }
            hal_audio_src_underrun_monitor_start(hwsrc_underrun_monitor_mode);
        }
#endif

}

BOOL Clock_Skew_ECDC_Initialize(void *para, CLK_SKEW_DIRECTION_TYPE_t clk_skew_dir)
{
    DSP_STREAMING_PARA_PTR stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    BOOL ecdc_en = false;

    if((clk_skew_dir == CLK_SKEW_DL && stream_ptr->sink->param.audio.audio_device == HAL_AUDIO_DEVICE_I2S_SLAVE) || (clk_skew_dir == CLK_SKEW_UL && stream_ptr->source->param.audio.audio_device == HAL_AUDIO_DEVICE_I2S_SLAVE)){
        ecdc_en = true;
    }
    return ecdc_en;
}


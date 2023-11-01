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

#ifndef _CLK_SKEW_COMPENSATION_H_
#define _CLK_SKEW_COMPENSATION_H_
#include "bt_types.h"
#include "dsp_feature_interface.h"
#include "dsp_utilities.h"
#include "common.h"
#include "skew_ctrl.h"
#include "hal_audio_afe_define.h"
#include "stream_n9sco.h"


//#define DSP_CLK_SKEW_DEBUG_LOG
//#define DSP_CLK_SKEW_MEMSIZE        (((sizeof(skew_ctrl_t)) + 3) / 4) * 4 // 4-byte alignment
#define DSP_RCDC_MAX_SAMPLE_DEVIATION  2
#define DSP_CLK_SKEW_TEMP_BUF_SIZE  (4096 + DSP_RCDC_MAX_SAMPLE_DEVIATION * 4)
#define BT_CLOCK_NUMBER_OF_BITS 28
#define BT_CLOCK_MAX_TICKS (((U32)1<<BT_CLOCK_NUMBER_OF_BITS)-1)
#define BTCLK_INVALID_CLK (0xFFFFFFFF)
#define BTINTRA_INVALID_CLK (0xFFFF)
#define BT_A2DP_MASK_BITS (0x3FFFFFF)//26bits
#define DSP_WAITING_THD (0x7FFFFFFF)
#define isr_samples_cnt_buffer_size (4)
#define CLK_SEW_MOVING_AV_ORDER 64

typedef enum {
    CLK_SKEW_DL,
    CLK_SKEW_UL,
} CLK_SKEW_DIRECTION_TYPE_t;

typedef enum CLK_SKEW_DL_TYPE_e {
    CLK_SKEW_DL_A2DP = 0,
    CLK_SKEW_DL_ESCO = 1,
    CLK_SKEW_DL_BLE_MUSIC = 2,
} CLK_SKEW_DL_TYPE_t;

typedef enum CLK_SKEW_FS_enum {
    CLK_SKEW_FS_8K              = 8000,
    CLK_SKEW_FS_11_025K         = 11025,
    CLK_SKEW_FS_12K             = 12000,
    CLK_SKEW_FS_16K             = 16000,
    CLK_SKEW_FS_22_05K          = 22050,
    CLK_SKEW_FS_24K             = 24000,
    CLK_SKEW_FS_32K             = 32000,
    CLK_SKEW_FS_44_1K           = 44100,
    CLK_SKEW_FS_48K             = 48000,
    CLK_SKEW_FS_64K             = 64000,
    CLK_SKEW_FS_88_2K           = 88200,
    CLK_SKEW_FS_96K             = 96000,
    CLK_SKEW_FS_192K            = 192000,
} CLK_SKEW_FS_t;

typedef struct dsp_clock_skew_ctrl_s {
    S32 initial_offset;
    S32 initial_drift_acc;
    S16 AccumulatedSamples;
    S16 CompensatedSamples;
    S16 CompensatedPolarity;
    U16 IntrDownCnt;
    U16 BytesPerSample;
    U16 FrameSize;
    BOOL Initialized;
    S16 InterruptHandleCnt;
    U16 UL_Prev_Polarity;
    S16 UL_Pol_Change_Samples;
    S32 IsrDriftAcc;
    S32 IsrCpSamples_Remainder;
    S64 HWSRC_ComSampleRemainder;
    S32 DataQue_32[8];
    S16 DataQue_16[8];
    BOOL PollingFlag;
    S32  isr_interval;//unit : 0.1ms
    clkskew_mode_t ClkSkewMode;
} DSP_CLOCK_SKEW_CTRL_t;

typedef struct rcdc_clk_info_t_s {
    uint32_t  audio_clk;
    uint32_t  native_clk;
} RCDC_BT_CLK_INFO_t;

typedef struct rcdc_clk_offset_info_s {
    U16 handle;
    U32 bt_clk_offset;
    U16 bt_intra_slot_offset;
} PACKED RCDC_CLK_OFFSET_INFO_t;

typedef struct dsp_clk_skew_moving_average_s{
    S32 r_buffer[CLK_SEW_MOVING_AV_ORDER];
    S64 sum;
    S32 index;
    BOOL steady_state;
}DSP_CLK_SEW_MOVING_AV_t;


typedef struct dsp_clock_skew_rcdc_ctrl_s {
    U32 n9_clk_prev;
    U32 n9_clk_next;
    U32 aud_clk_prev;
    U32 aud_clk_next;
    U32 bt_clk_offset_prev;
    U32 bt_clk_offset_next;
    U16 bt_intra_slot_offset_prev;
    U16 bt_intra_slot_offset_next;
    S64 drift_acc;
    S32 prephase;
    BOOL first_offset_flag;
    BOOL initial_offset_flag;
#ifdef ENABLE_HWSRC_CLKSKEW
    U32 frame_cnt;
#endif
    DSP_CLK_SEW_MOVING_AV_t moving_av;
} DSP_CLOCK_SKEW_RCDC_CTRL_t;

typedef struct dsp_clock_skew_pcdc_ctrl_s {
    int asi_cnt;
    int deviation_in_ppm;
    int debug_bytes;
    S16 accumulated_samples;
    S16 compensated_samples;
    S32 remainder_acc;
    S32 prephase;
} DSP_CLOCK_SKEW_PCDC_CTRL_t;

typedef struct dsp_clock_skew_ecdc_ctrl_s {
    U8  update_cnt;
    U8  compensated_cnt;
    U8  write_offset;
    U8  read_offset;
    U16 isr_samples_cnt[isr_samples_cnt_buffer_size];
    U32 observed_samples_acc;
    U32 isr_clk_prev;
    U32 isr_clk_next;
    S32 isr_drift;
    S64 drift_acc;
    S32 cp_samples;
    S32 afe_interval_cnt;
    DSP_CLK_SEW_MOVING_AV_t moving_av;
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) && defined(AIR_DUAL_CHIP_I2S_ENABLE)
    U32 cnt;
    S32 isr_drift_acc;
    U32 isr_bt_clk_prev;
    U32 isr_bt_clk_next;
    U16 isr_bt_phase_prev;
    U16 isr_bt_phase_next;
#endif
} DSP_CLOCK_SKEW_ECDC_CTRL_t;

typedef struct dsp_clock_skew_debug_s {
    S32 rcdcComSample26M_remainder;
    S32 rcdcComSample_acc;
    S32 rcdcComSample_acc_interval;
    S32 pcdcComSample26M_remainder;
    S32 pcdcComSample_acc;
    S32 ecdcComSample26M_remainder;
    S32 ecdcComSample_acc;
    S32 ComSample26M_remainder;
    S32 ComSample_acc;
    U32 debug_cnt;
    U32 debug_cnt_acc;
    S64 rcdcComSample_acc_cal_ppm;

    S32 isr_time_prev;
    S32 isr_drift_acc;
    U32 isr_cnt;
    U32 isr_cnt_th;
} DSP_CLOCK_SKEW_DEBUG_t;

typedef struct dsp_clock_skew_setup_s {
    BOOL ecdc_en;
    BOOL rcdc_en;
    BOOL pcdc_en;
    BOOL rcdc_av_en;
    BOOL rcdc_slow_adjustment_en;
    BOOL calibrate_isr_en;
    BOOL calibrat_isr_debug_en;
    S32  compensate_th_pos;
    S32  compensate_th_neg;
    U32  debug_interval_s;//debug log interval
    U16  clk_skew_order;// C_Flp_Ord_1(with 0 sample delay), C_Flp_Ord_3(with 1 sample delay), C_Flp_Ord_5(with 2 sample delay)
    U16  clk_skew_div_mode;// C_1_mode (1 sample/per frame)     C_div8_mode (1/8 sample/per frame)     C_div64_mode (1/64 sample/per frame)    C_div256_mode (1/256 sample/per frame)    C_div512_mode (1/512 sample/per frame)
    CLK_SKEW_DIRECTION_TYPE_t clk_skew_dir;
} DSP_CLOCK_SKEW_SETUP_t;

typedef struct dsp_clock_skew_param_s {
    DSP_CLOCK_SKEW_SETUP_t      ClkSkewSetup;
    DSP_CLOCK_SKEW_CTRL_t       ClkSkewCtrl;
    DSP_CLOCK_SKEW_RCDC_CTRL_t  ClkSkewRCDCCtrl;
    DSP_CLOCK_SKEW_PCDC_CTRL_t  ClkSkewPCDCCtrl;
    DSP_CLOCK_SKEW_ECDC_CTRL_t  ClkSkewECDCCtrl;
    DSP_CLOCK_SKEW_DEBUG_t      ClkSkewDebug;
} DSP_CLOCK_SKEW_PARAM_t;

#define DSP_CLK_SKEW_MEMSIZE(MaxChannelNum,MaxBytesPerFrame,PcdcEnable) ((((sizeof(DSP_CLOCK_SKEW_PARAM_t) + (MaxBytesPerFrame + DSP_RCDC_MAX_SAMPLE_DEVIATION * 4)) + 8)&0xFFFFFFF0) + (sizeof(skew_ctrl_t)*MaxChannelNum*(PcdcEnable + 1)) + 100)
extern CLK_SKEW_FS_t clk_skew_fs_converter(stream_samplerate_t fs_in);
void Clock_Skew_Initialize(void *para, DSP_CLOCK_SKEW_SETUP_t* ClkSkewSetup, S32 ini_asi);
BOOL Clock_Skew_ECDC_Initialize(void *para, CLK_SKEW_DIRECTION_TYPE_t clk_skew_dir);
S16 Clock_Skew_Get_Comp_Samples(SOURCE source, SINK sink);
S16 Clock_Skew_Get_Comp_Bytes(SOURCE source, SINK sink);
bool pcdc_asi_threshold_counter(SOURCE source, SINK sink, U32 sample_size, int fs);
void pcdc_asi_count_init(SOURCE source, SINK sink, U32 add_amount, S32 sample_rate);
void Clock_Skew_Offset_Update(BT_CLOCK_OFFSET_SCENARIO type, SOURCE source, SINK sink);
#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) && defined(AIR_DUAL_CHIP_I2S_ENABLE)
void Clock_Skew_Isr_Time_Update(SOURCE source, SINK sink, U32 bt_clk_next,U16 bt_phase_next);
#else
void Clock_Skew_Isr_Time_Update(SOURCE source, SINK sink, U32 clk_next, U32 default_samples_cnt);
#endif
void Clock_Skew_Samples_Cnt_Update(SOURCE source, SINK sink, U16 samples_cnt);
BOOL Clock_Skew_HWSRC_Is_Enable(SOURCE source, SINK sink);
BOOL Clock_Skew_ECDC_Is_Enable(SOURCE source, SINK sink);
BOOL Clock_Skew_Get_Polling_Flag(SOURCE source, SINK sink);
void Clock_Skew_Set_Polling_Flag(SOURCE source, SINK sink, BOOL flag);
S16 Clock_Skew_Check_Status_From_SrcSnk(SOURCE source, SINK sink);
void Clock_Skew_Check_Isr_Status_From_SrcSnk(SOURCE source, SINK sink, BTCLK bt_clk, BTPHASE bt_phase);
U32 Clock_Skew_Asrc_Get_Input_SampleSize(SOURCE source, SINK sink);
bool stream_function_clock_skew_process(void *para);
void ClkSkewMode_Selection(hal_ccni_message_t msg, hal_ccni_message_t *ack);

#ifdef ENABLE_HWSRC_CLKSKEW
extern bool ClkSkewMode_g;
extern bool ClkSkewMode_isModify_g;
#endif


#endif /* _CLK_SKEW_COMPENSATION_H_ */

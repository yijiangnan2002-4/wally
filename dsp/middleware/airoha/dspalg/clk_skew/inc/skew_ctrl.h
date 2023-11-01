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

#include <xtensa/tie/xt_core.h>
#include <xtensa/tie/xt_hifi2.h>
#include <xtensa/tie/xt_misc.h>
#include <xtensa/tie/xt_mul.h>
#include "types.h"

#ifndef __SKEW_CTRL_H__
#define __SKEW_CTRL_H__

#define Max_Intp_Ord    5

#define C_Flp_Ord_1     0   // 1-order interploation filter
#define C_Flp_Ord_3     1   // 3-order interploation filter
#define C_Flp_Ord_5     2   // 5-order interploation filter

#define C_1_mode        0   // 1     sample mode    
#define C_div8_mode     1   // 1/8   sample mode 
#define C_div64_mode    2   // 1/64  sample mode 
#define C_div256_mode   3   // 1/256 sample mode 
#define C_div512_mode   4   // 1/512 sample mode


typedef enum {
    C_Skew_Pass   = 0x0,// The fractional sample must be 0(i.e. integer sample), which can switch to use the C_Skew_Pass mode(without interpolation filter) for div1, div8, div64, div256 and div512 mode.
    C_Skew_Pass_1,      // pass during C_Skew_Inc_1 or C_Skew_Dec_1 mode
    C_Skew_Inc_1,       // +1 sample
    C_Skew_Dec_1,       // -1 sample

    C_Skew_Pass_div8,   // pass mode: fractional sample != 0 with interpolation filter during C_Skew_Inc_div8 or C_Skew_Dec_div8 mode
    C_Skew_Inc_div8,    // +1/8 sample
    C_Skew_Dec_div8,    // -1/8 sample

    C_Skew_Pass_div64,  // pass mode: fractional sample != 0 with interpolation filter during C_Skew_Inc_div64 or C_Skew_Dec_div64 mode
    C_Skew_Inc_div64,   // +1/64 sample
    C_Skew_Dec_div64,   // -1/64 sample

    C_Skew_Pass_div256, // pass mode: fractional sample != 0 with interpolation filter during C_Skew_Inc_div256 or C_Skew_Dec_div256 mode
    C_Skew_Inc_div256,  // +1/256 sample
    C_Skew_Dec_div256,  // -1/256 sample

    C_Skew_Pass_div512, // pass mode: fractional sample != 0 with interpolation filter during C_Skew_Inc_p0019531 or C_Skew_Dec_div512 mode
    C_Skew_Inc_div512,  // +1/512 sample
    C_Skew_Dec_div512,  // -1/512 sample

    C_Skew_Inp,         // input = skew_buf_len +/- 1, output = skew_buf_len        , skew_buf_len = framsize * framsize_num
    C_Skew_Oup          // input = skew_buf_len      , output = skew_buf_len +/- 1
    // PS:C_Skew_Pass_1: input  sample length = skew_buf_len (= framsize * framsize_num)
    //                   output sample length = skew_buf_len
    //
    //    C_Skew_Inc_1 : input  sample length = skew_buf_len     @ C_Skew_Oup mode
    //                   output sample length = skew_buf_len + 1 @ C_Skew_Oup mode
    //                   input  sample length = skew_buf_len - 1 @ C_Skew_Inp mode
    //                   output sample length = skew_buf_len     @ C_Skew_Inp mode
    //
    //    C_Skew_Dec_1 : input  sample length = skew_buf_len     @ C_Skew_Oup mode
    //                   output sample length = skew_buf_len - 1 @ C_Skew_Oup mode
    //                   input  sample length = skew_buf_len + 1 @ C_Skew_Inp mode
    //                   output sample length = skew_buf_len     @ C_Skew_Inp mode
} skew_mode_t;

typedef struct {
    U16     bits;               // 16-bit or 32-bit
    U16     framsize;
    U16     framsize_num;
    U16     framsize_cnt;
    U16     skew_buf_cnt;
    U16     order;
    U16     skew_compensation;  // C_Skew_Inc or C_Skew_Dec compensation
    U16     skew_io_mode;       // C_Skew_Inp or C_Skew_Oup
    U32     acc_phase;
    U32     phase;
    U32     phase_inc_1;
    U32     phase_inc_div8;
    U32     phase_inc_div64;
    U32     phase_inc_div256;
    U32     phase_inc_div512;
    U32     phase_dec_1;
    U32     phase_dec_div8;
    U32     phase_dec_div64;
    U32     phase_dec_div256;
    U32     phase_dec_div512;
    ALIGN(8) S32 stack_32b[2 * 2 * Max_Intp_Ord]; // store x[n] temp memory for n-order interpolation filter, *2(stereo) *2(temp buffer)

    S16     phase_mode;
    S16     phase_frac;
    S16     phase_state;        // Inc(>0), Dec(<0)
    ALIGN(8) S32 reserve_;
} skew_ctrl_t, *skew_ctrl_t_ptr;

// skew_ctrl.c
extern int  get_skewctrl_version(void);
extern void skew_ctrl_init(skew_ctrl_t *state, U16 bits, U16 skew_io_mode, U16 order);
extern void skew_ctrl_set_input_framesize(skew_ctrl_t *state, U16 framsize);
extern S16  skew_ctrl_process(skew_ctrl_t *state, void *in_buf_ptr, U16 *in_byte_cnt, void *ou_buf_ptr, U16 *ou_byte_cnt, U16 skew_comp);


// intp_filter.c
S16 intp_filter_16b(skew_ctrl_t *state, S16 *in_pt, S16 step);
S32 intp_filter_32b(skew_ctrl_t *state, S32 *in_pt, S16 step);

#endif //__SKEW_CTRL_H__


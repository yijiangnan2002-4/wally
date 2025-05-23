/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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
#ifndef _DSP_PARA_PLC_NVSTRUC_H_
#define _DSP_PARA_PLC_NVSTRUC_H_
#include "types.h"

typedef struct stru_dsp_mdsp_plc_para_s
{
U8  ENABLE     ;            /**< @Value 0x01 @Desc 1 */
U8  REVISION   ;            /**< @Value 0x01 @Desc 1 */

U16 option     ;            /**< @Value 0x0000 @Desc
                                        b[1] = 1(2-channel     ), 0(1-channel      )
                                        b[0] = 1(Enable PRE_EMP), 0(Disable PRE_EMP)
                                        */
S16 framesize   ;           /**< @Value 0x0000 @Desc 1 */
U16 reserve_1  ;            /**< @Value 0x0000 @Desc 1 */
S16 pitch_lag_max;          /**< @Value 0x0000 @Desc 1 */
S16 pitch_lag_min;          /**< @Value 0x0000 @Desc 1 */
S16 decay_level;           /**< @Value 0x0000 @Desc 1 */

U16 max_period;				/**< @Value 0x0000 @Desc 1 */
U16 debounce_frame_num ;	/*0(disable debounce_frame), max number is up to 16 */
U16 reserve_3 ;				/**< @Value 0x0000 @Desc 1 */
U16 reserve_4 ;				/**< @Value 0x0000 @Desc 1 */
U16 reserve_5 ;				/**< @Value 0x0000 @Desc 1 */
U16 reserve_6 ;				/**< @Value 0x0000 @Desc 1 */
U16 reserve_7 ;				/**< @Value 0x0000 @Desc 1 */
} DSP_PARA_AUDIO_PLC_STRU;

#endif /* _DSP_PARA_PLC_NVSTRUC_H_ */

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

#ifndef __AVM_DIRECT_UTIL_H__
#define __AVM_DIRECT_UTIL_H__
void avm_direct_log_msgid_i(const char *msg, uint32_t arg_cnt, ...);

#define AVM_DIRECT_LOG_MSGID_I(msg, arg_cnt, ...) avm_direct_log_msgid_i(msg, arg_cnt, ## __VA_ARGS__)

/*  "[avm_direct]cal pta duration:0x%x, drift_val:%d, nclk:0x%x, nclk_intra:0x%x, nclk:0x%x,nclk_intra:0x%x";  */
extern const char AVM_DIRECT_004[];
/*  "[avm_direct]: idx: %d, x:0x%08x, y:0x%08x\r\n"  */
extern const char AVM_DIRECT_005[];
/*  "[avm_direct]cal pta, asi_dur:0x%08x, drift:%d, base_nclk: 0x%08x, intra:0x%08x"  */
extern const char AVM_DIRECT_006[];
/*  "[avm_direct]Error, latency value should be the integer multiple of 1250us\r\n"  */
extern const char AVM_DIRECT_007[];
/*  "[avm_direct]m*1000000:%d, b*1000000:%d, r*1000000:%d"  */
extern const char AVM_DIRECT_008[];
/*  "[avm_direct] slope*1000000: %d, drift:%d, drift_temp:%d"  */
extern const char AVM_DIRECT_010[];
/*  "[avm_direct] to cancel audio play en\r\n"  */
extern const char AVM_DIRECT_011[];
/*  "[avm_direct] gap_hd is 0, maybe disconnect happened\r\n"  */
extern const char AVM_DIRECT_012[];
/*  "[avm_direct] ts_base:0x%08x, ts0:0x%08x"  */
extern const char AVM_DIRECT_013[];
#endif

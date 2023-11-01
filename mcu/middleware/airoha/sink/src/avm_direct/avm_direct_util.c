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

#include "syslog.h"

log_create_module(AVM_DIRECT, PRINT_LEVEL_INFO);

#if !defined(MTK_DEBUG_LEVEL_NONE)
ATTR_LOG_STRING_LIB AVM_DIRECT_004[] = LOG_INFO_PREFIX(AVM_DIRECT) "[avm_direct]cal pta duration:0x%x, drift_val:%d, nclk:0x%x, nclk_intra:0x%x, nclk:0x%x,nclk_intra:0x%x";
ATTR_LOG_STRING_LIB AVM_DIRECT_005[] = LOG_INFO_PREFIX(AVM_DIRECT) "[avm_direct]: idx: %d, x:0x%08x, y:0x%08x\r\n";
ATTR_LOG_STRING_LIB AVM_DIRECT_006[] = LOG_INFO_PREFIX(AVM_DIRECT) "[avm_direct]cal pta, asi_dur:0x%08x, drift:%d, base_nclk: 0x%08x, intra:0x%08x";
ATTR_LOG_STRING_LIB AVM_DIRECT_007[] = LOG_INFO_PREFIX(AVM_DIRECT) "[avm_direct]Error, latency value should be the integer multiple of 1250us\r\n";
ATTR_LOG_STRING_LIB AVM_DIRECT_008[] = LOG_INFO_PREFIX(AVM_DIRECT) "[avm_direct]m*1000000:%d, b*1000:%d, r*1000:%d";
ATTR_LOG_STRING_LIB AVM_DIRECT_010[] = LOG_INFO_PREFIX(AVM_DIRECT) "[avm_direct] slope*1000: %d, drift:%d, drift_temp:%d";
ATTR_LOG_STRING_LIB AVM_DIRECT_011[] = LOG_INFO_PREFIX(AVM_DIRECT) "[avm_direct] to cancel audio play en\r\n";
ATTR_LOG_STRING_LIB AVM_DIRECT_013[] = LOG_INFO_PREFIX(AVM_DIRECT) "[avm_direct] ts_dur:0x%08x, ts_base:0x%08x, ts0:0x%08x";
#else /* !defined(MTK_DEBUG_LEVEL_NONE) */
ATTR_LOG_STRING_LIB AVM_DIRECT_004[] = "";
ATTR_LOG_STRING_LIB AVM_DIRECT_005[] = "";
ATTR_LOG_STRING_LIB AVM_DIRECT_006[] = "";
ATTR_LOG_STRING_LIB AVM_DIRECT_007[] = "";
ATTR_LOG_STRING_LIB AVM_DIRECT_008[] = "";
ATTR_LOG_STRING_LIB AVM_DIRECT_010[] = "";
ATTR_LOG_STRING_LIB AVM_DIRECT_011[] = "";
ATTR_LOG_STRING_LIB AVM_DIRECT_013[] = "";
#endif /* !defined(MTK_DEBUG_LEVEL_NONE) */

void avm_direct_log_msgid_i(const char *msg, uint32_t arg_cnt, ...)
{
#ifdef MTK_DEBUG_LEVEL_INFO
    va_list ap;
    va_start(ap, arg_cnt);
    log_print_msgid(&log_control_block_AVM_DIRECT, PRINT_LEVEL_INFO, msg, arg_cnt, ap);
    va_end(ap);
#endif
}
#if 0
void avm_direct_log_msgid_i_f(const char *msg, ...)
{
#ifdef MTK_DEBUG_LEVEL_INFO
    va_list ap;
    va_start(ap, msg);
    vprint_module_log(&log_control_block_AVM_DIRECT, __FUNCTION__, __LINE__, PRINT_LEVEL_INFO, msg, ap);
    va_end(ap);
#endif
}
#endif


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

#ifndef __MUX_PORT_COMMON_H__
#define __MUX_PORT_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

/* CPU number define and limitation, Query current CPU number, always keep primary CPU as number 0 */
#if defined(MTK_CPU_NUMBER_0)
#define GET_CURRENT_CPU_ID()  0
#elif defined(MTK_CPU_NUMBER_1)
#define GET_CURRENT_CPU_ID()  1
#elif defined(MTK_CPU_NUMBER_2)
#define GET_CURRENT_CPU_ID()  2
#elif defined(MTK_CPU_NUMBER_3)
#define GET_CURRENT_CPU_ID()  3
#else
#error "Current CPU number exceed!!"
#endif /* MTK_CPU_NUMBER_0 */

#if defined(MTK_MAX_CPU_NUMBER_1)
#define MTK_MAX_CPU_NUMBER    1
#define MTK_SINGLE_CPU_ENV
#elif defined(MTK_MAX_CPU_NUMBER_2)
#define MTK_MAX_CPU_NUMBER    2
#elif defined(MTK_MAX_CPU_NUMBER_3)
#define MTK_MAX_CPU_NUMBER    3
#elif defined(MTK_MAX_CPU_NUMBER_4)
#define MTK_MAX_CPU_NUMBER    4
#else
#error "Max CPU number exceed!!"
#endif /* MTK_MAX_CPU_NUMBER_1 */

#define PORT_MUX_MAX_CPU_NUMBER 16
#define PORT_MUX_UNUSED(a) (void)a


#define MUX_NORMAL_HANDLE_MAGIC_NUMBER 0x55AA
#define MUX_LL_HANDLE_MAGIC_NUMBER 0xAA55

#ifdef AIR_LOW_LATENCY_MUX_ENABLE
#define is_mux_ll_handle(handle) (((((handle) >> 16)&0xFFFF) == MUX_LL_HANDLE_MAGIC_NUMBER) ? true: false)
#define is_mux_ll_port(port) ((((port) >= MUX_LL_UART_BEGIN) && ( (port) <= MUX_LL_UART_END)) ? true: false)
#endif

#ifdef __cplusplus
}
#endif

#endif


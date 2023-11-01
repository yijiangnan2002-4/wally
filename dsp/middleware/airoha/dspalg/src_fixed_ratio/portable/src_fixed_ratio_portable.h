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

#ifndef _SRC_FIXED_RATIO_PORTABLE_H_
#define _SRC_FIXED_RATIO_PORTABLE_H_

#if defined(AIR_FIXED_RATIO_SRC)

#if defined(AIR_FIXED_RATIO_SRC_USE_PIC)

/* Includes ------------------------------------------------------------------*/
#include "src_fixed_ratio_interface.h"

/* Public define -------------------------------------------------------------*/
/* Public typedef ------------------------------------------------------------*/
typedef int (*get_updn_samp_version_t)(void);
typedef int (*get_updn_samp_memsize_t)(int hq_en);
typedef void (*updn_samp_init_t)(void *updn_st, void *nvkey, int hq_en);
typedef int16_t (*updn_samp_prcs_16b_t)(void *updn_st, int is_upsample, int factor, S16 *InBuf, S16 *OuBuf, S16 sample_cnt);
typedef int16_t (*updn_samp_prcs_32b_t)(void *updn_st, int is_upsample, int factor, S32 *InBuf, S32 *OuBuf, S16 sample_cnt);

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
extern void *g_src_fixed_ratio_export_parameters[];

/* Public functions ----------------------------------------------------------*/
#define get_updn_samp_version ((get_updn_samp_version_t)g_src_fixed_ratio_export_parameters[0])
#define get_updn_samp_memsize ((get_updn_samp_memsize_t)g_src_fixed_ratio_export_parameters[1])
#define updn_samp_init ((updn_samp_init_t)g_src_fixed_ratio_export_parameters[2])
#define updn_samp_prcs_16b ((updn_samp_prcs_16b_t)g_src_fixed_ratio_export_parameters[3])
#define updn_samp_prcs_32b ((updn_samp_prcs_32b_t)g_src_fixed_ratio_export_parameters[4])

#endif /* AIR_FIXED_RATIO_SRC_USE_PIC */

#endif /* AIR_FIXED_RATIO_SRC */

#endif /* _SRC_FIXED_RATIO_PORTABLE_H_ */

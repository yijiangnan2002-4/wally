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
 
#ifndef __HAL_CACHE_INTERNAL_H__
#define __HAL_CACHE_INTERNAL_H__ 
#include "hal_cache.h"

#ifdef HAL_CACHE_MODULE_ENABLED

#include "hal_nvic.h"
#include "hal_nvic_internal.h"

#define CACHE_DISABLE   3
#define CACHE_ENABLE    2
#define CACHE_INIT      1
#define CACHE_IDLE      0

// #define XCHAL_CA_BYPASS         0x2
// #define XCHAL_CA_WRITEBACK      0x4

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t CACHE_STATUS_Type;

typedef union {
    uint32_t w;
} CACHE_CON_Type;

typedef uint32_t CACHE_REGION_EN_Type;          /**< Region Enable Type */

typedef union {
    uint32_t w;
} CACHE_ENTRY_N_Type;

typedef union {
    uint32_t w;
} CACHE_END_ENTRY_N_Type;

typedef struct {
    CACHE_ENTRY_N_Type cache_entry_n;
    CACHE_END_ENTRY_N_Type cache_end_entry_n;
} CACHE_ENTRY_Type;

extern CACHE_STATUS_Type g_cache_status;
extern CACHE_CON_Type g_cache_con;
extern CACHE_REGION_EN_Type g_cache_region_en;
extern CACHE_ENTRY_Type g_cache_entry[HAL_CACHE_REGION_MAX];

#define MTK_CACHE_REGION_SIZE_UNIT  (0x20000000)    /**< CACHE region size must be 512MB aligned */

typedef enum {
    CACHE_INVALIDATE_ALL_LINES = 1,             /**< Invalidate all CACHE line */
    CACHE_INVALIDATE_ONE_LINE_BY_ADDRESS = 2,   /**< Invalidate one line by address */
    CACHE_INVALIDATE_ONE_LINE_BY_SET_WAY = 4,   /**< Invalidate one line by set and way */
    CACHE_FLUSH_ALL_LINES = 9,                  /**< Flush all CACHE lines */
    CACHE_FLUSH_ONE_LINE_BY_ADDRESS = 10,       /**< Flush one line by address */
    CACHE_FLUSH_ONE_LINE_BY_SET_WAY = 12        /**< Flush one line by set and way */
} cache_line_command_t;

#ifdef HAL_SLEEP_MANAGER_ENABLED

/* Save CACHE status before entering deepsleep */
void cache_status_save(void);

/* Restore CACHE status after leaving deepsleep */
void cache_status_restore(void);

#endif

#ifdef __cplusplus
}
#endif

#endif /* HAL_CACHE_MODULE_ENABLED */
#endif /* __HAL_CACHE_INTERNAL_H__ */


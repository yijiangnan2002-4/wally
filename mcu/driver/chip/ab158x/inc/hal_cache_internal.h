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



#ifdef __cplusplus
extern "C" {
#endif

typedef union {
    struct {
        uint32_t MCEN: 1;                       /**< CACHE enable */
        uint32_t MPEN: 1;                       /**< MPU enable */
        uint32_t CNTEN0: 1;                     /**< Hit counter0 enable */
        uint32_t CNTEN1: 1;                     /**< Hit counter1 enable */
        uint32_t MPDEFEN: 1;                    /**< MPU default protection */
        uint32_t _reserved0: 2;                 /**< Reserved */
        uint32_t MDRF: 1;                       /**< Early restart */
        uint32_t CACHESIZE: 2;                  /**< CACHE size */
        uint32_t _reserved1: 22;                /**< Reserved */
    } b;
    uint32_t w;
} CACHE_CON_Type;

typedef uint32_t CACHE_REGION_EN_Type;          /**< Region Enable Type */

typedef union {
    struct {
        uint32_t _reserved0: 5;                 /**< bit 0 */
        uint32_t ATTR: 3;                       /**< Attribute */
        uint32_t C: 1;                          /**< Cacheable bit */
        uint32_t _reserved1: 3;                 /**< Reserved */
        uint32_t BASEADDR: 20;                  /**< Base address of CACHE region */
    } b;
    uint32_t w;
} CACHE_ENTRY_N_Type;

typedef union {
    struct {
        uint32_t _reserved0: 12;                /**< Reserved */
        uint32_t BASEADDR: 20;                  /**< End address of CACHE region */
    } b;
    uint32_t w;
} CACHE_END_ENTRY_N_Type;

typedef struct {
    CACHE_ENTRY_N_Type cache_entry_n;
    CACHE_END_ENTRY_N_Type cache_end_entry_n;
} CACHE_ENTRY_Type;


extern CACHE_CON_Type g_cache_con;
extern CACHE_REGION_EN_Type g_cache_region_en;
extern CACHE_ENTRY_Type g_cache_entry[HAL_CACHE_REGION_MAX];

#define MTK_CACHE_REGION_SIZE_UNIT  (0x1000)    /**< CACHE region size must be 4KB aligned */

typedef enum {
    CACHE_INVALIDATE_ALL_LINES = 1,             /**< Invalidate all CACHE line */
    CACHE_INVALIDATE_ONE_LINE_BY_ADDRESS = 2,   /**< Invalidate one line by address */
    CACHE_INVALIDATE_ONE_LINE_BY_SET_WAY = 4,   /**< Invalidate one line by set and way */
    CACHE_FLUSH_ALL_LINES = 9,                  /**< Flush all CACHE lines */
    CACHE_FLUSH_ONE_LINE_BY_ADDRESS = 10,       /**< Flush one line by address */
    CACHE_FLUSH_ONE_LINE_BY_SET_WAY = 12        /**< Flush one line by set and way */
} cache_line_command_t;

#ifdef HAL_SLEEP_MANAGER_ENABLED

/* NOTE:
 * CACHE_STATUS_BACKUP and CACHE_STATUS_RESTORE must
 * be called before and after sleep, and the stack cannot be used,
 * because the stack is generally on the Cached SYSRAM,
 * which may cause a fault after the cache restore.
 */

/* The cache tag and memory can be kept before and after sleep, so invalidate all is not necessary.
 * The flush all operation is necessary, because the dirty bit of the cache controller
 * is not in the tag RAM, which can ensure that the cache memory and main memory data are the same.
 */
#define CACHE_STATUS_BACKUP(); { \
        /* flush all */ \
        CACHE->CACHE_OP &= ~CACHE_OP_OP_MASK; \
        CACHE->CACHE_OP |= ((CACHE_FLUSH_ALL_LINES << CACHE_OP_OP_OFFSET) | CACHE_OP_EN_MASK); \
        /* cache disable */ \
        CACHE->CACHE_CON &= ~CACHE_CON_MCEN_MASK; \
    }


#define RESTORE_ONE_CACHE_REGION(num); \
        CACHE->CACHE_ENTRY_N[num] = g_cache_entry[num].cache_entry_n.w; \
        CACHE->CACHE_END_ENTRY_N[num] = g_cache_entry[num].cache_end_entry_n.w;


/* restores cache remap, region setting and enable cache controller */
#define CACHE_STATUS_RESTORE(); { \
        *((volatile uint32_t *)0xE0181000) = 0x24200017; \
        *((volatile uint32_t *)0xE0181004) = 0x04200000; \
        *((volatile uint32_t *)0xE0181008) = 0x34200017; \
        *((volatile uint32_t *)0xE018100C) = 0x14200000; \
        RESTORE_ONE_CACHE_REGION(0); \
        RESTORE_ONE_CACHE_REGION(1); \
        RESTORE_ONE_CACHE_REGION(2); \
        RESTORE_ONE_CACHE_REGION(3); \
        RESTORE_ONE_CACHE_REGION(4); \
        RESTORE_ONE_CACHE_REGION(5); \
        RESTORE_ONE_CACHE_REGION(6); \
        RESTORE_ONE_CACHE_REGION(7); \
        RESTORE_ONE_CACHE_REGION(8); \
        RESTORE_ONE_CACHE_REGION(9); \
        RESTORE_ONE_CACHE_REGION(10); \
        RESTORE_ONE_CACHE_REGION(11); \
        RESTORE_ONE_CACHE_REGION(12); \
        RESTORE_ONE_CACHE_REGION(13); \
        RESTORE_ONE_CACHE_REGION(14); \
        RESTORE_ONE_CACHE_REGION(15); \
        CACHE->CACHE_REGION_EN = g_cache_region_en; \
        CACHE->CACHE_CON = g_cache_con.w; \
    }

#endif

#ifdef __cplusplus
}
#endif

#endif /* HAL_CACHE_MODULE_ENABLED */
#endif /* __HAL_CACHE_INTERNAL_H__ */


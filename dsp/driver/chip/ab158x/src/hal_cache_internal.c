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

#include "hal_cache.h"

#ifdef HAL_CACHE_MODULE_ENABLED

#include "hal_cache_internal.h"
//#include <xtensa/hal.h>
// #include "memory_attribute.h"
// #include "memory_map.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ATTR_TEXT_IN_TCM

CACHE_STATUS_Type g_cache_status = CACHE_IDLE;
CACHE_CON_Type g_cache_con;
CACHE_REGION_EN_Type g_cache_region_en;
CACHE_ENTRY_Type g_cache_entry[HAL_CACHE_REGION_MAX];

#ifdef HAL_SLEEP_MANAGER_ENABLED

/* By running _xtos_core_save(XTOS_KEEPON_MEM, xxx) before sleep,
 * the cache tag and cache memory can be kept in the state.
 * The cache memory and cache tag are still valid after sleep,
 * and there is no need to do special flush & invalidate operations.
 */
#define CACHE_MEM_KEEP

/* flush and invalidate before enter deepsleep!!! */
void cache_status_save(void)
{
#ifndef CACHE_MEM_KEEP
    if (g_cache_status == CACHE_ENABLE) {
        /* Flush and Invalidate all dCACHE lines */
        xthal_dcache_all_writeback_inv();
        /* The sync operation insures any previous cache operations are visible to subsequent code */
        xthal_dcache_sync();
        /* Disable dCACHE */
        xthal_dcache_set_ways(0);
        /* The sync operation insures any previous cache operations are visible to subsequent code */
        xthal_dcache_sync();

        /* Invalidate all iCACHE lines */
        xthal_icache_all_invalidate();
        /* The sync operation insures any previous cache operations are visible to subsequent code */
        xthal_icache_sync();
        /* Disable iCACHE */
        xthal_icache_set_ways(0);
        /* The sync operation insures any previous cache operations are visible to subsequent code */
        xthal_icache_sync();
    }
#endif
}

/* restores only regions that are enabled before entering into deepsleep */
void cache_status_restore(void)
{
#ifndef CACHE_MEM_KEEP
    /* Cache have been invalidated in sleep_exit_prepare() */
    /* Cache settings have been restored in _xtos_core_restore() */
    if (g_cache_status == CACHE_ENABLE) {
        /* Enable dCACHE */
        xthal_dcache_set_ways(4);
        /* The sync operation insures any previous cache operations are visible to subsequent code */
        xthal_dcache_sync();

        /* Enable iCACHE */
        xthal_icache_set_ways(4);
        /* The sync operation insures any previous cache operations are visible to subsequent code */
        xthal_icache_sync();
    }
#endif
}

#endif

#ifdef __cplusplus
}
#endif

#endif /* HAL_CACHE_MODULE_ENABLED */


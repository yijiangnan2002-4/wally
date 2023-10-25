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
#include "memory_attribute.h"
#include "memory_map.h"

#ifdef __cplusplus
extern "C" {
#endif


ATTR_ZIDATA_IN_TCM CACHE_CON_Type g_cache_con;
ATTR_ZIDATA_IN_TCM CACHE_REGION_EN_Type g_cache_region_en;
ATTR_ZIDATA_IN_TCM CACHE_ENTRY_Type g_cache_entry[HAL_CACHE_REGION_MAX];

/**
 * sample code
 *
 * how to use cache hit rate
 *
 * @Author : Haonan Li <haonan li@airoha.com.tw>
 */
// hal_cache_hit_rate_t cache_hit_rate;

// hal_cache_hit_rate_start();
// the_function_need_monitor_();  /* user start and stop to monitor this function/ some code flow*/
// hal_cache_hit_rate_stop();
// hal_cache_hit_rate_get(&cache_hit_rate);
// hal_cache_hit_rate_clear(void)
// printf("cache hit rate 0 = %f [%lld/%lld]\r\n", cache_hit_rate.cache_hit_rate0, cache_hit_rate.cache_hit_count0, cache_hit_rate.cache_access_count0);
// printf("cache hit rate 1 = %f [%lld/%lld]\r\n", cache_hit_rate.cache_hit_rate1, cache_hit_rate.cache_hit_count1, cache_hit_rate.cache_access_count1);

#if 0
/* Private typedef -----------------------------------------------------------*/
typedef struct {
    uint64_t cache_hit_count0;
    uint64_t cache_access_count0;
    float cache_hit_rate0;
    uint64_t cache_hit_count1;
    uint64_t cache_access_count1;
    float cache_hit_rate1;
} hal_cache_hit_rate_t;

/* Private functions ---------------------------------------------------------*/
ATTR_TEXT_IN_TCM void hal_cache_hit_rate_start(void)
{
    CACHE->CACHE_CON &= (~CACHE_CON_CNTEN0_MASK) & (~CACHE_CON_CNTEN1_MASK);
    CACHE->CACHE_HCNT0L = 0x0;
    CACHE->CACHE_HCNT0U = 0x0;
    CACHE->CACHE_CCNT0L = 0x0;
    CACHE->CACHE_CCNT0U = 0x0;
    CACHE->CACHE_HCNT1L = 0x0;
    CACHE->CACHE_HCNT1U = 0x0;
    CACHE->CACHE_CCNT1L = 0x0;
    CACHE->CACHE_CCNT1U = 0x0;
    CACHE->CACHE_CON |= CACHE_CON_CNTEN0_MASK | CACHE_CON_CNTEN1_MASK;
}

ATTR_TEXT_IN_TCM void hal_cache_hit_rate_stop(void)
{
    CACHE->CACHE_CON &= (~CACHE_CON_CNTEN0_MASK) & (~CACHE_CON_CNTEN1_MASK);
}

ATTR_TEXT_IN_TCM void hal_cache_hit_rate_get(hal_cache_hit_rate_t *cache_hit_rate)
{
    cache_hit_rate->cache_hit_count0 = ((uint64_t)CACHE->CACHE_HCNT0U << 32) | (uint64_t)(CACHE->CACHE_HCNT0L);
    cache_hit_rate->cache_access_count0 = ((uint64_t)CACHE->CACHE_CCNT0U << 32) | (uint64_t)(CACHE->CACHE_CCNT0L);
    cache_hit_rate->cache_hit_rate0 = (float)cache_hit_rate->cache_hit_count0 / (float)cache_hit_rate->cache_access_count0;
    cache_hit_rate->cache_hit_count1 = ((uint64_t)CACHE->CACHE_HCNT1U << 32) | (uint64_t)(CACHE->CACHE_HCNT1L);
    cache_hit_rate->cache_access_count1 = ((uint64_t)CACHE->CACHE_CCNT1U << 32) | (uint64_t)(CACHE->CACHE_CCNT1L);
    cache_hit_rate->cache_hit_rate1 = (float)cache_hit_rate->cache_hit_count1 / (float)cache_hit_rate->cache_access_count1;
}

ATTR_TEXT_IN_TCM void hal_cache_hit_rate_clear(void)
{
    CACHE->CACHE_HCNT0L = 0x0;
    CACHE->CACHE_HCNT0U = 0x0;
    CACHE->CACHE_CCNT0L = 0x0;
    CACHE->CACHE_CCNT0U = 0x0;
    CACHE->CACHE_HCNT1L = 0x0;
    CACHE->CACHE_HCNT1U = 0x0;
    CACHE->CACHE_CCNT1L = 0x0;
    CACHE->CACHE_CCNT1U = 0x0;
}
#endif
#ifdef __cplusplus
}
#endif

#endif /* HAL_CACHE_MODULE_ENABLED */


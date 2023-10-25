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

#include "air_chip.h"
#include "memory_map.h"
#include "hal_feature_config.h"
#if defined(__GNUC__)
extern unsigned int _tcm_text_start[];
#elif defined(__CC_ARM)
extern unsigned int isr_vector$$Base[];
#elif defined(__ICCARM__)
#pragma section = ".intvec"
#endif
#include "hal_clock_internal.h"

/* ----------------------------------------------------------------------------
   -- Core clock macros
   ---------------------------------------------------------------------------- */
#define CLK_CM4_FREQ_26M      ((uint32_t) 26000000)
#define CLK_CM4_FREQ_52M      ((uint32_t) 52000000)
#define CLK_CM4_FREQ_78M      ((uint32_t) 78000000)
#define CLK_CM4_FREQ_104M     ((uint32_t)104000000)
#define CLK_CM4_FREQ_156M     ((uint32_t)156000000)
#define CLK_CM4_FREQ_208M     ((uint32_t)208000000)
#define CLK_CM4_FREQ_260M     ((uint32_t)260000000)
/* ----------------------------------------------------------------------------
   -- Core clock
   ---------------------------------------------------------------------------- */
uint32_t SystemCoreClock = CLK_CM4_FREQ_260M;

/**
   * @brief systick reload value reloaded via this function.
  *         This function can be called in init stage and system runtime.
  * @param  ticks value to be set
  * @retval 0 means successful
  */
uint32_t SysTick_Set(uint32_t ticks)
{
    uint32_t val;

    /* reload value impossible */
    if ((ticks - 1) > SysTick_LOAD_RELOAD_Msk) {
        return (1);
    }

    /* backup CTRL register */
    val = SysTick->CTRL;

    /* disable sys_tick */
    SysTick->CTRL &= ~(SysTick_CTRL_TICKINT_Msk |
                       SysTick_CTRL_ENABLE_Msk);

    /* set reload register */
    SysTick->LOAD  = ticks - 1;
    SysTick->VAL   = 0;

    /* restore CTRL register */
    SysTick->CTRL = val;

    return (0);
}

/**
   * @brief Update SystemCoreClock variable according to PLL config.
  *         The SystemCoreClock variable stands for core clock (HCLK), which can
  *         be used to setup the SysTick timer or other use.
  * @param  None
  * @retval None
  */
void SystemCoreClockUpdate(void)
{
    SystemCoreClock = get_curr_cpu_freq_hz();
}

/**
  * @brief  Setup system
  *         Initialize the FPU setting, vector table location and faults enabling.
  * @param  None
  * @retval None
  */
void SystemInit(void)
{
    /* FPU settings ------------------------------------------------------------*/
#if ((__FPU_PRESENT == 1) && (__FPU_USED == 1))
    SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2)); /* set CP10, CP11 Full Access in Secure mode */
#ifdef AIR_BTA_IC_PREMIUM_G3
#ifdef AIR_CPU_IN_SECURITY_MODE
    SCB_NS->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2)); /* set CP10, CP11 Full Access in Non-secure mode */
#endif /* AIR_CPU_IN_SECURITY_MODE */
#endif /* ((__FPU_PRESENT == 1) && (__FPU_USED == 1)) */

    SCB->CPACR |= ((3UL << 0 * 2) | (3UL << 1 * 2)); /* set CP0, CP1 Full Access in Secure mode (enable PowerQuad) */
#endif
    /* Configure the Vector Table location add offset address ------------------*/
#if defined(__GNUC__)
    SCB->VTOR  = (uint32_t)(_tcm_text_start);
#elif defined (__CC_ARM)
    SCB->VTOR  = (uint32_t)(isr_vector$$Base);
#elif defined(__ICCARM__)
    SCB->VTOR  = (uint32_t)__section_begin(".intvec");
#endif

#ifdef AIR_BTA_IC_PREMIUM_G3
    /* enable common faults */
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk |
                  SCB_SHCSR_USGFAULTENA_Msk |
                  SCB_SHCSR_SECUREFAULTENA_Msk |
                  SCB_SHCSR_BUSFAULTENA_Msk;
#elif defined (AIR_BTA_IC_PREMIUM_G2)
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk |
                  SCB_SHCSR_USGFAULTENA_Msk |
                  SCB_SHCSR_BUSFAULTENA_Msk;

#endif


    /* Enable div 0 fault. */
    SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk;
}

/**
  * @brief  CACHE preinit
  *         Init CACHE to accelerate region init progress.
  * @param  None
  * @retval None
  */
void CachePreInit(void)
{
#ifdef AIR_BTA_IC_PREMIUM_G2
    /* CACHE disable. */
    CACHE->CACHE_CON = 0x00;

    /* Flush all cache lines. */
    CACHE->CACHE_OP = 0x13;

    /* Invalidate all cache lines. */
    CACHE->CACHE_OP = 0x03;

    /* Set cacheable region.*/
    CACHE->CACHE_ENTRY_N[0] = CM4_BASE | 0x100;
    CACHE->CACHE_END_ENTRY_N[0] = CM4_BASE + CM4_LENGTH;

    CACHE->CACHE_REGION_EN = 1;

    switch (TCM_LENGTH) {
        /* 160K TCM/32K CACHE. */
        case 0x00028000:
            CACHE->CACHE_CON = 0x30D;
            break;
        /* 176K TCM/16K CACHE. */
        case 0x0002C000:
            CACHE->CACHE_CON = 0x20D;
            break;
        /* 184K TCM/8K CACHE. */
        case 0x0002E000:
            CACHE->CACHE_CON = 0x10D;
            break;
        /* 192K TCM/NO CACHE. */
        default:
            break;
    }
#elif defined(AIR_BTA_IC_PREMIUM_G3)
    __ISB();

    /* CACHE enable & set cache memory size */
    switch (TCM_LENGTH) {
        /* 224K TCM/32K CACHE */
        case 0x00038000:
            CACHE->CACHE_CON = 0x30D;
            break;
        /* 240K TCM/16K CACHE */
        case 0x0003C000:
            CACHE->CACHE_CON = 0x20D;
            break;
        /* 248K TCM/8K CACHE */
        case 0x0003E000:
            CACHE->CACHE_CON = 0x10D;
            break;
        /* 256K TCM/NO CACHE */
        default:
            break;
    }

    /* !!! Please note that flush & invalidate all cache lines must be performed
     * on the premise of cache controller enable, otherwise the tag memory
     * may be a random value, which may cause trouble to region init. !!!
     */

    /* Flush all cache lines */
    CACHE->CACHE_OP = 0x13;

    /* Invalidate all cache lines */
    CACHE->CACHE_OP = 0x03;

    /* Set cacheable region */
    CACHE->CACHE_ENTRY_N[0] = ROM_BASE | 0x100;
    CACHE->CACHE_END_ENTRY_N[0] = ROM_BASE + ROM_LENGTH;

    CACHE->CACHE_REGION_EN = 1;

    __ISB();
#elif defined(AIR_BTA_IC_STEREO_HIGH_G3)
    __ISB();

    /* CACHE enable & set cache memory size */
    switch (TCM_LENGTH) {
        /* 192K TCM/32K CACHE */
        case 0x00030000:
            CACHE->CACHE_CON = 0x30D;
            break;
        /* 208K TCM/16K CACHE */
        case 0x00034000:
            CACHE->CACHE_CON = 0x20D;
            break;
        /* 216K TCM/8K CACHE */
        case 0x00036000:
            CACHE->CACHE_CON = 0x10D;
            break;
        /* 224K TCM/NO CACHE */
        default:
            break;
    }

    /* !!! Please note that flush & invalidate all cache lines must be performed
     * on the premise of cache controller enable, otherwise the tag memory
     * may be a random value, which may cause trouble to region init. !!!
     */

    /* Flush all cache lines */
    CACHE->CACHE_OP = 0x13;

    /* Invalidate all cache lines */
    CACHE->CACHE_OP = 0x03;

    /* Set cacheable region */
    CACHE->CACHE_ENTRY_N[0] = ROM_BASE | 0x100;
    CACHE->CACHE_END_ENTRY_N[0] = ROM_BASE + ROM_LENGTH;

    CACHE->CACHE_REGION_EN = 1;

    __ISB();
#else
    #error "Please implement CachePreInit function."
#endif
}


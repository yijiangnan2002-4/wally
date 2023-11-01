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

#include "hal_resource_assignment.h"
#include "hal_ccni_config.h"
#include "hal_mpu.h"
#include "exception_handler.h"

#ifdef MTK_NVDM_ENABLE
#include "nvkey.h"
#include "nvkey_id_list.h"
#endif


#ifdef HAL_RTC_MODULE_ENABLED
#include "hal_rtc.h"
#endif

#ifdef AIR_ICE_DEBUG_ENABLE
#include "hal_ice_debug.h"
#endif

#define EXCEPTION_HW_SEMAPHORE 1
uint32_t EXCEPTION_EVENT_TO_DSP0 = IRQGEN_CM4_TO_DSP0_EXCEPTION;
uint32_t EXCEPTION_EVENT_FROM_DSP0 = IRQGEN_DSP0_TO_CM4_EXCEPTION;

uint32_t EXCEPTION_SHARE_MEMORY_ADDRESS_DSP0 = HW_SYSRAM_PRIVATE_MEMORY_EXCEPTION_START;

#ifdef AIR_BTA_IC_PREMIUM_G3
uint32_t HW_SYSRAM_PRIVATE_MEMORY_NS_EXCEPTION_INFO_START_PORT = HW_SYSRAM_PRIVATE_MEMORY_NS_EXCEPTION_INFO_START;
#endif

static uint32_t exception_wdt_mode = 0;

extern hal_hw_semaphore_status_t hw_semaphore_take(hal_hw_semaphore_id_t id);
extern uint32_t exception_get_reason();

#ifdef HAL_MPU_MODULE_ENABLED
extern hal_mpu_status_t hal_mpu_disable(void);
#endif

#if (EXCEPTION_SLAVES_TOTAL > 0)
extern hal_hw_semaphore_status_t hw_semaphore_take(hal_hw_semaphore_id_t id);
extern hal_hw_semaphore_status_t hw_semaphore_give(hal_hw_semaphore_id_t id);


int32_t exception_hw_semaphore_take()
{
    return hw_semaphore_take(EXCEPTION_HW_SEMAPHORE);
}

int32_t exception_hw_semaphore_give()
{
    return hw_semaphore_give(EXCEPTION_HW_SEMAPHORE);
}

#endif

const uint32_t HAL_GPT_CLOCK_SOURCE_1M_PORT = HAL_GPT_CLOCK_SOURCE_1M;
const uint32_t HAL_GPT_CLOCK_SOURCE_32K_PORT = HAL_GPT_CLOCK_SOURCE_32K;

extern  hal_core_status_t hal_core_status_read(hal_core_id_t id);

int exception_core_status_read()
{
    if (HAL_CORE_EXCEPTION == hal_core_status_read(HAL_CORE_MCU)) {
        return 0;
    } else {
        return -1;
    }
}

int exception_gpt_delay_ms(uint32_t ms)
{
    return hal_gpt_delay_ms(ms);
}

void  exception_disable_mpu(void)
{
#ifdef HAL_MPU_MODULE_ENABLED
    hal_mpu_disable();
#endif
}

void exception_gpt_get_free_run_count(uint32_t clock_source, uint32_t *count)
{
    if (clock_source == HAL_GPT_CLOCK_SOURCE_32K) {
        hal_gpt_get_free_run_count(clock_source, count);
    } else {
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, count);
    }
}

int exception_rtc_set_data(uint16_t offset, const char *buf, uint16_t len)
{
#ifdef HAL_RTC_MODULE_ENABLED
    return hal_rtc_set_data(offset, buf, len);
#else
    return 0;
#endif
}

int  exception_gpt_get_duration_count(uint32_t start_count, uint32_t end_count, uint32_t *duration_count)
{
    return hal_gpt_get_duration_count(start_count, end_count, duration_count);
}

int  exception_ccni_query_event_status(uint32_t  event, uint32_t *data)
{
    return hal_ccni_query_event_status(event, data);
}

void exception_alert_dsp0(void)
{
    if(0 != hal_ccni_set_event(EXCEPTION_EVENT_TO_DSP0, NULL)){
        exception_printf("[exception]:mcu send ccni to dsp fail\r\n");
    }
}

uint32_t exception_memview_dsp0_to_cm4(uint32_t dsp0_address)
{
    return hal_memview_dsp0_to_mcu(dsp0_address);
}

int exception_dump_config_init(void)
{
#if (defined(AIR_BTA_IC_PREMIUM_G2) || defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)) && defined(MTK_NVDM_ENABLE)
    extern exception_config_mode_t exception_config_mode;
    nvkey_status_t nvkey_status = NVKEY_STATUS_OK;

    uint32_t size = 4;
    log_hal_msgid_info("[Dump config]:0x%x\r\n", 1, (unsigned int)exception_config_mode.exception_mode);

    /* get user's exception dump configuartion */
    nvkey_status = nvkey_read_data(NVID_SYS_EXP_DUMP_MODE, (uint8_t *)&exception_config_mode.exception_mode, &size);

    log_hal_msgid_info("exception_nodump:%d\r\n", 1, exception_config_mode.exception_mode_t.exception_nodump);
    log_hal_msgid_info("exception_fulldump_text:%d\r\n", 1, exception_config_mode.exception_mode_t.exception_fulldump_text);
    log_hal_msgid_info("exception_fulldump_binary:%d\r\n", 1, exception_config_mode.exception_mode_t.exception_fulldump_binary);
    log_hal_msgid_info("exception_minidump:%d\r\n", 1, exception_config_mode.exception_mode_t.exception_minidump);
	log_hal_msgid_info("mask_irq_check_assert:%d\r\n", 1, exception_config_mode.exception_mode_t.mask_irq_check_assert);
    log_hal_msgid_info("reset_after_dump:%d\r\n", 1, exception_config_mode.exception_mode_t.reset_after_dump);
    log_hal_msgid_info("wdt_reset_mode:%d\r\n", 1, exception_config_mode.exception_mode_t.wdt_reset_mode);
    log_hal_msgid_info("systemhang_pmic_mode:%d\r\n", 1, exception_config_mode.exception_mode_t.systemhang_pmic_mode);
    log_hal_msgid_info("[Dump config]:0x%x\r\n", 1, (unsigned int)exception_config_mode.exception_mode);

    return nvkey_status;
#else
    return 0;
#endif
}

/* Private functions ---------------------------------------------------------*/
/******************************************************************************/
/*            Exception's wdt timeout Functions                               */
/******************************************************************************/
void exception_feed_wdt(void)
{
#ifdef HAL_WDT_MODULE_ENABLED
    hal_wdt_feed(HAL_WDT_FEED_MAGIC);
#endif
}

void exception_enable_wdt_reset(void)
{
#ifdef HAL_WDT_MODULE_ENABLED
    hal_wdt_config_t wdt_config;
    wdt_config.mode = HAL_WDT_MODE_RESET;
    wdt_config.seconds = 90;
    hal_wdt_disable(HAL_WDT_DISABLE_MAGIC);
    hal_wdt_init(&wdt_config);
    hal_wdt_enable(HAL_WDT_ENABLE_MAGIC);
#endif
    exception_wdt_mode = EXCEPTION_WDT_RESET;
}

void exception_disable_wdt_reset(void)
{
#ifdef HAL_WDT_MODULE_ENABLED
    if (exception_wdt_mode == EXCEPTION_WDT_RESET) {
        hal_wdt_disable(HAL_WDT_DISABLE_MAGIC);
    }
#endif
}

void exception_enable_wdt_interrupt(void)
{
    if ((exception_wdt_mode != EXCEPTION_WDT_INTERRUPT) && (exception_get_reason() != 2)) {
#ifdef HAL_WDT_MODULE_ENABLED
        hal_wdt_config_t wdt_config;
        wdt_config.mode = HAL_WDT_MODE_INTERRUPT;
        wdt_config.seconds = 30;
        hal_wdt_disable(HAL_WDT_DISABLE_MAGIC);
        hal_wdt_init(&wdt_config);
        hal_wdt_enable(HAL_WDT_ENABLE_MAGIC);
#endif
        exception_wdt_mode = EXCEPTION_WDT_INTERRUPT;
    }
}

#ifdef AIR_ICE_DEBUG_ENABLE
bool exception_ice_debug_is_enable(void)
{
    return hal_ice_debug_is_enabled();
}

void exception_ice_debug_test(void)
{
#if defined(AIR_BTA_IC_PREMIUM_G3)
    #define RG_WDT_RETN_DAT5    (0x420B00D4)
    #define RG_WDT_INT_STATUS    (0x420b0050)
#elif defined (AIR_BTA_IC_STEREO_HIGH_G3)
    #define RG_WDT_RETN_DAT5    (0xA20900D4)
    #define RG_WDT_INT_STATUS    (0xA2090050)
#endif
    uint32_t status;
    uint32_t ice_debug_status;

    /*for JTAG debug, waiting RG_RETN_DAT5*/
    while(*(volatile uint32_t *)RG_WDT_RETN_DAT5 == 0x12345678);

    /*for SQC, just use AT CMD sim JTAG behavior*/
    ice_debug_status = (*(volatile uint32_t *)HW_SYSRAM_PRIVATE_MEMORY_DSP_ICE_DEBUG_START) & 0xffff;
    if (0xA1FF == ice_debug_status) {       /*decouple_test_mcu*/
        hal_gpt_delay_ms(5000);
    } else if (0xA3FF == ice_debug_status) { /*stop_sync_test_mcu*/
        /*stop DSP*/
        MCU_CFG_PRI->DSP0CFG_STALL = 1;
        hal_gpt_delay_ms(5000);
        /*release DSP*/
        MCU_CFG_PRI->DSP0CFG_STALL = 0;
    } else if (0xA4FF == ice_debug_status) { /*stop_sync_test_dsp*/
        /*stop DSP*/
        MCU_CFG_PRI->DSP0CFG_STALL = 1;
        hal_gpt_delay_ms(5000);
        /*release DSP*/
        MCU_CFG_PRI->DSP0CFG_STALL = 0;
    }

    /*Clear WDT interrupt status*/
    status = *(volatile uint32_t *)RG_WDT_INT_STATUS;
    status = status;
    hal_wdt_disable_ext(HAL_WDT_DISABLE_MAGIC);
}
#else
bool exception_ice_debug_is_enable(void)
{
    return false;
}
void exception_ice_debug_test(void)
{

}

#endif

/******************************************************************************/
/*            Exception's Reboot Functions                                    */
/******************************************************************************/
__attribute__((weak)) void exception_reboot(void)
{
    /* It is defined as a weak function.
     * It needs to be implemented in project.
     */
#ifdef HAL_WDT_MODULE_ENABLED
    /* Wait there for 3s to ensure uart output done */
    hal_wdt_config_t wdt_config;
    wdt_config.mode = HAL_WDT_MODE_RESET;
    wdt_config.seconds = 3;
    hal_wdt_disable(HAL_WDT_DISABLE_MAGIC);
    hal_wdt_init(&wdt_config);
    hal_wdt_enable(HAL_WDT_ENABLE_MAGIC);
    /* In exception flow,uart mode is polling,so it's no need to wait for 3s */
    //hal_wdt_software_reset();
    while (1);
#endif
    return;
}


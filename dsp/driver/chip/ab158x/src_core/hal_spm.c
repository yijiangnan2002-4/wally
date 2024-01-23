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

#include "hal_spm.h"

#ifdef HAL_SLEEP_MANAGER_ENABLED

#include <stdio.h>
#include <string.h>
#include "syslog.h"
#include "memory_attribute.h"
#include "hal_platform.h"
#include "hal_gpt.h"

log_create_module(SPM, PRINT_LEVEL_INFO);
#define SPM_LOG_I(fmt,cnt,arg...)          LOG_I(SPM,fmt,cnt,##arg)
#define SPM_LOG_W(fmt,cnt,arg...)          LOG_W(SPM,fmt,cnt,##arg)
#define SPM_LOG_E(fmt,cnt,arg...)          LOG_E(SPM,fmt,cnt,##arg)
#define SPM_MSGID_I(fmt,cnt,arg...)        LOG_MSGID_I(SPM,fmt,cnt,##arg)
#define SPM_MSGID_W(fmt,cnt,arg...)        LOG_MSGID_W(SPM,fmt,cnt,##arg)
#define SPM_MSGID_E(fmt,cnt,arg...)        LOG_MSGID_E(SPM,fmt,cnt,##arg)

static int hw_latency_time_32k;

void spm_irq_handler(hal_nvic_irq_t irq_number)
{
    (void)irq_number;
    SPM_SET_DSP0_SW_STATE(0xF);
    *SPM_PCM_SW_INT_CLEAR_DSP = 1;
}

int spm_init(void)
{
    int result = 0;
    hal_nvic_register_isr_handler(SPM_IRQn, spm_irq_handler);
    hal_nvic_enable_irq(SPM_IRQn);

    hw_latency_time_32k = spm_latency_time_checking(); //SPM Latency Time
    //hw_latency_time_ms = (uint32_t)((((float)hw_latency_time_32k) * (1/32.768))) + 1;

    return (result);

}

void spm_mask_wakeup_source(uint32_t wakeup_source)
{
    /* SPM Clock Force to 26M bar, *SPM_CLK_SW_CON &= ~(1 << 8) */
    //*SPM_CLK_SW_CON &= ~(1 << 8);
    if (wakeup_source == HAL_SLEEP_MANAGER_WAKEUP_SOURCE_ALL) {
        *SPM_DSP_WAKEUP_SOURCE_MASK = 0xFF;
    } else {
        *SPM_DSP_WAKEUP_SOURCE_MASK |= (1 << wakeup_source);
    }
}

void spm_unmask_wakeup_source(uint32_t wakeup_source)
{
    /* SPM Clock Force to 26M bar, *SPM_CLK_SW_CON &= ~(1 << 8) */
    //*SPM_CLK_SW_CON &= ~(1 << 8);
    if (wakeup_source == HAL_SLEEP_MANAGER_WAKEUP_SOURCE_ALL) {
        *SPM_DSP_WAKEUP_SOURCE_MASK = 0x00;
    } else {
        *SPM_DSP_WAKEUP_SOURCE_MASK &= ~(1 << wakeup_source);
    }
}

int spm_latency_time_checking(void)
{
    int latency_time_32k;
    float hw_buffer_time = 1; //ms
    float hw_buffer_time_32k = 0;
    float hw_buffer_time_freq = 32.768;
    float latency_time_max_ms = 8; /* MaximumIdleTime - DEEP_SLEEP_SW_BACKUP_RESTORE_TIME */
    float latency_time_min_ms = 2;
    float latency_time_max_32k;
    float latency_time_min_32k;

    uint32_t xo_en_delay, osc_en_delay, pmu_en_delay, en_delay_max;

    uint32_t latency_time_check;
    uint32_t latency_time_error = 0xFFFFFFFF;

    osc_en_delay = ((*SPM_SEQUENCER_32K_REG_3 >> 0) & 0xFF);
    pmu_en_delay = ((*SPM_SEQUENCER_32K_REG_3 >> 8) & 0xFF);
    xo_en_delay = ((*SPM_SEQUENCER_32K_REG_3 >> 16) & 0xFF);
    en_delay_max = osc_en_delay;

    if(pmu_en_delay >= en_delay_max){
        en_delay_max = pmu_en_delay;
    }else if(xo_en_delay >= en_delay_max){
        en_delay_max = xo_en_delay;
    }

    hw_buffer_time_32k = (hw_buffer_time / (1 / (hw_buffer_time_freq)));
    latency_time_max_32k = (latency_time_max_ms / (1 / (hw_buffer_time_freq))) - 1; /* hw_latency_time_ms round up */
    latency_time_min_32k = (latency_time_min_ms / (1 / (hw_buffer_time_freq)));

    latency_time_check = (*SPM_SEQUENCER_32K_REG_1 + *SPM_SEQUENCER_32K_REG_2 + en_delay_max + *SPM_SEQUENCER_32K_REG_4);
    latency_time_32k = ((int)latency_time_check + (int)hw_buffer_time_32k);

    if(latency_time_32k < (int)latency_time_min_32k || latency_time_32k > (int)latency_time_max_32k ){
        SPM_MSGID_E("[dsp]spm_latency_time_checking is error, max of latency time is %d in 32k", 1, (int)latency_time_max_32k);

        return(latency_time_error);
    }

    SPM_MSGID_I("[SLP] SPM_SEQUENCER_32K_REG is 0x%08x, 0x%08x, 0x%08x, 0x%08x, latency_time_32k is %d", 5, 
                *SPM_SEQUENCER_32K_REG_1,
                *SPM_SEQUENCER_32K_REG_2,
                *SPM_SEQUENCER_32K_REG_3,
                *SPM_SEQUENCER_32K_REG_4,
                latency_time_32k
               );

    return(latency_time_32k);
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void spm_mtcmos_vow_on(void)
{

    //bool clock_old_status1;
    uint32_t mask;

    /* SPM Clock Force to 26M bar, *SPM_CLK_SW_CON &= ~(1 << 0) */
    //*SPM_CLK_SW_CON &= ~(1 << 0);

    SPM_MSGID_I("[dsp] spm vow mtcmos control on", 0);

    if ((*SPM_PWR_STATUS & 0x200) != 0) {
        SPM_MSGID_I("spm_mtcmos_vow is already on\r\n", 0);
        return;
    }

    hal_nvic_save_and_set_interrupt_mask(&mask);

    // clock_old_status1 = hal_clock_is_enabled(HAL_CLOCK_CG_AUD_VOW_BUS);
    // if (clock_old_status1 == false) {
    //     hal_clock_enable(HAL_CLOCK_CG_AUD_VOW_BUS);
    // }

    /*TINFO = " --- VOW power on by CMSYS API "*/
    //*VOW_PWR_CON = 0x12; sleep
    *SPM_VOW_PWR_CON = 0x16; // [2]: pwr_on = 1
    hal_gpt_delay_us(1); // 1us
    *SPM_VOW_PWR_CON = 0x1E; // [3]: pwr_on_2nd = 1
    hal_gpt_delay_us(1); // 1us

    // --- VOW SRAM begin ------
    //*VOW_SRAM_CONTROL_1 = 0x01010100;//[24]:MEM_ISO_EN [16]:ISOINTB [8]:SLEEPB [0]:PD

    // --- VOW SRAM end ------
    *SPM_VOW_PWR_CON = 0xE;   // [4]: clk_dis = 0
    hal_gpt_delay_us(1); // 1us
    *SPM_VOW_PWR_CON = 0x1E;  // [4]: clk_dis = 1
    hal_gpt_delay_us(1); // 1us
    *SPM_VOW_PWR_CON = 0x1C;  // [4]: clk_dis = 1, iso = 0
    hal_gpt_delay_us(1); // 1us
    *SPM_VOW_PWR_CON = 0x1D;  // [0]: rstb = 1
    hal_gpt_delay_us(1); // 1us

    // --- VOW SRAM MEM_ISO_EN begin ------
    //*VOW_SRAM_CONTROL_1 = 0x00010100;//[24]:MEM_ISO_EN [16]:ISOINTB [8]:SLEEPB [0]:PD

    // --- VOW SRAM end ------
    *SPM_VOW_PWR_CON = 0xD;   // [4]: clk_dis = 0
    hal_gpt_delay_us(1); // 1us

    //turn VOW protect_en=0
    *VOW_PROT_EN = 0x0;
    //wait VOW protect ready=0
    while (*VOW_PROT_RDY != 0x0);

    // if (clock_old_status1 == false) {
    //     hal_clock_disable(HAL_CLOCK_CG_AUD_VOW_BUS);
    // }

    hal_nvic_restore_interrupt_mask(mask);

}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void spm_mtcmos_vow_off(void)
{

    //bool clock_old_status1;
    uint32_t mask;

    /* SPM Clock Force to 26M bar, *SPM_CLK_SW_CON &= ~(1 << 0) */
    //*SPM_CLK_SW_CON &= ~(1 << 0);

    SPM_MSGID_I("[dsp] spm vow mtcmos control off", 0);

    if ((*SPM_PWR_STATUS & 0x200) == 0) {
        SPM_MSGID_I("spm_mtcmos_vow is already off\r\n", 0);
        return;
    }

    hal_nvic_save_and_set_interrupt_mask(&mask);

    //clock_old_status1 = hal_clock_is_enabled(HAL_CLOCK_CG_AUD_VOW_BUS);
    //if (clock_old_status1 == false) {
    //    hal_clock_enable(HAL_CLOCK_CG_AUD_VOW_BUS);
    //}

    //turn VOW protect_en=1
    *VOW_PROT_EN = 0x1;
    //wait VOW protect ready=1
    while (*VOW_PROT_RDY != 0x1);

    /*TINFO = " --- VOW power down by CMSYS API "*/
    *SPM_VOW_PWR_CON = 0x1D; // [4]: clk_dis = 1
    hal_gpt_delay_us(1); // 1us
    *SPM_VOW_PWR_CON = 0x1F; // [1]: iso = 1
    hal_gpt_delay_us(1); // 1us

//// --- VOW SRAM Power Down begin ------

//*VOW_SRAM_CONTROL_1 = 0x01010100;//[24]:MEM_ISO_EN=1
//*VOW_SRAM_CONTROL_1 = 0x01000100;//[16]:ISOINTB=0
//  *MDM_TM_WAIT_US = 1; // wait isointb to sleepb  > 1us
//*VOW_SRAM_CONTROL_1 = 0x01000001;//[8]:SLEEPB=0 [0]:PD=1
//// --- VOW SRAM end Power Down------

    *SPM_VOW_PWR_CON = 0x1E; // [0]: rstb = 0
    hal_gpt_delay_us(1); // 1us
    *SPM_VOW_PWR_CON = 0x1A; // [2]: pwr_on = 0
    hal_gpt_delay_us(1); // 1us
    *SPM_VOW_PWR_CON = 0x12; // [3]: pwr_on_2nd = 0
    hal_gpt_delay_us(1); // 1us

    //if (clock_old_status1 == false) {
    //    hal_clock_disable(HAL_CLOCK_CG_AUD_VOW_BUS);
    //}

    hal_nvic_restore_interrupt_mask(mask);

}

#define SPM_FPGA_DVT 0
#if SPM_FPGA_DVT
uint32_t SW_GPT_Handler;

void SW_GPT_callback(void)
{

    //printf("SW GPT callback!!");
    //hal_gpt_stop_timer(HAL_GPT_7); /* stop timer clear gpt irq */
    //hal_gpt_start_timer_ms(HAL_GPT_7, 1000000, HAL_GPT_TIMER_TYPE_ONE_SHOT);

    hal_gpt_sw_stop_timer_ms(SW_GPT_Handler); /* stop timer clear gpt irq */
    //hal_gpt_sw_start_timer_ms(SW_GPT_Handler, 2000, SW_GPT_callback, NULL);

}

void spm_dvt_test_case_deep_sleep()
{
    uint32_t mask, count = 0;

    int i;


    //for (i = 0; i < 31; i++) {
    //    hal_nvic_disable_irq(i);
    //}
    //hal_gpt_init(HAL_GPT_7);
    //hal_gpt_register_callback(HAL_GPT_7, HW_GPT_callback, NULL);
    hal_gpt_sw_get_timer(&SW_GPT_Handler);

    printf("sleep flow start test\r\n");

    *SPM_DSP_WAKEUP_SOURCE_MASK = 0xFFFFFFFE;
    while (1) {
        //hal_gpt_start_timer_us(HAL_GPT_7, 1000000, HAL_GPT_TIMER_TYPE_ONE_SHOT);
        hal_gpt_sw_start_timer_ms(SW_GPT_Handler, 3000, SW_GPT_callback, NULL); //32kHz clk for sw gpt
        hal_nvic_enable_irq(GPT_IRQn);

        /* critical section start */
        mask = (uint32_t)XTOS_SET_INTLEVEL(4); // hal_nvic_save_and_set_interrupt_mask(&mask);
        printf("DSP enter sleep\r\n");
        hal_sleep_manager_enter_sleep_mode(HAL_SLEEP_MODE_SLEEP);
        printf("DSP wakeup\r\n");
        /* Re-enable interrupts (critical section end) */
        XTOS_RESTORE_INTLEVEL(mask); // hal_nvic_restore_interrupt_mask(mask);

        if (count++ > 20) {
            while (1);
        }
    }
}

#endif

#endif

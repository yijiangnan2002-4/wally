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
 
#ifndef __HAL_SPM_H__
#define __HAL_SPM_H__

#include "hal_platform.h"
#include "hal_clock_platform.h"

#ifdef HAL_SLEEP_MANAGER_ENABLED

#include "stdio.h"
#include "hal_uart.h"
#include "hal_nvic.h"
#include "hal_nvic_internal.h"
#include <xtensa/hal.h>
#include <xtensa/xtruntime.h>
#include <xtensa/hal.h>

#ifdef AIR_CPU_IN_SECURITY_MODE
#define SPM_BASE    (0x42140000 ) /* All in Secure Environment */
#else
#define SPM_BASE    (0x52140000 ) /* SPM Non-secure Slave in Limited Trust Zone */
#endif

#define SPM_CMSYS_WAKEUP_SOURCE_MASK               (volatile uint32_t*)(SPM_BASE + 0x0000)
#define SPM_AUDIO_I2S_WAKEUP_SOURCE_MASK           (volatile uint32_t*)(SPM_BASE + 0x0004)
#define SPM_CONN_WAKEUP_SOURCE_MASK                (volatile uint32_t*)(SPM_BASE + 0x0008)
#define SPM_DSP_WAKEUP_SOURCE_MASK                 (volatile uint32_t*)(SPM_BASE + 0x000C)
#define SPM_POWER_ON_VAL0                          (volatile uint32_t*)(SPM_BASE + 0x0010)
#define SPM_POWER_ON_VAL1                          (volatile uint32_t*)(SPM_BASE + 0x0014)
#define SPM_SEQUENCER_32K_REG_0                    (volatile uint32_t*)(SPM_BASE + 0x0100)
#define SPM_SEQUENCER_32K_REG_1                    (volatile uint32_t*)(SPM_BASE + 0x0104)
#define SPM_SEQUENCER_32K_REG_2                    (volatile uint32_t*)(SPM_BASE + 0x0108)
#define SPM_SEQUENCER_32K_REG_3                    (volatile uint32_t*)(SPM_BASE + 0x010C)
#define SPM_SEQUENCER_32K_REG_4                    (volatile uint32_t*)(SPM_BASE + 0x0110)
#define SPM_SEQUENCER_32K_REG_5                    (volatile uint32_t*)(SPM_BASE + 0x0114)
#define SPM_FSM_XO_REG_6                           (volatile uint32_t*)(SPM_BASE + 0x0118)
#define SPM_SEQUENCER_26M_REG_0                    (volatile uint32_t*)(SPM_BASE + 0x0140)
#define SPM_SEQUENCER_26M_REG_1                    (volatile uint32_t*)(SPM_BASE + 0x0144)
#define SPM_SEQUENCER_26M_REG_2                    (volatile uint32_t*)(SPM_BASE + 0x0148)
#define SPM_IGNORE_CPU_ACTIVE                      (volatile uint32_t*)(SPM_BASE + 0x0150)
#define SPM_CLK_HANDLE_SELECT                      (volatile uint32_t*)(SPM_BASE + 0x0170)
#define SPM_CMSYS_REQUEST_XO_CON                   (volatile uint32_t*)(SPM_BASE + 0x0174)
#define SPM_DSP_REQUEST_XO_CON                     (volatile uint32_t*)(SPM_BASE + 0x0178)
#define SPM_REQUEST_XO_MASK                        (volatile uint32_t*)(SPM_BASE + 0x0180)
#define SPM_REQUEST_XO_GATE                        (volatile uint32_t*)(SPM_BASE + 0x0184)
#define SPM_SELECT_XO_RDY_SRC                      (volatile uint32_t*)(SPM_BASE + 0x0188)
#define SPM_XO_SETTLE_TIME                         (volatile uint32_t*)(SPM_BASE + 0x018C)
#define SPM_XO_RESOURCE_TIMING                     (volatile uint32_t*)(SPM_BASE + 0x0190)
#define SPM_XO_OUT_OFF_SETTLE_32K_SEL              (volatile uint32_t*)(SPM_BASE + 0x0194)
#define SPM_SPM_EMI_GENA                           (volatile uint32_t*)(SPM_BASE + 0x01A0)
#define SPM_SPM_EMI_DSRAM                          (volatile uint32_t*)(SPM_BASE + 0x01A4)
#define SPM_PCM_SW_INT_CLEAR_DSP                   (volatile uint32_t*)(SPM_BASE + 0x1030)
#define SPM_DUMMY_REG_A                            (volatile uint32_t*)(SPM_BASE + 0x0CBC)
#define SPM_DUMMY_REG_B                            (volatile uint32_t*)(SPM_BASE + 0x0CC0)
#define SPM_DSP_WAKEUP_SOURCE_STA_IN_B             (volatile uint32_t*)(SPM_BASE + 0x07F0)
#define SPM_DSP_WAKEUP_SOURCE_STA                  (volatile uint32_t*)(SPM_BASE + 0x07F4)
#define SPM_DSP_WAKEUP_SOURCE_STA_32K              (volatile uint32_t*)(SPM_BASE + 0x07F8)
#define SPM_PCM_RESERVE2                           (volatile uint32_t*)(SPM_BASE + 0x0B04)
#define SPM_PWR_STATUS                             (volatile uint32_t*)(SPM_BASE + 0x0808)
#define SPM_VOW_PWR_CON                            (volatile uint32_t*)(SPM_BASE + 0x1004)

#ifdef AIR_CPU_IN_SECURITY_MODE  /* check to Asa */
#define CKSYS_XO_CLK_BASE  0x42040000
#else
#define CKSYS_XO_CLK_BASE  0x52040000
#endif
#define SPM_CLK_SW_CON                        (volatile uint32_t *)(CKSYS_XO_CLK_BASE + 0xC50)

#ifdef AIR_CPU_IN_SECURITY_MODE
#define INFRA_MISC_CFG 0x422D0000
#else
#define INFRA_MISC_CFG 0x522D0000
#endif
#define    VOW_PROT_EN                             ((volatile uint32_t*)(INFRA_MISC_CFG + 0x80))
#define    VOW_PROT_RDY                            ((volatile uint32_t*)(INFRA_MISC_CFG + 0x84))


#define HAL_SPM_DEEBUG_ENABLE
#ifdef  HAL_SPM_DEEBUG_ENABLE
#define log_hal_debug(_message,...)               printf(_message, ##__VA_ARGS__)
#define SPM_SET_DSP0_SW_STATE(x)                  *((volatile uint32_t*)(SPM_BASE + 0x0810)) = (x + 0x90)
#else
#define log_hal_debug(_message,...)
#define SPM_SET_DSP0_SW_STATE(x)
#endif

#ifdef  HAL_SPM_DCM_ENABLE
#define SPM_CLOCK_FORCE_ON(x)                     *((volatile uint32_t*)(SPM_CLK_SW_CON)) &= (~(1 << x))
#else
#define SPM_CLOCK_FORCE_ON(x)
#endif

#define SPM_SET_LOCK_INFRA              *((volatile uint32_t*)(SPM_BASE + 0x0C00)) = *((volatile uint32_t*)(SPM_BASE + 0x0C00)) | 0x1
#define SPM_CLEAR_LOCK_INFRA            *((volatile uint32_t*)(SPM_BASE + 0x0C00)) = *((volatile uint32_t*)(SPM_BASE + 0x0C00)) & 0xFFFFFFFE
#define SPM_INFRA_OFF_FLAG              *((volatile uint32_t*)(SPM_BASE + 0x0CB4))

int spm_init(void);
int spm_latency_time_checking(void);
void spm_mask_wakeup_source(uint32_t wakeup_source);
void spm_unmask_wakeup_source(uint32_t wakeup_source);
void spm_mtcmos_vow_on(void);
void spm_mtcmos_vow_off(void);

#define SPM_FPGA_DVT 0
#if SPM_FPGA_DVT
void SW_GPT_callback(void);
void spm_dvt_test_case_deep_sleep(void);
void spm_dvt_dsp_basic_configuration(void);
#endif
#endif
#endif

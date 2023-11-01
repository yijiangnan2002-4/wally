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

#ifndef __HAL_CLOCK_PLATFORM_AB155X_H__
#define __HAL_CLOCK_PLATFORM_AB155X_H__

#include "hal_platform.h"
#include "air_chip.h"
#ifdef HAL_CLOCK_MODULE_ENABLED
#define RF_DESENSE  1
#define STR_VALUE(arg)      #arg
#define __stringify(name) STR_VALUE(name)
#ifdef AIR_CPU_IN_SECURITY_MODE
#define SEC_BIT 0x0
#else
#define SEC_BIT 0x10000000
#endif


/* Module Name: cksys, Base Address: 0x42030000 */
#define CKSYS_BASE                      ((volatile uint32_t *)(0x42030000 + SEC_BIT))
#define SYS_FREE_DCM_CON                ((volatile uint32_t *)(0x42030130 + SEC_BIT))
#define SFC_DCM_CON_0                   ((volatile uint32_t *)(0x42030140 + SEC_BIT))
#define SFC_DCM_CON_1                   ((volatile uint32_t *)(0x42030144 + SEC_BIT))
#define ESC_DCM_CON_0                   ((volatile uint32_t *)(0x42030148 + SEC_BIT))
#define ESC_DCM_CON_1                   ((volatile uint32_t *)(0x4203014C + SEC_BIT))
#define CKSYS_TST_SEL_0                 ((volatile uint32_t *)(0x42030220 + SEC_BIT))
#define CKSYS_TST_SEL_1                 ((volatile uint32_t *)(0x42030224 + SEC_BIT))
#define CKSYS_TST_SEL_2                 ((volatile uint32_t *)(0x42030228 + SEC_BIT))
#define CKSYS_CLK_CFG_0                 ((volatile uint32_t *)(0x42030230 + SEC_BIT))
#define CKSYS_CLK_CFG_1                 ((volatile uint32_t *)(0x42030234 + SEC_BIT))
#define CKSYS_CLK_CFG_2                 ((volatile uint32_t *)(0x42030238 + SEC_BIT))
#define CKSYS_CLK_CFG_3                 ((volatile uint32_t *)(0x4203023C + SEC_BIT))
#define CKSYS_CLK_CFG_4                 ((volatile uint32_t *)(0x42030240 + SEC_BIT))
#define CKSYS_CLK_CFG_5                 ((volatile uint32_t *)(0x42030244 + SEC_BIT))
#define CKSYS_CLK_UPDATE_0              ((volatile uint32_t *)(0x42030250 + SEC_BIT))
#define CKSYS_CLK_UPDATE_1              ((volatile uint32_t *)(0x42030254 + SEC_BIT))
#define CKSYS_CLK_UPDATE_2              ((volatile uint32_t *)(0x42030258 + SEC_BIT))
#define CKSYS_CLK_UPDATE_3              ((volatile uint32_t *)(0x4203025C + SEC_BIT))
#define CKSYS_CLK_UPDATE_STATUS_0       ((volatile uint32_t *)(0x42030260 + SEC_BIT))
#define CKSYS_CLK_UPDATE_STATUS_1       ((volatile uint32_t *)(0x42030264 + SEC_BIT))
#define CKSYS_CLK_UPDATE_STATUS_2       ((volatile uint32_t *)(0x42030268 + SEC_BIT))
#define CKSYS_CLK_UPDATE_STATUS_3       ((volatile uint32_t *)(0x4203026C + SEC_BIT))
#define CKSYS_CLK_FORCE_ON_0            ((volatile uint32_t *)(0x42030270 + SEC_BIT))
#define CKSYS_CLK_FORCE_ON_1            ((volatile uint32_t *)(0x42030274 + SEC_BIT))
#define CKSYS_CLK_FORCE_ON_2            ((volatile uint32_t *)(0x42030278 + SEC_BIT))
#define CKSYS_CLK_FORCE_ON_3            ((volatile uint32_t *)(0x4203027C + SEC_BIT))
#define CKSYS_CLK_DIV_0                 ((volatile uint32_t *)(0x42030280 + SEC_BIT))
#define CKSYS_CLK_DIV_1                 ((volatile uint32_t *)(0x42030284 + SEC_BIT))
#define CKSYS_CLK_DIV_2                 ((volatile uint32_t *)(0x42030288 + SEC_BIT))
#define CKSYS_CLK_DIV_3                 ((volatile uint32_t *)(0x4203028C + SEC_BIT))
#define CKSYS_CLK_DIV_4                 ((volatile uint32_t *)(0x42030290 + SEC_BIT))
#define CKSYS_CLK_DIV_5                 ((volatile uint32_t *)(0x42030294 + SEC_BIT))
#define CKSYS_REF_CLK_SEL               ((volatile uint32_t *)(0x420302A0 + SEC_BIT))
#define GPIO_CLKO_CTRL_A                ((volatile uint32_t *)(0x42030310 + SEC_BIT))
#define PLL_ABIST_FQMTR_CON0            ((volatile uint32_t *)(0x42030400 + SEC_BIT))
#define PLL_ABIST_FQMTR_CON1            ((volatile uint32_t *)(0x42030404 + SEC_BIT))
#define PLL_ABIST_FQMTR_CON2            ((volatile uint32_t *)(0x42030408 + SEC_BIT))
#define PLL_ABIST_FQMTR_DATA            ((volatile uint32_t *)(0x4203040C + SEC_BIT))
#define CKSYS_MISC_0                    ((volatile uint32_t *)(0x42030500 + SEC_BIT))
#define RG_AUD_CKDIV_CFG0               ((volatile uint32_t *)(0x42030300 + SEC_BIT))

/* Module Name: cksys_xo_clk, Base Address: 0x42040000 */
#define XO_PDN_PD_COND0         ((volatile uint32_t *)(0x42040B00 + SEC_BIT))
#define XO_PDN_PD_SETD0         ((volatile uint32_t *)(0x42040B10 + SEC_BIT))
#define XO_PDN_PD_CLRD0         ((volatile uint32_t *)(0x42040B20 + SEC_BIT))
#define XO_PDN_AO_COND0         ((volatile uint32_t *)(0x42040B30 + SEC_BIT))
#define XO_PDN_AO_SETD0         ((volatile uint32_t *)(0x42040B40 + SEC_BIT))
#define XO_PDN_AO_CLRD0         ((volatile uint32_t *)(0x42040B50 + SEC_BIT))
#define XO_PDN_TOP_COND0        ((volatile uint32_t *)(0x42040B60 + SEC_BIT))
#define XO_PDN_TOP_SETD0        ((volatile uint32_t *)(0x42040B70 + SEC_BIT))
#define XO_PDN_TOP_CLRD0        ((volatile uint32_t *)(0x42040B80 + SEC_BIT))
#define XO_PDN_TOP_COND1        ((volatile uint32_t *)(0x42040B90 + SEC_BIT))
#define XO_PDN_TOP_SETD1        ((volatile uint32_t *)(0x42040BA0 + SEC_BIT))
#define XO_PDN_TOP_CLRD1        ((volatile uint32_t *)(0x42040BB0 + SEC_BIT))
#define XO_DCM_CON_0            ((volatile uint32_t *)(0x42040C00 + SEC_BIT))
#define XO_DCM_CON_1            ((volatile uint32_t *)(0x42040C04 + SEC_BIT))
#define SPM_CLK_SW_CON          ((volatile uint32_t *)(0x42040C50 + SEC_BIT))

/* Module Name: mixedsys_d, Base Address: 0x42050000 */
#define CLKSQ_CON0              ((volatile uint32_t *)(0x42050000 + SEC_BIT))
#define CLKSQ_CON1              ((volatile uint32_t *)(0x42050004 + SEC_BIT))
#define CLKSQ_CON2              ((volatile uint32_t *)(0x42050008 + SEC_BIT))
#define CLKSQ_CON3              ((volatile uint32_t *)(0x4205000C + SEC_BIT))
#define CLKSQ_CON4              ((volatile uint32_t *)(0x42050010 + SEC_BIT))
#define CLKSQ_CON5              ((volatile uint32_t *)(0x42050014 + SEC_BIT))
#define CLKSQ_CON6              ((volatile uint32_t *)(0x42050018 + SEC_BIT))
#define UPLL_CON0               ((volatile uint32_t *)(0x42050100 + SEC_BIT))
#define UPLL_CON1               ((volatile uint32_t *)(0x42050104 + SEC_BIT))
#define UPLL_CON2               ((volatile uint32_t *)(0x42050108 + SEC_BIT))
#define UPLL_CON3               ((volatile uint32_t *)(0x4205010C + SEC_BIT))
#define UPLL_CON4               ((volatile uint32_t *)(0x42050110 + SEC_BIT))
#define UPLL_CON5               ((volatile uint32_t *)(0x42050114 + SEC_BIT))
#define UPLL_CON6               ((volatile uint32_t *)(0x42050118 + SEC_BIT))
#define UPLL_CON7               ((volatile uint32_t *)(0x4205011C + SEC_BIT))
#define UPLL_CON8               ((volatile uint32_t *)(0x42050120 + SEC_BIT))
#define UPLL_CON9               ((volatile uint32_t *)(0x42050124 + SEC_BIT))
#define UPLL_CON10              ((volatile uint32_t *)(0x42050128 + SEC_BIT))
#define LPOSC1_CON0             ((volatile uint32_t *)(0x42050200 + SEC_BIT))
#define LPOSC1_CON1             ((volatile uint32_t *)(0x42050204 + SEC_BIT))
#define LPOSC1_CON2             ((volatile uint32_t *)(0x42050208 + SEC_BIT))
#define LPOSC1_CON3             ((volatile uint32_t *)(0x4205020C + SEC_BIT))
#define LPOSC2_CON0             ((volatile uint32_t *)(0x42050250 + SEC_BIT))
#define LPOSC2_CON1             ((volatile uint32_t *)(0x42050254 + SEC_BIT))
#define LPOSC2_CON2             ((volatile uint32_t *)(0x42050258 + SEC_BIT))
#define LPOSC2_CON3             ((volatile uint32_t *)(0x4205025C + SEC_BIT))
#define SSC1_CON0               ((volatile uint32_t *)(0x42050300 + SEC_BIT))
#define SSC1_CON1               ((volatile uint32_t *)(0x42050304 + SEC_BIT))
#define SSC1_CON2               ((volatile uint32_t *)(0x42050308 + SEC_BIT))
#define SSC1_CON3               ((volatile uint32_t *)(0x4205030C + SEC_BIT))
#define SSC1_CON4               ((volatile uint32_t *)(0x42050310 + SEC_BIT))
#define SSC2_CON0               ((volatile uint32_t *)(0x42050350 + SEC_BIT))
#define SSC2_CON1               ((volatile uint32_t *)(0x42050354 + SEC_BIT))
#define SSC2_CON2               ((volatile uint32_t *)(0x42050358 + SEC_BIT))
#define SSC2_CON3               ((volatile uint32_t *)(0x4205035C + SEC_BIT))
#define SSC2_CON4               ((volatile uint32_t *)(0x42050360 + SEC_BIT))
#define SYS_ABIST_MON_CON0      ((volatile uint32_t *)(0x42050500 + SEC_BIT))
#define SYS_ABIST_MON_CON1      ((volatile uint32_t *)(0x42050504 + SEC_BIT))
#define SYS_ABIST_MON_CON2      ((volatile uint32_t *)(0x42050508 + SEC_BIT))
#define ABIST_MON_DATA0         ((volatile uint32_t *)(0x4205050C + SEC_BIT))
#define RSV_CON0                ((volatile uint32_t *)(0x42050510 + SEC_BIT))

/* Module Name: xpll_ctrl, Base Address: 0x42060000 */
#define XPLL_DBG_PROB           ((volatile uint32_t *)(0x42060400 + SEC_BIT))
#define XPLL_DBG_PROB_MON       ((volatile uint32_t *)(0x42060404 + SEC_BIT))
#define APLL2_CTL0              ((volatile uint32_t *)(0x42060100 + SEC_BIT))
#define APLL2_CTL1              ((volatile uint32_t *)(0x42060104 + SEC_BIT))
#define APLL2_CTL2              ((volatile uint32_t *)(0x42060108 + SEC_BIT))
#define APLL2_CTL3              ((volatile uint32_t *)(0x4206010C + SEC_BIT))
#define APLL2_CTL4              ((volatile uint32_t *)(0x42060110 + SEC_BIT))
#define APLL2_CTL5              ((volatile uint32_t *)(0x42060114 + SEC_BIT))
#define APLL2_CTL6              ((volatile uint32_t *)(0x42060118 + SEC_BIT))
#define APLL2_CTL7              ((volatile uint32_t *)(0x4206011C + SEC_BIT))
#define APLL2_CTL8              ((volatile uint32_t *)(0x42060120 + SEC_BIT))
#define APLL2_CTL9              ((volatile uint32_t *)(0x42060124 + SEC_BIT))
#define APLL2_CTL10             ((volatile uint32_t *)(0x42060128 + SEC_BIT))
#define APLL2_CTL11             ((volatile uint32_t *)(0x4206012C + SEC_BIT))
#define APLL2_CTL12             ((volatile uint32_t *)(0x42060130 + SEC_BIT))
#define APLL2_CTL13             ((volatile uint32_t *)(0x42060134 + SEC_BIT))
#define APLL2_CTL14             ((volatile uint32_t *)(0x42060138 + SEC_BIT))
#define APLL2_CTL15             ((volatile uint32_t *)(0x4206013C + SEC_BIT))
#define APLL2_CTL16             ((volatile uint32_t *)(0x42060140 + SEC_BIT))
#define APLL1_CTL0              ((volatile uint32_t *)(0x42060000 + SEC_BIT))
#define APLL1_CTL1              ((volatile uint32_t *)(0x42060004 + SEC_BIT))
#define APLL1_CTL2              ((volatile uint32_t *)(0x42060008 + SEC_BIT))
#define APLL1_CTL3              ((volatile uint32_t *)(0x4206000C + SEC_BIT))
#define APLL1_CTL4              ((volatile uint32_t *)(0x42060010 + SEC_BIT))
#define APLL1_CTL5              ((volatile uint32_t *)(0x42060014 + SEC_BIT))
#define APLL1_CTL6              ((volatile uint32_t *)(0x42060018 + SEC_BIT))
#define APLL1_CTL7              ((volatile uint32_t *)(0x4206001C + SEC_BIT))
#define APLL1_CTL8              ((volatile uint32_t *)(0x42060020 + SEC_BIT))
#define APLL1_CTL9              ((volatile uint32_t *)(0x42060024 + SEC_BIT))
#define APLL1_CTL10             ((volatile uint32_t *)(0x42060028 + SEC_BIT))
#define APLL1_CTL11             ((volatile uint32_t *)(0x4206002C + SEC_BIT))
#define APLL1_CTL12             ((volatile uint32_t *)(0x42060030 + SEC_BIT))
#define APLL1_CTL13             ((volatile uint32_t *)(0x42060034 + SEC_BIT))
#define APLL1_CTL14             ((volatile uint32_t *)(0x42060038 + SEC_BIT))
#define APLL1_CTL15             ((volatile uint32_t *)(0x4206003C + SEC_BIT))
#define APLL1_CTL16             ((volatile uint32_t *)(0x42060040 + SEC_BIT))

/* Module Name: dcxo_pwr_ctrl, Base Address: 0x4207_0000 */
#define DCXO_PCON0      ((volatile uint32_t *)(0x42070000 + SEC_BIT))
#define DCXO_PCON1      ((volatile uint32_t *)(0x42070004 + SEC_BIT))
#define DCXO_PCON2      ((volatile uint32_t *)(0x42070008 + SEC_BIT))
#define DCXO_PCON3      ((volatile uint32_t *)(0x4207000C + SEC_BIT))
#define DCXO_PCON4      ((volatile uint32_t *)(0x42070010 + SEC_BIT))
#define DCXO_PCON5      ((volatile uint32_t *)(0x42070014 + SEC_BIT))
#define DCXO_PCON6      ((volatile uint32_t *)(0x42070018 + SEC_BIT))
#define DCXO_PCON7      ((volatile uint32_t *)(0x4207001C + SEC_BIT))
#define DCXO_RSV        ((volatile uint32_t *)(0x42070020 + SEC_BIT))
#define DCXO_DEBUG0     ((volatile uint32_t *)(0x42070100 + SEC_BIT))
#define DCXO_DEBUG1     ((volatile uint32_t *)(0x42070104 + SEC_BIT))

/* Module Name: cksys_bus_clk, Base Address: 0x422B0000 */
#define BUS_DCM_CON_0       ((volatile uint32_t *)(0x422B0100 + SEC_BIT))
#define BUS_DCM_CON_1       ((volatile uint32_t *)(0x422B0104 + SEC_BIT))
#define CMSYS_DCM_CON_0     ((volatile uint32_t *)(0x422B0110 + SEC_BIT))
#define CMSYS_DCM_CON_1     ((volatile uint32_t *)(0x422B0114 + SEC_BIT))
#define DSP_DCM_CON_0       ((volatile uint32_t *)(0x422B0120 + SEC_BIT))
#define DSP_DCM_CON_1       ((volatile uint32_t *)(0x422B0124 + SEC_BIT))
#define AUD_DCM_CON_0       ((volatile uint32_t *)(0x422B0130 + SEC_BIT))
#define AUD_DCM_CON_1       ((volatile uint32_t *)(0x422B0134 + SEC_BIT))
#define CLOCK_SEL0          ((volatile uint32_t *)(0x422B0200 + SEC_BIT))
#define CLOCK_SEL1          ((volatile uint32_t *)(0x422B0204 + SEC_BIT))
#define PDN_PD_COND0        ((volatile uint32_t *)(0x422B0300 + SEC_BIT))
#define PDN_PD_SETD0        ((volatile uint32_t *)(0x422B0310 + SEC_BIT))
#define PDN_PD_CLRD0        ((volatile uint32_t *)(0x422B0320 + SEC_BIT))
#define PDN_AO_COND0        ((volatile uint32_t *)(0x422B0330 + SEC_BIT))
#define PDN_AO_SETD0        ((volatile uint32_t *)(0x422B0340 + SEC_BIT))
#define PDN_AO_CLRD0        ((volatile uint32_t *)(0x422B0350 + SEC_BIT))
#define PDN_TOP_COND0       ((volatile uint32_t *)(0x422B0360 + SEC_BIT))
#define PDN_TOP_SETD0       ((volatile uint32_t *)(0x422B0370 + SEC_BIT))
#define PDN_TOP_CLRD0       ((volatile uint32_t *)(0x422B0380 + SEC_BIT))


/* Module Name: top_misc_cfg, Base Address: 0x42010000 */
#define TOP_DEBUG_MON       ((volatile uint32_t *)(0x42010004 + SEC_BIT))
#define TOP_DEBUG_CTRL      ((volatile uint32_t *)(0x42010008 + SEC_BIT))


/*************************************************************************
 * Define clock mux register and bits structure
 *************************************************************************/


#endif /* HAL_CLOCK_MODULE_ENABLED */
#endif /* __HAL_CLOCK_PLATFORM_AB155X_H__*/

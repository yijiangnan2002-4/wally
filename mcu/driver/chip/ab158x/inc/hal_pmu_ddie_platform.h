/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
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

#ifndef __HAL_PMU_DDIE_PLATFORM_H__
#define __HAL_PMU_DDIE_PLATFORM_H__
#ifdef HAL_PMU_MODULE_ENABLED

#define PMU2_BC12_BASE                       (0x42090000)
#define PMU_DIG_BASE                         (0x42080000)
#define PMU_EFUSE_BASE                       (0x420C0000)
/*==========[D-die PMU]==========*/
#define PMU_DIG_RSV0                         (PMU_DIG_BASE+0x0000)
#define PMU_DIG_RSV1                         (PMU_DIG_BASE+0x0004)
#define PMU_DIG_STRUP_LATCH_DATA             (PMU_DIG_BASE+0x0008)
#define PMU_DIG_STRUP_LATCH_SET              (PMU_DIG_BASE+0x000c)
#define PMU_DIG_STRUP_LAT_READ_DATA          (PMU_DIG_BASE+0x0010)
#define PMU_DIG_STRUP_RG_CON0                (PMU_DIG_BASE+0x0014)
#define PMU_DIG_STRUP_RG_CON1                (PMU_DIG_BASE+0x0018)
#define PMU_DIG_STRUP_MON0                   (PMU_DIG_BASE+0x001c)
#define PMU_DIG_STRUP_MON1                   (PMU_DIG_BASE+0x0020)
#define PMU_DIG_STRUP_MON2                   (PMU_DIG_BASE+0x0024)
#define PMU_DIG_CTRL                         (PMU_DIG_BASE+0x0028)
#define PMU_DIG_GO_RTC                       (PMU_DIG_BASE+0x002c)
#define PMU_DIG_THR                          (PMU_DIG_BASE+0x0030)
#define PMU_DIG_HPBG0                        (PMU_DIG_BASE+0x0034)
#define PMU_DIG_HPBG1                        (PMU_DIG_BASE+0x0038)
#define PMU_DIG_HPBG2                        (PMU_DIG_BASE+0x003c)
#define PMU_DIG_HPBG3                        (PMU_DIG_BASE+0x0040)
#define PMU_DIG_HPBG4                        (PMU_DIG_BASE+0x0044)
#define PMU_DIG_BGRBUF0                      (PMU_DIG_BASE+0x0048)
#define PMU_DIG_BGRBUF1                      (PMU_DIG_BASE+0x004c)
#define PMU_DIG_LPBG0                        (PMU_DIG_BASE+0x0050)
#define PMU_DIG_LPBG1                        (PMU_DIG_BASE+0x0054)
#define PMU_DIG_LPBG2                        (PMU_DIG_BASE+0x0058)
#define PMU_DIG_LPBG3                        (PMU_DIG_BASE+0x005c)
#define PMU_DIG_LPBG4                        (PMU_DIG_BASE+0x0060)
#define PMU_DIG_LPBG5                        (PMU_DIG_BASE+0x0064)
#define PMU_DIG_VIOSW                        (PMU_DIG_BASE+0x0068)
#define PMU_DIG_STUPOSC                      (PMU_DIG_BASE+0x006c)
#define PMU_DIG_UVLO_VRTC                    (PMU_DIG_BASE+0x0070)
#define PMU_DIG_UVLO_18IN                    (PMU_DIG_BASE+0x0074)
#define PMU_DIG_USB0                         (PMU_DIG_BASE+0x0078)
#define PMU_DIG_USB1                         (PMU_DIG_BASE+0x007c)
#define PMU_DIG_USB2                         (PMU_DIG_BASE+0x0080)
#define PMU_DIG_USB3                         (PMU_DIG_BASE+0x0084)
#define PMU_DIG_VRF0                         (PMU_DIG_BASE+0x0088)
#define PMU_DIG_VRF1                         (PMU_DIG_BASE+0x008c)
#define PMU_DIG_VRF2                         (PMU_DIG_BASE+0x0090)
#define PMU_DIG_VRF3                         (PMU_DIG_BASE+0x0094)
#define PMU_DIG_VRF4                         (PMU_DIG_BASE+0x0098)
#define PMU_DIG_VRF5                         (PMU_DIG_BASE+0x009c)
#define PMU_DIG_VSRAM0                       (PMU_DIG_BASE+0x00a0)
#define PMU_DIG_VSRAM1                       (PMU_DIG_BASE+0x00a4)
#define PMU_DIG_MON                          (PMU_DIG_BASE+0x0144)
#define PMU_DIG_VSRAM2                       (PMU_DIG_BASE+0x00a8)
#define PMU_DIG_VSRAM3                       (PMU_DIG_BASE+0x00ac)
#define PMU_DIG_VSRAM4                       (PMU_DIG_BASE+0x00b0)
#define PMU_DIG_VSRAM5                       (PMU_DIG_BASE+0x00b4)
#define PMU_DIG_VRTC0                        (PMU_DIG_BASE+0x00b8)
#define PMU_DIG_VRTC1                        (PMU_DIG_BASE+0x00bc)
#define PMU_DIG_VRTC2                        (PMU_DIG_BASE+0x00c0)
#define PMU_DIG_VRTC3                        (PMU_DIG_BASE+0x00c4)
#define PMU_DIG_VRTC4                        (PMU_DIG_BASE+0x00c8)
#define PMU_DIG_GO_SLP_SEL                   (PMU_DIG_BASE+0x00d4)
#define PMU_DIG_GO_WDT_B_SEL                 (PMU_DIG_BASE+0x00d8)
#define PMU_DIG_DEG_CFG_VCORE0               (PMU_DIG_BASE+0x00dc)
#define PMU_DIG_DEG_CFG_VCORE1               (PMU_DIG_BASE+0x00e0)
#define PMU_DIG_DEG_CFG_VSRAM0               (PMU_DIG_BASE+0x00e4)
#define PMU_DIG_DEG_CFG_VSRAM1               (PMU_DIG_BASE+0x00e8)
#define PMU_DIG_DEG_CFG_VSRAM2               (PMU_DIG_BASE+0x00ec)
#define PMU_DIG_DEG_CFG_VRF0                 (PMU_DIG_BASE+0x00f0)
#define PMU_DIG_DEG_CFG_VRF1                 (PMU_DIG_BASE+0x00f4)
#define PMU_DIG_DEG_CFG_VRF2                 (PMU_DIG_BASE+0x00f8)
#define PMU_DIG_DEG_CFG_VUSB1                (PMU_DIG_BASE+0x00fc)
#define PMU_DIG_DEG_CFG_VUSB2                (PMU_DIG_BASE+0x0100)
#define PMU_DIG_INT_MISC                     (PMU_DIG_BASE+0x0104)
#define PMU_DIG_INT_STATUS_CLR               (PMU_DIG_BASE+0x0108)
#define PMU_DIG_INT_EN_CON                   (PMU_DIG_BASE+0x010c)
#define PMU_DIG_INT_EN_MASK                  (PMU_DIG_BASE+0x0110)
#define PMU_DIG_INT_RAW_STATUS               (PMU_DIG_BASE+0x0114)
#define PMU_DIG_INT_STATUS                   (PMU_DIG_BASE+0x0118)
#define PMU_DIG_DEBUG_MON                    (PMU_DIG_BASE+0x011c)
#define PMU_DIG_ATSTSEL0                     (PMU_DIG_BASE+0x0120)
#define PMU_DIG_ATSTSEL1                     (PMU_DIG_BASE+0x0124)
#define PMU_DIG_ATSTSEL2                     (PMU_DIG_BASE+0x0128)
#define PMU_DIG_SLP0                         (PMU_DIG_BASE+0x0130)
#define PMU_DIG_SLP1                         (PMU_DIG_BASE+0x0134)
#define PMU_DIG_SWCTL0                       (PMU_DIG_BASE+0x0138)
#define PMU_DIG_SWCTL1                       (PMU_DIG_BASE+0x013c)
#define PMU_DIG_TEST_CON                     (PMU_DIG_BASE+0x0140)
#define PMU_DIG_ATPG                         (PMU_DIG_BASE+0x0148)

#define PMU_STRUP_RG_CON0_MASK                0xFF
#define PMU_STRUP_RG_CON0_SHIFT               0
#define PMU_BOND_USE_VRF_MASK                 0x1
#define PMU_BOND_USE_VRF_SHIFT                5
#define PMU_BOND_AD_MASK                      0x1
#define PMU_BOND_AD_SHIFT                     4
#define PMU_STRUP_RG_CON1_MASK                0xFF
#define PMU_STRUP_RG_CON1_SHIFT               0
#define PMU_WDTRSTB_COLD_EN_MASK              0x1
#define PMU_WDTRSTB_COLD_EN_SHIFT             8
#define PMU_BYPASS_MODE_EN_MASK               0x1
#define PMU_BYPASS_MODE_EN_SHIFT              5
#define PMU_BGR_BUF_FORCEEN_MASK              0x1
#define PMU_BGR_BUF_FORCEEN_SHIFT             1
#define PMU_VRF_NDIS_EN_MASK                  0x1
#define PMU_VRF_NDIS_EN_SHIFT                 0
#define PMU_BGR_RSV_MASK                      0xFF
#define PMU_BGR_RSV_SHIFT                     0
#define PMU_VSRAM_VOSEL_NORMAL_MASK           0x7F
#define PMU_VSRAM_VOSEL_NORMAL_SHIFT          0
#define PMU_SW_GO_SLP_SEL_MASK                0x1
#define PMU_SW_GO_SLP_SEL_SHIFT               1
#define PMU_SW_RGU_WDT_B_SEL_MASK             0x1
#define PMU_SW_RGU_WDT_B_SEL_SHIFT            1
#define PMU_VSRAM_VOSEL_SLEEP_MASK            0x7F
#define PMU_VSRAM_VOSEL_SLEEP_SHIFT           8
#define PMU_GO_RTC_MASK                       0x1
#define PMU_GO_RTC_SHIFT                      0
#define PMU_SW_VRF_EN_MASK                    0x1
#define PMU_SW_VRF_EN_SHIFT                   1
#define PMU_VUSB_EN_MASK                      0x1
#define PMU_VUSB_EN_SHIFT                     8
#define PMU_USB18_SW_EN_MASK                  0x1
#define PMU_USB18_SW_EN_SHIFT                 4
#define PMU_USB_RSTB_MASK                     0x1
#define PMU_USB_RSTB_SHIFT                    0
#define PMU_VUSB_VOTRIM_MASK                  0x1F
#define PMU_VUSB_VOTRIM_SHIFT                 8
#define PMU_VUSB_VOSEL_MASK                   0x1F
#define PMU_VUSB_VOSEL_SHIFT                  0
#define PMU_VSRAM_VOSEL_NORMAL_MASK           0x7F
#define PMU_VSRAM_VOSEL_NORMAL_SHIFT          0
#define PMU_VSRAM_VOSEL_SLEEP_MASK            0x7F
#define PMU_VSRAM_VOSEL_SLEEP_SHIFT           8
#define PMU_RGS_STRUP_ONOFF_STATUS_MASK       0x7F
#define PMU_RGS_STRUP_ONOFF_STATUS_SHIFT      0
#define PMU_PMU_DIG_UVLO_VRTC_MASK            0x1F
#define PMU_PMU_DIG_UVLO_VRTC_SHIFT           0
#define PMU_PMU_DIG_UVLO_18IN_MASK            0x1F
#define PMU_PMU_DIG_UVLO_18IN_SHIFT           0
#define PMU_STRUP_RG_EN_MASK                  0x1
#define PMU_STRUP_RG_EN_SHIFT                 0
#define PMU_STRUP_LATCH_READ_DATA_MASK        0xFF
#define PMU_STRUP_LATCH_READ_DATA_SHIFT       0
#define PMU_VSRAM_VOSEL_SLEEP_MASK            0x7F
#define PMU_VSRAM_VOSEL_SLEEP_SHIFT           8
#define PMU_AD_VUSB_PG_STATUS_MASK            0x1
#define PMU_AD_VUSB_PG_STATUS_SHIFT           0
#define PMU_SW_RGU_WDT_B_SEL_MASK             0x1
#define PMU_SW_RGU_WDT_B_SEL_SHIFT            1
#define PMU_SW_RGU_WDT_B_MASK                 0x1
#define PMU_SW_RGU_WDT_B_SHIFT                0
#define PMU_VRF_VOSEL_MASK                    0x3f
#define PMU_VRF_VOSEL_SHIFT                   4
#define PMU_VRTC_VOSEL_MASK                   0x1f
#define PMU_VRTC_VOSEL_SHIFT                  0
#define PMU_AD_VUSB_PG_STATUS_MASK            0x1
#define PMU_AD_VUSB_PG_STATUS_SHIFT           0
#define PMU_VRTC_VOSEL_MASK                   0x1f
#define PMU_VRTC_VOSEL_SHIFT                  0
/*==========[BC1.2]==========*/
#define PMU2_ANA_CON0                        (PMU2_BC12_BASE+0x0500)
#define PMU2_ANA_CON1                        (PMU2_BC12_BASE+0x0504)
#define PMU2_ANA_RO                          (PMU2_BC12_BASE+0x0510)
#define PMU_BC12_IBIAS_EN_V12_ADDR                  PMU2_ANA_CON0
#define PMU_BC12_IBIAS_EN_V12_MASK                  0x1
#define PMU_BC12_IBIAS_EN_V12_SHIFT                 0
#define PMU_BC12_CMP_EN_V12_ADDR                    PMU2_ANA_CON0
#define PMU_BC12_CMP_EN_V12_MASK                    0x3
#define PMU_BC12_CMP_EN_V12_SHIFT                   1
#define PMU_BC12_DCD_EN_V12_ADDR                    PMU2_ANA_CON0
#define PMU_BC12_DCD_EN_V12_MASK                    0x1
#define PMU_BC12_DCD_EN_V12_SHIFT                   3
#define PMU_BC12_IPDC_EN_V12_ADDR                   PMU2_ANA_CON0
#define PMU_BC12_IPDC_EN_V12_MASK                   0x3
#define PMU_BC12_IPDC_EN_V12_SHIFT                  4
#define PMU_BC12_IPD_EN_V12_ADDR                    PMU2_ANA_CON0
#define PMU_BC12_IPD_EN_V12_MASK                    0x3
#define PMU_BC12_IPD_EN_V12_SHIFT                   6
#define PMU_BC12_IPD_HALF_EN_V12_ADDR               PMU2_ANA_CON0
#define PMU_BC12_IPD_HALF_EN_V12_MASK               0x1
#define PMU_BC12_IPD_HALF_EN_V12_SHIFT              8
#define PMU_BC12_IPU_EN_V12_ADDR                    PMU2_ANA_CON0
#define PMU_BC12_IPU_EN_V12_MASK                    0x3
#define PMU_BC12_IPU_EN_V12_SHIFT                   9
#define PMU_BC12_VREF_VTH_EN_V12_ADDR               PMU2_ANA_CON0
#define PMU_BC12_VREF_VTH_EN_V12_MASK               0x3
#define PMU_BC12_VREF_VTH_EN_V12_SHIFT              11
#define PMU_BC12_SRC_EN_V12_ADDR                    PMU2_ANA_CON0
#define PMU_BC12_SRC_EN_V12_MASK                    0x3
#define PMU_BC12_SRC_EN_V12_SHIFT                   13
#define PMU_BC12_IPU_TEST_EN_V12_ADDR               PMU2_ANA_CON1
#define PMU_BC12_IPU_TEST_EN_V12_MASK               0x1
#define PMU_BC12_IPU_TEST_EN_V12_SHIFT              0
#define PMU_BC12_CS_TRIM_V12_ADDR                   PMU2_ELR_0
#define PMU_BC12_CS_TRIM_V12_MASK                   0x3F
#define PMU_BC12_CS_TRIM_V12_SHIFT                  0
#define PMU_AQ_QI_BC12_CMP_OUT_V12_ADDR             PMU2_ANA_RO
#define PMU_AQ_QI_BC12_CMP_OUT_V12_MASK             0x1
#define PMU_AQ_QI_BC12_CMP_OUT_V12_SHIFT            0
/*==========[D-die PMU efuse]==========*/
#define PMU_M_ANA_CFG_DPMU0                  (PMU_EFUSE_BASE+0x0348)
#define PMU_M_ANA_CFG_DPMU1                  (PMU_EFUSE_BASE+0x034C)
#define PMU_M_ANA_CFG_DPMU2                  (PMU_EFUSE_BASE+0x0350)
#define PMU_M_ANA_CFG_DPMU3                  (PMU_EFUSE_BASE+0x0354)
#define PMU_LPBG_VREF0P45TRIM_TARGET_MASK                   0x1F
#define PMU_LPBG_VREF0P45TRIM_TARGET_SHIFT                  0
#define PMU_LPBG_VREF0P45TRIM_MASK                          0x1F
#define PMU_LPBG_VREF0P45TRIM_SHIFT                         8
#define PMU_LPBG_VREF0P6TRIM_TARGET_MASK                    0x1F
#define PMU_LPBG_VREF0P6TRIM_TARGET_SHIFT                   5
#define PMU_LPBG_VREF0P6TRIM_MASK                           0x1F
#define PMU_LPBG_VREF0P6TRIM_SHIFT                          0
#define PMU_LPBG_VREF0P85TRIM_TARGET_MASK                   0x1F
#define PMU_LPBG_VREF0P85TRIM_TARGET_SHIFT                  10
#define PMU_LPBG_VREF0P85TRIM_MASK                          0x1F
#define PMU_LPBG_VREF0P85TRIM_SHIFT                         0
#define PMU_LPBG_IBTRIM_TARGET_MASK                         0x1F
#define PMU_LPBG_IBTRIM_TARGET_SHIFT                        15
#define PMU_LPBG_IBTRIM_MASK                                0x1F
#define PMU_LPBG_IBTRIM_SHIFT                               0
#define PMU_BROM_DPMU_TRIM_EN_MASK                          0x1
#define PMU_BROM_DPMU_TRIM_EN_SHIFT                         31
#define PMU_VREF0P6_TRIM_HPBG_TARGET_MASK                   0x1F
#define PMU_VREF0P6_TRIM_HPBG_TARGET_SHIFT                  0
#define PMU_VREF0P6_TRIM_HPBG_MASK                          0x1F
#define PMU_VREF0P6_TRIM_HPBG_SHIFT                         0
#define PMU_VREF0P45_TRIM_HPBG_TARGET_MASK                  0xF
#define PMU_VREF0P45_TRIM_HPBG_TARGET_SHIFT                 5
#define PMU_VREF0P45_TRIM_HPBG_MASK                         0xF
#define PMU_VREF0P45_TRIM_HPBG_SHIFT                        8
#define PMU_VREF0P85_TRIM_HPBG_TARGET_MASK                  0xF
#define PMU_VREF0P85_TRIM_HPBG_TARGET_SHIFT                 9
#define PMU_VREF0P85_TRIM_HPBG_MASK                         0xF
#define PMU_VREF0P85_TRIM_HPBG_SHIFT                        4
#define PMU_VREF1P205_TRIM_HPBG_TARGET_MASK                 0xF
#define PMU_VREF1P205_TRIM_HPBG_TARGET_SHIFT                13
#define PMU_VREF1P205_TRIM_HPBG_MASK                        0xF
#define PMU_VREF1P205_TRIM_HPBG_SHIFT                       0
#define PMU_IBIAS_TRIM_HPBG_TARGET_MASK                     0xF
#define PMU_IBIAS_TRIM_HPBG_TARGET_SHIFT                    17
#define PMU_IBIAS_TRIM_HPBG_MASK                            0xF
#define PMU_IBIAS_TRIM_HPBG_SHIFT                           4
#define PMU_BGR_TCTRIM_HPBG_TARGET_MASK                     0xF
#define PMU_BGR_TCTRIM_HPBG_TARGET_SHIFT                    21
#define PMU_BGR_TCTRIM_HPBG_MASK                            0xF
#define PMU_BGR_TCTRIM_HPBG_SHIFT                           0
#define PMU_VRF_VOTRIM_TARGET_MASK                          0x1F
#define PMU_VRF_VOTRIM_TARGET_SHIFT                         10
#define PMU_VRF_VOTRIM_MASK                                 0x1F
#define PMU_VRF_VOTRIM_SHIFT                                8
#define PMU_VSRAM_VOTRIM_TARGET_MASK                        0x1F
#define PMU_VSRAM_VOTRIM_TARGET_SHIFT                       15
#define PMU_VSRAM_VOTRIM_MASK                               0x1F
#define PMU_VSRAM_VOTRIM_SHIFT                              8
#define PMU_VUSB_VOTRIM_TARGET_MASK                         0x1F
#define PMU_VUSB_VOTRIM_TARGET_SHIFT                        25
#define PMU_VUSB_VOTRIM_MASK                                0x1F
#define PMU_VUSB_VOTRIM_SHIFT                               8
#define PMU_VRTC_VOTRIM_TARGET_MASK                         0x1F
#define PMU_VRTC_VOTRIM_TARGET_SHIFT                        0
#define PMU_VRTC_VOTRIM_MASK                                0x1F
#define PMU_VRTC_VOTRIM_SHIFT                               0
//PMU_DIG_LPBG5
#define PMU_SET_LPBG_VREFTRIM_MASK                          0x1
#define PMU_SET_LPBG_VREFTRIM_SHIFT                         15
//PMU_DIG_HPBG4
#define PMU_SET_LPBG_IBTRIM_MASK                            0x1
#define PMU_SET_LPBG_IBTRIM_SHIFT                           0
//PMU_DIG_LPBG1
#define PMU_SET_LPBG_TCTRIM_MASK                            0x1
#define PMU_SET_LPBG_TCTRIM_SHIFT                           15
//PMU_DIG_HPBG4
#define PMU_SET_VREF_TRIM_HPBG_MASK                         0x1
#define PMU_SET_VREF_TRIM_HPBG_SHIFT                        0
//PMU_DIG_HPBG1
#define PMU_SET_TCTRIM_HPBG_MASK                            0x1
#define PMU_SET_TCTRIM_HPBG_SHIFT                           4
#endif //end of HAL_PMU_MODULE_ENABLED
#endif //end of __HAL_PMU_DDIE_PLATFORM_H__

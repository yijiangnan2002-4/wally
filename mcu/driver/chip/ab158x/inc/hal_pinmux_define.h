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

#ifndef __HAL_PINMUX_DEFINE_H__




#define __HAL_PINMUX_DEFINE_H__

#include "hal_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAL_GPIO_MODULE_ENABLED


#define HAL_GPIO_0_GPIO0    0
#define HAL_GPIO_0_AP_JTMS    1
#define HAL_GPIO_0_I2C0_SCL    2
#define HAL_GPIO_0_I2C0_SCL_EX    5
#define HAL_GPIO_0_BT_PLAYEN_INT    6
#define HAL_GPIO_0_BT_EPA_ELNA_CSD    7
#define HAL_GPIO_0_CTP1    8
#define HAL_GPIO_0_EINT0    9
#define HAL_GPIO_0_DEBUGMON0    10

#define HAL_GPIO_1_GPIO1    0
#define HAL_GPIO_1_AP_JTCK    1
#define HAL_GPIO_1_I2C0_SDA    2
#define HAL_GPIO_1_I2C0_SDA_EX    5
#define HAL_GPIO_1_AUDIO_EXT_SYNC_EN    6
#define HAL_GPIO_1_BT_EPA_ELNA_CPS    7
#define HAL_GPIO_1_CTP2    8
#define HAL_GPIO_1_EINT1    9
#define HAL_GPIO_1_DEBUGMON1    10

#define HAL_GPIO_2_GPIO2    0
#define HAL_GPIO_2_DSP_JTMS    1
#define HAL_GPIO_2_I2S_MST0_RX    3
#define HAL_GPIO_2_I2S_SLV2_RX    4
#define HAL_GPIO_2_BT_EPA_ELNA_CRX    7
#define HAL_GPIO_2_CTP3    8
#define HAL_GPIO_2_EINT2    9
#define HAL_GPIO_2_DEBUGMON2    10

#define HAL_GPIO_3_GPIO3    0
#define HAL_GPIO_3_DSP_JTCK    1
#define HAL_GPIO_3_I2S_MST0_CK    3
#define HAL_GPIO_3_I2S_SLV2_CK    4
#define HAL_GPIO_3_JTCK_EX    5
#define HAL_GPIO_3_BT_EPA_ELNA_CTX    7
#define HAL_GPIO_3_AUXADC6    8
#define HAL_GPIO_3_EINT3    9
#define HAL_GPIO_3_DEBUGMON3    10

#define HAL_GPIO_4_GPIO4    0
#define HAL_GPIO_4_DSP_JRST    1
#define HAL_GPIO_4_I2S_MST0_WS    3
#define HAL_GPIO_4_I2S_SLV2_WS    4
#define HAL_GPIO_4_JTAG_SEL_0_EX    5
#define HAL_GPIO_4_BT_EPA_ELNA_CHL    7
#define HAL_GPIO_4_AUXADC5    8
#define HAL_GPIO_4_EINT4    9
#define HAL_GPIO_4_DEBUGMON4    10

#define HAL_GPIO_5_GPIO5    0
#define HAL_GPIO_5_DSP_JTDI    1
#define HAL_GPIO_5_DMIC0_CLK    2
#define HAL_GPIO_5_I2S_MST0_MCLK    3
#define HAL_GPIO_5_JTAG_SEL_1_EX    5
#define HAL_GPIO_5_BT_EPA_ELNA_ANT_SEL     7
#define HAL_GPIO_5_AUXADC4    8
#define HAL_GPIO_5_EINT5    9
#define HAL_GPIO_5_DEBUGMON5    10

#define HAL_GPIO_6_GPIO6    0
#define HAL_GPIO_6_DSP_JTDO    1
#define HAL_GPIO_6_DMIC0_DAT    2
#define HAL_GPIO_6_I2S_MST0_TX    3
#define HAL_GPIO_6_I2S_SLV2_TX    4
#define HAL_GPIO_6_JTDO_EX    5
#define HAL_GPIO_6_COEX_W1_DAT    7
#define HAL_GPIO_6_AUXADC3    8
#define HAL_GPIO_6_EINT6    9
#define HAL_GPIO_6_DEBUGMON6    10

#define HAL_GPIO_7_GPIO7    0
#define HAL_GPIO_7_UART2_TXD    1
#define HAL_GPIO_7_I2C1_SCL    2
#define HAL_GPIO_7_UART0_TXD_EX    5
#define HAL_GPIO_7_COEX_W3_BT_WLAN    7
#define HAL_GPIO_7_AUXADC2    8
#define HAL_GPIO_7_EINT7    9
#define HAL_GPIO_7_DEBUGMON7    10

#define HAL_GPIO_8_GPIO8    0
#define HAL_GPIO_8_UART2_RXD    1
#define HAL_GPIO_8_I2C1_SDA    2
#define HAL_GPIO_8_UART0_RXD_EX    5
#define HAL_GPIO_8_COEX_W3_BT_ACT    7
#define HAL_GPIO_8_AUXADC1    8
#define HAL_GPIO_8_EINT8    9
#define HAL_GPIO_8_DEBUGMON8    10

#define HAL_GPIO_9_GPIO9    0
#define HAL_GPIO_9_UART1_TXD    1
#define HAL_GPIO_9_UART1_TXD_EX    5
#define HAL_GPIO_9_PWM13    7
#define HAL_GPIO_9_AUXADC0    8
#define HAL_GPIO_9_EINT9    9
#define HAL_GPIO_9_DEBUGMON9    10

#define HAL_GPIO_10_GPIO10    0
#define HAL_GPIO_10_UART1_RXD    1
#define HAL_GPIO_10_UART1_TXD    2
#define HAL_GPIO_10_UART1_RXD_EX    5
#define HAL_GPIO_10_PWM0    7
#define HAL_GPIO_10_RF_TEST_P    8
#define HAL_GPIO_10_EINT10    9
#define HAL_GPIO_10_DEBUGMON10    10

#define HAL_GPIO_11_GPIO11    0
#define HAL_GPIO_11_UART0_TXD    1
#define HAL_GPIO_11_UART0_TXD_EX    5
#define HAL_GPIO_11_PWM12    7
#define HAL_GPIO_11_RF_TEST_N    8
#define HAL_GPIO_11_EINT11    9
#define HAL_GPIO_11_DEBUGMON11    10

#define HAL_GPIO_12_GPIO12    0
#define HAL_GPIO_12_UART0_RXD    1
#define HAL_GPIO_12_UART0_TXD    2
#define HAL_GPIO_12_UART0_RXD_EX    5
#define HAL_GPIO_12_RF_TEST_BUS    8
#define HAL_GPIO_12_EINT12    9
#define HAL_GPIO_12_DEBUGMON12    10

#define HAL_GPIO_13_GPIO13    0
#define HAL_GPIO_13_I2C0_SCL    1
#define HAL_GPIO_13_UART1_TXD    3
#define HAL_GPIO_13_UART1_CTS    4
#define HAL_GPIO_13_I2C0_SCL_EX    5
#define HAL_GPIO_13_PAD_DFUNC_SFCK    6
#define HAL_GPIO_13_IRRX    7
#define HAL_GPIO_13_PWM4    8
#define HAL_GPIO_13_EINT13    9
#define HAL_GPIO_13_DEBUGMON13    10

#define HAL_GPIO_14_GPIO14    0
#define HAL_GPIO_14_I2C0_SDA    1
#define HAL_GPIO_14_I2S_MST0_TX    2
#define HAL_GPIO_14_UART1_RXD    3
#define HAL_GPIO_14_UART1_RTS    4
#define HAL_GPIO_14_I2C0_SDA_EX    5
#define HAL_GPIO_14_PAD_DFUNC_SIN    6
#define HAL_GPIO_14_COEX_W3_BT_WLAN    7
#define HAL_GPIO_14_PWM5    8
#define HAL_GPIO_14_EINT14    9
#define HAL_GPIO_14_DEBUGMON14    10

#define HAL_GPIO_15_GPIO15    0
#define HAL_GPIO_15_I2C1_SCL    1
#define HAL_GPIO_15_I2S_MST0_CK    2
#define HAL_GPIO_15_I2S_MST1_MCLK    3
#define HAL_GPIO_15_CLKO3    4
#define HAL_GPIO_15_I2C0_SCL_EX    5
#define HAL_GPIO_15_PAD_DFUNC_SOUT    6
#define HAL_GPIO_15_I3C1_SCL    7
#define HAL_GPIO_15_PWM6    8
#define HAL_GPIO_15_EINT15    9
#define HAL_GPIO_15_DEBUGMON15    10

#define HAL_GPIO_16_GPIO16    0
#define HAL_GPIO_16_I2C1_SDA    1
#define HAL_GPIO_16_I2S_MST0_WS    2
#define HAL_GPIO_16_I2S_MST1_CK    3
#define HAL_GPIO_16_I2S_SLV1_CK    4
#define HAL_GPIO_16_I2C0_SDA_EX    5
#define HAL_GPIO_16_PAD_DFUNC_SFCS0    6
#define HAL_GPIO_16_I3C1_SDA    7
#define HAL_GPIO_16_EINT16    9
#define HAL_GPIO_16_DEBUGMON0    10

#define HAL_GPIO_17_GPIO17    0
#define HAL_GPIO_17_I2C2_SCL    1
#define HAL_GPIO_17_I2S_MST0_RX    2
#define HAL_GPIO_17_I2S_MST1_WS    3
#define HAL_GPIO_17_I2S_SLV1_WS    4
#define HAL_GPIO_17_I2C0_SCL_EX    5
#define HAL_GPIO_17_I3C1_SCL    6
#define HAL_GPIO_17_I3C0_SCL    7
#define HAL_GPIO_17_AP_JTMS    8
#define HAL_GPIO_17_EINT17    9
#define HAL_GPIO_17_DEBUGMON1    10

#define HAL_GPIO_18_GPIO18    0
#define HAL_GPIO_18_I2C2_SDA    1
#define HAL_GPIO_18_I2S_MST0_MCLK    2
#define HAL_GPIO_18_I2S_MST1_RX    3
#define HAL_GPIO_18_I2S_SLV1_RX    4
#define HAL_GPIO_18_I2C0_SDA_EX    5
#define HAL_GPIO_18_I3C1_SDA    6
#define HAL_GPIO_18_I3C0_SDA    7
#define HAL_GPIO_18_AP_JTCK    8
#define HAL_GPIO_18_EINT18    9
#define HAL_GPIO_18_DEBUGMON2    10

#define HAL_GPIO_19_GPIO19    0
#define HAL_GPIO_19_ESC_SCK    1
#define HAL_GPIO_19_SPI_MST0_SCK    2
#define HAL_GPIO_19_CLKO1    3
#define HAL_GPIO_19_SPI_MST0_SCK_EX    5
#define HAL_GPIO_19_MSDC0_CLK0    6
#define HAL_GPIO_19_UART0_TXD    7
#define HAL_GPIO_19_SPI_SLV0_SCK    8
#define HAL_GPIO_19_EINT19    9
#define HAL_GPIO_19_DEBUGMON3    10

#define HAL_GPIO_20_GPIO20    0
#define HAL_GPIO_20_ESC_MOSI    1
#define HAL_GPIO_20_SPI_MST0_MOSI    2
#define HAL_GPIO_20_SPI_MST0_MOSI_EX    5
#define HAL_GPIO_20_MSDC0_CMD    6
#define HAL_GPIO_20_UART0_RXD    7
#define HAL_GPIO_20_SPI_SLV0_MOSI    8
#define HAL_GPIO_20_EINT20    9
#define HAL_GPIO_20_DEBUGMON4    10

#define HAL_GPIO_21_GPIO21    0
#define HAL_GPIO_21_ESC_MISO    1
#define HAL_GPIO_21_SPI_MST0_MISO    2
#define HAL_GPIO_21_SPI_MST0_CS3    4
#define HAL_GPIO_21_SPI_MST0_MISO_EX    5
#define HAL_GPIO_21_MSDC0_DAT0    6
#define HAL_GPIO_21_UART0_CTS    7
#define HAL_GPIO_21_SPI_SLV0_MISO    8
#define HAL_GPIO_21_EINT21    9
#define HAL_GPIO_21_DEBUGMON5    10

#define HAL_GPIO_22_GPIO22    0
#define HAL_GPIO_22_ESC_CS    1
#define HAL_GPIO_22_SPI_MST0_CS0    2
#define HAL_GPIO_22_SPI_MST0_CS0_EX    5
#define HAL_GPIO_22_MSDC0_DAT1    6
#define HAL_GPIO_22_UART0_RTS    7
#define HAL_GPIO_22_SPI_SLV0_CS    8
#define HAL_GPIO_22_EINT22    9
#define HAL_GPIO_22_DEBUGMON6    10

#define HAL_GPIO_23_GPIO23    0
#define HAL_GPIO_23_ESC_SIO2    1
#define HAL_GPIO_23_SPI_MST0_SIO2    2
#define HAL_GPIO_23_SPI_MST0_CS1    4
#define HAL_GPIO_23_SPI_MST0_CS1_EX    5
#define HAL_GPIO_23_MSDC0_DAT2    6
#define HAL_GPIO_23_COEX_W3_BT_PRI    7
#define HAL_GPIO_23_SPI_SLV0_SIO2    8
#define HAL_GPIO_23_EINT23    9
#define HAL_GPIO_23_DEBUGMON7    10

#define HAL_GPIO_24_GPIO24    0
#define HAL_GPIO_24_ESC_SIO3    1
#define HAL_GPIO_24_SPI_MST0_SIO3    2
#define HAL_GPIO_24_I2S_MST0_MCLK    3
#define HAL_GPIO_24_SPI_MST0_CS2    4
#define HAL_GPIO_24_SPI_MST0_CS2_EX    5
#define HAL_GPIO_24_MSDC0_DAT3    6
#define HAL_GPIO_24_I2S_SLV3_WS    7
#define HAL_GPIO_24_SPI_SLV0_SIO3    8
#define HAL_GPIO_24_EINT24    9
#define HAL_GPIO_24_DEBUGMON8    10

#define HAL_GPIO_25_GPIO25    0
#define HAL_GPIO_25_I2S_MST0_TX    1
#define HAL_GPIO_25_I2S_SLV0_TX    2
#define HAL_GPIO_25_CLKO2    3
#define HAL_GPIO_25_MSDC0_RST    6
#define HAL_GPIO_25_SPDIF    7
#define HAL_GPIO_25_EINT25    9
#define HAL_GPIO_25_DEBUGMON9    10

#define HAL_GPIO_26_GPIO26    0
#define HAL_GPIO_26_I2S_MST1_WS    1
#define HAL_GPIO_26_I2S_SLV1_TX    2
#define HAL_GPIO_26_I2S_MST1_TX    3
#define HAL_GPIO_26_ROSC_CLKO    6
#define HAL_GPIO_26_EINT26    9
#define HAL_GPIO_26_DEBUGMON10    10

#define HAL_GPIO_27_GPIO27    0
#define HAL_GPIO_27_I2S_MST0_RX    1
#define HAL_GPIO_27_I2S_SLV0_RX    2
#define HAL_GPIO_27_SPI_MST1_SCK    4
#define HAL_GPIO_27_PWM1    7
#define HAL_GPIO_27_BT_EPA_ELNA_CSD    8
#define HAL_GPIO_27_EINT27    9
#define HAL_GPIO_27_DEBUGMON11    10

#define HAL_GPIO_28_GPIO28    0
#define HAL_GPIO_28_I2S_MST1_RX    1
#define HAL_GPIO_28_I2S_SLV3_RX    2
#define HAL_GPIO_28_SPI_MST1_MOSI    4
#define HAL_GPIO_28_PWM2    7
#define HAL_GPIO_28_BT_EPA_ELNA_CPS    8
#define HAL_GPIO_28_EINT28    9
#define HAL_GPIO_28_DEBUGMON12    10

#define HAL_GPIO_29_GPIO29    0
#define HAL_GPIO_29_I2S_MST2_RX    1
#define HAL_GPIO_29_I2S_SLV4_RX    2
#define HAL_GPIO_29_SPDIF_RX1    3
#define HAL_GPIO_29_SPI_MST1_MISO    4
#define HAL_GPIO_29_SPI_MST1_CS3    6
#define HAL_GPIO_29_PWM3    7
#define HAL_GPIO_29_BT_EPA_ELNA_CRX    8
#define HAL_GPIO_29_EINT29    9
#define HAL_GPIO_29_DEBUGMON13    10

#define HAL_GPIO_30_GPIO30    0
#define HAL_GPIO_30_I2S_MST0_CK    1
#define HAL_GPIO_30_I2S_SLV0_CK    2
#define HAL_GPIO_30_SPI_MST1_CS0    4
#define HAL_GPIO_30_PWM4    7
#define HAL_GPIO_30_BT_EPA_ELNA_CTX    8
#define HAL_GPIO_30_EINT30    9
#define HAL_GPIO_30_DEBUGMON14    10

#define HAL_GPIO_31_GPIO31    0
#define HAL_GPIO_31_I2S_MST2_MCLK    1
#define HAL_GPIO_31_I2S_SLV1_CK    2
#define HAL_GPIO_31_SPI_MST1_SIO2    4
#define HAL_GPIO_31_SPI_MST1_CS1    6
#define HAL_GPIO_31_PWM5    7
#define HAL_GPIO_31_BT_EPA_ELNA_CHL    8
#define HAL_GPIO_31_EINT31    9
#define HAL_GPIO_31_DEBUGMON15    10

#define HAL_GPIO_32_GPIO32    0
#define HAL_GPIO_32_I2S_MST2_CK    1
#define HAL_GPIO_32_I2S_SLV1_WS    2
#define HAL_GPIO_32_SPI_MST1_SIO3    4
#define HAL_GPIO_32_SPI_MST1_CS2    6
#define HAL_GPIO_32_PWM6    7
#define HAL_GPIO_32_BT_EPA_ELNA_ANT_SEL     8
#define HAL_GPIO_32_EINT32    9
#define HAL_GPIO_32_DEBUGMON0    10

#define HAL_GPIO_33_GPIO33    0
#define HAL_GPIO_33_I2S_MST2_WS    1
#define HAL_GPIO_33_I2S_SLV1_RX    2
#define HAL_GPIO_33_SPI_MST2_SCK    4
#define HAL_GPIO_33_PWM7    7
#define HAL_GPIO_33_EINT33    9
#define HAL_GPIO_33_DEBUGMON1    10

#define HAL_GPIO_34_GPIO34    0
#define HAL_GPIO_34_I2S_MST1_CK    1
#define HAL_GPIO_34_I2S_SLV2_RX    2
#define HAL_GPIO_34_DSP_JTMS    3
#define HAL_GPIO_34_SPI_MST2_MOSI    4
#define HAL_GPIO_34_PWM8    7
#define HAL_GPIO_34_EINT34    9
#define HAL_GPIO_34_DEBUGMON2    10

#define HAL_GPIO_35_GPIO35    0
#define HAL_GPIO_35_I2S_MST0_WS    1
#define HAL_GPIO_35_I2S_SLV0_WS    2
#define HAL_GPIO_35_DSP_JTDO    3
#define HAL_GPIO_35_SPI_MST2_MISO    4
#define HAL_GPIO_35_SPI_MST2_CS3    6
#define HAL_GPIO_35_EINT35    9
#define HAL_GPIO_35_DEBUGMON3    10

#define HAL_GPIO_36_GPIO36    0
#define HAL_GPIO_36_I2S_MST1_TX    1
#define HAL_GPIO_36_I2S_SLV3_TX    2
#define HAL_GPIO_36_DMIC0_CLK    3
#define HAL_GPIO_36_SPI_MST2_CS0    4
#define HAL_GPIO_36_SPI_MST0_CS3_EX    5
#define HAL_GPIO_36_EINT36    9
#define HAL_GPIO_36_DEBUGMON4    10

#define HAL_GPIO_37_GPIO37    0
#define HAL_GPIO_37_I2S_MST2_TX    1
#define HAL_GPIO_37_I2S_SLV4_TX    2
#define HAL_GPIO_37_DMIC0_DAT    3
#define HAL_GPIO_37_SPI_MST2_SIO2    4
#define HAL_GPIO_37_SPI_MST0_CS4_EX    5
#define HAL_GPIO_37_SPI_MST2_CS1    6
#define HAL_GPIO_37_PWM11    7
#define HAL_GPIO_37_EINT37    9
#define HAL_GPIO_37_DEBUGMON5    10

#define HAL_GPIO_38_GPIO38    0
#define HAL_GPIO_38_I2S_MST3_CK    1
#define HAL_GPIO_38_I2S_SLV3_CK    2
#define HAL_GPIO_38_SPDIF_RX0    3
#define HAL_GPIO_38_SPI_MST2_SIO3    4
#define HAL_GPIO_38_SPI_MST0_CS5_EX    5
#define HAL_GPIO_38_SPI_MST2_CS2    6
#define HAL_GPIO_38_PWM12    7
#define HAL_GPIO_38_EINT38    9
#define HAL_GPIO_38_DEBUGMON6    10

#define HAL_GPIO_39_GPIO39    0
#define HAL_GPIO_39_I2S_MST3_WS    1
#define HAL_GPIO_39_I2S_SLV4_WS    2
#define HAL_GPIO_39_DMIC1_CLK    3
#define HAL_GPIO_39_SPI_MST0_CS6_EX    5
#define HAL_GPIO_39_PWM13    7
#define HAL_GPIO_39_EINT39    9
#define HAL_GPIO_39_DEBUGMON7    10

#define HAL_GPIO_40_GPIO40    0
#define HAL_GPIO_40_I2S_MST3_RX    1
#define HAL_GPIO_40_I2S_SLV4_CK    2
#define HAL_GPIO_40_DMIC1_DAT    3
#define HAL_GPIO_40_BT_EPA_ELNA_CSD    4
#define HAL_GPIO_40_SPI_MST0_CS7_EX    5
#define HAL_GPIO_40_SPI_MST1_SCK    6
#define HAL_GPIO_40_EINT40    9
#define HAL_GPIO_40_DEBUGMON8    10

#define HAL_GPIO_41_GPIO41    0
#define HAL_GPIO_41_I2S_MST3_MCLK    1
#define HAL_GPIO_41_I2S_SLV2_CK    2
#define HAL_GPIO_41_DMIC0_CLK    3
#define HAL_GPIO_41_BT_EPA_ELNA_CPS    4
#define HAL_GPIO_41_SPI_MST1_MOSI    6
#define HAL_GPIO_41_PWM9    7
#define HAL_GPIO_41_EINT41    9
#define HAL_GPIO_41_DEBUGMON9    10

#define HAL_GPIO_42_GPIO42    0
#define HAL_GPIO_42_I2S_MST1_MCLK    1
#define HAL_GPIO_42_I2S_SLV2_TX    2
#define HAL_GPIO_42_DMIC0_DAT    3
#define HAL_GPIO_42_BT_EPA_ELNA_CRX    4
#define HAL_GPIO_42_SPI_MST1_MISO    6
#define HAL_GPIO_42_PWM10    7
#define HAL_GPIO_42_SPI_MST1_CS3    8
#define HAL_GPIO_42_EINT42    9
#define HAL_GPIO_42_DEBUGMON10    10

#define HAL_GPIO_43_GPIO43    0
#define HAL_GPIO_43_I2S_MST3_TX    1
#define HAL_GPIO_43_I2S_SLV2_WS    2
#define HAL_GPIO_43_DSP_JTCK    3
#define HAL_GPIO_43_BT_EPA_ELNA_CTX    4
#define HAL_GPIO_43_SPI_MST1_CS0    6
#define HAL_GPIO_43_DMIC1_CLK    7
#define HAL_GPIO_43_EINT43    9
#define HAL_GPIO_43_DEBUGMON11    10

#define HAL_GPIO_44_GPIO44    0
#define HAL_GPIO_44_CLKO0    1
#define HAL_GPIO_44_DSP_JTDI    3
#define HAL_GPIO_44_BT_EPA_ELNA_CHL    4
#define HAL_GPIO_44_SPI_MST1_SIO2    6
#define HAL_GPIO_44_DMIC1_DAT    7
#define HAL_GPIO_44_SPI_MST1_CS1    8
#define HAL_GPIO_44_EINT44    9
#define HAL_GPIO_44_DEBUGMON12    10

#define HAL_GPIO_45_GPIO45    0
#define HAL_GPIO_45_DSP_JRST    3
#define HAL_GPIO_45_BT_EPA_ELNA_ANT_SEL     4
#define HAL_GPIO_45_SPI_MST1_SIO3    6
#define HAL_GPIO_45_SPI_MST1_CS2    8
#define HAL_GPIO_45_EINT45    9
#define HAL_GPIO_45_DEBUGMON13    10

#define HAL_GPIO_46_GPIO46    0
#define HAL_GPIO_46_DSP_JTDO    3
#define HAL_GPIO_46_GPIO_PIP_SEL_0    4
#define HAL_GPIO_46_SPI_MST2_SCK    6
#define HAL_GPIO_46_EINT46    9
#define HAL_GPIO_46_DEBUGMON14    10

#define HAL_GPIO_47_GPIO47    0
#define HAL_GPIO_47_UART2_TXD    2
#define HAL_GPIO_47_I2C0_SCL    3
#define HAL_GPIO_47_GPIO_PIP_SEL_1    4
#define HAL_GPIO_47_I3C0_SCL    6
#define HAL_GPIO_47_I2C2_SCL    7
#define HAL_GPIO_47_EINT47    9
#define HAL_GPIO_47_DEBUGMON15    10

#define HAL_GPIO_48_GPIO48    0
#define HAL_GPIO_48_UART2_RXD    2
#define HAL_GPIO_48_I2C0_SDA    3
#define HAL_GPIO_48_GPIO_PIP_SEL_2    4
#define HAL_GPIO_48_I3C0_SDA    6
#define HAL_GPIO_48_I2C2_SDA    7
#define HAL_GPIO_48_EINT48    9
#define HAL_GPIO_48_DEBUGMON0    10

#define HAL_GPIO_49_GPIO49    0
#define HAL_GPIO_49_UART1_TXD    2
#define HAL_GPIO_49_I2C1_SCL    3
#define HAL_GPIO_49_GPIO_PIP_SEL_3    4
#define HAL_GPIO_49_I3C1_SCL    6
#define HAL_GPIO_49_I2C2_SCL    7
#define HAL_GPIO_49_EINT49    9
#define HAL_GPIO_49_DEBUGMON1    10

#define HAL_GPIO_50_GPIO50    0
#define HAL_GPIO_50_UART1_RXD    2
#define HAL_GPIO_50_I2C1_SDA    3
#define HAL_GPIO_50_COEX_W1_DAT    4
#define HAL_GPIO_50_I3C1_SDA    6
#define HAL_GPIO_50_I2C2_SDA    7
#define HAL_GPIO_50_EINT50    9
#define HAL_GPIO_50_DEBUGMON2    10

#define HAL_GPIO_51_GPIO51    0
#define HAL_GPIO_51_CLKO0    1
#define HAL_GPIO_51_COEX_W3_BT_WLAN    4
#define HAL_GPIO_51_SPI_MST2_MOSI    6
#define HAL_GPIO_51_EINT51    9
#define HAL_GPIO_51_DEBUGMON3    10

#define HAL_GPIO_52_GPIO52    0
#define HAL_GPIO_52_PWM0    1
#define HAL_GPIO_52_COEX_W3_BT_ACT    4
#define HAL_GPIO_52_SPI_MST2_MISO    6
#define HAL_GPIO_52_SPI_MST2_CS3    7
#define HAL_GPIO_52_EINT52    9
#define HAL_GPIO_52_DEBUGMON4    10

#define HAL_GPIO_53_GPIO53    0
#define HAL_GPIO_53_IRRX    1
#define HAL_GPIO_53_COEX_W3_BT_PRI    4
#define HAL_GPIO_53_SPI_MST2_CS0    6
#define HAL_GPIO_53_EINT53    9
#define HAL_GPIO_53_DEBUGMON5    10

#define HAL_GPIO_54_GPIO54    0
#define HAL_GPIO_54_SPI_MST1_CS0    1
#define HAL_GPIO_54_COEX_W3_BT_WLAN    4
#define HAL_GPIO_54_SPI_MST2_SIO2    6
#define HAL_GPIO_54_SPI_MST2_CS1    7
#define HAL_GPIO_54_EINT54    9
#define HAL_GPIO_54_DEBUGMON6    10

#define HAL_GPIO_55_GPIO55    0
#define HAL_GPIO_55_SPI_MST1_SCK    1
#define HAL_GPIO_55_SPI_MST2_SIO3    6
#define HAL_GPIO_55_SPI_MST2_CS2    7
#define HAL_GPIO_55_EINT55    9
#define HAL_GPIO_55_DEBUGMON7    10

#define HAL_GPIO_56_GPIO56    0
#define HAL_GPIO_56_SPI_MST1_MOSI    1
#define HAL_GPIO_56_USB_ID    4
#define HAL_GPIO_56_EINT56    9
#define HAL_GPIO_56_DEBUGMON8    10

#define HAL_GPIO_57_GPIO57    0
#define HAL_GPIO_57_SPI_MST1_MISO    1
#define HAL_GPIO_57_USB_DRVVBUS    4
#define HAL_GPIO_57_EINT57    9
#define HAL_GPIO_57_DEBUGMON9    10

#ifdef __cplusplus
}
#endif

#endif /*HAL_GPIO_MODULE_ENABLED*/

#endif /*__HAL_PINMUX_DEFINE_H__*/


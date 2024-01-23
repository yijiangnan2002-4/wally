/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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

#include "hal_gpio.h"

#ifdef HAL_GPIO_MODULE_ENABLED
#include "hal_gpio_internal.h"
#include "hal_log.h"

const hal_gpio_cfg_reg_t gpio_cfg_table[] = {
//   DRV_REG  shift   IES_REG shift  PD_REG  PU_REG shift PUPD_REG  R0_REG     R1_REG  shift  RDSEL_REG shift  RSEL  shift  EH_REG  shift TDSEL_REG shift  SMT_REG SR_REG shift  G_REG shift
    { 0x20,     0,      0x50, 0,    0x60,    0x0070, 0,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0080,  0,   0XFFFF, 0XFF, 0XFF,   0XFF, 0x00B0,    0,    0x0090, 0x00A0, 0,    0x40, 0    },   //HAL_GPIO_0
    { 0x20,     3,      0x50, 1,    0x60,    0x0070, 1,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0080,  2,   0XFFFF, 0XFF, 0XFF,   0XFF, 0x00B0,    4,    0x0090, 0x00A0, 1,    0x40, 1    },   //HAL_GPIO_1
    { 0x20,     6,      0x50, 2,    0x60,    0x0070, 2,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0080,  4,   0XFFFF, 0XFF, 0XFF,   0XFF, 0x00B0,    8,    0x0090, 0x00A0, 2,    0x40, 2    },   //HAL_GPIO_2
    { 0x20,     9,      0x50, 3,    0x60,    0x0070, 3,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0080,  6,   0XFFFF, 0XFF, 0XFF,   0XFF, 0x00B0,    12,   0x0090, 0x00A0, 3,    0x40, 3    },   //HAL_GPIO_3
    { 0x20,     12,     0x50, 4,    0x60,    0x0070, 4,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0080,  8,   0XFFFF, 0XFF, 0XFF,   0XFF, 0x00B0,    16,   0x0090, 0x00A0, 4,    0x40, 4    },   //HAL_GPIO_4
    { 0x20,     15,     0x50, 5,    0x60,    0x0070, 5,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0080,  10,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x00B0,    20,   0x0090, 0x00A0, 5,    0x40, 5    },   //HAL_GPIO_5
    { 0x20,     18,     0x50, 6,    0x60,    0x0070, 6,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0080,  12,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x00B0,    24,   0x0090, 0x00A0, 6,    0x40, 6    },   //HAL_GPIO_6
    { 0x20,     21,     0x50, 7,    0x60,    0x0070, 7,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0080,  14,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x00B0,    28,   0x0090, 0x00A0, 7,    0x40, 7    },   //HAL_GPIO_7
    { 0x20,     24,     0x50, 8,    0x60,    0x0070, 8,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0080,  16,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x00C0,    0,    0x0090, 0x00A0, 8,    0x40, 8    },   //HAL_GPIO_8
    { 0x20,     27,     0x50, 9,    0x60,    0x0070, 9,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0080,  18,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x00C0,    4,    0x0090, 0x00A0, 9,    0x40, 9    },   //HAL_GPIO_9
    { 0x20,     0,      0xB0, 0,    0xD0,    0x0100, 0,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0140,  0,   0XFFFF, 0XFF, 0XFF,   0XFF, 0x01E0,    0,    0x01A0, 0x01C0, 0,    0x90, 0    },   //HAL_GPIO_10
    { 0x20,     3,      0xB0, 1,    0xD0,    0x0100, 1,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0140,  2,   0XFFFF, 0XFF, 0XFF,   0XFF, 0x01E0,    4,    0x01A0, 0x01C0, 1,    0x90, 1    },   //HAL_GPIO_11
    { 0x20,     6,      0xB0, 2,    0xD0,    0x0100, 2,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0140,  4,   0XFFFF, 0XFF, 0XFF,   0XFF, 0x01E0,    8,    0x01A0, 0x01C0, 2,    0x90, 2    },   //HAL_GPIO_12
    { 0x20,     9,      0xB0, 3,    0xD0,    0x0100, 3,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0140,  6,   0x0190, 0,    0x80,   0,    0x01E0,    12,   0x01A0, 0x01C0, 3,    0XFF, 0XFF },   //HAL_GPIO_13
    { 0x20,     12,     0xB0, 4,    0xD0,    0x0100, 4,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0140,  8,   0x0190, 2,    0x80,   3,    0x01E0,    16,   0x01A0, 0x01C0, 4,    0XFF, 0XFF },   //HAL_GPIO_14
    { 0x20,     15,     0xB0, 5,    0xD0,    0x0100, 5,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0140,  10,  0x0190, 4,    0x80,   6,    0x01E0,    20,   0x01A0, 0x01C0, 5,    0XFF, 0XFF },   //HAL_GPIO_15
    { 0x20,     18,     0xB0, 6,    0xD0,    0x0100, 6,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0140,  12,  0x0190, 6,    0x80,   9,    0x01E0,    24,   0x01A0, 0x01C0, 6,    0XFF, 0XFF },   //HAL_GPIO_16
    { 0x20,     21,     0xB0, 7,    0xD0,    0x0100, 7,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0140,  14,  0x0190, 8,    0x80,   12,   0x01E0,    28,   0x01A0, 0x01C0, 7,    0XFF, 0XFF },   //HAL_GPIO_17
    { 0x20,     24,     0xB0, 8,    0xD0,    0x0100, 8,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0140,  16,  0x0190, 10,   0x80,   15,   0x01F0,    0,    0x01A0, 0x01C0, 8,    0XFF, 0XFF },   //HAL_GPIO_18
    { 0x20,     27,     0xB0, 9,    0XFF,    0XFF,   0XFF, 0xF0,    0x0120,    0x0130,  0,      0x0140,  18,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x01F0,    4,    0x01A0, 0x01C0, 9,    0XFF, 0XFF },   //HAL_GPIO_19
    { 0x30,     0,      0xB0, 10,   0XFF,    0XFF,   0XFF, 0xF0,    0x0120,    0x0130,  1,      0x0140,  24,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x01F0,    8,    0x01A0, 0x01C0, 10,   0XFF, 0XFF },   //HAL_GPIO_20
    { 0x30,     3,      0xB0, 11,   0XFF,    0XFF,   0XFF, 0xF0,    0x0120,    0x0130,  2,      0x0150,  0,   0XFFFF, 0XFF, 0XFF,   0XFF, 0x01F0,    12,   0x01A0, 0x01C0, 11,   0XFF, 0XFF },   //HAL_GPIO_21
    { 0x30,     6,      0xB0, 12,   0XFF,    0XFF,   0XFF, 0xF0,    0x0120,    0x0130,  3,      0x0150,  6,   0XFFFF, 0XFF, 0XFF,   0XFF, 0x01F0,    16,   0x01A0, 0x01C0, 12,   0XFF, 0XFF },   //HAL_GPIO_22
    { 0x30,     9,      0xB0, 13,   0XFF,    0XFF,   0XFF, 0xF0,    0x0120,    0x0130,  4,      0x0150,  12,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x01F0,    20,   0x01A0, 0x01C0, 13,   0XFF, 0XFF },   //HAL_GPIO_23
    { 0x30,     12,     0xB0, 14,   0XFF,    0XFF,   0XFF, 0xF0,    0x0120,    0x0130,  5,      0x0150,  18,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x01F0,    24,   0x01A0, 0x01C0, 14,   0XFF, 0XFF },   //HAL_GPIO_24
    { 0x30,     15,     0xB0, 15,   0XFF,    0XFF,   0XFF, 0xF0,    0x0120,    0x0130,  6,      0x0150,  24,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x01F0,    28,   0x01A0, 0x01C0, 15,   0XFF, 0XFF },   //HAL_GPIO_25
    { 0x30,     18,     0xB0, 16,   0xD0,    0x0100, 9,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0150,  30,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x0200,    0,    0x01A0, 0x01C0, 16,   0x90, 3    },   //HAL_GPIO_26
    { 0x30,     21,     0xB0, 17,   0xD0,    0x0100, 10,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0160,  0,   0XFFFF, 0XFF, 0XFF,   0XFF, 0x0200,    4,    0x01A0, 0x01C0, 17,   0x90, 4    },   //HAL_GPIO_27
    { 0x30,     24,     0xB0, 18,   0xD0,    0x0100, 11,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0160,  2,   0XFFFF, 0XFF, 0XFF,   0XFF, 0x0200,    8,    0x01A0, 0x01C0, 18,   0x90, 5    },   //HAL_GPIO_28
    { 0x30,     27,     0xB0, 19,   0xD0,    0x0100, 12,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0160,  4,   0XFFFF, 0XFF, 0XFF,   0XFF, 0x0200,    12,   0x01A0, 0x01C0, 19,   0x90, 6    },   //HAL_GPIO_29
    { 0x40,     0,      0xB0, 20,   0xD0,    0x0100, 13,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0160,  6,   0XFFFF, 0XFF, 0XFF,   0XFF, 0x0200,    16,   0x01A0, 0x01C0, 20,   0x90, 7    },   //HAL_GPIO_30
    { 0x40,     3,      0xB0, 21,   0xD0,    0x0100, 14,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0160,  8,   0XFFFF, 0XFF, 0XFF,   0XFF, 0x0200,    20,   0x01A0, 0x01C0, 21,   0x90, 8    },   //HAL_GPIO_31
    { 0x40,     6,      0xB0, 22,   0xD0,    0x0100, 15,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0160,  10,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x0200,    24,   0x01A0, 0x01C0, 22,   0x90, 9    },   //HAL_GPIO_32
    { 0x40,     9,      0xB0, 23,   0xD0,    0x0100, 16,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0160,  12,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x0200,    28,   0x01A0, 0x01C0, 23,   0x90, 10   },   //HAL_GPIO_33
    { 0x40,     12,     0xB0, 24,   0xD0,    0x0100, 17,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0160,  14,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x0210,    0,    0x01A0, 0x01C0, 24,   0x90, 11   },   //HAL_GPIO_34
    { 0x40,     15,     0xB0, 25,   0xD0,    0x0100, 18,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0160,  16,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x0210,    4,    0x01A0, 0x01C0, 25,   0x90, 12   },   //HAL_GPIO_35
    { 0x40,     18,     0xB0, 26,   0xD0,    0x0100, 19,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0160,  18,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x0210,    8,    0x01A0, 0x01C0, 26,   0x90, 13   },   //HAL_GPIO_36
    { 0x40,     21,     0xB0, 27,   0xD0,    0x0100, 20,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0160,  20,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x0210,    12,   0x01A0, 0x01C0, 27,   0x90, 14   },   //HAL_GPIO_37
    { 0x40,     24,     0xB0, 28,   0xD0,    0x0100, 21,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0160,  22,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x0210,    16,   0x01A0, 0x01C0, 28,   0x90, 15   },   //HAL_GPIO_38
    { 0x40,     27,     0xB0, 29,   0xD0,    0x0100, 22,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0160,  24,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x0210,    20,   0x01A0, 0x01C0, 29,   0x90, 16   },   //HAL_GPIO_39
    { 0x50,     0,      0xB0, 30,   0xD0,    0x0100, 23,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0160,  26,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x0210,    24,   0x01A0, 0x01C0, 30,   0x90, 17   },   //HAL_GPIO_40
    { 0x50,     3,      0xB0, 31,   0xD0,    0x0100, 24,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0160,  28,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x0210,    28,   0x01A0, 0x01C0, 31,   0x90, 18   },   //HAL_GPIO_41
    { 0x50,     6,      0xC0, 0,    0xD0,    0x0100, 25,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0160,  30,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x0220,    0,    0x01B0, 0x01D0, 0,    0x90, 19   },   //HAL_GPIO_42
    { 0x50,     9,      0xC0, 1,    0xD0,    0x0100, 26,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0170,  0,   0XFFFF, 0XFF, 0XFF,   0XFF, 0x0220,    4,    0x01B0, 0x01D0, 1,    0x90, 20   },   //HAL_GPIO_43
    { 0x50,     12,     0xC0, 2,    0xD0,    0x0100, 27,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0170,  2,   0XFFFF, 0XFF, 0XFF,   0XFF, 0x0220,    8,    0x01B0, 0x01D0, 2,    0x90, 21   },   //HAL_GPIO_44
    { 0x50,     15,     0xC0, 3,    0xD0,    0x0100, 28,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0170,  4,   0XFFFF, 0XFF, 0XFF,   0XFF, 0x0220,    12,   0x01B0, 0x01D0, 3,    0x90, 22   },   //HAL_GPIO_45
    { 0x50,     18,     0xC0, 4,    0xD0,    0x0100, 29,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0170,  6,   0XFFFF, 0XFF, 0XFF,   0XFF, 0x0220,    16,   0x01B0, 0x01D0, 4,    0x90, 23   },   //HAL_GPIO_46
    { 0x50,     21,     0xC0, 5,    0xD0,    0x0100, 30,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0170,  8,   0x0190,   12, 0x80,   18,   0x0220,    20,   0x01B0, 0x01D0, 5,    0XFF, 0XFF },   //HAL_GPIO_47
    { 0x50,     24,     0xC0, 6,    0xD0,    0x0100, 31,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0170,  10,  0x0190,   14, 0x80,   21,   0x0220,    24,   0x01B0, 0x01D0, 6,    0XFF, 0XFF },   //HAL_GPIO_48
    { 0x50,     27,     0xC0, 7,    0xE0,    0x0110, 0,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0170,  12,  0x0190,   16, 0x80,   24,   0x0220,    28,   0x01B0, 0x01D0, 7,    0XFF, 0XFF },   //HAL_GPIO_49
    { 0x60,     0,      0xC0, 8,    0xE0,    0x0110, 1,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0170,  14,  0x0190,   18, 0x80,   27,   0x0230,    0,    0x01B0, 0x01D0, 8,    0XFF, 0XFF },   //HAL_GPIO_50
    { 0x60,     3,      0xC0, 9,    0xE0,    0x0110, 2,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0170,  16,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x0230,    4,    0x01B0, 0x01D0, 9,    0x90, 24   },   //HAL_GPIO_51
    { 0x60,     6,      0xC0, 10,   0xE0,    0x0110, 3,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0170,  18,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x0230,    8,    0x01B0, 0x01D0, 10,   0x90, 25   },   //HAL_GPIO_52
    { 0x60,     9,      0xC0, 11,   0xE0,    0x0110, 4,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0170,  20,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x0230,    12,   0x01B0, 0x01D0, 11,   0x90, 26   },   //HAL_GPIO_53
    { 0x60,     12,     0xC0, 12,   0xE0,    0x0110, 5,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0170,  22,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x0230,    16,   0x01B0, 0x01D0, 12,   0x90, 27   },   //HAL_GPIO_54
    { 0x60,     15,     0xC0, 13,   0xE0,    0x0110, 6,    0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0170,  24,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x0230,    20,   0x01B0, 0x01D0, 13,   0x90, 28   },   //HAL_GPIO_55
    { 0x30,     0,      0x50, 10,   0x60,    0x0070, 10,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0080,  20,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x00C0,    8,    0x0090, 0x00A0, 10,   0x40, 10   },   //HAL_GPIO_56
    { 0x30,     3,      0x50, 11,   0x60,    0x0070, 11,   0XFF,    0XFFFF,    0XFFFF,  0XFF,   0x0080,  22,  0XFFFF, 0XFF, 0XFF,   0XFF, 0x00C0,    12,   0x0090, 0x00A0, 11,   0x40, 11   },   //HAL_GPIO_57
};


void gpio_get_state(hal_gpio_pin_t gpio_pin, gpio_state_t *gpio_state)
{

    uint32_t mode;
    uint32_t dir;
    uint32_t din;
    uint32_t dout;
    uint32_t pu;
    uint32_t pd;
    uint32_t pupd;
    uint32_t r0;
    uint32_t r1;
    uint32_t rsel;

    gpio_pull_type_t pull_type;
    uint32_t temp;
    uint32_t shift;
    uint32_t reg_index;
    uint32_t bit_index;
    hal_gpio_driving_current_t driving_value;

    //const char *direct[2] = {"input", "output"};
    //const char *pull_state[10] = {"disable_pull", "PU_R", "PD_R", "PU_R0","PD_R0", "PU_R0_R1", "PD_R0_R1", "PUPD_Error","PU_R1","PD_R1"};


    reg_index = gpio_pin / 8;
    bit_index = (gpio_pin % 8) * 4;
    mode = (gpio_base->GPIO_MODE.RW[reg_index] >> (bit_index) & 0xf);

    reg_index = gpio_pin / 32;
    bit_index = gpio_pin % 32;
    dir  = (gpio_base->GPIO_DIR.RW[reg_index] >> (bit_index) & 0x1);
    din  = (gpio_base->GPIO_DIN.R[reg_index] >> (bit_index) & 0x1);
    dout = (gpio_base->GPIO_DOUT.RW[reg_index] >> (bit_index) & 0x1);

    pu = 0xf;
    pd = 0xf;
    pupd = 0xf;
    r0   = 0xf;
    r1   = 0xf;

    shift = 0xff;
    pull_type = GPIO_PUPD_ERR;

    if (gpio_cfg_table[gpio_pin].pupd_shift != 0xff) {
        shift = gpio_cfg_table[gpio_pin].pupd_shift;
        if((gpio_pin > HAL_GPIO_9) && (gpio_pin < HAL_GPIO_56)) {
            pu = (GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].pu_reg) >> shift) & 0x01;
            pd = (GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].pd_reg) >> shift) & 0x01;
        } else {
            pu = (GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].pu_reg) >> shift) & 0x01;
            pd = (GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].pd_reg) >> shift) & 0x01;
        }

        temp = (pu << 4) + pd;
        if (gpio_cfg_table[gpio_pin].rsel_shift != 0xff) {
            shift = gpio_cfg_table[gpio_pin].rsel_shift;
            if((gpio_pin > HAL_GPIO_9) && (gpio_pin < HAL_GPIO_56)) {
                rsel = (GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].rsel_reg) >> shift) & 0x03;
            } else {
                rsel = (GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].rsel_reg) >> shift) & 0x03;
            }

            temp |= ((rsel & 0x1) << 8 | ((rsel >> 1) & 0x1) << 12);
        }
        //log_hal_msgid_info("pu=%d pd=%d, temp=%.3x\r\n", 3, pu,pd,temp);

        if (temp == 0x00) {
            pull_type = GPIO_NO_PULL;
        } else if (temp == 0x10) {
            pull_type = GPIO_PU_R;
        } else if (temp == 0x01) {
            pull_type = GPIO_PD_R;
        } else if (temp == 0x101) {
            pull_type = GPIO_PD_RSEL01;
        } else if (temp == 0x1001) {
            pull_type = GPIO_PD_RSEL10;
        } else if (temp == 0x1101) {
            pull_type = GPIO_PD_RSEL11;
        } else if (temp == 0x110) {
            pull_type = GPIO_PU_RSEL01;
        } else if (temp == 0x1010) {
            pull_type = GPIO_PU_RSEL10;
        } else if (temp == 0x1110) {
            pull_type = GPIO_PU_RSEL11;
        } else if ((temp & 0xFF) == 0x11) {
            pull_type = GPIO_PUPD_ERR;
            log_hal_msgid_info("error pu = %x, pd= %x\r\n", 2, pu, pd);
        }
    } else if (gpio_cfg_table[gpio_pin].pupd_r0_r1_shift != 0xff) {
        shift = gpio_cfg_table[gpio_pin].pupd_r0_r1_shift;
        if((gpio_pin > HAL_GPIO_9) && (gpio_pin < HAL_GPIO_56)) {
            pupd = (GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].pupd_reg) >> shift) & 0x01;
            r0 = (GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].r0_reg) >> shift) & 0x01;
            r1 = (GPIO_REG32(IO_CFG_1_BASE + gpio_cfg_table[gpio_pin].r1_reg) >> shift) & 0x01;
        } else {
            pupd = (GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].pupd_reg) >> shift) & 0x01;
            r0 = (GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].r0_reg) >> shift) & 0x01;
            r1 = (GPIO_REG32(IO_CFG_0_BASE + gpio_cfg_table[gpio_pin].r1_reg) >> shift) & 0x01;
        }

        temp = (pupd << 8) + (r0 << 4) + r1;
        //log_hal_msgid_info("pupd=%d r0=%d, r1=%d, temp=%.3x\r\n", 4, pupd,r0,r1,temp);

        if (temp == 0x010) {
            pull_type = GPIO_PU_R0;
        } else if (temp == 0x001) {
            pull_type = GPIO_PU_R1;
        } else if (temp == 0x110) {
            pull_type = GPIO_PD_R0;
        } else if (temp == 0x101) {
            pull_type = GPIO_PD_R1;
        } else if (temp == 0x011) {
            pull_type = GPIO_PU_R0_R1;
        } else if (temp == 0x111) {
            pull_type = GPIO_PD_R0_R1;
        } else if ((temp == 0x100) || (temp == 0x000)) {
            pull_type = GPIO_NO_PULL;
        } else {
            pull_type = GPIO_PUPD_ERR;
            log_hal_msgid_info("error pupd-r0-r1 = %x\r\n", 1, temp);
        }
    }


    hal_gpio_get_driving_current((hal_gpio_pin_t)gpio_pin, &driving_value);

    gpio_state->mode = mode;
    gpio_state->dir  = dir;
    gpio_state->din  = din;
    gpio_state->dout = dout;
    gpio_state->pull_type = pull_type;
    gpio_state->current_type = (uint8_t)driving_value;

    //log_hal_msgid_info("GPIO%d, mode=%d, %s, din=%d, dout=%d, %s\r\n",6, gpio_pin, mode, direct[dir], din,dout,pull_state[pull_type]);
}

#endif


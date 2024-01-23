
/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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

/**
 * File: app_va_xiaoai_config.h
 *
 * Description: This file defines the interface of XiaoAI configure.
 *
 */

#ifndef __APP_VA_XIAOAI_CONFIG_H__
#define __APP_VA_XIAOAI_CONFIG_H__

#ifdef AIR_XIAOAI_ENABLE

#ifndef AIR_TWS_ENABLE
#error "For XiaoAI feature, please enable AIR_TWS_ENABLE"
#endif

#if 1
#define APP_VA_XIAOAI_VID           0x2717
#define APP_VA_XIAOAI_PID           0x5019

#define APP_VA_XIAOAI_VERSION       0x1001     /* 1.0.0.1 (initial version). */

#define APP_VA_XIAOAI_DEVICE_TYPE   0          /* 0 - invalid, device_type/color 1~15. */

#define APP_VA_XIAOAI_MAJOR_ID      0x01
#define APP_VA_XIAOAI_MINOR_ID      0x01

/* Customer configure option: MIUI Fast_Connect Product ID (24 bits). */
#define APP_MIUI_FC_PRODUCT_ID     {APP_VA_XIAOAI_MAJOR_ID, APP_VA_XIAOAI_MINOR_ID, 0x00}

#endif

#define APP_VA_XIAOAI_SILENCE_OTA_TIMER          (10 * 60 * 1000)   // 10 min when both close lid

#define APP_VA_XIAOAI_HIGH_BLE_CONN_INTERVAL     165
#define APP_VA_XIAOAI_LOW_BLE_CONN_INTERVAL      25


#ifdef AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE

/* Customer configure option: MIUI Fast_Connect secp256r1 private ID (32Bytes - 256bits). */
#define APP_MIUI_FC_PRIVATE_KEY    {                        \
    0x02, 0xB4, 0x37, 0xB0, 0xED, 0xD6, 0xBB, 0xD4,         \
    0x29, 0x06, 0x4A, 0x4E, 0x52, 0x9F, 0xCB, 0xF1,         \
    0xC4, 0x8D, 0x0D, 0x62, 0x49, 0x24, 0xD5, 0x92,         \
    0x27, 0x4B, 0x7E, 0xD8, 0x11, 0x93, 0xD7, 0x63          \
};

/* Customer configure option: MIUI Fast_Connect secp256r1 public ID (64Bytes - saved in MI Cloud Server). */
#define APP_MIUI_FC_PUBLIC_KEY    {                         \
    0xF7, 0xD4, 0x96, 0xA6, 0x2E, 0xCA, 0x41, 0x63,         \
    0x51, 0x54, 0x0A, 0xA3, 0x43, 0xBC, 0x69, 0x0A,         \
    0x61, 0x09, 0xF5, 0x51, 0x50, 0x06, 0x66, 0xB8,         \
    0x3B, 0x12, 0x51, 0xFB, 0x84, 0xFA, 0x28, 0x60,         \
    0x79, 0x5E, 0xBD, 0x63, 0xD3, 0xB8, 0x83, 0x6F,         \
    0x44, 0xA9, 0xA3, 0xE2, 0x8B, 0xB3, 0x40, 0x17,         \
    0xE0, 0x15, 0xF5, 0x97, 0x93, 0x05, 0xD8, 0x49,         \
    0xFD, 0xF8, 0xDE, 0x10, 0x12, 0x3B, 0x61, 0xD2          \
};

#endif


#endif /* AIR_XIAOAI_ENABLED */

#endif /* __APP_VA_XIAOAI_CONFIG_H__ */


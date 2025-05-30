/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#ifndef __SERIAL_PORT_CONFIG_H__
#define __SERIAL_PORT_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MTK_PORT_SERVICE_ENABLE

//#define MTK_PORT_SERVICE_NVDM_ENABLE

#ifdef MTK_PORT_SERVICE_SLIM_ENABLE
/* define size of internal buffers */
#define SERIAL_PORT_RECEIVE_BUFFER_SIZE 1024
#define SERIAL_PORT_SEND_BUFFER_SIZE    1024
#else
#define SERIAL_PORT_RECEIVE_BUFFER_SIZE 2048
#define SERIAL_PORT_SEND_BUFFER_SIZE    2048
#endif

/* define name of port setting in NVDM */
#define PORT_SETTING_GROUP_NAME         "port_service"
#define PORT_SETTING_NAME_PORT_ASSIGN   "port_assign"
#define PORT_SETTING_NAME_PORT_CONFIG   "port_config"

/* define max size of user name */
#define SERIAL_PORT_USER_NAME_SIZE      20

/* define default values for porting devices */
#define UART_DEFAULT_BAUDRATE           HAL_UART_BAUDRATE_921600//HAL_UART_BAUDRATE_115200
// #define USB_DEFAULT_XXX
#ifdef MTK_PORT_SERVICE_BT_ENABLE
#define BTSPP_DEFAULT_SERVICE_UUID128 {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
#define BTSPP_DEFAULT_SERVICE_NAME "Airoha_APP"
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif


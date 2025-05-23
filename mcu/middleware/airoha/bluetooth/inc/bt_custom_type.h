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

#ifndef BT_CUSTOM_TYPE_H
#define BT_CUSTOM_TYPE_H

#include "bt_type.h"


/*
 * +---------+-----------------+
 * |Module ID  | Data                      |
 * +---------+----------------- +
 */
#define BT_MODULE_CUSTOM_GENERAL            (BT_MODULE_CUSTOM | 0x00U<<BT_MODULE_CUSTOM_OFFSET)  /**< Define the General module. 0xF8000000.*/
#define BT_MODULE_CUSTOM_SINK               (BT_MODULE_CUSTOM | 0x01U<<BT_MODULE_CUSTOM_OFFSET)  /**< Define the Sink module. 0xF8100000.*/
#ifdef MTK_BT_CM_SUPPORT
#define BT_MODULE_CUSTOM_CM                 (BT_MODULE_CUSTOM | 0x02U<<BT_MODULE_CUSTOM_OFFSET)  /**< Define the Connection Manage module. 0xF8200000.*/
#else
#define BT_MODULE_CUSTOM_CONNECTION_MANAGER (BT_MODULE_CUSTOM | 0x02U<<BT_MODULE_CUSTOM_OFFSET)  /**< Define the Connection Manage module. 0xF8200000.*/
#endif
#define BT_MODULE_CUSTOM_ROLE_HANDOVER      (BT_MODULE_CUSTOM | 0x03U<<BT_MODULE_CUSTOM_OFFSET)  /**< Define the Role Handover module. 0xF8300000.*/
#define BT_MODULE_CUSTOM_AWS_MCE_SRV        (BT_MODULE_CUSTOM | 0x04U<<BT_MODULE_CUSTOM_OFFSET)  /**< Define the Aws mce service module. 0xF8400000.*/
#define BT_MODULE_CUSTOM_SOURCE             (BT_MODULE_CUSTOM | 0x05U<<BT_MODULE_CUSTOM_OFFSET)  /**< Define the Source module. 0xF8500000.*/


#endif /* BT_CUSTOM_TYPE_H */


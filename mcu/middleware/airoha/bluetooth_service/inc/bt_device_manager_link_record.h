/* Copyright Statement:
 *
 * (C) 2022  Airoha Technology Corp. All rights reserved.
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

#ifndef __BT_DEVICE_MANAGER_LINK_RECORD_H__
#define __BT_DEVICE_MANAGER_LINK_RECORD_H__

#include "bt_type.h"

#define BT_DEVICE_MANAGER_LINK_TYPE_LE  0x00
#define BT_DEVICE_MANAGER_LINK_TYPE_EDR 0x01
typedef uint8_t bt_device_manager_link_t;

#define BT_DEVICE_MANAGER_LINK_RECORD_MAXIMUM   (4)

typedef struct {
    bt_addr_type_t remote_bd_type;
    bt_bd_addr_t   remote_addr;
    bt_device_manager_link_t link_type;
} bt_device_manager_link_record_item_t;

typedef struct {
    uint8_t     connected_num;
    bt_device_manager_link_record_item_t connected_device[BT_DEVICE_MANAGER_LINK_RECORD_MAXIMUM];
} bt_device_manager_link_record_t;

/* Must be implemented by APP layer. */
void        bt_device_manager_link_record_takeover_callback(const bt_device_manager_link_record_item_t *item);

const bt_device_manager_link_record_t *
            bt_device_manager_link_record_get_connected_link();

void        bt_device_manager_link_record_set_max_num(uint8_t size);

#ifdef MTK_AWS_MCE_ENABLE
void        bt_device_manager_link_record_aws_update_context(const bt_device_manager_link_record_t *data);
#endif

bt_status_t bt_device_manager_link_record_init();

#endif /* __BT_DEVICE_MANAGER_LINK_RECORD_H__ */



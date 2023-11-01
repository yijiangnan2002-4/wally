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

#ifndef _BT_HOGP_INTERNAL_H_
#define _BT_HOGP_INTERNAL_H_
#include "bt_hogp.h"

#define BT_HOGP_REPORT_1_INDEX                  0x00
#define BT_HOGP_REPORT_2_INDEX                  0x01
#define BT_HOGP_REPORT_3_INDEX                  0x02
#define BT_HOGP_REPORT_INDEX_MAX                0x03
typedef uint8_t bt_hogp_report_index_t;

#define BT_HOGP_INVALID_REPORT_ID               0xFF

#define BT_HOGP_CONFIG_REPORT_1         BT_HOGP_INPUT_REPORT
#define BT_HOGP_CONFIG_REPORT_2         BT_HOGP_FEATURE_REPORT
#define BT_HOGP_CONFIG_REPORT_3         BT_HOGP_FEATURE_REPORT

typedef struct {
    uint8_t                 report_id;
    bt_hogp_report_t        report_type;
} bt_hogp_report_group_t;

bt_status_t bt_hogp_notify_user_by_callback(bt_hogp_event_t event, void *payload, void *output);
bt_hogp_report_group_t *bt_hogp_get_report_group_by_index(bt_hogp_report_index_t index);
bt_hogp_information_t *bt_hogp_get_hid_information(void);
bt_hogp_protocol_mode_t bt_hogp_get_protocol_mode(void);
bt_hogp_report_descriptor_t *bt_hogp_get_report_descriptor(void);

#endif/* _BT_HOGP_INTERNAL_H_ */

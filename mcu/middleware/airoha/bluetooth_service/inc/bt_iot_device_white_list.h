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
/* Airoha restricted information */


#ifndef __BT_IOT_DEVICE_WHITE_LIST_H__
#define __BT_IOT_DEVICE_WHITE_LIST_H__

#include "bt_device_manager_internal.h"

#define MAX_IOT_CASE    (20)

typedef enum {
    /* sink music iot case */
    BT_IOT_MUSIC_NOTIFY_VENDOR_CODEC_BY_N_PACKET = 1,
    BT_IOT_MUSIC_SET_LATENCY_TO_ORIGINAL,          // 2
    BT_IOT_MUSIC_SET_LATENCY_TO_250,               // 3
    BT_IOT_MUSIC_SET_SPECIAL_MTU,                  // 4
    BT_IOT_MUSIC_SEND_AVRCP_FORCE_PAUSE,           // 5
    BT_IOT_MUSIC_IGNORE_ABSOLUTE_VOLUME,           // 6
    BT_IOT_MUSIC_NOTIFY_N9_NO_SLEEP,               // 7
    BT_IOT_MUSIC_SET_SPECIAL_SAMPLE_COUNT_TO_MAGIC_NUMBER,  // 8
    BT_IOT_MUSIC_SET_LOCAL_ASI,                    // 9
    BT_IOT_MUSIC_IS_WALKMAN_WITH_SBC_CODEC_AND_NO_HFP,      // 10
    BT_IOT_MUSIC_IS_WALKMAN_SET_RATIO_AND_PCDC_OBSERVATION,      // 11
    BT_IOT_MUSIC_SET_LATENCY_TO_170,               // 12
    BT_IOT_CALL_ESCO_DISCONNECT_WITH_CALL_ACTIVE_RECONNECT,     /* 13, IOT issue: esco is disconnected when the headset is reconnected and call is active. */
    BT_IOT_CALL_HEADSET_BLOCK_ESCO_CONNECTION_WITH_RECONNECT,   /* 14, IOT issue: when the HFP reconnect and the call is exist, the headset will block the esco connection initiated by the dongle. */
    BT_IOT_MUSIC_NO_PRE_NEXT_PASS_THROUGH_CMD, /* 15, IOT issue: when music is paused , plug out & in dongle again, earbuds can't send prex & next passthrough cmd. */
    BT_IOT_IDENTIFY_LEA_SUPPORT_DEVICE, /* 16, To identify LEA support device. */
} bt_iot_device_white_list_iot_case_t;

typedef struct {
    char device_name[BT_DEVICE_MANAGER_NAME_LENGTH];
    bt_device_manager_db_remote_version_info_t  version_info;
    bt_device_manager_db_remote_pnp_info_t      pnp_info;
    uint32_t cod;
} bt_iot_device_identify_information_t;


uint32_t        bt_iot_device_white_list_get_iot_id(bt_iot_device_identify_information_t *identify_info);
bt_status_t     bt_iot_device_white_list_get_iot_case_list_by_addr(bt_bd_addr_t *addr, bt_iot_device_white_list_iot_case_t ret_list[MAX_IOT_CASE]);
bt_iot_device_white_list_iot_case_t     *bt_iot_device_white_list_get_iot_case_list_by_addr_2(bt_bd_addr_t *addr);
bool            bt_iot_device_white_list_check_iot_case(bt_bd_addr_t *addr, bt_iot_device_white_list_iot_case_t iot_case);

#endif

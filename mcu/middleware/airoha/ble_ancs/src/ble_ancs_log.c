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

#include "syslog.h"


#if !defined(MTK_DEBUG_LEVEL_NONE)
ATTR_LOG_STRING_LIB BLE_ANCS_001[] = LOG_INFO_PREFIX(ANCS) "[ANCS]i = %d, char_content = 0x%x";
ATTR_LOG_STRING_LIB BLE_ANCS_002[] = LOG_INFO_PREFIX(ANCS) "[ANCS]i = %d, conn_cntx = 0x%x";
ATTR_LOG_STRING_LIB BLE_ANCS_003[] = LOG_INFO_PREFIX(ANCS) "[ANCS]ble_ancs_init";
ATTR_LOG_STRING_LIB BLE_ANCS_004[] = LOG_INFO_PREFIX(ANCS) "[ANCS]ANCS is full!";
ATTR_LOG_STRING_LIB BLE_ANCS_005[] = LOG_INFO_PREFIX(ANCS) "[ANCS]ble_ancs_deinit";
ATTR_LOG_STRING_LIB BLE_ANCS_006[] = LOG_INFO_PREFIX(ANCS) "[ANCS]command_len = %d";
ATTR_LOG_STRING_LIB BLE_ANCS_007[] = LOG_INFO_PREFIX(ANCS) "[ANCS]Get_notification_attribute_command: attr_id = %d, command_len = %d";
ATTR_LOG_STRING_LIB BLE_ANCS_008[] = LOG_INFO_PREFIX(ANCS) "[ANCS]Get_app_attribute_command: attr_id = %d, command_len = %d";
ATTR_LOG_STRING_LIB BLE_ANCS_009[] = LOG_INFO_PREFIX(ANCS) "[ANCS]ERROR!!! length = %d";
ATTR_LOG_STRING_LIB BLE_ANCS_010[] = LOG_INFO_PREFIX(ANCS) "[ANCS]ble_ancs_parse_attributes: length = %d, data = 0x%x";
ATTR_LOG_STRING_LIB BLE_ANCS_011[] = LOG_INFO_PREFIX(ANCS) "[ANCS]cut_type = %d, left_len = %d";
ATTR_LOG_STRING_LIB BLE_ANCS_012[] = LOG_INFO_PREFIX(ANCS) "[ANCS]ERROR!!! Invalid length";
ATTR_LOG_STRING_LIB BLE_ANCS_013[] = LOG_INFO_PREFIX(ANCS) "[ANCS]: ERROR!!! Invalid UUID";
ATTR_LOG_STRING_LIB BLE_ANCS_014[] = LOG_INFO_PREFIX(ANCS) "[ANCS]appid_len = %d, length = %d";
ATTR_LOG_STRING_LIB BLE_ANCS_015[] = LOG_INFO_PREFIX(ANCS) "[ANCS]ERROR!!! wrong appid(1)";
ATTR_LOG_STRING_LIB BLE_ANCS_016[] = LOG_INFO_PREFIX(ANCS) "[ANCS]ERROR!!! wrong appid(2)";
ATTR_LOG_STRING_LIB BLE_ANCS_017[] = LOG_INFO_PREFIX(ANCS) "[ANCS]command id = %d, curr_attr = 0x%x";
ATTR_LOG_STRING_LIB BLE_ANCS_018[] = LOG_INFO_PREFIX(ANCS) "[ANCS]all attributed parsed!";
ATTR_LOG_STRING_LIB BLE_ANCS_019[] = LOG_INFO_PREFIX(ANCS) "[ANCS]ERROR!!! Can't find right attr_id = %d";
ATTR_LOG_STRING_LIB BLE_ANCS_020[] = LOG_INFO_PREFIX(ANCS) "[ANCS]left_len = %d, left_to_read = %d";
ATTR_LOG_STRING_LIB BLE_ANCS_021[] = LOG_INFO_PREFIX(ANCS) "[ANCS]ble_ancs_enable_data_source, conn_handle = %d";
ATTR_LOG_STRING_LIB BLE_ANCS_022[] = LOG_INFO_PREFIX(ANCS) "[ANCS]char = 0x%x, handle = 0x%x";
ATTR_LOG_STRING_LIB BLE_ANCS_023[] = LOG_INFO_PREFIX(ANCS) "[ANCS]Get characteristic fail!";
ATTR_LOG_STRING_LIB BLE_ANCS_024[] = LOG_INFO_PREFIX(ANCS) "[ANCS]Connection context is NULL";
ATTR_LOG_STRING_LIB BLE_ANCS_025[] = LOG_INFO_PREFIX(ANCS) "[ANCS]ble_ancs_disable_data_source, conn_handle = %d";
ATTR_LOG_STRING_LIB BLE_ANCS_026[] = LOG_INFO_PREFIX(ANCS) "[ANCS]Get characteristic fail!";
ATTR_LOG_STRING_LIB BLE_ANCS_027[] = LOG_INFO_PREFIX(ANCS) "[ANCS]Connection context is NULL";
ATTR_LOG_STRING_LIB BLE_ANCS_028[] = LOG_INFO_PREFIX(ANCS) "[ANCS]ble_ancs_enable_notification_source, conn_handle = %d";
ATTR_LOG_STRING_LIB BLE_ANCS_029[] = LOG_INFO_PREFIX(ANCS) "[ANCS]char = 0x%x, handle = 0x%x";
ATTR_LOG_STRING_LIB BLE_ANCS_030[] = LOG_INFO_PREFIX(ANCS) "[ANCS]enable notification status = %x";
ATTR_LOG_STRING_LIB BLE_ANCS_031[] = LOG_INFO_PREFIX(ANCS) "[ANCS]Get characteristic fail!";
ATTR_LOG_STRING_LIB BLE_ANCS_032[] = LOG_INFO_PREFIX(ANCS) "[ANCS]Connection context is NULL";
ATTR_LOG_STRING_LIB BLE_ANCS_033[] = LOG_INFO_PREFIX(ANCS) "[ANCS]ble_ancs_disable_notification_source, conn_handle = %d";
ATTR_LOG_STRING_LIB BLE_ANCS_034[] = LOG_INFO_PREFIX(ANCS) "[ANCS]Get characteristic fail!";
ATTR_LOG_STRING_LIB BLE_ANCS_035[] = LOG_INFO_PREFIX(ANCS) "[ANCS]Connection context is NULL";
ATTR_LOG_STRING_LIB BLE_ANCS_036[] = LOG_INFO_PREFIX(ANCS) "[ANCS]ble_ancs_get_notification_attributes: attr_num = %d, attr_list = %x";
ATTR_LOG_STRING_LIB BLE_ANCS_037[] = LOG_INFO_PREFIX(ANCS) "[ANCS]ERROR!!! Empty attrbute list!";
ATTR_LOG_STRING_LIB BLE_ANCS_038[] = LOG_INFO_PREFIX(ANCS) "[ANCS]req_attr = %x";
ATTR_LOG_STRING_LIB BLE_ANCS_039[] = LOG_INFO_PREFIX(ANCS) "[ANCS]Get characteristic fail!";
ATTR_LOG_STRING_LIB BLE_ANCS_040[] = LOG_INFO_PREFIX(ANCS) "[ANCS]Connection context is NULL";
ATTR_LOG_STRING_LIB BLE_ANCS_041[] = LOG_INFO_PREFIX(ANCS) "[ANCS]ble_ancs_get_app_attributes: appid_len = %d, appid = %x, attr_num = %d, p_attr = %x";
ATTR_LOG_STRING_LIB BLE_ANCS_042[] = LOG_INFO_PREFIX(ANCS) "[ANCS]ERROR!!! INPUT PARAMETER WRONG!";
ATTR_LOG_STRING_LIB BLE_ANCS_043[] = LOG_INFO_PREFIX(ANCS) "[ANCS]req_attr = %x";
ATTR_LOG_STRING_LIB BLE_ANCS_044[] = LOG_INFO_PREFIX(ANCS) "[ANCS]Get characteristic fail!";
ATTR_LOG_STRING_LIB BLE_ANCS_045[] = LOG_INFO_PREFIX(ANCS) "[ANCS]Connection context is NULL";
ATTR_LOG_STRING_LIB BLE_ANCS_046[] = LOG_INFO_PREFIX(ANCS) "[ANCS]ble_ancs_init";
ATTR_LOG_STRING_LIB BLE_ANCS_047[] = LOG_INFO_PREFIX(ANCS) "[ANCS]ERROR!!! INPUT PARAMETER WRONG!";
ATTR_LOG_STRING_LIB BLE_ANCS_048[] = LOG_INFO_PREFIX(ANCS) "[ANCS]Get characteristic fail!";
ATTR_LOG_STRING_LIB BLE_ANCS_049[] = LOG_INFO_PREFIX(ANCS) "[ANCS]Connection context is NULL";
ATTR_LOG_STRING_LIB BLE_ANCS_050[] = LOG_INFO_PREFIX(ANCS) "BT_ANCS_GATTC_WRITE_CHARC: status = %x";
ATTR_LOG_STRING_LIB BLE_ANCS_051[] = LOG_INFO_PREFIX(ANCS) "BT_ANCS_GATTC_WRITE_CHARC: context = 0x%x";
ATTR_LOG_STRING_LIB BLE_ANCS_052[] = LOG_INFO_PREFIX(ANCS) "BT_ANCS_GAP_LE_DISCONNECT_IND: status = %x";
#else
ATTR_LOG_STRING_LIB BLE_ANCS_001[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_002[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_003[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_004[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_005[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_006[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_007[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_008[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_009[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_010[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_011[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_012[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_013[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_014[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_015[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_016[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_017[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_018[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_019[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_020[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_021[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_022[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_023[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_024[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_025[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_026[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_027[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_028[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_029[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_030[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_031[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_032[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_033[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_034[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_035[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_036[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_037[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_038[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_039[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_040[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_041[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_042[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_043[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_044[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_045[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_046[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_047[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_048[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_049[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_050[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_051[] = "";
ATTR_LOG_STRING_LIB BLE_ANCS_052[] = "";

#endif




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

#ifndef __RACE_CMD_FOTA_SRC_H__
#define __RACE_CMD_FOTA_SRC_H__

#ifdef AIR_FOTA_SRC_ENABLE
#include "race_cmd.h"
#include "race_cmd_fota.h"

#define APP_FOTA_SRC_RACE "[FOTA_SRC] "

/******************************************** FOTA race API********************************************/
// send 0x5A
RACE_ERRCODE RACE_CmdHandler_FOTA_get_version_req();
RACE_ERRCODE RACE_CmdHandler_FOTA_query_partition_info_req();
RACE_ERRCODE RACE_CmdHandler_FOTA_start_req();
RACE_ERRCODE RACE_CmdHandler_FOTA_get_4k_erase_status_req(uint32_t address, uint32_t length);
RACE_ERRCODE RACE_CmdHandler_FOTA_get_partition_sha256_req(uint32_t address, uint32_t length);
RACE_ERRCODE RACE_CmdHandler_FOTA_erase_partition_req(uint32_t address, uint32_t length);
RACE_ERRCODE RACE_CmdHandler_FOTA_write_page_req(uint32_t address, const uint8_t *pData);
RACE_ERRCODE RACE_CmdHandler_FOTA_check_integrity_req();
RACE_ERRCODE RACE_CmdHandler_FOTA_commit_req();

// recv 0x5B
void *RACE_CmdHandler_FOTA_get_version_recv_res(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id);
void *RACE_CmdHandler_FOTA_query_partition_info_recv_res(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id);
void *RACE_CmdHandler_FOTA_start_recv_res(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id);
void *RACE_CmdHandler_FOTA_get_4k_erase_status_recv_res(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id);
void *RACE_CmdHandler_FOTA_get_partition_sha256_recv_res(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id);
void *RACE_CmdHandler_FOTA_erase_partition_recv_res(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id);
void *RACE_CmdHandler_FOTA_write_page_recv_res(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id);
void *RACE_CmdHandler_FOTA_check_integrity_recv_res(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id);
void *RACE_CmdHandler_FOTA_commit_recv_res(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id);

// recv 0x5D
void *RACE_CmdHandler_FOTA_get_version_recv_noti(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id);
void *RACE_CmdHandler_FOTA_start_recv_noti(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id);
void *RACE_CmdHandler_FOTA_get_4k_erase_status_recv_noti(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id);
void *RACE_CmdHandler_FOTA_get_partition_sha256_recv_noti(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id);
void *RACE_CmdHandler_FOTA_erase_partition_recv_noti(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id);
void *RACE_CmdHandler_FOTA_check_integrity_recv_noti(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id);

/******************************************** App race API********************************************/
// send 0x5B
void *RACE_CmdHandler_FOTA_SRC_query_state_res(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id);
void *RACE_CmdHandler_FOTA_SRC_trigger_into_state_res(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id);

// send 0x5D
RACE_ERRCODE RACE_CmdHandler_FOTA_SRC_query_dongle_version_noti(const race_fota_get_version_noti_struct *data);
RACE_ERRCODE RACE_CmdHandler_FOTA_SRC_query_pkg_info_noti();
RACE_ERRCODE RACE_CmdHandler_FOTA_SRC_state_execute_result_noti();
RACE_ERRCODE RACE_CmdHandler_FOTA_SRC_transferring_info_noti(uint32_t prcessed_size, uint32_t total_size);

#endif

#endif
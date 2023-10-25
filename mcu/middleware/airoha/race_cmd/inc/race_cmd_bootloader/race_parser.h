/* Copyright Statement:
*
* (C) 2023  Airoha Technology Corp. All rights reserved.
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

#if defined(__EXT_BOOTLOADER__)


#ifndef RACE_PARSER_H
#define RACE_PARSER_H



#ifdef __cplusplus
extern "C"
{
#endif

#if defined(AIR_BL_DFU_ENABLE)


#include "race_cmd_bl_dfu.h"

#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif


#define RACE_PROTOCOL_CHANNEL                   (0x05)
#define RACE_PROTOCOL_TYPE_CMD                  (0x5A)
#define RACE_PROTOCOL_TYPE_RSP                  (0x5B)
#define RACE_PROTOCOL_TYPE_CMD_WITHOUT_RSP      (0x5C)
#define RACE_PROTOCOL_TYPE_NOTIFICATION         (0x5D)
#define RACE_PROTOCOL_LENGTH_MAX                (RACE_BL_DFU_RW_MAX_SIZE + 20) // DataAddress(4byte) + DataLength(2byte) + CRC32(4byte) + reserved(10byte)
#define RACE_PROTOCOL_LENGTH_MIN                (2)
#define RACE_PROTOCOL_MIN_SIZE                  (6)  // race_channel(1byte) + race_type(1byte) + race_length(2byte) + race_id(2byte)
#define RACE_PROTOCOL_HEADER_SIZE               (4)  // race_channel(1byte) + race_type(1byte) + race_length(2byte)
#define RACE_PROTOCOL_TOTAL_DATA_MAX_LENGTH     (RACE_PROTOCOL_LENGTH_MAX + RACE_PROTOCOL_MIN_SIZE)

typedef struct {
    uint8_t     pktId;
    uint8_t     type;
    uint16_t    length;
    uint16_t    id;
} PACKED race_common_hdr_t;


void race_lw_mux_event_handler(uint8_t event, void *msg);

#endif

#ifdef __cplusplus
}
#endif

#endif

#endif


/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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

#include "FreeRTOS.h"
#include "task.h"
//#include "types.h"
#include "syslog.h"
#include "audio_dump.h"
#include "string.h"
#include "hal_nvic.h"

/******************************************************************************
* Constant Definitions
******************************************************************************/
#define MAX_DUMP_SIZE 2048
#define DUMP_ID_SIZE  2
#define ACCUM_BYTES_SIZE    4
#define DUMP_BY_TUNING_TOOL 0

log_create_module(audiodump, PRINT_LEVEL_INFO);
#define LOGMSGIDE(fmt,arg...)   LOG_MSGID_E(audiodump, "audiodump: "fmt,##arg)

/******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t g_audio_dump_buffer[MAX_DUMP_SIZE];
DSP_DATADUMP_CTRL_STRU DSP_Dump_NvKey;
DumpIDs_AccumBytes DumpIDsAccumBytes[AUDIO_DUMP_CONFIG_MAX_NUM] = {0};

/******************************************************************************
 * Function Declaration
 ******************************************************************************/
void LOG_AUDIO_DUMP(uint8_t *audio, uint32_t audio_size, DSP_DATADUMP_MASK_BIT dumpID);


/******************************************************************************
 * Functions
 ******************************************************************************/
log_create_module(audio_module, PRINT_LEVEL_INFO);

static uint32_t get_accum_bytes(DSP_DATADUMP_MASK_BIT dumpID)
{
    uint32_t i;
    for(i = 0; i < AUDIO_DUMP_CONFIG_MAX_NUM; i++){
        if(DumpIDsAccumBytes[i].dump_id == dumpID){
            return DumpIDsAccumBytes[i].dump_accum_bytes;
        }
    }
    return 0;
}

static void increase_accum_bytes(DSP_DATADUMP_MASK_BIT dumpID, uint32_t bytes)
{
    uint32_t i;
    for(i = 0; i < AUDIO_DUMP_CONFIG_MAX_NUM; i++){
        if(DumpIDsAccumBytes[i].dump_id == dumpID){
            DumpIDsAccumBytes[i].dump_accum_bytes += bytes;
            return;
        }else if(DumpIDsAccumBytes[i].dump_id == 0){ //if not found,add to array
            DumpIDsAccumBytes[i].dump_id = dumpID;
            DumpIDsAccumBytes[i].dump_accum_bytes += bytes;
            return;
        }
    }
}

void LOG_AUDIO_DUMP(uint8_t *audio, uint32_t audio_size, DSP_DATADUMP_MASK_BIT dumpID)
{
    uint32_t left_size, curr_size, left_send_size, curr_send_size = 0;
    uint8_t *p_curr_audio;
    uint16_t dump_id = dumpID;
    left_size    = audio_size;
    p_curr_audio = audio;
    while(left_size) {
        if (left_size <= (MAX_DUMP_SIZE - DUMP_ID_SIZE - ACCUM_BYTES_SIZE)) {
            curr_size = left_size;
        } else {
            curr_size = MAX_DUMP_SIZE - DUMP_ID_SIZE - ACCUM_BYTES_SIZE;
        }
        increase_accum_bytes(dump_id,curr_size);
        uint32_t accum_bytes = get_accum_bytes(dump_id);
        uint8_t *audio_buffer_array[] = {(uint8_t *)&dump_id, (uint8_t *)&accum_bytes, p_curr_audio, NULL};
        uint32_t audio_buffer_length_array[] = {DUMP_ID_SIZE, ACCUM_BYTES_SIZE, curr_size};
        left_send_size = curr_size + DUMP_ID_SIZE + ACCUM_BYTES_SIZE;
        LOG_TLVDUMP_I(audio_module, LOG_TYPE_AUDIO_V2_DATA, audio_buffer_array, audio_buffer_length_array, curr_send_size);
        if (curr_send_size != left_send_size) {
            LOGMSGIDE("[Audio Dump] droped, total %d, sent %d", 2, left_send_size, curr_send_size);
            increase_accum_bytes(dump_id, left_size - curr_send_size);
            return;
        }
        p_curr_audio += curr_size;
        left_size -= curr_size;
    }
}


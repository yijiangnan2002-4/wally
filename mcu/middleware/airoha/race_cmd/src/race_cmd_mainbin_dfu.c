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

#if defined(AIR_BL_DFU_ENABLE)

#include "race_cmd.h"
#include "race_xport.h"

#include "FreeRTOS.h"
#include "timers.h"
#include "hal_wdt.h"

#include "race_cmd_mainbin_dfu.h"
#include "crc_dfu.h"

#include "dfu_util.h"


#define RACE_ERROR_SUCCESS                      (0)
#define RACE_ERROR_CRC_FAIL                     (1)
#define RACE_ERROR_DFU_NOT_START                (2)
#define RACE_ERROR_FLASH_FAIL                   (3)
#define RACE_ERROR_PARAM_INVALID                (4)
#define RACE_ERROR_DATA_CRC_FAIL                (5)
#define RACE_ERROR_ALREADY_MAIN_BIN             (6)

#define RACE_FLAG_START_DFU                     (1)
#define RACE_FLAG_TRIGGER_WDT_RESET             (0)
#define RACE_FLAG_JUMP_MAIN_BIN                 (1)

#define RACE_CURRENT_BOOTLOADER_BIN             (1)
#define RACE_CURRENT_MAIN_BIN                   (2)

#define RACE_WDT_RESET_DELAY_TIME               (1200)

#define RACE_BL_DFU_START                       (0x211B)
#define RACE_BL_DFU_RESET                       (0x211C)

typedef struct {
    RACE_COMMON_HDR_STRU hdr;
    uint8_t flag;
    uint32_t crc;
} PACKED race_dfu_mb_start_cmd_t;

typedef struct {
    uint8_t status;
    uint8_t curr_bin;
} PACKED race_dfu_mb_start_rsp_t;

typedef struct {
    uint8_t status;
} PACKED race_dfu_mb_reset_rsp_t;

static TimerHandle_t g_reset_timer = NULL;


static void race_mb_dfu_wdt_timer_callback(TimerHandle_t hdl)
{
    hal_wdt_software_reset();
}

static void race_mb_dfu_start_wdt_timer(void)
{
    if (NULL == g_reset_timer) {
        g_reset_timer = xTimerCreate("RACE_MB_DFU_WDT", (RACE_WDT_RESET_DELAY_TIME / portTICK_PERIOD_MS),
            pdFALSE, NULL, race_mb_dfu_wdt_timer_callback);
    }
    if (NULL == g_reset_timer) {
        return ;
    }
    xTimerStart(g_reset_timer, 0);
}

void *race_cmd_handler_mb_dfu(ptr_race_pkt_t pcmd, uint16_t length, uint8_t channel_id)
{
    race_dfu_mb_start_rsp_t *pEvt = NULL;
    race_dfu_mb_start_cmd_t *cmd = (race_dfu_mb_start_cmd_t *)pcmd;
    uint32_t crc = 0;
    uint32_t crc_len = 0;
    uint8_t *crc_data = (uint8_t *)pcmd;
    if (NULL == pcmd) {
        return NULL;
    }

    crc_len = pcmd->hdr.length + 2 + 2 - 4; // +2: race_channel + race_type, +2: race_length, -4: crc32
    crc = crc32_calculate(&crc_data[0], crc_len);

    switch (cmd->hdr.id) {
        case RACE_BL_DFU_START: {
            pEvt = (race_dfu_mb_start_rsp_t *)RACE_ClaimPacket(RACE_TYPE_RESPONSE, pcmd->hdr.id, sizeof(race_dfu_mb_start_rsp_t), channel_id);
            pEvt->curr_bin = RACE_CURRENT_MAIN_BIN;
            break;
        }

        case RACE_BL_DFU_RESET: {
            pEvt = (race_dfu_mb_start_rsp_t *)RACE_ClaimPacket(RACE_TYPE_RESPONSE, pcmd->hdr.id, sizeof(race_dfu_mb_reset_rsp_t), channel_id);
            break;
        }

        default: {
            RACE_LOG_MSGID_E("[DFU] unknown cmd", 0);
            return NULL;
        }
    }
    if (crc != cmd->crc) {
        pEvt->status = RACE_ERROR_CRC_FAIL;
    } else if (cmd->hdr.id == RACE_ID_MB_DFU_START) {
        pEvt->status = RACE_ERROR_SUCCESS;
        if (cmd->flag == RACE_FLAG_START_DFU) {
            dfu_flag_set();  // set dfu flag in flash
            RACE_LOG_MSGID_I("[DFU] set dfu flag", 0);
        }
    } else if (cmd->hdr.id == RACE_ID_MB_DFU_RESET) {
        if (cmd->flag == RACE_FLAG_TRIGGER_WDT_RESET) {
            pEvt->status = RACE_ERROR_SUCCESS;
            race_mb_dfu_start_wdt_timer();
            RACE_LOG_MSGID_I("[DFU] trigger wdt reset...", 0);
        } else if (cmd->flag == RACE_FLAG_JUMP_MAIN_BIN) {
            pEvt->status = RACE_ERROR_ALREADY_MAIN_BIN;
        } else {
            pEvt->status = RACE_ERROR_PARAM_INVALID;
        }
    }

    return pEvt;
}

#endif


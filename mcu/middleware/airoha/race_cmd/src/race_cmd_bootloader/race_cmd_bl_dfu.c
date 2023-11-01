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

#if defined(AIR_BL_DFU_ENABLE)

#include "lw_mux.h"
#include "race_cmd_bl_dfu.h"
#include "race_parser.h"
#include "race_handler.h"

#include "hal_flash.h"
#include "hal_flash_disk_internal.h"
#include "hal_wdt.h"

#include "crc_dfu.h"

#include "bl_common.h"
#include "dfu_util.h"

#if defined(AIR_BL_USB_HID_DFU_ENABLE)
#include "bl_usb_hid.h"
#endif


/**************************************************************************************************
* Define
**************************************************************************************************/

#define RACE_ERROR_SUCCESS                      (0)
#define RACE_ERROR_CRC_FAIL                     (1)
#define RACE_ERROR_DFU_NOT_START                (2)
#define RACE_ERROR_FLASH_FAIL                   (3)
#define RACE_ERROR_PARAM_INVALID                (4)
#define RACE_ERROR_DATA_CRC_FAIL                (5)
#define RACE_ERROR_ALREADY_MAIN_BIN             (6)

#define CONVERT_ADDRESS_TO_LOGIC(addr)          ((addr) & (~(INT_RetrieveFlashBaseAddr())))

#define RACE_FLASH_WRITE(addr,data,len)         hal_flash_write(CONVERT_ADDRESS_TO_LOGIC(addr),(data),(len))
#define RACE_FLASH_READ(addr,data,len)          hal_flash_read(CONVERT_ADDRESS_TO_LOGIC(addr),(data),(len))
#define RACE_FLASH_ERASE(addr,block)            hal_flash_erase(CONVERT_ADDRESS_TO_LOGIC(addr),(block))

#define RACE_DA_WRITE_BYTES_RSP_LENGTH          (6 + 1 + 4)
#define RACE_DA_READ_BYTES_RSP_FIX_LENGTH       (6 + 1 + 4 + 4) // race_header(6) + status(1) + data_address(4) + crc32(4)
#define RACE_DA_ERASE_BYTES_RSP_LENGTH          (6 + 1 + 4)
#define RACE_DA_ERASE_BYTES_NOTIFY_LENGTH       (6 + 1 + 4 + 4)
#define RACE_DA_DATA_RANGE_CRC_RSP_LENGTH       (6 + 1 + 4)
#define RACE_DFU_START_RSP_LENGTH               (6 + 2)
#define RACE_DFU_RESET_RSP_LENGTH               (6 + 1)
#define RACE_DA_GET_FLASH_ADDRESS_RSP_LENGTH    (6 + 1 + 4)
#define RACE_DA_GET_FLASH_SIZE_RSP_LENGTH       (6 + 1 + 4)
#define RACE_DA_GET_FLASH_ID_RSP_LENGTH         (6 + 1 + 3)

#define RACE_FLAG_START_DFU                     (1)
#define RACE_FLAG_TRIGGER_WDT_RESET             (0)
#define RACE_FLAG_JUMP_MAIN_BIN                 (1)

#define RACE_CURRENT_BOOTLOADER_BIN             (1)
#define RACE_CURRENT_MAIN_BIN                   (2)

#define RACE_ERASE_LEN_4K                       (0x1000)
#define RACE_ERASE_LEN_32K                      (0x8000)
#define RACE_ERASE_LEN_64K                      (0x10000)


/**************************************************************************************************
* Structure
**************************************************************************************************/

typedef struct {
    bool is_dfu_started;
} race_bl_dfu_ctrl_t;

typedef struct {
    race_common_hdr_t *header;
    uint32_t data_address;
    uint16_t data_length;
    uint8_t *data;
    uint32_t crc;
} race_da_write_bytes_cmd_t;

typedef struct {
    race_common_hdr_t header;
    uint32_t data_address;
    uint16_t data_length;
    uint32_t crc;
} PACKED race_da_read_bytes_cmd_t;

typedef struct {
    race_common_hdr_t header;
    uint32_t data_address;
    uint32_t data_length;
    uint32_t crc;
} PACKED race_da_erase_bytes_cmd_t;

typedef struct {
    race_common_hdr_t header;
    uint32_t data_address;
    uint32_t data_length;
    uint32_t data_crc;
    uint32_t crc;
} PACKED race_da_data_range_crc_cmd_t;

typedef struct {
    race_common_hdr_t header;
    uint8_t flag;
    uint32_t crc;
} PACKED race_dfu_start_cmd_t;

typedef race_dfu_start_cmd_t race_dfu_reset_cmd_t;

typedef struct {
    race_common_hdr_t header;
    uint8_t status;
    uint32_t data_address;
} PACKED race_da_write_bytes_rsp_t;

typedef struct {
    race_common_hdr_t header;
    uint8_t status;
    uint32_t data_address;
    uint32_t crc;
    uint8_t data[0];
} PACKED race_da_read_bytes_rsp_t;

typedef struct {
    race_common_hdr_t header;
    uint8_t status;
    uint32_t data_address;
} PACKED race_da_erase_bytes_rsp_t;

typedef struct {
    race_common_hdr_t header;
    uint8_t status;
    uint32_t data_crc;
} PACKED race_da_data_range_crc_rsp_t;

typedef struct {
    race_common_hdr_t header;
    uint8_t status;
    uint8_t curr_bin;
} PACKED race_dfu_start_rsp_t;

typedef race_dfu_start_rsp_t race_dfu_reset_rsp_t;

typedef struct {
    race_common_hdr_t header;
    uint8_t status;
    uint32_t flash_addr;
} PACKED race_da_get_flash_addr_rsp_t;

typedef struct {
    race_common_hdr_t header;
    uint8_t status;
    uint32_t flash_size;
} PACKED race_da_get_flash_size_rsp_t;

typedef struct {
    race_common_hdr_t header;
    uint8_t status;
    uint8_t flash_id[3];
} PACKED race_da_get_flash_id_rsp_t;

static race_bl_dfu_ctrl_t g_race_bl_dfu_ctrl = {
    .is_dfu_started = false,
};


/**************************************************************************************************
* Static Variable
**************************************************************************************************/

static uint8_t g_race_rsp_buf[RACE_PROTOCOL_TOTAL_DATA_MAX_LENGTH];


/**************************************************************************************************
* Static Functions
**************************************************************************************************/

static void race_log_crc_fail(uint32_t data_crc, uint32_t real_crc)
{
    bl_print(LOG_DEBUG, "CRC:fail,%x != %x\r\n", data_crc, real_crc);
}

static uint32_t race_calculate_cmd_crc32(uint8_t *pcmd)
{
    race_common_hdr_t *hdr = (race_common_hdr_t *)pcmd;
    uint32_t i;
    uint32_t crc = 0;
    uint32_t crc_len = hdr->length + 2 + 2 - 4; // +2: race_channel + race_type, +2: race_length, -4: crc32
    uint32_t log_len = crc_len > 10 ? 10 : crc_len;

    crc = crc32_calculate(&pcmd[0], crc_len);
    bl_print(LOG_DEBUG, "CRC:len[%d]: ", crc_len);
    for (i = 0; i < log_len; i++) {
        bl_print(LOG_DEBUG, "%x ", pcmd[i]);
    }
    if (log_len < crc_len) {
        bl_print(LOG_DEBUG, "...");
    }
    bl_print(LOG_DEBUG, "\r\nCRC:res=%x\r\n", crc);
    return crc;
}

static bool race_calculate_flash_crc32(uint32_t *crc32_res, uint32_t address, uint32_t length)
{
    uint32_t crc_res;
    uint8_t *r_buf = &g_race_rsp_buf[0];
    uint32_t r_max_size = RACE_BL_DFU_RW_MAX_SIZE;
    uint32_t r_size;
    uint32_t r_offset = 0;
    bool is_end = false;
    bool is_start = true;

    while (1) {
        r_size = length - r_offset > r_max_size ? r_max_size : length - r_offset;
        is_end = r_offset + r_size >= length ? true : false;
        is_start = r_offset > 0 ? false : true;
        if (RACE_FLASH_READ(address + r_offset, r_buf, r_size) != HAL_FLASH_STATUS_OK) {
            return false;
        }
        crc_res = crc32_calculate_section(r_buf, r_size, crc_res, is_start, is_end);
        //bl_print(LOG_DEBUG, "race_calculate_flash_crc32, address:%x, length:%d, crc:%x, is_start:%d, is_end:%d\r\n",
        //    address + r_offset, r_size, crc_res, is_start, is_end);
        if (is_end) {
            break;
        }
        r_offset += r_size;
    }
    *crc32_res = crc_res;
    return true;
}

static bool race_dfu_is_started(void)
{
    return g_race_bl_dfu_ctrl.is_dfu_started;
}

static void race_dfu_start(void)
{
    g_race_bl_dfu_ctrl.is_dfu_started = true;
}

static void race_dfu_clear_start_flag(void)
{
    g_race_bl_dfu_ctrl.is_dfu_started = false;
}

static void race_full_header(race_common_hdr_t *header, uint8_t race_type, uint16_t length, uint16_t race_id)
{
    header->pktId = RACE_PROTOCOL_CHANNEL;
    header->type = race_type;
    header->length = length;
    header->id = race_id;
}

/**************************************************************************************************
* Public Functions
**************************************************************************************************/

void race_cmd_da_write_bytes(uint8_t *pcmd, uint32_t cmd_len, uint8_t port)
{
    race_da_write_bytes_cmd_t cmd;
    race_da_write_bytes_rsp_t *rsp = (race_da_write_bytes_rsp_t *)&g_race_rsp_buf[0];
    uint32_t offset;
    uint32_t crc = 0;

    offset = RACE_PROTOCOL_MIN_SIZE;
    cmd.header = (race_common_hdr_t *)pcmd;
    cmd.data_address = race_combo_u32(&pcmd[offset]);
    offset += 4;
    cmd.data_length = race_combo_u16(&pcmd[offset]);
    offset += 2;
    cmd.data = &pcmd[offset];
    offset += cmd.data_length;
    cmd.crc = race_combo_u32(&pcmd[offset]);

    race_full_header(&rsp->header, RACE_PROTOCOL_TYPE_RSP, RACE_DA_WRITE_BYTES_RSP_LENGTH - RACE_PROTOCOL_HEADER_SIZE, RACE_ID_DA_WRITE_BYTES);
    rsp->data_address = cmd.data_address;

    crc = race_calculate_cmd_crc32(pcmd);
    if (crc != cmd.crc) {
        rsp->status = RACE_ERROR_CRC_FAIL;
        race_log_crc_fail(cmd.crc, crc);
    } else if (false == race_dfu_is_started()) {
        rsp->status = RACE_ERROR_DFU_NOT_START;
    } else {
        bl_print(LOG_DEBUG, "[W\r\n");
        hal_flash_status_t flash_res = RACE_FLASH_WRITE(cmd.data_address, cmd.data, cmd.data_length);
        bl_print(LOG_DEBUG, "W]\r\n");
        if (flash_res == HAL_FLASH_STATUS_OK) {
            rsp->status = RACE_ERROR_SUCCESS;
        } else {
            rsp->status = RACE_ERROR_FLASH_FAIL;
            bl_print(LOG_DEBUG, "flash write error:%d\r\n", flash_res);
        }
    }

    lw_mux_tx((uint8_t *)rsp, RACE_DA_WRITE_BYTES_RSP_LENGTH, port);
}


void race_cmd_da_read_bytes(uint8_t *pcmd, uint32_t cmd_len, uint8_t port)
{
    race_da_read_bytes_cmd_t *cmd = (race_da_read_bytes_cmd_t *)pcmd;
    race_da_read_bytes_rsp_t *rsp = (race_da_read_bytes_rsp_t *)&g_race_rsp_buf[0];
    uint32_t crc = 0;
    uint32_t rsp_len = 0;

    race_full_header(&rsp->header, RACE_PROTOCOL_TYPE_RSP, RACE_DA_READ_BYTES_RSP_FIX_LENGTH - RACE_PROTOCOL_HEADER_SIZE, RACE_ID_DA_READ_BYTES);
    rsp->data_address = cmd->data_address;
    rsp->crc = 0;
    rsp_len = RACE_DA_READ_BYTES_RSP_FIX_LENGTH;

    crc = race_calculate_cmd_crc32(pcmd);
    if (crc != cmd->crc) {
        rsp->status = RACE_ERROR_CRC_FAIL;
        race_log_crc_fail(cmd->crc, crc);
    } else if (false == race_dfu_is_started()) {
        rsp->status = RACE_ERROR_DFU_NOT_START;
    } else if (0 >= cmd->data_length || RACE_BL_DFU_RW_MAX_SIZE < cmd->data_length) {
        rsp->status = RACE_ERROR_PARAM_INVALID;
    } else {
        hal_flash_status_t flash_res = RACE_FLASH_READ(cmd->data_address, &rsp->data[0], cmd->data_length);
        if (flash_res == HAL_FLASH_STATUS_OK) {
            rsp->status = RACE_ERROR_SUCCESS;
            rsp->crc = crc32_calculate(&rsp->data[0], cmd->data_length);
            rsp_len += cmd->data_length;
            rsp->header.length = rsp_len - RACE_PROTOCOL_HEADER_SIZE;
        } else {
            rsp->status = RACE_ERROR_FLASH_FAIL;
            bl_print(LOG_DEBUG, "flash read error:%d\r\n", flash_res);
        }
    }

    lw_mux_tx((uint8_t *)rsp, rsp_len, port);
}


void race_cmd_da_erase_bytes(uint8_t *pcmd, uint32_t cmd_len, uint8_t port)
{
    race_da_erase_bytes_cmd_t *cmd = (race_da_erase_bytes_cmd_t *)pcmd;
    race_da_erase_bytes_rsp_t *rsp = (race_da_erase_bytes_rsp_t *)&g_race_rsp_buf[0];
    uint32_t crc = 0;
    hal_flash_block_t block;

    race_full_header(&rsp->header, RACE_PROTOCOL_TYPE_RSP, RACE_DA_ERASE_BYTES_RSP_LENGTH - RACE_PROTOCOL_HEADER_SIZE, RACE_ID_DA_ERASE_BYTES);
    rsp->data_address = cmd->data_address;

    crc = race_calculate_cmd_crc32(pcmd);
    if (crc != cmd->crc) {
        rsp->status = RACE_ERROR_CRC_FAIL;
        race_log_crc_fail(cmd->crc, crc);
    } else {
        if (false == race_dfu_is_started()) {
            rsp->status = RACE_ERROR_DFU_NOT_START;
        } else {
            rsp->status = RACE_ERROR_SUCCESS;
        }
    }

    if (rsp->status != RACE_ERROR_SUCCESS) {
        lw_mux_tx((uint8_t *)rsp, RACE_DA_ERASE_BYTES_RSP_LENGTH, port);
        return ;
    }

    if (cmd->data_length == RACE_ERASE_LEN_4K) {
        block = HAL_FLASH_BLOCK_4K;
    } else if (cmd->data_length == RACE_ERASE_LEN_32K) {
        block = HAL_FLASH_BLOCK_32K;
    } else if (cmd->data_length == RACE_ERASE_LEN_64K) {
        block = HAL_FLASH_BLOCK_64K;
    } else {
        rsp->status = RACE_ERROR_PARAM_INVALID;
    }
    if (rsp->status == RACE_ERROR_SUCCESS) {
        bl_print(LOG_DEBUG, "[E\r\n");
        hal_flash_status_t flash_res = RACE_FLASH_ERASE(cmd->data_address, block);
        bl_print(LOG_DEBUG, "E]\r\n");
        if (flash_res != HAL_FLASH_STATUS_OK) {
            rsp->status = RACE_ERROR_FLASH_FAIL;
            bl_print(LOG_DEBUG, "flash erase error:%d\r\n", flash_res);
        }
    }

    lw_mux_tx((uint8_t *)rsp, RACE_DA_ERASE_BYTES_RSP_LENGTH, port);
}


void race_cmd_da_data_range_crc(uint8_t *pcmd, uint32_t cmd_len, uint8_t port)
{
    race_da_data_range_crc_cmd_t *cmd = (race_da_data_range_crc_cmd_t *)pcmd;
    race_da_data_range_crc_rsp_t rsp;
    uint32_t crc = 0;

    race_full_header(&rsp.header, RACE_PROTOCOL_TYPE_RSP, RACE_DA_DATA_RANGE_CRC_RSP_LENGTH - RACE_PROTOCOL_HEADER_SIZE, RACE_ID_DA_DATA_RANGE_CRC);
    rsp.status = RACE_ERROR_SUCCESS;

    crc = race_calculate_cmd_crc32(pcmd);

    if (crc != cmd->crc) {
        rsp.status = RACE_ERROR_CRC_FAIL;
        race_log_crc_fail(cmd->crc, crc);
    } else if (false == race_dfu_is_started()) {
        rsp.status = RACE_ERROR_DFU_NOT_START;
    } else if (0 >= cmd->data_length) {
        rsp.status = RACE_ERROR_PARAM_INVALID;
    } else {
        if (true == race_calculate_flash_crc32(&crc, cmd->data_address, cmd->data_length)) {
            rsp.status = RACE_ERROR_SUCCESS;
            rsp.data_crc = crc;
            if (rsp.data_crc != cmd->data_crc) {
                rsp.status = RACE_ERROR_DATA_CRC_FAIL;
            }
        } else {
            rsp.status = RACE_ERROR_FLASH_FAIL;
        }
    }

    lw_mux_tx((uint8_t *)&rsp, RACE_DA_DATA_RANGE_CRC_RSP_LENGTH, port);
}


void race_cmd_dfu_start(uint8_t *pcmd, uint32_t cmd_len, uint8_t port)
{
    race_dfu_start_cmd_t *cmd = (race_dfu_start_cmd_t *)pcmd;
    race_dfu_start_rsp_t *rsp = (race_dfu_start_rsp_t *)&g_race_rsp_buf[0];
    uint32_t crc = 0;

    race_full_header(&rsp->header, RACE_PROTOCOL_TYPE_RSP, RACE_DFU_START_RSP_LENGTH - RACE_PROTOCOL_HEADER_SIZE, RACE_ID_DFU_START);
    crc = race_calculate_cmd_crc32(pcmd);
    if (crc != cmd->crc) {
        race_log_crc_fail(cmd->crc, crc);
        rsp->status = RACE_ERROR_CRC_FAIL;
    } else {
        rsp->status = RACE_ERROR_SUCCESS;
        if (cmd->flag == RACE_FLAG_START_DFU) {
            race_dfu_start();
        }
    }
    rsp->curr_bin = RACE_CURRENT_BOOTLOADER_BIN;

    lw_mux_tx((uint8_t *)rsp, RACE_DFU_START_RSP_LENGTH, port);
}


extern void bl_start_user_code();

void race_cmd_dfu_reset(uint8_t *pcmd, uint32_t cmd_len, uint8_t port)
{
    race_dfu_reset_cmd_t *cmd = (race_dfu_reset_cmd_t *)pcmd;
    race_dfu_reset_rsp_t *rsp = (race_dfu_reset_rsp_t *)&g_race_rsp_buf[0];
    uint32_t crc = 0;

    race_full_header(&rsp->header, RACE_PROTOCOL_TYPE_RSP, RACE_DFU_RESET_RSP_LENGTH - RACE_PROTOCOL_HEADER_SIZE, RACE_ID_DFU_RESET);
    crc = race_calculate_cmd_crc32(pcmd);
    if (crc != cmd->crc) {
        race_log_crc_fail(cmd->crc, crc);
        rsp->status = RACE_ERROR_CRC_FAIL;
    } else if (false == race_dfu_is_started()) {
        rsp->status = RACE_ERROR_DFU_NOT_START;
    } else {
        rsp->status = RACE_ERROR_SUCCESS;
        if (cmd->flag == RACE_FLAG_TRIGGER_WDT_RESET) {
            race_dfu_clear_start_flag();
            dfu_flag_clear();
            lw_mux_tx((uint8_t *)&rsp, RACE_DFU_RESET_RSP_LENGTH, port);
            // todo: should we delay some micro seconds?
            hal_wdt_software_reset();
        } else if (cmd->flag == RACE_FLAG_JUMP_MAIN_BIN) {
            race_dfu_clear_start_flag();
            dfu_flag_clear();
            lw_mux_tx((uint8_t *)&rsp, RACE_DFU_RESET_RSP_LENGTH, port);
            // todo: should we delay some micro seconds to wait race response send success?
#if defined(AIR_BL_USB_HID_DFU_ENABLE)
            if (bl_usb_hid_is_ready() == true) {
                bl_usb_hid_deinit();
            }
#endif
            bl_start_user_code();
        } else {
            rsp->status = RACE_ERROR_PARAM_INVALID;
        }
    }

    lw_mux_tx((uint8_t *)rsp, RACE_DFU_RESET_RSP_LENGTH, port);
}


extern uint32_t INT_RetrieveFlashBaseAddr(void);
extern hal_flash_status_t hal_flash_get_disk_size(uint32_t *size);
extern void NOR_ReadID(const uint16_t CS, volatile uint16_t *BaseAddr, uint16_t *flashid);

void race_da_get_flash_addr(uint8_t *pcmd, uint32_t cmd_len, uint8_t port)
{
    race_da_get_flash_addr_rsp_t *rsp = (race_da_get_flash_addr_rsp_t *)&g_race_rsp_buf[0];
    race_full_header(&rsp->header, RACE_PROTOCOL_TYPE_RSP,
        RACE_DA_GET_FLASH_ADDRESS_RSP_LENGTH - RACE_PROTOCOL_HEADER_SIZE, RACE_ID_DA_GET_FLASH_ADDRESS);
    rsp->status = RACE_ERROR_SUCCESS;
    rsp->flash_addr = INT_RetrieveFlashBaseAddr();
    lw_mux_tx((uint8_t *)rsp, RACE_DA_GET_FLASH_ADDRESS_RSP_LENGTH, port);
}

void race_da_get_flash_size(uint8_t *pcmd, uint32_t cmd_len, uint8_t port)
{
    hal_flash_status_t res;
    race_da_get_flash_size_rsp_t *rsp = (race_da_get_flash_size_rsp_t *)&g_race_rsp_buf[0];
    race_full_header(&rsp->header, RACE_PROTOCOL_TYPE_RSP,
        RACE_DA_GET_FLASH_SIZE_RSP_LENGTH - RACE_PROTOCOL_HEADER_SIZE, RACE_ID_DA_GET_FLASH_SIZE);
    res = hal_flash_get_disk_size(&rsp->flash_size);
    if (HAL_FLASH_STATUS_OK == res) {
        rsp->status = RACE_ERROR_SUCCESS;
    } else {
        rsp->status = RACE_ERROR_FLASH_FAIL;
    }
    lw_mux_tx((uint8_t *)rsp, RACE_DA_GET_FLASH_SIZE_RSP_LENGTH, port);
}

void race_da_get_flash_id(uint8_t *pcmd, uint32_t cmd_len, uint8_t port)
{
    race_da_get_flash_id_rsp_t *rsp = (race_da_get_flash_id_rsp_t *)&g_race_rsp_buf[0];
    uint16_t flash_id[3] = {0};
    uint16_t i;
    NOR_ReadID(0, 0, flash_id);
    race_full_header(&rsp->header, RACE_PROTOCOL_TYPE_RSP,
        RACE_DA_GET_FLASH_ID_RSP_LENGTH - RACE_PROTOCOL_HEADER_SIZE, RACE_ID_DA_GET_FLASH_ID);
    rsp->status = RACE_ERROR_SUCCESS;
    for (i = 0; i < 3; i++) {
        rsp->flash_id[i] = flash_id[i];
    }
    lw_mux_tx((uint8_t *)rsp, RACE_DA_GET_FLASH_ID_RSP_LENGTH, port);
}


#endif

#endif


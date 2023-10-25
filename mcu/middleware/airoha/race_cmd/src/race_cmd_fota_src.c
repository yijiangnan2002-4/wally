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



#ifdef AIR_FOTA_SRC_ENABLE
//#error AIR_FOTA_SRC_ENABLE
#include "race_cmd_fota_src.h"
#include "race_xport.h"
#include "race_util.h"
#include "race_cmd_storage.h"
#include "race_cmd_bluetooth.h"
#include "fota_flash.h"
#include "apps_debug.h"
#include "race_noti.h"

#include "fota_util.h"
//#include "fota_flash.h"

//#define HEADSET_CONTROL_ON
#define FOTA_RESUME_MARKED

#define RACE_RES_OK (0)
#define FOTA_WRITE_BUFFER_UNIT (256)
#define FOTA_WRITE_PAGE_MAX_COUNT (3)
#define FOTA_4K_BLOCK_SIZE (4096)

/* For 2831, please check the FOTA begin address: 0x086C5000 */
#define HOST_FOTA_PATITION_ADDRESS (0x84B2000)
/* Using ULL2.0 for dongle FOTA */
#define FOTA_SRC_CHANNEL_ID (RACE_SERIAL_PORT_TYPE_BLE)

static uint8_t s_app_interaction_header_id = 0;
static uint8_t s_app_interaction_channel_id = 0;

static uint8_t *s_4k_erase_status_bit_buffer = NULL;
static uint32_t s_bit_buffer_byte_len = 0;
//
static uint32_t g_fota_partition_start_address = 0;
//
static uint32_t g_fota_pkg_length = 0;
static uint32_t g_fota_dut_partition_address = 0;
static uint32_t g_fota_dut_partition_length = 0;
static uint32_t g_fota_start_erase_address = 0;
static uint8_t g_fota_write_page_data[FOTA_WRITE_BUFFER_UNIT*FOTA_WRITE_PAGE_MAX_COUNT] = {0};



typedef enum {
    FOTA_IDLE_STATE_,
    FOTA_QUERY_STATE_,
    FOTA_TRANSFERRING_STATE_,
    FOTA_COMMIT_STATE_,
}FOTA_STATE_EN;

static FOTA_STATE_EN en_fota_state = FOTA_IDLE_STATE_;

/******************************************** FOTA util API********************************************/
static uint32_t FOTA_get_pkg_length()
{
    fota_integrity_check_type_enum integrity_check_type = FOTA_INTEGRITY_CHECK_TYPE_MAX;
    uint32_t signature_start_address = 0, data_start_address = 0;
    uint32_t data_length = 0;
    FotaStorageType storage_type = Invalid;

    FOTA_ERRCODE ret = fota_get_integrity_check_info(&integrity_check_type,
                                    &signature_start_address,
                                    &data_start_address,
                                    &data_length,
                                    &storage_type);
    if (FOTA_ERRCODE_SUCCESS != ret) {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"FOTA_get_pkg_length fota_flash_read fail: ret(%d)", 1, ret);
        return 0;
    }
    g_fota_partition_start_address = signature_start_address;
    data_length = data_length + SIGNATURE_SIZE ;
    RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"start_address(0x%x) read_fota_partition_length(%d)", 2,g_fota_partition_start_address, data_length);
    
    return data_length;
}

static uint8_t least_bit_pos(const uint8_t byte_value)
{
    return (byte_value & 1) ? 1 : (least_bit_pos(byte_value >> 1) + 1);
}

static uint8_t first_bit_pos(uint8_t byte_value)
{
    return 8 - (least_bit_pos(byte_value) - 1);
}

static uint32_t total_bit_pos(uint8_t byte_value, uint32_t pre_byte_count)
{
    return 8 * pre_byte_count + first_bit_pos(byte_value);
}

static uint32_t FOTA_get_4K_erase_bit_pos(const uint8_t *buf, const uint32_t len)
{
    uint32_t i = 0;
    for (i = len-1; i > 0; i--) {
        if (buf[i] != 0) {
            return total_bit_pos(buf[i], i);
        }
    }
    return 0;
}

static bool FOTA_get_partition_sha256_process(uint32_t address)
{
    if (address >= g_fota_start_erase_address) {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"FOTA_get_partition_sha256_process cur address(0x%X) is larger than start erase address(0x%X)", 2, address, g_fota_start_erase_address);
        return false;
    }

    #define GET_SHA256_UNIT (256*1024)

    uint32_t unit = GET_SHA256_UNIT;
    const uint32_t length = g_fota_start_erase_address - address;
    if (length < GET_SHA256_UNIT) {
        unit = length;
    }

    return RACE_CmdHandler_FOTA_get_partition_sha256_req(address, unit);
}

static bool FOTA_get_4k_erase_status_process(uint32_t address)
{
    #define GET_4K_ERASE_STATUS_UNIT (512*1024)

    uint32_t unit = GET_4K_ERASE_STATUS_UNIT;
    const uint32_t length = g_fota_dut_partition_address + g_fota_pkg_length - address;
    if (length < GET_4K_ERASE_STATUS_UNIT) {
        unit = length;
    }

    return RACE_CmdHandler_FOTA_get_4k_erase_status_req(address, unit);
}

static bool FOTA_write_page_process(uint32_t address)
{
    if (address < g_fota_dut_partition_address 
        || address > g_fota_dut_partition_address + g_fota_pkg_length
        || g_fota_partition_start_address <=0 ) {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"FOTA_write_page_process failed: address(0x%X), start(0x%X), length(0x%X),partition_start(0x%X)"
        , 4
        , address, g_fota_dut_partition_address, g_fota_pkg_length,g_fota_partition_start_address);
        return false;
    }

    memset(g_fota_write_page_data, 0, sizeof(g_fota_write_page_data));

    const uint32_t host_flash_address = g_fota_partition_start_address + (address - g_fota_dut_partition_address);
    
    FOTA_ERRCODE ret = fota_flash_read(host_flash_address, g_fota_write_page_data, sizeof(g_fota_write_page_data), true);
    RACE_LOG_MSGID_I(APP_FOTA_SRC_RACE"FOTA_write_page_process : write(%d)bytes, total(0d)"
    , 2
    , (address - g_fota_dut_partition_address), g_fota_pkg_length);
    if (FOTA_ERRCODE_SUCCESS != ret) {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"FOTA_write_page_process fota_flash_read fail: ret(%d), address(0x%X)", 2, ret, host_flash_address);
        return false;
    }

    return RACE_CmdHandler_FOTA_write_page_req(address, g_fota_write_page_data);
}

/******************************************** FOTA SRC race req API********************************************/
RACE_ERRCODE RACE_CmdHandler_FOTA_get_version_req()
{
    typedef struct {
        uint8_t role; /*0x00: agent; 0x01: partner*/
    } PACKED CMD;

    RACE_ERRCODE ret = RACE_ERRCODE_SUCCESS;

    CMD* pCmd = RACE_ClaimPacketAppID(RACE_APP_ID_NONE,
        RACE_TYPE_COMMAND,
        RACE_FOTA_GET_VERSION,
        sizeof(CMD),
        FOTA_SRC_CHANNEL_ID);
    if (pCmd) {
        pCmd->role = 0x00;
        ret = race_flush_packet((uint8_t*)pCmd, FOTA_SRC_CHANNEL_ID);
    } else {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_get_version_req RACE_ClaimPacketAppID fail", 0);
        ret = RACE_ERRCODE_NOT_ENOUGH_MEMORY;
    }

    return ret;
}

RACE_ERRCODE RACE_CmdHandler_FOTA_query_partition_info_req()
{
    typedef struct {
        uint8_t partition_id; /*0x00: FOTA partition; 0x01: ROFS partition*/
    } PACKED CMD;

    RACE_ERRCODE ret = RACE_ERRCODE_SUCCESS;

    CMD* pCmd = RACE_ClaimPacketAppID(RACE_APP_ID_NONE,
        RACE_TYPE_COMMAND,
        RACE_FOTA_QUERY_PARTITION_INFO,
        sizeof(CMD),
        FOTA_SRC_CHANNEL_ID);
    if (pCmd) {
        pCmd->partition_id = 0x00;
        ret = race_flush_packet((uint8_t*)pCmd, FOTA_SRC_CHANNEL_ID);
    } else {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_query_partition_info_req RACE_ClaimPacketAppID fail", 0);
        ret = RACE_ERRCODE_NOT_ENOUGH_MEMORY;
    }

    return ret;
}

RACE_ERRCODE RACE_CmdHandler_FOTA_start_req()
{
    typedef struct {
        uint8_t recipient; /*0x01: agent; 0x02: partner; 0x03: agent&partner*/
        uint8_t fota_mode; /*0x00: background mode; 0x01: active mode*/
    } PACKED CMD;

    RACE_ERRCODE ret = RACE_ERRCODE_SUCCESS;

    CMD* pCmd = RACE_ClaimPacketAppID(RACE_APP_ID_FOTA,
        RACE_TYPE_COMMAND,
        RACE_FOTA_START,
        sizeof(CMD),
        FOTA_SRC_CHANNEL_ID);
    if (pCmd) {
        pCmd->recipient = 0x01;
        pCmd->fota_mode = 0x00;
        ret = race_flush_packet((uint8_t*)pCmd, FOTA_SRC_CHANNEL_ID);
    } else {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_start_req RACE_ClaimPacketAppID fail", 0);
        ret = RACE_ERRCODE_NOT_ENOUGH_MEMORY;
    }

    return ret;
}

RACE_ERRCODE RACE_CmdHandler_FOTA_get_4k_erase_status_req(uint32_t address, uint32_t length)
{
    typedef struct {
        uint8_t storage_type;
        uint8_t agent_or_partner;
        uint32_t partition_address;
        uint32_t partition_length;
    } PACKED CMD;

    RACE_ERRCODE ret = RACE_ERRCODE_SUCCESS;

    CMD* pCmd = RACE_ClaimPacketAppID(RACE_APP_ID_FOTA,
        RACE_TYPE_COMMAND,
        RACE_STORAGE_GET_4K_ERASED_STATUS,
        sizeof(CMD),
        FOTA_SRC_CHANNEL_ID);
    if (pCmd) {
        pCmd->storage_type = 0x00;
        pCmd->agent_or_partner = 0x00;
        pCmd->partition_address = address;
        pCmd->partition_length = length;
        ret = race_flush_packet((uint8_t*)pCmd, FOTA_SRC_CHANNEL_ID);
    } else {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_get_4k_erase_status_req RACE_ClaimPacketAppID fail", 0);
        ret = RACE_ERRCODE_NOT_ENOUGH_MEMORY;
    }

    return ret;
}

RACE_ERRCODE RACE_CmdHandler_FOTA_get_partition_sha256_req(uint32_t address, uint32_t length)
{
    typedef struct {
        uint8_t storage_type; /*0x00: internal flash; 0x01: external flash*/
        uint8_t agent_or_partner; /*0x00: agent*/
        uint32_t partition_address;
        uint32_t partition_length;
    } PACKED CMD;

    RACE_ERRCODE ret = RACE_ERRCODE_SUCCESS;

    CMD* pCmd = RACE_ClaimPacketAppID(RACE_APP_ID_FOTA,
        RACE_TYPE_COMMAND,
        RACE_STORAGE_GET_PARTITION_SHA256,
        sizeof(CMD),
        FOTA_SRC_CHANNEL_ID);
    if (pCmd) {
        pCmd->storage_type = 0x00;
        pCmd->agent_or_partner = 0x00;
        pCmd->partition_address = address;
        pCmd->partition_length = length;
        ret = race_flush_packet((uint8_t*)pCmd, FOTA_SRC_CHANNEL_ID);
    } else {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_get_partition_sha256_req RACE_ClaimPacketAppID fail", 0);
        ret = RACE_ERRCODE_NOT_ENOUGH_MEMORY;
    }

    return ret;
}

RACE_ERRCODE RACE_CmdHandler_FOTA_erase_partition_req(uint32_t address, uint32_t length)
{
    typedef struct {
        uint8_t storage_type; /*0x00: internal flash; 0x01: external flash*/
        uint32_t partition_length;
        uint32_t partition_address;
    } PACKED CMD;

    RACE_ERRCODE ret = RACE_ERRCODE_SUCCESS;

    CMD* pCmd = RACE_ClaimPacketAppID(RACE_APP_ID_FOTA,
        RACE_TYPE_COMMAND,
        RACE_STORAGE_ERASE_PARTITION,
        sizeof(CMD),
        FOTA_SRC_CHANNEL_ID);
    if (pCmd) {
        pCmd->storage_type = 0x00;
        pCmd->partition_length = length;
        pCmd->partition_address = address;
        ret = race_flush_packet((uint8_t*)pCmd, FOTA_SRC_CHANNEL_ID);
//        RACE_LOG_MSGID_I(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_erase_partition_req len=%d addr=0x%x ret=%d"
//            , 3
//            ,pCmd->partition_length,pCmd->partition_address,ret);
    } else {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_erase_partition_req RACE_ClaimPacketAppID fail", 0);
        ret = RACE_ERRCODE_NOT_ENOUGH_MEMORY;
    }

    return ret;
}

RACE_ERRCODE RACE_CmdHandler_FOTA_write_page_req(uint32_t address, const uint8_t *pData)
{
    typedef struct {
        uint8_t crc;
        uint32_t storage_addr;
        uint8_t data[FOTA_WRITE_BUFFER_UNIT];
    } PACKED race_storage_page_info_struct;

    typedef struct {
        uint8_t storage_type;  /*0x00: internal flash; 0x01: external flash*/
        uint8_t page_count;
        race_storage_page_info_struct page_info[0];
    } PACKED CMD;

    RACE_ERRCODE ret = RACE_ERRCODE_SUCCESS;

    CMD* pCmd = RACE_ClaimPacketAppID(RACE_APP_ID_FOTA,
        RACE_TYPE_COMMAND,
        RACE_STORAGE_WRITE_PAGE,
        sizeof(CMD) + FOTA_WRITE_PAGE_MAX_COUNT * sizeof(race_storage_page_info_struct),
        FOTA_SRC_CHANNEL_ID);
    if (pCmd) {
        pCmd->storage_type = 0x00;
        pCmd->page_count = FOTA_WRITE_PAGE_MAX_COUNT;

        uint8_t i = 0;
        for (i = 0; i < pCmd->page_count; i++) {
            uint8_t crc8 = 0;
            pCmd->page_info[i].storage_addr = address+FOTA_WRITE_BUFFER_UNIT*i;
            memcpy(pCmd->page_info[i].data, pData+FOTA_WRITE_BUFFER_UNIT*i, FOTA_WRITE_BUFFER_UNIT);
            ret = race_storage_crc8_generate(&crc8, (uint32_t)(pCmd->page_info[i].data), FOTA_WRITE_BUFFER_UNIT, pCmd->storage_type);
            if (ret != RACE_ERRCODE_SUCCESS) {
                RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_write_page_req race_storage_crc8_generate fail(%d), address(0x%X)", 2, ret, pCmd->page_info[i].storage_addr);
                return ret;
            }
            pCmd->page_info[i].crc = crc8;
        }
        ret = race_flush_packet((uint8_t*)pCmd, FOTA_SRC_CHANNEL_ID);
    } else {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_write_page_req RACE_ClaimPacketAppID fail", 0);
        ret = RACE_ERRCODE_NOT_ENOUGH_MEMORY;
    }

    return ret;
}

RACE_ERRCODE RACE_CmdHandler_FOTA_check_integrity_req()
{
    typedef struct {
        uint8_t recipient_count;  /*default value: 0x01*/
        uint8_t recipient; /*0x00: agent; 0x01: partner*/
        uint8_t storage_type; /*0x00: internal flash; 0x01: external flash*/
    } PACKED CMD;

    RACE_ERRCODE ret = RACE_ERRCODE_SUCCESS;

    CMD* pCmd = RACE_ClaimPacketAppID(RACE_APP_ID_FOTA,
        RACE_TYPE_COMMAND,
        RACE_FOTA_CHECK_INTEGRITY,
        sizeof(CMD),
        FOTA_SRC_CHANNEL_ID);
    if (pCmd) {
        pCmd->recipient_count = 0x01;
        pCmd->recipient = 0x00;
        pCmd->storage_type = 0x00;
        ret = race_flush_packet((uint8_t*)pCmd, FOTA_SRC_CHANNEL_ID);
    } else {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_check_integrity_req RACE_ClaimPacketAppID fail", 0);
        ret = RACE_ERRCODE_NOT_ENOUGH_MEMORY;
    }

    return ret;
}

RACE_ERRCODE RACE_CmdHandler_FOTA_commit_req()
{
    RACE_ERRCODE ret = RACE_ERRCODE_SUCCESS;

    void *pCmd = RACE_ClaimPacketAppID(RACE_APP_ID_FOTA,
        RACE_TYPE_COMMAND,
        RACE_FOTA_COMMIT,
        0,
        FOTA_SRC_CHANNEL_ID);
    if (pCmd) {
        ret = race_flush_packet((uint8_t*)pCmd, FOTA_SRC_CHANNEL_ID);
    } else {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_commit_req RACE_ClaimPacketAppID fail", 0);
        ret = RACE_ERRCODE_NOT_ENOUGH_MEMORY;
    }

    return ret;
}


/******************************************** FOTA SRC race res API********************************************/
void *RACE_CmdHandler_FOTA_get_version_recv_res(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id)
{
    typedef struct {
        uint8_t status;
    } PACKED RSP;

    const RSP *data = (const RSP *)pCmdMsg->payload;
    uint8_t status = data->status;
    if (status != RACE_RES_OK) {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_get_version_recv_res fail(%d)", 1, status);
        return NULL;
    }

    return NULL;
}

void *RACE_CmdHandler_FOTA_query_partition_info_recv_res(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id)
{
    typedef struct {
        uint8_t status;
        uint8_t partition_id;
        uint8_t storage_type;
        uint32_t partition_address;
        uint32_t partition_length;
    } PACKED RSP;

    const RSP *data = (const RSP *)pCmdMsg->payload;
    uint8_t status = data->status;
    if (status != RACE_RES_OK) {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_query_partition_info_recv_res fail(%d)", 1, status);
        return NULL;
    }

    g_fota_dut_partition_address = data->partition_address;
    g_fota_dut_partition_length = data->partition_length;
    g_fota_pkg_length = FOTA_get_pkg_length();
    RACE_LOG_MSGID_I(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_query_partition_info_recv_res dut_fota_partition_address(0x%X), dut_fota_partition_length(0x%X), fota_pkg_length(0x%X)", 3, g_fota_dut_partition_address, g_fota_dut_partition_length, g_fota_pkg_length);

    if (g_fota_pkg_length > g_fota_dut_partition_length) {
        RACE_LOG_MSGID_I(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_query_partition_info_recv_res pkg size(0x%X) is out of FOTA region size(0x%X)", 2, g_fota_pkg_length, g_fota_dut_partition_length);
        return NULL;
    }

    RACE_CmdHandler_FOTA_start_req();

    return NULL;
}

void *RACE_CmdHandler_FOTA_start_recv_res(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id)
{
    typedef struct {
        uint8_t status;
    } PACKED RSP;

    const RSP *data = (const RSP *)pCmdMsg->payload;
    uint8_t status = data->status;
    if (status != RACE_RES_OK) {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_start_recv_res fail(%d)", 1, status);
        return NULL;
    }

    return NULL;
}

void *RACE_CmdHandler_FOTA_get_4k_erase_status_recv_res(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id)
{
    UNUSED(length);
    UNUSED(channel_id);

    typedef struct {
        uint8_t status;
    } PACKED RSP;

    const RSP *data = (const RSP *)pCmdMsg;
    uint8_t status = data->status;
    if (status != RACE_RES_OK) {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_get_4k_erase_status_recv_res fail(%d)", 1, status);
        return NULL;
    }

    return NULL;
}

void *RACE_CmdHandler_FOTA_get_partition_sha256_recv_res(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id)
{
    UNUSED(length);
    UNUSED(channel_id);

    typedef struct {
        uint8_t status;
    } PACKED RSP;

    const RSP *data = (const RSP *)pCmdMsg->payload;
    uint8_t status = data->status;
    if (status != RACE_RES_OK) {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_get_partition_sha256_recv_res fail(%d)", 1, status);
        return NULL;
    }

    return NULL;
}

void *RACE_CmdHandler_FOTA_erase_partition_recv_res(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id)
{
    UNUSED(length);
    UNUSED(channel_id);

    typedef struct {
        uint8_t status;
    } PACKED RSP;

    const RSP *data = (const RSP *)pCmdMsg->payload;
    uint8_t status = data->status;
    if (status != RACE_RES_OK) {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_erase_partition_recv_res fail(%d)", 1, status);
        return NULL;
    }

    return NULL;

}

void *RACE_CmdHandler_FOTA_write_page_recv_res(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id)
{
    UNUSED(length);
    UNUSED(channel_id);

    typedef struct {
        uint8_t status;
        uint8_t storage_type;
        uint8_t completed_page_count;
        uint32_t storage_address[0];
    } PACKED RSP;

    const RSP *data = (const RSP *)pCmdMsg->payload;
    uint8_t status = data->status;
    if (status != RACE_RES_OK) {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_write_page_recv_res fail(%d)", 1, status);
        return NULL;
    }

    uint8_t i = 0;
    uint32_t cur_page_max_address = 0;
    for (i = 0; i < data->completed_page_count; i++) {
        if (data->storage_address[i] > cur_page_max_address) {
            cur_page_max_address = data->storage_address[i];
        }
    }

    const uint32_t next_address = cur_page_max_address + FOTA_WRITE_BUFFER_UNIT;
    if (next_address < g_fota_dut_partition_address + g_fota_pkg_length) {
        FOTA_write_page_process(next_address);
    } else {
        RACE_CmdHandler_FOTA_check_integrity_req();
    }

    return NULL;
}

void *RACE_CmdHandler_FOTA_check_integrity_recv_res(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id)
{
    typedef struct {
        uint8_t status;
    } PACKED RSP;

    const RSP *data = (const RSP *)pCmdMsg->payload;
    uint8_t status = data->status;
    if (status != RACE_RES_OK) {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_check_integrity_recv_res fail(%d)", 1, status);
        return NULL;
    }

    return NULL;
}

void *RACE_CmdHandler_FOTA_commit_recv_res(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id)
{
    typedef struct {
        uint8_t status;
    } PACKED RSP;

    const RSP *data = (const RSP *)pCmdMsg->payload;
    uint8_t status = data->status;
    if (status != RACE_RES_OK) {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_commit_recv_res fail(%d)", 1, status);
        return NULL;
    }

//#ifndef HEADSET_CONTROL_ON
//    RACE_CmdHandler_FOTA_SRC_state_execute_result_noti();
//#endif

    return NULL;
}


/******************************************** FOTA SRC race noti API********************************************/
void *RACE_CmdHandler_FOTA_get_version_recv_noti(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id)
{
    UNUSED(length);
    UNUSED(channel_id);

    const race_fota_get_version_noti_struct *data = (const race_fota_get_version_noti_struct *)pCmdMsg->payload;
    uint8_t status = data->status;
    if (status != RACE_RES_OK) {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_get_version_recv_noti fail(%d)", 1, status);
        return NULL;
    }

    RACE_CmdHandler_FOTA_SRC_query_dongle_version_noti(data);
    RACE_CmdHandler_FOTA_SRC_query_pkg_info_noti();
    RACE_CmdHandler_FOTA_SRC_state_execute_result_noti();

    return NULL;
}

void *RACE_CmdHandler_FOTA_start_recv_noti(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id)
{
    UNUSED(length);
    UNUSED(channel_id);

    const race_fota_start_noti_struct *data = (const race_fota_start_noti_struct *)pCmdMsg->payload;
    uint8_t status = data->status;
    if (status != RACE_RES_OK) {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_start_recv_noti fail(%d)", 1, status);
        return NULL;
    }

    

#ifdef FOTA_RESUME_MARKED
    RACE_CmdHandler_FOTA_erase_partition_req(g_fota_dut_partition_address, FOTA_4K_BLOCK_SIZE);
#else
    uint32_t bit_count = 0;
    if (g_fota_pkg_length % FOTA_4K_BLOCK_SIZE == 0) {
        bit_count = g_fota_pkg_length / FOTA_4K_BLOCK_SIZE;
    } else {
        bit_count = g_fota_pkg_length / FOTA_4K_BLOCK_SIZE + 1;
    }
    s_bit_buffer_byte_len = bit_count/8;

    if (s_4k_erase_status_bit_buffer != NULL) {
        race_mem_free(s_4k_erase_status_bit_buffer);
        s_4k_erase_status_bit_buffer = NULL;
    }

    s_4k_erase_status_bit_buffer = (uint8_t *)race_mem_alloc(s_bit_buffer_byte_len);
    if (s_4k_erase_status_bit_buffer == NULL) {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_start_recv_noti alloc fail [%d]", 1,s_bit_buffer_byte_len);
        return NULL;
    }
    memset(s_4k_erase_status_bit_buffer, 0, s_bit_buffer_byte_len);

    FOTA_get_4k_erase_status_process(g_fota_dut_partition_address);
#endif
    
//    

    return NULL;
}

void *RACE_CmdHandler_FOTA_get_4k_erase_status_recv_noti(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id)
{
    UNUSED(length);
    UNUSED(channel_id);

    const race_storage_get_4k_erased_status_noti_struct *data =
        (const race_storage_get_4k_erased_status_noti_struct *)pCmdMsg;

    // Fill bit buffer
    const uint32_t cur_4k_block_count = (data->partition_address - g_fota_dut_partition_address) / FOTA_4K_BLOCK_SIZE;
    const uint32_t bit_map_offset_byte = cur_4k_block_count/8;
    memcpy(&s_4k_erase_status_bit_buffer[bit_map_offset_byte], data->erase_status, data->erase_status_size);

    const uint32_t next_address = data->partition_address + data->partition_length;
    if (next_address < g_fota_dut_partition_address + g_fota_pkg_length) {
        FOTA_get_4k_erase_status_process(next_address);
    } else {
        const uint32_t bit_pos = FOTA_get_4K_erase_bit_pos(s_4k_erase_status_bit_buffer, s_bit_buffer_byte_len);
        g_fota_start_erase_address = g_fota_dut_partition_address + bit_pos*FOTA_4K_BLOCK_SIZE;

        if (s_4k_erase_status_bit_buffer != NULL) {
            race_mem_free(s_4k_erase_status_bit_buffer);
            s_4k_erase_status_bit_buffer = NULL;
        }

        FOTA_get_partition_sha256_process(g_fota_dut_partition_address);
    }

    return NULL;
}

void *RACE_CmdHandler_FOTA_get_partition_sha256_recv_noti(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id)
{
    UNUSED(length);
    UNUSED(channel_id);

    const race_storage_get_partition_sha256_noti_struct *data = (const race_storage_get_partition_sha256_noti_struct *)pCmdMsg->payload;;
    uint8_t status = data->status;
    if (status != RACE_RES_OK) {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_get_partition_sha256_recv_noti fail(%d)", 1, status);
        return NULL;
    }

    uint8_t sha256[RACE_STORAGE_SHA256_SIZE] = {0};
    RACE_ERRCODE ret = race_storage_sha256_generate(sha256,
                                       data->partition_address,
                                       data->partition_length,
                                       data->storage_type);
    if (ret != RACE_ERRCODE_SUCCESS) {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_get_partition_sha256_recv_noti race_storage_sha256_generate fail(%d)", 1, ret);
        return NULL;
    }

    const uint8_t next_address = data->partition_address + data->partition_length;

    if (next_address < g_fota_start_erase_address) {
        if (memcmp(data->sha256, sha256, RACE_STORAGE_SHA256_SIZE) == 0) {
            FOTA_get_partition_sha256_process(next_address);
        } else {
            g_fota_start_erase_address = data->partition_address;
            RACE_CmdHandler_FOTA_erase_partition_req(g_fota_start_erase_address, FOTA_4K_BLOCK_SIZE);
        }
    } else {
        g_fota_start_erase_address = data->partition_address;
        RACE_CmdHandler_FOTA_erase_partition_req(g_fota_start_erase_address, FOTA_4K_BLOCK_SIZE);
    }

    return NULL;
}

void *RACE_CmdHandler_FOTA_erase_partition_recv_noti(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id)
{
    UNUSED(length);
    UNUSED(channel_id);

    const race_storage_erase_partition_noti_struct *data = (const race_storage_erase_partition_noti_struct *)pCmdMsg->payload;;
    uint8_t status = data->status;
    if (status != RACE_RES_OK) {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_erase_partition_recv_noti fail(%d)", 1, status);
        return NULL;
    }

    const uint32_t next_address = data->partition_address + data->partition_length;

    if (next_address < g_fota_dut_partition_address + g_fota_pkg_length) {
        RACE_CmdHandler_FOTA_erase_partition_req(next_address, FOTA_4K_BLOCK_SIZE);
        RACE_LOG_MSGID_I(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_erase_partition_recv_noti (%x)/(%x)"
        , 2, next_address,(g_fota_dut_partition_address + g_fota_pkg_length));
    } else {
//        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_erase_partition_recv_noti FOTA_write_page_process",0);
        #ifdef FOTA_RESUME_MARKED
        FOTA_write_page_process(g_fota_dut_partition_address);
        #else
        FOTA_write_page_process(g_fota_start_erase_address);
        #endif
//        
    }

    return NULL;
}

void *RACE_CmdHandler_FOTA_check_integrity_recv_noti(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id)
{
    UNUSED(length);
    UNUSED(channel_id);

    const race_fota_check_integrity_noti_struct *data = (const race_fota_check_integrity_noti_struct *)pCmdMsg->payload;
    uint8_t status = data->status;
    if (status != RACE_RES_OK) {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_check_integrity_recv_noti fail(%d)", 1, status);
        return NULL;
    }

    RACE_CmdHandler_FOTA_SRC_state_execute_result_noti();

    return NULL;
}


/******************************************** App race API********************************************/
void *RACE_CmdHandler_FOTA_SRC_query_state_res(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id)
{
    UNUSED(length);

    typedef struct {
        uint8_t status;
        uint8_t fota_state;
    } PACKED RSP;

    RSP *pEvt = RACE_ClaimPacketAppID(pCmdMsg->hdr.pktId.field.app_id,
                                      RACE_TYPE_RESPONSE,
                                      RACE_FOTA_SRC_QUERY_STATE,
                                      sizeof(RSP),
                                      channel_id);
    if (pEvt != NULL) {
        pEvt->fota_state = en_fota_state;
        pEvt->status = RACE_RES_OK;
        race_flush_packet((uint8_t*)pEvt, channel_id);
    } else {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_SRC_query_state_res RACE_ClaimPacketAppID fail", 0);
        return NULL;
    }

    return NULL;//pEvt;
}

void *RACE_CmdHandler_FOTA_SRC_trigger_into_state_res(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id)
{
    UNUSED(length);

    typedef struct {
        RACE_COMMON_HDR_STRU cmdhdr;
        uint8_t state;
    } PACKED CMD;

    typedef struct {
        uint8_t status;
    } PACKED RSP;

    CMD *pCmd = (CMD *)pCmdMsg;
    if (pCmd != NULL && pCmd->state <= FOTA_COMMIT_STATE_) {
        s_app_interaction_header_id = pCmdMsg->hdr.pktId.field.app_id;
        s_app_interaction_channel_id = channel_id;
        en_fota_state = pCmd->state;
    }

    RACE_LOG_MSGID_I(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_SRC_trigger_into_state_res en_fota_state(%d)", 1, en_fota_state);
    RSP *pEvt = RACE_ClaimPacketAppID(pCmdMsg->hdr.pktId.field.app_id,
                                      RACE_TYPE_RESPONSE,
                                      RACE_FOTA_SRC_TRIGGER_INTO_STATE,
                                      sizeof(RSP),
                                      channel_id);
    if (pEvt != NULL) {
        pEvt->status = RACE_RES_OK;
        race_flush_packet((uint8_t*)pEvt, channel_id);
    } else {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_SRC_trigger_into_state_res RACE_ClaimPacketAppID fail", 0);
        return NULL;
    }

    if (en_fota_state == FOTA_IDLE_STATE_) {
        RACE_CmdHandler_FOTA_SRC_state_execute_result_noti();
    } else if (en_fota_state == FOTA_QUERY_STATE_) {
        RACE_CmdHandler_FOTA_get_version_req();
    } else if (en_fota_state == FOTA_TRANSFERRING_STATE_) {
        RACE_CmdHandler_FOTA_query_partition_info_req();
    } else if (en_fota_state == FOTA_COMMIT_STATE_) {
        RACE_CmdHandler_FOTA_commit_req();
    }

    return NULL;//pEvt;
}

RACE_ERRCODE RACE_CmdHandler_FOTA_SRC_query_dongle_version_noti(const race_fota_get_version_noti_struct *data)
{
    typedef struct {
        uint8_t status;
        uint8_t version_len;
        uint8_t version_data[0];
    } PACKED RSP;

    RACE_ERRCODE ret = RACE_ERRCODE_SUCCESS;

    /* Get dongle fota version */
    uint8_t dongle_ver[FOTA_VERSION_MAX_SIZE] = {0};
    const uint8_t ver_len = (data->version_len < FOTA_VERSION_MAX_SIZE) ? data->version_len : FOTA_VERSION_MAX_SIZE;
    memcpy(dongle_ver, data->version, ver_len);
    APPS_LOG_DUMP_I(APP_FOTA_SRC_RACE"Get dongle fota version ", dongle_ver, ver_len);

    RSP *pEvt = RACE_ClaimPacketAppID(s_app_interaction_header_id,
                                      RACE_TYPE_NOTIFICATION,
                                      RACE_FOTA_SRC_QUERY_DONGLE_VERSION,
                                      sizeof(RSP)+ver_len,
                                      s_app_interaction_channel_id);
    if (pEvt != NULL) {
        pEvt->status = RACE_RES_OK;
        pEvt->version_len = ver_len;
        memcpy(pEvt->version_data, dongle_ver, ver_len);
        race_flush_packet((uint8_t*)pEvt, s_app_interaction_channel_id);
    } else {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_SRC_query_dongle_version_noti RACE_ClaimPacketAppID fail", 0);
        ret = RACE_ERRCODE_NOT_ENOUGH_MEMORY;
    }

    return ret;
}

static void RACE_CMD_Print_FOTA_Head_info(uint8_t *head_item, uint8_t item_len,uint8_t item_type)
{
    if (item_len >= 4 && head_item)
    {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_SRC_query_pkg_info_noti %d= 0x%02X,0x%02X,0x%02X,0x%02X"
        , 5
        ,item_type,head_item[0],head_item[1],head_item[2],head_item[3]);
    }
}


RACE_ERRCODE RACE_CmdHandler_FOTA_SRC_query_pkg_info_noti()
{
    typedef struct {
        uint8_t status;
        uint8_t version_len;
        uint8_t category_len;
        uint8_t device_type_len;
        uint8_t data[0];
    } PACKED RSP;

    RACE_ERRCODE ret = RACE_ERRCODE_SUCCESS;

    uint8_t pkg_ver_buf[FOTA_VERSION_MAX_SIZE] = {0};
    uint8_t category_buf[64] = {0};
    uint8_t device_type_buf[64] = {0};

    if (fota_parse_version_in_header(pkg_ver_buf,FOTA_VERSION_MAX_SIZE) != FOTA_ERRCODE_SUCCESS)
    {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_SRC_query_pkg_info_noti ,get version fail.",0);
    }
    if (fota_parse_categray_in_header(category_buf, 64) != FOTA_ERRCODE_SUCCESS)
    {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_SRC_query_pkg_info_noti ,get categray fail.",0);
    }
    if (fota_parse_device_type_in_header(device_type_buf, 64) != FOTA_ERRCODE_SUCCESS)
    {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_SRC_query_pkg_info_noti ,get device type fail.",0);
    }
   
    uint8_t temp_length = strlen((const char *)pkg_ver_buf);//FOTA_get_pkg_version_len();
    const uint8_t pkg_ver_len = temp_length > 64 ? 64 : temp_length; 
    temp_length = strlen((const char *)category_buf);//FOTA_get_pkg_category_len();
    const uint8_t category_len = temp_length > 64 ? 64 : temp_length; //strlen((const char *)category_buf);
    temp_length = strlen((const char *)device_type_buf);//FOTA_get_pkg_device_type_len();
    const uint8_t device_type_len = temp_length > 64 ? 64 : temp_length;//

    RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_SRC_query_pkg_info_noti ver_len=%d,category_len=%d,type_len=%d"
    , 3
    ,pkg_ver_len,category_len,device_type_len);

    RSP *pEvt = RACE_ClaimPacketAppID(s_app_interaction_header_id,
                                      RACE_TYPE_NOTIFICATION,
                                      RACE_FOTA_SRC_QUERY_PKG_INFO,
                                      (sizeof(RSP) + pkg_ver_len + 1 + category_len+ 1 + device_type_len + 1),
                                      s_app_interaction_channel_id);
    if (pEvt != NULL) {
        pEvt->status = RACE_RES_OK;
        pEvt->version_len = pkg_ver_len;
        memcpy(pEvt->data, pkg_ver_buf, pkg_ver_len);
        pEvt->category_len = category_len;
        memcpy(pEvt->data + pkg_ver_len, category_buf, category_len);
        pEvt->device_type_len = device_type_len;
        memcpy(pEvt->data  + pkg_ver_len + category_len, device_type_buf, device_type_len);
        race_flush_packet((uint8_t*)pEvt, s_app_interaction_channel_id);
    } else {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_SRC_query_pkg_info_noti RACE_ClaimPacketAppID fail", 0);
        ret = RACE_ERRCODE_NOT_ENOUGH_MEMORY;
    }

    return ret;
}

RACE_ERRCODE RACE_CmdHandler_FOTA_SRC_state_execute_result_noti()
{
    typedef struct {
        uint8_t status;
        uint8_t fota_state;
    } PACKED RSP;

    RACE_ERRCODE ret = RACE_ERRCODE_SUCCESS;

    RSP *noti = RACE_ClaimPacketAppID(s_app_interaction_header_id,
                                      RACE_TYPE_NOTIFICATION,
                                      RACE_FOTA_SRC_STATE_EXECUTE_RESULT,
                                      sizeof(RSP),
                                      s_app_interaction_channel_id);
    if (noti != NULL) {
        noti->status = RACE_RES_OK;
        noti->fota_state = en_fota_state;
        ret = race_flush_packet((uint8_t*)noti, s_app_interaction_channel_id);
    } else {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_SRC_state_execute_result_noti RACE_ClaimPacketAppID fail", 0);
        ret = RACE_ERRCODE_NOT_ENOUGH_MEMORY;
    }

    return ret;
}

RACE_ERRCODE RACE_CmdHandler_FOTA_SRC_transferring_info_noti(uint32_t prcessed_size, uint32_t total_size)
{
    typedef struct {
        uint8_t status;
        uint32_t prcessed_size;
        uint32_t total_size;
    } PACKED RSP;

    RACE_ERRCODE ret = RACE_ERRCODE_SUCCESS;

    RSP *noti = RACE_ClaimPacketAppID(s_app_interaction_header_id,
                                      RACE_TYPE_NOTIFICATION,
                                      RACE_FOTA_SRC_TRANSFERRING_NOTIFY,
                                      sizeof(RSP),
                                      s_app_interaction_channel_id);
    if (noti != NULL) {
        noti->status = RACE_RES_OK;
        noti->prcessed_size = prcessed_size;
        noti->total_size = total_size;
        ret = race_flush_packet((uint8_t*)noti, s_app_interaction_channel_id);
    } else {
        RACE_LOG_MSGID_E(APP_FOTA_SRC_RACE"RACE_CmdHandler_FOTA_SRC_transferring_info_noti RACE_ClaimPacketAppID fail", 0);
        ret = RACE_ERRCODE_NOT_ENOUGH_MEMORY;
    }

    return ret;
}

#endif

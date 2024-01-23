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


#include "race_cmd_feature.h"

#ifdef RACE_OFFLINE_LOG_CMD_ENABLE
#include "race_cmd.h"
#include "race_cmd_offline_log.h"
#include "race_lpcomm_trans.h"
#include "race_lpcomm_util.h"
#include "race_lpcomm_msg_struct.h"
#include "race_lpcomm_conn.h"
#include "race_noti.h"
#include "race_lpcomm_ps_noti.h"
#include "offline_dump.h"
#include "mux.h"

#ifdef MTK_MINIDUMP_ENABLE
#include "exception_handler.h"
#endif

#define CMD_GET_DUMP_INFO           0x00
#define CMD_SELECT_IDENX_DUMP       0x01

#define CMD_CELL_IS_INVALID         0x01
#define CMD_CELL_IS_NOT_INVALID     0x00

#define CMD_CELL_FLASH_LOG_IS_START     0x01
#define CMD_CELL_FLASH_LOG_IS_STOP      0x00

////////////////////////////////////////////////////////////////////////////////
// Constant Definitions ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// TYPE DEFINITIONS ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Global Variables ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// FUNCTION DECLARATIONS ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void *RACE_QUERY_OFFLINE_DUMP_REGION_INFO_HDR(PTR_RACE_COMMON_HDR_STRU pCmdMsg, uint16_t length, uint8_t channel_id)
{
    offline_dump_status_t ret;
    uint32_t region_idenx;
    uint32_t min_seq, max_seq;
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint8_t  region_type;
    } PACKED THIS_RACE_CMD_STRU;

    typedef struct {
        uint8_t  status;
        uint8_t  region_type;
        uint32_t res_min_seq;
        uint32_t res_max_seq;
    } PACKED *PTR_THIS_RACE_EVT_STRU;

    THIS_RACE_CMD_STRU *pThisCmd = (THIS_RACE_CMD_STRU *)pCmdMsg;
    PTR_THIS_RACE_EVT_STRU pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE, RACE_QUERY_OFFLINE_DUMP_REGION_INFO, 10, channel_id);
    if (pEvt == NULL) {
        return (void *)pEvt;
    }

    region_idenx = pThisCmd->region_type;
    pEvt->region_type       = region_idenx;
    pEvt->res_min_seq       = 0;
    pEvt->res_max_seq       = 0;

    /* region type check */
    if (region_idenx >= OFFLINE_REGION_MAX) {
        pEvt->status = RACE_ERRCODE_FAIL;
        return (void *)pEvt;
    }

    ret = offline_dump_region_query_seq_range(region_idenx, &min_seq, &max_seq);
    if (ret != OFFLINE_STATUS_OK) {
        pEvt->status = RACE_ERRCODE_FAIL;
    } else {
        pEvt->status            = RACE_ERRCODE_SUCCESS;
        pEvt->region_type       = region_idenx;
        pEvt->res_min_seq       = min_seq;
        pEvt->res_max_seq       = max_seq;
    }
    RACE_LOG_MSGID_I("OFFLINE DUMP: query_seq_range ret:%d [0:pass other:fail] type:%d min:%d max:%d", 4,
                        ret, pEvt->region_type, pEvt->res_min_seq, pEvt->res_max_seq);

    return (void *)pEvt;
}

void *RACE_QUERY_OFFLINE_DUMP_REGION_ADDRESS_HDR(PTR_RACE_COMMON_HDR_STRU pCmdMsg, uint16_t length, uint8_t channel_id)
{
    offline_dump_status_t ret;
    uint32_t region_idenx;
    uint32_t min_seq, max_seq, current_seq;
    uint32_t start_addr, total_length;
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint8_t  region_type;
        uint32_t dump_idenx;
    } PACKED THIS_RACE_CMD_STRU;

    typedef struct {
        uint8_t  status;
        uint8_t  region_type;
        uint32_t block_length;
        uint32_t dump_address;
    } PACKED *PTR_THIS_RACE_EVT_STRU;

    THIS_RACE_CMD_STRU *pThisCmd = (THIS_RACE_CMD_STRU *)pCmdMsg;
    PTR_THIS_RACE_EVT_STRU pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE, RACE_QUERY_OFFLINE_DUMP_REGION_ADDRESS, 10, channel_id);
    if (pEvt == NULL) {
        return (void *)pEvt;
    }

    region_idenx = pThisCmd->region_type;
    pEvt->region_type       = region_idenx;
    pEvt->block_length      = 0x0;
    pEvt->dump_address      = 0x0;

    /* region type check */
    if (region_idenx >= OFFLINE_REGION_MAX) {
        pEvt->status = RACE_ERRCODE_FAIL;
        return (void *)pEvt;
    }

    ret = offline_dump_region_query_seq_range(region_idenx, &min_seq, &max_seq);
    if (ret != OFFLINE_STATUS_OK) {
        pEvt->status = RACE_ERRCODE_FAIL;
        return (void *)pEvt;
    }

    if ((pThisCmd->dump_idenx > max_seq) || (pThisCmd->dump_idenx < min_seq)) {
        pEvt->status = RACE_ERRCODE_FAIL;
        return (void *)pEvt;
    } else {
        current_seq = pThisCmd->dump_idenx;
    }

    ret = offline_dump_region_query_by_seq(region_idenx, current_seq, &start_addr, &total_length);
    if (ret == OFFLINE_STATUS_OK) {
        pEvt->status            = RACE_ERRCODE_SUCCESS;
        pEvt->region_type       = region_idenx;
        pEvt->block_length      = total_length;
        pEvt->dump_address      = start_addr;
    } else {
        pEvt->status = RACE_ERRCODE_FAIL;
    }
    RACE_LOG_MSGID_D("OFFLINE DUMP: query_by_seq ret:%d [0:pass other:fail] type:%d length:%d address:0x%08x", 4,
                        ret, pEvt->region_type, pEvt->block_length, pEvt->dump_address);

    return (void *)pEvt;
}


void *RACE_OFFLINE_MARK_INVALID_HDR(PTR_RACE_COMMON_HDR_STRU pCmdMsg, uint16_t length, uint8_t channel_id)
{
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint8_t  region_type;
        uint32_t mark_seq;
    } PACKED THIS_RACE_CMD_STRU;

    typedef struct {
        uint8_t  status;
    } PACKED *PTR_THIS_RACE_EVT_STRU;

    THIS_RACE_CMD_STRU *pThisCmd = (THIS_RACE_CMD_STRU *)pCmdMsg;
    PTR_THIS_RACE_EVT_STRU pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE, RACE_OFFLINE_MARK_INVALID, 1, channel_id);
    if (pEvt == NULL) {
        return (void *)pEvt;
    }

    if (offline_dump_region_mark_invalid_cell(pThisCmd->region_type, pThisCmd->mark_seq) == OFFLINE_STATUS_OK) {
        pEvt->status = RACE_ERRCODE_SUCCESS;
    } else {
        pEvt->status = RACE_ERRCODE_FAIL;
    }

    LOG_MSGID_I(common, "Offline mark region:%d cell:%d status:%d", 3, pThisCmd->region_type, pThisCmd->mark_seq, pEvt->status);

    return (void *)pEvt;
}

void *RACE_OFFLINE_QUERY_CELL_INVALID_STATE_HDR(PTR_RACE_COMMON_HDR_STRU pCmdMsg, uint16_t length, uint8_t channel_id)
{
    offline_dump_status_t ret;
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint8_t  region_type;
        uint32_t demand_seq;
    } PACKED THIS_RACE_CMD_STRU;

    typedef struct {
        uint8_t  status;
        uint8_t  region_type;
        uint8_t  invalid_status;
    } PACKED *PTR_THIS_RACE_EVT_STRU;

    THIS_RACE_CMD_STRU *pThisCmd = (THIS_RACE_CMD_STRU *)pCmdMsg;
    PTR_THIS_RACE_EVT_STRU pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE, RACE_OFFLINE_QUERY_CELL_INVALID_STATE, 3, channel_id);

    if (pEvt == NULL) {
        return (void *)pEvt;
    }

    pEvt->status = RACE_ERRCODE_FAIL;
    pEvt->region_type = pThisCmd->region_type;
    pEvt->invalid_status = 0x00;

    ret = offline_dump_region_query_cell_invalid_state(pThisCmd->region_type, pThisCmd->demand_seq);
    if (ret == OFFLINE_STATUS_CELL_INVALID) {
        pEvt->status = RACE_ERRCODE_SUCCESS;
        pEvt->invalid_status = CMD_CELL_IS_INVALID;
    } else if (ret == OFFLINE_STATUS_CELL_NOT_INVALID) {
        pEvt->status = RACE_ERRCODE_SUCCESS;
        pEvt->invalid_status = CMD_CELL_IS_NOT_INVALID;
    } else {
        pEvt->status = RACE_ERRCODE_FAIL;
        pEvt->invalid_status = 0x00;
    }

    LOG_MSGID_I(common, "Offline query invalid region:%d cell:%d status:%d invalid:%d", 4,
                pThisCmd->region_type, pThisCmd->demand_seq, pEvt->status, pEvt->invalid_status);

    return (void *)pEvt;
}


void *RACE_GET_EXCEPTION_LOG_HDR(PTR_RACE_COMMON_HDR_STRU pCmdMsg, uint16_t length, uint8_t channel_id)
{
    typedef struct {
        uint8_t  status;
        uint32_t data_length;
        uint32_t data_address;
    } PACKED RSP;

    offline_dump_status_t ret;
    uint32_t min_seq = 0, max_seq = 0;
    uint32_t start_addr = 0, total_length = 0;

    RSP *pEvt = RACE_ClaimPacketAppID(pCmdMsg->pktId.field.app_id,
                                        RACE_TYPE_RESPONSE,
                                        RACE_BOOTREASON_OFFLINE_LOG_GET,
                                        sizeof(RSP),
                                        channel_id);
    if (pEvt) {
        ret = offline_dump_region_query_seq_range(OFFLINE_REGION_EXCEPTION_LOG, &min_seq, &max_seq);
        RACE_LOG_MSGID_I("Offline dump: query ret:%d [0:pass other:fail] min:%d max:%d", 3, ret, min_seq, max_seq);
#ifdef MTK_MINIDUMP_ENABLE
        {
            uint32_t minidump_seq = 0;
            exception_status_t minidump_qurey_status;
            minidump_qurey_status = exception_minidump_region_query_latest_index(&minidump_seq);
            RACE_LOG_MSGID_I("Offline dump: mini ret:%d [0:pass other:fail] min:%d max:%d", 2, minidump_qurey_status, minidump_seq);
            if (minidump_qurey_status == EXCEPTION_STATUS_OK) {
                minidump_seq = minidump_seq - 1;
                if (minidump_seq >= min_seq && minidump_seq <= max_seq) {
                    max_seq = minidump_seq;
                }
            }
        }
#endif
        if (ret == OFFLINE_STATUS_OK) {
            offline_dump_region_query_by_seq(OFFLINE_REGION_EXCEPTION_LOG, max_seq, &start_addr, &total_length);
            pEvt->status        = RACE_ERRCODE_SUCCESS;
            pEvt->data_address  = start_addr;
            pEvt->data_length   = total_length;
            RACE_LOG_MSGID_I("Offline dump: dump max_seq:%d start_addr:%d len:%d", 3, max_seq, start_addr, total_length);
        } else {
            pEvt->status        = RACE_ERRCODE_FAIL;
            pEvt->data_address  = 0;
            pEvt->data_length   = 0;
        }
        RACE_LOG_MSGID_D("OFFLINE DUMP: dump max_seq:%d start_addr:%d len:%d", 3, max_seq, start_addr, total_length);
    }

    return (void *)pEvt;
}

void *RACE_TRIGGER_OFFLINE_LOG_HDR(PTR_RACE_COMMON_HDR_STRU pCmdMsg, uint16_t length, uint8_t channel_id)
{
    typedef struct {
        uint8_t  status;
    } PACKED RSP;

#ifdef AIR_BTA_IC_PREMIUM_G3
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint32_t  unix_time;
    } PACKED THIS_RACE_CMD_STRU;
    THIS_RACE_CMD_STRU *pThisCmd = (THIS_RACE_CMD_STRU *)pCmdMsg;
#endif
    RSP *pEvt = RACE_ClaimPacketAppID(pCmdMsg->pktId.field.app_id,
                                        RACE_TYPE_RESPONSE,
                                        RACE_TRIGGER_OFFLINE_LOG,
                                        sizeof(RSP),
                                        channel_id);

    if (pEvt) {
#if defined(MTK_NVDM_ENABLE) && defined(MTK_MUX_ENABLE)
        #ifdef AIR_BTA_IC_PREMIUM_G3
        offline_dump_set_rtc_time_unix(pThisCmd->unix_time);
        RACE_LOG_MSGID_I("RACE_CmdHandler_offline_log Config Epoch Time[%d] !!! ", 1, pThisCmd->unix_time);
        #else
        RACE_LOG_MSGID_I("RACE_CmdHandler_offline_log Trigger Offline log!!!", 0);
        #endif
        pEvt->status = RACE_ERRCODE_SUCCESS;
#else
        pEvt->status = RACE_ERRCODE_NOT_SUPPORT;
#endif
    }
    return (void *)pEvt;
}

void *RACE_OFFLINE_QUERY_LOG_TO_FLASH_SWITCH_HDR(PTR_RACE_COMMON_HDR_STRU pCmdMsg, uint16_t length, uint8_t channel_id)
{
    uint32_t on_off_status = 0x0;
    mux_status_t control_status;

    typedef struct {
        uint8_t  status;
        uint8_t  switch_status;
    } PACKED RSP;

    RSP *pEvt = RACE_ClaimPacketAppID(pCmdMsg->pktId.field.app_id,
                                        RACE_TYPE_RESPONSE,
                                        RACE_OFFLINE_QUERY_LOG_TO_FLASH_SWITCH,
                                        sizeof(RSP),
                                        channel_id);

    if (pEvt) {
#if defined(MTK_NVDM_ENABLE) && defined(MTK_MUX_ENABLE)
    control_status = mux_control(MUX_FLASH, MUX_CMD_GET_CONNECTION_PARAM, (mux_ctrl_para_t *)&on_off_status);
    if (control_status != MUX_STATUS_OK) {
        pEvt->status = RACE_ERRCODE_FAIL;
    } else {
        pEvt->status = RACE_ERRCODE_SUCCESS;
    }

    pEvt->switch_status = on_off_status;
    RACE_LOG_MSGID_I("RACE_OFFLINE_QUERY_LOG_TO_FLASH_SWITCH_HDR status:%d switch_status:%d", 2, control_status, on_off_status);
#else
    pEvt->status = RACE_ERRCODE_NOT_SUPPORT;
    pEvt->switch_status = CMD_CELL_FLASH_LOG_IS_STOP;
#endif
    }

    return (void *)pEvt;
}

void *RACE_OFFLINE_SET_LOG_TO_FLASH_SWITCH_HDR(PTR_RACE_COMMON_HDR_STRU pCmdMsg, uint16_t length, uint8_t channel_id)
{
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint8_t  switch_on_off;
    } PACKED THIS_RACE_CMD_STRU;

    typedef struct {
        uint8_t  status;
    } PACKED RSP;

    THIS_RACE_CMD_STRU *pThisCmd = (THIS_RACE_CMD_STRU *)pCmdMsg;
    RSP *pEvt = RACE_ClaimPacketAppID(pCmdMsg->pktId.field.app_id,
                                        RACE_TYPE_RESPONSE,
                                        RACE_OFFLINE_SET_LOG_TO_FLASH_SWITCH,
                                        sizeof(RSP),
                                        channel_id);

    if (pEvt) {
#if defined(MTK_NVDM_ENABLE) && defined(MTK_MUX_ENABLE)
        RACE_LOG_MSGID_I("RACE_CmdHandler_offline_log switch log to flash:%d ", 1, pThisCmd->switch_on_off);
        if (pThisCmd->switch_on_off == 0x00) {
            mux_control(MUX_FLASH, MUX_CMD_DISCONNECT, NULL);
        } else {
            mux_control(MUX_FLASH, MUX_CMD_CONNECT, NULL);
        }
        pEvt->status = RACE_ERRCODE_SUCCESS;
#else
        pEvt->status = RACE_ERRCODE_FAIL;
#endif
    }
    return (void *)pEvt;
}

void *RACE_OFFLINE_SET_UNIX_TIME_HDR(PTR_RACE_COMMON_HDR_STRU pCmdMsg, uint16_t length, uint8_t channel_id)
{
    typedef struct {
        RACE_COMMON_HDR_STRU Hdr;
        uint32_t  set_unix_time;
    } PACKED THIS_RACE_CMD_STRU;

    typedef struct {
        uint8_t  status;
        uint32_t read_unix_time;
    } PACKED RSP;

    THIS_RACE_CMD_STRU *pThisCmd = (THIS_RACE_CMD_STRU *)pCmdMsg;
    RSP *pEvt = RACE_ClaimPacketAppID(pCmdMsg->pktId.field.app_id,
                                        RACE_TYPE_RESPONSE,
                                        RACE_OFFLINE_SET_UNIX_TIME,
                                        sizeof(RSP),
                                        channel_id);

    if (pEvt) {
        RACE_LOG_MSGID_I("RACE_OFFLINE_SET_UNIX_TIME_HDR Epoch Time[%d] !!! ", 1, pThisCmd->set_unix_time);
        if (offline_dump_set_rtc_time_unix(pThisCmd->set_unix_time) == true) {
            pEvt->status = RACE_ERRCODE_SUCCESS;
            pEvt->read_unix_time = pThisCmd->set_unix_time;
        } else {
            pEvt->status = RACE_ERRCODE_FAIL;
            pEvt->read_unix_time = 0;
        }
    }

    return (void *)pEvt;
}

void *RACE_CmdHandler_offline_log(ptr_race_pkt_t pCmdMsg, uint16_t length, uint8_t channel_id)
{
    RACE_LOG_MSGID_D("RACE_CmdHandler_offline_log, pCmdMsg->hdr.id[0x%X]", 1, (int)pCmdMsg->hdr.id);

    switch (pCmdMsg->hdr.id) {
        case RACE_TRIGGER_OFFLINE_LOG: { //0x1E03
            return RACE_TRIGGER_OFFLINE_LOG_HDR((PTR_RACE_COMMON_HDR_STRU) & (pCmdMsg->hdr), length, channel_id);
        }
        break;

        case RACE_QUERY_OFFLINE_DUMP_REGION_INFO: { //0x1E04
            return RACE_QUERY_OFFLINE_DUMP_REGION_INFO_HDR((PTR_RACE_COMMON_HDR_STRU) & (pCmdMsg->hdr), length, channel_id);
        }
        break;

        case RACE_QUERY_OFFLINE_DUMP_REGION_ADDRESS: { //0x1E05
            return RACE_QUERY_OFFLINE_DUMP_REGION_ADDRESS_HDR((PTR_RACE_COMMON_HDR_STRU) & (pCmdMsg->hdr), length, channel_id);
        }
        break;

        case RACE_BOOTREASON_OFFLINE_LOG_GET: { //0x1E06
            return RACE_GET_EXCEPTION_LOG_HDR((PTR_RACE_COMMON_HDR_STRU) & (pCmdMsg->hdr), length, channel_id);
        }
        break;

        case RACE_ID_OFFLINE_ASSERT: { //0x1E07
            assert(0);
        }
        break;

        case RACE_OFFLINE_MARK_INVALID: { //0x1E10
            return RACE_OFFLINE_MARK_INVALID_HDR((PTR_RACE_COMMON_HDR_STRU) & (pCmdMsg->hdr), length, channel_id);
        }
        break;

        case RACE_OFFLINE_QUERY_CELL_INVALID_STATE: { //0x1E11
            return RACE_OFFLINE_QUERY_CELL_INVALID_STATE_HDR((PTR_RACE_COMMON_HDR_STRU) & (pCmdMsg->hdr), length, channel_id);
        }
        break;

        case RACE_OFFLINE_QUERY_LOG_TO_FLASH_SWITCH: { //0x1E14
            return RACE_OFFLINE_QUERY_LOG_TO_FLASH_SWITCH_HDR((PTR_RACE_COMMON_HDR_STRU) & (pCmdMsg->hdr), length, channel_id);
        }
        break;

        case RACE_OFFLINE_SET_LOG_TO_FLASH_SWITCH: { //0x1E15
            return RACE_OFFLINE_SET_LOG_TO_FLASH_SWITCH_HDR((PTR_RACE_COMMON_HDR_STRU) & (pCmdMsg->hdr), length, channel_id);
        }
        break;

        case RACE_OFFLINE_SET_UNIX_TIME: { //0x1E16
            return RACE_OFFLINE_SET_UNIX_TIME_HDR((PTR_RACE_COMMON_HDR_STRU) & (pCmdMsg->hdr), length, channel_id);
        }
        break;

        default: {
            ;
        }
        break;
    }

    return NULL;
}

#endif /* RACE_OFFLINE_LOG_CMD_ENABLE */

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
#include "race_cmd.h"
#include "race_xport.h"
#include "hal_sleep_manager.h"
#include "hal.h"
#include "hal_pmu.h"
#include "race_util.h"

#if ((PRODUCT_VERSION == 1552) || defined(AM255X))
#include "hal_pmu_mt6388_platform.h"
#endif
#if (defined(AIR_BTA_IC_PREMIUM_G2) && defined(AIR_BTA_PMIC_LP))
#include "hal_pmu_ab2565_platform.h"
#endif
#if (defined(AIR_BTA_IC_PREMIUM_G2) && defined(AIR_BTA_PMIC_HP))
#include "hal_pmu_ab2568_platform.h"
#endif

#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
#include "battery_management_core.h"
#endif

#if defined(AIR_1WIRE_ENABLE)
#include "smchg_1wire.h"
#endif

//#define RACE_CMD_SYSTEM_LOG_ENABLE

////////////////////////////////////////////////////////////////////////////////
// Constant Definitions ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define RACE_WRITE_SFR                      0x0200
#define RACE_READ_SFR                       0x0201
#define RACE_SLEEP_CONTROL                  0x0220
#define RACE_WRITE_REG_I2C                  0x020E
#define RACE_READ_REG_I2C                   0x020F
#define RACE_CHARGER_ENABLE                 0x0240
#define RACE_SMCHG_CTRL                     0x0250

typedef enum {
    RACE_SMCHG_LOG_MODE,
    RACE_SMCHG_MAX,
} race_smchg_ctrl_t;


//////////////////////////////////////////////////////////////////////////////
// Global Variables ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
typedef struct stru_reg_sfr {
    uint32_t Addr;
    uint8_t NumByte;
    uint8_t Value[0];

} PACKED RACE_REG_SFR_STRU, * PTR_RACE_REG_SFR_STRU;

typedef struct stru_reg_sfr_1byte {
    uint32_t Addr;
    uint8_t NumByte;
    uint8_t Value;

} PACKED RACE_REG_SFR_1BYTE_STRU, * PTR_RACE_REG_SFR_1BYTE_STRU;

typedef struct stru_reg_sfr_2byte {
    uint32_t Addr;
    uint8_t NumByte;
    uint16_t Value;

} PACKED RACE_REG_SFR_2BYTE_STRU, * PTR_RACE_REG_SFR_2BYTE_STRU;

typedef struct stru_reg_sfr_4byte {
    uint32_t Addr;
    uint8_t NumByte;
    uint32_t Value;

} PACKED RACE_REG_SFR_4BYTE_STRU, * PTR_RACE_REG_SFR_4BYTE_STRU;

typedef struct stru_reg_i2c {
    uint16_t Value;
    uint16_t Addr;
} PACKED RACE_REG_I2C_STRU, * PTR_RACE_REG_I2C_STRU;

//////////////////////////////////////////////////////////////////////////////
// FUNCTION DECLARATIONS /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
/**
 * RACE_WRITE_SFR_HDR
 *
 * WRITE SFR RACE COMMAND Handler
 *
 */
void *RACE_WRITE_SFR_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct stru_race_sfr_cmd {
        race_pkt_t Hdr;
        uint8_t NumSfr;
        RACE_REG_SFR_STRU WrSfr[0];
    } PACKED *PTR_THIS_RACE_CMD_HDR_STRU;

    typedef struct stru_race_sfr_evt {
        uint8_t Status;
        uint32_t ErrAddr;
    } PACKED THIS_RACE_EVT_HDR_STRU, *PTR_THIS_RACE_EVT_HDR_STRU;

    PTR_THIS_RACE_CMD_HDR_STRU pCmd = (PTR_THIS_RACE_CMD_HDR_STRU)pCmdMsg;

    PTR_THIS_RACE_EVT_HDR_STRU pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE, RACE_WRITE_SFR, sizeof(THIS_RACE_EVT_HDR_STRU), channel_id);

    if (pEvt != NULL) {
        uint8_t idx;
        uint32_t Addr;
        uint8_t NumByte;
        uint8_t *tmp_P = (uint8_t *)pCmd->WrSfr;

        pEvt->Status = RACE_ERRCODE_SUCCESS;

        pEvt->ErrAddr = 0;

        for (idx = 0; idx < pCmd->NumSfr; idx++) {
            Addr = ((PTR_RACE_REG_SFR_STRU)tmp_P)->Addr;
            NumByte = ((PTR_RACE_REG_SFR_STRU)tmp_P)->NumByte;

            if (NumByte == 1) {
                uint8_t *pDR1;
                PTR_RACE_REG_SFR_1BYTE_STRU ptr = (PTR_RACE_REG_SFR_1BYTE_STRU)tmp_P;

                pDR1 = (uint8_t *)ptr->Addr;
                *pDR1 = ptr->Value;
                tmp_P += sizeof(RACE_REG_SFR_1BYTE_STRU);
            } else if ((NumByte == 2) && !(Addr % 2)) {
                uint16_t *pDR2;
                PTR_RACE_REG_SFR_2BYTE_STRU ptr = (PTR_RACE_REG_SFR_2BYTE_STRU)tmp_P;

                pDR2 = (uint16_t *)ptr->Addr;
                *pDR2 = ptr->Value;
                tmp_P += sizeof(RACE_REG_SFR_2BYTE_STRU);
            } else if ((NumByte == 4) && !(Addr % 4)) {
                uint32_t *pDR4;
                PTR_RACE_REG_SFR_4BYTE_STRU ptrTmp = (PTR_RACE_REG_SFR_4BYTE_STRU)tmp_P;

                pDR4 = (uint32_t *)ptrTmp->Addr;
                *pDR4 = ptrTmp->Value;
                tmp_P += sizeof(RACE_REG_SFR_4BYTE_STRU);
            } else {
                pEvt->Status = RACE_ERRCODE_PARAMETER_ERROR;
                pEvt->ErrAddr = Addr;
                break;
            }
        }
    }
    return pEvt;
}

void *RACE_READ_SFR_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct stru_rd_sfr {
        uint32_t Addr;
        uint8_t NumByte;
    } PACKED RACE_RD_SFR_STRU;

    typedef struct stru_race_sfr_cmd {
        race_pkt_t Hdr;
        uint8_t NumSfr;
        RACE_RD_SFR_STRU RdSfr[0];
    } PACKED *PTR_THIS_RACE_CMD_HDR_STRU;

    typedef struct stru_race_sfr_evt {
        uint8_t Status;
        RACE_REG_SFR_STRU RegSfr[0];
    } PACKED *PTR_THIS_RACE_EVT_HDR_STRU;

    PTR_THIS_RACE_CMD_HDR_STRU pCmd = (PTR_THIS_RACE_CMD_HDR_STRU)(pCmdMsg);

    PTR_THIS_RACE_EVT_HDR_STRU pEvt = NULL;

    uint8_t TotalLen = 1;
    uint8_t *pNextPara;
    uint8_t idx;
    uint32_t Addr;
    uint8_t NumByte;

    for (idx = 0; idx < pCmd->NumSfr; idx++) {
        Addr = pCmd->RdSfr[idx].Addr;
        NumByte = pCmd->RdSfr[idx].NumByte;

        if (NumByte == 1) {
            TotalLen += sizeof(RACE_REG_SFR_1BYTE_STRU);
        } else if ((NumByte == 2) && !(Addr % 2)) {
            TotalLen += sizeof(RACE_REG_SFR_2BYTE_STRU);
        } else if ((NumByte == 4) && !(Addr % 4)) {
            TotalLen += sizeof(RACE_REG_SFR_4BYTE_STRU);
        } else {
            TotalLen += sizeof(pEvt->RegSfr[0].Addr);
        }


    }

    pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE, RACE_READ_SFR, TotalLen, channel_id);

    if (pEvt != NULL) {
        pEvt->Status = RACE_ERRCODE_SUCCESS;

        PTR_RACE_REG_SFR_STRU tmp_P = pEvt->RegSfr;

        for (idx = 0; idx < pCmd->NumSfr; idx++) {
            Addr = pCmd->RdSfr[idx].Addr;
            NumByte = pCmd->RdSfr[idx].NumByte;

            if (NumByte == 1) {
                uint8_t *pDR1;
                PTR_RACE_REG_SFR_1BYTE_STRU ptr = (PTR_RACE_REG_SFR_1BYTE_STRU)tmp_P;

                pDR1 = (uint8_t *)Addr;
                ptr->Value = *pDR1;
                ptr->Addr = Addr;
                ptr->NumByte = NumByte;

                pNextPara = (uint8_t *)tmp_P;
                pNextPara += sizeof(RACE_REG_SFR_1BYTE_STRU);
                tmp_P = (PTR_RACE_REG_SFR_STRU)pNextPara;
            } else if ((NumByte == 2) && !(Addr % 2)) {
                uint16_t *pDR2;
                PTR_RACE_REG_SFR_2BYTE_STRU ptr = (PTR_RACE_REG_SFR_2BYTE_STRU)tmp_P;

                pDR2 = (uint16_t *)Addr;
                ptr->Value = *pDR2;
                ptr->Addr = Addr;
                ptr->NumByte = NumByte;
                pNextPara = (uint8_t *)tmp_P;

                pNextPara += sizeof(RACE_REG_SFR_2BYTE_STRU);
                tmp_P = (PTR_RACE_REG_SFR_STRU)pNextPara;
            } else if ((NumByte == 4) && !(Addr % 4)) {
                uint32_t *pDR4;
                PTR_RACE_REG_SFR_4BYTE_STRU ptr = (PTR_RACE_REG_SFR_4BYTE_STRU)tmp_P;

                pDR4 = (uint32_t *)Addr;
                ptr->Value = *pDR4;
                ptr->Addr = Addr;
                ptr->NumByte = NumByte;

                pNextPara = (uint8_t *)tmp_P;
                pNextPara += sizeof(RACE_REG_SFR_4BYTE_STRU);
                tmp_P = (PTR_RACE_REG_SFR_STRU)pNextPara;
            } else {
                pEvt->Status = RACE_ERRCODE_PARAMETER_ERROR;
                pEvt->RegSfr[0].Addr = Addr;
                break;
            }
        }
    }
    return pEvt;
}

void *RACE_WRITE_REG_I2C_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct stru_race_wr_reg_i2c_cmd {
        race_pkt_t Hdr;
        uint8_t NumReg;
        RACE_REG_I2C_STRU RegI2C[0];
    } PACKED *PTR_THIS_RACE_WR_REG_I2C_CMD_HDR_STRU;

    typedef struct stru_race_wr_reg_i2c_evt {
        uint8_t Status;
    } PACKED THIS_RACE_WR_REG_I2C_EVT_HDR_STRU, * PTR_THIS_RACE_WR_REG_I2C_EVT_HDR_STRU;


    PTR_THIS_RACE_WR_REG_I2C_CMD_HDR_STRU pCmd = (PTR_THIS_RACE_WR_REG_I2C_CMD_HDR_STRU)pCmdMsg;

    PTR_THIS_RACE_WR_REG_I2C_EVT_HDR_STRU pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE, RACE_WRITE_REG_I2C, sizeof(THIS_RACE_WR_REG_I2C_EVT_HDR_STRU), channel_id);

    if (pEvt != NULL) {
        uint8_t idx;

        pEvt->Status = RACE_ERRCODE_SUCCESS;
        for (idx = 0; idx < pCmd->NumReg; idx++) {
#if ((PRODUCT_VERSION == 1552) || defined(AM255X))
            pmu_set_register_value_2byte_mt6388(pCmd->RegI2C[idx].Addr, 0xFFFF, 0, pCmd->RegI2C[idx].Value);
#else
            #ifdef HAL_PMU_MODULE_ENABLED
            pmu_set_register_value(pCmd->RegI2C[idx].Addr, 0xFFFF, 0, pCmd->RegI2C[idx].Value);
            #endif
#endif
        }
    }

    return pEvt;

}

void *RACE_READ_REG_I2C_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct stru_reg_i2c_read {
        uint16_t Addr;
    } PACKED RACE_REG_I2C_READ_ADDR_STRU;

    typedef struct stru_race_rd_reg_i2c_cmd {
        race_pkt_t Hdr;
        uint8_t NumReg;
        RACE_REG_I2C_READ_ADDR_STRU RegI2C[0];
    } PACKED *PTR_THIS_RACE_RD_REG_I2C_CMD_HDR_STRU;

    typedef struct stru_race_rd_reg_i2c_evt {
        uint8_t Status;
        RACE_REG_I2C_STRU RegI2C[0];
    } PACKED *PTR_THIS_RACE_RD_REG_I2C_EVT_HDR_STRU;

    uint8_t idx;
    uint8_t TotalLen = 1;

    PTR_THIS_RACE_RD_REG_I2C_CMD_HDR_STRU pCmd = (PTR_THIS_RACE_RD_REG_I2C_CMD_HDR_STRU)pCmdMsg;


    for (idx = 0; idx < pCmd->NumReg; idx++) {
        TotalLen += sizeof(RACE_REG_I2C_STRU);
    }
    PTR_THIS_RACE_RD_REG_I2C_EVT_HDR_STRU pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE, RACE_READ_REG_I2C, TotalLen, channel_id);

    if (pEvt != NULL) {
        for (idx = 0; idx < pCmd->NumReg; idx++) {
#if ((PRODUCT_VERSION == 1552) || defined(AM255X))
            pEvt->RegI2C[idx].Value = pmu_get_register_value_2byte_mt6388(pCmd->RegI2C[idx].Addr, 0xFFFF, 0);
#else
            #ifdef HAL_PMU_MODULE_ENABLED
            pEvt->RegI2C[idx].Value = (uint16_t)pmu_get_register_value(pCmd->RegI2C[idx].Addr, 0xFFFF, 0);
            #endif
#endif
            pEvt->RegI2C[idx].Addr = pCmd->RegI2C[idx].Addr;
#ifdef RACE_CMD_SYSTEM_LOG_ENABLE
            RACE_LOG_MSGID_I("RACE_READ_REG_I2C_HDR, addr[0x%x], value[0x%x]", 2, pEvt->RegI2C[idx].Addr, pEvt->RegI2C[idx].Value);
#endif
        }
        pEvt->Status = RACE_ERRCODE_SUCCESS;
    }

    return pEvt;
}
extern uint8_t sleep_manager_handle;

void *RACE_SLEEP_CONTROL_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct stru_race_sleep_control_cmd {
        race_pkt_t Hdr;
        uint8_t oper;
    } PACKED *PTR_THIS_RACE_CMD_HDR_STRU;

    typedef struct stru_race_sleep_control_evt {
        uint8_t status;
    } PACKED THIS_RACE_EVT_HDR_STRU, *PTR_THIS_RACE_EVT_HDR_STRU;

    PTR_THIS_RACE_CMD_HDR_STRU pCmd = (PTR_THIS_RACE_CMD_HDR_STRU)pCmdMsg;

    PTR_THIS_RACE_EVT_HDR_STRU pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE, RACE_SLEEP_CONTROL, sizeof(THIS_RACE_EVT_HDR_STRU), channel_id);

#ifdef HAL_SLEEP_MANAGER_ENABLED
    hal_sleep_manager_status_t status = HAL_SLEEP_MANAGER_ERROR;

    if (pEvt != NULL) {
        if (pCmd->oper == 0) {
            status = hal_sleep_manager_unlock_sleep(sleep_manager_handle);
        } else if (pCmd->oper == 1) {
            status = hal_sleep_manager_lock_sleep(sleep_manager_handle);
        }

        pEvt->status = status;
    }
#endif /* HAL_SLEEP_MANAGER_ENABLED */
    return pEvt;
}

void *RACE_CHARGER_ENABLE_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct stru_race_charger_enable_cmd {
        race_pkt_t Hdr;
        uint8_t enable;
    } PACKED *PTR_THIS_RACE_CMD_HDR_STRU;

    typedef struct stru_race_charger_enable_evt {
        uint8_t status;
    } PACKED THIS_RACE_EVT_HDR_STRU, *PTR_THIS_RACE_EVT_HDR_STRU;

    PTR_THIS_RACE_CMD_HDR_STRU pCmd = (PTR_THIS_RACE_CMD_HDR_STRU)pCmdMsg;

    if (pCmd) {
#ifdef RACE_CMD_SYSTEM_LOG_ENABLE
        RACE_LOG_MSGID_I("RACE_CHARGER_ENABLE_HDR, pCmd->enable[0x%X]", 1, pCmd->enable);
#endif
    } else {
#ifdef RACE_CMD_SYSTEM_LOG_ENABLE
        RACE_LOG_MSGID_E("RACE_CHARGER_ENABLE_HDR fail, pCmd is NULL", 0);
#endif
        return NULL;
    }

    PTR_THIS_RACE_EVT_HDR_STRU pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE, RACE_CHARGER_ENABLE, sizeof(THIS_RACE_EVT_HDR_STRU), channel_id);

    if (pEvt != NULL) {
#ifdef MTK_BATTERY_MANAGEMENT_ENABLE
        battery_set_enable_charger(pCmd->enable);
#ifdef RACE_CMD_SYSTEM_LOG_ENABLE
        RACE_LOG_MSGID_I("[BM Race]CHARGER_STATE;[0x%X]", 1, battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_STATE));
#endif
#else
        #ifdef HAL_PMU_MODULE_ENABLED
        pmu_enable_charger(pCmd->enable);
        #endif
#ifdef RACE_CMD_SYSTEM_LOG_ENABLE
        RACE_LOG_MSGID_I("MTK_BATTERY_MANAGEMENT_ENABLE not enable", 0);
#endif
#endif
        pEvt->status = RACE_ERRCODE_SUCCESS;
    }
    return pEvt;
}

#if defined(AIR_1WIRE_ENABLE)
void *RACE_SMCHG_CTRL_HDR(ptr_race_pkt_t pCmdMsg, uint8_t channel_id)
{
    typedef struct {
        race_pkt_t Hdr;
        uint8_t cmd_id;
        uint8_t cmd_data[4];
    } PACKED *PTR_THIS_RACE_CMD_HDR_STRU;

    typedef struct {
        uint8_t status;
        uint8_t evt_id;
        uint8_t evt_data[4];
    } PACKED THIS_RACE_EVT_HDR_STRU, *PTR_THIS_RACE_EVT_HDR_STRU;

    PTR_THIS_RACE_CMD_HDR_STRU pCmd = (PTR_THIS_RACE_CMD_HDR_STRU)pCmdMsg;

    if (pCmd->cmd_id >= RACE_SMCHG_MAX) {
#ifdef RACE_CMD_SYSTEM_LOG_ENABLE
        RACE_LOG_MSGID_E("RACE_SMCHG_CTRL_HDR fail, cmd_id[0x%X]", 1, pCmd->cmd_id);
#endif
        return NULL;
    }

    PTR_THIS_RACE_EVT_HDR_STRU pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE, RACE_SMCHG_CTRL, sizeof(THIS_RACE_EVT_HDR_STRU), channel_id);

    if (pEvt != NULL) {
        pEvt->status = RACE_ERRCODE_SUCCESS;
        pEvt->evt_id = pCmd->cmd_id;
        pEvt->evt_data[0] = 0x00;
        pEvt->evt_data[1] = 0x00;
        pEvt->evt_data[2] = 0x00;
        pEvt->evt_data[3] = 0x00;

        race_send_pkt_t *pSndPkt = race_pointer_cnv_pkt_to_send_pkt((void *)pEvt);
        uint8_t *ptr = (uint8_t *)&pSndPkt->race_data;
        uint32_t size = pSndPkt->length, ret_size = 0;
        uint32_t port_handle = race_get_port_handle_by_channel_id(channel_id);
#ifdef RACE_CMD_SYSTEM_LOG_ENABLE
        RACE_LOG_MSGID_E("RACE_SMCHG_CTRL_HDR, size[%d]", 1, size);
#endif
        uint32_t time_s = 0, time_e = 0;
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &time_s);
        do {
            ret_size = race_port_send_data(port_handle, ptr, size);
            size -= ret_size;
            ptr += ret_size;
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &time_e);
            if ((time_e - time_s) > 2000) {
                break;
            }
        } while (size > 0);
#ifdef RACE_CMD_SYSTEM_LOG_ENABLE
        RACE_LOG_MSGID_W("RACE_SMCHG_CTRL_HDR, time[%dus]", 1, (time_e - time_s));
#endif
        race_mem_free(pSndPkt);

        if (pCmd->cmd_id == RACE_SMCHG_LOG_MODE) {
            if (smchg_cfg.one_wire_log) {
                smchg_1wire_com_mode_tx_done();
                if (race_mode_flag) {
                    smchg_1wire_leave_race_mode();
                    race_mode_flag = 0;
                }
                smchg_1wire_init_to_log_mode();
                smchg_1wire_log_mode();
            }
        }
    }
    return NULL;
}
#endif

void *RACE_CmdHandler_System(ptr_race_pkt_t pRaceHeaderCmd, uint16_t Length, uint8_t channel_id)
{
    RACE_LOG_MSGID_I("RACE_CmdHandler_System, type[0x%X], cmd[0x%X], ch[%d]", 3,
                     pRaceHeaderCmd->hdr.type, pRaceHeaderCmd->hdr.id, channel_id);

    void *ptr = NULL;
    switch (pRaceHeaderCmd->hdr.id) {
        case RACE_WRITE_SFR : {
            ptr = RACE_WRITE_SFR_HDR(pRaceHeaderCmd, channel_id);
        }
        break;

        case RACE_READ_SFR : {
            ptr = RACE_READ_SFR_HDR(pRaceHeaderCmd, channel_id);
        }
        break;

        case RACE_WRITE_REG_I2C : {
            ptr = RACE_WRITE_REG_I2C_HDR(pRaceHeaderCmd, channel_id);
        }
        break;

        case RACE_READ_REG_I2C : {
            ptr = RACE_READ_REG_I2C_HDR(pRaceHeaderCmd, channel_id);
        }
        break;

        case RACE_SLEEP_CONTROL : {
            ptr = RACE_SLEEP_CONTROL_HDR(pRaceHeaderCmd, channel_id);
        }
        break;

        case RACE_CHARGER_ENABLE : {
            ptr = RACE_CHARGER_ENABLE_HDR(pRaceHeaderCmd, channel_id);
        }
        break;
#if defined(AIR_1WIRE_ENABLE)
        case RACE_SMCHG_CTRL : {
            ptr = RACE_SMCHG_CTRL_HDR(pRaceHeaderCmd, channel_id);
        }
        break;
#endif
        default: {
#ifdef RACE_CMD_SYSTEM_LOG_ENABLE
            RACE_LOG_MSGID_E("unknown system race cmd, 0x%X", 1, pRaceHeaderCmd->hdr.id);
#endif
        }
        break;
    }
    return ptr;
}

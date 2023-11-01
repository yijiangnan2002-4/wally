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
#include "nvdm.h"
#include "race_event.h"
#include "race_cmd_key_cmd.h"
#include "race_event_internal.h"
#include "atci_main.h"

#ifdef MTK_AWS_MCE_ENABLE
#include "race_cmd_relay_cmd.h"
#endif

#include "bt_connection_manager_internal.h"
#ifdef MTK_BT_DUO_ENABLE
#include "bt_sink_srv_music.h"
#endif

/***************************************************************/
/*                   Defines                                   */
/***************************************************************/
static uint8_t g_race_key_cmd_channel = 0;

#ifdef MTK_BT_DUO_ENABLE
RACE_ERRCODE race_key_remote_device_addr(bt_bd_addr_t *addr)
{
    RACE_ERRCODE ret = RACE_ERRCODE_FAIL;
    uint32_t count = 0;
    bt_bd_addr_t addr_list[BT_SINK_SRV_CM_MAX_DEVICE_NUMBER];
    count = bt_sink_srv_cm_get_connected_device((~BT_SINK_SRV_PROFILE_AWS), addr_list);
    //RACE_LOG_MSGID_I("[race_key_game_mode] connected_dev_num:%d \n", 1, count);
    if (count != 0) {
        memcpy(addr, addr_list[0], sizeof(bt_bd_addr_t));
        ret = RACE_ERRCODE_SUCCESS;
    }
    return ret;
}

RACE_ERRCODE race_key_game_mode_set(bt_sink_srv_music_mode_t mode)
{
    RACE_ERRCODE ret = RACE_ERRCODE_FAIL;
    bt_bd_addr_t addr;

    ret = race_key_remote_device_addr(&addr);
    if (ret == RACE_ERRCODE_SUCCESS) {
        bt_status_t bt_ret = bt_sink_srv_music_set_mode(&addr, mode);
        RACE_LOG_MSGID_I("[race_key_game_mode] set mode, bt_ret=0x%08X", 1, bt_ret);
        if (bt_ret != BT_STATUS_SUCCESS) {
            ret = RACE_ERRCODE_FAIL;
        }
    }

    return ret;
}

RACE_ERRCODE race_key_game_mode_toggle()
{
    RACE_ERRCODE ret = RACE_ERRCODE_FAIL;
    bt_bd_addr_t addr;
    bt_sink_srv_music_mode_t mode;

    ret = race_key_remote_device_addr(&addr);
    if (ret == RACE_ERRCODE_SUCCESS) {
        mode = bt_sink_srv_music_get_mode(&addr);
        RACE_LOG_MSGID_I("[race_key_game_mode] toggle from mode=%d", 1, mode);
        if (mode == BT_SINK_SRV_MUSIC_DATA_NOT_FOUND) {
            ret = RACE_ERRCODE_FAIL;
        } else if (mode == BT_SINK_SRV_MUSIC_GAME_MODE) {
            ret = bt_sink_srv_music_set_mode(&addr, BT_SINK_SRV_MUSIC_NORMAL_MODE);
        } else {
            ret = bt_sink_srv_music_set_mode(&addr, BT_SINK_SRV_MUSIC_GAME_MODE);
        }
    }
    return ret;
}
#endif

void *RACE_CmdHandler_KEY(ptr_race_pkt_t pCmdMsg, uint16_t Length, uint8_t channel_id)
{
    typedef struct {
        uint8_t status;
    } PACKED RSP;

    typedef struct {
        RACE_COMMON_HDR_STRU cmdhdr;
        uint16_t reset_id;
    } PACKED CMD;
    CMD *cmd = (CMD *)pCmdMsg;

    RSP *pEvt = RACE_ClaimPacket(RACE_TYPE_RESPONSE, pCmdMsg->hdr.id, sizeof(RSP), channel_id);
    if (pEvt == NULL) {
        RACE_LOG_MSGID_E("response alloc fail", 0);
        return NULL;
    }
    pEvt->status = 0xff;
    RACE_ERRCODE err = RACE_ERRCODE_SUCCESS;
    g_race_key_cmd_channel = channel_id;
    switch (pCmdMsg->hdr.id) {
        case RACE_ID_ATPORT_EVENT: {
            //RACE_LOG_MSGID_I("RACE_ID_ATPORT_EVENT[0x%X], change AT share port to %d", 2, pCmdMsg->hdr.id, cmd->reset_id);
#ifdef MTK_MUX_ENABLE
            if (atci_mux_port_reinit(cmd->reset_id, true, false) == ATCI_STATUS_OK) {
                pEvt->status = 0x00;
            }
#endif
            break;
        }

        case RACE_ID_KEY_EVENT: {
            RACE_LOG_MSGID_I("key event race, reset_id=%d", 1, cmd->reset_id);
            switch (cmd->reset_id) {
#ifdef MTK_BT_DUO_ENABLE
                case RACE_KEY_ID_GAMEMODE_ON:
                    //RACE_LOG_MSGID_I("race cmd gamemode on[0x%X]", 1, cmd->reset_id);
                    err = race_key_game_mode_set(BT_SINK_SRV_MUSIC_GAME_MODE);
                    if (err == RACE_ERRCODE_SUCCESS) {
                        pEvt->status = 0x00;
                    }
                    break;
                case RACE_KEY_ID_GAMEMODE_OFF:
                    //RACE_LOG_MSGID_I("race cmd gamemode off[0x%X]", 1, cmd->reset_id);
                    err = race_key_game_mode_set(BT_SINK_SRV_MUSIC_NORMAL_MODE);
                    if (err == RACE_ERRCODE_SUCCESS) {
                        pEvt->status = 0x00;
                    }
                    break;
                case RACE_KEY_ID_GAMEMODE_TOGGLE:
                    //RACE_LOG_MSGID_I("race cmd gamemode toggle[0x%X]", 1, cmd->reset_id);
                    err = race_key_game_mode_toggle();
                    if (err == RACE_ERRCODE_SUCCESS) {
                        pEvt->status = 0x00;
                    }
                    break;
#endif
                default: {
                    uint16_t *msg_data = pvPortMalloc(sizeof(uint16_t));
                    if (msg_data == NULL) {
                        RACE_LOG_MSGID_E("race cmd factory reset, msg_data malloc fail", 0);
                        break;
                    }
                    memset(msg_data, 0, sizeof(uint16_t));
                    *msg_data = cmd->reset_id;
                    err = race_send_event_notify_msg(RACE_EVENT_TYPE_KEY, (void *)msg_data);
                    if (err == RACE_ERRCODE_SUCCESS) {
                        pEvt->status = 0x00;
                    }
                }
                break;
            }
            break;
        }

        default:
            break;
    }

    return pEvt;
}

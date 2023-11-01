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
#ifdef RACE_BT_CMD_ENABLE
#include "FreeRTOS.h"
#include "timers.h"
#include "serial_port.h"
#include "bt_system.h"
#include "bt_callback_manager.h"
#include "bt_sink_srv.h"
#include "bt_gap_le.h"
#include "bt_hci.h"
#include "bt_connection_manager_internal.h"
#include "race_xport.h"
#include "race_bt.h"
#include "race_lpcomm_aws.h"
#include "race_lpcomm_util.h"
#include "race_lpcomm_trans.h"
#include "race_lpcomm_conn.h"
#include "race_event_internal.h"
#ifdef RACE_FOTA_CMD_ENABLE
#include "race_fota.h"
#include "race_fota_util.h"
#endif

#include "mux.h"

#define RACE_BT_BLE_ADV_TIMEOUT_IN_MS    (60000)


typedef struct {
    bt_bd_addr_t sp_bd_addr;
    bt_handle_t ble_conn_hdl;
    bt_handle_t hfp_hdl;
    bt_handle_t a2dp_hdl;
} race_bt_context_struct;


race_bt_context_struct g_race_bt_cntx, *g_race_bt_cntx_ptr;

static void race_bt_handle_power_on(bt_msg_type_t pmsg)
{
    /*bt_msg_type_t msg_type = 0;
    if (NULL == pmsg) {
        return;
    }
    msg_type = (bt_msg_type_t)pmsg->msg_data;*/
    RACE_LOG_MSGID_I("pmsg = %x", 1, pmsg);
    if (BT_POWER_ON_CNF == pmsg) {
        race_bt_init();
        race_serial_port_open(RACE_SERIAL_PORT_TYPE_SPP,
                              SERIAL_PORT_DEV_BT_SPP);
        race_serial_port_open(RACE_SERIAL_PORT_TYPE_BLE,
                              SERIAL_PORT_DEV_BT_LE);
        race_serial_port_open(RACE_SERIAL_PORT_TYPE_BLE_1,
                              SERIAL_PORT_DEV_BT_LE_1);
        race_serial_port_open(RACE_SERIAL_PORT_TYPE_BLE_2,
                              SERIAL_PORT_DEV_BT_LE_2);
#ifdef MTK_AIRUPDATE_ENABLE
        race_serial_port_open(RACE_SERIAL_PORT_TYPE_AIRUPDATE,
                              SERIAL_PORT_DEV_BT_AIRUPDATE);
#endif
#ifdef MTK_IAP2_PROFILE_ENABLE
        race_serial_port_open(RACE_SERIAL_PORT_TYPE_IAP2,
                              SERIAL_PORT_DEV_IAP2_SESSION2);
#endif
#ifdef MTK_GATT_OVER_BREDR_ENABLE
        race_serial_port_open(RACE_SERIAL_PORT_TYPE_GATT_OVER_BREDR,
                              SERIAL_PORT_DEV_BT_GATT_OVER_BREDR);
#endif
#ifdef AIR_MUX_BT_HID_ENABLE
        race_serial_port_open(RACE_SERIAL_PORT_TYPE_HID,
                              SERIAL_PORT_DEV_HID_CONTROL);
#endif
    } else if (BT_POWER_OFF_CNF == pmsg) {
        race_bt_deinit();
#ifdef RACE_AWS_ENABLE
        race_lpcomm_deattach_proc(NULL,
                                  0,
                                  RACE_LPCOMM_ROLE_AGENT,
                                  RACE_LPCOMM_TRANS_METHOD_AWS);

        race_lpcomm_deattach_proc(NULL,
                                  0,
                                  RACE_LPCOMM_ROLE_PARTNER,
                                  RACE_LPCOMM_TRANS_METHOD_AWS);
#endif
        race_serial_port_close(RACE_SERIAL_PORT_TYPE_SPP);
        race_serial_port_close(RACE_SERIAL_PORT_TYPE_BLE);
#ifdef MTK_AIRUPDATE_ENABLE
        race_serial_port_close(RACE_SERIAL_PORT_TYPE_AIRUPDATE);
#endif
#ifdef MTK_IAP2_PROFILE_ENABLE
        race_serial_port_close(RACE_SERIAL_PORT_TYPE_IAP2);
#endif
#ifdef MTK_GATT_OVER_BREDR_ENABLE
        race_serial_port_close(RACE_SERIAL_PORT_TYPE_GATT_OVER_BREDR);
#endif

#ifdef RACE_FOTA_CMD_ENABLE
        if (race_fota_is_race_fota_running()) {
#ifdef RACE_LPCOMM_ENABLE
            race_lpcomm_role_enum role = race_fota_get_role();
#endif
            race_fota_sp_trans_method_enum sp_trans_method = race_fota_sp_trans_method_get();

            if (sp_trans_method != RACE_FOTA_SP_TRANS_METHOD_SPP &&
                sp_trans_method != RACE_FOTA_SP_TRANS_METHOD_BLE &&
                sp_trans_method != RACE_FOTA_SP_TRANS_METHOD_AIRUPDATE &&
                sp_trans_method != RACE_FOTA_SP_TRANS_METHOD_IAP2 &&
                sp_trans_method != RACE_FOTA_SP_TRANS_METHOD_GATT_OVER_BREDR &&
                sp_trans_method != RACE_FOTA_SP_TRANS_METHOD_BLE_1 &&
                sp_trans_method != RACE_FOTA_SP_TRANS_METHOD_BLE_2) {
                RACE_LOG_MSGID_I("No need to stop fota because it is not transferred via bt. sp_trans_method:%d", 1, sp_trans_method);
                return;
            }

            /* Do not update sp_online or lpcomm_peer_online for it may be running in BT task. */
#ifdef RACE_LPCOMM_ENABLE
            if (RACE_LPCOMM_ROLE_PARTNER == role) {
                race_fota_stop(RACE_FOTA_STOP_ORIGINATOR_PARTNER,
                               RACE_FOTA_STOP_REASON_BT_OFF);
            } else
#endif
            {
                race_fota_stop(RACE_FOTA_STOP_ORIGINATOR_AGENT,
                               RACE_FOTA_STOP_REASON_BT_OFF);
            }
        }
#endif
    }
}


void race_bt_set_ble_conn_hdl(bt_handle_t conn_hdl)
{
    if (g_race_bt_cntx_ptr) {
        g_race_bt_cntx_ptr->ble_conn_hdl = conn_hdl;
    }
}


bt_handle_t race_bt_get_ble_conn_hdl(void)
{
    if (g_race_bt_cntx_ptr) {
        return g_race_bt_cntx_ptr->ble_conn_hdl;
    }

    return BT_HANDLE_INVALID;
}


void race_bt_set_a2dp_conn_hdl(bt_handle_t conn_hdl)
{
    if (g_race_bt_cntx_ptr) {
        g_race_bt_cntx_ptr->a2dp_hdl = conn_hdl;
    }
}


bt_handle_t race_bt_get_a2dp_conn_hdl(void)
{
    if (g_race_bt_cntx_ptr) {
        return g_race_bt_cntx_ptr->a2dp_hdl;
    }

    return BT_HANDLE_INVALID;
}



void race_bt_set_hfp_conn_hdl(bt_handle_t conn_hdl)
{
    if (g_race_bt_cntx_ptr) {
        g_race_bt_cntx_ptr->hfp_hdl = conn_hdl;
    }
}


bt_handle_t race_bt_get_hfp_conn_hdl(void)
{
    if (g_race_bt_cntx_ptr) {
        return g_race_bt_cntx_ptr->hfp_hdl;
    }

    return BT_HANDLE_INVALID;
}


bt_bd_addr_t *race_bt_get_sp_bd_addr(void)
{
    bt_bd_addr_t *local_addr  = NULL;
    bt_bd_addr_t addr_list[BT_SINK_SRV_CM_MAX_DEVICE_NUMBER];
    uint32_t i = 0, count = 0;

    if (!g_race_bt_cntx_ptr) {
        return NULL;
    }

    local_addr  = bt_connection_manager_device_local_info_get_local_address();
    memset(addr_list, 0, sizeof(bt_bd_addr_t) * BT_SINK_SRV_CM_MAX_DEVICE_NUMBER);
    count = bt_sink_srv_cm_get_connected_device(0, addr_list);
    for (i = 0; i < count; i++) {
        if (0 != memcmp((void *) & (addr_list[i]), (void *)local_addr, sizeof(bt_bd_addr_t))) {
            memcpy(&(g_race_bt_cntx_ptr->sp_bd_addr), addr_list[i], BT_BD_ADDR_LEN);
            break;
        }
    }

    if (i >= count) {
        RACE_LOG_MSGID_I("No SP paired.", 0);
        memset(&(g_race_bt_cntx_ptr->sp_bd_addr), 0, BT_BD_ADDR_LEN);
        return NULL;
    }
    return &(g_race_bt_cntx_ptr->sp_bd_addr);
}


void race_bt_init(void)
{
    RACE_LOG_MSGID_I("race_bt_init", 0);

    if (!g_race_bt_cntx_ptr) {
        g_race_bt_cntx_ptr = &g_race_bt_cntx;
    }

    memset(g_race_bt_cntx_ptr, 0, sizeof(race_bt_context_struct));
    g_race_bt_cntx_ptr->ble_conn_hdl = BT_HANDLE_INVALID;
    g_race_bt_cntx_ptr->hfp_hdl = BT_HANDLE_INVALID;
    g_race_bt_cntx_ptr->a2dp_hdl = BT_HANDLE_INVALID;
    //race_register_general_msg_hdl(MSG_ID_RACE_BT_POWER_ON_OFF_IND, race_bt_handle_power_on);
}


void race_bt_deinit(void)
{
    RACE_LOG_MSGID_I("race_bt_deinit", 0);

    if (g_race_bt_cntx_ptr) {
        g_race_bt_cntx_ptr = NULL;
    }
}


#ifdef RACE_BT_EVENT_REGISTER_ENABLE
#define RACE_BT_EVENT_REGISTER_ID_NUM (1)

#define RACE_BT_EVENT_REGISTER_ID_BASE (10)
/* Not reach max */
#define RACE_BT_EVENT_REGISTER_ID_MAX  (RACE_BT_EVENT_REGISTER_ID_BASE + RACE_BT_EVENT_REGISTER_ID_NUM)

typedef struct {
    bool is_used;
    uint8_t registered_event;
    uint8_t curr_event_status;
    race_bt_event_hdl hdl;
    void *user_data;
} race_bt_event_register_info_struct;


race_bt_event_register_info_struct g_race_bt_event_cntx[RACE_BT_EVENT_REGISTER_ID_NUM];

bool race_bt_event_is_valid_register_id(uint32_t register_id)
{
    if (register_id < RACE_BT_EVENT_REGISTER_ID_BASE) {
        register_id = RACE_BT_EVENT_REGISTER_ID_MAX;
    }

    register_id -= RACE_BT_EVENT_REGISTER_ID_BASE;

    if (RACE_BT_EVENT_REGISTER_ID_NUM > register_id && 0 <= register_id) {
        return TRUE;
    }

    return FALSE;
}


/* The hdl will only be called once when all the events registered occur.
  * After hdl is invoked, it will be deregistered automatically.
  * The user_data should be freed by the user.
  * race_bt_event_hdl hdl
  */
uint32_t race_bt_event_register(uint8_t event, race_bt_event_hdl hdl, void *user_data)
{
    int32_t i = 0;

    for (i = 0; i < RACE_BT_EVENT_REGISTER_ID_NUM; i++) {
        if (g_race_bt_event_cntx[i].is_used &&
            g_race_bt_event_cntx[i].hdl == hdl) {
            return RACE_BT_EVENT_REGISTER_ID_MAX;
        }
    }

    for (i = 0; i < RACE_BT_EVENT_REGISTER_ID_NUM; i++) {
        if (!g_race_bt_event_cntx[i].is_used) {
            g_race_bt_event_cntx[i].registered_event = event;
            g_race_bt_event_cntx[i].curr_event_status = event;
            g_race_bt_event_cntx[i].hdl = hdl;
            g_race_bt_event_cntx[i].user_data = user_data;
            g_race_bt_event_cntx[i].is_used = TRUE;
            break;
        }
    }

    return i + RACE_BT_EVENT_REGISTER_ID_BASE;
}


void race_bt_event_deregister(uint32_t register_id)
{
    if (register_id < RACE_BT_EVENT_REGISTER_ID_BASE) {
        register_id = RACE_BT_EVENT_REGISTER_ID_MAX;
    }

    register_id -= RACE_BT_EVENT_REGISTER_ID_BASE;

    if (RACE_BT_EVENT_REGISTER_ID_NUM > register_id &&
        g_race_bt_event_cntx[register_id].is_used) {
        g_race_bt_event_cntx[register_id].is_used = FALSE;
        if (g_race_bt_event_cntx[register_id].hdl) {
            g_race_bt_event_cntx[register_id].hdl(g_race_bt_event_cntx[register_id].registered_event,
                                                  RACE_BT_EVENT_REASON_DEREGISTER,
                                                  g_race_bt_event_cntx[register_id].user_data);
        }
    }
}


void race_bt_event_occur(uint8_t event)
{
    int32_t i = 0;

    for (i = 0; i < RACE_BT_EVENT_REGISTER_ID_NUM; i++) {
        if (g_race_bt_event_cntx[i].is_used) {
            g_race_bt_event_cntx[i].curr_event_status &= (~event);
            if ((!g_race_bt_event_cntx[i].curr_event_status) &&
                g_race_bt_event_cntx[i].hdl) {
                g_race_bt_event_cntx[i].is_used = FALSE;
                g_race_bt_event_cntx[i].hdl(g_race_bt_event_cntx[i].registered_event,
                                            RACE_BT_EVENT_REASON_EVENT_OCCUR,
                                            g_race_bt_event_cntx[i].user_data);
            }
        }
    }
}
#endif /* RACE_BT_EVENT_REGISTER_ENABLE */


/* Currently it's running in the BT ask. To run, need to define RACE_BT_EVENT_MSG_HDL. */
bt_status_t race_bt_event_process(bt_msg_type_t msg_type,
                                  bt_status_t status,
                                  void *buff)
{
    RACE_LOG_MSGID_I("msg_type:%x, status:%d, buff:%x", 3, msg_type, status, buff);
    switch (msg_type) {
        case BT_POWER_ON_CNF: {
            /*race_general_msg_t msg_item = {0};
            msg_item.msg_id = MSG_ID_RACE_BT_POWER_ON_OFF_IND;
            msg_item.msg_data = (uint8_t *)BT_POWER_ON_CNF;
            race_send_msg(&msg_item);*/
            race_bt_handle_power_on(msg_type);
            break;
        }

        case BT_POWER_OFF_CNF: {
            /*race_general_msg_t msg_item = {0};
            msg_item.msg_id = MSG_ID_RACE_BT_POWER_ON_OFF_IND;
            msg_item.msg_data = (uint8_t *)BT_POWER_OFF_CNF;
            race_send_msg(&msg_item);*/
            race_bt_handle_power_on(msg_type);
            break;
        }

        case BT_GAP_LE_CONNECT_IND: {
            bt_gap_le_connection_ind_t *connection_ind = (bt_gap_le_connection_ind_t *)buff;

            if (!connection_ind) {
                break;
            }

            if (BT_HANDLE_INVALID == race_bt_get_ble_conn_hdl()) {
                race_bt_set_ble_conn_hdl(connection_ind->connection_handle);
            }
            break;
        }


        case BT_GAP_LE_DISCONNECT_IND: {
            bt_hci_evt_disconnect_complete_t *connection_ind = (bt_hci_evt_disconnect_complete_t *)buff;

            if (!connection_ind) {
                break;
            }

            if (connection_ind->connection_handle != BT_HANDLE_INVALID &&
                connection_ind->connection_handle == race_bt_get_ble_conn_hdl()) {
                race_bt_set_ble_conn_hdl(BT_HANDLE_INVALID);
            }
            break;
        }
        case BT_A2DP_CONNECT_IND: {
            bt_a2dp_connect_ind_t *con_ind = (bt_a2dp_connect_ind_t *)buff;

            if (!con_ind) {
                break;
            }

            race_bt_set_a2dp_conn_hdl(con_ind->handle);
            break;
        }
        case BT_A2DP_CONNECT_CNF: {
            bt_a2dp_connect_cnf_t *con_cnf = (bt_a2dp_connect_cnf_t *)buff;

            if (!con_cnf) {
                break;
            }
            race_bt_set_a2dp_conn_hdl(con_cnf->handle);
            break;
        }
        case BT_A2DP_DISCONNECT_CNF: {
            bt_a2dp_disconnect_cnf_t *dis_cnf = (bt_a2dp_disconnect_cnf_t *)buff;

            if (!dis_cnf) {
                break;
            }

            race_bt_set_a2dp_conn_hdl(BT_HANDLE_INVALID);

#ifdef RACE_BT_EVENT_REGISTER_ENABLE
            race_bt_event_occur(RACE_BT_EVENT_TYPE_A2DP_DISC);
#endif
            break;
        }
        case BT_A2DP_DISCONNECT_IND: {
            bt_a2dp_disconnect_ind_t *dis_ind = (bt_a2dp_disconnect_ind_t *)buff;

            if (!dis_ind) {
                break;
            }
            race_bt_set_a2dp_conn_hdl(BT_HANDLE_INVALID);

#ifdef RACE_BT_EVENT_REGISTER_ENABLE
            race_bt_event_occur(RACE_BT_EVENT_TYPE_A2DP_DISC);
#endif
            break;
        }
        case BT_HFP_SLC_CONNECTED_IND: {
            bt_hfp_slc_connected_ind_t *conn_ind = (bt_hfp_slc_connected_ind_t *) buff;

            if (!conn_ind) {
                break;
            }
            race_bt_set_hfp_conn_hdl(conn_ind->handle);

#ifdef RACE_BT_EVENT_REGISTER_ENABLE
            race_bt_event_occur(RACE_BT_EVENT_TYPE_A2DP_DISC);
#endif
            break;
        }
        case BT_HFP_DISCONNECT_IND: {
            bt_hfp_disconnect_ind_t *conn_ind = (bt_hfp_disconnect_ind_t *) buff;

            if (!conn_ind) {
                break;
            }
            race_bt_set_hfp_conn_hdl(BT_HANDLE_INVALID);

#ifdef RACE_BT_EVENT_REGISTER_ENABLE
            race_bt_event_occur(RACE_BT_EVENT_TYPE_HFP_DISC);
#endif
            break;
        }
        case BT_GAP_READ_RAW_RSSI_CNF: {
#ifdef RACE_BT_CMD_ENABLE
            //race_bt_set_rssi(buff);
#endif
        }
        default:
            break;
    }

    return BT_STATUS_SUCCESS;
}


#ifdef RACE_BT_EVENT_MSG_HDL
bt_status_t race_bt_app_event_handler(bt_msg_type_t msg_type,
                                      bt_status_t status,
                                      void *buff)

{
    race_general_msg_t msg_queue_item = {0};
    race_bt_event_msg_info_struct *msg_info = NULL;
    uint8_t buff_len = 0;
    RACE_ERRCODE ret = RACE_ERRCODE_FAIL;

    RACE_LOG_MSGID_I("msg_type:%x, status:%d, buff:%x", 3, msg_type, status, buff);
    msg_queue_item.msg_id = MSG_ID_RACE_LOCAL_BT_EVENT_IND;
    switch (msg_type) {
        case BT_POWER_ON_CNF:
        case BT_POWER_OFF_CNF: {
            buff_len = 0;
            break;
        }

        case BT_GAP_LE_CONNECT_IND: {
            buff_len = sizeof(bt_gap_le_connection_ind_t);
            break;
        }

        case BT_GAP_LE_DISCONNECT_IND: {
            buff_len = sizeof(bt_hci_evt_disconnect_complete_t);
            break;
        }

        case BT_A2DP_CONNECT_IND: {
            buff_len = sizeof(bt_a2dp_connect_ind_t);
            break;
        }

        case BT_A2DP_CONNECT_CNF: {
            buff_len = sizeof(bt_a2dp_connect_cnf_t);
            break;
        }

        case BT_A2DP_DISCONNECT_CNF: {
            buff_len = sizeof(bt_a2dp_disconnect_cnf_t);
            break;
        }

        case BT_A2DP_DISCONNECT_IND: {
            buff_len = sizeof(bt_a2dp_disconnect_ind_t);
            break;
        }

        case BT_HFP_SLC_CONNECTED_IND: {
            buff_len = sizeof(bt_a2dp_disconnect_ind_t);
            break;
        }

        case BT_HFP_DISCONNECT_IND: {
            buff_len = sizeof(bt_hfp_disconnect_ind_t);
            break;
        }

        default:
            return BT_STATUS_SUCCESS;
    }

    if (!buff) {
        buff_len = 0;
    }

    msg_info = race_mem_alloc(sizeof(race_bt_event_msg_info_struct) + buff_len);
    if (!msg_info) {
        return BT_STATUS_SUCCESS;
    }

    msg_info->msg_type = msg_type;
    msg_info->status = status;
    msg_info->buff_len = buff_len;
    if (buff_len) {
        memcpy(msg_info->buff, buff, buff_len);
    }

    msg_queue_item.msg_data = msg_info;
    ret = race_send_msg(&msg_queue_item);
    if (RACE_ERRCODE_SUCCESS != ret) {
        race_mem_free(msg_info);
    }

    return BT_STATUS_SUCCESS;
}


void race_bt_event_ind_msg_process(race_bt_event_msg_info_struct *msg_info)
{
    if (!msg_info) {
        return;
    }

    if (msg_info->buff_len) {
        race_bt_event_process(msg_info->msg, msg_info->status, msg_info->buff);
    } else {
        race_bt_event_process(msg_info->msg, msg_info->status, NULL);
    }
}
#endif /* RACE_BT_EVENT_MSG_HDL */

extern bool race_bt_is_connected(mux_port_t dev);

#ifdef MTK_AWS_MCE_ENABLE
void race_bt_notify_aws_state(bool aws_state)
{
#ifdef MTK_PORT_SERVICE_BT_ENABLE

    RACE_ERRCODE ret;

    typedef struct {
        uint8_t channel_type;
        uint8_t channel_id;
    } PACKED NOTI;
#ifdef RACE_RELAY_CMD_ENABLE
        if(race_bt_is_connected(MUX_BT_BLE)) {
            RACE_LOG_MSGID_I("notify_aws_state: send by BLE", 0);
            NOTI *Noti = RACE_ClaimPacket((uint8_t)RACE_TYPE_NOTIFICATION,
                                          (uint16_t)RACE_CMDRELAY_GET_AVA_DST,
                                          (uint16_t)sizeof(NOTI),
                                          MUX_BT_BLE);
            if (Noti) {
                if (aws_state) {
                    Noti->channel_type = PARTENER_MATCHED_CHANNEL_TYPE;/*means agent is in ref*/
                    Noti->channel_id = PARTENER_MATCHED_CHANNEL_ID;
                } else {
                    Noti->channel_type = 0;
                    Noti->channel_id = 0;
                }
                ret = race_noti_send((uint8_t *)Noti, MUX_BT_BLE, false);
                if (ret != RACE_ERRCODE_SUCCESS) {
                    RACE_FreePacket((void *)Noti);
                    Noti = NULL;
                }
            }

        }
        if (race_bt_is_connected(MUX_BT_BLE_1)) {
            RACE_LOG_MSGID_I("notify_aws_state: send by BLE1", 0);
            NOTI *Noti = RACE_ClaimPacket((uint8_t)RACE_TYPE_NOTIFICATION,
                                          (uint16_t)RACE_CMDRELAY_GET_AVA_DST,
                                          (uint16_t)sizeof(NOTI),
                                          MUX_BT_BLE_1);
            if (Noti) {
                if (aws_state) {
                    Noti->channel_type = PARTENER_MATCHED_CHANNEL_TYPE;/*means agent is in ref*/
                    Noti->channel_id = PARTENER_MATCHED_CHANNEL_ID;
                } else {
                    Noti->channel_type = 0;
                    Noti->channel_id = 0;
                }
                ret = race_noti_send((uint8_t *)Noti, MUX_BT_BLE_1, false);
                if (ret != RACE_ERRCODE_SUCCESS) {
                    RACE_FreePacket((void *)Noti);
                    Noti = NULL;
                }
            }
        }
        if (race_bt_is_connected(MUX_BT_BLE_2)) {
            RACE_LOG_MSGID_I("notify_aws_state: send by BLE2", 0);
            NOTI *Noti = RACE_ClaimPacket((uint8_t)RACE_TYPE_NOTIFICATION,
                                          (uint16_t)RACE_CMDRELAY_GET_AVA_DST,
                                          (uint16_t)sizeof(NOTI),
                                          MUX_BT_BLE_2);
            if (Noti) {
                if (aws_state) {
                    Noti->channel_type = PARTENER_MATCHED_CHANNEL_TYPE;/*means agent is in ref*/
                    Noti->channel_id = PARTENER_MATCHED_CHANNEL_ID;
                } else {
                    Noti->channel_type = 0;
                    Noti->channel_id = 0;
                }
                ret = race_noti_send((uint8_t *)Noti, MUX_BT_BLE_2, false);
                if (ret != RACE_ERRCODE_SUCCESS) {
                    RACE_FreePacket((void *)Noti);
                    Noti = NULL;
                }
            }
        }
        if (race_bt_is_connected(MUX_BT_SPP)) {
            RACE_LOG_MSGID_I("notify_aws_state: send by SPP", 0);
            NOTI *Noti = RACE_ClaimPacket((uint8_t)RACE_TYPE_NOTIFICATION,
                                          (uint16_t)RACE_CMDRELAY_GET_AVA_DST,
                                          (uint16_t)sizeof(NOTI),
                                          MUX_BT_SPP);
            if (Noti) {
                if (aws_state) {
                    Noti->channel_type = PARTENER_MATCHED_CHANNEL_TYPE;/*means agent is in ref*/
                    Noti->channel_id = PARTENER_MATCHED_CHANNEL_ID;
                } else {
                    Noti->channel_type = 0;
                    Noti->channel_id = 0;
                }
                ret = race_noti_send((uint8_t *)Noti, MUX_BT_SPP, false);
                if (ret != RACE_ERRCODE_SUCCESS) {
                    RACE_FreePacket((void *)Noti);
                    Noti = NULL;
                }
            }
        }
#ifdef MTK_GATT_OVER_BREDR_ENABLE
        if (race_bt_is_connected(MUX_BT_GATT_OVER_BREDR)) {
            RACE_LOG_MSGID_I("notify_aws_state: send by GOB", 0);
            NOTI *Noti = RACE_ClaimPacket((uint8_t)RACE_TYPE_NOTIFICATION,
                                          (uint16_t)RACE_CMDRELAY_GET_AVA_DST,
                                          (uint16_t)sizeof(NOTI),
                                          MUX_BT_GATT_OVER_BREDR);
            if (Noti) {
                if (aws_state) {
                    Noti->channel_type = PARTENER_MATCHED_CHANNEL_TYPE;/*means agent is in ref*/
                    Noti->channel_id = PARTENER_MATCHED_CHANNEL_ID;
                } else {
                    Noti->channel_type = 0;
                    Noti->channel_id = 0;
                }
                ret = race_noti_send((uint8_t *)Noti, MUX_BT_GATT_OVER_BREDR, false);
                if (ret != RACE_ERRCODE_SUCCESS) {
                    RACE_FreePacket((void *)Noti);
                    Noti = NULL;
                }
            }
        }
#endif
#endif

#endif

}
#endif
#endif /* RACE_BT_CMD_ENABLE */


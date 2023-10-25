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

#include "race_xport.h"
#include "serial_port_assignment.h"
#include "race_noti.h"
#include "race_event_internal.h"
#include "race_storage_util.h"
#include "race_bt.h"
#include "race_lpcomm_recv.h"
#include "race_timer.h"
#include "race_fota_util.h"
#include "race_cmd_bluetooth.h"
#include "race_cfu_internal.h"
#include "race_lpcomm_ps_list.h"
#include "race_usb_relay.h"

#ifdef AIR_RACE_CO_SYS_ENABLE
#include "race_cmd_relay_cmd_cosys.h"
#include "race_cmd_nvdm.h"
#endif
#ifdef RACE_LPCOMM_RETRY_ENABLE
#include "race_lpcomm_retry.h"
#endif
/**************************************************************************************************
* Define
**************************************************************************************************/

#define RACE_MUX_TX_BUFFER_DEFAULT_SIZE     (1024)
#define RACE_MUX_RX_BUFFER_DEFAULT_SIZE     (1024)

/**************************************************************************************************
* Structure
**************************************************************************************************/

/* MSG_ID_RACE_LOCAL_WRITE_CMD_IND message content structure */
typedef struct {
    uint32_t output_len;
    uint8_t buff[0];
} race_send_output_cmd_msg_t;

typedef struct {
    race_port_type_t port_type;
    RACE_ERRCODE (*open_function)(race_port_t port_);
} race_port_open_function_t;

typedef struct {
    mux_port_t mux_id;
    serial_port_dev_t dev_id;
} race_mux_port_map_serial_dev_t;

/**************************************************************************************************
* Static Variable
**************************************************************************************************/

static const race_port_open_function_t g_race_port_open_tbl[] = {
    {RACE_PORT_TYPE_NORMAL_UART,    race_port_normal_uart_init},
#if defined (MTK_USB_DEMO_ENABLED)
    {RACE_PORT_TYPE_USB,            race_usb_port_init},
#endif
//    {RACE_PORT_TYPE_1WIRE_UART, race_init_1wire_port},
//    {RACE_PORT_TYPE_COSYS_UART, },
//    {RACE_PORT_TYPE_EMCU_UART, },
    {RACE_PORT_TYPE_BT_BLE, race_port_bt_open},
//    {RACE_PORT_TYPE_AWS_MCE, },
    {RACE_PORT_TYPE_IAP2, race_port_bt_open},
    {RACE_PORT_TYPE_BT_HID, race_port_bt_open},
//    {RACE_PORT_TYPE_LL_UART, },
//    {RACE_PORT_TYPE_PSEUDO, },
};

static const race_mux_port_map_serial_dev_t g_port_map_tbl[] = {
    {MUX_UART_0, SERIAL_PORT_DEV_UART_0},
    {MUX_UART_1, SERIAL_PORT_DEV_UART_1},
    {MUX_UART_2, SERIAL_PORT_DEV_UART_2},
    {MUX_UART_3, SERIAL_PORT_DEV_UART_3},

    {MUX_USB_COM_1, SERIAL_PORT_DEV_USB_COM1},
    {MUX_USB_COM_2, SERIAL_PORT_DEV_USB_COM2},

#if defined(MTK_MUX_BT_ENABLE) && defined(MTK_PORT_SERVICE_BT_ENABLE)
    {MUX_BT_SPP, SERIAL_PORT_DEV_BT_SPP},
    {MUX_BT_BLE, SERIAL_PORT_DEV_BT_LE},
    {MUX_BT_BLE_1, SERIAL_PORT_DEV_BT_LE_1},
    {MUX_BT_BLE_2, SERIAL_PORT_DEV_BT_LE_2},

#if defined(MTK_GATT_OVER_BREDR_ENABLE) && defined (MTK_GATT_OVER_BREDR_ENABLE)
    {MUX_BT_GATT_OVER_BREDR, SERIAL_PORT_DEV_BT_GATT_OVER_BREDR},
#endif

#if defined(MTK_AIRUPDATE_ENABLE)
    {MUX_BT_AIRUPATE, SERIAL_PORT_DEV_BT_AIRUPDATE},
#endif
#endif

#if defined(MTK_IAP2_VIA_MUX_ENABLE) && defined (MTK_IAP2_PROFILE_ENABLE)
    {MUX_IAP2_SESSION2, SERIAL_PORT_DEV_IAP2_SESSION2},
#endif

#if defined(AIR_RACE_SCRIPT_ENABLE)
    {MUX_PORT_PSEUDO, SERIAL_PORT_DEV_PSEUDO},
#endif

#if defined(AIR_LOW_LATENCY_MUX_ENABLE)
    {MUX_LL_UART_0, SERIAL_PORT_DEV_LL_UART_0},
    {MUX_LL_UART_1, SERIAL_PORT_DEV_LL_UART_1},
    {MUX_LL_UART_2, SERIAL_PORT_DEV_LL_UART_2},
#endif

#if defined(AIR_MUX_BT_HID_ENABLE)
    {MUX_HID_CONTROL, SERIAL_PORT_DEV_HID_CONTROL},
    {MUX_HID_INTERUPT, SERIAL_PORT_DEV_HID_INTERUPT},
#endif
};


#ifdef AIR_RACE_CO_SYS_ENABLE
extern bool race_relay_send_cosys(race_pkt_t *race_pkt, uint16_t length, uint8_t channel_id, uint8_t relay_type);
#endif

/**************************************************************************************************
* Static Functions
**************************************************************************************************/

static RACE_ERRCODE race_open_default_port(race_port_t port, race_port_type_t port_type)
{
    RACE_ERRCODE res;
    uint16_t i;
    race_port_init_t port_config = {0};
    race_user_config_t race_user_config = {0};
    mux_port_setting_t setting = {
        .tx_buffer_size = RACE_MUX_TX_BUFFER_DEFAULT_SIZE,
        .rx_buffer_size = RACE_MUX_RX_BUFFER_DEFAULT_SIZE};
    uint16_t func_cnt = sizeof (g_race_port_open_tbl) / sizeof(g_race_port_open_tbl[0]);

    for (i = 0; i < func_cnt; i++) {
        if (port_type == g_race_port_open_tbl[i].port_type) {
            return g_race_port_open_tbl[i].open_function(port);
        }
    }

    if (race_get_port_type(port) != port_type) {
        return RACE_ERRCODE_PARAMETER_ERROR;
    }

    memset(&port_config, 0, sizeof(race_port_init_t));
    port_config.port = port;
    port_config.port_type = port_type;
    port_config.port_settings = &setting;

    res = race_init_port(&port_config);
    if (RACE_ERRCODE_SUCCESS != res) {
        return res;
    }

    memset(&race_user_config, 0, sizeof(race_user_config_t));
    race_user_config.port = port;
    race_user_config.port_type = port_type;
    race_user_config.user_name = NULL;
    res = race_open_port(&race_user_config);

    return res;
}

static void race_init_default_port(void)
{
    race_port_t race_port;
    mux_status_t status;
    mux_port_buffer_t query_port_buffer;
    query_port_buffer.count = 0;
    status = mux_query_port_numbers_from_nvdm("SYSLOG", (mux_port_buffer_t *)&query_port_buffer);

    if ((status == MUX_STATUS_OK) && (query_port_buffer.count == 1)) {
#ifndef MTK_USB_AUDIO_HID_ENABLE
        if (query_port_buffer.buf[0] == MUX_USB_COM_1) {
            race_port = MUX_USB_COM_2;
        } else if (query_port_buffer.buf[0] == MUX_USB_COM_2) {
            race_port = MUX_USB_COM_1;
        } else
#endif
        {
            race_port = query_port_buffer.buf[0];
        }
    } else {
        race_port = CONFIG_RACE_PORT;
    }
#ifdef AIR_UART1_EARLY_LOG_ENABLE
    RACE_LOG_MSGID_I("race_init, AIR_UART1_EARLY_LOG_ENABLE == 1, config race port to uart1", 0);
    race_port = MUX_UART_1;
#endif
    mux_open_save_to_nvdm(race_port, "RACE_CMD");

    race_open_default_port(race_port, race_get_port_type(race_port));

    /* init usb hid default port */
    if (false == RACE_PORT_IS_USB(race_port)){
#ifdef MTK_USB_AUDIO_HID_ENABLE
        race_port = MUX_USB_COM_2;
        race_open_default_port(race_port, RACE_PORT_TYPE_USB);
#elif (defined CONFIG_RACE_PORT_USB)
        if (RACE_PORT_IS_USB(CONFIG_RACE_PORT_USB)) {
            race_port = CONFIG_RACE_PORT_USB;
            race_open_default_port(race_port, RACE_PORT_TYPE_USB);
        }
#endif
    }

}

/*
* p_msg->dev_t: MUX port to send
* p_msg->msg_data: alloced pointer of race_send_output_cmd_msg_t
*/
static void race_handle_output_data(race_general_msg_t *p_msg)
{
    if (NULL == p_msg) {
        return;
    }
    race_send_output_cmd_msg_t *msg_data = (race_send_output_cmd_msg_t *)p_msg->msg_data;
    uint32_t port_handle = race_get_tx_handle_by_port((race_port_t)p_msg->dev_t);
    race_port_send_data_imp(port_handle, msg_data->buff, msg_data->output_len);
    race_mem_free(p_msg->msg_data);
}


#ifdef RACE_STORAGE_CMD_ENABLE
static void race_storage_nb_sha256_generate_continue_handler(race_general_msg_t *p_msg)
{
    race_storage_nb_sha256_generate_continue_msg_process((void *)p_msg->msg_data);
}
#endif

#ifdef RACE_BT_EVENT_MSG_HDL
static void race_bt_event_ind_handler(race_general_msg_t *p_msg)
{
    race_bt_event_ind_msg_process((race_bt_event_msg_info_struct *)p_msg->msg_data);
    race_mem_free(p_msg->msg_data);
}
#endif

static void race_timer_expiration_handler(race_general_msg_t *p_msg)
{
    race_timer_expiration_msg_process();
}

#ifdef RACE_BT_CMD_ENABLE
static void race_bt_notify_rssi_handler(race_general_msg_t *p_msg)
{
    race_bt_notify_rssi();
}
#endif

#ifdef RACE_LPCOMM_ENABLE
static void race_lpcomm_data_handler(race_general_msg_t *p_msg)
{
    race_lpcomm_data_recv_msg_process(p_msg);
    if (p_msg && p_msg->msg_data) {
        race_mem_free(p_msg->msg_data);
    }
}
#endif

#ifdef RACE_FOTA_CMD_ENABLE
static void race_fota_stop_handler(race_general_msg_t *p_msg)
{
    race_fota_stop_msg_process(p_msg);
    if (p_msg && p_msg->msg_data) {
        race_mem_free(p_msg->msg_data);
    }
}
#endif

static void race_default_init(void)
{
    race_register_general_msg_hdl(MSG_ID_SEND_PKT_BY_TASK, race_handle_output_data);
    race_register_general_msg_hdl(MSG_ID_RACE_LOCAL_DELAY_NOTI_IND, race_noti_delay_msg_process);
    race_register_general_msg_hdl(MSG_ID_RACE_LOCAL_EVENT_NOTIFY_REQ, race_event_notify_msg_process);

#ifdef RACE_STORAGE_CMD_ENABLE
    race_register_general_msg_hdl(MSG_ID_RACE_LOCAL_ERASE_PARTITION_CONTINUE_IND,
        (race_general_msg_hdl_t)race_storage_erase_partition_continue_msg_process);
    race_register_general_msg_hdl(MSG_ID_RACE_LOCAL_STORAGE_SHA256_GENERATE_CONTINUE_IND,
        race_storage_nb_sha256_generate_continue_handler);
#endif

#ifdef RACE_BT_EVENT_MSG_HDL
    race_register_general_msg_hdl(MSG_ID_RACE_LOCAL_BT_EVENT_IND, race_bt_event_ind_handler);
#endif

#ifdef RACE_LPCOMM_ENABLE
    race_register_general_msg_hdl(MSG_ID_RACE_LOCAL_LPCOMM_DATA_RECV_IND, race_lpcomm_data_handler);
#endif

    race_register_general_msg_hdl(MSG_ID_RACE_LOCAL_TIMER_EXPIRATION_IND, race_timer_expiration_handler);

#ifdef RACE_FOTA_CMD_ENABLE
    race_register_general_msg_hdl(MSG_ID_RACE_LOCAL_FOTA_STOP_IND, race_fota_stop_handler);
#endif

#if (defined RACE_RELAY_CMD_ENABLE) || (defined AIR_RACE_CO_SYS_ENABLE)
    race_register_general_msg_hdl(MSG_ID_RACE_LOCAL_RELAY_RACE_CMD, (race_general_msg_hdl_t)race_cmd_relay_aws_mce_msg_process);
#endif

#ifdef RACE_BT_CMD_ENABLE
    race_register_general_msg_hdl(MSG_ID_RACE_LOCAL_GET_RSSI_CMD, race_bt_notify_rssi_handler);
#endif

#ifdef RACE_CFU_ENABLE
    race_register_general_msg_hdl(MSG_ID_RACE_LOCAL_CFU_IND, (race_general_msg_hdl_t)race_cfu_msg_processer);
#endif

    race_init_default_port();

#if defined (MTK_USB_DEMO_ENABLED)
    race_usb_init();
#endif

#if defined (AIR_1WIRE_ENABLE)
    race_1wire_local_init();
#endif

}

/**************************************************************************************************
* Public Functions
**************************************************************************************************/

race_port_type_t race_get_port_type(race_port_t port)
{
    if (RACE_PORT_IS_UART(port)) {
        return RACE_PORT_TYPE_NORMAL_UART;
    } else if (RACE_PORT_IS_USB(port)) {
        return RACE_PORT_TYPE_USB;

#if defined(MTK_MUX_BT_ENABLE)
    } else if (MUX_BT_BEGIN <= port && MUX_BT_END >= port) {
        return RACE_PORT_TYPE_BT_BLE;
#endif

#if defined(MTK_MUX_AWS_MCE_ENABLE)
    } else if (MUX_AWS_MCE_BEGIN <= port && MUX_AWS_MCE_END >= port) {
        return RACE_PORT_TYPE_AWS_MCE;
#endif

#if defined(MTK_IAP2_VIA_MUX_ENABLE)
    } else if (MUX_IAP2_BEGIN <= port && MUX_IAP2_END >= port) {
        return RACE_PORT_TYPE_IAP2;
#endif

#if defined(AIR_RACE_SCRIPT_ENABLE)
    } else if (MUX_PORT_PSEUDO == port) {
        return RACE_PORT_TYPE_PSEUDO;
#endif

#if defined(AIR_LOW_LATENCY_MUX_ENABLE)
    } else if (MUX_LL_UART_BEGIN <= port && MUX_LL_UART_END >= port) {
        return RACE_PORT_TYPE_LL_UART;
#endif

#if defined(AIR_MUX_BT_HID_ENABLE)
    } else if (MUX_HID_BEGIN <= port && MUX_HID_END >= port) {
        return RACE_PORT_TYPE_LL_UART;
#endif

    }
    return RACE_PORT_TYPE_INVALID;
}

race_status_t race_flush_packet(uint8_t *ptr, uint8_t channel_id)
{
    race_status_t ret = RACE_STATUS_ERROR;
    race_send_pkt_t *pData = NULL;

    RACE_LOG_MSGID_I("race_flush_packet, ptr[0x%X] channel_id[%d]", 2, ptr, channel_id);

    if (ptr != NULL) {
        pData = race_pointer_cnv_pkt_to_send_pkt((void *)ptr);

#ifdef AIR_RACE_CO_SYS_ENABLE
        if (RACE_CHANNEL_ID_IS_RELAY_CMD_FLAG_SET(channel_id)) {
            bool ret_val = FALSE;

            channel_id = RACE_CHANNEL_ID_CLEAR_RELAY_CMD_FLAG(channel_id);
            ret_val = race_relay_send_cosys(&pData->race_data, pData->length, channel_id, RACE_CMD_RSP_FROM_PARTNER);
            race_mem_free(pData);
            return ret_val ? RACE_STATUS_OK : RACE_STATUS_ERROR;
        }
#elif (defined RACE_RELAY_CMD_ENABLE)
        if (RACE_CHANNEL_ID_IS_RELAY_CMD_FLAG_SET(channel_id)) {
            bt_status_t ret_val;

            channel_id = RACE_CHANNEL_ID_CLEAR_RELAY_CMD_FLAG(channel_id);
            ret_val = bt_send_aws_mce_race_cmd_data(&pData->race_data, pData->length, channel_id, RACE_CMD_RSP_FROM_PARTNER, 0);
            race_mem_free(pData);
            return (ret_val == BT_STATUS_SUCCESS) ? RACE_STATUS_OK : RACE_STATUS_ERROR;
        }
#endif

        //race_port_send_data(port_handle, (uint8_t*)&pData->race_data, pData->length);

        /* BTA-19762, fix dead lock. Change to send in race task. */
        race_general_msg_t msg_queue_item = {0};
        msg_queue_item.msg_id = MSG_ID_SEND_PKT_BY_TASK;
        msg_queue_item.dev_t = RACE_CHANNEL_ID_CLEAR_RELAY_CMD_FLAG(channel_id);
        msg_queue_item.msg_data = race_mem_alloc(sizeof(uint32_t) + pData->length);  // race_send_output_cmd_msg_t
        race_send_output_cmd_msg_t *data = (race_send_output_cmd_msg_t *)msg_queue_item.msg_data;
        if (data == NULL) {
            RACE_LOG_MSGID_E("race_flush_packet, alloc fail", 0);
            return RACE_STATUS_ERROR;
        }
        data->output_len = pData->length;
        memcpy(data->buff, (uint8_t *)&pData->race_data, pData->length);
        race_send_msg(&msg_queue_item);

#if (RACE_DEBUG_PRINT_ENABLE)
        race_dump((uint8_t *)&pData->race_data, RACE_DBG_FLUSH);
#endif
        ret = RACE_STATUS_OK;
        race_mem_free(pData);
    }

    return ret;
}

/*
* this function is for controller
*/
race_status_t race_flush_packet_relay(race_pkt_t *race_pkt, uint16_t length, uint8_t channel_id, uint8_t type, uint8_t send_idx)
{
#ifdef RACE_RELAY_CMD_ENABLE
    bt_send_aws_mce_race_cmd_data(race_pkt, length, channel_id, type, send_idx);
#endif

    return RACE_STATUS_OK;
}

uint32_t race_port_send_data(uint32_t port_handle, uint8_t *buf, uint32_t buf_len)
{
#ifdef AIR_RACE_CO_SYS_ENABLE

#if defined (AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined (AIR_DCHS_MODE_MASTER_ENABLE)
    if (port_handle == race_cosys_get_handle() || port_handle == race_emcu_get_handle())
#else
    if (port_handle == race_cosys_get_handle())
#endif
    {
        race_general_msg_t msg_queue_item = {0};
        msg_queue_item.msg_id = MSG_ID_SEND_PKT_BY_TASK;
#if defined (AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined (AIR_DCHS_MODE_MASTER_ENABLE)
        msg_queue_item.dev_t = (port_handle == race_cosys_get_handle()) ? race_cosys_get_port() : race_emcu_get_port();
#else
        msg_queue_item.dev_t = race_cosys_get_port();
#endif
        msg_queue_item.msg_data = race_mem_alloc(sizeof(uint32_t) + buf_len);
        race_send_output_cmd_msg_t *data = (race_send_output_cmd_msg_t *)msg_queue_item.msg_data;
        if (data == NULL) {
            RACE_LOG_MSGID_E("race_port_send_data alloc fail", 0);
            return 0;
        }
        data->output_len = buf_len;
        memcpy(data->buff, buf, buf_len);
        race_send_msg(&msg_queue_item);
        return buf_len;
    }
#endif

    return race_port_send_data_imp(port_handle, buf, buf_len);
}

uint32_t race_get_port_handle_by_channel_id(uint8_t channel_id)
{
    race_port_t port = (race_port_t)RACE_CHANNEL_ID_CLEAR_RELAY_CMD_FLAG(channel_id);
    return (uint32_t)race_get_tx_handle_by_port(port);
}


/*
* For APP, USB flag means device id.
*/
uint8_t race_get_device_id_by_conn_address(bt_bd_addr_t *peer_address)
{
#if defined RACE_USB_RELAY_ENABLE
    uint8_t port = race_get_channel_id_by_conn_address(peer_address);
    if (port == RACE_INVALID_CHANNEL_ID) {
        return RACE_TARGET_INVALID_DEVICE;
    }
    return race_relay_get_usb_tx_flag_by_src_port((race_port_t)port);
#else
    return RACE_TARGET_INVALID_DEVICE;
#endif
    return RACE_TARGET_INVALID_DEVICE;
}

/**************************************************************************************************
* work around for old api
**************************************************************************************************/

static mux_port_t race_muxid_from_portid(serial_port_dev_t port_id)
{
    uint16_t len = sizeof(g_port_map_tbl) / sizeof(g_port_map_tbl[0]);
    uint16_t i;
    for (i = 0; i < len; i++) {
        if (g_port_map_tbl[i].dev_id == port_id) {
            return g_port_map_tbl[i].mux_id;
        }
    }
    return 0xFF;
}

static serial_port_dev_t race_portid_from_muxid(mux_port_t mux_id)
{
    uint16_t len = sizeof(g_port_map_tbl) / sizeof(g_port_map_tbl[0]);
    uint16_t i;
    for (i = 0; i < len; i++) {
        if (g_port_map_tbl[i].mux_id == mux_id) {
            return g_port_map_tbl[i].dev_id;
        }
    }
    return SERIAL_PORT_DEV_UNDEFINED;
}

race_status_t race_serial_port_open(race_serial_port_type_enum port_type, serial_port_dev_t port_num)
{
    race_port_t port = race_muxid_from_portid(port_num);
    race_port_type_t type = race_get_port_type(port);
    if (race_open_default_port(port, type) == RACE_ERRCODE_SUCCESS) {
        return RACE_STATUS_OK;
    } else {
        return RACE_STATUS_ERROR;
    }
}

race_status_t race_serial_port_close(race_serial_port_type_enum port_type)
{
    race_port_t port = (race_port_t)port_type;
    if (port != RACE_INVALID_PORT) {
        race_close_port_for_all_user(port);
        return RACE_STATUS_OK;
    } else {
        return RACE_STATUS_ERROR;
    }
}

race_serial_port_type_enum race_get_port_type_by_channel_id(uint8_t channel_id)
{
    race_serial_port_type_enum port_type = (race_serial_port_type_enum)RACE_CHANNEL_ID_CLEAR_RELAY_CMD_FLAG(channel_id);
    if (channel_id != RACE_SERIAL_PORT_TYPE_NONE) {
        return port_type;
    } else {
        return RACE_SERIAL_PORT_TYPE_NONE;
    }
}

uint8_t race_get_channel_id_by_port_type(race_serial_port_type_enum port_type)
{
    return port_type;
}

uint32_t race_get_serial_port_handle(serial_port_dev_t port)
{
    race_port_t race_port = race_muxid_from_portid(port);
    return race_get_tx_handle_by_port(race_port);
}

race_serial_port_type_enum race_get_channel_id_by_port_handle(uint32_t port_handle)
{
    return (race_serial_port_type_enum)RACE_GET_PORT_BY_MUX_HANDLE(port_handle);
}

race_status_t race_init_port_service(serial_port_dev_t port)
{
    race_port_t race_port = race_muxid_from_portid(port);
    race_port_type_t type = race_get_port_type(race_port);
    if (race_open_default_port(race_port, type) == RACE_ERRCODE_SUCCESS) {
        return RACE_STATUS_OK;
    } else {
        return RACE_STATUS_ERROR;
    }
}

/**************************************************************************************************
* work around api for switch
**************************************************************************************************/

serial_port_dev_t race_switch_get_active_uart_port(void)
{
    race_port_t port = race_uart_get_active_port();
    if(RACE_INVALID_PORT != port) {
        return race_portid_from_muxid(port);
    }
    return SERIAL_PORT_DEV_UNDEFINED;
}

serial_port_dev_t race_switch_get_uart_default_port(void)
{
    return SERIAL_PORT_DEV_UART_1;
}

race_status_t race_serial_port_uart_init(serial_port_dev_t port)
{
    if (race_open_default_port(race_muxid_from_portid(port), RACE_PORT_TYPE_NORMAL_UART) == RACE_ERRCODE_SUCCESS) {
        return RACE_STATUS_OK;
    }
    return RACE_STATUS_ERROR;
}

race_status_t race_uart_deinit(void)
{
    // old api, just for build pass
    return RACE_STATUS_OK;
}


/**************************************************************************************************
* Task Functions
**************************************************************************************************/

void race_init(void)
{
    RACE_LOG_MSGID_I("race_init", 0);
    race_core_init();
    race_default_init();

#ifdef RACE_LPCOMM_ENABLE
    race_lpcomm_ps_list_init();

#ifdef RACE_LPCOMM_RETRY_ENABLE
    race_lpcomm_retry_init();
#endif
#endif

    race_event_init();
    race_port_bt_init();

#ifdef RACE_FOTA_CMD_ENABLE
    race_fota_init();
#endif

#ifdef AIR_RACE_CO_SYS_ENABLE
    race_cosys_init();
#endif

#if (defined RACE_RELAY_CMD_ENABLE) || (defined AIR_RACE_CO_SYS_ENABLE)
    race_relay_cmd_init();
#endif

#ifdef RACE_USB_RELAY_ENABLE
    race_usb_relay_init();
#endif
}


#ifdef RACE_CREATE_TASK_DYNAMICALLY

TaskHandle_t g_race_task_hdl;

bool race_is_task_existed(void)
{
    return g_race_task_hdl != NULL;
}

void race_create_task(void)
{
#ifdef MTK_AUDIO_TUNING_ENABLED
#define RACE_TASK_STACKSIZE         (1506) /*unit byte!*/
#else
#define RACE_TASK_STACKSIZE         (512) /*unit byte!*/
#endif

#define RACE_TASK_PRIO              (TASK_PRIORITY_NORMAL)
#define RACE_TASK_NAME              ("race command")

    BaseType_t ret = xTaskCreate(race_task,
                                 RACE_TASK_NAME,
                                 RACE_TASK_STACKSIZE,
                                 NULL,
                                 RACE_TASK_PRIO,
                                 &g_race_task_hdl);

    RACE_LOG_MSGID_E("xCreate race task, priority:%d, ret:%d, task hdl:0x%x", 3, RACE_TASK_PRIO, ret, (uint32_t)g_race_task_hdl);
}
#endif


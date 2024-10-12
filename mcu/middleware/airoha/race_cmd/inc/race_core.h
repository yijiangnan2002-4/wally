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
/* Airoha restricted information */


#ifndef RACE_CORE_H
#define RACE_CORE_H


#include "mux.h"
#include "race_cmd.h" // should include before race.h
#include "race_util.h"
#include "race.h"
#include <string.h>  /* for memset, memcpy */

#ifdef __cplusplus
extern "C"
{
#endif


/**************************************************************************************************
* LOG API
**************************************************************************************************/

#define RACE_LOG_F(fmt,arg...)                  LOG_I(race, fmt,##arg)
#define RACE_LOG_I(fmt,arg...)                  LOG_I(race, fmt,##arg)
#define RACE_LOG_W(fmt,arg...)                  LOG_W(race, fmt,##arg)
#define RACE_LOG_E(fmt,arg...)                  LOG_E(race, fmt,##arg)
//#define RACE_LOG_D(fmt,arg...)                LOG_E(race, fmt,##arg)
#define RACE_LOG_D(fmt,arg...)

#define RACE_LOG_MSGID_D(fmt,cnt,arg...)        LOG_MSGID_I(race,fmt,cnt,##arg)
#define RACE_LOG_MSGID_F(fmt,cnt,arg...)        LOG_MSGID_I(race,fmt,cnt,##arg)
#define RACE_LOG_MSGID_I(fmt,cnt,arg...)        LOG_MSGID_I(race,fmt,cnt,##arg)
#define RACE_LOG_MSGID_W(fmt,cnt,arg...)        LOG_MSGID_W(race,fmt,cnt,##arg)
#define RACE_LOG_MSGID_E(fmt,cnt,arg...)        LOG_MSGID_E(race,fmt,cnt,##arg)


/**************************************************************************************************
* Define
**************************************************************************************************/

#define RACE_GET_PORT_BY_MUX_HANDLE(mux_handle) ((race_port_t)((mux_handle) & 0xff))

#define RACE_PROTOCOL_HEADER_LEN                (4)  // RACE Header (1) + Type(1) + Length(2) = 4
#define RACE_PROTOCOL_MIN_BYTE                  (6)  // RACE Header (1) + Type(1) + Length(2) + RACE ID(2) = 6

#define RACE_MUX_HANDLE_INVALID                 (0xFFFFFFFF)
#define RACE_INVALID_PORT                       (0xFF)

#define RACE_DEBUG_INFO_ENABLE                  (1)

/**************************************************************************************************
* Structure
**************************************************************************************************/

#define RACE_PORT_FLAG_INITED_BY_OTHERS         (0x00000001)
#define RACE_PORT_FLAG_RX_PROTOCOL_LOCK         (0x00000002)
#define RACE_PORT_FLAG_TX_PROTOCOL_LOCK         (0x00000004)
typedef uint32_t race_port_flag_t;

#define RACE_PORT_TYPE_INVALID                  (0x00)
#define RACE_PORT_TYPE_NORMAL_UART              (0x01)
#define RACE_PORT_TYPE_1WIRE_UART               (0x02)
#define RACE_PORT_TYPE_COSYS_UART               (0x03)
#define RACE_PORT_TYPE_EMCU_UART                (0x04)
#define RACE_PORT_TYPE_USB                      (0x05)
#define RACE_PORT_TYPE_BT_BLE                   (0x06)
#define RACE_PORT_TYPE_AWS_MCE                  (0x07)
#define RACE_PORT_TYPE_IAP2                     (0x08)
#define RACE_PORT_TYPE_BT_HID                   (0x09)
#define RACE_PORT_TYPE_LL_UART                  (0x0A)
#define RACE_PORT_TYPE_PSEUDO                   (0x0B)
typedef uint32_t race_port_type_t;


enum {
    RACE_PORT_IDLE = 0,  // status before init/open
    RACE_PORT_INIT_SUC,
    RACE_PORT_INIT_FAIL,
};
typedef uint8_t race_port_status_t;

enum {
    RACE_USER_IDLE = 0,
    RACE_USER_ACTIVE,
    RACE_USER_CLOSED,
    RACE_USER_FAIL,
};
typedef uint8_t race_user_status_t;

enum {
    RACE_USER_PRIORITY_LOWEST = 0,
    RACE_USER_PRIORITY_HIGHEST = 0xFF,
};
typedef uint8_t race_user_priority_t;

typedef enum {
    RACE_TX_PROTOCOL_RESULT_SUCCESS = 0,
    RACE_TX_PROTOCOL_RESULT_BYPASS,
} race_tx_protocol_result_t;

typedef enum {
    RACE_RX_PROTOCOL_RESULT_SUCCESS = 0,
    RACE_RX_PROTOCOL_RESULT_BYPASS,
} race_rx_protocol_result_t;


typedef enum {
    MSG_ID_RACE_IDLE_EVENT = 0,
    MSG_ID_MUX_DATA_READY_TO_READ,  // 1
    MSG_ID_SEND_PKT_BY_TASK,  // 2
    MSG_ID_RACE_LOCAL_DELAY_NOTI_IND,  // 3
    MSG_ID_RACE_LOCAL_EVENT_NOTIFY_REQ,  // 4
    MSG_ID_RACE_LOCAL_RSP_NOTIFY_IND, /* deal race cmd whitch not from MUX */
    MSG_ID_RACE_LOCAL_TIMER_EXPIRATION_IND,

#ifdef RACE_LPCOMM_ENABLE
    MSG_ID_RACE_LOCAL_LPCOMM_DATA_RECV_IND,
#endif

#ifdef RACE_FOTA_CMD_ENABLE
    MSG_ID_RACE_LOCAL_FOTA_STOP_IND,
#endif

#ifdef RACE_CFU_ENABLE
    MSG_ID_RACE_LOCAL_CFU_IND,
#endif

#ifdef RACE_BT_CMD_ENABLE
    MSG_ID_RACE_LOCAL_GET_RSSI_CMD,
#endif

#ifdef RACE_STORAGE_CMD_ENABLE
        MSG_ID_RACE_LOCAL_ERASE_PARTITION_CONTINUE_IND,
        MSG_ID_RACE_LOCAL_STORAGE_SHA256_GENERATE_CONTINUE_IND,
#endif

#if (defined RACE_RELAY_CMD_ENABLE) || (defined AIR_RACE_CO_SYS_ENABLE)
    MSG_ID_RACE_LOCAL_RELAY_RACE_CMD,
#endif

#ifdef RACE_BT_EVENT_MSG_HDL
    MSG_ID_RACE_LOCAL_BT_EVENT_IND,
#endif

    MSG_ID_RACE_BT_POWER_ON_OFF_IND,
    MSG_ID_RACE_USB_PLUG_IND,
#if defined (AIR_1WIRE_ENABLE)
    MSG_ID_RACE_1WIRE_INIT_IND,
#endif

    MSG_ID_RACE_END
} race_msg_id_t;


typedef mux_port_t race_port_t;
typedef mux_handle_t race_port_handle_t;

struct race_mux_buffer_struct;
typedef struct race_mux_buffer_struct race_mux_buffer_t;

struct race_pkt_relay_info_struct;
typedef struct race_pkt_relay_info_struct race_pkt_relay_info_t;

#if RACE_DEBUG_INFO_ENABLE
struct race_debug_info_struct;
typedef struct race_debug_info_struct race_debug_info_t;
#endif

struct race_port_user_struct;
typedef struct race_port_user_struct race_port_user_t;

struct race_port_info_struct;
typedef struct race_port_info_struct race_port_info_t;

struct race_port_init_struct;
typedef struct race_port_init_struct race_port_init_t;

struct race_user_config_struct;
typedef struct race_user_config_struct race_user_config_t;


typedef uint32_t (*race_tx_function_t)(uint32_t mux_handle, uint8_t *p_data, uint32_t len);
typedef bool (*race_rx_function_t)(uint32_t mux_handle, mux_buffer_t *p_data, uint32_t *len);

/* mux_open callback */
typedef void (*race_mux_event_callback_t)(mux_handle_t handle, mux_event_t event, uint32_t data_len, void *user_data);

/* mux tx protocol callback*/
typedef race_tx_protocol_result_t (*race_tx_protocol_handler_t)(
    mux_handle_t handle, const mux_buffer_t payload[], uint32_t buffers_counter, mux_buffer_t *head, mux_buffer_t *tail, void *user_data);

/* mux rx protocol callback */
typedef race_rx_protocol_result_t (*race_rx_protocol_handler_t)(
    mux_handle_t *handle, mux_buffer_t buffers[], uint32_t buffers_counter, uint32_t *consume_len, uint32_t *package_len, void *user_data);

typedef void (*race_general_msg_hdl_t)(race_general_msg_t *p_msg);

/* check this pkt is for this user or not */
typedef bool (*race_check_pkt_user_t)(race_port_t port, RACE_COMMON_HDR_STRU *pkt_hdr, race_mux_buffer_t *pkt_buf);

/* check this pkt is for this user or not */
typedef uint32_t (*race_get_drop_size_t)(race_port_t port, RACE_COMMON_HDR_STRU *pkt_hdr, race_mux_buffer_t *pkt_buf);

/* check this pkt is relay or not */
typedef bool (*race_check_relay_t)(race_pkt_relay_info_t *relay_info_out, race_port_t port, race_port_type_t port_type, RACE_COMMON_HDR_STRU *pkt_hdr);


struct race_mux_buffer_struct{
    mux_buffer_t *mux_buffer;
    uint32_t counter;               // the count of mux_buffer
    uint32_t total_length;          // total data length
    uint32_t idx;                   // the index of current mux_buffer
    uint32_t offset;                // offset for current mux_buffer
    uint32_t left_data_len;         // total data left
};

struct race_pkt_relay_info_struct{
    bool is_relay_pkt;
    race_port_t from_port;
    race_port_t to_port;
    uint32_t param;
};

struct race_pkt_info_struct{
    mux_handle_t handle;
    uint32_t data_len;
    RACE_COMMON_HDR_STRU pkt_hdr;
    race_pkt_relay_info_t relay_info;
};


struct race_port_user_struct{
    struct race_port_user_struct *next;
    const char *user_name;
    race_port_handle_t handle;
    race_user_status_t status;
    race_user_priority_t priority;
    race_check_pkt_user_t check_function;
    race_get_drop_size_t get_drop_size;
    race_mux_event_callback_t mux_event_post_handler;
    race_tx_protocol_handler_t tx_protocol_handler;
};

#if RACE_DEBUG_INFO_ENABLE
struct race_debug_info_struct{
    uint32_t current_pkt_irq_cnt;
    uint32_t total_irq_cnt;
    uint32_t total_drop_data_len;
    uint32_t total_valid_pkt_len;
    uint32_t total_tx_fail_cnt;
    uint32_t total_tx_cnt;
    uint32_t tx_suc_byte;
    uint32_t total_relay_pkt;
};
#endif

struct race_port_info_struct{
    race_port_t port;
    race_port_type_t port_type;
    race_port_status_t port_status;
    race_port_flag_t port_flag;
    race_tx_protocol_handler_t tx_protocol_handler;
    race_rx_protocol_handler_t rx_protocol_handler;
    race_tx_function_t tx;
    race_rx_function_t rx;
    race_port_user_t *user_list;

#if RACE_DEBUG_INFO_ENABLE
    race_debug_info_t debug;
#endif
};

struct race_port_init_struct{
    race_port_t port;
    race_port_type_t port_type;
    mux_port_setting_t *port_settings;
    race_tx_protocol_handler_t tx_protocol_handler;     // set to NULL if not use
    race_rx_protocol_handler_t rx_protocol_handler;     // set to NULL if not use
    race_tx_function_t tx_function;                     // set to NULL if use default function
    race_rx_function_t rx_function;                     // set to NULL if use default function
};

struct race_user_config_struct{
    race_port_t port;
    race_port_type_t port_type;
    const char *user_name;                                      // set to NULL if use default name "RACE_CMD"
    race_user_priority_t priority;
    race_check_pkt_user_t check_function;                       // if user_name is NULL, ignore this, if not, this should not be NULL
    race_get_drop_size_t get_drop_size;
    race_mux_event_callback_t mux_event_handler;        // if user_name is NULL, ignore this, if not NULL, replace the default mux callback
    race_mux_event_callback_t mux_event_post_handler;     // set to NULL if not need
    race_tx_protocol_handler_t tx_protocol_handler;
};


/**************************************************************************************************
* Prototype
**************************************************************************************************/

RACE_ERRCODE race_init_port(race_port_init_t *config);
RACE_ERRCODE race_open_port(race_user_config_t *config);
RACE_ERRCODE race_close_port(race_port_t port, const char *user_name);
RACE_ERRCODE race_close_port_for_all_user(race_port_t port);

void race_multi_buffer_init(race_mux_buffer_t *p_race_buf, mux_buffer_t buffers[], uint32_t buffers_counter);
bool race_protocol_fetch(uint8_t *out_buf, uint32_t buf_len, race_mux_buffer_t *p_race_buf);
bool race_protocol_fetch_header(RACE_COMMON_HDR_STRU *p_race_header, race_mux_buffer_t *p_race_buf);

void race_start_sleep_lock_timer(void);

/*
* register race msg handler for race task
*/
void race_register_general_msg_hdl(uint32_t msg_id, race_general_msg_hdl_t hdl);
/*
* register relay check handler
*/
bool race_register_relay_check_handler(race_check_relay_t hdl);

uint32_t race_send_data_to_port(race_port_t port, uint8_t *data, uint32_t len);
uint32_t race_port_send_data_imp(uint32_t mux_handle, uint8_t *p_data, uint32_t len);
uint32_t race_send_data(uint32_t mux_handle, uint8_t *p_data, uint32_t len);
bool race_receive_data(uint32_t *receive_done_data_len, uint32_t mux_handle, mux_buffer_t *buffer);

mux_handle_t race_get_tx_handle_by_port(race_port_t port);
race_port_info_t *race_search_port(race_port_t port);
uint16_t race_search_port_type(race_port_info_t **res_buf, uint16_t res_cnt, race_port_type_t port_type);
race_port_user_t *race_search_user_by_name(race_port_info_t *port_info, const char *user_name);

void race_core_init(void);
void race_task(void *arg);


#ifdef __cplusplus
}
#endif


#endif


/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */
/* MediaTek restricted information */

#ifndef __BT_L2CAP_EXT_H__
#define __BT_L2CAP_EXT_H__


/**
 * @section bt_l2cap_api_usage How to use this module
 *
 * TO DO
 *
 */

/**

*

*/
#include "bt_common_ext.h"
#include "bt_l2cap.h"
#include "bt_hci_ext.h"
#include "bt_aws_mce.h"

/*---------------------------------------------------------------------------
 * PSM value for each protocols and profiles which is unique.
 */
typedef uint16_t bt_l2cap_psm_value;

/* Value for a Service Discovery Protocol */
#define BT_L2CAP_PSM_SDP              0x0001

/* Value for an RFCOMM server */
#define BT_L2CAP_PSM_RFCOMM           0x0003

/* Value for a TCS Binary server */
#define BT_L2CAP_PSM_TCS              0x0005

/* Value for a TCS Binary group */
#define BT_L2CAP_PSM_TCS_CORDLESS     0x0007

/* Value for the BNEP service */
#define BT_L2CAP_PSM_BNEP             0x000F

/* Value for the HID Control Channel */
#define BT_L2CAP_PSM_HID_CTRL         0x0011

/* Value for the HID Interrupt Channel */
#define BT_L2CAP_PSM_HID_INTR         0x0013

/* Value for the UPnP/ESDP service */
#define BT_L2CAP_PSM_UPNP             0x0015

/* Value for the A/V Control Transport protocol */
#define BT_L2CAP_PSM_AVCTP            0x0017

/* AV13 Value for the A/V Control Transport protocol -  */
#define BT_L2CAP_PSM_AVCTP_BROWSING   0x001b

/* Value for the A/V Distribution Transport protocol */
#define BT_L2CAP_PSM_AVDTP            0x0019

/* Value for the ATT protocol */
#define BT_L2CAP_PSM_ATT              0x001F

/* Value for Unrestricted Digital Information Control Plane protocol */
#define BT_L2CAP_PSM_UDI_C            0x001D

//Dynamic PSM value which is only used on special project, and can not modified to other values.
//Customer Can used the PSM value if the profile lib not to be linked.

/* Value for Common Test Profile */
#define BT_L2CAP_PSM_CTP               0x1001

/* The PSM value of the MAP MNS service for obex over L2CAP */
#define BT_L2CAP_PSM_MAP_MNS           0x3a01

/* Value for the AWS protocol */
#define BT_L2CAP_PSM_AWS               0x3a27

/* The PSM value for unused define */
#define BT_L2CAP_PSM_UNUSED            0xFFFF

/*---------------------------------------------------------------------------
 * L2CAP event type
 *
 *     Indicates the type of an L2CAP callback's event during an
 *     callback function call for each user. These events are delivered in the
 *     bt_l2cap_psm_callback_param_t structure's "event" field.
 *
 *     Depending on the event, a specific response may be required or
 *     certain fields in the bt_l2cap_psm_callback_param_t structure may be valid or
 *     invalid. The valid fields and required responses are described
 *     for each event below.
 */
//typedef uint8_t bt_l2cap_event_type_t;

/** Info rsp is received, after info request send.
 * (See bt_l2cap_send_information_req)
 */
#define BT_L2CAP_EVENT_INFO_RSP        5

/** Echo rsp is received, after echo request send.
 * (See bt_l2cap_send_echo_req)
 */
#define BT_L2CAP_EVENT_ECHO_RSP         6

/** The link which used to creat current channel, has occurred flush timeout.
 */
#define BT_L2CAP_EVENT_FLUSH_TIMEOUT    7

/** The acl link exception happened when channel inited but not start to connect.
 */
#define BT_L2CAP_EVENT_ACL_LINK_EXCEPTION     8


/**
 *  @brief Define for the result type of connecting the l2cap channel. It is used as the input paramter of #bt_l2cap_connect_rsp.
 */
#define BT_L2CAP_CONN_ACCEPTED                     0x0000      /**< The result indicates the local side accept the connecting request, and the event #BT_L2CAP_EVENT_CONNECT_CNF will be received later.*/
#define BT_L2CAP_CONN_PENDING                      0x0001      /**< The result indicates the device are in pairing procedure, and the local side need to call #bt_l2cap_connect_rsp again after pairing done, then the event #BT_L2CAP_EVENT_CONNECT_CNF will be received later.*/
#define BT_L2CAP_CONN_REJECT_PSM_NOT_SUPPORTED     0x0002      /**< The result indicates the local side will reject the connecting request due to PSM not support, and the event #BT_L2CAP_EVENT_DISCONNECTED will be received later.*/
#define BT_L2CAP_CONN_REJECT_SECURITY_BLOCK        0x0003      /**< The result indicates the local side will reject the connecting request due to no enough security level, and the event #BT_L2CAP_EVENT_DISCONNECTED will be received later.*/
#define BT_L2CAP_CONN_REJECT_NO_RESOURCES          0x0004      /**< The result indicates the local side will reject the connecting request due to no enough resource, and the event #BT_L2CAP_EVENT_DISCONNECTED will be received later.*/
typedef uint16_t bt_l2cap_connect_result_ext_t;            /**< The type of the channel connecting result.*/

/**
 *  @brief Define for the result type of connecting the l2cap channel. It is used as the input paramter of #bt_l2cap_connect_rsp.
 */

/*---------------------------------------------------------------------------
* Status for Connect rsp result == pending
**/
#define BT_L2CAP_PENDING_STATUS_NONE               0x0000
#define BT_L2CAP_PENDING_STATUS_AUTHENTICATION     0x0001
#define BT_L2CAP_PENDING_STATUS_AUTHORIZATION      0x0002

/*---------------------------------------------------------------------------
 * The constant for RX MTU size and the minimum requirement TX MTU of each profile.
 *
 */
#define BT_L2CAP_AVDTP_MAX_MTU_SIZE    0x037F
#define BT_L2CAP_AVDTP_MIN_MTU_SIZE    0x014F
#define BT_L2CAP_AWS_MAX_MTU_SIZE      0x037F
#define BT_L2CAP_AWS_MIN_MTU_SIZE      0x014F
#define BT_L2CAP_RFCOMM_MAX_MTU_SIZE   0x037F
#define BT_L2CAP_AVCTP_BROWSE_MIN_MTU_SIZE      0x014F

/*define for dynamic rx mtu config unsupport*/
#define BT_L2CAP_RX_MTU_DYNAMIC_NOT_SUPPORT 0xFFFF  /*#bt_l2cap_dynamic_config_info_t*/

/*The return status for the internal profile with dynamic PSM, thus customer profile can use this PSM when this internal profile is not linked.*/
#define BT_STATUS_L2CAP_PROFILE_NOT_LINKED_WITH_DYNAMIC_PSM                (BT_MODULE_L2CAP|0xFF)  /*#bt_status_t*/

/* if missing 3, use reject*/
#define BT_L2CAP_ERTM_SELECT_REJECT_MAX_MISSING_COUNT 2


/*---------------------------------------------------------------------------
* bt_l2cap_event_context_op event
*/

typedef uint8_t bt_l2cap_event_context_op;
#define BT_L2CAP_EVENT_CONTEXT_ALLOCATE    0x00
#define BT_L2CAP_EVENT_CONTEXT_FREE        0x01
#define BT_L2CAP_EVENT_CHANNEL_INITED      0X02
#define BT_L2CAP_EVENT_CONTEXT_CHECK_PROFILE_LINKED  0xFF

#define BT_L2CAP_ERTM_REMOTE_DEFAULT_MAX_TRANSMIT 0x03
#define BT_L2CAP_ERTM_REMOTE_DEFAULT_RETANSMISSION_TIMEOUT   0x07D0  //03E8
#define BT_L2CAP_ERTM_REMOTE_DEFAULT_MONITOR_TIMEOUT         0x07D0

#define BT_L2CAP_ERTM_LOCAL_ACCEPT_MAXIMUM_PDU_SIZE 0x0258

#define BT_L2CAP_ERTM_MAXIMUM_TX_WINDOW_SIZE   0x06
#define BT_L2CAP_ERTM_MAXIMUM_RX_WINDOW_SIZE   0x06

#define BT_L2CAP_MODE_ERTM 3


//typedef uint8_t bt_l2cap_channel_mode_t;
#define BT_L2CAP_MODE_ERTM_MANDATORY    1
#define BT_L2CAP_MODE_ERTM_PREFER       2

/*---------------------------------------------------------------------------
* bt_l2cap_context_allocate_t
*     use as param of @bt_l2cap_context_op_callback
*     bt_l2cap_context_allocate_t for event #BT_L2CAP_EVENT_CONTEXT_ALLOCATE
*     bt_l2cap_context_free_t for event #BT_L2CAP_EVENT_CONTEXT_FREE
*/
typedef struct {
    const bt_bd_addr_t *addr;
    void *channel;
    bt_l2cap_psm_value psm;
} bt_l2cap_context_allocate_t;

typedef struct {
    void *channel;
} bt_l2cap_context_free_t;

/*---------------------------------------------------------------------------
* bt_l2cap_context_operation_callback type
*
*     A function of this type is called when receive l2cap connect request
*     from remote device, and need profile/protocols provide channel context
*     first; or when close channel, need to notify profiles/protocols free channel.
*
*     Return BT_STATUS_SUCCESS, or BT_STATUS_OUT_OF_MEMORY when allocate channel.
*/
typedef bt_status_t (*bt_l2cap_context_op_callback)(bt_l2cap_event_context_op event, void *param);

/*
bt_status_t bt_avrcp_l2cap_context_op_callback(bt_l2cap_event_context_op event, void *param)
{
    if (BT_L2CAP_EVENT_CONTEXT_ALLOCATE == event) {
        bt_l2cap_context_allocate_t *p = (bt_l2cap_context_allocate_t*)param;
        p->channel = alloc_context();
        if (alloc_OOM) {
            return BT_STATUS_OUT_OF_MEMORY;
        }

    }else if(BT_L2CAP_EVENT_CONTEXT_FREE == event) {
        bt_l2cap_context_free_t *p = (bt_l2cap_context_free_t*)param;
        free_context(p->channel);
    }
    return BT_STATUS_SUCCESS;
}
*/


typedef struct {
    bt_l2cap_event_type_t event;                     /* L2CAP upper layer event. Always valid. */
    bt_status_t status;                              /*Completion status, based on the event type. Possible types are L2capDiscReason for BT_L2CAP_EVENT_DISCONNECTED events or BtStatus for most others. */
    union {
        bt_bd_addr_t *address;                       /**<The Bluetooth address of a remote device which is valid for the event #BT_L2CAP_EVENT_CONNECT_IND. */
        uint16_t remote_rx_mtu;                      /*MTU size of remote rx, only for event #BT_L2CAP_EVENT_CONNECT_CNF*/
#ifdef __BT_HB_DUO__
        bt_hci_packet_t  *packet;                    /*rx data for event #BT_L2CAP_EVENT_DATA_IND*/
#endif
    };
} bt_l2cap_psm_callback_param_t;


/*---------------------------------------------------------------------------
* bt_l2cap_psm_callback type
*
*     A function of this type is called to indicate L2CAP events to
*     a protocol service. When L2CAP calls this function, it provides
*     the channel ID that received the event and additional information
*     in the "info" parameter, also return user_data which user pass to
*     l2cap via "bt_l2cap_connect()"
*
*     Protocol services provide the callback function's address to
*     L2CAP by bt_l2cap_psm_info_t table.
*/
typedef bt_status_t (*bt_l2cap_psm_callback)(void *channel, bt_l2cap_psm_callback_param_t *info);

typedef const struct {
    uint16_t remote_retranmission_timeout;
    uint16_t remote_monitor_timeout;
    uint16_t maximum_pdu_size;
    uint8_t remote_maximum_transmit_times;
    uint8_t maximum_tx_window_size;
    uint8_t maximum_rx_window_size;
} bt_l2cap_ertm_channel_config_t;


/*-------------------------------------------------------------------------------------------------------------------*/
/*psm register table info */
typedef struct {
    bt_l2cap_channel_mode_t l2cap_mode;             /*mode support for this psm, such as: BT_L2CAP_BASIC_MODE*/
    bt_l2cap_psm_value psm;                         /* Identifier for the protocol service. */
    bt_l2cap_psm_callback callback;                 /* Callback to invoke whenever an event occurs. */
    bt_l2cap_context_op_callback op_cb;             /* Callback for profiles/protocols to allocate/free channel context*/
    uint16_t max_rx_mtu;                            /*Maximum receive MTU for this service. Should be equal to or less than L2CAP_MAXIMUM_MTU.*/
    uint16_t min_tx_mtu;                            /* Minimum acceptable value for remote device's receive MTU size. Aka, local transmit MTU. Should be no less than L2CAP_MINIMUM_MTU. */
    uint16_t security_level;                        /*secrurity level requriment for psm*/
    bt_l2cap_ertm_channel_config_t *ertm_config;
    bool mhdt_support;
} bt_l2cap_psm_info_t; 
/*Global structure*/
typedef struct {
    bt_linknode_t node;
    bt_module_free_handle_list_t oom_handle;    /* use for OOM handle*/
    uint32_t conn;                  /* with channel create on it*/
    bt_l2cap_psm_callback callback;           /* user callback of this channel */
    uint16_t flags;                         /*flags for config used or other used*/
    uint16_t local_channel;                     /* Local Channel Id (>= 0x0040) */
    uint16_t remote_channel;                    /* Remote devices channel id */
    union {
        uint16_t remote_rx_mtu;                      /*rx mtu size of remote device*/
        uint16_t config_fail_status;                 /*config fail reason when receive, and need to send disconnect request.*/
    };
    uint16_t psm_value;                     /* Profile PSM value*/
    uint8_t state;                          /* L2CAP Channel Connection State */
    uint8_t local_signaling_identifier;         /* use for find channel when receive rsp after send request*/
    uint8_t remote_signaling_identifier;        /* Identifier recv'd in signalling req */
    bool mhdt_support;
} bt_l2cap_channel_t;


typedef struct {
    bt_linknode_t data_pending_list;
    bt_linknode_t data_unack_list;
    bt_linknode_t data_select_reject_list;
    bt_linknode_t data_rx_oom_pending_list;
    uint8_t *segment_head;
    uint8_t *segment_send_fail_pkt;
    uint8_t *reassembly_head;
    uint16_t flags;
    uint16_t local_retransmission_timeout;
    uint16_t local_monitor_timeout;
    uint16_t remote_rx_pdu_size;
    uint16_t local_rx_pdu_size;
    uint8_t local_tx_window_size;
    uint8_t remote_tx_window_size;
    uint8_t local_max_transmit;
    uint8_t unacked_frames;
    uint8_t expected_ack_seq;
    uint8_t expected_tx_seq;
    uint8_t frames_sent;
    uint8_t next_tx_seq;
    uint8_t buffer_seq;
    uint8_t retry_count;
    uint8_t select_reject_actioned_seq;
    uint8_t tx_state;
    uint8_t rx_state;
    uint8_t buffer_seq_select_reject;
    uint8_t select_reject_list_head;
    uint8_t select_reject_list_num;
    uint8_t select_reject_list[BT_L2CAP_ERTM_SELECT_REJECT_MAX_MISSING_COUNT];
} bt_l2cap_channel_ertm_param_t;

#ifdef __MTK_AWS_MCE_ENABLE__
bt_status_t bt_l2cap_rho_get_data_internal(void *data, void *channel);


/*---------------------------------------------------------------------------
*    The data structrue of l2cap user should be defined as the following:
*
*    typedef struct {
*       BT_L2CAP_RHO_DATA_BASIC_MODE ( or BT_L2CAP_RHO_DATA_ERTM_MODE)
*       uint8_t xxx_a;
*       uint16_t xxx_b;
*       uint32_t xxx_c;
*     } bt_profile_xxx_rho_data;
*
*/

typedef struct {
    void *channel;
} bt_l2cap_rho_update_agent_context_t;  /*RHO update agent context.*/

typedef struct {
    const bt_bd_addr_t *addr;
    bt_l2cap_psm_value psm;
    void *channel;
    void *data;
} bt_l2cap_rho_update_partner_context_t; /*RHO update partener context.*/

typedef struct {
    union {
        bt_l2cap_rho_update_agent_context_t agent_context;
        bt_l2cap_rho_update_partner_context_t partner_context;
    };
} bt_l2cap_rho_update_context_t;  /*union of  update context.*/

/*rho update l2cap channel of agent/client*/
bt_status_t bt_l2cap_rho_update_context_internal(bt_aws_mce_role_t role, bt_l2cap_rho_update_context_t *update_context);


BT_PACKED(
typedef struct {
    uint16_t flags;                             /*flags for config used or other used*/
    uint16_t local_channel;                     /* Local Channel Id (>= 0x0040) */
    uint16_t remote_channel;                    /* Remote devices channel id */
    BT_PACKED(
    union{
        uint16_t remote_rx_mtu;                  /*rx mtu size of remote device*/
        uint16_t config_fail_status;             /*config fail reason when receive, and need to send disconnect request.*/
    });
    uint8_t state;                               /* L2CAP Channel Connection State */
    uint8_t local_signaling_identifier;         /* use for find channel when receive rsp after send request*/
}) bt_l2cap_rho_data_basic_mode_t;

typedef struct {
    bt_l2cap_rho_data_basic_mode_t basic_mode;
    uint16_t local_retransmission_timeout;
    uint16_t local_monitor_timeout;
    uint16_t remote_rx_pdu_size;
    uint16_t local_rx_pdu_size;
    uint8_t local_tx_window_size;
    uint8_t remote_tx_window_size;
    uint8_t local_max_transmit;
    uint8_t expected_ack_seq;
    uint8_t expected_tx_seq;
    uint8_t frames_sent;
    uint8_t next_tx_seq;
    uint8_t buffer_seq;
    uint8_t retry_count;
} bt_l2cap_rho_data_ertm_mode_t;

#define BT_L2CAP_RHO_DATA_BASIC_MODE bt_l2cap_rho_data_basic_mode_t parent;
#define BT_L2CAP_RHO_DATA_ERTM_MODE bt_l2cap_rho_data_ertm_mode_t parent;
#endif

/*---------------------------------------------------------------------------
* Use in profiles context which as parent, xxx is profiles
*    typedef struct {
*       BT_L2CAP_CHANNEL_DECLARE
*       uint8_t xxx_a;
*       uint16_t xxx_b;
*       uint32_t xxx_c;
*     } bt_xxx_l2cap_channel;
*/

#define BT_L2CAP_CHANNEL_DECLARE bt_l2cap_channel_t parent;


#define BT_L2CAP_CHANNEL_WITH_ERTM_DECLARE bt_l2cap_channel_t l2cap_parent; \
                                           bt_l2cap_channel_ertm_param_t ertm_param;


typedef struct {
    bt_l2cap_channel_t parent;
    bt_l2cap_channel_ertm_param_t ertm_param;
} bt_l2cap_channel_ertm_t;

/**
 * @brief     This function get channel by psm from acl link, should be used before @bt_l2cap_init_channel().
 * @param[in] addr                    is the address of the remote device.
 * @param[in] psm                     is the psm value of profiles/protocols
 * @return                            Return channel, or return NULL if not found
 */
bt_l2cap_channel_t *bt_l2cap_get_channel(const bt_bd_addr_t *addr,
                                         bt_l2cap_psm_value psm);

/**
 * @brief     This function check channel is valid or not by link.
 * @param[in] channel                 channel context which provide by @bt_l2cap_init_channel
 * @return                            #true means channel is valid.
 *                                    #false means channel is invalid.
 */
bool bt_l2cap_check_channel_is_valid(void *channel);

/**
 * @brief     This function init channel context for connect, context memory should come from profiles/protocols,
 *            after init, channel context will be used for l2cap other operations, and this function should be
 *            used before @bt_l2cap_connect_ext().
 * @param[in] channel                 channel context memory which provided by profiles/protocols
 * @param[in] addr                    is the address of the remote device
 * @param[in] psm                     is the psm value of profiles/protocols
 * @return                            #BT_STATUS_SUCCESS means init success
 *                                    #BT_L2CAP_STATUS_PSM_NOT_SUPPORT means psm not support
 *                                    #BT_STATUS_LINK_IS_DISCONNECTING means acl link is disconnecting
 */
bt_status_t bt_l2cap_init_channel(void *channel,
                                  const bt_bd_addr_t *addr,
                                  bt_l2cap_psm_value psm);

/**
 * @brief     This function deinit channel before @bt_l2cap_connect_ext(), if user not want to connect.
 * @param[in] channel                 channel context which provide by @bt_l2cap_init_channel
 * @return
 */
void bt_l2cap_deinit_channel(void *channel);

/**
 * @brief     This function connect l2cap channel with channel context, should be receive event #BT_L2CAP_EVENT_CONNECTE_CNF
 * @param[in] channel                 channel context which provide by @bt_l2cap_init_channel
 * @return                            #BT_STATUS_PENDING means connect request has send, wait for connect confirm.
 *                                    #BT_STATUS_FAIL means connect request fail
 */
bt_status_t bt_l2cap_connect_ext(void *channel);


/**
 * @brief     This function send connect rsp to remote deive.
 * @param[in] channel                 channel context which init on #BT_L2CAP_EVENT_CONTEXT_ALLOCATE event
 * @param[in] result                  connect rsp result for this channel
 * @return                            #BT_STATUS_PENDING means connect rsp has send, wait for connected confirm
 *                                    #BT_STATUS_OUT_OF_MEMORY means out of memory to send connect rsp
 */
bt_status_t bt_l2cap_connect_rsp_ext(void *channel, bt_l2cap_connect_result_ext_t result);

/**
 * @brief     This function disconnect l2cap channel with channel context
 * @param[in] channel                 channel context which init on #BT_L2CAP_EVENT_CONTEXT_ALLOCATE event
 * @return                            #BT_STATUS_PENDING means connect request has send, wait for disconnect confirm.
 */
bt_status_t bt_l2cap_disconnect_ext(void *channel);

/**
 * @brief     This function send data after channel connect success.
 * @param[in] channel                 channel context which provide by @bt_l2cap_init_channel
 * @param[in] packet                  l2cap data send by l2cap user
 * @param[in] length                  length of l2cap data
 * @return                            #BT_STATUS_SUCCESS means l2cap data send success.
 */
bt_status_t bt_l2cap_send_ext(void *channel,
                              const uint8_t *packet,
                              uint32_t length);

/**
 * @brief     This function send data which can be auto flushed when flush time out after channel connect success.
 * @param[in] channel                 channel context which provide by @bt_l2cap_init_channel
 * @param[in] packet                  l2cap data send by l2cap user
 * @param[in] length                  length of l2cap data
 * @return                            #BT_STATUS_SUCCESS means l2cap data send success.
 */
bt_status_t bt_l2cap_send_with_auto_flush(void *channel,
                                          const uint8_t *packet,
                                          uint32_t length);

/**
 * @brief     This function get channel by psm from acl link use connection structure.
 * @param[in] conn                    connection context which provide by @bt_l2cap_link_status_change_notify.
 * @param[in] psm                     is the psm value of profiles/protocols
 * @return                            Return channel, or return NULL if not found
 */
void *bt_l2cap_get_channel_by_connection(uint32_t conn, bt_l2cap_psm_value psm);

/*get tx mtu size, user should not send pdu exceed it*/
uint16_t bt_l2cap_get_tx_mtu(void *channel);

/*config rtx mtu size, should be called before channel start to connect*/
bt_status_t bt_l2cap_set_rx_mtu(bt_l2cap_psm_value psm, uint16_t rx_mtu);

bt_bd_addr_t *bt_l2cap_get_bd_addr_by_channel(void *channel);

/*get connection handle (uint16_t) by l2cap channel*/
bt_handle_t bt_l2cap_get_connection_handle_by_channel(void *channel);

/*get connection by l2cap channel*/
uint32_t bt_l2cap_get_connection_by_channel(void *channel);

bt_status_t bt_l2cap_set_mhdt_support(void *channel, bool is_support);

uint8_t *bt_l2cap_pdu_alloc(uint32_t size);
void bt_l2cap_pdu_free(uint8_t *p);
bool bt_l2cap_is_allcated_pdu_valid(uint8_t *p);
bt_status_t bt_l2cap_profile_register(const bt_l2cap_psm_info_t *psm_info);


#endif //__BT_L2CAP_EXT_H__


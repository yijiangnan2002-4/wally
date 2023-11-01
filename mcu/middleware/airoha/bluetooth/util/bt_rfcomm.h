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

/**
 * section bt_rfcomm_api_usage How to use this module
 *
 * TO DO
 *
 */
#ifndef __BT_RFCOMM_H__
#define __BT_RFCOMM_H__

#include "bt_l2cap_ext.h"
#ifdef __MTK_AWS_MCE_ENABLE__
#include "bt_aws_mce.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef uint8_t bt_rfcomm_payload_t;

#define BT_RFCOMM_INVALID_HANDLE 0x00000000

/* Address(1 byte) + Control(1 byte) + Length Indicator(2 bytes) */
#define BT_RFCOMM_FRAME_HEADER_LENGTH       4
/* FCS(1 byte) */
#define BT_RFCOMM_FRAME_FOOTER_LENGTH       1

#define BT_RFCOMM_PAYLOAD_TO_RFCOMM_FRAME(p) ((uint8_t *)(p) - BT_RFCOMM_FRAME_HEADER_LENGTH)
#define BT_RFCOMM_FRAME_TO_L2CAP_PACKET(p) ((uint8_t *)(p) - BT_HCI_L2CAP_HEADER_LENGTH)

#define BT_RFCOMM_PAYLOAD_ALLOC(size)    \
        ((void *)((uint8_t *)bt_l2cap_pdu_alloc(BT_RFCOMM_FRAME_HEADER_LENGTH + (size) + BT_RFCOMM_FRAME_FOOTER_LENGTH)+ \
            BT_RFCOMM_FRAME_HEADER_LENGTH))

#define BT_RFCOMM_PAYLOAD_TO_FRAME_LENGTH(size)    \
        (BT_RFCOMM_FRAME_HEADER_LENGTH + (size) + BT_RFCOMM_FRAME_FOOTER_LENGTH)

#define BT_RFCOMM_IS_ALLOCATED_FRAME_VALID(p)    \
    bt_l2cap_is_allcated_pdu_valid((uint8_t *)(p))

//BT_L2CAP_IS_ALLOCATED_PDU_VALID(((uint8_t *)(p) - BT_RFCOMM_FRAME_HEADER_LENGTH - BT_HCI_L2CAP_HEADER_LENGTH))
#define BT_RFCOMM_IS_ALLOCATED_PAYLOAD_VALID(p)  \
    bt_l2cap_is_allcated_pdu_valid((uint8_t *)(p) - BT_RFCOMM_FRAME_HEADER_LENGTH)

#define BT_RFCOMM_PAYLOAD_FREE(p)     \
    bt_l2cap_pdu_free((uint8_t *)(p) - BT_RFCOMM_FRAME_HEADER_LENGTH)

/*
uint8_t obex_data[5];
bt_rfcomm_payload_t *payload = BT_RFCOMM_PAYLOAD_ALLOC(5);
if (!BT_RFCOMM_IS_ALLOCATED_PAYLOAD_VALID(payload)) {
    return OOM;
}
bt_memcpy(payload, obex_data, 5);
bt_rfcomm_send_data(channel, payload, 5);
*/

/**< rfcomm server channel id for users to register sdp record*/
#define BT_RFCOMM_HFP_SERVER_ID 0x01   /*hfp server id*/
/**< The start of the obex server id. */
#define BT_RFCOMM_HSP_SERVER_ID 0x02
#define BT_RFCOMM_OBEX_SERVER_START 0x03
#define BT_RFCOMM_PBAP_SERVER_ID    0x03    /*pbap server id*/
#define BT_RFCOMM_MAP_SERVER_ID     0x04    /*map server id*/
#define BT_RFCOMM_OBEX_SERVER_END   0x04
/**< The end of the obex server id. */
#define BT_RFCOMM_HFP_AG_SERVER_ID  0x05
/**< The base of the spp server id. */
#define BT_RFCOMM_SPP_SERVER_START 0x06
#define BT_RFCOMM_SPP_SERVER_ID 0x06  /*spp server id*/
#define BT_RFCOMM_SPP_SERVER_END 0x1E
/**< The end of the spp server id. */

#define BT_RFCOMM_MAX_FRAME_SIZE_DISABLE_3M  (BT_L2CAP_DEFAULT_MTU - 5)
#define BT_RFCOMM_MAX_FRAME_SIZE (BT_L2CAP_RFCOMM_MAX_MTU_SIZE - 5)
#define BT_RFCOMM_DEFAULT_FRAME_SIZE    127
#define BT_RFCOMM_MIN_FRAME_SIZE       23

/**< RFCOMM status definitions. \*/
#define BT_STATUS_RFCOMM_REMOTE_NO_CRIDIT  (BT_MODULE_RFCOMM|0xE0)

typedef uint8_t bt_rfcomm_event_t;
#define BT_RFCOMM_EVENT_CONNECT_IND 0x01
#define BT_RFCOMM_EVENT_CONNECTED 0x02
#define BT_RFCOMM_EVENT_DISCONNECTED 0x03
#define BT_RFCOMM_EVENT_DATA_IND 0x04
#define BT_RFCOMM_EVENT_SEND_IND 0x05
#define BT_RFCOMM_EVENT_CONTEXT_ALLOCATE    0x06
#define BT_RFCOMM_EVENT_CONTEXT_FREE        0x07


/*---------------------------------------------------------------------------
* Use in profiles context which as parent, xxx is profiles
*    typedef struct {
*       BT_RFCOMM_CHANNEL_DECLARE
*       uint8_t xxx_a;
*       uint16_t xxx_b;
*       uint32_t xxx_c;
*     } bt_xxx_rfcomm_channel;
*/

#define BT_RFCOMM_CHANNEL_DECLARE bt_rfcomm_channel_t parent;


typedef struct {
    void *channel;         /* RFCOMM channel handle uplayer used*/
} bt_rfcomm_event_connect_ind_t;

typedef struct {
    void *channel;
    bt_status_t status;      /* status */
    uint16_t max_frame_size;
} bt_rfcomm_event_connect_cnf_t;

typedef struct {
    void *channel;
    bt_status_t status;      /* status */
} bt_rfcomm_event_disconnected_t;

typedef struct {
    void *channel;        /* profile user data for uplayer used*/
    uint16_t payload_length;  /*the playload length for profile used*/
    bt_hci_packet_t *packet; /*hci rx data, if data_length is 0, it means this hci packet is only credit frame without playload */
} bt_rfcomm_event_data_ind_t;

typedef struct {
    void *channel;        /* profile user data for uplayer used*/
} bt_rfcomm_event_send_ind_t;

typedef struct {
    bt_bd_addr_t *addr;
    void *channel;
    uint8_t server_id;
} bt_rfcomm_context_allocate_t;

typedef struct {
    void *channel;
} bt_rfcomm_context_free_t;

typedef struct {
    union {
        bt_rfcomm_event_connect_ind_t bt_rfcomm_event_connect_ind;
        bt_rfcomm_event_connect_cnf_t bt_rfcomm_event_connect_cnf;
        bt_rfcomm_event_disconnected_t bt_rfcomm_event_disconnected;
        bt_rfcomm_event_data_ind_t bt_rfcomm_event_data_ind;
        bt_rfcomm_event_send_ind_t bt_rfcomm_event_send_ind;
        bt_rfcomm_context_allocate_t bt_rfcomm_event_context_allocate;
        bt_rfcomm_context_free_t bt_rfcomm_event_context_free;
    } param;
} bt_rfcomm_callback_param_t;

typedef bt_status_t(*bt_rfcomm_callback)(
    bt_rfcomm_event_t event,
    bt_rfcomm_callback_param_t *param);

bt_status_t bt_rfcomm_spp_callback(
    bt_rfcomm_event_t event,
    bt_rfcomm_callback_param_t *param);

bt_status_t bt_rfcomm_hfp_callback(
    bt_rfcomm_event_t event,
    bt_rfcomm_callback_param_t *param);

bt_status_t bt_rfcomm_hsp_callback(
    bt_rfcomm_event_t event,
    bt_rfcomm_callback_param_t *param);

bt_status_t bt_goepc_rfcomm_callback(
    bt_rfcomm_event_t event,
    bt_rfcomm_callback_param_t *param);

bt_status_t bt_rfcomm_hfp_ag_callback(
    bt_rfcomm_event_t event,
    bt_rfcomm_callback_param_t *param);

/*RFCOMM channel Context*/
typedef struct {
    bt_linknode_t node;
    bt_module_free_handle_list_t ch_oom_handle;
    bt_rfcomm_callback callback;  /* user callback of this channel */
    uint32_t session_handle;        /*dlci belong to which session.*/
    uint16_t max_frame_size;      /* max frame size for each dlci  */
    uint8_t rx_credit;              /*rx credit for remote device to tx data*/
    uint8_t tx_credit;              /*tx credit for local device to tx data*/
    uint8_t dlci;                 /*Data link connection identifier*/
    uint8_t flags;                /* channel flags              */
    uint8_t state;                /* channel state              */
} bt_rfcomm_channel_t;

#ifdef __MTK_AWS_MCE_ENABLE__
BT_PACKED(
typedef struct {
    uint16_t max_frame_size;      /* max frame size for each dlci  */
    uint8_t rx_credit;            /*rx credit for remote device to tx data*/
    uint8_t tx_credit;            /*tx credit for local device to tx data*/
    uint8_t dlci;                 /*Data link connection identifier*/
    uint8_t flags;                /* channel flags              */
    uint8_t state;                /* channel state              */
}) bt_rfcomm_rho_channel_t;

typedef struct {
    void *channel;
} bt_rfcomm_rho_destory_context_t;

typedef struct {
    void *channel;
    const bt_bd_addr_t *addr;
    bt_rfcomm_callback callback;
    void *data;
} bt_rfcomm_rho_create_context_t;

typedef struct {
    union {
        bt_rfcomm_rho_destory_context_t agent;
        bt_rfcomm_rho_create_context_t partner;
    };
} bt_rfcomm_rho_update_context_t;  /*union of  update context.*/
#endif

#define BT_RFCOMM_RHO_CHANNEL_DECLARE bt_rfcomm_rho_channel_t parent;

bt_status_t bt_rfcomm_init_channel(
    void *channel,
    const bt_bd_addr_t *addr,
    bt_rfcomm_callback callback);

void bt_rfcomm_deinit_channel(void *channel);

void *bt_rfcomm_get_channel(
    const bt_bd_addr_t *addr, bt_rfcomm_callback callback);

uint8_t bt_rfcomm_get_all_channels(
    uint32_t *found_channels, const bt_bd_addr_t *address, bt_rfcomm_callback callback);

bt_status_t bt_rfcomm_connect(void *channel, uint8_t server_id);

bt_status_t bt_rfcomm_connect_response(void *channel, bool response);

bt_status_t bt_rfcomm_disconnect(void *channel);

bt_status_t bt_rfcomm_send_data(
    void *channel,
    const bt_rfcomm_payload_t *packet,
    uint16_t length);

bt_status_t bt_rfcomm_send_credit(void *channel, uint8_t credit);

bt_bd_addr_t *bt_rfcomm_get_bd_address(void *channel);

uint16_t bt_rfcomm_get_max_frame_size(void *channel);

uint8_t bt_rfcomm_get_credit(void *channel);

uint8_t bt_rfcomm_get_server_id(void *channel);

void bt_rfcomm_hold_credit(void *channel);

void bt_rfcomm_release_credit(void *channel);

#ifdef __MTK_AWS_MCE_ENABLE__
bt_status_t bt_rfcomm_rho_get_channel(void *data, void *channel);

void bt_rfcomm_rho_update_channel(
    bt_aws_mce_role_t role, bt_rfcomm_rho_update_context_t *parameter);
#endif

#ifdef __cplusplus
}
#endif

#endif /*__BT_RFCOMM_H__*/


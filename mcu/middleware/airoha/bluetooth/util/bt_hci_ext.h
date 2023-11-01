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

#ifndef __BT_HCI_EXT_H__
#define __BT_HCI_EXT_H__

#include "bt_type.h"
#include "bt_platform.h"
#include "bt_linknode.h"
BT_EXTERN_C_BEGIN
/**
 *  @brief      HCI command packet. Please refer to core spec 5.4.1 HCI Command Packet.
 */
BT_PACKED(
typedef struct {
    uint16_t           cmd_code; /**< Command code */
    uint8_t             length;   /**< Length of all of the parameters contained in this packet */
    bt_data_t           param;     /**< Parameters */
}) bt_hci_packet_cmd_t;

/**
 *  @brief      HCI event packet. Please refer to core spec 5.4.4 HCI Event Packet.
 */
BT_PACKED(
typedef struct {
    uint8_t             evt_code; /**< Event code */
    uint8_t             length;   /**< Length of all of the parameters contained in this packet */
    bt_data_t           param;    /**< Parameters */
}) bt_hci_packet_evt_t;

/**
 *  @brief      HCI ACL packet. Please refer to core spec, Please refer to core spec 5.4.2 HCI ACL Data Packets.
 */
BT_PACKED(
typedef struct {
    uint16_t    handle: 12; /**< Handle */
    uint16_t    pb_flag: 2; /**< Packet boundary flag */
    uint16_t    bc_flag: 2; /**< Broadcast flag */
    uint16_t    length;     /**< Data total length */
    bt_data_t   data;       /**< Data */
}) bt_hci_packet_acl_t;

#ifdef BT_LE_AUDIO_ENABLE
/**
 *  @brief      HCI iso data packet. Please refer to core spec.
 */
BT_PACKED(
typedef struct {
    uint16_t handle: 12;    /**< Handle */
    uint16_t pb_flag: 2;    /**< Packet boundary flag */
    uint16_t ts_flag: 1;    /**< time stamp flag */
    uint16_t reserve: 1;    /**< reserve for feture */
    uint16_t length:  14;   /**< Data total length */
    uint16_t len_reserve: 2;/**< reserve for feture */
    bt_data_t data;         /**< Data */
}) bt_hci_packet_iso_t;
#endif


#ifdef __BT_HB_DUO__
/**
 *  @brief      BT HCI packet.
 */
#define BT_HCI_PACKET_HEADER \
    bt_linknode_t node;\
    uint16_t    packet_length;\
    uint16_t    offset;\
    void*       callback;                   /*tx done callback*/ \
    uint32_t    time_stamp;                 /*reserve for time stamp*/ \
    uint8_t     ref_count;\
    uint8_t     is_rx;                        /*is rx packet*/ \
    uint8_t     reserve

BT_PACKED(
typedef struct {
    BT_HCI_PACKET_HEADER;
    uint8_t   indicator;                  /**< Indicate packet is CMD, ACL or EVT */
    BT_PACKED(
    union {
        bt_hci_packet_acl_t acl;            /**< ACL packet */
        bt_hci_packet_cmd_t cmd;            /**< Command packet */
        bt_hci_packet_evt_t evt;            /**< Event packet */
#ifdef BT_LE_AUDIO_ENABLE
        bt_hci_packet_iso_t iso;            /**< ISO packet */
#endif
    });
}) bt_hci_packet_t;

void bt_hci_hold_rx_packet(bt_hci_packet_t *packet);
void bt_hci_release_rx_packet(bt_hci_packet_t *packet);

BT_EXTERN_C_END

#endif /*__BT_HB_DUO__*/
#endif /* __BT_HCI_EXT_H__ */


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

#ifndef RACE_XPORT_H
#define RACE_XPORT_H

#include "stdint.h"
#include "memory_attribute.h"
#include "syslog.h"
#include "race_cmd.h"
#include "race.h"

#include "race_core.h"
#include "race_port_1wire.h"
#include "race_port_bt.h"
#include "race_port_cosys.h"
#include "race_port_pseudo.h"
#include "race_port_uart.h"
#include "race_port_usb.h"
#include "race_usb_relay.h"

#include "serial_port.h" /* BT and USB still use port serivce */

#ifdef __cplusplus
extern "C"
{
#endif


#define RACE_DEBUG_PRINT_ENABLE             (TRUE)

#define RACE_INVALID_CHANNEL_ID             (0xFF)  /* Now, channel id means port id, defined as mux_port_t */

#define RACE_TARGET_INVALID_DEVICE          (0xFF)

/*
* set relay flag
*/
#if (defined RACE_RELAY_CMD_ENABLE) || (defined AIR_RACE_CO_SYS_ENABLE)
#define RACE_CHANNEL_ID_RELAY_CMD_FLAG (0x80)

#define RACE_CHANNEL_ID_SET_RELAY_CMD_FLAG(channel_id)      (((channel_id) == RACE_INVALID_CHANNEL_ID) ? (channel_id) : (channel_id) | RACE_CHANNEL_ID_RELAY_CMD_FLAG)
#define RACE_CHANNEL_ID_CLEAR_RELAY_CMD_FLAG(channel_id)    (((channel_id) == RACE_INVALID_CHANNEL_ID) ? (channel_id) : (channel_id) & (~RACE_CHANNEL_ID_RELAY_CMD_FLAG))
#define RACE_CHANNEL_ID_IS_RELAY_CMD_FLAG_SET(channel_id)   (((channel_id) == RACE_INVALID_CHANNEL_ID) ? (false) : (channel_id) & RACE_CHANNEL_ID_RELAY_CMD_FLAG)
#else
#define RACE_CHANNEL_ID_SET_RELAY_CMD_FLAG(channel_id)      (channel_id)
#define RACE_CHANNEL_ID_CLEAR_RELAY_CMD_FLAG(channel_id)    (channel_id)
#define RACE_CHANNEL_ID_IS_RELAY_CMD_FLAG_SET(channel_id)   (FALSE)
#endif


/*
*   redefine race_serial_port_type_enum for old code
*/

#define RACE_SERIAL_PORT_TYPE_NONE              (RACE_INVALID_PORT)

#define RACE_SERIAL_PORT_TYPE_UART              (race_uart_get_active_port())
#define RACE_SERIAL_PORT_TYPE_USB               (race_usb_get_active_port())
#define RACE_SERIAL_PORT_TYPE_1WIRE             (race_1wire_get_active_port())

#ifdef AIR_RACE_CO_SYS_ENABLE
#define RACE_SERIAL_PORT_TYPE_COSYS             (race_cosys_get_port())
#endif

#if defined (AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined (AIR_DCHS_MODE_MASTER_ENABLE)
#define RACE_SERIAL_PORT_TYPE_EMCU              (race_emcu_get_port())
#else
#define RACE_SERIAL_PORT_TYPE_EMCU              (RACE_INVALID_PORT)
#endif

#if defined(MTK_MUX_BT_ENABLE)
#define RACE_SERIAL_PORT_TYPE_SPP               (MUX_BT_SPP)
#define RACE_SERIAL_PORT_TYPE_BLE               (MUX_BT_BLE)
#define RACE_SERIAL_PORT_TYPE_BLE_1             (MUX_BT_BLE_1)
#define RACE_SERIAL_PORT_TYPE_BLE_2             (MUX_BT_BLE_2)
#define RACE_SERIAL_PORT_TYPE_AIRUPDATE         (MUX_BT_AIRUPATE)

#if defined(MTK_GATT_OVER_BREDR_ENABLE)
#define RACE_SERIAL_PORT_TYPE_GATT_OVER_BREDR   (MUX_BT_GATT_OVER_BREDR)
#else
#define RACE_SERIAL_PORT_TYPE_GATT_OVER_BREDR   (RACE_INVALID_PORT)
#endif

#else
#define RACE_SERIAL_PORT_TYPE_SPP               (RACE_INVALID_PORT)
#define RACE_SERIAL_PORT_TYPE_BLE               (RACE_INVALID_PORT)
#define RACE_SERIAL_PORT_TYPE_BLE_1             (RACE_INVALID_PORT)
#define RACE_SERIAL_PORT_TYPE_BLE_2             (RACE_INVALID_PORT)
#define RACE_SERIAL_PORT_TYPE_AIRUPDATE         (RACE_INVALID_PORT)
#define RACE_SERIAL_PORT_TYPE_GATT_OVER_BREDR   (RACE_INVALID_PORT)
#endif

#if defined(MTK_IAP2_VIA_MUX_ENABLE)
#define RACE_SERIAL_PORT_TYPE_IAP2              (RACE_MUX_IAP2_PORT)
#else
#define RACE_SERIAL_PORT_TYPE_IAP2              (RACE_INVALID_PORT)
#endif

#if defined(AIR_MUX_BT_HID_ENABLE)
#define RACE_SERIAL_PORT_TYPE_HID               (RACE_MUX_HID_PORT)
#else
#define RACE_SERIAL_PORT_TYPE_HID               (RACE_INVALID_PORT)
#endif

#if defined(AIR_RACE_SCRIPT_ENABLE)
#define RACE_SERIAL_PORT_TYPE_PSEUDO            (MUX_PORT_PSEUDO)
#else
#define RACE_SERIAL_PORT_TYPE_PSEUDO            (RACE_INVALID_PORT)
#endif

#define RACE_SERIAL_PORT_TYPE_MAX               (0xFF)

typedef uint8_t race_serial_port_type_enum;


/**
 * @brief This function initializes the RACE module. It is used to set the port configuration for data transmission.
 * @param[in] port is the serial port number used to data transmission for RACE. For more details about this parameter, please refer to race_serial_port_list_init(port).
 * @return   #RACE_STATUS_OK the RACE initialized successfully. \n
 *               #RACE_STATUS_ERROR the UART initialization or the RACE local initialization failed due to the RACE initialization failure.
 * @note     The #race_init_port_service() can select the UART through HAL UART port directly, or select UART, USB or BT SPP server/client port through port service for data transmission.
 * @par       Example
 * @code
 *       ret = race_init_port_service(port);
 *       if (ret == RACE_STATUS_OK) {
 *          // The RACE initialized successfully.
 *          // Create an RACE task.
 *       } else {
 *          // The RACE initialization failed.
 *       }
 * @endcode
 */
extern race_status_t race_init_port_service(serial_port_dev_t port);

/**
 * @brief  This function is used to send any type of race CMD to the peer device whenever needed.
 *  For example, it can be used to send 5B back after the handler for the race CMD ID range returns.
 * @param[in]
 *  ptr: the pointer points to the race payload of the race CMD packet which is created by using RACE_ClaimPacket().
 *   - It can be set to the Non-NULL return value of RACE_ClaimPacket() simply.
 *  channel_id: it indicates the transport method of the race CMD received. Its type used is actually race_serial_port_type_num.
 * @return RACE_STATUS_OK: succeed; Otherwise: fail. In such case, the packet should be freed by RACE_FreePacket().
 */
race_status_t       race_flush_packet(uint8_t *ptr, uint8_t channel_id);

race_status_t race_flush_packet_relay(race_pkt_t *race_pkt, uint16_t length, uint8_t channel_id, uint8_t type, uint8_t send_idx);

race_port_type_t    race_get_port_type(race_port_t port);

uint32_t            race_port_send_data(uint32_t port, uint8_t *buf, uint32_t buf_len);
uint32_t            race_get_port_handle_by_channel_id(uint8_t channel_id);

race_status_t race_serial_port_open(race_serial_port_type_enum port_type, serial_port_dev_t port_num);
race_status_t race_serial_port_close(race_serial_port_type_enum port_type);

race_serial_port_type_enum race_get_port_type_by_channel_id(uint8_t channel_id);
race_serial_port_type_enum race_get_channel_id_by_port_handle(uint32_t port_handle);
uint8_t race_get_channel_id_by_port_type(race_serial_port_type_enum port_type);
uint32_t race_get_serial_port_handle(serial_port_dev_t port);
uint8_t race_get_device_id_by_conn_address(bt_bd_addr_t *peer_address);


/*
* old api for switch
*/
race_status_t       race_serial_port_uart_init(serial_port_dev_t port);
serial_port_dev_t   race_switch_get_active_uart_port(void);
serial_port_dev_t   race_switch_get_uart_default_port(void);
race_status_t race_uart_deinit(void);


#ifdef __cplusplus
}
#endif


#endif


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

#ifndef _BT_HOGP_H_
#define _BT_HOGP_H_

#include "bt_type.h"

/**
 * @addtogroup Bluetooth_Services_Group Bluetooth Services
 * @{
 * @addtogroup BluetoothServices_HOGP HOGP
 * @{
 * This HID over GATT profile defines how a device with bluetooth low energy wireless communications can support HID services over the bluetooth low energy protocol stack using the GATT.
 * This profile defines three roles: HID device, boot host, and report host. Use of the term HID host refers to both host roles: Boot Host, and Report Host. A report host is required to
 * support a HID Parser and be able to handle arbitrary formats for data transfers (known as Reports) whereas a Boot Host is not required to support a
 * HID parser as all data transfers (reports) for boot protocol mode are of predefined length and format.
 *
 * Terms and Acronyms
 * ======
 * |Terms                         |Details                                                                 |
 * |------------------------------|------------------------------------------------------------------------|
 * |\b HID                        |Human Interface Device. HID is a type of computer device usually used by humans that takes input from humans and gives output to humans. |
 * |\b HOGP                       |HID over GATT profile. Allowing Bluetooth LE-enabled Wireless mice, keyboards and other devices offering long-lasting battery life. |
 *
 * @section bt_hogp_api_usage How to use this module
 * -  This section describes initializing the HOGP service and handing various events.
 *  - Step 1: Mandatory, initializing the HOGP service.
 *   - Sample code:
 *      @code
 *         static uint8_t app_hogp_report_descriptor[] = {0x06, 0x0D, 0xff, \
 *                                                        0x09, 0x01, \
 *                                                        0xa1, 0x01, \
 *                                                        0x15, 0x00, \
 *                                                        0x26, 0xff, 0x00, \
 *                                                        0x75, 0x08, \
 *                                                        0x85, 0x81, \
 *                                                        0x09, 0x01, \
 *                                                        0x95, 0x32, \
 *                                                        0x81, 0x02, \
 *                                                        0x85, 0x82, \
 *                                                        0x09, 0x01, \
 *                                                        0x95, 0x32, \
 *                                                        0xb1, 0x02, \
 *                                                        0x85, 0x84, \
 *                                                        0x09, 0x01, \
 *                                                        0x95, 0x32, \
 *                                                        0x91, 0x02, \
 *                                                        0xc0
 *                                                        };
 *         static bt_status_t app_hogp_event_callback(bt_hogp_event_t event, void *buffer, void *output)
 *         {
 *              return BT_STATUS_SUCCESS;
 *         }
 *         bt_hogp_init_parameter_t init_parameter;
 *         init_parameter.mode = BT_HOGP_REPORT_PROTOCOL_MODE;
 *         init_parameter.hid_information.bcdhid = 0x10;        // Version = 1.0.
 *         init_parameter.hid_information.bcountrycode = 0x00;  // Hardware is not localized.
 *         init_parameter.hid_information.flags = BT_HOGP_INFO_FLAGS_REMOTEWAKE | BT_HOGP_INFO_FLAGS_NORMALLY_CONNECTABLE;
 *         init_parameter.report_descriptor.descriptor = app_hogp_report_descriptor;
 *         init_parameter.report_descriptor.length = sizeof(app_hogp_report_descriptor);
 *         bt_hogp_init(&init_parameter, app_hogp_event_callback);
 *      @endcode
 *  - Step 2: Mandatory, implement #bt_hogp_event_callback() to handle the HOGP events, such as connect, disconnect, get report and more.
 *   - Sample code:
 *      @code
 *         uint8_t app_hogp_report_response_table[][50] = {
 *            //custom feature
 *            0x03,0x00,0x00,0x48,0x32,0x32,0x00,0x00,
 *            0x00,0x00,0x30,0x45,0x48,0x30,0x30,0x55,0x32,
 *            0x30,0x31,0x33,0x31,0x30,0x33,0x02,0x11,0x01,
 *            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 *            0x00,0x12,0x01,0x45,0x54,0x0A,0x2A,0x9F,0xE0,
 *            0x00,0x00,0x00,0x00,0x00,0x00}
 *         };
 *         static bt_status_t app_hogp_event_callback(bt_hogp_event_t event, void *buffer, void *output)
 *         {
 *               switch (event) {
 *                     case BT_HOGP_EVENT_CONNECT_IND:
 *                     {
 *                        bt_hogp_connect_ind_t *ind = (bt_hogp_connect_ind_t *)buffer;
 *                     }
 *                     break;
 *                     case BT_HOGP_EVENT_DISCONNECT_IND:
 *                     {
 *                        bt_hogp_disconnect_ind_t *ind = (bt_hogp_disconnect_ind_t *)buffer;
 *                     }
 *                     break;
 *                     case BT_HOGP_EVENT_GET_REPORT_ID_IND:
 *                     {
 *                        bt_hogp_get_report_id_t *ind = (bt_hogp_get_report_id_t *)buffer;
 *                        if(output != NULL) {
 *                             bt_hogp_get_report_id_response_t response;
 *                             response.report_id = 0x81;
 *                             memcpy(output, &response, sizeof(bt_hogp_get_report_id_response_t));
 *                        }
 *                     }
 *                     break;
 *                     case BT_HOGP_EVENT_CONTORL_IND:
 *                     {
 *                        bt_hogp_control_ind_t *ind = (bt_hogp_control_ind_t *)buffer;
 *                     }
 *                     break;
 *                     case BT_HOGP_EVENT_GET_REPORT_IND:
 *                     {
 *                        bt_hogp_get_report_ind_t *ind = (bt_hogp_get_report_ind_t *)buffer;
 *                        if(output != NULL) {
 *                             bt_hogp_get_report_response_t response;
 *                             response.report_length = 50;
 *                             response.report_data = &app_hogp_report_response_table[0][ind->offset];
 *                             memcpy(output, &response, 50);
 *                        }
 *                     }
 *                     break
 *                     case BT_HOGP_EVENT_SET_REPORT_IND:
 *                     {
 *                        bt_hogp_set_report_ind_t *ind = (bt_hogp_set_report_ind_t *)buffer;
 *                     }
 *                     break;
 *                     default:
 *                        break;
 *               }
 *               return BT_STATUS_SUCCESS;
 *         }
 *      @endcode
 *
 */

/**
 * @defgroup BluetoothHOGP_define Define
 * @{
 * This section defines the macros for the HOGP.
 */

/**
 * @brief The event report to user
 */
typedef uint8_t bt_hogp_event_t;                                              /**< The type of HOGP event. */
#define BT_HOGP_EVENT_CONNECT_IND                            (0x01)           /**< The connection confirmation event is triggered when the connection is established between the local and remote devices. The structure for this event is defined as #bt_hogp_connect_ind_t. */
#define BT_HOGP_EVENT_DISCONNECT_IND                         (0x02)           /**< The disconnect indication event shows the local device and the remote device are disconnected. The structure for this event is defined as #bt_hogp_disconnect_ind_t. */
#define BT_HOGP_EVENT_GET_REPORT_ID_IND                      (0x03)           /**< The get report id request is received from the Host. The structure for this event is defined as #bt_hogp_get_report_id_t.
                                                                                        The user needs to reply with report id in the output parameter. The structure for this response is defined as #bt_hogp_get_report_id_response_t.*/
#define BT_HOGP_EVENT_CONTROL_IND                            (0x04)           /**< The hid control request is received from host. The structure for this event is defined as #bt_hogp_control_ind_t. */
#define BT_HOGP_EVENT_GET_REPORT_IND                         (0x05)           /**< The get report request is received from the host. The structure for this event is defined as #bt_hogp_get_report_ind_t.
                                                                                        The user needs to reply with report data in the output parameter. The structure for this response is defined as #bt_hogp_get_report_response_t*/
#define BT_HOGP_EVENT_SET_REPORT_IND                         (0x06)           /**< The set report request is received from host. The structure for this event is defined as #bt_hogp_set_report_ind_t. */

/**
 * @deprecated Use #BT_HOGP_EVENT_CONTROL_IND instead.
 */
#define BT_HOGP_EVENT_CONTORL_IND                            (BT_HOGP_EVENT_CONTROL_IND)      /**< This event will be phased out and removed in the next SDK major version. Do not use. */

/**
 * @brief The HOGP protocol mode.
 */
typedef uint8_t bt_hogp_protocol_mode_t;
#define BT_HOGP_BOOT_PROTOCOL_MODE       (0x00)            /**< It is only used for keyboard and mouse. */
#define BT_HOGP_REPORT_PROTOCOL_MODE     (0x01)            /**< The default mode after the HOGP is connected. */

/**
 * @brief The HOGP control command.
 */
typedef uint8_t bt_hogp_control_command_t;
#define BT_HOGP_CONTROL_COMMAND_SUSPEND          (0x00)    /**< Reduced power mode. */
#define BT_HOGP_CONTROL_COMMAND_EXIT_SUSPEND     (0x01)    /**< Exit the reduced power mode. */

/**
 * @brief The HOGP report type.
 */
typedef uint8_t bt_hogp_report_t;
#define BT_HOGP_INPUT_REPORT             (0x01)            /**< Input sent from the Device to the Host. */
#define BT_HOGP_OUTPUT_REPORT            (0x02)            /**< Output sent from the Host to the Device. */
#define BT_HOGP_FEATURE_REPORT           (0x03)            /**< Bi-directional transfer. */
#define BT_HOGP_REPORT_TYPE_NUM          (0x03)            /**< The report number. */

/**
 * @brief The HOGP report id type.
 */
typedef uint8_t bt_hogp_report_id_t;                       /**< The report id types. */

/**
 * @brief The HOGP information of flags define.
 */
#define BT_HOGP_INFO_FLAGS_REMOTEWAKE              (0x01)  /**< The value indicating whether HID Device is capable of sending a wake-signal to a HID Host. */
#define BT_HOGP_INFO_FLAGS_NORMALLY_CONNECTABLE    (0x02)  /**< The value indicating whether HID Device will be advertising when bonded but not connected. */

/**
 * @}
 */

/**
 * @defgroup BluetoothHOGP_struct Struct
 * @{
 * This section defines the structures for the HOGP.
 */

/**
 *  @brief This structure defines the HOGP information.
 */
typedef struct {
    uint16_t      bcdhid;               /**< HID class specification release number. */
    uint8_t       bcountrycode;         /**< Hardware target country. */
    uint8_t       flags;                /**< Bit0:RemoteWake, bit1: NormallyConnectable. */
} bt_hogp_information_t;

/**
 *  @brief This structure defines the report descriptor.
 */
typedef struct {
    uint8_t *descriptor;               /**< A pointer of the descriptor value. */
    uint16_t length;                   /**< The length of the descriptor value. */
} bt_hogp_report_descriptor_t;

/**
 *  @brief This structure defines the init parameter.
 */
typedef struct {
    bt_hogp_protocol_mode_t       mode;                    /**< The HOGP protocol mode. */
    bt_hogp_information_t         hid_information;         /**< The HOGP information. */
    bt_hogp_report_descriptor_t   report_descriptor;       /**< The HOGP report descriptor. */
} bt_hogp_init_parameter_t;

/**
*  @brief This structure define for #BT_HOGP_EVENT_CONNECT_IND, connect complete.
*/
typedef struct {
    bt_handle_t connection_handle;                         /**< The connection handle of the BLE. */
    bt_addr_t   *addr;                                     /**< The address of a remote device. */
} bt_hogp_connect_ind_t;

/**
*  @brief This structure define for #BT_HOGP_EVENT_DISCONNECT_IND, disconnect complete.
*/
typedef struct {
    bt_handle_t connection_handle;                         /**< The connection handle of the BLE. */
} bt_hogp_disconnect_ind_t;

/**
 * @deprecated Use #bt_hogp_connect_ind_t instead.
 */
typedef bt_hogp_connect_ind_t bt_hofp_connect_ind_t;       /* This structure will be phased out and removed in the next SDK major version. Do not use. */

/**
 * @deprecated Use #bt_hogp_disconnect_ind_t instead.
 */
typedef bt_hogp_disconnect_ind_t bt_hofp_disconnect_ind_t; /* This structure will be phased out and removed in the next SDK major version. Do not use. */

/**
*  @brief This structure define for #BT_HOGP_EVENT_GET_REPORT_ID_IND.
*/
typedef struct {
    bt_handle_t        connection_handle;                 /**< The connection handle of the BLE. */
    bt_hogp_report_t   type;                              /**< The report type. */
} bt_hogp_get_report_id_t;

/**
*  @brief This structure is defined to reply to report id when the BT_HOGP_EVENT_GET_REPORT_ID_IND is received.
*/
typedef struct {
    bt_hogp_report_id_t  report_id;                       /**< The report id. */
} bt_hogp_get_report_id_response_t;

/**
*  @brief This structure define for #BT_HOGP_EVENT_CONTORL_IND.
*/
typedef struct {
    bt_handle_t                 connection_handle;           /**< The HOGP connection handle of the current connection.*/
    bt_hogp_control_command_t   command;                     /**< The flag means suspend or exit suspend. */
} bt_hogp_control_ind_t;

/**
*  @brief This structure define for #BT_HOGP_EVENT_GET_REPORT_IND.
*/
typedef struct {
    bt_handle_t            connection_handle;                /**< The connection handle of the BLE. */
    bt_hogp_report_t       type;                             /**< The report type. */
    bt_hogp_report_id_t    report_id;                        /**< The report ID, it must exist in Boot mode or Report mode as a global item in the report descriptor.*/
    uint16_t               offset;                           /**< The value offset of get report response. */
} bt_hogp_get_report_ind_t;

/**
*  @brief This structure is defined to reply to report data when the BT_HOGP_EVENT_GET_REPORT_IND is received.
*/
typedef struct {
    uint8_t            report_length;                        /**< The length of the report value. */
    uint8_t            *report_data;                         /**< A pointer of the report value. */
} bt_hogp_get_report_response_t;

/**
*  @brief This structure define for #BT_HOGP_EVENT_SET_REPORT_IND.
*/
typedef struct {
    bt_handle_t         connection_handle;                   /**< The connection handle of the BLE. */
    bt_hogp_report_t    type;                                /**< The report type.*/
    uint8_t             report_id;                           /**< The report id. */
    uint8_t             report_length;                       /**< The length of the report value. */
    uint8_t             *report_data;                        /**< A pointer of the report value. */
} bt_hogp_set_report_ind_t;

/**
*  @brief This structure defines the packaged data sent to the remote device.
*/
typedef struct {
    bt_hogp_report_t    type;                                   /**< The report type. */
    uint16_t            packet_len;                             /**< The data length. */
    uint8_t             *packet;                                /**< The data to send to a remote device. */
} bt_hogp_data_t;

/**
 * @}
 */


/**
 * @brief The prototype of the callback function to handle HOGP events.
 * @param[in] event                 is the event type, define in the #bt_hogp_event_t.
 * @param[in] buffer                is the event payload.
 * @param[out] output               is the output parameter, fill the data when output is not NULL.
 * @return    For #BT_STATUS_SUCCESS, the callback function was called successful, otherwise failed.
 */
typedef bt_status_t (*bt_hogp_event_callback)(bt_hogp_event_t event, void *buffer, void *output);

/**
 * @brief This function is for the upper user to set init parameter and register an event handler.
 * @param[in] init_parameter          is the init parameter.
 * @param[in] callback                is a pointer for the callback function to be registered.
 * @return    For #BT_STATUS_SUCCESS, the function was called successful, otherwise failed.
 */
bt_status_t bt_hogp_init(bt_hogp_init_parameter_t *init_parameter, bt_hogp_event_callback callback);

/**
 * @brief This function is for the upper user to deinitialize HOGP.
 * @return    For #BT_STATUS_SUCCESS, the function was called successful, otherwise failed.
 */
bt_status_t bt_hogp_deinit(void);

/**
 * @brief This function is for the upper user to send data.
 * @param[in] connection_handle       is a connection handle of the BLE.
 * @param[in] data                    is a pointer for the data, the structure id defined as#bt_hogp_data_t.
 * @return    For #BT_STATUS_SUCCESS, the function was called successful, otherwise failed.
 */
bt_status_t bt_hogp_send_data(bt_handle_t connection_handle, bt_hogp_data_t *data);


/**
 * @}
 * @}
 */
#endif /* _BT_HOGP_H_ */

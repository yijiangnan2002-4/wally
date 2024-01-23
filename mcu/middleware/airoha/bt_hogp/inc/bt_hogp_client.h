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

#ifndef _BT_HOGP_CLIENT_H_
#define _BT_HOGP_CLIENT_H_

#include "bt_hogp.h"

/**
 * @brief The HID service UUID.
 */
#define BT_SIG_UUID16_HID               0x1812  /**< Human Interface Device service.*/
#define BT_SIG_UUID16_HIDS_INFO         0x2A4A  /**< HID Information service.*/
#define BT_SIG_UUID16_REPORT_MAP        0x2A4B  /**< Report Map service.*/
#define BT_SIG_UUID16_CTRL_POINT        0x2A4C  /**< HID Control Point service.*/
#define BT_SIG_UUID16_REPORT            0x2A4D  /**< Report service.*/
#define BT_SIG_UUID16_PROTOCOL_MODE     0x2A4E  /**< Report service.*/
#define BT_SIG_UUID16_CCCD              0x2902  /**< Client Characteristic Descriptor.*/
#define BT_SIG_UUID16_REPORT_REF        0x2908  /**< Report Reference Descriptor.*/

#define BT_HOGP_DISABLE_NOTIFICATION   0
#define BT_HOGP_ENABLE_NOTIFICATION    1
#define BT_HOGP_CCCD_VALUE_LEN         2

/**
 * @brief The HID service characteristic max number.
 */
#define BT_HOGP_CLIENT_MAX_LINK_NUM                     8
#define BT_HID_SERVICE_DISCOVERY_CHARACTER_MAX_NUMBER   20
#define BT_HID_SERVICE_DISCOVERY_DESCRIPTOR_MAX_NUMBER  10
#define BT_HID_SERVICE_REPORT_REF_MAX_NUMBER            10
#define BT_HID_SERVICE_REPORT_MAP_MAX_LENGTH            500

/**
 * @brief The HOGP Client state.
 */
typedef uint8_t bt_hogp_client_state_t;
#define HOGP_DISCOVEY_SERVICE_IDLE              (0)
#define HOGP_DISCOVEY_SERVICE_HID_INFO          (1)
#define HOGP_DISCOVEY_SERVICE_REPORT_MAP        (2)
#define HOGP_DISCOVEY_SERVICE_PROTOCOL_MODE     (3)
#define HOGP_DISCOVEY_SERVICE_REPORT            (4)
#define HOGP_DISCOVEY_SERVICE_REPORT_REF        (5)
#define HOGP_DISCOVEY_SERVICE_SET_CCCD          (6)
#define HOGP_DISCOVEY_SERVICE_COMPLETE          (7)

/**
 * @brief The event report to user
 */
typedef uint8_t bt_hogp_client_event_t;                                              /**< The type of HOGP Client event. */
#define BT_HOGP_CLIENT_EVENT_CONNECT_IND                            (0x01)           /**< The connection is connected. */
#define BT_HOGP_CLIENT_EVENT_DISCONNECT_IND                         (0x02)           /**< The connection is disconnected. */
#define BT_HOGP_CLIENT_EVENT_REPORT_MAP_IND                         (0x03)           /**< Notify app report map data. */
#define BT_HOGP_CLIENT_EVENT_INPUT_REPORT_IND                       (0x04)           /**< Notify app input report */

/**
 * @brief The HOGP Client report type.
 */
typedef uint8_t bt_hogp_client_report_t;
#define BT_HOGP_CLIENT_INPUT_REPORT             (0x01)            /**< Input sent from the Device to the Host. */
#define BT_HOGP_CLIENT_OUTPUT_REPORT            (0x02)            /**< Output sent from the Host to the Device. */
#define BT_HOGP_CLIENT_FEATURE_REPORT           (0x03)            /**< Bi-directional transfer. */
#define BT_HOGP_CLIENT_REPORT_TYPE_NUM          (0x03)            /**< The report number. */

/**
 * @brief The HOGP Client report id.
 */
typedef uint8_t bt_report_id_t;
#define BT_HOGP_KEYBOARD_INPUT_REPORT           (0x01)
#define BT_HOGP_MOUSE_INPUT_REPORT              (0x02)

/**
 * @brief The HOGP Client handlers.
 */
typedef struct{
    bt_handle_t hid_info_handle;
    bt_handle_t report_map_handle;
    bt_handle_t control_point_handle;
    bt_handle_t report_handle;
    bt_handle_t protocol_mode_handle;
}bt_hogp_client_handle_t;

/**
 * @brief The HOGP Client report reference descriptor value.
 */
typedef struct{
    bt_report_id_t report_id;
    uint8_t report_type;
    bt_handle_t cccd_handle;
    bt_handle_t report_ref_handle;
}bt_hogp_client_report_reference_descriptor_t;

/**
 * @brief The HOGP Client parameter value.
 */
typedef struct {
    bt_report_id_t      report_id;                              /**< The report id. */
    bt_handle_t         conn_handle;                            /**< The connection handle. */
} bt_hogp_client_para_t;

/**
 * @brief The HOGP Client report reference.
 */
typedef struct{
    uint8_t charc_num;
    uint8_t cccd_num;
    uint8_t ref_num;
    bt_handle_t value_handle[BT_HID_SERVICE_REPORT_REF_MAX_NUMBER];
    bt_hogp_client_report_reference_descriptor_t report_reference[BT_HID_SERVICE_REPORT_REF_MAX_NUMBER];
}bt_hogp_client_report_char_t;

typedef bt_status_t (*bt_hogp_client_event_callback)(bt_hogp_client_event_t event, bt_hogp_client_para_t *para, bt_status_t status, void *buffer, uint16_t length);

bt_status_t bt_hogp_client_send_output_report(uint8_t  report_id, bt_handle_t conn_handle, void *buffer, uint16_t length);

bt_status_t bt_hogp_client_init(bt_hogp_client_event_callback callback);

void bt_hogp_client_set_rediscovery_flag(bt_handle_t conn_handle, bool flag);

#endif /* _BT_HOGP_CLIENT_H_ */


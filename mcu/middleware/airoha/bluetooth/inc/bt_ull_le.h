/* Copyright Statement:
*
* (C) 2022 Airoha Technology Corp. All rights reserved.
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
* the License Agreement ("Permitted User"). If you are not a Permitted User,
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

#ifndef __BT_ULL_LE_H__
#define __BT_ULL_LE_H__


/**
 * @addtogroup Bluetooth
 * @{
 *
 * @addtogroup BluetoothULL ULL
 * @{
 * The Ultra Low Latency (ULL) is Airoha proprietary profile that defines the low wireless audio latency.
 * This section introduces the ULL profile APIs and events and how to use this module.
 *
 * Terms and Acronyms
 * ======
 * |Terms                         |Details                                                                  |
 * |------------------------------|-------------------------------------------------------------------------|
 * |\b ULL                        | Ultra low latency.                   |
 * |\b AIR CIS                    | Airoha connected isochronous stream. |
 * |\b AIR CIG                    | Airoha connected isochronous group.  |
 * |\b AIR ISO DATA PATH          | Airoha isochronous data path.        |
 *
 * @section bt_ull_api_usage How to use this module
 *  - Step1: Mandatory, initialize Bluetooth Ultra Low Latency profile.
 *  - Sample code:
 *    @code
 *        bt_ull_le_init();
 *    @endcode
 *
 *  - Step2: Mandatory, implement the event callback to process the ULL events, such as #BT_ULL_LE_AIR_CIS_ESTABLISHED_IND, #BT_ULL_LE_AIR_CIS_DISCONNECT_COMPLETE_IND.
 *  - Sample code:
 *    @code
 *       void bt_app_event_callback(bt_msg_type_t msg_type, bt_status_t status, void *data)
 *       {
 *          switch (msg_type)
 *          {
 *              case BT_ULL_LE_AIR_CIS_CONNECT_REQUEST_IND:
 *              {
 *                  bt_ull_le_air_cis_request_ind_t *request =
 *                      (bt_ull_le_air_cis_request_ind_t *)data;;
 *                  bt_ull_le_air_cis_connect_request_handler(status, request);
 *                  // Respond to the connection request.
 *                  break;
 *              }
 *              case BT_ULL_LE_AIR_CIS_ESTABLISHED_IND:
 *              {
 *                  bt_ull_le_air_cis_established_ind_t *established =
 *                      (bt_ull_le_air_cis_established_ind_t *)data;;
 *                  bt_ull_le_air_cis_established_handler(status, established);
 *                  // Handle the air cis established event.
 *                  break;
 *              }
 *              case BT_ULL_LE_AIR_CIS_DISCONNECT_COMPLETE_IND:
 *              {
 *                  bt_ull_le_air_cis_disconnect_complete_ind_t *disconnect =
 *                      (bt_ull_le_air_cis_disconnect_complete_ind_t *)data;
 *                  bt_ull_le_air_cis_disconnect_handler(status, disconnect);
 *                  // Handle the air cis disconnect complete event.
 *                  break;
 *              }
 *              case BT_ULL_LE_SETUP_AIR_ISO_DATA_PATH_CNF:
 *              {
 *                  bt_ull_le_setup_air_iso_data_path_cnf_t *iso_cnf =
 *                      (bt_ull_le_setup_air_iso_data_path_cnf_t *)data;;
 *                  bt_ull_le_air_cis_set_air_iso_data_path_cnf_handler(status, iso_cnf);
 *                  // Handle the air iso data path confirmation event.
 *                  break;
 *              }
 *              case BT_ULL_LE_CREATE_AIR_CIS_CNF:
 *              {
 *                  ...
 *                  break;
 *              }
 *
 *              ...
 *
 *              default:
 *              {
 *                  break;
 *              }
 *           }
 *       }
 *    @endcode
 *
 *  - Step3: Mandatory, set an air cig parameter to bt controller.
 *  - Sample code:
 *    @code
 *        bt_ull_le_air_cig_params_t params;
 *        uint8_t cis_count = 0x02;
 *        uint8_t count = 0;
 *        bt_ull_le_air_cis_params_t *cis_list = bt_ull_le_srv_memory_alloc(sizeof(bt_ull_le_air_cis_params_t) * cis_count);
 *        for (count = 0; count < cis_count; count ++) {
 *            cis_list[count].cis_id = BT_ULL_LE_CONN_SRV_AIR_CIS_ID_INVANLID + count;
 *            cis_list[count].max_sdu_m_to_s = BT_ULL_LE_DEFAULT_DOWNLINK_SDU_SIZE_190/cis_count;
 *            cis_list[count].max_sdu_s_to_m = BT_ULL_LE_DEFAULT_UPLINK_SDU_SIZE_40;
 *            cis_list[count].phy_m_to_s = BT_ULL_LE_CONN_SRV_PHY_LE_2M;
 *            cis_list[count].phy_s_to_m = BT_ULL_LE_CONN_SRV_PHY_LE_2M;
 *        }
 *        params.cig_id = BT_ULL_LE_CONN_SRV_AIR_CIG_ID_1;
 *        params.client_type = BT_ULL_HEADSET_CLIENT;
 *        params.sdu_interval_m_to_s = BT_ULL_LE_SDU_INTERVAL_5000_US;
 *        params.sdu_interval_s_to_m = BT_ULL_LE_SDU_INTERVAL_5000_US;
 *        params.sca = 0x00;
 *        params.max_share_count = 0x02;
 *        params.iso_interval = BT_ULL_LE_CONN_SRV_ISO_INTERVAL_5MS;
 *        params.max_uplink_num = 0x01;
 *        params.ft_m_to_s = BT_ULL_LE_CONN_SRV_FT_1;
 *        params.ft_s_to_m = BT_ULL_LE_CONN_SRV_FT_1;
 *        params.cis_count = cis_count;
 *        params.cis_list = cis_list;
 *        bt_ull_le_set_air_cig_parameters(&params);
 *        bt_ull_le_srv_memory_free(cis_list);
 *
 *    @endcode
 *  - Step4: Mandatory, create the air cis connection with remote device to transport audio streaming.
 *  - Sample code:
 *    @code
 *        bt_ull_le_create_air_cis_t param;
 *        uint8_t cis_count = 0x02;
 *        uint8_t count = 0;
 *        bt_ull_le_air_cis_set_t *cis_list = bt_ull_le_srv_memory_alloc(sizeof(bt_ull_le_air_cis_set_t) * cis_count);
 *        for (i = 0; i < cis_count; i ++) {
 *            cis_list[i].cis_connection_handle = cis_handle;
 *            // Cis handle return by controller after set air cig parameter.
 *            cis_list[i].acl_connection_handle = acl_handle;
 *            // Acl handle realte to the le acl link
 *            cis_list[i].ul_enable = (cis == BT_ULL_LE_CONN_SRV_AIR_CIS_ID_SINK1) ? true : false;
 *        }
 *        param.cis_count = cis_count;
 *        param.cis_list = cis_list;
 *        bt_ull_le_create_air_cis(&param);
 *        bt_ull_le_srv_memory_free(cis_list);
 *    @endcode
 *
 *  - Step5: Mandatory, set the air iso data path to enable audio streaming.
 *  - Sample code:
 *    @code
 *        bt_ull_le_setup_air_iso_data_path_t param;
 *        param.cis_connection_handle = cis_handle;
 *        param.data_path_id = BT_ULL_LE_CONN_SRV_DATA_PATH_ID_1;
 *        param.codec_format = 0x00;
 *        param.company_id = 0x00;
 *        param.vendor_codec_id = 0x00;
 *        param.controller_delay = 0x00;
 *        bt_ull_le_setup_air_iso_data_path(&param);
 *    @endcode
 *
 */

#include "bt_type.h"
#include "bt_system.h"
#include "bt_hci.h"

#define BT_ULL_LE_ACCESS_ADDR_LENGTH                             (0x04)                         /**< Access address length. */
#define BT_ULL_LE_FT_LABEL_MAX                                   (0x04)
#define BT_ULL_LE_AIR_CIS_CONNECT_REQUEST_IND                    (BT_MODULE_ULL | 0x0001)       /**< AIR CIS connection request indication with #bt_ull_le_air_cis_request_ind_t. */
#define BT_ULL_LE_AIR_CIS_ESTABLISHED_IND                        (BT_MODULE_ULL | 0x0002)       /**< AIR CIS established indication with #bt_ull_le_air_cis_established_ind_t. */
#define BT_ULL_LE_AIR_CIS_DISCONNECT_COMPLETE_IND                (BT_MODULE_ULL | 0x0003)       /**< AIR CIS disconnected indication with #bt_ull_le_air_cis_disconnect_t. */
#define BT_ULL_LE_AIR_CIG_PARAMS_CHANGED_IND                     (BT_MODULE_ULL | 0x0004)       /**< AIR CIG parameters changed indication with #bt_ull_le_air_cig_params_changed_ind_t. */
#define BT_ULL_LE_AIR_CIS_UPLINK_ACTIVIATED_IND                  (BT_MODULE_ULL | 0x0005)       /**< Uplink of AIR CIS activated indication with #bt_ull_le_activate_uplink_t.. */
#define BT_ULL_LE_AIR_CIS_QOS_REPORT_IND                         (BT_MODULE_ULL | 0x0030)       /**< AIR CIS QoS indication with #bt_ull_le_qos_report_ind_t. */

#define BT_ULL_LE_SET_AIR_CIG_PARAMS_CNF                         (BT_MODULE_ULL | 0x0006)       /**< Set AIR CIG parameters confirmation with #bt_ull_le_set_air_cig_params_cnf_t. */
#define BT_ULL_LE_REMOVE_AIR_CIG_CNF                             (BT_MODULE_ULL | 0x0008)       /**< Remove AIR CIG confirmation with #bt_ull_le_remove_air_cig_cnf_t. */
#define BT_ULL_LE_CREATE_AIR_CIS_CNF                             (BT_MODULE_ULL | 0x0007)       /**< Create AIR CIS connection confirmation with NULL payload. */
#define BT_ULL_LE_ACCEPT_AIR_CIS_CONNECT_REQUEST_CNF             (BT_MODULE_ULL | 0x0009)       /**< Accept AIR CIS connection request confirmation with NULL payload. */
#define BT_ULL_LE_REJECT_AIR_CIS_CONNECT_REQUEST_CNF             (BT_MODULE_ULL | 0x000A)       /**< Reject AIR CIS connection request confirmation with NULL payload. */
#define BT_ULL_LE_SETUP_AIR_ISO_DATA_PATH_CNF                    (BT_MODULE_ULL | 0x000B)       /**< Setup AIR ISO data path confirmation with #bt_ull_le_setup_air_iso_data_path_cnf_t. */
#define BT_ULL_LE_REMOVE_AIR_ISO_DATA_PATH_CNF                   (BT_MODULE_ULL | 0x000C)       /**< Remove AIR ISO data path confirmation with #bt_ull_le_remove_air_iso_data_path_cnf_t. */
#define BT_ULL_LE_DISCONNECT_AIR_CIS_CNF                         (BT_MODULE_ULL | 0x000D)       /**< Disconnect AIR CIS confirmation with NULL payload. */
#define BT_ULL_LE_CHANGE_AIR_CIG_PARAMS_CNF                      (BT_MODULE_ULL | 0x000E)       /**< Change AIR CIG parameters confirmation with #bt_ull_le_air_change_air_params_cnf_t. */
#define BT_ULL_LE_UNMUTE_AIR_CIS_CNF                             (BT_MODULE_ULL | 0x000F)       /**< Unmute AIR CIS confirmation with NULL payload. */
#define BT_ULL_LE_ACTIVATE_UPLINK_CNF                            (BT_MODULE_ULL | 0x0010)       /**< Activate uplink of AIR CIS confirmation with NULL payload. */
#define BT_ULL_LE_SET_AIR_CIG_PARAMS_TABLE_CNF                   (BT_MODULE_ULL | 0x0011)       /**< Set air parameters table confirmation with NULL payload. */
#define BT_ULL_LE_SET_ADV_SCAN_ACCESS_DDRESS_CNF                 (BT_MODULE_ULL | 0x0012)       /**< Set adv/scan access address confirmation with NULL payload. */
#define BT_ULL_LE_CHANGE_ENCODE_TIME_CNF                         (BT_MODULE_ULL | 0x0013)       /**< Set air parameters table confirmation with NULL payload. */
#define BT_ULL_LE_ENABLE_QOS_REPORT_CNF                          (BT_MODULE_ULL | 0x0014)       /**< Enable AIR CIS QoS report confirmation with NULL payload. */

/*************************************************************************AIR HID CIS**********************************************************************************************************/
/*******************************************************************************************************************************************************************************
* 
* Define AIR HID CIS event type
*
*******************************************************************************************************************************************************************************/
#define BT_ULL_LE_AIR_HID_CIS_ESTABLISHED_IND                    (BT_MODULE_ULL | 0x0030)       /**< AIR HID CIS established indication with #bt_ull_le_air_hid_cis_established_ind_t. */
#define BT_ULL_LE_AIR_HID_CIS_DISCONNECT_COMPLETE_IND            (BT_MODULE_ULL | 0x0031)       /**< AIR HIID CIS disconnected indication with #bt_ull_le_air_hid_cis_disconnect_t. */
#define BT_ULL_LE_SET_AIR_HID_CIG_PARAMS_CNF                     (BT_MODULE_ULL | 0x0032)       /**< Set AIR HID CIG parameters confirmation with #bt_ull_le_set_air_hid_cig_cnf_t. */
#define BT_ULL_LE_REMOVE_AIR_HID_CIG_CNF                         (BT_MODULE_ULL | 0x0033)       /**< Remove AIR HID CIG confirmation with #bt_ull_le_remove_air_hid_cig_cnf_t. */
#define BT_ULL_LE_CREATE_AIR_HID_CIS_CNF                         (BT_MODULE_ULL | 0x0034)       /**< Create AIR HID CIS connection confirmation with NULL payload. */
#define BT_ULL_LE_SYNC_AIR_HID_CIS_CNF                           (BT_MODULE_ULL | 0x0035)       /**< Create AIR HID CIS connection confirmation with NULL payload. */
#define BT_ULL_LE_CANCEL_CREATE_AIR_HID_CIS_CNF                  (BT_MODULE_ULL | 0x0036)       /**< Create AIR HID CIS connection confirmation with NULL payload. */
#define BT_ULL_LE_CANCEL_SYNC_AIR_HID_CIS_CNF                    (BT_MODULE_ULL | 0x0037)       /**< Create AIR HID CIS connection confirmation with NULL payload. */
#define BT_ULL_LE_SETUP_AIR_HID_ISO_DATA_PATH_CNF                (BT_MODULE_ULL | 0x0038)       /**< Setup AIR HID ISO data path confirmation with #bt_ull_le_setup_air_hid_iso_data_path_cnf_t. */
#define BT_ULL_LE_REMOVE_AIR_HID_ISO_DATA_PATH_CNF               (BT_MODULE_ULL | 0x0039)       /**< Remove AIR HID ISO data path confirmation with #bt_ull_le_remove_air_hid_iso_data_path_cnf_t. */
#define BT_ULL_LE_DISCONNECT_AIR_HID_CIS_CNF                     (BT_MODULE_ULL | 0x003A)       /**< Disconnect AIR HID CIS confirmation with NULL payload. */
#define BT_ULL_LE_SET_AIR_HID_CIS_SUB_EVENT_CNF                  (BT_MODULE_ULL | 0x003B)       /**< Set AIR HID CIS connectuion sub event with NULL payload. */

/***********************************************************************************************************************************************************************************/
#define BT_ULL_LE_REPLY_AIR_CIS_ACTION_ACCEPT           0x00    /**< Action accept. */
#define BT_ULL_LE_REPLY_AIR_CIS_ACTION_REJECT           0x01    /**< Action reject. */
typedef uint8_t bt_ull_le_reply_air_cis_action_t; /**< Reply AIR CIS action. */

#define BT_ULL_LE_AIR_ISO_DATA_PATH_DIRECTION_INPUT     0x00    /**< Input. Audio -> Controller*/
#define BT_ULL_LE_AIR_ISO_DATA_PATH_DIRECTION_OUTPUT    0x01    /**< Output. Controller -> Audio*/
typedef uint8_t bt_ull_le_air_iso_data_path_direction_t; /**< AIR ISO data path direction. */

#define BT_ULL_LE_AIR_ISO_DATA_PATH_ID_HCI              0x00    /**< HCI. */
#define BT_ULL_LE_AIR_ISO_DATA_PATH_ID_CHANNEL_1        0x01    /**< Channel 1. */
#define BT_ULL_LE_AIR_ISO_DATA_PATH_ID_CHANNEL_2        0x02    /**< Channel 2. */
#define BT_ULL_LE_AIR_ISO_DATA_PATH_ID_CHANNEL_3        0x03    /**< Channel 3. */
#define BT_ULL_LE_AIR_ISO_DATA_PATH_ID_CHANNEL_4        0x04    /**< Channel 4. */
#define BT_ULL_LE_AIR_ISO_DATA_PATH_ID_SPK_SPECIAL      0x30    /**< Special channel for ULL speaker. */
#define BT_ULL_LE_AIR_ISO_DATA_PATH_ID_DISABLE          0xFF    /**< Disable. */
typedef uint8_t bt_ull_le_air_iso_data_path_id_t; /**< AIR ISO data path ID. */

#define BT_ULL_LE_LEVEL_DVFS_HV                         0x00    /**< DVFS high. */
#define BT_ULL_LE_LEVEL_DVFS_LV                         0x01    /**< DVFS low. */
typedef uint8_t bt_ull_le_level_dvfs_t; /**< AIR ISO data path ID. */

/**
 * @brief       The structure definition for the command to disconnect AIR CIS.
 */
typedef bt_hci_cmd_disconnect_t bt_ull_le_air_cis_disconnect_t;

/**
 *  @brief      The structure definition for the command to reply AIR CIS request.
 */
typedef struct {
    bt_ull_le_reply_air_cis_action_t action;            /**< The action for replying AIR CIS request. */
    union {
        bt_hci_le_accept_cis_request_params_t accept;   /**< The accept parameters. */
        bt_hci_le_reject_cis_request_params_t reject;   /**< The reject parameters. */
    };
} bt_ull_le_reply_air_cis_request_t;

/**
 * @brief       The structure definition for the event of AIR CIS request.
 */
BT_PACKED (
typedef struct {
    bt_handle_t       acl_connection_handle;            /**< Connection handle of the ACL. */
    bt_handle_t       cis_connection_handle;            /**< Connection handle of the AIR CIS. */
    uint8_t           cig_id;                           /**< AIR CIG ID. */
    uint8_t           cis_id;                           /**< AIR CIS ID. */
}) bt_ull_le_air_cis_request_ind_t;

/**
 * @brief       The structure definition for the event of AIR CIS established.
 */
BT_PACKED (
typedef struct {
    uint8_t           status;                           /**< The status of the AIR CIS. */
    bt_handle_t       cis_connection_handle;            /**< Connection handle of the AIR CIS. */
    uint8_t           label_idx;                        /**< The index of cig parameters table. */
    uint8_t           ft_idx;                           /**< The index of the flush timeout ft_label. */
    uint32_t          cig_sync_delay : 24;              /**< The maximum time, in microseconds, for transmission of PDUs of all AIR CISes in an AIR CIG in an isochronous interval. */
    uint32_t          cis_sync_delay : 24;              /**< The maximum time, in microseconds, for transmission of PDUs of an AIR CIS in the isochronous interval. */
    uint32_t          transport_latency_m_to_s : 24;    /**< The maximum time, in microseconds, for transmission of SDUs of all AIR CISes in an AIR CIG from slave to master. */
    uint32_t          transport_latency_s_to_m : 24;    /**< The maximum time, in microseconds, for transmission of SDUs of all AIR CISes in an AIR CIG from master to slave. */
    uint8_t           phy_m_to_s;                       /**< The transmitter PHY of packets used from the master to slave. */
    uint8_t           phy_s_to_m;                       /**< The transmitter PHY of packets used from the slave to master. */
    bool              dl_enable;                        /**< The down link active state, true: active, false: inactive. */
    bool              ul_enable;                        /**< The uplink active state, true: active, false: inactive. */
    uint8_t           nse;                              /**< Maximum number of subevents in each isochronous event. */
    uint8_t           share_count;                      /**< Maximum number of subevents in each isochronous event. */
    uint8_t           bn_m_to_s;                        /**< The burst number for master to slave transmission. */
    uint8_t           bn_s_to_m;                        /**< The burst number for slave to master transmission. */
    uint8_t           ft_m_to_s;                        /**< The flush timeout, in multiples of the ISO_Interval for the AIR CIS, for each payload sent from the master to the slave. */
    uint8_t           ft_s_to_m;                        /**< The flush timeout, in multiples of the ISO_Interval for the AIR CIS, for each payload sent from the slave to the master.*/
    uint16_t          max_pdu_m_to_s;                   /**< Maximum size, in octets, of the payload from master to slave. */
    uint16_t          max_pdu_s_to_m;                   /**< Maximum size, in octets, of the payload from slave to master.*/
    uint16_t          iso_interval;                     /**< The time between two consecutive AIR CIS anchor points.*/
}) bt_ull_le_air_cis_established_ind_t;

/**
 *  @brief      The structure definition for the event of disconnect complete.
 */
BT_PACKED (
typedef struct {
    bt_hci_status_t   status;                           /**< Status. */
    bt_handle_t       cis_connection_handle;            /**< AIR CIS connection handle. */
    bt_hci_disconnect_reason_t reason;                  /**< Disconnect reason. */
}) bt_ull_le_air_cis_disconnect_complete_ind_t;

/**
 *  @brief      The structure definition for the AIR CIS parameters of AIR CIG structure.
 */
BT_PACKED (
typedef struct {
    uint8_t cis_id;                                     /**< identifies an AIR CIS and is set by the master's Host and passed to the slave's Host through the Link Layers during the process of establishing an AIR CIS. */
    uint16_t max_sdu_m_to_s;                            /**< The maximum size of an SDU from the master's Host. */
    uint16_t max_sdu_s_to_m;                            /**< The maximum size of an SDU from the slave's Host. */
    uint8_t phy_m_to_s;                                 /**< identifies which PHY to use for transmission from the master to the slave. */
    uint8_t phy_s_to_m;                                 /**< identifies which PHY to use for transmission from the slave to the master. */
}) bt_ull_le_air_cis_params_t;

/**
 *  @brief      The structure definition for the AIR CIG parameters.
 */
BT_PACKED (
typedef struct {
    uint8_t cig_id;                                     /**< Identifies a CIG. This parameter is allocated by the master's Host and passed to the slave's Host through the Link Layers during the process of establishing a CIS. */
    uint8_t client_type;                                /**< The client type of connecting with local device. */
    uint8_t default_label_index;                        /**< The default index of cig parameters table with #bt_ull_le_set_air_label_params_table_t*/
    uint8_t default_ft_index;                           /**< The default index of the flush timeout ft_label with #bt_ull_le_set_air_label_params_table_t*/
    uint32_t sdu_interval_m_to_s : 24;                  /**< The interval, in microseconds, of periodic SDUs. (0x000FF to 0xFFFFF) */
    uint32_t sdu_interval_s_to_m : 24;                  /**< The interval, in microseconds, of periodic SDUs. (0x000FF to 0xFFFFF) */
    uint8_t sca;                                        /**< The worst-case sleep clock accuracy of all the slaves that will participate in the AIR CIG. */
    uint8_t max_share_count;                            /**< The maximum number of times that every CIS Data PDU should be retransmitted. */
    uint16_t iso_interval;                              /**< The time between two consecutive BIG anchor points. (Range: 0x0004 to 0x0C80, Time = N * 1.25 ms.)*/
    uint8_t max_uplink_num;                             /**< The max uplink number in the AIR CIG*/
    uint8_t cis_count;                                  /**< Total number of CISes in the CIG being added or modified. The maximum value is limited to 16. */
    bt_ull_le_air_cis_params_t *cis_list;               /**< The parameters of each CIS in the AIR CIG. */
})bt_ull_le_air_cig_params_t;

/**
 *  @brief      The structure definition for the AIR CIS set parameters.
 */

BT_PACKED (
typedef struct {
    bt_handle_t cis_connection_handle;                  /**< The connection handle of the CIS to be established. */
    bt_handle_t acl_connection_handle;                  /**< The connection handle of the ACL connection associated with each CIS to be established. */
    bool        ul_enable;                              /**< The state of the uplink in the air CIS. */
}) bt_ull_le_air_cis_set_t;

/**
 *  @brief      The structure definition for the command to create the AIR CIS.
 */
 BT_PACKED(
typedef struct {
    uint8_t cis_count;                                  /**< The total number of CISes established by this command. */
    bt_ull_le_air_cis_set_t *cis_list;                  /**< The connection handle list of the CIS to be established. */
}) bt_ull_le_create_air_cis_t;

/**
 *  @brief      The structure definition for the command to remove the AIR CIG parameters.
 */
BT_PACKED (
typedef struct {
    uint8_t cig_id;                                     /**< Identifies an AIR CIG. */
}) bt_ull_le_remove_air_cig_t;

/**
 *  @brief      The structure definition for the command to change the AIR CIG parameters.
 */
BT_PACKED(
typedef struct {
    uint8_t cig_id;                                     /**< identifies a CIG. This parameter is allocated by the master's Host and passed to the slave's Host through the Link Layers during the process of establishing a CIS. */
    uint8_t cig_params_index;                           /**< Current latency of CIS in the CIG. */
    uint8_t ft_label_index;                             /**< The default index of the flush timeout ft_label with #bt_ull_le_set_air_label_params_table_t*/
}) bt_ull_le_change_air_cig_params_t;

/**
 *  @brief      The structure definition for the air label parameters.
 */
BT_PACKED(
typedef struct {
    uint32_t sdu_interval_m_to_s : 24;                  /**< The interval, in microseconds, of periodic SDUs. (0x000FF to 0xFFFFF) */
    uint32_t sdu_interval_s_to_m : 24;                  /**< The interval, in microseconds, of periodic SDUs. (0x000FF to 0xFFFFF) */
    uint8_t max_share_count;                            /**< The maximum number of times that every CIS Data PDU should be retransmitted. */
    uint16_t iso_interval;                              /**< The time between two consecutive BIG anchor points. (Range: 0x0004 to 0x0C80, Time = N * 1.25 ms.)*/
    uint8_t max_uplink_num;                             /**< The max uplink number in the AIR CIG*/
    uint8_t cis_count;                                  /**< Total number of CISes in the CIG being added or modified. The maximum value is limited to 16. */
    bt_ull_le_air_cis_params_t *cis_list;               /**< The parameters of each CIS in the AIR CIG. */

}) bt_ull_le_air_label_params_t;

/**
 *  @brief      The structure definition for the command to set the air parameters table.
 */
BT_PACKED(
typedef struct {
    bt_handle_t connection_handle;                   /**< The connection handle of the ACL. */
    uint8_t client_type;                             /**< The client type of connecting with local device. */
    bt_ull_le_level_dvfs_t dvfs_level;               /**< The DVFS level, 0x00: HV (default mode), 0x01: LV (low power mode). */
    uint8_t ft_label[BT_ULL_LE_FT_LABEL_MAX];        /**< The flush timeout in multiples of ISO_Interval for each payload sent from the slave to master. (0x01 to 0xFF) */
    uint8_t label_count;                             /**< The total number of the parameters table. */
    bt_ull_le_air_label_params_t *label_param_list;  /**< The parameters of each CIG in the CIG table. */
}) bt_ull_le_set_air_label_params_table_t;

/**
 *  @brief      The structure definition for the command to unmute the AIR CIS.
 */
BT_PACKED(
typedef struct {
    bt_handle_t cis_connection_handle;                  /**< The connection handle list of the CIS to be established. */
}) bt_ull_le_unmute_air_cis_t;

/**
 *  @brief      The structure definition for the command to activate the uplink.
 */
BT_PACKED(
typedef struct {
    bt_handle_t cis_connection_handle;                  /**< The connection handle list of the CIS to be established. */
}) bt_ull_le_activate_uplink_t;


/**
 *  @brief      The structure definition for the command to Setup AIR ISO data path.
 */
BT_PACKED (
typedef struct {
    bt_handle_t cis_connection_handle;                  /**< Connection handle of the CIS. Range: 0x0000 to 0x0EFF */
    bt_ull_le_air_iso_data_path_direction_t direction;  /**< The direction for which the data path is being configured. */
    bt_ull_le_air_iso_data_path_id_t data_path_id;      /**< The data transport path used. When set to a value in the range 0x01 to 0xFE, the data path shall use a vendor-specific transport interface */
    uint8_t codec_format;                               /**< The coding format used over the air.*/
    uint16_t company_id;                                /**< Company ID, see Assigned Numbers for Company Identifier. Shall be ignored if coding format is not 0xFF.*/
    uint16_t vendor_codec_id;                           /**< Vendor-defined codec ID. Shall be ignored if octet 0 is not 0xFF.*/
    uint32_t controller_delay : 24;                     /**< Controller delay in microseconds Range: 0x000000 to 0x3D0900 Time range: 0 s to 4 s */
    uint8_t codec_configuration_length;                 /**< Length of codec configuration. */
    uint8_t *codec_configuration;                       /**< Codec-specific configuration data. */
}) bt_ull_le_setup_air_iso_data_path_t;

/**
 *  @brief      The structure definition for the command to set the access address.
 */
BT_PACKED (
typedef struct {
    uint8_t acess_addr[BT_ULL_LE_ACCESS_ADDR_LENGTH];   /**< Access address for adv/scan*/
}) bt_ull_le_set_adv_scan_access_addr_t;

/**
 *  @brief      The structure definition for the command to change encode time.
 */
BT_PACKED(
typedef struct {
    uint32_t encode_time;                  /**< The encode time of dsp down link for dongle . */
}) bt_ull_le_change_encode_time_t;

/**
 *  @brief      The structure definition for the command to enable/disable Quality of Service report.
 */
BT_PACKED(
typedef struct {
    bt_handle_t acl_connection_handle;                  /**< The connection handle of the ACL. */
    uint8_t     enable;                                 /**< Enable or disable the Quality of Service report. */
    uint16_t    report_interval;                        /**< The interval of the Quality of Service report, unit: 10ms, value range: 10 ~ 100. */
    uint16_t    crc_threshold;                          /**< The report threshold of CRC error . */
    uint16_t    rx_timeout_threshold;                   /**< The report threshold of RX timeout error . */
    uint8_t     flush_timeout_threshold;                /**< The report threshold of flush timeout error . */
}) bt_ull_le_enable_qos_report_t;

/**
 *  @brief      The structure definition for #BT_ULL_LE_AIR_CIS_QOS_REPORT_IND. 
 */
BT_PACKED(
typedef struct {
    bt_hci_status_t   status;                                 /**< Status. */
    bt_handle_t       acl_connection_handle;                  /**< The connection handle of the ACL. */
    uint16_t          crc;                                    /**< The number of CRC error. */
    uint16_t          rx_timeout;                             /**< The number of RX timeout error . */
    uint8_t           flush_timeout;                          /**< The number of flush timeout error . */
}) bt_ull_le_qos_report_ind_t;


/** 
 * @brief      The structure definition for #BT_ULL_LE_AIR_CIG_PARAMS_CHANGED_IND. 
 */
BT_PACKED (
typedef struct {
    bt_hci_status_t   status;                           /**< Status. */
    uint8_t           label_index;                     /**< Current CIG parameters label index. */
    uint8_t           ft_label_index;                   /**< Current FT table index. */
}) bt_ull_le_air_cig_params_changed_ind_t;

/** @brief      The structure definition for #BT_ULL_LE_AIR_CIS_UPLINK_ACTIVIATED_IND. */
BT_PACKED (
typedef struct {
    bt_hci_status_t   status;                           /**< Status. */
    bt_handle_t       cis_connection_handle;            /**< Connection handle of the CIS or BIS. Range: 0x0000 to 0x0EFF */
    bool              dl_enable;                        /**< The dplink of CIS in the CIG is active or inactive. */
    bool              ul_enable;                        /**< The uplink of CIS in the CIG is active or inactive. */
}) bt_ull_le_air_cis_uplink_activiated_ind_t;

/**
 * @brief       The structure define for the associated parameter type in the callback for #BT_ULL_LE_SET_AIR_CIG_PARAMS_TABLE_CNF.
 */
BT_PACKED(
typedef struct {
    bt_hci_status_t   status;               /**< Status. */
    bt_handle_t       handle;               /**< Connection handle of the ACL. */
}) bt_ull_le_air_set_air_cig_table_cnf_t;

/**
 * @brief       The structure define for the associated parameter type in the callback for #BT_ULL_LE_CHANGE_AIR_CIG_PARAMS_CNF.
 */
BT_PACKED(
typedef struct {
    bt_hci_status_t   status;               /**< Status. */
    uint8_t           cig_id;               /**< identifies a CIG. This parameter is allocated by the master's Host and passed to the slave's Host through the Link Layers during the process of establishing a CIS. */
    uint8_t           label_index;          /**< Current CIG parameters tabel index. */
    uint8_t           ft_label_index;       /**< Current FT table index. */
}) bt_ull_le_air_change_air_params_cnf_t;


/**
 *  @brief      The structure definition for the command to remove the AIR ISO data path.
 */
typedef bt_hci_le_remove_iso_data_path_params_t bt_ull_le_remove_air_iso_data_path_t;

/**
 * @brief This structure defines the associated parameter type in the callback for #BT_ULL_LE_SETUP_AIR_ISO_DATA_PATH_CNF event.
 */
typedef bt_hci_le_setup_iso_data_path_t bt_ull_le_setup_air_iso_data_path_cnf_t;

/**
 * @brief This structure defines the associated parameter type in the callback for #BT_ULL_LE_REMOVE_AIR_ISO_DATA_PATH_CNF event.
 */
typedef bt_hci_le_remove_iso_data_path_t bt_ull_le_remove_air_iso_data_path_cnf_t;

/**
 * @brief This structure defines the associated parameter type in the callback for #BT_ULL_LE_SET_AIR_CIG_PARAMS_CNF event.
 */
typedef bt_hci_le_set_cig_params_t bt_ull_le_set_air_cig_params_cnf_t;

/**
 * @brief This structure defines the associated parameter type in the callback for #BT_ULL_LE_REMOVE_AIR_CIG_CNF event.
 */
typedef bt_hci_le_remove_cig_t bt_ull_le_remove_air_cig_cnf_t;

/*******************************************************************************************************************************************************************************
* 
* Define AIR HID CIS data type
*
*******************************************************************************************************************************************************************************/

/** @brief      The structure definition for the AIR HID CIS parameters of #bt_ull_le_set_air_hid_cig_t. */
BT_PACKED (
typedef struct {
    uint8_t   cis_id;                                     /**< Identifies an AIR HID CIS and is set by the master's Host and passed to the slave's Host through the Link Layers during the process of establishing an AIR HID CIS. */
    uint8_t   slave_type;                                 /**< The type of slave device . */
    uint32_t  sdu_interval_m_to_s : 24;                   /**< The interval, in microseconds, of periodic SDUs. (0x000FF to 0xFFFFF) */
    uint32_t  sdu_interval_s_to_m : 24;                   /**< The interval, in microseconds, of periodic SDUs. (0x000FF to 0xFFFFF) */
    uint8_t   share_num;                                  /**< The maximum number of times that every CIS Data PDU should be retransmitted. */
    uint8_t   max_uplink_num;                             /**< The max uplink number in the AIR HID CIG*/
    uint16_t  max_sdu_m_to_s;                             /**< The maximum size of an SDU from the master's Host. */
    uint16_t  max_sdu_s_to_m;                             /**< The maximum size of an SDU from the slave's Host. */
    uint8_t   phy_m_to_s;                                 /**< identifies which PHY to use for transmission from the master to the slave. */
    uint8_t   phy_s_to_m;                                 /**< identifies which PHY to use for transmission from the slave to the master. */
    uint8_t   ft_m_to_s;                                  /**< The flush timeout in multiples of ISO_Interval for each payload sent from the master to slave. (0x01 to 0xFF) */
    uint8_t   ft_s_to_m;                                  /**< The flush timeout in multiples of ISO_Interval for each payload sent from the slave to master. (0x01 to 0xFF) */
}) bt_ull_le_air_hid_cis_params_t;

/**
 *  @brief      The structure definition for the command to set AIR HID CIG parameters.
 */
BT_PACKED(
typedef struct {
    uint8_t  cig_id;                                      /**< Identifies a AIR HID CIG. This parameter is allocated by the master's Host and passed to the slave's Host through the Link Layers during the process of establishing a AIR HID CIS. */
    uint8_t  scnario;                                     /**< Current scnario.*/
    uint16_t slave_latency;                               /**< The defualt latency of slave device.*/
    uint8_t  d4_sub_event;                                /**< The defualt sub event.*/
    uint16_t iso_interval;                                /**< The time between two consecutive anchor points. (Time = N * us.)*/
    uint8_t  cis_count;                                   /**< Total number of CISes in the AIR HID CIG being added or modified. The maximum value is limited to 16. */
    bt_ull_le_air_hid_cis_params_t *cis_list;             /**< The parameters of each AIR HID CIS list in the AIR HID CIG. */
}) bt_ull_le_set_air_hid_cig_t;

/**
 * @brief       The structure definition parameters for AIR HID CISes of #bt_ull_le_set_air_hid_cig_cnf_t.
 */
BT_PACKED(
typedef struct {
    bt_handle_t cis_connection_handle;                    /**< Connection handles of the CISes. */
    uint8_t     cis_id;                                   /**< identifies an AIR HID CIS*/
}) bt_ull_le_set_air_hid_cis_info_t;

/**
 * @brief       LE set AIR HID CIG parameters complete event.
 */
BT_PACKED(
typedef struct {
    bt_hci_status_t status;                               /**< Status. */
    uint8_t         cig_id;                               /**< Identifies a AIR HID CIG. This parameter is allocated by the master's Host and passed to the slave's Host through the Link Layers during the process of establishing a AIR HID CIS. */
    uint8_t         cis_count;                            /**< Total count of the AIR HID CISes.*/
    bt_ull_le_set_air_hid_cis_info_t    cis_info[1];      /**< Connection info of the AIR HID CISes. */
}) bt_ull_le_set_air_hid_cig_cnf_t;

/**
 * @brief       The structure definition for AIR HID CIS connection parameters of #bt_ull_le_create_air_hid_cis_params_t.
 */
BT_PACKED(
typedef struct {
    uint8_t     uni_aa[4];                                /**< Universal vendor access address. */
    bt_addr_t   peer_addr;                                /**< Address information of the remote device. */
}) bt_ull_le_air_hid_cis_dev_info_t;

/**
 * @brief       The structure definition for sub parameters of #bt_ull_le_create_air_hid_cis_t.
 */
BT_PACKED(
typedef struct {
    bt_handle_t cis_connection_handle;                    /**< Connection handles of the AIR HID CIS.*/
    uint16_t    connection_timeout;                       /**< Supervision timeout of AIR HID CIS.  unit: N * 10ms.*/
    uint8_t     dev_count;                                /**< Total number of devices to connect. The maximum value is limited to 3. */
    uint8_t     dev_list[1];                              /**< The dynamic buffer of #bt_ull_le_air_hid_cis_dev_info_t*/
}) bt_ull_le_create_air_hid_cis_params_t;

/**
 * @brief       The structure definition for the command to create AIR HID CIS parameters.
 */
BT_PACKED(
typedef struct {
    uint16_t    create_timeout;                           /**< The create AIR HID CIS tiemout. unit: N * 10ms. */
    uint8_t     ltk[16];                                  /**< The long term key of AIR HID CISes. */
    uint8_t     skd[16];                                  /**< The session key diversifier of AIR HID CISes. */
    uint8_t     iv[8];                                    /**< The initialization vector of AIR HID CISes. */
    uint8_t     cis_count;                                /**< The number of AIR HID CISes*/
    uint8_t     cis_list[1];                              /**< The dynamic buffer of #bt_ull_le_create_air_hid_cis_params_t. */
}) bt_ull_le_create_air_hid_cis_t;

/**
 * @brief       The structure definition for the command to sync AIR HID CIS parameters.
 */
BT_PACKED(
typedef struct {
    uint16_t    sync_timeout;                             /**< The sync AIR HID CIS tiemout. unit: N * 10ms. */
    uint16_t    connection_timeout;                       /**< Supervision timeout of AIR HID CIS.  unit: N * 10ms.*/
    uint8_t     ltk[16];                                  /**< The long term key of AIR HID CISes. */
    uint8_t     skd[16];                                  /**< The session key diversifier of AIR HID CISes. */
    uint8_t     iv[8];                                    /**< The initialization vector of AIR HID CISes. */
    uint8_t     slave_type;                               /**< The type of slave device . */
    uint8_t     uni_aa[4];                                /**< Universal vendor access address. */
    bt_addr_t   peer_addr;                                /**< Address information of the remote device. */
}) bt_ull_le_sync_air_hid_cis_t;

/**
 * @brief       The structure definition for the event of AIR HID CIS established.
 */
BT_PACKED (
typedef struct {
    uint8_t           status;                             /**< The status of the AIR CIS. */
    bt_handle_t       acl_connection_handle;              /**< Pseudo connection handle of the LE ACL. */
    bt_handle_t       cis_connection_handle;              /**< Connection handle of the AIR CIS. */
    uint8_t           cis_id;                             /**< Identifies an AIR CIS and is set by the master's Host and passed to the slave's Host through the Link Layers during the process of establishing an AIR CIS. */
    uint8_t           slave_type;                         /**< The type of slave device . */
    uint8_t           uni_aa[4];                          /**< Universal vendor access address. */
    bt_addr_t         peer_aadr;                          /**< Address information of the remote device. */
    uint32_t          cig_sync_delay : 24;                /**< The maximum time, in microseconds, for transmission of PDUs of all AIR CISes in an AIR CIG in an isochronous interval. */
    uint32_t          cis_sync_delay : 24;                /**< The maximum time, in microseconds, for transmission of PDUs of an AIR CIS in the isochronous interval. */
    uint32_t          transport_latency_m_to_s : 24;      /**< The maximum time, in microseconds, for transmission of SDUs of all AIR CISes in an AIR CIG from slave to master. */
    uint32_t          transport_latency_s_to_m : 24;      /**< The maximum time, in microseconds, for transmission of SDUs of all AIR CISes in an AIR CIG from master to slave. */
    uint8_t           phy_m_to_s;                         /**< The transmitter PHY of packets used from the master to slave. */
    uint8_t           phy_s_to_m;                         /**< The transmitter PHY of packets used from the slave to master. */
    uint8_t           nse;                                /**< Maximum number of subevents in each isochronous event. */
    uint8_t           share_count;                        /**< Maximum number of subevents in each isochronous event. */
    uint8_t           bn_m_to_s;                          /**< The burst number for master to slave transmission. */
    uint8_t           bn_s_to_m;                          /**< The burst number for slave to master transmission. */
    uint8_t           ft_m_to_s;                          /**< The flush timeout, in multiples of the ISO_Interval for the AIR CIS, for each payload sent from the master to the slave. */
    uint8_t           ft_s_to_m;                          /**< The flush timeout, in multiples of the ISO_Interval for the AIR CIS, for each payload sent from the slave to the master.*/
    uint16_t          max_pdu_m_to_s;                     /**< Maximum size, in octets, of the payload from master to slave. */
    uint16_t          max_pdu_s_to_m;                     /**< Maximum size, in octets, of the payload from slave to master.*/
    uint16_t          iso_interval;                       /**< The time between two consecutive AIR CIS anchor points.*/
}) bt_ull_le_air_hid_cis_established_ind_t;

/**
 * @brief       The structure definition for the command to cancel create AIR HID CIS parameters.
 */
BT_PACKED(
typedef struct {
    bt_handle_t cis_handle;                               /**< Connection handles of the CISes. */
    uint8_t     reason;                                   /**< The reason to cancel. */
}) bt_ull_le_cancel_create_air_hid_cis_params_t;

/**
 * @brief       The structure definition for the command to set sub event parameters.
 */
BT_PACKED(
typedef struct {
    uint8_t     cig_id;                                   /**< Identifies a AIR HID CIG. This parameter is allocated by the master's Host and passed to the slave's Host through the Link Layers during the process of establishing a AIR HID CIS. */
    uint8_t     sub_event;                                /**< The number of sub event.*/
}) bt_ull_le_set_air_hid_cis_sub_event_t;

/**
 * @brief       The structure definition for the command to disconnect AIR HID CIS parameters.
 */
typedef bt_hci_cmd_disconnect_t bt_ull_le_air_hid_cis_disconnect_t;

/**
 *  @brief      The structure definition for the command to remove the AIR HID ISO data path parameters.
 */
typedef bt_hci_le_remove_iso_data_path_params_t bt_ull_le_remove_air_hid_iso_data_path_t;

/**
 *  @brief      The structure definition for the command to setup the AIR HID ISO data path parameters.
 */
typedef bt_ull_le_setup_air_iso_data_path_t bt_ull_le_setup_air_hid_iso_data_path_t;

/**
 *  @brief      The structure definition for the command to remove the AIR HID CIG parameters.
 */
typedef bt_ull_le_remove_air_cig_t bt_ull_le_remove_air_hid_cig_t;

/************************************************************event ind**************************************************************************************/
/**
 *  @brief      The structure definition for the event of AIR HID CIS disconnect complete parameters.
 */
typedef bt_ull_le_air_cis_disconnect_complete_ind_t bt_ull_le_air_hid_cis_disconnect_complete_ind_t;
/***********************************************************************************************************************************************************/

/******************************************************cmd compelet or command status cnf**********************************************************************/

/**
 * @brief This structure defines the associated parameter type in the callback for #BT_ULL_LE_REMOVE_AIR_HID_ISO_DATA_PATH_CNF event.
 */
typedef bt_hci_le_remove_iso_data_path_t bt_ull_le_remove_air_hid_iso_data_path_cnf_t;

/**
 * @brief This structure defines the associated parameter type in the callback for #BT_ULL_LE_SETUP_AIR_HID_ISO_DATA_PATH_CNF event.
 */
typedef bt_hci_le_setup_iso_data_path_t bt_ull_le_setup_air_hid_iso_data_path_cnf_t;

/**
 * @brief This structure defines the associated parameter type in the callback for #BT_ULL_LE_REMOVE_AIR_HID_CIG_CNF event.
 */
typedef bt_ull_le_remove_air_cig_cnf_t bt_ull_le_remove_air_hid_cig_cnf_t;

/***********************************************************************************************************************************************************************************/

/**
 * @brief     This function disconnects the air cis.
 * @param[in] disconnect         is the disconnect air cis parameters #bt_ull_le_air_cis_disconnect_t.
 * @return                       #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_disconnect_air_cis(bt_ull_le_air_cis_disconnect_t *disconnect);

/**
 * @brief     This function removes the air iso data path.
 * @param[in] remove_iso         is the remove air iso data path parameters #bt_ull_le_remove_air_iso_data_path_t.
 * @return                       #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_remove_air_iso_data_path(bt_ull_le_remove_air_iso_data_path_t *remove_iso);

/**
 * @brief     This function set up the air iso data path.
 * @param[in] setup_iso          is the setup for the air iso data path parameters #bt_ull_le_setup_air_iso_data_path_t.
 * @return                       #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_setup_air_iso_data_path(bt_ull_le_setup_air_iso_data_path_t *setup_iso);

/**
 * @brief     This function replies to the air cis connection request.
 * @param[in] reply              is the parameters for the air cis connection request reply #bt_ull_le_reply_air_cis_request_t.
 * @return                       #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_reply_air_cis_connect_request(bt_ull_le_reply_air_cis_request_t *reply);

/**
 * @brief     This function sets the air cig parameters.
 * @param[in] cig_params     is the air cig parameters #bt_ull_le_air_cig_params_t.
 * @return                            #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_set_air_cig_parameters(bt_ull_le_air_cig_params_t *cig_params);

/**
 * @brief     This function removes the air cig.
 * @param[in] remove_cig         is the remove air cig parameters #bt_ull_le_remove_air_cig_t.
 * @return                       #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_remove_air_cig(bt_ull_le_remove_air_cig_t *remove_cig);

/**
 * @brief     This function creates the air cis.
 * @param[in] create_cis         is the parameters to create air cis #bt_ull_le_create_air_cis_t.
 * @return                       #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_create_air_cis(bt_ull_le_create_air_cis_t *create_cis);

/**
 * @brief     This function sets the air params table to controller.
 * @param[in] table              is the parameters to set air params table #bt_ull_le_set_air_params_table_t.
 * @return                       #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_set_air_cig_params_table(bt_ull_le_set_air_label_params_table_t *table);

/**
 * @brief     This function changes the air cig parameters.
 * @param[in] cig_param          is the air cig parameters table #bt_ull_le_change_air_cig_params_t.
 * @return                       #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_change_air_params(bt_ull_le_change_air_cig_params_t *cig_param);

/**
 * @brief     This function unmutes the air cis, the controller calculates play information to dsp when receiving this command.
 * @param[in] unmute             is the parameters to unmute air cis #bt_ull_le_unmute_air_cis_t.
 * @return                       #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_unmute_air_cis(bt_ull_le_unmute_air_cis_t *unmute);

/**
 * @brief     This function activates the uplink.
 * @param[in] params             is the parameters to activate uplink #bt_ull_le_activate_uplink_t.
 * @return                       #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_activate_uplink(bt_ull_le_activate_uplink_t *params);

/**
 * @brief     This function set advertising/scan access address.
 * @param[in] params             is the access address #bt_ull_le_set_adv_scan_access_addr_t.
 * @return                       #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_set_access_address(bt_ull_le_set_adv_scan_access_addr_t *addr);

/**
 * @brief     This function change encode time.
 * @param[in] params             is the parameters to activate uplink #bt_ull_le_change_encode_time_t.
 * @return                       #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_change_encode_time(bt_ull_le_change_encode_time_t *encode_time);

/**
 * @brief     This function enable/disable the QoS report.
 * @param[in] params             is the parameters to enable or disable qos report #bt_ull_le_enable_qos_report_t.
 * @return                       #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_enable_qos_report(bt_ull_le_enable_qos_report_t *enable_qos);

/**
 * @brief    This function init bt ull profile.
 */
bt_status_t bt_ull_le_init(void);

/**
 * @brief    This function deinit bt ull profile.
 */
bt_status_t bt_ull_le_deinit(void);

/*******************************************************************************************************************************************************************************
* 
* Define AIR HID CIS APIS
*
*******************************************************************************************************************************************************************************/

/**
 * @brief     This function set AIR HID CIG.
 * @param[in] params             is the parameters to set air hid cig #bt_ull_le_set_air_hid_cig_t.
 * @return                       #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_set_air_hid_cig_parameters(bt_ull_le_set_air_hid_cig_t *params);

/**
 * @brief     This function removes AIR HID CIG.
 * @param[in] params             is the parameters to set air hid cig #bt_ull_le_remove_air_hid_cig_t.
 * @return                       #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_remove_air_hid_cig_parameters(bt_ull_le_remove_air_hid_cig_t *params);

/**
 * @brief     This function creates AIR HID CIS.
 * @param[in] params             is the parameters to set air hid cig #bt_ull_le_create_air_hid_cis_t.
 * @return                       #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_create_air_hid_cis(bt_ull_le_create_air_hid_cis_t *params);

/**
 * @brief     This function sync AIR HID CIS.
 * @param[in] params             is the parameters to set air hid cig #bt_ull_le_sync_air_hid_cis_t.
 * @return                       #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_sync_air_hid_cis(bt_ull_le_sync_air_hid_cis_t *params);

/**
 * @brief     This function disconnects the AIR HID CIS.
 * @param[in] disconnect         is the disconnect air cis parameters #bt_ull_le_air_hid_cis_disconnect_t.
 * @return                       #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_disconnect_air_hid_cis(bt_ull_le_air_hid_cis_disconnect_t *disconnect);

/**
 * @brief     This function set the connection sub event counts about AIR HID CIS.
 * @param[in] params             is the parameters to set air hid cig #bt_ull_le_set_air_hid_cis_sub_event_t.
 * @return                       #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_set_air_hid_cis_sub_event(bt_ull_le_set_air_hid_cis_sub_event_t *params);

/**
 * @brief     This function cancel creating AIR HID CIS.
 * @param[in] params             is the parameters to set air hid cig #bt_ull_le_cancel_create_air_hid_cis_params_t.
 * @return                       #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_cancel_creating_air_hid_cis(bt_ull_le_cancel_create_air_hid_cis_params_t *params);

/**
 * @brief     This function cancel sync AIR HID CIS.
 * @param[in] params            void.
 * @return                       #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_cancel_sync_air_hid_cis(void);

/**
 * @brief     This function removes the AIR HID ISO data path.
 * @param[in] remove_iso         is the remove air iso data path parameters #bt_ull_le_remove_air_hid_iso_data_path_t.
 * @return                       #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_remove_air_hid_iso_data_path(bt_ull_le_remove_air_hid_iso_data_path_t *remove_iso);

/**
 * @brief     This function set up the AIR HID ISO data path.
 * @param[in] setup_iso          is the setup for the air iso data path parameters #bt_ull_le_setup_air_hid_iso_data_path_t.
 * @return                       #BT_STATUS_SUCCESS, the operation completed successfully.
 *                               #BT_STATUS_FAIL, the operation has failed.
 */
bt_status_t bt_ull_le_setup_air_hid_iso_data_path(bt_ull_le_setup_air_hid_iso_data_path_t *setup_iso);

/***********************************************************************************************************************************************************************************/
/**
 * @}
 * @}
 */

#endif


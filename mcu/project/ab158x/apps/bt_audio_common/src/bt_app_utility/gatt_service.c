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

#include <string.h>
#include "bt_gattc.h"
#include "bt_gatt.h"
#include "bt_gatts.h"
#ifdef __BLE_FMP__
#include "ble_ias.h"
#endif
#ifdef MTK_BLE_BAS
#include "ble_bas.h"
#endif
#ifdef AIR_BT_FAST_PAIR_ENABLE
#include "bt_fast_pair.h"
#endif
#ifdef AIR_CUST_PAIR_ENABLE
#include "cust_pair.h"
#endif
#ifdef AIR_LE_AUDIO_ENABLE
#include "app_lea_service_conn_mgr.h"
#include "app_lea_service.h"
#endif
#ifdef AIR_SPEAKER_ENABLE
#include "bt_aws_mce_le_association.h"
#endif
#include "bt_gatts_service.h"

#ifndef MTK_AWS_MCE_ENABLE
extern const bt_gatts_service_t **bt_get_gatt_server(void);
#if _MSC_VER >= 1500
#pragma comment(linker,"/alternatename:_bt_get_gatt_server=_bt_gatt_service_get_table")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__ARMCC_VERSION)
#pragma weak bt_get_gatt_server = bt_gatt_service_get_table
#else
#error "Unsupported Platform"
#endif
#endif

//Declare every record here
//service collects all bt_gatts_service_rec_t
//IMPORTAMT: handle:0x0000 is reserved, please start your handle from 0x0001
//GAP 0x0001

#ifdef MTK_BLE_SMTCN_ENABLE
extern const bt_gatts_service_t bt_if_dtp_service;
#endif
#ifdef MTK_BLE_BAS
extern const bt_gatts_service_t ble_bas_service;
#endif

#ifdef AIR_CUST_PAIR_ENABLE
extern const bt_gatts_service_t bt_if_gatt_service_without_data_hash;
#endif
//extern const bt_gatts_service_t bt_if_gatt_service;

//extern const bt_gatts_service_t bt_if_gap_service;
//extern const bt_gatts_service_t bt_if_dogp_service;

//extern const bt_gatts_service_t ble_ias_service;

#ifdef MTK_PORT_SERVICE_BT_ENABLE
extern const bt_gatts_service_t ble_air_service;
#endif

extern const bt_gatts_service_t ble_dis_service;

#ifdef AIR_CUST_PAIR_ENABLE
extern const bt_gatts_service_t cust_pair_srv_service;
#endif

#ifdef AIR_AMA_ENABLE
extern const bt_gatts_service_t ble_ama_service;
#endif

#ifdef AIR_XIAOWEI_ENABLE
extern const bt_gatts_service_t xiaowei_ble_service;
#endif

#ifdef AIR_XIAOAI_ENABLE
extern const bt_gatts_service_t xiaoai_ble_service;
#ifdef AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE
extern const bt_gatts_service_t miui_fast_connect_service;
extern uint16_t miui_fc_get_conn_handle(void);
#endif
#endif

#ifdef AIR_LE_AUDIO_ENABLE
extern const bt_gatts_service_t ble_vcs_service;
extern const bt_gatts_service_t ble_vocs_service_channel_1;
#ifdef AIR_LE_AUDIO_HEADSET_ENABLE
extern const bt_gatts_service_t ble_vocs_service_channel_2;
#endif
extern const bt_gatts_service_t ble_aics_service;
extern const bt_gatts_service_t ble_mics_service;
extern const bt_gatts_service_t ble_csis_service;
extern const bt_gatts_service_t ble_cas_service;
extern const bt_gatts_service_t ble_ascs_service;
extern const bt_gatts_service_t ble_pacs_service;
extern const bt_gatts_service_t ble_bass_service;
extern const bt_gatts_service_t ble_tmas_service;
#ifdef AIR_TWS_ENABLE
extern const bt_gatts_service_t ble_dhss_service;
extern const bt_gatts_service_t ble_mps_service;
#endif

#ifdef AIR_LE_AUDIO_HAPS_ENABLE
extern const bt_gatts_service_t ble_has_service;
#endif
#ifdef AIR_LE_AUDIO_GMAP_ENABLE
extern const bt_gatts_service_t ble_gmas_service;
#endif

extern bool bt_le_audio_sink_is_link_valid(bt_handle_t handle);
#endif

#ifdef AIR_HOGP_ENABLE
extern const bt_gatts_service_t bt_hid_service;
#endif

#ifdef AIR_BLE_HRS_ENABLE
extern const bt_gatts_service_t ble_hrs_service;
#endif
#ifdef AIR_MS_TEAMS_ENABLE
extern const bt_gatts_service_t ms_teams_service;
#endif

/**< gatt server collects all service. */
const bt_gatts_service_t *bt_if_clm_gatt_server[] = {
    &bt_if_gap_service,                   /**< handle range: 0x0001 to 0x0009. */
    &bt_if_gatt_service,                  /**< handle range: 0x0011 to 0x0015. */
#ifdef MTK_BLE_SMTCN_ENABLE
    &bt_if_dtp_service,                   /**< handle range: 0x0014 to 0x0017. */

#endif
    //&bt_if_dogp_service,                /**< handle range: 0x0020 to 0x0025. */
#ifdef MTK_BLE_BAS
    &ble_bas_service,                     /**< handle range: 0x0031 to 0x0034. */
#endif
#ifdef __BLE_FMP__
    &ble_ias_service,                     /**< handle range: 0x0040 to 0x0042. */
#endif
#ifdef MTK_PORT_SERVICE_BT_ENABLE
    &ble_air_service,                     /**< handle range: 0x0051 to 0x0056. */
#endif
    &ble_dis_service,                     /**< handle range: 0x0060 to 0x0072. */
#ifdef AIR_CUST_PAIR_ENABLE
    &cust_pair_srv_service,               /**< handle range: 0x0080 to 0x0083. */
#endif
#ifdef AIR_AMA_ENABLE
    &ble_ama_service,                     /**< handle range: 0x00D0 to 0x00D5. */
#endif
#ifdef AIR_XIAOWEI_ENABLE
    &xiaowei_ble_service,                 /**< handle range: 0x00E0 to 0x00E5. */
#endif
#ifdef AIR_XIAOAI_ENABLE
#ifdef AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE
    &miui_fast_connect_service,           /**< handle range: 0x00E0 to 0x00EB. */
#endif
    &xiaoai_ble_service,                  /**< handle range: 0x00F0 to 0x00F5. */
#endif
#ifdef AIR_BT_FAST_PAIR_ENABLE
    &bt_fast_pair_service,                /**< handle range: 0x0100 to 0x0110. */
#endif
#ifdef AIR_SPEAKER_ENABLE
    &bt_aws_mce_le_association_service,   /**< handle range: 0x0120 to 0x 0130. */
#endif
#ifdef AIR_HOGP_ENABLE
    &bt_hid_service,                      /**< handle range: 0x0200 to 0x0217. */
#endif
#ifdef AIR_MS_TEAMS_ENABLE
    &ms_teams_service,                    /**< handle range: 0x0220 to 0x0226. */
#endif
    NULL
};

#ifdef AIR_LE_AUDIO_ENABLE
const bt_gatts_service_t *bt_if_lea_gatt_server[] = {
    &bt_if_gap_service,                   /**< handle range: 0x0001 to 0x0009. */
    &bt_if_gatt_service,                  /**< handle range: 0x0011 to 0x0015. */
#ifdef MTK_BLE_BAS
    &ble_bas_service,                     /**< handle range: 0x0031 to 0x0034. */
#endif
#ifdef __BLE_FMP__
    &ble_ias_service,                     /**< handle range: 0x0040 to 0x0042. */
#endif
#ifdef MTK_PORT_SERVICE_BT_ENABLE
    &ble_air_service,                     /**< handle range: 0x0051 to 0x0056. */
#endif
    &ble_dis_service,                     /**< handle range: 0x0060 to 0x0072. */
#if defined(AIR_CUST_PAIR_ENABLE) && defined(APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE)
    &cust_pair_srv_service,               /**< handle range: 0x0080 to 0x0083. */
#endif
#ifdef AIR_XIAOAI_ENABLE
#ifdef AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE
    &miui_fast_connect_service,           /**< handle range: 0x00E0 to 0x00EB. */
#endif
    &xiaoai_ble_service,                  /**< handle range: 0x00F0 to 0x00F5. */
#endif
#ifdef AIR_BT_FAST_PAIR_ENABLE
    &bt_fast_pair_service,                /**< handle range: 0x0100 to 0x0110. */
#endif
#ifdef AIR_BLE_HRS_ENABLE
    &ble_hrs_service,                     /**< handle range: 0x0541 to 0x0548. */
#endif
#ifdef AIR_HOGP_ENABLE
    &bt_hid_service,                      /**< handle range: 0x0200 to 0x0217. */
#endif
#if 1
#ifdef AIR_MS_TEAMS_ENABLE
    &ms_teams_service,                    /**< handle range: 0x0220 to 0x0226. */
#endif
#endif
#ifdef AIR_LE_AUDIO_ENABLE
    &ble_ascs_service,                    /**< handle range: 0x1103 to 0x110F. */
    &ble_pacs_service,                    /**< handle range: 0x1200 to 0x1212. */
    &ble_vcs_service,                     /**< handle range: 0x1301 to 0x1309. */
    &ble_vocs_service_channel_1,          /**< handle range: 0x2001 to 0x200C. */
#ifdef AIR_LE_AUDIO_HEADSET_ENABLE
    &ble_vocs_service_channel_2,          /**< handle range: 0x3001 to 0x300C. */
#endif
    &ble_aics_service,                    /**< handle range: 0x4001 to 0x4010. */
    &ble_mics_service,                    /**< handle range: 0x5001 to 0x5004. */
    &ble_csis_service,                    /**< handle range: 0x6001 to 0x600A. */
    &ble_cas_service,                     /**< handle range: 0x7001 to 0x7002. */
    &ble_bass_service,                    /**< handle range: 0xA201 to 0xA209. */
    &ble_tmas_service,                    /**< handle range: 0xA301 to 0xA303. */
#ifdef AIR_TWS_ENABLE
    &ble_dhss_service,                    /**< handle range: 0xA401 to 0xA403. */
    &ble_mps_service,                     /**< handle range: 0xA501 to 0xA507. */
#endif
#ifdef AIR_LE_AUDIO_HAPS_ENABLE
    &ble_has_service,                     /**< handle range: 0xA600 to 0xA608. */
#endif
#ifdef AIR_LE_AUDIO_GMAP_ENABLE
    &ble_gmas_service,                    /**< handle range: 0xA701 to 0xA703. */
#endif
#endif
    NULL
};
#endif

#ifdef AIR_XIAOAI_ENABLE
const bt_gatts_service_t *bt_if_xiaoai_gatt_server[] = {
    &bt_if_gap_service,                   /**< handle range: 0x0001 to 0x0009. */
    &bt_if_gatt_service,                  /**< handle range: 0x0011 to 0x0015. */
#ifdef MTK_BLE_BAS
    &ble_bas_service,                     /**< handle range: 0x0031 to 0x0034. */
#endif
    &ble_dis_service,                     /**< handle range: 0x0060 to 0x0072. */
#ifdef AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE
    &miui_fast_connect_service,           /**< handle range: 0x00E0 to 0x00EB. */
#endif
    &xiaoai_ble_service,                  /**< handle range: 0x00F0 to 0x00F5. */
    NULL
};
#endif

#ifdef AIR_CUST_PAIR_ENABLE
const bt_gatts_service_t *bt_cust_pair_gatt_server[] = {
    &bt_if_gap_service,                    /**< handle range: 0x0001 to 0x0009. */
    &bt_if_gatt_service_without_data_hash, /**< handle range: 0x0011 to 0x0015. */
#ifdef MTK_BLE_BAS
    &ble_bas_service,                      /**< handle range: 0x0031 to 0x0034. */
#endif
#ifdef MTK_PORT_SERVICE_BT_ENABLE
    &ble_air_service,                      /**< handle range: 0x0051 to 0x0056. */
#endif
    &ble_dis_service,                      /**< handle range: 0x0060 to 0x0072. */
    &cust_pair_srv_service,                /**< handle range: 0x0080 to 0x0083. */
    NULL
};
#endif

/**< When GATTS get req from remote client, GATTS will call bt_get_gatt_server() to get application's gatt service DB. */
/**< You have to return the DB(bt_gatts_service_t pointer) to gatts stack. */
const bt_gatts_service_t **bt_gatts_get_server_by_handle(bt_handle_t connection_handle)
{
#if defined(AIR_CUST_PAIR_ENABLE) && !defined(APP_BT_SWIFT_PAIR_LE_AUDIO_ENABLE)
    cust_pair_conn_type conn_type = cust_pair_get_conn_type(connection_handle);
    if (conn_type == CUST_PAIR_CONN_TYPE_CUST || conn_type == CUST_PAIR_CONN_TYPE_STD) {
        return bt_cust_pair_gatt_server;
    }
#endif
#ifdef AIR_XIAOAI_MIUI_FAST_CONNECT_ENABLE
    if (connection_handle != BT_HANDLE_INVALID && connection_handle == miui_fc_get_conn_handle()) {
        return bt_if_xiaoai_gatt_server;
    }
#endif
#ifdef AIR_LE_AUDIO_ENABLE
    if (app_lea_service_is_enabled_lea()) {
        uint8_t conn_type = app_lea_conn_mgr_get_conn_type(connection_handle);
        bool is_link_valid = bt_le_audio_sink_is_link_valid(connection_handle);
        //LOG_MSGID_I(BT_APP, "[LEA][CONN] handle=0x%08X conn_type=%d is_link_valid=%d",
        //            3, connection_handle, conn_type, is_link_valid);
        if (conn_type == APP_LEA_CONN_TYPE_LE_AUDIO || is_link_valid) {
            return bt_if_lea_gatt_server;
        }
    }
#endif
#ifndef MTK_AWS_MCE_ENABLE
    return bt_get_gatt_server();
#else
    return bt_if_clm_gatt_server;
#endif
}
#ifndef MTK_AWS_MCE_ENABLE

const bt_gatts_service_t **bt_gatt_service_get_table(void)
{
    return bt_if_clm_gatt_server;
}
#endif


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
/**
 * File: gatt_service.c
 *
 * Description: The file is GATT service definition.
 *
 */
#include <string.h>
#include "bt_gattc.h"
#include "bt_gatt.h"
#include "bt_gatts.h"
#ifdef __BLE_FMP__
#include "ble_ias.h"
#endif
#ifdef __BLE_BAS__
#include "ble_bas.h"
#endif
#ifdef __BT_FAST_PAIR_ENABLE__
#include "bt_fast_pair.h"
#endif
#include "bt_gatts_service.h"

extern const bt_gatts_service_t **bt_get_gatt_server(void);
#if _MSC_VER >= 1500
#pragma comment(linker,"/alternatename:_bt_get_gatt_server=_bt_gatt_service_get_table")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__ARMCC_VERSION)
#pragma weak bt_get_gatt_server = bt_gatt_service_get_table
#else
#error "Unsupported Platform"
#endif
/*
Declare every record here
service collects all bt_gatts_service_rec_t
IMPORTAMT: handle:0x0000 is reserved, please start your handle from 0x0001
GAP 0x0001
*/

#ifdef MTK_BLE_SMTCN_ENABLE
extern const bt_gatts_service_t bt_if_dtp_service;
#endif
/*
extern const bt_gatts_service_t bt_if_gatt_service;
extern const bt_gatts_service_t bt_if_gap_service;
extern const bt_gatts_service_t bt_if_dogp_service;
extern const bt_gatts_service_t ble_bas_service;
extern const bt_gatts_service_t ble_ias_service;
*/
#ifdef MTK_PORT_SERVICE_BT_ENABLE
extern const bt_gatts_service_t ble_air_service;
#endif

extern const bt_gatts_service_t ble_dis_service;

#ifdef GSOUND_LIBRARY_ENABLE
extern const bt_gatts_service_t gsound_bisto_comm;
extern const bt_gatts_service_t ancs_proxy_service;
extern const bt_gatts_service_t ams_proxy_service;
extern const bt_gatts_service_t bt_bisto_media_service;
#endif

#ifdef MTK_AMA_ENABLE
extern const bt_gatts_service_t ble_ama_service;
#endif

#ifdef MTK_VA_XIAOWEI_ENABLE
extern const bt_gatts_service_t xiaowei_ble_service;
#endif

#ifdef MTK_VA_XIAOAI_ENABLE
extern const bt_gatts_service_t xiaoai_ble_service;
#endif

#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_LE_AUDIO_UNICAST_ENABLE)
extern const bt_gatts_service_t ble_gmcs_service;
extern const bt_gatts_service_t ble_mcs_service_1;
extern const bt_gatts_service_t ble_mcs_service_2;
extern const bt_gatts_service_t ble_gtbs_service;
extern const bt_gatts_service_t ble_tbs_service;
extern const bt_gatts_service_t ble_tmas_service;
extern const bt_gatts_service_t ble_aird_service;
#ifdef AIR_LE_AUDIO_GMAP_ENABLE
extern const bt_gatts_service_t ble_gmas_service;
#endif
#endif

/**< gatt server collects all service. */
const bt_gatts_service_t *bt_if_clm_gatt_server[] = {
    &bt_if_gap_service,                    /**< handle range: 0x0001 to 0x0009. */
    &bt_if_gatt_service,                   /**< handle range: 0x0011 to 0x0015. */
#ifdef MTK_BLE_SMTCN_ENABLE
    &bt_if_dtp_service,                    /**< handle range: 0x0014 to 0x0017. */
#endif
    /*&bt_if_dogp_service,*/               /**< handle range: 0x0020 to 0x0025. */
#ifdef __BLE_BAS__
    &ble_bas_service,                      /**< handle range: 0x0031 to 0x0034. */
#endif
#ifdef __BLE_FMP__
    &ble_ias_service,                      /**< handle range: 0x0040 to 0x0042. */
#endif
#ifdef MTK_PORT_SERVICE_BT_ENABLE
    &ble_air_service,                      /**< handle range: 0x0051 to 0x0056. */
#endif
    &ble_dis_service,                      /**< handle range: 0x0060 to 0x0072. */
#ifdef GSOUND_LIBRARY_ENABLE
    &gsound_bisto_comm,                    /**< handle range: 0x0090 to 0x009C. */
    &ancs_proxy_service,                   /**< handle range: 0x00A0 to 0x00AB. */
    &ams_proxy_service,                    /**< handle range: 0x00B0 to 0x00BB. */
    &bt_bisto_media_service,               /**< handle range: 0x00C0 to 0x00CC. */
#endif
#ifdef MTK_AMA_ENABLE
    &ble_ama_service,                      /**< handle range: 0x00D0 to 0x00D5. */
#endif
#ifdef MTK_VA_XIAOWEI_ENABLE
    &xiaowei_ble_service,                  /**< handle range: 0x00E0 to 0x00E5. */
#endif
#ifdef MTK_VA_XIAOAI_ENABLE
    &xiaoai_ble_service,                   /**< handle range: 0x00F0 to 0x00F5. */
#endif
#ifdef __BT_FAST_PAIR_ENABLE__
    &bt_fast_pair_service,                 /**< handle range: 0x0100 to 0x0110. */
#endif
#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_LE_AUDIO_UNICAST_ENABLE)
    &ble_gmcs_service,                     /**< handle range: 0x6001 to 0x600C. */
    //&ble_mcs_service_1,                  /**< handle range: 0x7001 to 0x700C. */
    //&ble_mcs_service_2,                  /**< handle range: 0x8001 to 0x800C. */
    &ble_gtbs_service,                     /**< handle range: 0x9001 to 0x901C. */
    //&ble_tbs_service,                    /**< handle range: 0xA001 to 0xA019. */
    &ble_tmas_service,                     /**< handle range: 0xA301 to 0xA303. */
    &ble_aird_service,                     /**< handle range: 0xA401 to 0xA406. */
#ifdef AIR_LE_AUDIO_GMAP_ENABLE
    &ble_gmas_service,                     /**< handle range: 0xA701 to 0xA703. */
#endif
#endif

    NULL
};


/**< When GATTS get req from remote client, GATTS will call bt_get_gatt_server() to get application's gatt service DB. */
/**< You have to return the DB(bt_gatts_service_t pointer) to gatts stack. */
const bt_gatts_service_t **bt_gatts_get_server_by_handle(bt_handle_t connection_handle)
{
    return bt_get_gatt_server();
}

const bt_gatts_service_t **bt_gatt_service_get_table(void)
{
    return bt_if_clm_gatt_server;
}

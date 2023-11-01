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

#include "race_usb_relay.h"
#include "race_usb_relay_dongle.h"
#include "race_cmd_informational.h"

#if defined(MTK_USB_DEMO_ENABLED) && defined(MTK_USB_AUDIO_HID_ENABLE)
#include "usbaudio_drv.h"
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
#include "bt_ull_le_service.h"
#endif

/**************************************************************************************************
* Define
**************************************************************************************************/

#define RACE_ID_NO_REMOTE_RANGE_BEGIN   0x2E00
#define RACE_ID_NO_REMOTE_RANGE_END     0x2E1F

#define RACE_DONGLE_RELAY_DYNAMIC_DEVICE_ID_START       (0x80)

#define RACE_DONGLE_RELAY_LOCAL_DEVICE_ID               (0)

#if defined(AIR_DONGLE_RELAY_RACE_PACKET_ENABLE)

/**************************************************************************************************
* Structure
**************************************************************************************************/

typedef struct {
    uint8_t usb_flag;
    race_port_t dst_port;
} race_dongle_relay_map_t;

typedef struct {
    uint8_t usb_flag;
    race_port_t dst_port;
    bt_bd_addr_t bd_addr;
} race_dongle_relay_dynamic_map_t;

/**************************************************************************************************
* Static Variable
**************************************************************************************************/
#if defined(RACE_DYNAMIC_MAPPING_DEVICE_ID)

static const race_port_t g_race_dongle_valid_relay_port[] = {MUX_BT_SPP, MUX_BT_BLE, MUX_BT_BLE_1, MUX_BT_BLE_2};

#define RACE_DONGLE_RELAY_PORT_MAX      (sizeof(g_race_dongle_valid_relay_port) / sizeof(g_race_dongle_valid_relay_port[0]))

static race_dongle_relay_dynamic_map_t g_race_dongle_dynamic_map_tbl[RACE_DONGLE_RELAY_PORT_MAX];


#else /* #if defined(RACE_DYNAMIC_MAPPING_DEVICE_ID) */

static const race_dongle_relay_map_t g_race_dongle_usb_relay_map_tbl[] = {

#if defined(MTK_USB_DEMO_ENABLED) && defined(MTK_USB_AUDIO_HID_ENABLE)

#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
    {TARGET_REMOTE_DEVICE,          MUX_BT_BLE},
    {TARGET_REMOTE_MULTIDEVICE1,    MUX_BT_BLE_1},
    {TARGET_REMOTE_MULTIDEVICE2,    MUX_BT_BLE_2},
#else
    {TARGET_REMOTE_DEVICE,          MUX_BT_SPP},
#endif

#endif
};

#endif  /* #if defined(RACE_DYNAMIC_MAPPING_DEVICE_ID) */

static race_port_t g_race_dongle_relay_usb_port;


/**************************************************************************************************
* Static Functions
**************************************************************************************************/

static bool race_dongle_relay_get_usb_port(race_port_t *port)
{
    uint16_t usb_cnt;
    uint16_t i;
    race_port_t last_port = g_race_dongle_relay_usb_port;
    mux_handle_t mux_handle;
    race_port_info_t *info[MUX_USB_END - MUX_USB_BEGIN + 1] = {NULL};

    if (NULL == port) {
        return false;
    }
    if (MUX_USB_BEGIN <= last_port && MUX_USB_END >= last_port) {
        info[0] = race_search_port(last_port);
        if (NULL != info[0]) {
            if(mux_query_user_handle(last_port, "RACE_CMD", &mux_handle) == MUX_STATUS_OK) {
                *port = last_port;
                return true;
            }
        }
    }

    info[0] = NULL;
    usb_cnt = race_search_port_type(&info[0], MUX_USB_END - MUX_USB_BEGIN + 1, RACE_PORT_TYPE_USB);
    for (i = 0; i < usb_cnt; i++) {
        if(mux_query_user_handle(info[i]->port, "RACE_CMD", &mux_handle) == MUX_STATUS_OK) {
            *port = info[i]->port;
            return true;
        }
    }
    return false;
}

static bool race_dongle_relay_check(race_pkt_relay_info_t *relay_info_out, race_port_t port, race_port_type_t port_type, RACE_COMMON_HDR_STRU *pkt_hdr)
{
#if defined(MTK_USB_DEMO_ENABLED) && defined(MTK_USB_AUDIO_HID_ENABLE)
    uint8_t usb_mux_flag = 0;
    if (NULL == relay_info_out) {
        return false;
    }
    usb_mux_flag = usb_mux_get_data_flag();
    relay_info_out->from_port = port;

    if (RACE_PORT_TYPE_USB == port_type) {  /* from usb port, relay to bt/ble port */
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE) || defined(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE)
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
        if (BT_ULL_ROLE_SERVER == bt_ull_le_srv_get_role()) { //dongle
#endif
            if (race_relay_get_dst_port_by_usb_flag(usb_mux_flag, &relay_info_out->to_port) == true) {
                relay_info_out->is_relay_pkt = true;
                g_race_dongle_relay_usb_port = port;
                return true;
            }
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
        }
#endif

#else
        if (race_relay_get_dst_port_by_usb_flag(usb_mux_flag, &relay_info_out->to_port) == true) {
            relay_info_out->is_relay_pkt = true;
            g_race_dongle_relay_usb_port = port;
            return true;
        }
#endif

        /* NOTE: if usb flag is not 0, it must be relay pkt, but the remote device may be not connected */
        if (RACE_DONGLE_RELAY_LOCAL_DEVICE_ID != usb_mux_flag) {
            relay_info_out->is_relay_pkt = true;
            relay_info_out->to_port = RACE_INVALID_PORT;
            return true;
        }
    } else if (RACE_PORT_TYPE_BT_BLE == port_type) {  /* from bt/ble port, relay to usb port */
        if (pkt_hdr->type == RACE_TYPE_RESPONSE || pkt_hdr->type == RACE_TYPE_NOTIFICATION) {
            if ((pkt_hdr->id >= RACE_ID_NO_REMOTE_RANGE_BEGIN && pkt_hdr->id <= RACE_ID_NO_REMOTE_RANGE_END)
            || (pkt_hdr->id == RACE_READ_SDK_VERSION)
            #ifdef AIR_PURE_GAMING_ENABLE
            || (pkt_hdr->id == RACE_CRYSTAL_TRIM_SET_CAP_REMOTE)
            #endif
            ) {
                return false;
            } else {
                race_port_t to_usb_port;
                if (race_dongle_relay_get_usb_port(&to_usb_port) == true) {
                    relay_info_out->is_relay_pkt = true;
                    relay_info_out->to_port = to_usb_port;
                    return true;
                }
            }
        }
    }
#endif
    return false;
}

#ifdef RACE_DYNAMIC_MAPPING_DEVICE_ID

static race_dongle_relay_dynamic_map_t *race_dongle_relay_get_info_by_port(race_port_t port)
{
    uint16_t i;
    for (i = 0; i < RACE_DONGLE_RELAY_PORT_MAX; i++) {
        if (port == g_race_dongle_dynamic_map_tbl[i].dst_port) {
            return &g_race_dongle_dynamic_map_tbl[i];
        }
    }
    return NULL;
}

static bool race_dongle_relay_is_valid_port(race_port_t port)
{
    uint16_t i;
    for (i = 0; i < RACE_DONGLE_RELAY_PORT_MAX; i++) {
        if (port == g_race_dongle_valid_relay_port[i]) {
            return true;
        }
    }
    return false;
}

static bool race_dongle_relay_dynamic_get_flag_by_port(uint8_t *usb_flag, race_port_t port)
{
    uint16_t i;
    if (NULL == usb_flag || race_dongle_relay_is_valid_port(port) == false) {
        return false;
    }
    for (i = 0; i < RACE_DONGLE_RELAY_PORT_MAX; i++) {
        if (g_race_dongle_dynamic_map_tbl[i].dst_port == port) {
            *usb_flag = g_race_dongle_dynamic_map_tbl[i].usb_flag;
            return true;
        }
    }
    return false;
}

static bool race_dongle_relay_dynamic_get_port_by_flag(race_port_t *port, uint8_t usb_flag)
{
    uint16_t i;
    if (NULL == port) {
        return false;
    }
    for (i = 0; i < RACE_DONGLE_RELAY_PORT_MAX; i++) {
        if (g_race_dongle_dynamic_map_tbl[i].usb_flag == usb_flag && RACE_INVALID_PORT != g_race_dongle_dynamic_map_tbl[i].dst_port) {
            *port = g_race_dongle_dynamic_map_tbl[i].dst_port;
            return true;
        }
    }
    return false;
}

#else

static bool race_dongle_relay_get_flag_by_port(uint8_t *usb_flag, race_port_t port)
{
    uint16_t i;
    uint16_t buf_size = sizeof(g_race_dongle_usb_relay_map_tbl) / sizeof(g_race_dongle_usb_relay_map_tbl[0]);
    if (port == RACE_INVALID_PORT || NULL == usb_flag) {
        return false;
    }
    for (i = 0; i < buf_size; i++) {
        if (port == g_race_dongle_usb_relay_map_tbl[i].dst_port) {
            *usb_flag = g_race_dongle_usb_relay_map_tbl[i].usb_flag;
            return true;
        }
    }
    return false;
}

static bool race_dongle_relay_get_port_by_flag(race_port_t *port, uint8_t usb_flag)
{
    uint16_t i;
    uint16_t buf_size = sizeof(g_race_dongle_usb_relay_map_tbl) / sizeof(g_race_dongle_usb_relay_map_tbl[0]);
    if (NULL == port) {
        return false;
    }
    for (i = 0; i < buf_size; i++) {
        if (usb_flag == g_race_dongle_usb_relay_map_tbl[i].usb_flag) {
            *port = g_race_dongle_usb_relay_map_tbl[i].dst_port;
            return true;
        }
    }
    return false;
}

#endif

/**************************************************************************************************
* Static Functions
**************************************************************************************************/

#ifdef RACE_DYNAMIC_MAPPING_DEVICE_ID

void race_dongle_relay_conn_callback(race_port_t port, bt_bd_addr_t *bd_addr)
{
    if (NULL != bd_addr) {
        race_dongle_relay_dynamic_map_t *pinfo = NULL;
        if (race_dongle_relay_is_valid_port(port) == false) {
            return;
        }
        pinfo = race_dongle_relay_get_info_by_port(port);
        if (NULL == pinfo) {
            pinfo = race_dongle_relay_get_info_by_port(RACE_INVALID_PORT);
        }
        if (NULL != pinfo) {
            pinfo->dst_port = port;
            memcpy(&pinfo->bd_addr, bd_addr, sizeof(bt_bd_addr_t));
            RACE_LOG_MSGID_I("relay port connected: port:%d, pinfo:0x%x", 2, port, (uint32_t)pinfo);
        }
    }
}

void race_dongle_relay_disconn_callback(race_port_t port)
{
    if (race_dongle_relay_is_valid_port(port)) {
        race_dongle_relay_dynamic_map_t *pinfo = NULL;
        pinfo = race_dongle_relay_get_info_by_port(port);
        if (pinfo) {
            pinfo->dst_port = RACE_INVALID_PORT;
            RACE_LOG_MSGID_I("relay port disconnected: port:%d, pinfo:0x%x", 2, port, (uint32_t)pinfo);
        }
    }
}

#endif

void race_usb_relay_dongle_init(void)
{
    bool res = race_register_relay_check_handler(race_dongle_relay_check);
#ifdef RACE_DYNAMIC_MAPPING_DEVICE_ID
    race_usb_relay_map_t map_tbl = {race_dongle_relay_dynamic_get_flag_by_port, race_dongle_relay_dynamic_get_port_by_flag};
    uint16_t i;
    for (i = 0; i < RACE_DONGLE_RELAY_PORT_MAX; i++) {
        g_race_dongle_dynamic_map_tbl[i].dst_port = RACE_INVALID_PORT;
        g_race_dongle_dynamic_map_tbl[i].usb_flag = RACE_DONGLE_RELAY_DYNAMIC_DEVICE_ID_START + i;
    }
#else
    race_usb_relay_map_t map_tbl = {race_dongle_relay_get_flag_by_port, race_dongle_relay_get_port_by_flag};
#endif

    race_usb_relay_register_map_tbl(&map_tbl);
    assert(res && "ERROR: race_usb_relay_dongle_init, register handler fail!");
}

#endif



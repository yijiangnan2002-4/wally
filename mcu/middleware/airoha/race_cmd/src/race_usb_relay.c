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

#include "race_port_usb.h"
#include "race_usb_relay.h"
#include "race_cmd_informational.h"
#include "race_cmd_relay_cmd_cosys.h"

#if defined(MTK_USB_DEMO_ENABLED) && defined(MTK_USB_AUDIO_HID_ENABLE)
#include "usbaudio_drv.h"
#endif
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_ENABLE
#include "bt_ull_le_service.h"
#endif

#if defined(AIR_DONGLE_RELAY_RACE_PACKET_ENABLE)
#include "race_usb_relay_dongle.h"
#endif

#if defined (AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined (AIR_DCHS_MODE_MASTER_ENABLE)
#include "race_usb_relay_cosys.h"
#endif



#if defined RACE_USB_RELAY_ENABLE

/**************************************************************************************************
* Define
**************************************************************************************************/

#define RACE_RELAY_USB_PKT_CACHE_MAX    (30)

/**************************************************************************************************
* Structure
**************************************************************************************************/

typedef struct {
    uint8_t *data;
    race_port_t usb_port;
    uint8_t usb_flag;
} race_relay_pkt_usb_flag_t;

typedef struct {
    race_relay_pkt_usb_flag_t cache[RACE_RELAY_USB_PKT_CACHE_MAX];
    uint16_t cnt;
} race_relay_flag_cache_t;

/**************************************************************************************************
* Static Variable
**************************************************************************************************/

static race_relay_flag_cache_t g_race_usb_relay_pkt_cache;

static race_usb_relay_map_t g_race_usb_relay_map = {NULL, NULL};

/**************************************************************************************************
* Public Functions
**************************************************************************************************/

uint8_t race_relay_get_usb_tx_flag_by_src_port(race_port_t src_port)
{
    uint8_t usb_flag = 0;
    if (g_race_usb_relay_map.port_map_to_flag) {
        if (g_race_usb_relay_map.port_map_to_flag(&usb_flag, src_port) == true) {
            return usb_flag;
        }
    }
    return TARGET_LOCAL_DEVICE;
}

bool race_relay_get_dst_port_by_usb_flag(uint8_t usb_flag, race_port_t *dst_port)
{
    if (NULL == dst_port) {
        return false;
    }
    if (g_race_usb_relay_map.flag_map_to_port) {
        return g_race_usb_relay_map.flag_map_to_port(dst_port, usb_flag);
    }
    return false;
}

void race_usb_relay_set_flag_by_src_port(race_port_t src_port, race_port_t usb_dst_port, uint8_t *data)
{
    uint16_t i;
    if (NULL == data) {
        return;
    }
    if (RACE_PORT_IS_USB(usb_dst_port) == false) {
        return;
    }
    for (i = 0; i < RACE_RELAY_USB_PKT_CACHE_MAX; i++) {
        if (NULL == g_race_usb_relay_pkt_cache.cache[i].data) {
            g_race_usb_relay_pkt_cache.cache[i].data = data;
            g_race_usb_relay_pkt_cache.cache[i].usb_port = usb_dst_port;
            g_race_usb_relay_pkt_cache.cache[i].usb_flag = race_relay_get_usb_tx_flag_by_src_port(src_port);
            g_race_usb_relay_pkt_cache.cnt++;
            if (i > (RACE_RELAY_USB_PKT_CACHE_MAX >> 1)) {
                ;// log for warning
            }
            return ;
        }
    }
    // add log or assert here
}

uint8_t race_usb_relay_get_flag(race_port_t usb_dst_port, uint8_t *data)
{
    uint16_t i;
    uint8_t res_flag = TARGET_LOCAL_DEVICE;
    if (NULL == data) {
        return res_flag;
    }
    if (RACE_PORT_IS_USB(usb_dst_port) == false || 0 == g_race_usb_relay_pkt_cache.cnt) {
        return res_flag;
    }
    for (i = 0; i < RACE_RELAY_USB_PKT_CACHE_MAX; i++) {
        if (data == g_race_usb_relay_pkt_cache.cache[i].data && usb_dst_port == g_race_usb_relay_pkt_cache.cache[i].usb_port) {
            res_flag = g_race_usb_relay_pkt_cache.cache[i].usb_flag;
            g_race_usb_relay_pkt_cache.cache[i].data = NULL;
            g_race_usb_relay_pkt_cache.cache[i].usb_port = 0xff;
            g_race_usb_relay_pkt_cache.cnt--;
            return res_flag;
        }
    }
    return res_flag;
}

void race_usb_relay_clear_flag(race_port_t port)
{
    uint16_t i;
    for (i = 0; i < RACE_RELAY_USB_PKT_CACHE_MAX; i++) {
        if (port == g_race_usb_relay_pkt_cache.cache[i].usb_port) {
            if (g_race_usb_relay_pkt_cache.cache[i].data) {
                g_race_usb_relay_pkt_cache.cnt--;
            }
            g_race_usb_relay_pkt_cache.cache[i].data = NULL;
            g_race_usb_relay_pkt_cache.cache[i].usb_port = 0xff;
        }
    }
}

void race_usb_relay_add_flag_for_pkt(mux_handle_t handle, uint8_t *pkt)
{
#if defined(MTK_USB_AUDIO_HID_ENABLE)
    race_port_t usb_port = RACE_GET_PORT_BY_MUX_HANDLE(handle);
    uint8_t usb_flag = 0;
    if (RACE_PORT_IS_USB(usb_port)) {
        usb_flag = race_usb_relay_get_flag(usb_port, pkt);
        usb_mux_set_data_flag(usb_flag);
    }
#endif
}

void race_usb_relay_register_map_tbl(const race_usb_relay_map_t *map_tbl)
{
    if (map_tbl) {
        g_race_usb_relay_map.port_map_to_flag = map_tbl->port_map_to_flag;
        g_race_usb_relay_map.flag_map_to_port = map_tbl->flag_map_to_port;
    }
}

void race_usb_relay_init(void)
{
#ifdef AIR_DONGLE_RELAY_RACE_PACKET_ENABLE
    race_usb_relay_dongle_init();
#elif (defined AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE || defined (AIR_DCHS_MODE_MASTER_ENABLE))
    race_usb_relay_cosys_init();
#endif
    memset(&g_race_usb_relay_pkt_cache, 0, sizeof(race_relay_flag_cache_t));
}


#endif



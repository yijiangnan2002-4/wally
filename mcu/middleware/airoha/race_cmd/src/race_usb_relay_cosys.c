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
#include "race_cmd_relay_cmd_cosys.h"
#include "race_port_cosys.h"

#if defined(MTK_USB_DEMO_ENABLED) && defined(MTK_USB_AUDIO_HID_ENABLE)
#include "usbaudio_drv.h"
#endif

/**************************************************************************************************
* Define
**************************************************************************************************/

#if defined (AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined (AIR_DCHS_MODE_MASTER_ENABLE)

/**************************************************************************************************
* Structure
**************************************************************************************************/

typedef struct {
    uint8_t usb_flag;
    race_port_t dst_port;
} race_cosys_relay_map_t;


/**************************************************************************************************
* Static Variable
**************************************************************************************************/

static const race_cosys_relay_map_t g_race_cosys_usb_relay_map_tbl[] = {
    {TARGET_REMOTE_DEVICE,          RACE_DUAL_CHIP_COSYS_PORT},
    {TARGET_EXTERNAL_DEVICE,        RACE_DUAL_CHIP_EMCU_PORT},
};

#if defined(MTK_USB_DEMO_ENABLED) && defined(MTK_USB_AUDIO_HID_ENABLE)
static race_port_t g_race_cosys_relay_usb_port;
#endif

/**************************************************************************************************
* Static Functions
**************************************************************************************************/
#if defined(MTK_USB_DEMO_ENABLED) && defined(MTK_USB_AUDIO_HID_ENABLE)
static bool race_cosys_relay_get_usb_port(race_port_t *port)
{
    uint16_t usb_cnt;
    uint16_t i;
    race_port_t last_port = g_race_cosys_relay_usb_port;
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
#endif

static bool race_cosys_relay_check(race_pkt_relay_info_t *relay_info_out, race_port_t port, race_port_type_t port_type, RACE_COMMON_HDR_STRU *pkt_hdr)
{
#if defined(MTK_USB_DEMO_ENABLED) && defined(MTK_USB_AUDIO_HID_ENABLE)
    uint8_t usb_mux_flag = 0;
    usb_mux_flag = usb_mux_get_data_flag();
    relay_info_out->from_port = port;

    if (RACE_PORT_TYPE_USB == port_type) {
        if (race_relay_get_dst_port_by_usb_flag(usb_mux_flag, &relay_info_out->to_port) == true) {
            relay_info_out->is_relay_pkt = true;
            g_race_cosys_relay_usb_port = port;
            return true;
        }
        if (TARGET_LOCAL_DEVICE != usb_mux_flag) {
            relay_info_out->is_relay_pkt = true;
            relay_info_out->to_port = RACE_INVALID_PORT;
            return true;
        }
    } else if ((port == RACE_DUAL_CHIP_COSYS_PORT || port == RACE_DUAL_CHIP_EMCU_PORT)
               && (pkt_hdr->type == RACE_TYPE_RESPONSE || pkt_hdr->type == RACE_TYPE_NOTIFICATION)) {
        /* for 68 master, relay race cmd response to PC.*/
        race_port_t to_usb_port;
        if (race_cosys_relay_get_usb_port(&to_usb_port) == true) {
            relay_info_out->is_relay_pkt = true;
            relay_info_out->to_port = to_usb_port;
            return true;
        }
    }
#endif
    return false;
}


static bool race_usb_relay_cosys_get_flag_by_port(uint8_t *usb_flag, race_port_t port)
{
    uint16_t i;
    uint16_t buf_size = sizeof(g_race_cosys_usb_relay_map_tbl) / sizeof(g_race_cosys_usb_relay_map_tbl[0]);
    if (port == RACE_INVALID_PORT || NULL == usb_flag) {
        return false;
    }
    for (i = 0; i < buf_size; i++) {
        if (port == g_race_cosys_usb_relay_map_tbl[i].dst_port) {
            *usb_flag = g_race_cosys_usb_relay_map_tbl[i].usb_flag;
            return true;
        }
    }
    return false;
}

static bool race_usb_relay_cosys_get_port_by_flag(race_port_t *port, uint8_t usb_flag)
{
    uint16_t i;
    uint16_t buf_size = sizeof(g_race_cosys_usb_relay_map_tbl) / sizeof(g_race_cosys_usb_relay_map_tbl[0]);
    if (NULL == port) {
        return false;
    }
    for (i = 0; i < buf_size; i++) {
        if (usb_flag == g_race_cosys_usb_relay_map_tbl[i].usb_flag) {
            *port = g_race_cosys_usb_relay_map_tbl[i].dst_port;
            return true;
        }
    }
    return false;
}

void race_usb_relay_cosys_init(void)
{
    bool res = race_register_relay_check_handler(race_cosys_relay_check);
    race_usb_relay_map_t map_tbl = {race_usb_relay_cosys_get_flag_by_port, race_usb_relay_cosys_get_port_by_flag};
    race_usb_relay_register_map_tbl(&map_tbl);
    assert(res && "ERROR: race_usb_relay_cosys_init, register handler fail!");
}

#endif



#if (defined (AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) || defined (AIR_DCHS_MODE_SLAVE_ENABLE))

void *race_cosys_slave_relay_pkt(ptr_race_pkt_t pCmd, uint16_t length, uint8_t channel_id)
{
    if (pCmd->hdr.id == 0x2c82 || pCmd->hdr.id == 0x2c83) {
        if ((pCmd->payload[0] == 0x00) || (pCmd->payload[0] == 0x01)) {
            RACE_LOG_MSGID_I("race_cosys slave APP, id[0x%x], module[%d]", 2, pCmd->hdr.id, pCmd->payload[0]);
            race_relay_send_cosys_to_remote_internal(pCmd, pCmd->hdr.length + 4, channel_id);
        }
    }
    return NULL;
}

#endif



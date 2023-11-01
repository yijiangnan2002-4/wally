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


#ifndef RACE_USB_RELAY_H
#define RACE_USB_RELAY_H


#include "race_core.h"

#ifdef __cplusplus
extern "C"
{
#endif


#if defined(AIR_DONGLE_RELAY_RACE_PACKET_ENABLE) || defined (AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined (AIR_DCHS_MODE_MASTER_ENABLE)
#define RACE_USB_RELAY_ENABLE
#endif

/* To fix feature option auto switch fail */
#ifndef TARGET_REMOTE_DEVICE
#define TARGET_REMOTE_DEVICE        0x80
#endif

#ifndef TARGET_EXTERNAL_DEVICE
#define TARGET_EXTERNAL_DEVICE      0x90
#endif

#ifndef TARGET_LOCAL_DEVICE
#define TARGET_LOCAL_DEVICE         0x00
#endif

#if defined RACE_USB_RELAY_ENABLE


typedef bool (*race_usb_relay_get_flag_by_port_t)(uint8_t *usb_flag, race_port_t port);
typedef bool (*race_usb_relay_get_port_by_flag_t)(race_port_t *port, uint8_t usb_flag);

typedef struct {
    race_usb_relay_get_flag_by_port_t port_map_to_flag;
    race_usb_relay_get_port_by_flag_t flag_map_to_port;
} race_usb_relay_map_t;


#if defined(MTK_USB_AUDIO_HID_ENABLE)
#include "usbaudio_drv.h"
extern uint8_t usb_mux_get_data_flag(void);
extern uint8_t usb_mux_set_data_flag(uint8_t flag);
#endif


void race_usb_relay_init(void);
void race_usb_relay_register_map_tbl(const race_usb_relay_map_t *map_tbl);
uint8_t race_relay_get_usb_tx_flag_by_src_port(race_port_t src_port);
bool race_relay_get_dst_port_by_usb_flag(uint8_t usb_flag, race_port_t *dst_port);
void race_usb_relay_set_flag_by_src_port(race_port_t src_port, race_port_t usb_dst_port, uint8_t *data);
uint8_t race_usb_relay_get_flag(race_port_t usb_dst_port, uint8_t *data);
void race_usb_relay_clear_flag(race_port_t port);
void race_usb_relay_add_flag_for_pkt(mux_handle_t handle, uint8_t *pkt);


#endif


#ifdef __cplusplus
}
#endif


#endif


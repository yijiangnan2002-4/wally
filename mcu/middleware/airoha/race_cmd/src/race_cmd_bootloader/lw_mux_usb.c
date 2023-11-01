/* Copyright Statement:
*
* (C) 2023  Airoha Technology Corp. All rights reserved.
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


#if defined(__EXT_BOOTLOADER__)

#if defined(AIR_BL_DFU_ENABLE)
#if defined(AIR_BL_USB_HID_DFU_ENABLE)

#include "bl_common.h"
#include "bl_usb_hid.h"

uint32_t lw_mux_usb_tx(uint8_t *data, uint32_t len)
{
    uint32_t res_len = 0;
    bl_usb_hid_put(data, len, &res_len);
    bl_print(LOG_DEBUG, "TX:%d,%d\r\n", len, res_len);
    return res_len;
}

uint32_t lw_mux_usb_rx(uint8_t *data, uint32_t len)
{
    uint8_t *rx_buf = data;
    uint32_t buf_len = len;
    uint32_t rx_total_len = 0;
    uint32_t rx_len = 0;
    uint8_t status;
    uint32_t rx_try_cnt = 0;

    do {
        /* break while loop conditions:
        *   1) status != OK
        *   2) rx data len == 0
        *   3) rx data len < USB MAX PKT LEN && rx data len < rx buf size, this condition means: the current pkt is rx done.
        */
        rx_len = 0;
        status = bl_usb_hid_get(rx_buf, buf_len,  &rx_len);
        //bl_print(LOG_DEBUG, "{%d,%d,%d}\r\n", rx_try_cnt, buf_len, rx_len);
        rx_total_len += rx_len;
        rx_buf = &rx_buf[rx_len];
        buf_len -= rx_len;
        rx_try_cnt++;
    } while (BL_USB_HID_STATUS_OK == status && 0 < buf_len && (rx_len == buf_len || BL_USB_HID_PACKET_PAYLOAD_MAXLEN == rx_len));

    if (rx_total_len) {
        bl_print(LOG_DEBUG, "RX:%d,%d,%d\r\n", rx_try_cnt, len, rx_total_len);
    } else {
        bl_print(LOG_DEBUG, "=");
    }
    return rx_total_len;
}


#endif

#endif

#endif


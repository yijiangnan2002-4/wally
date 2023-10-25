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


#include "bl_usb_hid.h"

#if defined(AIR_BL_USB_HID_DFU_ENABLE)

#define USB_DEBUG_LOG_ENABLE

#include <string.h>

#include "usb.h"
#include "usbhid_drv.h"
#include "bl_common.h"
#include "hal_wdt.h"
#include "hal_gpt.h"

extern uint8_t usb_rx[];
extern uint8_t usb_tx[];
extern UsbHid_Struct g_UsbHid;
extern uint8_t usb_report_id;
extern uint8_t usb_report_type;
extern uint32_t usb_report_length;

void usb_hisr(hal_nvic_irq_t n);

static bool s_is_rx_data = false;
static bool s_is_tx_data = false;
static const uint32_t s_init_timeout = BL_USB_HID_INIT_TIMEOUT * 1000;
static const uint32_t s_rx_timeout = BL_USB_HID_RX_TIMEOUT * 1000;
static const uint32_t s_tx_timeout = BL_USB_HID_TX_TIMEOUT * 1000;

static void bl_usb_hid_rx_callback(uint8_t *data, uint32_t data_length)
{

#ifdef USB_DEBUG_LOG_ENABLE
    uint32_t len = (usb_rx[2] << 8) | usb_rx[1];
    bl_print(LOG_DEBUG, "bl_usb_hid_rx_callback len[%d] data:", len);
    for (uint32_t i = 0; i < 6; i++) {
        bl_print(LOG_DEBUG, "%x ", data[USB_HID_AIR_REPORT_DATA_INDEX + 1 + i]);
    }
    bl_print(LOG_DEBUG, "...");

    for (uint32_t i = len - 3; i < len; i++) {
        bl_print(LOG_DEBUG, "%x ", data[USB_HID_AIR_REPORT_DATA_INDEX + 1 + i]);
    }
    bl_print(LOG_DEBUG, "\r\n");
#else
    uint32_t len = (usb_rx[2] << 8) | usb_rx[1];
    bl_print(LOG_DEBUG, "bl_usb_hid_rx_callback len[%d]\r\n", len);
#endif

    if (s_is_rx_data == true) {
        bl_print(LOG_DEBUG, "bl_usb_hid_rx_callback packet is lost \r\n");
    }
    s_is_rx_data = true;
}

static void bl_usb_hid_tx_callback(uint8_t *data, uint32_t data_length)
{
#ifdef USB_DEBUG_LOG_ENABLE
    static uint16_t i = 0;
    if (s_is_tx_data == false) {
        i++;
        if (i == 500) {
            bl_print(LOG_DEBUG, "bl_usb_hid_tx_callback send zero data for 500 times ...\r\n");
            i = 0;
        }
    }
#endif
    s_is_tx_data = false;
}

static void bl_usb_plug_in_cb(usb_evt_t event, void* usb_data, void* user_data)
{
    usb_set_device_type(USB_HID);
    usb_custom_set_speed(false);
    usb_custom_set_product_info(BL_USB_HID_DFU_VID, BL_USB_HID_DFU_PID, BL_USB_HID_DFU_VER);
    usb_custom_set_class_info(0x0200, 0x00, 0x00, 0x00);
    usb_custom_set_string(USB_STRING_USAGE_PRODUCT, BL_USB_HID_DFU_NAME);
}

static void bl_usb_device_init_cb(usb_evt_t event, void* usb_data, void* user_data)
{
    usb_hid_set_dscr_enable(
        (usb_hid_report_dscr_type_t[1]){USB_REPORT_DSCR_TYPE_MUX2},
        1
    );
}

static void bl_usb_clear_tx_rx_buffer()
{
    memset(usb_tx, 0, USB_HID_REPORT_MAX_LEN);
    memset(usb_rx, 0, USB_HID_REPORT_MAX_LEN);
}

bool bl_usb_hid_is_ready(void)
{
    return (USB_Get_Device_State() == DEVSTATE_CONFIG);
}

uint8_t bl_usb_hid_init(void)
{
    bool is_timeout = false;
    uint32_t stime;  /* start time */
    uint32_t ctime;  /* current time */
    uint32_t wtime;  /* waiting time */
    uint8_t ret = BL_USB_HID_STATUS_OK;

    bl_print(LOG_DEBUG, "bl_usb_hid_init\r\n");

    usb_evt_register_cb(USB_USER_APP, USB_EVT_PLUG_IN, bl_usb_plug_in_cb, NULL);
    usb_evt_register_cb(USB_USER_APP, USB_EVT_DEVICE_INIT, bl_usb_device_init_cb, NULL);
    usb_hid_handler_rx_register(USB_HID_AIR2_OUT_REPORT_ID, USB_HID_AIR_REPORT_DATA_LEN, bl_usb_hid_rx_callback);
    usb_hid_handler_tx_register(USB_HID_AIR2_IN_REPORT_ID, USB_HID_AIR_REPORT_DATA_LEN, bl_usb_hid_tx_callback);

    usb_drv_enable();

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &stime);
    while((USB_Get_Device_State() != DEVSTATE_CONFIG) && (is_timeout == false)) {
        hal_wdt_feed(HAL_WDT_FEED_MAGIC);
        usb_hisr(0);

        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &ctime);
        hal_gpt_get_duration_count(stime, ctime, &wtime);
        if (wtime >= s_init_timeout) {
            is_timeout = true;
            ret =  BL_USB_HID_STATUS_TIMEOUT;
            break;
        }
        if (!is_vusb_ready()) {
            ret = BL_USB_HID_VBUS_INVALID;
            break;
        }
    }

    if (ret != BL_USB_HID_STATUS_OK) {
        bl_print(LOG_DEBUG, "bl_usb_hid_init failed [%d]\r\n", ret);
        usb_drv_disable();
        return ret;
    }

    bl_print(LOG_DEBUG, "bl_usb_hid_init done\r\n");
    return BL_USB_HID_STATUS_OK;
}

uint8_t bl_usb_hid_deinit(void)
{
    bl_print(LOG_DEBUG, "bl_usb_hid_deinit\r\n");
    bl_usb_clear_tx_rx_buffer();
    usb_drv_disable();
    bl_print(LOG_DEBUG, "bl_usb_hid_deinit done\r\n");
    return BL_USB_HID_STATUS_OK;
}

uint8_t bl_usb_hid_get(uint8_t *data, uint32_t length, uint32_t *rxlen)
{
    bool is_timeout = false;
    uint32_t stime;  /* start time */
    uint32_t ctime;  /* current time */
    uint32_t wtime;  /* waiting time */

    uint32_t tmplen = 0;

    static uint32_t cache_length = 0;
    static uint8_t* cache_ptr = 0;

    if (!bl_usb_hid_is_ready()) {
        return BL_USB_HID_STATUS_IS_NOT_INIT;
    }

    if(length == 0) {
        bl_print(LOG_DEBUG, "bl_usb_hid_get error, get length zero!\r\n");
        return BL_USB_HID_STATUS_GET_LENGTH_ZERO;
    }

    if (cache_length != 0) {
        bl_print(LOG_DEBUG, "bl_usb_hid_get use cache_length[%d], get length[%d]\r\n", cache_length, length);
        if (cache_length >= length) {
            *rxlen = length;
        } else {
            *rxlen = cache_length;
        }
        memcpy(data, cache_ptr, *rxlen);
        cache_ptr += *rxlen;
        cache_length -= *rxlen;
        if(cache_length == 0) {
            memset(usb_rx, 0, USB_HID_REPORT_MAX_LEN);
            s_is_rx_data = false;
        }
        bl_print(LOG_DEBUG, "bl_usb_hid_get status ok data[%d], length[%d], rxlen[%d]\r\n", data, length, *rxlen);
        return BL_USB_HID_STATUS_OK;
    }

    *rxlen = 0;

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &stime);
    while((s_is_rx_data == false) && (is_timeout == false)) {
        hal_wdt_feed(HAL_WDT_FEED_MAGIC);

        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &ctime);
        hal_gpt_get_duration_count(stime, ctime, &wtime);
        if (wtime >= s_rx_timeout) {
            is_timeout = true;
            return BL_USB_HID_STATUS_TIMEOUT;
        }
        if (!is_vusb_ready()) {
            return BL_USB_HID_VBUS_INVALID;
        }

        usb_hisr(0);
    }

    if (s_is_rx_data == true) {
        if(usb_rx[0] == USB_HID_AIR_OUT_REPORT_ID ||
            usb_rx[0] == USB_HID_AIR2_OUT_REPORT_ID) {
            tmplen = (usb_rx[2] << 8) | usb_rx[1];
            cache_length = tmplen;
            cache_ptr    = &usb_rx[USB_HID_AIR_REPORT_DATA_INDEX + 1];
            if (tmplen >= length) {
                *rxlen = length;
            } else {
                *rxlen = tmplen;
            }
            memcpy(data, cache_ptr, *rxlen);
            cache_ptr += *rxlen;
            cache_length -= *rxlen;
        }
    }

    if(cache_length == 0) {
        memset(usb_rx, 0, USB_HID_REPORT_MAX_LEN);
        s_is_rx_data = false;
    }

    bl_print(LOG_DEBUG, "bl_usb_hid_get status ok data[%d], length[%d], rxlen[%d]\r\n", data, length, *rxlen);
    return BL_USB_HID_STATUS_OK;
}

uint8_t bl_usb_hid_put(uint8_t *data, uint32_t length, uint32_t *txlen)
{
    bool is_timeout = false;
    uint32_t stime;  /* start time */
    uint32_t ctime;  /* current time */
    uint32_t wtime;  /* waiting time */
    uint16_t report_len;

    if (!bl_usb_hid_is_ready()) {
        return BL_USB_HID_STATUS_IS_NOT_INIT;
    }

    *txlen = 0;

    while (length && (is_timeout == false)) {
        if (length >= USB_HID_AIR_REPORT_DATA_LEN) {
            report_len = USB_HID_AIR_REPORT_DATA_LEN;
        } else {
            report_len = length;
        }

        // usb_tx[USB_HID_AIR_REPORT_ID_INDEX] = USB_HID_AIR_IN_REPORT_ID;
        // usb_tx[USB_HID_AIR_REPORT_LEN_INDEX] = report_len;
        usb_tx[USB_HID_AIR_REPORT_ID_INDEX] = USB_HID_AIR2_IN_REPORT_ID;
        usb_tx[1] = length & 0xFF;
        usb_tx[2] = (length >> 8) & 0xFF;
        usb_tx[3] = 0x00;

        memcpy(&usb_tx[4], data + (*txlen), report_len);
        s_is_tx_data = true;

        /* Wait tx done */
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &stime);
        while ((s_is_tx_data == true) && (is_timeout == false)) {
            hal_wdt_feed(HAL_WDT_FEED_MAGIC);

            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &ctime);
            hal_gpt_get_duration_count(stime, ctime, &wtime);
            if (wtime >= s_tx_timeout) {
                is_timeout = true;
                memset(usb_tx, 0, USB_HID_REPORT_MAX_LEN);
                return BL_USB_HID_STATUS_TIMEOUT;
            }
            if (!is_vusb_ready()) {
                memset(usb_tx, 0, USB_HID_REPORT_MAX_LEN);
                return BL_USB_HID_VBUS_INVALID;
            }

            usb_hisr(0);
        }

        /* index move */
        *txlen += report_len;
        length -= report_len;
    }

    memset(usb_tx, 0, USB_HID_REPORT_MAX_LEN);
    return BL_USB_HID_STATUS_OK;
}

#endif  /* AIR_BL_USB_HID_DFU_ENABLE */

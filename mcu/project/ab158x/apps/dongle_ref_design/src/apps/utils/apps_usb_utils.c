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

#include <assert.h>
#include <string.h>

#include "usbhid_drv.h"
#include "hal_nvic_internal.h"
#include "hal_usb_internal.h"
#include "apps_usb_utils.h"
#include "memory_attribute.h"
#include "hal_gpt.h"
#include "hal_gpio.h"

#ifdef AIR_PURE_GAMING_MS_ENABLE
#include "hid_plc.h"
#include "app_key_remap.h"
#endif

#if defined(AIR_NVIDIA_REFLEX_ENABLE)
#include "usb_dbg.h"
#endif

#include "app_dongle_ull_le_hid.h"


static bool app_usb_utils_filter_repeat_zero_pkt(uint8_t* data, uint16_t len);

typedef struct __attribute__((__packed__))
{
    uint8_t report_id;
    uint8_t key;
    int16_t x:12;
    int16_t y:12;
    int8_t wheel;
    int8_t acpan;
} bt_ms_packet_t;

typedef struct __attribute__((__packed__))
{
    uint8_t report_id;
    uint8_t modifier_key;
    uint8_t key[5];
} bt_kb_packet_t;

typedef struct __attribute__((__packed__))
{
    uint8_t report_id;
    uint8_t modifier_key;
    uint8_t key[15];
} kb_nkey_packet_t;

typedef struct __attribute__((__packed__))
{
    uint8_t report_id:5;
    uint8_t latency_h:3;
    uint8_t key:5;
    int8_t wheel:3;
    int16_t x;
    int16_t y;
    uint8_t latency_l;
} bt_nvms_packet_t;

kb_nkey_packet_t kb_n_key_report_data;

static void app_usb_utils_report_convert_ms_to_boot(uint8_t *in, uint8_t *out, uint16_t inlen, uint16_t *outlen)
{
    out[0] = ((bt_ms_packet_t*)in)->key;

    /**
     * Method 1 bitwise op
     * Less time cost
     */
    /* x, y Convert int12 to int8 */
    if (((in[3] & 0b00001111) != 0b0000) && ((in[3] & 0b00001111) != 0b1111)) {
        /* Over 8bit case, 0x7F add sign bit */
        out[1] = 0x7F + ((in[3] & 0b00001000) >> 3);
    } else {
        out[1] = in[2];
    }
    if (((in[4] & 0xF0) != 0x00) && ((in[4] & 0xF0) != 0xF0)) {
        /* Over 8bit case, 0x7F add sign bit */
        out[2] = 0x7F + ((in[4] & 0b10000000) >> 7);
    } else {
        out[2] = ((in[4] & 0x0F) << 4) | ((in[3] & 0xF0) >> 4);
    }

    /**
     * Method 2 struct cast
     * More readable but more time cost
     */
#if 0
    bt_ms_packet_t *packet = (bt_ms_packet_t *)in;
    if (packet->x < -128) {
        out[1] = 0x80;
    } else if (packet->x > 127) {
        out[1] = 0x7F;
    } else {
        out[1] = packet->x & 0xFF;
    }

    if (packet->y < -128) {
        out[2] = 0x80;
    } else if (packet->y > 127) {
        out[2] = 0x7F;
    } else {
        out[2] = packet->y & 0xFF;
    }
#endif

    /* Byte 3~7 are reserved */
    out[3] = 0;
    out[4] = 0;
    out[5] = 0;
    out[6] = 0;
    out[7] = 0;
    *outlen = 8;
}

static void app_usb_utils_report_convert_kb_to_boot(uint8_t *in, uint8_t *out, uint16_t inlen, uint16_t *outlen)
{
    out[0] = ((bt_kb_packet_t*)in)->modifier_key; /* Modifier Key */
    out[1] = 0;                                   /* Reserved */
    out[2] = ((bt_kb_packet_t*)in)->key[0];       /* Key 0 */
    out[3] = ((bt_kb_packet_t*)in)->key[1];       /* Key 1 */
    out[4] = ((bt_kb_packet_t*)in)->key[2];       /* Key 2 */
    out[5] = ((bt_kb_packet_t*)in)->key[3];       /* Key 3 */
    out[6] = ((bt_kb_packet_t*)in)->key[4];       /* Key 4 */
    out[7] = 0;						              /* Key 5 */
    *outlen = 8;
}

static void app_usb_utils_report_convert_kb_nkey_to_boot(uint8_t *in, uint8_t *out, uint16_t inlen, uint16_t *outlen)
{
    out[0] = ((kb_nkey_packet_t *)in)->modifier_key; /* Modifier Key */
    out[1] = 0;                                      /* Reserved */
    out[7] = 0;  
    *outlen = 8;
    int i = 0;
    int index = 2;

    for (i = 2; i < inlen; i++)
    {
        if (in[i] != 0)
        {
            if ((in[i] & 0x80) != 0)
            {
                out[index++] = (uint8_t)((i - 2) * 8 + 7);
                if (index == 7)
                {
                    return;
                }
            }
            if ((in[i] & 0x40) != 0)
            {
                out[index++] = (uint8_t)((i - 2) * 8 + 6);
                if (index == 7)
                {
                    return;
                }
            }
            if ((in[i] & 0x20) != 0)
            {
                out[index++] = (uint8_t)((i - 2) * 8 + 5);
                if (index == 7)
                {
                    return;
                }
            }
            if ((in[i] & 0x10) != 0)
            {
                out[index++] = (uint8_t)((i - 2) * 8 + 4);
                if (index == 7)
                {
                    return;
                }
            }
            if ((in[i] & 0x08) != 0)
            {
                out[index++] = (uint8_t)((i - 2) * 8 + 3);
                if (index == 7)
                {
                    return;
                }
            }
            if ((in[i] & 0x04) != 0)
            {
                out[index++] = (uint8_t)((i - 2) * 8 + 2);
                if (index == 7)
                {
                    return;
                }
            }
            if ((in[i] & 0x02) != 0)
            {
                out[index++] = (uint8_t)((i - 2) * 8 + 1);
                if (index == 7)
                {
                    return;
                }
            }
            if ((in[i] & 0x01) != 0)
            {
                out[index++] = (uint8_t)((i - 2) * 8 + 0);
                if (index == 7)
                {
                    return;
                }
            }
        }
    }
    return;
}

#if defined(AIR_NVIDIA_REFLEX_ENABLE)
#define NVRF_MULTI_LKEY_DETECT_WORKAROUND

static uint32_t s_nv_lat_device = 0;
static uint32_t s_nv_lat_dongle = 0;
static uint32_t s_nv_lat_const  = 0;
static uint32_t s_nv_lat_errmsk = 0;
static uint32_t s_nv_lat_total;
static uint32_t s_nv_lat_encode;

static volatile uint32_t s_nv_time_btrx = 0;
static volatile uint32_t s_nv_time_usbtx = 0;

#ifdef NVRF_MULTI_LKEY_DETECT_WORKAROUND
static uint8_t pre_lkey_in[7] = {0};
static uint8_t detect_lkey_duplicate = 0;
#endif /* NVRF_MULTI_LKEY_DETECT_WORKAROUND */
static uint8_t s_nv_pre_lkey  = 0;

static void app_usb_utils_nvms_clear_vars(void)
{
    s_nv_lat_device = 0;
    s_nv_lat_dongle = 0;
    s_nv_lat_errmsk = 0;
    s_nv_lat_total  = 0;
    s_nv_lat_encode = 0;

    s_nv_time_btrx  = 0;
    s_nv_time_usbtx = 0;

    s_nv_pre_lkey = 0;
}

void app_usb_utils_nvms_print_dbg_log(void)
{
    uint32_t savedMask;
    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    if (s_nv_lat_device) {
        LOG_MSGID_I(apps, "nvms_print_dbg_log dev:%d don:%d con:%d err:%d total:%d encode:%d",
            6, s_nv_lat_device, s_nv_lat_dongle, s_nv_lat_const, s_nv_lat_errmsk, s_nv_lat_total, s_nv_lat_encode);
    }
    hal_nvic_restore_interrupt_mask(savedMask);
}

void app_usb_utils_nvms_set_timer_usb_tx_done(void)
{
    uint32_t savedMask;
    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&s_nv_time_usbtx);
    hal_nvic_restore_interrupt_mask(savedMask);
}

static void app_usb_utils_report_convert_nvms(uint8_t *in, uint8_t *out, uint16_t inlen, uint16_t *outlen)
{
    bt_nvms_packet_t *packet = (bt_nvms_packet_t *)in;
    uint8_t cur_lkey = 0;
    uint8_t duplicate_packet_detect __unused = 0;
    extern uint32_t pka_return_hdl_intr_time(void);

    s_nv_lat_device = (packet->latency_h << 8) | packet->latency_l;
    cur_lkey        = packet->key;

    if (!s_nv_pre_lkey && cur_lkey && s_nv_lat_device == 0) {
        /* First L-Key Packet */
        s_nv_time_btrx = pka_return_hdl_intr_time();
#ifdef NVRF_MULTI_LKEY_DETECT_WORKAROUND
        pre_lkey_in[0] = in[0];
        pre_lkey_in[1] = in[1];
        pre_lkey_in[2] = in[2];
        pre_lkey_in[3] = in[3];
        pre_lkey_in[4] = in[4];
        pre_lkey_in[5] = in[5];
        pre_lkey_in[6] = in[6];
        detect_lkey_duplicate = 1;
#endif /* NVRF_MULTI_LKEY_DETECT_WORKAROUND */
    }
    else if (s_nv_pre_lkey && cur_lkey && s_nv_lat_device == 0) {
#ifdef NVRF_MULTI_LKEY_DETECT_WORKAROUND
        uint8_t idx;
        if (detect_lkey_duplicate) {
            for (idx = 0; idx < 7; idx++) {
                if (pre_lkey_in[idx] != in[idx]) {
                    break;
                }
            }
            if (idx == 7) {
                duplicate_packet_detect = 1;
            }
            detect_lkey_duplicate = 0;
        }
#endif /* NVRF_MULTI_LKEY_DETECT_WORKAROUND */
        if (duplicate_packet_detect == 0) {
            /* See as L-Key Packet */
            s_nv_time_btrx = pka_return_hdl_intr_time();
        }
    }
    else if (s_nv_pre_lkey && cur_lkey && s_nv_lat_device) {
        /* Latency Packet */
        if (s_nv_lat_device == 0x07FF) {
            s_nv_lat_errmsk = 0xF000;
        } else {
            s_nv_lat_errmsk = 0x0000;
        }

        /* Caculate Latency*/
        s_nv_lat_device = s_nv_lat_device * 10;
        if (s_nv_time_usbtx > s_nv_time_btrx) {
            s_nv_lat_dongle = s_nv_time_usbtx - s_nv_time_btrx;
        } else {
            s_nv_lat_dongle = s_nv_time_btrx - s_nv_time_usbtx + 1;
        }
        s_nv_lat_total  = s_nv_lat_device + s_nv_lat_dongle + s_nv_lat_const;
        s_nv_lat_encode = (s_nv_lat_total/10) | s_nv_lat_errmsk;
    }
    else if (!s_nv_pre_lkey && cur_lkey && s_nv_lat_device ) {
        s_nv_lat_errmsk = 0xF000;
        s_nv_lat_dongle = 0;
        s_nv_lat_total  = s_nv_lat_device + s_nv_lat_dongle + s_nv_lat_const;
        s_nv_lat_encode = (s_nv_lat_total/10) | s_nv_lat_errmsk;
    }
    else if (s_nv_pre_lkey && !cur_lkey) {
        /* L-Key Released */
        app_usb_utils_nvms_clear_vars();
    }
    else {
        /* Do-nothing */
    }

    s_nv_pre_lkey = cur_lkey;

#ifdef NVRF_MULTI_LKEY_DETECT_WORKAROUND
    if (duplicate_packet_detect) {
        *outlen = 0;
        goto _report_convert_nvms_end;
    }
#endif /* NVRF_MULTI_LKEY_DETECT_WORKAROUND */

    /* Gen Report */
    out[0] = packet->report_id;             /* Report ID */
    out[1] = packet->key;                   /* Keys */
    out[2] = 0;                             /* Reserved (0x00) */
    out[7] = packet->wheel;                 /* Wheel */
    out[11] = 0;                            /* Reserved (0x00) */
    out[12] = 0;                            /* Reserved (0x00) */

    if (s_nv_lat_device) {
        out[3] = 0;                         /* X LSB */
        out[4] = 0;                         /* X HSB */
        out[5] = 0;                         /* Y LSB */
        out[6] = 0;                         /* Y HSB */
        out[8]  = 0x03;                     /* Nvidia Latency - Extension ID */
        out[9]  = s_nv_lat_encode & 0xFF;   /* Nvidia Latency LSB */
        out[10] = (s_nv_lat_encode >> 8);   /* Nvidia Latency MSB */
    } else {
        out[3] = in[2];                     /* X LSB */
        out[4] = in[3];                     /* X HSB */
        out[5] = in[4];                     /* Y LSB */
        out[6] = in[5];                     /* Y HSB */
        out[8]  = 0x00;                     /* Nvidia Latency - Extension ID */
        out[9]  = 0x00;                     /* Nvidia Latency LSB */
        out[10] = 0x00;                     /* Nvidia Latency MSB */
    }
    *outlen = 13;

#ifdef NVRF_MULTI_LKEY_DETECT_WORKAROUND
_report_convert_nvms_end:
#endif /* NVRF_MULTI_LKEY_DETECT_WORKAROUND */
    if (usb_dbg_is_opt_enable(USB_DBG_OPT_RSVD_17)) {
        LOG_HEXDUMP_I(apps, "report_convert_nvms len:%2d", out, *outlen, *outlen);
    }
}

static void app_usb_utils_report_convert_nvms_to_boot(uint8_t *in, uint8_t *out, uint16_t inlen, uint16_t *outlen)
{
    bt_nvms_packet_t *packet = (bt_nvms_packet_t *)in;
    out[0] = packet->key;

    if (packet->x < -128) {
        out[1] = 0x80;
    } else if (packet->x > 127) {
        out[1] = 0x7F;
    } else {
        out[1] = packet->x & 0xFF;
    }

    if (packet->y < -128) {
        out[2] = 0x80;
    } else if (packet->y > 127) {
        out[2] = 0x7F;
    } else {
        out[2] = packet->y & 0xFF;
    }

    /* Byte 3~7 are reserved */
    out[3] = 0;
    out[4] = 0;
    out[5] = 0;
    out[6] = 0;
    out[7] = 0;
    *outlen = 8;
}
#endif /* AIR_NVIDIA_REFLEX_ENABLE */

typedef struct{
    bool need_plc;
    bool revised;
    uint8_t reason;
    uint8_t channel;
    uint8_t dup;
    uint8_t fail;
    uint8_t raw_data[3];
    uint8_t proc_data[3];
} usb_plc_cb_param;

void app_usb_utils_bt_callback(uint8_t* data, uint16_t len, usb_plc_cb_param *param);

#ifdef AIR_PURE_GAMING_MS_ENABLE

ATTR_ZIDATA_IN_TCM uint8_t bt_data[7];
ATTR_ZIDATA_IN_TCM volatile uint16_t bt_len;
ATTR_ZIDATA_IN_TCM usb_plc_cb_param bt_plc_param;

ATTR_TEXT_IN_TCM void usb_hid_bt_callback_in_sw_isr(void)
{
    // hal_gpio_set_output(USB_DEBUG_GPIO_2, HAL_GPIO_DATA_HIGH);
    app_usb_utils_bt_callback(&bt_data[0], bt_len, &bt_plc_param);
    // hal_gpio_set_output(USB_DEBUG_GPIO_2, HAL_GPIO_DATA_LOW);
}

ATTR_TEXT_IN_TCM void usb_hid_bt_callback_in_bt_isr(uint8_t* data, uint16_t len, usb_plc_cb_param *param)
{
    // hal_gpio_set_output(USB_DEBUG_GPIO_1, HAL_GPIO_DATA_HIGH);
    memcpy(&bt_data, data, len);
    bt_len = len;
    memcpy(&bt_plc_param, param, sizeof(usb_plc_cb_param));
    if ((param->need_plc) || (param->channel == 1))
    {
        hal_nvic_set_pending_irq(WDT_IRQn);
    }
    else
    {
        app_usb_utils_bt_callback(&bt_data[0], bt_len, &bt_plc_param);
    }
    // hal_gpio_set_output(USB_DEBUG_GPIO_1, HAL_GPIO_DATA_LOW);
}
#endif

ATTR_TEXT_IN_TCM static bool app_usb_utils_filter_repeat_zero_pkt(uint8_t* data, uint16_t len)
{
    static uint8_t g_ms_normal_zero_pkt_cnt = 0;
    static uint8_t g_ms_bios_zero_pkt_cnt = 0;

    if (len < 1) {
        return false;
    }

    if (hal_usb_get_suspend_state() == HAL_USB_POWER_STATE_SUSPEND) {
        /* if usb suspend, clear zero pkt cnt */
        g_ms_normal_zero_pkt_cnt = 0;
        g_ms_bios_zero_pkt_cnt = 0;
    }

    if (len == 7) { // normal mode
        g_ms_bios_zero_pkt_cnt = 0;
        if (data[0] == BT_HID_REPORT_ID_MS) {
            if ((((*(volatile uint32_t *)data) & 0xffffff00) == 0)
                && (*(volatile uint32_t *)(data + 3) == 0)) {
                /* all data is zero case */
                if(g_ms_normal_zero_pkt_cnt <= 1) {
                    g_ms_normal_zero_pkt_cnt++;
                }
            } else {
                g_ms_normal_zero_pkt_cnt = 0;
            }
            if(g_ms_normal_zero_pkt_cnt <= 1) {
                return true;
            }
        } else if (data[0] == BT_HID_REPORT_ID_KB) {
            g_ms_normal_zero_pkt_cnt = 0;
            return true;
        }
    } else if (len == 8) {  //BIOS mode
        g_ms_normal_zero_pkt_cnt = 0;
        // first 3 bytes is zero
        if (((*(volatile uint32_t *)data) & 0x00ffffff) == 0) {
            if(g_ms_bios_zero_pkt_cnt <= 1) {
                g_ms_bios_zero_pkt_cnt++;
            }
        } else {
            g_ms_bios_zero_pkt_cnt = 0;
        }

        if(g_ms_bios_zero_pkt_cnt <= 1) {
            return true;
        }
    } else if (len == 17||len==6) {
        return true;
    }
    return false;
}

#if defined(AIR_PURE_GAMING_MS_ENABLE)
/* true => send this package
 * false => don't need to send this package
 */
static uint8_t s_report_package[7] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
ATTR_TEXT_IN_TCM static bool app_usb_utils_filter_repeat_pkt(uint8_t* data, uint16_t len)
{
    int cmp_result = 0;
    if ((data == NULL) || (len < 1) || (len > 7)) {
        return false;
    }
    cmp_result = memcmp(data, s_report_package, 7);
    memcpy(s_report_package, data, 7);
    if(cmp_result){
        return true;
    }

    return false;
}
#endif

extern volatile uint8_t g_key_output;
ATTR_TEXT_IN_TCM void app_usb_utils_bt_callback(uint8_t* data, uint16_t len, usb_plc_cb_param *param)
{
    uint8_t report_id = 0;
    uint8_t port = USB_HID_MAX_DEVICE_NUM;
    uint8_t tx_status = USB_HID_STATUS_OK;
    ATTR_ALIGN(4) uint8_t cvtdata[16] = {0};
    uint8_t *pdata = NULL;
    uint16_t datalen = 0;
    static uint32_t s_drop_cnt = 0;

    if(len > USB_HID_EP_MAX_SIZE) {
        assert(0 && "data is exceed USB_HID_EP_MAX_SIZE.");
    }

    if (len) {
        report_id = data[0] & 0x1F;
        pdata = data;
        datalen = len;
        // LOG_MSGID_I(USBHID, "app_usb_utils_bt_callback, report_id:%d,plc:%d,reason:%d, channel:%d, dump:%d, fail:%d, raw:%x,%x,%x;proc:%x,%x,%x", 12, report_id, param->need_plc, param->reason, param->channel,
        //             param->dup,param->fail,param->raw_data[0],param->raw_data[1],param->raw_data[2],param->proc_data[0],param->proc_data[1],param->proc_data[2]);
    }
    // LOG_MSGID_I(USBHID, "app_usb_utils_bt_callback, plc:%d, report_id:%d, reason:%d, revised:%d", 4, param->need_plc, report_id, param->reason, param->revised);

#if defined(AIR_PURE_GAMING_MS_ENABLE)

    bool plc_flag = param->need_plc;
    bool plc_revised = param->revised;
    uint8_t plc_reason = param->reason;
    uint8_t plc_channel = param->channel;

_retry_send_data:

#ifdef USB_HID_DEBUG_GPIO_ENABLE
    // if (usb_dbg_is_opt_enable(USB_DBG_OPT_RSVD_HID_GPIO)) {
        // hal_gpio_set_output(USB_DEBUG_GPIO_3, HAL_GPIO_DATA_HIGH);
    // }
#endif

    // key remap before PLC
    if (report_id == BT_HID_REPORT_ID_MS && len > 1) {
        app_key_remap_key_status_update(&data[1], plc_reason);
    }

    if (plc_flag) {
        /* in this case, report_id is 0 */
        /* Mouse */
        port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_GMOUSE);

        int16_t nx = 0;
        int16_t ny = 0;
        if (len == 0) {
            app_hid_plc_handler(0, 0, true, plc_revised, plc_channel, &nx, &ny);
        }

        ATTR_ALIGN(4) uint8_t plcdata[16] = { 0 };
        plcdata[0] = BT_HID_REPORT_ID_MS;
        // save pre-packet keyvalue
        plcdata[1] = g_key_output;
        plcdata[2] = nx & 0xFF;
        plcdata[3] = ((nx >> 8) & 0x0F) | ((ny & 0x0F) << 4);
        plcdata[4] = (ny >> 4);
        // need to handle save pre-packet info
        plcdata[5] = 0;
        plcdata[6] = 0;
        plcdata[7] = 0;

        #define PLC_DATA_LEN (7)

        pdata = plcdata;
        datalen = PLC_DATA_LEN;

        /* In Boot Mode */
        if (usb_hid_is_boot_mode(port) == USB_HID_PROTOCOL_BOOT) {
            app_usb_utils_report_convert_ms_to_boot(plcdata, cvtdata, PLC_DATA_LEN, &datalen);
            pdata = cvtdata;
        }
    }
    else if (report_id == BT_HID_REPORT_ID_MS)
#else
    if (report_id == BT_HID_REPORT_ID_MS)
#endif
    {
        /* Mouse */
        port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_GMOUSE);
#if defined(AIR_PURE_GAMING_MS_ENABLE)
        // need to correct xy hid packet
        int16_t x = ((data[3] & 0x0F) << 8) | data[2];
        int16_t y = ((data[4] << 4)) | (data[3] >> 4);
        /* int12 -> int16 */
        if (x & 0x800)
        {
            x = x | 0xF000;
        }
        if (y & 0x800)
        {
            y = y | 0xF000;
        }

        int16_t mx = 0;
        int16_t my = 0;
        if (plc_reason == 0) { // normal packet
            g_key_output = data[1];
        } else if (plc_reason == 0x4) { // in case of mouse can't move case
            app_hid_plc_clear();
            if(app_usb_utils_filter_repeat_pkt(data, len)){
                tx_status = usb_hid_tx_non_blocking(port, data, datalen);
                if (tx_status != USB_HID_STATUS_OK) {
                    memset(s_report_package, 0xFF, sizeof(s_report_package));
                }
            }
            goto _usb_hid_bt_callback_end;
        } else {
            data[1] = g_key_output;
        }
        app_hid_plc_handler(x, y, false, plc_revised, plc_channel, &mx, &my);
        data[2] = mx & 0xFF;
        data[3] = ((mx >> 8) & 0x0F) | ((my & 0x0F) << 4);
        data[4] = (my >> 4);
#endif

        /* In Boot Mode */
        if (usb_hid_is_boot_mode(port) == USB_HID_PROTOCOL_BOOT) {
            app_usb_utils_report_convert_ms_to_boot(data, cvtdata, len, &datalen);
            pdata = cvtdata;
        }
    } else if (report_id == BT_HID_REPORT_ID_KB) {
        /* Keyboard */
        port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_GKEYBOARD);

        /* In Boot Mode */
        if (usb_hid_is_boot_mode(port) == USB_HID_PROTOCOL_BOOT) {
            app_usb_utils_report_convert_kb_to_boot(data, cvtdata, len, &datalen);
            pdata = cvtdata;
        }
    } else if (report_id == BT_HID_REPORT_ID_CONSUMER) {
        /* Comsumer */
        port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_GKEYBOARD);

        uint8_t consumer_data[6]={0x0,0x0,0x0,0x0,0x0,0x0};
        consumer_data[0]=data[0];
        consumer_data[1]=data[2];
        consumer_data[2]=data[3];
        consumer_data[3]=data[4];
        consumer_data[4]=data[5];
        consumer_data[5]=data[6];
        pdata=consumer_data;
        datalen=6;

        /* In Boot Mode */
        if (usb_hid_is_boot_mode(port) == USB_HID_PROTOCOL_BOOT) {
            /* Do nothing */
            pdata = NULL;
            datalen = 0;
        }
    } else if (report_id == BT_HID_REPORT_ID_KB_NKEY_GROUP1 
	|| report_id == BT_HID_REPORT_ID_KB_NKEY_GROUP2 
	|| report_id == BT_HID_REPORT_ID_KB_NKEY_GROUP3) {
        port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_GKEYBOARD);
        kb_n_key_report_data.report_id = BT_HID_REPORT_ID_KB;
        if (report_id == BT_HID_REPORT_ID_KB_NKEY_GROUP1)
        {
            memcpy((uint8_t *)&(kb_n_key_report_data.modifier_key), &data[1], 6);
            // memcpy((uint8_t *)&(kb_n_key_report_data.key[0]), &data[2], 5);
        }
        if (report_id == BT_HID_REPORT_ID_KB_NKEY_GROUP2)
        {
            memcpy((uint8_t *)&(kb_n_key_report_data.key[5]), &data[2], 5);
        }
        if (report_id == BT_HID_REPORT_ID_KB_NKEY_GROUP3)
        {
            memcpy((uint8_t *)&(kb_n_key_report_data.key[10]), &data[2], 5);
        }
        pdata = (uint8_t *)&kb_n_key_report_data.report_id;
        datalen = 17;

        if (usb_hid_is_boot_mode(port) == USB_HID_PROTOCOL_BOOT)
        {
            app_usb_utils_report_convert_kb_nkey_to_boot((uint8_t *)&kb_n_key_report_data.report_id, cvtdata, 17, &datalen);
            pdata = cvtdata;
        }
    }
#if defined(AIR_NVIDIA_REFLEX_ENABLE)
    else if (report_id == BT_HID_REPORT_ID_NVMS) {
        /* Mouse for Nvidia */
        port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_GMOUSE_NV);

        /* In Boot Mode */
        if (usb_hid_is_boot_mode(port) == USB_HID_PROTOCOL_BOOT) {
            app_usb_utils_report_convert_nvms_to_boot(data, cvtdata, len, &datalen);
            pdata = cvtdata;
        } else {
            app_usb_utils_report_convert_nvms(data, cvtdata, len, &datalen);
            pdata = cvtdata;
        }
    }
#endif

    if (port != USB_HID_MAX_DEVICE_NUM) {
#if defined(AIR_PURE_GAMING_MS_ENABLE)
        static volatile bool g_add_pkt_flag = false;
        if (plc_reason == 0x05 && plc_flag == true && g_add_pkt_flag == false) {
            if (app_usb_utils_filter_repeat_zero_pkt(pdata, datalen)) {
                usb_hid_tx_non_blocking(port, pdata, datalen);
            }
            // GPT delay 250us, modify reason to 1, revert channel
            hal_gpt_delay_us(250);
            plc_reason = 0x01;
            plc_channel = !plc_channel;
            g_add_pkt_flag = true;
            goto _retry_send_data;
        } else if (plc_reason == 0x01 && plc_flag == true && g_add_pkt_flag == true) {
            if (app_usb_utils_filter_repeat_zero_pkt(pdata, datalen)) {
                usb_hid_tx_non_blocking(port, pdata, datalen);
            }
            //LOG_MSGID_I(USBHID, "app_usb_utils_bt_callback reason(1)! plc_channel:%d", 1, plc_channel);
            g_add_pkt_flag = false;
        } else if (plc_reason == 0x06 && plc_flag == false && g_add_pkt_flag == false) {
            if (app_usb_utils_filter_repeat_zero_pkt(pdata, datalen)) {
                usb_hid_tx_non_blocking(port, pdata, datalen);
            }
            // GPT delay 250us, modify reason to 1, need plc to true, revert channel
            hal_gpt_delay_us(250);
            plc_reason = 0x01;
            plc_channel = !plc_channel;
            plc_flag = true;
            //LOG_MSGID_I(USBHID, "app_usb_utils_bt_callback reason(6)! plc_channel:%d", 1, plc_channel);
            g_add_pkt_flag = true;
            goto _retry_send_data;
        } else {
            if (app_usb_utils_filter_repeat_zero_pkt(pdata, datalen)) {
                tx_status = usb_hid_tx_non_blocking(port, pdata, datalen);
                if (tx_status != USB_HID_STATUS_OK) {
                    if(s_drop_cnt % 400 == 0) {
                        LOG_MSGID_E(USBHID, "app_usb_utils_bt_callback droped a packet! cnt:%d", 1, s_drop_cnt + 1);
                    }
                    s_drop_cnt++;
                }
            }
            g_add_pkt_flag = false;
        }
#else
        if (app_usb_utils_filter_repeat_zero_pkt(pdata, datalen)) {
            tx_status = usb_hid_tx_non_blocking(port, pdata, datalen);
            if (tx_status != USB_HID_STATUS_OK) {
                if(s_drop_cnt % 400 == 0) {
                    LOG_MSGID_E(USBHID, "app_usb_utils_bt_callback droped a packet! cnt:%d", 1, s_drop_cnt + 1);
                }
                s_drop_cnt++;
            }
        }
#endif
    }

#if defined(AIR_PURE_GAMING_MS_ENABLE)
_usb_hid_bt_callback_end:
#ifdef USB_HID_DEBUG_GPIO_ENABLE
    // if (usb_dbg_is_opt_enable(USB_DBG_OPT_RSVD_HID_GPIO)) {
        // hal_gpio_set_output(USB_DEBUG_GPIO_3, HAL_GPIO_DATA_LOW);
    // }
#endif
#endif

    return;
}

void app_usb_utils_register_bt_cb(void)
{
#ifdef AIR_PURE_GAMING_MS_ENABLE
    hal_nvic_disable_irq(WDT_IRQn);
    hal_nvic_set_priority(WDT_IRQn, (DEFAULT_IRQ_PRIORITY + 1));
    hal_nvic_register_isr_handler(WDT_IRQn, (hal_nvic_isr_t)usb_hid_bt_callback_in_sw_isr);
    hal_nvic_enable_irq(WDT_IRQn);

    extern void pka_hid_data_forward_callback_register(void* fcallback);
    // pka_hid_data_forward_callback_register((void *)usb_hid_bt_callback);
    pka_hid_data_forward_callback_register((void *)usb_hid_bt_callback_in_bt_isr);
#else
    extern void pka_hid_data_forward_callback_register(void* fcallback);
    pka_hid_data_forward_callback_register((void *)app_usb_utils_bt_callback);
#endif
}

void app_usb_utils_preintr_callback(uint8_t port)
{
#if defined(AIR_NVIDIA_REFLEX_ENABLE)
    app_usb_utils_nvms_set_timer_usb_tx_done();
#endif
}

void app_usb_utils_register_preintr_cb(void)
{
    uint8_t port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_GMOUSE);
    usb_hid_pretx_intr_register(port, app_usb_utils_preintr_callback);
}

void app_usb_utils_intr_callback(uint8_t port)
{
#if defined(AIR_NVIDIA_REFLEX_ENABLE)
    if (usb_dbg_is_opt_enable(USB_DBG_OPT_RSVD_18)) {
        app_usb_utils_nvms_print_dbg_log();
    }
#endif
}

void app_usb_utils_register_intr_cb(void)
{
    uint8_t port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_GMOUSE);
    usb_hid_tx_intr_register(port, app_usb_utils_intr_callback);
}

void app_usb_utils_hid_output_cb(uint8_t *data, uint32_t data_length)
{
    LOG_MSGID_I(USBHID, "app_usb_utils_hid_output_cb data_length:%d, data[0]:%d, data[1]:%d", 3, data_length, data[0], data[1]);
    if(!data || data[0] != BT_HID_REPORT_ID_KB){
        return;
    }
    bt_ull_le_hid_srv_action(BT_ULL_ACTION_LE_HID_CONTROL_RGB, (const void *)(data + 1), data_length);
}

void app_usb_utils_register_hid_output_cb(void)
{
    usb_hid_handler_rx_register(BT_HID_REPORT_ID_KB, 2, app_usb_utils_hid_output_cb);
}

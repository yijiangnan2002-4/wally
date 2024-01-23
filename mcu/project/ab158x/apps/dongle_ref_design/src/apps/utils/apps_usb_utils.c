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

#define HID_NORMAL_DATA_LEN (7)
#define HID_BIOS_DATA_LEN (8)
#define HID_CONSUMER_DATA_LEN (6)
#define HID_NKEY_DATA_LEN (15)
#define HID_NKEY_PKT_DATA_LEN (2 + HID_NKEY_DATA_LEN)
#if defined(AIR_NVIDIA_REFLEX_ENABLE)
#define HID_NVIDIA_DATA_LEN (13)
#endif

// #define HID_DATA_DEBUG
// #define KEY_MISSING_DEBUG

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
    uint8_t key[HID_NKEY_DATA_LEN];
} kb_nkey_packet_t;

typedef struct __attribute__((__packed__))
{
    uint8_t report_id:4;
    uint8_t flag_rsv:1;
    int8_t wheel:3;
    uint8_t key:5;
    uint8_t latency_h:3;
    int16_t x;
    int16_t y;
    uint8_t latency_l;
} bt_nvms_packet_t;

typedef struct{
    bool need_plc;
    bool revised;
    /* 0 --- RX success
     * 1 --- RX fail
     * 2 --- useless packets in LEC format that are not expected to appear in HID scenarios
     * 3 --- empty packet
     * 4 --- two consecutive empty packets( the accumulated amount of PLCs will be cleared )
     * 5 --- WB scan PLC needed in 1st and 2nd RX
     * 6 --- WB scan PLC needed in 2nd RX
     */
    uint8_t reason;
    uint8_t channel;
    uint8_t dup;
    uint8_t fail;
    uint8_t raw_data[3];
    uint8_t proc_data[3];
} usb_plc_cb_param;

static kb_nkey_packet_t s_kb_n_key_report_data;
#if defined(AIR_PURE_GAMING_KB_ENABLE)
bool s_tx_faild = false;
#endif

#if defined(AIR_NVIDIA_REFLEX_ENABLE)
uint8_t app_usb_utils_hid_report_convert_nvidia(uint8_t* data_in, uint8_t len_in, uint8_t* data_out, uint16_t *len_out);
#endif

static void app_usb_utils_bt_callback(uint8_t* data, uint16_t len, usb_plc_cb_param *param);

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
    *outlen = HID_BIOS_DATA_LEN;
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
    *outlen = HID_BIOS_DATA_LEN;
}

static void app_usb_utils_report_convert_kb_nkey_to_boot(uint8_t *in, uint8_t *out, uint16_t inlen, uint16_t *outlen)
{
    out[0] = ((kb_nkey_packet_t *)in)->modifier_key; /* Modifier Key */
    out[1] = 0;                                      /* Reserved */
    out[7] = 0;
    *outlen = HID_BIOS_DATA_LEN;

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
    int j;
    for(j=index;j<7;j++)
    {
       out[j]=0; 
    }
}

#if defined(AIR_NVIDIA_REFLEX_ENABLE)
static volatile uint32_t s_nv_time_usbtx = 0;
ATTR_TEXT_IN_TCM void app_usb_utils_nvms_set_timer_usb_tx_done(void)
{
    uint32_t savedMask = 0;
    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, (uint32_t *)&s_nv_time_usbtx);
    hal_nvic_restore_interrupt_mask(savedMask);
}

ATTR_TEXT_IN_TCM static void app_usb_utils_hid_report_to_nvms(uint8_t *in, uint8_t *out, uint16_t inlen, uint16_t *outlen)
{
    int16_t x_in = ((in[3] & 0x0F) << 8) | in[2];
    int16_t y_in = ((in[4] << 4)) | (in[3] >> 4);
    /* int12 -> int16 */
    if (x_in & 0x800) {
        x_in = x_in | 0xF000;
    }
    if (y_in & 0x800) {
        y_in = y_in | 0xF000;
    }

    /* Gen Report */
    out[0] = BT_HID_REPORT_ID_NVMS;         /* Report ID */
    out[1] = in[1];                         /* Keys */
    out[2] = 0;                             /* Reserved (0x00) */
    out[3] = x_in&0x00FF;                   /* X LSB */
    out[4] = (x_in>>8) & 0x00FF;              /* X HSB */
    out[5] = y_in&0x00FF;                   /* Y LSB */
    out[6] = (y_in>>8) & 0x00FF;              /* Y HSB */
    out[7] = in[5];                         /* Wheel */
    out[8]  = 0x03;                         /* Nvidia Latency - Extension ID */
    out[9]  = 0x00;                         /* Nvidia Latency LSB */
    out[10] = 0x00;                         /* Nvidia Latency MSB */
    out[11] = 0;                            /* Reserved (0x00) */
    out[12] = 0;                            /* Reserved (0x00) */
    *outlen = HID_NVIDIA_DATA_LEN;
}

ATTR_TEXT_IN_TCM static void app_usb_utils_hid_report_to_nvms_boot(uint8_t *in, uint8_t *out, uint16_t inlen, uint16_t *outlen)
{
    out[0] = in[1];

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

    /* Byte 3~7 are reserved */
    out[3] = 0;
    out[4] = 0;
    out[5] = 0;
    out[6] = 0;
    out[7] = 0;
    *outlen = HID_BIOS_DATA_LEN;
}

static volatile uint32_t s_nv_time_btrx = 0;
extern uint32_t pka_return_hdl_intr_time(void);
ATTR_TEXT_IN_TCM static void app_usb_utils_latency_report_to_nvms(uint8_t *in, uint8_t *out, uint16_t inlen, uint16_t *outlen)
{
    bt_nvms_packet_t *packet = (bt_nvms_packet_t *)in;

    LOG_MSGID_I(apps, "app_usb_utils_latency_report_to_nvms report_id:%d wheel:%d key:%d latency_h:%d x:%d y:%d latency_l:%d", 7,
        packet->report_id, packet->wheel, packet->key, packet->latency_h, packet->x, packet->y, packet->latency_l);

    const uint32_t nv_lat_const = 0;
    uint32_t nv_lat_errmsk = 0;

    // BT rx iqr start time
    LOG_MSGID_I(apps, "app_usb_utils_latency_report_to_nvms s_nv_time_btrx:%d", 1, s_nv_time_btrx);

    // Calculate latency
    uint32_t nv_lat_device = (packet->latency_h << 8) | packet->latency_l;
    if (nv_lat_device == 0x07FF) {
        nv_lat_errmsk = 0xF000;
    }
    nv_lat_device = nv_lat_device * 10;

    uint32_t nv_lat_dongle = 0;
    if (s_nv_time_usbtx > s_nv_time_btrx) {
        nv_lat_dongle = s_nv_time_usbtx - s_nv_time_btrx;
    } else {
        nv_lat_dongle = s_nv_time_btrx - s_nv_time_usbtx + 1;
    }
    LOG_MSGID_I(apps, "app_usb_utils_latency_report_to_nvms s_nv_time_usbtx:%d nv_time_btrx:%d", 2, s_nv_time_usbtx, s_nv_time_btrx);
    
    uint32_t nv_lat_total  = nv_lat_device + nv_lat_dongle + nv_lat_const;
    uint32_t nv_lat_encode = (nv_lat_total / 10) | nv_lat_errmsk;
    LOG_MSGID_I(apps, "app_usb_utils_latency_report_to_nvms nv_lat_device:%d nv_lat_dongle:%d nv_lat_total:%d nv_lat_encode:%d", 4, 
        nv_lat_device, nv_lat_dongle, nv_lat_total, nv_lat_encode);

    /* Gen Report */
    out[0] = packet->report_id;             /* Report ID */
    out[1] = packet->key;                   /* Keys */
    out[2] = 0x00;                          /* Reserved (0x00) */
    out[3] = 0x00;                          /* X LSB */
    out[4] = 0x00;                          /* X HSB */
    out[5] = 0x00;                          /* Y LSB */
    out[6] = 0x00;                          /* Y HSB */
    out[7] = 0x00;                          /* Wheel */
    out[8]  = 0x03;                         /* Nvidia Latency - Extension ID */
    out[9]  = nv_lat_encode & 0xFF;         /* Nvidia Latency LSB */
    out[10] = (nv_lat_encode >> 8);         /* Nvidia Latency MSB */
    out[11] = 0;                            /* Reserved (0x00) */
    out[12] = 0;                            /* Reserved (0x00) */
    *outlen = HID_NVIDIA_DATA_LEN;
}

ATTR_TEXT_IN_TCM static void app_usb_utils_latency_report_to_nvms_boot(uint8_t *in, uint8_t *out, uint16_t inlen, uint16_t *outlen)
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
    *outlen = HID_BIOS_DATA_LEN;
}

void app_usb_utils_preintr_callback(uint8_t port)
{
    app_usb_utils_nvms_set_timer_usb_tx_done();
}

void app_usb_utils_hid_preintr_reg(void)
{
    uint8_t port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_GMOUSE_NV);
    if (port != USB_HID_MAX_DEVICE_NUM) {
        usb_hid_pretx_intr_register(port, app_usb_utils_preintr_callback);
    }
}
#endif /* AIR_NVIDIA_REFLEX_ENABLE */


#ifdef AIR_PURE_GAMING_MS_ENABLE
ATTR_ZIDATA_IN_TCM uint8_t bt_data[HID_NORMAL_DATA_LEN] = {0};
ATTR_ZIDATA_IN_TCM volatile uint16_t bt_len = 0;
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
    if (bt_plc_param.need_plc || bt_plc_param.channel == 1) {
        hal_nvic_set_pending_irq(WDT_IRQn);
    } else {
        app_usb_utils_bt_callback(&bt_data[0], bt_len, &bt_plc_param);
    }
    // hal_gpio_set_output(USB_DEBUG_GPIO_1, HAL_GPIO_DATA_LOW);
}
#endif

ATTR_TEXT_IN_TCM static bool app_usb_utils_filter_repeat_zero_pkt(uint8_t* data, uint16_t len)
{
    static volatile uint8_t g_ms_normal_zero_pkt_cnt = 0;
    static volatile uint8_t g_ms_bios_zero_pkt_cnt = 0;
#if defined(AIR_PURE_GAMING_MS_ENABLE)
    static volatile uint8_t g_ms_key_only_zero_pkt_cnt = 0;
#endif

    if (hal_usb_get_suspend_state() == HAL_USB_POWER_STATE_SUSPEND) {
        /* if usb suspend, clear zero pkt cnt */
        g_ms_normal_zero_pkt_cnt = 0;
        g_ms_bios_zero_pkt_cnt = 0;
#if defined(AIR_PURE_GAMING_MS_ENABLE)
        g_ms_key_only_zero_pkt_cnt = 0;
#endif
    }

    if (len == HID_NORMAL_DATA_LEN) { // normal mode
        g_ms_bios_zero_pkt_cnt = 0;
        if (data[0] == BT_HID_REPORT_ID_MS) {
            if (data[1] != 0 && data[2] == 0 && (*(volatile uint32_t *)(data + 3) == 0)) {        
#if defined(AIR_PURE_GAMING_MS_ENABLE)
                if (data[1] == g_key_output) {
                    g_ms_normal_zero_pkt_cnt = 0;
                    if(g_ms_key_only_zero_pkt_cnt <= 1) {
                        g_ms_key_only_zero_pkt_cnt++;
                    }
                    if (g_ms_key_only_zero_pkt_cnt <= 1) {
                        //LOG_MSGID_I(USBHID, "########## app_usb_utils_bt_callback key only pkt: %d", 1, g_key_output);
                        return true;
                    }
                }
#else
                g_ms_normal_zero_pkt_cnt = 0;
                return true;
#endif
            } else if ((((*(volatile uint32_t *)data) & 0xffffff00) == 0)
                && (*(volatile uint32_t *)(data + 3) == 0)) {
#if defined(AIR_PURE_GAMING_MS_ENABLE)
                g_ms_key_only_zero_pkt_cnt = 0;
#endif
                if(g_ms_normal_zero_pkt_cnt <= 1) {
                    g_ms_normal_zero_pkt_cnt++;
                }
                if (g_ms_normal_zero_pkt_cnt <= 1) {
                    //LOG_MSGID_I(USBHID, "########## app_usb_utils_bt_callback zero pkt", 0);
                    return true;
                }
            } else {
                g_ms_normal_zero_pkt_cnt = 0;
#if defined(AIR_PURE_GAMING_MS_ENABLE)
                g_ms_key_only_zero_pkt_cnt = 0;
#endif
                return true;
            }
        } else if (data[0] == BT_HID_REPORT_ID_KB) {
            g_ms_normal_zero_pkt_cnt = 0;
#if defined(AIR_PURE_GAMING_MS_ENABLE)
            g_ms_key_only_zero_pkt_cnt = 0;
#endif
            return true;
        }
    } else if (len == HID_BIOS_DATA_LEN) {  //BIOS mode
        g_ms_normal_zero_pkt_cnt = 0;
#if defined(AIR_PURE_GAMING_MS_ENABLE)
        g_ms_key_only_zero_pkt_cnt = 0;
#endif
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
    } else if (len == HID_NKEY_PKT_DATA_LEN || len == HID_CONSUMER_DATA_LEN) {
        return true;
    }
#if defined(AIR_NVIDIA_REFLEX_ENABLE)
    else if (len == HID_NVIDIA_DATA_LEN) {
        return true;
    }
#endif
    return false;
}

ATTR_TEXT_IN_TCM void app_usb_utils_send_data(uint8_t port, uint8_t* data, uint16_t len)
{
#if defined(AIR_PURE_GAMING_MS_ENABLE)
    static volatile bool g_need_to_acc = false;
    static volatile int16_t g_acc_x = 0;
    static volatile int16_t g_acc_y = 0;

    int16_t x_in = ((data[3] & 0x0F) << 8) | data[2];
    int16_t y_in = ((data[4] << 4)) | (data[3] >> 4);
    /* int12 -> int16 */
    if (x_in & 0x800) {
        x_in = x_in | 0xF000;
    }
    if (y_in & 0x800) {
        y_in = y_in | 0xF000;
    }

    int16_t x_out = x_in;
    int16_t y_out = y_in;
    if (true == g_need_to_acc) {
        if (g_acc_x != 0 || g_acc_y != 0) {
            x_out += g_acc_x;
            y_out += g_acc_y;
            
            data[2] = x_out & 0xFF;
            data[3] = ((x_out >> 8) & 0x0F) | ((y_out & 0x0F) << 4);
            data[4] = (y_out >> 4);
        }
        g_need_to_acc = false;
    } else {
        g_acc_x = 0;
        g_acc_y = 0;
    }

    if (app_usb_utils_filter_repeat_zero_pkt(data, len)) {
#if defined(AIR_NVIDIA_REFLEX_ENABLE)
        // Note: convert MS hid data to nvidia
        if (len == HID_NORMAL_DATA_LEN && data[0] == BT_HID_REPORT_ID_MS) { // Normal mode
            uint8_t *nv_input = data;
            uint16_t nv_input_len = len;

            ATTR_ALIGN(4) uint8_t nv_output_data[20] = {0};
            uint16_t nv_output_len = 0;
            uint8_t nv_port = app_usb_utils_hid_report_convert_nvidia(nv_input, nv_input_len, nv_output_data, &nv_output_len);
            #ifdef HID_DATA_DEBUG
            LOG_HEXDUMP_I(USBHID, "app_usb_utils_send_data out1: port(%d)", nv_output_data, nv_output_len, nv_port);
            #endif
            uint8_t tx_status = usb_hid_tx_non_blocking(nv_port, nv_output_data, nv_output_len);
            if (tx_status != USB_HID_STATUS_OK) {
                LOG_MSGID_E(USBHID, "app_usb_utils_send_data fail:%d", 1, tx_status);
            }
        } else {
            #ifdef HID_DATA_DEBUG
            LOG_HEXDUMP_I(USBHID, "app_usb_utils_send_data out2: port(%d)", data, len, port);
            #endif
            uint8_t tx_status = usb_hid_tx_non_blocking(port, data, len);
            if (tx_status != USB_HID_STATUS_OK) {
                LOG_MSGID_E(USBHID, "app_usb_utils_send_data fail:%d", 1, tx_status); 
            }
        }
#else
        uint8_t tx_status = usb_hid_tx_non_blocking(port, data, len);
#ifdef HID_DATA_DEBUG
        LOG_HEXDUMP_I(USBHID, "app_usb_utils_send_data out3: ", data, len);
#endif
        if (USB_HID_TX_IS_BUSY == tx_status) {
            g_acc_x = x_out;
            g_acc_y = y_out;
            g_need_to_acc = true;
        }
#endif
    }
#else
    if (app_usb_utils_filter_repeat_zero_pkt(data, len)) {
        uint8_t tx_status = usb_hid_tx_non_blocking(port, data, len);
#ifdef HID_DATA_DEBUG
        LOG_HEXDUMP_I(USBHID, "app_usb_utils_send_data out4: ", data, len);
#endif
        if (tx_status != USB_HID_STATUS_OK) {
            static uint32_t s_drop_cnt = 0;
            if(s_drop_cnt % 400 == 0) {
                LOG_MSGID_E(USBHID, "app_usb_utils_send_data droped a packet! cnt:%d", 1, s_drop_cnt + 1);
            }
#if defined(AIR_PURE_GAMING_KB_ENABLE)
            s_tx_faild = true;
#endif/*AIR_PURE_GAMING_KB_ENABLE*/
            s_drop_cnt++;
        }
    }
#endif
}

#if defined(AIR_PURE_GAMING_MS_ENABLE)
ATTR_TEXT_IN_TCM uint8_t app_usb_utils_plc_add(hid_plc_param_t *para, uint8_t* data_out, uint16_t *len_out)
{
    para->x = 0;
    para->y = 0;

    int16_t nx = 0;
    int16_t ny = 0;
    app_hid_plc_handler(para, &nx, &ny);
    
    data_out[0] = BT_HID_REPORT_ID_MS;
    data_out[1] = g_key_output;
    data_out[2] = nx & 0xFF;
    data_out[3] = ((nx >> 8) & 0x0F) | ((ny & 0x0F) << 4);
    data_out[4] = (ny >> 4);
    *len_out = HID_NORMAL_DATA_LEN;
    
    /* In Boot Mode */
    uint8_t port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_GMOUSE);
    if (usb_hid_is_boot_mode(port) == USB_HID_PROTOCOL_BOOT) {
        app_usb_utils_report_convert_ms_to_boot(data_out, data_out, HID_NORMAL_DATA_LEN, len_out);
    }
    return port;
}

ATTR_TEXT_IN_TCM uint8_t app_usb_utils_plc_correct(hid_plc_param_t *para, uint8_t* data_in, uint8_t len_in, uint8_t* data_out, uint16_t *len_out)
{
    *len_out = len_in;

    if (para->reason == 0) { // normal packet
        g_key_output = data_in[1];
    } else if (para->reason == 0x4) { // in case of mouse can't move case(repeat zero pkt)
        app_hid_plc_clear();
        return USB_HID_MAX_DEVICE_NUM;
    } else {
        data_in[1] = g_key_output;
    }

    int16_t x = ((data_in[3] & 0x0F) << 8) | data_in[2];
    int16_t y = ((data_in[4] << 4)) | (data_in[3] >> 4);
    /* int12 -> int16 */
    if (x & 0x800) {
        x = x | 0xF000;
    }
    if (y & 0x800) {
        y = y | 0xF000;
    }
    para->x = x;
    para->y = y;

    int16_t mx = 0;
    int16_t my = 0;
    app_hid_plc_handler(para, &mx, &my);

    data_in[2] = mx & 0xFF;
    data_in[3] = ((mx >> 8) & 0x0F) | ((my & 0x0F) << 4);
    data_in[4] = (my >> 4);
    
    memcpy(data_out, data_in, *len_out);

    /* In Boot Mode */
    uint8_t port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_GMOUSE);
    if (usb_hid_is_boot_mode(port) == USB_HID_PROTOCOL_BOOT) {
        app_usb_utils_report_convert_ms_to_boot(data_in, data_out, len_in, len_out);
    }
    return port;
}
#endif

ATTR_TEXT_IN_TCM uint8_t app_usb_utils_ms_convert(uint8_t* data_in, uint8_t len_in, uint8_t* data_out, uint16_t *len_out)
{
    *len_out = len_in;
    memcpy(data_out, data_in, *len_out);

    /* In Boot Mode */
    uint8_t port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_GMOUSE);
    if (usb_hid_is_boot_mode(port) == USB_HID_PROTOCOL_BOOT) {
        app_usb_utils_report_convert_ms_to_boot(data_in, data_out, len_in, len_out);
    }
    return port;
}

ATTR_TEXT_IN_TCM uint8_t app_usb_utils_kb_convert(uint8_t* data_in, uint8_t len_in, uint8_t* data_out, uint16_t *len_out)
{
    *len_out = len_in;
    memcpy(data_out, data_in, *len_out);

    /* In Boot Mode */
    uint8_t port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_GKEYBOARD);
    if (usb_hid_is_boot_mode(port) == USB_HID_PROTOCOL_BOOT) {
        app_usb_utils_report_convert_kb_to_boot(data_in, data_out, len_in, len_out);
    }
    return port;
}

ATTR_TEXT_IN_TCM uint8_t app_usb_utils_consumer_convert(uint8_t* data_in, uint8_t len_in, uint8_t* data_out, uint16_t *len_out)
{
    *len_out = HID_CONSUMER_DATA_LEN;
#ifdef KEY_MISSING_DEBUG
    memcpy(data_out, data_in, 4);
#else
    memcpy(data_out, data_in, 6);
#endif
    /* In Boot Mode */
    uint8_t port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_GKEYBOARD);
    if (usb_hid_is_boot_mode(port) == USB_HID_PROTOCOL_BOOT) {
        /* Do nothing */
        memset(data_out, 0, *len_out);
        *len_out = 0;
    }
    return port;
}

ATTR_TEXT_IN_TCM uint8_t app_usb_utils_nkey_convert(uint8_t report_id, uint8_t* data_in, uint8_t len_in, uint8_t* data_out, uint16_t *len_out)
{
    *len_out = HID_NKEY_PKT_DATA_LEN;

    // static kb_nkey_packet_t s_kb_n_key_report_data;
#ifdef KEY_MISSING_DEBUG
    s_kb_n_key_report_data.report_id = BT_HID_REPORT_ID_KB;
    if (report_id == BT_HID_REPORT_ID_KB_NKEY_GROUP1) {
        memcpy(&s_kb_n_key_report_data.modifier_key, &data_in[1], 4);
    } else if (report_id == BT_HID_REPORT_ID_KB_NKEY_GROUP2) {
        memcpy(&(s_kb_n_key_report_data.key[5]), &data_in[1], 3);
    } else if (report_id == BT_HID_REPORT_ID_KB_NKEY_GROUP3) {
        memcpy(&(s_kb_n_key_report_data.key[10]), &data_in[1], 3);
    }
#else
    s_kb_n_key_report_data.report_id = BT_HID_REPORT_ID_KB;
    if (report_id == BT_HID_REPORT_ID_KB_NKEY_GROUP1) {
        memcpy(&s_kb_n_key_report_data.modifier_key, &data_in[1], 6);
    } else if (report_id == BT_HID_REPORT_ID_KB_NKEY_GROUP2) {
        memcpy(&(s_kb_n_key_report_data.key[5]), &data_in[1], 5);
    } else if (report_id == BT_HID_REPORT_ID_KB_NKEY_GROUP3) {
        memcpy(&(s_kb_n_key_report_data.key[10]), &data_in[1], 5);
    }
#endif
    memcpy(data_out, &s_kb_n_key_report_data, *len_out);
    
    /* In Boot Mode */
    uint8_t port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_GKEYBOARD);
    if (usb_hid_is_boot_mode(port) == USB_HID_PROTOCOL_BOOT) {
        app_usb_utils_report_convert_kb_nkey_to_boot((uint8_t *)&s_kb_n_key_report_data, data_out, HID_NKEY_PKT_DATA_LEN, len_out);
    }
    return port;
}

#if defined(AIR_NVIDIA_REFLEX_ENABLE)
ATTR_TEXT_IN_TCM uint8_t app_usb_utils_hid_report_convert_nvidia(uint8_t* data_in, uint8_t len_in, uint8_t* data_out, uint16_t *len_out)
{
    uint8_t port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_GMOUSE_NV);
    if (usb_hid_is_boot_mode(port) == USB_HID_PROTOCOL_BOOT) {      /* In Boot Mode */
        app_usb_utils_hid_report_to_nvms_boot(data_in, data_out, len_in, len_out);
    } else {    /* In Normal Mode */
        app_usb_utils_hid_report_to_nvms(data_in, data_out, len_in, len_out);
    }
    return port;
}

ATTR_TEXT_IN_TCM uint8_t app_usb_utils_latency_report_convert_nvidia(uint8_t* data_in, uint8_t len_in, uint8_t* data_out, uint16_t *len_out)
{
    uint8_t port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_GMOUSE_NV);
    if (usb_hid_is_boot_mode(port) == USB_HID_PROTOCOL_BOOT) {      /* In Boot Mode */
        app_usb_utils_latency_report_to_nvms_boot(data_in, data_out, len_in, len_out);
    } else {    /* In Normal Mode */
        app_usb_utils_latency_report_to_nvms(data_in, data_out, len_in, len_out);
    }
    return port;
}
#endif

#if defined(AIR_PURE_GAMING_MS_ENABLE)
ATTR_TEXT_IN_TCM bool app_usb_utils_send_handle_wbscan(hid_plc_param_t *para, uint8_t port, uint8_t* data, uint16_t len)
{
    static volatile bool s_wbscan_add_pkt = false;
    if (para->reason == 0x05 && para->flag == true && s_wbscan_add_pkt == false) {
        app_usb_utils_send_data(port, data, len);
        // GPT delay 250us, modify reason to 1, revert channel
        hal_gpt_delay_us(250);
        para->reason = 0x01;
        para->channel= !para->channel;
        s_wbscan_add_pkt = true;
        return true;
    } else if (para->reason == 0x01 && para->flag == true && s_wbscan_add_pkt == true) {
        app_usb_utils_send_data(port, data, len);
        s_wbscan_add_pkt = false;
    } else if (para->reason == 0x06 && para->flag == false && s_wbscan_add_pkt == false) {
        app_usb_utils_send_data(port, data, len);
        // GPT delay 250us, modify reason to 1, need plc to true, revert channel
        hal_gpt_delay_us(250);
        para->reason = 0x01;
        para->channel = !para->channel;
        para->flag = true;
        s_wbscan_add_pkt = true;
        return true;
    } else {
        s_wbscan_add_pkt = false;
        app_usb_utils_send_data(port, data, len);
    }
    return false;
}
#endif

ATTR_TEXT_IN_TCM static void app_usb_utils_bt_callback(uint8_t* data, uint16_t len, usb_plc_cb_param *param)
{
    uint8_t report_id = 0;
    uint8_t *pdata = NULL;
    uint16_t datalen = 0;
    uint8_t port = USB_HID_MAX_DEVICE_NUM;
    ATTR_ALIGN(4) uint8_t cvtdata[20] = {0};

#ifdef HID_DATA_DEBUG
    if(data != NULL && len != 0) {
        LOG_HEXDUMP_I(USBHID, "app_usb_utils_send_data in: need_plc(%d), reason(%d)", data, len, param->need_plc, param->reason);
    }
#endif

    if(len > USB_HID_EP_MAX_SIZE) {
        assert(0 && "data is exceed USB_HID_EP_MAX_SIZE.");
    }

    if (len > 0) {
        report_id = data[0] & 0x1F;
        pdata = data;
        datalen = len;
    }

#if defined (AIR_PURE_GAMING_MS_KB_ENABLE) || !defined (AIR_PURE_GAMING_MS_ENABLE)
    bool plc_flag = param->need_plc;
    if (plc_flag || param->reason) {
        //LOG_MSGID_I(USBHID, "plc_flag ture, return", 0);
        return;
    }
#endif

#if defined(AIR_PURE_GAMING_MS_ENABLE)
    hid_plc_param_t plc_para;
    plc_para.flag = param->need_plc;
    plc_para.revised = param->revised;
    plc_para.reason = param->reason;
    plc_para.channel = param->channel;

    // key remap flow
    if ((report_id == BT_HID_REPORT_ID_MS) && (datalen == HID_NORMAL_DATA_LEN)) {
        app_key_remap_key_status_update(pdata, datalen, plc_para.reason);
    }

    /* plc reason is not zero(not normal pkt) as well as RR is 125 or 250, need to filter this pkt*/
    plc_para.scenario = app_dongle_ull_le_get_scenario_from_ctx();
    if (plc_para.reason != 0x0 && (plc_para.scenario == BT_ULL_LE_HID_SRV_APP_SCENARIO_8 || plc_para.scenario == BT_ULL_LE_HID_SRV_APP_SCENARIO_9)) {
        return;
    }

_RESEND_HID_PKT:
    if (plc_para.flag) {
        port = app_usb_utils_plc_add(&plc_para, cvtdata, &datalen);
        pdata = cvtdata;
        goto _DATA_TO_USB;
    }
#endif

#if defined (AIR_NVIDIA_REFLEX_ENABLE)
    if (report_id == BT_HID_REPORT_ID_MS) {
        if (param->need_plc == false && param->reason == 0) {
            s_nv_time_btrx = pka_return_hdl_intr_time();
        }
    }
#endif

    switch (report_id) {
        case BT_HID_REPORT_ID_MS:
#if defined(AIR_PURE_GAMING_MS_ENABLE)
            port = app_usb_utils_plc_correct(&plc_para, data, len, cvtdata, &datalen);
#else
            port = app_usb_utils_ms_convert(data, len, cvtdata, &datalen);
#endif 
            pdata = cvtdata;
            break;
        case BT_HID_REPORT_ID_KB:
            port = app_usb_utils_kb_convert(data, len, cvtdata, &datalen);
            pdata = cvtdata;
            break;
        case BT_HID_REPORT_ID_CONSUMER:
            port = app_usb_utils_consumer_convert(data, len, cvtdata, &datalen);
            pdata = cvtdata;
            break;
        case BT_HID_REPORT_ID_KB_NKEY_GROUP1:
        case BT_HID_REPORT_ID_KB_NKEY_GROUP2:
        case BT_HID_REPORT_ID_KB_NKEY_GROUP3:
            port = app_usb_utils_nkey_convert(report_id, data, len, cvtdata, &datalen);
            pdata = cvtdata;
            break;
#if defined(AIR_NVIDIA_REFLEX_ENABLE)
        case BT_HID_REPORT_ID_NVMS:
            port = app_usb_utils_latency_report_convert_nvidia(data, len, cvtdata, &datalen);
            pdata = cvtdata;
            break;
#endif
        default:
            return;
    }

#if defined(AIR_PURE_GAMING_MS_ENABLE)
_DATA_TO_USB:
    if (app_usb_utils_send_handle_wbscan(&plc_para, port, pdata, datalen)) {
        datalen = 0;
        goto _RESEND_HID_PKT;
    }
#else
    app_usb_utils_send_data(port, pdata, datalen);
#endif
}

void app_usb_utils_hid_ms_release_pkt()
{
    uint8_t data_in[HID_NORMAL_DATA_LEN] = { BT_HID_REPORT_ID_MS, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    ATTR_ALIGN(4) uint8_t data_out[20] = {0};
    uint16_t data_out_len = 0;

    data_out_len = HID_NORMAL_DATA_LEN;
    memcpy(data_out, data_in, HID_NORMAL_DATA_LEN);

    /* In Boot Mode */
    uint8_t port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_GMOUSE);
    if (usb_hid_is_boot_mode(port) == USB_HID_PROTOCOL_BOOT) {
        app_usb_utils_report_convert_ms_to_boot(data_in, data_out, HID_NORMAL_DATA_LEN, &data_out_len);
    }
    
    uint8_t tx_status = usb_hid_tx_non_blocking(port, data_out, data_out_len);
    if (tx_status != USB_HID_STATUS_OK) {
        LOG_MSGID_E(USBHID, "app_usb_utils_hid_ms_release_pkt fail:%d", 1, tx_status);
    }
}

void app_usb_utils_hid_kb_release_pkt()
{
    uint8_t data_in[HID_NORMAL_DATA_LEN] = { BT_HID_REPORT_ID_KB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    ATTR_ALIGN(4) uint8_t data_out[20] = {0};
    uint16_t data_out_len = 0;

    data_out_len = HID_NORMAL_DATA_LEN;
    memcpy(data_out, data_in, HID_NORMAL_DATA_LEN);

    /* In Boot Mode */
    uint8_t port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_GKEYBOARD);
    if (usb_hid_is_boot_mode(port) == USB_HID_PROTOCOL_BOOT) {
        app_usb_utils_report_convert_kb_to_boot(data_in, data_out, HID_NORMAL_DATA_LEN, &data_out_len);
    }
    
    uint8_t tx_status = usb_hid_tx_non_blocking(port, data_out, data_out_len);
    if (tx_status != USB_HID_STATUS_OK) {
        LOG_MSGID_E(USBHID, "app_usb_utils_hid_kb_release_pkt fail:%d", 1, tx_status);
    }
}

void app_usb_utils_hid_nkey_release_pkt()
{
    uint8_t data_in[HID_NKEY_PKT_DATA_LEN] = { BT_HID_REPORT_ID_KB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    ATTR_ALIGN(4) uint8_t data_out[20] = {0};
    uint16_t data_out_len = 0;

    data_out_len = HID_NKEY_PKT_DATA_LEN;
    memcpy(data_out, data_in, HID_NKEY_PKT_DATA_LEN);

    /* In Boot Mode */
    uint8_t port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_GKEYBOARD);
    if (usb_hid_is_boot_mode(port) == USB_HID_PROTOCOL_BOOT) {
        app_usb_utils_report_convert_kb_nkey_to_boot(data_in, data_out, HID_NKEY_PKT_DATA_LEN, &data_out_len);
    }
    
    uint8_t tx_status = usb_hid_tx_non_blocking(port, data_out, data_out_len);
    if (tx_status != USB_HID_STATUS_OK) {
        LOG_MSGID_E(USBHID, "app_usb_utils_hid_nkey_release_pkt fail:%d", 1, tx_status);
    }
}

void app_usb_utils_hid_bt_reg(void)
{
#ifdef AIR_PURE_GAMING_MS_ENABLE
    hal_nvic_disable_irq(WDT_IRQn);
    hal_nvic_set_priority(WDT_IRQn, (DEFAULT_IRQ_PRIORITY + 1));
    hal_nvic_register_isr_handler(WDT_IRQn, (hal_nvic_isr_t)usb_hid_bt_callback_in_sw_isr);
    hal_nvic_enable_irq(WDT_IRQn);

    extern void pka_hid_data_forward_callback_register(void* fcallback);
    pka_hid_data_forward_callback_register((void *)usb_hid_bt_callback_in_bt_isr);
#else
    extern void pka_hid_data_forward_callback_register(void* fcallback);
    pka_hid_data_forward_callback_register((void *)app_usb_utils_bt_callback);
#endif
}

#if defined(AIR_PURE_GAMING_KB_ENABLE)

#define USB_HID_OUTPUT_DATA_LENGTH (2)
static uint8_t s_hid_output_data_cache[USB_HID_OUTPUT_DATA_LENGTH] = {0};

void app_usb_utils_hid_output_cb(uint8_t *data, uint32_t data_length)
{
    //LOG_MSGID_I(USBHID, "app_usb_utils_hid_output_cb data_length:%d, data[0]:%d, data[1]:%d", 3, data_length, data[0], data[1]);
    if(!data || data[0] != BT_HID_REPORT_ID_KB){
        return;
    }

    // Send payload to DUT, exclude report id
    bt_ull_le_hid_srv_action(BT_ULL_ACTION_LE_HID_CONTROL_RGB, (const void *)(data + 1), data_length-1);

    // Save newest hid output data, send it when next connection
    memcpy(s_hid_output_data_cache, data, USB_HID_OUTPUT_DATA_LENGTH);
}
void app_usb_utils_hid_output_cache_send()
{
    bt_ull_le_hid_srv_action(BT_ULL_ACTION_LE_HID_CONTROL_RGB, (const void *)&s_hid_output_data_cache[1], USB_HID_OUTPUT_DATA_LENGTH-1);
}

void app_usb_utils_hid_output_reg(void)
{
    usb_hid_handler_rx_register(BT_HID_REPORT_ID_KB, USB_HID_OUTPUT_DATA_LENGTH, app_usb_utils_hid_output_cb);
}

void usb_hid_tx_faild_handler_callback()
{
    if(s_tx_faild)
    {
        uint8_t port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_GKEYBOARD);
        uint8_t tx_status;
        tx_status= usb_hid_tx_non_blocking(port, (uint8_t *)(&s_kb_n_key_report_data), sizeof(kb_nkey_packet_t));
        if(tx_status == USB_HID_STATUS_OK)
        {
            s_tx_faild = false;
        }
    }
}

void app_usb_utils_hid_kb_intr_reg(void)
{
    uint8_t port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_GKEYBOARD);
    if (port != USB_HID_MAX_DEVICE_NUM) {
        usb_hid_tx_intr_register(port, (void *)usb_hid_tx_faild_handler_callback);
    }
}
#endif/*AIR_PURE_GAMING_KB_ENABLE*/

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


/**
 * @file usb_dbg.h
 * @brief Dispatch log api to different implement.
 */

#ifndef USB_DBG_H
#define USB_DBG_H

#ifdef AIR_USB_ENABLE

#include <stdint.h>

#ifdef __EXT_BOOTLOADER__

/**
 * Open USB_BL_DEBUG to enable debug pring in bootloader
 * In bootloader, bl_print provide less info than syslog.
 * Close it in default.
 */
// #define USB_BL_DEBUG

#include "bl_common.h"
#define log_create_module(module, level)
#define log_create_module_variant(module, on_off, level)

#if defined(MTK_DEBUG_LEVEL_INFO) && defined(USB_BL_DEBUG)
#define LOG_I(module, message, ...) bl_print(LOG_DEBUG, message "\r\n", ##__VA_ARGS__)
#else
#define LOG_I(module, message, ...)
#endif

#if defined(MTK_DEBUG_LEVEL_DEBUG) && defined(USB_BL_DEBUG)
#define LOG_MSGID_D(module, message, arg_cnt, ...) bl_print(LOG_DEBUG, message "\r\n", ##__VA_ARGS__)
#else
#define LOG_MSGID_D(module, message, arg_cnt, ...)
#endif

#if defined(MTK_DEBUG_LEVEL_INFO) && defined(USB_BL_DEBUG)
#define LOG_MSGID_I(module, message, arg_cnt, ...) bl_print(LOG_DEBUG, message "\r\n", ##__VA_ARGS__)
#else
#define LOG_MSGID_I(module, message, arg_cnt, ...)
#endif

#if defined(MTK_DEBUG_LEVEL_WARNING) && defined(USB_BL_DEBUG)
#define LOG_MSGID_W(module, message, arg_cnt, ...) bl_print(LOG_DEBUG, message "\r\n", ##__VA_ARGS__)
#else
#define LOG_MSGID_W(module, message, arg_cnt, ...)
#endif

#if defined(MTK_DEBUG_LEVEL_ERROR) && defined(USB_BL_DEBUG)
#define LOG_MSGID_E(module, message, arg_cnt, ...) bl_print(LOG_DEBUG, message "\r\n", ##__VA_ARGS__)
#else
#define LOG_MSGID_E(module, message, arg_cnt, ...)
#endif

#if defined(MTK_DEBUG_LEVEL_ERROR) && defined(USB_BL_DEBUG)
#define LOG_HEXDUMP_I(module, message, data, len, ...) bl_print(LOG_DEBUG, message "\r\n", ##__VA_ARGS__)
#else
#define LOG_HEXDUMP_I(module, message, data, len, ...)
#endif

#else
#include "syslog.h"

#endif /* __EXT_BOOTLOADER__ */

#define USB_DBG

typedef enum {
    USB_DBG_EP0_CMD          = (1<<0),
    USB_DBG_EP0_STATE_TX     = (1<<1),
    USB_DBG_EP0_STATE_RX     = (1<<2),
    USB_DBG_EP0_STATE_HDRL   = (1<<3),
    USB_DBG_EP0_STATE_IDLE   = (1<<4),
    USB_DBG_EP0_DATALEN_WAIT = (1<<5),
    USB_DBG_EP0_RSVD_6       = (1<<6),
    USB_DBG_EP0_RSVD_7       = (1<<7),

    USB_DBG_AUDIO_VOLUME     = (1<<8),
    USB_DBG_AUDIO_ISO_OUT    = (1<<9),
    USB_DBG_AUDIO_TX         = (1<<10),
    USB_DBG_AUDIO_ISO_IN     = (1<<11),
    USB_DBG_AUDIO_MISC       = (1<<12),
    USB_DBG_AUDIO_GET_INFO   = (1<<13),
    USB_DBG_AUDIO_RSVD_14    = (1<<14),
    USB_DBG_AUDIO_RSVD_15    = (1<<15),

    USB_DBG_HID_MUX          = (1<<16),
    USB_DBG_HID_READDATA     = (1<<17),
    USB_DBG_HID_SET_REPORT   = (1<<18),
    USB_DBG_HID_GET_REPORT   = (1<<19),
    USB_DBG_HID_RSVD_20      = (1<<20),
    USB_DBG_HID_RSVD_21      = (1<<21),
    USB_DBG_HID_RSVD_22      = (1<<22),
    USB_DBG_HID_RSVD_23      = (1<<23),

    // misc
    USB_DBG_XBOX_TX          = (1<<24),
    USB_DBG_XBOX_ISO_OUT     = (1<<25),
    USB_DBG_XBOX_ISO_IN      = (1<<26),
    USB_DBG_XBOX_MISC        = (1<<27),
    USB_DBG_RSVD_28          = (1<<28),
    USB_DBG_RSVD_29          = (1<<29),
    USB_DBG_RSVD_30          = (1<<30),
    USB_DBG_RSVD_31          = (1<<31),

} USB_DBG_LOG_t;


typedef enum {
    /* for common */
    USB_DBG_OPT_COMMON_GPIO             = 0,
    USB_DBG_OPT_RSVD_1                  = 1,
    USB_DBG_OPT_RSVD_2                  = 2,
    USB_DBG_OPT_RSVD_3                  = 3,
    USB_DBG_OPT_RSVD_4                  = 4,
    USB_DBG_OPT_RSVD_5                  = 5,
    USB_DBG_OPT_RSVD_6                  = 6,
    USB_DBG_OPT_RSVD_7                  = 7,

    /* for audio */
    USB_DBG_OPT_AUDIO_DUMP_RX1          = 8,
    USB_DBG_OPT_AUDIO_DUMP_TX1          = 9,
    USB_DBG_OPT_AUDIO_DUMP_RX2          = 10,
    USB_DBG_OPT_AUDIO_DUMP_TX2          = 11, /* Unused */
    USB_DBG_OPT_AUDIO_RX_TIME_MEASURE   = 12,
    USB_DBG_OPT_AUDIO_TX_TIME_MEASURE   = 13,
    USB_DBG_OPT_AUDIO_RX_TIME_ASSERT    = 14,
    USB_DBG_OPT_AUDIO_TX_TIME_ASSERT    = 15,

    /* for hid */
    USB_DBG_OPT_HID_GPIO                = 16,
    USB_DBG_OPT_RSVD_17                 = 17,
    USB_DBG_OPT_RSVD_18                 = 18,
    USB_DBG_OPT_RSVD_19                 = 19,
    USB_DBG_OPT_RSVD_20                 = 20,
    USB_DBG_OPT_RSVD_21                 = 21,
    USB_DBG_OPT_RSVD_22                 = 22,
    USB_DBG_OPT_RSVD_23                 = 23,

    /* reserved */
    USB_DBG_OPT_RSVD_24                 = 24,
    USB_DBG_OPT_RSVD_25                 = 25,
    USB_DBG_OPT_RSVD_26                 = 26,
    USB_DBG_OPT_RSVD_27                 = 27,
    USB_DBG_OPT_RSVD_28                 = 28,
    USB_DBG_OPT_RSVD_29                 = 29,
    USB_DBG_OPT_RSVD_30                 = 30,
    USB_DBG_OPT_RSVD_31                 = 31,

    /* reserved */
    USB_DBG_OPT_RSVD_32                 = 32,
    USB_DBG_OPT_RSVD_33                 = 33,
    USB_DBG_OPT_RSVD_34                 = 34,
    USB_DBG_OPT_RSVD_35                 = 35,
    USB_DBG_OPT_RSVD_36                 = 36,
    USB_DBG_OPT_RSVD_37                 = 37,
    USB_DBG_OPT_RSVD_38                 = 38,
    USB_DBG_OPT_RSVD_39                 = 39,

    /* reserved */
    USB_DBG_OPT_RSVD_40                 = 40,
    USB_DBG_OPT_RSVD_41                 = 41,
    USB_DBG_OPT_RSVD_42                 = 42,
    USB_DBG_OPT_RSVD_43                 = 43,
    USB_DBG_OPT_RSVD_44                 = 44,
    USB_DBG_OPT_RSVD_45                 = 45,
    USB_DBG_OPT_RSVD_46                 = 46,
    USB_DBG_OPT_RSVD_47                 = 47,

    /* reserved */
    USB_DBG_OPT_RSVD_48                 = 48,
    USB_DBG_OPT_RSVD_49                 = 49,
    USB_DBG_OPT_RSVD_50                 = 50,
    USB_DBG_OPT_RSVD_51                 = 51,
    USB_DBG_OPT_RSVD_52                 = 52,
    USB_DBG_OPT_RSVD_53                 = 53,
    USB_DBG_OPT_RSVD_54                 = 54,
    USB_DBG_OPT_RSVD_55                 = 55,

    /* reserved */
    USB_DBG_OPT_RSVD_56                 = 56,
    USB_DBG_OPT_RSVD_57                 = 57,
    USB_DBG_OPT_RSVD_58                 = 58,
    USB_DBG_OPT_RSVD_59                 = 59,
    USB_DBG_OPT_RSVD_60                 = 60,
    USB_DBG_OPT_RSVD_61                 = 61,
    USB_DBG_OPT_RSVD_62                 = 62,
    USB_DBG_OPT_RSVD_63                 = 63,

    USB_DBG_OPT_MAX                     = 63,
} USB_DBG_OPT_t;


#ifdef USB_DBG
extern uint32_t g_USB_DBG_filter;
extern uint32_t g_usb_dbg_opt[2];

#define USB_DBG_I(type, message, arg_cnt, ...)                          \
    if( g_USB_DBG_filter & type ) {                                     \
        LOG_MSGID_I(USB_DBG, "DBG "message, arg_cnt, ##__VA_ARGS__);    \
    }

#define USB_DBG_E(type, message, arg_cnt, ...)                          \
    if( g_USB_DBG_filter & type ) {                                     \
        LOG_MSGID_E(USB_DBG, "DBG "message, arg_cnt, ##__VA_ARGS__);    \
    }

uint8_t usb_dbg_is_opt_enable(uint8_t opt);

#else

#define USB_DBG_I(type, message, arg_cnt, ...)
#define USB_DBG_E(type, message, arg_cnt, ...)
#define usb_dbg_is_opt_enable(opt) (0)

#endif

#endif /* AIR_USB_ENABLE */

#endif /* USB_DBG_H */


/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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

#ifdef AIR_USB_AUDIO_ENABLE

#if defined(MTK_USB_AUDIO_V2_ENABLE)
#error "MTK_USB_AUDIO_V2_ENABLE, USB-Audio version is not supported !"
#endif

/* C library */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* USB Middleware includes */
#include "usb.h"
#include "usb_host_detect.h"
#include "usb_nvkey_struct.h"
#include "usb_resource.h"
#include "usbaudio_drv.h"

/* Other Middleware includes */
#ifdef AIR_NVDM_ENABLE
#include "nvkey.h"
#include "nvkey_id_list.h"
#endif /* AIR_NVDM_ENABLE */

/* Hal includes */
#include "hal.h"
#include "hal_usb.h"
#include "hal_usb_internal.h"
#include "hal_resource_assignment.h"

/* Other includes */
#include "memory_attribute.h"

#if defined(USB_AUDIO_TX_TIME_MEASURE) || defined(USB_AUDIO_RX_TIME_MEASURE)
#include "swla.h"
#endif

#if defined(AIR_AUDIO_DUMP_ENABLE)
#if defined(USB_AUDIO_DUMP_RX1) || defined(USB_AUDIO_DUMP_RX2) || defined(USB_AUDIO_DUMP_TX1)
#include "audio_dump.h"
#endif
#endif

/* Syslog create module for usbaudio_drv.c */
#include "usb_dbg.h"
log_create_module_variant(USBAUDIO_DRV, DEBUG_LOG_ON, PRINT_LEVEL_INFO);

#if defined(AIR_USB_AUDIO_ENABLE)

#define AUDFREQ_TO_U32(FREQ) ((FREQ.data[0] << 0) + (FREQ.data[1] << 8) + (FREQ.data[2] << 16))
#define U32_TO_AUDFREQ(U32)  ({((U32 & 0x0000FF) >> 0), ((U32 & 0x00FF00) >> 8), ((U32 & 0xFF0000) >> 16)})

#ifdef AIR_USB_AUDIO_1_SPK_ENABLE
#define AIR_USB_AUDIO_1_SPK_FEATURE_ENABLE (true)
#else
#define AIR_USB_AUDIO_1_SPK_FEATURE_ENABLE (false)
#endif /* AIR_USB_AUDIO_1_SPK_ENABLE */

#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
#define AIR_USB_AUDIO_1_MIC_FEATURE_ENABLE (true)
#else
#define AIR_USB_AUDIO_1_MIC_FEATURE_ENABLE (false)
#endif /* AIR_USB_AUDIO_1_MIC_ENABLE */

#ifdef AIR_USB_AUDIO_2_SPK_ENABLE
#define AIR_USB_AUDIO_2_SPK_FEATURE_ENABLE (true)
#else
#define AIR_USB_AUDIO_2_SPK_FEATURE_ENABLE (false)
#endif /* AIR_USB_AUDIO_2_SPK_ENABLE */

#ifdef AIR_USB_AUDIO_2_MIC_ENABLE
#define AIR_USB_AUDIO_2_MIC_FEATURE_ENABLE (true)
#else
#define AIR_USB_AUDIO_2_MIC_FEATURE_ENABLE (false)
#endif /* AIR_USB_AUDIO_2_MIC_ENABLE */

UsbAudioStruct USB_Audio[2];
UsbAudio_Struct g_UsbAudio[2] = {
    /* Audio 1 device settings */
    {
        .spk_feature_en    = AIR_USB_AUDIO_1_SPK_FEATURE_ENABLE,
        .spk_alt2_en       = false,
        .spk_alt3_en       = false,
        .mic_feature_en    = AIR_USB_AUDIO_1_MIC_FEATURE_ENABLE,
        .mic_alt2_en       = false,
        .mic_alt3_en       = false,
        .spk_terminal_type = 0x0402,
        .spk_cur           = 0xF59A,
        .spk_min           = 0xB600,
        .spk_max           = 0x0000,
        .spk_res           = 0x0100,
        .spk_chs           = 0x02,
        .spk_chs_map       = USB_AUDIO_CHANNEL_CONBINE_2CH,
        .spk_vc            = USB_AUDIO_VC_INDIVIUAL,
        .mic_terminal_type = 0x0402,
        .mic_cur           = 0xF59A,
        .mic_min           = 0xB600,
        .mic_max           = 0x0000,
        .mic_res           = 0x0100,
        .mic_chs           = 0x01,
        .mic_chs_map       = USB_AUDIO_CHANNEL_CONBINE_MONO,
        .mic_vc            = USB_AUDIO_VC_MASTER,
    },
    /* Audio 2 device settings */
    {
        .spk_feature_en    = false,
        .spk_alt2_en       = true,  /* default is true and decided by USB_AUDIO_RX1_ALT2_ENABLE or USB_Aduio_Set_RX2_Alt2 */
        .mic_feature_en    = false,
        .spk_terminal_type = 0x0402,
        .spk_cur           = 0xF59A,
        .spk_min           = 0xB600,
        .spk_max           = 0x0000,
        .spk_res           = 0x0100,
        .spk_chs           = 0x02,
        .spk_chs_map       = USB_AUDIO_CHANNEL_CONBINE_2CH,
        .spk_vc            = USB_AUDIO_VC_INDIVIUAL,
        .mic_terminal_type = 0x0402,
        .mic_cur           = 0xF59A,
        .mic_min           = 0xB600,
        .mic_max           = 0x0000,
        .mic_res           = 0x0100,
        .mic_chs           = 0x01,
        .mic_chs_map       = USB_AUDIO_CHANNEL_CONBINE_MONO,
        .mic_vc            = USB_AUDIO_VC_MASTER,
    },
};

/* static functions */
static int16_t usb_audio_dscr_interface_serialize(void *dscr, void *out, uint16_t ava_size) __unused;
static int16_t usb_audio_dscr_control_header_serialize(void *dscr, void *out, uint16_t ava_size) __unused;
static int16_t usb_audio_dscr_it_serialize(void *dscr, void *out, uint16_t ava_size) __unused;
static int16_t usb_audio_dscr_ot_serialize(void *dscr, void *out, uint16_t ava_size) __unused;
static int16_t usb_audio_dscr_mixer_serialize(void *dscr, void *out, uint16_t ava_size) __unused;
//static int16_t usb_audio_dscr_selector_serialize(void *dscr, void *out, uint16_t ava_size) __unused;
static int16_t usb_audio_dscr_feature_serialize(void *dscr, void *out, uint16_t ava_size) __unused;
static int16_t usb_audio_dscr_as_general_serialize(void *dscr, void *out, uint16_t ava_size) __unused;
static int16_t usb_audio_dscr_as_format_type_serialize(void *dscr, void *out, uint16_t ava_size) __unused;
static int16_t dscr_list_serialize(usb_audio_dscr_hdlr_t *dscr_list, uint8_t len, uint8_t *out, uint16_t ava_len) __unused;

static uint8_t _USB_Audio_Channel_Check(uint8_t ch)
{
    if(ch>USB_AUDIO_MAX_CHANNEL_NUM){
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Check_Channel ch:%d ?? force to 0", 1, ch);
        ch = 0;
    }

    return ch;
}

/******************************************  Configuration Start    *********************************************/
static usb_audio_dscr_interface_t audio1_interface_dscr = {
    .bLength            = USB_IFDSC_LENGTH,
    .bDescriptorType    = USB_INTERFACE,
    .bInterfaceNumber   = 0x00, /* Overwrite by USB_Audio1_ControlIf_Create */
    .bAlternateSetting  = 0x00,
    .bNumEndpoints      = 0x00,
    .bInterfaceClass    = 0x01,
    .bInterfaceSubClass = 0x01,
    .bInterfaceProtocol = USB_AUDIO_INTERFACE_PROTOCOL,
    .iInterface         = 0x00, /* Overwrite by USB_Audio1_ControlIf_Create */
};

static usb_audio_dscr_control_header_t audio1_ach_dscr = {
    .bLength            = 0x00, /* Overwrite by USB_Audio1_ControlIf_Create */
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AC_SUBTYPE_HEADER,
    .bcdADC             = 0x0100,
    .wTotalLength       = 0x0047, /* Overwrite by USB_Audio1_ControlIf_Create */
    .bInCollection      = 0x02,   /* Overwrite by USB_Audio1_ControlIf_Create */
    .baInterfaceNr      =         /* Overwrite by USB_Audio1_ControlIf_Create */
    {
        0x01,
        0x02,
    },
};

static usb_audio_dscr_it_t audio1_spk_it_dscr = {
    .bLength            = 0x0C,
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AC_SUBTYPE_INPUT_TERMINAL,
    .bTerminalID        = 0x01,
    .wTerminalType      = 0x0101,
    .bAssocTerminal     = 0x00,
    .bNrChannels        = 0x02,
    .wChannelConfig     = 0x0003, /* (L, R) */
    .iChannelNames      = 0x00,
    .iTerminal          = 0x00,
};

static usb_audio_dscr_feature_t audio1_spk_feature_dscr = {
    .bLength            = 0x07 + 1 * 3,  /* 0x07 + bControlSize * control_nr */
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AC_SUBTYPE_FEATURE_UNIT,
    .bUnitID            = 0x02,
    .bSourceID          = 0x01,
    .bControlSize       = 0x01,
    .bmaControls =
        {
            0x01, /* bmaControls(0); Master Channel support Mute      */
            0x02, /* bmaControls(1); Logical Channel 1 support Volume */
            0x02, /* bmaControls(2); Logical Channel 2 support Volume */
        },
    .iFeature   = 0x00,
    .control_nr = 0x03, /* num of bmaControls */
};

static usb_audio_dscr_ot_t audio1_spk_ot_dscr = {
    .bLength            = 0x09,
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AC_SUBTYPE_OUTPUT_TERMINAL,
    .bTerminalID        = 0x03,
    .wTerminalType      = 0x0402,
    .bAssocTerminal     = 0x00,
    .bSourceID          = 0x02,
    .iTerminal          = 0x00,
};

static usb_audio_dscr_it_t audio1_mic_it_dscr = {
    .bLength            = 0x0C,
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AC_SUBTYPE_INPUT_TERMINAL,
    .bTerminalID        = 0x05,
    .wTerminalType      = 0x0201,
    .bAssocTerminal     = 0x0B,
    .bNrChannels        = 0x01,
    .wChannelConfig     = 0x0000,
    .iChannelNames      = 0x00,
    .iTerminal          = 0x00,
};

static usb_audio_dscr_feature_t audio1_mic_feature_dscr = {
    .bLength            = 0x07 + 1 * 2,  /* 0x07 + bControlSize * control_nr */
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AC_SUBTYPE_FEATURE_UNIT,
    .bUnitID            = 0x06,
    .bSourceID          = 0x05,
    .bControlSize       = 0x01,
    .bmaControls =
        {
            0x01, /* bmaControls(0); Master Channel support Mute      */
            0x02, /* bmaControls(1); Logical Channel 1 support Volume */
        },
    .iFeature   = 0x00,
    .control_nr = 0x02, /* num of bmaControls */
};

/* Unused
static usb_audio_dscr_selector_t audio1_selector_dscr = {
    .bLength            = 0x07,
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AC_SUBTYPE_SELECTOR_UNIT,
    .bUnitID            = 0x04,
    .bNrInPins          = 0x01,
    .baSourceID =
        {
            0x06,
        },
    .iSelector = 0x00,
};
*/

static usb_audio_dscr_ot_t audio1_mic_ot_dscr = {
    .bLength            = 0x09,
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AC_SUBTYPE_OUTPUT_TERMINAL,
    .bTerminalID        = 0x07,
    .wTerminalType      = 0x0101,
    .bAssocTerminal     = 0x00,
    .bSourceID          = 0x06,
    .iTerminal          = 0x00,
};

static usb_audio_dscr_hdlr_t audio1_aci_dscrs[] = {
    {&audio1_interface_dscr, usb_audio_dscr_interface_serialize},
    {&audio1_ach_dscr, usb_audio_dscr_control_header_serialize},
};

static usb_audio_dscr_hdlr_t audio1_spk_dscrs[] = {
    {&audio1_spk_it_dscr, usb_audio_dscr_it_serialize},
    {&audio1_spk_feature_dscr, usb_audio_dscr_feature_serialize},
    {&audio1_spk_ot_dscr, usb_audio_dscr_ot_serialize},
};

static usb_audio_dscr_hdlr_t audio1_mic_dscrs[] = {
    {&audio1_mic_it_dscr, usb_audio_dscr_it_serialize},
    {&audio1_mic_feature_dscr, usb_audio_dscr_feature_serialize},
    //{&audio1_selector_dscr, usb_audio_dscr_selector_serialize},
    {&audio1_mic_ot_dscr, usb_audio_dscr_ot_serialize},
};

static usb_audio_dscr_interface_t audio2_interface_dscr = {
    .bLength            = USB_IFDSC_LENGTH,
    .bDescriptorType    = USB_INTERFACE,
    .bInterfaceNumber   = 0x00, /* Overwrite by USB_Audio2_ControlIf_Create */
    .bAlternateSetting  = 0x00,
    .bNumEndpoints      = 0x00,
    .bInterfaceClass    = 0x01,
    .bInterfaceSubClass = 0x01,
    .bInterfaceProtocol = USB_AUDIO_INTERFACE_PROTOCOL,
    .iInterface         = 0x00, /* Overwrite by USB_Audio2_ControlIf_Create */
};

static usb_audio_dscr_control_header_t audio2_ach_dscr = {
    .bLength            = 0x00, /* Overwrite by USB_Audio2_ControlIf_Create */
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AC_SUBTYPE_HEADER,
    .bcdADC             = 0x0100,
    .wTotalLength       = 0x0047, /* Overwrite by USB_Audio2_ControlIf_Create */
    .bInCollection      = 0x02,   /* Overwrite by USB_Audio2_ControlIf_Create */
    .baInterfaceNr      =         /* Overwrite by USB_Audio2_ControlIf_Create */
    {
        0x01,
        0x02,
    },
};

static usb_audio_dscr_it_t audio2_spk_it_dscr = {
    .bLength            = 0x0C,
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AC_SUBTYPE_INPUT_TERMINAL,
    .bTerminalID        = 0x09,
    .wTerminalType      = 0x0101,
    .bAssocTerminal     = 0x00,
    .bNrChannels        = 0x02,
    .wChannelConfig     = 0x0003, /* (L, R) */
    .iChannelNames      = 0x00,
    .iTerminal          = 0x00,
};

static usb_audio_dscr_feature_t audio2_spk_feature_dscr = {
    .bLength            = 0x07 + 1 * 3,  /* 0x07 + bControlSize * control_nr */
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AC_SUBTYPE_FEATURE_UNIT,
    .bUnitID            = 0x0A,
    .bSourceID          = 0x09,
    .bControlSize       = 0x01,
    .bmaControls =
        {
            0x01, /* bmaControls(0); Master Channel support Mute      */
            0x02, /* bmaControls(1); Logical Channel 1 support Volume */
            0x02, /* bmaControls(2); Logical Channel 2 support Volume */
        },
    .iFeature   = 0x00,
    .control_nr = 0x03, /* num of bmaControls */
};

static usb_audio_dscr_ot_t audio2_spk_ot_dscr = {
    .bLength            = 0x09,
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AC_SUBTYPE_OUTPUT_TERMINAL,
    .bTerminalID        = 0x0B,
    .wTerminalType      = 0x0302,
    .bAssocTerminal     = 0x00,
    .bSourceID          = 0x0A,
    .iTerminal          = 0x00,
};

static usb_audio_dscr_it_t audio2_mic_it_dscr = {
    .bLength            = 0x0C,
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AC_SUBTYPE_INPUT_TERMINAL,
    .bTerminalID        = 0x0C,
    .wTerminalType      = 0x0201,
    .bAssocTerminal     = 0x0B,
    .bNrChannels        = 0x01,
    .wChannelConfig     = 0x0000,
    .iChannelNames      = 0x00,
    .iTerminal          = 0x00,
};

static usb_audio_dscr_feature_t audio2_mic_feature_dscr = {
    .bLength            = 0x09,
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AC_SUBTYPE_FEATURE_UNIT,
    .bUnitID            = 0x0D,
    .bSourceID          = 0x0C,
    .bControlSize       = 0x01,
    .bmaControls =
        {
            0x01, /* bmaControls(0); Master Channel support Mute      */
            0x02, /* bmaControls(1); Logical Channel 1 support Volume */
        },
    .iFeature   = 0x00,
    .control_nr = 0x02, /* num of bmaControls */
};

/* Unused
static usb_audio_dscr_selector_t audio2_selector_dscr = {
    .bLength            = 0x07,
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AC_SUBTYPE_SELECTOR_UNIT,
    .bUnitID            = 0x0E,
    .bNrInPins          = 0x01,
    .baSourceID =
        {
            0x0D,
        },
    .iSelector = 0x00,
};*/

static usb_audio_dscr_ot_t audio2_mic_ot_dscr = {
    .bLength            = 0x09,
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AC_SUBTYPE_OUTPUT_TERMINAL,
    .bTerminalID        = 0x0F,
    .wTerminalType      = 0x0101,
    .bAssocTerminal     = 0x00,
    .bSourceID          = 0x0D,
    .iTerminal          = 0x00,
};

static usb_audio_dscr_hdlr_t audio2_aci_dscrs[] = {
    {&audio2_interface_dscr, usb_audio_dscr_interface_serialize},
    {&audio2_ach_dscr, usb_audio_dscr_control_header_serialize},
};

static usb_audio_dscr_hdlr_t audio2_spk_dscrs[] = {
    {&audio2_spk_it_dscr, usb_audio_dscr_it_serialize},
    {&audio2_spk_feature_dscr, usb_audio_dscr_feature_serialize},
    {&audio2_spk_ot_dscr, usb_audio_dscr_ot_serialize},
};

static usb_audio_dscr_hdlr_t audio2_mic_dscrs[] = {
    {&audio2_mic_it_dscr, usb_audio_dscr_it_serialize},
    {&audio2_mic_feature_dscr, usb_audio_dscr_feature_serialize},
    //{&audio2_selector_dscr, usb_audio_dscr_selector_serialize},
    {&audio2_mic_ot_dscr, usb_audio_dscr_ot_serialize},
};

/****************************************** Configuration End *********************************************/
static usb_audio_dscr_interface_t audio1_spk_stream_if_dscr = {
    .bLength            = USB_IFDSC_LENGTH,
    .bDescriptorType    = USB_INTERFACE,
    .bInterfaceNumber   = 0x00, /* Overwrite by USB_Audio1_StreamIf_Create */
    .bAlternateSetting  = 0x00,
    .bNumEndpoints      = 0x00,
    .bInterfaceClass    = 0x01,
    .bInterfaceSubClass = 0x02,
    .bInterfaceProtocol = USB_AUDIO_INTERFACE_PROTOCOL,
    .iInterface         = 0x00, /* Overwrite by USB_Audio1_StreamIf_Create */
};

static usb_audio_dscr_interface_t audio1_spk_stream_alt_if_dscr = {
    .bLength            = USB_IFDSC_LENGTH,
    .bDescriptorType    = USB_INTERFACE,
    .bInterfaceNumber   = 0x00, /* Overwrite by USB_Audio1_StreamIf_Create */
    .bAlternateSetting  = 0x01,
    .bNumEndpoints      = 0x01,
    .bInterfaceClass    = 0x01,
    .bInterfaceSubClass = 0x02,
    .bInterfaceProtocol = USB_AUDIO_INTERFACE_PROTOCOL,
    .iInterface         = 0x00, /* Overwrite by USB_Audio1_StreamIf_Create */
};

#ifdef USB_AUDIO_RX1_ALT2_ENABLE
static usb_audio_dscr_interface_t audio1_spk_stream_alt2_if_dscr = {
    .bLength            = USB_IFDSC_LENGTH,
    .bDescriptorType    = USB_INTERFACE,
    .bInterfaceNumber   = 0x00, /* Overwrite by USB_Audio1_StreamIf_Create */
    .bAlternateSetting  = 0x02,
    .bNumEndpoints      = 0x01,
    .bInterfaceClass    = 0x01,
    .bInterfaceSubClass = 0x02,
    .bInterfaceProtocol = USB_AUDIO_INTERFACE_PROTOCOL,
    .iInterface         = 0x00, /* Overwrite by USB_Audio1_StreamIf_Create */
};
#endif

#ifdef USB_AUDIO_RX1_ALT3_ENABLE
static usb_audio_dscr_interface_t audio1_spk_stream_alt3_if_dscr = {
    .bLength            = USB_IFDSC_LENGTH,
    .bDescriptorType    = USB_INTERFACE,
    .bInterfaceNumber   = 0x00, /* Overwrite by USB_Audio1_StreamIf_Create */
    .bAlternateSetting  = 0x03,
    .bNumEndpoints      = 0x01,
    .bInterfaceClass    = 0x01,
    .bInterfaceSubClass = 0x02,
    .bInterfaceProtocol = USB_AUDIO_INTERFACE_PROTOCOL,
    .iInterface         = 0x00, /* Overwrite by USB_Audio1_StreamIf_Create */
};
#endif

static usb_audio_dscr_as_general_t audio1_spk_stream_general_dscr = {
    .bLength            = 0x07,
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AS_SUBTYPE_GENERAL,
    .bTerminalLink      = 0x01, /* Overwrite by USB_Audio1_StreamIf_Create */
    .bDelay             = 0x01,
    .wFormatTag         = 0x0001, /* PCM */
};

static usb_audio_dscr_as_format_type_t audio1_spk_stream_format_dscr = {
    .bLength            = 0x08 + 3 * 1, /* 8 + freq_size(3) * num_of_freq */
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AS_SUBTYPE_FORMAT_TYPE,
    .bFormatType        = 0x01, /* FORMAT_TYPE_I        */
    .bNrChannels        = 0x02, /* 2 channels           */
    .bSubFrameSize      = 0x02, /* 2 bytes per subframe */
    .bBitResolution     = 0x10, /* 16 bits per sample   */
    .bSamFreqType       = 0x01, /* num of tSamFreq      */
    .tSamFreq =
        {
            {.data = {0x80, 0xBB, 0x00}}, /* tSamFreq[1]; 48000 Hz */
            {.data = {0x00, 0x77, 0x01}}, /* tSamFreq[2]; 96000 Hz */
        },
};

#ifdef USB_AUDIO_RX1_ALT2_ENABLE
static usb_audio_dscr_as_format_type_t audio1_spk_stream_alt2_format_dscr = {
    .bLength            = 0x08 + 3 * 2, /* 8 + freq_size(3) * num_of_freq */
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AS_SUBTYPE_FORMAT_TYPE,
    .bFormatType        = 0x01, /* FORMAT_TYPE_I        */
    .bNrChannels        = 0x02, /* 2 channels           */
    .bSubFrameSize      = 0x03, /* 3 bytes per subframe */
    .bBitResolution     = 0x18, /* 24 bits per sample   */
    .bSamFreqType       = 0x02, /* num of tSamFreq      */
    .tSamFreq =
        {
            {.data = {0x80, 0xBB, 0x00}}, /* tSamFreq[1]; 48000 Hz */
            {.data = {0x00, 0x77, 0x01}}, /* tSamFreq[2]; 96000 Hz */
        },
};
#endif

#ifdef USB_AUDIO_RX1_ALT3_ENABLE
static usb_audio_dscr_as_format_type_t audio1_spk_stream_alt3_format_dscr = {
    .bLength            = 0x08 + 3 * 2, /* 8 + freq_size(3) * num_of_freq */
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AS_SUBTYPE_FORMAT_TYPE,
    .bFormatType        = 0x01, /* FORMAT_TYPE_I        */
    .bNrChannels        = 0x02, /* 2 channels           */
    .bSubFrameSize      = 0x03, /* 3 bytes per subframe */
    .bBitResolution     = 0x18, /* 24 bits per sample   */
    .bSamFreqType       = 0x01, /* num of tSamFreq      */
    .tSamFreq =
        {
            {.data = {0x00, 0xEE, 0x02}}, /* tSamFreq[1]; 192000 Hz */
        },
};
#endif

static usb_audio_dscr_hdlr_t audio1_spk_as_if_dscrs[] = {
    {&audio1_spk_stream_if_dscr, usb_audio_dscr_interface_serialize},
};

static usb_audio_dscr_hdlr_t audio1_spk_as_if_alt_dscrs[] = {
    {&audio1_spk_stream_alt_if_dscr, usb_audio_dscr_interface_serialize},
    {&audio1_spk_stream_general_dscr, usb_audio_dscr_as_general_serialize},
    {&audio1_spk_stream_format_dscr, usb_audio_dscr_as_format_type_serialize},
};

#ifdef USB_AUDIO_RX1_ALT2_ENABLE
static usb_audio_dscr_hdlr_t audio1_spk_as_if_alt2_dscrs[] = {
    {&audio1_spk_stream_alt2_if_dscr, usb_audio_dscr_interface_serialize},
    {&audio1_spk_stream_general_dscr, usb_audio_dscr_as_general_serialize},
    {&audio1_spk_stream_alt2_format_dscr, usb_audio_dscr_as_format_type_serialize},
};
#endif

#ifdef USB_AUDIO_RX1_ALT3_ENABLE
static usb_audio_dscr_hdlr_t audio1_spk_as_if_alt3_dscrs[] = {
    {&audio1_spk_stream_alt3_if_dscr, usb_audio_dscr_interface_serialize},
    {&audio1_spk_stream_general_dscr, usb_audio_dscr_as_general_serialize},
    {&audio1_spk_stream_alt3_format_dscr, usb_audio_dscr_as_format_type_serialize},
};
#endif

#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
static usb_audio_dscr_interface_t audio1_mic_stream_if_dscr = {
    .bLength            = USB_IFDSC_LENGTH,
    .bDescriptorType    = USB_INTERFACE,
    .bInterfaceNumber   = 0x00, /* Overwrite by USB_Audio_StreamIf_Microphone_Create */
    .bAlternateSetting  = 0x00,
    .bNumEndpoints      = 0x00,
    .bInterfaceClass    = 0x01,
    .bInterfaceSubClass = 0x02,
    .bInterfaceProtocol = USB_AUDIO_INTERFACE_PROTOCOL,
    .iInterface         = 0x00, /* Overwrite by USB_Audio_StreamIf_Microphone_Create */
};

static usb_audio_dscr_interface_t audio1_mic_stream_alt_if_dscr = {
    .bLength            = USB_IFDSC_LENGTH,
    .bDescriptorType    = USB_INTERFACE,
    .bInterfaceNumber   = 0x00, /* Overwrite by USB_Audio_StreamIf_Microphone_Create */
    .bAlternateSetting  = 0x01,
    .bNumEndpoints      = 0x01,
    .bInterfaceClass    = 0x01,
    .bInterfaceSubClass = 0x02,
    .bInterfaceProtocol = USB_AUDIO_INTERFACE_PROTOCOL,
    .iInterface         = 0x00, /* Overwrite by USB_Audio_StreamIf_Microphone_Create */
};

#ifdef USB_AUDIO_TX1_ALT2_ENABLE
static usb_audio_dscr_interface_t audio1_mic_stream_alt2_if_dscr = {
    .bLength            = USB_IFDSC_LENGTH,
    .bDescriptorType    = USB_INTERFACE,
    .bInterfaceNumber   = 0x00, /* Overwrite by USB_Audio_StreamIf_Microphone_Create */
    .bAlternateSetting  = 0x02,
    .bNumEndpoints      = 0x01,
    .bInterfaceClass    = 0x01,
    .bInterfaceSubClass = 0x02,
    .bInterfaceProtocol = USB_AUDIO_INTERFACE_PROTOCOL,
    .iInterface         = 0x00, /* Overwrite by USB_Audio_StreamIf_Microphone_Create */
};
#endif

static usb_audio_dscr_as_general_t audio1_mic_stream_general_dscr = {
    .bLength            = 0x07,
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AS_SUBTYPE_GENERAL,
    .bTerminalLink      = 0x07, /* Overwrite by USB_Audio_StreamIf_Microphone_Create */
    .bDelay             = 0x01,
    .wFormatTag         = 0x0001, /* PCM */
};

static usb_audio_dscr_as_format_type_t audio1_mic_stream_format_dscr = {
    .bLength            = 0x08 + 3 * 2, /* 8 + freq_size(3) * num_of_freq */
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AS_SUBTYPE_FORMAT_TYPE,
    .bFormatType        = 0x01,     /* FORMAT_TYPE_I */
    .bNrChannels        = 0x01,     /* 1 channel */
    .bSubFrameSize      = 0x02,     /* 2 bytes per subframe */
    .bBitResolution     = 0x10,     /* 16 bits per sample */
    .bSamFreqType       = 0x02,     /* num of tSamFreq      */
    .tSamFreq =
        {
            {.data = {0x80, 0x3E, 0x00}}, /* tSamFreq[1]; 16000 Hz */
            {.data = {0x80, 0xBB, 0x00}}, /* tSamFreq[2]; 48000 Hz */
        },
};

#ifdef USB_AUDIO_TX1_ALT2_ENABLE
static usb_audio_dscr_as_format_type_t audio1_mic_stream_alt2_format_dscr = {
    .bLength            = 0x08 + 3 * 2, /* 8 + freq_size(3) * num_of_freq */
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AS_SUBTYPE_FORMAT_TYPE,
    .bFormatType        = 0x01,     /* FORMAT_TYPE_I */
    .bNrChannels        = 0x01,     /* 1 channel */
    .bSubFrameSize      = 0x03,     /* 3 bytes per subframe */
    .bBitResolution     = 0x18,     /* 24 bits per sample */
    .bSamFreqType       = 0x02,     /* num of tSamFreq      */
    .tSamFreq =
        {
            {.data = {0x80, 0x3E, 0x00}}, /* tSamFreq[1]; 16000 Hz */
            {.data = {0x80, 0xBB, 0x00}}, /* tSamFreq[2]; 48000 Hz */
        },
};
#endif

static usb_audio_dscr_hdlr_t audio1_mic_as_if_dscrs[] = {
    {&audio1_mic_stream_if_dscr, usb_audio_dscr_interface_serialize},
};

static usb_audio_dscr_hdlr_t audio1_mic_as_if_alt_dscrs[] = {
    {&audio1_mic_stream_alt_if_dscr, usb_audio_dscr_interface_serialize},
    {&audio1_mic_stream_general_dscr, usb_audio_dscr_as_general_serialize},
    {&audio1_mic_stream_format_dscr, usb_audio_dscr_as_format_type_serialize},
};

#ifdef USB_AUDIO_TX1_ALT2_ENABLE
static usb_audio_dscr_hdlr_t audio1_mic_as_if_alt2_dscrs[] = {
    {&audio1_mic_stream_alt2_if_dscr, usb_audio_dscr_interface_serialize},
    {&audio1_mic_stream_general_dscr, usb_audio_dscr_as_general_serialize},
    {&audio1_mic_stream_alt2_format_dscr, usb_audio_dscr_as_format_type_serialize},
};
#endif

#endif /* AIR_USB_AUDIO_1_MIC_ENABLE */

static usb_audio_dscr_interface_t audio2_spk_stream_if_dscr = {
    .bLength            = USB_IFDSC_LENGTH,
    .bDescriptorType    = USB_INTERFACE,
    .bInterfaceNumber   = 0x00, /* Overwrite by USB_Audio2_StreamIf_Create */
    .bAlternateSetting  = 0x00,
    .bNumEndpoints      = 0x00,
    .bInterfaceClass    = 0x01,
    .bInterfaceSubClass = 0x02,
    .bInterfaceProtocol = USB_AUDIO_INTERFACE_PROTOCOL,
    .iInterface         = 0x00, /* Overwrite by USB_Audio2_StreamIf_Create */
};

static usb_audio_dscr_interface_t audio2_spk_stream_alt_if_dscr = {
    .bLength            = USB_IFDSC_LENGTH,
    .bDescriptorType    = USB_INTERFACE,
    .bInterfaceNumber   = 0x00, /* Overwrite by USB_Audio2_StreamIf_Create */
    .bAlternateSetting  = 0x01,
    .bNumEndpoints      = 0x01,
    .bInterfaceClass    = 0x01,
    .bInterfaceSubClass = 0x02,
    .bInterfaceProtocol = USB_AUDIO_INTERFACE_PROTOCOL,
    .iInterface         = 0x00, /* Overwrite by USB_Audio2_StreamIf_Create */
};

#ifdef USB_AUDIO_RX2_ALT2_ENABLE
static usb_audio_dscr_interface_t audio2_spk_stream_alt2_if_dscr = {
    .bLength            = USB_IFDSC_LENGTH,
    .bDescriptorType    = USB_INTERFACE,
    .bInterfaceNumber   = 0x00, /* Overwrite by USB_Audio2_StreamIf_Create */
    .bAlternateSetting  = 0x02,
    .bNumEndpoints      = 0x01,
    .bInterfaceClass    = 0x01,
    .bInterfaceSubClass = 0x02,
    .bInterfaceProtocol = USB_AUDIO_INTERFACE_PROTOCOL,
    .iInterface         = 0x00, /* Overwrite by USB_Audio2_StreamIf_Create */
};
#endif

static usb_audio_dscr_as_general_t audio2_spk_stream_general_dscr = {
    .bLength            = 0x07,
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AS_SUBTYPE_GENERAL,
    .bTerminalLink      = 0x09, /* Overwrite by USB_Audio2_StreamIf_Create */
    .bDelay             = 0x03,
    .wFormatTag         = 0x0001, /* PCM */
};

static usb_audio_dscr_as_format_type_t audio2_spk_stream_format_dscr = {
    .bLength            = 0x08 + 3 * 2, /* 8 + freq_size(3) * num_of_freq */
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AS_SUBTYPE_FORMAT_TYPE,
    .bFormatType        = 0x01, /* FORMAT_TYPE_I        */
    .bNrChannels        = 0x02, /* 2 channels           */
    .bSubFrameSize      = 0x03, /* 3 bytes per subframe */
    .bBitResolution     = 0x18, /* 24 bits per sample   */
    .bSamFreqType       = 0x02, /* num of tSamFreq      */
    .tSamFreq =
        {
            {.data = {0x80, 0xBB, 0x00}}, /* tSamFreq[1]; 48000 Hz */
            {.data = {0x00, 0x77, 0x01}}, /* tSamFreq[2]; 96000 Hz */
        },
};

#ifdef USB_AUDIO_RX2_ALT2_ENABLE
static usb_audio_dscr_as_format_type_t audio2_spk_stream_alt2_format_dscr = {
    .bLength            = 0x08 + 3 * 2, /* 8 + freq_size(3) * num_of_freq */
    .bDescriptorType    = 0x24,
    .bDescriptorSubtype = USB_AUDIO_AS_SUBTYPE_FORMAT_TYPE,
    .bFormatType        = 0x01, /* FORMAT_TYPE_I        */
    .bNrChannels        = 0x02, /* 2 channels           */
    .bSubFrameSize      = 0x03, /* 2 bytes per subframe */
    .bBitResolution     = 0x18, /* 24 bits per sample   */
    .bSamFreqType       = 0x02, /* num of tSamFreq      */
    .tSamFreq =
        {
            {.data = {0x80, 0xBB, 0x00}}, /* tSamFreq[1]; 48000 Hz */
            {.data = {0x00, 0x77, 0x01}}, /* tSamFreq[2]; 96000 Hz */
        },
};
#endif

static usb_audio_dscr_hdlr_t audio2_spk_as_if_dscrs[] = {
    {&audio2_spk_stream_if_dscr, usb_audio_dscr_interface_serialize},
};

static usb_audio_dscr_hdlr_t audio2_spk_as_if_alt_dscrs[] = {
    {&audio2_spk_stream_alt_if_dscr, usb_audio_dscr_interface_serialize},
    {&audio2_spk_stream_general_dscr, usb_audio_dscr_as_general_serialize},
    {&audio2_spk_stream_format_dscr, usb_audio_dscr_as_format_type_serialize},
};

#ifdef USB_AUDIO_RX2_ALT2_ENABLE
static usb_audio_dscr_hdlr_t audio2_spk_as_if_alt2_dscrs[] = {
    {&audio2_spk_stream_alt2_if_dscr, usb_audio_dscr_interface_serialize},
    {&audio2_spk_stream_general_dscr, usb_audio_dscr_as_general_serialize},
    {&audio2_spk_stream_alt2_format_dscr, usb_audio_dscr_as_format_type_serialize},
};
#endif

/****************************************** Speaker Endpoint Start *********************************************/

const uint8_t audio_stream_ep_out_dscr[] = {
    /* Speaker Usage*/
    0x09,             /*bLength;*/
    USB_ENDPOINT,     /*bDescriptorType;*/
    0x00,             /*bEndpointAddress;*/
    USB_EP_ISO | (USB_ISO_ADAPTIVE << 2), /*bmAttributes;*/
    0x00,             /*wMaxPacketSize[2]; Will be replaced by HAL_USB_MAX_PACKET_SIZE_ENDPOINT_ISOCHRONOUS_HIGH_SPEED &*/
    0x00,             /*wMaxPacketSize[2]; HAL_USB_MAX_PACKET_SIZE_ENDPOINT_ISOCHRONOUS_FULL_SPEED.*/
    0x01,             /*bInterval; Will be changed to 0x04 if high-speed @ USB_Audio_StreamIf_Speed_Reset().*/
    0x00,             /*bRefresh;*/
    0x00,             /*bSynchAddress;*/

    0x07,             /*bLength*/
    0x25,             /*bDescriptorType; Audio Endpoint Descriptor*/
    0x01,             /*bDescriptorSubtype; General*/
    0x01,             /*bmAttributes; Sampling Frequency*/
    0x00,             /*bLockDelayUnits*/
    0x00,             /*wLockDelay; LSB*/
    0x00,             /*wLockDelay; MSB*/
};
/****************************************** Speaker Endpoint End *********************************************/

/****************************************** Microphone Endpoint Start *********************************************/
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
const uint8_t audio_stream_ep_in_dscr[] = { /* microphone usage*/
    0x09,             /*bLength;*/
    USB_ENDPOINT,     /*bDescriptorType;*/
    0x00,             /*bEndpointAddress;*/
    USB_EP_ISO | (USB_ISO_ADAPTIVE << 2), /*bmAttributes;*/
    0x02,             /*wMaxPacketSize[2]; Will be replaced by HAL_USB_MAX_PACKET_SIZE_ENDPOINT_ISOCHRONOUS_HIGH_SPEED &*/
    0x00,             /*wMaxPacketSize[2]; HAL_USB_MAX_PACKET_SIZE_ENDPOINT_ISOCHRONOUS_FULL_SPEED.*/
    0x01,             /*bInterval; Will be changed to 0x04 if high-speed @ USB_Audio_StreamIf_Speed_Reset().*/
    0x00,             /*bRefresh;*/
    0x00,             /*bSynchAddress;*/

    0x07,             /*bLength*/
    0x25,             /*bDescriptorType; Audio Endpoint Descriptor*/
    0x01,             /*bDescriptorSubtype; General*/
    0x01,             /*bmAttributes; Sampling Frequency*/
    0x00,             /*bLockDelayUnits*/
    0x00,             /*wLockDelay; LSB*/
    0x00,             /*wLockDelay; MSB*/
};
#endif
/******************************************Microphone Endpoint end*********************************************/
/**
 * Final setting would be set up by USB_Audio_Get_RX_Alt_Byte befor descriptort generator of usb_resource.c
 * 1. USB_Software_Create 2. USB_Software_Speed_Init
 */
static unsigned int USB_Audio_Stream_Speed_Reset_Packet_Size(uint32_t port, bool b_other_speed)
{
    unsigned int  max_packet;

    /**
     * NOTE
     * Each endpoint cost some bus bandwidth. The total cost of each
     * endpoint may not over 100%. If over 100%, Windows will show
     * "Not enough USB controller resources", and usb device can't
     * normally work.
     *
     * The isochronous transaction cost persent could refers to
     * USB2.0 spec 5.6.5.
     */
    max_packet = USB_AUDIO_RX_EP_MAX_SIZE;

    return max_packet;
}

#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
static unsigned int USB_Audio_Stream_Microphone_Speed_Reset_Packet_Size(uint32_t port, bool b_other_speed)
{
    unsigned int  max_packet;

    max_packet = USB_AUDIO_TX_EP_MAX_SIZE;

    return max_packet;
}
#endif

/************************************************************
    USB Audio SetInterface Convertor
*************************************************************/
/* Convertor from Alt number to enable/disable */
bool USB_Audio_RX_SetAlt2Enable(uint32_t port, uint8_t alt_num)
{
    bool enable = false;

    if((port == 0) && (alt_num == 0)){
        enable = false;
    }
    else if((port == 0) && (alt_num == 1)){
        enable = true;
    }
    #ifdef USB_AUDIO_RX1_ALT2_ENABLE
    else if((port == 0) && (alt_num == 2)){
        enable = true;
    }
    #endif
    #ifdef USB_AUDIO_RX1_ALT3_ENABLE
    else if((port == 0) && (alt_num == 3)){
        enable = true;
    }
    #endif
    else if((port == 1) && (alt_num == 0)){
        enable = false;
    }
    else if((port == 1) && (alt_num == 1)){
        enable = true;
    }
    #ifdef USB_AUDIO_RX2_ALT2_ENABLE
     else if((port == 1) && (alt_num == 2)){
        enable = true;
    }
    #endif
    else {
        enable = false;
    }

    return enable;
}

#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
bool USB_Audio_TX_SetAlt2Enable(uint32_t port, uint8_t alt_num)
{
    bool enable = false;

    if((port == 0) && (alt_num == 0)){
        enable = false;
    }
    else if((port == 0) && (alt_num == 1)){
        enable = true;
    }
    #ifdef USB_AUDIO_TX1_ALT2_ENABLE
    else if((port == 0) && (alt_num == 2)){
        enable = true;
    }
    #endif
    else {
        enable = false;
    }

    return enable;
}
#endif

/************************************************************
    USB Audio Get/Set interface and endpoint ID
*************************************************************/
uint8_t audio1_rx_if_id, audio1_rx_ep_id;
uint8_t audio1_tx_if_id, audio1_tx_ep_id;
uint8_t audio2_rx_if_id, audio2_rx_ep_id;

void USB_Audio_Init_Chat_Game_Info()
{
    audio1_rx_if_id = USB_AUDIO_UNUSED_ID;
    audio1_rx_ep_id = USB_AUDIO_UNUSED_ID;
    audio1_tx_if_id = USB_AUDIO_UNUSED_ID;
    audio1_tx_ep_id = USB_AUDIO_UNUSED_ID;
    audio2_rx_if_id = USB_AUDIO_UNUSED_ID;
    audio2_rx_ep_id = USB_AUDIO_UNUSED_ID;
}

void USB_Audio_Get_Chat_Info(uint8_t *rx_if_id, uint8_t *rx_ep_id, uint8_t *tx_if_id, uint8_t *tx_ep_id)
{
    *rx_if_id = audio1_rx_if_id;
    *rx_ep_id = audio1_rx_ep_id;
    *tx_if_id = audio1_tx_if_id;
    *tx_ep_id = audio1_tx_ep_id;
    USB_DBG_I( USB_DBG_AUDIO_GET_INFO, "USB_Audio_Get_Chat_Info rx_if_id:0x%X rx_ep_id:0x%X tx_if_id:0x%X tx_ep_id:0x%X", 4,
                *rx_if_id, *rx_ep_id, *tx_if_id, *tx_ep_id);
}

void USB_Audio_Get_Game_Info(uint8_t *rx_if_id, uint8_t *rx_ep_id)
{
    *rx_if_id = audio2_rx_if_id;
    *rx_ep_id = audio2_rx_ep_id;
    USB_DBG_I( USB_DBG_AUDIO_GET_INFO, "USB_Audio_Get_Game_Info rx_if_id:0x%X rx_ep_id:0x%X", 2, *rx_if_id, *rx_ep_id);
}

void USB_Audio_Set_Chat_Info(uint8_t rx_if_id, uint8_t rx_ep_id, uint8_t tx_if_id, uint8_t tx_ep_id)
{
    if (rx_if_id != USB_AUDIO_UNUSED_ID) {
        audio1_rx_if_id = rx_if_id;
    }
    if (rx_ep_id != USB_AUDIO_UNUSED_ID) {
        audio1_rx_ep_id = rx_ep_id;
    }
    if (tx_if_id != USB_AUDIO_UNUSED_ID) {
        audio1_tx_if_id = tx_if_id;
    }
    if (tx_ep_id != USB_AUDIO_UNUSED_ID) {
        audio1_tx_ep_id = tx_ep_id;
    }
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Set_Chat_Info rx_if_id:0x%X rx_ep_id:0x%X tx_if_id:0x%X tx_ep_id:0x%X", 4,
                audio1_rx_if_id, audio1_rx_ep_id, audio1_tx_if_id, audio1_tx_ep_id);
}

void USB_Audio_Set_Game_Info(uint8_t rx_if_id, uint8_t rx_ep_id)
{
    if (rx_if_id != USB_AUDIO_UNUSED_ID) {
        audio2_rx_if_id = rx_if_id;
    }
    if (rx_ep_id != USB_AUDIO_UNUSED_ID) {
        audio2_rx_ep_id = rx_ep_id;
    }
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Set_Game_Info rx_if_id:0x%X rx_ep_id:0x%X", 2, audio2_rx_if_id, audio2_rx_ep_id);
}


/************************************************************
    Interface initialization functions
*************************************************************/
uint8_t USB_Audio_Get_StreamIf_Num(uint8_t port)
{
    uint8_t if_id;

    if (g_UsbAudio[port].stream_interface_id == 0xFF) {
        /* Get resource number and register to gUsbDevice */
        g_UsbAudio[port].stream_if_info = USB_Get_Interface_Number(&if_id);
        g_UsbAudio[port].stream_interface_id = if_id;
    }

    return g_UsbAudio[port].stream_interface_id;
}

/************************************************************
    USB Audio setting by APP layer functions
*************************************************************/
void USB_Aduio_Set_RX1_Alt1(uint8_t smaple_rate_num, uint8_t smaple_size, uint8_t channel, uint32_t *sample_rate)
{
    audio1_spk_stream_format_dscr.bLength          = 0x08 + 3 * smaple_rate_num;
    audio1_spk_stream_format_dscr.bSamFreqType     = smaple_rate_num;
    audio1_spk_stream_format_dscr.bSubFrameSize    = smaple_size;
    audio1_spk_stream_format_dscr.bBitResolution   = smaple_size * 8;
    audio1_spk_stream_format_dscr.bNrChannels      = channel;

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Aduio_Set_RX1_Alt1 smaple_rate_num:%d smaple_size:%d channel:%d", 3,
                smaple_rate_num, smaple_size, channel);

    uint8_t i;
    for(i=0; i<USB_AUDIO_DSCR_MAX_FREQ_NUM; i++) {
        audio1_spk_stream_format_dscr.tSamFreq[i].data[0] = (uint8_t)(sample_rate[i]);
        audio1_spk_stream_format_dscr.tSamFreq[i].data[1] = (uint8_t)(sample_rate[i] >> 8);
        audio1_spk_stream_format_dscr.tSamFreq[i].data[2] = (uint8_t)(sample_rate[i] >> 16);
    }

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Aduio_Set_RX1_Alt1 smaple_rate: %d %d %d %d %d", 5,
                audio1_spk_stream_format_dscr.tSamFreq[0],
                audio1_spk_stream_format_dscr.tSamFreq[1],
                audio1_spk_stream_format_dscr.tSamFreq[2],
                audio1_spk_stream_format_dscr.tSamFreq[3],
                audio1_spk_stream_format_dscr.tSamFreq[4]);
}

void USB_Aduio_Set_RX1_Alt2(bool alt2_en, uint8_t smaple_rate_num, uint8_t smaple_size, uint8_t channel, uint32_t *sample_rate)
{
#ifdef USB_AUDIO_RX1_ALT2_ENABLE
    g_UsbAudio[0].spk_alt2_en = alt2_en;

    if(alt2_en == false){
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Aduio_Set_RX1_Alt2 alt2_en is disable", 0);
        return;
    }

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Aduio_Set_RX1_Alt2 smaple_rate_num:%d smaple_size:%d channel:%d", 3,
                smaple_rate_num, smaple_size, channel);

    audio1_spk_stream_alt2_format_dscr.bLength          = 0x08 + 3 * smaple_rate_num;
    audio1_spk_stream_alt2_format_dscr.bSamFreqType     = smaple_rate_num;
    audio1_spk_stream_alt2_format_dscr.bSubFrameSize    = smaple_size;
    audio1_spk_stream_alt2_format_dscr.bBitResolution   = smaple_size * 8;
    audio1_spk_stream_alt2_format_dscr.bNrChannels      = channel;

    uint8_t i;
    for(i=0; i<USB_AUDIO_DSCR_MAX_FREQ_NUM; i++) {
        audio1_spk_stream_alt2_format_dscr.tSamFreq[i].data[0] = (uint8_t)(sample_rate[i]);
        audio1_spk_stream_alt2_format_dscr.tSamFreq[i].data[1] = (uint8_t)(sample_rate[i] >> 8);
        audio1_spk_stream_alt2_format_dscr.tSamFreq[i].data[2] = (uint8_t)(sample_rate[i] >> 16);
    }

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Aduio_Set_RX1_Alt2 smaple_rate: %d %d %d %d %d", 5,
                audio1_spk_stream_alt2_format_dscr.tSamFreq[0],
                audio1_spk_stream_alt2_format_dscr.tSamFreq[1],
                audio1_spk_stream_alt2_format_dscr.tSamFreq[2],
                audio1_spk_stream_alt2_format_dscr.tSamFreq[3],
                audio1_spk_stream_alt2_format_dscr.tSamFreq[4]);

#else
    LOG_MSGID_W(USBAUDIO_DRV, "USB_Aduio_Set_RX1_Alt2 USB_AUDIO_RX1_ALT2_ENABLE is not enable", 0);
#endif
}

void USB_Aduio_Set_RX1_Alt3(bool alt3_en, uint8_t smaple_rate_num, uint8_t smaple_size, uint8_t channel, uint32_t *sample_rate)
{
#ifdef USB_AUDIO_RX1_ALT3_ENABLE
    g_UsbAudio[0].spk_alt3_en = alt3_en;

    if(alt3_en == false) {
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Aduio_Set_RX1_Alt3 alt3_en is disable", 0);
        return;
    }

    audio1_spk_stream_alt3_format_dscr.bLength          = 0x08 + 3 * smaple_rate_num;
    audio1_spk_stream_alt3_format_dscr.bSamFreqType     = smaple_rate_num;
    audio1_spk_stream_alt3_format_dscr.bSubFrameSize    = smaple_size;
    audio1_spk_stream_alt3_format_dscr.bBitResolution   = smaple_size * 8;
    audio1_spk_stream_alt3_format_dscr.bNrChannels      = channel;

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Aduio_Set_RX1_Alt3 smaple_rate_num:%d smaple_size:%d channel:%d", 3,
                smaple_rate_num, smaple_size, channel);

    uint8_t i;
    for(i=0; i<USB_AUDIO_DSCR_MAX_FREQ_NUM; i++) {
        audio1_spk_stream_alt3_format_dscr.tSamFreq[i].data[0] = (uint8_t)(sample_rate[i]);
        audio1_spk_stream_alt3_format_dscr.tSamFreq[i].data[1] = (uint8_t)(sample_rate[i] >> 8);
        audio1_spk_stream_alt3_format_dscr.tSamFreq[i].data[2] = (uint8_t)(sample_rate[i] >> 16);
    }

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Aduio_Set_RX1_Alt3 smaple_rate: %d %d %d %d %d", 5,
                audio1_spk_stream_alt3_format_dscr.tSamFreq[0],
                audio1_spk_stream_alt3_format_dscr.tSamFreq[1],
                audio1_spk_stream_alt3_format_dscr.tSamFreq[2],
                audio1_spk_stream_alt3_format_dscr.tSamFreq[3],
                audio1_spk_stream_alt3_format_dscr.tSamFreq[4]);
#else
    LOG_MSGID_W(USBAUDIO_DRV, "USB_Aduio_Set_RX1_Alt3 USB_AUDIO_RX1_ALT3_ENABLE is not enable", 0);
#endif
}


void USB_Aduio_Set_RX2_Alt1(uint8_t smaple_rate_num, uint8_t smaple_size, uint8_t channel, uint32_t *sample_rate)
{
    audio2_spk_stream_format_dscr.bLength          = 0x08 + 3 * smaple_rate_num;
    audio2_spk_stream_format_dscr.bSamFreqType     = smaple_rate_num;
    audio2_spk_stream_format_dscr.bSubFrameSize    = smaple_size;
    audio2_spk_stream_format_dscr.bBitResolution   = smaple_size * 8;
    audio2_spk_stream_format_dscr.bNrChannels      = channel;

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Aduio_Set_RX2_Alt1 smaple_rate_num:%d smaple_size:%d channel:%d ", 3,
                smaple_rate_num, smaple_size, channel);

    uint8_t i;
    for(i=0; i<USB_AUDIO_DSCR_MAX_FREQ_NUM; i++) {
        audio2_spk_stream_format_dscr.tSamFreq[i].data[0] = (uint8_t)(sample_rate[i]);
        audio2_spk_stream_format_dscr.tSamFreq[i].data[1] = (uint8_t)(sample_rate[i] >> 8);
        audio2_spk_stream_format_dscr.tSamFreq[i].data[2] = (uint8_t)(sample_rate[i] >> 16);
    }

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Aduio_Set_RX2_Alt1 smaple_rate: %d %d %d %d %d", 5,
                audio2_spk_stream_format_dscr.tSamFreq[0],
                audio2_spk_stream_format_dscr.tSamFreq[1],
                audio2_spk_stream_format_dscr.tSamFreq[2],
                audio2_spk_stream_format_dscr.tSamFreq[3],
                audio2_spk_stream_format_dscr.tSamFreq[4]);
}

void USB_Aduio_Set_RX2_Alt2(bool alt2_en, uint8_t smaple_rate_num, uint8_t smaple_size, uint8_t channel, uint32_t *sample_rate)
{
#ifdef USB_AUDIO_RX2_ALT2_ENABLE
    g_UsbAudio[1].spk_alt2_en = alt2_en;

    if(alt2_en == false){
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Aduio_Set_RX2_Alt2 alt2_en is disable", 0);
        return;
    }

    audio2_spk_stream_alt2_format_dscr.bLength          = 0x08 + 3 * smaple_rate_num;
    audio2_spk_stream_alt2_format_dscr.bSamFreqType     = smaple_rate_num;
    audio2_spk_stream_alt2_format_dscr.bSubFrameSize    = smaple_size;
    audio2_spk_stream_alt2_format_dscr.bBitResolution   = smaple_size * 8;
    audio2_spk_stream_alt2_format_dscr.bNrChannels      = channel;

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Aduio_Set_RX2_Alt2 smaple_rate_num:%d smaple_size:%d channel:%d", 3,
                smaple_rate_num, smaple_size, channel);

    uint8_t i;
    for(i=0; i<USB_AUDIO_DSCR_MAX_FREQ_NUM; i++) {
        audio2_spk_stream_alt2_format_dscr.tSamFreq[i].data[0] = (uint8_t)(sample_rate[i]);
        audio2_spk_stream_alt2_format_dscr.tSamFreq[i].data[1] = (uint8_t)(sample_rate[i] >> 8);
        audio2_spk_stream_alt2_format_dscr.tSamFreq[i].data[2] = (uint8_t)(sample_rate[i] >> 16);
    }

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Aduio_Set_RX2_Alt2 smaple_rate: %d %d %d %d %d", 5,
                audio2_spk_stream_alt2_format_dscr.tSamFreq[0],
                audio2_spk_stream_alt2_format_dscr.tSamFreq[1],
                audio2_spk_stream_alt2_format_dscr.tSamFreq[2],
                audio2_spk_stream_alt2_format_dscr.tSamFreq[3],
                audio2_spk_stream_alt2_format_dscr.tSamFreq[4]);
    return;
#else
    LOG_MSGID_W(USBAUDIO_DRV, "USB_Aduio_Set_RX2_Alt2 USB_AUDIO_RX2_ALT2_ENABLE is not enable", 0);
#endif
}


void USB_Aduio_Set_TX1_Alt1(uint8_t smaple_rate_num, uint8_t smaple_size, uint8_t channel, uint32_t *sample_rate)
{
#if defined(AIR_USB_AUDIO_1_MIC_ENABLE)
    audio1_mic_stream_format_dscr.bLength          = 0x08 + 3 * smaple_rate_num;
    audio1_mic_stream_format_dscr.bSamFreqType     = smaple_rate_num;
    audio1_mic_stream_format_dscr.bSubFrameSize    = smaple_size;
    audio1_mic_stream_format_dscr.bBitResolution   = smaple_size * 8;
    audio1_mic_stream_format_dscr.bNrChannels      = channel;

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Aduio_Set_TX1_Alt1 smaple_rate_num:%d smaple_size:%d channel:%d", 3,
                smaple_rate_num, smaple_size, channel);

    uint8_t i;
    for(i=0; i<USB_AUDIO_DSCR_MAX_FREQ_NUM; i++) {
        audio1_mic_stream_format_dscr.tSamFreq[i].data[0] = (uint8_t)(sample_rate[i]);
        audio1_mic_stream_format_dscr.tSamFreq[i].data[1] = (uint8_t)(sample_rate[i] >> 8);
        audio1_mic_stream_format_dscr.tSamFreq[i].data[2] = (uint8_t)(sample_rate[i] >> 16);
    }

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Aduio_Set_TX1_Alt1 smaple_rate: %d %d %d %d %d", 5,
                audio1_mic_stream_format_dscr.tSamFreq[0],
                audio1_mic_stream_format_dscr.tSamFreq[1],
                audio1_mic_stream_format_dscr.tSamFreq[2],
                audio1_mic_stream_format_dscr.tSamFreq[3],
                audio1_mic_stream_format_dscr.tSamFreq[4]);
#else
    LOG_MSGID_W(USBAUDIO_DRV, "USB_Aduio_Set_TX1_Alt1 AIR_USB_AUDIO_1_MIC_ENABLE is not enable", 0);
#endif
}

void USB_Aduio_Set_TX1_Alt2(bool alt2_en, uint8_t smaple_rate_num, uint8_t smaple_size, uint8_t channel, uint32_t *sample_rate)
{
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
#ifdef USB_AUDIO_TX1_ALT2_ENABLE
    g_UsbAudio[0].mic_alt2_en = alt2_en;

    if(alt2_en == false){
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Aduio_Set_TX1_Alt2 alt2_en is disable", 0);
        return;
    }

    audio1_mic_stream_alt2_format_dscr.bLength          = 0x08 + 3 * smaple_rate_num;
    audio1_mic_stream_alt2_format_dscr.bSamFreqType     = smaple_rate_num;
    audio1_mic_stream_alt2_format_dscr.bSubFrameSize    = smaple_size;
    audio1_mic_stream_alt2_format_dscr.bBitResolution   = smaple_size * 8;
    audio1_mic_stream_alt2_format_dscr.bNrChannels      = channel;

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Aduio_Set_TX1_Alt2 smaple_rate_num:%d smaple_size:%d channel:%d", 3,
                smaple_rate_num, smaple_size, channel);

    uint8_t i;
    for(i=0; i<USB_AUDIO_DSCR_MAX_FREQ_NUM; i++) {
        audio1_mic_stream_alt2_format_dscr.tSamFreq[i].data[0] = (uint8_t)(sample_rate[i]);
        audio1_mic_stream_alt2_format_dscr.tSamFreq[i].data[1] = (uint8_t)(sample_rate[i] >> 8);
        audio1_mic_stream_alt2_format_dscr.tSamFreq[i].data[2] = (uint8_t)(sample_rate[i] >> 16);
    }

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Aduio_Set_TX1_Alt2 smaple_rate: %d %d %d %d %d", 5,
                audio1_mic_stream_alt2_format_dscr.tSamFreq[0],
                audio1_mic_stream_alt2_format_dscr.tSamFreq[1],
                audio1_mic_stream_alt2_format_dscr.tSamFreq[2],
                audio1_mic_stream_alt2_format_dscr.tSamFreq[3],
                audio1_mic_stream_alt2_format_dscr.tSamFreq[4]);
#else
    LOG_MSGID_W(USBAUDIO_DRV, "USB_Aduio_Set_TX1_Alt2 AIR_USB_AUDIO_1_MIC_ENABLE is not enable", 0);
#endif
#endif
}

void usb_audio_set_audio_card(uint32_t port, bool spk, bool mic)
{
    LOG_MSGID_I(USBAUDIO_DRV, "usb_audio_set_audio_card port:%d spk:%d mic:%d", 3, port, spk, mic);

    /* Set up status of enable/disable */
    g_UsbAudio[port].spk_feature_en = spk;
    g_UsbAudio[port].mic_feature_en = mic;

    /* based on AIR_USB_AUDIO_ENABLE, assert when feature option or APP setting is invalid */
    if((g_UsbAudio[USB_AUDIO_1_PORT].spk_feature_en == false) &&
       (g_UsbAudio[USB_AUDIO_1_PORT].mic_feature_en == false) &&
       (g_UsbAudio[USB_AUDIO_2_PORT].spk_feature_en == false)
    ) {
        assert(0 && "No setting of speaker/microphone was enabled by APP!");
    }

    if((g_UsbAudio[USB_AUDIO_1_PORT].spk_feature_en == false) &&
       (g_UsbAudio[USB_AUDIO_2_PORT].spk_feature_en == true)
    ) {
        assert(0 && "Speaker1 need to be used first!");
    }

    if(g_UsbAudio[USB_AUDIO_2_PORT].mic_feature_en == true) {
        assert(0 && "Not support microphone2!");
    }

#if !defined(AIR_USB_AUDIO_1_SPK_ENABLE) && !defined(AIR_USB_AUDIO_1_MIC_ENABLE) && !defined(AIR_USB_AUDIO_2_SPK_ENABLE)
    assert(0 && "Feature option: No speaker or microphone was enabled!");
#endif

#ifndef AIR_USB_AUDIO_1_SPK_ENABLE
    if((port == USB_AUDIO_1_PORT) && spk){
        assert(0 && "Feature option: Speaker1 was not enabled!");
    }
#endif

#ifndef AIR_USB_AUDIO_1_MIC_ENABLE
    if((port == USB_AUDIO_1_PORT) && mic){
        assert(0 && "Feature option: Microphone was not enabled!");
    }
#endif

#ifndef AIR_USB_AUDIO_2_SPK_ENABLE
    if((port == USB_AUDIO_2_PORT) && spk){
        assert(0 && "Feature option: Speaker2 was not enabled!");
    }
#endif

#if !defined(AIR_USB_AUDIO_1_SPK_ENABLE) && defined(AIR_USB_AUDIO_2_SPK_ENABLE)
    assert(0 && "Feature option: Speaker1 need to be used first!");
#endif

}

#define SUB_DEVICE_SPK (0x00)
#define SUB_DEVICE_MIC (0x01)

void usb_audio_set_spk_channels(uint32_t port, uint8_t chs, uint16_t chs_map, usb_audio_vc_t vc)
{
    g_UsbAudio[port].spk_chs = chs;
    g_UsbAudio[port].spk_chs_map = chs_map;
    g_UsbAudio[port].spk_vc = vc;
}

void usb_audio_set_mic_channels(uint32_t port, uint8_t chs, uint16_t chs_map, usb_audio_vc_t vc)
{
    g_UsbAudio[port].mic_chs = chs;
    g_UsbAudio[port].mic_chs_map = chs_map;
    g_UsbAudio[port].mic_vc = vc;
}

void usb_audio_set_terminal_type(
    uint32_t port,
    usb_audio_termt_t spk_terminal,
    usb_audio_termt_t mic_terminal)
{
    LOG_MSGID_I(USBAUDIO_DRV, "usb_audio_set_terminal_type port:%d spk:0x%04X mic:0x%04X", 3, port, spk_terminal, mic_terminal);

    g_UsbAudio[port].spk_terminal_type = spk_terminal;
    g_UsbAudio[port].mic_terminal_type = mic_terminal;
}

#define SUB_DEVICE_SPK (0x00)
#define SUB_DEVICE_MIC (0x01)

usb_audio_vc_t usb_audio_get_vc(uint32_t port, uint8_t sub_device)
{
    /* Illegal input */
    if (port >= 2) {
        LOG_MSGID_E(USBAUDIO_DRV, "usb_audio_get_vc illegal parameter port:%d >= 2 ", 1, port);
        assert(0 && "Illegal parameter, port or sub_device is not illegal.");
        return 0;
    }

    /* Speaker or Microphone */
    if(sub_device == SUB_DEVICE_SPK) {
        return g_UsbAudio[port].spk_vc;
    }
    else if(sub_device == SUB_DEVICE_MIC) {
        return g_UsbAudio[port].mic_vc;
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "usb_audio_get_vc sub_device:%d is not speaker nor microphone", 1, sub_device);
        assert(0 && "Illegal parameter, port or sub_device is not illegal .");
        return 0;
    }
}

/************************************************************
    Helper functions to generate descriptors
*************************************************************/

static void usb_audio_update_channel_config(uint8_t port, uint8_t sub_device)
{
    usb_audio_dscr_it_t *id;
    usb_audio_dscr_feature_t *fd;
    UsbAudio_Struct *audio_str;
    usb_audio_vc_t vc;

    audio_str = &g_UsbAudio[port];
    if(port == 0 && sub_device == SUB_DEVICE_SPK) {
        /* audio 1 spk */
        id = &audio1_spk_it_dscr;
        fd = &audio1_spk_feature_dscr;
        vc = g_UsbAudio[port].spk_vc;
    }
    else if(port == 0 && sub_device == SUB_DEVICE_MIC) {
        /* audio 1 mic */
        id = &audio1_mic_it_dscr;
        fd = &audio1_mic_feature_dscr;
        vc = g_UsbAudio[port].mic_vc;
    }
    else if(port == 1 && sub_device == SUB_DEVICE_SPK) {
        /* audio 2 spk */
        id = &audio2_spk_it_dscr;
        fd = &audio2_spk_feature_dscr;
        vc = g_UsbAudio[port].spk_vc;
    }
    else if(port == 1 && sub_device == SUB_DEVICE_MIC) {
        /* audio 2 mic */
        id = &audio2_mic_it_dscr;
        fd = &audio2_mic_feature_dscr;
        vc = g_UsbAudio[port].mic_vc;
    }
    else {
        assert(0 && "Illegal parm of port and sub_device");
        return;
    }

    if (sub_device == SUB_DEVICE_SPK) {
        /* Set channel configs of spk */
        id->bNrChannels         = audio_str->spk_chs;
        id->wChannelConfig      = audio_str->spk_chs_map;
        fd->control_nr          = audio_str->spk_chs + 1; /* add 1 master Control */
        fd->bLength             = 0x07 + fd->control_nr * fd->bControlSize;

        if(USB_AUDIO_VC_INDIVIUAL == vc) {
            /* Master(0) Channel */
            fd->bmaControls[0] = USB_AUDIO_BMACONTROL_MUTE;
            /* Each(1~n) Channel */
            for (uint8_t i = 0; i < g_UsbAudio[0].spk_chs; i++) {
                fd->bmaControls[i + 1] = USB_AUDIO_BMACONTROL_VOLUME;
            }
        }
        else if(USB_AUDIO_VC_MASTER == vc) {
            /* Master(0) Channel */
            fd->bmaControls[0] = USB_AUDIO_BMACONTROL_MUTE | USB_AUDIO_BMACONTROL_VOLUME;
            /* Each(1~n) Channel */
            for (uint8_t i = 0; i < g_UsbAudio[0].spk_chs; i++) {
                fd->bmaControls[i + 1] = USB_AUDIO_BMACONTROL_NULL;
            }
        }
        else if(USB_AUDIO_VC_BOTH == vc) {
            /* Master(0) Channel */
            fd->bmaControls[0] = USB_AUDIO_BMACONTROL_MUTE | USB_AUDIO_BMACONTROL_VOLUME;
            /* Each(1~n) Channel */
            for (uint8_t i = 0; i < g_UsbAudio[0].spk_chs; i++) {
                fd->bmaControls[i + 1] = USB_AUDIO_BMACONTROL_VOLUME;
            }
        }
        else {
            assert(0 && "Illeagal volume control method(vc)");
        }
    }
    else if (sub_device == SUB_DEVICE_MIC) {
        /* Set channel configs of mic */
        id->bNrChannels         = audio_str->mic_chs;
        id->wChannelConfig      = audio_str->mic_chs_map;
        fd->control_nr          = audio_str->mic_chs + 1; /* add 1 master Control */
        fd->bLength             = 0x07 + fd->control_nr * fd->bControlSize;

        if(USB_AUDIO_VC_INDIVIUAL == vc) {
            /* Master(0) Channel */
            fd->bmaControls[0] = USB_AUDIO_BMACONTROL_MUTE;
            /* Each(1~n) Channel */
            for (uint8_t i = 0; i < g_UsbAudio[0].mic_chs; i++) {
                fd->bmaControls[i + 1] = USB_AUDIO_BMACONTROL_VOLUME;
            }
        }
        else if(USB_AUDIO_VC_MASTER == vc) {
            /* Master(0) Channel */
            fd->bmaControls[0] = USB_AUDIO_BMACONTROL_MUTE | USB_AUDIO_BMACONTROL_VOLUME;
            /* Each(1~n) Channel */
            for (uint8_t i = 0; i < g_UsbAudio[0].mic_chs; i++) {
                fd->bmaControls[i + 1] = USB_AUDIO_BMACONTROL_NULL;
            }
        }
        else if(USB_AUDIO_VC_BOTH == vc) {
            /* Master(0) Channel */
            fd->bmaControls[0] = USB_AUDIO_BMACONTROL_MUTE | USB_AUDIO_BMACONTROL_VOLUME;
            /* Each(1~n) Channel */
            for (uint8_t i = 0; i < g_UsbAudio[0].mic_chs; i++) {
                fd->bmaControls[i + 1] = USB_AUDIO_BMACONTROL_VOLUME;
            }
        }
        else {
            assert(0 && "Illeagal volume control method(vc)");
        }
    }
}

/************************************************************
    Interface initialization functions
*************************************************************/
/* Audio control interface create function, prepare descriptor */
void USB_Audio_ControlIf_Create(uint32_t port, void *ifname)
{
}

void USB_Audio1_ControlIf_Create(void *ifname)
{
    bool spk_en;
    bool mic_en;
    uint8_t ac_if_id;
    uint8_t spk_if_id;
    uint8_t spk_ep_id;
    uint8_t mic_if_id;
    uint8_t mic_ep_id;
    int16_t ach_length = 0;
    int16_t len        = 0;
    uint8_t *dscr_out;
    uint8_t port = 0;

    spk_en = g_UsbAudio[0].spk_feature_en;
    mic_en = g_UsbAudio[0].mic_feature_en;

    g_UsbAudio[0].control_if_info      = USB_Get_Interface_Number(&ac_if_id);
    g_UsbAudio[0].control_interface_id = ac_if_id;

    if (spk_en) {
        g_UsbAudio[0].stream_if_info      = USB_Get_Interface_Number(&spk_if_id);
        g_UsbAudio[0].stream_interface_id = spk_if_id;
        g_UsbAudio[0].stream_ep_out_info  = USB_Get_Iso_Rx_Ep(&spk_ep_id);
        g_UsbAudio[0].stream_ep_out_id    = spk_ep_id;
    }

    if (mic_en) {
        g_UsbAudio[0].stream_microphone_if_info      = USB_Get_Interface_Number(&mic_if_id);
        g_UsbAudio[0].stream_microphone_interface_id = mic_if_id;
        g_UsbAudio[0].stream_ep_in_info              = USB_Get_Iso_Tx_Ep(&mic_ep_id);
        g_UsbAudio[0].stream_ep_in_id                = mic_ep_id;
    }

    audio1_interface_dscr.bInterfaceNumber = g_UsbAudio[0].control_interface_id;
    audio1_interface_dscr.iInterface       = USB_String_Get_Id_By_Usage(USB_STRING_USAGE_AUDIO1);
    audio1_spk_ot_dscr.wTerminalType       = g_UsbAudio[0].spk_terminal_type;
    audio1_mic_it_dscr.wTerminalType       = g_UsbAudio[0].mic_terminal_type;

    usb_audio_update_channel_config(port, SUB_DEVICE_SPK);
    usb_audio_update_channel_config(port, SUB_DEVICE_MIC);

    audio1_ach_dscr.bInCollection = 0;
    if (spk_en) {
        audio1_ach_dscr.baInterfaceNr[0] = g_UsbAudio[0].stream_interface_id;
        audio1_ach_dscr.bInCollection++;
    }
    if (mic_en) {
        audio1_ach_dscr.baInterfaceNr[1] = g_UsbAudio[0].stream_microphone_interface_id;
        audio1_ach_dscr.bInCollection++;
    }
    audio1_ach_dscr.bLength = 8 + audio1_ach_dscr.bInCollection;

    /* calculate total length of AC Header */
    ach_length += audio1_ach_dscr.bLength;
    if (spk_en) {
        for (int i = 0; i < sizeof(audio1_spk_dscrs) / sizeof(usb_audio_dscr_hdlr_t); i++) {
            /* offset 0 is always length of dscr */
            ach_length += ((uint8_t *)(audio1_spk_dscrs[i].dscr))[0];
        }
    }
    if (mic_en) {
        for (int i = 0; i < sizeof(audio1_mic_dscrs) / sizeof(usb_audio_dscr_hdlr_t); i++) {
            /* offset 0 is always length of dscr */
            ach_length += ((uint8_t *)(audio1_mic_dscrs[i].dscr))[0];
        }
    }
    audio1_ach_dscr.wTotalLength = ach_length;

    /* write dscr to usb interface struct */
    dscr_out = g_UsbAudio[0].control_if_info->ifdscr.classif;
    len += dscr_list_serialize(audio1_aci_dscrs, sizeof(audio1_aci_dscrs) / sizeof(usb_audio_dscr_hdlr_t), dscr_out + len, 1024);
    if (spk_en) {
        len += dscr_list_serialize(audio1_spk_dscrs, sizeof(audio1_spk_dscrs) / sizeof(usb_audio_dscr_hdlr_t), dscr_out + len, 1024);
    }
    if (mic_en) {
        len += dscr_list_serialize(audio1_mic_dscrs, sizeof(audio1_mic_dscrs) / sizeof(usb_audio_dscr_hdlr_t), dscr_out + len, 1024);
    }
    g_UsbAudio[0].control_if_info->ifdscr_size = len;

    /* check if length if correct. If not, assert! */
    if (len != audio1_interface_dscr.bLength + audio1_ach_dscr.wTotalLength) {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio1_ControlIf_Create the length of descriptor is error:%d != %d, assert!",
                    2, len, audio1_interface_dscr.bLength + audio1_ach_dscr.wTotalLength);
        assert(0);
    }

    /* register interface callback */
    g_UsbAudio[0].control_if_info->if_class_specific_hdlr = USB_Audio1_Ep0_Command;
}

void USB_Audio2_ControlIf_Create(void *ifname)
{
    bool spk_en;
    bool mic_en;
    uint8_t ac_if_id;
    uint8_t spk_if_id;
    uint8_t spk_ep_id;
    uint8_t mic_if_id;
    uint8_t mic_ep_id;
    int16_t ach_length = 0;
    int16_t len        = 0;
    void *dscr_out;
    uint8_t port = 1;

    spk_en = g_UsbAudio[1].spk_feature_en;
    mic_en = g_UsbAudio[1].mic_feature_en;

    g_UsbAudio[1].control_if_info      = USB_Get_Interface_Number(&ac_if_id);
    g_UsbAudio[1].control_interface_id = ac_if_id;

    if (spk_en) {
        g_UsbAudio[1].stream_if_info      = USB_Get_Interface_Number(&spk_if_id);
        g_UsbAudio[1].stream_interface_id = spk_if_id;
        g_UsbAudio[1].stream_ep_out_info  = USB_Get_Iso_Rx_Ep(&spk_ep_id);
        g_UsbAudio[1].stream_ep_out_id    = spk_ep_id;
    }

    if (mic_en) {
        g_UsbAudio[1].stream_microphone_if_info      = USB_Get_Interface_Number(&mic_if_id);
        g_UsbAudio[1].stream_microphone_interface_id = mic_if_id;
        g_UsbAudio[1].stream_ep_in_info              = USB_Get_Iso_Tx_Ep(&mic_ep_id);
        g_UsbAudio[1].stream_ep_in_id                = mic_ep_id;
    }

    audio2_interface_dscr.bInterfaceNumber = g_UsbAudio[1].control_interface_id;
    audio2_interface_dscr.iInterface       = USB_String_Get_Id_By_Usage(USB_STRING_USAGE_AUDIO2);
    audio2_spk_ot_dscr.wTerminalType       = g_UsbAudio[1].spk_terminal_type;

    usb_audio_update_channel_config(port, SUB_DEVICE_SPK);
    usb_audio_update_channel_config(port, SUB_DEVICE_MIC);

    audio2_ach_dscr.bInCollection = 0;
    if (spk_en) {
        audio2_ach_dscr.baInterfaceNr[0] = g_UsbAudio[1].stream_interface_id;
        audio2_ach_dscr.bInCollection++;
    }
    if (mic_en) {
        audio2_ach_dscr.baInterfaceNr[1] = g_UsbAudio[1].stream_microphone_interface_id;
        audio2_ach_dscr.bInCollection++;
    }
    audio2_ach_dscr.bLength = 8 + audio2_ach_dscr.bInCollection;

    /* calculate total length of audio2 AC Header */
    ach_length += audio2_ach_dscr.bLength;
    if (spk_en) {
        for (int i = 0; i < sizeof(audio2_spk_dscrs) / sizeof(usb_audio_dscr_hdlr_t); i++) {
            /* offset 0 is always length of dscr */
            ach_length += ((uint8_t *)(audio2_spk_dscrs[i].dscr))[0];
        }
    }
    if (mic_en) {
        for (int i = 0; i < sizeof(audio2_mic_dscrs) / sizeof(usb_audio_dscr_hdlr_t); i++) {
            /* offset 0 is always length of dscr */
            ach_length += ((uint8_t *)(audio2_mic_dscrs[i].dscr))[0];
        }
    }
    audio2_ach_dscr.wTotalLength = ach_length;

    /* write dscr to usb interface struct */
    dscr_out = g_UsbAudio[1].control_if_info->ifdscr.classif;
    len += dscr_list_serialize(audio2_aci_dscrs, sizeof(audio2_aci_dscrs) / sizeof(usb_audio_dscr_hdlr_t), dscr_out + len, 1024);
    if (spk_en) {
        len += dscr_list_serialize(audio2_spk_dscrs, sizeof(audio2_spk_dscrs) / sizeof(usb_audio_dscr_hdlr_t), dscr_out + len, 1024);
    }
    if (mic_en) {
        len += dscr_list_serialize(audio2_mic_dscrs, sizeof(audio2_mic_dscrs) / sizeof(usb_audio_dscr_hdlr_t), dscr_out + len, 1024);
    }
    g_UsbAudio[1].control_if_info->ifdscr_size = len;

    /* check if length if correct. If not, assert! */
    if (len != audio2_interface_dscr.bLength + audio2_ach_dscr.wTotalLength) {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio2_ControlIf_Create the length of descriptor is error:%d != %d, assert!",
                    2, len, audio2_interface_dscr.bLength + audio2_ach_dscr.wTotalLength);
        assert(0);
    }

    /* register interface callback */
    g_UsbAudio[1].control_if_info->if_class_specific_hdlr = USB_Audio2_Ep0_Command;
}

void USB_Audio_ControlIf_Reset(uint32_t port)
{
}

void USB_Audio1_ControlIf_Reset(void)
{
}

void USB_Audio2_ControlIf_Reset(void)
{
}

void USB_Audio_ControlIf_Enable(uint32_t port)
{
}

void USB_Audio1_ControlIf_Enable(void)
{
}
void USB_Audio2_ControlIf_Enable(void)
{
}

/* Audio stream interface speed reset function, enable EP's speed-specific descriptor */
void USB_Audio_ControlIf_Speed_Reset(uint32_t port, bool b_other_speed)
{
}

void USB_Audio1_ControlIf_Speed_Reset(bool b_other_speed)
{
}

void USB_Audio2_ControlIf_Speed_Reset(bool b_other_speed)
{
}

/* Audio stream interface create function, prepare descriptor */
void USB_Audio_StreamIf_Create(uint32_t port, void *ifname)
{
}

void USB_Audio1_StreamIf_Create(void *ifname)
{
    uint8_t if_id, ep_rx_id;
    uint8_t port = 0;
    uint8_t *dscr_out;
    int16_t len;

    if(g_UsbAudio[port].spk_feature_en == false) {
        return;
    }

    if_id = g_UsbAudio[port].stream_interface_id;
    ep_rx_id = g_UsbAudio[port].stream_ep_out_id;

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_StreamIf_Create port:%d if_id:0x%X ep_rx_id:0x%X", 3,
                port, g_UsbAudio[port].stream_interface_id, g_UsbAudio[port].stream_ep_out_id);

    audio1_spk_stream_if_dscr.bInterfaceNumber = if_id;
    audio1_spk_stream_alt_if_dscr.bInterfaceNumber = if_id;
    audio1_spk_stream_general_dscr.bTerminalLink = audio1_spk_it_dscr.bTerminalID;

    /* generate audio stream interface */
    g_UsbAudio[port].stream_if_info->interface_name_ptr = (char *)ifname;

    dscr_out = g_UsbAudio[port].stream_if_info->ifdscr.classif;
    len = dscr_list_serialize(audio1_spk_as_if_dscrs, sizeof(audio1_spk_as_if_dscrs) / sizeof(usb_audio_dscr_hdlr_t), dscr_out, USB_MAX_IFDSC_LENGTH);
    g_UsbAudio[port].stream_if_info->ifdscr_size = len;

    /* generate audio stream alternate interface */
    dscr_out = g_UsbAudio[port].stream_if_info->alternate_if_info[0].ifdscr.classif;
    len      = dscr_list_serialize(audio1_spk_as_if_alt_dscrs, sizeof(audio1_spk_as_if_alt_dscrs) / sizeof(usb_audio_dscr_hdlr_t), dscr_out, USB_MAX_IFDSC_LENGTH);
    g_UsbAudio[port].stream_if_info->alternate_if_info[0].ifdscr_size = len;

    /* generate ep of alternate interface */
    g_UsbAudio[port].stream_if_info->alternate_if_info[0].ep_info[0] = g_UsbAudio[port].stream_ep_out_info;
    g_UsbAudio[port].stream_if_info->alternate_if_info[0].ep_info[0]->epdscr_size = sizeof(audio_stream_ep_out_dscr);

#ifdef USB_AUDIO_RX1_ALT2_ENABLE
    if(g_UsbAudio[port].spk_alt2_en) {
        audio1_spk_stream_alt2_if_dscr.bInterfaceNumber = if_id;

        /* generate audio stream alternate interface */
        dscr_out = g_UsbAudio[port].stream_if_info->alternate_if_info[1].ifdscr.classif;
        len      = dscr_list_serialize(audio1_spk_as_if_alt2_dscrs, sizeof(audio1_spk_as_if_alt2_dscrs) / sizeof(usb_audio_dscr_hdlr_t), dscr_out, USB_MAX_IFDSC_LENGTH);
        g_UsbAudio[port].stream_if_info->alternate_if_info[1].ifdscr_size = len;

        /* generate ep of alternate interface */
        g_UsbAudio[port].stream_if_info->alternate_if_info[1].ep_info[0] = g_UsbAudio[port].stream_ep_out_info;
        g_UsbAudio[port].stream_if_info->alternate_if_info[1].ep_info[0]->epdscr_size = sizeof(audio_stream_ep_out_dscr);
    }
#endif

#ifdef USB_AUDIO_RX1_ALT3_ENABLE
    if(g_UsbAudio[port].spk_alt3_en) {
        audio1_spk_stream_alt3_if_dscr.bInterfaceNumber = if_id;

        /* generate audio stream alternate interface */
        dscr_out = g_UsbAudio[port].stream_if_info->alternate_if_info[2].ifdscr.classif;
        len      = dscr_list_serialize(audio1_spk_as_if_alt3_dscrs, sizeof(audio1_spk_as_if_alt3_dscrs) / sizeof(usb_audio_dscr_hdlr_t), dscr_out, USB_MAX_IFDSC_LENGTH);
        g_UsbAudio[port].stream_if_info->alternate_if_info[2].ifdscr_size = len;

        /* generate ep of alternate interface */
        g_UsbAudio[port].stream_if_info->alternate_if_info[2].ep_info[0] = g_UsbAudio[port].stream_ep_out_info;
        g_UsbAudio[port].stream_if_info->alternate_if_info[2].ep_info[0]->epdscr_size = sizeof(audio_stream_ep_out_dscr);
    }
#endif

    memcpy((uint32_t *) & (g_UsbAudio[port].stream_ep_out_info->epdesc.classep), audio_stream_ep_out_dscr, sizeof(audio_stream_ep_out_dscr));

    g_UsbAudio[port].stream_if_info->if_class_specific_hdlr = USB_Audio1_Ep0_Command; /* Command: "SetCur" for "Sampling freq".*/

    /* RX Endpoint handler */
    g_UsbAudio[port].stream_ep_out_info->ep_reset = USB_Audio1_IsoOut_Reset;
    hal_usb_register_driver_callback(HAL_USB_DRV_HDLR_EP_RX, ep_rx_id, USB_Audio1_IsoOut_Hdr);

    /* Endpoint descriptor */
    g_UsbAudio[port].stream_ep_out_info->epdesc.stdep.bEndpointAddress = (USB_EP_DIR_OUT | ep_rx_id); /*OutPipe*/
    g_UsbAudio[port].stream_ep_out_info->ep_status.epout_status.byEP = ep_rx_id;

    /* Ep_id include direction */
    USB_Audio_Set_Chat_Info(if_id, (USB_EP_DIR_OUT | ep_rx_id), USB_AUDIO_UNUSED_ID, USB_AUDIO_UNUSED_ID);

    if (g_UsbAudio[port].rx_buffer == NULL) {
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_StreamIf_Create port:%d rx buffer size:%d", 2, port, USB_AUDIO_RX_BUFFER_SIZE);
        g_UsbAudio[port].rx_buffer = (uint8_t *)USB_Get_Memory(USB_AUDIO_RX_BUFFER_SIZE);
        g_UsbAudio[port].rx_buffer_len = USB_AUDIO_RX_BUFFER_SIZE;
    }

    hal_usb_get_dma_channel(0, ep_rx_id, HAL_USB_EP_DIRECTION_RX, false);

    /* Create IAD descriptor, if need it. e.g. (Audio + HID + CDC) */
    if (gUsbDevice.use_iad == true) {
        g_UsbAudio[port].iad_desc = USB_Get_IAD_Number();
        Usb_IAD_Dscr *iad_desc = g_UsbAudio[port].iad_desc;

        iad_desc->bLength           = USB_IAD_LENGTH;
        iad_desc->bDescriptorType   = USB_INTERFACE_ASSOCIATION;
        iad_desc->bFirstInterface   = g_UsbAudio[port].control_interface_id;
        iad_desc->bInterfaceCount   = 1 + (uint8_t)g_UsbAudio[port].spk_feature_en + (uint8_t)g_UsbAudio[port].mic_feature_en;
        iad_desc->bFunctionClass    = USB_IF_CLASS_AUDIO;
        iad_desc->bFunctionSubClass = USB_AUDIO_SUBCLASS_AUDIOCONTROL;
        iad_desc->bFunctionProtocol = USB_AUDIO_PROTOCOL_UNDEFINED;
        iad_desc->iFunction         = 0x00;
    }

    /* SYNC volume to APP/Audio module */
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_StreamIf_Create sync volume to app layer, ch:%d", 1, g_UsbAudio[0].spk_chs);
    uint8_t ch;

    if(g_UsbAudio[0].spk_chs == 0) {
        /* Master Volume*/
        USB_Send_Message(USB_AUDIO_SET_VOLUME, (void *)(g_UsbAudio[0].spk_cur+ (1 << 16) + ((USB_EP_DIR_OUT | g_UsbAudio[0].stream_ep_out_id) << 24)));
    } else {
        for(ch=1; ch<=(g_UsbAudio[0].spk_chs); ch++){
            USB_Send_Message(USB_AUDIO_SET_VOLUME, (void *)(g_UsbAudio[0].spk_cur + (ch << 16) + ((USB_EP_DIR_OUT | g_UsbAudio[0].stream_ep_out_id) << 24)));
        }
    }
}

void USB_Audio2_StreamIf_Create(void *ifname)
{
    uint8_t if_id, ep_rx_id;
    uint8_t port = 1;
    uint8_t *dscr_out;
    int16_t len;

    if(g_UsbAudio[port].spk_feature_en == false) {
        return;
    }

    if_id = g_UsbAudio[port].stream_interface_id;
    ep_rx_id = g_UsbAudio[port].stream_ep_out_id;

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio2_StreamIf_Create port:%d if_id:0x%X ep_rx_id:0x%X", 3,
                port, g_UsbAudio[port].stream_interface_id, g_UsbAudio[port].stream_ep_out_id);

    audio2_spk_stream_if_dscr.bInterfaceNumber = if_id;
    audio2_spk_stream_alt_if_dscr.bInterfaceNumber = if_id;
    audio2_spk_stream_general_dscr.bTerminalLink = audio2_spk_it_dscr.bTerminalID;

    /* generate audio stream interface */
    g_UsbAudio[port].stream_if_info->interface_name_ptr = (char *)ifname;

    dscr_out = g_UsbAudio[port].stream_if_info->ifdscr.classif;
    len = dscr_list_serialize(audio2_spk_as_if_dscrs, sizeof(audio2_spk_as_if_dscrs) / sizeof(usb_audio_dscr_hdlr_t), dscr_out, USB_MAX_IFDSC_LENGTH);
    g_UsbAudio[port].stream_if_info->ifdscr_size = len;

    /* generate audio stream alternate interface */
    dscr_out = g_UsbAudio[port].stream_if_info->alternate_if_info[0].ifdscr.classif;
    len      = dscr_list_serialize(audio2_spk_as_if_alt_dscrs, sizeof(audio2_spk_as_if_alt_dscrs) / sizeof(usb_audio_dscr_hdlr_t), dscr_out, USB_MAX_IFDSC_LENGTH);
    g_UsbAudio[port].stream_if_info->alternate_if_info[0].ifdscr_size = len;

    /* generate ep of alternate interface */
    g_UsbAudio[port].stream_if_info->alternate_if_info[0].ep_info[0] = g_UsbAudio[port].stream_ep_out_info;
    g_UsbAudio[port].stream_if_info->alternate_if_info[0].ep_info[0]->epdscr_size = sizeof(audio_stream_ep_out_dscr);

#ifdef USB_AUDIO_RX2_ALT2_ENABLE
    if(g_UsbAudio[port].spk_alt2_en) {
        audio2_spk_stream_alt2_if_dscr.bInterfaceNumber = if_id;

        /* generate audio stream alternate interface */
        dscr_out = g_UsbAudio[port].stream_if_info->alternate_if_info[1].ifdscr.classif;
        len      = dscr_list_serialize(audio2_spk_as_if_alt2_dscrs, sizeof(audio2_spk_as_if_alt2_dscrs) / sizeof(usb_audio_dscr_hdlr_t), dscr_out, USB_MAX_IFDSC_LENGTH);
        g_UsbAudio[port].stream_if_info->alternate_if_info[1].ifdscr_size = len;

        /* generate ep of alternate interface */
        g_UsbAudio[port].stream_if_info->alternate_if_info[1].ep_info[0] = g_UsbAudio[port].stream_ep_out_info;
        g_UsbAudio[port].stream_if_info->alternate_if_info[1].ep_info[0]->epdscr_size = sizeof(audio_stream_ep_out_dscr);
    }
#endif

    memcpy((uint32_t *) & (g_UsbAudio[port].stream_ep_out_info->epdesc.classep), audio_stream_ep_out_dscr, sizeof(audio_stream_ep_out_dscr));

    g_UsbAudio[port].stream_if_info->if_class_specific_hdlr = USB_Audio2_Ep0_Command; /* Command: "SetCur" for "Sampling freq".*/

    /* RX Endpoint handler */
    g_UsbAudio[port].stream_ep_out_info->ep_reset = USB_Audio2_IsoOut_Reset;
    hal_usb_register_driver_callback(HAL_USB_DRV_HDLR_EP_RX, ep_rx_id, USB_Audio2_IsoOut_Hdr);

    /* Endpoint descriptor */
    g_UsbAudio[port].stream_ep_out_info->epdesc.stdep.bEndpointAddress = (USB_EP_DIR_OUT | ep_rx_id); /*OutPipe*/
    g_UsbAudio[port].stream_ep_out_info->ep_status.epout_status.byEP = ep_rx_id;

    /* Ep_id include direction */
    USB_Audio_Set_Game_Info(if_id, (USB_EP_DIR_OUT | ep_rx_id));

    if (g_UsbAudio[port].rx_buffer == NULL) {
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio2_StreamIf_Create port:%d rx buffer size:%d", 2, port, USB_AUDIO_RX_BUFFER_SIZE);
        g_UsbAudio[port].rx_buffer = (uint8_t *)USB_Get_Memory(USB_AUDIO_RX_BUFFER_SIZE);
        g_UsbAudio[port].rx_buffer_len = USB_AUDIO_RX_BUFFER_SIZE;
    }

    hal_usb_get_dma_channel(0, ep_rx_id, HAL_USB_EP_DIRECTION_RX, false);

    /* Create IAD descriptor, if need it. e.g. (Audio + HID + CDC) */
    if (gUsbDevice.use_iad == true) {
        g_UsbAudio[port].iad_desc = USB_Get_IAD_Number();
        Usb_IAD_Dscr *iad_desc = g_UsbAudio[port].iad_desc;

        iad_desc->bLength           = USB_IAD_LENGTH;
        iad_desc->bDescriptorType   = USB_INTERFACE_ASSOCIATION;
        iad_desc->bFirstInterface   = g_UsbAudio[port].control_interface_id;
        iad_desc->bInterfaceCount   = 1 + (uint8_t)g_UsbAudio[port].spk_feature_en + (uint8_t)g_UsbAudio[port].mic_feature_en;
        iad_desc->bFunctionClass    = USB_IF_CLASS_AUDIO;
        iad_desc->bFunctionSubClass = USB_AUDIO_SUBCLASS_AUDIOCONTROL;
        iad_desc->bFunctionProtocol = USB_AUDIO_PROTOCOL_UNDEFINED;
        iad_desc->iFunction         = 0x00;
    }

    /* SYNC volume to APP/Audio module */
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio2_StreamIf_Create sync volume to app layer, ch:%d", 1, g_UsbAudio[1].spk_chs);
    uint8_t ch;

    if(g_UsbAudio[1].spk_chs == 0) {
        /* Master Volume*/
        USB_Send_Message(USB_AUDIO_SET_VOLUME, (void *)(g_UsbAudio[1].spk_cur+ (1 << 16) + ((USB_EP_DIR_OUT | g_UsbAudio[1].stream_ep_out_id) << 24)));
    } else {
        for(ch=1; ch<=(g_UsbAudio[1].spk_chs); ch++){
            USB_Send_Message(USB_AUDIO_SET_VOLUME, (void *)(g_UsbAudio[1].spk_cur + (ch << 16) + ((USB_EP_DIR_OUT | g_UsbAudio[1].stream_ep_out_id) << 24)));
        }
    }
}

/* Audio stream interface reset function, enable EP */
void USB_Audio_StreamIf_Reset(uint32_t port)
{
    if(g_UsbAudio[port].spk_feature_en == false) {
        return;
    }
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_StreamIf_Reset port:%d", 1, port);

    g_UsbAudio[port].rxpipe = &g_UsbAudio[port].stream_ep_out_info->ep_status.epout_status;

    /* Stop DMA */
    hal_usb_stop_dma_channel(g_UsbAudio[port].rxpipe->byEP, HAL_USB_EP_DIRECTION_RX);
}

void USB_Audio1_StreamIf_Reset(void)
{
    USB_Audio_StreamIf_Reset(0);
}
void USB_Audio2_StreamIf_Reset(void)
{
    USB_Audio_StreamIf_Reset(1);
}

void USB_Audio_StreamIf_Enable(uint32_t port)
{
    if(g_UsbAudio[port].spk_feature_en == false) {
        return;
    }
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_StreamIf_Enable port:%d", 1, port);

    /* Stop DMA */
    /* Compliance test tool will set configuration , but no reset */
    hal_usb_stop_dma_channel(g_UsbAudio[port].rxpipe->byEP, HAL_USB_EP_DIRECTION_RX);

    /*Non-DMA*/
    hal_usb_enable_rx_endpoint(g_UsbAudio[port].rxpipe->byEP, HAL_USB_EP_TRANSFER_ISO, HAL_USB_EP_USE_NO_DMA, true);
}
void USB_Audio1_StreamIf_Enable(void)
{
    USB_Audio_StreamIf_Enable(0);
}

void USB_Audio2_StreamIf_Enable(void)
{
    USB_Audio_StreamIf_Enable(1);
}

extern bool g_USB_dynamic_fifo;

/* Audio stream interface speed reset function, enable EP's speed-specific descriptor */
void USB_Audio_StreamIf_Speed_Reset(uint32_t port, bool b_other_speed)
{
    uint32_t temp_max_size;

    if(g_UsbAudio[port].spk_feature_en == false) {
        return;
    }

    if( g_USB_dynamic_fifo ) {
        temp_max_size = 0;
    } else {
        temp_max_size = USB_Audio_Stream_Speed_Reset_Packet_Size(port, (bool)b_other_speed);
    }

    g_UsbAudio[port].stream_ep_out_info->epdesc.stdep.wMaxPacketSize[0] = temp_max_size & 0xff;
    g_UsbAudio[port].stream_ep_out_info->epdesc.stdep.wMaxPacketSize[1] = (temp_max_size >> 8) & 0xff;

    if (hal_usb_is_high_speed() == true) {
        g_UsbAudio[port].stream_ep_out_info->epdesc.stdep.bInterval = 4; /* (2^(4-1))*0.125us=1ms*/
    } else {
        g_UsbAudio[port].stream_ep_out_info->epdesc.stdep.bInterval = 1; /* (2^(1-1))*1ms=1ms*/
    }

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_StreamIf_Speed_Reset port:%d wMaxPacketSize:%d b_other_speed=%d", 3, port, temp_max_size, b_other_speed);
}

void USB_Audio1_StreamIf_Speed_Reset(bool b_other_speed)
{
    USB_Audio_StreamIf_Speed_Reset(0, b_other_speed);
}

void USB_Audio2_StreamIf_Speed_Reset(bool b_other_speed)
{
    USB_Audio_StreamIf_Speed_Reset(1, b_other_speed);
}

#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
/* Audio stream interface create function, prepare descriptor */
void USB_Audio_StreamIf_Microphone_Create(void *ifname)
{
    uint8_t if_id, ep_tx_id;
    uint8_t port = 0;
    uint8_t *dscr_out;
    int16_t len;

    if(g_UsbAudio[port].mic_feature_en == false) {
        return;
    }

    if_id = g_UsbAudio[port].stream_microphone_interface_id;
    ep_tx_id = g_UsbAudio[port].stream_ep_in_id;

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_StreamIf_Microphone_Create port:0x%X if_id:0x%X ep_tx_id:0x%X", 3,
                port, g_UsbAudio[port].stream_microphone_interface_id, g_UsbAudio[port].stream_ep_in_id);

    audio1_mic_stream_if_dscr.bInterfaceNumber = if_id;
    audio1_mic_stream_alt_if_dscr.bInterfaceNumber = if_id;
    audio1_mic_stream_general_dscr.bTerminalLink = audio1_mic_ot_dscr.bTerminalID;

    /* generate audio stream interface */
    g_UsbAudio[port].stream_microphone_if_info->interface_name_ptr = (char *)ifname;
    dscr_out = g_UsbAudio[port].stream_microphone_if_info->ifdscr.classif;
    len = dscr_list_serialize(audio1_mic_as_if_dscrs, sizeof(audio1_mic_as_if_dscrs) / sizeof(usb_audio_dscr_hdlr_t), dscr_out, USB_MAX_IFDSC_LENGTH);
    g_UsbAudio[port].stream_microphone_if_info->ifdscr_size = len;

    /* generate audio stream alternate interface */
    dscr_out = g_UsbAudio[port].stream_microphone_if_info->alternate_if_info[0].ifdscr.classif;
    len      = dscr_list_serialize(audio1_mic_as_if_alt_dscrs, sizeof(audio1_mic_as_if_alt_dscrs) / sizeof(usb_audio_dscr_hdlr_t), dscr_out, USB_MAX_IFDSC_LENGTH);
    g_UsbAudio[port].stream_microphone_if_info->alternate_if_info[0].ifdscr_size = len;

    /* generate ep of alternate interface */
    g_UsbAudio[port].stream_microphone_if_info->alternate_if_info[0].ep_info[0] = g_UsbAudio[port].stream_ep_in_info;
    g_UsbAudio[port].stream_microphone_if_info->alternate_if_info[0].ep_info[0]->epdscr_size = sizeof(audio_stream_ep_in_dscr);

#ifdef USB_AUDIO_TX1_ALT2_ENABLE
    if(g_UsbAudio[port].mic_alt2_en) {
        audio1_mic_stream_alt2_if_dscr.bInterfaceNumber = if_id;

        /* generate audio stream alternate interface */
        dscr_out = g_UsbAudio[port].stream_microphone_if_info->alternate_if_info[1].ifdscr.classif;
        len      = dscr_list_serialize(audio1_mic_as_if_alt2_dscrs, sizeof(audio1_mic_as_if_alt2_dscrs) / sizeof(usb_audio_dscr_hdlr_t), dscr_out, USB_MAX_IFDSC_LENGTH);
        g_UsbAudio[port].stream_microphone_if_info->alternate_if_info[1].ifdscr_size = len;

        /* generate ep of alternate interface */
        g_UsbAudio[port].stream_microphone_if_info->alternate_if_info[1].ep_info[0] = g_UsbAudio[port].stream_ep_in_info;
        g_UsbAudio[port].stream_microphone_if_info->alternate_if_info[1].ep_info[0]->epdscr_size = sizeof(audio_stream_ep_in_dscr);
    }
#endif

    memcpy((uint32_t *) & (g_UsbAudio[port].stream_ep_in_info->epdesc.classep), audio_stream_ep_in_dscr, sizeof(audio_stream_ep_in_dscr));

    g_UsbAudio[port].stream_microphone_if_info->if_class_specific_hdlr = USB_Audio1_Ep0_Command; /* Command: "SetCur" for "Sampling freq".*/

    /* TX Endpoint handler */
    g_UsbAudio[port].stream_ep_in_info->ep_reset = USB_Audio_IsoIn_Reset;
    hal_usb_register_driver_callback(HAL_USB_DRV_HDLR_EP_TX, ep_tx_id, USB_Audio_IsoIn_Hdr);

    /* Endpoint descriptor */
    g_UsbAudio[port].stream_ep_in_info->epdesc.stdep.bEndpointAddress = (USB_EP_DIR_IN | ep_tx_id); /*InPipe*/
    g_UsbAudio[port].stream_ep_in_info->ep_status.epout_status.byEP = ep_tx_id;

    /* Ep_id include direction */
    USB_Audio_Set_Chat_Info(USB_AUDIO_UNUSED_ID, USB_AUDIO_UNUSED_ID, if_id, (USB_EP_DIR_IN | ep_tx_id));

    hal_usb_get_dma_channel(ep_tx_id, 0, HAL_USB_EP_DIRECTION_TX, false);

    /* SYNC volume to APP/Audio module */
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_StreamIf_Microphone_Create sync volume to app layer, ch:%d", 1, g_UsbAudio[0].mic_chs);
    uint8_t ch;

    if(g_UsbAudio[0].mic_chs == 0) {
        /* Master Volume*/
        USB_Send_Message(USB_MIC_SET_VOLUME, (void *)(g_UsbAudio[0].mic_cur+ (1 << 16) + ((USB_EP_DIR_IN | g_UsbAudio[0].stream_ep_in_id) << 24)));
    } else {
        for(ch=1; ch<=(g_UsbAudio[0].mic_chs); ch++){
            USB_Send_Message(USB_MIC_SET_VOLUME, (void *)(g_UsbAudio[0].mic_cur + (ch << 16) + ((USB_EP_DIR_IN | g_UsbAudio[0].stream_ep_in_id) << 24)));
        }
    }
}

/* Audio stream interface reset function, enable EP */
void USB_Audio_StreamIf_Microphone_Reset(void)
{
    uint32_t port = 0;

    if(g_UsbAudio[port].mic_feature_en == false) {
        return;
    }

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_StreamIf_Microphone_Reset", 0);

    g_UsbAudio[port].txpipe = &g_UsbAudio[port].stream_ep_in_info->ep_status.epout_status;
    hal_usb_stop_dma_channel(g_UsbAudio[port].txpipe->byEP, HAL_USB_EP_DIRECTION_TX);
}

void USB_Audio_StreamIf_Microphone_Enable(void)
{
    uint32_t port = 0;

    if(g_UsbAudio[port].mic_feature_en == false) {
        return;
    }

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_StreamIf_Microphone_Enable", 0);

    hal_usb_stop_dma_channel(g_UsbAudio[port].txpipe->byEP, HAL_USB_EP_DIRECTION_TX);
    hal_usb_enable_tx_endpoint(g_UsbAudio[port].txpipe->byEP, HAL_USB_EP_TRANSFER_ISO, HAL_USB_EP_USE_NO_DMA, true);
}


/* Audio stream interface speed reset function, enable EP's speed-specific descriptor */
void USB_Audio_StreamIf_Microphone_Speed_Reset(bool b_other_speed)
{
    uint32_t port = 0;

    if(g_UsbAudio[port].mic_feature_en == false) {
        return;
    }

    uint32_t temp_max_size;
    if( g_USB_dynamic_fifo ) {
        temp_max_size = 0;
    } else {
        temp_max_size = USB_Audio_Stream_Microphone_Speed_Reset_Packet_Size(port, (bool)b_other_speed);
    }

    g_UsbAudio[port].stream_ep_in_info->epdesc.stdep.wMaxPacketSize[0] = temp_max_size & 0xff;
    g_UsbAudio[port].stream_ep_in_info->epdesc.stdep.wMaxPacketSize[1] = (temp_max_size >> 8) & 0xff;

    if (hal_usb_is_high_speed() == true) {
        g_UsbAudio[port].stream_ep_in_info->epdesc.stdep.bInterval = 4; /* (2^(4-1))*0.125us=1ms*/
    } else {
        g_UsbAudio[port].stream_ep_in_info->epdesc.stdep.bInterval = 1; /* (2^(1-1))*1ms=1ms*/
    }

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_StreamIf_Speed_Microphone_Reset wMaxPacketSize:%d b_other_speed:%d", 2, temp_max_size, b_other_speed);
}
#endif /* AIR_USB_AUDIO_1_MIC_ENABLE*/

/*********************************************************************************************
    global variable g_UsbAudio[port] initialize and release functions
*********************************************************************************************/
/* USB Audio 1 : Speaker */
uint8_t g_usb_audio_mute;
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
uint8_t g_usb_audio_microphone_mute;
#endif

/* USB Audio 2 : Speaker */
uint8_t g_usb_audio2_mute;

/* Set/Get USB Audio setting from speaker or microphone */
/* USB Audio 1 : Speaker */
void USB_Audio_Set_Spk1_Cur(uint8_t ch, uint16_t volume)
{
    ch = _USB_Audio_Channel_Check(ch);

    g_UsbAudio[0].spk_cur_channel_vol[ch] = volume;

    /* Check the volume setting is valid */
    if((int16_t)volume < (int16_t)g_UsbAudio[0].spk_min){
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Set_Spk1_Cur volume:%d is smaller than min:0x%X", 2, volume, g_UsbAudio[0].spk_min);
        g_UsbAudio[0].spk_cur_channel_vol[ch] = g_UsbAudio[0].spk_min;
    }

    if((int16_t)volume > (int16_t)g_UsbAudio[0].spk_max ){
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Set_Spk1_Cur volume:%d is bigger than MAX:0x%X", 2, volume, g_UsbAudio[0].spk_max);
        g_UsbAudio[0].spk_cur_channel_vol[ch] = g_UsbAudio[0].spk_max;
    }

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Set_Spk1_Cur set ch:%d volume:0x%X", 2, ch, g_UsbAudio[0].spk_cur_channel_vol[ch]);
}

uint16_t USB_Audio_Get_Spk1_Cur(uint8_t ch)
{
    ch = _USB_Audio_Channel_Check(ch);
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_Spk1_Cur get ch:%d volume:0x%X", 2, ch, g_UsbAudio[0].spk_cur_channel_vol[ch]);

    return g_UsbAudio[0].spk_cur_channel_vol[ch];
}

void USB_Audio_Set_Spk1_Mute(bool mute)
{
    g_usb_audio_mute = (uint8_t)mute;

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Set_Spk1_Mute set mute:%d", 1, g_usb_audio_mute);
}

uint8_t USB_Audio_Get_Spk1_Mute()
{
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_Spk1_Mute get mute:%d", 1, g_usb_audio_mute);

    return g_usb_audio_mute;
}

uint16_t USB_Audio_Get_Spk1_Min(void)
{
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_Spk1_Min get min:0x%X", 1, g_UsbAudio[0].spk_min);

    return g_UsbAudio[0].spk_min;
}

uint16_t USB_Audio_Get_Spk1_Max(void)
{
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_Spk1_Max get MAX:0x%X", 1, g_UsbAudio[0].spk_max);

    return g_UsbAudio[0].spk_max;
}

uint16_t USB_Audio_Get_Spk1_Res(void)
{
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_Spk1_Res get res:0x%X", 1, g_UsbAudio[0].spk_res);

    return g_UsbAudio[0].spk_res;
}

#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
/* USB Audio 1 : Microphone */
void USB_Audio_Set_Mic1_Cur(uint8_t ch, uint16_t volume)
{
    ch = _USB_Audio_Channel_Check(ch);

    g_UsbAudio[0].mic_cur_channel_vol[ch] = volume;

    /* Check the volume setting is valid */
    if((int16_t)volume < (int16_t)g_UsbAudio[0].mic_min){
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Set_Mic1_Cur volume:%d is smaller than min:0x%X", 2, volume, g_UsbAudio[0].mic_min);
        g_UsbAudio[0].mic_cur_channel_vol[ch] = g_UsbAudio[0].mic_min;
    }

    if((int16_t)volume > (int16_t)g_UsbAudio[0].mic_max ){
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Set_Mic1_Cur volume:%d is bigger than MAX:0x%X", 2, volume, g_UsbAudio[0].mic_max);
        g_UsbAudio[0].mic_cur_channel_vol[ch] = g_UsbAudio[0].mic_max;
    }

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Set_Mic1_Cur set ch:%d volume:0x%X", 2, ch, g_UsbAudio[0].mic_cur_channel_vol[ch]);
}

uint16_t USB_Audio_Get_Mic1_Cur(uint8_t ch)
{
    ch = _USB_Audio_Channel_Check(ch);
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_Mic1_Cur get ch:%d volume:0x%X", 2, ch, (uint16_t)g_UsbAudio[0].mic_cur_channel_vol[ch]);

    return g_UsbAudio[0].mic_cur_channel_vol[ch];
}

void USB_Audio_Set_Mic1_Mute(bool mute)
{
    g_usb_audio_microphone_mute = (uint8_t)mute;

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Set_Mic1_Mute set mute:%d", 1, g_usb_audio_microphone_mute);
}

uint8_t USB_Audio_Get_Mic1_Mute()
{
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_Mic1_Mute get mute:%d", 1, g_usb_audio_microphone_mute);

    return g_usb_audio_microphone_mute;
}

uint16_t USB_Audio_Get_Mic1_Min(void)
{
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_Mic1_Min get min:0x%X", 1, g_UsbAudio[0].mic_min);

    return g_UsbAudio[0].mic_min;
}

uint16_t USB_Audio_Get_Mic1_Max(void)
{
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_Mic1_Max get MAX:0x%X", 1, g_UsbAudio[0].mic_max);

    return g_UsbAudio[0].mic_max;
}

uint16_t USB_Audio_Get_Mic1_Res(void)
{
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_Mic1_Res get res:0x%X", 1, g_UsbAudio[0].mic_res);

    return g_UsbAudio[0].mic_res;
}
#endif

/* USB Audio 2 : Speaker */
void USB_Audio_Set_Spk2_Cur(uint8_t ch, uint16_t volume)
{
    ch = _USB_Audio_Channel_Check(ch);

    g_UsbAudio[1].spk_cur_channel_vol[ch] = volume;

    /* Check the volume setting is valid */
    if((int16_t)volume < (int16_t)g_UsbAudio[1].spk_min){
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Set_Spk2_Cur volume:%d is smaller than min:0x%X", 2, volume, g_UsbAudio[1].spk_min);
        g_UsbAudio[1].spk_cur_channel_vol[ch] = g_UsbAudio[1].spk_min;
    }

    if((int16_t)volume > (int16_t)g_UsbAudio[1].spk_max ){
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Set_Spk2_Cur volume:%d is bigger than MAX:0x%X", 2, volume, g_UsbAudio[1].spk_max);
        g_UsbAudio[1].spk_cur_channel_vol[ch] = g_UsbAudio[1].spk_max;
    }

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Set_Spk2_Cur set ch:%d volume:0x%X", 2, ch, g_UsbAudio[1].spk_cur_channel_vol[ch]);
}

uint16_t USB_Audio_Get_Spk2_Cur(uint8_t ch)
{
    ch = _USB_Audio_Channel_Check(ch);
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_Spk2_Cur get ch:%d volume:0x%X", 2, ch, g_UsbAudio[1].spk_cur_channel_vol[ch]);

    return g_UsbAudio[1].spk_cur_channel_vol[ch];
}

void USB_Audio_Set_Spk2_Mute(bool mute)
{
    g_usb_audio2_mute = (uint8_t)mute;

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Set_Spk2_Mute set mute:%d", 1, g_usb_audio2_mute);
}

uint8_t USB_Audio_Get_Spk2_Mute()
{
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_Spk2_Mute get mute:%d", 1, g_usb_audio2_mute);

    return g_usb_audio2_mute;
}

uint16_t USB_Audio_Get_Spk2_Min(void)
{
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_Spk2_Min get min:0x%X", 1, g_UsbAudio[1].spk_min);

    return g_UsbAudio[1].spk_min;
}

uint16_t USB_Audio_Get_Spk2_Max(void)
{
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_Spk2_Max get MAX:0x%X", 1, g_UsbAudio[1].spk_max);

    return g_UsbAudio[1].spk_max;
}

uint16_t USB_Audio_Get_Spk2_Res(void)
{
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_Spk2_Res get res:0x%X", 1, g_UsbAudio[1].spk_res);

    return g_UsbAudio[1].spk_res;
}

/* Set up USB Audio setting from NVkey or by default */
void USB_Audio_Setting_By_NVKey(void)
{
    nvkey_status_t nvkey_status;
    uint32_t nvkey_size;
    usb_nvkey_audio_device_cfg_t audio_device_cfg;

    /* USB Audio 1 Speaker */
    nvkey_size   = sizeof(audio_device_cfg);
    nvkey_status = nvkey_read_data(NVID_USB_SETTING_1, (uint8_t *)&audio_device_cfg, &nvkey_size);
    if (nvkey_status == NVKEY_STATUS_OK) {
        g_UsbAudio[0].spk_terminal_type = audio_device_cfg.terminal_type;
        g_UsbAudio[0].spk_cur           = audio_device_cfg.cur;
        g_UsbAudio[0].spk_min           = audio_device_cfg.min;
        g_UsbAudio[0].spk_max           = audio_device_cfg.max;
        g_UsbAudio[0].spk_res           = audio_device_cfg.res;
    }

    #ifdef AIR_DCHS_MODE_ENABLE
        g_UsbAudio[0].spk_terminal_type = USB_AUDIO_TERMT_HEADSET;
    #endif

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Setting_By_NVKey audio1 spk:0x%04X volume cur:0x%04X min:0x%04X max:0x%04X res:0x%04X nvkey_status:%d", 6,
                audio_device_cfg.terminal_type, audio_device_cfg.cur, audio_device_cfg.min, audio_device_cfg.max, audio_device_cfg.res, nvkey_status);

#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
    /* USB Audio 1 Microphone */
    nvkey_size   = sizeof(audio_device_cfg);
    nvkey_status = nvkey_read_data(NVID_USB_SETTING_2, (uint8_t *)&audio_device_cfg, &nvkey_size);
    if (nvkey_status == NVKEY_STATUS_OK) {
        g_UsbAudio[0].mic_terminal_type = audio_device_cfg.terminal_type;
        g_UsbAudio[0].mic_cur           = audio_device_cfg.cur;
        g_UsbAudio[0].mic_min           = audio_device_cfg.min;
        g_UsbAudio[0].mic_max           = audio_device_cfg.max;
        g_UsbAudio[0].mic_res           = audio_device_cfg.res;
    }
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Setting_By_NVKey audio1 mic:0x%04X volume cur:0x%04X min:0x%04X max:0x%04X res:0x%04X nvkey_status:%d", 6,
                audio_device_cfg.terminal_type, audio_device_cfg.cur, audio_device_cfg.min, audio_device_cfg.max, audio_device_cfg.res, nvkey_status);
#endif /* AIR_USB_AUDIO_1_MIC_ENABLE */

#ifdef AIR_USB_AUDIO_2_SPK_ENABLE
    /* USB Audio 2 Speaker */
    nvkey_size   = sizeof(audio_device_cfg);
    nvkey_status = nvkey_read_data(NVID_USB_SETTING_3, (uint8_t *)&audio_device_cfg, &nvkey_size);
    if (nvkey_status == NVKEY_STATUS_OK) {
        g_UsbAudio[1].spk_terminal_type = audio_device_cfg.terminal_type;
        g_UsbAudio[1].spk_cur           = audio_device_cfg.cur;
        g_UsbAudio[1].spk_min           = audio_device_cfg.min;
        g_UsbAudio[1].spk_max           = audio_device_cfg.max;
        g_UsbAudio[1].spk_res           = audio_device_cfg.res;
    }
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Setting_By_NVKey audio2 spk:0x%04X volume cur:0x%04X min:0x%04X max:0x%04X res:0x%04X nvkey_status:%d", 6,
                audio_device_cfg.terminal_type, audio_device_cfg.cur, audio_device_cfg.min, audio_device_cfg.max, audio_device_cfg.res, nvkey_status);
#endif /* AIR_USB_AUDIO_2_SPK_ENABLE */
}

uint32_t g_usb_audio_sample_rate;
uint8_t  g_usb_audio_sample_size;     /* Unit: byte */
uint8_t  g_usb_audio_channel;

#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
uint32_t g_usb_audio_mic_sample_rate;
uint8_t  g_usb_audio_mic_sample_size; /* Unit: byte */
uint8_t  g_usb_audio_mic_channel;
#endif

uint32_t g_usb_audio2_sample_rate;
uint8_t  g_usb_audio2_sample_size;    /* Unit: byte */
uint8_t  g_usb_audio2_channel;

void USB_Init_Audio_Status(void)
{
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Init_Audio_Status", 0);

    /* Initilize the Chat/Game if_id and ep_id */
    USB_Audio_Init_Chat_Game_Info();

    /* USB Audio v1 */
    USB_Audio[0].audio_version = USB_AUDIO_VERSION_V1;
    USB_Audio[1].audio_version = USB_AUDIO_VERSION_V1;

    g_UsbAudio[0].rxpipe = NULL;
    g_UsbAudio[1].rxpipe = NULL;
    g_UsbAudio[0].stream_interface_id = 0xFF;
    g_UsbAudio[1].stream_interface_id = 0xFF;

#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
    g_UsbAudio[0].txpipe = NULL;
    g_UsbAudio[1].txpipe = NULL;
    g_UsbAudio[0].stream_microphone_interface_id = 0xFF;
    g_UsbAudio[1].stream_microphone_interface_id = 0xFF;
#endif

    /* Initilize the sample rate, size and channel by default value */
    g_usb_audio_sample_rate =(audio1_spk_stream_format_dscr.tSamFreq[0].data[0])       |
                             (audio1_spk_stream_format_dscr.tSamFreq[0].data[1] << 8)  |
                             (audio1_spk_stream_format_dscr.tSamFreq[0].data[2] << 16) ;
    g_usb_audio_sample_size = audio1_spk_stream_format_dscr.bSubFrameSize;     /* Unit: byte */
    g_usb_audio_channel     = audio1_spk_stream_format_dscr.bNrChannels;

#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
    g_usb_audio_mic_sample_rate =(audio1_mic_stream_format_dscr.tSamFreq[0].data[0])       |
                                 (audio1_mic_stream_format_dscr.tSamFreq[0].data[1] << 8)  |
                                 (audio1_mic_stream_format_dscr.tSamFreq[0].data[2] << 16) ;
    g_usb_audio_mic_sample_size = audio1_mic_stream_format_dscr.bSubFrameSize; /* Unit: byte */
    g_usb_audio_mic_channel     = audio1_mic_stream_format_dscr.bNrChannels;
#endif

    g_usb_audio2_sample_rate =(audio2_spk_stream_format_dscr.tSamFreq[0].data[0])       |
                              (audio2_spk_stream_format_dscr.tSamFreq[0].data[1] << 8)  |
                              (audio2_spk_stream_format_dscr.tSamFreq[0].data[2] << 16) ;
    g_usb_audio2_sample_size = audio2_spk_stream_format_dscr.bSubFrameSize;    /* Unit: byte */
    g_usb_audio2_channel     = audio2_spk_stream_format_dscr.bNrChannels;

    USB_Audio[0].initialized = true;
    USB_Audio[1].initialized = true;
}

/* Release global variable g_UsbAudio[port] */
void USB_Release_Audio_Status(void)
{
    LOG_MSGID_I(USBAUDIO_DRV, "USB_Release_Audio_Status", 0);

    if (g_UsbAudio[0].rxpipe != NULL) {
        hal_usb_release_dma_channel(0, g_UsbAudio[0].rxpipe->byEP, HAL_USB_EP_DIRECTION_RX, false);
    }
    if (g_UsbAudio[1].rxpipe != NULL) {
        hal_usb_release_dma_channel(0, g_UsbAudio[1].rxpipe->byEP, HAL_USB_EP_DIRECTION_RX, false);
    }

#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
    if (g_UsbAudio[0].txpipe != NULL) {
        hal_usb_release_dma_channel(g_UsbAudio[0].txpipe->byEP, 0, HAL_USB_EP_DIRECTION_TX, false);
    }
    if (g_UsbAudio[1].txpipe != NULL) {
        hal_usb_release_dma_channel(g_UsbAudio[1].txpipe->byEP, 0, HAL_USB_EP_DIRECTION_TX, false);
    }
#endif

    if (USB_Audio[0].unplug_cb) {
        USB_Send_Message(USB_AUDIO_UNPLUG, NULL);
    }
    if (USB_Audio[1].unplug_cb) {
        USB_Send_Message(USB_AUDIO_UNPLUG, NULL);
    }

    USB_Init_Audio_Status();

    if (g_UsbAudio[0].rx_buffer != NULL) {
        USB_Free_Memory((void *)g_UsbAudio[0].rx_buffer);
        g_UsbAudio[0].rx_buffer = NULL;
    }
    if (g_UsbAudio[1].rx_buffer != NULL) {
        USB_Free_Memory((void *)g_UsbAudio[1].rx_buffer);
        g_UsbAudio[1].rx_buffer = NULL;
    }

    if (gUsbDevice.conf != NULL) {
        USB_Free_Memory((void *)gUsbDevice.conf);
        gUsbDevice.conf = NULL;
    }
}

/************************************************************
    EP0 handle functions
************************************************************/
/* Parse class specific request */
static uint16_t g_usb_audio_wIndex;
static uint16_t g_usb_audio_wValue;
static uint16_t g_usb_audio_wLength;

static uint16_t g_usb_audio2_wIndex;
static uint16_t g_usb_audio2_wValue;
static uint16_t g_usb_audio2_wLength;

int32_t USB_Audio_Volume_dB_Convertor(int16_t raw_data)
{
    int32_t dB;

    /* dB = ((int32_t) raw_data) / 256 * 10000 */
    dB = (((int32_t) raw_data) * 625) / 16;

    USB_DBG_I( USB_DBG_AUDIO_VOLUME, "USB_Audio_Volume_dB_Convertor raw_data:0x%04X dB:%d", 2, raw_data, dB);

    return dB;
}

uint16_t USB_Audio_Volume_0to100_Convertor(uint16_t raw_data, uint16_t volume_min, uint16_t volume_max)
{
    uint16_t volume;

    /* 0~100% = min ~ max */
    // step   = ((int16_t)volume_max - (int16_t)volume_min) / 100;
    // volume = ((int16_t)raw_data   - (int16_t)volume_min) / step;
    volume = ((int16_t)raw_data - (int16_t)volume_min) * 100 / ((int16_t)volume_max - (int16_t)volume_min);

    USB_DBG_I( USB_DBG_AUDIO_VOLUME, "USB_Audio_Volume_0to100_Convertor volume:%d raw_data:0x%04X volume_max:0x%04X volume_min:0x%04X", 4, volume, raw_data, volume_max, volume_min);

    return volume;
}

static void USB_Audio1_Ep0_DataReceived(void *data)
{
    bool stall = false;
    uint16_t wValue  = g_usb_audio_wValue;
    uint16_t wIndex  = g_usb_audio_wIndex;
    uint16_t wLength = g_usb_audio_wLength;

    uint8_t  spk_ep_addr = (USB_EP_DIR_OUT | g_UsbAudio[0].stream_ep_out_id);
    uint16_t spk_wIndex  = (0x02 << 8 | g_UsbAudio[0].control_interface_id);

#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
    uint8_t  mic_ep_addr = (USB_EP_DIR_IN | g_UsbAudio[0].stream_ep_in_id);
    uint16_t mic_wIndex  = (0x06 << 8 | g_UsbAudio[0].control_interface_id);
#endif

    /* Check EP0 data is enough */
    USB_Check_EP0_DataLen(0, wLength);

    if (wValue == 0x0100 && wIndex == spk_ep_addr) {
        hal_usb_read_endpoint_fifo(0, wLength, &g_usb_audio_sample_rate);
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio1_Ep0_DataReceived speaker1 change sample rate to %d Hz", 1, g_usb_audio_sample_rate);
        USB_Send_Message(USB_AUDIO_SET_SAMPLING_FREQ, (void *)(g_usb_audio_sample_rate + (spk_ep_addr << 24)));
    }
    else if (wValue == 0x0100 && wIndex == spk_wIndex) {
        hal_usb_read_endpoint_fifo(0, wLength, &g_usb_audio_mute);
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio1_Ep0_DataReceived speaker1 change mute to %d", 1, g_usb_audio_mute);
        USB_Send_Message(USB_AUDIO_SET_MUTE, (void *)(g_usb_audio_mute + (spk_ep_addr << 24)));
    }
    else if ((wValue & 0x0200) && wIndex == spk_wIndex) {
        uint8_t ch_index = wValue & 0xFF;
        ch_index = _USB_Audio_Channel_Check(ch_index);
        hal_usb_read_endpoint_fifo(0, wLength, &g_UsbAudio[0].spk_cur_channel_vol[ch_index]);
        USB_Send_Message(USB_AUDIO_SET_VOLUME, (void *)(g_UsbAudio[0].spk_cur_channel_vol[ch_index] + (ch_index << 16) + (spk_ep_addr << 24)));
    }
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
    else if ((wValue & 0x0200) && wIndex == mic_wIndex) {
        uint8_t ch_index = wValue & 0xFF;
        ch_index = _USB_Audio_Channel_Check(ch_index);
        hal_usb_read_endpoint_fifo(0, wLength, &g_UsbAudio[0].mic_cur_channel_vol[ch_index]);
        #ifndef USB_TEST_MIC
        USB_Send_Message(USB_MIC_SET_VOLUME, (void *)(g_UsbAudio[0].mic_cur_channel_vol[ch_index] + (ch_index << 16) + (mic_ep_addr << 24)));
        #endif
    }
    else if (wValue == 0x0100 && wIndex == mic_wIndex) {
        hal_usb_read_endpoint_fifo(0, wLength, &g_usb_audio_microphone_mute);
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio1_Ep0_DataReceived microphone change mute to %d", 1, g_usb_audio_microphone_mute);
        #ifndef USB_TEST_MIC
        USB_Send_Message(USB_MIC_SET_MUTE, (void *)(g_usb_audio_microphone_mute + (mic_ep_addr << 24)));
        #endif
    }
    else if (wValue == 0x0100 && wIndex == mic_ep_addr) {
        hal_usb_read_endpoint_fifo(0, wLength, &g_usb_audio_mic_sample_rate);
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio1_Ep0_DataReceived microphone change sample rate to %d Hz", 1, g_usb_audio_mic_sample_rate);
        #ifndef USB_TEST_MIC
        USB_Send_Message(USB_MIC_SET_SAMPLING_FREQ, (void *)(g_usb_audio_mic_sample_rate + (mic_ep_addr << 24)));
        #endif
    }
#endif
    else {
        stall = true;
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio1_Ep0_DataReceived is out of case wValue:0x%X wIndex:0x%X", 2,
                    wValue, wIndex);
    }

    gUsbDevice.ep0_state = USB_EP0_RX_STATUS;
    hal_usb_update_endpoint_0_state(HAL_USB_EP0_DRV_STATE_READ_END, stall, true);
}

static void USB_Audio2_Ep0_DataReceived(void *data)
{
    bool stall = false;
    uint16_t wValue  = g_usb_audio2_wValue;
    uint16_t wIndex  = g_usb_audio2_wIndex;
    uint16_t wLength = g_usb_audio2_wLength;

    uint8_t  spk_ep_addr = (USB_EP_DIR_OUT | g_UsbAudio[1].stream_ep_out_id);
    uint16_t spk_wIndex  = (0x0A << 8 | g_UsbAudio[1].control_interface_id);

    /* Check EP0 data is enough */
    USB_Check_EP0_DataLen(0, wLength);

    /*wValue: control num  << 8 | channel*/
    /*wIndex: feature_unit << 8 | interface*/
    /*For audio_2,  wIndex =0x0A03*/
    if (wValue == 0x0100 && wIndex == spk_ep_addr) {
        hal_usb_read_endpoint_fifo(0, wLength, &g_usb_audio2_sample_rate);
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio2_Ep0_DataReceived speaker2 change sample rate to %d Hz", 1, g_usb_audio2_sample_rate);
        USB_Send_Message(USB_AUDIO_SET_SAMPLING_FREQ, (void *)(g_usb_audio2_sample_rate + (spk_ep_addr << 24)));
    }
    else if (wValue == 0x0100 && wIndex == spk_wIndex) {
        hal_usb_read_endpoint_fifo(0, wLength, &g_usb_audio2_mute);
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio2_Ep0_DataReceived speaker2 change mute to %d", 1, g_usb_audio2_mute);
        USB_Send_Message(USB_AUDIO_SET_MUTE, (void *)(g_usb_audio2_mute + (spk_ep_addr << 24)));
    }
    else if ((wValue & 0x0200) && wIndex == spk_wIndex) {
        uint8_t ch_index = wValue & 0x00FF;
        ch_index = _USB_Audio_Channel_Check(ch_index);
        hal_usb_read_endpoint_fifo(0, wLength, &g_UsbAudio[1].spk_cur_channel_vol[ch_index]);
        USB_Send_Message(USB_AUDIO_SET_VOLUME, (void *)(g_UsbAudio[1].spk_cur_channel_vol[ch_index] + (ch_index << 16) + (spk_ep_addr << 24)));
    }
    else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio2_Ep0_DataReceived is out of case wValue:0x%X wIndex:0x%X", 2,
                    wValue, wIndex);
        stall = true;
    }

    gUsbDevice.ep0_state = USB_EP0_RX_STATUS;
    hal_usb_update_endpoint_0_state(HAL_USB_EP0_DRV_STATE_READ_END, stall, true);
}

void USB_Audio1_V1_Ep0_Command(Usb_Ep0_Status *pep0state, Usb_Command *pcmd)
{
    bool  bError = false;
    uint16_t usb_aduio1_spk_wIndex = (0x02 << 8 | g_UsbAudio[0].control_interface_id);
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
    uint16_t usb_aduio1_mic_wIndex = (0x06 << 8 | g_UsbAudio[0].control_interface_id);
#endif

    /*wValue: control num << 8 | channel*/
    /*wIndex: interface << 8 | feature_unit*/
    switch (pcmd->bRequest) {
        case USB_AUDIO_1_REQ_GET_CUR:
            if (pcmd->wValue & 0x0100 && pcmd->wIndex == 0x01) { /* USB out EP for speaker*/
                USB_Generate_EP0Data(pep0state, pcmd, &g_usb_audio_sample_rate, pcmd->wLength);
            }
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
            else if (pcmd->wValue & 0x0100 && pcmd->wIndex == 0x81) { /* USB in EP for microphone*/
                USB_Generate_EP0Data(pep0state, pcmd, &g_usb_audio_mic_sample_rate, pcmd->wLength);
            } else if (pcmd->wValue & 0x0100 && pcmd->wIndex == usb_aduio1_mic_wIndex) { /* HSB 0x01: Mute Control; LSB 0x00: Master Channel. HSB 0x06: Feature Unit ID; LSB 0x00: Interface #*/
                USB_Generate_EP0Data(pep0state, pcmd, &g_usb_audio_microphone_mute, pcmd->wLength);
            } else if ((pcmd->wValue & 0x0200) && (pcmd->wIndex == usb_aduio1_mic_wIndex)) { /* HSB 0x02: Volume Control; LSB 0x01: Channel 1*/
                uint8_t ch_index = pcmd->wValue & 0x00FF;
                ch_index = _USB_Audio_Channel_Check(ch_index);
                LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio1_V1_Ep0_Command get mic volume, ch:%d raw:0x%04x", 2, ch_index, g_UsbAudio[0].mic_cur_channel_vol[ch_index]);
                USB_Generate_EP0Data(pep0state, pcmd, &g_UsbAudio[0].mic_cur_channel_vol[ch_index], pcmd->wLength);
            }
#endif
            else if (pcmd->wValue & 0x0100 && pcmd->wIndex == usb_aduio1_spk_wIndex) { /* HSB 0x01: Mute Control; LSB 0x00: Master Channel. HSB 0x02: Feature Unit ID; LSB 0x00: Interface #*/
                USB_Generate_EP0Data(pep0state, pcmd, &g_usb_audio_mute, pcmd->wLength);
            }  else if ((pcmd->wValue & 0x0200) && (pcmd->wIndex == usb_aduio1_spk_wIndex)) { /* HSB 0x02: Volume Control; LSB 0x01: Channel 1*/
                uint8_t ch_index = pcmd->wValue & 0x00FF;
                ch_index = _USB_Audio_Channel_Check(ch_index);
                LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio1_V1_Ep0_Command get speaker volume, ch:%d raw:0x%04x", 2, ch_index, g_UsbAudio[0].spk_cur_channel_vol[ch_index]);
                USB_Generate_EP0Data(pep0state, pcmd, &g_UsbAudio[0].spk_cur_channel_vol[ch_index], pcmd->wLength);
            }  else {
                bError = true;
            }
            break;
        case USB_AUDIO_1_REQ_GET_MIN:
            if ((pcmd->wValue & 0x0200) && (pcmd->wIndex == usb_aduio1_spk_wIndex)) { /* HSB 0x02: Volume Control; LSB 0x01: Channel 1*/
                USB_Generate_EP0Data(pep0state, pcmd, (void *)(&g_UsbAudio[0].spk_min), pcmd->wLength);
            }
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
            else if ((pcmd->wValue & 0x0200) && (pcmd->wIndex == usb_aduio1_mic_wIndex)) { /* HSB 0x02: Volume Control; LSB 0x01: Channel 1*/
                USB_Generate_EP0Data(pep0state, pcmd, (void *)(&g_UsbAudio[0].mic_min), pcmd->wLength);
            }
#endif
            else {
                bError = true;
            }
            break;
        case USB_AUDIO_1_REQ_GET_MAX:
            /* tell host the current setting */
            if ((pcmd->wValue & 0x0200) && (pcmd->wIndex == usb_aduio1_spk_wIndex)) { /* HSB 0x02: Volume Control; LSB 0x01: Channel 1*/
                USB_Generate_EP0Data(pep0state, pcmd, (void *)(&g_UsbAudio[0].spk_max), pcmd->wLength);
            }
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
            else if ((pcmd->wValue & 0x0200) && (pcmd->wIndex == usb_aduio1_mic_wIndex)) { /* HSB 0x02: Volume Control; LSB 0x01: Channel 1*/
                USB_Generate_EP0Data(pep0state, pcmd, (void *)(&g_UsbAudio[0].mic_max), pcmd->wLength);
            }
#endif
            else {
                bError = true;
            }
            break;
        case USB_AUDIO_1_REQ_GET_RES:
            if ((pcmd->wValue & 0x0200) && (pcmd->wIndex == usb_aduio1_spk_wIndex)) { /* HSB 0x02: Volume Control; LSB 0x01: Channel 1*/
                USB_Generate_EP0Data(pep0state, pcmd, (void *)(&g_UsbAudio[0].spk_res), pcmd->wLength);
            }
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
            else if ((pcmd->wValue & 0x0200) && (pcmd->wIndex == usb_aduio1_mic_wIndex)) { /* HSB 0x02: Volume Control; LSB 0x01: Channel 1*/
                USB_Generate_EP0Data(pep0state, pcmd, (void *)(&g_UsbAudio[0].mic_res), pcmd->wLength);
            }
#endif
            else {
                bError = true;
            }
            break;
        case USB_AUDIO_1_REQ_SET_CUR:
            g_usb_audio_wValue = pcmd->wValue;
            g_usb_audio_wIndex = pcmd->wIndex;
            g_usb_audio_wLength = pcmd->wLength;
            gUsbDevice.ep0_rx_handler = (usb_ep0_rx_ptr)USB_Audio1_Ep0_DataReceived;
            gUsbDevice.ep0_state = USB_EP0_RX;
            break;
        case USB_AUDIO_1_REQ_GET_MEM:
            //USB_Generate_EP0Data(pep0state, pcmd, &GetMEM_Data, pcmd->wLength);
            bError = true;
            break;
        case USB_AUDIO_1_REQ_GET_STAT:
            bError = true;
            break;
        default:
            bError = true;
            break;
    }


    if (bError != true) {
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio1_V1_Ep0_Command bmRequestType:0x%02X bRequest:0x%02X wValue:0x%04X wIndex:0x%04X wLength:0x%04X",
                    5, pcmd->bmRequestType, pcmd->bRequest, pcmd->wValue, pcmd->wIndex, pcmd->wLength);
    } else{
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio1_V1_Ep0_Command is out of case bmRequestType:0x%02X bRequest:0x%02X wValue:0x%04X wIndex:0x%04X wLength:0x%04X",
                    5, pcmd->bmRequestType, pcmd->bRequest, pcmd->wValue, pcmd->wIndex, pcmd->wLength);
    }

    /* Stall command if an error occured */
    USB_EP0_Command_Hdlr(bError);
}


void USB_Audio2_V1_Ep0_Command(Usb_Ep0_Status *pep0state, Usb_Command *pcmd)
{
    bool  bError = false;
    uint16_t usb_aduio2_spk_wIndex = (0x0A << 8 | g_UsbAudio[1].control_interface_id);

    /*wValue: control num << 8 | channel*/
    /*wIndex: feature_unit << 8 | interface*/
    /*For audio_2, the wValue = 0x0100,   wIndex =0X0A03*/
    switch (pcmd->bRequest) {
        case USB_AUDIO_1_REQ_GET_CUR:
            if (pcmd->wValue == 0x0100 && pcmd->wIndex == 0x02) { /* USB out EP for speaker*/
                USB_Generate_EP0Data(pep0state, pcmd, &g_usb_audio2_sample_rate, pcmd->wLength);
            } else if (pcmd->wValue == 0x0100 && pcmd->wIndex == usb_aduio2_spk_wIndex) { /* HSB 0x01: Mute Control; LSB 0x00: Master Channel. HSB 0x02: Feature Unit ID; LSB 0x00: Interface #*/
                USB_Generate_EP0Data(pep0state, pcmd, &g_usb_audio2_mute, pcmd->wLength);
            } else if ((pcmd->wValue & 0x0200) && (pcmd->wIndex == usb_aduio2_spk_wIndex)) { /* HSB 0x02: Volume Control; LSB 0x01: Channel 1*/
                uint8_t ch_index = pcmd->wValue & 0x00FF;
                ch_index = _USB_Audio_Channel_Check(ch_index);
                LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio2_V1_Ep0_Command get speaker volume, ch:%d raw:0x%04x", 2, ch_index, g_UsbAudio[1].spk_cur_channel_vol[ch_index]);
                USB_Generate_EP0Data(pep0state, pcmd, &g_UsbAudio[1].spk_cur_channel_vol[ch_index], pcmd->wLength);
            } else {
                bError = true;
            }
            break;
        case USB_AUDIO_1_REQ_GET_MIN:
            if ((pcmd->wValue & 0x0200) && (pcmd->wIndex == usb_aduio2_spk_wIndex)) { /* HSB 0x02: Volume Control; LSB 0x01: Channel 1*/
                USB_Generate_EP0Data(pep0state, pcmd, (void *)(&g_UsbAudio[1].spk_min), pcmd->wLength);
            } else {
                bError = true;
            }
            break;
        case USB_AUDIO_1_REQ_GET_MAX:
            /* tell host the current setting */
            if ((pcmd->wValue & 0x0200) && (pcmd->wIndex == usb_aduio2_spk_wIndex)) { /* HSB 0x02: Volume Control; LSB 0x01: Channel 1*/
                USB_Generate_EP0Data(pep0state, pcmd, (void *)(&g_UsbAudio[1].spk_max), pcmd->wLength);
            } else {
                bError = true;
            }
            break;
        case USB_AUDIO_1_REQ_GET_RES:
            if ((pcmd->wValue & 0x0200) && (pcmd->wIndex == usb_aduio2_spk_wIndex)) { /* HSB 0x02: Volume Control; LSB 0x01: Channel 1*/
                USB_Generate_EP0Data(pep0state, pcmd, (void *)(&g_UsbAudio[1].spk_res), pcmd->wLength);
            } else {
                bError = true;
            }
            break;
        case USB_AUDIO_1_REQ_SET_CUR:
            g_usb_audio2_wValue = pcmd->wValue;
            g_usb_audio2_wIndex = pcmd->wIndex;
            g_usb_audio2_wLength = pcmd->wLength;
            gUsbDevice.ep0_rx_handler = (usb_ep0_rx_ptr)USB_Audio2_Ep0_DataReceived;
            gUsbDevice.ep0_state = USB_EP0_RX;
            break;
        case USB_AUDIO_1_REQ_GET_MEM:
            //USB_Generate_EP0Data(pep0state, pcmd, &GetMEM_Data, pcmd->wLength);
            bError = true;
            break;
        case USB_AUDIO_1_REQ_GET_STAT:
            bError = true;
            break;
        default:
            bError = true;
            break;
    }

    if (bError != true) {
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio2_V1_Ep0_Command bmRequestType:0x%02X bRequest:0x%02X wValue:0x%04X wIndex:0x%04X wLength:0x%04X",
                    5, pcmd->bmRequestType, pcmd->bRequest, pcmd->wValue, pcmd->wIndex, pcmd->wLength);
    } else{
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio2_V1_Ep0_Command is out of case bmRequestType:0x%02X bRequest:0x%02X wValue:0x%04X wIndex:0x%04X wLength:0x%04X",
                    5, pcmd->bmRequestType, pcmd->bRequest, pcmd->wValue, pcmd->wIndex, pcmd->wLength);
    }

    /* Stall command if an error occured */
    USB_EP0_Command_Hdlr(bError);
}

void USB_Audio1_Ep0_Command(Usb_Ep0_Status *pep0state, Usb_Command *pcmd)
{
    if (USB_Audio[0].audio_version == USB_AUDIO_VERSION_V1) {
        USB_Audio1_V1_Ep0_Command(pep0state, pcmd);
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Ep0_Command version:%d is not %d", 2, (uint8_t)USB_Audio[0].audio_version, USB_AUDIO_VERSION_V1);
    }
}

void USB_Audio2_Ep0_Command(Usb_Ep0_Status *pep0state, Usb_Command *pcmd)
{
    if (USB_Audio[1].audio_version == USB_AUDIO_VERSION_V1) {
        USB_Audio2_V1_Ep0_Command(pep0state, pcmd);
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Ep0_Command version:%d is not %d", 2, (uint8_t)USB_Audio[0].audio_version, USB_AUDIO_VERSION_V1);
    }
}

/************************************************************
    Iso EP OUT handle functions
*************************************************************/
/* Set up RX, corresponding smaple size and channel */
uint32_t USB_Audio_Set_RX_Alt(uint32_t port, uint8_t alt_num)
{
    usb_audio_dscr_as_format_type_t *p = NULL;

    if((port == 0) && (alt_num == 0 || alt_num == 1)) { /* Speaker 1 */
        p = &audio1_spk_stream_format_dscr;
    }
#ifdef USB_AUDIO_RX1_ALT2_ENABLE
    else if((port == 0) && (alt_num == 2)) {
        p = &audio1_spk_stream_alt2_format_dscr;
    }
#endif
#ifdef USB_AUDIO_RX1_ALT3_ENABLE
    else if((port == 0) && (alt_num == 3)) {
        p = &audio1_spk_stream_alt3_format_dscr;
    }
#endif
    else if((port == 1) && (alt_num == 0 || alt_num == 1)) { /* Speaker 2 */
        p = &audio2_spk_stream_format_dscr;
    }
#ifdef USB_AUDIO_RX2_ALT2_ENABLE
    else if((port == 1) && (alt_num == 2)) {
        p = &audio2_spk_stream_alt2_format_dscr;
    }
#endif

    if (port == 0) {
        if (p) {
            g_usb_audio_sample_size = p->bSubFrameSize;
            g_usb_audio_channel = p->bNrChannels;
        }
        else {
            /* Invalid setting */
            g_usb_audio_sample_size = 0;
            g_usb_audio_channel     = 0;
        }
    }
    else if (port == 1) {
        if (p) {
            g_usb_audio2_sample_size = p->bSubFrameSize;
            g_usb_audio2_channel = p->bNrChannels;
        }
        else {
            /* Invalid setting  */
            g_usb_audio2_sample_size = 0;
            g_usb_audio2_channel     = 0;
        }
    }

    if(port == 0) {
         LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Set_RX_Alt port:%d alt_num:%d sample_size:%d channel:%d", 4,
                     port, alt_num, g_usb_audio_sample_size, g_usb_audio_channel);
    } else if (port == 1) {
         LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Set_RX_Alt port:%d alt_num:%d sample_size:%d channel:%d", 4,
                     port, alt_num, g_usb_audio2_sample_size, g_usb_audio2_channel);
    }

    return 0;
}

uint8_t USB_Audio_Get_RX_Channel(uint32_t port)
{
    uint8_t channel;

    if(port == 0){
        channel = g_usb_audio_channel;
    } else {
        channel = g_usb_audio2_channel;
    }

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_RX_Channel port:%d channel:%d", 2, port, channel);
    return channel;
}

uint8_t USB_Audio_Get_RX_Sample_Size(uint32_t port)
{
    uint8_t sample_size;

    if(port == 0){
        sample_size = g_usb_audio_sample_size;
    } else {
        sample_size = g_usb_audio2_sample_size;
    }

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_RX_Sample_Size port:%d sample_size:%d ", 2, port, sample_size);
    return sample_size;
}

uint32_t USB_Audio_Get_RX_Sample_Rate(uint32_t port)
{
    uint32_t sample_rate = 0;

    if (USB_Get_Device_State() == DEVSTATE_CONFIG) {
        if (port == 0) {
            sample_rate = g_usb_audio_sample_rate;
        } else if (port == 1) {
            sample_rate = g_usb_audio2_sample_rate;
        }
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_RX_Sample_Rate port:%d sample_rate:%d", 2, port, sample_rate);
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Get_RX_Sample_Rate port:%d sample_rate:%d USB state:%d is not ready", 3,
                    port, sample_rate, USB_Get_Device_State());
    }

    return sample_rate;
}

/* USB_AUDIO_DSCR_MAX_FREQ_NUM */
uint32_t Get_Maximum(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e)
{
   uint32_t i = 0;

   if (a > i) i = a;
   if (b > i) i = b;
   if (c > i) i = c;
   if (d > i) i = d;
   if (e > i) i = e;

   return i;
}

uint32_t USB_Audio_Get_RX_Sample_Rate_Max(uint32_t port, uint8_t alt_num)
{
    uint32_t sample_rate_max = 0;
    usb_audio_dscr_as_format_type_t *p = NULL;

    if((port == 0) && (alt_num == 1)) { /* Speaker 1 */
        p = &audio1_spk_stream_format_dscr;
    }
#ifdef USB_AUDIO_RX1_ALT2_ENABLE
    else if((port == 0) && (alt_num == 2)) {
        p = &audio1_spk_stream_alt2_format_dscr;
    }
#endif
#ifdef USB_AUDIO_RX1_ALT3_ENABLE
    else if((port == 0) && (alt_num == 3)) {
        p = &audio1_spk_stream_alt3_format_dscr;
    }
#endif
    else if((port == 1) && (alt_num == 1)) { /* Speaker 2 */
        p = &audio2_spk_stream_format_dscr;
    }
#ifdef USB_AUDIO_RX2_ALT2_ENABLE
    else if((port == 1) && (alt_num == 2)) {
        p = &audio2_spk_stream_alt2_format_dscr;
    }
#endif

    if (p) {
        sample_rate_max = Get_Maximum(AUDFREQ_TO_U32(p->tSamFreq[0]),
                                      AUDFREQ_TO_U32(p->tSamFreq[1]),
                                      AUDFREQ_TO_U32(p->tSamFreq[2]),
                                      AUDFREQ_TO_U32(p->tSamFreq[3]),
                                      AUDFREQ_TO_U32(p->tSamFreq[4]));
    }
    else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Get_RX_Sample_Rate_Max port:%d sample rate is 0", 1, port);
        sample_rate_max = 0;
    }

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_RX_Sample_Rate_Max port:%d sample rate max:%d", 2, port, sample_rate_max);

    return sample_rate_max;
}

uint32_t USB_Audio_Get_RX_Alt_Byte(uint32_t port, uint8_t alt_num)
{
    /* byte = sample rate x sample size x channel */
    bool hb_enable = false;
    uint8_t hb_packet;
    uint32_t byte;
    uint32_t sample_rate_max;
    usb_audio_dscr_as_format_type_t *p = NULL;

    /* Unint: K HZ */
    sample_rate_max = USB_Audio_Get_RX_Sample_Rate_Max(port, alt_num);
    sample_rate_max /= 1000;

    if((port == 0) && (alt_num == 1)) { /* Speaker 1 */
        p = &audio1_spk_stream_format_dscr;
    }
#ifdef USB_AUDIO_RX1_ALT2_ENABLE
    else if((port == 0) && (alt_num == 2)) {
        p = &audio1_spk_stream_alt2_format_dscr;
    }
#endif
#ifdef USB_AUDIO_RX1_ALT3_ENABLE
    else if((port == 0) && (alt_num == 3)) {
        p = &audio1_spk_stream_alt3_format_dscr;
    }
#endif
    else if((port == 1) && (alt_num == 1)) { /* Speaker 2 */
        p = &audio2_spk_stream_format_dscr;
    }
#ifdef USB_AUDIO_RX2_ALT2_ENABLE
    else if((port == 1) && (alt_num == 2)) {
        p = &audio2_spk_stream_alt2_format_dscr;
    }
#endif

    if (p) {
        byte = sample_rate_max * p->bSubFrameSize * p->bNrChannels;
    }
    else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Get_RX_Alt_Byte port:%d alt_num:%d byte is 0", 2, port, alt_num);
        byte = 0;
    }

    /* USB 2.0 Spec - 5.9.2 High Bandwidth Isochronous Endpoints */
    hb_packet = byte / HAL_USB_MAX_PACKET_SIZE_ENDPOINT_ISOCHRONOUS_HIGH_SPEED;
    if(hb_packet != 0){
        hb_enable = true;
    }

    if(hb_enable){
       /* USB 2.0 Spec - 9.6.6 Endpoint
        * Bits 12..11 specify the number of additional transaction opportunities per microframe:
        * 00 = None (1 transaction per microframe)
        * 01 = 1 additional (2 per microframe)
        * 10 = 2 additional (3 per microframe)
        * 11 = Reserved
        */
        byte /= (hb_packet + 1); /* Convert 1 additional to 2 per microframe e.g. 1152 = 576 + 576 */
        byte |= hb_packet << USB_RXMAP_M1_POS;

        hal_usb_set_high_bandwidth(true, hb_packet);
    } else {
        hal_usb_set_high_bandwidth(false, hb_packet);
    }

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_RX_Alt_Byte port:%d alt_num:%d byte:%d hb_packet:%d", 4,
               port, alt_num, byte%HAL_USB_MAX_PACKET_SIZE_ENDPOINT_ISOCHRONOUS_HIGH_SPEED, hb_packet);

    /* Include data_size packet inb bits 12..11 */
    return byte;
}

static uint32_t USB_Audio_Get_RX_Bytes(uint32_t port)
{
    /* byte = sample rate x sample size x channel */
    uint32_t byte;

    if(port == 0){
        byte =(g_usb_audio_sample_rate/1000) * g_usb_audio_sample_size * g_usb_audio_channel;
    } else if (port == 1){
        byte = (g_usb_audio2_sample_rate/1000) * g_usb_audio2_sample_size * g_usb_audio2_channel;
    } else {
        byte = 0;
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Get_RX_Bytes port:%d is not valid", 1, port);
    }

    return byte;
}


/************************************************************
    USB Audio API for User
*************************************************************/
void *USB_Audio_Rx_Buffer_Get_Read_Address(uint32_t port)
{
    uint8_t *p;

    if(port == USB_AUDIO_1_PORT || port == USB_AUDIO_2_PORT){
        p = g_UsbAudio[port].rx_buffer;
    } else {
        p = 0;
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Rx_Buffer_Get_Read_Address port:%d is invalid", 1, port);
    }

    return p;
}

/* Drop all data in buffer and exclude input parameter byte */
void USB_Audio_Rx_Buffer_Drop_Bytes(uint32_t port, uint32_t bytes)
{
    if(port == USB_AUDIO_1_PORT || port == USB_AUDIO_2_PORT){
        g_UsbAudio[port].rx_buffer_write = 0;
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Rx_Buffer_Drop_Bytes port:%d is invalid", 1, port);
    }

    return;
}

uint32_t USB_Audio_Get_Len_Received_Data(uint32_t port)
{
    uint32_t byte;

    if(port == USB_AUDIO_1_PORT || port == USB_AUDIO_2_PORT){
        byte = g_UsbAudio[port].rx_buffer_write;
        //LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_Len_Received_Data port:%d byte:%d", 2, port, byte);
    } else {
        byte = 0;
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Get_Len_Received_Data port:%d is invalid", 1, port);
    }

    return byte;
}

uint32_t USB_Audio_Read_Data(uint32_t port, void *dst, uint32_t len)
{
    //LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Read_Data port:%d dst:0x%X len:%d", 3, port, dst, len);

    memcpy(dst, USB_Audio_Rx_Buffer_Get_Read_Address(port), len);

    /* After read and clear buffer; otherwise, user would get available data length by USB_Audio_Get_Len_Received_Data  */
    USB_Audio_Rx_Buffer_Drop_Bytes(port, len);

    return len;
}

/************************************************************
    Iso EP OUT handler
*************************************************************/
#if defined(USB_DBG) && defined(USB_AUDIO_RX_TIME_MEASURE)
uint32_t usb_rx1_time1_start = 0;
uint32_t usb_rx1_time1_end   = 0;
uint32_t usb_rx1_time2_start = 0;
uint32_t usb_rx1_time2_end   = 0;

uint32_t usb_rx2_time1_start = 0;
uint32_t usb_rx2_time1_end   = 0;
uint32_t usb_rx2_time2_start = 0;
uint32_t usb_rx2_time2_end   = 0;
#endif

uint32_t usb_rx1_cnt = 0;
uint32_t usb_rx2_cnt = 0;
uint32_t usb_tx1_cnt = 0;

/* EP Iso Out interrupt handler, called by EP interrupt */
void USB_Audio1_IsoOut_Hdr(void)
{
#if defined(USB_DBG) && defined(USB_AUDIO_RX_TIME_MEASURE)
    uint32_t time_gap = 0;
    uint32_t time_gap1 = 0, time_gap2 = 0;
    if (usb_dbg_is_opt_enable(USB_DBG_OPT_AUDIO_RX_TIME_MEASURE)) {
        SLA_CustomLogging("RX1", SA_START);
        SLA_CustomLogging("RX1", SA_LABEL);

        /* Timer1 - Check the ISO out is expected 1ms/transaction */
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &usb_rx1_time1_start);

        time_gap = usb_rx1_time1_start - usb_rx1_time1_end;

        if ((time_gap > 1500) && (time_gap < 5000)) {
            LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio1_IsoOut_Hdr is not 1ms time gap:%dus", 1, time_gap);
            if (usb_dbg_is_opt_enable(USB_DBG_OPT_AUDIO_RX_TIME_ASSERT)) {
                assert(0);
            }
        } else {
            USB_DBG_I( USB_DBG_AUDIO_ISO_OUT, "USB_Audio1_IsoOut_Hdr time measurement gap:%dus", 1, time_gap);
        }

        usb_rx1_time1_end = usb_rx1_time1_start;

        /* Timer2 - Check time consuming of each operation */
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &usb_rx1_time2_start);
    }
#endif

    uint32_t port = USB_AUDIO_1_PORT;
    uint32_t nCount, fifo_len;
    uint8_t rx_ep = g_UsbAudio[port].rxpipe->byEP;
    uint8_t *p_buf;
    hal_usb_status_t dma_ret = HAL_USB_STATUS_ERROR;

    if (USB_Audio[port].initialized == false) {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio1_IsoOut_Hdr not be initialized", 0);
        return;
    }

    /* Check the rx byte is valid */
    nCount = USB_Audio_Get_RX_Bytes(port);
    fifo_len = hal_usb_get_rx_packet_length(rx_ep);

    if(nCount == 0 || fifo_len == 0){
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio1_IsoOut_Hdr rx bytes:%d or fifo byte:%d is 0, sample_rate:%d sample_size:%d channel:%d", 5,
                    nCount, fifo_len, g_usb_audio_sample_rate, g_usb_audio_sample_size, g_usb_audio_channel);
        hal_usb_clear_rx_endpoint_fifo(rx_ep, HAL_USB_EP_TRANSFER_ISO, false);
        hal_usb_set_endpoint_rx_ready(rx_ep);
        USB_Audio_Rx_Buffer_Drop_Bytes(port, fifo_len);
        return;
    }

    if(fifo_len > g_UsbAudio[port].rx_buffer_len ){
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio1_IsoOut_Hdr received data:%d is bigger than buffer size:%d, then flush USB FIFO", 2,
                    fifo_len, g_UsbAudio[port].rx_buffer_len);
        hal_usb_clear_rx_endpoint_fifo(rx_ep, HAL_USB_EP_TRANSFER_ISO, false);
        hal_usb_set_endpoint_rx_ready(rx_ep);
        USB_Audio_Rx_Buffer_Drop_Bytes(port, fifo_len);
        return;
    }

    /* Check the data is synchronous */
    if(fifo_len != nCount){
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio1_IsoOut_Hdr rx byte:%d != fifo byte:%d", 2, nCount, fifo_len);
    }

    /* Check USB RX IRQ per 1 sec */
    usb_rx1_cnt ++;
    if(usb_rx1_cnt%1000 == 0){
        if (USB_Audio[port].rx_cb){
            LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio1_IsoOut_Hdr IRQ Check - Enter RX Callback, rx byte:%d fifo byte:%d", 2, nCount, fifo_len);
        } else {
            LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio1_IsoOut_Hdr IRQ Check - RX Callback is NUll, rx byte:%d fifo byte:%d", 2, nCount, fifo_len);
        }
    }

    /* Always, write data from start address of rx buffer */
    p_buf = g_UsbAudio[port].rx_buffer;

    /* If nCount > 1024 then change to DMA mode */
#ifdef USB_AUDIO_RX_DMA_MODE
    if(0){
#else
    if(fifo_len < HAL_USB_MAX_PACKET_SIZE_ENDPOINT_ISOCHRONOUS_HIGH_SPEED){
#endif
        hal_usb_read_endpoint_fifo(rx_ep, fifo_len, p_buf);
    } else {
        hal_usb_start_dma_channel(rx_ep,
                                  HAL_USB_EP_DIRECTION_RX,
                                  HAL_USB_EP_TRANSFER_ISO,
                                  (void *)(hal_memview_mcu_to_infrasys((uint32_t)p_buf)),
                                  fifo_len,
                                  NULL, //hal_usb_dma_handler_t callback,
                                  NULL, //bool callback_upd_run,
                                  HAL_USB_DMA1_TYPE);

        /* Polling for DMA ready */
        uint32_t dma_timeout_start, dma_timeout_end, dma_timeout_duration;
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &dma_timeout_start);
        while(dma_ret != HAL_USB_STATUS_OK){
            dma_ret = hal_usb_is_dma_ready(rx_ep, HAL_USB_EP_DIRECTION_RX);

            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &dma_timeout_end);
            dma_timeout_duration = dma_timeout_end - dma_timeout_start;

            //LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio1_IsoOut_Hdr DMA is not ready %dus", 1, dma_timeout_duration);

            /* Timeout > 1ms */
            if(dma_timeout_duration > 1000){
                LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio1_IsoOut_Hdr wait DMA ready to timeout %dus", 1, dma_timeout_duration);
                return;
            }
        }
    }

#if defined(AIR_AUDIO_DUMP_ENABLE) && defined(USB_DBG) && defined(USB_AUDIO_DUMP_RX1)
    if (usb_dbg_is_opt_enable(USB_DBG_OPT_AUDIO_DUMP_RX1)) {
        LOG_AUDIO_DUMP(p_buf, fifo_len, USB_AUDIO_RX1);
    }
#endif

#if 0
    uint8_t *a;

    a = p_buf;

    LOG_MSGID_I(USBAUDIO_DRV, "USB [%02X][%02X][%02X][%02X][%02X][%02X][%02X][%02X] - 1s", 8,
                a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]);
    LOG_MSGID_I(USBAUDIO_DRV, "USB [%02X][%02X][%02X][%02X][%02X][%02X][%02X][%02X] - 1e", 8,
                a[552+0],a[552+1],a[552+2],a[552+3],a[552+4],a[552+5],a[552+6],a[552+7]);
    LOG_MSGID_I(USBAUDIO_DRV, "USB [%02X][%02X][%02X][%02X][%02X][%02X][%02X][%02X] - 2s", 8,
                a[576+0],a[576+1],a[576+2],a[576+3],a[576+4],a[576+5],a[576+6],a[576+7]);
    LOG_MSGID_I(USBAUDIO_DRV, "USB [%02X][%02X][%02X][%02X][%02X][%02X][%02X][%02X] - 2e", 8,
                a[576+0+552],a[576+1+552],a[576+2+552],a[576+3+552],a[576+4+552],a[576+5+552],a[576+6+552],a[576+7+552]);
#endif

    /* Use write pointer to record received data length */
    g_UsbAudio[port].rx_buffer_write = fifo_len;

#if defined(USB_DBG) && defined(USB_AUDIO_RX_TIME_MEASURE)
    if (usb_dbg_is_opt_enable(USB_DBG_OPT_AUDIO_RX_TIME_MEASURE)) {
        /* Timer2 - Get data */
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &usb_rx1_time2_end);
        time_gap1 = usb_rx1_time2_end - usb_rx1_time2_start;

        /* Timer2 - Audio callback */
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &usb_rx1_time2_start);
    }
#endif

    if (USB_Audio[port].rx_cb) {
        USB_DBG_I( USB_DBG_AUDIO_ISO_OUT, "USB_Audio1_IsoOut_Hdr enter RX callback", 0);
        hal_usb_set_endpoint_rx_ready(rx_ep);
        USB_Audio[port].rx_cb();
    } else {
        USB_DBG_I( USB_DBG_AUDIO_ISO_OUT, "USB_Audio1_IsoOut_Hdr RX callback is NULL", 0);
        hal_usb_set_endpoint_rx_ready(rx_ep);
        USB_Audio_Rx_Buffer_Drop_Bytes(port, fifo_len);
    }

#if defined(USB_DBG) && defined(USB_AUDIO_RX_TIME_MEASURE)
    if (usb_dbg_is_opt_enable(USB_DBG_OPT_AUDIO_RX_TIME_MEASURE)) {
        /* Timer2 - Audio callback */
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &usb_rx1_time2_end);
        time_gap2 = usb_rx1_time2_end - usb_rx1_time2_start;

        if((time_gap1 + time_gap2) > 500){
            LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio1_IsoOut_Hdr (1)Get data: %dus (2)audio callback: %dus > 500us", 2, time_gap1, time_gap2);
        } else {
            USB_DBG_I( USB_DBG_AUDIO_ISO_OUT, "USB_Audio1_IsoOut_Hdr (1)Get data: %dus (2)audio callback: %dus", 2, time_gap1, time_gap2);
        }

        SLA_CustomLogging("RX1", SA_STOP);
    }
#endif

    return;
}

/* EP Iso Out interrupt handler, called by EP interrupt */
void USB_Audio2_IsoOut_Hdr(void)
{
#if defined(USB_DBG) && defined(USB_AUDIO_RX_TIME_MEASURE)
    uint32_t time_gap = 0;
    uint32_t time_gap1 = 0, time_gap2 = 0;
    if (usb_dbg_is_opt_enable(USB_DBG_OPT_AUDIO_RX_TIME_MEASURE)) {
        SLA_CustomLogging("RX2", SA_START);
        SLA_CustomLogging("RX2", SA_LABEL);

        /* Timer1 - Check the ISO out is expected 1ms/transaction */
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &usb_rx2_time1_start);

        time_gap = usb_rx2_time1_start - usb_rx2_time1_end;

        if ((time_gap > 1500) && (time_gap < 5000)) {
            LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio2_IsoOut_Hdr is not 1ms time gap:%dus", 2, time_gap);
            if (usb_dbg_is_opt_enable(USB_DBG_OPT_AUDIO_RX_TIME_ASSERT)) {
                assert(0);
            }
        } else {
            USB_DBG_I( USB_DBG_AUDIO_ISO_OUT, "USB_Audio2_IsoOut_Hdr time measurement gap:%dus", 2, time_gap);
        }

        usb_rx2_time1_end = usb_rx2_time1_start;

        /* Timer2 - Check time consuming of each operation */
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &usb_rx2_time2_start);
    }
#endif

    uint32_t port = USB_AUDIO_2_PORT;
    uint32_t nCount, fifo_len;
    uint8_t rx_ep = g_UsbAudio[port].rxpipe->byEP;
    uint8_t *p_buf;

    if (USB_Audio[port].initialized == false) {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio2_IsoOut_Hdr was not be initialized", 0);
        return;
    }

    /* Check the rx byte is valid */
    nCount = USB_Audio_Get_RX_Bytes(port);
    fifo_len = hal_usb_get_rx_packet_length(rx_ep);

    if(nCount == 0 || fifo_len == 0){
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio2_IsoOut_Hdr rx bytes:%d or fifo byte:%d is 0, sample_rate:%d sample_size:%d channel:%d", 5,
                    nCount, fifo_len, g_usb_audio2_sample_rate, g_usb_audio2_sample_size, g_usb_audio2_channel);
        hal_usb_clear_rx_endpoint_fifo(rx_ep, HAL_USB_EP_TRANSFER_ISO, false);
        hal_usb_set_endpoint_rx_ready(rx_ep);
        USB_Audio_Rx_Buffer_Drop_Bytes(port, fifo_len);
        return;
    }

    if(fifo_len > g_UsbAudio[port].rx_buffer_len ){
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio2_IsoOut_Hdr received data:%d is bigger than buffer size:%d, then flush USB FIFO", 2,
                    fifo_len, g_UsbAudio[port].rx_buffer_len);
        hal_usb_clear_rx_endpoint_fifo(rx_ep, HAL_USB_EP_TRANSFER_ISO, false);
        hal_usb_set_endpoint_rx_ready(rx_ep);
        USB_Audio_Rx_Buffer_Drop_Bytes(port, fifo_len);
        return;
    }

    /* Check the data is synchronous */
    if(fifo_len != nCount){
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio2_IsoOut_Hdr rx byte:%d != fifo byte:%d", 2, nCount, fifo_len);
    }

    /* Check USB RX IRQ per 1 sec */
    usb_rx2_cnt ++;
    if(usb_rx2_cnt%1000 == 0){
        if (USB_Audio[port].rx_cb){
            LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio2_IsoOut_Hdr IRQ Check - Enter RX Callback, rx byte:%d fifo byte:%d", 2, nCount, fifo_len);
        } else {
            LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio2_IsoOut_Hdr IRQ Check - RX Callback is NUll, rx byte:%d fifo byte:%d", 2, nCount, fifo_len);
        }
    }

    /* Always, write data from start address of rx buffer */
    p_buf = g_UsbAudio[port].rx_buffer;

#ifdef USB_AUDIO_RX_DMA_MODE
    hal_usb_status_t dma_ret = HAL_USB_STATUS_ERROR;

    hal_usb_start_dma_channel(rx_ep,
                              HAL_USB_EP_DIRECTION_RX,
                              HAL_USB_EP_TRANSFER_ISO,
                              (void *)(hal_memview_mcu_to_infrasys((uint32_t)p_buf)),
                              fifo_len,
                              NULL, //hal_usb_dma_handler_t callback,
                              NULL, //bool callback_upd_run,
                              HAL_USB_DMA1_TYPE);

    /* Polling for DMA ready */
    uint32_t dma_timeout_start, dma_timeout_end, dma_timeout_duration;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &dma_timeout_start);
    while(dma_ret != HAL_USB_STATUS_OK){
        dma_ret = hal_usb_is_dma_ready(rx_ep, HAL_USB_EP_DIRECTION_RX);

        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &dma_timeout_end);
        dma_timeout_duration = dma_timeout_end - dma_timeout_start;

        //LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio2_IsoOut_Hdr DMA is not ready %dus", 1, dma_timeout_duration);

        /* Timeout > 1ms */
        if(dma_timeout_duration > 1000){
            LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio2_IsoOut_Hdr wait DMA ready to timeout %dus", 1, dma_timeout_duration);
            return;
        }
    }
#else
    hal_usb_read_endpoint_fifo(rx_ep, fifo_len, p_buf);
#endif

#if defined(AIR_AUDIO_DUMP_ENABLE) && defined(USB_DBG) && defined(USB_AUDIO_DUMP_RX2)
    if (usb_dbg_is_opt_enable(USB_DBG_OPT_AUDIO_DUMP_RX2)) {
        LOG_AUDIO_DUMP(p_buf, fifo_len, USB_AUDIO_RX2);
    }
#endif

    /* Use write pointer to record received data length */
    g_UsbAudio[port].rx_buffer_write = fifo_len;

#if defined(USB_DBG) && defined(USB_AUDIO_RX_TIME_MEASURE)
    if (usb_dbg_is_opt_enable(USB_DBG_OPT_AUDIO_RX_TIME_MEASURE)) {
        /* Timer2 - Get data */
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &usb_rx2_time2_end);
        time_gap1 = usb_rx2_time2_end - usb_rx2_time2_start;

        /* Timer2 - Audio callback */
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &usb_rx2_time2_start);
    }
#endif

    if (USB_Audio[port].rx_cb) {
        USB_DBG_I( USB_DBG_AUDIO_ISO_OUT, "USB_Audio2_IsoOut_Hdr enter RX callback", 0);
        hal_usb_set_endpoint_rx_ready(rx_ep);
        USB_Audio[port].rx_cb();
    } else {
        USB_DBG_I( USB_DBG_AUDIO_ISO_OUT, "USB_Audio2_IsoOut_Hdr RX callback is NULL", 0);
        hal_usb_set_endpoint_rx_ready(rx_ep);
        USB_Audio_Rx_Buffer_Drop_Bytes(port, fifo_len);
    }

#if defined(USB_DBG) && defined(USB_AUDIO_RX_TIME_MEASURE)
    if (usb_dbg_is_opt_enable(USB_DBG_OPT_AUDIO_RX_TIME_MEASURE)) {
        /* Timer2 - Audio callback */
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &usb_rx2_time2_end);
        time_gap2 = usb_rx2_time2_end - usb_rx2_time2_start;

        if((time_gap1 + time_gap2) > 500){
            LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio2_IsoOut_Hdr (1)Get data: %dus (2)audio callback: %dus > 500us", 2, time_gap1, time_gap2);
        } else {
            USB_DBG_I( USB_DBG_AUDIO_ISO_OUT, "USB_Audio2_IsoOut_Hdr (1)Get data: %dus (2)audio callback: %dus", 2, time_gap1, time_gap2);
        }

        SLA_CustomLogging("RX2", SA_STOP);
    }
#endif

}


/* EP Iso Out reset handler */
void USB_Audio1_IsoOut_Reset(void)
{
    g_UsbAudio[0].rxpipe = &g_UsbAudio[0].stream_ep_out_info->ep_status.epout_status;
}
void USB_Audio2_IsoOut_Reset(void)
{
    g_UsbAudio[1].rxpipe = &g_UsbAudio[1].stream_ep_out_info->ep_status.epout_status;
}

/************************************************************
    Iso EP IN handle functions
*************************************************************/
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
/* Set up TX, corresponding smaple size and channel */
uint32_t USB_Audio_Set_TX_Alt(uint32_t port, uint8_t alt_num)
{
    usb_audio_dscr_as_format_type_t *p = NULL;

    /* g_usb_audio_sample_rate set by host */
    if((port == 0) && (alt_num == 0 || alt_num == 1)){
        p = &audio1_mic_stream_format_dscr;
    }
    #ifdef USB_AUDIO_TX1_ALT2_ENABLE
    else if((port == 0) && (alt_num == 2)){
        p = &audio1_mic_stream_alt2_format_dscr;
    }
    #endif

    if (port == 0) {
        if (p) {
            g_usb_audio_mic_sample_size = p->bSubFrameSize;
            g_usb_audio_mic_channel = p->bNrChannels;
        }
        else {
            /* Invalid setting */
            g_usb_audio_mic_sample_size = 0;
            g_usb_audio_mic_channel     = 0;
        }
    }

    if(port == 0){
         LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Set_TX_Alt port:%d alt_num:%d sample_size:%d channel:%d", 4,
                     port, alt_num, g_usb_audio_mic_sample_size, g_usb_audio_mic_channel);
    } else {
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Set_TX_Alt port:%d is invalid", 4, port);
    }

    return 0;
}

uint8_t USB_Audio_Get_TX_Channel(uint32_t port)
{
    uint8_t channel = 0;

    if(port == 0){
        channel = g_usb_audio_mic_channel;
    }

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_TX_Channel port:%d channel:%d", 2, port, channel);
    return channel;
}

uint8_t USB_Audio_Get_TX_Sample_Size(uint32_t port)
{
    uint8_t sample_size = 0;

    if(port == 0){
        sample_size = g_usb_audio_mic_sample_size;
    }

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_TX_Sample_Size port:%d sample_size:%d ", 2, port, sample_size);
    return sample_size;
}

uint32_t USB_Audio_Get_TX_Sample_Rate(uint32_t port)
{
    uint32_t sample_rate = 0;

    if (USB_Get_Device_State() == DEVSTATE_CONFIG) {
        if (port == 0) {
            sample_rate = g_usb_audio_mic_sample_rate;
        }
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_TX_Sample_Size port:%d sample_size:%d", 2, port, sample_rate);
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Get_TX_Sample_Size port:%d sample_rate:%d USB state:%d is not ready", 3,
                    port, sample_rate, USB_Get_Device_State());
    }

    return sample_rate;
}

uint32_t USB_Audio_Get_TX_Sample_Rate_Max(uint32_t port, uint8_t alt_num)
{
    uint32_t sample_rate_max = 0;
    usb_audio_dscr_as_format_type_t *p = NULL;



    if((port == 0) && (alt_num == 1)){
        p = &audio1_mic_stream_format_dscr;
    }
    #ifdef USB_AUDIO_TX1_ALT2_ENABLE
    else if ((port == 0) && (alt_num == 2)){
        p = &audio1_mic_stream_alt2_format_dscr;
    }
    #endif


    if(p){
        sample_rate_max = Get_Maximum(AUDFREQ_TO_U32(p->tSamFreq[0]),
                                      AUDFREQ_TO_U32(p->tSamFreq[1]),
                                      AUDFREQ_TO_U32(p->tSamFreq[2]),
                                      AUDFREQ_TO_U32(p->tSamFreq[3]),
                                      AUDFREQ_TO_U32(p->tSamFreq[4]));
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Get_TX_Sample_Rate_Max port:%d sample rate = 0 ERROR !!", 1, port);
        sample_rate_max = 0;
    }

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_TX_Sample_Rate_Max port:%d sample rate max:%d", 2, port, sample_rate_max);
    return sample_rate_max;
}

uint32_t USB_Audio_Get_TX_Alt_Byte(uint32_t port, uint8_t alt_num)
{
    /* byte = sample rate x sample size x channel */
    uint32_t byte = 0;
    uint32_t sample_rate_max;
    usb_audio_dscr_as_format_type_t *p = NULL;

    /* Unint: K HZ */
    sample_rate_max = USB_Audio_Get_TX_Sample_Rate_Max(port, alt_num);
    sample_rate_max /= 1000;

    /* Microphone 1 */
    if((port == 0) && (alt_num == 1)) {
        p = &audio1_mic_stream_format_dscr;
    }
    #ifdef USB_AUDIO_TX1_ALT2_ENABLE
    else if((port == 0) && (alt_num == 2)){
         p = &audio1_mic_stream_alt2_format_dscr;
    }
    #endif

    if(p) {
        byte = sample_rate_max * p->bSubFrameSize * p->bNrChannels;
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Get_TX_Alt_Byte port:%d alt_num:%d byte is 0", 2, port, alt_num);
        return byte;
    }

    /* USB 2.0 Spec - 5.9.2 High Bandwidth Isochronous Endpoints */
    uint8_t hb_packet = byte / HAL_USB_MAX_PACKET_SIZE_ENDPOINT_ISOCHRONOUS_HIGH_SPEED;
    if(hb_packet != 0){

       /* USB 2.0 Spec - 9.6.6 Endpoint
        * Bits 12..11 specify the number of additional transaction opportunities per microframe:
        * 00 = None (1 transaction per microframe)
        * 01 = 1 additional (2 per microframe)
        * 10 = 2 additional (3 per microframe)
        * 11 = Reserved
        */
        byte /= (hb_packet + 1); /* Convert 1 additional to 2 per microframe e.g. 1152 = 576 + 576 */
        byte |= hb_packet << USB_RXMAP_M1_POS;
    }

    LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Get_TX_Alt_Byte port:%d alt_num:%d byte:%d hb_packet:%d", 4,
               port, alt_num, byte%HAL_USB_MAX_PACKET_SIZE_ENDPOINT_ISOCHRONOUS_HIGH_SPEED, hb_packet);

    /* Include data_size packet inb bits 12..11 */
    return byte;
}

uint32_t USB_Audio_Get_TX_Bytes(uint32_t port)
{
    /* byte = sample rate x sample size x channel */
    uint32_t byte = 0;

    if(port == 0){
        byte =(g_usb_audio_mic_sample_rate/1000) * g_usb_audio_mic_sample_size * g_usb_audio_mic_channel;
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Get_RX_Bytes port:%d is not valid", 1, port);
    }

    return byte;
}

#ifdef USB_TEST_MIC
#define USB_AUDIO_TEST_TONE
#endif

/* Create maxiumn size of tx buffer for mute data */
static const uint8_t mic_mute_data[USB_AUDIO_TX_EP_MAX_SIZE] = {0};

/* Send audio data API by endpoint */
uint32_t USB_Audio_TX_SendData(uint32_t port, uint32_t len , uint8_t *data)
{
    uint32_t ep_id = g_UsbAudio[port].stream_ep_in_id;
    uint32_t ret = len;
    uint32_t nCount;

    USB_DBG_I( USB_DBG_AUDIO_TX, "USB_Audio_TX_SendData ep_in_id[%x] data[%x] len[%x]", 3, ep_id, data, len);

    /* Check the length is valid */
    if (len == 0) {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_TX_SendData Data length is 0", 0);
    }

    nCount = USB_Audio_Get_TX_Bytes(port);
    if (len != nCount) {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_TX_SendData Data length %d byte dismatch with %d byte", 2, len, nCount);
    }

#if defined(AIR_AUDIO_DUMP_ENABLE) && defined(USB_DBG) && defined(USB_AUDIO_DUMP_TX1)
    if (usb_dbg_is_opt_enable(USB_DBG_OPT_AUDIO_DUMP_TX1)) {
        LOG_AUDIO_DUMP(data, len, USB_AUDIO_TX1);
    }
#endif

    /* Set data from share buffer into USB FIFO */
    hal_usb_write_endpoint_fifo(ep_id, len, (void *)data);

    /* Send usb data */
    hal_usb_set_endpoint_tx_ready(ep_id);

    USB_DBG_I( USB_DBG_AUDIO_TX, "USB_Audio_TX_SendData [%02X][%02X][%02X][%02X][%02X][%02X][%02X][%02X]", 8,
                data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);

    return ret;
}


/************************************************************
    Iso EP IN handler
*************************************************************/
#if defined(USB_DBG) && defined(USB_AUDIO_TX_TIME_MEASURE)
uint32_t usb_tx1_time1_start = 0;
uint32_t usb_tx1_time1_end  = 0;
uint32_t usb_tx1_time2_start = 0;
uint32_t usb_tx1_time2_end  = 0;
#endif

#ifdef USB_AUDIO_TEST_TONE

// test_16k_16bits_sin_tone_1Khz
const unsigned char test_16k_16bits_sin_tone_1Khz[32] = {
    0x00, 0x00, 0xD8, 0x2C, 0xDD, 0x52, 0x44, 0x6C, 0x30, 0x75, 0x44, 0x6C,
    0xDD, 0x52, 0xD8, 0x2C, 0x00, 0x00, 0x28, 0xD3, 0x23, 0xAD, 0xBC, 0x93,
    0xD0, 0x8A, 0xBC, 0x93, 0x23, 0xAD, 0x28, 0xD3};

// test_48k_16bits_sin_tone_1Khz
const uint8_t test_48k_16bits_sin_tone_1Khz[96] = {
    0x00, 0x00, 0x4B, 0x0F, 0x54, 0x1E, 0xD8, 0x2C, 0x97, 0x3A, 0x56, 0x47,
    0xDD, 0x52, 0xF8, 0x5C, 0x7C, 0x65, 0x44, 0x6C, 0x31, 0x71, 0x2F, 0x74,
    0x30, 0x75, 0x2F, 0x74, 0x31, 0x71, 0x44, 0x6C, 0x7C, 0x65, 0xF8, 0x5C,
    0xDD, 0x52, 0x56, 0x47, 0x98, 0x3A, 0xD8, 0x2C, 0x54, 0x1E, 0x4B, 0x0F,
    0x00, 0x00, 0xB5, 0xF0, 0xAC, 0xE1, 0x28, 0xD3, 0x69, 0xC5, 0xAA, 0xB8,
    0x23, 0xAD, 0x08, 0xA3, 0x84, 0x9A, 0xBC, 0x93, 0xCF, 0x8E, 0xD1, 0x8B,
    0xD0, 0x8A, 0xD1, 0x8B, 0xCF, 0x8E, 0xBC, 0x93, 0x84, 0x9A, 0x08, 0xA3,
    0x23, 0xAD, 0xAA, 0xB8, 0x68, 0xC5, 0x28, 0xD3, 0xAC, 0xE1, 0xB5, 0xF0};

// test_96k_16bits_sin_tone_1Khz
const uint8_t test_96k_16bits_sin_tone_1Khz[192] = {
    0x00, 0x00, 0xAA, 0x07, 0x4B, 0x0F, 0xDC, 0x16, 0x54, 0x1E, 0xAB, 0x25,
    0xD8, 0x2C, 0xD4, 0x33, 0x97, 0x3A, 0x1B, 0x41, 0x56, 0x47, 0x44, 0x4D,
    0xDD, 0x52, 0x1B, 0x58, 0xF8, 0x5C, 0x70, 0x61, 0x7C, 0x65, 0x1A, 0x69,
    0x44, 0x6C, 0xF7, 0x6E, 0x31, 0x71, 0xEF, 0x72, 0x2F, 0x74, 0xEF, 0x74,
    0x30, 0x75, 0xEF, 0x74, 0x2F, 0x74, 0xEF, 0x72, 0x31, 0x71, 0xF7, 0x6E,
    0x44, 0x6C, 0x1A, 0x69, 0x7C, 0x65, 0x70, 0x61, 0xF8, 0x5C, 0x1B, 0x58,
    0xDD, 0x52, 0x44, 0x4D, 0x56, 0x47, 0x1B, 0x41, 0x98, 0x3A, 0xD4, 0x33,
    0xD8, 0x2C, 0xAB, 0x25, 0x54, 0x1E, 0xDC, 0x16, 0x4B, 0x0F, 0xAA, 0x07,
    0x00, 0x00, 0x56, 0xF8, 0xB5, 0xF0, 0x24, 0xE9, 0xAC, 0xE1, 0x55, 0xDA,
    0x28, 0xD3, 0x2C, 0xCC, 0x69, 0xC5, 0xE5, 0xBE, 0xAA, 0xB8, 0xBC, 0xB2,
    0x23, 0xAD, 0xE5, 0xA7, 0x08, 0xA3, 0x90, 0x9E, 0x84, 0x9A, 0xE6, 0x96,
    0xBC, 0x93, 0x09, 0x91, 0xCF, 0x8E, 0x11, 0x8D, 0xD1, 0x8B, 0x11, 0x8B,
    0xD0, 0x8A, 0x11, 0x8B, 0xD1, 0x8B, 0x11, 0x8D, 0xCF, 0x8E, 0x09, 0x91,
    0xBC, 0x93, 0xE6, 0x96, 0x84, 0x9A, 0x90, 0x9E, 0x08, 0xA3, 0xE5, 0xA7,
    0x23, 0xAD, 0xBC, 0xB2, 0xAA, 0xB8, 0xE5, 0xBE, 0x68, 0xC5, 0x2C, 0xCC,
    0x28, 0xD3, 0x55, 0xDA, 0xAC, 0xE1, 0x24, 0xE9, 0xB5, 0xF0, 0x56, 0xF8};

// test_192k_16bits_sin_tone_1Khz
const uint8_t test_192k_16bits_sin_tone_1Khz[384] = {
    0x00, 0x00, 0xD5, 0x03, 0xAA, 0x07, 0x7C, 0x0B, 0x4B, 0x0F, 0x16, 0x13,
    0xDC, 0x16, 0x9C, 0x1A, 0x54, 0x1E, 0x04, 0x22, 0xAB, 0x25, 0x47, 0x29,
    0xD8, 0x2C, 0x5D, 0x30, 0xD4, 0x33, 0x3D, 0x37, 0x97, 0x3A, 0xE2, 0x3D,
    0x1B, 0x41, 0x42, 0x44, 0x56, 0x47, 0x57, 0x4A, 0x44, 0x4D, 0x1B, 0x50,
    0xDD, 0x52, 0x87, 0x55, 0x1B, 0x58, 0x96, 0x5A, 0xF8, 0x5C, 0x41, 0x5F,
    0x70, 0x61, 0x84, 0x63, 0x7C, 0x65, 0x59, 0x67, 0x1A, 0x69, 0xBD, 0x6A,
    0x44, 0x6C, 0xAD, 0x6D, 0xF7, 0x6E, 0x24, 0x70, 0x31, 0x71, 0x20, 0x72,
    0xEF, 0x72, 0x9F, 0x73, 0x2F, 0x74, 0x9F, 0x74, 0xEF, 0x74, 0x1F, 0x75,
    0x30, 0x75, 0x1F, 0x75, 0xEF, 0x74, 0x9F, 0x74, 0x2F, 0x74, 0x9F, 0x73,
    0xEF, 0x72, 0x20, 0x72, 0x31, 0x71, 0x24, 0x70, 0xF7, 0x6E, 0xAD, 0x6D,
    0x44, 0x6C, 0xBD, 0x6A, 0x1A, 0x69, 0x59, 0x67, 0x7C, 0x65, 0x84, 0x63,
    0x70, 0x61, 0x41, 0x5F, 0xF8, 0x5C, 0x96, 0x5A, 0x1B, 0x58, 0x87, 0x55,
    0xDD, 0x52, 0x1B, 0x50, 0x44, 0x4D, 0x57, 0x4A, 0x56, 0x47, 0x42, 0x44,
    0x1B, 0x41, 0xE2, 0x3D, 0x98, 0x3A, 0x3D, 0x37, 0xD4, 0x33, 0x5D, 0x30,
    0xD8, 0x2C, 0x47, 0x29, 0xAB, 0x25, 0x04, 0x22, 0x54, 0x1E, 0x9C, 0x1A,
    0xDC, 0x16, 0x16, 0x13, 0x4B, 0x0F, 0x7C, 0x0B, 0xAA, 0x07, 0xD5, 0x03,
    0x00, 0x00, 0x2B, 0xFC, 0x56, 0xF8, 0x84, 0xF4, 0xB5, 0xF0, 0xEA, 0xEC,
    0x24, 0xE9, 0x64, 0xE5, 0xAC, 0xE1, 0xFC, 0xDD, 0x55, 0xDA, 0xB9, 0xD6,
    0x28, 0xD3, 0xA3, 0xCF, 0x2C, 0xCC, 0xC3, 0xC8, 0x69, 0xC5, 0x1E, 0xC2,
    0xE5, 0xBE, 0xBE, 0xBB, 0xAA, 0xB8, 0xA9, 0xB5, 0xBC, 0xB2, 0xE5, 0xAF,
    0x23, 0xAD, 0x79, 0xAA, 0xE5, 0xA7, 0x6A, 0xA5, 0x08, 0xA3, 0xBF, 0xA0,
    0x90, 0x9E, 0x7C, 0x9C, 0x84, 0x9A, 0xA7, 0x98, 0xE6, 0x96, 0x43, 0x95,
    0xBC, 0x93, 0x53, 0x92, 0x09, 0x91, 0xDC, 0x8F, 0xCF, 0x8E, 0xE0, 0x8D,
    0x11, 0x8D, 0x61, 0x8C, 0xD1, 0x8B, 0x61, 0x8B, 0x11, 0x8B, 0xE1, 0x8A,
    0xD0, 0x8A, 0xE1, 0x8A, 0x11, 0x8B, 0x61, 0x8B, 0xD1, 0x8B, 0x61, 0x8C,
    0x11, 0x8D, 0xE0, 0x8D, 0xCF, 0x8E, 0xDC, 0x8F, 0x09, 0x91, 0x53, 0x92,
    0xBC, 0x93, 0x43, 0x95, 0xE6, 0x96, 0xA7, 0x98, 0x84, 0x9A, 0x7C, 0x9C,
    0x90, 0x9E, 0xBF, 0xA0, 0x08, 0xA3, 0x6A, 0xA5, 0xE5, 0xA7, 0x79, 0xAA,
    0x23, 0xAD, 0xE5, 0xAF, 0xBC, 0xB2, 0xA9, 0xB5, 0xAA, 0xB8, 0xBE, 0xBB,
    0xE5, 0xBE, 0x1E, 0xC2, 0x68, 0xC5, 0xC3, 0xC8, 0x2C, 0xCC, 0xA3, 0xCF,
    0x28, 0xD3, 0xB9, 0xD6, 0x55, 0xDA, 0xFC, 0xDD, 0xAC, 0xE1, 0x64, 0xE5,
    0x24, 0xE9, 0xEA, 0xEC, 0xB5, 0xF0, 0x84, 0xF4, 0x56, 0xF8, 0x2B, 0xFC};

// test_48k_24bits_sin_tone_1Khz
const uint8_t test_48k_24bits_sin_tone_1Khz[144] = {
    0x00, 0x00, 0x00, 0x00, 0x4B, 0x0F, 0x00, 0x54, 0x1E, 0x00, 0xD8, 0x2C,
    0x00, 0x97, 0x3A, 0x00, 0x56, 0x47, 0x00, 0xDD, 0x52, 0x00, 0xF8, 0x5C,
    0x00, 0x7C, 0x65, 0x00, 0x44, 0x6C, 0x00, 0x31, 0x71, 0x00, 0x2F, 0x74,
    0x00, 0x30, 0x75, 0x00, 0x2F, 0x74, 0x00, 0x31, 0x71, 0x00, 0x44, 0x6C,
    0x00, 0x7C, 0x65, 0x00, 0xF8, 0x5C, 0x00, 0xDD, 0x52, 0x00, 0x56, 0x47,
    0x00, 0x98, 0x3A, 0x00, 0xD8, 0x2C, 0x00, 0x54, 0x1E, 0x00, 0x4B, 0x0F,
    0x00, 0x00, 0x00, 0x00, 0xB5, 0xF0, 0x00, 0xAC, 0xE1, 0x00, 0x28, 0xD3,
    0x00, 0x69, 0xC5, 0x00, 0xAA, 0xB8, 0x00, 0x23, 0xAD, 0x00, 0x08, 0xA3,
    0x00, 0x84, 0x9A, 0x00, 0xBC, 0x93, 0x00, 0xCF, 0x8E, 0x00, 0xD1, 0x8B,
    0x00, 0xD0, 0x8A, 0x00, 0xD1, 0x8B, 0x00, 0xCF, 0x8E, 0x00, 0xBC, 0x93,
    0x00, 0x84, 0x9A, 0x00, 0x08, 0xA3, 0x00, 0x23, 0xAD, 0x00, 0xAA, 0xB8,
    0x00, 0x68, 0xC5, 0x00, 0x28, 0xD3, 0x00, 0xAC, 0xE1, 0x00, 0xB5, 0xF0};

// test_96k_24bits_sin_tone_1Khz
const uint8_t test_96k_24bits_sin_tone_1Khz[288] = {
    0x00, 0x00, 0x00, 0x00, 0xAA, 0x07, 0x00, 0x4B, 0x0F, 0x00, 0xDC, 0x16,
    0x00, 0x54, 0x1E, 0x00, 0xAB, 0x25, 0x00, 0xD8, 0x2C, 0x00, 0xD4, 0x33,
    0x00, 0x97, 0x3A, 0x00, 0x1B, 0x41, 0x00, 0x56, 0x47, 0x00, 0x44, 0x4D,
    0x00, 0xDD, 0x52, 0x00, 0x1B, 0x58, 0x00, 0xF8, 0x5C, 0x00, 0x70, 0x61,
    0x00, 0x7C, 0x65, 0x00, 0x1A, 0x69, 0x00, 0x44, 0x6C, 0x00, 0xF7, 0x6E,
    0x00, 0x31, 0x71, 0x00, 0xEF, 0x72, 0x00, 0x2F, 0x74, 0x00, 0xEF, 0x74,
    0x00, 0x30, 0x75, 0x00, 0xEF, 0x74, 0x00, 0x2F, 0x74, 0x00, 0xEF, 0x72,
    0x00, 0x31, 0x71, 0x00, 0xF7, 0x6E, 0x00, 0x44, 0x6C, 0x00, 0x1A, 0x69,
    0x00, 0x7C, 0x65, 0x00, 0x70, 0x61, 0x00, 0xF8, 0x5C, 0x00, 0x1B, 0x58,
    0x00, 0xDD, 0x52, 0x00, 0x44, 0x4D, 0x00, 0x56, 0x47, 0x00, 0x1B, 0x41,
    0x00, 0x98, 0x3A, 0x00, 0xD4, 0x33, 0x00, 0xD8, 0x2C, 0x00, 0xAB, 0x25,
    0x00, 0x54, 0x1E, 0x00, 0xDC, 0x16, 0x00, 0x4B, 0x0F, 0x00, 0xAA, 0x07,
    0x00, 0x00, 0x00, 0x00, 0x56, 0xF8, 0x00, 0xB5, 0xF0, 0x00, 0x24, 0xE9,
    0x00, 0xAC, 0xE1, 0x00, 0x55, 0xDA, 0x00, 0x28, 0xD3, 0x00, 0x2C, 0xCC,
    0x00, 0x69, 0xC5, 0x00, 0xE5, 0xBE, 0x00, 0xAA, 0xB8, 0x00, 0xBC, 0xB2,
    0x00, 0x23, 0xAD, 0x00, 0xE5, 0xA7, 0x00, 0x08, 0xA3, 0x00, 0x90, 0x9E,
    0x00, 0x84, 0x9A, 0x00, 0xE6, 0x96, 0x00, 0xBC, 0x93, 0x00, 0x09, 0x91,
    0x00, 0xCF, 0x8E, 0x00, 0x11, 0x8D, 0x00, 0xD1, 0x8B, 0x00, 0x11, 0x8B,
    0x00, 0xD0, 0x8A, 0x00, 0x11, 0x8B, 0x00, 0xD1, 0x8B, 0x00, 0x11, 0x8D,
    0x00, 0xCF, 0x8E, 0x00, 0x09, 0x91, 0x00, 0xBC, 0x93, 0x00, 0xE6, 0x96,
    0x00, 0x84, 0x9A, 0x00, 0x90, 0x9E, 0x00, 0x08, 0xA3, 0x00, 0xE5, 0xA7,
    0x00, 0x23, 0xAD, 0x00, 0xBC, 0xB2, 0x00, 0xAA, 0xB8, 0x00, 0xE5, 0xBE,
    0x00, 0x68, 0xC5, 0x00, 0x2C, 0xCC, 0x00, 0x28, 0xD3, 0x00, 0x55, 0xDA,
    0x00, 0xAC, 0xE1, 0x00, 0x24, 0xE9, 0x00, 0xB5, 0xF0, 0x00, 0x56, 0xF8};

// test_192k_24bits_sin_tone_1Khz
const uint8_t test_192k_24bits_sin_tone_1Khz[576] = {
    0x00, 0x00, 0x00, 0x00, 0xD5, 0x03, 0x00, 0xAA, 0x07, 0x00, 0x7C, 0x0B,
    0x00, 0x4B, 0x0F, 0x00, 0x16, 0x13, 0x00, 0xDC, 0x16, 0x00, 0x9C, 0x1A,
    0x00, 0x54, 0x1E, 0x00, 0x04, 0x22, 0x00, 0xAB, 0x25, 0x00, 0x47, 0x29,
    0x00, 0xD8, 0x2C, 0x00, 0x5D, 0x30, 0x00, 0xD4, 0x33, 0x00, 0x3D, 0x37,
    0x00, 0x97, 0x3A, 0x00, 0xE2, 0x3D, 0x00, 0x1B, 0x41, 0x00, 0x42, 0x44,
    0x00, 0x56, 0x47, 0x00, 0x57, 0x4A, 0x00, 0x44, 0x4D, 0x00, 0x1B, 0x50,
    0x00, 0xDD, 0x52, 0x00, 0x87, 0x55, 0x00, 0x1B, 0x58, 0x00, 0x96, 0x5A,
    0x00, 0xF8, 0x5C, 0x00, 0x41, 0x5F, 0x00, 0x70, 0x61, 0x00, 0x84, 0x63,
    0x00, 0x7C, 0x65, 0x00, 0x59, 0x67, 0x00, 0x1A, 0x69, 0x00, 0xBD, 0x6A,
    0x00, 0x44, 0x6C, 0x00, 0xAD, 0x6D, 0x00, 0xF7, 0x6E, 0x00, 0x24, 0x70,
    0x00, 0x31, 0x71, 0x00, 0x20, 0x72, 0x00, 0xEF, 0x72, 0x00, 0x9F, 0x73,
    0x00, 0x2F, 0x74, 0x00, 0x9F, 0x74, 0x00, 0xEF, 0x74, 0x00, 0x1F, 0x75,
    0x00, 0x30, 0x75, 0x00, 0x1F, 0x75, 0x00, 0xEF, 0x74, 0x00, 0x9F, 0x74,
    0x00, 0x2F, 0x74, 0x00, 0x9F, 0x73, 0x00, 0xEF, 0x72, 0x00, 0x20, 0x72,
    0x00, 0x31, 0x71, 0x00, 0x24, 0x70, 0x00, 0xF7, 0x6E, 0x00, 0xAD, 0x6D,
    0x00, 0x44, 0x6C, 0x00, 0xBD, 0x6A, 0x00, 0x1A, 0x69, 0x00, 0x59, 0x67,
    0x00, 0x7C, 0x65, 0x00, 0x84, 0x63, 0x00, 0x70, 0x61, 0x00, 0x41, 0x5F,
    0x00, 0xF8, 0x5C, 0x00, 0x96, 0x5A, 0x00, 0x1B, 0x58, 0x00, 0x87, 0x55,
    0x00, 0xDD, 0x52, 0x00, 0x1B, 0x50, 0x00, 0x44, 0x4D, 0x00, 0x57, 0x4A,
    0x00, 0x56, 0x47, 0x00, 0x42, 0x44, 0x00, 0x1B, 0x41, 0x00, 0xE2, 0x3D,
    0x00, 0x98, 0x3A, 0x00, 0x3D, 0x37, 0x00, 0xD4, 0x33, 0x00, 0x5D, 0x30,
    0x00, 0xD8, 0x2C, 0x00, 0x47, 0x29, 0x00, 0xAB, 0x25, 0x00, 0x04, 0x22,
    0x00, 0x54, 0x1E, 0x00, 0x9C, 0x1A, 0x00, 0xDC, 0x16, 0x00, 0x16, 0x13,
    0x00, 0x4B, 0x0F, 0x00, 0x7C, 0x0B, 0x00, 0xAA, 0x07, 0x00, 0xD5, 0x03,
    0x00, 0x00, 0x00, 0x00, 0x2B, 0xFC, 0x00, 0x56, 0xF8, 0x00, 0x84, 0xF4,
    0x00, 0xB5, 0xF0, 0x00, 0xEA, 0xEC, 0x00, 0x24, 0xE9, 0x00, 0x64, 0xE5,
    0x00, 0xAC, 0xE1, 0x00, 0xFC, 0xDD, 0x00, 0x55, 0xDA, 0x00, 0xB9, 0xD6,
    0x00, 0x28, 0xD3, 0x00, 0xA3, 0xCF, 0x00, 0x2C, 0xCC, 0x00, 0xC3, 0xC8,
    0x00, 0x69, 0xC5, 0x00, 0x1E, 0xC2, 0x00, 0xE5, 0xBE, 0x00, 0xBE, 0xBB,
    0x00, 0xAA, 0xB8, 0x00, 0xA9, 0xB5, 0x00, 0xBC, 0xB2, 0x00, 0xE5, 0xAF,
    0x00, 0x23, 0xAD, 0x00, 0x79, 0xAA, 0x00, 0xE5, 0xA7, 0x00, 0x6A, 0xA5,
    0x00, 0x08, 0xA3, 0x00, 0xBF, 0xA0, 0x00, 0x90, 0x9E, 0x00, 0x7C, 0x9C,
    0x00, 0x84, 0x9A, 0x00, 0xA7, 0x98, 0x00, 0xE6, 0x96, 0x00, 0x43, 0x95,
    0x00, 0xBC, 0x93, 0x00, 0x53, 0x92, 0x00, 0x09, 0x91, 0x00, 0xDC, 0x8F,
    0x00, 0xCF, 0x8E, 0x00, 0xE0, 0x8D, 0x00, 0x11, 0x8D, 0x00, 0x61, 0x8C,
    0x00, 0xD1, 0x8B, 0x00, 0x61, 0x8B, 0x00, 0x11, 0x8B, 0x00, 0xE1, 0x8A,
    0x00, 0xD0, 0x8A, 0x00, 0xE1, 0x8A, 0x00, 0x11, 0x8B, 0x00, 0x61, 0x8B,
    0x00, 0xD1, 0x8B, 0x00, 0x61, 0x8C, 0x00, 0x11, 0x8D, 0x00, 0xE0, 0x8D,
    0x00, 0xCF, 0x8E, 0x00, 0xDC, 0x8F, 0x00, 0x09, 0x91, 0x00, 0x53, 0x92,
    0x00, 0xBC, 0x93, 0x00, 0x43, 0x95, 0x00, 0xE6, 0x96, 0x00, 0xA7, 0x98,
    0x00, 0x84, 0x9A, 0x00, 0x7C, 0x9C, 0x00, 0x90, 0x9E, 0x00, 0xBF, 0xA0,
    0x00, 0x08, 0xA3, 0x00, 0x6A, 0xA5, 0x00, 0xE5, 0xA7, 0x00, 0x79, 0xAA,
    0x00, 0x23, 0xAD, 0x00, 0xE5, 0xAF, 0x00, 0xBC, 0xB2, 0x00, 0xA9, 0xB5,
    0x00, 0xAA, 0xB8, 0x00, 0xBE, 0xBB, 0x00, 0xE5, 0xBE, 0x00, 0x1E, 0xC2,
    0x00, 0x68, 0xC5, 0x00, 0xC3, 0xC8, 0x00, 0x2C, 0xCC, 0x00, 0xA3, 0xCF,
    0x00, 0x28, 0xD3, 0x00, 0xB9, 0xD6, 0x00, 0x55, 0xDA, 0x00, 0xFC, 0xDD,
    0x00, 0xAC, 0xE1, 0x00, 0x64, 0xE5, 0x00, 0x24, 0xE9, 0x00, 0xEA, 0xEC,
    0x00, 0xB5, 0xF0, 0x00, 0x84, 0xF4, 0x00, 0x56, 0xF8, 0x00, 0x2B, 0xFC};

uint8_t* TestTone_Covert(void)
{
    static uint8_t s_tmp[2048];

    if( (g_usb_audio_mic_channel==2) && (g_usb_audio_mic_sample_size==3) ) {

        if(g_usb_audio_mic_sample_rate==48000) {

            assert( sizeof(test_48k_24bits_sin_tone_1Khz)%3==0 );

            uint32_t index = 0;
            for(uint32_t i=0;i<sizeof(test_48k_24bits_sin_tone_1Khz); i+=3)
            {
                s_tmp[index++] = test_48k_24bits_sin_tone_1Khz[i+0];
                s_tmp[index++] = test_48k_24bits_sin_tone_1Khz[i+1];
                s_tmp[index++] = test_48k_24bits_sin_tone_1Khz[i+2];

                s_tmp[index++] = test_48k_24bits_sin_tone_1Khz[i+0];
                s_tmp[index++] = test_48k_24bits_sin_tone_1Khz[i+1];
                s_tmp[index++] = test_48k_24bits_sin_tone_1Khz[i+2];
            };

            assert( sizeof(s_tmp) > index );
            //LOG_MSGID_I(USBAUDIO_DRV, "TestTone_Covert 48K-24b 2Ch:%d (expect:288) ", 1, index);
            return s_tmp;
        }
        else if(g_usb_audio_mic_sample_rate==96000) {

            assert( sizeof(test_96k_24bits_sin_tone_1Khz)%3==0 );

            uint32_t index = 0;
            for(uint32_t i=0;i<sizeof(test_96k_24bits_sin_tone_1Khz); i+=3)
            {
                s_tmp[index++] = test_96k_24bits_sin_tone_1Khz[i+0];
                s_tmp[index++] = test_96k_24bits_sin_tone_1Khz[i+1];
                s_tmp[index++] = test_96k_24bits_sin_tone_1Khz[i+2];

                s_tmp[index++] = test_96k_24bits_sin_tone_1Khz[i+0];
                s_tmp[index++] = test_96k_24bits_sin_tone_1Khz[i+1];
                s_tmp[index++] = test_96k_24bits_sin_tone_1Khz[i+2];
            };

            assert( sizeof(s_tmp) > index );
            //LOG_MSGID_I(USBAUDIO_DRV, "TestTone_Covert 96K-24b 2Ch:%d (expect:576) ", 1, index);
            return s_tmp;
        }
        else if(g_usb_audio_mic_sample_rate==192000) {

            assert( sizeof(test_192k_24bits_sin_tone_1Khz)%3==0 );

            uint32_t index = 0;
            for(uint32_t i=0;i<sizeof(test_192k_24bits_sin_tone_1Khz); i+=3)
            {
                s_tmp[index++] = test_192k_24bits_sin_tone_1Khz[i+0];
                s_tmp[index++] = test_192k_24bits_sin_tone_1Khz[i+1];
                s_tmp[index++] = test_192k_24bits_sin_tone_1Khz[i+2];

                s_tmp[index++] = test_192k_24bits_sin_tone_1Khz[i+0];
                s_tmp[index++] = test_192k_24bits_sin_tone_1Khz[i+1];
                s_tmp[index++] = test_192k_24bits_sin_tone_1Khz[i+2];
            };

            assert( sizeof(s_tmp) > index );
            //LOG_MSGID_I(USBAUDIO_DRV, "TestTone_Covert 192K-24b 2Ch:%d (expect:1152) ", 1, index);
            return s_tmp;
        }

    }

    return 0;
}

static uint32_t _USB_Audio_IsoIn_Hdr_TestTone(uint32_t port,uint32_t nCount)
{
    uint32_t ret = 0;

    if (g_usb_audio_mic_channel==1) {
        if (g_usb_audio_mic_sample_size==2) {
            switch( g_usb_audio_mic_sample_rate ) {
                case 16000: {
                    ret = USB_Audio_TX_SendData(port, nCount, (uint8_t *)test_16k_16bits_sin_tone_1Khz);
                    break;
                }
                case 48000: {
                    ret = USB_Audio_TX_SendData(port, nCount, (uint8_t *)test_48k_16bits_sin_tone_1Khz);
                    break;
                }
                case 96000: {
                    ret = USB_Audio_TX_SendData(port, nCount, (uint8_t *)test_96k_16bits_sin_tone_1Khz);
                    break;
                }
                case 192000: {
                    ret = USB_Audio_TX_SendData(port, nCount, (uint8_t *)test_192k_16bits_sin_tone_1Khz);
                    break;
                }
            }
        } else if (g_usb_audio_mic_sample_size==3) {
            switch( g_usb_audio_mic_sample_rate ) {
                case 48000: {
                    ret = USB_Audio_TX_SendData(port, nCount, (uint8_t *)test_48k_24bits_sin_tone_1Khz);
                    break;
                }
                case 96000: {
                    ret = USB_Audio_TX_SendData(port, nCount, (uint8_t *)test_96k_24bits_sin_tone_1Khz);
                    break;
                }
                case 192000: {
                    ret = USB_Audio_TX_SendData(port, nCount, (uint8_t *)test_192k_24bits_sin_tone_1Khz);
                    break;
                }
            }
        }
    } else if (g_usb_audio_mic_channel==2) {
        if (g_usb_audio_mic_sample_size==3) {
            uint8_t* pData = TestTone_Covert();
            if( pData ) {
                ret = USB_Audio_TX_SendData(port, nCount, pData);
            }
        }
    }


    if (ret) {
        static uint32_t s_loop = 0;
        ++s_loop;
        if(s_loop==1000){
            s_loop = 0;
            LOG_MSGID_I(USBAUDIO_DRV, "_USB_Audio_IsoIn_Hdr_TestTone %d, %d bits, %d ch (expect:%d) ", 4, g_usb_audio_mic_sample_rate, g_usb_audio_mic_sample_size<<3, g_usb_audio_mic_channel, nCount);
        }
    } else {
        LOG_MSGID_I(USBAUDIO_DRV, "_USB_Audio_IsoIn_Hdr_TestTone Not Support! %d, %d bits, %d ch", 3, g_usb_audio_mic_sample_rate, g_usb_audio_mic_sample_size<<3, g_usb_audio_mic_channel);
    }

    return 0;
}

#endif

/* EP Iso In interrupt handler, called by EP interrupt */
void USB_Audio_IsoIn_Hdr(void)
{
#if defined(USB_DBG) && defined(USB_AUDIO_TX_TIME_MEASURE)
    uint32_t time_gap = 0;
    uint32_t time_gap1;
    if (usb_dbg_is_opt_enable(USB_DBG_OPT_AUDIO_TX_TIME_MEASURE)) {
        SLA_CustomLogging("TX1", SA_START);
        SLA_CustomLogging("TX1", SA_LABEL);

        /* Timer1 - Check the ISO in is expected 1ms/transaction */
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &usb_tx1_time1_start);

        time_gap = usb_tx1_time1_start - usb_tx1_time1_end;

        if ((time_gap > 1500) && (time_gap < 5000)) {
            LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_IsoIn_Hdr is not 1ms time gap:%dus", 2, time_gap);
            if (usb_dbg_is_opt_enable(USB_DBG_OPT_AUDIO_TX_TIME_ASSERT)) {
                assert(0);
            }
        } else {
            USB_DBG_I( USB_DBG_AUDIO_ISO_IN, "USB_Audio_IsoIn_Hdr time measurement gap:%dus", 2, time_gap);
        }

        usb_tx1_time1_end = usb_tx1_time1_start;

        /* Timer2 - Check time consuming of each operation */
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &usb_tx1_time2_start);
    }
#endif

    uint32_t port = 0;
    uint32_t nCount;

    nCount = USB_Audio_Get_TX_Bytes(port);

    /* Check USB TX IRQ per 1 sec */
    usb_tx1_cnt ++;
    if(usb_tx1_cnt%1000 == 0){
        if (USB_Audio[port].tx_cb){
            LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_IsoIn_Hdr IRQ check - Enter TX Callback nCount:%d", 1, nCount);
        } else {
            LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_IsoIn_Hdr IRQ check - TX Callback is NULL", 1, nCount);
        }
    }

#ifndef USB_AUDIO_TEST_TONE
    if (USB_Audio[port].tx_cb) {
        USB_DBG_I( USB_DBG_AUDIO_ISO_IN, "USB_Audio_IsoIn_Hdr TX callback", 0);
        USB_Audio[port].tx_cb();
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_IsoIn_Hdr TX callback is NULL", 0);
        /**
         * NOTE: Force convert mic_mute_data from (const uint8_t *) to (uint8_t *).
         *       Avoid discarded-qualifiers build warning.
         */
        USB_Audio_TX_SendData(port, nCount, (uint8_t *)mic_mute_data);
    }
#else
    _USB_Audio_IsoIn_Hdr_TestTone(port, nCount);
#endif

#if defined(USB_DBG) && defined(USB_AUDIO_TX_TIME_MEASURE)
    if (usb_dbg_is_opt_enable(USB_DBG_OPT_AUDIO_TX_TIME_MEASURE)) {
        /* Timer2 - Send data */
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &usb_tx1_time2_end);
        time_gap1 = usb_tx1_time2_end - usb_tx1_time2_start;

        if(time_gap1 > 500){
            LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_IsoIn_Hdr send data: %dus > 500us", 1, time_gap1);
        } else {
            USB_DBG_I( USB_DBG_AUDIO_ISO_IN, "USB_Audio_IsoIn_Hdr send data: %dus", 1, time_gap1);
        }
        SLA_CustomLogging("TX1", SA_STOP);
    }
#endif
}

/* EP Iso Out reset handler */
void USB_Audio_IsoIn_Reset(void)
{
    g_UsbAudio[0].txpipe = &g_UsbAudio[0].stream_ep_in_info->ep_status.epout_status;
}
#endif

void USB_Audio_Register_Rx_Callback(uint32_t port, AUDIO_RX_FUNC rx_cb)
{
    if (rx_cb != NULL) {
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Register_Rx_Callback port %d callback:0x%X ", 2, port, rx_cb);
    } else {
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Register_Rx_Callback port %d is NULL", 1, port);
    }

    USB_Audio[port].rx_cb = rx_cb;
}

void USB_Audio_Register_Tx_Callback(uint32_t port, AUDIO_TX_FUNC tx_cb)
{
    if (tx_cb != NULL) {
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Register_Tx_Callback port %d callback:0x%X ", 2, port, tx_cb);
    } else {
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Register_Tx_Callback port %d is NULL", 1, port);
    }

    USB_Audio[port].tx_cb = tx_cb;
}

void USB_Audio_Register_SetInterface_Callback(uint32_t port, AUDIO_SETINTERFACE_FUNC setinterface_cb)
{
    if (setinterface_cb != NULL) {
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Register_SetInterface_Callback port:%d callback:0x%X", 2, port, setinterface_cb);
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Register_SetInterface_Callback port:%d is NULL", 1, port);
    }

    USB_Audio[port].setinterface_cb = setinterface_cb;
}

void USB_Audio_Register_SetSamplingRate_Callback(uint32_t port, AUDIO_SETSAMPLINGRATE_FUNC setsamplingrate_cb)
{
    if (setsamplingrate_cb != NULL) {
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Register_SetSamplingRate_Callback port:%d callback:0x%X", 2, port, setsamplingrate_cb);
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Register_SetSamplingRate_Callback port:%d is NULL", 1, port);
    }

    USB_Audio[port].setsamplingrate_cb = setsamplingrate_cb;
}

void USB_Audio_Register_SetSampleSize_Callback(uint32_t port, AUDIO_SETSAMPLESIZE_FUNC setsamplesize_cb)
{
    if (setsamplesize_cb != NULL) {
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Register_SetSampleSize_Callback port:%d callback:0x%X", 2, port, setsamplesize_cb);
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Register_SetSampleSize_Callback port:%d is NULL", 1, port);
    }

    USB_Audio[port].setsamplesize_cb = setsamplesize_cb;
}

void USB_Audio_Register_SetChannel_Callback(uint32_t port, AUDIO_SETCHANNEL_FUNC setchannel_cb)
{
    if (setchannel_cb != NULL) {
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Register_SetChannel_Callback port:%d callback:0x%X", 2, port, setchannel_cb);
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Register_SetChannel_Callback port:%d is NULL", 1, port);
    }

    USB_Audio[port].setchannel_cb = setchannel_cb;
}

void USB_Audio_Register_Unplug_Callback(uint32_t port, AUDIO_UNPLUG_FUNC unplug_cb)
{
    if (unplug_cb != NULL) {
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Register_Unplug_Callback port:%d callback:0x%X", 2, port, unplug_cb);
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Register_Unplug_Callback port:%d is NULL", 1, port);
    }

    USB_Audio[port].unplug_cb = unplug_cb;
}

void USB_Audio_Register_VolumeChange_Callback(uint32_t port, AUDIO_VOLUMECHANGE_FUNC volumechange_cb)
{
    if (volumechange_cb != NULL) {
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Register_VolumeChange_Callback port:%d callback:0x%X", 2, port, volumechange_cb);
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Register_VolumeChange_Callback port:%d is NULL", 1, port);
    }

    USB_Audio[port].volumechange_cb = volumechange_cb;
}

void USB_Audio_Register_Mute_Callback(uint32_t port, AUDIO_MUTE_FUNC mute_cb)
{
    if (mute_cb != NULL) {
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Register_Mute_Callback port:%d callback:0x%X", 2, port, mute_cb);
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Register_Mute_Callback port:%d is NULL", 1, port);
    }

    USB_Audio[port].mute_cb = mute_cb;
}

#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
void USB_Audio_Register_Mic_SetInterface_Callback(uint32_t port, AUDIO_SETINTERFACE_FUNC setinterface_cb)
{
    port = 0;

    if(setinterface_cb != NULL){
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Register_Mic_SetInterface_Callback port:%d callback:0x%X", 2, port, setinterface_cb);
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Register_Mic_SetInterface_Callback port:%d is NULL", 1, port);
    }

    USB_Audio[port].setinterface_cb_mic = setinterface_cb;
}

void USB_Audio_Register_Mic_SetSamplingRate_Callback(uint32_t port, AUDIO_SETSAMPLINGRATE_FUNC setsamplingrate_cb)
{
    port = 0;

    if(setsamplingrate_cb != NULL){
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Register_Mic_SetSamplingRate_Callback port:%d callback:0x%X", 2, port, setsamplingrate_cb);
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Register_Mic_SetSamplingRate_Callback port:%d is NULL", 1, port);
    }

    USB_Audio[port].setsamplingrate_cb_mic = setsamplingrate_cb;
}

void USB_Audio_Register_Mic_SetSampleSize_Callback(uint32_t port, AUDIO_SETSAMPLESIZE_FUNC setsamplesize_cb)
{
    port = 0;

    if(setsamplesize_cb != NULL){
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Register_Mic_SetSampleSize_Callback port:%d callback:0x%X", 2, port, setsamplesize_cb);
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Register_Mic_SetSampleSize_Callback port:%d is NULL", 1, port);
    }

    USB_Audio[port].setsamplesize_cb_mic = setsamplesize_cb;
}

void USB_Audio_Register_Mic_SetChannel_Callback(uint32_t port, AUDIO_SETCHANNEL_FUNC setchannel_cb)
{
    port = 0;

    if(setchannel_cb != NULL){
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Register_Mic_SetChannel_Callback port:%d callback:0x%X", 2, port, setchannel_cb);
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Register_Mic_SetChannel_Callback port:%d is NULL", 1, port);
    }

    USB_Audio[port].setchannel_cb_mic = setchannel_cb;
}

void USB_Audio_Register_Mic_Unplug_Callback(uint32_t port, AUDIO_UNPLUG_FUNC unplug_cb)
{
    port = 0;

    if(unplug_cb != NULL){
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Register_Mic_Unplug_Callback port:%d callback:0x%X", 2, port, unplug_cb);
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Register_Mic_Unplug_Callback port:%d is NULL", 1, port);
    }

    USB_Audio[port].unplug_cb_mic = unplug_cb;
}

void USB_Audio_Register_Mic_VolumeChange_Callback(uint32_t port, AUDIO_VOLUMECHANGE_FUNC volumechange_cb)
{
    port = 0;

    if(volumechange_cb != NULL){
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Register_Mic_VolumeChange_Callback port:%d callback:0x%X", 2, port, volumechange_cb);
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Register_Mic_VolumeChange_Callback port:%d is NULL", 1, port);
    }

    USB_Audio[port].volumechange_cb_mic = volumechange_cb;
}

void USB_Audio_Register_Mic_Mute_Callback(uint32_t port, AUDIO_MUTE_FUNC mute_cb)
{
    port = 0;

    if(mute_cb != NULL){
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Register_Mic_Mute_Callback port:%d callback:0x%X", 2, port, mute_cb);
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Register_Mic_Mute_Callback port:%d is NULL", 1, port);
    }

    USB_Audio[port].mute_cb_mic = mute_cb;
}

#endif


/************************************************************
    USB main.c handle functions
************************************************************/
// Temp patch
extern hal_usb_status_t hal_usb_init_txmap(uint32_t ep_num, uint16_t data_size);

/* USB IRQ: Get the set interface command to execute USB HW */
void USB_Audio_Set_Interface(uint32_t msgs)
{
    bool alt2en, usb_tx_rx; /* TX:1 RX:0 */
    uint8_t if_num, alt_num, hb_packet;
    uint32_t port, audio_num, byte;

    if_num = (uint8_t)(((uint32_t)(msgs)) >> 16);
    alt_num = (uint8_t)(((uint32_t)(msgs)) & 0x0000FFFF);

    /* Set TX/RX and port number */
    if(if_num == g_UsbAudio[USB_AUDIO_1_PORT].stream_interface_id){
        port = USB_AUDIO_1_PORT;
        usb_tx_rx = USB_AUDIO_RX;
    } else if(if_num == g_UsbAudio[USB_AUDIO_2_PORT].stream_interface_id){
        port = USB_AUDIO_2_PORT;
        usb_tx_rx = USB_AUDIO_RX;
    } else if (if_num == g_UsbAudio[USB_AUDIO_1_PORT].stream_microphone_interface_id) {
        port = USB_AUDIO_1_PORT;
        usb_tx_rx = USB_AUDIO_TX;
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Set_Interface if_num:%d is invalid", 1, if_num);
        port = USB_AUDIO_1_PORT;
        usb_tx_rx = USB_AUDIO_RX;
        return;
    }

    audio_num = port + 1; // Port0: Audio1, Port1:Audio2

    //LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Set_Interface if_id:%d alt:%d port:%d usb_tx_rx:%d", 4,
    //            if_num, alt_num, port, usb_tx_rx);

    /* RX, Speaker */
    if(usb_tx_rx == USB_AUDIO_RX){
        /* Convertor from Alt number to enable/disable */
        alt2en = USB_Audio_RX_SetAlt2Enable(port, alt_num);

        if (alt2en) {
            /* High Band-width mode */
            byte = USB_Audio_Get_RX_Alt_Byte(port, alt_num);

            /* If high-band width mode is enanbled, need to re-init rxmap */
            hb_packet = byte & USB_RXMAP_M1_MASK;
            if(hb_packet){
                hal_usb_init_rxmap(g_UsbAudio[port].stream_ep_out_id, byte);
            }

            /* Convert to byte and packet */
            byte = byte &USB_RXMAP_MAX_PAYLOAD_MASK;

            LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Set_Interface audio%d speaker enable alt:%d max_byte:%d packet:%d", 4, audio_num, alt_num, byte, hb_packet);
            hal_usb_enable_rx_endpoint(g_UsbAudio[port].stream_ep_out_id, HAL_USB_EP_TRANSFER_ISO, HAL_USB_EP_USE_NO_DMA, false);
            hal_usb_clear_rx_endpoint_fifo(g_UsbAudio[port].stream_ep_out_id, HAL_USB_EP_TRANSFER_ISO, false);

            /* Reset buffer related variables. */
            g_UsbAudio[port].rx_buffer_write = 0;
        } else {
            LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Set_Interface audio%d speaker disable", 1, audio_num);
            hal_usb_disable_rx_endpoint(g_UsbAudio[port].stream_ep_out_id);

            /* Reset buffer related variables. */
            g_UsbAudio[port].rx_buffer_write = 0;
        }
    }

    /* TX, Microphone */
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
    if(usb_tx_rx == USB_AUDIO_TX){
        /* Convertor from Alt number to enable/disable */
        alt2en = USB_Audio_TX_SetAlt2Enable(port, alt_num);

        if (alt2en){
            /* High Band-width mode */
            byte = USB_Audio_Get_TX_Alt_Byte(port, alt_num);
            hal_usb_init_txmap(g_UsbAudio[port].stream_ep_in_id, byte);

            LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Set_Interface audio%d microphone enable alt:%d", 2, audio_num, alt_num);
            hal_usb_enable_tx_endpoint(g_UsbAudio[port].stream_ep_in_id, HAL_USB_EP_TRANSFER_ISO, HAL_USB_EP_USE_NO_DMA, false);
            hal_usb_clear_tx_endpoint_fifo(g_UsbAudio[port].stream_ep_in_id, HAL_USB_EP_TRANSFER_ISO, false);
            hal_usb_set_endpoint_tx_ready(g_UsbAudio[port].stream_ep_in_id);
        } else {
        LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Set_Interface audio%d microphone disable", 1, audio_num);
            hal_usb_disable_tx_endpoint(g_UsbAudio[port].stream_ep_in_id);
        }
    }
#endif
}

/* USB Task: decide the sample size and channel by set interface */
void USB_Audio_Set_Interface_CB(uint32_t msgs)
{
    bool alt2en, usb_tx_rx; /* TX:1 RX:0 */
    uint8_t if_num, alt_num;
    uint8_t sample_size = 0, channel = 0;
    uint32_t port, audio_num;

    if_num = (uint8_t)(((uint32_t)(msgs)) >> 16);
    alt_num = (uint8_t)(((uint32_t)(msgs)) & 0x0000FFFF);

    /* Set TX/RX and port number */
    if(if_num == g_UsbAudio[0].stream_interface_id){
        port = USB_AUDIO_1_PORT;
        usb_tx_rx = USB_AUDIO_RX;
    } else if(if_num == g_UsbAudio[1].stream_interface_id){
        port = USB_AUDIO_2_PORT;
        usb_tx_rx = USB_AUDIO_RX;
    } else if (if_num == g_UsbAudio[0].stream_microphone_interface_id) {
        port = USB_AUDIO_1_PORT;
        usb_tx_rx = USB_AUDIO_TX;
    } else {
        LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Set_Interface_CB if_num:%d is invalid", 1, if_num);
        port = USB_AUDIO_1_PORT;
        usb_tx_rx = USB_AUDIO_RX;
        return;
    }

    audio_num = port + 1; // Port0: Audio1, Port1:Audio2

    //LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Set_Interface_CB if_id:%d alt:%d port:%d usb_tx_rx:%d", 4,
    //            if_num, alt_num, port, usb_tx_rx);

    /* RX, Speaker */
    if(usb_tx_rx == USB_AUDIO_RX){
        /* Convertor from Alt number to enable/disable */
        alt2en = USB_Audio_RX_SetAlt2Enable(port, alt_num);

        /* According to ALT num, set up sample size and channel before enabling speaker */
        if(alt2en == true){
        USB_Audio_Set_RX_Alt(port, alt_num);

            sample_size = USB_Audio_Get_RX_Sample_Size(port);
            channel     = USB_Audio_Get_RX_Channel(port);

            /* Call callback function of sample size and channel */
        if (USB_Audio[port].setsamplesize_cb) {
                USB_Audio[port].setsamplesize_cb(g_UsbAudio[port].stream_ep_out_id, sample_size);
        } else {
            LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Set_Interface_CB audio%d setsamplesize_cb is NULL", 1, audio_num);
        }

        if (USB_Audio[port].setchannel_cb) {
                USB_Audio[port].setchannel_cb(g_UsbAudio[port].stream_ep_out_id, channel);
        } else {
            LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Set_Interface_CB audio%d setchannel_cb is NULL", 1, audio_num);
        }

            LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Set_Interface_CB audio%d speaker enable sample_size:%d channel:%d ", 3,
                                   audio_num, sample_size, channel);
        } else {
            LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Set_Interface_CB audio%d speaker disable", 1, audio_num);
        }

        /* Call callback function of interface */
            if (USB_Audio[port].setinterface_cb) {
                USB_Audio[port].setinterface_cb(if_num, alt2en);
            } else {
                LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Set_Interface_CB audio%d setinterface_cb is NULL", 1, audio_num);
            }
        }

#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
    /* TX, Microphone */
    if(usb_tx_rx == USB_AUDIO_TX){

        /* Convertor from Alt number to enable/disable */
        alt2en = USB_Audio_TX_SetAlt2Enable(port, alt_num);

        if(alt2en == true){
        USB_Audio_Set_TX_Alt(port, alt_num);

            sample_size = USB_Audio_Get_TX_Sample_Size(port);
            channel     = USB_Audio_Get_TX_Channel(port);

        if (USB_Audio[port].setsamplesize_cb_mic) {
            #ifndef USB_TEST_MIC
                USB_Audio[port].setsamplesize_cb_mic(USB_EP_DIR_IN | g_UsbAudio[port].stream_ep_in_id, sample_size);
            #endif
        } else {
            LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Set_Interface_CB audio%d setsamplesize_cb_mic is NULL", 1, audio_num);
        }

        if (USB_Audio[port].setchannel_cb_mic) {
            #ifndef USB_TEST_MIC
                 USB_Audio[port].setchannel_cb_mic(USB_EP_DIR_IN | g_UsbAudio[port].stream_ep_in_id, channel);
            #endif
        } else {
            LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Set_Interface_CB audio%d setchannel_cb_mic is NULL", 1, audio_num);
        }

            LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Set_Interface_CB audio%d microphone enable sample_size:%d channel:%d ", 3,
                        audio_num, sample_size, channel);
        } else {
            LOG_MSGID_I(USBAUDIO_DRV, "USB_Audio_Set_Interface_CB audio%d microphone disable sample_size:%d channel:%d ", 1, audio_num);
        }

        /* Call callback function of interface */
            if (USB_Audio[port].setinterface_cb_mic) {
                #ifndef USB_TEST_MIC
                USB_Audio[port].setinterface_cb_mic(if_num, alt2en);
                #endif
            } else {
                LOG_MSGID_E(USBAUDIO_DRV, "USB_Audio_Set_Interface_CB audio%d setinterface_cb_mic is NULL", 1, audio_num);
            }
        }
#endif
}


static int16_t usb_audio_dscr_interface_serialize(void *dscr, void *out, uint16_t ava_size)
{
    uint16_t size = sizeof(usb_audio_dscr_interface_t);

    if (ava_size < size) {
        assert(0);
        return -1;
    }

    for(int i = 0; i < size; i++) {
        ((uint8_t *)out)[i] = ((uint8_t *)dscr)[i];
    }

    return size;
}

static int16_t usb_audio_dscr_control_header_serialize(void *dscr, void *out, uint16_t ava_size)
{
    uint16_t size = sizeof(usb_audio_dscr_control_header_t);
    size = size - USB_AUDIO_DSCR_MAX_INTERFACE_NR + ((usb_audio_dscr_control_header_t*)dscr)->bInCollection;

    if (ava_size < size) {
        assert(0);
        return -1;
    }

    for(int i = 0; i < size; i++) {
        ((uint8_t *)out)[i] = ((uint8_t *)dscr)[i];
    }

    return size;
}

static int16_t usb_audio_dscr_it_serialize(void *dscr, void *out, uint16_t ava_size)
{
    uint16_t size = sizeof(usb_audio_dscr_it_t);

    if (ava_size < size) {
        assert(0);
        return -1;
    }

    for(int i = 0; i < size; i++) {
        ((uint8_t *)out)[i] = ((uint8_t *)dscr)[i];
    }

    return size;
}

static int16_t usb_audio_dscr_ot_serialize(void *dscr, void *out, uint16_t ava_size)
{
    uint16_t size = sizeof(usb_audio_dscr_ot_t);

    if (ava_size < size) {
        assert(0);
        return -1;
    }

    for(int i = 0; i < size; i++) {
        ((uint8_t *)out)[i] = ((uint8_t *)dscr)[i];
    }

    return size;
}

static int16_t usb_audio_dscr_mixer_serialize(void *dscr, void *out, uint16_t ava_size)
{
    uint16_t size = sizeof(usb_audio_dscr_mixer_t);
    size = size - USB_DSCR_MIXER_MAX_IN_PINS + ((usb_audio_dscr_mixer_t*)dscr)->bNrInPins;

    if (ava_size < size) {
        assert(0);
        return -1;
    }

    for(int i = 0; i < size; i++) {
        ((uint8_t *)out)[i] = ((uint8_t *)dscr)[i];
    }

    return size;
}

/* Unused
static int16_t usb_audio_dscr_selector_serialize(void *dscr, void *out, uint16_t ava_size)
{
    uint16_t size = sizeof(usb_audio_dscr_selector_t);
    size = size - USB_DSCR_SELECTOR_MAX_IN_PINS + ((usb_audio_dscr_selector_t*)dscr)->bNrInPins;

    if (ava_size < size) {
        assert(0);
        return -1;
    }

    for(int i = 0; i < size - 1 ; i++) {
        ((uint8_t *)out)[i] = ((uint8_t *)dscr)[i];
    }
    ((uint8_t *)out)[size - 1] = ((usb_audio_dscr_selector_t*)dscr)->iSelector;

    return size;
}*/

static int16_t usb_audio_dscr_feature_serialize(void *dscr, void *out, uint16_t ava_size)
{
    uint16_t size = sizeof(usb_audio_dscr_feature_t);
    size = size - USB_DSCR_FEATURE_MAX_CONTROLS - 1 + ((usb_audio_dscr_feature_t*)dscr)->control_nr;

    if (size != ((usb_audio_dscr_feature_t*)dscr)->bLength) {
        assert(0);
        return -1;
    }

    if (ava_size < size) {
        assert(0);
        return -1;
    }

    for(int i = 0; i < size - 1 ; i++) {
        ((uint8_t *)out)[i] = ((uint8_t *)dscr)[i];
    }
    ((uint8_t *)out)[size - 1] = ((usb_audio_dscr_feature_t*)dscr)->iFeature;

    return size;
}

static int16_t usb_audio_dscr_as_general_serialize(void *dscr, void *out, uint16_t ava_size)
{
    uint16_t size = sizeof(usb_audio_dscr_as_general_t);

    if (size != ((usb_audio_dscr_as_general_t*)dscr)->bLength) {
        assert(0);
        return -1;
    }

    if (ava_size < size) {
        assert(0);
        return -1;
    }

    for(int i = 0; i < size ; i++) {
        ((uint8_t *)out)[i] = ((uint8_t *)dscr)[i];
    }

    return size;
}

static int16_t usb_audio_dscr_as_format_type_serialize(void *dscr, void *out, uint16_t ava_size)
{
    uint16_t size = sizeof(usb_audio_dscr_as_format_type_t);
    size -= USB_AUDIO_DSCR_MAX_FREQ_NUM * sizeof(usb_audio_frequency_t);
    size += ((usb_audio_dscr_as_format_type_t*)dscr)->bSamFreqType * sizeof(usb_audio_frequency_t);

    if (size != ((usb_audio_dscr_as_format_type_t*)dscr)->bLength) {
        assert(0);
        return -1;
    }

    if (ava_size < size) {
        assert(0);
        return -1;
    }

    for(int i = 0; i < size ; i++) {
        ((uint8_t *)out)[i] = ((uint8_t *)dscr)[i];
    }

    return size;
}

int16_t dscr_list_serialize(usb_audio_dscr_hdlr_t *dscr_list, uint8_t len, uint8_t *out, uint16_t ava_len)
{
    int16_t tmp;
    int16_t idx = 0;

    for (int i = 0; i < len; i++) {
        tmp = dscr_list[i].serial_func(dscr_list[i].dscr, out + idx, ava_len);
        USB_DBG_I( USB_DBG_AUDIO_MISC, "dscr_list_serialize i[%d] tmp[%d]", 2, i, tmp);

        if (tmp < 0) {
            assert(0);
        }
        idx += tmp;
    }
    return idx;
}

#endif

#endif /* AIR_USB_AUDIO_ENABLE */

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

#ifndef USBAUDIO_DRV_H
#define USBAUDIO_DRV_H

/* C library */
#include <stdbool.h>
#include <stdint.h>

/* USB Middleware includes */
#include "bmd.h"
#include "usb.h"
#include "usbhid_drv.h"

/***********************************************
    USB Audio File Definition
************************************************/
/* Choose CPU or DMA mode for RX data copy */
//#define USB_AUDIO_RX_DMA_MODE

/* Default enum speaker order   : 1.Chat 2.Game
   Definition will change order : 1.Game 2.Chat */
//#define USB_AUDIO_INVERSE_SPEAER_ENUM

/* Used to check the Audio TX/RX interval */
#define USB_AUDIO_TX_TIME_MEASURE
#define USB_AUDIO_RX_TIME_MEASURE

/* Used to dump audio data for TX/RX */
#define USB_AUDIO_DUMP_RX1
#define USB_AUDIO_DUMP_RX2
#define USB_AUDIO_DUMP_TX1

#define USB_AUDIO_UNUSED_ID         0xFF

/***********************************************
    USB Audio Alt Interface Setting
************************************************/
#define USB_AUDIO_RX1_ALT2_ENABLE
#define USB_AUDIO_RX1_ALT3_ENABLE
#define USB_AUDIO_RX2_ALT2_ENABLE

/***********************************************
    USB Audio TX Alt Interface Setting
************************************************/
#define USB_AUDIO_TX1_ALT2_ENABLE

/***********************************************
    USB Audio Alt Interface Setting
************************************************/
#define USB_AUDIO_NULL                  0
#define USB_AUDIO_SAMPLE_RATE_NUM_1     1
#define USB_AUDIO_SAMPLE_RATE_NUM_2     2
#define USB_AUDIO_SAMPLE_RATE_NUM_3     3
#define USB_AUDIO_SAMPLE_RATE_NUM_4     4
#define USB_AUDIO_SAMPLE_RATE_NUM_5     5

#define USB_AUDIO_SAMPLE_RATE_16K       16000
#define USB_AUDIO_SAMPLE_RATE_32K       32000
#define USB_AUDIO_SAMPLE_RATE_48K       48000
#define USB_AUDIO_SAMPLE_RATE_96K       96000
#define USB_AUDIO_SAMPLE_RATE_192K     192000

#define USB_AUDIO_CHANNEL_1             1
#define USB_AUDIO_CHANNEL_2             2
#define USB_AUDIO_CHANNEL_8             8

#define USB_AUDIO_SAMPLE_SIZE_16BIT     2      /* Unit: byte */
#define USB_AUDIO_SAMPLE_SIZE_24BIT     3      /* Unit: byte */

#define USB_AUDIO_MAX_CHANNEL_NUM       8

/***********************************************
    USB Audio Class Specific Command Definition
************************************************/
/* USB 2.0 : Ch5.6.4 Isochronous Transfer Bus Access Constraints, unit:us */
#define USB_HS_ISO_TRANSFER_DRATION  1000
#define USB_HS_ISO_TRANSFER_DRIFT     125
#define USB_HS_ISO_TRANSFER_UPPER    (USB_HS_ISO_TRANSFER_DRATION + USB_HS_ISO_TRANSFER_DRIFT)
#define USB_HS_ISO_TRANSFER_LOWER    (USB_HS_ISO_TRANSFER_DRATION - USB_HS_ISO_TRANSFER_DRIFT)

/* Version UAC 1.0 class-specific request codes: A.9 */
#define USB_AUDIO_1_REQ_UNDEFINED    0x00
#define USB_AUDIO_1_REQ_SET_CUR      0x01
#define USB_AUDIO_1_REQ_SET_MIN      0x02
#define USB_AUDIO_1_REQ_SET_MAX      0x03
#define USB_AUDIO_1_REQ_SET_RES      0x04
#define USB_AUDIO_1_REQ_SET_MEM      0x05
#define USB_AUDIO_1_REQ_GET_CUR      0x81
#define USB_AUDIO_1_REQ_GET_MIN      0x82
#define USB_AUDIO_1_REQ_GET_MAX      0x83
#define USB_AUDIO_1_REQ_GET_RES      0x84
#define USB_AUDIO_1_REQ_GET_MEM      0x85
#define USB_AUDIO_1_REQ_GET_STAT     0xFF

#define USB_AUDIO_AC_SUBTYPE_UNDEFINED          0x00
#define USB_AUDIO_AC_SUBTYPE_HEADER             0x01
#define USB_AUDIO_AC_SUBTYPE_INPUT_TERMINAL     0x02
#define USB_AUDIO_AC_SUBTYPE_OUTPUT_TERMINAL    0x03
#define USB_AUDIO_AC_SUBTYPE_MIXER_UNIT         0x04
#define USB_AUDIO_AC_SUBTYPE_SELECTOR_UNIT      0x05
#define USB_AUDIO_AC_SUBTYPE_FEATURE_UNIT       0x06
#define USB_AUDIO_AC_SUBTYPE_PROCESSING_UNIT    0x07
#define USB_AUDIO_AC_SUBTYPE_EXTENSION_UNIT     0x08

#define USB_AUDIO_AS_SUBTYPE_UNDEFINED          0x00
#define USB_AUDIO_AS_SUBTYPE_GENERAL            0x01
#define USB_AUDIO_AS_SUBTYPE_FORMAT_TYPE        0x02
#define USB_AUDIO_AS_SUBTYPE_FORMAT_SPECIFIC    0x03

/* UAC 1.0 A.2 Audio Interface Subclass Codes */
#define USB_AUDIO_SUBCLASS_UNDEFINED            0x00
#define USB_AUDIO_SUBCLASS_AUDIOCONTROL         0x01
#define USB_AUDIO_SUBCLASS_AUDIOSTREAMING       0x02
#define USB_AUDIO_SUBCLASS_MIDISTREAMING        0x03

/* UAC 1.0 A.2 Audio Interface Subclass Codes */
#define USB_AUDIO_PROTOCOL_UNDEFINED            0x00

/* UAC 1.0 wChannelConfig bit allocations */
#define USB_AUDIO_CHANNEL_L                     (1 << 0)
#define USB_AUDIO_CHANNEL_R                     (1 << 1)
#define USB_AUDIO_CHANNEL_C                     (1 << 2)
#define USB_AUDIO_CHANNEL_LFE                   (1 << 3)
#define USB_AUDIO_CHANNEL_LS                    (1 << 4)
#define USB_AUDIO_CHANNEL_RS                    (1 << 5)
#define USB_AUDIO_CHANNEL_LC                    (1 << 6)
#define USB_AUDIO_CHANNEL_RC                    (1 << 7)
#define USB_AUDIO_CHANNEL_S                     (1 << 8)
#define USB_AUDIO_CHANNEL_SL                    (1 << 9)
#define USB_AUDIO_CHANNEL_SR                    (1 << 10)
#define USB_AUDIO_CHANNEL_T                     (1 << 11)

#define USB_AUDIO_CHANNEL_CONBINE_MONO          0x00
#define USB_AUDIO_CHANNEL_CONBINE_2CH           (USB_AUDIO_CHANNEL_L | USB_AUDIO_CHANNEL_R)
#define USB_AUDIO_CHANNEL_CONBINE_8CH           \
    (                                           \
        USB_AUDIO_CHANNEL_L   |                 \
        USB_AUDIO_CHANNEL_R   |                 \
        USB_AUDIO_CHANNEL_C   |                 \
        USB_AUDIO_CHANNEL_LFE |                 \
        USB_AUDIO_CHANNEL_LS  |                 \
        USB_AUDIO_CHANNEL_RS  |                 \
        USB_AUDIO_CHANNEL_LC  |                 \
        USB_AUDIO_CHANNEL_RC                    \
    )

/* UAC 1.0 bmaControls bit allocations */
#define USB_AUDIO_BMACONTROL_NULL               (0)
#define USB_AUDIO_BMACONTROL_MUTE               (1 << 0)
#define USB_AUDIO_BMACONTROL_VOLUME             (1 << 1)
#define USB_AUDIO_BMACONTROL_BASS               (1 << 2)
#define USB_AUDIO_BMACONTROL_MID                (1 << 3)
#define USB_AUDIO_BMACONTROL_TREBLE             (1 << 4)
#define USB_AUDIO_BMACONTROL_GRAPHIC_EQ         (1 << 5)
#define USB_AUDIO_BMACONTROL_AUTO_GAIN          (1 << 6)
#define USB_AUDIO_BMACONTROL_DELAY              (1 << 7)
#define USB_AUDIO_BMACONTROL_BASS_BOOST         (1 << 8)
#define USB_AUDIO_BMACONTROL_LOUDNESS           (1 << 9)

typedef enum {
    USB_AUDIO_TERMT_NULL = 0x0000,
    /* USB Terminal Types */
    USB_AUDIO_TERMT_UNDEFINED = 0x0100,
    USB_AUDIO_TERMT_STREAMING = 0x0101,
    USB_AUDIO_TERMT_VENDER = 0x01FF,
    /* Input Terminal Types */
    USB_AUDIO_TERMT_MICROPHONE = 0x0201,
    /* Output Terminal Types */
    USB_AUDIO_TERMT_SPEAKER = 0x0301,
    USB_AUDIO_TERMT_HEADPHONE = 0x0302,
    /* Bi-directional Terminal Types */
    USB_AUDIO_TERMT_HANDSET = 0x0401,
    USB_AUDIO_TERMT_HEADSET = 0x0402,
} usb_audio_termt_t;

typedef struct __attribute__((__packed__)) {
    uint8_t data[3];
} usb_audio_frequency_t;

/* Interface Descriptor */
typedef struct __attribute__((packed)) {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
} usb_audio_dscr_interface_t;

/* Class-Specific AC Interface Header Descriptor */
#define USB_AUDIO_DSCR_MAX_INTERFACE_NR 5
typedef struct __attribute__((packed)) {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint16_t bcdADC;
    uint16_t wTotalLength;
    uint8_t bInCollection;
    uint8_t baInterfaceNr[USB_AUDIO_DSCR_MAX_INTERFACE_NR];
} usb_audio_dscr_control_header_t;

/* Input Terminal Descriptor */
typedef struct __attribute__((packed)) {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bTerminalID;
    uint16_t wTerminalType;
    uint8_t bAssocTerminal;
    uint8_t bNrChannels;
    uint16_t wChannelConfig;
    uint8_t iChannelNames;
    uint8_t iTerminal;
} usb_audio_dscr_it_t;

/* Input Terminal Descriptor */
typedef struct __attribute__((packed)) {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bTerminalID;
    uint16_t wTerminalType;
    uint8_t bAssocTerminal;
    uint8_t bSourceID;
    uint8_t iTerminal;
} usb_audio_dscr_ot_t;

/* Mixer Unit Descriptor */
#define USB_DSCR_MIXER_MAX_IN_PINS 5
typedef struct __attribute__((packed)) {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bUnitID;
    uint8_t bNrInPins;
    uint8_t baSourceID[USB_DSCR_MIXER_MAX_IN_PINS];
} usb_audio_dscr_mixer_t;


/* Selector Unit Descriptor (for 2 unit select) */
#define USB_DSCR_SELECTOR_MAX_IN_PINS 5
typedef struct __attribute__((packed)) {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bUnitID;
    uint8_t bNrInPins;
    uint8_t baSourceID[USB_DSCR_SELECTOR_MAX_IN_PINS];
    uint8_t iSelector;
} usb_audio_dscr_selector_t;

/* Feature Unit Descriptor */
#define USB_DSCR_FEATURE_MAX_CONTROLS (1 + USB_AUDIO_MAX_CHANNEL_NUM)
typedef struct __attribute__((packed)) {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bUnitID;
    uint8_t bSourceID;
    uint8_t bControlSize;
    uint8_t bmaControls[USB_DSCR_FEATURE_MAX_CONTROLS];
    uint8_t iFeature;
    uint8_t control_nr;   /* custom field, record num of bmaControls */
} usb_audio_dscr_feature_t;

/* Class-Specific AS Interface Descriptor */
typedef struct __attribute__((__packed__)) {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bTerminalLink;
    uint8_t bDelay;
    uint16_t wFormatTag;
} usb_audio_dscr_as_general_t;

/* Class-Specific AS Format Type Descriptor */
#define USB_AUDIO_DSCR_MAX_FREQ_NUM 5
typedef struct __attribute__((__packed__)) {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bFormatType;
    uint8_t bNrChannels;
    uint8_t bSubFrameSize;
    uint8_t bBitResolution;
    uint8_t bSamFreqType;
    usb_audio_frequency_t tSamFreq[USB_AUDIO_DSCR_MAX_FREQ_NUM];
} usb_audio_dscr_as_format_type_t;

/**
 * USB audio descriptor serialize functions
 */
typedef int16_t (*usb_audio_serial_func_t)(void *, void *, uint16_t);

typedef struct {
    void *dscr;
    usb_audio_serial_func_t serial_func;
} usb_audio_dscr_hdlr_t;

/* usb audio customer definition */
#define USB_AUDIO_RX_BUFFER_SIZE     2304

#define USB_AUDIO_INTERFACE_PROTOCOL 0x00

/* usb audio ep maxium packet size */
#define USB_AUDIO_RX_EP_MAX_SIZE      192

/* 48(K Hz, sample rate) x 2(channel) x 3 byte (sample size) */
#define USB_AUDIO_TX_EP_MAX_SIZE      288


typedef enum {
    USB_AUDIO_VERSION_None = 0,
    USB_AUDIO_VERSION_V1 = 1,
    USB_AUDIO_VERSION_V2 = 2,
    USB_AUDIO_VERSION_V3 = 3
} usb_audio_version_t;


typedef enum {
    USB_AUDIO_MUTE_OFF = 0,
    USB_AUDIO_MUTE_ON  = 1
} usb_audio_mute_t;

/**
 * @brief USB Audio Volume Contol Method
 *   CreateIf function will change attr in feature & input terminal by this enum
 *     - bNrChannels
 *     - wChannelConfig
 *     - bmaControls
 */
typedef enum {
    USB_AUDIO_VC_INDIVIUAL   = 0, /**< Enable volume control for each channel */
    USB_AUDIO_VC_MASTER      = 1, /**< Enable volume control only for master channel */
    USB_AUDIO_VC_BOTH        = 2, /**< Enable volume control for master and each channel */
} usb_audio_vc_t;

typedef void (*AUDIO_RX_FUNC)(void);
typedef void (*AUDIO_TX_FUNC)(void);
typedef void (*AUDIO_SETINTERFACE_FUNC)(uint8_t bInterfaceNumber, uint8_t bAlternateSetting);
typedef void (*AUDIO_SETSAMPLINGRATE_FUNC)(uint8_t ep_number, uint32_t sampling_rate);
typedef void (*AUDIO_SETSAMPLESIZE_FUNC)(uint8_t ep_number, uint8_t sampling_size);
typedef void (*AUDIO_SETCHANNEL_FUNC)(uint8_t ep_number, uint8_t channel);
typedef void (*AUDIO_UNPLUG_FUNC)(void);
typedef void (*AUDIO_VOLUMECHANGE_FUNC)(uint8_t ep_number, uint8_t channel, uint32_t volume, int32_t dB);
typedef void (*AUDIO_MUTE_FUNC)(uint8_t ep_number, usb_audio_mute_t mute);

/* Audio device structure */
typedef struct {
    /**
     * Audio card config
     * Would effect descriptor
     */
    bool                spk_feature_en; /* = spk_alt1_en */
    bool                spk_alt2_en;
    bool                spk_alt3_en;
    bool                mic_feature_en; /* = mic_alt1_en */
    bool                mic_alt2_en;
    bool                mic_alt3_en;
    /* Speaker device settings */
    uint16_t            spk_terminal_type;
    uint16_t            spk_cur;
    uint16_t            spk_min;
    uint16_t            spk_max;
    uint16_t            spk_res;
    uint8_t             spk_chs;
    uint16_t            spk_chs_map;
    usb_audio_vc_t      spk_vc;
    /* Microphone device settings */
    uint16_t            mic_terminal_type;
    uint16_t            mic_cur;
    uint16_t            mic_min;
    uint16_t            mic_max;
    uint16_t            mic_res;
    uint8_t             mic_chs;
    uint16_t            mic_chs_map;
    usb_audio_vc_t      mic_vc;

    /**
     * Audio card static variable
     * Would exchange value with host
     */
    /**
     * channel_vol size is 1(master) + USB_AUDIO_MAX_CHANNEL_NUM
     */
    uint16_t            spk_cur_channel_vol[1 + USB_AUDIO_MAX_CHANNEL_NUM];
    uint16_t            mic_cur_channel_vol[1 + USB_AUDIO_MAX_CHANNEL_NUM];

    /**
     * Audio card internal resource
     * USB SW will use these varaibles to access resources
     */
    uint8_t             control_interface_id;
    uint8_t             stream_interface_id;
    uint8_t             stream_ep_out_id;
    Usb_EpBOut_Status   *rxpipe; /* bulk out EP, Data interface */
    Usb_Ep_Info         *stream_ep_out_info;
    Usb_Interface_Info  *control_if_info;
    Usb_Interface_Info  *stream_if_info;
    Usb_IAD_Dscr        *iad_desc;
    /* buffer for read DMA operation*/
    uint8_t             *rx_buffer;
    uint32_t            rx_buffer_len;
    volatile uint32_t   rx_buffer_write;
    volatile uint32_t   msg_notify;
    uint8_t             stream_microphone_interface_id;
    uint8_t             stream_ep_in_id;
    Usb_EpBOut_Status   *txpipe; /* bulk in EP, Data interface */
    Usb_Ep_Info         *stream_ep_in_info;
    Usb_Interface_Info  *stream_microphone_if_info;
} UsbAudio_Struct;

typedef struct {
    usb_audio_version_t         audio_version;
    bool                        initialized;
    AUDIO_RX_FUNC               rx_cb;
    AUDIO_TX_FUNC               tx_cb;
    AUDIO_SETINTERFACE_FUNC     setinterface_cb;
    AUDIO_SETSAMPLINGRATE_FUNC  setsamplingrate_cb;
    AUDIO_SETSAMPLESIZE_FUNC    setsamplesize_cb;
    AUDIO_SETCHANNEL_FUNC       setchannel_cb;
    AUDIO_UNPLUG_FUNC           unplug_cb;
    AUDIO_VOLUMECHANGE_FUNC     volumechange_cb;
    AUDIO_MUTE_FUNC             mute_cb;
    AUDIO_SETINTERFACE_FUNC     setinterface_cb_mic;
    AUDIO_SETSAMPLINGRATE_FUNC  setsamplingrate_cb_mic;
    AUDIO_SETSAMPLESIZE_FUNC    setsamplesize_cb_mic;
    AUDIO_SETCHANNEL_FUNC       setchannel_cb_mic;
    AUDIO_UNPLUG_FUNC           unplug_cb_mic;
    AUDIO_VOLUMECHANGE_FUNC     volumechange_cb_mic;
    AUDIO_MUTE_FUNC             mute_cb_mic;
} UsbAudioStruct;

#define USB_AUDIO_1_PORT 0
#define USB_AUDIO_2_PORT 1
#define USB_AUDIO_RX     0
#define USB_AUDIO_TX     1


/***********************************************
    function and global variable
************************************************/
extern UsbAudioStruct USB_Audio[2];
extern UsbAudio_Struct g_UsbAudio[2];

extern void USB_Audio_Setting_By_NVKey();

void USB_Aduio_Set_RX1_Alt1(uint8_t smaple_rate_num, uint8_t smaple_size, uint8_t channel, uint32_t *sample_rate);
void USB_Aduio_Set_RX1_Alt2(bool alt2_en, uint8_t smaple_rate_num, uint8_t smaple_size, uint8_t channel, uint32_t *sample_rate);
void USB_Aduio_Set_RX1_Alt3(bool alt3_en, uint8_t smaple_rate_num, uint8_t smaple_size, uint8_t channel, uint32_t *sample_rate);
void USB_Aduio_Set_RX2_Alt1(uint8_t smaple_rate_num, uint8_t smaple_size, uint8_t channel, uint32_t *sample_rate);
void USB_Aduio_Set_RX2_Alt2(bool alt2_en, uint8_t smaple_rate_num, uint8_t smaple_size, uint8_t channel, uint32_t *sample_rate);
void USB_Aduio_Set_TX1_Alt1(uint8_t smaple_rate_num, uint8_t smaple_size, uint8_t channel, uint32_t *sample_rate);
void USB_Aduio_Set_TX1_Alt2(bool alt2_en, uint8_t smaple_rate_num, uint8_t smaple_size, uint8_t channel, uint32_t *sample_rate);

void USB_Audio1_ControlIf_Create(void *ifname);
void USB_Audio2_ControlIf_Create(void *ifname);
void USB_Audio1_ControlIf_Reset(void);
void USB_Audio2_ControlIf_Reset(void);

void USB_Audio1_ControlIf_Enable(void);
void USB_Audio2_ControlIf_Enable(void);

void USB_Audio1_ControlIf_Speed_Reset(bool b_other_speed);
void USB_Audio2_ControlIf_Speed_Reset(bool b_other_speed);
void USB_Audio1_StreamIf_Create(void *ifname);
void USB_Audio2_StreamIf_Create(void *ifname);

void USB_Audio1_StreamIf_Reset(void);
void USB_Audio2_StreamIf_Reset(void);

void USB_Audio1_StreamIf_Enable(void);
void USB_Audio2_StreamIf_Enable(void);
void USB_Audio1_StreamIf_Speed_Reset(bool b_other_speed);
void USB_Audio2_StreamIf_Speed_Reset(bool b_other_speed);

#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
void USB_Audio_StreamIf_Microphone_Create(void *ifname);
void USB_Audio_StreamIf_Microphone_Reset(void);
void USB_Audio_StreamIf_Microphone_Enable(void);
void USB_Audio_StreamIf_Microphone_Speed_Reset(bool b_other_speed);
#endif
void USB_Audio1_Ep0_Command(Usb_Ep0_Status *pep0state, Usb_Command *pcmd);
void USB_Audio2_Ep0_Command(Usb_Ep0_Status *pep0state, Usb_Command *pcmd);

void USB_Audio1_IsoOut_Hdr(void);
void USB_Audio2_IsoOut_Hdr(void);

void USB_Audio1_IsoOut_Reset(void);
void USB_Audio2_IsoOut_Reset(void);
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
void USB_Audio_IsoIn_Hdr(void);
void USB_Audio_IsoIn_Reset(void);
#endif
void USB_Init_Audio_Status(void);
void USB_Release_Audio_Status(void);
bool USB_Audio_RX_SetAlt2Enable(uint32_t port, uint8_t alt_num);
uint32_t USB_Audio_Set_RX_Alt(uint32_t port, uint8_t alt_num);
uint32_t USB_Audio_Get_RX_Alt_Byte(uint32_t port, uint8_t alt_num);
uint8_t USB_Audio_Get_RX_Channel(uint32_t port);
uint8_t USB_Audio_Get_RX_Sample_Size(uint32_t port);
uint32_t USB_Audio_Get_RX_Sample_Rate(uint32_t port);
void USB_Audio_Get_Chat_Info(uint8_t *get_audio1_rx_if_id, uint8_t *get_audio1_rx_ep_id, uint8_t *get_audio1_tx_if_id, uint8_t *get_audio1_tx_ep_id);
void USB_Audio_Get_Game_Info(uint8_t *get_audio2_rx_if_id, uint8_t *get_audio2_rx_ep_id);
void USB_Audio_Register_Rx_Callback(uint32_t port, AUDIO_RX_FUNC rx_cb);
void USB_Audio_Register_Tx_Callback(uint32_t port, AUDIO_TX_FUNC tx_cb);
void USB_Audio_Register_SetInterface_Callback(uint32_t port, AUDIO_SETINTERFACE_FUNC setinterface_cb);
void USB_Audio_Register_SetSamplingRate_Callback(uint32_t port, AUDIO_SETSAMPLINGRATE_FUNC setsamplingrate_cb);
void USB_Audio_Register_SetSampleSize_Callback(uint32_t port, AUDIO_SETSAMPLESIZE_FUNC setsamplesize_cb);
void USB_Audio_Register_SetChannel_Callback(uint32_t port, AUDIO_SETCHANNEL_FUNC setchannel_cb);
void USB_Audio_Register_Unplug_Callback(uint32_t port, AUDIO_UNPLUG_FUNC unplug_cb);
void USB_Audio_Register_VolumeChange_Callback(uint32_t port, AUDIO_VOLUMECHANGE_FUNC volumechange_cb);
void USB_Audio_Register_Mute_Callback(uint32_t port, AUDIO_MUTE_FUNC mute_cb);
#ifdef AIR_USB_AUDIO_1_MIC_ENABLE
bool USB_Audio_TX_SetAlt2Enable(uint32_t port, uint8_t alt_num);
uint32_t USB_Audio_Set_TX_Alt(uint32_t port, uint8_t alt_num);
uint32_t USB_Audio_Get_TX_Alt_Byte(uint32_t port, uint8_t alt_num);
uint8_t USB_Audio_Get_TX_Channel(uint32_t port);
uint8_t USB_Audio_Get_TX_Sample_Size(uint32_t port);
uint32_t USB_Audio_Get_TX_Sample_Rate(uint32_t port);
uint32_t USB_Audio_Get_TX_Bytes(uint32_t port);
uint32_t USB_Audio_TX_SendData(uint32_t port, uint32_t len, uint8_t *data);
void USB_Audio_Register_Mic_SetInterface_Callback(uint32_t port, AUDIO_SETINTERFACE_FUNC setinterface_cb);
void USB_Audio_Register_Mic_SetSamplingRate_Callback(uint32_t port, AUDIO_SETSAMPLINGRATE_FUNC setsamplingrate_cb);
void USB_Audio_Register_Mic_SetSampleSize_Callback(uint32_t port, AUDIO_SETSAMPLESIZE_FUNC setsamplesize_cb);
void USB_Audio_Register_Mic_SetChannel_Callback(uint32_t port, AUDIO_SETCHANNEL_FUNC setchannel_cb);
void USB_Audio_Register_Mic_Unplug_Callback(uint32_t port, AUDIO_UNPLUG_FUNC unplug_cb);
void USB_Audio_Register_Mic_VolumeChange_Callback(uint32_t port, AUDIO_VOLUMECHANGE_FUNC volumechange_cb);
void USB_Audio_Register_Mic_Mute_Callback(uint32_t port, AUDIO_MUTE_FUNC mute_cb);
#endif
void *USB_Audio_Rx_Buffer_Get_Read_Address(uint32_t port);
void USB_Audio_Rx_Buffer_Drop_Bytes(uint32_t port, uint32_t bytes);
uint32_t USB_Audio_Get_Len_Received_Data(uint32_t port);
uint32_t USB_Audio_Read_Data(uint32_t port, void *dst, uint32_t len);

void usb_audio_set_audio_card(uint32_t port, bool spk, bool mic);
void usb_audio_set_terminal_type(uint32_t port, usb_audio_termt_t spk_terminal, usb_audio_termt_t mic_terminal);
void usb_audio_set_spk_channels(uint32_t port, uint8_t chs, uint16_t chs_map, usb_audio_vc_t vc);
void usb_audio_set_mic_channels(uint32_t port, uint8_t chs, uint16_t chs_map, usb_audio_vc_t vc);

int32_t  USB_Audio_Volume_dB_Convertor(int16_t raw_data);
uint16_t USB_Audio_Volume_0to100_Convertor(uint16_t raw_data, uint16_t volume_min, uint16_t volume_max);

void USB_Audio_Set_Interface(uint32_t msgs);
void USB_Audio_Set_Interface_CB(uint32_t msgs);

uint16_t USB_Audio_Get_Spk1_Cur(uint8_t ch);
uint16_t USB_Audio_Get_Mic1_Cur(uint8_t ch);
uint16_t USB_Audio_Get_Spk2_Cur(uint8_t ch);

void USB_Audio_Set_Spk1_Cur(uint8_t ch, uint16_t volume);
void USB_Audio_Set_Mic1_Cur(uint8_t ch, uint16_t volume);
void USB_Audio_Set_Spk2_Cur(uint8_t ch, uint16_t volume);

uint8_t USB_Audio_Get_Spk1_Mute();
uint8_t USB_Audio_Get_Mic1_Mute();
uint8_t USB_Audio_Get_Spk2_Mute();

void USB_Audio_Set_Spk1_Mute(bool mute);
void USB_Audio_Set_Mic1_Mute(bool mute);
void USB_Audio_Set_Spk2_Mute(bool mute);

#endif /* USBAUDIO_DRV_H */

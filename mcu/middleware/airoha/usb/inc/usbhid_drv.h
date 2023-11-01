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

#ifndef USBHID_DRV_H
#define USBHID_DRV_H

#ifdef AIR_USB_HID_ENABLE

/* USB Middleware includes */
#include "usb.h"


/******************************************************************************
 * USB HID Config
 *****************************************************************************/
// #define USB_HID_SUPPORT_IDLE_RATE
#ifdef AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
#define USB_HID_MAX_DEVICE_NUM 4
#else
#define USB_HID_MAX_DEVICE_NUM 1
#endif

/******************************************************************************
 * USB HID Debug Config
 *****************************************************************************/
#define USB_HID_DEBUG_GPIO_ENABLE

/******************************************************************************
 * USB HID Debug Config
 *****************************************************************************/
#define USB_HID_DEBUG_GPIO_ENABLE

/******************************************************************************
 * USB HID Common Definition
 *****************************************************************************/
/* USB HID Descriptor */
#define USB_HID_EP_MAX_SIZE                        64
#define USB_HID_REPOTR_DSCR_MAX_SIZE              512

/* USB HID Report Type*/
#define USB_HID_INPUT_REPORT_TYPE                0x01
#define USB_HID_OUTPUT_REPORT_TYPE               0x02
#define USB_HID_FEATURE_REPORT_TYPE              0x03

/* Align naming to race and mux  */
#define USB_MUX_PORT1                            0x00
#define USB_MUX_PORT2                            0x01
#define USB_MUX_PORT_UNUSE                       0xFF


/******************************************************************************
 * USB Report Definition
 *****************************************************************************/
/* USB HID Keyboard Enable*/
#define USB_HID_KEYBOARD_ENABLE

/* USB HID GYROMETER Sensor Enable*/
//#define USB_HID_CUSTOM_GYROMETER

/* USB HID Call Control Enable*/
#define USB_HID_TELEPHONY_CALL_CONTROL_ENABLE

#define USB_HID_GMOUSE_ENABLE
#define USB_HID_GKEYBOARD_ENABLE

/******************************************************************************
 * USB HID Report - List and Item
 *****************************************************************************/
/* Airoha In/Out Report for Race, FOTA and HID logging*/
#define USB_HID_AIR_OUT_REPORT_ID                0x06
#define USB_HID_AIR_IN_REPORT_ID                 0x07

#define USB_HID_AIR2_OUT_REPORT_ID               0x08
#define USB_HID_AIR2_IN_REPORT_ID                0x09

#define USB_HID_AIR_REPORT_ID_INDEX              0x00
#define USB_HID_AIR_REPORT_LEN_INDEX             0x01
#define USB_HID_AIR_REPORT_TARGET_INDEX          0x02
#define USB_HID_AIR_REPORT_DATA_INDEX            0x03

#ifdef AIR_BTA_IC_PREMIUM_G2
#define USB_HID_REPORT_MAX_LEN                   64  /* Maximum byte of HID Report */
#else
#define USB_HID_REPORT_MAX_LEN                   1024  /* Maximum byte of HID Report */
#endif
#define USB_HID_AIR_REPORT_LEN                   0x3E  /* AIR HID In/Out  Report is 62-byte*/
#define USB_HID_AIR_REPORT_DATA_LEN              0x3B  /* AIR HID ReportAvailable data is = 59(62-3)(0x3B) (-3 = report ID, data length, target device)*/
#define USB_HID_AIR2_REPORT_DATA_LEN           0x03FC
#define USB_HID_AIR_REPORT_ZERO_LEN              0x00

#define TARGET_LOCAL_DEVICE                      0x00
#define TARGET_REMOTE_DEVICE                     0x80
#define TARGET_REMOTE_MULTIDEVICE1               0x81
#define TARGET_REMOTE_MULTIDEVICE2               0x82
#define TARGET_REMOTE_MULTIDEVICE3               0x83
#define TARGET_REMOTE_MULTIDEVICE4               0x84
#define TARGET_EXTERNAL_DEVICE                   0x90
#define TARGET_INVALID_DEVICE                    0xFF

/* HID Report of Audio Control */
#define USB_HID_AUDIO_REPORT_ID                  0x0C

#define USB_HID_NONE                             0x00
#define USB_HID_VOLUME_UP                        0x01
#define USB_HID_VOLUME_DOWN                      0x02
#define USB_HID_VOLUME_MUTE                      0x04
#define USB_HID_PLAY_PAUSE                       0x08
#define USB_HID_SCAN_NEXT_TRACK                  0x10
#define USB_HID_SCAN_PREVIOUS_TRACK              0x20

/* HID Report of Gyroscope Data*/
#define USB_HID_GYROSCOPE_DATA_IN_REPORT_ID      0x03

/* HID Report of TEAMS */
#define USB_HID_TEAMS_FEATURE_REPORT_ID          0x9A
#define USB_HID_TEAMS_IN_REPORT_ID               0x9B

/* HID Report of Telephony Call Control */
#define USB_HID_TEL_CALL_CTL_REPORT_ID           0x05

#define USB_HID_HOOK_SWITCH                      0x01
#define USB_HID_LINE_BUSY_TONE                   0x02
#define USB_HID_LINE                             0x04
#define USB_HID_PHONE_MUTE                       0x08
#define USB_HID_FLASH                            0x10
#define USB_HID_REDIAL                           0x20
#define USB_HID_SPEED_DIAL                       0x40

/* HID Report of CFU */
#define USB_HID_CFU_DUMMY_IN_REPORT_ID           0x20
#define USB_HID_CFU_CONTENT_IN_REPORT_ID         0x22
#define USB_HID_CFU_OFFER_IN_REPORT_ID           0x25
#define USB_HID_CFU_CONTENT_OUT_REPORT_ID        0x20
#define USB_HID_CFU_OFFER_OUT_REPORT_ID          0x25
#define USB_HID_CFU_VERSION_FEATURE_REPORT_ID    0x20

#define USB_HID_CFU_DUMMY_IN_REPORT_LEN          (1+1)  /* CFU report length, +1 means report ID */
#define USB_HID_CFU_CONTENT_IN_REPORT_LEN       (1+16)
#define USB_HID_CFU_OFFER_IN_REPORT_LEN         (1+16)
#define USB_HID_CFU_CONTENT_OUT_REPORT_LEN      (1+60)
#define USB_HID_CFU_OFFER_OUT_REPORT_LEN        (1+16)
#define USB_HID_CFU_VERSION_FEATURE_REPORT_LEN  (1+60)


/******************************************************************************
 * USB HID Debug GPIO Definition
 *****************************************************************************/
#ifdef USB_HID_DEBUG_GPIO_ENABLE
#if defined(AIR_BTA_IC_PREMIUM_G2)
#define USB_DEBUG_GPIO_1 HAL_GPIO_15
#define USB_DEBUG_GPIO_1_GPIO_MODE HAL_GPIO_15_GPIO15
#define USB_DEBUG_GPIO_2 HAL_GPIO_16
#define USB_DEBUG_GPIO_2_GPIO_MODE HAL_GPIO_16_GPIO16
#define USB_DEBUG_GPIO_3 HAL_GPIO_17
#define USB_DEBUG_GPIO_3_GPIO_MODE HAL_GPIO_15_GPIO15
#define USB_DEBUG_GPIO_4 HAL_GPIO_18
#define USB_DEBUG_GPIO_4_GPIO_MODE HAL_GPIO_16_GPIO16

#elif defined(AIR_BTA_IC_STEREO_HIGH_G3)
#define USB_DEBUG_GPIO_1 HAL_GPIO_6
#define USB_DEBUG_GPIO_1_GPIO_MODE HAL_GPIO_6_GPIO6
#define USB_DEBUG_GPIO_2 HAL_GPIO_9
#define USB_DEBUG_GPIO_2_GPIO_MODE HAL_GPIO_9_GPIO9
#define USB_DEBUG_GPIO_3 HAL_GPIO_10
#define USB_DEBUG_GPIO_3_GPIO_MODE HAL_GPIO_10_GPIO10
#define USB_DEBUG_GPIO_4 HAL_GPIO_11
#define USB_DEBUG_GPIO_4_GPIO_MODE HAL_GPIO_11_GPIO11

#elif defined(AIR_BTA_IC_PREMIUM_G3)
#define USB_DEBUG_GPIO_1 HAL_GPIO_1
#define USB_DEBUG_GPIO_1_GPIO_MODE HAL_GPIO_1_GPIO1
#define USB_DEBUG_GPIO_2 HAL_GPIO_2
#define USB_DEBUG_GPIO_2_GPIO_MODE HAL_GPIO_2_GPIO2
#define USB_DEBUG_GPIO_3 HAL_GPIO_3
#define USB_DEBUG_GPIO_3_GPIO_MODE HAL_GPIO_3_GPIO3
#define USB_DEBUG_GPIO_4 HAL_GPIO_4
#define USB_DEBUG_GPIO_4_GPIO_MODE HAL_GPIO_4_GPIO4

#else
#define USB_DEBUG_GPIO_1 HAL_GPIO_1
#define USB_DEBUG_GPIO_1_GPIO_MODE HAL_GPIO_1_GPIO1
#define USB_DEBUG_GPIO_2 HAL_GPIO_2
#define USB_DEBUG_GPIO_2_GPIO_MODE HAL_GPIO_2_GPIO2
#define USB_DEBUG_GPIO_3 HAL_GPIO_3
#define USB_DEBUG_GPIO_3_GPIO_MODE HAL_GPIO_3_GPIO3
#define USB_DEBUG_GPIO_4 HAL_GPIO_4
#define USB_DEBUG_GPIO_4_GPIO_MODE HAL_GPIO_4_GPIO4
#endif
#endif /* USB_HID_DEBUG_GPIO_ENABLE */


/******************************************************************************
 * USB HID Report - API
 *****************************************************************************/
typedef enum {
    USB_HID_STATUS_OK                    = 0,     /* Operation completed successfully*/
    USB_HID_TX_LEN_IS_ZERO               = 1,     /* TX data length is zero */
    USB_HID_TX_LEN_TOO_LARGE             = 2,     /* TX data length is too large */
    USB_HID_CHARGER_DETECT_ERROR         = 3,     /* Charger type is wrong */
    USB_HID_SEND_DATA_ERROR              = 4,     /* HID TX status is error */
    USB_HID_IS_NOT_READY                 = 5,     /* HID is not ready */
    USB_HID_PORT_INVIALD                 = 6,
    USB_HID_TX_IS_BUSY                   = 7,
    USB_HID_TX_REPORT_IS_NOT_ENABLE      = 8,
    USB_HID_PORT_DISABLE                 = 9,
} USB_HID_Status_t;

typedef enum {
    USB_HID_KEYBOARD_KEY_WIN_KEY_ONLY          = 0x00,  /* Press Win Key Only*/
    USB_HID_KEYBOARD_KEY_C                     = 0x06,  /* Press Win Key + C */
    USB_HID_KEYBOARD_KEY_D                     = 0x07,  /* Press Win Key + D */  /* Minimize All Windows */
    USB_HID_KEYBOARD_KEY_E                     = 0x08,  /* Press Win Key + E */  /* Open My Computer */
    USB_HID_KEYBOARD_KEY_L                     = 0x0F,  /* Press Win Key + L */  /* Lock Computer */
} USB_HID_WINDOWS_KEY_MODIFIER_FUNC_t;

/* In SDK3.5, convert the API naming for compatibility for previous version */
#define USB_Audio_HID_TX USB_HID_TX_SendData
#define USB_Audio_HID_VolumeUp USB_HID_VolumeUp
#define USB_Audio_HID_VolumeDown USB_HID_VolumeDown
#define USB_Audio_HID_VolumeMute USB_HID_VolumeMute
#define USB_Audio_HID_PlayPause USB_HID_PlayPause
#define USB_Audio_HID_PlayPause_RejectCall USB_HID_PlayPause2RejectCall
#define USB_Audio_HID_ScanNextTrack USB_HID_ScanNextTrack
#define USB_Audio_HID_ScanPreviousTrack USB_HID_ScanPreviousTrack
#define USB_Audio_HID_Win_Key_Func USB_HID_Win_Key_Func
#define USB_Audio_HID_Gyroscope_Send_Rotate_Data_and_Reset USB_HID_Gyroscope_Send_Rotate_Data_and_Reset

USB_HID_Status_t USB_HID_TX_SendData(uint8_t *data, uint8_t data_len);
USB_HID_Status_t USB_HID_VolumeUp(uint8_t step);
USB_HID_Status_t USB_HID_VolumeDown(uint8_t step);
USB_HID_Status_t USB_HID_VolumeMute();
USB_HID_Status_t USB_HID_PlayPause();
USB_HID_Status_t USB_HID_PlayPause2RejectCall();
USB_HID_Status_t USB_HID_ScanNextTrack();
USB_HID_Status_t USB_HID_ScanPreviousTrack();
USB_HID_Status_t USB_HID_Win_Key_Func(USB_HID_WINDOWS_KEY_MODIFIER_FUNC_t key);
USB_HID_Status_t USB_HID_Gyroscope_Send_Rotate_Data_and_Reset(int16_t rx, int16_t  ry, int16_t  rz, int16_t vx, int16_t  vy, int16_t  vz, uint8_t frame_count);

/******************************************************************************
 * USB HID Transfer Fucntion
 *****************************************************************************/
/* USB MUX Handler */
typedef enum {
    HID_EVENT_READY_TO_READ = 0,        /**< Ready to read event. */
    HID_EVENT_READY_TO_WRITE,           /**< Ready to write event. */
    HID_EVENT_USB_CONNECTION,           /**< USB connection event. */
    HID_EVENT_USB_DISCONNECTION,        /**< USB disconnection event. */
    HID_EVENT_DROP_RX_DATA,             /**< USB drop rx event. */
} hid_callback_event_t;

typedef void (*mux_usb_hid_callback)(uint8_t usb_port, hid_callback_event_t event, void *parameter);
void USB_HID_Ep0_ReadData(void);
void USB_HID_Ep0_DispatchData(void);

/* USB HID Callback Handler */
#define USB_HID_HANDLER_MAX_NUM 8
#define USB_HID_HANDLER_INIT_REPORT_ID 0xFF
typedef void (*usb_hid_handler_callback)(uint8_t *data, uint32_t data_length);
typedef void (*usb_hid_pretx_cb_t)(uint8_t port);

typedef enum {
    USB_HID_HANDLER_STATUS_OK                  = 0,
    USB_HID_HANDLER_STATUS_FAIL                = 1,
    USB_HID_HANDLER_IS_NOT_READY               = 2,
} USB_HID_HANDLER_t;

typedef struct {
    uint8_t report_id;
    uint8_t report_length;
    usb_hid_handler_callback handler_callback;
} usb_hid_handler_t;

USB_HID_HANDLER_t usb_hid_handler_rx_register(uint8_t report_id, uint8_t report_length, usb_hid_handler_callback callback);
USB_HID_HANDLER_t usb_hid_handler_tx_register(uint8_t report_id, uint8_t report_length, usb_hid_handler_callback callback);
USB_HID_HANDLER_t usb_hid_rx_handler_unregister(uint8_t report_id);
USB_HID_HANDLER_t usb_hid_tx_handler_unregister(uint8_t report_id);
USB_HID_HANDLER_t usb_hid_tx_intr_register(uint8_t port, usb_hid_pretx_cb_t cb);
USB_HID_HANDLER_t usb_hid_pretx_intr_register(uint8_t port, usb_hid_pretx_cb_t cb);

/******************************************************************************
 * USB HID 1.11 spec
 *
 * Below is constant or code in USB HID 1.11 spec
 * https://www.usb.org/document-library/device-class-definition-hid-111
 *****************************************************************************/
/**
 * SubClass code of HID device.
 * 4.2 Subclass
 */
typedef enum {
    USB_HID_SUBCLASS_CODE_NONE = 0x00, /* No Subclass */
    USB_HID_SUBCLASS_CODE_BOOT = 0x01, /* Boot Interface Subclass */
} usb_hid_subclass_code_t;

/**
 * Protocol code of HID device.
 * 4.3 Protocols
 */
typedef enum {
    USB_HID_PROTOCOL_CODE_NONE = 0x00,
    USB_HID_PROTOCOL_CODE_KEYBOARD = 0x01,
    USB_HID_PROTOCOL_CODE_MOUSE    = 0x02,
} usb_hid_protocol_code_t;

/**
 * Value for Get/Set_Protocol Request
 * 7.2.6 Set_Protocol Request
 */
typedef enum {
    USB_HID_PROTOCOL_BOOT   = 0x00,
    USB_HID_PROTOCOL_REPORT = 0x01,
} usb_hid_protocol_t;

/**
 * Descriptor Type of HID class
 * 4.1 The HID Class
 */
typedef enum {
    USB_HID_DESC_TYPE_HID    = 0x21,
    USB_HID_DESC_TYPE_REPORT = 0x22,
    USB_HID_DESC_TYPE_PHY    = 0x23,
} usb_hid_desc_type_t;

/**
 * HID Class Specification release number
 */
typedef enum {
    USB_HID_BCD_101 = 0x0101,
    USB_HID_BCD_110 = 0x0110,
    USB_HID_BCD_111 = 0x0111,
} usb_hid_bcd_t;

/**
 * USB HID Country Code
 *
 * Most device is zero, not supported.
 * Keyboards may use the field to indicate the language of the key caps.
 */
typedef enum {
    USB_HID_CONTRY_NONE = 0,
} usb_hid_contry_t;

typedef enum {
    USB_HID_BREQUEST_GET_REPORT   = 0x01,
    USB_HID_BREQUEST_GET_IDLE     = 0x02,
    USB_HID_BREQUEST_GET_PROTOCOL = 0x03,
    USB_HID_BREQUEST_SET_REPORT   = 0x09,
    USB_HID_BREQUEST_SET_IDLE     = 0x0A,
    USB_HID_BREQUEST_SET_PROTOCOL = 0x0B,
} usb_hid_brequest_t;

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
} usb_hid_dscr_interface_t;

typedef struct __attribute__((packed)) {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
} usb_hid_dscr_endpoint_t;

/**
 * The continued descriptor follow HID Descriptor
 *
 * The num of continued descriptors is 1 in most condiction.
 */
typedef struct __attribute__((packed)) {
    uint8_t bDescriptorType;
    uint16_t wDescriptorLength;
} usb_hid_cont_dscr_t;

/**
 * HID Descriptor
 *
 * bNumDescriptors is 1 in most condiction.
 */
#define USB_DSCR_HID_MAX_CONT_DSCRS 5
typedef struct __attribute__((packed)) {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdHID;
    uint8_t bCountryCode;
    uint8_t bNumDescriptors;
    usb_hid_cont_dscr_t cont_dscrs[USB_DSCR_HID_MAX_CONT_DSCRS];
} usb_hid_dscr_hid_t;

/**
 * USB hid descriptor serialize function
 */
typedef int16_t (*usb_hid_serial_func_t)(void *, void *, uint16_t);

typedef struct {
    void *dscr;
    usb_hid_serial_func_t serial_func;
} usb_hid_dscr_hdlr_t;

typedef enum {
    USB_REPORT_DSCR_TYPE_AC,
    USB_REPORT_DSCR_TYPE_MUX,
    USB_REPORT_DSCR_TYPE_TEAMS,
    USB_REPORT_DSCR_TYPE_TELEPHONY,
    USB_REPORT_DSCR_TYPE_KEYBOARD,
    USB_REPORT_DSCR_TYPE_GYROMETER,
    USB_REPORT_DSCR_TYPE_SENSOR,
    USB_REPORT_DSCR_TYPE_CFU,
    USB_REPORT_DSCR_TYPE_CUSTOMER,
    USB_REPORT_DSCR_TYPE_GMOUSE,
    USB_REPORT_DSCR_TYPE_GKEYBOARD,
    USB_REPORT_DSCR_TYPE_GMOUSE_NV,
    USB_REPORT_DSCR_TYPE_MUX2,
    USB_REPORT_DSCR_TYPE_NUM, /* The available num of hid report dscrs */
} usb_hid_report_dscr_type_t;

typedef struct {
    usb_hid_report_dscr_type_t type;
    const char *name;
    const uint8_t *dscr;
    uint8_t length;
    bool enable;
    uint8_t hid_port;
} usb_hid_report_dscr_hdlr_t;

/******************************************************************************
 * USB HID Driver Structures
 *****************************************************************************/
typedef enum {
    USB_HID_DEV_PORT_0 = 0,
    USB_HID_DEV_PORT_1 = 1,
    USB_HID_DEV_PORT_2 = 2,
    USB_HID_DEV_PORT_3 = 3,
    USB_HID_DEV_PORT_4 = 4,
    USB_HID_DEV_PORT_5 = 5,
    USB_HID_DEV_PORT_6 = 6,
    USB_HID_DEV_PORT_7 = 7,
} usb_hid_dev_port_t;

#define USB_HID_DEV_PORT_0_MASK (1 << USB_HID_DEV_PORT_0)
#define USB_HID_DEV_PORT_1_MASK (1 << USB_HID_DEV_PORT_1)
#define USB_HID_DEV_PORT_2_MASK (1 << USB_HID_DEV_PORT_2)
#define USB_HID_DEV_PORT_3_MASK (1 << USB_HID_DEV_PORT_3)
#define USB_HID_DEV_PORT_4_MASK (1 << USB_HID_DEV_PORT_4)
#define USB_HID_DEV_PORT_5_MASK (1 << USB_HID_DEV_PORT_5)
#define USB_HID_DEV_PORT_6_MASK (1 << USB_HID_DEV_PORT_6)
#define USB_HID_DEV_PORT_7_MASK (1 << USB_HID_DEV_PORT_7)

typedef enum {
    USB_HID_EP_INTERVAL_125US = 0,
    USB_HID_EP_INTERVAL_250US = 1,
    USB_HID_EP_INTERVAL_500US = 2,
    USB_HID_EP_INTERVAL_1MS   = 3,
    USB_HID_EP_INTERVAL_2MS   = 4,
    USB_HID_EP_INTERVAL_4MS   = 5,
    USB_HID_EP_INTERVAL_8MS   = 6,
    USB_HID_EP_INTERVAL_16MS  = 7,
    USB_HID_EP_INTERVAL_NUM   = 8,
} usb_hid_ep_interval_t;

/* HID device structure */
typedef struct {
    /**
     * Config Variables
     * Would effect descriptor
     */
    bool                        enable;
    bool                        report_en_list[USB_REPORT_DSCR_TYPE_NUM];
    usb_hid_protocol_code_t     protocol_code;
    usb_hid_ep_interval_t       in_interval;

    /**
     * Static Variables
     * Would exchange value with host
     */
#ifdef USB_HID_SUPPORT_IDLE_RATE
    uint8_t *idle_val;
#endif
    usb_hid_protocol_t          protocol;

    /**
     * Internal Resource Variables
     * USB SW will use these varaibles to access resources
     */
    uint8_t                     data_interface_id;
    uint8_t                     data_ep_in_id;
    uint16_t                    report_dscr_length;
    uint8_t                     *report_dscr;
    usb_hid_dscr_interface_t    *if_dscr;
    usb_hid_dscr_hid_t          *hid_dscr;
    usb_hid_dscr_endpoint_t     *ep_in_dscr;
    Usb_Interface_Info          *if_info;
    Usb_Ep_Info                 *ep_in_info;
    Usb_IAD_Dscr                *iad_desc;
} UsbHid_Struct;

/**
 * This struct use to allocate a big ROM/RAM for big data stored like
 * descriptor.
 */
typedef struct __attribute__((__packed__)) {
    usb_hid_dscr_interface_t   if_dscr;
    usb_hid_dscr_hid_t         hid_dscr;
    usb_hid_dscr_endpoint_t    ep_in_dscr;
    uint8_t report_dscr[USB_HID_REPOTR_DSCR_MAX_SIZE];
#ifdef USB_HID_SUPPORT_IDLE_RATE
    uint8_t idle_val[255];
#endif
} usb_hid_resource_t;

extern UsbHid_Struct g_UsbHids[];
extern const usb_create_if_func_ptr usb_hid_if_create_funcs[8];
extern const usb_void_func usb_hid_if_init_funcs[8];
extern const usb_void_func usb_hid_if_enable_funcs[8];
extern const usb_speed_if_func_ptr usb_hid_if_speed_funcs[8];
extern const usb_void_func usb_hid_if_resume_funcs[8];

bool usb_hid_get_descriptor(Usb_Ep0_Status *pep0state, Usb_Command *pcmd);

void usb_hid_set_dscr_enable(usb_hid_report_dscr_type_t* list, uint8_t len);

void USB_Init_Hid_Status(void);
void USB_Release_Hid_Status(void);

void usb_hid_device_enable(uint8_t portmask);
void usb_hid_report_enable(uint8_t port, usb_hid_report_dscr_type_t* list, uint8_t len);
void usb_hid_set_interval(uint8_t port, usb_hid_ep_interval_t interval);
void usb_hid_set_protocol_code(uint8_t port, usb_hid_protocol_code_t subclass);

uint8_t usb_hid_is_boot_mode(uint8_t port);

uint8_t usb_hid_find_port_by_report(usb_hid_report_dscr_type_t report_type);
uint8_t usb_hid_tx_blocking(uint8_t port, uint8_t* data, uint16_t len, uint16_t timeout);
uint8_t usb_hid_tx_non_blocking(uint8_t port, uint8_t* data, uint16_t len);

uint8_t usb_hid_chk_suspend_and_rmwk(void);

#endif /* AIR_USB_HID_ENABLE */
#endif /* USBHID_DRV_H */
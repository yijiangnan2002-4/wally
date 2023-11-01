/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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


#ifndef __USB_HOST_H__
#define __USB_HOST_H__

/* C library */
#include <stdbool.h>
#include <stdint.h>

/* USB Middleware includes */
#include "usb_host_def.h"
#include "usb_host_device_op.h"
#include "usb_host_spec.h"

/* Hal includes */
#include "hal_usb_host.h"

/*! @brief USB host handle type define */
typedef void *usb_host_handle;
typedef void *usb_device_handle;

typedef void *usb_host_configuration_handle;

extern void *pvPortMallocNC( size_t xWantedSize );

/* Control request structure */
typedef struct _usb_host_ctrl_req {
    uint8_t               pipe_size;
    uint8_t               buff[64];
    uint16_t              length;
    usb_host_SetupPkt_t   setup;
    CTRL_StateTypeDef     state;
    uint8_t               errorcount;
} usb_host_ctrl_req_t;

typedef enum _usb_host_status_t {
    Status_USB_Success = 0x00U,            /*!< Success */
    Status_USB_Error,                      /*!< Failed */
    Status_USB_Busy,                       /*!< Busy */
    Status_USB_InvalidHandle,              /*!< Invalid handle */
    Status_USB_InvalidParameter,           /*!< Invalid parameter */
    Status_USB_InvalidRequest,             /*!< Invalid request */
    Status_USB_NotSupported,               /*!< Configuration is not supported */
    Status_USB_Retry,                      /*!< Enumeration get configuration retry */
    Status_USB_TransferStall,              /*!< Transfer stalled */
    Status_USB_TransferFailed,             /*!< Transfer failed */
    Status_USB_AllocFail,                  /*!< Allocation failed */
    Status_USB_TransferCancel,             /*!< The transfer cancelled */
    Status_USB_MSCStatusFail,              /*!< For MSD, the CSW status means fail */
    Status_USB_DataOverRun,                /*!< The amount of data returned by the endpoint exceeded either the size of the maximum data packet allowed from the endpoint or the remaining buffer size. */
} usb_host_status_t;

typedef usb_host_status_t (*host_callback_t)(usb_device_handle deviceHandle, usb_host_configuration_handle configurationHandle, uint32_t eventCode);


/*! @brief USB host instance structure */
typedef struct _usb_host_instance {
    HOST_StateTypeDef       gState;
    host_callback_t         deviceCallback;                                  /*!< Device attach/detach callback*/
    //usb_host_transfer_t transferList[USB_HOST_CONFIG_MAX_TRANSFERS]; /*!< Transfer resource*/
    //usb_host_transfer_t *transferHead;                               /*!< Idle transfer head*/
    void *deviceList;                                                /*!< Device list*/
    uint8_t addressBitMap[16];

} usb_host_instance_t;

/*! @brief Device instance */
typedef struct _usb_device_instance    usb_device_instance_t;
struct _usb_device_instance {
    struct _usb_device_instance             *next;                  /*  Next device, or NULL */
    usb_host_handle                         hostHandle;             /*  Host handle */
    usb_host_descriptor_device_t            *deviceDescriptor;      /*  Standard device descriptor */
    uint8_t *configurationDesc;                                     /*!< Configuration descriptor pointer */
    uint16_t configurationLen;                                      /*!< Configuration descriptor length */
    uint16_t configurationValue;                                    /*!< Configuration index */
    usb_host_ctrl_req_t                     Control_Pipe;
    usb_host_device_enumeration_status_t    enum_state;             /*!< Device state for enumeration */

    usb_host_status_t (*enum_callback)(usb_device_instance_t *device_instance, uint16_t dataLength); /*! When the last step transfer is done, the function is used to process the transfer data */

    uint8_t speed;
    uint8_t allocatedAddress;                  /* Temporary address for the device. When set address request succeeds, This value would write to "setAddress" field */
    uint8_t setAddress;                        /* 1 - 127 */
    uint8_t deviceAttachState;

};

typedef enum {
    /* Attach */
    USB_HOST_ATTACH_MSG               = 1,
    /* Detach */
    USB_HOST_DETACH_MSG               = 2,
    /* Reset  */
    USB_HOST_RESET_MSG                = 3,
    /* Suspend  */
    USB_HOST_SUSPEND_MSG              = 4,
    /* Resume  */
    USB_HOST_RESUME_MSG               = 5,
    /* Enumeration Done  */
    USB_HOST_ENUM_DONE_MSG            = 6,
    /* Control Pipe  */
    USB_CONTROL_PIPE_MSG              = 7,
    /* EPx Tx  */
    USB_EP_TX_MSG                     = 8,
    /* EPx Rx  */
    USB_EP_RX_MSG                     = 9,
    /* DMA Done  */
    USB_DMA_DONE_MSG                  = 10

} usb_host_msg_type_t;

typedef struct {
    usb_host_msg_type_t host_msg_id;
    void *data;
} usb_host_msg_t;



/*! @ Function prototype define */
usb_host_status_t USB_Host_Enumeration_Proccess(usb_host_handle hostHandle);
void Usb_Host_start(void);


#endif
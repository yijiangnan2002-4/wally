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


#ifndef __HAL_USB_HOST_H__
#define __HAL_USB_HOST_H__

#include "hal_platform.h"

#ifdef HAL_USB_HOST_MODULE_ENABLED

#include "usb_host_def.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @brief  The maximum packet size for high-speed bulk endpoints.*/
#define HAL_USB_HOST_HIGH_SPEED_BULK_MAX_PACKET_SIZE	512

/** @brief This is the direction of USB endpoint. The direction is based on the USB host's view. */
typedef enum {
    HAL_USB_HOST_EP_DIRECTION_RX = 0,    /**<  Host IN */
    HAL_USB_HOST_EP_DIRECTION_TX         /**<  Host OUT */
} hal_usb_host_endpoint_direction_t;

/** @brief Transfer type defined in USB 2.0 Specification. BULK, INTR, ISO */
typedef enum {
    HAL_USB_HOST_EP_TRANSFER_ILLEGLE = 0,   /**<  Illegal transfer type. */
    HAL_USB_HOST_EP_TRANSFER_ISO,           /**<  Isochronous pipe. */
    HAL_USB_HOST_EP_TRANSFER_BULK,          /**<  Bulk pipe. */
    HAL_USB_HOST_EP_TRANSFER_INTR           /**<  Interrupt pipe. */
} hal_usb_host_endpoint_transfer_type_t;

/** @brief Hal USB Host status. */
typedef enum {
    HAL_USB_HSOT_STATUS_INVALID_PARAMETER = -2,  /**<  An invalid parameter was given. */
    HAL_USB_HOST_STATUS_ERROR             = -1,  /**<  The function call failed. */
    HAL_USB_HOST_STATUS_OK                =  0   /**<  The function call was successful. */
} hal_usb_host_status_t;

/**
 * @brief This function initializes the USB host hardware module.
 * Call this function before using the USB host service.
 * It also enables the USB related clock tree and power source.
 * @return #HAL_USB_HOST_STATUS_OK, if the operation completed successfully.
 */
hal_usb_host_status_t hal_usb_host_init(void);
/**
 * @brief This function gets the USB device speed status, such as HS/FS/LS.
 * @return enum of USBH_SpeedTypeDef.
 * Sample code:
 * @code
 * void Get_USB_Device_Speed(void)
 * {
 *    USBH_SpeedTypeDef   speed;
 *
 *    speed = Check_Attach_Device_Speed();;
 * }
 * @endcode
 */
USBH_SpeedTypeDef hal_usb_host_check_attach_device_speed(void);
/**
 * @brief This function USB host issues the USB Reset signal to device.
 * @return #HAL_USB_HOST_STATUS_OK, if the operation completed successfully.
 */
hal_usb_host_status_t hal_usb_host_reset(void);
/**
 * @brief This function get default pipe endpoint 0 rx_ready bit is set or not.
 * @return #HAL_USB_HOST_STATUS_OK, if rx_ready bit is set, and #HAL_USB_HOST_STATUS_ERROR if rx_ready bit is not set
 */
hal_usb_host_status_t hal_usb_host_check_ep0_rx_ready(void);
/**
 * @brief This function set the USB host rx_request bit for a specific endpoint number in order to require data from this endpoint.
 * @param[in] ep_num the endpoint number required for data.
 * @return #HAL_USB_HOST_STATUS_OK, if the operation completed successfully
 */
hal_usb_host_status_t hal_usb_host_set_ep_rx_request(uint32_t ep_num);
/**
 * @brief This function reads the USB data from FIFO.
 * Reading from these addresses unloads data from the RX FIFO for the corresponding endpoint.
 * @param[in] ep_num is the endpoint number.
 * @param[in] nBytes is the number of bytes to read.
 * @param[in] pDst is a pointer to the destination.
 * @return #HAL_USB_HOST_STATUS_OK, if the operation completed successfully.
 */
hal_usb_host_status_t hal_usb_host_read_endpoint_fifo(uint32_t ep_num, uint16_t nBytes, void *pDst);
/**
 * @brief This function is for USB host to issue a setup command.
 * @param[in] pSrc is a pointer to the setup command source.
 * @param[in] len is the number of bytes of setup command.
 * @return #HAL_USB_HOST_STATUS_OK, if the operation completed successfully.
 */
hal_usb_host_status_t hal_usb_host_issue_setup_command(void *pSrc, uint32_t len);
/**
 * @brief This function sets the USB packet ready once the data is fully written into FIFO.
 * Hardware will send the packet out if data is written to the FIFO and the TX packet is set to ready.
 * Null length packet (ZLP) will be sent if the data is not written to the FIFO.
 * @param[in] ep_num is the endpoint number.
 * @return #HAL_USB_HOST_STATUS_OK, if the operation completed successfully.
 */
hal_usb_host_status_t hal_usb_host_set_ep_tx_ready(uint32_t ep_num);
/**
 * @brief This function clear rxcsr reg for the specific endpoint.
 * @param[in] ep_num is the endpoint number.
 * @return #HAL_USB_HOST_STATUS_OK, if the operation completed successfully.
 */
hal_usb_host_status_t hal_usb_host_clear_rxcsr(uint32_t ep_num);
/**
 * @brief This function initialize the USB endpoint in RX direction.
 * @param[in] ep_num is the endpoint to initialize.
 * @param[in] data_size is the maximum packet size at this endpoint. For more details about maximum packet size, please refer to USB 2.0 Specification.
 * @param[in] type is the endpoint type defined in USB 2.0 Specification, such as CTRL, BULK, INTR, ISO endpoint.
 * @param[in] double_fifo allocates two FIFO buffers, if double_fifo is enabled.
 * @return #HAL_USB_HOST_STATUS_OK, if the operation completed successfully.
 */
hal_usb_host_status_t hal_usb_host_rx_ep_init(uint32_t ep_num, uint16_t data_size, hal_usb_host_endpoint_transfer_type_t type, bool double_fifo);
/**
 * @brief This function initializes the USB endpoint in TX direction.
 * @param[in] ep_num is the endpoint to initialize.
 * @param[in] data_size is the maximum packet size at this endpoint. For more details about maximum packet size, please refer to USB 2.0 Specification.
 * @param[in] type is the endpoint type defined in USB 2.0 Specification, such as CTRL, BULK, INTR, ISO endpoint.
 * @param[in] double_fifo two FIFO buffers are allocated, if double_fifo is enabled.
 * It may cost more FIFO SRAM.
 * @return #HAL_USB_HOST_STATUS_OK, if the operation completed successfully.
 */
hal_usb_host_status_t hal_usb_host_tx_ep_init(uint32_t ep_num, uint16_t data_size, hal_usb_host_endpoint_transfer_type_t type, bool double_fifo);
/**
 * @brief This function sets up the USB DMA channel to transfer data.
 * @param[in] ep_num is the endpoint number.
 * @param[in] speed is enum of USBH_SpeedTypeDef means HS or FS.
 * @param[in] direction is enum of hal_usb_host_endpoint_direction_t means RX or TX.
 * @param[in] ep_type is the endpoint type defined in USB 2.0 Specification, such as CTRL, BULK, INTR, ISO endpoint.
 * @param[in] addr is a pointer to the address to apply the DMA transaction.
 * @param[in] length is the data length in bytes.
 * @return #HAL_USB_HOST_STATUS_OK, if the operation completed successfully.
 */
hal_usb_host_status_t hal_usb_host_start_dma_transfer(uint32_t ep_num, USBH_SpeedTypeDef speed, hal_usb_host_endpoint_direction_t direction, hal_usb_host_endpoint_transfer_type_t ep_type, uint32_t addr, uint32_t length);
/**
 * @brief This function gets the usb device function address number.
 * @return device function address number.
 */
uint8_t hal_usb_host_get_address(void);
/**
 * @brief This function gets the data packet length at the endpoint 0 RX FIFO.
 * @return the length of packet size in bytes.
 */
uint32_t hal_usb_host_get_ep0_pkt_len(void);
/**
 * @brief This function gets the data packet length in FIFO at the specific endpoint.
 * @param[in] ep_num is the endpoint number.
 * @return the length of packet size in bytes.
 */
uint32_t hal_usb_host_get_ep_rx_pkt_len(uint32_t ep_num);
/**
 * @brief This function writes the USB data to FIFO.
 * Writing to these addresses loads data into the TX FIFO for the corresponding endpoint.
 * @param[in] ep_num is the endpoint number.
 * @param[in] nBytes is the number of bytes to write.
 * @param[in] pSrc is a pointer to the source.
 * @return #HAL_USB_HOST_STATUS_OK, if the operation completed successfully.
 */
void hal_usb_host_write_ep_fifo(uint32_t ep_num, uint16_t nBytes, void *pSrc);
/**
 * @brief This function clear the specific endpoint rx_pkt_ready bit.
 * @param[in] ep_num is the endpoint number.
 */
void hal_usb_host_clear_ep_rx_pkt_ready(uint32_t ep_num);
/**
 * @brief This function set usb function address value to reg.
 * @param[in] addr is the device function address number.
 */
void hal_usb_host_set_address(uint8_t addr);
/**
 * @brief This function clear the endpoint 0 rx_pkt_ready bit.
 */
void hal_usb_host_clear_ep0_rx_ready(void);
/**
 * @brief This function issue Null length packet (ZLP) on endpoint 0 for status phase.
 */
void hal_usb_host_set_ep0_out_status(void);
/**
 * @brief This function receive Null length packet (ZLP) on endpoint 0 for status phase.
 */
void hal_usb_host_set_ep0_in_status(void);
/**
 * @brief This function clear endpoint 0 rx_pkt_ready bit & status_pkt bit.
 */
void hal_usb_host_clear_ep0_rx_status(void);
/**
 * @brief This function set endpoint 0 rx_request_pkt bit to do rx request.
 */
void hal_usb_host_set_ep0_rx_request(void);
/**
 * @brief This function set endpoint 0 rx_request_pkt bit.
 * @param[in] pkt_num is the total packet number.
 */
void hal_usb_host_set_rx_pkt_count(uint16_t pkt_num);


#endif /* HAL_USB_HOST_MODULE_ENABLED */

#ifdef __cplusplus
}
#endif
#endif /* __HAL_USB_HOST_H__ */

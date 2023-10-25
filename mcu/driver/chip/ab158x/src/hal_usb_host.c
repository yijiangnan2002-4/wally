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

#include "hal_usb_host.h"

#ifdef HAL_USB_HOST_MODULE_ENABLED

#include "hal_clock.h"
#include "hal_clock_internal.h"
#include "hal_clock_platform.h"
#include "hal_dvfs.h"
#include "hal_eint.h"
#include "hal_gpio.h"
#include "hal_gpt.h"
#include "hal_log.h"
#include "hal_nvic.h"
#include "hal_platform.h"
#include "hal_pmu.h"
#include "hal_resource_assignment.h"
#include "hal_usb_internal.h"

#include "usb_host.h"
#include "usb_host_def.h"
#include "usb_host_utility.h"

static volatile USB_REGISTER_T *musb = (USB_REGISTER_T *)USB_BASE;
static volatile USBPHY_HOST_REGISTER_T *musb_phy = (USBPHY_HOST_REGISTER_T *)USB_SIFSLV_BASE;

static volatile uint8_t device_attach = 0, ep0_tx_done = 0, ep0_rx_done = 0  ;


void hal_usb_phy_setting_print(void)
{
    log_hal_msgid_info("phy reg addr:0x%x, value:0x%x", 2, &musb_phy->usbphyacr0, (USB_DRV_Reg32(&musb_phy->usbphyacr0)));
    log_hal_msgid_info("phy reg addr:0x%x, value:0x%x", 2, &musb_phy->usbphyacr1, (USB_DRV_Reg32(&musb_phy->usbphyacr1)));
    log_hal_msgid_info("phy reg addr:0x%x, value:0x%x", 2, &musb_phy->usbphyacr2, (USB_DRV_Reg32(&musb_phy->usbphyacr2)));
    log_hal_msgid_info("phy reg addr:0x%x, value:0x%x", 2, &musb_phy->usbphyacr4, (USB_DRV_Reg32(&musb_phy->usbphyacr4)));
    log_hal_msgid_info("phy reg addr:0x%x, value:0x%x", 2, &musb_phy->usbphyacr5, (USB_DRV_Reg32(&musb_phy->usbphyacr5)));
    log_hal_msgid_info("phy reg addr:0x%x, value:0x%x", 2, &musb_phy->usbphyacr6, (USB_DRV_Reg32(&musb_phy->usbphyacr6)));
    log_hal_msgid_info("phy reg addr:0x%x, value:0x%x", 2, &musb_phy->u2phyacr3, (USB_DRV_Reg32(&musb_phy->u2phyacr3)));
    log_hal_msgid_info("phy reg addr:0x%x, value:0x%x", 2, &musb_phy->u2phyacr4, (USB_DRV_Reg32(&musb_phy->u2phyacr4)));
    log_hal_msgid_info("phy reg addr:0x%x, value:0x%x", 2, &musb_phy->u2phydcr0, (USB_DRV_Reg32(&musb_phy->u2phydcr0)));
    log_hal_msgid_info("phy reg addr:0x%x, value:0x%x", 2, &musb_phy->u2phydcr1, (USB_DRV_Reg32(&musb_phy->u2phydcr1)));
    log_hal_msgid_info("phy reg addr:0x%x, value:0x%x", 2, &musb_phy->u2phydtm0, (USB_DRV_Reg32(&musb_phy->u2phydtm0)));
    log_hal_msgid_info("phy reg addr:0x%x, value:0x%x", 2, &musb_phy->u2phydtm1, (USB_DRV_Reg32(&musb_phy->u2phydtm1)));
    log_hal_msgid_info("phy reg addr:0x%x, value:0x%x", 2, &musb_phy->u2phydmon0, (USB_DRV_Reg32(&musb_phy->u2phydmon0)));
}

hal_usb_host_status_t Check_Attach_Full_Speed_J_State(void)
{
    if ((USB_DRV_Reg8(&musb->devctl) & (USB_DEVCTL_HOSTMODE | USB_DEVCTL_FSDEV | USB_DEVCTL_LSDEV)) == (USB_DEVCTL_HOSTMODE | USB_DEVCTL_FSDEV)) {
        log_hal_msgid_info("J State & Host mode !!\r\n", 0);
        return HAL_USB_HOST_STATUS_OK;
    } else {
        log_hal_msgid_info("J state or Host mode error !!\r\n", 0);
        return HAL_USB_HOST_STATUS_ERROR;
    }
}

USBH_SpeedTypeDef hal_usb_host_check_attach_device_speed(void)
{
    if ((USB_DRV_Reg8(&musb->power) & USB_POWER_HSMODE)) {
        log_hal_msgid_info("High Speed Device !!\r\n", 0);
        return USB_HOST_SPEED_HIGH ;
    } else if ((USB_DRV_Reg8(&musb->devctl) & (USB_DEVCTL_LSDEV))) {
        log_hal_msgid_info("Low Speed Device !!\r\n", 0);
        return USB_HOST_SPEED_LOW ;
    } else {
        log_hal_msgid_info("Full Speed Device !!\r\n", 0);
        return USB_HOST_SPEED_FULL;
    }
}

hal_usb_host_status_t hal_usb_host_reset(void)
{
    // Host start to reset device
    USB_DRV_SetBits8(&musb->power, USB_POWER_RESET);
    hal_gpt_delay_ms(25);
    USB_DRV_ClearBits8(&musb->power, USB_POWER_RESET);
    return HAL_USB_HOST_STATUS_OK;
}

static void usb_host_eint_hisr(void)
{
    /*log_hal_msgid_info("usb_eint_hisr\n", 0);*/
}

hal_usb_host_status_t hal_usb_host_rx_ep_init(uint32_t ep_num, uint16_t data_size, hal_usb_host_endpoint_transfer_type_t type, bool double_fifo)
{
    uint32_t savedMask;

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, ep_num);

    //USB_DRV_WriteReg(&musb->txcsr, 0x00);
    USB_DRV_WriteReg(&musb->rxcsr, (USB_RXCSR_FLUSHFIFO | USB_RXCSR_CLRDATATOG));
    /* Double buffers, so flush twice */
    if (double_fifo) {
        USB_DRV_WriteReg(&musb->rxcsr, (USB_RXCSR_FLUSHFIFO | USB_RXCSR_CLRDATATOG));
    }

    USB_DRV_WriteReg(&musb->rxmap, data_size);
    //USB_DRV_WriteReg(&musb->rx1map, data_size);

    USB_DRV_WriteReg8(&musb->rxtype, ((type << 4) | ep_num));
    if (type == HAL_USB_HOST_EP_TRANSFER_BULK) {
        USB_DRV_WriteReg8(&musb->rxinterval, 0);
        USB_DRV_WriteReg8(&musb->rxfifosz, USB_FIFOSZ_SIZE_512);
    }
    USB_DRV_SetBits(&musb->intrrxe, (USB_INTRE_EPEN << ep_num));

    hal_nvic_restore_interrupt_mask(savedMask);

    log_hal_msgid_info("rx1map address %x, value %x", 2, &musb->rx1map, (USB_DRV_Reg(&musb->rx1map)));
    log_hal_msgid_info("rxfifosz address %x, value %x", 2, &musb->rxfifosz, (USB_DRV_Reg8(&musb->rxfifosz)));

    return HAL_USB_HOST_STATUS_OK;
}

hal_usb_host_status_t hal_usb_host_tx_ep_init(uint32_t ep_num, uint16_t data_size, hal_usb_host_endpoint_transfer_type_t type, bool double_fifo)
{
    uint32_t savedMask;

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, ep_num);

    //USB_DRV_WriteReg(&musb->txcsr, 0x00);
    USB_DRV_WriteReg(&musb->txcsr, (USB_TXCSR_FLUSHFIFO | USB_TXCSR_CLRDATATOG));
    /* Double buffers, so flush twice */
    if (double_fifo) {
        USB_DRV_WriteReg(&musb->txcsr, (USB_TXCSR_FLUSHFIFO | USB_TXCSR_CLRDATATOG));
    }
    USB_DRV_SetBits(&musb->txcsr, (USB_TXCSR_MODE));   // tx mode
    USB_DRV_WriteReg(&musb->txmap, data_size);

    USB_DRV_WriteReg8(&musb->txtype, ((type << 4) | ep_num));
    if (type == HAL_USB_HOST_EP_TRANSFER_BULK) {
        USB_DRV_WriteReg8(&musb->txinterval, 0);
        USB_DRV_WriteReg8(&musb->txfifosz, USB_FIFOSZ_SIZE_512);
    }
    USB_DRV_SetBits(&musb->intrtxe, (USB_INTRE_EPEN << ep_num));

    hal_nvic_restore_interrupt_mask(savedMask);


    return HAL_USB_HOST_STATUS_OK;
}

hal_usb_host_status_t hal_usb_host_clear_rxcsr(uint32_t ep_num)
{
    uint32_t savedMask;

    hal_nvic_save_and_set_interrupt_mask(&savedMask);

    USB_DRV_WriteReg8(&musb->index, ep_num);
    USB_DRV_WriteReg(&musb->rxcsr, 0x00);

    hal_nvic_restore_interrupt_mask(savedMask);

    return HAL_USB_HOST_STATUS_OK;
}

hal_usb_host_status_t hal_usb_host_start_dma_transfer(uint32_t ep_num, USBH_SpeedTypeDef speed, hal_usb_host_endpoint_direction_t direction, hal_usb_host_endpoint_transfer_type_t ep_type, uint32_t addr, uint32_t length)
{
    uint32_t savedMask;
    uint16_t dma_ctrl;
    uint8_t dma_ch, is_single = 1;

    dma_ch = ep_num % 8 ;

    log_hal_msgid_info("DMA channel %d!!\r\n", 1, dma_ch);

    if (((uint32_t)addr % 4) != 0) {
        log_hal_msgid_info("DMA address is not aligned 32 bit %x!!\r\n", 1, addr);
        return HAL_USB_HOST_STATUS_ERROR;
    }

    if (direction == HAL_USB_HOST_EP_DIRECTION_RX) {
        /* Stop DMA channel */
        USBDMA_Stop(dma_ch);

        if (speed == USB_HOST_SPEED_HIGH) {
            switch (ep_type) {
                case HAL_USB_HOST_EP_TRANSFER_BULK:

                    if (length <= HAL_USB_HOST_HIGH_SPEED_BULK_MAX_PACKET_SIZE) {
                        is_single = 1;
                    } else {
                        is_single = 0;
                    }

                    break;


            }
        }

        hal_nvic_save_and_set_interrupt_mask(&savedMask);
        USB_DRV_WriteReg8(&musb->index, ep_num);
        if (is_single) {
            USB_DRV_WriteReg(&musb->rxcsr, USB_DMA_RX_CSR_SINGLE | USB_RXCSR_RXPKTRDY);
        } else {
            USB_DRV_WriteReg(&musb->rxcsr, USB_DMA_RX_CSR | USB_RXCSR_AUTOREQ | USB_RXCSR_RXPKTRDY);
        }
        hal_nvic_restore_interrupt_mask(savedMask);

        log_hal_msgid_info("USB DMA Setup: len: %d addr:0x%x ", 2, length, addr);
        USB_DRV_WriteReg32(USB_DMAADDR(dma_ch), addr);
        USB_DRV_WriteReg32(USB_DMACNT(dma_ch), length);

        log_hal_msgid_info("USB DMA Setup: read back DMAADDR: 0x%x = 0x%x, DMACNT: 0x%x = 0x%x", 4,
                           USB_DMAADDR(dma_ch), USB_DRV_Reg32(USB_DMAADDR(dma_ch)),
                           USB_DMACNT(dma_ch), USB_DRV_Reg32(USB_DMACNT(dma_ch)));

        if (is_single) {
            dma_ctrl = USB_DMACNTL_INTEN | (ep_num << 4);
        } else {
            dma_ctrl = USB_DMACNTL_DMAMODE | USB_DMACNTL_INTEN | (ep_num << 4);
        }
        dma_ctrl |= USB_BURST_MODE_1 | USB_DMACNTL_DMAEN;
        USB_DRV_WriteReg(USB_DMACNTL(dma_ch), dma_ctrl);
    } else if (direction == HAL_USB_HOST_EP_DIRECTION_TX) {
        if (speed == USB_HOST_SPEED_HIGH) {
            switch (ep_type) {
                case HAL_USB_HOST_EP_TRANSFER_BULK:

                    if (length <= HAL_USB_HOST_HIGH_SPEED_BULK_MAX_PACKET_SIZE) {
                        is_single = 1;
                    } else {
                        is_single = 0;
                    }

                    break;


            }
        }
        hal_nvic_save_and_set_interrupt_mask(&savedMask);
        USB_DRV_WriteReg8(&musb->index, ep_num);
        if (is_single) {
            USB_DRV_WriteReg(&musb->txcsr, USB_DMA_TX_CSR_SINGLE);
        } else {
            USB_DRV_WriteReg(&musb->txcsr, USB_DMA_TX_CSR);
        }
        hal_nvic_restore_interrupt_mask(savedMask);

        log_hal_msgid_info("USB DMA Setup: len: %d addr:0x%x ", 2, length, addr);
        USB_DRV_WriteReg32(USB_DMAADDR(dma_ch), addr);
        USB_DRV_WriteReg32(USB_DMACNT(dma_ch), length);
        log_hal_msgid_info("USB DMA Setup: read back DMAADDR: 0x%x = 0x%x, DMACNT: 0x%x = 0x%x", 4,
                           USB_DMAADDR(dma_ch), USB_DRV_Reg32(USB_DMAADDR(dma_ch)),
                           USB_DMACNT(dma_ch), USB_DRV_Reg32(USB_DMACNT(dma_ch)));

        if (is_single) {
            dma_ctrl = USB_DMACNTL_DMADIR | USB_DMACNTL_INTEN | (ep_num << 4);
        } else {
            dma_ctrl = USB_DMACNTL_DMADIR | USB_DMACNTL_DMAMODE | USB_DMACNTL_INTEN | (ep_num << 4);
        }
        dma_ctrl |= USB_BURST_MODE_1 | USB_DMACNTL_DMAEN;
        USB_DRV_WriteReg(USB_DMACNTL(dma_ch), dma_ctrl);

    }

    return HAL_USB_HOST_STATUS_OK;
}

hal_usb_host_status_t hal_usb_host_set_ep_rx_request(uint32_t ep_num)
{
    uint32_t savedMask;

    log_hal_msgid_info("EP Rx Request !!!", 0);

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, ep_num);
    USB_DRV_SetBits(&musb->rxcsr, (USB_RXCSR_REQPKT));
    hal_nvic_restore_interrupt_mask(savedMask);

    return HAL_USB_HOST_STATUS_OK;
}

hal_usb_host_status_t hal_usb_host_set_ep_tx_ready(uint32_t ep_num)
{
    uint32_t savedMask;

    log_hal_msgid_info("EP Tx Ready !!!", 0);

    hal_gpt_delay_ms(3);
    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, ep_num);
    USB_DRV_SetBits(&musb->txcsr, USB_TXCSR_TXPKTRDY);
    hal_nvic_restore_interrupt_mask(savedMask);
    return HAL_USB_HOST_STATUS_OK;
}

void hal_usb_host_clear_ep_rx_pkt_ready(uint32_t ep_num)
{
    uint32_t savedMask;

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, ep_num);
    log_hal_msgid_info("RXCSR value:%x\r\n", 1, (USB_DRV_Reg(&musb->rxcsr)));
    USB_DRV_ClearBits(&musb->rxcsr, (USB_RXCSR_RXPKTRDY));
    hal_nvic_restore_interrupt_mask(savedMask);
}

void hal_usb_host_write_ep_fifo(uint32_t ep_num, uint16_t nBytes, void *pSrc)
{
    uint16_t nCount = nBytes;
    uint8_t *pby;
    uint32_t nAddr;
    uint32_t  temp_1, temp_2;
    uint32_t *pby1;

    if ((nBytes != 0) && (pSrc == NULL)) {
        log_hal_msgid_error("ASSERT\r\n", 0);
    }

    if (pSrc == NULL) {
        log_hal_msgid_info("usb_hw_epfifowrite Error: pSrc is NULL!!\r\n", 0);
        return;
    }

    /* not index register */
    nAddr = (uint32_t)(&musb->fifo0 + ep_num);

    if (((uint32_t)pSrc % 4) == 0) {
        /* Source address is 4 byte alignment */
        temp_1 = nCount / 4;
        temp_2 = nCount % 4;

        pby1 = (uint32_t *)pSrc;

        while (temp_1) {
            USB_DRV_WriteReg32(nAddr, *pby1++);
            temp_1--;
        }

        pby = (uint8_t *)pby1;

        while (temp_2) {
            USB_DRV_WriteReg8(nAddr, *pby++);
            temp_2--;
        }
    } else {
        pby = (uint8_t *)pSrc;

        /* write byte by byte */
        while (nCount) {
            USB_DRV_WriteReg8(nAddr, *pby++);
            nCount--;
        }
    }
}

hal_usb_host_status_t hal_usb_host_read_endpoint_fifo(uint32_t ep_num, uint16_t nBytes, void *pDst)
{
    uint8_t *pby;
    uint16_t nCount;
    uint32_t nAddr;
    uint32_t temp_1, temp_2;
    uint32_t *pby1;

    nCount = nBytes;

    if ((nBytes != 0) && (pDst == NULL)) {
        log_hal_msgid_error("hal_usb_host_read_endpoint_fifo : Get data but address is NULL Fail", 0);
        return HAL_USB_HOST_STATUS_ERROR;
    }

    if (pDst == NULL) {
        return HAL_USB_HOST_STATUS_ERROR;
    }

    /* not indexed register */
    nAddr = (uint32_t)(&musb->fifo0 + ep_num);

    if (((uint32_t)pDst % 4) == 0) {
        /* Destination address is 4 byte alignment */
        temp_1 = nCount / 4;
        temp_2 = nCount % 4;

        pby1 = (uint32_t *)pDst;

        while (temp_1) {
            *pby1++ = USB_DRV_Reg32(nAddr);    //lint !e613
            temp_1--;
        }

        pby = (uint8_t *)pby1;

        while (temp_2) {
            *pby++ = USB_DRV_Reg8(nAddr);    //lint !e613
            temp_2--;
        }
    } else {
        pby = (uint8_t *)pDst;

        /* Read byte by byte */
        while (nCount) {
            *pby++ = USB_DRV_Reg8(nAddr);    //lint !e613
            nCount--;
        }
    }

    return HAL_USB_HOST_STATUS_OK;
}

uint32_t hal_usb_host_get_ep_rx_pkt_len(uint32_t ep_num)
{
    uint16_t CSR;
    uint32_t savedMask;
    uint32_t nCount = 0;

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, ep_num);
    CSR = USB_DRV_Reg(&musb->rxcsr);

    if (CSR & USB_RXCSR_RXPKTRDY) {
        nCount = (uint32_t)USB_DRV_Reg(&musb->rxcount);
    }
    hal_nvic_restore_interrupt_mask(savedMask);

    return nCount;
}

uint32_t hal_usb_host_get_ep0_pkt_len(void)
{
    uint16_t CSR0;
    uint32_t savedMask;
    uint32_t nCount = 0;

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, 0);
    CSR0 = USB_DRV_Reg(&musb->csr0);

    if (CSR0 & USB_CSR0_RXPKTRDY) {
        nCount = (uint32_t)USB_DRV_Reg8(&musb->count0);
    }
    hal_nvic_restore_interrupt_mask(savedMask);

    return nCount;
}

void hal_usb_host_set_rx_pkt_count(uint16_t pkt_num)
{
    uint32_t savedMask;

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg(&musb->ep1rxpktcount, pkt_num);
    hal_nvic_restore_interrupt_mask(savedMask);
}

void hal_usb_host_set_ep0_rx_request(void)
{
    uint32_t savedMask;

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, 0);
    USB_DRV_SetBits(&musb->csr0, (USB_CSR0_REQPKT));
    hal_nvic_restore_interrupt_mask(savedMask);
}

hal_usb_host_status_t hal_usb_host_check_ep0_rx_ready(void)
{
    uint32_t savedMask;

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, 0);
    if ((USB_DRV_Reg(&musb->csr0) & USB_RXCSR_RXPKTRDY) == 0) {
        log_hal_msgid_error("ASSERT check_ep0_rx_ready\r\n", 0);
        hal_nvic_restore_interrupt_mask(savedMask);
        return HAL_USB_HOST_STATUS_ERROR;
    }

    hal_nvic_restore_interrupt_mask(savedMask);
    return HAL_USB_HOST_STATUS_OK;
}

void hal_usb_host_clear_ep0_rx_ready(void)
{
    uint32_t savedMask;

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, 0);
    USB_DRV_ClearBits(&musb->csr0, (USB_CSR0_RXPKTRDY));
    hal_nvic_restore_interrupt_mask(savedMask);
}

void hal_usb_host_clear_ep0_rx_status(void)
{
    uint32_t savedMask;

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, 0);
    USB_DRV_ClearBits(&musb->csr0, (USB_CSR0_RXPKTRDY | USB_CSR0_STATUSPKT));
    hal_nvic_restore_interrupt_mask(savedMask);
}

void Control_Pipe_Clean_Fifo(void)
{
    uint32_t savedMask;

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, 0);
    USB_DRV_WriteReg(&musb->csr0, USB_CSR0_FLUSHFIFO);
    hal_nvic_restore_interrupt_mask(savedMask);
}

void hal_usb_host_set_ep0_out_status(void)
{
    uint32_t savedMask;

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, 0);
    USB_DRV_SetBits(&musb->csr0, (USB_CSR0_TXPKTRDY | USB_CSR0_STATUSPKT));
    hal_nvic_restore_interrupt_mask(savedMask);
}

void hal_usb_host_set_ep0_in_status(void)
{
    uint32_t savedMask;

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, 0);
    USB_DRV_SetBits(&musb->csr0, (USB_CSR0_REQPKT | USB_CSR0_STATUSPKT));
    hal_nvic_restore_interrupt_mask(savedMask);
}

hal_usb_host_status_t hal_usb_host_issue_setup_command(void *pSrc, uint32_t len)
{
    uint32_t savedMask;

    log_hal_msgid_info("Issue Setup_command !!!", 0);

    hal_usb_host_write_ep_fifo(0, len, pSrc);
    hal_gpt_delay_ms(3);
    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, 0);
    USB_DRV_WriteReg(&musb->csr0, (USB_CSR0_SETUPPKT | USB_CSR0_TXPKTRDY));
    hal_nvic_restore_interrupt_mask(savedMask);

    return HAL_USB_HOST_STATUS_OK;
}

void hal_usb_host_set_address(uint8_t addr)
{
    //USB_DRV_WriteReg8(&musb->faddr, addr);
    USB_DRV_WriteReg(&musb->t0funcaddr, addr);
    USB_DRV_WriteReg(&musb->r1funcaddr, addr);
    USB_DRV_WriteReg(&musb->t2funcaddr, addr);

    log_hal_msgid_info("Read Device Address value:%x\r\n", 1, (USB_DRV_Reg(&musb->t0funcaddr)));
}

uint8_t hal_usb_host_get_address(void)
{
    log_hal_msgid_info("Read Device Address value:%x\r\n", 1, (USB_DRV_Reg(&musb->t0funcaddr)));
    return (USB_DRV_Reg(&musb->t0funcaddr));
}

void Generate_Test_Packet()
{
    volatile uint32_t delay = 0;

    uint8_t packet_test[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xAA, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE,
        0xEE, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xBF, 0xDF,
        0xEF, 0xF7, 0xFB, 0xFD, 0xFC, 0x7E, 0xBF, 0xDF,
        0xEF, 0xF7, 0xFB, 0xFD, 0x7E
    };

    hal_usb_host_write_ep_fifo(0, 53, packet_test);

    for (delay = 0; delay != 1000; delay++) ;

    USB_DRV_WriteReg8(&musb->index, 0);
    USB_DRV_WriteReg8(&musb->testmode, USB_TESTMODE_TESTPACKET);

    for (delay = 0; delay != 1000; delay++) ;

    USB_DRV_WriteReg(&musb->csr0, USB_CSR0_TXPKTRDY);


}

void usb_host_hisr(void)
{
    uint8_t   IntrUSB;
    uint16_t  IntrTx, IntrRx;
    uint32_t  DMAIntr_s;
    hal_usb_host_status_t ret ;

    // read and clear
    IntrUSB = USB_DRV_Reg8(&musb->intrusb);
    USB_DRV_WriteReg8(&musb->intrusb, IntrUSB);

    IntrTx = USB_DRV_Reg(&musb->intrtx);
    USB_DRV_WriteReg(&musb->intrtx, IntrTx);

    IntrRx = USB_DRV_Reg(&musb->intrrx);
    USB_DRV_WriteReg(&musb->intrrx, IntrRx);

    DMAIntr_s = USB_DRV_Reg8(&musb->dma_intr_status);
    USB_DRV_WriteReg8(&musb->dma_intr_status, DMAIntr_s);

    log_hal_msgid_info("###USB Host ISR %x\r\n", 1, IntrUSB);

    if (IntrUSB & USB_INTRUSB_CONN) {
        log_hal_msgid_info("Isr : Device attached !!!\r\n", 0);
        if ((USB_DRV_Reg8(&musb->devctl) & (USB_DEVCTL_FSDEV | USB_DEVCTL_LSDEV)) != USB_DEVCTL_FSDEV) {
            log_hal_msgid_error("ASSERT, Not High speed/Full speed device \r\n", 0);
        } else {
            log_hal_msgid_info("Device Attached DEVCTL value:%x\r\n", 1, (USB_DRV_Reg8(&musb->devctl)));
            device_attach  = 1 ;
            ret = Check_Attach_Full_Speed_J_State();
            if (ret == HAL_USB_HOST_STATUS_OK) {
                log_hal_msgid_info("Host Start to Reset Bus\r\n", 0);
                USB_Host_Send_Message_ISR(USB_HOST_ATTACH_MSG, NULL);
            }
        }
    }
    if (IntrUSB & USB_INTRUSB_DISCON) {
        //USB_DRV_SetBits8(&musb->intrusb, USB_INTRUSB_DISCON);
        log_hal_msgid_info("USB Device Detached %x\r\n", 0);
    }

    if (IntrTx) {
        log_hal_msgid_info("USB Tx Interrupt %x\r\n", 1, IntrTx);
        log_hal_msgid_info("CSR0 %x", 1, (USB_DRV_Reg(&musb->csr0)));
        if (IntrTx & USB_INTRTX_EP0) {
            log_hal_msgid_info("EP0 Tx done\r\n", 0);
            USB_Host_Send_Message_ISR(USB_CONTROL_PIPE_MSG, NULL);
            ep0_tx_done = 1 ;
        }
        if (IntrTx & USB_INTRTX_EP1) {
            log_hal_msgid_info("EP1 Tx done\r\n", 0);
        }
        if (IntrTx & USB_INTRTX_EP2) {
            log_hal_msgid_info("EP2 Tx done\r\n", 0);
            USB_Host_Send_Message_ISR(USB_EP_TX_MSG, NULL);
        }
    }

    if (IntrRx) {
        log_hal_msgid_info("USB Rx Interrupt %x\r\n", 1, IntrRx);
        if (IntrRx & USB_INTRTX_EP0) {
            log_hal_msgid_info("EP0 Rx done\r\n", 0);
            ep0_rx_done = 1 ;
        }
        if (IntrRx & USB_INTRRX_EP1) {
            USB_DRV_WriteReg8(&musb->index, 1);
            log_hal_msgid_info("RXCSR value:%x\r\n", 1, (USB_DRV_Reg(&musb->rxcsr)));
            //USB_DRV_ClearBits(&musb->rxcsr, USB_RXCSR_RXPKTRDY);
            log_hal_msgid_info("EP1 Rx done\r\n", 0);
            USB_Host_Send_Message_ISR(USB_EP_RX_MSG, NULL);
        }
    }

    if (DMAIntr_s) {
        log_hal_msgid_info("USB DMA Interrupt %x\r\n", 1, DMAIntr_s);
        //if(DMAIntr_s == 1)
        //    USB_Host_Send_Message_ISR(USB_DMA_DONE_MSG, NULL);
        //USB_Host_Send_Message_ISR(USB_DMA_DONE_MSG, NULL);
    }


}

hal_usb_host_status_t hal_usb_host_drv_create_isr(void)
{
    hal_eint_config_t eint_config;
    hal_eint_status_t result;

    /*interrupt*/
    hal_nvic_register_isr_handler((hal_nvic_irq_t)USB_IRQn, (hal_nvic_isr_t)usb_host_hisr);

    __set_BASEPRI(0);
    __set_PRIMASK(0);
    hal_nvic_enable_irq(USB_IRQn);

    //hal_nvic_set_pending_irq(USB_IRQn);

    /*eint*/
    eint_config.trigger_mode = HAL_EINT_LEVEL_LOW;
    eint_config.debounce_time = 0;
    hal_eint_mask(HAL_EINT_USB);
    result = hal_eint_init(HAL_EINT_USB, &eint_config);
    if (result != HAL_EINT_STATUS_OK) {
        log_hal_msgid_info("hal_eint_init fail: %d\r\n", 1, result);
        return HAL_USB_HOST_STATUS_ERROR;
    }
    result = hal_eint_register_callback((hal_eint_number_t)HAL_EINT_USB, (hal_eint_callback_t)usb_host_eint_hisr, NULL);
    if (result != HAL_EINT_STATUS_OK) {
        log_hal_msgid_info("hal_eint_register_callback fail: %d\r\n", 1, result);
        return HAL_USB_HOST_STATUS_ERROR;
    }

    return HAL_USB_HOST_STATUS_OK;
}

void Enable_L1_Int(void)
{
    USB_DRV_SetBits32(&musb->usb_l1intm, USB_L1INTM_DMA_INT_UNMASK | USB_L1INTM_USBCOM_INT_UNMASK | USB_L1INTM_TX_INT_UNMASK | USB_L1INTM_RX_INT_UNMASK);
    DRV_WriteReg8(&musb->dma_intr_unmask_set,  0xFF);
    log_hal_msgid_info("L1INTM addr:%x, value:%x\r\n", 2, &musb->usb_l1intm, (USB_DRV_Reg32(&musb->usb_l1intm)));
}

void hal_usb_host_control_pipe_tx_rx_interrupt_enable(void)
{
    /* enable EP0 tx rx interrupt */
    USB_DRV_SetBits(&musb->intrtxe, USB_INTRE_EPEN);
    USB_DRV_SetBits(&musb->intrrxe, USB_INTRE_EPEN);
}

hal_usb_host_status_t hal_usb_host_interrupt_init(void)
{
    hal_usb_host_status_t ret ;

    /* Enable USB system level interrupts, except SOF interrupt */
    USB_DRV_WriteReg8(&musb->intrusbe, ~(USB_INTRUSBE_SOF | USB_INTRUSBE_SESSREQ));

    ret = hal_usb_host_drv_create_isr();
    if (ret != HAL_USB_HOST_STATUS_OK) {
        log_hal_msgid_error("ASSERT, hal_usb_host_drv_create_isr failed \r\n", 0);
        return HAL_USB_HOST_STATUS_ERROR;
    }

    return HAL_USB_HOST_STATUS_OK;
}

#ifdef USB_HOST_GPIO_DBG
void GPIO_Debug(void)
{
    hal_gpio_init(HAL_GPIO_27);
    hal_pinmux_set_function(HAL_GPIO_27, 0);/* Set pin as GPIO mode.*/
    hal_gpio_set_direction(HAL_GPIO_27, HAL_GPIO_DIRECTION_OUTPUT); /* Set GPIO as output.*/
    hal_gpio_set_output(HAL_GPIO_27, HAL_GPIO_DATA_LOW); /*set gpio output low*/
}
#endif /* USB_HOST_GPIO_DBG */

void hal_usb_host_turn_on_Vcore_(void)
{
    // *(volatile uint32_t*)0x4207000C = 0x17B067F;
    // *(volatile uint32_t*)0x420302A0 = 0;
#if defined(DVFS_MODULE_ENABLED)
    hal_dvfs_lock_control(HAL_DVFS_OPP_HIGH, HAL_DVFS_UNLOCK);
    hal_dvfs_lock_control(HAL_DVFS_OPP_HIGH, HAL_DVFS_LOCK);
#endif
    clock_mux_sel(CLK_USB_SEL, 3); // 62.4MHz, UPLL

    hal_clock_enable(HAL_CLOCK_CG_USB_BUS);
    hal_clock_enable(HAL_CLOCK_CG_USB_DMA);
    hal_clock_enable(HAL_CLOCK_CG_USB);
    pmu_enable_usb_power(PMU_ON);
}

hal_usb_host_status_t hal_usb_host_init(void)
{
    hal_usb_host_turn_on_Vcore_();
#ifdef USB_HOST_GPIO_DBG
    GPIO_Debug();
#endif /* USB_HOST_GPIO_DBG */


    hal_usbphy_poweron_initialize();

    // USB PHY switch to host
    USB_DRV_ClearBits32(U2PHYDTM1, U2PHYDTM1_RG_IDDIG);

    hal_usb_phy_setting_print();

    // Start a session
    log_hal_msgid_info("USB Host Start a session !!!", 0);
    USB_DRV_SetBits(&musb->devctl, USB_DEVCTL_SESSION);

    //Check VBUS voltage reach above 4.75V
    while ((USB_DRV_Reg8(&musb->devctl) & USB_DEVCTL_VBUS) != USB_DEVCTL_VBUS);

#ifdef USB_HOST_GPIO_DBG
    hal_gpio_set_output(HAL_GPIO_27, HAL_GPIO_DATA_HIGH);
#endif /* USB_HOST_GPIO_DBG */
    while ((USB_DRV_Reg8(&musb->devctl) & USB_DEVCTL_BDEVICE));
#ifdef USB_HOST_GPIO_DBG
    hal_gpio_set_output(HAL_GPIO_27, HAL_GPIO_DATA_LOW);
#endif /* USB_HOST_GPIO_DBG */

    USB_DRV_SetBits8(&musb->power, USB_POWER_HSENAB);

    hal_usb_host_interrupt_init();
    Control_Pipe_Clean_Fifo();
    hal_usb_host_control_pipe_tx_rx_interrupt_enable();
    Enable_L1_Int();

    log_hal_msgid_info("USB Interrupt Enable addr:%x, value:%x\r\n", 2, &musb->intrusbe, (USB_DRV_Reg8(&musb->intrusbe)));


    //hal_nvic_set_pending_irq(USB_IRQn);
#if 0

    while (device_attach == 0);

    //Get Device Descriptor
    uint8_t setup_get_device[] = {
        0x80, 0x06, 0x00, 0x01, 0x00, 0x00, 0x40, 0x00
    };
    hal_usb_host_issue_setup_command(setup_get_device, 8);
    while (ep0_tx_done == 0);
    hal_usb_host_ep_rx_request(0);
    USB_DRV_WriteReg8(&musb->index, 0);
    hal_gpt_delay_ms(3);
    while ((USB_DRV_Reg(&musb->csr0) & USB_RXCSR_RXPKTRDY) == 0);
    ep0_rx_len = hal_usb_host_get_ep0_pkt_len();
    log_hal_msgid_info("ep0_rx_len: %d\r\n", 1, ep0_rx_len);
    hal_usb_host_read_endpoint_fifo(0, ep0_rx_len, rx_buff);
    log_hal_msgid_info("Rx data %x %x %x %x", 4, rx_buff[0], rx_buff[1], rx_buff[2], rx_buff[3]);
    hal_usb_host_clear_ep0_rx_ready();
    ep0_tx_done = 0 ;
    hal_usb_host_ep0_out_status();
    while (ep0_tx_done == 0);
    log_hal_msgid_info("Get Device Descriptor Done !!!", 0);

    //Set Address
    log_hal_msgid_info("USB Host Set Address !!!", 0);
    ep0_tx_done = 0 ;
    uint8_t setup_set_address[] = {
        0x00, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    hal_usb_host_issue_setup_command(setup_set_address, 8);

    while (ep0_tx_done == 0);
    log_hal_msgid_info("USB Host Status In !!!", 0);
    ep0_tx_done = 0 ;
    hal_usb_host_ep0_in_status();
    while (ep0_tx_done == 0);

    reg = (volatile uint32_t *)0x41020620 ;
    for (i = 0 ; i < 6 ; i++) {
        log_hal_msgid_info("reg %x = %x\r\n", 2, reg, *reg);
        reg++ ;
    }

    while (1);

#endif

    return HAL_USB_HOST_STATUS_OK;
}


#endif /* HAL_USB_HOST_MODULE_ENABLED */


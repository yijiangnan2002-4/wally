/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

/*****************************************************************************
 *
 * Filename:
 * ---------
 *    usbacm_adap.c
 *
 * Project:
 * --------
 *   Maui_Software
 *
 * Description:
 * ------------
 *   This file implements usb adaption layer for UART API
 *
 * Author:
 * -------
 *  Jensen Hu
 *
 *****************************************************************************/
#ifdef AIR_USB_CDC_ENABLE

#define __USB_COM_PORT_ENABLE__
#ifdef __USB_COM_PORT_ENABLE__

/* C library */
#include <string.h>

/* USB Middleware includes */
#include "bmd.h"
#include "usb.h"
#include "usb_custom.h"
#include "usb_custom_def.h"
#include "usbacm_adap.h"
#include "usbacm_drv.h"

/* Hal includes */
#include "hal.h"
#include "hal_gpt.h"

/* Syslog includes */
#include "usb_dbg.h"

#ifdef MTK_PORT_SERVICE_ENABLE
#include "serial_port.h"
#include "serial_port_uart.h"
#endif /* MTK_PORT_SERVICE_ENABLE */

/* Exception flag */
extern uint8_t INT_Exception_Enter;


/* functions*/
void USB2UART_ClrRxBuffer(uint8_t port);
uint16_t USB2UART_GetRxAvail(uint8_t port);
uint16_t USB2UART_GetBufAvail(BUFFER_INFO *buf_info);
uint16_t USB2UART_GetBytes(uint8_t port, uint8_t *buffaddr, uint16_t length);
void USB2UART_ClrTxBuffer(uint8_t port);
uint16_t USB2UART_GetTxRoomLeft(uint8_t port);
uint16_t USB2UART_PutBytes(uint8_t port, uint8_t *buffaddr, uint16_t length);
uint16_t USB2UART_Thru_PutBytes(uint8_t port, uint8_t *buffaddr, uint16_t length);
void USB2UART_PutData_Polling(uint8_t port, uint8_t *Buffaddr, uint16_t Length);
extern void USB2UART_Tx_DMA_Callback(uint8_t port);
extern void USB2UART_Send_Intr_Pending(uint32_t usb_port);
void USB2UART_Register_RxCallback(uint8_t port, UART_RX_FUNC func);
void USB2UART_Register_TxCallback(uint8_t port, UART_TX_FUNC func);
void USB2UART_Register_RxBuffer(uint8_t port, void* buffer, uint16_t size);
void USB2UART_Register_TxBuffer(uint8_t port, void* buffer, uint16_t size);
void USB2UART_port_init(uint8_t port);

void USB2UART_init(void);

static bool USB2UART_Check_Config(uint32_t usb_port)
{
    if ((g_UsbACM[usb_port].txpipe == NULL) || (USB2UARTPort[usb_port].initialized == false) || (USB_Get_Device_State() != DEVSTATE_CONFIG)) {
        LOG_MSGID_E(common, "USB2UART_Check_Config, txpipe=0x%x, init=%d, state=%d\n", 3, g_UsbACM[usb_port].txpipe, USB2UARTPort[usb_port].initialized, USB_Get_Device_State());
        return false;
    } else {
        return true;
    }
}


/* Initialize USB2UART setting, called when driver initialize, no matter user select as UART or not */
void USB2UART_init(void)
{
    uint32_t usb_port;

#ifdef MTK_PORT_SERVICE_ENABLE
    /* Initialize USB port value */
    USB_PORT[SERIAL_PORT_DEV_USB_COM1] = 0;

    /* Initialize USB port value */
    USB_PORT[SERIAL_PORT_DEV_USB_COM2] = 1;
#else
    USB_PORT[0] = 0;
    USB_PORT[1] = 1;
#endif


    /* Initialize USB port value */
    for (usb_port = 0; usb_port < MAX_USB_PORT_NUM; usb_port++) {

        g_UsbACM[usb_port].send_Txilm = false;
        g_UsbACM[usb_port].send_Rxilm = true;

        g_UsbACM[usb_port].config_send_Txilm = false;
        USB2UARTPort[usb_port].tx_cb = (UART_TX_FUNC)USB2UART_Dafault_Tx_Callback;
        USB2UARTPort[usb_port].rx_cb = (UART_RX_FUNC)USB2UART_Dafault_Rx_Callback;
    }

}

/* Clear Tx Ring buffer */
void USB2UART_Clear_Tx_Buffer(uint8_t port)
{
    USB2UART_ClrTxBuffer(port);
}

/* Clear RX buffer */
void USB2UART_Clear_Rx_Buffer(uint8_t port)
{
    USB2UART_ClrRxBuffer(port);
}


/************************************************************
    UART driver  functions
*************************************************************/

/* Clear RX buffer */
void USB2UART_ClrRxBuffer(uint8_t port)
{
    uint32_t usb_port = USB_PORT[port];

    NVIC_DisableIRQ(USB_IRQn);

    /* Clear ring buffer */
    USB2UARTPort[usb_port].Rx_Buffer.Write = 0;
    USB2UARTPort[usb_port].Rx_Buffer.Read = 0;
    g_UsbACM[usb_port].send_Rxilm = true;

    /* Clear hardware FIFO if current status is CDC ACM */
    if (USB2UART_Check_Config(usb_port) == true) {
        /* Clear RX FIFO */
        USB_Acm_Rx_ClrFifo(port);
        /* Clear interrupt mask variable */
        /* Open intr */
//      USB_Set_UnMask_Irq(true);
        USB_UnMask_COM_Intr(port);

    }

    NVIC_EnableIRQ(USB_IRQn);
}


/* Get available bytes in RX buffer */
uint16_t USB2UART_GetRxAvail(uint8_t port)
{
    uint32_t usb_port = USB_PORT[port];
    uint16_t real_count;
    uint32_t savedMask;


    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    Buf_GetBytesAvail(&(USB2UARTPort[usb_port].Rx_Buffer), real_count);
    hal_nvic_restore_interrupt_mask(savedMask);
    return real_count;
}

/* Get available bytes in RX buffer */
uint16_t USB2UART_GetBufAvail(BUFFER_INFO *buf_info)
{
//  uint32_t usb_port = USB_PORT[port];
    uint16_t real_count;

    Buf_GetBytesAvail(buf_info, real_count);

    return real_count;
}

/* Get bytes from RX buffer, parameter status shows escape and break status
     return value is the actually get bytes */
uint16_t USB2UART_GetBytes(uint8_t port, uint8_t *buffaddr, uint16_t length)
{
    uint32_t usb_port = USB_PORT[port];
    uint16_t real_count;
    uint16_t RoomLeft;
    uint32_t savedMask;
    int32_t remain;
    BUFFER_INFO *rx_buf_info = &(USB2UARTPort[usb_port].Rx_Buffer);

    /* Return directly if not match conditions */
    if (USB2UART_Check_Config(usb_port) == false) {
        LOG_MSGID_E(common, "USB2UART_GetBytes, enumeration not ready", 0);
        return 0;
    }

    /* Determine real data count */
    /* Note that the area to determine send_Rxilm must also contain in critical section.
       Otherwise if USB HISR activated before send_Rxilm setting as true,
       this message will be lost */
    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    real_count = USB2UART_GetBufAvail(rx_buf_info);

    //LOG_I(common, "USB2UART_GetBytes, real_count:%d, length:%d", real_count, length);

    if (real_count >= length) {
        real_count = length;
    } else {
        g_UsbACM[usb_port].send_Rxilm = true;  /*After this time get byte, buffer will be empty */
        //LOG_I(common, "USB2UART_GetBytes, set send_Rxilm\n");
    }

    hal_nvic_restore_interrupt_mask(savedMask);

    if (real_count != 0) {
        remain = (BRead(rx_buf_info) + real_count) - BLength(rx_buf_info);

        if (remain < 0) {
            /* dest, src, len */
            memcpy(buffaddr, BuffRead(rx_buf_info), real_count);
            BRead(rx_buf_info) += real_count;
        } else {
            memcpy(buffaddr, BuffRead(rx_buf_info), real_count - remain);
            memcpy((uint8_t *)(buffaddr + real_count - remain), BStartAddr(rx_buf_info), remain);
            BRead(rx_buf_info) = remain;
        }
    }

    NVIC_DisableIRQ(USB_IRQn);

    RoomLeft = USB2UART_GetBuffRoomLeft(rx_buf_info);

    if (RoomLeft >= HAL_USB_MAX_PACKET_SIZE_ENDPOINT_BULK_FULL_SPEED) {
        //USB_UnMask_COM_Intr(port);
        NVIC_EnableIRQ(USB_IRQn);
    }

    //LOG_HEXDUMP_D(common, "USB2UART_BUFF dump\n", buffaddr, real_count);

    return real_count;
}


/* Clear TX buffer */
void USB2UART_ClrTxBuffer(uint8_t port)
{
    uint32_t usb_port = USB_PORT[port];
    uint32_t savedMask;

    /* Stop DMA channel if current status is CDC ACM */
    if (USB2UART_Check_Config(usb_port) == true) {
        hal_usb_stop_dma_channel(g_UsbACM[usb_port].txpipe->byEP, HAL_USB_EP_DIRECTION_TX);
        g_UsbACM[usb_port].setup_dma = false;
        if (g_UsbACM[usb_port].put_start == true) {
            g_UsbACM[usb_port].dmaf_setmember |= 0x20;
        }
    }
    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB2UARTPort[usb_port].Tx_Buffer.Write = 0;
    USB2UARTPort[usb_port].Tx_Buffer.Read = 0;

    {
        g_UsbACM[usb_port].send_Txilm = true;
    }

    hal_nvic_restore_interrupt_mask(savedMask);

}


/*Get the left bytes for buffer */
uint16_t USB2UART_GetBuffRoomLeft(BUFFER_INFO *buf_info)
{
    uint16_t real_count;
    Buf_GetRoomLeft(buf_info, real_count);
    return real_count;
}
/*Get the left bytes for TX buffer */
uint16_t USB2UART_GetTxRoomLeft(uint8_t port)
{
    uint32_t usb_port = USB_PORT[port];
    uint16_t real_count;
    uint32_t  savedMask;


    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    real_count = USB2UART_GetBuffRoomLeft(&(USB2UARTPort[usb_port].Tx_Buffer));
    hal_nvic_restore_interrupt_mask(savedMask);
    return real_count;
}

/* Put bytes to tx buffer, return value is the actually put out bytes */
uint16_t USB2UART_Thru_PutBytes(uint8_t port, uint8_t *buffaddr, uint16_t length)
{
    uint32_t usb_port = USB_PORT[port];

#if 0
    uint32_t  savedMask;

    //hal_nvic_save_and_set_interrupt_mask(&savedMask);

    if (g_UsbACM[usb_port].dma_txcb_just_done == true) {

        dbg_print("USB2UART_Thru_PutBytes reentry", 0, usb_port);
        //return length;

    }
    g_UsbACM[usb_port].dma_txcb_just_done = true;
    //hal_nvic_restore_interrupt_mask(savedMask);
#endif


    /* Return directly if not match condition */
    if (USB2UART_Check_Config(usb_port) == false) {
        hal_gpt_delay_ms(500);
        if ((USB2UARTPort[usb_port].initialized == true) && (USB_Get_Device_State() != DEVSTATE_CONFIG)) {
            g_UsbACM[usb_port].config_send_Txilm = true;  /* for PC set config later then can issue the first message */
            //LOG_I(common, "USB2UART_PutBytes, enumeration not ready");
        } else {
            //LOG_I(common, "USB2UART_PutBytes, a-connection is back, enumeration ready");
        }
    }

    USB2UART_Thru_DMATransmit(port, buffaddr, length);

    g_UsbACM[usb_port].put_start = false;
    g_UsbACM[usb_port].dmaf_setmember = 0;

    return length;
}

/* Put bytes to tx buffer, return value is the actually put out bytes */
uint16_t USB2UART_PutBytes(uint8_t port, uint8_t *buffaddr, uint16_t length)
{
    uint32_t usb_port = USB_PORT[port];
    uint16_t  real_count;
    uint32_t  savedMask;
    bool  setup_dma = false;
    int32_t     remain;
    BUFFER_INFO     *tx_info = &(USB2UARTPort[usb_port].Tx_Buffer);

    /* Return directly if not match condition */
    if (USB2UART_Check_Config(usb_port) == false) {
        hal_gpt_delay_ms(500);
        if ((USB2UARTPort[usb_port].initialized == true) && (USB_Get_Device_State() != DEVSTATE_CONFIG)) {
            g_UsbACM[usb_port].config_send_Txilm = true;  /* for PC set config later then can issue the first message */
            //LOG_I(common, "USB2UART_PutBytes, enumeration not ready");
        } else {
            //LOG_I(common, "USB2UART_PutBytes, a-connection is back, enumeration ready");
        }
    }

    /* The same issue as USB2UART_GetBytes()
       The area to determine send_Txilm must also contain in critical section.
       Otherwise if DMA callback activated before send_Txilm setting as true,
       this message will be lost */
    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    real_count = USB2UART_GetBuffRoomLeft(tx_info);
    if ((real_count == 0) && (g_UsbACM[usb_port].dma_txcb_just_done == true)) {
        LOG_MSGID_I(common, "ASSERT", 0);
    }

    g_UsbACM[usb_port].dma_txcb_just_done = false;

    /* determine real sent data count */
    if (real_count > length) {
        real_count = length;
    } else {
        g_UsbACM[usb_port].send_Txilm = true;  /*After this time put bytes, buffer will be full */
        g_UsbACM[usb_port].config_send_Txilm = true; /* if be reseted, then it can issue the message waited for*/
    }
    hal_nvic_restore_interrupt_mask(savedMask);

    if (real_count != 0) {
        remain = (BWrite(tx_info) + real_count) - BLength(tx_info);

        if (remain < 0) {
            /* dest, src, len */
            memcpy(BuffWrite(tx_info), buffaddr, real_count);
            BWrite(tx_info) += real_count;
        } else {
            memcpy(BuffWrite(tx_info), buffaddr, real_count - remain);
            memcpy(BStartAddr(tx_info), (uint8_t *)(buffaddr + real_count - remain), remain);
            BWrite(tx_info) = remain;
        }
    }

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    /* In case USB is plugged out just before this critical section */
    if (g_UsbACM[usb_port].txpipe != NULL) {
        if (g_UsbACM[usb_port].setup_dma == false) {
            g_UsbACM[usb_port].setup_dma = true;
            setup_dma = true;
            g_UsbACM[usb_port].put_start = true;
            g_UsbACM[usb_port].dmaf_setmember = 0;
        }
    }
    hal_nvic_restore_interrupt_mask(savedMask);

    if (setup_dma == true) {
        if (USB2UART_Check_Config(usb_port) == false) {
            hal_gpt_delay_ms(200);
            if ((USB2UARTPort[usb_port].initialized == true) && (USB_Get_Device_State() != DEVSTATE_CONFIG)) {
                LOG_MSGID_E(common, "USB2UART_PutBytes, enumeration not ready, stop dma transfer then return error\n", 0);
                return 0;
            }
            //LOG_I(common, "USB2UART_PutBytes, b-connection is back, enumeration ready");
        }
        USB2UART_DMATransmit(port);
    }
    g_UsbACM[usb_port].put_start = false;
    g_UsbACM[usb_port].dmaf_setmember = 0;

    return real_count;
}

/* This function is only used for retrive exception log*/
void USB2UART_PutData_Polling(uint8_t port, uint8_t *Buffaddr, uint16_t Length)
{
    uint32_t usb_port = USB_PORT[port];
    uint16_t toSend = Length;

    while (toSend > 0) {
        if (toSend > 64) {
            Length = 64;
        } else {
            Length = toSend;
        }

        hal_usb_set_endpoint_tx_ready(g_UsbACM[usb_port].txpipe->byEP);
        while (!hal_usb_is_endpoint_tx_empty(g_UsbACM[usb_port].txpipe->byEP));
        hal_usb_write_endpoint_fifo(g_UsbACM[usb_port].txpipe->byEP, Length, Buffaddr);
        hal_usb_set_endpoint_tx_ready(g_UsbACM[usb_port].txpipe->byEP);
        while (!hal_usb_is_endpoint_tx_empty(g_UsbACM[usb_port].txpipe->byEP));

        toSend = toSend - Length;
        Buffaddr = Buffaddr + Length;
    }

    /* TX complete callback */
    USB2UARTPort[usb_port].tx_cb(port);
}

void USB2UART_Register_RxCallback(uint8_t port, UART_RX_FUNC func)
{
    USB2UARTPort[port].tx_cb = func;
}

void USB2UART_Register_TxCallback(uint8_t port, UART_TX_FUNC func)
{
    USB2UARTPort[port].rx_cb = func;
}

void USB2UART_Register_RxBuffer(uint8_t port, void* buffer, uint16_t size)
{
    USB2UARTPort[port].RingBuffers.rx_buffer = buffer;
    Buf_init(&(USB2UARTPort[port].Rx_Buffer), (uint8_t *)(USB2UARTPort[port].RingBuffers.rx_buffer), size);
}

void USB2UART_Register_TxBuffer(uint8_t port, void* buffer, uint16_t size)
{
    USB2UARTPort[port].RingBuffers.tx_buffer = buffer;
    Buf_init(&(USB2UARTPort[port].Tx_Buffer), (uint8_t *)(USB2UARTPort[port].RingBuffers.tx_buffer), size);
}

void USB2UART_port_init(uint8_t port)
{
    USB2UARTPort[port].initialized = true;

    g_UsbACM[port].send_Txilm = false;
    g_UsbACM[port].send_Rxilm = true;
    g_UsbACM[port].transfer_type = CDC_NORMAL;
    g_UsbACM[port].config_send_Txilm = false;

    if (USB_Get_Device_State() != DEVSTATE_CONFIG) {
        return;
    }

    if (g_UsbACM[port].txpipe != NULL) {
        /* Only RX EP needs to be enabled since TX EP will use DMA polling */
        hal_usb_enable_tx_endpoint(g_UsbACM[port].txpipe->byEP, HAL_USB_EP_TRANSFER_BULK, HAL_USB_EP_USE_ONLY_DMA, false);
        hal_usb_enable_rx_endpoint(g_UsbACM[port].rxpipe->byEP, HAL_USB_EP_TRANSFER_BULK, HAL_USB_EP_USE_NO_DMA, false);

        /* Open intr */
        USB_UnMask_COM_Intr(port);
    }
}

#endif /*__USB_COM_PORT_ENABLE__*/

#endif /* AIR_USB_CDC_ENABLE */

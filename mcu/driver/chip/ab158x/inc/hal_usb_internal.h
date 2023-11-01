/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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

#ifndef __HAL_USB_INTERNAL_H__
#define __HAL_USB_INTERNAL_H__

/* USB Common define item */
#define    DRV_WriteReg(addr,data)         ((*(volatile uint16_t *)(addr)) = (uint16_t)(data))
#define    DRV_Reg(addr)                   (*(volatile uint16_t *)(addr))
#define    DRV_WriteReg32(addr,data)       ((*(volatile uint32_t *)(addr)) = (uint32_t)(data))
#define    DRV_Reg32(addr)                 (*(volatile uint32_t *)(addr))
#define    DRV_WriteReg8(addr,data)        ((*(volatile uint8_t *)(addr)) = (uint8_t)(data))
#define    DRV_Reg8(addr)                  (*(volatile uint8_t *)(addr))
#define    USB_DRV_WriteReg(addr,data)     ((*(volatile uint16_t *)(addr)) = (uint16_t)(data))
#define    USB_DRV_Reg(addr)               (*(volatile uint16_t *)(addr))
#define    USB_DRV_WriteReg32(addr,data)   ((*(volatile uint32_t *)(addr)) = (uint32_t)(data))
#define    USB_DRV_Reg32(addr)             (*(volatile uint32_t *)(addr))
#define    USB_DRV_WriteReg8(addr,data)    ((*(volatile uint8_t *)(addr)) = (uint8_t)(data))
#define    USB_DRV_Reg8(addr)              (*(volatile uint8_t *)(addr))

#define USB_DRV_ClearBits(addr,data)     {\
   uint16_t temp;\
   temp = DRV_Reg(addr);\
   temp &=~(data);\
   DRV_WriteReg(addr,temp);\
}

#define USB_DRV_SetBits(addr,data)     {\
   uint16_t temp;\
   temp = DRV_Reg(addr);\
   temp |= (data);\
   DRV_WriteReg(addr,temp);\
}

#define USB_DRV_SetData(addr, bitmask, value)     {\
   uint16_t temp;\
   temp = (~(bitmask)) & DRV_Reg(addr);\
   temp |= ((value) & (bitmask));\
   DRV_WriteReg(addr,temp);\
}

#define USB_DRV_ClearBits32(addr,data)     {\
   uint32_t temp;\
   temp = DRV_Reg32(addr);\
   temp &=~(data);\
   DRV_WriteReg32(addr,temp);\
}

#define USB_DRV_SetBits32(addr,data)     {\
   uint32_t temp;\
   temp = DRV_Reg32(addr);\
   temp |= (data);\
   DRV_WriteReg32(addr,temp);\
}

#define USB_DRV_SetData32(addr, bitmask, value)     {\
   uint32_t temp;\
   temp = (~(bitmask)) & DRV_Reg32(addr);\
   temp |= ((value) & (bitmask));\
   DRV_WriteReg32(addr,temp);\
}

#define USB_DRV_ClearBits8(addr,data)     {\
   uint8_t temp;\
   temp = DRV_Reg8(addr);\
   temp &=~(data);\
   DRV_WriteReg8(addr,temp);\
}

#define USB_DRV_SetBits8(addr,data)     {\
   uint8_t temp;\
   temp = DRV_Reg8(addr);\
   temp |= (data);\
   DRV_WriteReg8(addr,temp);\
}

#define USB_DRV_SetData8(addr, bitmask, value)     {\
   uint8_t temp;\
   temp = (~(bitmask)) & DRV_Reg8(addr);\
   temp |= ((value) & (bitmask));\
   DRV_WriteReg8(addr,temp);\
}

#if defined (HAL_USB_MODULE_ENABLED) || defined (HAL_USB_HOST_MODULE_ENABLED)
#include "hal_usb.h"

/* USB_POWER */
#define    USB_POWER_ENABLESUSPENDM  (0x01)    /* RW */
#define    USB_POWER_SUSPENDMODE     (0x02)    /* P: RO ; H: WO */ /*Read clear by the intr. register*/
#define    USB_POWER_RESUME          (0x04)    /* RW */
#define    USB_POWER_RESET           (0x08)    /* P: RO ; H: RW */
#define    USB_POWER_HSMODE          (0x10)    /* RO */
#define    USB_POWER_HSENAB          (0x20)    /* RW */
#define    USB_POWER_SOFTCONN        (0x40)    /* RW */
#define    USB_POWER_ISOUPDATE       (0x80)    /* RW */

/* USB_INTRTX */
#define    USB_INTRTX_EP0            (0x01)    /* RO */
#define    USB_INTRTX_EP1            (0x02)    /* RO */
#define    USB_INTRTX_EP2            (0x04)    /* RO */
#define    USB_INTRTX_EP3            (0x08)    /* RO */
#define    USB_INTRTX_EP4            (0x10)    /* RO */

/* USB_INTRRX */
#define    USB_INTRRX_EP1            (0x02)    /* RO */
#define    USB_INTRRX_EP2            (0x04)    /* RO */
#define    USB_INTRRX_EP3            (0x08)    /* RO */

/* USB_INTRTXE & USB_INTRRXE */
#define    USB_INTRE_EPEN            (0x01)    /* RW */
#define    USB_INTRE_EPDIS           (0x00)    /* RW */

/* USB_INTRUSB */
#define    USB_INTRUSB_SUSPEND       (0x01)    /* RO */
#define    USB_INTRUSB_RESUME        (0x02)    /* RO */
#define    USB_INTRUSB_RESET         (0x04)    /* RO */
#define    USB_INTRUSB_BABBLE        (0x04)    /* RO */
#define    USB_INTRUSB_SOF           (0x08)    /* RO */
#define    USB_INTRUSB_CONN          (0x10)    /* RO */
#define    USB_INTRUSB_DISCON        (0x20)    /* RO */
#define    USB_INTRUSB_SESSREQ       (0x40)    /* RO */
#define    USB_INTRUSB_VBUSERROR     (0x80)    /* RO */

/* USB_INTRUSBE */
#define    USB_INTRUSBE_SUSPEND      (0x01)    /* RW */
#define    USB_INTRUSBE_RESUME       (0x02)    /* RW */
#define    USB_INTRUSBE_RESET        (0x04)    /* RW */
#define    USB_INTRUSBE_BABBLE       (0x04)    /* RW */
#define    USB_INTRUSBE_SOF          (0x08)    /* RW */
#define    USB_INTRUSBE_CONN         (0x10)    /* RW */
#define    USB_INTRUSBE_DISCON       (0x20)    /* RW */
#define    USB_INTRUSBE_SESSREQ      (0x40)    /* RW */
#define    USB_INTRUSBE_VBUSERROR    (0x80)    /* RW */

/* USB_TESTMODE */
#define    USB_TESTMODE_TESTSE0NAK   (0x01)    /* RW */
#define    USB_TESTMODE_TESTJ        (0x02)    /* RW */
#define    USB_TESTMODE_TESTK        (0x04)    /* RW */
#define    USB_TESTMODE_TESTPACKET   (0x08)    /* RW */
#define    USB_TESTMODE_FORCEHS      (0x10)    /* RW */
#define    USB_TESTMODE_FORCEFS      (0x20)    /* RW */
#define    USB_TESTMODE_FIFOACCESS   (0x40)    /* WO, AC */
#define    USB_TESTMODE_FORCEHOST    (0x80)    /* RW */

/* USB_DEVCTL */
#define    USB_DEVCTL_SESSION        (0x01)    /* RW */
#define    USB_DEVCTL_HOSTREQ        (0x02)    /* RW */
#define    USB_DEVCTL_HOSTMODE       (0x04)    /* RO */
#define    USB_DEVCTL_VBUS           (0x18)    /* RO */
#define    USB_DEVCTL_LSDEV          (0x20)    /* RO */
#define    USB_DEVCTL_FSDEV          (0x40)    /* RO */
#define    USB_DEVCTL_BDEVICE        (0x80)    /* RO */

#define    USB_DEVCTL_ABOVE_VBUS_VALID          (0x18)
#define    USB_DEVCTL_ABOVE_A_VALID             (0x10)
#define    USB_DEVCTL_ABOVE_SESSION_END         (0x01)
#define    USB_DEVCTL_BELOW_SESSION_END         (0x00)

/* USB_CSR0 */
#define    USB_CSR0_RXPKTRDY                  (0x0001)   /* RO */
#define    USB_CSR0_TXPKTRDY                  (0x0002)   /* RW, AC */
#define    USB_CSR0_SENTSTALL                 (0x0004)   /* RC */
#define    USB_CSR0_DATAEND                   (0x0008)   /* WO, AC */
#define    USB_CSR0_SETUPEND                  (0x0010)   /* RO */
#define    USB_CSR0_SENDSTALL                 (0x0020)   /* WO, AC */
#define    USB_CSR0_SERVICEDRXPKTRDY          (0x0040)   /* WO, AC */
#define    USB_CSR0_SERVICEDSETUPEND          (0x0080)   /* WO, AC */
#define    USB_CSR0_FLUSHFIFO                 (0x0100)   /* WO, AC */

/* in Host mode */
#define    USB_CSR0_RXSTALL                   (0x0004)
#define    USB_CSR0_SETUPPKT                  (0x0008)
#define    USB_CSR0_ERROR                     (0x0010)
#define    USB_CSR0_REQPKT                    (0x0020)
#define    USB_CSR0_STATUSPKT                 (0x0040)
#define    USB_CSR0_NAKTIMEOUT                (0x0080)
#define    USB_CSR0_FLUSHFIFO                 (0x0100)
#define    USB_CSR0_DISPING                   (0x0800)

/* USB_TXMAXP */
#define    USB_TXMAXP_MAXPAYLOAD_MASK         (0x07FF)
#define    USB_TXMAXP_HIGHSPEED_MASK          (0xF800)

/* USB_TXCSR */
#define    USB_TXCSR_TXPKTRDY                 (0x0001)    /* RW */
#define    USB_TXCSR_FIFONOTEMPTY             (0x0002)    /* RO */
#define    USB_TXCSR_UNDERRUN                 (0x0004)    /* RW */
#define    USB_TXCSR_FLUSHFIFO                (0x0008)    /* WO */
#define    USB_TXCSR_SENDSTALL                (0x0010)    /* RW */
#define    USB_TXCSR_SENTSTALL                (0x0020)    /* RC */
#define    USB_TXCSR_CLRDATATOG               (0x0040)    /* WO */
#define    USB_TXCSR_INCOMPTX                 (0x0080)    /* RC */
#define    USB_TXCSR_SETTXPKTRDY_TWICE        (0x0100)    /* RC */
#define    USB_TXCSR_DMAREQMODE               (0x0400)    /* RW */
#define    USB_TXCSR_FRCDATATOG               (0x0800)    /* RW */
#define    USB_TXCSR_DMAREQENAB               (0x1000)    /* RW */
#define    USB_TXCSR_ISO                      (0x4000)    /* RW */
#define    USB_TXCSR_AUTOSET                  (0x8000)    /* RW */
/* in Host mode */
#define    USB_TXCSR_ERROR                    (0x0004)
#define    USB_TXCSR_RXSTALL                  (0x0020)
#define    USB_TXCSR_NAKTIMEOUT               (0x0080)
#define    USB_TXCSR_MODE                     (0x2000)    /* RW */

/* USB_RXMAXP */
#define    USB_RXMAXP_MAXPAYLOAD_MASK         (0x07FF)
#define    USB_RXMAXP_HIGHSPEED_MASK          (0xF800)

/* USB_RXCSR */
#define    USB_RXCSR_RXPKTRDY                 (0x0001)    /* RC */
#define    USB_RXCSR_FIFOFULL                 (0x0002)    /* RO, AC */
#define    USB_RXCSR_OVERRUN                  (0x0004)    /* RC */
#define    USB_RXCSR_DATAERROR                (0x0008)    /* RO */
#define    USB_RXCSR_FLUSHFIFO                (0x0010)    /* WO, AC */
#define    USB_RXCSR_SENDSTALL                (0x0020)    /* RW */
#define    USB_RXCSR_SENTSTALL                (0x0040)    /* RC */
#define    USB_RXCSR_CLRDATATOG               (0x0080)    /* WO */
#define    USB_RXCSR_INCOMPRX                 (0x0100)    /* RC */
#define    USB_RXCSR_KEEPERRCTATUS            (0x0200)    /* RC */
#define    USB_RXCSR_DMAREQMODE               (0x0800)    /* RW */
#define    USB_RXCSR_DISNYET                  (0x1000)    /* RW */
#define    USB_RXCSR_PIDERROR                 (0x1000)    /* RO */
#define    USB_RXCSR_DMAREQENAB               (0x2000)    /* RW */
#define    USB_RXCSR_ISO                      (0x4000)    /* RW */
#define    USB_RXCSR_AUTOCLEAR                (0x8000)    /* RW */

/* in Host mode */
#define    USB_RXCSR_ERROR                    (0x0004)
#define    USB_RXCSR_NAKTIMEOUT               (0x0008)
#define    USB_RXCSR_REQPKT                   (0x0020)
#define    USB_RXCSR_RXSTALL                  (0x0040)
#define    USB_RXCSR_SETPEQPKT_TWICE          (0x0400)
#define    USB_RXCSR_AUTOREQ                  (0x4000)

/* USB_TXTYPE */
#define    USB_TXTYPE_EPNUM_MASK                (0x0f)
#define    USB_TXTYPE_ISO                       (0x10)
#define    USB_TXTYPE_BULK                      (0x20)
#define    USB_TXTYPE_INTR                      (0x30)
#define    USB_TXTYPE_PROTOCOL_MASK             (0x30)
#define    USB_TXTYPE_HIGH_SPEED                (0x40)
#define    USB_TXTYPE_FULL_SPEED                (0x80)

/* USB_RXTYPE */
#define    USB_RXTYPE_EPNUM_MASK                (0x0f)
#define    USB_RXTYPE_ISO                       (0x10)
#define    USB_RXTYPE_BULK                      (0x20)
#define    USB_RXTYPE_INTR                      (0x30)
#define    USB_RXTYPE_PROTOCOL_MASK             (0x30)
#define    USB_RXTYPE_HIGH_SPEED                (0x40)
#define    USB_RXTYPE_FULL_SPEED                (0x80)

/* USB_RXMAP */
#define    USB_RXMAP_MAX_PAYLOAD_MASK         (0x07FF)
#define    USB_RXMAP_M1_MASK                  (0x1800)
#define    USB_RXMAP_M1_POS                   (11)


/* USB_PWRUPCNT */
#define    USB_PWRUPCNT_MASK                    (0x0f)

/* USB_FIFOSZ */
#define    USB_FIFOSZ_SIZE_MASK                 (0x0F)
#define    USB_FIFOSZ_DPB                       (0x10)
#define    USB_FIFOSZ_SIZE_8                    (0x00)
#define    USB_FIFOSZ_SIZE_16                   (0x01)
#define    USB_FIFOSZ_SIZE_32                   (0x02)
#define    USB_FIFOSZ_SIZE_64                   (0x03)
#define    USB_FIFOSZ_SIZE_128                  (0x04)
#define    USB_FIFOSZ_SIZE_256                  (0x05)
#define    USB_FIFOSZ_SIZE_512                  (0x06)
#define    USB_FIFOSZ_SIZE_1024                 (0x07)
#define    USB_FIFOSZ_SIZE_2048                 (0x08)
#define    USB_FIFOSZ_SIZE_4096                 (0x09)

/* USB_FIFOADD */
#define    USB_FIFOADD_MASK                   (0x1FFF)

/* USB_RXFIFOADD */
#define    USB_RXFIFOADD_DATAERRINTREN        (0x8000)
#define    USB_RXFIFOADD_OVERRUNINTREN        (0x4000)

/* USB_FIFO2ADD */
#define    USB_FIFO2ADD_MASK                  (0x1FFF)
#define    USB_FIFO2ADD_EN                    (0x8000)

/* USB_BUSPERF3 */
#define    USB_BUSPERF3_DISUSBRESET           (0x0001)
#define    USB_BUSPERF3_SWRST                 (0x0002)

/* USB_RSTINFO */
#define    USB_RSTINFO_WTFSSE0                (0x00F0)
#define    USB_RSTINFO_WTCHRP                 (0x000F)

/* USB_L1INTS */
#define    USB_L1INTS_TX_INT_STATUS           (0x0001)
#define    USB_L1INTS_RX_INT_STATUS           (0x0002)
#define    USB_L1INTS_USBCOM_INT_STATUS       (0x0004)
#define    USB_L1INTS_DMA_INT_STATUS          (0x0008)
#define    USB_L1INTS_PSR_INT_STATUS          (0x0010)
#define    USB_L1INTS_QINT_STATUS             (0x0020)
#define    USB_L1INTS_QHIF_INT_STATUS         (0x0040)
#define    USB_L1INTS_DPDM_INT_STATUS         (0x0080)
#define    USB_L1INTS_VBUSVALID_INT_STATUS    (0x0100)
#define    USB_L1INTS_IDDIG_INT_STATUS        (0x0200)
#define    USB_L1INTS_DRVVBUS_INT_STATUS      (0x0400)
#define    USB_L1INTS_POWERDOWN_INT_STATUS    (0x0800)

/* USB_L1INTM */
#define    USB_L1INTM_TX_INT_UNMASK           (0x0001)
#define    USB_L1INTM_RX_INT_UNMASK           (0x0002)
#define    USB_L1INTM_USBCOM_INT_UNMASK       (0x0004)
#define    USB_L1INTM_DMA_INT_UNMASK          (0x0008)
#define    USB_L1INTM_PSR_INT_UNMASK          (0x0010)
#define    USB_L1INTM_QINT_UNMASK             (0x0020)
#define    USB_L1INTM_QHIF_INT_UNMASK         (0x0040)
#define    USB_L1INTM_DPDM_INT_UNMASK         (0x0080)
#define    USB_L1INTM_VBUSVALID_INT_UNMASK    (0x0100)
#define    USB_L1INTM_IDDIG_INT_UNMASK        (0x0200)
#define    USB_L1INTM_DRVVBUS_INT_UNMASK      (0x0400)
#define    USB_L1INTM_POWERDOWN_INT_UNMASK    (0x0800)

/* USB_L1INTP */
#define    USB_L1INTP_VBUSVALID_INT_POL       (0x0100)
#define    USB_L1INTP_IDDIG_INT_POL           (0x0200)
#define    USB_L1INTP_DRVVBUS_INT_POL         (0x0400)
#define    USB_L1INTP_POWERDOWN_INT_POL       (0x0800)

/* USB_L1INTC */
#define    USB_L1INTC_USB_INT_SYNC            (0x0001)

/* USB_DMACNTL */
#define    USB_DMACNTL_DMAEN                  (0x0001)
#define    USB_DMACNTL_DMADIR                 (0x0002)
#define    USB_DMACNTL_DMAMODE                (0x0004)
#define    USB_DMACNTL_INTEN                  (0x0008)
#define    USB_DMACNTL_EP_MASK                (0x00F0)
#define    USB_DMACNTL_BUSERR                 (0x0100)
#define    USB_DMACNTL_BURSTMODE_MASK         (0x0000)
#define    USB_DMACNTL_DMACHEN                (0x0800)
#define    USB_DMACNTL_DMACHEN                (0x0800)

/* USB_DMA BURST MODE */
#define    USB_BURST_MODE_0                   (0x0000)
#define    USB_BURST_MODE_1                   (0x0200)
#define    USB_BURST_MODE_2                   (0x0400)
#define    USB_BURST_MODE_3                   (0x0600)

/* USB_DMA_TIMER */
#define    USB_DMA_TIMER_ENTIMER              (0x0080)
#define    USB_DMA_TIMER_TIMEOUT_MASK         (0x007F)
#define    USB_DMA_TIMER_TIMEOUT_STATUS       (0x0100)    /* W1C */

/*USB_DMAQ_RQCSRn*/
#define    USB_DMACNTL_DMAEN_RXQ_ACTIVE       (0x8000)
#define    USB_DMACNTL_DMAEN_RXQ_STOP         (0x0004)
#define    USB_DMACNTL_DMAEN_RXQ_RESUME       (0x0002)
#define    USB_DMACNTL_DMAEN_RXQ_START        (0x0001)
#define    USB_DBG_PRB_OPSTATE_MASK       (0x0000003F)
#define    USB_DBG_PRB_LINESTATE_MASK     (0x000000C0)

//========================definition of global value===================//
#define    USB_MAX_FIFO_SIZE                     3264
#define    USB_FIFO_START_ADDRESS                  64
#define    USB_BULK_FIFO_UNIT_SIZE                512
#define    USB_ISO_TX_FIFO_UNIT_SIZE              512
#define    USB_ISO_RX_FIFO_UNIT_SIZE             1024
#define    USB_ISO_RX_HB_FIFO_UNIT_SIZE          2048
#define    USB_INTR_FIFO_UNIT_SIZE                 64

//==============================================
#define    USB_DMACNTL(_n)                (USB_BASE+0x1F4+(0x10*(_n)))    /* RW, _n = 1~6 */
#define    USB_DMAADDR(_n)                (USB_BASE+0x1F8+(0x10*(_n)))    /* RW, _n = 1~6 */
#define    USB_DMACNT(_n)                 (USB_BASE+0x1FC+(0x10*(_n)))    /* RW, _n = 1~6 */
#define    USB_DMA_REALCNT(_n)            (USB_BASE+0x270+(0x10*(_n)))    /* RO, _n = 1~6, 32bits  **/
#define    USB_DMA_TIMER(_n)              (USB_BASE+0x274+(0x10*(_n)))    /* RW, _n = 1~6, 16bits */

#define    USBDMA_CheckRunStat(_n)        (USB_DRV_Reg(USB_DMACNTL(_n))&USB_DMACNTL_DMAEN)
#define    USBDMA_CheckPPRunStat(_n)      (USB_DRV_Reg(USB_DMAPPCNTL(_n))&USB_DMACNTL_DMAEN)
#define    USBDMA_Stop(_n)                (USB_DRV_ClearBits(USB_DMACNTL(_n), USB_DMACNTL_DMAEN))

#define    USB_DMA_TIMER_TIMEOUT_TICK            0x7f
#define    USB_DIS_DMA_LIMITER                   0xff

//==============================================
#define    USB_DMA_TX_CSR                (USB_TXCSR_AUTOSET|USB_TXCSR_DMAREQENAB|USB_TXCSR_DMAREQMODE)
#define    USB_DMA_TX_CSR_SINGLE         (USB_TXCSR_DMAREQENAB)
#define    USB_DMA_TX_CSR_ISO            (USB_TXCSR_AUTOSET|USB_TXCSR_DMAREQENAB|USB_TXCSR_DMAREQMODE|USB_TXCSR_ISO)
#define    USB_DMA_RX_CSR                (USB_RXCSR_AUTOCLEAR|USB_RXCSR_DMAREQENAB)
#define    USB_DMA_RX_CSR_SINGLE         (USB_RXCSR_DMAREQENAB)
#define    USB_DMA_RX_CSR_ISO            (USB_RXCSR_AUTOCLEAR|USB_RXCSR_DMAREQENAB|USB_RXCSR_ISO)
#define    DRV_USB_HISR_ID                          0
typedef struct {
    bool        is_initialized;
    hal_usb_power_state_t                 power_state;
    uint32_t    is_unMaskUSB;             /*Tricky, used to control interrupt */
    uint32_t    is_ProUSBHISR;
    /* interrupt handler */
    hal_usb_driver_interrupt_handler_t    reset_hdlr;
    hal_usb_driver_interrupt_handler_t    suspend_hdlr;
    hal_usb_driver_interrupt_handler_t    resume_hdlr;
    hal_usb_driver_interrupt_handler_t    ep0_hdlr;
    hal_usb_driver_interrupt_handler_t    ep_tx_hdlr[HAL_USB_MAX_NUMBER_ENDPOINT_TX];
    hal_usb_driver_interrupt_handler_t    ep_rx_hdlr[HAL_USB_MAX_NUMBER_ENDPOINT_RX];
    /* Stall status */
    uint32_t    ep_tx_stall_status[HAL_USB_MAX_NUMBER_ENDPOINT_TX];
    uint32_t    ep_rx_stall_status[HAL_USB_MAX_NUMBER_ENDPOINT_RX];
    /* cable plug out : disconnect */
    uint32_t    usb_disconnect;
    /* Endpoint current status */
    //hal_usb_endpoint_state_t        ep_tx_enb_state[HAL_USB_MAX_NUMBER_ENDPOINT_TX];
    //hal_usb_endpoint_state_t        ep_rx_enb_state[HAL_USB_MAX_NUMBER_ENDPOINT_RX];
    uint32_t    ep_tx_enb_state[HAL_USB_MAX_NUMBER_ENDPOINT_TX];
    uint32_t    ep_rx_enb_state[HAL_USB_MAX_NUMBER_ENDPOINT_RX];
    /* FIFO flush interrupt */
    uint32_t    ep_tx_flush_intr[HAL_USB_MAX_NUMBER_ENDPOINT_TX];
    uint32_t    ep_rx_flush_intr[HAL_USB_MAX_NUMBER_ENDPOINT_RX];
    uint32_t    dma_burst_mode;
    /*HS FS flag*/
    bool        hs_mode_enable;                                      /* If true, HW use HS if it could. If false, HW always use FS */
    bool        hs_mode;                                             /* HW is currently running in HS(true) or FS(false) mode*/
    /* High-Bandwitdth mode */
    bool        hb_mode_enable;                                      /* High-Bandwitdth mode */
    uint8_t     hb_mode_multi_packet;                                /* M_1 Packet multiplier m */
    /*USB FIFO full and receive Suspend signal */
    uint32_t    suspend_with_FIFO_not_empty;
    uint32_t    suspend_count;
    /* DMA parameters */
    uint32_t    dma_port[2][HAL_USB_MAX_NUMBER_ENDPOINT];             /* record TX/RX ep's DMA port number */
    //hal_usb_endpoint_direction_t        dma_dir[HAL_USB_MAX_NUMBER_DMA];              /* record  DMA port's direction */
    uint32_t    dma_dir[HAL_USB_MAX_NUMBER_DMA];                      /* record  DMA port's direction */
    //usb_dmaq_CB            dmaq_callback[2][HAL_USB_MAX_NUMBER_DMA];                  /* take PP buffer */
    //usb_dmaq_CB            dmaq_empty_callback[2][HAL_USB_MAX_NUMBER_DMA];            /* take PP buffer */
    void       *privdata[2][HAL_USB_MAX_NUMBER_DMA];                  /* take PP buffer */
    uint32_t    dma_pktrdy[HAL_USB_MAX_NUMBER_DMA];
    uint32_t    dma_running[HAL_USB_MAX_NUMBER_DMA];
    uint32_t    dma_callback_upd_run[HAL_USB_MAX_NUMBER_DMA];         /* take PP buffer */
    hal_usb_dma_handler_t    dma_callback[HAL_USB_MAX_NUMBER_DMA];    /* take PP buffer */
    uint32_t    dma_tx_length[HAL_USB_MAX_NUMBER_DMA];
    uint32_t    is_bidirection_dma[HAL_USB_MAX_NUMBER_DMA];
    uint32_t    dma_channel;
    uint32_t    dma_tx_ep_num[HAL_USB_MAX_NUMBER_DMA];                /* record  DMA port's TX ep number */
    uint32_t    dma_rx_ep_num[HAL_USB_MAX_NUMBER_DMA];                /* record  DMA port's RX ep number */
    uint32_t    b_enable_dma_burst_auto_chge;                         /* For DVT use only, enable DMA auto changed burst mode */
} USB_Drv_Info;

typedef struct {
   /*0x00*/
   uint8_t  faddr;         /* FunctionAddressRegister(Devicemodeonly) */
   uint8_t  power;         /* PowerManagementRegister */
   uint16_t intrtx;        /* TxInterruptStatusRegister */
   uint16_t intrrx;        /* RxInterruptStatusRegister */
   uint16_t intrtxe;       /* TxInterruptEnableRegister */
   uint16_t intrrxe;       /* RxInterruptEnableRegister */
   uint8_t  intrusb;       /* CommonUSBInterruptRegister */
   uint8_t  intrusbe;      /* CommonUSBInterruptEnableRegister */
   uint16_t frame;         /* FrameNumberRegister */
   uint8_t  index;         /* EndpointSelectionIndexRegister */
   uint8_t  testmode;      /* TestModeEnableRegister */
   /*0x10*/
   uint16_t txmap;         /* TXMAPRegister */
   uint16_t txcsr;         /* TxCSRRegister */
   uint16_t rxmap;         /* RXMAPRegister */
   uint16_t rxcsr;         /* RXCSRRegister */
   uint16_t rxcount;       /* RxCountRegister */
   uint8_t  txtype;        /* TxTypeRegister */
   uint8_t  txinterval;    /* TxIntervalRegister */
   uint8_t  rxtype;        /* RxTypeRegister */
   uint8_t  rxinterval;    /* RxIntervalRegister */
   uint8_t  revered0[1];
   uint8_t  fifosize;      /* ConfiguredFIFOSizeRegister */
   /*0x20*/
   uint32_t fifo0;         /* USBEndpoint0FIFORegister */
   uint32_t fifo1;         /* USBEndpoint1FIFORegister */
   uint8_t  revered1[56];
   /*0x60*/
   uint8_t  devctl;        /* DeviceControlRegister */
   uint8_t  pwrupcnt;      /* PowerUpCounterRegister */
   uint8_t  txfifosz;      /* TxFIFOSizeRegister */
   uint8_t  rxfifosz;      /* RxFIFOSizeRegister */
   uint16_t txfifoadd;     /* TxFIFOAddressRegister */
   uint16_t rxfifoadd;     /* RxFIFOAddressRegister */
   uint8_t  revered2[4];
   uint16_t hwcaps;        /* HardwareCapabilityRegister */
   uint16_t hwsvers;       /* VersionRegister */
   /*0x70*/
   uint16_t busperf1;      /* USBBusPerformanceRegister1 */
   uint16_t busperf2;      /* USBBusPerformanceRegister2 */
   uint16_t busperf3;      /* USBBusPerformanceRegister3 */
   uint8_t  revered3[2];
   uint8_t  epinfo;        /* NumberofTxandRxRegister */
   uint8_t  raminfo;       /* WidthofRAMandNumberofDMAChannelRegister */
   uint8_t  linkinfo;      /* DelaytobeAppliedRegister */
   uint8_t  vplen;         /* VbusPulsingChargeRegister */
   uint8_t  hs_eof1;       /* TimeBufferAvailableonHSTransactionRegister */
   uint8_t  fs_eof1;       /* TimeBufferAvailableonFSTransactionRegister */
   uint8_t  ls_eof1;       /* TimeBufferAvailableonLSTransactionRegister */
   uint8_t  rst_info;      /* ResetInformationRegister */
   /*0x80*/
   uint16_t rxtog;         /* RxDataToggleSet/StatusRegister */
   uint16_t rxtogen;       /* RxDataToggleEnableRegister */
   uint16_t txtog;         /* TxDataToggleSet/StatusRegister */
   uint16_t txtogen;       /* TxDataToggleEnableRegister */
   uint8_t  revered4[24];
   /*0xA0*/
   uint32_t usb_l1ints;    /* USBLevel1InterruptStatusRegister */
   uint32_t usb_l1intm;    /* USBLevel1InterruptMaskRegister */
   uint32_t usb_l1intp;    /* USBLevel1InterruptPolarityRegister */
   uint32_t usb_l1intc;    /* USBLevel1InterruptControlRegister */
   uint8_t  revered5[80];
   /*0x102*/
   uint8_t  revered6[2];
   uint16_t csr0;          /* EP0ControlStatusRegister */
   uint8_t  revered7[4];
   uint16_t count0;        /* EP0ReceivedBytesRegister */
   uint8_t  type0;         /* EP0TypeRegister */
   uint8_t  naklimt0;      /* NAKLimitRegister */
   uint16_t sramconfigsize;/* SRAMSizeRegister */
   uint8_t  hbconfigdata;  /* HighBind-widthConfigurationRegister */
   uint8_t  configdata;    /* CoreConfigurationRegister */
   /*0x110*/
   uint16_t tx1map;        /* TX1MAPRegister */
   uint16_t tx1csr;        /* Tx1CSRRegister */
   uint16_t rx1map;        /* RX1MAPRegister */
   uint16_t rx1csr;        /* RX1CSRRegister */
   uint16_t rx1count;      /* Rx1CountRegister */
   uint8_t  tx1type;       /* Tx1TypeRegister */
   uint8_t  tx1interval;   /* Tx1IntervalRegister */
   uint8_t  rx1type;       /* Rx1TypeRegister */
   uint8_t  rx1interval;   /* Rx1IntervalRegister */
   uint8_t  revered8[1];
   uint8_t  fifosize1;     /* EP1ConfiguredFIFOSizeRegister */
   uint8_t  revered9[224];
   /*0x200*/
   uint8_t  dma_intr_status;
   uint8_t  dma_intr_unmask;
   uint8_t  dma_intr_unmask_clear;
   uint8_t  dma_intr_unmask_set;
   uint16_t dma_cntl_0;    /* DMAChannel0ControlRegister */
   uint8_t  revered10[2];  /*0x120~1FF*/
   uint32_t dma_addr_0;    /* DMAChannel0AddressRegister */
   uint32_t dma_count_0;   /* DMAChannel0ByteCountRegister */
   /*0x210*/
   uint32_t dma_limiter;   /* DMALimiterRegister */
   uint8_t  revered11[12];
   /*0x220*/
   uint32_t dma_config;    /* DMAConfigurationRegister */
   uint8_t  revered12[224];
   /*0x304*/
   uint16_t ep1rxpktcount; /* EP1RxPktCountRegister */
   uint8_t  revered14[378];
   /*0x480*/
   uint16_t t0funcaddr;    /* T0FunctionAddressRegister */
   uint16_t t0hubaddr;     /* T0HUBAddressRegister */
   uint8_t  revered15[4];
   uint16_t t1funcaddr;    /* T1FunctionAddressRegister */
   uint16_t t1hubaddr;     /* T1HUBAddressRegister */
   uint16_t r1funcaddr;    /* R1FunctionAddressRegister */
   uint16_t r1hubaddr;     /* R1HUBAddressRegister */
   uint16_t t2funcaddr;    /* T2FunctionAddressRegister */
   uint16_t t2hubaddr;     /* T2HUBAddressRegister */
   uint16_t r2funcaddr;    /* R2FunctionAddressRegister */
   uint16_t r2hubaddr;     /* R2HUBAddressRegister */
   uint16_t t3funcaddr;    /* T3FunctionAddressRegister */
   uint16_t t3hubaddr;     /* T3HUBAddressRegister */
   uint16_t r3funcaddr;    /* R3FunctionAddressRegister */
   uint16_t r3hubaddr;     /* R3HUBAddressRegister */
   uint16_t t4funcaddr;    /* T4FunctionAddressRegister */
   uint16_t t4hubaddr;     /* T4HUBAddressRegister */
   uint16_t r4funcaddr;    /* R4FunctionAddressRegister */
   uint16_t r4hubaddr;     /* R4HUBAddressRegister */
   uint16_t t5funcaddr;    /* T5FunctionAddressRegister */
   uint16_t t5hubaddr;     /* T5HUBAddressRegister */
   uint16_t r5funcaddr;    /* R5FunctionAddressRegister */
   uint16_t r5hubaddr;     /* R5HUBAddressRegister */
   uint16_t t6funcaddr;    /* T6FunctionAddressRegister */
   uint16_t t6hubaddr;     /* T6HUBAddressRegister */
   uint16_t r6funcaddr;    /* R6FunctionAddressRegister */
   uint16_t r6hubaddr;     /* R6HUBAddressRegister */
   uint16_t t7funcaddr;    /* T7FunctionAddressRegister */
   uint16_t t7hubaddr;     /* T7HUBAddressRegister */
   uint16_t r7funcaddr;    /* R7FunctionAddressRegister */
   uint16_t r7hubaddr;     /* R7HUBAddressRegister */
   uint16_t t8funcaddr;    /* T8FunctionAddressRegister */
   uint16_t t8hubaddr;     /* T8HUBAddressRegister */
   uint16_t r8funcaddr;    /* R8FunctionAddressRegister */
   uint16_t r8hubaddr;     /* R8HUBAddressRegister */
   uint16_t t9funcaddr;    /* T9FunctionAddressRegister */
   uint16_t t9hubaddr;     /* T9HUBAddressRegister */
   uint16_t r9funcaddr;    /* R9FunctionAddressRegister */
   uint16_t r9hubaddr;     /* R9HUBAddressRegister */
   uint16_t t10funcaddr;   /* T10FunctionAddressRegister */
   uint16_t t10hubaddr;    /* T10HUBAddressRegister */
   uint16_t r10funcaddr;   /* R10FunctionAddressRegister */
   uint16_t r10hubaddr;    /* R10HUBAddressRegister */
   uint16_t t11funcaddr;   /* T11FunctionAddressRegister */
   uint16_t t11hubaddr;    /* T11HUBAddressRegister */
   uint16_t r11funcaddr;   /* R11FunctionAddressRegister */
   uint16_t r11hubaddr;    /* R11HUBAddressRegister */
   uint16_t t12funcaddr;   /* T12FunctionAddressRegister */
   uint16_t t12hubaddr;    /* T12HUBAddressRegister */
   uint16_t r12funcaddr;   /* R12FunctionAddressRegister */
   uint16_t r12hubaddr;    /* R12HUBAddressRegister */
   uint16_t t13funcaddr;   /* T13FunctionAddressRegister */
   uint16_t t13hubaddr;    /* T13HUBAddressRegister */
   uint16_t r13funcaddr;   /* R13FunctionAddressRegister */
   uint16_t r13hubaddr;    /* R13HUBAddressRegister */
   uint16_t t14funcaddr;   /* T14FunctionAddressRegister */
   uint16_t t14hubaddr;    /* T14HUBAddressRegister */
   uint16_t r14funcaddr;   /* R14FunctionAddressRegister */
   uint16_t r14hubaddr;    /* R14HUBAddressRegister */
   uint16_t t15funcaddr;   /* T15FunctionAddressRegister */
   uint16_t t15hubaddr;    /* T15HUBAddressRegister */
   uint16_t r15funcaddr;   /* R15FunctionAddressRegister */
   uint16_t r15hubaddr;    /* R15HUBAddressRegister */
   uint8_t  revered16[260];
   /*0x604*/
   uint16_t tm1;           /* TestMode1Register */
   uint8_t  revered17[2];
   uint32_t hwver_date;    /* HWVersionControlRegister */
   uint8_t  revered18[120];
   /*0x684*/
   uint32_t srama;         /* SRAMAddressRegister */
   uint32_t sramd;         /* SRAMDataRegister */
   uint8_t  revered19[4];
   /*0x690*/
   uint32_t risc_size;     /* RISCSizeRegister */
   uint8_t  revered20[108];
   /*0x700*/
   uint32_t resreg;        /* ReservedRegister */
   uint8_t  revered21[44];
   /*0x730*/
   uint8_t  otg20_csrl;    /* OTG20RelatedControlRegisterL */
   uint8_t  otg20_csrh;    /* OTG20RelatedControlRegisterH */
} USB_REGISTER_T;

/* USBPHYACR0 */
#define USBPHYACR0_RG_USB20_BGR_EN                 (1 << 0)
#define USBPHYACR0_RG_USB20_CHP_EN                 (1 << 1)
#define USBPHYACR0_RG_USB20_REF_EN                 (1 << 4)
#define USBPHYACR0_RG_USB20_INTR_EN                (1 << 5)
#define USBPHYACR0_RG_USB20_USBPLL_FORCE_ON        (1 << 15)
#define USBPHYACR0_RG_USB20_PLL_FS_EN              (1 << 22)
#define USBPHYACR0_RG_USB20_BGR_DIV_MSK            (0x03 <<  2)
#define USBPHYACR0_RG_USB20_PLL_PREDIV_MSK         (0x03 <<  6)
#define USBPHYACR0_RG_USB20_PLL_FBDIV_MSK          (0x7F <<  8)
#define USBPHYACR0_RG_USB20_PLL_FS_MSK             (0x03 << 16)
#define USBPHYACR0_RG_USB20_PLL_IBIAS_MSK          (0x03 << 18)
#define USBPHYACR0_RG_USB20_PLL_ICHP_MSK           (0x03 << 20)
#define USBPHYACR0_RG_USB20_TX_PH_ROT_SEL_MSK      (0x07 << 24)
#define USBPHYACR0_RG_USB20_MPX_OUT_SEL_MSK        (0x07 << 28)

/* USBPHYACR1 */
#define USBPHYACR1_RG_USB20_MPX_SEL_MSK            (0xFF <<  0)
#define USBPHYACR1_RG_USB20_TERM_VREF_SEL_MSK      (0x07 <<  8)
#define USBPHYACR1_RG_USB20_VRT_VREF_SEL_MSK       (0x07 << 12)
#define USBPHYACR1_RG_USB20_OTG_VBUSTH_MSK         (0x03 << 16)
#define USBPHYACR1_RG_USB20_INTR_CAL_MSK           (0x1F << 19)
#define USBPHYACR1_RG_USB20_CLKREF_REV_MSK         (0xFF << 24)

/* USBPHYACR2 */
#define USBPHYACR2_RG_USB20_CLKREF_REV_1_MSK       (0xFF <<  0)
#define USBPHYACR2_RG_USB20_PLL_BW_MSK             (0x07 << 19)

/* USBPHYACR4 */
#define USBPHYACR4_RG_USB20_ICUSB_EN               (1 << 16)
#define USBPHYACR4_RG_USB20_DP_ABIST_SOURCE_EN     (1 << 31)
#define USBPHYACR4_RG_USB20_FS_SR_MSK              (0x03 <<  0)
#define USBPHYACR4_RG_USB20_LS_SR_MSK              (0x03 <<  4)
#define USBPHYACR4_RG_USB20_FS_CR_MSK              (0x03 <<  8)
#define USBPHYACR4_RG_USB20_LS_CR_MSK              (0x03 << 12)
#define USBPHYACR4_RG_USB20_DP_ABIST_SELE_MSK      (0x0F << 24)

/* USBPHYACR5 */
#define USBPHYACR5_RG_USB20_DM_ABIST_SOURCE_EN     (1 << 7)
#define USBPHYACR5_RG_USB20_GBIAS_ENB              (1 << 10)
#define USBPHYACR5_RG_USB20_HS_100U_U3_EN          (1 << 11)
#define USBPHYACR5_RG_USB20_HSTX_SRCAL_EN          (1 << 15)
#define USBPHYACR5_RG_USB20_CHIRP_EN               (1 << 16)
#define USBPHYACR5_RG_USB20_INLPBK_EN              (1 << 17)
#define USBPHYACR5_RG_USB20_PHYD_MONEN             (1 << 18)
#define USBPHYACR5_RG_USB20_HSTX_TMODE_EN          (1 << 19)
#define USBPHYACR5_RG_USB20_DISC_FIT_EN            (1 << 28)
#define USBPHYACR5_RG_USB20_DM_ABIST_SELE_MSK      (0x0F <<  0)
#define USBPHYACR5_RG_USB20_HSTX_SRCTRL_MSK        (0x07 << 12)
#define USBPHYACR5_RG_USB20_DISCD_MSK              (0x03 << 20)
#define USBPHYACR5_RG_USB20_SQD_MSK                (0x03 << 22)
#define USBPHYACR5_RG_USB20_HSTX_TMODE_SEL_MSK     (0x03 << 24)
#define USBPHYACR5_RG_USB20_INIT_SQ_EN_DG_MSK      (0x03 << 26)

/* USBPHYACR6 */
#define USBPHYACR6_RG_USB20_HSRX_TMODE_EN          (1 <<  8)
#define USBPHYACR6_RG_USB20_OTG_ABIST_EN           (1 << 19)
#define USBPHYACR6_RG_USB20_OTG_VBUSCMP_EN         (1 << 20)
#define USBPHYACR6_RG_USB20_SR_CLK_SEL             (1 << 22)
#define USBPHYACR6_RG_USB20_BC11_SW_EN             (1 << 23)
#define USBPHYACR6_RG_USB20_SQTH_MSK               (0x0F <<  0)
#define USBPHYACR6_RG_USB20_DISCTH_MSK             (0x0F <<  4)
#define USBPHYACR6_RG_USB20_HSRX_BIAS_EN_SEL_MSK   (0x03 <<  9)
#define USBPHYACR6_RG_USB20_HSRX_MMODE_SELE_MSK    (0x03 << 12)
#define USBPHYACR6_RG_USB20_PHY_REV_MSK            (0xFF << 24)

/* U2PHYACR3 */
#define U2PHYACR3_RG_USB20_EN_PD_DP                (1 <<  8)
#define U2PHYACR3_RG_USB20_EN_PU_DP                (1 <<  9)
#define U2PHYACR3_RG_USB20_EN_PD_DM                (1 << 10)
#define U2PHYACR3_RG_USB20_EN_PU_DM                (1 << 11)
#define U2PHYACR3_RG_USB20_PUPD_BIST_EN            (1 << 12)
#define U2PHYACR3_RG_USB20_TMODE_FS_LS_MODE        (1 << 16)
#define U2PHYACR3_RG_USB20_TMODE_FS_LS_RCV_EN      (1 << 17)
#define U2PHYACR3_RG_USB20_TMODE_FS_LS_TX_EN       (1 << 18)
#define U2PHYACR3_RG_USB20_USB11_TMODE_EN          (1 << 19)
#define U2PHYACR3_RG_USB20_HSTX_BIST_EN            (1 << 26)
#define U2PHYACR3_RG_USB20_HS_TERM_EN_MODE_MSK     (0x03 << 13)
#define U2PHYACR3_RG_USB20_HSTX_I_EN_MODE_MSK      (0x03 << 24)
#define U2PHYACR3_RG_USB20_HSTX_DBIST_MSK          (0x0F << 28)

/* U2PHYACR4 */
#define U2PHYACR4_RG_USB20_TX_VCMPDN_EN            (1 <<  4)
#define U2PHYACR4_RG_USB20_TX_BIAS_EN              (1 <<  5)
#define U2PHYACR4_usb20_gpio_mode                  (1 <<  8)
#define U2PHYACR4_rg_usb20_gpio_ctl                (1 <<  9)
#define U2PHYACR4_usb20_gpio_dp_oe                 (1 << 12)
#define U2PHYACR4_usb20_gpio_dm_oe                 (1 << 13)
#define U2PHYACR4_usb20_gpio_dp_i                  (1 << 14)
#define U2PHYACR4_usb20_gpio_dm_i                  (1 << 15)
#define U2PHYACR4_usb20_dp_100k_en                 (1 << 16)
#define U2PHYACR4_RG_USB20_DM_100K_EN              (1 << 17)
#define U2PHYACR4_rg_usb20_dp_100k_mode            (1 << 18)
#define U2PHYACR4_RG_USB20_HS_RCV_EN_MODE_MSK      (0x03 <<  0)
#define U2PHYACR4_RG_USB20_HS_SQ_EN_MODE_MSK       (0x03 <<  3)

/* U2PHYDCR0 */
#define U2PHYDCR0_RG_USB20_NEGEDGE_ENB             (1 << 4)
#define U2PHYDCR0_RG_USB20_TX_TST                  (1 << 5)
#define U2PHYDCR0_RG_USB20_EARLY_HSTX_I            (1 << 6)
#define U2PHYDCR0_RG_USB20_EBTHRLD                 (1 << 7)
#define U2PHYDCR0_RG_USB20_PLL_FORCE_ON            (1 << 24)
#define U2PHYDCR0_RG_USB20_PLL_STABLE              (1 << 25)
#define U2PHYDCR0_RG_USB20_GATED_ENB               (1 << 29)
#define U2PHYDCR0_RG_USB20_CDR_FILT_MSK            (0x0F   <<  0)
#define U2PHYDCR0_RG_USB20_PHYD_RESERVE_MSK        (0xFFFF <<  8)
#define U2PHYDCR0_RG_USB20_TESTMODE_MSK            (0x03   << 26)
#define U2PHYDCR0_RG_USB20_CDR_TST_MSK             (0x03   << 30)

/* U2PHYDCR1 */
#define U2PHYDCR1_RG_USB20_FORCE_TAP               (1 << 12)
#define U2PHYDCR1_RG_USB20_OTG_PROBE               (1 << 20)
#define U2PHYDCR1_rg_debug_en                      (1 << 22)
#define U2PHYDCR1_RG_USB20_DRVVBUS                 (1 << 23)
#define U2PHYDCR1_RG_USB20_TAPSEL_MSK              (0x03   <<  0)
#define U2PHYDCR1_RG_USB20_LBMODE_MSK              (0x0FFF << 13)
#define U2PHYDCR1_RG_USB20_BERTH_MSK               (0x03   << 16)
#define U2PHYDCR1_RG_USB20_SW_PLLMODE_MSK          (0x03   << 18)
#define U2PHYDCR1_RG_USB20_PROBE_SEL_MSK           (0xFF   << 24)

/* U2PHYDTM0 */
#define U2PHYDTM0_RG_TERMSEL                       (1 <<  2)
#define U2PHYDTM0_RG_SUSPENDM                      (1 <<  3)
#define U2PHYDTM0_RG_DPPULLDOWN                    (1 <<  6)
#define U2PHYDTM0_RG_DMPULLDOWN                    (1 <<  7)
#define U2PHYDTM0_RG_TXVALID                       (1 <<  8)
#define U2PHYDTM0_RG_TXVALIDH                      (1 <<  9)
#define U2PHYDTM0_RG_RESET                         (1 << 14)
#define U2PHYDTM0_utmi_muxsel                      (1 << 15)
#define U2PHYDTM0_force_opmode                     (1 << 16)
#define U2PHYDTM0_force_termsel                    (1 << 17)
#define U2PHYDTM0_force_suspendm                   (1 << 18)
#define U2PHYDTM0_force_xcvrsel                    (1 << 19)
#define U2PHYDTM0_force_dp_pulldown                (1 << 20)
#define U2PHYDTM0_force_dm_pulldown                (1 << 21)
#define U2PHYDTM0_force_txvalid                    (1 << 22)
#define U2PHYDTM0_force_datain                     (1 << 23)
#define U2PHYDTM0_force_drvvbus                    (1 << 24)
#define U2PHYDTM0_force_usb_clken                  (1 << 25)
#define U2PHYDTM0_force_uart_en                    (1 << 26)
#define U2PHYDTM0_force_uart_tx_oe                 (1 << 27)
#define U2PHYDTM0_force_uart_bias_en               (1 << 28)
#define U2PHYDTM0_force_uart_i                     (1 << 29)
#define U2PHYDTM0_RG_OPMODE_MSK                    (0x03 <<  0)
#define U2PHYDTM0_RG_XCVRSEL_MSK                   (0x03 <<  4)
#define U2PHYDTM0_RG_DATAIN_MSK                    (0x0F << 10)
#define U2PHYDTM0_rg_uart_mode_MSK                 (0x03 << 30)

/* U2PHYDTM1 */
#define U2PHYDTM1_RG_IDPULLUP                      (1 <<  0)
#define U2PHYDTM1_RG_IDDIG                         (1 <<  1)
#define U2PHYDTM1_RG_AVALID                        (1 <<  2)
#define U2PHYDTM1_RG_BVALID                        (1 <<  3)
#define U2PHYDTM1_RG_SESSEND                       (1 <<  4)
#define U2PHYDTM1_RG_VBUSVALID                     (1 <<  5)
#define U2PHYDTM1_force_idpullup                   (1 <<  8)
#define U2PHYDTM1_force_iddig                      (1 <<  9)
#define U2PHYDTM1_force_avalid                     (1 << 10)
#define U2PHYDTM1_force_bvalid                     (1 << 11)
#define U2PHYDTM1_force_sessend                    (1 << 12)
#define U2PHYDTM1_force_vbusvalid                  (1 << 13)
#define U2PHYDTM1_RG_UART_EN                       (1 << 16)
#define U2PHYDTM1_RG_UART_TX_OE                    (1 << 17)
#define U2PHYDTM1_RG_UART_BIAS_EN                  (1 << 18)
#define U2PHYDTM1_RG_UART_I                        (1 << 19)
#define U2PHYDTM1_RG_USB20_CLK60M_EN               (1 << 22)
#define U2PHYDTM1_RG_USB20_CLK48M_EN               (1 << 23)
#define U2PHYDTM1_RG_USB20_PRBS7_EN                (1 << 31)
#define U2PHYDTM1_rg_sifprb_sel_MSK                (0x03 <<  6)
#define U2PHYDTM1_RG_USB20_PRBS7_BITCNT_MSK        (0x3F << 24)

/* U2PHYDMON0 */
#define U2PHYDMON0_RG_USB20_BGLPF_FORCE_ON         (1 << 8)
#define U2PHYDMON0_RG_USB20_BGLPF_FORCE_OFF        (1 << 9)
#define U2PHYDMON0_RG_USB20_PRBS7_BERTH_MSK        (0xFF <<  0)
#define U2PHYDMON0_RG_USB20_BGLPF_EN_OPT_MSK       (0x0F << 10)
#define U2PHYDMON0_RG_USB20_EOP_CTL_MSK            (0x0F << 16)

/* U2PHYCR3 */
#define U2PHYCR3_force_linestate                   (1 << 2)
#define U2PHYCR3_RG_LINESTATE_MSK                  (0x03 << 0)

/* U2PHYCR4 */
#define U2PHYCR4_RG_USB20_FORCE_CHIRP_EN              (1 <<  0)
#define U2PHYCR4_RG_USB20_TX_CHIRP_SEL                (1 <<  1)
#define U2PHYCR4_RG_USB20_LBTOGEND                    (1 << 10)
#define U2PHYCR4_RG_USB20_CHIRP_DET_DISC_CYCLE1_MSK   (0x0F << 2)
#define U2PHYCR4_RG_USB20_CHIRP_DET_DISC_CYCLE2_MSK   (0x0F << 6)

/* FMCR0 */
#define FMCR0_RG_FREQDET_EN                        (1 << 24)
#define FMCR0_RG_FM_MODE                           (1 << 25)
#define FMCR0_RG_CYCLECNT                          (0x00FFFFFF <<  0)
#define FMCR0_RG_MONCLK_SEL_MSK                    (0x03 << 26)
#define FMCR0_RG_LOCKTH_MSK                        (0x0F << 28)

/* FMCR1 */
#define FMCR1_RG_TARGET_MSK                        (0xFFFFFFFF << 0)

/* FMCR2 */
#define FMCR2_RG_OFFSET_MSK                        (0xFFFFFFFF << 0)

/* FMMONR0 */
#define FMMONR0_USB_FM_OUT_MSK                     (0xFFFFFFFF << 0)

/* FMMONR1 */
#define FMMONR1_USB_FM_VLD                         (1 << 0)
#define FMMONR1_USBPLL_LOCK                        (1 << 1)
#define FMMONR1_RG_FRCK_EN                         (1 << 8)


/**
 * Below RG macro is dummy code.
 */
#define U2PHYAC0_RG_USB_LVSH_EN		(1U<<31)
#define	U2PHYACR3_RG_USB20_PHY_REV_7	(1<<7)
#define	U2PHYACR4_RG_USB20_DP_100K_EN	(1<<16)
#define	U2PHYACR2_RG_USB20_OTG_VBUSCMP_EN	(1<<27)
#define	U2PHYDTM0_RG_XCVRSEL_1_0	(1<<4)
#define	U2PHYDTM0_RG_XCVRSEL_1_0_OFST	(4)
#define	U2PHYDTM0_RG_DATAIN_3_0		(1<<10)
#define	U2PHYDTM0_RG_DATAIN_3_0_OFST		(10)

#define	U2PHYACR0_RG_USB20_HSTX_SRCAL_EN (1<<23)
#define	U2PHYACR0_RG_USB20_HSTX_SRCTRL_2_0 (1<16)
#define U2PHYACR0_RG_USB20_HSTX_SRCTRL_CLR      (0x7<<16)
#define USB_FMMONR1_RG_FRCK_EN (1<<8)

typedef struct {
   /*0x00*/
   uint32_t revered[512];
   uint32_t usbphyacr0;    /* 0x800 */
   uint32_t usbphyacr1;    /* 0x804 */
   uint32_t usbphyacr2;    /* 0x808 */
   uint32_t revered1;
   uint32_t usbphyacr4;    /* 0x810 */
   uint32_t usbphyacr5;    /* 0x814 */
   uint32_t usbphyacr6;    /* 0x818 */
   uint32_t u2phyacr3;     /* 0x81c */
   uint32_t u2phyacr4;     /* 0x820 */
   uint32_t revered2[15];
   uint32_t u2phydcr0;     /* 0x860 */
   uint32_t u2phydcr1;     /* 0x864 */
   uint32_t u2phydtm0;     /* 0x868 */
   uint32_t u2phydtm1;     /* 0x86c */
   uint32_t u2phydmon0;    /* 0x870 */
   uint32_t u2phydmon1;    /* 0x874 */
   uint32_t u2phydmon2;    /* 0x878 */
   uint32_t revered3;      /* 0x87c */
   uint32_t u2phycr3;      /* 0x880 */
   uint32_t revered4;      /* 0x884 */
   uint32_t u2phycr4;      /* 0x888 */
   uint32_t revered5[13];
   uint32_t u1phycr0;      /* 0x8c0 */
   uint32_t u1phycr1;      /* 0x8c4 */
   uint32_t u1phycr2;      /* 0x8c8 */
   uint32_t revered6[5];
   uint32_t regfppc;       /* 0x8e0 */
   uint32_t revered7[3];
   uint32_t versionc;      /* 0x8f0 */
   uint32_t revered8[2];
   uint32_t regfcom;       /* 0x8fc */
   uint32_t revered9[384];
   uint32_t fmcr0;         /* 0xf00 */
   uint32_t fmcr1;         /* 0xf04 */
   uint32_t fmcr2;         /* 0xf08 */
   uint32_t fmmonr0;       /* 0xf0c */
   uint32_t fmmonr1;       /* 0xf10 */
} USBPHY_REGISTER_T;

typedef struct {
    /*0x00*/
    uint32_t	revered[512];	//
    uint32_t	usbphyacr0;     //0x800
    uint32_t	usbphyacr1;     //0x804
    uint32_t	usbphyacr2;     //0x808
    uint32_t	revered1;      //
    uint32_t	usbphyacr4;    //0x810
    uint32_t	usbphyacr5;    //0x814
    uint32_t	usbphyacr6;    //0x818
    uint32_t	u2phyacr3;    //0x81c
    uint32_t	u2phyacr4;    //0x820
    uint32_t	revered2[15];  //
    uint32_t	u2phydcr0;    //0x860
    uint32_t	u2phydcr1;    //0x864
    uint32_t	u2phydtm0;    //0x868
    uint32_t	u2phydtm1;    //0x86c
    uint32_t	u2phydmon0;   //0x870
    uint32_t	u2phydmon1;   //0x874
    uint32_t	u2phydmon2;   //0x878
    uint32_t	revered3;     //0x87c
    uint32_t	u2phycr3;     //0x880
    uint32_t	revered4;     //0x884
    uint32_t	u2phycr4;     //0x888
    uint32_t	revered5[13];  //
    uint32_t	u1phycr0;     //0x8c0
    uint32_t	u1phycr1;     //0x8c4
    uint32_t	u1phycr2;     //0x8c8
    uint32_t	revered6[5];   //
    uint32_t	regfppc;      //0x8e0
    uint32_t	revered7[3];   //
    uint32_t	versionc;     //0x8f0
    uint32_t	revered8[2];   //
    uint32_t	regfcom;			//0x8fc
    uint32_t	revered9[384];
    uint32_t	fmcr0;				//0xf00
    uint32_t	fmcr1;				//0xf04
    uint32_t	fmcr2;				//0xf08
    uint32_t	fmmonr0;			//0xf0c
    uint32_t	fmmonr1;			//0xf10
}USBPHY_HOST_REGISTER_T;

typedef enum {
   HAL_USB_LINESTATE_DM_0_DP_0 = 0,
   HAL_USB_LINESTATE_DM_0_DP_1 = 1,
   HAL_USB_LINESTATE_DM_1_DP_0 = 2,
   HAL_USB_LINESTATE_DM_1_DP_1 = 3,
} hal_usb_linestate_t;

extern void hal_usbphy_poweron_initialize(void);
extern void hal_usbphy_deinit_case(void);
extern void hal_usbphy_save_current(void);
extern void hal_usbphy_recover(void);
extern void hal_usbphy_slew_rate_calibration(void);
extern void RestoreIRQMask(uint32_t);
extern uint32_t SaveAndSetIRQMask(void);
extern void hal_usbphy_save_current_for_charge(void);
extern void hal_usbphy_enter_suspend(void);
extern void hal_usbphy_leave_suspend(void);
extern void hal_usb_save_current(void);
extern hal_usb_status_t hal_usb_phy_preinit(void);
void hal_usbphy_suspend(void);
void hal_usbphy_resume(void);
bool hal_usbphy_detect_linestate_change(void);
void hal_usbphy_reset_linestate_change(void);
hal_usb_status_t hal_usb_set_speed(bool hs_enable);
hal_usb_power_state_t hal_usb_get_suspend_state(void);

hal_usb_linestate_t hal_usbphy_detect_linestate(void);

uint8_t hal_usb_rmwk_trigger(void);
uint8_t hal_usb_rmwk_check(void);
uint8_t hal_usb_rmwk_clear(void);

#endif /* HAL_USB_MODULE_ENABLED or HAL_USB_HOST_MODULE_ENABLED */
#endif /* __HAL_USB_INTERNAL_H__ */

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

#include "hal_usb.h"
#include "hal_usb_internal.h"

#ifdef HAL_USB_MODULE_ENABLED

#include "assert.h"
#include "hal_clock.h"
#include "hal_clock_internal.h"
#include "hal_clock_platform.h"
#include "hal_dvfs.h"
#include "hal_gpt.h"
#include "hal_log.h"
#include "hal_nvic.h"
#include "hal_pmu.h"
#include "hal_resource_assignment.h"
#include "hal_rtc_internal.h"

#ifdef HAL_USB_EINT_ENABLE
#include "hal_eint.h"
#endif /* HAL_USB_EINT_ENABLE */

volatile USB_REGISTER_T *musb = (USB_REGISTER_T *)USB_BASE;
static void usb_pdn_enable(void);
static void usb_pdn_disable(void);

static volatile USB_Drv_Info usb_info;
static volatile USB_Drv_Info *g_UsbDrvInfo = &usb_info;

#define HAL_USB_FLAG__W1C (1<<0)
uint8_t g_hal_usb_flag = HAL_USB_FLAG__W1C;

/* EP0's FIFO address is fixed from 0~63 */
static uint32_t g_FIFOadd = USB_FIFO_START_ADDRESS;
static uint8_t usb_get_dma_channel_num(uint8_t ep_num, hal_usb_endpoint_direction_t direction);
static void usb_hw_stop_dma_channel(uint32_t ep_num, hal_usb_endpoint_direction_t direction);
static void usb_ep_check(uint32_t ep_num, hal_usb_endpoint_direction_t direction, uint32_t line);
static void usb_ep_dma_running_check(uint32_t ep_num, hal_usb_endpoint_direction_t direction, uint32_t line);
static void usb_hw_epfifowrite(uint32_t ep_num, uint16_t nBytes, void *pSrc);
static bool usb_check_dma_time_out(uint8_t dma_chan);
static void usb_dma_callback_func(uint8_t dma_chan);
static void hal_usb_do_suspend(void);
static void hal_usb_do_resume(void);

static void hal_usb_detect_linestate(void* p);
static void hal_usb_start_gpt_detect_linestate(void);

extern uint8_t clock_set_pll_off(clock_pll_id pll_id);
#ifdef HAL_PMU_MODULE_ENABLED
extern pmu_power_vcore_voltage_t pmu_vcore_lock_control(pmu_power_stage_t mode, pmu_power_vcore_voltage_t vol, pmu_lock_parameter_t lock);
#endif

static bool usb_init_phy_ready = false;
/************************************************************
    USB PDN
*************************************************************/
static void usb_pdn_enable(void)
{
    /* Turn off PMU die power */
    pmu_enable_usb_power(PMU_OFF);

    /* Disable clock source */
    hal_clock_disable(HAL_CLOCK_CG_USB_DMA);
    hal_clock_disable(HAL_CLOCK_CG_USB_BUS);
    hal_clock_disable(HAL_CLOCK_CG_USB);

    pmu_lock_vcore(PMU_NORMAL, PMIC_VCORE_0P80_V, PMU_UNLOCK);
}

/* USB PDN disable */
static void usb_pdn_disable(void)
{
    /* Turn ON Vcore 0.8v */
    pmu_lock_vcore(PMU_NORMAL, PMIC_VCORE_0P80_V, PMU_LOCK);

    /* Switch UPLL 62.4MHz */
    clock_mux_sel(CLK_USB_SEL, 3);

    /* Enable clock source */
    hal_clock_enable(HAL_CLOCK_CG_USB_BUS);
    hal_clock_enable(HAL_CLOCK_CG_USB_DMA);
    hal_clock_enable(HAL_CLOCK_CG_USB);

    /* Turn on PMU die power */
    pmu_enable_usb_power(PMU_ON);
}

/************************************************************
    Dump USB RG for debug
*************************************************************/
#if 0
static void hal_dump_regs(void)
{
    volatile USBPHY_REGISTER_T *phy = (USBPHY_REGISTER_T *)USB_SIFSLV_BASE;
    volatile USB_REGISTER_T *mac = (USB_REGISTER_T *)USB_BASE;

    log_hal_msgid_info("mac->faddr                 (0x%08X) : 0x%08x", 2, &mac->faddr,          mac->faddr);
    log_hal_msgid_info("mac->power                 (0x%08X) : 0x%08x", 2, &mac->power,          mac->power);
    log_hal_msgid_info("mac->intrtx                (0x%08X) : 0x%08x", 2, &mac->intrtx,         mac->intrtx);
    log_hal_msgid_info("mac->intrrx                (0x%08X) : 0x%08x", 2, &mac->intrrx,         mac->intrrx);
    log_hal_msgid_info("mac->intrtxe               (0x%08X) : 0x%08x", 2, &mac->intrtxe,        mac->intrtxe);
    log_hal_msgid_info("mac->intrrxe               (0x%08X) : 0x%08x", 2, &mac->intrrxe,        mac->intrrxe);
    log_hal_msgid_info("mac->intrusb               (0x%08X) : 0x%08x", 2, &mac->intrusb,        mac->intrusb);
    log_hal_msgid_info("mac->intrusbe              (0x%08X) : 0x%08x", 2, &mac->intrusbe,       mac->intrusbe);
    log_hal_msgid_info("mac->frame                 (0x%08X) : 0x%08x", 2, &mac->frame,          mac->frame);
    log_hal_msgid_info("mac->index                 (0x%08X) : 0x%08x", 2, &mac->index,          mac->index);
    log_hal_msgid_info("mac->testmode              (0x%08X) : 0x%08x", 2, &mac->testmode,       mac->testmode);
    log_hal_msgid_info("mac->txmap                 (0x%08X) : 0x%08x", 2, &mac->txmap,          mac->txmap);
    log_hal_msgid_info("mac->txcsr                 (0x%08X) : 0x%08x", 2, &mac->txcsr,          mac->txcsr);
    log_hal_msgid_info("mac->rxmap                 (0x%08X) : 0x%08x", 2, &mac->rxmap,          mac->rxmap);
    log_hal_msgid_info("mac->rxcsr                 (0x%08X) : 0x%08x", 2, &mac->rxcsr,          mac->rxcsr);
    log_hal_msgid_info("mac->rxcount               (0x%08X) : 0x%08x", 2, &mac->rxcount,        mac->rxcount);
    log_hal_msgid_info("mac->txtype                (0x%08X) : 0x%08x", 2, &mac->txtype,         mac->txtype);
    log_hal_msgid_info("mac->txinterval            (0x%08X) : 0x%08x", 2, &mac->txinterval,     mac->txinterval);
    log_hal_msgid_info("mac->rxtype                (0x%08X) : 0x%08x", 2, &mac->rxtype,         mac->rxtype);
    log_hal_msgid_info("mac->rxinterval            (0x%08X) : 0x%08x", 2, &mac->rxinterval,     mac->rxinterval);
    log_hal_msgid_info("mac->fifosize              (0x%08X) : 0x%08x", 2, &mac->fifosize,       mac->fifosize);
    log_hal_msgid_info("mac->fifo0                 (0x%08X) : 0x%08x", 2, &mac->fifo0,          mac->fifo0);
    log_hal_msgid_info("mac->fifo1                 (0x%08X) : 0x%08x", 2, &mac->fifo1,          mac->fifo1);
    log_hal_msgid_info("mac->devctl                (0x%08X) : 0x%08x", 2, &mac->devctl,         mac->devctl);
    log_hal_msgid_info("mac->pwrupcnt              (0x%08X) : 0x%08x", 2, &mac->pwrupcnt,       mac->pwrupcnt);
    log_hal_msgid_info("mac->txfifosz              (0x%08X) : 0x%08x", 2, &mac->txfifosz,       mac->txfifosz);
    log_hal_msgid_info("mac->rxfifosz              (0x%08X) : 0x%08x", 2, &mac->rxfifosz,       mac->rxfifosz);
    log_hal_msgid_info("mac->txfifoadd             (0x%08X) : 0x%08x", 2, &mac->txfifoadd,      mac->txfifoadd);
    log_hal_msgid_info("mac->rxfifoadd             (0x%08X) : 0x%08x", 2, &mac->rxfifoadd,      mac->rxfifoadd);
    log_hal_msgid_info("mac->hwcaps                (0x%08X) : 0x%08x", 2, &mac->hwcaps,         mac->hwcaps);
    log_hal_msgid_info("mac->hwsvers               (0x%08X) : 0x%08x", 2, &mac->hwsvers,        mac->hwsvers);
    log_hal_msgid_info("mac->busperf1              (0x%08X) : 0x%08x", 2, &mac->busperf1,       mac->busperf1);
    log_hal_msgid_info("mac->busperf2              (0x%08X) : 0x%08x", 2, &mac->busperf2,       mac->busperf2);
    log_hal_msgid_info("mac->busperf3              (0x%08X) : 0x%08x", 2, &mac->busperf3,       mac->busperf3);
    log_hal_msgid_info("mac->epinfo                (0x%08X) : 0x%08x", 2, &mac->epinfo,         mac->epinfo);
    log_hal_msgid_info("mac->raminfo               (0x%08X) : 0x%08x", 2, &mac->raminfo,        mac->raminfo);
    log_hal_msgid_info("mac->linkinfo              (0x%08X) : 0x%08x", 2, &mac->linkinfo,       mac->linkinfo);
    log_hal_msgid_info("mac->vplen                 (0x%08X) : 0x%08x", 2, &mac->vplen,          mac->vplen);
    log_hal_msgid_info("mac->hs_eof1               (0x%08X) : 0x%08x", 2, &mac->hs_eof1,        mac->hs_eof1);
    log_hal_msgid_info("mac->fs_eof1               (0x%08X) : 0x%08x", 2, &mac->fs_eof1,        mac->fs_eof1);
    log_hal_msgid_info("mac->ls_eof1               (0x%08X) : 0x%08x", 2, &mac->ls_eof1,        mac->ls_eof1);
    log_hal_msgid_info("mac->rst_info              (0x%08X) : 0x%08x", 2, &mac->rst_info,       mac->rst_info);
    log_hal_msgid_info("mac->rxtog                 (0x%08X) : 0x%08x", 2, &mac->rxtog,          mac->rxtog);
    log_hal_msgid_info("mac->rxtogen               (0x%08X) : 0x%08x", 2, &mac->rxtogen,        mac->rxtogen);
    log_hal_msgid_info("mac->txtog                 (0x%08X) : 0x%08x", 2, &mac->txtog,          mac->txtog);
    log_hal_msgid_info("mac->txtogen               (0x%08X) : 0x%08x", 2, &mac->txtogen,        mac->txtogen);
    log_hal_msgid_info("mac->usb_l1ints            (0x%08X) : 0x%08x", 2, &mac->usb_l1ints,     mac->usb_l1ints);
    log_hal_msgid_info("mac->usb_l1intm            (0x%08X) : 0x%08x", 2, &mac->usb_l1intm,     mac->usb_l1intm);
    log_hal_msgid_info("mac->usb_l1intp            (0x%08X) : 0x%08x", 2, &mac->usb_l1intp,     mac->usb_l1intp);
    log_hal_msgid_info("mac->usb_l1intc            (0x%08X) : 0x%08x", 2, &mac->usb_l1intc,     mac->usb_l1intc);
    log_hal_msgid_info("mac->csr0                  (0x%08X) : 0x%08x", 2, &mac->csr0,           mac->csr0);
    log_hal_msgid_info("mac->count0                (0x%08X) : 0x%08x", 2, &mac->count0,         mac->count0);
    log_hal_msgid_info("mac->type0                 (0x%08X) : 0x%08x", 2, &mac->type0,          mac->type0);
    log_hal_msgid_info("mac->naklimt0              (0x%08X) : 0x%08x", 2, &mac->naklimt0,       mac->naklimt0);
    log_hal_msgid_info("mac->sramconfigsize        (0x%08X) : 0x%08x", 2, &mac->sramconfigsize, mac->sramconfigsize);
    log_hal_msgid_info("mac->hbconfigdata          (0x%08X) : 0x%08x", 2, &mac->hbconfigdata,   mac->hbconfigdata);
    log_hal_msgid_info("mac->configdata            (0x%08X) : 0x%08x", 2, &mac->configdata,     mac->configdata);
    log_hal_msgid_info("mac->tx1map                (0x%08X) : 0x%08x", 2, &mac->tx1map,         mac->tx1map);
    log_hal_msgid_info("mac->tx1csr                (0x%08X) : 0x%08x", 2, &mac->tx1csr,         mac->tx1csr);
    log_hal_msgid_info("mac->rx1map                (0x%08X) : 0x%08x", 2, &mac->rx1map,         mac->rx1map);
    log_hal_msgid_info("mac->rx1csr                (0x%08X) : 0x%08x", 2, &mac->rx1csr,         mac->rx1csr);
    log_hal_msgid_info("mac->rx1count              (0x%08X) : 0x%08x", 2, &mac->rx1count,       mac->rx1count);
    log_hal_msgid_info("mac->tx1type               (0x%08X) : 0x%08x", 2, &mac->tx1type,        mac->tx1type);
    log_hal_msgid_info("mac->tx1interval           (0x%08X) : 0x%08x", 2, &mac->tx1interval,    mac->tx1interval);
    log_hal_msgid_info("mac->rx1type               (0x%08X) : 0x%08x", 2, &mac->rx1type,        mac->rx1type);
    log_hal_msgid_info("mac->rx1interval           (0x%08X) : 0x%08x", 2, &mac->rx1interval,    mac->rx1interval);
    log_hal_msgid_info("mac->fifosize1             (0x%08X) : 0x%08x", 2, &mac->fifosize1,      mac->fifosize1);
    log_hal_msgid_info("mac->dma_intr_status       (0x%08X) : 0x%08x", 2, &mac->dma_intr_status,       mac->dma_intr_status);
    log_hal_msgid_info("mac->dma_intr_unmask       (0x%08X) : 0x%08x", 2, &mac->dma_intr_unmask,       mac->dma_intr_unmask);
    log_hal_msgid_info("mac->dma_intr_unmask_clear (0x%08X) : 0x%08x", 2, &mac->dma_intr_unmask_clear, mac->dma_intr_unmask_clear);
    log_hal_msgid_info("mac->dma_intr_unmask_set   (0x%08X) : 0x%08x", 2, &mac->dma_intr_unmask_set,   mac->dma_intr_unmask_set);
    log_hal_msgid_info("mac->dma_cntl_0            (0x%08X) : 0x%08x", 2, &mac->dma_cntl_0,     mac->dma_cntl_0);
    log_hal_msgid_info("mac->dma_addr_0            (0x%08X) : 0x%08x", 2, &mac->dma_addr_0,     mac->dma_addr_0);
    log_hal_msgid_info("mac->dma_count_0           (0x%08X) : 0x%08x", 2, &mac->dma_count_0,    mac->dma_count_0);
    log_hal_msgid_info("mac->dma_limiter           (0x%08X) : 0x%08x", 2, &mac->dma_limiter,    mac->dma_limiter);
    log_hal_msgid_info("mac->dma_config            (0x%08X) : 0x%08x", 2, &mac->dma_config,     mac->dma_config);
    log_hal_msgid_info("mac->ep1rxpktcount         (0x%08X) : 0x%08x", 2, &mac->ep1rxpktcount,  mac->ep1rxpktcount);
    log_hal_msgid_info("mac->t0funcaddr            (0x%08X) : 0x%08x", 2, &mac->t0funcaddr,     mac->t0funcaddr);
    log_hal_msgid_info("mac->t0hubaddr             (0x%08X) : 0x%08x", 2, &mac->t0hubaddr,      mac->t0hubaddr);
    log_hal_msgid_info("mac->t1funcaddr            (0x%08X) : 0x%08x", 2, &mac->t1funcaddr,     mac->t1funcaddr);
    log_hal_msgid_info("mac->t1hubaddr             (0x%08X) : 0x%08x", 2, &mac->t1hubaddr,      mac->t1hubaddr);
    log_hal_msgid_info("mac->r1funcaddr            (0x%08X) : 0x%08x", 2, &mac->r1funcaddr,     mac->r1funcaddr);
    log_hal_msgid_info("mac->r1hubaddr             (0x%08X) : 0x%08x", 2, &mac->r1hubaddr,      mac->r1hubaddr);
    log_hal_msgid_info("mac->tm1                   (0x%08X) : 0x%08x", 2, &mac->tm1,            mac->tm1);
    log_hal_msgid_info("mac->hwver_date            (0x%08X) : 0x%08x", 2, &mac->hwver_date,     mac->hwver_date);
    log_hal_msgid_info("mac->srama                 (0x%08X) : 0x%08x", 2, &mac->srama,          mac->srama);
    log_hal_msgid_info("mac->sramd                 (0x%08X) : 0x%08x", 2, &mac->sramd,          mac->sramd);
    log_hal_msgid_info("mac->risc_size             (0x%08X) : 0x%08x", 2, &mac->risc_size,      mac->risc_size);
    log_hal_msgid_info("mac->resreg                (0x%08X) : 0x%08x", 2, &mac->resreg,         mac->resreg);
    log_hal_msgid_info("mac->otg20_csrl            (0x%08X) : 0x%08x", 2, &mac->otg20_csrl,     mac->otg20_csrl);
    log_hal_msgid_info("mac->otg20_csrh            (0x%08X) : 0x%08x", 2, &mac->otg20_csrh,     mac->otg20_csrh);

    log_hal_msgid_info("phy->usbphyacr0            (0x%08x) : 0x%08x", 2, &phy->usbphyacr0, phy->usbphyacr0);
    log_hal_msgid_info("phy->usbphyacr1            (0x%08x) : 0x%08x", 2, &phy->usbphyacr1, phy->usbphyacr1);
    log_hal_msgid_info("phy->usbphyacr2            (0x%08x) : 0x%08x", 2, &phy->usbphyacr2, phy->usbphyacr2);
    log_hal_msgid_info("phy->revered1              (0x%08x) : 0x%08x", 2, &phy->revered1,   phy->revered1);
    log_hal_msgid_info("phy->usbphyacr4            (0x%08x) : 0x%08x", 2, &phy->usbphyacr4, phy->usbphyacr4);
    log_hal_msgid_info("phy->usbphyacr5            (0x%08x) : 0x%08x", 2, &phy->usbphyacr5, phy->usbphyacr5);
    log_hal_msgid_info("phy->usbphyacr6            (0x%08x) : 0x%08x", 2, &phy->usbphyacr6, phy->usbphyacr6);
    log_hal_msgid_info("phy->u2phyacr3             (0x%08x) : 0x%08x", 2, &phy->u2phyacr3,  phy->u2phyacr3);
    log_hal_msgid_info("phy->u2phyacr4             (0x%08x) : 0x%08x", 2, &phy->u2phyacr4,  phy->u2phyacr4);
    log_hal_msgid_info("phy->revered2              (0x%08x) : 0x%08x", 2, &phy->revered2,   phy->revered2);
    log_hal_msgid_info("phy->u2phydcr0             (0x%08x) : 0x%08x", 2, &phy->u2phydcr0,  phy->u2phydcr0);
    log_hal_msgid_info("phy->u2phydcr1             (0x%08x) : 0x%08x", 2, &phy->u2phydcr1,  phy->u2phydcr1);
    log_hal_msgid_info("phy->u2phydtm0             (0x%08x) : 0x%08x", 2, &phy->u2phydtm0,  phy->u2phydtm0);
    log_hal_msgid_info("phy->u2phydtm1             (0x%08x) : 0x%08x", 2, &phy->u2phydtm1,  phy->u2phydtm1);
    log_hal_msgid_info("phy->u2phydmon0            (0x%08x) : 0x%08x", 2, &phy->u2phydmon0, phy->u2phydmon0);
    log_hal_msgid_info("phy->u2phydmon1            (0x%08x) : 0x%08x", 2, &phy->u2phydmon1, phy->u2phydmon1);
    log_hal_msgid_info("phy->u2phydmon2            (0x%08x) : 0x%08x", 2, &phy->u2phydmon2, phy->u2phydmon2);
    log_hal_msgid_info("phy->revered3              (0x%08x) : 0x%08x", 2, &phy->revered3,   phy->revered3);
    log_hal_msgid_info("phy->u2phycr3              (0x%08x) : 0x%08x", 2, &phy->u2phycr3,   phy->u2phycr3);
    log_hal_msgid_info("phy->revered4              (0x%08x) : 0x%08x", 2, &phy->revered4,   phy->revered4);
    log_hal_msgid_info("phy->u2phycr4              (0x%08x) : 0x%08x", 2, &phy->u2phycr4,   phy->u2phycr4);
    log_hal_msgid_info("phy->revered5              (0x%08x) : 0x%08x", 2, &phy->revered5,   phy->revered5);
    log_hal_msgid_info("phy->u1phycr0              (0x%08x) : 0x%08x", 2, &phy->u1phycr0,   phy->u1phycr0);
    log_hal_msgid_info("phy->u1phycr1              (0x%08x) : 0x%08x", 2, &phy->u1phycr1,   phy->u1phycr1);
    log_hal_msgid_info("phy->u1phycr2              (0x%08x) : 0x%08x", 2, &phy->u1phycr2,   phy->u1phycr2);
    log_hal_msgid_info("phy->revered6              (0x%08x) : 0x%08x", 2, &phy->revered6,   phy->revered6);
    log_hal_msgid_info("phy->regfppc               (0x%08x) : 0x%08x", 2, &phy->regfppc,    phy->regfppc);
    log_hal_msgid_info("phy->revered7              (0x%08x) : 0x%08x", 2, &phy->revered7,   phy->revered7);
    log_hal_msgid_info("phy->versionc              (0x%08x) : 0x%08x", 2, &phy->versionc,   phy->versionc);
    log_hal_msgid_info("phy->revered8              (0x%08x) : 0x%08x", 2, &phy->revered8,   phy->revered8);
    log_hal_msgid_info("phy->regfcom               (0x%08x) : 0x%08x", 2, &phy->regfcom,    phy->regfcom);
    log_hal_msgid_info("phy->revered9              (0x%08x) : 0x%08x", 2, &phy->revered9,   phy->revered9);
    log_hal_msgid_info("phy->fmcr0                 (0x%08x) : 0x%08x", 2, &phy->fmcr0,      phy->fmcr0);
    log_hal_msgid_info("phy->fmcr1                 (0x%08x) : 0x%08x", 2, &phy->fmcr1,      phy->fmcr1);
    log_hal_msgid_info("phy->fmcr2                 (0x%08x) : 0x%08x", 2, &phy->fmcr2,      phy->fmcr2);
    log_hal_msgid_info("phy->fmmonr0               (0x%08x) : 0x%08x", 2, &phy->fmmonr0,    phy->fmmonr0);
    log_hal_msgid_info("phy->fmmonr1               (0x%08x) : 0x%08x", 2, &phy->fmmonr1,    phy->fmmonr1);
}
#endif

/************************************************************
    DMA utilities
*************************************************************/
static uint8_t usb_get_dma_channel_num(uint8_t ep_num, hal_usb_endpoint_direction_t direction)
{
    uint8_t dma_chan;
    dma_chan = g_UsbDrvInfo->dma_port[direction][ep_num - 1];

    if ((dma_chan == 0) || (dma_chan > HAL_USB_MAX_NUMBER_DMA) || (ep_num == 0)) {
        log_hal_msgid_error("usb_get_dma_channel_num dma_chan[%x] ep_num[%x] direction[%x] ERROR !!", 3,
                             dma_chan, ep_num, direction);
    }

    return dma_chan;
}

/* Stop DMA channel */
static void usb_hw_stop_dma_channel(uint32_t ep_num, hal_usb_endpoint_direction_t direction)
{
    uint8_t dma_chan;
    uint32_t savedMask;

    dma_chan = usb_get_dma_channel_num(ep_num, direction);

    hal_nvic_save_and_set_interrupt_mask(&savedMask);

    USBDMA_Stop(dma_chan);

    /* Clear pending DMA interrupts */
    DRV_WriteReg8(&musb->dma_intr_status, (1 << (dma_chan - 1)));
    g_UsbDrvInfo->dma_pktrdy[dma_chan - 1] = false;
    g_UsbDrvInfo->dma_running[dma_chan - 1] = false;

    hal_nvic_restore_interrupt_mask(savedMask);
}


/************************************************************
    driver debug utility
*************************************************************/
static void usb_ep_check(uint32_t ep_num, hal_usb_endpoint_direction_t direction, uint32_t line)
{
    if ((ep_num == 0) ||
        ((direction == HAL_USB_EP_DIRECTION_TX) && (ep_num > HAL_USB_MAX_NUMBER_ENDPOINT_TX)) ||
        ((direction == HAL_USB_EP_DIRECTION_RX) && (ep_num > HAL_USB_MAX_NUMBER_ENDPOINT_RX))) {
        log_hal_msgid_info("usb_ep_check ep_num[%x] direction[%x] line[%x] ERROR!!", 3, ep_num, direction, line);
        assert(0 && "USB endpoint number is invalid");
    }
}

static void usb_ep_dma_running_check(uint32_t ep_num, hal_usb_endpoint_direction_t direction, uint32_t line)
{
    uint8_t dma_chan;

    dma_chan = g_UsbDrvInfo->dma_port[direction][ep_num - 1];

    if ((dma_chan != 0) &&
        (USBDMA_CheckRunStat(dma_chan) != 0) &&
        (g_UsbDrvInfo->dma_dir[dma_chan - 1] == direction)) {
        /* Fix for USB compliance test program */
        usb_hw_stop_dma_channel(ep_num, direction);
    }
}

hal_usb_status_t hal_usb_enable_endpoint0(void)
{
    uint32_t savedMask;

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_SetBits(&musb->intrtxe, USB_INTRTX_EP0);
    hal_nvic_restore_interrupt_mask(savedMask);

    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_disable_endpoint0(void)
{
    uint32_t savedMask;

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_ClearBits(&musb->intrtxe, USB_INTRTX_EP0);
    hal_nvic_restore_interrupt_mask(savedMask);

    return HAL_USB_STATUS_OK;
}

/* Clear ep0 interrupt */
hal_usb_status_t hal_usb_clear_ep0_interrupt()
{
    uint32_t savedMask;

    /* w1c */
    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    DRV_WriteReg(&musb->intrtx, USB_INTRTX_EP0);
    hal_nvic_restore_interrupt_mask(savedMask);

    return HAL_USB_STATUS_OK;
}


/* Write data to FIFO EP */
static void usb_hw_epfifowrite(uint32_t ep_num, uint16_t nBytes, void *pSrc)
{
    uint8_t *pby;
    uint16_t nCount = nBytes;
    uint32_t temp_1, temp_2, nAddr;
    uint32_t *pby1;

    if ((nBytes != 0) && (pSrc == NULL)) {
        log_hal_msgid_error("usb_hw_epfifowrite : get data but address is NULL Fail", 0);
        return;
    }

    if (pSrc == NULL) {
        log_hal_msgid_info("usb_hw_epfifowrite : pSrc is NULL Fail", 0);
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
            USB_DRV_WriteReg32(nAddr, *pby1++);    //lint !e613
            temp_1--;
        }

        pby = (uint8_t *)pby1;

        while (temp_2) {
            USB_DRV_WriteReg8(nAddr, *pby++);    //lint !e613
            temp_2--;
        }
    } else {
        pby = (uint8_t *)pSrc;

        /* Read byte by byte */
        while (nCount) {
            USB_DRV_WriteReg8(nAddr, *pby++);    //lint !e613
            nCount--;
        }
    }
}

hal_usb_endpoint_state_t hal_usb_check_rx_endpoint_usage(uint32_t ep_num)
{
    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_RX, __LINE__);

    return g_UsbDrvInfo->ep_rx_enb_state[ep_num - 1];
}

hal_usb_endpoint_state_t hal_usb_check_tx_endpoint_usage(uint32_t ep_num)
{
    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_TX, __LINE__);

    return g_UsbDrvInfo->ep_tx_enb_state[ep_num - 1];
}

hal_usb_status_t hal_usb_configure_rx_endpoint_type(uint32_t ep_num, hal_usb_endpoint_transfer_type_t ep_type, bool b_is_use_dma)
{
    uint32_t savedMask;
    bool b_is_switch_to_dma;

    b_is_switch_to_dma = b_is_use_dma;

    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_RX, __LINE__);
    usb_ep_dma_running_check(ep_num, HAL_USB_EP_DIRECTION_RX, 0);

    if (b_is_switch_to_dma == true) {
        //if(g_UsbDrvInfo->ep_rx_enb_state[ep_num-1] == HAL_USB_EP_STATE_DMA)
        //    log_hal_msgid_info("ASSERT", 0);

        g_UsbDrvInfo->ep_rx_enb_state[ep_num - 1] = HAL_USB_EP_STATE_DMA;

        hal_nvic_save_and_set_interrupt_mask(&savedMask);
        USB_DRV_ClearBits(&musb->intrrxe, (USB_INTRE_EPEN << ep_num));
        USB_DRV_WriteReg8(&musb->index, ep_num);

        /* Must set related DMA bits, otherwise, packets arrive will result in RX ep intr */
        if (ep_type == HAL_USB_EP_TRANSFER_ISO) {
            USB_DRV_WriteReg(&musb->rxcsr, USB_DMA_RX_CSR_ISO | USB_RXCSR_RXPKTRDY);
        } else if (ep_type == HAL_USB_EP_TRANSFER_ISO) {
            USB_DRV_WriteReg(&musb->rxcsr, USB_RXCSR_DISNYET | USB_RXCSR_RXPKTRDY);
        } else {
            USB_DRV_WriteReg(&musb->rxcsr, USB_DMA_RX_CSR | USB_RXCSR_RXPKTRDY);
        }

        hal_nvic_restore_interrupt_mask(savedMask);
    } else {
        //if(g_UsbDrvInfo->ep_rx_enb_state[ep_num-1] == HAL_USB_EP_STATE_FIFO)
        //    log_hal_msgid_info("ASSERT", 0);

        g_UsbDrvInfo->ep_rx_enb_state[ep_num - 1] = HAL_USB_EP_STATE_FIFO;

        hal_nvic_save_and_set_interrupt_mask(&savedMask);
        USB_DRV_WriteReg8(&musb->index, ep_num);

        /* When we change ep state from DMA to FIFO, must make sure that FIFO is empty */
        /* But there may be a null packet coming here in DMA callback */
        if (USB_DRV_Reg(&musb->rxcsr)&USB_RXCSR_RXPKTRDY) {
            if (USB_DRV_Reg(&musb->rxcount) == 0) {
                USB_DRV_ClearBits(&musb->rxcsr, USB_RXCSR_RXPKTRDY);
            } else {
                log_hal_msgid_info("ASSERT", 0);
            }
        }

        if (ep_type == HAL_USB_EP_TRANSFER_ISO) {
            //USB_DRV_WriteReg(USB_RXCSR, (USB_RXCSR_ISO|USB_RXCSR_RXPKTRDY));
            /*It may clear STALL status, need to prevent it.*/
            USB_DRV_WriteReg(&musb->rxcsr, USB_DRV_Reg(&musb->rxcsr) & (USB_RXCSR_ISO | USB_RXCSR_RXPKTRDY | USB_RXCSR_SENDSTALL | USB_RXCSR_SENTSTALL));
        } else if (ep_type == HAL_USB_EP_TRANSFER_ISO) {
            USB_DRV_WriteReg(&musb->rxcsr, USB_DRV_Reg(&musb->rxcsr) & (USB_RXCSR_DISNYET | USB_RXCSR_RXPKTRDY | USB_RXCSR_SENDSTALL | USB_RXCSR_SENTSTALL));
        }  else {
            //USB_DRV_WriteReg(USB_RXCSR, USB_RXCSR_RXPKTRDY);
            /*It may clear STALL status, need to prevent it.*/
            USB_DRV_WriteReg(&musb->rxcsr, USB_DRV_Reg(&musb->rxcsr) & (USB_RXCSR_RXPKTRDY | USB_RXCSR_SENDSTALL | USB_RXCSR_SENTSTALL));
        }

        USB_DRV_SetBits(&musb->intrrxe, (USB_INTRE_EPEN << ep_num));
        hal_nvic_restore_interrupt_mask(savedMask);
    }

    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_configure_tx_endpoint_type(uint32_t ep_num, hal_usb_endpoint_transfer_type_t ep_type, bool b_is_use_dma)
{
    uint32_t savedMask;
    bool b_is_switch_to_dma;

    b_is_switch_to_dma = b_is_use_dma;

    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_TX, __LINE__);
    usb_ep_dma_running_check(ep_num, HAL_USB_EP_DIRECTION_TX, 0);

    if (b_is_switch_to_dma == true) {
        if (g_UsbDrvInfo->ep_tx_enb_state[ep_num - 1] == HAL_USB_EP_STATE_DMA) {
            log_hal_msgid_info("ASSERT", 0);
        }
        g_UsbDrvInfo->ep_tx_enb_state[ep_num - 1] = HAL_USB_EP_STATE_DMA;

        hal_nvic_save_and_set_interrupt_mask(&savedMask);
        USB_DRV_ClearBits(&musb->intrtxe, (USB_INTRE_EPEN << ep_num));
        USB_DRV_WriteReg8(&musb->index, ep_num);

        if (USB_DRV_Reg(&musb->txcsr)&USB_TXCSR_FIFONOTEMPTY) {
            log_hal_msgid_info("ASSERT", 0);
        }

        if (ep_type == HAL_USB_EP_TRANSFER_ISO) {
            USB_DRV_WriteReg(&musb->txcsr, USB_TXCSR_ISO);
        } else {
            USB_DRV_WriteReg(&musb->txcsr, 0x00);
        }

        /* Only is configured as multiple packet DMA TX mode */
        /*
                if(ep_type == USB_ENDPT_ISO)
                {
                    USB_DRV_WriteReg(USB_TXCSR, USB_DMA_TX_CSR_ISO);
                }
                else
                {
                    USB_DRV_WriteReg(USB_TXCSR, USB_DMA_TX_CSR);
                }
        */
        hal_nvic_restore_interrupt_mask(savedMask);
    } else {
        if (g_UsbDrvInfo->ep_tx_enb_state[ep_num - 1] == HAL_USB_EP_STATE_FIFO) {
            log_hal_msgid_info("ASSERT", 0);
        }

        g_UsbDrvInfo->ep_tx_enb_state[ep_num - 1] = HAL_USB_EP_STATE_FIFO;

        hal_nvic_save_and_set_interrupt_mask(&savedMask);
        USB_DRV_WriteReg8(&musb->index, ep_num);

        /* From DMA to FIFO, it may be not empty because this is DMA callback */
        //        if(USB_DRV_Reg(USB_TXCSR)&USB_TXCSR_FIFONOTEMPTY)
        //            ASSERT(0);

        /* When change ep state from DMA to FIFO, must make sure that FIFO is empty */
        if (USB_DRV_Reg(&musb->txcsr)&USB_TXCSR_TXPKTRDY) {
            log_hal_msgid_info("ASSERT", 0);
        }

        if (ep_type == HAL_USB_EP_TRANSFER_ISO) {
            USB_DRV_WriteReg(&musb->txcsr, USB_TXCSR_ISO);
        } else {
            USB_DRV_WriteReg(&musb->txcsr, 0x00);
        }

        USB_DRV_SetBits(&musb->intrtxe, (USB_INTRE_EPEN << ep_num));
        hal_nvic_restore_interrupt_mask(savedMask);
    }

    return HAL_USB_STATUS_OK;
}

/************************************************************
    HISR/LISR   interrupt handler
*************************************************************/
#ifdef HAL_USB_EINT_ENABLE
static void usb_eint_hisr(void)
{
    /*log_hal_msgid_info("usb_eint_hisr", 0);*/
}
#endif /* HAL_USB_EINT_ENABLE */

void usb_hisr(hal_nvic_irq_t irq_number)
{
    uint8_t IntrUSB;
    uint16_t  IntrTx;
    uint16_t  IntrTxE;
    uint16_t  TXCSR;
    uint16_t  RXCSR;
    uint16_t  IntrRx;
    uint16_t  IntrRxE;
    uint32_t  ep_num;
    //uint8_t USBMode;
    uint32_t savedMask;
    uint32_t DMAIntr;
    //uint32_t DMAQIntr;
    static uint32_t index;

    // Write 1 clear
    IntrUSB = musb->intrusb;
    USB_DRV_WriteReg8(&musb->intrusb, IntrUSB);

    // Write 1 clear
    IntrTx = musb->intrtx;
    //If EP0 interrupt isn't enabled, don't clear EP0 interrupt cause.
    IntrTxE =  musb->intrtxe;
    if((IntrTxE & USB_INTRTX_EP0) != USB_INTRTX_EP0){
        IntrTx &= 0xFFFE;
        log_hal_msgid_info("usb_hisr: EP0 Int disabled", 0);
    }
    USB_DRV_WriteReg(&musb->intrtx, IntrTx);

    // Write 1 clear
    IntrRx = musb->intrrx;
    IntrRxE = musb->intrrxe;
    IntrRx = IntrRx & IntrRxE;
    USB_DRV_WriteReg(&musb->intrrx, IntrRx);

    //USBMode = musb->power;

    /* Check for suspend interrupt */
    if (IntrUSB & USB_INTRUSB_SUSPEND) {
        g_UsbDrvInfo->power_state = HAL_USB_POWER_STATE_SUSPEND;
        log_hal_msgid_info("usb_hisr: USB_INTRUSB_SUSPEND", 0);
        hal_usb_do_suspend();
        hal_usbphy_enter_suspend();
        hal_usb_start_gpt_detect_linestate();
    }

    /* Check for resume interrupt */
    if (IntrUSB & USB_INTRUSB_RESUME && g_UsbDrvInfo->power_state == HAL_USB_POWER_STATE_SUSPEND) {
        log_hal_msgid_info("usb_hisr: USB_INTRUSB_RESUME", 0);
        g_UsbDrvInfo->power_state = HAL_USB_POWER_STATE_NORMAL;
        hal_usbphy_leave_suspend();
        hal_usb_do_resume();
    }

    /* Check for reset interrupt */
    if (IntrUSB & USB_INTRUSB_RESET) {
        hal_usbphy_leave_suspend();
        log_hal_msgid_info("usb_hisr: USB_INTRUSB_RESET", 0);
        g_UsbDrvInfo->power_state = HAL_USB_POWER_STATE_NORMAL;
        USB_DRV_WriteReg8(&musb->faddr, 0);

        if (g_UsbDrvInfo->reset_hdlr == NULL) {
            log_hal_msgid_error("usb_hisr : g_UsbDrvInfo->reset_hdlr = NULL Fail", 0);
        } else {
            /* Set SW_RST bit here, so we do not care about any following ep interrupts */
            g_UsbDrvInfo->reset_hdlr();
        }
    }

    /* Check for EP0 interrupt and check EP0 interrupt is enabled */
    IntrTxE =  musb->intrtxe;
    if (IntrTx & USB_INTRTX_EP0) {
        if(IntrTxE & USB_INTRTX_EP0){
        if (g_UsbDrvInfo->ep0_hdlr == NULL) {
            log_hal_msgid_error("usb_hisr : ep0_hdlr = NULL Fail", 0);
        } else {
            g_UsbDrvInfo->ep0_hdlr();
        }
        } else {
            log_hal_msgid_error("usb_hisr: skip ep0 handler", 0);
        }
    }

    /* Check for Bulk TX interrupt */
    /* TX interrupt : 1.TxPktRdy is clear, 2. SENTSTALL is set */
    for (ep_num = 1; ep_num <= HAL_USB_MAX_NUMBER_ENDPOINT_TX; ep_num++) {
        if (IntrTx & (0x01 << ep_num)) {
            hal_nvic_save_and_set_interrupt_mask(&savedMask);
            USB_DRV_WriteReg8(&musb->index, ep_num);
            TXCSR = USB_DRV_Reg(&musb->txcsr);
            hal_nvic_restore_interrupt_mask(savedMask);

            if ((TXCSR & USB_TXCSR_SENTSTALL) != 0) {
                /* Only clear SENTSTALL bit */
                TXCSR &= ~USB_TXCSR_SENTSTALL;
                hal_nvic_save_and_set_interrupt_mask(&savedMask);
                USB_DRV_WriteReg8(&musb->index, ep_num);
                USB_DRV_WriteReg(&musb->txcsr, TXCSR);
                hal_nvic_restore_interrupt_mask(savedMask);
            } else {
                if (g_UsbDrvInfo->ep_tx_hdlr[ep_num - 1] != NULL) {
                    g_UsbDrvInfo->ep_tx_hdlr[ep_num - 1]();
                } else {
                    if (g_UsbDrvInfo->ep_tx_flush_intr[ep_num - 1] == true) {
                        g_UsbDrvInfo->ep_tx_flush_intr[ep_num - 1] = false;
                    }
                }
            }
        }
    }

    /* Check for Bulk RX interrupt */
    /* RX interrupt : 1.RxPktRdy is set, 2. SENTSTALL is set */
    for (ep_num = 1; ep_num <= HAL_USB_MAX_NUMBER_ENDPOINT_RX; ep_num++) {
        if (IntrRx & (0x01 << ep_num)) {
            /*prevent race condition with putisrbyte */
            hal_nvic_save_and_set_interrupt_mask(&savedMask);
            USB_DRV_WriteReg8(&musb->index, ep_num);
            RXCSR = USB_DRV_Reg(&musb->rxcsr);
            hal_nvic_restore_interrupt_mask(savedMask);

            if (g_UsbDrvInfo->power_state != HAL_USB_POWER_STATE_NORMAL) {
                RXCSR = 0x00;
            }

            if ((RXCSR & USB_RXCSR_SENTSTALL) != 0) {
                RXCSR &= ~USB_RXCSR_SENTSTALL;
                /*prevent race condition with putisrbyte */
                hal_nvic_save_and_set_interrupt_mask(&savedMask);
                USB_DRV_WriteReg8(&musb->index, ep_num);
                USB_DRV_WriteReg(&musb->rxcsr, RXCSR);
                hal_nvic_restore_interrupt_mask(savedMask);
            } else if ((RXCSR & USB_RXCSR_RXPKTRDY) != 0) {
                if (g_UsbDrvInfo->ep_rx_hdlr[ep_num - 1] != NULL) {
                    g_UsbDrvInfo->ep_rx_hdlr[ep_num - 1]();
                } else {
                    /* No handler to receive this packet */
                    USB_DRV_ClearBits(&musb->rxcsr, USB_RXCSR_RXPKTRDY);
                }
            } else {
                if (g_UsbDrvInfo->ep_rx_flush_intr[ep_num - 1] == true) {
                    g_UsbDrvInfo->ep_rx_flush_intr[ep_num - 1] = false;
                }
            }
        }
    }

    /* When cable plugged out, it will trigger disconn interrupt */
    if (IntrUSB & USB_INTRUSB_DISCON) {
        log_hal_msgid_info("usb_hisr: USB_INTRUSB_DISCON Fail", 0);
    }

    /*******************/
    /*DMA_HISR        */
    /*******************/
    DMAIntr = USB_DRV_Reg8(&musb->dma_intr_status);
    while (DMAIntr != 0) {
        for (index = 0; index < HAL_USB_MAX_NUMBER_DMA; index++) {
            if ((DMAIntr & (1 << index)) != 0) {
                // write 1 clear first, then do call back function, order is very important !!!
                USB_DRV_WriteReg8(&musb->dma_intr_status, (0x01 << index));

                /* DMA interrupt still occur if stop DMA channel and DMA do a transfer done at the same time.
                 * Must skip it.
                 */
                if (g_UsbDrvInfo->dma_running[index] == true) {
                    /* DMA channel index+1 have interrupt */
                    //if(USB_DRV_Reg32(USB_DMA_REALCNT(index+1)) != 0)
                    if (1) {
                        usb_dma_callback_func(index + 1);
                    } else {
                        if (usb_check_dma_time_out(index + 1) == true) {
                            usb_dma_callback_func(index + 1);
                        } else {
                            log_hal_msgid_error("usb_hisr: dma check Fail", 0);
                        }
                    }
                }
            }
        }

        /* Read register again to see if any interrupt is generated again */
        /* if pong pong buffer serve done, then we do call back function here */
        DMAIntr = USB_DRV_Reg8(&musb->dma_intr_status);
    }
}

hal_usb_status_t hal_usb_drv_create_isr(void)
{
#ifdef HAL_USB_EINT_ENABLE
    hal_eint_config_t eint_config;
    hal_eint_status_t result;
#endif /* HAL_USB_EINT_ENABLE */

    /* interrupt */
    hal_nvic_register_isr_handler((hal_nvic_irq_t)USB_IRQn, usb_hisr);
    hal_nvic_enable_irq(USB_IRQn);

#ifdef HAL_USB_EINT_ENABLE
    /*eint*/
    eint_config.trigger_mode = HAL_EINT_LEVEL_LOW;
    eint_config.debounce_time = 0;

    hal_eint_mask(HAL_EINT_USB);

    result = hal_eint_init(HAL_EINT_USB, &eint_config);
    if (result != HAL_EINT_STATUS_OK) {
        log_hal_msgid_info("hal_usb_drv_create_isr : hal_eint_init %d Fail", 1, result);
        return HAL_USB_STATUS_ERROR;
    }

    result = hal_eint_register_callback((hal_eint_number_t)HAL_EINT_USB, (hal_eint_callback_t)usb_eint_hisr, NULL);
    if (result != HAL_EINT_STATUS_OK) {
        log_hal_msgid_info("hal_usb_drv_create_isr : hal_eint_register_callback %d Fail", 1, result);
        return HAL_USB_STATUS_ERROR;
    }

    //hal_eint_unmask(HAL_EINT_USB);
#endif /* HAL_USB_EINT_ENABLE */

    return HAL_USB_STATUS_OK;
}

/* enable system global interrupt */
static void usb_en_sys_intr(void)
{
    USB_DRV_WriteReg8(&musb->intrusbe, ~USB_INTRUSBE_SOF);
}

/* EP TX data prepared ready, set ready bit */
static void usb_hw_ep_tx_ready(uint32_t ep_num, hal_usb_endpoint_transfer_type_t ep_type)
{
    uint32_t savedMask;

    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_TX, __LINE__);
    usb_ep_dma_running_check(ep_num, HAL_USB_EP_DIRECTION_TX, 0);

    hal_nvic_save_and_set_interrupt_mask(&savedMask);  //DMA_Setup : clear UnderRun bit & Set TX_PkyReady at the same time
    USB_DRV_WriteReg8(&musb->index, ep_num);

    if (USB_DRV_Reg(&musb->txcsr)&USB_TXCSR_TXPKTRDY) { // check TX_PktReady bit
        log_hal_msgid_error("usb_hw_ep_tx_ready : USB_TXCSR_TXPKTRDY Fail", 0);
    }

    USB_DRV_SetBits(&musb->txcsr, USB_TXCSR_TXPKTRDY);
    hal_nvic_restore_interrupt_mask(savedMask);
}

/* EP RX data already read out, clear the data */
static void usb_hw_ep_rx_ready(uint32_t ep_num)
{
    uint32_t savedMask;
    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_RX, __LINE__);

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, ep_num);

    if ((USB_DRV_Reg(&musb->rxcsr)&USB_RXCSR_RXPKTRDY) == 0) { // check RX_PktReady bit
        log_hal_msgid_error("usb_hw_ep_rx_ready : USB_RXCSR_RXPKTRDY Fail", 0);
    }

    USB_DRV_ClearBits(&musb->rxcsr, USB_RXCSR_RXPKTRDY);
    hal_nvic_restore_interrupt_mask(savedMask);
}

/* Get status. See if ep in fifo is empty.
   If false, it means some data in fifo still wait to send out */
static bool usb_hw_is_ep_tx_empty(uint32_t ep_num)
{
    uint16_t CSR;
    uint32_t savedMask;

    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_TX, __LINE__);

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, ep_num);
    CSR = USB_DRV_Reg(&musb->txcsr);
    hal_nvic_restore_interrupt_mask(savedMask);

    if (CSR & USB_TXCSR_FIFONOTEMPTY) {
        return  false;
    } else {
        return  true;
    }
}

/************************************************************
    DMA control functions
*************************************************************/
static void usb_dma_callback_func(uint8_t dma_chan)
{
    hal_usb_dma_handler_t callback = g_UsbDrvInfo->dma_callback[dma_chan - 1];
    uint8_t ep_num;

    if (dma_chan == 0) {
        log_hal_msgid_error("usb_dma_callback_func : dma_chan = 0 Fail", 0);
    }

    g_UsbDrvInfo->dma_callback[dma_chan - 1] = NULL;

    if (g_UsbDrvInfo->dma_dir[dma_chan - 1] == HAL_USB_EP_DIRECTION_TX) {
        ep_num = g_UsbDrvInfo->dma_tx_ep_num[dma_chan - 1];
    } else {
        ep_num = g_UsbDrvInfo->dma_rx_ep_num[dma_chan - 1];
    }

    if (usb_check_dma_time_out(dma_chan) == false) {
        /* ep0 do not do this */
        if (g_UsbDrvInfo->dma_pktrdy[dma_chan - 1] == true) {
            if (g_UsbDrvInfo->dma_dir[dma_chan - 1] == HAL_USB_EP_DIRECTION_TX) {
                usb_hw_ep_tx_ready(ep_num, HAL_USB_EP_TRANSFER_BULK);
            } else if (g_UsbDrvInfo->dma_dir[dma_chan - 1] == HAL_USB_EP_DIRECTION_RX) {
                usb_hw_ep_rx_ready(ep_num);
            }
        }
    }

    g_UsbDrvInfo->dma_pktrdy[dma_chan - 1] = false;

    /* if application callback function does not request to clear drv_running set by itself, clear running state here*/
    if (g_UsbDrvInfo->dma_callback_upd_run[dma_chan - 1] == false) {
        g_UsbDrvInfo->dma_running[dma_chan - 1] = false;
    }

    if (callback != NULL) {
        callback();
    }
}

#ifdef __DMA_UNKNOWN_RX__
static void usb_enable_dma_timer_count(uint8_t dma_chan, bool enable, uint8_t timer_ticks)
{
    if (enable == true) {
        USB_DRV_WriteReg(USB_DMA_TIMER(dma_chan), USB_DMA_TIMER_ENTIMER | (timer_ticks & USB_DMA_TIMER_TIMEOUT_MASK));
    } else {
        // Disable DMA timer
        USB_DRV_WriteReg(USB_DMA_TIMER(dma_chan), 0);
    }
}
#endif

static bool usb_check_dma_time_out(uint8_t dma_chan)
{
    if ((USB_DRV_Reg(USB_DMA_TIMER(dma_chan))&USB_DMA_TIMER_TIMEOUT_STATUS) != 0) {
        return true;
    } else {
        return false;
    }
}

/*******************************************************************************
    USB Power Down Related
 *******************************************************************************/

static void _hal_usb_clr_all_interrupt(void)
{
     if(0==(g_hal_usb_flag&HAL_USB_FLAG__W1C)) {
         USB_DRV_Reg8(&musb->intrusb);
         USB_DRV_Reg(&musb->intrtx);
         USB_DRV_WriteReg(&musb->intrrx, 0xFFFF); // write 1 clear
     } else {
         USB_DRV_WriteReg8(&musb->intrusb,  0xFF); // write 1 clear
         USB_DRV_WriteReg(&musb->intrtx, 0xFFFF); // write 1 clear
         USB_DRV_WriteReg(&musb->intrrx, 0xFFFF); // write 1 clear
     }
}

static hal_usb_status_t hal_usb_pdn_mode(bool pdn_en)
{
    if (pdn_en == true) {
        /* Power down */
        hal_nvic_disable_irq(USB_IRQn);
        USB_DRV_ClearBits8(&musb->power, USB_POWER_SOFTCONN);

        USB_DRV_WriteReg(&musb->intrtxe, 0);
        USB_DRV_WriteReg(&musb->intrrxe, 0);
        USB_DRV_WriteReg8(&musb->intrusbe, 0);
        _hal_usb_clr_all_interrupt();

        hal_usbphy_deinit_case();
    } else {
        /* Enable sequence: 1. UPLL, 2.PDN_USB(USB power), 3.USB(USB register), 4.GPIO(D+) */
        usb_pdn_disable();
        hal_usbphy_poweron_initialize();

        USB_DRV_WriteReg(&musb->intrtxe, 0);
        USB_DRV_WriteReg(&musb->intrrxe, 0);
        USB_DRV_WriteReg8(&musb->intrusbe, 0);
        _hal_usb_clr_all_interrupt();

        USB_DRV_WriteReg8(&musb->devctl, 0x00);
    }
    return HAL_USB_STATUS_OK;
}

/*******************************************************************************
    USB SLT/DVT related
 *******************************************************************************/
bool usb_dma_test_ep0_loopback(uint8_t *tx_buf, uint8_t *rx_buf)
{
    bool cmp_result = true;
    uint8_t dma_chan = 2;
    uint8_t ep_num = 0;
    uint16_t dma_ctrl;
    uint32_t i;
    static uint32_t dma_burst_mode = 0;

    // initial buffer
    for (i = 0; i < 64; i++) {
        tx_buf[i] = i;
        rx_buf[i] = 0;
    }

    /* Disable INTR Setup */
    hal_nvic_disable_irq(USB_IRQn);

    /* Init Setup */
    dma_burst_mode = 0;
    {
        DRV_WriteReg32(USB_DMAADDR(dma_chan), tx_buf);
        DRV_WriteReg32(USB_DMACNT(dma_chan), 64);
        dma_ctrl = USB_DMACNTL_INTEN | (ep_num << 4);
        dma_ctrl |= ((dma_burst_mode & 0x03) << 9) | USB_DMACNTL_DMAEN;
        dma_ctrl |= USB_DMACNTL_DMADIR;
        DRV_WriteReg(USB_DMACNTL(dma_chan), dma_ctrl);
    }

    hal_gpt_delay_ms(20);
    if (DRV_Reg(USB_DMACNTL(dma_chan)) & 0x01) {
        log_hal_msgid_info("usb_dma_test_ep0_loopback: dma fail", 0);
        return false;
    }
    USB_DRV_WriteReg(&musb->csr0, USB_CSR0_TXPKTRDY);
    USB_DRV_WriteReg8(&musb->testmode, USB_TESTMODE_FIFOACCESS | USB_TESTMODE_FORCEHS);

    //RX
    {
        dma_ctrl = 0;
        DRV_WriteReg32(USB_DMAADDR(dma_chan), rx_buf);
        DRV_WriteReg32(USB_DMACNT(dma_chan), 64);
        dma_ctrl = USB_DMACNTL_INTEN | (ep_num << 4);
        dma_ctrl |= ((dma_burst_mode & 0x03) << 9) | USB_DMACNTL_DMAEN;
        DRV_WriteReg(USB_DMACNTL(dma_chan), dma_ctrl);
    }

    hal_gpt_delay_ms(20);
    for (i = 0; i < 64; i++) {
        if (tx_buf[i] != rx_buf[i]) {
            cmp_result = false;
            log_hal_msgid_info("usb_dma_test_ep0_loopback: compare data fail", 0);
            break;
        }
    }
    USB_DRV_WriteReg8(&musb->testmode, 0);
    return cmp_result;
}

hal_usb_status_t hal_usb_dcm_enable(void)
{
    /*
     * bit16: mcu dcm
     * bit18: usbip dcm
     */
    DRV_WriteReg32(&musb->resreg,  musb->resreg & (~(0x05 << 16)));
    log_hal_msgid_info("hal_usb_dcm_enable: 0x%X", 1, musb->resreg);

    return HAL_USB_STATUS_OK;
}

/*******************************************************************************
    Control functions for USB_DRV
 *******************************************************************************/
/* Initialize usb driver SW information, called at USB_Init() */
hal_usb_status_t hal_usb_configure_driver(void)
{
    //log_hal_msgid_info("hal_usb_configure_driver", 0);

    int32_t index;

    g_UsbDrvInfo->power_state = HAL_USB_POWER_STATE_NORMAL;
    g_UsbDrvInfo->is_unMaskUSB = true;
    g_UsbDrvInfo->reset_hdlr = NULL;
    g_UsbDrvInfo->ep0_hdlr = NULL;

    /* Init value can't be set up in USB_init() in order to avoid overlapping,
       after setting by APP layer */
    // g_UsbDrvInfo->hs_mode_enable = true;
    // g_UsbDrvInfo->hs_mode = false;

    for (index = (HAL_USB_MAX_NUMBER_ENDPOINT_TX - 1); index >= 0; index--) {
        g_UsbDrvInfo->ep_tx_enb_state[index] = HAL_USB_EP_STATE_DISABLE;
        g_UsbDrvInfo->ep_tx_hdlr[index] = NULL;
        g_UsbDrvInfo->ep_tx_stall_status[index] = false;
        g_UsbDrvInfo->ep_tx_flush_intr[index] = false;
    }

    for (index = (HAL_USB_MAX_NUMBER_ENDPOINT_RX - 1); index >= 0; index--) {
        g_UsbDrvInfo->ep_rx_enb_state[index] = HAL_USB_EP_STATE_DISABLE;
        g_UsbDrvInfo->ep_rx_hdlr[index] = NULL;
        g_UsbDrvInfo->ep_rx_stall_status[index] = false;
        g_UsbDrvInfo->ep_rx_flush_intr[index] = false;
    }

    for (index = (HAL_USB_MAX_NUMBER_ENDPOINT - 1); index >= 0; index--) {
        g_UsbDrvInfo->dma_port[HAL_USB_EP_DIRECTION_TX][index] = 0;
        g_UsbDrvInfo->dma_port[HAL_USB_EP_DIRECTION_RX][index] = 0;
    }
    g_UsbDrvInfo->dma_channel = 0;

    for (index = (HAL_USB_MAX_NUMBER_DMA - 1); index >= 0; index--) {
        g_UsbDrvInfo->dma_pktrdy[index] = false;
        g_UsbDrvInfo->dma_running[index] = false;
        g_UsbDrvInfo->dma_callback_upd_run[index] = false;
        g_UsbDrvInfo->dma_callback[index] = NULL;
        g_UsbDrvInfo->dma_tx_ep_num[index] = 0;
        g_UsbDrvInfo->dma_rx_ep_num[index] = 0;
    }

    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_set_speed(bool hs_enable)
{
    //log_hal_msgid_info("hal_usb_set_speed hs_enable:%d", 1, hs_enable);

    g_UsbDrvInfo->hs_mode_enable = hs_enable;

    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_set_high_bandwidth(bool hb_enable, uint8_t packet)
{
    g_UsbDrvInfo->hb_mode_enable = hb_enable;

    /* Check USB speed is high */
    if((g_UsbDrvInfo->hb_mode_enable == true) && (g_UsbDrvInfo->hs_mode_enable != true)){
        log_hal_msgid_error("hal_usb_set_high_bandwidth must set USB high speed ERROR !!", 0);
        assert(0);
    }

    /* USB 2.0 Spec Endpoint - wMaxPacketSize 01 = 1 additional (2 per microframe)
     * USB HW RM - maximum amount of data that can be transferred, either 2 or 5
     * e.g. 1152 byte can be divided into 2 packet
            - USB 2.0 Spec : 0x01 means additional 1
            - USB HW RM    : 0x02 means amount 2
     */
    if(packet >= 1){
        g_UsbDrvInfo->hb_mode_multi_packet = packet + 1;
    } else {
        g_UsbDrvInfo->hb_mode_multi_packet = 0;
    }

    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_register_driver_callback(hal_usb_driver_handler_type_t type, uint32_t ep_num, hal_usb_driver_interrupt_handler_t hdlr)
{
    switch (type) {
        case HAL_USB_DRV_HDLR_RESET:
            g_UsbDrvInfo->reset_hdlr = hdlr;
            break;
        case HAL_USB_DRV_HDLR_SUSPEND:
            g_UsbDrvInfo->suspend_hdlr = hdlr;
            break;
        case HAL_USB_DRV_HDLR_RESUME:
            g_UsbDrvInfo->resume_hdlr = hdlr;
            break;
        case HAL_USB_DRV_HDLR_EP0:
            g_UsbDrvInfo->ep0_hdlr = hdlr;
            break;
        case HAL_USB_DRV_HDLR_EP_TX:
            usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_TX, __LINE__);
            g_UsbDrvInfo->ep_tx_hdlr[ep_num - 1] = hdlr;
            break;
        case HAL_USB_DRV_HDLR_EP_RX:
            usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_RX, __LINE__);
            g_UsbDrvInfo->ep_rx_hdlr[ep_num - 1] = hdlr;
            break;
        default:
            log_hal_msgid_error("hal_usb_register_driver_callback : Fail", 0);
            break;
    }

    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_reset_drv_info(void)
{
    int32_t index;
    g_UsbDrvInfo->power_state = HAL_USB_POWER_STATE_NORMAL;
    g_UsbDrvInfo->is_unMaskUSB = true;

    if ((musb->power & USB_POWER_HSMODE) != 0) {
        g_UsbDrvInfo->hs_mode = true;
    } else {
        g_UsbDrvInfo->hs_mode = false;
    }
    //log_hal_msgid_info("hal_usb_reset_drv_info hs_mode:%d", 1, g_UsbDrvInfo->hs_mode);

    for (index = (HAL_USB_MAX_NUMBER_ENDPOINT_TX - 1); index >= 0; index--) {
        g_UsbDrvInfo->ep_tx_enb_state[index] = HAL_USB_EP_STATE_DISABLE;
        g_UsbDrvInfo->ep_tx_stall_status[index] = false;
        g_UsbDrvInfo->ep_tx_flush_intr[index] = false;
    }

    for (index = (HAL_USB_MAX_NUMBER_ENDPOINT_RX - 1); index >= 0; index--) {
        g_UsbDrvInfo->ep_rx_enb_state[index] = HAL_USB_EP_STATE_DISABLE;
        g_UsbDrvInfo->ep_rx_stall_status[index] = false;
        g_UsbDrvInfo->ep_rx_flush_intr[index] = false;
    }

    for (index = (HAL_USB_MAX_NUMBER_DMA - 1); index >= 0; index--) {
        USBDMA_Stop(index + 1);
    }

    //clear interrupt. write 1 clear
    DRV_WriteReg8(&musb->dma_intr_status, 0xFF);
    for (index = (HAL_USB_MAX_NUMBER_DMA - 1); index >= 0; index--) {
        g_UsbDrvInfo->dma_pktrdy[index] = false;
        g_UsbDrvInfo->dma_running[index] = false;
        g_UsbDrvInfo->dma_callback_upd_run[index] = false;
        g_UsbDrvInfo->dma_callback[index] = NULL;
    }

    return HAL_USB_STATUS_OK;
}

bool hal_usb_get_endpoint_stall_status(uint32_t  ep_num, hal_usb_endpoint_direction_t direction)
{
    bool result;

    usb_ep_check(ep_num, direction, __LINE__);
    if (direction == HAL_USB_EP_DIRECTION_TX) {
        result = (bool)g_UsbDrvInfo->ep_tx_stall_status[ep_num - 1];
    } else {
        result = (bool)g_UsbDrvInfo->ep_rx_stall_status[ep_num - 1];
    }

    return result;
}

hal_usb_status_t hal_usb_pull_up_dp_line(void)
{
    /* For   : Clear all interrupt before DP pull high
     * Reason: Windows MTP class won't trigger new reset signal if cable plugout and re-plugin.
     *         We need a reset interrupt to reset IP else force clear all interrupt before DP pull high.
     */
    _hal_usb_clr_all_interrupt();

    /*Pull up DP here!!*/
    USB_DRV_SetBits8(&musb->power, USB_POWER_SOFTCONN);

    g_UsbDrvInfo->usb_disconnect = false;
    log_hal_msgid_info("usb DP pull high", 0);
    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_reset_hardware(void)
{
    //log_hal_msgid_info("hal_usb_reset_hardware hs_mode_enable:%d", 1, g_UsbDrvInfo->hs_mode_enable);

    /* Enable software reset, USB IP only can be reset by SW when detecting reset signal from bus */
    USB_DRV_WriteReg(&musb->busperf3, (USB_BUSPERF3_DISUSBRESET | USB_BUSPERF3_SWRST));

    USB_DRV_WriteReg8(&musb->rst_info, 0x0A0);
    /* Because softconn has be decided to set or not */
    /* Set USB_POWER_ISOUPDATE will make ISO pipe with DMA abnormally */
    USB_DRV_SetBits8(&musb->power, (USB_POWER_ENABLESUSPENDM));

    /* Select HS or FS mode */
    if (g_UsbDrvInfo->hs_mode_enable) {
        USB_DRV_SetBits8(&musb->power, USB_POWER_HSENAB);
    } else {
        USB_DRV_ClearBits8(&musb->power, USB_POWER_HSENAB);
    }

    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_deinit(void)
{
    hal_usb_pdn_mode(true);
    usb_pdn_enable();
    g_UsbDrvInfo->is_initialized = false;
    return HAL_USB_STATUS_OK;
}


hal_usb_status_t hal_usb_set_address(uint8_t addr, hal_usb_set_address_state_t state)
{
    if (state == HAL_USB_SET_ADDR_STATUS) {
        USB_DRV_WriteReg8(&musb->faddr, addr);
    }

    return HAL_USB_STATUS_OK;
}

static uint32_t _hal_usb_init_endpoint_helper(bool rx_dir, uint32_t size, bool double_fifo, uint32_t line)
{
    uint8_t fifosz_flag;
    if(double_fifo) {
        fifosz_flag = USB_FIFOSZ_DPB;
    } else {
        fifosz_flag = 0;
    }

    // convert to MaxPacketSize format to uint16
    uint16_t tmp  = size >> USB_RXMAP_M1_POS;
    size = (tmp+1) * (size & USB_RXMAP_MAX_PAYLOAD_MASK);

    // alignment: Power of two
    if(size>4096) {
        assert(0);
    } else if(size>2048) {
        size = 4096;
        fifosz_flag |= USB_FIFOSZ_SIZE_4096;
    } else if(size>1024) {
        size = 2048;
        fifosz_flag |= USB_FIFOSZ_SIZE_2048;
    } else if(size>512) {
        size = 1024;
        fifosz_flag |= USB_FIFOSZ_SIZE_1024;
    } else if(size>256) {
        size = 512;
        fifosz_flag |= USB_FIFOSZ_SIZE_512;
    } else if(size>128) {
        size = 256;
        fifosz_flag |= USB_FIFOSZ_SIZE_256;
    } else if(size>64) {
        size = 128;
        fifosz_flag |= USB_FIFOSZ_SIZE_128;
    } else if(size>32) {
        size = 64;
        fifosz_flag |= USB_FIFOSZ_SIZE_64;
    } else if(size>16) {
        size = 32;
        fifosz_flag |= USB_FIFOSZ_SIZE_32;
    } else if(size>8) {
        size = 16;
        fifosz_flag |= USB_FIFOSZ_SIZE_16;
    } else {
        size = 8;
        fifosz_flag |= USB_FIFOSZ_SIZE_8;
    }

    if( rx_dir ) {
        USB_DRV_WriteReg8(&musb->rxfifosz, fifosz_flag);
        USB_DRV_WriteReg(&musb->rxfifoadd, (g_FIFOadd / 8));
    } else {
        USB_DRV_WriteReg8(&musb->txfifosz, fifosz_flag);
        USB_DRV_WriteReg(&musb->txfifoadd, (g_FIFOadd / 8));
    }

    g_FIFOadd += size;
    if(double_fifo) {
        g_FIFOadd += size;
    }

    if (g_FIFOadd > USB_MAX_FIFO_SIZE) {
        log_hal_msgid_error("hal_usb_setup_rxfifo beyond USB_MAX_FIFO_SIZE Fail in %d", 1, line);
        assert(0);
    }

    return size;
}

#define UNUSED(p) ((void)(p))
bool g_USB_dynamic_fifo = 1;

hal_usb_status_t hal_usb_init_txmap(uint32_t ep_num, uint16_t data_size)
{
    uint16_t map = data_size & USB_RXMAP_MAX_PAYLOAD_MASK;
    assert(1024>map);
    uint16_t m1 = data_size >> USB_RXMAP_M1_POS;

    if(m1) {
        assert(g_UsbDrvInfo->hs_mode_enable==true);
        map |= ((m1+1) << USB_RXMAP_M1_POS) & USB_RXMAP_M1_MASK;
    }

    log_hal_msgid_info("hal_usb_init_txmap ep_num:%d txmap:0x%x", 2, ep_num, map);

    uint32_t savedMask;
    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, ep_num);

    /* Double buffers, so flush twice */
    USB_DRV_WriteReg(&musb->txcsr, 0x00);
    USB_DRV_WriteReg(&musb->txcsr, (USB_TXCSR_FLUSHFIFO | USB_TXCSR_CLRDATATOG));
    USB_DRV_WriteReg(&musb->txcsr, (USB_TXCSR_FLUSHFIFO | USB_TXCSR_CLRDATATOG));

    USB_DRV_WriteReg(&musb->txmap, map);

    hal_nvic_restore_interrupt_mask(savedMask);

    return 0;
}

hal_usb_status_t hal_usb_init_tx_endpoint(uint32_t ep_num, uint16_t data_size, hal_usb_endpoint_transfer_type_t type, bool double_fifo)
{
    uint32_t savedMask;
    uint16_t fifo_size = 0;
    UNUSED(fifo_size);

    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_TX, __LINE__);
    usb_ep_dma_running_check(ep_num, HAL_USB_EP_DIRECTION_TX, 0);

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, ep_num);
    /* Double buffers, so flush twice */
    USB_DRV_WriteReg(&musb->txcsr, 0x00);
    USB_DRV_WriteReg(&musb->txcsr, (USB_TXCSR_FLUSHFIFO | USB_TXCSR_CLRDATATOG));
    USB_DRV_WriteReg(&musb->txcsr, (USB_TXCSR_FLUSHFIFO | USB_TXCSR_CLRDATATOG));

    // txmap will reset in hal_usb_init_txmap
    uint16_t map = data_size & USB_RXMAP_MAX_PAYLOAD_MASK;
    assert(1024>map);
    USB_DRV_WriteReg(&musb->txmap, map);

    if(g_UsbDrvInfo->hs_mode_enable){
        USB_DRV_WriteReg8(&musb->txtype, USB_TXTYPE_HIGH_SPEED | (type<<4) | (ep_num));
    } else {
        USB_DRV_WriteReg8(&musb->txtype, USB_TXTYPE_FULL_SPEED | (type<<4) | (ep_num));
    }

    bool rx_dir = 0;
    if( g_USB_dynamic_fifo) {
        /* Set FIFO address here */
        if (type == HAL_USB_EP_TRANSFER_BULK) {
            fifo_size = _hal_usb_init_endpoint_helper(rx_dir, data_size, double_fifo,__LINE__);
        } else if (type == HAL_USB_EP_TRANSFER_INTR) {
            fifo_size = _hal_usb_init_endpoint_helper(rx_dir, data_size, double_fifo,__LINE__);
        } else if (type == HAL_USB_EP_TRANSFER_ISO) {
            fifo_size = _hal_usb_init_endpoint_helper(rx_dir, data_size, double_fifo,__LINE__);
        } else {
            log_hal_msgid_error("hal_usb_init_tx_endpoint : illegal pipe", 0);
        }
    } else {
        /* Set FIFO address here */
        if (type == HAL_USB_EP_TRANSFER_BULK) {
            fifo_size = _hal_usb_init_endpoint_helper(rx_dir, USB_BULK_FIFO_UNIT_SIZE, double_fifo,__LINE__);
        } else if (type == HAL_USB_EP_TRANSFER_INTR) {
            fifo_size = _hal_usb_init_endpoint_helper(rx_dir, USB_INTR_FIFO_UNIT_SIZE, double_fifo,__LINE__);
        } else if (type == HAL_USB_EP_TRANSFER_ISO) {
            fifo_size = _hal_usb_init_endpoint_helper(rx_dir, USB_ISO_TX_FIFO_UNIT_SIZE, double_fifo,__LINE__);
        } else {
            log_hal_msgid_error("hal_usb_init_tx_endpoint : illegal pipe", 0);
        }
    }

    hal_nvic_restore_interrupt_mask(savedMask);

    log_hal_msgid_info("hal_usb_init_tx_endpoint %d,%d ep:0x%x fifo_used: %d", 4, data_size, fifo_size, ep_num, g_FIFOadd);

    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_init_rxmap(uint32_t ep_num, uint16_t data_size)
{
    log_hal_msgid_info("hal_usb_init_rxmap ep_num:%d data_size:%d", 2, ep_num, data_size);

    uint16_t rxmap = 0;
    uint32_t savedMask;

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, ep_num);

    /* RXMAP */
    rxmap |= data_size;
    rxmap |= ((g_UsbDrvInfo->hb_mode_multi_packet << USB_RXMAP_M1_POS) & USB_RXMAP_M1_MASK);
    USB_DRV_WriteReg(&musb->rxmap, rxmap);

    hal_nvic_restore_interrupt_mask(savedMask);

    return 0;
}

hal_usb_status_t hal_usb_init_rx_endpoint(uint32_t ep_num, uint16_t data_size, hal_usb_endpoint_transfer_type_t type, bool double_fifo)
{
    uint16_t rxmap = 0;
    uint16_t fifo_size = 0;
    UNUSED(fifo_size);
    uint32_t savedMask;

    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_RX, __LINE__);
    usb_ep_dma_running_check(ep_num, HAL_USB_EP_DIRECTION_RX, 0);

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, ep_num);
    USB_DRV_WriteReg(&musb->rxcsr, USB_RXCSR_RXPKTRDY);
    USB_DRV_WriteReg(&musb->rxcsr, (USB_RXCSR_FLUSHFIFO | USB_RXCSR_CLRDATATOG | USB_RXCSR_RXPKTRDY));
    USB_DRV_WriteReg(&musb->rxcsr, (USB_RXCSR_FLUSHFIFO | USB_RXCSR_CLRDATATOG | USB_RXCSR_RXPKTRDY));

    /* RXMAP */
    rxmap |= data_size;
    rxmap |= ((g_UsbDrvInfo->hb_mode_multi_packet << USB_RXMAP_M1_POS) & USB_RXMAP_M1_MASK);
    USB_DRV_WriteReg(&musb->rxmap, rxmap);

    /* RXTYPE Speed */
    if(g_UsbDrvInfo->hs_mode_enable){
        USB_DRV_WriteReg8(&musb->rxtype, USB_RXTYPE_HIGH_SPEED | (type<<4) | (ep_num));
    } else {
        USB_DRV_WriteReg8(&musb->rxtype, USB_RXTYPE_FULL_SPEED | (type<<4) | (ep_num));
    }

    /* Set FIFO address here */
    bool rx_dir = 1;
    if( g_USB_dynamic_fifo)
    {
        if (type == HAL_USB_EP_TRANSFER_BULK) {
            fifo_size = _hal_usb_init_endpoint_helper(rx_dir, data_size, double_fifo,__LINE__);
        } else if (type == HAL_USB_EP_TRANSFER_ISO) {
            if (double_fifo == true) {
                fifo_size = _hal_usb_init_endpoint_helper(rx_dir, data_size, 1,__LINE__);
            } else {
                /* single buffer, high band-width mode can't use double buffer */
                fifo_size = _hal_usb_init_endpoint_helper(rx_dir, data_size, 0,__LINE__);
            }
        } else if (type == HAL_USB_EP_TRANSFER_INTR) {
            fifo_size = _hal_usb_init_endpoint_helper(rx_dir, data_size, double_fifo,__LINE__);
        } else {
            log_hal_msgid_error("hal_usb_init_rx_endpoint : illegal pipe", 0);
        }
    } else {
        if (type == HAL_USB_EP_TRANSFER_BULK) {
            fifo_size = _hal_usb_init_endpoint_helper(rx_dir, USB_BULK_FIFO_UNIT_SIZE, double_fifo,__LINE__);
        } else if (type == HAL_USB_EP_TRANSFER_ISO) {
            if (double_fifo == true) {
                fifo_size = _hal_usb_init_endpoint_helper(rx_dir, USB_ISO_RX_FIFO_UNIT_SIZE, 1,__LINE__);
            } else {
                /* single buffer, high band-width mode can't use double buffer */
                if(g_UsbDrvInfo->hb_mode_enable == true){
                    fifo_size = _hal_usb_init_endpoint_helper(rx_dir, USB_ISO_RX_HB_FIFO_UNIT_SIZE, 0,__LINE__);
                } else {
                    fifo_size = _hal_usb_init_endpoint_helper(rx_dir, USB_ISO_RX_FIFO_UNIT_SIZE, 0,__LINE__);
                }
            }
        } else if (type == HAL_USB_EP_TRANSFER_INTR) {
            fifo_size = _hal_usb_init_endpoint_helper(rx_dir, USB_INTR_FIFO_UNIT_SIZE, double_fifo,__LINE__);
        } else {
            log_hal_msgid_error("hal_usb_init_rx_endpoint : illegal pipe", 0);
        }
    }

    hal_nvic_restore_interrupt_mask(savedMask);

    log_hal_msgid_info("hal_usb_init_rx_endpoint %d,%d ep:0x%x fifo_used: %d", 4, data_size, fifo_size, ep_num, g_FIFOadd);

    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_enable_tx_endpoint(uint32_t ep_num, hal_usb_endpoint_transfer_type_t ep_type, hal_usb_dma_usage_t dma_usage_type, bool is_flush)
{
    uint32_t savedMask;

    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_TX, __LINE__);
    usb_ep_dma_running_check(ep_num, HAL_USB_EP_DIRECTION_TX, 0);

    hal_nvic_save_and_set_interrupt_mask(&savedMask);

    USB_DRV_WriteReg8(&musb->index, ep_num);

    /* Double buffers, so we flush twice */
    if (is_flush == true) {
        if ((USB_DRV_Reg(&musb->txcsr)&USB_TXCSR_FIFONOTEMPTY) != 0) {
            g_UsbDrvInfo->ep_tx_flush_intr[ep_num - 1] = true;
        }
        USB_DRV_WriteReg(&musb->txcsr, 0x00);
        USB_DRV_WriteReg(&musb->txcsr, (USB_TXCSR_FLUSHFIFO | USB_TXCSR_CLRDATATOG));
        USB_DRV_WriteReg(&musb->txcsr, (USB_TXCSR_FLUSHFIFO | USB_TXCSR_CLRDATATOG));
    }

    if (dma_usage_type == HAL_USB_EP_USE_ONLY_DMA) {
        g_UsbDrvInfo->ep_tx_enb_state[ep_num - 1] = HAL_USB_EP_STATE_DMA;
        USB_DRV_ClearBits(&musb->intrtxe, (USB_INTRE_EPEN << ep_num));

        /* Only is configured as multiple packet DMA TX mode */
        if (ep_type == HAL_USB_EP_TRANSFER_ISO) {
            USB_DRV_WriteReg(&musb->txcsr, USB_DMA_TX_CSR_ISO);
        } else {
            USB_DRV_WriteReg(&musb->txcsr, USB_DMA_TX_CSR);
        }
    } else if (dma_usage_type == HAL_USB_EP_USE_NO_DMA) {
        /* EP default uses FIFO */
        g_UsbDrvInfo->ep_tx_enb_state[ep_num - 1] = HAL_USB_EP_STATE_FIFO;
        if (ep_type == HAL_USB_EP_TRANSFER_ISO) {
            USB_DRV_WriteReg(&musb->txcsr, USB_TXCSR_ISO);
        } else {
            USB_DRV_WriteReg(&musb->txcsr, 0x00);
        }
        USB_DRV_SetBits(&musb->intrtxe, (USB_INTRE_EPEN << ep_num));
    }

    hal_nvic_restore_interrupt_mask(savedMask);
    return HAL_USB_STATUS_OK;
}

uint8_t tx_toggle_token = 0;
hal_usb_status_t hal_usb_set_endpoint_tx_ready_toggle(uint32_t ep_num)
{
    uint32_t savedMask;

    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_TX, __LINE__);
    usb_ep_dma_running_check(ep_num, HAL_USB_EP_DIRECTION_TX, 0);

    hal_nvic_save_and_set_interrupt_mask(&savedMask);  //DMA_Setup : clear UnderRun bit & Set TX_PkyReady at the same time
    USB_DRV_WriteReg8(&musb->index, ep_num);

    /* Enable TX toggle bit*/
    USB_DRV_WriteReg(&musb->txtogen, 1 << ep_num);

    /* Set TX toggle bit */
    if (tx_toggle_token & (1 << ep_num)) {
        USB_DRV_WriteReg(&musb->txtog, 1 << ep_num);
        tx_toggle_token &= ~(1 << ep_num);
    } else {
        USB_DRV_WriteReg(&musb->txtog, 0x00);
        tx_toggle_token |= 1 << ep_num;
    }

    if (USB_DRV_Reg(&musb->txcsr)&USB_TXCSR_TXPKTRDY) { // check TX_PktReady bit
        //log_hal_msgid_error("hal_usb_set_endpoint_tx_ready : Fail", 0);
    }

    USB_DRV_SetBits(&musb->txcsr, USB_TXCSR_TXPKTRDY);
    hal_nvic_restore_interrupt_mask(savedMask);

    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_disable_tx_endpoint(uint32_t ep_num)
{
    uint32_t savedMask;

    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_TX, __LINE__);
    usb_ep_dma_running_check(ep_num, HAL_USB_EP_DIRECTION_TX, 0);

    g_UsbDrvInfo->ep_tx_enb_state[ep_num - 1] = HAL_USB_EP_STATE_DISABLE;

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_ClearBits(&musb->intrtxe, (USB_INTRE_EPEN << ep_num));

    hal_nvic_restore_interrupt_mask(savedMask);

    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_enable_rx_endpoint(uint32_t ep_num, hal_usb_endpoint_transfer_type_t ep_type, hal_usb_dma_usage_t dma_usage_type, bool is_flush)
{
    uint32_t savedMask;

    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_RX, __LINE__);
    usb_ep_dma_running_check(ep_num, HAL_USB_EP_DIRECTION_RX, 0);

    hal_nvic_save_and_set_interrupt_mask(&savedMask);

    if (dma_usage_type == HAL_USB_EP_USE_ONLY_DMA) {
        g_UsbDrvInfo->ep_rx_enb_state[ep_num - 1] = HAL_USB_EP_STATE_DMA;
        USB_DRV_ClearBits(&musb->intrrxe, (USB_INTRE_EPEN << ep_num));
    } else if (dma_usage_type == HAL_USB_EP_USE_NO_DMA) {
        /* EP default uses FIFO */
        g_UsbDrvInfo->ep_rx_enb_state[ep_num - 1] = HAL_USB_EP_STATE_FIFO;
        USB_DRV_SetBits(&musb->intrrxe, (USB_INTRE_EPEN << ep_num));
    }

    hal_nvic_restore_interrupt_mask(savedMask);

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, ep_num);

    /* Maybe double buffer, so flush twice */
    if (is_flush == true) {
        USB_DRV_WriteReg(&musb->rxcsr, USB_RXCSR_RXPKTRDY);
        USB_DRV_WriteReg(&musb->rxcsr, (USB_RXCSR_FLUSHFIFO | USB_RXCSR_CLRDATATOG | USB_RXCSR_RXPKTRDY));
        USB_DRV_WriteReg(&musb->rxcsr, (USB_RXCSR_FLUSHFIFO | USB_RXCSR_CLRDATATOG | USB_RXCSR_RXPKTRDY));
    }

    if (ep_type == HAL_USB_EP_TRANSFER_ISO) {
        USB_DRV_WriteReg(&musb->rxcsr, USB_RXCSR_ISO);
    } else if (ep_type == HAL_USB_EP_TRANSFER_INTR) {
        USB_DRV_WriteReg(&musb->rxcsr, USB_RXCSR_DISNYET);
    } else {
        USB_DRV_WriteReg(&musb->rxcsr, 0x00);
    }

    hal_nvic_restore_interrupt_mask(savedMask);
    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_disable_rx_endpoint(uint32_t ep_num)
{
    uint32_t savedMask;

    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_RX, __LINE__);
    usb_ep_dma_running_check(ep_num, HAL_USB_EP_DIRECTION_RX, 0);

    g_UsbDrvInfo->ep_rx_enb_state[ep_num - 1] = HAL_USB_EP_STATE_DISABLE;

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_ClearBits(&musb->intrrxe, (USB_INTRE_EPEN << ep_num));
    hal_nvic_restore_interrupt_mask(savedMask);

    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_clear_tx_endpoint_data_toggle(uint32_t ep_num)
{
    uint32_t savedMask;

    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_TX, __LINE__);
    usb_ep_dma_running_check(ep_num, HAL_USB_EP_DIRECTION_TX, 0);

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, ep_num);
    USB_DRV_SetBits(&musb->txcsr, USB_TXCSR_CLRDATATOG);
    hal_nvic_restore_interrupt_mask(savedMask);

    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_clear_rx_endpoint_data_toggle(uint32_t ep_num)
{
    uint32_t savedMask;

    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_RX, __LINE__);

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, ep_num);
    USB_DRV_SetBits(&musb->rxcsr, USB_RXCSR_CLRDATATOG);
    hal_nvic_restore_interrupt_mask(savedMask);

    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_read_endpoint_fifo(uint32_t ep_num, uint16_t nBytes, void *pDst)
{
    uint8_t *pby;
    uint16_t nCount;
    uint32_t nAddr;
#ifdef FOUR_BYTE_ACCESS
    uint32_t temp_1, temp_2;
    uint32_t *pby1;
#endif

    nCount = nBytes;

    if ((nBytes != 0) && (pDst == NULL)) {
        log_hal_msgid_error("hal_usb_read_endpoint_fifo : Get data but address is NULL Fail", 0);
    }

    if (pDst == NULL) {
        return HAL_USB_STATUS_ERROR;
    }

    /* not indexed register */
    nAddr = (uint32_t)(&musb->fifo0 + ep_num);

#ifdef FOUR_BYTE_ACCESS
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
    } else
#endif
    {
        pby = (uint8_t *)pDst;

        /* Read byte by byte */
        while (nCount) {
            *pby++ = USB_DRV_Reg8(nAddr);    //lint !e613
            nCount--;
        }
    }

    //fifo dump
    //if(nBytes>10)
    //    log_hal_dump("hal_usb_fifo dump", pDst, nBytes);

    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_write_endpoint_fifo(uint32_t ep_num, uint16_t nBytes, void *pSrc)
{
    usb_hw_epfifowrite(ep_num, nBytes, pSrc);
    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_set_endpoint_stall(uint32_t ep_num, hal_usb_endpoint_direction_t direction, bool stall_enable)
{
    uint32_t savedMask;

    usb_ep_check(ep_num, direction, __LINE__);
    usb_ep_dma_running_check(ep_num, direction, 0);

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, ep_num);

    /* Stall endpoint */
    if (direction == HAL_USB_EP_DIRECTION_RX) {
        USB_DRV_WriteReg(&musb->rxcsr, USB_RXCSR_RXPKTRDY);
        USB_DRV_WriteReg(&musb->rxcsr, (USB_RXCSR_FLUSHFIFO | USB_RXCSR_CLRDATATOG | USB_RXCSR_RXPKTRDY));
        USB_DRV_WriteReg(&musb->rxcsr, (USB_RXCSR_FLUSHFIFO | USB_RXCSR_CLRDATATOG | USB_RXCSR_RXPKTRDY));
        g_UsbDrvInfo->ep_rx_stall_status[ep_num - 1] = stall_enable;

        if (stall_enable == true) {
            USB_DRV_WriteReg(&musb->rxcsr, USB_RXCSR_SENDSTALL);
        }
    } else {
        /* For webcam case, when PC send clear featuere, we must reset toggle */
        if ((USB_DRV_Reg(&musb->txcsr)&USB_TXCSR_FIFONOTEMPTY) != 0) {
            g_UsbDrvInfo->ep_tx_flush_intr[ep_num - 1] = true;
        }

        USB_DRV_WriteReg(&musb->txcsr, 0x00);
        USB_DRV_WriteReg(&musb->txcsr, (USB_TXCSR_FLUSHFIFO | USB_TXCSR_CLRDATATOG));
        USB_DRV_WriteReg(&musb->txcsr, (USB_TXCSR_FLUSHFIFO | USB_TXCSR_CLRDATATOG));
        g_UsbDrvInfo->ep_tx_stall_status[ep_num - 1] = stall_enable;

        if (stall_enable == true) {
            USB_DRV_WriteReg(&musb->txcsr, USB_TXCSR_SENDSTALL);
        }
    }

    hal_nvic_restore_interrupt_mask(savedMask);

    return HAL_USB_STATUS_OK;
}


hal_usb_status_t hal_usb_get_endpoint_0_status(bool *p_transaction_end, bool *p_sent_stall)
{
    uint16_t CSR0;
    uint32_t savedMask;

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, 0);
    CSR0 = USB_DRV_Reg(&musb->csr0);
    hal_nvic_restore_interrupt_mask(savedMask);

    if (CSR0 & USB_CSR0_SENTSTALL) {
        *p_sent_stall = true;
    } else {
        *p_sent_stall = false;
    }

    if (CSR0 & USB_CSR0_SETUPEND) {
        *p_transaction_end = true;
    } else {
        *p_transaction_end = false;
    }

    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_update_endpoint_0_state(hal_usb_endpoint_0_driver_state_t state, bool stall, bool end)
{
    uint8_t reg_state;
    uint16_t IntrTxE;
    uint32_t savedMask;

    /* Clear sent stall */
    if (state == HAL_USB_EP0_DRV_STATE_CLEAR_SENT_STALL) {
        hal_nvic_save_and_set_interrupt_mask(&savedMask);
        USB_DRV_WriteReg8(&musb->index, 0);
        USB_DRV_ClearBits(&musb->csr0, USB_CSR0_SENTSTALL);
        hal_nvic_restore_interrupt_mask(savedMask);
        return HAL_USB_STATUS_OK;
    }

    /* Clear transaction end*/
    if (state == HAL_USB_EP0_DRV_STATE_TRANSACTION_END) {
        hal_nvic_save_and_set_interrupt_mask(&savedMask);
        USB_DRV_WriteReg8(&musb->index, 0);

        /* Only clear EP0 status during EP0 in usage */
        IntrTxE =  musb->intrtxe;
        if(IntrTxE & USB_INTRTX_EP0){
        USB_DRV_WriteReg(&musb->csr0, USB_CSR0_SERVICEDSETUPEND);
        } else {
            log_hal_msgid_error("hal_usb_update_endpoint_0_state skip clear TRANSACTION_END", 0);
        }

        hal_nvic_restore_interrupt_mask(savedMask);
        return HAL_USB_STATUS_OK;
    }

    /* ep0 read end or write ready*/
    if (state == HAL_USB_EP0_DRV_STATE_READ_END) {
        reg_state = USB_CSR0_SERVICEDRXPKTRDY;
    } else {
        reg_state = USB_CSR0_TXPKTRDY;
    }

    /* error occured, sent stall*/
    if (stall == true) {
        reg_state |= USB_CSR0_SENDSTALL;
    }

    /* last data for this transaction, set data end bit*/
    if (end == true) {
        reg_state |= USB_CSR0_DATAEND;
    }

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, 0);
    USB_DRV_WriteReg(&musb->csr0, reg_state);
    hal_nvic_restore_interrupt_mask(savedMask);

    /* Stall log */
    // if (stall == true) {
    //     log_hal_msgid_error("hal_usb_update_endpoint_0_state reg_state:%X csr0:0x%X in stall", 2, reg_state, musb->csr0);
    // }

    return HAL_USB_STATUS_OK;
}

uint32_t hal_usb_ep0_pkt_len(void)
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

hal_usb_status_t hal_usb_set_endpoint_tx_ready(uint32_t ep_num)
{
    uint32_t savedMask;

    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_TX, __LINE__);
    usb_ep_dma_running_check(ep_num, HAL_USB_EP_DIRECTION_TX, 0);

    hal_nvic_save_and_set_interrupt_mask(&savedMask);  //DMA_Setup : clear UnderRun bit & Set TX_PkyReady at the same time
    USB_DRV_WriteReg8(&musb->index, ep_num);

    if (USB_DRV_Reg(&musb->txcsr)&USB_TXCSR_TXPKTRDY) { // check TX_PktReady bit
        //log_hal_msgid_error("hal_usb_set_endpoint_tx_ready : Fail", 0);
    }

    USB_DRV_SetBits(&musb->txcsr, USB_TXCSR_TXPKTRDY);
    hal_nvic_restore_interrupt_mask(savedMask);

    return HAL_USB_STATUS_OK;
}

uint32_t hal_usb_get_rx_packet_length(uint32_t ep_num)
{
    uint16_t CSR;
    uint32_t savedMask;
    uint32_t nCount = 0;

    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_RX, __LINE__);

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, ep_num);
    CSR = USB_DRV_Reg(&musb->rxcsr);

    if (CSR & USB_RXCSR_RXPKTRDY) {
        nCount = (uint32_t)USB_DRV_Reg(&musb->rxcount);
    }
    hal_nvic_restore_interrupt_mask(savedMask);

    return nCount;
}

hal_usb_status_t hal_usb_set_endpoint_rx_ready(uint32_t ep_num)
{
    uint32_t savedMask;

    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_RX, __LINE__);

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, ep_num);
    USB_DRV_ClearBits(&musb->rxcsr, USB_RXCSR_RXPKTRDY);
    USB_DRV_ClearBits(&musb->rxcsr, USB_RXCSR_DMAREQENAB);
    hal_nvic_restore_interrupt_mask(savedMask);

    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_set_endpoint_txrx_ready(uint32_t ep_num)
{
    uint32_t savedMask;

    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_RX, __LINE__);
    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_TX, __LINE__);

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, ep_num);
    USB_DRV_ClearBits(&musb->rxcsr, USB_RXCSR_RXPKTRDY);
    USB_DRV_WriteReg8(&musb->index, ep_num);
    USB_DRV_SetBits(&musb->txcsr, USB_TXCSR_TXPKTRDY);
    hal_nvic_restore_interrupt_mask(savedMask);

    return HAL_USB_STATUS_OK;
}

bool hal_usb_is_endpoint_tx_empty(uint32_t ep_num)
{
    return usb_hw_is_ep_tx_empty(ep_num);
}

bool hal_usb_is_endpoint_rx_empty(uint32_t ep_num)
{
    bool result;
    uint16_t CSR;
    uint32_t savedMask;

    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_RX, __LINE__);

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, ep_num);
    CSR = USB_DRV_Reg(&musb->rxcsr);
    hal_nvic_restore_interrupt_mask(savedMask);

    if (CSR & USB_RXCSR_RXPKTRDY) {
        result = false;
    } else {
        result = true;
    }

    return result;
}

hal_usb_status_t hal_usb_clear_tx_endpoint_fifo(uint32_t ep_num, hal_usb_endpoint_transfer_type_t ep_type, bool b_reset_toggle)
{
    uint32_t savedMask;

    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_TX, __LINE__);
    usb_ep_dma_running_check(ep_num, HAL_USB_EP_DIRECTION_TX, 0);

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, ep_num);

    if ((USB_DRV_Reg(&musb->txcsr)&USB_TXCSR_FIFONOTEMPTY) != 0) {
        g_UsbDrvInfo->ep_tx_flush_intr[ep_num - 1] = true;
    }

    /* FLUSHFIFO only works with TXPKTRDY is 0, so we cannot "or" */
    /* DMAReqEnab must be "0" before FLUSHFIFO, otherwise DMA will complete and issue DMA interrupt */
    if (ep_type == HAL_USB_EP_TRANSFER_ISO) {
        // To clear all DMA bits
        USB_DRV_WriteReg(&musb->txcsr, USB_TXCSR_ISO);

        if (b_reset_toggle == true) {
            USB_DRV_WriteReg(&musb->txcsr, (USB_TXCSR_FLUSHFIFO | USB_TXCSR_CLRDATATOG | USB_TXCSR_ISO));
            USB_DRV_WriteReg(&musb->txcsr, (USB_TXCSR_FLUSHFIFO | USB_TXCSR_CLRDATATOG | USB_TXCSR_ISO));
        } else {
            USB_DRV_WriteReg(&musb->txcsr, (USB_TXCSR_FLUSHFIFO | USB_TXCSR_ISO));
            USB_DRV_WriteReg(&musb->txcsr, (USB_TXCSR_FLUSHFIFO | USB_TXCSR_ISO));
        }
    } else {
        // To clear all DMA bits
        USB_DRV_WriteReg(&musb->txcsr, 0x00);

        if (b_reset_toggle == true) {
            USB_DRV_WriteReg(&musb->txcsr, (USB_TXCSR_FLUSHFIFO | USB_TXCSR_CLRDATATOG));
            USB_DRV_WriteReg(&musb->txcsr, (USB_TXCSR_FLUSHFIFO | USB_TXCSR_CLRDATATOG));
        } else {
            USB_DRV_WriteReg(&musb->txcsr, USB_TXCSR_FLUSHFIFO);
            USB_DRV_WriteReg(&musb->txcsr, USB_TXCSR_FLUSHFIFO);
        }
    }
    hal_nvic_restore_interrupt_mask(savedMask);

    return HAL_USB_STATUS_OK;
}


hal_usb_status_t hal_usb_clear_rx_endpoint_fifo(uint32_t ep_num, hal_usb_endpoint_transfer_type_t ep_type, bool b_reset_toggle)
{
    uint32_t savedMask;

    usb_ep_check(ep_num, HAL_USB_EP_DIRECTION_RX, __LINE__);
    usb_ep_dma_running_check(ep_num, HAL_USB_EP_DIRECTION_RX, 0);

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    USB_DRV_WriteReg8(&musb->index, ep_num);

    if ((USB_DRV_Reg(&musb->rxcsr)&USB_RXCSR_RXPKTRDY) != 0) {
        g_UsbDrvInfo->ep_rx_flush_intr[ep_num - 1] = true;
    }

    if (ep_type == HAL_USB_EP_TRANSFER_ISO) {
        // To clear all DMA bits
        USB_DRV_WriteReg(&musb->rxcsr, USB_RXCSR_ISO | USB_RXCSR_RXPKTRDY);

        if (b_reset_toggle == true) {
            USB_DRV_WriteReg(&musb->rxcsr, (USB_RXCSR_FLUSHFIFO | USB_RXCSR_CLRDATATOG | USB_RXCSR_ISO | USB_RXCSR_RXPKTRDY));
            USB_DRV_WriteReg(&musb->rxcsr, (USB_RXCSR_FLUSHFIFO | USB_RXCSR_CLRDATATOG | USB_RXCSR_ISO | USB_RXCSR_RXPKTRDY));
        } else {
            USB_DRV_WriteReg(&musb->rxcsr, (USB_RXCSR_FLUSHFIFO | USB_RXCSR_ISO | USB_RXCSR_RXPKTRDY));
            USB_DRV_WriteReg(&musb->rxcsr, (USB_RXCSR_FLUSHFIFO | USB_RXCSR_ISO | USB_RXCSR_RXPKTRDY));
        }
    } else {
        // To clear all DMA bits
        USB_DRV_WriteReg(&musb->rxcsr, USB_RXCSR_RXPKTRDY);

        if (b_reset_toggle == true) {
            USB_DRV_WriteReg(&musb->rxcsr, (USB_RXCSR_FLUSHFIFO | USB_RXCSR_CLRDATATOG | USB_RXCSR_RXPKTRDY));
            USB_DRV_WriteReg(&musb->rxcsr, (USB_RXCSR_FLUSHFIFO | USB_RXCSR_CLRDATATOG | USB_RXCSR_RXPKTRDY));
        } else {
            USB_DRV_WriteReg(&musb->rxcsr, (USB_RXCSR_FLUSHFIFO | USB_RXCSR_RXPKTRDY));
            USB_DRV_WriteReg(&musb->rxcsr, (USB_RXCSR_FLUSHFIFO | USB_RXCSR_RXPKTRDY));
        }
    }

    hal_nvic_restore_interrupt_mask(savedMask);
    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_get_dma_channel(uint32_t ep_tx_num, uint32_t ep_rx_num, hal_usb_endpoint_direction_t direction, bool same_chan)
{
    /* Should not allow re-entry */
    static bool race_check = false;

    if (race_check == true) {
        log_hal_msgid_error("hal_usb_get_dma_channel : race check Fail", 0);
    }
    race_check = true;

    g_UsbDrvInfo->dma_channel++;

    if (g_UsbDrvInfo->dma_channel > HAL_USB_MAX_NUMBER_DMA) {
        log_hal_msgid_error("hal_usb_get_dma_channel : g_UsbDrvInfo->dma_channel:%d Fail", 1, g_UsbDrvInfo->dma_channel);
        assert(0);
    }

    if (same_chan == true) {
        usb_ep_check(ep_tx_num, HAL_USB_EP_DIRECTION_TX, __LINE__);
        usb_ep_check(ep_rx_num, HAL_USB_EP_DIRECTION_RX, __LINE__);

        g_UsbDrvInfo->is_bidirection_dma[g_UsbDrvInfo->dma_channel - 1] = true;
        /* the same channel */
        g_UsbDrvInfo->dma_port[HAL_USB_EP_DIRECTION_TX][ep_tx_num - 1] = g_UsbDrvInfo->dma_channel;
        g_UsbDrvInfo->dma_port[HAL_USB_EP_DIRECTION_RX][ep_rx_num - 1] = g_UsbDrvInfo->dma_channel;
        g_UsbDrvInfo->dma_tx_ep_num[g_UsbDrvInfo->dma_channel - 1] = ep_tx_num;
        g_UsbDrvInfo->dma_rx_ep_num[g_UsbDrvInfo->dma_channel - 1] = ep_rx_num;
    } else {
        g_UsbDrvInfo->is_bidirection_dma[g_UsbDrvInfo->dma_channel - 1] = false;
        g_UsbDrvInfo->dma_dir[g_UsbDrvInfo->dma_channel - 1] = direction;

        if (direction == HAL_USB_EP_DIRECTION_TX) {
            usb_ep_check(ep_tx_num, HAL_USB_EP_DIRECTION_TX, __LINE__);
            g_UsbDrvInfo->dma_port[HAL_USB_EP_DIRECTION_TX][ep_tx_num - 1] = g_UsbDrvInfo->dma_channel;
            g_UsbDrvInfo->dma_tx_ep_num[g_UsbDrvInfo->dma_channel - 1] = ep_tx_num;
        } else {
            usb_ep_check(ep_rx_num, HAL_USB_EP_DIRECTION_RX, __LINE__);
            g_UsbDrvInfo->dma_port[HAL_USB_EP_DIRECTION_RX][ep_rx_num - 1] = g_UsbDrvInfo->dma_channel;
            g_UsbDrvInfo->dma_rx_ep_num[g_UsbDrvInfo->dma_channel - 1] = ep_rx_num;
        }
    }

    race_check = false;

    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_stop_dma_channel(uint32_t ep_num, hal_usb_endpoint_direction_t direction)
{
    bool dma_pktrdy;
    uint8_t dma_chan;
    uint32_t savedMask;
    uint32_t left_count;
    volatile uint32_t delay;

    dma_chan = usb_get_dma_channel_num(ep_num, direction);

    hal_nvic_save_and_set_interrupt_mask(&savedMask);

    /* Stop DMA channel */
    USBDMA_Stop(dma_chan);

    /* Clear pending DMA interrupts */
    DRV_WriteReg8(&musb->dma_intr_status, (1 << (dma_chan - 1)));
    //DRV_WriteReg8(&musb->dma_intr_unmask_set, (1<<(dma_chan-1)));
    dma_pktrdy = g_UsbDrvInfo->dma_pktrdy[dma_chan - 1];
    g_UsbDrvInfo->dma_pktrdy[dma_chan - 1] = false;
    g_UsbDrvInfo->dma_running[dma_chan - 1] = false;
    hal_nvic_restore_interrupt_mask(savedMask);

    /* wait for dma stop */
    for (delay = 0 ; delay < 500 ; delay++) ;

    left_count = USB_DMACNT(dma_chan); //get DMA Real CNT
    if ((left_count == 0)  || (left_count > g_UsbDrvInfo->dma_tx_length[dma_chan - 1])) {  /* check for short pkt */
        /* drop data in FIFO*/
        if (dma_pktrdy == true) {
            if (g_UsbDrvInfo->dma_dir[dma_chan - 1] == HAL_USB_EP_DIRECTION_TX) {
                hal_nvic_save_and_set_interrupt_mask(&savedMask);
                USB_DRV_WriteReg8(&musb->index, ep_num);
                USB_DRV_SetBits(&musb->txcsr, USB_TXCSR_FLUSHFIFO | USB_TXCSR_TXPKTRDY);
                hal_nvic_restore_interrupt_mask(savedMask);
            }
        }
    }

    return HAL_USB_STATUS_OK;
}


hal_usb_status_t hal_usb_release_dma_channel(uint32_t ep_tx_num, uint32_t ep_rx_num, hal_usb_endpoint_direction_t direction, bool same_chan)
{
    uint8_t dma_chan;

    if (same_chan == true) {
        usb_ep_check(ep_tx_num, HAL_USB_EP_DIRECTION_TX, __LINE__);
        usb_ep_check(ep_rx_num, HAL_USB_EP_DIRECTION_RX, __LINE__);

        usb_hw_stop_dma_channel(ep_tx_num, HAL_USB_EP_DIRECTION_TX);
        dma_chan = usb_get_dma_channel_num(ep_tx_num, HAL_USB_EP_DIRECTION_TX);

        if (g_UsbDrvInfo->is_bidirection_dma[dma_chan - 1] == false) {
            log_hal_msgid_error("hal_usb_release_dma_channel : 1", 0);
        }

        g_UsbDrvInfo->dma_tx_ep_num[dma_chan - 1] = 0;
        g_UsbDrvInfo->dma_rx_ep_num[dma_chan - 1] = 0;
        g_UsbDrvInfo->dma_port[HAL_USB_EP_DIRECTION_TX][ep_tx_num - 1] = 0;
        g_UsbDrvInfo->dma_port[HAL_USB_EP_DIRECTION_RX][ep_rx_num - 1] = 0;
    } else {
        if (direction == HAL_USB_EP_DIRECTION_TX) {
            usb_ep_check(ep_tx_num, HAL_USB_EP_DIRECTION_TX, __LINE__);
            usb_hw_stop_dma_channel(ep_tx_num, HAL_USB_EP_DIRECTION_TX);
            dma_chan = usb_get_dma_channel_num(ep_tx_num, HAL_USB_EP_DIRECTION_TX);

            if (g_UsbDrvInfo->is_bidirection_dma[dma_chan - 1] == true) {
                log_hal_msgid_error("hal_usb_release_dma_channel : 2", 0);
            }

            g_UsbDrvInfo->dma_tx_ep_num[dma_chan - 1] = 0;
            g_UsbDrvInfo->dma_port[HAL_USB_EP_DIRECTION_TX][ep_tx_num - 1] = 0;
        } else {
            usb_ep_check(ep_rx_num, HAL_USB_EP_DIRECTION_RX, __LINE__);
            usb_hw_stop_dma_channel(ep_rx_num, HAL_USB_EP_DIRECTION_RX);
            dma_chan = usb_get_dma_channel_num(ep_rx_num, HAL_USB_EP_DIRECTION_RX);

            if (g_UsbDrvInfo->is_bidirection_dma[dma_chan - 1] == true) {
                log_hal_msgid_error("hal_usb_release_dma_channel : 3", 0);
            }

            g_UsbDrvInfo->dma_rx_ep_num[dma_chan - 1] = 0;
            g_UsbDrvInfo->dma_port[HAL_USB_EP_DIRECTION_RX][ep_rx_num - 1] = 0;
        }
    }

    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_start_dma_channel(uint32_t ep_num, hal_usb_endpoint_direction_t direction, hal_usb_endpoint_transfer_type_t ep_type, void *addr, uint32_t length,
                                           hal_usb_dma_handler_t callback, bool callback_upd_run, hal_usb_dma_type_t dma_type)
{
    bool known_size = true;
    uint8_t dma_chan;
    uint16_t dma_ctrl;
    uint32_t savedMask;
    static uint32_t dma_burst_mode = 0;

    if (length == 0) {
        log_hal_msgid_error("hal_usb_start_dma_channel : length = 0 Fail", 0);
    }

    if ((g_UsbDrvInfo->usb_disconnect == true) || (g_UsbDrvInfo->is_initialized == false)) {
        return HAL_USB_STATUS_OK;
    }

    dma_chan = usb_get_dma_channel_num(ep_num, direction);

    if (g_UsbDrvInfo->dma_running[dma_chan - 1] != false) {
        log_hal_msgid_error("hal_usb_start_dma_channel : 1", 0);
    }

    if (g_UsbDrvInfo->is_bidirection_dma[dma_chan - 1] == true) {
        g_UsbDrvInfo->dma_dir[dma_chan - 1] = direction;
    } else if (g_UsbDrvInfo->dma_dir[dma_chan - 1] != direction) {
        log_hal_msgid_error("hal_usb_start_dma_channel : 2", 0);
    }

    g_UsbDrvInfo->dma_running[dma_chan - 1] = true;
    g_UsbDrvInfo->dma_callback[dma_chan - 1] = callback;
    g_UsbDrvInfo->dma_callback_upd_run[dma_chan - 1] = callback_upd_run;
    g_UsbDrvInfo->dma_pktrdy[dma_chan - 1] = false;

    if (direction == HAL_USB_EP_DIRECTION_RX) {
        if (ep_type == HAL_USB_EP_TRANSFER_BULK) {
            if (g_UsbDrvInfo->hs_mode == true) {
                if (((length % HAL_USB_MAX_PACKET_SIZE_ENDPOINT_BULK_HIGH_SPEED) != 0)
                    && ((length % HAL_USB_MAX_PACKET_SIZE_ENDPOINT_BULK_HIGH_SPEED) <= (HAL_USB_MAX_PACKET_SIZE_ENDPOINT_BULK_HIGH_SPEED - 4))) {
                    g_UsbDrvInfo->dma_pktrdy[dma_chan - 1] = true;
                }
            } else {
                if (((length % HAL_USB_MAX_PACKET_SIZE_ENDPOINT_BULK_FULL_SPEED) != 0)
                    && ((length % HAL_USB_MAX_PACKET_SIZE_ENDPOINT_BULK_FULL_SPEED) <= (HAL_USB_MAX_PACKET_SIZE_ENDPOINT_BULK_FULL_SPEED - 4))) {
                    g_UsbDrvInfo->dma_pktrdy[dma_chan - 1] = true;
                }
            }
        } else if (ep_type == HAL_USB_EP_TRANSFER_ISO) {
            if (g_UsbDrvInfo->hs_mode == true) {
                if (((length % HAL_USB_MAX_PACKET_SIZE_ENDPOINT_ISOCHRONOUS_HIGH_SPEED) != 0)
                    && ((length % HAL_USB_MAX_PACKET_SIZE_ENDPOINT_ISOCHRONOUS_HIGH_SPEED) <= (HAL_USB_MAX_PACKET_SIZE_ENDPOINT_ISOCHRONOUS_HIGH_SPEED - 4))) {
                    g_UsbDrvInfo->dma_pktrdy[dma_chan - 1] = true;
                }
            } else {
                if (((length % HAL_USB_MAX_PACKET_SIZE_ENDPOINT_ISOCHRONOUS_FULL_SPEED) != 0)
                    && ((length % HAL_USB_MAX_PACKET_SIZE_ENDPOINT_ISOCHRONOUS_FULL_SPEED) <= (HAL_USB_MAX_PACKET_SIZE_ENDPOINT_ISOCHRONOUS_FULL_SPEED - 4))) {
                    g_UsbDrvInfo->dma_pktrdy[dma_chan - 1] = true;
                }
            }
        }
    } else {
        if (ep_type == HAL_USB_EP_TRANSFER_BULK) {
            if (g_UsbDrvInfo->hs_mode == true) {
                if (length % HAL_USB_MAX_PACKET_SIZE_ENDPOINT_BULK_HIGH_SPEED) {
                    g_UsbDrvInfo->dma_pktrdy[dma_chan - 1] = true;
                }
            } else {
                if (length % HAL_USB_MAX_PACKET_SIZE_ENDPOINT_BULK_FULL_SPEED) {
                    g_UsbDrvInfo->dma_pktrdy[dma_chan - 1] = true;
                }
            }
        } else if (ep_type == HAL_USB_EP_TRANSFER_INTR) {
            if (g_UsbDrvInfo->hs_mode == true) {
                if (length % HAL_USB_MAX_PACKET_SIZE_ENDPOINT_INTERRUPT_HIGH_SPEED) {
                    g_UsbDrvInfo->dma_pktrdy[dma_chan - 1] = true;
                }
            } else {
                if (length % HAL_USB_MAX_PACKET_SIZE_ENDPOINT_INTERRUPT_FULL_SPEED) {
                    g_UsbDrvInfo->dma_pktrdy[dma_chan - 1] = true;
                }
            }
        } else if (ep_type == HAL_USB_EP_TRANSFER_ISO) {
            if (g_UsbDrvInfo->hs_mode == true) {
                if (length % HAL_USB_MAX_PACKET_SIZE_ENDPOINT_ISOCHRONOUS_HIGH_SPEED) {
                    g_UsbDrvInfo->dma_pktrdy[dma_chan - 1] = true;
                }
            } else {
                if (length % HAL_USB_MAX_PACKET_SIZE_ENDPOINT_ISOCHRONOUS_FULL_SPEED) {
                    g_UsbDrvInfo->dma_pktrdy[dma_chan - 1] = true;
                }
            }
        }
    }

    if (g_UsbDrvInfo->b_enable_dma_burst_auto_chge == true) {
        dma_burst_mode++;
    } else {
        dma_burst_mode = g_UsbDrvInfo->dma_burst_mode;
    }

    /* DMA_CONFIG */
    if (dma_type == HAL_USB_DMA0_TYPE) {

    } else if (dma_type == HAL_USB_DMA1_TYPE) {
        if (direction == HAL_USB_EP_DIRECTION_TX) {
            hal_nvic_save_and_set_interrupt_mask(&savedMask);
            USB_DRV_WriteReg8(&musb->index, ep_num);

            /* Only is configured as multiple packet DMA TX mode */
            if (ep_type == HAL_USB_EP_TRANSFER_ISO) {
                USB_DRV_WriteReg(&musb->txcsr, USB_DMA_TX_CSR_ISO);
            } else {
                USB_DRV_WriteReg(&musb->txcsr, USB_DMA_TX_CSR);
            }

            hal_nvic_restore_interrupt_mask(savedMask);
            g_UsbDrvInfo->dma_tx_length[dma_chan - 1] = length;
            //log_hal_msgid_info("USB DMA Setup: leng: %d addr:0x%x ", 2, length, addr);
            USB_DRV_WriteReg32(USB_DMAADDR(dma_chan), addr);
            USB_DRV_WriteReg32(USB_DMACNT(dma_chan), length);
            //log_hal_msgid_info("USB DMA Setup: read back DMAADDR: 0x%x = 0x%x, DMACNT: 0x%x = 0x%x", 4,
            //USB_DMAADDR(dma_chan), USB_DRV_Reg32(USB_DMAADDR(dma_chan)),
            //USB_DMACNT(dma_chan), USB_DRV_Reg32(USB_DMACNT(dma_chan)));

            dma_ctrl = USB_DMACNTL_DMADIR | USB_DMACNTL_DMAMODE | USB_DMACNTL_INTEN | (ep_num << 4);
            dma_ctrl |= ((dma_burst_mode & 0x03) << 9) | USB_DMACNTL_DMAEN;

            USB_DRV_WriteReg(USB_DMACNTL(dma_chan), dma_ctrl);
        } else if (direction == HAL_USB_EP_DIRECTION_RX) {
            /* Stop DMA channel */
            USBDMA_Stop(dma_chan);

            if (known_size == true) {
                hal_nvic_save_and_set_interrupt_mask(&savedMask);
                USB_DRV_WriteReg8(&musb->index, ep_num);

                if (ep_type == HAL_USB_EP_TRANSFER_ISO) {
                    USB_DRV_WriteReg(&musb->rxcsr, USB_DMA_RX_CSR_ISO | USB_RXCSR_RXPKTRDY);
                } else {
                    USB_DRV_WriteReg(&musb->rxcsr, USB_DMA_RX_CSR | USB_RXCSR_RXPKTRDY);
                }

                hal_nvic_restore_interrupt_mask(savedMask);
            } else {
#ifdef  __DMA_UNKNOWN_RX__
                hal_nvic_save_and_set_interrupt_mask(&savedMask);
                USB_DRV_WriteReg8(&musb->index, ep_num);

                if (ep_type == HAL_USB_EP_TRANSFER_ISO) {
                    USB_DRV_WriteReg(&musb->rxcsr, USB_DMA_RX_CSR_ISO | USB_RXCSR_RXPKTRDY | USB_RXCSR_DMAREQMODE);
                } else {
                    USB_DRV_WriteReg(&musb->rxcsr, USB_DMA_RX_CSR | USB_RXCSR_RXPKTRDY | USB_RXCSR_DMAREQMODE);
                }

                hal_nvic_restore_interrupt_mask(savedMask);

                usb_enable_dma_timer_count(dma_chan, true, 0x7F);

#else   /* __DMA_UNKNOWN_RX__ */
                log_hal_msgid_error("hal_usb_start_dma_channel : 3", 0);
#endif  /* __DMA_UNKNOWN_RX__ */
            }

            USB_DRV_WriteReg32(USB_DMAADDR(dma_chan), addr);
            USB_DRV_WriteReg32(USB_DMACNT(dma_chan), length);

            dma_ctrl = USB_DMACNTL_DMAMODE | USB_DMACNTL_INTEN | (ep_num << 4);
            dma_ctrl |= ((dma_burst_mode & 0x03) << 9) | USB_DMACNTL_DMAEN;
            USB_DRV_WriteReg(USB_DMACNTL(dma_chan), dma_ctrl);
        }
    } else {
        log_hal_msgid_error("hal_usb_start_dma_channel : 4", 0);
    }
    return HAL_USB_STATUS_OK;
}

bool hal_usb_is_dma_running(uint32_t ep_num, hal_usb_endpoint_direction_t direction)
{
    bool result;
    uint8_t dma_chan;

    if (g_UsbDrvInfo->is_initialized == false) {
        return false;
    }

    dma_chan = usb_get_dma_channel_num(ep_num, direction);
    result = g_UsbDrvInfo->dma_running[dma_chan - 1];

    return result;
}

hal_usb_status_t hal_usb_is_dma_ready(uint32_t ep_num, hal_usb_endpoint_direction_t direction)
{
    uint8_t dma_chan;
    uint32_t DMAIntr;

    dma_chan = usb_get_dma_channel_num(ep_num, direction);

    DMAIntr = USB_DRV_Reg8(&musb->dma_intr_status);
    DMAIntr &= (1 << (dma_chan-1));

    if(DMAIntr) {
        /* write 1 clear */
        g_UsbDrvInfo->dma_running[dma_chan - 1] = false;
        USB_DRV_WriteReg8(&musb->dma_intr_status, (1 << (dma_chan-1)));
        return HAL_USB_STATUS_OK;
    }

    return HAL_USB_STATUS_ERROR;
}

bool hal_usb_is_high_speed(void)
{
    return (bool)g_UsbDrvInfo->hs_mode;
}

hal_usb_status_t hal_usb_enter_test_mode(hal_usb_test_mode_type_t test_selector)
{
    volatile uint32_t delay = 0;

    if (test_selector == HAL_USB_TEST_MODE_TYPE_J) {
        USB_DRV_WriteReg8(&musb->testmode, USB_TESTMODE_TESTJ);
    } else if (test_selector == HAL_USB_TEST_MODE_TYPE_K) {
        USB_DRV_WriteReg8(&musb->testmode, USB_TESTMODE_TESTK);
    } else if (test_selector == HAL_USB_TEST_MODE_TYPE_SE0_NAK) {
        USB_DRV_WriteReg8(&musb->testmode, USB_TESTMODE_TESTSE0NAK);
    } else if (test_selector == HAL_USB_TEST_MODE_TYPE_PACKET) {
        uint8_t packet_test[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
            0xAA, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE,
            0xEE, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xBF, 0xDF,
            0xEF, 0xF7, 0xFB, 0xFD, 0xFC, 0x7E, 0xBF, 0xDF,
            0xEF, 0xF7, 0xFB, 0xFD, 0x7E
        };

        usb_hw_epfifowrite(0, 53, packet_test);

        for (delay = 0; delay != 1000; delay++) ;

        USB_DRV_WriteReg8(&musb->index, 0);
        USB_DRV_WriteReg8(&musb->testmode, USB_TESTMODE_TESTPACKET);

        for (delay = 0; delay != 1000; delay++) ;

        USB_DRV_WriteReg(&musb->csr0, USB_CSR0_TXPKTRDY);
    } else {
        log_hal_msgid_error("hal_usb_enter_test_mode : USB_CSR0_TXPKTRDY Fail", 0);
    }

    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_reset_fifo(void)
{
    g_FIFOadd = USB_FIFO_START_ADDRESS;
    return HAL_USB_STATUS_OK;
}

/************************************************************
    Functions that is used whether USB_ENABLE is turned on or not
*************************************************************/
hal_usb_status_t hal_usb_init(void)
{
    hal_usb_pdn_mode(false);

    USB_DRV_WriteReg(&musb->busperf3, USB_DRV_Reg(&musb->busperf3) | (USB_BUSPERF3_DISUSBRESET | USB_BUSPERF3_SWRST));

    /* Enable system interrupts, but disable all ep interrupts */
    usb_en_sys_intr();
    USB_DRV_WriteReg8(&musb->index, 0);

    /* Flush ep0 FIFO */
    USB_DRV_WriteReg(&musb->csr0, USB_CSR0_FLUSHFIFO);

    /* enable EP0 interrupt */
    hal_usb_enable_endpoint0();

    /* Unmask USB L1 interrupt */
    DRV_WriteReg(&musb->usb_l1intm, (USB_L1INTM_DMA_INT_UNMASK | USB_L1INTM_TX_INT_UNMASK | USB_L1INTM_RX_INT_UNMASK | USB_L1INTM_USBCOM_INT_UNMASK | USB_L1INTM_PSR_INT_UNMASK) | USB_L1INTM_QINT_UNMASK | USB_L1INTM_QHIF_INT_UNMASK);
    DRV_WriteReg8(&musb->dma_intr_unmask_set,  0xFF);

    /*usb efuse setting.*/
    //log_hal_msgid_info("before modify 0xa20a0224 = %x ", 1, (*(volatile uint32_t *)(0xa20a0224)));
    //log_hal_msgid_info("before modify 0xa1030804 = %x ", 1, (*(volatile uint32_t *)(0xa1030804)));

    hal_usb_drv_create_isr();

    g_UsbDrvInfo->is_initialized = true;

    return HAL_USB_STATUS_OK;
}

hal_usb_status_t hal_usb_phy_preinit(void)
{
    uint32_t savedMask;
    bool do_init = false;

    hal_nvic_save_and_set_interrupt_mask(&savedMask);
    if (usb_init_phy_ready == false) {
        do_init = true;
        usb_init_phy_ready = true;
    }
    hal_nvic_restore_interrupt_mask(savedMask);

    if(do_init) {
        usb_pdn_disable();
        hal_usbphy_poweron_initialize();
        usb_pdn_enable();
    }

    return HAL_USB_STATUS_OK;
}

void hal_usb_save_current(void)
{
    usb_pdn_disable();
    hal_usb_pdn_mode(true);
    usb_pdn_enable();
}

/************************************************************
    Suspend/Resume
*************************************************************/

hal_usb_power_state_t hal_usb_get_suspend_state(void)
{
    return g_UsbDrvInfo->power_state;
}

static void hal_usb_do_suspend(void)
{
    log_hal_msgid_info("hal_usb_do_suspend", 0);

    if (g_UsbDrvInfo->suspend_hdlr != NULL) {
        g_UsbDrvInfo->suspend_hdlr();
    } else {
        log_hal_msgid_error("hal_usb_do_suspend : g_UsbDrvInfo->suspend_hdlr == NULL Fail", 0);
    }
}

static void hal_usb_do_resume(void)
{
    log_hal_msgid_info("hal_usb_do_resume", 0);
    if ((musb->power & USB_POWER_HSMODE) != 0) {
        g_UsbDrvInfo->hs_mode = true;
    } else {
        g_UsbDrvInfo->hs_mode = false;
    }

    if (g_UsbDrvInfo->resume_hdlr != NULL) {
        g_UsbDrvInfo->resume_hdlr();
    } else {
        log_hal_msgid_error("hal_usb_do_resume : g_UsbDrvInfo->resume_hdlr == NULL Fail", 0);
    }
}

/**
 * s_detect_linestate
 *   Only use in function
 *    - hal_usb_detect_linestate
 *    - hal_usb_start_gpt_detect_resume
 */
static bool s_detect_linestate = false;
#define HAL_USB_DETECT_LINESTATE_INTERVAL 1

static void hal_usb_detect_linestate(void* p)
{
    static hal_usb_linestate_t prev_ls;
    hal_usb_linestate_t ls = hal_usbphy_detect_linestate();

    if (prev_ls == HAL_USB_LINESTATE_DM_1_DP_0 && ls == HAL_USB_LINESTATE_DM_1_DP_0) {
        log_hal_msgid_info("hal_usb_detect_linestate, LINESTATE changed DM_1_DP_0, Resume Signal", 0);
        s_detect_linestate = false;
        clock_mux_sel(CLK_USB_SEL, 3);  /* 62.4 MHz, UPLL */
    }
    else if (prev_ls == HAL_USB_LINESTATE_DM_0_DP_0 && ls == HAL_USB_LINESTATE_DM_0_DP_0) {
        log_hal_msgid_info("hal_usb_detect_linestate, LINESTATE changed DM_0_DP_0, Reset Signal", 0);
        s_detect_linestate = false;
        clock_mux_sel(CLK_USB_SEL, 3);  /* 62.4 MHz, UPLL */
    }

    if (s_detect_linestate) {
        hal_usb_start_gpt_detect_linestate();
    }
    else {
        /* Do not trigger next detect */
    }
}

static void hal_usb_start_gpt_detect_linestate(void)
{
    static uint32_t gpt_handle = 0;
    hal_gpt_status_t gpt_status = HAL_GPT_STATUS_OK;
    s_detect_linestate = true;

    if (gpt_handle == 0) {
        log_hal_msgid_info("hal_usb_start_gpt_detect_linestate", 0);
        gpt_status = hal_gpt_sw_get_timer(&gpt_handle);

        if (gpt_status != HAL_GPT_STATUS_OK) {
            log_hal_msgid_error("hal_usb_start_gpt_detect_linestate, can't get timer, err[%d]", 1, gpt_status);
        }
    }

    hal_gpt_sw_start_timer_ms(gpt_handle, HAL_USB_DETECT_LINESTATE_INTERVAL, (hal_gpt_callback_t)hal_usb_detect_linestate, NULL);

    if (gpt_status != HAL_GPT_STATUS_OK) {
        log_hal_msgid_error("hal_usb_start_gpt_detect_linestate, can't start timer, err[%d]", 1, gpt_status);
    }
}

uint8_t hal_usb_rmwk_trigger(void)
{
/*
    if (hal_usb_get_suspend_state() == HAL_USB_POWER_STATE_SUSPEND) {
        hal_usb_do_resume();
        USB_DRV_SetBits8(&musb->power, USB_POWER_RESUME);
        return 0;
    } else {
        return 1;
    }
*/
    return 0;
}

uint8_t hal_usb_rmwk_check(void)
{
    //return (musb->power & USB_POWER_RESUME) >> 2;
    return 0;
}

uint8_t hal_usb_rmwk_clear(void)
{
    //USB_DRV_ClearBits8(&musb->power, USB_POWER_RESUME);
    return 0;
}

#endif /*HAL_USB_MODULE_ENABLED*/

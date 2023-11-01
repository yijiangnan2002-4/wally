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

#if defined (HAL_USB_MODULE_ENABLED) || defined (HAL_USB_HOST_MODULE_ENABLED)

#include "hal_clock.h"
#include "hal_clock_internal.h"
#include "hal_clock_platform.h"
#include "hal_eint.h"
#include "hal_gpt.h"
#include "hal_log.h"
#include "hal_pmu.h"

#include <stdio.h>

extern uint32_t sub_chip_version_get();
#define CHIP_VERSION_AB1585D    0x07

void hal_usbphy_poweron_initialize(void)
{
    pmu_enable_usb_power(PMU_ON);
    uint32_t data, chip_version;

    /* switch to USB function */
    USB_DRV_ClearBits32(U2PHYDTM0, U2PHYDTM0_FORCE_UART_EN);
    USB_DRV_ClearBits32(U2PHYDTM1, U2PHYDTM1_RG_UART_EN);

    /* force suspend till PHY set complete */
    USB_DRV_SetBits32(U2PHYDTM0, U2PHYDTM0_RG_SUSPENDM);
    USB_DRV_SetBits32(U2PHYDTM0, U2PHYDTM0_FORCE_SUSPENDM);

    /* DP/DM mode */
    USB_DRV_ClearBits32(U2PHYACR4, U2PHYACR4_USB20_GPIO_MODE);
    USB_DRV_ClearBits32(U2PHYACR4, U2PHYACR4_RG_USB20_GPIO_CTL);
    /* dp_100k & dm_100k disable */
    USB_DRV_ClearBits32(U2PHYACR4, U2PHYACR4_RG_USB20_DP_100K_EN);
    USB_DRV_ClearBits32(U2PHYACR4, U2PHYACR4_RG_USB20_DM_100K_EN);

    /* RG_USB20_INTR_CAL would be written in eFUSE 0x420C0370[4:0] */
    data = DRV_Reg32(0x420C0370) & 0b11111;
    printf("0x420C0370 & 0b11111 data = 0x%0X", (unsigned int)data);
    if (data == 0) {
        printf("not to set USBPHYACR1 RG_USB20_INTR_CAL");
    } else {
        USB_DRV_SetData32(USBPHYACR1, 0b11111 << 19, data << 19);
    }

    /* RG_HALF_PMOS_EN on */
    USB_DRV_SetData32(USBPHYACR2, 1 << 4, 1 << 4);

    /* Slew rate control */
    USB_DRV_ClearBits32(USBPHYACR5, USBPHYACR5_RG_USB20_HSTX_SRCTRL_MSK);
    USB_DRV_SetData32(USBPHYACR5, USBPHYACR5_RG_USB20_HSTX_SRCTRL_MSK, 0b000 << 12);

    /* DP/DM BC1.1 path Disable */
    USB_DRV_ClearBits32(USBPHYACR6, USBPHYACR6_RG_USB20_BC11_SW_EN);
    /* OTG Disable */
    USB_DRV_ClearBits32(USBPHYACR6, USBPHYACR6_RG_USB20_OTG_VBUSCMP_EN);
    /* pre-emphasis level */
    USB_DRV_ClearBits32(USBPHYACR6, 0b11 << 30);
    /* AB1585D dongle air desesne */
    chip_version = sub_chip_version_get();
    if(chip_version == CHIP_VERSION_AB1585D){
        USB_DRV_SetData32(USBPHYACR6, 0b11 << 30, 0b11 << 30);
    } else {
      USB_DRV_SetData32(USBPHYACR6, 0b11 << 30, 0b10 << 30);
    }
    printf("USBPHYACR6 0x%X, chip_version:0x%X", (unsigned int)(USB_DRV_Reg32(USBPHYACR6)), (unsigned int)chip_version);

    /* Release force suspendm */
    USB_DRV_ClearBits32(U2PHYDTM0, U2PHYDTM0_FORCE_SUSPENDM);

    /* Force to device mode */
    USB_DRV_SetBits32(U2PHYDTM1, U2PHYDTM1_RG_IDDIG);
    USB_DRV_SetBits32(U2PHYDTM1, U2PHYDTM1_force_iddig);
    /* USB PHY's hw of vbusvalid, bvalid, avalid is useless. Set force to bypass it. */
    USB_DRV_SetBits32(U2PHYDTM1, U2PHYDTM1_RG_VBUSVALID | U2PHYDTM1_RG_BVALID | U2PHYDTM1_RG_AVALID);
    USB_DRV_SetBits32(U2PHYDTM1, U2PHYDTM1_force_vbusvalid | U2PHYDTM1_force_bvalid | U2PHYDTM1_force_avalid);

    hal_gpt_delay_us(2000);

    USB_DRV_SetBits32(U2PHYDTM0, U2PHYDTM0_RG_SUSPENDM);
}

void hal_usbphy_deinit_case(void)
{
    WDT_REGISTER->WDT_RST3_UNION.WDT_RST3 = 0x01220121;
    hal_gpt_delay_ms(1);
    WDT_REGISTER->WDT_RST3_UNION.WDT_RST3 = 0x00220021;
}

void hal_usbphy_enter_suspend(void)
{
    USB_DRV_SetBits32(USBPHYACR0, USBPHYACR0_RG_USB20_BGR_EN);
    USB_DRV_SetBits32(USBPHYACR0, USBPHYACR0_RG_USB20_REF_EN);

    USB_DRV_SetBits32(U2PHYACR3, U2PHYACR3_RG_USB20_TMODE_FS_LS_RCV_EN);

    USB_DRV_ClearBits32(U2PHYACR4, U2PHYACR4_RG_USB20_HS_RCV_EN_MODE_MSK);
    USB_DRV_SetBits32(U2PHYACR4, 0b01 << 0);

    USB_DRV_SetBits32(U2PHYDMON0, U2PHYDMON0_RG_USB20_BGLPF_FORCE_ON);
    USB_DRV_SetBits32(U2PHYDMON0, U2PHYDMON0_RG_USB20_BGLPF_FORCE_OFF);

    clock_mux_sel(CLK_USB_SEL, 0); /* USB mux to DCXO */
}

void hal_usbphy_leave_suspend()
{
    clock_mux_sel(CLK_USB_SEL, 3);  /* 62.4 MHz, UPLL */

    USB_DRV_ClearBits32(USBPHYACR0, USBPHYACR0_RG_USB20_BGR_EN);
    USB_DRV_ClearBits32(USBPHYACR0, USBPHYACR0_RG_USB20_REF_EN);

    USB_DRV_ClearBits32(U2PHYACR3, U2PHYACR3_RG_USB20_TMODE_FS_LS_RCV_EN);

    USB_DRV_ClearBits32(U2PHYACR4, U2PHYACR4_RG_USB20_HS_RCV_EN_MODE_MSK);
    USB_DRV_SetBits32(U2PHYACR4, 0b10 << 0);

    USB_DRV_ClearBits32(U2PHYDMON0, U2PHYDMON0_RG_USB20_BGLPF_FORCE_ON);
    USB_DRV_ClearBits32(U2PHYDMON0, U2PHYDMON0_RG_USB20_BGLPF_FORCE_OFF);
}

hal_usb_linestate_t hal_usbphy_detect_linestate(void)
{
    hal_usb_linestate_t ls = (hal_usb_linestate_t)(0x3 & (USB_DRV_Reg32(U2PHYDMON1) >> 22));
    /* log_hal_msgid_info("hal_usbphy_detect_linestate DM_%d DP_%d", 2, (ls >> 1) & 0x01, ls & 0x01); */

    return ls;
}

#endif /* HAL_USB_MODULE_ENABLED or HAL_USB_HOST_MODULE_ENABLED */

/* Copyright Statement:
 *
 * (C) 2023  Airoha Technology Corp. All rights reserved.
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


#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "bl_common.h"
#include "dfu_util.h"
#include "hal_wdt_internal.h"

#include "lw_mux.h"
#include "race_cmd_bootloader.h"

#if defined(AIR_BL_USB_HID_DFU_ENABLE)
#include "bl_usb_hid.h"
#include "hal_pmu.h"

bool g_usb_is_deinit = true;
#endif


#if defined(AIR_BL_DFU_GPIO_CTRL_ENABLE)
#include "hal_gpio.h"

/*[NOTICE] Please confirm which GPIO you can use at first! HAL_GPIO_25 is just a demo.*/
#define DFU_EN_GPIO HAL_GPIO_25

bool bl_dfu_gpio_check(void)
{
    /*
       This is an example to use GPIO to enter DFU mode provided by Airoha.
       [NOTICE] Please confirm which GPIO you can use at first! HAL_GPIO_25 is just a demo.
    */

    hal_gpio_data_t gpio_status = HAL_GPIO_DATA_HIGH;

    hal_gpio_pull_up(DFU_EN_GPIO); /* Let HAL_GPIO_25 defualt pull up */
    hal_gpio_set_direction(DFU_EN_GPIO, HAL_GPIO_DIRECTION_INPUT);
    hal_gpio_get_input(DFU_EN_GPIO, &gpio_status);
    if(gpio_status == HAL_GPIO_DATA_HIGH)
    {
        bl_print(LOG_DEBUG, "[bl_dfu_gpio_check] GPIO%d high gpio_status:%d\r\n", DFU_EN_GPIO, gpio_status);
        return false;
    }
    else
    {
        bl_print(LOG_DEBUG, "[bl_dfu_gpio_check] GPIO%d low gpio_status:%d\r\n", DFU_EN_GPIO, gpio_status);
        return true;
    }
}

#endif /*AIR_BL_DFU_GPIO_CTRL_ENABLE*/

bool bl_dfu_customize_entry()
{
    /*
       Customer could write customization entry to enter DFU mode.
       For example, use GPIO to control.
    */

    bool dfu_enable = false;

#if defined(AIR_BL_DFU_GPIO_CTRL_ENABLE)
    /*This is an example to use GPIO to enter DFU mode provided by Airoha.*/
    dfu_enable = bl_dfu_gpio_check();
#endif

    return dfu_enable;
}

#if defined(AIR_BL_USB_HID_DFU_ENABLE)
bool bl_dfu_usb_detection(void)
{
    bool usb_exit = false;

    /* Detect VBUS */
    if(pmu_get_chr_detect_value())
    {
        /*Check USB initialization status*/
        if(!bl_usb_hid_is_ready())
        {
            /* VBUS on but USB is not ready */
            bl_print(LOG_DEBUG, "[bl_dfu_usb_detection] VBUS on but USB is not init \r\n");
            if(BL_USB_HID_STATUS_OK == bl_usb_hid_init()) /* Initialize USB */
            {
                race_bl_usb_plug_in_handler(); /* race init */
                usb_exit = true;
            }
        }
        else
        {
            /* VBUS on and USB is ready */
            usb_exit = true;
        }
    }
    else
    {
        /*Check USB initialization status*/
        if(bl_usb_hid_is_ready())
        {
            /* VBUS off but USB is ready */
            bl_print(LOG_DEBUG, "[bl_dfu_usb_detection] VBUS off but USB is init \r\n");
            if(BL_USB_HID_STATUS_OK == bl_usb_hid_deinit())  /* De-initialize USB */
            {
                race_bl_usb_plug_out_handler(); /* race de-init */
            }
        }
    }

    return usb_exit;
}
#endif


void bl_dfu_process(void)
{
    bool usb_res = true;

    race_bl_init();

    while(1)
    {

#if defined(AIR_BL_USB_HID_DFU_ENABLE)
        usb_res = bl_dfu_usb_detection();
#endif

        if(usb_res == true)
        {
            lw_mux_trigger_receiver();
        }

        hal_wdt_feed(HAL_WDT_FEED_MAGIC);
    }
}



void bl_dfu_entry(void)
{
    /*Check DFU enable flag*/
    if(dfu_flag_is_set() || bl_dfu_customize_entry())
    {
        bl_print(LOG_DEBUG, "[bl_dfu_entry] Enter DFU mode ... \r\n");
        bl_dfu_process();
    }
    else
    {
        bl_print(LOG_DEBUG, "[bl_dfu_entry] Leave DFU mode \r\n");
    }
}





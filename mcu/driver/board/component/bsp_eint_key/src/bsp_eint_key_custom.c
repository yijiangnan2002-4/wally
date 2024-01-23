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

#include "bsp_eint_key_custom.h"
#include "bsp_eint_key.h"
#include "bsp_eint_key_normal_gpio.h"
#include "bsp_eint_key_rtc_gpio.h"
#include "hal_gpio.h"
#include "hal_log.h"
#include "assert.h"
#include "airo_key_config.h"

#ifdef AIR_BSP_EINT_KEY_ACTIVE_LEVEL_CONFIGURABLE
#define BSP_EINTKEY_DECLARE_ACTIVE_LEVEL(num)\
    extern const unsigned char BSP_EINTKEY##num##_ACTIVE_LEVEL

#ifdef BSP_EINT_KEY_INTERNAL_PULL_ENABLE
#define BSP_EINTKEY_CHECK_ACTIVE_LEVEL(level)\
    if (level == 0xFF) {\
        LOG_MSGID_E(eint_key, "[eint_key]eint key is configured to disable pull by EPT on GPIO page, please check. key_data=%d \r\n", 1, new_driver->driver.key_data);\
        assert(0);\
    }
#else
#define BSP_EINTKEY_CHECK_ACTIVE_LEVEL(level)
#endif

#define BSP_EINTKEY_SET_ACTIVE_LEVEL(new_driver, level)\
do { \
    BSP_EINTKEY_CHECK_ACTIVE_LEVEL(level);\
    new_driver->driver.active_level = level;\
} while (0)
#else
#define BSP_EINTKEY_DECLARE_ACTIVE_LEVEL(num)
#define BSP_EINTKEY_SET_ACTIVE_LEVEL(new_driver, level)
#endif

#define BSP_EINTKEY_DECLARE_EXTERN_VAR(num)\
    extern const char BSP_EINTKEY##num##_PIN;\
    extern const char BSP_EINTKEY##num##_PIN_M_EINT;\
    extern const unsigned char BSP_EINTKEY##num##_EINT;\
    BSP_EINTKEY_DECLARE_ACTIVE_LEVEL(num)

typedef union {
    bsp_eint_key_normal_gpio_driver_t normal;
#ifdef AIR_BSP_EINT_KEY_RTC_GPIO_ENABLE
    bsp_eint_key_rtc_gpio_driver_t rtc;
#endif
} bsp_eint_key_gpio_driver_obj_t;

static bsp_eint_key_gpio_driver_obj_t bsp_eint_key_gpio_driver_buf[BSP_EINT_KEY_NUMBER];

#ifdef AIR_BSP_EINT_KEY_RTC_GPIO_ENABLE
extern void bsp_eint_key_rtc_gpio_driver_new(bsp_eint_key_rtc_gpio_driver_t *new_driver, const char gpio_port);

static void bsp_eintkey_rtc_gpio_driver_init(uint32_t key_index, uint32_t *available_key_nums, uint32_t key_data, uint32_t pin_num, uint32_t active_level)
{
    if (pin_num != 0xff) {
        bsp_eint_key_rtc_gpio_driver_t *new_driver = &bsp_eint_key_gpio_driver_buf[key_index].rtc;
        bsp_eint_key_rtc_gpio_driver_new(new_driver, pin_num);
        new_driver->driver.key_data = key_data;
        BSP_EINTKEY_SET_ACTIVE_LEVEL(new_driver, active_level);
        *available_key_nums += 1;
    }
}

#define BSP_EINTKEY_RTC_GPIO_DRIVER_INIT(key_index, available_key_nums) \
    bsp_eintkey_rtc_gpio_driver_init(key_index, &available_key_nums, BSP_EINT_KEY_DATA##key_index, BSP_EINTKEY##key_index##_PIN, BSP_EINTKEY##key_index##_ACTIVE_LEVEL)
#else
#define BSP_EINTKEY_RTC_GPIO_DRIVER_INIT(key_index, available_key_nums)
#endif

extern void bsp_eint_key_normal_gpio_driver_new(
    bsp_eint_key_normal_gpio_driver_t *new_driver, const char gpio_port, const char eint_mode, unsigned char eint_number);

static void bsp_eintkey_normal_gpio_driver_init(uint32_t key_index, uint32_t *available_key_nums, uint32_t key_data, uint32_t pin_num, uint32_t eint_mode, uint32_t eint_num, uint32_t active_level)
{
    bsp_eint_key_normal_gpio_driver_t *new_driver;

    if (pin_num != 0xff) {
        if (eint_num != 0xff) {
            new_driver = &bsp_eint_key_gpio_driver_buf[key_index].normal;
            bsp_eint_key_normal_gpio_driver_new(new_driver, pin_num, eint_mode, eint_num);
            new_driver->gpio_port = pin_num;
            new_driver->driver.key_data = key_data;
            BSP_EINTKEY_SET_ACTIVE_LEVEL(new_driver, active_level);
            *available_key_nums += 1;
        }
    }
}

#define BSP_EINTKEY_DRIVER_INIT(key_index, available_key_nums)\
    if (BSP_EINTKEY##key_index##_EINT == 0xff) {\
        BSP_EINTKEY_RTC_GPIO_DRIVER_INIT(key_index, available_key_nums);\
    } else {\
        bsp_eintkey_normal_gpio_driver_init(key_index, &available_key_nums, BSP_EINT_KEY_DATA##key_index, \
            BSP_EINTKEY##key_index##_PIN, BSP_EINTKEY##key_index##_PIN_M_EINT, BSP_EINTKEY##key_index##_EINT, BSP_EINTKEY##key_index##_ACTIVE_LEVEL);\
    }

/* Declaring the extern variable for eintkey.
The number of extern variable groups are decided by the possiable maximum value of BSP_EINT_KEY_NUMBER. */
#define BSP_EINTKEY_DECLARE_1_EXTERN_VAR() \
    BSP_EINTKEY_DECLARE_EXTERN_VAR(0)
#define BSP_EINTKEY_DECLARE_2_EXTERN_VAR() \
    BSP_EINTKEY_DECLARE_1_EXTERN_VAR();\
    BSP_EINTKEY_DECLARE_EXTERN_VAR(1)
#define BSP_EINTKEY_DECLARE_3_EXTERN_VAR() \
    BSP_EINTKEY_DECLARE_2_EXTERN_VAR();\
    BSP_EINTKEY_DECLARE_EXTERN_VAR(2)
#define BSP_EINTKEY_DECLARE_4_EXTERN_VAR() \
    BSP_EINTKEY_DECLARE_3_EXTERN_VAR();\
    BSP_EINTKEY_DECLARE_EXTERN_VAR(3)
#define BSP_EINTKEY_DECLARE_5_EXTERN_VAR() \
    BSP_EINTKEY_DECLARE_4_EXTERN_VAR();\
    BSP_EINTKEY_DECLARE_EXTERN_VAR(4)
#define BSP_EINTKEY_DECLARE_6_EXTERN_VAR() \
    BSP_EINTKEY_DECLARE_5_EXTERN_VAR();\
    BSP_EINTKEY_DECLARE_EXTERN_VAR(5)
#define BSP_EINTKEY_DECLARE_7_EXTERN_VAR() \
    BSP_EINTKEY_DECLARE_6_EXTERN_VAR();\
    BSP_EINTKEY_DECLARE_EXTERN_VAR(6)
#define BSP_EINTKEY_DECLARE_8_EXTERN_VAR() \
    BSP_EINTKEY_DECLARE_7_EXTERN_VAR();\
    BSP_EINTKEY_DECLARE_EXTERN_VAR(7)

#if BSP_EINT_KEY_NUMBER == 1
    BSP_EINTKEY_DECLARE_1_EXTERN_VAR();
#elif BSP_EINT_KEY_NUMBER == 2
    BSP_EINTKEY_DECLARE_2_EXTERN_VAR();
#elif BSP_EINT_KEY_NUMBER == 3
    BSP_EINTKEY_DECLARE_3_EXTERN_VAR();
#elif BSP_EINT_KEY_NUMBER == 4
    BSP_EINTKEY_DECLARE_4_EXTERN_VAR();
#elif BSP_EINT_KEY_NUMBER == 5
    BSP_EINTKEY_DECLARE_5_EXTERN_VAR();
#elif BSP_EINT_KEY_NUMBER == 6
    BSP_EINTKEY_DECLARE_6_EXTERN_VAR();
#elif BSP_EINT_KEY_NUMBER == 7
    BSP_EINTKEY_DECLARE_7_EXTERN_VAR();
#elif BSP_EINT_KEY_NUMBER == 8
    BSP_EINTKEY_DECLARE_8_EXTERN_VAR();
#endif

/* config the GPIO port, eint mode, eint number, key data, active level */
#define BSP_EINTKEY_DRIVER_INIT_1_EINTKEY(available_key_nums)\
    BSP_EINTKEY_DRIVER_INIT(0, available_key_nums)
#define BSP_EINTKEY_DRIVER_INIT_2_EINTKEY(available_key_nums)\
    BSP_EINTKEY_DRIVER_INIT_1_EINTKEY(available_key_nums);\
    BSP_EINTKEY_DRIVER_INIT(1, available_key_nums)
#define BSP_EINTKEY_DRIVER_INIT_3_EINTKEY(available_key_nums)\
    BSP_EINTKEY_DRIVER_INIT_2_EINTKEY(available_key_nums);\
    BSP_EINTKEY_DRIVER_INIT(2, available_key_nums)
#define BSP_EINTKEY_DRIVER_INIT_4_EINTKEY(available_key_nums)\
    BSP_EINTKEY_DRIVER_INIT_3_EINTKEY(available_key_nums);\
    BSP_EINTKEY_DRIVER_INIT(3, available_key_nums)
#define BSP_EINTKEY_DRIVER_INIT_5_EINTKEY(available_key_nums)\
    BSP_EINTKEY_DRIVER_INIT_4_EINTKEY(available_key_nums);\
    BSP_EINTKEY_DRIVER_INIT(4, available_key_nums)
#define BSP_EINTKEY_DRIVER_INIT_6_EINTKEY(available_key_nums)\
    BSP_EINTKEY_DRIVER_INIT_5_EINTKEY(available_key_nums);\
    BSP_EINTKEY_DRIVER_INIT(5, available_key_nums)
#define BSP_EINTKEY_DRIVER_INIT_7_EINTKEY(available_key_nums)\
    BSP_EINTKEY_DRIVER_INIT_6_EINTKEY(available_key_nums);\
    BSP_EINTKEY_DRIVER_INIT(6, available_key_nums)
#define BSP_EINTKEY_DRIVER_INIT_8_EINTKEY(available_key_nums)\
    BSP_EINTKEY_DRIVER_INIT_7_EINTKEY(available_key_nums);\
    BSP_EINTKEY_DRIVER_INIT(7, available_key_nums)

void bsp_eint_key_custom_init(void)
{
    uint32_t available_key_nums = 0;

#if BSP_EINT_KEY_NUMBER == 1
    BSP_EINTKEY_DRIVER_INIT_1_EINTKEY(available_key_nums);
#elif BSP_EINT_KEY_NUMBER == 2
    BSP_EINTKEY_DRIVER_INIT_2_EINTKEY(available_key_nums);
#elif BSP_EINT_KEY_NUMBER == 3
    BSP_EINTKEY_DRIVER_INIT_3_EINTKEY(available_key_nums);
#elif BSP_EINT_KEY_NUMBER == 4
    BSP_EINTKEY_DRIVER_INIT_4_EINTKEY(available_key_nums);
#elif BSP_EINT_KEY_NUMBER == 5
    BSP_EINTKEY_DRIVER_INIT_5_EINTKEY(available_key_nums);
#elif BSP_EINT_KEY_NUMBER == 6
    BSP_EINTKEY_DRIVER_INIT_6_EINTKEY(available_key_nums);
#elif BSP_EINT_KEY_NUMBER == 7
    BSP_EINTKEY_DRIVER_INIT_7_EINTKEY(available_key_nums);
#elif BSP_EINT_KEY_NUMBER == 8
    BSP_EINTKEY_DRIVER_INIT_8_EINTKEY(available_key_nums);
#endif
    if (0 == available_key_nums) {
        assert(0 && "[eint_key]eint key pin has not been configured by ept tool on GPIO page, please check BSP_EINTKEYx_PIN\r\n");
    }
}

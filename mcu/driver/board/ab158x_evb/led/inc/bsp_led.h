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

#ifndef __BSP_LED_H__
#define __BSP_LED_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hal.h"
#include "hal_isink.h"

/**
* @addtogroup BSP
* @{
* @addtogroup LED
* @{
* - BSP LED example code. \n
*  - Step 1. Call #bsp_led_init() to initial bsp led driver.
*  - Step 2. Call #bsp_led_config() to config led timing & callback information.
*  - Step 3. Call #bsp_led_start() make led begin ligthen.
*  - Step 4. Call #bsp_led_stop() make led suspend.
*  - Step 5. Call #bsp_led_deinit() deinit bsp led when not using led.
*  - sample code:
*    @code
*       uint8_t             led_id = BSP_LED_CHANNEL_0;
*       bsp_led_config_t    led_cfg;
*       bsp_led_state_t     led_sta;
*       //delay 2s then make led flashing twice, and then
*       memset(&led_cfg, 0 ,sizeof(led_cfg));
*        led_cfg.onoff = 1;      //enable led on
*       led_cfg.time_unit = 100; // time unit for t0/t1/t2/t3(unit:ms)
*       led_cfg.t0 = 20;         //delay start time
*        led_cfg.t1 = 10;        //led on time in one cycle
*        led_cfg.t2 = 10;        //led off time in one cycle
*        led_cfg.repeat_t1t2 = 2;//led repeat t0t1 times. if equal 0, led always repeat t1t2
*        led_cfg.t3 = 200;       //led off time after finish t1t2 repeat
*        led_cfg.repeat_ext  = 0;//ext loop times.if equal 0, led always repeat (t1t2_repeat + t3)
*        led_cfg.brightness  = 50;//brightness(0~255)
*
*       led_sta = bsp_led_init(led_id, BSP_LED_MODE_BLINK); //led have BSP_LED_MODE_BLINK and BSP_LED_MODE_BREATH mode
*       if(led_sta != BSP_LED_STATUS_OK){
*            printf("[test][led] bsp_led_init err %d", led_sta);
*         return;
*        }
*        led_sta = bsp_led_config(led_id, &led_cfg);
*        if(led_sta != BSP_LED_STATUS_OK){
*            printf("[test][led] bsp_led_config err %d", led_sta);
*            return;
*        }
*        led_sta = bsp_led_start(led_id);
*        if(led_sta != BSP_LED_STATUS_OK){
*            printf("[test][led] bsp_led_start err %d", led_sta);
*            return;
*        }
*
*        //if not want to using, need stop & deinit
*        led_sta = bsp_led_stop(led_id);
*        if(led_sta != BSP_LED_STATUS_OK){
*            printf("[test][led] bsp_led_start err %d", led_sta);
*            return;
*        }
*        led_sta = bsp_led_deinit(led_id);
*        if(led_sta != BSP_LED_STATUS_OK){
*            printf("[test][led] bsp_led_start err %d", led_sta);
*            return;
*        }
*    @endcode
*
* @}
* @}
*/


#define log_bsp_led_info(fmt, cnt, ...)  LOG_MSGID_I(bsp_led, fmt, cnt, ##__VA_ARGS__)
#define log_bsp_led_warn(fmt, cnt, ...)  LOG_MSGID_W(bsp_led, fmt, cnt, ##__VA_ARGS__)
#define log_bsp_led_error(fmt, cnt, ...) LOG_MSGID_E(bsp_led, fmt, cnt, ##__VA_ARGS__)

enum {
    BSP_LED_IOCTRL_GET_CYCLE,
    BSP_LED_IOCTRL_GET_EXT_CYCLE
};

typedef void (*bsp_led_callback_t)(uint8_t led, void *);

typedef enum {
    BSP_LED_STATUS_ERROR_BUSY          = -5,
    BSP_LED_STATUS_ERROR_NO_DEVICE     = -4,
    BSP_LED_STATUS_ERROR_CHANNEL       = -3,       /**< The ISINK error channel. */
    BSP_LED_STATUS_ERROR_INVALID_PARAMETER   = -2, /**< An error occurred, invalid parameter was given. */
    BSP_LED_STATUS_ERROR               = -1,        /**< The ISINK function error occurred. */
    BSP_LED_STATUS_OK   = 0                         /**< The ISINK operation completed successfully.*/
} bsp_led_status_t;

typedef enum {
    BSP_LED_MODE_BREATH = 0,
    BSP_LED_MODE_BLINK  = 1,
} bsp_led_mode_t;

#pragma  pack(1)

typedef struct {
    uint8_t     time_unit;    /*t 0~t3 time unit */
    uint8_t     t0;           /* time delay start t1t2_repeat */
    uint8_t     t1;           /* on time in one cycle(eqaul to 0, means led always off,when t2 !=0) */
    uint8_t     t2;           /* off time in one cycle(eqaul to 0, means led always on,when t1 !=0) */
    uint8_t     t3;           /* led idle time after finish t1t2_repeat*/
    uint16_t    repeat_t1t2;  /* t0t1  repeat times*/
    uint16_t    repeat_ext;   /* t0~t3 repeat times*/
    uint8_t     brightness;   /* led brightness*/
    uint8_t     onoff;        /* led on off(1:on,0:off)*/
    uint8_t     reserv;
    uint32_t    sync_time;
    void        *user_data;
    bsp_led_callback_t      call_back;
} bsp_led_config_t;
#pragma  pack()


enum {
    BSP_ISINK_LED_STATE_INIT_MASK       = 0x1,
    BSP_ISINK_LED_STATE_STARTED_MASK    = 0x2,
    BSP_ISINK_LED_STATE_ALWAYS_OFF_MASK = 0x4,
    BSP_ISINK_LED_STATE_ALWAYS_ON_MASK  = 0x8,
    BSP_ISINK_LED_STATE_KEY_MASK        = 0xEA00,
};

/**
 * @brief This function initialize the led driver.
 * @param[in] ledx is a led number id.
 * @param[in] mode is a mode that led display(breath &or blink mode).
 * @return  #HAL_RTC_STATUS_OK, the operation completed successfully.
 */
bsp_led_status_t    bsp_led_init(uint8_t ledx, bsp_led_mode_t mode);



/**
 * @brief This function deinit the led driver.
 * @param[in] ledx is a led number id.
 * @return  #HAL_RTC_STATUS_OK, the operation completed successfully.
 */
bsp_led_status_t    bsp_led_deinit(uint8_t ledx);


/**
 * @brief This function config led timing and others.
 * @param[in] ledx is a led number id.
 * @param[in] cfg is the led timing setting.
 * @return  #HAL_RTC_STATUS_OK, the operation completed successfully.
 */
bsp_led_status_t    bsp_led_config(uint8_t ledx, bsp_led_config_t *cfg);


/**
 * @brief This function start led flashing.
 * @param[in] ledx is a led number id.
 * @return  #HAL_RTC_STATUS_OK, the operation completed successfully.
 */
bsp_led_status_t    bsp_led_start(uint8_t ledx);


/**
 * @brief This function will suspend led flashing. user can resume flashing by call bsp_led_start.
 * @param[in] ledx is a led number id.
 * @return  #HAL_RTC_STATUS_OK, the operation completed successfully.
 */
bsp_led_status_t    bsp_led_stop(uint8_t ledx);


/**
 * @brief This function is resevered for furture using, currently not implement.
 * @param[in] ledx is a led number id.
 * @param[in] cmd is cmd that driver using.
 * @param[in] option is a parameter for extend..
 * @return #BSP_LED_STATUS_OK, the operation completed successfully, else failed.
 */
int                 bsp_led_ioctrl(uint32_t ledx, uint32_t cmd, uint32_t option);
#ifdef __cplusplus
}
#endif

#endif




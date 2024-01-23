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

#ifndef __APP_KEY_REMAP_H__
#define __APP_KEY_REMAP_H__

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    /* Key events are not reported. */
    remap_disable = 0,

    remap_to_left_key,
    remap_to_right_key,
    remap_to_middle_key,
    remap_to_fourth_key,
    remap_to_fifth_key,

    /* Remaps the keys on the mouse to certain keys
     * on the keyboard for the convenience of the user.
     */
    remap_to_kbd_key,

    /* wired mode: app_mouse_report_rate_change
     * wireless mode: send RACE_SET_REPORT_RATE to dongle
     */
    remap_to_report_rate_switch,

    /* Remap mouse buttons as macro functions for
     * quick implementation of certain keyboard combinations.
     */
    remap_to_macro,

    remap_to_fire_key,
    REMAP_RESERVED_FUNCTION,         /* reserved function */
    remap_to_dpi_control,
} key_remap_type_e;


/* Each remapping function can be described using up to 5 bytes. */
#define     APP_KEY_REMAP_MAX_FUNC_DESC_LEN         5
#define     APP_KEY_NUMBER                          5

#define     APP_KEY_REMAP_PROFILE_LEN               (APP_KEY_NUMBER*APP_KEY_REMAP_MAX_FUNC_DESC_LEN)
#define     APP_KEY_REMAP_MAX_PROFILE_IDX           3
#define     APP_KEY_REMAP_PROFILE_COUNT             (APP_KEY_REMAP_MAX_PROFILE_IDX+1)

#define     APP_KEY_REMAP_MAX_DPI_COUNT             5

#define     APP_KEY_REMAP_MACRO_SUPPORT_KEY_NUM     35
#define     APP_KEY_REMAP_MACRO_KEY_EVENT_IN_BYTES  5
#define     APP_KEY_REMAP_MACRO_NAME_LEN            16 /* include ending '\0' */
#define     APP_KEY_REMAP_MACRO_PLAY_MODE_LEN       2
#define     APP_KEY_REMAP_MACRO_DESCRIP_LEN         (uint8_t)(APP_KEY_REMAP_MACRO_NAME_LEN + \
                APP_KEY_REMAP_MACRO_PLAY_MODE_LEN)

#define     APP_KEY_REMAP_MACRO_MAX_LEN             (uint8_t)(APP_KEY_REMAP_MACRO_NAME_LEN + \
                APP_KEY_REMAP_MACRO_PLAY_MODE_LEN + \
                APP_KEY_REMAP_MACRO_KEY_EVENT_IN_BYTES*APP_KEY_REMAP_MACRO_SUPPORT_KEY_NUM)


/* offset of every key remap value */
#define     APP_KEY_PROFILE_CFG_MAX_LEN             (uint32_t)( \
                    APP_KEY_NUMBER*APP_KEY_REMAP_MAX_FUNC_DESC_LEN \
                )

extern volatile uint8_t g_key_output;
#define     APP_KEY_REMAP_GET_KEY_OUTPUT() g_key_output

void app_key_remap_key_status_update(uint8_t *data, uint8_t len, uint8_t plc_reason);
void app_key_remap_set_pattern(uint8_t key_idx, uint8_t *pdata, uint32_t len);
void app_key_remap_disconnection_request(void);
bool app_key_remap_init(void);

#endif //__APP_KEY_REMAP_H__


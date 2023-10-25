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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#if defined(AIR_PURE_GAMING_MS_ENABLE)

#include "FreeRTOS.h"
#include "timers.h"
#include "hal_nvic.h"
#include "app_key_remap.h"
#include "memory_attribute.h"
#include "apps_hid_keycode_id.h"
#include "apps_debug.h"
#include "app_dongle_ull_le_hid.h"

/* USB Middleware includes */
#include "usb.h"
#include "usb_resource.h"
#include "usbhid_drv.h"

#include "nvkey.h"
#include "nvkey_id_list.h"

/* macro can control the mouse button state and requires special coding. */
#define    SPECIAL_MACRO_LEFT_KEY           ((uint8_t)0xF0)
#define    SPECIAL_MACRO_RIGHT_KEY          ((uint8_t)0xF1)
#define    SPECIAL_MACRO_MIDDLE_KEY         ((uint8_t)0xF2)

#define    MEASURE_PROCESSING_TIME          0

#define    CTRL_VALUE_IN_COMBO              (uint8_t)0x01
#define    SHIFT_VALUE_IN_COMBO             (uint8_t)0x02
#define    ALT_VALUE_IN_COMBO               (uint8_t)0x04
#define    WIN_VALUE_IN_COMBO               (uint8_t)0x08

#define    NO_KBD_REPORT_DEFINITION         7   /* all 3 bits is 1 */
#define    NO_MACRO_PLAY_REQUEST            7
#define    NO_FIRE_KEY_REQUEST              7

/* 0~4 assign to standard keyboard report key_code */
#define    STD_KBD_REPORT_KEY_CODE_COUNT    5
#define    LEFT_KEY_SIG_IDX                 5
#define    RIGHT_KEY_SIG_IDX                6
#define    MIDDLE_KEY_SIG_IDX               7
#define    WIN_SIG_IDX                      8
#define    ALT_SIG_IDX                      9
#define    SHIFT_SIG_IDX                    10
#define    CTRL_SIG_IDX                     11

// 1 => key function
// 18 => macro name & play mode
// 5*35 => macro has max 35 record
#define    KEYMAPPING_PATTERN_SIZE          (1+18+5*35)
#define    KEY_REMAP_LOG_TAG                "[KR]"

#define    REPORT_ID_KEYBOARD               BT_HID_REPORT_ID_KB
#define    REPORT_ID_KEYBOARD_CONSUMER      BT_HID_REPORT_ID_CONSUMER
#define    STATUS_OK                        0


/* According to the definition of T_KEY_REPORT_S,
 * 5 keys can be sent at the same time,
 * and extra 4 key combinations:
 * Shift, Ctrl, Alt, and GUI (Win in Windows or Command in Mac or sth. else )
 * and macro can also control left, middle, right mouse buttons. */
#define    MACRO_TIME_CNT_NUMBER            (uint32_t)5+3+4


#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif /* PACKED */

#if MEASURE_PROCESSING_TIME
#define    SET_GPIO_HIGH(CH) { \
                hal_nvic_save_and_set_interrupt_mask(&irq_mask);\
                IOMUX.GPIO.reg |= BIT_MASK32(CH); \
            }

#define    SET_GPIO_LOW(CH) { \
                IOMUX.GPIO.reg &= ~(BIT_MASK32(CH)); \
                hal_nvic_restore_interrupt_mask(irq_mask); \
            }

#else
#define    SET_GPIO_HIGH(CH)
#define    SET_GPIO_LOW(CH)
#endif

/******************************************************************************/
/*  enum definition                                                           */
/******************************************************************************/
typedef enum {
    repeat_specified_times,
    repeat_until_the_key_release,
    repeat_until_any_key_press,
} macro_play_mode_e;

typedef enum {
    standard_kbd_report,
    consumer_kbd_report,
} key_report_type_e;


typedef enum {
    key_event_none,
    key_event_press,
    key_event_release,
} key_event_e;


/******************************************************************************/
/*  callback function definition                                              */
/******************************************************************************/
typedef uint32_t (*key_remap_callback_t)(uint8_t key_idx);

/******************************************************************************/
/*  structure or union definition                                                      */
/******************************************************************************/
typedef struct {
    uint8_t id;
    uint8_t combo;
    uint8_t keycode[5];
}PACKED T_KEY_REPORT_S;

//[id] [Key1_L] [Key1_H] [Key2_L] [Key2_H] [number]
typedef struct {
    uint8_t id;
    uint16_t keycode[3];
}PACKED T_KEY_REPORT_CONSUMER_S;

typedef union {
    T_KEY_REPORT_S std;             /* standard key code */
    T_KEY_REPORT_CONSUMER_S csm;    /* extended consumer key code */
} key_report_u;

typedef struct {
    uint8_t key_code;               /* only standard key code */
    uint16_t start_time_in_ms;
    uint16_t duration_time_in_ms;
} __attribute__((packed)) macro_record_t;

typedef struct {
    char name[16];          /* The macro name represented by the ascii code including the terminator. */

    /* !!! NOTE !!!:
     * Can't use macro_play_mode_e directly here, because the compiler
     * allocates 4 bytes for the enumeration by default, which is not expected. */
    uint8_t play_mode;

    uint8_t repeat_times;

    /* It does not take up space, but programmer can use rcds to quickly refer to the subsequent array. */
    macro_record_t rcds[0];
} __attribute__((packed)) macro_play_param_t;


typedef struct {
    macro_play_param_t *p_param;

    uint8_t rcds_total_number;
    uint8_t rcds_play_idx;
    uint8_t key_idx;

    uint8_t left_key_sig        :1;
    uint8_t right_key_sig       :1;
    uint8_t middle_key_sig      :1;
    uint8_t win_sig             :1;
    uint8_t alt_sig             :1;
    uint8_t shift_sig           :1;
    uint8_t ctrl_sig            :1;
    uint8_t RESERVED_SIG        :1;

    uint32_t time_cnt[MACRO_TIME_CNT_NUMBER];
    uint32_t current_time_in_us;
    uint32_t end_time_in_us;

    uint8_t key_code_used_cnt;
    uint8_t update_kbd_report_req;
    uint8_t play_round;
} __attribute__((packed)) macro_descrip_t;

/* The data to be sent needs to be prepared during initial or set_key_map stage,
 * so the required data is pre-defined in the control block. */
typedef struct {
    /* The output information of some keyboard keys prepared in advance,
     * when it is detected that the physical key is pressed and
     * the physical key is mapped as keyboard key output,
     * directly use this variable to output. */
    key_report_u kbds[APP_KEY_NUMBER];
    T_KEY_REPORT_S empty_std_kbd;
    T_KEY_REPORT_CONSUMER_S empty_csm_kbd;

    /* Pre-defined callback functions to implement some functions
     * such as report rate switching, and DPI control. */
    key_remap_callback_t func_cb[APP_KEY_NUMBER];

    macro_descrip_t *p_current_macro;
    macro_descrip_t macro_descriptors[APP_KEY_NUMBER];

    /* A pointer to array of describing the function of each key. */
    uint8_t *p_func_descrip[APP_KEY_NUMBER];
    uint8_t *p_firekey_param;

    /* Each bit represents a key, and a value of 1 indicates that the physical key is pressed. */
    uint8_t mouse_key_i;
    uint8_t mouse_key_o;

    uint8_t firekey_phy_key;

    uint32_t firekey_req             :3;
    uint32_t firekey_stop_req        :1;
    uint32_t firekey_sig             :1;    // 5 bits
    uint32_t macro_play_req          :3;
    uint32_t macro_preempt_play_req  :3;
    uint32_t macro_stop_req          :1;    // 13 bits
    uint32_t macro_async_stop_req    :1;
    uint32_t send_std_kbd_req        :3;
    uint32_t send_csm_kbd_req        :3;
    uint32_t send_std_empty_kbd_req  :1;
    uint32_t send_csm_empty_kbd_req  :1;    // 21 bits
    uint32_t gpt_timer_active        :1;    // 22 bits dongle side timer always on
    uint32_t firekey_repeat_times    :8;    // 30 bits
    uint32_t during_resotre          :1;    // 31 bits
    uint32_t disconnection_req       :1;

    uint32_t routine_interval_in_us;
} key_remap_ctrl_block_t;


/******************************************************************************/
/*  function prototype                                                        */
/******************************************************************************/
static void app_key_remap_prepare_data_by_key_func(uint8_t key_idx, key_remap_type_e func, bool quietly);
static void app_key_remap_prepare_data(void);
void app_key_remap_print_array(uint8_t *ptr, uint32_t len);
static void app_key_remap_init_key_report(key_report_type_e type, key_report_u *p_report);
static void release_key_event_cb(uint8_t key_idx);
static uint32_t remap_disable_key_press_event_cb(uint8_t key_idx);
static uint32_t remap_mouse_key_press_event_cb(uint8_t key_idx);
static uint32_t remap_kbd_key_press_event_cb(uint8_t key_idx);
static uint32_t remap_macro_key_press_event_cb(uint8_t key_idx);
static uint32_t remap_fire_key_press_event_cb(uint8_t key_idx);
static uint8_t app_mouse_attach_keyboard_report(T_KEY_REPORT_S *p_kbd);
static uint8_t app_mouse_attach_consumer_report(T_KEY_REPORT_CONSUMER_S *p_csm);
#define SET_KEY_OUTPUT(mouse_keys) {g_key_output = mouse_keys;}

ATTR_RODATA_IN_TCM const uint8_t s_mouse_button_mask[] = {
    /* L,    M,    R,   S1,   S2 */
    0x01, 0x02, 0x04, 0x10, 0x08
};

volatile uint8_t g_key_output;
static key_remap_ctrl_block_t krcb;
ATTR_RWDATA_IN_TCM static uint8_t s_app_key_remap_events[APP_KEY_NUMBER] = {
    (uint8_t)key_event_release,
    (uint8_t)key_event_release,
    (uint8_t)key_event_release,
    (uint8_t)key_event_release,
    (uint8_t)key_event_release,
};

ATTR_ZIDATA_IN_TCM static uint8_t s_key_remap_profile[APP_KEY_NUMBER][KEYMAPPING_PATTERN_SIZE] = {0};
static uint8_t s_key_remap_profile_valid_len[APP_KEY_NUMBER] = {0};

void app_key_remap_print_array(uint8_t *ptr, uint32_t len)
{
    APPS_LOG_DUMP_I(KEY_REMAP_LOG_TAG"output: %d", ptr, len, len);
}

ATTR_TEXT_IN_TCM
static uint8_t* key_remap_get_profile_description(uint8_t key_idx)
{
    return (uint8_t*)(&(s_key_remap_profile[key_idx][0]));
}

ATTR_TEXT_IN_TCM
static bool shift_judgement(uint8_t key_code)
{
    if((key_code == KC_LSHIFT) || (key_code == KC_RSHIFT)) {
        return true;
    }
    return false;
}

ATTR_TEXT_IN_TCM
static bool ctrl_judgement(uint8_t key_code)
{
    if((key_code == KC_LCTRL) ||
       (key_code == KC_RCTRL)
    ){
        return true;
    }
    return false;
}

ATTR_TEXT_IN_TCM
static bool alt_judgement(uint8_t key_code)
{
    if((key_code == KC_LALT) ||
       (key_code == KC_RALT)
    ){
        return true;
    }
    return false;
}

ATTR_TEXT_IN_TCM
static bool win_judgement(uint8_t key_code)
{
    if((key_code == KC_LGUI) ||
       (key_code == KC_RGUI)) {
        return true;
    }
    return false;
}


static const char *p_macro_play_mode_str[] = {
    "repeat_specified_times",
    "repeat_until_the_key_release",
    "repeat_until_any_key_press",
};


ATTR_TEXT_IN_TCM
static void app_key_remap_stop_play_macro(void)
{
    uint8_t key_idx = krcb.macro_play_req;

    krcb.macro_play_req = NO_MACRO_PLAY_REQUEST;
    krcb.p_current_macro->play_round = 0;
    krcb.p_current_macro->rcds_play_idx = 0;
    krcb.p_current_macro->key_code_used_cnt = 0;
    app_key_remap_init_key_report(standard_kbd_report, &(krcb.kbds[key_idx]));
    krcb.p_current_macro->update_kbd_report_req = 0;
    krcb.p_current_macro->current_time_in_us = 0;
    krcb.p_current_macro->left_key_sig = 0;
    krcb.p_current_macro->right_key_sig = 0;
    krcb.p_current_macro->middle_key_sig = 0;
    krcb.p_current_macro->win_sig = 0;
    krcb.p_current_macro->alt_sig = 0;
    krcb.p_current_macro->shift_sig = 0;
    krcb.p_current_macro->ctrl_sig = 0;
    krcb.p_current_macro->RESERVED_SIG = 0;
    memset(&(krcb.p_current_macro->time_cnt[0]), 0x0, sizeof(uint32_t)*MACRO_TIME_CNT_NUMBER);

    krcb.p_current_macro = (macro_descrip_t *)NULL;
}

ATTR_TEXT_IN_TCM
static void refresh_data_about_macro(uint32_t current_time_in_us, uint8_t key_idx, macro_play_param_t *p_macro_play_param)
{
    uint8_t idx, key_code_idx;
    uint32_t time_cnt = 0;
    macro_descrip_t *p_macro_descrip = &(krcb.macro_descriptors[key_idx]);
    bool empty_kbd_report = (p_macro_descrip->key_code_used_cnt == 0) ? (true) : (false);
    uint8_t macro_play_round, repeat_times, play_mode;

    // APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"MET: %d us, CT: %d, RPI: %d", 3,
    //     p_macro_descrip->end_time_in_us,
    //     current_time_in_us,
    //     p_macro_descrip->rcds_play_idx
    // );

    if(p_macro_descrip->end_time_in_us < current_time_in_us){
        p_macro_descrip->play_round++;
        p_macro_descrip->rcds_play_idx = 0;
        p_macro_descrip->key_code_used_cnt = 0;
        app_key_remap_init_key_report(standard_kbd_report, &(krcb.kbds[key_idx]));
        p_macro_descrip->update_kbd_report_req = 1;
        p_macro_descrip->current_time_in_us = 0;

        macro_play_round = p_macro_descrip->play_round;
        repeat_times = p_macro_descrip->p_param->repeat_times;
        play_mode = p_macro_descrip->p_param->play_mode;
        if(((macro_play_mode_e)play_mode == repeat_specified_times) && \
           (macro_play_round >= repeat_times)){
            krcb.macro_stop_req = 1;
            return;
        }

        /* The design of the reference mouse is that the stop marker is
         * asynchronous and must wait until a round of macro playback is complete
         * before detecting whether a stop is required. */
        if(krcb.macro_async_stop_req == 1){
            krcb.macro_stop_req = 1;
            krcb.macro_async_stop_req = 0;
            // APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"async stop macro", 0);
            return;
        }
        /* refill first package for next repeat playback */
        current_time_in_us = 0;
    }

    p_macro_descrip->current_time_in_us = current_time_in_us;

    for(idx = p_macro_descrip->rcds_play_idx;    \
        idx < p_macro_descrip->rcds_total_number;    \
        idx++)
    {
        // APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"STIM(%d): %d", 2, idx, p_macro_play_param->rcds[idx].start_time_in_ms);
        if(p_macro_play_param->rcds[idx].start_time_in_ms > (current_time_in_us/1000)){
            break;
        }
        p_macro_descrip->rcds_play_idx = idx + 1;
        time_cnt = p_macro_play_param->rcds[idx].duration_time_in_ms * 1000 / krcb.routine_interval_in_us;
        if(win_judgement(p_macro_play_param->rcds[idx].key_code)){
            krcb.kbds[key_idx].std.combo |= WIN_VALUE_IN_COMBO;
            p_macro_descrip->win_sig = 1;
            p_macro_descrip->time_cnt[WIN_SIG_IDX] = time_cnt;
        } else if(alt_judgement(p_macro_play_param->rcds[idx].key_code)) {
            krcb.kbds[key_idx].std.combo |= ALT_VALUE_IN_COMBO;
            p_macro_descrip->alt_sig = 1;
            p_macro_descrip->time_cnt[ALT_SIG_IDX] = time_cnt;
        } else if(shift_judgement(p_macro_play_param->rcds[idx].key_code)){
            krcb.kbds[key_idx].std.combo |= SHIFT_VALUE_IN_COMBO;
            p_macro_descrip->shift_sig = 1;
            p_macro_descrip->time_cnt[SHIFT_SIG_IDX] = time_cnt;
        } else if(ctrl_judgement(p_macro_play_param->rcds[idx].key_code)){
            krcb.kbds[key_idx].std.combo |= CTRL_VALUE_IN_COMBO;
            p_macro_descrip->ctrl_sig = 1;
            p_macro_descrip->time_cnt[CTRL_SIG_IDX] = time_cnt;
        } else if(p_macro_play_param->rcds[idx].key_code == SPECIAL_MACRO_LEFT_KEY){
            p_macro_descrip->left_key_sig = 1;
            p_macro_descrip->time_cnt[LEFT_KEY_SIG_IDX] = time_cnt;
        } else if(p_macro_play_param->rcds[idx].key_code == SPECIAL_MACRO_RIGHT_KEY){
            p_macro_descrip->right_key_sig = 1;
            p_macro_descrip->time_cnt[RIGHT_KEY_SIG_IDX] = time_cnt;
        } else if(p_macro_play_param->rcds[idx].key_code == SPECIAL_MACRO_MIDDLE_KEY){
            p_macro_descrip->middle_key_sig = 1;
            p_macro_descrip->time_cnt[MIDDLE_KEY_SIG_IDX] = time_cnt;
        } else {

            /* Except for the special buttons, common handles flow. */
            if(empty_kbd_report){
                p_macro_descrip->time_cnt[p_macro_descrip->key_code_used_cnt] = time_cnt;
                krcb.kbds[key_idx].std.keycode[p_macro_descrip->key_code_used_cnt] = p_macro_play_param->rcds[idx].key_code;
                p_macro_descrip->key_code_used_cnt++;
                p_macro_descrip->update_kbd_report_req = 1;
            } else {
                for(key_code_idx = 0; key_code_idx < STD_KBD_REPORT_KEY_CODE_COUNT; key_code_idx++){
                    if(krcb.kbds[key_idx].std.keycode[key_code_idx] == p_macro_play_param->rcds[idx].key_code){
                        break;
                    }
                    if(krcb.kbds[key_idx].std.keycode[key_code_idx] == 0){
                        krcb.kbds[key_idx].std.keycode[key_code_idx] = p_macro_play_param->rcds[idx].key_code;
                        p_macro_descrip->time_cnt[key_code_idx] = time_cnt;
                        p_macro_descrip->key_code_used_cnt++;
                        p_macro_descrip->update_kbd_report_req = 1;
                        break;
                    }
                }
            }

            if(p_macro_descrip->key_code_used_cnt >= STD_KBD_REPORT_KEY_CODE_COUNT){
                // TODO: The macro pattern needs to be checked in advance when setting the race command.
                APPS_LOG_MSGID_E(KEY_REMAP_LOG_TAG"macro pattern may wrong.", 0);
            }

            // APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"KCUC: %d ", 1, p_macro_descrip->key_code_used_cnt);
            // app_key_remap_print_array((void*)(&krcb.kbds[key_idx]), sizeof(key_report_u));
        }
    }
}

static key_report_type_e
fill_kbd_report_by_func_descriptor(uint8_t *p_key_func_descrip, uint8_t key_idx)
{
    uint16_t csm_key_code = *(p_key_func_descrip + 1) | (*(p_key_func_descrip + 2) << 8);
    // APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"consumer key code is 0x%04X", 1, csm_key_code);
    if(csm_key_code == 0x0){
        app_key_remap_init_key_report(standard_kbd_report, &(krcb.kbds[key_idx]));
        krcb.kbds[key_idx].std.combo = *(p_key_func_descrip + 4);
        /* This means that it is not possible to respond to the situation
         * where two physical keys are mapped to keyboard key functions and
         * the two physical keys are pressed at the same time. */
        krcb.kbds[key_idx].std.keycode[key_idx] = *(p_key_func_descrip + 3);
        return standard_kbd_report;
    } else {
        app_key_remap_init_key_report(consumer_kbd_report, &(krcb.kbds[key_idx]));
        krcb.kbds[key_idx].csm.keycode[0] = csm_key_code;
        return consumer_kbd_report;
    }
}

static void app_key_remap_prepare_data_by_key_func(uint8_t key_idx, key_remap_type_e func, bool quietly)
{
    uint8_t *p_key_func_descrip = key_remap_get_profile_description(key_idx);
    uint8_t idx;
    macro_play_param_t *p_macro_play_param;
    macro_descrip_t *p_macro_descrip = &(krcb.macro_descriptors[key_idx]);
    uint32_t end_time_in_us;

    switch(func){
        case remap_disable:
            krcb.func_cb[key_idx] = remap_disable_key_press_event_cb;
            break;
        case remap_to_left_key:
        case remap_to_right_key:
        case remap_to_middle_key:
        case remap_to_fourth_key:
        case remap_to_fifth_key:
            /* fall through, no special data need to be prepared */
            krcb.func_cb[key_idx] = remap_mouse_key_press_event_cb;
            break;
        case remap_to_kbd_key:
            /* 1 byte: key remap type
             * 2 bytes: consumer key code
             * 1 byte: std key code
             * 1 byte: combo
             */
            fill_kbd_report_by_func_descriptor(p_key_func_descrip, key_idx);
            krcb.func_cb[key_idx] = remap_kbd_key_press_event_cb;
            break;
        case remap_to_report_rate_switch:
            break;
        case remap_to_macro:
            /* not need free p_macro_descrip->p_param because its static memory */
            memset(p_macro_descrip, 0x0, sizeof(macro_descrip_t));
            /* 1 byte key function type + pay load */
            p_macro_descrip->p_param = (macro_play_param_t*)(key_remap_get_profile_description(key_idx) + 1);
            p_macro_play_param = (macro_play_param_t *)(p_macro_descrip->p_param);
            /* one byte for key function type
             * 18 bytes for macro name in ASCII
             */
            p_macro_descrip->rcds_total_number = (s_key_remap_profile_valid_len[key_idx] - 1 - 18)/5;
            p_macro_descrip->rcds_play_idx = 0;
            p_macro_descrip->key_code_used_cnt = 0;
            p_macro_descrip->key_idx = key_idx;
            p_macro_descrip->play_round = 0;
            app_key_remap_init_key_report(standard_kbd_report, &(krcb.kbds[key_idx]));
            p_macro_descrip->end_time_in_us = 0;
            for(idx = 0; idx < p_macro_descrip->rcds_total_number; idx++){
                end_time_in_us = 1000*(p_macro_play_param->rcds[idx].start_time_in_ms + \
                                        p_macro_play_param->rcds[idx].duration_time_in_ms);
                if(end_time_in_us > p_macro_descrip->end_time_in_us){
                    p_macro_descrip->end_time_in_us = end_time_in_us;
                }
            }

            /* Only the first package of data needs to be prepared,
                * so if the start time is not 0, there is no need to think too much. */
            refresh_data_about_macro(0, key_idx, p_macro_play_param);

            /* The first package of data prepared here, but program need to pay attention to
                * the time in the routine is from the beginning, in order to avoid sending
                * more keys such as shift, program need to clear these variables
                * after preparing the data of first package. */
            p_macro_descrip->rcds_play_idx = 0;
            p_macro_descrip->key_code_used_cnt = 0;

            if(!quietly){
                APPS_LOG_I(KEY_REMAP_LOG_TAG"macro name: %s", p_macro_play_param->name);
                APPS_LOG_I(KEY_REMAP_LOG_TAG"macro play mode: %d(%s) and repeat %d times",
                    p_macro_play_param->play_mode,
                    p_macro_play_mode_str[p_macro_play_param->play_mode],
                    p_macro_play_param->repeat_times
                );
                APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"%d records", 1, p_macro_descrip->rcds_total_number);

                APPS_LOG_MSGID_D(KEY_REMAP_LOG_TAG"prepared keyboard report: ", 0);
                app_key_remap_print_array((void*)(&krcb.kbds[key_idx]), sizeof(key_report_u));
                for(idx = 0; idx < MACRO_TIME_CNT_NUMBER; idx++){
                    APPS_LOG_MSGID_D(KEY_REMAP_LOG_TAG"TC[%d]: %d", 2, idx, p_macro_descrip->time_cnt[idx]);
                }
            }

            krcb.func_cb[key_idx] = remap_macro_key_press_event_cb;
            break;
        case remap_to_fire_key:
            if(!quietly){
                APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"remap key %d to fire key, interval is %d ms and ", 2, key_idx, *(p_key_func_descrip + 2));
                if(*(p_key_func_descrip + 1) == 0){
                    APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"continue to output fire signal( left key ) until the key is released.", 0);
                } else {
                    APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"repeat fire %d times.", 1, *(p_key_func_descrip + 1));
                }
            }
            krcb.func_cb[key_idx] = remap_fire_key_press_event_cb;
            break;
        default:
            break;
    }
}

static const char *p_func_str[] = {
    "disable",
    "left_key",
    "right_key",
    "middle_key",
    "fourth_key",
    "fifth_key",
    "kbd_key",
    "report_rate_switch",
    "macro",
    "fire_key",
    "RESERVED",
    "dpi_control",
};

static void app_key_remap_prepare_data(void)
{
    uint8_t key_idx, *p_key_func_descrip;
    key_remap_type_e func;

    /* All other global variables are initialized by the system during the boot phase,
     * and only those parts with default values other than 0 need to be initialized. */
    krcb.firekey_req = NO_FIRE_KEY_REQUEST;
    krcb.macro_play_req = NO_MACRO_PLAY_REQUEST;
    krcb.macro_preempt_play_req = NO_MACRO_PLAY_REQUEST;
    krcb.send_std_kbd_req = NO_KBD_REPORT_DEFINITION;
    krcb.send_csm_kbd_req = NO_KBD_REPORT_DEFINITION;
    app_key_remap_init_key_report(standard_kbd_report, (key_report_u*)(&(krcb.empty_std_kbd)));
    app_key_remap_init_key_report(consumer_kbd_report, (key_report_u*)(&(krcb.empty_csm_kbd)));

    for(key_idx = 0; key_idx < APP_KEY_NUMBER; key_idx++){
        p_key_func_descrip = key_remap_get_profile_description(key_idx);
        func = *p_key_func_descrip;
        krcb.p_func_descrip[key_idx] = p_key_func_descrip;
        APPS_LOG_I(KEY_REMAP_LOG_TAG"remap key %d => %d(%s)", key_idx, func, p_func_str[func]);
        app_key_remap_prepare_data_by_key_func(key_idx, func, false);
    }
    for(key_idx = 0; key_idx < APP_KEY_NUMBER; key_idx++){
        APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"keyboard report address: 0x%08X", 1, &(krcb.kbds[key_idx]));
        APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"key remap callback address: 0x%08X", 1, krcb.func_cb[key_idx]);
    }
}

static void app_key_remap_init_key_report(key_report_type_e type, key_report_u *p_report)
{
    memset(p_report, 0, sizeof(key_report_u));
    switch(type){
        case standard_kbd_report:
            p_report->std.id = REPORT_ID_KEYBOARD;
            break;
        case consumer_kbd_report:
            p_report->csm.id = REPORT_ID_KEYBOARD_CONSUMER;
            break;
        default:
            break;
    }
}

ATTR_TEXT_IN_TCM
void app_key_remap_key_status_update(uint8_t *p_button_mask, uint8_t pkt_reason)
{
    SET_GPIO_HIGH(14);

    // APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"I:%02X", 1, *p_button_mask);

    static uint8_t pre_phy_button_mask = 0xFF;
    if((pre_phy_button_mask == *p_button_mask) || (pkt_reason != 0x0)){
        *p_button_mask = g_key_output;
        // APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"CO:%02X", 1, g_key_output);
        SET_GPIO_LOW(14);
        return;
    } else {
        pre_phy_button_mask = *p_button_mask;
    }

    uint8_t idx, key_mask;
    key_report_u *p_kbd = (key_report_u *)NULL;
    uint8_t send_kbd_res = 0xFF, func_type;

    krcb.mouse_key_i = *p_button_mask;
    krcb.mouse_key_o = *p_button_mask;

    for(idx = 0; idx < APP_KEY_NUMBER; idx++){
        key_mask = s_mouse_button_mask[idx];
        if(krcb.mouse_key_i & key_mask){
            if(s_app_key_remap_events[idx] == (uint8_t)key_event_release){
                // APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"P: 0x%08X, I: %d", 2, krcb.func_cb[idx], idx);

                /* When the play mode of the macro in the playing state is repeat until any key press,
                 * if it responds to other keys(especially the left key), it may cause the
                 * character processing software to drop characters, but the lost keyboard report can
                 * be seen from the USB log.
                 * In order to avoid confusion and ***refer to the behavior of other mouse***, only let it
                 * be used as a stop sign, and not output the mouse key event to the PC.
                 */
                if(krcb.p_current_macro){
                    macro_play_param_t *p_macro_play_param = krcb.p_current_macro->p_param;
                    if(p_macro_play_param->play_mode == repeat_until_any_key_press){
                        krcb.macro_async_stop_req = 1;
                        /* clear output signal */
                        krcb.mouse_key_o &= ~(key_mask);
                        continue;
                    }
                }
                krcb.func_cb[idx](idx);
                s_app_key_remap_events[idx] = (uint8_t)key_event_press;
            }
            func_type = s_key_remap_profile[idx][0];
            /* handle the event that the button is pressed and needs to be level-triggered */
            if(func_type != (idx + 1)){
                /* clear output signal */
                krcb.mouse_key_o &= ~(s_mouse_button_mask[idx]);
                if((func_type >= (uint8_t)remap_to_left_key) && \
                   (func_type <= (uint8_t)remap_to_fifth_key))
                {
                    /* add key remap to output signal */
                    krcb.mouse_key_o |= s_mouse_button_mask[func_type-1];
                    // APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"L: 0x%08X, I: %d", 2, krcb.mouse_key_o, idx);
                }
            }
        } else {
            if(s_app_key_remap_events[idx] == (uint8_t)key_event_press){
                s_app_key_remap_events[idx] = (uint8_t)key_event_release;
                // APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"R: 0x%08X, I: %d", 2, release_key_event_cb, idx);
                release_key_event_cb(idx);
            }
        }
    }

    // APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"I:%02X,PCM:%08X,MR:%d,MPR:%d", 4,
    //     *p_button_mask,
    //     krcb.p_current_macro,
    //     krcb.macro_play_req,
    //     krcb.macro_preempt_play_req
    // );

    if(krcb.p_current_macro){
        /* If there is a demand for macro playback, program need to
         * ignore the sending requirements of other key board reports,
         * and the priority of macro play is higher. */
        krcb.send_std_kbd_req = NO_KBD_REPORT_DEFINITION;
        krcb.send_csm_kbd_req = NO_KBD_REPORT_DEFINITION;
        krcb.send_std_empty_kbd_req = 0;
        krcb.send_csm_empty_kbd_req = 0;
        krcb.firekey_req = NO_FIRE_KEY_REQUEST;
        krcb.firekey_stop_req = 0;
        krcb.firekey_sig = 0;

        p_kbd = &krcb.kbds[krcb.macro_play_req];
        send_kbd_res = app_mouse_attach_keyboard_report((T_KEY_REPORT_S*)p_kbd);

        /* When the key is mapped to a macro and the left/right/middle key needs to be output,
         * the output value needs to be re-modified, otherwise the behavior may be abnormal.
         */
        if(krcb.p_current_macro->left_key_sig){
            krcb.mouse_key_o |= s_mouse_button_mask[0];
        }
        if(krcb.p_current_macro->right_key_sig){
            krcb.mouse_key_o |= s_mouse_button_mask[1];
        }
        if(krcb.p_current_macro->middle_key_sig){
            krcb.mouse_key_o |= s_mouse_button_mask[2];
        }
    } else {
        if(krcb.send_std_kbd_req != NO_KBD_REPORT_DEFINITION){
            p_kbd = &krcb.kbds[krcb.send_std_kbd_req];
            send_kbd_res = app_mouse_attach_keyboard_report((T_KEY_REPORT_S*)p_kbd);
        } else if(krcb.send_csm_kbd_req != NO_KBD_REPORT_DEFINITION){
            p_kbd = &krcb.kbds[krcb.send_csm_kbd_req];
            send_kbd_res = app_mouse_attach_consumer_report((T_KEY_REPORT_CONSUMER_S*)p_kbd);
        } else if(krcb.send_std_empty_kbd_req){
            p_kbd = (key_report_u *)(&(krcb.empty_std_kbd));
            send_kbd_res = app_mouse_attach_keyboard_report((T_KEY_REPORT_S*)p_kbd);
        } else if(krcb.send_csm_empty_kbd_req){
            p_kbd = (key_report_u *)(&(krcb.empty_csm_kbd));
            send_kbd_res = app_mouse_attach_consumer_report((T_KEY_REPORT_CONSUMER_S*)p_kbd);
        } else {
            ;
        }

        // if(p_kbd){
        //     APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"K: %d, A: 0x%08X", 2, send_kbd_res, p_kbd);
        //     app_key_remap_print_array((void*)p_kbd, sizeof(key_report_u));
        //     APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"data: %d", 1, krcb.send_std_kbd_req);
        //     APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"data: %d", 1, krcb.send_csm_kbd_req);
        //     APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"data: %d", 1, krcb.send_std_empty_kbd_req);
        //     APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"data: %d", 1, krcb.send_csm_empty_kbd_req);
        // }
        // app_key_remap_print_array((void*)s_app_key_remap_events, sizeof(s_app_key_remap_events));

        krcb.send_std_kbd_req = NO_KBD_REPORT_DEFINITION;
        krcb.send_csm_kbd_req = NO_KBD_REPORT_DEFINITION;
        krcb.send_std_empty_kbd_req = 0;
        krcb.send_csm_empty_kbd_req = 0;

        if((send_kbd_res != 0) && (p_kbd != NULL)){
            APPS_LOG_MSGID_E(KEY_REMAP_LOG_TAG"lost one kbd report", 0);
            app_key_remap_print_array((void*)p_kbd, sizeof(key_report_u));
        }
    }

    SET_KEY_OUTPUT(krcb.mouse_key_o);
    *p_button_mask = krcb.mouse_key_o;
    // APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"O:%02X", 1, krcb.mouse_key_o);

    SET_GPIO_LOW(14);
}


ATTR_TEXT_IN_TCM
void app_dongle_key_remap_routine(TimerHandle_t xTimer)
{
    SET_GPIO_HIGH(5);

    // Handle remap process here if there is a key triggered
    static uint32_t routine_cnt = 0, routine_cnt_firekey_start = 0, firekey_period_cnt = 0;
    static uint8_t firekey_round = 0;
    static uint32_t routine_mouse_key_o = 0;
    static uint32_t current_time_in_us = 0;
    static uint8_t macro_end_double_confirm = 0;
    static uint8_t nothing_todo_cnt = 0;
    static uint8_t retry_curr_kbd = 0;

    uint8_t firekey_repeat_times;
    uint8_t button_mask = 0;
    uint32_t diff_cnt, idx, irq_mask;
    T_KEY_REPORT_S *p_kbd = &(krcb.empty_std_kbd);
    uint8_t macro_refresh_req = 1;
    uint8_t send_res;

    if(krcb.disconnection_req == 1){
        if(krcb.p_current_macro){
            /* stop playing macro right now */
            krcb.macro_stop_req = 1;
        } else if(krcb.firekey_req != NO_FIRE_KEY_REQUEST){
            krcb.firekey_req = NO_FIRE_KEY_REQUEST;
            krcb.firekey_sig = 0;
            krcb.firekey_repeat_times = 0;
            SET_KEY_OUTPUT(0);
        } else {
            /* only macro/firekey need to be stoped */
            ;
        }
        routine_cnt_firekey_start = 0;
        firekey_round = 0;
        routine_cnt = 0;
        nothing_todo_cnt = 0;
        krcb.disconnection_req = 0;
    }

    ++routine_cnt;
    if(krcb.p_current_macro){
        if(NO_MACRO_PLAY_REQUEST != krcb.macro_preempt_play_req){
            hal_nvic_save_and_set_interrupt_mask(&irq_mask);
            /* clear data of previous macro and prepare its first package data */
            app_key_remap_stop_play_macro();

            krcb.macro_play_req = krcb.macro_preempt_play_req;
            krcb.macro_preempt_play_req = NO_MACRO_PLAY_REQUEST;
            krcb.p_current_macro = &(krcb.macro_descriptors[krcb.macro_play_req]);
            krcb.p_current_macro->play_round = 0;
            krcb.p_current_macro->rcds_play_idx = 0;
            krcb.p_current_macro->key_code_used_cnt = 0;
            krcb.p_current_macro->current_time_in_us = 0;
            current_time_in_us = 0;
            macro_refresh_req = 0;
            hal_nvic_restore_interrupt_mask(irq_mask);
            refresh_data_about_macro(
                0,
                krcb.p_current_macro->key_idx,
                krcb.p_current_macro->p_param
            );
            retry_curr_kbd = 0;
        } else {
            if(retry_curr_kbd > 0){
                --retry_curr_kbd;
                p_kbd = &(krcb.kbds[krcb.p_current_macro->key_idx].std);
                send_res = app_mouse_attach_keyboard_report(p_kbd);
                // APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"CT: %d us, try %d %d", 3, current_time_in_us, retry_curr_kbd, send_res);
                if(send_res == STATUS_OK){
                    krcb.p_current_macro->update_kbd_report_req = 0;
                    retry_curr_kbd = 0;
                }
                SET_GPIO_LOW(5);
                return;
            }
            if(krcb.macro_stop_req == 1){
                current_time_in_us = 0;
                idx = krcb.p_current_macro->key_idx;
                app_key_remap_init_key_report(standard_kbd_report, &(krcb.kbds[idx]));
                SET_KEY_OUTPUT(0x0);
                uint8_t send_res = app_mouse_attach_keyboard_report(p_kbd);
                if(send_res == STATUS_OK){
                    // APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"STOP: ", 0);
                    // app_key_remap_print_array((void*)p_kbd, sizeof(T_KEY_REPORT_S));
                    krcb.p_current_macro->update_kbd_report_req = 0;
                    macro_end_double_confirm++;
                    if(macro_end_double_confirm == 2){
                        krcb.macro_stop_req = 0;
                        macro_end_double_confirm = 0;
                        refresh_data_about_macro(
                            0,
                            krcb.p_current_macro->key_idx,
                            krcb.p_current_macro->p_param
                        );
                        app_key_remap_stop_play_macro();
                    }
                }
                SET_GPIO_LOW(5);
                return;
            }
        }

        for(idx = 0; idx < MACRO_TIME_CNT_NUMBER; idx++){
            if(krcb.p_current_macro->time_cnt[idx]){
                // APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"TC[%d]: %d", 2, idx, krcb.p_current_macro->time_cnt[idx]);
                --(krcb.p_current_macro->time_cnt[idx]);
                if(0 == krcb.p_current_macro->time_cnt[idx]){
                    switch(idx){
                        case 0:
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                            /* fall through, generic processing flow. */
                            krcb.kbds[krcb.p_current_macro->key_idx].std.keycode[idx] = 0;
                            krcb.macro_descriptors[krcb.p_current_macro->key_idx].update_kbd_report_req = 1;
                            /* This variable needs to be subtracted only if it is recognized that
                             * it is data in the keyboard report, otherwise it may be subtracted several times,
                             * resulting in unexpected logging problems. */
                            krcb.macro_descriptors[krcb.p_current_macro->key_idx].key_code_used_cnt--;
                            break;
                        case LEFT_KEY_SIG_IDX:
                            krcb.p_current_macro->left_key_sig = 0;
                            break;
                        case RIGHT_KEY_SIG_IDX:
                            krcb.p_current_macro->right_key_sig = 0;
                            break;
                        case MIDDLE_KEY_SIG_IDX:
                            krcb.p_current_macro->middle_key_sig = 0;
                            break;
                        case WIN_SIG_IDX:
                            krcb.p_current_macro->win_sig = 0;
                            krcb.kbds[krcb.p_current_macro->key_idx].std.combo &= ~WIN_VALUE_IN_COMBO;
                            krcb.macro_descriptors[krcb.p_current_macro->key_idx].update_kbd_report_req = 1;
                            break;
                        case ALT_SIG_IDX:
                            krcb.p_current_macro->alt_sig = 0;
                            krcb.kbds[krcb.p_current_macro->key_idx].std.combo &= ~ALT_VALUE_IN_COMBO;
                            krcb.macro_descriptors[krcb.p_current_macro->key_idx].update_kbd_report_req = 1;
                            break;
                        case SHIFT_SIG_IDX:
                            krcb.p_current_macro->shift_sig = 0;
                            krcb.kbds[krcb.p_current_macro->key_idx].std.combo &= ~SHIFT_VALUE_IN_COMBO;
                            krcb.macro_descriptors[krcb.p_current_macro->key_idx].update_kbd_report_req = 1;
                            break;
                        case CTRL_SIG_IDX:
                            krcb.p_current_macro->ctrl_sig = 0;
                            krcb.kbds[krcb.p_current_macro->key_idx].std.combo &= ~CTRL_VALUE_IN_COMBO;
                            krcb.macro_descriptors[krcb.p_current_macro->key_idx].update_kbd_report_req = 1;
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        if(macro_refresh_req){
            refresh_data_about_macro(
                current_time_in_us,
                krcb.p_current_macro->key_idx,
                krcb.p_current_macro->p_param
            );
        }
        if(krcb.p_current_macro->current_time_in_us == 0){
            current_time_in_us = 0;
        }

        if(krcb.p_current_macro->left_key_sig){
            button_mask |= s_mouse_button_mask[0];
        }
        if(krcb.p_current_macro->right_key_sig){
            button_mask |= s_mouse_button_mask[1];
        }
        if(krcb.p_current_macro->middle_key_sig){
            button_mask |= s_mouse_button_mask[2];
        }

        if(krcb.p_current_macro->update_kbd_report_req == 1){
            p_kbd = &(krcb.kbds[krcb.p_current_macro->key_idx].std);

            send_res = app_mouse_attach_keyboard_report(p_kbd);
            if(send_res == STATUS_OK){
                krcb.p_current_macro->update_kbd_report_req = 0;
                // APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"CT: %d us, MPF", 1, current_time_in_us);
                // app_key_remap_print_array((void*)p_kbd, sizeof(T_KEY_REPORT_S));
            } else {
                retry_curr_kbd = 255;
                // APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"Fail at %d us, retry", 1, current_time_in_us);
            }
        }
        if(routine_mouse_key_o != button_mask){
            SET_KEY_OUTPUT(button_mask);
        }
        routine_mouse_key_o = button_mask;
        current_time_in_us += krcb.routine_interval_in_us;
    } else if (krcb.firekey_req != NO_FIRE_KEY_REQUEST) {
        if(routine_cnt_firekey_start == 0){
            routine_cnt_firekey_start = routine_cnt;
            firekey_period_cnt = (*(krcb.p_firekey_param + 2)) * 1000 / krcb.routine_interval_in_us;
        }
        diff_cnt = routine_cnt - routine_cnt_firekey_start;
        if((diff_cnt != 0) && ((diff_cnt % firekey_period_cnt) == 0)){
            ++firekey_round;
            /* Because the first round of packet data is sent from the status_update, special handling is required here. */
            krcb.firekey_sig = firekey_round % 2;
            firekey_repeat_times = 2*(*(krcb.p_firekey_param + 1));     /* press & release */
            krcb.firekey_repeat_times = firekey_repeat_times;
            if((firekey_repeat_times == 0) || (firekey_round < firekey_repeat_times)){

                /* Continue to output fire signal( left key ) until the key is released */
                if(((krcb.firekey_phy_key & krcb.mouse_key_i) == 0) && (firekey_repeat_times == 0)){
                    krcb.firekey_req = NO_FIRE_KEY_REQUEST;
                    krcb.firekey_sig = 0;
                    krcb.firekey_repeat_times = 0;
                }
            } else {
                krcb.firekey_req = NO_FIRE_KEY_REQUEST;
                krcb.firekey_sig = 0;
                krcb.firekey_repeat_times = 0;
            }
            // APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"D: 0x%08X, P:%d, R:%d, S: %d", 4, diff_cnt, firekey_period_cnt, firekey_round, krcb.firekey_sig);
        }
        if(krcb.firekey_sig == 1){
            button_mask |= s_mouse_button_mask[0];
        } else {
            ;
        }
        if(routine_mouse_key_o != button_mask){
            SET_KEY_OUTPUT(button_mask);
        }
        routine_mouse_key_o = button_mask;
    } else {
        routine_cnt_firekey_start = 0;
        firekey_round = 0;
        routine_cnt = 0;
        ++nothing_todo_cnt;
        if(nothing_todo_cnt >= 10){
            nothing_todo_cnt = 0;
        }
    }

    SET_GPIO_LOW(5);
}


ATTR_TEXT_IN_TCM
static void release_key_event_cb(uint8_t key_idx)
{
    uint8_t *p_key_func_descrip = key_remap_get_profile_description(key_idx);
    key_remap_type_e func = (key_remap_type_e)(*p_key_func_descrip);

    uint8_t repeat_times;
    macro_play_mode_e mode;
    macro_play_param_t *p_macro_play_param;
    uint32_t irq_mask;

    switch(func){
        case remap_disable:
        case remap_to_dpi_control:
        case remap_to_report_rate_switch:
            /* fall through, generic processing flow. */
            krcb.mouse_key_o &= ~(s_mouse_button_mask[key_idx]);
            break;
        case remap_to_left_key:
        case remap_to_right_key:
        case remap_to_middle_key:
        case remap_to_fourth_key:
        case remap_to_fifth_key:
            /* fall through, generic processing flow. */
            // APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"RKEC: func - 1 = %d", 1, func - 1);
            krcb.mouse_key_o &= ~(s_mouse_button_mask[func - 1]);
            break;
        case remap_to_kbd_key:
            if(krcb.kbds[key_idx].std.id == REPORT_ID_KEYBOARD){
                krcb.send_std_empty_kbd_req = 1;
            } else {
                krcb.send_csm_empty_kbd_req = 1;
            }
            krcb.mouse_key_o &= ~(s_mouse_button_mask[key_idx]);
            break;
        case remap_to_macro:
            krcb.mouse_key_o &= ~(s_mouse_button_mask[key_idx]);
            hal_nvic_save_and_set_interrupt_mask(&irq_mask);
            if((krcb.p_current_macro) && (krcb.p_current_macro == &(krcb.macro_descriptors[key_idx]))){
                p_macro_play_param = krcb.p_current_macro->p_param;
                repeat_times = p_macro_play_param->repeat_times;
                mode = p_macro_play_param->play_mode;
                if((repeat_times == 0) && (repeat_until_the_key_release == mode)){
                    krcb.macro_async_stop_req = 1;
                }
            }
            hal_nvic_restore_interrupt_mask(irq_mask);
            break;
        case remap_to_fire_key:
            krcb.mouse_key_o &= ~(s_mouse_button_mask[key_idx]);
            /* press & release */
            repeat_times = 2*(*(p_key_func_descrip + 1));
            if(repeat_times == 0){
                krcb.firekey_stop_req = 1;
            } else {
                /* When the firekey is output 1, the button is not allowed to be cleared,
                 * otherwise the user will feel like pressing it one more time. */
                if(krcb.firekey_sig == 1){
                    /* left key signal <=> fire key signal */
                    krcb.mouse_key_o |= s_mouse_button_mask[0];
                }
            }
            break;
        default:
            break;
    }
}

ATTR_TEXT_IN_TCM
static uint32_t remap_disable_key_press_event_cb(uint8_t key_idx)
{
    /* clear output signal */
    krcb.mouse_key_o &= ~(s_mouse_button_mask[key_idx]);
    return 0;
}

ATTR_TEXT_IN_TCM
static uint32_t remap_mouse_key_press_event_cb(uint8_t key_idx)
{
    uint8_t remap_key_idx = *(krcb.p_func_descrip[key_idx]) - 1;
    remap_disable_key_press_event_cb(key_idx);
    // APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"RKI: %d, key out: 0x%02X", 2, remap_key_idx, s_mouse_button_mask[remap_key_idx]);
    krcb.mouse_key_o |= s_mouse_button_mask[remap_key_idx];
    return 0;
}

ATTR_TEXT_IN_TCM
static uint32_t remap_kbd_key_press_event_cb(uint8_t key_idx)
{
    /* clear output signal */
    krcb.mouse_key_o &= ~(s_mouse_button_mask[key_idx]);

    if(!krcb.p_current_macro){
        switch(krcb.kbds[key_idx].std.id){
            case REPORT_ID_KEYBOARD:
                if(krcb.send_std_kbd_req == NO_KBD_REPORT_DEFINITION){
                    krcb.send_std_kbd_req = key_idx;
                }
                break;
            case REPORT_ID_KEYBOARD_CONSUMER:
                if(krcb.send_csm_kbd_req == NO_KBD_REPORT_DEFINITION){
                    krcb.send_csm_kbd_req = key_idx;
                }
                break;
            default:
                break;
        }
    }
    return 0;
}

ATTR_TEXT_IN_TCM
static uint32_t remap_macro_key_press_event_cb(uint8_t key_idx)
{
    /* clear output signal */
    krcb.mouse_key_o &= ~(s_mouse_button_mask[key_idx]);

    if(krcb.macro_play_req == NO_MACRO_PLAY_REQUEST){
        krcb.p_current_macro = &(krcb.macro_descriptors[key_idx]);
        krcb.macro_play_req = key_idx;
        if(krcb.macro_descriptors[key_idx].left_key_sig){
            krcb.mouse_key_o |= s_mouse_button_mask[0];
        }
        if(krcb.macro_descriptors[key_idx].right_key_sig){
            krcb.mouse_key_o |= s_mouse_button_mask[1];
        }
        if(krcb.macro_descriptors[key_idx].middle_key_sig){
            krcb.mouse_key_o |= s_mouse_button_mask[2];
        }
    } else {
        /* one macro is playing, and another macro key is pressed. */
        krcb.macro_preempt_play_req = key_idx;
    }
    return 0;
}

ATTR_TEXT_IN_TCM
static uint32_t remap_fire_key_press_event_cb(uint8_t key_idx)
{
    /* clear output signal */
    krcb.mouse_key_o &= ~(s_mouse_button_mask[key_idx]);

    if((krcb.firekey_req == NO_FIRE_KEY_REQUEST) && (!krcb.p_current_macro)){
        /* Prepare the first package of data and flag the need for more processing for the firekey feature. */
        /* The function of fireKey is to simulate the cycle of pressing and releasing the left mouse button. */
        krcb.firekey_req = key_idx;
        krcb.firekey_sig = 1;
        krcb.mouse_key_o |= s_mouse_button_mask[0];
        krcb.firekey_phy_key = s_mouse_button_mask[key_idx];
        krcb.p_firekey_param = krcb.p_func_descrip[key_idx];
    }
    return 0;
}


static void async_store_result(nvkey_status_t status, void *user_data)
{
    uint32_t id = (uint32_t)user_data;
    APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"store 0x%X to nvkey return %d", 2, id, status);
}

void app_key_remap_set_pattern(uint8_t key_idx, uint8_t *pdata, uint32_t len)
{
    nvkey_status_t op_res;

    assert(len <= KEYMAPPING_PATTERN_SIZE);
    memset(&s_key_remap_profile[key_idx][0], 0, KEYMAPPING_PATTERN_SIZE);
    memcpy(&s_key_remap_profile[key_idx][0], pdata, len);
    s_key_remap_profile_valid_len[key_idx] = len;
    app_key_remap_prepare_data_by_key_func(
        key_idx,
        (key_remap_type_e)(s_key_remap_profile[key_idx][0]),
        false
    );

    op_res = nvkey_write_data_non_blocking(
        NVID_APP_DONGLE_KEY_REMAP_1 + key_idx,
        (const uint8_t *)pdata,
        len,
        async_store_result,
        (const void *)(NVID_APP_DONGLE_KEY_REMAP_1 + key_idx)
    );
    assert(op_res == NVKEY_STATUS_OK);
}

ATTR_TEXT_IN_TCM
uint8_t app_mouse_attach_keyboard_report(T_KEY_REPORT_S *p_kbd)
{
    uint8_t port = usb_hid_find_port_by_report(USB_REPORT_DSCR_TYPE_GKEYBOARD);
    if(USB_HID_MAX_DEVICE_NUM != port){
        // APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"keymap data to USB", 0);
        return usb_hid_tx_non_blocking(port, (uint8_t*)p_kbd, 7);
    }
    return 0xFF;
}

ATTR_TEXT_IN_TCM
uint8_t app_mouse_attach_consumer_report(T_KEY_REPORT_CONSUMER_S *p_csm)
{
    return app_mouse_attach_keyboard_report((T_KEY_REPORT_S*)p_csm);
}

static void app_key_remap_load_data(void)
{
    uint16_t id;
    uint32_t size, idx = 0;
    nvkey_status_t op_res;
    for(id = NVID_APP_DONGLE_KEY_REMAP_1; id <= NVID_APP_DONGLE_KEY_REMAP_5; id++){
        size = KEYMAPPING_PATTERN_SIZE;
        op_res = nvkey_read_data(id, &(s_key_remap_profile[idx][0]), &size);
        APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"read 0x%X from nvkey return %d with size %d", 3, id, op_res, size);
        LOG_HEXDUMP_I(apps, KEY_REMAP_LOG_TAG"pattern", &(s_key_remap_profile[idx][0]), size);
        s_key_remap_profile_valid_len[idx] = size;
        assert((op_res == 0) && (size != 0));
        idx++;
    }
}

static TimerHandle_t s_key_remap_timer = NULL;
static bool s_key_remap_timer_is_started = false;

void app_key_remap_timer_start(void)
{
    uint32_t irq_mask;
    bool do_operation = false;

    hal_nvic_save_and_set_interrupt_mask(&irq_mask);
    if ((!s_key_remap_timer_is_started) && (s_key_remap_timer != NULL)) {
        s_key_remap_timer_is_started = true;
        do_operation = true;
    }
    hal_nvic_restore_interrupt_mask(irq_mask);

    if (do_operation) {
        xTimerStart(s_key_remap_timer, 0);
        APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"start key remap timer", 0);
    }
}

void app_key_remap_timer_stop(void)
{
    uint32_t irq_mask;
    bool do_operation = false;

    hal_nvic_save_and_set_interrupt_mask(&irq_mask);
    if ((s_key_remap_timer_is_started) && (s_key_remap_timer != NULL)) {
        s_key_remap_timer_is_started = false;
        do_operation = true;
    }
    hal_nvic_restore_interrupt_mask(irq_mask);

    if (do_operation) {
        xTimerStop(s_key_remap_timer, 0);
        APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"stop key remap timer", 0);
    }
}

bool app_key_remap_init()
{
    s_key_remap_timer = xTimerCreate("key remap", 0x1, pdTRUE, NULL, app_dongle_key_remap_routine);
    APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"create key remap timer return 0x%08X", 1, s_key_remap_timer);
    if (s_key_remap_timer != NULL) {
        krcb.routine_interval_in_us = 1000;
        app_key_remap_load_data();
        app_key_remap_prepare_data();
        app_key_remap_timer_start();
        return true;
    }
    return false;
}

void app_key_remap_disconnection_request(void)
{
    APPS_LOG_MSGID_I(KEY_REMAP_LOG_TAG"disconnection request", 0);
    memset(&s_app_key_remap_events[0], (uint8_t)key_event_release, APP_KEY_NUMBER);
    krcb.disconnection_req = 1;
}
#endif /* defined(AIR_PURE_GAMING_MS_ENABLE) */
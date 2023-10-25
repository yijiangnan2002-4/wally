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

#ifndef __ANC_API_H__
#define __ANC_API_H__

#ifdef MTK_ANC_ENABLE
#include "types.h"
#include "hal_ccni.h"
#include "hal_platform.h"

/**
 * @addtogroup Audio
 * @{
 * @addtogroup anc_control
 * @{
 *
 * The ANC is used for active noise cancellation.
 * The Pass-through is used for ambient sound.
 *
 * @section anc_control_api_usage How to use this module
 *
 * - An example on how to use the ANC APIs.
 *  - 1. Get biquad gain.
 *  - 2. Set biquad gain.
 *  - 3. Set ramp capability.
 *    - Sample code:
 *     @code
 *         void dsp_anc_get_biqaud_gain_demo()
 *        {
 *             int32_t bq_gain_FF_1 = dsp_anc_control_get_biquad_gain(AUDIO_ANC_CONTROL_FILTER_FF_1, NULL);
 *             int32_t bq_gain_FF_2 = dsp_anc_control_get_biquad_gain(AUDIO_ANC_CONTROL_FILTER_FF_2, NULL);
 *             int32_t bq_gain_FB_1 = dsp_anc_control_get_biquad_gain(AUDIO_ANC_CONTROL_FILTER_FB_1, NULL);
 *             int32_t bq_gain_FB_2 = dsp_anc_control_get_biquad_gain(AUDIO_ANC_CONTROL_FILTER_FB_2, NULL);
 *         }
 *        void dsp_anc_set_biqaud_gain_demo()
 *        {
 *             int32_t bq_gain_FF_1 = 0x12345678;
 *             int32_t bq_gain_FF_2 = 0x56781234;
 *             int32_t bq_gain_FB_1 = 0x91199119;
 *             int32_t bq_gain_FB_2 = 0x98765432;
 *             int32_t bq_gain_all  = 0x13571357;
 *             // Set by specific filter.
 *             dsp_anc_control_set_biquad_gain(AUDIO_ANC_CONTROL_FILTER_MASK_FF_1, bq_gain_FF_1, NULL);
 *             dsp_anc_control_set_biquad_gain(AUDIO_ANC_CONTROL_FILTER_MASK_FF_2, bq_gain_FF_2, NULL);
 *             dsp_anc_control_set_biquad_gain(AUDIO_ANC_CONTROL_FILTER_MASK_FB_1, bq_gain_FB_1, NULL);
 *             dsp_anc_control_set_biquad_gain(AUDIO_ANC_CONTROL_FILTER_MASK_FB_2, bq_gain_FB_2, NULL);
 *             // Set multi-pul filters at same time.
 *             dsp_anc_control_set_biquad_gain(AUDIO_ANC_CONTROL_FILTER_MASK_FF_1 |
 *                                             AUDIO_ANC_CONTROL_FILTER_MASK_FF_2 |
 *                                             AUDIO_ANC_CONTROL_FILTER_MASK_FB_1 |
 *                                             AUDIO_ANC_CONTROL_FILTER_MASK_FB_2, bq_gain_all, NULL);
 *         }
 *        void dsp_anc_set_ramp_gain_demo()
 *        {
 *             dsp_anc_control_result_t ret;
 *             int32_t dB_100_FF_1 = -100; //dB*100
 *             int32_t dB_100_FF_2 = -200; //dB*100
 *             int32_t dB_100_FB_1 = -300; //dB*100
 *             int32_t dB_100_FB_2 = -400; //dB*100
 *             int32_t dB_100_all  = -500; //dB*100
 *             // Set by specific filter.
 *             ret = dsp_anc_control_set_ramp_capability(AUDIO_ANC_CONTROL_FILTER_MASK_FF_1, dB_100_FF_1, DSP_ANC_CONTROL_UNASSIGNED_PARAM, DSP_ANC_CONTROL_UNASSIGNED_PARAM, DSP_ANC_CONTROL_UNASSIGNED_PARAM, NULL);
 *             ret = dsp_anc_control_set_ramp_capability(AUDIO_ANC_CONTROL_FILTER_MASK_FF_2, dB_100_FF_2, DSP_ANC_CONTROL_UNASSIGNED_PARAM, DSP_ANC_CONTROL_UNASSIGNED_PARAM, DSP_ANC_CONTROL_UNASSIGNED_PARAM, NULL);
 *             ret = dsp_anc_control_set_ramp_capability(AUDIO_ANC_CONTROL_FILTER_MASK_FB_1, dB_100_FB_1, DSP_ANC_CONTROL_UNASSIGNED_PARAM, DSP_ANC_CONTROL_UNASSIGNED_PARAM, DSP_ANC_CONTROL_UNASSIGNED_PARAM, NULL);
 *             ret = dsp_anc_control_set_ramp_capability(AUDIO_ANC_CONTROL_FILTER_MASK_FB_2, dB_100_FB_2, DSP_ANC_CONTROL_UNASSIGNED_PARAM, DSP_ANC_CONTROL_UNASSIGNED_PARAM, DSP_ANC_CONTROL_UNASSIGNED_PARAM, NULL);
 *             // Set multi-pul filters at same time.
 *             ret = dsp_anc_control_set_ramp_capability(AUDIO_ANC_CONTROL_FILTER_MASK_FF_1 |
 *                                                       AUDIO_ANC_CONTROL_FILTER_MASK_FF_2 |
 *                                                       AUDIO_ANC_CONTROL_FILTER_MASK_FB_1 |
 *                                                       AUDIO_ANC_CONTROL_FILTER_MASK_FB_2, dB_100_all, DSP_ANC_CONTROL_UNASSIGNED_PARAM, DSP_ANC_CONTROL_UNASSIGNED_PARAM, DSP_ANC_CONTROL_UNASSIGNED_PARAM, NULL);
 *         }
 *     @endcode
 */

#define ANC_FULL_ADAPT_STATE_RESET       (0)
#define ANC_FULL_ADAPT_STATE_PROCESS     (1)
#define ANC_FULL_ADAPT_STATE_FREEZE      (2) //for debug
#define ANC_FULL_ADAPT_STATE_SLEEP       (3)
#define ANC_FULL_ADAPT_STATE_SLEEP_DEBUG (4)
typedef uint8_t anc_full_adapt_state_t;

#define ANC_FULL_ADAPT_CONTROL_STOP      (0)
#define ANC_FULL_ADAPT_CONTROL_START     (1)
#define ANC_FULL_ADAPT_CONTROL_BYPASS    (2)
#define ANC_FULL_ADAPT_CONTROL_DEBUG     (3)
#define ANC_FULL_ADAPT_CONTROL_SET_DMA   (4)
#define ANC_FULL_ADAPT_CONTROL_SET_DT    (5)
#define ANC_FULL_ADAPT_CONTROL_AWAKE     (6)
#define ANC_FULL_ADAPT_CONTROL_SLEEP     (7)

typedef uint8_t anc_full_adapt_control_state_t;

#define ANC_CONTROL_RELAY_EVENT_HOWLING_ATTACK       (1)
#define ANC_CONTROL_RELAY_EVENT_HOWLING_RELEASE      (2)

typedef uint8_t anc_control_relay_event_t;

/** @brief Unassigning ANC gain. */
#define DSP_ANC_CONTROL_UNASSIGNED_GAIN  (0x7FFF)

/** @brief Unassigning ANC param. */
#define DSP_ANC_CONTROL_UNASSIGNED_PARAM (0xF)

/*anc_control.c*/
void dsp_anc_init(void);
void dsp_anc_stop(hal_ccni_message_t msg, hal_ccni_message_t *ack);
void dsp_anc_start(hal_ccni_message_t msg, hal_ccni_message_t *ack);
void dsp_set_anc_param(hal_ccni_message_t msg, hal_ccni_message_t *ack);
void dsp_set_anc_volume(hal_ccni_message_t msg, hal_ccni_message_t *ack);
void afe_anc_pwrdet_interrupt_handler(uint32_t irq_index);

/*hal_audio_anc.c*/
bool hal_audio_device_set_anc(hal_audio_device_parameter_t *handle, hal_audio_control_t device, hal_audio_control_status_t control);
void hal_audio_anc_set_reg(hal_audio_set_value_parameter_t *anc_set_param);
uint32_t hal_audio_anc_get_reg(hal_audio_get_value_parameter_t *anc_get_param);
void hal_audio_anc_init(void);
void hal_audio_anc_set_change_dl_rate(uint32_t rate);
bool hal_audio_anc_get_change_dl_rate(uint32_t *rate);
uint32_t hal_audio_anc_get_using_count(hal_audio_device_parameter_t *handle);

/** @brief ANC and Pass-through control return values. */
typedef enum {
    DSP_ANC_CONTROL_EXECUTION_NONE        = -2,  /**< The anc process result was in initail status.   */
    DSP_ANC_CONTROL_EXECUTION_FAIL        = -1,  /**< The anc process result was fail.   */
    DSP_ANC_CONTROL_EXECUTION_SUCCESS     =  0,  /**< The anc process result was successful.   */
    DSP_ANC_CONTROL_EXECUTION_CANCELLED   =  1,  /**< The anc process result was cancelled.   */
    DSP_ANC_CONTROL_EXECUTION_NOT_ALLOWED =  2,  /**< The anc process result was not allowed.   */
} dsp_anc_control_result_t;

/** @brief ANC and Pass-through type values. */
typedef enum {
    DSP_ANC_CONTROL_TYPE_HYBRID       = 0,  /**< The hybrid anc type.   */
    DSP_ANC_CONTROL_TYPE_FF           = 1,  /**< The feedforward anc type.   */
    DSP_ANC_CONTROL_TYPE_FB           = 2,  /**< The feedback anc type.   */
    DSP_ANC_CONTROL_TYPE_USER_DEFINED = 3,  /**< The user defined anc type.   */
    DSP_ANC_CONTROL_TYPE_PASSTHRU_FF  = 4,  /**< The pass-through feedforward anc type.   */
} dsp_anc_control_type_t;

typedef enum {
    DSP_ANC_CALLBACK_EVENT_RAMP,
    DSP_ANC_CALLBACK_EVENT_RESERVE_1,
} dsp_anc_callback_event_t;

typedef struct {
    uint8_t          ramp_status;
    uint8_t          biquad_status;
    uint32_t         ramp_gain[4];
    uint32_t         *p_reserve[4];
} dsp_anc_callback_param_t;

/** @brief The support misc settings capability structure of the ANC path. */
typedef struct dsp_anc_control_misc_s {
    union {
        struct {
                uint8_t  relay_event;  /**< Calibrate gain to L-ch feedforward anc.   */
        };
        uint32_t extend_use_parameters;                    /**< Extend use.   */
    };
} dsp_anc_control_misc_t;

typedef enum {
    DSP_ANC_CONTROL_AUDIO_SCENARIO_TYPE_IDLE = 0,
    DSP_ANC_CONTROL_AUDIO_SCENARIO_TYPE_CALL,
    DSP_ANC_CONTROL_AUDIO_SCENARIO_TYPE_A2DP,
    DSP_ANC_CONTROL_AUDIO_SCENARIO_TYPE_NUM,
} dsp_anc_control_audio_scenario_type_t;

typedef enum {
    DSP_ANC_CONTROL_AUDIO_SCENARIO_ANC_PASSTHRU_TYPE_ANC = 0,
    DSP_ANC_CONTROL_AUDIO_SCENARIO_ANC_PASSTHRU_TYPE_PASSTHRU = 1,
    DSP_ANC_CONTROL_AUDIO_SCENARIO_ANC_PASSTHRU_TYPE_NUM,
} dsp_anc_control_audio_scenario_anc_passthru_type_t;

/** @brief anc control filter choice dimensions. */
typedef enum {
    AUDIO_ANC_CONTROL_FILTER_FF_1 = 0,       /**< L channel of feedforward anc. */
    AUDIO_ANC_CONTROL_FILTER_FF_2,           /**< R channel of feedforward anc. */
    AUDIO_ANC_CONTROL_FILTER_FB_1,           /**< L channel of feedback anc. */
    AUDIO_ANC_CONTROL_FILTER_FB_2,           /**< R channel of feedback anc. */
    AUDIO_ANC_CONTROL_FILTER_OPTION_MAX,
} dsp_anc_control_filter_t;

/** @brief anc control filter mask choice dimensions. */
typedef enum {
    AUDIO_ANC_CONTROL_FILTER_MASK_FF_1        = (1 << AUDIO_ANC_CONTROL_FILTER_FF_1),
    AUDIO_ANC_CONTROL_FILTER_MASK_FF_2        = (1 << AUDIO_ANC_CONTROL_FILTER_FF_2),
    AUDIO_ANC_CONTROL_FILTER_MASK_FB_1        = (1 << AUDIO_ANC_CONTROL_FILTER_FB_1),
    AUDIO_ANC_CONTROL_FILTER_MASK_FB_2        = (1 << AUDIO_ANC_CONTROL_FILTER_FB_2),
} dsp_anc_control_filter_mask_t;

/** @brief hal audio device done closure entry for delay off procedure */
typedef void (*dsp_anc_notify_entry)(dsp_anc_callback_event_t event, dsp_anc_callback_param_t *param);

void dsp_set_anc_notify_callback(dsp_anc_notify_entry anc_notify_entry);
void dsp_get_anc_notify_param(dsp_anc_callback_param_t *param);
void dsp_anc_control_get_status(uint8_t *enable, dsp_anc_control_type_t *type, dsp_anc_control_misc_t *control_misc);
dsp_anc_control_result_t dsp_anc_control_set_ramp_capability(dsp_anc_control_filter_mask_t filter_mask, int16_t gain_value, uint8_t ramp_delay, uint8_t down_step, uint8_t up_step, dsp_anc_control_misc_t *control_misc);
dsp_anc_control_result_t dsp_anc_control_set_biquad_gain(dsp_anc_control_filter_mask_t filter_mask, int32_t bq_gain_value, dsp_anc_control_misc_t *control_misc);
int32_t dsp_anc_control_get_biquad_gain(dsp_anc_control_filter_t filter, dsp_anc_control_misc_t *control_misc);
void dsp_anc_apply_ramp_gain_by_audio_scenario(dsp_anc_control_audio_scenario_type_t scenario_type, bool enable, dsp_anc_control_misc_t *control_misc);

void anc_port_log_info(int32_t index, const char *message, uint32_t arg_cnt, ...);
void anc_port_log_notice(int32_t index, const char *message, uint32_t arg_cnt, ...);
void anc_port_log_error(int32_t index, const char *message, uint32_t arg_cnt, ...);
void anc_port_log_debug(int32_t index, const char *message, uint32_t arg_cnt, ...);

#endif //#ifdef MTK_ANC_ENABLE
#endif //#ifndef __ANC_API_H__


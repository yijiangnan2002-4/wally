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

#ifndef _DSP_STREAM_TASK_H_
#define _DSP_STREAM_TASK_H_

#include "dsp_callback.h"

/******************************************************************************
 * Constant  Definitions
 ******************************************************************************/
#define DSP_STREAM_TASK_MAX_NUMBER  6


/**Expected max execution time of each task, show warning log**/
#define DHP_TASK_MAX_RUNTIME 4500
#define DAV_TASK_MAX_RUNTIME 18000
#define DPR_TASK_MAX_RUNTIME 36000
#define DTDM_TASK_MAX_RUNTIME 4500
#define DDCHS_TASK_MAX_RUNTIME 4500


/**stream number**/
#ifdef AIR_MULTI_MIC_STREAM_ENABLE
#define NO_OF_HP_STREAM_FOR_MULTI_MIC_STREAM 4
#else
#define NO_OF_HP_STREAM_FOR_MULTI_MIC_STREAM 0
#endif
#if defined(AIR_GAMING_MODE_DONGLE_ENABLE)
#define NO_OF_HP_STREAM_FOR_GAMING_MODE_DONGLE 2
#else
#define NO_OF_HP_STREAM_FOR_GAMING_MODE_DONGLE 0
#endif
#ifdef AIR_WIRED_AUDIO_ENABLE
#define NO_OF_HP_STREAM_For_WIRED_AUDIO 2 ////line in(1)/usb-in(2)
#else
#define NO_OF_HP_STREAM_For_WIRED_AUDIO 0
#endif
#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE)
#define NO_OF_HP_STREAM_FOR_ADVANCED_PASSTHROUGH 1
#else
#define NO_OF_HP_STREAM_FOR_ADVANCED_PASSTHROUGH 0
#endif
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
#define NO_OF_HP_STREAM_FOR_BLE_AUDIO_DONGLE 2
#else
#define NO_OF_HP_STREAM_FOR_BLE_AUDIO_DONGLE 0
#endif
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
#define NO_OF_HP_STREAM_FOR_ULL_AUDIO_V2_DONGLE 2
#else
#define NO_OF_HP_STREAM_FOR_ULL_AUDIO_V2_DONGLE 0
#endif
#if defined(AIR_WIRELESS_MIC_RX_ENABLE)
#define NO_OF_HP_STREAM_FOR_WIRELESS_MIC_RX 3
#else
#define NO_OF_HP_STREAM_FOR_WIRELESS_MIC_RX 0
#endif
#if defined(AIR_FULL_ADAPTIVE_ANC_ENABLE)
#define NO_OF_HP_STREAM_FOR_FULL_ADAPTIVE_ANC 1
#else
#define NO_OF_HP_STREAM_FOR_FULL_ADAPTIVE_ANC 0
#endif
#define NO_OF_HP_STREAM         (1 + NO_OF_HP_STREAM_FOR_MULTI_MIC_STREAM + NO_OF_HP_STREAM_FOR_GAMING_MODE_DONGLE + NO_OF_HP_STREAM_For_WIRED_AUDIO + NO_OF_HP_STREAM_FOR_ADVANCED_PASSTHROUGH + NO_OF_HP_STREAM_FOR_BLE_AUDIO_DONGLE + NO_OF_HP_STREAM_FOR_ULL_AUDIO_V2_DONGLE + NO_OF_HP_STREAM_FOR_WIRELESS_MIC_RX + NO_OF_HP_STREAM_FOR_FULL_ADAPTIVE_ANC)

#ifdef AIR_MULTI_MIC_STREAM_ENABLE
#define NO_OF_TDM_STREAM         4
#else
#define NO_OF_TDM_STREAM         1
#endif

#ifdef AIR_MULTI_MIC_STREAM_ENABLE
#define NO_OF_DCHS_STREAM         4
#else
#define NO_OF_DCHS_STREAM         1
#endif

#define NO_OF_PR_STREAM         (4)

#define NO_OF_AV_STREAM         (3)
/******************************************************************************
 * Type Definitions
 ******************************************************************************/

/**stream task id**/
typedef enum {
    STREAM_TASK_PR = 0,
    STREAM_TASK_AV,
    STREAM_TASK_HP,
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_TASK_ENABLE
    STREAM_TASK_TDM,
#endif
#ifdef AIR_DCHS_MODE_ENABLE
    STREAM_TASK_DCHS,
#endif
    STREAM_TASK_END,
} dsp_steram_task_id_t;

typedef struct dsp_stream_task_s {
    DSP_STREAMING_PARA_PTR stream_ptr;
    uint32_t stream_number: 4;
    uint32_t task_idx: 4;
    uint32_t max_runtime: 24;
    TaskHandle_t task_id;
    VOID (*entry)(VOID *);
} dsp_stream_task_t, *dsp_stream_task_pointer_t;

typedef struct {
    dsp_steram_task_id_t            task_id;                /**< task id */
    const char                      task_name[10];          /**< task name */
    const configSTACK_DEPTH_TYPE    stack_depth;            /**< task stack depth */
    UBaseType_t                     priority;               /**< task priority */
    uint32_t                        no_of_stream;           /**< number of stream */
    uint32_t                        max_runtime;            /**< if task runtime>max_runtime, show debug message  */
} stream_task_config_t, *stream_task_config_ptr_t;

typedef struct {
    audio_scenario_type_t           scenario_type;          /**< scenario Type  */
    dsp_steram_task_id_t            task_id;                /**< task id */
    uint32_t                        afe_buffer_threshold;   /**< afe buffer warning logs threshold */
} task_manage_t, *task_manage_ptr_t;


/******************************************************************************
 * External Global Variables
 ******************************************************************************/
EXTERN TaskHandle_t  pStreamTaskHandler[];

#define DPR_TASK_ID pStreamTaskHandler[STREAM_TASK_PR]
#define DAV_TASK_ID pStreamTaskHandler[STREAM_TASK_AV]
#define DHP_TASK_ID pStreamTaskHandler[STREAM_TASK_HP]
#ifdef AIR_AUDIO_I2S_SLAVE_TDM_TASK_ENABLE
#define DTDM_TASK_ID pStreamTaskHandler[STREAM_TASK_TDM]
#endif
#if defined AIR_DCHS_MODE_ENABLE || defined AIR_MIXER_STREAM_ENABLE
#define DDCHS_TASK_ID pStreamTaskHandler[STREAM_TASK_DCHS]
#endif

/******************************************************************************
 * External Function Prototypes
 ******************************************************************************/
EXTERN VOID dsp_stream_task_initialize_all_parameters(VOID);
EXTERN VOID dsp_stream_task(VOID);
EXTERN TaskHandle_t dsp_stream_configure_streaming(DSP_CALLBACK_STREAM_CONFIG_PTR stream_config_ptr, TaskHandle_t task_id);
EXTERN DSP_CALLBACK_PTR dsp_stream_get_callback(SOURCE source, SINK sink);
EXTERN TaskHandle_t dsp_stream_get_task_id(SOURCE source, SINK sink);
//EXTERN VOID dsp_stream_task_parameters_init(VOID);
EXTERN VOID dsp_stream_deinit_all(VOID);
EXTERN VOID dsp_stream_task_create(VOID);
EXTERN VOID dsp_stream_config_table_init(stream_task_config_ptr_t config_table_ptr, task_manage_ptr_t manage_table_ptr);
EXTERN VOID dsp_stream_task_init(VOID);
EXTERN void stream_task_table_initialize(void);

#endif /* _DSP_STREAM_TASK_H_ */


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

#include "config.h"
#include "dsp.h"
#include "dsp_interface.h"
#include "dsp_audio_ctrl.h"
#include "dsp_drv_afe.h"
#include "dsp_sdk.h"
#include "task_def.h"
#include "stream.h"
#include "dsp_task.h"
#include "stream_audio_setting.h"



#ifdef PRELOADER_ENABLE
#include "preloader_pisplit.h"
#endif

#include "dsp_memory_region.h"

VOID* pvTaskParameter;

/**
 * DSP_Init
 *
 * Initialization for DSP Tasks
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 */
VOID DSP_Init(VOID)
{
#ifdef AIR_DSP_MEMORY_REGION_ENABLE
    dsp_memory_init();
#endif
    xTaskCreate((TaskFunction_t)DTM, "DTM_TASK", 0x1000 / sizeof(StackType_t), pvTaskParameter, TASK_PRIORITY_NORMAL, &pDTM_TaskHandler);
#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE) || defined(AIR_HEARTHROUGH_MAIN_ENABLE) || defined(AIR_CUSTOMIZED_LLF_ENABLE)
    xTaskCreate( (TaskFunction_t)DSP_LL_Task, "DLL_TASK", 0x8A0 / sizeof(StackType_t), pvTaskParameter, TASK_PRIORITY_HARD_REALTIME, &pDLL_TaskHandler );
#endif


    stream_task_table_initialize();

    dsp_stream_task_create();

    DSP_CTRL_Initialization();
#ifdef PRELOADER_ENABLE
    extern void pisplit_configure_static_pool();
    preloader_pisplit_init();
    pisplit_configure_static_pool();
#endif
    Stream_Init();
    dsp_sdk_initialize();
    dsp_stream_task_init();
}

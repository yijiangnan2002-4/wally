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

#if defined(AIR_SURROUND_AUDIO_ENABLE)

/* Includes ------------------------------------------------------------------*/
#include "dsp_feature_interface.h"
#include "dsp_audio_process.h"
#include "surround_audio.h"
#include "dsp_dump.h"
#include "dsp_memory.h"
#include "memory_attribute.h"
#include "preloader_pisplit.h"
#include "hal_gpt.h"

/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
/**
 * @brief This function is used to initialize the surround audio run-time environment.
 *
 * @param para is the input parameters.
 * @return true means there is a error.
 * @return false means there is no error.
 */
bool stream_function_surround_audio_initialize(void *para)
{
    UNUSED(para);

    /* Add your surround audio initialize code here */
    DSP_MW_LOG_I("[surround_audio] init done.!, %d", 0);

    return false;
}

/**
 * @brief This function is used to do the surround audio process.
 *
 * @param para is the input parameters.
 * @return true means there is a error.
 * @return false means there is no error.
 */
ATTR_TEXT_IN_IRAM bool stream_function_surround_audio_process(void *para)
{
    DSP_STREAMING_PARA_PTR stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    uint32_t channel_number = stream_function_get_channel_number(para);
    uint32_t in_frame_size = stream_function_get_output_size(para);

    if ((in_frame_size != 0) && (channel_number > 2))
    {
        /* Add your surround audio initialize code here */
    }

    /* modify output channel number */
    stream_ptr->callback.EntryPara.out_channel_num = 2;

    return false;
}

#endif /* AIR_SURROUND_AUDIO_ENABLE */

/* Copyright Statement:
 *
 * (C) 2022  Airoha Technology Corp. All rights reserved.
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

#include <stdint.h>
#include <stdio.h>
#include <xtensa/hal.h>
#include <xtensa/xtruntime.h>
#include "matchning_table.h"

#ifdef AIR_DSP_MEMORY_REGION_ENABLE
#define DSP_MATCHING_TABLE_FEATURE_NUMBER       20
#define DSP_MATCHING_TABLE_MIDDLEWARE_NUMBER    10


#if (DSP_MATCHING_TABLE_FEATURE_NUMBER > DSP_MEMORY_FEATURE_VENDER_NUMBER)
#error "DSP matching table feature error !!"
#endif

#if (DSP_MATCHING_TABLE_MIDDLEWARE_NUMBER > DSP_MEMORY_MIDDLEWARE_VENDER_TABLE_NUMBER)
#error "DSP matching table middleware error !!"
#endif


/**
 * @brief This table is used for customer to configure subcomponent_type by component ID and feature type
 *
 * @param feature_type is the feature type for comparison
 * @param component_type is the component ID for comparison
 * @param subcomponent_type is dedicated sub-component ID for the component_type
 * @param separated_instance_type is dedicated sub-component ID for separating working buffer from PIC
 *
 */
const dsp_memory_common_feature_subcomponent_map_t g_dsp_matching_table_feature_subcomponent[DSP_MATCHING_TABLE_FEATURE_NUMBER] = {
    //component_type                                   //feature_type                            //subcomponent_type                                      //separated_instance_type
    {DSP_COMPONENT_NO_USE,            DSP_DECODER_TYPE,                  DSP_MEMORY_NO_USE,                               DSP_MEMORY_NO_USE},
};


/**
 * @brief This table is used for customer to configure mapping ID by Scenario ID.
 *
 * @param scenario_type is user scenario ID
 * @param component_type is memory management component ID
 * @param fw_subcomponent_type is stream_feature_table_t DRAM ID
 * @param stream_in_subcomponent_type is stream input buffer DRAM ID,
 * @param stream_out_subcomponent_type is stream output buffer DRAM ID
 *
 */
const dsp_memory_middleware_id_t g_dsp_matching_table_middleware_table[DSP_MATCHING_TABLE_MIDDLEWARE_NUMBER] = {
     //scenario_type                                            //component_type,                         //fw_subcomponent_type,               //stream_in_subcomponent_type,            //stream_out_subcomponent_type,
    {AUDIO_SCENARIO_TYPE_COMMON,          DSP_COMPONENT_NO_USE,                    DSP_MEMORY_NO_USE,                    DSP_MEMORY_NO_USE,                    DSP_MEMORY_NO_USE},
};



void dsp_matching_table_initialize(void)
{
    dsp_memory_region_register_coutomer_feature((dsp_memory_common_feature_subcomponent_map_t *)g_dsp_matching_table_feature_subcomponent, DSP_MATCHING_TABLE_FEATURE_NUMBER);
    dsp_memory_region_register_coutomer_middleware((dsp_memory_middleware_id_t *)g_dsp_matching_table_middleware_table, DSP_MATCHING_TABLE_MIDDLEWARE_NUMBER);
}


#endif


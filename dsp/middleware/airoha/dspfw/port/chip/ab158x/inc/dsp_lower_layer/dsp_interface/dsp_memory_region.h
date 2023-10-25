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

#ifndef _DSP_MEMORY_REGION_H_
#define _DSP_MEMORY_REGION_H_

#ifdef AIR_DSP_MEMORY_REGION_ENABLE

#include <string.h>
#include "types.h"
#include "dlist.h"
#include "dsp_task.h"
#include "dsp_sdk.h"
#include "Component.h"
#include "ComponentPreDefine.h"
#include "dsp_predefine.h"


////////////////////////////////////////////////////////////////////////////////
// CONSTANT DEFINITIONS ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/** @brief This the number of g_dsp_memory_feature_subcomponent_table_vender.*/
#define DSP_MEMORY_FEATURE_VENDER_NUMBER 20

/** @brief This the number of g_dsp_memory_middleware_table_vender.*/
#define DSP_MEMORY_MIDDLEWARE_VENDER_TABLE_NUMBER 10


////////////////////////////////////////////////////////////////////////////////
// TYPE DEFINITIONS ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/* specify component ID and FW DRAM ID from different scenario ID*/
typedef enum
{
    DSP_MEMORY_STATUS_UNSUPPORTED_TYPE = -1,
    DSP_MEMORY_STATUS_UNUSED = 0,
    DSP_MEMORY_STATUS_OCCUPIED = 1,

}dsp_memory_status_t;


typedef struct
{
    audio_scenario_type_t   scenario_type;
    TotalComponentType_t    component_type;
    SubComponentType_t      fw_subcomponent_type;
    SubComponentType_t      stream_in_subcomponent_type;
    SubComponentType_t      stream_out_subcomponent_type;
} dsp_memory_middleware_id_t;


typedef struct
{
    TotalComponentType_t    component_type;
    stream_feature_type_t   feature_type;
    SubComponentType_t      subcomponent_type;
    SubComponentType_t      separated_instance_type;
} dsp_memory_common_feature_subcomponent_map_t;


typedef struct
{
    SubComponentType_t      subcomponent_type;
} dsp_memory_feature_subcomponent_table_t;


typedef struct
{
    SubComponentType_t      subcomponent_type;
    /* For Group Memory Acquireand and Release */
    GroupMemoryInfo_t       *subcomponent_memory_info_ptr;
} dsp_memory_feature_subcomponent_info_t, *dsp_memory_feature_subcomponent_info_ptr;



/* For Group Memory Acquireand and Release */
typedef struct
{
    dsp_memory_status_t     status;
    GroupMemoryInfo_t       fw_memory_info;
    GroupMemoryInfo_t       stream_in_memory_info;
    GroupMemoryInfo_t       stream_out_memory_info;
} dsp_memory_middleware_id_info_t;

typedef struct
{
    GroupMemoryInfo_t       subcomponent_memory_info;
    GroupMemoryInfo_t       instance_memory_info;
} dsp_memory_common_feature_subcomponent_map_info_t;

typedef struct
{
    GroupMemoryInfo_t       subcomponent_memory_info;
} dsp_memory_feature_subcomponent_table_info_t;


////////////////////////////////////////////////////////////////////////////////
// External Variables //////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
// Macro ///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// FUNCTION DECLARATIONS ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DspMemoryManagerReturnStatus_t dsp_memory_init();
void dsp_memory_region_register_coutomer_feature(dsp_memory_common_feature_subcomponent_map_t *table_ptr, uint32_t feature_number);
void dsp_memory_region_register_coutomer_middleware(dsp_memory_middleware_id_t *table_ptr, uint32_t table_number);
TotalComponentType_t dsp_memory_get_component_by_scenario (audio_scenario_type_t scenario_type);
SubComponentType_t dsp_memory_get_fw_subcomponent_by_scenario (audio_scenario_type_t scenario_type);
SubComponentType_t dsp_memory_get_stream_in_subcomponent_by_scenario (audio_scenario_type_t scenario_type);
SubComponentType_t dsp_memory_get_stream_out_subcomponent_by_scenario (audio_scenario_type_t scenario_type);
dsp_memory_feature_subcomponent_info_t dsp_memory_get_memory_id_by_feature_type (audio_scenario_type_t scenario_type, stream_feature_type_t feature_type, dsp_memory_feature_subcomponent_info_ptr separated_instance_ram);
GroupMemoryInfo_t *dsp_memory_get_fw_memory_info_by_scenario (audio_scenario_type_t scenario_type);
GroupMemoryInfo_t *dsp_memory_get_stream_in_memory_info_by_scenario (audio_scenario_type_t scenario_type);
GroupMemoryInfo_t *dsp_memory_get_stream_out_memory_info_by_scenario (audio_scenario_type_t scenario_type);

dsp_memory_status_t dsp_memory_get_status_by_scenario (audio_scenario_type_t scenario_type);
dsp_memory_status_t dsp_memory_set_status_by_scenario (audio_scenario_type_t scenario_type, dsp_memory_status_t set_status);



#endif /*  AIR_DSP_MEMORY_REGION_ENABLE  */

#endif /* _DSP_MEMORY_REGION_H_ */


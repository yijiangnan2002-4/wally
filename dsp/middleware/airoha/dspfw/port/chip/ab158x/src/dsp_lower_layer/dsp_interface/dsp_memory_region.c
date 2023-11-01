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
#include <string.h>
#include "config.h"
#include "types.h"
#include "dlist.h"
#include "dsp_temp.h"

#include "dsp_memory_region.h"

#include "preloader_pisplit.h"
#include "syslog.h"



#ifdef AIR_DSP_MEMORY_REGION_ENABLE

log_create_module(DSP_MEMORY, PRINT_LEVEL_DEBUG);

#define DSP_MEMORY_DEBUG_LOG
#if 1
#define DSP_MEMORY_LOG_E(_message, arg_cnt, ...)  LOG_MSGID_E(DSP_MEMORY,_message, arg_cnt, ##__VA_ARGS__)
#define DSP_MEMORY_LOG_W(_message, arg_cnt, ...)  LOG_MSGID_W(DSP_MEMORY,_message, arg_cnt, ##__VA_ARGS__)
#define DSP_MEMORY_LOG_I(_message, arg_cnt, ...)  LOG_MSGID_I(DSP_MEMORY,_message, arg_cnt, ##__VA_ARGS__)
#ifdef DSP_MEMORY_DEBUG_LOG
#define DSP_MEMORY_LOG_D(_message, arg_cnt, ...)  LOG_MSGID_D(DSP_MEMORY,_message, arg_cnt, ##__VA_ARGS__)
#else
#define DSP_MEMORY_LOG_D(_message, arg_cnt, ...)
#endif
#else
#define DSP_MEMORY_LOG_E(_message, arg_cnt, ...)  LOG_E(DSP_MEMORY,_message, ##__VA_ARGS__)
#define DSP_MEMORY_LOG_W(_message, arg_cnt, ...)  LOG_W(DSP_MEMORY,_message, ##__VA_ARGS__)
#define DSP_MEMORY_LOG_I(_message, arg_cnt, ...)  LOG_I(DSP_MEMORY,_message, ##__VA_ARGS__)
#define DSP_MEMORY_LOG_D(_message, arg_cnt, ...)  LOG_D(DSP_MEMORY,_message, ##__VA_ARGS__)
#endif


////////////////////////////////////////////////////////////////////////////////
// Constant Definitions ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////
// Global Variables ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



/**
 * @brief This table is used to configure subcomponent_type by component for common feature type.
 *          SDK won't get/free DMM memory for feature(PIC/working buffer) if it is not defined in this table.
 *
 * @param feature_type is the feature type for comparison
 * @param component_type is the component ID for comparison
 * @param subcomponent_type is dedicated sub-component ID for the component_type
 * @param separated_instance_type is dedicated sub-component ID for separating working buffer from PIC
 *
 */
dsp_memory_common_feature_subcomponent_map_t g_dsp_memory_feature_subcomponent_table[] = {
     //component_type                          //feature_type                               //subcomponent_type                          //separated_instance_type

    {Component_LE_AUDIO,         CODEC_DECODER_LC3,         SubComponent_LEAUDIO_LC3_DEC,                    DSP_MEMORY_NO_USE},
    {Component_LE_AUDIO,FUNC_CLK_SKEW_BLE_MUSIC_DL,         SubComponent_LEAUDIO_DL_SKEW,                    DSP_MEMORY_NO_USE},


    {Component_LE_CALL,            FUNC_RX_SWB_DRC,      SubComponent_LECALL_SWB_DRC_LIB,       SubComponent_LECALL_RX_SWB_DRC},
    {Component_LE_CALL,            FUNC_TX_SWB_DRC,      SubComponent_LECALL_SWB_DRC_LIB,       SubComponent_LECALL_TX_SWB_DRC},
    {Component_LE_CALL,          CODEC_DECODER_LC3,          SubComponent_LECALL_LC3_DEC,                    DSP_MEMORY_NO_USE},
    {Component_LE_CALL,          CODEC_ENCODER_LC3,          SubComponent_LECALL_LC3_DEC,                    DSP_MEMORY_NO_USE},
    {Component_LE_CALL,                 FUNC_RX_NR,           SubComponent_LECALL_NR_LIB,            SubComponent_LECALL_RX_NR},
    {Component_LE_CALL,                 FUNC_TX_NR,           SubComponent_LECALL_NR_LIB,            SubComponent_LECALL_TX_NR},
    {Component_LE_CALL,           FUNC_MIC_SW_GAIN,      SubComponent_LECALL_MIC_SW_GAIN,                    DSP_MEMORY_NO_USE},
    {Component_LE_CALL,  FUNC_CLK_SKEW_BLE_CALL_UL,      SubComponent_LECALL_CLK_SKEW_UL,                    DSP_MEMORY_NO_USE},
    {Component_LE_CALL,  FUNC_CLK_SKEW_BLE_CALL_DL,      SubComponent_LECALL_CLK_SKEW_DL,                    DSP_MEMORY_NO_USE},


    {Component_HFP,                     FUNC_RX_NR,            SubComponent_HFP_ECNR_LIB,             SubComponent_HFP_ECNR_DL},
    {Component_HFP,                       FUNC_PLC,                 SubComponent_HFP_PLC,                    DSP_MEMORY_NO_USE},
    {Component_HFP,                 FUNC_RX_WB_DRC,       SubComponent_HFP_VOICE_DRC_LIB,        SubComponent_HFP_VOICE_DRC_DL},
    {Component_HFP,                     FUNC_TX_NR,            SubComponent_HFP_ECNR_LIB,                    DSP_MEMORY_NO_USE},
    {Component_HFP,                     FUNC_TX_EQ,               SubComponent_HFP_TX_EQ,                    DSP_MEMORY_NO_USE},
    {Component_HFP,                 FUNC_TX_WB_DRC,       SubComponent_HFP_VOICE_DRC_LIB,        SubComponent_HFP_VOICE_DRC_UL},
    {Component_HFP,               FUNC_MIC_SW_GAIN,         SubComponent_HFP_MIC_SW_GAIN,                    DSP_MEMORY_NO_USE},
    {Component_HFP,             CODEC_DECODER_MSBC,            SubComponent_HFP_MSBC_DEC,                    DSP_MEMORY_NO_USE},
    {Component_HFP,             CODEC_ENCODER_MSBC,            SubComponent_HFP_MSBC_ENC,                    DSP_MEMORY_NO_USE},
    {Component_HFP,           FUNC_CLK_SKEW_HFP_UL,        SubComponent_HFP_CLK_SKEW_LIB,          SubComponent_HFP_CLKSKEW_UL},
    {Component_HFP,           FUNC_CLK_SKEW_HFP_DL,        SubComponent_HFP_CLK_SKEW_LIB,          SubComponent_HFP_CLKSKEW_DL},


    {Component_A2DP,             CODEC_DECODER_SBC,                SubComponent_A2DP_SBC,                    DSP_MEMORY_NO_USE},
    {Component_A2DP,             CODEC_DECODER_AAC,                SubComponent_A2DP_AAC,                    DSP_MEMORY_NO_USE},
    {Component_A2DP,          CODEC_DECODER_VENDOR,             SubComponent_A2DP_VENDOR,                    DSP_MEMORY_NO_USE},
    {Component_A2DP,         FUNC_CLK_SKEW_A2DP_DL,           SubComponent_A2DP_CLK_SKEW,                    DSP_MEMORY_NO_USE},
    {Component_A2DP,                FUNC_AUDIO_PLC,                SubComponent_A2DP_PLC,                    DSP_MEMORY_NO_USE},
    {Component_A2DP,                      FUNC_PEQ,                SubComponent_A2DP_PEQ,                    DSP_MEMORY_NO_USE},
    {Component_A2DP,                      FUNC_DRC,                SubComponent_A2DP_DRC,                    DSP_MEMORY_NO_USE},
    {Component_A2DP,                     FUNC_PEQ2,               SubComponent_A2DP_PEQ2,                    DSP_MEMORY_NO_USE},

    {Component_LINEIN,            FUNC_MIC_SW_GAIN,      SubComponent_LINEIN_MIC_SW_GAIN,                    DSP_MEMORY_NO_USE},

    {Component_WWE,               FUNC_MIC_SW_GAIN,         SubComponent_WWE_MIC_SW_GAIN,                    DSP_MEMORY_NO_USE},
    {Component_WWE,               FUNC_WWE_PREPROC,             SubComponent_WWE_PREPROC,                    DSP_MEMORY_NO_USE},
    {Component_WWE,                  FUNC_WWE_PROC,                SubComponent_WWE_PROC,                    DSP_MEMORY_NO_USE},

    {Component_RECORD,            FUNC_MIC_SW_GAIN,      SubComponent_RECORD_MIC_SW_GAIN,                    DSP_MEMORY_NO_USE},

    {Component_MULTI_MIC,         FUNC_MIC_SW_GAIN,           SubComponent_MULTIMIC_GAIN,                    DSP_MEMORY_NO_USE},
    {Component_MULTI_MIC,         FUNC_MIC_SW_GAIN,           SubComponent_MULTIMIC_GAIN,                    DSP_MEMORY_NO_USE},
    {Component_MULTI_MIC,         FUNC_FUNCTION_A,          SubComponent_MULTIMIC_FUNC_A,                    DSP_MEMORY_NO_USE},
    {Component_MULTI_MIC,         FUNC_FUNCTION_B,          SubComponent_MULTIMIC_FUNC_B,                    DSP_MEMORY_NO_USE},
    {Component_MULTI_MIC,         FUNC_FUNCTION_C,          SubComponent_MULTIMIC_FUNC_C,                    DSP_MEMORY_NO_USE},
    {Component_MULTI_MIC,         FUNC_FUNCTION_D,          SubComponent_MULTIMIC_FUNC_D,                    DSP_MEMORY_NO_USE},
    {Component_MULTI_MIC,         FUNC_FUNCTION_E,          SubComponent_MULTIMIC_FUNC_E,                    DSP_MEMORY_NO_USE},

    {Component_ESA,               FUNC_FUNCTION_F,               SubComponent_ESA_FUNC_F,                    DSP_MEMORY_NO_USE},

};


/**
 * @brief This table is used to configure mapping ID by Scenario ID.
 *          SDK won't get/free DMM memory for middleware(stream usage) if it is not defined in this table.
 *
 * @param component_type is memory management component ID
 * @param fw_subcomponent_type is stream_feature_table_t DRAM ID
 * @param stream_in_subcomponent_type is stream input buffer DRAM ID,
 * @param stream_out_subcomponent_type is stream output buffer DRAM ID
 *
 */
dsp_memory_middleware_id_t g_dsp_memory_middleware_table[] = {
     //scenario_type                                                                          //component_type,                         //fw_subcomponent_type,               //stream_in_subcomponent_type,               //stream_out_subcomponent_type,                                                                       ,
    {AUDIO_SCENARIO_TYPE_VP,                                         Component_VP,                   SubComponent_VP_FW,             SubComponent_VP_STREAM_IN,                    DSP_MEMORY_NO_USE},
    {AUDIO_SCENARIO_TYPE_HFP_UL,                                    Component_HFP,               SubComponent_HFP_TX_FW,         SubComponent_HFP_TX_STREAM_IN,                    DSP_MEMORY_NO_USE},
    {AUDIO_SCENARIO_TYPE_HFP_DL,                                    Component_HFP,               SubComponent_HFP_RX_FW,         SubComponent_HFP_RX_STREAM_IN,       SubComponent_HFP_RX_STREAM_OUT},
    {AUDIO_SCENARIO_TYPE_RECORD,                                 Component_RECORD,               SubComponent_RECORD_FW,         SubComponent_RECORD_STREAM_IN,                    DSP_MEMORY_NO_USE},
    {AUDIO_SCENARIO_TYPE_WWE,                                       Component_WWE,                  SubComponent_WWE_FW,            SubComponent_WWE_STREAM_IN,                    DSP_MEMORY_NO_USE},
    {AUDIO_SCENARIO_TYPE_A2DP,                                     Component_A2DP,                 SubComponent_A2DP_FW,           SubComponent_A2DP_STREAM_IN,         SubComponent_A2DP_STREAM_OUT},
    {AUDIO_SCENARIO_TYPE_BLE_UL,                                Component_LE_CALL,            SubComponent_LECALL_UL_FW,      SubComponent_LECALL_UL_STREAM_IN,                    DSP_MEMORY_NO_USE},
    {AUDIO_SCENARIO_TYPE_BLE_DL,                                Component_LE_CALL,            SubComponent_LECALL_DL_FW,      SubComponent_LECALL_DL_STREAM_IN,    SubComponent_LECALL_DL_STREAM_OUT},
    {AUDIO_SCENARIO_TYPE_BLE_MUSIC_DL,                         Component_LE_AUDIO,           SubComponent_LEAUDIO_DL_FW,     SubComponent_LEAUDIO_DL_STREAM_IN,   SubComponent_LEAUDIO_DL_STREAM_OUT},
    {AUDIO_SCENARIO_TYPE_LINE_IN,                                Component_LINEIN,               SubComponent_LINEIN_FW,         SubComponent_LINEIN_STREAM_IN,                    DSP_MEMORY_NO_USE},
    {AUDIO_SCENARIO_TYPE_MULTI_MIC_STREAM_FUNCTION_A,         Component_MULTI_MIC,            SubComponent_MULTIMIC_FW1,   SubComponent_MULTIMIC_FW1_STREAM_IN,                    DSP_MEMORY_NO_USE},
    {AUDIO_SCENARIO_TYPE_MULTI_MIC_STREAM_FUNCTION_B,               Component_ESA,                  SubComponent_ESA_FW,            SubComponent_ESA_STREAM_IN,                    DSP_MEMORY_NO_USE},
    {AUDIO_SCENARIO_TYPE_TDM,                                       Component_TDM,                  SubComponent_TDM_FW,            SubComponent_TDM_STREAM_IN,          SubComponent_TDM_STREAM_OUT},

};

#define DSP_MEMORY_COMMON_FEATURE_NUMBER sizeof(g_dsp_memory_feature_subcomponent_table)/sizeof(dsp_memory_common_feature_subcomponent_map_t)
#define DSP_MEMORY_MIDDLEWARE_TABLE_NUMBER sizeof(g_dsp_memory_middleware_table)/sizeof(dsp_memory_middleware_id_t)


dsp_memory_common_feature_subcomponent_map_info_t g_dsp_memory_feature_subcomponent_table_memory_ifo[DSP_MEMORY_COMMON_FEATURE_NUMBER];
dsp_memory_common_feature_subcomponent_map_info_t g_dsp_memory_feature_subcomponent_table_vender_memory_ifo[DSP_MEMORY_FEATURE_VENDER_NUMBER];

dsp_memory_middleware_id_info_t g_dsp_memory_middleware_table_memory_ifo[DSP_MEMORY_MIDDLEWARE_TABLE_NUMBER];
dsp_memory_middleware_id_info_t g_dsp_memory_middleware_table_vender_memory_ifo[DSP_MEMORY_MIDDLEWARE_VENDER_TABLE_NUMBER];

//vender table
dsp_memory_common_feature_subcomponent_map_t g_dsp_memory_feature_subcomponent_table_vender[DSP_MEMORY_FEATURE_VENDER_NUMBER];
dsp_memory_middleware_id_t g_dsp_memory_middleware_table_vender[DSP_MEMORY_MIDDLEWARE_VENDER_TABLE_NUMBER];

////////////////////////////////////////////////////////////////////////////////
// DSPMEM FUNCTION DECLARATIONS ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


DspMemoryManagerReturnStatus_t dsp_memory_init()
{
    return DspMemoryManagementInit();
}

void dsp_memory_region_register_coutomer_feature(dsp_memory_common_feature_subcomponent_map_t *table_ptr, uint32_t feature_number)
{
    if (feature_number > DSP_MEMORY_FEATURE_VENDER_NUMBER) {
        DSP_MEMORY_LOG_E("DSP MEMORY register feature exceed number :%d, table size: :%d", 2, feature_number, DSP_MEMORY_FEATURE_VENDER_NUMBER);
        assert(0);
    } else {
        memcpy(g_dsp_memory_feature_subcomponent_table_vender, table_ptr, feature_number*sizeof(dsp_memory_common_feature_subcomponent_map_t));
    }
}

void dsp_memory_region_register_coutomer_middleware(dsp_memory_middleware_id_t *table_ptr, uint32_t table_number)
{
    if (table_number > DSP_MEMORY_MIDDLEWARE_VENDER_TABLE_NUMBER) {
        DSP_MEMORY_LOG_E("DSP MEMORY register middleware exceed number :%d, table size: :%d", 2, table_number, DSP_MEMORY_MIDDLEWARE_VENDER_TABLE_NUMBER);
        assert(0);
    } else {
        memcpy(g_dsp_memory_middleware_table_vender, table_ptr, table_number*sizeof(dsp_memory_middleware_id_t));
    }
}

dsp_memory_middleware_id_t *dsp_memory_get_middleware_ptr (audio_scenario_type_t scenario_type)
{
    uint32_t index;
    for (index = 0 ; index < DSP_MEMORY_MIDDLEWARE_TABLE_NUMBER ; index++) {
        if (g_dsp_memory_middleware_table[index].scenario_type == scenario_type) {
            return &g_dsp_memory_middleware_table[index];
        }
    }
    for (index = 0 ; index < DSP_MEMORY_MIDDLEWARE_VENDER_TABLE_NUMBER ; index++) {
        if (g_dsp_memory_middleware_table_vender[index].scenario_type == scenario_type) {
            return &g_dsp_memory_middleware_table_vender[index];
        }
    }
    DSP_MEMORY_LOG_E("DSP MEMORY need to scenario matching IDs = %d ",1, scenario_type);
    assert(0);
    return 0;
}

dsp_memory_middleware_id_info_t *dsp_memory_get_middleware_id_info_ptr (audio_scenario_type_t scenario_type)
{
    uint32_t index;
    for (index = 0 ; index < DSP_MEMORY_MIDDLEWARE_TABLE_NUMBER ; index++) {
        if (g_dsp_memory_middleware_table[index].scenario_type == scenario_type) {
            return &g_dsp_memory_middleware_table_memory_ifo[index];
        }
    }
    for (index = 0 ; index < DSP_MEMORY_MIDDLEWARE_VENDER_TABLE_NUMBER ; index++) {
        if (g_dsp_memory_middleware_table_vender[index].scenario_type == scenario_type) {
            return &g_dsp_memory_middleware_table_vender_memory_ifo[index];
        }
    }
    DSP_MEMORY_LOG_E("DSP MEMORY need to scenario matching IDs = %d ",1, scenario_type);
    assert(0);
    return 0;
}

TotalComponentType_t dsp_memory_get_component_by_scenario (audio_scenario_type_t scenario_type)
{
    dsp_memory_middleware_id_t *middleware_ptr = NULL;
    middleware_ptr = dsp_memory_get_middleware_ptr(scenario_type);
    if (middleware_ptr) {
        DSP_MEMORY_LOG_D("DSP MEMORY dsp_memory_get_component_by_scenario scenario:%d, com:%d", 2, scenario_type, middleware_ptr->component_type);
        return middleware_ptr->component_type;
    }
    return DSP_COMPONENT_NO_USE;
}

SubComponentType_t dsp_memory_get_fw_subcomponent_by_scenario (audio_scenario_type_t scenario_type)
{
    dsp_memory_middleware_id_t *middleware_ptr = NULL;
    middleware_ptr = dsp_memory_get_middleware_ptr(scenario_type);
    if (middleware_ptr) {
        DSP_MEMORY_LOG_D("DSP MEMORY dsp_memory_get_fw_subcomponent_by_scenario scenario:%d, fw type:%d ", 2, scenario_type, middleware_ptr->fw_subcomponent_type);
        return middleware_ptr->fw_subcomponent_type;
    }
    return DSP_MEMORY_NO_USE;
}

SubComponentType_t dsp_memory_get_stream_in_subcomponent_by_scenario (audio_scenario_type_t scenario_type)
{
    dsp_memory_middleware_id_t *middleware_ptr = NULL;
    middleware_ptr = dsp_memory_get_middleware_ptr(scenario_type);
    if (middleware_ptr) {
        DSP_MEMORY_LOG_D("DSP MEMORY dsp_memory_get_stream_in_subcomponent_by_scenario scenario:%d, stream in type:%d", 2, scenario_type, middleware_ptr->stream_in_dram_id);
        return middleware_ptr->stream_in_subcomponent_type;
    }
    return DSP_MEMORY_NO_USE;
}

SubComponentType_t dsp_memory_get_stream_out_subcomponent_by_scenario (audio_scenario_type_t scenario_type)
{
    dsp_memory_middleware_id_t *middleware_ptr = NULL;
    middleware_ptr = dsp_memory_get_middleware_ptr(scenario_type);
    if (middleware_ptr) {
        DSP_MEMORY_LOG_D("DSP MEMORY dsp_memory_get_stream_out_subcomponent_by_scenario scenario:%d, stream in type:%d", 2, scenario_type, middleware_ptr->stream_out_dram_id);
        return middleware_ptr->stream_out_subcomponent_type;
    }
    return DSP_MEMORY_NO_USE;
}

dsp_memory_feature_subcomponent_info_t dsp_memory_get_memory_id_by_feature_type (audio_scenario_type_t scenario_type, stream_feature_type_t feature_type, dsp_memory_feature_subcomponent_info_ptr separated_instance_ram)
{

    dsp_memory_feature_subcomponent_info_t feature_subcomponent_info;
    feature_subcomponent_info.subcomponent_type = DSP_MEMORY_NO_USE;

    separated_instance_ram->subcomponent_type = DSP_MEMORY_NO_USE;
    if (feature_type < DSP_FEATURE_MAX_NUM) {
        //Check common type by scenario
        TotalComponentType_t    component_type = dsp_memory_get_component_by_scenario(scenario_type);
        uint32_t i;
        for (i=0 ; i<DSP_MEMORY_COMMON_FEATURE_NUMBER ; i++) {
            if ((component_type == g_dsp_memory_feature_subcomponent_table[i].component_type) &&
                (feature_type == g_dsp_memory_feature_subcomponent_table[i].feature_type)) {
                feature_subcomponent_info.subcomponent_type = g_dsp_memory_feature_subcomponent_table[i].subcomponent_type;
                feature_subcomponent_info.subcomponent_memory_info_ptr = &g_dsp_memory_feature_subcomponent_table_memory_ifo[i].subcomponent_memory_info;

                separated_instance_ram->subcomponent_type  = g_dsp_memory_feature_subcomponent_table[i].separated_instance_type;
                separated_instance_ram->subcomponent_memory_info_ptr = &g_dsp_memory_feature_subcomponent_table_memory_ifo[i].instance_memory_info;
                break;
            }
        }
        if (feature_subcomponent_info.subcomponent_type == DSP_MEMORY_NO_USE) {
            for (i=0 ; i<DSP_MEMORY_FEATURE_VENDER_NUMBER ; i++) {
                if ((component_type == g_dsp_memory_feature_subcomponent_table_vender[i].component_type) &&
                    (feature_type == g_dsp_memory_feature_subcomponent_table_vender[i].feature_type)) {
                    feature_subcomponent_info.subcomponent_type = g_dsp_memory_feature_subcomponent_table_vender[i].subcomponent_type;
                    feature_subcomponent_info.subcomponent_memory_info_ptr = &g_dsp_memory_feature_subcomponent_table_vender_memory_ifo[i].subcomponent_memory_info;

                    separated_instance_ram->subcomponent_type  = g_dsp_memory_feature_subcomponent_table_vender[i].separated_instance_type;
                    separated_instance_ram->subcomponent_memory_info_ptr = &g_dsp_memory_feature_subcomponent_table_vender_memory_ifo[i].instance_memory_info;
                    break;
                }
            }
        }

        if ((feature_subcomponent_info.subcomponent_type == DSP_MEMORY_COMMON_SUBCOMPONENT_ID) || (feature_subcomponent_info.subcomponent_type == DSP_MEMORY_NO_USE)) {
            DSP_MEMORY_LOG_W("DSP MEMORY no match ID for scenario_type: %d and feature type:%d", 2, scenario_type, feature_type);
        }


    }
    return feature_subcomponent_info;
}


GroupMemoryInfo_t *dsp_memory_get_fw_memory_info_by_scenario (audio_scenario_type_t scenario_type)
{
    dsp_memory_middleware_id_info_t *info_ptr = NULL;
    info_ptr = dsp_memory_get_middleware_id_info_ptr(scenario_type);
    if (info_ptr) {
        return &info_ptr->fw_memory_info;
    }
    return NULL;
}
GroupMemoryInfo_t *dsp_memory_get_stream_in_memory_info_by_scenario (audio_scenario_type_t scenario_type)
{
    dsp_memory_middleware_id_info_t *info_ptr = NULL;
    info_ptr = dsp_memory_get_middleware_id_info_ptr(scenario_type);
    if (info_ptr) {

        return &info_ptr->stream_in_memory_info;
    }
    return NULL;
}
GroupMemoryInfo_t *dsp_memory_get_stream_out_memory_info_by_scenario (audio_scenario_type_t scenario_type)
{
    dsp_memory_middleware_id_info_t *info_ptr = NULL;
    info_ptr = dsp_memory_get_middleware_id_info_ptr(scenario_type);
    if (info_ptr) {

        return &info_ptr->stream_out_memory_info;
    }
    return NULL;
}


dsp_memory_status_t dsp_memory_get_status_by_scenario (audio_scenario_type_t scenario_type)
{
    dsp_memory_middleware_id_info_t *info_ptr = NULL;
    info_ptr = dsp_memory_get_middleware_id_info_ptr(scenario_type);
    if (info_ptr) {

        return info_ptr->status;
    }
    return DSP_MEMORY_STATUS_UNSUPPORTED_TYPE;
}


ATTR_TEXT_IN_RAM_FOR_MASK_IRQ dsp_memory_status_t dsp_memory_set_status_by_scenario (audio_scenario_type_t scenario_type, dsp_memory_status_t set_status)
{
    uint32_t int_mask;
    dsp_memory_middleware_id_info_t *info_ptr = NULL;
    info_ptr = dsp_memory_get_middleware_id_info_ptr(scenario_type);
    if (info_ptr) {
        hal_nvic_save_and_set_interrupt_mask(&int_mask);
        info_ptr->status = set_status;
        hal_nvic_restore_interrupt_mask(int_mask);
        DSP_MEMORY_LOG_I("DSP MEMORY Set Status Change, Scenario_type:%d, Status:%d", 2, scenario_type, set_status);
        return info_ptr->status;
    }
    return DSP_MEMORY_STATUS_UNSUPPORTED_TYPE;
}

#endif /* AIR_DSP_MEMORY_REGION_ENABLE */


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
#include "ecnr_setting.h"
#include "bt_sink_srv_ami.h"
#include "nvkey.h"

#ifdef AIR_BT_AUDIO_SYNC_ENABLE
uint8_t g_hfp_RX_EQ_mode = 0;
uint8_t g_ble_RX_EQ_mode = 0;
uint32_t *g_nv_normal;
uint32_t *g_nv_mode1;
uint32_t *g_nv_mode2;
bool voice_set_RX_EQ_mode(cm4_dsp_audio_sync_scenario_type_t scenario, uint8_t mode)
{
    audio_src_srv_report("[CALL_RX_EQ] scenario %d, type %d", 2, scenario, mode);
    if (mode >= call_RX_EQ_MODE_MAX) {
        audio_src_srv_report("[CALL_RX_EQ] invalid mode: %d", 1, mode);
        return false;
    }
    switch (scenario) {
        case MCU2DSP_SYNC_REQUEST_HFP:
            if (((mode == call_RX_EQ_MODE_NORMAL) && (g_hfp_RX_EQ_mode == call_RX_EQ_MODE_NORMAL))
                || ((mode == call_RX_EQ_MODE_1) && (g_hfp_RX_EQ_mode == call_RX_EQ_MODE_1))
                || ((mode == call_RX_EQ_MODE_2) && (g_hfp_RX_EQ_mode == call_RX_EQ_MODE_2))) {
                audio_src_srv_report("[CALL_RX_EQ] HFP RX EQ mode: no mode change: %d", 1, mode);
                return false;
            }
            g_hfp_RX_EQ_mode = mode;
            audio_src_srv_report("[CALL_RX_EQ] g_hfp_RX_EQ_mode %d", 1, g_hfp_RX_EQ_mode);
            break;
        case MCU2DSP_SYNC_REQUEST_BLE:
            if (((mode == call_RX_EQ_MODE_NORMAL) && (g_ble_RX_EQ_mode == call_RX_EQ_MODE_NORMAL))
                || ((mode == call_RX_EQ_MODE_1) && (g_ble_RX_EQ_mode == call_RX_EQ_MODE_1))
                || ((mode == call_RX_EQ_MODE_2) && (g_ble_RX_EQ_mode == call_RX_EQ_MODE_2))) {
                audio_src_srv_report("[CALL_RX_EQ] BLE RX EQ mode: no mode change: %d", 1, mode);
                return false;
            }
            g_ble_RX_EQ_mode = mode;
            audio_src_srv_report("[CALL_RX_EQ] g_ble_RX_EQ_mode %d", 1, g_ble_RX_EQ_mode);
            break;
        default:
            return false;
            break;
    }
    return true;
}

void voice_set_RX_EQ_nv(cm4_dsp_audio_sync_scenario_type_t scenario, bool hfp_cvsd)
{
    sysram_status_t nvdm_status;
    uint32_t nvkey_length = 0;

    switch (scenario) {
        case MCU2DSP_SYNC_REQUEST_HFP:
            if (hfp_cvsd) {
                nvdm_status = flash_memory_query_nvdm_data_length(NVKEY_DSP_PARA_NB_RX_EQ, &nvkey_length);
                g_nv_normal = pvPortMalloc(nvkey_length);
                memset(g_nv_normal, 0, sizeof(nvkey_length));
                nvdm_status = flash_memory_read_nvdm_data(NVKEY_DSP_PARA_NB_RX_EQ, (uint8_t *)g_nv_normal, &nvkey_length);
                if (nvdm_status || !nvkey_length) {
                    audio_src_srv_report("[DSP SYNC] Read Nvkey length Fail id:0x%x, status:%d ", 2, NVKEY_DSP_PARA_NB_RX_EQ, nvdm_status);
                }
                nvkey_length = 0;
                nvdm_status = flash_memory_query_nvdm_data_length(NVKEY_DSP_PARA_NB_RX_EQ_3RD, &nvkey_length);
                g_nv_mode1 = pvPortMalloc(nvkey_length);
                memset(g_nv_mode1, 0, sizeof(nvkey_length));
                nvdm_status = flash_memory_read_nvdm_data(NVKEY_DSP_PARA_NB_RX_EQ_3RD, (uint8_t *)g_nv_mode1, &nvkey_length);
                if (nvdm_status || !nvkey_length) {
                    audio_src_srv_report("[DSP SYNC] Read Nvkey length Fail id:0x%x, status:%d ", 2, NVKEY_DSP_PARA_NB_RX_EQ_3RD, nvdm_status);
                }
                nvkey_length = 0;
                nvdm_status = flash_memory_query_nvdm_data_length(NVKEY_DSP_PARA_NB_RX_EQ_4TH, &nvkey_length);
                g_nv_mode2 = pvPortMalloc(nvkey_length);
                memset(g_nv_mode2, 0, sizeof(nvkey_length));
                nvdm_status = flash_memory_read_nvdm_data(NVKEY_DSP_PARA_NB_RX_EQ_4TH, (uint8_t *)g_nv_mode2, &nvkey_length);
                if (nvdm_status || !nvkey_length) {
                    audio_src_srv_report("[DSP SYNC] Read Nvkey length Fail id:0x%x, status:%d ", 2, NVKEY_DSP_PARA_NB_RX_EQ_4TH, nvdm_status);
                }
            } else {
                nvdm_status = flash_memory_query_nvdm_data_length(NVKEY_DSP_PARA_WB_RX_EQ, &nvkey_length);
                g_nv_normal = pvPortMalloc(nvkey_length);
                memset(g_nv_normal, 0, sizeof(nvkey_length));
                nvdm_status = flash_memory_read_nvdm_data(NVKEY_DSP_PARA_WB_RX_EQ, (uint8_t *)g_nv_normal, &nvkey_length);
                if (nvdm_status || !nvkey_length) {
                    audio_src_srv_report("[DSP SYNC] Read Nvkey length Fail id:0x%x, status:%d ", 2, NVKEY_DSP_PARA_WB_RX_EQ, nvdm_status);
                }
                nvkey_length = 0;
                nvdm_status = flash_memory_query_nvdm_data_length(NVKEY_DSP_PARA_WB_RX_EQ_3RD, &nvkey_length);
                g_nv_mode1 = pvPortMalloc(nvkey_length);
                memset(g_nv_mode1, 0, sizeof(nvkey_length));
                nvdm_status = flash_memory_read_nvdm_data(NVKEY_DSP_PARA_WB_RX_EQ_3RD, (uint8_t *)g_nv_mode1, &nvkey_length);
                if (nvdm_status || !nvkey_length) {
                    audio_src_srv_report("[DSP SYNC] Read Nvkey length Fail id:0x%x, status:%d ", 2, NVKEY_DSP_PARA_WB_RX_EQ_3RD, nvdm_status);
                }
                nvkey_length = 0;
                nvdm_status = flash_memory_query_nvdm_data_length(NVKEY_DSP_PARA_WB_RX_EQ_4TH, &nvkey_length);
                g_nv_mode2 = pvPortMalloc(nvkey_length);
                memset(g_nv_mode2, 0, sizeof(nvkey_length));
                nvdm_status = flash_memory_read_nvdm_data(NVKEY_DSP_PARA_WB_RX_EQ_4TH, (uint8_t *)g_nv_mode2, &nvkey_length);
                if (nvdm_status || !nvkey_length) {
                    audio_src_srv_report("[DSP SYNC] Read Nvkey length Fail id:0x%x, status:%d ", 2, NVKEY_DSP_PARA_WB_RX_EQ_4TH, nvdm_status);
                }
            }
            break;
        case MCU2DSP_SYNC_REQUEST_BLE:
            nvdm_status = flash_memory_query_nvdm_data_length(NVKEY_DSP_PARA_SWB_RX_EQ, &nvkey_length);
            g_nv_normal = pvPortMalloc(nvkey_length);
            memset(g_nv_normal, 0, sizeof(nvkey_length));
            nvdm_status = flash_memory_read_nvdm_data(NVKEY_DSP_PARA_SWB_RX_EQ, (uint8_t *)g_nv_normal, &nvkey_length);
            if (nvdm_status || !nvkey_length) {
                audio_src_srv_report("[DSP SYNC] Read Nvkey length Fail id:0x%x, status:%d ", 2, NVKEY_DSP_PARA_SWB_RX_EQ, nvdm_status);
            }
            nvdm_status = flash_memory_query_nvdm_data_length(NVKEY_DSP_PARA_SWB_RX_EQ_3RD, &nvkey_length);
            g_nv_mode1 = pvPortMalloc(nvkey_length);
            memset(g_nv_mode1, 0, sizeof(nvkey_length));
            nvdm_status = flash_memory_read_nvdm_data(NVKEY_DSP_PARA_SWB_RX_EQ_3RD, (uint8_t *)g_nv_mode1, &nvkey_length);
            if (nvdm_status || !nvkey_length) {
                audio_src_srv_report("[DSP SYNC] Read Nvkey length Fail id:0x%x, status:%d ", 2, NVKEY_DSP_PARA_SWB_RX_EQ_3RD, nvdm_status);
            }
            nvdm_status = flash_memory_query_nvdm_data_length(NVKEY_DSP_PARA_SWB_RX_EQ_4TH, &nvkey_length);
            g_nv_mode2 = pvPortMalloc(nvkey_length);
            memset(g_nv_mode2, 0, sizeof(nvkey_length));
            nvdm_status = flash_memory_read_nvdm_data(NVKEY_DSP_PARA_SWB_RX_EQ_4TH, (uint8_t *)g_nv_mode2, &nvkey_length);
            if (nvdm_status || !nvkey_length) {
                audio_src_srv_report("[DSP SYNC] Read Nvkey length Fail id:0x%x, status:%d ", 2, NVKEY_DSP_PARA_SWB_RX_EQ_4TH, nvdm_status);
            }
            break;
        default:
            // error happen
            audio_src_srv_report("[DSP SYNC] am type error [%d]", scenario);
            return;
            break;
    }
}
#endif

void voice_set_avc_enable(bool enable)
{
    uint32_t tableSize;
    nvkey_status_t status = nvkey_data_item_length(NVID_DSP_ALG_NR, &tableSize);
    if (status != NVKEY_STATUS_OK) {
        AUD_LOG_E("[AVC][am_set_avc_enable] query nvdm len Fail. Status:%d Len:%d\n", 2, 2, status, tableSize);
        AUDIO_ASSERT(0);
    }
    U16 *nvkey_value = (U16 *)pvPortMalloc(tableSize);
    AUDIO_ASSERT(nvkey_value && "[AVC][am_check_avc_enable] malloc fail");
    status = nvkey_read_data(NVID_DSP_ALG_NR, (U8 *)nvkey_value, &tableSize);
    if (status != NVKEY_STATUS_OK) {
        AUD_LOG_E("[AVC][am_set_avc_enable] read nvdm Fail. Status:%d tableSize:%d\n", 2, 2, status, tableSize);
        AUDIO_ASSERT(0);
    }
    //set nvkey ++
    nvkey_value[tableSize / 2 - 1] = enable;//last 2 bytes
    //--
    status = nvkey_write_data(NVID_DSP_ALG_NR, (U8 *)nvkey_value, tableSize);
    if (status != NVKEY_STATUS_OK) {
        AUD_LOG_E("[AVC][am_set_avc_enable] write nvdm Fail - err(%d)\r\n", 1, status);
        AUDIO_ASSERT(0);
    }
    vPortFree(nvkey_value);
    //ccni notify dsp
    hal_audio_dsp_controller_send_message(MSG_MCU2DSP_BT_VOICE_DL_AVC_PARA_SEND, 1, (uint32_t)enable, false);
    AUD_LOG_I("[AVC][am_set_avc_enable] set enable:%d success", 1, enable);
}

bool voice_check_avc_enable(void)
{
    uint32_t tableSize;
    nvkey_status_t status = nvkey_data_item_length(NVID_DSP_ALG_NR, &tableSize);
    if (status != NVKEY_STATUS_OK) {
        AUD_LOG_E("[AVC][am_check_avc_enable] query nvdm len Fail. Status:%d, tableSize:%d\n", 2, status, tableSize);
        AUDIO_ASSERT(0);
    }
    U16 *nvkey_value = (U16 *)pvPortMalloc(tableSize);
    AUDIO_ASSERT(nvkey_value && "[AVC][am_check_avc_enable] malloc fail");
    status = nvkey_read_data(NVID_DSP_ALG_NR, (uint8_t *)nvkey_value, &tableSize);
    if (status != NVKEY_STATUS_OK) {
        AUD_LOG_E("[AVC][am_check_avc_enable] read nvdm Fail. Status:%d, tableSize:%d\n", 2, status, tableSize);
        AUDIO_ASSERT(0);
    }
    bool enable = nvkey_value[tableSize / 2 - 1];//last 2 bytes
    AUD_LOG_I("[AVC][am_check_avc_enable] check enable:%d success", 1, enable);
    vPortFree(nvkey_value);
    return enable;
}
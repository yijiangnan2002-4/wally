/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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


/**
 * File: bt_customer_config.c
 *
 * Description: This file defines callback functions to configurate BT
 *
 */

#include "bt_customer_config.h"
#include "bt_app_common.h"
#include "bt_connection_manager_utils.h"
#include "bt_device_manager.h"
#include "syslog.h"
#include "FreeRTOS.h"
#include "nvkey_id_list.h"
#include "nvkey.h"
#include "bt_init.h"
#include "bt_sink_srv_ami.h"
#ifdef AIR_BT_SOURCE_ENABLE
#include "bt_source_srv.h"
#include "usb_main.h"
#include "app_dongle_session_manager.h"
#endif

/***********************************************************************************************
****************** Implement a weak symbol functions declared in middleware. *******************
***********************************************************************************************/

/**
 * @brief    This function implements the weak symbol API declared in bt_aws_mce_srv.h. It is used to ask user to decide aws role, It can be implemented by user.
 * @param[in] remote_addr    The air pairing remote device's address.
 * @return   #bt_aws_mce_role_t.
 */
bt_aws_mce_role_t bt_aws_mce_srv_air_pairing_get_aws_role(const bt_bd_addr_t *remote_addr)
{
    /* If have set audio channel, left channel should be agent. */
    audio_channel_t channel = ami_get_audio_channel();
    bt_bd_addr_t *local_addr;
    if (AUDIO_CHANNEL_L == channel) {
        return BT_AWS_MCE_ROLE_AGENT;
    } else if (AUDIO_CHANNEL_R == channel) {
        return BT_AWS_MCE_ROLE_PARTNER;
    } else {
        /* If have not set audio channel, compare bt address to decide which is agent. */
        LOG_MSGID_I(BT_APP, "[BT_CM][E] Audio channel is NONE, get role by address", 0);
        local_addr = bt_device_manager_aws_local_info_get_fixed_address();
        if (NULL != local_addr && NULL != remote_addr) {
            int32_t i = 0;
            while (i < sizeof(bt_bd_addr_t)) {
                if ((*local_addr)[i] > (*remote_addr)[i]) {
                    return BT_AWS_MCE_ROLE_AGENT;
                } else if ((*local_addr)[i] < (*remote_addr)[i]) {
                    return BT_AWS_MCE_ROLE_PARTNER;
                }
                i++;
            }
            configASSERT(0);
        } else {
            LOG_MSGID_I(BT_APP, "[BT_CM][E]Addr is null,  local addr %p, remote addr %p", 2, local_addr, remote_addr);
        }
    }
    return BT_AWS_MCE_ROLE_NONE;
}
/*************************************************************************************************/

#if 0
/**************** Get parameters for link loss reconnection **************************************/
const static bt_sink_srv_feature_reconnect_params_t link_loss_reconnect_params = {
    .attampts = 0xFF
};

const bt_sink_srv_feature_reconnect_params_t   *bt_sink_srv_get_link_loss_reconnect_params(void)
{
    return &link_loss_reconnect_params;
}
/*************************************************************************************************/

/**************** Get parameters for power on reconnection ***************************************/
const static bt_sink_srv_feature_reconnect_params_t power_on_reconnect_params = {
    .attampts = 0xFF
};

const bt_sink_srv_feature_reconnect_params_t   *bt_sink_srv_get_power_on_reconnect_params(void)
{
    return &power_on_reconnect_params;
}
/*************************************************************************************************/

/**************** Get parameters for visibility **************************************************/
const static bt_sink_srv_feature_visibility_params_t normal_visibility_params = {
    .visibility_duration = 0xFFFFFFFF,
    .power_on_be_visible_once = false
};

const bt_sink_srv_feature_visibility_params_t *bt_sink_srv_get_visibility_params(void)
{
    return &normal_visibility_params;
}
/*************************************************************************************************/
#endif

static uint8_t default_eir_data[240] = {/* 1Byte data length(Besides flag) - 1Byte data type flag - X Bytes data */
    /* 17Bytes data lengt, data type 128bits uuid(6), 128bits data*/
    0x11, 0x06,
    0x1D, 0x23, 0xBB, 0x1B, 0x00, 0x00, 0x10, 0x00, 0x30, 0x00, 0x50, 0x80, 0x5F, 0x9B, 0x34, 0xFA,
    /* Others */
    0x00
}; /* The variable records EIR data */

#if 0
const static bt_sink_srv_eir_information_t eir_params = {
    .uuid_128bit = (const uint8_t *) &default_eir_data,
    .rssi = 0
};

const bt_sink_srv_eir_information_t *bt_sink_srv_get_eir(void)
{
    return &eir_params;
}
/*************************************************************************************************/

/**************** Get the retry times when do switch role ****************************************/
uint8_t bt_sink_srv_get_role_switch_retry_times(void)
{
    return 0xFF;
}
/*************************************************************************************************/

/**************** Get the page timeout parameters ************************************************/
uint16_t bt_sink_srv_get_page_timeout_paramters(void)
{
    return 0x5dc0; /* 0x5dc0 * 0.625 msec = 15 sec */
}
/*************************************************************************************************/
#endif

/*************************************************************************************************/

/**************** Get the parameters for iOS-specific HFP AT commands *************************/
const static bt_sink_srv_hf_custom_command_xapl_params_t bt_customer_config_xapl_params = {
    .vendor_infomation = "MTK-HB-0400",
    .features = BT_SINK_SRV_HF_CUSTOM_FEATURE_NONE | BT_SINK_SRV_HF_CUSTOM_FEATURE_BATTERY_REPORT
}; /* The variable defined the parameters for iOS-specific HFP AT commands, please refer to bt_sink_srv.h */

/**
 * @brief This function implements the weak symbol API declared in bt_sink_srv.h. It is to get the parameters for iOS-specific HFP AT commands. This API invoked by the Sink service when the HFP connection was established.
 *        It should be implemented by the application.
 * @return The ios hf features parameters.
 */
const bt_sink_srv_hf_custom_command_xapl_params_t *bt_sink_srv_get_hfp_custom_command_xapl_params(void)
{
    return &bt_customer_config_xapl_params;
}
/*************************************************************************************************/

/**************************************************************************************************
*************** get configuration and features functions called in project. ***********************
**************************************************************************************************/

static bt_cm_config_t s_cm_config = {
    .max_connection_num = 0x01,     /* dongle only support 1 acl link */
    .connection_takeover = false,
#ifdef AIR_BT_ULTRA_LOW_LATENCY_ENABLE
    .request_role = BT_CM_ROLE_MASTER,
#else
    .request_role = BT_CM_ROLE_SLAVE,
#endif
    .request_role_retry_times = 0xFF,
    .page_timeout = 0xDAC0, /* 0xDAC0 * 0.625 msec = 35 sec */
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE) || defined(AIR_BLE_AUDIO_DONGLE_ENABLE) || defined(AIR_BT_SOURCE_ENABLE)
    .power_on_reconnect_profile = 0,
#else
    .power_on_reconnect_profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP)
    | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK),
#endif
    .power_on_reconnect_duration = 0,
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
    .link_loss_reconnect_profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL),
#elif defined(AIR_BT_SOURCE_ENABLE)
    .link_loss_reconnect_profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP_AG)
                                 | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SOURCE),
#elif defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
    .link_loss_reconnect_profile = 0,
#else
    .link_loss_reconnect_profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP)
    | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK),
#endif
    .link_loss_reconnect_duration = 0,
    .eir_data = {
        .data = default_eir_data,
        .length = 18
    }
}; /* The variable defines the parameters of bt_cm_config_t, refer to bt_connection_manager. */

/**
 * @brief    This function is used to get the bt connection configuration.
 * @return   pointer of bt_cm_config_t.
 */
const bt_cm_config_t *bt_customer_config_get_cm_config(void)
{
    bt_customer_config_get_gap_config();

    return &s_cm_config;
}

/******************** Config the parameter of sink GAP *******************************************/
#define BT_SINK_SRV_CM_DB_NAME  "BT_ULL_Dongle"                         /* Default BT name */
#define BT_SINK_SRV_CM_DB_COD   0x24450C                                /* Default BT COD(Class Of Device) refer to BT spec. */
#define BT_SINK_SRV_CM_DB_IO    BT_GAP_IO_CAPABILITY_NO_INPUT_NO_OUTPUT /* Default GAP IO capability */

static bt_gap_config_t g_bt_sink_srv_gap_config = {
    .inquiry_mode  = 2, /**< It indicates the inquiry result format.
                                                        0: Standerd inquiry result format (Default).
                                                        1: Inquiry result format with RSSI.
                                                        2: Inquiry result with RSSI format or Extended Inquiry Result(EIR) format. */
    .io_capability = BT_SINK_SRV_CM_DB_IO,
    .cod           = BT_SINK_SRV_CM_DB_COD, /* It defines the class of the local device. */
    .device_name   = {BT_SINK_SRV_CM_DB_NAME}, /* It defines the name of the local device with '\0' ending. */
}; /* The variable defined the GAP configuration of the project. */

/**
 * @brief    This function is used to get the GAP configuration.
 * @return   pointer of bt_gap_config_t.
 */
const bt_gap_config_t *bt_customer_config_get_gap_config(void)
{
    uint8_t name[BT_GAP_MAX_DEVICE_NAME_LENGTH] = {0};
    uint32_t name_length;
    uint32_t read_size = BT_GAP_MAX_DEVICE_NAME_LENGTH;
    nvkey_status_t nvkey_status = NVKEY_STATUS_ERROR;

    /* Read from NVID_APP_DEVICE_NAME_USER. */
    nvkey_status = nvkey_read_data(NVID_APP_DEVICE_NAME_USER, name, &read_size);
    if (nvkey_status == NVKEY_STATUS_OK) {
        /* To make sure it is a string */
        name[BT_GAP_MAX_DEVICE_NAME_LENGTH - 1] = '\0';
    }

    /* Read from NVID_APP_DEVICE_NAME_DEFAULT */
    read_size = BT_GAP_MAX_DEVICE_NAME_LENGTH;
    if (nvkey_status != NVKEY_STATUS_OK || strlen((char *)name) == 0) {
        nvkey_status = nvkey_read_data(NVID_APP_DEVICE_NAME_DEFAULT, name, &read_size);
        if (nvkey_status == NVKEY_STATUS_OK) {
            /* To make sure it is a string */
            name[BT_GAP_MAX_DEVICE_NAME_LENGTH - 1] = '\0';
        }
    }

    if (nvkey_status != NVKEY_STATUS_OK || strlen((char *)name) == 0) {
        /* Change BT local name to H_xxx for QA test (xxx is BT addr) */
        bt_bd_addr_t *local_addr;
#ifdef MTK_AWS_MCE_ENABLE
        /* Partner use the peer address to fix the BT name change after RHO */
        if (BT_AWS_MCE_ROLE_PARTNER == bt_device_manager_aws_local_info_get_role()) {
            local_addr = bt_device_manager_aws_local_info_get_peer_address();
        } else
#endif
        {
            local_addr = bt_device_manager_get_local_address();
        }

        snprintf((char *)name, sizeof(name), "H_%.2X%.2X%.2X%.2X%.2X%.2X",
                 (*local_addr)[5], (*local_addr)[4], (*local_addr)[3],
                 (*local_addr)[2], (*local_addr)[1], (*local_addr)[0]);
    }
    name_length = strlen((char *)name);
    memcpy(g_bt_sink_srv_gap_config.device_name, name, name_length + 1);
    LOG_I(BT_APP, "[BT_CM] device name:%s", g_bt_sink_srv_gap_config.device_name);

    default_eir_data[18] = name_length + 1;
    default_eir_data[19] = 0x09;
    memcpy((default_eir_data + 20), g_bt_sink_srv_gap_config.device_name, name_length);
    s_cm_config.eir_data.length = 18 + 2 + name_length;
    return &g_bt_sink_srv_gap_config;
}

/**
 * @brief    This function is used to get the BLE name.
 * @param[out] ble_name  The buffer of BLE name, it will be filled in the fuction.
 * @return   None.
 */
void bt_customer_config_get_ble_device_name(char ble_name[BT_GAP_LE_MAX_DEVICE_NAME_LENGTH])
{
    nvkey_status_t nvkey_status = NVKEY_STATUS_ERROR;
    const bt_gap_config_t *config = bt_customer_config_get_gap_config();

#ifndef GSOUND_LIBRARY_ENABLE
    /* When gsound enable, the BLE device name must be "LE-" + "BT EDR device name" */
    uint32_t read_size = BT_GAP_LE_MAX_DEVICE_NAME_LENGTH;
    nvkey_status = nvkey_read_data(NVID_BT_BLE_ADV_NAME, (uint8_t *)ble_name, &read_size);

    if (nvkey_status == NVKEY_STATUS_OK) {
        ble_name[BT_GAP_LE_MAX_DEVICE_NAME_LENGTH - 1] = '\0';
    }
#endif

    if (nvkey_status != NVKEY_STATUS_OK || strlen((char *)ble_name) == 0) {
        /* Use default BLE device name "LE-" + "BT EDR device name" */
        snprintf((char *)ble_name,
                 BT_GAP_LE_MAX_DEVICE_NAME_LENGTH,
                 "LE-%s",
                 config->device_name);
    }
}

/*************************************************************************************************/

/******************** Get feature config for bt sink *********************************************/
static bt_sink_feature_config_t bt_sink_features = {
    .features = BT_SINK_CONFIGURABLE_FEATURE_NONE
}; /* The variable defined the bt sink service features. Refer to bt_sink_srv.h. */

/**
 * @brief    This function is used to get the bt sink features.
 * @return   The pointer of bt_sink_feature_config_t.
 */
bt_sink_feature_config_t *bt_customer_config_get_bt_sink_features(void)
{
    return &bt_sink_features;
}
/*************************************************************************************************/

/************************* Get parameter for HFP profile *****************************************/
static bt_hfp_audio_codec_type_t g_hfp_audio_codec_type = (bt_hfp_audio_codec_type_t)(BT_HFP_CODEC_TYPE_CVSD | BT_HFP_CODEC_TYPE_MSBC);
/**
 * @brief    This function is used to get HFP parameters.
 * @return   BT_STATUS_SUCCESS or BT_STATUS_FAIL.
 */
bt_status_t bt_customer_config_hf_get_init_params(bt_hfp_init_param_t *param)
{
    /* For low power test, add cmd to modify hf audio codec */
    param->supported_codecs = g_hfp_audio_codec_type;
    param->indicators.service = BT_HFP_INDICATOR_OFF;
    param->indicators.signal = BT_HFP_INDICATOR_OFF;
    param->indicators.roaming = BT_HFP_INDICATOR_OFF;
    param->indicators.battery = BT_HFP_INDICATOR_OFF;
    param->support_features = (bt_hfp_init_support_feature_t)(BT_HFP_INIT_SUPPORT_3_WAY | BT_HFP_INIT_SUPPORT_CODEC_NEG | BT_HFP_INIT_SUPPORT_ENHANCED_CALL_STATUS | \
                                                              BT_HFP_INIT_SUPPORT_ENHANCED_CALL_CONTROL | BT_HFP_INIT_SUPPORT_CLI_PRESENTATION);

    param->disable_nrec = true;
    param->enable_call_waiting = true;
    param->enable_cli = true;
    return BT_STATUS_SUCCESS;
}

/**
 * @brief    This function is used to set the HFP's audio codec.
 * @return   None.
 */
void update_hfp_audio_codec(bt_hfp_audio_codec_type_t new_type)
{
    g_hfp_audio_codec_type = new_type;
}

bt_hfp_audio_codec_type_t get_hfp_audio_codec()
{
    return g_hfp_audio_codec_type;
}

/*************************************************************************************************/

/******************* Get parameter for feature mask configuration ********************************/
/**
 * @brief    This function is used to get BT initializing configuration.
 * @return   The BT initializing configuration mask, refer to bt_system.h.
 */
bt_init_feature_mask_t bt_customer_config_get_feature_mask_configuration(void)
{
    //bt_init_feature_mask_t config_feature = BT_INIT_FEATURE_MASK_DISABLE_SNIFF_MODE | BT_INIT_FEATURE_MASK_DISABLE_SNIFF_SUB_MODE | BT_INIT_FEATURE_MASK_BT_5_0;
    bt_init_feature_mask_t config_feature = /*BT_INIT_FEATURE_MASK_DISABLE_SNIFF_MODE | */BT_INIT_FEATURE_MASK_DISABLE_SNIFF_SUB_MODE | BT_INIT_FEATURE_MASK_BT_5_0;
    uint8_t enable_3M_config = 1;
    uint32_t config_size = sizeof(uint8_t);
    nvkey_status_t ret = nvkey_read_data(NVID_APP_A2DP_3M_CONFIG, (uint8_t *)(&enable_3M_config), &config_size);
    if (!enable_3M_config) {
        config_feature |= BT_INIT_FEATURE_MASK_DISABLE_3M;
        bt_a2dp_set_mtu_size(BT_A2DP_DISABLE_3M_MTU_SIZE);
    }
    LOG_MSGID_I(BT_APP, "[BT_CUSTOMER] ret:%d", 1, ret);
    return config_feature;
}
/*************************************************************************************************/
bool bt_sink_srv_aird_support_call_mode(bt_handle_t handle)
{
    return false;
}

#ifdef AIR_BT_SOURCE_ENABLE
#ifdef USB_HOST_DETECT_ENABLE
static const uint16_t g_bt_customer_config_volume_scale[] = {
    50,          /* unkown */
    15,          /* ANDROID */
    15,          /* IOS */
    15,          /* MAC */
    50,          /* WINDOWS */
    15,          /* XBOX */
    15,          /* PS */
    15,          /* SWITCH */
};
#endif

uint16_t g_ag_bqb_feature_mask = 0;
bt_status_t bt_source_srv_get_feature_config(bt_source_srv_t type, void *feature)
{
    LOG_MSGID_I(BT_APP, "[BT_CUSTOMER] source get feature config type = %02x", 1, type);
    bt_status_t status = BT_STATUS_FAIL;

    if (feature == NULL) {
        LOG_MSGID_I(BT_APP, "[BT_CUSTOMER] source get feature config is NULL", 0);
        return status;
    }

    if (type == BT_SOURCE_SRV_TYPE_HFP) {
        bt_source_srv_hfp_feature_config_t *hfp_feature_config = (bt_source_srv_hfp_feature_config_t *)feature;
        hfp_feature_config->feature = BT_SOURCE_SRV_HFP_FEATURE_CALL_REJECT | BT_HFP_AG_FEATURE_ENHANCED_CALL_STATUS | BT_HFP_AG_FEATURE_CODEC_NEGOTIATION |
                                      BT_HFP_AG_FEATURE_ESCO_S4_SETTINGS | BT_HFP_AG_FEATURE_VOICE_RECOGNITION | BT_HFP_AG_FEATURE_3_WAY | BT_HFP_AG_FEATURE_IN_BAND_RING;
        hfp_feature_config->feature &= ~g_ag_bqb_feature_mask;
        /* because not support three-way calling, so hold feature not set */ 
        hfp_feature_config->hold_feature = BT_SOURCE_SRV_HFP_HOLD_FEATURE_RELEASE_HELD_CALL | BT_SOURCE_SRV_HFP_HOLD_FEATURE_RELEASE_ACTIVE_CALL | BT_SOURCE_SRV_HFP_HOLD_FEATURE_HOLD_ACTIVE_CALL;
#ifdef AIR_USB_AUDIO_OUT_ENABLE
        hfp_feature_config->custom_feature = BT_SOURCE_SRV_HFP_CUSTOM_FEATURE_LOCAL_MIC;
#else
        hfp_feature_config->custom_feature = 0;
#endif

        hfp_feature_config->codec_type = BT_SOURCE_SRV_HFP_CODEC_TYPE_CVSD | BT_SOURCE_SRV_HFP_CODEC_TYPE_MSBC;
        status = BT_STATUS_SUCCESS;
    }

    if (type == BT_SOURCE_SRV_TYPE_NONE) {
        bt_source_srv_common_feature_config_t *common_feature_config = (bt_source_srv_common_feature_config_t *)feature;
#ifdef USB_HOST_DETECT_ENABLE
        USB_HOST_TYPE host_type = Get_USB_Host_Type();
        LOG_MSGID_I(BT_APP, "[BT_CUSTOMER] source get host type = %02x", 1, host_type);
        common_feature_config->host_volume_scale = g_bt_customer_config_volume_scale[host_type];
#else
        common_feature_config->host_volume_scale = 50;
#endif
        status = BT_STATUS_SUCCESS;
    }

    return status;
}

#define BT_CUSTOMER_CONFIG_PHONE_CARD_MAX      0x01
static uint8_t g_phone_card_number[] = "02811111111";
static uint8_t g_operator_information[] = "airoha";
uint32_t bt_source_srv_get_phone_card_information(bt_source_srv_t type, bt_source_srv_phone_card_info_t *phone_card, uint32_t phone_card_num)
{
    LOG_MSGID_I(BT_APP, "[BT_CUSTOMER] source get phone card information type = %02x", 1, type);
    if (phone_card == NULL) {
        LOG_MSGID_I(BT_APP, "[BT_CUSTOMER] ource get phone card information is NULL", 0);
        return 0;
    }

    phone_card->own_number = g_phone_card_number;
    phone_card->own_number_length = (uint8_t)strlen((char *)g_phone_card_number);
    phone_card->own_number_type = 129;
    phone_card->operator_information = g_operator_information;
    phone_card->operator_information_length = (uint8_t)strlen((char *)g_operator_information);
    phone_card->operator_mode = BT_SOURCE_SRV_OPERATOR_MODE_AUTOMATIC;
    phone_card->signal_strength = 5;
    phone_card->service_ability = BT_SOURCE_SRV_SERVICE_AVAILABILITY_ACTIVE;
    phone_card->roaming_state = BT_SOURCE_SRV_HFP_ROAMING_STATE_INACTIVE;
    phone_card->own_number_service = BT_SOURCE_SRV_PHONE_NUMBER_SERVICE_VOICE;

    return BT_CUSTOMER_CONFIG_PHONE_CARD_MAX;
}

extern bt_status_t app_dongle_session_manager_handle_edr_session_negotiation(bt_addr_t *edr_addr, app_dongle_session_manager_session_usage_t session_usage, app_dongle_session_manager_edr_session_info_t *session_info);
bt_source_srv_codec_t bt_source_srv_get_audio_codec_type(bt_source_srv_t type, const bt_addr_t *peer_address)
{
    app_dongle_session_manager_edr_session_info_t session_info = {0};
    app_dongle_session_manager_session_usage_t session_usage = (type == BT_SOURCE_SRV_TYPE_HFP) ? APP_DONGLE_SESSION_MGR_SESSION_USAGE_COMMUNICATION : APP_DONGLE_SESSION_MGR_SESSION_USAGE_MEDIA;
    if (app_dongle_session_manager_handle_edr_session_negotiation((bt_addr_t *)peer_address, session_usage, &session_info) == BT_STATUS_SUCCESS) {
        return session_info.session_type;
    }
    return (BT_SOURCE_SRV_CODEC_TYPE_CVSD | BT_SOURCE_SRV_CODEC_TYPE_MSBC | BT_SOURCE_SRV_CODEC_TYPE_SBC);
}

typedef struct {
    uint8_t     volume_value;           /* The volume value range is 0~15 */
    int32_t     gain;
} bt_source_srv_volume_mapping_t;

#define BT_SOURCE_SRV_DL_VOLUME_TABLE_MAX     (sizeof(g_dl_volume_mapping_table) / sizeof(bt_source_srv_volume_mapping_t))

#define BT_SOURCE_SRV_UL_VOLUME_TABLE_MAX     (sizeof(g_ul_volume_mapping_table) / sizeof(bt_source_srv_volume_mapping_t))

static const bt_source_srv_volume_mapping_t g_dl_volume_mapping_table[] = {
    {0,  -740000},
    {1,  -371000},
    {2,  -272000},
    {3,  -213000},
    {4,  -170000},
    {5,  -137000},
    {6,  -116000},
    {7,  -98000},
    {8,  -82000},
    {9,  -67000},
    {10, -53000},
    {11, -41000},
    {12, -30000},
    {13, -19000},
    {14, -9000},
    {15, 0},
};

static const bt_source_srv_volume_mapping_t g_ul_volume_mapping_table[] = {
    {0,  -740000},
    {1,  -371000},
    {2,  -272000},
    {3,  -213000},
    {4,  -170000},
    {5,  -137000},
    {6,  -116000},
    {7,  -98000},
    {8,  -82000},
    {9,  -67000},
    {10, -53000},
    {11, -41000},
    {12, -30000},
    {13, -19000},
    {14, -9000},
    {15, 0},
};

uint8_t bt_customer_config_get_volume_by_gain(bt_source_srv_port_t port, int32_t gain)
{
    LOG_MSGID_I(BT_APP, "[BT_CUSTOMER] source get port = %02x volume value by gain = %d", 2, port, gain);
    const bt_source_srv_volume_mapping_t *valume_mapping = NULL;
    uint32_t volume_table_max_number = 0;

    switch (port) {
        case BT_SOURCE_SRV_PORT_GAMING_SPEAKER:
        case BT_SOURCE_SRV_PORT_CHAT_SPEAKER: {
            valume_mapping = g_dl_volume_mapping_table;
            volume_table_max_number = BT_SOURCE_SRV_DL_VOLUME_TABLE_MAX;
        }
        break;
        case BT_SOURCE_SRV_PORT_MIC: {
            valume_mapping = g_ul_volume_mapping_table;
            volume_table_max_number = BT_SOURCE_SRV_UL_VOLUME_TABLE_MAX;
        }
        break;
        default:
            return BT_SOURCE_SRV_INVALID_VOLUME_VALUE;
    }

    /* max */
    if (gain == valume_mapping[volume_table_max_number - 1].gain) {
        return valume_mapping[volume_table_max_number - 1].volume_value;
    }

    for (uint32_t i = 0; i < volume_table_max_number - 1; i++) {
        /* min */
        if ((i == 0) && (valume_mapping[i].gain == gain)) {
            return valume_mapping[i].volume_value;
        }

        int32_t compare_gain = 0;
        if (gain < 0) {
            compare_gain = gain - 100;
        } else {
            compare_gain = gain + 100;
        }
        if ((compare_gain > valume_mapping[i].gain) && ((compare_gain <= valume_mapping[i + 1].gain))) {
            if (i == (volume_table_max_number - 2)) {
                return valume_mapping[i].volume_value;
            }
            return valume_mapping[i + 1].volume_value;
        }
    }
    return BT_SOURCE_SRV_INVALID_VOLUME_VALUE;
}
#endif


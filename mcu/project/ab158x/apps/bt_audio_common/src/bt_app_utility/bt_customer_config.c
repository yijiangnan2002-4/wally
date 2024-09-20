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
#include "bt_spp.h"
#include "apps_config_audio_helper.h"
#if defined(AIR_BT_FAST_PAIR_SASS_ENABLE) && defined(AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE)
#include "app_fast_pair.h"
#endif

#ifdef AIR_LE_AUDIO_ENABLE
#include "app_le_audio_aird_client.h"
#include "app_lea_service.h"

#ifdef AIR_TWS_ENABLE
extern bool app_le_audio_dhss_get_le_addr_list(uint8_t *dst);
extern bool app_le_audio_dhss_is_data_ready(void);
#endif
#endif

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
#include "app_hearing_aid_activity.h"
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */
#include "app_customer_nvkey_operation.h"

extern bool bt_sink_srv_music_is_must_play_dev_by_addr(bt_bd_addr_t *sp_addr);

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
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
    .max_connection_num = 2 + 1,
#elif defined(AIR_MULTI_POINT_ENABLE)
#if defined(AIR_3_LINK_MULTI_POINT_ENABLE)
    .max_connection_num = 3 + 1 + 1,
#else /* def AIR_3_LINK_MULTI_POINT_ENABLE */
    .max_connection_num = 2 + 1 + 1,
#endif /* ndef AIR_3_LINK_MULTI_POINT_ENABLE */
#else
    .max_connection_num = 1 + 1 + 1,
#endif  /* ndef AIR_MULTI_POINT_ENABLE */

#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
    .connection_takeover = false,
#else
    .connection_takeover = true,
#endif
    .request_role = BT_CM_ROLE_SLAVE,
    .request_role_retry_times = 0xFF,
    .page_timeout = 0x2000, /* 0x2000 * 0.625 msec = 5.12 sec */
    .power_on_reconnect_profile =
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
    BT_CM_PROFILE_SERVICE_MASK_NONE
#else
    BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS)
    | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP)
#ifdef AIR_BT_HID_ENABLE
    | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HID)
#endif
    | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK)
#ifdef MTK_IAP2_PROFILE_ENABLE
    | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_IAP2)
#endif
#endif
#ifdef MTK_GATT_OVER_BREDR_ENABLE
    | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_GATT_OVER_BREDR_AIR)
#endif
    ,
    .power_on_reconnect_duration = 0,
    .link_loss_reconnect_profile = BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS)
    | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HFP)
    | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_A2DP_SINK)
#ifdef AIR_BT_HID_ENABLE
    | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_HID)
#endif
#ifdef MTK_IAP2_PROFILE_ENABLE
    | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_IAP2)
#endif
#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
    | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL)
#endif
#ifdef MTK_GATT_OVER_BREDR_ENABLE
    | BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_GATT_OVER_BREDR_AIR)
#endif
    ,
    .link_loss_reconnect_duration = 0,
    .eir_data = {
        .data = default_eir_data,
        .length = 18
    }
}; /* The variable defines the parameters of bt_cm_config_t, refer to bt_connection_manager. */


/**
 * @brief    This function is used to set the max connection numbers.
 * @param    nums  The new max connection numbers.
 * @return   None.
 */
void set_max_connection_nums_cfg(uint8_t nums)
{
    s_cm_config.max_connection_num = nums;
    LOG_MSGID_I(BT_APP, "set the max connection nums to:%d", 1, nums);
}


/**
 * @brief    This function is used to get the bt connection configuration.
 * @return   pointer of bt_cm_config_t.
 */
const bt_cm_config_t *bt_customer_config_get_cm_config(void)
{
    bt_customer_config_get_gap_config();

    return &s_cm_config;
}

/**
 * @brief    This function is used to quickly get bt connection configuration.
 * @return   The pointer of bt_cm_config_t.
 */
const bt_cm_config_t *bt_customer_config_app_get_cm_config(void)
{
    return &s_cm_config;
}


/******************** Config the parameter of sink GAP *******************************************/
#define BT_SINK_SRV_CM_DB_NAME  "BT_Headset_Demo"                       /* Default BT name */
#if defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_LE_AUDIO_DUALMODE_ENABLE)
#define BT_SINK_SRV_CM_DB_COD 0x244404                                  /* Default BT COD(Class Of Device) refer to BT spec. Enable LE Audio bit. */
#else
#define BT_SINK_SRV_CM_DB_COD 0x240404                                  /* Default BT COD(Class Of Device) refer to BT spec. */
#endif
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
 #define AUDEARA 0x60
#define CLINICO 0x66

 #define AUDEARA_BUD "Audeara Buds"		
#define CLINICO_SOUND "Clinico Sound Buds"	

const bt_gap_config_t *bt_customer_config_get_gap_config(void)
{
    uint8_t name[BT_GAP_MAX_DEVICE_NAME_LENGTH] = {0};
    uint32_t name_length;
    uint32_t read_size = BT_GAP_MAX_DEVICE_NAME_LENGTH;
    nvkey_status_t nvkey_status = NVKEY_STATUS_ERROR;
    uint32_t len = 0;

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
#if defined(MTK_AWS_MCE_ENABLE) && !defined(AIR_SPEAKER_ENABLE)
        // To fix the BT name change after RHO
        if (BT_AWS_MCE_ROLE_PARTNER == bt_device_manager_aws_local_info_get_role()) {
            local_addr = bt_device_manager_aws_local_info_get_peer_address();
        } else
#endif
        {
            local_addr = bt_device_manager_get_local_address();
        }

#if 1	// richard for customer UI spec.
	//snprintf((char *)name, sizeof(name), "PSAP Earbuds");
	if(app_nvkey_btname_read()==AUDEARA)
	{
	snprintf((char *)name, sizeof(name), AUDEARA_BUD);
	}
	else if(app_nvkey_btname_read()==CLINICO)
	{
	snprintf((char *)name, sizeof(name), CLINICO_SOUND);

	}
	else{
	snprintf((char *)name, sizeof(name), AUDEARA_BUD);
	}
#else
        snprintf((char *)name, sizeof(name), "H_%.2X%.2X%.2X%.2X%.2X%.2X",
                 (*local_addr)[5], (*local_addr)[4], (*local_addr)[3],
                 (*local_addr)[2], (*local_addr)[1], (*local_addr)[0]);
#endif
    }
    name_length = strlen((char *)name);
    if (name_length > sizeof(g_bt_sink_srv_gap_config.device_name) - 1) {
        name_length = sizeof(g_bt_sink_srv_gap_config.device_name) - 1;
    }
    memcpy(g_bt_sink_srv_gap_config.device_name, name, name_length);
    g_bt_sink_srv_gap_config.device_name[name_length] = '\0';
    LOG_I(BT_APP, "[BT_CM] device name:%s", g_bt_sink_srv_gap_config.device_name);

    default_eir_data[18] = name_length + 1;
    default_eir_data[19] = 0x09;
    memcpy((default_eir_data + 20), g_bt_sink_srv_gap_config.device_name, name_length);
    len = 18 + 2 + name_length;

#if (defined(AIR_TWS_ENABLE) && defined(AIR_LE_AUDIO_ENABLE))
#if (defined(AIR_LE_AUDIO_DUALMODE_ENABLE))
    if ((app_lea_service_is_enable_dual_mode() == false) &&
        (app_le_audio_dhss_is_data_ready()))
     {
        default_eir_data[len] = 0x12;
        default_eir_data[len + 1] = 0xFF;
        default_eir_data[len + 2] = 0x46;
        default_eir_data[len + 3] = 0x00;
        app_le_audio_dhss_get_le_addr_list(&default_eir_data[len + 4]);
        default_eir_data[len + 18] = 0x00; /* 0 means TWS02 project */
        len += (0x12 + 1);
    }
#else
    if (app_le_audio_dhss_is_data_ready()) {
        default_eir_data[len] = 0x12;
        default_eir_data[len + 1] = 0xFF;
        default_eir_data[len + 2] = 0x46;
        default_eir_data[len + 3] = 0x00;
        app_le_audio_dhss_get_le_addr_list(&default_eir_data[len + 4]);
        default_eir_data[len + 18] = 0x00; /* 0 means TWS02 project */
        len += (0x12 + 1);
    }
#endif
#endif
#ifdef MTK_IAP2_PROFILE_ENABLE
    default_eir_data[len] = 0x11;
    default_eir_data[len + 1] = 0x07;
    uint8_t iap2_uuid[16] = {0xff, 0xca, 0xca, 0xde, 0xaf, 0xde, 0xca, 0xde, 0xde, 0xfa, 0xca, 0xde, 0x00, 0x00, 0x00, 0x00};
    memcpy(default_eir_data + len + 2, iap2_uuid, 16);
    len += (0x11 + 1);
#endif /* MTK_IAP2_PROFILE_ENABLE */
#ifdef AIR_BT_INTEL_EVO_ENABLE
    default_eir_data[len] = 0x09;
    default_eir_data[len + 1] = 0x10;
    uint16_t device_id_data[4] = {0x0000,0x0094,0x0004,0x0100};
    memcpy(default_eir_data + len + 2, device_id_data, 8);
#endif
    s_cm_config.eir_data.length = len;
    return &g_bt_sink_srv_gap_config;
}

#ifdef AIR_SPEAKER_ENABLE
#define BT_SINK_SRV_MODULE_INTERNAL_RANGE               (0x0A) /**<Internal used module ID range, it must not be changed by user.*/
#define BT_SINK_SRV_MODULE_INTERNAL_START               (0)    /**<Internal used module ID beginning, it must not be changed by user.*/
#define BT_SINK_SRV_MODULE_INTERNAL_CM                  (BT_SINK_SRV_MODULE_INTERNAL_START)          /**<Module ID: 0x00. Defined for the sink conmgr.*/
#define BT_SINK_SRV_MODULE_INTERNAL_MUSIC               (BT_SINK_SRV_MODULE_INTERNAL_START + 1)   /**<Module ID: 0x01. Defined for the sink music.*/
#define BT_SINK_SRV_MODULE_INTERNAL_CALL                (BT_SINK_SRV_MODULE_INTERNAL_START + 2)   /**<Module ID: 0x02. Defined for the sink call.*/
#define BT_SINK_SRV_MODULE_INTERNAL_VP_SYNC             (BT_SINK_SRV_MODULE_INTERNAL_START + 3) /**<Module ID: 0x03. Defined for the voice prompt sync. */
#define BT_SINK_SRV_MODULE_INTERNAL_AMI                 (BT_SINK_SRV_MODULE_INTERNAL_START + 4)     /**<Module ID: 0x04. Defined for audio manager interface.*/
#define BT_SINK_SRV_MODULE_GSOUND_AWS                   (BT_SINK_SRV_MODULE_INTERNAL_START + 5)     /**<Module ID: 0x05. Defined for Gsound Library.*/

#define BT_SINK_SRV_MODULE_INTERNAL_UT_TEST             (BT_SINK_SRV_MODULE_INTERNAL_START + 6) /**<Module ID: 0x06. Defined for unit test*/
#define BT_SINK_SRV_MODULE_INTERNAL_USED_NUM            BT_SINK_SRV_MODULE_INTERNAL_UT_TEST + 1  /**<Defined for the used module number for sink service*/
#define BT_SINK_SRV_MODULE_INTERNAL_END                 (BT_SINK_SRV_MODULE_INTERNAL_START + BT_SINK_SRV_MODULE_INTERNAL_RANGE - 1)
/**<Internal used module ID end, it must not be changed by user.*/
#define BT_SINK_SRV_MODULE_CUSTOM_START                (BT_SINK_SRV_MODULE_INTERNAL_END + 1)     /**<Module ID: 0x0A. Custom used module ID beginning, user can define it.*/
#define BT_SINK_SRV_MODULE_APP_FOTA                    (BT_SINK_SRV_MODULE_CUSTOM_START)         /**<Defined for fota application*/
#define BT_SINK_SRV_MODULE_APP_KEY                     (BT_SINK_SRV_MODULE_CUSTOM_START + 1)     /**<Defined for key application*/
#define BT_SINK_SRV_MODULE_APP_LED                     (BT_SINK_SRV_MODULE_CUSTOM_START + 2)     /**<Defined for led application*/
#define BT_SINK_SRV_MODULE_APP_CCASE                   (BT_SINK_SRV_MODULE_CUSTOM_START + 3)     /**<Defined for ccase application*/
#define BT_SINK_SRV_MODULE_APP_BLE                     (BT_SINK_SRV_MODULE_CUSTOM_START + 5)     /**<Module ID: 0x0F. Defined for ble application*/
#define BT_SINK_SRV_MODULE_CUSTOM_END                  (0xFF)                                    /**<Custom used module ID end*/

const static bt_aws_mce_report_module_map_t module_mapping_list[BT_AWS_MCE_REPORT_MODULE_MAX] = {
    /*-------------------------------- module id for middleware ----------------------------------*/
    {BT_AWS_MCE_REPORT_MODULE_CM,         BT_SINK_SRV_MODULE_INTERNAL_CM     },//30
    {BT_AWS_MCE_REPORT_MODULE_SINK_MUSIC, BT_SINK_SRV_MODULE_INTERNAL_MUSIC  },//31
    {BT_AWS_MCE_REPORT_MODULE_SINK_CALL,  BT_SINK_SRV_MODULE_INTERNAL_CALL   },//32
    {BT_AWS_MCE_REPORT_MODULE_BT_AIR,     BT_AWS_MCE_REPORT_INVALID_MODULE_ID},//33
    {BT_AWS_MCE_REPORT_MODULE_FOTA,       BT_SINK_SRV_MODULE_APP_FOTA        },//34
    {BT_AWS_MCE_REPORT_MODULE_TEST,       BT_SINK_SRV_MODULE_INTERNAL_UT_TEST},//35
    {BT_AWS_MCE_REPORT_MODULE_RELAY_CMD,  BT_AWS_MCE_REPORT_INVALID_MODULE_ID},//36
    {BT_AWS_MCE_REPORT_MODULE_HFP_AVC,    BT_AWS_MCE_REPORT_INVALID_MODULE_ID},//37
    {BT_AWS_MCE_REPORT_MODULE_GSOUND,     BT_SINK_SRV_MODULE_GSOUND_AWS      },//38
    {BT_AWS_MCE_REPORT_MODULE_DM,         BT_AWS_MCE_REPORT_INVALID_MODULE_ID},//39
    {BT_AWS_MCE_REPORT_MODULE_ANC,        BT_AWS_MCE_REPORT_INVALID_MODULE_ID},//3A
    {BT_AWS_MCE_REPORT_MODULE_PEQ,        BT_AWS_MCE_REPORT_INVALID_MODULE_ID},//3B
    {BT_AWS_MCE_REPORT_INVALID_MODULE_ID, BT_AWS_MCE_REPORT_INVALID_MODULE_ID},//3C
    {BT_AWS_MCE_REPORT_INVALID_MODULE_ID, BT_AWS_MCE_REPORT_INVALID_MODULE_ID},//3D
    {BT_AWS_MCE_REPORT_INVALID_MODULE_ID, BT_AWS_MCE_REPORT_INVALID_MODULE_ID},//3E
    {BT_AWS_MCE_REPORT_INVALID_MODULE_ID, BT_AWS_MCE_REPORT_INVALID_MODULE_ID},//3F
    /*---------------------------------- module id for app ---------------------------------------*/
    {BT_AWS_MCE_REPORT_MODULE_VP,         BT_SINK_SRV_MODULE_INTERNAL_VP_SYNC},//40
    {BT_AWS_MCE_REPORT_MODULE_LED,        BT_SINK_SRV_MODULE_APP_LED         },//41
    {BT_AWS_MCE_REPORT_MODULE_BATTERY,    BT_AWS_MCE_REPORT_INVALID_MODULE_ID},//42
    {BT_AWS_MCE_REPORT_MODULE_APP_ACTION, BT_AWS_MCE_REPORT_INVALID_MODULE_ID},//43
    {BT_AWS_MCE_REPORT_MODULE_BLE_APP,    BT_SINK_SRV_MODULE_APP_BLE         },//44
    {BT_AWS_MCE_REPORT_MODULE_FAST_PAIR,  BT_AWS_MCE_REPORT_INVALID_MODULE_ID},//45
    {BT_AWS_MCE_REPORT_MODULE_SMCHARGER,  BT_SINK_SRV_MODULE_APP_CCASE       },//46
    {BT_AWS_MCE_REPORT_MODULE_IN_EAR,     BT_AWS_MCE_REPORT_INVALID_MODULE_ID},//47
    {BT_AWS_MCE_REPORT_MODULE_AP,         BT_AWS_MCE_REPORT_INVALID_MODULE_ID},//48
    {BT_AWS_MCE_REPORT_INVALID_MODULE_ID, BT_AWS_MCE_REPORT_INVALID_MODULE_ID},//49
    {BT_AWS_MCE_REPORT_INVALID_MODULE_ID, BT_AWS_MCE_REPORT_INVALID_MODULE_ID},//4A
    {BT_AWS_MCE_REPORT_MODULE_VA_XIAOWEI, BT_AWS_MCE_REPORT_INVALID_MODULE_ID},//4B
    {BT_AWS_MCE_REPORT_MODULE_XIAOAI,     BT_AWS_MCE_REPORT_INVALID_MODULE_ID},//4C
    {BT_AWS_MCE_REPORT_INVALID_MODULE_ID, BT_AWS_MCE_REPORT_INVALID_MODULE_ID},//4D
    {BT_AWS_MCE_REPORT_INVALID_MODULE_ID, BT_AWS_MCE_REPORT_INVALID_MODULE_ID},//4E
    {BT_AWS_MCE_REPORT_INVALID_MODULE_ID, BT_AWS_MCE_REPORT_INVALID_MODULE_ID} //4F
};

uint8_t bt_aws_mce_report_get_mapped_module_id(bt_aws_mce_report_module_id_map_t type, uint8_t module_id)
{
    uint32_t i;
    uint8_t ret = BT_AWS_MCE_REPORT_INVALID_MODULE_ID;
    if (BT_AWS_MCE_REPORT_MODULE_ID_MAP_TYPE_NEW_TO_OLD == type) {
        for (i = 0; i < BT_AWS_MCE_REPORT_MODULE_MAX; i++) {
            if (module_id == module_mapping_list[i].new_module_id) {
                ret = module_mapping_list[i].old_module_id;
                break;
            }
        }
    } else if (BT_AWS_MCE_REPORT_MODULE_ID_MAP_TYPE_OLD_TO_NEW == type) {
        for (i = 0; i < BT_AWS_MCE_REPORT_MODULE_MAX; i++) {
            if (module_id == module_mapping_list[i].old_module_id) {
                ret = module_mapping_list[i].new_module_id;
                break;
            }
        }
    }

    return ret;
}

#endif

/**
 * @brief    This function is used to get the BLE name.
 * @param[out] ble_name  The buffer of BLE name, it will be filled in the fuction.
 * @return   None.
 */
void bt_customer_config_get_ble_device_name(char ble_name[BT_GAP_LE_MAX_DEVICE_NAME_LENGTH])
{
    nvkey_status_t nvkey_status = NVKEY_STATUS_ERROR;
    const bt_gap_config_t *config = bt_customer_config_get_gap_config();

#ifndef AIR_GSOUND_ENABLE
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

    param->support_features = (bt_hfp_init_support_feature_t)(
                                  BT_HFP_INIT_SUPPORT_3_WAY |
                                  BT_HFP_INIT_SUPPORT_CODEC_NEG |
                                  BT_HFP_INIT_SUPPORT_ENHANCED_CALL_STATUS |
                                  BT_HFP_INIT_SUPPORT_ENHANCED_CALL_CONTROL |
                                  BT_HFP_INIT_SUPPORT_CLI_PRESENTATION |
                                  BT_HFP_INIT_SUPPORT_HF_INDICATORS);

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
#if (defined(AIR_SWIFT_PAIR_ENABLE) && (defined(AIR_CUST_PAIR_ENABLE) || defined(APP_SWIFT_PAIR_LE_EDR_SECURE_MODE)))
    bt_init_feature_mask_t config_feature = BT_INIT_FEATURE_MASK_DISABLE_SNIFF_SUB_MODE | BT_INIT_FEATURE_MASK_BT_5_0 | BT_INIT_FEATURE_MASK_SECURITY_CONNECTION;
#else
#if (defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_LE_AUDIO_DUALMODE_ENABLE))
    bt_init_feature_mask_t config_feature;

    if (app_lea_service_is_enable_dual_mode()) {
        config_feature = BT_INIT_FEATURE_MASK_DISABLE_SNIFF_SUB_MODE | BT_INIT_FEATURE_MASK_BT_5_0 | BT_INIT_FEATURE_MASK_SECURITY_CONNECTION;
    } else {
        config_feature = BT_INIT_FEATURE_MASK_DISABLE_SNIFF_SUB_MODE | BT_INIT_FEATURE_MASK_BT_5_0;
    }
#else //#if (defined(AIR_LE_AUDIO_ENABLE) && defined(AIR_LE_AUDIO_DUALMODE_ENABLE))
    bt_init_feature_mask_t config_feature = BT_INIT_FEATURE_MASK_DISABLE_SNIFF_SUB_MODE | BT_INIT_FEATURE_MASK_BT_5_0;
#endif
#endif

    uint8_t enable_3M_config = 1;
    uint32_t config_size = sizeof(uint8_t);
    nvkey_status_t ret = nvkey_read_data(NVID_APP_A2DP_3M_CONFIG, (uint8_t *)(&enable_3M_config), &config_size);
    if (!enable_3M_config) {
        config_feature |= BT_INIT_FEATURE_MASK_DISABLE_3M;
        bt_a2dp_set_mtu_size(BT_A2DP_DISABLE_3M_MTU_SIZE);
    }
    LOG_MSGID_I(BT_APP, "[BT_CUSTOMER] ret:%d, config_feature:0x%04x", 2, ret, config_feature);
    return config_feature;
}
/*************************************************************************************************/

bool bt_a2dp_get_media_mtu_size(const bt_a2dp_codec_capability_t *codec, uint32_t *mtu_size)
{
    uint8_t enable_3M_config = 1;
    uint32_t config_size = sizeof(uint8_t);
    bool mtu_config_ret = false;
    nvkey_status_t ret = nvkey_read_data(NVID_APP_A2DP_3M_CONFIG, (uint8_t *)(&enable_3M_config), &config_size);

    if (!enable_3M_config) {
        *mtu_size = BT_A2DP_DISABLE_3M_MTU_SIZE;
        mtu_config_ret = true;
        if(codec->type == 0xff){
            *mtu_size = 679;
        }
    }


    LOG_MSGID_I(BT_APP, "[BT_CUSTOMER] mtu_config_ret:%d, *mtu_size:%d", 2, ret, *mtu_size);
    return mtu_config_ret;
}

/***************************** Get max RHO data length ***************************************/
uint16_t bt_role_handover_get_max_length(void)
{
    return 1008; /* WARNING: This value depends on platform, DO NOT CHANGE! */
}
/******************************************************************************************/

#if defined(AIR_BT_ULTRA_LOW_LATENCY_ENABLE)
bool bt_a2dp_allow_connection_check(const bt_bd_addr_t *address)
{
#ifndef AIR_BT_ULTRA_LOW_LATENCY_A2DP_STANDBY_ENABLE
    if (0 < bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL), NULL, 0)) {
        LOG_MSGID_I(BT_APP, "[BT_CUSTOMER] bt_a2dp_allow_connection_check false due to ULL profile connected", 0);
        return false;
    } else {
        LOG_MSGID_I(BT_APP, "[BT_CUSTOMER] bt_a2dp_allow_connection_check true", 0);
    }
#endif
    return true;
}

bt_status_t bt_spp_check_connect_ind(bt_spp_connect_ind_t *connect_ind)
{
    bt_bd_addr_t addr;
    if (0 < bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_CUSTOMIZED_ULL), &addr, 1)) {
        /* spp air server ID */
        if (21 == connect_ind->local_server_id && !memcmp(&addr, connect_ind->address, sizeof(bt_bd_addr_t))) {
            return BT_STATUS_SUCCESS;
        }
        LOG_MSGID_I(BT_APP, "[BT_CUSTOMER] bt_spp_check_connect_ind false due to ULL profile connected", 0);
        return BT_STATUS_FAIL;
    } else {
        LOG_MSGID_I(BT_APP, "[BT_CUSTOMER] bt_spp_check_connect_ind true", 0);
    }
    return BT_STATUS_SUCCESS;
}
#endif
#ifdef MTK_AWS_MCE_ENABLE
#define BT_SINK_STATE_MAX_INDEX 25
typedef struct {

    uint16_t le_state;

    uint16_t edr_state;

    uint16_t map_state;
} bt_sink_srv_state_map;
static const bt_sink_srv_state_map g_sink_state_tab[BT_SINK_STATE_MAX_INDEX] = {
#if 0
    {(BT_SINK_SRV_STATE_INCOMING | BT_SINK_SRV_STATE_OUTGOING | BT_SINK_SRV_STATE_ACTIVE | BT_SINK_SRV_STATE_HELD_ACTIVE), BT_SINK_SRV_STATE_STREAMING, BT_SINK_STATE_NOT_CHANGE}
#endif
    { BT_SINK_SRV_STATE_INCOMING, BT_SINK_SRV_STATE_STREAMING, BT_SINK_SRV_STATE_INCOMING},
    { BT_SINK_SRV_STATE_INCOMING, BT_SINK_SRV_STATE_INCOMING, 0xFF/*BT_SINK_STATE_INVAILD*/},
    { BT_SINK_SRV_STATE_INCOMING, (BT_SINK_SRV_STATE_OUTGOING | BT_SINK_SRV_STATE_ACTIVE | BT_SINK_SRV_STATE_HELD_ACTIVE), BT_SINK_SRV_STATE_TWC_INCOMING},
    { BT_SINK_SRV_STATE_OUTGOING, BT_SINK_SRV_STATE_STREAMING, BT_SINK_SRV_STATE_OUTGOING},
    { BT_SINK_SRV_STATE_OUTGOING, BT_SINK_SRV_STATE_INCOMING, BT_SINK_SRV_STATE_TWC_INCOMING},
    { BT_SINK_SRV_STATE_OUTGOING, BT_SINK_SRV_STATE_OUTGOING, 0xFF/*BT_SINK_STATE_INVAILD*/},
    { BT_SINK_SRV_STATE_OUTGOING, BT_SINK_SRV_STATE_ACTIVE,  BT_SINK_SRV_STATE_TWC_OUTGOING},
    { BT_SINK_SRV_STATE_OUTGOING, BT_SINK_SRV_STATE_HELD_REMAINING, BT_SINK_SRV_STATE_TWC_OUTGOING},
    { BT_SINK_SRV_STATE_ACTIVE, BT_SINK_SRV_STATE_STREAMING, BT_SINK_SRV_STATE_ACTIVE},
    { BT_SINK_SRV_STATE_ACTIVE, BT_SINK_SRV_STATE_INCOMING, BT_SINK_SRV_STATE_TWC_INCOMING},
    { BT_SINK_SRV_STATE_ACTIVE, BT_SINK_SRV_STATE_OUTGOING, BT_SINK_SRV_STATE_TWC_OUTGOING},
    { BT_SINK_SRV_STATE_ACTIVE, BT_SINK_SRV_STATE_ACTIVE, 0xFF/*BT_SINK_STATE_INVAILD*/},
    { BT_SINK_SRV_STATE_ACTIVE, BT_SINK_SRV_STATE_HELD_REMAINING, BT_SINK_SRV_STATE_HELD_ACTIVE},
    { BT_SINK_SRV_STATE_HELD_REMAINING, BT_SINK_SRV_STATE_STREAMING, BT_SINK_SRV_STATE_HELD_REMAINING},
    { BT_SINK_SRV_STATE_HELD_REMAINING, BT_SINK_SRV_STATE_INCOMING, BT_SINK_SRV_STATE_TWC_INCOMING},
    { BT_SINK_SRV_STATE_HELD_REMAINING, BT_SINK_SRV_STATE_OUTGOING, BT_SINK_SRV_STATE_TWC_OUTGOING},
    { BT_SINK_SRV_STATE_HELD_REMAINING, BT_SINK_SRV_STATE_ACTIVE, BT_SINK_SRV_STATE_HELD_ACTIVE},
    { BT_SINK_SRV_STATE_HELD_REMAINING, BT_SINK_SRV_STATE_HELD_REMAINING, 0xFF/*BT_SINK_STATE_INVAILD*/}
};

uint16_t bt_sink_srv_get_state_map_table(uint16_t le_state, uint16_t edr_state)
{
    uint8_t i = 0;
    LOG_MSGID_I(BT_APP, "[BT_state_map], i: %x, state:%x", 2, le_state, edr_state);
    for (; i < BT_SINK_STATE_MAX_INDEX; i++) {
        LOG_MSGID_I(BT_APP, "[BT_state_map]-----> i: %x, map_state:%x,le_state:%x", 3, i, g_sink_state_tab[i].le_state, g_sink_state_tab[i].edr_state);
        if ((g_sink_state_tab[i].le_state & le_state) && (g_sink_state_tab[i].edr_state & edr_state)) {
            break;
        }
    }
    if (i == BT_SINK_STATE_MAX_INDEX) {
        return 0xFF/*BT_SINK_STATE_INVAILD*/;
    }
    LOG_MSGID_I(BT_APP, "[BT_state_map], i: %x, state:%x", 2, i, g_sink_state_tab[i].map_state);
    return g_sink_state_tab[i].map_state;
}
#endif
bool bt_sink_srv_aird_support_call_mode(bt_handle_t handle)
{
#ifdef AIR_LE_AUDIO_ENABLE
    return app_le_audio_aird_client_is_support_hid_call(handle);
#else
    return FALSE;
#endif
}

bt_sink_srv_allow_result_t bt_customer_config_allow_play(bt_sink_srv_device_state_t *current_device, bt_sink_srv_device_state_t *coming_device)
{
    bt_sink_srv_allow_result_t result = BT_SINK_SRV_ALLOW_RESULT_BYPASS;
    LOG_MSGID_I(BT_APP, "[BT_CUSTOMER] bt_customer_config_allow_play", 0);

    if (NULL == current_device || NULL == coming_device) {
        return BT_SINK_SRV_ALLOW_RESULT_BYPASS;
    }

    if (current_device->type == coming_device->type &&
        0 == memcmp(current_device->address, coming_device->address, sizeof(bt_bd_addr_t))) {
        return BT_SINK_SRV_ALLOW_RESULT_BYPASS;
    }

#if defined(AIR_BT_FAST_PAIR_SASS_ENABLE) && defined(AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE)
    result = app_fast_pair_multi_link_get_allow_play(current_device, coming_device);
#endif

    if (BT_SINK_SRV_ALLOW_RESULT_BYPASS == result) {
        if (BT_SINK_SRV_STATE_STREAMING == current_device->music_state &&
            BT_SINK_SRV_STATE_STREAMING == coming_device->music_state
#if !defined(AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE)
            && bt_sink_srv_music_is_must_play_dev_by_addr(&coming_device->address)
#endif
        ) {
            result = BT_SINK_SRV_ALLOW_RESULT_ALLOW;
        }

        if (current_device->type != coming_device->type &&
            0 == memcmp(current_device->address, coming_device->address, sizeof(bt_bd_addr_t))) {
            /* For dual mode, interruption is allowed between same address. */
            if (BT_SINK_SRV_STATE_NONE != current_device->call_state &&
                BT_SINK_SRV_STATE_NONE != coming_device->call_state &&
                BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED != coming_device->sco_state) {
                result = BT_SINK_SRV_ALLOW_RESULT_ALLOW;
            }
        }
    }

    return result;
}

bt_status_t bt_sink_srv_get_config(bt_sink_srv_get_config_t type, bt_sink_srv_get_config_param_t *param, bt_sink_srv_config_t *config)
{
    switch (type) {
        case BT_SINK_SRV_GET_REJECT_CONFIG: {
            config->reject_config.reject_operation = BT_SINK_SRV_INTERRUPT_OPERATION_NONE;
            config->reject_config.will_reject_resume = false;

#ifdef AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE
            /* If 2nd music tries to interrupt 1st call, 2nd music should pause and will not resume. */
            if (BT_SINK_SRV_STATE_STREAMING == param->reject_config_param.reject_device_state.music_state) {
                config->reject_config.reject_operation |= BT_SINK_SRV_INTERRUPT_OPERATION_PAUSE_MUSIC;
                config->reject_config.will_reject_resume = false;
            }

            if (BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED == param->reject_config_param.current_device_state.sco_state &&
                BT_SINK_SRV_STATE_ACTIVE == param->reject_config_param.reject_device_state.call_state &&
                BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED != param->reject_config_param.reject_device_state.sco_state) {
                /* For Teams join, do nothing. */
                break;
            }

            /* If 2nd call  tries to interrupt 1st call, 2nd call should hold. */
            if ((BT_SINK_SRV_STATE_NONE != param->reject_config_param.reject_device_state.call_state ||
                BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED != param->reject_config_param.reject_device_state.sco_state) &&
                0 != memcmp(param->reject_config_param.current_device_state.address, param->reject_config_param.reject_device_state.address, sizeof(bt_bd_addr_t))) {
                config->reject_config.reject_operation |= BT_SINK_SRV_INTERRUPT_OPERATION_HOLD_CALL;
                config->reject_config.will_reject_resume = true;
                if (BT_SINK_SRV_STATE_ACTIVE == param->reject_config_param.reject_device_state.call_state) {
                    config->reject_config.reject_operation |= BT_SINK_SRV_INTERRUPT_OPERATION_TRANSFER_CALL_AUDIO;
                }
            }
#else
            /* If 2nd media tries to interrupt 1st media, 2nd media should resume after 1st paused. */
            /* If 2nd media tries to interrupt 1st call,  2nd media should resume after 1st call ended. */
            if (BT_SINK_SRV_STATE_STREAMING == param->reject_config_param.reject_device_state.music_state) {
                config->reject_config.will_reject_resume = true;
            }

            /* If 2nd call  tries to interrupt 1st call,  2nd call audio should transfer to SP. */
            if ((BT_SINK_SRV_STATE_STREAMING < param->reject_config_param.reject_device_state.call_state ||
                BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED != param->reject_config_param.reject_device_state.sco_state) &&
                0 != memcmp(param->reject_config_param.current_device_state.address, param->reject_config_param.reject_device_state.address, sizeof(bt_bd_addr_t))) {
                config->reject_config.reject_operation |= BT_SINK_SRV_INTERRUPT_OPERATION_TRANSFER_CALL_AUDIO;
                if (BT_SINK_SRV_DEVICE_LE == param->reject_config_param.reject_device_state.type) {
                    config->reject_config.will_reject_resume = true; /* LE should resume because cannnot transfer call audio. */
                }
            }
#endif

            break;
        }

        case BT_SINK_SRV_GET_SUSPEND_CONFIG: {
            config->suspend_config.suspend_operation = BT_SINK_SRV_INTERRUPT_OPERATION_NONE;
            config->suspend_config.will_suspend_resume = false;
            config->suspend_config.suspend_resume_timeout = 0;

#ifdef AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE
            /* If 2nd music tries to interrupt 1st music, 1st music should pause and will not resume. */
            /* If 2nd call  tries to interrupt 1st music, 1st music should pause and will resume. */
            if (BT_SINK_SRV_STATE_STREAMING == param->suspend_config_param.suspend_device_state.music_state) {
                config->suspend_config.suspend_operation |= BT_SINK_SRV_INTERRUPT_OPERATION_PAUSE_MUSIC;
                if (BT_SINK_SRV_STATE_NONE != param->suspend_config_param.coming_device_state.call_state ||
                    BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED != param->suspend_config_param.coming_device_state.sco_state) {
                    config->suspend_config.will_suspend_resume = true;
                } else {
                    config->suspend_config.will_suspend_resume = false;
                }
            }

            /* If 2nd call  tries to interrupt 1st call, 1st call  should hold and will resume. */
            if ((BT_SINK_SRV_STATE_NONE != param->suspend_config_param.suspend_device_state.call_state ||
                BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED != param->suspend_config_param.suspend_device_state.sco_state) &&
                0 != memcmp(param->suspend_config_param.suspend_device_state.address, param->suspend_config_param.coming_device_state.address, sizeof(bt_bd_addr_t))) {
                config->suspend_config.suspend_operation |= BT_SINK_SRV_INTERRUPT_OPERATION_HOLD_CALL;
                config->suspend_config.will_suspend_resume = true;
                if (BT_SINK_SRV_STATE_ACTIVE == param->suspend_config_param.suspend_device_state.call_state) {
                    config->suspend_config.suspend_operation |= BT_SINK_SRV_INTERRUPT_OPERATION_TRANSFER_CALL_AUDIO;
                }
            }
#else
            bt_bd_addr_t null_address = {0};

            /* If 2nd call try to interrupt 1st media, media should resume after call ended. */
            if (BT_SINK_SRV_STATE_STREAMING == param->suspend_config_param.suspend_device_state.music_state &&
                (0 == memcmp(param->suspend_config_param.suspend_device_state.address, null_address, sizeof(bt_bd_addr_t)) ||
                0 != memcmp(param->suspend_config_param.suspend_device_state.address, param->suspend_config_param.coming_device_state.address, sizeof(bt_bd_addr_t)))) {
                config->suspend_config.will_suspend_resume = true;
                config->suspend_config.suspend_operation |= BT_SINK_SRV_INTERRUPT_OPERATION_PAUSE_MUSIC;
            }

            /* If other try to interrupt 1st call, call should resume. */
            if (BT_SINK_SRV_STATE_NONE != param->suspend_config_param.suspend_device_state.call_state ||
                BT_SINK_SRV_SCO_CONNECTION_STATE_DISCONNECTED != param->suspend_config_param.suspend_device_state.sco_state) {
                config->suspend_config.will_suspend_resume = true;
            }
#endif

            break;
        }

        case BT_SINK_SRV_GET_RESUME_CONFIG: {
            config->resume_config.resume_operation = BT_SINK_SRV_INTERRUPT_OPERATION_NONE;

#if (defined AIR_BT_SINK_SRV_CUSTOMIZED_ENABLE) || (defined AIR_MS_TEAMS_UE_ENABLE)
            /* If music has been rejected or suspended, it should play while resume. */
            /* If call  has been rejected or suspended, it should unhold while resume. */
            if (BT_SINK_SRV_STATE_STREAMING == param->resume_config_param.resume_device_state.music_state) {
                config->resume_config.resume_operation |= BT_SINK_SRV_INTERRUPT_OPERATION_PLAY_MUSIC;
            } else {
                config->resume_config.resume_operation |= BT_SINK_SRV_INTERRUPT_OPERATION_UNHOLD_CALL;
                config->resume_config.resume_operation |= BT_SINK_SRV_INTERRUPT_OPERATION_TRANSFER_CALL_AUDIO;
            }
#else
            /* If 2nd call  has been rejected, do nothing while it can be resumed. */
            /* If 1st media has beed suspended, media should play after call ended. */
            if (BT_SINK_SRV_STATE_STREAMING == param->resume_config_param.resume_device_state.music_state) {
                config->resume_config.resume_operation |= BT_SINK_SRV_INTERRUPT_OPERATION_PLAY_MUSIC;
            }
#endif

            break;
        }

        default: {
            break;
        }
    }

    return BT_STATUS_SUCCESS;
}

bool bt_sink_srv_call_get_sidetone_enable_config(void)
{
    bool enable = true;
    const app_config_audio_helper_sidetone_data_t *data = apps_config_audio_helper_get_sidetone_data();

    if (data != NULL) {
        enable = data->enable && !apps_config_audio_helper_get_sidetone_temporary_disabled();
    }

#if defined(AIR_HEARING_AID_ENABLE) || defined(AIR_HEARTHROUGH_PSAP_ENABLE)
    enable = app_hearing_aid_is_ready_to_enable_side_tone();
#endif /* AIR_HEARING_AID_ENABLE || AIR_HEARTHROUGH_PSAP_ENABLE */

    LOG_MSGID_I(BT_APP, "[BT_CUSTOMER] get sidetone enable config, %x", 1, enable);
    return enable;
}


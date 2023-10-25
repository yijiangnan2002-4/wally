/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
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
 * File:iAP2_config.c
 *
 * Description: The file is to set IAP2 configuration.
 *
 */

#ifdef MTK_IAP2_PROFILE_ENABLE

#include "iAP2.h"
#include "iAP2_config.h"
#include "bt_type.h"

#include "bt_connection_manager.h"
#include "bt_device_manager.h"
#include "bt_customer_config.h"

extern uint8_t const gIAP2_Iden_Param[IAP2_PARAM_IDEN_MAX_SIZE];

/**
*  @brief IAP2 SYNC Payload, and you can modify it according to your request.
*/
iap2_sync_payload_t gIAP2_Sync_Payload = {
    1,      /* Link_version. */
    4,      /* Max_num_out_packets. */
    800,    /* Max_packet_length. */
    10000,    /* Retransmission_timeout, unit: ms. */
    5000,    /* Cumulative_ack_timeout, unit: ms. */
    3,      /* Max_num_retransmissions. */
    4,      /* Max_num_cumulative_acks. */
    {   /* Identifier, type, version. */
        { IAP2_SESSION_ID_CTRL, IAP2_SESSION_TYPE_CONTROL, 0x01 },
        { IAP2_SESSION_ID_EXTA, IAP2_SESSION_TYPE_EXTERNAL_ACCESSORY, 0x01 },
    }
};

/* It is recommended not to change the following part. */
uint8_t const *gIAP2_Iden_Param_pointer =
    gIAP2_Iden_Param;  /* A pointer that point to the iap2 idetification paramters */

/*static char device_name[15] = {"Airoha_iAP2"};*/  /* IAP2 device name. */

iap2_sync_payload_t *iap2_get_sync_payload(void)
{
    return (iap2_sync_payload_t *)&gIAP2_Sync_Payload;
}

uint8_t const *iap2_get_identification_parameters(void)
{
    return (uint8_t const *)gIAP2_Iden_Param_pointer;
}

uint8_t *iap2_get_bt_address(void)
{
    return (uint8_t *)bt_device_manager_get_local_address();
}

uint8_t *iap2_get_bt_local_name(void)
{
    /* return (uint8_t *)device_name; */
    const bt_gap_config_t *config = bt_customer_config_get_gap_config();
    if (config) {
        return (uint8_t *)(config->device_name);
    } else {
        return NULL;
    }
}

static const uint8_t serial_number[10] = {"20180827"};  /* IAP2 device serial number. */
uint8_t *iap2_get_serial_number(void)
{
    return (uint8_t *)serial_number;
}

#define IAP2_INFOS_MAX_LENGTH (16)
static const uint8_t iap2_model_name[IAP2_INFOS_MAX_LENGTH] = {"BT_iAP2"}; /* IAP2 model name. */

uint8_t *iap2_get_model_name(void)
{
    return (uint8_t *)iap2_model_name;
}

static const uint8_t iap2_fw_version[IAP2_INFOS_MAX_LENGTH] = {"V1.0"};  /* IAP2 firmware version. */
uint8_t *iap2_get_fw_version(void)
{
    return (uint8_t *)iap2_fw_version;
}

static const uint8_t iap2_hw_version[IAP2_INFOS_MAX_LENGTH] = {"V1.0"};  /* IAP2 hardware version. */
uint8_t *iap2_get_hw_version(void)
{
    return (uint8_t *)iap2_hw_version;
}

uint8_t iap2_get_tx_queue_max_length(void)
{
    return 10;/* Defined by user. */
}

static const uint8_t iap2_team_id[IAP2_INFOS_MAX_LENGTH] = {"44585VMYH5"};  /* IAP2 team ID, for APP match. */
uint8_t *iap2_get_team_id(void)
{
    return (uint8_t *)iap2_team_id;
}

#endif /*MTK_IAP2_PROFILE_ENABLE*/


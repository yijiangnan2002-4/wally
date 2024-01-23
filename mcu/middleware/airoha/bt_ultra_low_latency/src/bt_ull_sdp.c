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

#include "bt_type.h"
#include "bt_sdp.h"
#include "bt_spp.h"
#include "bt_callback_manager.h"
#include "bt_ull_utility.h"
#include "bt_utils.h"



/**< the ULL client UUID.*/
static const uint8_t bt_ull_client_uuid[16] = {BT_ULL_ROLE_CLIENT_UUID};
/****************************************************************************
 *
 * ROMable data
 * SPP service record
 ****************************************************************************/
static const uint8_t bt_spp_ull_service_class_id[] = {
    BT_SPP_SDP_ATTRIBUTE_UUID_LENGTH,
    BT_SPP_SDP_ATTRIBUTE_UUID(BT_ULL_ROLE_CLIENT_UUID)
};

static const uint8_t bt_spp_ull_protocol_descriptor_list[] = {
    BT_SPP_SDP_ATTRIBUTE_PROTOCOL_DESCRIPTOR(BT_ULL_ROLE_SERVER_ID)
};

static const uint8_t bt_spp_ull_browse_group[] = {
    BT_SPP_SDP_ATTRIBUTE_PUBLIC_BROWSE_GROUP
};

static const uint8_t bt_spp_ull_language[] = {
    BT_SPP_SDP_ATTRIBUTE_LANGUAGE
};

static const uint8_t bt_spp_ull_service_name[] = {
    BT_SPP_SDP_ATTRIBUTE_SIZE_OF_SERVICE_NAME(10),
    'A', 'i', 'r', 'o', 'h', 'a', '_', 'U', 'L', 'L'
};

static const bt_sdps_attribute_t bt_spp_ull_sdp_attributes[] = {
    /* Service Class ID List attribute */
    BT_SPP_SDP_ATTRIBUTE_SERVICE_CLASS_ID_LIST(bt_spp_ull_service_class_id),
    /* Protocol Descriptor List attribute */
    BT_SPP_SDP_ATTRIBUTE_PROTOCOL_DESC_LIST(bt_spp_ull_protocol_descriptor_list),
    /* Public Browse Group Service */
    BT_SPP_SDP_ATTRIBUTE_BROWSE_GROUP_LIST(bt_spp_ull_browse_group),
    /* Language Base ID List attribute */
    BT_SPP_SDP_ATTRIBUTE_LANGUAGE_BASE_LIST(bt_spp_ull_language),
    /* Serial Port Profile Service Name */
    BT_SPP_SDP_ATTRIBUTE_SERVICE_NAME(bt_spp_ull_service_name)
};

static const bt_sdps_record_t bt_spp_ull_sdp_record = {
    .attribute_list_length = sizeof(bt_spp_ull_sdp_attributes),
    .attribute_list = bt_spp_ull_sdp_attributes,
};

bt_status_t bt_ull_sdp_register(bt_ull_role_t role)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    if (BT_ULL_ROLE_CLIENT == role || BT_ULL_ROLE_SERVER == role) {
        status = bt_callback_manager_add_sdp_customized_record(&bt_spp_ull_sdp_record);
    } else {
        bt_utils_assert(0 && "unknown role");
    }
    return status;
}

const uint8_t *bt_ull_get_uuid(void)
{
    return bt_ull_client_uuid;
}



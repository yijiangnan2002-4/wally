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

#include "bt_aws_mce_le_association_internal.h"
#include "bt_aws_mce_le_association.h"

#if _MSC_VER >= 1500
#pragma comment(linker, "/alternatename:_bt_aws_mce_le_association_event_callback=_default_bt_aws_mce_le_association_event_callback")
#elif defined(__GNUC__) || defined(__ICCARM__) || defined(__ARMCC_VERSION) || defined(__CC_ARM)
#pragma weak bt_aws_mce_le_association_event_callback = default_bt_aws_mce_le_association_event_callback
#else
#error "Unsupported Platform"
#endif


char        bt_aws_mce_le_association_manufacturer[BT_AWS_MCE_LE_ASSOCIATION_MAX_MANUFACTURER_LEN + 1] = {0};
uint8_t     bt_aws_mce_le_association_version = 0;

void bt_aws_mce_le_association_set_manufacturer_version(const char *manufacturer, uint8_t version)
{
    if (manufacturer != NULL) {
        memset(bt_aws_mce_le_association_manufacturer, 0, BT_AWS_MCE_LE_ASSOCIATION_MAX_MANUFACTURER_LEN + 1);
        int manufacturer_len = strlen(manufacturer);
        if (manufacturer_len > BT_AWS_MCE_LE_ASSOCIATION_MAX_MANUFACTURER_LEN) {
            manufacturer_len = BT_AWS_MCE_LE_ASSOCIATION_MAX_MANUFACTURER_LEN;
        }
        memcpy(bt_aws_mce_le_association_manufacturer, manufacturer, manufacturer_len);
    }
    bt_aws_mce_le_association_version = version;
}

void default_bt_aws_mce_le_association_event_callback(bt_aws_mce_le_association_event_t event, bt_status_t status, void *buffer, uint16_t length)
{
    /* Do nothing */
}

bt_status_t bt_aws_mce_le_association_event_handler(bt_msg_type_t msg, bt_status_t status, void *buffer)
{
    bt_aws_mce_le_association_client_event_handler(msg, status, buffer);
    bt_aws_mce_le_association_service_event_handler(msg, status, buffer);
    return BT_STATUS_SUCCESS;
}

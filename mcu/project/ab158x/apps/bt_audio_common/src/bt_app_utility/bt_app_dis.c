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

#include "ble_dis.h"
#include "fota_util.h"
#include "app_customer_nvkey_operation.h"

#define SN_LEN 10
#define MANUFACTURER_NAME               "Audeara LTD"
#define MODEL_NUMBER                    "Audeara Buds"
#define MODEL_NUMBER2                    "Clinico Sound Earbuds CS1"
#define FIRMWARE_REVISION               FOTA_DEFAULT_VERSION
#define SOFTWARE_REVISION               "Version1.0"
#define HARDWARE_REVISION               "1.0"
#define SERIAL_NUMBER                   "20180422"
#define IEEE_DATA                       "Regulatory_Certification"
#define MANUFACTURER_ID                 0x1122334455
#define ORG_UNIQUE_ID                   0x667788

#define PNP_ID_VENDOR_ID_SOURCE         0x02
#define PNP_ID_VENDOR_ID                0x1915
#define PNP_ID_PRODUCT_ID               0xEEEE
#define PNP_ID_PRODUCT_VERSION          0x0001


bt_status_t ble_dis_get_characteristic_value_callback(ble_dis_charc_type_t charc, void *value)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    switch (charc) {
        case BLE_DIS_CHARC_MANUFACTURER_NAME: {
            if (value) {
                ble_dis_string_t *buffer = (ble_dis_string_t *)value;
                buffer->length = (uint16_t)strlen(MANUFACTURER_NAME);
                buffer->utf8_string = (uint8_t *)MANUFACTURER_NAME;
            }
            break;
        }
        case BLE_DIS_CHARC_MODEL_NUMBER: {
            if (value) {
                ble_dis_string_t *buffer = (ble_dis_string_t *)value;
		if(app_nvkey_btname_read()==0x60)
		{
	                buffer->length = (uint16_t)strlen(MODEL_NUMBER);
	                buffer->utf8_string = (uint8_t *)MODEL_NUMBER;
		}
		else if(app_nvkey_btname_read()==0x66)
		{
	                buffer->length = (uint16_t)strlen(MODEL_NUMBER2);
	                buffer->utf8_string = (uint8_t *)MODEL_NUMBER2;

		}
		else{
	                buffer->length = (uint16_t)strlen(MODEL_NUMBER);
	                buffer->utf8_string = (uint8_t *)MODEL_NUMBER;
		}
            }
            break;
        }
        case BLE_DIS_CHARC_SERIAL_NUMBER: {
            if (value) {
                uint8_t local_sn[SN_LEN] = {0};
		 app_nvkey_sn_read(local_sn, SN_LEN);
                ble_dis_string_t *buffer = (ble_dis_string_t *)value;
                buffer->length = (uint16_t)strlen(local_sn);
                buffer->utf8_string = (uint8_t *)local_sn;
            }
            break;
        }
        case BLE_DIS_CHARC_HARDWARE_REVISION: {
            if (value) {
                #define HW_VER_LEN 6
                ble_dis_string_t *buffer = (ble_dis_string_t *)value;
                uint8_t local_hw_version[HW_VER_LEN]={0};
                app_nvkey_hw_version_read(local_hw_version,HW_VER_LEN);
                buffer->length = (uint16_t)strlen(local_hw_version);
                buffer->utf8_string = (uint8_t *)local_hw_version;
            }
            break;
        }
        case BLE_DIS_CHARC_FIRMWARE_REVISION: {
            if (value) {
                ble_dis_string_t *buffer = (ble_dis_string_t *)value;
                buffer->length = (uint16_t)strlen(FIRMWARE_REVISION);
                buffer->utf8_string = (uint8_t *)FIRMWARE_REVISION;
            }
            break;
        }
        case BLE_DIS_CHARC_SOFTWARE_REVISION: {
            if (value) {
                ble_dis_string_t *buffer = (ble_dis_string_t *)value;
                buffer->length = (uint16_t)strlen(SOFTWARE_REVISION);
                buffer->utf8_string = (uint8_t *)SOFTWARE_REVISION;
            }
            break;
        }
        case BLE_DIS_CHARC_SYSTEM_ID: {
            if (value) {
                ble_dis_system_id_t *buffer = (ble_dis_system_id_t *)value;
                buffer->manufacturer_id = MANUFACTURER_ID;
                buffer->organizationally_unique_id = ORG_UNIQUE_ID;
            }
            break;
        }
        case BLE_DIS_CHARC_IEEE_DATA_LIST: {
            if (value) {
                ble_dis_ieee_data_t *buffer = (ble_dis_ieee_data_t *)value;
                buffer->list_length = (uint16_t)strlen(IEEE_DATA);
                buffer->data_list = (uint8_t *)IEEE_DATA;
            }
            break;
        }
        case BLE_DIS_CHARC_PNP_ID: {
            if (value) {
                ble_dis_pnp_id_t *buffer = (ble_dis_pnp_id_t *)value;
                buffer->vendor_id_source = PNP_ID_VENDOR_ID_SOURCE;
                buffer->vendor_id       = PNP_ID_VENDOR_ID;
                buffer->product_id      = PNP_ID_PRODUCT_ID;
                buffer->product_version = PNP_ID_PRODUCT_VERSION;

            }
            break;
        }

        default: {
            status = BT_STATUS_FAIL;
            break;
        }

    }

    return status;
}

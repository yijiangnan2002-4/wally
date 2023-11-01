/*
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

#ifdef AIR_LE_AUDIO_ENABLE

#include "apps_debug.h"

#include "app_lea_service.h"
#include "bt_callback_manager.h"
#include "bt_sdp.h"
#include "bt_connection_manager.h"
#include "apps_events_event_group.h"
#include "bt_device_manager.h"
#include "bt_sink_srv_ami.h"

#include "nvkey.h"
#include "nvkey_id_list.h"
#include "bt_customer_config.h"
#include "multi_ble_adv_manager.h"

/**************************************************************************************************
 * Define
**************************************************************************************************/
#define LOG_TAG     "[LEA][DHSS][SDP]"

#define DHSS_LE_ADDR_LENGTH         7   /* Addr type(1 byte) + Addr(6 bytes) */
#define DHSS_LE_ADDR_LIST_LENGTH    14

/* DUAL_HEADSET_SINK */
#define DHSS_UUID   0x32, 0xbf, 0x2f, 0xe6, 0x29, 0x7c, 0x4c, 0xeb, 0x83, 0x9c, 0x1b, 0xf2, 0x6a, 0x11, 0x55, 0xac

#define DHSS_ATTRIBUTE_ID_DUAL_HEADSET_PAIR_ADDRESS  0x84F6

#define DHSS_DATA_ELEMENT_DUAL_HEADSET_PAIR_ADDR    0x1C

#define DHSS_DATA_OFFSET_LE_ADDR_TYPE_1         3
#define DHSS_DATA_OFFSET_LE_ADDR_TYPE_2         10
#define DHSS_DATA_OFFSET_LE_ADDR_1              4
#define DHSS_DATA_OFFSET_LE_ADDR_2              11

static const uint8_t app_dhss_service_class_id[] = {
    BT_SDP_ATTRIBUTE_HEADER_8BIT(17),
    BT_SDP_UUID_128BIT(DHSS_UUID),
};

static const uint8_t app_dhss_protocol_descriptor_list[] = {
    BT_SDP_ATTRIBUTE_HEADER_8BIT(5),            /* Data element sequence */
    BT_SDP_ATTRIBUTE_HEADER_8BIT(3),            /* Data element sequence for L2CAP, 3 bytes */
    BT_SDP_UUID_16BIT(BT_SDP_PROTOCOL_L2CAP),   /* Uuid16 L2CAP */
};

static uint8_t app_dhss_dual_headset_pair_address[] = {
    BT_SDP_ATTRIBUTE_HEADER_8BIT(17),
    DHSS_DATA_ELEMENT_DUAL_HEADSET_PAIR_ADDR,
    0,                                          /* DHSS_DATA_OFFSET_LE_ADDR_TYPE_1 */
    0, 0, 0, 0, 0, 0,                           /* DHSS_DATA_OFFSET_LE_ADDR_1 */
    0,                                          /* DHSS_DATA_OFFSET_LE_ADDR_TYPE_2 */
    0, 0, 0, 0, 0, 0,                           /* DHSS_DATA_OFFSET_LE_ADDR_2 */
    0, 0,
};

static const bt_sdps_attribute_t app_dhss_sdp_attributes[] = {
    /* Service Class ID List attribute */
    {BT_SDP_ATTRIBUTE_ID_SERVICE_CLASS_ID_LIST, sizeof(app_dhss_service_class_id), app_dhss_service_class_id},
    /* Protocol Descriptor List attribute */
    {BT_SDP_ATTRIBUTE_ID_PROTOCOL_DESC_LIST, sizeof(app_dhss_protocol_descriptor_list), app_dhss_protocol_descriptor_list},
    /* DUAL_HEADSET_PAIR_ADDRESS */
    {DHSS_ATTRIBUTE_ID_DUAL_HEADSET_PAIR_ADDRESS, sizeof(app_dhss_dual_headset_pair_address), app_dhss_dual_headset_pair_address},
};

static const bt_sdps_record_t app_dhss_sdps_record = {
    .attribute_list_length = sizeof(app_dhss_sdp_attributes),
    .attribute_list = app_dhss_sdp_attributes,
};

static bool app_lea_dhss_is_bt_power_on = FALSE;
static bool app_lea_dhss_is_peer_le_addr_ready = FALSE;
static bool app_lea_dhss_read_peer_le_addr = FALSE;
static bool app_lea_dhss_read_local_le_addr = FALSE;

#ifdef MTK_AWS_MCE_ENABLE
static uint8_t app_lea_dhss_le_addr_offset_local = 0;
static uint8_t app_lea_dhss_le_addr_offset_peer = 0;
#endif

/**************************************************************************************************
 * Prototype
**************************************************************************************************/

/**************************************************************************************************
 * Static function
**************************************************************************************************/
static void app_le_audio_dhss_reverse_copy(uint8_t *dst, uint8_t *src)
{
    uint8_t i = 0;

    for (i = 0; i < BT_BD_ADDR_LEN; i++) {
        dst[i] = src[BT_BD_ADDR_LEN - 1 - i];
    }
}

static bool app_le_audio_dhss_reverse_compare(uint8_t *dst, uint8_t *src)
{
    uint8_t i = 0;

    for (i = 0; i < BT_BD_ADDR_LEN; i++) {
        if (dst[i] != src[BT_BD_ADDR_LEN - 1 - i]) {
            return FALSE;
        }
    }
    return TRUE;
}

#ifdef MTK_AWS_MCE_ENABLE
static bool app_le_audio_dhss_update_le_addr_offset(void)
{
    audio_channel_t channel = ami_get_audio_channel();
    /* Address: | L address| R address| */
    if (0 != app_lea_dhss_le_addr_offset_local && 0 != app_lea_dhss_le_addr_offset_peer) {
        return TRUE;
    }

    /* Get local audio location */
    switch (channel) {
        case AUDIO_CHANNEL_L:    /* Local: L */
            app_lea_dhss_le_addr_offset_local = DHSS_DATA_OFFSET_LE_ADDR_TYPE_1;
            app_lea_dhss_le_addr_offset_peer = DHSS_DATA_OFFSET_LE_ADDR_TYPE_2;
            break;
        case AUDIO_CHANNEL_R:    /* Local: R */
            app_lea_dhss_le_addr_offset_local = DHSS_DATA_OFFSET_LE_ADDR_TYPE_2;
            app_lea_dhss_le_addr_offset_peer = DHSS_DATA_OFFSET_LE_ADDR_TYPE_1;
            break;
        default:
            APPS_LOG_MSGID_E(LOG_TAG" update_le_addr_offset, error channel=%d", 1, channel);
            return FALSE;
    }
    return TRUE;
}
#endif

static void app_le_audio_dhss_update_eir(void)
{
    if (app_lea_dhss_is_peer_le_addr_ready && app_lea_dhss_read_local_le_addr && app_lea_dhss_is_bt_power_on) {
        const bt_cm_config_t *p_config = bt_customer_config_get_cm_config();

        APPS_LOG_MSGID_I(LOG_TAG" update_eir", 0);
        bt_gap_set_extended_inquiry_response((uint8_t *)(p_config->eir_data.data), p_config->eir_data.length);
    }
}

void app_le_audio_dhss_bt_power_on_update_eir(void)
{
    app_lea_dhss_is_bt_power_on = true;

    app_le_audio_dhss_update_eir();
}

bool app_le_audio_dhss_read_peer_le_addr(void)
{
#ifdef MTK_NVDM_ENABLE
    uint8_t offset = DHSS_DATA_OFFSET_LE_ADDR_TYPE_2;
    uint8_t addr[DHSS_LE_ADDR_LENGTH] = {0};
    uint32_t size = DHSS_LE_ADDR_LENGTH;
#endif

    if (app_lea_dhss_read_peer_le_addr) {
        //APPS_LOG_MSGID_I(LOG_TAG" read_peer_le_addr, already read!", 0);
        return TRUE;
    }

    app_lea_dhss_read_peer_le_addr = TRUE;

#ifdef MTK_NVDM_ENABLE
    nvkey_status_t status = nvkey_read_data(NVID_APP_LEA_DHSS_PAIR_LE_ADDR, addr, &size);
    if (NVKEY_STATUS_OK != status) {
        APPS_LOG_MSGID_E(LOG_TAG" read_peer_le_addr, error NVKEY status=%d", 1, status);
        return FALSE;
    }

#ifdef MTK_AWS_MCE_ENABLE
    /* Address: | L address| R address| */
    if (!app_le_audio_dhss_update_le_addr_offset()) {
        return FALSE;
    }
    offset = app_lea_dhss_le_addr_offset_peer;
#endif

    memcpy(&app_dhss_dual_headset_pair_address[offset], addr, DHSS_LE_ADDR_LENGTH);
    app_lea_dhss_is_peer_le_addr_ready = TRUE;
    APPS_LOG_MSGID_I(LOG_TAG" read_peer_le_addr, offset=%d type=%d addr=%02X:%02X:%02X:%02X:%02X:%02X",
                     8, offset, app_dhss_dual_headset_pair_address[offset],
                     app_dhss_dual_headset_pair_address[offset + 1],
                     app_dhss_dual_headset_pair_address[offset + 2],
                     app_dhss_dual_headset_pair_address[offset + 3],
                     app_dhss_dual_headset_pair_address[offset + 4],
                     app_dhss_dual_headset_pair_address[offset + 5],
                     app_dhss_dual_headset_pair_address[offset + 6]);
#endif

    return TRUE;
}

bool app_le_audio_dhss_read_local_le_addr(void)
{
    uint32_t nvdm_size = sizeof(bt_bd_addr_t) * MULTI_ADV_INSTANCE_MAX_COUNT;
    uint32_t count = 0;
    uint8_t temp_addr[sizeof(bt_bd_addr_t) * MULTI_ADV_INSTANCE_MAX_COUNT] = {0};
    uint8_t offset = DHSS_DATA_OFFSET_LE_ADDR_TYPE_1;

    if (app_lea_dhss_read_local_le_addr) {
        //APPS_LOG_MSGID_I(LOG_TAG" read_local_le_addr, already read!", 0);
        return TRUE;
    }

    if (NVKEY_STATUS_OK != nvkey_read_data(NVID_APP_MULTI_ADV_LE_ADDR, temp_addr, &nvdm_size)) {
        APPS_LOG_MSGID_E(LOG_TAG" read_local_le_addr, NVDM not exist!", 0);
        return FALSE;
    }

    count = (nvdm_size / sizeof(bt_bd_addr_t));
    if (count <= MULTI_ADV_INSTANCE_NOT_RHO) {
        APPS_LOG_MSGID_E(LOG_TAG" read_local_le_addr, not exist!", 0);
        return FALSE;
    }

#ifdef MTK_AWS_MCE_ENABLE
    /* Address: | L address| R address| */
    if (!app_le_audio_dhss_update_le_addr_offset()) {
        return FALSE;
    }
    offset = app_lea_dhss_le_addr_offset_local;
#endif

    if (app_lea_service_is_enable_dual_mode()) {
        app_dhss_dual_headset_pair_address[offset] = BT_ADDR_PUBLIC;
        app_le_audio_dhss_reverse_copy(&app_dhss_dual_headset_pair_address[offset + 1], (uint8_t *)bt_device_manager_aws_local_info_get_fixed_address());
    } else {
        app_dhss_dual_headset_pair_address[offset] = BT_ADDR_RANDOM;
        app_le_audio_dhss_reverse_copy(&app_dhss_dual_headset_pair_address[offset + 1], (temp_addr + sizeof(bt_bd_addr_t) * MULTI_ADV_INSTANCE_NOT_RHO));
    }
    app_lea_dhss_read_local_le_addr = TRUE;

    APPS_LOG_MSGID_I(LOG_TAG" read_local_le_addr, offset=%d type=%d addr=%02X:%02X:%02X:%02X:%02X:%02X",
                     8, offset, app_dhss_dual_headset_pair_address[offset],
                     app_dhss_dual_headset_pair_address[offset + 1],
                     app_dhss_dual_headset_pair_address[offset + 2],
                     app_dhss_dual_headset_pair_address[offset + 3],
                     app_dhss_dual_headset_pair_address[offset + 4],
                     app_dhss_dual_headset_pair_address[offset + 5],
                     app_dhss_dual_headset_pair_address[offset + 6]);

    return TRUE;
}



/**************************************************************************************************
 * Public function
**************************************************************************************************/
bool app_le_audio_dhss_get_le_addr_list(uint8_t *buf)
{
    if (NULL == buf) {
        //APPS_LOG_MSGID_E(LOG_TAG" get_le_addr_list, NULL buf", 0);
        return FALSE;
    }

    app_le_audio_dhss_read_peer_le_addr();

    if (!app_lea_dhss_is_peer_le_addr_ready) {
        APPS_LOG_MSGID_E(LOG_TAG" get_le_addr_list, NO peer addr", 0);
        return FALSE;
    }

    if (!app_le_audio_dhss_read_local_le_addr()) {
        APPS_LOG_MSGID_E(LOG_TAG" get_le_addr_list, read local fail", 0);
        return FALSE;
    }

    memcpy(buf, &app_dhss_dual_headset_pair_address[DHSS_DATA_OFFSET_LE_ADDR_TYPE_1], DHSS_LE_ADDR_LIST_LENGTH);

#ifdef MTK_AWS_MCE_ENABLE
    static uint32_t s_app_lea_dhss_log_count = 0;
    s_app_lea_dhss_log_count++;
    if (s_app_lea_dhss_log_count <= 10 || (s_app_lea_dhss_log_count % 10 == 0)) {
        if (0 != app_lea_dhss_le_addr_offset_local && 0 != app_lea_dhss_le_addr_offset_peer) {
            /* Address: | L address| R address| */
            if (DHSS_DATA_OFFSET_LE_ADDR_TYPE_1 == app_lea_dhss_le_addr_offset_local) {
                APPS_LOG_MSGID_I(LOG_TAG" get_le_addr_list, addr_1=L(local) addr_2=R(peer)", 0);
            } else {
                APPS_LOG_MSGID_I(LOG_TAG" get_le_addr_list, addr_1=L(peer) addr_2=R(local)", 0);
            }
        }

        APPS_LOG_MSGID_I(LOG_TAG" get_le_addr_list, addr_1 type=%d addr=%02X:%02X:%02X:%02X:%02X:%02X", 7,
                         app_dhss_dual_headset_pair_address[3],
                         app_dhss_dual_headset_pair_address[4],
                         app_dhss_dual_headset_pair_address[5],
                         app_dhss_dual_headset_pair_address[6],
                         app_dhss_dual_headset_pair_address[7],
                         app_dhss_dual_headset_pair_address[8],
                         app_dhss_dual_headset_pair_address[9]);
        APPS_LOG_MSGID_I(LOG_TAG" get_le_addr_list, addr_2 type=%d addr=%02X:%02X:%02X:%02X:%02X:%02X", 7,
                         app_dhss_dual_headset_pair_address[10],
                         app_dhss_dual_headset_pair_address[11],
                         app_dhss_dual_headset_pair_address[12],
                         app_dhss_dual_headset_pair_address[13],
                         app_dhss_dual_headset_pair_address[14],
                         app_dhss_dual_headset_pair_address[15],
                         app_dhss_dual_headset_pair_address[16]);
    }
#endif
    return TRUE;
}

bool app_le_audio_dhss_get_peer_le_addr(bt_addr_t *addr)
{
    uint8_t offset = DHSS_DATA_OFFSET_LE_ADDR_TYPE_2;

    if (NULL == addr) {
        APPS_LOG_MSGID_E(LOG_TAG" get_peer_le_addr, NULL addr", 0);
        return FALSE;
    }

    app_le_audio_dhss_read_peer_le_addr();
    if (!app_lea_dhss_is_peer_le_addr_ready) {
        APPS_LOG_MSGID_E(LOG_TAG" get_peer_le_addr, NO peer addr", 0);
        return FALSE;
    }

#ifdef MTK_AWS_MCE_ENABLE
    /* Address: | L address| R address| */
    offset = app_lea_dhss_le_addr_offset_peer;
#endif

    addr->type = app_dhss_dual_headset_pair_address[offset];
    memcpy(addr->addr, &app_dhss_dual_headset_pair_address[offset + 1], BT_BD_ADDR_LEN);

    return TRUE;
}

void app_le_audio_dhss_set_peer_le_addr(bt_addr_type_t type, uint8_t *addr)
{
    uint8_t offset = DHSS_DATA_OFFSET_LE_ADDR_TYPE_2;

    if (NULL == addr) {
        APPS_LOG_MSGID_E(LOG_TAG" set_peer_le_addr, error addr", 0);
        return;
    }

    APPS_LOG_MSGID_I(LOG_TAG" set_peer_le_addr, type = %d, is_paired=%d", 2, type, app_lea_dhss_is_peer_le_addr_ready);

#ifdef MTK_AWS_MCE_ENABLE
    /* Address: | L address| R address| */
    if (!app_le_audio_dhss_update_le_addr_offset()) {
        return;
    }
    offset = app_lea_dhss_le_addr_offset_peer;
#endif

    if (app_lea_dhss_is_peer_le_addr_ready &&
        app_le_audio_dhss_reverse_compare(&app_dhss_dual_headset_pair_address[offset + 1], addr)) {
        APPS_LOG_MSGID_I(LOG_TAG" set_peer_le_addr, already set", 0);
        return;
    }

    app_lea_dhss_is_peer_le_addr_ready = TRUE;
    app_dhss_dual_headset_pair_address[offset] = type;
    app_le_audio_dhss_reverse_copy(&app_dhss_dual_headset_pair_address[offset + 1], addr);

#ifdef MTK_NVDM_ENABLE
    uint32_t size = DHSS_LE_ADDR_LENGTH;
    nvkey_status_t status = nvkey_write_data(NVID_APP_LEA_DHSS_PAIR_LE_ADDR, (uint8_t *)&app_dhss_dual_headset_pair_address[offset], size);
    app_lea_dhss_read_peer_le_addr = TRUE;
    if (status != NVKEY_STATUS_OK) {
        APPS_LOG_MSGID_E(LOG_TAG" set_peer_le_addr, write nvkey error %d", 1, status);
    }
#endif

    APPS_LOG_MSGID_I(LOG_TAG" set_peer_le_addr, offset=%d type=%d addr=%02X:%02X:%02X:%02X:%02X:%02X",
                     8, offset, app_dhss_dual_headset_pair_address[offset],
                     app_dhss_dual_headset_pair_address[offset + 1],
                     app_dhss_dual_headset_pair_address[offset + 2],
                     app_dhss_dual_headset_pair_address[offset + 3],
                     app_dhss_dual_headset_pair_address[offset + 4],
                     app_dhss_dual_headset_pair_address[offset + 5],
                     app_dhss_dual_headset_pair_address[offset + 6]);

    /* check update EIR data: need local and paired le addr */
    app_le_audio_dhss_read_local_le_addr();
    app_le_audio_dhss_update_eir();
}

void app_le_audio_dhss_set_local_le_addr(bt_addr_type_t type, bt_bd_addr_t addr)
{
    uint8_t offset = DHSS_DATA_OFFSET_LE_ADDR_TYPE_1;

#ifdef MTK_AWS_MCE_ENABLE
    /* Address: | L address| R address| */
    if (!app_le_audio_dhss_update_le_addr_offset()) {
        return;
    }
    offset = app_lea_dhss_le_addr_offset_local;
#endif

    if (app_le_audio_dhss_reverse_compare(&app_dhss_dual_headset_pair_address[offset + 1], (uint8_t *)addr)) {
        APPS_LOG_MSGID_I(LOG_TAG" set_local_le_addr, already set", 0);
        return;
    }

    app_dhss_dual_headset_pair_address[offset] = type;
    app_le_audio_dhss_reverse_copy(&app_dhss_dual_headset_pair_address[offset + 1], (uint8_t *)addr);
    app_lea_dhss_read_local_le_addr = TRUE;

    APPS_LOG_MSGID_I(LOG_TAG" set_local_le_addr, offset=%d type=%d addr=%02X:%02X:%02X:%02X:%02X:%02X",
                     8, offset, app_dhss_dual_headset_pair_address[offset],
                     app_dhss_dual_headset_pair_address[offset + 1],
                     app_dhss_dual_headset_pair_address[offset + 2],
                     app_dhss_dual_headset_pair_address[offset + 3],
                     app_dhss_dual_headset_pair_address[offset + 4],
                     app_dhss_dual_headset_pair_address[offset + 5],
                     app_dhss_dual_headset_pair_address[offset + 6]);

    /* check update EIR data: need local and paired le addr */
    app_le_audio_dhss_read_peer_le_addr();
    app_le_audio_dhss_update_eir();
}

bool app_le_audio_dhss_is_data_ready(void)
{
    app_le_audio_dhss_read_peer_le_addr();
    app_le_audio_dhss_read_local_le_addr();

    if (!app_lea_dhss_is_peer_le_addr_ready || !app_lea_dhss_read_local_le_addr) {
        APPS_LOG_MSGID_E(LOG_TAG" is_data_ready, peer=%d local=%d",
                         2, app_lea_dhss_is_peer_le_addr_ready, app_lea_dhss_read_local_le_addr);
    }
    return (app_lea_dhss_is_peer_le_addr_ready && app_lea_dhss_read_local_le_addr);
}

void app_le_audio_dhss_proc_ui_shell_event(uint32_t event_group,
                                           uint32_t event_id,
                                           void *extra_data,
                                           size_t data_len)
{
    if ((EVENT_GROUP_UI_SHELL_BT == event_group) && (event_id == BT_POWER_ON_CNF)) {
        app_le_audio_dhss_bt_power_on_update_eir();
    }
}

void app_le_audio_dhss_sdp_init(void)
{
    //APPS_LOG_MSGID_I(LOG_TAG" init", 0);
    app_le_audio_dhss_read_peer_le_addr();
    app_le_audio_dhss_read_local_le_addr();
    bt_callback_manager_add_sdp_customized_record(&app_dhss_sdps_record);
}

#endif  /* AIR_LE_AUDIO_ENABLE */


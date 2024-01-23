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
#ifdef AIR_LE_AUDIO_ENABLE

#include "app_le_audio.h"
#include "app_le_audio_ba.h"

#include "bt_device_manager.h"

#include "bt_le_audio_msglog.h"

#include "ble_bap.h"
#if defined(AIR_LE_AUDIO_PBP_ENABLE)
#include "ble_pbp.h"
#endif
/**************************************************************************************************
* Define
**************************************************************************************************/

/**************************************************************************************************
* Structure
**************************************************************************************************/
#if defined(AIR_LE_AUDIO_PBP_ENABLE)
typedef struct {
    uint8_t length;
    uint8_t ad_type;
    uint16_t uuid;
    uint8_t pb_announcement_features;
    uint8_t metadata_length;
    uint8_t metadata[1];
} PACKED app_le_audio_ba_pb_announcement_t;
#endif
/**************************************************************************************************
* Variable
**************************************************************************************************/

/**************************************************************************************************
* Prototype
**************************************************************************************************/
extern void bt_app_common_at_cmd_print_report(char *string);

/**************************************************************************************************
* Static Functions
**************************************************************************************************/
#if defined(AIR_LE_AUDIO_PBP_ENABLE)
static void app_le_audio_ba_parse_pb_announcement_data(uint16_t data_length, uint8_t *p_adv)
{
    uint8_t acc_len, *p_broadcast_name;

    typedef struct {
        uint8_t len;
        uint8_t ad_type;
        uint8_t ad_data[1];
    } adv_t;

    acc_len = 0;

    while (acc_len < data_length) {
        adv_t *p_adv_data = (adv_t *)p_adv;
        app_le_audio_ba_pb_announcement_t *p_pb_announcement = (app_le_audio_ba_pb_announcement_t *)p_adv_data;

        if (BT_GAP_LE_AD_TYPE_SERVICE_DATA == p_adv_data->ad_type &&
            BT_PBP_UUID16_PUBLIC_BROADCAST_ANNOUNCEMENTS == p_pb_announcement->uuid) {
            LE_AUDIO_MSGLOG_I("[APP][BA][PBP] Public Broadcast Announcement: features:0x%x", 1, p_pb_announcement->pb_announcement_features);

        } else if (BT_GAP_LE_AD_TYPE_BROADCAST_NAME == p_adv_data->ad_type) {
            if (NULL != (p_broadcast_name = pvPortMalloc(p_adv_data->len))) {
                memset(p_broadcast_name, 0, p_adv_data->len);
                memcpy(p_broadcast_name, p_adv_data->ad_data, (p_adv_data->len - 1));
                LOG_I(LE_AUDIO, "[APP][BA][PBP] PB Announcement Name: %s", p_broadcast_name);
                vPortFree(p_broadcast_name);
            }
        }

        p_adv += (p_adv_data->len + 1);
        acc_len += (p_adv_data->len + 1);
    }
}
#endif

static void app_le_audio_ba_config_past_transfer(bt_handle_t handle)
{
    app_le_audio_ba_link_info_t *p_link_info = app_le_audio_ba_get_link_info_by_handle(handle);
    app_le_audio_ba_link_info_t *p_link_info_new = NULL;
    app_le_audio_ba_stream_info_t *p_stream_info = app_le_audio_ba_get_stream_info();
    uint8_t link_idx = 0, ba_mode = app_le_audio_ba_get_mode();
    bt_status_t status = BT_STATUS_FAIL;

    if ((NULL == p_link_info) || (NULL == p_stream_info)) {
        return;
    }

    /* Check if this link is already processing */
    if (APP_LE_AUDIO_BA_LINK_STATE_PAST_PROCESSING == p_link_info->ba_state) {
         p_link_info->ba_state = APP_LE_AUDIO_BA_LINK_STATE_PAST_FINISHED;
    } else {
        /* Check if other link is processing past */
        link_idx = app_le_audio_ba_check_state_if_exist(APP_LE_AUDIO_BA_LINK_STATE_PAST_PROCESSING);
        if (APP_LE_AUDIO_BA_INVALID_LINK_IDX != link_idx) {
            p_link_info->ba_state = APP_LE_AUDIO_BA_LINK_STATE_PAST_WAITING;
            return;
        }
    }

    /* Check if other link need to config past */
    if (APP_LE_AUDIO_BA_LINK_STATE_PAST_FINISHED == p_link_info->ba_state) {
        link_idx = app_le_audio_ba_check_state_if_exist(APP_LE_AUDIO_BA_LINK_STATE_PAST_WAITING);
        p_link_info_new = app_le_audio_ba_get_link_info_by_idx(link_idx);
        if (NULL == p_link_info_new) {
            return;
        }
    } else {
        p_link_info_new = p_link_info;
    }

    if (APP_LE_AUDIO_BA_COMMANDER_ONLY_MODE == ba_mode) {
        bt_hci_le_periodic_advrtising_sync_transfer_t param = {0};
        param.handle = p_link_info_new->handle;
        param.service_data = BT_BAP_UUID16_BASIC_AUDIO_ANNOUNCEMENTS_SERVICE;
        param.sync_handle = p_stream_info->sync_handle;
        status = bt_gap_le_periodic_advertising_sync_transfer(&param);
    } else if (APP_LE_AUDIO_BA_INITIATOR_AND_COMMANDER_MODE == ba_mode) {
        bt_hci_le_periodic_advrtising_set_info_transfer_t param = {0};
        param.handle = p_link_info_new->handle;
        param.service_data = BT_BAP_UUID16_BASIC_AUDIO_ANNOUNCEMENTS_SERVICE;
        param.adv_handle = APP_LE_AUDIO_BCST_ADV_HANDLE;
        status = bt_gap_le_periodic_advertising_set_info_transfer(&param);
    }

    if (BT_STATUS_SUCCESS == status) {
        p_link_info_new->ba_state = APP_LE_AUDIO_BA_LINK_STATE_PAST_PROCESSING;
    }
}

static void app_le_audio_ba_config_add_source_params(ble_bass_add_source_param_t *p_Data)
{
    if (NULL == p_Data) {
        return;
    }

    if (APP_LE_AUDIO_BA_INITIATOR_AND_COMMANDER_MODE == app_le_audio_ba_get_mode()) {
        bt_bd_addr_t *local_addr = bt_device_manager_get_local_address();
        uint8_t *p_bcst_id = app_le_audio_bcst_get_id();

        if (NULL == local_addr) {
            LE_AUDIO_MSGLOG_I("[APP][BA] Fail to config, local_addr not exist", 0);
            return;
        }

        memcpy(&p_Data->source_addr, local_addr, sizeof(bt_bd_addr_t));
        p_Data->source_addr_type = BT_ADDR_PUBLIC;
        p_Data->source_adv_sid = 0x08;
        if (p_bcst_id) {
            memcpy(&p_Data->broadcast_id[0], p_bcst_id, BLE_BASS_BROADCAST_ID_SIZE);
        }

    } else if (APP_LE_AUDIO_BA_COMMANDER_ONLY_MODE == app_le_audio_ba_get_mode()) {
        app_le_audio_ba_stream_info_t *p_stream_info = app_le_audio_ba_get_stream_info();

        if (NULL == p_stream_info) {
            return;
        }

        if (BT_HANDLE_INVALID == p_stream_info->sync_handle) {
            LE_AUDIO_MSGLOG_I("[APP][BA] Fail to config, PA is not synced", 0);
            return;
        }

        memcpy(&p_Data->source_addr_type, &p_stream_info->bms_addr.type, sizeof(bt_addr_t));
        p_Data->source_adv_sid = p_stream_info->advertising_sid;
        memcpy(&p_Data->broadcast_id[0], &p_stream_info->broadcast_id[0], BLE_BASS_BROADCAST_ID_SIZE);

    }
}

static uint32_t app_le_audio_ba_config_bis_index_by_link(uint8_t link_idx, bool is_sync_bis, uint32_t bis_index)
{
    app_le_audio_ba_link_info_t *p_link_info = app_le_audio_ba_get_link_info_by_idx(link_idx);
    uint32_t new_bis_index = 0;

    if (NULL == p_link_info) {
        return 0;
    }

    if (!p_link_info->sink_location) {
        if (is_sync_bis) {
            if (bis_index) {
                new_bis_index = bis_index;
            } else {
                new_bis_index = BT_BASS_BIS_SYNC_NO_PREFERENCE;
            }
        }

    } else if (p_link_info->sink_location <= (AUDIO_LOCATION_FRONT_RIGHT|AUDIO_LOCATION_FRONT_LEFT)) {
        if (is_sync_bis) {
            if (bis_index) {
                new_bis_index = bis_index;
            } else {
                new_bis_index = p_link_info->sink_location;
            }
        }

    }

    return new_bis_index;
}

static void app_le_audio_ba_config_link_to_ready(uint8_t link_idx)
{
    app_le_audio_ba_link_info_t *p_link_info = app_le_audio_ba_get_link_info_by_idx(link_idx);

    if (NULL == p_link_info) {
        return;
    }

    if (BT_HANDLE_INVALID == p_link_info->handle) {
        return;
    }

    LE_AUDIO_MSGLOG_I("[APP][BA] config_link_to_ready handle[%x] state [%x]", 2, p_link_info->handle, p_link_info->ba_state);

    if (APP_LE_AUDIO_BA_LINK_STATE_DISCOVERY_COMPLETE == p_link_info->ba_state) {
        app_le_audio_ba_read_broadcast_receive_state(p_link_info->handle);
    } else if (APP_LE_AUDIO_BA_LINK_STATE_READY == p_link_info->ba_state) {
        if (app_le_audio_ba_get_auto_play()) {
            app_le_audio_ba_play_by_link(link_idx);
        }
    } else if (APP_LE_AUDIO_BA_LINK_STATE_READY < p_link_info->ba_state) {
        p_link_info->add_source_retry = true;
        if (p_link_info->remove_source_needed) {
            ble_bass_remove_source(p_link_info->handle, p_link_info->source_id);
        }
    }
}

static void app_le_audio_ba_handle_bap_evt(uint8_t event, void *msg)
{
    app_le_audio_ba_stream_info_t *p_stream_info = app_le_audio_ba_get_stream_info();
    app_le_audio_ba_link_info_t *p_link_info = NULL;
    uint8_t link_idx = 0;
    char conn_string[50] = {0};

    if (NULL == msg) {
        return;
    }

    if (APP_LE_AUDIO_BA_COMMANDER_ONLY_MODE == app_le_audio_ba_get_mode()) {
        switch (event) {
            case BLE_BAP_BASE_BROADCAST_AUDIO_ANNOUNCEMENTS_IND: {
                ble_bap_broadcast_audio_announcements_ind_t *ind = (ble_bap_broadcast_audio_announcements_ind_t *)msg;
                LE_AUDIO_MSGLOG_I("[APP][BA] Find BMS: %02X:%02X:%02X:%02X:%02X:%02X, %02X", 7, ind->addr.addr[5], ind->addr.addr[4], ind->addr.addr[3], ind->addr.addr[2], ind->addr.addr[1], ind->addr.addr[0], ind->advertising_sid);
                snprintf((char *)conn_string, 50, "Find BMS: %02X:%02X:%02X:%02X:%02X:%02X, %02X\r\n", ind->addr.addr[5], ind->addr.addr[4], ind->addr.addr[3], ind->addr.addr[2], ind->addr.addr[1], ind->addr.addr[0], ind->advertising_sid);
                bt_app_common_at_cmd_print_report(conn_string);
#if defined(AIR_LE_AUDIO_PBP_ENABLE)
                app_le_audio_ba_parse_pb_announcement_data((uint16_t)ind->data_length, ind->data);
#endif
                break;
            }
            case BLE_BAP_BASE_SCAN_TIMEOUT_IND: {
                LE_AUDIO_MSGLOG_I("[APP][BA] Scan timeout", 0);
                snprintf((char *)conn_string, 50, "Scan timeout\r\n");
                bt_app_common_at_cmd_print_report(conn_string);
                break;
            }
            case BLE_BAP_BASE_PERIODIC_ADV_SYNC_ESTABLISHED_NOTIFY: {
                ble_bap_periodic_adv_sync_established_notify_t *noti = (ble_bap_periodic_adv_sync_established_notify_t *)msg;
                uint32_t broadcast_id = ble_bap_get_broadcast_id_by_addr(noti->advertiser_addr);

                LE_AUDIO_MSGLOG_I("[APP][BA] PA sync established, sync handle:%02x", 1, noti->sync_handle);
                snprintf((char *)conn_string, 50, "PA sync established, sync handle:%02x\r\n", noti->sync_handle);
                bt_app_common_at_cmd_print_report(conn_string);

                if (NULL != p_stream_info) {
                    p_stream_info->sync_handle = noti->sync_handle;
                    p_stream_info->advertising_sid = noti->advertising_sid;
                    memcpy(&p_stream_info->bms_addr, &noti->advertiser_addr, sizeof(bt_addr_t));
                    memcpy(&p_stream_info->broadcast_id[0], &broadcast_id, BLE_BASS_BROADCAST_ID_SIZE);
                }
                break;
            }
            case BLE_BAP_BASE_BASIC_AUDIO_ANNOUNCEMENTS_IND: {
                ble_bap_basic_audio_announcements_ind_t *ind = (ble_bap_basic_audio_announcements_ind_t*)msg;
                ble_bap_basic_audio_announcements_level_2_t *p_level_2 = NULL;
                ble_bap_basic_audio_announcements_level_3_t *p_level_3 = NULL;
                uint32_t channel_allocation = 0;
                uint8_t *temp = NULL;

                if (NULL == p_stream_info) {
                    return;
                }

                memcpy(&p_stream_info->pa_data, ind->level_1, sizeof(ble_bap_basic_audio_announcements_level_1_t));
                p_level_2 = &p_stream_info->pa_data.level_2[0];
                if (NULL != p_level_2->codec_specific_configuration) {
                    /* PTS Test: CAP/COM/BST/BV-05-C */
                    temp = (uint8_t*)ble_ascs_get_ltv_value_from_codec_specific_configuration(CODEC_CONFIGURATION_TYPE_OCTETS_PER_CODEC_FRAME,
                        p_level_2->codec_specific_configuration_length, p_level_2->codec_specific_configuration);

                    if (NULL != temp) {
                        p_stream_info->octets_per_frame = (temp[0] | (temp[1] << 8));
                    }
                }

                for (uint8_t i = 0; i < p_level_2->num_bis; i++) {
                    p_level_3 = &p_level_2->level_3[i];

                    if (NULL != p_level_3->codec_specific_configuration) {
                        temp = (uint8_t*)ble_ascs_get_ltv_value_from_codec_specific_configuration(CODEC_CONFIGURATION_TYPE_AUDIO_CHANNEL_ALLOCATION,
                            p_level_3->codec_specific_configuration_length, p_level_3->codec_specific_configuration);

                        if (NULL != temp) {
                            channel_allocation = (temp[0] | (temp[1] << 8) | (temp[2] << 16) | (temp[3] << 24));
                            LE_AUDIO_MSGLOG_I("[APP][BA] [Audio Channel Allocation:%04X] with [BIS index:%d]", 2, channel_allocation, p_level_3->bis_index);
                        }

                        for (link_idx = 0; link_idx < APP_LE_AUDIO_BA_LINK_MAX_NUM; link_idx++) {
                            if (NULL == (p_link_info = app_le_audio_ba_get_link_info_by_idx(link_idx))) {
                                continue;
                            }

                            if (BT_HANDLE_INVALID == p_link_info->handle) {
                                continue;
                            }

                            if ((channel_allocation && p_link_info->sink_location == channel_allocation) ||
                                (channel_allocation && (!p_link_info->sink_location) && i == (p_level_2->num_bis - 1))) {
                                snprintf((char *)conn_string, 50, "link[%d] is ready\r\n", link_idx);
                                bt_app_common_at_cmd_print_report(conn_string);
                            }
                        }
                    }
                }
                break;
            }
            case BLE_BAP_BASE_BIGINFO_ADV_REPORT_IND: {
                ble_bap_big_info_adv_report_ind_t *ind = (ble_bap_big_info_adv_report_ind_t*)msg;
                uint16_t temp_sdu_size = 0;
                uint16_t max_sdu = ind->maximum_sdu_size * (uint16_t)ind->num_bis;

                for (link_idx = 0; link_idx < APP_LE_AUDIO_BA_LINK_MAX_NUM; link_idx++) {
                    if (NULL == (p_link_info = app_le_audio_ba_get_link_info_by_idx(link_idx))) {
                        continue;
                    }

                    if (BT_HANDLE_INVALID == p_link_info->handle) {
                        continue;
                    }

                    if (NULL != p_stream_info) {
                        temp_sdu_size = ((uint16_t)p_link_info->frames_per_sdu) * p_stream_info->octets_per_frame;
                        /* PTS Test: CAP/COM/BST/BV-05-C */
                        if (temp_sdu_size > max_sdu) {
                            p_link_info->sink_pac_is_supported = false;
                            LE_AUDIO_MSGLOG_I("[APP][BA] Invalid sdu size", 0);
                        }
                    }
                }
                break;
            }
            case BLE_BAP_BASE_PERIODIC_ADV_TERNIMATE_IND: {
                ble_bap_periodic_adv_terminate_ind_t *noti = (ble_bap_periodic_adv_terminate_ind_t *)msg;

                LE_AUDIO_MSGLOG_I("[APP][BA] PA sync terminated, sync handle:%02x", 1, noti->sync_handle);
                snprintf((char *)conn_string, 50, "PA sync terminated, sync handle:%02x\r\n", noti->sync_handle);
                bt_app_common_at_cmd_print_report(conn_string);

                if (NULL != p_stream_info) {
                    memset(p_stream_info, 0, sizeof(app_le_audio_ba_stream_info_t));
                    p_stream_info->sync_handle = BT_HANDLE_INVALID;
                }
                break;
            }
            default:
                break;
        }
    }
}

static void app_le_audio_ba_handle_bass_evt(uint8_t event, void *msg)
{
    char conn_string[50] = {0};
    app_le_audio_ba_link_info_t *p_link_info = NULL;
    uint8_t link_idx = 0;

    if (NULL == msg) {
        return;
    }

    switch (event) {
        case BLE_BASS_READ_BROADCAST_RECEIVE_STATE_CNF: {
            ble_bass_read_broadcast_receive_state_cnf_t *p_cfm = (ble_bass_read_broadcast_receive_state_cnf_t *)msg;
            uint32_t bis_sync_state = 0;
            uint8_t empty_addr[6] = {0};

            if (NULL == (p_link_info = app_le_audio_ba_get_link_info_by_handle(p_cfm->handle))) {
                return;
            }

            p_link_info->source_id = p_cfm->source_id;

            if(p_cfm->subgroup_length > 0 && p_cfm->subgroup != NULL){
                memcpy(&bis_sync_state, &(p_cfm->subgroup[1]), sizeof(bis_sync_state));
            }

            LE_AUDIO_MSGLOG_I("[APP][BA] BLE_BASS_READ_BROADCAST_RECEIVE_STATE_CNF handle[%04x] pa_sync_state[%x] bis_sync_state[%x] ba_state[%x]"
                , 4, p_cfm->handle, p_cfm->pa_sync_state, bis_sync_state, p_link_info->ba_state);

            link_idx = app_le_audio_ba_get_link_idx_by_handle(p_cfm->handle);

            if (BT_BASS_PA_SYNC_STATE_SYNCHRONIZED_TO_PA == p_cfm->pa_sync_state) {
                p_link_info->ba_state = APP_LE_AUDIO_BA_LINK_STATE_PA_SYNCED;
                if (bis_sync_state) {
                    p_link_info->ba_state = APP_LE_AUDIO_BA_LINK_STATE_BIG_SYNCED;
                }
            } else if (0 != memcmp(&p_cfm->source_addr[0], &empty_addr[0], sizeof(bt_bd_addr_t))) {
                /* Device is not syncing BIG, shall remove source first */
                p_link_info->add_source_retry = true;
                ble_bass_remove_source(p_link_info->handle, p_link_info->source_id);
            } else {
                p_link_info->ba_state = APP_LE_AUDIO_BA_LINK_STATE_READY;
                if (APP_LE_AUDIO_BA_NOT_SUPPORT_MODE != app_le_audio_ba_get_mode()) {
                    if (app_le_audio_ba_get_auto_play()) {
                        app_le_audio_ba_play_by_link(link_idx);
                    }
                }
            }

            snprintf((char *)conn_string, 50, "link[%d] sync %02x:%02x:%02x:%02x:%02x:%02x\r\n", link_idx, p_cfm->source_addr[5], p_cfm->source_addr[4], p_cfm->source_addr[3], p_cfm->source_addr[2], p_cfm->source_addr[1], p_cfm->source_addr[0]);
            bt_app_common_at_cmd_print_report(conn_string);
            snprintf((char *)conn_string, 50, " PA:%02x, BIS:%lx\r\n", p_cfm->pa_sync_state, bis_sync_state);
            bt_app_common_at_cmd_print_report(conn_string);
            break;
        }
        case BLE_BASS_SET_BROADCAST_RECEIVE_STATE_NOTIFICATION_CNF: {
            LE_AUDIO_MSGLOG_I("[APP][BA] BLE_BASS_SET_BROADCAST_RECEIVE_STATE_NOTIFICATION_CNF", 0);
            break;
        }
        case BLE_BASS_REMOTE_SCAN_STOP_CNF: {
            LE_AUDIO_MSGLOG_I("[APP][BA] BLE_BASS_REMOTE_SCAN_STOP_CNF", 0);
            break;
        }
        case BLE_BASS_REMOTE_SCAN_START_CNF: {
            LE_AUDIO_MSGLOG_I("[APP][BA] BLE_BASS_REMOTE_SCAN_START_CNF", 0);
            break;
        }
        case BLE_BASS_ADD_SOURCE_CNF: {
            ble_bass_write_cnf_t *p_cfm = (ble_bass_write_cnf_t *) msg;
            LE_AUDIO_MSGLOG_I("[APP][BA] BLE_BASS_ADD_SOURCE_CNF handle[%x]", 1, p_cfm->handle);
            if (p_cfm->status == BT_STATUS_SUCCESS) {
                if (NULL != (p_link_info = app_le_audio_ba_get_link_info_by_handle(p_cfm->handle))) {
                    p_link_info->add_source_retry = false;
                }
            }
            break;
        }
        case BLE_BASS_MODIFY_SOURCE_CNF: {
            ble_bass_write_cnf_t *p_cfm = (ble_bass_write_cnf_t *) msg;
            LE_AUDIO_MSGLOG_I("[APP][BA] BLE_BASS_MODIFY_SOURCE_CNF handle[%x]", 1, p_cfm->handle);

            if (NULL == (p_link_info = app_le_audio_ba_get_link_info_by_handle(p_cfm->handle))) {
                return;
            }

            if (p_cfm->status == BT_STATUS_SUCCESS) {
                if (p_link_info->remove_source_needed) {
                    ble_bass_remove_source(p_link_info->handle, p_link_info->source_id);
                }
            }
            break;
        }
        case BLE_BASS_SET_BROADCAST_CODE_CNF: {
            LE_AUDIO_MSGLOG_I("[APP][BA] BLE_BASS_SET_BROADCAST_CODE_CNF", 0);
            break;
        }
        case BLE_BASS_REMOVE_SOURCE_CNF: {
            ble_bass_write_cnf_t *p_cfm = (ble_bass_write_cnf_t *) msg;
            LE_AUDIO_MSGLOG_I("[APP][BA] BLE_BASS_REMOVE_SOURCE_CNF handle[%x]", 1, p_cfm->handle);
            if (p_cfm->status == BT_STATUS_SUCCESS) {
                if (NULL == (p_link_info = app_le_audio_ba_get_link_info_by_handle(p_cfm->handle))) {
                    return;
                }

                p_link_info->ba_state = APP_LE_AUDIO_BA_LINK_STATE_READY;
                p_link_info->remove_source_needed = false;
                link_idx = app_le_audio_ba_get_link_idx_by_handle(p_cfm->handle);
                snprintf((char *)conn_string, 50, "link[%d] PAUSE", link_idx);
                bt_app_common_at_cmd_print_report(conn_string);

                if (p_link_info->add_source_retry) {
                    app_le_audio_ba_play_by_link(link_idx);
                }
            }
            break;
        }
        case BLE_BASS_BROADCAST_RECEIVE_STATE_NOTIFY: {
            ble_bass_broadcast_receive_state_notify_t *p_cfm = (ble_bass_broadcast_receive_state_notify_t *)msg;
            uint8_t empty_addr[6] = {0};
            uint32_t bis_sync_state = 0;

            if (NULL == (p_link_info = app_le_audio_ba_get_link_info_by_handle(p_cfm->handle))) {
                return;
            }

            if (0 == memcmp(&empty_addr[0], &p_cfm->source_addr[0], 6)) {
                LE_AUDIO_MSGLOG_I("[APP][BA] empty address handle[%x]", 1, p_cfm->handle);
                return;
            }

            p_link_info->source_id = p_cfm->source_id;

            if (p_cfm->subgroup_length > 0 && p_cfm->subgroup != NULL) {
                memcpy(&bis_sync_state, &(p_cfm->subgroup[1]), 4);
            }

            LE_AUDIO_MSGLOG_I("[APP][BA] BLE_BASS_BROADCAST_STATE_NOTIFY handle[%04x] Source_id[%x] pa_sync_state[%x] bis_sync_state[%x] big_encryption[%x] ba_state[%x]"
                , 6, p_cfm->handle, p_cfm->source_id, p_cfm->pa_sync_state, bis_sync_state, p_cfm->big_encryption, p_link_info->ba_state);

            link_idx = app_le_audio_ba_get_link_idx_by_handle(p_cfm->handle);

            if (BT_BASS_PA_SYNC_STATE_SYNCINFO_REQUEST == p_cfm->pa_sync_state) {
                if ((APP_LE_AUDIO_BA_LINK_STATE_PAST_PROCESSING != p_link_info->ba_state)
                    && (APP_LE_AUDIO_BA_LINK_STATE_PAST_WAITING != p_link_info->ba_state)) {
                    app_le_audio_ba_config_past_transfer(p_cfm->handle);
                }
                return;
            }

            if (BT_BASS_PA_SYNC_STATE_SYNCHRONIZED_TO_PA == p_cfm->pa_sync_state) {
                if (APP_LE_AUDIO_BA_LINK_STATE_PA_SYNCED != p_link_info->ba_state) {
                    snprintf((char *)conn_string, 50, "link[%d] PA synced wait for bis sync\r\n", link_idx);
                    bt_app_common_at_cmd_print_report(conn_string);
                    p_link_info->ba_state = APP_LE_AUDIO_BA_LINK_STATE_PA_SYNCED;
                }
                if (bis_sync_state) {
                    snprintf((char *)conn_string, 50, "link[%d] BIG synced [%02x%02x]\r\n", link_idx, (uint16_t)(bis_sync_state >> 16),(uint16_t)bis_sync_state);
                    bt_app_common_at_cmd_print_report(conn_string);
                    p_link_info->ba_state = APP_LE_AUDIO_BA_LINK_STATE_BIG_SYNCED;
                    return;
                }
            }

            if (BT_BASS_BIG_ENCRYPTION_BROADCAST_CODE_REQUIRED == p_cfm->big_encryption) {
                snprintf((char *)conn_string, 50, "link[%d] broadcast code required\r\n", link_idx);
                bt_app_common_at_cmd_print_report(conn_string);
                p_link_info->ba_state = APP_LE_AUDIO_BA_LINK_STATE_BROADCAST_CODE_REQUIRED;
                if (APP_LE_AUDIO_BA_INITIATOR_AND_COMMANDER_MODE == app_le_audio_ba_get_mode()) {
                    uint8_t *bcst_code = app_le_audio_bcst_get_code();
                    if (NULL != bcst_code) {
                        app_le_audio_ba_set_broadcast_code(link_idx, p_link_info->source_id, bcst_code);
                    }
                }
                return;
            }

            if (!bis_sync_state) {
                if (BT_BASS_PA_SYNC_STATE_NOT_SYNCHRONIZED_TO_PA == p_cfm->pa_sync_state) {
                    snprintf((char *)conn_string, 50, "link[%d] terminates BIG sync\r\n", link_idx);
                    bt_app_common_at_cmd_print_report(conn_string);
                } else if ((BT_BASS_PA_SYNC_STATE_FAILED_TO_SYNCHRONIZE_TO_PA == p_cfm->pa_sync_state) || (BT_BASS_PA_SYNC_STATE_NO_PAST == p_cfm->pa_sync_state)) {
                    LE_AUDIO_MSGLOG_I("[APP][BA] Fail to sync pa, should remove source", 0);
                } else {
                    return;
                }

                if (APP_LE_AUDIO_BA_LINK_STATE_DISCOVERY_COMPLETE == p_link_info->ba_state) {
                    /* After all le dsicovery complete, will triggger read bass state */
                    return;
                }

                p_link_info->ba_state = APP_LE_AUDIO_BA_LINK_STATE_REMOVE_SOURCE;
                p_link_info->remove_source_needed = true;
                if (p_link_info->add_source_retry) {
                    ble_bass_remove_source(p_link_info->handle, p_link_info->source_id);
                }
                return;
            }
            break;
        }
        case BLE_BASS_DISCOVER_SERVICE_COMPLETE_NOTIFY: {
            ble_bass_discover_service_complete_t *p_cfm = (ble_bass_discover_service_complete_t*)msg;

            if (NULL == (p_link_info = app_le_audio_ba_get_link_info_by_handle(p_cfm->handle))) {
                return;
            }

            LE_AUDIO_MSGLOG_I("[APP][BA] BLE_BASS_DISCOVER_SERVICE_COMPLETE_NOTIFY, handle:%X, status:%X", 2, p_cfm->handle, p_cfm->status);

            if (BT_STATUS_SUCCESS == p_cfm->status) {
                p_link_info->ba_state = APP_LE_AUDIO_BA_LINK_STATE_DISCOVERY_COMPLETE;
            }
            break;
        }
        default:
            break;
    }
}

/**************************************************************************************************
* Public Functions
**************************************************************************************************/
void app_le_audio_ba_handle_bap_client_evt(ble_bap_event_t event, void *msg)
{
    app_le_audio_ba_link_info_t *p_link_info = NULL;

    if (NULL == msg) {
        return;
    }

    switch (event) {
        case BLE_BAP_PACS_READ_SINK_PAC_CNF: {
            ble_bap_read_sink_pac_cnf_t *p_cfm = (ble_bap_read_sink_pac_cnf_t *)msg;
            uint8_t *temp = NULL;

            if (NULL == (p_link_info = app_le_audio_ba_get_link_info_by_handle(p_cfm->handle))) {
                return;
            }

            p_link_info->sink_pac_is_supported = true;

            if (p_cfm->pac_record_length) {
                /* PTS Test: CAP/COM/BST/BV-05-C */
                temp = (uint8_t*)ble_ascs_get_ltv_value_from_codec_specific_configuration(CODEC_CONFIGURATION_TYPE_CODEC_FRAME_BLOCKS_PER_SDU, p_cfm->pac_record[5], &p_cfm->pac_record[6]);
                if (NULL != temp) {
                    p_link_info->frames_per_sdu = temp[0];
                } else {
                    p_link_info->frames_per_sdu = MAX_SUPPORTED_LC3_FRAMES_PER_SDU_1;
                }
            }

            break;
        }
        case BLE_BAP_PACS_READ_SINK_LOCATION_CNF: {
            ble_bap_read_sink_location_cnf_t *p_cfm = (ble_bap_read_sink_location_cnf_t *)msg;

            if (NULL != (p_link_info = app_le_audio_ba_get_link_info_by_handle(p_cfm->handle))) {
                p_link_info->sink_location = p_cfm->location;
            }
            break;
        }
        default:
            break;
    }
}

void app_le_audio_ba_handle_sync_transfer_cnf(bt_msg_type_t msg, void *buff)
{
    switch (msg) {
        case BT_GAP_LE_PERIODIC_ADVERTISING_SYNC_TRANSFER_CNF: {
            if (APP_LE_AUDIO_BA_COMMANDER_ONLY_MODE == app_le_audio_ba_get_mode()) {
                bt_gap_le_periodic_advertising_sync_transfer_t *cfm = (bt_gap_le_periodic_advertising_sync_transfer_t *)buff;
                LE_AUDIO_MSGLOG_I("[APP][BA] BT_GAP_LE_PERIODIC_ADVERTISING_SYNC_TRANSFER_CNF, handle:%X, status:%X", 2, cfm->handle, cfm->status);
                app_le_audio_ba_config_past_transfer(cfm->handle);
            }
            break;
        }
        case BT_GAP_LE_PERIODIC_ADVERTISING_SET_INFO_TRANSFER_CNF: {
            if (APP_LE_AUDIO_BA_INITIATOR_AND_COMMANDER_MODE == app_le_audio_ba_get_mode()) {
                bt_gap_le_periodic_advertising_sync_transfer_t *cfm = (bt_gap_le_periodic_advertising_sync_transfer_t *)buff;
                LE_AUDIO_MSGLOG_I("[APP][BA] BT_GAP_LE_PERIODIC_ADVERTISING_SET_INFO_TRANSFER_CNF, handle:%X, status:%X", 2, cfm->handle, cfm->status);
                app_le_audio_ba_config_past_transfer(cfm->handle);
            }
            break;
        }
    }
}

void app_le_audio_ba_handle_connect_ind(bt_status_t ret, bt_gap_le_connection_ind_t *ind)
{
    app_le_audio_ba_link_info_t *p_link_info = NULL;
    uint8_t link_idx = 0;

    if (BT_STATUS_SUCCESS != ret) {
        return;
    }

    for (link_idx = 0; link_idx < APP_LE_AUDIO_BA_LINK_MAX_NUM; link_idx++) {
        p_link_info = app_le_audio_ba_get_link_info_by_idx(link_idx);

        if (NULL != p_link_info) {
            if (BT_HANDLE_INVALID == p_link_info->handle) {
                p_link_info->handle = ind->connection_handle;
                LE_AUDIO_MSGLOG_I("[APP][BA] Connected:%x,handle:%x", 2, link_idx, p_link_info->handle);
                return;
            }
        }
    }
}

void app_le_audio_ba_handle_disconnect_ind(bt_status_t ret, bt_gap_le_disconnect_ind_t *ind)
{
    uint8_t link_idx = app_le_audio_ba_get_link_idx_by_handle(ind->connection_handle);
    app_le_audio_ba_link_info_t *p_link_info = NULL;

    if (BT_STATUS_SUCCESS != ret) {
        return;
    }

    p_link_info = app_le_audio_ba_get_link_info_by_idx(link_idx);

    if (NULL != p_link_info) {
        LE_AUDIO_MSGLOG_I("[APP][BA] DisConnected:%x,handle:%x", 2, link_idx, p_link_info->handle);
        app_le_audio_ba_reset_link_info(link_idx);
    }
}

void app_le_audio_ba_read_broadcast_receive_state(bt_handle_t handle)
{
    app_le_audio_ba_link_info_t *p_link_info = app_le_audio_ba_get_link_info_by_handle(handle);
    bt_status_t status = BT_STATUS_FAIL;

    if (NULL == p_link_info) {
        return;
    }

    if (APP_LE_AUDIO_BA_LINK_STATE_READ_BROADCAST_RECEIVE_STATE == p_link_info->ba_state) {
        return;
    }

    status = ble_bass_read_broadcast_receive_state_req(handle);
    LE_AUDIO_MSGLOG_I("[APP][BA] BLE_BASS_READ_BROADCAST_RECEIVE_STATE_REQ, handle:%X, status:%X", 2, handle, status);

    if (BT_STATUS_SUCCESS == status) {
        p_link_info->ba_state = APP_LE_AUDIO_BA_LINK_STATE_READ_BROADCAST_RECEIVE_STATE;
    }
}

void app_le_audio_ba_play_all(uint8_t is_sync_pa, bool is_sync_bis, uint32_t bis_index)
{
    bt_status_t status = BT_STATUS_FAIL;
    ble_bass_add_source_param_t *buf = NULL;
    app_le_audio_ba_link_info_t *p_link_info = NULL;
    uint8_t i = 0, subgroup[] = {1, 0, 0, 0, 0, 0};
    uint32_t new_bis_index = 0;

    if (NULL == (buf = pvPortMalloc(sizeof(ble_bass_add_source_param_t)))) {
        LE_AUDIO_MSGLOG_I("[APP][BA] play, malloc failed", 0);
        return;
    }

    memset(buf, 0, sizeof(ble_bass_add_source_param_t));
    app_le_audio_ba_config_add_source_params(buf);

    buf->pa_sync = is_sync_pa;
    buf->subgroup_length = sizeof(subgroup);
    buf->subgroup = &subgroup[0];

    for (i = 0; i < APP_LE_AUDIO_BA_LINK_MAX_NUM; i++) {
        p_link_info = app_le_audio_ba_get_link_info_by_idx(i);

        if (NULL == p_link_info) {
            continue;
        }

        if (BT_HANDLE_INVALID == p_link_info->handle) {
            continue;
        } else if (false == p_link_info->sink_pac_is_supported) {
            if (APP_LE_AUDIO_BA_COMMANDER_ONLY_MODE == app_le_audio_ba_get_mode()) {
                continue;
            }
        }

        if (APP_LE_AUDIO_BA_LINK_STATE_READY != p_link_info->ba_state) {
            app_le_audio_ba_config_link_to_ready(i);
            continue;
        }

        new_bis_index = app_le_audio_ba_config_bis_index_by_link(i, is_sync_bis, bis_index);
        memcpy(&subgroup[1], &new_bis_index, 4);
        status = ble_bass_add_source(p_link_info->handle, buf);

        LE_AUDIO_MSGLOG_I("[APP][BA] play, handle[%x] status:%X bis_index[%x]", 3, p_link_info->handle, status, new_bis_index);

        if (BT_STATUS_SUCCESS != status) {
            p_link_info->add_source_retry = true;
        }
    }

    vPortFree(buf);
}

void app_le_audio_ba_play_by_link(uint8_t link_idx)
{
    bt_status_t status = BT_STATUS_FAIL;
    ble_bass_add_source_param_t *buf = NULL;
    app_le_audio_ba_link_info_t *p_link_info = NULL;
    uint8_t subgroup[] = {1, 0, 0, 0, 0, 0};
    uint32_t new_bis_index = 0;

    p_link_info = app_le_audio_ba_get_link_info_by_idx(link_idx);

    if (NULL == p_link_info) {
        return;
    }

    if (BT_HANDLE_INVALID == p_link_info->handle) {
        return;
    } else if (false == p_link_info->sink_pac_is_supported) {
        if (APP_LE_AUDIO_BA_COMMANDER_ONLY_MODE == app_le_audio_ba_get_mode()) {
            return;
        }
    }

    if (APP_LE_AUDIO_BA_LINK_STATE_READY != p_link_info->ba_state) {
        app_le_audio_ba_config_link_to_ready(link_idx);
        return;
    }

    if (NULL == (buf = pvPortMalloc(sizeof(ble_bass_add_source_param_t)))) {
        LE_AUDIO_MSGLOG_I("[APP][BA] play, malloc failed", 0);
        return;
    }

    memset(buf, 0, sizeof(ble_bass_add_source_param_t));
    app_le_audio_ba_config_add_source_params(buf);

    buf->pa_sync = BT_BASS_PA_SYNC_SYNCHRONIZE_TO_PA_PAST_AVAILABLE;
    buf->subgroup_length = sizeof(subgroup);
    buf->subgroup = &subgroup[0];

    new_bis_index = app_le_audio_ba_config_bis_index_by_link(link_idx, 1, 0);
    memcpy(&subgroup[1], &new_bis_index, 4);
    status = ble_bass_add_source(p_link_info->handle, buf);

    LE_AUDIO_MSGLOG_I("[APP][BA] play, handle[%x] status:%X bis_index[%x]", 3, p_link_info->handle, status, new_bis_index);

    if (BT_STATUS_SUCCESS != status) {
        p_link_info->add_source_retry = true;
    }

    vPortFree(buf);
}

void app_le_audio_ba_pause_all(uint8_t is_sync_pa, bool is_sync_bis, uint32_t bis_index)
{
    bt_status_t status = BT_STATUS_FAIL;
    ble_bass_modify_source_param_t *buf = NULL;
    app_le_audio_ba_link_info_t *p_link_info = NULL;
    uint8_t i = 0, subgroup[] = {1, 0, 0, 0, 0, 0};
    uint32_t new_bis_index = 0;


    if (NULL == (buf = pvPortMalloc(sizeof(ble_bass_modify_source_param_t)))) {
        LE_AUDIO_MSGLOG_I("[APP][BA] pause, malloc failed", 0);
        return;
    }

    memset(buf, 0, sizeof(ble_bass_modify_source_param_t));

    buf->pa_sync = is_sync_pa;
    buf->subgroup_length = sizeof(subgroup);
    buf->subgroup = &subgroup[0];

    for (i = 0; i < APP_LE_AUDIO_BA_LINK_MAX_NUM; i++) {
        p_link_info = app_le_audio_ba_get_link_info_by_idx(i);

        if (NULL == p_link_info) {
            continue;
        }

        if (BT_HANDLE_INVALID == p_link_info->handle) {
            continue;
        }

        if ((APP_LE_AUDIO_BA_LINK_STATE_PA_SYNCED != p_link_info->ba_state)
            && (APP_LE_AUDIO_BA_LINK_STATE_BIG_SYNCED != p_link_info->ba_state)) {
            LE_AUDIO_MSGLOG_I("[APP][BA] pause, handle[%x] invalid ba state[%x]", 2, p_link_info->handle, p_link_info->ba_state);
            continue;
        }

        buf->source_id = p_link_info->source_id;
        new_bis_index = app_le_audio_ba_config_bis_index_by_link(i, is_sync_bis, bis_index);
        memcpy(&subgroup[1], &new_bis_index, 4);
        status = ble_bass_modify_source(p_link_info->handle, buf);

        LE_AUDIO_MSGLOG_I("[APP][BA] pause, handle[%x] status:%X bis_index[%x]", 3, p_link_info->handle, status, new_bis_index);
    }

    vPortFree(buf);
}

bt_addr_t app_le_audio_ba_get_pa(void)
{
    app_le_audio_ba_stream_info_t *p_stream_info = app_le_audio_ba_get_stream_info();
    bt_addr_t addr= {0};

    if (NULL == p_stream_info) {
        return addr;
    }

    if ((p_stream_info->sync_handle) && (BT_HANDLE_INVALID != p_stream_info->sync_handle)) {
        return p_stream_info->bms_addr;
    }

    return addr;
}

void app_le_audio_ba_set_broadcast_code(uint8_t link_idx, uint8_t source_id,const uint8_t *param)
{
    app_le_audio_ba_link_info_t *p_link_info = app_le_audio_ba_get_link_info_by_idx(link_idx);

    if (NULL == p_link_info) {
        return;
    }

    if ((APP_LE_AUDIO_BA_LINK_STATE_BROADCAST_CODE_REQUIRED == p_link_info->ba_state)
        && (BT_HANDLE_INVALID != p_link_info->handle)) {
        ble_bass_set_broadcast_code(p_link_info->handle, source_id, param);
    }
}

void app_le_audio_ba_start(uint8_t ba_mode)
{
    uint8_t i = 0, link_num = app_le_audio_ba_get_link_num();

    LE_AUDIO_MSGLOG_I("[APP][BA] start mode:%x, link_num:%x", 2, ba_mode, link_num);

    app_le_audio_ba_set_mode(ba_mode);

    if (APP_LE_AUDIO_BA_INITIATOR_AND_COMMANDER_MODE == ba_mode) {
        if (link_num) {
            for (i = 0; i < APP_LE_AUDIO_BA_LINK_MAX_NUM; i++) {
                app_le_audio_ba_config_link_to_ready(i);
            }
        }

    } else if (APP_LE_AUDIO_BA_COMMANDER_ONLY_MODE == ba_mode) {
        ble_bap_scan_broadcast_source(DEFAULT_SCAN_TIMEOUT, BT_HCI_SCAN_FILTER_ACCEPT_ALL_ADVERTISING_PACKETS);

    }
}

void app_le_audio_ba_stop_assistant(void)
{
    app_le_audio_ba_stream_info_t *p_stream_info = app_le_audio_ba_get_stream_info();

    if (APP_LE_AUDIO_BA_COMMANDER_ONLY_MODE != app_le_audio_ba_get_mode()) {
        return;
    }

    if (NULL == p_stream_info) {
        return;
    }

    if (p_stream_info->sync_handle) {
        if (BT_HANDLE_INVALID != p_stream_info->sync_handle) {
            LE_AUDIO_MSGLOG_I("[APP][BA] stop broadcast assistant", 0);
            app_le_audio_ba_pause_all(0, 0, 0);
            ble_bap_stop_syncing_to_periodic_advertising(p_stream_info->sync_handle);
            memset(p_stream_info, 0, sizeof(app_le_audio_ba_stream_info_t));
            p_stream_info->sync_handle = BT_HANDLE_INVALID;
        }
    }
}

void app_le_audio_ba_init(void)
{
    app_le_audio_ba_utillity_init();

    ble_bap_init(app_le_audio_ba_handle_bap_evt, APP_LE_AUDIO_BA_LINK_MAX_NUM);

    ble_bass_client_init(app_le_audio_ba_handle_bass_evt, APP_LE_AUDIO_BA_LINK_MAX_NUM);
}

#endif  /* AIR_LE_AUDIO_ENABLE */


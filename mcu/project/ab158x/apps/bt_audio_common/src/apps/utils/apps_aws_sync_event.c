/* Copyright Statement:
 *
 * (C) 2017  Airoha Technology Corp. All rights reserved.
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

#include "apps_aws_sync_event.h"
#include "FreeRTOS.h"
#include "apps_debug.h"
#include "ui_shell_manager.h"
#include "bt_device_manager.h"
#include "bt_aws_mce_srv.h"
#include "hal_platform.h"
#include "hal_gpt.h"

#define LOG_TAG         "[app_aws_sync]"

bt_status_t apps_aws_sync_event_send_extra(uint32_t event_group, uint32_t event_id,
                                           const void *extra_data, uint32_t extra_data_len)
{
#ifdef AIR_SPEAKER_ENABLE
    bt_aws_mce_role_t role = bt_connection_manager_device_local_info_get_aws_role();
    bt_aws_mce_srv_mode_t aws_mode = bt_aws_mce_srv_get_mode();
    if (aws_mode == BT_AWS_MCE_SRV_MODE_BROADCAST || aws_mode == BT_AWS_MCE_SRV_MODE_SINGLE
        || role == BT_AWS_MCE_ROLE_NONE || role == BT_AWS_MCE_ROLE_CLINET) {
        APPS_LOG_MSGID_E(LOG_TAG" send fail, role=%02X aws_mode=%d", 2, role, aws_mode);
        return BT_STATUS_FAIL;
    }
#endif

    bt_status_t ret;
    uint8_t info_array[sizeof(bt_aws_mce_report_info_t)];
    bt_aws_mce_report_info_t *aws_data = (bt_aws_mce_report_info_t *)&info_array;
    uint8_t *data_array = pvPortMalloc(sizeof(uint32_t) + sizeof(uint32_t) + extra_data_len);
    if (!data_array) {
        ret = BT_STATUS_OUT_OF_MEMORY;
    } else {
        aws_data->param = data_array;
        aws_data->is_sync = false;
        aws_data->sync_time = 0;
        aws_data->module_id = BT_AWS_MCE_REPORT_MODULE_APP_ACTION;
        aws_data->param_len = 0;
        memcpy(data_array, &event_group, sizeof(uint32_t));
        aws_data->param_len += sizeof(uint32_t);
        memcpy(data_array + aws_data->param_len, &event_id, sizeof(event_id));
        aws_data->param_len += sizeof(uint32_t);
        if (extra_data && extra_data_len) {
            memcpy(data_array + aws_data->param_len, extra_data, extra_data_len);
            aws_data->param_len += extra_data_len;
        }
        ret = bt_aws_mce_report_send_event(aws_data);
        APPS_LOG_MSGID_I(LOG_TAG"apps_aws_sync_event_send_extra group-event: 0x%x-0x%x, len: %d, ret:%x", 4, event_group, event_id, extra_data_len, ret);
        vPortFree(data_array);
    }
    return ret;
}

bt_status_t apps_aws_sync_event_send(uint32_t event_group, uint32_t event_id)
{
    return apps_aws_sync_event_send_extra(event_group, event_id, NULL, 0);
}

bt_status_t apps_aws_sync_event_send_for_broadcast(bool is_urgent, uint32_t event_group, uint32_t event_id,
                                                   void *extra_data, uint32_t extra_data_len)
{
    APPS_LOG_MSGID_E(LOG_TAG" send_for_broadcast, is_urgent=%d event_group=%d event_id=%d",
                     3, is_urgent, event_group, event_id);

    bt_status_t ret;
    uint8_t info_array[sizeof(bt_aws_mce_report_info_t)];
    bt_aws_mce_report_info_t *aws_data = (bt_aws_mce_report_info_t *)&info_array;
    uint8_t *data_array = pvPortMalloc(sizeof(uint32_t) + sizeof(uint32_t) + extra_data_len);
    if (!data_array) {
        ret = BT_STATUS_OUT_OF_MEMORY;
    } else {
        aws_data->param = data_array;
        aws_data->is_sync = false;
        aws_data->sync_time = 0;
        aws_data->module_id = BT_AWS_MCE_REPORT_MODULE_APP_ACTION;
        aws_data->param_len = 0;
        memcpy(data_array, &event_group, sizeof(uint32_t));
        aws_data->param_len += sizeof(uint32_t);
        memcpy(data_array + aws_data->param_len, &event_id, sizeof(event_id));
        aws_data->param_len += sizeof(uint32_t);
        if (extra_data && extra_data_len) {
            memcpy(data_array + aws_data->param_len, extra_data, extra_data_len);
            aws_data->param_len += extra_data_len;
        }
        APPS_LOG_DUMP_I("apps_aws_sync_event_send_extra dump", aws_data->param,
                        aws_data->param_len);
        if (is_urgent) {
            ret = bt_aws_mce_report_send_urgent_event(aws_data);
        } else {
            ret = bt_aws_mce_report_send_event(aws_data);
        }
        vPortFree(data_array);
    }
    return ret;
}

/* To avoid print too much log. */
const bt_aws_mce_report_info_t *s_last_aws_report_info = NULL;

void apps_aws_sync_event_decode_extra(const bt_aws_mce_report_info_t *aws_data_ind, uint32_t *event_group, uint32_t *event_id,
                                      void **p_extra_data, uint32_t *p_extra_data_len)
{
    uint8_t *p_data = (uint8_t *)aws_data_ind->param;
    size_t data_len = aws_data_ind->param_len;
    if (aws_data_ind->module_id == BT_AWS_MCE_REPORT_MODULE_APP_ACTION) {
        *event_group = *(uint32_t *)p_data;
        p_data += sizeof(uint32_t);
        data_len -= sizeof(uint32_t);

        *event_id = *(uint32_t *)p_data;
        p_data += sizeof(uint32_t);
        data_len -= sizeof(uint32_t);

        if (p_extra_data && p_extra_data_len) {
            *p_extra_data_len = data_len;
            if (data_len) {
                *p_extra_data = p_data;
            }
        }

        if (s_last_aws_report_info != aws_data_ind) {
            APPS_LOG_MSGID_I(LOG_TAG"apps_aws_sync_event_decode_extra group-event: 0x%x-0x%x, len: %d", 3, *event_group, *event_id, data_len);
            s_last_aws_report_info = aws_data_ind;
        }
    }
}

void apps_aws_sync_event_decode(const bt_aws_mce_report_info_t *aws_data_ind, uint32_t *event_group, uint32_t *event_id)
{
    if (aws_data_ind->module_id == BT_AWS_MCE_REPORT_MODULE_APP_ACTION) {
        *event_group = *(uint32_t *)aws_data_ind->param;
        *event_id = *(uint32_t *)(aws_data_ind->param + sizeof(uint32_t));
    }
}

typedef struct {
    uint32_t                magic_num;
    uint32_t                event_group;
    uint32_t                event_id;
    bt_clock_t              target_clock;
    uint32_t                delay_ms;
    uint32_t                extra_data_len;
    uint8_t                 from_role;
    uint8_t                 extra_data[0];
} __attribute__((packed)) apps_aws_sync_future_event_t;

static bool apps_aws_sync_is_aws_connected()
{
    bt_aws_mce_srv_link_type_t link_type = bt_aws_mce_srv_get_link_type();
    if (link_type == BT_AWS_MCE_SRV_LINK_NONE) {
        return false;
    }
    return true;
}

static void apps_aws_sync_execute_future_sync_event_locally(bool from_isr,
                                                            uint8_t from_role,
                                                            uint32_t event_group,
                                                            uint32_t event_id,
                                                            uint32_t extra_data_len,
                                                            uint8_t *extra_data,
                                                            bt_clock_t *target_clock,
                                                            uint32_t delay_ms)
{
    uint32_t target_gpt = 0;
    uint32_t cur_gpt = 0;

    bt_status_t bt_clk_to_gpt_result = BT_STATUS_SUCCESS;

    if (target_clock != NULL) {
        if ((target_clock->nclk != 0) || (target_clock->nclk_intra != 0)) {
            bt_clk_to_gpt_result = bt_sink_srv_convert_bt_clock_2_gpt_count((const bt_clock_t *)target_clock, &target_gpt);

            if (bt_clk_to_gpt_result != BT_STATUS_SUCCESS) {
                APPS_LOG_MSGID_E(LOG_TAG"[apps_aws_sync_execute_future_sync_event_locally] Failed to calculate bt clock to GPT count with result : 0x%04x",
                                    1,
                                    bt_clk_to_gpt_result);
                target_gpt = 0;
            } else {
                hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &cur_gpt);
            }
        }
    }

    APPS_LOG_MSGID_I(LOG_TAG"[apps_aws_sync_execute_future_sync_event_locally] event_group : 0x%04x, event_id : 0x%04x, from_role : 0x%02x, extra_data_len : %d, target_gpt : %d, cur_gpt : %d",
                        6,
                        event_group,
                        event_id,
                        from_role,
                        extra_data_len,
                        target_gpt,
                        cur_gpt);

    if (target_gpt != 0 && (target_gpt < cur_gpt || (target_gpt - cur_gpt) > delay_ms * 1000)) {
        APPS_LOG_MSGID_W(LOG_TAG"[apps_aws_sync_execute_future_sync_event_locally] Reset target gpt to be 0", 0);
        target_gpt = 0;
    }

    uint32_t actual_delay_ms = 0;
    if (target_gpt != 0) {
        actual_delay_ms = (target_gpt - cur_gpt) / 1000;
    }

    APPS_LOG_MSGID_I(LOG_TAG"[apps_aws_sync_execute_future_sync_event_locally] actual_delay_ms : %d",
                        1,
                        actual_delay_ms);

    uint32_t future_data_local_event_len = sizeof(apps_aws_sync_future_event_local_event_t) + extra_data_len;
    apps_aws_sync_future_event_local_event_t *future_event_local_event = (apps_aws_sync_future_event_local_event_t *)pvPortMalloc(future_data_local_event_len);

    if (future_event_local_event == NULL) {
        APPS_LOG_MSGID_E(LOG_TAG"[apps_aws_sync_execute_future_sync_event_locally] Failed to allocate buffer for ui shell event, length : %d",
                            1,
                            future_data_local_event_len);
        return;
    }
    memset(future_event_local_event, 0, sizeof(uint8_t) * future_data_local_event_len);

    future_event_local_event->from_while_role = from_role;
    future_event_local_event->extra_data_len = extra_data_len;

    if ((extra_data_len > 0) && (extra_data != NULL)) {
        memcpy(future_event_local_event->extra_data, extra_data, extra_data_len);
    }

    ui_shell_send_event(from_isr,
                        EVENT_PRIORITY_HIGHEST,
                        event_group,
                        event_id,
                        (void *)future_event_local_event,
                        future_data_local_event_len,
                        NULL,
                        actual_delay_ms);
}

/**
 * @brief Check is under sniff mode or not.
 */
extern bool bt_gap_check_connection_sniff_state(bt_bd_addr_t *address);

/**
 * @brief AWS future sync event delay timeout under sniff mode.
 */
#define APP_AWS_SYNC_FUTURE_EVENT_TIMEOUT_UNDER_SNIFF_MODE      (600)

bt_status_t apps_aws_sync_send_future_sync_event(bool from_isr,
                                                    uint32_t event_group,
                                                    uint32_t event_id,
                                                    bool need_execute_locally,
                                                    uint8_t *extra_data,
                                                    uint32_t extra_data_len,
                                                    uint32_t delay_ms)
{

    uint8_t from_role = bt_device_manager_aws_local_info_get_role();

    if (apps_aws_sync_is_aws_connected() == false) {
        APPS_LOG_MSGID_E(LOG_TAG"[apps_aws_sync_send_future_sync_event] AWS disconnected state for event_group : 0x%04x, event_id : 0x%04x, len: %d, need_execute_locally : %d",
                            4,
                            event_group,
                            event_id,
                            extra_data_len,
                            need_execute_locally);

        if (need_execute_locally == true) {
            apps_aws_sync_execute_future_sync_event_locally(from_isr,
                                                            from_role,
                                                            event_group,
                                                            event_id,
                                                            extra_data_len,
                                                            extra_data,
                                                            NULL,
                                                            0);
        }
        return BT_STATUS_FAIL;
    }

    bt_status_t ret;
    uint8_t info_array[sizeof(bt_aws_mce_report_info_t)];
    bt_aws_mce_report_info_t *aws_data = (bt_aws_mce_report_info_t *)&info_array;

    uint32_t future_event_len = sizeof(apps_aws_sync_future_event_t) + extra_data_len;

    apps_aws_sync_future_event_t *future_event = (apps_aws_sync_future_event_t *)pvPortMalloc(future_event_len);

    if (future_event == NULL) {
        APPS_LOG_MSGID_E(LOG_TAG"[apps_aws_sync_send_future_sync_event] failed to allocate memory for event_group : 0x%04x, event_id : 0x%04x, len: %d, need_execute_locally : %d",
                            4,
                            event_group,
                            event_id,
                            extra_data_len,
                            need_execute_locally);

        if (need_execute_locally == true) {
            apps_aws_sync_execute_future_sync_event_locally(from_isr,
                                                            from_role,
                                                            event_group,
                                                            event_id,
                                                            extra_data_len,
                                                            extra_data,
                                                            NULL,
                                                            0);
        }

        return BT_STATUS_OUT_OF_MEMORY;
    } else {

        memset(future_event, 0, future_event_len);

        memset((void *)(&(future_event->target_clock)), 0, sizeof(bt_clock_t));

        /**
         * @brief Fix issue - 51128
         * Root cause:
         *  If the device wish to send data under sniff mode, should exist sniff mode firstly.
         *  But exist sniff mode will cause about 500ms delay, so when partner received this packet the time has been already phased out.
         *  Then the agent and partner is out of sync for the packet.
         * Solution:
         *  If device under sniff mode, enlarge the delay timeout for this sync event to make sure agent and partner execute at the same time.
         */
        bt_bd_addr_t aws_connected_address = {0};
        bool is_sniff_mode = false;
        uint32_t aws_connected_device = bt_cm_get_connected_devices(BT_CM_PROFILE_SERVICE_MASK(BT_CM_PROFILE_SERVICE_AWS),
                                                                    &aws_connected_address,
                                                                    1);

        if (aws_connected_device >= 1) {
            /**
             * @brief Check is under sniff mode or not
             */
            is_sniff_mode = bt_gap_check_connection_sniff_state(&aws_connected_address);
        }

        APPS_LOG_MSGID_I(LOG_TAG"[apps_aws_sync_send_future_sync_event] aws connected address : 0x%02x:%02x:%02x:%02x:%02x:%02x, aws_connected : %d, sniff_mode : %d",
                            8,
                            aws_connected_address[0],
                            aws_connected_address[1],
                            aws_connected_address[2],
                            aws_connected_address[3],
                            aws_connected_address[4],
                            aws_connected_address[5],
                            aws_connected_device,
                            is_sniff_mode);

        if (is_sniff_mode == true) {
            delay_ms = APP_AWS_SYNC_FUTURE_EVENT_TIMEOUT_UNDER_SNIFF_MODE;
        }

        if (delay_ms > 0) {
            bt_clock_t target_bt_clk = {0};
            bt_status_t get_bt_clock_result = bt_sink_srv_bt_clock_addition(&target_bt_clk, 0, delay_ms * 1000);

            if (get_bt_clock_result != BT_STATUS_SUCCESS) {
                APPS_LOG_MSGID_E(LOG_TAG"[apps_aws_sync_send_future_sync_event] Get bt clock failed : 0x%04x", 1, get_bt_clock_result);
            } else {
                APPS_LOG_MSGID_I(LOG_TAG"[apps_aws_sync_send_future_sync_event] target_bt_clk : %d, %d", 2, target_bt_clk.nclk, target_bt_clk.nclk_intra);
                memcpy((void *)(&(future_event->target_clock)), (void *)(&target_bt_clk), sizeof(bt_clock_t));
            }
        }

        future_event->magic_num = APPS_AWS_SYNC_FUTURE_SYNC_EVENT_MAGIC_NUM;
        future_event->event_group = event_group;
        future_event->event_id = event_id;
        future_event->delay_ms = delay_ms;
        future_event->extra_data_len = extra_data_len;
        future_event->from_role = from_role;

        if ((extra_data_len > 0) && (extra_data != NULL)) {
            memcpy(future_event->extra_data, extra_data, extra_data_len);
        }

        aws_data->param = future_event;
        aws_data->is_sync = false;
        aws_data->sync_time = 0;
        aws_data->module_id = BT_AWS_MCE_REPORT_MODULE_APP_ACTION;
        aws_data->param_len = future_event_len;

        ret = bt_aws_mce_report_send_event(aws_data);

        APPS_LOG_MSGID_I(LOG_TAG"[apps_aws_sync_send_future_sync_event] event_group: 0x%04x, event_id : 0x%04x, len: %d, ret:%x, need_execute_locally : %d",
                            5,
                            event_group,
                            event_id,
                            extra_data_len,
                            ret,
                            need_execute_locally);

        if (need_execute_locally == true) {
            apps_aws_sync_execute_future_sync_event_locally(from_isr,
                                                            from_role,
                                                            event_group,
                                                            event_id,
                                                            extra_data_len,
                                                            extra_data,
                                                            &(future_event->target_clock),
                                                            delay_ms);
        }

        vPortFree(future_event);
        future_event = NULL;
    }

    return ret;
}

void apps_aws_sync_handle_future_sync_event(uint8_t *data, uint32_t data_len)
{
    APPS_LOG_MSGID_I(LOG_TAG"[apps_aws_sync_handle_future_sync_event] data_len : %d, expected_len : %d, data : 0x%04x",
                            3,
                            data_len,
                            sizeof(apps_aws_sync_future_event_t),
                            data);

    if ((data_len < sizeof(apps_aws_sync_future_event_t)) || (data == NULL)) {
        return;
    }

    apps_aws_sync_future_event_t *future_event = (apps_aws_sync_future_event_t *)data;

    APPS_LOG_MSGID_I(LOG_TAG"[apps_aws_sync_handle_future_sync_event] from_role : 0x%02x, event_group : 0x%04x, event_id : 0x%04x, target_clock : %d, %d, delay_ms : %d",
                        6,
                        future_event->from_role,
                        future_event->event_group,
                        future_event->event_id,
                        future_event->target_clock.nclk,
                        future_event->target_clock.nclk_intra,
                        future_event->delay_ms);

    apps_aws_sync_execute_future_sync_event_locally(false,
                                                    future_event->from_role,
                                                    future_event->event_group,
                                                    future_event->event_id,
                                                    future_event->extra_data_len,
                                                    future_event->extra_data,
                                                    &(future_event->target_clock),
                                                    future_event->delay_ms);

}


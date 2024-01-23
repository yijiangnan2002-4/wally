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

#include "apps_debug.h"
#include "bt_type.h"
#include "bt_ull_le_service.h"
#include "app_dongle_le_race.h"


#define APP_ULL_DONGLE_LE_DEBUG

#define APPS_ULL_LE_FIND_CS_DURATION    (30)    /* The duration of do ULL LE pairing (seconds). */

/* Scan adv related, scan type */
#define APP_ULL_DONGLE_LE_SCAN_NEW_DEVICE               0   /* scan new device */
#define APP_ULL_DONGLE_LE_SCAN_BONDED_DEVICE            1   /* scan bonded device */
#define APP_ULL_DONGLE_LE_SCAN_COORDINATED_SET_BY_SIRK  2   /* scan coordinated set by SIRK. */
#define APP_ULL_DONGLE_LE_SCAN_RECONNECT_DEVICE         3   /* scan device to reconnect. */
typedef uint8_t app_ull_dongle_le_scan_t;

#define APP_ULL_DONGLE_LE_SUBRATE_STATE_DISABLED        0x00
#define APP_ULL_DONGLE_LE_SUBRATE_STATE_ENABLING        0x01
#define APP_ULL_DONGLE_LE_SUBRATE_STATE_ENABLED         0x02
typedef uint8_t app_ull_dongle_le_subrate_state_t;

typedef struct {
    app_ull_dongle_le_subrate_state_t state;
    uint16_t    latency;
    uint16_t    factor;
    uint16_t    supervision_timeout;
    uint16_t    continuation_number;
} app_ull_dongle_le_subrate_info_t;

/*link information Context. */
typedef struct {
    bt_addr_t addr;                             /* peer device address */
    bt_handle_t handle;                         /* connection handle */
    bool is_ull_connected;                      /**< ull channel is connected */
    bool is_disable_sniff;                      /**< ull link sniff mode is disabled or not */
    uint16_t conn_interval;
    uint16_t conn_to;
    app_ull_dongle_le_subrate_info_t subrate_info; 
} app_ull_dongle_le_link_info_t;

void app_ull_dongle_le_init(void);
bt_status_t app_ull_dongle_le_stop_scan(void);
bt_status_t app_ull_dongle_le_start_scan(void);
bt_status_t app_ull_dongle_le_scan_device(app_ull_dongle_le_scan_t scan_type);
bt_status_t app_ull_dongle_le_scan_new_device(void);
void app_ull_dongle_le_reset_bonded_info(void);
bool app_ull_dongle_le_is_connected(void);
bool app_ull_dongle_le_is_streaming(void);
void app_ull_dongle_le_srv_event_callback(bt_ull_event_t event, void *param, uint32_t param_len);

bt_status_t app_ull_dongle_le_connect(bt_addr_t *addr);
bt_status_t app_ull_dongle_le_disconnect_device(bt_addr_t *addr);
bt_status_t app_ull_dongle_le_disconnect_all_device(void);
void app_ull_dongle_le_delete_device(bt_addr_t *addr);
uint8_t app_ull_dongle_le_get_link_num(void);
void app_ull_dongle_le_set_sirk(bt_key_t *sirk, bool update_nvkey);
bt_key_t *app_ull_dongle_le_get_sirk(bool from_nvkey);
void app_ull_dongle_le_register_race_callback(app_dongle_le_race_event_callback_t callback);
#ifdef MTK_RACE_CMD_ENABLE
void app_ull_dongle_le_get_device_status_handler(uint8_t race_channel, app_dongle_le_race_get_device_status_cmd_t *cmd);
void app_ull_dongle_le_get_device_list_handler(uint8_t race_channel);
void app_ull_dongle_le_switch_active_device_handler(uint8_t race_channel, app_dongle_le_race_switch_active_audio_cmd_t *cmd);
void app_ull_dongle_le_get_paired_device_list_handler(uint8_t race_channel);
#endif
/**
 * @brief         This function gets the list of conneted ULL devices.
 * @param[out]    list               is the list of connected ULL devices.
 * @param[in,out] count              is the input and output parameter. As an input parameter, it is the length of the list and the maximum number of
 *                                   connected ULL devices that the list can hold. As an output parameter, it is the actual number of the connected ULL devices
 *                                   stored in the list upon the return of this function, which cannot exceed the length of the list.
 * @return        None.
 */
bt_status_t app_ull_dongle_le_get_connected_device_list(bt_addr_t *list, uint8_t *count);
uint16_t app_ull_dongle_le_get_conn_handle_by_addr(bt_addr_t *addr);
bt_addr_t *app_ull_dongle_le_get_bt_addr_by_conn_handle(uint16_t conn_handle);
app_ull_dongle_le_link_info_t *app_ull_dongle_le_get_link_info(bt_handle_t handle);


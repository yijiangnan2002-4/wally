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

#ifndef __BT_FAST_PAIR_UTILITY_H__
#define __BT_FAST_PAIR_UTILITY_H__

#include "nvkey_id_list.h"
#include "bt_type.h"
#include "bt_fast_pair.h"

#define BT_FAST_PAIR_DEBUG

#define bt_fast_pair_log(_message, arg_cnt,...) LOG_MSGID_I(BT_FAST_PAIR, _message, arg_cnt, ##__VA_ARGS__)

#ifdef BT_FAST_PAIR_DEBUG
void bt_fast_pair_log_msgid_i(const char *msg, uint32_t arg_cnt, ...);
void bt_fast_pair_log_msgid_w(const char *msg, uint32_t arg_cnt, ...);
void bt_fast_pair_log_msgid_e(const char *msg, uint32_t arg_cnt, ...);
void bt_fast_pair_log_msgid_d(const char *msg, uint32_t arg_cnt, ...);
void bt_fast_pair_log_dump_d(const char *func,
                             int line,
                             const void *data,
                             int length,
                             const char *message, ...);
/**
 * Define BT log with msg id.
 */
#define BT_FAST_PAIR_LOG_I(msg, arg_cnt, ...) bt_fast_pair_log_msgid_i(msg, arg_cnt, ## __VA_ARGS__)
#define BT_FAST_PAIR_LOG_W(msg, arg_cnt, ...) bt_fast_pair_log_msgid_w(msg, arg_cnt, ## __VA_ARGS__)
#define BT_FAST_PAIR_LOG_E(msg, arg_cnt, ...) bt_fast_pair_log_msgid_e(msg, arg_cnt, ## __VA_ARGS__)
#define BT_FAST_PAIR_LOG_D(msg, arg_cnt, ...) bt_fast_pair_log_msgid_d(msg, arg_cnt, ## __VA_ARGS__)

#define BT_FAST_PAIR_DUMP_I(msg, data, len, ...) bt_fast_pair_log_dump_i(__FUNCTION__, __LINE__, data, len, msg, ## __VA_ARGS__)
#else
#define BT_FAST_PAIR_LOG_I(...)
#define BT_FAST_PAIR_LOG_W(...)
#define BT_FAST_PAIR_LOG_E(...)
#define BT_FAST_PAIR_LOG_D(...)
#define BT_FAST_PAIR_DUMP_I(msg, arg_cnt, ...)
#endif

#define BT_FAST_PAIR_SPP_EVENT_CONNECTED        (0x01)
#define BT_FAST_PAIR_SPP_EVENT_DISCONNECTED     (0x02)
#define BT_FAST_PAIR_SPP_EVENT_DATA_IND         (0x03)
#define BT_FAST_PAIR_SPP_EVENT_BT_POWER_ON      (0x04)
#define BT_FAST_PAIR_SPP_EVENT_BT_POWER_OFF     (0x05)
typedef uint8_t bt_fast_pair_spp_event_t;

void                bt_fast_pair_spp_init(void);
void                bt_fast_pair_spp_event_handle_cb(bt_fast_pair_spp_event_t evt, bt_status_t status, bt_bd_addr_t *addr, uint8_t *data, uint16_t data_length);
bt_status_t         bt_fast_pair_spp_data_send(bt_bd_addr_t *addr, uint8_t *data, uint16_t data_length);
bt_status_t         bt_fast_pair_spp_disconnect(bt_bd_addr_t *addr);

typedef enum {
    BT_FAST_PAIR_NVKEY_STA_SUCCESS = 0x0,
    BT_FAST_PAIR_NVKEY_STA_FAIL = 0x1,
    BT_FAST_PAIR_NVKEY_STA_NOT_FOUND = 0x2
} bt_fast_pair_nvkey_sta_t;

typedef enum {
    BT_FAST_PAIR_NVDM_STA_SUCCESS = 0x0,
    BT_FAST_PAIR_NVDM_STA_FAIL = 0x1,
    BT_FAST_PAIR_NVDM_STA_NOT_FOUND = 0x2,
    BT_FAST_PAIR_NVDM_STA_INSUFFICIENT_SPACE = 0x3
} bt_fast_pair_nvdm_sta_t;

typedef enum {
    BT_FAST_PAIR_AWS_DATA_SYNC_SASS_STATE = 0x0,
    BT_FAST_PAIR_AWS_DATA_CUSTOM_DATA,
    BT_FAST_PAIR_AWS_DATA_ADD_PASSKEY,
    BT_FAST_PAIR_AWS_DATA_ADD_PASSKEY_RSP,
} bt_fast_pair_aws_data_sync_event_t;

typedef enum {
    BT_FAST_PAIR_ADD_PASSKEY_SUCCESS = 0x0,
    BT_FAST_PAIR_ADD_PASSKEY_PENDING,
    BT_FAST_PAIR_ADD_PASSKEY_FAIL,
} bt_fast_pair_additional_passkey_status_t;

/**
  * @brief This structure defines the peer device status.
 */
typedef struct {
    bt_bd_addr_t addr;                                          /**< The address of the connected seeker. */
    uint8_t account_index;                                      /**< The index of the account key in account key list of the seeker. */
    uint8_t bitmap_index;                                       /**< The position of the seeker in the bitmap. */
    uint8_t flag_playing : 1;                                   /**< One bit of flag to means the device is playing. */
} PACKED bt_fast_pair_sass_peer_device_t;

/**
  * @brief This structure defines the peer device status.
 */
typedef struct {
    uint8_t in_use_account_key;
    uint8_t custom_data;
    uint8_t peer_list_cnt;
    bt_fast_pair_sass_peer_device_t device_list[0];
} PACKED bt_fast_pair_sass_peer_state_t;

/**
  * @brief This structure defines the response of peer additional passkey.
 */
typedef struct {
    bool      status;
    uint32_t  peer_passkey;
} PACKED bt_fast_pair_peer_add_passkey_rsp_t;


#define NVKEYID_SPOT_EPHEMERAL_KEY      NVID_APP_FPS_SPOT_EIK
#define NVKEYID_SPOT_COUNTER            NVID_APP_FPS_SPOT_EID_COUNTER

typedef void (*spot_timer_callback)(void *timer_handle);

bt_fast_pair_nvdm_sta_t bt_fast_pair_nvdm_read(const char *group_name, const char *item_name, uint8_t *buf, uint32_t size);
bt_fast_pair_nvdm_sta_t bt_fast_pair_nvdm_write(const char *group_name, const char *data_item_name, const uint8_t *buffer, uint32_t size);
bt_fast_pair_nvdm_sta_t bt_fast_pair_nvdm_query_length(const char *group_name, const char *data_item_name, uint32_t *size);
bt_fast_pair_nvkey_sta_t bt_fast_pair_nvkey_read(uint16_t id, uint8_t *buf, uint32_t *size);
bt_fast_pair_nvkey_sta_t bt_fast_pair_nvkey_write(uint16_t id, uint8_t *data, uint32_t size);
bt_fast_pair_nvkey_sta_t bt_fast_pair_nvkey_length(uint16_t id, uint32_t *size);

bt_fast_pair_status_t bt_fast_pair_sha_256_hkdf(const uint8_t *salt,
                                                size_t salt_len, const uint8_t *ikm, size_t ikm_len,
                                                const uint8_t *info, size_t info_len,
                                                uint8_t *okm, size_t okm_len );

void *bt_fast_pair_spot_sys_timer_create(char *timer_name, bool repeat, uint32_t period_ms,
                                         void *usr_data, spot_timer_callback cb);
bool bt_fast_pair_spot_sys_timer_start(void *timer_handle);
bool bt_fast_pair_spot_sys_timer_stop(void *timer_handle);
bool bt_fast_pair_spot_sys_timer_delete(void *timer_handle);

uint32_t bt_fast_pair_spot_sys_run_time_second(void);
void bt_fast_pair_hex_dump(const char *msg, unsigned char *buf, unsigned int buf_len);

typedef void (*bt_fast_pair_task_function)(void *param);

typedef struct {
    bt_fast_pair_task_function      function;
    char                            name[10];
    unsigned int                    stack_depth;
    void                           *parameter;
    bool                            is_high_priority;
} bt_fast_pair_task_t;

void        bt_fast_pair_task_create(bt_fast_pair_task_t *task);
void        bt_fast_pair_task_destroy();
uint32_t    bt_fast_pair_task_get_running_ms();

void *bt_fast_pair_mutex_create(void);
void bt_fast_pair_mutex_take(void *mutex_handle);
void bt_fast_pair_mutex_give(void *mutex_handle);

/**
 * @brief     This function is for getting the received SASS state from peer earbud.
 * @return    The peer SASS state.
 */
const bt_fast_pair_sass_peer_state_t *bt_fast_pair_utility_get_sass_peer_state(void);

/**
 * @brief     This function is for sending SASS state to peer earbud.
 * @param[in] temp_data The local SASS data.
 * @param[in] in_use_account The local in use account key.
 */
void bt_fast_pair_sass_send_state_to_peer(bt_fast_pair_sass_temp_data_t *temp_data, uint8_t in_use_account);

/**
 * @brief     This function is for sending custom_data to peer earbud.
 * @param[in] custom_data The local custom_data.
 */
void bt_fast_pair_sass_send_custom_data_to_peer(uint8_t custom_data);

/**
 * @brief     This function is for clearing received peer SASS state.
 */
void bt_fast_pair_sass_clear_peer_data(void);

/**
 * @brief     This function is for sending adaitional passkey to peer earbud.
 * @param[in] add_passkey the additional passkey.
 * @return    The send status of the additional passkey. If is true, send the additional passkey success.
 */
bool bt_fast_pair_send_additional_passkey_to_peer(uint32_t add_passkey);

/**
 * @brief     This function is for sending adaitional passkey response to peer earbud.
 * @param[in] response the response of the handle additional passkey.
 */
void bt_fast_pair_send_additional_passkey_response(bt_fast_pair_peer_add_passkey_rsp_t response);

/**
 * @brief     This function is for peer earbud to handle adaitional passkey.
 * @param[in] add_passkey the additional passkey.
 */
void bt_fast_pair_core_peer_add_passkey_handle(uint32_t add_passkey);

/**
 * @brief     This function is for peer earbud to handle the response of adaitional passkey.
 * @param[in] add_passkey_rsp the additional passkey.
 */
void bt_fast_pair_core_peer_add_passkey_rsp(bt_fast_pair_peer_add_passkey_rsp_t add_passkey_rsp);

/**
 * @brief     This function is get the peer additional passkey of the peer earbud.
 * @param[out] buf the encrypted additional passkey.
 */
void bt_fast_pair_core_get_peer_add_passkey(uint8_t *buf);

/**
 * @brief     This function is for initializing the fastpair AWS data sync function.
 */
void bt_fast_pair_utility_aws_data_sync_init(void);

/**
 * @brief     This function is called to notify received SASS state from peer earbud. The function is implemented in fastpair lib.
 * @param[in] local_is_agent true means the local device is agent.
 */
void bt_fast_pair_sass_on_peer_state_updated(bool local_is_agent);

/**
 * @brief     This function is called to notify received SASS custom_data from peer earbud. The function is implemented in fastpair lib.
 * @param[in] local_is_agent true means the local device is agent.
 * @param[in] custom_data The value of custom_data.
 */
void bt_fast_pair_sass_on_peer_custom_data_updated(bool local_is_agent, uint8_t custom_data);

#endif /* __BT_FAST_PAIR_UTILITY_H__  */


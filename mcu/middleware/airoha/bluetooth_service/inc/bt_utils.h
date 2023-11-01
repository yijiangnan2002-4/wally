/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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


#ifndef __BT_UTILS_H__
#define __BT_UTILS_H__

#include <stdint.h>
#include <stdbool.h>
#include <syslog.h>
#include "FreeRTOSConfig.h"
#include "bt_type.h"

#ifdef __cplusplus
extern "C" {
#endif


#define     bt_utils_assert configASSERT

#define     BT_UTILS_LOG_MODULE_0(CREATE_FUNC)   CREATE_FUNC(BT_UTILS, PRINT_LEVEL_INFO)
//#define     BT_UTILS_LOG_MODULE_1(CREATE_FUNC)   BT_UTILS_LOG_MODULE_0(CREATE_FUNC);CREATE_FUNC(BT_DM, PRINT_LEVEL_INFO)
//#define     BT_UTILS_LOG_MODULE_2(CREATE_FUNC)   BT_UTILS_LOG_MODULE_1(CREATE_FUNC);CREATE_FUNC(BT_CM, PRINT_LEVEL_INFO)

#define     BT_UTILS_LOG_MODULE_CREATE(CREATE_FUNC) BT_UTILS_LOG_MODULE_0(CREATE_FUNC)

#define     bt_utils_log_i(_module, _message,...) LOG_I(#_module, (_message), ##__VA_ARGS__)
#define     bt_utils_log_d(_module, _message,...) LOG_D(#_module, (_message), ##__VA_ARGS__)
#define     bt_utils_log_w(_module, _message,...) LOG_W(#_module, (_message), ##__VA_ARGS__)
#define     bt_utils_log_e(_module, _message,...) LOG_E(#_module, (_message), ##__VA_ARGS__)
#define     bt_utils_log_id_i(_module, _message, arg_cnt,...) LOG_MSGID_I(#_module, _message, arg_cnt, ##__VA_ARGS__)
#define     bt_utils_log_id_d(_module, _message, arg_cnt,...) LOG_MSGID_D(#_module, _message, arg_cnt, ##__VA_ARGS__)
#define     bt_utils_log_id_w(_module, _message, arg_cnt,...) LOG_MSGID_W(#_module, _message, arg_cnt, ##__VA_ARGS__)
#define     bt_utils_log_id_e(_module, _message, arg_cnt,...) LOG_MSGID_E(#_module, _message, arg_cnt, ##__VA_ARGS__)

#define     BT_UTILS_CHECK_RET_WITH_VALUE_AND_LOG(CHECK_CONDITION, RET_VALUE, LOG_STRING, ...) \
    if (CHECK_CONDITION) {  \
        bt_utils_log_id_i(BT_UTILS, LOG_STRING, ##__VA_ARGS__); \
        return (RET_VALUE); \
    }

#define     BT_UTILS_CHECK_RET_WITH_VALUE_NO_LOG(CHECK_CONDITION, RET_VALUE)   \
    if (CHECK_CONDITION) { \
        return (RET_VALUE); \
    }

#define     BT_UTILS_CHECK_RET_NO_VALUE_WITH_LOG(CHECK_CONDITION, LOG_STRING, ...) \
    if (CHECK_CONDITION) {  \
        bt_utils_log_id_i(BT_UTILS, LOG_STRING, ##__VA_ARGS__); \
        return; \
    }

#define     BT_UTILS_CHECK_RET_NO_VALUE_NO_LOG(CHECK_CONDITION)    \
    if (CHECK_CONDITION) {  \
        return; \
    }
#ifndef UNUSED
  #define UNUSED(x)  ((void)(x))
#endif

#define BT_UTILS_SRV_NODE_FRONT   0x0
#define BT_UTILS_SRV_NODE_BACK    0x1
typedef uint32_t bt_utils_linknode_position_t;

typedef struct _bt_utils_linknode_t {
    struct _bt_utils_linknode_t *front;
} bt_utils_linknode_t;

typedef bool(*bt_utils_linknode_cmp_t)(const bt_utils_linknode_t *node, const void *data);

void       *bt_utils_memcpy(void *dest, const void *src, uint32_t size);
void       *bt_utils_memset(void *ptr, int32_t value, uint32_t num);
int32_t     bt_utils_memcmp(const void *dest, const void *src, uint32_t count);
char       *bt_utils_strcpy(char *dest, const char *src);
uint32_t    bt_utils_strlen(const char *string);
void       *bt_utils_memory_alloc(uint32_t size);
void        bt_utils_memory_free(void *point);
void        bt_utils_mutex_lock(void);
void        bt_utils_mutex_unlock(void);
void        bt_utils_get_address_from_string(const char *addr_str, bt_bd_addr_t *address);

void bt_utils_srv_linknode_insert_node(bt_utils_linknode_t *head, bt_utils_linknode_t *dest, bt_utils_linknode_position_t position);
bt_utils_linknode_t *bt_utils_srv_linknode_remove_node(bt_utils_linknode_t *head, bt_utils_linknode_t *dest);
bt_utils_linknode_t *bt_utils_linknode_travel_node(bt_utils_linknode_t *head, bt_utils_linknode_cmp_t func, const void *data);

#ifdef __cplusplus
}
#endif
#endif /* __BT_UTILS_H__ */


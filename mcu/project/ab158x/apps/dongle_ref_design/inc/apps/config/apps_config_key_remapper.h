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

/**
 * File: apps_config_key_remapper.h
 *
 * Description: This file defines the interface and type of apps_config_key_remapper.c.
 *
 */

#ifndef __APPS_CONFIG_KEY_REMAPPER_H__
#define __APPS_CONFIG_KEY_REMAPPER_H__

#include "apps_config_event_list.h"
#include "apps_config_state_list.h"
#ifdef AIRO_KEY_EVENT_ENABLE
#include "airo_key_event.h"
#endif
#include "apps_events_key_event.h"
#ifdef AIR_APP_MULTI_VA
#include "multi_va_manager.h"
#endif
#include "apps_config_nvkey_struct.h"

/** @brief
 * This enum defines serialized key events of airo_key_event_t, because airo_key_event_t is not suitable to read array.
 */
typedef enum {
    APPS_CONFIG_KEY_RELEASE = 0,            /**< serialized value of AIRO_KEY_RELEASE */
    APPS_CONFIG_KEY_SHORT_CLICK,            /**< serialized value of AIRO_KEY_SHORT_CLICK */
    APPS_CONFIG_KEY_DOUBLE_CLICK,           /**< serialized value of AIRO_KEY_DOUBLE_CLICK */
    APPS_CONFIG_KEY_TRIPLE_CLICK,           /**< serialized value of AIRO_KEY_TRIPLE_CLICK */
    APPS_CONFIG_KEY_LONG_PRESS_1,           /**< serialized value of AIRO_KEY_LONG_PRESS_1 */
    APPS_CONFIG_KEY_LONG_PRESS_2,           /**< serialized value of AIRO_KEY_LONG_PRESS_2 */
    APPS_CONFIG_KEY_LONG_PRESS_3,           /**< serialized value of AIRO_KEY_LONG_PRESS_3 */
    APPS_CONFIG_KEY_SLONG,                  /**< serialized value of AIRO_KEY_SLONG */
    APPS_CONFIG_KEY_REPEAT,                 /**< serialized value of AIRO_KEY_REPEAT */
    APPS_CONFIG_KEY_PRESS,                  /**< serialized value of AIRO_KEY_PRESS */
    APPS_CONFIG_KEY_LONG_PRESS_RELEASE_1,   /**< serialized value of AIRO_KEY_LONG_PRESS_RELEASE_1 */
    APPS_CONFIG_KEY_LONG_PRESS_RELEASE_2,   /**< serialized value of AIRO_KEY_LONG_PRESS_RELEASE_2 */
    APPS_CONFIG_KEY_LONG_PRESS_RELEASE_3,   /**< serialized value of AIRO_KEY_LONG_PRESS_RELEASE_3 */
    APPS_CONFIG_KEY_SLONG_RELEASE,          /**< serialized value of AIRO_KEY_SLONG_RELEASE */
    APPS_CONFIG_KEY_DLONG,                  /**< serialized value of AIRO_KEY_DLONG */
    APPS_CONFIG_KEY_DLONG_RELEASE,          /**< serialized value of AIRO_KEY_DLONG_RELEASE */
    APPS_CONFIG_KEY_INVALID,                /**< serialized value of AIRO_KEY_INVALID */
} apps_config_serialized_event_t;

/**
 *  @brief This structure defines the format of key mapping table.
 */
typedef struct {
    uint8_t key_id;             /* Means which key, refer to hal_keypad_table.h */
    uint16_t app_key_event;     /* The output action of the mapping table, refer to apps_config_key_action_t. */
    uint32_t supported_states;  /* MMI states. Every bit represent a state, refer to apps_config_state_t. */
} __attribute__((__packed__)) apps_config_key_event_map_t;

/**
 *  @brief This structure defines the format of configurable key & status mapping table.
 */
typedef struct {
    uint16_t app_key_event;     /* The output action of the mapping table, refer to apps_config_key_action_t. */
    uint32_t supported_states;  /* MMI states. Every bit represent a state, refer to apps_config_state_t. */
} __attribute__((__packed__)) apps_config_configurable_event_status_table_t;

/**
 * @brief      Get mapped key action. The output is mapped by key_id, key_event and MMI state.
 * @param[in]  key_id, means which key, refer to hal_keypad_table.h
 * @param[in]  key_event, means how to pressed the key, refer to airo_key_event_t.
 * @return     key action, refer to apps_config_key_action_t.
 */
apps_config_key_action_t apps_config_key_event_remapper_map_action(uint8_t key_id,
                                                                   airo_key_event_t key_event);

/**
 * @brief      Get mapped key action. The output is mapped by key_id, key_event and MMI state.
 * @param[in]  key_id, means which key, refer to hal_keypad_table.h
 * @param[in]  key_event, means how to pressed the key, refer to airo_key_event_t.
 * @param[in]  temp_mmi_state, the mmi state used for key remmap.
 * @return     key action, refer to apps_config_key_action_t.
 */
apps_config_key_action_t apps_config_key_event_remapper_map_action_in_temp_state(uint8_t key_id,
                                                                                 airo_key_event_t key_event,
                                                                                 apps_config_state_t temp_mmi_state);

/**
 * @brief      Set MMI state, it is useful to mapping key action.
 * @param[in]  state, MMI state.
 */
void apps_config_key_set_mmi_state(apps_config_state_t state);

/**
 * @brief      Get MMI state.
 * @return     current MMI state.
 */
apps_config_state_t apps_config_key_get_mmi_state(void);

/**
 * @brief      intialize the key remapper module.
 */
void apps_config_key_remaper_init_configurable_table(void);


#if defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE) || defined(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE) || defined(AIR_DCHS_MODE_ENABLE)
/**
 * @brief      Send the key remapper table to dual chip another side.
 */
void apps_config_key_remapper_dual_chip_sync_key_table(void);
#endif

/**
 * @brief      Write the key remapper table into NVDM and use it.
 * @param[in]  table, The target key remapper table.
 * @param[in]  data_size, The data size of the table.
 */
void apps_config_key_remapper_set_configurable_key_table(const apps_config_configurable_table_t *table, uint32_t data_size, bool peer_table);

#ifdef AIR_XIAOWEI_ENABLE
/**
 * @brief Get local key configure table
 *
 * @param table_count [out] Return s_configurable_table_len
 * @return const apps_config_configurable_table_t* Return s_configurable_table
 */
const apps_config_configurable_table_t *apps_config_get_local_key_configure_table(uint8_t *table_count);

/**
 * @brief Get peer key configure table
 *
 * @param table_count [out] Return the peer key configure table length
 * @return apps_config_configurable_table_t* Return the peer device key configure table
 * The return value is allocated in the function, should be freed after usage.
 * If not support AWS, return NULL;
 * If read nvkey failed, return NULL;
 */
apps_config_configurable_table_t *apps_config_get_peer_key_configure_table(uint8_t *table_count);
#endif /* AIR_XIAOWEI_ENABLE */

#ifdef MULTI_VA_SUPPORT_COMPETITION
/**
 * @brief      When support MULTI_VA_SUPPORT_COMPETITION, the mapping table is changable base on va_type.
 * @param[in]  old_va_type, last VA type.
 * @param[in]  new_va_type, current VA type.
 */
void apps_config_key_remapper_set_va_type(multi_va_type_t old_va_type,  multi_va_type_t new_va_type);
#endif

#ifdef AIR_XIAOAI_ENABLE
/**
 * @brief      Get mapped key action.
 * @param[in]  key_id, means which key, refer to hal_keypad_table.h
 * @param[in]  key_event, means how to pressed the key, refer to airo_key_event_t.
 * @param[in]  mmi_state, selected MMI state.
 * @param[in]  left_earbud, left or right.
 * @return     key action, refer to apps_config_key_action_t.
 */
apps_config_key_action_t apps_config_key_get_mapper_action(uint8_t key_id,
                                                           airo_key_event_t key_event,
                                                           apps_config_state_t mmi_state,
                                                           bool left_earbud);
#endif

#endif /* __APPS_CONFIG_KEY_REMAPPER_H__ */

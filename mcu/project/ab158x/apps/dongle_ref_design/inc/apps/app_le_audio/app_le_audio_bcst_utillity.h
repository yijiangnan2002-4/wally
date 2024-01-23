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

#ifndef __APP_LE_AUDIO_BCST_UTILLITY_H__
#define __APP_LE_AUDIO_BCST_UTILLITY_H__

#ifdef AIR_LE_AUDIO_ENABLE

#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

#include "bt_type.h"
/**************************************************************************************************
* Define
**************************************************************************************************/
/* Broadcast releated */
#define APP_LE_AUDIO_BCST_ADV_HANDLE    0x06
#define APP_LE_AUDIO_BCST_BIG_HANDLE    0x01
#define APP_LE_AUDIO_BCST_BIS_MAX_NUM   2

/* Broadcast state */
#define APP_LE_AUDIO_BCST_STATE_IDLE                     0      /* Idle */
#define APP_LE_AUDIO_BCST_STATE_START_AUDIO_STREAM       0x01   /* Start audio stream */
#define APP_LE_AUDIO_BCST_STATE_CONFIG_EXTENDED_ADV      0x02   /* Config extended ADV */
#define APP_LE_AUDIO_BCST_STATE_ENABLE_EXTENDED_ADV      0x03   /* Enable extended ADV */
#define APP_LE_AUDIO_BCST_STATE_CONFIG_PERIODIC_ADV      0x04   /* Config periodic ADV */
#define APP_LE_AUDIO_BCST_STATE_ENABLE_PERIODIC_ADV      0x05   /* Enable periodic ADV */
#define APP_LE_AUDIO_BCST_STATE_CREATE_BIG               0x06   /* Create BIG */
#define APP_LE_AUDIO_BCST_STATE_SETUP_ISO_DATA_PATH      0x07   /* Setup iso data path */
#define APP_LE_AUDIO_BCST_STATE_STREAMING                0x08   /* Streaming */
#define APP_LE_AUDIO_BCST_STATE_STOP_STREAMING           0x09   /* Prepare to stop streaming */
#define APP_LE_AUDIO_BCST_STATE_REMOVE_ISO_DATA_PATH     0x0A   /* Remove iso data path */
#define APP_LE_AUDIO_BCST_STATE_TERMINATED_BIG           0x0B   /* Terminate BIG */
#define APP_LE_AUDIO_BCST_STATE_DISABLE_PERIODIC_ADV     0x0C   /* Disable periodic ADV */
#define APP_LE_AUDIO_BCST_STATE_DISABLE_EXTENDED_ADV     0x0D   /* Disable extended ADV */
#define APP_LE_AUDIO_BCST_STATE_STOP_AUDIO_STREAM        0x0E   /* Start audio stream */
#define APP_LE_AUDIO_BCST_STATE_MAX                      0x0F
typedef uint8_t app_le_audio_bcst_state_t;

/**************************************************************************************************
* Structure
**************************************************************************************************/
typedef struct {
    app_le_audio_bcst_state_t curr_state;                                   /* current state */
    app_le_audio_bcst_state_t next_state;                                   /* next state */
    uint8_t bis_num;                                                        /* BIS number */
    bt_handle_t bis_handle[APP_LE_AUDIO_BCST_BIS_MAX_NUM];                  /* BIS handle */
    uint8_t sub_state;
    bool is_need_restart;
} app_le_audio_bcst_ctrl_t;

typedef struct {
    uint8_t sampling_rate;
    uint8_t sel_setting;
    bool    high_reliability;
} PACKED app_le_audio_bcst_qos_params_db_t;


/**************************************************************************************************
* Public function
**************************************************************************************************/
void app_le_audio_bcst_set_qos_params(uint8_t sampling_rate, uint8_t sel_setting, bool high_reliability);

uint32_t app_le_audio_bcst_get_sampling_rate(void);

uint16_t app_le_audio_bcst_get_sdu_size(void);

uint32_t app_le_audio_bcst_get_sdu_interval(void);

float app_le_audio_bcst_get_bitrate(void);

void app_le_audio_bcst_reset_info(void);

app_le_audio_bcst_state_t app_le_audio_bcst_gat_curr_state(void);

void app_le_audio_bcst_set_code(uint8_t *bcst_code);

uint8_t *app_le_audio_bcst_get_code(void);

void app_le_audio_bcst_set_id(uint32_t bcst_id);

uint8_t *app_le_audio_bcst_get_id(void);

bt_status_t app_le_audio_bcst_write_qos_params_nvkey(uint8_t sampling_rate, uint8_t sel_setting, bool high_reliability);

bt_status_t app_le_audio_bcst_read_qos_params_nvkey(app_le_audio_bcst_qos_params_db_t *bcst_qos_params_db);

#ifdef AIR_LE_AUDIO_GMAP_ENABLE
bool app_le_audio_bcst_gmap_set_qos_params(uint8_t sel_setting, uint8_t audio_config_level);
#endif /* AIR_LE_AUDIO_GMAP_ENABLE */

#endif /* AIR_LE_AUDIO_ENABLE */
#endif /* __APP_LE_AUDIO_BCST_UTILLITY_H__ */


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

#include <string.h>
#include "hal_log.h"
#include "airo_key_event.h"
#include "airo_key_event_internal.h"
#ifdef AIR_BSP_INEAR_ENABLE
#include "bsp_inear_abov_A96T516.h"
#endif

#ifdef MTK_NVDM_ENABLE
#include "nvdm.h"
#include "nvkey.h"
#include "nvkey_id_list.h"

const uint32_t airo_nvkey_key[15] = {
    NVID_AIRO_KEY_1,
    NVID_AIRO_KEY_2,
    NVID_AIRO_KEY_3,
    NVID_AIRO_KEY_4,
    NVID_AIRO_KEY_5,
    NVID_AIRO_KEY_6,
    NVID_AIRO_KEY_7,
    NVID_AIRO_KEY_8,
    NVID_AIRO_KEY_9,
    NVID_AIRO_KEY_A,
    NVID_AIRO_KEY_B,
    NVID_AIRO_KEY_C,
    NVID_AIRO_KEY_D,
    NVID_AIRO_KEY_E,
    NVID_AIRO_KEY_F,
};
#endif

static void airo_key_print_config(airo_key_event_config_t *config)
{
    log_hal_msgid_info("[airo_key] key_data       = 0x%x", 1, config->key_data);
    log_hal_msgid_info("[airo_key] t_press = %u t_release=%u t_silence=%u t_repeat=%u t_lp_1=%u t_lp_2=%u t_lp_3=%u t_slong=%u", 8, \
    config->time.t_press,config->time.t_release,config->time.t_silence,config->time.t_repeat,config->time.t_long_press_1,config->time.t_long_press_2,\
    config->time.t_long_press_3,config->time.t_slong);

    log_hal_msgid_info("[airo_key] press_release=%u slong=%u slong_repeat=%u long_level=%u long_repeat=%u multiple_click=%u", 6,\
    config->support.press_release, config->support.slong, config->support.slong_repeat, config->support.long_level, config->support.long_repeat, config->support.multiple_click);
}

bool airo_key_event_init(void)
{
    uint32_t nvkey_number = 0;
#ifdef MTK_NVDM_ENABLE
    uint32_t leng;
    uint32_t temp = 0;
    uint32_t i;

    nvdm_status_t ret;
#endif

    if (airo_key_event_state.has_initilized == true) {
        return false;
    }

    memset(&airo_key_event_state, 0, sizeof(airo_key_event_state_t));

#ifdef MTK_GSENSOR_KEY_ENABLE
    memset(&airo_gsensor_state, 0, sizeof(airo_key_gsensor_state_t));
#endif

#ifdef MTK_NVDM_ENABLE
    nvdm_init();

    leng = 4;

    ret  = nvkey_read_data(NVID_AIRO_KEY_NUMS, (uint8_t *)(&nvkey_number), &leng);
    if (NVDM_STATUS_OK != ret) {
        LOG_MSGID_E(common, "[airo_key] nvdm read total key error, ret=%d !!!\r\n", 1, ret);
    } else {
        if (((nvkey_number >> 16) & 0xffff) != 0xdead) {
            LOG_MSGID_W(common, "[airo_key] nvdm total key value error, nvkey_number=0x%x !!!\r\n", 1, nvkey_number);
        }

        temp = nvkey_number & 0xffff;
        if (temp > AIRO_KEY_SUPPORT_NUMBER) {
            temp = AIRO_KEY_SUPPORT_NUMBER;
        }
        leng = sizeof(airo_key_event_config_t);
        for (i = 0; i < temp; i++) {
            ret  = nvkey_read_data(airo_nvkey_key[i], (uint8_t *)(&airo_key_config[i]), &leng);
            if (NVDM_STATUS_OK != ret) {
                LOG_MSGID_E(common, "[airo_key] 0x%x read nvdm error, ret=%d\r\n", 2, airo_nvkey_key[i], ret);
            }
        }
    }

#endif
    LOG_MSGID_I(common, "[airo_key] total=%d, nvkey_number=%d eint_key=%d, captouch=%d, powerkey=%d, keypad=%d, psensor_key=%d\r\n", 7,
                AIRO_KEY_STATE_SIZE, \
                nvkey_number & 0xffff,\
                AIRO_EINT_KEY_NUMBER, \
                AIRO_KEY_CAPTOUCH_NUMBER, \
                AIRO_KEY_POWERKEY_NUMBER, \
                AIRO_KEY_KEYPAD_NUMBER, \
                AIRO_PSENSOR_KEY_NUMBER\
               );

    for (uint32_t i = 0; i < AIRO_KEY_SUPPORT_NUMBER; i++) {
        airo_key_print_config(&airo_key_config[i]);
    }

    airo_key_event_state.has_initilized = true;

    return true;

}

bool airo_key_register_callback(airo_key_callback_t callback, void *user_data)
{
    if (callback == NULL) {
        return false;
    }

    airo_key_event_context.callback  = callback;
    airo_key_event_context.user_data = user_data;

#ifdef AIRO_KEY_FEATRURE_POWERKEY
    hal_eint_mask(HAL_EINT_PMU);
    airo_powerkey_init();
#ifdef AIRO_KEY_FEATRURE_POWERKEY_POWERON_PRESS_EVENT
    airo_powerkey_poweron_press_event_simulation();
#endif
    hal_eint_unmask(HAL_EINT_PMU);
#endif

#ifdef MTK_KEYPAD_ENABLE
    airo_keypad_init();
#endif

#ifdef MTK_EINT_KEY_ENABLE
    airo_eint_key_init();
#ifdef AIRO_KEY_FEATRURE_EINTKEY_POWERON_PRESS_EVENT
    airo_eint_key_poweron_press_event_simulation();
#endif
#endif

#ifdef HAL_CAPTOUCH_MODULE_ENABLED
    airo_captouch_init();
#endif

#ifdef MTK_GSENSOR_KEY_ENABLE
    airo_key_gsensor_init();
#endif
#ifdef AIR_PSENSOR_KEY_ENABLE
    airo_key_psensor_init();
#endif
#ifdef AIR_BSP_INEAR_ENABLE
    bsp_inear_init();
#endif
    return true;
}

void airo_key_send_simulation_event(airo_key_event_t event, uint8_t key_data)
{
    airo_key_event_context.callback(event, key_data, airo_key_event_context.user_data);
}


bool airo_key_get_event_time(uint32_t key, airo_key_event_time_t *time)
{
    uint32_t i;
    bool result = false;

    airo_key_event_config_t *config = airo_key_config;
    if (time == NULL) {
        return false;
    }
    for (i = 0; i < AIRO_KEY_SUPPORT_NUMBER; i++, config++) {
        if (config->key_data == key) {
            memcpy(time, &config->time, sizeof(airo_key_event_time_t));
            result = true;
            break;
        }
    }
    return result;
}

bool airo_key_set_event_time(uint32_t key, airo_key_event_time_t *time)
{
    uint32_t i;
    bool result = false;

    airo_key_event_config_t *config = airo_key_config;
    if (time == NULL) {
        return false;
    }
    for (i = 0; i < AIRO_KEY_SUPPORT_NUMBER; i++, config++) {
        if (config->key_data == key) {
            memcpy(&config->time, time, sizeof(airo_key_event_time_t));
            result = true;
            break;
        }
    }

    return result;
}

bool airo_key_get_support_event(uint32_t key, airo_key_support_event_t *support)
{
    uint32_t i;
    bool result = false;

    airo_key_event_config_t *config = airo_key_config;
    if (support == NULL) {
        return false;
    }
    for (i = 0; i < AIRO_KEY_SUPPORT_NUMBER; i++, config++) {
        if (config->key_data == key) {
            memcpy(support, &config->support, sizeof(airo_key_support_event_t));
            result = true;
            break;
        }
    }
    return result;
}

bool airo_key_set_support_event(uint32_t key, airo_key_support_event_t *support)
{
    uint32_t i;
    bool result = false;

    airo_key_event_config_t *config = airo_key_config;
    if (support == NULL) {
        return false;
    }
    for (i = 0; i < AIRO_KEY_SUPPORT_NUMBER; i++, config++) {
        if (config->key_data == key) {
            memcpy(&config->support, support, sizeof(airo_key_support_event_t));
            result = true;
            break;
        }
    }

    return result;
}

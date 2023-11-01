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

#include "airo_powerkey.h"

#ifdef AIRO_KEY_FEATRURE_POWERKEY



#include "ept_keypad_drv.h"
#include "hal_log.h"
#include "hal_gpt.h"
#include "airo_key_event_internal.h"
#include "hal_pmu.h"
#if ((PRODUCT_VERSION == 1552) || defined(AM255X))
#include "hal_pmu_mt6388_platform.h"
#endif

extern pmu_operate_status_t pmu_pwrkey_normal_key_init(pmu_pwrkey_config_t *config);


static void airo_powerkey_send_state(airo_key_driven_t state)
{
    uint32_t time;
    airo_key_mapping_event_t key_event;

    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &time);
    key_event.state      = state;
    key_event.key_data   = POWERKEY_POSITION;
    key_event.time_stamp = time;

    airo_key_process_key(&key_event, AIRO_KEY_POWERKEY);
}

static bool airo_powerkey_is_pressed(void)
{
#if (defined(AB155X) || defined(AM255X))
    return pmu_get_register_value_2byte_mt6388(PMU_PWRKEY_VAL_ADDR, PMU_PWRKEY_VAL_MASK, PMU_PWRKEY_VAL_SHIFT) ? false : true;
#else
    return pmu_get_pwrkey_state();
#endif
}

static void airo_powerkey_callback1(void)
{
    airo_powerkey_send_state(AIRO_KEY_DRIVEN_PRESS);

}

static void airo_powerkey_callback2(void)
{
    airo_powerkey_send_state(AIRO_KEY_DRIVEN_RELEASE);
}


bool airo_powerkey_init(void)
{
    pmu_pwrkey_config_t config;
    pmu_operate_status_t ret;

    config.callback1  = airo_powerkey_callback1;
    config.user_data1 = NULL;
    config.callback2  = airo_powerkey_callback2;
    config.user_data2 = NULL;

    ret = pmu_pwrkey_normal_key_init(&config);
    if (ret != PMU_OK) {
        LOG_MSGID_I(common, "[powerkey] powerkey init fail\r\n", 0);
        return false;
    } else {
        return true;
    }
}


void airo_powerkey_poweron_press_event_simulation(void)
{
    if (airo_powerkey_is_pressed()) {
        LOG_MSGID_I(common, "[powerkey] press\r\n", 0);
    }
}
#endif /*AIRO_KEY_FEATRURE_POWERKEY*/


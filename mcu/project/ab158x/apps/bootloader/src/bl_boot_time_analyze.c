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

#include "bl_boot_time_analyze.h"


#ifdef AIR_BOOT_TIME_MEASURE_ENABLE

uint32_t bl_gpt_start[BOOT_TIME_MODULES_ITEM];
uint32_t bl_gpt_end[BOOT_TIME_MODULES_ITEM];
uint32_t bl_gpt_duration[BOOT_TIME_MODULES_ITEM];


const char *strings[BOOT_TIME_MODULES_ITEM] = {"Total", "Region_init", "Uart", "CLOCK" ,"DVFS" ,"FLASH" ,"LP" ,"FOTA" ,"USER_MODE" ,"WDT_RST" ,"PWR_KEY" ,"STT" ,"USB Deinit", "NVDM"};

ATTR_TEXT_IN_TCM void bl_latch_start_gpt(boot_time_modules module_idx)
{
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &bl_gpt_start[module_idx]);
}

ATTR_TEXT_IN_TCM void bl_latch_end_gpt(boot_time_modules module_idx)
{
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &bl_gpt_end[module_idx]);
    bl_gpt_duration[module_idx] = bl_gpt_end[module_idx] - bl_gpt_start[module_idx];
}

ATTR_TEXT_IN_TCM void bl_gpt_print_all(void)
{
    uint8_t i = 0;
    for(i = 0; i < BOOT_TIME_MODULES_ITEM ; i++)
    {
        bl_print(LOG_DEBUG, "%d [Module %s] duration_gpt:%d us\r\n", i, strings[i], bl_gpt_duration[i]);
    }
}

#else /*AIR_BOOT_TIME_MEASURE_ENABLE*/

ATTR_TEXT_IN_TCM void bl_latch_start_gpt(boot_time_modules module_idx){}
ATTR_TEXT_IN_TCM void bl_latch_end_gpt(boot_time_modules module_idx){}
ATTR_TEXT_IN_TCM void bl_gpt_print_all(void){}

#endif /*AIR_BOOT_TIME_MEASURE_ENABLE*/
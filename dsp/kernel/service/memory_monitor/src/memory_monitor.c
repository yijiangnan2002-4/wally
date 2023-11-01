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

#include <stdint.h>
#include <stdio.h>
#include "memory_monitor.h"
#define DWT_MONITOR_LIMIT_SIZE 64



/* ESC region address and size */
#ifndef HAL_ESC_MODULE_ENABLED
#define ESC_REGION_ADDR_START 0x0
#define ESC_REGION_SIZE 0x4000000
#endif

#define TCM_REGION_ADDR_START 0x4000000
#define TCM_REGION_SIZE       0x40000

#define MCU_SYSRAM_ADDR_START 0x4200000
#define MCU_SYSRAM_SIZE       0xc0000

extern unsigned int _DynamicVectors_start[];
extern unsigned int  _region_loader_end[];
monitor_region_info_t monitor_region_info[MONITOR_REGION_TOTAL_NUMBER];
uint32_t memory_monitor_region_number;
uint32_t monitor_current_address = 0;
uint32_t monitor_size = 0;

void memory_monitor_region_info_init(void)
{

    monitor_region_info_t monitor_region_info_temp[MONITOR_REGION_TOTAL_NUMBER] = {

#ifndef HAL_ESC_MODULE_ENABLED
        /* ESC cacheable region info */
        {"esc", ESC_REGION_ADDR_START, ESC_REGION_SIZE, WDE_DATA_RW},
#endif
        /* tcm region info*/
        {"tcm", TCM_REGION_ADDR_START , TCM_REGION_SIZE, WDE_DATA_RW},
        /* mcu sysram region info */
        {"sysram", MCU_SYSRAM_ADDR_START , MCU_SYSRAM_SIZE, WDE_DATA_RW},
#if 0
        /* dsp iram region info */
        {"iram", (uint32_t)_DynamicVectors_start , ((uint32_t)_region_loader_end - (uint32_t)_DynamicVectors_start) &(~0x3f), WDE_DATA_WO},
#endif
        {{NULL}, 0, 0, 0},
    };

    memory_monitor_region_number = sizeof(monitor_region_info_temp) / sizeof(monitor_region_info_temp[0]);
#if 0
    for (uint32_t i = 0; (i < MONITOR_REGION_TOTAL_NUMBER) && (monitor_region_info_temp[i].region_size != 0); i++ ) {
        printf("region_index:%d,region_name:%s.region_start_addr:0x%x,region_size:0x%x,region_attribute:0x%x\r\n",
               i, monitor_region_info_temp[i].region_name, monitor_region_info_temp[i].region_start_addr, monitor_region_info_temp[i].region_size, monitor_region_info_temp[i].attribute);
    }
#endif
    /* c99:All the expressions in an initializer for an object that has static storage duration shall be
     * constant expressions or string literals
    */
    memcpy(monitor_region_info, monitor_region_info_temp, sizeof(monitor_region_info_temp));
}

void memory_monitor_set_region_protected(void)
{
    static uint32_t idx = 0;
    /* update current address and size */
    monitor_current_address = monitor_region_info[idx].region_start_addr + monitor_size;
    monitor_size	+= DWT_MONITOR_LIMIT_SIZE;

    /* set dwt monitor */
    if (0 != hal_dwt_request_watchpoint(HAL_DWT_1, monitor_current_address, 0x6, monitor_region_info[idx].attribute)) {
        assert(0);
    }
    //*((volatile uint32_t *)0x4200000) = 0xaaaaaaaa;
    /* overlap handle*/
    if (monitor_current_address >= ( monitor_region_info[idx].region_start_addr +  monitor_region_info[idx].region_size)  ) {
        //printf("1-idx = %d,monitor_current_address = 0x%x,monitor_size = 0x%x,attribute = %d\r\n",
        //			idx,monitor_current_address,monitor_size,monitor_region_info[idx].attribute);
        idx++;
        if (idx >= memory_monitor_region_number) {
            idx = 0;
        }
        monitor_current_address = monitor_region_info[idx].region_start_addr;
        monitor_size = 0;
        //printf("2-idx = %d,monitor_current_address = 0x%x,monitor_size = 0x%x,attribute = %d\r\n",
        //			idx,monitor_current_address,monitor_size,monitor_region_info[idx].attribute);
    }

    if (idx >= memory_monitor_region_number ) {
        assert(0);
    }

}

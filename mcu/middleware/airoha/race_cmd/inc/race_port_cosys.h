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
/* Airoha restricted information */


#ifndef RACE_PORT_COSYS_H
#define RACE_PORT_COSYS_H


#include "race_core.h"

#ifdef __cplusplus
extern "C"
{
#endif


#ifdef AIR_LOW_LATENCY_MUX_ENABLE
#define RACE_DUAL_CHIP_COSYS_PORT MUX_LL_UART_1
#define RACE_COSYS_MUX_BUFF_SIZE  (4 * 1024)
#else

#ifdef AIR_BTA_IC_STEREO_HIGH_G3
#define RACE_DUAL_CHIP_COSYS_PORT MUX_UART_1
#else
#define RACE_DUAL_CHIP_COSYS_PORT MUX_UART_2
#endif

#define RACE_COSYS_MUX_BUFF_SIZE  (2 * 1024)
#endif

#ifdef AIR_BTA_IC_STEREO_HIGH_G3
#define RACE_DUAL_CHIP_EMCU_PORT  MUX_UART_2
#else
#define RACE_DUAL_CHIP_EMCU_PORT  MUX_UART_1
#endif

#define RACE_EMCU_MUX_BUFF_SIZE   (1 * 1024)

#ifdef AIR_RACE_CO_SYS_ENABLE

void race_cosys_init(void);

race_port_t race_cosys_get_port(void);
uint32_t race_cosys_get_handle(void);
race_port_t race_emcu_get_port(void);
uint32_t race_emcu_get_handle(void);


#endif

#ifdef __cplusplus
}
#endif


#endif


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

#ifndef _DRV_SMART_CHARGER_H_
#define _DRV_SMART_CHARGER_H_

//#include "types.h"
#include "hal_platform.h"
#include "smartchargertypes.h"
#include "types.h"

////////////////////////////////////////////////////////////////////////////////
// Type Definitions ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

typedef enum {
    HAL_SMART_CHARGER_STATUS_INVALID_PARAMETER = -2,  /**<  An invalid parameter was given. */
    HAL_SMART_CHARGER_STATUS_ERROR             = -1,  /**<  The function call failed. */
    HAL_SMART_CHARGER_STATUS_OK                =  0   /**<  The function call was successful. */
} hal_smart_charger_status_t;


////////////////////////////////////////////////////////////////////////////////
// Global Variables ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define SMCHG_LID_OPEN              1
#define SMCHG_LID_CLOSE_DONE        2
#define SMCHG_CHG_OFF               3
#define SMCHG_CHG_OUT               4
#define SMCHG_CHG_COMPL             11
#define SMCHG_CHG_RECHG             12
#define SMCHG_CHG_KEY               13
#define SMCHG_LID_CLOSE             14
#define SMCHG_USER_DATA             15
#define SMCHG_USER_DATA_3           16
#define SMCHG_CHG_IN                17

// richard for customer UI spec customer definition
#define DRV_CHARGER_EVENT_BATTERY_LEVEL  18
#define DRV_CHARGER_EVENT_CHARGER_STATE  19
#define DRV_CHARGER_EVENT_REVERSION_REPORT 20
#define DRV_CHARGER_EVENT_CHARGING_CURRENT_LIMIT 21
#define DRV_CHARGER_EVENT_SHIPPING_MODE_ENABLE 22
#define DRV_CHARGER_EVENT_EOC_CHECKING 23

////////////////////////////////////////////////////////////////////////////////
// FUNCTION PROTOTYPES /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#endif /* _DRV_SMART_CHARGER_H_ */

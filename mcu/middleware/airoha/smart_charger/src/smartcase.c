/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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

#include "types.h"

#include "smartcase.h"
#include "smartcharger.h"
#include "chargersmartcase.h"
#include "hal_log.h"
#include "assert.h"


#include "nvkey.h"
#include "nvkey_id_list.h"
#if defined(AIR_1WIRE_ENABLE)
#include "smchg_1wire.h"
#endif


extern HandlerData gChargerDrvHandle;


uint8_t ChgCase_GetCaseType(void)
{

#ifdef AIR_1WIRE_ENABLE
    return CASE_TYPE_SMART_1WIRE;
#else
    return CASE_TYPE_NONE;
#endif

}


void DrvCharger_SmartCase_Init(void)
{

    switch (smchg_cfg.chg_type) {
        case SMCHG_TYPE_1WIRE_2PINS:
            log_hal_msgid_info("smchg, chg_type[1wire_2pins]", 0);
#if defined(AIR_1WIRE_ENABLE)
            smchg_1wire_init();
#endif
            break;
        default:
            log_hal_msgid_info("smchg, chg_type[unknown]", 0);
            break;
    }
}

void DrvCharger_RegisterSmartCase(smartcharger_callback_t app)
{
    log_hal_msgid_info("smchg, DrvCharger_RegisterSmartCase register callback", 0);

    if (!app) {
        return;
    }
    ChargerSmartCase_SetSmartCaseHandle(app);

    if (ChgCase_GetCaseType() == CASE_TYPE_SMART_1WIRE) {
#if defined(AIR_1WIRE_ENABLE)
        smchg_1wire_pre_handle();
#endif
    } else {
        return;
    }
}

uint8_t DrvCharger_GetSmartCaseState()
{
    return ChargerSmartCase_GetSmartCaseState();
}



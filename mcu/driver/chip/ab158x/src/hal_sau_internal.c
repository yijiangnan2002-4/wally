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

#include "hal_sau.h"

#ifdef HAL_SAU_MODULE_ENABLED

#include "hal_sau_internal.h"
#include "memory_attribute.h"

#ifdef __cplusplus
extern "C" {
#endif


ATTR_ZIDATA_IN_TCM SAU_CTRL_Type g_sau_ctrl;
ATTR_ZIDATA_IN_TCM SAU_REGION_EN_Type g_sau_region_en;  //uint32_t type, each bit indicates one region,there is one risk due to ARM sau can support 256 SAU region.
ATTR_ZIDATA_IN_TCM SAU_ENTRY_Type g_sau_entry[HAL_SAU_REGION_MAX];

ATTR_TEXT_IN_TCM void sau_status_save(void)
{

}

/* restores only regions that are enabled before entering into deepsleep */
ATTR_TEXT_IN_TCM void sau_status_restore(void)
{
    SAU_REGION_EN_Type sau_region_en;
    hal_sau_region_t region;

    sau_region_en = g_sau_region_en;
    for (region = 0; region < HAL_SAU_REGION_MAX; region ++) {
        if (sau_region_en & 1) {
            /* If the region is enabled, restore the previous setting of the corresponding region*/
            SAU->RNR  = region;
            SAU->RBAR = g_sau_entry[region].sau_rbar.w;
            SAU->RLAR = g_sau_entry[region].sau_rlar.w;
        }
        sau_region_en = sau_region_en >> 1;
    }
    SAU->CTRL = g_sau_ctrl.w;
}


#ifdef __cplusplus
}
#endif

#endif /* HAL_SAU_MODULE_ENABLED */


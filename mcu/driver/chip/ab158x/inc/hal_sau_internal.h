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

#ifndef __HAL_SAU_INTERNAL_H__
#define __HAL_SAU_INTERNAL_H__
#include "hal_sau.h"

#ifdef HAL_SAU_MODULE_ENABLED


#ifdef __cplusplus
extern "C" {
#endif

typedef union {
    struct {
        uint32_t ENABLE: 1;                     /**< Enables the SAU */
        uint32_t ALLNS: 1;                      /**< All Non-secure. When SAU_CTRL.ENABLE is 0 this bit controls if the memory is marked as Non-secure or Secure.
                                                     This bit has no effect when SAU_ENABLE is 1.
                                                     Setting SAU_CTRL.ALLNS to 1 allows the security attribution of all addresses to be set by the IDAU in the system.*/
        uint32_t _reserved0: 30;                /**< Reserved bits*/
    } b;
    uint32_t w;
} SAU_CTRL_Type;                                /**< Security Attribution Unit Control Register.*/

typedef uint32_t SAU_REGION_EN_Type;

typedef union {
    struct {
        uint32_t _reserved0:  5 ;              /**< Reserved bits. */
        uint32_t ADDR:       27 ;              /**< Region base address field */
    } b;
    uint32_t w;
} SAU_RBAR_Type;                               /**< Security Attribution Unit Region Base Address Register. */

typedef union {
    struct {
        uint32_t EN:         1  ;              /**< Enable. SAU region enable.This bit reset to 0 on a Warm reset.0-SAU region is enabled,1-SAU region is disabled*/
        uint32_t NSC:        1  ;              /**< Non-secure callable. Controls whether Non-secure state is permitted to execute an SG instruction from this region..*/
        uint32_t _reserved0: 3  ;              /**< Reserved bits. */
        uint32_t LADDR:      27 ;              /**< Limit address. The upper inclusive limit of the selected SAU memory region. */
    } b;
    uint32_t w;
} SAU_RLAR_Type;                               /**< SAU Region Limit Address Register.
                                                    The SAU_RLAR defines the limit address of the SAU region selected by the SAU_RNR.*/

typedef struct {
    SAU_RBAR_Type sau_rbar;
    SAU_RLAR_Type sau_rlar;
} SAU_ENTRY_Type;


extern SAU_CTRL_Type g_sau_ctrl;
extern SAU_REGION_EN_Type g_sau_region_en;                   /*bit indicate which region is protected by SAU, debug used flag*/
extern SAU_ENTRY_Type g_sau_entry[HAL_SAU_REGION_MAX];

/* Save SAU status before entering deepsleep */
void sau_status_save(void);

/* Restore SAU status after leaving deepsleep */
void sau_status_restore(void);


#ifdef __cplusplus
}
#endif

#endif /* HAL_SAU_MODULE_ENABLED */
#endif /* __HAL_SAU_INTERNAL_H__ */


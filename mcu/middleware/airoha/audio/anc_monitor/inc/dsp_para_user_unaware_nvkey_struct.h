/* Copyright Statement:
*
* (C) 2020  Airoha Technology Corp. All rights reserved.
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
#ifndef _DSP_PARA_USER_UNAWARE_H_
#define _DSP_PARA_USER_UNAWARE_H_

#include "types.h"

/**
* @NvkeyDefine NVID_DSP_ALG_ANC_USR_UNAWARE
* @brief Parameter for DSP User Unaware algorithm
* @KeyID 0xE301
*/

typedef struct stru_dsp_user_unaware_para_s {
    //For algorithm
    U8 ENABLE;                      /**< @Value   0x01 @Desc 1 */
    U8 REVISION;                    /**< @Value   0x01 @Desc 1 */

    S16 alpha_par;                  /**< @Value   983      @Desc alpha par*/
    S32 thd;                        /**< @Value   500000 @Desc switch threshold */
    S32 switch_par;                 /**< @Value   18761   @Desc switch par */
    U16 pause_attenuation;          /**< @Value     0  @Desc Set 1 to suspend apply gain attenuation*/
    U16 _reserved0;                 /**< @Value     0  @Desc 0 */
} PACKED DSP_PARA_USER_UNAWARE_STRU;


#endif /* _DSP_PARA_USER_UNAWARE_H_ */

/*
Please sync the following files
\dsp\middleware\airoha\dspalg\user_unaware\inc\dsp_para_user_unaware_nvkey_struct.h
\mcu\middleware\airoha\audio\anc_monitor\inc\dsp_para_user_unaware_nvkey_struct.h
*/


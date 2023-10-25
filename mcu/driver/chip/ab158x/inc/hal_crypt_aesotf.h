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

/**
 * @file hal_crypt_aesotf.h
 *
 * First, Using AESOTF needs to set up region.
 * AESOTF SW, eFuse, CKDF key are supported.
 *
 */
#ifndef __CRYPT_AES_H__
#define __CRYPT_AES_H__

#include <stdint.h>
#include "hal_sec.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Define for AESOTF */
#define AESOTF_INIT_MODE    (0)
#define AESOTF_ENABLE_MODE  (1)

typedef enum {
    AESOTF_KEY_SOURCE_EFUSE_0   = 0,
    AESOTF_KEY_SOURCE_EFUSE_1   = 1,
    AESOTF_KEY_SOURCE_SW        = 2,
    AESOTF_KEY_SOURCE_HW_CKDF_0 = 3,
    AESOTF_KEY_SOURCE_HW_CKDF_1 = 4,
    AESOTF_KEY_SOURCE_MAX_NUM
} AESOTF_KEY_SOURCE;

typedef enum {
    AESOTF_REGION_0   = 0,
    AESOTF_REGION_1   = 1,
    AESOTF_REGION_2   = 2,
    AESOTF_REGION_3   = 3,
    AESOTF_REGION_4   = 4,
    AESOTF_REGION_5   = 5,
    AESOTF_REGION_6   = 6,
    AESOTF_REGION_7   = 7,
    AESOTF_REGION_ALL = 8,
    AESOTF_REGION_INIT= 9,
} AESOTF_REGION_ID;


int32_t aesotf_init(void);
int32_t aesotf_set_region(uint32_t begin_address, uint32_t end_address, uint8_t region);
extern int32_t aesotf_set_nonce_common(AESHandle_t handle, uint8_t *nonce);
int32_t aesotf_set_nonce(uint8_t *nonce);
int32_t aesotf_set_keysource(uint8_t key_src);
extern int32_t aesotf_set_key_common(AESHandle_t handle, uint8_t *key);
int32_t aesotf_set_key(uint8_t key_byte_length, uint8_t *key);
int32_t aesotf_enable(void);
int32_t aesotf_disable(void);


#ifdef __cplusplus
}
#endif

#endif /* __CRYPT_AES_H__ */


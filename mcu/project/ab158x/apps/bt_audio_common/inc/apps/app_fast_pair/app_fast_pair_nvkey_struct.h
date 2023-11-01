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

/**
 * File: app_fast_pair_nvkey_struct.h
 *
 * Description: This file defines the Nvkey struct of app_fast_pair_idle_activity.c.
 *
 */

#ifndef __APP_FAST_PAIR_NVKEY_STRUCT_H__
#define __APP_FAST_PAIR_NVKEY_STRUCT_H__

#include <stdint.h>

#define APP_FAST_PAIR_PERSONALIZED_NAME_SIZE        (128)   /* The size of maximum size of personalized name. */

/* Customer can store 10 model_set data in NVkey and there is a seleceted_set to indicate which model set is useful. */
#define APP_FAST_PAIR_MODEL_SET_MAX_NUMBER          (10)

/* NvkeyDefine NVID_APP_FAST_PAIR_CONFIGURE */
/**
 *  @brief This structure defines the data format of NVID_APP_FAST_PAIR_CONFIGURE.
 */
typedef struct {
    uint8_t     fps_enable;             /**< True means fast pair service feature is enable. */
    uint8_t     max_account;            /**< Maximum google account to support. */
    uint8_t     seleceted_set;          /**< Which model set is selected. */
    uint8_t     tx_power_available;     /**< True means set Tx power value in Adv data. */
    int8_t      tx_power_level;         /**< The value should be smaller than the Tx power value when level the device 1 meter. */
    uint8_t     component_num;          /**< The count of the components. In AWS project, it should be 3(left, right and charger case). */
    char        personalized_name[APP_FAST_PAIR_PERSONALIZED_NAME_SIZE];    /* The personalized name which is set in config tool. It's not changed when device is configured a personalized devcie by smart phone or do factory reset. */
    uint8_t     reserved[64];           /**< Reserved data. */
} PACKED app_fast_pair_nvkey_t;

/**
 *  @brief This structure defines the format of a model set. After the model is registered, Google will distribute a Model ID and Anti-Spoofing Public/Private Key Pair.
 */
typedef struct {
    uint32_t    model_id;           /**< The model id of the model. */
    uint8_t     private_key[32];    /**< The private key of the model. */
} PACKED app_fast_pair_set_t;

/* NvkeyDefine NVID_APP_FAST_PAIR_PROTECTED */
/**
 *  @brief This structure defines the format of a model stored in NVkey, it's the encrypted data from model set and app_fast_pair_nvdm_key.
 */
typedef struct {
    app_fast_pair_set_t sets[APP_FAST_PAIR_MODEL_SET_MAX_NUMBER];   /**< The encrypted data. */
    uint32_t            CRC;                                        /**< The CRC of the encrypted data. */
} PACKED app_fast_pair_protected_t;

#endif /* __APP_FAST_PAIR_NVKEY_STRUCT_H__ */

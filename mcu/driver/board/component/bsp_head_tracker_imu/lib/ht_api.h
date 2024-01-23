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

#ifndef __ADRSIM_MESSAGE_H__
#define __ADRSIM_MESSAGE_H__

#ifdef __cplusplus
    extern "C" {
#endif

/******************************************************************************
* Includes
******************************************************************************/
#include <stdint.h>

/******************************************************************************
* Preprocessor Constants
******************************************************************************/
#define ADRSIM_MESSAGE_CLASS_TABLE_CNT_MAX  (3)

/******************************************************************************
* Macros
******************************************************************************/

/******************************************************************************
* Typedefs
******************************************************************************/
typedef void (*adrsim_message_class_handle_t)(uint8_t index, uint16_t payload_length, const uint8_t *payload);

typedef enum {
    ADRSIM_MESSAGE_CLASSID_NONE        = 0x00,
    ADRSIM_MESSAGE_CLASSID_EVT         = 0x01,
    ADRSIM_MESSAGE_CLASSID_PROPRIETARY = 0x02,
    ADRSIM_MESSAGE_CLASSID_MAX         = 0xFF
} adrsim_message_classid_t;

typedef enum {
    DRSIM_MESSAGE_PROC_NONE = 0,
    DRSIM_MESSAGE_PROC_DATA = 0x01,
    DRSIM_MESSAGE_PROC_DR   = 0x02,
    DRSIM_MESSAGE_PROC_VMD  = 0x04
} drsim_message_proc_type_t;

typedef struct {
    uint8_t id;
    uint8_t out_index;
    uint16_t min_length;
} adrsim_message_content_t;

typedef struct {
    adrsim_message_class_handle_t handle;
    uint8_t id;
    uint8_t message_number;
    const adrsim_message_content_t message[];
} adrsim_message_class_t;

/******************************************************************************
* Constant Variables
******************************************************************************/

/******************************************************************************
* Variables
******************************************************************************/

/******************************************************************************
* Function Prototypes
******************************************************************************/
//void adrsim_message_init(void);
//void adrsim_message_deinit(void);
//adrsim_message_class_handle_t adrsim_message_find_handle(uint8_t *index, uint8_t class_id, uint8_t message_id, uint16_t payload_length);

void imu_api_initial(float* imu_algo_para);
void imu_api_process(int timestamp, float* accel, float* gyro);
void imu_api_get_attitude(float* att);
void imu_api_calibration(int cnt, float* accel, float* gyro);
void imu_api_get_cali_data(float* cali_config_data);
#ifdef __cplusplus
    }  /* extern "C" */
#endif

#endif /* __ADRSIM_MESSAGE_H___ */

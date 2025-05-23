/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#ifndef ATCI_ADPT_H
#define ATCI_ADPT_H
#include "atci.h"




/*--- Function ---*/
/* related with OS platform */
extern void    *atci_mem_alloc(uint32_t size);
extern void     atci_mem_free(void *buf);
extern uint32_t atci_mutex_create(void);
extern uint32_t atci_mutex_lock(uint32_t mutex_id);
extern uint32_t atci_mutex_unlock(uint32_t mutex_id);
extern uint32_t atci_queue_create(uint32_t queue_length, uint32_t item_size);
extern int32_t  atci_queue_receive_no_wait(uint32_t q_id, void *data);
int32_t atci_queue_receive_wait(uint32_t q_id, void *data, uint32_t delay_time);
extern uint16_t atci_queue_get_item_num(uint32_t q_id);
extern atci_status_t atci_queue_send(uint32_t q_id, void *data);
extern atci_status_t atci_queue_send_from_isr(uint32_t q_id, void *data);
extern uint32_t atci_semaphore_create(uint32_t uxMaxCount, uint32_t uxInitialCount);
extern atci_status_t atci_semaphore_give(uint32_t semaphore_id);
extern atci_status_t atci_semaphore_take(uint32_t semaphore_id);

/* internal use api */
extern void atci_dump_data(uint8_t *data, uint16_t len, const char *log_msg);
extern uint16_t atci_local_caculate_hash_value(uint8_t *at_name, uint32_t *hash_value1, uint32_t *hash_value2);
extern atci_status_t atci_input_command_handler(uint8_t *cmd_msg);

/**
 * @brief This function get parameter list from an AT execute command.
 * @param[in] *atcmd is the AT CMD data. For more details, please refer to #atci_parse_cmd_param_ex_t.
 * @param[out] **list_out is the parameter output array.
 * @param[in] list_cnt is the parameter output array size.
 * @return    #ATCI_STATUS_OK the ATCI module sent data to the UART successfully.
 * @return    the number of AT CMD parameters.
 */
extern uint16_t atci_get_parameter_list(atci_parse_cmd_param_t *atcmd, uint8_t **list_out, uint16_t list_cnt);



#endif

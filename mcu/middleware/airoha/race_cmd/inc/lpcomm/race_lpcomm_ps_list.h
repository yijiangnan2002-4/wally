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


#ifndef __RACE_LPCOMM_PS_LIST_H__
#define __RACE_LPCOMM_PS_LIST_H__


#include "race_cmd_feature.h"
#ifdef RACE_LPCOMM_ENABLE
#include "race_cmd.h"
#include "race_xport.h"
#include "race_util.h"


typedef struct {
    uint8_t channel_id;
    int8_t status;
    int8_t process_status;
    bool dual_cmd;
    void *tmp_result;
} race_lpcomm_process_status_info_struct, process_status_info;


typedef struct race_lpcomm_process_status_info_list {
    struct race_lpcomm_process_status_info_list *next;
    bool is_used;
    uint16_t process_id;
    process_status_info process_status;
} race_lpcomm_process_status_info_list_struct, process_status_info_list;


bool race_lpcomm_ps_list_init(void);

void race_lpcomm_ps_list_deinit(void);

process_status_info_list *race_lpcomm_ps_node_alloc(uint16_t process_id);

void race_lpcomm_ps_node_free(void *node);

RACE_ERRCODE race_lpcomm_ps_list_insert(process_status_info_list *list_node);

RACE_ERRCODE race_lpcomm_ps_list_remove(process_status_info_list *list_node);

RACE_ERRCODE race_lpcomm_ps_list_destory(void);

process_status_info_list *race_lpcomm_ps_list_node_find_by_process_id(uint16_t process_id);

#endif /* RACE_LPCOMM_ENABLE */
#endif /* __RACE_LPCOMM_PS_LIST_H__ */


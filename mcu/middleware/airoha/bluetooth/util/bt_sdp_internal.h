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
/* MediaTek restricted information */

#ifndef __BT_SDP_INTERNAL_H__
#define __BT_SDP_INTERNAL_H__

#include "bt_sdp.h"


typedef enum {
    BT_SDPC_EVENT_HANDLE_QUERY_RESULT,
    BT_SDPC_EVENT_ATTRIBUTE_QUERY_RESULT,
    BT_SDPC_EVENT_QUERY_ERROR
} bt_sdpc_event_t;


typedef struct {
    uint16_t length;
    uint8_t *data;
} bt_sdpc_attribute_query_result_t;

typedef struct {
    uint16_t handle_number;
    uint8_t *data;
} bt_sdpc_handle_query_result_t;

typedef struct {
    bt_status_t error_code;
} bt_sdpc_query_error_t;

typedef void (*bt_sdpc_query_callback)(bt_sdpc_event_t event_id, void *result, void *user_data);

typedef struct {
    bt_bd_addr_t address;               /* Remote device address */
    bt_sdpc_query_callback callback; /* Callback function to handle SDP events */
    uint16_t length;                    /* Buffer length of search_detail */
    const uint8_t *search_detail;          /* It's search pattern when search for handle, and searched attribute list when search for attribute. */
    uint32_t search_handle;          /* Only available when search for handle*/
    const void *user_data;
} bt_sdpc_request_t;


typedef const bt_sdps_record_t *(*bt_sdps_get_record)();

bt_status_t bt_sdpc_service_search(bt_sdpc_request_t *request_buffer);

bt_status_t bt_sdpc_service_attribute(bt_sdpc_request_t *request_buffer);

bt_status_t bt_sdpc_cancel_callback(const bt_bd_addr_t *address, bt_sdpc_query_callback callback);

/*register SDP Record weak functions*/

const bt_sdps_record_t *bt_sdps_get_hfp_record(void);

const bt_sdps_record_t *bt_sdps_get_hfp_ag_record(void);

const bt_sdps_record_t *bt_sdps_get_hsp_record(void);

const bt_sdps_record_t *bt_sdps_get_a2dp_src_record(void);

const bt_sdps_record_t *bt_sdps_get_a2dp_snk_record(void);

const bt_sdps_record_t *bt_sdps_get_avrcp_CT_record(void);

const bt_sdps_record_t *bt_sdps_get_avrcp_TG_record(void);

const bt_sdps_record_t *bt_sdps_get_sdp_record();

const bt_sdps_record_t *bt_sdps_get_map_record(void);

#endif/*__BT_SDP_INTERNAL_H__*/

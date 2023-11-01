/* Copyright Statement:
 *
 * (C) 2022  Airoha Technology Corp. All rights reserved.
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

#ifndef __BT_SOURCE_SRV_DM_H__
#define __BT_SOURCE_SRV_DM_H__
#include "bt_source_srv_music_internal.h"
#include "bt_source_srv_utils.h"
#include <syslog.h>
#ifdef MTK_BT_CM_SUPPORT
#include "bt_connection_manager.h"
#endif

#include "FreeRTOSConfig.h"



#ifdef __cplusplus
extern "C" {
#endif

#define __BT_SOURCE_SRV_DEBUG_INFO__
#define CMD_PARAM(s) s, bt_service_strlen(s)
#ifdef __BT_SOURCE_SRV_DEBUG_INFO__
#define bt_source_srv_report(_message,...) LOG_I(source_srv, (_message), ##__VA_ARGS__)
#define bt_source_srv_report_id(_message, art_cnt,...) LOG_MSGID_I(source_srv, _message, art_cnt, ##__VA_ARGS__)
#define bt_source_srv_a2dp_report_id(_message, art_cnt,...) LOG_MSGID_I(a2dp_srv, _message, art_cnt, ##__VA_ARGS__)
#define bt_source_srv_avrcp_report_id(_message, art_cnt,...) LOG_MSGID_I(avrcp_srv, _message, art_cnt, ##__VA_ARGS__)

#else
#define bt_sink_srv_report(_message,...);
#define bt_sink_srv_report_id(_message, art_cnt,...);
#endif

void bt_source_srv_music_cntx_init(void);
void bt_source_srv_music_cntx_set_bqb_flag(void);
void bt_source_srv_music_clean_a2dp_conn_info(bt_source_srv_music_device_t *dev);
void bt_source_srv_music_clean_avrcp_conn_info(bt_source_srv_music_device_t *dev);
bt_source_srv_music_device_t* bt_source_srv_music_get_device(bt_srv_music_device_type_t type, void *param);
bt_audio_srv_context_t * bt_source_srv_music_get_context(void);

#ifdef __cplusplus
}
#endif
#endif



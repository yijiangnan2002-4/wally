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

#include "bt_source_srv_device_manager.h"



static bt_audio_srv_context_t g_source_srv_device_cntx;//[BT_SOURCE]

bt_audio_srv_context_t * bt_source_srv_music_get_context()
{
    return &g_source_srv_device_cntx;
}

void bt_source_srv_music_cntx_init(void)
{
    uint32_t i = 0;
    memset(&g_source_srv_device_cntx, 0x00, sizeof(bt_audio_srv_context_t));
    for (i = 0; i < BT_SOURCE_SRV_MAX_DEVICE_NUMBER; ++i) {
        memset((void *)(&(g_source_srv_device_cntx.source_dev[i].dev_addr)), 0x00, sizeof(bt_bd_addr_t));
        memset((void *)(&(g_source_srv_device_cntx.source_dev[i].capabilty)), 0x00, sizeof(bt_a2dp_codec_capability_t));
        g_source_srv_device_cntx.source_dev[i].a2dp_state= BT_SOURCE_SRV_STATE_IDLE;
        g_source_srv_device_cntx.source_dev[i].a2dp_hd = BT_SOURCE_SRV_INVAILD_HANDLE;
        //g_source_srv_device_cntx.source_dev[i].a2dp_role = BT_A2DP_INVALID_ROLE;
        g_source_srv_device_cntx.source_dev[i].avrcp_hd = BT_SOURCE_SRV_INVAILD_HANDLE;
        g_source_srv_device_cntx.source_dev[i].avrcp_role = BT_AVRCP_ROLE_UNDEF;
        g_source_srv_device_cntx.source_dev[i].avrcp_state = BT_SOURCE_SRV_STATE_IDLE;
        g_source_srv_device_cntx.source_dev[i].avrcp_play_status = BT_AVRCP_EVENT_MEDIA_PLAY_STOPPED;
        g_source_srv_device_cntx.source_dev[i].audio_dev = NULL;
        g_source_srv_device_cntx.source_dev[i].last_cmd = BT_SOURCE_SRV_A2DP_INVAILD_LAST_CMD;
        g_source_srv_device_cntx.source_dev[i].flag = 0;
        bt_source_srv_report_id("bt_source_srv_cntx_init: i = %x, dev = 0x%x, dev->handle: %x", 3, i, &g_source_srv_device_cntx.source_dev[i], g_source_srv_device_cntx.source_dev[i].a2dp_hd);
    }
}

void bt_source_srv_music_cntx_set_bqb_flag()
{
    g_source_srv_device_cntx.bqb_flag = true;
}

bt_source_srv_music_device_t* bt_source_srv_music_get_device(bt_srv_music_device_type_t type, void *param)
{
    bt_source_srv_music_device_t *dev = NULL;
    bt_audio_srv_context_t *cntx = bt_source_srv_music_get_context();
    bt_bd_addr_t *dev_addr = NULL;
    int32_t i = 0;
    uint32_t *p_hd = NULL;
    switch (type) {
        case BT_SRV_MUSIC_DEVICE_ADDR_A2DP: {
            dev_addr = (bt_bd_addr_t *)param;

            for (i = 0; i < BT_SOURCE_SRV_MAX_DEVICE_NUMBER; ++i) {

                if ((cntx->source_dev[i].a2dp_hd != BT_SOURCE_SRV_INVAILD_HANDLE) && (memcmp(dev_addr, (cntx->source_dev[i].dev_addr), sizeof(bt_bd_addr_t)) == 0)) {
                    dev = &(cntx->source_dev[i]);
                    break;
                }
            }
            break;
        }
        case BT_SRV_MUSIC_DEVICE_UNUSED: {
            for (i = 0; i < BT_SOURCE_SRV_MAX_DEVICE_NUMBER; ++i) {
                if (cntx->source_dev[i].a2dp_hd == BT_SOURCE_SRV_INVAILD_HANDLE) {
                    dev = &(cntx->source_dev[i]);
                    break;
                }
            }
            break;
        }
        case BT_SRV_MUSIC_DEVICE_A2DP_HD: {
            p_hd = (uint32_t *)param;
            for (i = 0; i < BT_SOURCE_SRV_MAX_DEVICE_NUMBER; ++i) {
                if (cntx->source_dev[i].a2dp_hd == (*p_hd)) {
                   dev = &(cntx->source_dev[i]);
                   break;
                }
            }
            break;
        }
        case BT_SRV_MUSIC_DEVICE_AVRCP_HD: {
            p_hd = (uint32_t *)param;
            for (i = 0; i < BT_SOURCE_SRV_MAX_DEVICE_NUMBER; ++i) {
                if (cntx->source_dev[i].avrcp_hd == (*p_hd)) {
                   dev = &(cntx->source_dev[i]);
                   break;
                }
            }
        }
        case BT_SRV_MUSIC_DEVICE_ADDR_AVRCP: {
            dev_addr = (bt_bd_addr_t *)param;

            for (i = 0; i < BT_SOURCE_SRV_MAX_DEVICE_NUMBER; ++i) {
                if ((cntx->source_dev[i].avrcp_hd != BT_SOURCE_SRV_INVAILD_HANDLE) &&
                    (memcmp(dev_addr, &(cntx->source_dev[i].dev_addr), sizeof(bt_bd_addr_t)) == 0)) {
                    dev = &(cntx->source_dev[i]);
                    break;
                }
            }
            break;
        }
        case BT_SRV_MUSIC_DEVICE_AUDIO_RESOURCE_DEVICE: {
            uint32_t i = 0;
            for (i = 0; i < BT_SOURCE_SRV_MAX_DEVICE_NUMBER; i++) {
                if ((cntx->source_dev[i].a2dp_hd != BT_SOURCE_SRV_INVAILD_HANDLE) && (cntx->source_dev[i].audio_dev == param)) {
                    LOG_MSGID_I(source_srv, "[MUSIC_SOUR]resourc: find context = %02x by device = %02x", 2, cntx, param);
                    dev = &(cntx->source_dev[i]);
                    break;
                }
            }
        break;
        }
        case BT_SRV_MUSIC_DEVICE_HIGHLIGHT: {
            for (i = 0; i < BT_SOURCE_SRV_MAX_DEVICE_NUMBER; ++i) {
                LOG_MSGID_I(source_srv, "[MUSIC_SOUR]hightlight: find context = %02x by device = %02x", 2, cntx, cntx->source_dev[i].a2dp_state);

                if ((cntx->source_dev[i].a2dp_hd!= BT_SOURCE_SRV_INVAILD_HANDLE) && (cntx->source_dev[i].a2dp_state == BT_SOURCE_SRV_STATE_STREAMING)) {
                    dev = &(cntx->source_dev[i]);//to highlight
                }
            }
            break;
        }
        case BT_SRV_MUSIC_DEVICE_USED: {
            for (i = 0; i < BT_SOURCE_SRV_MAX_DEVICE_NUMBER; ++i) {
                if (g_source_srv_device_cntx.bqb_flag && ((cntx->source_dev[i].a2dp_hd!= BT_SOURCE_SRV_INVAILD_HANDLE) || (cntx->source_dev[i].avrcp_hd != BT_SOURCE_SRV_INVAILD_HANDLE))) {
                    dev = &(cntx->source_dev[i]);
                }
            }
            break;
        }
       case BT_SRV_MUSIC_DEVICE_VAILD_DEVICE: {
            for (i = 0; i < BT_SOURCE_SRV_MAX_DEVICE_NUMBER; ++i) {
                if ((cntx->source_dev[i].a2dp_hd!= BT_SOURCE_SRV_INVAILD_HANDLE) || (cntx->source_dev[i].avrcp_hd != BT_SOURCE_SRV_INVAILD_HANDLE)) {
                   dev = &(cntx->source_dev[i]);
                }
            }
            break;
       }
        default:
            break;
    }
    bt_source_srv_report_id("music_get_device, type:%x,dev: %x", 2, type, dev);

    return dev;
}


void bt_source_srv_music_clean_a2dp_conn_info(bt_source_srv_music_device_t *dev)
{
    dev->a2dp_hd = BT_SOURCE_SRV_INVAILD_HANDLE;
    bt_source_srv_report_id("clean_a2dp_conn_info: dev = 0x%x, a2dp_hd: %x", 2, dev, dev->a2dp_hd);
    dev->a2dp_state = BT_SOURCE_SRV_STATE_IDLE;
    dev->sub_state = BT_SOURCE_SRV_STATE_IDLE;
    dev->detect_flag = 0;
    dev->max_mtu = 0;
    memset(&(dev->capabilty), 0x00, sizeof(bt_a2dp_codec_capability_t));
}


void bt_source_srv_music_clean_avrcp_conn_info(bt_source_srv_music_device_t *dev)
{
    bt_source_srv_report_id("clean_avrcp_conn_info: dev = 0x%x, avrcp_hd: %x", 2, dev, dev->avrcp_hd);

    dev->avrcp_hd = BT_SOURCE_SRV_INVAILD_HANDLE;
    if (dev->a2dp_hd == BT_SOURCE_SRV_INVAILD_HANDLE) {
       memset(&(dev->dev_addr),0x00, sizeof(bt_bd_addr_t));
    }
    dev->avrcp_state = BT_SOURCE_SRV_STATE_IDLE;
    dev->avrcp_play_status = BT_AVRCP_EVENT_MEDIA_PLAY_STOPPED;
    dev->avrcp_role = BT_AVRCP_ROLE_UNDEF;
    dev->last_cmd = BT_SOURCE_SRV_A2DP_INVAILD_LAST_CMD;
    dev->flag = 0;
    dev->absolute_support = true;
}


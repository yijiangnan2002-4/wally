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

#ifndef __HAL_CAPTOUCH_NVKEY_STRUCT_H__
#define __HAL_CAPTOUCH_NVKEY_STRUCT_H__

#include <stdint.h>

#define PACKED __attribute__((packed))

typedef struct {
    bool hw_mode_en;
    uint8_t emavg_bitmap;
    bool ein_wake_en;
    bool eoff_wake_en;
    uint8_t emavg[4];
}PACKED captouch_ear_det_t;

typedef struct {
    bool hw_mode_en;
    uint8_t ch_bit_en;
    bool timer_ms_en;
    uint16_t timer_r;
    uint16_t timer_f;
}PACKED captouch_deb_t;

/* NvkeyDefine NVID_CPT_CALI_4CHDATA_EXT */
typedef struct {
    captouch_ear_det_t captouchEarDetetion;
    captouch_deb_t captouchDebounce;
    uint8_t ch_cal_init_bit_en;
    uint8_t cal_init_val[4];
    uint8_t ear_detect_bit_mask;
    uint8_t reserved[32];
}PACKED hal_captouch_nvdm_data_ext;

/* NvkeyDefine NVID_CPT_CALI_4CHDATA */
typedef struct {
    bool     is_key[4];
    bool     is_rs[4];
    bool     is_thr[4];
    uint8_t  hw_ch_map;
    uint8_t  mavg_r[4];
    uint8_t  avg_s[4];
    uint8_t  en_ch_map;
    uint8_t  coarse_cap[4];
    int16_t  thr_h[4];
    int16_t  thr_l[4];
    int8_t   fine_cap[4];
    uint8_t  swtune_en;
    uint8_t  swDebounceTime;
    uint8_t  reserve[2];
}PACKED hal_captouch_nvdm_data;

/* NvkeyDefine NVID_CPT_SDWU_DATA */
typedef struct
{
    uint8_t sdwu_en;
    uint8_t sdtime;
    uint8_t wutime;
}PACKED hal_captouch_sdwu_nvdm_data;

/* NvkeyDefine NVID_CPT_FEATURE */
typedef struct 
{
    uint8_t  IsCapTouchEnable;
}PACKED hal_captouch_feature_nvdm_data;

/* NvkeyDefine NVID_CPT_EARDETECT_DATA */
typedef struct
{
    uint8_t  detect_ch;
    int16_t  earCheckBase[4];
    uint8_t  BaseAvgNum;
    uint8_t  baseKFrozeTime;
    uint8_t  reserved[5];
}PACKED hal_captouch_eardetect_nvdm_data;

/* NvkeyDefine NVID_CPT_FORCETOUCH_DATA */
typedef struct
{
    uint8_t enable_ch;
    int16_t pre_thr;
    int16_t rel_thr;
    int16_t lev_thr;
    int16_t cba_thr;
    int16_t ini_delay;
    uint8_t  reserved;
}PACKED hal_captouch_forcetouch_nvdm_data;

/* NvkeyDefine NVID_CPT_TUNE_DATA */
typedef struct
{
    uint8_t coarseCapValue[4];
    int8_t  fineCap[4];
    int16_t earCheckBase[4];
}PACKED hal_captouch_tune_nvdm_data;

/* NvkeyDefine NVID_CPT_AUTOSUSPEND_DATA */
typedef struct
{
    uint8_t  lpsd_ch_bitmap;
    uint8_t  ch_bitmap;
    uint8_t  lpwu_ch_bitmap;
    uint8_t  time[4];
    uint8_t  reserved[1];
}PACKED hal_captouch_autosuspend_timeout_nvdm_data;

#endif /* __HAL_CAPTOUCH_NVKEY_STRUCT_H__ */

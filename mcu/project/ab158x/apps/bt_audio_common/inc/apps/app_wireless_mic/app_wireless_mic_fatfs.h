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

/**
 * File: app_wireless_mic_fatfs.h
 *
 * Description: This file defines the interface of app_wireless_mic_fatfs.c.
 */

#ifndef __APP_WIRELESS_MIC_FATFS_H__
#define __APP_WIRELESS_MIC_FATFS_H__

#ifdef AIR_WIRELESS_MIC_ENABLE
#include "ui_shell_activity.h"
#include "ff.h"

#include "FreeRTOS.h"
#include "semphr.h"

#ifndef PACKED
#define PACKED  __attribute__((packed))
#endif

#define APP_WIRELESS_MIC_LOCAL_RECORDER_SAMPLE_RATE     48000  /* HZ */
#define APP_WIRELESS_MIC_LOCAL_RECORDER_FRAME_INTERVAL  5000   /* us */

typedef struct {
    uint32_t       ChunkID;          /**< "RIFF"*/
    uint32_t       ChunkSize;        /**< 36 + SubChunk2Size*/
    uint32_t       Format;           /**< "WAVE"*/
} PACKED app_wm_wav_riff_t;

typedef struct {
    uint32_t       ChunkID;          /**< "fmt"*/
    uint32_t       ChunkSize;        /**< 16 for PCM.*/
    uint16_t       AudioFormat;      /**< PCM = 1.*/
    uint16_t       NumOfChannels;    /**<  Mono = 1, Stereo = 2*/
    uint32_t       SampleRate;       /**< Sample rate, 8000, 44100, 16000, etc*/
    uint32_t       ByteRate;         /**< SampleRate * NumChannels * BitsPerSample/8 */
    uint16_t       BlockAlign;       /**< NumChannels * BitsPerSample/8*/
    uint16_t       BitsPerSample;    /**< 8 bits = 8, 16 bits = 16, etc.*/
} PACKED app_wm_wav_fmt_t;

typedef struct {
    uint32_t       ChunkID;          /**< "data"*/
    uint32_t       ChunkSize;        /**< NumSamples * NumChannels * BitsPerSample/8*/
} PACKED app_wm_wav_data_t;

typedef struct {
    app_wm_wav_riff_t  riff;
    app_wm_wav_fmt_t   fmt;
    app_wm_wav_data_t  data;
} PACKED app_wm_wav_header_t;

void app_wireless_mic_fatfs_get_file_num(void);
void app_wireless_mic_fatfs_save_file_num(void);
FRESULT app_wireless_mic_fatfs_write_data_test(uint8_t *data, uint32_t len);
FRESULT app_wireless_mic_fatfs_write_data_test2(bool is_first_open, uint8_t *data, uint32_t len);
void app_wireless_mic_fatfs_create_wav_header(app_wm_wav_header_t *wavhead);
bool app_wireless_mic_fatfs_get_wav_file_status(void);
FRESULT app_wireless_mic_fatfs_creat_wav_file(void);
FRESULT app_wireless_mic_fatfs_close_wav_file(void);
FRESULT app_wireless_mic_fatfs_write_wav_data(uint8_t *data, uint32_t len);
void app_wireless_mic_fatfs_init(void);
void app_wireless_mic_fatfs_update_free_size(void);


#endif
#endif /* __APP_WIRELESS_MIC_FATFS_H__ */

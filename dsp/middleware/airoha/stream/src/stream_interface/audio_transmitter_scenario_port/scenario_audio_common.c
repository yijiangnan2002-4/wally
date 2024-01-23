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

#include "scenario_audio_common.h"

ATTR_TEXT_IN_IRAM void ShareBufferCopy_I_24bit_to_D_32bit_1ch(uint8_t* src_buf, uint32_t* dest_buf, uint32_t samples)
{
    uint32_t i, left_sample, data0, data1, data2;

    for (i = 0; i < samples / 4; i++) {
        data0 = *((uint32_t *)src_buf+i*3+0);
        data1 = *((uint32_t *)src_buf+i*3+1);
        data2 = *((uint32_t *)src_buf+i*3+2);
        dest_buf[i*4] = ((data0&0x00ffffff)<<8);
        dest_buf[i*4+1] = ((data1&0x0000ffff)<<16)|((data0&0xff000000)>>16);
        dest_buf[i*4+2] = ((data2&0x000000ff)<<24)|((data1&0xffff0000)>>8);
        dest_buf[i*4+3] = ((data2&0xffffff00));
    }
    left_sample = samples % 4;
    if (left_sample == 1) {
        data0 = *((uint32_t *)src_buf+i*3+0);
        dest_buf[i*4] = ((data0&0x00ffffff)<<8);
    } else if (left_sample == 2) {
        data0 = *((uint32_t *)src_buf+i*3+0);
        data1 = *((uint32_t *)src_buf+i*3+1);
        dest_buf[i*4] = ((data0&0x00ffffff)<<8);
        dest_buf[i*4+1] = ((data1&0x0000ffff)<<16)|((data0&0xff000000)>>16);
    } else if (left_sample == 3) {
        data0 = *((uint32_t *)src_buf+i*3+0);
        data1 = *((uint32_t *)src_buf+i*3+1);
        data2 = *((uint32_t *)src_buf+i*3+2);
        dest_buf[i*4] = ((data0&0x00ffffff)<<8);
        dest_buf[i*4+1] = ((data1&0x0000ffff)<<16)|((data0&0xff000000)>>16);
        dest_buf[i*4+2] = ((data2&0x000000ff)<<24)|((data1&0xffff0000)>>8);
    }
}

ATTR_TEXT_IN_IRAM_LEVEL_1 void ShareBufferCopy_I_16bit_to_D_16bit_2ch(uint32_t *src_buf, uint16_t *dest_buf1, uint16_t *dest_buf2, uint32_t samples)
{
    uint32_t data;
    uint32_t i;

    for (i = 0; i < samples; i++) {
        data = src_buf[i];
        dest_buf1[i] = (uint16_t)(data >> 0);
        dest_buf2[i] = (uint16_t)(data >> 16);
    }
}

ATTR_TEXT_IN_IRAM void ShareBufferCopy_I_16bit_to_D_32bit_2ch(uint32_t* src_buf, uint32_t* dest_buf1, uint32_t* dest_buf2, uint32_t samples)
{
    uint32_t data;
    uint32_t i;

    for (i = 0; i < samples; i++)
    {
        data = src_buf[i];
        dest_buf1[i] = ((uint32_t)((uint16_t)(data>> 0)))<<16;
        dest_buf2[i] = ((uint32_t)((uint16_t)(data>>16)))<<16;
    }
}

ATTR_TEXT_IN_IRAM void ShareBufferCopy_I_24bit_to_D_16bit_2ch(uint8_t* src_buf, uint16_t* dest_buf1, uint16_t* dest_buf2, uint32_t samples)
{
    uint32_t data0;
    uint32_t data1;
    uint32_t data2;
    uint32_t i;

    for (i = 0; i < (samples/2); i++)
    {
        data0 = *((uint32_t *)src_buf+i*3+0);
        data1 = *((uint32_t *)src_buf+i*3+1);
        data2 = *((uint32_t *)src_buf+i*3+2);
        dest_buf1[i*2]   = (uint16_t)((data0&0x00ffff00)>>8);
        dest_buf2[i*2]   = (uint16_t)((data1&0x0000ffff));
        dest_buf1[i*2+1] = (uint16_t)(((data2&0x000000ff)<<8)|((data1&0xff000000)>>24));
        dest_buf2[i*2+1] = (uint16_t)((data2&0xffff0000)>>16);
    }
}

ATTR_TEXT_IN_IRAM void ShareBufferCopy_I_24bit_to_D_32bit_2ch(uint8_t* src_buf, uint32_t* dest_buf1, uint32_t* dest_buf2, uint32_t samples)
{
    uint32_t data0;
    uint32_t data1;
    uint32_t data2;
    uint32_t i;

    for (i = 0; i < (samples/2); i++)
    {
        data0 = *((uint32_t *)src_buf+i*3+0);
        data1 = *((uint32_t *)src_buf+i*3+1);
        data2 = *((uint32_t *)src_buf+i*3+2);
        dest_buf1[i*2]   = ((data0&0x00ffffff)<<8);
        dest_buf2[i*2]   = ((data1&0x0000ffff)<<16)|((data0&0xff000000)>>16);
        dest_buf1[i*2+1] = ((data2&0x000000ff)<<24)|((data1&0xffff0000)>>8);
        dest_buf2[i*2+1] = ((data2&0xffffff00));
    }
}

ATTR_TEXT_IN_IRAM void ShareBufferCopy_I_16bit_to_D_32bit_8ch(uint32_t* src_buf, uint32_t* dest_buf1, uint32_t* dest_buf2, uint32_t* dest_buf3, uint32_t* dest_buf4, uint32_t* dest_buf5, uint32_t* dest_buf6, uint32_t* dest_buf7, uint32_t* dest_buf8, uint32_t samples)
{
    uint32_t data0;
    uint32_t data1;
    uint32_t data2;
    uint32_t data3;
    uint32_t i;

    for (i = 0; i < samples; i++)
    {
        data0 = src_buf[4*i + 0];
        data1 = src_buf[4*i + 1];
        data2 = src_buf[4*i + 2];
        data3 = src_buf[4*i + 3];
        dest_buf1[i] = ((uint32_t)((uint16_t)(data0>> 0)))<<16;
        dest_buf2[i] = ((uint32_t)((uint16_t)(data0>>16)))<<16;
        dest_buf3[i] = ((uint32_t)((uint16_t)(data1>> 0)))<<16;
        dest_buf4[i] = ((uint32_t)((uint16_t)(data1>>16)))<<16;
        dest_buf5[i] = ((uint32_t)((uint16_t)(data2>> 0)))<<16;
        dest_buf6[i] = ((uint32_t)((uint16_t)(data2>>16)))<<16;
        dest_buf7[i] = ((uint32_t)((uint16_t)(data3>> 0)))<<16;
        dest_buf8[i] = ((uint32_t)((uint16_t)(data3>>16)))<<16;
    }
}

ATTR_TEXT_IN_IRAM void ShareBufferCopy_I_16bit_to_D_16bit_8ch(uint32_t* src_buf, uint16_t* dest_buf1, uint16_t* dest_buf2, uint16_t* dest_buf3, uint16_t* dest_buf4, uint16_t* dest_buf5, uint16_t* dest_buf6, uint16_t* dest_buf7, uint16_t* dest_buf8, uint32_t samples)
{
    uint32_t data0;
    uint32_t data1;
    uint32_t data2;
    uint32_t data3;
    uint32_t i;

    for (i = 0; i < samples; i++)
    {
        data0 = src_buf[4*i + 0];
        data1 = src_buf[4*i + 1];
        data2 = src_buf[4*i + 2];
        data3 = src_buf[4*i + 3];
        dest_buf1[i] = ((uint16_t)(data0>> 0));
        dest_buf2[i] = ((uint16_t)(data0>>16));
        dest_buf3[i] = ((uint16_t)(data1>> 0));
        dest_buf4[i] = ((uint16_t)(data1>>16));
        dest_buf5[i] = ((uint16_t)(data2>> 0));
        dest_buf6[i] = ((uint16_t)(data2>>16));
        dest_buf7[i] = ((uint16_t)(data3>> 0));
        dest_buf8[i] = ((uint16_t)(data3>>16));
    }
}

ATTR_TEXT_IN_IRAM void ShareBufferCopy_I_24bit_to_D_32bit_8ch(uint32_t* src_buf, uint32_t* dest_buf1, uint32_t* dest_buf2, uint32_t* dest_buf3, uint32_t* dest_buf4, uint32_t* dest_buf5, uint32_t* dest_buf6, uint32_t* dest_buf7, uint32_t* dest_buf8, uint32_t samples)
{
    uint32_t data0;
    uint32_t data1;
    uint32_t data2;
    uint32_t data3;
    uint32_t data4;
    uint32_t data5;
    uint32_t i;

    for (i = 0; i < samples; i++)
    {
        data0 = src_buf[6*i + 0];
        data1 = src_buf[6*i + 1];
        data2 = src_buf[6*i + 2];
        data3 = src_buf[6*i + 3];
        data4 = src_buf[6*i + 4];
        data5 = src_buf[6*i + 5];
        dest_buf1[i] = (data0&0x00ffffff)<<8;
        dest_buf2[i] = ((data1&0x0000ffff)<<16)|((data0&0xff000000)>>16);
        dest_buf3[i] = ((data2&0x000000ff)<<24)|((data1&0xffff0000)>>8);
        dest_buf4[i] = (data2&0xffffff00);
        dest_buf5[i] = (data3&0x00ffffff)<<8;
        dest_buf6[i] = ((data4&0x0000ffff)<<16)|((data3&0xff000000)>>16);
        dest_buf7[i] = ((data5&0x000000ff)<<24)|((data4&0xffff0000)>>8);
        dest_buf8[i] = (data5&0xffffff00);
    }
}

ATTR_TEXT_IN_IRAM void ShareBufferCopy_I_24bit_to_D_16bit_8ch(uint32_t* src_buf, uint16_t* dest_buf1, uint16_t* dest_buf2, uint16_t* dest_buf3, uint16_t* dest_buf4, uint16_t* dest_buf5, uint16_t* dest_buf6, uint16_t* dest_buf7, uint16_t* dest_buf8, uint32_t samples)
{
    uint32_t data0;
    uint32_t data1;
    uint32_t data2;
    uint32_t data3;
    uint32_t data4;
    uint32_t data5;
    uint32_t i;

    for (i = 0; i < samples; i++)
    {
        data0 = src_buf[6*i + 0];
        data1 = src_buf[6*i + 1];
        data2 = src_buf[6*i + 2];
        data3 = src_buf[6*i + 3];
        data4 = src_buf[6*i + 4];
        data5 = src_buf[6*i + 5];
        dest_buf1[i] = (uint16_t)((data0&0x00ffff00)>>8);
        dest_buf2[i] = (uint16_t)((data1&0x0000ffff));
        dest_buf3[i] = (uint16_t)(((data2&0x000000ff)<<8)|((data1&0xff000000)>>24));
        dest_buf4[i] = (uint16_t)((data2&0xffff0000)>>16);
        dest_buf5[i] = (uint16_t)((data3&0x00ffff00)>>8);
        dest_buf6[i] = (uint16_t)((data4&0x0000ffff)<<16);
        dest_buf7[i] = (uint16_t)(((data5&0x000000ff)<<8)|((data4&0xff000000)>>24));
        dest_buf8[i] = (uint16_t)((data5&0xffff0000)>>16);
    }
}

void ShareBufferCopy_D_32bit_to_D_24bit_1ch(uint32_t* src_buf, uint8_t* dest_buf1, uint32_t samples)
{
    uint32_t data;
    uint32_t i;

    for (i = 0; i < samples; i++)
    {
        data = src_buf[i];
        *(dest_buf1+i*3+0) = (uint8_t)((data>> 8)&0xff);
        *(dest_buf1+i*3+1) = (uint8_t)((data>>16)&0xff);
        *(dest_buf1+i*3+2) = (uint8_t)((data>>24)&0xff);
    }
}

void ShareBufferCopy_D_32bit_to_I_24bit_2ch(uint32_t* src_buf1, uint32_t* src_buf2, uint8_t* dest_buf1, uint32_t samples)
{
    uint32_t data1;
    uint32_t data2;
    uint32_t i;

    for (i = 0; i < samples; i++)
    {
        data1 = src_buf1[i];
        data2 = src_buf2[i];
        *(dest_buf1+i*6+0) = (uint8_t)((data1>> 8)&0xff);
        *(dest_buf1+i*6+1) = (uint8_t)((data1>>16)&0xff);
        *(dest_buf1+i*6+2) = (uint8_t)((data1>>24)&0xff);
        *(dest_buf1+i*6+3) = (uint8_t)((data2>> 8)&0xff);
        *(dest_buf1+i*6+4) = (uint8_t)((data2>>16)&0xff);
        *(dest_buf1+i*6+5) = (uint8_t)((data2>>24)&0xff);
    }
}

void ShareBufferCopy_D_16bit_to_D_16bit_1ch(uint16_t* src_buf, uint16_t* dest_buf1, uint32_t samples)
{
    uint16_t data;
    uint32_t i;

    for (i = 0; i < samples; i++)
    {
        data = src_buf[i];
        *(dest_buf1+i) = data;
    }
}

void ShareBufferCopy_D_16bit_to_I_16bit_2ch(uint16_t* src_buf1, uint16_t* src_buf2, uint16_t* dest_buf1, uint32_t samples)
{
    uint32_t data1;
    uint32_t data2;
    uint32_t i;

    for (i = 0; i < samples; i++)
    {
        data1 = src_buf1[i];
        data2 = src_buf2[i];
        data1 = (data1) | (data2 << 16);
        *((uint32_t* )dest_buf1+i) = data1;
    }
}

void ShareBufferCopy_D_32bit_to_D_16bit_1ch(uint32_t* src_buf, uint16_t* dest_buf1, uint32_t samples)
{
    uint32_t data;
    uint32_t i;

    for (i = 0; i < samples; i++)
    {
        data = src_buf[i];
        *(dest_buf1+i) = (uint16_t)((data >> 16)&0xffff);
    }
}

void ShareBufferCopy_D_32bit_to_I_16bit_2ch(uint32_t* src_buf1, uint32_t* src_buf2, uint16_t* dest_buf1, uint32_t samples)
{
    uint32_t data1;
    uint32_t data2;
    uint32_t i;

    for (i = 0; i < samples; i++)
    {
        data1 = src_buf1[i];
        data2 = src_buf2[i];
        data1 = ((data1 >> 16)&0x0000ffff) | ((data2 >> 0)&0xffff0000);
        *((uint32_t* )dest_buf1+i) = data1;
    }
}

void ShareBufferCopy_D_16bit_to_I_24bit_2ch(uint16_t* src_buf1, uint16_t* src_buf2, uint8_t* dest_buf1, uint32_t samples)
{
    uint32_t data32;
    uint16_t data16;
    uint32_t i;

    for (i = 0; i < samples; i++)
    {
        if ((i%2) == 0)
        {
            data32 = (src_buf1[i]<<8); // 0x00XXXX00
            data16 = src_buf2[i]; //0xXXXX
            *(uint32_t *)(dest_buf1 + i*6) = data32;
            *(uint16_t *)(dest_buf1 + i*6 + 4) = data16;
        }
        else
        {
            data16 = (src_buf1[i]&0x00ff)<<8; //0xXX00
            data32 = (src_buf2[i]<<16) | ((src_buf1[i]&0xff00)>>8); // 0xXXXX00XX
            *(uint16_t *)(dest_buf1 + i*6) = data16;
            *(uint32_t *)(dest_buf1 + i*6 + 2) = data32;
        }
    }
}

void ShareBufferCopy_D_16bit_to_D_24bit_1ch(uint16_t* src_buf1, uint8_t* dest_buf1, uint32_t samples)
{
    uint32_t data32;
    uint16_t data16;
    uint32_t i;

    for (i = 0; i < samples; i++)
    {
        if ((i%2) == 0)
        {
            if (i != (samples - 1))
            {
                data32 = (src_buf1[i]<<8); // 0x00XXXX00
                *(uint32_t *)(dest_buf1 + (i/2)*6) = data32;
            }
            else
            {
                /* prevent overflow */
                data32 = (src_buf1[i]<<8); // 0x00XXXX00
                *(uint16_t *)(dest_buf1 + (i/2)*6) = (uint16_t)(data32&0x0000ffff);
                *(uint8_t *)(dest_buf1 + (i/2)*6 + 2) = (uint8_t)((data32&0x00ff0000)>>16);
            }
        }
        else
        {
            data16 = src_buf1[i]; //0xXXXX
            *(uint16_t *)(dest_buf1 + (i/2)*6 + 4) = data16;
        }
    }
}

void ShareBufferCopy_D_16bit_to_D_32bit_1ch(uint16_t* src_buf, uint32_t* dest_buf, uint32_t samples)
{
    uint32_t data;
    uint32_t i;

    for (i = 0; i < samples; i++)
    {
        data = src_buf[i];
        *(dest_buf+i) = data<<16;
    }
}

void ShareBufferCopy_D_16bit_to_I_24bit_1ch(uint16_t* src_buf1, uint8_t* dest_buf1, uint32_t samples)
{
    uint32_t data32;
    uint32_t i, j;

    j = 0;
    for (i = 0; i < samples; i++) {
        if ((i % 4) == 0) {
            data32 = src_buf1[i] << 8; // 0x00XXXX00
            *(uint32_t *)(dest_buf1 + j * 12) = data32;
        } else if ((i % 4) == 1) {
            data32 = src_buf1[i]; //0x0000XXXX
            *(uint32_t *)(dest_buf1 + j * 12 + 4) = data32;
        } else if ((i % 4) == 2) {
            data32 = (src_buf1[i] & 0x00ff) << 24; // 0xXX000000
            *(uint32_t *)(dest_buf1 + j * 12 + 4) |= data32;
            data32 = (src_buf1[i] & 0xff00) >> 8;
            *(uint32_t *)(dest_buf1 + j * 12 + 8) = data32;
        } else {
            data32 = src_buf1[i] << 16; // 0xXXXX0000
            *(uint32_t *)(dest_buf1 + j * 12 + 8) |= data32;
            j++;
        }
    }
}

#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE) || defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE) || defined(AIR_GAMING_MODE_DONGLE_ENABLE) || defined(AIR_BT_AUDIO_DONGLE_ENABLE) || defined (AIR_ULL_BLE_HEADSET_ENABLE)

/* Includes ------------------------------------------------------------------*/
#include "scenario_ble_audio.h"
#include "hal_audio_driver.h"
#include "scenario_bt_audio.h"
#include "stream_audio_transmitter.h"
#include "sink_inter.h"
#if defined(AIR_BT_AUDIO_DONGLE_USB_ENABLE)
#include "sw_buffer_interface.h"
#include "clk_skew_sw.h"
#endif /* AIR_BT_AUDIO_DONGLE_USB_ENABLE */
#include "dsp_dump.h"
#ifdef AIR_ULL_AUDIO_V2_DONGLE_ENABLE
#include "scenario_ull_audio_v2.h"
#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE */

#ifdef AIR_FIXED_RATIO_SRC
#include "src_fixed_ratio_interface.h"
#endif /* AIR_FIXED_RATIO_SRC */

#ifdef AIR_SOFTWARE_SRC_ENABLE
#include "sw_src_interface.h"
#endif /* AIR_SOFTWARE_SRC_ENABLE */

/* Private define ------------------------------------------------------------*/
#define DL_CLOCK_SKEW_CHECK_COUNT                                   (4)
#define UL_CLOCK_SKEW_CHECK_COUNT                                   (4)
#define AUDIO_USB_IN_FS_MAX_NUM                                     (6)
#define AUDIO_USB_IN_FS_ELEMENT_SIZE                                (3)

log_create_module(common_dongle_log, PRINT_LEVEL_INFO);
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE) || defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
/* Playinfor for le link */
audio_dongle_init_le_play_info_t audio_dongle_bt_init_play_info =
{
    .dl_retransmission_window_clk = 0,
    .dl_retransmission_window_phase = 0,
};
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
extern void ull_audio_v2_dongle_init_play_info(hal_ccni_message_t msg, hal_ccni_message_t *ack);
#endif
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
extern ble_audio_dongle_ul_handle_t *ble_audio_dongle_first_ul_handle;
#endif
#endif
/* ----------------------------------- Unified FS Convert ----------------------------------------------------------- */
const uint32_t audio_usb_in_sample_rate_select[AUDIO_USB_IN_FS_MAX_NUM] = {
    16000, 24000, 32000, 48000, 96000, 192000
};

// low resolution, support <= 48k output
const static uint32_t audio_usb_in_unified_sample_rate_setting_low_res[AUDIO_USB_IN_FS_MAX_NUM][AUDIO_USB_IN_FS_ELEMENT_SIZE] = {
    { 16000,  48000, 48000 },
    { 24000,  48000, 48000 },
    { 32000,  96000, 48000 },
    { 48000,  48000, 48000 },
    { 96000,  48000, 48000 },
    { 192000, 48000, 48000 }
};

// high resolution, support 48/96/192k output
const static uint32_t audio_usb_in_unified_sample_rate_setting_hi_res[AUDIO_USB_IN_FS_MAX_NUM][AUDIO_USB_IN_FS_ELEMENT_SIZE] = {
    { 16000,  48000, 96000 },
    { 24000,  48000, 96000 },
    { 32000,  96000, 96000 },
    { 48000,  96000, 96000 },
    { 96000,  96000, 96000 },
    { 192000, 96000, 96000 }
};

const static uint32_t audio_usb_in_unified_sample_rate_setting_nb[AUDIO_USB_IN_FS_MAX_NUM][AUDIO_USB_IN_FS_ELEMENT_SIZE] = {
    { 16000,  16000, 16000 },
    { 24000,  48000, 16000 },
    { 32000,  16000, 16000 },
    { 48000,  16000, 16000 },
    { 96000,  48000, 16000 },
    { 192000, 64000, 16000 }
};

// const static uint32_t bt_audio_usb_unified_sample_rate_setting_call_mode[AUDIO_USB_IN_FS_MAX_NUM][AUDIO_USB_IN_FS_ELEMENT_SIZE] = {
//     { 16000,  16000, 16000 },
//     { 24000,  16000, 16000 },
//     { 32000,  16000, 16000 },
//     { 48000,  16000, 16000 },
//     { 96000,  48000, 16000 },
//     { 192000, 64000, 16000 } /* NG */
// };

/* Public variables ----------------------------------------------------------*/
const uint32_t crc32_tab[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};
/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
/****************************************************************************************************************************************************/
/*                                                         USB COMMON                                                                               */
/****************************************************************************************************************************************************/
uint8_t audio_dongle_get_format_bytes(hal_audio_format_t pcm_format)
{
    uint8_t pcm_format_bytes = 0;
    if ((pcm_format < HAL_AUDIO_PCM_FORMAT_S16_LE) && (pcm_format >= HAL_AUDIO_PCM_FORMAT_S8)) {
        pcm_format_bytes = 1;
    } else if ((pcm_format < HAL_AUDIO_PCM_FORMAT_S24_LE) && (pcm_format >= HAL_AUDIO_PCM_FORMAT_S16_LE)) {
        pcm_format_bytes = 2;
    } else if ((pcm_format < HAL_AUDIO_PCM_FORMAT_LAST) && (pcm_format >= HAL_AUDIO_PCM_FORMAT_S24_LE)) {
        pcm_format_bytes = 4;
    } else {
        AUDIO_ASSERT(0 && "hal error format");
    }
    return pcm_format_bytes;
}

uint8_t audio_dongle_get_usb_format_bytes(hal_audio_format_t pcm_format)
{
    uint8_t pcm_format_bytes = 0;
    if ((pcm_format < HAL_AUDIO_PCM_FORMAT_S16_LE) && (pcm_format >= HAL_AUDIO_PCM_FORMAT_S8)) {
        pcm_format_bytes = 1;
    } else if ((pcm_format < HAL_AUDIO_PCM_FORMAT_S24_LE) && (pcm_format >= HAL_AUDIO_PCM_FORMAT_S16_LE)) {
        pcm_format_bytes = 2;
    } else if ((pcm_format < HAL_AUDIO_PCM_FORMAT_S32_LE) && (pcm_format >= HAL_AUDIO_PCM_FORMAT_S24_LE)) {
        pcm_format_bytes = 3;
    } else if ((pcm_format < HAL_AUDIO_PCM_FORMAT_LAST) && (pcm_format >= HAL_AUDIO_PCM_FORMAT_S32_LE)) {
        pcm_format_bytes = 4;
    } else {
        AUDIO_ASSERT(0 && "hal error format");
    }
    return pcm_format_bytes;
}

uint32_t audio_dongle_get_n9_share_buffer_data_size(n9_dsp_share_info_t *share_info)
{
    AUDIO_ASSERT(share_info && "share_info is NULL");
    uint32_t avail_size = 0;
    uint32_t read_offset = share_info->read_offset;
    uint32_t write_offset = share_info->write_offset;
    if (read_offset < write_offset) {
        /* normal case */
        avail_size = write_offset - read_offset;
    } else if (read_offset == write_offset) {
        if(share_info->bBufferIsFull == true) {
            /* buffer is full, so read_offset == write_offset */
            avail_size = share_info->sub_info.block_info.block_size * share_info->sub_info.block_info.block_num;
        } else {
            /* buffer is empty, so read_offset == write_offset */
            // BT_DONGLE_LOG_W("[Dongle Common] usb in buffer is empty", 0);
            avail_size = 0;
        }
    } else {
        /* buffer wrapper case */
        avail_size = share_info->sub_info.block_info.block_size * share_info->sub_info.block_info.block_num
                    - read_offset
                    + write_offset;
    }
    return avail_size;
}

static bool audio_dl_unified_fs_convertor_sample_rate_check(uint32_t sample_rate)
{
    bool ret = false;
    for (uint32_t i = 0; i < AUDIO_USB_IN_FS_MAX_NUM; i ++) {
        if (sample_rate == audio_usb_in_sample_rate_select[i]) {
            ret = true;
        }
    }
    return ret;
}

static uint32_t audio_dl_unified_fs_convertor_sample_rate_get(uint32_t in_rate, audio_dsp_codec_type_t codec_type, stream_resolution_t codec_resolution, uint8_t sequence)
{
    uint32_t sample_rate = 0;
    uint32_t raw_index   = 0;
    if (sequence > (AUDIO_USB_IN_FS_ELEMENT_SIZE - 1)) {
        AUDIO_ASSERT(0 && "Unified fs convert input param error!");
        return 0;
    }
    switch (in_rate) {
        case 16000:
            raw_index = 0;
            break;
        case 24000:
            raw_index = 1;
            break;
        case 32000:
            raw_index = 2;
            break;
        case 48000:
            raw_index = 3;
            break;
        case 96000:
            raw_index = 4;
            break;
        case 192000:
            raw_index = 5;
            break;
        default:
            AUDIO_ASSERT(0 && "Unified fs convert sample rate error!");
            break;
    }
    if (raw_index > (uint32_t)(AUDIO_USB_IN_FS_MAX_NUM - 1)) {
        AUDIO_ASSERT(0 && "Unified fs convert raw index is over the limitation!");
    }
    switch (codec_type) {
        case AUDIO_DSP_CODEC_TYPE_MSBC:
        case AUDIO_DSP_CODEC_TYPE_CVSD:
            sample_rate = audio_usb_in_unified_sample_rate_setting_nb[raw_index][sequence];
            break;
        default:
            if (codec_resolution == RESOLUTION_16BIT) {
                sample_rate = audio_usb_in_unified_sample_rate_setting_low_res[raw_index][sequence];
            } else {
                sample_rate = audio_usb_in_unified_sample_rate_setting_hi_res[raw_index][sequence];
            }
            break;
    }
    return sample_rate;
}

static uint32_t audio_dl_unified_fs_convertor_depth_get(uint32_t in_rate, uint32_t out_rate, audio_dsp_codec_type_t codec_type, stream_resolution_t codec_resolution, uint32_t *rate)
{
    uint32_t rate_index_0          = in_rate;
    uint32_t rate_index_1          = 0;
    uint32_t rate_index_2          = 0;
    uint32_t rate_index_3          = out_rate;
    uint32_t fixed_ratio_src_depth = 0;
    rate_index_1 = audio_dl_unified_fs_convertor_sample_rate_get(in_rate, codec_type, codec_resolution, 1);
    rate_index_2 = audio_dl_unified_fs_convertor_sample_rate_get(in_rate, codec_type, codec_resolution, 2);
    if (rate_index_0 != rate_index_3) {
        if (rate_index_0 != rate_index_1) {
            rate[fixed_ratio_src_depth] = rate_index_1;
            fixed_ratio_src_depth ++;
        }
        if (rate_index_1 != rate_index_2) {
            rate[fixed_ratio_src_depth] = rate_index_2;
            fixed_ratio_src_depth ++;
        }
        if ((codec_resolution == RESOLUTION_32BIT) || (codec_type == AUDIO_DSP_CODEC_TYPE_MSBC) || (codec_type == AUDIO_DSP_CODEC_TYPE_CVSD)) {
            if (rate_index_2 != rate_index_3) {
                rate[fixed_ratio_src_depth] = rate_index_3;
                fixed_ratio_src_depth ++;
            }
        }
        if (fixed_ratio_src_depth == 0) {
            fixed_ratio_src_depth = 1;
            rate[0] = rate_index_0;
        }
    } else {
        fixed_ratio_src_depth = 1;
        rate[0] = rate_index_0;
    }

    return fixed_ratio_src_depth;
}

bool audio_dl_unified_fs_convertor_init(audio_dl_unified_fs_convertor_param_t *param)
{
    bool                ret                     = false;
    bool                check_value             = false;
    uint32_t            process_sample_rate_max = param->process_sample_rate_max;
    uint32_t            frame_samples_base      = 0;
    bool                is_bypass_src           = false;
    uint32_t            src_fixed_ratio_depth   = 0;
    int32_t             src_fixed_ratio_cnt     = 0;
    uint32_t            src_fixed_ratio_index   = 0;
    uint32_t            in_rate                 = param->in_rate;
    uint32_t            out_rate                = param->out_rate;
    uint32_t            period_ms               = param->period_ms;
    uint32_t            in_ch_num               = param->in_ch_num;
    hal_audio_format_t  pcm_format              = param->pcm_format;
    audio_dsp_codec_type_t codec_type           = param->codec_type;
    uint32_t            process_max_size        = param->process_max_size;
    uint32_t            sample_size             = 0;
    stream_resolution_t stream_resolution       = RESOLUTION_16BIT;
    SOURCE              source                  = param->source;
    uint32_t            ratio                   = 1;
    uint32_t            next_rate               = 0;
    uint32_t            rate[3]                 = {0};
#ifdef AIR_FIXED_RATIO_SRC
    src_fixed_ratio_config_t sw_src0_config     = {0};
    src_fixed_ratio_config_t sw_src1_config     = {0};
    src_fixed_ratio_config_t sw_src2_config     = {0};
    src_fixed_ratio_port_quality_mode_e mode    = SRC_FIXED_RATIO_PORT_NORMAL_QUALITY;
#endif
    UNUSED(codec_type);
    UNUSED(process_max_size);
    check_value = audio_dl_unified_fs_convertor_sample_rate_check(in_rate);
    AUDIO_ASSERT(check_value && "[Dongle Common][DL][ERROR] fs is not right, check usb in sample rate and bt out sample rate");
    process_sample_rate_max = MAX(in_rate, out_rate);
    frame_samples_base      = period_ms * AUDIO_DONGLE_SAMPLE_RATE_8K / 1000 / 1000;     // 8K base frame
    sample_size             = audio_dongle_get_format_bytes(pcm_format);
    stream_resolution       = (sample_size == 4) ? RESOLUTION_32BIT : RESOLUTION_16BIT;
    // is_bypass_src           = (in_rate == out_rate) ? true : false;
    src_fixed_ratio_depth   = audio_dl_unified_fs_convertor_depth_get(in_rate, out_rate, codec_type, stream_resolution, rate);
    src_fixed_ratio_cnt     = (int32_t)src_fixed_ratio_depth;
    if ((codec_type == AUDIO_DSP_CODEC_TYPE_MSBC) || (codec_type == AUDIO_DSP_CODEC_TYPE_CVSD)) {
        mode = SRC_FIXED_RATIO_PORT_HIGH_QUALITY;
    }
    /* 1st src fixed ratio, msbc/cvsd/sbc: 16k/48k/96k -> 48k, 32k -> 96k, 192k -> 96k */
    /* 1st src fixed ratio, lhdc: 16k/48k/96k -> 48k, 32k -> 96k, 192k -> 96k */
#ifdef AIR_FIXED_RATIO_SRC
    if (src_fixed_ratio_cnt >= 0) {
        sw_src0_config.channel_number            = in_ch_num;
        sw_src0_config.in_sampling_rate          = in_rate;
        sw_src0_config.out_sampling_rate         = rate[src_fixed_ratio_index];
        sw_src0_config.resolution                = stream_resolution;
        sw_src0_config.multi_cvt_mode            = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_CONSECUTIVE;
        sw_src0_config.cvt_num                   = src_fixed_ratio_depth;
        sw_src0_config.with_codec                = false;
        param->src0_port                         = stream_function_src_fixed_ratio_get_port(source);
        process_sample_rate_max                  = MAX(process_sample_rate_max, sw_src0_config.out_sampling_rate);
        sw_src0_config.max_frame_buff_size       = process_max_size;
        sw_src0_config.quality_mode              = mode;
        if (sw_src0_config.in_sampling_rate <= sw_src0_config.out_sampling_rate) { // increase size
            ratio                               = sw_src0_config.out_sampling_rate / sw_src0_config.in_sampling_rate;
            process_max_size                   *= ratio;
            sw_src0_config.max_frame_buff_size  = process_max_size;
        } else {
            ratio = sw_src0_config.in_sampling_rate / sw_src0_config.out_sampling_rate;
            process_max_size /= ratio;
        }
        next_rate = sw_src0_config.out_sampling_rate;
        // stream_function_src_fixed_ratio_init((src_fixed_ratio_port_t *)param->src0_port, &sw_src0_config);
        src_fixed_ratio_index ++;
        src_fixed_ratio_cnt --;
    }

    /* 2nd src fixed ratio, 48k -> 48k, 96k -> 48k */
    if (src_fixed_ratio_cnt > 0) {
        sw_src1_config.channel_number            = in_ch_num;
        sw_src1_config.in_sampling_rate          = next_rate;
        sw_src1_config.out_sampling_rate         = rate[src_fixed_ratio_index];
        sw_src1_config.resolution                = stream_resolution;
        sw_src1_config.multi_cvt_mode            = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_CONSECUTIVE;
        sw_src1_config.cvt_num                   = src_fixed_ratio_depth;
        sw_src1_config.with_codec                = false;
        param->src1_port                         = stream_function_src_fixed_ratio_get_2nd_port(source);
        process_sample_rate_max                  = MAX(process_sample_rate_max, sw_src1_config.out_sampling_rate);
        sw_src1_config.max_frame_buff_size       = process_max_size;
        sw_src1_config.quality_mode              = mode;
        if (sw_src1_config.in_sampling_rate <= sw_src1_config.out_sampling_rate) { // increase size
            ratio                               = sw_src1_config.out_sampling_rate / sw_src1_config.in_sampling_rate;
            process_max_size                   *= ratio;
            sw_src1_config.max_frame_buff_size  = process_max_size;
        } else {
            ratio = sw_src1_config.in_sampling_rate / sw_src1_config.out_sampling_rate;
            process_max_size /= ratio;
        }
        next_rate = sw_src1_config.out_sampling_rate;
        COMMON_DONGLE_LOG_I("[Dongle Common][scenario type %d] sw fixed src1 0x%x info, %d, %d, %d, %d, %d, %d, %d, %d, depth %d", 11,
                    source->scenario_type,
                    param->src1_port,
                    sw_src1_config.multi_cvt_mode,
                    sw_src1_config.cvt_num,
                    sw_src1_config.with_codec,
                    sw_src1_config.channel_number,
                    sw_src1_config.resolution,
                    sw_src1_config.in_sampling_rate,
                    sw_src1_config.out_sampling_rate,
                    sw_src1_config.max_frame_buff_size,
                    src_fixed_ratio_depth
                    );
        stream_function_src_fixed_ratio_init((src_fixed_ratio_port_t *)param->src1_port, &sw_src1_config);
        src_fixed_ratio_index ++;
        src_fixed_ratio_cnt --;
    }
#endif /* AIR_FIXED_RATIO_SRC */
    /* 3st sbc: sw src, 48k -> 48k/44.1k/32k/16k/8k */
    /* 3st lhdc: fixed ratio, 48k -> 48k/44.1k/32k/16k/8k */
    if ((stream_resolution == RESOLUTION_32BIT) || (codec_type == AUDIO_DSP_CODEC_TYPE_MSBC) || (codec_type == AUDIO_DSP_CODEC_TYPE_CVSD)) {
#ifdef AIR_FIXED_RATIO_SRC
        if (src_fixed_ratio_cnt > 0) {
            /* 3nd src fixed ratio, 48k -> 48k, 96k -> 48k */
            sw_src2_config.channel_number            = in_ch_num;
            sw_src2_config.in_sampling_rate          = next_rate;
            sw_src2_config.out_sampling_rate         = out_rate;
            sw_src2_config.resolution                = stream_resolution;
            sw_src2_config.multi_cvt_mode            = SRC_FIXED_RATIO_PORT_MUTI_CVT_MODE_CONSECUTIVE;
            sw_src2_config.cvt_num                   = src_fixed_ratio_depth;
            sw_src2_config.with_codec                = false;
            param->src2_port                         = stream_function_src_fixed_ratio_get_3rd_port(source);
            process_sample_rate_max                  = MAX(process_sample_rate_max, sw_src2_config.out_sampling_rate);
            sw_src2_config.max_frame_buff_size       = process_max_size;
            sw_src2_config.quality_mode              = mode;
            if (sw_src2_config.in_sampling_rate <= sw_src2_config.out_sampling_rate) { // increase size
                ratio                               = sw_src2_config.out_sampling_rate / sw_src2_config.in_sampling_rate;
                process_max_size                   *= ratio;
                sw_src2_config.max_frame_buff_size  = process_max_size;
            } else {
                ratio = sw_src2_config.in_sampling_rate / sw_src2_config.out_sampling_rate;
                process_max_size /= ratio;
            }
            next_rate = sw_src2_config.out_sampling_rate;
            COMMON_DONGLE_LOG_I("[Dongle Common][scenario type %d] sw fixed src2 0x%x info, %d, %d, %d, %d, %d, %d, %d, %d, depth %d", 11,
                        source->scenario_type,
                        param->src2_port,
                        sw_src2_config.multi_cvt_mode,
                        sw_src2_config.cvt_num,
                        sw_src2_config.with_codec,
                        sw_src2_config.channel_number,
                        sw_src2_config.resolution,
                        sw_src2_config.in_sampling_rate,
                        sw_src2_config.out_sampling_rate,
                        sw_src2_config.max_frame_buff_size,
                        src_fixed_ratio_depth
                        );
            stream_function_src_fixed_ratio_init((src_fixed_ratio_port_t *)param->src2_port, &sw_src2_config);
            src_fixed_ratio_cnt --;
        }
#endif /* AIR_FIXED_RATIO_SRC */
    } else {
#ifdef AIR_SOFTWARE_SRC_ENABLE
        sw_src_config_t sw_src_config    = {0};
        AUDIO_ASSERT((out_rate <= 48000) && "classic audio not support sample rate over 48000");
        sw_src_config.mode               = SW_SRC_MODE_NORMAL;
        sw_src_config.channel_num        = in_ch_num;
        sw_src_config.in_res             = stream_resolution;
        sw_src_config.in_sampling_rate   = next_rate;
        sw_src_config.in_frame_size_max  = process_max_size;
        sw_src_config.in_frame_size_max *= sample_size;
        sw_src_config.out_res            = stream_resolution;
        sw_src_config.out_sampling_rate  = out_rate;
        if (sw_src_config.in_sampling_rate <= sw_src_config.out_sampling_rate) { // increase size
            ratio                               = sw_src_config.out_sampling_rate / sw_src_config.in_sampling_rate;
            process_max_size                   *= ratio;
        } else {
            ratio = sw_src_config.in_sampling_rate / sw_src_config.out_sampling_rate;
            process_max_size /= ratio;
        }
        sw_src_config.out_frame_size_max = process_max_size;
        param->src2_port                 = stream_function_sw_src_get_port(source);
        process_sample_rate_max          = MAX(process_sample_rate_max, sw_src_config.out_sampling_rate);
        next_rate = sw_src_config.out_sampling_rate;
        COMMON_DONGLE_LOG_I("[Dongle Common][scenario type %d] sw src 0x%x info, %d, %d, %d, %d, %d, %d, %d, %d\r\n", 10,
                    source->scenario_type,
                    param->src2_port,
                    sw_src_config.mode,
                    sw_src_config.channel_num,
                    sw_src_config.in_res,
                    sw_src_config.in_sampling_rate,
                    sw_src_config.in_frame_size_max,
                    sw_src_config.out_res,
                    sw_src_config.out_sampling_rate,
                    sw_src_config.out_frame_size_max
                    );
        stream_function_sw_src_init((sw_src_port_t *)param->src2_port, &sw_src_config);
#endif /*    */
    }
#ifdef AIR_FIXED_RATIO_SRC
    sw_src0_config.max_frame_buff_size = MAX(sw_src0_config.max_frame_buff_size, sw_src1_config.max_frame_buff_size);
    sw_src0_config.max_frame_buff_size = MAX(sw_src0_config.max_frame_buff_size, sw_src2_config.max_frame_buff_size);
    COMMON_DONGLE_LOG_I("[Dongle Common][scenario type %d] sw fixed src0 0x%x bypass %d, info, %d, %d, %d, %d, %d, %d, %d, %d, depth %d", 12,
                source->scenario_type,
                param->src0_port,
                is_bypass_src,
                sw_src0_config.multi_cvt_mode,
                sw_src0_config.cvt_num,
                sw_src0_config.with_codec,
                sw_src0_config.channel_number,
                sw_src0_config.resolution,
                sw_src0_config.in_sampling_rate,
                sw_src0_config.out_sampling_rate,
                sw_src0_config.max_frame_buff_size,
                src_fixed_ratio_depth
                );
    stream_function_src_fixed_ratio_init((src_fixed_ratio_port_t *)param->src0_port, &sw_src0_config);
#endif
    /* update parameters */
    param->process_sample_rate_max = process_sample_rate_max;
    ret = true;
    return ret;
}

bool audio_dl_unified_fs_convertor_deinit(audio_dl_unified_fs_convertor_param_t *param)
{
    uint32_t            sample_size       = 0;
    hal_audio_format_t  pcm_format        = param->pcm_format;
    stream_resolution_t stream_resolution = RESOLUTION_16BIT;
    void*               src0_port         = param->src0_port;
    void*               src1_port         = param->src1_port;
    void*               src2_port         = param->src2_port;
    sample_size       = audio_dongle_get_format_bytes(pcm_format);
    stream_resolution = (sample_size == 4) ? RESOLUTION_32BIT : RESOLUTION_16BIT;
    /* 1st src fixed ratio */
    if (src0_port) {
        #ifdef AIR_FIXED_RATIO_SRC
        stream_function_src_fixed_ratio_deinit((src_fixed_ratio_port_t *)src0_port);
        #endif
    }
    /* 2nd src fixed ratio */
    if (src1_port) {
        #ifdef AIR_FIXED_RATIO_SRC
        stream_function_src_fixed_ratio_deinit((src_fixed_ratio_port_t *)src1_port);
        #endif
    }
    /* 3rd src */
    if (stream_resolution == RESOLUTION_32BIT) {
        if (src2_port) {
            #ifdef AIR_FIXED_RATIO_SRC
            stream_function_src_fixed_ratio_deinit((src_fixed_ratio_port_t *)src2_port);
            #endif
        }
    } else {
        if (src2_port) {
            #ifdef AIR_SOFTWARE_SRC_ENABLE
            stream_function_sw_src_deinit((sw_src_port_t *)src2_port);
            #endif
        }
    }
    return true;
}

#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
uint32_t audio_dongle_get_n9_share_buffer_data_size_without_header(n9_dsp_share_info_t *share_info)
{
    uint32_t avail_size       = 0;
    uint32_t ret              = 0;
    uint32_t unprocess_frames = 0;
    avail_size = audio_dongle_get_n9_share_buffer_data_size(share_info);
    /* get unprocess frame number */
    unprocess_frames = (avail_size / share_info->sub_info.block_info.block_size);
    /* convert to one channel */
    ret = avail_size - unprocess_frames * sizeof(audio_transmitter_frame_header_t);
    return ret;
}
#endif /* AIR_AUDIO_TRANSMITTER_ENABLE */

/* Get Play infor for ULL2.0/LEA Dongle */
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE) || defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void audio_dongle_init_le_play_info(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    audio_dongle_init_le_play_info_t *play_info = NULL;
    uint32_t i = 0;
    uint32_t saved_mask = 0;
    UNUSED(ack);
    UNUSED(i);
    UNUSED(saved_mask);
    /* save play info to the global variables */
    play_info = (audio_dongle_init_le_play_info_t *)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    memcpy(&audio_dongle_bt_init_play_info, play_info, sizeof(audio_dongle_init_le_play_info_t));

    /* update uplink BT transmission window clk */
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
    ull_audio_v2_dongle_init_play_info(msg, ack);
#endif
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
    ble_audio_dongle_ul_handle_t *le_handle = NULL;
    hal_nvic_save_and_set_interrupt_mask(&saved_mask);
    if (ble_audio_dongle_first_ul_handle != NULL)
    {
        le_handle = ble_audio_dongle_first_ul_handle;
        for (i = 0; i < (uint32_t)ble_audio_dongle_first_ul_handle->total_number; i++)
        {
            if ((audio_dongle_bt_init_play_info.dl_retransmission_window_clk != 0) && (le_handle->bt_retry_window < audio_dongle_bt_init_play_info.dl_retransmission_window_clk))
            {
                le_handle->bt_retry_window = audio_dongle_bt_init_play_info.dl_retransmission_window_clk;
            }
            /* switch the next handle */
            le_handle = le_handle->next_ul_handle;
        }
    }
    hal_nvic_restore_interrupt_mask(saved_mask);
#endif

    COMMON_DONGLE_LOG_I("[Dongle Common][UL][config] play_info->dl_timestamp_clk %d, dl_timestamp_phase %d", 2,
        audio_dongle_bt_init_play_info.dl_retransmission_window_clk,
        audio_dongle_bt_init_play_info.dl_retransmission_window_phase
        );
}
#endif /* dongle for ull2.0/lea */

#if defined(AIR_BT_AUDIO_DONGLE_USB_ENABLE)
int32_t audio_dongle_ul_usb_clock_skew_check(void *dongle_handle, audio_dongle_type_t dongle_type)
{
    int32_t compensatory_samples = 0;
    int32_t remain_samples;
    int32_t input_frame = 1;
    int32_t frame_samples = 0;
    int32_t sample_size = 0;
    audio_dongle_first_packet_status_t first_packet_status = AUDIO_DONGLE_UL_FIRST_PACKET_NOT_READY;
    uint16_t fetch_count = 0;
    sw_buffer_port_t *buffer_port_0 = NULL;
    int16_t clk_skew_count = 0;
    int32_t clk_skew_watermark_samples = 0;
    hal_audio_format_t pcm_format = HAL_AUDIO_PCM_FORMAT_DUMMY;
    /* get parameters from dongle */
    switch (dongle_type) {
#ifdef AIR_BT_AUDIO_DONGLE_ENABLE
        case AUDIO_DONGLE_TYPE_BT:
            first_packet_status = ((bt_audio_dongle_handle_t *)dongle_handle)->first_packet_status;
            buffer_port_0 = ((bt_audio_dongle_handle_t *)dongle_handle)->buffer_port_0;
            clk_skew_count = ((bt_audio_dongle_handle_t *)dongle_handle)->clk_skew_count;
            clk_skew_watermark_samples = ((bt_audio_dongle_handle_t *)dongle_handle)->clk_skew_watermark_samples;
            pcm_format = ((bt_audio_dongle_handle_t *)dongle_handle)->stream_info.ul_info.source.bt_in.sample_format;
            fetch_count = ((bt_audio_dongle_handle_t *)dongle_handle)->fetch_count;
            frame_samples = ((bt_audio_dongle_handle_t *)dongle_handle)->stream_info.ul_info.source.bt_in.frame_samples;
            break;
#endif
        default:
            break;
    }
    sample_size = audio_dongle_get_format_bytes(pcm_format);
    if (first_packet_status == AUDIO_DONGLE_UL_FIRST_PACKET_PLAYED) {
        /* get remain samples */
        remain_samples = stream_function_sw_buffer_get_channel_used_size(buffer_port_0, 1) / sample_size;

        if (fetch_count != 0) {
            input_frame = 0; /* input 1 frame, then output 1 frame, so input frame is zero */
        }
        if (remain_samples + input_frame * frame_samples > clk_skew_watermark_samples) {
            if (clk_skew_count > -UL_CLOCK_SKEW_CHECK_COUNT) {
                clk_skew_count -= 1;
            }
        } else if (remain_samples + input_frame * frame_samples < clk_skew_watermark_samples) {
            if (clk_skew_count < UL_CLOCK_SKEW_CHECK_COUNT) {
                clk_skew_count += 1;
            }
        } else {
            // if ((dongle_handle->clk_skew_count < UL_CLOCK_SKEW_CHECK_COUNT) && (dongle_handle->clk_skew_count > 0))
            // {
            //     dongle_handle->clk_skew_count -= 1;
            // }
            // else if ((dongle_handle->clk_skew_count > UL_CLOCK_SKEW_CHECK_COUNT) && (dongle_handle->clk_skew_count < 0))
            // {
            //     dongle_handle->clk_skew_count += 1;
            // }
            clk_skew_count = 0;
        }

        if (clk_skew_count == UL_CLOCK_SKEW_CHECK_COUNT) {
            compensatory_samples = 1;
        } else if (clk_skew_count == -UL_CLOCK_SKEW_CHECK_COUNT) {
            compensatory_samples = -1;
        } else {
            compensatory_samples = 0;
        }
    }
    /* update parameters to dongle */
    switch (dongle_type) {
#ifdef AIR_BT_AUDIO_DONGLE_ENABLE
        case AUDIO_DONGLE_TYPE_BT:
            ((bt_audio_dongle_handle_t *)dongle_handle)->clk_skew_count = clk_skew_count;
            break;
#endif
        default:
            break;
    }

    return compensatory_samples;
}

ATTR_TEXT_IN_IRAM int32_t audio_dongle_dl_usb_clock_skew_check(void *dongle_handle, uint32_t input_samples, uint32_t *buffer0_output_size, audio_dongle_type_t dongle_type)
{
    int32_t compensatory_samples = 0;
    int32_t remain_samples = 0;
    int32_t remain_samples_0 = 0;
    int32_t output_samples = 0;
    uint32_t buffer_output_size = 0;
    int32_t frac_rpt = 0;
    audio_dongle_first_packet_status_t first_packet_status = AUDIO_DONGLE_DL_FIRST_PACKET_NOT_READY;
    sw_buffer_port_t *buffer_port_0 = NULL;
    sw_clk_skew_port_t *clk_skew_port = NULL;
    int16_t clk_skew_count = 0;
    int32_t clk_skew_watermark_samples = 0;
    int32_t clk_skew_compensation_mode = 0;
    uint32_t buffer_default_output_size = 0;
    hal_audio_format_t pcm_format = HAL_AUDIO_PCM_FORMAT_DUMMY;
    uint8_t bytes_per_sample = 0;
    SOURCE source = NULL;
    /* get parameters from dongle */
    switch (dongle_type) {
#ifdef AIR_BT_AUDIO_DONGLE_ENABLE
        case AUDIO_DONGLE_TYPE_BT:
            first_packet_status = ((bt_audio_dongle_handle_t *)dongle_handle)->first_packet_status;
            output_samples = ((bt_audio_dongle_handle_t *)dongle_handle)->src_in_frame_samples;
            buffer_default_output_size = ((bt_audio_dongle_handle_t *)dongle_handle)->buffer_default_output_size;
            buffer_output_size = buffer_default_output_size;
            buffer_port_0 = ((bt_audio_dongle_handle_t *)dongle_handle)->buffer_port_0;
            clk_skew_port = ((bt_audio_dongle_handle_t *)dongle_handle)->clk_skew_port;
            clk_skew_count = ((bt_audio_dongle_handle_t *)dongle_handle)->clk_skew_count;
            clk_skew_watermark_samples = ((bt_audio_dongle_handle_t *)dongle_handle)->clk_skew_watermark_samples;
            clk_skew_compensation_mode = ((bt_audio_dongle_handle_t *)dongle_handle)->clk_skew_compensation_mode;
            source     = ((bt_audio_dongle_handle_t *)dongle_handle)->source;
            // BT classic dongle only support 16bit.
            pcm_format = ((bt_audio_dongle_handle_t *)dongle_handle)->stream_info.dl_info.sink.bt_out.sample_format;
            break;
#endif
        default:
            break;
    }
    bytes_per_sample = audio_dongle_get_format_bytes(pcm_format);
    if (first_packet_status == AUDIO_DONGLE_DL_FIRST_PACKET_READY) {
        /* get remain samples */
        remain_samples_0 = stream_function_sw_buffer_get_channel_used_size(buffer_port_0, 1) / bytes_per_sample;

        if ((clk_skew_count > -DL_CLOCK_SKEW_CHECK_COUNT) && (clk_skew_count < DL_CLOCK_SKEW_CHECK_COUNT)) {
            /* in here, clock skew is not started */
            remain_samples = remain_samples_0 + input_samples - output_samples;
            if (remain_samples < 0) {
                /* reset state machine */
                compensatory_samples = 0;
                clk_skew_count = 0;
                COMMON_DONGLE_LOG_E("[Dongle Common][DL][ERROR][Scenario type %d] samples are not enough, %d, %d, %d\r\n", 4,
                    source->scenario_type,
                    remain_samples_0,
                    input_samples,
                    output_samples
                    );
            } else if (remain_samples < clk_skew_watermark_samples) {
                /* usb clock is slower than bt clock */
                clk_skew_count += 1;
                if (clk_skew_count == DL_CLOCK_SKEW_CHECK_COUNT) {
                    /* do nothing */
                }
            } else if (remain_samples > clk_skew_watermark_samples) {
                /* usb clock is faster than bt clock */
                clk_skew_count -= 1;
                if (clk_skew_count == -DL_CLOCK_SKEW_CHECK_COUNT) {
                    /* do nothing */
                }
            } else {
                /* usb clock is as the same as bt clock */
                if (clk_skew_count > 0) {
                    clk_skew_count -= 1;
                } else if (clk_skew_count < 0) {
                    clk_skew_count += 1;
                }
            }
            /* do not compensatory */
            compensatory_samples = 0;
        } else {
            /* in here, clock skew is running */
            remain_samples = remain_samples_0+input_samples-output_samples;
            if (remain_samples < 0) {
                /* reset state machine */
                compensatory_samples = 0;
                clk_skew_count = 0;
                COMMON_DONGLE_LOG_E("[Dongle Common][DL][ERROR][Scenario type %d] samples are not enough, %d, %d, %d\r\n", 4,
                    source->scenario_type,
                    remain_samples_0,
                    input_samples,
                    output_samples
                    );
            } else if (clk_skew_count == DL_CLOCK_SKEW_CHECK_COUNT) {
                if (remain_samples >= clk_skew_watermark_samples) {
                    /* watermark is ok and stop compensatory */
                    compensatory_samples = 0;
                    clk_skew_count = 0;
                } else {
                    /* do +1 sample compensatory */
                    compensatory_samples = 1;
                    stream_function_sw_clk_skew_get_frac_rpt(clk_skew_port, 1, &frac_rpt);
                    if (frac_rpt == (clk_skew_compensation_mode - 1)) {
                        buffer_output_size = buffer_default_output_size - (bytes_per_sample * compensatory_samples);
                    }
                }
            } else if (clk_skew_count == -DL_CLOCK_SKEW_CHECK_COUNT) {
                if (remain_samples <= clk_skew_watermark_samples) {
                    /* watermark is ok and stop compensatory */
                    compensatory_samples = 0;
                    clk_skew_count = 0;
                } else {
                    /* do -1 sample compensatory */
                    compensatory_samples = -1;
                    stream_function_sw_clk_skew_get_frac_rpt(clk_skew_port, 1, &frac_rpt);
                    if (frac_rpt == -(clk_skew_compensation_mode - 1)) {
                        buffer_output_size = buffer_default_output_size + (bytes_per_sample * -compensatory_samples);
                    }
                }
            } else {
                COMMON_DONGLE_LOG_E("[Dongle Common][DL][ERROR][Scenario type %d] error clk skew count, %d\r\n", 2,
                    source->scenario_type,
                    clk_skew_count
                    );
                AUDIO_ASSERT(0);
            }
        }
    }
    COMMON_DONGLE_LOG_I("[Dongle Common][DL] %d %d %d %d %d %d %d %d", 8,
        remain_samples, clk_skew_count, compensatory_samples, buffer_output_size, clk_skew_watermark_samples, frac_rpt, input_samples, output_samples);
    /* update parameters to dongle */
    switch (dongle_type) {
#ifdef AIR_BT_AUDIO_DONGLE_ENABLE
        case AUDIO_DONGLE_TYPE_BT:
            //((bt_audio_dongle_handle_t *)dongle_handle)->buffer_default_output_size = buffer_output_size;
            ((bt_audio_dongle_handle_t *)dongle_handle)->clk_skew_count = clk_skew_count;
            break;
#endif
        default:
            break;
    }
    *buffer0_output_size = buffer_output_size;
    return compensatory_samples;
}

ATTR_TEXT_IN_IRAM void audio_dongle_dl_usb_data_copy(
    void *dongle_handle,
    SINK sink,
    uint8_t *src_buf,
    uint32_t samples,
    uint32_t samples_offset,
    audio_dongle_type_t dongle_type)
{
    hal_audio_format_t     usb_in_sample_format = HAL_AUDIO_PCM_FORMAT_DUMMY;
    hal_audio_format_t     bt_out_sample_format = HAL_AUDIO_PCM_FORMAT_DUMMY;
    DSP_STREAMING_PARA_PTR stream               = DSP_Streaming_Get(sink->transform->source, sink);
    uint8_t                channel_num          = 0;
    /* get parameters from dongle */
    switch (dongle_type) {
#ifdef AIR_BT_AUDIO_DONGLE_ENABLE
        case AUDIO_DONGLE_TYPE_BT:
            usb_in_sample_format = ((bt_audio_dongle_handle_t *)dongle_handle)->stream_info.dl_info.source.usb_in.sample_format;
            bt_out_sample_format = ((bt_audio_dongle_handle_t *)dongle_handle)->stream_info.dl_info.sink.bt_out.sample_format;
            channel_num = ((bt_audio_dongle_handle_t *)dongle_handle)->stream_info.dl_info.source.usb_in.channel_num;
            break;
#endif
        default:
            break;
    }
    switch (channel_num) {
        case 2:
            if ((usb_in_sample_format < HAL_AUDIO_PCM_FORMAT_S24_LE) && (bt_out_sample_format >= HAL_AUDIO_PCM_FORMAT_S24_LE)) {
                ShareBufferCopy_I_16bit_to_D_32bit_2ch((uint32_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[0])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[1])+samples_offset,
                                                        samples);
            } else if ((usb_in_sample_format < HAL_AUDIO_PCM_FORMAT_S24_LE) && (bt_out_sample_format < HAL_AUDIO_PCM_FORMAT_S24_LE)) {
                ShareBufferCopy_I_16bit_to_D_16bit_2ch((uint32_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[0])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[1])+samples_offset,
                                                        samples);
                LOG_AUDIO_DUMP((uint8_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)), samples << 2, AUDIO_BT_SRC_DONGLE_DL_MCU_USB_IN_DUAL);
            } else if ((usb_in_sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE) && (bt_out_sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE)) {
                ShareBufferCopy_I_24bit_to_D_32bit_2ch((uint8_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[0])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[1])+samples_offset,
                                                        samples);
            } else if ((usb_in_sample_format >= HAL_AUDIO_PCM_FORMAT_S24_LE) && (bt_out_sample_format < HAL_AUDIO_PCM_FORMAT_S24_LE)) {
                ShareBufferCopy_I_24bit_to_D_16bit_2ch((uint8_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[0])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[1])+samples_offset,
                                                        samples);
            } else {
                COMMON_DONGLE_LOG_E("[Dongle Common][DL][ERROR][handle 0x%x]sample format is not supported, %u, %u\r\n", 3, dongle_handle, usb_in_sample_format, bt_out_sample_format);
                AUDIO_ASSERT(0);
            }
            break;

        case 8:
            if ((usb_in_sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) && (bt_out_sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE)) {
                ShareBufferCopy_I_16bit_to_D_32bit_8ch((uint32_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[0])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[1])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[2])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[3])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[4])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[5])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[6])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[7])+samples_offset,
                                                        samples);
            } else if ((usb_in_sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE) && (bt_out_sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)) {
                ShareBufferCopy_I_16bit_to_D_16bit_8ch((uint32_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[0])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[1])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[2])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[3])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[4])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[5])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[6])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[7])+samples_offset,
                                                        samples);
            } else if ((usb_in_sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE) && (bt_out_sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE)) {
                ShareBufferCopy_I_24bit_to_D_32bit_8ch((uint32_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[0])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[1])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[2])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[3])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[4])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[5])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[6])+samples_offset,
                                                        (uint32_t *)(stream->callback.EntryPara.in_ptr[7])+samples_offset,
                                                        samples);
            } else if ((usb_in_sample_format == HAL_AUDIO_PCM_FORMAT_S24_LE) && (bt_out_sample_format == HAL_AUDIO_PCM_FORMAT_S16_LE)) {
                ShareBufferCopy_I_24bit_to_D_16bit_8ch((uint32_t *)(src_buf + sizeof(audio_transmitter_frame_header_t)),
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[0])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[1])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[2])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[3])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[4])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[5])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[6])+samples_offset,
                                                        (uint16_t *)(stream->callback.EntryPara.in_ptr[7])+samples_offset,
                                                        samples);
            } else {
                COMMON_DONGLE_LOG_E("[Dongle Common][DL][ERROR][handle 0x%x]sample format is not supported, %u, %u\r\n", 2, dongle_handle, usb_in_sample_format);
                AUDIO_ASSERT(0);
            }
            break;

        default:
            COMMON_DONGLE_LOG_E("[Dongle Common][DL][ERROR][handle 0x%x]channel num is not supported, %u, %u\r\n", 2, dongle_handle, channel_num);
            AUDIO_ASSERT(0);
    }
}

#endif /* AIR_BT_AUDIO_DONGLE_USB_ENABLE */
/****************************************************************************************************************************************************/
/*                                                         AFE IN COMMON                                                                            */
/****************************************************************************************************************************************************/
#if (defined AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE) || (defined AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE) || defined AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE || defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE || defined AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE || defined AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE
bool g_i2s_slv_common_tracking_start_flag = false;
void audio_dongle_dl_afe_start(void *dongle_handle, audio_dongle_type_t dongle_type)
{
    audio_dongle_afe_in_info_t   *afe_in     = NULL;
    hal_audio_memory_parameter_t *mem_handle = NULL;
    SOURCE                       source      = NULL;
    SINK                         sink        = NULL;
    uint32_t                     gpt_count   = 0;
    hal_audio_agent_t            agent       = HAL_AUDIO_AGENT_ERROR;

    if (dongle_type == AUDIO_DONGLE_TYPE_BLE) {
#if defined AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE || defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
            afe_in     = &((ble_audio_dongle_dl_handle_t *)dongle_handle)->source_info.afe_in;
            mem_handle = &((ble_audio_dongle_dl_handle_t *)dongle_handle)->source->param.audio.mem_handle;
            source     = ((ble_audio_dongle_dl_handle_t *)dongle_handle)->source;
            sink       = ((ble_audio_dongle_dl_handle_t *)dongle_handle)->sink;
#endif
    } else if (dongle_type == AUDIO_DONGLE_TYPE_BT) {
#if defined AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE || defined AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE
            afe_in     = &((bt_audio_dongle_handle_t *)dongle_handle)->afe_in;
            mem_handle = &((bt_audio_dongle_handle_t *)dongle_handle)->source->param.audio.mem_handle;
            source     = ((bt_audio_dongle_handle_t *)dongle_handle)->source;
            sink       = ((bt_audio_dongle_handle_t *)dongle_handle)->sink;
#endif
    } else if (dongle_type == AUDIO_DONGLE_TYPE_ULL_V2) {
#if (defined AIR_GAMING_MODE_DONGLE_V2_AFE_IN_ENABLE)
        afe_in     = &((ull_audio_v2_dongle_dl_handle_t *)dongle_handle)->source_info.afe_in;
        mem_handle = &((ull_audio_v2_dongle_dl_handle_t *)dongle_handle)->source->param.audio.mem_handle;
        source     = ((ull_audio_v2_dongle_dl_handle_t *)dongle_handle)->source;
        sink       = ((ull_audio_v2_dongle_dl_handle_t *)dongle_handle)->sink;
#endif
    }
    UNUSED(dongle_handle);
    UNUSED(sink);
    if (!afe_in) {
        AUDIO_ASSERT(0 && "dongle type is not correct");
        return;
    }

    if ((mem_handle->memory_select != HAL_AUDIO_MEMORY_UL_SLAVE_DMA) &&
        (mem_handle->memory_select != HAL_AUDIO_MEMORY_UL_SLAVE_TDM) &&
        (mem_handle->memory_select != 0)) {
        // interconn mode
        agent = hal_memory_convert_agent(mem_handle->memory_select);
        /* disable AFE irq here */
        if (source->param.audio.memory != HAL_AUDIO_MEM_SUB) {
            /* sub source will use this irq */
            hal_memory_set_irq_enable(agent, HAL_AUDIO_CONTROL_OFF);
        }
    }

    /* set flag */
    source->param.audio.drop_redundant_data_at_first_time = true; // not used ?

    /* enable afe agent here */
    if ((source->param.audio.mem_handle.pure_agent_with_src != true) && // i2s slv interconn tracking mode
        (source->param.audio.AfeBlkControl.u4asrcflag != true)) {       // i2s slv vdma tracking mode
        /* interconn non-tracking mode */
        hal_audio_trigger_start_parameter_t start_parameter;
        start_parameter.memory_select = mem_handle->memory_select;
        start_parameter.enable = true;
        hal_audio_set_value((hal_audio_set_value_parameter_t *)&start_parameter, HAL_AUDIO_SET_TRIGGER_MEMORY_START);
        /* set agent regsiters' address */
        switch (agent) {
            case HAL_AUDIO_AGENT_MEMORY_VUL1:
                afe_in->afe_vul_cur_addr  = AFE_VUL_CUR;
                afe_in->afe_vul_base_addr = AFE_VUL_BASE;
                break;
            case HAL_AUDIO_AGENT_MEMORY_VUL2:
                afe_in->afe_vul_cur_addr  = AFE_VUL2_CUR;
                afe_in->afe_vul_base_addr = AFE_VUL2_BASE;
                break;
            case HAL_AUDIO_AGENT_MEMORY_VUL3:
                afe_in->afe_vul_cur_addr  = AFE_VUL3_CUR;
                afe_in->afe_vul_base_addr = AFE_VUL3_BASE;
                break;
            case HAL_AUDIO_AGENT_MEMORY_AWB:
                afe_in->afe_vul_cur_addr  = AFE_AWB_CUR;
                afe_in->afe_vul_base_addr = AFE_AWB_BASE;
                break;
            case HAL_AUDIO_AGENT_MEMORY_AWB2:
                afe_in->afe_vul_cur_addr  = AFE_AWB2_CUR;
                afe_in->afe_vul_base_addr = AFE_AWB2_BASE;
                break;
            default:
                AUDIO_ASSERT(0 && "[Dongle Common][DL][AFE IN] ERROR: unknow agent");
        }
    } else {
        /* Tracking mode */
        if (source->param.audio.audio_device == HAL_AUDIO_DEVICE_I2S_SLAVE) {
            if (source->param.audio.AfeBlkControl.u4asrcflag) {
                uint8_t src_id = source->param.audio.AfeBlkControl.u4asrcid;
                afe_in->afe_vul_cur_addr  = ASM_CH01_OBUF_WRPNT + 0x100 * src_id;
                afe_in->afe_vul_base_addr = ASM_OBUF_SADR + 0x100 * src_id;
            } else { // interconn mode use hwsrc2
                afe_in->afe_vul_cur_addr  = ASM2_CH01_OBUF_WRPNT;
                afe_in->afe_vul_base_addr = ASM2_OBUF_SADR;
            }
            g_i2s_slv_common_tracking_start_flag = false;
            afe_in->i2s_slv_flag = false;
        } else {
            AUDIO_ASSERT(0 && "[Dongle][DL][AFE IN] setting is wrong, plz check it!");
        }
    }
    /* clear stream info */
    source->streamBuffer.BufferInfo.ReadOffset  = 0;
    source->streamBuffer.BufferInfo.WriteOffset = 0;
    source->streamBuffer.BufferInfo.bBufferIsFull = false;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);

    COMMON_DONGLE_LOG_I("[Dongle Common][DL][AFE IN] scenario type %d-%d, start stream gpt count = 0x%x, cur_addr = 0x%x, base_addr = 0x%x, cur_wo = 0x%x, cur_base = 0x%x", 7,
                source->scenario_type,
                sink->scenario_type,
                gpt_count,
                afe_in->afe_vul_cur_addr,
                afe_in->afe_vul_base_addr,
                AFE_GET_REG(afe_in->afe_vul_cur_addr),
                AFE_GET_REG(afe_in->afe_vul_base_addr)
                );

}

void audio_dongle_afe_in_ccni_handler(void *dongle_handle, audio_dongle_type_t dongle_type)
{
    uint32_t gpt_count = 0;
    uint32_t gpt_count_1 = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count);
    audio_dongle_afe_in_info_t   *afe_in     = NULL;
    SOURCE                       source      = NULL;
    SINK                         sink        = NULL;
    audio_dongle_data_status_t   data_status = AUDIO_DONGLE_DL_DATA_EMPTY;
    audio_dongle_stream_status_t stream_status = AUDIO_DONGLE_STREAM_DEINIT;
    audio_dongle_first_packet_status_t first_packet_status = AUDIO_DONGLE_DL_FIRST_PACKET_NOT_READY;
    uint32_t prefill_size = 0;
    if (dongle_type == AUDIO_DONGLE_TYPE_BLE) {
#if defined AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE || defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
            afe_in     = &((ble_audio_dongle_dl_handle_t *)dongle_handle)->source_info.afe_in;
            source     = ((ble_audio_dongle_dl_handle_t *)dongle_handle)->source;
            sink       = ((ble_audio_dongle_dl_handle_t *)dongle_handle)->sink;
            prefill_size = source->param.audio.frame_size;
#endif
    } else if (dongle_type == AUDIO_DONGLE_TYPE_BT) {
#if defined AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE || defined AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE
            afe_in     = &((bt_audio_dongle_handle_t *)dongle_handle)->afe_in;
            source     = ((bt_audio_dongle_handle_t *)dongle_handle)->source;
            sink       = ((bt_audio_dongle_handle_t *)dongle_handle)->sink;
            stream_status = ((bt_audio_dongle_handle_t *)dongle_handle)->stream_status;
            data_status = ((bt_audio_dongle_handle_t *)dongle_handle)->data_status;
            first_packet_status = ((bt_audio_dongle_handle_t *)dongle_handle)->first_packet_status;
            prefill_size = ((((bt_audio_dongle_handle_t *)dongle_handle)->stream_info.dl_info.sink.bt_out).frame_interval) * source->param.audio.src_rate / 1000 / 1000 * (source->param.audio.format_bytes);
#endif
    }
    UNUSED(dongle_handle);
    UNUSED(sink);
    BUFFER_INFO *buffer_info = NULL;
    AUDIO_PARAMETER *runtime = NULL;
    uint32_t data_size;
    uint32_t buffer_per_channel_shift;
    if ((stream_status == AUDIO_DONGLE_STREAM_START) || (stream_status == AUDIO_DONGLE_STREAM_RUNNING)) {
        /* stream is started */
        /* set timestamp for debug */
        /* increase fetch count */
        /* afe in config */
        buffer_info = &source->streamBuffer.BufferInfo;
        runtime = &source->param.audio;
        /* update read & write pointer */
        afe_in->pre_write_offset  = buffer_info->WriteOffset;
        afe_in->pre_read_offset   = buffer_info->ReadOffset;
        afe_in->afe_vul_cur       = AFE_GET_REG(afe_in->afe_vul_cur_addr);
        afe_in->afe_vul_base      = AFE_GET_REG(afe_in->afe_vul_base_addr);
        afe_in->cur_write_offset  = afe_in->afe_vul_cur - afe_in->afe_vul_base;
        buffer_info->WriteOffset = afe_in->cur_write_offset;
        /* check if the samples are enough */
        buffer_per_channel_shift = ((source->param.audio.channel_num>=2) && (source->buftype == BUFFER_TYPE_INTERLEAVED_BUFFER )) ? 1 : 0;
        data_size = (afe_in->cur_write_offset >= afe_in->pre_read_offset)
                    ? (afe_in->cur_write_offset - afe_in->pre_read_offset)>>buffer_per_channel_shift
                    : (source->streamBuffer.BufferInfo.length - afe_in->pre_read_offset + afe_in->cur_write_offset)>>buffer_per_channel_shift;
        /* update afe buffer current size before process */
        afe_in->afe_buffer_latency_size = data_size;
        if (runtime->irq_exist == false) {
            /* get the consuming time */
            /* Jitter between BT CCNI IRQ and Audio HW, to avoid that data is less than a frame: 5ms * sample_rate * format */
            uint32_t duration_cnt = 0;
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &gpt_count_1);
            hal_gpt_get_duration_count(gpt_count, gpt_count_1, &duration_cnt);
            uint32_t ratio_ms = duration_cnt/100 + 1; // add 0.1ms to avoid some abnormal case
            uint32_t jitter_size = (uint64_t)source->param.audio.frame_size * ratio_ms / 50;
            /* stream is played till the samples in AFE buffer is enough */
            if (data_size >= (prefill_size + jitter_size)) {
                if ((source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN) ||
                    (source->param.audio.audio_device & HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE)) {
                    /* tracking mode should buffer some data to avoid irq period jitter */
                    #ifndef AIR_I2S_SLAVE_ENABLE
                    buffer_info->ReadOffset += ((data_size - (prefill_size + jitter_size)) << buffer_per_channel_shift);
                    buffer_info->ReadOffset = (source->streamBuffer.BufferInfo.ReadOffset % (source->streamBuffer.BufferInfo.length));
                    if (source->param.audio.format_bytes == 4) {
                        buffer_info->ReadOffset = EIGHT_BYTE_ALIGNED(buffer_info->ReadOffset);
                    } else {
                        buffer_info->ReadOffset = FOUR_BYTE_ALIGNED(buffer_info->ReadOffset);
                    }
                    afe_in->pre_read_offset = buffer_info->ReadOffset;
                    #endif
                } else {
                    /* LINE IN only need the latest 5ms data */
                    buffer_info->ReadOffset += ((data_size - (prefill_size + jitter_size)) << buffer_per_channel_shift);
                    buffer_info->ReadOffset = (source->streamBuffer.BufferInfo.ReadOffset % (source->streamBuffer.BufferInfo.length));
                    if (source->param.audio.format_bytes == 4) {
                        buffer_info->ReadOffset = EIGHT_BYTE_ALIGNED(buffer_info->ReadOffset);
                    } else {
                        buffer_info->ReadOffset = FOUR_BYTE_ALIGNED(buffer_info->ReadOffset);
                    }
                    afe_in->pre_read_offset = buffer_info->ReadOffset;
                }

                /* samples are enough, so update data status */
                data_status = AUDIO_DONGLE_DL_DATA_IN_STREAM;

                /* update flag */
                runtime->irq_exist = true;

                COMMON_DONGLE_LOG_I("[Dongle Common][DL][AFE IN] scenario type %d, first handle afe buffer ro[%d] wo[%d] %d", 4,
                    source->scenario_type,
                    buffer_info->ReadOffset,
                    buffer_info->WriteOffset,
                    jitter_size
                    );
                /* update this status to trigger sw mixer handle */
                first_packet_status =  AUDIO_DONGLE_DL_FIRST_PACKET_READY;

                /* Handler the stream */
                AudioCheckTransformHandle(source->transform);
            }
            if ((source->scenario_type == AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_I2S_IN) ||
                (source->param.audio.audio_device & HAL_AUDIO_CONTROL_DEVICE_I2S_SLAVE)) {
                uint8_t  src_id = source->param.audio.AfeBlkControl.u4asrcid;
                uint32_t src_tracking_rate = AFE_READ(ASM_FREQUENCY_2 + 0x100 * src_id) * 100 / 4096;
                if ((buffer_info->WriteOffset & (~15)) == (buffer_info->ReadOffset & (~15))) {
                    COMMON_DONGLE_LOG_E("[Dongle Common][DL][AFE IN] hwsrc start fail owo[%d] oro[%d] src_%d rate %d", 4,
                        buffer_info->WriteOffset,
                        buffer_info->ReadOffset,
                        src_id,
                        src_tracking_rate
                        );
                }
            }
        } else {
            if (data_size >= source->param.audio.frame_size) {
                /* samples are enough, so update data status */
                data_status = AUDIO_DONGLE_DL_DATA_IN_STREAM;
            } else {
                COMMON_DONGLE_LOG_W("[Dongle Common][DL][AFE IN] scenario type %d, size %d < frame size %d ccni_wo[%d] cur_wo[%d]", 5,
                    source->scenario_type,
                    data_size,
                    source->param.audio.frame_size,
                    afe_in->cur_write_offset,
                    AFE_GET_REG(afe_in->afe_vul_cur_addr) - AFE_GET_REG(afe_in->afe_vul_base_addr)
                    );
            }
            /* Handler the stream */
            AudioCheckTransformHandle(source->transform);
        }
        #if defined AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE || defined AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE || defined AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE
        /* vdma tracking mode */
        if ((source->param.audio.AfeBlkControl.u4asrcflag) && (!afe_in->i2s_slv_flag)) {
            afe_in->i2s_slv_flag = true;
            vdma_channel_t dma_channel;
            vdma_status_t i2s_vdma_status;
            uint32_t port_tmp = 0;
            extern vdma_channel_t g_i2s_slave_vdma_channel_infra[];
            if (source->param.audio.audio_interface == HAL_AUDIO_INTERFACE_1) {
                port_tmp = 0;
            } else if (source->param.audio.audio_interface == HAL_AUDIO_INTERFACE_2) {
                port_tmp = 1;
            } else if (source->param.audio.audio_interface == HAL_AUDIO_INTERFACE_3) {
                port_tmp = 2;
            } else {
                COMMON_DONGLE_LOG_E("[Dongle Common][DL][AFE IN] start i2s slave audio_interface = %d fail", 1, source->param.audio.audio_interface);
                AUDIO_ASSERT(0);
            }
            dma_channel = g_i2s_slave_vdma_channel_infra[port_tmp * 2 + 1];
            i2s_vdma_status = vdma_start(dma_channel);
            if (i2s_vdma_status != VDMA_OK) {
                COMMON_DONGLE_LOG_E("[Dongle Common][DL][AFE IN] DSP DL start i2s slave set vdma_start fail %d", 1, i2s_vdma_status);
                AUDIO_ASSERT(0);
            }
            buffer_info->ReadOffset = buffer_info->WriteOffset; // clear the buffer
            uint8_t src_id = source->param.audio.AfeBlkControl.u4asrcid;
            /* BUG FIX HWSRC3 RG is not 0x100 * src_id */
            AFE_WRITE(ASM_CH01_OBUF_RDPNT +  0x100 * src_id, AFE_GET_REG(ASM_CH01_OBUF_WRPNT +  0x100 * src_id));
            COMMON_DONGLE_LOG_I("[Dongle Common][DL][AFE IN] VDMA Start ch %d", 1, dma_channel);
        }
        /* interconn tracking mode */
        // if ((source->param.audio.mem_handle.pure_agent_with_src) && (!g_i2s_slv_common_tracking_start_flag)) {
        //     // g_i2s_slv_common_tracking_start_flag = true;
        //     // /* start gpt timer to trigger vul1 irq */
        //     // hal_gpt_status_t gpt_status = HAL_GPT_STATUS_OK;
        //     // if (gpt_i2s_slv_trigger_handle == 0) {
        //     //     gpt_status = hal_gpt_sw_get_timer(&gpt_i2s_slv_trigger_handle);
        //     //     if (gpt_status != HAL_GPT_STATUS_OK) {
        //     //         BLE_LOG_E("[ble audio dongle][dl][afe in] get gpt handle fail %d", 1, gpt_status);
        //     //         AUDIO_ASSERT(0);
        //     //     }
        //     // }
        //     //gpt_status = hal_gpt_sw_start_timer_us(gpt_i2s_slv_trigger_handle, 2500, (hal_gpt_callback_t)ull_audio_v2_dongle_dl_i2s_slv_gpt_cb, c_handle);
        // }
        #endif /* AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE */
        // DSP_MW_LOG_E("TEST slave_%d asrc out buffer RPTR=WPTR, R=0x%x, W=0x%x, asrc in buffer, R=0x%x, W=0x%x", 5,
        //         0,
        //         AFE_GET_REG(ASM_CH01_OBUF_RDPNT + 0),
        //         AFE_GET_REG(ASM_CH01_OBUF_WRPNT + 0),
        //         AFE_GET_REG(ASM_CH01_IBUF_RDPNT + 0),
        //         AFE_GET_REG(ASM_CH01_IBUF_WRPNT + 0)
        //         );
    }
    /* update parameters to dongle */
    switch (dongle_type) {
#if defined AIR_BT_AUDIO_DONGLE_LINE_IN_ENABLE || defined AIR_BT_AUDIO_DONGLE_I2S_IN_ENABLE
        case AUDIO_DONGLE_TYPE_BT:
            //((bt_audio_dongle_handle_t *)dongle_handle)->buffer_default_output_size = buffer_output_size;
            ((bt_audio_dongle_handle_t *)dongle_handle)->data_status = data_status;
            ((bt_audio_dongle_handle_t *)dongle_handle)->first_packet_status = first_packet_status;
            break;
#endif
        default:
            break;
    }
}

#endif /* afe in */

void audio_dongle_sink_get_share_buffer_avail_size(SINK sink, uint32_t *avail_size)
{
    uint32_t wo = sink->streamBuffer.ShareBufferInfo.write_offset;
    uint32_t ro = sink->streamBuffer.ShareBufferInfo.read_offset;
    uint32_t length = sink->streamBuffer.ShareBufferInfo.length;
    if (ro < wo) {
        /* normal case */
        *avail_size = length - wo + ro;
    } else if (ro == wo) {
        if(sink->streamBuffer.ShareBufferInfo.bBufferIsFull == true) {
            /* buffer is full, so read_offset == write_offset */
            *avail_size = 0;
        } else {
            /* buffer is empty, so read_offset == write_offset */
            *avail_size = length;
        }
    } else {
        /* buffer wrapper case */
        *avail_size = ro - wo;
    }
}

#endif /* AIR_BLE_AUDIO_DONGLE_ENABLE */

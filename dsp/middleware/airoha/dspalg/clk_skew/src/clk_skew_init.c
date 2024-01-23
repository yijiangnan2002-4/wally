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

#include "clk_skew_init.h"
#include "dsp_callback.h"

#ifdef AIR_DCHS_MODE_ENABLE
#include "stream_dchs.h"
#endif

/**
 * stream_function_clock_skew_a2dp_initialize
 *
 * This function is used to init memory space for clock skew a2dp function
 *
 *
 * @para : Default parameter of callback function
 *
 */
bool stream_function_clock_skew_a2dp_initialize(void *para)
{
    DSP_CLOCK_SKEW_SETUP_t ClkSkewSetup;
    DSP_STREAMING_PARA_PTR stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    S32 ini_asi = 0;

    memset(&ClkSkewSetup,0,sizeof(DSP_CLOCK_SKEW_SETUP_t));

    ClkSkewSetup.calibrat_isr_debug_en = false;
    ClkSkewSetup.calibrate_isr_en = true;
    ClkSkewSetup.clk_skew_dir = CLK_SKEW_DL;
    ClkSkewSetup.clk_skew_div_mode = C_1_mode;
    ClkSkewSetup.clk_skew_order = C_Flp_Ord_5;
    ClkSkewSetup.debug_interval_s = 5;
    ClkSkewSetup.compensate_th_neg = -20000000;
    ClkSkewSetup.compensate_th_pos = 20000000;
    ClkSkewSetup.pcdc_en = true;
    ClkSkewSetup.rcdc_en = true;
    #ifdef AIR_DCHS_MODE_ENABLE
    if(dchs_get_device_mode() != DCHS_MODE_SINGLE){
        ClkSkewSetup.rcdc_en = false;
    }
    #endif


    ClkSkewSetup.ecdc_en = Clock_Skew_ECDC_Initialize(para, ClkSkewSetup.clk_skew_dir);

    ClkSkewSetup.rcdc_av_en = true;

    if(stream_ptr->source->param.n9_a2dp.codec_info.codec_cap.type == BT_A2DP_CODEC_VENDOR){
        ClkSkewSetup.rcdc_slow_adjustment_en = true;
    }

    ini_asi = stream_ptr->source->param.n9_a2dp.predict_asi;

    Clock_Skew_Initialize(para, &ClkSkewSetup, ini_asi);

    return FALSE;
}

/**
 * stream_function_clock_skew_hfp_dl_initialize
 *
 * This function is used to init memory space for clock skew hfp dl function
 *
 *
 * @para : Default parameter of callback function
 *
 */
bool stream_function_clock_skew_hfp_dl_initialize(void *para)
{
    DSP_CLOCK_SKEW_SETUP_t ClkSkewSetup;
    memset(&ClkSkewSetup,0,sizeof(DSP_CLOCK_SKEW_SETUP_t));
    ClkSkewSetup.calibrat_isr_debug_en = true;
    ClkSkewSetup.calibrate_isr_en = true;
    ClkSkewSetup.clk_skew_dir = CLK_SKEW_DL;
    ClkSkewSetup.clk_skew_div_mode = C_1_mode;// C_1_mode (1 sample/per frame) 
    ClkSkewSetup.clk_skew_order = C_Flp_Ord_1;// C_Flp_Ord_1(with 0 sample delay)
    ClkSkewSetup.debug_interval_s = 5;//debug log interval
    ClkSkewSetup.compensate_th_neg = -20000000;// 1sample = 26000000
    ClkSkewSetup.compensate_th_pos = 20000000;// 1sample = 26000000
    ClkSkewSetup.pcdc_en = false;
    ClkSkewSetup.rcdc_en = true;
    #ifdef AIR_DCHS_MODE_ENABLE
    if(dchs_get_device_mode() != DCHS_MODE_SINGLE){
        ClkSkewSetup.rcdc_en = false;
    }
    #endif
    ClkSkewSetup.ecdc_en = Clock_Skew_ECDC_Initialize(para, ClkSkewSetup.clk_skew_dir);
    Clock_Skew_Initialize(para, &ClkSkewSetup ,0);

    return FALSE;
}

/**
 * stream_function_clock_skew_hfp_ul_initialize
 *
 * This function is used to init memory space for clock skew hfp ul function
 *
 *
 * @para : Default parameter of callback function
 *
 */
bool stream_function_clock_skew_hfp_ul_initialize(void *para)
{
    DSP_CLOCK_SKEW_SETUP_t ClkSkewSetup;
    memset(&ClkSkewSetup,0,sizeof(DSP_CLOCK_SKEW_SETUP_t));
    ClkSkewSetup.calibrat_isr_debug_en = true;
    ClkSkewSetup.calibrate_isr_en = true;
    ClkSkewSetup.clk_skew_dir = CLK_SKEW_UL;
    ClkSkewSetup.clk_skew_div_mode = C_1_mode;// C_1_mode (1 sample/per frame) 
    ClkSkewSetup.clk_skew_order = C_Flp_Ord_1;// C_Flp_Ord_1(with 0 sample delay)
    ClkSkewSetup.debug_interval_s = 5;//debug log interval
    ClkSkewSetup.compensate_th_neg = -20000000;// 1sample = 26000000
    ClkSkewSetup.compensate_th_pos = 20000000;// 1sample = 26000000
    ClkSkewSetup.pcdc_en = false;
    ClkSkewSetup.rcdc_en = true;
    #ifdef AIR_DCHS_MODE_ENABLE
    if(dchs_get_device_mode() != DCHS_MODE_SINGLE){
        ClkSkewSetup.rcdc_en = false;
    }
    #endif
    ClkSkewSetup.ecdc_en = Clock_Skew_ECDC_Initialize(para, ClkSkewSetup.clk_skew_dir);
    Clock_Skew_Initialize(para, &ClkSkewSetup ,0);

    return FALSE;
}

/**
 * stream_function_clock_skew_le_music_initialize
 *
 * This function is used to init memory space for clock skew le dl function
 *
 *
 * @para : Default parameter of callback function
 *
 */
bool stream_function_clock_skew_le_music_initialize(void *para)
{
    DSP_CLOCK_SKEW_SETUP_t ClkSkewSetup;
    memset(&ClkSkewSetup,0,sizeof(DSP_CLOCK_SKEW_SETUP_t));
    ClkSkewSetup.calibrat_isr_debug_en = false;
    ClkSkewSetup.calibrate_isr_en = true;
    ClkSkewSetup.clk_skew_dir = CLK_SKEW_DL;
    ClkSkewSetup.clk_skew_div_mode = C_1_mode;
    ClkSkewSetup.clk_skew_order = C_Flp_Ord_5;
    ClkSkewSetup.debug_interval_s = 5;
    ClkSkewSetup.compensate_th_neg = -20000000;
    ClkSkewSetup.compensate_th_pos = 20000000;
    ClkSkewSetup.pcdc_en = false;
    ClkSkewSetup.rcdc_en = true;
    #ifdef AIR_DCHS_MODE_ENABLE
    if(dchs_get_device_mode() != DCHS_MODE_SINGLE){
        ClkSkewSetup.rcdc_en = false;
    }
    #endif
    ClkSkewSetup.ecdc_en = Clock_Skew_ECDC_Initialize(para, ClkSkewSetup.clk_skew_dir);
    Clock_Skew_Initialize(para, &ClkSkewSetup ,0);

    return FALSE;
}


/**
 * stream_function_clock_skew_le_call_dl_initialize
 *
 * This function is used to init memory space for clock skew le dl function
 *
 *
 * @para : Default parameter of callback function
 *
 */
bool stream_function_clock_skew_le_call_dl_initialize(void *para)
{
    DSP_CLOCK_SKEW_SETUP_t ClkSkewSetup;
    memset(&ClkSkewSetup,0,sizeof(DSP_CLOCK_SKEW_SETUP_t));
    ClkSkewSetup.calibrat_isr_debug_en = true;
    ClkSkewSetup.calibrate_isr_en = true;
    ClkSkewSetup.clk_skew_dir = CLK_SKEW_DL;
    ClkSkewSetup.clk_skew_div_mode = C_1_mode;
    ClkSkewSetup.clk_skew_order = C_Flp_Ord_1;
    ClkSkewSetup.debug_interval_s = 5;
    ClkSkewSetup.compensate_th_neg = -20000000;
    ClkSkewSetup.compensate_th_pos = 20000000;
    ClkSkewSetup.pcdc_en = false;
    ClkSkewSetup.rcdc_en = true;
    #ifdef AIR_DCHS_MODE_ENABLE
    if(dchs_get_device_mode() != DCHS_MODE_SINGLE){
        ClkSkewSetup.rcdc_en = false;
    }
    #endif
    ClkSkewSetup.ecdc_en = Clock_Skew_ECDC_Initialize(para, ClkSkewSetup.clk_skew_dir);
    Clock_Skew_Initialize(para, &ClkSkewSetup ,0);

    return FALSE;
}

/**
 * stream_function_clock_skew_le_call_ul_initialize
 *
 * This function is used to init memory space for clock skew le ul function
 *
 *
 * @para : Default parameter of callback function
 *
 */
bool stream_function_clock_skew_le_call_ul_initialize(void *para)
{
    DSP_CLOCK_SKEW_SETUP_t ClkSkewSetup;
    memset(&ClkSkewSetup,0,sizeof(DSP_CLOCK_SKEW_SETUP_t));
    ClkSkewSetup.calibrat_isr_debug_en = true;
    ClkSkewSetup.calibrate_isr_en = true;
    ClkSkewSetup.clk_skew_dir = CLK_SKEW_UL;
    ClkSkewSetup.clk_skew_div_mode = C_1_mode;
    ClkSkewSetup.clk_skew_order = C_Flp_Ord_1;
    ClkSkewSetup.debug_interval_s = 5;
    ClkSkewSetup.compensate_th_neg = -20000000;
    ClkSkewSetup.compensate_th_pos = 20000000;
    ClkSkewSetup.pcdc_en = false;
    ClkSkewSetup.rcdc_en = true;
    #ifdef AIR_DCHS_MODE_ENABLE
    if(dchs_get_device_mode() != DCHS_MODE_SINGLE){
        ClkSkewSetup.rcdc_en = false;
    }
    #endif

    Clock_Skew_Initialize(para, &ClkSkewSetup ,0);

    return FALSE;
}



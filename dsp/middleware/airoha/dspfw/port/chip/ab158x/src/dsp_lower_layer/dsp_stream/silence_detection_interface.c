/* Copyright Statement:
 *
 * (C) 2021  Airoha Technology Corp. All rights reserved.
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

#include "silence_detection_interface.h"
#include "hal_audio_volume.h"
#include "hal_audio_driver.h"
#include "audio_nvdm_common.h"
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
#include "scenario_ble_audio.h"
#endif /* AIR_BLE_AUDIO_DONGLE_ENABLE */
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
#include "scenario_ull_audio_v2.h"
#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE*/
#if defined(AIR_BT_AUDIO_DONGLE_ENABLE)
#include "scenario_bt_audio.h"
#endif /*AIR_BT_AUDIO_DONGLE_ENABLE */

#ifdef AIR_AUDIO_HARDWARE_ENABLE
#define abs32(x) ( (x >= 0) ? x : (-x) )
extern afe_volume_digital_control_t afe_digital_gain[AFE_HW_DIGITAL_GAIN_NUM];
extern afe_volume_analog_control_t afe_analog_gain[AFE_HW_ANALOG_GAIN_NUM];
extern hal_audio_performance_mode_t afe_adc_performance_mode[AFE_ANALOG_NUMBER];
#if defined(AIR_BTA_IC_PREMIUM_G2)
extern int32_t hal_audio_agent_user_count[HAL_AUDIO_AGENT_NUMBERS];
#else
extern bool hal_audio_status_get_agent_status(hal_audio_agent_t agent);
#endif
#ifdef MTK_ANC_ENABLE
extern bool g_anc_nle_apply;
#endif
static SD_INSTANCE_t SD_Parameter;
static U8 isInitial = 0;
static BOOL NvKeyfromCM4 = FALSE;
static BOOL IniDataUpdate = FALSE;
static S32 HW_Gain = 100;

U32 Get_AutoPowerOff_Time(void)
{
    return SD_Parameter.NvKey.AutoPowerOff_Time;
}

U32 Get_NLE_Time(void)
{
    return SD_Parameter.NvKey.NLE_Time;
}

U32 Get_ExtAmpOff_Time(void)
{
    return SD_Parameter.NvKey.ExtAmpOff_Time;
}

U8 Sink_Audio_SilenceDetection_Get_InitCnt(void)
{
    return isInitial;
}

void Sink_Audio_SilenceDetection_Load_Nvkey(void *nvkey)
{
    //DSP_MW_LOG_I("[SD]load nvkey",0);
    memcpy(&(SD_Parameter.NvKey), nvkey, sizeof(SD_NVKEY_STATE));
    NvKeyfromCM4 = TRUE;
}

void Sink_Audio_SilenceDetection_TH_Update(S32 hw_gain)
{
    S32 Gain = 0;
    if(hw_gain != HW_Gain){
        HW_Gain = hw_gain;
        Gain = HW_Gain/100;
        SD_Parameter.AutoPowerOff_TH_16 = (S32)afe_calculate_digital_gain_index((SD_Parameter.NvKey.AutoPowerOff_TH_dB)*100 - HW_Gain,0X7FFF);//16bits
        SD_Parameter.NLE_TH_16 = (S32)afe_calculate_digital_gain_index((SD_Parameter.NvKey.NLE_TH_dB)*100 - HW_Gain,0X7FFF);//16bits
        SD_Parameter.RSDM_TH_16 = (S32)afe_calculate_digital_gain_index((SD_Parameter.NvKey.RSDM_TH_dB)*100 - HW_Gain,0X7FFF);//16bits
        SD_Parameter.ExtAmpOff_TH_16 = (S32)afe_calculate_digital_gain_index((SD_Parameter.NvKey.ExtAmpOff_TH_dB)*100 - HW_Gain,0X7FFF);//16bits

        SD_Parameter.AutoPowerOff_TH_32 = (S32)afe_calculate_digital_gain_index((SD_Parameter.NvKey.AutoPowerOff_TH_dB)*100 - HW_Gain,0X7FFFFFFF);//32bits
        SD_Parameter.NLE_TH_32 = (S32)afe_calculate_digital_gain_index((SD_Parameter.NvKey.NLE_TH_dB)*100 - HW_Gain,0X7FFFFFFF);//32bits
        SD_Parameter.RSDM_TH_32 = (S32)afe_calculate_digital_gain_index((SD_Parameter.NvKey.RSDM_TH_dB)*100 - HW_Gain,0X7FFFFFFF);//32bits
        SD_Parameter.ExtAmpOff_TH_32 = (S32)afe_calculate_digital_gain_index((SD_Parameter.NvKey.ExtAmpOff_TH_dB)*100 - HW_Gain,0X7FFFFFFF);//32bits
        //DSP_MW_LOG_I("[SD]TH Update, HW Gain:%ddB",1,Gain);
        //DSP_MW_LOG_I("[SD]APO TH:%ddB,TH:%d(16Bits),TH:%d(32Bits)",3,SD_Parameter.NvKey.AutoPowerOff_TH_dB - Gain,SD_Parameter.AutoPowerOff_TH_16,SD_Parameter.AutoPowerOff_TH_32);
        //DSP_MW_LOG_I("[SD]NLE TH:%ddB,TH:%d(16Bits),TH:%d(32Bits)",3,SD_Parameter.NvKey.NLE_TH_dB - Gain,SD_Parameter.NLE_TH_16,SD_Parameter.NLE_TH_32);
        //DSP_MW_LOG_I("[SD]RSDM TH:%ddB,TH:%d(16Bits),TH:%d(32Bits)",3,SD_Parameter.NvKey.RSDM_TH_dB - Gain,SD_Parameter.RSDM_TH_16,SD_Parameter.RSDM_TH_32);
        //DSP_MW_LOG_I("[SD]EAO TH:%ddB,TH:%d(16Bits),TH:%d(32Bits)",3,SD_Parameter.NvKey.ExtAmpOff_TH_dB - Gain,SD_Parameter.ExtAmpOff_TH_16,SD_Parameter.ExtAmpOff_TH_32);
    }
}

void Sink_Audio_SilenceDetection_Register(SINK sink)
{
    FuncPtr APO_FunPtr = SD_Parameter.APO_FunPtr;
    FuncPtr NLE_FunPtr = SD_Parameter.NLE_FunPtr;
    FuncPtr EAO_FunPtr = SD_Parameter.EAO_FunPtr;

    if(NvKeyfromCM4){
        SD_Parameter.RegisteredSink[sink->type] = TRUE;
    }

    if((!isInitial) && NvKeyfromCM4){
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M,&(SD_Parameter.StartCnt));

        SD_Parameter.APO_MaxValue_16 = 0;
        SD_Parameter.NLE_MaxValue_16 = 0;
        SD_Parameter.EAO_MaxValue_16 = 0;
        SD_Parameter.APO_MaxValue_32 = 0;
        SD_Parameter.NLE_MaxValue_32 = 0;
        SD_Parameter.EAO_MaxValue_32 = 0;
        SD_Parameter.SampleCntAccumulate = 0;

        if(NvKeyfromCM4 && (!IniDataUpdate)){
            Sink_Audio_SilenceDetection_TH_Update(afe_digital_gain[AFE_HW_DIGITAL_GAIN1].index);
            IniDataUpdate = TRUE;
        }

        if(SD_Parameter.APO_SilenceFlag != TRUE){
            SD_Parameter.APO_SilenceFlag = TRUE;
            if(SD_Parameter.NvKey.APO_isEnable){
                APO_FunPtr(SD_Parameter.APO_SilenceFlag);
            }
        }

        if(SD_Parameter.NLE_SilenceFlag != TRUE){
            SD_Parameter.NLE_SilenceFlag = TRUE;
            if(SD_Parameter.NvKey.NLE_isEnable){
                NLE_FunPtr(SD_Parameter.NLE_SilenceFlag);
            }
        }

        if(SD_Parameter.EAO_SilenceFlag != TRUE){
            SD_Parameter.EAO_SilenceFlag = TRUE;
            if(SD_Parameter.NvKey.EAO_isEnable){
                EAO_FunPtr(SD_Parameter.EAO_SilenceFlag);
            }
        }
    }
    if(NvKeyfromCM4 && IniDataUpdate){
        isInitial++;
    }
    //DSP_MW_LOG_I("[SD]Silence Detection Init SinkType:0x%x Cnt:%d",2,sink->type,isInitial);
}

void Sink_Audio_SilenceDetection_Unregister(SINK sink)
{
    FuncPtr APO_FunPtr = SD_Parameter.APO_FunPtr;
    FuncPtr NLE_FunPtr = SD_Parameter.NLE_FunPtr;
    FuncPtr EAO_FunPtr = SD_Parameter.EAO_FunPtr;

    SD_Parameter.RegisteredSink[sink->type] = FALSE;
    isInitial--;

    if(!isInitial){

        if(SD_Parameter.APO_SilenceFlag != TRUE){
            SD_Parameter.APO_SilenceFlag = TRUE;
            if(SD_Parameter.NvKey.APO_isEnable){
                APO_FunPtr(SD_Parameter.APO_SilenceFlag);
            }
        }

        if(SD_Parameter.NLE_SilenceFlag != TRUE){
            SD_Parameter.NLE_SilenceFlag = TRUE;
            if(SD_Parameter.NvKey.NLE_isEnable){
                NLE_FunPtr(SD_Parameter.NLE_SilenceFlag);
            }
        }

        if(SD_Parameter.EAO_SilenceFlag != TRUE){
            SD_Parameter.EAO_SilenceFlag = TRUE;
            if(SD_Parameter.NvKey.EAO_isEnable){
                EAO_FunPtr(SD_Parameter.EAO_SilenceFlag);
            }
        }
    }
    //DSP_MW_LOG_I("[SD]Silence Detection Close SinkType:0x%x Cnt:%d",2,sink->type,isInitial);
}

void Sink_Audio_SilenceDetection_Init(VOID *APO_FunPtr,VOID *NLE_FunPtr,VOID *EAO_FunPtr,VOID *RSDM_FunPtr)
{
    U32 i = 0;
    for(i = 0;i < SINK_TYPE_MAX;i++){
        SD_Parameter.RegisteredSink[i] = FALSE;
    }
    SD_Parameter.APO_FunPtr = APO_FunPtr;
    SD_Parameter.NLE_FunPtr = NLE_FunPtr;
    SD_Parameter.RSDM_FunPtr = RSDM_FunPtr;
    SD_Parameter.EAO_FunPtr = EAO_FunPtr;

    SD_Parameter.APO_MaxValue_16 = 0;
    SD_Parameter.RSDM_MaxValue_16 = 0;
    SD_Parameter.NLE_MaxValue_16 = 0;
    SD_Parameter.RSDM_MaxValue_16 = 0;
    SD_Parameter.EAO_MaxValue_16 = 0;
    SD_Parameter.APO_MaxValue_32 = 0;
    SD_Parameter.NLE_MaxValue_32 = 0;
    SD_Parameter.RSDM_MaxValue_32 = 0;
    SD_Parameter.EAO_MaxValue_32 = 0;

    SD_Parameter.StartCnt = 0XFFFFFFFF;
    SD_Parameter.Mutex = FALSE;

    if(NvKeyfromCM4){
        Sink_Audio_SilenceDetection_TH_Update(afe_digital_gain[AFE_HW_DIGITAL_GAIN1].index);
        IniDataUpdate = TRUE;
    }

    //DSP_MW_LOG_I("[SD]APO_EN:%d, Time:%ds",2,SD_Parameter.NvKey.APO_isEnable,SD_Parameter.NvKey.AutoPowerOff_Time);
    //DSP_MW_LOG_I("[SD]NLE_EN:%d, Time:%ds",2,SD_Parameter.NvKey.NLE_isEnable,SD_Parameter.NvKey.NLE_Time);
    //DSP_MW_LOG_I("[SD]RSDM_EN:%d, Time:%ds",2,SD_Parameter.NvKey.RSDM_isEnable,SD_Parameter.NvKey.RSDM_Time);
    //DSP_MW_LOG_I("[SD]EAO_EN:%d, Time:%ds",2,SD_Parameter.NvKey.EAO_isEnable,SD_Parameter.NvKey.ExtAmpOff_Time);

    SD_Parameter.DetectTime_s = Silence_DetectTime_s;
    SD_Parameter.SampleCntAccumulate = 0;

    SD_Parameter.APO_SilenceFlag = FALSE;
    SD_Parameter.NLE_SilenceFlag = FALSE;
    SD_Parameter.RSDM_SilenceFlag = FALSE;
    SD_Parameter.EAO_SilenceFlag = FALSE;

}
void Sink_Audio_NLE_Init(void)
{
    SD_Parameter.NLE_First_Silence = TRUE;
    SD_Parameter.RSDM_First_Silence = TRUE;
    SD_Parameter.NLE_SilenceFlag = TRUE;
    SD_Parameter.RSDM_SilenceFlag = FALSE;
    SD_Parameter.NLE_MaxValue_16 = 0;
    SD_Parameter.NLE_MaxValue_32 = 0;
    SD_Parameter.RSDM_MaxValue_16 = 0;
    SD_Parameter.RSDM_MaxValue_32 = 0;
    SD_Parameter.NvKey.NLE_isEnable = FALSE;
    AFE_SET_REG(AUDIO_TOP_CON0, 0 << 15, 1 << 15);                            // turn on nle clock source
    AFE_SET_REG(AFE_DL_NLE_L_CFG1, 0, 0xFFFFFFFF);
    AFE_SET_REG(AFE_DL_NLE_R_CFG1, 0, 0xFFFFFFFF);
}

void Sink_Audio_NLE_Deinit(void)
{
    AFE_SET_REG(AUDIO_TOP_CON0, 1 << 15, 1 << 15);                            // turn off nle clock source
}

void Sink_Audio_NLE_Enable(BOOL enable)
{
    if (SD_Parameter.NvKey.NLE_isEnable != enable) {
        SD_Parameter.NLE_SilenceFlag = FALSE;
        SD_Parameter.NLE_FunPtr(SD_Parameter.NLE_SilenceFlag);
        hal_audio_dl_set_nle_enable(enable);
        SD_Parameter.NvKey.NLE_isEnable = enable;
    }
}

#define ABS(x) ((x)<0 ? (-x) : (x))
void Sink_Audio_SilenceDetection(VOID* SrcBuf, U16 CopySize, SINK sink)
{
    FuncPtr APO_FunPtr = SD_Parameter.APO_FunPtr;
    FuncPtr NLE_FunPtr = SD_Parameter.NLE_FunPtr;
    FuncPtr RSDM_FunPtr = SD_Parameter.RSDM_FunPtr;
    FuncPtr EAO_FunPtr = SD_Parameter.EAO_FunPtr;
    S32 AutoPowerOff_TH;
    S32 ExtAmpOff_TH;
    S32 NLE_TH;
    S32 RSDM_TH;
    U32 format = sink->param.audio.format_bytes;
    U32 SampleCnt = CopySize/format;
    S32 *APO_Max;
    S32 *NLE_Max;
    S32 *RSDM_Max;
    S32 *EAO_Max;
    U32 NowCnt;
    U32 NLE_NowCnt;
    U32 RSDM_NowCnt;
    S16 *Buf_16;
    S32 *Buf_32;
    U32 i = 0;
    U32 NLE_Silence_Time_us = 0;
    U32 RSDM_Silence_Time_us = 0;
    U32 NLE_Duration_Count = 0;
    U32 RSDM_Duration_Count = 0;
    afe_hardware_digital_gain_t gain_select;
    BOOL NLE_MUTEX = FALSE;
    BOOL RSDM_MUTEX = FALSE;
    SD_Parameter.RSDM_MaxValue_16 = 0;
    SD_Parameter.RSDM_MaxValue_32 = 0;
    SD_Parameter.NvKey.RSDM_isEnable = FALSE;

    #if defined(AIR_NLE_ENABLE) || defined(AIR_RESET_SDM_ENABLE)
    if (
    #if defined(AIR_BTA_IC_PREMIUM_G2)
        ((afe_analog_gain[AFE_HW_ANALOG_GAIN_OUTPUT].analog_mode == HAL_AUDIO_ANALOG_OUTPUT_CLASSG) ||
         (afe_analog_gain[AFE_HW_ANALOG_GAIN_OUTPUT].analog_mode == HAL_AUDIO_ANALOG_OUTPUT_CLASSAB))
        && (!hal_audio_agent_user_count[HAL_AUDIO_AGENT_DEVICE_ANC])
        && (!hal_audio_agent_user_count[HAL_AUDIO_AGENT_DEVICE_SIDETONE])
    #else
        ((afe_analog_gain[AFE_HW_ANALOG_GAIN_OUTPUT].analog_mode == HAL_AUDIO_ANALOG_OUTPUT_CLASSG2) ||
         (afe_analog_gain[AFE_HW_ANALOG_GAIN_OUTPUT].analog_mode == HAL_AUDIO_ANALOG_OUTPUT_CLASSAB) ||
         (afe_analog_gain[AFE_HW_ANALOG_GAIN_OUTPUT].analog_mode == HAL_AUDIO_ANALOG_OUTPUT_CLASSG3))
        && (!hal_audio_status_get_agent_status(HAL_AUDIO_AGENT_DEVICE_ANC))
        && (!hal_audio_status_get_agent_status(HAL_AUDIO_AGENT_DEVICE_SIDETONE))
    #endif
    ) {
        #ifdef AIR_NLE_ENABLE
            Sink_Audio_NLE_Enable(TRUE);
            SD_Parameter.NvKey.NLE_TH_dB = NLE_Silence_DetectTH_dB;
        #endif
        #ifdef AIR_RESET_SDM_ENABLE
            SD_Parameter.NvKey.RSDM_isEnable = TRUE;
        #endif
            if (((afe_digital_gain[AFE_HW_DIGITAL_GAIN1].register_value == 0) || (afe_digital_gain[AFE_HW_DIGITAL_GAIN1].register_value == 0xFFFFFFFF)) &&
                       ((afe_digital_gain[AFE_HW_DIGITAL_GAIN2].register_value == 0) || (afe_digital_gain[AFE_HW_DIGITAL_GAIN2].register_value == 0xFFFFFFFF)) &&
                       ((afe_digital_gain[AFE_HW_DIGITAL_GAIN3].register_value == 0) || (afe_digital_gain[AFE_HW_DIGITAL_GAIN3].register_value == 0xFFFFFFFF)))
            {
                #ifdef AIR_NLE_ENABLE
                    SD_Parameter.NLE_First_Silence = TRUE;
                    NLE_MUTEX = TRUE;
                    if (SD_Parameter.NLE_SilenceFlag == FALSE) {
                        SD_Parameter.NLE_SilenceFlag = TRUE;
                        NLE_FunPtr(SD_Parameter.NLE_SilenceFlag);
                    }
                #endif
                #ifdef AIR_RESET_SDM_ENABLE
                    SD_Parameter.RSDM_First_Silence = TRUE;
                    RSDM_MUTEX = TRUE;
                    if (SD_Parameter.RSDM_SilenceFlag == FALSE) {
                        SD_Parameter.RSDM_SilenceFlag = TRUE;
                        RSDM_FunPtr(SD_Parameter.RSDM_SilenceFlag);
                    }
                #endif
            }
    } else {
        #ifdef AIR_NLE_ENABLE
            SD_Parameter.NLE_First_Silence = TRUE;
            NLE_MUTEX = TRUE;
            Sink_Audio_NLE_Enable(FALSE);
            if ((AFE_GET_REG(AFE_DL_NLE_L_CFG1) & 0x80) && (AFE_GET_REG(AFE_DL_NLE_R_CFG1) & 0x80)) { //error handling
                DSP_MW_LOG_I("[NLE DEBUG] AFE_DL_NLE_L_CFG0:0x%x, AFE_DL_NLE_R_CFG0:0x%x",2,AFE_GET_REG(AFE_DL_NLE_L_CFG1),AFE_GET_REG(AFE_DL_NLE_R_CFG1));
                SD_Parameter.NvKey.NLE_isEnable = FALSE;
                hal_audio_dl_set_nle_enable(FALSE);
            }
        #endif
        #ifdef AIR_RESET_SDM_ENABLE
            SD_Parameter.RSDM_First_Silence = TRUE;
            RSDM_MUTEX = TRUE;
            if (SD_Parameter.RSDM_SilenceFlag == TRUE) {
                SD_Parameter.RSDM_SilenceFlag = FALSE;
            }
        #endif
    }
    #endif

    if(SD_Parameter.NvKey.RSDM_isEnable){
        gain_select = AFE_HW_DIGITAL_GAIN1;
        RSDM_Silence_Time_us = RSDM_Silence_DetectTime_OTR_us;
        if (sink->scenario_type == AUDIO_SCENARIO_TYPE_VP) {
            gain_select = AFE_HW_DIGITAL_GAIN2;
        } else if (sink->scenario_type == AUDIO_SCENARIO_TYPE_A2DP) {
            RSDM_Silence_Time_us = RSDM_Silence_DetectTime_A2DP_us;
        }
    }
    if(SD_Parameter.NvKey.NLE_isEnable){
        NLE_Silence_Time_us = NLE_Silence_DetectTime_us;
    }

    if((SD_Parameter.NvKey.APO_isEnable || SD_Parameter.NvKey.EAO_isEnable || SD_Parameter.NvKey.NLE_isEnable || SD_Parameter.NvKey.RSDM_isEnable)){

        Sink_Audio_SilenceDetection_TH_Update(afe_digital_gain[AFE_HW_DIGITAL_GAIN1].index);

        if(format == 2){//16bits
            Buf_16 = (S16*)SrcBuf;
            APO_Max = &(SD_Parameter.APO_MaxValue_16);
            NLE_Max = &(SD_Parameter.NLE_MaxValue_16);
            RSDM_Max = &(SD_Parameter.RSDM_MaxValue_16);
            EAO_Max = &(SD_Parameter.EAO_MaxValue_16);
            AutoPowerOff_TH = SD_Parameter.AutoPowerOff_TH_16;
            ExtAmpOff_TH = SD_Parameter.ExtAmpOff_TH_16;
            NLE_TH = SD_Parameter.NLE_TH_16;
            RSDM_TH = SD_Parameter.RSDM_TH_16;
        }else if(format == 4){//32bits
            Buf_32 = (S32*)SrcBuf;
            APO_Max = &(SD_Parameter.APO_MaxValue_32);
            NLE_Max = &(SD_Parameter.NLE_MaxValue_32);
            RSDM_Max = &(SD_Parameter.RSDM_MaxValue_32);
            EAO_Max = &(SD_Parameter.EAO_MaxValue_32);
            AutoPowerOff_TH = SD_Parameter.AutoPowerOff_TH_32;
            ExtAmpOff_TH = SD_Parameter.ExtAmpOff_TH_32;
            NLE_TH = SD_Parameter.NLE_TH_32;
            RSDM_TH = SD_Parameter.RSDM_TH_32;
        }
        S32 Max = 0;
        if (format == 2) {//16bits
            SD_Parameter.NLE_MaxValue_16 = ABS(*(Buf_16));
            SD_Parameter.RSDM_MaxValue_16 = ABS(*(Buf_16));
            for (i = 0; i < SampleCnt; i++) {
                S16 value = ABS(*(Buf_16+i));
                if (value > Max) {
                    Max = value;
                }
            }
        } else if (format == 4) {//32bits
            SD_Parameter.NLE_MaxValue_32 = ABS(*(Buf_32));
            SD_Parameter.RSDM_MaxValue_32 = ABS(*(Buf_32));
            for (i = 0; i < SampleCnt; i++) {
                S32 value = ABS(*(Buf_32+i));
                if (value > Max) {
                    Max = value;
                }
            }
        }
        if(sink->type != SINK_TYPE_VP_AUDIO){
            *APO_Max = Max;
        }
        *NLE_Max = Max;
        *RSDM_Max = Max;
        *EAO_Max = Max;

        if(SD_Parameter.NvKey.EAO_isEnable && (*EAO_Max >= ExtAmpOff_TH) && SD_Parameter.EAO_SilenceFlag == TRUE){
            SD_Parameter.EAO_SilenceFlag = FALSE;
            EAO_FunPtr(SD_Parameter.EAO_SilenceFlag);
        }
        if(SD_Parameter.NvKey.NLE_isEnable && (*NLE_Max >= ABS(NLE_TH)) && (NLE_MUTEX == FALSE)){
            SD_Parameter.NLE_First_Silence = TRUE;
            if (SD_Parameter.NLE_SilenceFlag == TRUE) {
                SD_Parameter.NLE_SilenceFlag = FALSE;
                NLE_FunPtr(SD_Parameter.NLE_SilenceFlag);
            }
        }
        if(SD_Parameter.NvKey.RSDM_isEnable && (*RSDM_Max > 0) && (RSDM_MUTEX == FALSE)){
            SD_Parameter.RSDM_First_Silence = TRUE;
            if (SD_Parameter.RSDM_SilenceFlag == TRUE) {
                SD_Parameter.RSDM_SilenceFlag = FALSE;
            }
        }

        if(SD_Parameter.Mutex == FALSE){
            SD_Parameter.Mutex = TRUE;
            hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M,&NowCnt);
            //DSP_MW_LOG_I("[SD]APO_Max:%d|0x%x, NLE_Max:%d|0x%x, NRSDM_Max:%d|0x%x, EAO_Max:%d|0x%x",8,*APO_Max,*APO_Max,*NLE_Max,*NLE_Max,*RSDM_Max,*RSDM_Max,*EAO_Max,*EAO_Max);

            if((SD_Parameter.NvKey.NLE_isEnable) && (NLE_MUTEX == FALSE)){
                //DSP_MW_LOG_I("#[NLE DEBUG]# %d, %d, %d, %d",4,SD_Parameter.NLE_MaxValue_32,SD_Parameter.NLE_TH_32,SD_Parameter.NLE_MaxValue_16,SD_Parameter.NLE_TH_16);
                if(((format == 4) && (ABS(SD_Parameter.NLE_MaxValue_32) < ABS(SD_Parameter.NLE_TH_32))) || ((format == 2) && (ABS(SD_Parameter.NLE_MaxValue_16) < ABS(SD_Parameter.NLE_TH_16)))){
                    //DSP_MW_LOG_I("#[NLE DEBUG] on 3# %d,%d,%d,%d",4,*NLE_Max,(SD_Parameter.NLE_TH_32),abs(SD_Parameter.NLE_MaxValue_16),abs(SD_Parameter.NLE_TH_16));
                    if (SD_Parameter.NLE_First_Silence) {
                        //printf("[DEBUG 0] NLE_Duration_Count: %d",NLE_Duration_Count);
                        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &SD_Parameter.NLE_StartCnt); //silence timer start
                        SD_Parameter.NLE_First_Silence = FALSE;
                    }
                    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &NLE_NowCnt);
                    if (SD_Parameter.NLE_StartCnt) {
                        hal_gpt_get_duration_count(SD_Parameter.NLE_StartCnt, NLE_NowCnt, &NLE_Duration_Count);
                    }
                    //printf("[DEBUG 1] NLE_SilenceFlag:%d, NLE_StartCnt:%d, NLE_NowCnt:%d, NLE_Duration_Count:%d",SD_Parameter.NLE_SilenceFlag,SD_Parameter.NLE_StartCnt,NLE_NowCnt,NLE_Duration_Count);
                    if (NLE_Duration_Count >= NLE_Silence_Time_us && (SD_Parameter.NLE_SilenceFlag == FALSE)) {
                        //printf("[DEBUG 2] NLE_Duration_Count: %d",NLE_Duration_Count);
                        SD_Parameter.NLE_SilenceFlag = TRUE;
                        NLE_FunPtr(SD_Parameter.NLE_SilenceFlag);
                    }
                }
            }
            if((SD_Parameter.NvKey.RSDM_isEnable) && (RSDM_MUTEX == FALSE)){
                if(((format == 4) && (SD_Parameter.RSDM_MaxValue_32 == 0)) || ((format == 2) && (SD_Parameter.RSDM_MaxValue_16 == 0))){
                    if (SD_Parameter.RSDM_First_Silence) {
                        //printf("[DEBUG] RSDM_Duration_Count 1: %d",RSDM_Duration_Count);
                        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &SD_Parameter.RSDM_StartCnt); //silence timer start
                        SD_Parameter.RSDM_First_Silence = FALSE;
                    }
                    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &RSDM_NowCnt);
                    if (SD_Parameter.RSDM_StartCnt) {
                        hal_gpt_get_duration_count(SD_Parameter.RSDM_StartCnt, RSDM_NowCnt, &RSDM_Duration_Count);
                    }
                    if (RSDM_Duration_Count >= RSDM_Silence_Time_us && (SD_Parameter.RSDM_SilenceFlag == FALSE)) {
                        //printf("[DEBUG] RSDM_Duration_Count: %d",RSDM_Duration_Count);
                        SD_Parameter.RSDM_SilenceFlag = TRUE;
                        RSDM_FunPtr(SD_Parameter.RSDM_SilenceFlag);
                    }
                }
            }

            if((NowCnt - SD_Parameter.StartCnt) >= (SD_Parameter.DetectTime_s*1000000) && SD_Parameter.RegisteredSink[sink->type]){
                SD_Parameter.StartCnt = NowCnt;

                if(SD_Parameter.NvKey.APO_isEnable){
                    if((SD_Parameter.APO_MaxValue_32 < SD_Parameter.AutoPowerOff_TH_32) && (SD_Parameter.APO_MaxValue_16 < SD_Parameter.AutoPowerOff_TH_16) && (SD_Parameter.APO_SilenceFlag == FALSE)){
                        SD_Parameter.APO_SilenceFlag = TRUE;
                        APO_FunPtr(SD_Parameter.APO_SilenceFlag);
                    }else if(((SD_Parameter.APO_MaxValue_32 >= SD_Parameter.AutoPowerOff_TH_32) || (SD_Parameter.APO_MaxValue_16 >= SD_Parameter.AutoPowerOff_TH_16)) && SD_Parameter.APO_SilenceFlag == TRUE){
                        SD_Parameter.APO_SilenceFlag = FALSE;
                        APO_FunPtr(SD_Parameter.APO_SilenceFlag);
                    }
                }

                if(SD_Parameter.NvKey.EAO_isEnable){
                    if((SD_Parameter.EAO_MaxValue_32 < SD_Parameter.ExtAmpOff_TH_32) && (SD_Parameter.EAO_MaxValue_16 < SD_Parameter.ExtAmpOff_TH_16)){
                        SD_Parameter.EAO_SilenceFlag = TRUE;
                        EAO_FunPtr(SD_Parameter.EAO_SilenceFlag);
                    }
                }

                SD_Parameter.APO_MaxValue_16 = 0;
                SD_Parameter.EAO_MaxValue_16 = 0;
                SD_Parameter.APO_MaxValue_32 = 0;
                SD_Parameter.EAO_MaxValue_32 = 0;
            }
            SD_Parameter.Mutex = FALSE;
        }


    }

    return;
}
#endif

void SilenceDetection_Scenario_Init(void *arg)
{
    audio_scenario_type_t scenario = (audio_scenario_type_t)arg;

    switch (scenario)
    {
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1:
            ull_audio_v2_dongle_silence_detection_init(scenario);
            break;
#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE*/
#if defined(AIR_BT_AUDIO_DONGLE_ENABLE) && defined(AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE)
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
            bt_audio_dongle_silence_detection_init(scenario);
            break;
#endif /* AIR_BT_AUDIO_DONGLE_ENABLE*/

        default:
            DSP_MW_LOG_E("[SD] scenario is not supported, %d", 1, scenario);
            break;
    }
}

void SilenceDetection_Scenario_Deinit(void *arg)
{
    audio_scenario_type_t scenario = (audio_scenario_type_t)arg;

    switch (scenario)
    {
#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1:
            ull_audio_v2_dongle_silence_detection_deinit(scenario);
            break;
#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE*/

#if defined(AIR_BT_AUDIO_DONGLE_ENABLE) && defined(AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE)
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
            bt_audio_dongle_silence_detection_deinit(scenario);
            break;
#endif /* AIR_BT_AUDIO_DONGLE_ENABLE*/

        default:
            DSP_MW_LOG_E("[SD] scenario is not supported, %d", 1, scenario);
            break;
    }
}

void SilenceDetection_Scenario_Enable(void *arg)
{
    audio_scenario_type_t scenario = (audio_scenario_type_t)arg;

    switch (scenario)
    {
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
        case AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1:
            ble_dongle_silence_detection_enable(scenario);
            break;
#endif /* AIR_BLE_AUDIO_DONGLE_ENABLE */

#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1:
            ull_audio_v2_dongle_silence_detection_enable(scenario);
            break;
#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE*/

#if defined(AIR_BT_AUDIO_DONGLE_ENABLE) && defined(AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE)
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
            bt_audio_dongle_silence_detection_enable(scenario);
            break;
#endif /* AIR_BT_AUDIO_DONGLE_ENABLE*/

        default:
            DSP_MW_LOG_E("[SD] scenario is not supported, %d", 1, scenario);
            break;
    }
}

void SilenceDetection_Scenario_Disable(void *arg)
{
    audio_scenario_type_t scenario = (audio_scenario_type_t)arg;

    switch (scenario)
    {
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
        case AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1:
            ble_dongle_silence_detection_disable(scenario);
            break;
#endif /* AIR_BLE_AUDIO_DONGLE_ENABLE */

#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1:
            ull_audio_v2_dongle_silence_detection_disable(scenario);
            break;
#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE*/

#if defined(AIR_BT_AUDIO_DONGLE_ENABLE) && defined(AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE)
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
            bt_audio_dongle_silence_detection_disable(scenario);
            break;
#endif /* AIR_BT_AUDIO_DONGLE_ENABLE*/

        default:
            DSP_MW_LOG_E("[SD] scenario is not supported, %d", 1, scenario);
            break;
    }
}

void SilenceDetection_Scenario_Process(void *arg)
{
    audio_scenario_type_t scenario = (audio_scenario_type_t)arg;

    switch (scenario)
    {
#if defined(AIR_BLE_AUDIO_DONGLE_ENABLE)
        case AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BLE_AUDIO_DONGLE_MUSIC_USB_IN_1:
            ble_dongle_silence_detection_process(scenario);
            break;
#endif /* AIR_BLE_AUDIO_DONGLE_ENABLE */

#if defined(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_ULL_AUDIO_V2_DONGLE_DL_USB_IN_1:
            ull_audio_v2_dongle_silence_detection_process(scenario);
            break;
#endif /* AIR_ULL_AUDIO_V2_DONGLE_ENABLE*/

#if defined(AIR_BT_AUDIO_DONGLE_ENABLE) && defined(AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE)
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_USB_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_USB_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_A2DP_AFE_IN_2:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_0:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_1:
        case AUDIO_SCENARIO_TYPE_BT_AUDIO_DONGLE_DL_HFP_AFE_IN_2:
            bt_audio_dongle_silence_detection_process(scenario);
            break;
#endif /* AIR_BT_AUDIO_DONGLE_ENABLE*/

        default:
            DSP_MW_LOG_E("[SD] scenario is not supported, %d", 1, scenario);
            break;
    }
}

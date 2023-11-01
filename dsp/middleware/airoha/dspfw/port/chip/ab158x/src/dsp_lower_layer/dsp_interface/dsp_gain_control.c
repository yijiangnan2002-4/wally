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

#include "types.h"
#include "dsp_audio_ctrl.h"
#include "stream_audio_driver.h"
#include "hal_audio.h"
#include "audio_nvdm_common.h"
#include "dsp_temp.h"

#include "hal_audio_control.h"
#include "hal_audio_driver.h"
#include "hal_audio_volume.h"
#include "hal_gpt.h"

#if defined(AIR_AUDIO_VOLUME_MONITOR_ENABLE)
#include "volume_estimator_interface.h"
#endif

/******************************************************************************
 * Definitions
 ******************************************************************************/
extern void hal_audio_set_stream_in_volume_for_multiple_microphone(uint16_t volume_index0, uint16_t volume_index1, hal_audio_input_gain_select_t gain_select);
extern hal_audio_performance_mode_t hal_get_adc_performance_mode(afe_analog_select_t analog_select);
extern bool hal_audio_device_set_dac(hal_audio_device_parameter_dac_t *handle, hal_audio_control_t device, hal_audio_control_status_t control);

typedef enum DSP_ANA_GAIN_IDX_e {
    ANA_OUT_GAIN_NEG_36_DB,
    ANA_OUT_GAIN_NEG_33_DB,
    ANA_OUT_GAIN_NEG_30_DB,
    ANA_OUT_GAIN_NEG_27_DB,
    ANA_OUT_GAIN_NEG_24_DB,
    ANA_OUT_GAIN_NEG_21_DB,
    ANA_OUT_GAIN_NEG_18_DB,
    ANA_OUT_GAIN_NEG_15_DB,
    ANA_OUT_GAIN_NEG_12_DB,
    ANA_OUT_GAIN_NEG_9_DB,
    ANA_OUT_GAIN_NEG_6_DB,
    ANA_OUT_GAIN_NEG_5_DB,
    ANA_OUT_GAIN_NEG_4_DB,
    ANA_OUT_GAIN_NEG_3_DB,
    ANA_OUT_GAIN_NEG_2_DB,
    ANA_OUT_GAIN_NEG_1_DB,
    ANA_OUT_GAIN_0_DB,
    ANA_OUT_GAIN_POS_1_DB,
    ANA_OUT_GAIN_POS_2_DB,
    ANA_OUT_GAIN_POS_3_DB,
    ANA_OUT_GAIN_POS_6_DB,
    ANA_OUT_GAIN_POS_9_DB,
    ANA_OUT_GAIN_MAX_NO,
} DSP_ANA_GAIN_IDX_t;


const S32 DSP_ANA_GAIN_TABLE[] = {
    0x00000000, // -36 dB
    0x00000001, // -33 dB
    0x00000003, // -30 dB
    0x00000007, // -27 dB
    0x0000000F, // -24 dB
    0x0000001F, // -21 dB
    0x0000003F, // -18 dB
    0x0000007F, // -15 dB
    0x000000FF, // -12 dB
    0x000001FF, // - 9 dB
    0x000003FF, // - 6 dB
    0x000007FF, // - 5 dB
    0x00000FFF, // - 4 dB
    0x00001FFF, // - 3 dB
    0x00003FFF, // - 2 dB
    0x00007FFF, // - 1 dB
    0x0000FFFF, //   0 dB
    0x0001FFFF, //   1 dB
    0x0003FFFF, //   2 dB
    0x0007FFFF, //   3 dB
    0x000FFFFF, //   6 dB
    0x001FFFFF, //   9 dB
};

typedef enum DSP_AU_CTRL_CH_e {
    AUDIO_CTRL_CHANNEL_L,
    AUDIO_CTRL_CHANNEL_R,
} DSP_AU_CTRL_CH_t;

#define TOTAL_ANALOG_OUT_GAIN_STEP (sizeof(DSP_ANA_GAIN_TABLE)/sizeof(DSP_ANA_GAIN_TABLE[0]))


/******************************************************************************
 * Function Prototypes
 ******************************************************************************/
VOID DSP_GC_Init(VOID);
VOID DSP_GC_LoadAnalogOutGainTableByAudioComponent(AUDIO_GAIN_COMPONENT_t Component);
VOID DSP_GC_LoadAnalogInGainTableByAudioComponent(AUDIO_GAIN_COMPONENT_t Component);
VOID DSP_GC_LoadAInPGTableByAudioComponent(AUDIO_GAIN_COMPONENT_t Component);
VOID DSP_GC_LoadDigitalInGainTableByAudioComponent(AUDIO_GAIN_COMPONENT_t Component);
VOID DSP_GC_LoadDInPGTableByAudioComponent(AUDIO_GAIN_COMPONENT_t Component);
S16 DSP_GC_ConvertQ11Form(S16 Value);
S32 DSP_GC_ConvertQ11Form_32bit(S16 Value);
S16 DSP_GC_ConvertPercentageToIdx(S16 Percentage, S16 TotalIndex);

/* Digital Out */
S16 DSP_GC_GetDigitalGainIndexFromComponent(AUDIO_GAIN_COMPONENT_t Component, S16 TotalIndex);
S16 DSP_GC_GetDigitalOutLevel(AUDIO_GAIN_COMPONENT_t Component);
VOID DSP_GC_SetDigitalOutLevel(AUDIO_GAIN_COMPONENT_t Component, S16 Gain);
S16 DSP_GC_GetDigitalOutGain_AT(VOID);

/* Digital In */
S16 DSP_GC_GetDigitalInLevel(VOID);
VOID DSP_GC_SetDigitalInLevel(S16 PercentageGain);
S16 DSP_GC_GetDigitalInLevelByPercentageTable(VOID);
VOID DSP_GC_SetDigitalInLevelByGainTable(AUDIO_GAIN_COMPONENT_t Component);
VOID DSP_GC_LoadDigitalInLevelToDfe(VOID);

/* Analog Out */
S16 DSP_GC_GetAnalogOutLevel(VOID);
VOID DSP_GC_SetAnalogOutLevel(S16 PercentageGain);
VOID DSP_GC_SetAnalogOutScaleByGainTable(AUDIO_GAIN_COMPONENT_t Component);
VOID DSP_GC_LoadAnalogOutLevelToAfe(VOID);

VOID DSP_GC_SetAnalogOutOnSequence(VOID);
VOID DSP_GC_SetAnalogOutOffSequence(VOID);
DSP_ANA_GAIN_IDX_t DSP_GC_GetCurrTabIdx(VOID);
DSP_ANA_GAIN_IDX_t DSP_GC_SearchAnalogGainTableIndex(S32 Gain);
VOID DSP_GC_SetToTargetGainByStepFunction(DSP_ANA_GAIN_IDX_t InitialTabIdx, DSP_ANA_GAIN_IDX_t TargetTabIdx);

/* Analog In */
S16 DSP_GC_GetAnalogInLevel(AU_AFE_IN_GAIN_COMPONENT_t AfeComponent);
VOID DSP_GC_SetAnalogInLevel(AU_AFE_IN_GAIN_COMPONENT_t AfeComponent, S16 Gain);
VOID DSP_GC_SetAnalogInLevelByGainTable(AUDIO_GAIN_COMPONENT_t Component);
S16 DSP_GC_GetAnalogInLevelByPercentageTable(DSP_AU_CTRL_CH_t Channel);
VOID DSP_GC_LoadAnalogInLevelToAfe(AU_AFE_IN_GAIN_COMPONENT_t AfeComponent);
VOID DSP_GC_LoadAnalogInLevelToAfeConcurrently(VOID);

/* Overall */
VOID DSP_GC_SetDefaultLevelByGainTable(AUDIO_GAIN_COMPONENT_t Component);
VOID DSP_GC_LoadDefaultGainToAfe(VOID);
VOID DSP_GC_SetAfeGainLevelByPercentage(S16 PercentageGain);
VOID DSP_GC_UpdateAfeGains(S16 PercentageGain);


/**
 * DSP_GC_Init
 *
 * Initialization of Gain paramters
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_Init(VOID)
{
    U16 idx;

    AUDIO_ASSERT(ANA_OUT_GAIN_MAX_NO == TOTAL_ANALOG_OUT_GAIN_STEP);

    for (idx = 0 ; idx < AUDIO_GAIN_MAX_COMPONENT ; idx++) {
        gAudioCtrl.Gc.DigitalOut.Reg[idx]                               = 100;
    }

    gAudioCtrl.Gc.DigitalOut.Reg[AUDIO_GAIN_VP]                         = 70;
    gAudioCtrl.Gc.DigitalOut.Reg[AUDIO_GAIN_RT]                         = 70;

    gAudioCtrl.Gc.AnalogOut                                             = 100;

    gAudioCtrl.Gc.GainTable.DigitalIn.Field.Gain                        = 0;

    gAudioCtrl.Gc.GainTable.AnalogIn.Field.LineGain_L                   = 0;            /* 0x5E */
    gAudioCtrl.Gc.GainTable.AnalogIn.Field.LineGain_R                   = 0;            /* 0x5E */
    gAudioCtrl.Gc.GainTable.AnalogIn.Field.MicGain_L                    = 0;            /* 0x5E */
    gAudioCtrl.Gc.GainTable.AnalogIn.Field.MicGain_R                    = 0;            /* 0x5E */
    gAudioCtrl.Gc.GainTable.AnalogIn.Field.AncMicGain_L                 = 0x01;         /* 0x65 */
    gAudioCtrl.Gc.GainTable.AnalogIn.Field.AncMicGain_R                 = 0x01;         /* 0x65 */
    gAudioCtrl.Gc.GainTable.AnalogIn.Field.VadMicGain                   = 0x10;         /* 0x67 */

    gAudioCtrl.Gc.StaticControl.TablePtr                                = pvPortMalloc(sizeof(DSP_STATIC_PERCENTAGE_GAIN_TABLE_t));

    gAudioCtrl.Gc.StaticControl.TablePtr->InputPercentCtrl.Enable       = FALSE;
    gAudioCtrl.Gc.StaticControl.TablePtr->InputPercentCtrl.Percentage   = 50;

    DSP_GC_SetDefaultLevelByGainTable(AUDIO_GAIN_A2DP);
    DSP_GC_LoadDefaultGainToAfe();
}



/**
 * DSP_GC_LoadAnalogOutGainTableByAudioComponent
 *
 * Load Gain Table by Audio Component
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_LoadAnalogOutGainTableByAudioComponent(AUDIO_GAIN_COMPONENT_t Component)
{
    /*
        S16 KeyId;

        switch (Component)
        {
            case AUDIO_GAIN_A2DP:
                KeyId = NVKEY_DSP_PARA_ANALOG_GAINTABLE_A2DP;
                break;

            case AUDIO_GAIN_LINE:
                KeyId = NVKEY_DSP_PARA_ANALOG_GAINTABLE_LINE;
                break;

            case AUDIO_GAIN_SCO:
                KeyId = NVKEY_DSP_PARA_ANALOG_GAINTABLE_SCO;
                break;

            case AUDIO_GAIN_SCO_NB:
                KeyId = NVKEY_DSP_PARA_ANALOG_GAINTABLE_SCO_NB;
                break;

            case AUDIO_GAIN_VC:
                KeyId = NVKEY_DSP_PARA_ANALOG_GAINTABLE_VC;
                break;

            case AUDIO_GAIN_VP:
                KeyId = NVKEY_DSP_PARA_ANALOG_GAINTABLE_VP;
                break;

            case AUDIO_GAIN_RT:
                KeyId = NVKEY_DSP_PARA_ANALOG_GAINTABLE_RT;
                break;

            case AUDIO_GAIN_AT:
                KeyId = NVKEY_DSP_PARA_ANALOG_GAINTABLE_AT;
                break;

            default:
                return;
        }

        NVKEY_ReadFullKey(KeyId,
                          &gAudioCtrl.Gc.StaticControl.TablePtr->AOutPercentage,
                          sizeof(DSP_ANALOG_OUT_GAIN_TABLE_CTRL_t));
                          */
    UNUSED(Component);
}



/**
 * DSP_GC_LoadAnalogInGainTableByAudioComponent
 *
 * Load Gain Table by Audio Component
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_LoadAnalogInGainTableByAudioComponent(AUDIO_GAIN_COMPONENT_t Component)
{
    /*
        S16 KeyId;

        switch (Component)
        {
            case AUDIO_GAIN_A2DP:
                KeyId = NVKEY_DSP_PARA_AIN_GAINTABLE_A2DP;
                break;

            case AUDIO_GAIN_LINE:
                KeyId = NVKEY_DSP_PARA_AIN_GAINTABLE_LINE;
                break;

            case AUDIO_GAIN_SCO:
                KeyId = NVKEY_DSP_PARA_AIN_GAINTABLE_SCO;
                break;

            case AUDIO_GAIN_SCO_NB:
                KeyId = NVKEY_DSP_PARA_AIN_GAINTABLE_SCO_NB;
                break;

            case AUDIO_GAIN_VC:
                KeyId = NVKEY_DSP_PARA_AIN_GAINTABLE_VC;
                break;

            case AUDIO_GAIN_VP:
                KeyId = NVKEY_DSP_PARA_AIN_GAINTABLE_VP;
                break;

            case AUDIO_GAIN_RT:
                KeyId = NVKEY_DSP_PARA_AIN_GAINTABLE_RT;
                break;

            case AUDIO_GAIN_AT:
                KeyId = NVKEY_DSP_PARA_AIN_GAINTABLE_AT;
                break;

            default:
                return;
        }

        NVKEY_ReadFullKey(KeyId,
                          &gAudioCtrl.Gc.GainTable.AnalogIn,
                          sizeof(DSP_GAIN_ANA_IN_CTRL_t));
                          */
    UNUSED(Component);
}


/**
 * DSP_GC_LoadAInPGTableByAudioComponent
 *
 * Load Analog Input Percentage Gain Table by Audio Component
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_LoadAInPGTableByAudioComponent(AUDIO_GAIN_COMPONENT_t Component)
{
    /*
        S16 KeyIdL, KeyIdR;

        switch (Component)
        {
            case AUDIO_GAIN_SCO:
                KeyIdL = NVKEY_DSP_PARA_AIN_GP_TABLE_SCO_L;
                KeyIdR = NVKEY_DSP_PARA_AIN_GP_TABLE_SCO_R;
                break;

            case AUDIO_GAIN_SCO_NB:
                KeyIdL = NVKEY_DSP_PARA_AIN_GP_TABLE_SCO_NB_L;
                KeyIdR = NVKEY_DSP_PARA_AIN_GP_TABLE_SCO_NB_R;
                break;

            case AUDIO_GAIN_AT:
                KeyIdL = NVKEY_DSP_PARA_AIN_GP_TABLE_AT_L;
                KeyIdR = NVKEY_DSP_PARA_AIN_GP_TABLE_AT_R;
                break;

            default:
                AUDIO_ASSERT(FALSE); // should not enter
        }

        NVKEY_ReadFullKey(KeyIdL,
                          &gAudioCtrl.Gc.StaticControl.TablePtr->AInPercentageL,
                          sizeof(DSP_ANALOG_IN_GAIN_TABLE_CTRL_t));

        NVKEY_ReadFullKey(KeyIdR,
                          &gAudioCtrl.Gc.StaticControl.TablePtr->AInPercentageR,
                          sizeof(DSP_ANALOG_IN_GAIN_TABLE_CTRL_t));
                          */
    UNUSED(Component);
}



/**
 * DSP_GC_LoadDigitalInGainTableByAudioComponent
 *
 * Load Gain Table by Audio Component
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_LoadDigitalInGainTableByAudioComponent(AUDIO_GAIN_COMPONENT_t Component)
{
    /*
        S16 KeyId;

        switch (Component)
        {
            case AUDIO_GAIN_A2DP:
                KeyId = NVKEY_DSP_PARA_DIN_GAINTABLE_A2DP;
                break;

            case AUDIO_GAIN_LINE:
                KeyId = NVKEY_DSP_PARA_DIN_GAINTABLE_LINE;
                break;

            case AUDIO_GAIN_SCO:
            case AUDIO_GAIN_SCO_NB:
                KeyId = NVKEY_DSP_PARA_DIN_GAINTABLE_SCO;
                break;

            case AUDIO_GAIN_VC:
                KeyId = NVKEY_DSP_PARA_DIN_GAINTABLE_VC;
                break;

            case AUDIO_GAIN_VP:
                KeyId = NVKEY_DSP_PARA_DIN_GAINTABLE_VP;
                break;

            case AUDIO_GAIN_RT:
                KeyId = NVKEY_DSP_PARA_DIN_GAINTABLE_RT;
                break;

            case AUDIO_GAIN_AT: //Falls through
                KeyId = NVKEY_DSP_PARA_DIN_GAINTABLE_AT;
                break;

            default:
                return;
        }

        NVKEY_ReadFullKey(KeyId,
                          &gAudioCtrl.Gc.GainTable.DigitalIn.Field.Gain,
                          sizeof(S16));
                          */
    UNUSED(Component);
}


/**
 * DSP_GC_LoadDInPGTableByAudioComponent
 *
 * Load Analog Input Percentage Gain Table by Audio Component
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_LoadDInPGTableByAudioComponent(AUDIO_GAIN_COMPONENT_t Component)
{
    /*
        S16 KeyId;

        switch (Component)
        {
            case AUDIO_GAIN_SCO:
                KeyId = NVKEY_DSP_PARA_DIN_GP_TABLE_SCO;
                break;

            case AUDIO_GAIN_SCO_NB:
                KeyId = NVKEY_DSP_PARA_DIN_GP_TABLE_SCO_NB;
                break;

            case AUDIO_GAIN_AT:
                KeyId = NVKEY_DSP_PARA_DIN_GP_TABLE_AT;
                break;

            default:
                AUDIO_ASSERT(FALSE); // should not enter
        }

        NVKEY_ReadFullKey(KeyId,
                          &gAudioCtrl.Gc.StaticControl.TablePtr->DInPercentage,
                          sizeof(DSP_DIGITAL_IN_GAIN_TABLE_CTRL_t));
                          */
    UNUSED(Component);
}

/**
 * DSP_GC_ConvertPercentageToIdx
 *
 * Convert Gain Percentage To Gain Index
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
S16 DSP_GC_ConvertPercentageToIdx(S16 Percentage, S16 TotalIndex)
{
    S16 Index;

    Index = (TotalIndex * Percentage) / 100;

    if (Index == TotalIndex) {
        Index = (TotalIndex - 1);
    }

    return Index;
}


/**
 * DSP_GC_GetDigitalGainIndexFromComponent
 *
 * Get Gain Index From Gain Component
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
S16 DSP_GC_GetDigitalGainIndexFromComponent(AUDIO_GAIN_COMPONENT_t Component, S16 TotalIndex)
{
    return (DSP_GC_ConvertPercentageToIdx(DSP_GC_GetDigitalOutLevel(Component), TotalIndex));
}


/**
 * DSP_GC_GetDigitalOutLevel
 *
 * Get digital output gain
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
S16 DSP_GC_GetDigitalOutLevel(AUDIO_GAIN_COMPONENT_t Component)
{
    return gAudioCtrl.Gc.DigitalOut.Reg[Component];
}


/**
 * DSP_GC_SetDigitalOutLevel
 *
 * Set digital output gain
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_SetDigitalOutLevel(AUDIO_GAIN_COMPONENT_t Component, S16 Gain)
{
    AUDIO_ASSERT(Gain <= 100);

    switch (Component) {
        case AUDIO_GAIN_A2DP:   // Falls through
        case AUDIO_GAIN_VP:     // Falls through
        case AUDIO_GAIN_LINE:   // Falls through
        case AUDIO_GAIN_SCO:    // Falls through
        case AUDIO_GAIN_VC:     // Falls through
        case AUDIO_GAIN_RT:     // Falls through
        case AUDIO_GAIN_AT:
            gAudioCtrl.Gc.DigitalOut.Reg[Component] = Gain;
        /*
        logPrint(LOG_DSP,
                 PRINT_LEVEL_INFO,
                 DSP_INFO_DigitalOutputGainString,
                 2,
                 (U32)Component,
                 (U32)Gain);
        if (AUDIO_GAIN_AT == Component)
        {
            MDSP_AT_UpdateGain();
        }
        */
        default:
            break;
    }
}


/**
 * DSP_GC_GetDigitalOutGain_AT
 *
 * API to get digital out value in Q11
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 */
S16 DSP_GC_GetDigitalOutGain_AT(VOID)
{
    /*
    DSP_OUT_GAIN_TABLE_CTRL_t* pAddr;

    pAddr = (DSP_OUT_GAIN_TABLE_CTRL_t*)NVKEY_GetPayloadFlashAddress(NVKEY_DSP_PARA_DIGITAL_GAINTABLE_AT);

    S16 TotalGainIndex = pAddr->TotalIndex;
    S16 GainIdx = DSP_GC_GetDigitalGainIndexFromComponent(AUDIO_GAIN_AT, TotalGainIndex);

    return DSP_GC_ConvertQ11Form(pAddr->OutGainIndex[GainIdx]);
    */
    return 0;
}


/**
 * DSP_GC_GetDigitalInLevel
 *
 * Set digital input gain
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
S16 DSP_GC_GetDigitalInLevel(VOID)
{
    return gAudioCtrl.Gc.GainTable.DigitalIn.Field.Gain;
}


/**
 * DSP_GC_SetDigitalInLevel
 *
 * Set digital input gain
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_SetDigitalInLevel(S16 PercentageGain)
{
    if ((PercentageGain <= 100) && (PercentageGain >= 0)) {
        gAudioCtrl.Gc.GainTable.DigitalIn.Field.Gain = PercentageGain;
    } else {
        // warning
    }
}


/**
 * DSP_GC_GetDigitalInLevelByPercentageTable
 *
 * Get digital input gain from Percentage Table
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
S16 DSP_GC_GetDigitalInLevelByPercentageTable(VOID)
{
    S32 Gain, GainIdx;
    S16 GainPercentage;

    GainPercentage = gAudioCtrl.Gc.StaticControl.TablePtr->InputPercentCtrl.Percentage;

    GainIdx
        = DSP_GC_ConvertPercentageToIdx(GainPercentage,
                                        gAudioCtrl.Gc.StaticControl.TablePtr->DInPercentage.TotalIndex);

    Gain = gAudioCtrl.Gc.StaticControl.TablePtr->DInPercentage.GainIndex[GainIdx];

    return Gain;
}


/**
 * DSP_GC_SetDigitalInLevelByGainTable
 *
 * Set digital input gain table
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_SetDigitalInLevelByGainTable(AUDIO_GAIN_COMPONENT_t Component)
{
    if (gAudioCtrl.Gc.StaticControl.TablePtr->InputPercentCtrl.Enable) {
        DSP_GC_LoadDInPGTableByAudioComponent(Component);
        gAudioCtrl.Gc.GainTable.DigitalIn.Field.Gain
            = DSP_GC_GetDigitalInLevelByPercentageTable();
    } else {
        DSP_GC_LoadDigitalInGainTableByAudioComponent(Component);
    }
}


/**
 * DSP_GC_LoadDigitalInLevelToDfe
 *
 * Set digital input gain to dfe
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_LoadDigitalInLevelToDfe(VOID)
{
    S16 Gain;

    Gain = DSP_GC_GetDigitalInLevel();
    /*
        logPrint(LOG_DSP,
                 PRINT_LEVEL_INFO,
                 DSP_INFO_DigitalInputGainString,
                 1,
                 (U32)Gain);

        if ((Gain < 8) && (Gain >= 0))
        {
            AUDIO_CODEC.DWN_FIL_SET0.field.DEC1_FIL_DIG_GAIN = Gain;
        }
        else
        {
            // warning
        }
        */
}


/**
 * DSP_GC_GetAnalogOutLevel
 *
 * Set analog output gain
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
S16 DSP_GC_GetAnalogOutLevel(VOID)
{
    return gAudioCtrl.Gc.AnalogOut;
}


/**
 * DSP_GC_SetAnalogIOutLevel
 *
 * Set analog output gain
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_SetAnalogOutLevel(S16 PercentageGain)
{
    if ((PercentageGain <= 100) && (PercentageGain >= 0)) {
        gAudioCtrl.Gc.AnalogOut = PercentageGain;
    } else {
        // warning
    }
}


/**
 * DSP_GC_SetAnalogOutScaleByGainTable
 *
 * Set analog output gain table
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_SetAnalogOutScaleByGainTable(AUDIO_GAIN_COMPONENT_t Component)
{
    if (Component < AUDIO_GAIN_MAX_COMPONENT) {
        DSP_GC_LoadAnalogOutGainTableByAudioComponent(Component);
    }
}


/**
 * DSP_GC_LoadAnalogOutLevelToAfe
 *
 * Set analog output gain to afe
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_LoadAnalogOutLevelToAfe(VOID)
{
    S32 Gain, GainIdx;
    S16 GainPercentage;

    GainPercentage = DSP_GC_GetAnalogOutLevel();

    GainIdx
        = DSP_GC_ConvertPercentageToIdx(GainPercentage,
                                        gAudioCtrl.Gc.StaticControl.TablePtr->AOutPercentage.TotalIndex);

    Gain = gAudioCtrl.Gc.StaticControl.TablePtr->AOutPercentage.GainIndex[GainIdx];
    /*
        logPrint(LOG_DSP,
                 PRINT_LEVEL_INFO,
                 DSP_INFO_AnalogOutputGainString,
                 1,
                 (U32)Gain);
    */
    DSP_DRV_AFE_SetOutputGain(AU_AFE_OUT_COMPONENT_EAR_GAIN, Gain);
}


/**
 * DSP_GC_ResetAnalogOutGain
 *
 * Reset analog output to -6 dB
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_ResetAnalogOutGain(VOID)
{
    DSP_DRV_AFE_SetOutputGain(AU_AFE_OUT_COMPONENT_EAR_GAIN, DSP_ANA_GAIN_TABLE[ANA_OUT_GAIN_NEG_6_DB]);
}


/**
 * DSP_GC_SetAnalogOutOnSequence
 *
 * Set analog output audio on sequence
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_SetAnalogOutOnSequence(VOID)
{
    DSP_ANA_GAIN_IDX_t TargetTabIdx = DSP_GC_GetCurrTabIdx();

    DSP_GC_SetToTargetGainByStepFunction(ANA_OUT_GAIN_NEG_6_DB, TargetTabIdx);
}


/**
 * DSP_GC_SetAnalogOutOffSequence
 *
 * Set analog output audio off sequence
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_SetAnalogOutOffSequence(VOID)
{
    DSP_ANA_GAIN_IDX_t CurrTabIdx = DSP_GC_GetCurrTabIdx();

    DSP_GC_SetToTargetGainByStepFunction(CurrTabIdx, ANA_OUT_GAIN_NEG_6_DB);
}


/**
 * DSP_GC_GetCurrTabIdx
 *
 * Get current table index by current gain level
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
DSP_ANA_GAIN_IDX_t DSP_GC_GetCurrTabIdx(VOID)
{
    S32 Gain, GainIdx;
    S16 GainPercentage;
    DSP_ANA_GAIN_IDX_t CurrTabIdx;

    GainPercentage = DSP_GC_GetAnalogOutLevel();

    GainIdx
        = DSP_GC_ConvertPercentageToIdx(GainPercentage,
                                        gAudioCtrl.Gc.StaticControl.TablePtr->AOutPercentage.TotalIndex);

    Gain = gAudioCtrl.Gc.StaticControl.TablePtr->AOutPercentage.GainIndex[GainIdx];

    CurrTabIdx = DSP_GC_SearchAnalogGainTableIndex(Gain);

    return CurrTabIdx;
}


/**
 * DSP_GC_SearchAnalogGainTableIndex
 *
 * Search for corresponding gain index in table from input Gain
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
DSP_ANA_GAIN_IDX_t DSP_GC_SearchAnalogGainTableIndex(S32 Gain)
{
    DSP_ANA_GAIN_IDX_t Idx = 0;

    while (Idx < TOTAL_ANALOG_OUT_GAIN_STEP) {
        if (Gain > DSP_ANA_GAIN_TABLE[Idx]) {
            Idx++;
        } else {
            break;
        }
    }

    return Idx;
}


/**
 * DSP_GC_SetToTargetGainByStepFunction
 *
 * Set analog output gain from InitialTabIdx to TargetTabIdx
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_SetToTargetGainByStepFunction(DSP_ANA_GAIN_IDX_t InitialTabIdx, DSP_ANA_GAIN_IDX_t TargetTabIdx)
{
    DSP_ANA_GAIN_IDX_t TabIdx = InitialTabIdx;

    if (TargetTabIdx > TabIdx) {
        for (; TabIdx <= TargetTabIdx ; TabIdx++) {
            DSP_DRV_AFE_SetOutputGain(AU_AFE_OUT_COMPONENT_EAR_GAIN, DSP_ANA_GAIN_TABLE[TabIdx]);
        }

    } else if (TargetTabIdx < TabIdx) {
        for (; TabIdx >= TargetTabIdx ; TabIdx--) {
            DSP_DRV_AFE_SetOutputGain(AU_AFE_OUT_COMPONENT_EAR_GAIN, DSP_ANA_GAIN_TABLE[TabIdx]);
        }
    } else {
        DSP_DRV_AFE_SetOutputGain(AU_AFE_OUT_COMPONENT_EAR_GAIN, DSP_ANA_GAIN_TABLE[TabIdx]);
    }

}


/**
 * DSP_GC_GetAnalogInLevel
 *
 * Get analog input gain
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
S16 DSP_GC_GetAnalogInLevel(AU_AFE_IN_GAIN_COMPONENT_t AfeComponent)
{
    if (AfeComponent < AU_AFE_IN_COMPONENT_NO) {
        return gAudioCtrl.Gc.GainTable.AnalogIn.Reg[AfeComponent];
    }

    return 0;
}


/**
 * DSP_GC_SetAnalogInLevel
 *
 * Set analog input gain
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_SetAnalogInLevel(AU_AFE_IN_GAIN_COMPONENT_t AfeComponent, S16 Gain)
{
    if (AfeComponent < AU_AFE_IN_COMPONENT_NO) {
        gAudioCtrl.Gc.GainTable.AnalogIn.Reg[AfeComponent] = Gain;
    }
}


/**
 * DSP_GC_GetAnalogInLevelByPercentageTable
 *
 * Get analog input gain from Percentage Table
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
S16 DSP_GC_GetAnalogInLevelByPercentageTable(DSP_AU_CTRL_CH_t Channel)
{
    S32 Gain, GainIdx;
    S16 GainPercentage;

    GainPercentage = gAudioCtrl.Gc.StaticControl.TablePtr->InputPercentCtrl.Percentage;

    if (Channel == AUDIO_CTRL_CHANNEL_L) { //L
        GainIdx
            = DSP_GC_ConvertPercentageToIdx(GainPercentage,
                                            gAudioCtrl.Gc.StaticControl.TablePtr->AInPercentageL.TotalIndex);

        Gain = gAudioCtrl.Gc.StaticControl.TablePtr->AInPercentageL.GainIndex[GainIdx];
    } else { //R
        GainIdx
            = DSP_GC_ConvertPercentageToIdx(GainPercentage,
                                            gAudioCtrl.Gc.StaticControl.TablePtr->AInPercentageR.TotalIndex);

        Gain = gAudioCtrl.Gc.StaticControl.TablePtr->AInPercentageR.GainIndex[GainIdx];
    }

    return Gain;
}


/**
 * DSP_GC_SetAnalogInLevelByGainTable
 *
 * Set analog input gain table
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_SetAnalogInLevelByGainTable(AUDIO_GAIN_COMPONENT_t Component)
{
    if (Component < AUDIO_GAIN_MAX_COMPONENT) {
        DSP_GC_LoadAnalogInGainTableByAudioComponent(Component);

        if (gAudioCtrl.Gc.StaticControl.TablePtr->InputPercentCtrl.Enable) {
            DSP_GC_LoadAInPGTableByAudioComponent(Component);

            gAudioCtrl.Gc.GainTable.AnalogIn.Field.MicGain_L
                = DSP_GC_GetAnalogInLevelByPercentageTable(AUDIO_CTRL_CHANNEL_L);

            gAudioCtrl.Gc.GainTable.AnalogIn.Field.MicGain_R
                = DSP_GC_GetAnalogInLevelByPercentageTable(AUDIO_CTRL_CHANNEL_R);
        }
    }
}


/**
 * DSP_GC_LoadAnalogInLevelToAfe
 *
 * Set analog input gain to afe
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_LoadAnalogInLevelToAfe(AU_AFE_IN_GAIN_COMPONENT_t AfeComponent)
{
    S16 Gain;

    if (AfeComponent < AU_AFE_IN_COMPONENT_NO) {
        Gain = DSP_GC_GetAnalogInLevel(AfeComponent);

        DSP_DRV_AFE_SetInputGain(AfeComponent, Gain);
    }
}


/**
 * DSP_GC_LoadAnalogInLevelToAfeConcurrently
 *
 * Set analog input gain from current settings concurrently
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_LoadAnalogInLevelToAfeConcurrently(VOID)
{
    U16 AfeComponent;

    for (AfeComponent = 0 ; AfeComponent < AU_AFE_IN_COMPONENT_NO ; AfeComponent++) {
        if ((AfeComponent == AU_AFE_IN_COMPONENT_ANC_MIC_L) || (AfeComponent == AU_AFE_IN_COMPONENT_ANC_MIC_R)) {
            continue;
        }

        DSP_DRV_AFE_SetInputGain(AfeComponent, gAudioCtrl.Gc.GainTable.AnalogIn.Reg[AfeComponent]);
    }
}


/**
 * DSP_GC_SetDefaultLevelByGainTable
 *
 * Load Default by Audio Component
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_SetDefaultLevelByGainTable(AUDIO_GAIN_COMPONENT_t Component)
{
    if ((AUDIO_GAIN_SCO_NB == Component)
        || (AUDIO_GAIN_SCO == Component)) {
#if 1 // Do not enable until verified
        gAudioCtrl.Gc.StaticControl.TablePtr->InputPercentCtrl.Enable = TRUE;
#else
        gAudioCtrl.Gc.StaticControl.TablePtr->InputPercentCtrl.Enable = FALSE;
#endif
    } else {
        gAudioCtrl.Gc.StaticControl.TablePtr->InputPercentCtrl.Enable = FALSE;
    }

    DSP_GC_SetAnalogOutScaleByGainTable(Component);
    DSP_GC_SetAnalogInLevelByGainTable(Component);
    DSP_GC_SetDigitalInLevelByGainTable(Component);
}


/**
 * DSP_GC_LoadDefaultGainToAfe
 *
 * Load Default Gain to Audio front end
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_LoadDefaultGainToAfe(VOID)
{
    DSP_GC_LoadDigitalInLevelToDfe();
    DSP_GC_LoadAnalogOutLevelToAfe();
    DSP_GC_LoadAnalogInLevelToAfeConcurrently();
}


/**
 * DSP_GC_SetAfeGainLevelByPercentage
 *
 * Update audio front end gain level by percentage
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_SetAfeGainLevelByPercentage(S16 PercentageGain)
{
    if ((PercentageGain <= 100) && (PercentageGain >= 0)
        && (gAudioCtrl.Gc.StaticControl.TablePtr->InputPercentCtrl.Enable)) {
        gAudioCtrl.Gc.StaticControl.TablePtr->InputPercentCtrl.Percentage = PercentageGain;

        gAudioCtrl.Gc.GainTable.DigitalIn.Field.Gain
            = DSP_GC_GetDigitalInLevelByPercentageTable();

        gAudioCtrl.Gc.GainTable.AnalogIn.Field.MicGain_L
            = DSP_GC_GetAnalogInLevelByPercentageTable(AUDIO_CTRL_CHANNEL_L);

        gAudioCtrl.Gc.GainTable.AnalogIn.Field.MicGain_R
            = DSP_GC_GetAnalogInLevelByPercentageTable(AUDIO_CTRL_CHANNEL_R);
    } else {
        // warning
    }
}


/**
 * DSP_GC_UpdateAfeGains
 *
 * Update all gain levels by percentage
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_GC_UpdateAfeGains(S16 PercentageGain)
{
    /* Update Analog In & Digital In Level */
    DSP_GC_SetAfeGainLevelByPercentage(PercentageGain);

    /* Update Analog Out Level */
    DSP_GC_SetAnalogOutLevel(PercentageGain);

    /* Update Set Afe Level to Hardware */
    DSP_GC_LoadDefaultGainToAfe();
}


/**
 * DSP_GC_MuteAudioSource
 *
 * API to mute audio source
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 * @Ctrl: TRUE for mute, FALSE for unmute
 *
 */
VOID DSP_GC_MuteAudioSource(BOOL Ctrl)
{
    Source_Audio_Configuration(0, AUDIO_SOURCE_MUTE_ENABLE, Ctrl);
}


/**
 * DSP_GC_MuteAudioSink
 *
 * API to mute audio sink
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 * @Ctrl: TRUE for mute, FALSE for unmute
 *
 */
VOID DSP_GC_MuteAudioSink(BOOL Ctrl)
{
    Sink_Audio_Configuration(Sink_blks[SINK_TYPE_AUDIO], AUDIO_SINK_MUTE_ENABLE, Ctrl);
}


/*Set volume from CM4 ccni msg*/

void DSP_GC_SetOutputVolume(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    uint16_t output_index = (uint16_t)(msg.ccni_message[0] & 0xFFFF);
    uint16_t d_gain_index = (uint16_t)msg.ccni_message[1];
    uint16_t a_gain_index = (uint32_t)(msg.ccni_message[1] >> 16);
    hal_audio_set_stream_out_volume(output_index, d_gain_index, a_gain_index);
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
    if (output_index == 0)
    {
        /* HWGAIN3 for I2S_MST1 */
        hal_audio_set_stream_out_volume(2, d_gain_index, a_gain_index);
        DSP_MW_LOG_I("[MULTI] DSP_GC_SetOutputVolume index 0x%x: a = [%d] (0x%x), d = [%d] (0x%x)", 5, 2, (int16_t)a_gain_index, a_gain_index,
            (int16_t)d_gain_index, d_gain_index);
    }
#endif /* AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE */
    DSP_MW_LOG_I("DSP_GC_SetOutputVolume index 0x%x: a = [%d] (0x%x), d = [%d] (0x%x)", 5, output_index, (int16_t)a_gain_index, a_gain_index,
        (int16_t)d_gain_index, d_gain_index);
    UNUSED(ack);
}

void DSP_GC_SetInputVolume(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    uint16_t input_index = (uint16_t)(msg.ccni_message[0] & 0xFFFF);
    int16_t gain_index0 = (int16_t)msg.ccni_message[1];
    uint16_t gain_index1 = (uint32_t)(msg.ccni_message[1] >> 16);

    if (input_index == HAL_AUDIO_INPUT_GAIN_SELECTION_D0_A0) {
        hal_audio_set_stream_in_volume(gain_index0, gain_index1);
    } else {
        hal_audio_set_stream_in_volume_for_multiple_microphone(gain_index0, gain_index1, input_index);
    }
    DSP_MW_LOG_I("DSP_GC_SetInputVolume index %2d: gain_index0(d gain) = [%d], gain_index1(a gain) = [%d]", 3,
                input_index, (int16_t)gain_index0, (int16_t)gain_index1);
    UNUSED(ack);
}

void DSP_GC_MuteOutput(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    bool enable = (bool)(uint16_t)msg.ccni_message[1];
    hal_audio_hw_stream_out_index_t hw_gain_index = (hal_audio_hw_stream_out_index_t)(uint32_t)(msg.ccni_message[1] >> 16);
    DSP_MW_LOG_I("mute enable = %d, hw_gain_index (1 << index) = 0x%x", 2, enable, hw_gain_index);
#if defined(HAL_AUDIO_SUPPORT_MULTIPLE_STREAM_OUT)
    hal_audio_mute_stream_out(enable, hw_gain_index);
#else
    hal_audio_mute_stream_out(enable);
#endif
    //AudioAfeConfiguration(AUDIO_SINK_MUTE_ENABLE, enable);
    UNUSED(ack);
}

void DSP_GC_SetGainParameters(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    int16_t  gain_compensation[2] = {
        (int16_t)((msg.ccni_message[1]) & 0xFFFF), (int16_t)((msg.ccni_message[1] >> 16) & 0xFFFF)
    };
    uint16_t gain_per_step[2] = {
        (uint16_t)((msg.ccni_message[0]) & 0xFF), (uint16_t)((msg.ccni_message[0] >> 8) & 0xFF)
    };
    bool     is_hw_gain12 = false;
    uint32_t i            = 0;
        hal_audio_volume_digital_gain_setting_parameter_t   digital_gain_setting;
    hal_audio_memory_selection_t memory_select[2] = {
        HAL_AUDIO_MEMORY_DUMMY, HAL_AUDIO_MEMORY_DUMMY
    };
    if ((msg.ccni_message[1] >> 16) != 0x5AA5) {  // 0x5AA5 means DL4&DL3
        is_hw_gain12 = true;
        memory_select[0] = HAL_AUDIO_MEMORY_DL_DL1 | HAL_AUDIO_MEMORY_DL_SRC1;
        memory_select[1] = HAL_AUDIO_MEMORY_DL_DL2 | HAL_AUDIO_MEMORY_DL_SRC2;
    } else {
        memory_select[0] = HAL_AUDIO_MEMORY_DL_DL3;
    }
    for (i = 0; i < 2; i ++) {
        if (memory_select[i] == HAL_AUDIO_MEMORY_DUMMY) {
            continue; // ignore hw gain 4
        }
        digital_gain_setting.index_compensation = (uint32_t)((int32_t)((int16_t)gain_compensation[i]));
        digital_gain_setting.memory_select = memory_select[i];
        digital_gain_setting.sample_per_step = (uint32_t)gain_per_step[i];
        hal_audio_control_set_value((hal_audio_set_value_parameter_t *)&digital_gain_setting, HAL_AUDIO_SET_VOLUME_HW_DIGITAL_SETTING);
        DSP_MW_LOG_I("DSP_GC_Set HW gain param gain%d_compensation %d step %d", 3, is_hw_gain12 ? 3 : i, gain_compensation[i], gain_per_step[i]);
    }
#ifdef AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
    digital_gain_setting.index_compensation = (uint32_t)((int32_t)((int16_t)gain_compensation[0]));
    digital_gain_setting.memory_select = HAL_AUDIO_MEMORY_DL_DL3;
    digital_gain_setting.sample_per_step = (uint32_t)gain_per_step[0];
    hal_audio_control_set_value((hal_audio_set_value_parameter_t *)&digital_gain_setting, HAL_AUDIO_SET_VOLUME_HW_DIGITAL_SETTING);
    DSP_MW_LOG_I("[MULTI] gain3_compensation = gain1_compensation = 0x%x, gain3_per_step = gain1_per_step = 0x%x", 2, gain_compensation[0], gain_per_step[0]);
#endif
    UNUSED(ack);
}

#ifdef AIR_SOFTWARE_GAIN_ENABLE
extern bool g_call_mute_flag;
#endif

void DSP_GC_MuteInput(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    hal_audio_stream_in_scenario_t type;
    uint16_t input_index = (uint16_t)(msg.ccni_message[0] & 0xFFFF);
    bool enable = (bool)(msg.ccni_message[1] & 0xFFFF);

    UNUSED(ack);

    type = (input_index >> 8) & 0x7F;
    if ((input_index & HAL_AUDIO_STREAM_IN_SCENARIO_MARK) && (type < HAL_AUDIO_STREAM_IN_SCENARIO_MAX)) {
#ifdef AIR_SOFTWARE_GAIN_ENABLE
        if ((type == HAL_AUDIO_STREAM_IN_SCENARIO_HFP) || (type == HAL_AUDIO_STREAM_IN_SCENARIO_BLE_CALL)) {
            DSP_MW_LOG_I("Call_UL_SW_Gain_Mute_control %d", 1, enable);
            g_call_mute_flag = enable;
            return;
        }
#endif
    }

    hal_audio_mute_stream_in(enable);
}

void DSP_GC_SetHWGainWithFadeTime(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    uint16_t output_index = (uint16_t)(msg.ccni_message[0] & 0xFFFF);
    uint16_t d_gain_index = (uint16_t)msg.ccni_message[1];
    uint16_t fade_time = (uint32_t)(msg.ccni_message[1] >> 16);
    hal_audio_volume_digital_gain_fade_time_setting_parameter_t digital_gain_fade_time_setting;
    digital_gain_fade_time_setting.fade_time = fade_time;
    digital_gain_fade_time_setting.gain_index = (uint32_t)((int32_t)((int16_t)d_gain_index));
    DSP_MW_LOG_I("Set HW gain:%d, fade time: %d, memory select:%d", 3, d_gain_index, fade_time, output_index);
    if (output_index < 2) {
        digital_gain_fade_time_setting.memory_select = (hal_audio_memory_selection_t) (output_index + 1);
        hal_audio_control_set_value((hal_audio_set_value_parameter_t *)&digital_gain_fade_time_setting, HAL_AUDIO_SET_VOLUME_HW_DIGITAL_FADE_TIME_SETTING);
    }
    UNUSED(ack);
}

void DSP_GC_POWER_OFF_DAC_IMMEDIATELY(void)
{
    // mute hw gain1/hw gain2/hw gain3
    hal_audio_set_stream_out_volume(HAL_AUDIO_STREAM_OUT1, 0xD120, HAL_AUDIO_INVALID_ANALOG_GAIN_INDEX); // -120dB is mute for HW Gain0
    hal_audio_set_stream_out_volume(HAL_AUDIO_STREAM_OUT2, 0xD120, HAL_AUDIO_INVALID_ANALOG_GAIN_INDEX); // -120dB is mute for HW Gain1
    hal_audio_set_stream_out_volume(HAL_AUDIO_STREAM_OUT3, 0xD120, HAL_AUDIO_INVALID_ANALOG_GAIN_INDEX); // -120dB is mute for HW Gain2
    #if defined(AIR_BTA_IC_PREMIUM_G3) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
    hal_audio_set_stream_out_volume(HAL_AUDIO_STREAM_OUT4, 0xD120, HAL_AUDIO_INVALID_ANALOG_GAIN_INDEX); // -120dB is mute for HW Gain3
    #endif
    hal_gpt_delay_ms(50); // waiting for hw-gain ramp!

    hal_audio_device_parameter_t dev_handle;
    memset(&dev_handle, 0, sizeof(hal_audio_device_parameter_t));
    // get dac type
    hal_audio_control_t dac_device = hal_audio_device_analog_get_output(AFE_ANALOG_DAC);
    if (dac_device == HAL_AUDIO_CONTROL_NONE) {
        DSP_MW_LOG_W("[POWER OFF NOW] ATTENTION: dac is not open. type %d", 1, dac_device);
        return;
    }
    // get current dl output rate
    dev_handle.dac.rate = afe_samplerate_get_dl_samplerate();
    // get current sdm setting
    dev_handle.dac.dl_sdm_setting = hal_audio_dl_get_sdm();
    // get current dac performance
#if defined(AIR_BTA_IC_PREMIUM_G2)
    dev_handle.dac.with_high_performance = hal_get_adc_performance_mode(AFE_ANALOG_DAC);
#else
    dev_handle.dac.performance = hal_get_adc_performance_mode(AFE_ANALOG_DAC);
#endif
    // get current dac mode(classd/classg)
    dev_handle.dac.dac_mode = hal_volume_get_analog_mode(AFE_HW_ANALOG_GAIN_OUTPUT);
    hal_audio_device_set_dac(&dev_handle.dac, dac_device, HAL_AUDIO_CONTROL_OFF);
    DSP_MW_LOG_I("[POWER OFF NOW] dac rate:[%d] dl_sdm_setting:[%d] performance:[%d] dac_mode:[%d] dac_device[0x%x]", 5,
                 dev_handle.dac.rate,
                 dev_handle.dac.dl_sdm_setting,
#if defined(AIR_BTA_IC_PREMIUM_G2)
                 dev_handle.dac.with_high_performance,
#else
                 dev_handle.dac.performance,
#endif
                 dev_handle.dac.dac_mode,
                 dac_device);
}

#if defined(AIR_AUDIO_VOLUME_MONITOR_ENABLE)
void DSP_GC_VOLUME_MONITOR_INIT(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    audio_volume_monitor_node_t *node = (audio_volume_monitor_node_t *)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    volume_estimator_port_t *port = volume_estimator_get_the_port_by_scenario_type(node->type);
    if (!port) {
        AUDIO_ASSERT(0 && "Can't find the port");
    }
    port->node = node;
    DSP_MW_LOG_I("[Volume Monitor] enable: type %d 0x%x len %d data addr 0x%x", 4,
        node->type,
        node,
        node->volume_len,
        node->volume_data
        );
}

void DSP_GC_VOLUME_MONITOR_DEINIT(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    audio_scenario_type_t type = (audio_scenario_type_t)msg.ccni_message[1];
    volume_estimator_port_t *port = volume_estimator_get_the_port_by_scenario_type(type);
    if (!port) {
        DSP_MW_LOG_E("[Volume Monitor] disable: type %d, can't find the port. Please check the usage sequence.", 1, type);
        // AUDIO_ASSERT(0 && "Can't find the port");
        return;
    }
    DSP_MW_LOG_I("[Volume Monitor] disable: type %d", 1, type);
    port->node = NULL;
}
#endif

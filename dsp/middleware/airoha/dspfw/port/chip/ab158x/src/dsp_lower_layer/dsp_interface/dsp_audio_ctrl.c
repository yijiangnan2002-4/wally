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

#include "config.h"
#include "types.h"
#include "dsp_audio_ctrl.h"
#include "dsp_drv_afe.h"
#include "dsp_gain_control.h"
#include "sink.h"
#include "source.h"
#include "dsp_temp.h"
#ifdef MTK_ANC_ENABLE
#include "anc_api.h"
#endif
#include "stream_n9sco.h"

#ifdef AIR_BTA_IC_PREMIUM_G2
#include "hal_resource_assignment.h"
#endif
/**
 *
 *  Definition
 *
 */


/**
 *
 *  Type Definition
 *
 */
typedef void (*AUDIO_CONTROL_ENTRY)(void);

typedef struct AUDIO_CONTROL_STRU_s {
    AUDIO_CONTROL_ENTRY Entry;
} AUDIO_CONTROL_STRU_t;


/**
 *
 *  Buffer & Control
 *
 */
DSP_AUDIO_CTRL_t gAudioCtrl;

VOID DSP_CTRL_Initialization(VOID);
VOID DSP_CTRL_Deinitialization(VOID);
VOID DSP_CTRL_ChangeStatus(AU_DSP_STATUS_LIST_t Component, AU_DSP_STATUS_CH_t Stat);
VOID DSP_CTRL_AudioHandleProcess(AU_DSP_STATUS_LIST_t Component, AU_DSP_STATUS_CH_t Stat);
VOID DSP_CTRL_AudioOffProcedure_DspInput(VOID);
VOID DSP_CTRL_AudioOnProcedure_DspInput(VOID);
VOID DSP_CTRL_AudioOffProcedure_DspOutput(VOID);
VOID DSP_CTRL_AudioOnProcedure_DspOutput(VOID);
VOID DSP_CTRL_AudioOffProcedure(VOID);
VOID DSP_CTRL_AudioOnProcedure(VOID);
VOID DSP_CTRL_DummyOff(VOID);
VOID DSP_CTRL_DummyOn(VOID);
#ifdef AIR_BTA_IC_STEREO_HIGH_G3
VOID DSP_CTRL_AudioAnaInitialize(VOID);
#endif

static CODE AUDIO_CONTROL_ENTRY pAudioOnCtrlEntry[] = {
    DSP_CTRL_AudioOffProcedure_DspInput,
    DSP_CTRL_DummyOn,
    DSP_CTRL_DummyOn,
    DSP_CTRL_AudioOnProcedure_DspOutput,
    DSP_CTRL_DummyOn,
    DSP_CTRL_DummyOn,
    DSP_CTRL_DummyOn,
    DSP_CTRL_AudioOnProcedure,
};


static CODE AUDIO_CONTROL_ENTRY pAudioOffCtrlEntry[] = {
    DSP_CTRL_AudioOffProcedure_DspInput,
    DSP_CTRL_DummyOff,
    DSP_CTRL_DummyOff,
    DSP_CTRL_AudioOffProcedure_DspOutput,
    DSP_CTRL_DummyOff,
    DSP_CTRL_DummyOff,
    DSP_CTRL_DummyOff,
    DSP_CTRL_AudioOffProcedure,
};


/**
 * DSP_CTRL_Initialization
 *
 * This function is used to initialize control status of all audio components
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 */
VOID DSP_CTRL_Initialization(VOID)
{
    DSP_CTRL_Deinitialization();
#ifdef AIR_AUDIO_HARDWARE_ENABLE
    DSP_GC_Init();                  //[ToDo] gain control
    DSP_DRV_AFE_Init();
#endif /* AIR_AUDIO_HARDWARE_ENABLE */
#ifdef AIR_BTA_IC_STEREO_HIGH_G3
    DSP_CTRL_AudioAnaInitialize();
#endif
#ifdef MTK_ANC_ENABLE
    dsp_anc_init();
#endif
}


/**
 * DSP_CTRL_Deinitialization
 *
 * This function is used to de-initialize control status of all audio components
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 */
VOID DSP_CTRL_Deinitialization(VOID)
{
    U32 Idx;
    for (Idx = 0 ; Idx <= AU_STATUS_ALL ; Idx++) {
        if (gAudioCtrl.Status[Idx] == AU_DSP_STATUS_ON) {
            DSP_CTRL_ChangeStatus(Idx, AU_DSP_STATUS_OFF);
        }
        gAudioCtrl.Status[Idx] = AU_DSP_STATUS_OFF;
    }
}


/**
 * DSP_CTRL_ChangeStatus
 *
 * This function is used to change control status of specific component,
 * which should always be accompanied by operation of audio source/sink
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 * @Stat : Status to be set
 *
 */
VOID DSP_CTRL_ChangeStatus(AU_DSP_STATUS_LIST_t Component, AU_DSP_STATUS_CH_t Stat)
{
    U32 Idx = 0;
    U32 ComponentExist = 0;

    UNUSED(ComponentExist);

    if (Stat == AU_DSP_STATUS_ON) {
        DSP_CTRL_AudioHandleProcess(Component, AU_DSP_STATUS_ON);

        if (Component < AU_STATUS_INPUT) {
            if (gAudioCtrl.Status[AU_STATUS_INPUT] == AU_DSP_STATUS_OFF) {
                DSP_CTRL_AudioHandleProcess(AU_STATUS_INPUT, AU_DSP_STATUS_ON);
            }
        } else if (Component < AU_STATUS_OUTPUT) {
            if (gAudioCtrl.Status[AU_STATUS_OUTPUT] == AU_DSP_STATUS_OFF) {
                DSP_CTRL_AudioHandleProcess(AU_STATUS_OUTPUT, AU_DSP_STATUS_ON);
            }
        }

        if (gAudioCtrl.Status[AU_STATUS_ALL] == AU_DSP_STATUS_OFF) {
            DSP_CTRL_AudioHandleProcess(AU_STATUS_ALL, AU_DSP_STATUS_ON);
        }
    } else {
        DSP_CTRL_AudioHandleProcess(Component, AU_DSP_STATUS_OFF);

        if (Component < AU_STATUS_INPUT) {
            for (Idx = 0 ; Idx < AU_STATUS_INPUT ; Idx++) {
                if (gAudioCtrl.Status[Idx] != AU_DSP_STATUS_OFF) {
                    return;
                }
            }
            DSP_CTRL_AudioHandleProcess(AU_STATUS_INPUT, AU_DSP_STATUS_OFF);
        } else if (Component < AU_STATUS_OUTPUT) {
            for (Idx = (AU_STATUS_INPUT + 1) ; Idx < AU_STATUS_OUTPUT ; Idx++) {
                if (gAudioCtrl.Status[Idx] != AU_DSP_STATUS_OFF) {
                    return;
                }
            }
            DSP_CTRL_AudioHandleProcess(AU_STATUS_OUTPUT, AU_DSP_STATUS_OFF);
        }

        for (Idx = 0 ; Idx < AU_STATUS_ALL ; Idx++) {
            if (gAudioCtrl.Status[Idx] != AU_DSP_STATUS_OFF) {
                return;
            }
        }
        DSP_CTRL_AudioHandleProcess(AU_STATUS_ALL, AU_DSP_STATUS_OFF);
    }
}


/**
 * DSP_CTRL_AudioHandleProcess
 *
 * This function is used to control audio compenent and correspoding behavior
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */
VOID DSP_CTRL_AudioHandleProcess(AU_DSP_STATUS_LIST_t Component, AU_DSP_STATUS_CH_t Stat)
{
    gAudioCtrl.Status[Component] = Stat;

    switch (Stat) {
        case AU_DSP_STATUS_ON:
            pAudioOnCtrlEntry[Component]();
            break;

        case AU_DSP_STATUS_OFF:
            pAudioOffCtrlEntry[Component]();
            break;

        default:
            break;
    }
}

VOID DSP_CTRL_AudioOffProcedure_DspInput(VOID)
{

}

VOID DSP_CTRL_AudioOnProcedure_DspInput(VOID)
{

}

VOID DSP_CTRL_AudioOffProcedure_DspOutput(VOID)
{

}

VOID DSP_CTRL_AudioOnProcedure_DspOutput(VOID)
{

}

VOID DSP_CTRL_AudioOffProcedure(VOID)
{

}

VOID DSP_CTRL_AudioOnProcedure(VOID)
{

}

VOID DSP_CTRL_DummyOff(VOID)
{

}

VOID DSP_CTRL_DummyOn(VOID)
{

}

#ifdef AIR_BTA_IC_STEREO_HIGH_G3
VOID DSP_CTRL_AudioAnaInitialize(VOID)
{
    /*To reduce power consumption*/
    HAL_AUDIO_LOG_ERROR("[ANA_init] ana RG initialize \n", 0);
    AFE_WRITE(AUDENC_ANA_CON43, 0x400);
    AFE_WRITE(AUDDEC_ANA_CON9, 0x2);
    // ANA_SET_REG((AUDENC_ANA_CON43), (0x04 << AUDENC_ANA_CON43_RG_AUDUL_ADC23_REV0_POS) | (0x00 << AUDENC_ANA_CON43_RG_AUDUL_PGA23_REV0_POS), AUDENC_ANA_CON43_RG_AUDUL_ADC23_REV0_MASK | AUDENC_ANA_CON43_RG_AUDUL_PGA23_REV0_MASK);
}
#endif

#if (defined AIR_BTA_IC_PREMIUM_G2) || (defined AIR_BTA_IC_STEREO_HIGH_G3)
extern bool hal_audio_device_set_mic_bias(hal_audio_mic_bias_parameter_t *mic_bias);
VOID DSP_CTRL_AudioMicBiasControl(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    mcu2dsp_open_param_p           open_param = NULL;
    hal_audio_mic_bias_parameter_t mic_bias   = {0};
    bool                           control    = msg.ccni_message[0] & 0xFFFF;
    /* remap to non-cacheable address */
    if (msg.ccni_message[1] == 0) {
        AUDIO_ASSERT(0 && "Invalid open parameter pointer from mcu side. $DSP_CTRL_AudioMicBiasControl");
        return;
    }
    open_param = (mcu2dsp_open_param_p)hal_memview_cm4_to_dsp0(msg.ccni_message[1]);
    /* copy parameters from mcu to dsp */
    for (uint8_t i = 0; i < 5; i++) {
        mic_bias.bias_voltage[i] = open_param->stream_in_param.afe.bias_voltage[i];
    }
    mic_bias.bias1_2_with_LDO0 = open_param->stream_in_param.afe.bias1_2_with_LDO0;
    mic_bias.bias_select       = open_param->stream_in_param.afe.bias_select;
    mic_bias.enable            = control;
    hal_audio_device_set_mic_bias(&mic_bias);
    HAL_AUDIO_LOG_INFO("[DSP_CTRL] mic bias control, enable %d, voltage %d, %d, %d, %d, %d, bias1_2_with_LDO0 %d, bias_select %d", 8,
        mic_bias.enable,
        mic_bias.bias_voltage[0],
        mic_bias.bias_voltage[1],
        mic_bias.bias_voltage[2],
        mic_bias.bias_voltage[3],
        mic_bias.bias_voltage[4],
        mic_bias.bias1_2_with_LDO0,
        mic_bias.bias_select
        );
}
#endif
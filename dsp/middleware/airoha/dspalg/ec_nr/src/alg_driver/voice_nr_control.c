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

#include "hal.h"
#include "dsp_audio_msg_define.h"
#include "dsp_share_memory.h"
#include "dsp_audio_msg.h"
#include "voice_nr_driver.h"
#include "voice_nr_portable.h"
#include "voice_nr_interface.h"
#include "assert.h"

extern DSP_PARA_AEC_STRU *g_voice_ecnr_nvkey_aec;

/* Query voice NR lib version */
uint32_t voice_nr_query_version(dsp_alg_lib_type_t type, uint8_t *version)
{
    uint32_t length;

    if (type == DSP_ALG_LIB_TYPE_IGO) {
#if defined(AIR_3RD_PARTY_NR_ENABLE)
        length = voice_ecnr_nr_query_version(version);
#else
        UNUSED(version);
        length = 0;
#endif
    } else {
        length = 0;
    }

    return length;
}

/* Voice NR control: CM4 Set NR feature */
#if defined(AIR_3RD_PARTY_NR_ENABLE)
typedef enum {
    VOICE_NR_PARA_TYPE_NR_LEVEL = 0,
    VOICE_NR_PARA_TYPE_MAX = 0xFFFF,
} voice_nr_param_type_t;

void voice_nr_set_param(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    voice_nr_param_type_t msg16 = (voice_nr_param_type_t)(msg.ccni_message[0] & 0xFFFF);
    uint32_t msg32 = msg.ccni_message[1];

    UNUSED(ack);

    switch (msg16) {
        case VOICE_NR_PARA_TYPE_NR_LEVEL:
            voice_ecnr_nr_set_nr_level(msg32);
            break;
        default:
            return;
    }
}
#else
void voice_nr_set_param(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(msg);
    UNUSED(ack);
}
#endif

/* Voice NR control: airdump */
#ifdef AIR_AIRDUMP_ENABLE

#define AEC_REF_GAIN_SIZE                         (8)
#define AEC_PRELIM_COEF_SIZE                      (9)
#define AEC_DUALMIC_POWER_COEF_SIZE               (1)
#define AEC_INEAR_SIZE                            (6)
#define BAND5_PRELIM_COEF_SIZE                    (3)

#define AEC_AIRDUMP_FRAME_SIZE                    (AEC_PRELIM_COEF_SIZE + AEC_REF_GAIN_SIZE + AEC_DUALMIC_POWER_COEF_SIZE + AEC_INEAR_SIZE + BAND5_PRELIM_COEF_SIZE)

#define AEC_AIRDUMP_FRAME_NUM                     (10)

typedef struct {
    bool past_state;
    bool over_halfbuf;
} AIRDUMP_WO_STATE_t;

typedef struct {
    AIRDUMP_WO_STATE_t  wo_state;
    bool                airdump_en;
} AIRDUMP_DSP_CTRL_t;

AIRDUMPCTRL_t *rAirDumpCtrl;

static AIRDUMP_DSP_CTRL_t rAirDumpDSPCtrl;

static bool AEC_NR_Check2MicEnabled(void)
{
    bool Status;

    if ((g_voice_ecnr_nvkey_aec->AEC_NR_EN & 0x0005) == 0x0005) {
        //DSP_MW_LOG_I("[ECNR] It's 2-mic ECNR algorithm, 0x%x", 1, (g_voice_ecnr_nvkey_aec->AEC_NR_EN & 0x0005));
        Status = TRUE;
    } else {
        //DSP_MW_LOG_I("[ECNR] It's 1-mic ECNR algorithm, 0x%x", 1, (g_voice_ecnr_nvkey_aec->AEC_NR_EN & 0x0005));
        Status = FALSE;
    }

    return Status;
}

static void AirDump_update_notify_count(uint32_t writeoffset)
{
    bool wo_past = rAirDumpDSPCtrl.wo_state.past_state;
    bool wo_over = rAirDumpDSPCtrl.wo_state.over_halfbuf;
    if ((writeoffset >= (rAirDumpCtrl->length / 2)) && (wo_over == FALSE)) {
        wo_over = TRUE;
    } else if ((writeoffset < (rAirDumpCtrl->length / 2)) && (wo_over == TRUE)) {
        wo_over = FALSE;
    }

    if (wo_past != wo_over) {
        rAirDumpCtrl->notify_count++;
        wo_past = wo_over;
        rAirDumpDSPCtrl.wo_state.over_halfbuf = wo_over;
        rAirDumpDSPCtrl.wo_state.past_state = wo_past;
    }
}

static void AirDump_update_writeoffset_share_information(uint32_t writeoffset)
{
    rAirDumpCtrl->write_offset = writeoffset;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void voice_nr_airdump_on_off(bool state)
{
    rAirDumpDSPCtrl.airdump_en = state;
    U32 mask;
    if (state == TRUE) {
        hal_nvic_save_and_set_interrupt_mask(&mask);
        rAirDumpCtrl->length = (sizeof(int16_t)) * (AEC_AIRDUMP_FRAME_SIZE) * AEC_AIRDUMP_FRAME_NUM;
        rAirDumpCtrl->write_offset = 0;
        rAirDumpCtrl->read_offset = 0;
        rAirDumpCtrl->notify_count = 0;
        hal_nvic_restore_interrupt_mask(mask);
        rAirDumpDSPCtrl.wo_state.over_halfbuf = false;

        DSP_MW_LOG_I("[DSP][VOICE_NR] [AirDump] AIRDUMP Enable Done\r\n", 0);
    } else if (state == FALSE) {
        hal_nvic_save_and_set_interrupt_mask(&mask);
        rAirDumpCtrl->length = 0;
        rAirDumpCtrl->write_offset = 0;
        rAirDumpCtrl->read_offset = 0;
        rAirDumpCtrl->notify_count = 0;
        rAirDumpDSPCtrl.wo_state.over_halfbuf = false;
        hal_nvic_restore_interrupt_mask(mask);
        DSP_MW_LOG_I("[DSP][VOICE_NR] [AirDump] AIRDUMP Disable Done\r\n", 0);
    }
}

void voice_nr_airdump(void)
{
    int16_t i2Buf[AEC_AIRDUMP_FRAME_SIZE];
    uint16_t u2BufLen = sizeof(int16_t) * (AEC_AIRDUMP_FRAME_SIZE);
    uint8_t *wptr;
    uint32_t writeoffset;

    if (!(rAirDumpDSPCtrl.airdump_en && AEC_NR_Check2MicEnabled())) {
        return;
    }

    EC_PreLim_Coef_READBACK(&i2Buf[0]);
    if (gDspAlgParameter.EscoMode.Tx != VOICE_FB) {
        EC_REF_GAIN_READBACK(&i2Buf[AEC_PRELIM_COEF_SIZE]);
    } else {
        #if defined (AIR_VOICE_BAND_CONFIG_TYPE_FB)
        EC_REF_GAIN_READBACK_FB(&i2Buf[AEC_PRELIM_COEF_SIZE]);
        #endif
    }

    i2Buf[AEC_PRELIM_COEF_SIZE + AEC_REF_GAIN_SIZE] = DaulMIC_power_Coef_READBACK();

#ifdef AIR_ECNR_1MIC_INEAR_ENABLE
    Get_AirDump_Inear(&i2Buf[AEC_PRELIM_COEF_SIZE + AEC_REF_GAIN_SIZE + AEC_DUALMIC_POWER_COEF_SIZE]);
#endif

    if (gDspAlgParameter.EscoMode.Tx == VOICE_SWB){
        BAND5_PreLim_Coef_READBACK(&i2Buf[AEC_PRELIM_COEF_SIZE+AEC_REF_GAIN_SIZE+AEC_DUALMIC_POWER_COEF_SIZE+AEC_INEAR_SIZE]);
    }

    wptr = (uint8_t *)(&rAirDumpCtrl->data[0] + rAirDumpCtrl->write_offset);
    memcpy(wptr, (uint8_t *)i2Buf, u2BufLen);
    writeoffset = (rAirDumpCtrl->write_offset + u2BufLen) % (rAirDumpCtrl->length);

    AirDump_update_notify_count(writeoffset);
    AirDump_update_writeoffset_share_information(writeoffset);
}

#endif

//==============================================
//    NR bypass
//==============================================

static bool gAecNrEn_MP = true;

void voice_nr_enable(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);

    switch (msg.ccni_message[1]) {
        case 0:
            DSP_MW_LOG_I("[DSP][VOICE_NR] Disable", 0);
            gAecNrEn_MP = FALSE;
            break;
        case 1:
            DSP_MW_LOG_I("[DSP][VOICE_NR] Enable", 0);
            gAecNrEn_MP = TRUE;
            break;
    }
}


bool voice_nr_check_enable(void)
{
    if ((!(g_voice_ecnr_nvkey_aec->AEC_NR_EN & 0x0001)) || (gAecNrEn_MP == FALSE)) {
        //DSP_MW_LOG_I("[AEC] Disable", 0);
        return false;
    }

    return true;
}


//==============================================
//    Get Ref Gain
//==============================================

void voice_nr_get_ref_gain(int16_t *gain_addr)
{
    uint32_t length;

    AUDIO_ASSERT(gain_addr != NULL);

#if defined(AIR_3RD_PARTY_NR_ENABLE) && defined(AIR_ECNR_PREV_PART_ENABLE)
#ifndef AIR_ECNR_SEPARATE_MODE_ENABLE
    length = voice_ecnr_ec_get_ref_gain(gain_addr);
#endif
#endif
#if defined(AIR_3RD_PARTY_NR_ENABLE)
    length = voice_ecnr_ec_postec_get_ref_gain(gain_addr);
#endif
#if defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE) || defined(AIR_ECNR_1_OR_2_MIC_ENABLE)
    length = voice_ecnr_all_get_ref_gain(gain_addr);
#endif

    if (length == 0) {
        memset(gain_addr, 0, sizeof(int16_t) * 8);
    }
}


//==============================================
//    AVC control
//==============================================
typedef enum {
    avc_vol_dB0 = 8192,
    avc_vol_dB1 = 9192,
    avc_vol_dB2 = 10314,
    avc_vol_dB3 = 11572,
    avc_vol_dB4 = 12984,
    avc_vol_dB5 = 14568,
    avc_vol_dB6 = 16346,
    avc_vol_dB7 = 18340,
    avc_vol_dB8 = 20578,
    avc_vol_dB9 = 23089,
    avc_vol_dB10 = 25906,
    avc_vol_dB11 = 29067,
    avc_vol_dB12 = 32613,
    avc_vol_max  = 32767,
} eAVC_VOL_DB;

extern bool gfgAvcUpdate;
extern bool gfgAvcSend;
static int16_t gi2AvcVol;

static bool avc_vol_compare(int16_t i2PreAvcVol, int16_t i2PostAvcVol)
{
    bool diffrent_dB_class;

    if (avc_vol_dB0 <= i2PreAvcVol && i2PreAvcVol < avc_vol_dB4) {
        if (avc_vol_dB0 <= i2PostAvcVol && i2PostAvcVol < avc_vol_dB4) {
            if (i2PostAvcVol == avc_vol_dB0 && i2PreAvcVol != avc_vol_dB0) {
                return diffrent_dB_class = true;
            }
            return diffrent_dB_class = false;
        } else {
            return diffrent_dB_class = true;
        }

    } else if (avc_vol_dB4 <= i2PreAvcVol && i2PreAvcVol < avc_vol_dB8) {
        if (avc_vol_dB4 <= i2PostAvcVol && i2PostAvcVol < avc_vol_dB8) {
            return diffrent_dB_class = false;
        } else {
            return diffrent_dB_class = true;
        }
    } else if (avc_vol_dB8 <= i2PreAvcVol) {
        if (avc_vol_dB8 <= i2PostAvcVol) {
            if (i2PostAvcVol == avc_vol_max && i2PreAvcVol != avc_vol_max) {
                return diffrent_dB_class = true;
            }
            return diffrent_dB_class = false;
        } else {
            return diffrent_dB_class = true;
        }
    } else {
        return diffrent_dB_class = false;
    }

}

/* Check whether need to update AVC to other side */
void voice_avc_vol_send_update(int16_t pre_vol, int16_t post_vol, int16_t scenario_type)
{
    hal_ccni_message_t msg;

    if (post_vol >> 6 != pre_vol >> 6) {
        if (avc_vol_compare(pre_vol, post_vol)) {
            if ((gfgAvcUpdate == false) && (gfgAvcSend == true)) {
                DSP_MW_LOG_I("[DSP][VOICE_NR] [NDVC][DSP send]former:%d, later:%d", 2, pre_vol, post_vol);
                memset((void *)&msg, 0, sizeof(hal_ccni_message_t));
                msg.ccni_message[0] = (MSG_DSP2MCU_AVC_PARA_SEND << 16) | scenario_type;
                msg.ccni_message[1] = post_vol;
                aud_msg_tx_handler(msg, 0, false);
                gfgAvcSend = false;
            }
        }
    }
    gDspAlgParameter.AecNr.avc_vol = pre_vol;

    if (gfgAvcUpdate == true) {
        gDspAlgParameter.AecNr.avc_vol = gi2AvcVol;
        gfgAvcUpdate = false;
        gfgAvcSend = true;
        //DSP_MW_LOG_I("[DSP][VOICE_NR] [NDVC][DSP send] gi2AvcVol%d, gfgAvcUpdate %d, gfgAvcSend %d",3,gi2AvcVol,gfgAvcUpdate,gfgAvcSend);
    }
}

/* Check whether need to update AVC from other side */
void voice_avc_vol_receive_update(int16_t avc_vol)
{
    if (gfgAvcUpdate == false) {
        gfgAvcUpdate = true;
        gi2AvcVol = avc_vol;
    }

    DSP_MW_LOG_I("[DSP][VOICE_NR] [NDVC][DSP receive] avc_vol: %d, gfgAvcUpdate: %d", 2, avc_vol, gfgAvcUpdate);
}


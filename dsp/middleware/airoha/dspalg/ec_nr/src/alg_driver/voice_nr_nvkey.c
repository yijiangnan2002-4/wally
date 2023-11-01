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

#ifdef AIR_BTA_IC_PREMIUM_G2

#include <string.h>
#include "voice_nr_portable.h"

static const S32 HPF_4K_COEF_PL[11 * 6] = {	//100/125/150/200/250/400 Hz
    0x0066B7CC, 0xFF995EC2, 0x0066B7CB, 0xFF9565FA, 0x005B1B9C, 0x007F18D2, 0xFF8174E2, 0x007F18D1, 0xFF84D426, 0x0079A578, 0x00400000,
    0x00617E23, 0xFF9EA361, 0x00617E22, 0xFF9A9223, 0x0053B9CF, 0x007F58FE, 0xFF818515, 0x007F58FD, 0xFF867F86, 0x0078207A, 0x00400000,
    0x005C8B74, 0xFFA3A27A, 0x005C8B73, 0xFF9FA811, 0x004CF816, 0x007F8DC2, 0xFF81B2F5, 0x007F8DC1, 0xFF885737, 0x0076A381, 0x00400000,
    0x00536B38, 0xFFACDEDC, 0x00536B37, 0xFFA98D10, 0x00412469, 0x007FD665, 0xFF8266F3, 0x007FD664, 0xFF8C8556, 0x0073C348, 0x00400000,
    0x004B396F, 0xFFB52FD1, 0x004B396E, 0xFFB30FD4, 0x00374967, 0x007FF57F, 0xFF838F16, 0x007FF57E, 0xFF9151E6, 0x007107E9, 0x00400000,
    0x00373D9D, 0xFFC98FAB, 0x00373D9C, 0xFFCD4E37, 0x0022E620, 0x007F770A, 0xFF89ACE7, 0x007F7709, 0xFFA2EF4E, 0x0069CB60, 0x00400000,
};

static const S32 HPF_8K_COEF_PL[11 * 6] = {	//100/125/150/200/250/400 Hz
    0x0070E3DC, 0xFF8F2252, 0x0070E3DB, 0xFF8AD1A9, 0x006BF686, 0x007FC644, 0xFF805D4C, 0x007FC643, 0xFF820863, 0x007CC5B9, 0x00400000,
    0x006E8953, 0xFF918023, 0x006E8952, 0xFF8D7D61, 0x00677891, 0x007F4B64, 0xFF80EBFB, 0x007F4B63, 0xFF82A971, 0x007BFB13, 0x00400000,
    0x006C3CE2, 0xFF93D076, 0x006C3CE1, 0xFF9024CD, 0x00632B7B, 0x007ECD00, 0xFF818273, 0x007ECCFF, 0xFF835685, 0x007B3222, 0x00400000,
    0x0066B827, 0xFF995E66, 0x0066B826, 0xFF95658B, 0x005B1C40, 0x007F18E9, 0xFF8174C6, 0x007F18E8, 0xFF84D405, 0x0079A599, 0x00400000,
    0x00617E7A, 0xFF9EA30A, 0x00617E79, 0xFF9A91B6, 0x0053BA65, 0x007F5916, 0xFF8184F6, 0x007F5915, 0xFF867F61, 0x0078209A, 0x00400000,
    0x00536B7C, 0xFFACDE96, 0x00536B7B, 0xFFA98CAE, 0x004124D6, 0x007FD67F, 0xFF8266CE, 0x007FD67E, 0xFF8C8529, 0x0073C364, 0x00400000,
};

static const S32 HPF_16K_COEF_PL[11 * 6] = {	//100/125/150/200/250/400 Hz
    0x0076FE9F, 0xFF890301, 0x0076FE9E, 0xFF856EC7, 0x00758D42, 0x007F5D7F, 0xFF80AB5D, 0x007F5D7E, 0xFF80EB0D, 0x007E5FE2, 0x00400000,
    0x0075BDA6, 0xFF8A44DE, 0x0075BDA5, 0xFF86C8B8, 0x007313B6, 0x007F24C5, 0xFF80E90D, 0x007F24C4, 0xFF812DB7, 0x007DF8C7, 0x00400000,
    0x0074806F, 0xFF8B8327, 0x0074806E, 0xFF8821D9, 0x0070A7A6, 0x007EEB1E, 0xFF8128BF, 0x007EEB1D, 0xFF817380, 0x007D920E, 0x00400000,
    0x0070E40C, 0xFF8F2222, 0x0070E40B, 0xFF8AD173, 0x006BF6E3, 0x007FC64E, 0xFF805D41, 0x007FC64D, 0xFF820857, 0x007CC5C8, 0x00400000,
    0x006E8984, 0xFF917FF1, 0x006E8983, 0xFF8D7D28, 0x006778EF, 0x007F4B6F, 0xFF80EBEE, 0x007F4B6E, 0xFF82A963, 0x007BFB24, 0x00400000,
    0x0066B855, 0xFF995E38, 0x0066B854, 0xFF956553, 0x005B1C92, 0x007F18F4, 0xFF8174B7, 0x007F18F3, 0xFF84D3F4, 0x0079A5A9, 0x00400000,
};

#define MEMCPY(A,B,C) memcpy((void *)&(A), (void *)&(B), ((U32)&(C)-(U32)&(A)+2))
#define MEMCPY4(A,B,C) memcpy((void *)&(A), (void *)&(B), ((U32)&(C)-(U32)&(A)+4))

static void HPF_COEF_ASSIGN(U8 *coef, S32 *tab, int sw)
{
    S32 *tab_coef = tab + 11 * sw;
    memcpy(coef, tab_coef, 11 * sizeof(S32));
}

void voice_nvkey_aec_nr_porting_layer(void *p_nvkey_aec, void *p_nvkey_nr, void *p_nvkey_inear, void *p_nvkey_aec_nr)
{
    int HPF_sw;
    DSP_PARA_AEC_STRU *aec = (DSP_PARA_AEC_STRU *)p_nvkey_aec;
    DSP_PARA_NR_STRU *nr = (DSP_PARA_NR_STRU *)p_nvkey_nr;
    DSP_PARA_INEAR_STRU *inear = (DSP_PARA_INEAR_STRU *)p_nvkey_inear;
    DSP_PARA_AEC_NR_STRU *all = (DSP_PARA_AEC_NR_STRU *)p_nvkey_aec_nr;
    memset(all, 0, sizeof(DSP_PARA_AEC_NR_STRU));
    all->ENABLE = 1;
    all->REVISION = 1;
    //p_nvkey_aec -> p_nvkey_aec_nr
    MEMCPY(all->AEC_NR_EN, aec->AEC_NR_EN, all->CH4_REF2_GAIN);
    all->DSP_EC_SW = aec->DSP_EC_SW;
    all->AEC_Corel_Ratio1_Sensitive = aec->AEC_Corel_Ratio1_Sensitive;
    MEMCPY(all->AEC_Corel_Ratio4_Sensitive, aec->AEC_Corel_Ratio4_Sensitive, all->AEC_REF_GAIN_AUTO);
    all->AEC_tap_length = aec->AEC_tap_length;
    MEMCPY(all->AEC_ENG_VAD_THR, aec->AEC_ENG_VAD_THR, all->AEC_fast_dn_alpha);
    all->SBEC_PF_order_12      = aec->SBEC_PF_order_12     ;
    all->SBEC_PF_order_34      = aec->SBEC_PF_order_34     ;
    all->SBEC_DT_ratio_thrd_12 = aec->SBEC_DT_ratio_thrd_12;
    all->SBEC_DT_ratio_thrd_34 = aec->SBEC_DT_ratio_thrd_34;
    all->SBEC_PF_MIN_12        = aec->SBEC_PF_MIN_12       ;
    all->SBEC_PF_MIN_34        = aec->SBEC_PF_MIN_34       ;
    all->SBEC_noise_paste_gain = aec->SBEC_noise_paste_gain;
    //p_nvkey_nr -> p_nvkey_aec_nr
    MEMCPY(all->AVC_ALPHA, nr->AVC_ALPHA, all->AVC_VOL_MAX);
    MEMCPY(all->WB_NR_TX_POW_MIN_BUF_PERIOD, nr->WB_NR_TX_POW_MIN_BUF_PERIOD, all->NB_VOICE_TX_GAIN);
    MEMCPY(all->PD_VOICE_IN_WIND, nr->PD_VOICE_IN_WIND, all->M2_BANDH_GAIN);
    MEMCPY(all->M2_REF_MIC_CH1_GAIN, nr->M2_REF_MIC_CH1_GAIN, all->Harmonics_thrd);
    MEMCPY(all->M2_lp_coef_attu, nr->M2_lp_coef_attu, all->low_noise_fast_alpha);
    HPF_sw = nr->HPF_COEF_SW;
    MEMCPY(all->high_noise_thrd, nr->high_noise_thrd, all->M2_VAD_THRD_43_high);
    //p_nvkey_inear -> p_nvkey_aec_nr
    MEMCPY(all->IE_main_type, inear->IE_main_type, all->IE_CH4_REF_GAIN);
    MEMCPY4(all->IE_main_nest_over_est1, inear->IE_main_nest_over_est1, all->IE_inear_nest_decay1);
    // one of both is used
#if 0
    all->IE_Dummy2 = inear->IE_Dummy2;
#else
    all->mic_noise_thr = inear->IE_Dummy2;
#endif
    MEMCPY(all->IE_MSC_VAD_THR, inear->IE_MSC_VAD_THR, all->IE_band_ratio_alpha);
    all->IE_SBEC_PF_order_12     = inear->IE_SBEC_PF_order_12     ;
    all->IE_SBEC_PF_order_34     = inear->IE_SBEC_PF_order_34     ;
    all->IE_SBEC_DT_ratio_thrd_12 = inear->IE_SBEC_DT_ratio_thrd_12;
    all->IE_SBEC_DT_ratio_thrd_34 = inear->IE_SBEC_DT_ratio_thrd_34;
    all->IE_SBEC_PF_MIN_12       = inear->IE_SBEC_PF_MIN_12       ;
    all->IE_SBEC_PF_MIN_34       = inear->IE_SBEC_PF_MIN_34       ;
    MEMCPY(all->vad_end_bin, inear->vad_end_bin, all->IE_wind_pow_thd);
    //HPF COEF
    HPF_COEF_ASSIGN((U8 *)&all->VOICE_TX_HP_COEF_0, (S32 *)HPF_4K_COEF_PL, HPF_sw);
    HPF_COEF_ASSIGN((U8 *)&all->VOICE_WB_RX_HP_COEF_0, (S32 *)HPF_16K_COEF_PL, HPF_sw);
    HPF_COEF_ASSIGN((U8 *)&all->VOICE_NB_RX_HP_COEF_0, (S32 *)HPF_8K_COEF_PL, HPF_sw);
    HPF_COEF_ASSIGN((U8 *)&all->VOICE_IE_HP_COEF_0, (S32 *)HPF_16K_COEF_PL, HPF_sw);
}

#endif

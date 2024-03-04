/*
 * Copyright (c) 2012-2013 by Tensilica Inc. ALL RIGHTS RESERVED.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef __PRELOADER_PILSI_CONFIGURE_H__
#define __PRELOADER_PILSI_CONFIGURE_H__

#include "xt_library_loader.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifdef MTK_BT_A2DP_LC3_USE_PIC
extern xtlib_packaged_library pisplit_lc3_codec;
extern xtlib_packaged_library pisplit_lc3i_fft7p5ms;
extern xtlib_packaged_library pisplit_lc3i_fft10ms;
#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
extern xtlib_packaged_library pisplit_lc3i_codec_APLC;
#else
extern xtlib_packaged_library pisplit_lc3i_codec;
#endif
#ifdef AIR_BT_CODEC_BLE_V2_ENABLED
#define LC3_LIB &pisplit_lc3_codec,
#else
#define LC3_LIB
#endif
#define LC3I_FFT7_5MS_LIB &pisplit_lc3i_fft7p5ms,
#define LC3I_FFT10MS_LIB &pisplit_lc3i_fft10ms,
#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)
#define LC3I_LIB &pisplit_lc3i_codec_APLC,
#else
#define LC3I_LIB &pisplit_lc3i_codec,
#endif
#else
#define LC3_LIB
#define LC3I_FFT7_5MS_LIB
#define LC3I_FFT10MS_LIB
#define LC3I_LIB
#endif

#ifdef AIR_AUDIO_ULD_ENCODE_USE_PIC_ENABLE
extern xtlib_packaged_library pisplit_uld_enc;
#define ULD_ENC_LIB &pisplit_uld_enc,
#else
#define ULD_ENC_LIB
#endif

#ifdef AIR_AUDIO_ULD_DECODE_USE_PIC_ENABLE
extern xtlib_packaged_library pisplit_uld_dec;
#define ULD_DEC_LIB &pisplit_uld_dec,
#else
#define ULD_DEC_LIB
#endif

#ifdef MTK_BT_A2DP_AAC_USE_PIC
extern xtlib_packaged_library pisplit_AAC_dec_5x;
#define AAC_LIB &pisplit_AAC_dec_5x,
#else
#define AAC_LIB
#endif

#ifdef MTK_BT_PEQ_USE_PIC
extern xtlib_packaged_library pisplit_peq2;// user muts be extern your lib to here!!!
#define PEQ_LIB &pisplit_peq2,
#else
#define PEQ_LIB
#endif

#ifdef MTK_BT_A2DP_SBC_USE_PIC
#define SBC_LIB &pisplit_sbc_dec,
extern xtlib_packaged_library pisplit_sbc_dec;
#else
#define SBC_LIB
#endif

#ifdef AIR_BT_A2DP_SBC_ENCODER_ENABLE
#define SBC_ENC_LIB &pisplit_sbc_enc,
extern xtlib_packaged_library pisplit_sbc_enc;
#else
#define SBC_ENC_LIB
#endif


#ifdef MTK_BT_A2DP_MSBC_USE_PIC
extern xtlib_packaged_library pisplit_msbc_dec;// user muts be extern your lib to here!!!
extern xtlib_packaged_library pisplit_msbc_enc;// user muts be extern your lib to here!!!
#define MSBC_DEC_LIB &pisplit_msbc_dec,
#define MSBC_ENC_LIB &pisplit_msbc_enc,
#else
#define MSBC_DEC_LIB
#define MSBC_ENC_LIB
#endif

#ifdef MTK_BT_CLK_SKEW_USE_PIC
extern xtlib_packaged_library pisplit_skew_ctrl;// user muts be extern your lib to here!!!
#define CLK_SKEW_LIB &pisplit_skew_ctrl,
#else
#define CLK_SKEW_LIB
#endif

#ifdef MTK_PLC_USE_PIC
extern xtlib_packaged_library pisplit_plc_pitch;// user muts be extern your lib to here!!!
#define PLC_LIB &pisplit_plc_pitch,
#else
#define PLC_LIB
#endif

#ifdef MTK_BT_A2DP_CPD_USE_PIC
extern xtlib_packaged_library pisplit_cpd;// user muts be extern your lib to here!!!
#define CPD_LIB &pisplit_cpd,
#else
#define CPD_LIB
#endif

#if defined(AIR_BTA_IC_PREMIUM_G2) && defined(MTK_BT_A2DP_CPD_USE_PIC) && defined(AIR_HEARING_PROTECTION_ENABLE)
extern xtlib_packaged_library pisplit_cpd_hp;// user muts be extern your lib to here!!!
#define CPD_HP_LIB &pisplit_cpd_hp,
#else
#define CPD_HP_LIB
#endif

#ifdef MTK_BT_AGC_USE_PIC
extern xtlib_packaged_library pisplit_agc;// user muts be extern your lib to here!!!
#define AGC_LIB &pisplit_agc,
#else
#define AGC_LIB
#endif

#ifdef AIR_ADAPITVE_EQ_DETECTION_USE_PIC_ENABLE
extern xtlib_packaged_library pisplit_aeq;// user muts be extern your lib to here!!!
#define AEQ_LIB &pisplit_aeq,
#else
#define AEQ_LIB
#endif

#ifdef MTK_WWE_USE_PIC

#ifdef MTK_WWE_AMA_USE_PIC
extern xtlib_packaged_library pisplit_library_pryon1000;// user muts be extern your lib to here!!!
#define WWE_LIB_AMA &pisplit_library_pryon1000,
#else
#define WWE_LIB_AMA
#endif
#ifdef MTK_WWE_GSOUND_USE_PIC
extern xtlib_packaged_library pisplit_google_hotword_dsp_multi_bank;// user muts be extern your lib to here!!!
#define WWE_LIB_GSOUND &pisplit_google_hotword_dsp_multi_bank,
#else
#define WWE_LIB_GSOUND
#endif
#else
#define WWE_LIB_AMA
#define WWE_LIB_GSOUND
#endif

#ifdef AIR_VOICE_NR_USE_PIC_ENABLE

#if defined(AIR_BTA_IC_PREMIUM_G2) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
#if (defined(AIR_ECNR_1_OR_2_MIC_ENABLE) || defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE))
extern xtlib_packaged_library pisplit_ecnr_inear_v2_swb_prcs;// user must be extern your lib to here!!!
#define ECNR_LIB &pisplit_ecnr_inear_v2_swb_prcs,
#elif defined(AIR_3RD_PARTY_NR_ENABLE)
extern xtlib_packaged_library pisplit_ec_rxnr_swb_prcs;// user must be extern your lib to here!!!
#define ECNR_LIB &pisplit_ec_rxnr_swb_prcs,
#if defined(AIR_VOICE_BAND_CONFIG_TYPE_FB) && defined(AIR_BTA_IC_STEREO_HIGH_G3)
extern xtlib_packaged_library pisplit_ec_fb;// user must be extern your lib to here!!!
#define ECFB_LIB &pisplit_ec_fb,
#endif
#endif
#else
#if (defined(AIR_ECNR_1_OR_2_MIC_ENABLE) || defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE))
extern xtlib_packaged_library pisplit_ecnr_inear_v2_swb;// user must be extern your lib to here!!!
#define ECNR_LIB &pisplit_ecnr_inear_v2_swb,
#elif defined(AIR_3RD_PARTY_NR_ENABLE)
extern xtlib_packaged_library pisplit_ec_rxnr_swb_prcs;// user must be extern your lib to here!!!
#define ECNR_LIB &pisplit_ec_rxnr_swb_prcs,
#else
#define ECNR_LIB
#endif
#endif
#else
#define ECNR_LIB
#endif

#ifndef ECFB_LIB
#define ECFB_LIB
#endif

#ifdef AIR_VOICE_NR_ENABLE
#ifdef AIR_ECNR_PREV_PART_USE_PIC_ENABLE
#ifndef AIR_ECNR_SEPARATE_MODE_ENABLE
#if defined(AIR_BTA_IC_PREMIUM_G2)
extern xtlib_packaged_library pisplit_ec120;// user muts be extern your lib to here!!!
#define EC120_LIB &pisplit_ec120,
#elif defined(AIR_BTA_IC_PREMIUM_G3)
extern xtlib_packaged_library pisplit_ec120_swb;// user muts be extern your lib to here!!!
#define EC120_LIB &pisplit_ec120_swb,
#else
#define EC120_LIB
#endif
#else
#define EC120_LIB
#endif
#else
#define EC120_LIB
#endif
#else
#define EC120_LIB
#endif

#ifdef AIR_VOICE_NR_ENABLE
#ifdef AIR_ECNR_POST_PART_USE_PIC_ENABLE
#ifndef AIR_ECNR_SEPARATE_MODE_ENABLE
     extern xtlib_packaged_library pisplit_post_ec;// user muts be extern your lib to here!!!
     #define POST_EC_LIB &pisplit_post_ec,
#else
     #define POST_EC_LIB
#endif
#else
     #define POST_EC_LIB
#endif
#else
     #define POST_EC_LIB
#endif

#ifdef AIR_VOICE_NR_USE_PIC_ENABLE
#if defined(AIR_3RD_PARTY_NR_ENABLE) && !defined(AIR_BTA_IC_STEREO_HIGH_G3)
extern xtlib_packaged_library pisplit_fir_eq;// user muts be extern your lib to here!!!
#define TX_EQ_LIB &pisplit_fir_eq,
#else
#define TX_EQ_LIB
#endif
#else
#define TX_EQ_LIB
#endif

#ifdef AIR_VOICE_NR_USE_PIC_ENABLE
#if defined(AIR_3RD_PARTY_NR_ENABLE)
#if defined(AIR_AI_NR_PREMIUM_ENABLE) || defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)
#if defined(AIR_AI_NR_PREMIUM_INEAR_500K_PRO_DISTRACTOR_ENABLE)
extern xtlib_packaged_library pisplit_igo_aibf15;// user muts be extern your lib to here!!!
#define IGO_TXNR_LIB &pisplit_igo_aibf15,
#else
extern xtlib_packaged_library pisplit_igo_txnr;// user muts be extern your lib to here!!!
#define IGO_TXNR_LIB &pisplit_igo_txnr,
#endif
#else
#define IGO_TXNR_LIB
#endif
#else
#define IGO_TXNR_LIB
#endif
#else
#define IGO_TXNR_LIB
#endif

#ifdef AIR_VOICE_NR_USE_PIC_ENABLE
#if defined(AIR_HFP_DNN_PATH_ENABLE) && defined(AIR_DNN_LIB_ENABLE)
extern xtlib_packaged_library pisplit_igo_txnr;// user muts be extern your lib to here!!!
#define DNN_NR_LIB &pisplit_igo_txnr,
#else
#define DNN_NR_LIB
#endif
#else
#define DNN_NR_LIB
#endif

#ifdef AIR_BT_LE_LC3PLUS_USE_PIC
#ifdef AIR_BT_LE_LC3PLUS_USE_ALL_MODE
extern xtlib_packaged_library pisplit_lc3plus_codec_all;
#define LC3PLUS_LIB &pisplit_lc3plus_codec_all,
#define LC3PLUSI_LIB
#define LC3PLUSI_FFT2_5MS_LIB
#define LC3PLUSI_FFT5_0MS_LIB
#define LC3PLUSI_FFT10_0MS_LIB
#else
//extern xtlib_packaged_library pisplit_lc3plus_codec;
extern xtlib_packaged_library pisplit_lc3plusi_codec;
extern xtlib_packaged_library pisplit_lc3plusi_fft2p5ms;
extern xtlib_packaged_library pisplit_lc3plusi_fft5ms;
extern xtlib_packaged_library pisplit_lc3plusi_fft10ms;
#define LC3PLUS_LIB// &pisplit_lc3plus_codec,
#define LC3PLUSI_LIB &pisplit_lc3plusi_codec,
#define LC3PLUSI_FFT2_5MS_LIB &pisplit_lc3plusi_fft2p5ms,
#define LC3PLUSI_FFT5_0MS_LIB &pisplit_lc3plusi_fft5ms,
#define LC3PLUSI_FFT10_0MS_LIB &pisplit_lc3plusi_fft10ms,
#endif /* AIR_BT_LE_LC3PLUS_USE_ALL_MODE */
#else
#define LC3PLUS_LIB
#define LC3PLUSI_LIB
#define LC3PLUSI_FFT2_5MS_LIB
#define LC3PLUSI_FFT5_0MS_LIB
#define LC3PLUSI_FFT10_0MS_LIB
#endif

#if defined(AIR_BT_A2DP_CVSD_USE_PIC_ENABLE) && defined(AIR_BT_HFP_CVSD_ENABLE)
extern xtlib_packaged_library pisplit_cvsd_dec;// user muts be extern your lib to here!!!
extern xtlib_packaged_library pisplit_cvsd_enc;// user muts be extern your lib to here!!!
#define CVSD_DEC_LIB &pisplit_cvsd_dec,
#define CVSD_ENC_LIB &pisplit_cvsd_enc,
#else
#define CVSD_DEC_LIB
#define CVSD_ENC_LIB
#endif

// user muts be add your lib to here!!!
#if defined(AIR_ANC_USER_UNAWARE_ENABLE) && defined(AIR_USER_UNAWARE_USE_PIC)
extern xtlib_packaged_library pisplit_UserUnaware;// user muts be extern your lib to here!!!
#define UNAWARE_LIB &pisplit_UserUnaware,
#else
#define UNAWARE_LIB
#endif

#if defined(AIR_AFC_ENABLE) && defined(AIR_AFC_USE_PIC)
extern xtlib_packaged_library pisplit_lp_afc_1568;// user muts be extern your lib to here!!!
#define AFC_LIB &pisplit_lp_afc_1568,
#else
#define AFC_LIB
#endif

#if defined(AIR_LD_NR_ENABLE) && defined(AIR_LD_NR_USE_PIC)
extern xtlib_packaged_library pisplit_ld_nr_1565;// user muts be extern your lib to here!!!
#define LD_NR_LIB &pisplit_ld_nr_1565,
#else
#define LD_NR_LIB
#endif

#if defined(AIR_AT_AGC_ENABLE) && defined(AIR_AT_AGC_USE_PIC)
extern xtlib_packaged_library pisplit_agc_1565;// user muts be extern your lib to here!!!
#define AT_AGC_LIB &pisplit_agc_1565,
#else
#define AT_AGC_LIB
#endif

#ifdef AIR_FIXED_RATIO_SRC_USE_PIC
extern xtlib_packaged_library pisplit_sampler_by_n;
#define FIXED_RATIO_SRC_LIB &pisplit_sampler_by_n,
#else
#define FIXED_RATIO_SRC_LIB
#endif /* AIR_FIXED_RATIO_SRC_USE_PIC */

#ifdef MTK_BT_A2DP_VENDOR_1_USE_PIC
#include "dsp_vendor_codec_sdk.h"
extern xtlib_packaged_library vendor_1_pic_library;// user muts be extern your lib to here!!!
#else
#define VEND_1_LIB
#endif

#ifdef MTK_BT_CELT_USE_PIC
#ifdef AIR_CELT_MULTI_VERSIONS_SUPPORT
extern xtlib_packaged_library pisplit_celt_codec_1565;
extern xtlib_packaged_library pisplit_celt_codec_21073101_1565;
#define CELT_LIB &pisplit_celt_codec_1565, &pisplit_celt_codec_21073101_1565,
#else
extern xtlib_packaged_library pisplit_celt_codec_1565;
#define CELT_LIB &pisplit_celt_codec_1565,
#endif /* AIR_CELT_MULTI_VERSIONS_SUPPORT */
#else
#define CELT_LIB
#endif

#ifdef MTK_BT_A2DP_VENDOR_2_USE_PIC
extern xtlib_packaged_library pisplit_vendor_2_dec;
#define VENDOR_2_DEC_LIB &pisplit_vendor_2_dec,
#else
#define VENDOR_2_DEC_LIB
#endif

#ifdef AIR_FULL_ADAPTIVE_ANC_USE_PIC
#if defined(AIR_BTA_IC_PREMIUM_G3)
extern xtlib_packaged_library pisplit_full_adapt;
#define FULL_ADAPT_LIB &pisplit_full_adapt,
#else
extern xtlib_packaged_library pisplit_smart_adapt;
#define FULL_ADAPT_LIB &pisplit_smart_adapt,
#endif
#else
#define FULL_ADAPT_LIB
#endif

#ifdef AIR_HW_VIVID_PT_USE_PIC_ENABLE
extern xtlib_packaged_library pisplit_ld_nr_1577;
#define VIVID_PT_LD_NR_LIB &pisplit_ld_nr_1577,
#else
#define VIVID_PT_LD_NR_LIB
#endif

#ifdef AIR_BT_LHDC_ENCODER_USE_PIC
extern xtlib_packaged_library pisplit_lhdc_enc;
#define LHDC_ENC_LIB &pisplit_lhdc_enc,
#else
#define LHDC_ENC_LIB
#endif

#ifdef AIR_FADP_ANC_COMPENSATION_USE_PIC_ENABLE
extern xtlib_packaged_library pisplit_szd;
#define SZD_LIB &pisplit_szd,
#else
#define SZD_LIB
#endif

#if defined(AIR_HEARTHROUGH_HA_USE_PIC) || defined(AIR_HEARTHROUGH_PSAP_USE_PIC)
extern xtlib_packaged_library pisplit_awha_afc;
extern xtlib_packaged_library pisplit_awha_afc_fs;
extern xtlib_packaged_library pisplit_awha_fft;
extern xtlib_packaged_library pisplit_awha_nr;
extern xtlib_packaged_library pisplit_awha_ola;
extern xtlib_packaged_library pisplit_awha_beamforming;
extern xtlib_packaged_library pisplit_awha_drc;
extern xtlib_packaged_library pisplit_awha_math;
extern xtlib_packaged_library pisplit_awha_ndm;
extern xtlib_packaged_library pisplit_awha_calib;
extern xtlib_packaged_library pisplit_awha_tln;
extern xtlib_packaged_library pisplit_awha_ha_biquad;
#ifdef AIR_BTA_IC_PREMIUM_G3
extern xtlib_packaged_library pisplit_awha_wnr;
#endif
#ifdef AIR_HEARTHROUGH_HA_ENABLE
extern xtlib_packaged_library pisplit_awha_inr;
#define HEARTHROUGH_HA_LIB &pisplit_awha_afc, &pisplit_awha_afc_fs, &pisplit_awha_inr, &pisplit_awha_fft, &pisplit_awha_nr, &pisplit_awha_ola, &pisplit_awha_beamforming, &pisplit_awha_drc, &pisplit_awha_wnr, &pisplit_awha_math, &pisplit_awha_ndm, &pisplit_awha_calib, &pisplit_awha_tln, &pisplit_awha_ha_biquad,
#define HEARTHROUGH_PSAP_LIB
#else
#define HEARTHROUGH_HA_LIB
#ifdef AIR_BTA_IC_PREMIUM_G3
#define HEARTHROUGH_PSAP_LIB &pisplit_awha_afc, &pisplit_awha_afc_fs, &pisplit_awha_fft, &pisplit_awha_nr, &pisplit_awha_ola, &pisplit_awha_beamforming, &pisplit_awha_drc, &pisplit_awha_wnr, &pisplit_awha_math, &pisplit_awha_ndm, &pisplit_awha_calib, &pisplit_awha_tln, &pisplit_awha_ha_biquad,
#else
#define HEARTHROUGH_PSAP_LIB &pisplit_awha_afc, &pisplit_awha_afc_fs, &pisplit_awha_fft, &pisplit_awha_nr, &pisplit_awha_ola, &pisplit_awha_beamforming, &pisplit_awha_drc, &pisplit_awha_math, &pisplit_awha_ndm, &pisplit_awha_calib, &pisplit_awha_tln, &pisplit_awha_ha_biquad,
#endif
#endif
#else
#define HEARTHROUGH_HA_LIB
#define HEARTHROUGH_PSAP_LIB
#endif

#ifdef AIR_HEARTHROUGH_VIVID_PT_USE_PIC
extern xtlib_packaged_library pisplit_awha_vpt_afc;
extern xtlib_packaged_library pisplit_awha_vpt_limiter;
#define HEARTHROUGH_VIVID_PT_LIB &pisplit_awha_vpt_afc, &pisplit_awha_vpt_limiter,
#else
#define HEARTHROUGH_VIVID_PT_LIB
#endif


#define PIC_LIB_LIST {\
    MSBC_DEC_LIB\
    MSBC_ENC_LIB\
    PLC_LIB\
    CPD_LIB\
    CPD_HP_LIB\
    AGC_LIB\
    AEQ_LIB\
    SBC_LIB\
    CLK_SKEW_LIB\
    PEQ_LIB\
    WWE_LIB_AMA\
    WWE_LIB_GSOUND\
    ECNR_LIB\
    ECFB_LIB\
    IGO_TXNR_LIB\
    TX_EQ_LIB\
    DNN_NR_LIB\
    EC120_LIB\
    POST_EC_LIB\
    CVSD_DEC_LIB\
    CVSD_ENC_LIB\
    LC3_LIB\
    LC3I_FFT7_5MS_LIB\
    LC3I_FFT10MS_LIB\
    LC3I_LIB\
    ULD_ENC_LIB\
    ULD_DEC_LIB\
    UNAWARE_LIB\
    AFC_LIB\
    LD_NR_LIB\
    AT_AGC_LIB\
    AAC_LIB\
    LC3PLUS_LIB\
    LC3PLUSI_LIB\
    LC3PLUSI_FFT2_5MS_LIB\
    LC3PLUSI_FFT5_0MS_LIB\
    LC3PLUSI_FFT10_0MS_LIB\
    FIXED_RATIO_SRC_LIB\
    VEND_1_LIB\
    CELT_LIB\
    SBC_ENC_LIB\
    AEQ_LIB\
    VENDOR_2_DEC_LIB\
    FULL_ADAPT_LIB\
    VIVID_PT_LD_NR_LIB\
    LHDC_ENC_LIB\
    HEARTHROUGH_HA_LIB\
    HEARTHROUGH_PSAP_LIB\
    HEARTHROUGH_VIVID_PT_LIB\
    SZD_LIB\
}

#ifdef __cplusplus
}
#endif

#endif /* __PRELOADER_PILSI_CONFIGURE_H__ */

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

#ifndef __LC3_CODEC_API_H__
#define __LC3_CODEC_API_H__

#define C_LC3I_MAX_NUM 4
//#define M 16

typedef enum {
    LC3I_DL_AUDIO_10_MS   = 0, /*!<DL AUDIO mode, frame interval equals to 10 ms */
    LC3I_DL_AUDIO_7P5_MS  = 1, /*!<DL AUDIO mode, frame interval equals to 7.5 ms */
    LC3I_DL_CALL_10_MS    = 2, /*!<DL CALL mode, frame interval equals to 10 ms */
    LC3I_DL_CALL_7P5_MS   = 3, /*!<DL CALL mode, frame interval equals to 7.5 ms */
} LC3I_DL_PIC_Type_T;

typedef enum {
    LC3I_UL_CALL_10_MS    = 0, /*!<UL CALL mode, frame interval equals to 10 ms */
    LC3I_UL_CALL_7P5_MS   = 1, /*!<UL CALL mode, frame interval equals to 7.5 ms */
} LC3I_UL_PIC_Type_T;


typedef struct {
    short alg;          // codec algorithm
    void *p_mem;        // working memory pointer for encoder or decoder
    void *p_tmp;        // temp.   memory pointer for encoder or decoder
} LC3I_PTR;

typedef enum {
    C_LC3I_Enc = 0,
    C_LC3I_Dec = 1
} LC3I_Mode;

typedef enum {
    C_Alg_OFF     = 0,
    C_Alg_LC3     = 1,
    C_Alg_LC3plus = 2
} LC3I_Alg;

typedef enum {
    LC3I_PLC_STANDARD = 0,
    LC3I_PLC_ADVANCED = 1,
    LC3I_TPLC         = 2,
} LC3I_PLC_MODE_T;

typedef enum {
    LC3I_INTERVAL_2P5_MS = 25, /*!< frame interval equals to 2.5 ms */
    LC3I_INTERVAL_5_MS   = 50, /*!< frame interval equals to 5 ms */
    LC3I_INTERVAL_7P5_MS = 75, /*!< frame interval equals to 7.5 ms */
    LC3I_INTERVAL_10_MS  = 100, /*!< frame interval equals to 10 ms */
} LC3I_FRAME_INTERVAL_T;

typedef enum {
    LC3I_NO_DELAY_COMPENSATION           = 0,
    LC3I_DELAY_COMPENSATION_IN_DECODER   = 1,
    LC3I_DELAY_COMPENSATION_SPLIT_EQUALLY = 2,
} LC3I_DELAY_COMPENSATION_T;

typedef enum {
    C_framelength    = 0,
    C_mem_size       = 1,
    C_scratch_ptr    = 2
} LC3I_Reserved;

typedef struct ENCDEC {
    short alg;                  // codec algorithm: 0(NONE), 1(LC3), 2(LC3plus)
    short mode;                 // 0(encode), 1(decode)
    short plcmeth;              // 0(LC3I_PLC_STANDARD), 1(LC3I_PLC_ADVANCED Avdanced)
    short frame_ms;             // LC3: 75(7.5ms) and 100(10ms), LC3plus: 25(2.5ms), 50(5ms) and 100(10ms)
    short ch;                   // 0(mono), 1(stereo)
    short bps;                  // 16(16-bit), 24(24-bit)
    int   sr;                   // set samplerate = 48K is input as 48000
    int   bitrate;              // set birate = 32K is input as 32000
    short delay;                // 0: Don't use delay compensation
    // 1: Compensate delay in decoder (default)
    // 2: Split delay equally between encoder and decoder
    short lfe;                  // 0: normal channel for lc3plus. (default)
    // 1: low frequency enhancement channel for lc3plus.
    short Reserved[3];
} ENCDEC;

typedef struct LC3I_Param {
    short frame_ms;// LC3plus: 25(2.5ms), 50(5ms) and 100(10ms)
    int   sr;// set samplerate = 48K is input as 48000
} LC3I_Param;

typedef struct {
    void (*fix_fft_init)(void);
    void (*fix_fft10)(void);
    void (*fix_fft15)(void);
    void (*fix_fft20)(void);
    void (*fix_fft30)(void);
    void (*fix_fft40)(void);
    void (*FFT4N)(void);
    void (*FFT8N)(void);
    void (*FFT12N)(void);

} Multi_FFT;

typedef struct {
    void (*ProcessPLCmain_Adv_Init)(void);
    void (*ProcessPLCmain_Adv_fx)(void);
    void (*ProcessPLCmain_Std_fx)(void);
    void (*ProcessPLCDampingScrambling_main_fx)(void);
} Multi_APLC;

typedef enum {
    C_LC3_Voice_Enc = 0,  // Voice encoder needs to memory size
    C_LC3_Audio_Enc = 1   // Reserved
} LC3_ENC_MODE_T;

typedef enum {
    C_LC3_Voice_Dec = 0,  // Voice decoder needs to memory size
    C_LC3_Audio_Dec = 1   // Audio decoder needs to memory size
} LC3_DEC_MODE_T;

/*! Decoder packet loss concealment mode */
typedef enum {
    LC3_PLC_DISABLE  = 0,
    LC3_PLC_ADVANCED = 1,
    LC3_TPLC = 2
} LC3_PLC_MODE_T;

typedef enum {
    LC3_INTERVAL_7P5_MS = 75, /*!< frame interval equals to 7.5 ms */
    LC3_INTERVAL_10_MS = 100, /*!< frame interval equals to 10 ms */
} LC3_FRAME_INTERVAL_T;

typedef enum {
    LC3_NO_DELAY_COMPENSATION           = 0,
    LC3_DELAY_COMPENSATION_IN_DECODER   = 1,
    LC3_DELAY_COMPENSATION_SPLIT_EQUALLY = 2,
} LC3_DELAY_COMPENSATION_T;

typedef enum {
    LC3_OK = 0,                           /*!< No error occurred */
    LC3_ALIGN_ERROR,                      /*!< Unaligned pointer */
    LC3_BITRATE_ERROR,                    /*!< Invalid bitrate */
    LC3_BITRATE_SET_ERROR,                /*!< Function called after bitrate has been set */
    LC3_BITRATE_UNSET_ERROR,              /*!< Function called before bitrate has been set */
    LC3_CHANNELS_ERROR,                   /*!< Invalid number of channels */
    LC3_DECODE_ERROR,                     /*!< Frame failed to decode and was concealed */
    LC3_EPMODE_ERROR,                     /*!< Invalid EP mode */
    LC3_EPMR_ERROR,                       /*!< Invalid EPMR */
    LC3_BITDEPTH_ERROR,                   /*!< Function called with illegal bit depth */
    LC3_FRAMEMS_ERROR,                    /*!< Invalid frame_ms */
    LC3_FRAMESIZE_ERROR,                  /*!< Frame size below minimum or above maximum */
    LC3_HRMODE_ERROR,                     /*!< Invalid usage of hrmode, sampling rate and frame size */
    LC3_NULL_ERROR,                       /*!< Pointer argument is null */
    LC3_NUMBYTES_ERROR,                   /*!< Invalid number of bytes */
    LC3_PADDING_ERROR,                    /*!< Padding error */
    LC3_PLCMODE_ERROR,                    /*!< Invalid PLC method */
    LC3_PLCMODE_CONF_ERROR,               /*!< PLC method not supported due to hrmode or frame size */
    LC3_RESTRICT_BT_BINARY_ERROR,         /*!< Unsupported combination of frame length, sampling rate and bitrate */
    LC3_SAMPLERATE_ERROR,                 /*!< Invalid sample rate */
    LC3_SCRATCH_INVALID_ERROR,            /*!< Scratch space not allocated or size invalidated */
    LC3_SET_BANDWIDTH_NOT_SUPPORTED,      /*!< Bandwidth controller not available */
    LC3_LFE_MODE_NOT_SUPPORTED,           /*!< LFE support not available */
    LC3_ERROR_PROTECTION_NOT_SUPPORTED,   /*!< Error protection not available */
    LC3_ALLOC_ERROR,                      /*!< Table allocation failed */
    LC3_WAV_FORMAT_NOT_SUPPORTED,         /*!< Unsupported waveform format */

    /* START WARNING */
    LC3_WARNING,
    LC3_BW_WARNING,                       /*!< Invalid bandwidth cutoff frequency */
    LC3_Error_LAST,                       /*!< Invalid error code */

    LC3_MEM_NO_ENOUGH, // Add
} LC3_ERR_T;


#if defined(AIR_BTA_IC_PREMIUM_G3) && defined(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE)

int  LC3PLUSN_Enc_Prcs(void *p_lc3i_ptr, void *p_lc3i_tab, uint8_t *bytes, uint8_t *BufI, uint32_t *nBytes, Multi_FFT *FFTx);
int  LC3PLUSN_Dec_Prcs(void *p_lc3i_ptr, void *p_lc3i_tab, uint8_t *bytes, uint8_t *BufO, int32_t nBytes, int16_t packet_lost_st, int32_t bfi_ext, Multi_FFT *FFTx);
void LC3PLUSN_Dec_Get_Param(void *p_lc3i_ptr, uint32_t *nSamples, int32_t *delay);

//void LC3PLUSN_Tab_Link(void *p_lc3i_tab);

int LC3PLUSN_Tab_Common_Get_MemSize(void);
int LC3PLUSN_Tab_Enc_Get_MemSize(LC3I_Param *para);
#define LC3PLUSN_Tab_Dec_Get_MemSize LC3PLUSN_Tab_Enc_Get_MemSize

int LC3PLUSN_Tab_Common_Init(void *p_lc3i_tab);
int LC3PLUSN_Tab_Enc_Init(void *p_lc3i_tab, void *p_tab_mem, LC3I_Param *para);
int LC3PLUSN_Tab_Dec_Init(void *p_lc3i_tab, void *p_tab_mem, LC3I_Param *para);

uint32_t LC3PLUSN_Enc_Get_MemSize(int nChannels, int sampleRate, short frame_ms);
uint32_t LC3PLUSN_Dec_Get_MemSize(int nChannels, int sampleRate, short frame_ms, int plcMeth);

LC3_ERR_T LC3PLUSN_Enc_Init(void *p_lc3i_mem_ext, uint16_t bps, uint32_t SampleRate, uint16_t nChannels, uint32_t bitrate, uint16_t frame_ms, uint16_t delay, uint16_t lfe);
LC3_ERR_T LC3PLUSN_Dec_Init(void *p_lc3i_mem_ext, uint16_t bps, uint32_t sampleRate, uint16_t nChannels, uint16_t frame_ms, uint16_t delay, int plcMeth, int fix_concealment);

int LC3PLUSN_Enc_Set_BitRate(void *p_lc3i_ptr, int bitrate);

#else
/* V1.8 API, deprecated
LC3_ERR_T LC3_Enc_Init(void *p_lc3_mem_ext, uint16_t bits, uint32_t SampleRate, uint32_t BitRate);
LC3_ERR_T LC3_Dec_Init(void *p_lc3_mem_ext, uint32_t bits, uint32_t SampleRate, uint32_t nChannels, LC3_DEC_MODE_T mode, LC3_PLC_MODE_T plcMeth);
*/


/* V1.9 API
 */
uint32_t LC3_Enc_Get_MemSize(void);
LC3_ERR_T LC3_Enc_Init(void *p_lc3_mem_ext, uint16_t bits, uint32_t SampleRate, uint16_t nChannels, uint32_t BitRate, uint16_t frame_ms, uint16_t delay);
LC3_ERR_T LC3_Enc_Prcs(void *p_lc3_mem_ext, uint8_t *bytes, uint8_t *BufI, uint32_t *nBytes);
LC3_ERR_T LC3_Enc_Set_BitRate(void *p_lc3_mem_ext, uint32_t BitRate);

uint32_t LC3_Dec_Get_MemSize(LC3_DEC_MODE_T mode);
void LC3_Dec_Get_Param(void *p_lc3_mem_ext, uint32_t *nSamples, uint32_t *delay);
LC3_ERR_T LC3_Dec_Init(void *p_lc3_mem_ext, uint32_t bits, uint32_t SampleRate, uint32_t nChannels, LC3_DEC_MODE_T mode, LC3_PLC_MODE_T plcMeth, uint16_t frame_ms, uint16_t delay);
LC3_ERR_T LC3_Dec_Prcs(void *p_lc3_mem_ext, uint8_t *bytes, uint8_t *BufO, uint32_t nBytes, uint16_t packet_lost_st, uint32_t bfi_ext);

LC3_ERR_T LC3I_Enc_Prcs (void *p_lc3i_ptr, void *p_lc3i_tab, uint8_t *bytes, uint8_t *BufI, uint32_t *nBytes, Multi_FFT *FFTx);
LC3_ERR_T LC3I_Dec_Prcs (void *p_lc3i_ptr, void *p_lc3i_tab, uint8_t *bytes, uint8_t *BufO, int32_t nBytes, int16_t packet_lost_st, int32_t bfi_ext, Multi_FFT *FFTx);

void LC3I_Dec_Get_Param(void *p_lc3i_ptr, uint32_t *nSamples, int32_t *delay);

uint32_t LC3I_Enc_Get_MemSize(int nChannels, int sampleRate, short frame_ms);
uint32_t LC3I_Dec_Get_MemSize(int nChannels, int sampleRate, short frame_ms, int plcMeth);

LC3_ERR_T LC3I_Enc_Init(void *p_lc3i_mem_ext, uint16_t bps, uint32_t SampleRate, uint16_t nChannels, uint32_t bitrate, uint16_t frame_ms, uint16_t delay, uint16_t lfe);
LC3_ERR_T LC3I_Dec_Init(void *p_lc3i_mem_ext, uint16_t bps, uint32_t sampleRate, uint16_t nChannels, uint16_t frame_ms, uint16_t delay, int plcMeth);

int LC3I_Enc_Set_BitRate(void *p_lc3i_ptr, int bitrate);

/*Get size*/
int LC3I_Tab_Common_Get_MemSize(void);
int LC3I_Tab_Enc_Get_MemSize(LC3I_Param *para);
#define LC3I_Tab_Dec_Get_MemSize LC3I_Tab_Enc_Get_MemSize

/* Init */
uint32_t LC3I_Get_Version(void);
int LC3I_Tab_Common_Init(void *p_lc3i_tab);
int LC3I_Tab_Enc_Init(void *p_lc3i_tab, void *p_tab_mem, LC3I_Param *para);
int LC3I_Tab_Dec_Init(void *p_lc3i_tab, void *p_tab_mem, LC3I_Param *para);
//void LC3I_Tab_Link(void *p_lc3i_tab);
#endif
void LC3I_Set_Dec_Param(U16 Channel, U16 frame_interval,U16 plc_mode,U32 sample_rate, U32 bit_rate);
void LC3I_Set_Enc_Param(U16 Channel, U16 frame_interval,U32 sample_rate, U32 bit_rate);

#endif /* LC3_CODEC_API_H */


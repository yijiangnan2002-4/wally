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

#include "dsp_feature_interface.h"
#include "dsp_audio_process.h"
#include "dsp_para_cpd.h"
#include "dsp_share_memory.h"
#include "compander_interface.h"
#include "dsp_gain_control.h"
#include "dsp_rom_table.h"
#include "dsp_audio_ctrl.h"
#include "dsp_control.h"
#include "dsp_memory.h"
#include "audio_nvdm_common.h"
#include "dsp_dump.h"
#include "dsp_stream_task.h"
#include "source_inter.h"
#include "hal_audio_cm4_dsp_message.h"
#include "dsp_audio_msg.h"
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
#include "audio_transmitter_mcu_dsp_common.h"
#endif
//current cpd version: 20190625 for 88.2kHz

#ifdef MTK_BT_A2DP_CPD_USE_PIC
#include "cpd_portable.h"
#endif

#ifdef AIR_VOLUME_CONTROL_ON_DRC_ENABLE
#include "hal_audio_volume.h"
#endif

/*
 * Buffer Control
*/
VO_CPD_INSTANCE_ptr VoRxCpdMemoryPtr = NULL;
VO_CPD_INSTANCE_ptr VoTxCpdMemoryPtr = NULL;
AU_CPD_INSTANCE_ptr Au1CpdMemoryPtr = NULL;
AU_CPD_INSTANCE_ptr Au2CpdMemoryPtr = NULL;
AU_CPD_INSTANCE_ptr Au3CpdMemoryPtr = NULL;
AU_CPD_INSTANCE_ptr AdeqCpdMemoryPtr = NULL;
AU_CPD_INSTANCE_ptr AuVPCpdMemoryPtr = NULL;
AU_CPD_INSTANCE_ptr AuVPAeqCpdMemoryPtr = NULL;
AU_CPD_INSTANCE_ptr Au_Usb_CpdMemoryPtr = NULL;

/**
 *
 *  Type Enum
 *
 */

typedef enum {
    MODE_NB_TX_CPD  = 0,
    MODE_WB_TX_CPD  = 1,
    MODE_UWB_TX_CPD = 2,
    MODE_VO_TX      = MODE_UWB_TX_CPD,
    MODE_NB_RX_CPD  = 4,
    MODE_WB_RX_CPD  = 5,
    MODE_UWB_RX_CPD = 6,
    MODE_VP_CPD     = 20,
    MODE_VO_RX      = MODE_VP_CPD,
} eCPD_VO_MODE;

typedef enum {
    MODE_AU1_CPD = 0,
    MODE_AU2_CPD = 1,
    MODE_AU3_CPD = 2,
    MODE_AU4_CPD = 3,
    MODE_AU5_CPD = 4,
    MODE_AU6_CPD = 5,
    MODE_AU_USB_CPD = 6,
} eCPD_AU_MODE;

typedef enum {
    CPD_AU      = 0,
    CPD_VO      = 1,
    CPD_RMS_AU  = 2,
    CPD_RMS_VO  = 3,
} eCPD_AU_VO_MODE;

typedef enum {
    CPD_REINIT_MODE_NONE,
    CPD_REINIT_MODE_SINGLE,
    CPD_REINIT_MODE_MAX
} eCPD_reinit_mode_t;

static eCPD_reinit_mode_t g_cpd_reinit_mode = CPD_REINIT_MODE_NONE;
#define AUDIO_CPD_DEBUG_LOG_ENABLE  (0)
extern uint32_t peq_control_off;
extern uint32_t aeq_control_off;
#define CPD_AU_FRAME_SIZE           (32)
#define CPD_AU_FRAME_SIZE_8         (8)
#define CPD_AU_FRAME_SIZE_20        (20)
#define CPD_ZERO_GAIN_INDEX         (91)

#define CPD_AU_RECOVERY_GAIN        (8)     //current: peq version 20181109_v1_for_mono_mode
#define CPD3_AU_RECOVERY_GAIN       (4)    //peq3 version 20181109_v1_for_mono_mode

//#define CPD_VALID_MEMORY_CHECK_VALUE              ((U32)0x6128800)
#define CPD_VALID_MEMORY_CHECK_VALUE              ((U32)0x6A8E04DB)
#define CPD3_VALID_MEMORY_CHECK_VALUE              ((U32)0x9A9A9A9A)
#define CPDAD_VALID_MEMORY_CHECK_VALUE              ((U32)0x9A9A9A9B)
#define CPDAD_VALID_USB_MEMORY_CHECK_VALUE              ((U32)0x9A9A9A9C)
#define CPD_RUNTIME_INVALID_MEMORY_CHECK_VALUE    ((U32)0x5A5A5A5A)
#define CPD_RUNTIME_LINEIN_INVALID_MEMORY_CHECK_VALUE    ((U32)0x8A8A8A8A)
#define CPD_RUNTIME_ADEQ_INVALID_MEMORY_CHECK_VALUE    ((U32)0x8A8A8A8B)
#define CPD_RUNTIME_USB_INVALID_MEMORY_CHECK_VALUE    ((U32)0x8A8A8A8C)
#define CPD_VO_RX_VALID_MEMORY_CHECK_VALUE        ((U32)0x6A6A6A6A)
#define CPD_VO_TX_VALID_MEMORY_CHECK_VALUE        ((U32)0x7A7A7A7A)
#define CPD_AU_MEMSIZE          (sizeof(AU_CPD_INSTANCE_t))
#define CPD_VO_MEMSIZE          (sizeof(VO_CPD_INSTANCE_t))
#ifdef AIR_A2DP_DRC_TO_USE_DGAIN_ENABLE
extern uint32_t DRC_DGAIN;
#endif

static AU_CPD_CTRL_t aud_cpd_ctrl = {
    .enable = 0,
    .trigger_drc_change = 1,
    .force_disable = 0,
    .audio_path = 0,
    .peq2_nvkey_id = 0,
};

static AU_CPD_CTRL_t aud_cpd2_ctrl = {
    .enable = 0,
    .trigger_drc_change = 1,
    .force_disable = 0,
    .audio_path = 0,
};

static AU_CPD_CTRL_t aud_cpd3_ctrl = {
    .enable = 0,
    .trigger_drc_change = 1,
    .force_disable = 0,
    .audio_path = 1,
};

AU_CPD_CTRL_t aud_adaptive_eq_drc_ctrl = {
    .enable = 0,
    .trigger_drc_change = 1,
    .force_disable = 0,
    .audio_path = 2,
};

#ifdef AIR_VP_PEQ_ENABLE
static AU_CPD_CTRL_t aud_vp_cpd_ctrl = {
    .enable = 0,
    .trigger_drc_change = 1,
    .force_disable = 0,
    .audio_path = 4,
    .peq2_nvkey_id = 0,
};
#ifdef AIR_ADAPTIVE_EQ_ENABLE
static AU_CPD_CTRL_t aud_vp_aeq_cpd_ctrl = {
    .enable = 0,
    .trigger_drc_change = 1,
    .force_disable = 0,
    .audio_path = 5,
};
#endif
#endif
static AU_CPD_CTRL_t aud_usb_peq_cpd_ctrl = {
    .enable = 0,
    .trigger_drc_change = 1,
    .force_disable = 0,
    .audio_path = 8,
};

#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
extern U8 peq_get_trigger_drc(U8 phase_id, U8 type);
#endif

#ifdef AIR_HEARING_PROTECTION_ENABLE
#define LEQ_CHECK_FRAME_INTERVAL (1)
#define LEQ_CHECK_DIFF_VALUE (200)
extern U16 g_cpd_nv_length;
extern U8 g_cpd_hse_mode;
bool gLEQUpdate;
static S16 gLEQ_gain;
S16 g_LEQ_gain_prev = 0, g_LEQ_gain_post = 0;
U8 g_LEQ_cnt = 0;
#endif
/**
 *
 * Function Prototype
 *
 */
bool CPD_MemInit(void *para, eCPD_AU_VO_MODE au_vo_mode, eCPD_VO_MODE vo_mode, eCPD_AU_MODE au_mode);
void CPD_AU_MemAlloc(AU_CPD_INSTANCE_ptr *memoryptr, DSP_STREAMING_PARA_PTR stream_ptr);
void CPD_VO_MemAlloc(VO_CPD_INSTANCE_ptr *memoryptr, DSP_STREAMING_PARA_PTR stream_ptr);
bool CPD_MemCheck(eCPD_AU_VO_MODE au_vo_mode, eCPD_VO_MODE vo_mode, eCPD_AU_MODE au_mode);

bool stream_function_drc_voice_tx_swb_initialize(void *para);
bool stream_function_drc_voice_tx_wb_initialize(void *para);
bool stream_function_drc_voice_tx_nb_initialize(void *para);
bool stream_function_drc_voice_rx_swb_initialize(void *para);
bool stream_function_drc_voice_rx_wb_initialize(void *para);
bool stream_function_drc_voice_rx_nb_initialize(void *para);
bool stream_function_drc_voice_tx_process(void *para);
bool Voice_Prompt_CPD_Process(void *para);
bool stream_function_drc_voice_rx_process(void *para);
bool Voice_RX_NB_CPD_Process(void *para);
bool Voice_CPD_Process(void *para);
bool Voice_CPD_Mode_Init(void *para, eCPD_VO_MODE vo_mode);
VOID Voice_CPD_EC_Para_Init(VOID);

static const S16 DSPGAIN_Q11[124] = {
    0x0,  //for mute
    0x2,  //-60.0dB
    0x3,  //-57.0dB
    0x4,  //-54.0dB
    0x5,  //-52.0dB
    0x7,  //-49.0dB
    0x8,  //-48.0dB
    0x9,  //-47.0dB
    0xA,  //-46.0dB
    0xC,  //-45.0dB
    0xD,  //-44.0dB
    0xE,  //-43.0dB
    0x10,  //-42.0dB
    0x12,  //-41.0dB
    0x14,  //-40.0dB
    0x17,  //-39.0dB
    0x1A,  //-38.0dB
    0x1D,  //-37.0dB
    0x1F,  //-36.5dB
    0x20,  //-36.0dB
    0x22,  //-35.5dB
    0x24,  //-35.0dB
    0x27,  //-34.5dB
    0x29,  //-34.0dB
    0x2B,  //-33.5dB
    0x2E,  //-33.0dB
    0x31,  //-32.5dB
    0x33,  //-32.0dB
    0x36,  //-31.5dB
    0x3A,  //-31.0dB
    0x3D,  //-30.5dB
    0x41,  //-30.0dB
    0x45,  //-29.5dB
    0x49,  //-29.0dB
    0x4D,  //-28.5dB
    0x52,  //-28.0dB
    0x56,  //-27.5dB
    0x5B,  //-27.0dB
    0x61,  //-26.5dB
    0x67,  //-26.0dB
    0x6D,  //-25.5dB
    0x73,  //-25.0dB
    0x7A,  //-24.5dB
    0x81,  //-24.0dB
    0x89,  //-23.5dB
    0x91,  //-23.0dB
    0x9A,  //-22.5dB
    0xA3,  //-22.0dB
    0xAC,  //-21.5dB
    0xB7,  //-21.0dB
    0xC1,  //-20.5dB
    0xCD,  //-20.0dB
    0xD9,  //-19.5dB
    0xE6,  //-19.0dB
    0xF3,  //-18.5dB
    0x102,  //-18.0dB
    0x111,  //-17.5dB
    0x121,  //-17.0dB
    0x132,  //-16.5dB
    0x145,  //-16.0dB
    0x158,  //-15.5dB
    0x16C,  //-15.0dB
    0x182,  //-14.5dB
    0x199,  //-14.0dB
    0x1B1,  //-13.5dB
    0x1CA,  //-13.0dB
    0x1E6,  //-12.5dB
    0x202,  //-12.0dB
    0x221,  //-11.5dB
    0x241,  //-11.0dB
    0x263,  //-10.5dB
    0x288,  //-10.0dB
    0x2AE,  //-9.5dB
    0x2D7,  //-9.0dB
    0x302,  //-8.5dB
    0x32F,  //-8.0dB
    0x360,  //-7.5dB
    0x393,  //-7.0dB
    0x3C9,  //-6.5dB
    0x402,  //-6.0dB
    0x43F,  //-5.5dB
    0x480,  //-5.0dB
    0x4C4,  //-4.5dB
    0x50C,  //-4.0dB
    0x559,  //-3.5dB
    0x5AA,  //-3.0dB
    0x600,  //-2.5dB
    0x65B,  //-2.0dB
    0x6BB,  //-1.5dB
    0x721,  //-1.0dB
    0x78D,  //-0.5dB
    0x800,  //0.0dB
    0x879,  //0.5dB
    0x8FA,  //1.0dB
    0x982,  //1.5dB
    0xA12,  //2.0dB
    0xAAB,  //2.5dB
    0xB4D,  //3.0dB
    0xBF8,  //3.5dB
    0xCAE,  //4.0dB
    0xD6E,  //4.5dB
    0xE3A,  //5.0dB
    0xF12,  //5.5dB
    0xFF6,  //6.0dB
    0x10E8,  //6.5dB
    0x11E9,  //7.0dB
    0x12F9,  //7.5dB
    0x1418,  //8.0dB
    0x1549,  //8.5dB
    0x168C,  //9.0dB
    0x17E2,  //9.5dB
    0x194C,  //10.0dB
    0x1ACC,  //10.5dB
    0x1C63,  //11.0dB
    0x1E11,  //11.5dB
    0x1FD9,  //12.0dB
    0x21BC,  //12.5dB
    0x23BC,  //13.0dB
    0x25DA,  //13.5dB
    0x2818,  //14.0dB
    0x2A79,  //14.5dB
    0x2CFD,  //15.0dB
    0x2FA7,  //15.5dB
    0x327A,  //16.0dB
};

static const S32 DSPGAIN_Q11_32bit[140] = {
    0x00000000,     //for mute
    0x00020C49,     //-60.0dB
    0x0002E493,     //-57.0dB
    0x00041617,     //-54.0dB
    0x000524F3,     //-52.0dB
    0x0007443E,     //-49.0dB
    0x0008273A,     //-48.0dB
    0x000925E8,     //-47.0dB
    0x000A43AA,     //-46.0dB
    0x000B8449,     //-45.0dB
    0x000CEC08,     //-44.0dB
    0x000E7FAC,     //-43.0dB
    0x00104491,     //-42.0dB
    0x001240B8,     //-41.0dB
    0x00147AE1,     //-40.0dB
    0x0016FA9B,     //-39.0dB
    0x0019C865,     //-38.0dB
    0x001CEDC3,     //-37.0dB
    0x001EA495,     //-36.5dB
    0x00207567,     //-36.0dB
    0x002261C4,     //-35.5dB
    0x00246B4E,     //-35.0dB
    0x002693BF,     //-34.5dB
    0x0028DCEB,     //-34.0dB
    0x002B48C4,     //-33.5dB
    0x002DD958,     //-33.0dB
    0x003090D3,     //-32.5dB
    0x00337184,     //-32.0dB
    0x00367DDC,     //-31.5dB
    0x0039B871,     //-31.0dB
    0x003D2400,     //-30.5dB
    0x0040C371,     //-30.0dB
    0x004499D6,     //-29.5dB
    0x0048AA70,     //-29.0dB
    0x004CF8B4,     //-28.5dB
    0x00518847,     //-28.0dB
    0x00565D0A,     //-27.5dB
    0x005B7B15,     //-27.0dB
    0x0060E6C0,     //-26.5dB
    0x0066A4A5,     //-26.0dB
    0x006CB9A2,     //-25.5dB
    0x00732AE1,     //-25.0dB
    0x0079FDD9,     //-24.5dB
    0x00813856,     //-24.0dB
    0x0088E078,     //-23.5dB
    0x0090FCBF,     //-23.0dB
    0x0099940D,     //-22.5dB
    0x00A2ADAD,     //-22.0dB
    0x00AC5156,     //-21.5dB
    0x00B68737,     //-21.0dB
    0x00C157FA,     //-20.5dB
    0x00CCCCCC,     //-20.0dB
    0x00D8EF66,     //-19.5dB
    0x00E5CA14,     //-19.0dB
    0x00F367BE,     //-18.5dB
    0x0101D3F2,     //-18.0dB
    0x01111AED,     //-17.5dB
    0x012149A5,     //-17.0dB
    0x01326DD7,     //-16.5dB
    0x0144960C,     //-16.0dB
    0x0157D1AE,     //-15.5dB
    0x016C310E,     //-15.0dB
    0x0181C576,     //-14.5dB
    0x0198A135,     //-14.0dB
    0x01B0D7B1,     //-13.5dB
    0x01CA7D76,     //-13.0dB
    0x01E5A847,     //-12.5dB
    0x02026F30,     //-12.0dB
    0x0220EA9F,     //-11.5dB
    0x0241346F,     //-11.0dB
    0x02636807,     //-10.5dB
    0x0287A26C,     //-10.0dB
    0x02AE025C,     //-9.5dB
    0x02D6A866,     //-9.0dB
    0x0301B70A,     //-8.5dB
    0x032F52CF,     //-8.0dB
    0x035FA26A,     //-7.5dB
    0x0392CED8,     //-7.0dB
    0x03C90386,     //-6.5dB
    0x04026E73,     //-6.0dB
    0x043F4057,     //-5.5dB
    0x047FACCF,     //-5.0dB
    0x04C3EA83,     //-4.5dB
    0x050C335D,     //-4.0dB
    0x0558C4B2,     //-3.5dB
    0x05A9DF7A,     //-3.0dB
    0x05FFC889,     //-2.5dB
    0x065AC8C2,     //-2.0dB
    0x06BB2D60,     //-1.5dB
    0x0721482B,     //-1.0dB
    0x078D6FC9,     //-0.5dB
    0x08000000,     //0.0dB
    0x08795A04,     //0.5dB
    0x08F9E4CF,     //1.0dB
    0x09820D74,     //1.5dB
    0x0A12477C,     //2.0dB
    0x0AAB0D48,     //2.5dB
    0x0B4CE07B,     //3.0dB
    0x0BF84A66,     //3.5dB
    0x0CADDC7B,     //4.0dB
    0x0D6E30CD,     //4.5dB
    0x0E39EA8E,     //5.0dB
    0x0F11B69D,     //5.5dB
    0x0FF64C16,     //6.0dB
    0x10E86CF1,     //6.5dB
    0x11E8E6A0,     //7.0dB
    0x12F892C7,     //7.5dB
    0x141857E9,     //8.0dB
    0x15492A38,     //8.5dB
    0x168C0C59,     //9.0dB
    0x17E21048,     //9.5dB
    0x194C583A,     //10.0dB
    0x1ACC179A,     //10.5dB
    0x1C629405,     //11.0dB
    0x1E112669,     //11.5dB
    0x1FD93C1F,     //12.0dB
    0x21BC5829,     //12.5dB
    0x23BC1478,     //13.0dB
    0x25DA2345,     //13.5dB
    0x28185086,     //14.0dB
    0x2A78836F,     //14.5dB
    0x2CFCC016,     //15.0dB
    0x2FA72923,     //15.5dB
    0x327A01A4,     //16.0dB
    0x3577AEF5,     //16.5dB
    0x38A2BACB,     //17.0dB
    0x3BFDD55A,     //17.5dB
    0x3F8BD79D,     //18.0dB
    0x434FC5C2,     //18.5dB
    0x474CD1B7,     //19.0dB
    0x4B865DE3,     //19.5dB
    0x50000000,     //20.0dB
    0x54BD842B,     //20.5dB
    0x59C2F01D,     //21.0dB
    0x5F14868E,     //21.5dB
    0x64B6CADC,     //22.0dB
    0x6AAE84D8,     //22.5dB
    0x7100C4D7,     //23.0dB
    0x77B2E7FF,     //23.5dB
    0x7ECA9CD2,     //24.0dB
};

#define Q11_TABLE_SIZE (sizeof(DSPGAIN_Q11)/sizeof(DSPGAIN_Q11[0]))

/**
 * DSP_GC_ConvertQ11Form_32bit
 *
 * Convert table value to Q11 format
 *
 * @Author : Yoyo <SYChiu@airoha.com.tw>
 *
 *
 */

static S16 DSP_GC_ConvertQ11Form(S16 Value)
{
    configASSERT(Value < (S16)Q11_TABLE_SIZE);
    return DSPGAIN_Q11[Value];
}

static S32 DSP_GC_ConvertQ11Form_32bit(S16 Value)
{
    configASSERT(Value < (S16)Q11_TABLE_SIZE);
    return DSPGAIN_Q11_32bit[Value];
}

/**
 * CPD_MemInit
 *
 * This function is used to init memory space for CPD process
 *
 *
 * @para : Default parameter of callback function
 * @return : Initialize result
 */
bool CPD_MemInit(void *para, eCPD_AU_VO_MODE au_vo_mode, eCPD_VO_MODE vo_mode, eCPD_AU_MODE au_mode)
{
    DSP_STREAMING_PARA_PTR stream_ptr;
    stream_ptr = DSP_STREAMING_GET_FROM_PRAR(para);
    U32 AUInstanceSize = 0;
    U32 VOInstanceSize = 0;
    AUInstanceSize = get_CPD_memsize(CPD_AU);
#ifdef AIR_HEARING_PROTECTION_ENABLE
    #ifdef AIR_BTA_IC_PREMIUM_G2
    extern bool g_hearing_protection_enable;
    if(g_hearing_protection_enable){
        VOInstanceSize = get_CPD_memsize(CPD_RMS_VO);
    }else{
        VOInstanceSize = get_CPD_memsize(CPD_VO);
    }
    #else
    VOInstanceSize = get_CPD_memsize(CPD_RMS_VO);
    #endif
#else
    VOInstanceSize = get_CPD_memsize(CPD_VO);
#endif

    if (!CPD_MemCheck(au_vo_mode, vo_mode, au_mode)) {
        if (au_vo_mode == CPD_AU) {
            if (au_mode == MODE_AU1_CPD) {
                if (!(Au1CpdMemoryPtr && (Au1CpdMemoryPtr->MemoryCheck == CPD_RUNTIME_INVALID_MEMORY_CHECK_VALUE))) {
                    if ((DSP_DRC_AU_MEMSIZE + AUInstanceSize) != stream_function_get_working_buffer_length(para)) {
                        DSP_MW_LOG_E("DRC1 MEM SIZE is insufficient require:%d, allocated:%d", 2, DSP_DRC_AU_MEMSIZE + AUInstanceSize, stream_function_get_working_buffer_length(para));
                        assert(false);
                    }
                    Au1CpdMemoryPtr = (AU_CPD_INSTANCE_ptr)stream_function_get_working_buffer(para);
                }
                Au1CpdMemoryPtr->MemoryCheck = CPD_VALID_MEMORY_CHECK_VALUE;
                return FALSE;

            } else if (au_mode == MODE_AU2_CPD) {
                if (!(Au2CpdMemoryPtr && (Au2CpdMemoryPtr->MemoryCheck == CPD_RUNTIME_INVALID_MEMORY_CHECK_VALUE))) {
                    if ((DSP_DRC_AU_MEMSIZE + AUInstanceSize) != stream_function_get_working_buffer_length(para)) {
                        DSP_MW_LOG_E("DRC2 MEM SIZE is insufficient require:%d, allocated:%d", 2, DSP_DRC_AU_MEMSIZE + AUInstanceSize, stream_function_get_working_buffer_length(para));
                        assert(false);
                    }
                    Au2CpdMemoryPtr = (AU_CPD_INSTANCE_ptr)stream_function_get_working_buffer(para);
                }
                Au2CpdMemoryPtr->MemoryCheck = CPD_VALID_MEMORY_CHECK_VALUE;
                return FALSE;
            } else if (au_mode == MODE_AU3_CPD) {
                if (!(Au3CpdMemoryPtr && (Au3CpdMemoryPtr->MemoryCheck == CPD_RUNTIME_LINEIN_INVALID_MEMORY_CHECK_VALUE))) {
                    if ((DSP_DRC_AU_MEMSIZE + AUInstanceSize) != stream_function_get_working_buffer_length(para)) {
                        DSP_MW_LOG_E("DRC3 MEM SIZE is insufficient require:%d, allocated:%d", 2, DSP_DRC_AU_MEMSIZE + AUInstanceSize, stream_function_get_working_buffer_length(para));
                        assert(false);
                    }
                    Au3CpdMemoryPtr = (AU_CPD_INSTANCE_ptr)stream_function_get_working_buffer(para);
                }
                Au3CpdMemoryPtr->MemoryCheck = CPD3_VALID_MEMORY_CHECK_VALUE;
                return FALSE;
            } else if (au_mode == MODE_AU4_CPD) {
                if (!(AdeqCpdMemoryPtr && (AdeqCpdMemoryPtr->MemoryCheck == CPD_RUNTIME_ADEQ_INVALID_MEMORY_CHECK_VALUE))) {
                    AdeqCpdMemoryPtr = (AU_CPD_INSTANCE_ptr)DSPMEM_tmalloc(DAV_TASK_ID, CPD_AU_MEMSIZE + get_CPD_memsize(CPD_AU), stream_ptr);
                }
                AdeqCpdMemoryPtr->MemoryCheck = CPDAD_VALID_MEMORY_CHECK_VALUE;
                return FALSE;
#ifdef AIR_VP_PEQ_ENABLE
            } else if (au_mode == MODE_AU5_CPD) {
                if (!(AuVPCpdMemoryPtr && (AuVPCpdMemoryPtr->MemoryCheck == CPD_RUNTIME_INVALID_MEMORY_CHECK_VALUE))) {
                    if ((DSP_DRC_AU_MEMSIZE + AUInstanceSize) != stream_function_get_working_buffer_length(para)) {
                        DSP_MW_LOG_E("VP DRC MEM SIZE is insufficient require:%d, allocated:%d", 2, DSP_DRC_AU_MEMSIZE+AUInstanceSize, stream_function_get_working_buffer_length(para));
                        assert(false);
                    }
                    AuVPCpdMemoryPtr = (AU_CPD_INSTANCE_ptr)stream_function_get_working_buffer(para);
                }
                AuVPCpdMemoryPtr->MemoryCheck = CPD_VALID_MEMORY_CHECK_VALUE;
                return FALSE;
#ifdef AIR_ADAPTIVE_EQ_ENABLE
            } else if (au_mode == MODE_AU6_CPD) {
                if (!(AuVPAeqCpdMemoryPtr && (AuVPAeqCpdMemoryPtr->MemoryCheck == CPD_RUNTIME_INVALID_MEMORY_CHECK_VALUE))) {
                    AuVPAeqCpdMemoryPtr = (AU_CPD_INSTANCE_ptr)DSPMEM_tmalloc(DAV_TASK_ID, CPD_AU_MEMSIZE + get_CPD_memsize(CPD_AU), stream_ptr);
                }
                AuVPAeqCpdMemoryPtr->MemoryCheck = CPD_VALID_MEMORY_CHECK_VALUE;
                return FALSE;
#endif
#endif
            } else if (au_mode == MODE_AU_USB_CPD) {
                if (!(Au_Usb_CpdMemoryPtr && (Au_Usb_CpdMemoryPtr->MemoryCheck == CPD_RUNTIME_USB_INVALID_MEMORY_CHECK_VALUE))) {
                    if ((DSP_DRC_AU_MEMSIZE + AUInstanceSize) != stream_function_get_working_buffer_length(para)) {
                        DSP_MW_LOG_E("DRC3 MEM SIZE is insufficient require:%d, allocated:%d", 2, DSP_DRC_AU_MEMSIZE + AUInstanceSize, stream_function_get_working_buffer_length(para));
                        assert(false);
                    }
                    Au_Usb_CpdMemoryPtr = (AU_CPD_INSTANCE_ptr)stream_function_get_working_buffer(para);
                }
                Au_Usb_CpdMemoryPtr->MemoryCheck = CPDAD_VALID_USB_MEMORY_CHECK_VALUE;
                return FALSE;
            } else {
                DSP_MW_LOG_W("[CPD_MemInit] No this type of AU CPD : %d", 1, au_mode);
            }

        } else if (au_vo_mode == CPD_VO) {
            if (vo_mode <= MODE_VO_TX) {
                if (!(VoTxCpdMemoryPtr && (VoTxCpdMemoryPtr->MemoryCheck == CPD_VO_TX_VALID_MEMORY_CHECK_VALUE))) {
                    if ((DSP_DRC_VO_MEMSIZE + VOInstanceSize) > stream_function_get_working_buffer_length(para)) {
                        DSP_MW_LOG_E("VO_TX MEM SIZE is insufficient require:%d, allocated:%d", 2, DSP_DRC_VO_MEMSIZE + VOInstanceSize, stream_function_get_working_buffer_length(para));
                        assert(false);
                    }
                    VoTxCpdMemoryPtr = (VO_CPD_INSTANCE_ptr)stream_function_get_working_buffer(para);
                }
                VoTxCpdMemoryPtr->MemoryCheck = CPD_VO_TX_VALID_MEMORY_CHECK_VALUE;
                return FALSE;

            } else if ((vo_mode > MODE_VO_TX) && (vo_mode <= MODE_VO_RX)) {
                if (!(VoRxCpdMemoryPtr && (VoRxCpdMemoryPtr->MemoryCheck == CPD_VO_RX_VALID_MEMORY_CHECK_VALUE))) {
                    //DSP_MW_LOG_W("[CPD_MemInit] VO CPD : %d, normal MEM %d, RMS MEM %d, %d, %d", 5, vo_mode, get_CPD_memsize(1), get_CPD_memsize(3), DSP_DRC_VO_MEMSIZE, stream_function_get_working_buffer_length(para));
                    if ((DSP_DRC_VO_MEMSIZE + VOInstanceSize) > stream_function_get_working_buffer_length(para)) {
                        DSP_MW_LOG_E("VO_RX MEM SIZE is insufficient require:%d, allocated:%d", 2, DSP_DRC_VO_MEMSIZE + VOInstanceSize, stream_function_get_working_buffer_length(para));
                        assert(false);
                    }
                    VoRxCpdMemoryPtr = (VO_CPD_INSTANCE_ptr)stream_function_get_working_buffer(para);
                }
                VoRxCpdMemoryPtr->MemoryCheck = CPD_VO_RX_VALID_MEMORY_CHECK_VALUE;

                return FALSE;
            } else {
                DSP_MW_LOG_W("[CPD_MemInit] No this type of VO CPD : %d", 1, vo_mode);
            }

        } else {
            DSP_MW_LOG_W("[CPD_MemInit] No this type of CPD : %d", 1, au_vo_mode);
        }
    }

    return TRUE;
}



/**
 * CPD_MemCheck
 *
 * This function is used to check init memory space for CPD process
 *
 *
 * @para : Default parameter of callback function
 * @return : Check result
 */
bool CPD_MemCheck(eCPD_AU_VO_MODE au_vo_mode, eCPD_VO_MODE vo_mode, eCPD_AU_MODE au_mode)
{
    if (au_vo_mode == CPD_AU) {
        if (au_mode == MODE_AU1_CPD) {
            if (NULL != Au1CpdMemoryPtr) {
                if (CPD_VALID_MEMORY_CHECK_VALUE == Au1CpdMemoryPtr->MemoryCheck) {
                    return TRUE;
                }
            }
        } else if (au_mode == MODE_AU2_CPD) {
            if (NULL != Au2CpdMemoryPtr) {
                if (CPD_VALID_MEMORY_CHECK_VALUE == Au2CpdMemoryPtr->MemoryCheck) {
                    return TRUE;
                }
            }
        } else if (au_mode == MODE_AU3_CPD) {
            if (NULL != Au3CpdMemoryPtr) {
                if (CPD3_VALID_MEMORY_CHECK_VALUE == Au3CpdMemoryPtr->MemoryCheck) {
                    return TRUE;
                }
            }
        } else if (au_mode == MODE_AU4_CPD) {
            if (NULL != AdeqCpdMemoryPtr) {
                if (CPDAD_VALID_MEMORY_CHECK_VALUE == AdeqCpdMemoryPtr->MemoryCheck) {
                    return TRUE;
                }
            }
#ifdef AIR_VP_PEQ_ENABLE
        } else if (au_mode == MODE_AU5_CPD) {
            if (NULL != AuVPCpdMemoryPtr) {
                if (CPD_VALID_MEMORY_CHECK_VALUE == AuVPCpdMemoryPtr->MemoryCheck) {
                    return TRUE;
                }
            }
#ifdef AIR_ADAPTIVE_EQ_ENABLE
        } else if (au_mode == MODE_AU6_CPD) {
            if (NULL != AuVPAeqCpdMemoryPtr) {
                if (CPD_VALID_MEMORY_CHECK_VALUE == AuVPAeqCpdMemoryPtr->MemoryCheck) {
                    return TRUE;
                }
            }
#endif
#endif
        } else if (au_mode == MODE_AU_USB_CPD) {
            if (NULL != Au_Usb_CpdMemoryPtr) {
                if (CPDAD_VALID_USB_MEMORY_CHECK_VALUE == Au_Usb_CpdMemoryPtr->MemoryCheck) {
                    return TRUE;
                }
            }
        } else {
            DSP_MW_LOG_W("[CPD Mem Check] No this type of AU CPD : %d", 1, au_mode);
        }

    } else if (au_vo_mode == CPD_VO) {
        if (vo_mode <= MODE_VO_TX) {
            if (NULL != VoTxCpdMemoryPtr) {
                if (CPD_VO_TX_VALID_MEMORY_CHECK_VALUE == VoTxCpdMemoryPtr->MemoryCheck) {
                    return TRUE;
                }
            }
        } else if ((vo_mode > MODE_VO_TX) && (vo_mode <= MODE_VO_RX)) {
            if (NULL != VoRxCpdMemoryPtr) {
                if (CPD_VO_RX_VALID_MEMORY_CHECK_VALUE == VoRxCpdMemoryPtr->MemoryCheck) {
                    DSP_MW_LOG_W("[CPD Mem Check] VoRxCpdMemoryPtr valid", 0);
                    return TRUE;
                } else {
                    DSP_MW_LOG_W("[CPD Mem Check] VoRxCpdMemoryPtr in-valid", 0);
                }
            }
        } else {
            DSP_MW_LOG_W("[CPD Mem Check] No this type of VO CPD : %d", 1, vo_mode);
        }
    } else {
        DSP_MW_LOG_W("[CPD Mem Check] No this type of CPD : %d", 1, au_vo_mode);
    }

    return FALSE;
}


void stream_function_cpd_deinitialize(bool is_Rx_only)
{
    if(is_Rx_only){
        if (VoRxCpdMemoryPtr) {
            //VoRxCpdMemoryPtr->MemoryCheck = CPD_RUNTIME_INVALID_MEMORY_CHECK_VALUE;
            memset(VoRxCpdMemoryPtr, 0, sizeof(VO_CPD_INSTANCE_t));
            VoRxCpdMemoryPtr = NULL;
        }
    }else{
        if (Au1CpdMemoryPtr) {
            Au1CpdMemoryPtr->MemoryCheck = CPD_RUNTIME_INVALID_MEMORY_CHECK_VALUE;
        }
        if (Au2CpdMemoryPtr) {
            Au2CpdMemoryPtr->MemoryCheck = CPD_RUNTIME_INVALID_MEMORY_CHECK_VALUE;
        }
        if (Au3CpdMemoryPtr) {
            Au3CpdMemoryPtr->MemoryCheck = CPD_RUNTIME_LINEIN_INVALID_MEMORY_CHECK_VALUE;
        }
        if (AdeqCpdMemoryPtr) {
            AdeqCpdMemoryPtr->MemoryCheck = CPD_RUNTIME_ADEQ_INVALID_MEMORY_CHECK_VALUE;
        }
    #ifdef AIR_VP_PEQ_ENABLE
        if (AuVPCpdMemoryPtr) {
            AuVPCpdMemoryPtr->MemoryCheck = CPD_RUNTIME_INVALID_MEMORY_CHECK_VALUE;
        }
    #ifdef AIR_ADAPTIVE_EQ_ENABLE
        if (AuVPAeqCpdMemoryPtr) {
            AuVPAeqCpdMemoryPtr->MemoryCheck = CPD_RUNTIME_ADEQ_INVALID_MEMORY_CHECK_VALUE;
        }
    #endif
    #endif
        if (VoRxCpdMemoryPtr) {
            VoRxCpdMemoryPtr->MemoryCheck = CPD_RUNTIME_INVALID_MEMORY_CHECK_VALUE;
        }
        if (VoTxCpdMemoryPtr) {
            VoTxCpdMemoryPtr->MemoryCheck = CPD_RUNTIME_INVALID_MEMORY_CHECK_VALUE;
        }
        if (Au_Usb_CpdMemoryPtr) {
            Au_Usb_CpdMemoryPtr->MemoryCheck = CPD_RUNTIME_USB_INVALID_MEMORY_CHECK_VALUE;
        }
    }
}

/**
 * stream_function_drc_voice_tx_swb_initialize
 *
 * This function is used to init memory space for voice TX CPD
 *
 *
 * @para : Default parameter of callback function
 *
 */
bool stream_function_drc_voice_tx_swb_initialize(void *para)
{
    Voice_CPD_EC_Para_Init();
    Voice_CPD_Mode_Init(para, MODE_UWB_TX_CPD);

    return FALSE;
}

/**
 * stream_function_drc_voice_tx_wb_initialize
 *
 * This function is used to init memory space for voice TX CPD
 *
 *
 * @para : Default parameter of callback function
 *
 */
bool stream_function_drc_voice_tx_wb_initialize(void *para)
{
    Voice_CPD_EC_Para_Init();
    Voice_CPD_Mode_Init(para, MODE_WB_TX_CPD);

    return FALSE;
}


/**
 * stream_function_drc_voice_tx_nb_initialize
 *
 * This function is used to init memory space for voice TX CPD
 *
 *
 * @para : Default parameter of callback function
 *
 */
bool stream_function_drc_voice_tx_nb_initialize(void *para)
{
    Voice_CPD_EC_Para_Init();
    Voice_CPD_Mode_Init(para, MODE_NB_TX_CPD);

    return FALSE;
}


/**
 * stream_function_drc_voice_rx_swb_initialize
 *
 * This function is used to init memory space for voice RX CPD
 *
 *
 * @para : Default parameter of callback function
 *
 */
bool stream_function_drc_voice_rx_swb_initialize(void *para)
{
    Voice_CPD_EC_Para_Init();
    Voice_CPD_Mode_Init(para, MODE_UWB_RX_CPD);

    return FALSE;
}


/**
 * stream_function_drc_voice_rx_wb_initialize
 *
 * This function is used to init memory space for voice RX CPD
 *
 *
 * @para : Default parameter of callback function
 *
 */
bool stream_function_drc_voice_rx_wb_initialize(void *para)
{
    Voice_CPD_EC_Para_Init();
    switch (gDspAlgParameter.EscoMode.Rx) {
        case VOICE_NB:
            DSP_MW_LOG_D("RX NB CPD init start (cvsd)", 0);
            Voice_CPD_Mode_Init(para, MODE_NB_RX_CPD);
            break;
        case VOICE_WB:
            DSP_MW_LOG_D("RX WB CPD init start (mSBC)", 0);
            Voice_CPD_Mode_Init(para, MODE_WB_RX_CPD);
            break;
        default:
            Voice_CPD_Mode_Init(para, MODE_WB_RX_CPD);
            break;
    }

    return FALSE;
}


/**
 * stream_function_drc_voice_rx_nb_initialize
 *
 * This function is used to init memory space for voice RX CPD
 *
 *
 * @para : Default parameter of callback function
 *
 */
bool stream_function_drc_voice_rx_nb_initialize(void *para)
{
    Voice_CPD_EC_Para_Init();
    Voice_CPD_Mode_Init(para, MODE_NB_RX_CPD);

    return FALSE;
}


/**
 * Voice_VP_CPD_Init
 *
 * This function is used to init memory space for VP CPD
 *
 *
 * @para : Default parameter of callback function
 *
 */
bool Voice_VP_CPD_Init(void *para)
{
    Voice_CPD_Mode_Init(para, MODE_VP_CPD);

    return FALSE;
}

#if defined(AIR_HEARING_PROTECTION_ENABLE) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
#define RMS_DRC_DUMP_SIZE 512
static S16 g_rms_monitor_rx_dump_buf[RMS_DRC_DUMP_SIZE];
#endif

#ifdef AIR_HEARING_PROTECTION_ENABLE
static S32 g_rms_mode = CPD_HSE_MODE_NORMAL;
void voice_leq_send_update(int16_t pre_leq, int16_t post_leq)
{
    hal_ccni_message_t msg;
    if (abs(pre_leq-post_leq)>LEQ_CHECK_DIFF_VALUE) {
#if AUDIO_CPD_DEBUG_LOG_ENABLE
        DSP_MW_LOG_I("[DSP][CPD][DSP send]former:%d, later:%d", 2, pre_leq, post_leq);
#endif
        memset((void *)&msg, 0, sizeof(hal_ccni_message_t));
        msg.ccni_message[0] = MSG_DSP2MCU_LEQ_PARA_SEND << 16;
        msg.ccni_message[1] = post_leq;
        aud_msg_tx_handler(msg, 0, false);
        g_LEQ_gain_prev = g_LEQ_gain_post;
    }

}

/* Check whether need to update LEQ from other side */
void voice_leq_receive_update(int16_t leq)
{
    gLEQUpdate = true;
    gLEQ_gain = leq;
#if AUDIO_CPD_DEBUG_LOG_ENABLE
    DSP_MW_LOG_I("[DSP][CPD][DSP receive] leq: %d, gLEQUpdate: %d", 2, leq, gLEQUpdate);
#endif
}
#endif

/**
 * stream_function_drc_voice_tx_process
 *
 * voice TX CPD main process
 *
 *
 * @para : Default parameter of callback function
 *
 */
bool stream_function_drc_voice_tx_process(void *para)
{
    S16 *Buf = (S16 *)stream_function_get_1st_inout_buffer(para);
    U16 FrameSize = stream_function_get_output_size(para);
    S16 Gain;
    void *p_scratch = &VoTxCpdMemoryPtr->ScratchMemory[0];

    Gain = 0x0800; /* Voice TX don't care about this parameter */

#ifdef AIR_AUDIO_DUMP_ENABLE
    LOG_AUDIO_DUMP((U8 *)Buf, (U32)FrameSize, VOICE_TX_CPD_IN);
#endif

    compander_VO_proc(p_scratch,
                      Buf,
#if defined(AIR_HEARING_PROTECTION_ENABLE) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
                      NULL,
#endif
                      FrameSize / sizeof(S16),
                      Gain,
                      &gDspAlgParameter.AecNr);

#ifdef AIR_AUDIO_DUMP_ENABLE
    LOG_AUDIO_DUMP((U8 *)Buf, (U32)FrameSize, VOICE_TX_CPD_OUT);
    LOG_AUDIO_DUMP((U8 *)Buf, (U32)FrameSize, VOICE_TX_OUT);
#endif

    return FALSE;
}


/**
 * Voice_Prompt_CPD_Process
 *
 * voice prompt CPD wrapper function
 *
 *
 * @para : Default parameter of callback function
 *
 */
bool Voice_Prompt_CPD_Process(void *para)
{
    return Voice_CPD_Process(para);
}


/**
 * stream_function_drc_voice_rx_process
 *
 * voice RX CPD wrapper function
 *
 *
 * @para : Default parameter of callback function
 *
 */
bool stream_function_drc_voice_rx_process(void *para)
{
    return Voice_CPD_Process(para);
}


/**
 * Voice_RX_NB_CPD_Process
 *
 * voice RX NB CPD wrapper function
 *
 *
 * @para : Default parameter of callback function
 *
 */
bool Voice_RX_NB_CPD_Process(void *para)
{
    return Voice_CPD_Process(para);
}


/**
 * Voice_CPD_Process
 *
 * voice CPD main process
 *
 *
 * @para : Default parameter of callback function
 *
 */
bool Voice_CPD_Process(void *para)
{
    S16 *Buf = (S16 *)stream_function_get_1st_inout_buffer(para);
    U16 FrameSize = stream_function_get_output_size(para);
    S16 Gain;
    void *p_scratch = &VoRxCpdMemoryPtr->ScratchMemory[0];

    if(g_cpd_reinit_mode == CPD_REINIT_MODE_SINGLE){
        switch (gDspAlgParameter.EscoMode.Rx) {
            case VOICE_NB:
                DSP_MW_LOG_D("RX NB CPD init start (cvsd)", 0);
                Voice_CPD_Mode_Init(para, MODE_NB_RX_CPD);
                break;
            case VOICE_WB:
                DSP_MW_LOG_D("RX WB CPD init start (mSBC)", 0);
                Voice_CPD_Mode_Init(para, MODE_WB_RX_CPD);
                break;
            case VOICE_SWB:
                DSP_MW_LOG_D("RX SWB CPD init start", 0);
                Voice_CPD_Mode_Init(para, MODE_UWB_RX_CPD);
                break;
            default:
                Voice_CPD_Mode_Init(para, MODE_WB_RX_CPD);
                break;
        }
        g_cpd_reinit_mode = CPD_REINIT_MODE_NONE;
    }

    Gain = DSP_GC_ConvertQ11Form(CPD_ZERO_GAIN_INDEX + VoRxCpdMemoryPtr->positive_gain);
#ifdef AIR_VOLUME_CONTROL_ON_DRC_ENABLE
    Gain = afe_calculate_digital_gain_index(afe_volume_digital_get_gain_index(AFE_HW_DIGITAL_GAIN1), 0x800);//DSPGAIN_Q11: (0x800=0dB)
#endif


    /*Voice_CPD_EC_Para_Init();
    Gain = 0x0800;*/

#ifdef AIR_AUDIO_DUMP_ENABLE
    LOG_AUDIO_DUMP((U8 *)Buf, (U32)FrameSize, VOICE_RX_CPD_IN);
#endif

    compander_VO_proc(p_scratch,
                      Buf,
#if defined(AIR_HEARING_PROTECTION_ENABLE) || defined(AIR_BTA_IC_STEREO_HIGH_G3)
                      g_rms_monitor_rx_dump_buf,
#endif
                      FrameSize / sizeof(S16),
                      Gain,
                      &gDspAlgParameter.AecNr);

#ifdef AIR_HEARING_PROTECTION_ENABLE
    LOG_AUDIO_DUMP((U8 *)g_rms_monitor_rx_dump_buf, (U32)FrameSize, AUDIO_INS_OUT_L);

    if(g_rms_mode == CPD_HSE_MODE_2){
        if(g_LEQ_cnt == LEQ_CHECK_FRAME_INTERVAL){
            g_LEQ_gain_post = get_LEQ_gain(p_scratch);
            g_LEQ_cnt = 0;
#if AUDIO_CPD_DEBUG_LOG_ENABLE
            DSP_MW_LOG_I("[DSP][CPD]g_LEQ_gain_prev:%d, g_LEQ_gain_post:%d", 2, g_LEQ_gain_prev, g_LEQ_gain_post);
#endif
            voice_leq_send_update(g_LEQ_gain_prev,g_LEQ_gain_post);
        }else {
            g_LEQ_cnt ++;
        }
    }else if(g_rms_mode == CPD_HSE_MODE_1){
        if (gLEQUpdate == true) {
            store_LEQ_gain(p_scratch, gLEQ_gain);
            gLEQUpdate = false;
#if AUDIO_CPD_DEBUG_LOG_ENABLE
            DSP_MW_LOG_I("[DSP][CPD] store_LEQ_gain %d, gLEQUpdate %d",2,gLEQ_gain,gLEQUpdate);
#endif
        }
    }
#endif

#ifdef AIR_AUDIO_DUMP_ENABLE
    LOG_AUDIO_DUMP((U8 *)Buf, (U32)FrameSize, VOICE_RX_CPD_OUT);
    LOG_AUDIO_DUMP((U8 *)Buf, (U32)FrameSize, VOICE_RX_OUT);
#endif

    U16 i;
    for (i = 1 ; i < stream_function_get_device_channel_number(para) ; i++) {
        memcpy(stream_function_get_inout_buffer(para, i + 1),
               stream_function_get_inout_buffer(para, 1),
               stream_function_get_output_size(para));
    }

    return FALSE;
}



/**
 * Voice_CPD_Mode_Init
 *
 * This function is used to init voice CPD with mode settings
 *
 *
 * @para : Default parameter of callback function
 *
 */
EXTERN CPD_VO_NVKEY_STATE VO_NvKey;
bool Voice_CPD_Mode_Init(void *para, eCPD_VO_MODE vo_mode)
{
    U16 KeyId;
    void *p_nvkey;
    void *p_scratch;
#ifdef AIR_HEARING_PROTECTION_ENABLE
    gLEQUpdate = false;
    DSP_MW_LOG_I("[CPD] initialize gLEQUpdate %d", 1,gLEQUpdate);
    g_rms_mode = g_cpd_hse_mode;
#endif
    if (CPD_MemInit(para, CPD_VO, vo_mode, NULL)) {
        return FALSE;
    }
    if ((vo_mode == MODE_UWB_RX_CPD) || (vo_mode == MODE_WB_RX_CPD) || (vo_mode == MODE_NB_RX_CPD)) {
        POSITIVE_GAIN_NVKEY_t positive_gain_nvkey;
        nvkey_read_full_key(NVKEY_DSP_PARA_POSITIVE_GAIN, &positive_gain_nvkey, sizeof(POSITIVE_GAIN_NVKEY_t));
        if(vo_mode == MODE_UWB_RX_CPD){
            VoRxCpdMemoryPtr->positive_gain = positive_gain_nvkey.vo_rx_swb_positive_gain;
        }else{
            VoRxCpdMemoryPtr->positive_gain = (vo_mode == MODE_WB_RX_CPD) ? positive_gain_nvkey.vo_rx_wb_positive_gain : positive_gain_nvkey.vo_rx_nb_positive_gain;
            }
        DSP_MW_LOG_I("RX CPD positive_gain index:%d, mode:%d", 2, VoRxCpdMemoryPtr->positive_gain, vo_mode);
    }

    switch (vo_mode) {
        case MODE_NB_TX_CPD:
            KeyId = NVKEY_DSP_PARA_NB_TX_VO_CPD;
            p_nvkey = &VoTxCpdMemoryPtr->NvKey;
            p_scratch = &VoTxCpdMemoryPtr->ScratchMemory[0];
            break;

        case MODE_WB_TX_CPD:
            KeyId = NVKEY_DSP_PARA_WB_TX_VO_CPD;
            p_nvkey = &VoTxCpdMemoryPtr->NvKey;
            p_scratch = &VoTxCpdMemoryPtr->ScratchMemory[0];
            break;

        case MODE_UWB_TX_CPD:
            KeyId = NVKEY_DSP_PARA_SWB_TX_VO_CPD;
            p_nvkey = &VoTxCpdMemoryPtr->NvKey;
            p_scratch = &VoTxCpdMemoryPtr->ScratchMemory[0];
            break;

        case MODE_NB_RX_CPD:
            KeyId = NVKEY_DSP_PARA_NB_RX_VO_CPD;
            vo_mode = MODE_WB_RX_CPD;  // use NB NVKEY but WB CPD Prc
            p_nvkey = &VoRxCpdMemoryPtr->NvKey;
            p_scratch = &VoRxCpdMemoryPtr->ScratchMemory[0];
            break;

        case MODE_WB_RX_CPD:
            KeyId = NVKEY_DSP_PARA_WB_RX_VO_CPD;
            p_nvkey = &VoRxCpdMemoryPtr->NvKey;
            p_scratch = &VoRxCpdMemoryPtr->ScratchMemory[0];
            break;

        case MODE_UWB_RX_CPD:
            KeyId = NVKEY_DSP_PARA_SWB_RX_VO_CPD;
            p_nvkey = &VoRxCpdMemoryPtr->NvKey;
            p_scratch = &VoRxCpdMemoryPtr->ScratchMemory[0];
            break;

        case MODE_VP_CPD:
            KeyId = NVKEY_DSP_PARA_VP_CPD;
            p_nvkey = &VoRxCpdMemoryPtr->NvKey;
            p_scratch = &VoRxCpdMemoryPtr->ScratchMemory[0];
            break;

        default:
            KeyId = NVKEY_DSP_PARA_WB_RX_VO_CPD;
            p_nvkey = &VoRxCpdMemoryPtr->NvKey;
            p_scratch = &VoRxCpdMemoryPtr->ScratchMemory[0];
            break;
    }



#if (CPD_TEST_DEFAULT_PARA)
    p_nvkey = &VO_NvKey;
#else
    nvkey_read_full_key(KeyId, p_nvkey, sizeof(CPD_VO_NVKEY_STATE));
#endif

#ifdef AIR_HEARING_PROTECTION_ENABLE
    if (((vo_mode == MODE_WB_RX_CPD) || (vo_mode == MODE_UWB_RX_CPD)) && (g_rms_mode != CPD_HSE_MODE_NORMAL)) {
        int32_t i, coef_num, coef_size, sample_index;
        int16_t *p_peq_nvkey, *p_curr_ptr = NULL;
        /* 2byte(sample rate number) + 2byte(version) + 2byte(1st sample rate index) + Nbyte(Payload) + .... */
        p_curr_ptr = VoRxCpdMemoryPtr->voice_peq;
        nvkey_read_full_key(NVKEY_DSP_PARA_VOICE_PEQ_COEF_01, p_curr_ptr, g_cpd_nv_length);
        coef_num = *(uint16_t *)p_curr_ptr;
        coef_size = ((g_cpd_nv_length) - 4) / (coef_num*2);
        p_curr_ptr += 2;
        p_peq_nvkey = NULL;
        for (i = 0; i < coef_num; i++) {
            sample_index = *(uint16_t *)p_curr_ptr;
            DSP_MW_LOG_I("DRC init: %d, %d, %d, %x, %x, %x, %x, %x, %x", 9, sample_index, coef_size, vo_mode, p_curr_ptr[0], p_curr_ptr[1], p_curr_ptr[2], p_curr_ptr[3], p_curr_ptr[4], p_curr_ptr[5]);
            if (((sample_index == 0) && (vo_mode == MODE_UWB_RX_CPD)) ||
                ((sample_index == 3) && (vo_mode == MODE_WB_RX_CPD))) {
                p_peq_nvkey = p_curr_ptr + 2;
                break;
            }
            p_curr_ptr += coef_size;
        }
        if (p_peq_nvkey == NULL) {
            AUDIO_ASSERT(0);
        }
DSP_MW_LOG_I("DRC init: p_scratch 0x%x,p_peq_nvkey %d, vo mode %d, RMS mode %d", 4, p_scratch,p_peq_nvkey,vo_mode, g_rms_mode);
        compander_VO_init(p_scratch, p_nvkey, p_peq_nvkey, vo_mode, g_rms_mode); /* Set to RMS mode */
        LOG_AUDIO_DUMP((U8 *)p_nvkey, sizeof(CPD_VO_NVKEY_STATE), AUDIO_SOUNDBAR_INPUT);
        LOG_AUDIO_DUMP((U8 *)p_peq_nvkey, (coef_size - 2)*2, AUDIO_SOUNDBAR_TX);
        //DSP_MW_LOG_I("DRC init: p_scratch 0x%x,vo mode %d, RMS mode %d", 3, p_scratch,vo_mode, g_rms_mode);
    } else {
        compander_VO_init(p_scratch, p_nvkey, NULL, vo_mode, 0); /* Set to Normal mode */
    }
#elif defined(AIR_BTA_IC_STEREO_HIGH_G3)
    compander_VO_init(p_scratch, p_nvkey, NULL, vo_mode, 0); /* Set to Normal mode */
#else
    compander_VO_Set_LowLatency(p_scratch);
    compander_VO_init(p_scratch, p_nvkey, vo_mode);
#endif
    DSP_MW_LOG_I("CPD VO init done, version: 0x%08x", 1, SVN_version());

    return TRUE;
}


/**
 * Voice_CPD_EC_Para_Init
 *
 * This function is used to init parameters for voice CPD
 *
 *
 * @para : Default parameter of callback function
 *
 */
VOID Voice_CPD_EC_Para_Init(VOID)
{
    gDspAlgParameter.AecNr.TX_GAIN = 0x0800;
    gDspAlgParameter.AecNr.CPD_VAD = 0;
    gDspAlgParameter.AecNr.avc_vol = 0x2000;
}

/**
 * Audio_CPD_Enable
 *
 * This function is used to enable/disable audio compander.
 *
 *
 * @enable : enable or disable
 * @phase_id : for DRC or DRC2
 */
void Audio_CPD_Enable(U8 enable, U8 phase_id, U8 force_disable, U8 type, U16 peq_nvkey_id)
{
    if ((force_disable == 0) || (force_disable == 1)) {
        if (phase_id == 0) {
            if (type == 0) {
                aud_cpd_ctrl.force_disable = force_disable;
            } else if (type == 1) {
                aud_cpd3_ctrl.force_disable = force_disable;
#ifdef AIR_VP_PEQ_ENABLE
            } else if (type == 4) {
                aud_vp_cpd_ctrl.force_disable = force_disable;
#ifdef AIR_ADAPTIVE_EQ_ENABLE
            } else if (type == 5) {
                aud_vp_aeq_cpd_ctrl.force_disable = force_disable;
#endif
#endif
            } else if (type == 8) {
                aud_usb_peq_cpd_ctrl.force_disable = force_disable;
            } else {
                // aud_adaptive_eq_drc_ctrl.force_disable = force_disable;
            }
        } else if (phase_id == 1) {
            aud_cpd2_ctrl.force_disable = force_disable;
            aud_cpd_ctrl.peq2_nvkey_id = peq_nvkey_id;
#ifdef AIR_VP_PEQ_ENABLE
            aud_vp_cpd_ctrl.peq2_nvkey_id = peq_nvkey_id;
#endif
        }
    }
    if (phase_id == 0) {
        if (type == 0) {
            aud_cpd_ctrl.enable = enable;
        } else if (type == 1) {
            aud_cpd3_ctrl.enable = enable;
#ifdef AIR_VP_PEQ_ENABLE
        } else if (type == 4) {
            aud_vp_cpd_ctrl.enable = enable;
#ifdef AIR_ADAPTIVE_EQ_ENABLE
        } else if (type == 5) {
            aud_vp_aeq_cpd_ctrl.enable = enable;
#endif
#endif
        } else if (type == 8) {
            aud_usb_peq_cpd_ctrl.enable = enable;
        } else {
            // aud_adaptive_eq_drc_ctrl.enable = enable;
        }
    } else if (phase_id == 1) {
        aud_cpd2_ctrl.enable = enable;
        aud_cpd_ctrl.peq2_nvkey_id = peq_nvkey_id;
#ifdef AIR_VP_PEQ_ENABLE
        aud_vp_cpd_ctrl.peq2_nvkey_id = peq_nvkey_id;
#endif
    } else {
        DSP_MW_LOG_E("[Audio_CPD_Enable] Un-supported phase id: %d\n", 1, phase_id);
        return;
    }
}

static S32 Audio_CPD_Get_Fs(U8 SampleRate)
{
    //band_sw = 0(48K), 1(44.1K), 2(96K), 3(88.2K)
    S32 band_sw;
    switch (SampleRate) {
        case FS_RATE_48K:
            band_sw = 0;
            break;
        case FS_RATE_44_1K:
            band_sw = 1;
            break;
        case FS_RATE_96K:
            band_sw = 2;
            break;
        case FS_RATE_88_2K:
            band_sw = 3;
            break;
        default:
            band_sw = 0;
            break;
    }
    return band_sw;
}

/**
 * Audio_CPD_Algorithm_Init
 *
 * This function is used to init memory space for audio CPD
 *
 *
 * @para : Default parameter of callback function
 * @recovery_gain : shift bits to recover gain from feature ahead.
 *
 */
static bool Audio_CPD_Algorithm_Init(void *para, AU_CPD_CTRL_t *ctrl, U16 recovery_gain, AU_CPD_INSTANCE_ptr au_cpd_instance, DSP_ALG_NVKEY_e nvkey_id)
{
    void *p_scratch = &au_cpd_instance->ScratchMemory[0];
    void *p_nvkey = &au_cpd_instance->NvKey;

    U8 SampleRate = stream_function_get_samplingrate(para);

    nvkey_read_full_key(nvkey_id, p_nvkey, sizeof(CPD_AU_NVKEY_STATE));

    ctrl->sample_rate = SampleRate;
    ctrl->enabled = ctrl->enable;

    ctrl->sample_base = CPD_AU_FRAME_SIZE;
#ifdef AIR_BT_CODEC_BLE_ENABLED
    DSP_ENTRY_PARA *const entry_para = (DSP_ENTRY_PARA_PTR)para;
    const N9BLE_PARAMETER ble_para = DSP_STREAMING_GET_FROM_PRAR(entry_para)->source->param.n9ble;
    if (ble_para.context_type == BLE_CONTENT_TYPE_ULL_BLE) {
        ctrl->sample_base = CPD_AU_FRAME_SIZE_8;
    } else if ((entry_para)->number.field.source_type == SOURCE_TYPE_N9BLE) {
        ctrl->sample_base = CPD_AU_FRAME_SIZE_20;
    }
#endif
#ifdef MTK_CELT_DEC_ENABLE
    if (stream_function_get_decoder_type(para) == CODEC_DECODER_CELT_HD) {
        ctrl->sample_base = CPD_AU_FRAME_SIZE_20;
        DSP_MW_LOG_I("DRC init with frame size 20", 0);
    }
#endif

#ifdef AIR_WIRELESS_MIC_TX_ENABLE
    ctrl->sample_base = CPD_AU_FRAME_SIZE_8;
#endif
#ifdef AIR_WIRED_AUDIO_ENABLE
    audio_scenario_type_t wired_audio_type;
    wired_audio_type = DSP_STREAMING_GET_FROM_PRAR(para)->source->scenario_type;
    if ((wired_audio_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_0) ||
        (wired_audio_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_USB_IN_1) ||
        (wired_audio_type == AUDIO_SCENARIO_TYPE_WIRED_AUDIO_LINE_IN)) {
        ctrl->sample_base = CPD_AU_FRAME_SIZE_8;
    }
#endif
#ifdef AIR_ULL_AUDIO_V2_DONGLE_ENABLE
    ctrl->sample_base = CPD_AU_FRAME_SIZE_8;
#endif
#ifdef MTK_BT_A2DP_VENDOR_2_ENABLE
    if (stream_function_get_decoder_type(para) == CODEC_DECODER_VENDOR_2) {
        ctrl->sample_base = CPD_AU_FRAME_SIZE_8;
    }
#endif
#ifdef AIR_BT_A2DP_LC3PLUS_ENABLE
    if (stream_function_get_decoder_type(para) == CODEC_DECODER_LC3PLUS) {
        ctrl->sample_base = CPD_AU_FRAME_SIZE_8;
    }
#endif

    // set the process unit (samples)
    switch (ctrl->sample_base) {
        case CPD_AU_FRAME_SIZE_8:  // fallthrough
            compander_AU_SetFrame8_init(p_scratch);
            break;
        case CPD_AU_FRAME_SIZE_20: {
            compander_AU_SetFrame120_init(p_scratch);
            break;
        }
        default: {
            // pass
        }
    }
    //CPD_AU_NVKEY_ptr->CPD_AU_MODE = 0x1; //dual-band
    au_cpd_instance->NvKey.band_sw = Audio_CPD_Get_Fs(SampleRate);
    if ((au_cpd_instance->NvKey.CPD_AU_MODE & 0x1) && (au_cpd_instance->NvKey.band_sw > 1)) {
        DSP_MW_LOG_E("AU DRC doesn't support dual-band mode for 88.2/96kHz, so force to single-band mode", 0);
        au_cpd_instance->NvKey.CPD_AU_MODE &= (~0x1);
    }
    compander_AU_init(p_scratch, p_nvkey, recovery_gain, au_cpd_instance->NvKey.band_sw);

    DSP_MW_LOG_I("DRC init ver: %d, fs:%dkH, enable:%d, phase_id:%d, positive_gain_index:%d, band_sw:%d, frame_size:%d", 7, SVN_version(), SampleRate, ctrl->enable, ctrl->phase_id, au_cpd_instance->positive_gain, au_cpd_instance->NvKey.band_sw, ctrl->sample_base);

    return FALSE;
}

/**
 * stream_function_drc_audio_initialize
 *
 * This function is used to init memory space for audio CPD with PEQ
 *
 *
 * @para : Default parameter of callback function
 *
 */
bool stream_function_drc_audio_initialize(void *para)
{
    if (CPD_MemInit(para, CPD_AU, NULL, MODE_AU1_CPD)) {
        return TRUE;
    }

    POSITIVE_GAIN_NVKEY_t positive_gain_nvkey;
    memset(&positive_gain_nvkey, 0, sizeof(POSITIVE_GAIN_NVKEY_t));
    nvkey_read_full_key(NVKEY_DSP_PARA_POSITIVE_GAIN, &positive_gain_nvkey, sizeof(POSITIVE_GAIN_NVKEY_t));
    Au1CpdMemoryPtr->positive_gain = (S32)positive_gain_nvkey.au_positive_gain;

    aud_cpd_ctrl.phase_id = 0;
    return Audio_CPD_Algorithm_Init(para, &aud_cpd_ctrl, CPD_AU_RECOVERY_GAIN, Au1CpdMemoryPtr, NVKEY_DSP_PARA_A2DP_AU_CPD);
}

bool stream_function_drc_audio2_initialize(void *para)
{
    if (CPD_MemInit(para, CPD_AU, NULL, MODE_AU2_CPD)) {
        return TRUE;
    }

    POSITIVE_GAIN_NVKEY_t positive_gain_nvkey;
    nvkey_read_full_key(NVKEY_DSP_PARA_POSITIVE_GAIN, &positive_gain_nvkey, sizeof(POSITIVE_GAIN_NVKEY_t));
    Au2CpdMemoryPtr->positive_gain = (S32)positive_gain_nvkey.au_positive_gain;

    aud_cpd2_ctrl.phase_id = 1;
    return Audio_CPD_Algorithm_Init(para, &aud_cpd2_ctrl, CPD_AU_RECOVERY_GAIN, Au2CpdMemoryPtr, NVKEY_DSP_PARA_A2DP_AU_CPD);
}

bool stream_function_drc_audio3_initialize(void *para)
{
    if (CPD_MemInit(para, CPD_AU, NULL, MODE_AU3_CPD)) {
        return TRUE;
    }

    POSITIVE_GAIN_NVKEY_t positive_gain_nvkey;
    nvkey_read_full_key(NVKEY_DSP_PARA_POSITIVE_GAIN, &positive_gain_nvkey, sizeof(POSITIVE_GAIN_NVKEY_t));
    Au3CpdMemoryPtr->positive_gain = (S32)positive_gain_nvkey.au_positive_gain;

    aud_cpd3_ctrl.phase_id = 0;
    return Audio_CPD_Algorithm_Init(para, &aud_cpd3_ctrl, CPD_AU_RECOVERY_GAIN, Au3CpdMemoryPtr, NVKEY_DSP_PARA_LINE_AU_CPD);
}

bool stream_function_adaptive_eq_drc_initialize(void *para)
{
    if (CPD_MemInit(para, CPD_AU, NULL, MODE_AU4_CPD)) {
        return TRUE;
    }

    POSITIVE_GAIN_NVKEY_t positive_gain_nvkey;
    memset(&positive_gain_nvkey, 0, sizeof(POSITIVE_GAIN_NVKEY_t));
    AdeqCpdMemoryPtr->positive_gain = 0;

    aud_adaptive_eq_drc_ctrl.phase_id = 0;
    return Audio_CPD_Algorithm_Init(para, &aud_adaptive_eq_drc_ctrl, CPD3_AU_RECOVERY_GAIN, AdeqCpdMemoryPtr, NVKEY_DSP_PARA_A2DP_AU_CPD);
}

#ifdef AIR_VP_PEQ_ENABLE
bool stream_function_drc_vp_initialize(void *para)
{
    if (CPD_MemInit(para, CPD_AU, NULL, MODE_AU5_CPD)) {
        return TRUE;
    }

    POSITIVE_GAIN_NVKEY_t positive_gain_nvkey;
    nvkey_read_full_key(NVKEY_DSP_PARA_POSITIVE_GAIN, &positive_gain_nvkey, sizeof(POSITIVE_GAIN_NVKEY_t));
    AuVPCpdMemoryPtr->positive_gain = (S32)positive_gain_nvkey.au_positive_gain;

    aud_vp_cpd_ctrl.phase_id = 0;
    if (aud_cpd_ctrl.peq2_nvkey_id == 0) {
        return Audio_CPD_Algorithm_Init(para, &aud_vp_cpd_ctrl, CPD3_AU_RECOVERY_GAIN, AuVPCpdMemoryPtr, NVKEY_DSP_PARA_A2DP_AU_CPD);
    }
    return Audio_CPD_Algorithm_Init(para, &aud_vp_cpd_ctrl, CPD_AU_RECOVERY_GAIN, AuVPCpdMemoryPtr, NVKEY_DSP_PARA_A2DP_AU_CPD);
}

#ifdef AIR_ADAPTIVE_EQ_ENABLE
bool stream_function_drc_vp_aeq_initialize(void *para)
{
    if (CPD_MemInit(para, CPD_AU, NULL, MODE_AU6_CPD)) {
        return TRUE;
    }

    POSITIVE_GAIN_NVKEY_t positive_gain_nvkey;
    memset(&positive_gain_nvkey, 0, sizeof(POSITIVE_GAIN_NVKEY_t));
    AuVPAeqCpdMemoryPtr->positive_gain = 0;

    aud_vp_aeq_cpd_ctrl.phase_id = 0;
    return Audio_CPD_Algorithm_Init(para, &aud_vp_aeq_cpd_ctrl, CPD3_AU_RECOVERY_GAIN, AuVPAeqCpdMemoryPtr, NVKEY_DSP_PARA_A2DP_AU_CPD);
}
#endif
#endif

bool stream_function_wired_usb_drc_initialize(void *para)
{
    if (CPD_MemInit(para, CPD_AU, NULL, MODE_AU_USB_CPD)) {
        return TRUE;
    }
    POSITIVE_GAIN_NVKEY_t positive_gain_nvkey;
    memset(&positive_gain_nvkey, 0, sizeof(POSITIVE_GAIN_NVKEY_t));
    Au_Usb_CpdMemoryPtr->positive_gain = 0;
    aud_usb_peq_cpd_ctrl.phase_id = 0;
    return Audio_CPD_Algorithm_Init(para, &aud_usb_peq_cpd_ctrl, CPD3_AU_RECOVERY_GAIN, Au_Usb_CpdMemoryPtr, NVKEY_DSP_PARA_LINE_AU_CPD);
}
/**
 * Audio_PureCPD_Init
 *
 * This function is used to init memory space for pure audio CPD without PEQ
 *
 *
 * @para : Default parameter of callback function
 *
 */
bool Audio_PureCPD_Init(void *para)
{
    return Audio_CPD_Algorithm_Init(para, &aud_cpd_ctrl, 0, Au1CpdMemoryPtr, NVKEY_DSP_PARA_A2DP_AU_CPD);
}

void CPD_Reinit(void *p_scratch, void *p_nvkey, S32 band_sw, U8 audio_path)
{
#ifndef AIR_VP_PEQ_ENABLE
    if (audio_path == 0) {
#else
    if ((audio_path == 0) || (audio_path == 4)) {
#endif
        compander_AU_init(p_scratch, p_nvkey, CPD_AU_RECOVERY_GAIN, band_sw);
    } else {
        compander_AU_init(p_scratch, p_nvkey, CPD3_AU_RECOVERY_GAIN, band_sw);
    }
    DSP_MW_LOG_I("DRC re-init", 0);
}

#if defined(AIR_LINE_IN_LATENCY_LOW) || defined(AIR_LINE_IN_LATENCY_MEDIUM) || defined(AIR_USB_IN_LATENCY_LOW)
ATTR_TEXT_IN_IRAM static bool drc_audio_process(void *para, AU_CPD_CTRL_t *ctrl, AU_CPD_INSTANCE_ptr CPD_AU_ptr)
#else
static bool drc_audio_process(void *para, AU_CPD_CTRL_t *ctrl, AU_CPD_INSTANCE_ptr CPD_AU_ptr)
#endif
{
#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
    if(peq_control_off == 1){
        return false;
    }
#endif
    S32 *BufL = (S32 *)stream_function_get_1st_inout_buffer(para);
    S32 *BufR = (S32 *)stream_function_get_2nd_inout_buffer(para);
    U16 FrameSize = stream_function_get_output_size(para);
    S16 dev_channels = (S16)stream_function_get_device_channel_number(para);
    U8 SampleRate = stream_function_get_samplingrate(para);
    U16 CpdSampleSize;
#ifndef MTK_BT_A2DP_CPD_USE_PIC
    S16 Gain;//DSPGAIN_Q11: (0x800=0dB)
#else
    S32 Gain;//DSPGAIN_Q11: (0x08000000=0dB)
#endif
    S16 channel_mode = 2; // 2:stereo 1:mono
    void *p_scratch = &CPD_AU_ptr->ScratchMemory[0];
    void *p_nvkey = &CPD_AU_ptr->NvKey;

#ifndef MTK_BT_A2DP_CPD_USE_PIC
    Gain = DSP_GC_ConvertQ11Form((S16)(CPD_ZERO_GAIN_INDEX + CPD_AU_ptr->positive_gain));
#else
    Gain = DSP_GC_ConvertQ11Form_32bit((S16)(CPD_ZERO_GAIN_INDEX + CPD_AU_ptr->positive_gain));
#endif

#ifdef AIR_VOLUME_CONTROL_ON_DRC_ENABLE
#ifndef MTK_BT_A2DP_CPD_USE_PIC
    Gain = afe_calculate_digital_gain_index(afe_volume_digital_get_gain_index(AFE_HW_DIGITAL_GAIN1), 0x800);//DSPGAIN_Q11: (0x800=0dB)
#else
    Gain = afe_calculate_digital_gain_index(afe_volume_digital_get_gain_index(AFE_HW_DIGITAL_GAIN1), 0x08000000);//DSPGAIN_Q11: (0x08000000=0dB)
#endif
#endif

    if (FrameSize && ctrl->sample_rate != SampleRate) {
        ctrl->sample_rate = SampleRate;
        CPD_AU_ptr->NvKey.band_sw = Audio_CPD_Get_Fs(SampleRate);
        if ((CPD_AU_ptr->NvKey.CPD_AU_MODE & 0x1) && (CPD_AU_ptr->NvKey.band_sw > 1)) {
            DSP_MW_LOG_E("AU DRC doesn't support dual-band mode for 88.2/96kHz, so force to single-band mode", 0);
            CPD_AU_ptr->NvKey.CPD_AU_MODE &= (~0x1);
        }
        CPD_Reinit(p_scratch, p_nvkey, CPD_AU_ptr->NvKey.band_sw, ctrl->audio_path);
        DSP_MW_LOG_I("DRC re-init to %dkH", 1, SampleRate);
    }

    if ((FrameSize == 0) || (FrameSize % ctrl->sample_base)) {
        return FALSE;
    }

    if (dev_channels == 1) {
        BufR = NULL;
    }

    if (stream_function_get_output_resolution(para) == RESOLUTION_32BIT) {
        CpdSampleSize = FrameSize >> 2;
    } else {
        CpdSampleSize = FrameSize >> 1;
        dsp_converter_16bit_to_32bit(BufL, (S16 *)BufL, CpdSampleSize);
        dsp_converter_16bit_to_32bit(BufR, (S16 *)BufR, CpdSampleSize);
        stream_function_modify_output_size(para, CpdSampleSize << 2);
        stream_function_modify_output_resolution(para, RESOLUTION_32BIT);
    }

#ifdef AIR_AUDIO_DUMP_ENABLE
    if (ctrl == &aud_cpd_ctrl) {
        LOG_AUDIO_DUMP((U8 *)BufL, (U32)(CpdSampleSize << 2), AUDIO_CPD_IN_L);
        if (BufR) {
            LOG_AUDIO_DUMP((U8 *)BufR, (U32)(CpdSampleSize << 2), AUDIO_CPD_IN_R);
        }
    }
#endif
    channel_mode = (BufR == NULL) ? 1 : 2;
    compander_AU_proc(CPD_AU_ptr->ScratchMemory,
                      BufL,
                      BufR,
                      CpdSampleSize,
                      Gain,
                      channel_mode);

#ifdef AIR_AUDIO_DUMP_ENABLE
    if (ctrl == &aud_cpd_ctrl) {
        LOG_AUDIO_DUMP((U8 *)BufL, (U32)(CpdSampleSize << 2), AUDIO_CPD_OUT_L);
        if (BufR) {
            LOG_AUDIO_DUMP((U8 *)BufR, (U32)(CpdSampleSize << 2), AUDIO_CPD_OUT_R);
        }
    }
#endif

    return FALSE;
}

static bool adaptive_eq_drc_audio_process(void *para, AU_CPD_CTRL_t *ctrl, AU_CPD_INSTANCE_ptr CPD_AU_ptr)
{
#if defined(MTK_PEQ_ENABLE) || defined(MTK_LINEIN_PEQ_ENABLE)
    if(aeq_control_off == 1){
        return false;
    }
#endif
    S32 *BufL = (S32 *)stream_function_get_1st_inout_buffer(para);
    S32 *BufR = (S32 *)stream_function_get_2nd_inout_buffer(para);
    U16 FrameSize = stream_function_get_output_size(para);
    S16 dev_channels = (S16)stream_function_get_device_channel_number(para);
    U8 SampleRate = stream_function_get_samplingrate(para);
    U16 CpdSampleSize;
#ifndef MTK_BT_A2DP_CPD_USE_PIC
    S16 Gain;//DSPGAIN_Q11: (0x800=0dB)
#else
    S32 Gain;//DSPGAIN_Q11: (0x08000000=0dB)
#endif
    S16 channel_mode = 2; // 2:stereo 1:mono
    void *p_scratch = &CPD_AU_ptr->ScratchMemory[0];
    void *p_nvkey = &CPD_AU_ptr->NvKey;

#ifndef MTK_BT_A2DP_CPD_USE_PIC
    Gain = DSP_GC_ConvertQ11Form((S16)(CPD_ZERO_GAIN_INDEX + CPD_AU_ptr->positive_gain));
#else
    Gain = DSP_GC_ConvertQ11Form_32bit((S16)(CPD_ZERO_GAIN_INDEX + CPD_AU_ptr->positive_gain));
#endif

    if (ctrl->sample_rate != SampleRate) {
        ctrl->sample_rate = SampleRate;
        CPD_AU_ptr->NvKey.band_sw = Audio_CPD_Get_Fs(SampleRate);
        if ((CPD_AU_ptr->NvKey.CPD_AU_MODE & 0x1) && (CPD_AU_ptr->NvKey.band_sw > 1)) {
            DSP_MW_LOG_E("AU DRC doesn't support dual-band mode for 88.2/96kHz, so force to single-band mode", 0);
            CPD_AU_ptr->NvKey.CPD_AU_MODE &= (~0x1);
        }
        CPD_Reinit(p_scratch, p_nvkey, CPD_AU_ptr->NvKey.band_sw, ctrl->audio_path);
        DSP_MW_LOG_I("DRC re-init to %dkH", 1, SampleRate);
    }

    if (ctrl->enable == 0) {
        return FALSE;
    }

    if ((FrameSize == 0) || (FrameSize % ctrl->sample_base)) {
        return FALSE;
    }

    if (dev_channels == 1) {
        BufR = NULL;
    }

    CpdSampleSize = FrameSize >> 2;

    channel_mode = (BufR == NULL) ? 1 : 2;
    compander_AU_proc(CPD_AU_ptr->ScratchMemory,
                      BufL,
                      BufR,
                      CpdSampleSize,
                      Gain,
                      channel_mode);

    return FALSE;
}

/**
 * stream_function_drc_audio_process
 *
 * audio CPD main process
 *
 *
 * @para : Default parameter of callback function
 *
 */
bool stream_function_drc_audio_process(void *para)
{
    return drc_audio_process(para, &aud_cpd_ctrl, Au1CpdMemoryPtr);
}

bool stream_function_drc_audio2_process(void *para)
{
    return drc_audio_process(para, &aud_cpd2_ctrl, Au2CpdMemoryPtr);
}

bool stream_function_drc_audio3_process(void *para)
{
    return drc_audio_process(para, &aud_cpd3_ctrl, Au3CpdMemoryPtr);
}

bool stream_function_adaptive_eq_drc_process(void *para)
{
    return adaptive_eq_drc_audio_process(para, &aud_adaptive_eq_drc_ctrl, AdeqCpdMemoryPtr);
}

#ifdef AIR_VP_PEQ_ENABLE
bool stream_function_drc_vp_process(void *para)
{
    return drc_audio_process(para, &aud_vp_cpd_ctrl, AuVPCpdMemoryPtr);
}
#ifdef AIR_ADAPTIVE_EQ_ENABLE
bool stream_function_drc_vp_aeq_process(void *para)
{
    if (aud_adaptive_eq_drc_ctrl.enable != 1) {
        return false;
    } else {
        return adaptive_eq_drc_audio_process(para, &aud_vp_aeq_cpd_ctrl, AuVPAeqCpdMemoryPtr);
    }
}
#endif
#endif

bool stream_function_wired_usb_drc_process(void *para)
{
    return drc_audio_process(para, &aud_usb_peq_cpd_ctrl, Au_Usb_CpdMemoryPtr);
}
#ifdef PRELOADER_ENABLE
bool Voice_CPD_Open(void *para)
{
    DSP_MW_LOG_I("[PIC] Voice Compander Open", 0);
    UNUSED(para);
    return TRUE;
}

bool Voice_CPD_Close(void *para)
{
    DSP_MW_LOG_I("[PIC] Voice Compander Close", 0);
    UNUSED(para);
    return TRUE;
}

bool Audio_CPD_Open(void *para)
{
    DSP_MW_LOG_I("[PIC] Audio Compander Open", 0);
    UNUSED(para);
    return TRUE;
}

bool Audio_CPD_Close(void *para)
{
    DSP_MW_LOG_I("[PIC] Audio Compander Close", 0);
    UNUSED(para);
    return TRUE;
}
#endif


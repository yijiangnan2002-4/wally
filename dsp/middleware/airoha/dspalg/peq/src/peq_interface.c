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
#include "dsp_share_memory.h"
#include "dsp_memory.h"
#include "dsp_callback.h"
#include "dsp_audio_process.h"
#include "peq_interface.h"
#include "audio_nvdm_common.h"
#include "dsp_sdk.h"
#include "long_term_clk_skew.h"
#include "clk_skew.h"
#include "dsp_dump.h"
#ifdef MTK_BT_PEQ_USE_PIC
#include "peq_portable.h"
#endif
#include "exception_handler.h"
#include "compander_interface.h"
#include "source_inter.h"
#ifdef AIR_ADAPTIVE_EQ_ENABLE
#include "adaptive_eq_interface.h"
#include "hal_audio_cm4_dsp_message.h"
#include "dsp_audio_msg.h"
#endif
#include "hal_resource_assignment.h"
#ifdef AIR_AUDIO_TRANSMITTER_ENABLE
#include "audio_transmitter_mcu_dsp_common.h"
#endif

#define NVDM_ITEM_NULL      (0)

#define NEED_UPDATE_NO      (0)
#define NEED_UPDATE_NOW     (1)
#define NEED_UPDATE_LATER   (2)

#define CH_MASK_L           (0x1 << 0)
#define CH_MASK_R           (0x1 << 1)
#define CH_MASK_STEREO      (CH_MASK_L | CH_MASK_R)

#define SUPPORT_SWITCH_COEF_OVERLAP
#define MACRO_CHECK_AVAILABLE_NVKEY(x) ((x >= NVKEY_DSP_PARA_PEQ_COEF_01) && (x <= NVKEY_DSP_PARA_AEQ_COEF_7))
uint32_t peq_control_off = 0;
uint32_t aeq_control_off = 0;
extern ltcs_header_type_t ltcs_receive;
extern ltcs_ctrl_type_t ltcs_ctrl;
extern AU_CPD_CTRL_t aud_adaptive_eq_drc_ctrl;
#ifdef AIR_ADAPTIVE_EQ_ENABLE
extern DSP_AEQ_CTRL_g aeq_ctrl_g;
aeq_share_info_t *share_addr;
#endif
static DSP_PEQ_CTRL_t peq_ctrl = {
    .p_peq_inter_param = 0,
    .peq_nvkey_id = 0,
    .sample_rate = 0,
    .enable = false,
    .proc_ch_mask = CH_MASK_STEREO,
    .p_overlap_buffer = NULL,
    .peq_mode = 0,
    .audio_path = 0,
};
static DSP_PEQ_CTRL_t peq_ctrl_2 = {
    .p_peq_inter_param = 0,
    .peq_nvkey_id = 0,
    .sample_rate = 0,
    .enable = false,
    .proc_ch_mask = CH_MASK_STEREO,
    .p_overlap_buffer = NULL,
    .peq_mode = 0,
    .audio_path = 0,
};
static DSP_PEQ_CTRL_t peq_ctrl_3 = {
    .p_peq_inter_param = 0,
    .peq_nvkey_id = 0,
    .sample_rate = 0,
    .enable = false,
    .proc_ch_mask = CH_MASK_STEREO,
    .p_overlap_buffer = NULL,
    .peq_mode = 0,
    .audio_path = 1,
};
static DSP_PEQ_CTRL_t peq_ctrl_4 = {
    .p_peq_inter_param = 0,
    .peq_nvkey_id = 0,
    .sample_rate = 0,
    .enable = false,
    .proc_ch_mask = CH_MASK_STEREO,
    .p_overlap_buffer = NULL,
    .peq_mode = 0,
    .audio_path = 2,
};
static DSP_PEQ_CTRL_t peq_ctrl_5 = {
    .p_peq_inter_param = 0,
    .peq_nvkey_id = 0,
    .sample_rate = 0,
    .enable = false,
    .proc_ch_mask = CH_MASK_STEREO,
    .p_overlap_buffer = NULL,
    .peq_mode = 0,
    .audio_path = 2,
};


static DSP_PEQ_CTRL_t peq_ctrl_adaptive_eq = {
    .p_peq_inter_param = 0,
    .peq_nvkey_id = 0,
    .sample_rate = 0,
    .enable = false,
    .proc_ch_mask = CH_MASK_STEREO,
    .p_overlap_buffer = NULL,
    .peq_mode = 0,
    .audio_path = 3,
};

static DSP_PEQ_CTRL_t peq_ctrl_mic_peq = {
    .p_peq_inter_param = 0,
    .peq_nvkey_id = 0,
    .sample_rate = 0,
    .enable = false,
    .proc_ch_mask = CH_MASK_STEREO,
    .p_overlap_buffer = NULL,
    .peq_mode = 0,
    .audio_path = 6,
};

static DSP_PEQ_CTRL_t peq_ctrl_usb_peq = {
    .p_peq_inter_param = 0,
    .peq_nvkey_id = 0,
    .sample_rate = 0,
    .enable = false,
    .proc_ch_mask = CH_MASK_STEREO,
    .p_overlap_buffer = NULL,
    .peq_mode = 0,
    .audio_path = 8,
};
static DSP_PEQ_CTRL_t peq_ctrl_advanced_record_peq = {
    .p_peq_inter_param = 0,
    .peq_nvkey_id = 0,
    .sample_rate = 0,
    .enable = false,
    .proc_ch_mask = CH_MASK_STEREO,
    .p_overlap_buffer = NULL,
    .peq_mode = 0,
    .audio_path = 6,
};

#ifdef AIR_VP_PEQ_ENABLE
static DSP_PEQ_CTRL_t peq_ctrl_vp = {
    .p_peq_inter_param = 0,
    .peq_nvkey_id = 0,
    .sample_rate = 0,
    .enable = false,
    .proc_ch_mask = CH_MASK_STEREO,
    .p_overlap_buffer = NULL,
    .peq_mode = 0,
    .audio_path = 4,
};

static DSP_PEQ_CTRL_t peq_ctrl_vp_2 = {
    .p_peq_inter_param = 0,
    .peq_nvkey_id = 0,
    .sample_rate = 0,
    .enable = false,
    .proc_ch_mask = CH_MASK_STEREO,
    .p_overlap_buffer = NULL,
    .peq_mode = 0,
    .audio_path = 4,
};
#ifdef AIR_ADAPTIVE_EQ_ENABLE
static DSP_PEQ_CTRL_t peq_ctrl_vp_aeq = {
    .p_peq_inter_param = 0,
    .peq_nvkey_id = 0,
    .sample_rate = 0,
    .enable = false,
    .proc_ch_mask = CH_MASK_STEREO,
    .p_overlap_buffer = NULL,
    .peq_mode = 0,
    .audio_path = 5,
};
#endif
#endif

#if defined(AIR_ADAPTIVE_EQ_ENABLE) && defined(MTK_PEQ_ENABLE)
static DSP_ADEQ_CTRL_t adaptive_eq1_param = {
    .p_peq_inter_param = 0,
    .peq_nvkey_id = 0,
};
static DSP_ADEQ_CTRL_t adaptive_eq2_param = {
    .p_peq_inter_param = 0,
    .peq_nvkey_id = 0,
};
static DSP_ADEQ_CTRL_t adaptive_eq3_param = {
    .p_peq_inter_param = 0,
    .peq_nvkey_id = 0,
};
static DSP_ADEQ_CTRL_t adaptive_eq4_param = {
    .p_peq_inter_param = 0,
    .peq_nvkey_id = 0,
};
static DSP_ADEQ_CTRL_t adaptive_eq5_param = {
    .p_peq_inter_param = 0,
    .peq_nvkey_id = 0,
};
static DSP_ADEQ_CTRL_t adaptive_eq6_param = {
    .p_peq_inter_param = 0,
    .peq_nvkey_id = 0,
};
static DSP_ADEQ_CTRL_t adaptive_eq7_param = {
    .p_peq_inter_param = 0,
    .peq_nvkey_id = 0,
};
#endif
#ifdef MTK_DEQ_ENABLE
static DSP_PEQ_CTRL_t deq_ctrl = {
    .p_peq_inter_param = 0,
    .peq_nvkey_id = 0,
    .sample_rate = 0,
    .enable = false,
    .proc_ch_mask = CH_MASK_R,
    .p_overlap_buffer = NULL,
    .peq_mode = 1,
};
#define DEQ_0DB_REGISTER_VALUE      (32768)
static uint8_t g_deq_phase_inverse = 0;
static uint8_t g_deq_delay_samples = 0;
static uint32_t g_deq_digital_gain = DEQ_0DB_REGISTER_VALUE;
static uint8_t g_earbuds_ch = 0; //0:headset  1:earbuds-L  2: earbuds-R
#define DEQ_DEBUG (1)
#if (DEQ_DEBUG == 1)
uint32_t deq_mute_ch = 0;
#endif
#endif

#ifdef AIR_HEARTHROUGH_VIVID_PT_ENABLE
static DSP_PEQ_CTRL_t peq_ctrl_vivid_pt = {
    .p_peq_inter_param = 0,
    .peq_nvkey_id = 0,
    .sample_rate = 0,
    .enable = false,
    .proc_ch_mask = CH_MASK_L,
    .p_overlap_buffer = NULL,
    .peq_mode = 1,
    .audio_path = 9,
};
#endif

#ifdef DSP_PEQ_SYNC_WITH_BT_CLOCK_PTS
static DSP_PEQ_SYNC_CTRL_t peq_sync_ctrl;
#endif
static const uint8_t peq_allpass_coef_441kHz[] = {
    0x01, 0x00, 0x90, 0x6B, 0x00, 0x22, 0x3D, 0x95, 0x00, 0xB7, 0x0A, 0x6A, 0x00, 0xA3, 0xF4, 0x80,
    0x00, 0xF5, 0x30, 0x7E, 0x00, 0x80, 0xC2, 0x04, 0x00, 0x8F,
};
static const uint8_t peq_allpass_coef_48kHz[] = {
    0x01, 0x00, 0x90, 0x6B, 0x00, 0x22, 0x2C, 0x95, 0x00, 0x46, 0x2A, 0x6A, 0x00, 0x12, 0xE0, 0x80,
    0x00, 0x34, 0x55, 0x7E, 0x00, 0xE8, 0xC2, 0x04, 0x00, 0x8F,
};
static const uint8_t peq_allpass_coef_96kHz[] = {
    0x01, 0x00, 0x90, 0x6B, 0x00, 0x22, 0xCC, 0x94, 0x00, 0x08, 0xDC, 0x6A, 0x00, 0x81, 0x6D, 0x80,
    0x00, 0xAE, 0x2A, 0x7F, 0x00, 0x3D, 0xC2, 0x04, 0x00, 0x8F,
};
static const uint8_t peq_allpass_coef_192kHz[] = {
    0x01, 0x00, 0x90, 0x6B, 0x00, 0x22, 0x9D, 0x94, 0x00, 0x70, 0x36, 0x6B, 0x00, 0x2C, 0x36, 0x80,
    0x00, 0x3B, 0x94, 0x7F, 0x00, 0xF1, 0xC2, 0x04, 0x00, 0x8F,
};

/**
 * PEQ_Overlap
 *
 * This function is used to overlap two PEQ process
 *
 *
 * @Buf1 : Stream processed with current PEQ
 * @Buf2 : Stream processed with original PEQ
 * @SwitchProgress : Percentage of switch progress
 * @FrameSamples : Sample length per frame
 *
 */
VOID peq_overlap(S32 *Buf1, S32 *Buf2, S32 *SwitchProgress, U32 FrameSamples)
{
    U16 Frame = 0;
    U16 Index = 0;

    for (Frame = 0 ; Frame < (FrameSamples / PEQ_OVERLAP_FRAME_SIZE) ; Frame++) {
        for (Index = 0 ; Index < PEQ_OVERLAP_FRAME_SIZE ; Index++) {
            Buf1[Frame * PEQ_OVERLAP_FRAME_SIZE + Index]
                = (PEQ_SWITCH_TOTAL_PERCENTAGE - *SwitchProgress) * (Buf2[Frame * PEQ_OVERLAP_FRAME_SIZE + Index] / PEQ_SWITCH_TOTAL_PERCENTAGE)
                  + *SwitchProgress * (Buf1[Frame * PEQ_OVERLAP_FRAME_SIZE + Index] / PEQ_SWITCH_TOTAL_PERCENTAGE) ;
        }

        if (*SwitchProgress + PEQ_SWITCH_STEP_PERCENTAGE < PEQ_SWITCH_TOTAL_PERCENTAGE) {
            *SwitchProgress += PEQ_SWITCH_STEP_PERCENTAGE;
        } else {
            *SwitchProgress = PEQ_SWITCH_TOTAL_PERCENTAGE;
        }
    }
}

static DSP_PEQ_SUB_CTRL_t *peq_get_sub_ctrl_by_status(DSP_PEQ_CTRL_t *p_peq_ctrl, uint8_t status)
{
    if (p_peq_ctrl->sub_ctrl[0].status == status) {
        return &p_peq_ctrl->sub_ctrl[0];
    } else if (p_peq_ctrl->sub_ctrl[1].status == status) {
        return &p_peq_ctrl->sub_ctrl[1];
    }
    return NULL;
}
#if defined(AIR_VP_PEQ_ENABLE) || defined(MTK_DEQ_ENABLE)
static uint8_t peq_check_all_off(DSP_PEQ_CTRL_t *p_peq_ctrl)
{
    if ((p_peq_ctrl->sub_ctrl[0].status == PEQ_STATUS_OFF) && (p_peq_ctrl->sub_ctrl[1].status == PEQ_STATUS_OFF)) {
        return 1;
    } else {
        return 0;
    }
}
#endif
/**
 * sample_rate_to_element_id
 *
 * This function is used to transform sampling rate to element ID of nvkey.
 *
 *
 * @rate: sampling rate of input pcm.
 *
 */
static uint8_t sample_rate_to_element_id(uint32_t sample_rate)
{
    /*PEQ only support FS: 32000/44100/48000 (Hz)*/
    uint8_t element_id = PEQ_44_1K;
    switch (sample_rate) {
        case FS_RATE_32K:
        case 32000:
            element_id = PEQ_32K;
            break;
        case FS_RATE_44_1K:
        case 44100:
            element_id = PEQ_44_1K;
            break;
        case FS_RATE_48K:
        case 48000:
            element_id = PEQ_48K;
            break;
        case FS_RATE_88_2K:
        case 88200:
            element_id = PEQ_88_2K;
            break;
        case FS_RATE_96K:
        case 96000:
            element_id = PEQ_96K;
            break;
        case FS_RATE_24K:
        case 24000:
            element_id = PEQ_24K;
            break;
        case FS_RATE_50K:
        case 50000:
            element_id = PEQ_50K;
            break;
        case FS_RATE_192K:
        case 192000:
            element_id = PEQ_192K;
            break;
        default:
            element_id = PEQ_48K;
            break;
    }
    return element_id;
}

/**
 * peq_get_inter_param
 *
 * This function is used to get internal parameter pointer from nvkey parameter.
 *
 *
 * @sample_rate: sampling rate of input pcm.
 *
 */
static uint8_t peq_get_inter_param(DSP_PEQ_CTRL_t *p_peq_ctrl, uint32_t sample_rate)
{
    uint8_t element_id = sample_rate_to_element_id(sample_rate);
    uint8_t i, valid = 0;

    for (i = 0; i < p_peq_ctrl->peq_nvkey_param.numOfElement; i++) {
        if (p_peq_ctrl->peq_nvkey_param.peq_element_param[i].elementID == element_id) {
            p_peq_ctrl->p_peq_inter_param = p_peq_ctrl->peq_nvkey_param.peq_element_param[i].peq_inter_param;
            valid = 1;
            break;
        }
    }
    return valid;
}

void peq_get_defalut_param(DSP_PEQ_CTRL_t *p_peq_ctrl, uint8_t rate)
{
    if(rate == FS_RATE_44_1K){
        p_peq_ctrl->p_peq_inter_param = (S16 *)peq_allpass_coef_441kHz;
    }else if(rate == FS_RATE_48K){
        p_peq_ctrl->p_peq_inter_param = (S16 *)peq_allpass_coef_48kHz;
    }else if(rate == FS_RATE_96K){
        p_peq_ctrl->p_peq_inter_param = (S16 *)peq_allpass_coef_96kHz;
    }else if(rate == FS_RATE_192K){
        p_peq_ctrl->p_peq_inter_param = (S16 *)peq_allpass_coef_192kHz;
    }else{
        DSP_MW_LOG_E("[%d] peq_get_defalut_param fail, rate:%d \n", 2, p_peq_ctrl->phase_id, rate);
    }
}
#ifdef DSP_PEQ_SYNC_WITH_BT_CLOCK_PTS
/**
 * peq_calculate_target_timestamp
 *
 * This function is used to calculate target PTS from anchor info and target bt clock.
 *
 *
 * @target_bt_clk: target bt clock.
 *
 */

static uint32_t peq_calculate_sample_count(uint32_t target_bt_clk, uint32_t anchor_bt_clk, uint32_t sample_rate, uint32_t drift)
{
    S64 diff_us;
    if ((anchor_bt_clk > (int)(1 << 27)) && (target_bt_clk < (int)(1 << 27))) { /* btc wrap */
        diff_us = (((S64)target_bt_clk + (S64)(1 << 28)) * 625 / 2) - ((S64)anchor_bt_clk * 625 / 2);
    } else {
        diff_us = ((S64)target_bt_clk * 625 / 2) - ((S64)anchor_bt_clk * 625 / 2);
    }
    return ((U32)((S64)(diff_us * sample_rate) / (S64)(1000000 + drift)));
}

static uint32_t peq_calculate_timestamp(uint32_t sample_diff, uint32_t old_timestamp)
{
    uint32_t new_timestamp;
#if defined(AIR_BTA_IC_PREMIUM_G2)
    uint32_t asi_alignment = 1024;
    new_timestamp = old_timestamp + sample_diff * 1;
    new_timestamp = (new_timestamp + asi_alignment / 2) & (~(asi_alignment - 1));
#else
    ltcs_bt_type_t type = ((ltcs_receive.a2dp_audio_type == LTCS_TYPE_AAC) && (ltcs_ctrl.ts_ratio == -1)) ? LTCS_TYPE_IOS_AAC : ltcs_receive.a2dp_audio_type;
    peq_sync_ctrl.use_seqno = ((type == LTCS_TYPE_IOS_AAC) || (type == LTCS_TYPE_IOS_SBC)) ? 1 : 0;

    switch (type) {
        case LTCS_TYPE_IOS_AAC:
            new_timestamp = old_timestamp + (sample_diff >> 10); //sample_count/1024
            new_timestamp = (new_timestamp >= 65536) ? (new_timestamp - 65536) : new_timestamp;
            break;
        case LTCS_TYPE_AAC:
        case LTCS_TYPE_ANDROID_AAC:
        case LTCS_TYPE_SBC:
        case LTCS_TYPE_VENDOR:
            new_timestamp = (old_timestamp + (sample_diff * ltcs_ctrl.ts_ratio));
            break;
        case LTCS_TYPE_IOS_SBC:
            new_timestamp = old_timestamp + (sample_diff / (ltcs_receive.timestamp * 128)); //sample_count/(num_frame*128)
            new_timestamp = (new_timestamp >= 65536) ? (new_timestamp - 65536) : new_timestamp;
            break;
        case LTCS_TYPE_OTHERS:
        default:
            return 0;
    }
#endif
    return new_timestamp;
}

static uint32_t peq_calculate_target_timestamp(uint32_t target_bt_clk, uint32_t base_bt_clk, uint32_t base_timestamp)
{
    uint32_t sample_count, target_timestamp;
    target_bt_clk -= ltcs_ctrl.sink_latency * 2 / 625;
    sample_count = peq_calculate_sample_count(target_bt_clk, base_bt_clk, ltcs_ctrl.sample_rate, ltcs_ctrl.drift_val);
    target_timestamp = peq_calculate_timestamp(sample_count, base_timestamp);
    return target_timestamp;
}

static uint32_t peq_calculate_anchor_timestamp(uint32_t asi_threshold, uint32_t old_anchor_timestamp)
{
    uint32_t new_anchor_timestamp = peq_calculate_timestamp(asi_threshold, old_anchor_timestamp);
    // DSP_MW_LOG_I("update peq anchor_timestamp: %d -> %d, asi_threshold:%d\n", 3, old_anchor_timestamp, new_anchor_timestamp, asi_threshold);
    return new_anchor_timestamp;
}
#endif

/**
 * peq_get_coef_nvdm
 *
 * This function is used to get PEQ parameter from nvdm and set to peq controller.
 *
 *
 * @dst_addr        : Destination address of PEQ parameters.
 * @peq_nvkey_id : PEQ parameter nvkey ID.
 *
 */
static sysram_status_t peq_get_coef_nvdm(uint16_t key_id, uint8_t *src_ptr, DSP_PEQ_NVKEY_t *dst_ptr)
{
    nat_nvdm_info_t *p_nat_nvdm_info = (nat_nvdm_info_t *)src_ptr;
    uint16_t *u16src_pt;
    uint16_t chksum = 0;
    int32_t i;
    if (src_ptr == NULL) {
        return -1;
    }
    u16src_pt = (uint16_t *)(src_ptr + p_nat_nvdm_info->offset);

    if (p_nat_nvdm_info->nvdm_id != key_id) {
        return NVDM_STATUS_NAT_ITEM_NOT_FOUND;
    }

    src_ptr = src_ptr + p_nat_nvdm_info->offset;
    for (i = 0 ; i < p_nat_nvdm_info->length ; i++) {
        chksum += (uint16_t)(*src_ptr++);
    }
    if (chksum != p_nat_nvdm_info->chksum) {
        return NVDM_STATUS_NAT_INCORRECT_CHECKSUM;
    }

    /* Memory copy param */
    dst_ptr->numOfElement = u16src_pt[0];
    dst_ptr->peqAlgorithmVer = u16src_pt[1];
    u16src_pt += 2;
    if (dst_ptr->numOfElement > FW_MAX_ELEMENT) {
        return -1;
    }
    for (i = 0; i < dst_ptr->numOfElement; i++) {
        uint16_t length = u16src_pt[1] + 2;
        memcpy(&dst_ptr->peq_element_param[i], u16src_pt, sizeof(uint16_t) * length);
        u16src_pt += length;
    }

    return NVDM_STATUS_NAT_OK;
}

U8 peq_get_trigger_drc(U8 phase_id, U8 type)
{
    if (phase_id == 0) {
        if (type == 0) {
            return peq_ctrl.trigger_drc;
        } else if(type == 1) {
            return peq_ctrl_3.trigger_drc;
#ifdef AIR_VP_PEQ_ENABLE
        } else if (type == 4) {
            return peq_ctrl_vp.trigger_drc;
#ifdef AIR_ADAPTIVE_EQ_ENABLE
        } else if (type == 5) {
            return peq_ctrl_vp_aeq.trigger_drc;
#endif
#endif
        } else if(type == 8) {
            return peq_ctrl_usb_peq.trigger_drc;
        } else{
            return peq_ctrl_adaptive_eq.trigger_drc;
        }
    } else if (phase_id == 1) {
        return peq_ctrl_2.trigger_drc;
    } else {
        return 0;
    }
}

/**
 * PEQ_Update_Info
 *
 * This function is used to update current packet timestamp or seq number.
 *
 *
 * @anchor_bt_clk: anchor bt clock from share buffer.
 * @anchor_timestamp: predicted timestamp of first packet from cm4.
 *
 */
void PEQ_Update_Info(U32 anchor_bt_clk, U32 anchor_timestamp)
{
#ifdef DSP_PEQ_SYNC_WITH_BT_CLOCK_PTS
    anchor_bt_clk -= (ltcs_ctrl.sink_latency * 2 / 625);

    if (peq_sync_ctrl.started == 0) {
        peq_sync_ctrl.prepare_bt_clk = anchor_bt_clk;
        peq_sync_ctrl.anchor_bt_clk = anchor_bt_clk;
        peq_sync_ctrl.anchor_timestamp = anchor_timestamp;
        peq_sync_ctrl.started = 1;
        DSP_MW_LOG_I("peq first pkt: ts:%d clk:%d\n", 2, peq_sync_ctrl.anchor_timestamp, peq_sync_ctrl.anchor_bt_clk);
    }

    if (peq_sync_ctrl.prepare_bt_clk != anchor_bt_clk) {
        peq_sync_ctrl.prepare_bt_clk = anchor_bt_clk;
    }

    peq_sync_ctrl.current_timestamp = anchor_timestamp;
#else
    UNUSED(anchor_bt_clk);
    UNUSED(anchor_timestamp);
#endif
}

/**
 * PEQ_Reset_Info
 *
 * This function is used to reset parameters when source open.
 *
 *
 */
void PEQ_Reset_Info(void)
{
#ifdef DSP_PEQ_SYNC_WITH_BT_CLOCK_PTS
    peq_sync_ctrl.started = 0;
    peq_sync_ctrl.asi_cnt = 0;
#endif
}

#ifdef AIR_ADAPTIVE_EQ_ENABLE
static VOID adaptive_eq_notify_mcu_index(S16 eq_index)
{
    hal_ccni_message_t msg;

    memset((void *)&msg, 0, sizeof(hal_ccni_message_t));

    msg.ccni_message[0] = MSG_DSP2MCU_AEQ_NOTIFY_INDEX << 16;
    msg.ccni_message[1] = eq_index;

    if (AUDIO_MSG_STATUS_OK != aud_msg_tx_handler(msg, 0, FALSE)) {
        DSP_MW_LOG_I("[AEQ][adaptive_eq_notify_mcu_index]adaptive_eq index Fail!", 0);
    } else {
        DSP_MW_LOG_I("[AEQ][adaptive_eq_notify_mcu_index]adaptive_eq index Success! eq_index:%d", 1, eq_index);
    }
}
#endif

void Audio_Peq_Enable_Control(hal_ccni_message_t msg)
{
    if ((msg.ccni_message[0] & 0xFFFF) == PEQ_DISABLE_ALL) {  
        peq_control_off = 1;
    }else{
        peq_control_off = 0;
    }
}

/**
 * PEQ_Set_Param
 *
 * This function is used to update PEQ parameter and nvkey ID.
 *
 *
 * @msg : ccni message with index MSG_MCU2DSP_COMMON_PEQ_SET_PARAM.
 *
 */
void PEQ_Set_Param(hal_ccni_message_t msg, hal_ccni_message_t *ack, BOOL BypassTimestamp)
{
    mcu2dsp_peq_param_p peq_param;
    DSP_PEQ_CTRL_t *p_peq_ctrl;
    uint32_t force_reinit = 0;
    int status;
    UNUSED(ack);

    peq_param = (mcu2dsp_peq_param_p)hal_memview_mcu_to_dsp0(msg.ccni_message[1]);
    peq_param->nvkey_addr = (uint8_t *)hal_memview_mcu_to_dsp0((uint32_t)peq_param->nvkey_addr);

    if (peq_param->phase_id == 0) {
        if ((msg.ccni_message[0] & 0xFFFF) == PEQ_AUDIO_PATH_A2DP) {
            p_peq_ctrl = &peq_ctrl;
        } else if ((msg.ccni_message[0] & 0xFFFF) == PEQ_AUDIO_PATH_LINEIN) {
            p_peq_ctrl = &peq_ctrl_3;
        } else if ((msg.ccni_message[0] & 0xFFFF) == PEQ_AUDIO_PATH_ADVANCED_PASSTHROUGH) {
            p_peq_ctrl = &peq_ctrl_4;
        } else if ((msg.ccni_message[0] & 0xFFFF) == PEQ_AUDIO_PATH_ADAPTIVE_EQ) {
            p_peq_ctrl = &peq_ctrl_adaptive_eq;
#ifdef AIR_VP_PEQ_ENABLE
        } else if ((msg.ccni_message[0] & 0xFFFF) == PEQ_AUDIO_PATH_VP) {
            p_peq_ctrl = &peq_ctrl_vp;
#ifdef AIR_ADAPTIVE_EQ_ENABLE
        } else if ((msg.ccni_message[0] & 0xFFFF) == PEQ_AUDIO_PATH_VP_AEQ) {
            p_peq_ctrl = &peq_ctrl_vp_aeq;
#endif
#endif
#if defined (AIR_WIRED_AUDIO_ENABLE)||(AIR_WIRELESS_MIC_TX_ENABLE)
        } else if ((msg.ccni_message[0] & 0xFFFF) == PEQ_AUDIO_PATH_MIC) {
            p_peq_ctrl = &peq_ctrl_mic_peq;
#endif
        } else if ((msg.ccni_message[0] & 0xFFFF) == PEQ_AUDIO_PATH_ADVANCED_RECORD) {
            p_peq_ctrl = &peq_ctrl_advanced_record_peq;
        } else if ((msg.ccni_message[0] & 0xFFFF) == PEQ_AUDIO_PATH_USB) {
            p_peq_ctrl = &peq_ctrl_usb_peq;
#ifdef AIR_HEARTHROUGH_VIVID_PT_ENABLE
        } else if ((msg.ccni_message[0] & 0xFFFF) == PEQ_AUDIO_PATH_VIVID_PT) {
            p_peq_ctrl = &peq_ctrl_vivid_pt;
#endif
        } else {
            p_peq_ctrl = &peq_ctrl_5;
        }
    } else if (peq_param->phase_id == 1) {
        p_peq_ctrl = &peq_ctrl_2;
#ifdef AIR_VP_PEQ_ENABLE
        if ((msg.ccni_message[0] & 0xFFFF) == PEQ_AUDIO_PATH_VP) {
            p_peq_ctrl = &peq_ctrl_vp_2;
        }
#endif
#ifdef MTK_DEQ_ENABLE
    } else if (peq_param->phase_id == 2) {
        p_peq_ctrl = &deq_ctrl;
#endif
    } else {
        DSP_MW_LOG_E("[PEQ_Set_Param] Un-supported phase id: %d\n", 1, peq_param->phase_id);
        return;
    }

    if ((peq_param->peq_nvkey_id == NVKEY_DSP_PARA_PEQ) || (peq_param->peq_nvkey_id == 0) || (peq_param->peq_nvkey_id == NVKEY_DSP_PARA_PSAP_AUDIO_PEQ)
        || ((peq_param->peq_nvkey_id >= NVKEY_DSP_PARA_PEQ_COEF_29) && (peq_param->peq_nvkey_id <= NVKEY_DSP_PARA_PEQ_COEF_32))
#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE)
        || (peq_param->peq_nvkey_id == NVKEY_DSP_PARA_ADVANCED_PASSTHROUGH_SPEACIL_PEQ)
        || ((peq_param->peq_nvkey_id >= NVKEY_DSP_PARA_ADVANCED_PASSTHROUGH_PRE_PEQ_1) && (peq_param->peq_nvkey_id <= NVKEY_DSP_PARA_ADVANCED_PASSTHROUGH_PRE_PEQ_3))
#endif /* AIR_ADVANCED_PASSTHROUGH_ENABLE */
#if defined (AIR_WIRED_AUDIO_ENABLE)||(AIR_WIRELESS_MIC_TX_ENABLE)||(AIR_RECORD_ADVANCED_ENABLE)
        || ((peq_param->peq_nvkey_id >= NVKEY_DSP_PARA_MIC_EQ_COEF_1) && (peq_param->peq_nvkey_id <= NVKEY_DSP_PARA_MIC_EQ_COEF_5))
#endif
        || ((peq_param->peq_nvkey_id >= NVKEY_DSP_PARA_VIVID_PT_PEQ_1) && (peq_param->peq_nvkey_id <= NVKEY_DSP_PARA_VIVID_PT_PEQ_3))
       ) {
        force_reinit = 1;
    }

    if ((force_reinit == 1)
        || (MACRO_CHECK_AVAILABLE_NVKEY(peq_param->peq_nvkey_id) /*&& (p_peq_ctrl->peq_nvkey_id != peq_param->peq_nvkey_id)*/)) {
        p_peq_ctrl->peq_nvkey_id = peq_param->peq_nvkey_id;
#ifdef AIR_ADAPTIVE_EQ_ENABLE
        if(p_peq_ctrl == &peq_ctrl_adaptive_eq){
            if(peq_param->peq_nvkey_id != NVKEY_DSP_PARA_PEQ){
                aeq_ctrl_g.aeq_nvk = peq_param->peq_nvkey_id;
                aeq_ctrl_g.aeq_sonud_mode = aeq_nvk_get_sound_mode(peq_param->peq_nvkey_id);
                share_addr->aeq_sound_mode = aeq_ctrl_g.aeq_sonud_mode;
                // adaptive_eq_notify_mcu_index(aeq_ctrl_g.aeq_sonud_mode);
                adaptive_eq_notify_mcu_index(aeq_ctrl_g.aeq_sonud_mode);
                DSP_MW_LOG_I("[AEQ_Set_Param] AEQ nvkid:0x%x\n",1,peq_param->peq_nvkey_id);
            }
        }
#endif
        if (peq_param->peq_nvkey_id != 0) {
            status = peq_get_coef_nvdm(p_peq_ctrl->peq_nvkey_id, peq_param->nvkey_addr, &p_peq_ctrl->peq_nvkey_param);
            if (status != 0) {
                DSP_MW_LOG_E("[PEQ_Set_Param] Get PEQ param error: status:%d  nvkeyID:%d \n", 2, status, p_peq_ctrl->peq_nvkey_id);
                return;
            }
        }
        if (peq_param->setting_mode == PEQ_DIRECT) {
            p_peq_ctrl->need_update = NEED_UPDATE_NOW;
            p_peq_ctrl->enable = (p_peq_ctrl->peq_nvkey_id == 0) ? false : true;
        } else if (peq_param->setting_mode == PEQ_SYNC) {
#ifdef DSP_PEQ_SYNC_WITH_BT_CLOCK_PTS
            if (!BypassTimestamp) {
                p_peq_ctrl->target_timestamp = peq_calculate_target_timestamp(peq_param->target_bt_clk, peq_sync_ctrl.anchor_bt_clk, peq_sync_ctrl.anchor_timestamp);
            } else {
                p_peq_ctrl->target_timestamp = peq_param->target_bt_clk;
            }
            p_peq_ctrl->need_update = NEED_UPDATE_LATER;
#else
            p_peq_ctrl->need_update = NEED_UPDATE_NOW;
            p_peq_ctrl->enable = (p_peq_ctrl->peq_nvkey_id == 0) ? false : true;
#endif
        }
        if (p_peq_ctrl->peq_nvkey_id != 0) {
            DSP_MW_LOG_I("[PEQ_Set_Param] nvkeyID:0x%x,target_timestamp:%d\n", 2, p_peq_ctrl->peq_nvkey_id,p_peq_ctrl->target_timestamp);
        } else {
            DSP_MW_LOG_I("[PEQ_Set_Param] Disable PEQ\n", 0);
        }
    }
}

#if defined(AIR_ADAPTIVE_EQ_ENABLE) && defined(MTK_PEQ_ENABLE)
void Adaptive_Eq_Set_Share_Info(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);
    share_addr = (aeq_share_info_t *)hal_memview_mcu_to_dsp0(msg.ccni_message[1]);
}

/**
 * Adaptive_Eq_Set_Param
 *
 * This function is used to update adaptive eq parameter and nvkey ID.
 *
 *
 * @msg : ccni message with index MSG_MCU2DSP_COMMON_ADEQ_SET_PARAM.
 *
 */
void Adaptive_Eq_Set_Param(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    mcu2dsp_peq_param_p peq_param;
    DSP_ADEQ_CTRL_t *p_peq_ctrl;
    int status;
    uint8_t i, adaptive_eq_number;
    UNUSED(ack);

    adaptive_eq_number = (msg.ccni_message[0] & 0xFFFF);
    peq_param = (mcu2dsp_peq_param_p)hal_memview_mcu_to_dsp0(msg.ccni_message[1]);
    aeq_ctrl_g.aeq_number = adaptive_eq_number;

    for (i = 0; i < adaptive_eq_number; i++) {
        peq_param[i].nvkey_addr = (uint8_t *)hal_memview_mcu_to_dsp0((uint32_t)peq_param[i].nvkey_addr);
        if (i == 0) {
            p_peq_ctrl = &adaptive_eq1_param;
        } else if (i == 1) {
            p_peq_ctrl = &adaptive_eq2_param;
        } else if (i == 2) {
            p_peq_ctrl = &adaptive_eq3_param;
        } else if (i == 3) {
            p_peq_ctrl = &adaptive_eq4_param;
        } else if (i == 4) {
            p_peq_ctrl = &adaptive_eq5_param;
        } else if (i == 5) {
            p_peq_ctrl = &adaptive_eq6_param;
        } else if (i == 6) {
            p_peq_ctrl = &adaptive_eq7_param;
        } else {
            return;
        }

        if (MACRO_CHECK_AVAILABLE_NVKEY(peq_param[i].peq_nvkey_id) /*&& (p_peq_ctrl->peq_nvkey_id != peq_param->peq_nvkey_id)*/) {
            p_peq_ctrl->peq_nvkey_id = peq_param[i].peq_nvkey_id;
            if (peq_param[i].peq_nvkey_id != 0) {
                status = peq_get_coef_nvdm(p_peq_ctrl->peq_nvkey_id, peq_param[i].nvkey_addr, &p_peq_ctrl->peq_nvkey_param);
                if (status != 0) {
                    DSP_MW_LOG_E("[Adaptive_Peq_Set_Param] Get AEQ param error: status:%d  nvkeyID:%d \n", 2, status, p_peq_ctrl->peq_nvkey_id);
                    return;
                }
            }
        }
    }
}

void adaptive_eq_update_sound_mode()
{
    DSP_PEQ_CTRL_t *p_peq_ctrl;
    DSP_ADEQ_CTRL_t *p_peq_ctrl2;

    p_peq_ctrl2 = &adaptive_eq1_param;
    p_peq_ctrl = &peq_ctrl_adaptive_eq;

#ifdef AIR_VP_PEQ_ENABLE
    DSP_PEQ_CTRL_t *p_peq_ctrl_vp;
    p_peq_ctrl_vp = &peq_ctrl_vp_aeq;
#endif

    *(volatile uint32_t *)(share_addr) = aeq_ctrl_g.aeq_sonud_mode;
    adaptive_eq_notify_mcu_index(aeq_ctrl_g.aeq_sonud_mode);
    switch(aeq_ctrl_g.aeq_sonud_mode){
        case 1: p_peq_ctrl2 = &adaptive_eq1_param; break;
        case 2: p_peq_ctrl2 = &adaptive_eq2_param; break;
        case 3: p_peq_ctrl2 = &adaptive_eq3_param; break;
        case 4: p_peq_ctrl2 = &adaptive_eq4_param; break;
        case 5: p_peq_ctrl2 = &adaptive_eq5_param; break;
        case 6: p_peq_ctrl2 = &adaptive_eq6_param; break;
        case 7: p_peq_ctrl2 = &adaptive_eq7_param; break;
        default:
            DSP_MW_LOG_E("[AEQ_update_sound_mode]AEQ_update_sound_mode error: aeq_sonud_mode:%d  \n", 1,aeq_ctrl_g.aeq_sonud_mode);
        break;
    }
    if (MACRO_CHECK_AVAILABLE_NVKEY(p_peq_ctrl2->peq_nvkey_id)) {
        if (p_peq_ctrl2->peq_nvkey_id != 0) {
            p_peq_ctrl->peq_nvkey_id = p_peq_ctrl2->peq_nvkey_id;
            p_peq_ctrl->peq_nvkey_param = p_peq_ctrl2->peq_nvkey_param;
#ifdef AIR_VP_PEQ_ENABLE
            p_peq_ctrl_vp->peq_nvkey_id = p_peq_ctrl2->peq_nvkey_id;
            p_peq_ctrl_vp->peq_nvkey_param = p_peq_ctrl2->peq_nvkey_param;
#endif
        }
        p_peq_ctrl->need_update = NEED_UPDATE_NOW;
        p_peq_ctrl->enable = (p_peq_ctrl->peq_nvkey_id == 0) ? false : true;
#ifdef AIR_VP_PEQ_ENABLE
        p_peq_ctrl_vp->need_update = NEED_UPDATE_NOW;
        p_peq_ctrl_vp->enable = (p_peq_ctrl_vp->peq_nvkey_id == 0) ? false : true;
#endif
        if (p_peq_ctrl->peq_nvkey_id != 0) {
            aeq_ctrl_g.aeq_nvk = p_peq_ctrl2->peq_nvkey_id;
            DSP_MW_LOG_I("[AEQ_update_sound_mode] AEQ sound mode:%d nvkeyID:0x%x\n", 2,aeq_ctrl_g.aeq_sonud_mode,aeq_ctrl_g.aeq_nvk);
        } else {
            DSP_MW_LOG_I("[AEQ_update_sound_mode] AEQ sound error\n", 0);
        }
    }
}
#endif

#ifdef AIR_VP_PEQ_ENABLE
bool peq_copy_updated_param(DSP_PEQ_CTRL_t *p_src_peq_ctrl, DSP_PEQ_CTRL_t *p_dst_peq_ctrl)
{
    if ( peq_check_all_off(p_src_peq_ctrl) == 0 ) {
        p_dst_peq_ctrl->peq_nvkey_id = p_src_peq_ctrl->peq_nvkey_id;

        if (p_src_peq_ctrl->peq_nvkey_id != 0) {
            memset(&p_dst_peq_ctrl->peq_nvkey_param, 0, sizeof(DSP_PEQ_NVKEY_t));
            p_dst_peq_ctrl->peq_nvkey_param = p_src_peq_ctrl->peq_nvkey_param;
            p_dst_peq_ctrl->enable = p_src_peq_ctrl->enable;

            DSP_MW_LOG_I("[PEQ_Set_Param][COPY] nvkeyID:0x%x, update:%d, bt clk:%d, enable:%d\n", 4,
                                                                    p_dst_peq_ctrl->peq_nvkey_id,
                                                                    p_dst_peq_ctrl->need_update,
                                                                    p_dst_peq_ctrl->target_timestamp,
                                                                    p_dst_peq_ctrl->enable);
        } else {
            DSP_MW_LOG_I("[PEQ_Set_Param] Disable PEQ\n", 0);
        }
    }
    return false;
}

#endif

static bool peq_interface_init(void *para, DSP_PEQ_CTRL_t *p_peq_ctrl)
{
    if (p_peq_ctrl == &peq_ctrl_4) {
        p_peq_ctrl->sub_ctrl[0].peq_instance = (PEQ_ST *)((uint32_t)stream_function_get_working_buffer(para) + DSP_PEQ_MEMSIZE);
    } else {
        p_peq_ctrl->sub_ctrl[0].peq_instance = (PEQ_ST *)stream_function_get_working_buffer(para);
    }
    p_peq_ctrl->sub_ctrl[1].peq_instance = (PEQ_ST *)((U8 *)p_peq_ctrl->sub_ctrl[0].peq_instance + DSP_PEQ_INSTANCE_MEMSIZE);
    p_peq_ctrl->p_overlap_buffer = (S32 *)((U8 *)p_peq_ctrl->sub_ctrl[1].peq_instance + DSP_PEQ_INSTANCE_MEMSIZE);

    memset(p_peq_ctrl->sub_ctrl[0].peq_instance, 0, DSP_PEQ_INSTANCE_MEMSIZE);
    memset(p_peq_ctrl->sub_ctrl[1].peq_instance, 0, DSP_PEQ_INSTANCE_MEMSIZE);
    memset(p_peq_ctrl->p_overlap_buffer, 0, DSP_PEQ_OVERLAP_BUFSIZE);

#ifdef DSP_PEQ_SYNC_WITH_BT_CLOCK_PTS
#ifdef AIR_VP_PEQ_ENABLE
#ifdef AIR_ADAPTIVE_EQ_ENABLE
    // Do not modify sync status if init VP PEQ
    if ((p_peq_ctrl != &peq_ctrl_vp) || (p_peq_ctrl != &peq_ctrl_vp_2) || (p_peq_ctrl != &peq_ctrl_vp_aeq)) {
#else
    if ((p_peq_ctrl != &peq_ctrl_vp) || (p_peq_ctrl != &peq_ctrl_vp_2)) {
#endif
        peq_sync_ctrl.started = 0;
        peq_sync_ctrl.asi_cnt = 0;
    }
#else
    peq_sync_ctrl.started = 0;
    peq_sync_ctrl.asi_cnt = 0;
#endif
#endif
    p_peq_ctrl->p_peq_inter_param = NULL;
    p_peq_ctrl->trigger_drc = 0;
    p_peq_ctrl->sub_ctrl[0].status = PEQ_STATUS_OFF;
    p_peq_ctrl->sub_ctrl[1].status = PEQ_STATUS_OFF;
    p_peq_ctrl->sub_ctrl[0].overlap_progress = 0;
    p_peq_ctrl->sub_ctrl[1].overlap_progress = 0;

    if (p_peq_ctrl->need_update != NEED_UPDATE_NOW) { //for case: change sound mode before a2dp playing
        p_peq_ctrl->enable = (p_peq_ctrl->peq_nvkey_id == 0) ? false : true;
    }
    p_peq_ctrl->sample_rate = stream_function_get_samplingrate(para);
    p_peq_ctrl->need_update = NEED_UPDATE_NOW;
    return false;
}

/**
 * stream_function_peq_initialize
 *
 * This function is used to init memory space for PEQ function
 *
 *
 * @para : Default parameter of callback function
 *
 */
bool stream_function_peq_initialize(void *para)
{
    peq_ctrl.phase_id = 0;
    return peq_interface_init(para, &peq_ctrl);
}

bool stream_function_peq2_initialize(void *para)
{
    peq_ctrl_2.phase_id = 1;
    return peq_interface_init(para, &peq_ctrl_2);
}

bool stream_function_peq3_initialize(void *para)
{
    peq_ctrl_3.phase_id = 0;
    return peq_interface_init(para, &peq_ctrl_3);
}

bool stream_function_peq4_initialize(void *para)
{
    peq_ctrl_5.phase_id = 0;
    peq_interface_init(para, &peq_ctrl_5);
    peq_ctrl_4.phase_id = 1;
    return peq_interface_init(para, &peq_ctrl_4);
}

#if defined (AIR_WIRED_AUDIO_ENABLE)||(AIR_WIRELESS_MIC_TX_ENABLE)
bool stream_function_mic_peq_initialize(void *para)
{
    peq_ctrl_mic_peq.phase_id = 0;
    return peq_interface_init(para, &peq_ctrl_mic_peq);
}
#endif
bool stream_function_wired_usb_peq_initialize(void *para)
{
    peq_ctrl_usb_peq.phase_id = 0;
    return peq_interface_init(para, &peq_ctrl_usb_peq);
}

bool stream_function_advanced_record_peq_initialize(void *para)
{
    peq_ctrl_advanced_record_peq.phase_id = 0;
    return peq_interface_init(para, &peq_ctrl_advanced_record_peq);
}

bool stream_function_adaptive_eq_initialize(void *para)
{
    peq_ctrl.phase_id = 0;
    return peq_interface_init(para, &peq_ctrl_adaptive_eq);
}

#ifdef AIR_VP_PEQ_ENABLE
bool stream_function_vp_peq_initialize(void *para)
{
    peq_ctrl_vp.phase_id = 0;
    if (peq_ctrl.enable == 1) {
        peq_copy_updated_param(&peq_ctrl, &peq_ctrl_vp);
    }

    return peq_interface_init(para, &peq_ctrl_vp);
}

bool stream_function_vp_peq2_initialize(void *para)
{
    peq_ctrl_vp_2.phase_id = 1;
    if (peq_ctrl_2.enable == 1) {
        peq_copy_updated_param(&peq_ctrl_2, &peq_ctrl_vp_2);
    }
    return peq_interface_init(para, &peq_ctrl_vp_2);
}

#ifdef AIR_ADAPTIVE_EQ_ENABLE
bool stream_function_vp_aeq_initialize(void *para)
{
    peq_ctrl_vp_aeq.phase_id = 0;
    if ( aeq_ctrl_g.aeq_enable == 1 ) {
        peq_copy_updated_param(&peq_ctrl_adaptive_eq, &peq_ctrl_vp_aeq);
    }
    return peq_interface_init(para, &peq_ctrl_vp_aeq);
}
#endif
#endif
#ifdef AIR_HEARTHROUGH_VIVID_PT_ENABLE
bool stream_function_vivid_peq_initialize(void *para)
{
    peq_ctrl_vivid_pt.phase_id = 0;
    return peq_interface_init(para, &peq_ctrl_vivid_pt);
}
#endif

#ifdef MTK_DEQ_ENABLE
bool stream_function_deq_initialize(void *para)
{
    if (stream_function_get_device_channel_number(para) != 1) {
    }

    deq_ctrl.phase_id = 2;
    peq_interface_init(para, &deq_ctrl);
    memset(deq_ctrl.p_overlap_buffer + DSP_PEQ_OVERLAP_BUF_SAMPLES, 0, (DEQ_MAX_DELAY_SAMPLES << 2));
    return false;
}
bool stream_function_deq_mute_initialize(void *para)
{
    UNUSED(para);
    if (stream_function_get_device_channel_number(para) != 1) {
    }
    return false;
}
#endif

/**
 * stream_function_peq_process
 *
 * PEQ main process
 *
 *
 * @para : Default parameter of callback function
 *
 */
#if defined(AIR_HEARTHROUGH_VIVID_PT_ENABLE) || defined(AIR_LINE_IN_LATENCY_LOW) || defined(AIR_LINE_IN_LATENCY_MEDIUM) || defined(AIR_USB_IN_LATENCY_LOW)
ATTR_TEXT_IN_IRAM static bool peq_inferace_process(void *para, DSP_PEQ_CTRL_t *p_peq_ctrl)
#else
static bool peq_inferace_process(void *para, DSP_PEQ_CTRL_t *p_peq_ctrl)
#endif
{
    if((peq_control_off == 1) && (p_peq_ctrl != &peq_ctrl_adaptive_eq)){
        return false;
    }
    S32 *Buf1 = (S32 *)stream_function_get_1st_inout_buffer(para);
    S32 *Buf2 = (S32 *)stream_function_get_2nd_inout_buffer(para);
    S16 FrameSize = (S16)stream_function_get_output_size(para);
    // S16 dev_channels = (S16)stream_function_get_device_channel_number(para);
    U8 proc_resolution = (U8)stream_function_get_output_resolution(para);
    U8 rate = stream_function_get_samplingrate(para);
    S32 FrameSamples;

#ifdef DSP_PEQ_SYNC_WITH_BT_CLOCK_PTS
    if (((p_peq_ctrl->need_update == NEED_UPDATE_LATER) && ((S32)((p_peq_ctrl->target_timestamp << 15) - (peq_sync_ctrl.current_timestamp << 15)) <= 0))
        || (p_peq_ctrl->sample_rate != rate)) {
        if (p_peq_ctrl->sample_rate == rate) {
            if (peq_sync_ctrl.use_seqno == 1) {
                DSP_MW_LOG_I("[%d] update PEQ at ts: =============== %c 0x%x 0x%x %d ============== \n", 5, p_peq_ctrl->phase_id, ((peq_sync_ctrl.current_timestamp - p_peq_ctrl->target_timestamp) > 1) ? 'N' : 'Y', p_peq_ctrl->target_timestamp, peq_sync_ctrl.current_timestamp, (peq_sync_ctrl.current_timestamp - p_peq_ctrl->target_timestamp));
            } else {
                DSP_MW_LOG_I("[%d] update PEQ at ts: =============== %c 0x%x 0x%x %d ============== \n", 5, p_peq_ctrl->phase_id, ((peq_sync_ctrl.current_timestamp - p_peq_ctrl->target_timestamp) > 1024) ? 'N' : 'Y', p_peq_ctrl->target_timestamp, peq_sync_ctrl.current_timestamp, (peq_sync_ctrl.current_timestamp - p_peq_ctrl->target_timestamp));
            }
        }
        p_peq_ctrl->sample_rate = rate;
        p_peq_ctrl->need_update = NEED_UPDATE_NOW;
        p_peq_ctrl->enable = (p_peq_ctrl->peq_nvkey_id == 0) ? false : true;
    } else
#endif
    if ((p_peq_ctrl->trigger_drc == 1) && (p_peq_ctrl->need_update == NEED_UPDATE_NO)) {
        p_peq_ctrl->trigger_drc = 0;
    }

    if ((p_peq_ctrl->need_update == NEED_UPDATE_NOW) && (p_peq_ctrl->enable == true)) {
        p_peq_ctrl->trigger_drc = 1;
    }

    if (FrameSize == 0) {
        return false;
    } else if ((p_peq_ctrl->enable == false) && (p_peq_ctrl == &peq_ctrl_adaptive_eq)) {
        //trigger drc (disable) after peq ramp down finish.
        // if ((p_peq_ctrl->sub_ctrl[0].overlap_progress == 100) || (p_peq_ctrl->sub_ctrl[1].overlap_progress == 100)) {
        //     p_peq_ctrl->trigger_drc = 1;
        //     aud_adaptive_eq_drc_ctrl.enable = 0;
        //     p_peq_ctrl->sub_ctrl[0].overlap_progress = 0;
        //     p_peq_ctrl->sub_ctrl[1].overlap_progress = 0;
        // }
        aeq_control_off = 1;
        return false;
    }
    if (p_peq_ctrl->need_update == NEED_UPDATE_NOW) {
        /*-----------------------
                 There are legal three conditions:
                     OFF    OFF
                     OFF    ON
                     IN      OUT
               -----------------------*/
        DSP_PEQ_SUB_CTRL_t *available_sub_ctrl = peq_get_sub_ctrl_by_status(p_peq_ctrl, PEQ_STATUS_OFF);
        DSP_PEQ_SUB_CTRL_t *current_sub_ctrl = peq_get_sub_ctrl_by_status(p_peq_ctrl, PEQ_STATUS_ON);
        if (available_sub_ctrl == NULL) {
            available_sub_ctrl = peq_get_sub_ctrl_by_status(p_peq_ctrl, PEQ_STATUS_FADE_IN);
            current_sub_ctrl = peq_get_sub_ctrl_by_status(p_peq_ctrl, PEQ_STATUS_FADE_OUT);
        }
        if (available_sub_ctrl == NULL) {
            AUDIO_ASSERT(0 && "Get NULL available_sub_ctrl");
            return false;
        }

        p_peq_ctrl->need_update = NEED_UPDATE_NO;
        if (p_peq_ctrl->enable == false) {
            peq_get_defalut_param(p_peq_ctrl,rate);
        } else if (peq_get_inter_param(p_peq_ctrl, rate) == 0) {
            peq_get_defalut_param(p_peq_ctrl,rate);
        }
        peq_init(p_peq_ctrl->p_peq_inter_param, available_sub_ctrl->peq_instance);
#ifdef SUPPORT_SWITCH_COEF_OVERLAP
        available_sub_ctrl->overlap_progress = 1;
        available_sub_ctrl->status = PEQ_STATUS_FADE_IN;
        if (current_sub_ctrl == NULL) {
            current_sub_ctrl = peq_get_sub_ctrl_by_status(p_peq_ctrl, PEQ_STATUS_OFF);
            peq_init((S16 *)peq_allpass_coef_441kHz, current_sub_ctrl->peq_instance);
        }
        current_sub_ctrl->overlap_progress = PEQ_SWITCH_TOTAL_PERCENTAGE - 1;
        current_sub_ctrl->status = PEQ_STATUS_FADE_OUT;
#else
        available_sub_ctrl->status = PEQ_STATUS_ON;
        if (current_sub_ctrl != NULL) {
            current_sub_ctrl->status = PEQ_STATUS_OFF;
        }
#endif
        //peq_instance->output_gain = peq_instance->output_gain << 4; //temp for without DRC
        DSP_MW_LOG_I("[%d] peq_init to version:0x%x 0x%x, nvkeyID=0x%x, sample_rate=%d, output_gain=0x%x, status: %d %d\n", 8, p_peq_ctrl->phase_id, peq_version(),(uint32_t)available_sub_ctrl, p_peq_ctrl->peq_nvkey_id, rate, available_sub_ctrl->peq_instance->output_gain, p_peq_ctrl->sub_ctrl[0].status, p_peq_ctrl->sub_ctrl[1].status);
    }

    // if ((p_peq_ctrl->phase_id != 2) && (dev_channels == 1)) {
    //     Buf2 = NULL;
    // }

    if (p_peq_ctrl->proc_ch_mask == CH_MASK_L) {
        Buf2 = NULL;
    } else if (p_peq_ctrl->proc_ch_mask == CH_MASK_R) {
        Buf1 = NULL;
    }

    if (proc_resolution == RESOLUTION_16BIT) {
        FrameSamples = FrameSize >> 1;
        if (Buf1) {
            dsp_converter_16bit_to_32bit(Buf1, (S16 *)Buf1, FrameSamples);
        }
        if (Buf2) {
            dsp_converter_16bit_to_32bit(Buf2, (S16 *)Buf2, FrameSamples);
        }
        stream_function_modify_output_size(para, FrameSamples << 2);
        stream_function_modify_output_resolution(para, RESOLUTION_32BIT);
    } else {
        FrameSamples = FrameSize >> 2;
    }


    if ((p_peq_ctrl == &peq_ctrl)||(p_peq_ctrl == &peq_ctrl_3)||(p_peq_ctrl == &peq_ctrl_mic_peq)) {
#ifdef AIR_AUDIO_DUMP_ENABLE
        if (Buf1) {
            LOG_AUDIO_DUMP((U8 *)Buf1, (U32)(FrameSamples << 2), AUDIO_PEQ_IN_L);
        }
        if (Buf2) {
            LOG_AUDIO_DUMP((U8 *)Buf2, (U32)(FrameSamples << 2), AUDIO_PEQ_IN_R);
        }
#endif
    } else if ((p_peq_ctrl == &peq_ctrl_2)||(p_peq_ctrl == &peq_ctrl_advanced_record_peq)) {
#ifdef AIR_AUDIO_DUMP_ENABLE
        if (Buf1) {
            LOG_AUDIO_DUMP((U8 *)Buf1, (U32)(FrameSamples << 2), AUDIO_PEQ2_IN_L);
        }
        if (Buf2) {
            LOG_AUDIO_DUMP((U8 *)Buf2, (U32)(FrameSamples << 2), AUDIO_PEQ2_IN_R);
        }
#endif
    }


    if (Buf1 || Buf2) {
        if (p_peq_ctrl->sub_ctrl[0].status == PEQ_STATUS_ON) {
            peq_process(Buf1, Buf2, FrameSamples, p_peq_ctrl->sub_ctrl[0].peq_instance, p_peq_ctrl->peq_mode);
        } else if (p_peq_ctrl->sub_ctrl[1].status == PEQ_STATUS_ON) {
            peq_process(Buf1, Buf2, FrameSamples, p_peq_ctrl->sub_ctrl[1].peq_instance, p_peq_ctrl->peq_mode);
        } else if (((p_peq_ctrl->sub_ctrl[0].status == PEQ_STATUS_FADE_OUT) && (p_peq_ctrl->sub_ctrl[1].status == PEQ_STATUS_FADE_IN))
                   || ((p_peq_ctrl->sub_ctrl[1].status == PEQ_STATUS_FADE_OUT) && (p_peq_ctrl->sub_ctrl[0].status == PEQ_STATUS_FADE_IN))) {
            uint32_t fade_in = (p_peq_ctrl->sub_ctrl[1].status == PEQ_STATUS_FADE_IN) ? 1 : 0;
            uint32_t fade_out = (fade_in == 1) ? 0 : 1;
            S32 overlap_progress = (S32)p_peq_ctrl->sub_ctrl[fade_in].overlap_progress;
            if (Buf1) {
                overlap_progress = (S32)p_peq_ctrl->sub_ctrl[fade_in].overlap_progress;
                memcpy(p_peq_ctrl->p_overlap_buffer, Buf1, FrameSamples << 2);
                peq_process(p_peq_ctrl->p_overlap_buffer, 0, FrameSamples, p_peq_ctrl->sub_ctrl[fade_out].peq_instance, p_peq_ctrl->peq_mode);
                peq_process(Buf1, 0, FrameSamples, p_peq_ctrl->sub_ctrl[fade_in].peq_instance, p_peq_ctrl->peq_mode);
                peq_overlap(Buf1, p_peq_ctrl->p_overlap_buffer, &overlap_progress, FrameSamples);
            }

            if (Buf2) {
                overlap_progress = (S32)p_peq_ctrl->sub_ctrl[fade_in].overlap_progress;
                memcpy(p_peq_ctrl->p_overlap_buffer, Buf2, FrameSamples << 2);
                peq_process(0, p_peq_ctrl->p_overlap_buffer, FrameSamples, p_peq_ctrl->sub_ctrl[fade_out].peq_instance, p_peq_ctrl->peq_mode);
                peq_process(0, Buf2, FrameSamples, p_peq_ctrl->sub_ctrl[fade_in].peq_instance, p_peq_ctrl->peq_mode);
                peq_overlap(Buf2, p_peq_ctrl->p_overlap_buffer, &overlap_progress, FrameSamples);
            }
            if(p_peq_ctrl == &peq_ctrl_adaptive_eq){
                aud_adaptive_eq_drc_ctrl.enable = 1;
            }
            //DSP_MW_LOG_I("PEQ mixing progress: %d (in/out:%d/%d)\n",3,overlap_progress,fade_in,fade_out);
            p_peq_ctrl->sub_ctrl[fade_in].overlap_progress = (S16)overlap_progress;
            p_peq_ctrl->sub_ctrl[fade_out].overlap_progress = (S16)(PEQ_SWITCH_TOTAL_PERCENTAGE - overlap_progress);
            if (overlap_progress >= PEQ_SWITCH_TOTAL_PERCENTAGE) {
                p_peq_ctrl->sub_ctrl[fade_in].status = PEQ_STATUS_ON;
                p_peq_ctrl->sub_ctrl[fade_out].status = PEQ_STATUS_OFF;
                DSP_MW_LOG_I("peq mxing finish, status: %d %d\n", 2, p_peq_ctrl->sub_ctrl[0].status, p_peq_ctrl->sub_ctrl[1].status);
#ifdef AIR_ADAPTIVE_EQ_ENABLE
                if (p_peq_ctrl == &peq_ctrl_adaptive_eq) {
                    if(aeq_ctrl_g.aeq_update){
                    aeq_ctrl_g.aeq_enable = 1;
                    aeq_control_off = 0;
                    }
                    DSP_MW_LOG_I("aeq mxing finish, enable:%d,aeq_update: %d\n", 2, aeq_ctrl_g.aeq_enable, aeq_ctrl_g.aeq_update);
                }
#endif
            }
        }
    } else {
        DSP_MW_LOG_W("PEQ Buf1 & Buf2 are both NULL, proc_ch:%d out_ch_num:%d \n", 2, p_peq_ctrl->proc_ch_mask, stream_function_get_channel_number(para));
    }

    if ((p_peq_ctrl == &peq_ctrl)||(p_peq_ctrl == &peq_ctrl_3)||(p_peq_ctrl == &peq_ctrl_mic_peq)) {
#ifdef AIR_AUDIO_DUMP_ENABLE
        if (Buf1) {
            LOG_AUDIO_DUMP((U8 *)Buf1, (U32)(FrameSamples << 2), AUDIO_PEQ_OUT_L);
        }
        if (Buf2) {
            LOG_AUDIO_DUMP((U8 *)Buf2, (U32)(FrameSamples << 2), AUDIO_PEQ_OUT_R);
        }
#endif
    } else if ((p_peq_ctrl == &peq_ctrl_2)||(p_peq_ctrl == &peq_ctrl_advanced_record_peq)) {
#ifdef AIR_AUDIO_DUMP_ENABLE
        if (Buf1) {
            LOG_AUDIO_DUMP((U8 *)Buf1, (U32)(FrameSamples << 2), AUDIO_PEQ2_OUT_L);
        }
        if (Buf2) {
            LOG_AUDIO_DUMP((U8 *)Buf2, (U32)(FrameSamples << 2), AUDIO_PEQ2_OUT_R);
        }
#endif
    }


    return false;
}

bool stream_function_peq_process(void *para)
{
#ifdef DSP_PEQ_SYNC_WITH_BT_CLOCK_PTS
    if (((DSP_ENTRY_PARA_PTR)para)->number.field.source_type == SOURCE_TYPE_A2DP) {
        S16 FrameSize = (S16)stream_function_get_output_size(para);
        U8 proc_resolution = (U8)stream_function_get_output_resolution(para);
        CLK_SKEW_FS_t fs = clk_skew_fs_converter((stream_samplerate_t)(U8)stream_function_get_samplingrate(para));

        peq_sync_ctrl.asi_cnt += (proc_resolution == RESOLUTION_16BIT) ? (FrameSize >> 1) : (FrameSize >> 2);
        //printf("[asi]%d [pts]0x%x [thr]%d",p_peq_ctrl->asi_cnt,p_peq_ctrl->current_timestamp,lt_clk_skew_get_asi_threshold());
        if (peq_sync_ctrl.asi_cnt >= lt_clk_skew_get_asi_threshold((int) fs)) {
            peq_sync_ctrl.asi_cnt -= lt_clk_skew_get_asi_threshold((int) fs);
            peq_sync_ctrl.anchor_bt_clk = peq_sync_ctrl.prepare_bt_clk;
            peq_sync_ctrl.anchor_timestamp = peq_calculate_anchor_timestamp(lt_clk_skew_get_asi_threshold((int) fs), peq_sync_ctrl.anchor_timestamp); //p_peq_ctrl->current_timestamp;
            // DSP_MW_LOG_I("updated peq anchor_timestamp: 0x%x anchor_clk: 0x%x asi_cnt: 0x%x\n", 3, peq_sync_ctrl.anchor_timestamp, peq_sync_ctrl.anchor_bt_clk, peq_sync_ctrl.asi_cnt);
        }
    }
#endif

    return peq_inferace_process(para, &peq_ctrl);
}

bool stream_function_peq2_process(void *para)
{
    return peq_inferace_process(para, &peq_ctrl_2);
}
bool stream_function_peq3_process(void *para)
{
    return peq_inferace_process(para, &peq_ctrl_3);
}
bool stream_function_peq4_process(void *para)
{
    peq_inferace_process(para, &peq_ctrl_5);
    return peq_inferace_process(para, &peq_ctrl_4);
}

#if defined (AIR_WIRED_AUDIO_ENABLE)||(AIR_WIRELESS_MIC_TX_ENABLE)
bool stream_function_mic_peq_process(void *para)
{
    return peq_inferace_process(para, &peq_ctrl_mic_peq);
}
#endif
bool stream_function_wired_usb_peq_process(void *para)
{
    return peq_inferace_process(para, &peq_ctrl_usb_peq);
}

bool stream_function_advanced_record_peq_process(void *para)
{
    return peq_inferace_process(para, &peq_ctrl_advanced_record_peq);
}

#ifdef AIR_ADAPTIVE_EQ_ENABLE
bool stream_function_adaptive_eq_process(void *para)
{
    return peq_inferace_process(para, &peq_ctrl_adaptive_eq);
}
#endif

bool stream_function_instant_peq_process(void *para)
{
    if (peq_ctrl.need_update == NEED_UPDATE_LATER) {
        (peq_ctrl.need_update = NEED_UPDATE_NOW);
    }
    return peq_inferace_process(para, &peq_ctrl);
}

#ifdef AIR_VP_PEQ_ENABLE
bool stream_function_vp_peq_process(void *para)
{
    return peq_inferace_process(para, &peq_ctrl_vp);
}

bool stream_function_vp_peq2_process(void *para)
{
    return peq_inferace_process(para, &peq_ctrl_vp_2);
}
#ifdef AIR_ADAPTIVE_EQ_ENABLE
bool stream_function_vp_aeq_process(void *para)
{
    if (peq_check_all_off(&peq_ctrl_adaptive_eq)) {
        /* AEQ bypass */
        return false;
    } else {
        return peq_inferace_process(para, &peq_ctrl_vp_aeq);
    }
}
#endif
#endif
#ifdef AIR_HEARTHROUGH_VIVID_PT_ENABLE
ATTR_TEXT_IN_IRAM bool stream_function_vivid_peq_process(void *para)
{
    return peq_inferace_process(para, &peq_ctrl_vivid_pt);
}
#endif

#ifdef MTK_DEQ_ENABLE
void dsp_deq_set_param(hal_ccni_message_t msg, hal_ccni_message_t *ack)
{
    UNUSED(ack);

    if ((msg.ccni_message[0] & 0xFFFF) == 0) {
        int16_t deq_digital_gain = (int16_t)(msg.ccni_message[1] & 0xFFFF);
        g_deq_digital_gain  = afe_calculate_digital_gain_index((uint32_t)deq_digital_gain, DEQ_0DB_REGISTER_VALUE);
        g_deq_delay_samples = (uint8_t)((msg.ccni_message[1] >> 16) & 0xFF);
        g_earbuds_ch        = (uint8_t)((msg.ccni_message[1] >> 24) & 0x3);
        g_deq_phase_inverse = (uint8_t)((msg.ccni_message[1] >> 26) & 0x1);
    } else {
        g_earbuds_ch        = (uint8_t)((msg.ccni_message[1] >> 24) & 0x3);
    }
    //deq_ctrl.proc_ch_mask = CH_MASK_R; // for DAC L and DAC R
    //deq_ctrl.proc_ch_mask = (g_earbuds_ch == 2) ? CH_MASK_L : CH_MASK_R; // for DAC DUAL
}

static void deq_apply_gain(int32_t *inbuf, int32_t *outbuf, uint32_t samples, uint32_t gain, uint8_t phase_inverse)
{
    //inbuf: Q31
    //outbuf: Q31
    //gain: Q15, inverse
    //MIPS: 0.573M
    int32_t i;
    int64_t temp64;
    int32_t temp32;
    if (phase_inverse == 0) {
        for (i = samples - 1; i >= 0; i--) {
#if 1 // with saturate protection
            temp64 = (int64_t)((int64_t)inbuf[i] * (int64_t)gain);
            temp32 = (int32_t)(temp64 >> 15);
            temp64 >>= 46;
            if ((temp64 == 0) || (temp64 == -1)) {
                outbuf[i] = temp32;
            } else {
                outbuf[i] = (temp64 < 0) ? 0x80000000 : 0x7FFFFFFF;
            }
#else
            outbuf[i] = (int32_t)(((int64_t)((int64_t)inbuf[i] * (int64_t)gain)) >> 15);
#endif
        }
    } else {
        for (i = samples - 1; i >= 0; i--) {
#if 1 // with saturate protection
            temp64 = -(int64_t)((int64_t)inbuf[i] * (int64_t)gain);
            temp32 = (int32_t)(temp64 >> 15);
            temp64 >>= 46;
            if ((temp64 == 0) || (temp64 == -1)) {
                outbuf[i] = temp32;
            } else {
                outbuf[i] = (temp64 < 0) ? 0x80000000 : 0x7FFFFFFF;
            }
#else
            outbuf[i] = -(int32_t)(((int64_t)((int64_t)inbuf[i] * (int64_t)gain)) >> 15);
#endif
        }
    }
}

bool stream_function_deq_process(void *para)
{
    DSP_PEQ_CTRL_t *p_deq_ctrl = &deq_ctrl;
    uint8_t enable = ((p_deq_ctrl->enable == true) || (peq_check_all_off(p_deq_ctrl) == 0)) ? 1 : 0;
    S16 FrameSize = (S16)stream_function_get_output_size(para);
    U8 proc_res = (U8)stream_function_get_output_resolution(para);
    S32 FrameSamples = (proc_res == RESOLUTION_16BIT) ? (FrameSize >> 1) : (FrameSize >> 2);
    S32 *SrcBuf, *DesBuf;

    deq_ctrl.proc_ch_mask = CH_MASK_R;

    if (deq_ctrl.proc_ch_mask == CH_MASK_L) { //this device is R ch, use L ch as anc reference ch
        DesBuf = (S32 *)stream_function_get_1st_inout_buffer(para);
        SrcBuf = (S32 *)stream_function_get_2nd_inout_buffer(para);
    } else {
        SrcBuf = (S32 *)stream_function_get_1st_inout_buffer(para);
        DesBuf = (S32 *)stream_function_get_2nd_inout_buffer(para);
    }
    if ((DesBuf != NULL) && (SrcBuf != NULL)) {
        memcpy(DesBuf, SrcBuf, FrameSize);
    }

    if (enable == 1) {
        if ((DesBuf != NULL) && (SrcBuf != NULL)) {
            if (proc_res == RESOLUTION_16BIT) {
                dsp_converter_16bit_to_32bit(DesBuf, (S16 *)DesBuf, FrameSamples);
                stream_function_modify_output_size(para, FrameSamples << 2);
                stream_function_modify_output_resolution(para, RESOLUTION_32BIT);
            }
            FrameSize = (S16)stream_function_get_output_size(para);
            if (g_deq_delay_samples == 0) {
                deq_apply_gain(DesBuf, DesBuf, FrameSamples, g_deq_digital_gain, g_deq_phase_inverse);
            } else {
                U32 offset = (DSP_PEQ_OVERLAP_BUF_SAMPLES - FrameSamples); //only support 32bits resolution
                U32 delay_samples = (U32)g_deq_delay_samples;
                S32 *buffer = p_deq_ctrl->p_overlap_buffer;
                memcpy(buffer + offset, buffer + DSP_PEQ_OVERLAP_BUF_SAMPLES, (delay_samples << 2));
                deq_apply_gain(DesBuf, (buffer + offset + delay_samples), FrameSamples, g_deq_digital_gain, g_deq_phase_inverse);
                memcpy(DesBuf, buffer + offset, FrameSize);
            }
        }
    }

    peq_inferace_process(para, p_deq_ctrl);

    if (enable == 1) {
        if (DesBuf != NULL) {
            if (proc_res == RESOLUTION_16BIT) {
                dsp_converter_32bit_to_16bit((S16 *)DesBuf, DesBuf, FrameSamples);
                stream_function_modify_output_size(para, FrameSamples << 1);
                stream_function_modify_output_resolution(para, RESOLUTION_16BIT);
            }
        }
    }
#if (DEQ_DEBUG == 1)
    if (deq_mute_ch & 0x1) {
        S16 FrameSize = (S16)stream_function_get_output_size(para);
        memset(stream_function_get_1st_inout_buffer(para), 0, FrameSize);
    }
    if (deq_mute_ch & 0x2) {
        S16 FrameSize = (S16)stream_function_get_output_size(para);
        memset(stream_function_get_2nd_inout_buffer(para), 0, FrameSize);
    }
#endif
    return false;
}

bool stream_function_deq_mute_process(void *para)
{
    deq_ctrl.proc_ch_mask = CH_MASK_R;//mute which channel
    if (deq_ctrl.proc_ch_mask & CH_MASK_L) {
        S16 FrameSize = (S16)stream_function_get_output_size(para);
        memset(stream_function_get_1st_inout_buffer(para), 0, FrameSize);
    }
    if (deq_ctrl.proc_ch_mask & CH_MASK_R) {
        S16 FrameSize = (S16)stream_function_get_output_size(para);
        memset(stream_function_get_2nd_inout_buffer(para), 0, FrameSize);
    }
#if (DEQ_DEBUG == 1)
    if (deq_mute_ch & 0x1) {
        S16 FrameSize = (S16)stream_function_get_output_size(para);
        memset(stream_function_get_1st_inout_buffer(para), 0, FrameSize);
    }
    if (deq_mute_ch & 0x2) {
        S16 FrameSize = (S16)stream_function_get_output_size(para);
        memset(stream_function_get_2nd_inout_buffer(para), 0, FrameSize);
    }
#endif
    return false;
}
#endif

#ifdef PRELOADER_ENABLE
BOOL PEQ_Open(VOID *para)
{
    UNUSED(para);
    return true;
}

BOOL PEQ_Close(VOID *para)
{
    UNUSED(para);
    return true;
}
#endif


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
#ifndef __VOICE_NR_DRIVER_H__
#define __VOICE_NR_DRIVER_H__

#include "types.h"

/* Predefine the size of working buffer
 * 3RD NR + Inhouse EC/PostEC   (55360(IGO 1+1) + 32840) byte
 * Inhouse EC                               () byte
 * 3RD NR + Inhouse PostEC          (IGO_NR_MEMSIZE + ) byte
 * Inhouse ECNR
 *      Inhouse 1/2 MIC             67296 byte
 *      Inhouse 1+1 MIC             67296 byte
 *      Inhouse 2+1 MIC             67296 byte
 */
#if defined(AIR_BTA_IC_PREMIUM_G2)
#if defined(AIR_AI_NR_PREMIUM_INEAR_ENABLE)
#define VOICE_NR_MEMSIZE    88200
#else
#define VOICE_NR_MEMSIZE    69000
#endif
#elif defined(AIR_BTA_IC_PREMIUM_G3)
#define VOICE_NR_MEMSIZE    88200
#endif

typedef enum {
    VOICE_ECNR_MIC_MODE_1,
    VOICE_ECNR_MIC_MODE_2,
    VOICE_ECNR_MIC_MODE_1_1,
    VOICE_ECNR_MIC_MODE_2_1,
    VOICE_ECNR_MIC_MODE_MAX,
} voice_ecnr_mic_mode_t;

typedef enum {
    VOICE_ECNR_STATUS_OK,
    VOICE_ECNR_STATUS_BYPASS,
    VOICE_ECNR_STATUS_ERROR,
    VOICE_ECNR_STATUS_MAX,
} voice_ecnr_status_t;

void voice_ecnr_set_mic_mode(voice_ecnr_mic_mode_t mode);
void voice_ecnr_check_txnr_lib_match(void *para);

#if defined(AIR_3RD_PARTY_NR_ENABLE)
#if defined(AIR_ECNR_PREV_PART_ENABLE)
#ifndef AIR_ECNR_SEPARATE_MODE_ENABLE
bool voice_ecnr_ec_config_period(uint32_t period);
bool voice_ecnr_ec_check_running_status(void);
voice_ecnr_status_t voice_ecnr_ec_init(void *para);
voice_ecnr_status_t voice_ecnr_ec_deinit(void);
voice_ecnr_status_t voice_ecnr_ec_process(void *para, uint32_t length, int16_t *main_mic, int16_t *ff_mic, int16_t *fb_mic, int16_t *ec_in, int16_t *out);
uint32_t voice_ecnr_ec_get_ref_gain(int16_t *gain_buf);
uint8_t voice_ecnr_ec_get_postec_gain(void);
#endif
#endif
#endif

#if defined(AIR_3RD_PARTY_NR_ENABLE)
bool voice_ecnr_nr_get_framework_workingbuffer(int16_t** NR_out_buf);
bool voice_ecnr_nr_check_running_status(void);
voice_ecnr_status_t voice_ecnr_nr_init(void *para);
voice_ecnr_status_t voice_ecnr_nr_deinit(void);
voice_ecnr_status_t voice_ecnr_nr_process(void *para, uint32_t length, int16_t *main_mic, int16_t *ff_mic, int16_t *fb_mic, int16_t *ref, int16_t *out);
uint32_t voice_ecnr_nr_query_version(uint8_t *version);
void voice_ecnr_nr_set_nr_level(uint32_t nr_level);
#endif

#if defined(AIR_3RD_PARTY_NR_ENABLE)
#if defined(AIR_ECNR_POST_PART_ENABLE)
#ifndef AIR_ECNR_SEPARATE_MODE_ENABLE
voice_ecnr_status_t voice_ecnr_postec_init(void *para);
voice_ecnr_status_t voice_ecnr_postec_deinit(void);
voice_ecnr_status_t voice_ecnr_postec_process(void *para, uint32_t length, int16_t *in, int16_t *out);
#endif
#endif
#endif

#if defined(AIR_3RD_PARTY_NR_ENABLE) && !defined(AIR_BTA_IC_STEREO_HIGH_G3)
bool voice_ecnr_tx_eq_check_running_status(void);
voice_ecnr_status_t voice_ecnr_tx_eq_init(void *para);
voice_ecnr_status_t voice_ecnr_tx_eq_deinit(void);
voice_ecnr_status_t voice_ecnr_tx_eq_process(void *para, uint32_t length, int16_t *in_out);
voice_ecnr_status_t voice_RX_EQ_update(uint32_t *p_rx_eq);
#endif

#if defined(AIR_3RD_PARTY_NR_ENABLE)
bool voice_ecnr_ec_postec_get_framework_workingbuffer(int16_t** EC_out1_buf, int16_t** EC_out2_buf, int16_t** EC_fb_buf);
bool voice_ecnr_ec_postec_check_running_status(void);
voice_ecnr_status_t voice_ecnr_ec_postec_init(void *para);
voice_ecnr_status_t voice_ecnr_ec_postec_deinit(void);
voice_ecnr_status_t voice_ecnr_ec_postec_ec_process(void *para, uint32_t length, int16_t *main_mic, int16_t *ff_mic, int16_t *fb_mic, int16_t *ec_in, int16_t *main_out, int16_t *ff_out, int16_t *fb_out);
voice_ecnr_status_t voice_ecnr_ec_postec_process(void *para, uint32_t length, int16_t *in, int16_t *out);
uint32_t voice_ecnr_ec_postec_get_ref_gain(int16_t *gain_buf);
#endif

#if defined(AIR_ECNR_1MIC_INEAR_ENABLE) || defined(AIR_ECNR_2MIC_INEAR_ENABLE) || defined(AIR_ECNR_1_OR_2_MIC_ENABLE)
bool voice_ecnr_all_get_framework_workingbuffer(int16_t** out_buf);
bool voice_ecnr_all_check_running_status(void);
voice_ecnr_status_t voice_ecnr_all_init(void *para);
voice_ecnr_status_t voice_ecnr_all_deinit(void);
voice_ecnr_status_t voice_ecnr_all_process(void *para, uint32_t length, int16_t *main_mic, int16_t *ff_mic, int16_t *fb_mic, int16_t *ec_in, int16_t *out);
uint32_t voice_ecnr_all_get_ref_gain(int16_t *gain_buf);
#endif

voice_ecnr_status_t voice_rxnr_process(void *para, uint32_t length, int16_t *in_out);

#endif /* __VOICE_NR_DRIVER_H__ */

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

#include "leakage_compensation.h"
//#include "dsp_feature_interface.h"
#include "hal_define.h"
#include "dsp_callback.h"
#include "dsp_dump.h"
#include "dsp_audio_msg_define.h"
#include "dsp_temp.h"
#include <assert.h>
#include "dsp_memory.h"
#include "dsp_audio_msg.h"

//#define LEAKAGE_REPORT_THD  (6000000)
//#define LEAKAGE_DEBUG_NO_RESPONSE_THD  (7000000)
#define LEAKAGE_DEBUG_PERIOD  (1000000)
uint32_t leakage_report_thd = 6000000;
uint32_t leakage_debug_no_response_thd = 7000000;


static bool wz_report_flag = false;
static uint16_t leakage_result_cnt = 0;//8byte cnt + 8byte result
uint32_t leakage_report_start_count = 0;
uint32_t leakage_report_period_start_count = 0;
unsigned long lc_max_count = 0;
bool anc_leakage_compensation_src_usage_flag = false;


S16 NR[240];
S32 TABLE_LOG2_1530[65] = {
    0x80000000, -2099449135, -2052148003, -2005558192, -1959658627, -1914429152, -1869850483, -1825904158,
    -1782572486, -1739838512, -1697685970, -1656099252, -1615063367, -1574563914, -1534587050, -1495119459,
    -1456148328, -1417661324, -1379646565, -1342092602, -1304988398, -1268323307, -1232087058, -1196269734,
    -1160861760, -1125853884, -1091237166, -1057002962, -1023142909, -989648917, -956513158, -923728047,
    -891286243, -859180629, -827404309, -795950598, -764813009, -733985252, -703461222, -673234992,
    -643300807, -613653078, -584286375, -555195419, -526375081, -497820372, -469526440, -441488565,
    -413702155, -386162738, -358865962, -331807589, -304983491, -278389645, -252022132, -225877133,
    -199950923, -174239871, -148740435, -123449160, -98362674, -73477689, -48790993, -24299450,
    0,
};



#define ENDIAN_RVRS(A)                  (A)
DSP_PARA_LD_STRU ld_NvKey;


void leakage_detection_set_duration(uint32_t report_thd, uint32_t no_response_thd)
{
    if ((leakage_report_thd != report_thd) || (leakage_debug_no_response_thd != no_response_thd)) {
        leakage_report_thd = report_thd;
        leakage_debug_no_response_thd = no_response_thd;
        DSP_MW_LOG_I("[RECORD_LC]leakage_detection_set_duration, leakage_report_thd:%d, leakage_debug_no_response_thd:%d", 2, leakage_report_thd, leakage_debug_no_response_thd);
    }
}

void leakage_detection_load_nvkey(void *nvkey)
{
    memcpy(&ld_NvKey, nvkey, sizeof(DSP_PARA_LD_STRU));
}

bool stream_function_leakage_compensation_initialize(void *para)
{
    void *p_working_memory = NULL;
    lc_max_count = 0;
    wz_report_flag = false;
    leakage_result_cnt = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &leakage_report_start_count);
    leakage_report_period_start_count = leakage_report_start_count;
    int mem_size = ld_get_aec_nr_memsize();

    if ((uint32_t)mem_size > stream_codec_get_working_buffer_length(para)) {
        AUDIO_ASSERT(0 && "[RECORD_LC]mem size is insufficient.");
    }

    p_working_memory = stream_function_get_working_buffer(para);
    if (p_working_memory == NULL) {
        AUDIO_ASSERT(0 && "[Leakage detection] working_buffer allocate fail");
    }

    DSP_MW_LOG_I("[RECORD_LC]LD_DspParaInit params\
                   ENABLE=0x%x, ld_thrd=0x%x, AEC_MU_FAC=0x%x,\
                   AEC_ENG_VAD_THR=0x%x, AEC_ENG_VAD_HANG=0x%x, AEC_ENG_POW_INIT=0x%x,\
                   AEC_slow_up_alpha=0x%x, AEC_slow_dn_alpha=0x%x, \
                   AEC_fast_up_alpha=0x%x, AEC_fast_dn_alpha=0x%x,\
                   RXIN_TXREF_DELAY=0x%x, MIC_EC_DELAY=0x%x, AEC_MU_MIN=0x%x,\
                   DIGITAL_GAIN=0x%x, LPF_en=0x%x, tone_thr:0x%x",
                    16,
                    ld_NvKey.ENABLE,
                    ld_NvKey.ld_thrd,
                    ld_NvKey.AEC_MU_FAC,
                    ld_NvKey.AEC_ENG_VAD_THR,
                    ld_NvKey.AEC_ENG_VAD_HANG,
                    ld_NvKey.AEC_ENG_POW_INIT,
                    ld_NvKey.AEC_slow_up_alpha,
                    ld_NvKey.AEC_slow_dn_alpha,
                    ld_NvKey.AEC_fast_up_alpha,
                    ld_NvKey.AEC_fast_dn_alpha,
                    ld_NvKey.RXIN_TXREF_DELAY,
                    ld_NvKey.MIC_EC_DELAY,
                    ld_NvKey.AEC_MU_MIN,
                    ld_NvKey.DIGITAL_GAIN,
                    ld_NvKey.LPF_en,
                    ld_NvKey.tone_thr);

        LeakageDetect_Init(0, NULL, p_working_memory, (void*)&ld_NvKey);

    return 0;
}

bool stream_function_leakage_compensation_process(void *para)
{
    uint32_t start_count = 0;
    uint32_t end_count = 0;
    uint32_t count = 0;
    uint32_t leakage_report_cur_count = 0;
    uint32_t leakage_report_count = 0;
    uint32_t leakage_report_period_count = 0;
    hal_ccni_message_t msg;
    uint16_t output_size = stream_function_get_output_size(para);
    uint32_t channel_number = stream_function_get_channel_number(para);
    S16 *EC_path_buf = (channel_number == 2) ? (S16 *)stream_function_get_2nd_inout_buffer(para) : (S16 *)stream_function_get_3rd_inout_buffer(para);
    S16 *InBuf     = (S16 *)stream_function_get_1st_inout_buffer(para);
    S32 ld_iwxe1_power, ld_aecref1_power;
    S16 fe_vad_cntr;

    if (!wz_report_flag) {
#ifdef AIR_AUDIO_DUMP_ENABLE
        LOG_AUDIO_DUMP((U8 *)InBuf, (U32)output_size, SOURCE_IN3);
        LOG_AUDIO_DUMP((U8 *)EC_path_buf, (U32)output_size, SOURCE_IN4);
#else
        UNUSED(output_size);
#endif

        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &start_count);
        LeakageDetect_Prcs(InBuf, EC_path_buf, NR, &ld_iwxe1_power, &ld_aecref1_power, &fe_vad_cntr);
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &end_count);
        hal_gpt_get_duration_count(start_count, end_count, &count);
        lc_max_count = (count > lc_max_count) ? count : lc_max_count;

        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &leakage_report_cur_count);
        hal_gpt_get_duration_count(leakage_report_start_count, leakage_report_cur_count, &leakage_report_count);
        hal_gpt_get_duration_count(leakage_report_period_start_count, leakage_report_cur_count, &leakage_report_period_count);
    }

    // >THD over 3 frames --> PASS, otherwise FAIL
    if ((!wz_report_flag) && (leakage_report_count > leakage_report_thd)) {
        uint8_t report_cnt = (leakage_result_cnt >> 8);
        uint8_t report_res = (leakage_result_cnt & 0xFF);
        S8 res_flag = 0;
        double mean_power = 0;
        //Debug
//        DSP_MW_LOG_I("[RECORD_LC]report_cnt:%d, report_res:%d", 2, report_cnt, report_res);

        report_cnt++;
        if (ld_aecref1_power != 0) {
            mean_power = (double)(ld_iwxe1_power) / (double)(ld_aecref1_power);
            if (mean_power > (double)(ld_NvKey.ld_thrd)/32768) {
                //PASS
                report_res++;
            } else {
                //FAIL
                res_flag = -1;
            }

        } else {
            //FAIL
            res_flag = -2;
        }
        DSP_MW_LOG_I("[RECORD_LC]frame %d pwr:%d ->res:%d(0=PASS) (ld_iwxe1_power:%d, ld_aecref1_power:%d, fe_vad_cntr:%d)", 6, report_cnt, (int)(mean_power*32768), res_flag, ld_iwxe1_power, ld_aecref1_power, fe_vad_cntr);

        if (report_cnt >= 3) {
            if (report_res >= 3) {//third consecutive PASS
                msg.ccni_message[0] = (MSG_DSP2MCU_RECORD_LC_WZ_REPORT << 16);
                msg.ccni_message[1] = LD_STATUS_PASS;

            } else {
                msg.ccni_message[0] = (MSG_DSP2MCU_RECORD_LC_WZ_REPORT << 16);
                msg.ccni_message[1] = LD_STATUS_FAIL_CASE_1;
            }
            // DSP_MW_LOG_I("[RECORD_LC]DSP send ccni, ld_iwxe1_power:%d, ld_aecref1_power:%d, fe_vad_cntr:%d, result:%d, max_algo_duration=%d, leakage_report_count:%d us \r\n", 6,
            //              ld_iwxe1_power, ld_aecref1_power, fe_vad_cntr, msg.ccni_message[1], lc_max_count, leakage_report_count);
            aud_msg_tx_handler(msg, 0, false);
            wz_report_flag = true;
            return 0;
        } else {
            leakage_result_cnt = (report_cnt << 8) | (report_res & 0xFF);
            //Debug
            //DSP_MW_LOG_I("[RECORD_LC]report_cnt:%d, report_res:%d, leakage_result_cnt:%d", 3, report_cnt, report_res, leakage_result_cnt);
        }
    }

    if ((!wz_report_flag) && (leakage_report_count > leakage_debug_no_response_thd)) {
        DSP_MW_LOG_E("[RECORD_LC] No response within %d us, ld_iwxe1_power:%d, ld_aecref1_power:%d, fe_vad_cntr:%d, result:%d, max_algo_duration=%d, leakage_report_count:%d us", 7,
                     leakage_debug_no_response_thd, ld_iwxe1_power, ld_aecref1_power, fe_vad_cntr, LD_STATUS_TIMEOUT, lc_max_count, leakage_report_count);
        msg.ccni_message[0] = (MSG_DSP2MCU_RECORD_LC_WZ_REPORT << 16);
        msg.ccni_message[1] = LD_STATUS_TIMEOUT;
        aud_msg_tx_handler(msg, 0, false);
        wz_report_flag = true;
    } else if ((!wz_report_flag) && (leakage_report_period_count > LEAKAGE_DEBUG_PERIOD)) {
        hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_1M, &leakage_report_period_start_count);
        DSP_MW_LOG_I("[RECORD_LC]DSP debug, ld_iwxe1_power:%d, ld_aecref1_power:%d, fe_vad_cntr:%d, max_algo_duration=%d, leakage_report_count:%d us \r\n", 5,
                     ld_iwxe1_power, ld_aecref1_power, fe_vad_cntr, lc_max_count, leakage_report_count);
    }

    return 0;
}


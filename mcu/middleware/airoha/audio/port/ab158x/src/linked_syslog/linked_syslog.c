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

#include "syslog.h"
#include <stdint.h>
#include <string.h>

#ifdef MTK_ANC_ENABLE
#ifdef MTK_ANC_V2
#ifndef MTK_LEAKAGE_DETECTION_ENABLE
typedef void (*anc_leakage_compensation_callback_t)(uint16_t leakage_status);
void anc_leakage_detection_racecmd_callback(uint16_t leakage_status)
{
}
void anc_leakage_detection_racecmd_response(uint16_t leakage_status, uint8_t fromPartner)
{
}
void audio_anc_leakage_compensation_set_status(bool status)
{
}
bool audio_anc_leakage_compensation_get_status(void)
{
    return 0;
}
void audio_anc_leakage_compensation_terminate(void)
{
}
void audio_anc_leakage_compensation_start(anc_leakage_compensation_callback_t callback)
{
}
#endif

log_create_module(anc, PRINT_LEVEL_INFO);
log_create_module(anc_MP, PRINT_LEVEL_INFO);

#if !defined(MTK_DEBUG_LEVEL_NONE)
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_0[]  = LOG_DEBUG_PREFIX(anc) "anc_mutex_creat error\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_1[]  = LOG_DEBUG_PREFIX(anc) "anc_mutex_take error\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_2[]  = LOG_DEBUG_PREFIX(anc) "anc_mutex_give error\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_3[]  = LOG_DEBUG_PREFIX(anc) "[Convert feature]Unvalid filter type\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_4[]  = LOG_DEBUG_PREFIX(anc) "[Convert setting]Unvalid filter type\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_5[]  = LOG_DEBUG_PREFIX(anc) "Customized ANC callback - event:%d result:%d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_6[]  = LOG_DEBUG_PREFIX(anc) "anc_set_dvfs_control not pair, g_anc_debug:%d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_7[]  = LOG_DEBUG_PREFIX(anc) "anc_off and waiting async ack (anc_stop_delay_timer_callback)\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_8[]  = LOG_DEBUG_PREFIX(anc) "[ANC Sync] enter anc timer callback event %d\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_9[]  = LOG_DEBUG_PREFIX(anc) "[ANC Sync] sync time may wrong(>500ms): %d ms\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_10[] = LOG_DEBUG_PREFIX(anc) "[ANC Sync] start anc one-shot timer with duration %d ms\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_11[] = LOG_DEBUG_PREFIX(anc) "[ANC Sync] change timer period failed\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_12[] = LOG_DEBUG_PREFIX(anc) "[ANC sync] type(%d) fail to send aws mce report\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_13[] = LOG_DEBUG_PREFIX(anc) "[ANC dirt] fail to send aws mce report\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_14[] = LOG_DEBUG_PREFIX(anc) "anc_sync_control ignore event:%d because HFP happening.";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_15[] = LOG_DEBUG_PREFIX(anc) "[ANC Sync] is special link case, agent delay %d ms to apply and tell partner to apply directly\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_16[] = LOG_DEBUG_PREFIX(anc) "[ANC Sync] wait timer to be available : %d ms !\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_17[] = LOG_DEBUG_PREFIX(anc) "[ANC Sync] timer is unavailable too long !\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_18[] = LOG_DEBUG_PREFIX(anc) "AWS MCE Report: 0x%x 0x%x 0x%x\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_19[] = LOG_DEBUG_PREFIX(anc) "[ANC]Write ANC misc param error\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_20[] = LOG_DEBUG_PREFIX(anc) "[ANC]Backup ANC status 0x%x, ret:%d\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_21[] = LOG_DEBUG_PREFIX(anc) "[ANC]It's under MP calibration, don't backup ANC status\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_22[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_misc_param Fail. Status:%u tableSize:%lu Len:%lu\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_23[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_misc_param size not match. NVDM tableSize (%lu) != (%d)\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_24[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_calibrate_gain Fail. Status:%u tableSize:%lu Len:%lu\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_25[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_calibrate_gain size not match. NVDM tableSize (%lu) != (%d)\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_26[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_calibrate_gain is failed to read param - err(%d)\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_27[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_set_calibrate_gain is failed to write param - err(%d)\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_28[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_filter_coef Fail. Status:%u Len:%lu\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_29[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_filter_coef size not match. NVDM tableSize (%lu) != (%d)\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_30[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_filter_coef i(%d) id(0x%x) buffer(0x%x) length(%d) status(%d), tableSize(%lu)\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_31[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_filter_coef failed to read 0x%x- err(%d)\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_32[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_param Fail. Status:%u Len:%lu\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_33[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_param size not match. NVDM tableSize (%lu) != (%d)\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_34[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_param read 0x%x Fail. Status:%u tableSize:%lu Len:%lu\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_35[] = LOG_DEBUG_PREFIX(anc) "frequency is risen to MAX V.";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_36[] = LOG_DEBUG_PREFIX(anc) "frequency is set back %d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_37[] = LOG_DEBUG_PREFIX(anc) "[ANC Sync] Partner target_clk:0x%x, 0x%x, duration:%d us \n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_38[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_pwr_det_param Fail. Status:%u Len:%lu\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_39[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_pwr_det_param size not match. NVDM tableSize (%lu) != (%d)\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_40[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_pwr_det_param read 0x%x Fail. Status:%u tableSize:%lu Len:%lu\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_41[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_pwr_det_param (%d)(%d)(0x%x)(%d)(%d)(0x%x)(0x%x)(0x%x)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_42[] = LOG_DEBUG_PREFIX(anc) "anc_callback_service %d cb:0x%x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_43[] = LOG_DEBUG_PREFIX(anc) "[anc_copy_filter] pvPortMallocNC fail\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_44[] = LOG_DEBUG_PREFIX(anc) "[anc_copy_filter] is failed to anc_get_filter_coef - err(%d)\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_45[] = LOG_DEBUG_PREFIX(anc) "[anc_copy_filter](0x%x) pvPortMallocNC fail\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_46[] = LOG_DEBUG_PREFIX(anc) "anc_copy_filter, result: %d\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_47[] = LOG_DEBUG_PREFIX(anc) "[anc_realtime_update_param] set_reg_mask(%d) ch(%d) value(0x%x)\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_48[] = LOG_DEBUG_PREFIX(anc) "anc_realtime_update_param reg mask(%d) error\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_49[] = LOG_DEBUG_PREFIX(anc) "[anc_realtime_update](0x%x) pvPortMallocNC fail\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_50[] = LOG_DEBUG_PREFIX(anc) "anc_realtime_update, result: %d\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_51[] = LOG_DEBUG_PREFIX(anc) "anc on with flash_no: %d, filter type: %d, 0x%x, %d\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_52[] = LOG_DEBUG_PREFIX(anc) "anc on with copy filter %d\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_53[] = LOG_DEBUG_PREFIX(anc) "[anc_on] pvPortMallocNC fail\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_54[] = LOG_DEBUG_PREFIX(anc) "[anc_on] is failed to anc_get_filter_coef - err(%d)\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_55[] = LOG_DEBUG_PREFIX(anc) "[anc_on] pANC_PASSTHRU_param pvPortMallocNC fail\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_56[] = LOG_DEBUG_PREFIX(anc) "[anc_on] anc_get_param error(%d)\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_57[] = LOG_DEBUG_PREFIX(anc) "anc_on, result: %d\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_58[] = LOG_DEBUG_PREFIX(anc) "receive anc ack(0x%x)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_59[] = LOG_DEBUG_PREFIX(anc) "anc_off and waiting async ack\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_60[] = LOG_DEBUG_PREFIX(anc) "[ANC] audio_anc_init() finish error: 0x%x\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_61[] = LOG_DEBUG_PREFIX(anc) "[anc_set_param] set_reg_mask(0x%x)(0x%x) value(0x%x), error:0x%x\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_62[] = LOG_DEBUG_PREFIX(anc) "[anc_set_param] set debug mode\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_63[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_set_param reg mask(%d) error\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_64[] = LOG_DEBUG_PREFIX(anc) "get_hybrid_capability %x %d %d \r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_65[] = LOG_DEBUG_PREFIX(anc) "ANC_SET_RUNTIME_VOL gain:%d, type:%d, cur_type:%d, cur_id:%d, EN(%d)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_66[] = LOG_DEBUG_PREFIX(anc) "[ANC][Init]anc failed to get misc param.\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_67[] = LOG_DEBUG_PREFIX(anc) "anc failed to register aws mce report callback\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_68[] = LOG_DEBUG_PREFIX(anc) "audio_anc_init: create timer failed\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_69[] = LOG_DEBUG_PREFIX(anc) "[ANC] create g_anc_stop_dly_timer failed\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_70[] = LOG_DEBUG_PREFIX(anc) "[ANC][API]audio_anc_enable_by_type type(%d) error.\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_71[] = LOG_DEBUG_PREFIX(anc) "audio_anc_read_calibrate_gain_param ERROR!! Gain(0x%x) Filter(0x%x)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_72[] = LOG_DEBUG_PREFIX(anc) "audio_anc_read_calibrate_gain_param Filter(0x%x) ERROR!!";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_73[] = LOG_DEBUG_PREFIX(anc) "audio_anc_write_calibrate_gain_param Filter(0x%x) ERROR!!";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_74[] = LOG_DEBUG_PREFIX(anc) "[ANC][API]audio_anc_enable type(%d) error.\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_75[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc failed to reinit. Status:%u tableSize:%lu Len:%lu\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_76[] = LOG_DEBUG_PREFIX(anc) "audio_anc_control ignore event:%d because HFP happening.";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_77[] = LOG_DEBUG_PREFIX(anc) "[ANC]audio_anc_control with event:%d flash_id(%d) type(%d)(0x%x)(%d)(0x%x)\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_78[] = LOG_DEBUG_PREFIX(anc) "[RECORD_LC]read nvkey mask[0]:0x%x, mask[1]:0x%x, mask[2]:%d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_79[] = LOG_DEBUG_PREFIX(anc) "[RECORD_LC]read nvkey id 0x%x fail, status:%d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_80[] = LOG_DEBUG_PREFIX(anc) "[RECORD_LC][CALLBACK]HAL_AUDIO_EVENT_ERROR\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_81[] = LOG_DEBUG_PREFIX(anc) "[RECORD_LC][CALLBACK]read stream in failed\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_82[] = LOG_DEBUG_PREFIX(anc) "[RECORD_LC]AM_CB, aud_id:%x, msg_id:%x, sub_msg:%x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_83[] = LOG_DEBUG_PREFIX(anc) "[RECORD_LC]AM_CB, AUD_SUSPEND_BY_HFP";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_84[] = LOG_DEBUG_PREFIX(anc) "[RECORD_LC]AM_CB, AUD_SUSPEND_BY_IND";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_85[] = LOG_DEBUG_PREFIX(anc) "[RECORD_LC]Leakage compensation start.";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_86[] = LOG_DEBUG_PREFIX(anc) "[RECORD_LC]g_reocrd_id %x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_87[] = LOG_DEBUG_PREFIX(anc) "[RECORD_LC]after ccni, status:%d, w_set:%d\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_88[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]TYPE_ENABLE input parameter error";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_89[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]TYPE_ENABLE. cmd_syncMode(%d) cmd_ancType(%d), cmd_flash_no(%d)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_90[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]ENABLE input parameter error(3)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_91[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]ENABLE. cmd_syncMode(%d) filter(0x%x) sram_bank_mask(0x%x)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_92[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]DISABLE input parameter error";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_93[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]DISABLE. cmd_syncMode(%d)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_94[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]COPY_FILTER input parameter error";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_95[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]COPY_FILTER cmd_syncMode(%d) at_copy_type(%d) filter(0x%x) sram_bank(0x%x)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_96[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]COPY_FILTER cmd_flash_no none";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_97[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]COPY_FILTER(0x1) input cmd_no. error";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_98[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]COPY_FILTER(0x1) input parameter. none";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_99[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]COPY_FILTER(0x1)(%d) sscanf error count(%d)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_100[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]COPY_FILTER(0x2) input cmd_no. error";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_101[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]COPY_FILTER(0x2) input parameter. none";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_102[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]COPY_FILTER(0x2)(%d) sscanf error count(%d)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_103[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]SET_REG input parameter error";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_104[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]SET_REG cmd_syncMode(%d) set_RG_type(%d) cmd_filter(0x%x) value_1(0x%x)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_105[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]SET value_2,3 error";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_106[] = LOG_DEBUG_PREFIX(anc) "[ANC][AM]Enable User_define with flash_id(%d) type(%d) filter_mask(0x%x) sram_bank_mask(%d)\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_107[] = LOG_DEBUG_PREFIX(anc) "[ANC][AM]Enable with flash_id(%d) type(%d) filter_mask(0x%x) sram_bank_mask(%d)\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_108[] = LOG_DEBUG_PREFIX(anc) "[ANC][AM]Disable\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_109[] = LOG_DEBUG_PREFIX(anc) "[ANC][AM]Copy filter with  flash_id(%d) type(%d) filter_mask(0x%x) sram_bank(%d)\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_110[] = LOG_DEBUG_PREFIX(anc) "[ANC][RACE]RACE_ANC_ON cmd_length(%d) V2_on_param(%d) flash_no(%d) anc_type(%d) syncMode(%d)\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_111[] = LOG_DEBUG_PREFIX(anc) "[ANC][RACE]RACE_ANC_OFF cmd_length(%d) V2_off_param(%d) syncMode(%d)\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_112[] = LOG_DEBUG_PREFIX(anc) "[ANC][RACE]RACE_ANC_SET_RUNTIME_VOL cmd_length(%d) V2_runtime_vol_param(%d) runtime_vol(0x%x) syncMode(%d)\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_113[] = LOG_DEBUG_PREFIX(anc) "[ANC][RACE]event id(%d) error.";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_114[] = LOG_DEBUG_PREFIX(anc) "[ANC][Entry]source(%d) error.";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_115[] = LOG_DEBUG_PREFIX(anc) "[ANC]Dummy anc disable, state:%d, enable:%d, filter_id:%d, type:%d suspend_flag:%d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_116[] = LOG_DEBUG_PREFIX(anc) "[ANC]Dummy anc enable, state:%d, enable:%d, filter_id:%d, type:%d, keep_disable:%d";

ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_117[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_misc_param Fail. Status:%u Len:%lu\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_118[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_misc_param size not match. NVDM tableSize (%lu) != (%d)\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_119[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_misc_param read 0x%x Fail. Status:%u tableSize:%lu Len:%lu\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_120[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_misc_param %x-%x-%x-%x--%x %x-%x-%x-%x--%x";

ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_121[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_misc_2_param Fail. Status:%u Len:%lu\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_122[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_misc_2_param size not match. NVDM tableSize (%lu) != (%d)\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_123[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_misc_2_param read 0x%x Fail. Status:%u tableSize:%lu Len:%lu\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_124[] = LOG_DEBUG_PREFIX(anc) "[ANC]anc_get_misc_2_param 0x%x, Index:0x%x Status:%u tableSize:%lu Len:%lu\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_125[] = LOG_DEBUG_PREFIX(anc) "[ANC][Init]ANC Feature_1 enable at non-support enviorment.\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_126[] = LOG_DEBUG_PREFIX(anc) "ANC_RUNTIME_GAIN_CONVERT_RAMP_GAIN ch[%d] runtime_gain(%d) extend(%d)(%d)(%d)(%d)(%d)(%d)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_127[] = LOG_DEBUG_PREFIX(anc) "AUDIO_ANC_CONTROL_EXTEND_RAMP_TYPE(%d) %d %d %d %d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_128[] = LOG_DEBUG_PREFIX(anc) "[ANC Sync] SET_RAMP_GAIN_EXTEND targer: %d, cur: %d, duration: %d";

ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_129[] = LOG_DEBUG_PREFIX(anc) "[FADP] FUll adaptive ANC has been enabled. Reject request(%d).";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_130[] = LOG_DEBUG_PREFIX(anc) "[anc_on] pAdap_FF_Filter_l pvPortMalloc fail";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_131[] = LOG_DEBUG_PREFIX(anc) "[anc_on] pAdap_FF_Filter_r pvPortMalloc fail";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_132[] = LOG_DEBUG_PREFIX(anc) "[anc_on] pAdap_FF_NVDM pvPortMalloc fail";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_133[] = LOG_DEBUG_PREFIX(anc) "ramp_extend pvPortMallocNC fail";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_134[] = LOG_DEBUG_PREFIX(anc) "[FADP]mcu flash_param(%d) success (0x%x)(%d)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_135[] = LOG_DEBUG_PREFIX(anc) "[FADP]mcu flash_param(%d) fail";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_136[] = LOG_DEBUG_PREFIX(anc) "SET_DEMO_DISABLE (0x%x)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_137[] = LOG_DEBUG_PREFIX(anc) "SET_DEMO_ENABLE (0x%x)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_138[] = LOG_DEBUG_PREFIX(anc) "[FADP]Start input parameter error";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_139[] = LOG_DEBUG_PREFIX(anc) "[FADP]Start. mode(%d)";

ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_140[] = LOG_DEBUG_PREFIX(anc) "[ANC_Set_Ramp_Gain_Capability] Error! Ramp gain value should between +6dB to -90dB, gain_value %d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_141[] = LOG_DEBUG_PREFIX(anc) "[ANC_Set_Ramp_Gain_Capability] Error! Unsupported ramp rate setting, ramp_delay %d, ramp_down_step %d, ramp_up_step %d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_142[] = LOG_DEBUG_PREFIX(anc) "[ANC_Set_Ramp_Gain_Capability] filter_group[%d] 0x%x, extend_gain %d, ramp_delay %d, down_step %d, up_step %d, val 0x%x\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_143[] = LOG_DEBUG_PREFIX(anc) "[ANC_Set_Ramp_Gain_Capability] Error! param NULL";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_144[] = LOG_DEBUG_PREFIX(anc) "[ANC_Ramp_Cap] set_or_get %d, Read_or_Write(%d) Nvkey data Fail, id:0x%x, status:%d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_145[] = LOG_DEBUG_PREFIX(anc) "[ANC_Ramp_Cap] set_or_get %d, filter_mask 0x%x, gain_value %d, delay %d, up_tep %d, dn_step %d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_146[] = LOG_DEBUG_PREFIX(anc) "[ANC_Ramp_Cap] set_or_get %d, Malloc Fail!";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_147[] = LOG_DEBUG_PREFIX(anc) "[anc_realtime_update] pvPortMallocNC fail\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_148[] = LOG_DEBUG_PREFIX(anc) "[switch_specific_filter]Invalid filter number, filter range is 0 to 3\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_149[] = LOG_DEBUG_PREFIX(anc) "[switch_specific_filter]Input coef is NULL\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_150[] = LOG_DEBUG_PREFIX(anc) "[FADP]Coef full dump index(%d): %d, %d, %d, %d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_151[] = LOG_DEBUG_PREFIX(anc) "[Rdebug]MCU_FADP result(%d) state(%d),sleep(0x%x),status(%d)(%d)\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_152[] = LOG_DEBUG_PREFIX(anc) "[anc_get_biquad_gain_param] Fail. Status:%u Len:%lu\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_153[] = LOG_DEBUG_PREFIX(anc) "[anc_get_biquad_gain_param] size not match. NVDM tableSize (%lu) != (%d)\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_154[] = LOG_DEBUG_PREFIX(anc) "[anc_get_biquad_gain_param] failed to read biquad gain from nvdm - err(%d)\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_155[] = LOG_DEBUG_PREFIX(anc) "[audio_sidetone_ANC_handler] IIR sidetone enable\n\r";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_156[] = LOG_DEBUG_PREFIX(anc) "[audio_sidetone_ANC_handler] IIR sidetone disable\n\r";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_157[] = LOG_DEBUG_PREFIX(anc) "[audio_anc_control_sidetone_set_volume] err(%d)\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_158[] = LOG_DEBUG_PREFIX(anc) "[audio_sidetone_ANC_handler] change timer period failed\n\r";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_159[] = LOG_DEBUG_PREFIX(anc) "[switch_specific_filter]coef update enable, filter = %d, option = %d\n";

ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_160[] = LOG_DEBUG_PREFIX(anc) "[ANC][API] enable_with_resume(%d) BYPASS_enable(%d), waiting for suspend done(%d)\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_161[] = LOG_DEBUG_PREFIX(anc) "[ANC][RACE]RACE_ANC_ON, because Hearing Aid, cannot enable anc_type(%d)\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_162[] = LOG_DEBUG_PREFIX(anc) "[switch_specific_filter]seamless switch fail, flag = %d\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_163[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]SEAMLESS_ENABLE input parameter error";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_164[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]SEAMLESS_ENABLE. enable(%d)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_165[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]SPECIFIC_FILTER input parameter error";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_166[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]SPECIFIC_FILTER(0x1)(%d) sscanf error count(%d)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_167[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]SET_SEAMLESS_PARAM input parameter error";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_168[] = LOG_DEBUG_PREFIX(anc) "[ANC][ATCI]SET_SEAMLESS_PARAM. step(%d), delay(%d)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_169[] = LOG_DEBUG_PREFIX(anc) "[seamless]audio_anc_control_enable_seamless(), enable = %d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_170[] = LOG_DEBUG_PREFIX(anc) "[seamless]Set seamless param Success, step = %d, delay = %d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_171[] = LOG_DEBUG_PREFIX(anc) "[seamless]Set seamless param Fail. One of param is 0, step = %d, delay = %d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_172[] = LOG_DEBUG_PREFIX(anc) "[ANC][Init]Read bonding option EFUSE = 0x%x, support_hybrid %d.\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_173[] = LOG_DEBUG_PREFIX(anc) "[ANC][Howling] Enable(%d), type(%d)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_174[] = LOG_DEBUG_PREFIX(anc) "[switch_specific_filter]seamless switch fail, ANC/PT not enable %d, %d\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_175[] = LOG_DEBUG_PREFIX(anc) "[seamless]set param fail, flag = %d\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_176[] = LOG_DEBUG_PREFIX(anc) "[ANC][Calibration] set_component_gain fail, index(%d), gain(%d)";

/*for Leakage detection*/
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_200[] = LOG_DEBUG_PREFIX(anc) "[RECORD_LC] audio_anc_control ignore event:%d because LD happening.";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_201[] = LOG_DEBUG_PREFIX(anc) "[RECORD_LC] anc leakage detection start in audio_anc_control";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_202[] = LOG_DEBUG_PREFIX(anc) "[RECORD_LC] anc leakage detection stop in audio_anc_control, enabled:%d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_203[] = LOG_DEBUG_PREFIX(anc) "[RECORD_LC] anc leakage detection resume dl";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_204[] = LOG_DEBUG_PREFIX(anc) "[RECORD_LC] audio_anc_control ignore event:%d because leakage detection happening.";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_205[] = LOG_DEBUG_PREFIX(anc) "[RECORD_LC]audio_anc_control mute music:%d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_206[] = LOG_DEBUG_PREFIX(anc) "[RECORD_LC][debug]callback:0x%x";

/*for User tirgger adaptive FF ANC*/
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_220[] = LOG_DEBUG_PREFIX(anc) "[user_trigger_ff]read filter_coef 0x%x from nvdm - stat(%d)\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_221[] = LOG_DEBUG_PREFIX(anc) "[user_trigger_ff]write filter_coef 0x%x from nvdm - err(%d)\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_222[] = LOG_DEBUG_PREFIX(anc) "[user_trigger_ff] read/write filter, vPortFree\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_223[] = LOG_DEBUG_PREFIX(anc) "[user_trigger_ff]Wrong filter selection:%d\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_224[] = LOG_DEBUG_PREFIX(anc) "[user_trigger_ff]update keep filter, current_filter_no:%d, filter_select:%d\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_225[] = LOG_DEBUG_PREFIX(anc) "[user_trigger_ff]Write Filter 0x%x result:%d\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_226[] = LOG_DEBUG_PREFIX(anc) "[user_trigger_ff]restore to factory settings, result:%d\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_227[] = LOG_DEBUG_PREFIX(anc) "[user_trigger_ff]Query Status, UT_done = %d, UT_filter_selection = %d\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_228[] = LOG_DEBUG_PREFIX(anc) "[user_trigger_ff]read 0x%x from nvdm fail - err\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_229[] = LOG_DEBUG_PREFIX(anc) "[user_trigger_ff]user_trigger_status_mutex create error\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_230[] = LOG_DEBUG_PREFIX(anc) "[user_trigger_ff]user_trigger_status_mutex take error\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_231[] = LOG_DEBUG_PREFIX(anc) "[user_trigger_ff]user_trigger_status_mutex give error\r\n";

/*Adaptive PT*/
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_232[] = LOG_DEBUG_PREFIX(anc) "[ADAP_PT]ANC_SET_EXTEND_RAMP_GAIN, wind_status(%d), wind_regular_extend_gain(%d), (%d)(%d)(%d)(%d), ramp_mask 0x%x\r\n";

/*for full adaptive anc sync*/
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_250[] = LOG_DEBUG_PREFIX(anc) "[FADP SYNC] fail to send aws mce report\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_251[] = LOG_DEBUG_PREFIX(anc) "[FADP SYNC] fail to send aws mce report drift\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_252[] = LOG_DEBUG_PREFIX(anc) "[FADP SYNC] adapt anc ctrl with event %d target %d sleep 0x%x\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_253[] = LOG_DEBUG_PREFIX(anc) "[FADP SYNC] wait timer to be available : %d ms !\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_254[] = LOG_DEBUG_PREFIX(anc) "[FADP SYNC] timer is unavailable too long !\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_255[] = LOG_DEBUG_PREFIX(anc) "[FADP SYNC] AWS Report: %d 0x%x 0x%x 0x%x\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_256[] = LOG_DEBUG_PREFIX(anc) "[FADP SYNC] CH_R target_clk:0x%x, 0x%x, duration:%d us\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_257[] = LOG_DEBUG_PREFIX(anc) "[FADP SYNC] start sync one-shot timer with duration %d ms\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_258[] = LOG_DEBUG_PREFIX(anc) "[FADP SYNC] change sync timer period failed\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_259[] = LOG_DEBUG_PREFIX(anc) "[FADP SYNC] start ctrl one-shot timer with duration %d ms\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_260[] = LOG_DEBUG_PREFIX(anc) "[FADP SYNC] change ctrl timer period failed\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_261[] = LOG_DEBUG_PREFIX(anc) "[FADP SYNC] Condition changed 0x%x -> 0x%x\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_262[] = LOG_DEBUG_PREFIX(anc) "[FADP SYNC] sleep handler error\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_263[] = LOG_DEBUG_PREFIX(anc) "[FADP SYNC] CH_L proc set DMA 0x%x\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_264[] = LOG_DEBUG_PREFIX(anc) "[FADP SYNC] CH_R proc set DMA 0x%x\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_265[] = LOG_DEBUG_PREFIX(anc) "[FDAP SYNC] failed to register aws mce report callback\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_266[] = LOG_DEBUG_PREFIX(anc) "[FDAP SYNC] audio_anc_init: create AncAdaptSyncTimer failed\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_267[] = LOG_DEBUG_PREFIX(anc) "[FDAP SYNC] audio_anc_init: create AncAdaptCtrlTimer failed\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_268[] = LOG_DEBUG_PREFIX(anc) "[FADP SYNC] CH %d msg_callback (%d)(%d) 0x%x -> 0x%x\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_269[] = LOG_DEBUG_PREFIX(anc) "[FADP SYNC] sleep_handler curr:0x%x in:0x%x %d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_270[] = LOG_DEBUG_PREFIX(anc) "[FADP SYNC] adapt anc timer callback event 0x%x -> 0x%x\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_271[] = LOG_DEBUG_PREFIX(anc) "[FADP SYNC] start chk one-shot timer with duration %d ms\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_272[] = LOG_DEBUG_PREFIX(anc) "[FADP SYNC] change chk timer period failed\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_273[] = LOG_DEBUG_PREFIX(anc) "[FADP SYNC] %d STATUS_SYNC %d %d %d %d\n";

ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_274[] = LOG_DEBUG_PREFIX(anc) "[Fade out] Event(%d) Scenario(%d) wait end : %d ms !\n";

ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_900[] = LOG_DEBUG_PREFIX(anc_MP) "Pass-through: ON , cap: %d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_901[] = LOG_DEBUG_PREFIX(anc_MP) "ANC: ON , filter: %d type: %d cap: %d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_902[] = LOG_DEBUG_PREFIX(anc_MP) "ANC/Pass-thru: OFF";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_903[] = LOG_DEBUG_PREFIX(anc_MP) "ANC/Pass-thru: OFF, Sidetone Enable";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_904[] = LOG_DEBUG_PREFIX(anc_MP) "ANC/Pass-thru: DUMMY OFF in some cases.";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_999[] = LOG_DEBUG_PREFIX(anc) "[ANC] DBG";

static log_control_block_t *g_log_control_blocks[] = {
    &LOG_CONTROL_BLOCK_SYMBOL(anc),
    &LOG_CONTROL_BLOCK_SYMBOL(anc_MP),
};

void anc_port_log_info(int32_t index, const char *message, uint32_t arg_cnt, ...)
{
    va_list ap;
    va_start(ap, arg_cnt);
    log_print_msgid(g_log_control_blocks[index], PRINT_LEVEL_INFO, message, arg_cnt, ap);
    va_end(ap);
}

void anc_port_log_notice(int32_t index, const char *message, uint32_t arg_cnt, ...)
{
    va_list ap;
    va_start(ap, arg_cnt);
    log_print_msgid(g_log_control_blocks[index], PRINT_LEVEL_WARNING, message, arg_cnt, ap);
    va_end(ap);
}

void anc_port_log_error(int32_t index, const char *message, uint32_t arg_cnt, ...)
{
    va_list ap;
    va_start(ap, arg_cnt);
    log_print_msgid(g_log_control_blocks[index], PRINT_LEVEL_ERROR, message, arg_cnt, ap);
    va_end(ap);
}

void anc_port_log_debug(int32_t index, const char *message, uint32_t arg_cnt, ...)
{
    va_list ap;
    va_start(ap, arg_cnt);
    log_print_msgid(g_log_control_blocks[index], PRINT_LEVEL_DEBUG, message, arg_cnt, ap);
    va_end(ap);
}

void anc_port_log_MP(int32_t index, const char *message, uint32_t arg_cnt, ...)
{
    va_list ap;
    va_start(ap, arg_cnt);
    log_print_msgid(g_log_control_blocks[index], PRINT_LEVEL_INFO, message, arg_cnt, ap);
    va_end(ap);
}
#else
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_0[]  = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_1[]  = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_2[]  = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_3[]  = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_4[]  = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_5[]  = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_6[]  = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_7[]  = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_8[]  = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_9[]  = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_10[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_11[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_12[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_13[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_14[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_15[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_16[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_17[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_18[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_19[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_20[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_21[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_22[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_23[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_24[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_25[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_26[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_27[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_28[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_29[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_30[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_31[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_32[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_33[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_34[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_35[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_36[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_37[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_38[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_39[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_40[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_41[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_42[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_43[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_44[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_45[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_46[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_47[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_48[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_49[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_50[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_51[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_52[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_53[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_54[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_55[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_56[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_57[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_58[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_59[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_60[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_61[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_62[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_63[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_64[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_65[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_66[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_67[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_68[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_69[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_70[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_71[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_72[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_73[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_74[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_75[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_76[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_77[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_78[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_79[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_80[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_81[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_82[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_83[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_84[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_85[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_86[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_87[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_88[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_89[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_90[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_91[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_92[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_93[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_94[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_95[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_96[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_97[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_98[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_99[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_100[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_101[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_102[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_103[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_104[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_105[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_106[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_107[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_108[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_109[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_110[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_111[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_112[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_113[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_114[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_115[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_116[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_117[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_118[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_119[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_120[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_121[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_122[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_123[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_124[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_125[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_126[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_127[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_128[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_129[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_130[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_131[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_132[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_133[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_134[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_135[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_136[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_137[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_138[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_139[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_140[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_141[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_142[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_143[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_144[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_145[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_146[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_147[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_148[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_149[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_150[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_151[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_152[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_153[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_154[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_155[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_156[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_157[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_158[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_159[] = "";

ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_160[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_161[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_162[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_163[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_164[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_165[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_166[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_167[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_168[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_169[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_170[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_171[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_172[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_173[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_174[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_175[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_176[] = "";

ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_200[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_201[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_202[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_203[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_204[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_205[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_206[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_220[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_221[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_222[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_223[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_224[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_225[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_226[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_227[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_228[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_229[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_230[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_231[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_232[] = "";

/*for full adaptive anc sync*/
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_250[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_251[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_252[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_253[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_254[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_255[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_256[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_257[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_258[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_259[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_260[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_261[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_262[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_263[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_264[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_265[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_266[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_267[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_268[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_269[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_270[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_271[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_272[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_273[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_274[] = "";

ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_900[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_901[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_902[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_903[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_904[] = "";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_999[] = "";

void anc_port_log_info(int32_t index, const char *message, uint32_t arg_cnt, ...)
{
}
void anc_port_log_notice(int32_t index, const char *message, uint32_t arg_cnt, ...)
{
}
void anc_port_log_error(int32_t index, const char *message, uint32_t arg_cnt, ...)
{
}
void anc_port_log_debug(int32_t index, const char *message, uint32_t arg_cnt, ...)
{
}
void anc_port_log_MP(int32_t index, const char *message, uint32_t arg_cnt, ...)
{
}
#endif
#endif
#endif

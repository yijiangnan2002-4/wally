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
#include "types.h"

#ifdef MTK_ANC_ENABLE
//log_create_module(anc, PRINT_LEVEL_INFO);
//log_create_module(full_adapt_anc, PRINT_LEVEL_INFO);
log_create_module_variant(anc, DEBUG_LOG_ON, PRINT_LEVEL_INFO);
log_create_module_variant(full_adapt_anc, DEBUG_LOG_ON, PRINT_LEVEL_INFO);

#if !defined(MTK_DEBUG_LEVEL_NONE)
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_0[]  = LOG_DEBUG_PREFIX(anc) "ANC Start , type: %d\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_1[]  = LOG_DEBUG_PREFIX(anc) "ANC Start Finish, ret : %d, Time_use 32K(%d) 1M(%d) ADC(%d) DAC(%d) ANC(%d)\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_2[]  = LOG_DEBUG_PREFIX(anc) "=============debug ANC ============\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_3[]  = LOG_DEBUG_PREFIX(anc) "0x%04X: 0x%08x 0x%08x 0x%08x 0x%08x\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_4[]  = LOG_DEBUG_PREFIX(anc) "=============debug ANC END ============\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_5[]  = LOG_DEBUG_PREFIX(anc) "Error!!! anc_get_device_by_mic_mask with NULL pointer";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_6[]  = LOG_DEBUG_PREFIX(anc) "[ANC_API] Enable:%d ANC MIC sub_ch(%d), mic(%d) device:%d interface:%d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_7[]  = LOG_DEBUG_PREFIX(anc) "[ANC_API] Error!!! malloc fail for source_device";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_8[]  = LOG_DEBUG_PREFIX(anc) "[ANC API] Error!!! device malloc fail for connect_ul4_spdif";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_9[]  = LOG_DEBUG_PREFIX(anc) "[ANC API] Error!!! path malloc fail for connect_ul4_spdif";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_10[] = LOG_DEBUG_PREFIX(anc) "[ANC_API]Headset project cannot support ANC_FEATURE_1.\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_11[] = LOG_DEBUG_PREFIX(anc) "[ANC_API]feature_param.mix %x %x %x %x 0x%x.\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_12[] = LOG_DEBUG_PREFIX(anc) "[ANC_API]Headset project cannot support FF ANC_FEATURE_1.\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_13[] = LOG_DEBUG_PREFIX(anc) "[ANC_API]feature_param.mix %x %x %x %x 0x%x.\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_14[] = LOG_DEBUG_PREFIX(anc) "[ANC_API]Headset project cannot support FB ANC_FEATURE_1.\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_15[] = LOG_DEBUG_PREFIX(anc) "[anc_passthru_get_param] Error!! SMOOTH_SWITCH_FILTER ERROR_2\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_16[] = LOG_DEBUG_PREFIX(anc) "[anc_passthru_get_param] Error!! Un-supported type:%d\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_17[] = LOG_DEBUG_PREFIX(anc) "[anc_passthru_get_param] type:%d, filter_mask_l:0x%x, filter_mask_r:0x%x sram_mask:0x%x\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_18[] = LOG_DEBUG_PREFIX(anc) "[anc_passthru_set_param] Error!! ANC_REG_ID_RAMP_GAIN_USER_DEFINE. type:%d\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_19[] = LOG_DEBUG_PREFIX(anc) "[anc_passthru_set_param] ANC_REG_ID_RAMP_GAIN_EXTEND. extend_ramp:(%d)(%d)\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_20[] = LOG_DEBUG_PREFIX(anc) "[anc_passthru_set_param] Error!! ANC not enable. Set REG id:%d\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_21[] = LOG_DEBUG_PREFIX(anc) "[ANC API] anc_passthru_switch_type_callback";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_22[] = LOG_DEBUG_PREFIX(anc) "[ANC_API] Switch type, need to disable sub(%d) MIC: %d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_23[] = LOG_DEBUG_PREFIX(anc) "[ANC API] anc_passthru_switch_type_callback finish";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_24[] = LOG_DEBUG_PREFIX(anc) "[ANC API] disable anc/passthru finish";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_25[] = LOG_DEBUG_PREFIX(anc) "[ANC API] anc_passthru_stop_callback finish";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_26[] = LOG_DEBUG_PREFIX(anc) "[ANC API] anc on DONE, notify CM4\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_27[] = LOG_DEBUG_PREFIX(anc) "[ANC API][notify_callback] Ramp has been stable gain_1(0x%x) gain_2(0x%x)\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_28[] = LOG_DEBUG_PREFIX(anc) "[ANC API][notify_callback] Ramp has been not stable\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_29[] = LOG_DEBUG_PREFIX(anc) "[ANC API][notify_callback] band_num_1(%d), out_gain_1(0x%x)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_30[] = LOG_DEBUG_PREFIX(anc) "[ANC API][notify_callback] band_num_2(%d), out_gain_2(0x%x)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_31[] = LOG_DEBUG_PREFIX(anc) "[ANC API][notify_callback] filter parameter change done\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_32[] = LOG_DEBUG_PREFIX(anc) "[ANC API][notify_callback] Event(%d) ERROR\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_33[] = LOG_DEBUG_PREFIX(anc) "[ANC API][notify_callback][GET] Parameter wasn't reliable.\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_34[] = LOG_DEBUG_PREFIX(anc) "[ANC API][notify_callback][GET] Ramp status(%d) gain_1(0x%x) gain_2(0x%x)\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_35[] = LOG_DEBUG_PREFIX(anc) "[ANC API][notify_callback][GET] band_num_1(%d), out_gain_1(0x%x)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_36[] = LOG_DEBUG_PREFIX(anc) "[ANC API][notify_callback][GET] band_num_2(%d), out_gain_2(0x%x)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_37[] = LOG_DEBUG_PREFIX(anc) "[ANC_API] get ramp mask fail\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_38[] = LOG_DEBUG_PREFIX(anc) "Error!!! malloc fail for switch anc type";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_39[] = LOG_DEBUG_PREFIX(anc) "Error!!! null anc_param for type switch\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_40[] = LOG_DEBUG_PREFIX(anc) "Error!!! Not support user defined type and other type at the same time.\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_41[] = LOG_DEBUG_PREFIX(anc) "[ANC_API] Switch flash_no: %d->%d , need to enable another sram bank: 0x%x->0x%x for filter 0x%x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_42[] = LOG_DEBUG_PREFIX(anc) "[ANC_API] Switch type: %d->%d , need to disable ANC filters: 0x%x 0x%x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_43[] = LOG_DEBUG_PREFIX(anc) "[ANC_API] Switch type: %d->%d , need to enable MIC mask:0x%x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_44[] = LOG_DEBUG_PREFIX(anc) "[ANC_API] Switch type: %d->%d , need to enable ANC filter: 0x%x 0x%x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_45[] = LOG_DEBUG_PREFIX(anc) "[ANC_API]Enter audio_anc_passthru_enable.\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_46[] = LOG_DEBUG_PREFIX(anc) "[ANC_API]Wrong flash_no : %d.\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_47[] = LOG_DEBUG_PREFIX(anc) "[ANC_API]Wrong type : %d.\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_48[] = LOG_DEBUG_PREFIX(anc) "[ANC_API]NULL anc nvdm param.\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_49[] = LOG_DEBUG_PREFIX(anc) "[ANC_API]NULL passthru nvdm param.\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_50[] = LOG_DEBUG_PREFIX(anc) "[ANC_API]NULL biquad filter.\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_51[] = LOG_DEBUG_PREFIX(anc) "[ANC_API]NULL biquad filter for headset R.\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_52[] = LOG_DEBUG_PREFIX(anc) "[ANC_API]Enable SMOOTH_SWITCH_FILTER ERROR_1.\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_53[] = LOG_DEBUG_PREFIX(anc) "[ANC_API]Analog setting (0x%x) (0x%x) (0x%x) (0x%x).";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_54[] = LOG_DEBUG_PREFIX(anc) "[ANC_API]Enable malloc fail.\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_55[] = LOG_DEBUG_PREFIX(anc) "[ANC API][DEBUG] anc_passthru_stop_callback finish";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_56[] = LOG_DEBUG_PREFIX(anc) "[ANC API] disable anc/passthru: is turning off with speed mode: %d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_57[] = LOG_DEBUG_PREFIX(anc) "[ANC API] disable ANC/Passthru FAIL due to malloc FAIL";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_58[] = LOG_DEBUG_PREFIX(anc) "[ANC API] realtime update FAIL due to anc disabled";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_59[] = LOG_DEBUG_PREFIX(anc) "[ANC API] realtime update FAIL due to abnormal filter:%d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_60[] = LOG_DEBUG_PREFIX(anc) "[ANC API] realtime update FAIL due to filter:%d inactive";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_61[] = LOG_DEBUG_PREFIX(anc) "[ANC API] realtime update FAIL due to abnormal length:%d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_62[] = LOG_DEBUG_PREFIX(anc) "[ANC API] realtime update FAIL due to malloc FAIL";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_63[] = LOG_DEBUG_PREFIX(anc) "[ANC_API] Realtime update coefficient , need to enable another sram bank: 0x%x->0x%x for filter 0x%x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_64[] = LOG_DEBUG_PREFIX(anc) "ANC Stop\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_65[] = LOG_DEBUG_PREFIX(anc) "ANC Stop Finish, ret : %d \r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_66[] = LOG_DEBUG_PREFIX(anc) "dsp_set_anc_param mode:%d \n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_67[] = LOG_DEBUG_PREFIX(anc) "dsp_set_anc_param, reg_id : %d, filter_mask(0x%x), value(0x%x)\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_68[] = LOG_DEBUG_PREFIX(anc) "dsp_set_anc_param, switch_ramp_dly : %d, switch_up_step(%d), switch_dn_step(%d) switch_delay(%d)\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_69[] = LOG_DEBUG_PREFIX(anc) "dsp_set_anc_param ERROR, switch_ramp_dly : %d, switch_up_step(%d), switch_dn_step(%d)\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_70[] = LOG_DEBUG_PREFIX(anc) "ANC_Filter_mask 0x%x\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_71[] = LOG_DEBUG_PREFIX(anc) "ANC set volume msg: 0x%x 0x%x\r\n";
/*hal_audio_anc.c*/
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_72[] = LOG_DEBUG_PREFIX(anc) "DSP - Hal Audio ANC GPT set filter_id = %d!!\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_73[] = LOG_DEBUG_PREFIX(anc) "DSP - Hal Audio ANC free one_shot Timer error!!, error id = %d.\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_74[] = LOG_DEBUG_PREFIX(anc) "hal_anc_reset CTL_0:0x%08x  CTL_9:0x%08x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_75[] = LOG_DEBUG_PREFIX(anc) "Set_notify(%d) status(%d) param_1:0x%x, param_2:0x%x, param_3:0x%x, param_4:0x%x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_76[] = LOG_DEBUG_PREFIX(anc) "Set_notify(%d) ch(%d) param_1:(%d)(%d), param_2:(%d)(%d), param_3:(%d)(%d), param_4:(%d)(%d)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_77[] = LOG_DEBUG_PREFIX(anc) "Set_notify(%d) event error.";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_78[] = LOG_DEBUG_PREFIX(anc) "anc set biquad with NULL ptr. filter_id:%d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_79[] = LOG_DEBUG_PREFIX(anc) "anc set biquad error: invalid filter_id:0x%x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_80[] = LOG_DEBUG_PREFIX(anc) "anc set biquad error: use active sram bank. filter_id:0x%x sram_mux:0x%x, active_filter_mask:0x%x active_sram_mux:0x%x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_81[] = LOG_DEBUG_PREFIX(anc) "anc set biquad error: invalid band_num:%d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_82[] = LOG_DEBUG_PREFIX(anc) "hal_anc_set_biquad_coef: filter_id:0x%x sram:0x%x band_num:%d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_83[] = LOG_DEBUG_PREFIX(anc) "filter[%d-%d] b[%d] 0x%x 0x%x 0x%x 0x%x 0x%x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_84[] = LOG_DEBUG_PREFIX(anc) "hal_anc_set_biquad_enable enable:%d filter_id:%d sram_mux:0x%x band_num:%d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_85[] = LOG_DEBUG_PREFIX(anc) "invalid anc param: bitshift:%d for filter_id:%d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_86[] = LOG_DEBUG_PREFIX(anc) "hal_anc_set_bit_shift (%d), (0x%x) 0x%08x 0x%08x 0x%08x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_87[] = LOG_DEBUG_PREFIX(anc) "hal_anc_set_dg_gain (%d), (0x%x) 0x%08x 0x%08x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_88[] = LOG_DEBUG_PREFIX(anc) "hal_anc_set_BQ_gain (%d), (0x%x) 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_89[] = LOG_DEBUG_PREFIX(anc) "hal_anc_set_ramp_gain filter:%d gain:%ddB(0x%x), dly:%d step:%d %d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_90[] = LOG_DEBUG_PREFIX(anc) "hal_anc_set_LM filter:%d 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_91[] = LOG_DEBUG_PREFIX(anc) "hal_anc_set_LM_param filter:%d reg_id: %d 0x%x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_92[] = LOG_DEBUG_PREFIX(anc) "hal_anc_set_deq filter:%d delay:%d s_g:%d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_93[] = LOG_DEBUG_PREFIX(anc) "hal_anc_set_mic filter:%d mic:%d MIC_SEL(0x%x)=0x%x CTL_0=0x%x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_94[] = LOG_DEBUG_PREFIX(anc) "hal_anc_set_deq_mix, enable:%d, filter_mask:0x%x, deq01_dest_ch:%d %d, DGAIN_SET_0 0x%x, DGAIN_SET_3 0x%x, CTL_0 0x%x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_95[] = LOG_DEBUG_PREFIX(anc) "hal_anc_set_classg enable:%d filter_mask:0x%x CLASSG_SET_0=0x%08x CLASSG_SET_3=0x%08x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_96[] = LOG_DEBUG_PREFIX(anc) "hal_anc_set_omix enable:%d filter_mask_l:0x%x filter_mask_r:0x%x OMIX_SET_0:0x%08x,OMIX_SET_1:0x%08x,OMIX_SET_2:0x%08x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_97[] = LOG_DEBUG_PREFIX(anc) "hal_anc_set_enable enable:%d filter_mask_l:0x%x filter_mask_r:0x%x CTL_0:0x%08x, CTL_9:0x%08x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_98[] = LOG_DEBUG_PREFIX(anc) "hal_anc_set_ul4_path enable:%d, ch_select:%d/%d, input_select:%d/%d, ul_rate:%d, CTL_0:0x%08x, TO_UL_SEL0:0x%08x, TO_UL_SEL1:0x%08x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_99[] = LOG_DEBUG_PREFIX(anc) "hal_anc_pwd_enable val:0x%x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_100[] = LOG_DEBUG_PREFIX(anc) "hal_anc_pwd_enable:%d, index_mask:0x%x threshold:%d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_101[] = LOG_DEBUG_PREFIX(anc) "hal_anc_pwd_set_threshold SET: ch:%d, %d(dB)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_102[] = LOG_DEBUG_PREFIX(anc) "hal_anc_pwd_set_threshold MOVE AWAY IRQ_status:0x%x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_103[] = LOG_DEBUG_PREFIX(anc) "hal_anc_pwd_set_threshold SET OFFSET before";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_104[] = LOG_DEBUG_PREFIX(anc) "hal_anc_pwd_set_threshold SET OFFSET: ch:%d, %d dB";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_105[] = LOG_DEBUG_PREFIX(anc) "hal_anc_pwd_set_threshold SET OFFSET after";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_106[] = LOG_DEBUG_PREFIX(anc) "DSP - Hal Audio ANC get one_shot Timer error!!, error id = %d.\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_107[] = LOG_DEBUG_PREFIX(anc) "DSP - Hal Audio ANC start one_shot Timer error!!, error id = %d.\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_108[] = LOG_DEBUG_PREFIX(anc) "DSP - Hal Audio ANC switch sram bank to 0x%x, filter_id:%d cur_gain:0x%x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_109[] = LOG_DEBUG_PREFIX(anc) "[Rdebug]Filter_id(%d) m_mask(0x%x) c_mask(0x%x) dn_gain(0x%x) continue";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_110[] = LOG_DEBUG_PREFIX(anc) "[Rdebug]Filter_id(%d) mask(0x%x) t_gain(%d) c_gain(%d) dn_gain(0x%x)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_111[] = LOG_DEBUG_PREFIX(anc) "DSP - Hal Audio ANC t_service filter_id(%d) mask(0x%x) t_gain(%d) c_gain(0x%x) dn_gain(0x%x)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_112[] = LOG_DEBUG_PREFIX(anc) "DSP - timer_period %d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_113[] = LOG_DEBUG_PREFIX(anc) "hal_anc_set_ramp_related:(%d)(%d)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_114[] = LOG_DEBUG_PREFIX(anc) "hal_anc_get_ramp_related: ret(%d), (%d)(%d) (%d)(%d)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_115[] = LOG_DEBUG_PREFIX(anc) "Hal Audio ANC t_service: Attack(%d) ch(%d) t_gain(%d) c_gain(0x%x) dn_gain(0x%x)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_116[] = LOG_DEBUG_PREFIX(anc) "Hal Audio ANC t_service: Release(%d) ch(%d) t_gain(%d) c_gain(0x%x) dn_gain(0x%x)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_117[] = LOG_DEBUG_PREFIX(anc) "afe_adapt_anc_dma_interrupt_handler";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_118[] = LOG_DEBUG_PREFIX(anc) "DSP adapt_anc_dma OFFSET! Event(0x%x), G0 cur:0x%x, base:0x%x, diff:0x%x;";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_119[] = LOG_DEBUG_PREFIX(anc) "anc_dma OFFSET_OVERFLOW ! pre %d,w %d,r %d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_120[] = LOG_DEBUG_PREFIX(anc) "[ANC_Set_Ramp_Gain_Capability] filter_mask 0x%x, gain %d, ramp_delay %d, down_step %d, up_step %d\r\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_121[] = LOG_DEBUG_PREFIX(anc) "[switch_bank]from nvkey biquad_step_period is fail 0x%x\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_122[] = LOG_DEBUG_PREFIX(anc) "[multi_frame]Nothing to sync config\n";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_123[] = LOG_DEBUG_PREFIX(anc) "[DMA][%d]hal_anc_set_dma enable:%d, ramp mask(0x%x)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_124[] = LOG_DEBUG_PREFIX(anc) "anc_mutex_creat error";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_125[] = LOG_DEBUG_PREFIX(anc) "[ADAP_PT]anc_passthru_set_param, g_ADAP_PT_pre_wnd_status %d, g_ADAP_PT_cur_wnd_status %d, g_ADAP_PT_change_wnd_status %d, ramp_mask 0x%x, (%d)(%d)(%d)(%d)(%d)(%d)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_126[] = LOG_DEBUG_PREFIX(anc) "[ADAP_PT]dsp_anc_control_suspend_adapt_passthru, suspend %d, save original settings, ch(%d)(%d/%d/%d/%d), ch(%d)(%d/%d/%d/%d)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_127[] = LOG_DEBUG_PREFIX(anc) "[ADAP_PT]ramp_reason %d, another smooth ramp is left behind! ch %d, g_smooth_ramp_dn_cnt %d, g_smooth_ramp_up_cnt %d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_128[] = LOG_DEBUG_PREFIX(anc) "[anc_ramp_by_scenario] scenario_type %d, enable %d, ret %d, scenario_mask 0x%x, dominate_scenario %d->%d, anc_status %d->%d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_129[] = LOG_DEBUG_PREFIX(anc) "[anc_ramp_by_scenario] dominate_scenario_gain %d/%d/%d/%d/%d/%d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_130[] = LOG_DEBUG_PREFIX(anc) "hal_anc_reset_biquad_taps reset:%d filter_id:%d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_131[] = LOG_DEBUG_PREFIX(anc) "hal_anc_reset_deq_mix reset:%d filter_id:0x%x DGAIN_SET_0=0x%x CTL_0=0x%x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_132[] = LOG_DEBUG_PREFIX(anc) "hal_anc_set_adap[%d]: filter_id:%d, sram_mux:0x%x, R_en(0x%x), W_en(0x%x), Value(0x%x)(0x%x)(0x%x)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_133[] = LOG_DEBUG_PREFIX(anc) "Adap filter[%d-%d] i[%d] %d, %d, %d, %d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_134[] = LOG_DEBUG_PREFIX(anc) "[DMA]set_group D0[0x%x], D1[0x%x], D2[0x%x], U[0x%x], G0[0x%x], G1[0x%x], G2[0x%x], IRQ0[0x%x], IRQ1[0x%x], CTL_0:0x%08x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_135[] = LOG_DEBUG_PREFIX(anc) "[DMA][%d]set_enable[0x%x][0x%x], CTL_0:0x%08x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_136[] = LOG_DEBUG_PREFIX(anc) "[ANC_SDM]hal_anc_set_sdm, sel %d, SDM_SET0 0x%x, SDM_SET1 0x%x, SDM_SET2 0x%x, SDM_SET3 0x%x, SDM_SET4 0x%x, SDM_SET5 0x%x, SDM_SET6 0x%x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_137[] = LOG_DEBUG_PREFIX(anc) "[ANC_SDM]hal_anc_set_sdm, ANC_CTL0 0x%x, ANC_CTL9 0x%x, DGAIN_SET_0 0x%x, DGAIN_SET_3 0x%x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_138[] = LOG_DEBUG_PREFIX(anc) "[ANC_SDM]hal_anc_set_cascade_path, enable %d, input_sel %d, dest_sel %d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_139[] = LOG_DEBUG_PREFIX(anc) "[ANC_SDM]hal_anc_set_cascade_path, enable %d, dgain_set0 0x%x, dgain_set3 0x%x";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_140[] = LOG_DEBUG_PREFIX(anc) "[ANC_decouple]hal_anc_set_decouple_path, filter_id(%d), input_sel(%d), ANC_DEQ_CTL0(0x%x), ANC_DEQ_CTL1(0x%x), ANC_DEQ_CTL2(0x%x)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_141[] = LOG_DEBUG_PREFIX(anc) "[ANC_LIB]!!!!! (%d)ramp over count, reset (0x%x) !!!!!";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_142[] = LOG_DEBUG_PREFIX(anc) "hal_anc_set_FBC_SPE setting(%d) -> setting(%d), ramp request(%d)";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_143[] = LOG_DEBUG_PREFIX(anc) "[Wind Detection]anc_set_wnd_status, update wnd status %d->%d";
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_999[] = LOG_DEBUG_PREFIX(anc) "[ANC] DBG";

#ifdef AIR_FULL_ADAPTIVE_ANC_ENABLE
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_0[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Full Adapt ANC]Initialize, init_done:%u, nvkey_ready:%u, version:0x%x, scratch_memory_size:%d";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_1[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Full Adapt ANC]Initialize ANC_PARA(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d), debug mode(%d), Lib mode(%d)";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_2[] = LOG_DEBUG_PREFIX(full_adapt_anc) "shap_dump i(%d), %d, %d, %d, %d, %d, %d, %d, %d";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_3[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Full Adapt ANC]Initialize, ignore due to init_done.";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_4[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Full Adapt ANC]Initialize done";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_5[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Full Adapt ANC]process, on_schedule(%d), out_of_schedule(%d), Lib_state(%d), over(%d), status_L[%d][%d], status_R[%d][%d], peo(%d)(%d), sleep(0x%x)";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_6[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Rdbeug]VS low priority task on_schedule(%d), out_of_schedule(%d), first_out_flag(%d), mic_dump_over(%d), adap_flag(%d)(%d)";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_7[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Rdbeug](%d)FF_IN_dump i(%d), %d, %d, %d, %d, %d, %d, %d, %d, %d, %d";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_8[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Rdbeug](%d)OMIX_OUT_FB_dump i(%d), %d, %d, %d, %d, %d, %d, %d, %d, %d, %d";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_9[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Rdbeug](%d)FB_IN_dump i(%d), %d, %d, %d, %d, %d, %d, %d, %d, %d, %d";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_10[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Rdbeug]FB_MIC_IN i(%d), %d, %d, %d, %d, %d, %d, %d, %d, %d, %d";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_11[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Rdbeug](%d)OMIX_OUT_FF_dump i(%d), %d, %d, %d, %d, %d, %d, %d, %d, %d, %d";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_12[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Rdbeug](%d)300-tap dump 4 index(%d): 0x%x, 0x%x, 0x%x, 0x%x";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_13[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Rdbeug](%d)300-tap full dump index(%d): %d, %d, %d, %d";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_14[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Rdbeug](%d)REF_dump i(%d), %d, %d, %d, %d, %d, %d, %d, %d, %d, %d";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_15[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Rdbeug](%d)D_dump i(%d), %d, %d, %d, %d, %d, %d, %d, %d, %d, %d";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_16[] = LOG_DEBUG_PREFIX(full_adapt_anc) "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_17[] = LOG_DEBUG_PREFIX(full_adapt_anc) "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_18[] = LOG_DEBUG_PREFIX(full_adapt_anc) "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_19[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Rdebug]DSP SourceSizeAudioAdapt_ANC full(%d) data_size(0x%x) frame_size(0x%x)";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_20[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Rdebug]DSP SourceSizeAudioAdapt_ANC size(%d)";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_21[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Rdebug]DSP SourceSizeAudioAdapt_ANC size(%d)";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_22[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Rdebug]DSP adapt_anc SourceDrop %d";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_23[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Rdebug]DSP adapt_anc ReadOffset 0x%x";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_24[] = LOG_DEBUG_PREFIX(full_adapt_anc) "DSP adapt_anc SourceConfig";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_25[] = LOG_DEBUG_PREFIX(full_adapt_anc) "DSP adapt_anc SourceClose";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_26[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Rdebug]DSP adapt_anc SourceReadBUF";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_27[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Rdebug]DSP BUFFER_type(%d) ch(%d) dst(0x%x) src(0x%x) length(%d) mute(%d)";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_28[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Rdebug]DSP BUFFER_type(%d) ch(%d) dst_1(0x%x) dst_2(0x%x) src(0x%x) length(%d)";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_29[] = LOG_DEBUG_PREFIX(full_adapt_anc) "DSP adapt_anc ops parametser invalid";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_30[] = LOG_DEBUG_PREFIX(full_adapt_anc) "DSP adapt_anc ops parametser invalid sink";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_31[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Rdebug][A_ANC]afe init, start addr(0x%x) size(%d)";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_32[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Rdebug][A_ANC]afe_number=%d,startaddr[0]=0x%x,startaddr[%d]=0x%x";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_33[] = LOG_DEBUG_PREFIX(full_adapt_anc) "Stream in aduio adapt_anc";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_34[] = LOG_DEBUG_PREFIX(full_adapt_anc) "DSP source create successfully";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_35[] = LOG_DEBUG_PREFIX(full_adapt_anc) "DSP source create fail";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_36[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[ANC_API] ADAP_ANC Open";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_37[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[ANC_API] Source(0x%x) Sink(0x%x)";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_38[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[ANC_API] ADAP_ANC Start";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_39[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[ANC_API] ADAP_ANC Start sharp_p NULL";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_40[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[ANC_API] transform failed";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_41[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[ANC_API] Start END (%d),ramp(0x%x)";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_42[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[ANC_API] ADAP_ANC Stop";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_43[] = LOG_DEBUG_PREFIX(full_adapt_anc) "ADAP_ANC not exit, just ack.";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_44[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[ANC_API] ADAP_ANC Close";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_45[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[ANC_API] ADAP_ANC Stop process timeout";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_46[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Full Adapt ANC]GPT check Period(%d) S_s(%d), S_e(%d), S_d(%d), L_s(%d), L_e(%d), L_d(%d)";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_47[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Full Adapt ANC]GPT check SW diff_1(%d), diff_2(%d), diff_3(%d), diff_4(%d), diff_5(%d), diff_6(%d), diff_7(%d), diff_8(%d)";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_48[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Full Adapt ANC]GPT check SW diff_1(%d), diff_2(%d), diff_3(%d)";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_49[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Full Adapt ANC]GPT check SW 4_1(%d), 4_2(%d), 4_3(%d), 4_4(%d)";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_50[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Full Adapt ANC]GPT check SW 4_1(0x%x)(0x%x)(%d), 4_3(0x%x)(0x%x)(%d), 5(0x%x)(0x%x)(%d), 6(0x%x)(0x%x)(%d), 7(0x%x)(0x%x)(%d)";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_51[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Full Adapt ANC]GPT check SW_diff_all(%d) SRAM diff_all(%d), diff_2(%d), diff_3(%d), diff_4(%d)";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_52[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Full Adapt ANC]Initialize Sem(%d) create fail";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_53[] = LOG_DEBUG_PREFIX(full_adapt_anc) "FADP sem take, caller:%d sem:0x%x";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_54[] = LOG_DEBUG_PREFIX(full_adapt_anc) "FADP sem take(%d) FAIL, caller:%d sem:0x%x";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_55[] = LOG_DEBUG_PREFIX(full_adapt_anc) "FADP sem give, caller:%d sem:0x%x";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_56[] = LOG_DEBUG_PREFIX(full_adapt_anc) "FADP sem give(%d) FAIL, caller:%d sem:0x%x";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_57[] = LOG_DEBUG_PREFIX(full_adapt_anc) "FADP change process buffer into (0x%x),(0x%x)";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_58[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Full Adapt ANC]process(%d), collect_use(0x%x), cpy_dest(0x%x), cpy_src(0x%x), size(%d)";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_59[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[ANC_API] ADAP_ANC Change Lib state (%d) -> (%d), scenario(%d)";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_60[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[ANC_API] ADAP_ANC not enable just store request. control state(%d), scenario(%d)";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_61[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Rdbeug](%d)g_FIR_FILTER b00(0x%x), shift_b(%d)";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_62[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Adapt_ANC] DSP_SET_PARA s_ms(%d), a_ms(%d), dt_status(%d)(%d), DMA_state(%d), init_flag(%d), peo_enable(%d), a_chg(%d), scenario(%d), state(%d), gpt(%d)";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_63[] = LOG_DEBUG_PREFIX(full_adapt_anc) "GC_dump(%d) i(%d), %d, %d, %d, %d, %d, %d, %d";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_64[] = LOG_DEBUG_PREFIX(full_adapt_anc) "function point is NULL!!! full adaptive anc library not load or had been unload!!!";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_65[] = LOG_DEBUG_PREFIX(full_adapt_anc) "p_export_parameters is NULL, please check!!!";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_66[] = LOG_DEBUG_PREFIX(full_adapt_anc) "pisplit_full_adapt preloader_pisplit_get_handle() error!!!!";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_67[] = LOG_DEBUG_PREFIX(full_adapt_anc) "pisplit_full_adapt preloader_pisplit_load() error!!!!";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_68[] = LOG_DEBUG_PREFIX(full_adapt_anc) "pisplit_full_adapt preloader_pisplit_unload() error!!!!";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_69[] = LOG_DEBUG_PREFIX(full_adapt_anc) "[Full Adapt ANC]Compensation path %d, %d";
#endif

#ifdef AIR_HW_VIVID_PT_ENABLE
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_0[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]Initialize, ignore due to init_done.";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_1[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]Initialize done";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_2[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]Start sharp_p NULL";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_3[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]DSP HWVivid_PT SourceClose";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_4[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]DSP adapt_anc ops parametser invalid";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_5[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]DSP adapt_anc ops parametser invalid sink";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_6[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]afe init, start addr(0x%x) size(%d)";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_7[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]afe_number=%d,startaddr[0]=0x%x,startaddr[%d]=0x%x";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_8[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]Stream in aduio hw_vivid_pt";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_9[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]DSP source create fail";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_10[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]HW VIVID PT OPEN";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_11[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]Source(0x%x) Sink(0x%x)";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_12[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]HW VIVID PT Start";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_13[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]transform failed";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_14[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]Start END ramp(0x%x)";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_15[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]HW_VIVID_PT Stop";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_16[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]HW_VIVID_PT Stop process timeout";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_17[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]HW_VIVID_PT not exit, just ack.";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_18[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]HW_VIVID_PT Close";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_19[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]LD_NR version: %x, mem_size: %d bytes";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_20[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]LD_NR proc: %d, dbg: %d";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_21[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru](%d)FF_IN_L_dump i(%d), %d, %d, %d, %d, %d, %d, %d, %d, %d, %d";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_22[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru](%d)300-tap_L full dump index(%d): 0x%x, 0x%x, 0x%x, 0x%x";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_23[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru](%d)g_FIR_FILTER b00(0x%x), shift_b(%d)";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_24[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]GPT check %d, %d, %d";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_25[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]Disable LDNR processing";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_26[] = LOG_DEBUG_PREFIX(hw_vivid_pt) "[HW Vivid Passthru]init_entry mask 0x%x";
#endif

static log_control_block_t *g_log_control_blocks[] = {
    &LOG_CONTROL_BLOCK_SYMBOL(anc),
    &LOG_CONTROL_BLOCK_SYMBOL(full_adapt_anc),
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
ATTR_LOG_STRING_LIB  g_ANC_msg_id_string_999[] = "";

#ifdef AIR_FULL_ADAPTIVE_ANC_ENABLE
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_0[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_1[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_2[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_3[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_4[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_5[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_6[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_7[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_8[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_9[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_10[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_11[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_12[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_13[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_14[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_15[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_16[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_17[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_18[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_19[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_20[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_21[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_22[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_23[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_24[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_25[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_26[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_27[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_28[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_29[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_30[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_31[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_32[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_33[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_34[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_35[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_36[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_37[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_38[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_39[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_40[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_41[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_42[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_43[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_44[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_45[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_46[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_47[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_48[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_49[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_50[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_51[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_52[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_53[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_54[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_55[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_56[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_57[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_58[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_59[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_60[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_61[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_62[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_63[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_64[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_65[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_66[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_67[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_68[] = "";
ATTR_LOG_STRING_LIB  g_FA_ANC_msg_id_string_69[] = "";
#endif

#ifdef AIR_HW_VIVID_PT_ENABLE
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_0[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_1[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_2[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_3[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_4[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_5[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_6[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_7[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_8[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_9[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_10[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_11[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_12[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_13[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_14[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_15[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_16[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_17[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_18[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_19[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_20[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_21[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_22[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_23[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_24[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_25[] = "";
ATTR_LOG_STRING_LIB  g_HW_VIVID_PT_msg_id_string_26[] = "";
#endif

void anc_port_log_info(int32_t index, const char *message, uint32_t arg_cnt, ...)
{
    UNUSED(index);
    UNUSED(message);
    UNUSED(arg_cnt);
}
void anc_port_log_notice(int32_t index, const char *message, uint32_t arg_cnt, ...)
{
    UNUSED(index);
    UNUSED(message);
    UNUSED(arg_cnt);
}
void anc_port_log_error(int32_t index, const char *message, uint32_t arg_cnt, ...)
{
    UNUSED(index);
    UNUSED(message);
    UNUSED(arg_cnt);
}
void anc_port_log_debug(int32_t index, const char *message, uint32_t arg_cnt, ...)
{
    UNUSED(index);
    UNUSED(message);
    UNUSED(arg_cnt);
}
void anc_port_log_MP(uint8_t index, uint8_t arg_cnt, uint8_t value1, uint8_t value2, uint8_t value3)
{
    UNUSED(index);
    UNUSED(arg_cnt);
    UNUSED(value1);
    UNUSED(value2);
    UNUSED(value3);
}
#endif
#endif

#if defined(AIR_ADVANCED_PASSTHROUGH_ENABLE)
log_create_module(psap, PRINT_LEVEL_INFO);
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_0[]  = LOG_DEBUG_PREFIX(psap) "hearing-aid feature set param fail, p_nvkey==NULL\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_1[]  = LOG_DEBUG_PREFIX(psap) "hearing-aid feature set param fail, p_AFC_NVKEY==NULL\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_2[]  = LOG_DEBUG_PREFIX(psap) "hearing-aid feature set param fail, p_LD_NR_NVKEY==NULL\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_3[]  = LOG_DEBUG_PREFIX(psap) "hearing-aid feature set param fail, p_AT_AGC_NVKEY==NULL\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_4[]  = LOG_DEBUG_PREFIX(psap) "hearing-aid feature set param fail, p_AT_AGC_DRC_NVKEY==NULL\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_5[]  = LOG_DEBUG_PREFIX(psap) "hearing-aid feature set param fail, p_MISC_NVKEY==NULL\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_6[]  = LOG_DEBUG_PREFIX(psap) "hearing-aid feature set param fail, nvkey 0x%x not found\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_7[]  = LOG_DEBUG_PREFIX(psap) "hearing-aid feature init\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_8[]  = LOG_DEBUG_PREFIX(psap) "hearing-aid sink afe buffer prefill size: %d (samples),ch:%d\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_9[]  = LOG_DEBUG_PREFIX(psap) "hearing-aid MISC NVKEY NULL\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_10[]  = LOG_DEBUG_PREFIX(psap) "hearing-aid gain setting: 0x%x, 0x%x, 0x%x ; afc_cal:%d\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_11[]  = LOG_DEBUG_PREFIX(psap) "hearing-aid AFC NVKEY NULL\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_12[]  = LOG_DEBUG_PREFIX(psap) "hearing-aid LD_NR NVKEY NULL\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_13[]  = LOG_DEBUG_PREFIX(psap) "hearing-aid AT_AGC NVKEY NULL\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_14[]  = LOG_DEBUG_PREFIX(psap) "hearing-aid AT_AGC_DRC NVKEY NULL\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_15[]  = LOG_DEBUG_PREFIX(psap) "hearing-aid feature deinit\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_16[]  = LOG_DEBUG_PREFIX(psap) "hearing-aid stream start, gpt count = 0x%x\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_50[]  = LOG_DEBUG_PREFIX(psap) "[AFC] Port not enough!\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_51[]  = LOG_DEBUG_PREFIX(psap) "[AFC] Port is NULL!\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_52[]  = LOG_DEBUG_PREFIX(psap) "[AFC] status is error, %d!\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_53[]  = LOG_DEBUG_PREFIX(psap) "[AFC] error channel number, %d, %d!\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_54[]  = LOG_DEBUG_PREFIX(psap) "[AFC] Port is not found!\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_55[]  = LOG_DEBUG_PREFIX(psap) "[AFC] error setting %d, %d, %d!\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_56[]  = LOG_DEBUG_PREFIX(psap) "[AFC][Version 0x%x] init done, %u, %u, %u, %u, %u! cal:%d\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_57[]  = LOG_DEBUG_PREFIX(psap) "[AFC][param] %u, %u, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_58[]  = LOG_DEBUG_PREFIX(psap) "[AFC] finish time %u\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_70[]  = LOG_DEBUG_PREFIX(psap) "[LD_NR][BG] ld_nr_background_entry\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_71[]  = LOG_DEBUG_PREFIX(psap) "[LD_NR][BG] find port %d status:%d, enable:%d, pro_en:%d, pro_fr_num:%d, pro_fr_acc:%d\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_72[]  = LOG_DEBUG_PREFIX(psap) "[LD_NR][BG] background process port %d\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_73[]  = LOG_DEBUG_PREFIX(psap) "[LD_NR][BG] proc2 ch%d res:%d, duration:%u\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_74[]  = LOG_DEBUG_PREFIX(psap) "[LD_NR][BG] no user then delete background entry\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_75[]  = LOG_DEBUG_PREFIX(psap) "[LD_NR] Port not enough!\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_76[]  = LOG_DEBUG_PREFIX(psap) "[LD_NR] Port is NULL!\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_77[]  = LOG_DEBUG_PREFIX(psap) "[LD_NR] status is error, %d!\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_78[]  = LOG_DEBUG_PREFIX(psap) "[LD_NR] Port is not found!\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_79[]  = LOG_DEBUG_PREFIX(psap) "[LD_NR] error setting %d, %d, %d!\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_80[]  = LOG_DEBUG_PREFIX(psap) "[LD_NR][Version 0x%x] init done, %u, %u, %u, %u, %u!\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_81[]  = LOG_DEBUG_PREFIX(psap) "[LD_NR] proc1 fr%d res:%d, duration:%u, finish time:%u\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_82[]  = LOG_DEBUG_PREFIX(psap) "[LD_NR] abnormal res:%d, duration:%u\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_90[]  = LOG_DEBUG_PREFIX(psap) "[AT_AGC] error band_sw!\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_91[]  = LOG_DEBUG_PREFIX(psap) "[AT_AGC] Port not enough!\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_92[]  = LOG_DEBUG_PREFIX(psap) "[AT_AGC] Port is NULL!\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_93[]  = LOG_DEBUG_PREFIX(psap) "[AT_AGC] status is error, %d!\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_94[]  = LOG_DEBUG_PREFIX(psap) "[AT_AGC] Port is not found!\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_95[]  = LOG_DEBUG_PREFIX(psap) "[AT_AGC] error setting %d, %d, %d!\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_96[]  = LOG_DEBUG_PREFIX(psap) "[AT_AGC][Version 0x%x] init done, %u, %u, %u, 0x%x, %u!\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_97[]  = LOG_DEBUG_PREFIX(psap) "[AT_AGC]Target DRC Gain = 0x%x, Current DRC Gain = 0x%x\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_98[]  = LOG_DEBUG_PREFIX(psap) "[AT_AGC]Target DRC Gain = 0x%x, Current DRC Gain = 0x%x\r\n";
ATTR_LOG_STRING_LIB  g_PSAP_msg_id_string_99[]  = LOG_DEBUG_PREFIX(psap) "[AT_AGC] finish time %u\r\n";
#if !defined(MTK_DEBUG_LEVEL_NONE)
static log_control_block_t *g_psap_log_control_blocks[] = {
    &LOG_CONTROL_BLOCK_SYMBOL(psap),
};
void psap_port_log_info(int32_t index, const char *message, uint32_t arg_cnt, ...)
{
    va_list ap;
    va_start(ap, arg_cnt);
    log_print_msgid(g_psap_log_control_blocks[index], PRINT_LEVEL_INFO, message, arg_cnt, ap);
    va_end(ap);
}
void psap_port_log_notice(int32_t index, const char *message, uint32_t arg_cnt, ...)
{
    va_list ap;
    va_start(ap, arg_cnt);
    log_print_msgid(g_psap_log_control_blocks[index], PRINT_LEVEL_WARNING, message, arg_cnt, ap);
    va_end(ap);
}
void psap_port_log_error(int32_t index, const char *message, uint32_t arg_cnt, ...)
{
    va_list ap;
    va_start(ap, arg_cnt);
    log_print_msgid(g_psap_log_control_blocks[index], PRINT_LEVEL_ERROR, message, arg_cnt, ap);
    va_end(ap);
}
void psap_port_log_debug(int32_t index, const char *message, uint32_t arg_cnt, ...)
{
    va_list ap;
    va_start(ap, arg_cnt);
    log_print_msgid(g_psap_log_control_blocks[index], PRINT_LEVEL_DEBUG, message, arg_cnt, ap);
    va_end(ap);
}
#else
void psap_port_log_info(int32_t index, const char *message, uint32_t arg_cnt, ...)
{
}
void psap_port_log_notice(int32_t index, const char *message, uint32_t arg_cnt, ...)
{
}
void psap_port_log_error(int32_t index, const char *message, uint32_t arg_cnt, ...)
{
}
void psap_port_log_debug(int32_t index, const char *message, uint32_t arg_cnt, ...)
{
}
void psap_port_log_MP(uint8_t index, uint8_t arg_cnt, uint8_t value1, uint8_t value2, uint8_t value3)
{
}
#endif
#endif

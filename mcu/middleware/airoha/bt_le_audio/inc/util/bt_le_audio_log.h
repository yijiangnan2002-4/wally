/* Copyright Statement:
 *
 * (C) 2019  Airoha Technology Corp. All rights reserved.
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

#ifndef __BT_LE_AUDIO_LOG_H__
#define __BT_LE_AUDIO_LOG_H__

/** @brief This function calls the log_print_msgid function to print the user's log string with an debug level.
 *  @param[in] msg is the string message ID.
 *  @param[in] ... is the parameter list corresponding with the message string.
*/
void le_audio_log_d(const char *msg, uint32_t arg_cnt, ...);

/** @brief This function calls the log_print_msgid function to print the user's log string with an information level.
 *  @param[in] msg is the string message ID.
 *  @param[in] ... is the parameter list corresponding with the message string.
*/
void le_audio_log_i(const char *msg, uint32_t arg_cnt, ...);

/** @brief This function calls the log_print_msgid function to print the user's log string with an warning level.
 *  @param[in] msg is the string message ID.
 *  @param[in] ... is the parameter list corresponding with the message string.
*/
void le_audio_log_w(const char *msg, uint32_t arg_cnt, ...);

/** @brief This function calls the log_print_msgid function to print the user's log string with an error level.
 *  @param[in] msg is the string message ID.
 *  @param[in] ... is the parameter list corresponding with the message string.
*/
void le_audio_log_e(const char *msg, uint32_t arg_cnt, ...);

/** @brief This macro calls the le_audio_log_d function to print the user's log string with an debug level.
 *  @param[in] msg is the string message ID.
 *  @param[in] ... is the parameter list corresponding with the message string.
*/
#define LE_AUDIO_LOG_D(msg, arg_cnt, ...)   le_audio_log_d(msg, arg_cnt, ## __VA_ARGS__)

/** @brief This macro calls the le_audio_log_d function to print the user's log string with an information level.
 *  @param[in] msg is the string message ID.
 *  @param[in] ... is the parameter list corresponding with the message string.
*/
#define LE_AUDIO_LOG_I(msg, arg_cnt, ...)   le_audio_log_i(msg, arg_cnt, ## __VA_ARGS__)

/** @brief This macro calls the le_audio_log_d function to print the user's log string with an warning level.
 *  @param[in] msg is the string message ID.
 *  @param[in] ... is the parameter list corresponding with the message string.
*/
#define LE_AUDIO_LOG_W(msg, arg_cnt, ...)   le_audio_log_w(msg, arg_cnt, ## __VA_ARGS__)

/** @brief This macro calls the le_audio_log_d function to print the user's log string with an error level.
 *  @param[in] msg is the string message ID.
 *  @param[in] ... is the parameter list corresponding with the message string.
*/
#define LE_AUDIO_LOG_E(msg, arg_cnt, ...)   le_audio_log_e(msg, arg_cnt, ## __VA_ARGS__)

/**
 * @brief Defines string message ID.
 */
extern const char BT_LE_AUDIO_000[];
extern const char BT_LE_AUDIO_001[];
extern const char BT_LE_AUDIO_002[];
extern const char BT_LE_AUDIO_003[];
extern const char BT_LE_AUDIO_004[];
extern const char BT_LE_AUDIO_005[];
extern const char BT_LE_AUDIO_006[];
extern const char BT_LE_AUDIO_007[];
extern const char BT_LE_AUDIO_008[];
extern const char BT_LE_AUDIO_009[];

/* CCP */
extern const char BT_LE_AUDIO_CCP_000[];
extern const char BT_LE_AUDIO_CCP_001[];
extern const char BT_LE_AUDIO_CCP_002[];
extern const char BT_LE_AUDIO_CCP_003[];
extern const char BT_LE_AUDIO_CCP_004[];
extern const char BT_LE_AUDIO_CCP_005[];
extern const char BT_LE_AUDIO_CCP_006[];
extern const char BT_LE_AUDIO_CCP_007[];
extern const char BT_LE_AUDIO_CCP_008[];
extern const char BT_LE_AUDIO_CCP_009[];
extern const char BT_LE_AUDIO_CCP_00A[];
extern const char BT_LE_AUDIO_CCP_00B[];
extern const char BT_LE_AUDIO_CCP_00C[];
extern const char BT_LE_AUDIO_CCP_00D[];
extern const char BT_LE_AUDIO_CCP_00E[];
extern const char BT_LE_AUDIO_CCP_00F[];

/* TBS */
extern const char BT_LE_AUDIO_TBS_000[];
extern const char BT_LE_AUDIO_TBS_001[];
extern const char BT_LE_AUDIO_TBS_002[];
extern const char BT_LE_AUDIO_TBS_003[];
extern const char BT_LE_AUDIO_TBS_004[];
extern const char BT_LE_AUDIO_TBS_005[];
extern const char BT_LE_AUDIO_TBS_006[];

/* MCP */
extern const char BT_LE_AUDIO_MCP_000[];
extern const char BT_LE_AUDIO_MCP_001[];
extern const char BT_LE_AUDIO_MCP_002[];
extern const char BT_LE_AUDIO_MCP_003[];
extern const char BT_LE_AUDIO_MCP_004[];
extern const char BT_LE_AUDIO_MCP_005[];
extern const char BT_LE_AUDIO_MCP_006[];
extern const char BT_LE_AUDIO_MCP_007[];
extern const char BT_LE_AUDIO_MCP_008[];
extern const char BT_LE_AUDIO_MCP_009[];
extern const char BT_LE_AUDIO_MCP_00A[];
extern const char BT_LE_AUDIO_MCP_00B[];
extern const char BT_LE_AUDIO_MCP_00C[];
extern const char BT_LE_AUDIO_MCP_00D[];
extern const char BT_LE_AUDIO_MCP_00E[];
extern const char BT_LE_AUDIO_MCP_100[];
extern const char BT_LE_AUDIO_MCP_101[];
extern const char BT_LE_AUDIO_MCP_102[];

/* VCP */
extern const char BT_LE_AUDIO_VCP_000[];
extern const char BT_LE_AUDIO_VCP_001[];
extern const char BT_LE_AUDIO_VCP_002[];

/* VCS */
extern const char BT_LE_AUDIO_VCS_001[];
extern const char BT_LE_AUDIO_VCS_002[];
extern const char BT_LE_AUDIO_VCS_003[];
extern const char BT_LE_AUDIO_VCS_004[];
extern const char BT_LE_AUDIO_VCS_005[];
extern const char BT_LE_AUDIO_VCS_006[];
extern const char BT_LE_AUDIO_VCS_007[];
extern const char BT_LE_AUDIO_VCS_008[];
extern const char BT_LE_AUDIO_VCS_009[];
extern const char BT_LE_AUDIO_VCS_00A[];
extern const char BT_LE_AUDIO_VCS_00B[];
extern const char BT_LE_AUDIO_VCS_00C[];
extern const char BT_LE_AUDIO_VCS_00D[];
extern const char BT_LE_AUDIO_VCS_00E[];
extern const char BT_LE_AUDIO_VCS_00F[];
extern const char BT_LE_AUDIO_VCS_010[];

/* ASCS */
extern const char BT_LE_AUDIO_ASCS_000[];
extern const char BT_LE_AUDIO_ASCS_001[];
extern const char BT_LE_AUDIO_ASCS_002[];
extern const char BT_LE_AUDIO_ASCS_003[];
extern const char BT_LE_AUDIO_ASCS_004[];
extern const char BT_LE_AUDIO_ASCS_005[];
extern const char BT_LE_AUDIO_ASCS_006[];
extern const char BT_LE_AUDIO_ASCS_007[];
extern const char BT_LE_AUDIO_ASCS_008[];
extern const char BT_LE_AUDIO_ASCS_009[];
extern const char BT_LE_AUDIO_ASCS_00A[];
extern const char BT_LE_AUDIO_ASCS_00B[];
extern const char BT_LE_AUDIO_ASCS_00C[];
extern const char BT_LE_AUDIO_ASCS_00D[];
extern const char BT_LE_AUDIO_ASCS_00E[];
extern const char BT_LE_AUDIO_ASCS_00F[];
extern const char BT_LE_AUDIO_ASCS_010[];
extern const char BT_LE_AUDIO_ASCS_011[];
extern const char BT_LE_AUDIO_ASCS_012[];
extern const char BT_LE_AUDIO_ASCS_013[];
extern const char BT_LE_AUDIO_ASCS_014[];
extern const char BT_LE_AUDIO_ASCS_015[];
extern const char BT_LE_AUDIO_ASCS_016[];
extern const char BT_LE_AUDIO_ASCS_017[];
extern const char BT_LE_AUDIO_ASCS_018[];
extern const char BT_LE_AUDIO_ASCS_019[];
extern const char BT_LE_AUDIO_ASCS_01A[];
extern const char BT_LE_AUDIO_ASCS_01B[];
extern const char BT_LE_AUDIO_ASCS_01C[];
extern const char BT_LE_AUDIO_ASCS_01D[];
extern const char BT_LE_AUDIO_ASCS_01E[];
extern const char BT_LE_AUDIO_ASCS_01F[];
extern const char BT_LE_AUDIO_ASCS_020[];
extern const char BT_LE_AUDIO_ASCS_021[];
extern const char BT_LE_AUDIO_ASCS_022[];
extern const char BT_LE_AUDIO_ASCS_023[];
extern const char BT_LE_AUDIO_ASCS_024[];
extern const char BT_LE_AUDIO_ASCS_025[];
extern const char BT_LE_AUDIO_ASCS_026[];
extern const char BT_LE_AUDIO_ASCS_027[];

extern const char BT_LE_AUDIO_ASCS_ASE_OPCODE_STRING[][30];
extern const char BT_LE_AUDIO_ASCS_ASE_OPCODE_STRING2[][30];

/* PACS */
extern const char BT_LE_AUDIO_PACS_000[];
extern const char BT_LE_AUDIO_PACS_001[];
extern const char BT_LE_AUDIO_PACS_002[];

/* BAP */
extern const char BT_LE_AUDIO_BAP_000[];
extern const char BT_LE_AUDIO_BAP_001[];
extern const char BT_LE_AUDIO_BAP_002[];
extern const char BT_LE_AUDIO_BAP_003[];
extern const char BT_LE_AUDIO_BAP_004[];
extern const char BT_LE_AUDIO_BAP_005[];
extern const char BT_LE_AUDIO_BAP_006[];
extern const char BT_LE_AUDIO_BAP_007[];
extern const char BT_LE_AUDIO_BAP_008[];
extern const char BT_LE_AUDIO_BAP_009[];
extern const char BT_LE_AUDIO_BAP_00A[];

/* BASS */
extern const char BT_LE_AUDIO_BASS_000[];
extern const char BT_LE_AUDIO_BASS_001[];
extern const char BT_LE_AUDIO_BASS_002[];

/* CAP */
extern const char BT_LE_AUDIO_CAP_000[];
extern const char BT_LE_AUDIO_CAP_001[];
extern const char BT_LE_AUDIO_CAP_002[];
extern const char BT_LE_AUDIO_CAP_003[];
extern const char BT_LE_AUDIO_CAP_004[];

/* CSIS */
extern const char BT_LE_AUDIO_CSIS_000[];
extern const char BT_LE_AUDIO_CSIS_001[];

/* CSIP */
extern const char BT_LE_AUDIO_CSIP_000[];
extern const char BT_LE_AUDIO_CSIP_001[];
extern const char BT_LE_AUDIO_CSIP_002[];
extern const char BT_LE_AUDIO_CSIP_003[];
extern const char BT_LE_AUDIO_CSIP_004[];

/* HAPS */
extern const char BT_LE_AUDIO_HAPS_000[];
extern const char BT_LE_AUDIO_HAPS_001[];
extern const char BT_LE_AUDIO_HAPS_002[];
extern const char BT_LE_AUDIO_HAPS_003[];
extern const char BT_LE_AUDIO_HAPS_004[];
extern const char BT_LE_AUDIO_HAPS_005[];
extern const char BT_LE_AUDIO_HAPS_006[];
extern const char BT_LE_AUDIO_HAPS_007[];
extern const char BT_LE_AUDIO_HAPS_008[];
extern const char BT_LE_AUDIO_HAPS_009[];
extern const char BT_LE_AUDIO_HAPS_00A[];
extern const char BT_LE_AUDIO_HAPS_00B[];
extern const char BT_LE_AUDIO_HAPS_00C[];
extern const char BT_LE_AUDIO_HAPS_00D[];
extern const char BT_LE_AUDIO_HAPS_00E[];
extern const char BT_LE_AUDIO_HAPS_00F[];
extern const char BT_LE_AUDIO_HAPS_010[];
extern const char BT_LE_AUDIO_HAPS_011[];
extern const char BT_LE_AUDIO_HAPS_012[];
extern const char BT_LE_AUDIO_HAPS_013[];
extern const char BT_LE_AUDIO_HAPS_014[];
extern const char BT_LE_AUDIO_HAPS_015[];
extern const char BT_LE_AUDIO_HAPS_016[];
extern const char BT_LE_AUDIO_HAPS_017[];

extern const char BT_LE_AUDIO_MICP_001[];

/* HAPC */
extern const char BT_LE_AUDIO_HAPC_001[];
extern const char BT_LE_AUDIO_HAPC_002[];
extern const char BT_LE_AUDIO_HAPC_003[];
extern const char BT_LE_AUDIO_HAPC_004[];
extern const char BT_LE_AUDIO_HAPC_005[];
extern const char BT_LE_AUDIO_HAPC_006[];
extern const char BT_LE_AUDIO_HAPC_007[];
extern const char BT_LE_AUDIO_HAPC_008[];
extern const char BT_LE_AUDIO_HAPC_009[];
extern const char BT_LE_AUDIO_HAPC_00A[];
extern const char BT_LE_AUDIO_HAPC_00B[];
extern const char BT_LE_AUDIO_HAPC_00C[];
extern const char BT_LE_AUDIO_HAPC_00D[];
extern const char BT_LE_AUDIO_HAPC_00E[];
extern const char BT_LE_AUDIO_HAPC_00F[];

/* IAC */
extern const char BT_LE_AUDIO_IAC_001[];
extern const char BT_LE_AUDIO_IAC_002[];

/* BAP Client */
extern const char BT_LE_AUDIO_BAPC_000[];
extern const char BT_LE_AUDIO_BAPC_001[];
extern const char BT_LE_AUDIO_BAPC_002[];
extern const char BT_LE_AUDIO_BAPC_003[];

#endif  /* __BT_LE_AUDIO_LOG_H__ */


/* Copyright Statement:
 *
 * (C) 2018  Airoha Technology Corp. All rights reserved.
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

#ifdef MTK_NVDM_ENABLE

#include "hal.h"
#include "syslog.h"
#include <assert.h>
#include "nvdm_port.h"
#include "memory_attribute.h"

#ifdef __EXT_BOOTLOADER__
#include "bl_common.h"

#undef ATTR_LOG_STRING_LIB
#define ATTR_LOG_STRING_LIB const char

#undef LOG_INFO_PREFIX
#define LOG_INFO_PREFIX(module) "\r\n[M:" #module " C:I]:"

#undef LOG_WARNING_PREFIX
#define LOG_WARNING_PREFIX(module) "\r\n[M:" #module " C:W]:"

#undef LOG_ERROR_PREFIX
#define LOG_ERROR_PREFIX(module) "\r\n[M:" #module " C:E]:"

#endif

#if !defined(MTK_DEBUG_LEVEL_NONE)
ATTR_LOG_STRING_LIB nvdm_001[] = LOG_INFO_PREFIX(nvdm) "data item header info show below:";
ATTR_LOG_STRING_LIB nvdm_002[] = LOG_INFO_PREFIX(nvdm) "status: 0x%x";
ATTR_LOG_STRING_LIB nvdm_003[] = LOG_INFO_PREFIX(nvdm) "pnum: %d";
ATTR_LOG_STRING_LIB nvdm_004[] = LOG_INFO_PREFIX(nvdm) "offset: 0x%x";
ATTR_LOG_STRING_LIB nvdm_005[] = LOG_INFO_PREFIX(nvdm) "sequence_number: %d";
ATTR_LOG_STRING_LIB nvdm_006[] = LOG_INFO_PREFIX(nvdm) "group_name_size: %d";
ATTR_LOG_STRING_LIB nvdm_007[] = LOG_INFO_PREFIX(nvdm) "data_item_name_size: %d";
ATTR_LOG_STRING_LIB nvdm_008[] = LOG_INFO_PREFIX(nvdm) "value_size: %d";
ATTR_LOG_STRING_LIB nvdm_009[] = LOG_INFO_PREFIX(nvdm) "index: %d";
ATTR_LOG_STRING_LIB nvdm_010[] = LOG_INFO_PREFIX(nvdm) "type: %d";
ATTR_LOG_STRING_LIB nvdm_011[] = LOG_INFO_PREFIX(nvdm) "hash_name: 0x%x";
ATTR_LOG_STRING_LIB nvdm_012[] = LOG_INFO_PREFIX(nvdm) "hashname = 0x%x";
ATTR_LOG_STRING_LIB nvdm_013[] = LOG_INFO_PREFIX(nvdm) "nvdm_read_data_item: begin to read";
ATTR_LOG_STRING_LIB nvdm_015[] = LOG_INFO_PREFIX(nvdm) "nvdm_write_data_item: begin to write";
ATTR_LOG_STRING_LIB nvdm_016[] = LOG_INFO_PREFIX(nvdm) "find_data_item_by_hashname return %d";
ATTR_LOG_STRING_LIB nvdm_017[] = LOG_INFO_PREFIX(nvdm) "peb free space is not enough";
ATTR_LOG_STRING_LIB nvdm_018[] = LOG_WARNING_PREFIX(nvdm) "too many data items in nvdm region";
ATTR_LOG_STRING_LIB nvdm_019[] = LOG_INFO_PREFIX(nvdm) "new data item append";
ATTR_LOG_STRING_LIB nvdm_020[] = LOG_INFO_PREFIX(nvdm) "old data item overwrite";
ATTR_LOG_STRING_LIB nvdm_022[] = LOG_INFO_PREFIX(nvdm) "nvdm_write_data_item_non_blocking: begin to write";
ATTR_LOG_STRING_LIB nvdm_023[] = LOG_ERROR_PREFIX(nvdm) "Can't alloc memory!!";
ATTR_LOG_STRING_LIB nvdm_024[] = LOG_WARNING_PREFIX(nvdm) "Can't send queue!!";
ATTR_LOG_STRING_LIB nvdm_025[] = LOG_INFO_PREFIX(nvdm) "nvdm_delete_data_item: enter";
ATTR_LOG_STRING_LIB nvdm_026[] = LOG_INFO_PREFIX(nvdm) "nvdm_delete_group: enter";
ATTR_LOG_STRING_LIB nvdm_027[] = LOG_INFO_PREFIX(nvdm) "nvdm_delete_all: enter";
ATTR_LOG_STRING_LIB nvdm_028[] = LOG_INFO_PREFIX(nvdm) "nvdm_query_begin: enter";
ATTR_LOG_STRING_LIB nvdm_029[] = LOG_INFO_PREFIX(nvdm) "nvdm_query_end: enter";
ATTR_LOG_STRING_LIB nvdm_030[] = LOG_INFO_PREFIX(nvdm) "nvdm_query_next_group_name: enter";
ATTR_LOG_STRING_LIB nvdm_031[] = LOG_INFO_PREFIX(nvdm) "nvdm_query_next_data_item_name: enter";
ATTR_LOG_STRING_LIB nvdm_032[] = LOG_INFO_PREFIX(nvdm) "nvdm_query_data_item_length: begin to query";
ATTR_LOG_STRING_LIB nvdm_034[] = LOG_INFO_PREFIX(nvdm) "scanning pnum(%d) to analysis data item info";
ATTR_LOG_STRING_LIB nvdm_035[] = LOG_ERROR_PREFIX(nvdm) "pnum=%d, offset=0x%x";
ATTR_LOG_STRING_LIB nvdm_036[] = LOG_ERROR_PREFIX(nvdm) "Detect index of data item with out of range, max = %d, curr = %d";
ATTR_LOG_STRING_LIB nvdm_037[] = LOG_WARNING_PREFIX(nvdm) "detect checksum error";
ATTR_LOG_STRING_LIB nvdm_038[] = LOG_ERROR_PREFIX(nvdm) "too many data items in nvdm region";
ATTR_LOG_STRING_LIB nvdm_039[] = LOG_INFO_PREFIX(nvdm) "detect two valid copy of data item";
ATTR_LOG_STRING_LIB nvdm_040[] = LOG_INFO_PREFIX(nvdm) "copy1(pnum=%d, offset=0x%x), copy2(pnum=%d, offset=0x%x)";
ATTR_LOG_STRING_LIB nvdm_041[] = LOG_ERROR_PREFIX(nvdm) "abnormal_data_item = %d at %d block with offset 0x%x";
ATTR_LOG_STRING_LIB nvdm_042[] = LOG_ERROR_PREFIX(nvdm) "Max size of data item must less than or equal to 2048 bytes";
ATTR_LOG_STRING_LIB nvdm_043[] = LOG_ERROR_PREFIX(nvdm) "alloc data_item_headers fail";
ATTR_LOG_STRING_LIB nvdm_044[] = LOG_ERROR_PREFIX(nvdm) "old_src_pnum=%d, old_pos=0x%x, new_src_pnum=%d, new_pos=0x%x, item_size=%d";
ATTR_LOG_STRING_LIB nvdm_045[] = LOG_ERROR_PREFIX(nvdm) "Error item status(0x%x) at src_pnum=%d, pos=0x%x";
ATTR_LOG_STRING_LIB nvdm_046[] = LOG_ERROR_PREFIX(nvdm) "pnum=%d, offset=0x%x, len=%d";
ATTR_LOG_STRING_LIB nvdm_047[] = LOG_ERROR_PREFIX(nvdm) "addr=0x%x, pnum=%d, offset=0x%x, len=%d";
ATTR_LOG_STRING_LIB nvdm_048[] = LOG_ERROR_PREFIX(nvdm) "pnum=%d";
ATTR_LOG_STRING_LIB nvdm_049[] = LOG_WARNING_PREFIX(nvdm) "region info show below:";
ATTR_LOG_STRING_LIB nvdm_050[] = LOG_WARNING_PREFIX(nvdm) "peb    valid    free    dirty    erase_count    is_reserved";
ATTR_LOG_STRING_LIB nvdm_051[] = LOG_WARNING_PREFIX(nvdm) "%3d     %4d    %4d     %4d       %8d              %d";
ATTR_LOG_STRING_LIB nvdm_052[] = LOG_WARNING_PREFIX(nvdm) "valid_data_size = %d";
ATTR_LOG_STRING_LIB nvdm_053[] = LOG_INFO_PREFIX(nvdm) "peb header(%d) info show below:";
ATTR_LOG_STRING_LIB nvdm_054[] = LOG_INFO_PREFIX(nvdm) "magic: %x";
ATTR_LOG_STRING_LIB nvdm_055[] = LOG_INFO_PREFIX(nvdm) "erase_count: %x";
ATTR_LOG_STRING_LIB nvdm_056[] = LOG_INFO_PREFIX(nvdm) "status: %x";
ATTR_LOG_STRING_LIB nvdm_057[] = LOG_INFO_PREFIX(nvdm) "peb_reserved: %x";
ATTR_LOG_STRING_LIB nvdm_058[] = LOG_INFO_PREFIX(nvdm) "version: %x";
ATTR_LOG_STRING_LIB nvdm_059[] = LOG_ERROR_PREFIX(nvdm) "pnum=%d";
ATTR_LOG_STRING_LIB nvdm_060[] = LOG_ERROR_PREFIX(nvdm) "pnum=%d";
ATTR_LOG_STRING_LIB nvdm_061[] = LOG_ERROR_PREFIX(nvdm) "offset=0x%x";
ATTR_LOG_STRING_LIB nvdm_062[] = LOG_ERROR_PREFIX(nvdm) "len=%d";
ATTR_LOG_STRING_LIB nvdm_063[] = LOG_ERROR_PREFIX(nvdm) "magic=0x%x, erase_count=0x%x, status=0x%x, peb_reserved=0x%x";
ATTR_LOG_STRING_LIB nvdm_064[] = LOG_ERROR_PREFIX(nvdm) "pnum=%d";
ATTR_LOG_STRING_LIB nvdm_065[] = LOG_ERROR_PREFIX(nvdm) "offset=0x%x";
ATTR_LOG_STRING_LIB nvdm_066[] = LOG_ERROR_PREFIX(nvdm) "len=%d";
ATTR_LOG_STRING_LIB nvdm_067[] = LOG_ERROR_PREFIX(nvdm) "pnum=%d";
ATTR_LOG_STRING_LIB nvdm_068[] = LOG_ERROR_PREFIX(nvdm) "pnum=%d";
ATTR_LOG_STRING_LIB nvdm_069[] = LOG_ERROR_PREFIX(nvdm) "pnum=%d";
ATTR_LOG_STRING_LIB nvdm_070[] = LOG_ERROR_PREFIX(nvdm) "pnum=%d";
ATTR_LOG_STRING_LIB nvdm_071[] = LOG_ERROR_PREFIX(nvdm) "pnum=%d";
ATTR_LOG_STRING_LIB nvdm_072[] = LOG_INFO_PREFIX(nvdm) "found no valid data in reclaiming pebs when relocate_pebs()";
ATTR_LOG_STRING_LIB nvdm_073[] = LOG_ERROR_PREFIX(nvdm) "target_peb=%d";
ATTR_LOG_STRING_LIB nvdm_074[] = LOG_INFO_PREFIX(nvdm) "found a target peb(%d) for reclaiming";
ATTR_LOG_STRING_LIB nvdm_075[] = LOG_INFO_PREFIX(nvdm) "merge peb: %d, data_size: %d";
ATTR_LOG_STRING_LIB nvdm_076[] = LOG_WARNING_PREFIX(nvdm) "start garbage collection!!!";
ATTR_LOG_STRING_LIB nvdm_077[] = LOG_ERROR_PREFIX(nvdm) "GC buffer alloc fail";
ATTR_LOG_STRING_LIB nvdm_078[] = LOG_INFO_PREFIX(nvdm) "non_reserved_pebs = %d";
ATTR_LOG_STRING_LIB nvdm_079[] = LOG_INFO_PREFIX(nvdm) "mean_erase_count = %d";
ATTR_LOG_STRING_LIB nvdm_080[] = LOG_INFO_PREFIX(nvdm) "reclaim blocks select by erase count = %d";
ATTR_LOG_STRING_LIB nvdm_081[] = LOG_INFO_PREFIX(nvdm) "reclaim peb_list(no-sort): ";
ATTR_LOG_STRING_LIB nvdm_082[] = LOG_INFO_PREFIX(nvdm) "%d";
ATTR_LOG_STRING_LIB nvdm_083[] = LOG_INFO_PREFIX(nvdm) "reclaim peb_list(sort): ";
ATTR_LOG_STRING_LIB nvdm_084[] = LOG_INFO_PREFIX(nvdm) "%d";
ATTR_LOG_STRING_LIB nvdm_085[] = LOG_INFO_PREFIX(nvdm) "reclaim blocks select by valid size = %d";
ATTR_LOG_STRING_LIB nvdm_086[] = LOG_INFO_PREFIX(nvdm) "reclaim peb_list(no-sort): ";
ATTR_LOG_STRING_LIB nvdm_087[] = LOG_INFO_PREFIX(nvdm) "%d";
ATTR_LOG_STRING_LIB nvdm_088[] = LOG_INFO_PREFIX(nvdm) "reclaim peb_list(sort): ";
ATTR_LOG_STRING_LIB nvdm_089[] = LOG_INFO_PREFIX(nvdm) "%d";
ATTR_LOG_STRING_LIB nvdm_090[] = LOG_INFO_PREFIX(nvdm) "find_free_peb: target_peb = %d, reserved_peb = %d, reserved_peb_cnt = %d";
ATTR_LOG_STRING_LIB nvdm_091[] = LOG_WARNING_PREFIX(nvdm) "config information: [IS]: %d  [GN]: %d  [IN]: %d  [IC]: %d  [PS]: %d  [PC]: %d  [AS]: %d";
ATTR_LOG_STRING_LIB nvdm_092[] = LOG_INFO_PREFIX(nvdm) "space_is_enough: valid_data_size = %d, new add size = %d";
ATTR_LOG_STRING_LIB nvdm_093[] = LOG_ERROR_PREFIX(nvdm) "detect valid_data_size abnormal";
ATTR_LOG_STRING_LIB nvdm_094[] = LOG_ERROR_PREFIX(nvdm) "reclaiming_peb alloc fail";
ATTR_LOG_STRING_LIB nvdm_095[] = LOG_INFO_PREFIX(nvdm) "scan and verify peb headers";
ATTR_LOG_STRING_LIB nvdm_096[] = LOG_INFO_PREFIX(nvdm) "before verify peb header";
ATTR_LOG_STRING_LIB nvdm_097[] = LOG_ERROR_PREFIX(nvdm) "peb_header validate fail, pnum=%d";
ATTR_LOG_STRING_LIB nvdm_098[] = LOG_ERROR_PREFIX(nvdm) "find more than one transfering peb, first=%d, second=%d";
ATTR_LOG_STRING_LIB nvdm_099[] = LOG_ERROR_PREFIX(nvdm) "find more than one transfered peb, first=%d, second=%d";
ATTR_LOG_STRING_LIB nvdm_100[] = LOG_ERROR_PREFIX(nvdm) "peb_header validate fail, pnum=%d";
ATTR_LOG_STRING_LIB nvdm_101[] = LOG_ERROR_PREFIX(nvdm) "peb_header validate fail, pnum=%d";
ATTR_LOG_STRING_LIB nvdm_102[] = LOG_INFO_PREFIX(nvdm) "after verify peb header";
ATTR_LOG_STRING_LIB nvdm_103[] = LOG_INFO_PREFIX(nvdm) "transfering_peb = %d";
ATTR_LOG_STRING_LIB nvdm_104[] = LOG_INFO_PREFIX(nvdm) "transfered_peb = %d";
ATTR_LOG_STRING_LIB nvdm_105[] = LOG_INFO_PREFIX(nvdm) "reclaiming_peb[%d] = %d";
ATTR_LOG_STRING_LIB nvdm_106[] = LOG_INFO_PREFIX(nvdm) "update erase count for unknown pebs";
ATTR_LOG_STRING_LIB nvdm_107[] = LOG_INFO_PREFIX(nvdm) "scan all non-reserved pebs including reclaiming pebs and transfering peb";
ATTR_LOG_STRING_LIB nvdm_108[] = LOG_INFO_PREFIX(nvdm) "found a peb in transfering status";
ATTR_LOG_STRING_LIB nvdm_109[] = LOG_INFO_PREFIX(nvdm) "found a peb in transfered status";
ATTR_LOG_STRING_LIB nvdm_110[] = LOG_ERROR_PREFIX(nvdm) "reclaim_idx=%d, transfered_peb=%d, transfering_peb=%d";
ATTR_LOG_STRING_LIB nvdm_111[] = LOG_INFO_PREFIX(nvdm) "calculate total valid data size";
ATTR_LOG_STRING_LIB nvdm_112[] = LOG_ERROR_PREFIX(nvdm) "Count of PEB for NVDM region must greater than or equal to 2";
ATTR_LOG_STRING_LIB nvdm_113[] = LOG_ERROR_PREFIX(nvdm) "alloc peb_info fail";
ATTR_LOG_STRING_LIB nvdm_114[] = LOG_WARNING_PREFIX(nvdm) "nvdm init finished";
ATTR_LOG_STRING_LIB nvdm_115[] = LOG_WARNING_PREFIX(nvdm) "garbage collection finished, about %d ms used";
ATTR_LOG_STRING_LIB nvdm_116[] = LOG_ERROR_PREFIX(nvdm) "invalid data item at (%d, %d) with %d bytes( %d, %d )";
ATTR_LOG_STRING_LIB nvdm_117[] = LOG_WARNING_PREFIX(nvdm) "Canceled %d non-blocking write data";
ATTR_LOG_STRING_LIB nvdm_118[] = LOG_WARNING_PREFIX(nvdm) "[%c] Receive null pointer";
ATTR_LOG_STRING_LIB nvdm_119[] = LOG_WARNING_PREFIX(nvdm) "[%c] Size dismatch(%u, %u)";
ATTR_LOG_STRING_LIB nvdm_120[] = LOG_WARNING_PREFIX(nvdm) "[%c] Length dismatch: G(%u, %u), I(%u, %u)";
ATTR_LOG_STRING_LIB nvdm_121[] = LOG_WARNING_PREFIX(nvdm) "[%c] The NVDM driver has not been initialized.";
ATTR_LOG_STRING_LIB nvdm_122[] = LOG_WARNING_PREFIX(nvdm) "Unsupported data type(%d).";
ATTR_LOG_STRING_LIB nvdm_123[] = LOG_WARNING_PREFIX(nvdm) "Skip %u partition because of cfg dismatch.";
ATTR_LOG_STRING_LIB nvdm_124[] = LOG_WARNING_PREFIX(nvdm) "[NBW_Cancel] next ptr is 0x%x and dummy head is 0x%x";
ATTR_LOG_STRING_LIB nvdm_125[] = LOG_WARNING_PREFIX(nvdm) "partition %u need %u byte for %u item headers.";
ATTR_LOG_STRING_LIB nvdm_126[] = LOG_WARNING_PREFIX(nvdm) "partition %u need %u byte for %u PEB headers.";
ATTR_LOG_STRING_LIB nvdm_127[] = LOG_WARNING_PREFIX(nvdm) "NVDM partition[%d] info: [F]%u [D]%u [V]%u [C]%u [P]%u";
ATTR_LOG_STRING_LIB nvdm_128[] = LOG_WARNING_PREFIX(nvdm) "Actively trigger GC with { %d, %d }.";
ATTR_LOG_STRING_LIB nvdm_129[] = LOG_WARNING_PREFIX(nvdm) "Actively trigger GC done { %d, %d, %d ms }.";
ATTR_LOG_STRING_LIB nvdm_130[] = LOG_WARNING_PREFIX(nvdm) "malloc fail %d";

#else /* !defined(MTK_DEBUG_LEVEL_NONE) */
ATTR_LOG_STRING_LIB nvdm_001[] = "";
ATTR_LOG_STRING_LIB nvdm_002[] = "";
ATTR_LOG_STRING_LIB nvdm_003[] = "";
ATTR_LOG_STRING_LIB nvdm_004[] = "";
ATTR_LOG_STRING_LIB nvdm_005[] = "";
ATTR_LOG_STRING_LIB nvdm_006[] = "";
ATTR_LOG_STRING_LIB nvdm_007[] = "";
ATTR_LOG_STRING_LIB nvdm_008[] = "";
ATTR_LOG_STRING_LIB nvdm_009[] = "";
ATTR_LOG_STRING_LIB nvdm_010[] = "";
ATTR_LOG_STRING_LIB nvdm_011[] = "";
ATTR_LOG_STRING_LIB nvdm_012[] = "";
ATTR_LOG_STRING_LIB nvdm_013[] = "";
ATTR_LOG_STRING_LIB nvdm_015[] = "";
ATTR_LOG_STRING_LIB nvdm_016[] = "";
ATTR_LOG_STRING_LIB nvdm_017[] = "";
ATTR_LOG_STRING_LIB nvdm_018[] = "";
ATTR_LOG_STRING_LIB nvdm_019[] = "";
ATTR_LOG_STRING_LIB nvdm_020[] = "";
ATTR_LOG_STRING_LIB nvdm_022[] = "";
ATTR_LOG_STRING_LIB nvdm_023[] = "";
ATTR_LOG_STRING_LIB nvdm_024[] = "";
ATTR_LOG_STRING_LIB nvdm_025[] = "";
ATTR_LOG_STRING_LIB nvdm_026[] = "";
ATTR_LOG_STRING_LIB nvdm_027[] = "";
ATTR_LOG_STRING_LIB nvdm_028[] = "";
ATTR_LOG_STRING_LIB nvdm_029[] = "";
ATTR_LOG_STRING_LIB nvdm_030[] = "";
ATTR_LOG_STRING_LIB nvdm_031[] = "";
ATTR_LOG_STRING_LIB nvdm_032[] = "";
ATTR_LOG_STRING_LIB nvdm_034[] = "";
ATTR_LOG_STRING_LIB nvdm_035[] = "";
ATTR_LOG_STRING_LIB nvdm_036[] = "";
ATTR_LOG_STRING_LIB nvdm_037[] = "";
ATTR_LOG_STRING_LIB nvdm_038[] = "";
ATTR_LOG_STRING_LIB nvdm_039[] = "";
ATTR_LOG_STRING_LIB nvdm_040[] = "";
ATTR_LOG_STRING_LIB nvdm_041[] = "";
ATTR_LOG_STRING_LIB nvdm_042[] = "";
ATTR_LOG_STRING_LIB nvdm_043[] = "";
ATTR_LOG_STRING_LIB nvdm_044[] = "";
ATTR_LOG_STRING_LIB nvdm_045[] = "";
ATTR_LOG_STRING_LIB nvdm_046[] = "";
ATTR_LOG_STRING_LIB nvdm_047[] = "";
ATTR_LOG_STRING_LIB nvdm_048[] = "";
ATTR_LOG_STRING_LIB nvdm_049[] = "";
ATTR_LOG_STRING_LIB nvdm_050[] = "";
ATTR_LOG_STRING_LIB nvdm_051[] = "";
ATTR_LOG_STRING_LIB nvdm_052[] = "";
ATTR_LOG_STRING_LIB nvdm_053[] = "";
ATTR_LOG_STRING_LIB nvdm_054[] = "";
ATTR_LOG_STRING_LIB nvdm_055[] = "";
ATTR_LOG_STRING_LIB nvdm_056[] = "";
ATTR_LOG_STRING_LIB nvdm_057[] = "";
ATTR_LOG_STRING_LIB nvdm_058[] = "";
ATTR_LOG_STRING_LIB nvdm_059[] = "";
ATTR_LOG_STRING_LIB nvdm_060[] = "";
ATTR_LOG_STRING_LIB nvdm_061[] = "";
ATTR_LOG_STRING_LIB nvdm_062[] = "";
ATTR_LOG_STRING_LIB nvdm_063[] = "";
ATTR_LOG_STRING_LIB nvdm_064[] = "";
ATTR_LOG_STRING_LIB nvdm_065[] = "";
ATTR_LOG_STRING_LIB nvdm_066[] = "";
ATTR_LOG_STRING_LIB nvdm_067[] = "";
ATTR_LOG_STRING_LIB nvdm_068[] = "";
ATTR_LOG_STRING_LIB nvdm_069[] = "";
ATTR_LOG_STRING_LIB nvdm_070[] = "";
ATTR_LOG_STRING_LIB nvdm_071[] = "";
ATTR_LOG_STRING_LIB nvdm_072[] = "";
ATTR_LOG_STRING_LIB nvdm_073[] = "";
ATTR_LOG_STRING_LIB nvdm_074[] = "";
ATTR_LOG_STRING_LIB nvdm_075[] = "";
ATTR_LOG_STRING_LIB nvdm_076[] = "";
ATTR_LOG_STRING_LIB nvdm_077[] = "";
ATTR_LOG_STRING_LIB nvdm_078[] = "";
ATTR_LOG_STRING_LIB nvdm_079[] = "";
ATTR_LOG_STRING_LIB nvdm_080[] = "";
ATTR_LOG_STRING_LIB nvdm_081[] = "";
ATTR_LOG_STRING_LIB nvdm_082[] = "";
ATTR_LOG_STRING_LIB nvdm_083[] = "";
ATTR_LOG_STRING_LIB nvdm_084[] = "";
ATTR_LOG_STRING_LIB nvdm_085[] = "";
ATTR_LOG_STRING_LIB nvdm_086[] = "";
ATTR_LOG_STRING_LIB nvdm_087[] = "";
ATTR_LOG_STRING_LIB nvdm_088[] = "";
ATTR_LOG_STRING_LIB nvdm_089[] = "";
ATTR_LOG_STRING_LIB nvdm_090[] = "";
ATTR_LOG_STRING_LIB nvdm_091[] = "";
ATTR_LOG_STRING_LIB nvdm_092[] = "";
ATTR_LOG_STRING_LIB nvdm_093[] = "";
ATTR_LOG_STRING_LIB nvdm_094[] = "";
ATTR_LOG_STRING_LIB nvdm_095[] = "";
ATTR_LOG_STRING_LIB nvdm_096[] = "";
ATTR_LOG_STRING_LIB nvdm_097[] = "";
ATTR_LOG_STRING_LIB nvdm_098[] = "";
ATTR_LOG_STRING_LIB nvdm_099[] = "";
ATTR_LOG_STRING_LIB nvdm_100[] = "";
ATTR_LOG_STRING_LIB nvdm_101[] = "";
ATTR_LOG_STRING_LIB nvdm_102[] = "";
ATTR_LOG_STRING_LIB nvdm_103[] = "";
ATTR_LOG_STRING_LIB nvdm_104[] = "";
ATTR_LOG_STRING_LIB nvdm_105[] = "";
ATTR_LOG_STRING_LIB nvdm_106[] = "";
ATTR_LOG_STRING_LIB nvdm_107[] = "";
ATTR_LOG_STRING_LIB nvdm_108[] = "";
ATTR_LOG_STRING_LIB nvdm_109[] = "";
ATTR_LOG_STRING_LIB nvdm_110[] = "";
ATTR_LOG_STRING_LIB nvdm_111[] = "";
ATTR_LOG_STRING_LIB nvdm_112[] = "";
ATTR_LOG_STRING_LIB nvdm_113[] = "";
ATTR_LOG_STRING_LIB nvdm_114[] = "";
ATTR_LOG_STRING_LIB nvdm_115[] = "";
ATTR_LOG_STRING_LIB nvdm_116[] = "";
ATTR_LOG_STRING_LIB nvdm_117[] = "";
ATTR_LOG_STRING_LIB nvdm_118[] = "";
ATTR_LOG_STRING_LIB nvdm_119[] = "";
ATTR_LOG_STRING_LIB nvdm_120[] = "";
ATTR_LOG_STRING_LIB nvdm_121[] = "";
ATTR_LOG_STRING_LIB nvdm_122[] = "";
ATTR_LOG_STRING_LIB nvdm_123[] = "";
ATTR_LOG_STRING_LIB nvdm_124[] = "";
ATTR_LOG_STRING_LIB nvdm_125[] = "";
ATTR_LOG_STRING_LIB nvdm_126[] = "";
ATTR_LOG_STRING_LIB nvdm_127[] = "";
ATTR_LOG_STRING_LIB nvdm_128[] = "";
ATTR_LOG_STRING_LIB nvdm_129[] = "";
ATTR_LOG_STRING_LIB nvdm_130[] = "";

#endif /* !defined(MTK_DEBUG_LEVEL_NONE) */

#ifndef __EXT_BOOTLOADER__
log_create_module(nvdm, PRINT_LEVEL_WARNING);
#if !defined (MTK_DEBUG_LEVEL_NONE)

void nvdm_log_info(const char *message, ...)
{
    va_list ap;

    va_start(ap, message);
    // vprint_module_log(&LOG_CONTROL_BLOCK_SYMBOL(nvdm), __FUNCTION__, __LINE__, PRINT_LEVEL_INFO, message, ap);
    va_end(ap);
}

void nvdm_log_warning(const char *message, ...)
{
    va_list ap;

    va_start(ap, message);
    vprint_module_log(&LOG_CONTROL_BLOCK_SYMBOL(nvdm), __FUNCTION__, __LINE__, PRINT_LEVEL_WARNING, message, ap);
    va_end(ap);
}

void nvdm_log_error(const char *message, ...)
{
    va_list ap;

    va_start(ap, message);
    vprint_module_log(&LOG_CONTROL_BLOCK_SYMBOL(nvdm), __FUNCTION__, __LINE__, PRINT_LEVEL_ERROR, message, ap);
    va_end(ap);
}

void nvdm_log_msgid_info(const char *message, uint32_t arg_cnt, ...)
{
    va_list ap;

    va_start(ap, arg_cnt);
    // log_print_msgid(&LOG_CONTROL_BLOCK_SYMBOL(nvdm), PRINT_LEVEL_INFO, message, arg_cnt, ap);
    va_end(ap);
}

void nvdm_log_msgid_warning(const char *message, uint32_t arg_cnt, ...)
{
    va_list ap;

    va_start(ap, arg_cnt);
    log_print_msgid(&LOG_CONTROL_BLOCK_SYMBOL(nvdm), PRINT_LEVEL_WARNING, message, arg_cnt, ap);
    va_end(ap);
}

void nvdm_log_msgid_error(const char *message, uint32_t arg_cnt, ...)
{
    va_list ap;

    va_start(ap, arg_cnt);
    log_print_msgid(&LOG_CONTROL_BLOCK_SYMBOL(nvdm), PRINT_LEVEL_ERROR, message, arg_cnt, ap);
    va_end(ap);
}

#else /* MTK_DEBUG_LEVEL_NONE */

void nvdm_log_info(const char *message, ...)
{}

void nvdm_log_warning(const char *message, ...)
{}

void nvdm_log_error(const char *message, ...)
{}

void nvdm_log_msgid_info(const char *message, uint32_t arg_cnt, ...)
{}

void nvdm_log_msgid_warning(const char *message, uint32_t arg_cnt, ...)
{}

void nvdm_log_msgid_error(const char *message, uint32_t arg_cnt, ...)
{}

#endif /* MTK_DEBUG_LEVEL_NONE */
#endif /* __EXT_BOOTLOADER__ */


uint32_t nvdm_port_get_count(void)
{
#ifdef HAL_GPT_MODULE_ENABLED
    uint32_t count = 0;
    hal_gpt_get_free_run_count(HAL_GPT_CLOCK_SOURCE_32K, &count);
    return count;
#else
    return 0;
#endif
}


uint32_t nvdm_port_get_duration_time(uint32_t begin, uint32_t end)
{
#ifdef HAL_GPT_MODULE_ENABLED
    uint32_t count = 0;
    hal_gpt_get_duration_count(begin, end, &count);
    return (uint32_t)(count >> 5U);
#else
    return 0;
#endif
}


void nvdm_port_flash_read(uint32_t address, uint8_t *buffer, uint32_t length)
{
    hal_flash_status_t status;

    status = hal_flash_read(address, buffer, length);
    if (status != HAL_FLASH_STATUS_OK) {
#ifndef __EXT_BOOTLOADER__
        LOG_MSGID_E(nvdm, "hal_flash_read: address = 0x%x, buffer = 0x%x, length = %d, status = %d", 4, address, (uint32_t)buffer, length, status);
#else
        bl_print(LOG_ERROR, "hal_flash_read: address = %x, buffer = %x, length = %d, status = %d", address, (uint32_t)buffer, length, status);
#endif
        assert(0);
    }
}

void nvdm_port_flash_write(uint32_t address, const uint8_t *buffer, uint32_t length)
{
    hal_flash_status_t status;

    status = hal_flash_write(address, buffer, length);
    if (status != HAL_FLASH_STATUS_OK) {
#ifndef __EXT_BOOTLOADER__
        LOG_MSGID_E(nvdm, "hal_flash_write: address = 0x%x, buffer = 0x%x, length = %d, status = %d", 4, address, (uint32_t)buffer, length, status);
#else
        bl_print(LOG_ERROR, "hal_flash_write: address = %x, buffer = %x, length = %d, status = %d", address, (uint32_t)buffer, length, status);
#endif
        assert(0);
    }
}

/* erase unit is 4K large(which is size of PEB) */
void nvdm_port_flash_erase(uint32_t address)
{
    hal_flash_status_t status;

    status = hal_flash_erase(address, HAL_FLASH_BLOCK_4K);
    if (status != HAL_FLASH_STATUS_OK) {
#ifndef __EXT_BOOTLOADER__
        LOG_MSGID_E(nvdm, "hal_flash_erase: address = 0x%08x, status = %d", 2, address, status);
#else
        bl_print(LOG_ERROR, "hal_flash_erase: address = %x, status = %d", address, status);
#endif
        assert(0);
    }
}

void nvdm_port_must_assert(void)
{
    assert(0);
}

#ifdef FREERTOS_ENABLE
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

typedef SemaphoreHandle_t nvdm_port_mutex_t;
static nvdm_port_mutex_t g_nvdm_mutex, g_nvdm_protect_mutex;
static TaskHandle_t g_task_handler;

void nvdm_port_mutex_creat(void)
{
    /* g_nvdm_mutex = xSemaphoreCreateMutex(); */
    g_nvdm_mutex = xSemaphoreCreateRecursiveMutex();

    if (g_nvdm_mutex == NULL) {
        LOG_MSGID_E(nvdm, "nvdm_port_mutex_creat error\r\n", 0);
        assert(0);
        return;
    }
}

void nvdm_port_mutex_take(void)
{
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        /* if (xSemaphoreTake(g_nvdm_mutex, portMAX_DELAY) == pdFALSE) { */
        if (xSemaphoreTakeRecursive(g_nvdm_mutex, portMAX_DELAY) == pdFALSE) {
            LOG_MSGID_E(nvdm, "nvdm_port_mutex_take error\r\n", 0);
            assert(0);
            return;
        }
    }
}

void nvdm_port_mutex_give(void)
{
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        /* if (xSemaphoreGive(g_nvdm_mutex) == pdFALSE) { */
        if (xSemaphoreGiveRecursive(g_nvdm_mutex) == pdFALSE) {
            LOG_MSGID_E(nvdm, "nvdm_port_mutex_give error\r\n", 0);
            assert(0);
            return;
        }
    }
}

void nvdm_port_protect_mutex_create(void)
{
    g_nvdm_protect_mutex = xSemaphoreCreateRecursiveMutex();

    if (g_nvdm_protect_mutex == NULL) {
        LOG_MSGID_E(nvdm, "nvdm_port_protect_mutex_create error\r\n", 0);
        assert(0);
        return;
    }
}

void nvdm_port_protect_mutex_take(void)
{
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        if (xSemaphoreTakeRecursive(g_nvdm_protect_mutex, portMAX_DELAY) == pdFALSE) {
            LOG_MSGID_E(nvdm, "nvdm_port_protect_mutex_take error\r\n", 0);
            assert(0);
            return;
        }
    }
}

void nvdm_port_protect_mutex_give(void)
{
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        if (xSemaphoreGiveRecursive(g_nvdm_protect_mutex) == pdFALSE) {
            LOG_MSGID_E(nvdm, "nvdm_port_protect_mutex_give error\r\n", 0);
            assert(0);
            return;
        }
    }
}

void *nvdm_port_malloc(uint32_t size)
{
    return pvPortMalloc(size);
}

void nvdm_port_free(void *pdata)
{
    vPortFree(pdata);
}

void nvdm_port_get_task_handler(void)
{
    g_task_handler = xTaskGetCurrentTaskHandle();
}

void nvdm_port_reset_task_handler(void)
{
    g_task_handler = (TaskHandle_t)NULL;
}

bool nvdm_port_query_task_handler(void)
{
    if (xTaskGetCurrentTaskHandle() != g_task_handler) {
        return false;
    }

    return true;
}


const char *nvdm_port_get_curr_task_name(void)
{
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        return (const char *)pcTaskGetName(NULL);
    } else {
        return (const char *)"main";
    }
}

#ifdef SYSTEM_DAEMON_TASK_ENABLE
#include "system_daemon.h"

bool nvdm_port_send_queue(void)
{
    BaseType_t ret;

    ret = system_daemon_send_message(SYSTEM_DAEMON_ID_NVDM, NULL);
    if (ret != pdPASS) {
        return false;
    }

    return true;
}

bool nvdm_request_gc_in_daemon(const void *para)
{
    BaseType_t ret;

    ret = system_daemon_send_message(SYSTEM_DAEMON_ID_NVDM_GC, para);
    if (ret != pdPASS) {
        return false;
    }

    return true;
}
#else
bool nvdm_port_send_queue(void)
{
    return false;
}

bool nvdm_req_gc_in_daemon(const void *para)
{
    return false;
}
#endif

#else

#include "malloc.h"
void nvdm_port_mutex_creat(void)
{}

void nvdm_port_mutex_take(void)
{}

void nvdm_port_mutex_give(void)
{}

void nvdm_port_protect_mutex_create(void)
{}

void nvdm_port_protect_mutex_take(void)
{}

void nvdm_port_protect_mutex_give(void)
{}

void *nvdm_port_malloc(uint32_t size)
{
#ifdef __EXT_BOOTLOADER__
    /* bootloader not support malloc */
    (void)size;
    bl_print(LOG_ERROR, "BL environment not support malloc");
    return NULL;
#else
    return malloc(size);
#endif
}

void nvdm_port_free(void *pdata)
{
#ifdef __EXT_BOOTLOADER__
    /* bootloader not support free */
    (void)pdata;
    bl_print(LOG_ERROR, "BL environment not support free");
#else
    free(pdata);
#endif
}

void nvdm_port_get_task_handler(void)
{}

void nvdm_port_reset_task_handler(void)
{}

bool nvdm_port_query_task_handler(void)
{
    return true;
}

const char *nvdm_port_get_curr_task_name(void)
{
    return (const char *)"main";
}

bool nvdm_port_send_queue(void)
{
    return false;
}

bool nvdm_req_gc_in_daemon(const void *para)
{
    return false;
}

#endif


#include "nvdm_config.h"

static nvdm_partition_cfg_t cfg_array[] = {
    /* Non-FOTAble NVDM Partition */
    {
        0xDEADBEEF,
        NVDM_PORT_PEB_SIZE,
        0xDEADBEEF,
        NVDM_PORT_MAX_DATA_ITEM_SIZE,
        NVDM_PORT_GROUP_NAME_MAX_LENGTH,
        NVDM_PORT_DATA_ITEM_NAME_MAX_LENGTH,
        NVDM_PORT_DAT_ITEM_COUNT
    },
    /* FOTAble NVDM Partition */
    {
        0xDEADBEEF,
        NVDM_PORT_PEB_SIZE,
        0xDEADBEEF,
        NVDM_PORT_MAX_DATA_ITEM_SIZE,
        NVDM_PORT_GROUP_NAME_MAX_LENGTH,
        NVDM_PORT_DATA_ITEM_NAME_MAX_LENGTH,
        NVDM_PORT_DAT_ITEM_COUNT
    },
};
#define MVDM_PARTITION_NUMBER (sizeof(cfg_array)/sizeof(nvdm_partition_cfg_t))


nvdm_partition_cfg_t *nvdm_port_load_partition_info(uint32_t *partition_num)
{
    uint32_t idx;
    for (idx = 0; idx < MVDM_PARTITION_NUMBER; idx++) {
        switch (idx) {
            case 0:
                cfg_array[idx].base_addr = ROM_NVDM_BASE;
                cfg_array[idx].peb_count = ROM_NVDM_LENGTH / NVDM_PORT_PEB_SIZE;
                break;
            case 1:
                cfg_array[idx].base_addr = ROM_NVDM_OU_BASE;
                cfg_array[idx].peb_count = ROM_NVDM_OU_LENGTH / NVDM_PORT_PEB_SIZE;
                break;
            default:
#ifndef __EXT_BOOTLOADER__
                LOG_MSGID_E(nvdm, "cfg_array error( %u <-> %u)\r\n", 2, idx, MVDM_PARTITION_NUMBER);
#else
                bl_print(LOG_ERROR, "cfg_array error( %u <-> %u)\r\n", idx, MVDM_PARTITION_NUMBER);
#endif
                nvdm_port_must_assert();
                break;
        }
    }
    *partition_num = MVDM_PARTITION_NUMBER;
    return cfg_array;
}


bool nvdm_port_get_max_item_cfg(nvdm_partition_cfg_t *p_cfg)
{
    bool result = true;
    uint32_t idx;
    if (p_cfg == NULL) {
        result = false;
        return result;
    }
    p_cfg->base_addr = 0;
    p_cfg->peb_size = 0;
    p_cfg->peb_count = 0;

    p_cfg->max_item_size = 0;
    p_cfg->max_group_name_size = 0;
    p_cfg->max_item_name_size = 0;
    p_cfg->total_item_count = 0;
    for (idx = 0; idx < MVDM_PARTITION_NUMBER; idx++) {
        if (p_cfg->max_item_size < cfg_array[idx].max_item_size) {
            p_cfg->max_item_size = cfg_array[idx].max_item_size;
        }

        if (p_cfg->max_group_name_size < cfg_array[idx].max_group_name_size) {
            p_cfg->max_group_name_size = cfg_array[idx].max_group_name_size;
        }

        if (p_cfg->max_item_name_size < cfg_array[idx].max_item_name_size) {
            p_cfg->max_item_name_size = cfg_array[idx].max_item_name_size;
        }

        if (p_cfg->total_item_count < cfg_array[idx].total_item_count) {
            p_cfg->total_item_count = cfg_array[idx].total_item_count;
        }
    }
    return result;
}


/* Before operating the flash, it will go through the function nvdm_port_get_peb_address,
 * and it is necessary to deal with the situation of illegal parameters uniformly.
 * If it is within the block or offset range, it means it is valid, otherwise an assert is required.
 */
uint32_t nvdm_port_get_peb_address(uint32_t partition, int32_t pnum, int32_t offset)
{
    if (partition >= MVDM_PARTITION_NUMBER) {
#ifndef __EXT_BOOTLOADER__
        LOG_MSGID_E(nvdm, "parameter partition: %d, allow partition: %d\r\n", 2, partition, MVDM_PARTITION_NUMBER);
#else
        bl_print(LOG_ERROR, "parameter partition: %d, allow partition: %d\r\n", partition, MVDM_PARTITION_NUMBER);
#endif
        nvdm_port_must_assert();
        return 0xDEADBEEF;
    }
    if ((pnum >= cfg_array[partition].peb_count) || (offset >= cfg_array[partition].peb_size)) {
#ifndef __EXT_BOOTLOADER__
        LOG_MSGID_E(nvdm, "nvdm_port_get_peb_address error, partition: %d, pnum: %d, offset: %d\r\n", 3,
                    partition, pnum, offset);
#else
        bl_print(LOG_ERROR, "nvdm_port_get_peb_address error, partition: %d, pnum: %d, offset: %d\r\n", partition, pnum, offset);
#endif
        nvdm_port_must_assert();
        return 0xDEADBEEF;
    }
    return cfg_array[partition].base_addr + pnum * cfg_array[partition].peb_size + offset;
}

void nvdm_port_poweroff_time_set(void)
{}
void nvdm_port_poweroff(uint32_t poweroff_time)
{}

#endif


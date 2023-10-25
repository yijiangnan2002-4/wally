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

/*
 * [regions of PEBs]
 * we maintain NVDM_PEB_COUNT logic erase blocks, including main PEBs and reserved PEBs.
 * main PEBs are used to store data items header and actural data.
 * reserved PEBs are used in garbage collection to store data item's content merged from other main dirty PEBs.
 *
 * [composition of every PEB]
 * dirty data + valid data + free space
 *
 * [Flow for PEB allocation]
 * 1, find most best fit PEB with required free space;
 * 2, if found, return number and offset of PEB;
 * 3, if not found, begin to start garbage collection;
 * 4, scan PEBs to find victims, and it's criteria is:
 *      - find erase count of PEB less than average erase count to much;
 *      - try to merge PEBs to reserved PEBs;
 * 5, sort victims PEBs with valid data items size;
 * 6, try to merge PEBs to reserved PEBs;
 *      - at least two PEBs can be merge into one reserved PEB;
 * 7, if merge fail, directlly reclaim least valid size space PEB;
 *      - we should report this situation;
 */

#include "nvdm.h"
#include "nvdm_port.h"
#include "nvdm_internal.h"
#include "nvdm_msgid_log.h"

nvdm_ctrl_block_t g_ncb;

#ifdef __EXT_BOOTLOADER__

#include "nvdm_config.h"

#define NVDM_PARTITION_NUMBER 2
static nvdm_partition_info_t s_partition_info[NVDM_PARTITION_NUMBER];
static int32_t s_gc_buffer[NVDM_PARTITION_NUMBER][NVDM_PEB_NUMBER];
static peb_info_t s_peb_info[NVDM_PARTITION_NUMBER][NVDM_PEB_NUMBER];
static data_item_header_t s_item_hdrs[NVDM_PARTITION_NUMBER][NVDM_PORT_DAT_ITEM_COUNT];
#endif

void peb_print_info(uint32_t num)
{
    uint32_t i;
    nvdm_partition_info_t *p_info = &(g_ncb.partition[num]);
    nvdm_partition_cfg_t *p_cfg = p_info->p_cfg;

    nvdm_log_msgid_warning(nvdm_049, 0);
    nvdm_log_msgid_warning(nvdm_050, 0);
    for (i = 0; i < g_ncb.partition[num].p_cfg->peb_count; i++) {
        nvdm_log_msgid_warning(nvdm_051, 6,
                               i,
                               g_ncb.partition[num].p_cfg->peb_size - PEB_HEADER_SIZE - p_info->pebs[i].free - p_info->pebs[i].dirty,
                               p_info->pebs[i].free,
                               p_info->pebs[i].dirty,
                               p_info->pebs[i].erase_count,
                               p_info->pebs[i].is_reserved);
    }
    nvdm_log_msgid_warning(nvdm_052, 1, g_ncb.partition[num].valid_data_size);
    nvdm_log_msgid_warning(nvdm_091, 7,
                           p_cfg->max_item_size,
                           p_cfg->max_group_name_size,
                           p_cfg->max_item_name_size,
                           p_cfg->total_item_count,
                           p_cfg->peb_size,
                           p_cfg->peb_count,
                           p_info->avaliable_space
                          );
}

static void peb_header_print_info(uint32_t peb_index, peb_header_t *peb_hdr)
{
#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_053, 1, peb_index);
    nvdm_log_msgid_info(nvdm_054, 1, peb_hdr->magic);
    nvdm_log_msgid_info(nvdm_055, 1, peb_hdr->erase_count);
    nvdm_log_msgid_info(nvdm_056, 1, peb_hdr->status);
    nvdm_log_msgid_info(nvdm_057, 1, peb_hdr->peb_reserved);
    nvdm_log_msgid_info(nvdm_058, 1, peb_hdr->version);
#else
    AVOID_UNUSED_BUILD_WARNING(peb_index);
    AVOID_UNUSED_BUILD_WARNING(peb_hdr);
#endif
}

static bool peb_header_is_validate(peb_header_t *peb_header, int32_t is_empty)
{
    if (peb_header->magic != PEB_HEADER_MAGIC) {
        return false;
    }

    if (peb_header->erase_count == ERASE_COUNT_MAX) {
        return false;
    }

    if (peb_header->version != NVDM_VERSION) {
        return false;
    }

    if (is_empty) {
        if ((peb_header->status != PEB_STATUS_EMPTY) ||
            (peb_header->peb_reserved != 0xFF)) {
            return false;
        }
    } else {
        if (peb_header->peb_reserved == 0xFF) {
            return false;
        }
    }

    return true;
}

void peb_read_header(uint32_t partition, int32_t pnum, peb_header_t *peb_header)
{
    uint8_t buf[PEB_HEADER_SIZE];

    peb_io_read(partition, pnum, 0, buf, PEB_HEADER_SIZE);

    if (peb_header) {
        *peb_header = *(peb_header_t *)buf;
    }
}

void peb_write_data(uint32_t partition, int32_t pnum, int32_t offset, const uint8_t *buf, int32_t len)
{
    int32_t ret;
    peb_header_t peb_header;

    /*
     * We write to the data area of the physical eraseblock. Make
     * sure it has valid EC headers.
     */
    peb_read_header(partition, pnum, &peb_header);
    ret = peb_header_is_validate(&peb_header, 0);
    if (ret == false) {
        nvdm_log_msgid_error(nvdm_063, 4,
                             peb_header.magic, peb_header.erase_count,
                             peb_header.status, peb_header.peb_reserved);

        /* Actively trigger assert to erase and
         * reinitialize the block during the next boot process
         * by using function peb_scan.
         */
        nvdm_port_must_assert();
        return;
    }

    peb_io_write(partition, pnum, PEB_HEADER_SIZE + offset, buf, len);
}

void peb_read_data(uint32_t partition, int32_t pnum, int32_t offset, uint8_t *buf, int32_t len)
{
    peb_io_read(partition, pnum, PEB_HEADER_SIZE + offset, buf, len);
}

void peb_update_status(uint32_t partition, int32_t pnum, peb_status_t status)
{
    peb_io_write(partition, pnum, PEB_STATUS_OFFSET, (uint8_t *)&status, 1);
}

static void peb_write_hdr_erase_count(uint32_t partition, int32_t pnum, uint32_t erase_count)
{
    peb_io_write(partition, pnum, PEB_ERASE_COUNT_OFFSET, (uint8_t *)&erase_count, 4);
}

static void peb_write_hdr_peb_reserved(uint32_t partition, int32_t pnum)
{
    uint8_t unreserved_mark = PEB_UNRESERVED;

    peb_io_write(partition, pnum, PEB_RESERVED_OFFSET, &unreserved_mark, 1);
}

static void peb_write_hdr_magic_number(uint32_t partition, int32_t pnum)
{
    uint32_t magic_number;

    magic_number = PEB_HEADER_MAGIC;
    peb_io_write(partition, pnum, PEB_MAGIC_OFFSET, (uint8_t *)&magic_number, 4);
}

static void peb_write_hdr_version(uint32_t partition, int32_t pnum)
{
    uint8_t version = NVDM_VERSION;

    peb_io_write(partition, pnum, PEB_VERSION_OFFSET, &version, 1);
}

int32_t peb_activing(uint32_t partition, int32_t pnum)
{
    peb_header_t hdr;
    nvdm_partition_info_t *p_info = &(g_ncb.partition[partition]);
    nvdm_partition_cfg_t *p_cfg = p_info->p_cfg;

    peb_read_header(partition, pnum, &hdr);
    if ((p_info->pebs[pnum].free == (p_cfg->peb_size - PEB_HEADER_SIZE)) &&
        (hdr.status >= PEB_STATUS_ACTIVING)
       ) {
        peb_update_status(partition, pnum, PEB_STATUS_ACTIVING);
#if MARK_UNUSED_POWEROFF_CALL
        nvdm_port_poweroff(9);
#endif
        peb_write_hdr_peb_reserved(partition, pnum);
        p_info->pebs[pnum].is_reserved = 0;
        return 1;
    }

    return 0;
}

static void peb_reclaim(uint32_t partition, int32_t pnum)
{
    nvdm_partition_info_t *p_info = &(g_ncb.partition[partition]);
    nvdm_partition_cfg_t *p_cfg = p_info->p_cfg;

    peb_update_status(partition, pnum, PEB_STATUS_ERASING);
#if MARK_UNUSED_POWEROFF_CALL
    nvdm_port_poweroff(10);
#endif
    peb_erase(partition, pnum);
    peb_write_hdr_magic_number(partition, pnum);
    peb_write_hdr_erase_count(partition, pnum, ++(p_info->pebs[pnum].erase_count));
    peb_write_hdr_version(partition, pnum);
    peb_update_status(partition, pnum, PEB_STATUS_EMPTY);
#if MARK_UNUSED_POWEROFF_CALL
    nvdm_port_poweroff(11);
#endif

    p_info->pebs[pnum].is_reserved = 1;
    p_info->pebs[pnum].dirty = 0;
    p_info->pebs[pnum].free = (p_cfg->peb_size - PEB_HEADER_SIZE);
}

/* relocate one or more pebs with large of dirty data to one new empty peb */
static void relocate_pebs(uint32_t partition, int32_t *source_pebs, int32_t source_peb_count)
{
    int32_t i, offset;
    int32_t target_peb;
    uint32_t least_erase_count, total_valid_data;
    nvdm_partition_info_t *p_info = &(g_ncb.partition[partition]);
    nvdm_partition_cfg_t *p_cfg = p_info->p_cfg;

    /* there is a special case that all source_pebs cantain no valid data at all.
     * So we just reclaim these pebs and erase them.
     */
    total_valid_data = 0;
    for (i = 0; i < source_peb_count; i++) {
        total_valid_data += (p_cfg->peb_size - PEB_HEADER_SIZE) -
                            p_info->pebs[source_pebs[i]].dirty -
                            p_info->pebs[source_pebs[i]].free;
    }
    if (total_valid_data == 0) {
#if MARK_UNUSED_LOG_INFO_CALL
        nvdm_log_msgid_info(nvdm_072, 0);
#endif
        for (i = 0; i < source_peb_count; i++) {
            peb_reclaim(partition, source_pebs[i]);
        }

        return;
    }

    /* mark source pebs which is needed relocation */
    for (i = 0; i < source_peb_count; i++) {
        peb_update_status(partition, source_pebs[i], PEB_STATUS_RECLAIMING);
#if MARK_UNUSED_POWEROFF_CALL
        nvdm_port_poweroff(12);
#endif
    }

    /* search a target peb and mark it */
    least_erase_count = ERASE_COUNT_MAX;
    target_peb = p_cfg->peb_count;
    for (i = 0; i < p_cfg->peb_count; i++) {
        if (!p_info->pebs[i].is_reserved) {
            continue;
        }
        if (least_erase_count > p_info->pebs[i].erase_count) {
            target_peb = i;
            least_erase_count = p_info->pebs[i].erase_count;
        }
    }
    if (target_peb >= p_cfg->peb_count) {
        /* 1. For the case of erase count of all blocks are 0xFFFF_FFFF,
         * it seems unpossible to happen, but keep the code.
         * 2. For the case of no available reserved block while doing GC, we need to do some special processing in the flash driver.
         * NVDM provides the nvdm_port_enable_write API to determine
         * whether it is in the NVDM interval in the incoming address and writing range, and whether it is written by NVDM.
         */
        nvdm_log_msgid_error(nvdm_073, 1, target_peb);
        return;
    }
#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_074, 1, target_peb);
#endif
    peb_update_status(partition, target_peb, PEB_STATUS_ACTIVING);
#if MARK_UNUSED_POWEROFF_CALL
    nvdm_port_poweroff(13);
#endif
    peb_write_hdr_peb_reserved(partition, target_peb);
    p_info->pebs[target_peb].is_reserved = 0;
    peb_update_status(partition, target_peb, PEB_STATUS_TRANSFERING);
#if MARK_UNUSED_POWEROFF_CALL
    nvdm_port_poweroff(14);
#endif
    /* begin transfer data from source pebs to target peb */
    offset = 0;
    for (i = 0; i < source_peb_count; i++) {
        offset = data_item_migration(partition, source_pebs[i], target_peb, offset);
    }

    /* We can't dirrectly update status of discard pebs to PEB_STATUS_ACTIVED,
     * or we can't deal with those pebs in init flow if power-off happen below.
     */
    peb_update_status(partition, target_peb, PEB_STATUS_TRANSFERED);
#if MARK_UNUSED_POWEROFF_CALL
    nvdm_port_poweroff(15);
#endif
    p_info->pebs[target_peb].is_reserved = 0;

    /* put back all discard pebs to wear-leaving */
    for (i = 0; i < source_peb_count; i++) {
        peb_reclaim(partition, source_pebs[i]);
    }

    /* Now we can update status of target peb to PEB_STATUS_ACTIVED safety */
    peb_update_status(partition, target_peb, PEB_STATUS_ACTIVED);
#if MARK_UNUSED_POWEROFF_CALL
    nvdm_port_poweroff(16);
#endif
}


/* Define max number of PEBs during each round of reclaim. */
#define MAX_MERGE_PEBS_PER_ROUND 3
/* Define max loop count during each garbage reclaim. */
#define MAX_MERGE_LOOP_PER_GARBAGE_COLLECTION 1
static void garbage_reclaim_pebs(uint32_t partition, int32_t found_blocks, int32_t *peb_list)
{
    int32_t i, sum_valid, merge_cnt, merge_start, hard_merge, merge_loop, curr_valid, empty_pebs;
    uint32_t valid_data;
    nvdm_partition_info_t *p_info = &(g_ncb.partition[partition]);
    nvdm_partition_cfg_t *p_cfg = p_info->p_cfg;

    /* try to merge PEBs to reserved PEBs as many as possible.
       * The max number of merge PEBs and the max of merge loop need to be limited to prevent too many time stay here.
       * Also peb_list[] is list in order, so we need to handle merge of empty PEBs.
       */
    merge_start = 0;
    hard_merge = 0;
    merge_loop = MAX_MERGE_LOOP_PER_GARBAGE_COLLECTION;
    do {
        sum_valid = 0;
        merge_cnt = 0;
        empty_pebs = 0;
        for (i = merge_start; i < found_blocks; i++) {
            curr_valid = (p_cfg->peb_size - PEB_HEADER_SIZE) - p_info->pebs[peb_list[i]].dirty - p_info->pebs[peb_list[i]].free;
            if (curr_valid == 0) {
                empty_pebs++;
            } else {
                if (empty_pebs > 0) {
                    /* Finding an empty data block. */
                    break;
                } else {
                    sum_valid += curr_valid;
                    if (sum_valid > (p_cfg->peb_size - PEB_HEADER_SIZE)) {
                        break;
                    }
                }
            }
            merge_cnt++;
            if (merge_cnt >= MAX_MERGE_PEBS_PER_ROUND) {
                break;
            }
        }

#ifndef NVDM_SLIM_LOG
        for (i = merge_start; i < merge_start + merge_cnt; i++) {
            valid_data = p_cfg->peb_size - PEB_HEADER_SIZE - p_info->pebs[peb_list[i]].dirty - p_info->pebs[peb_list[i]].free;
            nvdm_log_msgid_warning(nvdm_075, 2, peb_list[i], valid_data);
        }
#endif
        /* no need to continue merge if merge_cnt less than 2 */
        if (merge_cnt < 2) {
            hard_merge = 1;
        }
        /* now let us start merging operation */
        relocate_pebs(partition, &peb_list[merge_start], merge_cnt);
        /* if merge loop reach limit or merge hard, stop merge attemption again. */
        if ((--merge_loop == 0) || (hard_merge == 1)) {
            break;
        }
        merge_start += merge_cnt;
    } while (merge_start < found_blocks);
}

/* NVDM will start garbage collection for PEBs
 * with PEB_ERASE_COUNT_CRITERION less than average erase count
 */
#define PEB_ERASE_COUNT_CRITERION 1
static void garbage_collection_peb(uint32_t partition, uint32_t gc_round)
{
    int32_t i, j, max;
    int32_t cur_dirty, tmp_dirty;
    uint32_t cur_erase_count, tmp_erase_count;
    int32_t found_blocks, non_reserved_pebs;
    int32_t *peb_list, tmp_peb;
    uint64_t mean_erase_count;
    const uint32_t max_allowed_gc_round = 3;
    nvdm_partition_info_t *p_info = &(g_ncb.partition[partition]);
    nvdm_partition_cfg_t *p_cfg = p_info->p_cfg;

#ifndef NVDM_SLIM_LOG
    uint32_t count[2], duration_time;

    nvdm_log_msgid_warning(nvdm_076, 0);
    count[0] = nvdm_port_get_count();
#endif

    peb_list = p_info->gc_buffer;

    /* Method 1: Reclaim PEBs by erase count. */
    /* 1. scan all non-reserved pebs to calculate average erase counter. */
    mean_erase_count = 0;
    non_reserved_pebs = 0;
    for (i = 0; i < p_cfg->peb_count; i++) {
        if (p_info->pebs[i].is_reserved) {
            continue;
        }
        mean_erase_count += p_info->pebs[i].erase_count;
        non_reserved_pebs++;
    }

    if (non_reserved_pebs != 0) {
        mean_erase_count /= non_reserved_pebs;
    } else {
        /* If there is no data in NVDM Partition,
         * but when garbage collection is actively triggered,
         * non_reserved_pebs may be 0.
         */
        for (i = 0; i < p_cfg->peb_count; i++) {
            mean_erase_count += p_info->pebs[i].erase_count;
        }
        mean_erase_count /= p_cfg->peb_count;
    }
#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_078, 1, non_reserved_pebs);
    nvdm_log_msgid_info(nvdm_079, 1, mean_erase_count);
#endif
    /* 2. find all PEBs with erase counter below threshold. */
    found_blocks = 0;
    for (i = 0; i < p_cfg->peb_count; i++) {
        /* If a certain write sequence occurs, resulting in one or more BLOCKs,
         * its erasure times are less than 5% of the average, but there are many valid data in it.
         * If the number of garbage collection rounds is not limited,
         * it may cause garbage collection to move data between these blocks.
         * In this way, the time to write certain data will be much longer.
         */

        if (gc_round >= max_allowed_gc_round) {
            found_blocks = 0;
            break;
        }
        if (p_info->pebs[i].is_reserved) {
            continue;
        }
        /* Change the variable of the PEB_ERASE_COUNT_CRITERION macro from 5% to 1%.
         * NVDM will try to ensure the balance of the erase times,
         * and the difference between the erase times of each BLOCK and the average erase times is as small as 1%.
         * However, if a certain write sequence occurs, which causes the number of garbage collection rounds
         * to exceed the maximum allowable number of rounds,
         * NVDM needs to recycle according to valid data to ensure efficiency.
         */
        if (p_info->pebs[i].erase_count < (mean_erase_count * (100 - PEB_ERASE_COUNT_CRITERION)) / 100) {
            peb_list[found_blocks++] = i;
        }
    }
    /* 3. sort victims PEBs with erase count */
    if (found_blocks) {
#if MARK_UNUSED_LOG_INFO_CALL
        nvdm_log_msgid_info(nvdm_080, 1, found_blocks);
        nvdm_log_msgid_info(nvdm_081, 0);
        for (i = 0; i < found_blocks; i++) {
            nvdm_log_msgid_info(nvdm_082, 1, peb_list[i]);
        }
#endif
        for (i = 0; i < found_blocks; i++) {
            cur_erase_count = p_info->pebs[peb_list[i]].erase_count;
            max = i;
            for (j = i; j < found_blocks; j++) {
                tmp_erase_count = p_info->pebs[peb_list[j]].erase_count;
                if (cur_erase_count > tmp_erase_count) {
                    cur_erase_count = tmp_erase_count;
                    max = j;
                }
            }
            if (i != max) {
                tmp_peb = peb_list[max];
                peb_list[max] = peb_list[i];
                peb_list[i] = tmp_peb;
            }
        }
#if MARK_UNUSED_LOG_INFO_CALL
        nvdm_log_msgid_info(nvdm_083, 0);
        for (i = 0; i < found_blocks; i++) {
            nvdm_log_msgid_info(nvdm_084, 1, peb_list[i]);
        }
#endif
    }

    /* Method 2: Reclaim PEBs by valid size. */
    /* 1. find all non researved PEBs. */
    if (found_blocks == 0) {
        for (i = 0; i < p_cfg->peb_count; i++) {
            if (p_info->pebs[i].is_reserved) {
                continue;
            }
            peb_list[found_blocks++] = i;
        }
#if MARK_UNUSED_LOG_INFO_CALL
        nvdm_log_msgid_info(nvdm_085, 1, found_blocks);
        nvdm_log_msgid_info(nvdm_086, 0);
        for (i = 0; i < found_blocks; i++) {
            nvdm_log_msgid_info(nvdm_087, 1, peb_list[i]);
        }
#endif
    }
    /* 2. sort in descending order according to the dirty size in victims PEBs. */
    for (i = 0; i < found_blocks; i++) {
        cur_dirty = p_info->pebs[peb_list[i]].dirty;
        max = i;
        for (j = i; j < found_blocks; j++) {
            tmp_dirty = p_info->pebs[peb_list[j]].dirty;
            if (cur_dirty < tmp_dirty) {
                cur_dirty = tmp_dirty;
                max = j;
            }
        }
        if (i != max) {
            tmp_peb = peb_list[max];
            peb_list[max] = peb_list[i];
            peb_list[i] = tmp_peb;
        }
    }
#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_088, 0);
    for (i = 0; i < found_blocks; i++) {
        nvdm_log_msgid_info(nvdm_089, 1, peb_list[i]);
    }
#endif
    /* 3. begin to reclaim those pebs */
    garbage_reclaim_pebs(partition, found_blocks, peb_list);

#ifndef NVDM_SLIM_LOG
    count[1] = nvdm_port_get_count();
    duration_time = nvdm_port_get_duration_time(count[0], count[1]);
    nvdm_log_msgid_warning(nvdm_115, 1, duration_time);
#endif
}

static int32_t find_free_peb(uint32_t partition, int32_t size)
{
    nvdm_partition_info_t *p_info = &(g_ncb.partition[partition]);
    nvdm_partition_cfg_t *p_cfg = p_info->p_cfg;
    int32_t i, reserved_peb = -1, reserved_peb_cnt, target_peb = -1;
    int32_t min_free_space = (p_cfg->peb_size - PEB_HEADER_SIZE);
    uint32_t least_erase_count;

    /* find in non-reserved pebs frist */
    least_erase_count = ERASE_COUNT_MAX;
    reserved_peb_cnt = 0;
    for (i = 0; i < p_cfg->peb_count; i++) {
        if (p_info->pebs[i].is_reserved) {
            reserved_peb_cnt++;
            if (least_erase_count > p_info->pebs[i].erase_count) {
                reserved_peb = i;
                least_erase_count = p_info->pebs[i].erase_count;
            }
            continue;
        }
        if (p_info->pebs[i].free > size) {
            if ((target_peb < 0) || (p_info->pebs[i].free < min_free_space)) {
                min_free_space = p_info->pebs[i].free;
                target_peb = i;
            }
        }
    }

#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_090, 3, target_peb, reserved_peb, reserved_peb_cnt);
#endif

    if (target_peb >= 0) {
        return target_peb;
    }

    /* use reserved peb if we have (exclude backup peb) */
    if (reserved_peb_cnt > NVDM_RESERVED_PEB_COUNT) {
        target_peb = reserved_peb;
    }

    return target_peb;
}

/* allocate a logic erase block with at least free space size */
int32_t space_allocation(uint32_t partition, int32_t alloc_size, int32_t added_size, int32_t *poffset, const char *group_name, const char *item_name)
{
    int32_t target_peb = -1;
    const char *task_name = nvdm_port_get_curr_task_name();
    uint32_t gc_count = 0;
    nvdm_partition_info_t *p_info = &(g_ncb.partition[partition]);
    nvdm_partition_cfg_t *p_cfg = p_info->p_cfg;

    target_peb = find_free_peb(partition, alloc_size);
    while (target_peb < 0) {
        ++gc_count;
        garbage_collection_peb(partition, gc_count);

        target_peb = find_free_peb(partition, alloc_size);
        if (target_peb >= 0) {
            break;
        }
    }
    if (gc_count != 0) {
        nvdm_log_warning("task %s write { %u, %s, %s } GC %d",
                         task_name, partition,
                         group_name, item_name, gc_count
                        );
    }

    *poffset = (p_cfg->peb_size - PEB_HEADER_SIZE) - p_info->pebs[target_peb].free;

    /* Modify this variable after the data is actually written. */
    /* p_info->valid_data_size += added_size; */

    return target_peb;
}


/* This function decides max avail size of NVDM's region.
 * Normally we consider two factors:
 *  - Max size of data item during all user defined data items.
 *  - Limit total size of data items so that garbage collection don't happen too frequently.
 */
#define NVDM_MAX_USAGE_RATIO 80
static uint32_t calculate_total_avail_space(uint32_t partition)
{
    uint32_t max_reasonable_size, criteria1, criteria2;
    nvdm_partition_info_t *p_info = &(g_ncb.partition[partition]);
    nvdm_partition_cfg_t *p_cfg = p_info->p_cfg;

    criteria1 = (NVDM_MAX_USAGE_RATIO *
                 (p_cfg->peb_size - PEB_HEADER_SIZE) *
                 (p_cfg->peb_count - NVDM_RESERVED_PEB_COUNT)) / 100;
    criteria2 = (p_cfg->peb_count - NVDM_RESERVED_PEB_COUNT) *
                (p_cfg->peb_size - PEB_HEADER_SIZE - p_cfg->max_item_size - DATA_ITEM_HEADER_SIZE - DATA_ITEM_CHECKSUM_SIZE - p_cfg->max_group_name_size - p_cfg->max_item_name_size - 2);
    if (criteria1 > criteria2) {
        max_reasonable_size = criteria2;
    } else {
        max_reasonable_size = criteria1;
    }

    return max_reasonable_size;
}

bool space_is_enough(uint32_t partition, int32_t size)
{
    nvdm_partition_info_t *p_info = &(g_ncb.partition[partition]);

#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_092, 2, p_info->valid_data_size, size);
#endif

    if ((p_info->valid_data_size + size) > p_info->avaliable_space) {
        return false;
    }

    return true;
}

void space_sub_valid(uint32_t partition, int32_t size)
{
    nvdm_partition_info_t *p_info = &(g_ncb.partition[partition]);
    if ((int32_t)p_info->valid_data_size < size) {
        /* Ignore & Error log & Assert Once.
         * Memory corruption occurred, resulting in incorrect variables.
         */
        nvdm_log_msgid_error(nvdm_093, 0);
        nvdm_port_must_assert();
    }
    p_info->valid_data_size -= size;
}

void peb_add_drity(uint32_t partition, int32_t pnum, int32_t drity)
{
    g_ncb.partition[partition].pebs[pnum].dirty += drity;
}

void peb_add_free(uint32_t partition, int32_t pnum, int32_t free)
{
    g_ncb.partition[partition].pebs[pnum].free += free;
}

void peb_sub_free(uint32_t partition, int32_t pnum, int32_t free)
{
    g_ncb.partition[partition].pebs[pnum].free -= free;
}

static void peb_reinit(uint32_t partition, int32_t pnum)
{
    peb_erase(partition, pnum);
    peb_write_hdr_magic_number(partition, pnum);
    peb_write_hdr_version(partition, pnum);
    peb_update_status(partition, pnum, PEB_STATUS_EMPTY);
    g_ncb.partition[partition].pebs[pnum].is_reserved = 1;
}

static void peb_scan(uint32_t partition)
{
    int32_t i, j, ret;
    peb_header_t peb_hdr;
    uint8_t peb_status;
    int32_t reclaim_idx, *reclaiming_peb;
    int32_t transfering_peb, transfered_peb;
    uint32_t avg_erase_count = 0;
    nvdm_partition_info_t *p_info = &(g_ncb.partition[partition]);
    nvdm_partition_cfg_t *p_cfg = p_info->p_cfg;

    reclaiming_peb = p_info->gc_buffer;
    memset(reclaiming_peb, 0, (p_cfg->peb_count * sizeof(int32_t)));

    for (i = 0; i < p_cfg->peb_count; i++) {
        p_info->pebs[i].erase_count = ERASE_COUNT_MAX;
    }

#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_095, 0);
#endif
    reclaim_idx = 0;
    transfering_peb = -1;
    transfered_peb = -1;
    for (i = 0; i < p_cfg->peb_count; i++) {
        peb_read_header(partition, i, &peb_hdr);
#if MARK_UNUSED_LOG_INFO_CALL
        nvdm_log_msgid_info(nvdm_096, 0);
#endif
        peb_header_print_info(i, &peb_hdr);
        peb_status = peb_hdr.status;
        switch (peb_status) {
            case PEB_STATUS_ACTIVING:
                /* need erase and erase count is valid */
                peb_erase(partition, i);
                peb_write_hdr_magic_number(partition, i);
                peb_write_hdr_erase_count(partition, i, ++(peb_hdr.erase_count));
                peb_write_hdr_version(partition, i);
                p_info->pebs[i].erase_count = peb_hdr.erase_count;
                peb_update_status(partition, i, PEB_STATUS_EMPTY);
#if MARK_UNUSED_POWEROFF_CALL
                nvdm_port_poweroff(17);
#endif
                p_info->pebs[i].is_reserved = 1;
                break;
            case PEB_STATUS_TRANSFERING:
            case PEB_STATUS_TRANSFERED:
            case PEB_STATUS_RECLAIMING:
                ret = peb_header_is_validate(&peb_hdr, 0);
                if (ret == false) {
                    nvdm_log_msgid_error(nvdm_097, 1, i);
                    /* need erase and erase count is invalid */
                    peb_reinit(partition, i);
                    continue;
                }
                p_info->pebs[i].erase_count = peb_hdr.erase_count;
                /* we just mark those pebs, and deal with them after init complete. */
                if (peb_status == PEB_STATUS_TRANSFERING) {
                    if (transfering_peb >= 0) {
                        nvdm_log_msgid_error(nvdm_098, 2, transfering_peb, i);
                        /* need erase and erase count is invalid */
                        /* There is no valid information in the block marked as transferring state and can be erased directly. */
                        peb_reclaim(partition, i);
                        continue;
                    }
                    transfering_peb = i;
                } else if (peb_status == PEB_STATUS_TRANSFERED) {
                    if (transfered_peb >= 0) {
                        nvdm_log_msgid_error(nvdm_099, 2, transfered_peb, i);
                        /* To simplify the processing, assume that all flash block
                         * marked as transferred blocks contain valid data.
                         * If the data in it is duplicate, or there is no valid data,
                         * data_item_scan function will process it.
                         */
                        peb_update_status(partition, i, PEB_STATUS_ACTIVED);
                        continue;
                    }
                    transfered_peb = i;
                } else {
                    /* there may be multiple reclaiming pebs exist */
                    reclaiming_peb[reclaim_idx++] = i;
                }
                break;
            case PEB_STATUS_ACTIVED:
                ret = peb_header_is_validate(&peb_hdr, 0);
                if (ret == false) {
                    nvdm_log_msgid_error(nvdm_100, 1, i);
                    /* need erase and erase count is invalid */
                    peb_reinit(partition, i);
                    continue;
                }
                p_info->pebs[i].erase_count = peb_hdr.erase_count;
                break;
            case PEB_STATUS_EMPTY:
                ret = peb_header_is_validate(&peb_hdr, 1);
                if (ret == false) {
                    nvdm_log_msgid_error(nvdm_101, 1, i);
                    /* need erase and erase count is invalid */
                    peb_reinit(partition, i);
                    continue;
                }
                p_info->pebs[i].erase_count = peb_hdr.erase_count;
                p_info->pebs[i].is_reserved = 1;
                break;
            case PEB_STATUS_VIRGIN:
            case PEB_STATUS_ERASING:
            default:
                /* need erase and erase count is invalid */
                peb_reinit(partition, i);
        }
#if MARK_UNUSED_LOG_INFO_CALL
        nvdm_log_msgid_info(nvdm_102, 0);
#endif
        peb_read_header(partition, i, &peb_hdr);
        peb_header_print_info(i, &peb_hdr);
    }
#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_103, 1, transfering_peb);
    nvdm_log_msgid_info(nvdm_104, 1, transfered_peb);
    for (i = 0; i < reclaim_idx; i++) {
        nvdm_log_msgid_info(nvdm_105, 2, i, reclaiming_peb[i]);
    }

    /* update erase count for unknown pebs */
    nvdm_log_msgid_info(nvdm_106, 0);
#endif
    avg_erase_count = 0;
    for (i = 0; i < p_cfg->peb_count; i++) {
        if (p_info->pebs[i].erase_count != ERASE_COUNT_MAX) {
            avg_erase_count += p_info->pebs[i].erase_count;
        }
    }
    avg_erase_count /= p_cfg->peb_count;
    for (i = 0; i < p_cfg->peb_count; i++) {
        if (p_info->pebs[i].erase_count == ERASE_COUNT_MAX) {
            /* peb header need to update here if erase count is invalid */
            peb_write_hdr_erase_count(partition, i, avg_erase_count);
            p_info->pebs[i].erase_count = avg_erase_count;
            peb_update_status(partition, i, PEB_STATUS_EMPTY);
#if MARK_UNUSED_POWEROFF_CALL
            nvdm_port_poweroff(18);
#endif
        }
    }

    /* scan all non-reserved pebs including reclaiming pebs and transfering peb */
#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_107, 0);
#endif
    for (i = 0; i < p_cfg->peb_count; i++) {
        /* skip transfering peb */
        if (i == transfering_peb) {
            continue;
        }
        /* skip reclaiming pebs and use reference from transfered peb if it exist */
        if (transfered_peb >= 0) {
            for (j = 0; j < reclaim_idx; j++) {
                if (i == reclaiming_peb[j]) {
                    break;
                }
            }
            if (j < reclaim_idx) {
                continue;
            }
        }

        if (p_info->pebs[i].is_reserved == 0) {
            /* transfered and active pebs can be scanned here */
            data_item_scan(partition, i);
        } else {
            p_info->pebs[i].free = (p_cfg->peb_size - PEB_HEADER_SIZE);
        }
    }

    /* deal with break from garbage collection */
    if ((reclaim_idx > 0) && (transfered_peb < 0)) {
        /* when power-off last time, data transfering is going on.
             * so we just restart garbage collection breaked by last power-off.
             */
#if MARK_UNUSED_LOG_INFO_CALL
        nvdm_log_msgid_info(nvdm_108, 0);
#endif
        if (transfering_peb >= 0) {
            peb_reclaim(partition, transfering_peb);
        }
        relocate_pebs(partition, reclaiming_peb, reclaim_idx);
    } else if ((transfered_peb >= 0) && (transfering_peb < 0)) {
        /* when power-off last time, data transfer is complete,
             * but source pebs maybe have not put back to wear-weaving yet
             */
#if MARK_UNUSED_LOG_INFO_CALL
        nvdm_log_msgid_info(nvdm_109, 0);
#endif
        for (i = 0; i < reclaim_idx; i++) {
            peb_reclaim(partition, reclaiming_peb[i]);
        }
        peb_update_status(partition, transfered_peb, PEB_STATUS_ACTIVED);
#if MARK_UNUSED_POWEROFF_CALL
        nvdm_port_poweroff(19);
#endif
    } else {
        if ((reclaim_idx == 0) && (transfering_peb == -1) && (transfered_peb == -1)) {
            /* No additional processing is required, because no garbage collection is found. */
            ;
        } else {
            /* In this case, there are definitely other non-NVDM Driver codes
             * that have modified the NVDM interval data !!!
             * After the above processing, there are only three possible situations
             * ( 0 means no block, 1 means has a block ):
             *     Reclaiming    Transferring    Transferred
             *     0             1               0
             *     0             1               1
             *     1             1               1
             * First change the block in the Transferred state to Activated, and perform data scanning on it.
             * Then, according to whether there is a Reclaiming block, decide how to deal with the transfer.
             *   If there is Reclaiming, continue to do garbage collection.
             *   If there is no Reclaiming, erase the Transferring.
             */

            nvdm_log_msgid_error(nvdm_110, 3, reclaim_idx, transfered_peb, transfering_peb);
            if (transfered_peb >= 0) {
                peb_update_status(partition, transfered_peb, PEB_STATUS_ACTIVED);
                transfered_peb = -1;
            }

            if (reclaim_idx > 0) {
                if (transfering_peb >= 0) {
                    peb_reclaim(partition, transfering_peb);
                }
                relocate_pebs(partition, reclaiming_peb, reclaim_idx);
            } else {
                peb_reclaim(partition, transfering_peb);
            }
        }
    }

    /* calculate total valid data size */
#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_111, 0);
#endif
    p_info->valid_data_size = 0;
    for (i = 0; i < p_cfg->peb_count; i++) {
        p_info->valid_data_size += (p_cfg->peb_size - PEB_HEADER_SIZE) - p_info->pebs[i].free - p_info->pebs[i].dirty;
    }
}


nvdm_status_t nvdm_init(void)
{
    uint32_t i, num, size;
    nvdm_partition_cfg_t *p_cfg;
    if (g_ncb.init_status == true) {
        return NVDM_STATUS_ERROR;
    }
#if MARK_UNUSED_POWEROFF_CALL
    nvdm_port_poweroff_time_set();
#endif

    p_cfg = nvdm_port_load_partition_info(&(g_ncb.partition_count));
#ifndef __EXT_BOOTLOADER__
    g_ncb.partition = (nvdm_partition_info_t *)nvdm_port_malloc(sizeof(nvdm_partition_info_t) * g_ncb.partition_count);
#else
    if (g_ncb.partition_count != NVDM_PARTITION_NUMBER) {
        nvdm_port_must_assert();
    }
    g_ncb.partition = (nvdm_partition_info_t *)(s_partition_info);
#endif

    nvdm_port_mutex_creat();
    nvdm_port_protect_mutex_create();

    for (num = 0; num < g_ncb.partition_count; num++) {
        g_ncb.partition[num].p_cfg = &(p_cfg[num]);
        if (g_ncb.partition[num].p_cfg->max_item_size > MAX_DATA_ITEM_SIZE) {
            nvdm_log_msgid_error(nvdm_042, 0);
            /* Must assert the configuration error, or else NVDM can not work. */
            nvdm_port_must_assert();
            return NVDM_STATUS_INVALID_PARAMETER;
        }

        size = g_ncb.partition[num].p_cfg->total_item_count * sizeof(data_item_header_on_ram_t);
        nvdm_log_msgid_warning(nvdm_125, 3, num, size, g_ncb.partition[num].p_cfg->total_item_count);
#ifndef __EXT_BOOTLOADER__
        g_ncb.partition[num].hdrs = (data_item_header_on_ram_t *)nvdm_port_malloc(size);
#else
        g_ncb.partition[num].hdrs = (data_item_header_on_ram_t *)(&s_item_hdrs[num][0]);
#endif

        if (g_ncb.partition[num].hdrs == NULL) {
            nvdm_log_msgid_error(nvdm_043, 0);
            /* Ignore the scenario where malloc fails and does not trigger assert,
             * but the program needs to ensure that NVDM cannot work.
             */
            return NVDM_STATUS_ERROR;
        }

        memset(g_ncb.partition[num].hdrs, 0, size);
        for (i = 0; i < g_ncb.partition[num].p_cfg->total_item_count; i++) {
            g_ncb.partition[num].hdrs[i].status = DATA_ITEM_STATUS_EMPTY;
        }
        g_ncb.partition[num].curr_item_count = 0;

        if (g_ncb.partition[num].p_cfg->peb_count < 2) {
            nvdm_log_msgid_error(nvdm_112, 0);
            /* Must assert here, or else NVDM can not work. */
            nvdm_port_must_assert();
            return NVDM_STATUS_ERROR;
        }

        /* The driver needs to allocate the required buffers during the initialization phase and
        * hold it all the time to avoid the chance of crash.
        */
        size = g_ncb.partition[num].p_cfg->peb_count * sizeof(int32_t);
#ifndef __EXT_BOOTLOADER__
        g_ncb.partition[num].gc_buffer = (int32_t *)nvdm_port_malloc(size);
#else
        g_ncb.partition[num].gc_buffer = (int32_t *)(&s_gc_buffer[num][0]);
#endif
        if (g_ncb.partition[num].gc_buffer == NULL) {
            nvdm_log_msgid_error(nvdm_077, 0);
            /* Must assert here, or else NVDM can not work. */
            nvdm_port_must_assert();
            return NVDM_STATUS_ERROR;
        }

        g_ncb.partition[num].avaliable_space = calculate_total_avail_space(num);
        g_ncb.partition[num].valid_data_size = 0;

        size = g_ncb.partition[num].p_cfg->peb_count * sizeof(peb_info_t);
        nvdm_log_msgid_warning(nvdm_126, 3, num, size, g_ncb.partition[num].p_cfg->peb_count);
#ifndef __EXT_BOOTLOADER__
        g_ncb.partition[num].pebs = (peb_info_t *)nvdm_port_malloc(size);
#else
        g_ncb.partition[num].pebs = (peb_info_t *)(&s_peb_info[num][0]);
#endif
        if (g_ncb.partition[num].pebs == NULL) {
            nvdm_log_msgid_error(nvdm_113, 0);
            /* Ignore the scenario where malloc fails and does not trigger assert,
            * but the program needs to ensure that NVDM cannot work.
            */
            return NVDM_STATUS_ERROR;
        }
        memset(g_ncb.partition[num].pebs, 0, size);

        peb_scan(num);

        if (find_free_peb(num, g_ncb.partition[num].p_cfg->max_item_size) < 0) {
            garbage_collection_peb(num, 3);
        }

        peb_print_info(num);
    }

#ifdef SYSTEM_DAEMON_TASK_ENABLE
    /* Doubly Linked Circular List */
    g_ncb.nbw_reqs.prev = &g_ncb.nbw_reqs;
    g_ncb.nbw_reqs.next = &g_ncb.nbw_reqs;

    g_ncb.has_nbw_req = 0;
    g_ncb.curr_nbw = NULL;
#endif

    g_ncb.init_status = 1;
    nvdm_log_msgid_warning(nvdm_114, 0);

    return NVDM_STATUS_OK;
}


nvdm_status_t nvdm_query_space_information(uint32_t partition, nvdm_space_info_t *info)
{
    nvdm_partition_info_t *p_info;
    nvdm_partition_cfg_t *p_cfg;
    int peb_idx;

    if (g_ncb.init_status == false) {
        return NVDM_STATUS_ERROR;
    }

    if ((info == NULL) || (partition >= g_ncb.partition_count)) {
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    nvdm_port_mutex_take();

    p_info = &(g_ncb.partition[partition]);
    p_cfg = p_info->p_cfg;

    info->free = 0;
    info->dirty = 0;
    info->valid = p_info->valid_data_size;
    info->capacity = p_info->avaliable_space;
    info->phy_size = p_cfg->peb_count * p_cfg->peb_size;
    for (peb_idx = 0; peb_idx < p_cfg->peb_count; peb_idx++) {
        info->free += p_info->pebs[peb_idx].free;
        info->dirty += p_info->pebs[peb_idx].dirty;
    }

    nvdm_log_msgid_warning(nvdm_127, 6, partition,
                           info->free, info->dirty, info->valid,
                           info->capacity, info->phy_size
                          );

    if ((info->free + info->dirty + info->valid + \
         (p_cfg->peb_count * PEB_HEADER_SIZE)) \
        != (info->phy_size)
       ) {
        /* Actively trigger assert to reinitialize the nvdm data. */
        nvdm_port_must_assert();
    }

    nvdm_port_mutex_give();

    return NVDM_STATUS_OK;
}


nvdm_status_t nvdm_trigger_garbage_collection(uint32_t partition, nvdm_gc_type_t type, uint32_t gc_bytes)
{
    nvdm_space_info_t space;
    uint32_t dirty_rcd[2], gc_round = 0;
    uint32_t count[2], duration_time;

    if (g_ncb.init_status == false) {
        return NVDM_STATUS_ERROR;
    }

#ifdef SYSTEM_DAEMON_TASK_ENABLE
    if ((type != NVDM_GC_IN_DAEMON) && (type != NVDM_GC_IN_CURR)) {
#else
    if ((type != NVDM_GC_IN_CURR)) {
#endif
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    if (partition >= g_ncb.partition_count) {
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    nvdm_port_mutex_take();

    nvdm_query_space_information(partition, &space);
    if (space.dirty < gc_bytes) {
        nvdm_port_mutex_give();
        return NVDM_STATUS_INVALID_PARAMETER;
    }

#ifdef SYSTEM_DAEMON_TASK_ENABLE
    uint32_t gc_info = gc_bytes & (uint32_t)0x00FFFFFF;
    gc_info |= (uint32_t)(partition << 24);
    if (type == NVDM_GC_IN_DAEMON) {
        bool send_req = nvdm_request_gc_in_daemon((const void *)gc_info);
        if (send_req == false) {
            nvdm_port_mutex_give();
            return NVDM_STATUS_ERROR;
        } else {
            nvdm_port_mutex_give();
            return NVDM_STATUS_OK;
        }
    }
#endif

    dirty_rcd[0] = space.dirty;
    dirty_rcd[1] = space.dirty;

    nvdm_log_msgid_warning(nvdm_128, 3, partition, type, gc_bytes);
    count[0] = nvdm_port_get_count();

    if (gc_bytes == 0) {
        gc_bytes = space.phy_size;
    }

    while ((dirty_rcd[0] - dirty_rcd[1]) < gc_bytes) {
        if (dirty_rcd[1] > dirty_rcd[0]) {
            nvdm_port_must_assert();
        }

        if ((gc_bytes == space.phy_size) &&
            (dirty_rcd[1] == 0)
           ) {
            break;
        }

        garbage_collection_peb(partition, gc_round);
        gc_round++;

        nvdm_query_space_information(partition, &space);
        dirty_rcd[1] = space.dirty;
    }

    count[1] = nvdm_port_get_count();
    duration_time = nvdm_port_get_duration_time(count[0], count[1]);
    peb_print_info(partition);
    nvdm_log_msgid_warning(nvdm_129, 3, partition, gc_round, duration_time);

    nvdm_port_mutex_give();
    return NVDM_STATUS_OK;
}


nvdm_status_t nvdm_obtain_item_count(uint32_t partition, uint32_t *item_count)
{
    nvdm_partition_info_t *p_info = &(g_ncb.partition[partition]);
    nvdm_partition_cfg_t *p_cfg = p_info->p_cfg;
    uint32_t idx, tmp_count;
    if (item_count == NULL) {
        return NVDM_STATUS_ERROR;
    }
    nvdm_port_mutex_take();
    nvdm_port_protect_mutex_take();
    tmp_count = 0;
    for (idx = 0; idx < p_cfg->total_item_count; idx++) {
        if (p_info->hdrs[idx].status == DATA_ITEM_STATUS_VALID) {
            ++tmp_count;
        }
    }
    *item_count = tmp_count;
    nvdm_port_protect_mutex_give();
    nvdm_port_mutex_give();
    return NVDM_STATUS_OK;
}

#endif


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

#include "nvdm.h"
#include "nvdm_port.h"
#include "nvdm_internal.h"
#include "nvdm_msgid_log.h"

#ifdef __EXT_BOOTLOADER__
#include "nvdm_config.h"
#endif

extern nvdm_ctrl_block_t g_ncb;


static void data_item_header_print_info(data_item_header_t *header)
{
#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_001, 0);
    nvdm_log_msgid_info(nvdm_002, 1, header->status);
    nvdm_log_msgid_info(nvdm_003, 1, header->pnum);
    nvdm_log_msgid_info(nvdm_004, 1, header->offset);
    nvdm_log_msgid_info(nvdm_005, 1, header->sequence_number);
    nvdm_log_msgid_info(nvdm_006, 1, header->group_name_size);
    nvdm_log_msgid_info(nvdm_007, 1, header->data_item_name_size);
    nvdm_log_msgid_info(nvdm_008, 1, header->value_size);
    nvdm_log_msgid_info(nvdm_009, 1, header->index);
    nvdm_log_msgid_info(nvdm_010, 1, header->type);
    nvdm_log_msgid_info(nvdm_011, 1, header->hash_name);
#else
    AVOID_UNUSED_BUILD_WARNING(header);
#endif
}


#ifndef __EXT_BOOTLOADER__
static int32_t find_empty_data_item(uint32_t partition)
{
    int32_t i;
    nvdm_partition_info_t *p_info = &(g_ncb.partition[partition]);
    nvdm_partition_cfg_t *p_cfg = p_info->p_cfg;

    for (i = 0; i < p_cfg->total_item_count; i++) {
        if (p_info->hdrs[i].status == DATA_ITEM_STATUS_EMPTY || p_info->hdrs[i].status == DATA_ITEM_STATUS_DELETE) {
            return i;
        }
    }
    return -1;
}
#endif


int update_data_item_header_on_ram(data_item_header_on_ram_t *ram_data_item, data_item_header_t *flash_data_item)
{
    if (flash_data_item == NULL || ram_data_item == NULL) {
        return -1;
    }
    ram_data_item->status    = flash_data_item->status;
    ram_data_item->hash_name = flash_data_item->hash_name; //for hash_uint8
    ram_data_item->offset    = flash_data_item->offset;
    ram_data_item->pnum      = flash_data_item->pnum;
    return 0;
}





static uint16_t calculate_checksum(uint16_t checksum, const uint8_t *buffer, int32_t size)
{
    uint8_t *byte_checksum;
    int32_t i;

    byte_checksum = (uint8_t *)&checksum;

    for (i = 0; i < size; i++) {
        if (i & 0x01) {
            *(byte_checksum + 1) += *(buffer + i);
        } else {
            *byte_checksum += *(buffer + i);
        }
    }
    return checksum;
}


static uint16_t calculate_data_item_checksum(data_item_header_t *header, uint32_t partition, int32_t pnum, int32_t position)
{
    int32_t i, offset, fragment;
    uint16_t checksum;
    uint8_t *working_buffer = g_ncb.working_buffer;

    checksum = 0;

    /* checksum for data item's header
     * skip frist byte because it's not calculated by checksum.
     */
    checksum = calculate_checksum(checksum, &header->pnum, DATA_ITEM_HEADER_SIZE - 1);

    /* add checksum for group name and data item name */
    offset = position;
    peb_read_data(partition, pnum, offset, working_buffer, header->group_name_size + header->data_item_name_size);
    checksum = calculate_checksum(checksum, working_buffer, header->group_name_size + header->data_item_name_size);

    /* add checksum for data item's value */
    offset += header->group_name_size + header->data_item_name_size;
    fragment = header->value_size / NVDM_BUFFER_SIZE;
    for (i = 0; i < fragment; i++) {
        memset(working_buffer, 0, NVDM_BUFFER_SIZE);
        peb_read_data(partition, pnum, offset, working_buffer, NVDM_BUFFER_SIZE);
        checksum = calculate_checksum(checksum, working_buffer, NVDM_BUFFER_SIZE);
        offset += NVDM_BUFFER_SIZE;
    }
    if (header->value_size % NVDM_BUFFER_SIZE) {
        memset(working_buffer, 0, NVDM_BUFFER_SIZE);
        peb_read_data(partition, pnum, offset, working_buffer, header->value_size % NVDM_BUFFER_SIZE);
        checksum = calculate_checksum(checksum, working_buffer, header->value_size % NVDM_BUFFER_SIZE);
    }

    return checksum;
}


static int32_t search_data_item_by_name(uint32_t partition, const char *group_name, const char *data_item_name, uint32_t *hasename)
{
    int32_t i, temp = 0;
    uint32_t hash, a = 63689, b = 378551;
    char str[64];
    nvdm_partition_info_t *p_info = &(g_ncb.partition[partition]);
    nvdm_partition_cfg_t *p_cfg = p_info->p_cfg;
    data_item_header_t *pdata_item = NULL;
    int cnt_hash_match = 0;
    uint8_t  u8tmp = 0;


    for (i = 0; * (group_name + i) != '\0'; i++) {
        str[i] = *(group_name + i);
    }
    temp += i;
    for (i = 0; * (data_item_name + i) != '\0'; i++) {
        str[i + temp] = *(data_item_name + i);
    }
    temp += i;
    str[temp] = '\0';

    hash = 0;
    for (i = 0; i < temp; i++) {
        hash = hash * a + str[i];
        a = a * b;
    }

    if (hasename != NULL) {
        *hasename = hash;
    }
#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_012, 1, hash);
#endif

#ifdef __EXT_BOOTLOADER__
    data_item_header_t data_item;
    pdata_item = &data_item;
#else
    pdata_item = nvdm_port_malloc(sizeof(data_item_header_t));
    if (pdata_item == NULL) {
        nvdm_log_msgid_warning("[nvdm] search_data_item_by_name:malloc fail", 0);
        return -2;
    }
#endif

    temp = -1;
    u8tmp = (hash & 0xFF); //for hash_uint8
    for (i = 0; i < p_cfg->total_item_count; i++) {
        /*skip all invalid data item*/
        if (p_info->hdrs[i].status != DATA_ITEM_STATUS_VALID) {
            continue;
        }
        /*hash name match?*/
        if (u8tmp == p_info->hdrs[i].hash_name) {
            cnt_hash_match++;
            peb_read_data(partition, p_info->hdrs[i].pnum, p_info->hdrs[i].offset, (uint8_t *)pdata_item, DATA_ITEM_HEADER_SIZE);
            peb_read_data(partition, p_info->hdrs[i].pnum, p_info->hdrs[i].offset + DATA_ITEM_HEADER_SIZE, (uint8_t *)str, pdata_item->group_name_size);
            if (strcmp(str, group_name)) {
                continue;
            }
            peb_read_data(partition, p_info->hdrs[i].pnum, p_info->hdrs[i].offset + DATA_ITEM_HEADER_SIZE + pdata_item->group_name_size, (uint8_t *)str, pdata_item->data_item_name_size);
            if (strcmp(str, data_item_name) == 0) {
                temp = i;
                break;
            }
        }
    }
#ifndef __EXT_BOOTLOADER__
    nvdm_port_free(pdata_item);
#endif
    return temp;
}


#ifdef SYSTEM_DAEMON_TASK_ENABLE
nvdm_status_t nvdm_read_data_from_nbw_reqs(
    const char *group_name, const char *data_item_name,
    uint8_t *buffer, uint32_t *size)
{
    uint32_t grp_name_size = 0, item_name_size = 0;
    nvdm_status_t op_res = NVDM_STATUS_ITEM_NOT_FOUND;
    async_nw_para_t *p_nw_para = (async_nw_para_t *)(g_ncb.nbw_reqs.prev);
    char *p_grp_name, *p_item_name;
    uint8_t *p_buf, *p_data;

    if (p_nw_para == (async_nw_para_t *)(&g_ncb.nbw_reqs)) {
        op_res = NVDM_STATUS_ITEM_NOT_FOUND;
    } else {
        /* Traverse all the data on the double circular linked list in reverse order,
         * and compare the group name and item name.
         */
        grp_name_size = strlen(group_name) + 1;
        item_name_size = strlen(data_item_name) + 1;
        while (p_nw_para != (async_nw_para_t *)(&g_ncb.nbw_reqs)) {
            p_buf = (uint8_t *)p_nw_para;
            p_buf += sizeof(async_nw_para_t);
            p_grp_name = (char *)p_buf;
            p_buf += p_nw_para->group_name_size;
            p_item_name = (char *)p_buf;
            p_buf += p_nw_para->data_item_name_size;
            p_data = p_buf;

            // nvdm_log_warning("traverse { %s, %s } item from RAM", 2, group_name, data_item_name);

            if ((grp_name_size == p_nw_para->group_name_size) &&
                (item_name_size == p_nw_para->data_item_name_size) &&
                (strncmp(group_name, p_grp_name, grp_name_size) == 0) &&
                (strncmp(data_item_name, p_item_name, item_name_size) == 0)
               ) {
                if (p_nw_para->data_item_size <= *size) {
                    memcpy(buffer, p_data, p_nw_para->data_item_size);
                    buffer += p_nw_para->data_item_size;
                    memset(buffer, 0, *size - p_nw_para->data_item_size);
                    *size = p_nw_para->data_item_size;
                    op_res = NVDM_STATUS_OK;
                    nvdm_log_warning("R { %s, %s } from RAM", group_name, data_item_name);
                    break;
                } else {
                    op_res = NVDM_STATUS_INVALID_PARAMETER;
                    *size = 0;
                }
            }

            p_nw_para = (async_nw_para_t *)(p_nw_para->item.prev);
        }
    }

    return op_res;
}
#endif


static nvdm_status_t write_nvdm_parameter_check(uint32_t partition,
                                                const char *group_name,
                                                const char *data_item_name,
                                                nvdm_data_item_type_t type,
                                                const uint8_t *buffer,
                                                uint32_t size)
{
    nvdm_partition_info_t *p_info = &(g_ncb.partition[partition]);
    nvdm_partition_cfg_t *p_cfg = p_info->p_cfg;

    if ((group_name == NULL) ||
        (data_item_name == NULL) ||
        (buffer == NULL)) {
        nvdm_log_msgid_error(nvdm_118, 1, 'W');
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    if ((type != NVDM_DATA_ITEM_TYPE_RAW_DATA) &&
        (type != NVDM_DATA_ITEM_TYPE_STRING)) {
        nvdm_log_msgid_error(nvdm_122, 1, type);
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    if (g_ncb.init_status == false) {
        nvdm_log_msgid_error(nvdm_121, 1, 'W');
        return NVDM_STATUS_ERROR;
    }

    if (partition >= g_ncb.partition_count) {
        nvdm_log_msgid_error(nvdm_119, 3, 'P', partition, g_ncb.partition_count);
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    if ((size > p_cfg->max_item_size) || (size == 0)) {
        nvdm_log_msgid_error(nvdm_119, 3, 'W', p_cfg->max_item_size, size);
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    if ((strlen(group_name) > p_cfg->max_group_name_size) ||
        (strlen(data_item_name) > p_cfg->max_item_name_size)) {
        nvdm_log_msgid_error(nvdm_120, 5, 'W',
                             p_cfg->max_group_name_size, strlen(group_name),
                             p_cfg->max_item_name_size, strlen(data_item_name)
                            );
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    return NVDM_STATUS_OK;
}

#define    INVALID_PARTITION    ((uint32_t)0xDEADBEEF)
uint32_t determine_which_partition(const char *group, const char *item,
                                   nvdm_data_item_type_t type,
                                   const uint8_t *buffer,
                                   uint32_t size, int32_t *idx, uint32_t check_parameter)
{
    nvdm_status_t op_ret;
    uint32_t ret = INVALID_PARTITION;
    uint32_t partition, max = g_ncb.partition_count;
    uint32_t hashname;
    int32_t item_idx = -1;

    for (partition = 0; partition < max; partition++) {
        if (check_parameter) {
            op_ret = write_nvdm_parameter_check(partition, group, item, type, buffer, size);
            if (op_ret != NVDM_STATUS_OK) {
                /* If the parameters do not match,
                 * then continue to look for the next partition.
                 */
                nvdm_log_msgid_warning(nvdm_123, 1, partition);
                continue;
            }
        }
        item_idx = search_data_item_by_name(partition, group, item, &hashname);
        if (item_idx == -2) {
            return 0xDEADBEEF;
        }
        if (item_idx != -1) {
            ret = partition;
            *idx = item_idx;
            break;
        }
    }
    *idx = item_idx;
    if (ret == INVALID_PARTITION) {
        /* The data is written to the first NVDM partition by default,
         * just like the software is installed to the C drive by default.
         */
        ret = 0;
    }
    return ret;
}



nvdm_status_t nvdm_read_data_item(const char *group_name,
                                  const char *data_item_name,
                                  uint8_t *buffer,
                                  uint32_t *size)
{
    int32_t index;
    uint32_t offset, partition;
    uint16_t checksum1, checksum2;
    nvdm_partition_info_t *p_info;
    nvdm_partition_cfg_t  *p_cfg;
    nvdm_data_item_type_t type = NVDM_DATA_ITEM_TYPE_RAW_DATA;
    data_item_header_t  data_item_head;
#ifdef SYSTEM_DAEMON_TASK_ENABLE
    nvdm_status_t read_from_ram = NVDM_STATUS_OK;
#endif

#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_013, 0);
#endif

    if ((group_name == NULL) ||
        (data_item_name == NULL) ||
        (buffer == NULL) ||
        (size == NULL) ||
        (*size == 0)
       ) {
        if (size != NULL) {
            *size = 0;
        }
        nvdm_log_msgid_error(nvdm_118, 1, 'R');
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    if (g_ncb.init_status == false) {
        nvdm_log_msgid_error(nvdm_121, 1, 'R');
        return NVDM_STATUS_ERROR;
    }

    nvdm_port_protect_mutex_take();

    partition = determine_which_partition(group_name, data_item_name, type, buffer, *size, &index, 0);
    if (INVALID_PARTITION == partition) {
        nvdm_port_protect_mutex_give();
        return NVDM_STATUS_ERROR;
    }
    p_info = &(g_ncb.partition[partition]);
    p_cfg = p_info->p_cfg;

    if ((strlen(group_name) > p_cfg->max_group_name_size) ||
        (strlen(data_item_name) > p_cfg->max_item_name_size)
       ) {
        nvdm_log_msgid_error(nvdm_120, 5, 'R',
                             p_cfg->max_group_name_size, strlen(group_name),
                             p_cfg->max_item_name_size, strlen(data_item_name)
                            );
        nvdm_port_protect_mutex_give();
        return NVDM_STATUS_INVALID_PARAMETER;
    }

#ifdef SYSTEM_DAEMON_TASK_ENABLE
    if (nvdm_port_query_task_handler() == false) {
        /* For the consistency of API behavior, if the current task does not
         * start to query NVDM data, then first try to read the data from RAM,
         * and then read from Flash.
         * Otherwise, only read data from Flash.
         */
        read_from_ram = nvdm_read_data_from_nbw_reqs(group_name, data_item_name, buffer, size);
        if (read_from_ram != NVDM_STATUS_ITEM_NOT_FOUND) {
            nvdm_port_protect_mutex_give();
            return read_from_ram;
        }
    }
#endif

    if (index < 0) {
        *size = 0;
        nvdm_port_protect_mutex_give();
        return NVDM_STATUS_ITEM_NOT_FOUND;
    }
    peb_read_data(partition, p_info->hdrs[index].pnum, p_info->hdrs[index].offset, (uint8_t *)&data_item_head, DATA_ITEM_HEADER_SIZE);

    /* check whether buffer size is enough */
    if (*size < data_item_head.value_size) {
        *size = 0;
        nvdm_log_msgid_error(nvdm_119, 3, 'R',
                             data_item_head.value_size, *size
                            );
        nvdm_port_protect_mutex_give();
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    /* verify checksum of date item */
    checksum1 = calculate_data_item_checksum(&data_item_head,
                                             partition,
                                             data_item_head.pnum,
                                             data_item_head.offset + DATA_ITEM_HEADER_SIZE);
    offset = data_item_head.offset + DATA_ITEM_HEADER_SIZE + data_item_head.group_name_size + data_item_head.data_item_name_size + data_item_head.value_size;
    peb_read_data(partition, data_item_head.pnum, offset, (uint8_t *)&checksum2, DATA_ITEM_CHECKSUM_SIZE);
    if (checksum1 != checksum2) {
        *size = 0;
        nvdm_port_protect_mutex_give();
        return NVDM_STATUS_INCORRECT_CHECKSUM;
    }

    /* checksum is ok, so read it to user buffer */
    offset = data_item_head.offset + DATA_ITEM_HEADER_SIZE + data_item_head.group_name_size + data_item_head.data_item_name_size;
    peb_read_data(partition, data_item_head.pnum, offset, buffer, data_item_head.value_size);
    buffer += data_item_head.value_size;
    memset(buffer, 0, *size - data_item_head.value_size);

    *size = data_item_head.value_size;
    if (*size == 0) {
        nvdm_port_must_assert();
    }

    nvdm_port_protect_mutex_give();

    return NVDM_STATUS_OK;
}


#ifndef __EXT_BOOTLOADER__


static int32_t cmp_item_data_in_buffer_and_flash(uint32_t partition, const uint8_t *buffer, uint32_t buff_sz, const data_item_header_t *header)
{
    /* Compare whether the data on the RAM is the same as the data on the flash. */
    int32_t i, offset, fragment, cmp_res = 0;
    uint8_t *working_buffer = g_ncb.working_buffer;
    uint32_t cmp_len;

    /* check parameter is valid or not */
    if (buffer == NULL || header == NULL || buff_sz == 0) {
        return 1;
    }
    /* check the data size is equal or not*/
    if (header->value_size != buff_sz) {
        if (header->value_size > buff_sz) {
            return 1;
        } else {
            return -1;
        }
    }
    /* Calculate the offset where the user data is located. */
    offset = header->offset + sizeof(data_item_header_t) + header->group_name_size + header->data_item_name_size;

    fragment = header->value_size / NVDM_BUFFER_SIZE;
    for (i = 0; i < fragment; i++) {
        peb_read_data(partition, header->pnum, offset, working_buffer, NVDM_BUFFER_SIZE);
        offset += NVDM_BUFFER_SIZE;
        cmp_len = NVDM_BUFFER_SIZE;
        cmp_res = memcmp(working_buffer, buffer, cmp_len);
        if (cmp_res != 0) {
            return cmp_res;
        }
        buffer += cmp_len;
    }
    if (header->value_size % NVDM_BUFFER_SIZE) {
        peb_read_data(partition, header->pnum, offset, working_buffer, header->value_size % NVDM_BUFFER_SIZE);
        cmp_len = header->value_size % NVDM_BUFFER_SIZE;
        cmp_res = memcmp(working_buffer, buffer, cmp_len);
        if (cmp_res != 0) {
            return cmp_res;
        }
        buffer += cmp_len;
    }

    return cmp_res;
}


nvdm_status_t nvdm_create_data_item(uint32_t partition,
                                    const char *group_name,
                                    const char *data_item_name,
                                    nvdm_data_item_type_t type,
                                    const uint8_t *buffer,
                                    uint32_t size)
{
    int32_t append, peb_status_update;
    int32_t added_size, alloc_size, group_name_size, data_item_name_size;
    int32_t index, pnum, offset, temp;
    uint32_t hashname;
    uint32_t checksum_twice_match = 1;
    uint16_t checksum, checksum_after_write, checksum_on_flash;
    uint8_t *working_buffer;
    data_item_header_t *p_data_item_header;
    data_item_header_t *p_old_item_header;
    data_item_header_t data_item_header, hdr_for_checksum;
    data_item_status_t status;
    int32_t ret, cmp_res = 0;
    nvdm_status_t nvdm_status;
    nvdm_partition_info_t *p_info;
    nvdm_partition_cfg_t *p_cfg;

#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_015, 0);
#endif

    nvdm_port_mutex_take();

    p_info = &(g_ncb.partition[partition]);
    p_cfg = p_info->p_cfg;

    nvdm_status = write_nvdm_parameter_check(partition, group_name, data_item_name, type, buffer, size);
    if (nvdm_status != NVDM_STATUS_OK) {
        nvdm_port_mutex_give();
        return nvdm_status;
    }

#ifdef SYSTEM_DAEMON_TASK_ENABLE
    /* Since at the application layer, both blocking and non-blocking writes to
     * the same data may be called, the Driver needs to be aware of this situation.
     * In this case, NVDM driver will make the blocked write data take effect,
     * and delete requests for non-blocking writes at the same time.
     */
    nvdm_cancel_non_blocking_write(group_name, data_item_name);
#endif

    group_name_size = strlen(group_name) + 1;
    data_item_name_size = strlen(data_item_name) + 1;

    index = search_data_item_by_name(partition, group_name, data_item_name, &hashname);
#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_016, 1, index);
#endif
    if (index == -2) {
        nvdm_port_mutex_give();
        return NVDM_STATUS_ERROR;
    } else if (index < 0) {
        append = 1;
        /* find a empty position to fill in */
        index = find_empty_data_item(partition);
    } else {
        append = 0;
        peb_read_data(partition, p_info->hdrs[index].pnum, p_info->hdrs[index].offset, (uint8_t *)&data_item_header, DATA_ITEM_HEADER_SIZE);
        if (data_item_header.value_size == size) {
            /* When writing an existing data item, it is necessary to compare
             * whether the user data matches the existing data on the flash.
             * If the data matches, there is no need to write flash, just return OK.
             */
            nvdm_port_protect_mutex_take();
            cmp_res = cmp_item_data_in_buffer_and_flash(partition, buffer, size, &data_item_header);
            if (cmp_res == 0) {
                nvdm_port_protect_mutex_give();
                nvdm_port_mutex_give();
                return NVDM_STATUS_OK;
            }
            nvdm_port_protect_mutex_give();
        }
    }

    /* check whether we have enough free space for append */
    if (append) {
        added_size = size + group_name_size + data_item_name_size + DATA_ITEM_HEADER_SIZE + DATA_ITEM_CHECKSUM_SIZE;
    } else {
        added_size = (int32_t)size - (int32_t)data_item_header.value_size;
    }
    ret = space_is_enough(partition, added_size);
    if (ret == false) {
#if MARK_UNUSED_LOG_INFO_CALL
        nvdm_log_msgid_info(nvdm_017, 0);
#endif
        nvdm_port_mutex_give();
        return NVDM_STATUS_INSUFFICIENT_SPACE;
    }

    /* find a peb with require free space to place data item */
    alloc_size = size + group_name_size + data_item_name_size + DATA_ITEM_HEADER_SIZE + DATA_ITEM_CHECKSUM_SIZE;
    pnum = space_allocation(partition, alloc_size, added_size, &offset, group_name, data_item_name);
    temp = offset;

    /* write item header information to temp header first */
    p_data_item_header = &data_item_header;
    if (append) {
        if (p_info->curr_item_count >= p_cfg->total_item_count) {
            nvdm_log_msgid_warning(nvdm_018, 0);
            nvdm_port_mutex_give();
            return NVDM_STATUS_ERROR;
        }
        /* new add a NVDM entry */
#if MARK_UNUSED_LOG_INFO_CALL
        nvdm_log_msgid_info(nvdm_019, 0);
#endif
        p_data_item_header->type = type;
        p_data_item_header->hash_name = hashname;
        p_data_item_header->value_size = size;
        p_data_item_header->reserved = 0xFF00 | (index >> 8);
        p_data_item_header->index = index & 0xFF;
        p_info->curr_item_count++;

        /* first write */
        p_data_item_header->sequence_number = 1;
        p_data_item_header->pnum = pnum;
        p_data_item_header->group_name_size = group_name_size;
        p_data_item_header->data_item_name_size = data_item_name_size;
        p_data_item_header->offset = offset;
    } else {
        /* modify an old NVDM entry */
#if MARK_UNUSED_LOG_INFO_CALL
        nvdm_log_msgid_info(nvdm_020, 0);
#endif
        /* Use sequence_number to record the number of writes of data items. */
        p_data_item_header->sequence_number = (p_data_item_header->sequence_number) + 1;
        p_data_item_header->pnum = pnum;
        p_data_item_header->offset = offset;
        p_data_item_header->value_size = size;
        p_data_item_header->type = type;
        /* old version use only one byte to decide the index */
        if (p_data_item_header->reserved == 0x00FF) {
            p_data_item_header->reserved = 0xFF00;
        }
    }
    p_data_item_header->status = DATA_ITEM_STATUS_WRITING;

    nvdm_log_warning("task: %s write { %u, %s, %s } with %d bytes",
                     nvdm_port_get_curr_task_name(),
                     partition, group_name, data_item_name, size
                    );

    /* calculate checksum for new data item copy */
    checksum = 0;
    /* DATA_ITEM_HEADER_SIZE-1 must be power of 2 */
    checksum = calculate_checksum(checksum, &p_data_item_header->pnum, DATA_ITEM_HEADER_SIZE - 1);

    /* can't use g_ncb.working_buffer because of the race condition with nvdm_read_data_item */
    working_buffer = nvdm_port_malloc((group_name_size + data_item_name_size));
    if (working_buffer == NULL) {
        nvdm_port_mutex_give();
        return NVDM_STATUS_ERROR;
    }

    memcpy(working_buffer, group_name, group_name_size);
    working_buffer += group_name_size;
    memcpy(working_buffer, data_item_name, data_item_name_size);
    working_buffer -= group_name_size;
    checksum = calculate_checksum(checksum, working_buffer, (group_name_size + data_item_name_size));
    checksum = calculate_checksum(checksum, buffer, size);

    /* this peb is frist written, so status of PEB need to be modified */
    if (peb_activing(partition, pnum)) {
        peb_status_update = 1;
    } else {
        peb_status_update = 0;
    }

    peb_sub_free(partition, pnum, alloc_size);

    /* set status of data item to writing */
    status = DATA_ITEM_STATUS_WRITING;
    peb_write_data(partition, pnum, offset, (uint8_t *)&status, 1);
    p_data_item_header->status = DATA_ITEM_STATUS_WRITING;
#if MARK_UNUSED_POWEROFF_CALL
    nvdm_port_poweroff(1);
#endif
    /* write header of data item (not including status) */
    offset += 1;
    peb_write_data(partition, pnum, offset, &p_data_item_header->pnum, DATA_ITEM_HEADER_SIZE - 1);
    /* write group name and data item name */
    offset += DATA_ITEM_HEADER_SIZE - 1;
    peb_write_data(partition, pnum, offset, working_buffer, group_name_size + data_item_name_size);

    /* this variable will not be used again */
    nvdm_port_free(working_buffer);

    /* write value of data item */
    offset += group_name_size + data_item_name_size;
    peb_write_data(partition, pnum, offset, (uint8_t *)buffer, size);
    /* write checksum of data item */
    offset += size;
    peb_write_data(partition, pnum, offset, (uint8_t *)&checksum, DATA_ITEM_CHECKSUM_SIZE);

    nvdm_port_protect_mutex_take();
    peb_read_data(partition, pnum, offset, (uint8_t *)&checksum_on_flash, DATA_ITEM_CHECKSUM_SIZE);
    /* It shares the global buffer g_ncb.working_buffer with nvdm read operation
     * and needs to be mutually exclusive with read behavior.
     * For compatibility reasons, the global physical address is not used directly to access the flash.
     */

    /* The data written to the non-volatile device must be read back first,
     * otherwise checking the checksum twice is meaningless !!!
     */
    peb_read_data(partition, pnum, p_data_item_header->offset, (uint8_t *)&hdr_for_checksum, DATA_ITEM_HEADER_SIZE);
    checksum_after_write = calculate_data_item_checksum(
                               &hdr_for_checksum,
                               partition,
                               p_data_item_header->pnum,
                               p_data_item_header->offset + DATA_ITEM_HEADER_SIZE);

    if (checksum != checksum_after_write || checksum_on_flash != checksum) {
        checksum_twice_match = 0;
    }
    nvdm_port_protect_mutex_give();

    /* set status of data item to valid */
    offset -= p_data_item_header->value_size + DATA_ITEM_HEADER_SIZE + group_name_size + data_item_name_size;

    /* If before and after writing, the checksum calculated using user input is inconsistent,
     * indicating that the buffer is not thread-safe !!!
     */
    if (checksum_twice_match == 0) {
        status = DATA_ITEM_STATUS_DELETE;
    } else {
        status = DATA_ITEM_STATUS_VALID;
    }
    peb_write_data(partition, pnum, offset, (uint8_t *)&status, 1);
    if (checksum_twice_match == 0) {
        p_data_item_header->status = DATA_ITEM_STATUS_DELETE;
    } else {
        p_data_item_header->status = DATA_ITEM_STATUS_VALID;
    }

#if MARK_UNUSED_POWEROFF_CALL
    nvdm_port_poweroff(2);
#endif

    if (peb_status_update) {
        /* Now we have at least one data item in PEB,
         * so update it's status to PEB_STATUS_ACTIVED.
         */
        peb_update_status(partition, pnum, PEB_STATUS_ACTIVED);
#if MARK_UNUSED_POWEROFF_CALL
        nvdm_port_poweroff(3);
#endif
    }

    if ((!append) && (checksum_twice_match == 1)) {
        /* read old data item header for calc dirty size */
        peb_read_data(partition, p_info->hdrs[index].pnum, p_info->hdrs[index].offset, (uint8_t *)&data_item_header, DATA_ITEM_HEADER_SIZE);
        p_old_item_header = &data_item_header;

        /* Because we have write new copy successfully,
         * so we can invalidate old copy now!
         */
        status = DATA_ITEM_STATUS_DELETE;
        peb_write_data(partition, p_old_item_header->pnum, p_old_item_header->offset, (uint8_t *)&status, 1);
#if MARK_UNUSED_POWEROFF_CALL
        nvdm_port_poweroff(4);
#endif
        /* mark drity for old copy */
        size = DATA_ITEM_HEADER_SIZE +
               p_old_item_header->group_name_size +
               p_old_item_header->data_item_name_size +
               p_old_item_header->value_size +
               DATA_ITEM_CHECKSUM_SIZE;
        peb_add_drity(partition, p_old_item_header->pnum, size);
    }

    /* after user data had write to flash, update the header information to global variable */
    nvdm_port_protect_mutex_take();
    if (checksum_twice_match == 1) {
        /*update the header info on ram*/
        p_info->hdrs[index].status = DATA_ITEM_STATUS_VALID;
        p_info->hdrs[index].pnum = pnum;
        p_info->hdrs[index].offset = temp;
        p_info->hdrs[index].hash_name = hashname;
        /* Modify g_valid_data_size after the data is actually written. */
        space_sub_valid(partition, -added_size);
    } else {
        /* Mark the newly written data as delete, and actively trigger a restart to prevent the nvdm driver from writing data later. */
        peb_write_data(partition, p_data_item_header->pnum, p_data_item_header->offset, (uint8_t *) & (p_data_item_header->status), 1);
        nvdm_log_error("task %s write { %u, %s, %s }, calc checksum twice: 0x%08X and 0x%08X, trigger assert",
                       nvdm_port_get_curr_task_name(),
                       partition,
                       group_name, data_item_name,
                       checksum, checksum_after_write
                      );
        /* Then the driver needs to directly mark the position of the next data item header as 0x00,
         * because 0x00 is not a valid data item header state, so the nvdm driver will not
         * write data to the back after the device restarts.
         */
        offset += DATA_ITEM_HEADER_SIZE + hdr_for_checksum.group_name_size +
                  hdr_for_checksum.data_item_name_size + hdr_for_checksum.value_size + 2;
        if (offset < p_cfg->peb_size) {
            p_data_item_header->status = 0x00;
            peb_write_data(partition, pnum, offset, (uint8_t *) & (p_data_item_header->status), 1);
        }
        nvdm_port_must_assert();
    }
    nvdm_port_protect_mutex_give();

    nvdm_port_mutex_give();

    if (checksum_twice_match == 1) {
        return NVDM_STATUS_OK;
    } else {
        return NVDM_STATUS_INCORRECT_CHECKSUM;
    }
}


nvdm_status_t nvdm_write_data_item(const char *group_name,
                                   const char *data_item_name,
                                   nvdm_data_item_type_t type,
                                   const uint8_t *buffer,
                                   uint32_t size)
{
    uint32_t partition;
    int32_t index;
    nvdm_status_t op_ret;
    nvdm_port_mutex_take();

    nvdm_port_protect_mutex_take();
    partition = determine_which_partition(group_name, data_item_name, type, buffer, size, &index, 1);
    if (INVALID_PARTITION == partition) {
        nvdm_port_protect_mutex_give();
        nvdm_port_mutex_give();
        return NVDM_STATUS_ERROR;
    }
    nvdm_port_protect_mutex_give();

    op_ret = nvdm_create_data_item(partition, group_name, data_item_name, type, buffer, size);
    nvdm_port_mutex_give();
    return op_ret;
}


#ifdef SYSTEM_DAEMON_TASK_ENABLE


bool nbw_req_is_empty(void)
{
    async_nw_para_t *nbw_para;
    nvdm_port_protect_mutex_take();
    nbw_para = (async_nw_para_t *)(g_ncb.nbw_reqs.next);
    if (nbw_para != (async_nw_para_t *)(&g_ncb.nbw_reqs)) {
        nvdm_port_protect_mutex_give();
        return false;
    }
    nvdm_port_protect_mutex_give();
    return true;
}


static bool same_item_with_last_nbw_req(const char *group_name, const char *data_item_name, uint32_t new_item_size)
{
    async_nw_para_t *last_elem;
    last_elem = (async_nw_para_t *)(g_ncb.nbw_reqs.prev);
    bool ret = false;
    uint32_t gn_size, din_size;
    const char *p_le_gn, *p_le_din;       /* pointers of last element group name and data item name */

    gn_size = strlen(group_name) + 1;     /* group name size */
    din_size = strlen(data_item_name) + 1; /* data item name size */

    p_le_gn = (const char *)((uint8_t *)last_elem + sizeof(async_nw_para_t));
    p_le_din = (const char *)((uint8_t *)p_le_gn + last_elem->group_name_size);

    if ((gn_size == last_elem->group_name_size) &&
        (din_size == last_elem->data_item_name_size) &&
        (new_item_size == last_elem->data_item_size) &&
        (strcmp(p_le_gn, group_name) == 0) &&
        (strcmp(p_le_din, data_item_name) == 0)
       ) {
        /* Only when the group name, data item name,
         * and actual data length are all the same,
         * it is considered the same data.
         */
        ret = true;
    }

    return ret;
}


static async_nw_para_t *construct_nbw_req(const char *group_name,
                                          const char *data_item_name,
                                          nvdm_data_item_type_t type,
                                          const uint8_t *buffer,
                                          uint32_t size,
                                          const nvdm_user_callback_t callback,
                                          const void *user_data)
{
    uint32_t group_name_size, data_item_name_size, alloc_size;
    uint8_t *p_buf;
    async_nw_para_t *nbw_para = NULL;

    group_name_size = strlen(group_name) + 1;
    data_item_name_size = strlen(data_item_name) + 1;
    alloc_size = sizeof(async_nw_para_t) + group_name_size + data_item_name_size + size;
    nbw_para = (async_nw_para_t *)nvdm_port_malloc(alloc_size);
    if (nbw_para == NULL) {
        nvdm_log_msgid_error(nvdm_023, 0);
        /* Ignore Malloc fail and just return Error code. */
        return nbw_para;
    }

    nbw_para->item.prev = NULL;
    nbw_para->item.next = NULL;
    nbw_para->callback = callback;
    nbw_para->user_data = (void *)user_data;
    nbw_para->group_name_size = group_name_size;
    nbw_para->data_item_name_size = data_item_name_size;
    nbw_para->data_item_size = size;
    nbw_para->type = type;
    p_buf = (uint8_t *)nbw_para + sizeof(async_nw_para_t);
    memcpy(p_buf, group_name, group_name_size);
    p_buf += group_name_size;
    memcpy(p_buf, data_item_name, data_item_name_size);
    p_buf += data_item_name_size;
    memcpy(p_buf, buffer, size);
    return nbw_para;
}


nvdm_status_t nvdm_write_data_item_non_blocking(const char *group_name,
                                                const char *data_item_name,
                                                nvdm_data_item_type_t type,
                                                const uint8_t *buffer,
                                                uint32_t size,
                                                const nvdm_user_callback_t callback,
                                                const void *user_data)
{
    uint8_t *p_buf;
    async_nw_para_t *nbw_para;

#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_022, 0);
#endif

    /* The operation here is simply to check the input parameters.
     * As for whether it meets the configuration of each partition,
     * it is checked by the underlying blocking write API. */
    if ((callback == NULL) ||
        (group_name == NULL) ||
        (data_item_name == NULL) ||
        (buffer == NULL) ||
        (size == 0)
       ) {
        if (size == 0) {
            nvdm_log_msgid_error(nvdm_119, 3, 'N', size, size);
        } else {
            nvdm_log_msgid_error(nvdm_118, 1, 'N');
        }
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    nvdm_port_protect_mutex_take();
    if (nbw_req_is_empty() == true) {
        /* If the double-linked list is empty, construct nbw_req directly and add it to the linked list. */
        nbw_para = construct_nbw_req(group_name, data_item_name, type, buffer, size, callback, user_data);
        if (nbw_para == NULL) {
            nvdm_port_protect_mutex_give();
            nvdm_log_msgid_error(nvdm_118, 1, 'C');
            return NVDM_STATUS_ERROR;
        }
        if (nvdm_port_send_queue() == false) {
            nvdm_port_free(nbw_para);
            nvdm_log_msgid_warning(nvdm_024, 0);
            nvdm_port_protect_mutex_give();
            return NVDM_STATUS_ERROR;
        } else {
            /* Make sure that there is no non-blocking write requirement and
             * send the request to the daemon task before appending the data on the NVDM linked list.
             */
            /* Insert data from the end of the doubly circular linked list. */
            nbw_para->item.next = (&g_ncb.nbw_reqs);
            nbw_para->item.prev = (g_ncb.nbw_reqs.prev);
            g_ncb.nbw_reqs.prev->next = (nvdm_dcll_t *)nbw_para;
            g_ncb.nbw_reqs.prev = (nvdm_dcll_t *)nbw_para;
        }
    } else {
        /* If the linked list is not empty, it means that a non-blocking write event
         * has been sent to the daemon task before, so there is no need to send it again here.
         */

        if (((async_nw_para_t *)(g_ncb.nbw_reqs.prev) != g_ncb.curr_nbw) &&
            (same_item_with_last_nbw_req(group_name, data_item_name, size) == true)
           ) {
            /* If the last element has the same group name and item as the element user want to write again,
             * then the two pieces of data can be merged directly on RAM.
             */
            nbw_para = (async_nw_para_t *)(g_ncb.nbw_reqs.prev);
            nbw_para->callback(NVDM_STATUS_ITEM_MERGED_BY_NBW, nbw_para->user_data);
            p_buf = (uint8_t *)nbw_para + sizeof(async_nw_para_t) + \
                    nbw_para->group_name_size + nbw_para->data_item_name_size;
            memcpy(p_buf, buffer, size);
            nbw_para->callback = (nvdm_user_callback_t)callback;
            nbw_para->user_data = (void *)user_data;
        } else {
            nbw_para = construct_nbw_req(group_name, data_item_name, type, buffer, size, callback, user_data);
            /* Insert data from the end of the doubly circular linked list. */
            nbw_para->item.next = (&g_ncb.nbw_reqs);
            nbw_para->item.prev = (g_ncb.nbw_reqs.prev);
            g_ncb.nbw_reqs.prev->next = (nvdm_dcll_t *)nbw_para;
            g_ncb.nbw_reqs.prev = (nvdm_dcll_t *)nbw_para;
        }
    }
    nvdm_port_protect_mutex_give();

    return NVDM_STATUS_OK;
}


void system_daemon_nvdm_msg_handler(void)
{
    async_nw_para_t *nbw_para;
    char *group_name;
    char *data_item_name;
    uint8_t *p_data;
    nvdm_data_item_type_t type;
    uint8_t *p_buf;
    nvdm_status_t status;
    bool no_nbw_req = false;

    nvdm_port_protect_mutex_take();
    no_nbw_req = nbw_req_is_empty();
    if (no_nbw_req == false) {
        /* Mark the API that calls blocking writes as non-blocking writes,
         * otherwise the data may be deleted.
         */
        g_ncb.has_nbw_req = true;
        nbw_para = (async_nw_para_t *)(g_ncb.nbw_reqs.next);
        g_ncb.curr_nbw = nbw_para;
    }
    nvdm_port_protect_mutex_give();

    if (no_nbw_req == false) {
        p_buf = (uint8_t *)nbw_para;
        p_buf += sizeof(async_nw_para_t);
        group_name = (char *)p_buf;
        p_buf += nbw_para->group_name_size;
        data_item_name = (char *)p_buf;
        p_buf += nbw_para->data_item_name_size;
        p_data = p_buf;
        type = nbw_para->type;

        /* Because the mutex used for reading and writing is separated,
         * if driver use protect mutex to protect nvdm_write_data_item, it may cause !!! DEADLOCK !!!.
         * At the same time, protect mutex is not used here,
         * so a new Non-Blocking Write request is allowed during data writing.
         */
        status = nvdm_write_data_item(group_name, data_item_name, type, p_data, nbw_para->data_item_size);

        nvdm_port_protect_mutex_take();
        /* !!! NOTE !!!
         * The relationship of nbw_para on the double-linked list may change,
         * but it does not affect the removal of this item from the DCLL.
         */

        nbw_para->callback(status, nbw_para->user_data);
        /* Remove the specified data item from the list. */
        nbw_para->item.prev->next = nbw_para->item.next;
        nbw_para->item.next->prev = nbw_para->item.prev;
        nvdm_port_free((void *)nbw_para);

        if (nbw_req_is_empty() == false) {
            /* If there is still data on RAM that has not been written to Flash,
             * then the NVDM non-blocking write event is sent back to the daemon task.
             */
            nvdm_port_send_queue();
        }
        g_ncb.has_nbw_req = false;
        g_ncb.curr_nbw = NULL;
        nvdm_port_protect_mutex_give();
    }
}


void system_daemon_handle_nvdm_gc(const void *pdata)
{
    uint32_t gc_byte = (uint32_t)pdata & (uint32_t)0x00FFFFFF;
    uint32_t partition = ((uint32_t)pdata & (uint32_t)0xFF000000) >> 24;
    nvdm_trigger_garbage_collection(partition, NVDM_GC_IN_CURR, gc_byte);
}

nvdm_status_t nvdm_cancel_non_blocking_write(const char *group_name, const char *item_name)
{
    nvdm_status_t op_res = NVDM_STATUS_ITEM_NOT_FOUND;
    async_nw_para_t *nbw_para, *backup_next;
    uint8_t *p_buf;
    char *p_nbw_grp_name, *p_nbw_item_name;
    uint8_t cur_grp_name_size, cur_item_name_size;
    uint32_t canceled_item_count = 0;

    if ((group_name == NULL) || (item_name == NULL)) {
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    cur_grp_name_size = strlen(group_name) + 1;
    cur_item_name_size = strlen(item_name) + 1;

    if ((cur_grp_name_size == 1) || (cur_item_name_size == 1)) {
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    nvdm_port_protect_mutex_take();

    nbw_para = ((async_nw_para_t *)(g_ncb.nbw_reqs.next));
    while (nbw_para != ((async_nw_para_t *)(&g_ncb.nbw_reqs))) {
        if (nbw_para == NULL) {
            nvdm_port_must_assert();
        }
        p_buf = (uint8_t *)nbw_para;
        p_buf += sizeof(async_nw_para_t);
        p_nbw_grp_name = (char *)p_buf;
        p_buf += nbw_para->group_name_size;
        p_nbw_item_name = (char *)p_buf;
        p_buf += nbw_para->data_item_name_size;

#if MARK_UNUSED_LOG_INFO_CALL
        nvdm_log_info("nvdm_cancel_nbw { %s, %s } travers { %s, %s }",
                      group_name, item_name, p_nbw_grp_name, p_nbw_item_name);
#endif

        backup_next = (async_nw_para_t *)(nbw_para->item.next);

        if ((nbw_para->group_name_size == cur_grp_name_size) &&
            (nbw_para->data_item_name_size == cur_item_name_size)
           ) {
            if ((strcmp(group_name, p_nbw_grp_name) == 0) &&
                (strcmp(item_name, p_nbw_item_name) == 0)
               ) {
                if ((g_ncb.has_nbw_req == true) &&
                    (nbw_para == ((async_nw_para_t *)(g_ncb.nbw_reqs.next)))
                   ) {
                    /* NVDM driver will call this API when using blocking writing method,
                     * but in this case, it cannot be cancelled,
                     * otherwise the non-blocking write will be lost.
                     */
                    nvdm_log_warning("NBW of {%s, %s} cannot be cancelled because it is being written.",
                                     group_name, item_name);
                    op_res = NVDM_STATUS_ERROR;
                    break;
                } else {
                    /* If the driver does not cancel by itself, or
                     * the data being written is not the first in the DCLL,
                     * then the non-blocking write data can be cancelled.
                     */

                    nbw_para->callback(NVDM_STATUS_ITEM_MERGED_BY_BW, nbw_para->user_data);

                    /* Remove the specified data item from the list. */
                    nbw_para->item.prev->next = nbw_para->item.next;
                    nbw_para->item.next->prev = nbw_para->item.prev;
                    nvdm_port_free((void *)nbw_para);

                    nvdm_log_error("The User of { %s, %s } uses both BW & NBW.", group_name, item_name);
                    op_res = NVDM_STATUS_OK;

                    /* There may be multiple NBW requests to the same item on DCLL,
                     * so driver can't break out of the loop directly here.
                     */
                    ++canceled_item_count;
                }
            }
        }

        if (canceled_item_count == 0) {
#if MARK_UNUSED_LOG_INFO_CALL
            nvdm_log_msgid_info(nvdm_124, 2, nbw_para->item.next, &g_ncb.nbw_reqs);
#endif
        }

        nbw_para = backup_next;
    }
    if (canceled_item_count != 0) {
        nvdm_log_msgid_warning(nvdm_117, 1, canceled_item_count);
    }
    nvdm_port_protect_mutex_give();
    return op_res;
}

#endif /* SYSTEM_DAEMON_TASK_ENABLE */


static void data_item_delete(uint32_t partition, uint32_t index)
{
    nvdm_partition_info_t *p_info = &(g_ncb.partition[partition]);
    uint32_t size;
    data_item_status_t status;
    data_item_header_t header;

    /* change the status of data item in flash from valid to delete */
    status = DATA_ITEM_STATUS_DELETE;
    peb_write_data(partition,
                   p_info->hdrs[index].pnum,
                   p_info->hdrs[index].offset,
                   (uint8_t *)&status,
                   1);
#if MARK_UNUSED_POWEROFF_CALL
    nvdm_port_poweroff(2);
#endif
    peb_read_data(partition, p_info->hdrs[index].pnum, p_info->hdrs[index].offset, (uint8_t *)&header, DATA_ITEM_HEADER_SIZE);

    /* recalculate the dirty value of that PEB */
    size = DATA_ITEM_HEADER_SIZE +
           header.group_name_size +
           header.data_item_name_size +
           header.value_size +
           DATA_ITEM_CHECKSUM_SIZE;
    peb_add_drity(partition, header.pnum, size);
    /* free the data item header in memory */
    p_info->hdrs[index].status = DATA_ITEM_STATUS_DELETE;
    /* update global variables */
    p_info->curr_item_count--;
    space_sub_valid(partition, size);
}


nvdm_status_t nvdm_delete_data_item(const char *group_name, const char *data_item_name)
{
    int32_t index;
    uint32_t partition;
    nvdm_partition_info_t *p_info = NULL;
    nvdm_partition_cfg_t *p_cfg = NULL;

#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_025, 0);
#endif

    if ((group_name == NULL) || (data_item_name == NULL)) {
        nvdm_log_msgid_error(nvdm_118, 1, 'D');
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    if (g_ncb.init_status == false) {
        nvdm_log_msgid_error(nvdm_121, 1, 'D');
        return NVDM_STATUS_ERROR;
    }

    nvdm_port_mutex_take();
    nvdm_port_protect_mutex_take();

    partition = determine_which_partition(
                    group_name, data_item_name,
                    NVDM_DATA_ITEM_TYPE_RAW_DATA, NULL, 0, &index, 0);
    if (INVALID_PARTITION == partition) {
        nvdm_port_protect_mutex_give();
        nvdm_port_mutex_give();
        return NVDM_STATUS_ERROR;
    }
    p_info = &(g_ncb.partition[partition]);
    p_cfg = p_info->p_cfg;

    if ((strlen(group_name) > p_cfg->max_group_name_size) ||
        (strlen(data_item_name) > p_cfg->max_item_name_size)) {
        nvdm_log_msgid_error(nvdm_120, 5, 'D',
                             p_cfg->max_group_name_size, strlen(group_name),
                             p_cfg->max_item_name_size, strlen(data_item_name)
                            );
        nvdm_port_protect_mutex_give();
        nvdm_port_mutex_give();
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    if (index < 0) {
        nvdm_port_protect_mutex_give();
        nvdm_port_mutex_give();
        return NVDM_STATUS_ITEM_NOT_FOUND;
    }

    nvdm_log_warning("task: %s delete { %u, %s, %s }",
                     nvdm_port_get_curr_task_name(),
                     partition, group_name, data_item_name
                    );

    data_item_delete(partition, index);

    nvdm_port_protect_mutex_give();
    nvdm_port_mutex_give();

    return NVDM_STATUS_OK;
}


nvdm_status_t nvdm_delete_group(const char *group_name)
{
    uint32_t index, partition;
    char str[64];
    bool delete_done;
    nvdm_partition_info_t *p_info = NULL;
    nvdm_partition_cfg_t *p_cfg = NULL;
    data_item_header_t  header;

#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_026, 0);
#endif

    if (group_name == NULL) {
        nvdm_log_msgid_error(nvdm_118, 1, 'D');
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    if (g_ncb.init_status == false) {
        nvdm_log_msgid_error(nvdm_121, 1, 'D');
        return NVDM_STATUS_ERROR;
    }

    nvdm_port_mutex_take();
    nvdm_port_protect_mutex_take();

    delete_done = false;
    for (partition = 0; partition < g_ncb.partition_count; partition++) {
        p_info = &(g_ncb.partition[partition]);
        p_cfg = p_info->p_cfg;

        for (index = 0; index < p_cfg->total_item_count; index++) {
            /* skip free date item header */
            if (p_info->hdrs[index].status != DATA_ITEM_STATUS_VALID) {
                continue;
            }
            peb_read_data(partition,
                          p_info->hdrs[index].pnum,
                          p_info->hdrs[index].offset,
                          (uint8_t *) &header,
                          DATA_ITEM_HEADER_SIZE
                         );
            /* check whether group name is match */
            peb_read_data(partition,
                          header.pnum,
                          header.offset + DATA_ITEM_HEADER_SIZE,
                          (uint8_t *)str, header.group_name_size
                         );
            if (strcmp(str, group_name) != 0) {
                continue;
            }
            data_item_delete(partition, index);
            delete_done = true;
        }
    }

    nvdm_port_protect_mutex_give();
    nvdm_port_mutex_give();

    if (delete_done == false) {
        return NVDM_STATUS_ITEM_NOT_FOUND;
    }

    return NVDM_STATUS_OK;
}


nvdm_status_t nvdm_delete_all(void)
{
    uint32_t index;
    bool delete_done;
    uint32_t partition;
    nvdm_partition_info_t *p_info = NULL;
    nvdm_partition_cfg_t *p_cfg = NULL;

#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_027, 0);
#endif

    if (g_ncb.init_status == false) {
        nvdm_log_msgid_error(nvdm_121, 1, 'D');
        return NVDM_STATUS_ERROR;
    }

    nvdm_port_mutex_take();
    nvdm_port_protect_mutex_take();

    delete_done = false;

    for (partition = 0; partition < g_ncb.partition_count; partition++) {
        p_info = &(g_ncb.partition[partition]);
        p_cfg = p_info->p_cfg;

        for (index = 0; index < p_cfg->total_item_count; index++) {
            /* skip free date item header */
            if (p_info->hdrs[index].status != DATA_ITEM_STATUS_VALID) {
                continue;
            }
            data_item_delete(partition, index);
            delete_done = true;
        }
    }

    nvdm_port_protect_mutex_give();
    nvdm_port_mutex_give();

    if (delete_done == false) {
        return NVDM_STATUS_ITEM_NOT_FOUND;
    }

    return NVDM_STATUS_OK;
}


nvdm_status_t nvdm_query_begin(void)
{
    bool is_frist_group_name;
    uint32_t i, malloc_size, curr_iter_idx, items_before_this;
    uint8_t *search_bitmask;
    uint32_t partition, curr_item_num, total_item_num;
    nvdm_partition_info_t *p_info;
    nvdm_partition_cfg_t *p_cfg;
    nvdm_status_t status = NVDM_STATUS_OK;
    //char search_str[64], compare_str[64];
    char *search_str;
    char *compare_str;
    data_item_header_t header;

#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_028, 0);
#endif

    if (g_ncb.init_status == false) {
        nvdm_log_msgid_error(nvdm_121, 1, 'Q');
        return NVDM_STATUS_ERROR;
    }

    nvdm_port_mutex_take();

    if (g_ncb.during_query == true) {
        nvdm_port_mutex_give();
        return NVDM_STATUS_ERROR;
    }

    search_str  = nvdm_port_malloc(64);
    compare_str = nvdm_port_malloc(64);
    if (search_str == NULL || compare_str == NULL) {
        nvdm_log_msgid_error(nvdm_130, 1, 0);
        status = NVDM_STATUS_ERROR;
        goto free_ret;
    }

    nvdm_port_get_task_handler();

    curr_item_num = 0;
    total_item_num = 0;
    for (partition = 0; partition < g_ncb.partition_count; partition++) {
        p_info = &(g_ncb.partition[partition]);
        p_cfg = p_info->p_cfg;
        curr_item_num += p_info->curr_item_count;
        total_item_num += p_cfg->total_item_count;
    }

    /* malloc memory used by search table */
    malloc_size = (curr_item_num + 1) * sizeof(uint32_t) + (curr_item_num) * sizeof(query_item_t) + (total_item_num / 8 + 1);
    g_ncb.grp_name_table = (uint32_t *)nvdm_port_malloc(malloc_size);
    if (g_ncb.grp_name_table == NULL) {
        nvdm_port_mutex_give();
        nvdm_log_msgid_error(nvdm_130, 1, 1);
        status = NVDM_STATUS_ERROR;
        goto free_ret;
    }
    memset((void *)g_ncb.grp_name_table, 0, malloc_size);

    /* need one more to store the max existed_items_num */
    g_ncb.item_name_table = (query_item_t *)(g_ncb.grp_name_table + curr_item_num + 1);
    search_bitmask = (uint8_t *)(g_ncb.item_name_table + curr_item_num);

    /* search and reorder the group name table */
    curr_iter_idx = 0;
    g_ncb.group_count = 0;
    while (curr_iter_idx < curr_item_num) {
        /* search next group name */
        is_frist_group_name = true;
        items_before_this = 0;
        for (partition = 0; partition < g_ncb.partition_count; partition++) {
            p_info = &(g_ncb.partition[partition]);
            p_cfg = p_info->p_cfg;
            for (i = 0; i < p_cfg->total_item_count; i++) {
                /* skip free hole and the data item searched before */
                if ((p_info->hdrs[i].status != DATA_ITEM_STATUS_VALID) ||
                    (search_bitmask[(items_before_this + i) / 8] & (1 << ((items_before_this + i) % 8)))
                   ) {
                    continue;
                }

                /*read back header for check*/
                peb_read_data(partition, p_info->hdrs[i].pnum, p_info->hdrs[i].offset, (uint8_t *)&header, DATA_ITEM_HEADER_SIZE);

                /* the frist data item is dirrectly used as search name */
                if (is_frist_group_name == true) {
                    g_ncb.grp_name_table[g_ncb.group_count] = curr_iter_idx;
                    ++g_ncb.group_count;
                    peb_read_data(partition,
                                  header.pnum,
                                  header.offset + DATA_ITEM_HEADER_SIZE,
                                  (uint8_t *)search_str,
                                  header.group_name_size
                                 );
                    g_ncb.item_name_table[curr_iter_idx].par = partition;
                    g_ncb.item_name_table[curr_iter_idx].idx = i;
                    ++curr_iter_idx;
                    search_bitmask[(items_before_this + i) / 8] |= 1 << ((items_before_this + i) % 8);
                    is_frist_group_name = false;
                } else {
                    peb_read_data(partition,
                                  header.pnum,
                                  header.offset + DATA_ITEM_HEADER_SIZE,
                                  (uint8_t *)compare_str,
                                  header.group_name_size
                                 );
                    if (strcmp(search_str, compare_str) == 0) {
                        g_ncb.item_name_table[curr_iter_idx].par = partition;
                        g_ncb.item_name_table[curr_iter_idx].idx = i;
                        ++curr_iter_idx;
                        search_bitmask[(items_before_this + i) / 8] |= 1 << ((items_before_this + i) % 8);
                    }
                }
            }

            items_before_this += p_cfg->total_item_count;
        }
    }
    g_ncb.grp_name_table[g_ncb.group_count] = curr_item_num;
    g_ncb.curr_group_idx = 0;
    g_ncb.during_query = true;

free_ret:
    nvdm_port_free(search_str);
    nvdm_port_free(compare_str);
    return status;
}

nvdm_status_t nvdm_query_end(void)
{
#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_029, 0);
#endif

    if (nvdm_port_query_task_handler() == false) {
        return NVDM_STATUS_ERROR;
    }

    if (g_ncb.during_query == false) {
        return NVDM_STATUS_ERROR;
    }

    nvdm_port_free(g_ncb.grp_name_table);

    g_ncb.during_query = false;
    nvdm_port_reset_task_handler();

    nvdm_port_mutex_give();

    return NVDM_STATUS_OK;
}

nvdm_status_t nvdm_query_next_group_name(char *group_name)
{
    uint32_t index, partition;
    data_item_header_t *hdr;
    nvdm_partition_info_t *p_info;
    data_item_header_t header;

#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_030, 0);
#endif

    if (group_name == NULL) {
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    if (nvdm_port_query_task_handler() == false) {
        return NVDM_STATUS_ERROR;
    }

    if (g_ncb.during_query == false) {
        return NVDM_STATUS_ERROR;
    }

    if (g_ncb.curr_group_idx >= g_ncb.group_count) {
        return NVDM_STATUS_ITEM_NOT_FOUND;
    }
    index = g_ncb.grp_name_table[g_ncb.curr_group_idx++];
    partition = g_ncb.item_name_table[index].par;
    index = g_ncb.item_name_table[index].idx;
    p_info = &(g_ncb.partition[partition]);
    hdr = &header;
    peb_read_data(partition, p_info->hdrs[index].pnum, p_info->hdrs[index].offset, (uint8_t *)&header, DATA_ITEM_HEADER_SIZE);
    /* check whether group name is match */
    peb_read_data(partition, hdr->pnum, hdr->offset + DATA_ITEM_HEADER_SIZE, (uint8_t *)group_name, hdr->group_name_size);

    g_ncb.curr_item_idx = 0;

    return NVDM_STATUS_OK;
}

nvdm_status_t nvdm_query_data_item_count(const char *group_name, uint32_t *count)
{
    if ((group_name == NULL) || (count == NULL)) {
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    if (nvdm_port_query_task_handler() == false) {
        return NVDM_STATUS_ERROR;
    }

    if (g_ncb.during_query == false) {
        return NVDM_STATUS_ERROR;
    }

    /* Forbid to call nvdm_query_next_data_item_name() before nvdm_query_next_group_name() */
    if (g_ncb.curr_group_idx == 0) {
        return NVDM_STATUS_ERROR;
    }

    *count = g_ncb.grp_name_table[g_ncb.curr_group_idx] - g_ncb.grp_name_table[g_ncb.curr_group_idx - 1];

    return NVDM_STATUS_OK;
}

nvdm_status_t nvdm_query_next_data_item_name(char *data_item_name)
{
    uint32_t index;
    data_item_header_t *hdr;
    uint32_t partition;
    nvdm_partition_info_t *p_info;
    data_item_header_t header;

#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_031, 0);
#endif

    if (data_item_name == NULL) {
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    if (nvdm_port_query_task_handler() == false) {
        return NVDM_STATUS_ERROR;
    }

    if (g_ncb.during_query == false) {
        return NVDM_STATUS_ERROR;
    }

    /* Forbid to call nvdm_query_next_data_item_name() before nvdm_query_next_group_name() */
    if (g_ncb.curr_group_idx == 0) {
        return NVDM_STATUS_ERROR;
    }

    if (g_ncb.curr_item_idx >= (g_ncb.grp_name_table[g_ncb.curr_group_idx] - g_ncb.grp_name_table[g_ncb.curr_group_idx - 1])) {
        return NVDM_STATUS_ITEM_NOT_FOUND;
    }

    index = g_ncb.grp_name_table[g_ncb.curr_group_idx - 1];
    partition = g_ncb.item_name_table[index + g_ncb.curr_item_idx].par;
    index = g_ncb.item_name_table[index + g_ncb.curr_item_idx].idx;
    ++g_ncb.curr_item_idx;
    p_info = &(g_ncb.partition[partition]);
    hdr = &header;
    peb_read_data(partition, p_info->hdrs[index].pnum, p_info->hdrs[index].offset, (uint8_t *)&header, DATA_ITEM_HEADER_SIZE);

    /* check whether group name is match */
    peb_read_data(partition,
                  hdr->pnum,
                  hdr->offset + DATA_ITEM_HEADER_SIZE + hdr->group_name_size,
                  (uint8_t *)data_item_name,
                  hdr->data_item_name_size);

    return NVDM_STATUS_OK;
}


/* It is not a common Query class API, but AT/RACE cmd or nvkey wrapper needs to use them. */

nvdm_status_t nvdm_query_data_item_length(const char *group_name, const char *data_item_name, uint32_t *size)
{
    uint32_t partition;
    int32_t index;
    nvdm_partition_info_t *p_info;
    data_item_header_t header;

#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_032, 0);
#endif

    if ((group_name == NULL) ||
        (data_item_name == NULL) ||
        (size == NULL)) {
        if (size != NULL) {
            *size = 0;
        }
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    if (g_ncb.init_status == false) {
        nvdm_log_msgid_error(nvdm_121, 1, 'Q');
        return NVDM_STATUS_ERROR;
    }

    nvdm_port_protect_mutex_take();
    partition = determine_which_partition(group_name, data_item_name, NVDM_DATA_ITEM_TYPE_RAW_DATA, NULL, 0, &index, 0);
    if (INVALID_PARTITION == partition) {
        nvdm_port_protect_mutex_give();
        return NVDM_STATUS_ERROR;
    }
    p_info = &(g_ncb.partition[partition]);

    if (index < 0) {
        *size = 0;
        nvdm_port_protect_mutex_give();
        return NVDM_STATUS_ITEM_NOT_FOUND;
    }
    peb_read_data(partition, p_info->hdrs[index].pnum, p_info->hdrs[index].offset, (uint8_t *)&header, DATA_ITEM_HEADER_SIZE);

    *size = header.value_size;

    nvdm_port_protect_mutex_give();

#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_info("group_name = %s, data_item_name = %s, size = %d", group_name, data_item_name, *size);
#endif

    return NVDM_STATUS_OK;
}


nvdm_status_t nvdm_query_data_item_type(const char *group_name, const char *data_item_name, nvdm_data_item_type_t *type)
{
    int32_t index;
    uint32_t partition;
    nvdm_partition_info_t *p_info;
    data_item_header_t header;

    if ((group_name == NULL) ||
        (data_item_name == NULL) ||
        (type == NULL)) {
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    if (g_ncb.init_status == false) {
        nvdm_log_msgid_error(nvdm_121, 1, 'Q');
        return NVDM_STATUS_ERROR;
    }

    nvdm_port_protect_mutex_take();

    partition = determine_which_partition(group_name, data_item_name, NVDM_DATA_ITEM_TYPE_RAW_DATA, NULL, 0, &index, 0);
    if (INVALID_PARTITION == partition) {
        nvdm_port_protect_mutex_give();
        return NVDM_STATUS_ERROR;
    }
    p_info = &(g_ncb.partition[partition]);

    if (index < 0) {
        nvdm_port_protect_mutex_give();
        return NVDM_STATUS_ITEM_NOT_FOUND;
    }
    peb_read_data(partition, p_info->hdrs[index].pnum, p_info->hdrs[index].offset, (uint8_t *)&header, DATA_ITEM_HEADER_SIZE);
    nvdm_port_protect_mutex_give();
    *type = header.type;

    return NVDM_STATUS_OK;
}

nvdm_status_t nvdm_query_space_info(uint32_t *total_avail_space, uint32_t *curr_used_space)
{
    nvdm_partition_info_t *p_info = &(g_ncb.partition[0]);

    if ((total_avail_space == NULL) ||
        (curr_used_space == NULL)
       ) {
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    if (g_ncb.init_status == false) {
        nvdm_log_msgid_error(nvdm_121, 1, 'Q');
        return NVDM_STATUS_ERROR;
    }

    nvdm_port_mutex_take();

    *total_avail_space = p_info->avaliable_space;
    *curr_used_space = p_info->valid_data_size;

    nvdm_port_mutex_give();

    return NVDM_STATUS_OK;
}


nvdm_status_t nvdm_query_space_by_partition(uint32_t partition, uint32_t *total_avail_space, uint32_t *curr_used_space)
{
    nvdm_partition_info_t *p_info = &(g_ncb.partition[partition]);

    if ((total_avail_space == NULL) ||
        (curr_used_space == NULL) ||
        (partition >= g_ncb.partition_count)
       ) {
        return NVDM_STATUS_INVALID_PARAMETER;
    }

    if (g_ncb.init_status == false) {
        nvdm_log_msgid_error(nvdm_121, 1, 'Q');
        return NVDM_STATUS_ERROR;
    }

    nvdm_port_mutex_take();

    *total_avail_space = p_info->avaliable_space;
    *curr_used_space = p_info->valid_data_size;

    nvdm_port_mutex_give();

    return NVDM_STATUS_OK;
}

/* It is not a common Query class API, but AT/RACE cmd or nvkey wrapper needs to use them. */


#endif  /* #ifndef __EXT_BOOTLOADER__ */

static bool calculate_data_item_index(data_item_header_t *data_item_header, uint32_t *curr_index)
{
    bool is_old_index;

    if (data_item_header->reserved == 0xFF) {
        *curr_index = data_item_header->index;
        is_old_index = true;
    } else {
        *curr_index = (uint32_t)(data_item_header->index) +
                      ((uint32_t)(data_item_header->reserved & 0xFF) << 8);
        is_old_index = false;
    }

    return is_old_index;
}


void data_item_scan(uint32_t partition, int32_t pnum)
{
    uint32_t curr_index;
    int32_t offset, oldpnum, old_offset;
    int32_t peb_drity, peb_valid;
    data_item_header_t data_item_header, cur_header;
    uint16_t checksum1, checksum2;
    uint32_t size;
    static int32_t abnormal_data_item = -1;
    data_item_status_t status;
    nvdm_partition_info_t *p_info = &(g_ncb.partition[partition]);
    nvdm_partition_cfg_t *p_cfg = p_info->p_cfg;

    const char *p_group_name = NULL, *p_item_name = NULL;
    uint32_t scan_limit = p_cfg->peb_size - PEB_HEADER_SIZE - DATA_ITEM_HEADER_SIZE;

#if MARK_UNUSED_LOG_INFO_CALL
    nvdm_log_msgid_info(nvdm_034, 1, pnum);
#endif
    offset = 0;
    peb_drity = 0;
    peb_valid = 0;
    /* scan entire peb content */
    while (offset < scan_limit) {
        peb_read_data(partition, pnum, offset, (uint8_t *)&data_item_header, DATA_ITEM_HEADER_SIZE);
        data_item_header_print_info(&data_item_header);
        size = DATA_ITEM_HEADER_SIZE + data_item_header.group_name_size + data_item_header.data_item_name_size + data_item_header.value_size + DATA_ITEM_CHECKSUM_SIZE;
        if ((size > scan_limit) ||
            (peb_drity + size > p_cfg->peb_size) ||
            (peb_valid + size > p_cfg->peb_size)
           ) {
            /* When the size of a data item exceeds the specified range, it may cause data confusion.
             * Here, the subsequent data of the flash block is marked as garbage, and it will be processed during garbage collection later.
             */
            if (data_item_header.status != DATA_ITEM_STATUS_EMPTY) {
                nvdm_log_msgid_error(nvdm_116, 5, pnum, offset, size, peb_valid, peb_drity);
                peb_add_drity(partition, pnum, (p_cfg->peb_size - PEB_HEADER_SIZE) - peb_drity - peb_valid);
                return;
            }
        }
        switch (data_item_header.status) {
            case DATA_ITEM_STATUS_EMPTY:
                peb_add_free(partition, pnum, (p_cfg->peb_size - PEB_HEADER_SIZE) - peb_drity - peb_valid);
                return;
            case DATA_ITEM_STATUS_WRITING:
                /* we can't belive data item header if we found it's writting,
                          * so just mark rest of space is dirty.
                          */
                peb_add_drity(partition, pnum, (p_cfg->peb_size - PEB_HEADER_SIZE) - peb_drity - peb_valid);
                return;
            case DATA_ITEM_STATUS_VALID:
                break;
            case DATA_ITEM_STATUS_DELETE:
                peb_drity += size;
                offset += size;
                peb_add_drity(partition, pnum, size);
                continue;
            default:
                /* Mark the data behind it as garbage, so that the data will be reclaimed during GC. */
                nvdm_log_msgid_error(nvdm_035, 2, pnum, offset);
                peb_add_drity(partition, pnum, (p_cfg->peb_size - PEB_HEADER_SIZE) - peb_drity - peb_valid);
                return;
        }

        calculate_data_item_index(&data_item_header, &curr_index);
        if (curr_index >= p_cfg->total_item_count) {
            nvdm_log_msgid_error(nvdm_036, 2, p_cfg->total_item_count, curr_index);
            /* Must assert the configuration error, or else NVDM can not work. */
            nvdm_port_must_assert();
            return;
        }

        old_offset = offset;

        offset += DATA_ITEM_HEADER_SIZE + data_item_header.group_name_size + data_item_header.data_item_name_size + data_item_header.value_size;
        peb_read_data(partition, pnum, offset, (uint8_t *)&checksum2, DATA_ITEM_CHECKSUM_SIZE);
        offset += DATA_ITEM_CHECKSUM_SIZE;

        /* verify checksum of data item */
        if (data_item_header.group_name_size + data_item_header.data_item_name_size > NVDM_BUFFER_SIZE) {
            /* skip the invalid data item header with wrong name size */
            checksum1 = ~checksum2;
        } else {
            checksum1 = calculate_data_item_checksum(&data_item_header, partition, pnum, data_item_header.offset + DATA_ITEM_HEADER_SIZE);
        }
        if (checksum1 != checksum2) {
            nvdm_log_msgid_warning(nvdm_037, 0);

            peb_drity += size;
            peb_add_drity(partition, pnum, size);

            /* When the checksum error is detected, the item needs to be deleted.
             * If it is not deleted, always assert may occur !!!
             * Since the checksum of the current item is incorrect, its index may be occupied by other items.
             * If NVDM need to recycle the item whose checksum is abnormal when garbage collection is triggered,
             * the driver will determine that there is a pair of items with the same index, and their status are valid.
             * This situation is absolutely not allowed in NVDM.
             */
            data_item_header.status = DATA_ITEM_STATUS_DELETE;
            peb_write_data(partition, pnum, old_offset, (const uint8_t *)(&(data_item_header.status)), 1);

            /* For compatibility reasons, the global physical address is not used directly to access the flash.
             * Here the address is only offset against the Flash starting address.
             */
            if (data_item_header.group_name_size + data_item_header.data_item_name_size <= NVDM_BUFFER_SIZE) {
                p_group_name = (const char *)(&g_ncb.working_buffer);
                peb_read_data(
                    partition,
                    pnum,
                    old_offset + DATA_ITEM_HEADER_SIZE,
                    (uint8_t *)p_group_name,
                    data_item_header.group_name_size
                );
                g_ncb.working_buffer[data_item_header.group_name_size] = '\0';

                p_item_name = (const char *)(&g_ncb.working_buffer + data_item_header.group_name_size);
                peb_read_data(
                    partition,
                    pnum,
                    old_offset + DATA_ITEM_HEADER_SIZE + data_item_header.group_name_size,
                    (uint8_t *)p_item_name,
                    data_item_header.data_item_name_size
                );
                g_ncb.working_buffer[data_item_header.group_name_size + data_item_header.data_item_name_size] = '\0';

                nvdm_log_warning("mark { %s, %s }, ( %d, %d ) <==> ( %d, %d ) with read size %d as DELETE",
                                 p_group_name, p_item_name, pnum, old_offset,
                                 data_item_header.pnum, data_item_header.offset, size
                                );
            }

            continue;
        }
        if (p_info->hdrs[curr_index].status == DATA_ITEM_STATUS_VALID) {
            peb_read_data(partition, p_info->hdrs[curr_index].pnum, p_info->hdrs[curr_index].offset, (uint8_t *)&cur_header, DATA_ITEM_HEADER_SIZE);
        }

        /* update count of data items */
        if (p_info->hdrs[curr_index].status != DATA_ITEM_STATUS_VALID) {
            /* we find this frist time */
            update_data_item_header_on_ram(&p_info->hdrs[curr_index], &data_item_header);

            p_info->curr_item_count++;
            if (p_info->curr_item_count > p_cfg->total_item_count) {
                nvdm_log_msgid_error(nvdm_038, 0);
                /* Must assert the configuration error, or else NVDM can not work. */
                nvdm_port_must_assert();
            }
            peb_valid += size;
        } else {
            /* we found it before, so compare sequence number of them
                    * this is possible that new copy is total update
                    * but old copy has not been invalidated when power-off happen.
                    */
#if MARK_UNUSED_LOG_INFO_CALL
            nvdm_log_msgid_info(nvdm_039, 0);
#endif
            nvdm_log_msgid_warning(nvdm_040, 4,
                                   p_info->hdrs[curr_index].pnum,
                                   p_info->hdrs[curr_index].offset,
                                   data_item_header.pnum,
                                   data_item_header.offset);
            if (abnormal_data_item > 0) {
                /* this should only happen once at most */

                /* The item has been checked by the checksum.
                 * But there are two or more pairs of NVDM data items with the same Index( NVDM internal ID ).
                 * Delete data items that were discovered later.
                 */
                nvdm_log_msgid_error(nvdm_041, 3, abnormal_data_item, pnum, old_offset);
                status = DATA_ITEM_STATUS_DELETE;
                peb_write_data(partition, pnum, old_offset, (uint8_t *)&status, 1);
                peb_drity += size;
                peb_add_drity(partition, pnum, size);
                ++abnormal_data_item;
                continue;
            } else {
                abnormal_data_item = 1;
            }
            if (cur_header.sequence_number < data_item_header.sequence_number) {
                /* we find new copy, so mark old peb as delete */
                status = DATA_ITEM_STATUS_DELETE;
                peb_write_data(partition,
                               p_info->hdrs[curr_index].pnum,
                               p_info->hdrs[curr_index].offset,
                               (uint8_t *)&status,
                               1);
#if MARK_UNUSED_POWEROFF_CALL
                nvdm_port_poweroff(5);
#endif
                /* add valid info */
                peb_valid += size;
                /* add dirty info */
                oldpnum = p_info->hdrs[curr_index].pnum;
                size = cur_header.value_size +
                       cur_header.group_name_size +
                       cur_header.data_item_name_size +
                       DATA_ITEM_CHECKSUM_SIZE + DATA_ITEM_HEADER_SIZE;
                peb_add_drity(partition, oldpnum, size);
                /* if we found old copy in same peb, we must substract it's size from peb_valid */
                if (oldpnum == pnum) {
                    peb_valid -= size;
                }
                /*update data item header info on ram*/
                update_data_item_header_on_ram(&p_info->hdrs[curr_index], &data_item_header);

                /* if we found it in the same peb last time */
                if (oldpnum == pnum) {
                    peb_drity += size;
                }
            } else {
                /* we find old copy, so mark it as delete directly */
                status = DATA_ITEM_STATUS_DELETE;
                peb_write_data(partition,
                               data_item_header.pnum,
                               data_item_header.offset,
                               (uint8_t *)&status,
                               1);
#if MARK_UNUSED_POWEROFF_CALL
                nvdm_port_poweroff(6);
#endif
                peb_drity += size;
                peb_add_drity(partition, pnum, size);
            }
        }
    }

    /* If there is dark space exist, it should also be considered as free space. */
    if (offset >= (p_cfg->peb_size - PEB_HEADER_SIZE - DATA_ITEM_HEADER_SIZE)) {
        peb_add_free(partition, pnum, (p_cfg->peb_size - PEB_HEADER_SIZE) - peb_drity - peb_valid);
    }
}


static void data_migration(uint32_t partition, int32_t src_pnum, int32_t src_offset,
                           int32_t dst_pnum, int32_t dst_offset, int32_t size)
{
    int32_t i, delta, fragment;
    uint8_t *working_buffer = g_ncb.migration_buffer;

    fragment = size / NVDM_BUFFER_SIZE;
    delta = 0;
    for (i = 0; i < fragment; i++) {
        memset(working_buffer, 0, NVDM_BUFFER_SIZE);
        peb_read_data(partition, src_pnum, src_offset + delta, working_buffer, NVDM_BUFFER_SIZE);
        peb_write_data(partition, dst_pnum, dst_offset + delta, working_buffer, NVDM_BUFFER_SIZE);
        delta += NVDM_BUFFER_SIZE;
    }
    if (size % NVDM_BUFFER_SIZE) {
        memset(working_buffer, 0, NVDM_BUFFER_SIZE);
        peb_read_data(partition, src_pnum, src_offset + delta, working_buffer, size % NVDM_BUFFER_SIZE);
        peb_write_data(partition, dst_pnum, dst_offset + delta, working_buffer, size % NVDM_BUFFER_SIZE);
    }
}


int32_t data_item_migration(uint32_t partition, int32_t src_pnum, int32_t dst_pnum, int32_t offset)
{
    data_item_header_t data_item_header, tmp_hdr;
    data_item_status_t status;
    int32_t pos, size, scan_range;
    uint16_t checksum;
    uint32_t curr_index;
    bool is_old_index;
    nvdm_partition_info_t *p_info = &(g_ncb.partition[partition]);
    nvdm_partition_cfg_t *p_cfg = p_info->p_cfg;

    /* search valid data item */
    pos = 0;
    scan_range = p_cfg->peb_size - PEB_HEADER_SIZE - p_info->pebs[src_pnum].free;
    while (pos < scan_range) {
        peb_read_data(partition, src_pnum, pos, (uint8_t *)&data_item_header, DATA_ITEM_HEADER_SIZE);
        size = DATA_ITEM_HEADER_SIZE +
               data_item_header.group_name_size + data_item_header.data_item_name_size +
               data_item_header.value_size +
               DATA_ITEM_CHECKSUM_SIZE;

        if ((size > scan_range) ||
            ((pos + size) > scan_range)
           ) {
            peb_print_info(partition);
            nvdm_log_msgid_warning(nvdm_116, 5, src_pnum, pos, size, scan_range, data_item_header.status);
            return offset;
        }

        switch (data_item_header.status) {
            case DATA_ITEM_STATUS_WRITING:
            case DATA_ITEM_STATUS_EMPTY:
                /* no more data item after it, just return */
                return offset;
            case DATA_ITEM_STATUS_DELETE:
                /* do nothing, just skip it to find next data item.
                          * data item is marked as delete status, it must be an old copy.
                          */
                pos += size;
                break;
            case DATA_ITEM_STATUS_VALID:
                is_old_index = calculate_data_item_index(&data_item_header, &curr_index);
                if ((p_info->hdrs[curr_index].pnum != data_item_header.pnum) ||
                    (p_info->hdrs[curr_index].offset != data_item_header.offset)
                   ) {
                    /* find old copy, this should not happen,
                                 * because it's fixed in init phase.
                                 */
                    nvdm_log_msgid_error(nvdm_044, 5,
                                         src_pnum, pos,
                                         p_info->hdrs[curr_index].pnum,
                                         p_info->hdrs[curr_index].offset,
                                         size);

                    /* In the process of nvdm_init, delete data items whose status is VALID but with abnormal data.
                     * Delete data items that were discovered later.
                     */
                    status = DATA_ITEM_STATUS_DELETE;
                    peb_write_data(partition, src_pnum, pos, (uint8_t *)&status, 1);
                    /* update offset for next item */
                    pos += size;
                    peb_add_drity(partition, src_pnum, size);
                } else {
                    memcpy(&tmp_hdr, &data_item_header, DATA_ITEM_HEADER_SIZE);

                    /* find up-to-date copy, so migrate it to target peb update header */
                    tmp_hdr.pnum = dst_pnum;
                    tmp_hdr.offset = offset;

                    /* old version use only one byte to decide the index */
                    if (is_old_index) {
                        tmp_hdr.reserved = 0xFF00;
                    }

                    /* calculate new checksum */
                    nvdm_port_protect_mutex_take();
                    checksum = calculate_data_item_checksum(&tmp_hdr, partition, src_pnum, data_item_header.offset + DATA_ITEM_HEADER_SIZE);
                    nvdm_port_protect_mutex_give();

                    /* mark writ of beginning */
                    status = DATA_ITEM_STATUS_WRITING;
                    peb_write_data(partition, dst_pnum, offset, (uint8_t *)&status, 1);
#if MARK_UNUSED_POWEROFF_CALL
                    nvdm_port_poweroff(7);
#endif

                    /* write header of data item */
                    peb_write_data(partition, dst_pnum, offset + 1, &tmp_hdr.pnum, DATA_ITEM_HEADER_SIZE - 1);

                    /* write group name, data item name and value of data item */
                    data_migration(partition, src_pnum, pos + DATA_ITEM_HEADER_SIZE, dst_pnum, offset + DATA_ITEM_HEADER_SIZE, data_item_header.group_name_size + data_item_header.data_item_name_size + data_item_header.value_size);

                    /* write checksum of data item */
                    peb_write_data(partition, dst_pnum, offset + DATA_ITEM_HEADER_SIZE + data_item_header.group_name_size + data_item_header.data_item_name_size + data_item_header.value_size, (uint8_t *)&checksum, DATA_ITEM_CHECKSUM_SIZE);

                    /* mark write of end */
                    status = DATA_ITEM_STATUS_VALID;
                    peb_write_data(partition, dst_pnum, offset, (uint8_t *)&status, 1);
#if MARK_UNUSED_POWEROFF_CALL
                    nvdm_port_poweroff(8);
#endif

                    /* substract free size of target peb */
                    peb_sub_free(partition, dst_pnum, size);

                    /* update offset for next write */
                    offset += size;
                    pos += size;

                    nvdm_port_protect_mutex_take();
                    update_data_item_header_on_ram(&p_info->hdrs[curr_index], &tmp_hdr);
                    nvdm_port_protect_mutex_give();
                }
                break;
            default:
                nvdm_log_msgid_error(nvdm_045, 3, data_item_header.status, src_pnum, pos);
                /* Mark the data behind it as garbage and no need to move the data behind. */
                return offset;
        }
    }

    return offset;
}


#ifndef __EXT_BOOTLOADER__
static nvdm_status_t reset_nvdm_parameter_check(const nvdm_item_id_t *array, uint32_t len, nvdm_reset_action_t action)
{
    nvdm_status_t check_res = NVDM_STATUS_OK;
    uint32_t idx = 0;
    const nvdm_item_id_t *cur = NULL;

    if (g_ncb.init_status == false) {
        nvdm_log_msgid_error(nvdm_121, 1, 'D');
        return NVDM_STATUS_ERROR;
    }

    if ((action > NVDM_ACTION_DELETE) || (len == 0) || (array == NULL)) {
        return NVDM_STATUS_INVALID_PARAMETER;
    }
    for (idx = 0; idx < len; idx++) {
        cur = array + idx;
        if (cur->group_name == NULL) {
            check_res = NVDM_STATUS_INVALID_PARAMETER;
            break;
        }
        if ((cur->group_name == NULL) && (cur->item_name == NULL)) {
            check_res = NVDM_STATUS_INVALID_PARAMETER;
            break;
        }
        /* When group_name is not NULL and item_name is NULL, it means all items in group. */
    }

    return check_res;
}

static bool reset_nvdm_item_match(const nvdm_item_id_t *array, uint32_t len, const char *group_name, const char *item_name)
{
    bool item_is_match  = false;
    uint32_t idx = 0;
    for (idx = 0; idx < len; idx++) {
        if (array[idx].item_name == NULL) {
            /* When group_name is not NULL and item_name is NULL,
             * it means all items in group.
             */
            if (strcmp(array[idx].group_name, group_name) == 0) {
                item_is_match = true;
                break;
            }
        } else {
            if ((strcmp(array[idx].group_name, group_name) == 0) &&
                (strcmp(array[idx].item_name, item_name) == 0)
               ) {
                item_is_match = true;
                break;
            }
        }
    }
    return item_is_match;
}

nvdm_status_t nvdm_reset_items(const nvdm_item_id_t *array, uint32_t len, nvdm_reset_action_t action)
{
    nvdm_status_t op_res = reset_nvdm_parameter_check(array, len, action);
    char *group_name = NULL, *item_name = NULL;
    bool item_is_match  = false;
    uint32_t partition = 0;
    nvdm_partition_info_t *p_info = &(g_ncb.partition[partition]);
    nvdm_partition_cfg_t *p_cfg = p_info->p_cfg;

    if (op_res != NVDM_STATUS_OK) {
        return op_res;
    }

    /* align with nvdm_write_data_item API, there is a '\0' at the end of C string */
    group_name = (char *)nvdm_port_malloc(p_cfg->max_group_name_size + 1);
    item_name = (char *)nvdm_port_malloc(p_cfg->max_item_name_size + 1);
    if ((group_name == NULL) || (item_name == NULL)) {
        if (group_name != NULL) {
            nvdm_port_free(group_name);
        }
        if (item_name != NULL) {
            nvdm_port_free(item_name);
        }
        return NVDM_STATUS_ERROR;
    }

    op_res = nvdm_query_begin();
    if (op_res != NVDM_STATUS_OK) {
        nvdm_port_free(group_name);
        nvdm_port_free(item_name);
        return op_res;
    }
    while (nvdm_query_next_group_name(group_name) == NVDM_STATUS_OK) {
        while (nvdm_query_next_data_item_name(item_name) == NVDM_STATUS_OK) {
            item_is_match = reset_nvdm_item_match(array, len, group_name, item_name);
            if (action == NVDM_ACTION_KEEP) {
                /* for NVDM_ACTION_KEEP, delete all items not in array */
                if (item_is_match == false) {
                    nvdm_delete_data_item(group_name, item_name);
                }
            } else {
                /* for NVDM_ACTION_DELETE, delete all items in array */
                if (item_is_match == true) {
                    nvdm_delete_data_item(group_name, item_name);
                }
            }
        }
    }
    op_res = nvdm_query_end();
    nvdm_port_free(group_name);
    nvdm_port_free(item_name);
    return op_res;
}
#endif /* #ifdef __EXT_BOOTLOADER__ */


#endif


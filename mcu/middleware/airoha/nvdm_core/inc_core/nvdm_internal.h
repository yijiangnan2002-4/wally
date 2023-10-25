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

#ifndef __NVDM_INTERNAL_H__
#define __NVDM_INTERNAL_H__

#ifdef MTK_NVDM_ENABLE

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "nvdm.h"
#include "nvdm_port.h"

#ifndef NULL
#define NULL    (void *)(0)
#endif


#define MARK_UNUSED_LOG_INFO_CALL               0
#define MARK_UNUSED_POWEROFF_CALL               0
#define AVOID_UNUSED_BUILD_WARNING(x)           (void)(x)


/* Check whether the size of the structure is
 * equal to a specific value at compile time.
 */
#define    SIZE_OF_TYPE_EQUAL_TO(type, size); \
static inline char check_size_of_##type##_equal_to_##size() \
{ \
    char __dummy1[sizeof(type) - size]; \
    char __dummy2[size - sizeof(type)]; \
    return __dummy1[0] + __dummy2[0]; \
}

typedef enum {
    PEB_STATUS_VIRGIN = 0xFF,          /* the block status is undefined,
                                          it maybe has erased or not erased completely */
    PEB_STATUS_EMPTY = 0xFE,           /* the block has valid PEB header, no valid data */
    PEB_STATUS_ACTIVING = 0xFC,        /* the block is frist being filled with data item */
    PEB_STATUS_TRANSFERING = 0xF8,     /* the block is being transfered data from reclaimed block */
    PEB_STATUS_TRANSFERED = 0xF0,      /* the block has being transfered data completely */
    PEB_STATUS_ACTIVED = 0xE0,         /* the block has valid PEB header and data */
    PEB_STATUS_RECLAIMING = 0xC0,      /* the block is very dirty, and is being reclaimed */
    PEB_STATUS_ERASING = 0x80,         /* the block is being erased */
} peb_status_t;

#define PEB_HEADER_MAGIC    (0x4d44564e)  /* "NVDM" */
#define PEB_UNRESERVED  (0xF0)
#define NVDM_VERSION    (0x01)

typedef struct {
    uint32_t magic;          /* offset: 0, len: 4; erase header magic number */
    uint32_t erase_count;    /* offset: 4, len: 4; erase count of this block */
    peb_status_t status;     /* offset: 8, len: 1; status of PEB */
    uint8_t peb_reserved;    /* offset: 9, len: 1; block is reserved and not used */
    uint8_t version;         /* offset: 10, len: 1; version of NVDM */
    uint8_t reserved;        /* offset: 11, len: 1; reserved byte */
} peb_header_t;
#define PEB_MAGIC_OFFSET        (0)     /* magic field offset to PEB begin */
#define PEB_ERASE_COUNT_OFFSET  (4)     /* erase_count field offset to PEB begin */
#define PEB_STATUS_OFFSET       (8)     /* status field offset to PEB begin */
#define PEB_RESERVED_OFFSET     (9)     /* peb_reserved field offset to PEB begin */
#define PEB_VERSION_OFFSET      (10)    /* peb_version field offset to PEB begin */
#define PEB_HEADER_SIZE sizeof(peb_header_t)
#define ERASE_COUNT_MAX (0xffffffff)

SIZE_OF_TYPE_EQUAL_TO(peb_header_t, 12);

#define MAX_WRITE_SEQUENCE_NUMBER   (0xffffffff)

typedef enum {
    DATA_ITEM_STATUS_EMPTY = 0xFF,      /* no data item exist after it */
    DATA_ITEM_STATUS_WRITING = 0xFE,    /* data item is being written to new place */
    DATA_ITEM_STATUS_VALID = 0xFC,      /* data item has been written to this place successfully */
    DATA_ITEM_STATUS_DELETE = 0xF8,     /* data item has been discarded because of new copy is ready */
} data_item_status_t;

typedef struct {
    data_item_status_t status;      /* offset: 0, len: 1; status of data item */
    uint8_t pnum;                   /* offset: 1, len: 1; which PEB this data item record store at */
    uint16_t reserved;              /* offset: 2, len: 2; reserved bytes */
    uint16_t offset;                /* offset: 4, len: 2; offset in PEB where data item record begin */
    uint8_t group_name_size;        /* offset: 6, len: 1; length of group name */
    uint8_t data_item_name_size;    /* offset: 7, len: 1; length of data item name */
    uint16_t value_size;            /* offset: 8, len: 2; size of data item's content */
    uint8_t index;                  /* offset: 10, len: 1; index for data item */
    nvdm_data_item_type_t type;     /* offset: 11, len: 1; display type of data item,
                                                           it can be type of binary/hex/decimal/string/structure */
    uint32_t sequence_number;       /* offset: 12, len: 4; write sequence number for this data item record */
    uint32_t hash_name;             /* offset: 16, len: 4; hash name of this data item */
} data_item_header_t;

#pragma pack(1)
typedef struct {
    uint8_t  status;                  /* offset: 0, len: 1; -2:empty, -1:invalid item, 0: valid item, 1:valid item & value_size > 0  */
    uint8_t  pnum;                  /* offset: 1, len: 1; which PEB this data item record store at */
    uint16_t offset;                /* offset: 2, len: 2; offset in PEB where data item record begin */
    uint8_t hash_name;             /* offset: 4, len: 2; hash name of this data item */
} data_item_header_on_ram_t;
#pragma pack()


#define DATA_ITEM_HEADER_SIZE   (sizeof(data_item_header_t))
#define DATA_ITEM_CHECKSUM_SIZE (2)
#define MAX_DATA_ITEM_SIZE  (2048)

SIZE_OF_TYPE_EQUAL_TO(data_item_header_t, 20);

/* define buffer size of internal used buffer */
#define NVDM_BUFFER_SIZE    (128)

/* Number of blocks used for reclaiming blocks */
#define NVDM_RESERVED_PEB_COUNT (1)

#ifdef SYSTEM_DAEMON_TASK_ENABLE

typedef struct list_item {
    struct list_item *prev;
    struct list_item *next;
} nvdm_dcll_t;
/* The dcll is an abbreviation of Doubly Circular Linked List. */

typedef struct {
    nvdm_dcll_t item;                 /* The node of the double circular linked list,
                                         and it is placed at the head for general use. */
    nvdm_user_callback_t callback;    /* The callback function passed in by the user which
                                         is called after NWB is completed or cancelled. */
    void *user_data;                  /* The user parameters that need to be passed into the callback. */
    uint8_t group_name_size;          /* The length of the group name of the data item. */
    uint8_t data_item_name_size;      /* The length of the item name of the data item. */
    uint16_t data_item_size;          /* The real length of the user data of the data item. */
    nvdm_data_item_type_t type;       /* The type of the data item. */
} async_nw_para_t;
#endif

typedef struct {
    uint16_t free;           /* Record the number of bytes in each block that have not been written. */
    uint16_t dirty;          /* Record the number of bytes in each block that have not marked deleted. */
    int32_t is_reserved;     /* Mark whether the block is reserved and used as the target block of the GC. */
    uint32_t erase_count;    /* Record the number of erasing of the current block. */
} peb_info_t;


typedef struct {
    nvdm_partition_cfg_t *p_cfg;    /* Point to all kinds of information about
                                       the current partitionobtained from the user. */

    uint32_t avaliable_space;       /* The maximum amount of available data calculated from
                                       the current partition configuration. */
    uint32_t valid_data_size;       /* Record the effective data amount of the current partition. */
    uint32_t curr_item_count;       /* The number of currently valid data items. */

    data_item_header_on_ram_t *hdrs;       /* The data item header information recorded in RAM.
                                       The number of this array is the same as p_cfg->total_item_count. */
    peb_info_t *pebs;               /* All block information in the current partition.
                                       The number of this array is the same as p_cfg->peb_count. */
    int32_t *gc_buffer;             /* The buffer used to sort the block numbers for GC.
                                       The number of this array is the same as p_cfg->peb_count. */
} nvdm_partition_info_t;


typedef struct {
    uint16_t par;    /* Used to mark the partition of the data item when querying. */
    uint16_t idx;    /* Used to mark the index of the data item when querying. */
} query_item_t;


typedef struct {
    uint32_t init_status;                /* Initialization status of nvdm driver. */

    uint32_t partition_count;            /* The number of partitions managed in the entire NVDM driver. */
    nvdm_partition_info_t *partition;    /* For all partition information, the number of arrays
                                            is the same as partition_count. */

    uint32_t during_query;               /* Whether it is currently in query state. */
    uint32_t group_count;                /* The number of all group names in the query process. */
    uint32_t curr_group_idx;             /* The index of the current group name in the query process. */
    uint32_t curr_item_idx;              /* The index of the current data item in the query process. */
    uint32_t *grp_name_table;            /* The array used to sort the group name in the query process.
                                            The number of arrays is the same as the number of data items in all partitions. */
    query_item_t *item_name_table;       /* The array of all data items index used to sort in the query process. */

#ifdef SYSTEM_DAEMON_TASK_ENABLE
    nvdm_dcll_t nbw_reqs;                /* The dummy node of the double circular linked list
                                            used by the non-blocking write service. */

    /* In order to make nvdm_write_data_item aware that it is called
     * by nvdm_write_data_item_non_blocking, this flag be added.
     */
    uint32_t has_nbw_req;                /* Indicates whether there is a request for non-blocking writing. */
    async_nw_para_t *curr_nbw;           /* Indicates which item is currently written. */
#endif

    uint8_t working_buffer[NVDM_BUFFER_SIZE];
    uint8_t migration_buffer[NVDM_BUFFER_SIZE];
} nvdm_ctrl_block_t;



void peb_print_info(uint32_t partition);
void data_item_scan(uint32_t partition, int32_t pnum);
int32_t data_item_migration(uint32_t partition, int32_t src_pnum, int32_t dst_pnum, int32_t offset);
void peb_io_read(uint32_t partition, int32_t pnum, int32_t offset, uint8_t *buf, int32_t len);
void peb_io_write(uint32_t partition, int32_t pnum, int32_t offset, const uint8_t *buf, int32_t len);
void peb_erase(uint32_t partition, int32_t pnum);
void peb_read_header(uint32_t partition, int32_t pnum, peb_header_t *peb_header);
void peb_write_data(uint32_t partition, int32_t pnum, int32_t offset, const uint8_t *buf, int32_t len);
void peb_read_data(uint32_t partition, int32_t pnum, int32_t offset, uint8_t *buf, int32_t len);
void peb_update_status(uint32_t partition, int32_t pnum, peb_status_t status);
int32_t peb_activing(uint32_t partition, int32_t pnum);
int32_t space_allocation(
    uint32_t partition,
    int32_t alloc_size,
    int32_t added_size,
    int32_t *poffset,
    const char *group_name,
    const char *item_name);
bool space_is_enough(uint32_t partition, int32_t size);
void space_sub_valid(uint32_t partition, int32_t size);
void peb_add_drity(uint32_t partition, int32_t pnum, int32_t drity);
void peb_add_free(uint32_t partition, int32_t pnum, int32_t free);
void peb_sub_free(uint32_t partition, int32_t pnum, int32_t free);
nvdm_status_t nvdm_query_data_item_length(const char *group_name, const char *data_item_name, uint32_t *size);
nvdm_status_t nvdm_query_data_item_count(const char *group_name, uint32_t *count);
nvdm_status_t nvdm_query_space_info(uint32_t *total_avail_space, uint32_t *curr_used_space);
nvdm_status_t nvdm_query_space_by_partition(uint32_t partition, uint32_t *total_avail_space, uint32_t *curr_used_space);
nvdm_status_t nvdm_query_data_item_type(const char *group_name, const char *data_item_name, nvdm_data_item_type_t *type);
nvdm_status_t nvdm_cancel_non_blocking_write(const char *group_name, const char *item_name);
uint32_t determine_which_partition(const char *group, const char *item,
                                   nvdm_data_item_type_t type,
                                   const uint8_t *buffer,
                                   uint32_t size, int32_t *idx, uint32_t check_parameter);

#endif

#endif


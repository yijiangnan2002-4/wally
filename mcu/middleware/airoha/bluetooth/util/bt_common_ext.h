/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */
/* MediaTek restricted information */

#ifndef __BT_COMMON_EXT_H__
#define __BT_COMMON_EXT_H__

#include "bt_linknode.h"
#include "bt_system.h"
#include "bt_config.h"
BT_EXTERN_C_BEGIN

#define BT_MEMORY_RESERVED_IMPORTANT_DATA_SIZE     0x64

typedef bt_status_t (*bt_module_free_handle_callback_t)(void *ptr);
typedef struct {
    bt_linknode_t  free_handle_list;
    bt_module_free_handle_callback_t callback;
} bt_module_free_handle_list_t;

void bt_module_handle_process(void);
void bt_module_add_to_free_list(bt_linknode_t *node);
void bt_module_delete_from_free_list(bt_linknode_t *node);

/*****************************************************************************
 * FUNCTION
 *  bt_store_little_endian_from_16
 * DESCRIPTION
 *  Store 16 bit value into a buffer in Little Endian format.
 * PARAMETERS
 *  buff            [OUT]
 *  le_value        [IN]
 * RETURNS
 *  void
 *****************************************************************************/
void bt_store_little_endian_from_16(uint8_t *buff, uint16_t le_value);
/*****************************************************************************
 * FUNCTION
 *  bt_store_little_endian_from_32
 * DESCRIPTION
 *  Store 32 bit value into a buffer in Little Endian format.
 * PARAMETERS
 *  buff            [OUT]
 *  le_value        [IN]
 * RETURNS
 *  void
 *****************************************************************************/
void bt_store_little_endian_from_32(uint8_t *buff, uint32_t le_value);
/*****************************************************************************
 * FUNCTION
 *  bt_retrieve_little_endian_to_16
 * DESCRIPTION
 *  Retrieve a 16-bit number from the given buffer. The number
 *  is in Little-Endian format.
 * PARAMETERS
 *  ptr     [IN]
 * RETURNS
 *  16-bit number.
 *****************************************************************************/
uint16_t bt_retrieve_little_endian_to_16(const uint8_t *ptr);
/*****************************************************************************
 * FUNCTION
 *  bt_retrieve_little_endian_to_32
 * DESCRIPTION
 *  Retrieve a 32-bit number from the given buffer. The number
 *  is in Little-Endian format.
 * PARAMETERS
 *  ptr     [IN]
 * RETURNS
 *  32-bit number.
 *****************************************************************************/
uint32_t bt_retrieve_little_endian_to_32(const uint8_t *ptr);

/*****************************************************************************
* FUNCTION
*  bt_endian_order_swap
* DESCRIPTION
*  Swap the endian order from src to dest. This can swap little-endian to big
*  -endian or in contrast. Len only support 2 or 4 btyes
* PARAMETERS
*  dest     [IN]
*  src      [IN]
*  len      [IN]
* RETURNS
*  bool to indiacte whether the len is supported.
*****************************************************************************/
bool bt_endian_order_swap(uint8_t *dest, const uint8_t *src, uint8_t len);

/**
 * @brief   Allocate packet of size bytes.
 * @param[in] type #BT_MEMORY_TX_BUFFER or #BT_MEMORY_RX_BUFFER.
 * @param[in] size Size of packet, in bytes.
 * @return  Return a pointer to the beginning of the packet, or NULL if out of memory.
 */
//char *bt_mm_allocate_packet(bt_memory_packet_t type, uint32_t size);
char *bt_mm_allocate_packet_internal(bt_memory_packet_t type, uint32_t size
#ifdef BT_DEBUG
                                     , const char *func_p, uint32_t line_p
#endif
                                    );
/**
 * @brief   Free packet previously allocated by bt_mm_allocate_packet().
 * @param[in] type #BT_MEMORY_TX_BUFFER or #BT_MEMORY_RX_BUFFER.
 * @param[in] ptr  A pointer, point to the beginning of the packet.
 *
 */
//void bt_mm_free_packet(bt_memory_packet_t type, char *ptr);
void bt_mm_free_packet_internal(bt_memory_packet_t type, char *ptr
#ifdef BT_DEBUG
                                , const char *func_p, uint32_t line_p
#endif
                               );

//API for connection and timer control block, that are fixed size
/**
 * @brief   Allocate specific control block(Timer/Connection).
 * @param[in] type #BT_MEMORY_CONTROL_BLOCK_TIMER or #BT_MEMORY_CONTROL_BLOCK_LE_CONNECTION.
 * @return  Return a pointer to the control block, or NULL if out of memory.
 */
//void *bt_mm_allocate_fixed_size_by_type(bt_memory_control_block_t type);
void *bt_mm_allocate_fixed_size_by_type_internal(bt_memory_control_block_t type
#ifdef BT_DEBUG
                                                 , const char *func_p, uint32_t line_p
#endif
                                                );
/**
 * @brief   Free control block previously allocated by bt_mm_allocate_fixed_size_by_type().
 * @param[in] type #BT_MEMORY_CONTROL_BLOCK_TIMER or #BT_MEMORY_CONTROL_BLOCK_LE_CONNECTION.
 * @param[in] ptr  A pointer, point to the control block.
 *
 */
//void bt_mm_free_fixed_size_by_type(bt_memory_control_block_t type, void *ptr);
void bt_mm_free_fixed_size_by_type_internal(bt_memory_control_block_t type, void *ptr
#ifdef BT_DEBUG
                                            , const char *func_p, uint32_t line_p
#endif
                                           );

#ifdef BT_DEBUG
#define bt_mm_allocate_packet(type, size)           bt_mm_allocate_packet_internal(type, size, __FUNCTION__, __LINE__)
#define bt_mm_free_packet(type, ptr)               bt_mm_free_packet_internal(type, ptr, __FUNCTION__, __LINE__)
#define bt_mm_allocate_fixed_size_by_type(type)     bt_mm_allocate_fixed_size_by_type_internal(type, __FUNCTION__, __LINE__)
#define bt_mm_free_fixed_size_by_type(type, ptr)    bt_mm_free_fixed_size_by_type_internal(type, ptr, __FUNCTION__, __LINE__)
#else
#define bt_mm_allocate_packet(type, size)           bt_mm_allocate_packet_internal(type, size)
#define bt_mm_free_packet(type, ptr)               bt_mm_free_packet_internal(type, ptr)
#define bt_mm_allocate_fixed_size_by_type(type)     bt_mm_allocate_fixed_size_by_type_internal(type)
#define bt_mm_free_fixed_size_by_type(type, ptr)    bt_mm_free_fixed_size_by_type_internal(type, ptr)
#endif

void bt_power_on_a2dp_callback();
void bt_power_on_avrcp_callback();
void bt_power_on_hfp_callback();
void bt_power_on_hsp_callback();
void bt_power_on_hid_callback();
void bt_power_on_spp_callback();
void bt_power_on_goep_callback();
void bt_power_on_pbapc_callback();
void bt_power_on_avrcp_br_callback();

#ifndef BT_TO_UPPERS
#define BT_TO_UPPERS(ch)  (((ch) >= 'a' && (ch) <= 'z') ? ((ch) - 'a' + 'A') : (ch))
#endif

BT_EXTERN_C_END

#endif /* __BT_COMMON_EXT_H__ */


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

#ifndef _FRAMESIZE_ADJUSTOR_INTERFACE_H_
#define _FRAMESIZE_ADJUSTOR_INTERFACE_H_

/* Includes ------------------------------------------------------------------*/
#include "dsp_utilities.h"
#include "dsp_buffer.h"
#include "dsp_feature_interface.h"
#include "dsp_callback.h"
#include "dsp_sdk.h"
#include <stdint.h>

/* Public define -------------------------------------------------------------*/
#define FRAMESIZE_ADJUSTOR_PORT_MAX            6
#define FRAMESIZE_ADJUSTOR_LIST_MAX            3

/* Public typedef ------------------------------------------------------------*/

typedef bool (*CusFunc_initialize)(void*, void*);
typedef void (*CusFunc_process)(uint8_t*, uint8_t*, uint32_t, void*);

typedef enum {
    FRAMESIZE_ADJUSTOR_STATUS_ERROR = -1,
    FRAMESIZE_ADJUSTOR_STATUS_OK = 0
} framesize_adjustor_status_t;

typedef enum {
    FRAMESIZE_ADJUSTOR_PORT_STATUS_DEINIT = 0,
    FRAMESIZE_ADJUSTOR_PORT_STATUS_INIT = 1,
    FRAMESIZE_ADJUSTOR_PORT_STATUS_RUNNING = 2
} framesize_adjustor_port_status_t;

typedef enum {
    FRAMESIZE_ADJUSTOR_MODE_FIXED_INOUT_LENGTH,
    FRAMESIZE_ADJUSTOR_MODE_MULTI_BUFFERS,
    FRAMESIZE_ADJUSTOR_MODE_CHANGE_INOUT_LENGTH,
    FRAMESIZE_ADJUSTOR_MODE_END
} framesize_adjustor_mode_t;

typedef enum {
    FRAMESIZE_ADJUSTOR_BUFFER_INPUT,
    FRAMESIZE_ADJUSTOR_BUFFER_OUTPUT,
    FRAMESIZE_ADJUSTOR_BUFFER_END,
} framesize_adjustor_buffer_type_t;

typedef struct {
    uint32_t buf_size;
    uint32_t data_size;
    uint8_t *buf;
    uint8_t *read_from;
    uint8_t *write_to;
    uint8_t* temp;
} framesize_adjustor_channel_buf_t;

typedef struct {
    framesize_adjustor_channel_buf_t in;
    framesize_adjustor_channel_buf_t out;
} framesize_adjustor_channel_t;

typedef enum {
    FRAMESIZE_ADJUSTOR_FUNCTION_DEINTERLEAVED,
    FRAMESIZE_ADJUSTOR_FUNCTION_INTERLEAVED,
    FRAMESIZE_ADJUSTOR_FUNCTION_END,
} framesize_adjustor_function_type_t;

typedef struct {
    framesize_adjustor_port_status_t status;
    void *owner;
    DSP_STREAMING_PARA_PTR stream;
    framesize_adjustor_mode_t mode;
    framesize_adjustor_function_type_t type;
    uint16_t watermark_max_size;
    uint16_t inout_size;
    uint16_t process_size;
    uint16_t total_channels;
    framesize_adjustor_channel_t *channel;
    uint16_t process_sample_resolution;
    bool trustInitParam;
    CusFunc_initialize fun_initialize;
    CusFunc_process fun_process;
    uint32_t MemSize;
    void* MemPtr;
} framesize_adjustor_port_t;

typedef struct {
    void *owner;
    uint16_t current_index;
    uint16_t buffer_total;
    framesize_adjustor_port_t **port_list;
} framesize_adjustor_list_t;

typedef struct {
    /* framesize adjustor mode: only support FRAMESIZE_ADJUSTOR_MODE_FIXED_INOUT_LENGTH at now. */
    framesize_adjustor_mode_t mode;
    /* framesize adjustor type: support INTERLEAVED/ DEINTERLEAVED function. */
    framesize_adjustor_function_type_t type;
    /* The total channel numbers of the framesize adjustor. For example if the data is stereo, the total_channels should be 2. */
    uint16_t total_channels;
    /* The input/ output sample number of each channel.
        For example if the input/ output sample number is 120, it means the framesize adjustor will input/ output 120B data in every stream process */
    uint16_t inout_sample_num;
    /* The processing sample number for alg.*/
    uint16_t process_sample_num;
    /* The process_sample_resolution each channel.
        For example if the process_sample_resolution is RESOLUTION_32BIT, it means the process_sample_resolution is 32bit */
    uint16_t process_sample_resolution;
    bool trustInitParam;
} framesize_adjustor_config_t;

/* Public macro --------------------------------------------------------------*/
/* Public variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
/**
 * @brief This function is used to get the framesize adjustor port or query the owner's framesize adjustor port.
 *        If the owner does not have a framesize adjustor port, it will malloc a port for this owner.
 *        If the owner have a framesize adjustor port, it will return the port directly.
 *        If the owner is NULL, it will return the first unused port.
 *
 * @param owner is who want to get or query a framesize adjustor port or NULL.
 * @return framesize_adjustor_port_t* is the result.
 */
extern framesize_adjustor_port_t *stream_function_framesize_adjustor_get_port(void *owner);

/**
 * @brief This function is used to get the framesize adjustor port.
 *        If the owner does not have a framesize adjustor port, it will malloc a port for this owner.
 *        If the owner have a framesize adjustor port, it also will malloc a port for this owner.
 *        If the owner is NULL, it will return the first unused port.
 *
 * @param owner is who want to get a framesize adjustor port or NULL.
 * @return framesize_adjustor_port_t* is the result.
 */
extern framesize_adjustor_port_t *stream_function_framesize_adjustor_get_unused_port(void *owner);

/**
 * @brief This function is used to initialize the framesize adjustor port.
 *
 * @param port is the port which needs to be initialized.
 * @param config is the default configurations of the framesize adjustor port.
 * @param MemSize is the size of working buffer which fun_process needed.
 * @param fun_initialize is the customer's initialize function.
 * @param fun_process is the customer's processing function.
 * @return framesize_adjustor_status_t is operation result. FRAMESIZE_ADJUSTOR_STATUS_ERROR means there is a error. FRAMESIZE_ADJUSTOR_STATUS_OK means operation is done successfully.
 */
extern framesize_adjustor_status_t stream_function_framesize_adjustor_init(framesize_adjustor_port_t *port, framesize_adjustor_config_t *config, uint32_t MemSize, CusFunc_initialize fun_initialize, CusFunc_process fun_process);

/**
 * @brief This function is used to calculate the greatest common divisor.
 *
 * @param m and n are both numbers.
 * @return greatest common divisor between m and n.
 */
extern uint16_t stream_function_framesize_adjustor_gcd(uint16_t m, uint16_t n);

/**
 * @brief This function is used to calculate the prefill sample number.
 *
 * @param in_sample_num is the number of input sample.
 * @param process_sample_num is the number of pcocessing sample.
 * @return a number of prefill sample.
 */
extern uint16_t stream_function_framesize_adjustor_get_prefill_sample(uint16_t in_sample_num, uint16_t process_sample_num);

/**
 * @brief This function is used to de-initialize the framesize adjustor port.
 *
 * @param port is the port which needs to be de-initialized.
 * @return framesize_adjustor_status_t is operation result. FRAMESIZE_ADJUSTOR_STATUS_ERROR means there is a error. FRAMESIZE_ADJUSTOR_STATUS_OK means operation is done successfully.
 */
extern framesize_adjustor_status_t stream_function_framesize_adjustor_deinit(framesize_adjustor_port_t *port);

/**
 * @brief This function is used to query the channel's used space size of the framesize adjustor port.
 *
 * @param port is the port which needs to be queried.
 * @param channel_number is the channel whose used space size needs to be queried.
 * @return uint32_t is the used space size.
 */
extern uint32_t stream_function_framesize_adjustor_get_channel_used_size(framesize_adjustor_port_t *port, uint16_t channel_number, framesize_adjustor_buffer_type_t buffer_type);

/**
 * @brief This function is used to query the channel's free space size of the framesize adjustor port.
 *
 * @param port is the port which needs to be queried.
 * @param channel_number is the channel whose free space size needs to be queried.
 * @return uint32_t is the free space size.
 */
extern uint32_t stream_function_framesize_adjustor_get_channel_free_size(framesize_adjustor_port_t *port, uint16_t channel_number, framesize_adjustor_buffer_type_t buffer_type);

/**
 * @brief This function is used to get the current read pointer of the framesize adjustor port.
 *
 * @param port is the port which needs to be queried.
 * @param channel_number is the channel whose read pointer to be queried.
 * @return uint8_t* is pointer to the current read pointer.
 */
extern uint8_t *stream_function_framesize_adjustor_get_channel_read_pointer(framesize_adjustor_port_t *port, uint16_t channel_number, framesize_adjustor_buffer_type_t buffer_type);

/**
 * @brief This function is used to get the current write pointer of the framesize adjustor port.
 *
 * @param port is the port which needs to be queried.
 * @param channel_number is the channel whose write pointer to be queried.
 * @return uint8_t* is pointer to the current write pointer.
 */
extern uint8_t *stream_function_framesize_adjustor_get_channel_write_pointer(framesize_adjustor_port_t *port, uint16_t channel_number, framesize_adjustor_buffer_type_t buffer_type);

/**
 * @brief This function is used to get the start address pointer of the framesize adjustor port.
 *
 * @param port is the port which needs to be queried.
 * @param channel_number is the channel whose the start address pointer to be queried.
 * @return uint8_t* is pointer to the start address pointer.
 */
extern uint8_t *stream_function_framesize_adjustor_get_channel_start_pointer(framesize_adjustor_port_t *port, uint16_t channel_number, framesize_adjustor_buffer_type_t buffer_type);

/**
 * @brief This function is used to get the end address pointer of the framesize adjustor port.
 *
 * @param port is the port which needs to be queried.
 * @param channel_number is the channel whose the end address pointer to be queried.
 * @return uint8_t* is pointer to the end address pointer.
 */
extern uint8_t *stream_function_framesize_adjustor_get_channel_end_pointer(framesize_adjustor_port_t *port, uint16_t channel_number, framesize_adjustor_buffer_type_t buffer_type);

/**
 * @brief This function is used to configurate the channel's prefill size of the framesize adjustor port.
 *
 * @param port is the port which needs to be configurated.
 * @param channel_number is the channel whose prefill size needs to be configurated.
 * @param perfill_size is the prefill size value.
 * @return framesize_adjustor_status_t is operation result. FRAMESIZE_ADJUSTOR_STATUS_ERROR means there is a error. FRAMESIZE_ADJUSTOR_STATUS_OK means operation is done successfully.
 */
extern framesize_adjustor_status_t stream_function_framesize_adjustor_config_channel_prefill_size(framesize_adjustor_port_t *port, uint16_t channel_number, uint16_t perfill_size, bool set_zeros, framesize_adjustor_buffer_type_t buffer_type);

/**
 * @brief This function is used to reset the channel's buffer status of the framesize adjustor port.
 *
 * @param port is the port which needs to be reset.
 * @param channel_number is the channel whose buffer status is reset.
 * @param set_zeros is the option which if needs to zero the framesize adjustor. If it is true, the data in the framesize adjustor will be set 0 after the buffer is reseted.
 * @return framesize_adjustor_status_t is operation result. FRAMESIZE_ADJUSTOR_STATUS_ERROR means there is a error. FRAMESIZE_ADJUSTOR_STATUS_OK means operation is done successfully.
 */
extern framesize_adjustor_status_t stream_function_framesize_adjustor_reset_channel_buffer(framesize_adjustor_port_t *port, uint16_t channel_number, bool set_zeros, framesize_adjustor_buffer_type_t buffer_type);

/**
 * @brief This function is used to get the multi-buffers list.
 *        If the owner does not have a multi-buffers list, it will malloc a list for this owner.
 *        If the owner have a multi-buffers list, it will return the list directly.
 *        If the owner is NULL, it will return the first unused multi-buffers list.
 *
 * @param owner is who want to get or query a multi-buffe list or NULL.
 * @return framesize_adjustor_list_t* is the result.
 */
extern framesize_adjustor_list_t *stream_function_framesize_adjustor_get_list(void *owner);

/**
 * @brief This function is used to initialize the multi-buffers list.
 *
 * @param list is who needs to be initialized.
 * @param buffer_total_number means how much framesize adjustors will be inserted into the multi-buffers list.
 * @return framesize_adjustor_status_t is operation result. FRAMESIZE_ADJUSTOR_STATUS_ERROR means there is a error. FRAMESIZE_ADJUSTOR_STATUS_OK means operation is done successfully.
 */
extern framesize_adjustor_status_t stream_function_framesize_adjustor_list_init(framesize_adjustor_list_t *list, uint16_t buffer_total_number);

/**
 * @brief This function is used to de-initialize the multi-buffers list.
 *
 * @param list is who needs to be de-initialized.
 * @return framesize_adjustor_status_t is operation result. FRAMESIZE_ADJUSTOR_STATUS_ERROR means there is a error. FRAMESIZE_ADJUSTOR_STATUS_OK means operation is done successfully.
 */
extern framesize_adjustor_status_t stream_function_framesize_adjustor_list_deinit(framesize_adjustor_list_t *list);


/**
 * @brief This function is used to insert a specail framesize adjustor into the multi-buffers list.
 *
 * @param list is the multi-buffers list who will be insert by a specail framesize adjustor.
 * @param port is the framesize adjustor port who will be insert into a multi-buffers list.
 * @param buffer_index is the index in the multi-buffers list. For example, buffer_index is 0 means the specail framesize adjustor will be the first buffer item in the multi-buffers list.
 * @return framesize_adjustor_status_t is operation result. FRAMESIZE_ADJUSTOR_STATUS_ERROR means there is a error. FRAMESIZE_ADJUSTOR_STATUS_OK means operation is done successfully.
 */
extern framesize_adjustor_status_t stream_function_framesize_adjustor_list_insert_buffer(framesize_adjustor_list_t *list, framesize_adjustor_port_t *port, uint16_t buffer_index);

/**
 * @brief  This function is used to initialize the framesize adjustor run-time environment in stream flow.
 *
 * @param para is the input parameters.
 * @return true means there is a error.
 * @return false means there is no error.
 */
extern bool stream_function_framesize_adjustor_initialize(void *para);

/**
 * @brief This function is used to do the framesize adjustor process in stream flow.
 *
 * @param para is the input parameters.
 * @return true means there is a error.
 * @return false means there is no error.
 */
extern bool stream_function_framesize_adjustor_process(void *para);

/**
 * @brief These function are used to do the framesize adjustor customer's functions in stream flow.
 *
 * @param in_buf is the input buffer address.
 * @param in_buf is the output buffer address.
 * @param size is the input/ output buffer size.
 */
extern void framesize_adjustor_sample_Fun1_process(uint8_t* in_buf, uint8_t* out_buf, uint32_t size, void* working_buffer_ptr);
extern void framesize_adjustor_sample_Fun2_process(uint8_t* in_buf, uint8_t* out_buf, uint32_t size, void* working_buffer_ptr);
extern bool framesize_adjustor_sample_Fun1_initialize(void* para, void* working_buf);
extern bool framesize_adjustor_sample_Fun2_initialize(void* para, void* working_buf);

#endif /* _FRAMESIZE_ADJUSTOR_INTERFACE_H_ */

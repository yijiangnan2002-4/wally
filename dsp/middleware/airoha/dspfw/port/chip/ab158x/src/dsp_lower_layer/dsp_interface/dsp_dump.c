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

#include "types.h"
#include "syslog.h"
#include "dsp_dump.h"
#include "audio_nvdm_common.h"
#include "string.h"
#include "dsp_temp.h"
#include "hal_nvic.h"
#include "hal_gpt.h"

//#define AIR_AUDIO_DUMP_BY_SPDIF_ENABLE
//#define MTK_AUDIO_DUMP_BY_SPDIF_DEBUG_ENABLE

/******************************************************************************
* Constant Definitions
******************************************************************************/
#define MAX_DUMP_SIZE 2046
#define DUMP_ID_SIZE  2
#define ACCUM_BYTES_SIZE    4
#define DUMP_BY_TUNING_TOOL 0

#define AUDIO_DUMP_DEVICE_NONE         0
#define AUDIO_DUMP_DEVICE_MUX          2
#define AUDIO_DUMP_DEVICE_SPDIF        3
#define AUDIO_DUMP_DEVICE_I2S_MASTER   4
#define AUDIO_DUMP_DEVICE_I2S_SLAVE    5


/******************************************************************************
 * Variables
 ******************************************************************************/
#ifdef AIR_AUDIO_DUMP_BY_SPDIF_ENABLE
/*
 * 10ms: (96000 * 2 * 24) / (8 * 100) = 5760
 * MEMIF buffer requirement: 384 * 3 * 2 = 2304
 */
#define SDPIF_IRQ_COUNTER 384 /* 192 for each 2ms */
U8 g_audio_dump_buffer[16 * 1024 + 32];
#endif

DumpIDs_AccumBytes DumpIDsAccumBytes[AUDIO_DUMP_CONFIG_MAX_NUM] = {0};
U16  AudioDumpCfgIDs[AUDIO_DUMP_CONFIG_MAX_NUM] = {0};
U32  AudioDumpDevice; /**< @Value 0x00000002 @Desc 0:Disable, 1:LOGGING, 2:MUX, 3:SPDIF, 4:I2S_MASTER, 5:I2S_SLAVE, */
U32  DumpIdCfgVersion = 0;
static bool block_dump_flag = false;

/******************************************************************************
 * Function Declaration
 ******************************************************************************/
void LOG_AUDIO_DUMP(U8 *audio, U32 audio_size, U32 dumpID);


/******************************************************************************
 * Functions
 ******************************************************************************/
log_create_module(audio_module, PRINT_LEVEL_INFO);

bool check_cfg_dump_id(DSP_DATADUMP_MASK_BIT dumpID)
{
    U32 i;
    for(i = 0; i < AUDIO_DUMP_CONFIG_MAX_NUM; i++){
        if(dumpID == AudioDumpCfgIDs[i]){
            return true;
        }
    }
    return false;
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void add_cfg_dump_id(DSP_DATADUMP_MASK_BIT dumpID)
{
    U32 i;
    bool add_id_result = false;
    bool add_accute_bytes_result = false;
    U32 mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    for(i = 0; i < AUDIO_DUMP_CONFIG_MAX_NUM; i++){
        if(AudioDumpCfgIDs[i] == 0 && add_id_result == false){
            AudioDumpCfgIDs[i] = dumpID;
            add_id_result = true;
        }
        if(DumpIDsAccumBytes[i].dump_id == 0 && add_accute_bytes_result == false){
            DumpIDsAccumBytes[i].dump_id = dumpID;
            DumpIDsAccumBytes[i].dump_accum_bytes = 0;
            add_accute_bytes_result = true;
        }
    }
    hal_nvic_restore_interrupt_mask(mask);
    if(add_id_result == false || add_accute_bytes_result == false){
        DSP_MW_LOG_E("[Audio Dump] cfg dump id fail, exceeds the MAX cfg number:%d", 1, AUDIO_DUMP_CONFIG_MAX_NUM);
    }
}

ATTR_TEXT_IN_RAM_FOR_MASK_IRQ void remove_cfg_dump_id(DSP_DATADUMP_MASK_BIT dumpID)
{
    U32 i;
    U32 mask;
    hal_nvic_save_and_set_interrupt_mask(&mask);
    for(i = 0; i < AUDIO_DUMP_CONFIG_MAX_NUM; i++){
        if(AudioDumpCfgIDs[i] == dumpID){
            AudioDumpCfgIDs[i] = 0;
        }
        if(DumpIDsAccumBytes[i].dump_id == dumpID){
            DumpIDsAccumBytes[i].dump_id = 0;
            DumpIDsAccumBytes[i].dump_accum_bytes = 0;
        }
    }
    hal_nvic_restore_interrupt_mask(mask);
}

static U32 get_accum_bytes(DSP_DATADUMP_MASK_BIT dumpID)
{
    U32 i;
    for(i = 0; i < AUDIO_DUMP_CONFIG_MAX_NUM; i++){
        if(DumpIDsAccumBytes[i].dump_id == dumpID){
            return DumpIDsAccumBytes[i].dump_accum_bytes;
        }
    }
    DSP_MW_LOG_E("[Audio Dump] get accum bytes fail, dump id:%d", 1, dumpID);
    return 0;
}

static void increase_accum_bytes(DSP_DATADUMP_MASK_BIT dumpID, U32 bytes)
{
    U32 i;
    for(i = 0; i < AUDIO_DUMP_CONFIG_MAX_NUM; i++){
        if(DumpIDsAccumBytes[i].dump_id == dumpID){
            DumpIDsAccumBytes[i].dump_accum_bytes += bytes;
            return;
        }
        else if(DumpIDsAccumBytes[i].dump_id == 0){ //if not found,add to array for test
            DumpIDsAccumBytes[i].dump_id = dumpID;
            DumpIDsAccumBytes[i].dump_accum_bytes += bytes;
            return;
        }
    }
    DSP_MW_LOG_E("[Audio Dump] dump accum bytes increase fail, Not Found id:%d", 1, dumpID);
}

#ifdef AIR_AUDIO_DUMP_BY_SPDIF_ENABLE

#include "FreeRTOS.h"
#include "semphr.h"
#include "dtm.h"
#include "hal_audio.h"
#include "dsp_buffer.h"
#include "common.h"

typedef struct{
    U8  pkt_id;
    U8  type;
    U16 length;
    U16 race_id;
    U16 dump_id;
    U32 accum_bytes;
    U32 _reserved;
} DSP_DATADUMP_HEADER, *DSP_DATADUMP_HEADER_PTR;

typedef struct
{
    U32 BufSize;

    hal_audio_memory_parameter_t memory;
    hal_audio_device_parameter_t device;
    hal_audio_path_parameter_t   path;
    U8  TempBuf[8];
    U8* BufPtr;
    U16 BufCnt;
    U16 BufWo;
    U16 BufRo;

    U16 _reserved;

    DSP_DATADUMP_HEADER header;
    QueueHandle_t DataDump_Semaphore;
} DSP_DATADUMP_CTRL_STRU, *DSP_DATADUMP_CTRL_STRU_ptr;

#define DSP_DUMP_HEADER_STRUCT_SIZE         (sizeof(DSP_DATADUMP_HEADER))
#define DSP_DATADUMP_SYNC_WORD      (0x5A)
#define DSP_DATADUMP_MASK_ID_TYPE   (0xFE)

static DSP_DATADUMP_CTRL_STRU DSP_DumpData_Ctrl;

#ifdef MTK_AUDIO_DUMP_BY_SPDIF_DEBUG_ENABLE
static uint8_t g_test_buffer[512];
#endif

VOID DSP_DataDump_UpdateBufferWriteOffset(VOID *sizePtr)
{
    U16 *size;
    size = (U16 *)sizePtr;
    DSP_DumpData_Ctrl.BufWo  = (DSP_DumpData_Ctrl.BufWo + *size) % DSP_DumpData_Ctrl.BufSize;
    DSP_DumpData_Ctrl.BufCnt += *size;
}

VOID DSP_DataDump_UpdateBufferReadOffset(VOID *sizePtr)
{
    U16 *size;
    size = (U16 *)sizePtr;
    DSP_DumpData_Ctrl.BufRo  = (DSP_DumpData_Ctrl.BufRo + *size) % DSP_DumpData_Ctrl.BufSize;
    DSP_DumpData_Ctrl.BufCnt -= *size;
}

static void DSP_DataDump_DataOut(void)
{
    uint32_t dl_read_pointer, frame_size, write_offset, readSize;
    int32_t out_remain;
    uint32_t out_ptr, save_out_ptr;
    hal_audio_get_value_parameter_t get_value_parameter;
    DSP_DATADUMP_HEADER outHeader;

    if ((xSemaphoreTake(DSP_DumpData_Ctrl.DataDump_Semaphore, portMAX_DELAY)) != pdTRUE) {
        assert(0);
        return;
    }
    get_value_parameter.get_current_offset.memory_select = HAL_AUDIO_MEMORY_DL_DL12;
    get_value_parameter.get_current_offset.pure_agent_with_src = false;
    dl_read_pointer = hal_audio_get_value(&get_value_parameter, HAL_AUDIO_GET_MEMORY_OUTPUT_CURRENT_OFFSET);

    frame_size = DSP_DumpData_Ctrl.memory.irq_counter * sizeof(uint32_t) * 2;// Interleaved buffer

    write_offset = ((1 + ((dl_read_pointer - (U32)DSP_DumpData_Ctrl.memory.buffer_addr) / frame_size)) * frame_size) % DSP_DumpData_Ctrl.memory.buffer_length;
    out_remain = frame_size;

    out_ptr = (DSP_DumpData_Ctrl.memory.buffer_addr + write_offset);
    save_out_ptr = out_ptr;
    if ((write_offset + frame_size) <= DSP_DumpData_Ctrl.memory.buffer_length) {
        memset((VOID *)out_ptr, 0, frame_size);
    } else {
        memset((VOID *)out_ptr, 0, DSP_DumpData_Ctrl.memory.buffer_length - write_offset);
        memset((VOID *)(DSP_DumpData_Ctrl.memory.buffer_addr), 0, frame_size - (DSP_DumpData_Ctrl.memory.buffer_length - write_offset));
    }

    out_remain = frame_size;
    while ((out_remain > 0 ) && (DSP_DumpData_Ctrl.BufCnt != 0)) {
        if((out_remain == (int32_t)frame_size) || (DSP_DumpData_Ctrl.header.length==0)) {

            if (out_remain <= (int32_t)DSP_DUMP_HEADER_STRUCT_SIZE)
            {
                break;
            }
            if (DSP_DumpData_Ctrl.header.length == 0)
            {
                readSize = DSP_DUMP_HEADER_STRUCT_SIZE;
                DSP_C2D_BufferCopy((VOID*)&DSP_DumpData_Ctrl.header,
                                   (VOID*)((U8*)DSP_DumpData_Ctrl.BufPtr + DSP_DumpData_Ctrl.BufRo),
                                   readSize,
                                   (VOID *)DSP_DumpData_Ctrl.BufPtr,
                                   DSP_DumpData_Ctrl.BufSize);

#ifdef MTK_AUDIO_DUMP_BY_SPDIF_DEBUG_ENABLE
                DSP_MW_LOG_I("[SPDIF_DUMP][read] detect new package: BufWo = 0x%08x, BufRo = 0x%08x, store = 0x%08x", 33,
                                DSP_DumpData_Ctrl.BufWo, DSP_DumpData_Ctrl.BufRo,
                                &DSP_DumpData_Ctrl.header);
#endif

                DSP_DataDump_UpdateBufferReadOffset((VOID *)&readSize);
            }
            outHeader.pkt_id      = 0x05;
            outHeader.type        = 0x5D;
			outHeader.race_id     = 0x0F34;
#ifdef MTK_AUDIO_DUMP_SPDIF_TRUNCATE_24BIT
            outHeader.length = MIN(out_remain - DSP_DUMP_HEADER_STRUCT_SIZE, DSP_DumpData_Ctrl.header.length);
            outHeader.length = outHeader.length * 3 / 4;
#else
            outHeader.length      = MIN((((out_remain-DSP_DUMP_HEADER_STRUCT_SIZE)*3)>>2), DSP_DumpData_Ctrl.header.length)+8; // 8 = 2bytes race id + 2bytes dump id + 4bytes accum_bytes
#endif

            outHeader.dump_id     = DSP_DumpData_Ctrl.header.dump_id;
            outHeader.accum_bytes = DSP_DumpData_Ctrl.header.accum_bytes;

            DSP_DATADUMP_HEADER_PTR tempPtr = &outHeader;
            //32bit -> 24bit copy
            memcpy((VOID*)(out_ptr + 1),  (VOID*)(tempPtr  ), 3);
            memcpy((VOID*)(out_ptr + 5),  ((VOID*)tempPtr)+3, 3);
            memcpy((VOID*)(out_ptr + 9),  ((VOID*)tempPtr)+6, 3);
            memcpy((VOID*)(out_ptr + 13), ((VOID*)tempPtr)+9, 3);

            out_remain   -= DSP_DUMP_HEADER_STRUCT_SIZE;
            write_offset += DSP_DUMP_HEADER_STRUCT_SIZE;

#ifdef MTK_AUDIO_DUMP_BY_SPDIF_DEBUG_ENABLE
            DSP_MW_LOG_I("[SPDIF_DUMP][read] assembly new package: write_offset = 0x%08x, BufWo = 0x%08x, BufRo = 0x%08x, field.length=%d, store_length=%d,field.accum_bytes=%d,", 6,
                            write_offset, DSP_DumpData_Ctrl.BufWo, DSP_DumpData_Ctrl.BufRo,
                            outHeader.length, DSP_DumpData_Ctrl.header.length,outHeader.accum_bytes);
            //printf("[SPDIF_DUMP][read] assembly out_length=%d,out_race_id=0x%X,out_dump_id=%d,out_accum=%d",outtemp->length,outtemp->race_id,outtemp->dump_id,outtemp->accum_bytes);
#endif
        } else {

#ifdef MTK_AUDIO_DUMP_SPDIF_TRUNCATE_24BIT
            readSize = MIN(DSP_DumpData_Ctrl.header.length, 8);
            memset(DSP_DumpData_Ctrl.TempBuf, 0, 8);
            DSP_C2D_BufferCopy((VOID *)DSP_DumpData_Ctrl.TempBuf,
                               (VOID *)((U8 *)DSP_DumpData_Ctrl.BufPtr + DSP_DumpData_Ctrl.BufRo),
                               readSize,
                               (VOID *)DSP_DumpData_Ctrl.BufPtr,
                               DSP_DumpData_Ctrl.BufSize);

            memcpy((VOID *)(out_ptr), (VOID *)&DSP_DumpData_Ctrl.TempBuf[0], readSize);
            DSP_DumpData_Ctrl.header.length -= readSize;
            DSP_DataDump_UpdateBufferReadOffset((VOID *)&readSize);
#else
            readSize = MIN(DSP_DumpData_Ctrl.header.length, 6);
            memset(DSP_DumpData_Ctrl.TempBuf, 0, 6);
            DSP_C2D_BufferCopy((VOID *)DSP_DumpData_Ctrl.TempBuf,
                               (VOID *)((U8 *)DSP_DumpData_Ctrl.BufPtr + DSP_DumpData_Ctrl.BufRo),
                               readSize,
                               (VOID *)DSP_DumpData_Ctrl.BufPtr,
                               DSP_DumpData_Ctrl.BufSize);

            //32bit -> 24bit copy
            memcpy((VOID*)(out_ptr + 1), (VOID*)&DSP_DumpData_Ctrl.TempBuf[0], 3);
            memcpy((VOID*)(out_ptr + 5), (VOID*)&DSP_DumpData_Ctrl.TempBuf[3], 3);

            DSP_DumpData_Ctrl.header.length -= readSize;
            DSP_DataDump_UpdateBufferReadOffset((VOID*)&readSize);
#endif
            out_remain   -= 8;
            write_offset += 8;

        }
        write_offset %= DSP_DumpData_Ctrl.memory.buffer_length;
        out_ptr = (DSP_DumpData_Ctrl.memory.buffer_addr + write_offset);
    }

    xSemaphoreGive(DSP_DumpData_Ctrl.DataDump_Semaphore);
}





/**
 * @brief Data in for dump.
 *
 * @param inBuf         Pointer to the source of data to be dump.
 * @param dataLength    Pointer to the source of data to be dump.
 * @param dumpID        dump ID.
 */
static U32 DSP_DataDump_DataIn(U8 *inBuf, U16 dataLength, DSP_DATADUMP_MASK_BIT dumpID)
{
    U16 i;
    U32 irq_mask;
    U16 storeSize = 0, readSize = 0, remainSize;
    DSP_DATADUMP_HEADER_PTR inBufPtr;
    DSP_DATADUMP_HEADER inTempBuf = {0};

    if (inBuf == NULL){
        return 0;
    }
    if ((xSemaphoreTake(DSP_DumpData_Ctrl.DataDump_Semaphore, portMAX_DELAY)) != pdTRUE) {
        assert(0);
        return -1;
    }

    for (i=0 ; i<dataLength ; i+=readSize)
    {
        remainSize
            = (DSP_DumpData_Ctrl.BufSize - DSP_DumpData_Ctrl.BufCnt > DSP_DUMP_HEADER_STRUCT_SIZE)
              ? (DSP_DumpData_Ctrl.BufSize - DSP_DumpData_Ctrl.BufCnt - DSP_DUMP_HEADER_STRUCT_SIZE)
              : 0;

        if ((DSP_DumpData_Ctrl.BufSize-DSP_DumpData_Ctrl.BufCnt) > DSP_DUMP_HEADER_STRUCT_SIZE)
        {
            readSize = MIN(remainSize, dataLength-i);
        }
        else
        {
            xSemaphoreGive(DSP_DumpData_Ctrl.DataDump_Semaphore);
            return i;
        }

        inBufPtr = &inTempBuf;
        increase_accum_bytes(dumpID,readSize);
        U32 accum_bytes = get_accum_bytes(dumpID);
        inBufPtr->length      = readSize;
        inBufPtr->dump_id     = dumpID;
        inBufPtr->accum_bytes = accum_bytes;


        DSP_D2C_BufferCopy((U8 *)((U32)DSP_DumpData_Ctrl.BufPtr + DSP_DumpData_Ctrl.BufWo),
                           inBufPtr,
                           DSP_DUMP_HEADER_STRUCT_SIZE,
                           DSP_DumpData_Ctrl.BufPtr,
                           (U16)DSP_DumpData_Ctrl.BufSize);

        DSP_D2C_BufferCopy((U8 *)((U32)DSP_DumpData_Ctrl.BufPtr + ((DSP_DumpData_Ctrl.BufWo + DSP_DUMP_HEADER_STRUCT_SIZE) % DSP_DumpData_Ctrl.BufSize)),
                           inBuf,
                           readSize,
                           DSP_DumpData_Ctrl.BufPtr,
                           (U16)DSP_DumpData_Ctrl.BufSize);

#ifdef MTK_AUDIO_DUMP_BY_SPDIF_DEBUG_ENABLE
        DSP_MW_LOG_I("[SPDIF_DUMP][write] dumpID = %d, BufWo = 0x%08x, BufRo = 0x%08x, WriteSize = %d,Accum_bytes=%d", 5, dumpID, DSP_DumpData_Ctrl.BufWo, DSP_DumpData_Ctrl.BufRo, readSize,accum_bytes);
#endif

        inBuf += readSize;

        storeSize = readSize + DSP_DUMP_HEADER_STRUCT_SIZE;

        hal_nvic_save_and_set_interrupt_mask(&irq_mask);
        DSP_DataDump_UpdateBufferWriteOffset(&storeSize);
        hal_nvic_restore_interrupt_mask(irq_mask);

    }

    xSemaphoreGive(DSP_DumpData_Ctrl.DataDump_Semaphore);

    //DSP_MW_LOG_I("[SPDIF_DUMP] data in end", 0);

    return dataLength;
}


void audio_dump_task_handler(uint32_t arg)
{
    arg = arg;

    DSP_DataDump_DataOut();
}

static void spdif_dump_isr_handler(void)
{
    /* wake up DTM task to do MEMIF buffer fill */
    DTM_enqueue(DTM_EVENT_ID_AUDIO_DUMP, 0, true);
}

static void spdif_dump_init(void)
{
    #if 0 //for local test
    hal_gpio_init(25);
    hal_pinmux_set_function(25, 7);
    #endif
    hal_audio_device_parameter_t *device_handle = &(DSP_DumpData_Ctrl.device);
    hal_audio_path_parameter_t *path_handle = &(DSP_DumpData_Ctrl.path);
    hal_audio_memory_parameter_t *memory_handle = &(DSP_DumpData_Ctrl.memory);
    hal_audio_irq_parameter_t   register_irq_handler;

    memset(device_handle, 0, sizeof(hal_audio_device_parameter_t));
    memset(path_handle, 0, sizeof(hal_audio_path_parameter_t));
    memset(memory_handle, 0, sizeof(hal_audio_memory_parameter_t));

    //Set device parameters
    device_handle->spdif.i2s_setting.audio_device           = HAL_AUDIO_CONTROL_DEVICE_SPDIF;
    device_handle->spdif.i2s_setting.rate                   = 96000;
    device_handle->spdif.i2s_setting.i2s_interface          = HAL_AUDIO_INTERFACE_1;
    device_handle->spdif.i2s_setting.i2s_format             = HAL_AUDIO_I2S_I2S;
    device_handle->spdif.i2s_setting.word_length            = HAL_AUDIO_I2S_WORD_LENGTH_32BIT;
    device_handle->spdif.i2s_setting.mclk_divider           = 2;
    device_handle->spdif.i2s_setting.with_mclk              = false;
    device_handle->spdif.i2s_setting.is_low_jitter          = true; //Need to open PLL
    device_handle->spdif.i2s_setting.is_rx_swap             = false;
    device_handle->spdif.i2s_setting.is_tx_swap             = false;
    device_handle->spdif.i2s_setting.is_internal_loopback   = false;
    device_handle->spdif.i2s_setting.is_recombinant         = false;

    //Set memory parameters
    memory_handle->memory_select            = HAL_AUDIO_MEMORY_DL_DL12;
    memory_handle->pcm_format               = HAL_AUDIO_PCM_FORMAT_S32_LE;
    memory_handle->sync_status              = HAL_AUDIO_MEMORY_SYNC_NONE;
    memory_handle->audio_path_rate          = device_handle->spdif.i2s_setting.rate;
    memory_handle->buffer_addr              = 0;
    memory_handle->irq_counter              = SDPIF_IRQ_COUNTER; //Units:sample
    memory_handle->buffer_length            = memory_handle->irq_counter * sizeof(uint32_t) * 2 * 2; /* store 2 frame */
    memory_handle->initial_buffer_offset    = 32;
    memory_handle->with_mono_channel        = false;
    memory_handle->pure_agent_with_src      = false;
    memory_handle->src_tracking_clock_source = HAL_AUDIO_SRC_TRACKING_DISABLE;

    //Set path parameters
    path_handle->input.interconn_sequence[0]    = HAL_AUDIO_INTERCONN_SELECT_INPUT_MEMORY_DL12_CH1;
    path_handle->input.interconn_sequence[1]    = HAL_AUDIO_INTERCONN_SELECT_INPUT_MEMORY_DL12_CH2;
    path_handle->output.interconn_sequence[0]   = HAL_AUDIO_INTERCONN_SELECT_OUTPUT_DEVICE_MASTER_I2S0_CH1; //Todo : Convert audio device
    path_handle->output.interconn_sequence[1]   = HAL_AUDIO_INTERCONN_SELECT_OUTPUT_DEVICE_MASTER_I2S0_CH2; //Todo : Convert audio device
    path_handle->connection_number              = 2;
    path_handle->audio_input_rate[0]          = memory_handle->audio_path_rate;
    path_handle->audio_input_rate[1]          = memory_handle->audio_path_rate;
    path_handle->audio_output_rate[0]         = device_handle->spdif.i2s_setting.rate;
    path_handle->audio_output_rate[1]          = device_handle->spdif.i2s_setting.rate;
    path_handle->with_hw_gain                   = false;
    path_handle->with_updown_sampler[0]        = false;
    path_handle->with_updown_sampler[1]        = false;
    path_handle->with_dl_deq_mixer              = false;

    //Register ISR handler
    register_irq_handler.audio_irq = HAL_AUDIO_IRQ_AUDIOSYS;
    register_irq_handler.memory_select = memory_handle->memory_select;
    register_irq_handler.entry = spdif_dump_isr_handler; //ISR entry
    hal_audio_set_value((hal_audio_set_value_parameter_t *)&register_irq_handler, HAL_AUDIO_SET_IRQ_HANDLER);

    //Enable
    hal_audio_set_device(device_handle, device_handle->common.audio_device, HAL_AUDIO_CONTROL_ON);
    hal_audio_set_path(path_handle, HAL_AUDIO_CONTROL_ON);
    hal_audio_set_memory(memory_handle, HAL_AUDIO_CONTROL_MEMORY_INTERFACE, HAL_AUDIO_CONTROL_ON);

    DSP_DumpData_Ctrl.BufPtr = &g_audio_dump_buffer[16];
    DSP_DumpData_Ctrl.BufSize = sizeof(g_audio_dump_buffer) - 16;
    DSP_DumpData_Ctrl.BufCnt = 0;
    DSP_DumpData_Ctrl.BufWo  = 0;
    DSP_DumpData_Ctrl.BufRo  = 0;

    DSP_DumpData_Ctrl.DataDump_Semaphore = xSemaphoreCreateMutex();
    AUDIO_ASSERT(DSP_DumpData_Ctrl.DataDump_Semaphore != NULL);

#ifdef MTK_AUDIO_DUMP_BY_SPDIF_DEBUG_ENABLE

    {
        uint32_t i, seq;
        uint8_t *p_buf;

#if 1
        for (i = 0; i < sizeof(g_test_buffer); i++) {
            g_test_buffer[i] = i % 256;
        }
#endif
#if 0
        seq = 0;
        p_buf = (uint8_t *)(memory_handle->buffer_addr);
        for (i = 0; i < memory_handle->buffer_length; i += 4) {
            p_buf[i] = 0;
            p_buf[i + 1] = seq++ % 256;
            p_buf[i + 2] = seq++ % 256;
            p_buf[i + 3] = seq++ % 256;
        }
#endif
    }
    DSP_MW_LOG_I("[SPDIF_DUMP] init 0x%08x, 0x%08x, 0x%08x, 0x%08x", 4, *(volatile uint32_t *)(0xC00003E4), *(volatile uint32_t *)(0xC00003A0), *(volatile uint32_t *)(0xC00002E4), *(volatile uint32_t *)(0xC000030C));
#endif

    DSP_MW_LOG_I("[SPDIF_DUMP] init done", 0);
}

void audio_dump_init(void)
{
    if (AudioDumpDevice == AUDIO_DUMP_DEVICE_SPDIF) {
        spdif_dump_init();
    }
}

#endif

void audio_dump_set_block(bool is_block_dump)
{
    block_dump_flag = is_block_dump;
    DSP_MW_LOG_I("[Audio Dump] set block dump enable:%d", 1, block_dump_flag);
}

ATTR_TEXT_IN_IRAM void audio_dump_normal(U8 *audio, U32 audio_size, U32 dumpID)
{
    U32 left_size, curr_size, left_send_size, curr_send_size = 0;
    U8 *p_curr_audio;
    U16 dump_id = dumpID;
    if (AudioDumpDevice == AUDIO_DUMP_DEVICE_LOGGING) {
        left_size    = audio_size;
        p_curr_audio = audio;
        while(left_size) {
            if (left_size <= (MAX_DUMP_SIZE - DUMP_ID_SIZE - ACCUM_BYTES_SIZE)) {
                curr_size = left_size;
            } else {
                curr_size = MAX_DUMP_SIZE - DUMP_ID_SIZE - ACCUM_BYTES_SIZE;
            }
            increase_accum_bytes(dump_id,curr_size);
            U32 accum_bytes = get_accum_bytes(dump_id);
            uint8_t *audio_buffer_array[] = {(uint8_t *)&dump_id, (uint8_t *)&accum_bytes, p_curr_audio, NULL};
            uint32_t audio_buffer_length_array[] = {DUMP_ID_SIZE, ACCUM_BYTES_SIZE, curr_size};
            left_send_size = curr_size + DUMP_ID_SIZE + ACCUM_BYTES_SIZE;
            LOG_TLVDUMP_I(audio_module, LOG_TYPE_AUDIO_V2_DATA, audio_buffer_array, audio_buffer_length_array, curr_send_size);
            if (curr_send_size != left_send_size) {
                DSP_MW_LOG_E("[Audio Dump] drop happen, id:%d, sent %d, cur packet total %d, total size:%d", 4, dump_id, curr_send_size, left_send_size, audio_size);
                increase_accum_bytes(dump_id, left_size - curr_size);
                return;
            }
            p_curr_audio += curr_size;
            left_size -= curr_size;
        }
    }
#ifdef AIR_AUDIO_DUMP_BY_SPDIF_ENABLE
    else if (AudioDumpDevice == AUDIO_DUMP_DEVICE_SPDIF) {
        U32 miss_cnt = 0;
#ifdef MTK_AUDIO_DUMP_BY_SPDIF_DEBUG_ENABLE
        left_size = sizeof(g_test_buffer);
        p_curr_audio = g_test_buffer;
#else
        left_size = audio_size;
        p_curr_audio = audio;
#endif
        do {
            curr_size = DSP_DataDump_DataIn(p_curr_audio, left_size, dumpID);
            if ((curr_size == 0) && (miss_cnt++ >= 3)) {
                miss_cnt--;
                DSP_MW_LOG_E("audio dump drop happen, miss_cnt:%d", 1, miss_cnt);
                break;
            }
            left_size -= curr_size;
            p_curr_audio += curr_size;
        } while (left_size);
    }
#endif
    else {
        DSP_MW_LOG_E("audio dump, device %d not support", 1, AudioDumpDevice);
    }
}

/******************************************************
 *                                                    *
 * This API will Block your task untill dump finish.  *
 * If set 6M, the dump data rate is 500k/s.           *
 *                                                    *
*******************************************************/
void audio_dump_block(U8 *buffer, U32 length, U32 dump_id)
{
    U32 one_time_len = 512;
    U32 dump_times   = length / one_time_len;
    U32 left_size    = length % one_time_len;
    for (U32 i = 0; i < dump_times; i++) {
        audio_dump_normal((U8 *)(buffer + i * one_time_len), one_time_len, dump_id);
        hal_gpt_delay_ms(1); //512 bytes need 1ms
    }
    if (left_size) {
        audio_dump_normal((U8 *)(buffer + dump_times * one_time_len), left_size, dump_id);
    }
}

ATTR_TEXT_IN_IRAM void LOG_AUDIO_DUMP(U8 *audio, U32 audio_size, U32 dump_id)
{
    if (check_cfg_dump_id(dump_id) || (dump_id > DSP_DATADUMP_TEST_ID_MIN)){
        if(audio == NULL){
            DSP_MW_LOG_E("[Audio Dump] get NULL buffer,dump id: %d", 1, dump_id);
            return;
        }
        if(block_dump_flag){
            audio_dump_block(audio, audio_size, dump_id);
        } else {
            audio_dump_normal(audio, audio_size, dump_id);
        }
    }
}

